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

#  define max(a, b)                             \
    ({ __typeof__ (a) _a = (a);                 \
        __typeof__ (b) _b = (b);                \
        _a > _b ? _a : _b; })
#  define min(a, b)                             \
    ({ __typeof__ (a) _a = (a);                 \
        __typeof__ (b) _b = (b);                \
        _a < _b ? _a : _b; })

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
                void *result = memCustom(MemAllocMode_ALLOC, size, NULL, m);
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
# else
global_variable thread_local Memory_Allocator_Function *memDefaultAllocator = &memUser;
global_variable thread_local Memory_User *memDefaultAllocatorData = NULL;
#  define memAlloc(size)          memUserAlloc(memDefaultAllocatorData, size)
#  define memRealloc(ptr, size)   memUserRealloc(memDefaultAllocatorData, ptr, size)
#  define memDealloc(ptr)         memUserDealloc(memDefaultAllocatorData, ptr)
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

#define sbufUnsortedRemove(sb, i, z) (((sb)[(i)] = (sb)[sbuf_Len(sb) - 1]), \
                                      ((sb)[sbuf_Len(sb) - 1] = (z)),   \
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
        Size new_cap = max(2 * sbuf_Cap(buf), 4);

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
 * Pointer-Pointer Hash Map
 */

typedef struct Hashmap {
    struct HashmapKeys {
        U64  hash;
        Uptr key;
    } *keys;

    Uptr *values;

    Memory_Allocator_Function *allocator;

    Size total_slots, filled_slots;

    U64 a, b, m; /* Hashing constants */

    U64 r; /* Last random number */
} Hashmap;

typedef enum HM_Flag {
    HM_Flag_EMPTY,
    HM_Flag_FILLED,
    HM_Flag_VACATED, // Previously filled
} HM_Flag;


/* API ---------------------------------------------------
 * Hashmap hmCreate  (MemAllocator allocator,
 *                    Size min_slots);
 * void    hmDelete  (Hashmap hm);
 * void*   hmInsert  (Hashmap *hm, void *key, void *value);
 * Uptr    hmInsertI (Hashmap *hm, Uptr key, Uptr value);
 * void*   hmLookup  (Hashmap *hm, void *key);
 * Uptr    hmLookupI (Hashmap *hm, Uptr key);
 * void*   hmRemove  (Hashmap *hm, void *key);
 * Uptr    hmRemoveI (Hashmap *hm, Uptr key);
 */


#define hm_GetFlag(hash) ((hash) >> 62)
#define hm_GetHash(hash) ((hash) & 0x3FFFFFFFFFFFFFFFULL)
#define hm_SetFlag(hash, flag) (hm_GetHash(hash) | (((U64)(flag)) << 62))

/*
 * https://en.wikipedia.org/wiki/Universal_hashing#Avoiding_modular_arithmetic

 * w is number of bits in machine word (64 in our case)
 * s is the number of buckets/bins (slots in the hash table) to which the
 *   universe of hashable objects is to be mapped
 * m is log2(s) (=> m = 2^s) and is equal to the number of bits in the final hash
 * a is a random odd positive integer < 2^w (fitting in w bits)
 * b is a random non-negative integer < 2^(w-m) (fitting in (w-m) bits)
 *
 * r is the last random number generated and is just an implementation detail.
 */

header_function
void hm_UpdateConstants (Hashmap *hm)
{
    hm->m = hm->m + 1;
    hm->total_slots = 1ULL << (hm->m);

    do {
        hm->r = u64Rand(hm->r);
        hm->a = hm->r;
    } while ((hm->a & 0x01) != 0x01); // Make sure that hm.a is odd

    hm->r = u64Rand(hm->r);
    // b should be (64 - m) bits long
    hm->b = hm->r >> hm->m;
}

header_function
U64 hm_Hash (Hashmap *hm, Uptr key)
{
    U64 result =  ((hm->a * key) + hm->b) & (0xFFFFFFFFFFFFFFFFULL >> (64 - hm->m));
    return result;
}

