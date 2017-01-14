
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

inline void fix_stencil_pass(u8* read, u8* write) {
    u32 pixel;
    std::memcpy(&pixel, read, 4);
    pixel = (pixel >> 24) | (pixel << 8);
    std::memcpy(write, &pixel, 4);
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
        image_pass<&Encode::fix_stencil_pass, 8, 8, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}
