// MultibootPE.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <Windows.h>
#include <stdint.h>

#pragma pack(push,1)
struct MULTIBOOT_HEADER
{
	uint32_t magic;
	uint32_t flags;
	uint32_t checksum;
	uint32_t header_addr;
	uint32_t load_addr;
	uint32_t load_end_addr;
	uint32_t bss_end_addr;
	uint32_t entry_addr;
	uint32_t mode_type;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
};
#pragma pack(pop)

int help()
{
	printf("MultibootPE filename\n");
	return 0;
}

int main(int argc, char** argv)
{
	if (argc != 2) return help();
	FILE* fp = fopen(argv[1],"rb+");
	if (fp == NULL)
	{
		printf("Open file %s failed\n", argv[1]);
		return help();
	}
	char file_buf[4096];
	int size = fread(file_buf, 1, sizeof(file_buf), fp);
	if (size != sizeof(file_buf) || *((UINT16*)file_buf)!= 'ZM')
	{
		printf("Bad size OR type for file %s\n", argv[1]);
		return help();
	}
	PIMAGE_DOS_HEADER doshdr = (PIMAGE_DOS_HEADER)file_buf;
	PIMAGE_NT_HEADERS nthdr = (PIMAGE_NT_HEADERS)(file_buf + doshdr->e_lfanew);
	UINT32 pe_base = nthdr->OptionalHeader.ImageBase;
	UINT32 pe_size = nthdr->OptionalHeader.SizeOfImage;
	UINT32 pe_entry = pe_base + nthdr->OptionalHeader.AddressOfEntryPoint;
	MULTIBOOT_HEADER* multiboot = (MULTIBOOT_HEADER*)(file_buf + sizeof(IMAGE_DOS_HEADER));
	memset(multiboot, 0, sizeof(MULTIBOOT_HEADER));
	multiboot->magic = 0x1BADB002;
	multiboot->flags = 0x00010003;
	multiboot->checksum = 0 - (multiboot->magic + multiboot->flags);
	multiboot->header_addr = (UINT32)(pe_base + sizeof(IMAGE_DOS_HEADER));
	multiboot->load_addr = pe_base;
	multiboot->load_end_addr = pe_base + pe_size;
	multiboot->bss_end_addr = 0;
	multiboot->entry_addr = pe_entry;
	fseek(fp, 0, SEEK_SET);
	size = fwrite(file_buf, 1, sizeof(file_buf), fp);
	if (size != sizeof(file_buf))
	{
		printf("Write multiboot info failed");
		fclose(fp);
		return help();
	}
	printf("PE info:\n");
	printf("  ImageBase:  %08X\n", pe_base);
	printf("  ImageSize:  %08X\n", pe_size);
	printf("  PeHeader:   %08X\n", doshdr->e_lfanew);
	printf("  EntryPoint: %08X\n", pe_entry);

	printf("\nmultiboot info %08X--%08X:\n", sizeof(IMAGE_DOS_HEADER), sizeof(IMAGE_DOS_HEADER)+ sizeof(MULTIBOOT_HEADER)-1);
	printf("  magic:         %08X\n", multiboot->magic);
	printf("  flags:         %08X\n", multiboot->flags);
	printf("  checksum:      %08X\n", multiboot->checksum);
	printf("  header_addr:   %08X\n", multiboot->header_addr);
	printf("  load_addr:     %08X\n", multiboot->load_addr);
	printf("  load_end_addr: %08X\n", multiboot->load_end_addr);
	printf("  bss_end_addr:  %08X\n", multiboot->bss_end_addr);
	printf("  entry_addr:    %08X\n", multiboot->entry_addr);
	fclose(fp);
	return 0;
}

