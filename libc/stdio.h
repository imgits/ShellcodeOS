#pragma once
typedef char* va_list;
//#undef printf
extern "C" int __cdecl printf(const char *fmt, ...);
extern "C" int __cdecl sprintf(char *buf, const char *fmt, ...);
extern "C" int __cdecl vsprintf(char *buf, const char *fmt, va_list args);