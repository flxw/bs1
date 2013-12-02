/* Betriebssystem & Middleware
 *
 * Betriebssysteme I WS 2011/2012
 *
 * Uebung 3.3
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <windows.h>

#define XSIZE 500
#define YSIZE 500
#include "algorithm.h"

/* cArray - pointer to colorArray
   tpArray - coordinates for which the colors are to be calculated
   numberOfCalculations - guess what */
struct ThreadData {
	int** threadPositionArray;
	char** calculatedColors;
	int coordinateCount;
};

DWORD WINAPI calculateColorsInsideThread(LPVOID stupidVoid) {
	struct ThreadData *td = (struct ThreadData*) stupidVoid;
	int i,x,y;
	char* c;

	for (i = 0; i < td->coordinateCount; ++i) {
		x = td->threadPositionArray[i][0];
		y = td->threadPositionArray[i][1];
		c = td->calculatedColors[i];

		getColorValuesAt(x * (2.0 / XSIZE) - 1.5, y * (2.0 / YSIZE) - 1.0,&c[2],&c[1],&c[0]);
	}

	return 0;
}


int main(int argc, char *argv[])
{
    FILE *fd;
	//int threadCount = atoi(argv[1]);
	int threadId,currentCalculation;
	int assignedCoordinates = 0;
	int threadCount = 100;
	int calculationsPerThread = ceil((float)XSIZE*YSIZE/(float)threadCount);
    int len,x,y;
    char *dsc;
    char colorArray[YSIZE][XSIZE][3];
	int*** threadPositionArray = (int***)malloc(threadCount * sizeof(int***));
	struct ThreadData* dataArray = (struct ThreadData*)malloc(threadCount * sizeof(struct ThreadData));
    short svalue;
    int   lvalue;
    unsigned char header[54],*ptr=&header[0];
	HANDLE* threadHandles = (HANDLE*)malloc(threadCount * sizeof(HANDLE));
	int i,j;
	int lastThreadCalculations = XSIZE*YSIZE - (threadCount-1)*calculationsPerThread;

	// --- init threadposition Array
	for (j=0; j < threadCount; ++j) {
		dataArray[j].threadPositionArray = (int**)malloc(calculationsPerThread * sizeof(int**));
		dataArray[j].calculatedColors    = (char**)malloc(calculationsPerThread * sizeof(char**));
		//threadPositionArray[j] = (int**)malloc(calculationsPerThread * sizeof(int**));
		for (i=0; i < calculationsPerThread; ++i) {
			//threadPositionArray[j][i] = (int*)malloc(2 * sizeof(int));
			dataArray[j].threadPositionArray[i] = (int*)malloc(2 * sizeof(int));
			dataArray[j].calculatedColors[i] = (char*)malloc(3 * sizeof(char));
		}
	}
      
    getDescription(NULL,&len);
    if(NULL==(dsc=(char*)malloc(sizeof(char)*len)))
    {
        perror("malloc");
        exit(1);
    }
    getDescription(dsc,&len);
    
    printf("Calculate %s %d\n",dsc,getId());
    fd=fopen("test.bmp","wb+");
    if(NULL==fd)
    {
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

	// ---- prepare thread startup
	// ---- note down individual coordinates here for each thread
	assignedCoordinates = 0;

    for(y=YSIZE-1;y>=0;y--)
    {
            for(x=0;x<XSIZE;x++)
            {
				threadId = (int)(assignedCoordinates/calculationsPerThread);
				currentCalculation = assignedCoordinates%calculationsPerThread;
				dataArray[threadId].threadPositionArray[currentCalculation][0] = x;
				dataArray[threadId].threadPositionArray[currentCalculation][1] = y;
				//dataArray[threadId].coordinateCount = calculationsPerThread;
				dataArray[threadId].coordinateCount = (threadId+1 == threadCount) ? lastThreadCalculations : calculationsPerThread;
				assignedCoordinates++;
            }
	}


	// --- thread startup -------------------------
	for (i=0; i < threadCount; ++i) {
		threadHandles[i] = CreateThread(
			NULL,
			0,
			calculateColorsInsideThread,
			&dataArray[i],
			0,
			NULL);
	}

	// --- thread shutdown ----
	for(i = 0; i < threadCount; i += MAXIMUM_WAIT_OBJECTS) {
		WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS, threadHandles, TRUE, INFINITE);
	}

	// --- extract results ----
	assignedCoordinates = 0;
	for (j = 0; j < threadCount; ++j) {
		for (i = 0; i < dataArray[j].coordinateCount; ++i) {
			len=fwrite(dataArray[j].calculatedColors[i],1,3,fd);
			if(-1==len || len!=3) {
				perror("write");
				exit(4);
			}
		}
	}

    fclose(fd);
}