#pragma once
#include "page_frame.h"

#define  PAGE_DIR		0xC0300000
#define  PAGE_TABLE		0xC0000000

class MMU
{
public:
	MMU();
	~MMU();
	void	Init();
	uint32	map_memory(uint32 virtual_addr, uint32 physics_addr, uint32 size);
	void	unmap_memory(uint32 virtual_address, uint32 size);
};

