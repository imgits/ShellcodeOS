#include "page_frame.h"
#include <string.h>

byte*  PAGE_FRAME_DB::m_page_frame_database = (byte*)PAGE_FRAME_BASE;
uint32 PAGE_FRAME_DB::m_page_frame_min = 0;
uint32 PAGE_FRAME_DB::m_page_frame_max = 0;
uint32 PAGE_FRAME_DB::m_next_free_page_frame = 0;

bool   PAGE_FRAME_DB::Init(uint32 page_frame_min, uint32 page_frame_max)
{
	m_page_frame_min = page_frame_min;
	m_page_frame_max = page_frame_max;
	m_next_free_page_frame = m_page_frame_min;

	uint32* page_dir = (uint32*)PAGE_DIR_BASE;
	uint32*  page_table = (uint32*)(m_page_frame_min++ * PAGE_SIZE);
	page_dir[PD_INDEX(PAGE_FRAME_BASE)] = (uint32)page_table | PT_PRESENT | PT_WRITABLE;
	//为page_frame_db分配4M物理内存
	uint32  page_frame_start = m_page_frame_min;
	m_page_frame_min += MB(4) >> 12;
	for (int i = 0; i < MB(4) >> 12; i++)
	{
		uint32 pa = (page_frame_start + i)* PAGE_SIZE;
		uint32 va = PAGE_FRAME_BASE + i * PAGE_SIZE;
		page_table[i] = pa | PT_PRESENT | PT_WRITABLE;
		memset((void*)va, 0, PAGE_SIZE);
	}
	return true;
}

uint32 PAGE_FRAME_DB::alloc_physical_page()
{
	for (uint32 i = m_next_free_page_frame; i < m_page_frame_max; i++)
	{
		if (m_page_frame_database[i] == PAGE_FRAME_FREE)
		{
			m_next_free_page_frame = i + 1;
			return i;
		}
	}
	return 0;
}

void   PAGE_FRAME_DB::free_physical_page(uint32 page)
{
	m_page_frame_database[page] = PAGE_FRAME_FREE;
	if (page < m_next_free_page_frame)
	{
		m_next_free_page_frame = page;
	}
}

uint32 PAGE_FRAME_DB::alloc_physical_pages(uint32 pages)
{
	for (uint32 i = m_next_free_page_frame; i < m_page_frame_max - pages; i++)
	{
		if (m_page_frame_database[i] == PAGE_FRAME_FREE)
		{
			uint32 j = i;
			for (j++; j < i + pages && j < m_page_frame_max; j++)
			{
				if (m_page_frame_database[j] != PAGE_FRAME_FREE) break;
			}
			if (j = i + pages)
			{
				m_next_free_page_frame = j;
				return i;
			}
		}
	}
	return 0;
}

void   PAGE_FRAME_DB::free_physical_pages(uint32  start_page, uint32 pages)
{
	for (uint32 i = start_page; i < start_page + pages; i++)
	{
		m_page_frame_database[i] = PAGE_FRAME_FREE;
	}
	if (m_next_free_page_frame > start_page)
	{
		m_next_free_page_frame = start_page;
	}
}

uint32 PAGE_FRAME_DB::map_pages(uint32 physical_addr, uint32 virtual_addr, int size, int protect)
{
	uint32* page_dir = (uint32*)PAGE_DIR_BASE;

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
			uint32  page_table_PA = alloc_physical_page();
			page_dir[pd_index] = page_table_PA | PT_PRESENT | PT_WRITABLE;
			memset(page_table_VA, 0, PAGE_SIZE);
		}
		page_table_VA[pt_index] = pa | protect;
	}
	return virtual_address;
}

void PAGE_FRAME_DB::unmap_pages(uint32 virtual_addr, int size)
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
			free_physical_page(page);
		}
	}
}