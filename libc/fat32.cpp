#include "Fat32.h"
#include "ioport.h"
#include "vga.h"

#ifdef _DEBUG
bool	read_sectors(void* sec_buf, uint32 first_sector, int total_secs)
{
	static FILE* fp_vhd = NULL;
	if (fp_vhd == NULL) fp_vhd = fopen("e:\\ShellcodeOS.vhd", "rb");
	fseek(fp_vhd, first_sector * 512, SEEK_SET);
	int len = fread(sec_buf, 512, total_secs, fp_vhd);
	return len == total_secs;
}
#else
bool	 read_sectors(void* sec_buf, uint32 first_sector, int total_secs)
{
	uint8 *pdata = (uint8*)sec_buf;
	for (int start_sector = first_sector; start_sector < first_sector + total_secs; start_sector++)
	{
		outportb(0x1f2, 1);//读取的扇区数

		outportb(0x1f3, start_sector & 0xFF); //LBA地址7~0
		outportb(0x1f4, (start_sector >> 8) & 0xFF); //LBA地址15~8
		outportb(0x1f5, (start_sector >> 16) & 0xFF); //LBA地址23~16
		outportb(0x1f6, 0xE0 | ((start_sector >> 24) & 0x0F)); //第一硬盘  LBA地址27~24

		outportb(0x1f7, 0x20); //读命令

							   //等待
		while ((inportb(0x1f7) & 0x88) != 8); //不忙，且硬盘已准备好数据传输 
		__asm   cld
		__asm	mov  ecx, 512 / 2
		__asm   mov  edi, pdata
		__asm   mov  edx, 0x1f0
		__asm   rep  insw
		pdata += 512;
	}
	return true;
}
#endif

FAT32::FAT32()
{
	m_driver = 0;
	
	m_volume_start_sector = 0;
	m_fat_start_sector = 0;
	m_cluster_start_sector = 0;
	m_sectors_per_cluster = 0;
	m_rootdir_start_cluster = 0;
}

bool FAT32::Init(byte  driver, uint32 volume_start_sector)
{
	m_driver = driver;
	m_volume_start_sector = volume_start_sector;

	FAT32_BOOT_SECTOR fat32_boot;
	if (!read_sectors(&fat32_boot, m_volume_start_sector, 1))
	{
		return false;
	}

	m_fat_start_sector = m_volume_start_sector + fat32_boot.reserved_sector_count;
	m_cluster_start_sector = m_fat_start_sector + (fat32_boot.fat_count * fat32_boot.sectors_per_fat32);
	m_sectors_per_cluster = fat32_boot.sectors_per_cluster;
	m_rootdir_start_cluster = fat32_boot.fat32_root_cluster;
	return true;
}

int 	 FAT32::get_part_name(const char* pathname, char * part_name_buf, int buf_size)
{
	if (pathname[0] != '\\' && pathname[0] != '/')
	{
		return -1;
	}
	const char* pname = pathname + 1;
	char ch = '\n';
	for (int i = 0; i < buf_size; i++)
	{
		ch = *pname++;
		part_name_buf[i] = ch;
		switch (ch)
		{
		case 0:
			return i;
		case '/':
		case '\\':
			part_name_buf[i] = 0;
			return i;
		case '\r':
		case '\n':
		case '?':
		case '*':
		case '<':
		case '>':
		case '|':
		case '"':
			part_name_buf[i] = 0;
			break;
		}
	}
	return -1;
}

uint32 	 FAT32::cluster_to_LBA(uint32 cluster)
{
	return  m_cluster_start_sector + (cluster - 2) * m_sectors_per_cluster;
}

int    	 FAT32::copy_short_name(FAT32_DIR_ENTRY* dir_entry, char* short_name)
{
	char* pname = short_name;
	const char* dir_entry_name = (const char*)dir_entry->name;
	for (int i = 0; i < 8; i++)
	{
		if (dir_entry_name[i] == ' ') break;
		*pname++ = dir_entry_name[i];
	}
	bool first_ext_char = true;
	for (int i = 8; i < 11; i++)
	{
		if (dir_entry_name[i] == ' ') break;
		if (first_ext_char)
		{
			*pname++ = '.';
			first_ext_char = false;
		}
		*pname++ = dir_entry_name[i];
	}
	*pname = 0;
	return pname - short_name;
}

int    	 FAT32::strcmp_nocase(const char* str1, const char * str2)
{
	unsigned char *p1 = (unsigned char *)str1;
	unsigned char *p2 = (unsigned char *)str2;
	unsigned char ch1 = *p1, ch2 = *p2;
	while (*p1 && *p2)
	{
		ch1 = *p1;
		ch2 = *p2;
		if (ch1 >= 'A' && ch1 <= 'Z') ch1 += 0x20;
		if (ch2 >= 'A' && ch2 <= 'Z') ch2 += 0x20;
		if (ch1 != ch2) break;
		p1++;
		p2++;
	}
	if (ch1 > ch2) return 1;
	if (ch1 < ch2) return -1;
	return 0;
}

