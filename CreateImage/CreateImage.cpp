// CreateImage.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "VirtualDisk.h"
#include <stdint.h>

void exit_error(const char *fmt, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	int len = 0;
	len = vsprintf(buf, fmt, args);
	va_end(args);
	if (len > sizeof(buf)) len = sizeof(buf) - 1;
	buf[len] = 0;
	MessageBox(NULL,buf,NULL,IDOK);
	exit(GetLastError());
}

int load_file(const char* filename, void* buf, int bufsize)
{
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		exit_error("打开文件%s失败", filename);
	}
	fseek(fp, 0, SEEK_END);
	int filesize = ftell(fp);
	if (bufsize < filesize)
	{
		exit_error("读取文件%s缓冲区太小", filename);
	}
	rewind(fp);
	int len = fread(buf, 1, filesize, fp);
	if (len != filesize)
	{
		exit_error("读取文件%s失败", filename);
	}
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

void copy_file(char* fromfile, char* tofile)
{
	FILE* fpfrom = fopen(fromfile, "rb");
	FILE* fpto = fopen(tofile, "wb");
	if (fpfrom==NULL) exit_error("打开文件%s 失败\n", fromfile);
	if (fpto == NULL) exit_error("创建文件%s 失败\n", tofile);
	fseek(fpfrom, 0, SEEK_END);
	int fsize = ftell(fpfrom);
	rewind(fpfrom);
	char buf[0x10000];
	while(fsize>0)
	{
		int len = fread(buf, 1, sizeof(buf), fpfrom);
		if (len <= 0) break;
		if (fwrite(buf, 1, len, fpto) != len) break;
		fsize -= len;
	}
	if (fsize != 0) exit_error("复制文件%s ==> %s 失败\n", fromfile, tofile);
}
extern "C" void* dlmalloc(size_t bytes);

static void  __declspec(naked) INT_ENTRY()
{
	/*FA             */__asm      cli
	/*68 78 56 34 12 */__asm      push        12345678h//error_code 
	/*68 78 56 34 12 */__asm      push        12345678h//int_no
	/*68 78 56 34 12 */__asm      push        12345678h//int_handler_object
	/*68 78 56 34 12 */__asm	  push		  0//int_dispatch 
	/*C3             */__asm	  call		  [esp]
		               __asm      call        dlmalloc
		__asm      add    esp, 4+12
	__asm      sti
	__asm      iretd
}

static void  __declspec(naked) INT_DISPATCH()
{
	__asm cli
	__asm cld
	__asm push        12345678h//error_code 
	__asm push        12345678h//int_no
	__asm push        12345678h//int_handler_object
	__asm pushad
	__asm push ds
	__asm push es
	__asm push fs
	__asm push gs
	__asm mov  eax, 0x10
	__asm mov  ds, ax
	__asm mov  es, ax
	__asm mov  fs, ax
	__asm mov  gs, ax
	__asm push esp
	__asm mov  eax, INT_ENTRY
	__asm call eax;
	__asm add  esp, 4
	__asm pop  gs
	__asm pop  fs
	__asm pop  es
	__asm pop  ds
	__asm popad
	__asm add  esp, 12
	__asm sti
	__asm iretd
	//千万注意iret和iretd的区别，iret被编译成iretw
	//查找此错误杀死了无路脑细胞
}


//e:\shellcodeOS.vhd  bin\boot.bin bin\BootLdr.exe bin\OsLdr.exe L:\boot\OsLdr.exe bin\scOs.exe l:\os\ScOS.exe
int _tmain(int argc, _TCHAR* argv[])
{
	//INT_ENTRY();
	//INT_DISPATCH();
#define  PML4_BASE  0xFFFFF6FB7DBED000
#define  PDP_BASE   0xFFFFF6FB7DA00000
#define  PD_BASE    0xFFFFF6FB40000000
#define  PT_BASE    0xFFFFF68000000000
	uint64_t max_mem = 0x0000FFFFFFFFFFFF;
	uint64_t max_pages = (max_mem >> 12);
	uint64_t PT_END = PT_BASE + max_pages * 8;
	printf("PT_END=%0I64X\n", PT_END);

	dlmalloc(1024);
	VirtualDisk vhd;
	if (vhd.Open(argv[1]))
	{
		
		vhd.Detach();
		vhd.Close();
	}
	else exit_error("打开VHD文件失败");

	MBR  boot_mbr;
	MBR  vhd_mbr;
	int len;
	char boot_loader[1024 * 64];
	FILE* fvhd = fopen(argv[1], "rb+");
	if (fvhd == NULL)
	{
		exit_error("打开VHD文件失败");
	}
	if ((len = fread(&vhd_mbr, 1, 512, fvhd)) != 512)
	{
		exit_error("读VHD文件MBR失败");
	}
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
		Sleep(1000);
		copy_file(argv[4], argv[5]);
		copy_file(argv[6], argv[7]);
		return 0;
		BOOL ret = CopyFile(argv[4], argv[5], false);
		if (!ret) exit_error("复制文件%s失败", argv[4]);
		ret = CopyFile(argv[6], argv[7], false);
		if (!ret) exit_error("复制文件%s失败", argv[6]);
		return 0;
	}
	else exit_error("打开或挂载VHD文件失败");

	return 0;
}


