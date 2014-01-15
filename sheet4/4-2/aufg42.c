/* 
 * Betriebssystem & Middleware
 * Hasso-Plattner-Institut 
 *
 * Betriebssysteme I WS 2013/2014
 *  
 * Uebung 4.2
 */
 
#include "memory.h"
#include <stdio.h>


int main(int argc, char **argv)
{
    memoryInit();
    char *ptr1, *ptr2, *ptr3, *ptr4;
	//check bs_free implementation for valid arguments 
    ptr1=(char*)bs_malloc(96); //96 Bytes
	if(NULL==ptr1)
	    perror("63 Bytes");
	ptr2=bs_malloc(964); //964 Bytes
	if(NULL==ptr2)
	    perror("964 Bytes");
    ptr3=(char*)bs_malloc(3000); //3000 Bytes
	if(NULL==ptr3)
	    perror("3000 Bytes");
    bs_free(ptr1); 
	ptr3=(char*)bs_malloc(3000); //3000 Bytes
	if(NULL==ptr3)
	    perror("3000 Bytes");
	bs_free(ptr2);
	bs_free(ptr3);
    ptr1=(char*)bs_malloc(4090); //96 Bytes
	if(NULL==ptr1)
	    perror("4090 Bytes");
	ptr2=(char*)bs_malloc(5); //96 Bytes
	if(NULL==ptr2)
	    perror("5 Bytes");
    bs_free(ptr2); 
	bs_free(ptr1);

    memoryCleanup();
	return 0;
}
