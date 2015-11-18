#include "idt.h"
#include "gdt.h"
#include "stdio.h"
#include <string.h>
#include "intrin.h"
#include "int_handler.h"
#include "c++.h"
#include "8259.h"
#include "kernel.h"

#define   ERROR_CODE			0x12345678
#define   INT_NO				0x12345678
#define   INT_HANDLER_OBJECT	0x12345678

static void  __declspec(naked) INT_ENTRY()
{
	__asm   cli
	__asm	cld
	__asm      push        ERROR_CODE
	__asm      push        INT_NO
	__asm      push        INT_HANDLER_OBJECT
	__asm pushad
	__asm push ds
	__asm push es
	__asm push fs
	__asm push gs
	__asm mov  ax, SEL_KERNEL_DATA
	__asm mov  ds, ax
	__asm mov  es, ax
	__asm mov  fs, ax
	__asm mov  gs, ax
	__asm push esp
	__asm mov  eax,INT_HANDLER::int_dispatch 
	__asm call eax
	__asm add  esp, 4
	__asm pop  gs
	__asm pop  fs
	__asm pop  es
	__asm pop  ds
	__asm popad
	__asm add  esp, 12
	__asm sti
	__asm iretd 
	//千万注意iret和iretd的区别，iret被编译成iretw
	//查找此错误杀死了无路脑细胞
}

/*
00924120 FA                   cli
00924121 FC                   cld
00924122 68 78 56 34 12       push        12345678h
00924127 68 78 56 34 12       push        12345678h
0092412C 68 78 56 34 12       push        12345678h
00924131 60                   pushad
00924132 1E                   push        ds
00924133 06                   push        es
00924134 0F A0                push        fs
00924136 0F A8                push        gs
00924138 66 B8 10 00          mov         ax,10h
0092413C 66 8E D8             mov         ds,ax
0092413F 66 8E C0             mov         es,ax
00924142 66 8E E0             mov         fs,ax
00924145 66 8E E8             mov         gs,ax
00924148 54                   push        esp
00924149 B8 00 41 92 00       mov         eax,924100h
0092414E FF D0                call        eax
00924150 83 C4 04             add         esp,4
00924153 0F A9                pop         gs
00924155 0F A1                pop         fs
00924157 07                   pop         es
00924158 1F                   pop         ds
00924159 61                   popad
0092415A 83 C4 0C             add         esp,0Ch
0092415D FB                   sti
0092415E CF                   iretd
*/

INT_HANDLER::INT_HANDLER()
{
	memset(&m_int_entry_stub, 0xCC, sizeof(m_int_entry_stub));
}

void	INT_HANDLER::Register(IDT* idt, int int_no, bool has_error_code)
{
	memcpy(&m_int_entry_stub, INT_ENTRY, sizeof(m_int_entry_stub));
	m_int_entry_stub.int_handler = (uint32)this;
	m_int_entry_stub.int_no = int_no;

	if (has_error_code)
	{
		m_int_entry_stub.push_error_code = 0x90;
		m_int_entry_stub.error_code = 0x90909090;
	}
	idt->set_intr_gate(int_no, &m_int_entry_stub); //因为有虚函数，所以 &m_int_entry_stub !=this
	if (int_no >= PIC_INT_FIRST && int_no <= PIC_INT_LAST)
	{
		PIC::enable_irq(int_no - PIC_INT_FIRST);
	}
}

void INT_HANDLER::int_dispatch(INT_CONTEXT* context)
{
	context->handler_obj->int_handler(context);
	if (context->int_no >= PIC_INT_FIRST && context->int_no <= PIC_INT_LAST)
	{
		//发送中断结束命令EOI（0x20)
		outportb(PIC1_CMD_PORT, 0x20);//向主片发送中断结束命令
		outportb(PIC2_CMD_PORT, 0x20);//向从片发送中断结束命令
	}
}

