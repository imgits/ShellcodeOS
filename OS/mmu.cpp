#include "mmu.h"
#include <string.h>

MMU::MMU()
{
}


MMU::~MMU()
{
}

void MMU::Init()
{
}

uint32 MMU::map_memory(uint32 virtual_addr, uint32 physical_addr, uint32 size)
{
	uint32* page_dir = (uint32*)PAGE_DIR;
	uint32 protect = PT_PRESENT | PT_WRITABLE;

	uint32 virtual_address = virtual_addr & 0xfffff000;
	uint32 physical_address = physical_addr & 0xfffff000;
	uint32 pages = ((virtual_addr - virtual_address) + size + PAGE_SIZE - 1) >> 12;
	for (uint32 i = 0; i < pages; i++)
	{
		uint32 va = virtual_address + i * PAGE_SIZE;
		uint32 pa = physical_address + i * PAGE_SIZE;
		uint32 pd_index = PD_INDEX(va);
		uint32 pt_index = PT_INDEX(va);
		uint32* page_table_VA = (uint32*)(PAGE_TABLE_BASE + (pd_index * PAGE_SIZE));
		if (page_dir[pd_index] == 0)
		{
			uint32  page_table_PA = PAGE_FRAME_DB::alloc_physical_page();
			page_dir[pd_index] = page_table_PA | PT_PRESENT | PT_WRITABLE;
			memset(page_table_VA, 0, PAGE_SIZE);
		}
		page_table_VA[pt_index] = pa | protect;
	}
	return virtual_address;
}

void MMU::unmap_memory(uint32 virtual_addr, uint32 size)
{
	uint32* page_dir = (uint32*)PAGE_DIR;
	uint32 protect = PT_PRESENT | PT_WRITABLE;

	uint32 virtual_address = virtual_addr & 0xfffff000;
	uint32 pages = ((virtual_addr - virtual_address) + size + PAGE_SIZE - 1) >> 12;

	for (uint32 i = 0; i < pages; i++)
	{
		uint32 va = virtual_address + i * PAGE_SIZE;
		uint32 pd_index = PD_INDEX(va);
		uint32 pt_index = PT_INDEX(va);
		uint32* page_table_VA = (uint32*)(PAGE_TABLE_BASE + (pd_index * PAGE_SIZE));
		if (page_dir[pd_index] != 0)
		{
			page_table_VA[pt_index] = 0;
		}
	}
}

