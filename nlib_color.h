/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2019 Naman Dixit
 * SPDX-License-Identifier: 0BSD
 */

#if !defined(NLIB_COLOR_H_INCLUDE_GUARD)

#ifdef LANG_CPP
extern "C" {
#endif

typedef union Color {
    F32 elem[4];
    struct { F32 r, g, b, a; } rgba; // Red, Blue, Green, Alpha
    struct { F32 h, s, v, a; } hsva; // Hue, Saturation, Value (brightness), Alpha
    struct { F32 c, m, y, k; } cmyk; // Cyan, Magenta, Yellow, Black
    alignas(16) Byte alignment[16];
} Color;

header_function
Color colorNew (F32 a, F32 b, F32 c, F32 d)
{
    Color result;
    result.rgba.r = a;
    result.rgba.g = b;
    result.rgba.b = c;
    result.rgba.a = d;

    return result;
}

header_function
U32 colorBGRX32LE(U8 r, U8 g, U8 b)
{
    return ((cast_val(r, U32) << 16U) | (cast_val(g, U32) << 8U) | cast_val(b, U32));
}

#ifdef LANG_CPP
}
#endif

#define NLIB_COLOR_H_INCLUDE_GUARD
#endif
