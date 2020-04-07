/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2019 Naman Dixit
 */

#if !defined(NLIB_COLOR_H_INCLUDE_GUARD)

typedef union {
    F32 elem[4];
    struct { F32 r, g, b, a; } rgba; // Red, Blue, Green, Alpha
    struct { F32 h, s, v, a; } hsva; // Hue, Saturation, Value (brightness), Alpha
    struct { F32 c, m, y, k; } cmyk; // Cyan, Magenta, Yellow, Black
    alignas(16) Byte alignment[16];
} Color;

header_function
Color colorNew (F32 a, F32 b, F32 c, F32 d)
{
    Color result = (Color){.rgba = {a, b, c, d}};
    return result;
}

header_function
U32 colorBGRX32LE(U8 r, U8 g, U8 b)
{
    return (((U32)r << 16U) | ((U32)g << 8U) | (U32)b);
}

#define NLIB_COLOR_H_INCLUDE_GUARD
#endif
