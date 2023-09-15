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

#include "MemoryObjectFactory.h"
#include "CL/cl.h"
#include "Context.h"
#include "cl_shared_ptr.hpp"
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Framework;

MemoryObjectFactory *MemoryObjectFactory::Instance = nullptr;

MemoryObjectFactory *MemoryObjectFactory::GetInstance() {
  static MemoryObjectFactory *S = [] {
    Instance = new MemoryObjectFactory;
    return Instance;
  }();

  return S;
}

bool MemoryObjectFactory::FactoryKey::operator<(
    const FactoryKey &_Right) const {
  return ((clObjType < _Right.clObjType) ||
          ((clObjType == _Right.clObjType) &&
           (iSupportedDevices < _Right.iSupportedDevices)) ||
          ((clObjType == _Right.clObjType) &&
           (iSupportedDevices == _Right.iSupportedDevices) &&
           (iGfxSysSharing < _Right.iGfxSysSharing))) ||
         ((clObjType == _Right.clObjType) &&
          (iSupportedDevices == _Right.iSupportedDevices) &&
          (iGfxSysSharing == _Right.iGfxSysSharing) &&
          (iAuxId < _Right.iAuxId));
}

void MemoryObjectFactory::RegisterMemoryObjectCreator(
    cl_bitfield iSupportedDevices, int iGfxSysSharing,
    cl_mem_object_type clObjType, int iAuxId,
    fn_MemoryObjectCreator *pMemObjCreator) {
  FactoryKey key;

  key.clObjType = clObjType;
  key.iSupportedDevices = iSupportedDevices;
  key.iGfxSysSharing = iGfxSysSharing;
  key.iAuxId = iAuxId;

  assert(m_memObjMap.find(key) == m_memObjMap.end());
  m_memObjMap[key] = pMemObjCreator;
}

cl_err_code MemoryObjectFactory::CreateMemoryObject(
    cl_bitfield iRequiredDevices, cl_mem_object_type clObjType,
    int iGfxSysSharing, SharedPtr<Context> pContext,
    SharedPtr<MemoryObject> *pMemObject, int iAuxId) {
  FactoryKey key;

  assert(NULL != pMemObject);

  key.clObjType = clObjType;
  key.iSupportedDevices = iRequiredDevices;
  key.iGfxSysSharing = iGfxSysSharing;
  key.iAuxId = iAuxId;

  std::map<FactoryKey, fn_MemoryObjectCreator *>::iterator it =
      m_memObjMap.find(key);
  if (it == m_memObjMap.end()) {
    // try generic device
    key.iSupportedDevices = CL_DEVICE_TYPE_ALL;
    it = m_memObjMap.find(key);
    if (it == m_memObjMap.end()) {
      return CL_ERR_FAILURE;
    }
  }
  SharedPtr<MemoryObject> pMemObj = it->second(pContext, clObjType);
  if (NULL == pMemObj.GetPtr()) {
    return CL_OUT_OF_HOST_MEMORY;
  }

  *pMemObject = pMemObj;
  return CL_SUCCESS;
}
