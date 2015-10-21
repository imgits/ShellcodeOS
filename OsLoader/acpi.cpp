// ------------------------------------------------------------------------------------------------
// acpi/acpi.c
// ------------------------------------------------------------------------------------------------
#include "stdio.h"
#include "acpi.h"
#include "string.h"

// ------------------------------------------------------------------------------------------------
// Globals
uint g_acpiCpuCount;
u8 g_acpiCpuIds[MAX_CPU_COUNT];

u8 *g_localApicAddr = NULL;
u8 *g_ioApicAddr = NULL;
// ------------------------------------------------------------------------------------------------
static ACPI_MADT *s_madt;

ACPI::ACPI()
{

}

ACPI::~ACPI()
{

}

//http://wiki.osdev.org/RSDP
void ACPI::Init()
{
	printf("Search main BIOS area below 1MB\n");
	u8 *p = (u8 *)0x000e0000;
	u8 *end = (u8 *)0x000fffff;
	while (p < end)
	{
		u64 signature = *(u64 *)p;
		if (signature == 0x2052545020445352) // 'RSD PTR '
		{
			if (ParseRSDP((ACPI_RSDP10*)p))
			{
				break;
			}
		}
		p += 16;
	}
}

//http://wiki.osdev.org/RSDP
bool ACPI::ParseRSDP(ACPI_RSDP10* rsdp)
{
	// Parse Root System Description Pointer
	printf("RSDP found at %08X\n", rsdp);

	// Verify checksum
	u8 sum = 0;
	for (uint i = 0; i < 20; ++i)
	{
		sum += ((u8*)rsdp)[i];
	}
	if (sum)
	{
		printf("Checksum failed\n");
		return false;
	}

	// Print OEM
	char oem[7];
	memcpy(oem, rsdp->OEMID, 6);
	oem[6] = '\0';
	printf("OEM = %s\n", oem);

	// Check version
	if (rsdp->Revision == 0)
	{
		printf("Version 1\n");

		u32 rsdtAddr = rsdp->RsdtAddress;
		ParseRSDT((ACPI_RSDT*)rsdtAddr);
	}
	else if (rsdp->Revision == 2)
	{
		ACPI_RSDP20* rsdp20 = (ACPI_RSDP20*)rsdp;

		u32 rsdtAddr = rsdp20->RsdtAddress;
		u64 xsdtAddr = rsdp20->XsdtAddress;

		printf("Version 3 rsdtAddr=%X xsdtAddr=%llX \n", rsdtAddr, xsdtAddr);

		if (xsdtAddr)
		{
			ParseXSDT((ACPI_XSDT*)xsdtAddr);
		}
		else
		{
			ParseRSDT((ACPI_RSDT*)rsdtAddr);
		}
	}
	else
	{
		printf("Unsupported ACPI version %d\n", rsdp->Revision);
	}
	return true;
}

//http://wiki.osdev.org/RSDT
void ACPI::ParseRSDT(ACPI_RSDT *rsdt)
{
	int entries = (rsdt->length - sizeof(ACPI_HEADER)) / 4;
	printf("ParseRSDT entries=%d\n", entries);
	for (int i = 0; i < entries; i++)
	{
		ACPI_HEADER* dt = rsdt->DescriptionTable[i];
		ParseDT(dt);
	}
}

//http://wiki.osdev.org/XSDT
void ACPI::ParseXSDT(ACPI_XSDT *xsdt)
{
	int entries = (xsdt->length - sizeof(ACPI_HEADER)) / 8;
	printf("ParseXSDT entries=%d\n", entries);
	for (int i = 0; i < entries; i++)
	{
		ACPI_HEADER* dt = (ACPI_HEADER*)xsdt->DescriptionTable[i];
		ParseDT(dt);
	}
}

//
void ACPI::ParseDT(ACPI_HEADER *header)
{
	u32 signature = header->signature;

	char sigStr[5];
	memcpy(sigStr, &signature, 4);
	sigStr[4] = 0;
	printf("%s 0x%x\n", sigStr, signature);

	if (signature == 'PCAF') //0x50434146) 
	{
		ParseFADT((ACPI_FADT *)header);
	}
	else if (signature == 'CIPA') //0x43495041)
	{
		ParseAPIC((ACPI_MADT *)header);
	}
}

// ------------------------------------------------------------------------------------------------
void ACPI::ParseFADT(ACPI_FADT *fadt)
{
	if (fadt->smiCommandPort)
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

//http://wiki.osdev.org/MADT
void ACPI::ParseAPIC(ACPI_MADT *madt)
{
	s_madt = madt;

	printf("Local APIC Address = 0x%08x\n", madt->localApicAddr);
	g_localApicAddr = (u8 *)(uintptr_t)madt->localApicAddr;

	u8 *p = (u8 *)(madt + 1);
	u8 *end = (u8 *)madt + madt->length;

	while (p < end)
	{
		APIC_HEADER *header = (APIC_HEADER *)p;
		u8 type = header->type;
		u8 length = header->length;
		IO_APIC *io_apic;
		LOCAL_APIC *local_apic;
		APIC_INT_OVERRIDE *apic_int_override;
		switch (type)
		{
		case APIC_TYPE_LOCAL_APIC:
			local_apic = (LOCAL_APIC *)p;
			printf("Found CPU: %d %d %x\n", local_apic->acpiProcessorId, local_apic->apicId, local_apic->flags);
			if (g_acpiCpuCount < MAX_CPU_COUNT)
			{
				g_acpiCpuIds[g_acpiCpuCount] = local_apic->apicId;
				++g_acpiCpuCount;
			}
			break;
		case APIC_TYPE_IO_APIC:
			io_apic = (IO_APIC *)p;
			printf("Found I/O APIC: %d 0x%08x %d\n", io_apic->ioApicId, io_apic->ioApicAddress, io_apic->globalSystemInterruptBase);
			g_ioApicAddr = (u8 *)(uintptr_t)io_apic->ioApicAddress;
			break;
		case APIC_TYPE_INTERRUPT_OVERRIDE:
			apic_int_override = (APIC_INT_OVERRIDE *)p;
			printf("Found Interrupt Override: %d %d %d 0x%04x\n", 
				apic_int_override->bus, apic_int_override->source, apic_int_override->interrupt, apic_int_override->flags);
			break;
		default:
			printf("Unknown APIC structure %d\n", type);
			break;
		}

		p += length;
	}
}




