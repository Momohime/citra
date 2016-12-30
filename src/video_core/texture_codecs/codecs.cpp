#include <cstring>
#include <memory>
#include <utility>
#include "common/assert.h"
#include "video_core/texture_codecs/codecs.h"

/*
 * Static compilers can't always detect if vectorization is possible,
 * if the programmer is 100% sure it's possible to vectorize a set
 * of actions, it can hint the compiler that it can vectorize a loop
 * unconditionaly.
 */
#ifdef _MSC_VER
#define VECTORIZE_NEXT __pragma("loop( ivdep )")
#elif __GNUC__
#define VECTORIZE_NEXT _Pragma("GCC ivdep")
#elif __clang__
#define VECTORIZE_NEXT _Pragma("clang loop vectorize(enable) interleave(enable)")
#else
#define VECTORIZE_NEXT
#endif

// from GNU C Library under GPL 2.
// https://github.com/lattera/glibc/blob/master/bits/byteswap.h
/* Swap bytes in 32 bit value.  */

#ifdef __GNUC__
static inline unsigned int byte_swap_32(unsigned int __bsx) {
    return __builtin_bswap32(__bsx);
}
#else
#define __bswap_constant_32(x)                                                                     \
    ((((x)&0xff000000u) >> 24) | (((x)&0x00ff0000u) >> 8) | (((x)&0x0000ff00u) << 8) |             \
     (((x)&0x000000ffu) << 24))
static inline unsigned int byte_swap_32(unsigned int __bsx) {
    return __bswap_constant_32(__bsx);
}
#endif

// lil detail for linux, funny that mingw has no problem without it.
using namespace std;

// Note: The function layout is made on purpose to help the compiler
// unfold the loop and simplify the moves to the best appropiate type in use.
// compiling for ivy-bridge-up will unfold the loop further and use AVX2
template <int read_size>
inline void decode_simple(u8* from, u8* out) {
    memcpy(out, from, read_size * 2);
}

template <int read_size>
inline void encode_simple(u8* from, u8* out) {
    memcpy(from, out, read_size * 2);
}

inline void decode_depth(u8* from, u8* out) {
    out[0] = from[0];
    out[1] = from[1];
    out[2] = from[2];
    out[3] = 0;
    out[4] = from[3];
    out[5] = from[4];
    out[6] = from[5];
    out[7] = 0;
}

inline void encode_depth(u8* from, u8* out) {
    out[0] = from[0];
    out[1] = from[1];
    out[2] = from[2];
    out[3] = from[4];
    out[4] = from[5];
    out[5] = from[6];
}

// finaly, we decode to cursors/encode to blocks, the corresponding data by
// moving the appropiate
// 02 03 -> encode/decode second
// -----
// 00 01 -> encode/decode first
template <void func(u8*, u8*), int read_size, int write_size>
inline void morton_block2x2(u8* from, u8*& w1, u8*& w2) {
    func(from, w1);
    w1 += write_size * 2;
    func(from + read_size * 2, w2);
    w2 += write_size * 2;
}

// Again, we subdivide the 4x4 tiles and assign the each 2x2 subblock to the
// corresponding cursors.
//
// 10 11 | 14 15
// 08 09 | 12 13
// ------------
// 02 03 | 06 07
// 00 01 | 04 05
template <void func(u8*, u8*), int read_size, int write_size>
inline void morton_block4x4(u8* from, u8** w1, u8** w2) {
    u8* tmp_block = from;
    morton_block2x2<func, read_size, write_size>(tmp_block, w1[0], w1[1]);
    tmp_block += read_size * 4;
    morton_block2x2<func, read_size, write_size>(tmp_block, w1[0], w1[1]);
    tmp_block += read_size * 4;
    morton_block2x2<func, read_size, write_size>(tmp_block, w2[0], w2[1]);
    tmp_block += read_size * 4;
    morton_block2x2<func, read_size, write_size>(tmp_block, w2[0], w2[1]);
}

