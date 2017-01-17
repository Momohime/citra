
#pragma once

#include <array>
#include "common/assert.h"
#include "core/hw/gpu.h"
#include "video_core/pica.h"

namespace Pica {

namespace Texture {

struct Format {

    enum class Type {
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

    static const u32 GetBpp(Type format) {
        static const std::array<unsigned int, 18> bpp_table = {
            32, // RGBA8
            24, // RGB8
            16, // RGB5A1
            16, // RGB565
            16, // RGBA4
            16, // IA8
            16, // RG8
            8,  // I8
            8,  // A8
            8,  // IA4
            4,  // I4
            4,  // A4
            4,  // ETC1
            8,  // ETC1A4
            16, // D16
            0,
            24, // D24
            32, // D24S8
        };

        ASSERT((u32)format < ARRAY_SIZE(bpp_table));
        return bpp_table[(u32)format];
    }

    static constexpr Type FromTextureFormat(Regs::TextureFormat format) {
        return ((unsigned int)format < 14) ? (Type)format : Type::Invalid;
    }

    static constexpr Type FromColorFormat(Regs::ColorFormat format) {
        return ((unsigned int)format < 5) ? (Type)format : Type::Invalid;
    }

    static constexpr Type FromDepthFormat(Regs::DepthFormat format) {
        return ((unsigned int)format < 4) ? (Type)((unsigned int)format + 14) : Type::Invalid;
    }

    static const Type FromGPUPixelFormat(GPU::Regs::PixelFormat format) {
        switch (format) {
        // RGB565 and RGB5A1 are switched in PixelFormat compared to ColorFormat
        case GPU::Regs::PixelFormat::RGB565:
            return Type::RGB565;
        case GPU::Regs::PixelFormat::RGB5A1:
            return Type::RGB5A1;
        default:
            return ((unsigned int)format < 5) ? (Type)format : Type::Invalid;
        }
    }

}; // Format

struct Info {
    PAddr physical_address;
    int width;
    int height;
    int stride;
    Pica::Regs::TextureFormat format;

    static Info FromPicaRegister(const Pica::Regs::TextureConfig& config,
                                 const Pica::Regs::TextureFormat& format);
};

} // Texture

} // Pica
