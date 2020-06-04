// Original work Copyright 2018 Ulf Adams
// Modified work Copyright 2020 Naman Dixit
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.

/* NOTE(naman): This file is an amalgamation (and subsequent modification) of:
 * ryu/ryu.h
 * ryu/common.h
 * ryu/d2s_intrinsics.h
 */

#if !defined(NLIB_RYU_H_INCLUDE_GUARD)

# include "nlib_print_ryu_tables.h"

// Returns the number of decimal digits in v, which must not contain more than 9 digits.
header_function
U32 ryu_decimalLength9 (U32 v) {
    // Function precondition: v is not a 10-digit number.
    // (f2s: 9 digits are sufficient for round-tripping.)
    // (d2fixed: We print 9-digit blocks.)
    claim(v < 1000000000);
    if (v >= 100000000) { return 9; }
    if (v >= 10000000) { return 8; }
    if (v >= 1000000) { return 7; }
    if (v >= 100000) { return 6; }
    if (v >= 10000) { return 5; }
    if (v >= 1000) { return 4; }
    if (v >= 100) { return 3; }
    if (v >= 10) { return 2; }
    return 1;
}

// Returns e == 0 ? 1 : [log_2(5^e)]; requires 0 <= e <= 3528.
header_function
S32 ryu_log2pow5 (S32 e) {
    // This approximation works up to the point that the multiplication overflows at e = 3529.
    // If the multiplication were done in 64 bits, it would fail at 5^4004 which is just greater
    // than 2^9297.
    claim(e >= 0);
    claim(e <= 3528);
    return (S32) ((((U32) e) * 1217359) >> 19);
}

// Returns e == 0 ? 1 : ceil(log_2(5^e)); requires 0 <= e <= 3528.
header_function
S32 ryu_pow5bits (S32 e) {
    // This approximation works up to the point that the multiplication overflows at e = 3529.
    // If the multiplication were done in 64 bits, it would fail at 5^4004 which is just greater
    // than 2^9297.
    claim(e >= 0);
    claim(e <= 3528);
    return (S32) (((((U32) e) * 1217359) >> 19) + 1);
}

// Returns e == 0 ? 1 : ceil(log_2(5^e)); requires 0 <= e <= 3528.
header_function
S32 ryu_ceil_log2pow5 (S32 e) {
    return ryu_log2pow5(e) + 1;
}

// Returns floor(log_10(2^e)); requires 0 <= e <= 1650.
header_function
U32 ryu_log10Pow2 (S32 e) {
    // The first value this approximation fails for is 2^1651 which is just greater than 10^297.
    claim(e >= 0);
    claim(e <= 1650);
    return (((U32) e) * 78913) >> 18;
}

// Returns floor(log_10(5^e)); requires 0 <= e <= 2620.
header_function
U32 ryu_log10Pow5 (S32 e) {
    // The first value this approximation fails for is 5^2621 which is just greater than 10^1832.
    claim(e >= 0);
    claim(e <= 2620);
    return (((U32) e) * 732923) >> 20;
}

header_function
U64 ryu_F64_to_U64 (F64 d) {
    union {F64 f; U64 u; } fu;
    fu.f = d;
    U64 bits = fu.u;
    return bits;
}

// ABSL avoids uint128_t on Win32 even if __SIZEOF_INT128__ is defined.
// Let's do the same for now.
#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER) && !defined(NLIB_RYU_NO_INTRINSICS)
#define RYU_HAS_UINT128_TYPE
#elif defined(_MSC_VER) && !defined(NLIB_RYU_NO_INTRINSICS) && defined(_M_X64)
#define RYU_HAS_64_BIT_MSVC_INTRINSICS
#endif

#if defined(RYU_HAS_UINT128_TYPE)
typedef __uint128_t Ryu_U128;
#endif

#if defined(RYU_HAS_64_BIT_MSVC_INTRINSICS)

header_function
U64 ryu_umul128 (U64 a, U64 b, U64* productHi) {
    return _umul128(a, b, productHi);
}

header_function
U64 ryu_shiftright128 (U64 lo, U64 hi, U32 dist) {
    // For the __shiftright128 intrinsic, the shift value is always
    // modulo 64.
    // In the current implementation of the F64-precision version
    // of Ryu, the shift value is always < 64. (In the case
    // RYU_OPTIMIZE_SIZE == 0, the shift value is in the range [49, 58].
    // Otherwise in the range [2, 59].)
    // Check this here in case a future change requires larger shift
    // values. In this case this function needs to be adjusted.
    claim(dist < 64);
    return __shiftright128(lo, hi, (Byte) dist);
}

