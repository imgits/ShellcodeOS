#include <Windows.h>
#include <stdio.h>
#include "typedef.h"
//#include "ioport.h"
#include "vga.h"
//#include "C++.h"
#include "mmu.h"
//#include "fat32.h"
//#include "pe.h"
//#include "acpi.h"
#include "kernel_file.h"
#include "bios.h"


char    os_kernel_filename[256] = "\\os\\scos.exe";

#define KERNEL_BASE	0x80000000
//extern uint32	g_page_frame_min;
//extern uint32	g_page_frame_max;
/*
uint32 load_kernel(byte boot_driver)
{
	//读主引导扇区
	MBR mbr;
	read_sectors(&mbr, 0, 1);
	uint32 volume0_start_sector = mbr.partition_table[0].first_sector;

	FAT32 fat32;
	fat32.Init(boot_driver, volume0_start_sector);

	FILE_OBJECT file;
	if (!fat32.open_file(&file, os_kernel))
	{
		printf("Open %s failed\n", os_kernel);
		__asm jmp $
	}
	printf("Open %s OK\n", os_kernel);
	
	//加载PE头，主要是为了获取SizeOfImage，以便分配足够的内存空间
	//因为file.size != SizeOfImage (可能)
	//http://blog.csdn.net/wbcuc/article/details/6225344
	//编译时添加选项/Gs32768，避免调用__chkstk
	char pe_header[4096];
	if (fat32.read_file(&file, pe_header, 4096) != 4096)
	{
		printf("Read OS image header failed\n");
		__asm jmp $
	}

	PE pe(pe_header);
	uint32 kernel_image_size = pe.ImageSize();
	
	printf("start_cluster=%X, file_size=%d image_size=%d\n", file.start_cluster, file.size, kernel_image_size);

	//分配内核加载空间
	char* os_kernel_buf = (char*)alloc_memory(OS_KERNEL_BASE, kernel_image_size);

	if (fat32.load_file(&file, os_kernel_buf, file.size) != file.size)
	{
		printf("Load OS failed\n");
		__asm jmp $
	}
	//将未初始化内存清零
	memset(os_kernel_buf + file.size, 0, kernel_image_size - file.size);
	return kernel_image_size;
}
*/

memory_info meminfo;

void	main(byte boot_drive, uint64 mem_size)
{
	printf("\nmemsize=%llX\n", mem_size);
	puts("Hello world\n", 10);
	puts("OsLoader.exe is starting...\n", 10);

	BIOS bios;
	//bios.get_mem_info(meminfo);

	KERNEL_FILE kernel_file;
	kernel_file.open(boot_drive, os_kernel_filename);
	uint32 kernel_image_size = kernel_file.image_size();

	MMU mmu;
	mmu.init(MB(1)>>12);
	mmu.map_low_1M_memory();
	
	mmu.startup_page_mode();//进入分页模式
	//__asm jmp $
	//bios.puts("call BIOS after mmu.startup_page_mode\n");

	char * kernel_buf = (char*)mmu.alloc_memory(KERNEL_BASE,kernel_image_size);
	
	kernel_file.load(kernel_buf, kernel_image_size);
	
	bios.get_mem_info(meminfo);
	uint64 memsize = 0;
	for (int i = 0; i < meminfo.map_count; i++)
	{
		uint64 length = meminfo.mem_maps[i].length;
		uint64 begin = meminfo.mem_maps[i].base_addr;
		uint64 end = begin + length;
		//printf("%d %08X-%08X %08X-%08X %08X-%08X %d ",
		//	i,
		//	begin >> 32, begin & 0xffffffff,
		//	end   >> 32, end   & 0xffffffff,
		//	length>> 32, length & 0xffffffff,
		//	meminfo.mem_maps[i].type);

		printf("%d %016llX %016llX %016llX %d ",
			i,
			begin,
			end,
			length,
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
	kernel_main os_main = kernel_file.entry_point();
	os_main(kernel_image_size, mmu.next_free_page_frame());

	__asm jmp $
}

