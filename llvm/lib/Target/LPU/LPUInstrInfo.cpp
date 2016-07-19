//===-- LPUInstrInfo.cpp - LPU Instruction Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the LPU implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "LPUInstrInfo.h"
#include "LPU.h"
#include "LPUMachineFunctionInfo.h"
#include "LPUTargetMachine.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "LPUGenInstrInfo.inc"

// GetCondFromBranchOpc - Return the LPU CC that matches
// the correspondent Branch instruction opcode.
static LPU::CondCode GetCondFromBranchOpc(unsigned BrOpc)
{
  switch (BrOpc) {
  default: return LPU::COND_INVALID;
  case LPU::BT  : return LPU::COND_T;
  case LPU::BF  : return LPU::COND_F;
  }
}

// GetCondBranchFromCond - Return the Branch instruction
// opcode that matches the cc.
static unsigned GetCondBranchFromCond(LPU::CondCode CC)
{
  switch (CC) {
  default: llvm_unreachable("Illegal condition code!");
  case LPU::COND_T  : return LPU::BT;
  case LPU::COND_F  : return LPU::BF;
  }
}

// GetOppositeBranchCondition - Return the inverse of the specified condition
static LPU::CondCode GetOppositeBranchCondition(LPU::CondCode CC)
{
  switch (CC) {
  default: llvm_unreachable("Illegal condition code!");
  case LPU::COND_T  : return LPU::COND_F;
  case LPU::COND_F  : return LPU::COND_T;
  }
}

// Pin the vtable to this file.
void LPUInstrInfo::anchor() {}

LPUInstrInfo::LPUInstrInfo(LPUSubtarget &STI)
  : LPUGenInstrInfo(LPU::ADJCALLSTACKDOWN, LPU::ADJCALLSTACKUP),
    RI(*this) {}

void LPUInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I, DebugLoc DL,
                                  unsigned DestReg, unsigned SrcReg,
                                  bool KillSrc) const {
  // This could determine the opcode based on the minimum size of the source
  // and destination
  // For now, just use MOV64 to make sure all bits are moved.
  // Ideally, this would be based on the actual bits in the value...
  unsigned Opc = LPU::MOV64;

  BuildMI(MBB, I, DL, get(Opc), DestReg)
    .addReg(SrcReg, getKillRegState(KillSrc));
}


void LPUInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator MI,
                                          unsigned SrcReg, bool isKill, int FrameIdx,
                                          const TargetRegisterClass *RC,
                                          const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();
  unsigned opc;

  if        (RC == &LPU::I0RegClass  || RC == &LPU::RI0RegClass  || RC == &LPU::CI0RegClass ||
             RC == &LPU::I1RegClass  || RC == &LPU::RI1RegClass  || RC == &LPU::CI1RegClass ||
             RC == &LPU::I8RegClass  || RC == &LPU::RI8RegClass  || RC == &LPU::CI8RegClass) {
    opc = LPU::ST8D;
  } else if (RC == &LPU::I16RegClass || RC == &LPU::RI16RegClass || RC == &LPU::CI16RegClass) {
    opc = LPU::ST16D;
  } else if (RC == &LPU::I32RegClass || RC == &LPU::RI32RegClass || RC == &LPU::CI32RegClass) {
    opc = LPU::ST32D;
  } else if (RC == &LPU::I64RegClass || RC == &LPU::RI64RegClass || RC == &LPU::CI64RegClass) {
    opc = LPU::ST64D;
  } else {
    llvm_unreachable("Unknown register class");
  }

  BuildMI(MBB, MI, DL, get(opc)).addFrameIndex(FrameIdx).addImm(0)
    .addReg(SrcReg, getKillRegState(isKill));

}

void LPUInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                           MachineBasicBlock::iterator MI,
                                           unsigned DestReg, int FrameIdx,
                                           const TargetRegisterClass *RC,
                                           const TargetRegisterInfo *TRI) const{
  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();
  unsigned opc;

  if        (RC == &LPU::I0RegClass  || RC == &LPU::RI0RegClass  || RC == &LPU::CI0RegClass ||
             RC == &LPU::I1RegClass  || RC == &LPU::RI1RegClass  || RC == &LPU::CI1RegClass ||
             RC == &LPU::I8RegClass  || RC == &LPU::RI8RegClass  || RC == &LPU::CI8RegClass) {
    opc = LPU::LD8D;
  } else if (RC == &LPU::I16RegClass || RC == &LPU::RI16RegClass || RC == &LPU::CI16RegClass) {
    opc = LPU::LD16D;
  } else if (RC == &LPU::I32RegClass || RC == &LPU::RI32RegClass || RC == &LPU::CI32RegClass) {
    opc = LPU::LD32D;
  } else if (RC == &LPU::I64RegClass || RC == &LPU::RI64RegClass || RC == &LPU::CI64RegClass) {
    opc = LPU::LD64D;
  } else {
    llvm_unreachable("Unknown register class");
  }

  BuildMI(MBB, MI, DL, get(opc), DestReg).addFrameIndex(FrameIdx).addImm(0);
}

