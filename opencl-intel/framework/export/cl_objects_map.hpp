///////////////////////////////////////////////////////////
//  OCLObjectsMap.hpp
//  Implementation of the Class OCLObjectsMap
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

template <class HandleType>
Intel::OpenCL::Utils::AtomicCounter OCLObjectsMap<HandleType>::m_iNextGenKey(1);

template <class HandleType>
OCLObjectsMap<HandleType>::~OCLObjectsMap()
{
	m_mapObjects.clear();
}

template <class HandleType>
HandleType* OCLObjectsMap<HandleType>::AddObject(OCLObject<HandleType> * pObject)
{
	assert ( NULL != pObject );
	HandleType* hObjectHandle = pObject->GetHandle();
	assert(hObjectHandle);
	cl_int iObjectId = m_iNextGenKey++;
	pObject->SetId(iObjectId);
	OclAutoMutex mu(&m_muMapMutex);
	m_mapObjects[hObjectHandle] = pObject;

	return hObjectHandle;
}

template <class HandleType>
cl_err_code OCLObjectsMap<HandleType>::AddObject(OCLObject<HandleType> * pObject, bool bAssignId)
{
	if (NULL == pObject)
	{
		return CL_INVALID_VALUE;
	}
	HandleType* hObjectHandle = pObject->GetHandle();
	if (bAssignId)
	{
		pObject->SetId(m_iNextGenKey++);
	}

	OclAutoMutex mu(&m_muMapMutex);
	map<HandleType*, OCLObject<HandleType>*>::iterator it = m_mapObjects.find(hObjectHandle);
	if (it != m_mapObjects.end())
	{
		return CL_ERR_KEY_ALLREADY_EXISTS;
	}
	m_mapObjects[hObjectHandle] = pObject;
	return CL_SUCCESS;
}

template <class HandleType>
cl_err_code OCLObjectsMap<HandleType>::GetOCLObject(HandleType* hObjectHandle, OCLObject<HandleType> ** ppObject)
{
	assert ("Invalid input" && (NULL != ppObject) );

	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);

	map<HandleType*, OCLObject<HandleType>*>::const_iterator it = m_mapObjects.find(hObjectHandle);
	if (it == m_mapObjects.end())
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	*ppObject = it->second;
	return CL_SUCCESS;
}

template <class HandleType>
cl_err_code OCLObjectsMap<HandleType>::GetObjectByIndex(cl_uint uiIndex, OCLObject<HandleType> ** ppObject)
{
	assert ("Invalid input" && (NULL != ppObject) );

	OclAutoMutex mu(&m_muMapMutex);
	if (uiIndex > m_mapObjects.size())
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	map<HandleType*, OCLObject<HandleType>*>::const_iterator it = m_mapObjects.begin();
	for (cl_uint ui = 0; ui < uiIndex; ++ui)
	{
		++it;
	}
	*ppObject = it->second;
	return CL_SUCCESS;
}

template <class HandleType>
cl_err_code OCLObjectsMap<HandleType>::RemoveObject(HandleType* hObjectHandle)
{
	OclAutoMutex mu(&m_muMapMutex);
	map<HandleType*, OCLObject<HandleType>*>::iterator it = m_mapObjects.find(hObjectHandle);
	if (it == m_mapObjects.end())
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	m_mapObjects.erase(it);
	return CL_SUCCESS;
}

template <class HandleType>
cl_err_code OCLObjectsMap<HandleType>::GetObjects(cl_uint uiObjectCount, OCLObject<HandleType> ** ppObjects, cl_uint * puiObjectCountRet)
{
	OclAutoMutex mu(&m_muMapMutex);
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
		map<HandleType*, OCLObject<HandleType>*>::const_iterator it = m_mapObjects.begin();
		for(cl_int i=0; i< (int)m_mapObjects.size(), it != m_mapObjects.end(); ++i, it++)
		{
			ppObjects[i] = it->second;
		}
	}
	return CL_SUCCESS;
}

template <class HandleType>
cl_err_code OCLObjectsMap<HandleType>::GetIDs(cl_uint uiIdsCount, HandleType** pIds, cl_uint * puiIdsCountRet)
{
	OclAutoMutex mu(&m_muMapMutex);
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
		map<HandleType*, OCLObject<HandleType>*>::const_iterator it = m_mapObjects.begin();
		for(cl_int i=0; i< (int)m_mapObjects.size(), it != m_mapObjects.end(); ++i, it++)
		{
			pIds[i] = it->first;
		}
	}
	return CL_SUCCESS;
}

template <class HandleType>
cl_uint OCLObjectsMap<HandleType>::Count()
{
	return m_mapObjects.size();
}

template <class HandleType>
void OCLObjectsMap<HandleType>::Clear()
{
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	m_mapObjects.clear();
}

template <class HandleType>
bool OCLObjectsMap<HandleType>::IsExists(HandleType* hObjectHandle)
{
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	return (m_mapObjects.find(hObjectHandle) != m_mapObjects.end());
}

template <class HandleType>
cl_err_code OCLObjectsMap<HandleType>::ReleaseObject(HandleType* hObject)
{
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	map<HandleType*, OCLObject<HandleType>*>::iterator it = m_mapObjects.find(hObject);
	if (m_mapObjects.end() == it)
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	long newRef = it->second->Release();
	if (newRef < 0)
	{
		return CL_ERR_FAILURE;
	}
	else if (0 == newRef)
	{
		m_mapObjects.erase(it);
	}
	return CL_SUCCESS;
}

template <class HandleType>
void OCLObjectsMap<HandleType>::ReleaseAllObjects()
{
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	map<HandleType*, OCLObject<HandleType>*>::iterator it = m_mapObjects.begin();
	while (it != m_mapObjects.end())
	{
		it->second->Release();
	}
	m_mapObjects.clear();
}
