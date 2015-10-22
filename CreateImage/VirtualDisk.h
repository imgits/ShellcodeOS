#pragma once
#include <Windows.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <atlconv.h>

#define DEFIND_GUID
#include <initguid.h>
#include <virtdisk.h>
#pragma comment(lib, "VirtDisk.lib")

#define PHYS_PATH_LEN 1024+1

class VirtualDisk
{
	HANDLE m_hVHD;
	WCHAR  m_PhysicalDiskPath[PHYS_PATH_LEN];

public:
	VirtualDisk()
	{
		m_hVHD = INVALID_HANDLE_VALUE;
	}

	~VirtualDisk()
	{
		Detach();
		if (m_hVHD != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hVHD);
		}
	}
public: 
	bool CreateFixed(char* path, ULONGLONG size, VIRTUAL_DISK_ACCESS_MASK accessMask = VIRTUAL_DISK_ACCESS_ALL, PSECURITY_DESCRIPTOR securityDescriptor=NULL)
	{
		assert(INVALID_HANDLE_VALUE == m_hVHD);
		assert(0 != path);
		assert(0 == size % 512);
		USES_CONVERSION;
		WCHAR* wPath = A2W(path);

		VIRTUAL_STORAGE_TYPE VirtualStorageType =
		{
			VIRTUAL_STORAGE_TYPE_DEVICE_VHD,
			VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT
		};
		CREATE_VIRTUAL_DISK_PARAMETERS parameters =
		{
			CREATE_VIRTUAL_DISK_VERSION_1
		};
		parameters.Version1.MaximumSize = size;
		parameters.Version1.BlockSizeInBytes = CREATE_VIRTUAL_DISK_PARAMETERS_DEFAULT_BLOCK_SIZE;
		parameters.Version1.SectorSizeInBytes = CREATE_VIRTUAL_DISK_PARAMETERS_DEFAULT_SECTOR_SIZE;
		parameters.Version1.SourcePath = NULL;
		DWORD ret = ::CreateVirtualDisk(
			&VirtualStorageType, 
			wPath,
			accessMask, 
			securityDescriptor, 
			CREATE_VIRTUAL_DISK_FLAG_FULL_PHYSICAL_ALLOCATION,
			0, // no provider-specific flags
			&parameters, 
			NULL, 
			&m_hVHD);
		if (ret == ERROR_SUCCESS) return true;
		PrintErrorMessage(ret);
		return false;
	}

	bool Open(char* path, VIRTUAL_DISK_ACCESS_MASK accessMask = VIRTUAL_DISK_ACCESS_ALL)
	{
		assert(INVALID_HANDLE_VALUE == m_hVHD);
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
			&m_hVHD);
		if (ret == ERROR_SUCCESS) return true;
		PrintErrorMessage(ret);
		return false;
	}

	void Close()
	{
		if (m_hVHD != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hVHD);
			m_hVHD = INVALID_HANDLE_VALUE;
		}
	}

	bool Attach()
	{
		assert(INVALID_HANDLE_VALUE != m_hVHD);
		ATTACH_VIRTUAL_DISK_PARAMETERS iparams;
		iparams.Version = ATTACH_VIRTUAL_DISK_VERSION_1;
		DWORD ret = AttachVirtualDisk(m_hVHD, NULL,
			//ATTACH_VIRTUAL_DISK_FLAG_NO_DRIVE_LETTER |
			ATTACH_VIRTUAL_DISK_FLAG_PERMANENT_LIFETIME, 
			0, &iparams, NULL);
		if (ret == ERROR_SUCCESS) return true;
		PrintErrorMessage(ret);
		return false;
	}

	//http://stackoverflow.com/questions/24396644/programmatically-mount-a-microsoft-virtual-hard-drive-vhd
	bool Attach(char* VolumeMountPoint)
	{
		assert(INVALID_HANDLE_VALUE != m_hVHD);
		ATTACH_VIRTUAL_DISK_PARAMETERS iparams;
		iparams.Version = ATTACH_VIRTUAL_DISK_VERSION_1;
		DWORD ret = ERROR_SUCCESS;
		ret = AttachVirtualDisk(m_hVHD, NULL,
			ATTACH_VIRTUAL_DISK_FLAG_NO_DRIVE_LETTER |
			ATTACH_VIRTUAL_DISK_FLAG_PERMANENT_LIFETIME,
			0, &iparams, NULL);
		if (ret != ERROR_SUCCESS)
		{
			PrintErrorMessage(ret);
			return false;
		}

		if (!GetPhysicalPath()) return false;
		int len = wcslen(m_PhysicalDiskPath);
		DWORD DriverNumber = m_PhysicalDiskPath[len - 1] - '0';
		return MountVolume(VolumeMountPoint, DriverNumber);
	}

	bool Detach()
	{
		assert(INVALID_HANDLE_VALUE != m_hVHD);
		DWORD ret = DetachVirtualDisk(m_hVHD, DETACH_VIRTUAL_DISK_FLAG_NONE, 0);
		if (ret == ERROR_SUCCESS) return true;
		//PrintErrorMessage(ret);
		return false;
	}

	WCHAR* GetPhysicalPath()
	{
		assert(INVALID_HANDLE_VALUE != m_hVHD);
		ULONG sizePhysicalDisk = sizeof(m_PhysicalDiskPath);
		memset(m_PhysicalDiskPath, 0, sizeof(m_PhysicalDiskPath));
		DWORD ret = GetVirtualDiskPhysicalPath(m_hVHD, &sizePhysicalDisk, m_PhysicalDiskPath);
		if (ret == ERROR_SUCCESS) return m_PhysicalDiskPath;
		PrintErrorMessage(ret);
		return NULL;
	}

