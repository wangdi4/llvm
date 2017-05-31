//===--- CSA.cpp - CSA ToolChain Implementations ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/ArgList.h"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang::driver::toolchains;
using namespace clang;
using namespace llvm::opt;

void
CSAToolChain::addClangTargetOptions(const llvm::opt::ArgList &DriverArgs,
                                    llvm::opt::ArgStringList &CC1Args) const {
  // Linux default options to pass to cc1
  Linux::addClangTargetOptions(DriverArgs, CC1Args);

  // Examine the effective (last/rightmost) optimization option given.
  if (Arg *A = DriverArgs.getLastArg(options::OPT_O_Group)) {
    // If -O0 is specified, disable dataflow conversion and memop ordering.
    if (A->getOption().matches(options::OPT_O0)) {
      CC1Args.push_back("-mllvm");
      CC1Args.push_back("-csa-cvt-cf-df-pass=0");
      CC1Args.push_back("-mllvm");
      CC1Args.push_back("-csa-order-memops=0");
    }
  } else {
    // If no -O option was given, then the usual default is like -O0. Instead,
    // the CSA toolchain enables optimization by default. This is unlike other
    // Targets.
    CC1Args.push_back("-O");
  }

  // Since we're using the CSAToolChain, we know the target is CSA.
  // Check for the -fopenmp-targets=... list to see if we're compiling an
  // offloaded statement.
  Arg *targets = DriverArgs.getLastArg(options::OPT_fopenmp_targets_EQ);
  if (nullptr == targets) {

    // Include some inlined libm implementations. (This may be temporary until a
    // real math library is in place.) Note that we only want to incur this
    // header once, so it doesn't get run again for the -fopenmp-targets=csa
    // invocation of cc1.
    CC1Args.push_back("-include");
    CC1Args.push_back("__clang_csa_math.h");

    return;
  }

  // Check the list of arguments for "csa"
  for (unsigned i = 0; i < targets->getNumValues(); i++) {

    // If we found an offload for the CSA. Add -mllvm -csa-wrap-asm to the
    // options so any CSA assembly code will be "wrapped" as strings and
    // won't cause errors when the x86 assembler sees them.
    const char *tgt = targets->getValue(i);
    if (0 == strcmp(tgt, "csa")) {
      CC1Args.push_back("-mllvm");
      CC1Args.push_back("-csa-wrap-asm");
      return;
    }
  }
}

