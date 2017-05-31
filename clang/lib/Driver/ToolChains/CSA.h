//===--- CSA.h - CSA ToolChain Implementations --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_CSA_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_CSA_H

#include "Linux.h"
#include "clang/Driver/ToolChain.h"

namespace clang {
namespace driver {
namespace toolchains {

// Tweaks to the toolchain for CSA
class LLVM_LIBRARY_VISIBILITY CSAToolChain : public Linux {
public:
  CSAToolChain(const Driver &D, const llvm::Triple &Triple,
               const llvm::opt::ArgList &Args) :
    Linux(D, Triple, Args) {}

  // CSA defaults to -fno-math-errno. This means that the math functions
  // will have the "readnone" attribute indicating that they do not change
  // any global state (like errno)
  bool IsMathErrnoDefault() const override { return false; }

  // If this is an OpenMP offload build, add -mllvm -csa-wrap-asm to the
  // options
  void addClangTargetOptions(const llvm::opt::ArgList &DriverArgs,
                             llvm::opt::ArgStringList &CC1Args) const override;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_CSA_H
