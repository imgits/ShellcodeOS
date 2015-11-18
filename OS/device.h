#pragma once
#include <typedef.h>
#include <stdint.h>

class DEVICE
{
public:
//private:
	uint16_t m_vendor;
	uint16_t m_device;
	uint8_t  m_class;
	uint8_t  m_subclass;

	uint32_t m_bus;
	uint32_t m_slot;
	uint16_t m_func;

	uint32_t m_membase;
	uint32_t m_memsize;
	uint32_t m_iobase;
	uint32_t m_irq;

	void *m_data;
	//struct pci_driver *drv;
public:
	DEVICE();
	~DEVICE();
};

