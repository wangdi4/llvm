///////////////////////////////////////////////////////////
//  OCLObjectsMap.cpp
//  Implementation of the Class OCLObjectsMap
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "OCLObjectsMap.h"
using namespace Intel::OpenCL::Framework;

OCLObjectsMap::OCLObjectsMap()
{
	m_iMaxKey = 1;
}
OCLObjectsMap::~OCLObjectsMap()
{
	Clear();
}
cl_int OCLObjectsMap::AddObject(OCLObject * pObject)
{
	if (NULL == pObject)
	{
		return CL_ERR_INITILIZATION_FAILED;
	}
	pObject->SetId(m_iMaxKey);
	m_mapObjects[m_iMaxKey] = pObject;
	++m_iMaxKey;

	return pObject->GetId();
}
cl_err_code OCLObjectsMap::GetOCLObject(cl_int iObjectId, OCLObject ** ppObject)
{
	if (NULL == ppObject)
	{
		return CL_ERR_INITILIZATION_FAILED;
	}
	map<cl_int,OCLObject*>::iterator it = m_mapObjects.find(iObjectId);
	if (it == m_mapObjects.end())
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	*ppObject = it->second;
	return CL_SUCCESS;
}
cl_err_code OCLObjectsMap::GetObjectByIndex(cl_uint uiIndex, OCLObject ** ppObject)
{
	if (NULL == ppObject)
	{
		return CL_ERR_INITILIZATION_FAILED;
	}
	if (m_mapObjects.size() == 0)
	{
		return CL_ERR_LIST_EMPTY;
	}
	if (uiIndex >= m_mapObjects.size())
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	map<cl_int,OCLObject*>::iterator it = m_mapObjects.begin();
	while (it != m_mapObjects.end())
	{
		if (uiIndex == 0)
		{
			*ppObject = it->second;
			return CL_SUCCESS;
		}
		uiIndex--;
		it++;
	}
	return CL_ERR_KEY_NOT_FOUND;
}
cl_err_code OCLObjectsMap::RemoveObject(cl_int iObjectId)
{
	map<cl_int,OCLObject*>::iterator it = m_mapObjects.find(iObjectId);
	if (it == m_mapObjects.end())
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	m_mapObjects.erase(it);
	return CL_SUCCESS;
}
const cl_uint OCLObjectsMap::Count()
{
	return m_mapObjects.size();
}
void OCLObjectsMap::Clear()
{
	m_mapObjects.clear();
}