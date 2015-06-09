// Copyright (c) 2006-2012 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  OCLObjectsMap.hpp
//  Implementation of the Class OCLObjectsMap
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

template <class HandleType, class ParentHandleType>
Intel::OpenCL::Utils::AtomicCounter OCLObjectsMap<HandleType, ParentHandleType>::m_iNextGenKey(1);

template <class HandleType, class ParentHandleType>
OCLObjectsMap<HandleType, ParentHandleType>::~OCLObjectsMap()
{
    m_mapObjects.clear();
}

template <class HandleType, class ParentHandleType>
HandleType* OCLObjectsMap<HandleType, ParentHandleType>::AddObject(const SharedPtr<OCLObject<HandleType, ParentHandleType> >& pObject)
{
    assert ( 0 != pObject );
    HandleType* hObjectHandle = pObject->GetHandle();
    assert(hObjectHandle);
    cl_int iObjectId = m_iNextGenKey++;
    pObject->SetId(iObjectId);
    Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
    /*
    map<HandleType*, OCLObject<HandleType, ParentHandleType>*>::iterator it = m_mapObjects.find(hObjectHandle);
    if (it != m_mapObjects.end())
    {
        return CL_ERR_KEY_ALLREADY_EXISTS;
    }
    */
    if (m_bDisableAdding)
    {
        return NULL;
    }
    m_mapObjects[hObjectHandle] = pObject;
    return hObjectHandle;
}

template <class HandleType, class ParentHandleType>
void OCLObjectsMap<HandleType, ParentHandleType>::DisableAdding()
{
    Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
    m_bDisableAdding = true;
}

template <class HandleType, class ParentHandleType>
void OCLObjectsMap<HandleType, ParentHandleType>::EnableAdding()
{
    Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
    m_bDisableAdding = false;
}

template <class HandleType, class ParentHandleType>
cl_err_code OCLObjectsMap<HandleType, ParentHandleType>::AddObject(const SharedPtr<OCLObject<HandleType, ParentHandleType> >& pObject, bool bAssignId)
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

    if (m_bDisableAdding)
    {
        return CL_ERR_FAILURE;
    }

    HandleTypeMapIterator it = m_mapObjects.find(hObjectHandle);
    if (it != m_mapObjects.end())
    {
        return CL_ERR_KEY_ALLREADY_EXISTS;
    }
    m_mapObjects[hObjectHandle] = pObject;
    return CL_SUCCESS;
}

template <class HandleType, class ParentHandleType>
SharedPtr<OCLObject<HandleType, ParentHandleType> > OCLObjectsMap<HandleType, ParentHandleType>::GetOCLObject(HandleType* hObjectHandle)
{
    Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);

    HandleTypeMapConstIterator it = m_mapObjects.find(hObjectHandle);
    if (it == m_mapObjects.end())
    {
        return NULL;
    }
    return it->second;
}

template <class HandleType, class ParentHandleType>
OCLObject<HandleType, ParentHandleType>* OCLObjectsMap<HandleType, ParentHandleType>::GetOCLObjectPtr(HandleType* hObjectHandle)
{
    Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);

    HandleTypeMapConstIterator it = m_mapObjects.find(hObjectHandle);
    if (it == m_mapObjects.end())
    {
        return NULL;
    }
    return it->second.GetPtr();
}

template <class HandleType, class ParentHandleType>
SharedPtr<OCLObject<HandleType, ParentHandleType> > OCLObjectsMap<HandleType, ParentHandleType>::GetObjectByIndex(cl_uint uiIndex)
{
    Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
    if (uiIndex > m_mapObjects.size())
    {
        return NULL;
    }
    HandleTypeMapConstIterator it = m_mapObjects.begin();
    for (cl_uint ui = 0; ui < uiIndex; ++ui)
    {
        ++it;
    }
    return it->second;
}

template<class HandleType, class ParentHandleType>
template<class F>
bool OCLObjectsMap<HandleType, ParentHandleType>::ForEach(F& functor)
{
    Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);

    for (HandleTypeMapIterator iter = m_mapObjects.begin(); iter != m_mapObjects.end(); iter++)
    {
        if (!functor(iter->second))
        {
            return false;
        }
    }
    return true;
}

