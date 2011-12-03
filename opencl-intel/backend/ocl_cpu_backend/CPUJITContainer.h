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

File Name:  CPUJITContainer.h

\*****************************************************************************/
#pragma once

#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include "Kernel.h"
#include "CPUCompiler.h"

namespace llvm {
    class Module;
    class Function;
    class ExecutionEngine;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class CPUJITContainer: public IKernelJITContainer
{
public:
    CPUJITContainer(const void* pFuncCode,
                 llvm::Function* pFunction,
                 llvm::Module* pModule,
                 CPUCompiler* pCompiler,
                 KernelJITProperties* pProps);
    ~CPUJITContainer();

    /*
     * ICLDevBackendJITContainer methods
     */
    virtual const void* GetJITCode() const { return m_pFuncCode; }
    virtual size_t GetJITCodeSize() const { return 0; } // TODO: Check this later

    /*
     * IJITContainer methods
     */
    KernelJITProperties* GetProps() const  { return m_pProps; }

    /**
     * Serialization methods for the class (used by the serialization service)
     */
/*
    void Serialize(IOutputStream& ost, SerializationStatus* stats);
    void Deserialize(IInputStream& ist, SerializationStatus* stats);
*/
private:
    const void*            m_pFuncCode;
    llvm::Function*        m_pFunction;
    llvm::Module*          m_pModule; // not owned by the class 

    CPUCompiler*              m_pCompiler; //not owned by the class
    KernelJITProperties*   m_pProps;

    // Klockwork Issue
    CPUJITContainer ( const CPUJITContainer& x );

    // Klockwork Issue
    CPUJITContainer& operator= ( const CPUJITContainer& x );
};
}}} // namespace
