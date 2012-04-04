#ifndef MALLOC
#define MALLOC

/* Turn off assert-checking. */
/*#define NDEBUG*/

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHUNK_SIZE 16
#define SBRK_SIZE 65536

typedef enum bool {
	FALSE = 0,
	TRUE = 1
} bool;

typedef struct MemoryHeader {
	void * memory;
	size_t memorySize;
	bool memoryAllocated;
	struct MemoryHeader * nextHeader;
} MemoryHeader;

void * calloc(size_t nmemb, size_t size);
void * malloc(size_t size);
void free(void * ptr);
void * realloc(void * ptr, size_t size);

MemoryHeader * findFreeMemory(MemoryHeader * memoryList, size_t desiredSize);
int _initialize_gMemoryList();

#endif

