#pragma once
#include "typedef.h"
#include <string.h>
#include <stdio.h>

#define MEMTYPE_RAM       1
#define MEMTYPE_RESERVED  2
#define MEMTYPE_ACPI      3
#define MEMTYPE_NVS       4

#define MAX_MEM_MAP		32

#pragma pack(push,1)
struct  regs_t
{
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
	unsigned short gs, fs, es, ds;
	unsigned int eflags;
};

#pragma pack(pop)

extern "C" void callbios(unsigned char intnum, regs_t *regs);
extern "C" void callbios_end(void);

class BIOS
{
public:
	BIOS()
	{
		uint32 callbios_size = (uint32)callbios_end - (uint32)callbios;
		printf("callbios=%08X callbios_end=%08X size=%08X(%d)\n", (uint32)callbios, (uint32)callbios_end, callbios_size,  callbios_size);
	}

	void putc(byte ch)
	{
		regs_t regs;
		memset(&regs, 0, sizeof(regs));
		regs.eax = 0x0E00 + ch;
		callbios(0x10, &regs);
	}

	void puts(char* str)
	{
		while (*str)
		{
			putc(*str++);
		}
	}

	//Detecting Memory (x86):http://wiki.osdev.org/Detecting_Memory_(x86)
	//Memory Map (x86):http://wiki.osdev.org/Memory_Map_(x86)
	//http://files.osdev.org/mirrors/geezer/osd/ram/index.htm#layout
	bool get_mem_info(memory_info& meminfo)
	{
		regs_t regs;
		memset(&regs, 0, sizeof(regs));
		memset(&meminfo, 0, sizeof(meminfo));
		regs.ebx = 0;
		for (int i = 0; i < MAX_MEM_MAP; i++)
		{
			regs.eax = 0x0E820;
			regs.ecx = 64;
			regs.edx = 0x534D4150; //'SMAP' ÖÐ¶ÏÒªÇó
			
			regs.edi = (uint32)&(meminfo.mem_maps[i]);
			regs.es  = regs.edi>>4;
			regs.edi &= 0x0f;

			callbios(0x15, &regs);
			if (regs.eflags & 0x00000001) //jc 
			{
				printf("call BOIS INT 15h failed\n");
				__asm jmp $
			}
			//printf("callbios INT 15h OK\n");
			if (regs.ebx == 0)
			{
				meminfo.map_count = i + 1;
				break;
			}
		}
		printf("meminfo.map_count=%d\n", meminfo.map_count);
		return true;
	}
};