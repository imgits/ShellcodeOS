// VhdCopy.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include "../CreateImage/VirtualDisk.h"

bool GUI_error = false;

void exit_error(const char *fmt, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	int len = 0;
	len = vsprintf(buf, fmt, args);
	va_end(args);
	if (len > sizeof(buf)) len = sizeof(buf) - 1;
	buf[len] = 0;
	if (GUI_error) MessageBox(NULL, buf, NULL, IDOK);
	else printf("%s\n", buf);
	exit(GetLastError());
}

int  help()
{
	printf(
		"Usage:\n"
		"    VhdCopy vhdfile e:\\fromfile vhd:\\tofile [-g]\n"
		"    VhdCopy vhdfile vhd:\\fromfile e:\\tofile [-g]\n"
		" -g Show error with MessageBox\n"
		);
	return 0;
}

int main(int argc, char** argv)
{
	if (argc == 4)
	{
		printf("VhdCopy %s %s %s\n", argv[1], argv[2], argv[3]);
		//return help();
	}
	else if (argc == 5 && _stricmp(argv[4],"-g")==0)
	{
		printf("VhdCopy %s %s %s %s\n", argv[1], argv[2], argv[3], argv[4]);
		GUI_error = true;
	}
	else
	{
		help();
		exit_error("Bad command line arguments\n");
	}
	VirtualDisk vhd;
	if (!vhd.Open(argv[1]))
	{
		help();
		exit_error("Open VHD file \"%s\" failed\n", argv[1]);
	}
	else if (!vhd.Attach())
	{
		help();
		exit_error("Attach VHD file \"%s\" failed\n", argv[1]);
	}
	char VhdVolume = vhd.AttachedVolumeName();
	if (VhdVolume == 0)
	{
		help();
		exit_error("Unknown attached VHD volume name\n");
	}

	char *fromfile = NULL, *tofile = NULL;
	if (_memicmp(argv[2], "vhd:\\", 5) == 0)
	{
		fromfile = argv[2] + 2;
		fromfile[0] = VhdVolume;
		tofile = argv[3];
	}
	else if (_memicmp(argv[3], "vhd:\\", 5) == 0)
	{
		fromfile = argv[2];
		tofile = argv[3] + 2;
		tofile[0] = VhdVolume;
	}
	else
	{
		help();
		exit_error("Bad command line arguments\n");
	}
	BOOL ret = CopyFile(fromfile, tofile, false);
	if (!ret)
	{
		exit_error("CopyFile failed with error %08X\n", GetLastError());
	}
	return 0;
}

