///////////////////////////////////////////////////////////
//  OCLObjectsMap.cpp
//  Implementation of the Class OCLObjectsMap
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "cl_objects_map.h"
using namespace Intel::OpenCL::Framework;

cl_int OCLObjectsMap::m_iNextGenKey = 1;

OCLObjectsMap::OCLObjectsMap()
{
}
OCLObjectsMap::~OCLObjectsMap()
{
	Clear();
}
cl_int OCLObjectsMap::AddObject(OCLObject * pObject)
{
	if (NULL == pObject)
	{
		return CL_INVALID_VALUE;
	}
	pObject->SetId(m_iNextGenKey);
	m_mapObjects[m_iNextGenKey] = pObject;
	++m_iNextGenKey;

	return pObject->GetId();
}
cl_err_code OCLObjectsMap::AddObject(OCLObject * pObject, int iObjectId, bool bAssignId)
{
	if (NULL == pObject || 0 == iObjectId)
	{
		return CL_INVALID_VALUE;
	}
	map<cl_int, OCLObject*>::iterator it = m_mapObjects.find(iObjectId);
	if (it != m_mapObjects.end())
	{
		return CL_ERR_KEY_ALLREADY_EXISTS;
	}
	if (bAssignId == true)
	{
		pObject->SetId(iObjectId);
	}
	m_mapObjects[iObjectId] = pObject;
	return CL_SUCCESS;
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
cl_err_code OCLObjectsMap::RemoveObject(cl_int iObjectId, OCLObject ** ppObjectRet)
{
	map<cl_int,OCLObject*>::iterator it = m_mapObjects.find(iObjectId);
	if (it == m_mapObjects.end())
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	if (NULL != ppObjectRet)
	{
		*ppObjectRet = it->second;
	}
	m_mapObjects.erase(it);
	return CL_SUCCESS;
}
cl_err_code OCLObjectsMap::GetObjects(cl_uint uiObjectCount, OCLObject ** ppObjects, cl_uint * puiObjectCountRet)
{
	if (NULL == ppObjects && NULL == puiObjectCountRet)
	{
		return CL_INVALID_VALUE;
	}
	if (NULL != ppObjects && uiObjectCount < m_mapObjects.size())
	{
		return CL_INVALID_VALUE;
	}
	if (NULL != puiObjectCountRet)
	{
		*puiObjectCountRet = m_mapObjects.size();
	}
	if (NULL != ppObjects)
	{
		map<cl_int, OCLObject*>::iterator it = m_mapObjects.begin();
		for(cl_int i=0; i< (int)m_mapObjects.size(), it != m_mapObjects.end(); ++i, it++)
		{
			ppObjects[i] = it->second;
		}
	}
	return CL_SUCCESS;
}
cl_err_code OCLObjectsMap::GetIDs(cl_uint uiIdsCount, cl_int * pIds, cl_uint * puiIdsCountRet)
{
	if (NULL == pIds && NULL == puiIdsCountRet)
	{
		return CL_INVALID_VALUE;
	}
	if (uiIdsCount < m_mapObjects.size())
	{
		return CL_INVALID_VALUE;
	}
	if (NULL != puiIdsCountRet)
	{
		*puiIdsCountRet = m_mapObjects.size();
	}
	if (NULL != pIds)
	{
		map<cl_int, OCLObject*>::iterator it = m_mapObjects.begin();
		for(cl_int i=0; i< (int)m_mapObjects.size(), it != m_mapObjects.end(); ++i, it++)
		{
			pIds[i] = it->first;
		}
	}
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
bool OCLObjectsMap::IsExists(cl_int iObjectId)
{
	//map<cl_int, OCLObject*>::iterator it = m_mapObjects.find(iObjectId);
	return (m_mapObjects.find(iObjectId) != m_mapObjects.end());
}