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
