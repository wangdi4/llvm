//==---- clang_copmiler.h --- OpenCL front-end compiler -------*- C++ -*---=
//
// Copyright (C) 2009-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------===

#pragma once

#include <frontend_api.h>
#include "clang_device_info.h"
#include "cl_config.h"

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

