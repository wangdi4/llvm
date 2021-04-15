//===- BuiltinImport.h - Import DPC++ builtin modules ---------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BUILTIN_IMPORT_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_BUILTIN_IMPORT_H

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Import builtin function from builtin modules.
class BuiltinImportPass : public PassInfoMixin<BuiltinImportPass> {
public:
  BuiltinImportPass(const SmallVector<Module *, 2> &BuiltinModules =
                        SmallVector<Module *, 2>(),
                    StringRef CPUPrefix = "");

  static StringRef name() { return "BuiltinImportPass"; }

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  /// Glue for old PM.
  bool runImpl(Module &M);

private:
  using FuncVec = SmallVector<Function *, 8>;

  /// Update svml function names from shared libraries to reflect cpu prefix and
  /// their calling conventions.
  /// \param SvmlFunctions Shared svml functions used by the module.
  /// \param M The module to process.
  void UpdateSvmlBuiltin(const FuncVec &SvmlFunctions, Module &M) const;

  /// Get all the functions called by given function.
  /// \param Func The given function.
  /// \param CalledFuncs The list of all functions called by Func.
  /// \param SvmlFunctions List of shared svml functions called by Func.
  void GetCalledFunctions(const Function *Func, FuncVec &CalledFuncs,
                          FuncVec &SvmlFunctions) const;

  /// Find all functions and global variables from the \p Modules used by the
  /// function \p Root. Recursively looks in the functions called by the \p
  /// Root, gathering a complete list of \p Root dependencies.
  /// \param Root top level function for lookup.
  /// \param Modules modules with definitions of \p Root dependencies.
  /// \param UsedFunctions functions used by the \p Root.
  /// \param UsedGlobals global variables used by the \p Root.
  /// \param SvmlFunctions Shared svml functions used by the \p Root.
  void ExploreUses(Function *Root, SmallVectorImpl<Module *> &Modules,
                   SetVector<GlobalValue *> &UsedFunctions,
                   SetVector<GlobalVariable *> &UsedGlobals,
                   FuncVec &SvmlFunctions);

private:
  /// Source module list - contains the source functions to import.
  SmallVector<Module *, 2> BuiltinModules;

  /// Holds cpu perfix that would replace 'shared' substr in svml funcs.
  StringRef CPUPrefix;
};

} // namespace llvm

#endif
