//===-- CSAStatistics.cpp - CSA Statistics --------------------------------===//
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
#include "CSATargetMachine.h"
#include "InstPrinter/CSAInstPrinter.h"
#include "MachineCDG.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SparseSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
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

#define DEBUG_TYPE "csa-statistics"

static cl::opt<int> CSAStatisticsPass("csa-statistics", cl::Hidden,
                                      cl::desc("CSA Specific: Statistics"),
                                      cl::init(1));

namespace {
class CSAStatistics : public MachineFunctionPass {
public:
  static char ID;
  CSAStatistics();

  StringRef getPassName() const override { return "CSA: Statistics"; }

  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfo>();
    AU.addRequired<ControlDependenceGraph>();
    // AU.addRequired<LiveVariables>();
    AU.addRequired<MachineDominatorTree>();
    AU.addRequired<MachinePostDominatorTree>();
    AU.addRequired<AAResultsWrapperPass>();
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
  void CollectStatsForLoop(MachineLoop *L, raw_fd_ostream &O);

private:
  MachineFunction *thisMF;
  MachineLoopInfo *MLI;
  const CSAInstrInfo *TII;
};
} // namespace

namespace llvm {
void initializeCSAStatisticsPass(PassRegistry &);
}

//  Because of the namespace-related syntax limitations of gcc, we need
//  To hoist init out of namespace blocks.
char CSAStatistics::ID = 0;
INITIALIZE_PASS(CSAStatistics, "csa-statistics", "CSA Statistics", true, true)

CSAStatistics::CSAStatistics() : MachineFunctionPass(ID) {
  initializeCSAStatisticsPass(*PassRegistry::getPassRegistry());
}

MachineFunctionPass *llvm::createCSAStatisticsPass() {
  return new CSAStatistics();
}

bool CSAStatistics::runOnMachineFunction(MachineFunction &MF) {
  if (CSAStatisticsPass == 0)
    return false;
  thisMF = &MF;
  TII =
    static_cast<const CSAInstrInfo *>(thisMF->getSubtarget().getInstrInfo());
  MLI = &getAnalysis<MachineLoopInfo>();

  bool Modified        = false;
  std::string Filename = MF.getName().str() + "_stats" + ".txt";
  std::error_code EC;
  StringRef srcFilename = MF.getFunction().getParent()->getSourceFileName();
  LLVM_DEBUG(errs() << "Writing '" << Filename << "'...");
  raw_fd_ostream O(Filename, EC, sys::fs::OF_Text);
  O << "CSA Statistics for function: " << MF.getName().str() << " in "
    << srcFilename << "\n";

  for (MachineLoopInfo::iterator LI = MLI->begin(), LE = MLI->end(); LI != LE;
       ++LI) {
    CollectStatsForLoop(*LI, O);
  }
  O << "Top level: "
    << "\n";
  unsigned numAdd    = 0;
  unsigned numSub    = 0;
  unsigned numMul    = 0;
  unsigned numDiv    = 0;
  unsigned numFMA    = 0;
  unsigned numPick   = 0;
  unsigned numSwitch = 0;
  unsigned numLoad   = 0;
  unsigned numStore  = 0;
  unsigned numCmp    = 0;
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end();
       BB != E; ++BB) {
    if (MLI->getLoopFor(&*BB))
      continue;
    for (MachineBasicBlock::iterator II = BB->begin(), E = BB->end(); II != E;
         ++II) {
      MachineInstr *MI = &*II;
      if (TII->isLoad(MI))
        numLoad++;
      else if (TII->isStore(MI))
        numStore++;
      else if (TII->isPick(MI) || TII->isPickany(MI))
        numPick++;
      else if (TII->isSwitch(MI))
        numSwitch++;
      else if (TII->isFMA(MI))
        numFMA++;
      else if (TII->isDiv(MI))
        numDiv++;
      else if (TII->isMul(MI))
        numMul++;
      else if (TII->isCmp(MI))
        numCmp++;
      else if (TII->isAdd(MI))
        numAdd++;
      else if (TII->isSub(MI))
        numSub++;
    }
  }
  if (numStore)
    O << "numStore: " << numStore << "\n";
  if (numLoad)
    O << "numLoad: " << numLoad << "\n";
  if (numPick)
    O << "numPick: " << numPick << "\n";
  if (numSwitch)
    O << "numSwitch: " << numSwitch << "\n";
  if (numFMA)
    O << "numFMA: " << numFMA << "\n";
  if (numDiv)
    O << "numDiv: " << numDiv << "\n";
  if (numMul)
    O << "numMul: " << numMul << "\n";
  if (numCmp)
    O << "numCmp: " << numCmp << "\n";
  if (numAdd)
    O << "numAdd: " << numAdd << "\n";
  if (numSub)
    O << "numSub: " << numSub << "\n";
  O.close();
  return Modified;
}

