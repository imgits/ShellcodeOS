#pragma once
#include "typedef.h"
#include <string.h>
#define OBJECT_NAME_SIZE	16
#define OBJECT_TYPE_SIZE	16

#define DEFAULT_OBJECT_NAME "OsObject"
#define DEFAULT_OBJECT_TYPE "Object"

class Object
{
private:
	char m_obj_name[OBJECT_NAME_SIZE];
	char m_obj_type[OBJECT_TYPE_SIZE];
public:
	Object()
	{
		strcpy(m_obj_name, DEFAULT_OBJECT_NAME);
		strcpy(m_obj_type, DEFAULT_OBJECT_TYPE);
	}

	Object(char* objname, char* objtype)
	{
		name(objname);
		type(objtype);
	}
	~Object()
	{
	}

	char* name()
	{
		return m_obj_name;
	}

	char* name(char* objname)
	{
		if (objname == NULL)
		{
			strcpy(m_obj_name, DEFAULT_OBJECT_NAME);
			return m_obj_name;
		}
		int len = strlen(objname);
		strncpy(m_obj_name, objname, OBJECT_NAME_SIZE);
		m_obj_name[OBJECT_NAME_SIZE - 1] = 0;
		return m_obj_name;
	}

	char* type()
	{
		return m_obj_type;
	}

	char* type(char* objtype)
	{
		if (objtype == NULL)
		{
			strcpy(m_obj_type, DEFAULT_OBJECT_TYPE);
			return m_obj_type;
		}
		strncpy(m_obj_type, objtype, OBJECT_TYPE_SIZE);
		m_obj_type[OBJECT_TYPE_SIZE - 1] = 0;
		return m_obj_type;
	}
};

