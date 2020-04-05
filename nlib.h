/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2018 Naman Dixit
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

# if defined(COMPILER_MSVC)
#  if !defined(__cplusplus) // TODO(naman): See if this is actually works and is the best way.
#   define LANGUAGE_C99  // TODO(naman): Update when Microsoft gets off its ass.
#  else
#   error Language not supported
#  endif
# endif

# if defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#  if (__STDC_VERSION__ == 199409)
#   define LANGUAGE_C89
#  elif (__STDC_VERSION__ == 199901)
#   define LANGUAGE_C99
#  elif (__STDC_VERSION__ == 201112) || (__STDC_VERSION__ == 201710)
#   define LANGUAGE_C11
#  else
#   error Language not supported
#  endif
# endif

# if defined(OS_WINDOWS)
#  define ENDIAN_BIG
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

/* NOTE(naman): Manually define NLIB_EXCLUDE_CRT if you want to not include CRT.
   (done to maintain backwards compatibility). If you do prevent CRT from inclusion, do
   the following from the code:

   1. Run memUserCreate()
*/

# if defined(COMPILER_MSVC)
#  pragma warning(push)
#   pragma warning(disable:4668)
# endif

# include <stddef.h>

# if defined(COMPILER_MSVC)
#  pragma warning(pop)
# endif

# include <limits.h>
# include <stdint.h>
# include <stdarg.h>
# include <inttypes.h>
# include <stdnoreturn.h>
# include <float.h>
# include <stddef.h>
# include <errno.h>

#include <stdatomic.h>
# include <stdio.h>
# include <string.h>

# if !defined(NLIB_EXCLUDE_CRT)
#  include <stdlib.h>
# endif

/* ===========================
 * Misc C Headers Includes
 */


/* ===========================
 * Platform Headers Includes
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

# elif defined(OS_LINUX)
// ...

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
# define true                1U
# define false               0U

typedef unsigned char        Byte;
typedef char                 Char;

/* ========================
 * Preprocessor Definitions
 */

# define elemin(array) (sizeof(array)/sizeof((array)[0]))
#define containerof(ptr, type, member)                                  \
    ((type *)( ((Byte *)(true ? (ptr) : (type *)NULL)) - offsetof(type, member) ))

# define KiB(x) (   (x) * 1024ULL)
# define MiB(x) (KiB(x) * 1024ULL)
# define GiB(x) (MiB(x) * 1024ULL)
# define TiB(x) (GiB(x) * 1024ULL)

# define THOUSAND 1000L
# define MILLION  1000000L
# define BILLION  1000000000L

# define unused_variable(var) (void)var

# define global_variable   static
# define persistent_value  static

# define internal_function static
# define header_function   static inline

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

#  if defined(LANGUAGE_C11)
#   include <threads.h>
#  else
#   define thread_local __declspec( thread )
#  endif

#  define swap_endian(x) _byteswap_ulong(x)

# elif defined(COMPILER_CLANG) || defined(COMPILER_GCC)

// Creates shadowing issues if used like max(min(), min()), etc.
/* #  define max(a, b)                             \ */
/*     ({ __typeof__ (a) _a = (a);                 \ */
/*         __typeof__ (b) _b = (b);                \ */
/*         _a > _b ? _a : _b; }) */
/* #  define min(a, b)                             \ */
/*     ({ __typeof__ (a) _a = (a);                 \ */
/*         __typeof__ (b) _b = (b);                \ */
/*         _a < _b ? _a : _b; }) */

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)

#  if defined(LANGUAGE_C11)
#   include <stdalign.h>
#  else
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wreserved-id-macro"
#   define _Alignof __alignof__
#   define alignof _Alignof
#   define __alignof_is_defined 1
#   define _Alignas(a) __attribute__ ((aligned (a)))
#   define alignas _Alignas
#   define __alignas_is_defined 1
#   pragma clang diagnostic pop

/* Malloc Alignment: https://msdn.microsoft.com/en-us/library/ycsb6wwf.aspx
 */
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

#  if defined(LANGUAGE_C11)
#   include <threads.h>
#  else
#   define thread_local __thread
#  endif

#  define swap_endian(x) __builtin_bswap32(x)

# endif

/* =======================
 * Other nlib libraries
 */

# include "unicode.h"


/* =======================
 * OS Specific Hacks
 */

# if defined(OS_WINDOWS)

header_function
void reportDebugV(Char *format, va_list ap)
{
    Char buffer[2048] = {0};

    stbsp_vsnprintf(buffer, 2048, format, ap);
    buffer[2047] = '\0';

    LPWSTR wcstr = unicodeWin32UTF16FromUTF8(buffer);
    OutputDebugStringW(wcstr);
    unicodeWin32UTF16Dealloc(wcstr);

    return;
}

header_function
void reportDebug(Char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    reportDebugV(format, ap);
    va_end(ap);

    return;
}


#  if defined(BUILD_INTERNAL)
#   define report(...)  reportDebug(__VA_ARGS__)
#   define reportv(...) reportDebugV(__VA_ARGS__)
#   define breakpoint() __debugbreak()
#   define quit() breakpoint()
#  else
#   define report(...)      fprintf(stderr, __VA_ARGS__)
#   define reportv(format, va_list) vfprintf(stderr, format, va_list)
#   define quit() abort()
#   define breakpoint() do{report("Fired breakpoint in release code, quitting...\n");quit();}while(0)
#  endif

# elif defined(OS_LINUX)