void CSAStatistics::CollectStatsForLoop(MachineLoop *L, raw_fd_ostream &O) {
  for (MachineLoop::iterator LI = L->begin(), LE = L->end(); LI != LE; ++LI) {
    CollectStatsForLoop(*LI, O);
  }
  unsigned numAdd    = 0;
  unsigned numSub    = 0;
  unsigned numMul    = 0;
  unsigned numDiv    = 0;
  unsigned numFMA    = 0;
  unsigned numPick   = 0;
  unsigned numSwitch = 0;
  unsigned numLoad   = 0;
  unsigned numStore  = 0;
  unsigned numCmp    = 0;

  MachineLoop *mloop = L;
  unsigned lineno    = 0;
  for (MachineLoop::block_iterator BI = mloop->block_begin(),
                                   BE = mloop->block_end();
       BI != BE; ++BI) {
    MachineBasicBlock *mbb = *BI;
    // only conside blocks in the current loop level, blocks in the nested level
    // are done before.
    if (MLI->getLoopFor(mbb) != mloop)
      continue;
    for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
      MachineInstr *MI = &*I;
      if (MI->getDebugLoc().get()) {
        lineno = MI->getDebugLoc().getLine();
        break;
      }
    }
    if (lineno)
      break;
  }
  // current loop name + line number
  O << "loop header: " << mloop->getHeader()->getBasicBlock()->getName()
    << "; source line number: " << lineno << "\n";
  // parent loop name
  if (mloop->getParentLoop()) {
    O << "parent loop header: "
      << mloop->getParentLoop()->getHeader()->getBasicBlock()->getName()
      << "\n";
  }

  for (MachineLoop::block_iterator BI = mloop->block_begin(),
                                   BE = mloop->block_end();
       BI != BE; ++BI) {
    MachineBasicBlock *mbb = *BI;
    // only conside blocks in the current loop level, blocks in the nested level
    // are done before.
    if (MLI->getLoopFor(mbb) != mloop)
      continue;
    for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
      MachineInstr *MI = &*I;
      if (TII->isLoad(MI))
        numLoad++;
      else if (TII->isStore(MI))
        numStore++;
      else if (TII->isPick(MI) || TII->isPickany(MI))
        numPick++;
      else if (TII->isSwitch(MI))
        numSwitch++;
      else if (TII->isFMA(MI))
        numFMA++;
      else if (TII->isDiv(MI))
        numDiv++;
      else if (TII->isMul(MI))
        numMul++;
      else if (TII->isCmp(MI))
        numCmp++;
      else if (TII->isAdd(MI))
        numAdd++;
      else if (TII->isSub(MI))
        numSub++;
    }
  }
  if (numStore)
    O << "numStore: " << numStore << "\n";
  if (numLoad)
    O << "numLoad: " << numLoad << "\n";
  if (numPick)
    O << "numPick: " << numPick << "\n";
  if (numSwitch)
    O << "numSwitch: " << numSwitch << "\n";
  if (numFMA)
    O << "numFMA: " << numFMA << "\n";
  if (numDiv)
    O << "numDiv: " << numDiv << "\n";
  if (numMul)
    O << "numMul: " << numMul << "\n";
  if (numCmp)
    O << "numCmp: " << numCmp << "\n";
  if (numAdd)
    O << "numAdd: " << numAdd << "\n";
  if (numSub)
    O << "numSub: " << numSub << "\n";
}
