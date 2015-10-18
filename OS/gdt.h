#pragma once

#define GDT_TO_SEL(gdt) ((gdt) << 3)

#define GDT_NULL			0
#define GDT_KERNEL_CODE		1
#define GDT_KERNEL_DATA		2
#define GDT_USER_CODE		3
#define GDT_USER_DATA		4
#define GDT_SYS_TSS			5
#define GDT_TIB				6
#define GDT_AUX1			7
#define GDT_APM40			8
#define GDT_APMCS			9
#define GDT_APMCS16			10
#define GDT_APMDS			11
#define GDT_PNPTEXT			12
#define GDT_PNPDATA			13
#define GDT_PNPTHUNK		14
#define GDT_AUX2			15

#define GDT_MAX		16

// Each descriptor should have exactly one of next 8 codes to define the type of descriptor

#define D_LDT   0x02  // LDT segment
#define D_TASK  0x05  // Task gate
#define D_TSS   0x09  // TSS
#define D_CALL  0x0C  // 386 call gate
#define D_INT   0x0E  // 386 interrupt gate
#define D_TRAP  0x0F  // 386 trap gate

#define D_DATA  0x10  // Data segment
#define D_CODE  0x18  // Code segment

// Descriptors may include the following as appropriate:

#define D_DPL3         0x60   // DPL3 or mask for DPL
#define D_DPL2         0x40   // DPL2 or mask for DPL
#define D_DPL1         0x20   // DPL1 or mask for DPL
#define D_DPL0         0x00   // DPL0 or mask for DPL

#define D_PRESENT      0x80   // Present

// Segment descriptors (not gates) may include:

#define D_ACCESSED 0x1  // Accessed (data or code)

#define D_WRITE    0x2  // Writable (data segments only)
#define D_EXDOWN   0x4  // Expand down (data segments only)

#define D_READ     0x2  // Readable (code segments only)
#define D_CONFORM  0x4  // Conforming (code segments only)

#define D_BUSY     0x2  // Busy (TSS only)

// Granularity flags

#define D_BIG      0x4    // Default to 32 bit mode
#define D_BIG_LIM  0x8    // Limit is in 4K units


#pragma pack(push, 1)

struct segment 
{
	unsigned short limit_low;      // Limit 0..15
	unsigned short base_low;       // Base  0..15
	unsigned char base_med;        // Base  16..23
	unsigned char access;          // Access byte
	unsigned char limit_high;      // Limit 16..19 + Granularity << 4
	unsigned char base_high;       // Base 24..31
};

struct gate {
	unsigned short offset_low;   // Offset 0..15
	unsigned short selector;     // Selector
	unsigned char notused;
	unsigned char access;        // Access flags
	unsigned short offset_high;  // Offset 16..31
};

struct desc {
	unsigned long low;
	unsigned long high;
};

union dte {
	struct segment segment;
	struct gate gate;
	struct desc desc;
};

struct selector {
	unsigned short limit;
	void *dt;
};

struct fullptr {
	unsigned long offset;
	unsigned short segment;
};

#pragma pack(pop)


class GDT
{
private:
	segment m_gdt[GDT_MAX];
public:
	GDT();
	~GDT();
	void init_seg(struct segment *seg, unsigned long addr, unsigned long size, int access, int granularity);
};

