#include "idt.h"
#include "gdt.h"
#include "stdio.h"
#include <string.h>
#include "intrin.h"

void idt_irq_dispatch(IRQ_CONTEXT* context);

#define IRQ_ENTRY_WITH_ERROR(irq_no) \
void  __declspec(naked) irq_entry_##irq_no() \
{ \
	__asm	cli \
	__asm	push		irq_no  /*中断请求向量*/ \
	__asm	pushad \
	__asm   push	esp \
	__asm	call		idt_irq_dispatch \
	__asm   add		esp, 4 \
	__asm	popad \
	__asm   add		esp,	4 \
	__asm	sti \
	__asm	iretd \
}

#define IRQ_ENTRY_NO_ERROR(irq_no) \
void  __declspec(naked) irq_entry_##irq_no() \
{ \
	__asm	cli \
	__asm	push		irq_no  /*伪error code*/ \
	__asm	push		irq_no  /*中断请求向量*/ \
	__asm	pushad \
	__asm   push	esp \
	__asm	call		idt_irq_dispatch \
	__asm   add		esp, 4 \
	__asm	popad \
	__asm   add		esp,	 8 \
	__asm	sti \
	__asm	iretd \
}

IRQ_ENTRY_NO_ERROR(0x00) //除法错误
IRQ_ENTRY_NO_ERROR(0x01) //单步调试	INT 1
IRQ_ENTRY_NO_ERROR(0x02) //非屏蔽中断NMI
IRQ_ENTRY_NO_ERROR(0x03) //断点	INT 3
IRQ_ENTRY_NO_ERROR(0x04) //溢出中断	INTO
IRQ_ENTRY_NO_ERROR(0x05) //边界越界  BOUND
IRQ_ENTRY_NO_ERROR(0x06)//无效指令   UD2
IRQ_ENTRY_NO_ERROR(0x07) //! device not available
IRQ_ENTRY_WITH_ERROR(0x08) //! double fault
IRQ_ENTRY_NO_ERROR(0x09) //协处理器跨段
IRQ_ENTRY_WITH_ERROR(0x0A) //无效TSS
IRQ_ENTRY_WITH_ERROR(0x0B) //段不存在
IRQ_ENTRY_WITH_ERROR(0x0C) //栈故障
IRQ_ENTRY_WITH_ERROR(0x0D) //常规保护
IRQ_ENTRY_WITH_ERROR(0x0E) //页故障
IRQ_ENTRY_NO_ERROR(0x0F) //15(0x0F) 因特尔保留

IRQ_ENTRY_NO_ERROR(0x10) //浮点处理器错误
IRQ_ENTRY_WITH_ERROR(0x11) //对齐检查
IRQ_ENTRY_NO_ERROR(0x12) //机器检查
IRQ_ENTRY_NO_ERROR(0x13) //SIMD异常
IRQ_ENTRY_NO_ERROR(0x14) //20-31(0x14--0x1F) 因特尔保留
IRQ_ENTRY_NO_ERROR(0x15)
IRQ_ENTRY_NO_ERROR(0x16)
IRQ_ENTRY_NO_ERROR(0x17)
IRQ_ENTRY_NO_ERROR(0x18)
IRQ_ENTRY_NO_ERROR(0x19)
IRQ_ENTRY_NO_ERROR(0x1A)
IRQ_ENTRY_NO_ERROR(0x1B)
IRQ_ENTRY_NO_ERROR(0x1C)
IRQ_ENTRY_NO_ERROR(0x1D)
IRQ_ENTRY_NO_ERROR(0x1E)
IRQ_ENTRY_NO_ERROR(0x1F)

//32-255 用户定义中断
IRQ_ENTRY_NO_ERROR(0x20)
IRQ_ENTRY_NO_ERROR(0x21)
IRQ_ENTRY_NO_ERROR(0x22)
IRQ_ENTRY_NO_ERROR(0x23)
IRQ_ENTRY_NO_ERROR(0x24)
IRQ_ENTRY_NO_ERROR(0x25)
IRQ_ENTRY_NO_ERROR(0x26)
IRQ_ENTRY_NO_ERROR(0x27)
IRQ_ENTRY_NO_ERROR(0x28)
IRQ_ENTRY_NO_ERROR(0x29)
IRQ_ENTRY_NO_ERROR(0x2A)
IRQ_ENTRY_NO_ERROR(0x2B)
IRQ_ENTRY_NO_ERROR(0x2C)
IRQ_ENTRY_NO_ERROR(0x2D)
IRQ_ENTRY_NO_ERROR(0x2E)
IRQ_ENTRY_NO_ERROR(0x2F)

void  __declspec(naked) irq_default_entry()
{
	__asm iretd
}

static void* irq_entrys[32] = 
{
	irq_entry_0x00,
	irq_entry_0x01,
	irq_entry_0x02,
	irq_entry_0x03,
	irq_entry_0x04,
	irq_entry_0x05,
	irq_entry_0x06,
	irq_entry_0x07,
	irq_entry_0x08,
	irq_entry_0x09,
	irq_entry_0x0A,
	irq_entry_0x0B,
	irq_entry_0x0C,
	irq_entry_0x0D,
	irq_entry_0x0E,
	irq_entry_0x0F,

	irq_entry_0x10,
	irq_entry_0x11,
	irq_entry_0x12,
	irq_entry_0x13,
	irq_entry_0x14,
	irq_entry_0x15,
	irq_entry_0x16,
	irq_entry_0x17,
	irq_entry_0x18,
	irq_entry_0x19,
	irq_entry_0x1A,
	irq_entry_0x1B,
	irq_entry_0x1C,
	irq_entry_0x1D,
	irq_entry_0x1E,
	irq_entry_0x1F
};

IDT g_idt;

void idt_irq_dispatch( IRQ_CONTEXT* context)
{
	g_idt.irq_dispatch(context);
}

void IDT::irq_dispatch(IRQ_CONTEXT* context)
{
	uint32 irq_no = context->int_no;
	IRQ_HANDLER irq_handler = g_idt.m_irq_handlers[irq_no];
	if (irq_handler != NULL) irq_handler(context);
}

IDT::IDT()
{
	memset(m_idt, 0, sizeof(m_idt));
	memset(m_irq_handlers, 0, sizeof(m_irq_handlers));
}

IDT::~IDT()
{
}

void IDT::Init()
{
	__asm cli

	for (int i = 0x00; i < 0x1F; i++)
	{
		set_idt_entry(i, IDT_TRAP_GATE, irq_entrys[i]);
	}

	for (int i = 0x20; i < 0x2F; i++)
	{
		set_idt_entry(i, IDT_INTR_GATE, irq_entrys[i]);
	}

	for (int i = 0x30; i < MAX_IDT_NUM; i++)
	{
		set_idt_entry(i, IDT_INTR_GATE, irq_default_entry);
	}
	
	IDTR idtr;
	idtr.limit = sizeof(m_idt) - 1;
	idtr.base =  m_idt;
	__asm	lidt		idtr; // 内嵌汇编载入 idt 表描述符

	printf("IDT init() OK\n");
}

void IDT::set_tss_gate(int vector, IRQ_HANDLER* handler)
{

}

void IDT::set_call_gate(int vector, IRQ_HANDLER* handler)
{

}

void IDT::set_intr_gate(int vector, IRQ_HANDLER* handler)
{
	m_irq_handlers[vector] = handler;
}

void IDT::set_trap_gate(int vector, IRQ_HANDLER* handler)
{

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
