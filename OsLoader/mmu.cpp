#include "mmu.h"
#include <string.h>
#include "stdio.h"

byte*	g_page_frame_database = (byte*)PAGE_FRAME_BASE;
uint32	g_page_frame_min = 0;
uint32	g_page_frame_max = 0;
uint32*  g_page_dir = (uint32*)NULL;

uint32 __alloc_page_frame();
void*  __alloc_memory_before_paging(uint32 virtual_address, uint32 size);
void*  __map_memory_before_paging(uint32 physical_address, uint32 virtual_address, uint32 size);



void*   init_page_frame_database(uint64 memsize)
{
	g_page_frame_min = PAGE_FRAME_INDEX(0x100000);
	g_page_frame_max = PAGE_FRAME_INDEX(memsize);

	printf("g_page_frame_min=%x\n", g_page_frame_min);
	printf("g_page_frame_max=%x\n", g_page_frame_max);

	//分配页目录帧
	//也可以理解为分配虚拟地址映射页表(0xC0000000~0xC03fffff)
	//自映射到0xC0300000 = 0xC0000000 + (0xC0000000>>12)
	g_page_dir = (uint32*)__alloc_page_frame();
	memset(g_page_dir, 0, PAGE_SIZE);
	g_page_dir[PD_INDEX(PAGE_TABLE_BASE)] = (uint32)g_page_dir | PT_PRESENT | PT_WRITABLE;

	printf("g_page_dir=%08X\n", g_page_dir);

	//映射物理页PA_0x00000000~0x000fffff ==> VA_0x00000000~0x000fffff(1M)
	//映射物理页PA_0x00000000~0x000fffff ==> VA_0xC00500000~C005fffff(1M)
	__map_memory_before_paging(0x00000000, 0x00000000, 0x00100000);
	__map_memory_before_paging(0x00000000, PAGE_LOW1M_BASE, 0x00100000);

	printf("map low 1M memory OK\n");

	//映射分配page_frame_database并映射至VA_0xC00400000~C004fffff(1M)
	__alloc_memory_before_paging(PAGE_FRAME_BASE, 0x00100000);

	printf("Create page frame database OK\n");

	for (int32 i = 0; i < (0x100000 >> 12); i++)
	{
		g_page_frame_database[i] = PAGE_FRAME_LOW1M;
	}

	for (int32 i = g_page_frame_max; i < 0x100000; i++)
	{
		g_page_frame_database[i] = PAGE_FRAME_NONE;
	}
	
	return g_page_dir;
}

uint32  __alloc_page_frame()
{
	uint32 page_addr = g_page_frame_min * PAGE_SIZE;
	g_page_frame_min ++;
	return page_addr;
}

void*   __map_memory_before_paging(uint32 physical_address, uint32 virtual_address, uint32 size)
{//物理内存已经分配，此时还没有打开分页机制，page_dir、page_table均为线性(物理)地址
	uint32* page_dir = (uint32*)g_page_dir;
	uint32 pages = (size + PAGE_SIZE - 1) >> 12;
	uint32 protect = PT_PRESENT | PT_WRITABLE;

	for (uint32 i = 0; i < pages; i++)
	{
		uint32 va = virtual_address + i * PAGE_SIZE;
		uint32 pa = physical_address + i * PAGE_SIZE;
		uint32 pd_index = PD_INDEX(va);
		uint32 pt_index = PT_INDEX(va);
		uint32 page_table = ((uint32)page_dir[pd_index]) & 0xFFFFF000;
		//printf("map_address_space(%4d %08X == > %08X page_table=%08X\n", i, pa, va, page_table);
		if (page_table == 0)
		{
			page_table = __alloc_page_frame();
			page_dir[pd_index] = page_table | PT_PRESENT | PT_WRITABLE;
			memset((void*)page_table, 0, PAGE_SIZE);
		}
		((uint32*)page_table)[pt_index] = pa | protect;
	}
	//printf("map_address_space end\n");
	return (void*)virtual_address;
}

void*   __alloc_memory_before_paging(uint32 virtual_address, uint32 size)
{//物理内存还没有分配, 此时还没有打开分页机制，page_dir、page_table均为线性(物理)地址
	uint32* page_dir = (uint32*)g_page_dir;
	uint32 pages = (size + PAGE_SIZE - 1) >> 12;
	uint32 protect = PT_PRESENT | PT_WRITABLE;
	for (uint32 i = 0; i < pages; i++)
	{
		uint32 va = virtual_address + i * PAGE_SIZE;
		uint32 pa = __alloc_page_frame();
		uint32 pd_index = PD_INDEX(va);
		uint32 pt_index = PT_INDEX(va);
		uint32 page_table = ((uint32)page_dir[pd_index]) & 0xFFFFF000;
		//printf("map_address_space(%4d %08X == > %08X page_table=%08X\n", i, pa, va, page_table);
		if (page_table == 0)
		{
			page_table = __alloc_page_frame();
			page_dir[pd_index] = page_table | PT_PRESENT | PT_WRITABLE;
			memset((void*)page_table, 0, PAGE_SIZE);
		}
		((uint32*)page_table)[pt_index] = pa | protect;
	}
	//printf("map_address_space end\n");
	return (void*)virtual_address;
}

void*   alloc_memory(uint32 virtual_address, uint32 size)
{//分页机制已经开启
	uint32* page_dir = (uint32*)0xC0300000;
	uint32 pages = (size + PAGE_SIZE - 1) >> 12;
	uint32 protect = PT_PRESENT | PT_WRITABLE;
	for (uint32 i = 0; i < pages; i++)
	{
		uint32 va = virtual_address + i * PAGE_SIZE;
		uint32 pa = __alloc_page_frame();
		uint32 pd_index = PD_INDEX(va);
		uint32 pt_index = PT_INDEX(va);
		uint32* page_table_VA = (uint32*)(PAGE_TABLE_BASE + (pd_index * PAGE_SIZE));
		if (page_dir[pd_index] == 0)
		{
			uint32  page_table_PA = __alloc_page_frame();
			page_dir[pd_index] = page_table_PA | PT_PRESENT | PT_WRITABLE;
			memset(page_table_VA, 0, PAGE_SIZE);
		}
		page_table_VA[pt_index] = pa | protect;
	}
	return (void*)virtual_address;
}