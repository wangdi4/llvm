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
	m_mapDirtyObjects.clear();
}

template <class HandleType>
HandleType* OCLObjectsMap<HandleType>::AddObject(OCLObject<HandleType> * pObject)
{
	OclAutoMutex mu(&m_muMapMutex, false);
	assert ( NULL != pObject );
	HandleType* hObjectHandle = pObject->GetHandle();
	assert(hObjectHandle);
	cl_int iObjectId = m_iNextGenKey++;
	pObject->SetId(iObjectId);
	m_muMapMutex.Lock();
	m_mapObjects[hObjectHandle] = pObject;

	return hObjectHandle;
}

template <class HandleType>
cl_err_code OCLObjectsMap<HandleType>::AddObject(OCLObject<HandleType> * pObject, bool bAssignId)
{
	OclAutoMutex mu(&m_muMapMutex, false);
	if (NULL == pObject)
	{
		return CL_INVALID_VALUE;
	}
	HandleType* hObjectHandle = pObject->GetHandle();

	m_muMapMutex.Lock();
	map<HandleType*, OCLObject<HandleType>*>::iterator it = m_mapObjects.find(hObjectHandle);
	if (it != m_mapObjects.end())
	{
		return CL_ERR_KEY_ALLREADY_EXISTS;
	}
	if (bAssignId == true)
	{
		pObject->SetId(m_iNextGenKey++);
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
	if (m_mapObjects.size() == 0)
	{
		return CL_ERR_LIST_EMPTY;
	}
	if (uiIndex >= m_mapObjects.size())
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	map<HandleType*, OCLObject<HandleType>*>::const_iterator it = m_mapObjects.begin();
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

template <class HandleType>
cl_err_code OCLObjectsMap<HandleType>::RemoveObject(HandleType* hObjectHandle, OCLObject<HandleType> ** ppObjectRet, bool bSetDirty)
{
	OclAutoMutex mu(&m_muMapMutex);
	map<HandleType*, OCLObject<HandleType>*>::iterator it = m_mapObjects.find(hObjectHandle);
	if (it == m_mapObjects.end())
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	if (NULL != ppObjectRet)
	{
		*ppObjectRet = it->second;
	}
	if (true == bSetDirty)
	{
		m_mapDirtyObjects[it->first] = it->second;
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
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	return m_mapObjects.size();
}

template <class HandleType>
void OCLObjectsMap<HandleType>::Clear(bool bSetDirty)
{
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	if (true == bSetDirty)
	{
		map<HandleType*, OCLObject<HandleType>*>::iterator it = m_mapObjects.begin();
		while (it != m_mapObjects.end())
		{
			m_mapDirtyObjects[it->first] = it->second;
			it++;
		}
	}
	m_mapObjects.clear();
}

template <class HandleType>
bool OCLObjectsMap<HandleType>::IsExists(HandleType* hObjectHandle)
{
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	return (m_mapObjects.find(hObjectHandle) != m_mapObjects.end());
}

template <class HandleType>
void OCLObjectsMap<HandleType>::GarbageCollector( bool bIsTerminate )
{
	Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
	map<HandleType*, OCLObject<HandleType>*>::iterator it = m_mapDirtyObjects.begin();
	while (it != m_mapDirtyObjects.end())
	{
		OCLObject<HandleType> * pObject = it->second;
		if (NULL != pObject && pObject->ReadyForDeletion())
		{
			it->second = NULL;
            pObject->Cleanup(bIsTerminate);
			delete pObject;
            HandleType* hObjectHandle = it->first;
            it++;
            m_mapDirtyObjects.erase(hObjectHandle);
		}
        else
        {
            it++;
        }
	}
}
