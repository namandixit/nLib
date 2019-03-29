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
# elif defined(COMPILER_CLANG)
#  if defined(__i386__)
#   define ARCH_X86
#  elif defined(__x86_64__)
#   define ARCH_x64
#  endif
# endif

# if !defined(ARCH_X86) && !defined(ARCH_X64)
#  error Architecture not supported
# endif


/* ===========================
 * Standard C Headers Includes
 */

# if defined(COMPILER_MSVC)
#  pragma warning(push)
#   pragma warning(disable:4668)
# endif

# include <stddef.h>

# if defined(COMPILER_MSVC)
#  pragma warning(pop)
# endif

# include <stdint.h>
# include <stdlib.h>
# include <stdarg.h>

/* ===========================
 * Misc C Headers Includes
 */

// stb_sprintf
# define STB_SPRINTF_IMPLEMENTATION
# define STB_SPRINTF_STATIC

# if defined(COMPILER_MSVC)
#  pragma warning(push)
#   pragma warning(disable:4100)
#   pragma warning(disable:4820)
#    include "stb/stb_sprintf.h"
#  pragma warning(pop)
# endif

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wlanguage-extension-token"
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wcast-qual"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wdouble-promotion"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#   pragma clang diagnostic ignored "-Wconditional-uninitialized"
#    include "stb/stb_sprintf.h"
#  pragma clang diagnostic pop
# endif

/* ===========================
 * Platform Headers Includes
 */

# if defined(OS_WINDOWS)

#  if defined(COMPILER_MSVC)
#   pragma warning(push)
#    pragma warning(disable:4255)
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

typedef uint8_t              U8;
typedef uint16_t             U16;
typedef uint32_t             U32;
typedef uint64_t             U64;

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

# if !defined(max)
#  define max(x, y) ((x) >= (y) ? (x) : (y))
# endif

# if !defined(min)
# define min(x, y) ((x) <= (y) ? (x) : (y))
# endif

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

# elif defined(COMPILER_CLANG)

#  include <stdalign.h>

# endif


/* =======================
 * OS Specific Hacks
 */

# if defined(OS_WINDOWS)

header_function
void reportDebug(Char *format, ...)
{
    Char buffer[2048] = {0};

    va_list ap;

    va_start(ap, format);
    stbsp_vsnprintf(buffer, 2048, format, ap);
    va_end(ap);

    OutputDebugString(buffer);

    return;
}

header_function
void reportDebugV(Char *format, va_list ap)
{
    Char buffer[2048] = {0};
    stbsp_vsnprintf(buffer, 2048, format, ap);
    OutputDebugString(buffer);

    return;
}

#  if defined(BUILD_INTERNAL)
#   define report(...)  reportDebug(__VA_ARGS__)
#   define reportv(...) reportDebugV(__VA_ARGS__)
#   define breakpoint() __debugbreak()
#   define quit() breakpoint()
#  else
#   define report(...)
#   define reportv(...)
#   define quit() abort()
#  endif

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

#  if defined(BUILD_INTERNAL)
#   define report(format, ...)      fprintf(stderr, format, ##__VA_ARGS__)
#   define reportv(format, va_list) vfprintf(stderr, format, va_list)
#   define breakpoint() __asm__ volatile("int $0x03")
#   define quit() breakpoint()
#  else
#   define report(format, ...)
#   define reportv(format, va_list)
#   define quit() abort()
#  endif

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
    U64 result = 64 - (__builtin_clzll(x) + 1);
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
    U64 upper = product >> 64, lower = product;
    U64 log_upper = u64Log2(upper);
    U64 shift_amount = 64 - (log_upper + 1);
    upper = (upper << shift_amount) | (lower >> log_upper);

    U64 result = upper * a;

    return result;
}

# endif

/* ===============================
 * Custom Memory Allocator Helpers
 */

