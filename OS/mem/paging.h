#pragma once
#include "typedef.h"

class PAGER
{
public:
	static uint32 map_pages(uint32 physical_address, uint32 virtual_address, int size, int protect);
	static void unmap_pages(uint32 virtual_address, int size);
};

