
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
    Codec(const u8* target, u32 width, u32 height) {
        this->target_buffer = target;
        this->setWidth(width);
        this->setHeight(height);
    }
    virtual ~Codec() {}

    /*
     * Orders the codec to decode the 'target_buffer' with the current
     * configurations.
     */
    virtual void decode();

    /*
     * Orders the codec to encode the 'target_buffer' with the current
     * configurations.
     */
    virtual void encode();

    /*
     * returns the number of bits used by the internal buffer.
     */
    inline u32 getInternalBitsPerPixel() const {
        return this->expected_nibbles_size * 4;
    }

    // Common Passes
    /*
     * Configures the texture tiling.
     * @param active : do the tiling/swizzling pass.
     * @param tiling : texture tiling.
     */
    void configTiling(bool active, u32 tiling);

    /*
     * On Decode:
     *      Transforms the result into RGBA format.
     * On Encode:
     *      Transforms the result into it's original format if the input buffer
     *  was previously converted to RGBA.
     */
    void configRGBATransform(bool active);

    /*
     * On Decode:
     *      Does nothing (nop).
     * On Encode:
     *      Tells the codec that the input buffer was previously decoded.
     */
    void configPreconvertedRGBA(bool active);

    // Allows the use of an external buffer provided by the user for
    // writting the result.
    void setExternalBuffer(u8* external);

    // Obtain the result of decode/encode operations when no external buffer had
    // been set.
    std::unique_ptr<u8[]> transferInternalBuffer();

    // Run validation passes to the current configuration.
    virtual void validate();

    // Check if the codec is in an ivalid state. Do this before decoding/encoding.
    bool invalid() const;

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

    const u8* target_buffer;               // Initial read buffer
    u8* passing_buffer;                    // pointer aliasing: Used and modified by passes
    std::unique_ptr<u8[]> internal_buffer; // used if no external buffer is provided
    bool external_result_buffer = false;

    void init(bool decode);

    typedef Codec super;

    inline void setWidth(u32 width) {
        this->width = width;
    }

    inline void setHeight(u32 height) {
        this->height = height;
    }

    inline void decode_morton_pass() const;
    inline void encode_morton_pass() const;
    u32 getTexel(const u32 x, const u32 y) const;
    const u8* getTile(const u32 x, const u32 y) const;
};

namespace CodecFactory {
std::unique_ptr<Codec> build(Pica::Texture::Format::Type format, u8* target, u32 width, u32 height);
};

} // Texture

} // Pica
