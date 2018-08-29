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

#include "svm_commands.h"
#include "command_queue.h"
#include "context_module.h"
#include "cl_user_logger.h"

using namespace Intel::OpenCL::Framework;

cl_err_code SVMFreeCommand::Execute()
{
    if (m_freeFunc != nullptr)
    {
        if (nullptr != g_pUserLogger && g_pUserLogger->IsApiLoggingEnabled())
        {
            std::stringstream stream;
            stream << "SVMFreeCommand callback(" << GetCommandQueue()->GetHandle() << ", " << m_svmPtrs.size() << ", " << &m_svmPtrs[0] << ", " << m_pUserData << ")"
                << std::endl;
            g_pUserLogger->PrintString(stream.str());
        }
        m_freeFunc(GetCommandQueue()->GetHandle(), m_svmPtrs.size(), &m_svmPtrs[0], m_pUserData);
    }
    else
    {
        SharedPtr<Context> pContext = GetCommandQueue()->GetContext();
        for (std::vector<void* >::const_iterator iter = m_svmPtrs.begin(); iter != m_svmPtrs.end(); iter++)
        {
            pContext->SVMFree(*iter);
        }
    }
    return RuntimeCommand::Execute();
}

cl_err_code RuntimeSVMMemcpyCommand::Execute()
{
	MEMCPY_S(m_pDstPtr, m_size, m_pSrcPtr, m_size);
	return RuntimeCommand::Execute();
}

cl_err_code RuntimeSVMMemFillCommand::Execute()
{
	CopyPattern(m_pPattern, m_szPatternSize, m_pSvmPtr, m_size);
	return RuntimeCommand::Execute();
}
