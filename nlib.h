/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2018 Naman Dixit
 * SPDX-License-Identifier: 0BSD OR BSL-1.0 OR Unlicense
 */

#if !defined(NLIB_H_INCLUDE_GUARD)

/* ****************************************************************************
 * COMMON *********************************************************************
 */

/* ===============
 * Platform Identification
 */

# if defined(_MSC_VER)
#  if defined(__clang__)
#   define COMPILER_CLANG
#   define COMPILER_CLANG_WITH_MSVC
#  else
#   define COMPILER_MSVC
#  endif
# elif defined (__GNUC__)
#  if defined(__clang__)
#   define COMPILER_CLANG
#   define COMPILER_CLANG_WITH_GCC
#  else
#   define COMPILER_GCC
#  endif
# elif defined(__clang__)
#  define COMPILER_CLANG
# else
#  error Compiler not supported
# endif

# if defined(_WIN32)
#  define OS_WINDOWS
# elif defined(__linux__)
#  define OS_LINUX
# else
#  error Operating system not supported
# endif

# if defined(COMPILER_MSVC) || defined(COMPILER_CLANG_WITH_MSVC)
#  if defined(_M_IX86)
#   define ARCH_X86
#  elif defined(_M_X64)
#   define ARCH_X64
#  endif
# elif defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#  if defined(__i386__)
#   define ARCH_X86
#  elif defined(__x86_64__)
#   define ARCH_X64
#  endif
# endif

# if !defined(ARCH_X64)  // && !defined(ARCH_X86)
#  error Architecture not supported
# endif

# if defined(ARCH_X86)
#  define BITWIDTH_32
# elif defined(ARCH_X64)
#  define BITWIDTH_64
# else
#  error "Bitwidth not supported"
# endif

# if defined(COMPILER_MSVC)
#  if !defined(__cplusplus) // TODO(naman): See if this is actually works and is the best way.
#   define LANG_C       // TODO(naman): Update when Microsoft gets off its ass.
#  else
#   define LANG_CPP
#  endif
# endif

# if defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#  if defined(__STDC_VERSION__)
#   if (__STDC_VERSION__ == 199409)
#    define LANG_C 1989
#   elif (__STDC_VERSION__ == 199901)
#    define LANG_C 1999
#   elif (__STDC_VERSION__ == 201112) || (__STDC_VERSION__ == 201710)
#    define LANG_C 2011
#   else
#    define LANG_C
#   endif
#  elif defined(__cplusplus)
#   if __cplusplus == 199711L
#    define LANG_CPP 1998
#   elif __cplusplus == 201103L
#    define LANG_CPP 2011
#   elif __cplusplus == 201402L
#    define LANG_CPP 2014
#   elif __cplusplus == 201703L
#    define LANG_CPP 2017
#   else
#    define LANG_CPP
#   endif
#  else
#   error Language not supported
#  endif
# endif

# if defined(OS_WINDOWS)
#  define ENDIAN_LITTLE
# else
#  include <endian.h>
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#   define ENDIAN_LITTLE
#  elif __BYTE_ORDER == __BIG_ENDIAN
#   define ENDIAN_BIG
#  else
#   error Can not determine endianness
#  endif
# endif

/* ===========================
 * Standard C Headers Includes
 */

/* NOTE(naman): Define NLIB_NO_LIBC to prevent the inclusion of libc functions.
*/

// NOTE(naman): These warnings are disabled permanently
# if defined(COMPILER_CLANG) && defined(LANG_C)
#  pragma clang diagnostic ignored "-Wgnu-statement-expression" // to use min/max
# endif

# if defined(COMPILER_MSVC)
#  pragma warning(push)
#   pragma warning(disable:4668)
# endif

# include <stddef.h>

# if defined(COMPILER_MSVC)
#  pragma warning(pop)
# endif

# include <stdint.h>
# include <inttypes.h>
# include <limits.h>
# include <stdarg.h>
# include <stdnoreturn.h>
# include <float.h>

# if defined(LANG_CPP)
extern "C" {
# endif

/* ===============
 * Primitive Types
 */

typedef int8_t               S8;
typedef int16_t              S16;
typedef int32_t              S32;
typedef int64_t              S64;
typedef int                  Sint;

typedef uint8_t              U8;
typedef uint16_t             U16;
typedef uint32_t             U32;
typedef uint64_t             U64;
typedef unsigned             Uint;

typedef size_t               Size;

typedef uintptr_t            Uptr;
typedef intptr_t             Sptr;
typedef ptrdiff_t            Dptr;

typedef float                F32;
typedef double               F64;

typedef U8                   B8;
typedef U16                  B16;
typedef U32                  B32;
typedef U64                  B64;
# if defined(LANG_C)
#  define true                1U
#  define false               0U
# endif
typedef unsigned char        Byte;
typedef char                 Char;

/* ========================
 * Preprocessor Definitions
 */

# define elemin(array) (sizeof(array)/sizeof((array)[0]))
# define containerof(ptr, type, member)                                  \
    ((type *)( ((Byte *)(true ? (ptr) : (type *)NULL)) - offsetof(type, member) ))

# define isOdd(x)  ((x) % 2 ? true : false)
# define isEven(x) (!isOdd(x))

# define KiB(x) (   (x) * 1024ULL)
# define MiB(x) (KiB(x) * 1024ULL)
# define GiB(x) (MiB(x) * 1024ULL)
# define TiB(x) (GiB(x) * 1024ULL)

# define THOUSAND 1000L
# define MILLION  1000000L
# define BILLION  1000000000L

# define unused_variable(var) (void)var

# define global_variable   static
# define global_immutable  static const
# define persistent_value  static

# define internal_function static
# define header_function   static inline

# if defined(LANG_C)
#  define nullptr NULL
# endif

/* =======================
 * Compiler Specific Hacks
 */

/* Microsoft Visual C */
# if defined(COMPILER_MSVC)

// #  if _MSC_VER != 1916 // Visual Studio 2017 Version 15.7
// #   error MSVC version not supported
// #  endif // MSVC Versions

#  define _Alignof __alignof
#  define alignof _Alignof
#  define __alignof_is_defined 1

/* In alignas(a), 'a' should be a power of two that is at least the type's
   alignment and at most the implementation's alignment limit.  This limit is
   2**13 on MSVC. To be portable to MSVC through at least version 10.0,
   'a' should be an integer constant, as MSVC does not support expressions
   such as 1 << 3.

   The following C11 requirements are not supported here:

   - If 'a' is zero, alignas has no effect.
   - alignas can be used multiple times; the strictest one wins.
   - alignas (TYPE) is equivalent to alignas (alignof (TYPE)).
*/

#  define _Alignas(a) __declspec(align(a))
#  define alignas _Alignas
#  define __alignas_is_defined 1

/* Malloc Alignment: https://msdn.microsoft.com/en-us/library/ycsb6wwf.aspx
 */
#  if defined(ARCH_x86)
// Alignment is 8 bytes
typedef union {
    alignas(8) Byte alignment[8];
    F64 a;
} max_align_t;
#  elif defined(ARCH_X64)
// Alignment is 16 bytes
typedef union {
    alignas(16) Byte alignment[16];
    alignas(16) struct { F64 a, b; } f;
} max_align_t;
#  endif

#  define thread_local __declspec(thread)

#  define swap_endian(x) _byteswap_ulong(x)

#  define fallthrough

# elif defined(COMPILER_CLANG) || defined(COMPILER_GCC)

#  if defined(COMPILER_GCC)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpedantic"
#  endif

#  if defined(COMPILER_CLANG) && defined(LANG_CPP)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wreserved-id-macro"
#  endif

#  if defined(LANG_C)
#   define max__paste(a, b) a##b
#   define max__impl(a, b, l) ({                         \
            __typeof__(a) max__paste(__a, l) = (a);     \
            __typeof__(b) max__paste(__b, l) = (b);     \
            max__paste(__a, l) > max__paste(__b, l) ?   \
                max__paste(__a, l) :                    \
                max__paste(__b, l);                     \
        })
