#include <array>
#include <cstring>
#include <memory>
#include <utility>
#include "common/assert.h"
#include "common/bit_field.h"
#include "common/color.h"
#include "common/common_types.h"
#include "common/math_util.h"
#include "common/swap.h"
#include "common/vector_math.h"
#include "video_core/texture/internal/etc1.h"
#include "video_core/texture/internal/texture_utils.h"

namespace {

#ifdef _DEBUG
#define CONST_FIX static
#else
#define CONST_FIX constexpr
#endif

CONST_FIX std::array<u8[2], 8> etc1_modifier_table = {{
    {2, 8}, {5, 17}, {9, 29}, {13, 42}, {18, 60}, {24, 80}, {33, 106}, {47, 183},
}};

constexpr u32 buildRGBA(u32 r, u32 g, u32 b, u32 a) {
    return (a << 24) | (b << 16) | (g << 8) | r;
}

union ETC1Tile {
    u64 raw;

    // Each of these two is a collection of 16 bits (one per lookup value)
    BitField<0, 16, u64> table_subindexes;
    BitField<16, 16, u64> negation_flags;

    unsigned GetTableSubIndex(unsigned index) const {
        return (table_subindexes >> index) & 1;
    }

    bool GetNegationFlag(unsigned index) const {
        return ((negation_flags >> index) & 1) == 1;
    }

    BitField<32, 1, u64> flip;
    BitField<33, 1, u64> differential_mode;

    BitField<34, 3, u64> table_index_2;
    BitField<37, 3, u64> table_index_1;

    union {
        // delta value + base value
        BitField<40, 3, s64> db;
        BitField<43, 5, u64> b;

        BitField<48, 3, s64> dg;
        BitField<51, 5, u64> g;

        BitField<56, 3, s64> dr;
        BitField<59, 5, u64> r;
    } differential;

    union {
        BitField<40, 4, u64> b2;
        BitField<44, 4, u64> b1;

        BitField<48, 4, u64> g2;
        BitField<52, 4, u64> g1;

        BitField<56, 4, u64> r2;
        BitField<60, 4, u64> r1;
    } separate;

    const u32 GetRGB(u32 x, u32 y) const {
        int texel = 4 * x + y;

        if (flip)
            std::swap(x, y);

        // Lookup base value
        Math::Vec3<int> ret;
        if (differential_mode) {
            ret.r() = static_cast<int>(differential.r);
            ret.g() = static_cast<int>(differential.g);
            ret.b() = static_cast<int>(differential.b);
            if (x >= 2) {
                ret.r() += static_cast<int>(differential.dr);
                ret.g() += static_cast<int>(differential.dg);
                ret.b() += static_cast<int>(differential.db);
            }
            ret.r() = Color::Convert5To8(ret.r());
            ret.g() = Color::Convert5To8(ret.g());
            ret.b() = Color::Convert5To8(ret.b());
        } else {
            if (x < 2) {
                ret.r() = Color::Convert4To8(static_cast<u8>(separate.r1));
                ret.g() = Color::Convert4To8(static_cast<u8>(separate.g1));
                ret.b() = Color::Convert4To8(static_cast<u8>(separate.b1));
            } else {
                ret.r() = Color::Convert4To8(static_cast<u8>(separate.r2));
                ret.g() = Color::Convert4To8(static_cast<u8>(separate.g2));
                ret.b() = Color::Convert4To8(static_cast<u8>(separate.b2));
            }
        }

        // Add modifier
        unsigned table_index =
            static_cast<int>((x < 2) ? table_index_1.Value() : table_index_2.Value());

        int modifier = etc1_modifier_table[table_index][GetTableSubIndex(texel)];
        if (GetNegationFlag(texel))
            modifier *= -1;

        ret.r() = MathUtil::Clamp(ret.r() + modifier, 0, 255);
        ret.g() = MathUtil::Clamp(ret.g() + modifier, 0, 255);
        ret.b() = MathUtil::Clamp(ret.b() + modifier, 0, 255);

        return buildRGBA(ret.r(), ret.g(), ret.b(), 0);
    }
};

} // anonymous namespace

inline void etc1_pass(u8* etc1_buffer, u8* linear_buffer, u32 x_blocks) {
    const size_t line = 8 * 4;
    alignas(64) u8 tmp[line * 8];
    for (u32 i = 0; i < 4; i++) {
        ETC1Tile tile;
        const size_t index = (i % 2) * (line / 2) + (i / 2) * line * 4;
        std::memcpy(&tile.raw, &etc1_buffer[i * 8], 8);
        for (u32 k = 0; k < 4; k++) {
            for (u32 j = 0; j < 4; j++) {
                auto rgb = tile.GetRGB(j, k);
                u32 rgba = rgb | 0xFF000000;
                std::memcpy(&tmp[k * line + j * 4 + index], &rgba, 4);
            }
        }
    }
    tiling_pass<&decode, 8, 8>(linear_buffer, tmp, x_blocks);
}

inline void etc1a4_pass(u8* etc1_buffer, u8* linear_buffer, u32 x_blocks) {
    const size_t line = 8 * 4;
    alignas(64) u8 tmp[line * 8];
    for (u32 i = 0; i < 4; i++) {
        ETC1Tile tile;
        u64 alpha_tile;
        const size_t index = (i % 2) * (line / 2) + (i / 2) * line * 4;
        std::memcpy(&alpha_tile, &etc1_buffer[i * 16], 8);
        std::memcpy(&tile.raw, &etc1_buffer[i * 16 + 8], 8);
        for (u32 k = 0; k < 4; k++) {
            for (u32 j = 0; j < 4; j++) {
                u32 alpha = (alpha_tile >> (4 * (j * 4 + k))) & 0x0F;
                alpha |= (alpha << 4);
                auto rgb = tile.GetRGB(j, k);
                u32 rgba = rgb | (alpha << 24);
                std::memcpy(&tmp[k * line + j * 4 + index], &rgba, 4);
            }
        }
    }
    tiling_pass<&decode, 8, 8>(linear_buffer, tmp, x_blocks);
}

void ETC1A4(u8* etc1_buffer, u8* matrix_buffer, u32 width, u32 height) {
    const u32 x_blocks = (width / 8);
    const u32 y_blocks = (height / 8);
    const size_t line_size = 8 * 4;
    const size_t tile_size = 8 * 8;
    const size_t stride_size = width * line_size;
    matrix_buffer = matrix_buffer + (height * width * 4) - stride_size;
    for (u32 y = 0; y < y_blocks; y++) {
        u8* linear_buffer = matrix_buffer;
        for (u32 x = 0; x != x_blocks; x++) {
            etc1a4_pass(etc1_buffer, linear_buffer, x_blocks);
            linear_buffer += line_size;
            etc1_buffer += tile_size;
        }
        matrix_buffer -= stride_size;
    }
}

void ETC1(u8* etc1_buffer, u8* matrix_buffer, u32 width, u32 height) {
    const u32 x_blocks = (width / 8);
    const u32 y_blocks = (height / 8);
    const size_t line_size = 8 * 4;
    const size_t tile_size = 8 * 8 / 2;
    const size_t stride_size = width * line_size;
    matrix_buffer = matrix_buffer + (height * width * 4) - stride_size;
    for (u32 y = 0; y < y_blocks; y++) {
        u8* linear_buffer = matrix_buffer;
        for (u32 x = 0; x != x_blocks; x++) {
            etc1_pass(etc1_buffer, linear_buffer, x_blocks);
            linear_buffer += line_size;
            etc1_buffer += tile_size;
        }
        matrix_buffer -= stride_size;
    }
}
