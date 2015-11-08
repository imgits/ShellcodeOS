#include <Windows.h>
#include "stdio.h"
#include "intrin.h"
#include "typedef.h"
#include "ioport.h"
#include "vga.h"
#include "C++.h"
#include "page_frame.h"
#include "gdt.h"
#include "idt.h"
#include "pci.h"
#include "cpu.h"
#include "tss.h"
#include "mmu.h"
#include "process.h"
#include "trap.h"
#include "8253.h"
#include "8259.h"
#include "keyboard.h"
#include "rtc.h"
#include "bios.h"
#include "system.h"
#include "liballoc.h"
#include "paging.h"

CPU			  cpu;

uint32 get_mem_info(memory_info& meminfo)
{
	BIOS bios;
	bios.get_mem_info(meminfo);
	uint64 memsize = 0;
	for (int i = 0; i < meminfo.map_count; i++)
	{
		uint64 length = meminfo.mem_maps[i].length;
		uint64 begin = meminfo.mem_maps[i].base_addr;
		uint64 end = begin + length;
		printf("%d %08X-%08X %08X-%08X %08X-%08X %d ",
			i,
			(uint32)(begin >> 32), (uint32)(begin & 0xffffffff),
			(uint32)(end >> 32), (uint32)(end & 0xffffffff),
			(uint32)(length >> 32), (uint32)(length & 0xffffffff),
			meminfo.mem_maps[i].type);
		switch (meminfo.mem_maps[i].type)
		{
		case MEMTYPE_RAM:
			printf("RAM\n");
			if (memsize < end && end < 0x100000000I64)
			{
				memsize = end;
			}
			break;
		case MEMTYPE_RESERVED: printf("RESERVED\n"); break;
		case MEMTYPE_ACPI: printf("ACPI\n"); break;
		case MEMTYPE_NVS: printf("NVS\n"); break;
		default:	printf("\n"); break;
		}
	}
	return memsize;
}
#include "list.h"

void main(uint32 kernel_image_size)
{
	puts("\nHello world\n", 30);
	puts("Shellcode OS is starting...\n", 30);

	CppInit();
	//callbios 要求内存地址位于1M一下，
	//因此，此处从栈(esp<0x00090000)中分配meminfo
	PAGER::Init(kernel_image_size);
	panic("");
	memory_info meminfo;
	uint32 memsize = get_mem_info(meminfo);

	printf("memsize=%08X(%dMb)\n", memsize, memsize >> 20);

	System.Init(kernel_image_size, &meminfo);

	LIST<PROCESS>* process_list = new LIST<PROCESS>();
	PROCESS* proc = new PROCESS();
	printf("process_list=%08X process=%08X\n", process_list, (uint32)proc);
	process_list->insert_head(proc);
	process_list->remove_tail();
	delete process_list;
	delete proc;

	_enable();
	panic("");
	
	_enable();
	__asm jmp $
}