// We subdivide the 8x8 tiles and assign the each 4x4 subblock to the
// corresponding cursors.
//
// 42 43 46 47 | 58 59 62 63
// 40 41 44 45 | 56 57 60 61
// 34 35 38 39 | 50 51 54 55
// 32 33 36 37 | 48 49 52 53
// -----------------------
// 10 11 14 15 | 26 27 30 31
// 08 09 12 13 | 24 25 28 29
// 02 03 06 07 | 18 19 22 23
// 00 01 04 05 | 16 17 20 21
template <void func(u8*, u8*), int read_size, int write_size>
inline void morton_block8x8(u8* from, u8** cursors) {
    u8* tmp_block = from;
    morton_block4x4<func, read_size, write_size>(tmp_block, &cursors[0], &cursors[2]);
    tmp_block += read_size * 16;
    morton_block4x4<func, read_size, write_size>(tmp_block, &cursors[0], &cursors[2]);
    tmp_block += read_size * 16;
    morton_block4x4<func, read_size, write_size>(tmp_block, &cursors[4], &cursors[6]);
    tmp_block += read_size * 16;
    morton_block4x4<func, read_size, write_size>(tmp_block, &cursors[4], &cursors[6]);
}

template <int read_size>
inline void rewind_cursors(u8** cursors, u8* write_p, u32 width) {
    cursors[0] = write_p;
    cursors[1] = write_p - read_size * width;
    cursors[2] = write_p - read_size * 2 * width;
    cursors[3] = write_p - read_size * 3 * width;
    cursors[4] = write_p - read_size * 4 * width;
    cursors[5] = write_p - read_size * 5 * width;
    cursors[6] = write_p - read_size * 6 * width;
    cursors[7] = write_p - read_size * 7 * width;
}

// from video_cor/utils.h
// Images are split into 8x8 tiles. Each tile is composed of four 4x4 subtiles each
// of which is composed of four 2x2 subtiles each of which is composed of four texels.
// Each structure is embedded into the next-bigger one in a diagonal pattern, e.g.
// texels are laid out in a 2x2 subtile like this:
// 2 3
// 0 1
//
// The full 8x8 tile has the texels arranged like this:
//
// 42 43 46 47 58 59 62 63
// 40 41 44 45 56 57 60 61
// 34 35 38 39 50 51 54 55
// 32 33 36 37 48 49 52 53
// 10 11 14 15 26 27 30 31
// 08 09 12 13 24 25 28 29
// 02 03 06 07 18 19 22 23
// 00 01 04 05 16 17 20 21
//
// This pattern is what's called Z-order curve, or Morton order.
//
// The algorithm below processos z-ordered images block by block.
// reading/writting in 8 cursors which point to the start of each
// row of a normal width*height raw pixel image.
template <void func(u8*, u8*), int read_size, int write_size>
inline void morton(u8* in_p, u8* write_p, u32 width, u32 height) {
    u32 x_blocks = (width / 8);
    u32 y_blocks = (height / 8);
    u8* block_pointer = in_p;
    u8* cursors[8];
    u32 step = (8 * width) * write_size;
    write_p += read_size * (width * (height - 1));
    for (u32 y = 0; y != y_blocks; y++) {
        rewind_cursors<read_size>(cursors, write_p, width);
        VECTORIZE_NEXT for (u32 x = 0; x != x_blocks; x++) {
            morton_block8x8<func, read_size, write_size>(block_pointer, cursors);
            block_pointer += 64 * read_size;
        }
        write_p -= step;
    }
}

// These macros are used to unroll/unfold the same action on tight loops
// should be used on actions that don't branch the pipeline.
// Static compilers can't detect unrollable loops easily. Normaly,
// they require some profiling data to unroll loops.
#define LOOP_UNROLL_1(CODE) CODE
#define LOOP_UNROLL_2(CODE)                                                                        \
    LOOP_UNROLL_1(CODE);                                                                           \
    LOOP_UNROLL_1(CODE)
#define LOOP_UNROLL_4(CODE)                                                                        \
    LOOP_UNROLL_2(CODE);                                                                           \
    LOOP_UNROLL_2(CODE)
#define LOOP_UNROLL_8(CODE)                                                                        \
    LOOP_UNROLL_4(CODE);                                                                           \
    LOOP_UNROLL_4(CODE)
#define LOOP_UNROLL_16(CODE)                                                                       \
    LOOP_UNROLL_8(CODE);                                                                           \
    LOOP_UNROLL_8(CODE)

