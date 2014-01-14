#include "memory.h"
#include <stdio.h>
#include <errno.h>

/* Do not change !! */
static char mem[4096];


#ifdef BUDDY
void *bs_malloc(size_t size)
{
    errno=ENOMEM;
    return NULL;
}

void bs_free(void *ptr)
{
}
#endif 

#ifdef FIRST

typedef struct freeEntry{
    unsigned int size;
    struct freeEntry* next;
} freeEntry;

freeEntry* first;

void memInit(void) {
    first = (freeEntry*)(&mem[0]);
    first->size = 4096 - sizeof(freeEntry);
    first->next = NULL;
}

void *bs_malloc(size_t size)
{
    freeEntry* currentEntry = first;
    freeEntry* previousFreeEntry = first;

    while(currentEntry->size < (size + sizeof(unsigned int))) {
        printf("freeEntry @ %p with size %d is too small - using %p\n", currentEntry, currentEntry->size, currentEntry->next);
        if(currentEntry->next == NULL) {
            errno = ENOMEM;
            return NULL;
        }

        previousFreeEntry = currentEntry;
        currentEntry = currentEntry->next;
        getchar();
    }

    printf("Choosing free block @ %p\n", currentEntry);

    // this position will be returned
    char* returnPos = ((char*)(currentEntry + 1));

    // create new freeEntry after data block
    freeEntry* newFreeEntry = (freeEntry*)(returnPos + size + 1);
    newFreeEntry->next = currentEntry->next;
    newFreeEntry->size = currentEntry->size - size;

    if(previousFreeEntry == first) {
        first = newFreeEntry;
    }else{
        previousFreeEntry->next = newFreeEntry;
    }

    printf("New freeEntry @ %p with size %d\n", newFreeEntry, newFreeEntry->size);

    // create block info in front of block
    unsigned int* dataSize = ((unsigned int*)returnPos) + 1;
    *dataSize = size;

    printf("dataSize @ %p = %d\n\n", dataSize, *dataSize);

    return (void*)returnPos;
}

void bs_free(void *ptr)
{
    printf("returning data @ %p\n", ptr);
    unsigned int* dataSize = ((unsigned int*)ptr) - 1);
    printf("data Size: %d\n", *dataSize);

    // create new freeEntry
    freeEntry* newFreeEntry = (freeEntry*)dataSize;
    newFreeEntry->next = first->next;
    first->next = newFreeEntry;
    newFreeEntry->size = *dataSize - sizeof(freeEntry);

}
#endif
  
