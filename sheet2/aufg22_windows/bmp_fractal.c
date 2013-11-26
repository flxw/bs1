/* Betriebssystem & Middleware
 *
 * Betriebssysteme I WS 2011/2012
 *
 * Uebung 3.3
 */
#include <stdio.h>
#include <errno.h>

#define XSIZE 500
#define YSIZE 500
#include "algorithm.h"


int main(int argc, char *argv[])
{
    FILE *fd;
    int len,x,y;
    char *dsc;
    char bgr[3];
    short svalue;
    int   lvalue;
    unsigned char header[54],*ptr=&header[0];
      
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
    
    if(-1==len || len!=sizeof(header))
    {
        perror("write");
        exit(2);
    }
#pragma message("!!!!       Implement Multithreading here    !!!!")    
    for(y=YSIZE-1;y>=0;y--)
    {
            for(x=0;x<XSIZE;x++)
            {
                getColorValuesAt(x * (2.0 / XSIZE) - 1.5, y * (2.0 / YSIZE) - 1.0,&bgr[2],&bgr[1],&bgr[0]); 
                
                len=fwrite(bgr,1,3,fd);
                if(-1==len || len!=3)
                {
                    perror("write");
                    exit(4);
                }
            }
            /*no padding required because 1500%4 =0*/
    }        
    fclose(fd);
    
}
