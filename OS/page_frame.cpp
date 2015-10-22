#include "page_frame.h"
#include <string.h>

byte*	PAGE_FRAME_DB::m_page_frame_database = (byte*)PAGE_FRAME_BASE;
uint32	PAGE_FRAME_DB::m_page_frame_min=0;
uint32	PAGE_FRAME_DB::m_page_frame_max=0;
uint32	PAGE_FRAME_DB::m_next_free_page_frame=0;


bool   PAGE_FRAME_DB::Init(uint32 page_frame_min, uint32 page_frame_max)
{
	m_page_frame_min = page_frame_min;
	m_page_frame_max = page_frame_max;
	m_next_free_page_frame = m_page_frame_min;
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

