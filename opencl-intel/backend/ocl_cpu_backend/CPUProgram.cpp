// Copyright 2010-2021 Intel Corporation.
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

#include "CPUProgram.h"
#include "BitCodeContainer.h"
#include "CPUBlockToKernelMapper.h"
#include "CompilationUtils.h"
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
  *GVCount = m_globalVariables.size();
  if (*GVCount > 0)
    *GVs = &m_globalVariables[0];
}

cl_dev_err_code CPUProgram::Finalize() {
  assert(
      ((m_pExecutionEngine && !m_LLJIT) || (!m_pExecutionEngine && m_LLJIT)) &&
      "Only one of MCJIT and m_LLJIT should be enabled");
  using CtorTy = void (*)();
  if (m_pExecutionEngine) {
    m_pExecutionEngine->finalizeObject();
    m_pExecutionEngine->runStaticConstructorsDestructors(/*isDtors*/ false);
    // Setup kernel JIT address.
    for (size_t i = 0; i < m_kernels->GetCount(); ++i) {
      Kernel *kernel = m_kernels->GetKernel(i);
      for (unsigned j =0; j < kernel->GetKernelJITCount(); ++j) {
        IKernelJITContainer *jitContainer = kernel->GetKernelJIT(j);
        const std::string &kernelName = jitContainer->GetFunctionName();
        jitContainer->SetJITCode(GetPointerToFunction(kernelName));
      }
    }
    for (const std::string &name : m_globalCtors) {
      auto ctor = reinterpret_cast<CtorTy>(GetPointerToFunction(name));
      ctor();
    }
  } else {
    if (HasCachedExecutable()) {
      for (const std::string &name : m_globalCtors) {
        auto ctor = reinterpret_cast<CtorTy>(GetPointerToFunction(name));
        ctor();
      }
    } else {
      llvm::Error err = m_LLJIT->initialize(m_LLJIT->getMainJITDylib());
      if (err) {
        llvm::logAllUnhandledErrors(std::move(err), llvm::errs());
        return CL_DEV_JIT_FAIL;
      }
    }
  }

  // Get pointer of global variables
  for (auto &gv : m_globalVariables) {
    gv.pointer = GetPointerToGlobalValue(gv.name);
    assert(gv.pointer && "failed to get address of global variable");
  }

  // The BlockToKernelMapper must be created after finalizing object/loading
  // dll, since it needs to find symbol address of block invoke kernel. The
  // later is available after finalizing object/loading dll if native debugger
  // is enabled on Windows.
  CreateAndSetBlockToKernelMapper();
  return CL_DEV_SUCCESS;
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

void CPUProgram::CreateAndSetBlockToKernelMapper() {
  // create block to kernel mapper
  IBlockToKernelMapper *pMapper =
      new CPUBlockToKernelMapper((Program *)this);
  assert(pMapper && "IBlockToKernelMapper object is NULL");
  assert(!GetRuntimeService().isNull() && "RuntimeService in Program is NULL");
  // set in RuntimeService new BlockToKernelMapper object
  GetRuntimeService()->SetBlockToKernelMapper(pMapper);
}
}}} // namespace
