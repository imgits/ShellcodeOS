#pragma once
#include "typedef.h"

#define GDT_TO_SEL(gdt) ((gdt) << 3)

#define GDT_NULL			0
#define GDT_KERNEL_CODE		1
#define GDT_KERNEL_DATA		2
#define GDT_USER_CODE		3
#define GDT_USER_DATA		4
#define GDT_USER_FS			5
#define GDT_SYS_TSS			6
#define GDT_TIB				7
#define GDT_AUX1			8
#define GDT_APM40			9
#define GDT_APMCS			10
#define GDT_APMCS16			11
#define GDT_APMDS			12
#define GDT_PNPTEXT			13
#define GDT_PNPDATA			14
#define GDT_PNPTHUNK		15
#define GDT_AUX2			16

#define MAX_GDT_NUM				32

/* 描述符类型值说明 */
#define	DA_16			0x0000	/* 16 位段				*/
#define	DA_32			0x4000	/* 32 位段				*/
#define	DA_LIMIT_4K		0x8000	/* 段界限粒度为 4K 字节			*/

#define	DA_DPL0			0x00	/* DPL = 0				*/
#define	DA_DPL1			0x20	/* DPL = 1				*/
#define	DA_DPL2			0x40	/* DPL = 2				*/
#define	DA_DPL3			0x60	/* DPL = 3				*/

/* 存储段描述符类型值说明 */
#define	DA_DR			0x90	/* 存在的只读数据段类型值		*/
#define	DA_DRW			0x92	/* 存在的可读写数据段属性值		*/
#define	DA_DRWA			0x93	/* 存在的已访问可读写数据段类型值	*/
#define	DA_C			0x98	/* 存在的只执行代码段属性值		*/
#define	DA_CR			0x9A	/* 存在的可执行可读代码段属性值		*/
#define	DA_CCO			0x9C	/* 存在的只执行一致代码段属性值		*/
#define	DA_CCOR			0x9E	/* 存在的可执行可读一致代码段属性值	*/

/* 系统段描述符类型值说明 */
#define	DA_LDT			0x82	/* 局部描述符表段类型值			*/
#define	DA_TaskGate		0x85	/* 任务门类型值				*/
#define	DA_386TSS		0x89	/* 可用 386 任务状态段类型值		*/
#define	DA_386CGate		0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		0x8F	/* 386 陷阱门类型值			*/

/* 选择子类型值说明 */
/* 其中, SA_ : Selector Attribute */
#define	SA_RPL_MASK	0xFFFC
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3

#define	SA_TI_MASK	0xFFFB
#define	SA_TIG		0
#define	SA_TIL		4

#define    USER_FS_BASE				0X7EFDD000

#pragma pack(push, 1)
//段描述符
typedef struct SEGMENT_DESC
{
	uint16		    limit_low;
	uint16		    base_low;
	uint8			base_mid;
	uint8			attr1;
	uint8			limit_high_attr2;
	uint8			base_high;
}SEGMENT_DESC;

struct GDTR
{
	uint16		limit;
	void*		base;
};
#pragma pack(pop)

class GDT
{
private:
	SEGMENT_DESC m_gdt[MAX_GDT_NUM];
private:
	uint32 GDT::find_free_gdt_entry();
	bool set_gdt_entry(uint32 vector, uint32 base, uint32 limit, uint32 attribute);
public:
	GDT();
	~GDT();
	void	Init();
	uint32  add_gdt_entry(uint32 base, uint32 limit, uint32 attribute);
	uint32  add_ldt_entry(uint32 base, uint32 limit, uint32 attribute);
	uint32  add_tss_entry(uint32 base, uint32 limit);
	uint32  add_call_gate(void* handler, int argc);

};

