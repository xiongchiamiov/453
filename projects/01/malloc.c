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

MemoryHeader * gMemoryList = NULL;

/** 
 * @param size bytes of memory to allocate
 * @return pointer to allocated memory on success, NULL on failure
 */
void * malloc(size_t size) {
    MemoryHeader * freeMemory;
    void * newChunk;
    MemoryHeader * newHeader;
    
    /* Traverse list looking for free memory.  If we don't find enough, try to
     * get more from the system. */
    while ((freeMemory = findFreeMemory(gMemoryList, size)) == NULL) {
        newChunk =  sbrk(CHUNK_SIZE);
        /* No more memory available? Bail. */
        if ((int)newChunk == -1) {
            errno = ENOMEM;
            return NULL;
        }
        
        /* Add the new memory into our list */
        newHeader = newChunk;
        newHeader->memoryAllocated = FALSE;
        newHeader->memorySize = CHUNK_SIZE - sizeof(MemoryHeader);
        newHeader->memory = (void *)((int)newChunk - sizeof(MemoryHeader));
        newHeader->nextHeader = gMemoryList;
        gMemoryList = newHeader;
    }
    
    /* Update list to set memory as allocated. */
    freeMemory->memoryAllocated = TRUE;
    /* Return pointer to memory. */
    return freeMemory->memory;
}

/**
 * Search through a list of MemoryHeader-s to find one with free memory.
 * @param memoryList pointer to the MemoryHeader at the start of the list
 * @param desiredSize amount of memory we need free
 * @return pointer to MemoryHeader with free memory on success, NULL on failure
 */
MemoryHeader * findFreeMemory(MemoryHeader * memoryList, size_t desiredSize) {
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
    return findFreeMemory(memoryList->nextHeader, desiredSize);
}

/**
 * Contiguously allocate zero-filled memory for some number of objects.
 * @param count number of objects
 * @param size size of objects
 * @return pointer to allocated memory on success, NULL on failure
 */
void * calloc(size_t count, size_t size);

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

