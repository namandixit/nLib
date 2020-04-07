/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2019 Naman Dixit
 */

#if !defined(NLIB_PRINT_H_INCLUDE_GUARD)

typedef enum Print_Flags {
    Print_Flags_LEFT_JUSTIFIED = 1u << 0,
    Print_Flags_LEADING_PLUS   = 1u << 1,
    Print_Flags_LEADING_SPACE  = 1u << 2,
    Print_Flags_LEADING_ZERO   = 1u << 3,
    Print_Flags_INTMAX         = 1u << 4,
    Print_Flags_NEGATIVE       = 1u << 5,
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
                    case '+': { // if we have leading plus
                        flags |= Print_Flags_LEADING_PLUS;
                        fmt++;
                        continue;
                    } break;
                    case ' ': { // if we have leading plus
                        flags |= Print_Flags_LEADING_ZERO;
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

                    len = (((Uptr)num_str + PRINT_SIZE) - (Uptr)str);

                    if (precision < 0) {
                        precision = 0;
                    }
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

                    len = (((Uptr)num_str + PRINT_SIZE) - (Uptr)str);

                    if (precision < 0) {
                        precision = 0;
                    }
                } break;

                case 'X':
                case 'x': { // hex
                    B32 upper = (fmt[0] == 'X') ? true : false;

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

                    len = (((Uptr)num_str + PRINT_SIZE) - (Uptr)str);

                    if (precision < 0) {
                        precision = 0;
                    }
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
                            num = (U32)-i;
                            flags |= Print_Flags_NEGATIVE;
                        }
                    }

                    // convert to string
                    str = num_str + PRINT_SIZE;
                    len = 0;

                    U64 num_dec = num;

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
                    } else if (flags & Print_Flags_LEADING_PLUS) {
                        head_str[head_index++] = '+';
                    } else if (flags & Print_Flags_LEADING_ZERO) {
                        head_str[head_index++] = ' ';
                    }

                    if (precision < 0) {
                        precision = 0;
                    }
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

                    len = (((Uptr)num_str + PRINT_SIZE) - (Uptr)str);
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

            Sint zeros_head_tail = precision; // TODO(naman): + trailing_zeroes (related to floats)
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
            if (field_width + precision) {
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
                    for (Size j = 0; j < (head_size - head_index); j++) {
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

#define NLIB_PRINT_H_INCLUDE_GUARD
#endif
