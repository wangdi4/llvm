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

#include "svm_commands.h"
#include "cl_shared_ptr.hpp"
#include "cl_sys_info.h"
#include "cl_user_logger.h"
#include "command_queue.h"
#include "context_module.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

cl_err_code SVMFreeCommand::Execute() {
  if (m_freeFunc != nullptr) {
    if (FrameworkUserLogger::GetInstance()->IsApiLoggingEnabled()) {
      std::stringstream stream;
      stream << "SVMFreeCommand callback(" << GetCommandQueue()->GetHandle()
             << ", " << m_svmPtrs.size() << ", " << &m_svmPtrs[0] << ", "
             << m_pUserData << ")" << std::endl;
      FrameworkUserLogger::GetInstance()->PrintString(stream.str());
    }
    m_freeFunc(GetCommandQueue()->GetHandle(), m_svmPtrs.size(), &m_svmPtrs[0],
               m_pUserData);
  } else {
    SharedPtr<Context> pContext = GetCommandQueue()->GetContext();
    for (std::vector<void *>::const_iterator iter = m_svmPtrs.begin();
         iter != m_svmPtrs.end(); iter++) {
      pContext->SVMFree(*iter);
    }
  }
  return RuntimeCommand::Execute();
}

cl_err_code RuntimeSVMMemcpyCommand::Execute() {
  NotifyCmdStatusChanged(CL_RUNNING, CL_SUCCESS, HostTime());

  MEMCPY_S(m_pDstPtr, m_size, m_pSrcPtr, m_size);

  NotifyCmdStatusChanged(CL_COMPLETE, CL_SUCCESS, HostTime());

  return m_returnCode;
}

cl_err_code RuntimeSVMMemFillCommand::Execute() {
  NotifyCmdStatusChanged(CL_RUNNING, CL_SUCCESS, HostTime());

  CopyPattern(m_pPattern, m_szPatternSize, m_pSvmPtr, m_size);

  NotifyCmdStatusChanged(CL_COMPLETE, CL_SUCCESS, HostTime());

  return m_returnCode;
}
