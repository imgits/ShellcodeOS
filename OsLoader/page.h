//#include <stdio.h>
#include "typedef.h"
#include "vga.h"
/*
2^12 = 4K
2^(12+9)=2^21=2M
2^(12+9+9)=2^30=1G
2^(12+9+9+9)=2^39=512G
2^(12+9+9+9+9)=2^48=256T
 
bits size
10 = 1K
20 = 1M
30 = 1G
40 = 1T
43 = 8T 
44 = 16T
48 = 256T
50 = ?
60 = 1EB
64 = 16EB
*/

#define  PML4_BASE  0xFFFFF6FB7DBED000
#define  PDP_BASE   0xFFFFF6FB7DA00000
#define  PD_BASE    0xFFFFF6FB40000000
#define  PT_BASE    0xFFFFF68000000000

uint32 alloc_physical_pages(uint32 pages);
void   free_physical_pages(uint32  start_page, uint32 pages);

class CPager
{
private:

public:
	CPager()
	{
		printf("CPager()");
	}
	~CPager()
	{

	}
};