#   define max(a, b) max__impl(a, b, __COUNTER__)

#   define min__paste(a, b) a##b
#   define min__impl(a, b, l) ({                         \
            __typeof__(a) min__paste(__a, l) = (a);     \
            __typeof__(b) min__paste(__b, l) = (b);     \
            min__paste(__a, l) < min__paste(__b, l) ?   \
                min__paste(__a, l) :                    \
                min__paste(__b, l);                     \
        })
#   define min(a, b) min__impl(a, b, __COUNTER__)
#  endif

#  if defined(COMPILER_CLANG) && defined(LANG_CPP)
#   pragma clang diagnostic pop
#  endif

#  if defined(COMPILER_GCC)
#   pragma GCC diagnostic pop
#  endif

#  if (defined(LANG_C) && LANG_C >= 2011) && !defined(NLIB_NO_LIBC)
#   include <stdalign.h>
#  elif defined(LANG_CPP) && LANG_CPP >= 2011
//  Don't do anything, stdalign.h's macros are keywords in the C++ language
#  else
#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wreserved-id-macro"
#   endif

#   define _Alignof __alignof__
#   define alignof _Alignof
#   define __alignof_is_defined 1
#   define _Alignas(a) __attribute__ ((aligned (a)))
#   define alignas _Alignas
#   define __alignas_is_defined 1

#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic pop
#   endif
#  endif

#  if ((defined(LANG_C) && LANG_C >= 2011) || (defined(LANG_CPP) && LANG_CPP >= 2011)) && !defined(NLIB_NO_LIBC)
#   include <threads.h>
#  else
#   define thread_local __thread
#  endif

// NOTE(naman): max_align_t is declared in stddef.h but only in C/C++ >= 11
#  if (defined(LANG_C) && LANG_C < 2011) || (defined(LANG_CPP) && LANG_CPP < 2011)
// Malloc Alignment: https://msdn.microsoft.com/en-us/library/ycsb6wwf.aspx
#   if defined(ARCH_x86)
  // Alignment is 8 bytes
typedef union {
    alignas(8) Byte alignment[8];
    F64 a;
} max_align_t;
#   elif defined(ARCH_X64)
// Alignment is 16 bytes
typedef union {
    alignas(16) Byte alignment[16];
    alignas(16) struct { F64 a, b; } f;
} max_align_t;
#   endif
#  endif

#  define swap_endian(x) __builtin_bswap32(x)

#  define fallthrough __attribute__((fallthrough))

# endif

/* ===========================
 * Platform-specific Headers
 */

# if defined(OS_WINDOWS)

#  if defined(COMPILER_MSVC)
#   pragma warning(push)
#    pragma warning(disable:4255)
#    pragma warning(disable:4668)
#  endif

#  include <Windows.h>

#  if defined(COMPILER_MSVC)
#   pragma warning(pop)
#  endif

#  if defined(COMPILER_MSVC)
#   pragma warning(push)
#    pragma warning(disable:4820)
#    pragma warning(disable:4668)
#    pragma warning(disable:4255)
#  endif

#  include <intrin.h>

#  if defined(COMPILER_MSVC)
#   pragma warning(pop)
#  endif

#  include "nlib_nolibc.h"

# elif defined(OS_LINUX)

#  if defined(NLIB_NO_LIBC)
#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wreserved-id-macro"
#   endif
#   include "nlib_linux.h"
#   include "nlib_nolibc.h"
#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic pop
#   endif
#  else // defined(NLIB_NO_LIBC)
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <sys/mman.h>
#   include <fcntl.h>
#   include <unistd.h>
#   include <string.h>
#   include <stdio.h>
#  endif
# endif

/* =======================
 * Other nlib libraries
 */

# include "nlib_bitbang.h"
# include "nlib_maths.h"
# include "nlib_string.h"

# define NLIB_PRINT_INTERFACE_ONLY
# include "nlib_print.h"
# undef NLIB_PRINT_INTERFACE_ONLY

# include "nlib_debug.h"
# include "nlib_memory.h"
# include "nlib_print.h"
# include "nlib_color.h"
# include "nlib_unicode.h"

/* ===============================
 * Random Number Generator
 */

# if defined(OS_WINDOWS)

/* Linear Congruential Generator
 *
 * If x is the last random number,
 *    m is a number greater than zero that is a power of two,
 *    a is a number between 0 and m,
 * then the next random number is ((x * a) % m).
 *
 * Unfortunately, the lower bits don't have enought randomness in them. The LSB doesn't
 * change at all, the second LSB alternates, the one after that toggles every 2 turns and so
 * on. Therefore, we try to get rid of the LSBs by pulling in some MSBs.
 *
 * NOTE(naman): Seed should be an odd number or the randomness might drop drastically.
 * NOTE(naman): "a" should be equal to 5(mod 8) or 3(mod 8).
 */

header_function
U64 randomU64 (U64 seed)
{
    U64 previous = seed;
    if (previous == 0) {
        // This seed has been tested and should be preferred in normal circumstances.
        previous = 2531011ULL;
    }

    U64 a = 214013ULL;

    U64 upper = 0, lower = 0;
    lower = _umul128(previous, a, &upper);
    U64 log_upper = upper ? mathLog2U64(upper) : 0;
    U64 shift_amount = 64 - (log_upper + 1);
    upper = (upper << shift_amount) | (lower >> log_upper);

    U64 result = upper * a;

    return result;
}

# elif defined(OS_LINUX)

