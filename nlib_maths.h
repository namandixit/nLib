/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2019 Naman Dixit
 */

#if !defined(NLIB_MATHS_H_INCLUDE_GUARD)

/* Wrapper for needed math.h functions */

#define MATH_PI 3.14159265358979323846f

// Square Root ===============================================================

#if defined(LANGAUGE_C11)
# define mathSqrt(x) _Generic((x),                      \
                              F32 : mathSqrt##F32       \
        )(x)
#endif

header_function
F32 mathSqrtF32 (F32 x)
{
    F32 y = sqrtf(x);
    return y;
}

// Power of 2 ================================================================

#if defined(LANGAUGE_C11)
# define mathPow2(x) _Generic((x),                      \
                              F32 : mathPow2##F32,      \
                              U32 : mathPow2##U32       \
        )(x)
#endif

header_function
F32 mathPow2F32 (F32 x)
{
    F32 y = exp2f(x);
    return y;
}

header_function
U32 mathPow2U32 (U32 x)
{
    U32 y = 1 << x;
    return y;
}

// Logarithm with base 2 =====================================================

#if defined(LANGAUGE_C11)
# define mathLog2(x) _Generic((x),                      \
                              F32 : mathLog2##F32,      \
                              U32: mathLog2##U32,       \
                              U64: mathLog2##U64        \
        )(x)
#endif

header_function
F32 mathLog2F32 (F32 x)
{
    F32 y = log2f(x);
    return y;
}

# if defined(OS_WINDOWS)
/* _BitScanReverse(&r, x) scans for the first 1-bit from left in x. Once it finds it,
 * it returns the number of bits after the found 1-bit.
 *
 * If b is the bit-width of the number,
 *    p is the log2 of the closest lower power of two and
 *    r is the number of bits to the right of the first 1-bit when seen from left;
 * then a number between 2^p and 2^(p+1) has the form: (b-p-1 0-bits) 1 (p bits)
 *
 * => r = p
 *
 * Thus, the rounded-down log2 of the number is r.
 */

header_function
U32 mathLog2U32(U32 x)
{
    unsigned long result = 0;
    _BitScanReverse(&result, x);
    return result;
}

header_function
U64 mathLog2U64(U64 x)
{
    unsigned long result = 0;
    _BitScanReverse64(&result, x);
    return result;
}
# elif defined(OS_LINUX)
/* __builtin_clzll(x) returns the leading number of 0-bits in x, starting from
 * most significant position.
 *
 * If b is the bit-width of the number,
 *    p is the closest lower power of two and
 *    lz is the number of leading 0-bits; then
 * then a number between 2^p and 2^(p+1) has the form: (b-p-1 0-bits) 1 (p bits)
 *
 * => lz = b-p-1
 * => p = b-(lz+1)
 *
 * Thus, the rounded-down log2 of the number is b-(lz+1).
 */

header_function
U32 mathLog2U32(U32 x)
{
    U32 result = (U32)(32 - ((U64)__builtin_clz(x) + 1));
    return result;
}

header_function
U64 mathLog2U64(U64 x)
{
    U64 result = 64ULL - ((U64)__builtin_clzll(x) + 1ULL);
    return result;
}
#endif

// Sine of an angle in radians ===============================================

#if defined(LANGAUGE_C11)
# define mathSin(x) _Generic((x),               \
                             F32 : mathSin##F32 \
        )(x)
#endif

header_function
F32 mathSinF32 (F32 x)
{
    F32 y = sinf(x);
    return y;
}

// Cosine of an angle in radians =============================================

#if defined(LANGAUGE_C11)
# define mathCos(x) _Generic((x),               \
                             F32 : mathCos##F32 \
        )(x)
#endif

header_function
F32 mathCosF32 (F32 x)
{
    F32 y = cosf(x);
    return y;
}

// Tangent of an angle in radians ============================================

#if defined(LANGAUGE_C11)
# define mathTan(x) _Generic((x),               \
                             F32 : mathTan##F32 \
        )(x)
#endif

header_function
F32 mathTanF32 (F32 x)
{
    F32 y = tanf(x);
    return y;
}

// Radians from degrees ======================================================
#if defined(LANGAUGE_C11)
# define mathRadians(x) _Generic((x),                   \
                                 F32 : mathRadians##F32 \
        )(x)
#endif

header_function
F32 mathRadiansF32 (F32 degrees)
{
    F32 radians = (degrees * MATH_PI) / 180.0f;
    return radians;
}

// Degrees from radians ======================================================

#if defined(LANGAUGE_C11)
# define mathDegrees(x) _Generic((x),                   \
                                 F32 : mathDegrees##F32 \
        )(x)
#endif

header_function
F32 mathDegreesF32 (F32 radians)
{
    F32 degrees = (radians * 180.0f) / MATH_PI;
    return degrees;
}

#define NLIB_MATHS_H_INCLUDE_GUARD
#endif