#  if defined(BUILD_INTERNAL)
#   define report(...)      fprintf(stderr, __VA_ARGS__)
#   define reportv(format, va_list) vfprintf(stderr, format, va_list)
#   define breakpoint() __asm__ volatile("int $0x03")
#   define quit() breakpoint()
#  else
#   define report(...)      fprintf(stderr, __VA_ARGS__)
#   define reportv(format, va_list) vfprintf(stderr, format, va_list)
#   define quit() abort()
#   define breakpoint() do{report("Fired breakpoint in release code, quitting...\n");quit();}while(0)
#  endif

# endif

/* ===============================
 * Integer Mathematics Functions
 */

# if defined(OS_WINDOWS)

/* _BitScanReverse64(&r, x) scans for the first 1-bit from left in x. Once it finds it,
 * it returns the number of bits after the found 1-bit.
 *
 * If b is the bit-width of the number,
 *    p is the closest lower power of two and
 *    r is the number of bits to the right of the first 1-bit when seen from left; then
 * then a number between 2^p and 2^(p+1) has the form: (b-p-1 0-bits) 1 (p bits)
 *
 * => r = p
 *
 * Thus, the rounded-down log2 of the number is r.
 */
header_function
U64 u64Log2(U64 x)
{
    unsigned long result = 0;
    _BitScanReverse64(&result, x);
    return result;
}

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
U64 u64Rand (U64 seed)
{
    U64 previous = seed;
    if (previous == 0) {
        // This seed has been tested and should be preferred in normal circumstances.
        previous = 2531011ULL;
    }

    U64 a = 214013ULL;

    U64 upper = 0, lower = 0;
    lower = _umul128(previous, a, &upper);
    U64 log_upper = u64Log2(upper);
    U64 shift_amount = 64 - (log_upper + 1);
    upper = (upper << shift_amount) | (lower >> log_upper);

    U64 result = upper * a;

    return result;
}

# elif defined(OS_LINUX)

/* __builtin_clzll(x) returns the leading number of 0-bits in x, starting from
 * most significant position.
 *
 * If b is the bit-width of the number,
 *    p is the closest lower power of two and
 *    lz is the number of leading 0-bits; then
 * then a number between 2^p and 2^(p+1) has the form: (b-p-1 0-bits) 1 (p bits)
 *
 * => lz = b-p-1
 * => p = b-(lz+1)
 *
 * Thus, the rounded-down log2 of the number is b-(lz+1).
 */
header_function
U64 u64Log2(U64 x)
{
    U64 result = 64ULL - ((U64)__builtin_clzll(x) + 1ULL);
    return result;
}

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
U64 u64Rand (U64 seed)
{
    U64 previous = seed;
    if (previous == 0) {
        // This seed has been tested and should be preferred in normal circumstances.
        previous = 2531011ULL;
    }

    U64 a = 214013ULL;

    __uint128_t product = (__uint128_t)previous * (__uint128_t)a;
    U64 upper = product >> 64, lower = (U64)product;
    U64 log_upper = u64Log2(upper);
    U64 shift_amount = 64 - (log_upper + 1);
    upper = (upper << shift_amount) | (lower >> log_upper);

    U64 result = upper * a;

    return result;
}

# endif

header_function
U64 u64NextPowerOf2 (U64 x)
{
    U64 result = 0;
    if ((x != 0) && ((x & (x - 1)) == 0)) { // If x is a power of true
        result = x;
    } else {
        result = 1 << (u64Log2(x) + 1);
    }

    return result;
}



/* ****************************************************************************
 * LIBRARIES ******************************************************************
 */

/* ==============
 * Claim (assert)
 */

# define claim(cond) claim_(cond, #cond, __FILE__, __LINE__)

header_function
void claim_ (B32 cond,
             Char *cond_str,
             Char *filename, U32 line_num)
{
    if (!cond) {
        report("Claim \"%s\" Failed in %s:%u\n\n",
               cond_str, filename, line_num);

        quit();
    }
}

/* ===================
 * String Functions
 */

header_function
B32 strequal (Char *str1, Char *str2)
{
    B32 result = (strcmp(str1, str2) == 0);
    return result;
}

header_function
Size strprefix(Char *str, Char *pre)
{
    Size lenpre = strlen(pre);
    Size lenstr = strlen(str);

    if (lenstr < lenpre) {
        return 0;
    } else {
        if (memcmp(pre, str, lenpre) == 0) {
            return lenpre;
        } else {
            return 0;
        }
    }
}

header_function
B32 strsuffix (Char *str, Char *suf)
{
    Char *string = strrchr(str, suf[0]);
    B32 result = false;

    if(string != NULL) {
        if (strcmp(string, suf) == 0) {
            result = true;
        }
    }

    return result;
}

/* ===================
 * Unit Test Framework
 */

# define utTest(cond) ut_Test(cond, #cond, __FILE__, __LINE__)

header_function
void ut_Test (B32 cond,
              Char *cond_str,
              Char *filename, U32 line_num) {
    if (!(cond)) {
        report("Test Failed: (%s:%u) %s\n", filename, line_num, cond_str);
        quit();
    }
}

/* ****************************************************************************
 * MEMORY ALLOCATORS **********************************************************
 */
// FIXME(naman): Custom Memory Allocator is buggy, fix it. (see game.c in viscera)
/* ===============================
 * User Memory Allocator Helpers
 */

#define MEM_MAX_ALIGN_MINUS_ONE (alignof(max_align_t) - 1u)
#define memAlignUp(p) (((p) + MEM_MAX_ALIGN_MINUS_ONE) & (~ MEM_MAX_ALIGN_MINUS_ONE))
#define memAlignDown(p) (memAlignUp((p) - MEM_MAX_ALIGN_MINUS_ONE))

