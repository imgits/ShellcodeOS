#pragma once
#include "page_frame.h"

#define  PAGE_DIR		0xC0300000
#define  PAGE_TABLE	0xC0000000

class MMU
{
private:
	PAGE_FRAME_DB* m_page_frame_db;
public:
	MMU();
	~MMU();
	void Init(PAGE_FRAME_DB* page_frame_db);
	uint32 map_memory(uint32 virtual_addr, uint32 physics_addr, uint32 size);
	void   unmap_memory(uint32 virtual_address, uint32 size);
};

