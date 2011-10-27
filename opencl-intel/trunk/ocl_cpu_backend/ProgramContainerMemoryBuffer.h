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

File Name:  ProgramContainerMemoryBuffer.h

\*****************************************************************************/
#pragma once

#include <memory>
#include "llvm/Support/MemoryBuffer.h"
#include "cl_dev_backend_api.h"
#include "cl_types.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/**
 * Represents the memory buffer for holding the LLVM BitCode
 * 
 * Extends the LLVM Memory buffer to store the pointers to
 * program container and program headers. The memory buffer
 * pointers (BeginBuffer, EndBuffer) will point to the IR buffer
 * (past the headers).
 */
class ProgramContainerMemoryBuffer: public llvm::MemoryBuffer
{
protected:
    ProgramContainerMemoryBuffer( const cl_prog_container_header* pByteCodeContainer );

public:
    const cl_prog_container_header* GetContainerHeader() const
    { 
        return m_pContainerHeader; 
    }

    const cl_llvm_prog_header* GetProgHeader() const 
    {
        return m_pProgHeader;
    }

    size_t GetProgramSize() const
    {
        return m_pContainerHeader->container_size + sizeof(cl_prog_container_header);
    }

    const char *getBufferIdentifier() const 
    {
        return "Program";
    }
    
    static ProgramContainerMemoryBuffer* Create( const cl_prog_container_header* pHeader );

private:
    const cl_prog_container_header* m_pContainerHeader;
    const cl_llvm_prog_header* m_pProgHeader;
};

}}} // namespace
