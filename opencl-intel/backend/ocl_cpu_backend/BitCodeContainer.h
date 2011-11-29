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

File Name:  BitCodeContainer.h

\*****************************************************************************/
#pragma once

#include "cl_dev_backend_api.h"
#include "cl_types.h"

namespace llvm
{
    class Module;
}


namespace Intel { namespace OpenCL { namespace DeviceBackend {

class ProgramContainerMemoryBuffer;

/**
 * Represents the container for LLVM serialized bitcode
 *
 * Also get ownership on LLVM materialized module 
 */
class BitCodeContainer : public ICLDevBackendCodeContainer
{
public:
    BitCodeContainer(const cl_prog_container_header* pByteCodeContainer);
    ~BitCodeContainer();

    const cl_prog_container_header* GetCode() const;

    size_t GetCodeSize() const;

    /**
     * Get ownership on passed module
     */
    void   SetModule( void* pModule);
    
    /**
     * Retun the LLVM Module as a plain pointer
     */
    void*  GetModule() const;

    /**
     * Returns the pointer to the serialized bitcode buffer
     */
    const cl_llvm_prog_header* GetProgramHeader() const;

    /**
     * Retunrs the serialized bitcode buffer as a plain pointer (convert to LLVM MemoryBuffer)
     */
    void* GetMemoryBuffer() const;

    /**
     * Releases the Bit Code Container
     */
    void Release();

private:
    void*  m_pModule;
    ProgramContainerMemoryBuffer* m_pBuffer;

    // Klockwork Issue
    BitCodeContainer ( const BitCodeContainer& x );

    // Klockwork Issue
    BitCodeContainer& operator= ( const BitCodeContainer& x );

};

}}} // namespace
