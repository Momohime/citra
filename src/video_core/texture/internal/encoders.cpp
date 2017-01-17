
void RGBACodec::encode() {
    super::encode();
}

void RGBCodec::encode() {
    super::encode();
}

void RGB5A1Codec::encode() {
    super::encode();
}

void RGB565Codec::encode() {
    super::encode();
}

void RGBA4Codec::encode() {
    super::encode();
}

void RG8Codec::encode() {
    super::encode();
}

void IA8Codec::encode() {
    super::encode();
}

void IA4Codec::encode() {
    super::encode();
}

void I8Codec::encode() {
    super::encode();
}

void I4Codec::encode() {
    super::encode();
}

void A8Codec::encode() {
    super::encode();
}

void A4Codec::encode() {
    super::encode();
}

void ETC1Codec::encode() {
    super::encode();
}

void ETC1A4Codec::encode() {
    super::encode();
}

namespace Encode {

inline void contract_depth16_pass(u8* read, u8* write) {
    alignas(4) u8 pixel[4];
    std::memcpy(pixel, read, 4);
    std::memcpy(write, pixel, 2);
}

inline void contract_depth24_pass(u8* read, u8* write) {
    alignas(4) u8 pixel[4];
    std::memcpy(pixel, read, 4);
    std::memcpy(write, pixel, 3);
}

inline void d24s8_pass(u8* target, u32 width, u32 height) {
    const size_t sub_iters = 8;
    const size_t iters = width * height / sub_iters;
    for (u32 i = 0; i < iters; i++) {
        for (u32 j = 0; j < sub_iters; j++) {
            u32 pixel;
            std::memcpy(&pixel, target, 4);
            pixel = (pixel >> 8) | (pixel << 24);
            std::memcpy(target, &pixel, 4);
            target += 4;
        }
    }
}

} // Anonymous

void D16Codec::encode() {
    super::encode();
    if (this->raw_RGBA)
        image_pass<&Encode::contract_depth16_pass, 8, 4, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void D24Codec::encode() {
    super::encode();
    if (this->raw_RGBA)
        image_pass<&Encode::contract_depth24_pass, 8, 6>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void D24S8Codec::encode() {
    super::encode();
    if (this->raw_RGBA)
        Encode::d24s8_pass(this->passing_buffer, this->width, this->height);
}