#define memBytesFromBits(b) (((b)+(CHAR_BIT-1))/(CHAR_BIT))

#define memSetBit(array, index)                                 \
    ((array)[(index)/CHAR_BIT] |= (1U << ((index)%CHAR_BIT)))
#define memResetBit(array, index)                               \
    ((array)[(index)/CHAR_BIT] &= ~(1U << ((index)%CHAR_BIT)))
#define memToggleBit(array, index)                              \
    ((array)[(index)/CHAR_BIT] ^= ~(1U << ((index)%CHAR_BIT)))
#define memTestBit(array, index)                                \
    ((array)[(index)/CHAR_BIT] & (1U << ((index)%CHAR_BIT)))


# if defined(COMPILER_CLANG)
#  pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wcast-align"
# endif

typedef enum Memory_Allocator_Mode {
    Memory_Allocator_Mode_NONE,
    Memory_Allocator_Mode_CREATE,
    Memory_Allocator_Mode_ALLOC,
    Memory_Allocator_Mode_REALLOC,
    Memory_Allocator_Mode_DEALLOC,
    Memory_Allocator_Mode_DEALLOC_ALL,
    Memory_Allocator_Mode_SIZE,
} Memory_Allocator_Mode;

# define MEMORY_ALLOCATOR(allocator)               \
    void* allocator(Memory_Allocator_Mode mode,          \
                    Size size, void* old_ptr,   \
                    void *data)
typedef MEMORY_ALLOCATOR(Memory_Allocator_Function);

/* =============================
 * General Purpose User Memory Allocator
 */

typedef struct Memory_User_Buddy Memory_User_Buddy;

typedef struct Memory_User {
    Memory_User_Buddy *b;
    Byte *base;
    Size total;
    Size filled;
} Memory_User;

struct Memory_User_Buddy {
    Byte *free_bits;
    Byte *split_bits;

    struct Memory_User_Buddy *prev;
    struct Memory_User_Buddy *next;

    Byte *arena;
    Size arena_size;
    Size leaf_size;
    Size leaf_count;
    Size block_count;
    U8 level_max;
    U8 level_count;

    Byte _pad1[6];
};

/*
  -------------------------------------------------------------------
  L:0 |                                0                             |
  -------------------------------------------------------------------
  L:1 |               1                |              2              |
  -------------------------------------------------------------------
  L:2 |       3       |       4        |       5      |       6      |
  -------------------------------------------------------------------
  L:3 |   7   |   8   |   9   |   10   |  11   |  12  |  13   |  14  |
  -------------------------------------------------------------------
*/

#define memcb_CountOfBlocksAtLevel(level) (1u << (level))
#define memcb_SizeOfBlocksAtLevel(b, level)                     \
    (((b)->arena_size)/memcb_CountOfBlocksAtLevel(level))
#define memcb_IndexOfBlockInLevel(b, ptr, level) /* Zero based */       \
    ((Uptr)((ptr) - (b)->arena) / memcb_SizeOfBlocksAtLevel(b, level))
#define memcb_IndexOfLeaf(b, ptr) /* Zero based */      \
    ((Uptr)((ptr) - ((b)->arena)) / ((b)->leaf_size))
#define memcb_PointerToBlockInLevel(b, index, level)                    \
    (((index) * memcb_SizeOfBlocksAtLevel(b, level)) + (b)->arena)
#define memcb_PreviousBlockInLevel(b, ptr, level)       \
    ((void*)(((Byte*)ptr) - memcb_SizeOfBlocksAtLevel(b, level)))
#define memcb_NextBlockInLevel(b, ptr, level)           \
    ((void*)(((Byte*)ptr) + memcb_SizeOfBlocksAtLevel(b, level)))
#define memcb_BlocksBeforeThisLevel(level)                              \
    ((1 << (level)) - 1) /* Using sum of GP, blocks before level n are (2ⁿ - 1) */
#define memcb_GlobalIndexOfBlock(b, ptr, level) \
    (memcb_BlocksBeforeThisLevel(level) +       \
     memcb_IndexOfBlockInLevel(b, ptr, level))
#define memcb_IndexOfFirstChild(index)          \
    ((2 * (index)) + 1)
#define memcb_IndexOfSecondChild(index)         \
    ((2 * (index)) + 2)
#define memcb_IndexOfParent(index)              \
    (((index) - 1) / 2)

header_function
Memory_User_Buddy memcb_Init (Byte* arena, Size arena_size, Size leaf_size)
{
    Size leaf_count  = arena_size / leaf_size;
    U8 level_max   = (U8)u64Log2(leaf_count);
    U8 level_count = level_max + 1;
    Size block_count  = (1 << level_count) - 1; // 2ⁿ⁺¹ - 1 where n is level_max (sum of GP)

    Memory_User_Buddy buddy = {0};
    buddy.arena = arena;
    buddy.arena_size = arena_size;
    buddy.leaf_size = leaf_size;
    buddy.leaf_count = leaf_count;
    buddy.level_max = level_max;
    buddy.level_count = level_count;
    buddy.block_count = block_count;

    return buddy;
}

