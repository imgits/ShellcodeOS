#include "tss.h"
#include "stdio.h"
#include <string.h>

TSS::TSS()
{
	//在程序管理器的TSS中设置必要的项目 
	memset(this, 0, sizeof(TSS)); //反向链=0
}

void TSS::Init(GDT * gdt)
{
	m_back_link = 0;
	//m_esp0 = KERNEL_STACK_TOP;
	m_ss0 = GDT_KERNEL_CODE;
	m_cr3 = CR3();//登记CR3(PDBR)
	m_eip = 0;//(uint32)UserProcess;
	m_eflags = 0x00000202;

	m_ldt = 0;//没有LDT。处理器允许没有LDT的任务
	m_trap = 0;
	m_iobase = 103;//没有I/O位图。0特权级事实上不需要。
				   //创建程序管理器的TSS描述符，并安装到GDT中 
	printf("TSS Init() OK\n");
	Register(gdt);
}

TSS::~TSS()
{
}

bool   TSS::Register(GDT * gdt)
{
	uint16 tss_seg = gdt->add_tss_entry((uint32)this, 103);
	m_tss_seg = tss_seg;
	//任务寄存器TR中的内容是任务存在的标志，该内容也决定了当前任务是谁。
	//下面的指令为当前正在执行的0特权级任务“程序管理器”后补手续（TSS）。
	__asm	mov	ax, tss_seg //index = 5
	__asm	ltr		ax
	printf("TSS Register() OK\n");
}