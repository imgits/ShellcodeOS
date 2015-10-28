#pragma once
#include "typedef.h"

//http://wiki.osdev.org/Spinlock
class SPIN_LOCK
{
private:
	uint32 m_lock;
public:
	SPIN_LOCK()
	{
		m_lock = 0;
	}

	void lock()
	{
		__asm	acquire_lock:
		__asm	lock	bts	dword ptr [ecx], 0;//Attempt to acquire the lock(in case lock is uncontended)
		__asm	jnc		acquire_lock_ok

		__asm	spin_with_pause:
		__asm	pause	//Tell CPU we're spinning
		__asm	test	dword ptr[ecx], 1// Is the lock free ?
		__asm	jnz		spin_with_pause// no, wait
		__asm	jmp		acquire_lock// retry

		__asm   acquire_lock_ok:
		__asm	ret
	}
	
	void unlock()
	{
		m_lock = 0;
	}

	bool is_locked()
	{
		return m_lock != 0;

	}

};

