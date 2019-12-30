/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2019 Naman Dixit
 */

#if !defined(SPRINTF_H_INCLUDE_GUARD)

typedef enum Print_Flags {
    Print_Flags_LEFT_JUSTIFIED = 1u << 0,
    Print_Flags_LEADING_PLUS = 1u << 1,
    Print_Flags_LEADING_SPACE = 1u << 2,
    Print_Flags_LEADING_ZERO = 1u << 4,
    Print_Flags_INTMAX = 1u << 5,
    Print_Flags_NEGATIVE = 1u << 7,
} Print_Flags;

header_function
Size vsprintfcb (Char *buffer, Char *format, va_list va)
{
    Sint tlen = 0;
    Char *fmt = format;
    Char *buf = buffer;

    while (true) {
        Uint field_width = 0;
        Uint precision = 0;
        Sint tz;
        U32 flags = 0;

        fw = 0;
        pr = -1;
        tz = 0;

        // Copy everything up to the next % (or end of string)
        while ((fmt[0] != '%') && (fmt[0] != '\0')) {
            buf[0] = fmt[0];
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
                    case '+': { // if we have leading plus
                        flags |= Print_Flags_LEADING_PLUS;
                        fmt++;
                        continue;
                    } break;
                    case ' ': { // if we have leading space
                        flags |= Print_Flags_LEADING_SPACE;
                        fmt++;
                        continue;
                    } break;
                    case '0': { // if we have leading zero
                        flags |= Print_Flags_LEADINGZERO;
                        fmt++;
                        goto flags_done;
                    };
                    default: {
                        goto flags_done;
                    } break;
                }
            }
        flags_done:

            // get the field width
            if (f[0] == '*') {
                field_width = va_arg(va, Uint);
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
                    precision = va_arg(va, Uint);
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
                    flags |= Print_Flags_INTMAX;
                    fmt++;
                } break;
                case 'z': { // are we 64-bit on size_t?
                    flags |= (sizeof(Size) == 8) ? Print_Flags_INTMAX : 0;
                    fmt++;
                } break;
                case 't': { // are we 64-bit on ptrdiff_t?
                    flags |= (sizeof(Dptr) == 8) ? Print_Flags_INTMAX : 0;
                    fmt++;
                } break;
                default: {
                } break;
            }