#else // defined(RYU_HAS_64_BIT_MSVC_INTRINSICS)

header_function
U64 ryu_umul128 (U64 a, U64 b, U64* productHi) {
    // The casts here help MSVC to avoid calls to the __allmul library function.
    U32 aLo = (U32)a;
    U32 aHi = (U32)(a >> 32);
    U32 bLo = (U32)b;
    U32 bHi = (U32)(b >> 32);

    U64 b00 = (U64)aLo * bLo;
    U64 b01 = (U64)aLo * bHi;
    U64 b10 = (U64)aHi * bLo;
    U64 b11 = (U64)aHi * bHi;

    U32 b00Lo = (U32)b00;
    U32 b00Hi = (U32)(b00 >> 32);

    U64 mid1 = b10 + b00Hi;
    U32 mid1Lo = (U32)(mid1);
    U32 mid1Hi = (U32)(mid1 >> 32);

    U64 mid2 = b01 + mid1Lo;
    U32 mid2Lo = (U32)(mid2);
    U32 mid2Hi = (U32)(mid2 >> 32);

    U64 pHi = b11 + mid1Hi + mid2Hi;
    U64 pLo = ((U64)mid2Lo << 32) | b00Lo;

    *productHi = pHi;
    return pLo;
}

header_function
U64 ryu_shiftright128 (U64 lo, U64 hi, U32 dist) {
    // We don't need to handle the case dist >= 64 here (see above).
    claim(dist < 64);
#if defined(RYU_OPTIMIZE_SIZE) || !defined(BITWIDTH_32)
    claim(dist > 0);
    return (hi << (64 - dist)) | (lo >> dist);
#else
    // Avoid a 64-bit shift by taking advantage of the range of shift values.
    claim(dist >= 32);
    return (hi << (64 - dist)) | ((U32)(lo >> 32) >> (dist - 32));
#endif
}

#endif // defined(RYU_HAS_64_BIT_MSVC_INTRINSICS)

#if defined(BITWIDTH_32)

// Returns the high 64 bits of the 128-bit product of a and b.
header_function
U64 ryu_umulh (U64 a, U64 b) {
    // Reuse the ryu_umul128 implementation.
    // Optimizers will likely eliminate the instructions used to compute the
    // low part of the product.
    U64 hi;
    ryu_umul128(a, b, &hi);
    return hi;
}

// On 32-bit platforms, compilers typically generate calls to library
// functions for 64-bit divisions, even if the divisor is a constant.
//
// E.g.:
// https://bugs.llvm.org/show_bug.cgi?id=37932
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=17958
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=37443
//
// The functions here perform division-by-constant using multiplications
// in the same way as 64-bit compilers would do.
//
// NB:
// The multipliers and shift values are the ones generated by clang x64
// for expressions like x/5, x/10, etc.

header_function
U64 ryu_div5 (U64 x) {
    return ryu_umulh(x, 0xCCCCCCCCCCCCCCCDu) >> 2;
}

header_function
U64 ryu_div10 (U64 x) {
    return ryu_umulh(x, 0xCCCCCCCCCCCCCCCDu) >> 3;
}

header_function
U64 ryu_div100 (U64 x) {
    return ryu_umulh(x >> 2, 0x28F5C28F5C28F5C3u) >> 2;
}

header_function
U64 ryu_div1e8 (U64 x) {
    return ryu_umulh(x, 0xABCC77118461CEFDu) >> 26;
}

header_function
U64 ryu_div1e9 (U64 x) {
    return ryu_umulh(x >> 9, 0x44B82FA09B5A53u) >> 11;
}

header_function
U32 ryu_mod1e9 (U64 x) {
    // Avoid 64-bit math as much as possible.
    // Returning (U32) (x - 1000000000 * ryu_div1e9(x)) would
    // perform 32x64-bit multiplication and 64-bit subtraction.
    // x and 1000000000 * ryu_div1e9(x) are guaranteed to differ by
    // less than 10^9, so their highest 32 bits must be identical,
    // so we can truncate both sides to U32 before subtracting.
    // We can also simplify (U32) (1000000000 * ryu_div1e9(x)).
    // We can truncate before multiplying instead of after, as multiplying
    // the highest 32 bits of ryu_div1e9(x) can't affect the lowest 32 bits.
    return ((U32) x) - 1000000000 * ((U32) ryu_div1e9(x));
}

