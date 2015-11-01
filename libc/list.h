#pragma once
#include "c++.h"
//链表模板元素必须包含T* prev;T* next;
template<class T>
class LIST
{
private:
	struct NODE
	{
		T*  obj;
		NODE* prev;
		NODE* next;
		NODE() { obj = NULL; prev = next = NULL; }
		NODE(T* _obj) { obj = _obj; prev = next = NULL; }
	};
private:
	NODE	m_head;
public:
	LIST()
	{
		m_head.prev = m_head.next = &m_head;
	}

	~LIST()
	{
		clear();
	}

	bool is_empty()
	{
		return m_head.next == &m_head;
	}

	bool   insert_head(T* obj)
	{
		NODE* node = new NODE(obj);
		if (node == NULL) return false;

		//得到第1个结点的指针
		//为空链表时也满足
		NODE* first = m_head.next;

		//在头结点和第1个结点之间插入时需要:
		//  头结点的prior不变;头结点的next指向新结点
		//  新结点的prior指向头结点;新结点的next指向第1个结点
		//  第1个结点的prior指向新结点;第1个结点的next不变
		m_head.next = node;
		node->prev = &m_head;
		node->next = first;
		first->prev = node;

		return true;
	}

	bool insert_tail(T* obj)
	{
		NODE* node = new NODE(obj);
		if (node == NULL) return false;

		//得到最后一个结点的指针
		//为空链表时也满足
		NODE* last = m_head.prev;

		//在最后一个结点和头结点之间插入时需要:
		//  最后一个结点的next指向新结点;最后一个结点的prior不变
		//  新结点的prior指向最后一个结点;新结点的next指向头结点
		//  头结点的next不变;头结点的prior指向新结点
		last->next = node;
		node->prev = last;
		node->next = &m_head;
		m_head.prev = node;

		return true;
	}

	bool  remove_head()
	{
		if (is_empty()) return false;

		//需要移除第1个结点,则应该先保存第2个结点
		//若不存在第2个结点也满足条件(此时second即为头结点)
		NODE* second = m_head.next->next;
		NODE* removed = m_head.next;

		//移除第1个结点需要:
		//  头结点的next指向第2个结点(若不存在,则指向自己);头结点的prior不变
		//  第2个结点的prior指向头结点;第2个结点的next不变
		m_head.next = second;
		second->prev = &m_head;

		delete removed;
		return true;
	}

	bool  remove_tail()
	{
		if (is_empty()) return false;

		//需要移除最后一个结点,需要保存倒数第2个结点指针
		//若不存在倒数第2个(仅一个结点时),倒数第2个就是头结点
		NODE* second_last = m_head.prev->prev;
		NODE* removed = m_head.prev;
		
		//移除一个结点需要
		//  倒数第2个结点的next指向头结点,prior不变
		//  头结点的prior指向倒数第2个结点,next不变
		second_last->next = &m_head;
		m_head.prev = second_last;
		
		delete removed;
		return true;
	}

	bool  remove(T* obj)
	{
		NODE* node = find(obj);
		if (node == NULL) return false;

		//移除该结点需要:
		// 将当前结点的上一个结点:
		//      next指向当前结点的next
		// 将当前结点的下一个结点:
		//      prior指向当前结点的上一个结点
		node->prev->next = node->next;
		node->next->prev = node->prev;

		delete node;
		return true;
	}

	void clear()
	{
		NODE* node = m_head.next;
		while (node != &m_head)
		{
			node = node->next;
			delete node->prev;
		}
	}

	T*    head()
	{
		return (m_head.next == &m_head) ? NULL : m_head.next->obj;
	}

	T*    tail()
	{
		return (m_head.prev == &m_head) ? NULL : m_head.prev->obj;
	}

	T*    next(T* obj)
	{
		for (NODE* node = m_head.next; node != &m_head; node = node->next)
		{
			if (node->data == obj)
			{
				return (node->next == &m_head) ? NULL : node->next->obj;
			}
		}
		return NULL;
	}

	T*    prev(T* obj)
	{
		for (NODE* node = m_head.next; node != &m_head; node = node->next)
		{
			if (node->data == obj)
			{
				return (node->prev == &m_head) ? NULL : node->prev->obj;
			}
		}
		return NULL;
	}

private:
	NODE* find(T* obj)
	{
		for (NODE* node = m_head.next; node != &m_head; node = node->next)
		{
			if (node->data == obj) return node;
		}
		return NULL;
	}

};

