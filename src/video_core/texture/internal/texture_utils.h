#pragma once

#include <array>
#include <cstring>
#include <memory>
#include <utility>
#include "common/color.h"
#include "common/swap.h"

#if ((defined(__clang__) || defined(__GNUC__)) && !defined(__INTEL_COMPILER))
#define CLANG_OR_GCC
#endif
///////////////////////////////////////////////////////////////////////////////
// Optimizations
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma inline_recursion(on)
#elif defined(CLANG_OR_GCC)
#pragma GCC push_options
#pragma GCC optimize("-fpeel-loops")
#pragma GCC optimize("-fpredictive-commoning")
#pragma GCC optimize("-ftree-loop-distribute-patterns")
#pragma GCC optimize("-ftree-vectorize")
#endif

// Pre: width % 8 == 0 && height % 8 == 0
template <void pass(u8*, u8*), u32 read_size, u32 write_size, u32 tuning = 2>
inline void image_pass_aux_rev(u8* target, u32 width, u32 height) {
    const u32 nibbles = (read_size < 2) & 0x01;
    const u32 pixels = width * height / (1 + nibbles);
    const u32 read_size_amortized = read_size / (2 - nibbles);
    const u32 write_size_amortized = write_size / (2 - nibbles);
    const u32 sub_iters = tuning;
    const u32 iters = pixels / sub_iters;
    u8* read = target + (pixels - 1) * read_size_amortized;
    u8* write = target + (pixels - 1) * write_size_amortized;
    for (u32 i = 0; i < iters; i++) {
        // Sub_iterations allow the compiler to know a set of inner
        // iterations within compile time, thus it can do better optimizations.
        for (u32 k = 0; k < sub_iters; k++) {
            pass(read, write);
            read -= read_size_amortized;
            write -= write_size_amortized;
        }
    }
}

// Pre: width % 8 == 0 && height % 8 == 0
template <void pass(u8*, u8*), u32 read_size, u32 write_size, u32 tuning = 2>
inline void image_pass_aux(u8* target, u32 width, u32 height) {
    const u32 nibbles = (write_size < 2) & 0x01;
    const u32 pixels = width * height / (1 + nibbles);
    const u32 read_size_amortized = read_size / (2 - nibbles);
    const u32 write_size_amortized = write_size / (2 - nibbles);
    const u32 sub_iters = tuning;
    const u32 iters = pixels / sub_iters;
    u8* read = target;
    u8* write = target;
    for (u32 i = 0; i < iters; i++) {
        // Sub_iterations allow the compiler to know a set of inner
        // iterations within compile time, thus it can do better optimizations.
        for (u32 k = 0; k < sub_iters; k++) {
            pass(read, write);
            read += read_size_amortized;
            write += write_size_amortized;
        }
    }
}

template <void pass(u8*, u8*), u32 read_size, u32 write_size, u32 tuning = 2>
inline void image_pass(u8* target, u32 width, u32 height) {
    if (read_size > write_size)
        image_pass_aux<pass, read_size, write_size, tuning>(target, width, height);
    else
        image_pass_aux_rev<pass, read_size, write_size, tuning>(target, width, height);
}

#if defined(CLANG_OR_GCC)
#pragma GCC pop_options
#endif
