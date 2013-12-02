/* Betriebssystem & Middleware
 *
 * Betriebssysteme I WS 2011/2012
 *
 * Uebung 3.3
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#define XSIZE 500
#define YSIZE 500
#include "algorithm.h"

struct ThreadData {
    int startPos;
    int pixelCount;
};

// globals ftw oder so
char colorArray[YSIZE][XSIZE][3];


void *threadRoutine (void *dataPointer) {
    struct ThreadData *td = (struct ThreadData *) dataPointer;
    int i, x, y;
    char* c;

    printf("new thread starting at offset %d, calculating %d values.\n", td->startPos, td->pixelCount);

    for (i = 0; i < td->pixelCount; ++i) {
        x = (td->startPos + i) % XSIZE;
        y = (td->startPos + i) / XSIZE;
        c = colorArray[y][x];

        getColorValuesAt(x * (2.0 / XSIZE) - 1.5, y * (2.0 / YSIZE) - 1.0,&c[2],&c[1],&c[0]);
    }

    // we are the only one with a pointer to this - free it
    free(td);
    td = NULL;
}

int main(int argc, char *argv[]) {
    FILE *fd;
    int len,x,y,i;
    char *dsc;
    char bgr[3];
    short svalue;
    int   lvalue;
    unsigned char header[54],*ptr=&header[0];
    int threadCount;
    int calculationsPerThread;
    pthread_t * threads;


    if(argc != 3) {
        perror("usage: bmp_fractal threadCount outputFilename.bmp");
        exit(1);
    }

    if((threadCount = atoi(argv[1])) < 1) {
        perror("That is not a valid thread count.");
        exit(1);
    }

    calculationsPerThread = ceil( (float)XSIZE * (float)YSIZE / (float)threadCount);

    getDescription(NULL,&len);
    if(NULL==(dsc=(char*)malloc(sizeof(char)*len))) {
        perror("malloc");
        exit(1);
    }
    getDescription(dsc,&len);

    // Goennen Sie sich die Funktion getId der algorithm.h, welche sehr klar zugeordnet werden kann
    printf("Calculate %s %d\n",dsc,getId());
    // open the file
    fd=fopen(argv[2],"wb+");
    if(NULL==fd) {
        perror("open"); exit(1);
    }

    svalue=0x4d42;
    memcpy(ptr,&svalue,2);//signatur
    ptr+=2;
    lvalue=XSIZE*YSIZE*3+54;
    memcpy(ptr,&lvalue,4); //filesize
    ptr+=4;
    lvalue=0;
    memcpy(ptr,&lvalue,4);//reserved
    ptr+=4;
    lvalue=54;
    memcpy(ptr,&lvalue,4);//image offset
    ptr+=4;
    lvalue=40;
    memcpy(ptr,&lvalue,4);//size of header follows
    ptr+=4;
    lvalue=XSIZE;
    memcpy(ptr,&lvalue,4);//with of image
    ptr+=4;
    lvalue=YSIZE;
    memcpy(ptr,&lvalue,4); //height of image
    ptr+=4;
    svalue=1;
    memcpy(ptr,&svalue,2); //number of planes
    ptr+=2;
    svalue=24;
    memcpy(ptr,&svalue,2); //number of pixel
    ptr+=2;
    lvalue=0; //compression
    memcpy(ptr,&lvalue,4); //compression
    ptr+=4;
    lvalue=XSIZE*YSIZE*3;
    memcpy(ptr,&lvalue,4); //size of image
    ptr+=4;
    lvalue=0;
    memcpy(ptr,&lvalue,4); //xres
    ptr+=4;
    lvalue=0;
    memcpy(ptr,&lvalue,4); //yres
    ptr+=4;
    lvalue=0;
    memcpy(ptr,&lvalue,4); //number of colortables
    ptr+=4;
    lvalue=0;
    memcpy(ptr,&lvalue,4); //number of important colors
    ptr+=4;

    len=fwrite(header,1,sizeof(header),fd); //write header

    if(-1==len || len!=sizeof(header)) {
        perror("write");
        exit(2);
    }

    // allocate space for the threads - free them after they joined
    threads = (pthread_t*) malloc(threadCount * sizeof(pthread_t));
    if(threads == NULL) {
	perror("malloc for threads failed");
	exit(2);
    }

    for(i=0; i<threadCount; i++) {
	// allocate space for the ThreadData structs
	// each thread will free those themselves so we can 'leak' it here
        struct ThreadData* td = (struct ThreadData*) malloc(sizeof(struct ThreadData));
	if(td == NULL) {
		perror("malloc for ThreadData failed");
		exit(2);
	}

        td->startPos = i * calculationsPerThread;
        td->pixelCount = calculationsPerThread;
        if(td->startPos + td->pixelCount > XSIZE * YSIZE) {
            printf("trimming %d to %d\n", td->pixelCount, ((XSIZE * YSIZE) - td->startPos));
            td->pixelCount = (XSIZE * YSIZE) - td->startPos;
        }

	// takeoff!
        pthread_create(&threads[i], NULL, threadRoutine, (void*) td);
    }

    printf("waiting for threads...\n");

    for(i=0; i<threadCount; i++) {
        pthread_join(threads[i], NULL);
        printf("thread %d joined.\n", i);
    }
    
    // free thread memory
    free(threads);

    /* write precalculated values to the file */
    for(y=YSIZE-1;y>=0;y--) {
            for(x=0;x<XSIZE;x++) {
	    	len=fwrite(&colorArray[y][x], 1, 3, fd);
                if(-1==len || len!=3) {
                    perror("write");
                    exit(4);
                }
            }
            /*no padding required because 1500%4 =0*/
    }
    fclose(fd);

    // I could free the char *dsc here that was malloc'ed beforehand
    // 
    // Quite funny that non-freed memory will give penalty
    // but in the initial setting they leak memory... :p
    free(dsc);

    // let's be verbose and explicitly return 0
    return 0;
}