/* Linear Congruential Generator
 *
 * If x is the last random number,
 *    m is a number greater than zero that is a power of two,
 *    a is a number between 0 and m,
 * then the next random number is ((x * a) % m).
 *
 * Unfortunately, the lower bits don't have enought randomness in them. The LSB doesn't
 * change at all, the second LSB alternates, the one after that toggles every 2 turns and so
 * on. Therefore, we try to get rid of the LSBs by pulling in some MSBs.
 *
 * We do the multiplcation twice because Chi-Square Test indicated that this method
 * gives better randomness. Don't ask.
 *
 * NOTE(naman): Seed should be an odd number or the randomness might drop drastically.
 * NOTE(naman): "a" should be equal to 5(mod 8) or 3(mod 8).
 */

header_function
U64 randomU64 (U64 seed)
{
    U64 previous = seed;
    if (previous == 0) {
        // This seed has been tested and should be preferred in normal circumstances.
        previous = 2531011ULL;
    }

    U64 a = 214013ULL;

    __uint128_t product = (__uint128_t)previous * (__uint128_t)a;
    U64 upper = product >> 64, lower = (U64)product;
    U64 log_upper = upper ? mathLog2U64(upper) : 0;
    U64 shift_amount = 64 - (log_upper + 1);
    upper = (upper << shift_amount) | (lower >> log_upper);

    U64 result = upper * a;

    return result;
}

# endif


/* ****************************************************************************
 * DATA STRUCTURES ************************************************************
 */

/* ==============
 * Strechy Buffer
 */

/* API ----------------------------------------
 * Size  sbufAdd       (T *ptr, T elem)
 * void  sbufDelete    (T *ptr)
 * T*    sbufEnd       (T *ptr)
 *
 * Size  sbufSizeof    (T *ptr)
 * Size  sbufElemin    (T *ptr)
 * Size  sbufMaxSizeof (T *ptr)
 * Size  sbufMaxElemin (T *ptr)
 */

# if defined(LANG_C)

#  define sbufMemoryAllocate(s) memVirtual(Memory_Allocator_Mode_ALLOCATE, s, NULL, NULL)
#  define sbufMemoryReallocate(p, s) memVirtual(Memory_Allocator_Mode_REALLOCATE, s, p, NULL)
#  define sbufMemoryDeallocate(p) memVirtual(Memory_Allocator_Mode_DEALLOCATE, 0, p, NULL)

typedef struct Sbuf_Header {
    Size cap; // NOTE(naman): Maximum number of elements that can be stored
    Size len; // NOTE(naman): Count of elements actually stored
    void *userdata;
    B64 fixed; // Can't use allocations/reallocation
    Byte buf[];
} Sbuf_Header;

#  define sbuf_GetHeader(sb) ((Sbuf_Header*)(void*)((Byte*)(sb) - offsetof(Sbuf_Header, buf)))

#  define sbuf_Len(sb)         ((sb) ? sbuf_GetHeader(sb)->len : 0U)
#  define sbuf_Cap(sb)         ((sb) ? sbuf_GetHeader(sb)->cap : 0U)

#  define sbufCreate(t, m, s)  sbuf_CreateFixed(sizeof(t), m, s)
// NOTE(naman): In sbufAdd, we check if there is enough space even after growing the sbuf
// to deal with fixed sbufs (since sbuf_Grow will simply return in that case).
// Also, if there wasn;t enough space, we return existing length just to avoid returning void
// which is not allowed by C99.
#  define sbufAdd(sb, ...)     ((sb) = sbuf_Grow((sb), sizeof(*(sb))),   \
                               ((sbuf_Len(sb) + 1) <= sbuf_Cap(sb) ?     \
                                ((sb)[sbuf_Len(sb)] = (__VA_ARGS__),    \
                                 ((sbuf_GetHeader(sb))->len)++) :        \
                                (claim(!(sbuf_GetHeader(sb)->fixed) &&   \
                                       "Fixed sbuf's space full"),      \
                                 (sbuf_GetHeader(sb))->len)))
#  define sbufDelete(sb)       ((sb) && !(sbuf_GetHeader(sb)->fixed) ?    \
                               (sbufMemoryDeallocate(sbuf_GetHeader(sb)), (sb) = NULL) : \
                               ((sb) ? ((sb) = NULL) : 0))
#  define sbufClear(sb)        ((sb) ?                                   \
                               (memset((sb), 0, sbufSizeof(sb)),        \
                                sbuf_GetHeader(sb)->len = 0) :          \
                               0)
#  define sbufResize(sb, n)    (((n) > sbufMaxElemin(sb)) ?              \
                               ((sb) = sbuf_Resize(sb, n, sizeof(*(sb)))) : \
                               0)

#  define sbufSizeof(sb)       (sbuf_Len(sb) * sizeof(*(sb)))
#  define sbufElemin(sb)       (sbuf_Len(sb))
#  define sbufMaxSizeof(sb)    (sbuf_Cap(sb) * sizeof(*(sb)))
#  define sbufMaxElemin(sb)    (sbuf_Cap(sb))
#  define sbufEnd(sb)          ((sb) + sbuf_Len(sb))

# define sbufPrint(sb, ...) ((sb) = sbuf_Print((sb), __VA_ARGS__))

# define sbufUnsortedRemove(sb, i) (((sb)[(i)] = (sb)[sbuf_Len(sb) - 1]), \
                                   ((sbuf_GetHeader(sb)->len)--))

#  if defined(COMPILER_CLANG)
#   pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wcast-align"
#  endif

header_function
void* sbuf_CreateFixed (Size elem_size, void *memory, Size memory_size)
{
    Sbuf_Header *header = (Sbuf_Header *)memory;

    header->cap = (memory_size - sizeof(*header)) / elem_size;
    header->len = 0;
    header->fixed = true;

    return header->buf;
}

header_function
void* sbuf_Grow (void *buf, Size elem_size)
{
    if ((buf != NULL) && sbuf_GetHeader(buf)->fixed) return buf;

    if ((sbuf_Len(buf) + 1) <= sbuf_Cap(buf)) {
        return buf;
    } else {
        Size new_cap = max(2 * sbuf_Cap(buf), 4U);

        Size new_size = (new_cap * elem_size) + memAlignUp(sizeof(Sbuf_Header));

        Sbuf_Header *new_header = NULL;

        if (buf != NULL) {
            new_header = (Sbuf_Header *)sbufMemoryReallocate(sbuf_GetHeader(buf), new_size);
        } else {
            new_header = (Sbuf_Header *)sbufMemoryAllocate(new_size);
            *new_header = (Sbuf_Header){.len = 0,
                                        .userdata = NULL};
        }

        new_header->cap = new_cap;

        return new_header->buf;
    }
}

