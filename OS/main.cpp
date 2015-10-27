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
#include "acpi.h"
#include "trap.h"
#include "8253.h"
#include "8259.h"
#include "keyboard.h"
#include "rtc.h"
#include "bios.h"

PAGE_FRAME_DB  page_frame_db;
CPU			  cpu;
GDT			  gdt;
IDT			  idt;
TSS           tss;
MMU			  mmu;
TRAP			  trap;

void main(uint32 kernel_image_size, uint32 next_free_page_frame)
{
	puts("\nHello world\n", 30);
	puts("Shellcode OS is starting...\n", 30);

	CppInit();

	BIOS bios;
	memory_info meminfo;
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
			(uint32)(end   >> 32), (uint32)(end   & 0xffffffff),
			(uint32)(length>> 32), (uint32)(length & 0xffffffff),
			meminfo.mem_maps[i].type);

		switch (meminfo.mem_maps[i].type)
		{
		case MEMTYPE_RAM:
			printf("RAM\n");
			if (memsize < end)
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

	//PAGE_FRAME_DB::Init(next_free_page_frame, 0xfffff);
	
	//mmu.Init();

	//ACPI acpi;
	//acpi.Init(&mmu);

	uint32 CR3 = __readcr3();
	printf("PageDir=%08X\n", CR3);

	gdt.Init();
	idt.Init();
	tss.Init(CR3,&gdt);
	trap.Init(&idt);
	PIC::Init(&idt);
	PIT::Init();
	RTC::Init();
	//Keyboard::Init();
	_enable();
	__asm jmp $
}