#define MEM_MAX_ALIGN_MINUS_ONE (alignof(max_align_t) - 1u)
#define memAlignUp(p) (((p) + MEM_MAX_ALIGN_MINUS_ONE) & (~ MEM_MAX_ALIGN_MINUS_ONE))
#define memAlignDown(p) (mem_ALIGN_UP((p) - MEM_MAX_ALIGN_MINUS_ONE))


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

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wcast-align"
# endif

typedef enum MemAllocMode {
    MemAllocMode_NONE,
    MemAllocMode_ALLOC,
    MemAllocMode_REALLOC,
    MemAllocMode_DEALLOC,
    MemAllocMode_DEALLOC_ALL,
} MemAllocMode;

# define MEM_ALLOCATOR(allocator)               \
    void* allocator(MemAllocMode mode,          \
                    Size size, void* old_ptr,   \
                    void *allocator_data)
typedef MEM_ALLOCATOR(MemAllocator);

/* =============================
 * Default Memory Allocator
 */

struct MemCRT_Header {
    Size size;
};

# define memCRTAlloc(size)        memCRT(MemAllocMode_ALLOC,   size, NULL, NULL)
# define memCRTRealloc(ptr, size) memCRT(MemAllocMode_REALLOC, size, ptr,  NULL)
# define memCRTDealloc(ptr)       memCRT(MemAllocMode_DEALLOC, 0,    ptr,  NULL)

header_function
MEM_ALLOCATOR(memCRT)
{
    unused_variable(allocator_data);
    switch (mode) {
        case MemAllocMode_ALLOC: {
            Size memory_size = memAlignUp(size);
            Size header_size = memAlignUp(sizeof(struct MemCRT_Header));
            Size total_size = memory_size + header_size;

            Byte *mem = malloc(total_size);

            struct MemCRT_Header *header = (struct MemCRT_Header *)mem;
            header->size = memory_size;

            Byte *result = mem + header_size;

            memset(result, 0, memory_size);

            return result;
        } break;

        case MemAllocMode_REALLOC: {
            Size memory_size = memAlignUp(size);
            Size header_size = memAlignUp(sizeof(struct MemCRT_Header));
            Size total_size = memory_size + header_size;

            Byte *mem = malloc(total_size);

            struct MemCRT_Header *header = (struct MemCRT_Header *)mem;
            header->size = memory_size;

            Byte *result = mem + header_size;

            Byte *previous_mem = (Byte*)old_ptr - header_size;
            struct MemCRT_Header *previous_header = (struct MemCRT_Header *)previous_mem;
            Size previous_size = previous_header->size;

            memcpy(result, old_ptr, previous_size);
            memset(result + previous_size, 0, memory_size - previous_size);

            memCRTDealloc(old_ptr);

            return result;
        } break;

        case MemAllocMode_DEALLOC: {
            Size header_size = memAlignUp(sizeof(struct MemCRT_Header));
            Byte *mem = (Byte*)old_ptr - header_size;
            free(mem);
        } break;

        case MemAllocMode_DEALLOC_ALL: {
            // TODO(naman): Maybe we should use a off-the-shelf malloc that allows this?
        } break;

        case MemAllocMode_NONE: {
            breakpoint();
        } break;
    }

    return NULL;
}

/* =============================
 * Tree Memory Allocator
 */

struct MemTree_Header {
    void *child;
    void *parent;
    void *next;
    void *previous;
};

# define memTree_GetHeader(ptr) ((struct MemTree_Header*)               \
                                 ((Byte*)(ptr) - memAlignUp(sizeof(struct MemTree_Header))))

# define memTreeAlloc(size, parent) memTree(MemAllocMode_ALLOC,   size, NULL, parent)
# define memTreeDealloc(ptr)        memTree(MemAllocMode_DEALLOC, 0,    ptr,  NULL)

