#include <iostream>
#include <memory>
#include "common/common_types.h"
#include "video_core/texture/codec.h"

#pragma once

// each texture format codec
class RGBACodec : public Pica::Texture::Codec {
public:
    RGBACodec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 8;
};

class RGBCodec : public Pica::Texture::Codec {
public:
    RGBCodec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 6;
};

class RGB5A1Codec : public Pica::Texture::Codec {
public:
    RGB5A1Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 4;
};

class RGBA4Codec : public Pica::Texture::Codec {
public:
    RGBA4Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 4;
};

class RGB565Codec : public Pica::Texture::Codec {
public:
    RGB565Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 4;
};

class RG8Codec : public Pica::Texture::Codec {
public:
    RG8Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 4;
};

class IA8Codec : public Pica::Texture::Codec {
public:
    IA8Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 4;
};

class I8Codec : public Pica::Texture::Codec {
public:
    I8Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 2;
};

class A8Codec : public Pica::Texture::Codec {
public:
    A8Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 2;
};

class IA4Codec : public Pica::Texture::Codec {
public:
    IA4Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 2;
};

class I4Codec : public Pica::Texture::Codec {
public:
    I4Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 1;
};

class A4Codec : public Pica::Texture::Codec {
public:
    A4Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 1;
};

class ETC1Codec : public Pica::Texture::Codec {
public:
    ETC1Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 1;
};

class ETC1A4Codec : public Pica::Texture::Codec {
public:
    ETC1A4Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 2;
};

class D16Codec : public Pica::Texture::Codec {
public:
    D16Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 4;
};

class D24Codec : public Pica::Texture::Codec {
public:
    D24Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 6;
};

class D24S8Codec : public Pica::Texture::Codec {
public:
    D24S8Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {}
    void decode();
    void encode();

protected:
    const u32 format_size = 8;
};