#define PRINT_SIZE 512 // big enough for e308 (with commas) or e-307
            Char num_str[PRINT_SIZE];
            Char *str = NULL;
            Size len = 0;

            switch (fmt[0]) { // handle each replacement
                case 's': { // string
                    // get the string
                    s = va_arg(va, Char*);
                    if (s == NULL)
                        s = "null";
                    // get the length
                    while (s[len] != '\0') {
                        len++;
                    }

                    // clamp to precision
                    if (len > precision)
                        len = precision;
                } break;

                case 'c': { // char
                    // get the character
                    s = num_str + PRINT_SIZE - 1;
                    *s = (Char)va_arg(va, Sint);
                    len = 1;
                } break;

                case 'n': { // weird write-bytes specifier
                    Sint *d = va_arg(va, Sint*);
                    *d = (Sint)(buf - buffer);
                } break;

                case 'b': { // binary
                    U64 num = 0;
                    if (flags & Print_Flags_INTMAX) {
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

                    len = ((num_str + PRINT_SIZE) - str);
                } break;

                case 'o': { // octal
                    U64 num = 0;
                    if (flags & Print_Flags_INTMAX) {
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

                    len = ((num_str + PRINT_SIZE) - str);
                } break;

                case 'p': { // pointer
                    flags |= (sizeof(void*) == 8) ? Print_Flags_INTMAX : 0;
                    precision = sizeof(void*) * 2;

                    U64 num = 0;
                    if (flags & Print_Flags_INTMAX) {
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

                    len = ((num_str + PRINT_SIZE) - str);
                } break
                    // fall through - to X

                case 'X':
                case 'x': { // hex
                    B32 upper = (f[0] == 'X') ? true : false;

                    U64 num = 0;
                    if (flags & Print_Flags_INTMAX) {
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

                    len = ((num_str + PRINT_SIZE) - str);
                } break;

                case 'u':
                case 'd': { // integer
                    // get the integer and abs it
                    U64 num = 0;
                    if (flags & Print_Flags_INTMAX) {
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
                            n64 = (U32)-i;
                            flags |= Print_Flags_NEGATIVE;
                        }
                    }

                    // convert to string
                    str = num_str + PRINT_SIZE;
                    len = 0;

                    U64 num_dec = num;
                    U32 n = 0;

                    while (num_dec) {
                        str--;
                        *s = (Char)(num_dec % 10) + '0';
                        num_dec /= 10;
                    }

                    // get the length that we copied
                    len = ((num_str + PRINT_SIZE) - str);

                    if (len == 0) {
                        --str;
                        *str = '0';
                        len = 1;
                    }

                    if (flags & Print_Flags_NEGATIVE) {
                        str--;
                        *str = '-';
                        len++;
                    } else if (flags & Print_Flags_LEADING_SPACE) {
                        str--;
                        *str = ' ';
                        len++;
                    } else if (flags & Print_Flags_LEADING_PLUS) {
                        str--;
                        *str = '+';
                        len++;
                    }

                    if (precision < 0) {
                        precision = 0;
                    }
                } break;

                default: { // unknown, just copy code
                    str = num_str + PRINT_SIZE - 1;
                    *str = fmt[0];
                    len = 1;
                } break;
            }

            // get field_width=leading/trailing space, precision=leading zeros
            if (precision < len) {
                precision = len;
            }

            U32 n = precision; // TODO(naman): + tz (related to floats)
            if (field_width < n) {
                field_width = n;
            }

            field_width -= n;
            precision -= len;

            // handle right justify and leading zeros
            if ((flags & Print_Flags_LEFT_JUSTIFIED) == 0) {
                if (flags & Print_Flags_LEADING_ZERO) { // then everything is in precision
                    precision = (field_width > precision) ? field_width : precision;
                    field_width = 0;
                }
            }

            // copy the spaces and/or zeros
            if (field_width + precision) {
                S32 i = 0;
                U32 c = 0;

                // copy leading spaces (or when doing %8.4d stuff)
                if ((flags & Print_Flags_LEFT_JUSTIFIED) == 0)
                    while (field_width > 0) {
                        stbsp__cb_buf_clamp(i, fw);
                        fw -= i;
                        while (i) {
                            if ((((stbsp__uintptr)bf) & 3) == 0)
                                break;
                            *bf++ = ' ';
                            --i;
                        }
                        while (i >= 4) {
                            *(stbsp__uint32 *)bf = 0x20202020;
                            bf += 4;
                            i -= 4;
                        }
                        while (i) {
                            *bf++ = ' ';
                            --i;
                        }
                        stbsp__chk_cb_buf(1);
                    }

                // copy leader
                sn = lead + 1;
                while (lead[0]) {
                    stbsp__cb_buf_clamp(i, lead[0]);
                    lead[0] -= (char)i;
                    while (i) {
                        *bf++ = *sn++;
                        --i;
                    }
                    stbsp__chk_cb_buf(1);
                }

                // copy leading zeros
                c = cs >> 24;
                cs &= 0xffffff;
                cs = (fl & Print_Flags_THOUSAND_COMMA) ? ((stbsp__uint32)(c - ((pr + cs) % (c + 1)))) : 0;
                while (pr > 0) {
                    stbsp__cb_buf_clamp(i, pr);
                    pr -= i;
                    if ((fl & Print_Flags_THOUSAND_COMMA) == 0) {
                        while (i) {
                            if ((((stbsp__uintptr)bf) & 3) == 0)
                                break;
                            *bf++ = '0';
                            --i;
                        }
                        while (i >= 4) {
                            *(stbsp__uint32 *)bf = 0x30303030;
                            bf += 4;
                            i -= 4;
                        }
                    }
                    while (i) {
                        if ((fl & Print_Flags_THOUSAND_COMMA) && (cs++ == c)) {
                            cs = 0;
                            *bf++ = stbsp__comma;
                        } else
                            *bf++ = '0';
                        --i;
                    }
                    stbsp__chk_cb_buf(1);
                }
            }

            // copy leader if there is still one
            sn = lead + 1;
            while (lead[0]) {
                stbsp__int32 i;
                stbsp__cb_buf_clamp(i, lead[0]);
                lead[0] -= (char)i;
                while (i) {
                    *bf++ = *sn++;
                    --i;
                }
                stbsp__chk_cb_buf(1);
            }

            // copy the string
            n = l;
            while (n) {
                stbsp__int32 i;
                stbsp__cb_buf_clamp(i, n);
                n -= i;
                Print_Flags_UNALIGNED(while (i >= 4) {
                        *(stbsp__uint32 *)bf = *(stbsp__uint32 *)s;
                        bf += 4;
                        s += 4;
                        i -= 4;
                    })
                    while (i) {
                        *bf++ = *s++;
                        --i;
                    }
                stbsp__chk_cb_buf(1);
            }

            // copy trailing zeros
            while (tz) {
                stbsp__int32 i;
                stbsp__cb_buf_clamp(i, tz);
                tz -= i;
                while (i) {
                    if ((((stbsp__uintptr)bf) & 3) == 0)
                        break;
                    *bf++ = '0';
                    --i;
                }
                while (i >= 4) {
                    *(stbsp__uint32 *)bf = 0x30303030;
                    bf += 4;
                    i -= 4;
                }
                while (i) {
                    *bf++ = '0';
                    --i;
                }
                stbsp__chk_cb_buf(1);
            }

            // copy tail if there is one
            sn = tail + 1;
            while (tail[0]) {
                stbsp__int32 i;
                stbsp__cb_buf_clamp(i, tail[0]);
                tail[0] -= (char)i;
                while (i) {
                    *bf++ = *sn++;
                    --i;
                }
                stbsp__chk_cb_buf(1);
            }

            // handle the left justify
            if (fl & Print_Flags_LEFTJUST)
                if (fw > 0) {
                    while (fw) {
                        stbsp__int32 i;
                        stbsp__cb_buf_clamp(i, fw);
                        fw -= i;
                        while (i) {
                            if ((((stbsp__uintptr)bf) & 3) == 0)
                                break;
                            *bf++ = ' ';
                            --i;
                        }
                        while (i >= 4) {
                            *(stbsp__uint32 *)bf = 0x20202020;
                            bf += 4;
                            i -= 4;
                        }
                        while (i--)
                            *bf++ = ' ';
                        stbsp__chk_cb_buf(1);
                    }
                }

            ++f;
        }
    }
endfmt:

    if (!callback)
        *bf = 0;
    else
        stbsp__flush_cb();

done:
    return tlen + (int)(bf - buf);
}

#define SPRINTF_H_INCLUDE_GUARD
#endif
