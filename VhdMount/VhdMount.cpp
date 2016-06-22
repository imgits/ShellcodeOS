// VhdMount.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <atlconv.h>

#define DEFIND_GUID
#include <initguid.h>
#include <virtdisk.h>
#pragma comment(lib, "VirtDisk.lib")

int  help()
{
	printf(
		"Usage:\n"
		"    VhdMount /a vhdfile\n"
		"    VhdMount /d vhdfile\n"
	);
	return 0;
}

void PrintErrorMessage(ULONG ErrorId)
{
	char* Message = NULL;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		ErrorId,
		0,
		(LPSTR)&Message,
		16,
		NULL);
	printf("%s\n", Message);
	LocalFree(Message);
}

HANDLE OpenVhd(char* path, VIRTUAL_DISK_ACCESS_MASK accessMask = VIRTUAL_DISK_ACCESS_ALL)
{
	HANDLE hVHD = NULL;
	assert(0 != path);
	USES_CONVERSION;
	WCHAR* wPath = A2W(path);
	VIRTUAL_STORAGE_TYPE VirtualStorageType =
	{
		VIRTUAL_STORAGE_TYPE_DEVICE_VHD,
		VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT
	};
	OPEN_VIRTUAL_DISK_PARAMETERS oparams;
	oparams.Version = OPEN_VIRTUAL_DISK_VERSION_1;
	oparams.Version1.RWDepth = OPEN_VIRTUAL_DISK_RW_DEPTH_DEFAULT;

	DWORD ret = OpenVirtualDisk(
		&VirtualStorageType,
		wPath,
		accessMask,
		OPEN_VIRTUAL_DISK_FLAG_NONE,
		&oparams,
		&hVHD);
	if (ret == ERROR_SUCCESS) return hVHD;
	PrintErrorMessage(ret);
	return NULL;
}

bool AttachVhd(HANDLE hVHD)
{
	ATTACH_VIRTUAL_DISK_PARAMETERS iparams;
	iparams.Version = ATTACH_VIRTUAL_DISK_VERSION_1;
	DWORD ret = AttachVirtualDisk(hVHD, NULL,
		//ATTACH_VIRTUAL_DISK_FLAG_NO_DRIVE_LETTER |
		ATTACH_VIRTUAL_DISK_FLAG_PERMANENT_LIFETIME,
		0, &iparams, NULL);
	if (ret != ERROR_SUCCESS)
	{
		PrintErrorMessage(ret);
		return false;
	}
	return true;
}

bool DetachVhd(HANDLE hVHD)
{
	DWORD ret = DetachVirtualDisk(hVHD, DETACH_VIRTUAL_DISK_FLAG_NONE, 0);
	if (ret != ERROR_SUCCESS)
	{
		PrintErrorMessage(ret);
		return false;
	}
	return true;
}


void CloseVhd(HANDLE hVHD)
{
	if (hVHD != NULL)
	{
		CloseHandle(hVHD);
	}
}

int main(int argc, char** argv)
{
	if (argc != 3)  return help();
	HANDLE hVHD = OpenVhd(argv[2]);
	if (hVHD == NULL) return help();
	if (_stricmp(argv[1], "/a") == 0)
	{
		if (AttachVhd(hVHD)) printf("Mount %s OK!\n", argv[2]);
	}
	else if (_stricmp(argv[1], "/d") == 0)
	{
		if (DetachVhd(hVHD)) printf("Unmount %s OK!\n", argv[2]);
	}
	else help();
    return 0;
}

