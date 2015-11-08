#include <windows.h>
#include "stdio.h"
#include "pe_file.h"

#define KERNEL_LOADER_BASE	0x20000
#define KERNEL_LOADER_SIZE	0x20000

char    os_loader_filename[256] = "\\boot\\osldr.exe";

void main(byte boot_drive)
{
	printf("\nBootLoader is starting...\n");
	
	PE_FILE os_loader;

	if (!os_loader.open(boot_drive, os_loader_filename))
	{
		printf("Open file %s failed\n", os_loader_filename);
		__asm jmp $
	}

	uint32 image_size = os_loader.image_size();
	printf("OsLoader image_size=%d\n", image_size);

	if (!os_loader.load((void*)KERNEL_LOADER_BASE, image_size))
	{
		printf("Load file %s failed\n", os_loader_filename);
		__asm jmp $
	}
	printf("Load file %s OK\n", os_loader_filename);

	typedef void (*osloader_main)(byte boot_drive);
	osloader_main osldr_main = (osloader_main)os_loader.entry_point();
	osldr_main(boot_drive);

	__asm jmp $
}

