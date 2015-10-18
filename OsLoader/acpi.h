#include <stdint.h>
#include "typedef.h"

#define MAX_CPU_COUNT 16

#define BIOS32_SERVICE_ADDR_LOW            pkv_(0x000E0000)
#define BIOS32_SERVICE_ADDR_HIGH            pkv_(0x000FFFFF)

#pragma pack(push,1)
// ------------------------------------------------------------------------------------------------
struct ACPI_HEADER
{
	u32		signature;
	u32		length;
	u8		revision;
	u8		checksum;
	u8		oem[6];
	u8		oemTableId[8];
	u32		oemRevision;
	u32		creatorId;
	u32		creatorRevision;
};

struct ACPI_RSDT : ACPI_HEADER
{
	ACPI_HEADER* DescriptionTable[1];
};

struct ACPI_XSDT : ACPI_HEADER
{
	u64 DescriptionTable[1];
};

//http://wiki.osdev.org/FADT
struct GenericAddressStructure
{
	uint8_t AddressSpace;
	uint8_t BitWidth;
	uint8_t BitOffset;
	uint8_t AccessSize;
	uint64_t Address;
};

struct ACPI_FADT : ACPI_HEADER
{
	//ACPI_HEADER header;
	u32 firmwareControl;
	u32 dsdt;
	u8 reserved;
	u8 preferredPMProfile;
	u16 sciInterrupt;
	u32 smiCommandPort;
	u8 acpiEnable;
	u8 acpiDisable;
	uint32_t FirmwareCtrl;
	uint32_t Dsdt;

	// field used in ACPI 1.0; no longer in use, for compatibility only
	uint8_t  Reserved;

	uint8_t  PreferredPowerManagementProfile;
	uint16_t SCI_Interrupt;
	uint32_t SMI_CommandPort;
	uint8_t  AcpiEnable;
	uint8_t  AcpiDisable;
	uint8_t  S4BIOS_REQ;
	uint8_t  PSTATE_Control;
	uint32_t PM1aEventBlock;
	uint32_t PM1bEventBlock;
	uint32_t PM1aControlBlock;
	uint32_t PM1bControlBlock;
	uint32_t PM2ControlBlock;
	uint32_t PMTimerBlock;
	uint32_t GPE0Block;
	uint32_t GPE1Block;
	uint8_t  PM1EventLength;
	uint8_t  PM1ControlLength;
	uint8_t  PM2ControlLength;
	uint8_t  PMTimerLength;
	uint8_t  GPE0Length;
	uint8_t  GPE1Length;
	uint8_t  GPE1Base;
	uint8_t  CStateControl;
	uint16_t WorstC2Latency;
	uint16_t WorstC3Latency;
	uint16_t FlushSize;
	uint16_t FlushStride;
	uint8_t  DutyOffset;
	uint8_t  DutyWidth;
	uint8_t  DayAlarm;
	uint8_t  MonthAlarm;
	uint8_t  Century;

	// reserved in ACPI 1.0; used since ACPI 2.0+
	uint16_t BootArchitectureFlags;

	uint8_t  Reserved2;
	uint32_t Flags;

	// 12 byte structure; see below for details
	GenericAddressStructure ResetReg;

	uint8_t  ResetValue;
	uint8_t  Reserved3[3];

	// 64bit pointers - Available on ACPI 2.0+
	uint64_t                X_FirmwareControl;
	uint64_t                X_Dsdt;

	GenericAddressStructure X_PM1aEventBlock;
	GenericAddressStructure X_PM1bEventBlock;
	GenericAddressStructure X_PM1aControlBlock;
	GenericAddressStructure X_PM1bControlBlock;
	GenericAddressStructure X_PM2ControlBlock;
	GenericAddressStructure X_PMTimerBlock;
	GenericAddressStructure X_GPE0Block;
	GenericAddressStructure X_GPE1Block;
};

//http://wiki.osdev.org/MADT
struct ACPI_MADT : ACPI_HEADER
{
	//ACPI_HEADER header;
	u32 localApicAddr;
	u32 flags;
};

// ------------------------------------------------------------------------------------------------
struct APIC_HEADER
{
	u8 type;
	u8 length;
};

// APIC structure types
#define APIC_TYPE_LOCAL_APIC            0
#define APIC_TYPE_IO_APIC               1
#define APIC_TYPE_INTERRUPT_OVERRIDE    2

// ------------------------------------------------------------------------------------------------
struct LOCAL_APIC : APIC_HEADER
{
	//APIC_HEADER header;
	u8 acpiProcessorId;
	u8 apicId;
	u32 flags;
};

// ------------------------------------------------------------------------------------------------
struct IO_APIC : APIC_HEADER
{
	//APIC_HEADER header;
	u8 ioApicId;
	u8 reserved;
	u32 ioApicAddress;
	u32 globalSystemInterruptBase;
};

// ------------------------------------------------------------------------------------------------
struct APIC_INT_OVERRIDE : APIC_HEADER
{
	//APIC_HEADER header;
	u8 bus;
	u8 source;
	u32 interrupt;
	u16 flags;
};

struct ACPI_RSDP10 
{
	char		Signature[8];
	uint8		Checksum;
	char		OEMID[6];
	uint8		Revision;
	uint32		RsdtAddress; //32-bit physical (I repeat: physical) address of the RSDT table. 
};

struct ACPI_RSDP20 : ACPI_RSDP10
{
	uint32		Length;
	uint64		XsdtAddress;//64-bit physical address of the XSDT table.
	uint8		ExtendedChecksum;
	uint8		Reserved[3];
};

#pragma pack(pop)

class ACPI
{
public:
	ACPI();
	~ACPI();
	void Init();
	bool ParseRSDP(ACPI_RSDP10* rsdp);
	void ParseRSDT(ACPI_RSDT *rsdt);
	void ParseXSDT(ACPI_XSDT *xsdt);
	void ParseDT(ACPI_HEADER *header);
	void ParseFADT(ACPI_FADT *fadt);
	void ParseAPIC(ACPI_MADT *madt);
};
