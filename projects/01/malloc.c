/**
 * CPE 453 Spring 2012
 * Assignment 01
 * https://users.csc.calpoly.edu/~pnico/class/s12/cpe453/asgn/asgn1/asgn1.pdf
 * 
 * Implementation of a memory management system.  Specifically, a
 * reimplementation of calloc, malloc, free, and realloc.
 *
 * @author James Pearson
 */

#include "malloc.h"

MemoryHeader * gMemoryListHead = NULL;
MemoryHeader * gMemoryListTail = NULL;

/** 
 * @param size bytes of memory to allocate
 * @return pointer to allocated memory on success, NULL on failure
 */
void * malloc(size_t size) {
    MemoryHeader * freeMemory;
    void * newChunk;
    MemoryHeader * newHeader;
    
    if (getenv("SUPER_DEBUG_MALLOC")) {
        fprintf(stderr, "malloc(%ld)\n", (long)size);
    }
    
    /* Make sure we have some memory to start working with. */
    if (gMemoryListHead == NULL) {
        if (_initialize_gMemoryList() == EXIT_FAILURE) {
            errno = ENOMEM;
            return NULL;
        }
    }
    
    /* Traverse list looking for free memory.  If we don't find enough, try to
     * get more from the system. */
    while ((freeMemory = _find_free_memory(gMemoryListHead, size)) == NULL) {
        newChunk =  sbrk(SBRK_SIZE);
        /* No more memory available? Bail. */
        if ((size_t)newChunk == -1) {
            errno = ENOMEM;
            return NULL;
        }
        
        /* Add the new memory onto the end of our list */
        assert(gMemoryListTail->memoryAllocated == FALSE);
        gMemoryListTail->memorySize += SBRK_SIZE;

        if (getenv("SUPER_DEBUG_MALLOC")) {
            fprintf(stderr, "new memory size: %ld\n",
                    (long)gMemoryListTail->memorySize);
        }
    }
    
    /* Update list to set memory as allocated. */
    freeMemory->memoryAllocated = TRUE;
    
    /* Create a new header for the rest of the free memory. */
    newHeader = (void *)((size_t)(freeMemory->memory) + size);
    newHeader->memoryAllocated = FALSE;
    newHeader->memorySize = freeMemory->memorySize
                          - size
                          - sizeof(MemoryHeader);
    newHeader->memory = (void *)((size_t)newHeader + sizeof(MemoryHeader));
    if (freeMemory == gMemoryListTail) {
        newHeader->nextHeader = NULL;
        gMemoryListTail->nextHeader = newHeader;
        gMemoryListTail = newHeader;
    } else {
        newHeader->nextHeader = freeMemory->nextHeader;
        freeMemory->nextHeader = newHeader;
    }
    
    if (getenv("DEBUG_MALLOC")) {
        fprintf(stderr, "MALLOC: malloc(%ld)       =>  (ptr=0x%8ld, size=%ld)",
                (long)size, (long)(freeMemory->memory),
                (long)(freeMemory->memorySize));
    }
    
    /* Return pointer to memory. */
    return freeMemory->memory;
}

/**
 * Contiguously allocate zero-filled memory for some number of objects.
 * @param count number of objects
 * @param size size of objects
 * @return pointer to allocated memory on success, NULL on failure
 */
void * calloc(size_t count, size_t size) {
    void * allocatedMemory = malloc(count * size);
    memset(allocatedMemory, 0, count * size);
    
    if (getenv("DEBUG_MALLOC")) {
        fprintf(stderr, "MALLOC: calloc(%ld, %ld)   =>  (ptr=0x%8ld)",
                (long)count, (long)size, (long)(allocatedMemory));
    }
    
    return allocatedMemory;
}

/**
 * Try to resize a chunk of allocated memory.
 * @param ptr a pointer to the memory to be resized
 * @param size desired new size of allocated memory
 * @return pointer to allocated memory on success, NULL on failure
 */
void * realloc(void * ptr, size_t size);

/**
 * Deallocate memory.
 * @param ptr pointer to the memory to be freed
 */
void free(void * ptr);

/*************************/
/*** Utility functions ***/
/*************************/

/**
 * Do some initial setup of our global memory list.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int _initialize_gMemoryList() {
    void * newChunk;
    
    if (getenv("SUPER_DEBUG_MALLOC")) {
        fprintf(stderr, "_initialize_gMemoryList()\n");
    }
    
    newChunk = sbrk(SBRK_SIZE);

    assert(gMemoryListHead == NULL);
    assert(gMemoryListTail == NULL);
    
    /* No more memory available? Bail. */
    if ((size_t)newChunk == -1) {
        errno = ENOMEM;
        return EXIT_FAILURE;
    }
    
    gMemoryListHead = newChunk;
    gMemoryListHead->memoryAllocated = FALSE;
    gMemoryListHead->memorySize = SBRK_SIZE - sizeof(MemoryHeader);
    gMemoryListHead->memory = (void *)((size_t)newChunk + sizeof(MemoryHeader));
    gMemoryListHead->nextHeader = NULL;
    
    gMemoryListTail = gMemoryListHead;
    
    if (getenv("SUPER_DEBUG_MALLOC")) {
        fprintf(stderr, "starting off with memory size: %ld\n",
                (long)gMemoryListTail->memorySize);
    }
    
    return EXIT_SUCCESS;
}

/**
 * Search through a list of MemoryHeader-s to find one with free memory.
 * TODO: DRY out this and _find_address_owner.
 * @param memoryList pointer to the MemoryHeader at the start of the list
 * @param desiredSize amount of memory we need free
 * @return pointer to MemoryHeader with free memory on success, NULL on failure
 */
MemoryHeader * _find_free_memory(MemoryHeader * memoryList,
                                 size_t desiredSize) {
    if (getenv("SUPER_DEBUG_MALLOC")) {
        fprintf(stderr, "_find_free_memory()\n");
    }
    
    /* Have we reached the end of the list? */
    if (memoryList == NULL) {
        return NULL;
    }
    
    /* Got enough free space here? */
    if (memoryList->memoryAllocated == FALSE
     && memoryList->memorySize >= desiredSize) {
        return memoryList;
    }
    
    /* Recurse on down the list. */
    return _find_free_memory(memoryList->nextHeader, desiredSize);
}

/**
 * Search through a list of MemoryHeader-s to find the one that "owns" a
 * particular address in memory.
 * TODO: DRY out this and _find_free_memory.
 * @param memoryList pointer to the MemoryHeader at the start of the list
 * @param address a pointer to an address in memory we're trying to parent
 * @return pointer to MemoryHeader owning address on success, NULL on failure
 */
MemoryHeader * _find_address_owner(MemoryHeader * memoryList, void * address) {
    if (getenv("SUPER_DEBUG_MALLOC")) {
        fprintf(stderr, "_find_address_owner()\n");
    }
    
    /* Have we reached the end of the list? */
    if (memoryList == NULL) {
        return NULL;
    }
    
    /* Are we bounding address? */
    if (address >= memoryList->memory
     && (size_t)address <= (size_t)(memoryList->memory) + memoryList->memorySize) {
        return memoryList;
    }
    
    /* Recurse on down the list. */
    return _find_address_owner(memoryList->nextHeader, address);
}

