#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

double drand48(void)  
{  
	#define m 0x100000000LL  
	#define c 0xB16  
	#define a 0x5DEECE66DLL  
	static unsigned long long seed = 1;
    seed = (a * seed + c) & 0xFFFFFFFFFFFFLL;  
    unsigned int x = seed >> 16;  
    return  ((double)x / (double)m);  
      
} 