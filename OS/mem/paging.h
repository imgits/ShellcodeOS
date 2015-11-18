#pragma once
#include "typedef.h"

#define   PAGE_SIZE						0x1000
#define   PAGE_SIZE_BITS				12

#define   PDE_INDEX(virtual_addr)			((virtual_addr>>22))
#define   PTE_INDEX(virtual_addr)			((virtual_addr>>12)&0x3FF)

#define   PDE_ADDRESS(pde)					((pde)&0xFFFFF000)
#define   PDE_ATTR(pde)						((pde)&0x00000FFF)

#define   PTE_ADDRESS(pte)					((pte)&0xFFFFF000)
#define   PTE_ATTR(pte)						((pte)&0x00000FFF)


#define   KERNEL_BASE						0x80000000
#define   PAGE_TABLE_BASE					0xC0000000
#define   PAGE_DIR_BASE						0xC0300000 //=(0xC0000000 + (0xC0000000>>12))
#define   PT_BASE(virtual_addr)				(PAGE_TABLE_BASE + PD_INDEX(virtual_addr)* PAGE_SIZE)
#define   PTE(virtual_addr)					(PAGE_TABLE_BASE + (virtual_addr>>12))

#define     GET_PDE(addr)  ((uint32*)PAGE_DIR_BASE)[(uint32)(addr)>>22]
#define     GET_PTE(addr)  ((uint32*)PAGE_TABLE_BASE)[(uint32)(addr)>>12]
#define     GET_PAGE_TABLE(addr)  (PAGE_TABLE_BASE + ((uint32)(addr)>>22)* PAGE_SIZE)

#define     SET_PDE(addr, val)  ((uint32*)PAGE_DIR_BASE)[(uint32)(addr)>>22] = val;
#define     SET_PTE(addr, val)  {((uint32*)PAGE_TABLE_BASE)[(uint32)(addr)>>12]=val; __asm  invlpg addr } 


#define     MAP_PAGE(page_pa,page_va) { ((uint32*)PAGE_TABLE_BASE)[(uint32)(page_va)>>12] = page_pa | PT_PRESENT | PT_WRITABLE; __asm  invlpg page_va } 
#define     MAP_PAGE_TABLE(page_dir, pt_pa,pt_va) ((uint32*)page_dir)[((uint32)(pt_va)>>12)&0x3FF] = pt_pa | PT_PRESENT | PT_WRITABLE;

#define     PAGE_ALGINED(addr)		((((uint32)addr) & 0x00000FFF) ==0)  	
#define     CHECK_PAGE_ALGINED(addr)  if ((((uint32)addr) & 0x00000FFF) !=0) panic("CHECK_PAGE_ALGINED(%08X)",addr); 	

#define		PT_PRESENT   0x001

#define		PT_WRITABLE  0x002
#define		PT_READONLY  0x000

#define		PT_USER      0x004
#define		PT_KERNEL    0x000

#define		PT_ACCESSED  0x020
#define		PT_DIRTY     0x040


#define   PAGE_FRAME_FREE					0
#define   PAGE_FRAME_LOW1M					1
#define   PAGE_FRAME_USER					2
#define   PAGE_FRAME_KERNEL					3
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

#define MAX_MEM_MAP		32

struct  memory_map
{
	uint64 base_addr;
	uint64 length;
	uint32 type;
};
struct memory_info
{
	uint32	   map_count;
	memory_map mem_maps[MAX_MEM_MAP];
};

class PAGER
{
private:
	uint32 m_cr3;
	static byte*	m_page_frame_database;
	static bool     m_database_usable;
	static uint32	m_page_frame_min;
	static uint32	m_page_frame_max;
	static uint32	m_next_free_page_frame;
	static uint32   m_kernel_image_size;
	static uint32   m_ram_size;
	static memory_info m_meminfo;
public:
	PAGER();
	static bool		Init(uint32 kernel_image_size);
	static uint32	alloc_physical_page();
	static uint32	alloc_physical_pages(uint32 pages);
	static void		free_physical_page(uint32 page);
	static void		free_physical_pages(uint32  start_page, uint32 pages);

	uint32* new_page_dir();
	static uint32 new_page_table(uint32 virtual_address);
	static uint32 map_pages(uint32 physical_address, uint32 virtual_address, int size, int protect = (PT_PRESENT | PT_WRITABLE) );
	static void   unmap_pages(uint32 virtual_address, int size);

	static uint32 new_page_table(uint32* page_dir, uint32 virtual_address);
	static uint32 map_pages(uint32* page_dir, uint32 physical_address, uint32 virtual_address, int size, int protect = (PT_PRESENT | PT_WRITABLE));

private:
	static uint32   get_mem_info();
	static void		create_page_frame_db();
	static void		rebuild_os_page_table();
};

	