header_function
void* memcb_GetFreeBlockAtLevel (Memory_User_Buddy *buddy, Size level)
{
    Byte *found_block = NULL;
    Size level_current = level;

    do {
        if (level_current == 0) {
            if (memTestBit(buddy->free_bits, 0)) {
                found_block = buddy->arena;
            }
            break;
        }

        Size free_bits_begin = memcb_GlobalIndexOfBlock(buddy,
                                                        buddy->arena,
                                                        level_current);
        Size free_bits_end = free_bits_begin + memcb_CountOfBlocksAtLevel(level_current);

        for (Size bi = free_bits_begin; bi < free_bits_end; bi = bi + 1) {
            if (memTestBit(buddy->free_bits, bi)) {
                found_block = (buddy->arena +
                               (memcb_SizeOfBlocksAtLevel(buddy,
                                                                    level_current) *
                                (bi - memcb_BlocksBeforeThisLevel(level_current))));
                break;
            }
        }

        if (level_current == 0) {
            break;
        } else if (found_block == NULL) {
            level_current--;
        }
    } while (found_block == NULL);

    if (found_block != NULL) {
        for (Size lvl = level_current + 1; lvl <= level; lvl++) {
            Byte *found_block_sibiling = (found_block +
                                          memcb_SizeOfBlocksAtLevel(buddy, lvl));

            memSetBit(buddy->free_bits,
                     memcb_GlobalIndexOfBlock(buddy, found_block, lvl));
            memSetBit(buddy->free_bits,
                      memcb_GlobalIndexOfBlock(buddy, found_block_sibiling, lvl));
            memResetBit(buddy->free_bits,
                        memcb_IndexOfParent(memcb_GlobalIndexOfBlock(buddy,
                                                                     found_block,
                                                                     lvl)));
            memSetBit(buddy->split_bits,
                      memcb_IndexOfParent(memcb_GlobalIndexOfBlock(buddy,
                                                                   found_block,
                                                                   lvl)));
        }

        memResetBit(buddy->free_bits,
                   memcb_GlobalIndexOfBlock(buddy, found_block, level));
    }

    return found_block;
}

header_function
void* memcb_Alloc (Memory_User_Buddy *bbuddy, Size size)
{
    Size size_real = size;

    if (size_real == 0 || bbuddy == NULL) {
        return NULL;
    }

    if (size_real < bbuddy->leaf_size) {
        size_real = bbuddy->leaf_size;
    }

    if (size_real > bbuddy->arena_size) {
        return NULL;
    }

    Size total_size = u64NextPowerOf2(size_real);

    // "inverse" because it is taking smallest block as level 0
    U8 inverse_level = (U8)u64Log2(total_size/(bbuddy->leaf_size));
    U8 level = bbuddy->level_max - inverse_level; // Actually, largest block is level 0

    Byte *m = memcb_GetFreeBlockAtLevel(bbuddy, level);

    return m;
}

header_function
Byte* memcb_MergeBuddies (Memory_User_Buddy *buddy, Byte *block, Size level)
{
    if (level == 0) return NULL;

    Byte *result = NULL;

    Size local_index  = memcb_IndexOfBlockInLevel(buddy, block, level);

    Byte *parent_block = NULL; // the pointer to the merged blocks (if they get merged)
    Byte *buddy_block = NULL; // pointer to buddy block

    if (local_index % 2 == 1) { // odd = merge with previous block
        buddy_block = memcb_PreviousBlockInLevel(buddy, block, level);
        parent_block = buddy_block;
    } else { // even = merge with next block
        buddy_block = memcb_NextBlockInLevel(buddy, block, level);
        parent_block = block;
    }

    Size block_global_index = memcb_GlobalIndexOfBlock(buddy, block, level);
    Size buddy_global_index = memcb_GlobalIndexOfBlock(buddy, buddy_block, level);
    Size parent_global_index = memcb_GlobalIndexOfBlock(buddy, parent_block, level - 1);

    if (memTestBit(buddy->free_bits, buddy_global_index)) { // Buddy is also free
        memResetBit(buddy->free_bits, buddy_global_index);
        memResetBit(buddy->free_bits, block_global_index);
        memSetBit(buddy->free_bits, parent_global_index);
        memResetBit(buddy->split_bits, parent_global_index);

        result = parent_block;
    }

    return result;
}

header_function
void memcb_ReleaseBlockAtLevel (Memory_User_Buddy *buddy, Byte *ptr, Size level)
{
    memSetBit(buddy->free_bits, memcb_GlobalIndexOfBlock(buddy, ptr, level));

    Size merge_level = level;
    Byte *merge_ptr = ptr;

    while (merge_level > 0) {
        merge_ptr = memcb_MergeBuddies(buddy, merge_ptr, merge_level);
        if (merge_ptr == NULL) break;
        merge_level--;
    }

    return;
}

header_function
void memcb_Dealloc (Memory_User_Buddy *buddy, void *ptr)
{
    if (ptr == NULL || buddy == NULL) return;

     U8 level_min = buddy->level_max; //The real level will never be zero since we already
                                      // made a bunch of allocations at that level manually during init.

     while ((level_min < buddy->level_count) && (level_min > 0)) {
         if (memcb_IndexOfBlockInLevel(buddy, (Byte*)ptr, level_min) % 2 == 1) break;
         level_min--;
     }

    U8 level = buddy->level_max;
    while ((level >= level_min) && (level <= buddy->level_max)) {
        if (level == 0) {
            break;
        } else if (memTestBit(buddy->split_bits,
                             memcb_IndexOfParent(memcb_GlobalIndexOfBlock(buddy,
                                                                          (Byte*)ptr,
                                                                          level)))) {
            break;
        }
        level--;
    }

    memcb_ReleaseBlockAtLevel(buddy, ptr, level);

    return;
}

