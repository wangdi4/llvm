//===-- X86FixupBWInsts.cpp - Fixup Byte or Word instructions -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the pass that looks through the machine instructions
// late in the compilation, and finds byte or word instructions that
// can be profitably replaced with 32 bit instructions that give equivalent
// results for the bits of the results that are used. There are two possible
// reasons to do this.
//
// One reason is to avoid false-dependences on the upper portions
// of the registers.  Only instructions that have a destination register which
// is not in any of the src registers can be affected by this.  Those
// instructions are primarily loads, and register-to-register moves.  It would
// seem like cmov(s) would also be affected, but because of the way cmov is
// really implemented by most machines as reading both the dst and src regs,
// and then "merging" the two based on a condition, it really should be
// considered as reading the dst as well.
//
// The other reason to do this is for potential code size savings.  Word
// operations need an extra override byte compared to their 32 bit
// versions. So this can convert many
// word operations to their larger size, saving a byte in encoding. This may
// introduce partial register dependencies, so this should only be done
// when you can prove the partial register dependence won't exist, or when
// optimizing for minimum code size.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
using namespace llvm;

#define DEBUG_TYPE "x86-fixup-bw-insts"

namespace {
class FixupBWInstPass : public MachineFunctionPass {
  static char ID;

  const char *getPassName() const override {
    return "X86 Byte/Word Instruction Fixup";
  }

  /// \brief Loop over all of the instructions in the basic block
  /// replacing applicable byte or word instructions with better
  /// alternatives.
  void processBasicBlock(MachineFunction &MF,
                         MachineBasicBlock &MBB) const;

  /// \brief This sets the SuperDestReg to the 32 bit super reg
  /// of the original destination register of the MachineInstr MI
  /// passed in. It returns true if that super register is dead
  /// just prior to OrigMI, and false if not.
  bool getSuperRegDestIfDead(MachineInstr *OrigMI,
                             MVT::SimpleValueType OrigDestVT,
                             unsigned &SuperDestReg) const;

  /// \brief Change the MachineInstr MI into the equivalent extending load
  /// to 32 bit register if it is safe to do so.  Return the replacement
  /// instruction if OK, otherwise return nullptr.
  MachineInstr *tryReplaceLoad(unsigned New32BitOpcode,
                               MVT::SimpleValueType OrigDestVT,
                               MachineInstr *MI) const;

public:
  FixupBWInstPass() : MachineFunctionPass(ID) {}

  /// \brief Loop over all of the basic blocks,
  /// replacing byte and word instructions by equivalent 32 bit instructions
  /// where performance or code size can be improved.
  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  MachineFunction *MF;
  const X86InstrInfo *TII; // Machine instruction info.
  bool OptForSize;
};
char FixupBWInstPass::ID = 0;
}

FunctionPass *llvm::createX86FixupBWInsts() { return new FixupBWInstPass(); }

bool FixupBWInstPass::runOnMachineFunction(MachineFunction &MF) {
  this->MF = &MF;
  TII = MF.getSubtarget<X86Subtarget>().getInstrInfo();
  OptForSize = MF.getFunction()->optForSize();

  DEBUG(dbgs() << "Start X86FixupBWInsts\n";);

  // Process all basic blocks.
  for (auto &MBB : MF)
    processBasicBlock(MF, MBB);

  DEBUG(dbgs() << "End X86FixupBWInsts\n";);

  return true;
}

