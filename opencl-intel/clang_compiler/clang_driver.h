//==---- clang_driver.h ----- OpenCL front-end compiler -------*- C++ -*---=
//
// Copyright (C) 2009-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------===

#pragma once

#include "common_clang.h"
#include "clang_device_info.h"
#include <frontend_api.h>
#include <cl_synch_objects.h>
#include "cl_config.h"

#include "llvm/ADT/SmallVector.h"

#include <list>
#include <vector>
#include <string>
#include <cstdint>

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

    // SPIR-V -> llvm::Module converter wrapper.
    class ClangFECompilerParseSPIRVTask : ClangFETask
    {
    public:
         ClangFECompilerParseSPIRVTask(
             Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor* pProgDesc,
             Intel::OpenCL::ClangFE::CLANG_DEV_INFO const& sDeviceInfo);
        /// \brief Translate SPIR-V into LLVM-IR
        /// \return error code of clCompileProgram API
        int ParseSPIRV(IOCLFEBinaryResult* *pBinaryResult);
    private:
        /// \brief Read 32bit integer value and convert it to little-endian if necessary
        std::uint32_t getSPIRVWord(std::uint32_t const* wordPtr) const;
        /// \brief Check a SPIR-V module's version, capabilities, and memory model are supported
        bool isSPIRVSupported() const;

        Intel::OpenCL::FECompilerAPI::FESPIRVProgramDescriptor* m_pProgDesc;
        Intel::OpenCL::ClangFE::CLANG_DEV_INFO                  m_sDeviceInfo;
        bool m_littleEndian; ///< True if SPIR-V module byte order is little-endian
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

    class OCLFEBinaryResult : public IOCLFEBinaryResult {
        // IOCLFEBinaryResult
    public:
        size_t GetIRSize() const override { return m_IRBuffer.size(); }
        const void *GetIR() const override { return m_IRBuffer.data(); }
        const char *GetIRName() const override { return m_IRName.c_str(); }
        IR_TYPE GetIRType() const override { return m_type; }
        const char *GetErrorLog() const override { return m_log.c_str(); }
        void Release() override { delete this; }
        // OCLFEBinaryResult
    public:
        OCLFEBinaryResult()
            : m_type(IR_TYPE_UNKNOWN) {}
        llvm::SmallVectorImpl<char> &getIRBufferRef() { return m_IRBuffer; }
        std::string &getLogRef() { return m_log; }
        void setLog(const std::string &log) { m_log = log; }
        void setIRName(const std::string &name) { m_IRName = name; }
        void setIRType(Intel::OpenCL::ClangFE::IR_TYPE type) { m_type = type; }
    private:
        llvm::SmallVector<char, 4096> m_IRBuffer;
        std::string m_log;
        std::string m_IRName;
        Intel::OpenCL::ClangFE::IR_TYPE m_type;
    };
}}}
