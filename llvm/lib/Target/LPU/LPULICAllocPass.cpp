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
#include "llvm/CodeGen/MachineRegisterInfo.h"
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
  MachineRegisterInfo *MRI = &MF.getRegInfo();
// const TargetFrameLowering &TFI = *TM.getSubtargetImpl()->getFrameLowering();
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  bool Modified = false;

  typedef std::map<int,int> RegMap;
  // Map from virtual register index, to physical register/LIC number
  RegMap virtToLIC;
  // Map from LIC to a replacement LIC
  RegMap LICToLIC;
  BitVector mustBeReg;
  // Hack.  Start packing with c20.
  // c0/c1 - control ouput/input
  // c2..c3 - up to 2 results
  // c4..c19 - up to 16 input params
  const int FirstAllocatable = 1;//LPU::C20;
  int nextLICAlloc = FirstAllocatable;

  // For a 1st cut, simply allocate a LIC ("physical register") for each
  // virtual register as they are encountered - i.e. there is no real
  // "allocation".
  // If we change to using LICs as a separate operand type, this is where
  // they would be introduced...

  //  MRI->setPhysRegUsed(LPU::C0);
  //  MRI->setPhysRegUsed(LPU::C1);
  //  MRI->addLiveIn(LPU::C1);

  // 1st pass - allocate LICs for each VR.
  // We also track through (and remove) copies
  for (MachineFunction::iterator BB = MF.begin(), E = MF.end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      MachineInstr *MI = I;
      if (MI->isPHI()) {
        // Loop through operands.
        // - If 0 phys reg, allocate a new phys reg
        // - If 1 phys reg., map all others to it
        // - If >1 phys reg. - problem - we need to insert a rename at a source...
        //   (ignore for temp solution?)
        int preg = -1;
        for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
          if (MI->getOperand(i).isReg()) {
            unsigned reg = MI->getOperand(i).getReg();
            int thispreg = -1;
            if (TargetRegisterInfo::isVirtualRegister(reg)) {
              unsigned vidx = TRI.virtReg2Index(reg);
              // If there is a mapping, that is the preg.  If not, not a constraint
              RegMap::iterator rmi = virtToLIC.find(vidx);
              if (rmi != virtToLIC.end()) {
                thispreg = rmi->second;
                MI->getOperand(i).setReg(thispreg);
                //printf("phi opnd %d - changing v%d to %s\n",i,vidx,LPUInstPrinter::getRegisterName(thispreg));
              } else {
                //printf("phi opnd %d - unmapped vreg v%d\n",i,vidx);
              }
            } else {
              thispreg = reg;
              //printf("phi opnd %d - already %s\n",i,LPUInstPrinter::getRegisterName(thispreg));
            }
            if (thispreg != -1) {
              assert((preg==-1 || preg==thispreg) &&
                     "can't deal with PHI with multiple physregs");
              preg = thispreg;
            }
          }
        }
        // If no preg yet, allocate one
        if (preg == -1) {
          preg = nextLICAlloc++;
          //printf("phi alloc preg %d\n", preg);
        }
        // Convert all virtuals to the preg and mark each for mapping...
        for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
          if (MI->getOperand(i).isReg()) {
            unsigned reg = MI->getOperand(i).getReg();
            if (TargetRegisterInfo::isVirtualRegister(reg)) {
              unsigned vidx = TRI.virtReg2Index(reg);
              // If there is no mapping, make it be to this phys reg
              RegMap::iterator rmi = virtToLIC.find(vidx);
              if (rmi == virtToLIC.end()) {
                virtToLIC[vidx] = preg;
                //printf("phi added mapping v%d to %s\n", vidx, LPUInstPrinter::getRegisterName(preg));
              }
              // Check that the mapping is right
              //printf("phi index: v%d, to %s, preg is %s\n", vidx,
              //       LPUInstPrinter::getRegisterName(virtToLIC[vidx]),
              //       LPUInstPrinter::getRegisterName(preg));
              assert(virtToLIC[vidx] == preg && "inconsistent mapping");
              MI->getOperand(i).setReg(preg);
            }
          }
        }
      } else if (MI->isCopy()) {
        // If this is a copy, handle specially.
        // If the output is a VR, we do not allocate a new register,
        // but instead mark the mapping for the output VR to be to the input
        // to the copy.  If the output is a physical register, we note that on
        // a second pass, a replacement should take place.
        unsigned Dst = MI->getOperand(0).getReg();
        unsigned Src = MI->getOperand(1).getReg();
        if (TargetRegisterInfo::isVirtualRegister(Src)) {
          Src = virtToLIC[TRI.virtReg2Index(Src)];
          MI->getOperand(1).setReg(Src);
        }
        MRI->setPhysRegUsed(Src);
        if (TargetRegisterInfo::isVirtualRegister(Dst)) {
          // Set the translation for the copy-defined Virt to be the input Phys
          Dst = virtToLIC[TRI.virtReg2Index(Dst)] = Src;
          MI->getOperand(0).setReg(Src);
        }
        MRI->setPhysRegUsed(Dst);
        // If the source doesn't match the dest, AND the source isn't
        // an input or output, plan to map the Src to the Dst on the next sweep
        if (Src != Dst && Src >= (unsigned)FirstAllocatable) {
          //printf("inserting %d => %d\n", Src, Dst);
          LICToLIC[Src] = Dst;
        }
      } else {
        // For each operand, if it is a VR, and there is no phys reg already
        // assigned, assign one.  Regardless, replace all VRs with phys regs.
        for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
          if (MI->getOperand(i).isReg()) {
            unsigned reg = MI->getOperand(i).getReg();
            unsigned lic = 0;
            if (TargetRegisterInfo::isVirtualRegister(reg)) {
              unsigned vidx = TRI.virtReg2Index(reg);
              RegMap::iterator rmi = virtToLIC.find(vidx);
              if (rmi == virtToLIC.end()) {
                // "allocate" a new physical register
                virtToLIC[vidx] = lic = nextLICAlloc++;
                //printf("allocating %d (%s) for vreg%d\n", lic,
                //       LPUInstPrinter::getRegisterName(lic), vidx);
              } else {
                lic = virtToLIC[vidx];
              }
              MI->getOperand(i).setReg(lic);
            } else {
              lic = reg;
            }
            MRI->setPhysRegUsed(lic);
          }
        }
      }
    }
  }

  // 2nd pass to remove copies targeting assigned outputs (e.g. results.)
  for (MachineFunction::iterator BB = MF.begin(), E = MF.end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end();) {
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
      ++I;
      if (MI->isPHI()) {
        // Remove it...
        MI->eraseFromParent();
      }
    }
  }

  // for any LICs that were used in C3..C19, mark as live in
  /*
  for (unsigned reg=LPU::C3; reg<=LPU::C19; reg++) {
    if (MRI->isPhysRegUsed(reg)) {
      MRI->addLiveIn(reg);
    }
  }
  */

  return Modified;
}
