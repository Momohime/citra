#include "common/color.h"
#include "common/common_types.h"
#include "common/vector_math.h"
#include "video_core/texture/internal/codecs.h"
#include "video_core/texture/internal/etc1.h"
#include "video_core/texture/internal/morton.h"
#include "video_core/texture/internal/texture_utils.h"
///////////////////////////////////////////////////////////////////////////////
// Optimizations
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma inline_recursion(on)
#elif defined(CLANG_OR_GCC)
#pragma GCC optimize("-fpeel-loops")
#pragma GCC optimize("-fpredictive-commoning")
#pragma GCC optimize("-ftree-loop-distribute-patterns")
#pragma GCC optimize("-ftree-vectorize")
#endif

// Decoders
#include "decoders.cpp"

// Encoders
#include "encoders.cpp"

// Lookup Texture
#include "lookup.cpp"
