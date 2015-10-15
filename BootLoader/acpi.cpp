// ------------------------------------------------------------------------------------------------
// acpi/acpi.c
// ------------------------------------------------------------------------------------------------
//#include <stdio.h>
#include "acpi.h"
#include "string.h"
#include "vga.h"
// ------------------------------------------------------------------------------------------------
// Globals
uint g_acpiCpuCount;
u8 g_acpiCpuIds[MAX_CPU_COUNT];

u8 *g_localApicAddr = NULL;
u8 *g_ioApicAddr = NULL;
// ------------------------------------------------------------------------------------------------
static AcpiMadt *s_madt;

// ------------------------------------------------------------------------------------------------
static void AcpiParseFacp(AcpiFadt *facp)
{
	if (facp->smiCommandPort)
	{
		//printf("Enabling ACPI\n");
		//IoWrite8(facp->smiCommandPort, facp->acpiEnable);

		// TODO - wait for SCI_EN bit
	}
	else
	{
		printf("ACPI already enabled\n");
	}
}

// ------------------------------------------------------------------------------------------------
static void AcpiParseApic(AcpiMadt *madt)
{
	s_madt = madt;

	printf("Local APIC Address = 0x%08x\n", madt->localApicAddr);
	g_localApicAddr = (u8 *)(uintptr_t)madt->localApicAddr;

	u8 *p = (u8 *)(madt + 1);
	u8 *end = (u8 *)madt + madt->header.length;

	while (p < end)
	{
		ApicHeader *header = (ApicHeader *)p;
		u8 type = header->type;
		u8 length = header->length;

		if (type == APIC_TYPE_LOCAL_APIC)
		{
			ApicLocalApic *s = (ApicLocalApic *)p;

			printf("Found CPU: %d %d %x\n", s->acpiProcessorId, s->apicId, s->flags);
			if (g_acpiCpuCount < MAX_CPU_COUNT)
			{
				g_acpiCpuIds[g_acpiCpuCount] = s->apicId;
				++g_acpiCpuCount;
			}
		}
		else if (type == APIC_TYPE_IO_APIC)
		{
			ApicIoApic *s = (ApicIoApic *)p;

			printf("Found I/O APIC: %d 0x%08x %d\n", s->ioApicId, s->ioApicAddress, s->globalSystemInterruptBase);
			g_ioApicAddr = (u8 *)(uintptr_t)s->ioApicAddress;
		}
		else if (type == APIC_TYPE_INTERRUPT_OVERRIDE)
		{
			ApicInterruptOverride *s = (ApicInterruptOverride *)p;

			printf("Found Interrupt Override: %d %d %d 0x%04x\n", s->bus, s->source, s->interrupt, s->flags);
		}
		else
		{
			printf("Unknown APIC structure %d\n", type);
		}

		p += length;
	}
}

// ------------------------------------------------------------------------------------------------
static void AcpiParseDT(AcpiHeader *header)
{
	u32 signature = header->signature;

	char sigStr[5];
	memcpy(sigStr, &signature, 4);
	sigStr[4] = 0;
	printf("%s 0x%x\n", sigStr, signature);

	if (signature == 0x50434146)
	{
		AcpiParseFacp((AcpiFadt *)header);
	}
	else if (signature == 0x43495041)
	{
		AcpiParseApic((AcpiMadt *)header);
	}
}

// ------------------------------------------------------------------------------------------------
static void AcpiParseRsdt(AcpiHeader *rsdt)
{
	u32 *p = (u32 *)(rsdt + 1);
	u32 *end = (u32 *)((u8*)rsdt + rsdt->length);

	while (p < end)
	{
		u32 address = *p++;
		AcpiParseDT((AcpiHeader *)(uintptr_t)address);
	}
}

// ------------------------------------------------------------------------------------------------
static void AcpiParseXsdt(AcpiHeader *xsdt)
{
	u64 *p = (u64 *)(xsdt + 1);
	u64 *end = (u64 *)((u8*)xsdt + xsdt->length);

	while (p < end)
	{
		u64 address = *p++;
		AcpiParseDT((AcpiHeader *)(uintptr_t)address);
	}
}

// ------------------------------------------------------------------------------------------------
static bool AcpiParseRsdp(u8 *p)
{
	// Parse Root System Description Pointer
	printf("RSDP found at %08X\n",p);

	// Verify checksum
	u8 sum = 0;
	for (uint i = 0; i < 20; ++i) 
	{
		sum += p[i];
	}
	if (sum)
	{
		printf("Checksum failed\n");
		return false;
	}

	// Print OEM
	char oem[7];
	memcpy(oem, p + 9, 6);
	oem[6] = '\0';
	printf("OEM = %s\n", oem);

	// Check version
	u8 revision = p[15];
	if (revision == 0)
	{
		printf("Version 1\n");

		u32 rsdtAddr = *(u32 *)(p + 16);
		AcpiParseRsdt((AcpiHeader *)(uintptr_t)rsdtAddr);
	}
	else if (revision == 2)
	{
		printf("Version 2\n");

		u32 rsdtAddr = *(u32 *)(p + 16);
		u64 xsdtAddr = *(u64 *)(p + 24);

		if (xsdtAddr)
		{
			AcpiParseXsdt((AcpiHeader *)(uintptr_t)xsdtAddr);
		}
		else
		{
			AcpiParseRsdt((AcpiHeader *)(uintptr_t)rsdtAddr);
		}
	}
	else
	{
		printf("Unsupported ACPI version %d\n", revision);
	}
	return true;
}

// ------------------------------------------------------------------------------------------------
void AcpiInit()
{
	printf("Search main BIOS area below 1MB\n");
	u8 *p = (u8 *)0x000e0000;
	u8 *end = (u8 *)0x000fffff;
	while (p < end)
	{
		u64 signature = *(u64 *)p;

		if (signature == 0x2052545020445352) // 'RSD PTR '
		{
			if (AcpiParseRsdp(p))
			{
				break;
			}
		}
		p += 16;
	}
}

// ------------------------------------------------------------------------------------------------
uint AcpiRemapIrq(uint irq)
{
	AcpiMadt *madt = s_madt;

	u8 *p = (u8 *)(madt + 1);
	u8 *end = (u8 *)madt + madt->header.length;

	while (p < end)
	{
		ApicHeader *header = (ApicHeader *)p;
		u8 type = header->type;
		u8 length = header->length;

		if (type == APIC_TYPE_INTERRUPT_OVERRIDE)
		{
			ApicInterruptOverride *s = (ApicInterruptOverride *)p;

			if (s->source == irq)
			{
				return s->interrupt;
			}
		}

		p += length;
	}

	return irq;
}