bool FixupBWInstPass::getSuperRegDestIfDead(MachineInstr *MI,
                                            MVT::SimpleValueType OrigDestVT,
                                            unsigned &SuperDestReg) const {

  unsigned OrigDestReg = MI->getOperand(0).getReg();
  SuperDestReg = getX86SubSuperRegister(OrigDestReg, MVT::i32);

  // Make sure that the sub-register that this instruction has as its
  // destination is the lowest order sub-register of the super-register.
  // If it isn't, then the register isn't really dead even if the
  // super-register is considered dead.
  // This test works because getX86SubSuperRegister returns the low portion
  // register by default when getting a sub-register, so if that doesn't
  // match the original destination register, then the original destination
  // register must not have been the low register portion of that size.
  if (getX86SubSuperRegister(SuperDestReg, OrigDestVT) != OrigDestReg)
    return false;

  MachineBasicBlock::LivenessQueryResult NewDestLQR =
    MI->getParent()->computeRegisterLiveness(&TII->getRegisterInfo(),
                                             SuperDestReg, MI);

  return NewDestLQR == MachineBasicBlock::LQR_Dead;
}

MachineInstr *FixupBWInstPass::tryReplaceLoad(unsigned New32BitOpcode,
                                              MVT::SimpleValueType OrigDestVT,
                                              MachineInstr *MI) const {
  unsigned NewDestReg;

  if (!getSuperRegDestIfDead(MI, OrigDestVT, NewDestReg))
    return nullptr;

  // Safe to change the instruction.

  MachineInstrBuilder MIB =
      BuildMI(*MF, MI->getDebugLoc(), TII->get(New32BitOpcode), NewDestReg);

  unsigned NumArgs = MI->getNumOperands();
  for (unsigned i = 1; i < NumArgs; ++i)
    MIB.addOperand(MI->getOperand(i));

  MIB->setMemRefs(MI->memoperands_begin(), MI->memoperands_end());

  return MIB;
}

void FixupBWInstPass::processBasicBlock(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {

  //
  // This algorithm doesn't delete the instructions it is replacing
  // right away.  By leaving the existing instructions in place, the
  // register liveness information doesn't change, and this makes the
  // analysis that goes on be better than if the replaced instructions
  // were immediately removed.
  //
  // This algorithm always adds a replacement instruction
  // before the original instruction it is replacing. In the old instruction,
  // the definition of the partial register is considered as a kill of
  // the full register.  So, leaving in the old instruction serves
  // to show that the upper portion of the full register continues to be dead.
  // Whereas, if the old instruction is removed, now it looks as if the upper
  // portion of the register may be used in subsequent instructions, and that
  // may inhibit later instructions that use the partial register from
  // seeing that the upper portion of the register is dead. 
  //
  // So, this data structure keeps track of the instructions that
  // have been replaced and are to be deleted prior to exiting this
  // function.
  SmallVector<MachineInstr *, 8> MIToBeDeleted;
  bool IsSingleBBLoop = MBB.isSuccessor(&MBB);

  for (MachineBasicBlock::iterator I = MBB.begin(); I != MBB.end(); ++I) {
    MachineInstr *NewMI = nullptr;
    MachineInstr *MI = I;

    // See if this is an instruction of the type we are currently looking for.
    switch (MI->getOpcode()) {

    case X86::MOV8rm:
      // Only replace 8 bit loads with the zero extending versions if
      // in a very tight loop and not optimizing for size. This takes
      // an extra byte to encode, and provides limited performance upside.
      if (IsSingleBBLoop && !OptForSize)
        NewMI = tryReplaceLoad(X86::MOVZX32rm8, MVT::i8, MI);
      break;

    case X86::MOV16rm:
      // Always try to replace 16 bit load with 32 bit zero extending.
      // Code size is the same, and there is sometimes a perf advantage
      // from eliminating a false dependence on the upper portion of
      // the register.
      NewMI = tryReplaceLoad(X86::MOVZX32rm16, MVT::i16, MI);
      break;

    default:
      // nothing to do here.
      break;
    }

    if (NewMI) {
      MBB.insert(I, NewMI);
      MIToBeDeleted.push_back(MI);
    }
  }

  while (!MIToBeDeleted.empty()) {
    MachineInstr *MI = MIToBeDeleted.back();
    MIToBeDeleted.pop_back();
    MBB.erase(MI);
  }
}
