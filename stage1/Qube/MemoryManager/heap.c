#include "../Common/Qube.h"
#include "heap.h"

//--------imported functions--------
#include "memory_manager.h"
#include "../Common/spin_lock.h"
//----------------------------------

#define BLOCK_SIZE (0x10)
// Configurable values:
#define CLUSTER_SIZE (0x100*PAGE_SIZE) // must be: (a) power of 2. (b) multiple of PAGE_SIZE. (c) less or equal to 0x10000*BLOCK_SIZE.
#define CLUSTERS_AREA_SIZE (0x10*CLUSTER_SIZE)
#define MAX_ALLOC_IN_CLUSTER (PAGE_SIZE*2 - BLOCK_SIZE) // must be: (a) multiple of BLOCK_SIZE. (b) less than or equal to (CLUSTERS_AREA_SIZE-BLOCK_SIZE).

//----------------------------------
#define ALLOC_PAGES(x) alloc_pages_(KHEAP, PAGE_SIZE*x)
#define FREE_PAGES(x) free_pages_(x)
#define COMMIT_PAGES(x) commit_pages_(KHEAP, PAGE_SIZE*x)
#define ASSIGN_COMMITED(addr, num_of_pages) assign_committed_(addr, 0x1000*num_of_pages, -1)
#define UNASSIGN_COMMITED(addr, num_of_pages) unassign_committed_(addr, 0x1000*num_of_pages)

#define HEAP_LOCK SpinLock
#define INIT_LOCK(x) spin_init(&x)
#define LOCK(x) spin_lock(&x)
#define UNLOCK(x) spin_unlock(&x)

//----------------------------------



// Non-config values:
#define NUM_OF_BLOCKS(x) ((x+BLOCK_SIZE-1)/BLOCK_SIZE)
#define FREE_LIST_ARRAY_SIZE (MAX_ALLOC_IN_CLUSTER/BLOCK_SIZE + 1)
#define CLUSTERS_BITMAP_SIZE (((CLUSTERS_AREA_SIZE / CLUSTER_SIZE) + 7) / 8)


// Every chunk has at least 8 bytes header with the heap cookie.
// The whole header is maximum of BLOCK_SIZE
struct Chunk_;
typedef struct Chunk_{
    uint64 cookie;
    uint16 is_free;
    uint16 size_in_blocks;
    //int16 prev_is_free;
    uint16 prev_size_in_blocks;
// those fields exist on free chunk only.
    struct Chunk_ * next;
    struct Chunk_ * prev;
} __attribute__((packed)) Chunk;

typedef struct {
	Chunk * free_lists[FREE_LIST_ARRAY_SIZE]; // number of free_lists.
	void* clusters_area;
	uint32 cluster_start_mask;
	uint64 cookie;
	uint32 stat_num_of_big_allocs;
	uint32 stat_total_user_alloc;
	uint32 stat_total_mem_alloc;
	uint32 stat_num_of_allocs;
	uint32 stat_num_of_splits;
	uint32 stat_num_of_block_alloc[FREE_LIST_ARRAY_SIZE];
	uint32 stat_num_of_frees;
	uint32 stat_num_of_block_free[FREE_LIST_ARRAY_SIZE];
} HeapStruct;

HEAP_LOCK g_heap_lock;
HeapStruct g_heap_struct;
uint8 clusters_bitmap[CLUSTERS_BITMAP_SIZE];

QResult HEAP_INIT_FUNC() {
	INIT_LOCK(g_heap_lock);
	void* clusters_area;
	clusters_area = COMMIT_PAGES(NUM_OF_PAGES(CLUSTERS_AREA_SIZE));
	if (clusters_area) {
		g_heap_struct.cluster_start_mask = ((uint64)clusters_area)&(CLUSTER_SIZE - 1);
		// TODO: we dont realy should fail if cluster_area is not aligned to CLUSTER_SIZE.
		g_heap_struct.clusters_area = clusters_area;
		return QSuccess;
	}
	else {
		return QFail;
	}
}

void heap_corrupt(Chunk * free_chunk) {
    // TODO - implement
}

