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

#include "GraphicsApiMemoryObject.h"
#include "enqueue_commands.h"

namespace Intel {
namespace OpenCL {
namespace Framework {
/**
 * @class   SyncGraphicsApiObjects
 *
 * @brief   Common base class for synchronization of graphics API objects, such
 * as OpenGL and Direct3D
 *
 * @author  Aharon
 * @date    7/13/2011
 *
 * @sa  Intel::OpenCL::Framework::RuntimeCommand
 */

class SyncGraphicsApiObjects : public RuntimeCommand {
  const bool m_bIsAcquireCmd;
  const cl_command_type m_cmdType;
  const size_t m_uiMemObjNum;
  SharedPtr<GraphicsApiMemoryObject> *m_pMemObjects;

protected:
  /**
   * @fn  SyncGraphicsApiObjects::SyncGraphicsApiObjects(cl_command_type
   * cmdType, unsigned int uiMemObjNum, SharedPtr<IOclCommandQueueBase>
   * cmdQueue, ocl_entry_points * pOclEntryPoints,
   * SharedPtr<GraphicsApiMemoryObject>* pMemObjects, bool bIsAcquireCmd);
   *
   * @brief   Constructor.
   *
   * @author  Aharon
   * @date    7/13/2011
   */

  SyncGraphicsApiObjects(cl_command_type cmdType, size_t uiMemObjNum,
                         SharedPtr<IOclCommandQueueBase> cmdQueue,
                         SharedPtr<GraphicsApiMemoryObject> *pMemObjects,
                         bool bIsAcquireCmd);

public:
  /**
   * @fn  SyncGraphicsApiObjects::~SyncGraphicsApiObjects();
   *
   * @brief   Finaliser.
   *
   * @author  Aharon
   * @date    7/13/2011
   */

  virtual ~SyncGraphicsApiObjects();

  SyncGraphicsApiObjects(const SyncGraphicsApiObjects &s) = delete;
  SyncGraphicsApiObjects &operator=(const SyncGraphicsApiObjects &s) = delete;

  /**
   * @fn  unsigned int SyncGraphicsApiObjects::GetNumMemObjs() const
   *
   * @brief   Gets the number of memory objects.
   *
   * @author  Aharon
   * @date    7/13/2011
   *
   * @return  The number of memory objects.
   */

  size_t GetNumMemObjs() const { return m_uiMemObjNum; }

  // inherited methods:

  virtual cl_command_type GetCommandType() const override { return m_cmdType; }

  virtual ECommandExecutionType GetExecutionType() const override {
    return RUNTIME_EXECUTION_TYPE;
  }

  virtual cl_err_code Init() override;

  cl_err_code CommandDone() override;

  virtual bool isControlCommand() const override { return false; }

  /**
   * @fn  const GraphicsApiMemoryObject&
   * SyncGraphicsApiObjects::GetMemoryObject(size_t Index) const
   *
   * @brief   Gets a memory object.
   *
   * @author  Aharon
   * @date    7/13/2011
   *
   * @param   Index   Zero-based index of the memory object.
   *
   * @return  The memory object.
   */

  const GraphicsApiMemoryObject &GetMemoryObject(size_t Index) const {
    return *m_pMemObjects[Index];
  }

  /**
   * @fn  GraphicsApiMemoryObject&
   * SyncGraphicsApiObjects::GetMemoryObject(size_t Index)
   *
   * @brief   Gets a memory object.
   *
   * @author  Aharon
   * @date    7/13/2011
   *
   * @param   Index   Zero-based index of the memory object.
   *
   * @return  The memory object.
   */

  GraphicsApiMemoryObject &GetMemoryObject(size_t Index) {
    return *m_pMemObjects[Index];
  }
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
