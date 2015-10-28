#pragma once
#include "typedef.h"
#include "cpu.h"
#include "8253.h"
#include "8259.h"
#include "Keyboard.h"
#include "page_frame.h"
#include "rtc.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "mmu.h"
#include "kernel.h"

#define MAX_CPU_NUM					32

//#define SYSTEM_OBJECT_BASE			0xBFFFE000

class SYSTEM //: public Object
{
private:
	uint32 m_kernel_size;
	uint32 m_mem_size;
public:
	friend KBD;
	friend PAGE_FRAME_DB;
private:
	GDT    m_gdt;
	IDT    m_idt;
	TSS    m_tss;
public:
	uint32			m_cpu_count;
	CPU*			m_cpus[MAX_CPU_NUM];
	MMU<PAGE_SIZE>  m_kmem;
public:
	SYSTEM();
	~SYSTEM();
	bool Init(uint32 kernel_image_size,uint32 mem_size);
};

extern  SYSTEM System;

