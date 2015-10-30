#pragma once
#include "typedef.h"
#include <string.h>
#include <stdio.h>
#include "../Os/kernel.h"

#define   PAGE_SIZE						0x1000
#define   PAGE_FRAME_INDEX(addr)			((addr)>>12)
#define   PD_INDEX(virtual_addr)			((virtual_addr)>>22)
#define   PT_INDEX(virtual_addr)			(((virtual_addr)>>12)&0x3FF)


#define   PAGE_FRAME_FREE					0
#define   PAGE_FRAME_LOW1M					1
#define   PAGE_FRAME_USER					2
#define   PAGE_FRAME_KERNEL					3
#define   PAGE_FRAME_NONE					0xff

#define		PT_PRESENT   0x001
#define		PT_WRITABLE  0x002
#define		PT_USER      0x004
#define		PT_ACCESSED  0x020
#define		PT_DIRTY     0x040

#define		MB(size)     ((uint32)size<<20) 

//http://wiki.osdev.org/Memory_Map_(x86)
struct BIOS_MEMORY
{
	byte  real_mode_ivt[0x00000400 - 0x00000000];	//0x00000000~0x000003ff
	byte  bios_data_area[0x00000500 - 0x00000400];		//0x00000400~0x000004ff
	byte  free_mem1[0x00007C00 - 0x00000500];//0x00000500~0x00007BFF
	byte  boot_mbr[0x00007E00 - 0x00007C00];			//0x00007C00~0x00007DFF
	byte  free_mem2[0x0009FC00 - 0x00007E00];				//0x00007E00	0x0007FFFF
	byte  bios_data_ext[0x000A0000 - 0x0009FC00];//		0x0009FC00 	0x0009FFFF
	byte  video_buf[0x000C0000 - 0x000A0000];
	byte  bios_rom[0x00100000 - 0x000C0000];
};

struct OS_MEM
{//0x10000
	byte boot_loader[0x10000];//0x00010000-0x0001FFFF
	byte os_loader[0x10000];	//0x00020000-0x0002FFFF
	byte page_dir[0x1000];	//0x00030000-0x00030FFF
	byte idt[0x1000];	//0x00030000-0x00030FFF
	byte gdt[0x1000];	//0x00030000-0x00030FFF
	byte tss[0x1000];	//0x00030000-0x00030FFF
};

//void*  init_page_frame_database(uint64 memsize);
//void*  alloc_memory(uint32 virtual_address, uint32 size);

class MMU
{
	uint32* m_page_dir;//物理地址
	uint32  m_next_free_page_frame;
public:
	MMU()
	{
		m_page_dir = NULL;
		m_next_free_page_frame = 0;
	}

	void    init(uint32 first_free_page_frame)
	{
		if (m_page_dir!=NULL)
		{
			printf("MMU already inited\n");
			return;
		}

		m_page_dir = NULL;
		m_next_free_page_frame = first_free_page_frame;

		//创建页目录
		//也可以理解为分配虚拟地址映射页表(0xC0000000~0xC03fffff)
		//自映射到0xC0300000 = 0xC0000000 + (0xC0000000>>12)
		m_page_dir = (uint32*)alloc_page_frame();
		memset(m_page_dir, 0, PAGE_SIZE);
		m_page_dir[PD_INDEX(PAGE_TABLE_BASE)] = (uint32)m_page_dir | PT_PRESENT | PT_WRITABLE;
		printf("page_dir=%08X\n", m_page_dir);
	}


	uint32* page_dir()
	{
		return m_page_dir;
	}

	void   startup_page_mode()
	{
		uint32 page_dir = (uint32)m_page_dir;
		__asm mov		eax, dword ptr[page_dir]
		__asm mov		cr3, eax
		__asm mov		eax, cr0
		__asm or			eax, 0x80000000
		__asm mov		cr0, eax
		printf("Entry page mode\n");
	}
	
	uint32 next_free_page_frame()
	{
		return m_next_free_page_frame;
	}

	uint32  alloc_page_frame()
	{
		uint32 page_addr = m_next_free_page_frame * PAGE_SIZE;
		m_next_free_page_frame++;
		return page_addr;
	}

	bool   map_low_1M_memory()
	{//物理内存已经分配，此时还没有打开分页机制，page_dir、page_table均为线性(物理)地址
		if (m_page_dir == NULL)
		{
			printf("MMU hasn't inited\n");
			return false;
		}
		uint32* page_table0 = (uint32*)alloc_page_frame();
		memset((void*)page_table0, 0, PAGE_SIZE);
		m_page_dir[0] = (uint32)page_table0 | PT_PRESENT | PT_WRITABLE;
		for (uint32 i = 0; i < (MB(1)>>12); i++)
		{
			uint32 addr = i * PAGE_SIZE;
			uint32 pt_index = PT_INDEX(addr);
			page_table0[pt_index] = addr | PT_PRESENT | PT_WRITABLE;
		}
		printf("map_low_1M_memory OK\n");
		return true;
	}

	void*   alloc_memory(uint32 virtual_address, uint32 size)
	{//分页机制已经开启
		if (m_page_dir == NULL)
		{
			printf("MMU hasn't inited\n");
			return false;
		}
		uint32* page_dir = (uint32*)0xC0300000;
		uint32 pages = (size + PAGE_SIZE - 1) >> 12;
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
			page_table_VA[pt_index] = pa | PT_PRESENT | PT_WRITABLE;
		}
		printf("Alloc memory at %08X size=%d\n", virtual_address, size);
		return (void*)virtual_address;
	}
};