void * alloc_new_cluster() {
	// TODO: handle ASSIGN_COMMITED failure case. (out of physical pages)
	uint32 i, j;
	for (i = 0; i < CLUSTERS_BITMAP_SIZE; i++) {
		if (clusters_bitmap[i] != 0xFF) {
			break;
		}
	}
	if (clusters_bitmap[i] != 0xFF) {
		uint8 x = clusters_bitmap[i];
		for (j = 0; j < 8; j++) {
			if (!(x&(1 << j))) {
				break;
			}
		}
		clusters_bitmap[i] |= (1 << j);
		void *addr = (void*)((uint8*)g_heap_struct.clusters_area + (8 * i + j)*CLUSTER_SIZE);
		ASSIGN_COMMITED(addr, (CLUSTER_SIZE / PAGE_SIZE));
		return addr;
	}
	return NULL;
}

void free_cluster(void* addr) {
	UNASSIGN_COMMITED(addr, (CLUSTER_SIZE / PAGE_SIZE));
	uint32 cluster_index = ((uint64)addr - (uint64)g_heap_struct.clusters_area) / CLUSTER_SIZE;
	clusters_bitmap[cluster_index / 8] &= ~(1 << (cluster_index % 8));
}

static inline Chunk* get_next_chunk(Chunk* c) {
	Chunk* next = (Chunk*)((uint64)c + BLOCK_SIZE * (c->size_in_blocks + 1));
	if ((((uint64)next) & (CLUSTER_SIZE - 1)) != g_heap_struct.cluster_start_mask){
		return next;
	}
	return NULL;
}

static inline Chunk* get_prev_chunk(Chunk* c) {
	if (c->prev_size_in_blocks == 0) {
		return NULL;
	}
	Chunk* prev = (Chunk*)((uint64)c - BLOCK_SIZE * (c->prev_size_in_blocks + 1));
	return prev;
}

static inline void validate_chunk(Chunk * chunk, int size_in_blocks, BOOL is_free) {
	if (chunk->cookie != g_heap_struct.cookie) { heap_corrupt(chunk); }
	if (chunk->size_in_blocks != size_in_blocks) { heap_corrupt(chunk); }
	if (chunk->is_free != is_free) { heap_corrupt(chunk); }
}

static inline void add_to_list(Chunk * chunk, uint32 index) {
	if (index >= FREE_LIST_ARRAY_SIZE) {
		index = 0;
	}
    chunk->prev = NULL;
	chunk->next = g_heap_struct.free_lists[index];
    g_heap_struct.free_lists[index] = chunk;
}

static inline void remove_from_list(Chunk * chunk, uint32 index) {
	if (index >= FREE_LIST_ARRAY_SIZE) {
		index = 0;
	}
    if (chunk->prev) {
        if (chunk->prev->next != chunk) heap_corrupt(chunk);  
        chunk->prev->next = chunk->next;
    } 
	else { // it is the first element in the list
        if (g_heap_struct.free_lists[index] != chunk) heap_corrupt(chunk);
        g_heap_struct.free_lists[index] = chunk->next;
    }
    if (chunk->next) {
        if (chunk->next->prev != chunk) heap_corrupt(chunk);
        chunk->next->prev = chunk->prev;
    }
    // TODO: do we want to zero the chunk->next and chunk->prev to prevent information disclosure?
}

