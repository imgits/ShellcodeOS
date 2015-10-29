#include "page_frame.h"
#include <string.h>
#include "bios.h"

byte*  PAGE_FRAME_DB::m_page_frame_database = (byte*)PAGE_FRAME_BASE;
bool   PAGE_FRAME_DB::m_database_usable = false;
uint32 PAGE_FRAME_DB::m_page_frame_min = 0;
uint32 PAGE_FRAME_DB::m_page_frame_max = 0;
uint32 PAGE_FRAME_DB::m_next_free_page_frame = 0;

bool   PAGE_FRAME_DB::Init(uint32 page_frame_used, memory_info* meminfo)
{
	m_page_frame_min = SIZE_TO_PAGES(MB(1)) + page_frame_used;
	m_page_frame_max = 0;
	m_database_usable = false;

	//为page_frame_db分配1M物理内存,映射到0xC00400000-0xC004FFFFF
	uint32  database_PA = alloc_physical_pages(SIZE_TO_PAGES(MB(1)));
	PAGER::map_pages(database_PA, PAGE_FRAME_BASE, MB(1));
	memset((void*)PAGE_FRAME_BASE, PAGE_FRAME_NONE, MB(1));

	//将低端1M物理内存映射0xC00500000-0xC005FFFFF
	PAGER::map_pages(0, PAGE_LOW1M_BASE, MB(1));

	m_page_frame_max = map_mem_space(meminfo);
	m_next_free_page_frame = m_page_frame_min;

	m_database_usable = true;

	return true;
}

uint32   PAGE_FRAME_DB::map_mem_space(memory_info* meminfo)
{
	uint64 memsize = 0;
	for (int i = 0; i < meminfo->map_count;i++)
	{
		uint64 length = meminfo->mem_maps[i].length;
		uint64 begin = meminfo->mem_maps[i].base_addr;
		uint64 end = begin + length;
		if (end < MB(1) || begin >= 0x100000000I64) continue;
		switch (meminfo->mem_maps[i].type)
		{
		case MEMTYPE_RAM:
			memsize = end;
			for (int j = SIZE_TO_PAGES(begin); j < SIZE_TO_PAGES(end); j++)
			{
				m_page_frame_database[j] = PAGE_FRAME_FREE;
			}
			break;
		case MEMTYPE_RESERVED: 
			for (int j = SIZE_TO_PAGES(begin); j < SIZE_TO_PAGES(end); j++)
			{
				m_page_frame_database[j] = PAGE_FRAME_RESERVED;
			}
			break;
		case MEMTYPE_ACPI: 
			for (int j = SIZE_TO_PAGES(begin); j < SIZE_TO_PAGES(end); j++)
			{
				m_page_frame_database[j] = PAGE_FRAME_ACPI;
			}
			PAGER::map_pages(begin, begin, end - begin, PT_PRESENT | PT_READONLY);
			break;
		case MEMTYPE_NVS: 
			for (int j = SIZE_TO_PAGES(begin); j < SIZE_TO_PAGES(end); j++)
			{
				m_page_frame_database[j] = PAGE_FRAME_NVS;
			}
			PAGER::map_pages(begin, begin, end - begin, PT_PRESENT | PT_READONLY);
			break;
		default:	
			break;
		}
	}
	return SIZE_TO_PAGES(memsize);
}

uint32 PAGE_FRAME_DB::alloc_physical_page()
{
	if (!m_database_usable)
	{
		uint32 addr = m_page_frame_min * PAGE_SIZE;
		m_page_frame_min++;
		return addr;
	}

	for (uint32 i = m_next_free_page_frame; i < m_page_frame_max; i++)
	{
		if (m_page_frame_database[i] == PAGE_FRAME_FREE)
		{
			m_next_free_page_frame = i + 1;
			return i*PAGE_SIZE;
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
	if (!m_database_usable)
	{
		uint32 addr = m_page_frame_min * PAGE_SIZE;
		m_page_frame_min+=pages;
		return addr;
	}

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
				return i*PAGE_SIZE;
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

