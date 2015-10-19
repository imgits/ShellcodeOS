#include "pic.h"
#include "gdt.h"
#include "stdio.h"

struct cpu_state 
{
	uint32	eax;
	uint32	ecx;
	uint32	edx;
	uint32	ebx;
	uint32	esp;
	uint32	ebp;
	uint32	esi;
	uint32	edi;
};

struct stack_state 
{
	uint32	error_code;
	uint32	eip;
	uint32	cs;
	uint32  eflags;
};

//32-255 用户定义中断
#define INTERRUPT_HANDLER(irq_no) \
	__asm	cli \
	__asm	push		irq_no  /*中断请求向量*/ \
	__asm	pushad \
	__asm	call		interrupt_dispatch \
	__asm	popad \
	__asm   add		esp,	4 \
	__asm	sti \
	__asm	iretd

void __cdecl interrupt_dispatch(
	uint32 _edi, uint32 _esi, uint32 _ebp, uint32 _esp,
	uint32 _ebx, uint32 _edx, uint32 _ecx, uint32 _eax,
	uint32 int_no,
	uint32 _eip, uint32 _cs, uint32 _eflags)
{
	static int Counter = 0;
	static int CounterSec = 0;
	static int Timer = 0;
	switch (int_no)
	{
	case IRQ_8253_COUNTER://counter
		if (++Counter == 100)
		{
			CounterSec++;
			printf("Clock=%d int_no=%d\n", CounterSec, int_no);
			Counter = 0;
		}
		break;
	case IRQ_KEYBOARD://keyboard
					  //keyboard_handler();
		break;
	case _8259A_SLAVE_INTERRUPT_VECTOR0 + 0://timer
		Timer++;
		printf("Timer %d int_no=%d\n", Timer, int_no);
		outportb(0x70, 0x0c);
		inportb(0x71);
		break;
	}
	//发送中断结束命令EOI（0x20)
	if (int_no < _8259A_SLAVE_INTERRUPT_VECTOR0)
	{
		outportb(0x20, 0x20);//向主片发送中断结束命令
	}
	else 
	{
		outportb(0xA0, 0x20);//向从片发送中断结束命令
	}
}

#ifndef INTERRUPT_HANDLER_XX

HARDWARE_INTERRUPT irq_00()
{//计数器中断
	INTERRUPT_HANDLER(_8259A_MASTER_INTERRUPT_VECTOR0 + 0);
}

HARDWARE_INTERRUPT irq_01()
{//键盘
	INTERRUPT_HANDLER(_8259A_MASTER_INTERRUPT_VECTOR0 + 1);
}

HARDWARE_INTERRUPT irq_02()
{//级联
	INTERRUPT_HANDLER(_8259A_MASTER_INTERRUPT_VECTOR0 + 2);
}

HARDWARE_INTERRUPT irq_03()
{//COM2
	INTERRUPT_HANDLER(_8259A_MASTER_INTERRUPT_VECTOR0 + 3);
}

HARDWARE_INTERRUPT irq_04()
{//COM1
	INTERRUPT_HANDLER(_8259A_MASTER_INTERRUPT_VECTOR0 + 4);
}

HARDWARE_INTERRUPT irq_05()
{//并口2
	INTERRUPT_HANDLER(_8259A_MASTER_INTERRUPT_VECTOR0 + 5);
}

HARDWARE_INTERRUPT irq_06()
{//软驱
	INTERRUPT_HANDLER(_8259A_MASTER_INTERRUPT_VECTOR0 + 6);
}

HARDWARE_INTERRUPT irq_07()
{//并口1
	INTERRUPT_HANDLER(_8259A_MASTER_INTERRUPT_VECTOR0 + 7);
}

HARDWARE_INTERRUPT irq_08()
{//时钟
	INTERRUPT_HANDLER(_8259A_SLAVE_INTERRUPT_VECTOR0 + 0);
}

HARDWARE_INTERRUPT irq_09()
{//INT 0x0A ？？
	INTERRUPT_HANDLER(_8259A_SLAVE_INTERRUPT_VECTOR0 + 1);
}

HARDWARE_INTERRUPT irq_10()
{//保留
	INTERRUPT_HANDLER(_8259A_SLAVE_INTERRUPT_VECTOR0 + 2);
}

HARDWARE_INTERRUPT irq_11()
{//保留
	INTERRUPT_HANDLER(_8259A_SLAVE_INTERRUPT_VECTOR0 + 3);
}

HARDWARE_INTERRUPT irq_12()
{//PS2鼠标
	INTERRUPT_HANDLER(_8259A_SLAVE_INTERRUPT_VECTOR0 + 4);
}

HARDWARE_INTERRUPT irq_13()
{//协处理器
	INTERRUPT_HANDLER(_8259A_SLAVE_INTERRUPT_VECTOR0 + 5);
}

HARDWARE_INTERRUPT irq_14()
{//硬盘
	INTERRUPT_HANDLER(_8259A_SLAVE_INTERRUPT_VECTOR0 + 6);
}