header_function
Hashmap hmCreate (Memory_Allocator_Function allocator,
                  Size min_slots)
{
    Hashmap hm = {0};

    if (allocator == NULL) {
        hm.allocator = memDefaultAllocator;
    }

    hm.m = u64Log2(max(min_slots, 4)); // Log of closest lower power of two

    // This will make m log of closest upper power of two
    hm_UpdateConstants(&hm);

    hm.keys   = hm.allocator(MemAllocMode_ALLOC,
                             (hm.total_slots) * sizeof(*(hm.keys)),
                             NULL, MEM_ALLOCATOR_USER_DATA);
    hm.values = hm.allocator(MemAllocMode_ALLOC,
                             (hm.total_slots) * sizeof(*(hm.values)),
                             NULL, MEM_ALLOCATOR_USER_DATA);

    // For NULL keys
    hm.keys[0].hash = hm_SetFlag(hm.keys[0].hash, HM_Flag_FILLED);
    hm.filled_slots = 1;

    return hm;
}

header_function
void hmDelete (Hashmap hm)
{
    hm.allocator(MemAllocMode_DEALLOC, 0, hm.keys,   NULL);
    hm.allocator(MemAllocMode_DEALLOC, 0, hm.values, NULL);
}

header_function
Size hm_LinearProbeSearch (Hashmap *hm, Uptr key)
{
    if (key == 0) return 0;

    U64 hash = hm_Hash(hm, key);
    U64 hash_real = hm_GetHash(hash);

    Size index = 0, i = 0;
    B32 found = false;

    for (i = 0; !found && (i < hm->total_slots); ++i) {
        index = (hash_real + i) % (hm->total_slots);

        HM_Flag flag = hm_GetFlag(hm->keys[index].hash);
        if ((flag == HM_Flag_FILLED) &&
            (hm->keys[index].key == key)) {
            found = true;
        } else if (flag == HM_Flag_VACATED) {
            continue;
        } else if (flag == HM_Flag_EMPTY) {
            break;
        }
    }

    if (i == hm->total_slots) {
        index = 0;
    }

    Size result = (found ? index : 0);

    return result;
}

header_function
Uptr hm_LinearProbeInsertion (Hashmap *hm,
                              U64 hash, Uptr key, Uptr value)
{
    Uptr result_value = value;

    for (Size i = 0; i < hm->total_slots; ++i) {
        Size index = (hash + i) % (hm->total_slots);
        HM_Flag flag  = hm_GetFlag(hm->keys[index].hash);

        B32 fill_now = false;
        switch (flag) {
            case HM_Flag_FILLED: {
                if (hm->keys[index].key == key) {
                    result_value = hm->values[index];
                    fill_now = true;
                }
            } break;
            case HM_Flag_VACATED:
            case HM_Flag_EMPTY: {
                fill_now = true;
            } break;
        }

        if (fill_now) {
            hm->keys[index].key = key;
            hm->values[index] = value;
            hm->keys[index].hash = hm_SetFlag(hash, HM_Flag_FILLED);
            break;
        }
    }

    return result_value;
}

