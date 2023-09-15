//===-- CSAOptDFPass.cpp - CSA optimization of data flow ------------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass optimizes post-dataflow code.  In particular, it does things
// like insert sequence operations when appropriate.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSASeqOpt.h"
#include "CSATargetMachine.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

static cl::opt<int> OptDFPass("csa-opt-df-pass", cl::Hidden,
                              cl::desc("CSA Specific: Optimize data flow pass"),
                              cl::init(1));

#define DEBUG_TYPE "csa-opt-df"
#define PASS_NAME "CSA: (Sequence) Optimizations to Data Flow"

const TargetRegisterClass *const SeqPredRC = &CSA::CI1RegClass;

namespace llvm {

class CSAOptDFPass : public MachineFunctionPass {
public:
  static char ID;
  CSAOptDFPass() : MachineFunctionPass(ID) {
    initializeCSAOptDFPassPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return PASS_NAME;
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineOptimizationRemarkEmitterPass>();
    AU.addRequired<CSALoopInfoPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};
} // namespace llvm

MachineFunctionPass *llvm::createCSAOptDFPass() { return new CSAOptDFPass(); }

char CSAOptDFPass::ID = 0;

INITIALIZE_PASS_BEGIN(CSAOptDFPass, DEBUG_TYPE, PASS_NAME, false, false)
INITIALIZE_PASS_DEPENDENCY(MachineOptimizationRemarkEmitterPass)
INITIALIZE_PASS_DEPENDENCY(CSALoopInfoPass)
INITIALIZE_PASS_END(CSAOptDFPass, DEBUG_TYPE, PASS_NAME, false, false)


bool CSAOptDFPass::runOnMachineFunction(MachineFunction &MF) {
  if (!shouldRunDataflowPass(MF))
    return false;

  if (OptDFPass == 0)
    return false;

  auto &ORE = getAnalysis<MachineOptimizationRemarkEmitterPass>().getORE();
  auto &LI = getAnalysis<CSALoopInfoPass>();
  CSASeqOpt seqOpt(&MF, ORE, LI, DEBUG_TYPE);
  seqOpt.SequenceOPT(false);
  return true;
}