template <void func(u8*&)>
inline void map_image(u8*& out_buffer, u32 width, u32 height) {
    u32 writes = width * height / 16; // 16 unfolds
    VECTORIZE_NEXT for (u32 i = 0; i != writes; i++) {
        LOOP_UNROLL_16(func(out_buffer));
    }
    // Now just do the rest
    writes = width * height - (writes * 16);
    u32 jump = (writes % 8);
    // This form of loop unfolding works for every set of data at the
    // expense of not marshelling/vectorizing but won't break the pipeline
    switch (jump) {
        do {
            jump = 8;
            func(out_buffer);
        case 7:
            func(out_buffer);
        case 6:
            func(out_buffer);
        case 5:
            func(out_buffer);
        case 4:
            func(out_buffer);
        case 3:
            func(out_buffer);
        case 2:
            func(out_buffer);
        case 1:
            func(out_buffer);
        case 0:
        default:
            writes -= jump;
        } while (writes != 0);
    }
}

template <void func(u8*&, u8*&)>
inline void unfold_image(u8*& read_cursor, u8*& write_cursor, u32 width, u32 height) {
    u32 writes = width * height / 16; // 16 unfolds
    VECTORIZE_NEXT for (u32 i = 0; i != writes; i++) {
        LOOP_UNROLL_16(func(read_cursor, write_cursor));
    }
    // Now just do the rest
    writes = width * height - (writes * 16);
    u32 jump = (writes % 8);
    // This form of loop unfolding works for every set of data at the
    // expense of not marshelling/vectorizing but won't break the pipeline
    switch (jump) {
        do {
            jump = 8;
            func(read_cursor, write_cursor);
        case 7:
            func(read_cursor, write_cursor);
        case 6:
            func(read_cursor, write_cursor);
        case 5:
            func(read_cursor, write_cursor);
        case 4:
            func(read_cursor, write_cursor);
        case 3:
            func(read_cursor, write_cursor);
        case 2:
            func(read_cursor, write_cursor);
        case 1:
            func(read_cursor, write_cursor);
        case 0:
        default:
            writes -= jump;
        } while (writes != 0);
    }
}

// Big Endian Decoding
inline void big_u32(u8*& out_buffer) {
    u32 tmp;
    memcpy(&tmp, out_buffer, sizeof(u32));
    tmp = byte_swap_32(tmp);
    memcpy(out_buffer, &tmp, sizeof(u32));
    out_buffer += 4;
}

inline void color_i8(u8*& read_cursor, u8*& write_cursor) {
    read_cursor -= 1;
    write_cursor -= 4;
    u32 tmp = 0;
    u8 tmp2;
    memcpy(&tmp2, read_cursor, sizeof(u8));
    tmp = tmp2 & 0x000000FF;
    tmp = (tmp << 16) | (tmp << 8) | tmp | 0xFF000000;
    memcpy(write_cursor, &tmp, sizeof(u32));
}

inline void color_a8(u8*& read_cursor, u8*& write_cursor) {
    read_cursor -= 1;
    write_cursor -= 4;
    u32 tmp = 0;
    u8 tmp2;
    memcpy(&tmp2, read_cursor, sizeof(u8));
    tmp = tmp2 & 0x000000FF;
    tmp = tmp << 24;
    memcpy(write_cursor, &tmp, sizeof(u32));
}

inline void color_ia8(u8*& read_cursor, u8*& write_cursor) {
    read_cursor -= 2;
    write_cursor -= 4;
    u32 tmp = 0;
    u16 tmp2;
    memcpy(&tmp2, read_cursor, sizeof(u16));
    tmp = tmp2 & 0x0000FF00;
    tmp2 = tmp2 & 0x00FF;
    tmp = (tmp << 8) | (tmp >> 8) | tmp | (tmp2 << 24);
    memcpy(write_cursor, &tmp, sizeof(u32));
}

static inline void rotateLeft(u8*& out_buffer) {
    u32 tmp;
    memcpy(&tmp, out_buffer, sizeof(u32));
    tmp = (tmp >> 8) | (tmp << 24);
    memcpy(out_buffer, &tmp, sizeof(u32));
    out_buffer += 4;
}

static inline void rotateRight(u8*& out_buffer) {
    u32 tmp;
    memcpy(&tmp, out_buffer, sizeof(u32));
    tmp = (tmp >> 24) | (tmp << 8);
    memcpy(out_buffer, &tmp, sizeof(u32));
    out_buffer += 4;
}

constexpr u8 Convert4To8(u8 value) {
    return (value << 4) | value;
}

inline void nimble_write(u8*& in_buffer, u8*& out_buffer) {
    out_buffer[0] = Convert4To8((*in_buffer & 0xF0) >> 4);
    out_buffer[1] = Convert4To8(*in_buffer & 0x0F);
    in_buffer++;
    out_buffer += 2;
}

