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
#   include <pthread.h>
#   include <semaphore.h>
#  endif
# endif

/* =======================
 * Other nlib libraries
 */

# define NLIB_PRINT_INTERFACE_ONLY
# include "nlib_print.h"
# undef NLIB_PRINT_INTERFACE_ONLY

# include "nlib_debug.h"
# include "nlib_bitbang.h"
# include "nlib_maths.h"
# include "nlib_numbers.h"
# include "nlib_string.h"
# include "nlib_memory.h"
# include "nlib_print.h"
# include "nlib_color.h"
# include "nlib_unicode.h"
# include "nlib_data_structures.h"

# if defined(LANG_CPP)
}
# endif

#define NLIB_H_INCLUDE_GUARD
#endif // NLIB_H_INCLUDE_GUARD
