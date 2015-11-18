#include "disk.h"
#include "io.h"
#include "stdio.h"

DISK::DISK()
{
}


DISK::~DISK()
{
}

bool DISK::Init(bool primary, bool master)
{
	if (primary)
	{
		m_bus = ATA_PRIMARY_BUS;
		m_control = ATA_PRIMARY_CONTROL;
	}
	else
	{
		m_bus = ATA_SECONDARY_BUS;
		m_control = ATA_SECONDARY_CONTROL;
	}
	m_master = master;
	return true;
}

void DISK::soft_reset() 
{
	outportb(m_control, 0x04);
	outportb(m_control, 0x00);
}

void DISK::io_wait()
{
	inportb(m_bus + ATA_REG_ALTSTATUS);
	inportb(m_bus + ATA_REG_ALTSTATUS);
	inportb(m_bus + ATA_REG_ALTSTATUS);
	inportb(m_bus + ATA_REG_ALTSTATUS);
}

ATADEV_TYPE DISK::detect_device_type()
{
	soft_reset();
	if (m_master) outportb(m_bus + ATA_REG_HDDEVSEL, 0xA0);
	else outportb(m_bus + ATA_REG_HDDEVSEL, 0xB0);
	io_wait();

	unsigned char cl = inportb(m_bus + ATA_REG_LBA1); /* CYL_LO */
	unsigned char ch = inportb(m_bus + ATA_REG_LBA2); /* CYL_HI */

	printf("ATA device type %02X%02X\n",ch,cl);
													  /* differentiate ATA, ATAPI, SATA and SATAPI */
	if (cl == 0x14 && ch == 0xEB) return ATADEV_PATAPI;
	if (cl == 0x69 && ch == 0x96) return ATADEV_SATAPI;
	if (cl == 0 && ch == 0) return ATADEV_PATA;
	if (cl == 0x3c && ch == 0xc3) return ATADEV_SATA;
	return ATADEV_UNKNOWN;
}
