// CreateImage.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "VirtualDisk.h"
#include <stdint.h>

int load_file(const char* filename, void* buf, int bufsize)
{
	FILE* fp = fopen(filename, "rb");
	fseek(fp, 0, SEEK_END);
	int filesize = ftell(fp);
	if (bufsize < filesize)
	{
		return -1;
	}
	rewind(fp);
	int len = fread(buf, 1, filesize, fp);
	fclose(fp);
	return len;
}

#pragma pack(push,1)
struct PTINFO
{
	uint8_t		active;			// 0x80 if partition active
	uint8_t		start_head;		// starting head
	uint8_t		start_sector;	// starting cylinder and sector (low byte)
	uint8_t		start_cylinder;	// starting cylinder and sector (high byte)
	uint8_t		type;			// type ID byte
	uint8_t		end_head;		// ending head
	uint8_t		end_sector;		// ending cylinder and sector (low byte)
	uint8_t		end_cylinder;	// ending cylinder and sector (high byte)
	uint32_t		first_sector;	// starting sector# (low byte)
	uint32_t		total_sectors;	// size of partition (low byte)
};

struct MBR
{
	char far_jmp_start[5];
	char bootdrv;
	uint32_t boot_loader_main;
	struct
	{
		byte			size;
		byte			zero;
		uint16_t		bootldr_sectors;
		uint16_t		bootldr_offset;
		uint16_t		bootldr_segment;
		uint64_t		bootldr_start_sector;
	}Disk_Address_Packet;
	char boot_code[0x1be - 5 - 1 -4 - 16];
	PTINFO partition_table[4];
	uint8_t sig_0x55;				// 0x55 signature byte
	uint8_t sig_0xaa;				// 0xaa signature byte
};

#pragma pack(pop)

void  __declspec(naked) INT_ENTRY()
{
	__asm cli
	__asm push 0x12345678
	__asm push 0x12345678
	__asm push 0x12345678
	__asm ret
}

//e:\shellcodeOS.vhd  bin\boot.bin bin\BootLdr.exe bin\OsLdr.exe l:\boot\OsLdr.exe bin\scOs.exe l:\os\ScOS.exe
int _tmain(int argc, _TCHAR* argv[])
{
	//INT_ENTRY();
#define  PML4_BASE  0xFFFFF6FB7DBED000
#define  PDP_BASE   0xFFFFF6FB7DA00000
#define  PD_BASE    0xFFFFF6FB40000000
#define  PT_BASE    0xFFFFF68000000000
	uint64_t max_mem = 0x0000FFFFFFFFFFFF;
	uint64_t max_pages = (max_mem >> 12);
	uint64_t PT_END = PT_BASE + max_pages * 8;
	printf("PT_END=%0I64X\n", PT_END);

	VirtualDisk vhd;
	if (vhd.Open(argv[1]))
	{
		vhd.Detach();
		vhd.Close();
	}


	MBR  boot_mbr;
	MBR  vhd_mbr;
	int len;
	char boot_loader[1024 * 64];
	try
	{
		FILE* fvhd = fopen(argv[1], "rb+");
		len = fread(&vhd_mbr, 1, 512, fvhd);
		int mbr_size = load_file(argv[2], &boot_mbr, sizeof(MBR));
		int bootldr_size = load_file(argv[3], boot_loader, sizeof(boot_loader));
		int bootldr_sectors = (bootldr_size + 511) / 512;
		boot_mbr.Disk_Address_Packet.bootldr_sectors = bootldr_sectors;
		memcpy(boot_mbr.partition_table, vhd_mbr.partition_table, 16 * 4);

		PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER)boot_loader;
		PIMAGE_NT_HEADERS pe_hdr = (PIMAGE_NT_HEADERS)(boot_loader + dos_hdr->e_lfanew);
		uint32_t EntryPoint = pe_hdr->OptionalHeader.ImageBase + pe_hdr->OptionalHeader.AddressOfEntryPoint;
		boot_mbr.boot_loader_main = EntryPoint;

		rewind(fvhd);
		len = fwrite(&boot_mbr, 1, 512, fvhd);
		len = fwrite(&boot_loader, 512, bootldr_sectors, fvhd);
		fclose(fvhd);

		char VolumeMountPoint[5] = "c:\\";
		VolumeMountPoint[0] = argv[5][0];
		if (vhd.Open(argv[1]) &&
			vhd.Attach())
		{
			Sleep(100);
			BOOL ret = CopyFile(argv[4], argv[5], false);
			if (ret)
			{
				ret = CopyFile(argv[6], argv[7], false);
				if (ret)
					return 0;
			}
		}
	}
	catch (...)
	{
		printf("Create image failed\n");
	}
	return 0;
}


