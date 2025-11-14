////////////////////////////////////////////////////////////////////////////////
/// Copyright 2025 Daniel S. Buckstein
/// 
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
/// 
///     http://www.apache.org/licenses/LICENSE-2.0
/// 
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
////////////////////////////////////////////////////////////////////////////////

/*
* cat_memory.c
* Memory management implementation.
*/

#include "cat/utility/cat_memory.h"
#include "cat/cat_platform.inl"
#include "cat/utility/cat_test.h"

#include <assert.h>
#include <string.h>

cat_implementation_begin;

#pragma region Aeris Definitions

typedef struct MemoryBlock
{
    size_t sizeInBytes;
    struct MemoryBlock* nextBlock;
    size_t   isFree;
}
MemoryBlock;

static uint8_t* g_memoryPool = NULL;
static size_t      g_memoryPoolSize = 0;
static MemoryBlock* g_firstBlock = NULL;

static inline void* GetBlockPayload(MemoryBlock* block)
{ return (uint8_t*)block + sizeof(MemoryBlock);}

static inline MemoryBlock* GetBlockHeader(void* payloadPtr)
{return (MemoryBlock*)((uint8_t*)payloadPtr - sizeof(MemoryBlock));}
static void MergeAdjacentFreeBlocks(void);

#pragma endregion

#pragma region Aeris Functions
static void MergeAdjacentFreeBlocks(void)
{
    MemoryBlock* block = g_firstBlock;

    while (block && block->nextBlock)
    {
        MemoryBlock* next = block->nextBlock;
        uint8_t* blockEnd = (uint8_t*)block + sizeof(MemoryBlock) + block->sizeInBytes;

        // If both blocks are free and are physically adjacent, merge them.
        if (block->isFree && next->isFree && blockEnd == (uint8_t*)next)
        {
            block->sizeInBytes += sizeof(MemoryBlock) + next->sizeInBytes;
            block->nextBlock = next->nextBlock;
        }
        else
        {
            block = next;
        }
    }
}
#pragma endregion


#ifdef CAT_DEBUG
typedef struct cat_malloc_metadata_s
{
#ifdef _WIN32
    //****TO-DO-MEMORY: fill in this structure.
    uint32_t reserved;
#else // #ifdef _WIN32
    uint32_t reserved;
#endif // #else // #ifdef _WIN32
} cat_malloc_metadata_t;
#endif // #ifdef CAT_DEBUG


cat_impl void* cat_memset(void* const p_block, uint8_t const value, size_t const set_size)
{
    assert_or_bail(p_block) NULL;
    assert_or_bail(set_size) NULL;
    return memset(p_block, value, set_size);
}

cat_impl void* cat_memclr(void* const p_block, size_t const clr_size)
{
    return cat_memset(p_block, 0xFF, clr_size);
}

cat_impl void* cat_memcpy(void* const p_block_dst, void const* const p_block_src, size_t const cpy_size)
{
    assert_or_bail(p_block_dst) NULL;
    assert_or_bail(p_block_src) NULL;
    assert_or_bail(cpy_size) NULL;
    return memcpy(p_block_dst, p_block_src, cpy_size);
}

cat_impl bool cat_memcmp(void const* const p_block_lh, void const* const p_block_rh, size_t const cmp_size)
{
    assert_or_bail(p_block_lh) false;
    assert_or_bail(p_block_rh) false;
    assert_or_bail(cmp_size) false;
    return (memcmp(p_block_lh, p_block_rh, cmp_size) == 0);
}

cat_impl void* cat_malloc(size_t const block_size)
{
#ifdef CAT_DEBUG
    cat_malloc_metadata_t* p_meta = NULL;
#endif // #ifdef CAT_DEBUG
    void* p_block = NULL;
    assert_or_bail(block_size) NULL;
    p_block = malloc(block_size);
#ifdef CAT_DEBUG
    unused(p_meta);
#endif // #ifdef CAT_DEBUG
    return p_block;
}

cat_impl void* cat_calloc(size_t const element_count, size_t const element_size)
{
#ifdef CAT_DEBUG
    cat_malloc_metadata_t* p_meta = NULL;
#endif // #ifdef CAT_DEBUG
    void* p_block = NULL;
    assert_or_bail(element_count) NULL;
    assert_or_bail(element_size) NULL;
    p_block = calloc(element_count, element_size);
#ifdef CAT_DEBUG
    unused(p_meta);
#endif // #ifdef CAT_DEBUG
    return p_block;
}

