#include <windows.h>
#include "stdio.h"
#include "Fat32.h"
#include "vga.h"
#include "pe.h"

char    kernel_loader[256] = "\\boot\\osldr.exe";

#define KERNEL_LOADER_BASE	0x20000
#define KERNEL_LOADER_SIZE	0x20000

uint32 load_os_loader(byte driver)
{
	//读主引导扇区
	void*   loader_buf = (void*)KERNEL_LOADER_BASE;
	uint32  loader_buf_size = KERNEL_LOADER_SIZE;

	MBR mbr;
	read_sectors(&mbr, 0, 1);
	printf("read MBR OK\n");

	uint32 volume0_start_sector = mbr.partition_table[0].first_sector;
	printf("Volume[0] start sector=%d\n", volume0_start_sector);

	FAT32 fat32;

	fat32.Init(driver, volume0_start_sector);

	FILE_OBJECT file;
	if (!fat32.open_file(&file, kernel_loader))
	{
		printf("Open \\boot\\osldr.exe failed\n");
		__asm jmp $
	}
	printf("Open \\boot\\osldr.exe OK\n");
	printf("start_cluster=%X, size=%d\n", file.start_cluster, file.size);

	if (fat32.load_file(&file, loader_buf, loader_buf_size) != file.size)
	{
		printf("Load Osloader failed\n");
		__asm jmp $
	}
	printf("Load Osloader OK\n");

	return file.size;
}

void main(byte boot_drive)
{
	printf("\nBootLoader is starting...\n");
	
	uint32 file_size = load_os_loader(boot_drive);

	typedef void (*osloader_main)(byte boot_drive);

	PE pe((void*)KERNEL_LOADER_BASE);
	uint32 image_size = pe.ImageSize();
	memset((char*)KERNEL_LOADER_BASE + file_size, 0, image_size - file_size);

	printf("OsLoader filesize = %d imagesize=%d\n", file_size, image_size);

	osloader_main osldr_main = (osloader_main)pe.EntryPoint();
	osldr_main(boot_drive);

	__asm jmp $
}

