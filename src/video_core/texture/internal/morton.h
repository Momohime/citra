#pragma once

#include "common/common_types.h"

u32 EncodeMorton(u32 x, u32 y);
u32 DecodeMortonX(u32 code);
u32 DecodeMortonY(u32 code);

namespace Decoders {
bool Morton_8x8(u8* morton_buffer, u8* matrix_buffer, u32 width, u32 height, u32 bpp);
}

namespace Encoders {
bool Morton_8x8(u8* morton_buffer, u8* matrix_buffer, u32 width, u32 height, u32 bpp);
}
