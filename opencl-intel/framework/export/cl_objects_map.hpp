// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

template <class HandleType, class ParentHandleType>
std::atomic<long> OCLObjectsMap<HandleType, ParentHandleType>::m_iNextGenKey(1);

template <class HandleType, class ParentHandleType>
OCLObjectsMap<HandleType, ParentHandleType>::~OCLObjectsMap() {
  m_mapObjects.clear();
}

template <class HandleType, class ParentHandleType>
HandleType *OCLObjectsMap<HandleType, ParentHandleType>::AddObject(
    const SharedPtr<OCLObject<HandleType, ParentHandleType>> &pObject) {
  assert(0 != pObject);
  HandleType *hObjectHandle = pObject->GetHandle();
  assert(hObjectHandle);
  cl_int iObjectId = m_iNextGenKey++;
  pObject->SetId(iObjectId);
  std::lock_guard<std::mutex> mu(m_muMapMutex);
  /*
  map<HandleType*, OCLObject<HandleType, ParentHandleType>*>::iterator it =
  m_mapObjects.find(hObjectHandle); if (it != m_mapObjects.end())
  {
      return CL_ERR_KEY_ALLREADY_EXISTS;
  }
  */
  if (m_bDisableAdding) {
    return NULL;
  }
  m_mapObjects[hObjectHandle] = pObject;
  return hObjectHandle;
}

template <class HandleType, class ParentHandleType>
void OCLObjectsMap<HandleType, ParentHandleType>::DisableAdding() {
  std::lock_guard<std::mutex> mu(m_muMapMutex);
  m_bDisableAdding = true;
}

template <class HandleType, class ParentHandleType>
void OCLObjectsMap<HandleType, ParentHandleType>::EnableAdding() {
  std::lock_guard<std::mutex> mu(m_muMapMutex);
  m_bDisableAdding = false;
}

template <class HandleType, class ParentHandleType>
cl_err_code OCLObjectsMap<HandleType, ParentHandleType>::AddObject(
    const SharedPtr<OCLObject<HandleType, ParentHandleType>> &pObject,
    bool bAssignId) {
  if (NULL == pObject.GetPtr()) {
    return CL_INVALID_VALUE;
  }
  HandleType *hObjectHandle = pObject->GetHandle();
  if (bAssignId) {
    pObject->SetId(m_iNextGenKey++);
  }

  std::lock_guard<std::mutex> mu(m_muMapMutex);

  if (m_bDisableAdding) {
    return CL_ERR_FAILURE;
  }

  HandleTypeMapIterator it = m_mapObjects.find(hObjectHandle);
  if (it != m_mapObjects.end()) {
    return CL_ERR_KEY_ALLREADY_EXISTS;
  }
  m_mapObjects[hObjectHandle] = pObject;
  return CL_SUCCESS;
}

template <class HandleType, class ParentHandleType>
SharedPtr<OCLObject<HandleType, ParentHandleType>>
OCLObjectsMap<HandleType, ParentHandleType>::GetOCLObject(
    HandleType *hObjectHandle) {
  std::lock_guard<std::mutex> mu(m_muMapMutex);

  HandleTypeMapConstIterator it = m_mapObjects.find(hObjectHandle);
  if (it == m_mapObjects.end()) {
    return NULL;
  }
  return it->second;
}

template <class HandleType, class ParentHandleType>
OCLObject<HandleType, ParentHandleType> *
OCLObjectsMap<HandleType, ParentHandleType>::GetOCLObjectPtr(
    HandleType *hObjectHandle) {
  std::lock_guard<std::mutex> mu(m_muMapMutex);

  HandleTypeMapConstIterator it = m_mapObjects.find(hObjectHandle);
  if (it == m_mapObjects.end()) {
    return NULL;
  }
  return it->second.GetPtr();
}

template <class HandleType, class ParentHandleType>
SharedPtr<OCLObject<HandleType, ParentHandleType>>
OCLObjectsMap<HandleType, ParentHandleType>::GetObjectByIndex(cl_uint uiIndex) {
  std::lock_guard<std::mutex> mu(m_muMapMutex);
  if (uiIndex > m_mapObjects.size()) {
    return NULL;
  }
  HandleTypeMapConstIterator it = m_mapObjects.begin();
  for (cl_uint ui = 0; ui < uiIndex; ++ui) {
    ++it;
  }
  return it->second;
}

template <class HandleType, class ParentHandleType>
template <class F>
bool OCLObjectsMap<HandleType, ParentHandleType>::ForEach(F &functor) {
  std::lock_guard<std::mutex> mu(m_muMapMutex);

  for (HandleTypeMapIterator iter = m_mapObjects.begin();
       iter != m_mapObjects.end(); iter++) {
    if (!functor(iter->second)) {
      return false;
    }
  }
  return true;
}

