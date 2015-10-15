/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
//#include <ntoskrnl.h>
//#include <msvcrt/ctype.h>
//#include <stdio.h>
#include <wchar.h>

#undef isxdigit
int isxdigit(int c)
{
 return _isctype(c,_HEX);
}

#undef iswxdigit
int iswxdigit(wint_t c)
{
 return iswctype(c,_HEX);
}

