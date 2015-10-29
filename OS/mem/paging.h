#pragma once
#include "typedef.h"

#define		PT_PRESENT   0x001

#define		PT_WRITABLE  0x002
#define		PT_READONLY  0x000

#define		PT_USER      0x004
#define		PT_KERNEL    0x000

#define		PT_ACCESSED  0x020
#define		PT_DIRTY     0x040


class PAGER
{
public:
	static uint32 map_pages(uint32 physical_address, uint32 virtual_address, int size, int protect = (PT_PRESENT | PT_WRITABLE) );
	static void unmap_pages(uint32 virtual_address, int size);
};

