//===-- LPULicAllocation.cpp - LPU Frame Information ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains LIC Allocation support.
//
//===----------------------------------------------------------------------===//

#include "LPULicAllocation.h"

#define DEBUG_TYPE "lic-alloc"

#include <map>
#include "InstPrinter/LPUInstPrinter.h"
#include "LPU.h"
#include "LPUInstrInfo.h"
#include "LPUTargetMachine.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;

//insert PICKs/SWITCHes/init code to make live range LIC allocatable
bool LPULicAllocation::
makeLiveRangesLicAllocatable(MachineInstr *MI, MachineBasicBlock* BB, 
                             std::set<unsigned> &LiveRangesSet) {
/*
  for (unsigned i = 1, e = MI->getNumOperands(); i != e; i+=2) {
    MachineOperand &currOperand = MI->getOperand(i);
    if (currOperand.isReg()) {
      unsigned Reg = currOperand.getReg();
      if (LiveRangesSet.count(Reg) != 0) continue;
      LiveRangesSet.insert(Reg);
      if (currOperand.isUse()) { //src
        MachineInstr *DefMI = MRI->getVRegDef(Reg)
	if (DefMI && (DefMI->getParent() != BB)) { //live into BB
          // insert PICK and new live range
          MachineBasicBlock* DefBB = DefMI->getParent();
	  if (DefBB->hasMultipleSuccessors() &&  !DefMI->isSwitch()) {
	  // insert PICK and new live range
          }
        }
      }
      if (currOperand.isDef()) { //dest reg
      }
    }

    if (currOperand.isImm()) {
    }
  }  
*/
  return false;
}

//allocate LICs to live ranges that are LIC allocatable
bool LPULicAllocation::
allocateLicsInLoop(MachineInstr *MI, MachineBasicBlock* BB) {
  //  const TargetMachine &TM = BB->getParent()->getTarget();
  //  const TargetRegisterInfo *TRI = TM.getSubtargetImpl()->getRegisterInfo();
  //  MachineRegisterInfo *MRI = &BB->getParent()->getRegInfo();
  bool modified = false;
  for (unsigned i = 1, e = MI->getNumOperands(); i != e; i+=2) {
    MachineOperand &currOperand = MI->getOperand(i);
    if (currOperand.isReg()) {
      /*
      unsigned Reg = currOperand.getReg();
      if (TRI->isLIC(Reg)) continue;
      MachineInstr *DefMI = MRI->getVRegDef(Reg)
      if (DefMI && DefMI->getParent() != BB) continue;
      bool allocLic = true;
      unsigned numUses = 0;
      for each use of Reg {
        numUses++;
	if (useMI->getParent() != BB) {
          allocLic = false;
	  break;
	} 
      }
      if (!allocLic) continue;
      if (numUses > 1) {
	 continue;
	 //generateCopyAndRename(Reg,numUses);
      }
      else { 
	assert(numUses == 1 && "LPU LIC allocation insanity");
        TRI->mapRegToLIC(Reg); 
        modified = true;
      }
      */
    }
  }
  return modified;
}