bool   	 FAT32::search_dir_entry(const char* entry_name, uint32& start_cluster, uint32 &file_size)
{
	char   fat_buf[SECTOR_SIZE];
	char   dir_buf[SECTOR_SIZE];
	char   short_name[16];
	uint32 last_fat_sector = -1;
	uint32 next_cluster = start_cluster;
	do
	{
		uint32 cluster_sector = cluster_to_LBA(next_cluster);

		for (int i = 0; i < m_sectors_per_cluster; i++)
		{
			read_sectors(dir_buf, cluster_sector + i, 1);
			FAT32_DIR_ENTRY *pdir = (FAT32_DIR_ENTRY *)dir_buf;
			for (int j = 0; j < SECTOR_SIZE / sizeof(FAT32_DIR_ENTRY); j++, pdir++)
			{
				if (pdir->attributes == 0x0f ||//不处理长文件名
					pdir->name[0] == 0x00 ||
					pdir->name[0] == 0x05 ||
					pdir->name[0] == 0xe5 ||
					pdir->attributes & DIR_ENTRY_ATTR_VOLUME_ID)
				{
					continue;
				}
				copy_short_name(pdir, short_name);
				//printf("short_name:%s\n", short_name);
				if (strcmp_nocase(entry_name, short_name) == 0)
				{
					//printf("Found short_name:%s\n", short_name);
					start_cluster = ((uint32)pdir->firstClusterHigh << 16) + pdir->firstClusterLow;
					file_size = pdir->fileSize;
					return true;
				}
			}
		}
		uint32 fat_sector = m_fat_start_sector + (next_cluster * 4) / SECTOR_SIZE;
		uint32 fat_offset = (next_cluster * 4) % SECTOR_SIZE;
		if (last_fat_sector != fat_sector)
		{
			read_sectors(fat_buf, fat_sector, 1);
			last_fat_sector = fat_sector;
		}
		next_cluster = *(uint32*)(fat_buf + fat_offset);
	} while (next_cluster < 0x0ffffff7);
	return false;
}

bool	   	 FAT32::open_file(FILE_OBJECT *file, const char* filename)
{
	char part_name[32];
	int part_len = 0;
	const char* next_path = filename;
	uint32 dir_start_cluster = m_rootdir_start_cluster;
	uint32 file_size = 0;
	uint32 dir_entry_type = 0;
	memset(file, 0, sizeof(FILE_OBJECT));

	do
	{
		part_len = get_part_name(next_path, part_name, sizeof(part_name));
		if (part_len <= 0) return false;
		next_path += 1 + part_len;
		if (!search_dir_entry(part_name, dir_start_cluster, file_size)) break;
		//printf("found part name:%s\n", part_name);
		if (*next_path == 0)
		{
			file->start_cluster = dir_start_cluster;
			file->size = file_size;
			return true;
		}
	} while (part_len > 0);
	return false;
}

uint32 	 FAT32::load_file(FILE_OBJECT *file, void* filebuf, uint32 bufsize)
{
	uint32 file_sectors = (file->size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	uint32 buf_sectors = bufsize / SECTOR_SIZE;
	if (buf_sectors < file_sectors)
	{
		return -1;
	}
	//printf("FAT32::load_file 0\n");
	
	uint32 next_cluster = file->start_cluster;
	uint8* pbuf = (uint8*)filebuf;
	uint32 last_fat_sector = -1;
	uint8  fat_buf[SECTOR_SIZE];
	uint32 read_sector_count = 0;
	do
	{
		//printf("FAT32::load_file 1\n");
		uint32 cluster_sector = cluster_to_LBA(next_cluster);
		for (int i = 0;
		(i < m_sectors_per_cluster) && (read_sector_count < file_sectors);
			i++, read_sector_count++, pbuf += SECTOR_SIZE)
		{
			//printf("FAT32::load_file pbuf=%08X\n", pbuf);
			read_sectors(pbuf, cluster_sector + i, 1);
		}
		//printf("FAT32::load_file 2\n");
		uint32 fat_sector = m_fat_start_sector + (next_cluster * 4) / SECTOR_SIZE;
		uint32 fat_offset = (next_cluster * 4) % SECTOR_SIZE;
		if (last_fat_sector != fat_sector)
		{
			read_sectors(fat_buf, fat_sector, 1);
			last_fat_sector = fat_sector;
		}
		//printf("FAT32::load_file 3\n");
		next_cluster = *(uint32*)(fat_buf + fat_offset);
	} while ((next_cluster < 0x0ffffff7) && (read_sector_count < file_sectors));
	//printf("FAT32::load_file 4\n");
	if ((read_sector_count != file_sectors) ||
		(next_cluster < 0x0ffffff7))
	{
		return -1;
	}
	return file->size;
}