header_function
void* sbuf_Resize (void *buf, Size elem_count, Size elem_size)
{
    if ((buf != NULL) && sbuf_GetHeader(buf)->fixed) return buf;

    Size new_cap = elem_count;

    Size new_size = (new_cap * elem_size) + sizeof(Sbuf_Header);

    Sbuf_Header *new_header = NULL;

    if (buf != NULL) {
        new_header = (Sbuf_Header *)sbufMemoryReallocate(sbuf_GetHeader(buf), new_size);
    } else {
        new_header = (Sbuf_Header *)sbufMemoryAllocate(new_size);
        *new_header = (Sbuf_Header){.cap = 0,
                                    .len = 0,
                                    .userdata = NULL};
    }

    new_header->cap = new_cap;

    return new_header->buf;
}

#  if defined(COMPILER_CLANG)
#   pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wformat-nonliteral"
#  endif
header_function
Char* sbuf_Print(Char *buf, Char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    Size size = printStringVarArg(NULL, fmt, args);
    va_end(args);

    sbufResize(buf, sbufElemin(buf) + size + 1);

    va_start(args, fmt);
    printStringVarArg(sbufEnd(buf), fmt, args);
    va_end(args);

    sbuf_GetHeader(buf)->len += size;
    return buf;
}
#  if defined(COMPILER_CLANG)
#   pragma clang diagnostic pop
#  endif

#  if defined(NLIB_TESTS)
header_function
void sbufUnitTest (void)
{
    S32 *buf = NULL;

    sbufAdd(buf, 42);

    utTest(buf != NULL);

    sbufAdd(buf, 1234);

    utTest(sbufElemin(buf) == 2);
    utTest(sbufMaxElemin(buf) >= sbufElemin(buf));

    utTest(buf[0] == 42);
    utTest(buf[1] == 1234);

    sbufDelete(buf);

    utTest(buf == NULL);

    Char *stream = NULL;
    sbufPrint(stream, "Hello, %s\n", "World!");
    sbufPrint(stream, "Still here? %d\n", 420);
    sbufPrint(stream, "GO AWAY!!!\n");

    utTest(streq(stream, "Hello, World!\nStill here? 420\nGO AWAY!!!\n"));
}
#  endif

#  if defined(COMPILER_CLANG)
#   pragma clang diagnostic pop
#  endif
# endif // defined(LANG_C)

/* ==========================
 * Interning
 *
 * Char* internString (Intern_String *is, Char *str)
 */

# if defined(LANG_C)
#  define INTERN_EQUALITY(func_name) B32 func_name (void *a, void *b, Size b_index)
typedef INTERN_EQUALITY(Intern_Equality_Function);

typedef struct Intern {
    struct Intern_List {
        Size *indices;
        U8 *secondary_hashes;
    } lists[256];
} Intern;

header_function
B32 internCheck (Intern *it, U8 hash1, U8 hash2,
                 void *datum, void *data, Intern_Equality_Function *eqf,
                 Size *result)
{
    if (it->lists[hash1].secondary_hashes != NULL) {
        // Our data has probably been inserted already.
        // (or atleast some data with same hash has been inserted :)
        for (Size i = 0;
             i < sbufElemin(it->lists[hash1].secondary_hashes);
             i++) {
            Size index = it->lists[hash1].indices[i];
            if ((it->lists[hash1].secondary_hashes[i] == hash2) && eqf(datum, data, index)) {
                // This is our data, return it
                if (result != NULL) {
                    *result = index;
                }
                return true;
            }
        }

        return false;
    } else {
        return false;
    }
}

header_function
void internData (Intern *it, U8 hash1, U8 hash2, Size index)
{
    sbufAdd(it->lists[hash1].secondary_hashes, hash2);
    sbufAdd(it->lists[hash1].indices, index);
    utTest(sbufElemin(it->lists[hash1].secondary_hashes) == sbufElemin(it->lists[hash1].indices));
}

header_function
U8 internStringPearsonHash (void *buffer, Size len, B32 which)
{
    // NOTE(naman): Pearson's hash for 8-bit hashing
    // https://en.wikipedia.org/wiki/Pearson_hashing
    persistent_value U8 hash_lookup_table1[256] =
        {
            // 0-255 shuffled in any (random) order suffices
            98,    6,   85, 150,  36,  23, 112, 164, 135, 207, 169,   5,  26,  64, 165, 219, // 01
            61,   20,   68,  89, 130,  63,  52, 102,  24, 229, 132, 245,  80, 216, 195, 115, // 02
            90,  168,  156, 203, 177, 120,   2, 190, 188,   7, 100, 185, 174, 243, 162,  10, // 03
            237,  18,  253, 225,   8, 208, 172, 244, 255, 126, 101,  79, 145, 235, 228, 121, // 04
            123, 251,   67, 250, 161,   0, 107,  97, 241, 111, 181,  82, 249,  33,  69,  55, // 05
            59,  153,   29,   9, 213, 167,  84,  93,  30,  46, 94,   75, 151, 114,  73, 222, // 06
            197,  96,  210,  45,  16, 227, 248, 202,  51, 152, 252, 125,  81, 206, 215, 186, // 07
            39,  158,  178, 187, 131, 136,   1,  49,  50,  17, 141,  91,  47, 129,  60,  99, // 08
            154,  35,   86, 171, 105,  34,  38, 200, 147,  58,  77, 118, 173, 246,  76, 254, // 09
            133, 232,  196, 144, 198, 124,  53,   4, 108,  74, 223, 234, 134, 230, 157, 139, // 10
            189, 205,  199, 128, 176,  19, 211, 236, 127, 192, 231,  70, 233,  88, 146,  44, // 11
            183, 201,   22,  83,  13, 214, 116, 109, 159,  32,  95, 226, 140, 220,  57,  12, // 12
            221,  31,  209, 182, 143,  92, 149, 184, 148,  62, 113,  65,  37,  27, 106, 166, // 13
            3,    14,  204,  72,  21,  41,  56,  66,  28, 193,  40, 217,  25,  54, 179, 117, // 14
            238,  87,  240, 155, 180, 170, 242, 212, 191, 163,  78, 218, 137, 194, 175, 110, // 15
            43,  119,  224,  71, 122, 142,  42, 160, 104,  48, 247, 103,  15,  11, 138, 239, // 16
        };

    persistent_value U8 hash_lookup_table2[256] =
        {
            251, 175, 119, 215,  81,  14,  79, 191, 103,  49, 181, 143, 186, 157,   0, 232, // 01
            31,   32,  55,  60, 152,  58,  17, 237, 174,  70, 160, 144, 220,  90,  57, 223, // 02
            59,    3,  18, 140, 111, 166, 203, 196, 134, 243, 124,  95, 222, 179, 197,  65, // 03
            180,  48,  36,  15, 107,  46, 233, 130, 165,  30, 123, 161, 209,  23,  97,  16, // 04
            40,   91, 219,  61, 100,  10, 210, 109, 250, 127,  22, 138,  29, 108, 244,  67, // 05
            207,   9, 178, 204,  74,  98, 126, 249, 167, 116,  34,  77, 193, 200, 121,   5, // 06
            20,  113,  71,  35, 128,  13, 182,  94,  25, 226, 227, 199,  75,  27,  41, 245, // 07
            230, 224,  43, 225, 177,  26, 155, 150, 212, 142, 218, 115, 241,  73,  88, 105, // 08
            39,  114,  62, 255, 192, 201, 145, 214, 168, 158, 221, 148, 154, 122,  12,  84, // 09
            82,  163,  44, 139, 228, 236, 205, 242, 217,  11, 187, 146, 159,  64,  86, 239, // 10
            195,  42, 106, 198, 118, 112, 184, 172,  87,   2, 173, 117, 176, 229, 247, 253, // 11
            137, 185,  99, 164, 102, 147,  45,  66, 231,  52, 141, 211, 194, 206, 246, 238, // 12
            56,  110,  78, 248,  63, 240, 189,  93,  92,  51,  53, 183,  19, 171,  72,  50, // 13
            33,  104, 101,  69,   8, 252,  83, 120,  76, 135,  85,  54, 202, 125, 188, 213, // 14
            96,  235, 136, 208, 162, 129, 190, 132, 156,  38,  47,   1,   7, 254,  24,   4, // 15
            216, 131,  89,  21,  28, 133,  37, 153, 149,  80, 170,  68,   6, 169, 234, 151, // 16
        };

    Char *string = buffer;

    U8 hash = (U8)len;
    for (Size i = 0; i < len; i++) {
        if (which == true) {
            hash = hash_lookup_table1[hash ^ string[i]];
        } else {
            hash = hash_lookup_table2[hash ^ string[i]];
        }
    }

    return hash;
}

