//===-- CSAAllocUnitPass.cpp - CSA Unit Allocation Pass -------------------===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Process NonSequential operations and allocate them to units.
//
//===----------------------------------------------------------------------===//

//#include <map>
#include "CSA.h"
#include "CSAInstrInfo.h"
#include "CSATargetMachine.h"
#include "CSAUtils.h"
#include "InstPrinter/CSAInstPrinter.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

using namespace llvm;

static cl::opt<int>
  AllocUnitPass("csa-alloc-unit", cl::Hidden,
                cl::desc("CSA Specific: Unit allocation pass"), cl::init(1));

static cl::opt<bool> UseAutounit(
  "csa-use-autounit",
  cl::desc(
    "CSA Specific: Let the assembler assign units for non-sequential operations"
    " automatically"),
  cl::init(true));

#define DEBUG_TYPE "csa-unit-alloc"
#define PASS_NAME "CSA: Allocate Unit Pass"

namespace {
class CSAAllocUnitPass : public MachineFunctionPass {
  std::map<int, int> IIToFU;

public:
  static char ID;
  CSAAllocUnitPass() : MachineFunctionPass(ID) {
    initializeCSAAllocUnitPassPass(*PassRegistry::getPassRegistry());
    // TBD(jsukha): Special case: unknown schedule class causes
    // problems.  Currently, LLVM "COPY" statements introduced by
    // other phases fall into this category.
    IIToFU[0] = CSA::FUNCUNIT::VIR;
    IIToFU[CSA::Sched::IIPseudo]    = CSA::FUNCUNIT::ALU;
    IIToFU[CSA::Sched::IIVir]       = CSA::FUNCUNIT::VIR;
    IIToFU[CSA::Sched::IIALU]       = CSA::FUNCUNIT::ALU;
    IIToFU[CSA::Sched::IISAdd]      = CSA::FUNCUNIT::ALU;
    IIToFU[CSA::Sched::IIShft]      = CSA::FUNCUNIT::SHF;
    IIToFU[CSA::Sched::IICmpF]      = CSA::FUNCUNIT::FCM;
    IIToFU[CSA::Sched::IIAddF32]    = CSA::FUNCUNIT::FMA;
    IIToFU[CSA::Sched::IIAddF64]    = CSA::FUNCUNIT::FMA;
    IIToFU[CSA::Sched::IIMulI8]     = CSA::FUNCUNIT::IMA;
    IIToFU[CSA::Sched::IIMulI16]    = CSA::FUNCUNIT::IMA;
    IIToFU[CSA::Sched::IIMulI32]    = CSA::FUNCUNIT::IMA;
    IIToFU[CSA::Sched::IIMulI64]    = CSA::FUNCUNIT::IMA;
    IIToFU[CSA::Sched::IIMulF32]    = CSA::FUNCUNIT::FMA;
    IIToFU[CSA::Sched::IIMulF64]    = CSA::FUNCUNIT::FMA;
    IIToFU[CSA::Sched::IIFMAF32]    = CSA::FUNCUNIT::FMA;
    IIToFU[CSA::Sched::IIFMAF64]    = CSA::FUNCUNIT::FMA;
    IIToFU[CSA::Sched::IIDivI8]     = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IIDivI16]    = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IIDivI32]    = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IIDivI64]    = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IIDivF32]    = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IIDivF64]    = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IISqrtF32]   = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IISqrtF64]   = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IIMathF32]   = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IIMathF64]   = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IIRcpAF32]   = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IIRcpAF64]   = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IIRSqrtAF32] = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IIRSqrtAF64] = CSA::FUNCUNIT::DIV;
    IIToFU[CSA::Sched::IICvtIF]     = CSA::FUNCUNIT::CIF;
    IIToFU[CSA::Sched::IICvtFI]     = CSA::FUNCUNIT::CFI;
    IIToFU[CSA::Sched::IICvtFF]     = CSA::FUNCUNIT::FMA;
    IIToFU[CSA::Sched::IILD]        = CSA::FUNCUNIT::MEM;
    IIToFU[CSA::Sched::IIST]        = CSA::FUNCUNIT::MEM;
    IIToFU[CSA::Sched::IIATM]       = CSA::FUNCUNIT::MEM;
    // temporarily commented out.  (If no patterns, the II doesn't get
    // defined...)
    //    IIToFU[CSA::Sched::IISeq    ] = CSA::FUNCUNIT::ALU;
    IIToFU[CSA::Sched::IICtl] = CSA::FUNCUNIT::SXU;
  }

  StringRef getPassName() const override { return PASS_NAME; }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
} // namespace

