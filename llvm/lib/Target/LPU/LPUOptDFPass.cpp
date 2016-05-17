//===-- LPUOptDFPass.cpp - LPU optimization of data flow ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass optimizes post-dataflow code.  In particular, it does things
// like insert sequence operations when appropriate.
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
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;

static cl::opt<int>
OptDFPass("lpu-opt-df-pass", cl::Hidden,
               cl::desc("LPU Specific: Optimize data flow pass"),
               cl::init(0));

#define DEBUG_TYPE "lpu-opt-df"

namespace {
class LPUOptDFPass : public MachineFunctionPass {
public:
  static char ID;
  LPUOptDFPass() : MachineFunctionPass(ID) { thisMF = nullptr;}

  const char* getPassName() const override {
    return "LPU Convert Control Flow to Data Flow";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    //AU.addRequired<MachineLoopInfo>();
    //AU.addRequired<LiveVariables>();
    //AU.addRequired<MachineDominatorTree>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  MachineFunction *thisMF;
};
}

MachineFunctionPass *llvm::createLPUOptDFPass() {
  return new LPUOptDFPass();
}

char LPUOptDFPass::ID = 0;

bool LPUOptDFPass::runOnMachineFunction(MachineFunction &MF) {

  if (OptDFPass == 0) return false;

  thisMF = &MF;

  bool Modified = false;

  // Using SEQ in place of pick/add/cmp/switch pattern.
  //
  // Loop over the instructions in the routine, to match basically the pattern:
  //
  //  init1 cres, 0                    // initialized channel for compare res
  //  pick64 pres, cres*, base, sres   // base is value in, sres flows around
  //  copy pres*, pres
  // ... {note: uses of pres are OK - they are the values of the sequence}
  //  add64  ares, pres*, stride       // increment by stride
  // ...
  //  cmplt64 cres, ares, bound        // compare against bound
  //  copy1 cres*, cres                // multiple uses of cres ok
  // ...
  //  switch64 ign, sres, cres*, ares  // send value around
  // ...
  //
  // and replace it with
  //  seq64 sres1, cres*, base, bound, stride
  //
  // Note: This may not be quite right, but it gives the idea.  Note that it
  // could be more general
  
  return Modified;

}