#else // defined(BITWIDTH_32)

header_function
U64 ryu_div5(U64 x) {
    return x / 5;
}

header_function
U64 ryu_div10(U64 x) {
    return x / 10;
}

header_function
U64 ryu_div100(U64 x) {
    return x / 100;
}

header_function
U64 ryu_div1e8(U64 x) {
    return x / 100000000;
}

header_function
U64 ryu_div1e9(U64 x) {
    return x / 1000000000;
}

header_function
U32 ryu_mod1e9(U64 x) {
    return (U32) (x - 1000000000 * ryu_div1e9(x));
}

#endif // defined(BITWIDTH_32)

header_function
U32 ryu_pow5Factor (U64 value) {
    U32 count = 0;
    for (;;) {
        claim(value != 0);
        U64 q = ryu_div5(value);
        U32 r = ((U32) value) - 5 * ((U32) q);
        if (r != 0) {
            break;
        }
        value = q;
        ++count;
    }
    return count;
}

// Returns true if value is divisible by 5^p.
header_function
B32 ryu_multipleOfPowerOf5 (U64 value, U32 p) {
    // I tried a case distinction on p, but there was no performance difference.
    return ryu_pow5Factor(value) >= p;
}

// Returns true if value is divisible by 2^p.
header_function
B32 ryu_multipleOfPowerOf2 (U64 value, U32 p) {
    claim(value != 0);
    claim(p < 64);
    // __builtin_ctzll doesn't appear to be faster here.
    return (value & ((1ull << p) - 1)) == 0;
}

// We need a 64x128-bit multiplication and a subsequent 128-bit shift.
// Multiplication:
//   The 64-bit factor is variable and passed in, the 128-bit factor comes
//   from a lookup table. We know that the 64-bit factor only has 55
//   significant bits (i.e., the 9 topmost bits are zeros). The 128-bit
//   factor only has 124 significant bits (i.e., the 4 topmost bits are
//   zeros).
// Shift:
//   In principle, the multiplication result requires 55 + 124 = 179 bits to
//   represent. However, we then shift this value to the right by j, which is
//   at least j >= 115, so the result is guaranteed to fit into 179 - 115 = 64
//   bits. This means that we only need the topmost 64 significant bits of
//   the 64x128-bit multiplication.
//
// There are several ways to do this:
// 1. Best case: the compiler exposes a 128-bit type.
//    We perform two 64x64-bit multiplications, add the higher 64 bits of the
//    lower result to the higher result, and shift by j - 64 bits.
//
//    We explicitly cast from 64-bit to 128-bit, so the compiler can tell
//    that these are only 64-bit inputs, and can map these to the best
//    possible sequence of assembly instructions.
//    x64 machines happen to have matching assembly instructions for
//    64x64-bit multiplications and 128-bit shifts.
//
// 2. Second best case: the compiler exposes intrinsics for the x64 assembly
//    instructions mentioned in 1.
//
// 3. We only have 64x64 bit instructions that return the lower 64 bits of
//    the result, i.e., we have to use plain C.
//    Our inputs are less than the full width, so we have three options:
//    a. Ignore this fact and just implement the intrinsics manually.
//    b. Split both into 31-bit pieces, which guarantees no internal overflow,
//       but requires extra work upfront (unless we change the lookup table).
//    c. Split only the first factor into 31-bit pieces, which also guarantees
//       no internal overflow, but requires extra work since the intermediate
//       results are not perfectly aligned.
#if defined(RYU_HAS_UINT128_TYPE)

// Best case: use 128-bit type.
header_function
U64 ryu_mulShift64 (U64 m, U64* mul, S32 j) {
    Ryu_U128 b0 = ((Ryu_U128) m) * mul[0];
    Ryu_U128 b2 = ((Ryu_U128) m) * mul[1];
    return (U64) (((b0 >> 64) + b2) >> (j - 64));
}