header_function
Uptr hmInsertI (Hashmap *hm, Uptr key, Uptr value)
{
    if ((key == 0) || (value == 0)) return 0;

    if ((2U * (hm->filled_slots)) > (hm->total_slots)) {
        Size total_slots = hm->total_slots;
        struct HashmapKeys *keys   = hm->keys;
        Uptr *values = hm->values;

        hm_UpdateConstants(hm);

        hm->keys   = (hm->allocator)(MemAllocMode_ALLOC,
                                     sizeof(*(hm->keys)) * hm->total_slots,
                                     NULL, MEM_ALLOCATOR_USER_DATA);
        hm->values = (hm->allocator)(MemAllocMode_ALLOC,
                                     sizeof(*(hm->values)) * hm->total_slots,
                                     NULL, MEM_ALLOCATOR_USER_DATA);

        // For NULL keys
        hm->keys[0].hash = hm_SetFlag(hm->keys[0].hash, HM_Flag_FILLED);

        for (Size i = 1; i < total_slots; ++i) {
            U64 hash_i_old = keys[i].hash;
            Uptr key_i     = keys[i].key;
            Uptr value_i   = values[i];
            HM_Flag flag_i   = hm_GetFlag(hash_i_old);
            U64 hash_i_new = hm_Hash(hm, key_i);
            if (flag_i == HM_Flag_FILLED) {
                hm_LinearProbeInsertion(hm, hash_i_new, key_i, value_i);
            }
        }

        (hm->allocator)(MemAllocMode_DEALLOC, 0, keys,   MEM_ALLOCATOR_USER_DATA);
        (hm->allocator)(MemAllocMode_DEALLOC, 0, values, MEM_ALLOCATOR_USER_DATA);
    }

    U64 hash = hm_Hash(hm, key);

    Uptr result_value = hm_LinearProbeInsertion(hm, hash, key, value);
    hm->filled_slots += 1;

    return result_value;
}

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wbad-function-cast"
# endif

header_function
void* hmInsert (Hashmap *hm, void *key, void *value)
{
    void *result_value = (void*)hmInsertI(hm, (Uptr)key, (Uptr)value);
    return result_value;
}

header_function
Uptr hmLookupI (Hashmap *hm, Uptr key)
{
    Size location = hm_LinearProbeSearch(hm, key);
    Uptr result_value = hm->values[location];

    return result_value;
}

header_function
void* hmLookup (Hashmap *hm, void *key)
{
    void *result_value = (void*)hmLookupI(hm, (Uptr)key);
    return result_value;
}

header_function
Uptr hmRemoveI (Hashmap *hm, Uptr key)
{
    Size location = hm_LinearProbeSearch(hm, key);

    Uptr result_value = 0;
    if (location != 0) {
        U64 hash = hm->keys[location].hash;
        hm->keys[location].hash = hm_SetFlag(hash, HM_Flag_VACATED);
        hm->filled_slots -= 1;
        result_value = hm->values[location];
    }

    return result_value;
}

header_function
void* hmRemove (Hashmap *hm, void *key)
{
    void *result_value = (void*)hmRemoveI(hm, (Uptr)key);
    return result_value;
}

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic pop
# endif

