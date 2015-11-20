// pcidatabase.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <list>
#include <map>

using namespace std;

map<uint64_t, string> vendor_name_list;
map<uint64_t, string> device_name_list;
map<uint64_t, string> subsys_name_list;

map<uint64_t, string> class_name_list;
map<uint64_t, string> subclass_name_list;
map<uint64_t, string> progif_name_list;

struct PCI_IDS
{
	uint32_t vendor;
	uint32_t device;
	uint32_t subvendor;
	uint32_t subdevice;
};

struct PCI_DEVICE_CLASS
{
	uint32_t baseclass;
	uint32_t subclass;
	uint32_t progif;
};

vector<PCI_IDS>		pci_device_list;
vector<PCI_DEVICE_CLASS> device_class_list;

char* process_string(char* str)
{
	while (*str == ' ') str++;
	for (int i = 0; str[i] != 0; i++)
	{
		if (str[i] == '\"') str[i] = '\'';
		else if (str[i] == '\\') str[i] = '/';
	}
	return str;
}

int main(int argc, char** argv)
{
	FILE* pci_ids = fopen("pci.ids", "rt");
	if (pci_ids == NULL)
	{
		printf("Open pci.ids failed\n");
		return 0;
	}
	FILE* pci_h = fopen("../OS/pci/pci_ids.h", "wt");
	if (pci_h == NULL)
	{
		printf("Create pci.h failed\n");
		return 0;
	}
	
	char line[1204];
	uint64_t vendor=0;
	uint64_t device=0;
	uint64_t subvendor=0;	
	uint64_t subdevice = 0;
	uint64_t key = 0;
	char* next=NULL;
	PCI_IDS pci_device;
	while (fgets(line, sizeof(line), pci_ids))
	{
		if (line[0] == 0 || line[0] == '\n' || line[0] == '#') continue;
		if (line[0] == 'C' && line[1] == ' ') break;
		int len = strlen(line);
		line[len - 1] = 0;
		if ((line[0] >= '0' && line[0] <= '9') ||
			(line[0] >= 'a' && line[0] <= 'f'))
		{
			vendor = strtol(line, &next, 16);
			vendor_name_list[vendor] = process_string(next);
			pci_device.vendor = vendor;
			pci_device.device = 0;
			pci_device.subvendor = 0;
			pci_device.subdevice = 0;
			pci_device_list.push_back(pci_device);
		}
		else if (line[0] == '\t' && line[1] != '\t')
		{
			if ((line[1] >= '0' && line[1] <= '9') ||
				(line[1] >= 'a' && line[1] <= 'f'))
			{
				next = NULL;
				device = strtol(line+1, &next, 16);
				key = (vendor << 16) | device;
				device_name_list[key] = process_string(next);;
				pci_device.vendor = vendor;
				pci_device.device = device;
				pci_device.subvendor = 0;
				pci_device.subdevice = 0;
				pci_device_list.push_back(pci_device);
			}
		}
		else if (line[0] == '\t' && line[1] == '\t')
		{
			if ((line[2] >= '0' && line[2] <= '9') ||
				(line[2] >= 'a' && line[2] <= 'f'))
			{
				next = NULL;
				subvendor = strtol(line+2, &next, 16);
				if ((line[7] >= '0' && line[7] <= '9') ||
						(line[7] >= 'a' && line[7] <= 'f'))
				{
					subdevice = strtol(line+7, &next, 16);
					key = (vendor << 48) | (device<<32) | (subvendor << 16) | subdevice;
					subsys_name_list[key] = process_string(next);;
					pci_device.vendor = vendor;
					pci_device.device = device;
					pci_device.subvendor = subvendor;
					pci_device.subdevice = subdevice;
					pci_device_list.push_back(pci_device);
				}
			}
		}
	}

	fprintf(pci_h,
		"#include <stdint.h>\n"
		"\n"
		);

	fprintf(pci_h,"\n//vendor name list\n");
	map<uint64_t, string>::iterator it = vendor_name_list.begin();
	for (;it != vendor_name_list.end(); it++)
	{
		fprintf(pci_h, "static char* vendor_%04X=\"%s\";\n", (uint32_t)it->first, it->second.c_str());
	}

	fprintf(pci_h, "\n//device name list\n");
	it = device_name_list.begin();
	for (;it != device_name_list.end(); it++)
	{
		fprintf(pci_h, "static char* device_%08X=\"%s\";\n", (uint32_t)it->first, it->second.c_str());
	}

	fprintf(pci_h, "\n//subsystem name list\n");
	it = subsys_name_list.begin();
	for (;it != subsys_name_list.end(); it++)
	{
		fprintf(pci_h, "static char* subsys_%016llX=\"%s\";\n", it->first, it->second.c_str());
	}

	fprintf(pci_h,
		"\n"
		"#pragma pack(push,1)\n"
		"struct PCI_IDS\n"
		"{\n"
		"    uint32_t vendor;\n"
		"    uint32_t device;\n"
		"    uint32_t subvendor;\n"
		"    uint32_t subdevice;\n"
		"    char* vendor_name;\n"
		"    char* device_name;\n"
		"    char* subsystem_name;\n"
		"};\n"
		"#pragma pack(pop)\n"
		"\n"
		"static PCI_IDS pci_database[%d]={\n",
		pci_device_list.size()
		);
	
	vector<PCI_IDS>::iterator pci = pci_device_list.begin();
	for (; pci != pci_device_list.end(); pci++)
	{
		if (pci->device == 0)
		{
			fprintf(pci_h, "{0x%04X,0x%04X,0x%04X,0x%04X,vendor_%04X,NULL,NULL},\n",
				pci->vendor, pci->device, pci->subvendor, pci->subdevice,
				pci->vendor);
		}
		else if (pci->subvendor == 0 && pci->subdevice == 0)
		{
			fprintf(pci_h, "{0x%04X,0x%04X,0x%04X,0x%04X,vendor_%04X,device_%04X%04X,NULL},\n",
				pci->vendor, pci->device, pci->subvendor, pci->subdevice,
				pci->vendor,
				pci->vendor, pci->device);
		}
		else
		{
			fprintf(pci_h, "{0x%04X,0x%04X,0x%04X,0x%04X,vendor_%04X,device_%04X%04X,subsys_%04X%04X%04X%04X},\n",
				pci->vendor, pci->device, pci->subvendor, pci->subdevice,
				pci->vendor,
				pci->vendor, pci->device,
				pci->vendor, pci->device, pci->subvendor, pci->subdevice);
		}
	}
	fprintf(pci_h, "};\n");

	//read baseclass && subclass
	uint32_t baseclass = 0;
	uint32_t subclass = 0;
	uint32_t progif = 0;
	PCI_DEVICE_CLASS device_class;
	do
	{
		if (line[0] == 0 || line[0] == '\n' || line[0] == '#') continue;
		int len = strlen(line);
		line[len - 1] = 0;
		if (line[0] == 'C' && line[1] == ' ')
		{
			baseclass = strtol(line + 2, &next, 16);
			class_name_list[baseclass] = process_string(next);

			device_class.baseclass = baseclass;
			device_class.subclass = 0;
			device_class.progif = 0;
			device_class_list.push_back(device_class);
		}
		else if (line[0] == '\t'&& line[1] != '\t')
		{
			subclass = strtol(line + 1, &next, 16);
			key = (baseclass << 8) | subclass;
			subclass_name_list[key] = process_string(next);;

			device_class.baseclass = baseclass;
			device_class.subclass = subclass;
			device_class.progif = 0;
			device_class_list.push_back(device_class);
		}
		else if (line[0] == '\t'&& line[1] == '\t')
		{
			progif = strtol(line + 2, &next, 16);
			key = (baseclass << 16) | (subclass << 8) | progif;
			progif_name_list[key] = process_string(next);

			device_class.baseclass = baseclass;
			device_class.subclass = subclass;
			device_class.progif = progif;
			device_class_list.push_back(device_class);

		}
	} while (fgets(line, sizeof(line), pci_ids));

	fprintf(pci_h, "\n//baseclass name list\n");
	it = class_name_list.begin();
	for (;it != class_name_list.end(); it++)
	{
		fprintf(pci_h, "static char* class_%02X=\"%s\";\n", (uint32_t)it->first, it->second.c_str());
	}

	fprintf(pci_h, "\n//subclass name list\n");
	it = subclass_name_list.begin();
	for (;it != subclass_name_list.end(); it++)
	{
		fprintf(pci_h, "static char* subclass_%04X=\"%s\";\n", (uint32_t)it->first, it->second.c_str());
	}

	fprintf(pci_h, "\n//progif name list\n");
	it = progif_name_list.begin();
	for (;it != progif_name_list.end(); it++)
	{
		fprintf(pci_h, "static char* progif_%06X=\"%s\";\n", (uint32_t)it->first, it->second.c_str());
	}


	fprintf(pci_h,
		"\n"
		"#pragma pack(push,1)\n"
		"struct PCI_DEVICE_CLASS\n"
		"{\n"
		"    uint32_t baseclass;\n"
		"    uint32_t subclass;\n"
		"    uint32_t progif;\n"
		"    char* baseclass_name;\n"
		"    char* subclass_name;\n"
		"    char* progif_name;\n"
		"};\n"
		"#pragma pack(pop)\n"
		"\n"
		"static PCI_DEVICE_CLASS pci_device_classes[%d]={\n",
		device_class_list.size()
		);

	vector<PCI_DEVICE_CLASS>::iterator it1 = device_class_list.begin();
	for (; it1 != device_class_list.end(); it1++)
	{
		if (it1->subclass == 0)
		{
			fprintf(pci_h, "{0x%02X,0x%02X,0x%02X,class_%02X,NULL,NULL},\n",
				it1->baseclass, it1->subclass, it1->progif,
				it1->baseclass);
		}
		else if (it1->progif == 0)
		{
			fprintf(pci_h, "{0x%02X,0x%02X,0x%02X,class_%02X,subclass_%02X%02X,NULL},\n",
				it1->baseclass, it1->subclass, it1->progif,
				it1->baseclass,
				it1->baseclass, it1->subclass);
		}
		else
		{
			fprintf(pci_h, "{0x%02X,0x%02X,0x%02X,class_%02X,subclass_%02X%02X,progif_%02X%02X%02X},\n",
				it1->baseclass, it1->subclass, it1->progif,
				it1->baseclass,
				it1->baseclass, it1->subclass,
				it1->baseclass, it1->subclass, it1->progif);
		}
	}
	fprintf(pci_h, "};\n");

	fprintf(pci_h,
		"\n"
		"static PCI_IDS* get_device_ids(uint32_t vendor, uint32_t device, uint32_t subvendor, uint32_t subdevice) \n"
		"{ \n"
		"	PCI_IDS* pci_ids = pci_database; \n"
		"	for (int i = 0; i < sizeof(pci_database) / sizeof(pci_database[0]); i++, pci_ids++) \n"
		"	{ \n"
		"		if (pci_ids->vendor == vendor && \n"
		"			pci_ids->device == device && \n"
		"			pci_ids->subvendor == subvendor && \n"
		"			pci_ids->subdevice == subdevice) \n"
		"		{ \n"
		"			return pci_ids; \n"
		"		} \n"
		"	} \n"
		"	return NULL; \n"
		"} \n");

	fprintf(pci_h,
		"\n"
		"static PCI_DEVICE_CLASS* get_device_class(uint32_t baseclass, uint32_t subclass, uint32_t progif) \n"
		"{ \n"
	    "   PCI_DEVICE_CLASS* device_class = pci_device_classes; \n"
		"   for (int i = 0; i < sizeof(device_class) / sizeof(device_class[0]); i++, device_class++) \n"
		"   { \n"
		"        if (device_class->baseclass == baseclass && \n"
		"            device_class->subclass == subclass && \n"
		"	         device_class->progif == progif) \n"
		"        { \n"
		"	        return device_class; \n"
		"        } \n"
	    "    } \n"
	    "    return NULL; \n"
        "} \n");


	fclose(pci_ids);
	fclose(pci_h);
    return 0;
}


