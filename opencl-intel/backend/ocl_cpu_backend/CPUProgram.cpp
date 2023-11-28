// Copyright 2010 Intel Corporation.
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
#ifndef _WIN32
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

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

void CPUProgram::ReleaseExecutionEngine() {
  // We have to remove the built-ins module from execEngine
  // since this module is owned by compiler.
  if (m_pExecutionEngine) {
    for (SmallVector<Module *, 2>::iterator it = m_bltnFuncList.begin();
         it != m_bltnFuncList.end(); ++it) {
      m_pExecutionEngine->removeModule(*it);
    }

    if (m_pIRCodeContainer->GetModule()) {
      m_pExecutionEngine->removeModule(m_pIRCodeContainer->GetModule());
    }
  }
}

CPUProgram::~CPUProgram() {
  if (m_codeProfilingStatus == PROFILING_GCOV)
    llvmWriteoutFilesPtr();

  // Freeing the execution engine is sufficient to cleanup all memory in
  // MCJIT
  ReleaseExecutionEngine();
}

uintptr_t CPUProgram::LLJITLookUp(StringRef Name) const {
  uintptr_t Addr = 0;
  try {
    auto Sym = m_LLJIT->lookup(Name);
    if (Error Err = Sym.takeError()) {
      logAllUnhandledErrors(std::move(Err), LLJITLogStream);
      throw Exceptions::CompilerException("Failed to lookup symbol " +
                                          Name.str() + '\n' + getLLJITLog());
    }
    Addr = static_cast<uintptr_t>(Sym->getValue());
  } catch (std::bad_array_new_length &e) {
    throw Exceptions::CompilerException(
        "Failed to lookup symbol " + Name.str() + " because of out of memory");
  }
  return Addr;
}

void *CPUProgram::GetPointerToGlobalValue(StringRef Name) const {
  assert((m_pExecutionEngine || m_LLJIT) && "Invalid JIT");
  uintptr_t Addr;
  if (m_LLJIT) {
    Addr = LLJITLookUp(Name);
  } else
    Addr = static_cast<uintptr_t>(
        m_pExecutionEngine->getGlobalValueAddress(Name.str()));
  return reinterpret_cast<void *>(Addr);
}

void *CPUProgram::GetPointerToFunction(StringRef Name) const {
  assert((m_pExecutionEngine || m_LLJIT) && "Invalid JIT");
  uintptr_t Addr;
  if (m_LLJIT) {
    Addr = LLJITLookUp(Name);
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
      Error err = m_LLJIT->initialize(m_LLJIT->getMainJITDylib());
      if (err) {
        logAllUnhandledErrors(std::move(err), errs());
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

#ifndef _WIN32
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

void CPUProgram::LoadProfileLib() {
  assert(m_LLJIT && "profiling only supports LLJIT now");
  std::string ClangRuntimePath = Intel::OpenCL::Utils::GetClangRuntimePath();
  SmallString<128> ProfileLibPath(ClangRuntimePath);
  sys::path::append(ProfileLibPath, PROFILE_LIB_NAME);

  std::string Env;
  if (Intel::OpenCL::Utils::getEnvVar(Env,
                                      "CL_CONFIG_FORCE_PROFILE_LIB_PATH") &&
      !Env.empty())
    ProfileLibPath = Env;

  if (!sys::fs::exists(ProfileLibPath)) {
    logAllUnhandledErrors(
        createStringError(errc::no_such_file_or_directory,
                          "The program was built with profiling but the "
                          "clang profile library is not found"),
        errs());
    throw Exceptions::DeviceBackendExceptionBase(
        "Clang profile library is not found");
  }
  auto &JD = m_LLJIT->getMainJITDylib();
  unique_function<Expected<orc::MaterializationUnit::Interface>(
      orc::ExecutionSession & ES, MemoryBufferRef ObjBuffer)>
      GetObjFileInterface = orc::getObjectFileInterface;
  auto G = orc::StaticLibraryDefinitionGenerator::Load(
      m_LLJIT->getObjLinkingLayer(), ProfileLibPath.c_str(),
      std::move(GetObjFileInterface));
  if (!G) {
    logAllUnhandledErrors(std::move(G.takeError()), errs());
    throw Exceptions::CompilerException("Failed to load clang profile library");
  }
  JD.addGenerator(std::move(*G));
  // Get code coverage data write out functions
  llvmWriteoutFilesPtr =
      reinterpret_cast<funcPtr>(GetPointerToFunction("llvm_writeout_files"));
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
  assert(!GetRuntimeService().isNull() && "RuntimeService in Program is NULL");
  // set in RuntimeService new BlockToKernelMapper object
  GetRuntimeService()->SetBlockToKernelMapper(pMapper);
}
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