header_function
void* memcb_GetMemory (Memory_User *m, Size size)
{
    if ((m->filled + size) > m->total) {
        fprintf(stderr, "Memory full: Total = %lu, Filled = %lu\n", m->total, m->filled);
        fflush(stdout);
        return NULL;
    }

    void *result = m->base + m->filled;

    m->filled += size;

    return result;
}

# define memUserCreate(m, base, size)    memUser(Memory_Allocator_Mode_CREATE,  size, base, m)
# define memUserAlloc(m, size)           memUser(Memory_Allocator_Mode_ALLOC,   size, NULL, m)
# define memUserRealloc(m, ptr, size)    memUser(Memory_Allocator_Mode_REALLOC, size, ptr,  m)
# define memUserDealloc(m, ptr)          memUser(Memory_Allocator_Mode_DEALLOC, 0,    ptr,  m)

header_function
MEMORY_ALLOCATOR(memUser)
{
    Memory_User *m = data;

    switch (mode) {
        case Memory_Allocator_Mode_CREATE: {
            m->total = size;
            m->base = old_ptr;
            m->filled = 0;
            m->b = NULL;
        } break;

        case Memory_Allocator_Mode_ALLOC: {
            if (size == 0) return NULL;
            size = u64NextPowerOf2(size);

            void *mem = NULL;

            for (Memory_User_Buddy *b = m->b; b != NULL; b = b->next) {
                if ((size >= b->leaf_size) && (size <= b->arena_size)) {
                    mem = memcb_Alloc(b, size);
                    if (mem != NULL) break;
                } else {
                    continue;
                }
            }

            if (mem == NULL) {
                Size leaf_size = size;
                Size arena_size = 1 << (u64Log2(size) + 5);

                Memory_User_Buddy *buddy = memcb_GetMemory(m, sizeof(*buddy));
                *buddy = (Memory_User_Buddy){0};

                Byte *arena = memcb_GetMemory(m, arena_size);

                *buddy = memcb_Init(arena, arena_size, leaf_size);

                Size size_of_free_bits = memBytesFromBits(buddy->block_count);
                Size size_of_split_bits = memBytesFromBits(buddy->block_count - buddy->leaf_count);
                Size total_size = memAlignUp(size_of_free_bits + size_of_split_bits);

                Byte *overhead = memcb_GetMemory(m, total_size);
                memset(overhead, 0, total_size);

                buddy->free_bits = overhead;
                buddy->split_bits = overhead + size_of_free_bits;
                memSetBit(buddy->free_bits, 0);

                if (m->b != NULL) {
                    m->b->prev = buddy;
                }

                buddy->next = m->b;
                m->b = buddy;

                mem = memcb_Alloc(buddy, size);
            }

            if (mem == NULL) breakpoint();
            return mem;
        } break;

        case Memory_Allocator_Mode_REALLOC: {
            if (old_ptr == NULL) {
                void *result = memUser(Memory_Allocator_Mode_ALLOC, size, NULL, m);
                return result;
            }

            if (size == 0) {
                memUser(Memory_Allocator_Mode_DEALLOC, 0, old_ptr, m);
                return NULL;
            }

            Memory_User_Buddy *buddy = NULL;

            for (Memory_User_Buddy *b = m->b; b != NULL; b = b->next) {
                if (((Byte*)old_ptr > (Byte*)&(b->arena)) && ((Byte*)old_ptr < (((Byte*)&(b->arena)) + b->arena_size))) {
                    buddy = b;
                    break;
                }
            }

            if (buddy == NULL) return NULL;

            U8 level_min = buddy->level_max; // The real level will never be zero since we already
            // made a bunch of allocations at that level manually during init.

            while ((level_min < buddy->level_count) && (level_min > 0)) {
                if (memcb_IndexOfBlockInLevel(buddy, (Byte*)old_ptr, level_min) % 2 == 1) break;
                level_min--;
            }

            U8 level = buddy->level_max;
            while ((level >= level_min) && (level <= buddy->level_max)) {
                if (level == 0) {
                    break;
                } else if (memTestBit(buddy->split_bits,
                                     memcb_IndexOfParent(memcb_GlobalIndexOfBlock(buddy, (Byte*)old_ptr, level)))) {
                    break;
                }
                level--;
            }

            Size old_size = memcb_SizeOfBlocksAtLevel(buddy, level);
            if (old_size > size) {
                return old_ptr;
            }

            if (size < (2 * old_size)) {
                size = 2 * old_size;
            }

            B32 reallocated = false;
            Byte *new_ptr = NULL;

            if (level != 0) {
                Size local_index  = memcb_IndexOfBlockInLevel(buddy, (Byte*)old_ptr, level);

                Byte *parent_ptr = NULL; // the pointer to the merged blocks (if they get merged)
                Byte *buddy_ptr = NULL; // pointer to buddy block

                if (local_index % 2 == 0) {
                    buddy_ptr = memcb_NextBlockInLevel(buddy, old_ptr, level);
                    parent_ptr = old_ptr;

                    Size buddy_global_index = memcb_GlobalIndexOfBlock(buddy, buddy_ptr, level);
                    Size parent_global_index = memcb_GlobalIndexOfBlock(buddy, parent_ptr, level - 1);

                    if (memTestBit(buddy->free_bits, buddy_global_index)) {
                        memResetBit(buddy->free_bits, buddy_global_index);
                        memResetBit(buddy->split_bits, parent_global_index);

                        reallocated = true;
                        new_ptr = old_ptr;
                    }
                } else {
                    buddy_ptr = memcb_PreviousBlockInLevel(buddy, old_ptr, level);
                    parent_ptr = buddy_ptr;

                    Size buddy_global_index = memcb_GlobalIndexOfBlock(buddy, buddy_ptr, level);
                    Size parent_global_index = memcb_GlobalIndexOfBlock(buddy, parent_ptr, level - 1);

                    if (memTestBit(buddy->free_bits, buddy_global_index)) {
                        memcpy(buddy_ptr, old_ptr, old_size);

                        memResetBit(buddy->free_bits, buddy_global_index);
                        memResetBit(buddy->split_bits, parent_global_index);

                        reallocated = true;
                        new_ptr = old_ptr;
                    }
                }
            }

            if (!reallocated) {
                new_ptr = memUser(Memory_Allocator_Mode_ALLOC, size, NULL, m);
                if (new_ptr == NULL) return NULL;
                memcpy(new_ptr, old_ptr, old_size);
                memUser(Memory_Allocator_Mode_DEALLOC, 0, old_ptr,  m);
            }

            return new_ptr;
        } break;

        case Memory_Allocator_Mode_DEALLOC: {
            for (Memory_User_Buddy *b = m->b; b != NULL; b = b->next) {
                if (((Byte*)old_ptr > (Byte*)&(b->arena)) && ((Byte*)old_ptr < (((Byte*)&(b->arena)) + b->arena_size))) {
                    memcb_Dealloc(b, old_ptr);
                    break;
                }
            }

            return NULL;
        } break;

        case Memory_Allocator_Mode_DEALLOC_ALL: {
            // TODO(naman): Maybe we should use a off-the-shelf malloc that allows this?
        } break;

        case Memory_Allocator_Mode_SIZE: {
            // TODO(naman): Implement this
        } break;

        case Memory_Allocator_Mode_NONE: {
            breakpoint();
        } break;
    }

    return NULL;
}