void * HEAP_ALLOC_FUNC(uint32 size) {
	if (size == 0) {
		return NULL;
	}
    if (size > MAX_ALLOC_IN_CLUSTER) {
        g_heap_struct.stat_num_of_big_allocs++;
        return ALLOC_PAGES(NUM_OF_PAGES(size));
    }

	LOCK(g_heap_lock);
    
	uint32 num_of_blocks = NUM_OF_BLOCKS(size);
	Chunk *chunk = NULL;
	for (uint32 i = num_of_blocks; i < FREE_LIST_ARRAY_SIZE; i++) {
		chunk = g_heap_struct.free_lists[i];
		if (chunk) {
			validate_chunk(chunk, i, TRUE);
			g_heap_struct.free_lists[i] = chunk->next;
			chunk->next->prev = NULL;
			break;
		}
	}
	
	if (chunk == NULL) {
		for (chunk = g_heap_struct.free_lists[0]; chunk; chunk = chunk->next) {
			if (chunk->size_in_blocks >= num_of_blocks) {
				validate_chunk(chunk, chunk->size_in_blocks, TRUE);
				remove_from_list(chunk,0);
				break;
			}
		}
	}
	if (chunk == NULL) {
		chunk = (Chunk*)alloc_new_cluster();
		chunk->cookie = g_heap_struct.cookie;
		chunk->is_free = TRUE;
		chunk->prev_size_in_blocks = 0;
		chunk->size_in_blocks = CLUSTER_SIZE / BLOCK_SIZE - 1;
		chunk->prev = NULL;
	}
	if (chunk == NULL) {
		return NULL;
	}
	chunk->is_free = FALSE;
	if ((chunk->size_in_blocks - num_of_blocks) > 1) {
		Chunk* next_chunk = get_next_chunk(chunk);
		Chunk* reminder = (Chunk*)(((uint8*)chunk + BLOCK_SIZE) + num_of_blocks * BLOCK_SIZE); // always has place to the header
		uint32 reminder_index = chunk->size_in_blocks - num_of_blocks - 1;
		reminder->cookie = g_heap_struct.cookie;
		reminder->size_in_blocks = reminder_index;
		reminder->is_free = TRUE;
		reminder->prev_size_in_blocks = num_of_blocks;

		chunk->size_in_blocks = num_of_blocks;
		if (reminder_index >= FREE_LIST_ARRAY_SIZE) {
			reminder_index = 0;
		}
		reminder->prev = NULL;
		reminder->next = g_heap_struct.free_lists[reminder_index];
		g_heap_struct.free_lists[reminder_index] = reminder;

		if (next_chunk) {
			next_chunk->prev_size_in_blocks = reminder_index;
		}
	}
	g_heap_struct.stat_total_mem_alloc += ((num_of_blocks + 1)*BLOCK_SIZE);
	UNLOCK(g_heap_lock);
	return (void*)((uint8*)chunk + BLOCK_SIZE);
}

void HEAP_FREE_FUNC(void * address) {
	if ((address < g_heap_struct.clusters_area) || (address >= (void*)((uint64)g_heap_struct.clusters_area + CLUSTERS_AREA_SIZE))){
		FREE_PAGES(address);
		return;
	}
	LOCK(g_heap_lock);
    Chunk * chunk = (Chunk*)((uint8*) address - BLOCK_SIZE);
    uint32 num_of_blocks = chunk->size_in_blocks;
    g_heap_struct.stat_num_of_block_free[num_of_blocks]++;
	// validate chunk
	if (chunk->cookie != g_heap_struct.cookie) { heap_corrupt(chunk); }
	if (chunk->is_free != FALSE) { heap_corrupt(chunk); }

	Chunk* next = get_next_chunk(chunk);
	Chunk* prev = get_prev_chunk(chunk);

    // consolidates:
    if (next) { // There is next chunk. We not the last one.
		// validate
		if (next->cookie != g_heap_struct.cookie) { heap_corrupt(chunk); }
        if (next->is_free) {
            // consolidate:
            remove_from_list(next, next->size_in_blocks);
			num_of_blocks += next->size_in_blocks + 1; // +1 for the header.
        } 
    }



    if (prev) { // There is something before it.
		// validate
		if (prev->cookie != g_heap_struct.cookie) { heap_corrupt(chunk); }
        if (prev->is_free) {
            // consolidate:
            remove_from_list(prev, prev->size_in_blocks);
			num_of_blocks += prev->size_in_blocks + 1; // +1 for the header.
            chunk = prev;
        }
    }
    
    // Write the header:
	chunk->size_in_blocks = num_of_blocks;
	chunk->is_free = TRUE;
	if (get_next_chunk(chunk)) {
		Chunk * next_chunk = get_next_chunk(chunk);
		next_chunk->prev_size_in_blocks = num_of_blocks;
	}
	if (chunk->size_in_blocks < (CLUSTER_SIZE / BLOCK_SIZE - 1)) {
		add_to_list(chunk, num_of_blocks);
	}
	else {
		free_cluster(chunk);
	}
    g_heap_struct.stat_total_mem_alloc -= ((num_of_blocks+1)*BLOCK_SIZE);
    g_heap_struct.stat_num_of_frees++;
	UNLOCK(g_heap_lock);
    return;
}