template <class HandleType, class ParentHandleType>
cl_err_code OCLObjectsMap<HandleType, ParentHandleType>::RemoveObject(
    HandleType *hObjectHandle) {
  // m_muMapMutex does not support recursive locking.
  // Use manual Lock/Unlock to ensure that lock is released before the
  // destructor of SharedPtr is called to avoid deadlocks
  m_muMapMutex.lock();
  HandleTypeMapIterator it = m_mapObjects.find(hObjectHandle);
  if (it == m_mapObjects.end()) {
    m_muMapMutex.unlock();
    return CL_ERR_KEY_NOT_FOUND;
  }
  // This is necessary to prevent a race between object release and object
  // create in the unfortunate event that the OS reuses the pointer used as an
  // object handle
  SharedPtr<OCLObject<HandleType, ParentHandleType>> obj = it->second;
  if (m_bPreserveUserHandles) {
    obj->SetPreserveHandleOnDetele();
  }
  m_mapObjects.erase(it);
  m_muMapMutex.unlock();
  // destructor of SharedPtr will be called here
  return CL_SUCCESS;
}

template <class HandleType, class ParentHandleType>
void OCLObjectsMap<HandleType, ParentHandleType>::GetObjects(
    std::vector<SharedPtr<OCLObject<HandleType, ParentHandleType>>> &Objects) {
  std::lock_guard<std::mutex> mu(m_muMapMutex);
  if (m_mapObjects.size() == 0)
    return;
  Objects.reserve(m_mapObjects.size());
  for (const auto &it : m_mapObjects) {
    Objects.push_back(it.second);
  }
}

template <class HandleType, class ParentHandleType>
cl_err_code OCLObjectsMap<HandleType, ParentHandleType>::GetIDs(
    cl_uint uiIdsCount, HandleType **pIds, cl_uint *puiIdsCountRet) {
  std::lock_guard<std::mutex> mu(m_muMapMutex);
  if (NULL == pIds && NULL == puiIdsCountRet) {
    return CL_INVALID_VALUE;
  }
  if (uiIdsCount < m_mapObjects.size()) {
    return CL_INVALID_VALUE;
  }
  if (NULL != puiIdsCountRet) {
    assert(m_mapObjects.size() <= CL_MAX_UINT32);
    *puiIdsCountRet = (cl_uint)m_mapObjects.size();
  }
  if (NULL != pIds) {
    HandleTypeMapConstIterator it = m_mapObjects.begin();
    for (cl_int i = 0;
         ((i < (int)m_mapObjects.size()) && (it != m_mapObjects.end()));
         ++i, it++) {
      pIds[i] = it->first;
    }
  }
  return CL_SUCCESS;
}

template <class HandleType, class ParentHandleType>
cl_uint OCLObjectsMap<HandleType, ParentHandleType>::Count() const {
  std::lock_guard<std::mutex> mu(m_muMapMutex);
  assert(m_mapObjects.size() <= CL_MAX_UINT32);
  return (cl_uint)m_mapObjects.size();
}

template <class HandleType, class ParentHandleType>
void OCLObjectsMap<HandleType, ParentHandleType>::Clear() {
  std::lock_guard<std::mutex> mu(m_muMapMutex);
  m_mapObjects.clear();
}

template <class HandleType, class ParentHandleType>
bool OCLObjectsMap<HandleType, ParentHandleType>::IsExists(
    HandleType *hObjectHandle) {
  std::lock_guard<std::mutex> mu(m_muMapMutex);
  return (m_mapObjects.find(hObjectHandle) != m_mapObjects.end());
}

template <class HandleType, class ParentHandleType>
cl_err_code OCLObjectsMap<HandleType, ParentHandleType>::ReleaseObject(
    HandleType *hObject) {
  // m_muMapMutex does not support recursive locking.
  // Use manual Lock/Unlock to ensure that lock is released before the
  // destructor of SharedPtr is called to avoid deadlocks
  m_muMapMutex.lock();
  HandleTypeMapIterator it = m_mapObjects.find(hObject);
  if (m_mapObjects.end() == it) {
    m_muMapMutex.unlock();
    return CL_ERR_KEY_NOT_FOUND;
  }
  if (m_bPreserveUserHandles) {
    it->second->SetPreserveHandleOnDetele();
  }
  long newRef = it->second->Release();
  if (newRef < 0) {
    m_muMapMutex.unlock();
    return CL_ERR_FAILURE;
  } else if (0 == newRef) {
    SharedPtr<OCLObject<HandleType, ParentHandleType>> obj = it->second;
    m_mapObjects.erase(it);
    m_muMapMutex.unlock();
    // SharedPtr destructor will be called here
    return CL_SUCCESS;
  }
  m_muMapMutex.unlock();
  return CL_SUCCESS;
}

template <class HandleType, class ParentHandleType>
void OCLObjectsMap<HandleType, ParentHandleType>::ReleaseAllObjects(
    bool bTerminate) {
  std::lock_guard<std::mutex> mu(m_muMapMutex);
  HandleTypeMapIterator it = m_mapObjects.begin();
  while (it != m_mapObjects.end()) {
    if (m_bPreserveUserHandles) {
      it->second->SetPreserveHandleOnDetele();
    }
    it->second->SetTerminate(bTerminate);
    it->second->Release();
    ++it;
  }
  m_mapObjects.clear();
}
