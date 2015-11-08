#include <Windows.h>
#include <stdio.h>
#include "typedef.h"
#include "vga.h"
#include "mmu.h"
#include "pe_file.h"

char    os_kernel_filename[256] = "\\os\\scos.exe";

void	main(byte boot_drive)
{
	puts("Hello world\n", 10);
	puts("OsLoader.exe is starting...\n", 10);

	//初始化页目录、页表并进入分页模式
	startup_page_mode();

	PE_FILE kernel_file;
	kernel_file.open(boot_drive, os_kernel_filename);
	uint32 kernel_image_size = kernel_file.image_size();
	char * kernel_buf = (char*)KERNEL_BASE;
	kernel_file.load(kernel_buf, kernel_image_size);

	printf("Load OS OK\n");

	typedef void(*kernel_main)(uint32 kernel_image_size);
	kernel_main os_main = (kernel_main)kernel_file.entry_point();
	os_main(kernel_image_size);

	__asm jmp $
}

