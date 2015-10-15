#include <stdint.h>
#include "typedef.h"

#define MAX_CPU_COUNT 16

#pragma pack(push,1)
// ------------------------------------------------------------------------------------------------
struct AcpiHeader
{
	u32 signature;
	u32 length;
	u8 revision;
	u8 checksum;
	u8 oem[6];
	u8 oemTableId[8];
	u32 oemRevision;
	u32 creatorId;
	u32 creatorRevision;
};

// ------------------------------------------------------------------------------------------------
struct AcpiFadt
{
	AcpiHeader header;
	u32 firmwareControl;
	u32 dsdt;
	u8 reserved;
	u8 preferredPMProfile;
	u16 sciInterrupt;
	u32 smiCommandPort;
	u8 acpiEnable;
	u8 acpiDisable;
	// TODO - fill in rest of data
};

// ------------------------------------------------------------------------------------------------
struct AcpiMadt
{
	AcpiHeader header;
	u32 localApicAddr;
	u32 flags;
};

// ------------------------------------------------------------------------------------------------
struct ApicHeader
{
	u8 type;
	u8 length;
};

// APIC structure types
#define APIC_TYPE_LOCAL_APIC            0
#define APIC_TYPE_IO_APIC               1
#define APIC_TYPE_INTERRUPT_OVERRIDE    2

// ------------------------------------------------------------------------------------------------
struct ApicLocalApic
{
	ApicHeader header;
	u8 acpiProcessorId;
	u8 apicId;
	u32 flags;
};

// ------------------------------------------------------------------------------------------------
struct ApicIoApic
{
	ApicHeader header;
	u8 ioApicId;
	u8 reserved;
	u32 ioApicAddress;
	u32 globalSystemInterruptBase;
};

// ------------------------------------------------------------------------------------------------
struct ApicInterruptOverride
{
	ApicHeader header;
	u8 bus;
	u8 source;
	u32 interrupt;
	u16 flags;
};

#pragma pack(pop)

void AcpiInit();