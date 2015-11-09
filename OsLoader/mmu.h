#pragma once
#include "typedef.h"
#include <string.h>
#include <stdio.h>
#include <intrin.h>

#include "../Os/kernel.h"

#define   PAGE_SIZE							0x1000
#define   PDE_INDEX(virtual_addr)			((virtual_addr)>>22)
#define   PTE_INDEX(virtual_addr)			(((virtual_addr)>>12)&0x3FF)
#define   PDE_ADDRESS(pde)					((pde)&0xFFFFF000)
#define   PDE_ATTR(pde)						((pde)&0x00000FFF)

#define		PT_PRESENT   0x001
#define		PT_WRITABLE  0x002
#define		PT_USER      0x004
#define		PT_ACCESSED  0x020
#define		PT_DIRTY     0x040
#define		PT_4M		 0x080

#define		MB(size)     ((uint32)size<<20) 

#define     PAGE_DIR_PA				0x00030000
#define     PT_LOW1M_PA   			0x00031000
#define     PT_KERNEL_PA   			0x00032000

#if 0
//将物理内存映射到虚拟地址空间
//页面大小为4K，内存按照4M边界对齐
static void    map_4K_pages(uint32* page_dir, uint32* page_table, uint32 physcail_address, uint32 virtual_address, uint32 size)
{
	page_dir[PDE_INDEX(virtual_address)] = (uint32)page_table | PT_PRESENT | PT_WRITABLE;
	uint32 pages = size >> 12;
	for (int i = 0; i < pages; i++)
	{
		uint32 pa = physcail_address + i*PAGE_SIZE;
		page_table[i] = pa | PT_PRESENT | PT_WRITABLE;
	}
}

//映射4M大页面
static void    map_4M_pages(uint32* page_dir, uint32 physcail_address, uint32 virtual_address, uint32 size)
{
	uint32 pages = size >> 22;
	for (int i = 0; i < pages; i++)
	{
		uint32 pa = physcail_address + i*MB(4);
		uint32 va = virtual_address + i*MB(4);
		page_dir[PDE_INDEX(va)] = pa | PT_PRESENT | PT_WRITABLE | PT_4M;
	}
}
#endif

//在以4M为边界的4M范围内映射内存地址空间
static void    map_pages_within_4M(uint32* page_dir, uint32* page_table, uint32 physcail_address, uint32 virtual_address, uint32 size)
{
	memset(page_table, 0, PAGE_SIZE);
	page_dir[PDE_INDEX(virtual_address)] = (uint32)page_table | PT_PRESENT | PT_WRITABLE;

	uint32 pages = size >> 12;
	uint32 pt_index = PTE_INDEX(virtual_address);
	for (int i = pt_index; i < pt_index + pages; i++)
	{
		uint32 pa = physcail_address + i*PAGE_SIZE;
		page_table[i] = pa | PT_PRESENT | PT_WRITABLE;
	}
}

static void	startup_page_mode()
{
	uint32* page_dir = (uint32*)PAGE_DIR_PA;
	memset(page_dir, 0, PAGE_SIZE);
	
	//页目录自映射
	page_dir[PDE_INDEX(PAGE_TABLE_BASE)] = (uint32)page_dir | PT_PRESENT | PT_WRITABLE;
	
	//映射物理内存00000000-000FFFFF至虚拟地址空间00000000-000FFFFF
	map_pages_within_4M(page_dir, (uint32*)PT_LOW1M_PA, 0, 0, MB(1));

	//映射物理内存00100000-004FFFFF至虚拟地址空间80000000-803FFFFF
	//OsLoader假定OS kernel大小超过4M
	map_pages_within_4M(page_dir, (uint32*)PT_KERNEL_PA, KERNEL_START_PA, KERNEL_BASE, MB(4));

	__asm mov		eax, dword ptr[page_dir]
	__asm mov		cr3, eax
	
	__writecr4(__readcr4() | 0x00000010); //CR4.PSE = 1 允许使用4M大页面
	
	__asm mov		eax, cr0
	__asm or		eax, 0x80000000
	__asm mov		cr0, eax
	printf("Entry page mode\n");
}
