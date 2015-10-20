#include "8259.h"
#include "gdt.h"
#include "stdio.h"

//32-255 用户定义中断
#define INTERRUPT_HANDLER(irq_no) \
{ \
	__asm	cli \
	__asm	push		irq_no  /*中断请求向量*/ \
	__asm	pushad \
	__asm   push	esp \
	__asm	call		interrupt_dispatch \
	__asm   add		esp, 4 \
	__asm	popad \
	__asm   add		esp,	4 \
	__asm	sti \
	__asm	iretd \
}

void __cdecl interrupt_dispatch(INTERRUPT_CONTEXT* context)
{
	static int Counter = 0;
	static int CounterSec = 0;
	static int Timer = 0;
	int irq_no = context->int_no;
	//IRQ_HANDLER irq_handler = NULL;
	switch (irq_no)
	{
	case IRQ_PIT://8253 Counter
		if (++Counter == 100)
		{
			CounterSec++;
			printf("Clock=%d int_no=%d\n", CounterSec, irq_no);
			Counter = 0;
		}
		break;
	case IRQ_KEYBOARD://keyboard
					  //keyboard_handler();
		break;

	case IRQ_RTC://COMS Real time clock
		Timer++;
		printf("Timer %d int_no=%d\n", Timer, irq_no);
		outportb(0x70, 0x0c);
		inportb(0x71);
		break;
	}
	//发送中断结束命令EOI（0x20)
	outportb(PIC1_CMD_PORT, 0x20);//向主片发送中断结束命令
	outportb(PIC2_CMD_PORT, 0x20);//向从片发送中断结束命令
}

#ifndef INTERRUPT_HANDLER_XX

#define HARDWARE_INTERRUPT void  __declspec(naked) 

HARDWARE_INTERRUPT irq_00() {	INTERRUPT_HANDLER( 0); }//计数器中断
HARDWARE_INTERRUPT irq_01() {	INTERRUPT_HANDLER( 1); }//键盘
HARDWARE_INTERRUPT irq_02() {	INTERRUPT_HANDLER( 2); }//级联
HARDWARE_INTERRUPT irq_03() {	INTERRUPT_HANDLER( 3); }//COM2
HARDWARE_INTERRUPT irq_04() {	INTERRUPT_HANDLER( 4); }//COM1
HARDWARE_INTERRUPT irq_05() {	INTERRUPT_HANDLER( 5); }//并口2
HARDWARE_INTERRUPT irq_06() {	INTERRUPT_HANDLER( 6); }//软驱
HARDWARE_INTERRUPT irq_07() {	INTERRUPT_HANDLER( 7); }//并口1

HARDWARE_INTERRUPT irq_08() {	INTERRUPT_HANDLER( 8); }//CMOS时钟
HARDWARE_INTERRUPT irq_09() {	INTERRUPT_HANDLER( 9); }//INT 0x0A ？？
HARDWARE_INTERRUPT irq_10() {	INTERRUPT_HANDLER( 10); }//保留
HARDWARE_INTERRUPT irq_11() {	INTERRUPT_HANDLER( 11); }//保留
HARDWARE_INTERRUPT irq_12() {	INTERRUPT_HANDLER( 12); }//PS2鼠标
HARDWARE_INTERRUPT irq_13() {	INTERRUPT_HANDLER( 13); }//协处理器
HARDWARE_INTERRUPT irq_14() {	INTERRUPT_HANDLER( 14); }//主硬盘
HARDWARE_INTERRUPT irq_15() {	INTERRUPT_HANDLER( 15); }//从硬盘

#endif //INTERRUPT_HANDLER_XX

PIC::PIC()
{
}

PIC::~PIC()
{
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
	idt->set_idt_entry(PIC1_INT0 + 0, DA_386IGate, irq_00);
	idt->set_idt_entry(PIC1_INT0 + 1, DA_386IGate, irq_01);
	idt->set_idt_entry(PIC1_INT0 + 2, DA_386IGate, irq_02);
	idt->set_idt_entry(PIC1_INT0 + 3, DA_386IGate, irq_03);
	idt->set_idt_entry(PIC1_INT0 + 4, DA_386IGate, irq_04);
	idt->set_idt_entry(PIC1_INT0 + 5, DA_386IGate, irq_05);
	idt->set_idt_entry(PIC1_INT0 + 6, DA_386IGate, irq_06);
	idt->set_idt_entry(PIC1_INT0 + 7, DA_386IGate, irq_07);

	idt->set_idt_entry(PIC2_INT0 + 0, DA_386IGate, irq_08);
	idt->set_idt_entry(PIC2_INT0 + 1, DA_386IGate, irq_09);
	idt->set_idt_entry(PIC2_INT0 + 2, DA_386IGate, irq_10);
	idt->set_idt_entry(PIC2_INT0 + 3, DA_386IGate, irq_11);
	idt->set_idt_entry(PIC2_INT0 + 4, DA_386IGate, irq_12);
	idt->set_idt_entry(PIC2_INT0 + 5, DA_386IGate, irq_13);
	idt->set_idt_entry(PIC2_INT0 + 6, DA_386IGate, irq_14);
	idt->set_idt_entry(PIC2_INT0 + 7, DA_386IGate, irq_15);

	//init_cmos_timer();
	//init_8253_counter(20);
	//init_keyboard();
	//enable_irq(IRQ_PIT);
	//enable_irq(IRQ_KEYBOARD);
	//enable_irq(8);
	//__asm sti;
	printf("8259A init() OK\n");
}

bool PIC::register_irq(int irq_no, IRQ_HANDLER irq_handler)
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

////http://wiki.osdev.org/RTC
//void PIC::init_cmos_timer()
//{
//	//设置和时钟中断相关的硬件 
//	outportb(0x70, 0x0b | 0x80); // RTC寄存器B | 阻断NMI
//	outportb(0x71, 0x12); //设置寄存器B，禁止周期性中断，开放更新结束后中断，BCD码，24小时制
//	outportb(0x70, 0x0c); //读RTC寄存器C，复位未决的中断状态
//	inportb(0x71);
//}
