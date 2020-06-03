/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2019 Naman Dixit
 * SPDX-License-Identifier: 0BSD OR BSL-1.0 OR Unlicense
 */

/* Parts of the file are using a modified version of the Ryu's C implementation.
 * Original work Copyright 2018 Ulf Adams
 * See https://github.com/ulfjack/ryu/blob/master/LICENSE-Boost for more details
 */

/* Define:
 * NLIB_PRINT_RYU_FLOAT for Ryu float->str algorithm (default)
 * NLIB_PRINT_BAD_FLOAT for cheap <U64 algorithm
 */

// Takes 12151 bytes (11.86 KiB) without the unit test
// Command: readelf -s test.linux.x64.libc | perl -ne 'if(/(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/) { print $3 . " " . $8. "\n";}' |sort -n | grep -i print | awk '{count=count+$1}END{print count/1024}'

#if defined(NLIB_PRINT_INTERFACE_ONLY)

header_function Size printStringVarArg (Char *, Char *, va_list);
header_function Size say  (Char *format, ...);
header_function Size sayv (Char *format, va_list ap);
header_function Size err  (Char *format, ...);
header_function Size errv (Char *format, va_list ap);

#if defined(OS_WINDOWS)
# if defined(BUILD_DEBUG)
#  define report(...)              printDebugOutput(__VA_ARGS__)
#  define reportv(format, va_list) printDebugOutput(format, va_list)
# else // = if !defined(BUILD_DEBUG)
#  define report(...)              err(stderr, __VA_ARGS__)
#  define reportv(format, va_list) errv(stderr, format, va_list)
# endif // defined(BUILD_DEBUG)
#elif defined(OS_LINUX)
# if defined(BUILD_DEBUG)
#  define report(...)              err(__VA_ARGS__)
#  define reportv(format, va_list) errv(format, va_list)
# else // = if !defined(BUILD_DEBUG)
#  define report(...)              err(__VA_ARGS__)
#  define reportv(format, va_list) errv(format, va_list)
# endif // defined(BUILD_DEBUG)
#endif // defined(OS_WINDOWS)


#elif !defined(NLIB_PRINT_H_INCLUDE_GUARD)

# if !defined(NLIB_PRINT_RYU_FLOAT) && !defined(NLIB_PRINT_BAD_FLOAT)
#  define NLIB_PRINT_RYU_FLOAT
# endif

# if defined(NLIB_PRINT_RYU_FLOAT)
#  include "nlib_ryu.h"
# endif // defined(NLIB_PRINT_RYU_FLOAT)

typedef enum Print_Flags {
    Print_Flags_LEFT_JUSTIFIED = 1u << 0,
    Print_Flags_ALTERNATE_FORM = 1u << 1,
    Print_Flags_LEADING_PLUS   = 1u << 2,
    Print_Flags_LEADING_SPACE  = 1u << 3,
    Print_Flags_LEADING_ZERO   = 1u << 4,
    Print_Flags_INT64          = 1u << 5,
    Print_Flags_NEGATIVE       = 1u << 6,
    Print_Flags_FLOAT_FIXED    = 1u << 7,
    Print_Flags_FLOAT_EXP      = 1u << 8,
    Print_Flags_FLOAT_HEX      = 1u << 9,
} Print_Flags;

