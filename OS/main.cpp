#include <Windows.h>
#include "typedef.h"
#include "ioport.h"
#include "vga.h"
#include "C++.h"
#include "page_frame.h"
#include "gdt.h"
#include "idt.h"
#include "pci.h"
#include "cpu.h"


PAGE_FRAME_DB page_frame_db;
CPU			  cpu;
GDT			  gdt;

void main(uint32 kernel_size, uint32 page_frame_min, uint32 page_frame_max)
{
	puts("\nHello world\n", 30);
	puts("Shellcode OS is starting...\n", 30);

	CppInit();
	page_frame_db.init(page_frame_min, page_frame_max);
	gdt.Init();

	__asm jmp $
}

