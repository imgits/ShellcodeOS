#pragma once
// Constructor prototypes
// Call this function as soon as possible. Basically should be at the moment you
// jump into your C/C++ kernel. But keep in mind that kernel is not yet initialized,
// and you can't use a lot of stuff in your constructors!
//tp://wiki.osdev.org/Visual_C++_Runtime
bool CppInit();

void* __cdecl operator new(size_t size);
void* __cdecl operator new[](size_t size);
void __cdecl  operator delete(void *p);
void __cdecl  operator delete[](void *p);
void __cdecl  operator delete(void *p, unsigned int size);

inline void* __cdecl operator new(size_t size, void* address)
{
	return address;
}

static int __cdecl _purecall()
{
	return 0;
}

//#include <typeinfo.h>

// provide symbol:
// type_info::'vftable' ["const type_info::`vftable'" (??_7type_info@@6B@)].
// needed when compiling classes with virtual methods with /GR 
class type_info
{
public:
	//type_info(const type_info& rhs)
	//{
	//}

	//type_info& operator=(const type_info& rhs)
	//{
	//	return *this;
	//}

	virtual ~type_info()
	{
	}
};