header_function
void memTree_DeallocRecursive (void *node)
{
    struct MemTree_Header *node_header = memTree_GetHeader(node);
    void *child = node_header->child;

    if (child != NULL) {
        void *now = child;

        while (now != NULL) {
            struct MemTree_Header *now_header = memTree_GetHeader(now);
            void *sibling = now_header->next;
            memTree_DeallocRecursive(now);
            node_header->child = sibling; // To prevent dangling pointers
            now = sibling;
        }
    }

    claim(node_header->child == NULL);

    if (node_header->previous != NULL) {
        struct MemTree_Header *previous_header = memTree_GetHeader(node_header->previous);
        previous_header->next = node_header->next;
    }

    if (node_header->next != NULL) {
        struct MemTree_Header *next_header = memTree_GetHeader(node_header->next);
        next_header->previous = node_header->previous;
    }

    if (node_header->parent != NULL) {
        struct MemTree_Header *parent_header = memTree_GetHeader(node_header->parent);
        parent_header->child = node_header->next;
    }

    memCRTDealloc(node_header);

    return;
}

header_function
MEM_ALLOCATOR(memTree)
{
    switch (mode) {
        case MemAllocMode_ALLOC: {
            void *parent = allocator_data;

            Size requested_size = size;
            Size allocated_size = memAlignUp(requested_size);

            Size header_size = sizeof(struct MemTree_Header);
            Size aligned_header_size = memAlignUp(header_size);

            Size total_size = allocated_size + aligned_header_size;

            void *mem = memCRTAlloc(total_size);

            void *result = (Byte*)mem + aligned_header_size;

            struct MemTree_Header *header = mem;
            header->parent = parent;
            header->child = NULL;
            header->next = NULL;
            header->previous = NULL;

            if (parent == NULL) {
                return result;
            }

            struct MemTree_Header *parent_header = memTree_GetHeader(parent);

            if (parent_header->child == NULL) {
                parent_header->child = result;
                return result;
            }

            void *first_child = parent_header->child;
            struct MemTree_Header *first_child_header = memTree_GetHeader(first_child);

            void *last_sibling = first_child;
            void *sibling = first_child_header->next;

            while (sibling != NULL) {
                struct MemTree_Header *sibling_header = memTree_GetHeader(sibling);
                last_sibling = sibling;
                sibling = sibling_header->next;
            }

            struct MemTree_Header *last_sibling_header = memTree_GetHeader(last_sibling);
            last_sibling_header->next = result;
            header->previous = last_sibling;

            return result;
        } break;

        case MemAllocMode_REALLOC: {
            // TODO(naman): Just allocate a new node and patch-up the the pointers?
        } break;

        case MemAllocMode_DEALLOC: {
            if (old_ptr == NULL) {
                return NULL;
            }

            memTree_DeallocRecursive(old_ptr);

            return NULL;
        } break;

        case MemAllocMode_DEALLOC_ALL: {
            // TODO(naman): Should we somehow save the top level parent in the header?
        } break;

        case MemAllocMode_NONE: break;
    }

    return NULL;
}

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
 * Size  sbufAdd (T *ptr, T elem)
 * void  sbufDelete (T *ptr)
 * Size  sbufLen (T *ptr)
 * Size  sbufCap (T *ptr)
 * void* sbufToMemTree (T *ptr, void *parent)
 */

typedef struct Sbuf_Header {
    Size cap;
    Size len;
    Byte buf[];
} Sbuf_Header;

# define sbuf_GetHeader(sb) ((Sbuf_Header*)(void*)((Byte*)(sb) - offsetof(Sbuf_Header, buf)))
# define sbufLen(sb)        (((sb) != NULL) ? sbuf_GetHeader(sb)->len : 0U)
# define sbufCap(sb)        (((sb) != NULL) ? sbuf_GetHeader(sb)->cap : 0U)
# define sbuf_Fit(sb, n)    (sbufLen(sb) + (n) < sbufCap(sb) ?          \
                             0 :                                        \
                             ((sb) = sbuf_Grow(sb, sbufLen(sb) + (n), sizeof(*(sb)))))
# define sbufAdd(sb, ...)   (sbuf_Fit(sb, 1),                   \
                             (sb)[sbufLen(sb)] = (__VA_ARGS__), \
                             sbuf_GetHeader(sb)->len++)
# define sbufDelete(sb)     (((sb) != NULL) ?                           \
                             (memCRTDealloc(sbuf_GetHeader(sb)), (sb) = NULL) : \
                             0)

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wcast-align"
# endif

