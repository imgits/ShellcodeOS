#pragma once
#include "typedef.h"

#define   PAGE_SIZE							0x1000
#define   PAGE_FRAME_INDEX(addr)			((addr)>>12)
#define   PD_INDEX(virtual_addr)			((virtual_addr)>>22)
#define   PT_INDEX(virtual_addr)			(((virtual_addr)>>12)&0x3FF)

#define   KERNEL_BASE						0x80000000 //~0xBFFFFFFF
#define   PAGE_TABLE_BASE					0xC0000000 //~0xC03FFFFF
#define   PAGE_FRAME_BASE					0xC0400000 //~0xC04FFFFF
#define   PAGE_LOW1M_BASE					0xC0500000 //~0xC05FFFFF
#define   PAGE_SYSTEM_BASE				0xC0600000 //~0xC06003FF

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

void*  init_page_frame_database(uint64 memsize);
void*  alloc_memory(uint32 virtual_address, uint32 size);