MachineFunctionPass *llvm::createCSAAllocUnitPass() {
  return new CSAAllocUnitPass();
}

char CSAAllocUnitPass::ID = 0;

INITIALIZE_PASS(CSAAllocUnitPass, DEBUG_TYPE, PASS_NAME, false, false)

bool CSAAllocUnitPass::runOnMachineFunction(MachineFunction &MF) {

  if (AllocUnitPass == 0)
    return false;

  //  const TargetMachine &TM = MF.getTarget();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  // Code starts out on the sequential unit
  bool isSequential = !(csa_utils::isAlwaysDataFlowLinkageSet());

  for (MachineFunction::iterator BB = MF.begin(), E = MF.end(); BB != E; ++BB) {
    //    DEBUG(errs() << "Basic block (name=" << BB->getName() << ") has "
    //    << BB->size() << " instructions.\n");

    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      MachineInstr *MI = &*I;
      // DEBUG(errs() << *I << "\n");

      if (UseAutounit) {

        // With UseAutounit, all non-sequential instructions should use
        // CSA::FUNCUNIT::Auto to have their units assigned automatically by the
        // assembler. Sequential instructions should still use
        // CSA::FUNCUNIT::SXU.
        if (MI->getFlag(MachineInstr::NonSequential)) {
          if (isSequential) {
            BuildMI(*BB, MI, MI->getDebugLoc(), TII.get(CSA::UNIT))
              .addImm(CSA::FUNCUNIT::Auto);
            isSequential = false;
          }
        } else {
          if (!isSequential) {
            BuildMI(*BB, MI, MI->getDebugLoc(), TII.get(CSA::UNIT))
              .addImm(CSA::FUNCUNIT::SXU);
            isSequential = true;
          }
        }

      } else {

        // Without UseAutounit, non-sequential operations should still have
        // their specific units assigned here.

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
          // instr type.  Then lookup the class based on the type.  This could
          // be moved to a separate function and made more sophisticated.  (e.g.
          // should shift[/add] be on a shift unit when the shift amount is
          // non-const and >3, but on an ALU otherwise?)
          unsigned schedClass = MI->getDesc().getSchedClass();
          unsigned unit       = IIToFU[schedClass];

          if (schedClass == 0) {
            // Print a warning message for instructions with unknown
            // schedule class.
            LLVM_DEBUG(
              errs()
              << "WARNING: Encountered machine instruction " << *MI
              << " with unknown schedule class. Assigning to virtual unit.\n");
          }

          LLVM_DEBUG(errs() << "MI " << *MI << ": schedClass " << schedClass
                     << " maps to unit " << unit << "\n");
          BuildMI(*BB, MI, MI->getDebugLoc(), TII.get(CSA::UNIT)).addImm(unit);
          isSequential = (unit == CSA::FUNCUNIT::SXU);
        } else if (!isSequential) {
          BuildMI(*BB, MI, MI->getDebugLoc(), TII.get(CSA::UNIT))
            .addImm(CSA::FUNCUNIT::SXU);
          isSequential = true;
        }
      }
    }

    // If we are NOT ending the block on the sequential unit, add a unit switch
    // so that the successive block (and in particular, the label starting
    // the block) will be on the sxu, even if later instructions are not.
    // (Basically, block boundaries represent flow control, and flow control
    // MUST be on the sequential unit...)
    if (!isSequential && !(csa_utils::isAlwaysDataFlowLinkageSet())) {
      BuildMI(*BB, BB->end(), DebugLoc(), TII.get(CSA::UNIT))
        .addImm(CSA::FUNCUNIT::SXU);
      isSequential = true;
    }
  }

  bool Modified = false;

  return Modified;
}
