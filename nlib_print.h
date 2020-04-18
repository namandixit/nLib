/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2019 Naman Dixit
 */

// Takes 12151 bytes (11.86 KiB) without the unit test
// Command: readelf -s test.linux.x64|perl -ne 'if(/(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/) { print $3 . " " . $8. "\n";}'|sort -n | grep -i print

#if !defined(NLIB_PRINT_H_INCLUDE_GUARD)

# if !defined(PRINT_TEST_ONLY)

#  if 0
#   define p(...) printf(__VA_ARGS__)
#  else
#   define p(...)
#  endif

typedef enum Print_Flags {
    Print_Flags_LEFT_JUSTIFIED     = 1u << 0,
    Print_Flags_ALTERNATE_FORM     = 1u << 1,
    Print_Flags_LEADING_PLUS       = 1u << 2,
    Print_Flags_LEADING_SPACE      = 1u << 3,
    Print_Flags_LEADING_ZERO       = 1u << 4,
    Print_Flags_INT64              = 1u << 5,
    Print_Flags_NEGATIVE           = 1u << 6,
    Print_Flags_FLOAT_NO_LOW_BOUND = 1u << 7,
} Print_Flags;

header_function
Size printStringVarArg (Char *buffer, Char *format, va_list va)
{
    Char *fmt = format;
    Char *buf = buffer;

    while (true) {
        Sint field_width = 0;
        Sint precision = -1;
        Sint trailing_zeroes = 0;
        U32 flags = 0;

        // Copy everything up to the next % (or end of string)
        while ((fmt[0] != '%') && (fmt[0] != '\0')) {
            if (buffer != NULL) {
                buf[0] = fmt[0];
            }
            fmt++;
            buf++;
        }

        if (fmt[0] == '%') {
            fmt++;

            // read the modifiers first

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

# define PRINT_SIZE 512ULL // big enough for e308 (with commas) or e-307
            Char num_str[PRINT_SIZE];
            Char *str = NULL;

            Char head_str[8] = {0};
            Size head_index = 0;

//            Char tail_str[8] = {0};
//            Size tail_index = 0;

            Size len = 0;

            switch (fmt[0]) { // handle each replacement
                case 's': { // string
                    // get the string
                    str = va_arg(va, Char*);
                    if (str == NULL)
                        str = "null";
                    // get the length
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
                    str = num_str + PRINT_SIZE - 1;
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
                    str = num_str + PRINT_SIZE;

                    while (true) {
                        U64 n = num_dec & 0x1;
                        num_dec = num_dec >> 1;

                        str--;
                        *str = (n == 1) ? '1' : '0';

                        if ((num_dec != 0) || (((num_str + PRINT_SIZE) - str) < precision)) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    len = (((Uptr)num_str + PRINT_SIZE) - (Uptr)str);

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
                    str = num_str + PRINT_SIZE;

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

                        if ((num_dec != 0) || (((num_str + PRINT_SIZE) - str) < precision)) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    len = (((Uptr)num_str + PRINT_SIZE) - (Uptr)str);

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
                    str = num_str + PRINT_SIZE;

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

                        if ((num_dec != 0) || (((num_str + PRINT_SIZE) - str) < precision)) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    len = (((Uptr)num_str + PRINT_SIZE) - (Uptr)str);

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
                    str = num_str + PRINT_SIZE;

                    if ((num == 0) && (precision == 0)) {
                        break;
                    }

                    while (num_dec) {
                        str--;
                        *str = (Char)(num_dec % 10) + '0';
                        num_dec /= 10;
                    }

                    // get the length that we copied
                    len = (((Uptr)num_str + PRINT_SIZE) - (Uptr)str);

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
                    str = num_str + PRINT_SIZE;

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

                        if ((num_dec != 0) || (((num_str + PRINT_SIZE) - str) < precision)) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    len = (((Uptr)num_str + PRINT_SIZE) - (Uptr)str);
                } break;

                case 'F': { // Only upper bound for scientific notation
                    flags |= Print_Flags_FLOAT_NO_LOW_BOUND;
                } // fallthrough

                case 'f': { // floating-point number
                    if (precision < 0) {
                        precision = 6;
                    }

                    F64 f64 = va_arg(va, F64);
                    union FU64 { F64 f; U64 u;} fu64 = {.f = f64};
                    U64 bits = fu64.u;

                    /* NOTE(naman): 64-bit IEEE-754 Floating Point structure
                       ____________________________________________
                       |  Sign bit | Exponent bits | Mantissa bits|
                       |     1     |       11      |      52      |
                       --------------------------------------------
                    */

# define            MANTISSA_BITS 52
# define            EXPONENT_BITS 11
# define            BIAS 1023

                    // Is the top bit set?
                    B32 negative  = ((bits >> (MANTISSA_BITS + EXPONENT_BITS)) & 1) != 0;
                    // Remove all mantissa bits ------------------ and then reset the sign bit
                    U32 exponent_biased = (((U32)(bits >> MANTISSA_BITS)) &
                                           ((1U << EXPONENT_BITS) - 1U));
                    // Reset all bits except for the mantissa bits
                    U64 mantissa  = bits & ((1ULL << MANTISSA_BITS) - 1ULL);


                    if (negative) {
                        flags |= Print_Flags_NEGATIVE;
                    }

                    if (exponent_biased == 0x7FF) { // Handle NaN and Inf
                        str = num_str + PRINT_SIZE;

                        if (mantissa) {
                            *--str = 'N';
                            *--str = 'a';
                            *--str = 'N';
                        } else {
                            *--str = 'f';
                            *--str = 'n';
                            *--str = 'I';
                        }
                    }

                    if (str == NULL) {
                        F64 value = (f64 < 0) ? -f64 : f64;
                        B32 power_of_e_nonsense = false;
                        Sint exponent = 0;

                        { // Limit the "range" of float
                            // log10(2^64) = 19 = max integral F64 storable in U64 without
                            // loss of information
                            F64 positive_threshold = 1e19;

                            if (value >= positive_threshold) {
                                power_of_e_nonsense = true;
                                if (value >= 1e256) {
                                    value /= 1e256;
                                    exponent += 256;
                                }
                                if (value >= 1e128) {
                                    value /= 1e128;
                                    exponent += 128;
                                }
                                if (value >= 1e64) {
                                    value /= 1e64;
                                    exponent += 64;
                                }
                                if (value >= 1e32) {
                                    value /= 1e32;
                                    exponent += 32;
                                }
                                if (value >= 1e16) {
                                    value /= 1e16;
                                    exponent += 16;
                                }
                                if (value >= 1e8) {
                                    value /= 1e8;
                                    exponent += 8;
                                }
                                if (value >= 1e4) {
                                    value /= 1e4;
                                    exponent += 4;
                                }
                                if (value >= 1e2) {
                                    value /= 1e2;
                                    exponent += 2;
                                }
                                if (value >= 1e1) {
                                    value /= 1e1;
                                    exponent += 1;
                                }
                            }

                            if (!(flags & Print_Flags_FLOAT_NO_LOW_BOUND)) {
                                // 10^-(precision-1)
                                // (so that we get atleast one digit)
                                F64 negative_threshold = pow(10.0, 1.0 - (F64)precision);


                                if (value > 0 && value <= negative_threshold) {
                                    power_of_e_nonsense = true;
                                    if (value < 1e-255) {
                                        value *= 1e256;
                                        exponent -= 256;
                                    }
                                    if (value < 1e-127) {
                                        value *= 1e128;
                                        exponent -= 128;
                                    }
                                    if (value < 1e-63) {
                                        value *= 1e64;
                                        exponent -= 64;
                                    }
                                    if (value < 1e-31) {
                                        value *= 1e32;
                                        exponent -= 32;
                                    }
                                    if (value < 1e-15) {
                                        value *= 1e16;
                                        exponent -= 16;
                                    }
                                    if (value < 1e-7) {
                                        value *= 1e8;
                                        exponent -= 8;
                                    }
                                    if (value < 1e-3) {
                                        value *= 1e4;
                                        exponent -= 4;
                                    }
                                    if (value < 1e-1) {
                                        value *= 1e2;
                                        exponent -= 2;
                                    }
                                    if (value < 1e0) {
                                        value *= 1e1;
                                        exponent -= 1;
                                    }
                                }
                            }
                        }

                        U64 integral_part = (U64)value;
                        p("\tIntegral part: %lu\n", integral_part);
                        F64 remainder = value - integral_part;
                        p("\tRemainder: %.19f\n", remainder);

                        F64 remainder_modified = remainder * 1e19;
                        p("\tRemainder modified: %19f\n", remainder_modified);
                        U64 decimal_part = (U64)remainder_modified;

                        p("\tDecimal part: %lu, %llu\n", decimal_part, 1ULL << 63);

                        // rounding
                        remainder_modified -= decimal_part;
                        p("\tRemainder modified (rounded): %19f\n", remainder_modified);
                        if (remainder_modified >= 0.5) {
                            decimal_part++;
                            if (decimal_part >= 10000000000000000000ULL) { // 19 zeroes
                                decimal_part = 0;
                                integral_part++;
                                if (exponent != 0 && integral_part >= 10) {
                                    exponent++;
                                    integral_part = 1;
                                }
                            }
                        }
                        p("\tDecimal part (Rounded): %lu, %llu\n", decimal_part, 1ULL << 63);

                        str = num_str + PRINT_SIZE;

                        { // Write exponent (if needed)
                            if (power_of_e_nonsense) {
                                // convert to string
                                Sint num_dec = (exponent > 0) ? exponent : -exponent;

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

                                if (exponent < 0) {
                                    *--str = '-';
                                } else {
                                    *--str = '+';
                                }

                                *--str = 'e';
                            }
                        }

                        { // Write decimal part
                            if (precision > 0) {
                                Char tmp_buf[PRINT_SIZE] = {0};
                                Char *tmp = tmp_buf + PRINT_SIZE;
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

                                p("\tTEMP: %.*s\n",
                                  (int)((Uptr)tmp_buf + PRINT_SIZE - (Uptr)tmp),
                                  tmp);

                                if ((Uint)precision < width) {
                                    for (Char *temp = tmp_buf + PRINT_SIZE - 1;
                                         (Uptr)temp - (Uptr)tmp >= (Uint)precision;
                                         temp--) {
                                        Size index = 0;

                                        if ((temp[index] - '0') > 5) {
                                            B32 carry = false;
                                            do {
                                                p("\tTEMP CHAR: %c %c\n", temp[index], temp[index-1]);

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

                                p("\tTEMP: %.*s\n",
                                  (int)((Uptr)tmp_buf + PRINT_SIZE - (Uptr)tmp),
                                  tmp);

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
                    }

                    // get the length that we copied
                    len = (((Uptr)num_str + PRINT_SIZE) - (Uptr)str);

                    if (len == 0) {
                        *--str = '0';
                        len = 1;
                    }

                    if (flags & Print_Flags_NEGATIVE) {
                        head_str[head_index++] = '-';
                    } else if (flags & Print_Flags_LEADING_PLUS) {
                        head_str[head_index++] = '+';
                    } else if (flags & Print_Flags_LEADING_SPACE) {
                        if (!(flags & Print_Flags_NEGATIVE)) {
                            head_str[head_index++] = ' ';
                        }
                    }

                    if (flags & Print_Flags_LEADING_ZERO) {
                        head_str[head_index++] = '0';
                    }

                    precision = 0;

                    p("== %.*s\n", (int)len, str);
                } break;

                default: { // unknown, just copy code
                    str = num_str + PRINT_SIZE - 1;
                    *str = fmt[0];
                    len = 1;
                } break;
            }

            Size head_size = head_index;
            head_index = 0;
//            Size tail_size = tail_index;
//            tail_index = 0;

            // get field_width=leading/trailing space, precision=leading zeros
            if ((Size)precision < len) {
                precision = (Sint)len;
            }

            Sint zeros_head_tail = precision + (Sint)head_size; // TODO(naman): + trailing_zeroes (related to floats)
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

                // copy the head
                {
                    for (Size j = 0; j < head_size; j++) {
                        if (buffer != NULL) {
                            buf[0] = head_str[head_index++];
                        }
                        buf++;
                    }
                }

                // copy leading zeros
                // TODO(naman): What is this doing?
                /* U32 c = 0; */
                /* c = cs >> 24; */
                /* cs &= 0xffffff; */
                /* cs = (fl & Print_Flags_THOUSAND_COMMA) ? ((stbsp__uint32)(c - ((pr + cs) % (c + 1)))) : 0; */
                {
                    for (Size j = 0; j < (Size)precision; j++) {
                        if (buffer != NULL) {
                            buf[0] = '0';
                        }
                        buf++;
                    }
                }
            }

            // copy the head
            {
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

            // copy the string
            {
                for (Size j = 0; j < len; j++) {
                    if (buffer != NULL) {
                        buf[0] = str[0];
                    }
                    buf++;
                    str++;
                }
            }

            // copy trailing zeroes
            {
                for (Size j = 0; j < (Size)trailing_zeroes; j++) {
                    if (buffer != NULL) {
                        buf[0] = '0';
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

# if defined(OS_WINDOWS)

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

# elif defined(OS_LINUX)

header_function
Size printConsole (Sint fd, Char *format, va_list ap)
{
    va_list ap1, ap2;
    va_copy(ap1, ap);
    va_copy(ap2, ap);

    Size buffer_size = printStringVarArg(NULL, format, ap1);
    Char *buffer = nlib_Malloc(buffer_size);
    printStringVarArg(buffer, format, ap2);

    if (fd == 1) {
        fputs(buffer, stdout);
    } else {
        fputs(buffer, stderr);
    }

    nlib_Dealloc(buffer);

    va_end(ap1);
    va_end(ap2);

    return buffer_size;
}

# endif

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
Size err (Char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    Size buffer_size = printConsole(2, format, ap);
    va_end(ap);

    return buffer_size;
}

#endif // !PRINT_TEST_ONLY

#define NLIB_PRINT_H_INCLUDE_GUARD
#endif

# if defined(NLIB_TESTS) && defined(PRINT_TEST_ONLY)

#if !defined(NLIB_PRINT_TEST_INCLUDE_GUARD)

#define CHECK_END(str) utTest((strcmp(buf, (str)) == 0) || ((unsigned) ret == strlen(str)))

#define SPRINTF printString

#define CHECK9(str, v1, v2, v3, v4, v5, v6, v7, v8, v9) do { Size ret = SPRINTF(buf, v1, v2, v3, v4, v5, v6, v7, v8, v9); CHECK_END(str); } while (0)
#define CHECK8(str, v1, v2, v3, v4, v5, v6, v7, v8    ) do { Size ret = SPRINTF(buf, v1, v2, v3, v4, v5, v6, v7, v8    ); CHECK_END(str); } while (0)
#define CHECK7(str, v1, v2, v3, v4, v5, v6, v7        ) do { Size ret = SPRINTF(buf, v1, v2, v3, v4, v5, v6, v7        ); CHECK_END(str); } while (0)
#define CHECK6(str, v1, v2, v3, v4, v5, v6            ) do { Size ret = SPRINTF(buf, v1, v2, v3, v4, v5, v6            ); CHECK_END(str); } while (0)
#define CHECK5(str, v1, v2, v3, v4, v5                ) do { Size ret = SPRINTF(buf, v1, v2, v3, v4, v5                ); CHECK_END(str); } while (0)
#define CHECK4(str, v1, v2, v3, v4                    ) do { Size ret = SPRINTF(buf, v1, v2, v3, v4                    ); CHECK_END(str); } while (0)
#define CHECK3(str, v1, v2, v3                        ) do { Size ret = SPRINTF(buf, v1, v2, v3                        ); CHECK_END(str); } while (0)
#define CHECK2(str, v1, v2                            ) do { Size ret = SPRINTF(buf, v1, v2                            ); CHECK_END(str); } while (0)
#define CHECK1(str, v1                                ) do { Size ret = SPRINTF(buf, v1                                ); CHECK_END(str); } while (0)

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
    CHECK4("2 -3 %a", "%zd %td %a", (ssize_t)2, (ptrdiff_t)-3, 23);

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
    /* double pow_2_85 = 38685626227668133590597632.0;
    CHECK2("38685626227668133600000000.0", "%.1f", pow_2_85); */ //FIXME(naman): Upper bound
    CHECK2("0.000000500000000000000000", "%.24f", 5e-7);
    CHECK2("0.000000000000000020000000", "%.24f", 2e-17);
    CHECK3("0.0000000100 100000000", "%.10f %.0f", 1e-8, 1e+8);
    CHECK2("100056789.0", "%.1f", 100056789.0);
    CHECK4(" 1.23 %", "%*.*f %%", 5, 2, 1.23);

    // TODO(naman): Implement these when proper float support is added
    /* CHECK2("-3.000000e+00", "%e", -3.0); */
    /* CHECK2("4.1E+00", "%.1E", 4.1); */
    /* CHECK2("-5.20e+00", "%+4.2e", -5.2); */
    /* CHECK3("+0.3 -3", "%+g %+g", 0.3, -3.0); */
    /* CHECK2("4", "%.1G", 4.1); */
    /* CHECK2("-5.2", "%+4.2g", -5.2); */
    /* CHECK2("3e-300", "%g", 3e-300); */
    /* CHECK2("1", "%.0g", 1.2); */
    /* CHECK3(" 3.7 3.71", "% .3g %.3g", 3.704, 3.706); */
    /* CHECK3("2e-315:1e+308", "%g:%g", 2e-315, 1e+308); */
    /* CHECK2("0x1.0091177587f83p-1022", "%a", 2.23e-308); */
    /* CHECK2("-0X1.AB0P-5", "%.3A", -0X1.abp-5); */

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

#define NLIB_PRINT_TEST_INCLUDE_GUARD
#endif

#endif
