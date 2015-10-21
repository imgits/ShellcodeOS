#include "8253.h"

static PIT g_pit;

PIT::PIT()
{
	m_counter = 0;
	m_timer = 0;
}

PIT::~PIT()
{
}

bool PIT::Init(PIC* pic)
{
	m_counter = 0;
	m_timer = 0;

	// 设置8253定时芯片。把计数器通道0设置成每隔50ms向中断控制器发送一个中断请求信号。
	outportb(0x43, 0x36);// 控制字:设置通道0工作在方式3、计数初值采用二进制。
						 // 8253芯片控制字寄存器写端口。
	uint16 count = PIT_CLOCK / TIMER_FREQ; //20HZ
	outportb(0x40, count & 0xff);
	outportb(0x40, (count >> 8) & 0xff);

	pic->register_irq(IRQ_PIT, PIT::pit_irq_handler);
	return true;
}

void	PIT::pit_irq_handler(IRQ_CONTEXT* context)
{
	g_pit.irq_handler(context);
}

void	PIT::irq_handler(IRQ_CONTEXT* context)
{
	if (++m_counter >= 10)
	{
		m_timer++;
	}
}

bool PIT::register_timer(uint32 id, uint32 period, TIMER_HANDLER handler)
{
	int free_item = -1;
	for (int i = 0; i < MAX_PIT_HANDLERS; i++)
	{
		if (m_handlers[i].id == id)
		{
			m_handlers[i].period = period;
			m_handlers[i].handler = handler;
			m_handlers[i].counter = 0;
			return true;
		}
		if (m_handlers[i].handler == NULL && free_item == -1 ) free_item = i;
	}
	if (free_item == -1) return false;
	m_handlers[free_item].id = id;
	m_handlers[free_item].period = period;
	m_handlers[free_item].counter = 0;
	m_handlers[free_item].handler = handler;
	return true;
}

//http://wiki.osdev.org/RTC
//void PIC::init_cmos_timer()
//{
//	//设置和时钟中断相关的硬件 
//	outportb(0x70, 0x0b | 0x80); // RTC寄存器B | 阻断NMI
//	outportb(0x71, 0x12); //设置寄存器B，禁止周期性中断，开放更新结束后中断，BCD码，24小时制
//	outportb(0x70, 0x0c); //读RTC寄存器C，复位未决的中断状态
//	inportb(0x71);
//}

