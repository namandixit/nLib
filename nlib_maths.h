/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2019 Naman Dixit
 */

#if !defined(NLIB_MATHS_H_INCLUDE_GUARD)

# if !defined(NLIB_NO_LIBC)
#  include <math.h>
# else
// TODO(naman): Add some alternative to math.h
#  include <math.h>
# endif
/* Wrapper for needed math.h functions */

# if defined(LANGUAGE_C11)
#  define mathGeneric(...) _Generic(...)
#  else
#  define mathGeneric(...) claim(false && "Generic function only supported in C11")
# endif

# define MATH_PI 3.14159265358979323846

# define mathSqrt(x) mathGeneric((x),                   \
                                 F32 : mathSqrtF32      \
                                 F64 : mathSqrtF64      \
        )(x)

header_function F32 mathSqrtF32 (F32 x) { F32 y = sqrtf(x); return y; }
header_function F64 mathSqrtF64 (F64 x) { F64 y = sqrt(x);  return y; }

# define mathPow2(x) mathGeneric((x),                    \
                                F32 : mathPow2F32,      \
                                F64 : mathPow2F64,      \
                                U32 : mathPow2U32       \
                                U64 : mathPow2U64       \
        )(x)

header_function F32 mathPow2F32 (F32 x) { F32 y = exp2f(x); return y; }
header_function F64 mathPow2F64 (F64 x) { F64 y = exp2(x);  return y; }
header_function U32 mathPow2U32 (U32 x) { U32 y = 1 << x;   return y; }
header_function U64 mathPow2U64 (U64 x) { U64 y = 1 << x;   return y; }

# define mathLog2(x) mathGeneric((x),                    \
                                F32 : mathLog2F32,      \
                                F64 : mathLog2F64,      \
                                U32:  mathLog2U32,      \
                                U64:  mathLog2U64       \
        )(x)

header_function F32 mathLog2F32 (F32 x) { F32 y = log2f(x); return y; }
header_function F64 mathLog2F64 (F64 x) { F64 y = log2(x);  return y; }
header_function U32 mathLog2U32 (U32 x) { U32 y = x ? bitFindMSBU32(x) : 0; return y; }
header_function U64 mathLog2U64 (U64 x) { U64 y = x ? bitFindMSBU64(x) : 0; return y; }

# define mathIsPowerOf2(x) mathGeneric((x),                      \
                                      U32 : mathIsPowerOf2U32   \
                                      U64 : mathIsPowerOf2U64   \
        )(x)

header_function B32 mathIsPowerOf2U32 (U32 x) { B32 b = (x & (x - 1)) == 0; return b; }
header_function B64 mathIsPowerOf2U64 (U64 x) { B64 b = (x & (x - 1)) == 0; return b; }

# define mathNextPowerOf2(x) mathGeneric((x),                    \
                                        U32 : mathNextPowerOf2U32 \
                                        U64 : mathNextPowerOf2U64 \
        )(x)


header_function U32 mathNextPowerOf2U32 (U32 x) { U32 y = mathIsPowerOf2U32(x) ? x : (1U << (mathLog2U32(x) + 1U)); return y; }
header_function U64 mathNextPowerOf2U64 (U64 x) { U64 y = mathIsPowerOf2U64(x) ? x : (1LLU << (mathLog2U64(x) + 1LLU)); return y; }

# define mathPrevPowerOf2(x) mathGeneric((x),                    \
                                        U32 : mathPrevPowerOf2U32 \
                                        U64 : mathPrevPowerOf2U64 \
        )(x)


header_function U32 mathPrevPowerOf2U32 (U32 x) { U32 y = mathIsPowerOf2U32(x) ? (1U << (mathLog2U32(x) - 1U)) : x; return y; }
header_function U64 mathPrevPowerOf2U64 (U64 x) { U64 y = mathIsPowerOf2U64(x) ? (1LLU << (mathLog2U64(x) - 1LLU)) : x; return y; }

# define mathSin(x) mathGeneric((x),             \
                               F32 : mathSinF32 \
                               F64 : mathSinF64 \
        )(x)

header_function F32 mathSinF32 (F32 x) { F32 y = sinf(x); return y; }
header_function F64 mathSinF64 (F64 x) { F64 y = sin(x);  return y; }

# define mathCos(x) mathGeneric((x),             \
                               F32 : mathCosF32 \
                               F64 : mathCosF64 \
        )(x)

header_function F32 mathCosF32 (F32 x) { F32 y = cosf(x); return y; }
header_function F64 mathCosF64 (F64 x) { F64 y = cos(x);  return y; }

# define mathTan(x) mathGeneric((x),             \
                               F32 : mathTanF32 \
                               F64 : mathTanF64 \
        )(x)

header_function F32 mathTanF32 (F32 x) { F32 y = tanf(x); return y; }
header_function F64 mathTanF64 (F64 x) { F64 y = tan(x);  return y; }

# define mathRadians(x) mathGeneric((x),                 \
                                   F32 : mathRadiansF64 \
                                   F64 : mathRadiansF64 \
        )(x)

header_function F32 mathRadiansF32 (F32 degrees) { F32 radians = (degrees * (F32)MATH_PI) / 180.0f; return radians; }
header_function F64 mathRadiansF64 (F64 degrees) { F64 radians = (degrees * MATH_PI)      / 180.0;  return radians; }

# define mathDegrees(x) mathGeneric((x),                 \
                                   F32 : mathDegreesF32 \
                                   F64 : mathDegreesF64 \
        )(x)

header_function F32 mathDegreesF32 (F32 radians) { F32 degrees = (radians * 180.0f) / (F32)MATH_PI; return degrees; }
header_function F64 mathDegreesF64 (F64 radians) { F64 degrees = (radians * 180.0) / MATH_PI;       return degrees; }

# define NLIB_MATHS_H_INCLUDE_GUARD
#endif
