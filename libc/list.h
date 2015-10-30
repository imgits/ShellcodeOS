#pragma once
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
		NODE() { obj = prev = next = NULL;}
		NODE(T* _obj) { obj = _obj; prev = next = NULL; }
	};
private:
	NODE*	m_head;
public:
	LIST()
	{
		m_head = NULL;
	}

	~LIST()
	{
	}

	bool push_back(T* obj)
	{
		if (m_head == NULL)
		{
			NODE* m_head = new NODE(obj);
			if (m_head == NULL) return false;
			m_head->prev = m_head;
			m_head->next = m_head;
		}
		else
		{
			NODE* node = new NODE(obj);
			if (node == NULL) return false;
			NODE* last_tail = m_head->prev;
			node->prev = last_tail;
			node->next = m_head;
			last_tail->next = node;
			m_head->prev = node;
		}
		return true;
	}

	bool   push_front(T* obj)
	{
		if (m_head == NULL)
		{
			NODE* m_head = new NODE(obj);
			if (m_head == NULL) return false;
			m_head->prev = m_head;
			m_head->next = m_head;
		}
		else
		{
			NODE* node = new NODE(obj);
			if (m_head == NULL) return false;
			NODE* tail = m_head->prev;
			node->prev = tail;
			node->next = m_head;
			m_head->prev = node;
			m_head = node;
			tail->next = m_head;
		}
		return true;
	}

	bool   insert_before(T* obj, T* newobj)
	{
		NODE* obj_node = find(obj);
		if (obj_node == NULL) return false;
		if (obj_node == m_head) return push_front(newobj);
		NODE* node = new NODE(newobj);
		if (node == NULL) return false;
		node->prev = obj_node;
		node->next = obj_node->next;
		node->next->prev = obj;
		obj_node->next = obj;
		return true;
	}

	T*   insert_after(T* obj, T* newobj)
	{
		NODE* obj_node = find(obj);
		if (obj_node == NULL) return false;
		if (obj_node == m_head) return push_front(newobj);
		NODE* node = new NODE(newobj);
		if (node == NULL) return false;
		node->prev = obj_node;
		node->next = obj_node->next;
		node->next->prev = obj;
		obj_node->next = obj;
		return true;
	}

	NODE* find(T* obj)
	{
		NODE* node = m_head;
		while (node)
		{
			if (node->data == obj) return node;
		}
		return NULL;
	}

	bool remove(T* obj)
	{
		return true;
	}
};

