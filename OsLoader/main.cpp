#include <Windows.h>
#include <stdio.h>
#include "typedef.h"
#include "vga.h"
#include "mmu.h"
#include "kernel_file.h"

char    os_kernel_filename[256] = "\\os\\scos.exe";

void	main(byte boot_drive)
{
	puts("Hello world\n", 10);
	puts("OsLoader.exe is starting...\n", 10);

	MMU mmu;
	mmu.init(MB(1)>>12);
	mmu.map_low_1M_memory();
	mmu.startup_page_mode();//进入分页模式

	KERNEL_FILE kernel_file;
	kernel_file.open(boot_drive, os_kernel_filename);
	uint32 kernel_image_size = kernel_file.image_size();
	char * kernel_buf = (char*)mmu.alloc_memory(KERNEL_BASE,kernel_image_size);
	kernel_file.load(kernel_buf, kernel_image_size);
	printf("next_free_page_frame=%08X\n", mmu.next_free_page_frame());
	//panic("");
	kernel_main os_main = kernel_file.entry_point();
	os_main(kernel_image_size);

	__asm jmp $
}

