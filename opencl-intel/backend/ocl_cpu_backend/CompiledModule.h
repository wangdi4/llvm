// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"

#include <memory>

namespace llvm {
  class Function;
  class MemoryBuffer;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/// Container for a Module and an ExecutionEngine
class CompiledModule {
public:
  CompiledModule(llvm::Module *M, std::unique_ptr<llvm::ExecutionEngine> EE)
    : m_M(M)
    , m_EE(std::move(EE))
  { }

  CompiledModule(llvm::Module *M, std::unique_ptr<llvm::orc::LLJIT> LLJIT)
    : m_M(M)
    , m_LLJIT(std::move(LLJIT))
  { }

  ~CompiledModule() {
    if (m_LLJIT)
      delete m_M;
  }

  /// Looks up an llvm::Function by name in the Module.
  llvm::Function* getFunction(llvm::StringRef name) {
      return m_M->getFunction(name);
  }

  /// Looks up a function pointer (to compiled code) in the ExecutionEngine
  void* getPointerToFunction(llvm::Function* func) {
    llvm::StringRef name = func->getName();
    void *addr;
    if (m_LLJIT) {
      auto sym = m_LLJIT->lookup(name);
      if (llvm::Error err = sym.takeError()) {
        llvm::logAllUnhandledErrors(std::move(err), llvm::errs());
        addr = nullptr;
      } else
        addr = reinterpret_cast<void *>(static_cast<uintptr_t>(
            sym->getAddress()));
    } else
      addr = reinterpret_cast<void *>(static_cast<uintptr_t>(
          m_EE->getFunctionAddress(name.str())));
    return addr;
  }

private:
  llvm::Module *m_M;

  // Freed automatically at destruction
  std::unique_ptr<llvm::ExecutionEngine> m_EE;

  std::unique_ptr<llvm::orc::LLJIT> m_LLJIT;
};

}}}
