// Configurable values:
#define BLOCK_SIZE (0x20)
#define MAX_ALLOC_SIZE (PAGE_SIZE*2) // must be multiple of BLOCK_SIZE
#define REGION_SIZE (MAX_ALLOC_SIZE * 0x1000)

// Non-config values:
#define FREE_LIST_ARRAY_SIZE (MAX_ALLOC_SIZE/BLOCK_SIZE + 1)


// Every chunk has at least 8 bytes header with the heap cookie.
// The whole header is maximum of BLOCK_SIZE
struct FreeChunk {
    int64 heap_cookie;
    int16 is_free;
    int16 size_in_blocks;
    
    int16 prev_is_free;
    int16 prev_size_in_blocks;
    
    struct FreeChunk * next;
    struct FreeChunk * prev;
}

struct HeapStruct {
    void * region; // pointer to the first mapped region.
    struct FreeChunk * free_lists[FREE_LIST_ARRAY_SIZE] // number of free_lists.
    
}

static KernelGlobaldata * g_kgd;
void * system_allocate_memory(int size) {
    // call the system allocator
}

void system_free_memory(void * addr) {
    // call the system allocator
}



struct HeapStruct * get_heap_struct() {
    // If we in kernel mode, return the heap_struct from the KernelGlobalData.
    return g_kgd->heap_struct;
}


int heap_init(KernelGlobalData * kgd) {
    // For now we use the KernelGlobalData, but actually we want to use some USER-KERNEL same struct to
    // make heap to every process.
    ASSERT (sizeof(FreeChunk) == BLOCK_SIZE);
    g_kgd = kgd;
}

void heap_corrupt(FreeChunk * free_chunk) {
    // TODO - implement
}

__inline__ void validate_chunk(FreeChunk * chunk, HeapStruct * hs, int free_chunk_index, bool is_free) {
    if (free_chunk->cookie != hs->cookie) heap_corrupt(free_chunk);
    if (chunk->size_in_blocks != free_chunk_index && i < FREE_LIST_ARRAY_SIZE - 1) { // heap corrupt!
        heap_corrupt(chunk);
    }
    if (chunk->is_free != is_free) heap_corrupt(chunk);
}

__inline__ void write_chunk_header(FreeChunk * chunk, HeapStruct * hs, int size_in_blocks, bool is_free) {
    chunk->cookie = hs->cookie;
    chunk->size_in_blocks = size_in_blocks;
    chunk->is_free = is_free;
    FreeChunk * next_chunk = chunk + size_in_blocks + 1
    if (is_in_heap(next_chunk)) { // There is next chunk (We not the last chunk in the heap)
        next_chunk->prev_size_in_blocks = size_in_blocks;
        next_chunk->prev_is_free = is_free;
    }
}

__inline__ void add_to_list(FreeChunk * chunk, HeapStruct * hs, int index) {
    chunk->prev = NULL;
    chunk->next = hs->free_lists[index]
    hs->free_lists[index] = chunk;
}
__inline__ void remove_from_list(FreeChunk * chunk, HeapStruct * hs, int index) {
    if (chunk->prev) {
        if (chunk->prev->next != chunk) heap_corrupt(chunk);  
        chunk->prev->next = chunk->next;
    } else { // it is the first element in the list
        if (hs->free_lists[index] != chunk) heap_corrupt(chunk);
        hs->free_lists[index] = chunk->next;
    }
    if (chunk->next) {
        if (chunk->next->prev != chunk) heap_corrupt(chunk);
        chunk->next->prev = chunk->prev;
    }
    // TODO: do we want to zero the chunk->next and chunk->prev to prevent information disclosure?
}
void * malloc(int size) {
    // TODO: add lock.
    HeapStruct * hs = get_heap_struct();
    if (size > MAX_ALLOC_SIZE) {
        hs->stat_num_of_big_allocs++;
        return system_allocate_memory(size);
    }
    
    void * ret = NULL;
    ret = _malloc_int(size);
    if (ret == NULL) return ret;
    // update some stats:
    hs->stat_total_user_alloc += size;
    hs->stat_total_mem_alloc += size / BLOCK_SIZE * BLOCK_SIZE + sizeof(FreeChunk);
    hs->stat_num_of_allocs++;
    hs->stat_num_of_block_alloc[size / BLOCK_SIZE]++;
    return ret;
}
    