header_function
U64 ryu_mulShiftAll64 (U64 m, U64* mul, S32 j,
                       U64* vp, U64* vm, U32 mmShift) {
//  m <<= 2;
//  Ryu_U128 b0 = ((Ryu_U128) m) * mul[0]; // 0
//  Ryu_U128 b2 = ((Ryu_U128) m) * mul[1]; // 64
//
//  Ryu_U128 hi = (b0 >> 64) + b2;
//  Ryu_U128 lo = b0 & 0xffffffffffffffffull;
//  Ryu_U128 factor = (((Ryu_U128) mul[1]) << 64) + mul[0];
//  Ryu_U128 vpLo = lo + (factor << 1);
//  *vp = (U64) ((hi + (vpLo >> 64)) >> (j - 64));
//  Ryu_U128 vmLo = lo - (factor << mmShift);
//  *vm = (U64) ((hi + (vmLo >> 64) - (((Ryu_U128) 1ull) << 64)) >> (j - 64));
//  return (U64) (hi >> (j - 64));
    *vp = ryu_mulShift64(4 * m + 2, mul, j);
    *vm = ryu_mulShift64(4 * m - 1 - mmShift, mul, j);
    return ryu_mulShift64(4 * m, mul, j);
}

#elif defined(RYU_HAS_64_BIT_MSVC_INTRINSICS)

header_function
U64 ryu_mulShift64 (U64 m, U64* mul, S32 j) {
    // m is maximum 55 bits
    U64 high1;                                   // 128
    U64 low1 = ryu_umul128(m, mul[1], &high1); // 64
    U64 high0;                                   // 64
    ryu_umul128(m, mul[0], &high0);                       // 0
    U64 sum = high0 + low1;
    if (sum < high0) {
        ++high1; // overflow into high1
    }
    return ryu_shiftright128(sum, high1, j - 64);
}

header_function
U64 ryu_mulShiftAll64 (U64 m, U64* mul, S32 j,
                       U64* vp, U64* vm, U32 mmShift) {
    *vp = ryu_mulShift64(4 * m + 2, mul, j);
    *vm = ryu_mulShift64(4 * m - 1 - mmShift, mul, j);
    return ryu_mulShift64(4 * m, mul, j);
}

#else // !defined(RYU_HAS_UINT128_TYPE) && !defined(RYU_HAS_64_BIT_MSVC_INTRINSICS)

header_function
U64 ryu_mulShift64 (U64 m, U64* mul, S32 j) {
    // m is maximum 55 bits
    U64 high1;                                   // 128
    U64 low1 = ryu_umul128(m, mul[1], &high1); // 64
    U64 high0;                                   // 64
    ryu_umul128(m, mul[0], &high0);                       // 0
    U64 sum = high0 + low1;
    if (sum < high0) {
        ++high1; // overflow into high1
    }
    return ryu_shiftright128(sum, high1, j - 64);
}

// This is faster if we don't have a 64x64->128-bit multiplication.
header_function
U64 ryu_mulShiftAll64 (U64 m, U64* mul, S32 j,
                       U64* vp, U64* vm, U32 mmShift) {
    m <<= 1;
    // m is maximum 55 bits
    U64 tmp;
    U64 lo = ryu_umul128(m, mul[0], &tmp);
    U64 hi;
    U64 mid = tmp + ryu_umul128(m, mul[1], &hi);
    hi += mid < tmp; // overflow into hi

    U64 lo2 = lo + mul[0];
    U64 mid2 = mid + mul[1] + (lo2 < lo);
    U64 hi2 = hi + (mid2 < mid);
    *vp = ryu_shiftright128(mid2, hi2, (U32) (j - 64 - 1));

    if (mmShift == 1) {
        U64 lo3 = lo - mul[0];
        U64 mid3 = mid - mul[1] - (lo3 > lo);
        U64 hi3 = hi - (mid3 > mid);
        *vm = ryu_shiftright128(mid3, hi3, (U32) (j - 64 - 1));
    } else {
        U64 lo3 = lo + lo;
        U64 mid3 = mid + mid + (lo3 < lo);
        U64 hi3 = hi + hi + (mid3 < mid);
        U64 lo4 = lo3 - mul[0];
        U64 mid4 = mid3 - mul[1] - (lo4 > lo3);
        U64 hi4 = hi3 - (mid4 > mid3);
        *vm = ryu_shiftright128(mid4, hi4, (U32) (j - 64));
    }

    return ryu_shiftright128(mid, hi, (U32) (j - 64 - 1));
}

#endif // RYU_HAS_64_BIT_MSVC_INTRINSICS

#define RYU_DOUBLE_MANTISSA_BITS 52
#define RYU_DOUBLE_EXPONENT_BITS 11
#define RYU_DOUBLE_BIAS 1023

