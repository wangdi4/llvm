// Copyright (c) 2006-2014 Intel Corporation
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

#include "UserLoggerOutputParams.h"

using namespace Intel::OpenCL::Framework;

void OutputParamsValueProvider::Print2Logger()
{
    Intel::OpenCL::Utils::UserLogger& instance = GetUserLoggerInstance();
    instance.SetExpectOutputParams(false);
    if (CL_SUCCEEDED(instance.GetLastRetVal()))
    {
        for (std::vector<ParamInfo>::const_iterator iter = m_outputParamsVec.begin(); iter != m_outputParamsVec.end(); ++iter)
        {
            instance.PrintOutputParam(iter->m_name, iter->m_addr, iter->m_size, iter->m_bIsPtr2Ptr, iter->m_bIsUnsigned);
        }
        if (NULL != m_specialPrinter)
        {
            const std::string str2Print = m_specialPrinter->GetStringToPrint();
            if (!str2Print.empty())
            {
                instance.PrintString(", " + str2Print, false);
            }
        }
    }
    instance.PrintString("\n", false);
}
