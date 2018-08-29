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

#include <memory>

namespace llvm {
  class Function;
  class MemoryBuffer;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/// Container for a Module and an ExecutionEngine
class CompiledModule {
public:
  CompiledModule(llvm::Module* M, llvm::ExecutionEngine* EE)
    : m_Mod(M)
    , m_EE(EE)
  { }

  /// Looks up an llvm::Function by name in the Module.
  llvm::Function* getFunction(llvm::StringRef name) {
      assert(m_Mod != 0 && "Module must be initialized");
      return m_Mod->getFunction(name);
  }

  /// Looks up a function pointer (to compiled code) in the ExecutionEngine
  void* getPointerToFunction(llvm::Function* func) {
    return reinterpret_cast<void*>(m_EE->getFunctionAddress(func->getName().str()));
  }

private:
  // Pointer to Module owned by ExecutionEngine
  llvm::Module* m_Mod;

  // Freed automatically at destruction
  std::unique_ptr<llvm::ExecutionEngine> m_EE;
};

}}}