typedef struct Intern_String {
    Intern intern;
    Char **strings;
} Intern_String;

header_function
INTERN_EQUALITY(internStringEquality) {
    Char *sa = a;
    Char **ss = b;
    Char *sb = ss[b_index];
    B32 result = streq(sa, sb);
    return result;
}

header_function
Char *internString (Intern_String *is, Char *str)
{
    U8 hash1 = internStringPearsonHash(str, strlen(str), true);
    U8 hash2 = internStringPearsonHash(str, strlen(str), false);

    Size index = 0;

    if (internCheck(&is->intern, hash1, hash2, str, is->strings, internStringEquality, &index)) {
        Char *result = is->strings[index];
        return result;
    } else {
        Size index_new = sbufElemin(is->strings);

        Char *str_new = NULL;
        for (Char *s = str; s[0] != '\0'; s++) {
            sbufAdd(str_new, s[0]);
        }
        sbufAdd(str_new, '\0');

        sbufAdd(is->strings, str_new);

        internData(&is->intern, hash1, hash2, index_new);
        Char *result = is->strings[index_new];
        return result;
    }
}

header_function
Char *internStringCheck (Intern_String *is, Char *str)
{
    U8 hash1 = internStringPearsonHash(str, strlen(str), true);
    U8 hash2 = internStringPearsonHash(str, strlen(str), false);

    Size index = 0;

    if (internCheck(&is->intern, hash1, hash2, str, is->strings, &internStringEquality, &index)) {
        Char *result = is->strings[index];
        return result;
    } else {
        return NULL;
    }
}

#  if defined(BUILD_DEBUG)

header_function
void internStringDebugPrint (Intern_String *is)
{
    for (Size i = 0; i < elemin(is->intern.lists); i++) {
        for (Size j = 0; j < sbufElemin(is->intern.lists[i].indices); j++) {
            report("%s\n", is->strings[is->intern.lists[i].indices[j]]);
        }
    }
}

#  endif

typedef struct Intern_Integer {
    Intern intern;
    U64 *integers;
} Intern_Integer;

header_function
INTERN_EQUALITY(internIntegerEquality) {
    U64 ia = ((U64*)a)[0];
    U64 ib = ((U64*)b)[b_index];

    B32 result = (ia == ib);
    return result;
}

// SEE(naman): https://stackoverflow.com/a/8546542
header_function
U8 internIntegerHash8Bit (U64 key, B32 which)
{
    U8 result = 0;
    U64 q = 0;

    // NOTE(naman): Both q are prime.
    if (which == true) {
        q = 33149;
    } else {
        q = 146519;
    }

    Byte *b = (Byte*)(&key);
    for (Size i = 0; i < sizeof(key); i++) {
        result += (U8)((U64)(b[i]) * q);
    }

    return result;
}

header_function
U64 internInteger (Intern_Integer *ii, U64 num)
{
    U8 hash1 = internIntegerHash8Bit(num, true);
    U8 hash2 = internIntegerHash8Bit(num, false);

    U64 num_copy = num; // Just in case
    Size index = 0;

    if (internCheck(&ii->intern, hash1, hash2,
                    &num_copy, ii->integers, &internIntegerEquality, &index)) {
        return num;
    } else {
        sbufAdd(ii->integers, num);
        internData(&ii->intern, hash1, hash2, sbufElemin(ii->integers));
        return num;
    }
}

header_function
U64 internIntegerCheck (Intern_Integer *ii, U64 num)
{
    U8 hash1 = internIntegerHash8Bit(num, true);
    U8 hash2 = internIntegerHash8Bit(num, false);

    U64 num_copy = num; // Just in case
    Size index = 0;

    if (internCheck(&ii->intern, hash1, hash2,
                    &num_copy, ii->integers, &internIntegerEquality, &index)) {
        return num;
    } else {
        return false;
    }
}

#  if defined(NLIB_TESTS)
header_function
void internUnitTest (void)
{
    Char x[] = "Hello";
    Char y[] = "Hello";

    Intern_String is = {0};

    utTest(x != y);

    Char *y_interned = internString(&is, y);
    Char *x_interned = internString(&is, x);
    utTest(x_interned == y_interned);

    Char z[] = "World";
    Char *z_interned = internString(&is, z);
    utTest(x_interned != z_interned);

    Char p[] = "Hello!!";
    Char *p_interned = internString(&is, p);
    utTest(x_interned != p_interned);

    // TODO(naman): Write tests to see what happens if two strings with same hash are interned.

    return;
}
#  endif
# endif // defined(LANG_C)

/* ==========================
 * Hashing Infrastructure
 */

// TODO(naman): Add hash collision detection

// FNV-1a
header_function
U64 hashString (Char *str)
{
    U64 hash = 0xCBF29CE484222325ULL; // FNV_offset_basis
    for (Size i = 0; str[i] != '\0'; i++) {
        hash = hash ^ (U64)str[i];
        hash = hash * 0x100000001B3ULL; // FNV_prime
    }

    claim(hash != 0);
    return hash;
}

// splitmix64 (xoshiro.di.unimi.it/splitmix64.c)
header_function
U64 hashInteger(U64 x)
{
    x ^= x >> 30;
    x *= 0xBF58476D1CE4E5B9ULL;
    x ^= x >> 27;
    x *= 0x94D049BB133111EBULL;
    x ^= x >> 31;

    claim(x != 0);
    return x;
}

