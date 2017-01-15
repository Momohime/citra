// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <memory>
#include <set>
#include <tuple>
#include <boost/icl/interval_map.hpp>
#include <glad/glad.h>
#include "common/assert.h"
#include "common/common_funcs.h"
#include "common/common_types.h"
#include "core/hw/gpu.h"
#include "video_core/pica.h"
#include "video_core/renderer_opengl/gl_resource_manager.h"
#include "video_core/texture/formats.h"

namespace MathUtil {
template <class T>
struct Rectangle;
}

struct CachedSurface;

using SurfaceCache = boost::icl::interval_map<PAddr, std::set<std::shared_ptr<CachedSurface>>>;

struct CachedSurface {

    enum class SurfaceType {
        Color = 0,
        Texture = 1,
        Depth = 2,
        DepthStencil = 3,
        Invalid = 4,
    };

    static bool CheckFormatsBlittable(Pica::Texture::Format::Type pixel_format_a,
                                      Pica::Texture::Format::Type pixel_format_b) {
        SurfaceType a_type = GetFormatType(pixel_format_a);
        SurfaceType b_type = GetFormatType(pixel_format_b);

        if ((a_type == SurfaceType::Color || a_type == SurfaceType::Texture) &&
            (b_type == SurfaceType::Color || b_type == SurfaceType::Texture)) {
            return true;
        }

        if (a_type == SurfaceType::Depth && b_type == SurfaceType::Depth) {
            return true;
        }

        if (a_type == SurfaceType::DepthStencil && b_type == SurfaceType::DepthStencil) {
            return true;
        }

        return false;
    }

    static SurfaceType GetFormatType(Pica::Texture::Format::Type pixel_format) {
        if ((unsigned int)pixel_format < 5) {
            return SurfaceType::Color;
        }

        if ((unsigned int)pixel_format < 14) {
            return SurfaceType::Texture;
        }

        if (pixel_format == Pica::Texture::Format::Type::D16 ||
            pixel_format == Pica::Texture::Format::Type::D24) {
            return SurfaceType::Depth;
        }

        if (pixel_format == Pica::Texture::Format::Type::D24S8) {
            return SurfaceType::DepthStencil;
        }

        return SurfaceType::Invalid;
    }

    u32 GetScaledWidth() const {
        return (u32)(width * res_scale_width);
    }

    u32 GetScaledHeight() const {
        return (u32)(height * res_scale_height);
    }

    PAddr addr;
    u32 size;

    PAddr min_valid;
    PAddr max_valid;

    OGLTexture texture;
    u32 width;
    u32 height;
    /// Stride between lines, in pixels. Only valid for images in linear format.
    u32 pixel_stride = 0;
    float res_scale_width = 1.f;
    float res_scale_height = 1.f;

    bool is_tiled;
    Pica::Texture::Format::Type pixel_format;
    bool dirty;
};

class RasterizerCacheOpenGL : NonCopyable {
public:
    RasterizerCacheOpenGL();
    ~RasterizerCacheOpenGL();

    /// Blits one texture to another
    void BlitTextures(GLuint src_tex, GLuint dst_tex, CachedSurface::SurfaceType type,
                      const MathUtil::Rectangle<int>& src_rect,
                      const MathUtil::Rectangle<int>& dst_rect);

    /// Attempt to blit one surface's texture to another
    bool TryBlitSurfaces(CachedSurface* src_surface, const MathUtil::Rectangle<int>& src_rect,
                         CachedSurface* dst_surface, const MathUtil::Rectangle<int>& dst_rect);

    /// Loads a texture from 3DS memory to OpenGL and caches it (if not already cached)
    CachedSurface* GetSurface(const CachedSurface& params, bool match_res_scale,
                              bool load_if_create);

    /// Attempt to find a subrect (resolution scaled) of a surface, otherwise loads a texture from
    /// 3DS memory to OpenGL and caches it (if not already cached)
    CachedSurface* GetSurfaceRect(const CachedSurface& params, bool match_res_scale,
                                  bool load_if_create, MathUtil::Rectangle<int>& out_rect);

    /// Gets a surface based on the texture configuration
    CachedSurface* GetTextureSurface(const Pica::Regs::FullTextureConfig& config);

    /// Gets the color and depth surfaces and rect (resolution scaled) based on the framebuffer
    /// configuration
    std::tuple<CachedSurface*, CachedSurface*, MathUtil::Rectangle<int>> GetFramebufferSurfaces(
        const Pica::Regs::FramebufferConfig& config);

    /// Attempt to get a surface that exactly matches the fill region and format
    CachedSurface* TryGetFillSurface(const GPU::Regs::MemoryFillConfig& config);

    /// Write the surface back to memory
    void FlushSurface(CachedSurface* surface);

    /// Write any cached resources overlapping the region back to memory (if dirty) and optionally
    /// invalidate them in the cache
    void FlushRegion(PAddr addr, u32 size, const CachedSurface* skip_surface, bool invalidate);

    /// Flush all cached resources tracked by this cache manager
    void FlushAll();

private:
    SurfaceCache surface_cache;
    OGLFramebuffer transfer_framebuffers[2];
};
