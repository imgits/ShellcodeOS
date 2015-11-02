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


//http://forum.osdev.org/viewtopic.php?t=14261
class Mutex 
{
protected:
	int     m_lock;
	char *  m_name;
public:
	Mutex(char *name=NULL) 
	{
		m_name = name;
		m_lock = 0;
	}

	void lock(void) 
	{
		//while (test_and_set(1, &m_lock)){}
	}

	void unlock(void) 
	{
		m_lock = 0;
	}
};


unsigned long fetch_and_increment(unsigned long *mem)
{
	unsigned long val = 1;
	//asm volatile ("lock; xadd %1, (%2)"
	//	: "=r"(val)
	//	: "r"(val), "r"(mem)
	//	: "%1", "memory");
	return val;
}

unsigned long entries = 0;
unsigned long exits = 0;

void lock()
{
	int ticket = fetch_and_increment(&entries);
	while (ticket != exits)
	{
		//schedule(); // or anything else you can do while waiting for your turn
	}
}

void unlock()
{
	exits = exits + 1;
}