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
#include "BitCodeContainer.h"
#include "CompilationUtils.h"
#include "CPUProgram.h"
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
            m_pExecutionEngine->removeModule(m_pIRCodeContainer->GetModule());
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

void* CPUProgram::GetPointerToGlobalValue(llvm::StringRef Name) const {
    assert((m_pExecutionEngine || m_LLJIT) && "Invalid JIT");
    uintptr_t Addr;
    if (m_LLJIT) {
        auto Sym = m_LLJIT->lookup(Name);
        if (llvm::Error Err = Sym.takeError()) {
            llvm::logAllUnhandledErrors(std::move(Err), llvm::errs());
            throw Exceptions::CompilerException("Failed to lookup symbol " +
                                                Name.str());
        }
        Addr = static_cast<uintptr_t>(Sym->getAddress());
    } else
        Addr = static_cast<uintptr_t>(
            m_pExecutionEngine->getGlobalValueAddress(Name.str()));
    return reinterpret_cast<void *>(Addr);
}

void* CPUProgram::GetPointerToFunction(llvm::StringRef Name) const {
    assert((m_pExecutionEngine || m_LLJIT) && "Invalid JIT");
    uintptr_t Addr;
    if (m_LLJIT) {
        auto Sym = m_LLJIT->lookup(Name);
        if (llvm::Error Err = Sym.takeError()) {
            llvm::logAllUnhandledErrors(std::move(Err), llvm::errs());
            throw Exceptions::CompilerException("Failed to lookup symbol " +
                                                Name.str());
        }
        Addr = static_cast<uintptr_t>(Sym->getAddress());
    } else
        Addr = static_cast<uintptr_t>(
            m_pExecutionEngine->getFunctionAddress(Name.str()));
    return reinterpret_cast<void *>(Addr);
}

cl_ulong CPUProgram::GetFunctionPointerFor(const char *FunctionName) const {
    return (cl_ulong)(GetPointerToFunction(FunctionName));
}

void CPUProgram::GetGlobalVariablePointers(const cl_prog_gv **GVs,
                                           size_t *GVCount) const {
    const std::vector<cl_prog_gv> &GlobalVariables = GetGlobalVariables();
    *GVCount = GlobalVariables.size();
    if (*GVCount > 0)
        *GVs = &GlobalVariables[0];
}

void CPUProgram::Deserialize(IInputStream& ist, SerializationStatus* stats,
                             size_t maxPrivateMemSize)
{
    void* pModule = (nullptr != m_pIRCodeContainer) ? m_pIRCodeContainer->GetModule() : nullptr;
    stats->SetPointerMark("pModule", pModule);
    stats->SetPointerMark("pProgram", this);
    Program::Deserialize(ist, stats, maxPrivateMemSize);
}

void CPUProgram::SetObjectCache(ObjectCodeCache *oc) {
  m_ObjectCodeCache.reset(oc);
}

}}} // namespace
