// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the CLANG_COMPILER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// CLANG_COMPILER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#pragma once

#include <cl_synch_objects.h>
#include <frontend_api.h>
#include "clang_device_info.h"
#include "cl_config.h"
#include "common_clang.h"

namespace Intel { namespace OpenCL { namespace ClangFE {

    class ClangFECompiler : public Intel::OpenCL::FECompilerAPI::IOCLFECompiler
    {
    public:
        ClangFECompiler(const void* pszDeviceInfo);

        // IOCLFECompiler
        int CompileProgram(Intel::OpenCL::FECompilerAPI::FECompileProgramDescriptor* pProgDesc,
                           IOCLFEBinaryResult* *pBinaryResult);

        int LinkPrograms(Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor* pProgDesc,
                         IOCLFEBinaryResult* *pBinaryResult);

        int ParseSPIRV(Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor* pProgDesc,
                         IOCLFEBinaryResult* *pBinaryResult);

        int GetKernelArgInfo(const void*        pBin,
                             size_t             uiBinarySize,
                             const char*        szKernelName,
                             Intel::OpenCL::ClangFE::IOCLFEKernelArgInfo*   *pArgInfo);

        bool CheckCompileOptions(const char*  szOptions,
                                 char*        szUnrecognizedOptions,
                                 size_t       uiUnrecognizedOptionsSize);

        bool CheckLinkOptions(const char*  szOptions,
                              char*        szUnrecognizedOptions,
                              size_t       uiUnrecognizedOptionsSize);

        void Release()
        {
            delete this;
        }

        static void ShutDown();

    protected:
        virtual ~ClangFECompiler();

        CLANG_DEV_INFO m_sDeviceInfo;

        Intel::OpenCL::Utils::BasicCLConfigWrapper m_config;
    };

}}}
