#include "system.h"
#include <string.h>
#include "page_frame.h"
#include "trap.h"
//#include "bios.h"

SYSTEM  System;

SYSTEM::SYSTEM()
{
}

SYSTEM::~SYSTEM()
{
}

bool SYSTEM::Init(uint32 kernel_image_size, memory_info* meminfo)
{
	m_gdt.Init();
	m_idt.Init();
	m_tss.Init(&m_gdt);
	TRAP::Init(&m_idt);
	PIC::Init(&m_idt);
	PIT::Init();
	m_rtc.Init(&m_idt);

	KBD::Init();

	m_kernel_image_size = kernel_image_size;
	memcpy(&m_meminfo, meminfo, sizeof(memory_info));
	init_kernel_mmu();

	return true;
}

bool   SYSTEM::init_kernel_mmu()
{
#define PAGE_DIR_LOW1M_FARMES	1	//页目录占用1页
#define PAGE_TABLE_LOW1M_FRAMES	1   //0-4M低端内存映射页表占用1页
#define PAGE_TABLE_KERNEL_FRAMES	((m_kernel_image_size+ MB(4)-1)/MB(4))//内核页表，每4M占用1页
#define KERNEL_FRAMES   SIZE_TO_PAGES(m_kernel_image_size) //内核实际占用页数

	uint32 page_frame_used = SIZE_TO_PAGES(MB(1))
		+ PAGE_DIR_LOW1M_FARMES
		+ PAGE_TABLE_LOW1M_FRAMES
		+ PAGE_TABLE_KERNEL_FRAMES
		+ KERNEL_FRAMES;

	PAGE_FRAME_DB::Init(page_frame_used, &m_meminfo);

	//m_kmem.Init();
	m_kmem.reserve_virtual_space(PAGE_TABLE_BASE, MB(4));
	m_kmem.reserve_virtual_space(PAGE_FRAME_BASE, MB(1));
	m_kmem.reserve_virtual_space(PAGE_LOW1M_BASE, MB(1));
	m_kmem.reserve_virtual_space(KERNEL_BASE, m_kernel_image_size);

	m_ram_size = 0;
	for (int i = 0; i < m_meminfo.map_count; i++)
	{
		uint64 length = m_meminfo.mem_maps[i].length;
		uint64 begin = m_meminfo.mem_maps[i].base_addr;
		uint64 end = begin + length;
		if (end < KERNEL_BASE || begin >= 0x100000000I64) continue;
		switch (m_meminfo.mem_maps[i].type)
		{
		case MEMTYPE_RAM:
			if (m_ram_size < end && end < 0x100000000I64)
			{
				m_ram_size = end;
			}
			break;
		case MEMTYPE_ACPI: 
		case MEMTYPE_NVS:
			m_kmem.reserve_virtual_space(begin, end - begin);
			break;
		}
	}
	return true;
}

