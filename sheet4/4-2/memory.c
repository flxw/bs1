#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#define MEM_SIZE 4096

/* Do not change !! */
/* ... I did =O */
static char mem[MEM_SIZE];

pthread_mutex_t lock;

/* wrapper functions to make it reentrant */
void* bs_malloc(size_t size) {
    pthread_mutex_lock(&lock);    

    void* returnValue = bs_unsafe_malloc(size);

    pthread_mutex_unlock(&lock);

    return returnValue;
}

void bs_free(void* pointer) {
    pthread_mutex_lock(&lock);

    bs_unsafe_free(pointer);

    pthread_mutex_unlock(&lock);
}


#ifdef BUDDY

typedef struct Node {
    unsigned int startPos;
    unsigned int size;
    char free;
    struct Node* next;
    struct Node* prev;
    struct Node* childLeft;
    struct Node* childRight;
    struct Node* parent;
} Node;

// this Node structure is quite heavy and itself > 16 Bytes
// so we should not give out memory chunks smaller than 16 bytes

#define BUDDY_MIN_SIZE 32

Node** nodeListArray;
unsigned int orderCount;

void bs_dump(void) {

    printf("\n");

    int i=0;
    Node* nodePtr = NULL;

    for( i=0; i<orderCount; i++) {
        printf("order %d: ", i);
        nodePtr = nodeListArray[i]; 
        while(nodePtr) {
            printf("[ %d - %d (%s) ] ", nodePtr->startPos, nodePtr->startPos + nodePtr->size, nodePtr->free?"free":"not free");
            nodePtr = nodePtr->next;
        }
        printf("\n");
    }
    printf("\n");
}

void memoryInit(void) {

    // initialize mutex
    pthread_mutex_init(&lock, NULL);

    // initialize array of lists
    orderCount = 1;
    unsigned int logReference = BUDDY_MIN_SIZE;
    while(logReference < MEM_SIZE) {
        logReference *= 2;
        orderCount++;
    }
    if(logReference > MEM_SIZE) {
        printf("MEM_SIZE and BUDDY_MIN_SIZE have to be powers of 2!\n");
        exit(1);
    }

    nodeListArray = (Node**)malloc(orderCount * sizeof(Node*));

    // initialize nodeListArray
    unsigned int i=0;
    for(i=0; i<orderCount; i++) {
        nodeListArray[i] = NULL;
    }

    // initialize root node
    Node* rootNode = (Node*)malloc(sizeof(Node));
    nodeListArray[orderCount - 1] = rootNode;

    rootNode->startPos = 0;
    rootNode->size = MEM_SIZE;
    rootNode->free = 1;
    rootNode->next = NULL;
    rootNode->prev = NULL;
    rootNode->childLeft = NULL;
    rootNode->childRight = NULL;
    rootNode->parent = NULL;
}

void memoryCleanup(void) {
    int i=0;
    Node* nodePtr = NULL;

    for( i=(orderCount-1); i>=0; i--) {
        nodePtr = nodeListArray[i];
        while(nodePtr) {
            Node* toBeDeleted = nodePtr;
            nodePtr = nodePtr->next;
            free(toBeDeleted);
        }
    }
    free(nodeListArray);

    pthread_mutex_destroy(&lock);
}

Node* splitBlockInHalf(Node* nodePtr) {
    
    if(!nodePtr || !nodePtr->free) { return NULL; }

    // determine order
    int order = 0;
    int logReference = BUDDY_MIN_SIZE;
    while(logReference < nodePtr->size) {
        logReference *= 2;
        order++;
    }

    // decrease order
    int newOrder = order - 1;

    // create new Block
    Node* newPtr   = (Node*)malloc(sizeof(Node));
    Node* buddyPtr = (Node*)malloc(sizeof(Node));

    newPtr->startPos = nodePtr->startPos;
    newPtr->size = nodePtr->size / 2;
    newPtr->free = 1;
    newPtr->next = buddyPtr;
    newPtr->prev = NULL;
    newPtr->childLeft = NULL;
    newPtr->childRight = NULL;
    newPtr->parent = nodePtr;

    // create buddy block
    buddyPtr->startPos = nodePtr->startPos + newPtr->size;
    buddyPtr->size = nodePtr->size / 2;
    buddyPtr->free = 1;
    buddyPtr->next = NULL;
    buddyPtr->prev = newPtr;
    buddyPtr->childLeft = NULL;
    buddyPtr->childRight = NULL;
    buddyPtr->parent = nodePtr;

    nodePtr->childLeft = newPtr;
    nodePtr->childRight = buddyPtr;

    // mark original block as occupied
    nodePtr->free = 0;

    // append nodeListArray
    if(!nodeListArray[newOrder]) {
        nodeListArray[newOrder] = newPtr;
        newPtr->prev = NULL;
    }else{
        Node* lastNode = nodeListArray[newOrder];
        while(lastNode && lastNode->next){
            lastNode = lastNode->next;
        }
        lastNode->next = newPtr;
        newPtr->prev = lastNode;
    }

    return newPtr;
}

