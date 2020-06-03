/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 * SPDX-License-Identifier: 0BSD OR BSL-1.0 OR Unlicense
 */

#if !defined(NLIB_STRING_H_INCLUDE_GUARD)

# if defined(NLIB_NO_LIBC)
header_function
Sint strcmp (const Char *s1, const Char *s2)
{
    while(s1[0]) {
        if (s1[0] != s2[0]) break;
        s1++;
        s2++;
    }

    return s1[0] - s2[0];
}

header_function
Sint strncmp (const Char *s1, const Char *s2, Size count)
{
    Sint result = 0;

    for (Size i = 0; i < count; i++)
    {
        if (s1[i] != s2[i]) {
            result = (s1[i] < s2[i]) ? -1 : 1;
            break;
        }
    }

    return result;
}

header_function
Size strlen (const Char *s)
{
    Size len = 0;

    for (Size i = 0; s[i] != '\0'; i++) {
        len++;
    }

    len++;

    return len;
}
# endif

header_function
B32 streq (const Char *str1, const Char *str2)
{
    B32 result = (strcmp(str1, str2) == 0);
    return result;
}

header_function
B32 strneq (const Char *str1, const Char *str2, Size count)
{
    B32 result = (strncmp(str1, str2, count) == 0);
    return result;
}

header_function
Size strprefix (Char *str, Char *pre)
{
    Size lenpre = strlen(pre);
    Size lenstr = strlen(str);

    if (lenstr < lenpre) {
        return 0;
    } else {
        if (strneq(pre, str, lenpre)) {
            return lenpre;
        } else {
            return 0;
        }
    }
}

header_function
Size strsuffix (Char *str, Char *suf)
{
    Size lensuf = strlen(suf);
    Size lenstr = strlen(str);

    if (lenstr < lensuf) {
        return 0;
    } else {
        if (strneq(suf, str + (lenstr - lensuf), lensuf)) {
            return lensuf;
        } else {
            return 0;
        }
    }
}

#define NLIB_STRING_H_INCLUDE_GUARD
#endif