header_function
void* sbuf_Grow (void *buf, Size new_cap, Size elem_size)
{
    claim(sbufCap(buf) <= ((SIZE_MAX - 1)/2));

    Size old_len = sbufLen(buf);

    Size new_cap_max = max(1 + 2 * sbufCap(buf), new_cap);
    claim(new_cap <= new_cap_max);
    claim(new_cap_max <= (SIZE_MAX - offsetof(Sbuf_Header, buf))/elem_size);

    Size new_size = (new_cap_max * elem_size) + sizeof(Sbuf_Header);

    Sbuf_Header *new_header = NULL;

    if (buf != NULL) {
        new_header = memCRTRealloc(sbuf_GetHeader(buf), new_size);
        new_header->len = old_len;
    } else {
        new_header = memCRTAlloc(new_size);
        new_header->len = 0;
    }

    new_header->cap = new_cap_max;

    return new_header->buf;
}

# define sbufToMemTree(sb, parent) sbuf_ToMemTree(sb, sizeof(*sb), parent)

header_function
void* sbuf_ToMemTree (void *sbuf, Size elem_size, void *parent)
{
    if (sbuf == NULL) return NULL;

    struct Sbuf_Header *hdr = sbuf_GetHeader(sbuf);
    Size total_size = elem_size * hdr->len;

    void *mem = memTreeAlloc(total_size, parent);

    memcpy(mem, sbuf, total_size);

    return mem;
}

header_function
void sbufUnitTest (void)
{
    S32 *buf = NULL;

    sbufAdd(buf, 42);
    utTest(buf != NULL);

    sbufAdd(buf, 1234);

    utTest(sbufLen(buf) == 2);
    utTest(sbufCap(buf) >= sbufLen(buf));

    utTest(buf[0] == 42);
    utTest(buf[1] == 1234);

    sbufDelete(buf);

    utTest(buf == NULL);
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
        Sptr key;
    } *keys;

    Sptr *values;

    MemAllocator *allocator;

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
 * Sptr    hmInsertI (Hashmap *hm, Sptr key, Sptr value);
 * void*   hmLookup  (Hashmap *hm, void *key);
 * Sptr    hmLookupI (Hashmap *hm, Sptr key);
 * void*   hmRemove  (Hashmap *hm, void *key);
 * Sptr    hmRemoveI (Hashmap *hm, Sptr key);
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
U64 hm_Hash (Hashmap *hm, Sptr key)
{
    U64 result =  ((hm->a * *((Uptr*)&key)) + hm->b) & (0xFFFFFFFFFFFFFFFFULL >> (64 - hm->m));
    return result;
}

