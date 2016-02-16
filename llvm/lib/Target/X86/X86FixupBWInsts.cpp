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
#include "llvm/CodeGen/MachineLoopInfo.h"
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
                             unsigned OrigDestSize,
                             unsigned &SuperDestReg) const;

  /// \brief Change the MachineInstr MI into the equivalent extending load
  /// to 32 bit register if it is safe to do so.  Return the replacement
  /// instruction if OK, otherwise return nullptr.
  MachineInstr *tryReplaceLoad(unsigned New32BitOpcode,
                               unsigned OrigDestSize,
                               MachineInstr *MI) const;

public:
  FixupBWInstPass() : MachineFunctionPass(ID) {}
  
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfo>();   //  Need machine loop info.
    MachineFunctionPass::getAnalysisUsage(AU);
  }
  
  /// \brief Loop over all of the basic blocks,
  /// replacing byte and word instructions by equivalent 32 bit instructions
  /// where performance or code size can be improved.
  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  MachineFunction *MF;
  const X86InstrInfo *TII; // Machine instruction info.
  bool OptForSize;
  MachineLoopInfo *MLI;  // Machine loop info.
};
char FixupBWInstPass::ID = 0;
}

FunctionPass *llvm::createX86FixupBWInsts() { return new FixupBWInstPass(); }

bool FixupBWInstPass::runOnMachineFunction(MachineFunction &MF) {
  this->MF = &MF;
  TII = MF.getSubtarget<X86Subtarget>().getInstrInfo();
  OptForSize = MF.getFunction()->optForSize();
  MLI = &getAnalysis<MachineLoopInfo>();

  DEBUG(dbgs() << "Start X86FixupBWInsts\n";);

  // Process all basic blocks.
  for (auto &MBB : MF)
    processBasicBlock(MF, MBB);

  DEBUG(dbgs() << "End X86FixupBWInsts\n";);

  return true;
}

bool FixupBWInstPass::getSuperRegDestIfDead(MachineInstr *MI,
                                            unsigned OrigDestSize,
                                            unsigned &SuperDestReg) const {

  unsigned OrigDestReg = MI->getOperand(0).getReg();
  SuperDestReg = getX86SubSuperRegister(OrigDestReg, 32);

  // Make sure that the sub-register that this instruction has as its
  // destination is the lowest order sub-register of the super-register.
  // If it isn't, then the register isn't really dead even if the
  // super-register is considered dead.
  // This test works because getX86SubSuperRegister returns the low portion
  // register by default when getting a sub-register, so if that doesn't
  // match the original destination register, then the original destination
  // register must not have been the low register portion of that size.
  if (getX86SubSuperRegister(SuperDestReg, OrigDestSize) != OrigDestReg)
    return false;

  MachineBasicBlock::LivenessQueryResult LQR =
    MI->getParent()->computeRegisterLiveness(&TII->getRegisterInfo(),
                                             SuperDestReg, MI);

  if (LQR != MachineBasicBlock::LQR_Dead)
    return false;

  if (OrigDestSize == 8) {
    // In the case of byte registers, we also have to check that the upper
    // byte register is also dead. That is considered to be independent of
    // whether the super-register is dead.
    unsigned UpperByteReg =
      getX86SubSuperRegister(SuperDestReg, 8, true);

    LQR = MI->getParent()->computeRegisterLiveness(&TII->getRegisterInfo(),
                                                   UpperByteReg, MI);
    if (LQR != MachineBasicBlock::LQR_Dead)
      return false;
  }

  return true;
}

MachineInstr *FixupBWInstPass::tryReplaceLoad(unsigned New32BitOpcode,
                                              unsigned OrigDestSize,
                                              MachineInstr *MI) const {
  unsigned NewDestReg;

  if (!getSuperRegDestIfDead(MI, OrigDestSize, NewDestReg))
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
  // This algorithm always creates a replacement instruction
  // and notes that and the original in a data structure, until the
  // whole BB has been analyzed.  This keeps the replacement instructions
  // from making it seem as if the larger register might be live.
  //
  SmallVector<std::pair<MachineInstr *, MachineInstr *>, 8> MIReplacements;

  for (MachineBasicBlock::iterator I = MBB.begin(); I != MBB.end(); ++I) {
    MachineInstr *NewMI = nullptr;
    MachineInstr *MI = I;

    // See if this is an instruction of the type we are currently looking for.
    switch (MI->getOpcode()) {

    case X86::MOV8rm:
      // Only replace 8 bit loads with the zero extending versions if
      // in an inner most loop and not optimizing for size. This takes
      // an extra byte to encode, and provides limited performance upside.
      if (MachineLoop *ML = MLI->getLoopFor(&MBB)) {
        if (ML->begin() == ML->end() && !OptForSize)
          NewMI = tryReplaceLoad(X86::MOVZX32rm8, 8, MI);
      }  
      break;

    case X86::MOV16rm:
      // Always try to replace 16 bit load with 32 bit zero extending.
      // Code size is the same, and there is sometimes a perf advantage
      // from eliminating a false dependence on the upper portion of
      // the register.
      NewMI = tryReplaceLoad(X86::MOVZX32rm16, 16, MI);
      break;

    default:
      // nothing to do here.
      break;
    }

    if (NewMI)
      MIReplacements.push_back(std::make_pair(MI, NewMI));
  }

  while (!MIReplacements.empty()) {
    MachineInstr *MI = MIReplacements.back().first;
    MachineInstr *NewMI = MIReplacements.back().second;
    MIReplacements.pop_back();
    MBB.insert(MI, NewMI);
    MBB.erase(MI);
  }
}
