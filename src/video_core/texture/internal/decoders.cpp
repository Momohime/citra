

namespace {

template <const Math::Vec4<u8> decode_func(const u8*)>
inline void rgba_pass(u8* read, u8* write) {
    auto pixel = decode_func(read).ToRGBA();
    std::memcpy(write, &pixel, 4);
}

} // Anonymous

void RGBACodec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&rgba_pass<&Color::DecodeRGBA8>, 8, 8, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void RGBCodec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&rgba_pass<&Color::DecodeRGB8>, 6, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void RGB5A1Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&rgba_pass<&Color::DecodeRGB5A1>, 4, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void RGB565Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&rgba_pass<&Color::DecodeRGB565>, 4, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void RGBA4Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&rgba_pass<&Color::DecodeRGBA4>, 4, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void RG8Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&rgba_pass<&Color::DecodeRG8>, 4, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

namespace {

inline u16 convert_nibbles(u8 nibbles) {
    u16 split = (nibbles & 0xF0) << 4 | (nibbles & 0x0F);
    split |= (split << 4);
    return split;
}

inline u32 build_luminance(u32 intensity, u32 alpha) {
    return (alpha << 24) | (intensity << 16) | (intensity << 8) | intensity;
}

inline void intensity_alpha_pass(u8* read, u8* write) {
    u16 pixel;
    std::memcpy(&pixel, read, 2);
    u32 result = build_luminance(pixel >> 8, pixel & 0x00FF);
    std::memcpy(write, &result, 4);
}

inline void intensity_alpha_nibbles_pass(u8* read, u8* write) {
    alignas(4) u8 pixel;
    std::memcpy(&pixel, read, 1);
    u16 tmp = convert_nibbles(pixel);
    u32 result = build_luminance(tmp >> 8, tmp & 0x00FF);
    std::memcpy(write, &result, 4);
}

inline void intensity_pass(u8* read, u8* write) {
    u8 pixel;
    std::memcpy(&pixel, read, 1);
    u32 result = build_luminance(pixel, 255);
    std::memcpy(write, &result, 4);
}

inline void intensity_nibbles_pass(u8* read, u8* write) {
    u8 pixel;
    std::memcpy(&pixel, read, 1);
    u16 tmp = convert_nibbles(pixel);
    u32 result = build_luminance(tmp & 0x00FF, 255);
    std::memcpy(write, &result, 4);
    result = build_luminance(tmp >> 8, 255);
    std::memcpy(write + 4, &result, 4);
}

inline void alpha_pass(u8* read, u8* write) {
    u8 pixel;
    std::memcpy(&pixel, read, 1);
    u32 result = build_luminance(0, pixel);
    std::memcpy(write, &result, 4);
}

inline void alpha_nibbles_pass(u8* read, u8* write) {
    u8 pixel;
    std::memcpy(&pixel, read, 1);
    u16 tmp = convert_nibbles(pixel);
    u32 result = build_luminance(0, tmp & 0x00FF);
    std::memcpy(write, &result, 4);
    result = build_luminance(0, tmp >> 8);
    std::memcpy(write + 4, &result, 4);
}

} // Anonymous

void IA8Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&intensity_alpha_pass, 4, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void IA4Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&intensity_alpha_nibbles_pass, 2, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void I8Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&intensity_pass, 2, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void I4Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&intensity_nibbles_pass, 1, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void A8Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&alpha_pass, 2, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void A4Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&alpha_nibbles_pass, 1, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void ETC1Codec::decode() {
    this->init(true);
    ETC1(this->target_buffer, this->passing_buffer, this->width, this->height);
}

void ETC1A4Codec::decode() {
    this->init(true);
    ETC1A4(this->target_buffer, this->passing_buffer, this->width, this->height);
}

namespace Decode {

inline void expand_depth16_pass(u8* read, u8* write) {
    alignas(4) u8 pixel[4];
    std::memcpy(pixel, read, 2);
    pixel[2] = 255;
    pixel[3] = 255;
    std::memcpy(write, pixel, 4);
}

inline void expand_depth24_pass(u8* read, u8* write) {
    alignas(4) u8 pixel[4];
    std::memcpy(pixel, read, 3);
    pixel[3] = 255;
    std::memcpy(write, pixel, 4);
}

inline void d24s8_pass(u8* target, u32 width, u32 height) {
    const size_t sub_iters = 8;
    const size_t iters = width * height / sub_iters;
    for (u32 i = 0; i < iters; i++) {
        for (u32 j = 0; j < sub_iters; j++) {
            u32 pixel;
            std::memcpy(&pixel, target, 4);
            pixel = (pixel >> 24) | (pixel << 8);
            std::memcpy(target, &pixel, 4);
            target += 4;
        }
    }
}

} // Anonymous

void D16Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&Decode::expand_depth16_pass, 4, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void D24Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&Decode::expand_depth24_pass, 6, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void D24S8Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        Decode::d24s8_pass(this->passing_buffer, this->width, this->height);
}
