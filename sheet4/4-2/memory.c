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

    /* only needed for buddy algorithm */
    struct Node* childLeft;
    struct Node* childRight;
    struct Node* parent;
} Node;


#ifdef BUDDY

#define BUDDY_MIN_SIZE 16

Node** nodeListArray;
unsigned int orderCount;

void bs_dump(void) {

    printf("\n");

    int i=0;
    Node* nodePtr = NULL;

    for( i=(orderCount-1); i>=0; i--) {
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
}

Node* splitBlockInHalf(Node* nodePtr) {
    
    if(!nodePtr || !nodePtr->free) { return NULL; }

    printf("splitting Block @ %d with size %d...\n", nodePtr->startPos, nodePtr->size);

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

    printf("Create blocks @ %d and @ %d with size %d\n", newPtr->startPos, buddyPtr->startPos, newPtr->size);

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

    Node* nodePtr = nodeListArray[order];
    while(nodePtr && !nodePtr->free) {
        nodePtr = nodePtr->next;
    }

    if(nodePtr && nodePtr->free) {
        // yay!
        printf("found Block of order %d starting @ %d with size %d\n", order, nodePtr->startPos, nodePtr->size);
        return nodePtr;   
    }

    printf("no block of order %d left\n", order);
    // we need to find a higher order block and split it
    nodePtr = findFreeBlockOfOrder(order + 1);
    
    if(nodePtr == NULL) {
        printf("could not find block of order %d :(\n", order);
        return NULL;
    }

    // we found a block of order + 1
    nodePtr = splitBlockInHalf(nodePtr);

    return nodePtr;
}

void *bs_malloc(size_t size) {

    bs_dump();

    printf("request for %d bytes\n", size);

    int order = 0;
    int logReference = BUDDY_MIN_SIZE;
    while(logReference < size) {
        logReference *= 2;
        order++;
    }
    if(order >= orderCount) {
        printf("%d >= orderCount (%d)\n", order, orderCount);
        errno = ENOMEM;
        return NULL;
    }

    printf("required: order %d or higher\n", order);

    Node* nodePtr = findFreeBlockOfOrder(order);

    if(!nodePtr) {
        errno=ENOMEM;
        return NULL;
    }

    nodePtr->free = 0;

    return (void*)(mem + nodePtr->startPos);

}

void collapseNode(Node* nodePtr) {

    if(!nodePtr) { return; }
    if(!nodePtr->childLeft || !nodePtr->childRight) { return; }

    if(nodePtr->childLeft && nodePtr->childLeft->free && nodePtr->childRight->free) {
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

        printf("collapsing blocks %d - %d and %d - %d\n", nodePtr->childLeft->startPos, nodePtr->childLeft->startPos + nodePtr->childLeft->size, nodePtr->childRight->startPos, nodePtr->childRight->startPos + nodePtr->childRight->size);

        // free children
        free(nodePtr->childLeft);
        free(nodePtr->childRight);

        nodePtr->childLeft = NULL;
        nodePtr->childRight = NULL;

        nodePtr->free = 1;

        // collapse parent if possible
        collapseNode(nodePtr->parent);
    }
}

void bs_free(void *ptr) {
    bs_dump();

    printf("request to free pointer %p\n", ptr);

    long offset = (long)ptr - (long)mem;
    if(offset < 0 || offset > MEM_SIZE) {
        printf("that pointer (%p) is not inside my data block.\n", ptr);
        return;
    }

    printf("... that is @ %d\n", offset);

    // binary search
    Node* nodePtr = nodeListArray[orderCount - 1];

    while(nodePtr) {
        printf("  [ %d - %d ]\n", nodePtr->startPos, nodePtr->startPos + nodePtr->size);
        if(nodePtr->startPos == offset && !nodePtr->childLeft) {
            printf("this pointer belongs to block @ %d with size %d -- freeing that one.\n", nodePtr->startPos, nodePtr->size);
            nodePtr->free = 1;
            if(nodePtr->parent) {
                collapseNode(nodePtr->parent);
            }
            return;
        }

        if(!nodePtr->childLeft && nodePtr->startPos != offset) {
            printf("(block %d - %d) I can't find the Node that this pointer belongs to :(\n", nodePtr->startPos, nodePtr->startPos + nodePtr->size);
            return;
        }

        if(nodePtr->childRight->startPos > offset) {
            printf("->childLeft\n");
            nodePtr = nodePtr->childLeft;
        }else{
            printf("->childRight\n");
            nodePtr = nodePtr->childRight;
        }

    }

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
  
