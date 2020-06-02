/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 * SPDX-License-Identifier: 0BSD OR BSL-1.0 OR Unlicense
 */

#if !defined(NLIB_STDINC_H_INCLUDE_GUARD)

/* This file contains functions whose existence is assumed by various compilers. This file is
 * only useful when trying to prevent a dependency on the platform libc.
 */

#if defined(OS_WINDOWS)

// TODO(naman): Add SDL_stdlib.h header's contents here:
// https://hg.libsdl.org/SDL/file/tip/src/stdlib/SDL_stdlib.c

#elif defined(OS_LINUX)

#  if defined(NLIB_NO_LIBC)
// GCC/Clang require these functions to exist in a freestanding environment
// These function were copied from https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/tools/include/nolibc/nolibc.h
#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wmissing-prototypes"
#   endif
void *memmove(void *dst, const void *src, Size len)
{
    SSize pos = (dst <= src) ? -1 : (S64)len;
    void *ret = dst;

    while (len--) {
        pos += (dst <= src) ? 1 : -1;
        ((Char*)dst)[pos] = ((Char*)src)[pos];
    }
    return ret;
}

void *memset(void *dst, S32 b, Size len)
{
    char *p = dst;

    while (len--)
        *(p++) = (Char)b;
    return dst;
}

S32 memcmp(const void *s1, const void *s2, Size n)
{
    size_t ofs = 0;
    char c1 = 0;

    while (ofs < n && !(c1 = ((Char *)s1)[ofs] - ((Char *)s2)[ofs])) {
        ofs++;
    }
    return c1;
}

void *memcpy(void *dst, const void *src, Size len)
{
    return memmove(dst, src, len);
}
#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic pop
#   endif
#  endif


#endif


#define NLIB_STDINC_H_INCLUDE_GUARD
#endif
