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
        m_pExecutionEngine = nullptr;
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
    void* pModule = (nullptr != m_pIRCodeContainer) ? m_pIRCodeContainer->GetModule() : nullptr;
    stats->SetPointerMark("pModule", pModule);
    stats->SetPointerMark("pProgram", this);
    Program::Deserialize(ist, stats);
}

void CPUProgram::SetObjectCache(ObjectCodeCache *oc) {
  m_ObjectCodeCache.reset(oc);
}

}}} // namespace
