//===-- LPUAllocUnitPass.cpp - LPU Unit Allocation Pass -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Process NonSequential operations and allocate them to units.
//
//===----------------------------------------------------------------------===//

//#include <map>
#include "LPU.h"
#include "InstPrinter/LPUInstPrinter.h"
#include "LPUInstrInfo.h"
#include "LPUTargetMachine.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;

static cl::opt<int>
AllocUnitPass("lpu-alloc-unit", cl::Hidden,
		   cl::desc("LPU Specific: Unit allocation pass"),
		   cl::init(1));

#define DEBUG_TYPE "lpu-unit-alloc"

namespace {
class LPUAllocUnitPass : public MachineFunctionPass {
public:
  static char ID;
  LPUAllocUnitPass() : MachineFunctionPass(ID) { }

  const char* getPassName() const override {
    return "LPU Allocate Unit Pass";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
}

MachineFunctionPass *llvm::createLPUAllocUnitPass() {
  return new LPUAllocUnitPass();
}

char LPUAllocUnitPass::ID = 0;

bool LPUAllocUnitPass::runOnMachineFunction(MachineFunction &MF) {

  if (AllocUnitPass == 0) return false;

  //  const TargetMachine &TM = MF.getTarget();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  // Code starts out on the sequential unit
  bool isSequential = true;

  for (MachineFunction::iterator BB = MF.begin(), E = MF.end(); BB != E; ++BB) {
    // The next statement works since operator<<(ostream&,...)
    // is overloaded for Instruction&
    DEBUG(errs() << "Basic block (name=" << BB->getName() << ") has "
          << BB->size() << " instructions.\n");

    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      MachineInstr *MI=I;
      // The next statement works since operator<<(ostream&,...)
      // is overloaded for Instruction&
      DEBUG(errs() << *I << "\n");

      // If this operation has the NonSequential flag set, allocate a UNIT
      // pseudo-op based on the instruction's preferred functional unit kind.
      // (Is the BuildMI right?  The only operand to UNIT is the literal
      // for the unit.  The doc describes it only as the target register.
      // But, UNIT doesn't have a target register...)
      // TODO: Need to query the scheduler tables (Inst Itinerary) to find
      // the functional unit that should be used for MI.
      // TODO?: Should the UNIT and op cells be placed in an instruction
      // bundle?
      if (MI->getFlag(MachineInstr::NonSequential)) {
        BuildMI(*BB, MI, MI->getDebugLoc(), TII.get(LPU::UNIT), 0 /* approp unit */);
        isSequential = false;
      } else if (!isSequential) {
        BuildMI(*BB, MI, MI->getDebugLoc(), TII.get(LPU::UNIT), 0 /* SXU */);
        isSequential = true;
      }

    }

  }

  bool Modified = false;

  return Modified;

}
