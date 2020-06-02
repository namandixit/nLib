/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 * SPDX-License-Identifier: 0BSD OR BSL-1.0 OR Unlicense
 */

#if !defined(NLIB_BITBANG_H_INCLUDE_GUARD)

// NOTE(naman): Bit vectors are supposed to be zero-indexed.
// NOTE(naman): Base type of bit vectors shouldn't have size > 8 (to prevent shift overflow).

# define bitToBytes(b) (((b)+(CHAR_BIT-1))/(CHAR_BIT))

# define bit_I(array, index) ((index)/(CHAR_BIT * sizeof(*(array))))
# define bit_M(array, index) ((index)%(CHAR_BIT * sizeof(*(array))))
# define bitSet(array, index)                                           \
    ((array)[bit_I(array, index)] |= (1LLU << bit_M(array, index)))
# define bitReset(array, index)                                         \
    ((array)[bit_I(array, index)] &= ~(1LLU << bit_M(array, index)))
# define bitToggle(array, index)                                        \
    ((array)[bit_I(array, index)] ^= ~(1LLU << bit_M(array, index)))
# define bitTest(array, index)                                          \
    ((array)[bit_I(array, index)] & (1LLU << bit_M(array, index)))

# define bitFindMSB(x) _Generic((x),                    \
                                U32: bitFindMSBU32,     \
                                U64: bitFindMSBU64,     \
                                )(x)

# define bitFindLSB(x) _Generic((x),                    \
                                U32: bitFindLSBU32,     \
                                U64: bitFindLSBU64,     \
                                )(x)

# if defined(OS_WINDOWS)
/* _BitScanReverse(&r, x) scans for the first 1-bit from left in x. Once it finds it,
 * it returns the number of bits to the right of the found 1-bit.
 */
header_function
U32 bitFindMSBU32 (U32 x)
{
    unsigned long result = 0;
    Byte found = _BitScanReverse(&result, x);
    result = found ? result : 0;
    return result;
}

header_function
U64 bitFindMSBU64 (U64 x)
{
    unsigned long result = 0;
    Byte found = _BitScanReverse64(&result, x);
    result = found ? result : 0;
    return result;
}

/* _BitScanReverse(&r, x) scans for the first 1-bit from right in x. Once it finds it,
 * it returns the number of bits to the right of the found 1-bit.
 */
header_function
U32 bitFindLSBU32 (U32 x)
{
    unsigned long result = 0;
    Byte found = _BitScanForward(&result, x);
    result = found ? result : (U32)-1;
    return result;
}

header_function
U64 bitFindLSBU64 (U64 x)
{
    unsigned long result = 0;
    Byte found = _BitScanForward64(&result, x);
    result = found ? result : (U32)-1;
    return result;
}

# elif defined(OS_LINUX)
/* __builtin_clz(x) returns the number of leading 0-bits in x, starting from
 * most significant position.
 */
header_function
U32 bitFindMSBU32 (U32 x)
{
    if (x == 0) return (U32)-1;
    U32 result = 32U - (U32)__builtin_clz(x) - 1U;
    return result;
}

header_function
U64 bitFindMSBU64 (U64 x)
{
    if (x == 0) return (U64)-1;
    U64 result = 64LLU - (U64)__builtin_clzll(x) - 1LLU;
    return result;
}

/* __builtin_ctz(x) returns the number of trailing 0-bits in x, starting from
 * least significant position.
 */
header_function
U32 bitFindLSBU32 (U32 x)
{
    if (x == 0) return (U32)-1;
    U32 result = (U32)__builtin_ctz(x);
    return result;
}

header_function
U64 bitFindLSBU64 (U64 x)
{
    if (x == 0) return (U64)-1;
    U64 result = (U64)__builtin_ctzll(x);
    return result;
}

# endif

#define NLIB_BITBANG_H_INCLUDE_GUARD
#endif
