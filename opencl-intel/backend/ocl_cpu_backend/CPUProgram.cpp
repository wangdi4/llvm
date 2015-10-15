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

File Name:  CPUProgram.cpp

\*****************************************************************************/
#define NOMINMAX

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "CPUProgram.h"
#include "BitCodeContainer.h"
#include "Kernel.h"
#include "ObjectCodeCache.h"


namespace Intel { namespace OpenCL { namespace DeviceBackend {

void CPUProgram::ReleaseExecutionEngine()
{
    // We have to remove the built-ins module from execEngine
    // since this module is owned by compiler.
    if (m_pExecutionEngine)
    {
        for (llvm::SmallVector<llvm::Module*, 2>::iterator it = m_bltnFuncList.begin(); it != m_bltnFuncList.end(); ++it)
        {
            m_pExecutionEngine->removeModule(*it);
        }

        if (m_pIRCodeContainer->GetModule())
        {
            m_pExecutionEngine->removeModule(static_cast<llvm::Module*>(m_pIRCodeContainer->GetModule()));
        }

        delete m_pExecutionEngine;
        m_pExecutionEngine = NULL;
    }
}

CPUProgram::~CPUProgram()
{
    // Freeing the execution engine is sufficient to cleanup all memory in
    // MCJIT
    ReleaseExecutionEngine();
}

void* CPUProgram::GetPointerToFunction(llvm::Function* F) {
    return reinterpret_cast<void*>(m_pExecutionEngine->getFunctionAddress(F->getName().str()));
}

void CPUProgram::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    void* pModule = (NULL != m_pIRCodeContainer) ? m_pIRCodeContainer->GetModule() : NULL;
    stats->SetPointerMark("pModule", pModule);
    stats->SetPointerMark("pProgram", this);
    Program::Deserialize(ist, stats);
}

void CPUProgram::SetObjectCache(ObjectCodeCache *oc) {
  m_ObjectCodeCache.reset(oc);
}

}}} // namespace
