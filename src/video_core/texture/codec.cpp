#include "codec.h"
#include "internal\codecs.h"
#include "internal\morton.h"

namespace Pica {
namespace Texture {

void Codec::decode() {
    this->init(true);
    if (this->morton)
        this->decode_morton_pass();
};

void Codec::encode() {
    this->init(false);
    if (this->morton)
        this->encode_morton_pass();
};

void Codec::setSize() {
    this->start_nibbles_size = format_size;
};

inline void Codec::setWidth(u32 width) {
    this->width = width;
}

inline void Codec::setHeight(u32 height) {
    this->height = height;
}

void Codec::configTiling(bool active, u32 tiling) {
    this->morton = true;
    this->morton_pass_tiling = tiling;
    if (tiling != 8 && tiling != 32) {
        this->invalid_state = true;
    }
}

void Codec::configRGBATransform(bool active) {
    this->raw_RGBA = active;
}

void Codec::configPreConvertedRGBA(bool active) {
    this->preconverted = active;
}

void Codec::setExternalBuffer(u8* external) {
    this->external_result_buffer = true;
    this->passing_buffer = external;
}

std::unique_ptr<u8[]> Codec::transferInternalBuffer() {
    if (!this->external_result_buffer) {
        std::unique_ptr<u8[]> result(std::move(this->internal_buffer));
        return result;
    }
    return nullptr;
}

bool Codec::invalid() {
    return this->invalid_state;
}

void Codec::init(bool decode) {
    if (decode) {
        if (this->raw_RGBA)
            this->expected_nibbles_size = 8;
    } else {
        this->start_nibbles_size = this->format_size;
        if (this->raw_RGBA)
            this->expected_nibbles_size = this->format_size;
        if (this->preconverted)
            this->start_nibbles_size = 8;
    }
    if (!this->external_result_buffer) {
        size_t buff_size = this->width * this->height * this->expected_nibbles_size / 2;
        this->internal_buffer = std::make_unique<u8[]>(buff_size);
        this->passing_buffer = this->internal_buffer.get();
    }
}

inline void Codec::decode_morton_pass() {
    if (this->morton_pass_tiling == 8)
        Decoders::Morton_8x8(this->target_buffer, this->passing_buffer, this->width, this->height,
                             this->start_nibbles_size * 4);
    else if (this->morton_pass_tiling == 32)
        Decoders::Morton_32x32(this->target_buffer, this->passing_buffer, this->width, this->height,
                               this->start_nibbles_size * 4);
}

inline void Codec::encode_morton_pass() {
    if (this->morton_pass_tiling == 8)
        Encoders::Morton_8x8(this->target_buffer, this->passing_buffer, this->width, this->height,
                             this->start_nibbles_size * 4);
    else if (this->morton_pass_tiling == 32)
        Encoders::Morton_32x32(this->target_buffer, this->passing_buffer, this->width, this->height,
                               this->start_nibbles_size * 4);
}

std::unique_ptr<Codec> CodecFactory::build(Format format, u8* target, u32 width, u32 height) {
    switch (format) {
    case Format::RGBA8:
        return std::make_unique<RGBACodec>(target, width, height);
    case Format::RGB8:
        return std::make_unique<RGBCodec>(target, width, height);
    case Format::RGB5A1:
        return std::make_unique<RGB5A1Codec>(target, width, height);
    case Format::RGB565:
        return std::make_unique<RGB565Codec>(target, width, height);
    case Format::RGBA4:
        return std::make_unique<RGBA4Codec>(target, width, height);
    case Format::RG8:
        return std::make_unique<RG8Codec>(target, width, height);
    case Format::IA8:
        return std::make_unique<IA8Codec>(target, width, height);
    case Format::I8:
        return std::make_unique<I8Codec>(target, width, height);
    case Format::A8:
        return std::make_unique<A8Codec>(target, width, height);
    case Format::IA4:
        return std::make_unique<IA4Codec>(target, width, height);
    case Format::I4:
        return std::make_unique<I4Codec>(target, width, height);
    case Format::A4:
        return std::make_unique<A4Codec>(target, width, height);
    case Format::ETC1:
        return std::make_unique<ETC1Codec>(target, width, height);
    case Format::ETC1A4:
        return std::make_unique<ETC1A4Codec>(target, width, height);
    case Format::D16:
        return std::make_unique<D16Codec>(target, width, height);
    case Format::D24:
        return std::make_unique<D24Codec>(target, width, height);
    case Format::D24S8:
        return std::make_unique<D24S8Codec>(target, width, height);
    default:
        return nullptr;
    }
}

} // Texture
} // Pica
