//http://wiki.osdev.org/PCI
//https://msdn.microsoft.com/en-us/library/ms903537.aspx

#pragma once
#include "typedef.h"
#include <stdint.h>
#include "device.h"

#define		PCI_CONFIG_ADDRESS	0xCF8
#define		PCI_CONFIG_DATA		0xCFC

#define getVendorID(bus, device, function)		read_word(bus, device, function, 0x00)
#define getVendorID(bus, device, function)		read_word(bus, device, function, 0x00)
#define getDeviceID(bus, device, function)		read_word(bus, device, function, 0x02)
#define getCommand(bus, device, function)		read_word(bus, device, function, 0x04)
#define getStatus(bus, device, function)		read_word(bus, device, function, 0x06)
#define getRevisionID(bus, device, function)	read_byte(bus, device, function, 0x08)
#define getProgIF(bus, device, function)		read_byte(bus, device, function, 0x09)
#define getBaseClass(bus, device, function)		read_byte(bus, device, function, 0x0A)
#define getSubClass(bus, device, function)		read_byte(bus, device, function, 0x0B)
#define getCacheLineSize(bus, device, function)	read_byte(bus, device, function, 0x0C)
#define getLatencyTimer(bus, device, function)	read_byte(bus, device, function, 0x0D)
#define getHeaderType(bus, device, function)	read_byte(bus, device, function, 0x0E)
#define getBIST(bus, device, function)			read_byte(bus, device, function, 0x0F)

enum PCI_DEVICE_CLASS
{
	VGA_DEVICE=0,//VGA-Compatible Device
	MASS_STORAGE_DEVICE=1,//Mass Storage Controller
	NETWORK_DEVICE=2,//Network Controller
	MULTIMEDIA_DEVICE=3,//Multimedia Device
	MEMORY_DEVICE=4,//Memory Controller
	BRIDGE_DEVICE=5,//Bridge Device
	COMMUNICATIONS_DEVICE=6,//Communications Device
	SYSTEM_DEVICE=7,//System Peripheral
	INPUT_DEVICE=8,//Input Controller
	DOCKING_DEVICE=9,//Docking Station
	PROCESSOR_DEVICE=10,//Processor
	BUS_DEVICE=11,//USB 1394 ...
	WIRELESS_DEVICE=12,//Wireless Controller
	FIFO_DEVICE=13,//I20 FIFO
	AV_DEVICE=14,//TV Controller
	ENCRYPTION_DEVICE=15,//Encryption / Decryption
	DSP_DEVICE=16
};

class PCI
{
public:
	PCI();
	~PCI();
	void	 scan_devices();
	uint32_t read_byte(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset);
	uint32_t read_word(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset);
	uint32_t read_dword(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset);

	void	 write_byte(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint32_t val);
	void	 write_word(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint32_t val);
	void	 write_dword(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset, uint32_t val);

	uint32_t get_device_type(uint32_t bus, uint32_t slot, uint32_t func, uint32_t offset);

	uint32_t getBAR0(uint32_t bus, uint32_t device, uint32_t function)
	{
		return read_dword(bus, device, function, 0x10);
	}
	uint32_t getBAR1(uint32_t bus, uint32_t device, uint32_t function)
	{
		return read_dword(bus, device, function, 0x14);
	}
	uint32_t getBAR2(uint32_t bus, uint32_t device, uint32_t function)
	{
		return read_dword(bus, device, function, 0x18);
	}
	uint32_t getBAR3(uint32_t bus, uint32_t device, uint32_t function)
	{
		return read_dword(bus, device, function, 0x1C);
	}
	uint32_t getBAR4(uint32_t bus, uint32_t device, uint32_t function)
	{
		return read_dword(bus, device, function, 0x20);
	}
	uint32_t getBAR5(uint32_t bus, uint32_t device, uint32_t function)
	{
		return read_dword(bus, device, function, 0x24);
	}

	uint32_t pciCheckVendor(uint32_t bus, uint32_t slot);
};

