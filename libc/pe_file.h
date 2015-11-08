#pragma once
#include "typedef.h"
#include "fat32.h"

#define PE_HEAD_BUF_SIZE	4096

class PE_FILE
{
private:
	byte		m_drive;
	FAT32		m_fat32;
	uint32		m_volume_start_sector;

	FILE_OBJECT m_file;
	char		m_header_buf[PE_HEAD_BUF_SIZE];
	byte*       m_image_base;
public:
	PE_FILE();
	bool	open(byte drive, char* filename);
	uint32	image_size();
	bool	load(void* file_buf, int buf_size);
	uint32	entry_point();
};
