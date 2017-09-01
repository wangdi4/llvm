//===------------------ Intel_XmainOptLevelPass.cpp -----------------------===//
//
// Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This is an immutable pass which stores the opt level.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_XmainOptLevelPass.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "xmain-opt-level-pass"

static cl::opt<unsigned>
    XmainOptLevel("xmain-opt-level", cl::init(-1), cl::Hidden,
                  cl::desc("Command line option to set opt level"));

char XmainOptLevelPass::ID = 0;
INITIALIZE_PASS(XmainOptLevelPass, "xmain-opt-level-pass",
                "Xmain opt level pass", false, true)

bool XmainOptLevelPass::doInitialization(Module &M) {
  // This setup is used to override the opt level using command line when using
  // 'opt' tool.
  OptLevel = (XmainOptLevel == unsigned(-1)) ? OptLevel : XmainOptLevel;
  return false;
}

ImmutablePass *llvm::createXmainOptLevelPass(unsigned OptLevel) {
  return new XmainOptLevelPass(OptLevel);
}
