#pragma once

#define MULTIBOOT_HEADER_MAGIC     0x1BADB002
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002



typedef unsigned short     multiboot_uint16_t;
typedef unsigned int       multiboot_uint32_t;
typedef unsigned long long multiboot_uint64_t;

struct multiboot_elf_section_header_table {
	multiboot_uint32_t num;
	multiboot_uint32_t size;
	multiboot_uint32_t addr;
	multiboot_uint32_t shndx;
};

struct multiboot_aout_symbol_table {
	multiboot_uint32_t tabsize;
	multiboot_uint32_t strsize;
	multiboot_uint32_t addr;
	multiboot_uint32_t reserved;
};

typedef struct multiboot_info {
	multiboot_uint32_t flags;
	multiboot_uint32_t mem_lower;
	multiboot_uint32_t mem_upper;
	multiboot_uint32_t boot_device;
	multiboot_uint32_t cmdline;
	multiboot_uint32_t mod_count;
	multiboot_uint32_t mod_addr;
	union {
		struct multiboot_aout_symbol_table aout_sym;
		struct multiboot_elf_section_header_table elf_sec;
	} u;
	multiboot_uint32_t mmap_length;
	multiboot_uint32_t mmap_addr;
	multiboot_uint32_t drives_length;
	multiboot_uint32_t drives_addr;
	multiboot_uint32_t config_table;
	multiboot_uint32_t boot_loader_name;
	multiboot_uint32_t apm_table;
	multiboot_uint32_t vbe_control_info;
	multiboot_uint32_t vbe_mode_info;
	multiboot_uint16_t vbe_mode;
	multiboot_uint16_t vbe_interface_seg;
	multiboot_uint16_t vbe_interface_off;
	multiboot_uint16_t vbe_interface_len;
} multiboot_info;

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED  2
typedef struct multiboot_mmap_entry {
	multiboot_uint32_t size;
	multiboot_uint32_t base_low;
	multiboot_uint32_t base_high;
	multiboot_uint32_t len_low;
	multiboot_uint32_t len_high;
	multiboot_uint32_t type;
};

typedef struct multiboot_mod_list {
	multiboot_uint32_t start;
	multiboot_uint32_t end;
	multiboot_uint32_t args;
	multiboot_uint32_t pad;
} multiboot_mod_list;

