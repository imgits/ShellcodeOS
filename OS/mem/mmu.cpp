#include "mmu.h"
#include <string.h>
#include "page_frame.h"
#include "paging.h"

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
	return NULL;
}

extern "C"  int liballoc_free(void*ptr, int size)
{
	return 0;
}