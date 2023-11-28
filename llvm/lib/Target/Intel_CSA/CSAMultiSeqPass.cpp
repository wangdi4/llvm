//===-- CSAStatistics.cpp - CSA Statistics --------===//
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
// This file "reexpresses" the code containing traditional control flow
// into a basically data flow representation suitable for the CSA.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSAInstrInfo.h"
#include "CSASeqOpt.h"
#include "CSATargetMachine.h"
#include "InstPrinter/CSAInstPrinter.h"
#include "MachineCDG.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SparseSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineOptimizationRemarkEmitter.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineSSAUpdater.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include <map>

using namespace llvm;

#define DEBUG_TYPE "csa-multi-sequence"
#define PASS_NAME "CSA: Multiple Sequence"

static cl::opt<int> CSAMultiSeqPass("csa-multi-seq", cl::Hidden,
                                    cl::desc("CSA Specific: Multiple Sequence"),
                                    cl::init(1));

namespace {
class CSAMultiSeq : public MachineFunctionPass {
public:
  static char ID;
  CSAMultiSeq();

  StringRef getPassName() const override { return PASS_NAME; }

  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineOptimizationRemarkEmitterPass>();
    AU.addRequired<CSALoopInfoPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  MachineFunction *thisMF;
  MachineLoopInfo *MLI;
  const CSAInstrInfo *TII;
};
} // namespace

namespace llvm {
void initializeCSAMultiSeqPass(PassRegistry &);
}

//  Because of the namespace-related syntax limitations of gcc, we need
//  To hoist init out of namespace blocks.
char CSAMultiSeq::ID = 0;
INITIALIZE_PASS_BEGIN(CSAMultiSeq, DEBUG_TYPE, PASS_NAME, false, false)
INITIALIZE_PASS_DEPENDENCY(MachineOptimizationRemarkEmitterPass)
INITIALIZE_PASS_DEPENDENCY(CSALoopInfoPass)
INITIALIZE_PASS_END(CSAMultiSeq, DEBUG_TYPE, PASS_NAME, false, false)

CSAMultiSeq::CSAMultiSeq() : MachineFunctionPass(ID) {
  initializeCSAMultiSeqPass(*PassRegistry::getPassRegistry());
}

MachineFunctionPass *llvm::createCSAMultiSeqPass() { return new CSAMultiSeq(); }

bool CSAMultiSeq::runOnMachineFunction(MachineFunction &MF) {
  if (!shouldRunDataflowPass(MF))
    return false;

  if (CSAMultiSeqPass == 0)
    return false;
  auto &ORE = getAnalysis<MachineOptimizationRemarkEmitterPass>().getORE();
  auto &LI = getAnalysis<CSALoopInfoPass>();
  CSASeqOpt seqOpt(&MF, ORE, LI, DEBUG_TYPE);
  seqOpt.SequenceOPT(true);
  return true;
}
