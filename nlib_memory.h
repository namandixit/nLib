/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 * SPDX-License-Identifier: BSL-1.0 OR Unlicense
 */

#if !defined(NLIB_MEMORY_H_INCLUDE_GUARD)

# if 0
#  define p(...) say(__VA_ARGS__)
# else
#  define p(...)
# endif

# define MEM_MAX_ALIGN_MINUS_ONE (alignof(max_align_t) - 1u)
# define memAlignUp(p) (((p) + MEM_MAX_ALIGN_MINUS_ONE) & (~ MEM_MAX_ALIGN_MINUS_ONE))
# define memAlignDown(p) (memAlignUp((p) - MEM_MAX_ALIGN_MINUS_ONE))

typedef enum Memory_Allocator_Mode {
    Memory_Allocator_Mode_NONE,
    Memory_Allocator_Mode_ALLOCATE,
    Memory_Allocator_Mode_REALLOCATE,
    Memory_Allocator_Mode_DEALLOCATE,
} Memory_Allocator_Mode;

# define MEMORY_ALLOCATOR(allocator)                    \
    void* allocator (Memory_Allocator_Mode mode,        \
                     Size size,                         \
                     void* old_ptr,                     \
                     void *data)

typedef MEMORY_ALLOCATOR(Memory_Allocator_Function);

/* ****************************************************************************
 * Virtual Allocator **********************************************************
 */

typedef struct Memory_Virtual_Header {
    Size asked_size;
    Size usable_size;
    Size total_size;
} Memory_Virtual_Header;

header_function
MEMORY_ALLOCATOR(memVirtual)
{
    unused_variable(data);

    switch (mode) {
        case Memory_Allocator_Mode_ALLOCATE: {
            Size header_size = memAlignUp(sizeof(Memory_Virtual_Header));
            Size asked_size = memAlignUp(size);

            Size total_size = header_size + (asked_size * KiB(1));

            void *memory = mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                                MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

            Size allocated_pages = (total_size + KiB(4) - 1) / KiB(4);
            Size allocated_size = allocated_pages * KiB(4);
            Size usable_size = allocated_size - header_size;

            Memory_Virtual_Header *header = memory;
            header->asked_size = asked_size;
            header->usable_size = usable_size;
            header->total_size = total_size;

            void *mem = (Byte*)memory + header_size;

            return mem;
        } break;

        case Memory_Allocator_Mode_REALLOCATE: {
            Size header_size = memAlignUp(sizeof(Memory_Virtual_Header));
            Size asked_size = memAlignUp(size);

            Memory_Virtual_Header *old_header = (void*)((Byte*)old_ptr - header_size);

            if (old_header->usable_size >= asked_size) {
                return old_ptr;
            }

            Size asked_size_prev = old_header->asked_size;
            Size total_size_prev = old_header->total_size;

            Size total_size = header_size + (asked_size * KiB(1));

            void *memory = mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                                MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

            Size allocated_pages = (total_size + KiB(4) - 1) / KiB(4);
            Size allocated_size = allocated_pages * KiB(4);
            Size usable_size = allocated_size - header_size;

            Memory_Virtual_Header *header = memory;
            header->asked_size = asked_size;
            header->usable_size = usable_size;
            header->total_size = total_size;

            void *mem = (Byte*)memory + header_size;

            memcpy(mem, old_ptr, asked_size_prev);
            munmap(header, total_size_prev);

            return mem;
        } break;

        case Memory_Allocator_Mode_DEALLOCATE: {
            Size header_size = memAlignUp(sizeof(Memory_Virtual_Header));

            Memory_Virtual_Header *header = (void*)((Byte*)old_ptr - header_size);
            munmap(header, header->total_size);

            return NULL;
        } break;

        case Memory_Allocator_Mode_NONE: {
            claim(false && "Control shouldn't reach here");
            return NULL;
        } break;
    }

    claim(false && "Control shouldn't reach here");
    return NULL;
}