namespace Pica {

namespace Encoders {

bool Morton(u8* in_buffer, u8* out_buffer, u32 width, u32 height, u32 bytespp) {
    // Sanity checks
    std::swap(in_buffer, out_buffer);
    ASSERT(in_buffer != nullptr && out_buffer != nullptr);
    ASSERT(((u64)in_buffer & 3) == 0);
    ASSERT(((u64)out_buffer & 3) == 0);
    ASSERT(width >= 8);
    ASSERT(height >= 8);
    ASSERT((width * height) % 64 == 0);
    switch (bytespp) {
    case 1: {
        morton<&encode_simple<1>, 1, 1>(in_buffer, out_buffer, width, height);
        return true;
        break;
    }
    case 2: {
        morton<&encode_simple<2>, 2, 2>(in_buffer, out_buffer, width, height);
        return true;
        break;
    }
    case 3: {
        morton<&encode_simple<3>, 3, 3>(in_buffer, out_buffer, width, height);
        return true;
        break;
    }
    case 4: {
        morton<&encode_simple<4>, 4, 4>(in_buffer, out_buffer, width, height);
        return true;
        break;
    }
    default: {
        return false;
        break;
    }
    }
}

void MortonU32_U24(u8* in_buffer, u8* out_buffer, u32 width, u32 height) {
    morton<&encode_depth, 4, 3>(in_buffer, out_buffer, width, height);
}

void Depth(u8* out_buffer, u32 width, u32 height) {
    map_image<&rotateLeft>(out_buffer, width, height);
}

} // Encoders

namespace Decoders {

void MortonU24_U32(u8* in_buffer, u8* out_buffer, u32 width, u32 height) {
    morton<&decode_depth, 3, 4>(in_buffer, out_buffer, width, height);
}

bool Morton(u8* in_buffer, u8* out_buffer, u32 width, u32 height, u32 bytespp) {
    // Sanity checks
    ASSERT(in_buffer != nullptr && out_buffer != nullptr);
    ASSERT(((u64)in_buffer & 3) == 0);
    ASSERT(((u64)out_buffer & 3) == 0);
    ASSERT(width >= 8);
    ASSERT(height >= 8);
    ASSERT((width * height) % 64 == 0);
    switch (bytespp) {
    case 1: {
        morton<&decode_simple<1>, 1, 1>(in_buffer, out_buffer, width, height);
        return true;
        break;
    }
    case 2: {
        morton<&decode_simple<2>, 2, 2>(in_buffer, out_buffer, width, height);
        return true;
        break;
    }
    case 3: {
        morton<&decode_simple<3>, 3, 3>(in_buffer, out_buffer, width, height);
        return true;
        break;
    }
    case 4: {
        morton<&decode_simple<4>, 4, 4>(in_buffer, out_buffer, width, height);
        return true;
        break;
    }
    default: {
        return false;
        break;
    }
    }
}

void BigEndian(u8* out_buffer, u32 width, u32 height) {
    map_image<&big_u32>(out_buffer, width, height);
}

void Depth(u8* out_buffer, u32 width, u32 height) {
    map_image<&rotateRight>(out_buffer, width, height);
}

void I8(u8* out_buffer, u32 width, u32 height) {
    u8* read_cursor = out_buffer + (width * height);
    u8* write_cursor = out_buffer + (width * height * 4);
    unfold_image<&color_i8>(read_cursor, write_cursor, width, height);
}

void A8(u8* out_buffer, u32 width, u32 height) {
    u8* read_cursor = out_buffer + (width * height);
    u8* write_cursor = out_buffer + (width * height * 4);
    unfold_image<&color_a8>(read_cursor, write_cursor, width, height);
}

void IA8(u8* out_buffer, u32 width, u32 height) {
    u8* read_cursor = out_buffer + (width * height * 2);
    u8* write_cursor = out_buffer + (width * height * 4);
    unfold_image<&color_ia8>(read_cursor, write_cursor, width, height);
}

// Nimbles

void Nimbles(u8* in_buffer, u8* out_buffer, u32 width, u32 height) {
    u32 writes = width * height / 32; // 16 unfolds
    for (u32 i = 0; i != writes; i++) {
        LOOP_UNROLL_16(nimble_write(in_buffer, out_buffer));
    }
    // Now just do the rest
    writes = width * height - (writes * 32);
    for (u32 i = 0; i != writes; i++) {
        LOOP_UNROLL_1(nimble_write(in_buffer, out_buffer));
    }
}

} // TextureUtils

} // Pica
