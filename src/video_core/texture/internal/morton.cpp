#include <cstring>
#include <memory>
#include <utility>
#include "common/common_types.h"
#include "video_core/texture/internal/morton.h"
#include "video_core/texture/internal/texture_utils.h"

///////////////////////////////////////////////////////////////////////////////
// Optimizations
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma inline_recursion(on)
// Normaly set to 16 by default, the best balance seems to be on 8 for this module
#pragma inline_depth(8)
// favor fast code over small code.
#pragma optimize("t", on)
#pragma intrinsic(memcpy)
#define __hot
#define __no_inline __declspec(noinline)
#elif defined(CLANG_OR_GCC)
// The next 3 will swizle memory copying to help find the best sse/avx shuffling
// in case it's possible. Compilation tests have proven effective use of these
// flags on gcc and clang.
#pragma GCC optimize("-fpredictive-commoning")
#pragma GCC optimize("-ftree-loop-distribute-patterns")
#pragma GCC optimize("-ftree-vectorize")
#pragma GCC option("--param inline-unit-growth=400")
#pragma GCC option("--param large-function-growth=800")
// The beauty of these compiler options is that they generate better code than
// hand written intrinsics, since inline expanding memeory transfers can be pattern
// matched with vector instructions available in the target.
#define __no_inline __attribute__((noinline))
#define __hot __attribute__((hot))
#if !defined(__forceinline)
#define __forceinline attribute__((always_inline))
#endif
#else
#define __hot
#define __no_inline
#define __forceinline
#endif

#pragma region Z_Order
/////////////////////////////////////////////////////////////////////////////
//          Z-Order:
//
//                    0-->1
//                      /
//                    2-->3
//
// for more information look at: https://en.wikipedia.org/wiki/Z-order_curve
/////////////////////////////////////////////////////////////////////////////
#define TOP_LEFT 0
#define TOP_RIGHT 1
#define BOTTOM_LEFT 2
#define BOTTOM_RIGHT 3

constexpr u32 isRight(u32 block_index) {
    return (block_index % 2);
}

constexpr u32 isBottom(u32 block_index) {
    return (block_index / 2);
}

template <void codec(u8*, u8*, size_t), size_t nibbles, u32 blocks, size_t block_size>
__forceinline static void swizzle_block(u8*& morton_block, u8* linear_block);

template <void codec(u8*, u8*, size_t), size_t nibbles, u32 block_index, u32 blocks,
          size_t block_size>
__forceinline static void swizzle_block_aux(u8*& morton_block, u8* linear_block) {
    // move the linear_block pointer to the appropiate block
    const size_t right = isRight(block_index) * (blocks * nibbles) / 2;
    const size_t down = isBottom(block_index) * block_size;
    u8* new_linear = linear_block + right + down;
    swizzle_block<codec, nibbles, blocks, block_size>(morton_block, new_linear);
}

template <void codec(u8*, u8*, size_t), size_t nibbles, u32 blocks, size_t block_size>
__forceinline static void swizzle_block(u8*& morton_block, u8* linear_block) {
    const size_t new_block_size = block_size / 2;
    if (blocks <= 2) {
        // We handle 2*2 blocks on z-order
        const size_t read_size = nibbles; // just for clearness. It's the same amount
        // TOP_LEFT & TOP_RIGHT
        codec(morton_block, linear_block, read_size);
        morton_block += read_size;
        // BOTTOM_LEFT & BOTTOM_RIGHT
        codec(morton_block, linear_block + new_block_size, read_size);
        morton_block += read_size;
    } else {
        // we divide the block into 4 blocks in z-order corecursively
        // until we have 2x2 blocks.
        const u32 subdivide = blocks / 2;
        swizzle_block_aux<codec, nibbles, TOP_LEFT, subdivide, new_block_size>(morton_block,
                                                                               linear_block);
        swizzle_block_aux<codec, nibbles, TOP_RIGHT, subdivide, new_block_size>(morton_block,
                                                                                linear_block);
        swizzle_block_aux<codec, nibbles, BOTTOM_LEFT, subdivide, new_block_size>(morton_block,
                                                                                  linear_block);
        swizzle_block_aux<codec, nibbles, BOTTOM_RIGHT, subdivide, new_block_size>(morton_block,
                                                                                   linear_block);
    }
}

template <void codec(u8*, u8*, size_t), size_t nibbles, size_t lines_per_block>
__forceinline static void swizzle_pass(u8* morton_block, u8* linear_block) {
    const size_t block_size = (lines_per_block * lines_per_block * nibbles) / 2;
    swizzle_block<codec, nibbles, lines_per_block, block_size>(morton_block, linear_block);
}
#pragma endregion Z_Order

