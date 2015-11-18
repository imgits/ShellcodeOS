#include "mmu.h"
#include <string.h>
#include "paging.h"
#include "system.h"
#include "liballoc.h"

extern "C"  int liballoc_lock()
{
	return 0;
}

extern "C"  int liballoc_unlock()
{
	return 0;
}

extern "C"  void* liballoc_alloc(int pages)
{
	return (void*)System.m_kmem.alloc_virtual_memory(pages*PAGE_SIZE);
}

extern "C"  int liballoc_free(void*ptr, int size)
{
	return 0;
}