#define RYU_POW10_ADDITIONAL_BITS 120

#if defined(RYU_HAS_UINT128_TYPE)
header_function
Ryu_U128 ryu_umul256 (Ryu_U128 a, U64 bHi, U64 bLo, Ryu_U128* productHi) {
    U64 aLo = (U64)a;
    U64 aHi = (U64)(a >> 64);

    Ryu_U128 b00 = (Ryu_U128)aLo * bLo;
    Ryu_U128 b01 = (Ryu_U128)aLo * bHi;
    Ryu_U128 b10 = (Ryu_U128)aHi * bLo;
    Ryu_U128 b11 = (Ryu_U128)aHi * bHi;

    U64 b00Lo = (U64)b00;
    U64 b00Hi = (U64)(b00 >> 64);

    Ryu_U128 mid1 = b10 + b00Hi;
    U64 mid1Lo = (U64)(mid1);
    U64 mid1Hi = (U64)(mid1 >> 64);

    Ryu_U128 mid2 = b01 + mid1Lo;
    U64 mid2Lo = (U64)(mid2);
    U64 mid2Hi = (U64)(mid2 >> 64);

    Ryu_U128 pHi = b11 + mid1Hi + mid2Hi;
    Ryu_U128 pLo = ((Ryu_U128)mid2Lo << 64) | b00Lo;

    *productHi = pHi;
    return pLo;
}

// Returns the high 128 bits of the 256-bit product of a and b.
header_function
Ryu_U128 ryu_umul256_hi (Ryu_U128 a, U64 bHi, U64 bLo) {
    // Reuse the ryu_umul256 implementation.
    // Optimizers will likely eliminate the instructions used to compute the
    // low part of the product.
    Ryu_U128 hi;
    ryu_umul256(a, bHi, bLo, &hi);
    return hi;
}

// Unfortunately, gcc/clang do not automatically turn a 128-bit integer division
// into a multiplication, so we have to do it manually.
header_function
U32 ryu_uint128_mod1e9 (Ryu_U128 v) {
    // After multiplying, we're going to shift right by 29, then truncate to U32.
    // This means that we need only 29 + 32 = 61 bits, so we can truncate to U64 before shifting.
    U64 multiplied = (U64) ryu_umul256_hi(v, 0x89705F4136B4A597u, 0x31680A88F8953031u);

    // For U32 truncation, see the ryu_mod1e9() comment in d2s_intrinsics.h.
    U32 shifted = (U32) (multiplied >> 29);

    return ((U32) v) - 1000000000 * shifted;
}

// Best case: use 128-bit type.
header_function
U32 ryu_mulShift_mod1e9 (U64 m, const U64* mul, S32 j) {
    Ryu_U128 b0 = ((Ryu_U128) m) * mul[0]; // 0
    Ryu_U128 b1 = ((Ryu_U128) m) * mul[1]; // 64
    Ryu_U128 b2 = ((Ryu_U128) m) * mul[2]; // 128

    claim(j >= 128);
    claim(j <= 180);
    // j: [128, 256)
    Ryu_U128 mid = b1 + (U64) (b0 >> 64); // 64
    Ryu_U128 s1 = b2 + (U64) (mid >> 64); // 128
    return ryu_uint128_mod1e9(s1 >> (j - 128));
}

#else // RYU_HAS_UINT128_TYPE

#if defined(RYU_HAS_64_BIT_MSVC_INTRINSICS)
// Returns the low 64 bits of the high 128 bits of the 256-bit product of a and b.
header_function
U64 ryu_umul256_hi128_lo64 (
    U64 aHi, U64 aLo, U64 bHi, U64 bLo) {
    U64 b00Hi;
    U64 b00Lo = ryu_umul128(aLo, bLo, &b00Hi);
    U64 b01Hi;
    U64 b01Lo = ryu_umul128(aLo, bHi, &b01Hi);
    U64 b10Hi;
    U64 b10Lo = ryu_umul128(aHi, bLo, &b10Hi);
    U64 b11Hi;
    U64 b11Lo = ryu_umul128(aHi, bHi, &b11Hi);
    (void) b00Lo; // unused
    (void) b11Hi; // unused
    U64 temp1Lo = b10Lo + b00Hi;
    U64 temp1Hi = b10Hi + (temp1Lo < b10Lo);
    U64 temp2Lo = b01Lo + temp1Lo;
    U64 temp2Hi = b01Hi + (temp2Lo < b01Lo);
    return b11Lo + temp1Hi + temp2Hi;
}