/* Universal Hashing: https://en.wikipedia.org/wiki/Universal_hashing#Avoiding_modular_arithmetic
 *
 * NOTE(naman): Implementation notes
 * w is number of bits in machine word (64 in our case)
 * s is the number of buckets/bins (slots in the hash table) to which the
 *   universe of hashable objects is to be mapped
 * m is log2(s) (=> m = 2^s) and is equal to the number of bits in the final hash
 * a is a random odd positive integer < 2^w (fitting in w bits)
 * b is a random non-negative integer < 2^(w-m) (fitting in (w-m) bits)
 * SEE(naman): https://en.wikipedia.org/wiki/Universal_hashing#Avoiding_modular_arithmetic
 *
 * r is the last random number generated and is just an implementation detail.
 */

typedef struct Hash_Universal {
    U64 a, b, m; /* Hashing constants */

    U64 r; /* Last random number for Universal Hashing */
} Hash_Universal;

header_function
void hashUniversalConstantsUpdate (Hash_Universal *h)
{
    do {
        h->r = randomU64(h->r);
        h->a = h->r;
    } while ((h->a == 0) || ((h->a > 0) && ((h->a & 0x01) != 0x01))); // Make sure that 'a' is odd

    h->r = randomU64(h->r);
    // b should be (64 - m) bits long
    h->b = h->r & (0xFFFFFFFFFFFFFFFFULL >> h->m);
}

header_function
U64 hashUniversal (Hash_Universal h, U64 key)
{
    // NOTE(naman): Remember that we don't want 64-bit hashes, we want hashes < 2^m (s)
    U64 result =  ((h.a * key) + h.b) >> (64 - h.m);
    return result;
}

/* ==============
 * Hash Table
 */

/* API ---------------------------------------------------
 * NOTE(naman): Zero key and value are considered invalid.
 *
 * Hash_Table htCreate (Size min_slots);
 * void       htDelete (Hash_Table ht);
 * U64        htInsert (Hash_Table *ht, U64 key, U64 value);
 * U64        htLookup (Hash_Table *ht, U64 key);
 * U64        htRemove (Hash_Table *ht, U64 key);
 */

# if defined(LANG_C)

typedef struct Hash_Table {
    Hash_Universal univ;
    U64 *keys;
    U64 *values;
    Size slot_count;
    Size slot_filled;
    Memory_Allocator_Function *allocator;
    B64 fixed; // Use fixed memory
    U64 collision_count;
} Hash_Table;

#  define htMemoryAllocate(s)   memBuddy(Memory_Allocator_Mode_ALLOCATE, s, NULL, NULL)
#  define htMemoryDeallocate(p) memBuddy(Memory_Allocator_Mode_DEALLOCATE, 0, p, NULL)

header_function
Hash_Table ht_Create (Size slots_atleast, Memory_Allocator_Function *allocator,
                       void *memory, Size memory_size)
{
    claim((allocator == NULL) || (memory == NULL)); // Can't have fixed memory with allocator
    claim((memory == NULL) || (slots_atleast == 0)); // Can't have fixed memory without fixed slots

    Hash_Table ht = {0};

    if (memory) {
        // NOTE(naman): We are willing to waste some memory because not going overboard
        // is more important. However, the client code should try to provide
        // memory_size such that memory_size/16 is power of 2.
        Size slots = memory_size / (sizeof(*(ht.keys)) + sizeof(*(ht.values)));
        ht.slot_count = mathPrevPowerOf2U64(slots);
    } else {
        // NOTE(naman): We try to make the initial hash table a bit larger than expected.
        // The reason for this is that if we have only a small amount of elements, we would
        // just use a associative array instead of a hash table to do the lookup.
        Size slots = max(slots_atleast, 64U);
        ht.slot_count = mathNextPowerOf2U64(slots);
    }

    ht.univ.m = mathLog2U64(ht.slot_count);
    hashUniversalConstantsUpdate(&ht.univ);

    if (memory) {
        ht.fixed = true;
        ht.keys = memory;
        ht.values = (void*)((Byte*)memory + ht.slot_count * sizeof((ht.keys)));
    } else if (allocator) {
        ht.allocator = allocator;
        ht.keys     = allocator(Memory_Allocator_Mode_ALLOCATE,
                                ht.slot_count * sizeof(*(ht.keys)),
                                NULL, NULL);
        ht.values   = allocator(Memory_Allocator_Mode_ALLOCATE,
                                ht.slot_count * sizeof(*(ht.values)),
                                NULL, NULL);
    } else {
        ht.keys     = htMemoryAllocate(ht.slot_count * sizeof(*(ht.keys)));
        ht.values   = htMemoryAllocate(ht.slot_count * sizeof(*(ht.values)));
    }

    memset(ht.keys,   0, ht.slot_count * sizeof(*(ht.keys)));
    memset(ht.values, 0, ht.slot_count * sizeof(*(ht.values)));

    return ht;
}

header_function
Hash_Table htCreate (void)
{
    Hash_Table ht = ht_Create(0, NULL, NULL, 0);
    return ht;
}

header_function
Hash_Table htCreateWithSlots (Size slots)
{
    Hash_Table ht = ht_Create(slots, NULL, NULL, 0);
    return ht;
}

header_function
Hash_Table htCreateWithAllocator (Memory_Allocator_Function *allocator)
{
    Hash_Table ht = ht_Create(0, allocator, NULL, 0);
    return ht;
}

header_function
Hash_Table htCreateWithSlotsAndAllocator (Size slots, Memory_Allocator_Function *allocator)
{
    Hash_Table ht = ht_Create(slots, allocator, NULL, 0);
    return ht;
}

header_function
Hash_Table htCreateFixed (void *memory, Size memory_size)
{
    Hash_Table ht = ht_Create(0, NULL, memory, memory_size);
    return ht;
}

header_function
void htDelete (Hash_Table ht)
{
    if (ht.fixed) {
        claim(false && "Trying to free a fixed-memory hash table");
    } else if (ht.allocator) {
        ht.allocator(Memory_Allocator_Mode_DEALLOCATE, 0, ht.keys, NULL);
        ht.allocator(Memory_Allocator_Mode_DEALLOCATE, 0, ht.values, NULL);
    } else {
        htMemoryDeallocate(ht.keys);
        htMemoryDeallocate(ht.values);
    }
}

header_function
B32 ht_LinearProbeSearch (Hash_Table *ht, U64 key, U64 hash, Size *value)
{
    Size index = 0;
    B32 found = false;

    // TODO(naman): WE ARE SEARCHING EVERYTHING!!! WTF?????
    for (Size i = 0; !found && (i < ht->slot_count); ++i) {
        index = (hash + i) % (ht->slot_count);
        if (ht->keys[index] == key) {
            found = true;
            break;
        }
    }

    *value = index;

    return found;
}

