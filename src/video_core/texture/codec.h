
#pragma once

#include <iostream>
#include <memory>
#include "common/common_types.h"
#include "common/vector_math.h"
#include "video_core/texture/formats.h"

namespace Pica {

namespace Texture {

class Codec {

public:
    Codec(u8* target, u32 width, u32 height) {
        this->target_buffer = target;
        this->setWidth(width);
        this->setHeight(height);
    }
    virtual ~Codec() {}

    virtual void decode();
    virtual void encode();

    // for legacy code compatibility
    // returns the corresponding texel in RGBA format.
    // prefer full decode/encode than texel lookups for full image decoding.
    virtual const Math::Vec4<u8> lookupTexel(u32 x, u32 y) = 0;

    inline void setWidth(u32 width) {
        this->width = width;
    }

    inline void setHeight(u32 height) {
        this->height = height;
    }

    inline u32 getInternalBytesPerPixel() {
        return this->expected_nibbles_size / 2;
    }

    // Common Passes
    void configTiling(bool active, u32 tiling);
    void configRGBATransform(bool active);
    void configPreConvertedRGBA(bool active);

    void setExternalBuffer(u8* external);
    std::unique_ptr<u8[]> transferInternalBuffer();

    virtual void validate();
    bool invalid();

protected:
    u32 width;
    u32 height;

    // passes
    bool invalid_state = false;
    bool morton = true;
    u32 morton_pass_tiling = 8;
    bool raw_RGBA = false;
    bool preconverted = false;

    u32 start_nibbles_size;
    u32 expected_nibbles_size;

    virtual void setSize() {
        this->start_nibbles_size = 8;
    };

    u8* target_buffer;                     // Initial read buffer
    u8* passing_buffer;                    // pointer aliasing: Used and modified by passes
    std::unique_ptr<u8[]> internal_buffer; // used if no external buffer is provided
    bool external_result_buffer = false;

    void init(bool decode);

    typedef Codec super;

    inline void decode_morton_pass();
    inline void encode_morton_pass();
    u32 getTexel(u32 x, u32 y);
    u8* getTile(u32 x, u32 y);
};

namespace CodecFactory {
std::unique_ptr<Codec> build(Pica::Texture::Format::Type format, u8* target, u32 width, u32 height);
};

} // Texture

} // Pica
