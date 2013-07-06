#include <stdio.h>
#include <stdlib.h>

char *
strsep(char **stringp, const char* delim)
{
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;

	if ((s = *stringp) == NULL)
		return (NULL);
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

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