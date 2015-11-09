#pragma once
#include "typedef.h"


#define panic(fmt,...) { printf("%s::%s() line %d:"#fmt,__FILE__,__FUNCTION__,__LINE__, __VA_ARGS__); __asm jmp $}

#define   KERNEL_START_PA					0x00100000

#define   KERNEL_BASE						0x80000000 //~0xBFFFFFFF
#define   PAGE_TABLE_BASE					0xC0000000 //~0xC03FFFFF
#define   PAGE_DIR_BASE						0xC0300000 // (0xC0000000 + (0xC0000000>>12))
#define   PAGE_FRAME_BASE					0xC0400000 //~0xC04FFFFF
#define   PAGE_LOW1M_BASE					0xC0500000 //~0xC05FFFFF

#define   PAGE_DIR_PHYSICAL_ADDR			0x00040000
#define   PAGE_TABLE_0x00000000_PHYS_ADDR	0x00041000
#define   PAGE_TABLE_0x80000000_PHYS_ADDR	0x00042000




