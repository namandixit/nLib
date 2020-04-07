/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 */

#if !defined(NLIB_MEMORY_H_INCLUDE_GUARD)

# define MEM_MAX_ALIGN_MINUS_ONE (alignof(max_align_t) - 1u)
# define memAlignUp(p) (((p) + MEM_MAX_ALIGN_MINUS_ONE) & (~ MEM_MAX_ALIGN_MINUS_ONE))
# define memAlignDown(p) (memAlignUp((p) - MEM_MAX_ALIGN_MINUS_ONE))

# define memBytesFromBits(b) (((b)+(CHAR_BIT-1))/(CHAR_BIT))

# define memSetBit(array, index)                                 \
    ((array)[(index)/CHAR_BIT] |= (1U << ((index)%CHAR_BIT)))
# define memResetBit(array, index)                               \
    ((array)[(index)/CHAR_BIT] &= ~(1U << ((index)%CHAR_BIT)))
# define memToggleBit(array, index)                              \
    ((array)[(index)/CHAR_BIT] ^= ~(1U << ((index)%CHAR_BIT)))
# define memTestBit(array, index)                                \
    ((array)[(index)/CHAR_BIT] & (1U << ((index)%CHAR_BIT)))

/* ****************************************************************************
 * MEMORY ALLOCATORS **********************************************************
 */

# if !defined(NLIB_NO_ALLOCATOR)

typedef enum Memory_Allocator_Mode {
    Memory_Allocator_Mode_NONE,
    Memory_Allocator_Mode_CREATE,
    Memory_Allocator_Mode_ALLOCATE,
    Memory_Allocator_Mode_REALLOCATE,
    Memory_Allocator_Mode_DEALLOCATE,
    Memory_Allocator_Mode_DEALLOCATE_EVERYTHING,
    Memory_Allocator_Mode_GET_ALLOCATION_SIZE,
} Memory_Allocator_Mode;

#  define MEMORY_ALLOCATOR(allocator)           \
    void* allocator(Memory_Allocator_Mode mode, \
                    Size count, Size size,      \
                    void* old_ptr,              \
                    void *data)

typedef MEMORY_ALLOCATOR(Memory_Allocator_Function);

/* =============================
 * dlmalloc Memory Allocator
 */

#  if !defined(NLIB_MALLOC_NO_GLOBAL) && !defined(NLIB_MALLOC_CUSTOM)

#   define MALLOC_DL_GLOBAL

#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wexpansion-to-defined"
#    pragma clang diagnostic ignored "-Wstrict-prototypes"
#    pragma clang diagnostic ignored "-Wpadded"
#    pragma clang diagnostic ignored "-Wsign-conversion"
#    pragma clang diagnostic ignored "-Wnull-pointer-arithmetic"
#    pragma clang diagnostic ignored "-Wcast-align"
#    pragma clang diagnostic ignored "-Wextra-semi-stmt"
#    pragma clang diagnostic ignored "-Wshorten-64-to-32"
#    pragma clang diagnostic ignored "-Wint-to-pointer-cast"
#    pragma clang diagnostic ignored "-Wbad-function-cast"
#    pragma clang diagnostic ignored "-Wreserved-id-macro"
#    pragma clang diagnostic ignored "-Wmissing-prototypes"
#   endif

#   if defined(COMPILER_GCC)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wexpansion-to-defined"
#   endif

#   define USE_DL_PREFIX 1
#   include "nlib/ext/dlmalloc/malloc.c"

#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic pop
#   endif

#   if defined(COMPILER_GCC)
#    pragma GCC diagnostic pop
#   endif

#   define nlib_Malloc(size)        dlmalloc(size)
#   define nlib_Calloc(count, size) dlcalloc(count, size)
#   define nlib_Realloc(ptr, size)  dlrealloc(ptr, size)
#   define nlib_Dealloc(ptr)        dlfree(ptr)

#  endif

#  if defined(NLIB_MALLOC_NO_GLOBAL)

#   define MALLOC_DL_LOCAL

#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wexpansion-to-defined"
#    pragma clang diagnostic ignored "-Wstrict-prototypes"
#    pragma clang diagnostic ignored "-Wpadded"
#    pragma clang diagnostic ignored "-Wsign-conversion"
#    pragma clang diagnostic ignored "-Wnull-pointer-arithmetic"
#    pragma clang diagnostic ignored "-Wcast-align"
#    pragma clang diagnostic ignored "-Wextra-semi-stmt"
#    pragma clang diagnostic ignored "-Wshorten-64-to-32"
#    pragma clang diagnostic ignored "-Wreserved-id-macro"
#    pragma clang diagnostic ignored "-Wcast-qual"
#    pragma clang diagnostic ignored "-Wmissing-prototypes"
#   endif

#   if defined(COMPILER_GCC)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wexpansion-to-defined"
#   endif

#   define USE_DL_PREFIX 1
#   define ONLY_MSPACES 1
#   include "nlib/ext/dlmalloc/malloc.c"

#   if defined(COMPILER_CLANG)
#    pragma clang diagnostic pop
#   endif

#   if defined(COMPILER_GCC)
#    pragma GCC diagnostic pop
#   endif

global_variable thread_local mspace mspace_handle = NULL;
#   define nlib_Malloc(size)        mspace_malloc(mspace_handle, size)
#   define nlib_Calloc(count, size) mspace_calloc(mspace_handle, count, size)
#   define nlib_Realloc(ptr, size)  mspace_realloc(mspace_handle, ptr, size)
#   define nlib_Dealloc(ptr)        mspace_free(mspace_handle, ptr)

#  endif

#  if defined(NLIB_MALLOC_CUSTOM)

#   define MALLOC_CUSTOM

#  endif

# endif

#define NLIB_MEMORY_H_INCLUDE_GUARD
#endif
