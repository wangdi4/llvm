/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  CompiledModule.h

\*****************************************************************************/
#pragma once

#include "llvm/ADT/OwningPtr.h"
#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"

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
      return m_EE->getPointerToFunction(func);
  }

private:
  // Pointer to Module owned by ExecutionEngine
  llvm::Module* m_Mod;

  // Freed automatically at destruction
  llvm::OwningPtr<llvm::ExecutionEngine> m_EE;
};

}}}
