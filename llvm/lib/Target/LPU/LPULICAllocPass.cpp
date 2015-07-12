//===-- LPULICAllocPass.cpp - LPU LIC allocation --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file "allocates" and converts virtual registers into LICs.
//
//===----------------------------------------------------------------------===//

#include <map>
#include "LPU.h"
#include "InstPrinter/LPUInstPrinter.h"
#include "LPUInstrInfo.h"
#include "LPUTargetMachine.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;

#define DEBUG_TYPE "lpu-lic-alloc"

namespace {
class LPULICAllocPass : public MachineFunctionPass {
public:
  static char ID;
  LPULICAllocPass() : MachineFunctionPass(ID) {}

  const char* getPassName() const override {
    return "LPU LIC Allocation";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
}

MachineFunctionPass *llvm::createLPULICAllocPass() {
  return new LPULICAllocPass();
}

char LPULICAllocPass::ID = 0;

bool LPULICAllocPass::runOnMachineFunction(MachineFunction &MF) {
  const TargetMachine &TM = MF.getTarget();
  //  const TargetFrameLowering &TFI = *TM.getSubtargetImpl()->getFrameLowering();
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  bool Modified = false;

  // Map from virtual register, to physical register number
  typedef std::map<int,int> RegMap;
  RegMap regmap;
  // Hack.  Start packing with c20.
  // c0/c1 - control input/output
  // c2..c17 - up to 16 input params
  // c18..c19 - up to 2 results
  int nextLICAlloc = LPU::C20;

  // For a 1st cut, simply allocate a LIC ("physical register") for each
  // virtual register as they are encountered - i.e. there is no real
  // "allocation".
  // If we change to using LICs as a separate operand type, this is where
  // they would be introduced...

  for (MachineFunction::iterator BB = MF.begin(), E = MF.end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      MachineInstr *MI = I;
      for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
        if (MI->getOperand(i).isReg()) {
          unsigned reg = MI->getOperand(i).getReg();
          if (TargetRegisterInfo::isVirtualRegister(reg)) {
            unsigned vreg = TRI.virtReg2Index(reg);
            unsigned lic = 0;
            RegMap::iterator rmi = regmap.find(vreg);
            if (rmi == regmap.end()) {
              // "allocate" a new physical register
              regmap[vreg] = lic = nextLICAlloc++;
              printf("allocating %d (%s) for vreg%d\n", lic,
              LPUInstPrinter::getRegisterName(lic), vreg);
            } else {
	      lic = regmap[vreg];
	    }
            MI->getOperand(i).setReg(lic);
          }
        }
      }
    }
  }

  return Modified;
}
