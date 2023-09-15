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

#pragma once

#include "MemoryObject.h"

#include <list>
#include <mutex>

namespace Intel {
namespace OpenCL {
namespace Framework {

/**
 * @class   GraphicsApiMemoryObject
 *
 * @brief   Common base class for Graphics API MemoryObjects, like OpenGL and
 * Direct3D
 *
 * @author  Aharon
 * @date    7/13/2011
 *
 * @sa  Intel::OpenCL::Framework::MemoryObject
 */

class GraphicsApiMemoryObject : public MemoryObject {

public:
  /**
   * @fn  GraphicsApiMemoryObject::GraphicsApiMemoryObject(SharedPtr<Context>
   * pContext, ocl_entry_points* pOclEntryPoints)
   *
   * @brief   Constructor.
   *
   * @author  Aharon
   * @date    7/13/2011
   */

  GraphicsApiMemoryObject(SharedPtr<Context> pContext)
      : MemoryObject(pContext),
        m_itCurrentAcquriedObject(m_lstAcquiredObjectDescriptors.end()) {}

public:
  PREPARE_SHARED_PTR(GraphicsApiMemoryObject)

  /**
   * @fn  virtual GraphicsApiMemoryObject::~GraphicsApiMemoryObject();
   *
   * @brief   Finaliser.
   *
   * @author  Aharon
   * @date    7/13/2011
   */

  virtual ~GraphicsApiMemoryObject();

  cl_err_code SetAcquireCmdEvent(
      SharedPtr<OclEvent>
          pEvent); // Set Event of Acquire command that belongs to the object.

  cl_err_code ClearAcquireCmdEvent(); // Clear Event of Acquire command that
                                      // belongs to the object.

  // inherited methods:
  virtual cl_err_code LockOnDevice(IN const SharedPtr<FissionableDevice> &dev,
                                   IN MemObjUsage usage,
                                   OUT MemObjUsage *pOutUsageLocked,
                                   OUT SharedPtr<OclEvent> &pOutEvent) override;
  virtual cl_err_code UnLockOnDevice(IN const SharedPtr<FissionableDevice> &dev,
                                     IN MemObjUsage usage) override;

  virtual cl_err_code UpdateHostPtr(cl_mem_flags clMemFlags,
                                    void *pHostPtr) override;

  virtual cl_err_code CheckBoundsRect(const size_t *pszOrigin,
                                      const size_t *pszRegion,
                                      size_t szRowPitch,
                                      size_t szSlicePitch) const override;

  virtual void *
  GetBackingStoreData(const size_t *pszOrigin = NULL) const override;

  virtual cl_err_code
  CreateDeviceResource(const SharedPtr<FissionableDevice> &pDevice) override;

  virtual cl_err_code
  GetDeviceDescriptor(const SharedPtr<FissionableDevice> &pDevice,
                      IOCLDevMemoryObject **ppDevObject,
                      SharedPtr<OclEvent> *ppEvent) override;

  virtual cl_err_code
  UpdateDeviceDescriptor(const SharedPtr<FissionableDevice> &IN pDevice,
                         IOCLDevMemoryObject *OUT *ppDevObject) override;

  virtual bool
  IsSupportedByDevice(const SharedPtr<FissionableDevice> &pDevice) override;

  // In the case when Backing Store region is different from Host Map pointer
  // provided by user we need to synchronize user area with device area
  // after/before each map/unmap command
  //
  virtual bool
  IsSynchDataWithHostRequired(cl_dev_cmd_param_map *IN pMapInfo,
                              void *IN pHostMapDataPtr) const override;
  virtual cl_err_code SynchDataToHost(cl_dev_cmd_param_map *IN pMapInfo,
                                      void *IN pHostMapDataPtr) override;
  virtual cl_err_code SynchDataFromHost(cl_dev_cmd_param_map *IN pMapInfo,
                                        void *IN pHostMapDataPtr) override;

protected:
  // This function is responsible for creating a supporting child object
  virtual cl_err_code CreateChildObject() = 0;

  // inherited methods:

  virtual cl_err_code
  MemObjCreateDevMappedRegion(const SharedPtr<FissionableDevice> &,
                              cl_dev_cmd_param_map *cmd_param_map,
                              void **pHostMapDataPtr) override;

  virtual cl_err_code MemObjReleaseDevMappedRegion(
      const SharedPtr<FissionableDevice> &, cl_dev_cmd_param_map *cmd_param_map,
      void *pHostMapDataPtr, bool force_unmap = false) override;

  mutable std::recursive_mutex m_muAcquireRelease;
  // This list hold the ordered events which represent acquire commands and
  // appropriate child object, the latest event located in the tail. Scenario:
  // clEnqueueAcquireGL...()
  // clEnqueuNDRange()
  // clEnqueueReleaseGL...()
  // clEnqueueAcquireGL...()
  // clEnqueuNDRange()
  // clEnqueueReleaseGL...()
  // ...
  // clFinish()

  enum ClGfxObjectState {
    CL_GFX_OBJECT_VALID,
    CL_GFX_OBJECT_NOT_READY,
    CL_GFX_OBJECT_NOT_ACQUIRED,
    CL_GFX_OBJECT_FAIL_IN_ACQUIRE
  };

  class AcquiredObject : public SharedPtr<MemoryObject> {
  public:
    AcquiredObject(ClGfxObjectState state, const SharedPtr<MemoryObject> &pObj =
                                               SharedPtr<MemoryObject>())
        : SharedPtr<MemoryObject>(pObj), m_state(state) {
      assert(Invarient());
    }

    AcquiredObject(const SharedPtr<MemoryObject> &pObj)
        : SharedPtr<MemoryObject>(pObj), m_state(CL_GFX_OBJECT_VALID) {
      assert(Invarient());
    }

    operator ClGfxObjectState() { return m_state; }

  private:
    bool Invarient() {
      return !(CL_GFX_OBJECT_VALID == m_state && NULL == m_ptr) &&
             !(CL_GFX_OBJECT_VALID != m_state && NULL != m_ptr);
    }

    ClGfxObjectState m_state;
  };

  // The list is protectd by m_muAcquireRelease
  typedef std::list<std::pair<SharedPtr<OclEvent>, AcquiredObject>>
      t_AcquiredObjects;
  t_AcquiredObjects m_lstAcquiredObjectDescriptors;
  t_AcquiredObjects::iterator m_itCurrentAcquriedObject;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
