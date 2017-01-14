#include "common/common_types.h"

#pragma once

enum class MortonPass { Tile8x8, Tile32x32 };

namespace Decoders {
bool Morton_8x8(u8* morton_buffer, u8* matrix_buffer, u32 width, u32 height, u32 bpp);
bool Morton_32x32(u8* morton_buffer, u8* matrix_buffer, u32 width, u32 height, u32 bpp);
}

namespace Encoders {
bool Morton_8x8(u8* morton_buffer, u8* matrix_buffer, u32 width, u32 height, u32 bpp);
bool Morton_32x32(u8* morton_buffer, u8* matrix_buffer, u32 width, u32 height, u32 bpp);
}
