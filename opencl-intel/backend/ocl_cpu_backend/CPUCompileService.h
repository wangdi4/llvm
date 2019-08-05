// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#pragma once

#include "cl_dev_backend_api.h"

#include "CPUProgramBuilder.h"
#include "CompileService.h"
#include "ICompilerConfig.h"
#include "ProgramBuilder.h"

#include <string>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class CPUCompileService: public CompileService
{
public:
    CPUCompileService(const ICompilerConfig& config);
    virtual ~CPUCompileService() { }

    const ProgramBuilder* GetProgramBuilder() const { return &m_programBuilder; }
    ProgramBuilder* GetProgramBuilder() { return &m_programBuilder; }

    cl_dev_err_code DumpJITCodeContainer( const ICLDevBackendCodeContainer* pCodeContainer,
                                          const std::string& filePath) const;

    cl_dev_err_code CheckProgramBinary( const void* pBinary,
                                        size_t uiBinarySize );
private:
    CPUProgramBuilder m_programBuilder;
};

}}}
