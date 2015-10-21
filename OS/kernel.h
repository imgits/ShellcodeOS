#pragma once
#include "typedef.h"
#include "stdio.h"
#include "intrin.h"

#define panic(fmt,...) { printf("%s::%s() line %d:%s\n",__FILE__,__FUNCTION__,__LINE, fmt, __VA_ARGS__); __asm jmp $;}

