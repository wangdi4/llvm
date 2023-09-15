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

#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include "cl_synch_objects.h"
#include "cl_types.h"
#include <map>

namespace Intel {
namespace OpenCL {
namespace CPUDevice {

class MemoryAllocator {

public:
  MemoryAllocator(cl_int devId, IOCLDevLogDescriptor *pLogDesc,
                  cl_ulong maxAllocSize,
                  DeviceBackend::ICLDevBackendImageService *pImageService);
  virtual ~MemoryAllocator();

  cl_dev_err_code GetAllocProperties(cl_mem_object_type IN memObjType,
                                     cl_dev_alloc_prop *OUT pAllocProp);
  // Create/Release functions
  cl_dev_err_code CreateObject(cl_dev_subdevice_id node_id, cl_mem_flags flags,
                               const cl_image_format *format, size_t dim_count,
                               const size_t *dim,
                               IOCLDevRTMemObjectService *pRTMemObjService,
                               IOCLDevMemoryObject **memObj);

  // Utility functions
  static void *CalculateOffsetPointer(void *pBasePtr, cl_uint dim_count,
                                      const size_t *origin, const size_t *pitch,
                                      size_t elemSize);

protected:
  cl_int m_iDevId;
  cl_ulong m_maxAllocSize;
  IOCLDevLogDescriptor *m_pLogDescriptor;
  cl_int m_iLogHandle;
  DeviceBackend::ICLDevBackendImageService *m_pImageService;
};

class CPUDevMemoryObject : public IOCLDevMemoryObject {
public:
  friend class CPUDevMemorySubObject;

  CPUDevMemoryObject(cl_int iLogHandle, IOCLDevLogDescriptor *pLogDescriptor,
                     cl_dev_subdevice_id nodeId, cl_mem_flags memFlags,
                     const cl_image_format *pImgFormat, size_t dimCount,
                     const size_t *dim,
                     IOCLDevRTMemObjectService *pRTMemObjService,
                     DeviceBackend::ICLDevBackendImageService *pImageService);

  virtual ~CPUDevMemoryObject();

  cl_dev_err_code Init();

  cl_dev_err_code
  clDevMemObjCreateMappedRegion(cl_dev_cmd_param_map *pMapParams) override;
  cl_dev_err_code clDevMemObjUnmapAndReleaseMappedRegion(
      cl_dev_cmd_param_map *pMapParams) override {
    return clDevMemObjReleaseMappedRegion(pMapParams);
  }
  cl_dev_err_code
  clDevMemObjReleaseMappedRegion(cl_dev_cmd_param_map *pMapParams) override;
  cl_dev_err_code clDevMemObjRelease() override;
  cl_dev_err_code
  clDevMemObjGetDescriptor(cl_device_type dev_type, cl_dev_subdevice_id node_id,
                           cl_dev_memobj_handle *handle) override;

  cl_dev_err_code
  clDevMemObjCreateSubObject(cl_mem_flags mem_flags, const size_t *origin,
                             const size_t *size,
                             IOCLDevRTMemObjectService IN *pBSService,
                             IOCLDevMemoryObject **ppSubObject) override;

  cl_dev_err_code
  clDevMemObjUpdateBackingStore(void *operation_handle,
                                cl_dev_bs_update_state *pUpdateState) override;
  cl_dev_err_code clDevMemObjUpdateFromBackingStore(
      void *operation_handle, cl_dev_bs_update_state *pUpdateState) override;
  cl_dev_err_code clDevMemObjInvalidateData() override;

protected:
  CPUDevMemoryObject(cl_int iLogHandle, IOCLDevLogDescriptor *pLogDescriptor)
      : m_pLogDescriptor(pLogDescriptor), m_iLogHandle(iLogHandle),
        m_nodeId(nullptr), m_memFlags(0), m_pRTMemObjService(nullptr),
        m_pBackingStore(nullptr), m_pImageService(nullptr) {}

  IOCLDevLogDescriptor *m_pLogDescriptor;
  cl_int m_iLogHandle;

  // Object Management
  cl_dev_subdevice_id m_nodeId;

  cl_mem_obj_descriptor m_objDecr;
  cl_mem_flags m_memFlags;
  IOCLDevRTMemObjectService *m_pRTMemObjService;
  IOCLDevBackingStore *m_pBackingStore;
  DeviceBackend::ICLDevBackendImageService *m_pImageService;
};

class CPUDevMemorySubObject : public CPUDevMemoryObject {
public:
  CPUDevMemorySubObject(cl_int iLogHandle, IOCLDevLogDescriptor *pLogDescriptor,
                        CPUDevMemoryObject *pParent);

  cl_dev_err_code Init(cl_mem_flags mem_flags, const size_t *origin,
                       const size_t *size,
                       IOCLDevRTMemObjectService IN *pBSService);

protected:
  CPUDevMemoryObject *m_pParent;
};

} // namespace CPUDevice
} // namespace OpenCL
} // namespace Intel