header_function
U32 ryu_uint128_mod1e9 (U64 vHi, U64 vLo) {
    // After multiplying, we're going to shift right by 29, then truncate to U32.
    // This means that we need only 29 + 32 = 61 bits, so we can truncate to U64 before shifting.
    U64 multiplied = ryu_umul256_hi128_lo64(vHi, vLo, 0x89705F4136B4A597u, 0x31680A88F8953031u);

    // For U32 truncation, see the ryu_mod1e9() comment in d2s_intrinsics.h.
    U32 shifted = (U32) (multiplied >> 29);

    return ((U32) vLo) - 1000000000 * shifted;
}
#endif // RYU_HAS_64_BIT_MSVC_INTRINSICS

header_function
U32 ryu_mulShift_mod1e9 (U64 m, U64* mul, S32 j) {
    U64 high0;                                   // 64
    U64 low0 = ryu_umul128(m, mul[0], &high0); // 0
    U64 high1;                                   // 128
    U64 low1 = ryu_umul128(m, mul[1], &high1); // 64
    U64 high2;                                   // 192
    U64 low2 = ryu_umul128(m, mul[2], &high2); // 128
    U64 s0low = low0;              // 0
    (void) s0low; // unused
    U64 s0high = low1 + high0;     // 64
    U32 c1 = s0high < low1;
    U64 s1low = low2 + high1 + c1; // 128
    U32 c2 = s1low < low2; // high1 + c1 can't overflow, so compare against low2
    U64 s1high = high2 + c2;       // 192

    claim(j >= 128);
    claim(j <= 180);
#if defined(RYU_HAS_64_BIT_MSVC_INTRINSICS)
    U32 dist = (U32) (j - 128); // dist: [0, 52]
    U64 shiftedhigh = s1high >> dist;
    U64 shiftedlow = ryu_shiftright128(s1low, s1high, dist);
    return ryu_uint128_mod1e9(shiftedhigh, shiftedlow);
#else // RYU_HAS_64_BIT_MSVC_INTRINSICS
    if (j < 160) { // j: [128, 160)
        U64 r0 = ryu_mod1e9(s1high);
        U64 r1 = ryu_mod1e9((r0 << 32) | (s1low >> 32));
        U64 r2 = ((r1 << 32) | (s1low & 0xffffffff));
        return ryu_mod1e9(r2 >> (j - 128));
    } else { // j: [160, 192)
        U64 r0 = ryu_mod1e9(s1high);
        U64 r1 = ((r0 << 32) | (s1low >> 32));
        return ryu_mod1e9(r1 >> (j - 160));
    }
#endif // RYU_HAS_64_BIT_MSVC_INTRINSICS
}
#endif // RYU_HAS_UINT128_TYPE

// Convert `digits` to a sequence of decimal digits. Append the digits to the result.
// The caller has to guarantee that:
//   10^(olength-1) <= digits < 10^olength
// e.g., by passing `olength` as `decimalLength9(digits)`.
header_function
void ryu_append_n_digits (U32 olength, U32 digits, Char* result) {
    U32 i = 0;
    while (digits >= 10000) {
#if defined(COMPILER_CLANG) // https://bugs.llvm.org/show_bug.cgi?id=38217
        U32 c = digits - 10000 * (digits / 10000);
#else
        U32 c = digits % 10000;
#endif
        digits /= 10000;
        U32 c0 = (c % 100) << 1;
        U32 c1 = (c / 100) << 1;
        memcpy(result + olength - i - 2, RYU_DIGIT_TABLE + c0, 2);
        memcpy(result + olength - i - 4, RYU_DIGIT_TABLE + c1, 2);
        i += 4;
    }
    if (digits >= 100) {
        U32 c = (digits % 100) << 1;
        digits /= 100;
        memcpy(result + olength - i - 2, RYU_DIGIT_TABLE + c, 2);
        i += 2;
    }
    if (digits >= 10) {
        U32 c = digits << 1;
        memcpy(result + olength - i - 2, RYU_DIGIT_TABLE + c, 2);
    } else {
        result[0] = (Char) ('0' + digits);
    }
}

