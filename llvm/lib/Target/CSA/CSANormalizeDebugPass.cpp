//===- CSANormalizeDebug.cpp - Connect LICs only used by DBG_VALUEs  --===//
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSATargetMachine.h"
#include "CSAInstrInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;

#define DEBUG_TYPE "csa-normalize-debug"

STATISTIC(NumDbgValueMovs,
    "Number of MOVs added to connect LICs named by DBG_VALUEs");

namespace llvm {
namespace CSA { // Register classes
  // Register class for ANYC, a superclass representing a channel of any width.
  // This constant is declared in "CSAGenRegisterInfo.inc", but that file is
  // huge and #including it would slow compilation.
  extern const TargetRegisterClass ANYCRegClass;
}
}

namespace {
  class CSANormalizeDebug : public MachineFunctionPass {
    bool runOnMachineFunction(MachineFunction &MF) override;

    const TargetRegisterInfo  *TRI;
    const MachineRegisterInfo *MRI;
    const CSAInstrInfo        *TII;

  public:
    static char ID; // Pass identification, replacement for typeid
    CSANormalizeDebug() : MachineFunctionPass(ID) { }
  };
}

// The declaration for this factory function is in file "CSA.h"
MachineFunctionPass *llvm::createCSANormalizeDebugPass() {
  return new CSANormalizeDebug();
}

char CSANormalizeDebug::ID = 0;

bool CSANormalizeDebug::runOnMachineFunction(MachineFunction &MF) {

  if (skipFunction(*MF.getFunction()))
    return false;

  bool AnyChanges = false;
  MRI = &MF.getRegInfo();
  TRI = MF.getSubtarget().getRegisterInfo();
  TII = static_cast<const CSAInstrInfo*>(MF.getSubtarget<CSASubtarget>().getInstrInfo());

  for (MachineBasicBlock &MB : MF) {
    for (MachineInstr &MI : MB) {
      if (!(MI.isDebugValue() && MI.getNumOperands()>0 && MI.getOperand(0).isReg()))
        continue;

      unsigned reg = MI.getOperand(0).getReg();

      // Only worry about DBG_VALUEs for LICs.
      if (!CSA::ANYCRegClass.contains(reg))
        continue;

      // Skip LICs with no producer.
      if (MRI->def_empty(reg))
        continue;

      // Skip LICs which have another use -- they are already connected.
      if (!MRI->use_nodbg_empty(reg))
        continue;

      // Determine the size of MOV to be used according to the LIC producer.
      // If OpInfo doesn't know of a specific register class for any reason,
      // fall back to 64-bit. This is a bit painful because we're dealing with
      // a physical register which we just happen to know only has a single
      // def/producer.
      assert(MRI->hasOneDef(reg) && "LIC has multiple producers?");
      MachineOperand *defOp = &*(MRI->def_begin(reg));
      MachineInstr *defInst = defOp->getParent();
      unsigned opNum = defInst->getOperandNo(defOp);
      unsigned rcEnum = defInst->getDesc().OpInfo[opNum].RegClass;
      bool rcKnown = rcEnum < TRI->getNumRegClasses();
      const TargetRegisterClass* rc = rcKnown ? TRI->getRegClass(rcEnum) : &CSA::CI64RegClass;

      MachineInstr *ignMov = BuildMI(*MI.getParent(), &MI, DebugLoc(),
          TII->get(TII->getMoveOpcode(rc)), CSA::IGN).addReg(reg);
      ignMov->setFlag(MachineInstr::NonSequential);
      NumDbgValueMovs++;
      AnyChanges = true;
    }
  }

  return AnyChanges;
}