header_function
Size printStringVarArg (Char *buffer, Char *format, va_list va)
{
    Char *fmt = format;
    Char *buf = buffer;

    while (true) {
        // Copy everything up to the next % (or end of string)
        while ((fmt[0] != '%') && (fmt[0] != '\0')) {
            if (buffer != NULL) {
                buf[0] = fmt[0];
            }
            fmt++;
            buf++;
        }

        if (fmt[0] == '%') {
            Char *format_pointer = fmt;

            fmt++;

            // read the modifiers first
            Sint field_width = 0;
            Sint precision = -1;
            Sint trailing_zeroes = 0;
            U32 flags = 0;

            // flags
            while (true) {
                switch (fmt[0]) {
                    case '-': { // if we have left justify
                        flags |= Print_Flags_LEFT_JUSTIFIED;
                        fmt++;
                        continue;
                    } break;
                    case '#': { // if we use alternate form
                        flags |= Print_Flags_ALTERNATE_FORM;
                        fmt++;
                        continue;
                    } break;
                    case '+': { // if we have leading plus
                        flags |= Print_Flags_LEADING_PLUS;
                        fmt++;
                        continue;
                    } break;
                    case ' ': { // if we have leading plus
                        flags |= Print_Flags_LEADING_SPACE;
                        fmt++;
                        continue;
                    } break;
                    case '0': { // if we have leading zero
                        flags |= Print_Flags_LEADING_ZERO;
                        fmt++;
                        goto flags_done;
                    } break;
                    default: {
                        goto flags_done;
                    } break;
                }
            }
        flags_done:

            // get the field width
            if (fmt[0] == '*') {
                field_width = va_arg(va, Sint);
                fmt++;
            } else {
                while ((fmt[0] >= '0') && (fmt[0] <= '9')) {
                    field_width = (field_width * 10) + (fmt[0] - '0');
                    fmt++;
                }
            }

            // get the precision
            if (fmt[0] == '.') {
                fmt++;
                if (fmt[0] == '*') {
                    precision = va_arg(va, Sint);
                    fmt++;
                } else {
                    precision = 0;
                    while ((fmt[0] >= '0') && (fmt[0] <= '9')) {
                        precision = (precision * 10) + (fmt[0] - '0');
                        fmt++;
                    }
                }
            }

            // handle integer size overrides
            switch (fmt[0]) {
                case 'l': { // are we 64-bit
                    flags |= Print_Flags_INT64;
                    fmt++;
                } break;
                case 'z': { // are we 64-bit on size_t?
                    flags |= (sizeof(Size) == 8) ? Print_Flags_INT64 : 0;
                    fmt++;
                } break;
                case 't': { // are we 64-bit on ptrdiff_t?
                    flags |= (sizeof(Dptr) == 8) ? Print_Flags_INT64 : 0;
                    fmt++;
                } break;
                default: {
                } break;
            }

#  define NLIB_PRINT_SIZE 2048ULL
            Char num_str[NLIB_PRINT_SIZE];
            Char *str = NULL;

            Char head_str[8] = {0};
            Size head_index = 0;

            Char tail_str[8] = {0};
            Size tail_index = 0;

            Size len = 0;

            switch (fmt[0]) { // handle each replacement
                case 's': { // string
                    // get the string
                    str = va_arg(va, Char*);
                    if (str == NULL) {
                        str = "null";
                    }

                    // NOTE(naman): By this point, str is most definitely not NULL
                    while (str[len] != '\0') {
                        len++;
                    }

                    // clamp to precision
                    // Since precision inits at -1, if none was mentioned, this will not execute
                    if (len > (Size)precision) {
                        len = (Size)precision;
                    }

                    precision = 0;
                } break;

                case 'c': { // char
                    // get the character
                    str = num_str + NLIB_PRINT_SIZE - 1;
                    *str = (Char)va_arg(va, Sint);
                    len = 1;
                    precision = 0;
                } break;

                case 'n': { // weird write-bytes specifier
                    Sint *d = va_arg(va, Sint*);
                    *d = (Sint)(buf - buffer);
                } break;

                case 'b': { // binary
                    U64 num = 0;
                    if (flags & Print_Flags_INT64) {
                        num = va_arg(va, U64);
                    } else {
                        num = va_arg(va, U32);
                    }

                    U64 num_dec = num;
                    str = num_str + NLIB_PRINT_SIZE;

                    while (true) {
                        U64 n = num_dec & 0x1;
                        num_dec = num_dec >> 1;

                        str--;
                        *str = (n == 1) ? '1' : '0';

                        if ((num_dec != 0) || (((num_str + NLIB_PRINT_SIZE) - str) < precision)) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    len = (((Uptr)num_str + NLIB_PRINT_SIZE) - (Uptr)str);

                    if (precision < 0) {
                        precision = 0;
                    }
                } break;

                case 'o': { // octal
                    U64 num = 0;
                    if (flags & Print_Flags_INT64) {
                        num = va_arg(va, U64);
                    } else {
                        num = va_arg(va, U32);
                    }

                    U64 num_dec = num;
                    str = num_str + NLIB_PRINT_SIZE;

                    while (true) {
                        U64 n = num_dec & 0x7;
                        num_dec = num_dec >> 3;

                        str--;
                        switch (n) {
                            case 0: *str = '0'; break;
                            case 1: *str = '1'; break;
                            case 2: *str = '2'; break;
                            case 3: *str = '3'; break;
                            case 4: *str = '4'; break;
                            case 5: *str = '5'; break;
                            case 6: *str = '6'; break;
                            case 7: *str = '7'; break;
                        }

                        if ((num_dec != 0) || (((num_str + NLIB_PRINT_SIZE) - str) < precision)) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    len = (((Uptr)num_str + NLIB_PRINT_SIZE) - (Uptr)str);

                    if (flags & Print_Flags_ALTERNATE_FORM) {
                        head_str[head_index++] = '0';
//                        head_str[head_index++] = 'o';
                    }

                    if (precision < 0) {
                        precision = 0;
                    }
                } break;

                case 'X':
                case 'x': { // hex
                    B32 upper = (fmt[0] == 'X') ? true : false;

                    U64 num = 0;
                    if (flags & Print_Flags_INT64) {
                        num = va_arg(va, U64);
                    } else {
                        num = va_arg(va, U32);
                    }

                    U64 num_dec = num;
                    str = num_str + NLIB_PRINT_SIZE;

                    if ((num == 0) && (precision == 0)) {
                        break;
                    }

                    while (true) {
                        U64 n = num_dec & 0xf;
                        num_dec = num_dec >> 4;

                        str--;
                        switch (n) {
                            case 0x0: *str = '0'; break;
                            case 0x1: *str = '1'; break;
                            case 0x2: *str = '2'; break;
                            case 0x3: *str = '3'; break;
                            case 0x4: *str = '4'; break;
                            case 0x5: *str = '5'; break;
                            case 0x6: *str = '6'; break;
                            case 0x7: *str = '7'; break;
                            case 0x8: *str = '8'; break;
                            case 0x9: *str = '9'; break;
                            case 0xA: *str = upper ? 'A' : 'a'; break;
                            case 0xB: *str = upper ? 'B' : 'b'; break;
                            case 0xC: *str = upper ? 'C' : 'c'; break;
                            case 0xD: *str = upper ? 'D' : 'd'; break;
                            case 0xE: *str = upper ? 'E' : 'e'; break;
                            case 0xF: *str = upper ? 'F' : 'f'; break;
                        }

                        if ((num_dec != 0) || (((num_str + NLIB_PRINT_SIZE) - str) < precision)) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    len = (((Uptr)num_str + NLIB_PRINT_SIZE) - (Uptr)str);

                    if (flags & Print_Flags_ALTERNATE_FORM) {
                        head_str[head_index++] = '0';
                        if (upper) {
                            head_str[head_index++] = 'X';
                        } else {
                            head_str[head_index++] = 'x';
                        }
                    }

                    if (precision < 0) {
                        precision = 0;
                    }
                } break;

                case 'u':
                case 'd': { // integer
                    // get the integer and abs it
                    U64 num = 0;
                    if (flags & Print_Flags_INT64) {
                        S64 i64 = va_arg(va, S64);
                        num = (U64)i64;
                        if ((fmt[0] != 'u') && (i64 < 0)) {
                            num = (U64)-i64;
                            flags |= Print_Flags_NEGATIVE;
                        }
                    } else {
                        S32 i = va_arg(va, S32);
                        num = (U32)i;
                        if ((fmt[0] != 'u') && (i < 0)) {
                            num = (U32)-i;
                            flags |= Print_Flags_NEGATIVE;
                        }
                    }

                    // convert to string
                    U64 num_dec = num;
                    str = num_str + NLIB_PRINT_SIZE;

                    if ((num == 0) && (precision == 0)) {
                        break;
                    }

                    while (num_dec) {
                        str--;
                        *str = (Char)(num_dec % 10) + '0';
                        num_dec /= 10;
                    }

                    // get the length that we copied
                    len = (((Uptr)num_str + NLIB_PRINT_SIZE) - (Uptr)str);

                    if (len == 0) {
                        --str;
                        *str = '0';
                        len = 1;
                    }

                    if (flags & Print_Flags_NEGATIVE) {
                        head_str[head_index++] = '-';
                    }

                    if (flags & Print_Flags_LEADING_PLUS) {
                        head_str[head_index++] = '+';
                    }

                    if (flags & Print_Flags_LEADING_SPACE) {
                        if (!(flags & Print_Flags_NEGATIVE)) {
                            head_str[head_index++] = ' ';
                        }
                    }

                    if (flags & Print_Flags_LEADING_ZERO) {
                        head_str[head_index++] = '0';
                    }

                    if (precision < 0) {
                        precision = 0;
                    }
                } break;

                case 'p': { // pointer
                    flags |= (sizeof(void*) == 8) ? Print_Flags_INT64 : 0;
                    precision = sizeof(void*) * 2;

                    U64 num = 0;
                    if (flags & Print_Flags_INT64) {
                        num = va_arg(va, U64);
                    } else {
                        num = va_arg(va, U32);
                    }

                    U64 num_dec = num;
                    str = num_str + NLIB_PRINT_SIZE;

                    while (true) {
                        U64 n = num_dec & 0xf;
                        num_dec = num_dec >> 4;

                        str--;
                        switch (n) {
                            case 0x0: *str = '0'; break;
                            case 0x1: *str = '1'; break;
                            case 0x2: *str = '2'; break;
                            case 0x3: *str = '3'; break;
                            case 0x4: *str = '4'; break;
                            case 0x5: *str = '5'; break;
                            case 0x6: *str = '6'; break;
                            case 0x7: *str = '7'; break;
                            case 0x8: *str = '8'; break;
                            case 0x9: *str = '9'; break;
                            case 0xA: *str = 'A'; break;
                            case 0xB: *str = 'B'; break;
                            case 0xC: *str = 'C'; break;
                            case 0xD: *str = 'D'; break;
                            case 0xE: *str = 'E'; break;
                            case 0xF: *str = 'F'; break;
                        }

                        if ((num_dec != 0) || (((num_str + NLIB_PRINT_SIZE) - str) < precision)) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    len = (((Uptr)num_str + NLIB_PRINT_SIZE) - (Uptr)str);
                } break;

                case 'f': case 'F':
                case 'e': case 'E':
                case 'a': case 'A': {
                    switch (fmt[0]) {
                        case 'f': case 'F': { flags |= Print_Flags_FLOAT_FIXED;   } break;
                        case 'e': case 'E': { flags |= Print_Flags_FLOAT_EXP;     } break;
                        case 'a': case 'A': { flags |= Print_Flags_FLOAT_HEX;     } break;
                    }

                    B32 capital = false;
                    switch (fmt[0]) {
                        case 'F': case 'E': case 'A': { capital = true;  } break;
                    }

                    if ((precision < 0) && !(flags & Print_Flags_FLOAT_HEX)) precision = 6;

                    F64 f64 = va_arg(va, F64);
                    union FU64 { F64 f; U64 u;} fu64 = {.f = f64};
                    U64 bits = fu64.u;

                    /* NOTE(naman): 64-bit IEEE-754 Floating Point structure
                       ____________________________________________
                       |  Sign bit | Exponent bits | Mantissa bits|
                       |     1     |       11      |      52      |
                       --------------------------------------------
                    */

#  define            F64_MANTISSA_BITS 52
#  define            F64_EXPONENT_BITS 11
#  define            F64_BIAS 1023

                    // Is the top bit set?
                    B32 negative  = ((bits >> (F64_MANTISSA_BITS + F64_EXPONENT_BITS)) & 1) != 0;
                    // Remove all mantissa bits ------------------ and then reset the sign bit
                    U32 exponent_biased = (((U32)(bits >> F64_MANTISSA_BITS)) &
                                           ((1U << F64_EXPONENT_BITS) - 1U));
                    S32 exponent = (S32)exponent_biased - F64_BIAS;
                    // Reset all bits except for the mantissa bits
                    U64 mantissa  = bits & ((1ULL << F64_MANTISSA_BITS) - 1ULL);

                    if (negative) {
                        flags |= Print_Flags_NEGATIVE;
                    }

                    { // Leading String
                        if (flags & Print_Flags_NEGATIVE) {
                            head_str[head_index++] = '-';
                        } else if (flags & Print_Flags_LEADING_PLUS) {
                            head_str[head_index++] = '+';
                        } else if (flags & Print_Flags_LEADING_SPACE) {
                            if (!(flags & Print_Flags_NEGATIVE)) {
                                head_str[head_index++] = ' ';
                            }
                        }

                        if ((flags & Print_Flags_LEADING_ZERO) &&
                            !(flags & Print_Flags_FLOAT_HEX)) {
                            head_str[head_index++] = '0';
                        }
                    }

                    if (exponent_biased == 0x7FF) { // Handle NaN and Inf
                        if (capital) {
                            if (mantissa) {
                                str = "NAN";
                            } else {
                                str = "INF";
                            }
                        } else {
                            if (mantissa) {
                                str = "nan";
                            } else {
                                str = "inf";
                            }
                        }

                        len = 3;
                    } else if (exponent_biased == 0 && mantissa == 0) {
                        if (flags & Print_Flags_FLOAT_HEX) {
                            head_str[head_index++] = '0';
                            head_str[head_index++] = capital ? 'X' : 'x';
                            head_str[head_index++] = '0';
                        }

                        str = num_str;

                        *str++ = '0';

                        if (precision > 0) {
                            *str++ = '.';
                            trailing_zeroes += precision;
                        }

                        len = (Size)(Dptr)(str - num_str);
                        str = num_str;

                        if (flags & Print_Flags_FLOAT_EXP) {
                            tail_str[tail_index++] = capital ? 'E' : 'e';
                            tail_str[tail_index++] = '+';
                            tail_str[tail_index++] = '0';
                            tail_str[tail_index++] = '0';
                        } else if (flags & Print_Flags_FLOAT_HEX) {
                            tail_str[tail_index++] = capital ? 'P' : 'p';
                            tail_str[tail_index++] = '+';
                            tail_str[tail_index++] = '0';
                        }
                    }

                    if ((str == NULL) && (flags & Print_Flags_FLOAT_HEX)) {
                        S32 ex = exponent;
                        B32 denormal = false;

                        if ((exponent_biased == 0) && (mantissa != 0)) { // Denormals
                            denormal = true;
                            ex = -1022;
                        }

                        head_str[head_index++] = '0';
                        head_str[head_index++] = capital ? 'X' : 'x';

                        if (mantissa || precision) {
                            U64 man = mantissa;
                            S32 pr = precision > 0 ? precision : 13;

                            if (denormal == false) {
                                // NOTE(naman): This sets the 53rd bit
                                man |= 1ULL << 52;
                            }

                            /* NOTE(naman): This makes it so that the MSB of mantissa is on
                             * 60th bit, and the normal/denormal bit (that we set above)
                             * is on 61st.
                             *
                             *    bitsInF64 - (bitsInHex * (MANTISSA_BITS/bitsInHex + 1))
                             * => 64 - (4 * (13 + 1))
                             * => 8
                             * (1 at the end comes to handle the 53rd bit)
                             */
                            man <<= 8;

                            // NOTE(naman): This will do the rounding if the precision is
                            // smaller. It does this by incrementing the MSB of mantissa
                            // that won't be printed. Doing that will send a carry forward
                            // if the hex associated with that MSB was >= 8; and not if it was
                            // < 7. Similarly, the carry will propagate further, rounding each
                            // hex to its nearest value, with ties away from zero.
                            if (pr < 13) {
                                const U64 one_at_unprinted_msb_if_precision_is[13] = {
                                    576460752303423488ULL, // 800000000000000
                                    36028797018963968ULL,  // 80000000000000
                                    2251799813685248ULL,   // 8000000000000
                                    140737488355328ULL,    // 800000000000
                                    8796093022208ULL,      // 80000000000
                                    549755813888ULL,       // 8000000000
                                    34359738368ULL,        // 800000000
                                    2147483648ULL,         // 80000000
                                    134217728ULL,          // 8000000
                                    8388608ULL,            // 800000
                                    524288ULL,             // 80000
                                    32768ULL,              // 8000
                                    2048ULL,               // 800
                                };

                                U64 one_at_unprinted_msb  = one_at_unprinted_msb_if_precision_is[pr];

                                U64 ones_after_unprinted_msb = one_at_unprinted_msb - 1ULL;
                                U64 zeroes_at_unprinted = ((Size)-1) ^ ((ones_after_unprinted_msb << 1) | 1);
                                U64 one_at_printed_lsb = one_at_unprinted_msb << 1;

                                // https://indepth.dev/how-to-round-binary-numbers/
                                U64 lower = man & zeroes_at_unprinted;
                                U64 middle = lower | one_at_unprinted_msb;
                                U64 upper = lower + one_at_printed_lsb;

                                if (man < middle) {
                                    man = lower;
                                } else if (man > middle) {
                                    man = upper;
                                } else {
                                    if ((lower & one_at_printed_lsb) == 0) {
                                        man = lower;
                                    } else {
                                        man = upper;
                                    }
                                }

#  if 0 // Rounding as implemented in stb_printf.h
                                U64 one_at_60th_pos = 1ULL << 59;
                                U64 count_printed_bits = (U64)pr * 4ULL;
                                U64 one_at_unprinted_msb = (one_at_60th_pos >> count_printed_bits);

                                man += one_at_unprinted_msb;
#  endif
                            }

                            str = num_str;
                            Char *hexs = capital ? "0123456789ABCDEF" : "0123456789abcdef";

                            // NOTE(naman): This prints 0/1 depending on normal/denormal status
                            *str++ = hexs[(man >> 60) & 0xF];
                            man <<= 4;

                            if (precision) *str++ = '.';

                            S32 n = pr;

                            if (n > 13) n = 13;
                            if (pr > (S32)n) trailing_zeroes = pr - n;

                            U32 count_of_end_zeroes = 0;
                            while (n--) {
                                if ((man >> 60) & 0xF) {
                                    count_of_end_zeroes = 0;
                                } else {
                                    count_of_end_zeroes++;
                                }

                                *str++ = hexs[(man >> 60) & 0xF];
                                man <<= 4;
                            }

                            if (precision >= 0) {
                                count_of_end_zeroes = 0;
                            }

                            len = (Size)(Dptr)(str - num_str - count_of_end_zeroes);
                            str = num_str;
                        } else {
                            len = 0;
                        }

                        {
                            tail_str[tail_index++] = capital ? 'P' : 'p';
                            tail_str[tail_index++] = ex > 0 ? '+' : '-';
                            ex = ex > 0 ? ex : -ex;

                            Char es[4] = {0};
                            Char *ep = &es[elemin(es)];
                            Char el = 0;
                            while (ex) {
                                *--ep = (Char)(ex % 10) + '0';
                                el++;
                                ex /= 10;
                            }

                            while (el) {
                                el--;
                                tail_str[tail_index++] = *ep++;
                            }
                        }
                    }

                    if (str == NULL) {
#  if defined(NLIB_PRINT_BAD_FLOAT)
                        F64 value = (f64 < 0) ? -f64 : f64;
                        B32 power_of_e_nonsense = false;
                        Sint print_exponent = 0;

                        // NOTE(naman): We just overload 'F' to remove lower bound too
                        if (capital && (flags & Print_Flags_FLOAT_FIXED)) {
                            flags |= Print_Flags_FLOAT_NO_LOW_BOUND;
                        }

                        { // Limit the "range" of float
                            // log10(2^64) = 19 = max integral F64 storable in U64 without
                            // loss of information
                            F64 upper_threshold = 1e19;

                            if (value >= upper_threshold) {
                                power_of_e_nonsense = true;
                                if (value >= 1e256) {
                                    value /= 1e256;
                                    print_exponent += 256;
                                }
                                if (value >= 1e128) {
                                    value /= 1e128;
                                    print_exponent += 128;
                                }
                                if (value >= 1e64) {
                                    value /= 1e64;
                                    print_exponent += 64;
                                }
                                if (value >= 1e32) {
                                    value /= 1e32;
                                    print_exponent += 32;
                                }
                                if (value >= 1e16) {
                                    value /= 1e16;
                                    print_exponent += 16;
                                }
                                if (value >= 1e8) {
                                    value /= 1e8;
                                    print_exponent += 8;
                                }
                                if (value >= 1e4) {
                                    value /= 1e4;
                                    print_exponent += 4;
                                }
                                if (value >= 1e2) {
                                    value /= 1e2;
                                    print_exponent += 2;
                                }
                                if (value >= 1e1) {
                                    value /= 1e1;
                                    print_exponent += 1;
                                }
                            }

                            if (!(flags & Print_Flags_FLOAT_NO_LOW_BOUND)) {
                                // 10^-(precision-1)
                                // (so that we get atleast one digit)
                                F64 powers_of_10[20] = {
                                    1.000000,
                                    10.000000,
                                    100.000000,
                                    1000.000000,
                                    10000.000000,
                                    100000.000000,
                                    1000000.000000,
                                    10000000.000000,
                                    100000000.000000,
                                    1000000000.000000,
                                    10000000000.000000,
                                    100000000000.000000,
                                    1000000000000.000000,
                                    10000000000000.000000,
                                    100000000000000.000000,
                                    1000000000000000.000000,
                                    10000000000000000.000000,
                                    100000000000000000.000000,
                                    1000000000000000000.000000,
                                    10000000000000000000.000000,
                                };

                                F64 lower_threshold;
                                // lower_threshold = pow(10.0, 1.0 - (F64)precision);
                                S32 power = 1 - precision;
                                if (power < 20) {
                                    lower_threshold = powers_of_10[power];
                                } else {
                                    lower_threshold = 1.0;
                                    for (Sint i = power; i > 0; i--) {
                                        lower_threshold *= 10.0;
                                    }
                                }

                                if (value > 0 && value <= lower_threshold) {
                                    power_of_e_nonsense = true;
                                    if (value < 1e-255) {
                                        value *= 1e256;
                                        print_exponent -= 256;
                                    }
                                    if (value < 1e-127) {
                                        value *= 1e128;
                                        print_exponent -= 128;
                                    }
                                    if (value < 1e-63) {
                                        value *= 1e64;
                                        print_exponent -= 64;
                                    }
                                    if (value < 1e-31) {
                                        value *= 1e32;
                                        print_exponent -= 32;
                                    }
                                    if (value < 1e-15) {
                                        value *= 1e16;
                                        print_exponent -= 16;
                                    }
                                    if (value < 1e-7) {
                                        value *= 1e8;
                                        print_exponent -= 8;
                                    }
                                    if (value < 1e-3) {
                                        value *= 1e4;
                                        print_exponent -= 4;
                                    }
                                    if (value < 1e-1) {
                                        value *= 1e2;
                                        print_exponent -= 2;
                                    }
                                    if (value < 1e0) {
                                        value *= 1e1;
                                        print_exponent -= 1;
                                    }
                                }
                            }
                        }

                        U64 integral_part = (U64)value;
                        F64 remainder = value - (F64)integral_part;

                        F64 remainder_modified = remainder * 1e19;
                        U64 decimal_part = (U64)remainder_modified;

                        // rounding
                        remainder_modified -= (F64)decimal_part;
                        if (remainder_modified >= 0.5) {
                            decimal_part++;
                            if (decimal_part >= 10000000000000000000ULL) { // 19 zeroes
                                decimal_part = 0;
                                integral_part++;
                                if (print_exponent != 0 && integral_part >= 10) {
                                    print_exponent++;
                                    integral_part = 1;
                                }
                            }
                        }

                        str = num_str + NLIB_PRINT_SIZE;

                        { // Write print_exponent (if needed)
                            if (power_of_e_nonsense) {
                                // convert to string
                                Sint num_dec = (print_exponent > 0) ? print_exponent : -print_exponent;

                                Char *str_now = str;

                                if (num_dec) {
                                    while (num_dec) {
                                        *--str = (Char)(num_dec % 10) + '0';
                                        num_dec /= 10;
                                    }
                                } else {
                                    *--str = '0';
                                }

                                if ((Uptr)str_now - (Uptr)str == 1) {
                                    *--str = '0';
                                }

                                if (print_exponent < 0) {
                                    *--str = '-';
                                } else {
                                    *--str = '+';
                                }

                                *--str = 'e';
                            }
                        }

                        { // Write decimal part
                            if (precision > 0) {
                                Char tmp_buf[NLIB_PRINT_SIZE] = {0};
                                Char *tmp = tmp_buf + NLIB_PRINT_SIZE;
                                Uint width = 19; // TODO(naman): Is this correct?

                                Uint width_iter = width;

                                // FIXME(naman): Get more decimal digits instead of just
                                // printing zeroes
                                for (Uint i = (Uint)precision; i > width; i--) {
                                    *--tmp = '0';
                                }

                                do {
                                    *--tmp = (Char)(decimal_part % 10) + '0';
                                    decimal_part = decimal_part / 10;
                                } while (--width_iter);

                                if ((Uint)precision < width) {
                                    for (Char *temp = tmp_buf + NLIB_PRINT_SIZE - 1;
                                         (Uptr)temp - (Uptr)tmp >= (Uint)precision;
                                         temp--) {
                                        Size index = 0;

                                        if ((temp[index] - '0') > 5) {
                                            B32 carry = false;
                                            do {
                                                if ((temp[index-1] - '0') == 9) {
                                                    temp[index-1] = '0';
                                                    carry = true;
                                                } else {
                                                    temp[index-1] += 1;
                                                    carry = false;
                                                }
                                                index--;
                                            } while (carry && ((Uptr) temp - index >= (Uptr)tmp));
                                        }
                                    }
                                }

                                str -= precision;
                                for (Size i = 0; i < (Size)precision; i++) {
                                    str[i] = tmp[i];
                                }

                                *--str = '.';
                            }
                        }

                        { // Write integral
                            do {
                                *--str = (Char)(integral_part % 10) + '0';
                                integral_part = integral_part / 10;
                            } while (integral_part);
                        }

                        // get the length that we copied
                        len = (((Uptr)num_str + NLIB_PRINT_SIZE) - (Uptr)str);

                        if (len == 0) {
                            *--str = '0';
                            len = 1;
                        }
#  elif defined(NLIB_PRINT_RYU_FLOAT)
                        str = num_str;
//                      Sint ryu_len = ryu_d2fixed_buffered_n(f64, (U32)precision, str);
//                      len = (Size)ryu_len;

                        S32 e2;
                        U64 m2;

                        if (exponent_biased == 0) {
                            e2 = 1 - F64_BIAS - F64_MANTISSA_BITS;
                            m2 = mantissa;
                        } else {
                            e2 = (S32)exponent_biased - F64_BIAS - F64_MANTISSA_BITS;
                            m2 = (1ULL << F64_MANTISSA_BITS) | mantissa;
                        }

#   define INDEX_FOR_EXPONENT(e) ((e) + 15) / 16
#   define POW_10_BITS_FOR_INDEX(index) (16 * (index) + RYU_POW10_ADDITIONAL_BITS)
// +1 for ceil, +16 for mantissa, +8 to round up when dividing by 9
#   define LENGTH_FOR_INDEX(index) ((ryu_log10Pow2(16 * ((S32)index)) + 1 + 16 + 8) / 9)

                        if (flags & Print_Flags_FLOAT_FIXED) {
                            B32 has_seen_non_zero_digit = false;

                            if (e2 >= -52) {
                                U32 index = e2 < 0 ? 0 : INDEX_FOR_EXPONENT((U32)e2);
                                U32 p10_bits = POW_10_BITS_FOR_INDEX(index);
                                S32 index_length = LENGTH_FOR_INDEX(index);

                                for (S32 i = index_length - 1; i >= 0; --i) {
                                    U32 j = p10_bits - (U32)e2;
                                    // Temporary: j is usually around 128, and by shifting a bit, we push it to 128 or above, which is
                                    // a slightly faster code path in ryu_mulShift_mod1e9. Instead, we can just increase the multipliers.
                                    const U64 *table_lookup = RYU_POW10_SPLIT[RYU_POW10_OFFSET[index] + i];
                                    U32 digits = ryu_mulShift_mod1e9(m2 << 8,
                                                                     table_lookup,
                                                                     (S32)(j + 8));
                                    if (has_seen_non_zero_digit) {
                                        ryu_append_nine_digits(digits, str + len);
                                        len += 9;
                                    } else if (digits != 0) {
                                        U32 olength = ryu_decimalLength9(digits);
                                        ryu_append_n_digits(olength, digits, str + len);
                                        len += olength;
                                        has_seen_non_zero_digit = true;
                                    } else {
                                        // skip the digits
                                    }
                                }
                            }

                            if (!has_seen_non_zero_digit) {
                                str[len++] = '0';
                            }

                            if (precision > 0) {
                                str[len++] = '.';
                            }

                            if (e2 >= 0) {
                                if (precision > 0) {
                                    trailing_zeroes += precision;
                                }
                            } else {
                                S32 idx = -e2 / 16;

                                U32 blocks = ((U32)precision / 9) + 1;
                                U32 i = 0;

                                if (blocks <= RYU_MIN_BLOCK_2[idx]) {
                                    i = blocks;
                                    trailing_zeroes += precision;
                                } else if (i < RYU_MIN_BLOCK_2[idx]) {
                                    i = RYU_MIN_BLOCK_2[idx];
                                    trailing_zeroes += 9 * i;
                                }

                                // 0 = don't round up; 1 = round up unconditionally; 2 = round up if odd.
                                Sint round_up = 0;

                                for (; i < blocks; ++i) {
                                    S32 j = RYU_ADDITIONAL_BITS_2 + (-e2 - 16 * idx);
                                    U32 p = RYU_POW10_OFFSET_2[idx] + i - RYU_MIN_BLOCK_2[idx];

                                    if (p >= RYU_POW10_OFFSET_2[idx + 1]) {
                                        // If the remaining digits are all 0, then we might as well use memset.
                                        // No rounding required in this case.
                                        U32 fill = (U32)precision - 9 * i;
                                        trailing_zeroes += fill;
                                        break;
                                    }

                                    // Temporary: j is usually around 128, and by shifting a bit, we push it to 128 or above, which is
                                    // a slightly faster code path in ryu_mulShift_mod1e9. Instead, we can just increase the multipliers.
                                    U32 digits = ryu_mulShift_mod1e9(m2 << 8, RYU_POW10_SPLIT_2[p], j + 8);

                                    if (i < blocks - 1) {
                                        ryu_append_nine_digits(digits, str + len);
                                        len += 9;
                                    } else {
                                        U32 maximum = (U32)precision - 9 * i;
                                        U32 last_digit = 0;

                                        for (U32 k = 0; k < 9 - maximum; ++k) {
                                            last_digit = digits % 10;
                                            digits /= 10;
                                        }

                                        if (last_digit != 5) {
                                            round_up = last_digit > 5;
                                        } else {
                                            // Is m * 10^(additionalDigits + 1) / 2^(-e2) integer?
                                            S32 required_twos = -e2 - (S32) precision - 1;
                                            B32 any_trailing_zeros = (required_twos <= 0)
                                                || (required_twos < 60 &&
                                                    ryu_multipleOfPowerOf2(m2,
                                                                           (U32)required_twos));
                                            round_up = any_trailing_zeros ? 2 : 1;
                                        }

                                        if (maximum > 0) {
                                            ryu_append_c_digits(maximum, digits, str + len);
                                            len += maximum;
                                        }

                                        break;
                                    }
                                }

                                if (round_up != 0) {
                                    Sint round_index = (Sint)len;
                                    Sint dot_index = 0; // '.' can't be located at index 0
                                    while (true) {
                                        --round_index;
                                        Char c;
# if defined(COMPILER_CLANG)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wcomma"
# endif
                                        if (round_index == -1 || (c = str[round_index], c == '-')) {
                                            str[round_index + 1] = '1';
                                            if (dot_index > 0) {
                                                str[dot_index] = '0';
                                                str[dot_index + 1] = '.';
                                            }
                                            str[len++] = '0';
                                            break;
                                        }
# if defined(COMPILER_CLANG)
#  pragma clang diagnostic pop
# endif
                                        if (c == '.') {
                                            dot_index = round_index;
                                            continue;
                                        } else if (c == '9') {
                                            str[round_index] = '0';
                                            round_up = 1;
                                            continue;
                                        } else {
                                            if (round_up == 2 && c % 2 == 0) {
                                                break;
                                            }
                                            str[round_index] = c + 1;
                                            break;
                                        }
                                    }
                                }
                            }
                        } else if (flags & Print_Flags_FLOAT_EXP) {
                            B32 print_decimal_point = precision > 0;
                            ++precision;

                            U32 digits = 0;
                            U32 printed_digits = 0;
                            U32 available_digits = 0;
                            S32 exp = 0;

                            if (e2 >= -52) {
                                U32 idx = e2 < 0 ? 0 : INDEX_FOR_EXPONENT((U32)e2);
                                U32 p10bits = POW_10_BITS_FOR_INDEX(idx);
                                S32 index_length = LENGTH_FOR_INDEX(idx);

                                for (S32 i = index_length - 1; i >= 0; --i) {
                                    U32 j = p10bits - (U32)e2;
                                    // Temporary: j is usually around 128, and by shifting a bit, we push it to 128 or above, which is
                                    // a slightly faster code path in ryu_mulShift_mod1e9. Instead, we can just increase the multipliers.
                                    const U64 *table_lookup = RYU_POW10_SPLIT[RYU_POW10_OFFSET[idx] + i];
                                    digits = ryu_mulShift_mod1e9(m2 << 8, table_lookup, (S32)(j + 8));

                                    if (printed_digits != 0) {
                                        if (printed_digits + 9 > (Uint)precision) {
                                            available_digits = 9;
                                            break;
                                        }

                                        ryu_append_nine_digits(digits, str + len);
                                        len += 9;
                                        printed_digits += 9;
                                    } else if (digits != 0) {
                                        available_digits = ryu_decimalLength9(digits);
                                        exp = i * 9 + (S32) available_digits - 1;

                                        if (available_digits > (Uint)precision) {
                                            break;
                                        }

                                        if (print_decimal_point) {
                                            ryu_append_d_digits(available_digits, digits, str + len);
                                            len += available_digits + 1; // +1 for decimal point
                                        } else {
                                            str[len++] = (Char)('0' + digits);
                                        }

                                        printed_digits = available_digits;
                                        available_digits = 0;
                                    }
                                }
                            }

                            if (e2 < 0 && available_digits == 0) {
                                S32 idx = -e2 / 16;

                                for (S32 i = RYU_MIN_BLOCK_2[idx]; i < 200; ++i) {
                                    S32 j = RYU_ADDITIONAL_BITS_2 + (-e2 - 16 * idx);
                                    U32 p = RYU_POW10_OFFSET_2[idx] + (U32) i - RYU_MIN_BLOCK_2[idx];
                                    // Temporary: j is usually around 128, and by shifting a bit, we push it to 128 or above, which is
                                    // a slightly faster code path in ryu_mulShift_mod1e9. Instead, we can just increase the multipliers.
                                    digits = (p >= RYU_POW10_OFFSET_2[idx + 1]) ? 0 : ryu_mulShift_mod1e9(m2 << 8, RYU_POW10_SPLIT_2[p], j + 8);

                                    if (printed_digits != 0) {
                                        if (printed_digits + 9 > (Uint)precision) {
                                            available_digits = 9;
                                            break;
                                        }
                                        ryu_append_nine_digits(digits, str + len);
                                        len += 9;
                                        printed_digits += 9;
                                    } else if (digits != 0) {
                                        available_digits = ryu_decimalLength9(digits);
                                        exp = -(i + 1) * 9 + (S32) available_digits - 1;
                                        if (available_digits > (Uint)precision) {
                                            break;
                                        }
                                        if (print_decimal_point) {
                                            ryu_append_d_digits(available_digits, digits, str + len);
                                            len += available_digits + 1; // +1 for decimal point
                                        } else {
                                            str[len++] = (Char)('0' + digits);
                                        }
                                        printed_digits = available_digits;
                                        available_digits = 0;
                                    }
                                }
                            }

                            U32 maximum = (U32)precision - printed_digits;

                            if (available_digits == 0) {
                                digits = 0;
                            }
                            U32 last_digit = 0;
                            if (available_digits > maximum) {
                                for (U32 k = 0; k < available_digits - maximum; ++k) {
                                    last_digit = digits % 10;
                                    digits /= 10;
                                }
                            }

                            // 0 = don't round up; 1 = round up unconditionally; 2 = round up if odd.
                            Sint round_up = 0;
                            if (last_digit != 5) {
                                round_up = last_digit > 5;
                            } else {
                                // Is m * 2^e2 * 10^(precision + 1 - exp) integer?
                                // precision was already increased by 1, so we don't need to write + 1 here.
                                S32 rexp = (S32) precision - exp;
                                S32 requiredTwos = -e2 - rexp;
                                B32 any_trailing_zeros = requiredTwos <= 0
                                    || (requiredTwos < 60 && ryu_multipleOfPowerOf2(m2, (U32) requiredTwos));
                                if (rexp < 0) {
                                    S32 requiredFives = -rexp;
                                    any_trailing_zeros = any_trailing_zeros && ryu_multipleOfPowerOf5(m2, (U32) requiredFives);
                                }
                                round_up = any_trailing_zeros ? 2 : 1;
                            }

                            if (printed_digits != 0) {
                                if (digits == 0) {
                                    memset(str + len, '0', maximum);
                                } else {
                                    ryu_append_c_digits(maximum, digits, str + len);
                                }
                                len += maximum;
                            } else {
                                if (print_decimal_point) {
                                    ryu_append_d_digits(maximum, digits, str + len);
                                    len += maximum + 1; // +1 for decimal point
                                } else {
                                    str[len++] = (Char)('0' + digits);
                                }
                            }

                            if (round_up != 0) {
                                Sint round_index = (Sint)len;
                                while (true) {
                                    --round_index;
                                    Char c;
# if defined(COMPILER_CLANG)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wcomma"
# endif
                                    if (round_index == -1 || (c = str[round_index], c == '-')) {
                                        str[round_index + 1] = '1';
                                        ++exp;
                                        break;
                                    }
# if defined(COMPILER_CLANG)
#  pragma clang diagnostic pop
# endif
                                    if (c == '.') {
                                        continue;
                                    } else if (c == '9') {
                                        str[round_index] = '0';
                                        round_up = 1;
                                        continue;
                                    } else {
                                        if (round_up == 2 && c % 2 == 0) {
                                            break;
                                        }
                                        str[round_index] = c + 1;
                                        break;
                                    }
                                }
                            }

                            str[len++] = 'e';
                            if (exp < 0) {
                                str[len++] = '-';
                                exp = -exp;
                            } else {
                                str[len++] = '+';
                            }

                            if (exp >= 100) {
                                S32 c = exp % 10;
                                memcpy(str + len, RYU_DIGIT_TABLE + 2 * (exp / 10), 2);
                                str[len + 2] = (Char) ('0' + c);
                                len += 3;
                            } else {
                                memcpy(str + len, RYU_DIGIT_TABLE + 2 * exp, 2);
                                len += 2;
                            }

                        }
#  endif
                    }

                    precision = 0;
                } break;

                case '%': {
                    str = num_str;
                    str[0] = '%';
                    len = 1;
                    precision = 0;
                } break;

                default: { // unknown, just copy code
                    str = num_str;

                    while (format_pointer <= fmt) {
                        str[0] = format_pointer[0];
                        format_pointer++;
                        str++;
                    }

                    len = (Size)(Dptr)(str - num_str);
                    str = num_str;
                    precision = 0;
                } break;
            }

            Size head_size = head_index;
            head_index = 0;
            Size tail_size = tail_index;
            tail_index = 0;

            // get field_width=leading/trailing space, precision=leading zeros
            if ((Size)precision < len) {
                precision = (Sint)len;
            }

            Sint zeros_head_tail = precision + (Sint)head_size + (Sint)tail_size + trailing_zeroes;
            if (field_width < zeros_head_tail) {
                field_width = zeros_head_tail;
            }

            field_width -= zeros_head_tail;
            precision -= len;

            // handle right justify and leading zeros
            if ((flags & Print_Flags_LEFT_JUSTIFIED) == 0) {
                if (flags & Print_Flags_LEADING_ZERO) { // then everything is in precision
                    precision = (field_width > precision) ? field_width : precision;
                    field_width = 0;
                }
            }

            // copy leading spaces
            if ((field_width + precision) > 0) {
                // copy leading spaces (or when doing %8.4d stuff)
                if ((flags & Print_Flags_LEFT_JUSTIFIED) == 0) {
                    for (Size j = 0; j < (Size)field_width; j++) {
                        if (buffer != NULL) {
                            buf[0] = ' ';
                        }
                        buf++;
                    }
                }

                { // copy the head
                    for (Size j = 0; j < head_size; j++) {
                        if (buffer != NULL) {
                            buf[0] = head_str[head_index++];
                        }
                        buf++;
                    }
                }

                // TODO(naman): What is this doing?
                /* U32 c = 0; */
                /* c = cs >> 24; */
                /* cs &= 0xffffff; */
                /* cs = (fl & Print_Flags_THOUSAND_COMMA) ? ((stbsp__uint32)(c - ((pr + cs) % (c + 1)))) : 0; */
                { // copy leading zeros
                    for (Size j = 0; j < (Size)precision; j++) {
                        if (buffer != NULL) {
                            buf[0] = '0';
                        }
                        buf++;
                    }
                }
            }

            { // copy the head
                if (head_index < head_size) {
                    Size repeat = head_size - head_index;
                    for (Size j = 0; j < repeat; j++) {
                        if (buffer != NULL) {
                            buf[0] = head_str[head_index++];
                        }
                        buf++;
                    }
                }
            }

            { // copy the string
                for (Size j = 0; j < len; j++) {
                    if (buffer != NULL) {
                        buf[0] = str[0];
                    }
                    buf++;
                    str++;
                }
            }

            { // copy trailing zeroes
                for (Size j = 0; j < (Size)trailing_zeroes; j++) {
                    if (buffer != NULL) {
                        buf[0] = '0';
                    }
                    buf++;
                }
            }

            { // copy the tail
                for (Size j = 0; j < tail_size; j++) {
                    if (buffer != NULL) {
                        buf[0] = tail_str[tail_index++];
                    }
                    buf++;
                }
            }

            // handle the left justify
            if (flags & Print_Flags_LEFT_JUSTIFIED)
                if (field_width > 0) {
                    for (Size j = 0; j < (Size)field_width; j++) {
                        if (buffer != NULL) {
                            buf[0] = ' ';
                        }
                        buf++;
                    }
                }

            fmt++;
        } else if (fmt[0] =='\0') {
            break;
        }
    }

    if (buffer != NULL) {
        buf[0] = '\0';
    }
    buf++;

    return (Size)(Uptr)(buf - buffer);
}

#  if defined(OS_WINDOWS)

header_function
Size printConsole (Sint fd, Char *format, va_list ap)
{
    va_list ap1, ap2;
    va_copy(ap1, ap);
    va_copy(ap2, ap);

    Size buffer_size = printStringVarArg(NULL, format, ap1);
    Char *buffer = nlib_Malloc(buffer_size);
    printStringVarArg(buffer, format, ap2);

    HANDLE out_stream;
    if (fd == 1) {
        out_stream = GetStdHandle(STD_OUTPUT_HANDLE);
    } else {
        out_stream = GetStdHandle(STD_ERROR_HANDLE);
    }

    if ((out_stream != NULL) && (out_stream != INVALID_HANDLE_VALUE)) {
        DWORD written = 0;
        WriteConsoleA(out_stream, buffer, buffer_size, &written, NULL);
    }

    nlib_Dealloc(buffer);

    va_end(ap1);
    va_end(ap2);

    return buffer_size;
}

header_function
Size printDebugOutput (Char *format, va_list ap)
{
    va_list ap1, ap2;
    va_copy(ap1, ap);
    va_copy(ap2, ap);

    Size buffer_size = printStringVarArg(NULL, format, ap1);
    Char *buffer = nlib_Malloc(buffer_size);
    printStringVarArg(buffer, format, ap2);

    LPWSTR wcstr = unicodeWin32UTF16FromUTF8(buffer);
    OutputDebugStringW(wcstr);
    unicodeWin32UTF16Dealloc(wcstr);

    nlib_Dealloc(buffer);

    va_end(ap1);
    va_end(ap2);

    return buffer_size;
}

#  elif defined(OS_LINUX)

header_function
Size printConsole (Sint fd, Char *format, va_list ap)
{
    va_list ap1, ap2;
    va_copy(ap1, ap);
    va_copy(ap2, ap);

    Size buffer_size = printStringVarArg(NULL, format, ap1);
    Char *buffer = memAlloc(buffer_size);
    printStringVarArg(buffer, format, ap2);

#   if defined(NLIB_NO_LIBC)
    write(fd, buffer, buffer_size);
#   else
    if (fd == 1) {
        fputs(buffer, stdout);
    } else {
        fputs(buffer, stderr);
    }
#   endif

    memDealloc(buffer);

    va_end(ap1);
    va_end(ap2);

    return buffer_size;
}

#  endif

header_function
Size printString (Char *buffer, Char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    Size buffer_size = printStringVarArg(buffer, format, ap);
    va_end(ap);

    return buffer_size;
}

header_function
Size say (Char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    Size buffer_size = printConsole(1, format, ap);
    va_end(ap);

    return buffer_size;
}

header_function
Size sayv (Char *format, va_list ap)
{
    Size buffer_size = printConsole(1, format, ap);
    return buffer_size;
}

header_function
Size err (Char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    Size buffer_size = printConsole(2, format, ap);
    va_end(ap);

    return buffer_size;
}

header_function
Size errv (Char *format, va_list ap)
{
    Size buffer_size = printConsole(2, format, ap);
    return buffer_size;
}

#  if defined(NLIB_TESTS)

#   define CHECK_END(str) utTest(stringEqual(buf, (str)) || ((unsigned) ret == stringLength(str)))
#   define SPRINTF printString

#   define CHECK9(str, v1, v2, v3, v4, v5, v6, v7, v8, v9) do { Size ret = SPRINTF(buf, v1, v2, v3, v4, v5, v6, v7, v8, v9); CHECK_END(str); } while (0)
#   define CHECK8(str, v1, v2, v3, v4, v5, v6, v7, v8    ) do { Size ret = SPRINTF(buf, v1, v2, v3, v4, v5, v6, v7, v8    ); CHECK_END(str); } while (0)
#   define CHECK7(str, v1, v2, v3, v4, v5, v6, v7        ) do { Size ret = SPRINTF(buf, v1, v2, v3, v4, v5, v6, v7        ); CHECK_END(str); } while (0)
#   define CHECK6(str, v1, v2, v3, v4, v5, v6            ) do { Size ret = SPRINTF(buf, v1, v2, v3, v4, v5, v6            ); CHECK_END(str); } while (0)
#   define CHECK5(str, v1, v2, v3, v4, v5                ) do { Size ret = SPRINTF(buf, v1, v2, v3, v4, v5                ); CHECK_END(str); } while (0)
#   define CHECK4(str, v1, v2, v3, v4                    ) do { Size ret = SPRINTF(buf, v1, v2, v3, v4                    ); CHECK_END(str); } while (0)
#   define CHECK3(str, v1, v2, v3                        ) do { Size ret = SPRINTF(buf, v1, v2, v3                        ); CHECK_END(str); } while (0)
#   define CHECK2(str, v1, v2                            ) do { Size ret = SPRINTF(buf, v1, v2                            ); CHECK_END(str); } while (0)
#   define CHECK1(str, v1                                ) do { Size ret = SPRINTF(buf, v1                                ); CHECK_END(str); } while (0)

header_function
void printUnitTest (void)
{
    Char buf[1024];
    Sint n = 0;
//    double pow_2_75 = 37778931862957161709568.0;

    // integers
    CHECK4("a b     1", "%c %s     %d", 'a', "b", 1);
    CHECK2("abc     ", "%-8.3s", "abcdefgh");
    CHECK2("+5", "%+2d", 5);
    CHECK2("  6", "% 3d", 6);
    CHECK2("-7  ", "%-4d", -7);
    CHECK2("+0", "%+d", 0);
    CHECK3("     00003:     00004", "%10.5d:%10.5d", 3, 4);
    CHECK2("-100006789", "%d", -100006789);
    CHECK3("20 0020", "%u %04u", 20u, 20u);
    CHECK4("12 1e 3C", "%o %x %X", 10u, 30u, 60u);
    CHECK4(" 12 1e 3C ", "%3o %2x %-3X", 10u, 30u, 60u);
    CHECK4("012 0x1e 0X3C", "%#o %#x %#X", 10u, 30u, 60u);
    CHECK2("", "%.0x", 0);
    CHECK2("",  "%.0d", 0);  // glibc gives "" as specified by C99(?)
    CHECK3("33 555", "%d %ld", (short)33, 555l);
    CHECK4("2 -3 %.", "%zd %td %.", (S64)2, (Dptr)-3, 23);

    // floating-point numbers
    CHECK2("-3.000000", "%f", -3.0);
    CHECK2("-8.8888888800", "%.10f", -8.88888888);
    CHECK2("880.0888888800", "%.10f", 880.08888888);
    CHECK2("4.1", "%.1f", 4.1);
    CHECK2(" 0", "% .0F", 0.1);
    CHECK2("0.00", "%.2F", 1e-4);
    CHECK2("-5.20", "%+4.2f", -5.2);
    CHECK2("0.0       ", "%-10.1f", 0.);
    CHECK2("-0.000000", "%f", -0.);
    CHECK2("0.000001", "%F", 9.09834e-07);
    double pow_2_85 = 38685626227668133590597632.0;
    CHECK2("38685626227668133600000000.0", "%.1f", pow_2_85); //FIXME(naman): Upper bound
    CHECK2("0.000000500000000000000000", "%.24f", 5e-7);
    CHECK2("0.000000000000000020000000", "%.24f", 2e-17);
    CHECK3("0.0000000100 100000000", "%.10f %.0f", 1e-8, 1e+8);
    CHECK2("100056789.0", "%.1f", 100056789.0);
    CHECK4(" 1.23 %", "%*.*f %%", 5, 2, 1.23);

    CHECK2("-3.000000e+00", "%e", -3.0);
    CHECK2("4.1E+00", "%.1E", 4.1);
    CHECK2("-5.20e+00", "%+4.2e", -5.2);
    // TODO(naman): Implement these when proper float support is added
//    CHECK3("+0.3 -3", "%+g %+g", 0.3, -3.0);
//    CHECK2("4", "%.1G", 4.1);
//    CHECK2("-5.2", "%+4.2g", -5.2);
//    CHECK2("3e-300", "%g", 3e-300);
//    CHECK2("1", "%.0g", 1.2);
//    CHECK3(" 3.7 3.71", "% .3g %.3g", 3.704, 3.706);
//    CHECK3("2e-315:1e+308", "%g:%g", 2e-315, 1e+308);
//    CHECK2("0x1.0091177587f83p-1022", "%a", 2.23e-308);
//    CHECK2("-0X1.AB0P-5", "%.3A", -0X1.abp-5);

    CHECK3("Inf NaN", "%f %f", (F64)INFINITY, (F64)NAN);
    CHECK2("NaN", "%.1f", (F64)NAN);

    // %p
    CHECK2("0000000000000000", "%p", (void*) NULL);

    // %n
    CHECK3("aaa ", "%.3s %n", "aaaaaaaaaaaaa", &n);
    utTest(n == 4);

#if 0
    // snprintf
    assert(SNPRINTF(buf, 100, " %s     %d",  "b", 123) == 10);
    assert(strcmp(buf, " b     123") == 0);
    assert(SNPRINTF(buf, 100, "%f", pow_2_75) == 30);
    assert(strncmp(buf, "37778931862957161709568.000000", 17) == 0);
    n = SNPRINTF(buf, 10, "number %f", 123.456789);
    assert(strcmp(buf, "number 12") == 0);
    assert(n == 17);  // written vs would-be written bytes
    n = SNPRINTF(buf, 0, "7 chars");
    assert(n == 7);
    // stb_sprintf uses internal buffer of 512 chars - test longer string
    assert(SPRINTF(buf, "%d  %600s", 3, "abc") == 603);
    assert(strlen(buf) == 603);
    SNPRINTF(buf, 550, "%d  %600s", 3, "abc");
    assert(strlen(buf) == 549);
    assert(SNPRINTF(buf, 600, "%510s     %c", "a", 'b') == 516);

    // length check
    assert(SNPRINTF(NULL, 0, " %s     %d",  "b", 123) == 10);

    // ' modifier. Non-standard, but supported by glibc.
#if !USE_STB
    setlocale(LC_NUMERIC, "");  // C locale does not group digits
#endif
    CHECK2("1,200,000", "%'d", 1200000);
    CHECK2("-100,006,789", "%'d", -100006789);
#if !defined(_MSC_VER) || _MSC_VER >= 1600
    CHECK2("9,888,777,666", "%'lld", 9888777666ll);
#endif
    CHECK2("200,000,000.000000", "%'18f", 2e8);
    CHECK2("100,056,789", "%'.0f", 100056789.0);
    CHECK2("100,056,789.0", "%'.1f", 100056789.0);
#if USE_STB  // difference in leading zeros
    CHECK2("000,001,200,000", "%'015d", 1200000);
#else
    CHECK2("0000001,200,000", "%'015d", 1200000);
#endif

    // things not supported by glibc
#if USE_STB
    CHECK2("null", "%s", (char*) NULL);
    CHECK2("123,4abc:", "%'x:", 0x1234ABC);
    CHECK2("100000000", "%b", 256);
    CHECK3("0b10 0B11", "%#b %#B", 2, 3);
#if !defined(_MSC_VER) || _MSC_VER >= 1600
    CHECK4("2 3 4", "%I64d %I32d %Id", 2ll, 3, 4ll);
#endif
    CHECK3("1k 2.54 M", "%$_d %$.2d", 1000, 2536000);
    CHECK3("2.42 Mi 2.4 M", "%$$.2d %$$$d", 2536000, 2536000);

    // different separators
    stbsp_set_separators(' ', ',');
    CHECK2("12 345,678900", "%'f", 12345.6789);
#endif
#endif
}

#endif // defined(NLIB_TESTS)

#define NLIB_PRINT_H_INCLUDE_GUARD
#endif // defined(NLIB_PRINT_INTERFACE_ONLY)
