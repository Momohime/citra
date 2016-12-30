
#pragma once

#include "common/common_types.h"

namespace Pica {

namespace Encoders {
/**
 * Encodes textures in raw texel data into z-order/morton-order
 * @param in_buffer pointer to the texture that needs encoding.
 * @param out_buffer pointer to a buffer where the encoded image will be written.
 * @param width texture's width
 * @param width texture's height
 * @param bytespp bytes per pixel
 */
bool Morton(u8* in_buffer, u8* out_buffer, u32 width, u32 height, u32 bytespp);
void MortonU32_U24(u8* in_buffer, u8* out_buffer, u32 width, u32 height);

void Depth(u8* out_buffer, u32 width, u32 height);

} // Encoders

namespace Decoders {

/**
 * Decodes textures using z-order/morton-order into raw texel data
 * @param in_buffer pointer to the texture that needs decoding.
 * @param out_buffer pointer to a buffer where the decoded image will be written.
 * @param width texture's width
 * @param width texture's height
 * @param bytespp bytes per pixel
 */
bool Morton(u8* in_buffer, u8* out_buffer, u32 width, u32 height, u32 bytespp);
void MortonU24_U32(u8* in_buffer, u8* out_buffer, u32 width, u32 height);

void BigEndian(u8* out_buffer, u32 width, u32 height);

void Depth(u8* out_buffer, u32 width, u32 height);

void I8(u8* out_buffer, u32 width, u32 height);
void A8(u8* out_buffer, u32 width, u32 height);
void IA8(u8* out_buffer, u32 width, u32 height);

void Nimbles(u8* in_buffer, u8* out_buffer, u32 width, u32 height);

} // Decoders

} // Pica
