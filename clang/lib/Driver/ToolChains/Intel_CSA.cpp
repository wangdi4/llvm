#if INTEL_FEATURE_CSA//===- Intel_CSA.cpp - CSA ToolChain Implementations -===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "Intel_CSA.h"
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

void CSAToolChain::addClangTargetOptions(const llvm::opt::ArgList &DriverArgs,
    ArgStringList &CC1Args, Action::OffloadKind DeviceOffloadKind) const {

  // Linux default options to pass to cc1
  Linux::addClangTargetOptions(DriverArgs, CC1Args, DeviceOffloadKind);

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

  // Disable builtin knowledge of memset and memcpy. The purpose of this is to
  // disable LoopIdiomRecognize for CSA, temporarily, until we do intelligent
  // things with the memset/memcpy intrinsics.
  CC1Args.push_back("-fno-builtin-memset");
  CC1Args.push_back("-fno-builtin-memcpy");

  // Since we're using the CSAToolChain, we know the target is CSA.
  // Check for the -fopenmp-targets=... list to see if we're compiling an
  // offloaded statement.
  Arg *targets = DriverArgs.getLastArg(options::OPT_fopenmp_targets_EQ);
  if (nullptr == targets) {
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

// This is currently only used when doing offload from x86. This link phase is
// preparing a blob for the upcoming x86 link, which includes the blob as
// specified by the Clang-generated offload linker script.
//
// Essentially, this just calls "ld [-static|-shared] -o OUTPUT INPUTS...",
// where INPUTS are x86 object files containing the IR to be offloaded to CSA.
// Note that the tools::gnutools::Linker nearly does what we want, except it
// also wants to link in startup files and possibly other things that we don't
// need.
void CSA::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                 const InputInfo &Output,
                                 const InputInfoList &Inputs,
                                 const ArgList &Args,
                                 const char *LinkingOutput) const {
  const auto &TC =
      static_cast<const toolchains::CSAToolChain &>(getToolChain());
  assert(TC.getTriple().getArch() == llvm::Triple::csa && "Wrong platform");

  ArgStringList CmdArgs;
  if (Args.hasArg(options::OPT_static))
    CmdArgs.push_back("-static");
  else if (Args.hasArg(options::OPT_shared))
    CmdArgs.push_back("-shared");

  CmdArgs.push_back(Args.MakeArgString("-o"));
  CmdArgs.push_back(Args.MakeArgString(Output.getFilename()));

  // This is a simple version of AddLinkerInputs. It simply includes appends
  // all file inputs.
  for (const auto &II : Inputs)
    if (II.isFilename())
      CmdArgs.push_back(II.getFilename());

  const char *Exec = Args.MakeArgString(TC.GetLinkerPath());
  C.addCommand(std::make_unique<Command>(JA, *this, Exec, CmdArgs, Inputs));
}

Tool *CSAToolChain::buildLinker() const {
  return new tools::CSA::Linker(*this);
}
#endif  // INTEL_FEATURE_CSA