header_function
void hmUnitTest (void)
{
    Hashmap hm = hmCreate(memDefaultAllocator, 1);

    /* No Entries */
    utTest(hmLookup(&hm, (void*)0) == NULL);
    utTest(hmLookup(&hm, (void*)1) == NULL);
    utTest(hmLookup(&hm, (void*)2) == NULL);

    /* Insertion Test */
    Size f0 = hm.filled_slots;

    hmInsert(&hm, (void*)1, (void*)1);
    utTest(hm.filled_slots == (f0 + 1));
    utTest(hmLookup(&hm, (void*)0) == NULL);
    utTest(hmLookup(&hm, (void*)1) == (void*)1);
    utTest(hmLookup(&hm, (void*)2) == NULL);

    hmInsert(&hm, (void*)2, (void*)42);
    utTest(hm.filled_slots == (f0 + 2));
    utTest(hmLookup(&hm, (void*)0) == NULL);
    utTest(hmLookup(&hm, (void*)1) == (void*)1);
    utTest(hmLookup(&hm, (void*)2) == (void*)42);

    /* Duplicate Key */
    void *v1 = hmInsert(&hm, (void*)2, (void*)24);
    utTest(v1 == (void*)42);
    utTest(hmLookup(&hm, (void*)0) == NULL);
    utTest(hmLookup(&hm, (void*)1) == (void*)1);
    utTest(hmLookup(&hm, (void*)2) == (void*)24);

    /* Removal Test */
    void *v2 = hmRemove(&hm, (void*)2);
    utTest(v2 == (void*)24);
    utTest(hmLookup(&hm, (void*)2) == NULL);

    void *v3 = hmRemove(&hm, (void*)1);
    utTest(v3 == (void*)1);
    utTest(hmLookup(&hm, (void*)1) == NULL);

    /* NULL Check */
    Size f1 = hm.filled_slots;
    hmInsert(&hm, (void*)0, (void*)1);
    utTest(hm.filled_slots == f1);
    utTest(hmLookup(&hm, (void*)0) == (void*)0);

    Size f2 = hm.filled_slots;
    hmRemove(&hm, (void*)0);
    utTest(hm.filled_slots == f2);
    utTest(hmLookup(&hm, (void*)0) == (void*)0);

    /* Expansion Test */
    hmInsert(&hm, (void*)3, (void*)33);
    utTest(hmLookup(&hm, (void*)3) == (void*)33);

    hmInsert(&hm, (void*)4, (void*)44);
    utTest(hmLookup(&hm, (void*)4) == (void*)44);

    hmInsert(&hm, (void*)5, (void*)55);
    utTest(hmLookup(&hm, (void*)5) == (void*)55);

    hmInsert(&hm, (void*)6, (void*)66);
    utTest(hmLookup(&hm, (void*)6) == (void*)66);

    hmInsert(&hm, (void*)7, (void*)77);
    utTest(hmLookup(&hm, (void*)7) == (void*)77);

    hmInsert(&hm, (void*)8, (void*)88);
    utTest(hmLookup(&hm, (void*)8) == (void*)88);

    hmInsert(&hm, (void*)9, (void*)99);
    utTest(hmLookup(&hm, (void*)9) == (void*)99);

    /* Removal after Expansion */

    hmRemove(&hm, (void*)3);
    utTest(hmLookup(&hm, (void*)3) == NULL);

    hmRemove(&hm, (void*)4);
    utTest(hmLookup(&hm, (void*)4) == NULL);

    hmRemove(&hm, (void*)5);
    utTest(hmLookup(&hm, (void*)5) == NULL);

    hmRemove(&hm, (void*)6);
    utTest(hmLookup(&hm, (void*)6) == NULL);

    hmRemove(&hm, (void*)7);
    utTest(hmLookup(&hm, (void*)7) == NULL);

    hmRemove(&hm, (void*)8);
    utTest(hmLookup(&hm, (void*)8) == NULL);

    hmRemove(&hm, (void*)9);
    utTest(hmLookup(&hm, (void*)9) == NULL);

    hmDelete(hm);

    return;
}

/* ==========================
 * Text Interning
 */

/* API ----------------------------------------------------
 * Char* internBuffer (InternTable *it, Char *start, Char *end)
 * Char* internString (InternTable *it, Char *str)
 */

// TODO(naman): Replace all the Char* with Txt after implementing a proper string type

typedef struct {
    Size string_index;
    Size string_length;
    Size child_index;
    B32 used;
    B32 has_child;
} InternElem;

typedef struct {
    InternElem base_elems[256];

    InternElem *more_elems;
    Size more_elem_count;
    Size more_elem_next_entry;

    Char *strings;
    Size strings_size;
    Size strings_next_index;
} InternTable;

header_function
InternTable internCreate (void)
{
    InternTable i = {0};

    i.strings_size = 1024;
    i.strings = memAlloc(i.strings_size);

    return i;
}

header_function
Char *intern_StringInsert (InternTable *it, InternElem *elems, Size index,
                           Char *start, Size string_len)
{
    elems[index].used = true;
    elems[index].string_index = it->strings_next_index;
    elems[index].string_length = string_len;

    if (it->strings_next_index + string_len > it->strings_size) {
        it->strings_size = string_len + (2 * it->strings_size);
        it->strings = memRealloc(it->strings, it->strings_size);
    }

    memcpy(&(it->strings[it->strings_next_index]), start, string_len);
    it->strings[it->strings_next_index + string_len] = '\0';
    it->strings_next_index = it->strings_next_index + string_len + 1;

    Char *result = &(it->strings[elems[index].string_index]);
    return result;
}

