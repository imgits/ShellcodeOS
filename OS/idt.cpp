#include "idt.h"
#include "gdt.h"
#include "stdio.h"
#include <string.h>

IDT::IDT()
{
	memset(this, 0, sizeof(m_idt));
}

IDT::~IDT()
{
}

void IDT::Init()
{
	__asm		cli

	// 保存实模式中断屏蔽寄存器(IMREG)值
	memset(this, 0, sizeof(m_idt));
	
	IDTR idtr;
	idtr.limit = sizeof(m_idt) - 1;
	idtr.base =  m_idt;
	__asm	lidt		idtr; // 内嵌汇编载入 idt 表描述符
								 //设置系统服务中断向量
								 //SetIdtEntry(0x2E, DA_386IGate,system_service ); 
	printf("IDT init() OK\n");
}

//设置Interrupt Descriptor Table项
void IDT::set_idt_entry(int vector, uchar desc_type, void* handler)
{
	uint32 base = (uint32)handler;
	GATE_DESC* desc = m_idt + vector;
	desc->offset_low = base & 0xFFFF;
	desc->selector = GDT_KERNEL_CODE;	//系统代码段
	desc->dcount = 0;
	desc->attr = desc_type;
	desc->offset_high = (base >> 16) & 0xFFFF;
}
