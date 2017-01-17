#include "common/color.h"
#include "common/math_util.h"
#include "common/swap.h"
#include "common/vector_math.h"
#include "video_core/texture/codec.h"
#include "video_core/texture/internal/codecs.h"
#include "video_core/texture/internal/morton.h"

namespace Pica {
namespace Texture {

void Codec::decode() {
    this->init(true);
    if (this->morton)
        this->decode_morton_pass();
}

void Codec::encode() {
    this->init(false);
    if (this->morton)
        this->encode_morton_pass();
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
    this->setSize();
    this->expected_nibbles_size = this->start_nibbles_size;
    if (decode) {
        if (this->raw_RGBA)
            this->expected_nibbles_size = 8;
    } else {
        if (this->preconverted)
            this->start_nibbles_size = 8;
        if (!this->raw_RGBA)
            this->expected_nibbles_size = this->start_nibbles_size;
    }
    this->validate();
    if (!this->external_result_buffer) {
        size_t buff_size = this->width * this->height * this->expected_nibbles_size / 2;
        this->internal_buffer = std::make_unique<u8[]>(buff_size);
        this->passing_buffer = this->internal_buffer.get();
    }
}

void Codec::validate() {
    if (this->width < 8) {
        this->invalid_state = true;
        return;
    }
    if (this->height < 8) {
        this->invalid_state = true;
        return;
    }
    if (this->target_buffer == nullptr) {
        this->invalid_state = true;
        return;
    }
    if (this->external_result_buffer && this->passing_buffer == nullptr) {
        this->invalid_state = true;
        return;
    }
    if (this->morton && this->morton_pass_tiling != 8) {
        this->invalid_state = true;
        return;
    }
    this->invalid_state = false;
}

inline void Codec::decode_morton_pass() {
    if (this->morton_pass_tiling == 8)
        Decoders::Morton_8x8(this->target_buffer, this->passing_buffer, this->width, this->height,
                             this->start_nibbles_size * 4);
}

inline void Codec::encode_morton_pass() {
    if (this->morton_pass_tiling == 8)
        Encoders::Morton_8x8(this->passing_buffer, this->target_buffer, this->width, this->height,
                             this->start_nibbles_size * 4);
}

std::unique_ptr<Codec> CodecFactory::build(Format::Type format, u8* target, u32 width, u32 height) {
    switch (format) {
    case Format::Type::RGBA8:
        return std::make_unique<RGBACodec>(target, width, height);
    case Format::Type::RGB8:
        return std::make_unique<RGBCodec>(target, width, height);
    case Format::Type::RGB5A1:
        return std::make_unique<RGB5A1Codec>(target, width, height);
    case Format::Type::RGB565:
        return std::make_unique<RGB565Codec>(target, width, height);
    case Format::Type::RGBA4:
        return std::make_unique<RGBA4Codec>(target, width, height);
    case Format::Type::RG8:
        return std::make_unique<RG8Codec>(target, width, height);
    case Format::Type::IA8:
        return std::make_unique<IA8Codec>(target, width, height);
    case Format::Type::I8:
        return std::make_unique<I8Codec>(target, width, height);
    case Format::Type::A8:
        return std::make_unique<A8Codec>(target, width, height);
    case Format::Type::IA4:
        return std::make_unique<IA4Codec>(target, width, height);
    case Format::Type::I4:
        return std::make_unique<I4Codec>(target, width, height);
    case Format::Type::A4:
        return std::make_unique<A4Codec>(target, width, height);
    case Format::Type::ETC1:
        return std::make_unique<ETC1Codec>(target, width, height);
    case Format::Type::ETC1A4:
        return std::make_unique<ETC1A4Codec>(target, width, height);
    case Format::Type::D16:
        return std::make_unique<D16Codec>(target, width, height);
    case Format::Type::D24:
        return std::make_unique<D24Codec>(target, width, height);
    case Format::Type::D24S8:
        return std::make_unique<D24S8Codec>(target, width, height);
    default:
        return nullptr;
    }
}

} // Texture
} // Pica
