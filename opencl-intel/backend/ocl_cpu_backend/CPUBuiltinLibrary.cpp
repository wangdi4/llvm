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
    char szRTLibSvmlSharedName[MAX_PATH];

    std::string strErr;

    Utils::SystemInfo::GetModuleDirectory(szModuleName, MAX_PATH);

    //Klocwork warning - false alarm the Id is always in correct bounds
    const char* pCPUPrefix = m_cpuId.GetCPUPrefix();

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
    // installed SDK on Windows contains .dll and .rtl except clbltfnshared.rtl in the folders ...\bin\x86 or ...\bin\x64
    // but clbltfnshared.rtl is placed in the ...\bin\common folder
    // or, if it not SDK, but development install, clbltfnshared.rtl is with the other .rtl and .dll together
    llvm::StringRef fName(szModuleName);
    std::string s = fName.str();
    if ( fName.endswith("\\bin\\x64\\") || fName.endswith("\\bin\\x86\\") ) {        
        s.replace(s.end()-5,s.end(),"\\common\\");
    }

    sprintf_s(szRTLibName, MAX_PATH, "%sclbltfn%s.rtl", szModuleName, pCPUPrefix);
    sprintf_s(szRTLibSvmlSharedName, MAX_PATH, "%sclbltfnshared.rtl", s.c_str());
#else
    snprintf(szRTLibName, MAX_PATH, "%sclbltfn%s.rtl", szModuleName, pCPUPrefix);
    snprintf(szRTLibSvmlSharedName, MAX_PATH, "%sclbltfnshared.rtl", szModuleName);
#endif
    // load particular (not shared) RTL optimized for one architecture
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> rtlBufferOrErr = llvm::MemoryBuffer::getFile(szRTLibName);
    if(!rtlBufferOrErr)
    {
        throw Exceptions::DeviceBackendExceptionBase(std::string("Failed to load the builtins rtl library"));
    }
    m_pRtlBuffer = rtlBufferOrErr.get().release();

    // on KNC we don't have shared (common) library
    if (m_cpuId.GetCPU() != MIC_KNC) {
        // on CPU load shared RTL to memory, it will be linked with the particular (not shared) RTL
        llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> rtlBufferSvmlSharedOrErr = llvm::MemoryBuffer::getFile(szRTLibSvmlSharedName);
        if(!rtlBufferSvmlSharedOrErr)   {
            throw Exceptions::DeviceBackendExceptionBase(std::string("Failed to load the shared builtins rtl library"));
        }
        m_pRtlBufferSvmlShared = rtlBufferSvmlSharedOrErr.get().release();
    }
}

}}} // namespace
