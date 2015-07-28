//===-- LPUConvertControlPass.cpp - LPU control flow conversion -----------===//
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
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;

#define DEBUG_TYPE "lpu-convert-control"

namespace {
class LPUConvertControlPass : public MachineFunctionPass {
public:
  static char ID;
  LPUConvertControlPass() : MachineFunctionPass(ID) {}

  const char* getPassName() const override {
    return "LPU Convert Control Flow";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
}

MachineFunctionPass *llvm::createLPUConvertControlPass() {
  return new LPUConvertControlPass();
}

char LPUConvertControlPass::ID = 0;

bool LPUConvertControlPass::runOnMachineFunction(MachineFunction &MF) {
  const TargetMachine &TM = MF.getTarget();
  //  MachineRegisterInfo *MRI = &MF.getRegInfo();
// const TargetFrameLowering &TFI = *TM.getSubtargetImpl()->getFrameLowering();
//  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  bool Modified = false;

  for (MachineFunction::iterator BB = MF.begin(), E = MF.end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      //MachineInstr *MI = I;
      // TBS
    }
  }

  return Modified;
}