private:

	bool MountVolume(char* VolumeMountPoint, DWORD driveNumber)
	{
		char volumeName[MAX_PATH];
		DWORD bytesReturned;
		VOLUME_DISK_EXTENTS diskExtents;
		HANDLE hFileVolume = INVALID_HANDLE_VALUE;
		HANDLE hFindVolume = FindFirstVolume(volumeName, sizeof(volumeName));
		if (hFindVolume == INVALID_HANDLE_VALUE)
		{
			PrintErrorMessage(GetLastError());
			return false;
		}

		bool hadTrailingBackslash = false;
		bool MountResult = false;
		do 
		{
			printf("%s\n", volumeName);
			// I had a problem where CreateFile complained about the trailing \ and
			// SetVolumeMountPoint desperately wanted the backslash there. I ended up 
			// doing this to get it working but I'm not a fan and I'd greatly 
			// appreciate it if someone has any further info on this matter
			int backslashPos = strlen(volumeName) - 1;
			if (hadTrailingBackslash = volumeName[backslashPos] == '\\') 
			{
				volumeName[backslashPos] = 0;
			}
			hFileVolume = CreateFile(volumeName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if (hFileVolume == INVALID_HANDLE_VALUE)
			{
				PrintErrorMessage(GetLastError());
				break;
			}

			if (!DeviceIoControl(hFileVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL,
				0, &diskExtents, sizeof(diskExtents), &bytesReturned, NULL))
			{
				PrintErrorMessage(GetLastError());
				break;
			}

			// If the volume were to span across multiple physical disks, you'd find 
			// more than one Extents here but we don't have to worry about that with VHD
			// Note that 'driveNumber' would be the integer you extracted out of 
			// 'physicalDrive' in the previous snippet
			if (diskExtents.Extents[0].DiskNumber == driveNumber)
			{
				if (hadTrailingBackslash) 
				{
					volumeName[backslashPos] = '\\';
				}

				// Found volume that's on the VHD, let's mount it with a letter of our choosing.
				// Warning: SetVolumeMountPoint requires elevation
				if (!(MountResult = SetVolumeMountPoint(VolumeMountPoint, volumeName)))
				{
					PrintErrorMessage(GetLastError());
				}
				break;
			}
			CloseHandle(hFileVolume);
			hFileVolume = INVALID_HANDLE_VALUE;
		} while (FindNextVolume(hFindVolume, volumeName, sizeof(volumeName)));
		FindVolumeClose(hFindVolume);
		if (hFileVolume!= INVALID_HANDLE_VALUE) CloseHandle(hFileVolume);
		return MountResult;
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

};

