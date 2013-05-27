// Copyright (c) 2006-2007 Intel Corporation
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
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////
#include "crt_config.h"
#include "crt_registry.h"

CrtConfig::CrtConfig()
{
};

CrtConfig::~CrtConfig()
{
}


cl_uint CrtConfig::getNumPlatforms()
{
    return (cl_uint)m_libraryNames.size();
}

std::string& CrtConfig::getPlatformLibName(cl_uint index)
{
    return m_libraryNames[index];
}

crt_err_code CrtConfig::Init()
{
    crt_err_code err = CRT_SUCCESS;
    char pCpuPath[MAX_PATH];

    if(OCLCRT::Utils::GetCpuPathFromRegistry(pCpuPath) == true)
    {
#if defined(_WIN64)
        sprintf_s(pCpuPath, MAX_PATH, "%s%s", pCpuPath, "\\bin\\x64");
#else
        sprintf_s(pCpuPath, MAX_PATH, "%s%s", pCpuPath, "\\bin\\x86");
#endif
        SetDllDirectory(pCpuPath);
    }

#if defined(_WIN64)
    m_libraryNames.push_back("intelocl64.dll");
    m_libraryNames.push_back("igdrcl64.dll");
#else
    m_libraryNames.push_back("intelocl32.dll");
    m_libraryNames.push_back("igdrcl32.dll");
#endif

    return err;
}
