// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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

#include "enqueue_commands.h"
#include <vector>

namespace Intel {
namespace OpenCL {
namespace Framework {

/**
 * This class is responsible for performing the command enqueued by
 * clEnqueueUSMMemcpy in case both destination and source pointers are system
 * pointers
 */
class RuntimeUSMMemcpyCommand : public RuntimeCommand {
public:
  /**
   * Constructor
   * @param pDstPtr a pointer to the destination
   * @param pSrcPtr a pointer to the source
   * @param size the size of the memory region to copy
   * @param cmdQueue a pointer to the IOclCommandQueueBase on which
   * this command is enqueued
   * @param bIsDependentOnEvents whether this command is dependent on some
   * events
   */
  RuntimeUSMMemcpyCommand(void *pDstPtr, const void *pSrcPtr, size_t size,
                          const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                          bool bIsDependentOnEvents)
      : RuntimeCommand(cmdQueue, bIsDependentOnEvents), m_pDstPtr(pDstPtr),
        m_pSrcPtr(pSrcPtr), m_size(size) {}

  // overriden methods:

  cl_err_code Execute() {
    MEMCPY_S(m_pDstPtr, m_size, m_pSrcPtr, m_size);
    return RuntimeCommand::Execute();
  };

  cl_command_type GetCommandType() const { return CL_COMMAND_MEMCPY_INTEL; }

  const char *GetCommandName() const { return "CL_COMMAND_MEMCPY_INTEL"; }

private:
  void *const m_pDstPtr;
  const void *const m_pSrcPtr;
  const size_t m_size;
};

/**
 * This class is responsible for performing the command enqueued by
 * clEnqueueUSMMemset in case the USM pointer is a system pointer
 */
class RuntimeUSMMemsetCommand : public RuntimeCommand {
public:
  /**
   * Constructor
   * @param pUsmPtr a pointer to a memory region that will be set with the
   * pattern
   * @param pPattern a pointer to the data pattern
   * @param szPatternSize size in bytes of the pattern
   * @param size size in bytes of the region being set
   * @param cmdQueue a pointer to the IOclCommandQueueBase on which
   * this command is enqueued
   * @param bIsDependentOnEvents whether this command is dependent on some
   * events
   */
  RuntimeUSMMemsetCommand(void *pUsmPtr, const void *pPattern,
                          size_t szPatternSize, size_t size,
                          const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                          bool bIsDependentOnEvents)
      : RuntimeCommand(cmdQueue, bIsDependentOnEvents), m_pUsmPtr(pUsmPtr),
        m_pPattern(pPattern), m_szPatternSize(szPatternSize), m_size(size) {}

  // overriden methods:

  cl_err_code Execute() {
    CopyPattern(m_pPattern, m_szPatternSize, m_pUsmPtr, m_size);
    return RuntimeCommand::Execute();
  }

  cl_command_type GetCommandType() const { return CL_COMMAND_MEMSET_INTEL; }

  const char *GetCommandName() const { return "CL_COMMAND_MEMSET_INTEL"; }

private:
  void *const m_pUsmPtr;
  const void *const m_pPattern;
  const size_t m_szPatternSize;
  const size_t m_size;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
