#pragma once
#include  "typedef.h"
#include  "idt.h"
#include  "gdt.h"

#pragma pack(push, 1)
struct TRAP_CONTEXT
{
	uint32	gs, fs, es, ds;
	uint32	edi, esi, ebp, kesp, ebx, edx, ecx, eax;
	uint32	int_no, err_code;
	uint32	eip, cs, eflags, esp, ss;
};	//19*4=76 Bytes
#pragma pack(pop)

class TRAP
{
public:
	void Init(IDT* idt);
};