header_function
U8 intern_PearsonHash (Char *string, Size len)
{
    // NOTE(naman): Pearson's hash for 8-bit hashing
    // https://en.wikipedia.org/wiki/Pearson_hashing
    persistent_value U8 hash_lookup_table[256] = {
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

    U8 hash = (U8)len;
    for (Size i = 0; i < len; i++) {
        hash = hash_lookup_table[hash ^ string[i]];
    }

    return hash;
}

// Intern a non-zero terminated string
//     start points to the first character.
//     end points to the character after the last character.
header_function
Char * internStringBuffer (InternTable *it, Char *start, Char *end)
{
    Size len = (Size)(Dptr)(end - start); // Length of string

    U8 base_index = intern_PearsonHash(start, len);

    if (it->base_elems[base_index].used) {
        // Our string has probably been inserted already.
        // (or atleast some string with same hash has been inserted :)
        Char *value = &(it->strings[it->base_elems[base_index].string_index]);

        if ((it->base_elems[base_index].string_length == len) &&
            (strncmp(value, start, len) == 0)) {
            // This is our string, return it
            return value;
        }

        // That wasn't our string, keep searching for it...
        // Perhaps our string lies deeper in a rabbit hole?
        if (it->base_elems[base_index].has_child) {
            // More strings with same hash have been added, our string might be one of them!
            Size i = it->base_elems[base_index].child_index;

            do {
                value = &(it->strings[it->more_elems[i].string_index]);
                if ((it->more_elems[i].string_length == len) &&
                    (strncmp(value, start, len) == 0)) {
                    // Found it, this is our string. Return it
                    return value;
                }

                i = it->more_elems[i].child_index;
            } while (it->more_elems[i].has_child);


            // No, it wasn't.
            // That means our string hasn't been added yet.
            // So, let's add it!!! (and return it)
            if (it->more_elem_next_entry == it->more_elem_count) {
                it->more_elem_count = 2 * it->more_elem_count;
                it->more_elems = memRealloc(it->more_elems,
                                               sizeof(*(it->more_elems)) * it->more_elem_count);
            }

            Size entry_index = it->more_elem_next_entry;
            it->more_elem_next_entry += 1;

            it->more_elems[i].has_child = true;
            it->more_elems[i].child_index = entry_index;


            Char *result = intern_StringInsert(it, it->more_elems, entry_index, start, len);
            return result;
        } else {
            // Base didn't have a child, so we'll make it adopt our string by necessary force.
            if (it->more_elems != NULL) {
                // Though a rabbit hole exists, our string is not yet in it.
                // So by gods, we are going to shove it right in.
                if (it->more_elem_next_entry == it->more_elem_count) {
                    it->more_elem_count = 2 * (it->more_elem_count);
                    it->more_elems = memRealloc(it->more_elems,
                                                   sizeof(*(it->more_elems)) *
                                                   it->more_elem_count);
                }

                Size entry_index = it->more_elem_next_entry;
                it->more_elem_next_entry += 1;

                it->base_elems[base_index].has_child = true;
                it->base_elems[base_index].child_index = entry_index;

                Char *result = intern_StringInsert(it, it->more_elems, entry_index, start, len);
                return result;
            } else {
                // There is no rabbit hole to begin with, there never was! :(
                // But we, the brave earthlings, are going to make one. Or die trying.
                it->more_elem_count = 256;
                it->more_elems = memRealloc(it->more_elems,
                                               sizeof(*(it->more_elems)) * it->more_elem_count);

                // And now let's put in the element and return it.
                Size entry_index = 0;
                it->more_elem_next_entry = 1;

                it->base_elems[base_index].has_child = true;
                it->base_elems[base_index].child_index = entry_index;

                Char *result = intern_StringInsert(it, it->more_elems, entry_index, start, len);
                return result;
            }
        }
    } else {
        // The string is a lie.
        // Tyler Durden never existed.
        // God is dead.
        // E.g., string has not been inserted before, insert it and return a pointer to it.
        Char *result = intern_StringInsert(it, it->base_elems, base_index, start, len);
        return result;
    }
}

// Intern a non-zero terminated string
//     start points to the first character.
//     end points to the character after the last character.
header_function
Char * internCheckStringBuffer (InternTable *it, Char *start, Char *end)
{
    Size len = (Size)(Dptr)(end - start); // Length of string

    U8 base_index = intern_PearsonHash(start, len);

    if (it->base_elems[base_index].used) {
        // Our string has probably been inserted already.
        // (or atleast some string with same hash has been inserted :)
        Char *value = &(it->strings[it->base_elems[base_index].string_index]);

        if ((it->base_elems[base_index].string_length == len) &&
            (strncmp(value, start, len) == 0)) {
            // This is our string, return it
            return value;
        }

        // That wasn't our string, keep searching for it...
        // Perhaps our string lies deeper in a rabbit hole?
        if (it->base_elems[base_index].has_child) {
            // More strings with same hash have been added, our string might be one of them!
            Size i = it->base_elems[base_index].child_index;

            do {
                value = &(it->strings[it->more_elems[i].string_index]);
                if ((it->more_elems[i].string_length == len) &&
                    (strncmp(value, start, len) == 0)) {
                    // Found it, this is our string. Return it
                    return value;
                }

                i = it->more_elems[i].child_index;
            } while (it->more_elems[i].has_child);


            // No, it wasn't.
            // That means our string hasn't been added yet.
            return NULL;
        } else {
            // Base didn't have a child, so that's that.
            return NULL;
        }
    } else {
        // The string is a lie.
        // Tyler Durden never existed.
        // God is dead.
        return NULL;
    }
}

// Intern a zero-terminated C-string
header_function
Char* internString (InternTable *it, Char *str)
{
    Size len = strlen(str);
    Char *result = internStringBuffer(it, str, str + len);

    return result;
}

// Check if a zero-terminated C-string has already been interned
header_function
Char* internCheckString (InternTable *it, Char *str)
{
    Size len = strlen(str);
    Char *result = internCheckStringBuffer(it, str, str + len);

    return result;
}

header_function
void internUnitTest (void)
{
    Char x[] = "Hello";
    Char y[] = "Hello";

    InternTable intern_table = internCreate();

    utTest(x != y);

    Char *y_interned = internString(&intern_table, y);
    Char *x_interned = internString(&intern_table, x);
    utTest(x_interned == y_interned);

    Char z[] = "World";
    Char *z_interned = internString(&intern_table, z);
    utTest(x_interned != z_interned);

    Char p[] = "Hello!!";
    Char *p_interned = internString(&intern_table, p);
    utTest(x_interned != p_interned);

    // TODO(naman): Write tests to see what happens if two strings with same hash are interned.

    return;
}

/* ==========================
 * Intrusive Linked List
 */

/* API ----------------------------------------------------
 * List* listInsert (List *list, List_Node *node)
 * List_Node* listRemove (List *list)
 */

typedef struct List_Node {
  struct List_Node* next;
  struct List_Node* prev;
} List_Node;

typedef List_Node List;

# define listInsert(l, ln) ((l) = list_Insert(l, ln))
# define listRemove(l) (list_Remove(l))

header_function
List_Node* list_Insert (List_Node *l, List_Node *ln)
{
    if (l == NULL) {
        l = memAlloc(sizeof(*l));
        l->prev = l;
        l->next = l;
    }

    List_Node *next = l->next;

    l->next = ln;

    ln->next = next;
    ln->prev = l;

    next->prev = ln;

    return l;
}

header_function
List_Node* list_Remove (List_Node *l)
{
    if (l == NULL) return NULL;
    if (l == l->next) return NULL;

    List_Node *removee = l->next;
    List_Node *successor = removee->next;

    l->next = successor;
    successor->prev = l;

    removee->next = NULL;
    removee->prev = NULL;

    return removee;
}

/* ****************************************************************************
 * UNIT TESTS FOR EVERYTHING IN THIS FILE *************************************
 */

header_function
void nlibUnitTest (void)
{
    sbufUnitTest();
    hmUnitTest();
    internUnitTest();

    return;
}


#define NLIB_H_INCLUDE_GUARD
#endif // NLIB_H_INCLUDE_GUARD
