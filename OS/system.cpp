#include "system.h"
#include <string.h>
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
	PAGER::Init(kernel_image_size);
	m_gdt.Init();
	m_idt.Init();
	m_tss.Init(&m_gdt);
	TRAP::Init(&m_idt);
	PIC::Init(&m_idt);
	PIT::Init();
	RTC::Init(&m_idt);

	KBD::Init();

	m_kernel_image_size = kernel_image_size;
	memcpy(&m_meminfo, meminfo, sizeof(memory_info));
	init_kernel_mmu();
	alloc_stack();

	m_pci.scan_devices();

	//m_disk.Init(true, true);
	//m_disk.detect_device_type();

	//m_disk.Init(true, false);
	//m_disk.detect_device_type();

	//m_disk.Init(false, true);
	//m_disk.detect_device_type();

	//m_disk.Init(false, false);
	//m_disk.detect_device_type();

	return true;
}

bool   SYSTEM::alloc_stack()
{
	m_stack_base = m_kmem.alloc_virtual_memory(SYSTEM_STACK_SIZE + 2*PAGE_SIZE);
	m_stack_top = m_stack_base + SYSTEM_STACK_SIZE;

	//设置底部隔离区
	m_kmem.free_virtual_memory(m_stack_base, PAGE_SIZE);
	m_kmem.reserve_virtual_space(m_stack_base, PAGE_SIZE);
	m_stack_base += PAGE_SIZE;

	//设置顶部隔离区
	m_stack_top -= PAGE_SIZE;
	m_kmem.free_virtual_memory(m_stack_top, PAGE_SIZE);
	m_kmem.reserve_virtual_space(m_stack_top, PAGE_SIZE);

	printf("sys_stack_base=%08X sys_stack_top=%08X\n", m_stack_base, m_stack_top);
	return true;
}

uint32  SYSTEM::get_stack()
{
	return m_stack_top;
}

bool   SYSTEM::init_kernel_mmu()
{
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