header_function
Hashmap hmCreate (MemAllocator allocator,
                  Size min_slots)
{
    Hashmap hm = {0};
    hm.allocator = allocator;
    hm.m = u64Log2(min_slots); // Log of closest lower power of two

    // This will make m log of closest upper power of two
    hm_UpdateConstants(&hm);

    hm.keys   = allocator(MemAllocMode_ALLOC,
                          (hm.total_slots) * sizeof(*(hm.keys)),
                          NULL, NULL);
    hm.values = allocator(MemAllocMode_ALLOC,
                          (hm.total_slots) * sizeof(*(hm.values)),
                          NULL, NULL);

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
Size hm_LinearProbeSearch (Hashmap *hm, Sptr key)
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
Sptr hm_LinearProbeInsertion (Hashmap *hm,
                              U64 hash, Sptr key, Sptr value)
{
    Sptr result_value = value;

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
Sptr hmInsertI (Hashmap *hm, Sptr key, Sptr value)
{
    if ((key == 0) || (value == 0)) return 0;

    if ((2U * (hm->filled_slots)) > (hm->total_slots)) {
        Size total_slots = hm->total_slots;
        struct HashmapKeys *keys   = hm->keys;
        Sptr *values = hm->values;

        hm_UpdateConstants(hm);

        hm->keys   = (hm->allocator)(MemAllocMode_ALLOC,
                                     sizeof(*(hm->keys)) * hm->total_slots,
                                     NULL, NULL);
        hm->values = (hm->allocator)(MemAllocMode_ALLOC,
                                     sizeof(*(hm->values)) * hm->total_slots,
                                     NULL, NULL);

        // For NULL keys
        hm->keys[0].hash = hm_SetFlag(hm->keys[0].hash, HM_Flag_FILLED);

        for (Size i = 1; i < total_slots; ++i) {
            U64 hash_i_old = keys[i].hash;
            Sptr key_i     = keys[i].key;
            Sptr value_i   = values[i];
            HM_Flag flag_i   = hm_GetFlag(hash_i_old);
            U64 hash_i_new = hm_Hash(hm, key_i);
            if (flag_i == HM_Flag_FILLED) {
                hm_LinearProbeInsertion(hm, hash_i_new, key_i, value_i);
            }
        }

        (hm->allocator)(MemAllocMode_DEALLOC, 0, keys,   NULL);
        (hm->allocator)(MemAllocMode_DEALLOC, 0, values, NULL);
    }

    U64 hash = hm_Hash(hm, key);

    Sptr result_value = hm_LinearProbeInsertion(hm, hash, key, value);
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
    void *result_value = (void*)hmInsertI(hm, (Sptr)key, (Sptr)value);
    return result_value;
}

header_function
Sptr hmLookupI (Hashmap *hm, Sptr key)
{
    Size location = hm_LinearProbeSearch(hm, key);
    Sptr result_value = hm->values[location];

    return result_value;
}

header_function
void* hmLookup (Hashmap *hm, void *key)
{
    void *result_value = (void*)hmLookupI(hm, (Sptr)key);
    return result_value;
}

header_function
Sptr hmRemoveI (Hashmap *hm, Sptr key)
{
    Size location = hm_LinearProbeSearch(hm, key);

    Sptr result_value = 0;
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
    void *result_value = (void*)hmRemoveI(hm, (Sptr)key);
    return result_value;
}

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic pop
# endif

header_function
void hmUnitTest (void)
{
    Hashmap hm = hmCreate(memCRT, 1);

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
 * Char* internBuffer (Hashmap *hm, Char *start, Char *end)
 * Char* internString (Hashmap *hm, Char *str)
 */

// TODO(naman): Replace all the Char* with Txt after implementing a proper string type

// Intern a non-zero terminated string
//     start points to the first character.
//     end points to the character after the last character.
header_function
Char* internBuffer (Hashmap *hm, Char *start, Char *end)
{
    Size len = (Size)(Dptr)(end - start); // Length of string

    // Hash String
    // Jenkins One-at-a-time Hash
    // https://en.wikipedia.org/wiki/Jenkins_hash_function#one-at-a-time
    U32 hash = 0;
    {
        // TODO(naman): Move this to txtHash once a string type is implemented.
        for (Size i = 0; i < len; i++) {
            hash += (U32)(start[i]);
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }

        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);
    }

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wbad-function-cast"
# endif

    void *value = (void*)hmLookupI(hm, hash);

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic pop
# endif

    // TODO(naman): Use strncmp here and move the allocation in the else-branch
    Char *str = memCRTAlloc(len + 1);
    memcpy(str, start, len);
    str[len] = '\0';

    if ((value != 0) && (strcmp(value, str) == 0)) {
            // If the string has already been interned, return the pointer to that internment;
        memCRTDealloc(str);
        return value;
    } else {
        // else, create a new internment.
        hmInsertI(hm, hash, (Sptr)str);

        return str;
    }
}

// Intern a zero-terminated C-string
header_function
Char* internString (Hashmap *hm, Char *str)
{
    Size len = strlen(str);
    Char *result = internBuffer(hm, str, str + len);

    return result;
}

header_function
void internUnitTest (void)
{
    Char x[] = "Hello";
    Char y[] = "Hello";

    Hashmap intern_table = hmCreate(memCRT, 8);

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

    return;
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
