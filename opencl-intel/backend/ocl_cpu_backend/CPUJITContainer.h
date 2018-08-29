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
#include "cl_device_api.h"
#include "Kernel.h"
#include "CPUCompiler.h"
#include "Serializer.h"

namespace llvm {
    class Module;
    class Function;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class CPUJITContainer: public IKernelJITContainer
{
public:
    CPUJITContainer();

    CPUJITContainer(const void* pFuncCode,
                 llvm::Function* pFunction,
                 llvm::Module* pModule,
                 KernelJITProperties* pProps);
    virtual ~CPUJITContainer();

    /*
     * ICLDevBackendJITContainer methods
     */
    virtual const void* GetJITCode() const { return m_pFuncCode; }
    virtual size_t GetJITCodeSize() const { return 0; } // TODO: Check this later
    virtual int GetLineNumber(void* pointer) const { return -1; }

    /*
     * IJITContainer methods
     */
    KernelJITProperties* GetProps() const  { return m_pProps; }

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    void Serialize(IOutputStream& ost, SerializationStatus* stats) const;
    void Deserialize(IInputStream& ist, SerializationStatus* stats);

private:
    const void*            m_pFuncCode;
    llvm::Function*        m_pFunction;
    llvm::Module*          m_pModule; // not owned by the class 

    KernelJITProperties*   m_pProps;

    // Klockwork Issue
    CPUJITContainer ( const CPUJITContainer& x );

    // Klockwork Issue
    CPUJITContainer& operator= ( const CPUJITContainer& x );
};
}}} // namespace

