// Copyright (c) 2006-2009 Intel Corporation
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

///////////////////////////////////////////////////////////
//  clang_driver.h
///////////////////////////////////////////////////////////

#pragma once

#include "clang_device_info.h"
#include <frontend_api.h>
#include <cl_synch_objects.h>
#include "cl_config.h"

#include <string>
#include <list>
#include <vector>

namespace Intel { namespace OpenCL { namespace ClangFE {
    typedef std::list<std::string> ArgListType;

    class ClangFETask
    {
    protected:
        static Intel::OpenCL::Utils::OclMutex s_serializingMutex;
    };

    class ClangFECompilerCompileTask : ClangFETask
    {
    public:
        ClangFECompilerCompileTask(Intel::OpenCL::FECompilerAPI::FECompileProgramDescriptor* pProgDesc,
                                   Intel::OpenCL::ClangFE::CLANG_DEV_INFO sDeviceInfo,
                                   const Intel::OpenCL::Utils::BasicCLConfigWrapper& config)
        : m_pProgDesc(pProgDesc),
          m_sDeviceInfo(sDeviceInfo),
          m_config(config)
        {}

        ~ClangFECompilerCompileTask()
        {}

        int Compile(IOCLFEBinaryResult* *pBinaryResult);

    private:
        ClangFECompilerCompileTask(const ClangFECompilerCompileTask&);
        ClangFECompilerCompileTask& operator= (ClangFECompilerCompileTask const &);

    private:
        Intel::OpenCL::FECompilerAPI::FECompileProgramDescriptor* m_pProgDesc;
        Intel::OpenCL::ClangFE::CLANG_DEV_INFO    m_sDeviceInfo;

        int  CLSTDSet;
        std::string m_triple;

        const Intel::OpenCL::Utils::BasicCLConfigWrapper& m_config;
    };

    class ClangFECompilerLinkTask : ClangFETask
    {
    public:
        ClangFECompilerLinkTask(Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor *pProgDesc)
            : m_pProgDesc(pProgDesc)
        {}

        int Link(IOCLFEBinaryResult* *pBinaryResult);
    private:
        Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor* m_pProgDesc;
    };

    class ClangFECompilerGetKernelArgInfoTask : ClangFETask
    {
    public:
        ClangFECompilerGetKernelArgInfoTask(){}
        virtual ~ClangFECompilerGetKernelArgInfoTask(){}

        int GetKernelArgInfo(const void*    pBin,
                             size_t         uiBinarySize,
                             const char*    szKernelName,
                             IOCLFEKernelArgInfo** ppResult);
    };

    // ClangFECompilerCheckCompileOptions
    // Input: szOptions - a string representing the compile options
    // Output: szUnrecognizedOptions - a new string containing the unrecognized options separated by spaces
    // Returns: 'true' if the compile options are legal and 'false' otherwise
    bool ClangFECompilerCheckCompileOptions(const char*  szOptions,
                                            char*        szUnrecognizedOptions,
                                            size_t       uiUnrecognizedOptionsSize,
                                            const Intel::OpenCL::Utils::BasicCLConfigWrapper& config);

    // ClangFECompilerCheckLinkOptions
    // Input: szOptions - a string representing the link options
    // Output: szUnrecognizedOptions - a new string containing the unrecognized options separated by spaces
    // Returns: 'true' if the link options are legal and 'false' otherwise
    bool ClangFECompilerCheckLinkOptions(const char*  szOptions,
                                         char*        szUnrecognizedOptions,
                                         size_t       uiUnrecognizedOptionsSize);
}}}
