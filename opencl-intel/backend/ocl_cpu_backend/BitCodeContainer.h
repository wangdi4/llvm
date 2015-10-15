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
#include "llvm/ADT/StringRef.h"

#include <memory>

// forward decl
namespace llvm { class MemoryBuffer; }

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
    BitCodeContainer(const void *pBinary, size_t uiBinarySize, const char* name = "");
    ~BitCodeContainer();

    const void* GetCode() const;

    size_t GetCodeSize() const;

    /**
     * Get ownership on passed module
     */
    void   SetModule( void* pModule);

    /**
     * Return the LLVM Module as a plain pointer
     */
    void*  GetModule() const;

    /**
     * Returns the serialized bitcode buffer as a plain pointer (convert to LLVM MemoryBuffer)
     */
    void* GetMemoryBuffer() const;

    /**
     * Releases the Bit Code Container
     */
    void Release();

private:
    void*  m_pModule;
    std::unique_ptr<llvm::MemoryBuffer> m_pBuffer;

    // Klockwork Issue
    BitCodeContainer ( const BitCodeContainer& x );

    // Klockwork Issue
    BitCodeContainer& operator= ( const BitCodeContainer& x );
};
}}} // namespace