unsigned LPUInstrInfo::RemoveBranch(MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;

  while (I != MBB.begin()) {
    --I;
    if (I->isDebugValue())
      continue;
    if (I->getOpcode() != LPU::BR &&
        I->getOpcode() != LPU::BT &&
        I->getOpcode() != LPU::BF)
      break;
    // Remove the branch.
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }

  return Count;
}

bool LPUInstrInfo::
ReverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 2 && "Invalid branch condition!");
  Cond[0].setImm(GetOppositeBranchCondition((LPU::CondCode)Cond[0].getImm()));
  return false;
}
/*
bool LPUInstrInfo::isUnpredicatedTerminator(const MachineInstr *MI) const {
  if (!MI->isTerminator()) return false;

  // Conditional branch is a special case.
  if (MI->isBranch() && !MI->isBarrier())
    return true;
  if (!MI->isPredicable())
    return true;
  return !isPredicated(MI);
}
*/
bool LPUInstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *&TBB,
                                    MachineBasicBlock *&FBB,
                                    SmallVectorImpl<MachineOperand> &Cond,
                                    bool AllowModify) const {
  // Start from the bottom of the block and work up, examining the
  // terminator instructions.
  MachineBasicBlock::iterator I = MBB.end();
  while (I != MBB.begin()) {
    --I;
    if (I->isDebugValue())
      continue;

    // Working from the bottom, when we see a non-terminator
    // instruction, we're done.
    if (!isUnpredicatedTerminator(I))
      break;

    // A terminator that isn't a branch can't easily be handled
    // by this analysis.
    if (!I->isBranch())
      return true;

    // Handle unconditional branches.
    if (I->getOpcode() == LPU::BR) {
      if (!AllowModify) {
        TBB = I->getOperand(0).getMBB();
        continue;
      }

      // If the block has any instructions after a BR, delete them.
      while (std::next(I) != MBB.end())
        std::next(I)->eraseFromParent();
      Cond.clear();
      FBB = nullptr;

      // Delete the BR if it's equivalent to a fall-through.
      if (MBB.isLayoutSuccessor(I->getOperand(0).getMBB())) {
        TBB = nullptr;
        I->eraseFromParent();
        I = MBB.end();
        continue;
      }

      // TBB is used to indicate the unconditinal destination.
      TBB = I->getOperand(0).getMBB();
      continue;
    }

    // Handle conditional branches.  This filters indirect jumps
    LPU::CondCode BranchCode = GetCondFromBranchOpc(I->getOpcode());
    if (BranchCode == LPU::COND_INVALID)
      return true;  // Can't handle weird stuff.

    // Working from the bottom, handle the first conditional branch.
    if (Cond.empty()) {
      FBB = TBB;
      TBB = I->getOperand(1).getMBB();
      Cond.push_back(MachineOperand::CreateImm(BranchCode));
      Cond.push_back(I->getOperand(0));
      continue;
    }

    // Handle subsequent conditional branches. Only handle the case where all
    // conditional branches branch to the same destination.
    assert(Cond.size() == 2);
    assert(TBB);

    // Only handle the case where all conditional branches branch to
    // the same destination.
    if (TBB != I->getOperand(1).getMBB())
      return true;

    LPU::CondCode OldBranchCode = (LPU::CondCode)Cond[0].getImm();
    // If the conditions are the same, we can leave them alone.
    if (OldBranchCode == BranchCode)
      continue;

    return true;
  }

  return false;
}