template <size_t nibbles, size_t lines_per_block>
__hot inline static void encode_pass(u8* morton_buffer, u8* linear_buffer, u32 x_blocks) {
    const u32 tile_size = (lines_per_block * lines_per_block * nibbles) / 2;
    alignas(64) u8 tmp[tile_size];
    tiling_pass<&encode, nibbles, lines_per_block>(linear_buffer, tmp, x_blocks);
    swizzle_pass<&encode, nibbles, lines_per_block>(morton_buffer, tmp);
}

template <size_t nibbles, size_t lines_per_block>
__hot inline static void decode_pass(u8* morton_buffer, u8* linear_buffer, u32 x_blocks) {
    const u32 tile_size = (lines_per_block * lines_per_block * nibbles) / 2;
    alignas(64) u8 tmp[tile_size];
    swizzle_pass<&decode, nibbles, lines_per_block>(morton_buffer, tmp);
    tiling_pass<&decode, nibbles, lines_per_block>(linear_buffer, tmp, x_blocks);
}

template <void codec(u8*, u8*, u32), size_t nibbles, size_t lines_per_block>
__hot static void morton_pass(u8* morton_buffer, u8* matrix_buffer, u32 width, u32 height) {
    const u32 x_blocks = (width / lines_per_block);
    const u32 y_blocks = (height / lines_per_block);
    const size_t line_size = (lines_per_block * nibbles) / 2;
    const size_t tile_size = lines_per_block * line_size;
    const size_t stride_size = width * line_size;
    matrix_buffer = matrix_buffer + ((height * width * nibbles) / 2) - stride_size;
    for (u32 y = 0; y < y_blocks; y++) {
        u8* linear_buffer = matrix_buffer;
        for (u32 x = 0; x != x_blocks; x++) {
            codec(morton_buffer, linear_buffer, x_blocks);
            linear_buffer += line_size;
            morton_buffer += tile_size;
        }
        matrix_buffer -= stride_size;
    }
}

// keep hot code together
__no_inline __hot static void morton_8x8_32(u8* morton_buffer, u8* matrix_buffer, u32 width,
                                            u32 height, bool decode) {
    if (decode)
        morton_pass<&decode_pass<8, 8>, 8, 8>(morton_buffer, matrix_buffer, width, height);
    else
        morton_pass<&encode_pass<8, 8>, 8, 8>(morton_buffer, matrix_buffer, width, height);
}

namespace Decoders {

bool Morton_8x8(u8* morton_buffer, u8* matrix_buffer, u32 width, u32 height, u32 bpp) {
    if (bpp == 32) {
        morton_8x8_32(morton_buffer, matrix_buffer, width, height, true);
        return true;
    }
    switch (bpp) {
    case 4: {
        morton_pass<&decode_pass<1, 8>, 1, 8>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 8: {
        morton_pass<&decode_pass<2, 8>, 2, 8>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 16: {
        morton_pass<&decode_pass<4, 8>, 4, 8>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 24: {
        morton_pass<&decode_pass<6, 8>, 6, 8>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    default: {
        return false;
        break;
    }
    }
}

bool Morton_32x32(u8* morton_buffer, u8* matrix_buffer, u32 width, u32 height, u32 bpp) {
    switch (bpp) {
    case 4: {
        morton_pass<&decode_pass<1, 32>, 1, 32>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 8: {
        morton_pass<&decode_pass<2, 32>, 2, 32>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 16: {
        morton_pass<&decode_pass<4, 32>, 4, 32>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 24: {
        morton_pass<&decode_pass<6, 32>, 6, 32>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 32: {
        morton_pass<&decode_pass<8, 32>, 8, 32>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    default: {
        return false;
        break;
    }
    }
}
}

namespace Encoders {

bool Morton_8x8(u8* morton_buffer, u8* matrix_buffer, u32 width, u32 height, u32 bpp) {
    if (bpp == 32) {
        morton_8x8_32(morton_buffer, matrix_buffer, width, height, false);
        return true;
    }
    switch (bpp) {
    case 4: {
        morton_pass<&encode_pass<1, 8>, 1, 8>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 8: {
        morton_pass<&encode_pass<2, 8>, 2, 8>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 16: {
        morton_pass<&encode_pass<4, 8>, 4, 8>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 24: {
        morton_pass<&encode_pass<6, 8>, 6, 8>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    default: {
        return false;
        break;
    }
    }
}

bool Morton_32x32(u8* morton_buffer, u8* matrix_buffer, u32 width, u32 height, u32 bpp) {
    switch (bpp) {
    case 4: {
        morton_pass<&encode_pass<1, 32>, 1, 32>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 8: {
        morton_pass<&encode_pass<2, 32>, 2, 32>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 16: {
        morton_pass<&encode_pass<4, 32>, 4, 32>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 24: {
        morton_pass<&encode_pass<6, 32>, 6, 32>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    case 32: {
        morton_pass<&encode_pass<8, 32>, 8, 32>(morton_buffer, matrix_buffer, width, height);
        return true;
        break;
    }
    default: {
        return false;
        break;
    }
    }
}
}
