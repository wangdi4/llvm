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
// const TargetFrameLowering &TFI = *TM.getSubtargetImpl()->getFrameLowering();
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  bool Modified = false;

  typedef std::map<int,int> RegMap;
  // Map from virtual register index, to physical register number
  RegMap virtToLIC;
  // Map from LIC to a replacement LIC
  RegMap LICToLIC;
  // Hack.  Start packing with c20.
  // c0/c1 - control input/output
  // c2..c17 - up to 16 input params
  // c18..c19 - up to 2 results
  const int FirstAllocatable = LPU::C20;
  int nextLICAlloc = FirstAllocatable;

  // For a 1st cut, simply allocate a LIC ("physical register") for each
  // virtual register as they are encountered - i.e. there is no real
  // "allocation".
  // If we change to using LICs as a separate operand type, this is where
  // they would be introduced...

  // 1st pass - allocate LICs for each VR.
  // We also track through (and remove) copies
  for (MachineFunction::iterator BB = MF.begin(), E = MF.end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      MachineInstr *MI = I;
      // If this is a copy, handle specially.
      // If the output is a VR, we do not allocate a new register,
      // but instead mark the mapping for the output VR to be to the input
      // to the copy.  If the output is a physical register, we note that on
      // a second pass, a replacement should take place.
      if (MI->isCopy()) {
        unsigned Dst = MI->getOperand(0).getReg();
        unsigned Src = MI->getOperand(1).getReg();
        if (TargetRegisterInfo::isVirtualRegister(Src)) {
          Src = virtToLIC[TRI.virtReg2Index(Src)];
          MI->getOperand(1).setReg(Src);
        }
        if (TargetRegisterInfo::isVirtualRegister(Dst)) {
          // Set the translation for the copy-defined Virt to be the input Phys
          Dst = virtToLIC[TRI.virtReg2Index(Dst)] = Src;
          MI->getOperand(0).setReg(Src);
        }
        // If the source doesn't match the dest, AND the source isn't
        // an input or output, plan to map the Src to the Dst on the next sweep
        if (Src != Dst && Src >= FirstAllocatable) {
          printf("inserting %d => %d\n", Src, Dst);
          LICToLIC[Src] = Dst;
        }
      } else {
        // For each operand, if it is a VR, and there is no phys reg already
        // assigned, assign one.  Regardless, replace all VRs with phys regs.
        for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
          if (MI->getOperand(i).isReg()) {
            unsigned reg = MI->getOperand(i).getReg();
            if (TargetRegisterInfo::isVirtualRegister(reg)) {
              unsigned vidx = TRI.virtReg2Index(reg);
              unsigned lic = 0;
              RegMap::iterator rmi = virtToLIC.find(vidx);
              if (rmi == virtToLIC.end()) {
                // "allocate" a new physical register
                virtToLIC[vidx] = lic = nextLICAlloc++;
                printf("allocating %d (%s) for vreg%d\n", lic,
                       LPUInstPrinter::getRegisterName(lic), vidx);
              } else {
                lic = virtToLIC[vidx];
              }
              MI->getOperand(i).setReg(lic);
            }
          }
        }
      }
    }
  }
  // 2nd pass to remove copies targeting assigned outputs (e.g. results.)
  for (MachineFunction::iterator BB = MF.begin(), E = MF.end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      MachineInstr *MI = I;
      for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
        if (MI->getOperand(i).isReg()) {
          unsigned lic = MI->getOperand(i).getReg();
          RegMap::iterator rmi = LICToLIC.find(lic);
          if (rmi != LICToLIC.end()) {
            MI->getOperand(i).setReg(rmi->second);
          }
        }
      }
    }
  }

  return Modified;
}