unsigned
LPUInstrInfo::InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                              MachineBasicBlock *FBB,
                              const SmallVectorImpl<MachineOperand> &Cond,
                              DebugLoc DL) const {
  // Shouldn't be a fall through.
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 2 || Cond.size() == 0) &&
         "LPU branch conditions have two components!");

  if (FBB == 0) { // One way branch.
    if (Cond.empty()) {
      // Unconditional branch?
      BuildMI(&MBB, DL, get(LPU::BR)).addMBB(TBB);
    } else {
      // Conditional branch.
      unsigned Opc = GetCondBranchFromCond((LPU::CondCode)Cond[0].getImm());
      BuildMI(&MBB, DL, get(Opc)).addReg(Cond[1].getReg()).addMBB(TBB);
    }
    return 1;
  }

  // Two-way Conditional branch.
  unsigned Opc = GetCondBranchFromCond((LPU::CondCode)Cond[0].getImm());
  BuildMI(&MBB, DL, get(Opc)).addReg(Cond[1].getReg()).addMBB(TBB);
  BuildMI(&MBB, DL, get(LPU::BR)).addMBB(FBB);

  return 2;
}
/*
/// GetInstSize - Return the number of bytes of code the specified
/// instruction may be.  This returns the maximum number of bytes.
///
unsigned LPUInstrInfo::GetInstSizeInBytes(const MachineInstr *MI) const {
  const MCInstrDesc &Desc = MI->getDesc();

  switch (Desc.TSFlags & LPUII::SizeMask) {
  default:
    switch (Desc.getOpcode()) {
    default: llvm_unreachable("Unknown instruction size!");
    case TargetOpcode::CFI_INSTRUCTION:
    case TargetOpcode::EH_LABEL:
    case TargetOpcode::IMPLICIT_DEF:
    case TargetOpcode::KILL:
    case TargetOpcode::DBG_VALUE:
      return 0;
    case TargetOpcode::INLINEASM: {
      const MachineFunction *MF = MI->getParent()->getParent();
      const TargetInstrInfo &TII = *MF->getSubtarget().getInstrInfo();
      return TII.getInlineAsmLength(MI->getOperand(0).getSymbolName(),
                                    *MF->getTarget().getMCAsmInfo());
    }
    }
  case LPUII::SizeSpecial:
    switch (MI->getOpcode()) {
    default: llvm_unreachable("Unknown instruction size!");
    case LPU::SAR8r1c:
    case LPU::SAR16r1c:
      return 4;
    }
  case LPUII::Size2Bytes:
    return 2;
  case LPUII::Size4Bytes:
    return 4;
  case LPUII::Size6Bytes:
    return 6;
  }
}
*/
unsigned 
LPUInstrInfo::getPickSwitchOpcode(const TargetRegisterClass *RC, 
				    bool isPick) const {

  if (isPick) {
    switch (RC->getID()) {
    default: llvm_unreachable("Unknown Target register class!");
    case LPU::I1RegClassID: return LPU::PICK1;
    case LPU::I8RegClassID: return LPU::PICK8;
    case LPU::I16RegClassID: return LPU::PICK16;
    case LPU::I32RegClassID: return LPU::PICK32;
    case LPU::I64RegClassID: return LPU::PICK64;
    case LPU::RI1RegClassID: return LPU::PICK1;
    case LPU::RI8RegClassID: return LPU::PICK8;
    case LPU::RI16RegClassID: return LPU::PICK16;
    case LPU::RI32RegClassID: return LPU::PICK32;
    case LPU::RI64RegClassID: return LPU::PICK64;
    }
  }


  switch (RC->getID()) {
  default: llvm_unreachable("Unknown Target register class!");
  case LPU::I1RegClassID: return LPU::SWITCH1;
  case LPU::I8RegClassID: return LPU::SWITCH8;
  case LPU::I16RegClassID: return LPU::SWITCH16;
  case LPU::I32RegClassID: return LPU::SWITCH32;
  case LPU::I64RegClassID: return LPU::SWITCH64;

  case LPU::RI1RegClassID: return LPU::SWITCH1;
  case LPU::RI8RegClassID: return LPU::SWITCH8;
  case LPU::RI16RegClassID: return LPU::SWITCH16;
  case LPU::RI32RegClassID: return LPU::SWITCH32;
  case LPU::RI64RegClassID: return LPU::SWITCH64;
  }

}


// Convert opcode of LD/ST into a corresponding opcode for OLD/OST.
// Returns current_opcode if it is not a LD or ST.
unsigned
LPUInstrInfo::get_ordered_opcode_for_LDST(unsigned current_opcode)  const {
  // HACK: this code relies on the (possibly fragile) assumption that
  // (a) we have exactly one ordered store operation for every normal
  // store opcode, and (b) that the difference between the ordered and
  // non-ordered version of these opcodes is always a constant.
  //
  // If either of these conditions are violated, e.g., because there
  // is some other new instruction that starts with OLD or OST, then
  // bad things will happen...
  if ((current_opcode >= LPU::ST1) &&  (current_opcode <= LPU::ST8i)) {
    return current_opcode + (LPU::OST1 - LPU::ST1);
  }
  else if ((current_opcode >= LPU::LD1) && (current_opcode <= LPU::LD8X)) {
    // Analogous hack for LD operations. 
    return current_opcode + (LPU::OLD1 - LPU::LD1);
  }
  else {
    return current_opcode;
  }
}


bool LPUInstrInfo::isLoad(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::LD1 && MI->getOpcode() <= LPU::LD8X;
}