Node* findFreeBlockOfOrder(int order) {

    if(order >= orderCount) {
        return NULL;
    }

    // find a free block
    Node* nodePtr = nodeListArray[order];
    while(nodePtr && !nodePtr->free) {
        nodePtr = nodePtr->next;
    }

    // did we find one in our list?
    if(nodePtr && nodePtr->free) {
        return nodePtr;   
    }

    // otherwise we need to find a higher order block
    nodePtr = findFreeBlockOfOrder(order + 1);
    
    if(nodePtr == NULL) {
        return NULL;
    }

    // we found a block of order + 1 and need to split it
    nodePtr = splitBlockInHalf(nodePtr);

    return nodePtr;
}

void *bs_unsafe_malloc(size_t size) {

    bs_dump();


    printf("request for %d bytes\n", (unsigned int)size);

    // determine minimum order
    int order = 0;
    int logReference = BUDDY_MIN_SIZE;
    while(logReference < size) {
        logReference *= 2;
        order++;
    }
    if(order >= orderCount) {
        errno = ENOMEM;
        return NULL;
    }


    Node* nodePtr = findFreeBlockOfOrder(order);

    if(!nodePtr) {
        errno=ENOMEM;
        return NULL;
    }

    nodePtr->free = 0;

    return (void*)(mem + nodePtr->startPos);

}

void collapseNode(Node* nodePtr) {

    // only collapse this node if we have to children and they are free
    if(!nodePtr) { return; }
    if(!nodePtr->childLeft || !nodePtr->childRight) { return; }
    if(!nodePtr->childLeft->free || !nodePtr->childRight->free) { return; }

    // determine order to know which list to edit
    int order = 0;
    int logReference = BUDDY_MIN_SIZE;
    while(logReference < nodePtr->childLeft->size) {
        logReference *= 2;
        order++;
    }            
    
    // remove children from list
    if(nodePtr->childRight->next) {
        nodePtr->childRight->next->prev = nodePtr->childLeft->prev;
    }
    if(nodePtr->childLeft->prev) {
        nodePtr->childLeft->prev->next = nodePtr->childRight->next;
    }
    if(nodeListArray[order] == nodePtr->childLeft) {
        nodeListArray[order] = nodePtr->childRight->next;
    }

    // free children
    free(nodePtr->childLeft);
    free(nodePtr->childRight);

    nodePtr->childLeft = NULL;
    nodePtr->childRight = NULL;

    nodePtr->free = 1;

    // collapse parent if possible
    collapseNode(nodePtr->parent);
}

void bs_unsafe_free(void *ptr) {
    bs_dump();


    printf("request to free pointer %p\n", ptr);

    long offset = (long)ptr - (long)mem;
    if(offset < 0 || offset > MEM_SIZE) {
        printf("that pointer (%p) is not inside my data block.\n", ptr);
        return;
    }

    // binary search
    Node* nodePtr = nodeListArray[orderCount - 1];

    while(nodePtr) {
        if(nodePtr->startPos == offset && !nodePtr->childLeft) {
            nodePtr->free = 1;
            if(nodePtr->parent) {
                collapseNode(nodePtr->parent);
            }
            return;
        }

        if(!nodePtr->childLeft && nodePtr->startPos != offset) {
            return;
        }

        if(nodePtr->childRight->startPos > offset) {
            nodePtr = nodePtr->childLeft;
        }else{
            nodePtr = nodePtr->childRight;
        }
    }
}
#endif 


#ifdef FIRST

// diet Node
typedef struct Node {
    unsigned int startPos;
    unsigned int size;
    char free;
    struct Node* next;
    struct Node* prev;
} Node;

Node* head;

void memoryInit(void) {

    pthread_mutex_init(&lock, NULL);

    head = (Node*)malloc(sizeof(Node));
    head->startPos = 0;
    head->size = MEM_SIZE;
    head->free = 1;
    head->next = NULL;
    head->prev = NULL;
}

void memoryCleanup(void) {
    Node* ptr;
    while(head->next != NULL) {
        ptr = head->next;
        head->next = ptr->next;
        free(ptr);
    }
    free(head);

    pthread_mutex_destroy(&lock);
}

void bs_dump(Node* nodePtr) {
    printf("\n");
    while(nodePtr) {
        printf("%d -> %d (%d), %s free, [%p < %p > %p]\n", nodePtr->startPos, nodePtr->startPos + nodePtr->size, nodePtr->size, nodePtr->free?"":"not", nodePtr->prev, nodePtr, nodePtr->next);
        nodePtr = nodePtr->next;
    }
    printf("\n");
}

void *bs_unsafe_malloc(size_t size) {
    bs_dump(head);

    printf("request for %d bytes\n", (unsigned int)size);

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

void bs_unsafe_free(void *ptr) {
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

        nodePtr->size = nodePtr->size + nodePtr->next->size;
        nodePtr->next = nodePtr->next->next;

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
    return;
}
#endif
  
