#include "8259.h"
#include "gdt.h"
#include "stdio.h"

PIC_IRQ_HANDLER PIC::m_irq_handlers[16] = { 0 };

#define PIC_IRQ_ENTRY(irq_id) \
void  __declspec(naked) pic_irq_entry_##irq_id() \
{ \
	__asm	cli \
	__asm	push		irq_id  /*中断请求向量*/ \
	__asm	pushad \
	__asm   push	esp \
	__asm	call	PIC::irq_dispatch \
	__asm   add		esp, 4 \
	__asm	popad \
	__asm   add		esp,	4 \
	__asm	sti \
	__asm	iretd \
}

//32-255 用户定义中断
PIC_IRQ_ENTRY(0x00)
PIC_IRQ_ENTRY(0x01)
PIC_IRQ_ENTRY(0x02)
PIC_IRQ_ENTRY(0x03)
PIC_IRQ_ENTRY(0x04)
PIC_IRQ_ENTRY(0x05)
PIC_IRQ_ENTRY(0x06)
PIC_IRQ_ENTRY(0x07)

PIC_IRQ_ENTRY(0x08)
PIC_IRQ_ENTRY(0x09)
PIC_IRQ_ENTRY(0x0A)
PIC_IRQ_ENTRY(0x0B)
PIC_IRQ_ENTRY(0x0C)
PIC_IRQ_ENTRY(0x0D)
PIC_IRQ_ENTRY(0x0E)
PIC_IRQ_ENTRY(0x0F)

void* pic_irq_entries[16]
{
	pic_irq_entry_0x00,
	pic_irq_entry_0x01,
	pic_irq_entry_0x02,
	pic_irq_entry_0x03,
	pic_irq_entry_0x04,
	pic_irq_entry_0x05,
	pic_irq_entry_0x06,
	pic_irq_entry_0x07,
	pic_irq_entry_0x08,
	pic_irq_entry_0x09,
	pic_irq_entry_0x0A,
	pic_irq_entry_0x0B,
	pic_irq_entry_0x0C,
	pic_irq_entry_0x0D,
	pic_irq_entry_0x0E,
	pic_irq_entry_0x0F
};

void PIC::irq_dispatch(PIC_IRQ_CONTEXT* context)
{
	static int Counter = 0;
	static int CounterSec = 0;
	static int Timer = 0;
	int irq_id = context->int_id;
	PIC_IRQ_HANDLER irq_handler = m_irq_handlers[irq_id];
	if (irq_handler != NULL)
	{
		irq_handler(context);
	}
	//发送中断结束命令EOI（0x20)
	outportb(PIC1_CMD_PORT, 0x20);//向主片发送中断结束命令
	outportb(PIC2_CMD_PORT, 0x20);//向从片发送中断结束命令
}

void PIC::Init(IDT* idt)
{
	// 给中断寄存器编程
	// 发送 ICW1 : 使用 ICW4，级联工作
	outportb(PIC1_CMD_PORT, 0x11);
	outportb(PIC2_CMD_PORT, 0x11);
	// 发送 ICW2，中断起始号从 0x20 开始（第一片）及 0x28开始（第二片）
	outportb(PIC1_DATA_PORT, PIC1_INT0);
	outportb(PIC2_DATA_PORT, PIC2_INT0);
	// 发送 ICW3
	outportb(PIC1_DATA_PORT, 0x4);
	outportb(PIC2_DATA_PORT, 0x2);
	// 发送 ICW4
	outportb(PIC1_DATA_PORT, 0x1);
	outportb(PIC2_DATA_PORT, 0x1);
	// 设置中断屏蔽位 OCW1 ，屏蔽所有中断请求
	outportb(PIC1_DATA_PORT, 0xFB);
	outportb(PIC2_DATA_PORT, 0xFF);

	//设置硬件中断向量
	idt->set_intr_gate(PIC1_INT0 + 0, pic_irq_entry_0x00);
	idt->set_intr_gate(PIC1_INT0 + 1, pic_irq_entry_0x01);
	idt->set_intr_gate(PIC1_INT0 + 2, pic_irq_entry_0x02);
	idt->set_intr_gate(PIC1_INT0 + 3, pic_irq_entry_0x03);
	idt->set_intr_gate(PIC1_INT0 + 4, pic_irq_entry_0x04);
	idt->set_intr_gate(PIC1_INT0 + 5, pic_irq_entry_0x05);
	idt->set_intr_gate(PIC1_INT0 + 6, pic_irq_entry_0x06);
	idt->set_intr_gate(PIC1_INT0 + 7, pic_irq_entry_0x07);

	idt->set_intr_gate(PIC2_INT0 + 0, pic_irq_entry_0x08);
	idt->set_intr_gate(PIC2_INT0 + 1, pic_irq_entry_0x09);
	idt->set_intr_gate(PIC2_INT0 + 2, pic_irq_entry_0x0A);
	idt->set_intr_gate(PIC2_INT0 + 3, pic_irq_entry_0x0B);
	idt->set_intr_gate(PIC2_INT0 + 4, pic_irq_entry_0x0C);
	idt->set_intr_gate(PIC2_INT0 + 5, pic_irq_entry_0x0D);
	idt->set_intr_gate(PIC2_INT0 + 6, pic_irq_entry_0x0E);
	idt->set_intr_gate(PIC2_INT0 + 7, pic_irq_entry_0x0F);

	printf("8259A init() OK\n");
}

bool PIC::register_irq(int irq_no, PIC_IRQ_HANDLER irq_handler)
{
	if (irq_no < 0 || irq_no > 15) return false;
	m_irq_handlers[irq_no] = irq_handler;
	if (irq_handler) enable_irq(irq_no);
	else disable_irq(irq_no);
	return true;
}

// 设置IRQ屏蔽位图
void PIC::enable_irq(int irq)
{
	uint8 mask_word;
	if (irq < 0 || irq > 15) return;
	uint8 mask_bit = 1 << (irq % 8);
	if (irq < 8)
	{
		mask_word = inportb(PIC1_DATA_PORT);
		mask_word &= ~mask_bit;
		outportb(PIC1_DATA_PORT, mask_word);
	}
	else
	{
		mask_word = inportb(PIC2_DATA_PORT);
		mask_word &= ~mask_bit;
		outportb(PIC2_DATA_PORT, mask_word);
		// 必须同时打开主片的 IRQ2
		mask_word = inportb(PIC1_DATA_PORT);
		mask_word &= 0xFD;
		outportb(PIC1_DATA_PORT, mask_word);
	}
}

void PIC::disable_irq(int irq)
{
	uint8 mask_word;
	if (irq < 0 || irq > 15) return;
	uint8 mask_bit = 1 << (irq % 8);
	if (irq < 8)
	{
		mask_word = inportb(PIC1_DATA_PORT);
		mask_word |= mask_bit;
		outportb(PIC1_DATA_PORT, mask_word);
	}
	else
	{
		mask_word = inportb(PIC2_DATA_PORT);
		mask_word |= mask_bit;
		outportb(PIC2_DATA_PORT, mask_word);
	}
}

void  PIC::dump_pic_irq_context(PIC_IRQ_CONTEXT* context)
{
	printf("cs:eip=%02X:%08X ss:esp=%02X:%08X int_id=%d\n",
		context->cs, context->eip, context->ss, context->esp, context->int_id);
	printf("eax=%08X ebx=%08X ecx=%08X edx=%08X\n", context->eax, context->ebx, context->ecx, context->edx);
	printf("ebp=%08X edi=%08X esi=%08X eflags=%08X\n", context->ebp, context->edi, context->esi, context->eflags);
}