/* ****************************************************************************
 * DLMalloc Allocator *********************************************************
 */

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wexpansion-to-defined"
#  pragma clang diagnostic ignored "-Wstrict-prototypes"
#  pragma clang diagnostic ignored "-Wpadded"
#  pragma clang diagnostic ignored "-Wsign-conversion"
#  pragma clang diagnostic ignored "-Wnull-pointer-arithmetic"
#  pragma clang diagnostic ignored "-Wcast-align"
#  pragma clang diagnostic ignored "-Wextra-semi-stmt"
#  pragma clang diagnostic ignored "-Wshorten-64-to-32"
#  pragma clang diagnostic ignored "-Wint-to-pointer-cast"
#  pragma clang diagnostic ignored "-Wbad-function-cast"
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#  pragma clang diagnostic ignored "-Wmissing-prototypes"
# endif

# if defined(COMPILER_GCC)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wexpansion-to-defined"
# endif

# if defined(NLIB_NO_LIBC)
#  define LACKS_UNISTD_H     1
#  define LACKS_FCNTL_H      1
#  define LACKS_SYS_PARAM_H  1
#  define LACKS_SYS_MMAN_H   1
#  define LACKS_STRINGS_H    1
#  define LACKS_STRING_H     1
#  define LACKS_SYS_TYPES_H  1
#  define LACKS_ERRNO_H      1
#  define LACKS_STDLIB_H     1
#  define LACKS_SCHED_H      1
#  define LACKS_TIME_H       1
#  define NO_MALLINFO        1
#  define NO_MALLOC_STATS    1
#  define HAVE_MORECORE      0 // Disable sbrk, use mmap only
#  define ABORT quit()
#  define fprintf(s, ...) err(__VA_ARGS__)
# endif

# define USE_DL_PREFIX 1
# include "ext/dlmalloc/malloc.c"

# if defined(COMPILER_CLANG)
#  pragma clang diagnostic pop
# endif

# if defined(COMPILER_GCC)
#  pragma GCC diagnostic pop
# endif

header_function
MEMORY_ALLOCATOR(memDL)
{
    unused_variable(data);

    switch (mode) {
        case Memory_Allocator_Mode_ALLOCATE: {
            Size *memory = dlmalloc(size);
            return memory;
        } break;

        case Memory_Allocator_Mode_REALLOCATE: {
            Size *memory = dlrealloc(old_ptr, size);
            return memory;
        } break;

        case Memory_Allocator_Mode_DEALLOCATE: {
            dlfree(old_ptr);
            return NULL;
        } break;

        case Memory_Allocator_Mode_NONE: {
            claim(false && "Control shouldn't reach here");
            return NULL;
        } break;
    }

    claim(false && "Control shouldn't reach here");
    return NULL;
}

# define memAlloc(size)        memDL(Memory_Allocator_Mode_ALLOCATE,   size, NULL, NULL)
# define memRealloc(ptr, size) memDL(Memory_Allocator_Mode_REALLOCATE, size, ptr,  NULL)
# define memDealloc(ptr)       memDL(Memory_Allocator_Mode_DEALLOCATE, 0,    ptr,  NULL)


/* ****************************************************************************
 * Buddy Allocator ************************************************************
 *
 * This is a binary buddy allocator. We use it since our hash table always expands
 * by a factor of 2.
 */

typedef struct Memory_Buddy_Data {
    Byte *free_bits;
    Byte *split_bits;

    struct Memory_Buddy_Data *prev;
    struct Memory_Buddy_Data *next;

    Byte *arena;
    Size arena_size;
    Size leaf_size;
    Size leaf_count;
    Size block_count;
    U64 level_max;
    U64 level_count;
} Memory_Buddy_Data;

typedef struct Memory_Buddy {
    Memory_Buddy_Data *now;
} Memory_Buddy;

global_variable thread_local Memory_Buddy NLIB_GLOBAL_memory_hash_table;

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

#define memb_CountOfBlocksAtLevel(level) (1u << (level))
#define memb_SizeOfBlocksAtLevel(b, level)              \
    (((b)->arena_size)/memb_CountOfBlocksAtLevel(level))
#define memb_IndexOfBlockInLevel(b, ptr, level) /* Zero based */        \
    ((Uptr)(((Byte*)(ptr)) - (b)->arena) / memb_SizeOfBlocksAtLevel(b, level))
