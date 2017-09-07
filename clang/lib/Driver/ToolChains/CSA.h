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
#if CSA_XMAIN
                             llvm::opt::ArgStringList &CC1Args,
                             Action::OffloadKind DeviceOffloadKind) const override;
#else
                             llvm::opt::ArgStringList &CC1Args
                             ) const override;
#endif
protected:
  Tool *buildLinker() const override;

};

} // end namespace toolchains
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_CSA_H
