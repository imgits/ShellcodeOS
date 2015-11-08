#include "pe_file.h"
#include "stdio.h"
#include <Windows.h>

PE_FILE::PE_FILE()
{
	m_drive = 0;
	m_volume_start_sector = 0;
	m_image_base = NULL;
}

bool PE_FILE::open(byte drive, char* filename)
{
	m_drive = drive;
	MBR mbr;
	read_sectors(&mbr, 0, 1);
	int active_part = 0;
	for (int i = 0; i < 4; i++)
	{
		if (mbr.partition_table[i].active == 0x80)
		{
			active_part = i;
			break;
		}
	}
	m_volume_start_sector = mbr.partition_table[active_part].first_sector;

	m_fat32.Init(drive, m_volume_start_sector);

	if (!m_fat32.open_file(&m_file, filename))
	{
		//printf("Open %s failed\n", filename);
		//__asm jmp $
		return false;
	}

	//加载PE头，主要是为了获取SizeOfImage，以便分配足够的内存空间
	//http://blog.csdn.net/wbcuc/article/details/6225344
	//编译时添加选项/Gs32768，避免调用__chkstk
	if (m_fat32.read_file(&m_file, m_header_buf, PE_HEAD_BUF_SIZE) != PE_HEAD_BUF_SIZE)
	{
		//printf("Read PE image header failed\n");
		//__asm jmp $
		return false;
	}

	//printf("Open %s OK\n", filename);
	return true;
}

uint32 PE_FILE::image_size()
{
	PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER)m_header_buf;
	PIMAGE_NT_HEADERS pe_hdr = (PIMAGE_NT_HEADERS)(m_header_buf + dos_hdr->e_lfanew);
	uint32 image_size = pe_hdr->OptionalHeader.SizeOfImage;
	return image_size;
}

bool PE_FILE::load(void* file_buf, int buf_size)
{
	m_image_base = (byte*)file_buf;
	if (m_fat32.load_file(&m_file, file_buf, buf_size) != m_file.size)
	{
		//printf("Load OS failed\n");
		//__asm jmp $
		return false;
	}
	//将未初始化内存清零
	memset((byte*)file_buf + m_file.size, 0, buf_size - m_file.size);
	return true;
}

uint32 PE_FILE::entry_point()
{
	PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER)m_header_buf;
	PIMAGE_NT_HEADERS pe_hdr = (PIMAGE_NT_HEADERS)(m_header_buf + dos_hdr->e_lfanew);
	uint32 entry = pe_hdr->OptionalHeader.ImageBase + pe_hdr->OptionalHeader.AddressOfEntryPoint;
	return entry;
}