void * _malloc_int(size) {
    // TODO: Do we want to add cache?
    int free_list_index = size / BLOCK_SIZE;
    // First try to find in the free list:
    FreeChunk * chunk = hs->free_list[free_list_index];
    if (chunk != NULL) {
        validate_chunk(chunk, hs, free_list_index, TRUE);
        remove_from_list(chunk, hs, free_list_index);
        write_chunk_header(chunk, hs, free_list_index, FALSE);
        return (void *)&(chunk+1); // return the pointer to the user.
    }
    // If we not found in the free list, search in the next lists:
    for (int i = free_list_index + 1; i < FREE_LIST_ARRAY_SIZE; i++) {
        chunk = hs->free_list[i];
        if (chunk != NULL) { // we found one! split the buffer.
            hs->stat_num_of_splits++;
            validate_chunk(chunk, hs, i, TRUE);
            remove_from_list(chunk, hs, i);
            // Dont forget that we asserted that sizeof(FreeChunk) == BLOCK_SIZE
            FreeChunk * reminder = ((char*)(chunk + 1)) + free_list_index * BLOCK_SIZE; // always has place to the header
            int reminder_index = chunk->size_in_blocks - free_list_index - 1;
            write_chunk_header(reminder, hs, reminder_index, TRUE); 
            if (reminder_index >= FREE_LIST_ARRAY_SIZE - 1) { // We need to put the reminder in the last list.
                remindex_index = FREE_LIST_ARRAY_SIZE - 1;
            }
            write_chunk_header(chunk, hs, free_list_index, FALSE); // allocated.
            add_to_list(reminder, hs, reminder_index);
            return (void*)&(chunk+1);
        }
    }
    // If we not found any free space, get more memory:
    // TODO - implement it, if needed.
    return NULL;

}

void free(void * address) {
    if (address == NULL) return;
    FreeChunk * chunk = ((FreeChunk *) address) - 1;
    int num_of_blocks = chunk->size_in_blocks;
    validate_chunk(chunk, hs, num_of_blocks); // We cant really validate the size.
    // consolidates:
    if (!is_chunk_first(chunk)) { // There is something before it.
        if (chunk->prev_is_free) {
            FreeChunk * prev = chunk - chunk->prev_size_in_blocks - 1;
            if (prev->is_free != chunk->prev_is_free) heap_corrupt(chunk);
            if (prev->size_in_blocks != chunk->prev_size_in_blocks) heap_corrupt(chunk);
            validate_chunk(prev, hs, prev->size_in_blocks); // We cant really validate the size.
            // consolidate:
            remove_from_list(prev, hs, prev->size_in_blocks);
            num_of_blocks += prev->num_of_blocks + 1 // +1 for the header.
            chunk = prev;
        }
    }
    FreeChunk * next = chunk + num_of_blocks + 1;
    if (chunk_in_heap(next)) { // There is next chunk. We not the last one.
        if (next->is_free) {
            validate_chunk(next, hs, next->size_in_blocks); // We cant really validate the size.
            // consolidate:
            remove_from_list(next, hs, next->size_in_blocks);
            num_of_blocks += next->size_in_blocks + 1 // +1 for the header.
        } 
    }
    // Write the header:
    write_chunk_header(chunk, hs, num_of_blocks, TRUE);
    if (num_of_blocks >= FREE_LIST_ARRAY_SIZE-1) {
        num_of_blocks = FREE_LIST_ARRAY_SIZE-1;
    }
    add_to_list(chunk, hs, num_of_blocks);
    hs->stat_total_mem_alloc -= (num_of_blocks*BLOCK_SIZE + sizeof(FreeChunk));
    hs->stat_num_of_frees++;
    hs->stat_num_of_block_free[size / BLOCK_SIZE]++;
    return;
}