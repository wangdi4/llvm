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
  std::map<int,int> IIToFU;
public:
  static char ID;
  LPUAllocUnitPass() : MachineFunctionPass(ID) {
    // TBD(jsukha): Special case: unknown schedule class causes
    // problems.  Currently, LLVM "COPY" statements introduced by
    // other phases fall into this category.
    IIToFU[0] = LPU::FUNCUNIT::VIR;
    
    IIToFU[LPU::Sched::IIPseudo ] = LPU::FUNCUNIT::ALU;
    IIToFU[LPU::Sched::IIVir    ] = LPU::FUNCUNIT::VIR;
    IIToFU[LPU::Sched::IIALU    ] = LPU::FUNCUNIT::ALU;
    IIToFU[LPU::Sched::IISAdd   ] = LPU::FUNCUNIT::ALU;
    IIToFU[LPU::Sched::IIShft   ] = LPU::FUNCUNIT::SHF;
    IIToFU[LPU::Sched::IICmpF   ] = LPU::FUNCUNIT::FCM;
    IIToFU[LPU::Sched::IIAddF16 ] = LPU::FUNCUNIT::FMA;
    IIToFU[LPU::Sched::IIAddF32 ] = LPU::FUNCUNIT::FMA;
    IIToFU[LPU::Sched::IIAddF64 ] = LPU::FUNCUNIT::FMA;
    IIToFU[LPU::Sched::IIMulI8  ] = LPU::FUNCUNIT::IMA;
    IIToFU[LPU::Sched::IIMulI16 ] = LPU::FUNCUNIT::IMA;
    IIToFU[LPU::Sched::IIMulI32 ] = LPU::FUNCUNIT::IMA;
    IIToFU[LPU::Sched::IIMulI64 ] = LPU::FUNCUNIT::IMA;
    IIToFU[LPU::Sched::IIMulF16 ] = LPU::FUNCUNIT::FMA;
    IIToFU[LPU::Sched::IIMulF32 ] = LPU::FUNCUNIT::FMA;
    IIToFU[LPU::Sched::IIMulF64 ] = LPU::FUNCUNIT::FMA;
    IIToFU[LPU::Sched::IIFMAF16 ] = LPU::FUNCUNIT::FMA;
    IIToFU[LPU::Sched::IIFMAF32 ] = LPU::FUNCUNIT::FMA;
    IIToFU[LPU::Sched::IIFMAF64 ] = LPU::FUNCUNIT::FMA;
    IIToFU[LPU::Sched::IIDivI8  ] = LPU::FUNCUNIT::DIV;
    IIToFU[LPU::Sched::IIDivI16 ] = LPU::FUNCUNIT::DIV;
    IIToFU[LPU::Sched::IIDivI32 ] = LPU::FUNCUNIT::DIV;
    IIToFU[LPU::Sched::IIDivI64 ] = LPU::FUNCUNIT::DIV;
    IIToFU[LPU::Sched::IIDivF16 ] = LPU::FUNCUNIT::DIV;
    IIToFU[LPU::Sched::IIDivF32 ] = LPU::FUNCUNIT::DIV;
    IIToFU[LPU::Sched::IIDivF64 ] = LPU::FUNCUNIT::DIV;
    IIToFU[LPU::Sched::IISqrtF16] = LPU::FUNCUNIT::DIV;
    IIToFU[LPU::Sched::IISqrtF32] = LPU::FUNCUNIT::DIV;
    IIToFU[LPU::Sched::IISqrtF64] = LPU::FUNCUNIT::DIV;
    IIToFU[LPU::Sched::IIMathF16] = LPU::FUNCUNIT::DIV;
    IIToFU[LPU::Sched::IIMathF32] = LPU::FUNCUNIT::DIV;
    IIToFU[LPU::Sched::IIMathF64] = LPU::FUNCUNIT::DIV;
    IIToFU[LPU::Sched::IICvtIF  ] = LPU::FUNCUNIT::CIF;
    IIToFU[LPU::Sched::IICvtFI  ] = LPU::FUNCUNIT::CFI;
    IIToFU[LPU::Sched::IICvtFF  ] = LPU::FUNCUNIT::FMA;
    IIToFU[LPU::Sched::IILD     ] = LPU::FUNCUNIT::MEM;
    IIToFU[LPU::Sched::IIST     ] = LPU::FUNCUNIT::MEM;
    IIToFU[LPU::Sched::IIATM    ] = LPU::FUNCUNIT::MEM;
    // temporarily commented out.  (If no patterns, the II doesn't get
    // defined...)
    //    IIToFU[LPU::Sched::IISeq    ] = LPU::FUNCUNIT::ALU;
    IIToFU[LPU::Sched::IICtl    ] = LPU::FUNCUNIT::SXU;
  }

  StringRef getPassName() const override {
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
    //    DEBUG(errs() << "Basic block (name=" << BB->getName() << ") has "
    //    << BB->size() << " instructions.\n");

    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      MachineInstr *MI=&*I;
      //DEBUG(errs() << *I << "\n");

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
        // Get the scheduling class (II - InstructionItinerary) value from the
        // instr type.  Then lookup the class based on the type.  This could be
        // moved to a separate function and made more sophisticated.  (e.g.
        // should shift[/add] be on a shift unit when the shift amount is
        // non-const and >3, but on an ALU otherwise?)
        unsigned schedClass = MI->getDesc().getSchedClass();
        unsigned unit = IIToFU[schedClass];


        if (schedClass == 0) {
          // Print a warning message for instructions with unknown
          // schedule class.
          DEBUG(errs() << "WARNING: Encountered machine instruction " <<
                *MI << " with unknown schedule class. Assigning to virtual unit.\n");
        }

        DEBUG(errs() << "MI " << *MI << ": schedClass " << schedClass <<
              " maps to unit " << unit << "\n");
        BuildMI(*BB, MI, MI->getDebugLoc(), TII.get(LPU::UNIT)).
          addImm(unit);
        isSequential = (unit == LPU::FUNCUNIT::SXU);
      } else if (!isSequential) {
        BuildMI(*BB, MI, MI->getDebugLoc(), TII.get(LPU::UNIT)).
          addImm(LPU::FUNCUNIT::SXU);
        isSequential = true;
      }

    }

    // If we are NOT ending the block on the sequential unit, add a unit switch
    // so that the successive block (and in particular, the label starting
    // the block) will be on the sxu, even if later instructions are not.
    // (Basically, block boundaries represent flow control, and flow control
    // MUST be on the sequential unit...)
    if (!isSequential) {
      BuildMI(*BB, BB->end(), DebugLoc(), TII.get(LPU::UNIT)).
        addImm(LPU::FUNCUNIT::SXU);
      isSequential = true;
    }

  }

  bool Modified = false;

  return Modified;

}
