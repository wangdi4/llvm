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
#ifndef WIN32
#include "cl_amx_syscall.h"
#endif
#include "BitCodeContainer.h"
#include "CPUBlockToKernelMapper.h"
#include "CPUProgram.h"
#include "Kernel.h"
#include "ObjectCodeCache.h"
#include "cl_sys_info.h"

#include "llvm/ExecutionEngine/Orc/ObjectFileInterface.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/Path.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

void CPUProgram::ReleaseExecutionEngine() {
  // We have to remove the built-ins module from execEngine
  // since this module is owned by compiler.
  if (m_pExecutionEngine) {
    for (llvm::SmallVector<llvm::Module *, 2>::iterator it =
             m_bltnFuncList.begin();
         it != m_bltnFuncList.end(); ++it) {
      m_pExecutionEngine->removeModule(*it);
    }

    if (m_pIRCodeContainer->GetModule()) {
      m_pExecutionEngine->removeModule(m_pIRCodeContainer->GetModule());
    }
  }
}

CPUProgram::~CPUProgram() {
  using llvm_writeout_files_ptr = void (*)(void);
  if (m_codeProfilingStatus == PROFILING_GCOV) {
    auto llvm_writeout_files = reinterpret_cast<llvm_writeout_files_ptr>(
        GetPointerToFunction("llvm_writeout_files"));
    llvm_writeout_files();
  }
  // Freeing the execution engine is sufficient to cleanup all memory in
  // MCJIT
  ReleaseExecutionEngine();
}

void *CPUProgram::GetPointerToGlobalValue(llvm::StringRef Name) const {
  assert((m_pExecutionEngine || m_LLJIT) && "Invalid JIT");
  uintptr_t Addr;
  if (m_LLJIT) {
    auto Sym = m_LLJIT->lookup(Name);
    if (llvm::Error Err = Sym.takeError()) {
      llvm::logAllUnhandledErrors(std::move(Err), LLJITLogStream);
      throw Exceptions::CompilerException("Failed to lookup symbol " +
                                          Name.str() + '\n' + getLLJITLog());
    }
    Addr = static_cast<uintptr_t>(Sym->getValue());
  } else
    Addr = static_cast<uintptr_t>(
        m_pExecutionEngine->getGlobalValueAddress(Name.str()));
  return reinterpret_cast<void *>(Addr);
}

void *CPUProgram::GetPointerToFunction(llvm::StringRef Name) const {
  assert((m_pExecutionEngine || m_LLJIT) && "Invalid JIT");
  uintptr_t Addr;
  if (m_LLJIT) {
    auto Sym = m_LLJIT->lookup(Name);
    if (llvm::Error Err = Sym.takeError()) {
      llvm::logAllUnhandledErrors(std::move(Err), LLJITLogStream);
      throw Exceptions::CompilerException("Failed to lookup symbol " +
                                          Name.str() + '\n' + getLLJITLog());
    }
    Addr = static_cast<uintptr_t>(Sym->getValue());
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
      for (unsigned j = 0; j < kernel->GetKernelJITCount(); ++j) {
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

#ifndef WIN32
  // Read the HasMatrixCall Property of each kernel and run amx syscall.
  static llvm::once_flag OnceFlag;
  for (size_t i = 0; i < m_kernels->GetCount(); ++i) {
    Kernel *kernel = m_kernels->GetKernel(i);
    if (kernel->GetKernelProporties()->HasMatrixCall()) {
      llvm::call_once(OnceFlag, [&] {
        // we can't emit exception for checking the status of
        // requestPermXtileData here. Otherwise, for sde users,
        // requestPermXtileData will fail and lead to exception.
        Intel::OpenCL::Utils::requestPermXtileData();
      });
      break;
    }
  }
#endif
  return CL_DEV_SUCCESS;
}

void CPUProgram::LoadProfileLib() const {
  assert(m_LLJIT && "profiling only supports LLJIT now");
  std::string ClangRuntimePath = Intel::OpenCL::Utils::GetClangRuntimePath();
  SmallString<128> ProfileLibPath(ClangRuntimePath);
  llvm::sys::path::append(ProfileLibPath, PROFILE_LIB_NAME);

  std::string Env;
  if (Intel::OpenCL::Utils::getEnvVar(Env,
                                      "CL_CONFIG_FORCE_PROFILE_LIB_PATH") &&
      !Env.empty())
    ProfileLibPath = Env;

  if (!llvm::sys::fs::exists(ProfileLibPath)) {
    llvm::logAllUnhandledErrors(
        llvm::createStringError(llvm::errc::no_such_file_or_directory,
                                "The program was built with profiling but the "
                                "clang profile library is not found"),
        llvm::errs());
    throw Exceptions::DeviceBackendExceptionBase(
        "Clang profile library is not found");
  }
  auto &JD = m_LLJIT->getMainJITDylib();
  unique_function<Expected<llvm::orc::MaterializationUnit::Interface>(
      llvm::orc::ExecutionSession & ES, MemoryBufferRef ObjBuffer)>
      GetObjFileInterface = llvm::orc::getObjectFileInterface;
  auto G = llvm::orc::StaticLibraryDefinitionGenerator::Load(
      m_LLJIT->getObjLinkingLayer(), ProfileLibPath.c_str(),
      m_LLJIT->getTargetTriple(), std::move(GetObjFileInterface));
  if (!G) {
    llvm::logAllUnhandledErrors(std::move(G.takeError()), llvm::errs());
    throw Exceptions::CompilerException("Failed to load clang profile library");
  }
  JD.addGenerator(std::move(*G));
}

void CPUProgram::Deserialize(IInputStream &ist, SerializationStatus *stats) {
  void *pModule = (nullptr != m_pIRCodeContainer)
                      ? m_pIRCodeContainer->GetModule()
                      : nullptr;
  stats->SetPointerMark("pModule", pModule);
  stats->SetPointerMark("pProgram", this);
  Program::Deserialize(ist, stats);
}

void CPUProgram::SetObjectCache(ObjectCodeCache *oc) {
  m_ObjectCodeCache.reset(oc);
}

void CPUProgram::CreateAndSetBlockToKernelMapper() {
  // create block to kernel mapper
  IBlockToKernelMapper *pMapper = new CPUBlockToKernelMapper((Program *)this);
  assert(pMapper && "IBlockToKernelMapper object is NULL");
  assert(!GetRuntimeService().isNull() && "RuntimeService in Program is NULL");
  // set in RuntimeService new BlockToKernelMapper object
  GetRuntimeService()->SetBlockToKernelMapper(pMapper);
}
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
