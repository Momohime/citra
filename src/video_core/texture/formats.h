#pragma once

namespace Pica {

namespace Texture {

enum class Format {
    // First 5 formats are shared between textures and color buffers
    RGBA8 = 0,
    RGB8 = 1,
    RGB5A1 = 2,
    RGB565 = 3,
    RGBA4 = 4,

    // Texture-only formats
    IA8 = 5,
    RG8 = 6,
    I8 = 7,
    A8 = 8,
    IA4 = 9,
    I4 = 10,
    A4 = 11,
    ETC1 = 12,
    ETC1A4 = 13,

    // Depth buffer-only formats
    D16 = 14,
    // gap
    D24 = 16,
    D24S8 = 17,

    Invalid = 255,
};

} // Texture

} // Pica