header_function
U64 ht_LinearProbeInsertion (Hash_Table *ht,
                             U64 hash, U64 key, U64 value)
{
    U64 result_value = value;

    for (Size i = 0; i < ht->slot_count; ++i) {
        Size index = (hash + i) % (ht->slot_count);
        if ((ht->keys[index] == key) || (ht->values[index] == 0))  {
            result_value = ht->values[index];
            ht->keys[index] = key;
            ht->values[index] = value;
            break;
        } else {
            ht->collision_count++;
        }
    }

    return result_value;
}

header_function
U64 htInsert (Hash_Table *ht, U64 key, U64 value)
{
    if ((key == 0) || (value == 0)) return 0;

    if (ht->fixed) {
        if (ht->slot_filled >= ht->slot_count) {
            return 0;
        }
    } else if ((ht->collision_count > ht->slot_count) ||
               ((2 * ht->slot_filled) >= ht->slot_count)) {
        // FIXME(naman): Figure out the correct condition on which to resize on.
        Size slot_count_prev = ht->slot_count;
        U64 *keys   = ht->keys;
        U64 *values = ht->values;

        if ((2 * ht->slot_filled) >= ht->slot_count) { // Only increase size if need be
            ht->univ.m = ht->univ.m + 1;
            ht->slot_count = 1ULL << (ht->univ.m);
            hashUniversalConstantsUpdate(&(ht->univ));
        }

        if (ht->allocator) {
            ht->keys   = ht->allocator(Memory_Allocator_Mode_ALLOCATE,
                                  ht->slot_count * sizeof(*(ht->keys)),
                                  NULL, NULL);
            ht->values = ht->allocator(Memory_Allocator_Mode_ALLOCATE,
                                  ht->slot_count * sizeof(*(ht->values)),
                                  NULL, NULL);
        } else {
            ht->keys   = htMemoryAllocate(ht->slot_count * sizeof(*(ht->keys)));
            ht->values = htMemoryAllocate(ht->slot_count * sizeof(*(ht->values)));
        }

        memset(ht->keys,   0, ht->slot_count * sizeof(*(ht->keys)));
        memset(ht->values, 0, ht->slot_count * sizeof(*(ht->values)));

        for (Size i = 0; i < slot_count_prev; ++i) {
            U64 key_i   = keys[i];
            U64 value_i = values[i];
            if (value_i != 0) {
                U64 hash_new = hashUniversal(ht->univ, key_i);
                ht_LinearProbeInsertion(ht, hash_new, key_i, value_i);
            }
        }

        if (ht->allocator) {
            ht->allocator(Memory_Allocator_Mode_DEALLOCATE, 0, keys, NULL);
            ht->allocator(Memory_Allocator_Mode_DEALLOCATE, 0, values, NULL);
        } else {
            htMemoryDeallocate(keys);
            htMemoryDeallocate(values);
        }

        ht->collision_count = 0;
    }

    U64 hash = hashUniversal(ht->univ, key);
    U64 result_value = ht_LinearProbeInsertion(ht, hash, key, value);
    if (result_value == 0) {
        ht->slot_filled++;
    }

    return result_value;
}

header_function
U64 htLookup (Hash_Table *ht, U64 key)
{
    if (key == 0) return 0;

    Size location = 0;
    U64 result_value = 0;

    U64 hash = hashUniversal(ht->univ, key);
    if (ht_LinearProbeSearch(ht, key, hash, &location)) {
        result_value = ht->values[location];
    }

    return result_value;
}

header_function
U64 htRemove (Hash_Table *ht, U64 key)
{
    if (key == 0) return 0;

    Size location = 0;
    U64 result_value = 0;

    U64 hash = hashUniversal(ht->univ, key);
    if (ht_LinearProbeSearch(ht, key, hash, &location)) {
        result_value = ht->values[location];
    }

    if (result_value != 0) {
        ht->values[location] = 0;
        ht->keys[location] = 0;
        ht->slot_filled -= 1;
    }

    return result_value;
}

#  if defined(NLIB_TESTS)
header_function
void htUnitTest (void)
{
    Hash_Table ht = htCreate();

    /* No Entries */
    utTest(htLookup(&ht, 0) == 0);
    utTest(htLookup(&ht, 1) == 0);
    utTest(htLookup(&ht, 2) == 0);

    /* Insertion Test */
    Size f0 = ht.slot_filled;

    htInsert(&ht, 1, 1);
    utTest(ht.slot_filled == (f0 + 1));
    utTest(htLookup(&ht, 0) == 0);
    utTest(htLookup(&ht, 1) == 1);
    utTest(htLookup(&ht, 2) == 0);

    htInsert(&ht, 2, 42);
    utTest(ht.slot_filled == (f0 + 2));
    utTest(htLookup(&ht, 0) == 0);
    utTest(htLookup(&ht, 1) == 1);
    utTest(htLookup(&ht, 2) == 42);

    /* Duplicate Key */
    U64 v1 = htInsert(&ht, 2, 24);
    utTest(v1 == 42);
    utTest(htLookup(&ht, 0) == 0);
    utTest(htLookup(&ht, 1) == 1);
    utTest(htLookup(&ht, 2) == 24);

    /* Removal Test */
    U64 v2 = htRemove(&ht, 2);
    utTest(v2 == 24);
    utTest(htLookup(&ht, 2) == 0);

    U64 v3 = htRemove(&ht, 1);
    utTest(v3 == 1);
    utTest(htLookup(&ht, 1) == 0);

    /* NULL Check */
    Size f1 = ht.slot_filled;
    htInsert(&ht, 0, 1);
    utTest(ht.slot_filled == f1);
    utTest(htLookup(&ht, 0) == 0);

    Size f2 = ht.slot_filled;
    htRemove(&ht, 0);
    utTest(ht.slot_filled == f2);
    utTest(htLookup(&ht, 0) == 0);

    /* Expansion Test */
    htInsert(&ht, 3, 33);
    utTest(htLookup(&ht, 3) == 33);

    htInsert(&ht, 4, 44);
    utTest(htLookup(&ht, 4) == 44);

    htInsert(&ht, 5, 55);
    utTest(htLookup(&ht, 5) == 55);

    htInsert(&ht, 6, 66);
    utTest(htLookup(&ht, 6) == 66);

    htInsert(&ht, 7, 77);
    utTest(htLookup(&ht, 7) == 77);

    htInsert(&ht, 8, 88);
    utTest(htLookup(&ht, 8) == 88);

    htInsert(&ht, 9, 99);
    utTest(htLookup(&ht, 9) == 99);

    /* Removal after Expansion */

    htRemove(&ht, 3);
    utTest(htLookup(&ht, 3) == 0);

    htRemove(&ht, 4);
    utTest(htLookup(&ht, 4) == 0);

    htRemove(&ht, 5);
    utTest(htLookup(&ht, 5) == 0);

    htRemove(&ht, 6);
    utTest(htLookup(&ht, 6) == 0);

    htRemove(&ht, 7);
    utTest(htLookup(&ht, 7) == 0);

    htRemove(&ht, 8);
    utTest(htLookup(&ht, 8) == 0);

    htRemove(&ht, 9);
    utTest(htLookup(&ht, 9) == 0);

    htDelete(ht);

    Hash_Table htalloc = htCreate();
    for (Size i = 1; i < 100; i++) {
        htInsert(&htalloc, i, 200 + i);
        claim(htLookup(&htalloc, i) == (200 + i));
    }
    for (Size i = 1; i < 100; i++) {
        claim(htLookup(&htalloc, i) == (200 + i));
    }
    htDelete(htalloc);

    return;
}
#  endif
# endif // defined(LANG_C)

