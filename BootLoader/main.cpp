#include <windows.h>
//#include <stdio.h>
#include "Fat32.h"
#if _DEBUG
#include <stdlib.h>
#include <string.h>
#else
#include "vga.h"
#include "acpi.h"
#endif

char    kernel_loader[256] = "\\boot\\osldr.exe";

void*   kernel_buf = (void*)0x100000;
void*   kernel_start = (void*)0x101000;
uint32  kernel_buf_size = 0x100000;
//memory_map
#pragma pack(push,1)
struct  memory_map
{
	uint64 BaseAddr;
	uint64 Length;
	uint32 Type;
};
#pragma pack(pop)

#define MEMTYPE_RAM       1
#define MEMTYPE_RESERVED  2
#define MEMTYPE_ACPI      3
#define MEMTYPE_NVS       4

int main(memory_map* mem_map, int32 count)
{
	printf("\nBootLoader is starting...\n");
	printf("memory map=%08X count=%08X:\n", mem_map, count);

	memory_map* pmap = mem_map;
	for (int i = 0; i < count; i++)
	{
		printf("%d %08X %08X %08X %d ", 
			i, 
			(uint32)pmap->BaseAddr, 
			(uint32)(pmap->BaseAddr + pmap->Length), 
			(uint32)pmap->Length, 
			pmap->Type);
		switch (pmap->Type)
		{
		case MEMTYPE_RAM: printf("RAM\n"); break;
		case MEMTYPE_RESERVED: printf("RESERVED\n"); break;
		case MEMTYPE_ACPI: printf("ACPI\n"); break;
		case MEMTYPE_NVS: printf("NVS\n"); break;
		default:	printf("\n"); break;
		}
		pmap++;
	}
	//读主引导扇区

	MBR mbr;
	read_sectors(&mbr, 0, 1);
	printf("read MBR OK\n");

	uint32 volume0_start_sector = mbr.partition_table[0].first_sector;
	printf("Volume[0] start sector=%d\n", volume0_start_sector);

	
	FAT32 fat32;

	fat32.Init(mbr.bootdrv, volume0_start_sector);

	FILE_OBJECT file;
	if (!fat32.open_file(&file, kernel_loader))
	{
		printf("Open \\boot\\osldr.exe failed\n");
		__asm jmp $
	}
	printf("Open \\boot\\osldr.exe OK\n");
	printf("start_cluster=%X, size=%d\n", file.start_cluster, file.size);
	
	kernel_buf_size = 0x100000;
#if _DEBUG
	kernel_buf = (void*)malloc(kernel_buf_size);
#else
	kernel_buf = (void*)0x100000;
	kernel_buf_size = 0x100000;
#endif
	if (fat32.load_file(&file, kernel_buf, kernel_buf_size) != file.size)
	{
		printf("Load Osloader failed\n");
		__asm jmp $
	}
	printf("Load Osloader OK\n");
	AcpiInit();
	PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER)kernel_buf;
	PIMAGE_NT_HEADERS pe_hdr = (PIMAGE_NT_HEADERS)((char*)kernel_buf + dos_hdr->e_lfanew);
	uint32 EntryPoint = pe_hdr->OptionalHeader.ImageBase + pe_hdr->OptionalHeader.AddressOfEntryPoint;
	__asm mov eax, EntryPoint
	__asm jmp eax
	return 0;
}

