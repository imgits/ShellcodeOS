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
	m_mmu = NULL;
}

ACPI::~ACPI()
{

}

//http://wiki.osdev.org/RSDP
void ACPI::Init(MMU* mmu)
{
	m_mmu = mmu;
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
			//m_mmu->map_memory(xsdtAddr, xsdtAddr, 4096, PT_PRESENT | PT_WRITABLE);
			ParseXSDT((ACPI_XSDT*)xsdtAddr);
			//m_mmu->unmap_memory(xsdtAddr, 4096);
		}
		else
		{
			//m_mmu->map_memory(rsdtAddr, rsdtAddr, 4096, PT_PRESENT | PT_WRITABLE);
			ParseRSDT((ACPI_RSDT*)rsdtAddr);
			//m_mmu->unmap_memory(rsdtAddr, 4096);
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
		switch (type)
		{
		case APIC_TYPE_LOCAL_APIC:
		{
			LOCAL_APIC *s = (LOCAL_APIC *)p;

			printf("Found CPU: %d %d %x\n", s->acpiProcessorId, s->apicId, s->flags);
			if (g_acpiCpuCount < MAX_CPU_COUNT)
			{
				g_acpiCpuIds[g_acpiCpuCount] = s->apicId;
				++g_acpiCpuCount;
			}
			break;
		}
		case APIC_TYPE_IO_APIC:
		{
			IO_APIC *s = (IO_APIC *)p;

			printf("Found I/O APIC: %d 0x%08x %d\n", s->ioApicId, s->ioApicAddress, s->globalSystemInterruptBase);
			g_ioApicAddr = (u8 *)(uintptr_t)s->ioApicAddress;
			break;
		}
		case APIC_TYPE_INTERRUPT_OVERRIDE:
		{
			APIC_INT_OVERRIDE *s = (APIC_INT_OVERRIDE *)p;
			printf("Found Interrupt Override: %d %d %d 0x%04x\n", s->bus, s->source, s->interrupt, s->flags);
			break;
		}
		default:
		{
			printf("Unknown APIC structure %d\n", type);
		}
		}

		p += length;
	}
}




