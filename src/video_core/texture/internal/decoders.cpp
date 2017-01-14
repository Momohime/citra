
namespace {

template <const Math::Vec4<u8> decode_func(const u8*)>
inline void rgba_pass(u8* read, u8* write) {
    u32 pixel = decode_func(read).ToRGBA();
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
    return ((u16)Color::Convert4To8((nibbles & 0xF0) >> 4) << 8) |
           (u16)Color::Convert4To8((nibbles & 0x0F));
}

inline u32 build_luminance(u8 intensity, u8 alpha) {
    return (alpha << 24) | (intensity << 16) | (intensity << 8) | intensity;
}

inline void intensity_alpha_pass(u8* read, u8* write) {
    alignas(4) u8 pixel[2];
    std::memcpy(pixel, read, 2);
    u32 result = build_luminance(pixel[0], pixel[1]);
    std::memcpy(write, &result, 4);
}

inline void intensity_alpha_nibbles_pass(u8* read, u8* write) {
    alignas(4) u8 pixel[2];
    std::memcpy(pixel, read, 1);
    u16 tmp = convert_nibbles(pixel[0]);
    std::memcpy(pixel, &tmp, 2);
    u32 result = build_luminance(pixel[0], pixel[1]);
    std::memcpy(write, &result, 4);
}

inline void intensity_pass(u8* read, u8* write) {
    alignas(4) u8 pixel[1];
    std::memcpy(pixel, read, 1);
    u32 result = build_luminance(pixel[0], 255);
    std::memcpy(write, &result, 4);
}

inline void intensity_nibbles_pass(u8* read, u8* write) {
    alignas(4) u8 pixel[2];
    std::memcpy(pixel, read, 1);
    u16 tmp = convert_nibbles(pixel[0]);
    std::memcpy(pixel, &tmp, 2);
    u32 result = build_luminance(pixel[0], 255);
    std::memcpy(write, &result, 4);
    result = build_luminance(pixel[1], 255);
    std::memcpy(write + 4, &result, 4);
}

inline void alpha_pass(u8* read, u8* write) {
    alignas(4) u8 pixel[1];
    std::memcpy(pixel, read, 1);
    u32 result = build_luminance(0, pixel[0]);
    std::memcpy(write, &result, 4);
}

inline void alpha_nibbles_pass(u8* read, u8* write) {
    alignas(4) u8 pixel[2];
    std::memcpy(pixel, read, 1);
    u16 tmp = convert_nibbles(pixel[0]);
    std::memcpy(pixel, &tmp, 2);
    u32 result = build_luminance(0, pixel[0]);
    std::memcpy(write, &result, 4);
    result = build_luminance(0, pixel[1]);
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

namespace {

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

inline void fix_stencil_pass(u8* read, u8* write) {
    u32 pixel;
    std::memcpy(&pixel, read, 4);
    pixel = (pixel << 8) | (pixel >> 24);
    std::memcpy(write, &pixel, 4);
}

} // Anonymous

void D16Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&expand_depth16_pass, 4, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void D24Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&expand_depth24_pass, 6, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}

void D24S8Codec::decode() {
    super::decode();
    if (this->raw_RGBA)
        image_pass<&fix_stencil_pass, 8, 8, 8>(
            // clang-format off
            this->passing_buffer, this->width, this->height
            // clang-format on
            );
}
