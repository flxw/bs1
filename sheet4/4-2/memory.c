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
void *bs_malloc(size_t size)
{
   errno=ENOMEM;
   return NULL;
}

void bs_free(void *ptr)
{
}
#endif
  