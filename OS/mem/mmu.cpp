#include "mmu.h"
#include <string.h>
#include "page_frame.h"
#include "paging.h"

MMU<4096>	  kmem;

char buf[1024 * 1024];
extern "C"  int liballoc_lock()
{
	return 0;
}

extern "C"  int liballoc_unlock()
{
	return 0;
}

extern "C"  void* liballoc_alloc(int size)
{
	return buf;
}

extern "C"  int liballoc_free(void*ptr, int size)
{
	return 0;
}