HARDWARE_INTERRUPT irq_15()
{//保留
	INTERRUPT_HANDLER(_8259A_SLAVE_INTERRUPT_VECTOR0 + 7);
}
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
	outportb(0x20, 0x11);
	outportb(0xA0, 0x11);
	// 发送 ICW2，中断起始号从 0x20 开始（第一片）及 0x28开始（第二片）
	outportb(0x21, _8259A_MASTER_INTERRUPT_VECTOR0);
	outportb(0xA1, _8259A_SLAVE_INTERRUPT_VECTOR0);
	// 发送 ICW3
	outportb(0x21, 0x4);
	outportb(0xA1, 0x2);
	// 发送 ICW4
	outportb(0x21, 0x1);
	outportb(0xA1, 0x1);
	// 设置中断屏蔽位 OCW1 ，屏蔽所有中断请求
	outportb(0x21, 0xFB);
	outportb(0xA1, 0xFF);

	//设置硬件中断向量
	idt->set_idt_entry(_8259A_MASTER_INTERRUPT_VECTOR0 + 0, DA_386IGate, irq_00);
	idt->set_idt_entry(_8259A_MASTER_INTERRUPT_VECTOR0 + 1, DA_386IGate, irq_01);
	idt->set_idt_entry(_8259A_MASTER_INTERRUPT_VECTOR0 + 2, DA_386IGate, irq_02);
	idt->set_idt_entry(_8259A_MASTER_INTERRUPT_VECTOR0 + 3, DA_386IGate, irq_03);
	idt->set_idt_entry(_8259A_MASTER_INTERRUPT_VECTOR0 + 4, DA_386IGate, irq_04);
	idt->set_idt_entry(_8259A_MASTER_INTERRUPT_VECTOR0 + 5, DA_386IGate, irq_05);
	idt->set_idt_entry(_8259A_MASTER_INTERRUPT_VECTOR0 + 6, DA_386IGate, irq_06);
	idt->set_idt_entry(_8259A_MASTER_INTERRUPT_VECTOR0 + 7, DA_386IGate, irq_07);

	idt->set_idt_entry(_8259A_SLAVE_INTERRUPT_VECTOR0 + 0, DA_386IGate, irq_08);
	idt->set_idt_entry(_8259A_SLAVE_INTERRUPT_VECTOR0 + 1, DA_386IGate, irq_09);
	idt->set_idt_entry(_8259A_SLAVE_INTERRUPT_VECTOR0 + 2, DA_386IGate, irq_10);
	idt->set_idt_entry(_8259A_SLAVE_INTERRUPT_VECTOR0 + 3, DA_386IGate, irq_11);
	idt->set_idt_entry(_8259A_SLAVE_INTERRUPT_VECTOR0 + 4, DA_386IGate, irq_12);
	idt->set_idt_entry(_8259A_SLAVE_INTERRUPT_VECTOR0 + 5, DA_386IGate, irq_13);
	idt->set_idt_entry(_8259A_SLAVE_INTERRUPT_VECTOR0 + 6, DA_386IGate, irq_14);
	idt->set_idt_entry(_8259A_SLAVE_INTERRUPT_VECTOR0 + 7, DA_386IGate, irq_15);

	init_cmos_timer();
	init_8253_counter(20);
	//init_keyboard();
	enable_irq(0);
	enable_irq(1);
	//enable_irq(8);
	//__asm sti;
	printf("8259A init() OK\n");
}
// 设置IRQ屏蔽位图
void PIC::enable_irq(int irq)
{
	uint8 mask_word;
	if (irq < 0 || irq > 15) return;
	if (irq < 8) mask_word = inportb(0x21);
	else 	mask_word = inportb(0xA1);
	uint8 mask_bit = 1 << (irq % 8);
	mask_word &= ~mask_bit;
	if (irq < 8)	outportb(0x21, mask_word);
	else
	{
		outportb(0xA1, mask_word);
		// 必须同时打开主片的 IRQ2
		mask_word = inportb(0x21);
		mask_word &= 0xFD;
		outportb(0x21, mask_word);
	}
}

void PIC::disable_irq(int irq)
{
	uint8 mask_word;
	if (irq < 0 || irq > 15) return;
	if (irq < 8) mask_word = inportb(0x21);
	else 	mask_word = inportb(0xA1);
	uint8 mask_bit = 1 << (irq % 8);
	mask_word |= mask_bit;
	if (irq < 8)	outportb(0x21, mask_word);
	else outportb(0xA1, mask_word);
}


void PIC::init_cmos_timer()
{
	//设置和时钟中断相关的硬件 
	outportb(0x70, 0x0b | 0x80); // RTC寄存器B | 阻断NMI
	outportb(0x71, 0x12); //设置寄存器B，禁止周期性中断，开放更新结束后中断，BCD码，24小时制
	outportb(0x70, 0x0c); //读RTC寄存器C，复位未决的中断状态
	inportb(0x71);
}

void PIC::init_8253_counter(int freq)
{
	// 设置8253定时芯片。把计数器通道0设置成每隔50ms向中断控制器发送一个中断请求信号。
	outportb(0x43, 0x36);// 控制字:设置通道0工作在方式3、计数初值采用二进制。
						 // 8253芯片控制字寄存器写端口。
	uint16 count = 1193180 / freq; //20HZ
	outportb(0x40, count & 0xff);
	outportb(0x40, (count >> 8) & 0xff);
}