/* =============================
 * stdlib Memory Allocator
 */

# if !defined(NLIB_EXCLUDE_CRT)

struct MemCRT_Header {
    Size size;
};

#  define memCRTAlloc(size)        memCRT(Memory_Allocator_Mode_ALLOC,   size, NULL, NULL)
#  define memCRTRealloc(ptr, size) memCRT(Memory_Allocator_Mode_REALLOC, size, ptr,  NULL)
#  define memCRTDealloc(ptr)       memCRT(Memory_Allocator_Mode_DEALLOC, 0,    ptr,  NULL)
#  define memCRTSize(ptr)          memCRT(Memory_Allocator_Mode_SIZE,    0,    ptr,  NULL)

header_function
MEMORY_ALLOCATOR(memCRT)
{
    unused_variable(data);
    switch (mode) {
        case Memory_Allocator_Mode_CREATE: {
            // NOTE(naman): Not needed for now
        } break;

        case Memory_Allocator_Mode_ALLOC: {
            Size memory_size = memAlignUp(size);
            Size header_size = memAlignUp(sizeof(struct MemCRT_Header));
            Size total_size = memory_size + header_size;

            Byte *mem = malloc(total_size);
            memset(mem, 0, total_size);

            struct MemCRT_Header *header = (struct MemCRT_Header *)mem;
            header->size = memory_size;

            Byte *result = mem + header_size;

            return result;
        } break;

        case Memory_Allocator_Mode_REALLOC: {
            Size memory_size = memAlignUp(size);
            Size header_size = memAlignUp(sizeof(struct MemCRT_Header));
            Size total_size = memory_size + header_size;

            Byte *mem = malloc(total_size);

            struct MemCRT_Header *header = (struct MemCRT_Header *)mem;
            header->size = memory_size;

            Byte *result = mem + header_size;

            if (old_ptr != NULL) {
                Byte *previous_mem = (Byte*)old_ptr - header_size;
                struct MemCRT_Header *previous_header = (struct MemCRT_Header *)previous_mem;
                Size previous_size = previous_header->size;

                memcpy(result, old_ptr, previous_size);
                memset(result + previous_size, 0, memory_size - previous_size);

                memCRTDealloc(old_ptr);
            } else {
                memset(result, 0, memory_size);
            }

            return result;
        } break;

        case Memory_Allocator_Mode_DEALLOC: {
            if (old_ptr == NULL) {
                return NULL;
            }

            Size header_size = memAlignUp(sizeof(struct MemCRT_Header));
            Byte *mem = (Byte*)old_ptr - header_size;
            free(mem);
        } break;

        case Memory_Allocator_Mode_DEALLOC_ALL: {
            // TODO(naman): Maybe we should use a off-the-shelf malloc that allows this?
        } break;

        case Memory_Allocator_Mode_SIZE: {
            struct MemCRT_Header *header = (void*)((Byte*)old_ptr - sizeof(*header));

            return &header->size;
        } break;

        case Memory_Allocator_Mode_NONE: {
            breakpoint();
        } break;
    }

    return NULL;
}

# endif

# if !defined(NLIB_EXCLUDE_CRT)
global_variable thread_local Memory_Allocator_Function *memDefaultAllocator = &memCRT;
global_variable thread_local void *memDefaultAllocatorData = NULL;
#  define memAlloc(size)          memCRTAlloc(size)
#  define memRealloc(ptr, size)   memCRTRealloc(ptr, size)
#  define memDealloc(ptr)         memCRTDealloc(ptr)
#  define memSize(ptr)            *(Size*)memCRTSize(ptr)
# else
global_variable thread_local Memory_Allocator_Function *memDefaultAllocator = &memUser;
global_variable thread_local Memory_User *memDefaultAllocatorData = NULL;
#  define memAlloc(size)          memUserAlloc(memDefaultAllocatorData, size)
#  define memRealloc(ptr, size)   memUserRealloc(memDefaultAllocatorData, ptr, size)
#  define memDealloc(ptr)         memUserDealloc(memDefaultAllocatorData, ptr)
#  define memSize(ptr)            *(Size*)memUserSize(memDefaultAllocatorData, ptr)
# endif

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic pop
# endif


