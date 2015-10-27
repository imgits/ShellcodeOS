#include "idt.h"
#include "gdt.h"
#include "stdio.h"
#include <string.h>
#include "intrin.h"

void  __declspec(naked) irq_default_entry()
{
	__asm iretd
}


IDT::IDT()
{
	memset(m_idt, 0, sizeof(m_idt));
}

IDT::~IDT()
{
}

void IDT::Init()
{
	__asm cli

	for (int i = 0; i < MAX_IDT_NUM; i++)
	{
		set_idt_entry(i, IDT_INTR_GATE, irq_default_entry);
	}
	
	IDTR idtr;
	idtr.limit = sizeof(m_idt) - 1;
	idtr.base =  m_idt;
	__asm	lidt		idtr; // 内嵌汇编载入 idt 表描述符

	printf("IDT init() OK\n");
}

void IDT::set_tss_gate(int vector, void* handler)
{
	set_idt_entry(vector, IDT_TSS_GATE, handler);
}

void IDT::set_call_gate(int vector, void* handler)
{
	set_idt_entry(vector, IDT_CALL_GATE, handler);
}

void IDT::set_intr_gate(int vector, void* handler)
{
	set_idt_entry(vector, IDT_INTR_GATE, handler);
}

void IDT::set_trap_gate(int vector, void* handler)
{
	set_idt_entry(vector, IDT_TRAP_GATE, handler);
}

//设置Interrupt Descriptor Table项
void IDT::set_idt_entry(int vector, uchar desc_type, void* handler)
{
	uint32 base = (uint32)handler;
	GATE_DESC* desc = m_idt + vector;
	desc->offset_low = base & 0xFFFF;
	desc->selector = SEL_KERNEL_CODE;	//系统代码段
	desc->dcount = 0;
	desc->attr = desc_type;
	desc->offset_high = (base >> 16) & 0xFFFF;
}
