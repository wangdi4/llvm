// Copyright (c) 2006-2013 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

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