cat_impl void* cat_realloc(void* const p_block, size_t const block_size)
{
#ifdef CAT_DEBUG
    cat_malloc_metadata_t* p_meta = NULL;
#endif // #ifdef CAT_DEBUG
    void* p_block_new = NULL;
    assert_or_bail(p_block) NULL;
    assert_or_bail(block_size) NULL;
    p_block_new = realloc(p_block, block_size);
#ifdef CAT_DEBUG
    unused(p_meta);
#endif // #ifdef CAT_DEBUG
    return p_block_new;
}

cat_impl void cat_free(void* const p_block)
{
#ifdef CAT_DEBUG
    cat_malloc_metadata_t* p_meta = NULL;
#endif // #ifdef CAT_DEBUG
    assert_or_bail(p_block);
#ifdef CAT_DEBUG
    unused(p_meta);
#endif // #ifdef CAT_DEBUG
    free(p_block);
}

cat_impl bool cat_memory_pool_create(size_t const pool_size)
{
    assert_or_bail(pool_size) false;
    
    //****TO-DO-MEMORY: allocate and initialize pool.

    if (g_memoryPool != NULL)
        return false;

    g_memoryPool = (uint8_t*)malloc(pool_size);
    assert_or_bail(g_memoryPool) false;

    g_memoryPoolSize = pool_size;

    g_firstBlock = (MemoryBlock*)g_memoryPool;
    g_firstBlock->sizeInBytes = pool_size - sizeof(MemoryBlock);
    g_firstBlock->isFree = true;
    g_firstBlock->nextBlock = NULL;

    return true;
}

cat_impl bool cat_memory_pool_destroy(void)
{
    //****TO-DO-MEMORY: safely deallocate pool allocated above.

    if (!g_memoryPool)
        return false;

    free(g_memoryPool);

    g_memoryPool = NULL;
    g_firstBlock = NULL;
    g_memoryPoolSize = 0;

    return true;
}

cat_impl void* cat_memory_alloc(size_t const block_size)
{
    //****TO-DO-MEMORY: reserve block in managed pool.
    assert_or_bail(block_size) NULL;

    if (!g_memoryPool)
        return NULL;

    MemoryBlock* block = g_firstBlock;

    // Search for the first block
    while (block)
    {
        if (block->isFree && block->sizeInBytes >= block_size)
        {
            size_t leftover = block->sizeInBytes - block_size;

            // Split block if space remains
            if (leftover > sizeof(MemoryBlock))
            {
                uint8_t* newBlockAddr =
                    (uint8_t*)block + sizeof(MemoryBlock) + block_size;

                MemoryBlock* newBlock = (MemoryBlock*)newBlockAddr;

                newBlock->sizeInBytes = leftover - sizeof(MemoryBlock);
                newBlock->isFree = true;
                newBlock->nextBlock = block->nextBlock;

                block->nextBlock = newBlock;
                block->sizeInBytes = block_size;
            }

            block->isFree = false;
            return GetBlockPayload(block);
        }

        block = block->nextBlock;
    }
    return NULL;
}


cat_impl bool cat_memory_dealloc(void* const p_block)
{
    assert_or_bail(p_block) false;

    //****TO-DO-MEMORY: safely release block reserved above.

	if (!g_memoryPool)
        return false;

    MemoryBlock* block = GetBlockHeader(p_block);
    block->isFree = true;

    //Merge Neighbours
    MergeAdjacentFreeBlocks();

    return true;
}


#include "cat/utility/cat_time.h"
#include "cat/utility/cat_console.h"


cat_noinl int cat_memory_test(void)
{
    bool result = false;
    void* block_lh = cat_malloc(1024);
    void* block_rh = cat_malloc(2048);

    if (block_lh && block_rh)
    {
        cat_console_clear();
        cat_memset(block_lh, 0xFF, 1024);
        cat_memclr(block_rh, 2048);
        result = cat_memcmp(block_lh, block_rh, 2048);
        printf("\nMemory: \n    Blocks are equal: %"PRIi32, (int32_t)result);
        cat_memcpy(block_lh, block_rh, 1024);
        result = cat_memcmp(block_lh, block_rh, 1024);
        printf("\nMemory: \n    Blocks are equal: %"PRIi32, (int32_t)result);
        cat_platform_sleep(cat_platform_time_rate());
    }

    cat_free(block_lh);
    block_lh = NULL;
    cat_free(block_rh);
    block_rh = NULL;
    CAT_ASSERT(result);
    return 0;
}


cat_implementation_end;