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
#include "llvm/ADT/StringRef.h"

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
                                  MachineBasicBlock::iterator I,
                                  const DebugLoc &DL, unsigned DestReg,
                                  unsigned SrcReg, bool KillSrc) const {
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

unsigned LPUInstrInfo::removeBranch(MachineBasicBlock &MBB, int *BytesAdded) const {
  assert(!BytesAdded && "code size not handled");

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
reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 2 && "Invalid branch condition!");
  Cond[0].setImm(GetOppositeBranchCondition((LPU::CondCode)Cond[0].getImm()));
  return false;
}
/*
bool LPUInstrInfo::isUnpredicatedTerminator(const MachineInstr &MI) const {
  if (!MI.isTerminator()) return false;

  // Conditional branch is a special case.
  if (MI.isBranch() && !MI.isBarrier())
    return true;
  if (!MI.isPredicable())
    return true;
  return !isPredicated(MI);
}
*/
bool LPUInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
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
    if (!isUnpredicatedTerminator(*I))
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
LPUInstrInfo::insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                              MachineBasicBlock *FBB,
                              ArrayRef<MachineOperand> Cond,
                              const DebugLoc &DL,
                              int *BytesAdded) const {
  assert(!BytesAdded && "code size not handled");
  // Shouldn't be a fall through.
  assert(TBB && "insertBranch must not be told to insert a fallthrough");
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

// This should probably be set up differently:
// Have a generic operation enum (e.g. PICK, SWITCH, CMPLEU, ADD, ... etc.)
// Then have
//  genop = genericOpcodeFromSpecific(opcode)
//  VT = typeFromOpcode(opcode) - primary result type
// and:
// plus things like:
//  opc = selectSeq(addOp, cmpOp)  // return a sequence based on add+

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

// Convert opcode of LD/ST/ATM* into a corresponding opcode for OLD/OST/OATM*.
// Returns current_opcode if it is not a LD, ST, or ATM*.
unsigned
LPUInstrInfo::get_ordered_opcode_for_LDST(unsigned current_opcode)  const {
  // This is still a bit of a hack, and certainly slower than before, but it
  // should be less fragile. This depends on the opcode names for conversion to
  // ordered opcodes. Specifically, if the unordered opcode is named "BAR",
  // then the ordered opcode must be that which is named exactly "OBAR".
  StringRef current = getName(current_opcode);
  unsigned new_op;
  bool found = false;
  for(new_op=0; new_op<getNumOpcodes(); ++new_op) {
      StringRef candidate = StringRef(getName(new_op));
      if(candidate.startswith("O") && candidate.endswith(current)
              && candidate.size() == current.size()+1){
          found = true;
          break;
      }
  }

  if(found) {
      return new_op;
  }

  return current_opcode;
}


bool LPUInstrInfo::isLoad(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::LD1 && MI->getOpcode() <= LPU::LD8X;
}

bool LPUInstrInfo::isStore(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::ST1 && MI->getOpcode() <= LPU::ST8X;
}

bool LPUInstrInfo::isOrderedLoad(MachineInstr *MI) const {
  return ((MI->getOpcode() >= LPU::OLD1) && (MI->getOpcode() <= LPU::OLD8X));
}

bool LPUInstrInfo::isOrderedStore(MachineInstr *MI) const {
  return ((MI->getOpcode() >= LPU::OST1) && (MI->getOpcode() <= LPU::OST8X));
}

bool LPUInstrInfo::isMul(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::MUL16 && MI->getOpcode() <= LPU::MULF64;
}


bool LPUInstrInfo::isDiv(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::DIVF16 && MI->getOpcode() <= LPU::DIVU8;
}

bool LPUInstrInfo::isFMA(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::FMAF16 && MI->getOpcode() <= LPU::FMAF64;
}

bool LPUInstrInfo::isAdd(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::ADD16 && MI->getOpcode() <= LPU::ADDF64;
}

bool LPUInstrInfo::isSub(MachineInstr *MI) const {
	return MI->getOpcode() >= LPU::SUB16 && MI->getOpcode() <= LPU::SUBF64;
}

bool LPUInstrInfo::isShift(MachineInstr *MI) const {
        return (MI->getOpcode() >= LPU::SLL16 && MI->getOpcode() <= LPU::SLL8) ||
          (MI->getOpcode() >= LPU::SRA16 && MI->getOpcode() <= LPU::SRL8);
}

bool LPUInstrInfo::isCmp(MachineInstr *MI) const {
    return ((MI->getOpcode() >= LPU::CMPEQ16) &&
            (MI->getOpcode() <= LPU::CMPUOF64));
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
  return
    MI->getOpcode() == LPU::COPY0 ||
    MI->getOpcode() == LPU::COPY1 ||
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

bool LPUInstrInfo::isAtomic(MachineInstr *MI) const {
    return MI->getOpcode() >= LPU::ATMADD16 && MI->getOpcode() <= LPU::ATMXOR8;
}

bool LPUInstrInfo::isOrderedAtomic(MachineInstr *MI) const {
    return MI->getOpcode() >= LPU::OATMADD16 && MI->getOpcode() <= LPU::OATMXOR8;
}

bool LPUInstrInfo::isSeqOT(MachineInstr *MI) const {
    return ((MI->getOpcode() >= LPU::SEQOTGE) &&
            (MI->getOpcode() <= LPU::SEQOTNE8));
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
  if (RC == &LPU::I1RegClass || RC == &LPU::CI1RegClass || RC == &LPU::RI1RegClass)  return LPU::MOV1;
  else if (RC == &LPU::I8RegClass || RC == &LPU::CI8RegClass || RC == &LPU::RI8RegClass)  return LPU::MOV8;
  else if (RC == &LPU::I16RegClass || RC == &LPU::CI16RegClass || RC == &LPU::RI16RegClass) return LPU::MOV16;
  else if (RC == &LPU::I32RegClass || RC == &LPU::CI32RegClass || RC == &LPU::RI32RegClass) return LPU::MOV32;
  else if (RC == &LPU::I64RegClass || RC == &LPU::CI64RegClass || RC == &LPU::RI64RegClass) return LPU::MOV64;
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



// TBD(jsukha): This table lookup works for now, but there must be a
// better way to implement this matching operation...
unsigned LPUInstrInfo::commuteNegateCompareOpcode(unsigned cmp_opcode,
                                                  bool commute_compare_operands,
                                                  bool negate_eq) const {

    // We need to a switch a "<" to a ">=" if we swap the operands,
    // and if we negate the output.  If we do both, then they cancel
    // each other out.
    bool swap_ltgt = commute_compare_operands ^ negate_eq;
      
    switch (cmp_opcode) {
      // == maps to == (self)
    case LPU::CMPEQ8:
      return negate_eq ? LPU::CMPNE8 :  LPU::CMPEQ8;
    case LPU::CMPEQ16:
      return negate_eq ? LPU::CMPNE16 :  LPU::CMPEQ16;      
    case LPU::CMPEQ32:
      return negate_eq ? LPU::CMPNE32 :  LPU::CMPEQ32;            
    case LPU::CMPEQ64:
      return negate_eq ? LPU::CMPNE64 :  LPU::CMPEQ64;
      
    // ">=" maps to "<"
    case LPU::CMPGES8:
      return swap_ltgt ? LPU::CMPLTS8 : LPU::CMPGES8;
    case LPU::CMPGES16:
      return swap_ltgt ? LPU::CMPLTS16 : LPU::CMPGES16;      
    case LPU::CMPGES32:
      return swap_ltgt ? LPU::CMPLTS32 : LPU::CMPGES32;      
    case LPU::CMPGES64:
      return swap_ltgt ? LPU::CMPLTS64 : LPU::CMPGES64;      
    case LPU::CMPGEU8:
      return swap_ltgt ? LPU::CMPLTU8 : LPU::CMPGEU8;
    case LPU::CMPGEU16:
      return swap_ltgt ? LPU::CMPLTU16 : LPU::CMPGEU16;      
    case LPU::CMPGEU32:
      return swap_ltgt ? LPU::CMPLTU32 : LPU::CMPGEU32;
    case LPU::CMPGEU64:
      return swap_ltgt ? LPU::CMPLTU64 : LPU::CMPGEU64;

    // ">" maps to "<="
    case LPU::CMPGTS8:
      return swap_ltgt ? LPU::CMPLES8 : LPU::CMPGTS8;
    case LPU::CMPGTS16:
      return swap_ltgt ? LPU::CMPLES16 : LPU::CMPGTS16;      
    case LPU::CMPGTS32:
      return swap_ltgt ? LPU::CMPLES32 : LPU::CMPGTS32;      
    case LPU::CMPGTS64:
      return swap_ltgt ? LPU::CMPLES64 : LPU::CMPGTS64;      
    case LPU::CMPGTU8:
      return swap_ltgt ? LPU::CMPLEU8 : LPU::CMPGTU8;
    case LPU::CMPGTU16:
      return swap_ltgt ? LPU::CMPLEU16 : LPU::CMPGTU16;      
    case LPU::CMPGTU32:
      return swap_ltgt ? LPU::CMPLEU32 : LPU::CMPGTU32;      
    case LPU::CMPGTU64:
      return swap_ltgt ? LPU::CMPLEU64 : LPU::CMPGTU64;      

    // "<=" maps to ">"
    case LPU::CMPLES8:
      return swap_ltgt ? LPU::CMPGTS8 : LPU::CMPLES8;
    case LPU::CMPLES16:
      return swap_ltgt ? LPU::CMPGTS16 : LPU::CMPLES16;      
    case LPU::CMPLES32:
      return swap_ltgt ? LPU::CMPGTS32 : LPU::CMPLES32;      
    case LPU::CMPLES64:
      return swap_ltgt ? LPU::CMPGTS64 : LPU::CMPLES64;      
    case LPU::CMPLEU8:
      return swap_ltgt ? LPU::CMPGTU8 : LPU::CMPLEU8;
    case LPU::CMPLEU16:
      return swap_ltgt ? LPU::CMPGTU16 : LPU::CMPLEU16;
    case LPU::CMPLEU32:
      return swap_ltgt ? LPU::CMPGTU32 : LPU::CMPLEU32;      
    case LPU::CMPLEU64:
      return swap_ltgt ? LPU::CMPGTU64 : LPU::CMPLEU64;            

    // "<" maps to ">="
    case LPU::CMPLTS8:
      return swap_ltgt ? LPU::CMPGES8 : LPU::CMPLTS8;
    case LPU::CMPLTS16:
      return swap_ltgt ? LPU::CMPGES16 : LPU::CMPLTS16;      
    case LPU::CMPLTS32:
      return swap_ltgt ? LPU::CMPGES32 : LPU::CMPLTS32;      
    case LPU::CMPLTS64:
      return swap_ltgt ? LPU::CMPGES64 : LPU::CMPLTS64;      
    case LPU::CMPLTU8:
      return swap_ltgt ? LPU::CMPGEU8 : LPU::CMPLTU8;
    case LPU::CMPLTU16:
      return swap_ltgt ? LPU::CMPGEU16 : LPU::CMPLTU16;
    case LPU::CMPLTU32:
      return swap_ltgt ? LPU::CMPGEU32 : LPU::CMPLTU32;      
    case LPU::CMPLTU64:
      return swap_ltgt ? LPU::CMPGEU64 : LPU::CMPLTU64;      

    // != maps to !=  (self)
    case LPU::CMPNE8:
      return negate_eq ? LPU::CMPEQ8 : LPU::CMPNE8;
    case LPU::CMPNE16:
      return negate_eq ? LPU::CMPEQ16 : LPU::CMPNE16;      
    case LPU::CMPNE32:
      return negate_eq ? LPU::CMPEQ32 : LPU::CMPNE32;      
    case LPU::CMPNE64:
      return negate_eq ? LPU::CMPEQ64 : LPU::CMPNE64;      

    // Floating-point equal: maps to self. 
    // == maps to ==
    case LPU::CMPOEQF16:
      return negate_eq ? LPU::CMPONEF16 : LPU::CMPOEQF16;
    case LPU::CMPOEQF32:
      return negate_eq ? LPU::CMPONEF32 : LPU::CMPOEQF32;      
    case LPU::CMPOEQF64:
      return negate_eq ? LPU::CMPONEF64 : LPU::CMPOEQF64;
    case LPU::CMPUEQF16:
      return negate_eq ? LPU::CMPUNEF16 : LPU::CMPUEQF16;
    case LPU::CMPUEQF32:
      return negate_eq ? LPU::CMPUNEF32 : LPU::CMPUEQF32;      
    case LPU::CMPUEQF64:
      return negate_eq ? LPU::CMPUNEF64 : LPU::CMPUEQF64;      

    // >= to <
    case LPU::CMPOGEF16:
      return swap_ltgt ? LPU::CMPOLTF16 : LPU::CMPOGEF16;
    case LPU::CMPOGEF32:
      return swap_ltgt ? LPU::CMPOLTF32 : LPU::CMPOGEF32;      
    case LPU::CMPOGEF64:
      return swap_ltgt ? LPU::CMPOLTF64 : LPU::CMPOGEF64;      
    case LPU::CMPUGEF16:
      return swap_ltgt ? LPU::CMPULTF16 : LPU::CMPUGEF16;
    case LPU::CMPUGEF32:
      return swap_ltgt ? LPU::CMPULTF32 : LPU::CMPUGEF32;
    case LPU::CMPUGEF64:
      return swap_ltgt ? LPU::CMPULTF64 : LPU::CMPUGEF64;      
      
    // > to <=
    case LPU::CMPOGTF16:
      return swap_ltgt ? LPU::CMPOLEF16 : LPU::CMPOGTF16;
    case LPU::CMPOGTF32:
      return swap_ltgt ? LPU::CMPOLEF32 : LPU::CMPOGTF32;      
    case LPU::CMPOGTF64:
      return swap_ltgt ? LPU::CMPOLEF64 : LPU::CMPOGTF64;      
    case LPU::CMPUGTF16:
      return swap_ltgt ? LPU::CMPULEF16 : LPU::CMPUGTF16;
    case LPU::CMPUGTF32:
      return swap_ltgt ? LPU::CMPULEF32 : LPU::CMPUGTF32;      
    case LPU::CMPUGTF64:
      return swap_ltgt ? LPU::CMPULEF64 : LPU::CMPUGTF64;      
      
    // <= to >
    case LPU::CMPOLEF16:
      return swap_ltgt ? LPU::CMPOGTF16 : LPU::CMPOLEF16;
    case LPU::CMPOLEF32:
      return swap_ltgt ? LPU::CMPOGTF32 : LPU::CMPOLEF32;
    case LPU::CMPOLEF64:
      return swap_ltgt ? LPU::CMPOGTF64 : LPU::CMPOLEF64;      
    case LPU::CMPULEF16:
      return swap_ltgt ? LPU::CMPUGTF16 : LPU::CMPULEF16;
    case LPU::CMPULEF32:
      return swap_ltgt ? LPU::CMPUGTF32 : LPU::CMPULEF32;      
    case LPU::CMPULEF64:
      return swap_ltgt ? LPU::CMPUGTF64 : LPU::CMPULEF64;            
      
    // < to >=
    case LPU::CMPOLTF16:
      return swap_ltgt ? LPU::CMPOGEF16 : LPU::CMPOLTF16;
    case LPU::CMPOLTF32:
      return swap_ltgt ? LPU::CMPOGEF32 : LPU::CMPOLTF32;
    case LPU::CMPOLTF64:
      return swap_ltgt ? LPU::CMPOGEF64 : LPU::CMPOLTF64;      
    case LPU::CMPULTF16:
      return swap_ltgt ? LPU::CMPUGEF16 : LPU::CMPULTF16;
    case LPU::CMPULTF32:
      return swap_ltgt ? LPU::CMPUGEF32 : LPU::CMPULTF32;      
    case LPU::CMPULTF64:
      return swap_ltgt ? LPU::CMPUGEF64 : LPU::CMPULTF64;            

    // != maps to !=
    case LPU::CMPONEF16:
      return negate_eq ? LPU::CMPOEQF16 : LPU::CMPONEF16;
    case LPU::CMPONEF32:
      return negate_eq ? LPU::CMPOEQF32 : LPU::CMPONEF32;
    case LPU::CMPONEF64:
      return negate_eq ? LPU::CMPOEQF64 : LPU::CMPONEF64;
    case LPU::CMPUNEF16:
      return negate_eq ? LPU::CMPUEQF16 : LPU::CMPUNEF16;
    case LPU::CMPUNEF32:
      return negate_eq ? LPU::CMPUEQF32 : LPU::CMPUNEF32;      
    case LPU::CMPUNEF64:
      return negate_eq ? LPU::CMPUEQF64 : LPU::CMPUNEF64;      

    // Die by default.  We should never call this method on any opcode
    // which is not a compare.
    default:
      assert(0);
      return cmp_opcode;
    }
}


unsigned LPUInstrInfo::
convertCompareOpToSeqOTOp(unsigned cmp_opcode) const {
    switch (cmp_opcode) {
    // ">="
    case LPU::CMPGES8:
      return LPU::SEQOTGES8;
    case LPU::CMPGES16:
      return LPU::SEQOTGES16;
    case LPU::CMPGES32:
      return LPU::SEQOTGES32;
    case LPU::CMPGES64:
      return LPU::SEQOTGES64;
    case LPU::CMPGEU8:
      return LPU::SEQOTGEU8;
    case LPU::CMPGEU16:
      return LPU::SEQOTGEU16;
    case LPU::CMPGEU32:
      return LPU::SEQOTGEU32;
    case LPU::CMPGEU64:
      return LPU::SEQOTGEU64;

    // ">" 
    case LPU::CMPGTS8:
      return LPU::SEQOTGTS8;
    case LPU::CMPGTS16:
      return LPU::SEQOTGTS16;
    case LPU::CMPGTS32:
      return LPU::SEQOTGTS32;
    case LPU::CMPGTS64:
      return LPU::SEQOTGTS64;
    case LPU::CMPGTU8:
      return LPU::SEQOTGTU8;
    case LPU::CMPGTU16:
      return LPU::SEQOTGTU16;
    case LPU::CMPGTU32:
      return LPU::SEQOTGTU32;
    case LPU::CMPGTU64:
      return LPU::SEQOTGTU64;

    // "<=" 
    case LPU::CMPLES8:
      return LPU::SEQOTLES8;
    case LPU::CMPLES16:
      return LPU::SEQOTLES16;
    case LPU::CMPLES32:
      return LPU::SEQOTLES32;
    case LPU::CMPLES64:
      return LPU::SEQOTLES64;
    case LPU::CMPLEU8:
      return LPU::SEQOTLEU8;
    case LPU::CMPLEU16:
      return LPU::SEQOTLEU16;
    case LPU::CMPLEU32:
      return LPU::SEQOTLEU32;
    case LPU::CMPLEU64:
      return LPU::SEQOTLEU64;

    // "<" 
    case LPU::CMPLTS8:
      return LPU::SEQOTLTS8;
    case LPU::CMPLTS16:
      return LPU::SEQOTLTS16;
    case LPU::CMPLTS32:
      return LPU::SEQOTLTS32;
    case LPU::CMPLTS64:
      return LPU::SEQOTLTS64;
    case LPU::CMPLTU8:
      return LPU::SEQOTLTU8;
    case LPU::CMPLTU16:
      return LPU::SEQOTLTU16;
    case LPU::CMPLTU32:
      return LPU::SEQOTLTU32;
    case LPU::CMPLTU64:
      return LPU::SEQOTLTU64;

    // !=
    case LPU::CMPNE8:
      return LPU::SEQOTNE8;
    case LPU::CMPNE16:
      return LPU::SEQOTNE16;
    case LPU::CMPNE32:
      return LPU::SEQOTNE32;
    case LPU::CMPNE64:
      return LPU::SEQOTNE64;
      

    // By default, return the same opcode. 
    default:
      return cmp_opcode;
    }
}


unsigned LPUInstrInfo::
promoteSeqOTOpBitwidth(unsigned seq_opcode,
                       int bitwidth) const {
    switch (seq_opcode) {
      // This code is relying on the fall-through of switch, to end up
      // picking the smallest size that is both larger than the size
      // specified in seq_opcode and >= bitwidth.

    //">="
    case LPU::SEQOTGES8:
      if (bitwidth <= 8) { return LPU::SEQOTGES8; }
    case LPU::SEQOTGES16:
      if (bitwidth <= 16) { return LPU::SEQOTGES16; }      
    case LPU::SEQOTGES32:
      if (bitwidth <= 32) { return LPU::SEQOTGES32; }            
    case LPU::SEQOTGES64:
      return LPU::SEQOTGES64;

    case LPU::SEQOTGEU8:
      if (bitwidth <= 8) { return LPU::SEQOTGEU8; }
    case LPU::SEQOTGEU16:
      if (bitwidth <= 16) { return LPU::SEQOTGEU16; }      
    case LPU::SEQOTGEU32:
      if (bitwidth <= 32) { return LPU::SEQOTGEU32; }            
    case LPU::SEQOTGEU64:
      return LPU::SEQOTGEU64;

    // ">"
    case LPU::SEQOTGTS8:
      if (bitwidth <= 8) { return LPU::SEQOTGTS8; }
    case LPU::SEQOTGTS16:
      if (bitwidth <= 16) { return LPU::SEQOTGTS16; }      
    case LPU::SEQOTGTS32:
      if (bitwidth <= 32) { return LPU::SEQOTGTS32; }            
    case LPU::SEQOTGTS64:
      return LPU::SEQOTGTS64;

    case LPU::SEQOTGTU8:
      if (bitwidth <= 8) { return LPU::SEQOTGTU8; }
    case LPU::SEQOTGTU16:
      if (bitwidth <= 16) { return LPU::SEQOTGTU16; }      
    case LPU::SEQOTGTU32:
      if (bitwidth <= 32) { return LPU::SEQOTGTU32; }            
    case LPU::SEQOTGTU64:
      return LPU::SEQOTGTU64;


    // "<="
    case LPU::SEQOTLES8:
      if (bitwidth <= 8) { return LPU::SEQOTLES8; }
    case LPU::SEQOTLES16:
      if (bitwidth <= 16) { return LPU::SEQOTLES16; }      
    case LPU::SEQOTLES32:
      if (bitwidth <= 32) { return LPU::SEQOTLES32; }            
    case LPU::SEQOTLES64:
      return LPU::SEQOTLES64;

    case LPU::SEQOTLEU8:
      if (bitwidth <= 8) { return LPU::SEQOTLEU8; }
    case LPU::SEQOTLEU16:
      if (bitwidth <= 16) { return LPU::SEQOTLEU16; }      
    case LPU::SEQOTLEU32:
      if (bitwidth <= 32) { return LPU::SEQOTLEU32; }            
    case LPU::SEQOTLEU64:
      return LPU::SEQOTLEU64;

    // "<"
    case LPU::SEQOTLTS8:
      if (bitwidth <= 8) { return LPU::SEQOTLTS8; }
    case LPU::SEQOTLTS16:
      if (bitwidth <= 16) { return LPU::SEQOTLTS16; }      
    case LPU::SEQOTLTS32:
      if (bitwidth <= 32) { return LPU::SEQOTLTS32; }            
    case LPU::SEQOTLTS64:
      return LPU::SEQOTLTS64;

    case LPU::SEQOTLTU8:
      if (bitwidth <= 8) { return LPU::SEQOTLTU8; }
    case LPU::SEQOTLTU16:
      if (bitwidth <= 16) { return LPU::SEQOTLTU16; }      
    case LPU::SEQOTLTU32:
      if (bitwidth <= 32) { return LPU::SEQOTLTU32; }            
    case LPU::SEQOTLTU64:
      return LPU::SEQOTLTU64;

    // !=
    case LPU::SEQOTNE8:
      if (bitwidth <= 8) { return LPU::SEQOTNE8; }
    case LPU::SEQOTNE16:
      if (bitwidth <= 16) { return LPU::SEQOTNE16; }      
    case LPU::SEQOTNE32:
      if (bitwidth <= 32) { return LPU::SEQOTNE32; }            
    case LPU::SEQOTNE64:
      return LPU::SEQOTNE64;
      
    // By default, return the same opcode. 
    default:
      return seq_opcode;
    }
}


bool
LPUInstrInfo::
convertAddToStrideOp(unsigned add_opcode,
                     unsigned* strideOpcode) const {
  switch (add_opcode) {
  case LPU::ADD64:
    *strideOpcode = LPU::STRIDE64;
    return true;

  case LPU::ADD32:
    *strideOpcode = LPU::STRIDE32;
    return true;
    
  case LPU::ADD16:
    *strideOpcode = LPU::STRIDE16;
    return true;

  case LPU::ADD8:
    *strideOpcode = LPU::STRIDE8;
    return true;
    
  default:
    // No match. return false. 
    return false;
  }
}

bool
LPUInstrInfo::
convertSubToStrideOp(unsigned sub_opcode,
                     unsigned* strideOpcode) const {
  switch (sub_opcode) {
  case LPU::SUB64:
    *strideOpcode = LPU::STRIDE64;
    return true;

  case LPU::SUB32:
    *strideOpcode = LPU::STRIDE32;
    return true;
    
  case LPU::SUB16:
    *strideOpcode = LPU::STRIDE16;
    return true;

  case LPU::SUB8:
    *strideOpcode = LPU::STRIDE8;
    return true;
    
  default:
    // No match. return false. 
    return false;
  }
}

bool
LPUInstrInfo::
negateOpForStride(unsigned strideOpcode,
                  unsigned* negOpcode) const {
  switch (strideOpcode) {
  case LPU::STRIDE64:
    *negOpcode = LPU::NEG64;
    return true;

  case LPU::STRIDE32:
    *negOpcode = LPU::NEG32;
    return true;
    
  case LPU::STRIDE16:
    *negOpcode = LPU::NEG16;
    return true;

  case LPU::STRIDE8:
    *negOpcode = LPU::NEG8;
    return true;

  default:
    // No match. return false. 
    return false;
  }
}

const TargetRegisterClass*
LPUInstrInfo::
getStrideInputRC(unsigned strideOpcode) const {
  switch (strideOpcode) {
  case LPU::STRIDE64:
    return &LPU::CI64RegClass;
  case LPU::STRIDE32:
    return &LPU::CI32RegClass;
  case LPU::STRIDE16:
    return &LPU::CI16RegClass;
  case LPU::STRIDE8:
    return &LPU::CI8RegClass;
  default:
    // No match. return false.
    return NULL;
  }
}


bool
LPUInstrInfo::
convertPickToRepeatOp(unsigned pick_opcode,
                      unsigned* repeat_opcode) const {
  switch (pick_opcode) {
  case LPU::PICK64:
    *repeat_opcode = LPU::REPEAT64;
    return true;

  case LPU::PICK32:
    *repeat_opcode = LPU::REPEAT32;
    return true;
    
  case LPU::PICK16:
    *repeat_opcode = LPU::REPEAT16;
    return true;

  case LPU::PICK8:
  case LPU::PICK1:
    // TBD(jsukha): We don't have a REPEAT1 statement, so just use the
    // REPEAT8 for now.
    *repeat_opcode = LPU::REPEAT8;
    return true;
    
  default:
    // No match. return false. 
    return false;
  }
}

bool
LPUInstrInfo::
isCommutingReductionTransform(MachineInstr* MI) const {
  unsigned opcode = MI->getOpcode();
  // NOTE: we are leaving out AND1, OR1, and XOR1.
  // We don't have a 1-bit reduction op...
  return (isAdd(MI) ||
          isMul(MI) ||
          ((LPU::AND16 <= opcode) && (opcode <= LPU::AND8)) ||
          ((LPU::OR16 <= opcode) && (opcode <= LPU::OR8)) ||
          ((LPU::XOR16 <= opcode) && (opcode <= LPU::XOR8)));
}


bool
LPUInstrInfo::
convertTransformToReductionOp(unsigned transform_opcode,
                              unsigned* reduction_opcode) const {
  switch (transform_opcode) {

  case LPU::FMAF64:
    *reduction_opcode = LPU::FMSREDAF64;
    return true;
  case LPU::FMAF32:
    *reduction_opcode = LPU::FMSREDAF32;
    return true;

  case LPU::ADDF64:
    *reduction_opcode = LPU::SREDADDF64;
    return true;
  case LPU::ADDF32:
    *reduction_opcode = LPU::SREDADDF32;
    return true;    
  case LPU::ADD64:
    *reduction_opcode = LPU::SREDADD64;
    return true;
  case LPU::ADD32:
    *reduction_opcode = LPU::SREDADD32;
    return true;
  case LPU::ADD16:
    *reduction_opcode = LPU::SREDADD16;
    return true;

  case LPU::SUBF64:
    *reduction_opcode = LPU::SREDSUBF64;
    return true;
  case LPU::SUBF32:
    *reduction_opcode = LPU::SREDSUBF32;
    return true;    
  case LPU::SUB64:
    *reduction_opcode = LPU::SREDSUB64;
    return true;
  case LPU::SUB32:
    *reduction_opcode = LPU::SREDSUB32;
    return true;
  case LPU::SUB16:
    *reduction_opcode = LPU::SREDSUB16;
    return true;

  case LPU::MULF64:
    *reduction_opcode = LPU::SREDMULF64;
    return true;
  case LPU::MULF32:
    *reduction_opcode = LPU::SREDMULF32;
    return true;    
  case LPU::MUL64:
    *reduction_opcode = LPU::SREDMUL64;
    return true;
  case LPU::MUL32:
    *reduction_opcode = LPU::SREDMUL32;
    return true;
  case LPU::MUL16:
    *reduction_opcode = LPU::SREDMUL16;
    return true;

  case LPU::AND64:
    *reduction_opcode = LPU::SREDAND64;
    return true;
  case LPU::AND32:
    *reduction_opcode = LPU::SREDAND32;
    return true;
  case LPU::AND16:
    *reduction_opcode = LPU::SREDAND16;
    return true;
  case LPU::AND8:
    *reduction_opcode = LPU::SREDAND8;
    return true;

  case LPU::OR64:
    *reduction_opcode = LPU::SREDOR64;
    return true;
  case LPU::OR32:
    *reduction_opcode = LPU::SREDOR32;
    return true;
  case LPU::OR16:
    *reduction_opcode = LPU::SREDOR16;
    return true;
  case LPU::OR8:
    *reduction_opcode = LPU::SREDOR8;
    return true;

  case LPU::XOR64:
    *reduction_opcode = LPU::SREDXOR64;
    return true;
  case LPU::XOR32:
    *reduction_opcode = LPU::SREDXOR32;
    return true;
  case LPU::XOR16:
    *reduction_opcode = LPU::SREDXOR16;
    return true;
  case LPU::XOR8:
    *reduction_opcode = LPU::SREDXOR8;
    return true;
    
  default:
    // No match. return false. 
    return false;
  }
}
