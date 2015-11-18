#pragma once
#include "typedef.h"
#include "idt.h"

#pragma pack(push, 1)
struct X86_PUSH32
{
	byte	push32_opcode;
	uint32	push32_oprand;
};

struct INT_ENTRY_STUB
{
	byte cli;
	byte cld;
	byte push_error_code;	uint32 error_code;
	byte push_int_no;		uint32 int_no;
	byte push_int_handler;	uint32 int_handler;
	byte pushad;
	byte push_ds;
	byte push_es;
	byte push_fs[2];
	byte push_gs[2];
	byte mov_ax_0x10[4];
	byte mov_ds_ax[3];
	byte mov_es_ax[3];
	byte mov_fs_ax[3];
	byte mov_gs_ax[3];
	byte push_esp[3];
	byte mov_eax_int_dispatch[5];
	byte call_eax[2];
	byte add_esp_0x04[3];
	byte pop_gs[2];
	byte pop_fs[2];
	byte pop_es;
	byte pop_ds;
	byte popad;
	byte add_esp_number[3];
	byte sti;
	byte iretd;
};

class INT_HANDLER;

struct INT_CONTEXT
{
	/*00*/uint32	gs;
	/*04*/uint32	fs;
	/*08*/uint32	es;
	/*0C*/uint32	ds;
	/*10*/uint32	edi;
	/*14*/uint32	esi;
	/*18*/uint32	ebp;
	/*1C*/uint32	esp__;
	/*20*/uint32	ebx;
	/*24*/uint32	edx;
	/*28*/uint32	ecx;
	/*2C*/uint32	eax;
	/*30*/INT_HANDLER* handler_obj;
	/*34*/uint32	int_no;
	/*38*/uint32    error_code;
	/*3C*/uint32	eip;
	/*40*/uint32	cs;
	/*44*/uint32    eflags;
	/*48*/uint32    esp;
	/*4C*/uint32    ss;
};
#pragma pack(pop)

class INT_HANDLER
{
protected:
	INT_ENTRY_STUB m_int_entry_stub;
public:
	INT_HANDLER();
	void	Register(IDT* idt, int int_no, bool has_error_code=false);
private:
	static  void int_dispatch(INT_CONTEXT* context);
	virtual void int_handler(INT_CONTEXT* context) =0;
};


