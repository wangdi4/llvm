//===--- CSA.h - CSA ToolChain Implementations --------------*- C++ -*-===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_CSA_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_CSA_H

#include "Linux.h"
#include "clang/Driver/ToolChain.h"

namespace clang {
namespace driver {
namespace tools {
namespace CSA {

class LLVM_LIBRARY_VISIBILITY Linker : public GnuTool {
 public:
   Linker(const ToolChain &TC)
       : GnuTool("csa::Linker", "csa-ld", TC) {}

   bool hasIntegratedCPP() const override { return false; }

   void ConstructJob(Compilation &C, const JobAction &JA,
                     const InputInfo &Output, const InputInfoList &Inputs,
                     const llvm::opt::ArgList &TCArgs,
                     const char *LinkingOutput) const override;
};
} // end namespace CSA
} // end namespace tools

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
                             llvm::opt::ArgStringList &CC1Args,
                             Action::OffloadKind DeviceOffloadKind) const override;
protected:
  Tool *buildLinker() const override;

};

} // end namespace toolchains
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_CSA_H
