#include "paging.h"
#include "page_frame.h"
#include <string.h>

uint32 PAGER::map_pages(uint32 physical_addr, uint32 virtual_addr, int size, int protect)
{
	uint32* page_dir = (uint32*)PAGE_DIR_BASE;
	if (USER_SPACE(virtual_addr)) protect |= PT_USER;
	
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
		{//页表不存在，则分配一个物理页作为页表
			uint32  page_table_PA = PAGE_FRAME_DB::alloc_physical_page();

			if  (KERNEL_SPACE(va)) page_dir[pd_index] = page_table_PA | PT_PRESENT | PT_WRITABLE;
			else page_dir[pd_index] = page_table_PA | PT_PRESENT | PT_WRITABLE | PT_USER;
			memset(page_table_VA, 0, PAGE_SIZE);
		}
		page_table_VA[pt_index] = pa | protect;
	}
	return virtual_address;
}

void PAGER::unmap_pages(uint32 virtual_addr, int size)
{
	uint32* page_dir = (uint32*)PAGE_DIR_BASE;

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
			uint32 page = page_table_VA[pt_index] >> 12;
			page_table_VA[pt_index] = 0;
			__asm { invlpg va } //无效TLB
			PAGE_FRAME_DB::free_physical_page(page);
		}
	}
}
