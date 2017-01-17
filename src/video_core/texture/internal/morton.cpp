#include "common/common_types.h"
#include "video_core/texture/internal/morton.h"

static u32 Part1By1(u32 x) {
    x &= 0x0000ffff;                 // x = ---- ---- ---- ---- fedc ba98 7654 3210
    x = (x ^ (x << 8)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
    x = (x ^ (x << 4)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
    x = (x ^ (x << 2)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
    x = (x ^ (x << 1)) & 0x55555555; // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
    return x;
}

static u32 Compact1By1(u32 x) {
    x &= 0x55555555;                 // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
    x = (x ^ (x >> 1)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
    x = (x ^ (x >> 2)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
    x = (x ^ (x >> 4)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
    x = (x ^ (x >> 8)) & 0x0000ffff; // x = ---- ---- ---- ---- fedc ba98 7654 3210
    return x;
}

static u32 EncodeMorton(u32 x, u32 y) {
    return (Part1By1(y) << 1) | Part1By1(x);
}

static u32 DecodeMortonX(u32 code) {
    return Compact1By1(code >> 0);
}

static u32 DecodeMortonY(u32 code) {
    return Compact1By1(code >> 1);
}

u32 MortonOffset(u32 x, u32 y, u32 width, u32 height, u32 tiling, u32 bpp) {
    u32 tile = (x + y * height) * width / (tiling * tiling);
    tile = (tile * bpp) / 8;
    return tile + EncodeMorton(x % tiling, y % tiling);
}

#include "morton8x8_optimized.cpp"