template <class HandleType, class ParentHandleType>
cl_err_code OCLObjectsMap<HandleType, ParentHandleType>::RemoveObject(HandleType* hObjectHandle)
{
    // m_muMapMutex does not support recursive locking. 
    // Use manual Lock/Unlock to ensure that lock is released before the destructor of SharedPtr is called to avoid deadlocks
    m_muMapMutex.Lock(); 
    HandleTypeMapIterator it = m_mapObjects.find(hObjectHandle);
    if (it == m_mapObjects.end())
    {
        m_muMapMutex.Unlock();
        return CL_ERR_KEY_NOT_FOUND;
    }
    //This is necessary to prevent a race between object release and object create in the unfortunate event that the OS reuses the pointer used as an object handle
    SharedPtr<OCLObject<HandleType, ParentHandleType> > obj = it->second;
    if (m_bPreserveUserHandles)
    {
        obj->SetPreserveHandleOnDetele();
    }
    m_mapObjects.erase(it);
    m_muMapMutex.Unlock();
    // destructor of SharedPtr will be called here
    return CL_SUCCESS;
}

template <class HandleType, class ParentHandleType>
cl_err_code OCLObjectsMap<HandleType, ParentHandleType>::GetObjects(cl_uint uiObjectCount, SharedPtr<OCLObject<HandleType, ParentHandleType> >* ppObjects, cl_uint * puiObjectCountRet)
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

template <class HandleType, class ParentHandleType>
cl_err_code OCLObjectsMap<HandleType, ParentHandleType>::GetIDs(cl_uint uiIdsCount, HandleType** pIds, cl_uint * puiIdsCountRet)
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

template <class HandleType, class ParentHandleType>
cl_uint OCLObjectsMap<HandleType, ParentHandleType>::Count() const
{
    Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
    assert(m_mapObjects.size() <= CL_MAX_UINT32);
    return (cl_uint)m_mapObjects.size();
}

template <class HandleType, class ParentHandleType>
void OCLObjectsMap<HandleType, ParentHandleType>::Clear()
{
    Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
    m_mapObjects.clear();
}

template <class HandleType, class ParentHandleType>
bool OCLObjectsMap<HandleType, ParentHandleType>::IsExists(HandleType* hObjectHandle)
{
    Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
    return (m_mapObjects.find(hObjectHandle) != m_mapObjects.end());
}

template <class HandleType, class ParentHandleType>
cl_err_code OCLObjectsMap<HandleType, ParentHandleType>::ReleaseObject(HandleType* hObject)
{
    // m_muMapMutex does not support recursive locking. 
    // Use manual Lock/Unlock to ensure that lock is released before the destructor of SharedPtr is called to avoid deadlocks    
    m_muMapMutex.Lock();
    HandleTypeMapIterator it = m_mapObjects.find(hObject);
    if (m_mapObjects.end() == it)
    {
        m_muMapMutex.Unlock();
        return CL_ERR_KEY_NOT_FOUND;
    }
    if (m_bPreserveUserHandles)
    {
        it->second->SetPreserveHandleOnDetele();
    }
    long newRef = it->second->Release();
    if (newRef < 0)
    {
        m_muMapMutex.Unlock();
        return CL_ERR_FAILURE;
    }
    else if (0 == newRef)
    {
        SharedPtr<OCLObject<HandleType, ParentHandleType> > obj = it->second;
        m_mapObjects.erase(it);
        m_muMapMutex.Unlock();
        // SharedPtr destructor will be called here
        return CL_SUCCESS;
    }
    m_muMapMutex.Unlock();
    return CL_SUCCESS;
}

template <class HandleType, class ParentHandleType>
void OCLObjectsMap<HandleType, ParentHandleType>::ReleaseAllObjects(bool bTerminate)
{
    Intel::OpenCL::Utils::OclAutoMutex mu(&m_muMapMutex);
    HandleTypeMapIterator it = m_mapObjects.begin();
    while (it != m_mapObjects.end())
    {
        if (m_bPreserveUserHandles)
        {
            it->second->SetPreserveHandleOnDetele();
        }
        it->second->SetTerminate(bTerminate);
        it->second->Release();
        ++it;
    }
    m_mapObjects.clear();
}
