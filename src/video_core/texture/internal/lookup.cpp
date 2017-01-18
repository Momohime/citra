
const Math::Vec4<u8> RGBACodec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    u8* tmp = reinterpret_cast<u8*>(&texel);
    return Color::DecodeRGBA8(tmp);
}

const Math::Vec4<u8> RGBCodec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    u8* tmp = reinterpret_cast<u8*>(&texel);
    return Color::DecodeRGB8(tmp);
}

const Math::Vec4<u8> RGB5A1Codec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    u8* tmp = reinterpret_cast<u8*>(&texel);
    return Color::DecodeRGB5A1(tmp);
}

const Math::Vec4<u8> RGB565Codec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    u8* tmp = reinterpret_cast<u8*>(&texel);
    return Color::DecodeRGB565(tmp);
}

const Math::Vec4<u8> RGBA4Codec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    u8* tmp = reinterpret_cast<u8*>(&texel);
    return Color::DecodeRGBA4(tmp);
}

const Math::Vec4<u8> RG8Codec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    u8* tmp = reinterpret_cast<u8*>(&texel);
    return Color::DecodeRG8(tmp);
}

const Math::Vec4<u8> IA8Codec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    u8 intensity = (texel & 0x00FF00) >> 8;
    u8 alpha = (texel & 0x0000FF);
    Math::Vec4<u8> result(intensity, intensity, intensity, alpha);
    return result;
}

const Math::Vec4<u8> IA4Codec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    u8 intensity = Color::Convert4To8((texel & 0x00F0) >> 4);
    u8 alpha = Color::Convert4To8(texel & 0x0F);
    Math::Vec4<u8> result(intensity, intensity, intensity, alpha);
    return result;
}

const Math::Vec4<u8> I8Codec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    u8 intensity = (texel & 0x0000FF);
    Math::Vec4<u8> result(intensity, intensity, intensity, 255);
    return result;
}

const Math::Vec4<u8> I4Codec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    const u32 bit = 1 - (x % 2);
    u8 intensity = Color::Convert4To8((texel & 0x0000FF) >> bit);
    Math::Vec4<u8> result(intensity, intensity, intensity, 255);
    return result;
}

const Math::Vec4<u8> A8Codec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    u8 alpha = (texel & 0x0000FF);
    Math::Vec4<u8> result(0, 0, 0, alpha);
    return result;
}

const Math::Vec4<u8> A4Codec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    const u32 bit = 1 - (x % 2);
    u8 alpha = Color::Convert4To8((texel & 0x0000FF) >> bit);
    Math::Vec4<u8> result(0, 0, 0, alpha);
    return result;
}

const Math::Vec4<u8> ETC1Codec::lookupTexel(u32 x, u32 y) {
    Math::Vec4<u8> result(0, 0, 0, 255);
    return result;
}

const Math::Vec4<u8> ETC1A4Codec::lookupTexel(u32 x, u32 y) {
    Math::Vec4<u8> result(0, 0, 0, 255);
    return result;
}

const Math::Vec4<u8> D16Codec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    u8* tmp = reinterpret_cast<u8*>(&texel);
    return Color::DecodeRG8(tmp);
}

const Math::Vec4<u8> D24Codec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    u8* tmp = reinterpret_cast<u8*>(&texel);
    return Color::DecodeRGB8(tmp);
}

const Math::Vec4<u8> D24S8Codec::lookupTexel(u32 x, u32 y) {
    u32 texel = super::getTexel(x, y);
    u8* tmp = reinterpret_cast<u8*>(&texel);
    return Color::DecodeRGBA8(tmp);
}
