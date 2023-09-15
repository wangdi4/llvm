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

#include "sync_graphics_api_objects.h"
#include "command_queue.h"

namespace Intel {
namespace OpenCL {
namespace Framework {
/**
 * @fn  SyncGraphicsApiObjects::SyncGraphicsApiObjects(cl_command_type cmdType,
 *      unsigned int uiMemObjNum, SharedPtr<IOclCommandQueueBase> cmdQueue,
 *      ocl_entry_points * pOclEntryPoints, SharedPtr<GraphicsApiMemoryObject>*
 * pMemObjects)
 */

SyncGraphicsApiObjects::SyncGraphicsApiObjects(
    cl_command_type cmdType, size_t uiMemObjNum,
    SharedPtr<IOclCommandQueueBase> cmdQueue,
    SharedPtr<GraphicsApiMemoryObject> *pMemObjects, bool bIsAcquireCmd)
    : RuntimeCommand(cmdQueue), m_bIsAcquireCmd(bIsAcquireCmd),
      m_cmdType(cmdType), m_uiMemObjNum(uiMemObjNum) {
  m_pMemObjects = new SharedPtr<GraphicsApiMemoryObject>[uiMemObjNum];
  for (size_t i = 0; i < uiMemObjNum; i++) {
    m_pMemObjects[i] = pMemObjects[i];
  }
}

/**
 * @fn  SyncGraphicsApiObjects::~SyncGraphicsApiObjects()
 */

SyncGraphicsApiObjects::~SyncGraphicsApiObjects() {
  if (m_pMemObjects)
    delete[] m_pMemObjects;
}

/**
 * @fn  cl_err_code SyncGraphicsApiObjects::Init()
 */

cl_err_code SyncGraphicsApiObjects::Init() {
  if (m_bIsAcquireCmd) {
    for (unsigned int i = 0; i < m_uiMemObjNum; ++i) {
      m_pMemObjects[i]->SetAcquireCmdEvent(m_Event);
    }
  }
  return CL_SUCCESS;
}

/**
 * @fn  cl_err_code SyncGraphicsApiObjects::CommandDone()
 */

cl_err_code SyncGraphicsApiObjects::CommandDone() {
  if (m_bIsAcquireCmd) {
    // if Acquire command succeed - do nothing

    if (GetReturnCode() != CL_SUCCESS) {
      // if Acquire command failed, we need to undo what we did in Init
      for (unsigned int i = 0; i < m_uiMemObjNum; i++) {
        m_pMemObjects[i]->ClearAcquireCmdEvent();
      }
    }
  } else {
    // not m_bIsAcquireCmd e.g. release

    if (GetReturnCode() == CL_SUCCESS) {
      for (unsigned int i = 0; i < m_uiMemObjNum; i++) {
        m_pMemObjects[i]->SetAcquireCmdEvent(nullptr);
      }
    }
    // if release command failed - do nothing
  }
  if (m_pMemObjects) {
    delete[] m_pMemObjects;
    m_pMemObjects = nullptr;
  }
  return CL_SUCCESS;
}

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
