#ifndef __ALGORITHM_H
#define __ALGORITHM_H

  #ifdef __FRACTAL_EXPORT
    #if defined(_WIN32) || defined(_WIN64)
      #define FRACTALDLL   __declspec( dllexport )
    #else
      #define FRACTALDLL
    #endif
  #else
    
    #if defined(_M_X64)
      #pragma comment(lib,"fractal_x64.lib")
      #pragma message("x86_64") 
      #define FRACTALDLL   __declspec( dllimport )
    #elif defined(_M_IX86)
      #pragma comment(lib,"fractal_x86.lib")
      #pragma message("x86")
      #define FRACTALDLL   __declspec( dllimport )
    #else
      #define FRACTALDLL	
    #endif
  #endif

  #if ! defined(BYTE)
   typedef unsigned char BYTE;
  #endif

FRACTALDLL int getColorValuesAt(double x, double y, BYTE *red_value, BYTE *green_value, BYTE *blue_value);
FRACTALDLL int getDescription(char *name, int *len); /* returns the description of the algorithm.
                                                       * returnvalue 0   operation successful
                                                       *             -1  invalid arguments
                                                       * if name==NULL then len contains required length
                                                       * if len is less than the required size, then -1 is returned
                                                       * and len contains the required length
                                                       */
FRACTALDLL int getId();/* returns algorithm id */

#endif
