//===-- LPUCvtCFDFPass.cpp - LPU convert control flow to data flow --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file "reexpresses" the code containing traditional control flow
// into a basically data flow representation suitable for the LPU.
//
//===----------------------------------------------------------------------===//

#include <map>
#include "LPU.h"
#include "InstPrinter/LPUInstPrinter.h"
#include "LPUInstrInfo.h"
#include "LPUTargetMachine.h"
#include "LPULicAllocation.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachinePostDominators.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "MachineCDG.h"

using namespace llvm;

static cl::opt<int>
CvtCFDFPass("lpu-cvt-cf-df-pass", cl::Hidden,
               cl::desc("LPU Specific: Convert control flow to data flow pass"),
               cl::init(0));

#define DEBUG_TYPE "lpu-cvt-ctl-df"

namespace {
class LPUCvtCFDFPass : public MachineFunctionPass {
public:
  static char ID;
  LPUCvtCFDFPass() : MachineFunctionPass(ID) { thisMF = nullptr;}

  const char* getPassName() const override {
    return "LPU Convert Control Flow to Data Flow";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfo>();
    AU.addRequired<ControlDependenceGraph>();
    //AU.addRequired<LiveVariables>();
    AU.addRequired<MachinePostDominatorTree>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  MachineFunction *thisMF;
  MachinePostDominatorTree *PDT;
};
}

MachineFunctionPass *llvm::createLPUCvtCFDFPass() {
  return new LPUCvtCFDFPass();
}

char LPUCvtCFDFPass::ID = 0;

bool LPUCvtCFDFPass::runOnMachineFunction(MachineFunction &MF) {

  if (CvtCFDFPass == 0) return false;

  thisMF = &MF;

  PDT = &getAnalysis<MachinePostDominatorTree>();

  bool Modified = false;

  // for now only well formed innermost loop regions are processed in this pass
  MachineLoopInfo *MLI = &getAnalysis<MachineLoopInfo>();
  ControlDependenceGraph *CDG = &getAnalysis<ControlDependenceGraph>();
  ControlDependenceNode *root = CDG->getRoot();

  if (!MLI) {
    DEBUG(errs() << "no loop info.\n");
    return false;
  }

  return Modified;

}
