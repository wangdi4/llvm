/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  MICCompileService.h

\*****************************************************************************/
#pragma once

#include "CompileService.h"
#include "IMICCompilerConfig.h"
#include "MICProgramBuilder.h"
#include "ProgramBuilder.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class MICCompileService: public CompileService
{
public:
    MICCompileService(const IMICCompilerConfig& config);
    virtual ~MICCompileService() {}

    const ProgramBuilder* GetProgramBuilder() const { return &m_programBuilder; }
    ProgramBuilder* GetProgramBuilder() { return &m_programBuilder; }

    cl_dev_err_code MICCompileService::CheckProgramBinary(const void* pBinary, size_t uiBinarySize) const { return CL_DEV_SUCCESS; }
private:
    MICProgramBuilder m_programBuilder;
};

}}}
