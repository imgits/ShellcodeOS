#pragma once
#include "typedef.h"
#include "fat32.h"
#include <stdio.h>
#include "pe.h"

typedef void(*kernel_main)(uint32 kernel_image_size, uint32 next_free_page_frame);

class KERNEL_FILE
{
private:
	byte   m_drive;
	FAT32  m_fat32;
	uint32 m_active_part;
	uint32 m_boot_volume_start_sector;

	FILE_OBJECT m_file;
	uint32 m_image_size;
	char*  m_kernel_buf;
public:
	KERNEL_FILE()
	{
		m_drive = 0;
		m_active_part=0;
		m_boot_volume_start_sector=0;
		m_image_size=0;
		m_kernel_buf = NULL;
	}
	bool open(byte drive, char* filename)
	{
		m_drive = drive;
		MBR mbr;
		read_sectors(&mbr, 0, 1);
		m_active_part =0;
		for (int i = 0; i < 4; i++)
		{
			if (mbr.partition_table[i].active == 0x80)
			{
				m_active_part = i;
				break;
			}
		}
		m_boot_volume_start_sector = mbr.partition_table[m_active_part].first_sector;

		m_fat32.Init(drive, m_boot_volume_start_sector);
		
		if (!m_fat32.open_file(&m_file, filename))
		{
			printf("Open %s failed\n", filename);
			__asm jmp $
		}
		printf("Open %s OK\n", filename);
		return true;
	}

	uint32 image_size()
	{
		//加载PE头，主要是为了获取SizeOfImage，以便分配足够的内存空间
		//因为file.size != SizeOfImage (可能)
		//http://blog.csdn.net/wbcuc/article/details/6225344
		//编译时添加选项/Gs32768，避免调用__chkstk
		char pe_header[4096];
		if (m_fat32.read_file(&m_file, pe_header, 4096) != 4096)
		{
			printf("Read OS image header failed\n");
			__asm jmp $
		}

		PE pe(pe_header);
		m_image_size = pe.ImageSize();
		printf("file size =%d image size= %d\n", m_file.size, m_image_size);
		return m_image_size;
	}

	bool load(char* kernel_buf, int buf_size)
	{
		m_kernel_buf = kernel_buf;
		if (m_fat32.load_file(&m_file, kernel_buf, buf_size) != m_file.size)
		{
			printf("Load OS failed\n");
			__asm jmp $
		}
		//将未初始化内存清零
		memset(kernel_buf + m_file.size, 0, buf_size - m_file.size);
		return true;
	}

	kernel_main entry_point()
	{
		PE pe(m_kernel_buf);
		kernel_main os_main = (kernel_main)pe.EntryPoint();
		printf("kernel EntryPoint=%08X\n", os_main);
		return os_main;
	}

};
