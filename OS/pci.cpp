#include "PCI.h"
#include "io.h"
#include "stdio.h"
#include "list.h"

static LIST<DEVICE> sys_devices;

PCI::PCI()
{
}


PCI::~PCI()
{
}

uint32_t PCI::read_byte(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset)
{
	uint32_t id = ((bus) << 16) | ((slot) << 11) | ((func) << 8);
	uint32_t addr = 0x80000000 | id | (offset & 0xfc);
	outportd(0xCF8, addr);
	uint32 val = inportb(0xCFC + (offset & 0x03));
	return val;
}

uint32_t PCI::read_word(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset)
{
	uint32_t id = ((bus) << 16) | ((slot) << 11) | ((func) << 8);
	uint32_t addr = 0x80000000 | id | (offset & 0xfc);
	outportd(0xCF8, addr);
	uint32 val = inportw(0xCFC + (offset & 0x02));
	return val;
}

uint32_t PCI::read_dword(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset)
{
	uint32_t id = ((bus) << 16) | ((slot) << 11) | ((func) << 8);
	uint32_t addr = 0x80000000 | id | (offset & 0xfc);
	outportd(0xCF8, addr);
	uint32 val = inportd(0xCFC);
	return val;
}

void PCI::write_byte(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint32_t val)
{
	uint32_t id = ((bus) << 16) | ((slot) << 11) | ((func) << 8);
	uint32_t addr = 0x80000000 | id | (offset & 0xfc);
	outportd(0xCF8, addr);
	outportb(0xCFC + (offset & 0x03),val);
}

void PCI::write_word(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint32_t val)
{
	uint32_t id = ((bus) << 16) | ((slot) << 11) | ((func) << 8);
	uint32_t addr = 0x80000000 | id | (offset & 0xfc);
	outportd(0xCF8, addr);
	outportb(0xCFC + (offset & 0x02), val);
}

void PCI::write_dword(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint32_t val)
{
	uint32_t id = ((bus) << 16) | ((slot) << 11) | ((func) << 8);
	uint32_t addr = 0x80000000 | id | (offset & 0xfc);
	outportd(0xCF8, addr);
	outportb(0xCFC, val);
}

uint32_t PCI::get_device_type(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset)
{
	return
		getBaseClass(bus, slot, func) << 16 |
		getSubClass(bus, slot, func) << 8 |
		getProgIF(bus, slot, func);
}

void PCI::scan_devices()
{
	int device_count = 0;
	for (uint32_t bus = 0; bus < 256; bus++)
	{
		for (uint32_t slot = 0; slot < 32; slot++)
		{
			for (uint32_t function = 0; function < 8; function++)
			{
				uint32_t vendor = getVendorID(bus, slot, function);
				if (vendor == 0xffff) break;
			
				uint32_t device = getDeviceID(bus, slot, function);
				uint32_t Class= getBaseClass(bus, slot, function);
				uint32_t Subclass = getSubClass(bus, slot, function);
				printf("%d %02X:%02X:%X vendor: %x device: %x class: %x subclass: %x \n", 
					device_count++, bus, slot,function, vendor, device, Class, Subclass);
				uint32_t header_type = getHeaderType(bus, slot, function);
				if (header_type & 0x80 == 0) break;

				/*pci_device *pdev = (pci_device *)malloc(sizeof(pci_device));
				pdev->vendor = vendor;
				pdev->device = device;
				pdev->func = function;
				pdev->driver = 0;
				add_pci_device(pdev);*/
			}
		}
	}
}

uint32_t PCI::pciCheckVendor(uint32_t bus, uint32_t slot)
{
	uint32_t vendor, device;
	/* check if device is valid */
	if ((vendor = read_word(bus, slot, 0, 0)) != 0xFFFF)
	{
		device = read_word(bus, slot, 0, 2);
		/* valid device */
	} return (vendor);
}
