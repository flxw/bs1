#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* Do not change !! */
static char mem[4096];


#ifdef BUDDY
void *bs_malloc(size_t size) {
    errno=ENOMEM;
    return NULL;
}

void bs_free(void *ptr) {
}
#endif 

#ifdef FIRST

typedef struct Node {
    unsigned int startPos;
    unsigned int size;
    char free;
    struct Node* next;
} Node;

Node* head;

void memoryInit(void) {
    head = (Node*)malloc(sizeof(Node));
    head->startPos = 0;
    head->size = 4096;
    head->free = 1;
    head->next = NULL;
}

void memoryCleanup(void) {
    Node* ptr;
    while(head->next != NULL) {
        ptr = head->next;
        head->next = ptr->next;
        free(ptr);
    }
    free(head);
}

void *bs_malloc(size_t size) {
    Node* nodePtr = head;
    while((!nodePtr->free) || (nodePtr->size < size)) {
        nodePtr = nodePtr->next;
        if(nodePtr == NULL) {
            printf("Not enough space left :(\n");
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
    printf("trimmed block @ %d to size %d\n", nodePtr->startPos, nodePtr->size);

    if(sizeDifference > 0) {
        if(nodePtr->next && nodePtr->next->free) {
            // free block, adjust size
            nodePtr->next->startPos = nodePtr->next->startPos - sizeDifference;
            nodePtr->next->size = nodePtr->next->size + sizeDifference;
            printf("next block @ %d grown to size %d\n", nodePtr->next->startPos, nodePtr->next->size);
        }else{
            // used block, create new free block in between
            Node* newFreeNode = (Node*)malloc(sizeof(Node));
            newFreeNode->startPos = nodePtr->startPos + size;
            newFreeNode->size = sizeDifference;
            newFreeNode->free =1;
            newFreeNode->next = nodePtr->next;
            nodePtr->next = newFreeNode;
            printf("new free block @ %d with size %d\n", newFreeNode->startPos, newFreeNode->size);
        }
    }

    return returnValue;
}

void bs_free(void *ptr) {
}
#endif
  