// Convert `digits` to a sequence of decimal digits. Print the first digit, followed by a decimal
// dot '.' followed by the remaining digits. The caller has to guarantee that:
//   10^(olength-1) <= digits < 10^olength
// e.g., by passing `olength` as `decimalLength9(digits)`.
header_function
void ryu_append_d_digits (U32 olength, U32 digits, Char* result) {
    U32 i = 0;
    while (digits >= 10000) {
#if defined(COMPILER_CLANG) // https://bugs.llvm.org/show_bug.cgi?id=38217
        U32 c = digits - 10000 * (digits / 10000);
#else
        U32 c = digits % 10000;
#endif
        digits /= 10000;
        U32 c0 = (c % 100) << 1;
        U32 c1 = (c / 100) << 1;
        memcpy(result + olength + 1 - i - 2, RYU_DIGIT_TABLE + c0, 2);
        memcpy(result + olength + 1 - i - 4, RYU_DIGIT_TABLE + c1, 2);
        i += 4;
    }
    if (digits >= 100) {
        U32 c = (digits % 100) << 1;
        digits /= 100;
        memcpy(result + olength + 1 - i - 2, RYU_DIGIT_TABLE + c, 2);
        i += 2;
    }
    if (digits >= 10) {
        U32 c = digits << 1;
        result[2] = RYU_DIGIT_TABLE[c + 1];
        result[1] = '.';
        result[0] = RYU_DIGIT_TABLE[c];
    } else {
        result[1] = '.';
        result[0] = (Char) ('0' + digits);
    }
}

// Convert `digits` to decimal and write the last `count` decimal digits to result.
// If `digits` contains additional digits, then those are silently ignored.
header_function
void ryu_append_c_digits (U32 count, U32 digits, Char* result) {
    // Copy pairs of digits from RYU_DIGIT_TABLE.
    U32 i = 0;
    for (; i < count - 1; i += 2) {
        U32 c = (digits % 100) << 1;
        digits /= 100;
        memcpy(result + count - i - 2, RYU_DIGIT_TABLE + c, 2);
    }
    // Generate the last digit if count is odd.
    if (i < count) {
        Char c = (Char) ('0' + (digits % 10));
        result[count - i - 1] = c;
    }
}

// Convert `digits` to decimal and write the last 9 decimal digits to result.
// If `digits` contains additional digits, then those are silently ignored.
header_function
void ryu_append_nine_digits (U32 digits, Char* result) {
    if (digits == 0) {
        memset(result, '0', 9);
        return;
    }

    for (U32 i = 0; i < 5; i += 4) {
#if defined(COMPILER_CLANG) // https://bugs.llvm.org/show_bug.cgi?id=38217
        U32 c = digits - 10000 * (digits / 10000);
#else
        U32 c = digits % 10000;
#endif
        digits /= 10000;
        U32 c0 = (c % 100) << 1;
        U32 c1 = (c / 100) << 1;
        memcpy(result + 7 - i, RYU_DIGIT_TABLE + c0, 2);
        memcpy(result + 5 - i, RYU_DIGIT_TABLE + c1, 2);
    }
    result[0] = (Char) ('0' + digits);
}

header_function
U32 ryu_indexForExponent (U32 e) {
    return (e + 15) / 16;
}

header_function
U32 ryu_pow10BitsForIndex (U32 idx) {
    return 16 * idx + RYU_POW10_ADDITIONAL_BITS;
}

header_function
U32 ryu_lengthForIndex (U32 idx) {
    // +1 for ceil, +16 for mantissa, +8 to round up when dividing by 9
    return (ryu_log10Pow2(16 * (S32) idx) + 1 + 16 + 8) / 9;
}

header_function
Sint ryu_copy_special_str_printf (Char* result, B32 sign, U64 mantissa) {
#if defined(_MSC_VER)
    // TODO: Check that -nan is expected output on Windows.
    if (sign) {
        result[0] = '-';
    }
    if (mantissa) {
        if (mantissa < (1ull << (RYU_DOUBLE_MANTISSA_BITS - 1))) {
            memcpy(result + sign, "nan(snan)", 9);
            return sign + 9;
        }
        memcpy(result + sign, "nan", 3);
        return sign + 3;
    }
#else
    if (mantissa) {
        memcpy(result, "nan", 3);
        return 3;
    }
    if (sign) {
        result[0] = '-';
    }
#endif
    memcpy(result + sign, "Infinity", 8);
    return (Sint)sign + 8;
}
#define NLIB_RYU_H_INCLUDE_GUARD
#endif
