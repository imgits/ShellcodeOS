#pragma once
template<class T>
class LIST
{
private:
	T	m_list_head;
	T   m_list_tail;
public:
	LIST()
	{
		m_list_head = NULL;
		m_list_tail = NULL;
	}

	~LIST()
	{
	}

	T   push_back(T obj)
	{
		return obj;
	}

	T   push_front(T obj)
	{
		return obj;
	}

	T   find(T obj)
	{
		return obj;
	}

	bool Remove(T obj)
	{
		return true;
	}


};

