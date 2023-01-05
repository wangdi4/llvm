// INTEL CONFIDENTIAL
//
// Copyright 2006-2020 Intel Corporation.
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

#include "cl_sys_info.h"
#include "enqueue_commands.h"
#include <vector>

namespace Intel {
namespace OpenCL {
namespace Framework {

/**
 * This class is responsible for performing the command enqueued by
 * clEnqueueMemcpyINTEL in case both destination and source pointers are system
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

  cl_err_code Execute() override {
    NotifyCmdStatusChanged(CL_RUNNING, CL_SUCCESS,
                           Intel::OpenCL::Utils::HostTime());

    MEMCPY_S(m_pDstPtr, m_size, m_pSrcPtr, m_size);

    NotifyCmdStatusChanged(CL_COMPLETE, CL_SUCCESS,
                           Intel::OpenCL::Utils::HostTime());

    return m_returnCode;
  };

  cl_command_type GetCommandType() const override {
    return CL_COMMAND_MEMCPY_INTEL;
  }

  const char *GetCommandName() const override {
    return "CL_COMMAND_MEMCPY_INTEL";
  }

private:
  void *const m_pDstPtr;
  const void *const m_pSrcPtr;
  const size_t m_size;
};

/**
 * This class is responsible for performing the command enqueued by
 * clEnqueueMemFillINTEL in case the USM pointer is a system pointer
 */
class RuntimeUSMMemFillCommand : public RuntimeCommand {
public:
  /**
   * Constructor
   * @param usmPtr a pointer to a memory region that will be filled with the
   * pattern
   * @param pattern a pointer to the data pattern
   * @param patternSize size in bytes of the pattern
   * @param size size in bytes of the region being filled
   * @param cmdQueue a pointer to the IOclCommandQueueBase on which
   * this command is enqueued
   * @param isDependentOnEvents whether this command is dependent on some events
   */
  RuntimeUSMMemFillCommand(void *usmPtr, const void *pattern,
                           size_t patternSize, size_t size,
                           const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                           bool isDependentOnEvents)
      : RuntimeCommand(cmdQueue, isDependentOnEvents), m_usmPtr(usmPtr),
        m_pattern(pattern), m_patternSize(patternSize), m_size(size) {}

  // overriden methods:

  cl_err_code Execute() override {
    NotifyCmdStatusChanged(CL_RUNNING, CL_SUCCESS,
                           Intel::OpenCL::Utils::HostTime());

    CopyPattern(m_pattern, m_patternSize, m_usmPtr, m_size);

    NotifyCmdStatusChanged(CL_COMPLETE, CL_SUCCESS,
                           Intel::OpenCL::Utils::HostTime());

    return m_returnCode;
  }

  cl_command_type GetCommandType() const override {
    return CL_COMMAND_MEMFILL_INTEL;
  }

  const char *GetCommandName() const override {
    return "CL_COMMAND_MEMFILL_INTEL";
  }

private:
  void *const m_usmPtr;
  const void *const m_pattern;
  const size_t m_patternSize;
  const size_t m_size;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
