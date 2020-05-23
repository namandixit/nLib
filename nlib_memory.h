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

#   define nlib_Malloc(size) ({do{(void)size;}while(0); NULL;})
#   define nlib_Calloc(count, size) ({do{(void)count;(void)size;}while(0); NULL;})
#   define nlib_Realloc(ptr, size) ({do{(void)ptr;(void)size;}while(0); NULL;})
#   define nlib_Dealloc(ptr) ({do{(void)ptr;}while(0); NULL;})

#define NLIB_MEMORY_H_INCLUDE_GUARD
#endif