/* ****************************************************************************
 * DATA STRUCTURES ******************************************************************
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

typedef struct Sbuf_Header {
    Size cap; // NOTE(naman): Maximum number of elements that can be stored
    Size len; // NOTE(naman): Count of elements actually stored
    void *userdata;
    Byte buf[];
} Sbuf_Header;

# define sbuf_GetHeader(sb) ((Sbuf_Header*)(void*)((Byte*)(sb) - offsetof(Sbuf_Header, buf)))

# define sbuf_Len(sb)         ((sb) ? sbuf_GetHeader(sb)->len : 0U)
# define sbuf_Cap(sb)         ((sb) ? sbuf_GetHeader(sb)->cap : 0U)

# define sbufAdd(sb, ...)     ((sb) = sbuf_Grow((sb), sizeof(*(sb))),   \
                               (sb)[sbuf_Len(sb)] = (__VA_ARGS__),      \
                               ((sbuf_GetHeader(sb))->len)++)
# define sbufDelete(sb)       ((sb) ?                                   \
                               (memDealloc(sbuf_GetHeader(sb)), (sb) = NULL) : \
                               0)
# define sbufClear(sb)        ((sb) ?                                   \
                               (memset((sb), 0, sbufSizeof(sb)),        \
                                sbuf_GetHeader(sb)->len = 0) :          \
                               0)
# define sbufResize(sb, n)    (((n) > sbufMaxElemin(sb)) ?              \
                               ((sb) = sbuf_Resize(sb, n, sizeof(*(sb)))) : \
                               0)

# define sbufSizeof(sb)       (sbuf_Len(sb) * sizeof(*(sb)))
# define sbufElemin(sb)       (sbuf_Len(sb))
# define sbufMaxSizeof(sb)    (sbuf_Cap(sb) * sizeof(*(sb)))
# define sbufMaxElemin(sb)    (sbuf_Cap(sb))
# define sbufEnd(sb)          ((sb) + sbuf_Len(sb))

#define sbufPrint(sb, ...) ((sb) = sbuf_Print((sb), __VA_ARGS__))

#define sbufUnsortedRemove(sb, i) (((sb)[(i)] = (sb)[sbuf_Len(sb) - 1]), \
                                   ((sbuf_GetHeader(sb)->len)--))

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wcast-align"
# endif

header_function
void* sbuf_Grow (void *buf, Size elem_size)
{
    if ((sbuf_Len(buf) + 1) <= sbuf_Cap(buf)) {
        return buf;
    } else {
        Size new_cap = max(2 * sbuf_Cap(buf), 4U);

        Size new_size = (new_cap * elem_size) + sizeof(Sbuf_Header);

        Sbuf_Header *new_header = NULL;

        if (buf != NULL) {
            new_header = memRealloc(sbuf_GetHeader(buf), new_size);
        } else {
            new_header = memAlloc(new_size);
        }

        new_header->cap = new_cap;

        return new_header->buf;
    }
}

header_function
void* sbuf_Resize (void *buf, Size elem_count, Size elem_size)
{
    Size new_cap = elem_count;

    Size new_size = (new_cap * elem_size) + sizeof(Sbuf_Header);

    Sbuf_Header *new_header = NULL;

    if (buf != NULL) {
        new_header = memRealloc(sbuf_GetHeader(buf), new_size);
    } else {
        new_header = memAlloc(new_size);
    }

    new_header->cap = new_cap;

    return new_header->buf;
}

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wformat-nonliteral"
# endif
header_function
Char* sbuf_Print(Char *buf, const Char *fmt, ...)
{
    // TODO(naman): Replace with stbsp_vsnprintf once the following bug is taken care of:
    // https://github.com/nothings/stb/issues/612
    va_list args;

    va_start(args, fmt);
    size_t cap = sbufMaxElemin(buf) - sbufElemin(buf);
    int n = 1 + vsnprintf(sbufEnd(buf), cap, fmt, args);
    va_end(args);

    if ((Size)n > cap) {
        sbufResize(buf, (Size)n + sbufElemin(buf));
        size_t new_cap = sbufMaxElemin(buf) - sbufElemin(buf);
    va_start(args, fmt);
        n = 1 + vsnprintf(sbufEnd(buf), new_cap, fmt, args);
    va_end(args);
    }

    sbuf_GetHeader(buf)->len += ((Size)n - 1);
    return buf;
}
# if defined(COMPILER_CLANG)
#  pragma clang diagnostic pop
# endif

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

    utTest(strcmp(stream, "Hello, World!\nStill here? 420\nGO AWAY!!!\n") == 0);
}

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic pop
# endif



/* ==========================
 * Interning
 *
 * Char* internString (Intern_String *is, Char *str)
 */

#define INTERN_EQUALITY(func_name) B32 func_name (void *a, void *b, Size b_index)
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
        for (Size i = 0; i < sbufElemin(it->lists[hash1].secondary_hashes); i++) {
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
    Char *strings;
} Intern_String;

header_function
INTERN_EQUALITY(internStringEquality) {
    Char *sa = a;
    Char *sb = (Char *)b + b_index;
    B32 result = (strcmp(sa, sb) == 0);
    return result;
}

