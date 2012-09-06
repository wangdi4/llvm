/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  MICBuiltinLibrary.cpp

\*****************************************************************************/

#include "CPUBuiltinLibrary.h"
#include "exceptions.h"
#include "SystemInfo.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/DynamicLibrary.h"

#include <stdio.h>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <linux/limits.h>
    #define MAX_PATH PATH_MAX
#endif


namespace Intel { namespace OpenCL { namespace DeviceBackend {

void CPUBuiltinLibrary::Load()
{
    char szModuleName[MAX_PATH];
    char szRTLibName[MAX_PATH];
    std::string strErr;

    Utils::SystemInfo::GetModuleDirectory(szModuleName, MAX_PATH);

    //Klocwork warning - false alarm the Id is always in correct bounds
    const char* pCPUPrefix = m_cpuId.GetCPUPrefix();

    if( Intel::CPU_SANDYBRIDGE == m_cpuId.GetCPU() && !m_cpuId.HasAVX1())
    {
        // Use SSE4 if AVX1 is not supported
        pCPUPrefix = Intel::CPUId::GetCPUPrefix(Intel::CPU_COREI7, sizeof(void*)==8);
    }

    // Load SVML functions
#if defined (_WIN32)
    sprintf_s(szRTLibName, MAX_PATH, "%s__ocl_svml_%s.dll", szModuleName, pCPUPrefix);
#else
    snprintf(szRTLibName, MAX_PATH, "%s__ocl_svml_%s.so", szModuleName, pCPUPrefix);
#endif
    if (llvm::sys::DynamicLibrary::LoadLibraryPermanently(szRTLibName,&strErr)) 
    {
        throw Exceptions::DeviceBackendExceptionBase(std::string("Loading SVML library failed - ") + strErr);
    }
    
    // Load LLVM built-ins module
#if defined (_WIN32)
    sprintf_s(szRTLibName, MAX_PATH, "%sclbltfn%s.rtl", szModuleName, pCPUPrefix);
#else
    snprintf(szRTLibName, MAX_PATH, "%sclbltfn%s.rtl", szModuleName, pCPUPrefix);
#endif
     
    llvm::error_code ret = llvm::MemoryBuffer::getFile(szRTLibName, m_pRtlBuffer);
    if( !m_pRtlBuffer  || ret.value() != 0)
    {
        throw Exceptions::DeviceBackendExceptionBase(std::string("Failed to load the builtins rtl library"));
    }
}

}}} // namespace
