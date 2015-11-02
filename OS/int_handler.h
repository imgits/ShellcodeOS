#pragma once
#include "typedef.h"

#pragma pack(push, 1)
struct ASM_PUSH32
{
	byte		push32_opcode;
	uint32	push32_oprand;
};

struct INT_STUB
{
	byte cli;
	ASM_PUSH32 push_error_code;
	ASM_PUSH32 push_int_no;
	ASM_PUSH32 push_int_handler;
	ASM_PUSH32 push_int_dispatch;
	byte ret;
};

class INT_HANDLER;

struct INT_CONTEXT
{
	uint32	edi;
	uint32	esi;
	uint32	ebp;
	uint32	tmp;
	uint32	ebx;
	uint32	edx;
	uint32	ecx;
	uint32	eax;
	INT_HANDLER* int_handler;
	uint32	int_no;
	uint32  error_code;
	uint32	eip;
	uint32	cs;
	uint32  eflags;
	uint32  esp;
	uint32  ss;
};

class INT_HANDLER
{
	INT_STUB m_int_stub;
public:
	INT_HANDLER();
	static void dispatch(INT_CONTEXT* context);
	virtual 
		void handler(INT_CONTEXT* context);
};

#pragma pack(pop)
