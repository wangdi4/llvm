/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ClangCompatFixer.h

\*****************************************************************************/

#ifndef __CLANG_COMPAT_FIXER_H__
#define __CLANG_COMPAT_FIXER_H__

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/InstrTypes.h"


namespace Intel { namespace OpenCL { namespace DeviceBackend {

  using namespace llvm;

  /// @brief  ClangCompatFixer is responsible for fixing incompatibilities between (Apple's) 
  ///         clang output and the backend's expectations.
  ///         This way the backend only needs to support "one true way" of doing things,
  ///         and if the clang output is different, it is modified to look the way the
  ///         backend expects.
  ///         Currently, it contains two such fixes:
  ///         1) llvm.fmuladd intrinsics are broken into an fmul + fadd pair
  ///         2) The representation of implicit locals is changed to "marked globals"
  ///            instead of "marked allocas"
  class ClangCompatFixer : public ModulePass {

  public:
    /// Pass identification, replacement for typeid
    static char ID;
    
    // Constructor
    ClangCompatFixer() : ModulePass(ID) {}

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "ClangCompatFixer";
    }

    /// @brief    LLVM Module pass entry
    /// @param M  Module to transform
    /// @returns  true if changed
    virtual bool runOnModule(Module &M);

  private:
    /// @brief Breaks FMA intrinsics back into a mul+add
    /// @param F - Function to replace FMAs in
    /// @return true if the function was changed, false otherwise
    bool handleFMAIntrinsics(Function &F);
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __CLANG_COMPAT_FIXER_H__
