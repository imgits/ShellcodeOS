#pragma once
#include "typedef.h"

#define   PAGE_SIZE							0x1000
#define   PD_INDEX(virtual_addr)			((virtual_addr>>22))
#define   PT_INDEX(virtual_addr)			((virtual_addr>>12)&0x3FF)

#define   KERNEL_BASE						0x80000000
#define   PAGE_TABLE_BASE					0xC0000000
#define   PAGE_FRAME_BASE					0xC0400000
#define   PAGE_LOW1M_BASE					0xC0500000
#define   PMODE_VIDEO_BASE				(PAGE_LOW1M_BASE + 0x000B8000)
#define   PAGE_FRAME_FREE					0
#define   PAGE_FRAME_LOW1M				1
#define   PAGE_FRAME_USER					2
#define   PAGE_FRAME_KERNEL				3
#define   PAGE_FRAME_NONE					0xff

#define		PT_PRESENT   0x001
#define		PT_WRITABLE  0x002
#define		PT_USER      0x004
#define		PT_ACCESSED  0x020
#define		PT_DIRTY     0x040

class PAGE_FRAME_DB
{
	static byte* g_page_frame_database;

	uint32	m_page_frame_min;
	uint32	m_page_frame_max;
	uint32	m_next_free_page_frame;
public:
	PAGE_FRAME_DB();
	~PAGE_FRAME_DB();
	bool		init(uint32 page_frame_min, uint32 page_frame_max);
	uint32	alloc_physical_page();
	void		free_physical_page(uint32 page);
	uint32	alloc_physical_pages(uint32 pages);
	void		free_physical_pages(uint32  start_page, uint32 pages);
};
