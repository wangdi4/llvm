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
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	/*
	map<HandleType*, OCLObject<HandleType>*>::iterator it = m_mapObjects.find(hObjectHandle);
	if (it != m_mapObjects.end())
	{
		return CL_ERR_KEY_ALLREADY_EXISTS;
	}
	*/
	m_mapObjects[hObjectHandle] = pObject;
	pObject->AddPendency();

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

	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	HandleTypeMapIterator it = m_mapObjects.find(hObjectHandle);
	if (it != m_mapObjects.end())
	{
		return CL_ERR_KEY_ALLREADY_EXISTS;
	}
	m_mapObjects[hObjectHandle] = pObject;
	//This is necessary to prevent a race between object release and object create in the unfortunate event that the OS reuses the pointer used as an object handle
	pObject->AddPendency();
	return CL_SUCCESS;
}

template <class HandleType>
cl_err_code OCLObjectsMap<HandleType>::GetOCLObject(HandleType* hObjectHandle, OCLObject<HandleType> ** ppObject)
{
	assert ("Invalid input" && (NULL != ppObject) );

	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);

	HandleTypeMapConstIterator it = m_mapObjects.find(hObjectHandle);
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

	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	if (uiIndex > m_mapObjects.size())
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	HandleTypeMapConstIterator it = m_mapObjects.begin();
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
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	HandleTypeMapIterator it = m_mapObjects.find(hObjectHandle);
	if (it == m_mapObjects.end())
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	//This is necessary to prevent a race between object release and object create in the unfortunate event that the OS reuses the pointer used as an object handle
	OCLObject<HandleType>* obj = it->second;
	m_mapObjects.erase(it);
	obj->RemovePendency();
	return CL_SUCCESS;
}

template <class HandleType>
cl_err_code OCLObjectsMap<HandleType>::GetObjects(cl_uint uiObjectCount, OCLObject<HandleType> ** ppObjects, cl_uint * puiObjectCountRet)
{
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
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
		assert(m_mapObjects.size() <= CL_MAX_UINT32);
		*puiObjectCountRet = (cl_uint)m_mapObjects.size();
	}
	if (NULL != ppObjects)
	{
		HandleTypeMapConstIterator it = m_mapObjects.begin();
		for(cl_int i=0; ((i< (int)m_mapObjects.size()) && (it != m_mapObjects.end())); ++i, it++)
		{
			ppObjects[i] = it->second;
		}
	}
	return CL_SUCCESS;
}

template <class HandleType>
cl_err_code OCLObjectsMap<HandleType>::GetIDs(cl_uint uiIdsCount, HandleType** pIds, cl_uint * puiIdsCountRet)
{
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
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
		assert(m_mapObjects.size() <= CL_MAX_UINT32);
		*puiIdsCountRet = (cl_uint)m_mapObjects.size();
	}
	if (NULL != pIds)
	{
		HandleTypeMapConstIterator it = m_mapObjects.begin();
		for(cl_int i=0; ((i< (int)m_mapObjects.size()) && (it != m_mapObjects.end())); ++i, it++)
		{
			pIds[i] = it->first;
		}
	}
	return CL_SUCCESS;
}

template <class HandleType>
cl_uint OCLObjectsMap<HandleType>::Count()
{
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	assert(m_mapObjects.size() <= CL_MAX_UINT32);
	return (cl_uint)m_mapObjects.size();
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
	HandleTypeMapIterator it = m_mapObjects.find(hObject);
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
		OCLObject<HandleType>* obj = it->second;
		m_mapObjects.erase(it);
		obj->RemovePendency();
	}
	return CL_SUCCESS;
}

template <class HandleType>
void OCLObjectsMap<HandleType>::ReleaseAllObjects()
{
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	HandleTypeMapIterator it = m_mapObjects.begin();
	while (it != m_mapObjects.end())
	{
		it->second->Release();
		it->second->RemovePendency();
		++it;
	}
	m_mapObjects.clear();
}