/* ==============
 * Map
 */

/* API ----------------------------------------
 * void  mapInsert     (T *ptr, U64 key, T value)
 * void  mapRemove     (T *ptr, U64 key)
 * B32   mapExists     (T *ptr, U64 key)
 * T     mapLookup     (T *ptr, U64 key)
 * T*    mapGetRef     (T *ptr, U64 key)
 * void  mapDelete     (T *ptr)
 */

# if defined(LANG_C)
typedef struct Map_Userdata {
    Hash_Table table;
    Size *free_list;
} Map_Userdata;

#  define mapInsert(_map, _key, _value) do {                            \
        Sbuf_Header *shdr = NULL;                                       \
        if ((_map) == NULL) {                                           \
            (_map) = sbuf_Grow((_map),                                  \
                               sizeof(*(_map)));                        \
            shdr = sbuf_GetHeader(_map);                                \
            (shdr->len)++;                                               \
            shdr->userdata = memAlloc(sizeof(Map_Userdata));             \
            *(Map_Userdata*)(shdr->userdata) = (Map_Userdata){0};        \
            ((Map_Userdata*)shdr->userdata)->table = htCreate();          \
        } else {                                                        \
            shdr = sbuf_GetHeader((_map));                              \
        }                                                               \
                                                                        \
        Size insertion_index = 0;                                       \
                                                                        \
        if ((_key) != 0) {                                              \
            if (sbufElemin(((Map_Userdata*)shdr->userdata)->free_list) > 0) { \
                (_map)[((Map_Userdata*)shdr->userdata)->free_list[0]] = (_value); \
                insertion_index = ((Map_Userdata*)shdr->userdata)->free_list[0]; \
                sbufUnsortedRemove(((Map_Userdata*)shdr->userdata)->free_list, 0); \
            } else {                                                    \
                sbufAdd((_map), (_value));                              \
                insertion_index = sbufElemin((_map)) - 1;               \
            }                                                           \
                                                                        \
            htInsert(&((Map_Userdata*)shdr->userdata)->table, (_key), insertion_index); \
        }                                                               \
    } while (0)

header_function
B32 mapExists (void *map, U64 key) {
    if (map != NULL) {
        Sbuf_Header *shdr = sbuf_GetHeader(map);
        Size index = htLookup(&((Map_Userdata*)shdr->userdata)->table, key);
        if (index != 0) {
            return true;
        }
    }
    return false;
}

#  define mapLookup(_map, _key) ((_map)[(htLookup(&((Map_Userdata*)(sbuf_GetHeader(_map))->userdata)->table, \
                                                (_key)))])
#  define mapGetRef(_map, _key) (&mapLookup((_map), (_key)))

#  define mapRemove(_map, _key) do {                                      \
        if ((_map) != NULL) {                                           \
            Sbuf_Header *shdr = sbuf_GetHeader(_map);                   \
            Size index = htLookup(&((Map_Userdata*)shdr->userdata)->table, \
                                  (_key));                              \
            sbufAdd(((Map_Userdata*)shdr->userdata)->free_list, index); \
            htRemove(&((Map_Userdata*)shdr->userdata)->table, (_key));  \
        }                                                               \
    } while (0)

#  if defined(NLIB_TESTS)
header_function
void mapUnitTest (void)
{
    F32 *fm = NULL;

    /* No Entries */
    utTest(mapExists(fm, 0) == false);
    utTest(mapExists(fm, 1) == false);
    utTest(mapExists(fm, 2) == false);

    /* Insertion Test */

    mapInsert(fm, 1, 1.0f);

    Sbuf_Header *fmsh = sbuf_GetHeader(fm);
    Map_Userdata *fmu = fmsh->userdata;
    Size fh0 = fmu->table.slot_filled - 1;
    Size fs0 = sbufElemin(fm) - 1;

    utTest(fmu->table.slot_filled == (fh0 + 1));
    utTest(sbufElemin(fm) == (fs0 + 1));
    utTest(mapExists(fm, 0) == false);
    utTest(mapExists(fm, 1) == true);
    utTest(mapLookup(fm, 1) == 1.0f);
    utTest(mapExists(fm, 2) == false);

    mapInsert(fm, 2, 42.0f);
    utTest(fmu->table.slot_filled == (fh0 + 2));
    utTest(sbufElemin(fm) == (fs0 + 2));
    utTest(mapExists(fm, 0) == false);
    utTest(mapExists(fm, 1) == true);
    utTest(mapLookup(fm, 1) == 1.0f);
    utTest(mapExists(fm, 2) == true);
    utTest(mapLookup(fm, 2) == 42.0f);

    /* Duplicate Key */
    mapInsert(fm, 2, 24.0f);
    utTest(mapLookup(fm, 1) == 1.0f);
    utTest(mapLookup(fm, 2) == 24.0f);

    /* Removal Test */
    Size fh_r = fmu->table.slot_filled;

    mapRemove(fm, 2);
    utTest(mapExists(fm, 2) == false);
    utTest(fmu->table.slot_filled == fh_r - 1);

    mapRemove(fm, 1);
    utTest(mapExists(fm, 1) == false);
    utTest(fmu->table.slot_filled == fh_r - 2);

    /* NULL Check */
    Size fh1 = fmu->table.slot_filled;
    Size fs1 = sbufElemin(fm);
    mapInsert(fm, 0, 13.0f);
    utTest(fmu->table.slot_filled == fh1);
    utTest(sbufElemin(fm) == fs1);
    utTest(mapExists(fm, 0) == false);

    Size fh2 = fmu->table.slot_filled;
    Size fs2 = sbufElemin(fm);
    mapRemove(fm, 0);
    utTest(fmu->table.slot_filled == fh2);
    utTest(sbufElemin(fm) == fs2);
    utTest(mapExists(fm, 0) == false);

    return;
}
#  endif

# endif // defined(LANG_C)

# if defined(LANG_CPP)
}
# endif

#define NLIB_H_INCLUDE_GUARD
#endif // NLIB_H_INCLUDE_GUARD
