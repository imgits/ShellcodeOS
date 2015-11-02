#include "paging.h"
#include <string.h>
#include <stdio.h>
#include "kernel.h"
#include "syscall.h"
#include "gdt.h"
#include <intrin.h>

bool SYSCALL::Init()
{
	// Set code segment of kernel
	__writemsr(IA32_SYSENTER_CS, GDT_TO_SEL(GDT_KERNEL_CODE));

	// Stack to use for syscalls
	//__writemsr(IA32_SYSENTER_ESP, (uint32_t)&syscall_stack, 0);

	// Syscall handler to jump to for syscalls
	__writemsr(IA32_SYSENTER_ESP, (uint32)SYSCALL::syscall_enter);
	
	return true;
}

void SYSCALL::syscall_enter()
{

}
