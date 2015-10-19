#pragma once
#include  "typedef.h"

#pragma pack(push, 1)
// √≈√Ë ˆ∑˚
typedef struct GATE_DESC
{
	uint16		offset_low;
	uint16		selector;
	uint8		dcount;
	uint8		attr;
	uint16		offset_high;
}GATE_DESC;

struct IDTR
{
	uint16 limit;
	void*  base;
};

#pragma pack(pop)

#define		MAX_IDT_NUM		256

class IDT
{
private:
	GATE_DESC	m_idt[MAX_IDT_NUM];	//	256 gdt items
public:
	IDT(void);
	~IDT(void);
	void Init();
	void set_idt_entry(int vector, uchar desc_type, void* handler);
};

