#include <windows.h>
#include "stdio.h"
#include "Fat32.h"
#include "vga.h"
#include "pe.h"

char    kernel_loader[256] = "\\boot\\osldr.exe";

#define KERNEL_LOADER_BASE	0x20000

//memory_map
#pragma pack(push,1)
struct  memory_map
{
	uint64 BaseAddr;
	uint64 Length;
	uint32 Type;
};

struct memory_info
{
	uint32	  map_count;
	memory_map mem_maps[1];
};

//http://www.ctyme.com/intr/rb-0715.htm
struct disk_params
{
	uint16 size;	//00h    WORD(call) size of buffer
				//(001Ah for v1.x, 001Eh for v2.x, 42h for v3.0)
				//(ret) size of returned data
	uint16 flags;//	02h    WORD    information flags(see #00274)
	/*
	Bit(s)  Description     (Table 00274)
	0      DMA boundary errors handled transparently
	1      cylinder/head/sectors-per-track information is valid
	2      removable drive
	3      write with verify supported
	4      drive has change-line support (required if drive >= 80h is removable)
	5      drive can be locked (required if drive >= 80h is removable)
	6      CHS information set to maximum supported values, not current media
	15-7   reserved (0)
	*/
	uint32 cylinders;//04h    DWORD   number of physical cylinders on drive
	uint32 heads;//08h    DWORD   number of physical heads on drive
	uint32 sectors_per_track;//0Ch    DWORD   number of physical sectors per track
	uint64 total_sectors;//10h    QWORD   total number of sectors on drive
	uint16 bytes_per_sector;//	18h    WORD    bytes per sector
	//--- v2.0+ ---
	uint32 config_params;	//	1Ah    DWORD->EDD configuration parameters(see #00278)
						 //FFFFh:FFFFh if not available
	//--- v3.0 ---
	uint16 signature;//1Eh    WORD    signature BEDDh to indicate presence of Device Path info
	byte   length;//20h    BYTE    length of Device Path information, including signature and this byte(24h for v3.0)
	byte   reserved1[3];//		21h  3 BYTEs   reserved(0)
	byte   bus_name[4];//		24h  4 BYTEs   ASCIZ name of host bus("ISA" or "PCI")
	byte   interface_name[8];//		28h  8 BYTEs   ASCIZ name of interface type
			//"ATA"	"ATAPI"	"SCSI"	"USB""1394" IEEE 1394 (FireWire) 	"FIBRE" Fibre Channel
	byte   interface_path[8];//30h  8 BYTEs   Interface Path(see #00275)
	byte	   device_path[8];//		38h  8 BYTEs   Device Path(see #00276)
	byte   reserved2;//		40h    BYTE    reserved(0)
	byte   checksum;//41h    BYTE    checksum of bytes 1Eh - 40h
	//(two's complement of sum, which makes the 8 - bit sum of bytes 1Eh - 41h equal 00h)
};

struct disk_info
{
	byte driver;
	byte type;
	uint16 cylinders;
	byte heads;
	byte sectors;
	//uint16 params_seg;
	//uint16 params_offset;
};

#pragma pack(pop)

#define MEMTYPE_RAM       1
#define MEMTYPE_RESERVED  2
#define MEMTYPE_ACPI      3
#define MEMTYPE_NVS       4

extern "C" void bios_print_string(char *str);
extern "C" int bios_get_drive_params(int drive, int *cyls, int *heads, int *sects);
extern "C" int bios_read_disk(int drive, int cyl, int head, int sect, int nsect, void *buffer);
extern "C" int vesa_get_info(struct vesa_info *info);
extern "C" int vesa_get_mode_info(int mode, struct vesa_mode_info *info);
extern "C" int vesa_set_mode(int mode);


uint32 load_os_loader()
{
	//读主引导扇区
	void*   loader_buf = (void*)KERNEL_LOADER_BASE;
	uint32  loader_buf_size = 0x20000;

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

	if (fat32.load_file(&file, loader_buf, loader_buf_size) != file.size)
	{
		printf("Load Osloader failed\n");
		__asm jmp $
	}
	printf("Load Osloader OK\n");

	return file.size;
}

struct  regs16_t
{
	unsigned short di, si, bp, sp, bx, dx, cx, ax;
	unsigned short gs, fs, es, ds, eflags;
};

extern "C" int callbios(unsigned char intnum, regs16_t *regs);


void main(disk_info* disk, memory_info* mem_info)
{
	printf("\nBootLoader is starting...\n");
	printf("disk driver=%d,type=%d, cylinders=%d, heads=%d, sectors=%d\n", 
		disk->driver, disk->type, disk->cylinders, disk->heads, disk->sectors);
	
	//disk_params* params = (disk_params*)((((uint32)disk->params_seg)<<4) + disk->params_offset);
	//printf("disk_param=%X seg=%X offset=%X size = %d\n", params, disk->params_seg, disk->params_offset, params->size);

	uint64 memsize = 0;
	uint32 mem_map_count = mem_info->map_count;
	memory_map* mem_maps= mem_info->mem_maps;
	printf("memory map=%08X count=%08X:\n", mem_maps, mem_map_count);
	for (int i = 0; i < mem_map_count; i++)
	{
		printf("%d %08X %08X %08X %d ", 
			i, 
			(uint32)mem_maps->BaseAddr,
			(uint32)(mem_maps->BaseAddr + mem_maps->Length),
			(uint32)mem_maps->Length,
			mem_maps->Type);
		switch (mem_maps->Type)
		{
		case MEMTYPE_RAM: 
			printf("RAM\n"); 
			if (memsize < (uint64)(mem_maps->BaseAddr + mem_maps->Length))
			{
				memsize = (uint64)(mem_maps->BaseAddr + mem_maps->Length);
			}
			break;
		case MEMTYPE_RESERVED: printf("RESERVED\n"); break;
		case MEMTYPE_ACPI: printf("ACPI\n"); break;
		case MEMTYPE_NVS: printf("NVS\n"); break;
		default:	printf("\n"); break;
		}
		mem_maps++;
	}
	printf("memsize=%llX\n", memsize);
	
	load_os_loader();

	typedef void (*osloader_main)(uint64 memsize);

	PE pe((void*)KERNEL_LOADER_BASE);
	osloader_main osldr_main = (osloader_main)pe.EntryPoint();
	osldr_main(memsize);

	__asm jmp $
}

