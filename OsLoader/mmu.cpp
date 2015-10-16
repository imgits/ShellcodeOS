#include "mmu.h"
#include <string.h>

//4G = 1M*4K = 8*128K*4K = 8*32*4K*4K
static byte*		g_page_frame_bitmap = NULL;
static uint32	g_page_frame_min = 0;
static uint32	g_page_frame_max = 0;
static uint32	g_next_free_page = 0;
static uint32*   g_page_dir = (uint32*)PAGE_DIR_PHYSICAL_ADDRESS;
static uint32*   g_page_table_0_4M = (uint32*)NULL;
static uint32*   g_page_table_0x000000000 = (uint32*)NULL;
static uint32*   g_page_table_0x800000000 = (uint32*)NULL;
static uint32*   g_page_table_0xC00400000 = (uint32*)NULL;

void   init_page_frame_database(uint64 memsize)
{
	g_page_frame_min = 0x100000 >> 12;
	g_page_frame_max = memsize >> 12;

	//分配页目录帧
	g_page_dir = (uint32*)(g_page_frame_min++ * PAGE_SIZE);
	memset(g_page_dir, 0, PAGE_SIZE);
	g_page_dir[PD_INDEX(PAGE_TABLE_BASE)] = (uint32)g_page_dir | PT_PRESENT | PT_WRITABLE;

	//分配页表，用于映射PA_0x00000000~0x000fffff ==> VA_0x00000000~0x000fffff
	g_page_table_0x000000000 = (uint32*)(g_page_frame_min++ * PAGE_SIZE);
	g_page_dir[PD_INDEX(0x00000000)] = (uint32)g_page_table_0x000000000 | PT_PRESENT | PT_WRITABLE;
	memset(g_page_table_0x000000000, 0, PAGE_SIZE);
	for (int32 i = 0; i < 256; i++)
	{
		g_page_table_0x000000000[i] = (i * PAGE_SIZE) | PT_PRESENT | PT_WRITABLE;
	}

	//分配页表，用于映射内核代码至VA_0x80000000~....
	//g_page_table_0x800000000 = (uint32*)(g_page_frame_min++ * PAGE_SIZE);

	//分配页表
	//映射page_frame_database至VA_0xC00400000~C004fffff(1M)
	//映射PA_0x00000000~0x000fffff至VA_0xC00500000~C005fffff(1M)
	g_page_table_0xC00400000 = (uint32*)(g_page_frame_min++ * PAGE_SIZE);

	
	
	
	g_page_dir[PD_INDEX(PAGE_FRAME_BASE)] = (uint32)g_page_table_0xC00400000 | PT_PRESENT | PT_WRITABLE;


	uint32 physical_pages = g_page_frame_max;
	uint32 database_pages = (physical_pages / 8 / 0x1000) + 1;
	
	for (int32 i = 0; i < database_pages; i++)
	{
		uint32 page_frame = g_page_frame_min++;
		uint32 page_addr = page_frame * PAGE_SIZE;
		uint32 page_va = PAGE_FRAME_BASE + (i * PAGE_SIZE);
		g_page_table_0xC00400000[PT_INDEX(page_va)] = page_addr | PT_PRESENT | PT_WRITABLE;
		memset((void*)page_addr, 0, PAGE_SIZE);
	}
	
}

uint32 get_page_state(uint32 page)
{
	return g_page_frame_bitmap[page >> 3] & (1 << (page & 7));
}

void set_page_used(uint32 page)
{
	g_page_frame_bitmap[page >> 3] |= (1 << (page & 7));
}

void set_page_free(uint32 page)
{
	g_page_frame_bitmap[page >> 3] &= ~(1 << (page & 7));
}

uint32 alloc_physical_page()
{
	for (uint32 i = g_next_free_page; i < g_page_frame_max; i++)
	{
		if (get_page_state(i) == 0)
		{
			g_next_free_page = i + 1;
			return i;
		}
	}
	return 0;
}

void   free_physical_page(uint32 page)
{
	set_page_free(page);
	if (page < g_next_free_page)
	{
		g_next_free_page = page;
	}
}

uint32 alloc_physical_pages(uint32 pages)
{
	for (uint32 i = g_next_free_page; i < g_page_frame_max-pages; i++)
	{
		if (get_page_state(i) == 0)
		{
			uint32 j = i;
			for (j++; j < i + pages && j < g_page_frame_max; j++)
			{
				if (get_page_state(i) != 0) break;
			}
			if (j = i + pages)
			{
				g_next_free_page = j;
				return i;
			}
		}
	}
	return 0;
}

void   free_physical_pages(uint32  start_page, uint32 pages)
{
	for (uint32 i = start_page; i < start_page + pages; i++)
	{
		set_page_free(i);
	}
	if (g_next_free_page > start_page)
	{
		g_next_free_page = start_page;
	}
}

uint32* get_page_table_va(uint32 virtual_address)
{
	uint32 pd_index = PD_INDEX(virtual_address);
	uint32 pde = g_page_dir[pd_index];
	uint32 page_table_pa = pde & PAGE_FRAME_MASK;
	return (uint32*)page_table_pa;
}

uint32* get_page_table_pa(uint32 virtual_address)
{
	uint32 pd_index = PD_INDEX(virtual_address);
	if (g_page_dir[pd_index] == 0)
	{
		return (uint32*)NULL;
	}
	uint32* page_table = (uint32*)(PAGE_TABLE_BASE + (pd_index * PAGE_SIZE));
	return page_table;
}

void* map_pages(uint32 virtual_address, uint32 physical_address, uint32 pages, uint32 protect)
{
	for (uint32 i = 0; i < pages; i++)
	{
		uint32 va = virtual_address + i * PAGE_SIZE;
		uint32 pa = physical_address + i * PAGE_SIZE;
		uint32 pd_index = PD_INDEX(va);
		uint32 pt_index = PT_INDEX(va);
		if (g_page_dir[pd_index] == 0)
		{
			uint32 pt_pa = alloc_physical_page();
			g_page_dir[pd_index] = pt_pa | PT_PRESENT | PT_WRITABLE;
		}
		uint32* page_table = (uint32*)(PAGE_TABLE_BASE + (pd_index * PAGE_SIZE));
		page_table[pt_index] = pa | protect;
	}
	return (void*)virtual_address;
}