bool LPUInstrInfo::isStore(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::ST1 && MI->getOpcode() <= LPU::ST8i;
}

bool LPUInstrInfo::isMul(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::MUL16 && MI->getOpcode() <= LPU::MULF64i;
}


bool LPUInstrInfo::isDiv(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::DIVF16 && MI->getOpcode() <= LPU::DIVU8i1;
}

bool LPUInstrInfo::isFMA(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::FMAF16 && MI->getOpcode() <= LPU::FMAF64xi;
}

bool LPUInstrInfo::isAdd(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::ADD16 && MI->getOpcode() <= LPU::ADDF64i;
}

bool LPUInstrInfo::isSub(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::SUB16 && MI->getOpcode() <= LPU::SUBF64i1;
}

bool LPUInstrInfo::isShift(MachineInstr *MI) const {
        return (MI->getOpcode() >= LPU::SLL16 && MI->getOpcode() <= LPU::SLL8i1) ||
          (MI->getOpcode() >= LPU::SRA16 && MI->getOpcode() <= LPU::SRL8i1);
}

bool LPUInstrInfo::isCmp(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::CMPEQ1 && MI->getOpcode() <= LPU::CMPUOF64i;
}

bool LPUInstrInfo::isSwitch(MachineInstr *MI) const {
	return MI->getOpcode() == LPU::SWITCH1 ||
         MI->getOpcode() == LPU::SWITCH8 ||
         MI->getOpcode() == LPU::SWITCH16 ||
         MI->getOpcode() == LPU::SWITCH32 ||
         MI->getOpcode() == LPU::SWITCH64;
}

bool LPUInstrInfo::isPick(MachineInstr *MI) const {
        return MI->getOpcode() == LPU::PICK1 ||
		MI->getOpcode() == LPU::PICK8 ||
		MI->getOpcode() == LPU::PICK16 ||
		MI->getOpcode() == LPU::PICK32 ||
		MI->getOpcode() == LPU::PICK64;
}

bool LPUInstrInfo::isCopy(MachineInstr *MI) const {
  return MI->getOpcode() == LPU::COPY1 ||
    MI->getOpcode() == LPU::COPY8 ||
    MI->getOpcode() == LPU::COPY16 ||
    MI->getOpcode() == LPU::COPY32 ||
    MI->getOpcode() == LPU::COPY64;
}

bool LPUInstrInfo::isMOV(MachineInstr *MI) const {
  return MI->getOpcode() == LPU::MOV1 ||
    MI->getOpcode() == LPU::MOV8 ||
    MI->getOpcode() == LPU::MOV16 ||
    MI->getOpcode() == LPU::MOV32 ||
    MI->getOpcode() == LPU::MOV64;
}


bool LPUInstrInfo::isInit(MachineInstr *MI) const {
	return MI->getOpcode() == LPU::INIT1 ||
		MI->getOpcode() == LPU::INIT8 ||
		MI->getOpcode() == LPU::INIT16 ||
		MI->getOpcode() == LPU::INIT32 ||
		MI->getOpcode() == LPU::INIT64;
}


unsigned
LPUInstrInfo::getCopyOpcode(const TargetRegisterClass *RC) const {
  if      (RC == &LPU::I1RegClass)  return LPU::COPY1;
  else if (RC == &LPU::I8RegClass)  return LPU::COPY8;
  else if (RC == &LPU::I16RegClass) return LPU::COPY16;
  else if (RC == &LPU::I32RegClass) return LPU::COPY32;
  else if (RC == &LPU::I64RegClass) return LPU::COPY64;
  else
    llvm_unreachable("Unknown Target LIC class!");
}

unsigned
LPUInstrInfo::getMoveOpcode(const TargetRegisterClass *RC) const {
  if (RC == &LPU::I1RegClass)  return LPU::MOV1;
  else if (RC == &LPU::I8RegClass)  return LPU::MOV8;
  else if (RC == &LPU::I16RegClass) return LPU::MOV16;
  else if (RC == &LPU::I32RegClass) return LPU::MOV32;
  else if (RC == &LPU::I64RegClass) return LPU::MOV64;
  else
    llvm_unreachable("Unknown Target LIC class!");
}

unsigned
LPUInstrInfo::getInitOpcode(const TargetRegisterClass *RC) const {
  if (RC == &LPU::I1RegClass)  return LPU::INIT1;
  else if (RC == &LPU::I8RegClass)  return LPU::INIT8;
  else if (RC == &LPU::I16RegClass) return LPU::INIT16;
  else if (RC == &LPU::I32RegClass) return LPU::INIT32;
  else if (RC == &LPU::I64RegClass) return LPU::INIT64;
  else
    llvm_unreachable("Unknown Target LIC class!");
}