header_function
Char *internString (Intern_String *is, Char *str)
{
    U8 hash1 = internStringPearsonHash(str, strlen(str), true);
    U8 hash2 = internStringPearsonHash(str, strlen(str), false);

    Size index = 0;

    if (internCheck(&is->intern, hash1, hash2, str, is->strings, &internStringEquality, &index)) {
        Char *result = is->strings + index;
        return result;
    } else {
        Size index_new = sbufElemin(is->strings);
        for (Char *s = str; s[0] != '\0'; s++) {
            sbufAdd(is->strings, s[0]);
        }
        sbufAdd(is->strings, '\0');
        internData(&is->intern, hash1, hash2, index_new);
        Char *result = is->strings + index_new;
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
        Char *result = is->strings + index;
        return result;
    } else {
        return NULL;
    }
}

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
        h->r = u64Rand(h->r);
        h->a = h->r;
    } while ((h->a > 0) && ((h->a & 0x01) != 0x01)); // Make sure that 'a' is odd

    h->r = u64Rand(h->r);
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
 *
 * Hash_Table htCreateWithAlloc (Size min_slots,
 *                               Memory_Allocator_Function *allocator, void *allocator_data);
 */

typedef struct Hash_Table {
    Hash_Universal univ;
    Memory_Allocator_Function *allocator;
    void *allocator_data;
    U64 *keys;
    U64 *values;
    Size slot_count;
    Size slot_filled;
} Hash_Table;

header_function
Hash_Table htCreateWithAlloc (Size slots_atleast,
                              Memory_Allocator_Function *allocator, void *allocator_data)
{
    Hash_Table ht = {0};

    ht.univ.m = u64Log2(max(slots_atleast, 1U)); // Log of closest lower power of two

    // This will make m log of closest upper power of two
    ht.univ.m = ht.univ.m + 1;
    ht.slot_count = 1ULL << (ht.univ.m);
    hashUniversalConstantsUpdate(&ht.univ);

    if (allocator != NULL) {
        ht.allocator = allocator;
        ht.allocator_data = allocator_data;
    } else {
        ht.allocator = memDefaultAllocator;
        ht.allocator_data = memDefaultAllocatorData;
    }

    ht.keys     = ht.allocator(Memory_Allocator_Mode_ALLOC,
                               (ht.slot_count) * sizeof(*(ht.keys)), NULL,
                               ht.allocator_data);
    ht.values   = ht.allocator(Memory_Allocator_Mode_ALLOC,
                               (ht.slot_count) * sizeof(*(ht.values)), NULL,
                               ht.allocator_data);

    return ht;
}

header_function
Hash_Table htCreate (Size slots_atleast)
{
    Hash_Table ht = htCreateWithAlloc(slots_atleast, NULL, NULL);

    return ht;
}

header_function
void htDelete (Hash_Table ht)
{
    ht.allocator(Memory_Allocator_Mode_DEALLOC, 0, ht.keys,   NULL);
    ht.allocator(Memory_Allocator_Mode_DEALLOC, 0, ht.values, NULL);
}

header_function
B32 ht_LinearProbeSearch (Hash_Table *ht, U64 key, Size *value)
{
    Size index = 0;
    B32 found = false;

    for (Size i = 0; !found && (i < ht->slot_count); ++i) {
        index = (key + i) % (ht->slot_count);
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
        }
    }

    return result_value;
}

header_function
U64 htInsert (Hash_Table *ht, U64 key, U64 value)
{
    if ((key == 0) || (value == 0)) return 0;

    // FIXME(naman): Use number of collisions as the parameter for resizing
    if ((2U * (ht->slot_filled)) > (ht->slot_count)) {
        Size slot_count_prev = ht->slot_count;
        U64 *keys   = ht->keys;
        U64 *values = ht->values;

        ht->univ.m = ht->univ.m + 1;
        ht->slot_count = 1ULL << (ht->univ.m);
        hashUniversalConstantsUpdate(&(ht->univ));

        ht->keys   = ht->allocator(Memory_Allocator_Mode_ALLOC,
                                   sizeof(*(ht->keys)) * ht->slot_count,
                                   NULL, ht->allocator_data);
        ht->values = ht->allocator(Memory_Allocator_Mode_ALLOC,
                                   sizeof(*(ht->values)) * ht->slot_count,
                                   NULL, ht->allocator_data);

        for (Size i = 0; i < slot_count_prev; ++i) {
            U64 key_i     = keys[i];
            U64 value_i   = values[i];

            if (value_i != 0) {
                U64 hash_new = hashUniversal(ht->univ, key_i);
                ht_LinearProbeInsertion(ht, hash_new, key_i, value_i);
            }
        }

        ht->allocator(Memory_Allocator_Mode_DEALLOC, 0, keys,   ht->allocator_data);
        ht->allocator(Memory_Allocator_Mode_DEALLOC, 0, values, ht->allocator_data);
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

    if (ht_LinearProbeSearch(ht, key, &location)) {
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

    if (ht_LinearProbeSearch(ht, key, &location)) {
        result_value = ht->values[location];
    }

    if (result_value != 0) {
        ht->values[location] = 0;
        ht->keys[location] = 0;
        ht->slot_filled -= 1;
    }

    return result_value;
}

header_function
void htUnitTest (void)
{
    Hash_Table ht = htCreate(0);

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
    fflush(stdout);
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

    return;
}

#define NLIB_H_INCLUDE_GUARD
#endif // NLIB_H_INCLUDE_GUARD