#define memb_IndexOfLeaf(b, ptr) /* Zero based */       \
    ((Uptr)((ptr) - ((b)->arena)) / ((b)->leaf_size))
#define memb_PointerToBlockInLevel(b, index, level)                     \
    (((index) * memb_SizeOfBlocksAtLevel(b, level)) + (b)->arena)
#define memb_PreviousBlockInLevel(b, ptr, level)        \
    ((ptr) - memb_SizeOfBlocksAtLevel(b, level))
#define memb_NextBlockInLevel(b, ptr, level)            \
    ((ptr) + memb_SizeOfBlocksAtLevel(b, level))
#define memb_BlocksBeforeThisLevel(level)                               \
    ((1 << (level)) - 1) /* Using sum of GP, blocks before level n are (2ⁿ - 1) */
#define memb_GlobalIndexOfBlock(b, ptr, level)  \
    (memb_BlocksBeforeThisLevel(level) +        \
     memb_IndexOfBlockInLevel(b, ptr, level))
#define memb_IndexOfFirstChild(index)           \
    ((2 * (index)) + 1)
#define memb_IndexOfSecondChild(index)          \
    ((2 * (index)) + 2)
#define memb_IndexOfParent(index)               \
    (((index) - 1) / 2)

internal_function
Memory_Buddy_Data memBuddy_Init (Byte* arena, Size arena_size, Size leaf_size)
{
    Size leaf_count  = arena_size / leaf_size;
    U64 level_max   = mathLog2U64(leaf_count);
    U64 level_count = level_max + 1;
    Size block_count  = (1 << level_count) - 1; // 2ⁿ⁺¹ - 1 where n is level_max (sum of GP)

    Memory_Buddy_Data buddy = {0};
    buddy.arena = arena;
    buddy.arena_size = arena_size;
    buddy.leaf_size = leaf_size;
    buddy.leaf_count = leaf_count;
    buddy.level_max = level_max;
    buddy.level_count = level_count;
    buddy.block_count = block_count;

    return buddy;
}

internal_function
void* memBuddy_Alloc (Memory_Buddy_Data *buddy, Size size)
{
    Size size_real = size;

    if (size_real == 0 || buddy == NULL) {
        return NULL;
    }

    if (size_real < buddy->leaf_size) {
        size_real = buddy->leaf_size;
    }

    if (size_real > buddy->arena_size) {
        return NULL;
    }

    size_real = mathNextPowerOf2U64(size_real);
    // This is the real size that will be allocated

    // "inverse" because the calculation regards the smallest block as level 0
    U64 inverse_level = mathLog2U64(size_real/(buddy->leaf_size));
    U64 level = buddy->level_max - inverse_level; // Actually, largest block is level 0
    p("\tSearch for free block beginning in level %u\n", level);
    Byte *found_block = NULL;
    { // Search for free block beginning in level = level and going upwards (towards level 0)
        Size level_current = level;

        while (level_current <= level) {
            if (level_current == 0) {
                if (bitTest(buddy->free_bits, 0)) {
                    p("\tAllocating entire arena\n");
                    found_block = buddy->arena;
                }
                break;
            }

            Size free_bits_begin = memb_GlobalIndexOfBlock(buddy, buddy->arena, level_current);
            Size free_bits_end = free_bits_begin + memb_CountOfBlocksAtLevel(level_current);

            Size block_size_of_level = memb_SizeOfBlocksAtLevel(buddy, level_current);
            Size blocks_before_this_level = memb_BlocksBeforeThisLevel(level_current);

            p("\t\tScanning in level %u from bits [%u, %u) to find free block\n",
              level_current, free_bits_begin, free_bits_end);
            // Scanning in level_current from [free_bits_begin, free_bits_end) to find free block
            for (Size bi = free_bits_begin; bi < free_bits_end; bi = bi + 1) {
                if (bitTest(buddy->free_bits, bi)) {
                    found_block = (buddy->arena +
                                   (block_size_of_level * (bi - blocks_before_this_level)));
                    break;
                }
            }

            if (found_block == NULL) {
                level_current--;
            } else {
                break;
            }
        }

        if (found_block != NULL) {
            for (Size lvl = level_current + 1; lvl <= level; lvl++) {
                Byte *found_block_sibiling = found_block + memb_SizeOfBlocksAtLevel(buddy, lvl);

                p("\t\tSplitting block in level: %u (block size: %u) in position: %u\n",
                  lvl - 1,
                  memb_SizeOfBlocksAtLevel(buddy, lvl - 1),
                  ((lvl > 1) ?
                   (memb_IndexOfParent(memb_GlobalIndexOfBlock(buddy, found_block, lvl)) -
                    memb_BlocksBeforeThisLevel(lvl-1)) :
                   memb_IndexOfParent(memb_GlobalIndexOfBlock(buddy, found_block, lvl))));
                // Splitting block
                bitSet(buddy->free_bits,  memb_GlobalIndexOfBlock(buddy, found_block, lvl));
                bitSet(buddy->free_bits,  memb_GlobalIndexOfBlock(buddy, found_block_sibiling, lvl));
                bitReset(buddy->free_bits,  memb_IndexOfParent(memb_GlobalIndexOfBlock(buddy, found_block, lvl)));
                bitSet(buddy->split_bits, memb_IndexOfParent(memb_GlobalIndexOfBlock(buddy, found_block, lvl)));
            }

            p("\tAllocing at level: %u, block size: %u, local_index: %lu\n", level, memb_SizeOfBlocksAtLevel(buddy, level), memb_IndexOfBlockInLevel(buddy, found_block, level));

            bitReset(buddy->free_bits,  memb_GlobalIndexOfBlock(buddy, found_block, level));
        }
    }

    return found_block;
}

