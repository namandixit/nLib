/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
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
void *memmove(void *dst, void *src, size_t len)
{
    ssize_t pos = (dst <= src) ? -1 : (long)len;
    void *ret = dst;

    while (len--) {
        pos += (dst <= src) ? 1 : -1;
        ((char *)dst)[pos] = ((char *)src)[pos];
    }
    return ret;
}

void *memset(void *dst, int b, size_t len)
{
    char *p = dst;

    while (len--)
        *(p++) = (char)b;
    return dst;
}

int memcmp(void *s1, void *s2, size_t n)
{
    size_t ofs = 0;
    char c1 = 0;

    while (ofs < n && !(c1 = ((char *)s1)[ofs] - ((char *)s2)[ofs])) {
        ofs++;
    }
    return c1;
}

void *memcpy(void *dst, void *src, size_t len)
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
