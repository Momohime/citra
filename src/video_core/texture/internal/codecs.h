
#pragma once

#include <iostream>
#include <memory>
#include "common/common_types.h"
#include "common/vector_math.h"
#include "video_core/texture/codec.h"
#include "video_core/texture/formats.h"

// each texture format codec
class RGBACodec : public Pica::Texture::Codec {
public:
    RGBACodec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::RGBA8
        ) / 4;
        // clang-format on
    };
};

class RGBCodec : public Pica::Texture::Codec {
public:
    RGBCodec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::RGB8
        ) / 4;
        // clang-format on
    };
};

class RGB5A1Codec : public Pica::Texture::Codec {
public:
    RGB5A1Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::RGB5A1
        ) / 4;
        // clang-format on
    };
};

class RGBA4Codec : public Pica::Texture::Codec {
public:
    RGBA4Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::RGBA4
        ) / 4;
        // clang-format on
    };
};

class RGB565Codec : public Pica::Texture::Codec {
public:
    RGB565Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::RGB565
        ) / 4;
        // clang-format on
    };
};

class RG8Codec : public Pica::Texture::Codec {
public:
    RG8Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::RG8
        ) / 4;
        // clang-format on
    };
};

class IA8Codec : public Pica::Texture::Codec {
public:
    IA8Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::IA8
        ) / 4;
        // clang-format on
    };
};

class I8Codec : public Pica::Texture::Codec {
public:
    I8Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::I8
        ) / 4;
        // clang-format on
    };
};

class A8Codec : public Pica::Texture::Codec {
public:
    A8Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::A8
        ) / 4;
        // clang-format on
    };
};

class IA4Codec : public Pica::Texture::Codec {
public:
    IA4Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::IA4
        ) / 4;
        // clang-format on
    };
};

class I4Codec : public Pica::Texture::Codec {
public:
    I4Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::I4
        ) / 4;
        // clang-format on
    };
};

class A4Codec : public Pica::Texture::Codec {
public:
    A4Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::A4
        ) / 4;
        // clang-format on
    };
};

class ETC1Codec : public Pica::Texture::Codec {
public:
    ETC1Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::ETC1
        ) / 4;
        // clang-format on
    };
};

class ETC1A4Codec : public Pica::Texture::Codec {
public:
    ETC1A4Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::ETC1A4
        ) / 4;
        // clang-format on
    };
};

class D16Codec : public Pica::Texture::Codec {
public:
    D16Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::D16
        ) / 4;
        // clang-format on
    };
};

class D24Codec : public Pica::Texture::Codec {
public:
    D24Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::D24
        ) / 4;
        // clang-format on
    };
};

class D24S8Codec : public Pica::Texture::Codec {
public:
    D24S8Codec(u8* target, u32 width, u32 height) : Pica::Texture::Codec(target, width, height) {
        this->setSize();
    }
    void decode();
    void encode();
    const Math::Vec4<u8> lookupTexel(u32 x, u32 y);

protected:
    virtual void setSize() {
        // clang-format off
        this->start_nibbles_size = Pica::Texture::Format::GetBpp(
            Pica::Texture::Format::Type::D24S8
        ) / 4;
        // clang-format on
    };
};
