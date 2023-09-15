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

#include "GraphicsApiMemoryObject.h"
#include "cl_shared_ptr.hpp"
#include "memobj_event.h"
#include "ocl_event.h"

using namespace std;

namespace Intel {
namespace OpenCL {
namespace Framework {
/**
 * @fn  GraphicsApiMemoryObject::~GraphicsApiMemoryObject()
 */

GraphicsApiMemoryObject::~GraphicsApiMemoryObject() {
  for (t_AcquiredObjects::iterator it = m_lstAcquiredObjectDescriptors.begin();
       it != m_lstAcquiredObjectDescriptors.end(); it++) {
    (*it).second->Release();
  }
}

/**
 * @fn  cl_err_code GraphicsApiMemoryObject::UpdateHostPtr(cl_mem_flags
 * clMemFlags, void* pHostPtr)
 */

cl_err_code GraphicsApiMemoryObject::UpdateHostPtr(cl_mem_flags /*clMemFlags*/,
                                                   void * /*pHostPtr*/) {
  return CL_SUCCESS;
}

/**
 * @fn  cl_err_code GraphicsApiMemoryObject::LockOnDevice( IN const
 * ConstSharedPtr<FissionableDevice>& dev, IN MemObjUsage usage, OUT
 * SharedPtr<OclEvent>* pOutEvent )
 */

cl_err_code GraphicsApiMemoryObject::LockOnDevice(
    IN const SharedPtr<FissionableDevice> &dev, IN MemObjUsage usage,
    OUT MemObjUsage *pOutUsageLocked, OUT SharedPtr<OclEvent> &pOutEvent) {
  pOutEvent = NULL;
  *pOutUsageLocked = usage;
  std::lock_guard<std::recursive_mutex> mu(m_muAcquireRelease);
  if (m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject) {
    return CL_SUCCESS;
  }

  return m_itCurrentAcquriedObject->second->LockOnDevice(
      dev, usage, pOutUsageLocked, pOutEvent);
}

/**
 * @fn  cl_err_code GraphicsApiMemoryObject::UnLockOnDevice( IN
 * ConstSharedPtr<FissionableDevice> dev, IN MemObjUsage usage )
 */

cl_err_code GraphicsApiMemoryObject::UnLockOnDevice(
    IN const SharedPtr<FissionableDevice> &dev, IN MemObjUsage usage) {
  std::lock_guard<std::recursive_mutex> mu(m_muAcquireRelease);
  if (m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject) {
    return CL_SUCCESS;
  }
  return m_itCurrentAcquriedObject->second->UnLockOnDevice(dev, usage);
}

/**
 * @fn  cl_err_code GraphicsApiMemoryObject::CheckBoundsRect(const size_t*
 * pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch)
 * const
 */

cl_err_code GraphicsApiMemoryObject::CheckBoundsRect(
    const size_t *pszOrigin, const size_t *pszRegion, size_t szRowPitch,
    size_t szSlicePitch) const {
  std::lock_guard<std::recursive_mutex> mu(m_muAcquireRelease);
  if (m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject) {
    return CL_INVALID_VALUE;
  }
  return m_itCurrentAcquriedObject->second->CheckBoundsRect(
      pszOrigin, pszRegion, szRowPitch, szSlicePitch);
}

/**
 * @fn  void* GraphicsApiMemoryObject::GetBackingStoreData(const size_t*
 * pszOrigin) const
 */

void *
GraphicsApiMemoryObject::GetBackingStoreData(const size_t *pszOrigin) const {
  std::lock_guard<std::recursive_mutex> mu(m_muAcquireRelease);
  if (m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject) {
    return NULL;
  }
  return m_itCurrentAcquriedObject->second->GetBackingStoreData(pszOrigin);
}

/**
 * @fn  cl_err_code
 * GraphicsApiMemoryObject::CreateDeviceResource(SharedPtr<FissionableDevice>
 * pDevice)
 */

cl_err_code GraphicsApiMemoryObject::CreateDeviceResource(
    const SharedPtr<FissionableDevice> & /*pDevice*/) {
  return CL_SUCCESS;
}

/**
 * @fn  bool
 * GraphicsApiMemoryObject::IsSupportedByDevice(SharedPtr<FissionableDevice>
 * pDevice)
 */

bool GraphicsApiMemoryObject::IsSupportedByDevice(
    const SharedPtr<FissionableDevice> & /*pDevice*/) {
  return true;
}

/**
 * @fn  cl_err_code
 * GraphicsApiMemoryObject::MemObjCreateDevMappedRegion(SharedPtr<FissionableDevice>
 * pDevice, cl_dev_cmd_param_map* cmd_param_map)
 */

cl_err_code GraphicsApiMemoryObject::MemObjCreateDevMappedRegion(
    const SharedPtr<FissionableDevice> &pDevice,
    cl_dev_cmd_param_map *cmd_param_map, void **pHostMapDataPtr) {
  std::lock_guard<std::recursive_mutex> mu(m_muAcquireRelease);
  if (m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject) {
    return CL_INVALID_OPERATION;
  }
  return m_itCurrentAcquriedObject->second->MemObjCreateDevMappedRegion(
      pDevice, cmd_param_map, pHostMapDataPtr);
}

/**
 * @fn  cl_err_code
 * GraphicsApiMemoryObject::MemObjReleaseDevMappedRegion(SharedPtr<FissionableDevice>
 * pDevice, cl_dev_cmd_param_map* cmd_param_map)
 */

cl_err_code GraphicsApiMemoryObject::MemObjReleaseDevMappedRegion(
    const SharedPtr<FissionableDevice> &pDevice,
    cl_dev_cmd_param_map *cmd_param_map, void *pHostMapDataPtr,
    bool force_unmap) {
  std::lock_guard<std::recursive_mutex> mu(m_muAcquireRelease);
  if (m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject) {
    return CL_INVALID_OPERATION;
  }
  return m_itCurrentAcquriedObject->second->MemObjReleaseDevMappedRegion(
      pDevice, cmd_param_map, pHostMapDataPtr, force_unmap);
}

bool GraphicsApiMemoryObject::IsSynchDataWithHostRequired(
    cl_dev_cmd_param_map *IN pMapInfo, void *IN pHostMapDataPtr) const {
  std::lock_guard<std::recursive_mutex> mu(m_muAcquireRelease);
  if (m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject) {
    return false;
  }
  return m_itCurrentAcquriedObject->second->IsSynchDataWithHostRequired(
      pMapInfo, pHostMapDataPtr);
}

cl_err_code
GraphicsApiMemoryObject::SynchDataToHost(cl_dev_cmd_param_map *IN pMapInfo,
                                         void *IN pHostMapDataPtr) {
  std::lock_guard<std::recursive_mutex> mu(m_muAcquireRelease);
  if (m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject) {
    return CL_INVALID_OPERATION;
  }
  return m_itCurrentAcquriedObject->second->SynchDataToHost(pMapInfo,
                                                            pHostMapDataPtr);
}

cl_err_code
GraphicsApiMemoryObject::SynchDataFromHost(cl_dev_cmd_param_map *IN pMapInfo,
                                           void *IN pHostMapDataPtr) {
  std::lock_guard<std::recursive_mutex> mu(m_muAcquireRelease);
  if (m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject) {
    return CL_INVALID_OPERATION;
  }
  return m_itCurrentAcquriedObject->second->SynchDataFromHost(pMapInfo,
                                                              pHostMapDataPtr);
}

/**
 * @fn  cl_err_code
 * GraphicsApiMemoryObject::SetAcquireCmdEvent(SharedPtr<OclEvent> pEvent)
 */

cl_err_code
GraphicsApiMemoryObject::SetAcquireCmdEvent(SharedPtr<OclEvent> pEvent) {
  std::lock_guard<std::recursive_mutex> mu(m_muAcquireRelease);

  if (NULL != pEvent.GetPtr()) {
    m_lstAcquiredObjectDescriptors.push_back(
        t_AcquiredObjects::value_type(pEvent, CL_GFX_OBJECT_NOT_ACQUIRED));
    if (m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject) {
      m_itCurrentAcquriedObject = m_lstAcquiredObjectDescriptors.begin();
    }
  } else {
    assert(!m_lstAcquiredObjectDescriptors.empty() &&
           "On Release the Aquired Event list must be NOT empty");

    AcquiredObject &pMemObj = m_lstAcquiredObjectDescriptors.front().second;
    if (CL_GFX_OBJECT_NOT_ACQUIRED == pMemObj) {
      // Nothing to do with NON acquried objects
      return CL_SUCCESS;
    }

    if ((CL_GFX_OBJECT_NOT_READY != pMemObj) &&
        (CL_GFX_OBJECT_FAIL_IN_ACQUIRE != pMemObj)) {
      pMemObj->Release(); // Relase allocated child object
    }
    m_lstAcquiredObjectDescriptors.pop_front();
    m_itCurrentAcquriedObject = m_lstAcquiredObjectDescriptors.begin();
  }

  return CL_SUCCESS;
}

/**
 * @fn  cl_err_code GraphicsApiMemoryObject::ClearAcquireCmdEvent()
 */
cl_err_code GraphicsApiMemoryObject::ClearAcquireCmdEvent() {
  // we get there in case of failure - undo what we did in init

  std::lock_guard<std::recursive_mutex> mu(m_muAcquireRelease);

  // release the last object desc - which was insirted in init
  m_lstAcquiredObjectDescriptors.pop_back();

  if (m_lstAcquiredObjectDescriptors.empty()) {
    m_itCurrentAcquriedObject = m_lstAcquiredObjectDescriptors.end();
  }

  return CL_SUCCESS;
}

/**
 * @fn  cl_err_code
 * GraphicsApiMemoryObject::GetDeviceDescriptor(SharedPtr<FissionableDevice>
 * pDevice, IOCLDevMemoryObject* *ppDevObject, SharedPtr<OclEvent>* ppEvent)
 */

cl_err_code GraphicsApiMemoryObject::GetDeviceDescriptor(
    const SharedPtr<FissionableDevice> &pDevice,
    IOCLDevMemoryObject **ppDevObject, SharedPtr<OclEvent> *ppEvent) {
  std::lock_guard<std::recursive_mutex> mu(m_muAcquireRelease);

  if (m_lstAcquiredObjectDescriptors.empty()) {
    // Trying to get device descriptor before acquire operation was enqueued
    return CL_INVALID_OPERATION;
  }

  // Need to check if retriving curren acquried object descriptor or not
  if (--m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject) {
    if (CL_GFX_OBJECT_NOT_READY == m_itCurrentAcquriedObject->second) {
      // Here the acquire operation is not finished and we need to create child
      // object
      cl_err_code err = CreateChildObject();
      if (CL_FAILED(err)) {
        return err;
      }
    }

    if (CL_GFX_OBJECT_FAIL_IN_ACQUIRE == m_itCurrentAcquriedObject->second) {
      return CL_OUT_OF_RESOURCES;
    }

    SharedPtr<MemoryObject> pCurrentChild = m_itCurrentAcquriedObject->second;
    if (NULL != pCurrentChild.GetPtr()) {
      return pCurrentChild->GetDeviceDescriptor(pDevice, ppDevObject, ppEvent);
    }
  }

  // Retrieving descriptor of the object that still was not aquired

  // Now we need to create event that will updated on acquire completion
  assert(NULL != ppEvent);
  SharedPtr<OclEvent> pNewEvent =
      MemoryObjectEvent::Allocate(ppDevObject, this, pDevice);
  if (NULL == pNewEvent.GetPtr()) {
    return CL_OUT_OF_HOST_MEMORY;
  }

  // Link to the acquire event
  pNewEvent->AddDependentOn(m_lstAcquiredObjectDescriptors.back().first);
  *ppEvent = pNewEvent;
  // Event is born with user RefCount == 1, we should release it
  pNewEvent->Release();

  return CL_NOT_READY;
}

cl_err_code GraphicsApiMemoryObject::UpdateDeviceDescriptor(
    const SharedPtr<FissionableDevice> &IN pDevice,
    IOCLDevMemoryObject *OUT *ppDevObject) {
  std::lock_guard<std::recursive_mutex> mu(m_muAcquireRelease);

  if (m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject) {
    // Trying to get device descriptor before acquire operation was enqueued
    return CL_INVALID_OPERATION;
  }

  if (CL_GFX_OBJECT_NOT_READY == m_itCurrentAcquriedObject->second) {
    // Here the acquire operation is not finished and we need to create child
    // object
    cl_err_code err = CreateChildObject();
    if (CL_FAILED(err)) {
      return err;
    }
  }

  const SharedPtr<MemoryObject> &pCurrentChild =
      m_itCurrentAcquriedObject->second;
  if (NULL != pCurrentChild.GetPtr()) {
    cl_err_code ret =
        pCurrentChild->GetDeviceDescriptor(pDevice, ppDevObject, NULL);
    /*{
        cl_dev_memobj_handle handle;
        (*ppDevObject)->clDevMemObjGetDescriptor(0, 0, &handle);
        if (((long*)((cl_mem_obj_descriptor*)handle)->pData)[0] !=
    0x3edc41ac)
        {
            printf("bug in update!\n");
        }
    }*/
    return ret;
  }

  if (CL_GFX_OBJECT_FAIL_IN_ACQUIRE == m_itCurrentAcquriedObject->second) {
    return CL_OUT_OF_RESOURCES;
  }

  assert(0 && "We should not get to this line. After acquire completed, it "
              "should be a valid object or an error code");
  return CL_INVALID_OPERATION;
}

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
