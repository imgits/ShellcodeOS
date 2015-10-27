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

#define MAX_CPU_NUM					32
#define OS_KERNEL_BASE				0x80000000
#define SYSTEM_OBJECT_BASE			0xBFFFE000

#define panic(fmt,...) { printf("%s::%s() line %d:%s\n",__FILE__,__FUNCTION__,__LINE__, fmt, __VA_ARGS__); __asm jmp $}

class SYSTEM //: public Object
{
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
	bool Init(uint32 page_frame_min, uint32 page_frame_max);
};

extern  SYSTEM System;

