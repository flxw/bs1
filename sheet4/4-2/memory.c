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
    while(currentEntry->size < size) {
        if(currentEntry->next == NULL) {
            errno = ENOMEM;
            return NULL;
        }

        currentEntry = currentEntry->next;
    }

    errno=ENOMEM;
    return NULL;
}

void bs_free(void *ptr)
{
}
#endif
  
