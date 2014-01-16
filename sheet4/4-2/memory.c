#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define MEM_SIZE 4096

/* Do not change !! */
/* ... I did =O */
static char mem[MEM_SIZE];

/* works for both algorithms */
typedef struct Node {
    unsigned int startPos;
    unsigned int size;
    char free;
    struct Node* next;
    struct Node* prev;
} Node;


#ifdef BUDDY

#define BUDDY_MIN_SIZE 16

Node** nodeListArray;
unsigned int maxOrder;

void memoryInit(void) {
    // initialize array of lists
    maxOrder = 0;
    unsigned int logReference = BUDDY_MIN_SIZE;
    while(logReference < MEM_SIZE) {
        logReference *= 2;
        maxOrder++;
    }
    if(logReference > MEM_SIZE) {
        printf("MEM_SIZE and BUDDY_MIN_SIZE have to be powers of 2!\n");
        exit(1);
    }

    nodeListArray = (Node**)malloc(maxOrder * sizeof(Node*));

    printf("maxOrder: %d\n", maxOrder);

    // initialize nodeListArray
    unsigned int i=0;
    for(i=0; i<maxOrder; i++) {
        nodeListArray[i] = NULL;
    }

    // initialize root node
    Node* rootNode = (Node*)malloc(sizeof(Node));
    nodeListArray[maxOrder - 1] = rootNode;

    rootNode->startPos = 0;
    rootNode->size = MEM_SIZE;
    rootNode->free = 1;
    rootNode->next = NULL;
    rootNode->prev = NULL;
}

void memoryCleanup(void) {
    
}


void *bs_malloc(size_t size) {
    printf("request for %d bytes\n", size);

    int order = 0;
    int logReference = BUDDY_MIN_SIZE;
    while(logReference < size) {
        logReference *= 2;
        order++;
    }
    if(order > maxOrder) {
        printf("%d > maxOrder (%d)\n", order, maxOrder);
        errno = ENOMEM;
        return NULL;
    }

    printf("required: order %d or higher\n", order);

    Node* nodePtr = nodeListArray[order];
    while(nodePtr && nodePtr->next && !nodePtr->free) {
        
    }

    errno=ENOMEM;
    return NULL;
}

void bs_free(void *ptr) {
}
#endif 


#ifdef FIRST
Node* head;

void memoryInit(void) {
    head = (Node*)malloc(sizeof(Node));
    head->startPos = 0;
    head->size = MEM_SIZE;
    head->free = 1;
    head->next = NULL;
    head->prev = NULL;
}

void memoryCleanup(void) {
    printf("cleanup...\n");
    Node* ptr;
    while(head->next != NULL) {
        ptr = head->next;
        head->next = ptr->next;
        free(ptr);
    }
    free(head);
}

void bs_dump(Node* nodePtr) {
    printf("\n");
    while(nodePtr) {
        printf("%d -> %d (%d), %s free, [%p < %p > %p]\n", nodePtr->startPos, nodePtr->startPos + nodePtr->size, nodePtr->size, nodePtr->free?"":"not", nodePtr->prev, nodePtr, nodePtr->next);
        nodePtr = nodePtr->next;
    }
    printf("\n");
}

void *bs_malloc(size_t size) {
    bs_dump(head);

    printf("request for %d bytes\n", size);

    Node* nodePtr = head;
    while((!nodePtr->free) || (nodePtr->size < size)) {
        nodePtr = nodePtr->next;
        if(nodePtr == NULL) {
            errno = ENOMEM;
            return NULL;
        }
    }

    void* returnValue = (void*)(mem + nodePtr->startPos);

    // this block is used
    nodePtr->free = 0;

    // maybe we need to shuffle the next block around
    int sizeDifference = nodePtr->size - size;
    nodePtr->size = size;

    if(sizeDifference > 0) {
        if(nodePtr->next && nodePtr->next->free) {
            // free block, adjust size
            nodePtr->next->startPos = nodePtr->next->startPos - sizeDifference;
            nodePtr->next->size = nodePtr->next->size + sizeDifference;
        }else{
            // used block, create new free block in between
            Node* newFreeNode = (Node*)malloc(sizeof(Node));
            newFreeNode->startPos = nodePtr->startPos + size;
            newFreeNode->size = sizeDifference;
            newFreeNode->free =1;
            newFreeNode->next = nodePtr->next;
            newFreeNode->prev = nodePtr;
            
            if(nodePtr->next) {
                nodePtr->next->prev = newFreeNode;
            }
            nodePtr->next = newFreeNode;
        }
    }

    return returnValue;
}

void bs_free(void *ptr) {
    bs_dump(head);

    printf("request to free pointer %p\n", ptr);

    long offset = (long)ptr - (long)mem;
    if(offset < 0 || offset > MEM_SIZE) {
        printf("that pointer (%p) is not inside my data block.\n", ptr);
        return;
    }

    // find block in chain
    Node* nodePtr = head;
    while(nodePtr->startPos != offset) {
        nodePtr = nodePtr->next;
        if(nodePtr == NULL) {
            printf("pointer (%p) is not pointing at a block begin\n", ptr);
            return;
        }
    }

    nodePtr->free = 1;

    // try to concatenate free block afterwards
    if(nodePtr->next && nodePtr->next->free) {
        Node* toBeFreed = nodePtr->next;

        if(nodePtr->next->next) {
            nodePtr->next->next->prev = nodePtr;
        }

        nodePtr->next = nodePtr->next->next;
        nodePtr->size = nodePtr->size + nodePtr->next->size;

        free(toBeFreed);
    }

    // try to concatenate free block before
    if(nodePtr->prev && nodePtr->prev->free) {
        Node* toBeFreed = nodePtr;

        if(nodePtr->next) {
            nodePtr->next->prev = nodePtr->prev;
        }

        nodePtr->prev->next = nodePtr->next;
        nodePtr->prev->size = nodePtr->size + nodePtr->prev->size;

        free(toBeFreed);
    }
}
#endif
  
