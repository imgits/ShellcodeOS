#pragma once
#include "typedef.h"
#include "paging.h"

#define   PAGE_SIZE							0x1000
#define   PAGE_SIZE_BITS					12
#define   PD_INDEX(virtual_addr)			((virtual_addr>>22))
#define   PT_INDEX(virtual_addr)			((virtual_addr>>12)&0x3FF)

#define   KERNEL_BASE						0x80000000
#define   PAGE_TABLE_BASE					0xC0000000
#define   PAGE_DIR_BASE						0xC0300000 //=(0xC0000000 + (0xC0000000>>12))
#define   PT_BASE(virtual_addr)			    (PAGE_TABLE_BASE + PD_INDEX(virtual_addr)* PAGE_SIZE)
#define   PTE(virtual_addr)			        (PAGE_TABLE_BASE + (virtual_addr>>12))

#define   PAGE_FRAME_BASE					0xC0400000
#define   PAGE_LOW1M_BASE					0xC0500000
#define   PMODE_VIDEO_BASE					(PAGE_LOW1M_BASE + 0x000B8000)

#define   PAGE_FRAME_FREE					0
#define   PAGE_FRAME_LOW1M				1
#define   PAGE_FRAME_USER					2
#define   PAGE_FRAME_KERNEL				3
#define   PAGE_FRAME_ACPI					4
#define   PAGE_FRAME_NVS					5
#define   PAGE_FRAME_RESERVED				6
#define   PAGE_FRAME_NONE					0xff

#define KB(x)                    ((uint32)x<<10)
#define MB(x)                    ((uint32)x<<20)
#define GB(x)                    ((uint32)x<<30)

#define PAGES_TO_SIZE(pages)	 ((uint32)(pages)<<12)
#define SIZE_TO_PAGES(size)      (((uint32)(size) + PAGE_SIZE -1)>>12)

#define USER_SPACE(addr)       (((uint32)addr) < KERNEL_BASE)
#define KERNEL_SPACE(addr)     (((uint32)addr) >= KERNEL_BASE)

struct memory_info;

class PAGE_FRAME_DB
{
private:
	static byte*		m_page_frame_database;
	static bool      m_database_usable;
	static uint32	m_page_frame_min;
	static uint32	m_page_frame_max;
	static uint32	m_next_free_page_frame;
private:
	static uint32    map_mem_space(memory_info* meminfo);
public:
	static bool		Init(uint32 page_frame_used, memory_info* meminfo);
	static uint32	alloc_physical_page();
	static uint32	alloc_physical_pages(uint32 pages);
	static void		free_physical_page(uint32 page);
	static void		free_physical_pages(uint32  start_page, uint32 pages);

};