header_function
MEMORY_ALLOCATOR(memBuddy)
{
    unused_variable(data);

    switch (mode) {
        case Memory_Allocator_Mode_ALLOCATE: {
            if (size == 0) return NULL;

            Size size_real = memAlignUp(mathNextPowerOf2U64(size));

            void *mem = NULL;

            for (Memory_Buddy_Data *b = NLIB_GLOBAL_memory_hash_table.now; b != NULL; b = b->next) {
                mem = memBuddy_Alloc(b, size_real);
                if (mem != NULL) break;
            }

            if (mem == NULL) {
                // TODO(naman): Do some real analysis for the optimal overhead, please!!
                // Formula for overhead percentage should be: 100*(2*(2^(log2(x/y)+1)-1)-(x/y))/(8*x)
                // Simplified: (75 x - 50 y)/(2 x y)
                // Wolfram Alpha's link to the formula: https://www.wolframalpha.com/input/?i=100*%282*%282%5E%28log2%28x%2Fy%29%2B1%29-1%29-%28x%2Fy%29%29%2F%288*x%29%2C+y+%3D+64%2C+solve+for+x
                // BUT THIS DOESN'T WORK FOR SOME FUCKING REASON, FUCK ME!!!
                Size leaf_size = size_real;
                Size arena_size = leaf_size * (1ULL << 8);

                Memory_Buddy_Data not_buddy = memBuddy_Init(NULL, arena_size, leaf_size);

                Size size_of_free_bits = bitToBytes(not_buddy.block_count);
                Size size_of_split_bits = bitToBytes(not_buddy.block_count - not_buddy.leaf_count);
                Size total_bits_size = memAlignUp(size_of_free_bits + size_of_split_bits);

                Size buddy_size = memAlignUp(sizeof(not_buddy));
                Size total_size = arena_size + total_bits_size + buddy_size;

                Byte *memory = mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                                    MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

                Memory_Buddy_Data *buddy = (void*)memory;
                Byte *arena = memory + buddy_size;
                Byte *free_bits = arena + arena_size;
                Byte *split_bits = free_bits + size_of_free_bits;

                *buddy = memBuddy_Init(arena, arena_size, leaf_size);

                buddy->free_bits = free_bits;
                buddy->split_bits = split_bits;

                memset(buddy->free_bits, 0, total_bits_size);

                bitSet(buddy->free_bits, 0);

                if (NLIB_GLOBAL_memory_hash_table.now != NULL) {
                    NLIB_GLOBAL_memory_hash_table.now->prev = buddy;
                }

                buddy->next = NLIB_GLOBAL_memory_hash_table.now;
                NLIB_GLOBAL_memory_hash_table.now = buddy;

                mem = memBuddy_Alloc(buddy, size_real);
            }

            claim(mem != NULL);
            return mem;
        } break;

        case Memory_Allocator_Mode_DEALLOCATE: {
            Byte *ptr = old_ptr;

            if (ptr == NULL) return NULL;

            B32 deallocation_arena_found = false;
            for (Memory_Buddy_Data *b = NLIB_GLOBAL_memory_hash_table.now; b != NULL; b = b->next) {
                if ((ptr >= b->arena) && (ptr < (b->arena + b->arena_size))) {
                    U64 level_min = 0;

                    /* while (level_min < b->level_count) { */
                    /*     if (isOdd(memb_IndexOfBlockInLevel(b, (Byte*)ptr, level_min))) break; */
                    /*     level_min--; */
                    /* } */

                    U64 level = b->level_max;
                    while ((level >= level_min) && (level <= b->level_max)) {
                        if (level == 0) {
                            break;
                        } else if (bitTest(b->split_bits,
                                           memb_IndexOfParent(memb_GlobalIndexOfBlock(b, (Byte*)ptr, level)))) {
                            break;
                        }
                        level--;
                    }

                    p("\tFreeing at level: %u, block size: %u\n", level, memb_SizeOfBlocksAtLevel(b, level));
                    {
                        bitSet(b->free_bits, memb_GlobalIndexOfBlock(b, ptr, level));

                        p("\t\tDeallocating block:%u and level:%u\n", memb_IndexOfBlockInLevel(b, ptr, level), level);

                        Size merge_level = level;
                        Byte *merge_ptr = ptr;

                        while (merge_level > 0) {
                            Size local_index  = memb_IndexOfBlockInLevel(b, merge_ptr, merge_level);

                            Byte *parent_block = NULL; // the pointer to the merged blocks (if they get merged)
                            Byte *buddy_block = NULL; // pointer to buddy block

                            if (isOdd(local_index)) { // odd = merge with previous block
                                buddy_block = memb_PreviousBlockInLevel(b, merge_ptr, merge_level);
                                parent_block = buddy_block;
                            } else { // even = merge with next block
                                buddy_block = memb_NextBlockInLevel(b, merge_ptr, merge_level);
                                parent_block = merge_ptr;
                            }

                            Size block_global_index = memb_GlobalIndexOfBlock(b, merge_ptr, merge_level);
                            Size buddy_global_index = memb_GlobalIndexOfBlock(b, buddy_block, merge_level);
                            Size parent_global_index = memb_GlobalIndexOfBlock(b, parent_block, merge_level - 1);

                            if (bitTest(b->free_bits, buddy_global_index)) { // Buddy is also free
                                p("\t\t\tMerging block:%u and block:%u at level:%u to create block:%u at level:%u\n",
                                  memb_IndexOfBlockInLevel(b, merge_ptr, merge_level),
                                  memb_IndexOfBlockInLevel(b, buddy_block, merge_level),
                                  merge_level,
                                  memb_IndexOfBlockInLevel(b, parent_block, merge_level - 1),
                                  merge_level - 1);
                                bitReset(b->free_bits, buddy_global_index);
                                bitReset(b->free_bits, block_global_index);
                                bitSet(b->free_bits, parent_global_index);
                                bitReset(b->split_bits, parent_global_index);

                                merge_ptr = parent_block;
                            } else {
                                merge_ptr = NULL;
                            }

                            if (merge_ptr == NULL) break;
                            merge_level--;
                        }
                    }

                    deallocation_arena_found = true;
                    break;
                }
            }

            claim(deallocation_arena_found);
            return NULL;
        } break;

        case Memory_Allocator_Mode_REALLOCATE: {
            claim(false && "Reallocation not supported in Buddy Allocator");
            return NULL;
        } break;

        case Memory_Allocator_Mode_NONE: {
            claim(false && "Control shouldn't reach here");
            return NULL;
        } break;
    }

    claim(false && "Control shouldn't reach here");
    return NULL;
}

#define NLIB_MEMORY_H_INCLUDE_GUARD
#endif
