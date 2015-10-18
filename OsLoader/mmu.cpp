#include "mmu.h"
#include <string.h>
#include "stdio.h"

byte*	g_page_frame_database = (byte*)PAGE_FRAME_BASE;
uint32	g_page_frame_min = 0;
uint32	g_page_frame_max = 0;
uint32*  g_page_dir = (uint32*)NULL;

void*   init_page_frame_database(uint64 memsize)
{
	g_page_frame_min = PAGE_FRAME_INDEX(0x100000);
	g_page_frame_max = PAGE_FRAME_INDEX(memsize);

	printf("g_page_frame_min=%x\n", g_page_frame_min);
	printf("g_page_frame_max=%x\n", g_page_frame_max);

	//分配页目录帧
	//也可以理解为分配虚拟地址映射页表(0xC0000000~0xC03fffff)
	//自映射到0xC0300000 = 0xC0000000 + (0xC0000000>>12)
	g_page_dir = (uint32*)alloc_page_frame();

	memset(g_page_dir, 0, PAGE_SIZE);
	g_page_dir[PD_INDEX(PAGE_TABLE_BASE)] = (uint32)g_page_dir | PT_PRESENT | PT_WRITABLE;

	printf("g_page_dir=%08X\n", g_page_dir);

	//分配虚拟地址映射页表(0x00000000~0x003fffff)
	//用于映射PA_0x00000000~0x000fffff ==> VA_0x00000000~0x000fffff
	uint32* page_table = (uint32*)alloc_page_frame();
	g_page_dir[PD_INDEX(0x00000000)] = (uint32)page_table | PT_PRESENT | PT_WRITABLE;
	memset(page_table, 0, PAGE_SIZE);
	for (int32 i = 0; i < 256; i++)
	{//映射前256项(1M)
		page_table[i] = (i * PAGE_SIZE) | PT_PRESENT | PT_WRITABLE;
	}
	printf("g_page_table_0x000000000=%08X\n", page_table);

	//分配页表，用于映射内核代码至VA_0x80000000~....
	//g_page_table_0x800000000 = (uint32*)(g_page_frame_min++ * PAGE_SIZE);

	//分配虚拟地址映射页表(0xC0400000~0xC07fffff)
	page_table = (uint32*)alloc_page_frame();
	g_page_dir[PD_INDEX(PAGE_FRAME_BASE)] = (uint32)page_table | PT_PRESENT | PT_WRITABLE;
	memset(page_table, 0, PAGE_SIZE);
	//映射page_frame_database至VA_0xC00400000~C004fffff(1M)
	for (int32 i = 0; i < 256; i++)
	{//分配并映射256页(1M)
		uint32 page_frame = alloc_page_frame();
		uint32 page_addr = page_frame * PAGE_SIZE;
		uint32 page_va = PAGE_FRAME_BASE + (i * PAGE_SIZE);
		page_table[PT_INDEX(page_va)] = page_addr | PT_PRESENT | PT_WRITABLE;
		memset((void*)page_addr, 0, PAGE_SIZE);
	}
	//映射物理页PA_0x00000000~0x000fffff至VA_0xC00500000~C005fffff(1M)
	for (int32 i = 0; i < 256; i++)
	{//映射256页(1M)
		uint32 page_addr = i * PAGE_SIZE;
		uint32 page_va = PAGE_LOW1M_BASE + (i * PAGE_SIZE);
		page_table[PT_INDEX(page_va)] = page_addr | PT_PRESENT | PT_WRITABLE;
	}
	
	printf("g_page_table_0xC00400000=%08X\n", page_table);

	for (int32 i = 0; i < (0x100000 >> 12); i++)
	{
		g_page_frame_database[i] = PAGE_FRAME_LOW1M;
	}

	//for (int32 i = g_page_frame_max; i < 0x100000; i++)
	//{
	//	g_page_frame_database[i] = PAGE_FRAME_NONE;
	//}
	
	return g_page_dir;
}

uint32  alloc_page_frame()
{
	uint32 page_addr = g_page_frame_min * PAGE_SIZE;
	g_page_frame_min ++;
	return page_addr;
}

void*   map_kernel_space(uint32 virtual_address, uint32 size)
{
	uint32* page_dir = (uint32*)0xC0300000;
	uint32 physical_address = 0;
	uint32 pages = (size + PAGE_SIZE - 1) >> 12;
	uint32 protect = PT_PRESENT | PT_WRITABLE;

	for (uint32 i = 0; i < pages; i++)
	{
		uint32 va = virtual_address + i * PAGE_SIZE;
		uint32 pa = alloc_page_frame();
		uint32 pd_index = PD_INDEX(va);
		uint32 pt_index = PT_INDEX(va);
		uint32* page_table_VA = (uint32*)(PAGE_TABLE_BASE + (pd_index * PAGE_SIZE));
		if (page_dir[pd_index] == 0)
		{
			uint32  page_table_PA = alloc_page_frame();
			page_dir[pd_index] = page_table_PA | PT_PRESENT | PT_WRITABLE;
			memset(page_table_VA, 0, PAGE_SIZE);
		}
		page_table_VA[pt_index] = pa | protect;
	}
	return (void*)virtual_address;
}