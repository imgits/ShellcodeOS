#pragma once 
#include "typedef.h"

#define SECTOR_SIZE		512
#define MAX_PATH			260
#define NULL             0

#define DIR_ENTRY_ATTR_READ_ONLY     0x01
#define DIR_ENTRY_ATTR_HIDDEN        0x02
#define DIR_ENTRY_ATTR_SYSTEM        0x04
#define DIR_ENTRY_ATTR_VOLUME_ID     0x08
#define DIR_ENTRY_ATTR_DIRECTORY     0x10
#define DIR_ENTRY_ATTR_ARCHIVE       0x20
#define DIR_ENTRY_ATTR_LONG_NAME     0x0f


#pragma pack(push,1)
//Partition table entry structure
struct PTINFO
{
	uint8		active;			// 0x80 if partition active
	uint8		start_head;		// starting head
	uint8		start_sector;	// starting cylinder and sector (low byte)
	uint8		start_cylinder;	// starting cylinder and sector (high byte)
	uint8		type;			// type ID byte
	uint8		end_head;		// ending head
	uint8		end_sector;		// ending cylinder and sector (low byte)
	uint8		end_cylinder;	// ending cylinder and sector (high byte)
	uint32		first_sector;	// starting sector# (low byte)
	uint32		total_sectors;	// size of partition (low byte)
};

//Master Boot Record structure
struct MBR
{
	uint8 bootcode[0x1be];	// boot sector
	PTINFO ptable[4];			// four partition table structures
	uint8 sig_0x55;				// 0x55 signature byte
	uint8 sig_0xaa;				// 0xaa signature byte
};

struct FAT32_BOOT_SECTOR
{
	uint8 jump[3];
	char    oem_id[8];
	uint16 bytes_per_sector;
	uint8  sectors_per_cluster;
	uint16 reserved_sector_count;
	uint8  fat_count;//16
	uint16 root_dir_entry_count;
	uint16 total_sectors16;
	uint8  media_type;
	uint16 sectors_per_fat16;
	uint16 sectors_per_track;
	uint16 head_count;
	uint32 hiddden_sectors;
	uint32 total_sectors32;
	uint32 sectors_per_fat32;
	uint16 fat32_flags;
	uint16 fat32_version;
	uint32 fat32_root_cluster;
	uint16 fat32_fsinfo;
	uint16 fat32_back_boot_block;
	uint8  fat32_reserved[12];
	uint8  drive_number;
	uint8  reserved1;
	uint8  boot_signature;
	uint32 volume_serial_number;
	char     volume_label[11];
	char     file_system_type[8];
	uint8  boot_code[420];
	uint8  sig_0x55;
	uint8  sig_0xaa;
};

struct FAT32_DIR_ENTRY
{
	uint8  name[11];
	uint8  attributes;
	uint8  reservedNT;
	uint8  creationTimeTenths;
	uint16 creationTime;
	uint16 creationDate;
	uint16 lastAccessDate;
	uint16 firstClusterHigh;
	uint16 lastWriteTime;
	uint16 lastWriteDate;
	uint16 firstClusterLow;
	uint32 fileSize;
};

struct FAT32_LONG_NAME_ENTRY
{
	uint8 index : 5;
	uint8 unused1 : 1;
	uint8 last_item : 1;
	uint8 unused2 : 1;
	uint16 unicode_name1[5];
	uint8 attribute;
	uint8 unused3;
	uint8 check_sum;
	uint16 unicode_name2[6];
	uint16 start_cluster;
	uint16 unicode_name3[2];
};

struct FILE_OBJECT
{
	uint32 start_cluster;
	uint32 size;
	uint32 read_write_ptr;
	uint32 cache_offset;
	uint8  cache_buf[SECTOR_SIZE];
};


byte inportb(uint16 port);
void outportb(uint16 port, byte val);
bool	read_sectors(void* sec_buf, uint32 first_sector, int total_secs);
int get_part_name(const char* pathname, char * part_name_buf, int buf_size);
uint32 cluster_to_LBA(uint32 cluster);
int    copy_short_name(FAT32_DIR_ENTRY* dir_entry, char* short_name);
int    strcmp_nocase(const char* str1, const char * str2);
bool   search_dir_entry(const char* entry_name, uint32& start_cluster, uint32 &file_size);
bool	   open_file(FILE_OBJECT *file, const char* filename);
uint32 load_file(FILE_OBJECT *file, void* filebuf, uint32 bufsize);

#pragma intrinsic (memcpy) 
extern "C" void* memcpy(void* destination, const void* source, size_t num);

extern "C" void * __cdecl memset(void *mem, int value, size_t count);
#pragma intrinsic (memset) 

#pragma pack(pop)