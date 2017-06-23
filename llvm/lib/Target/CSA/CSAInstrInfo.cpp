//===-- CSAInstrInfo.cpp - CSA Instruction Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the CSA implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "CSAInstrInfo.h"
#include "CSA.h"
#include "CSAMachineFunctionInfo.h"
#include "CSATargetMachine.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/ADT/StringRef.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "CSAGenInstrInfo.inc"

// GetCondFromBranchOpc - Return the CSA CC that matches
// the correspondent Branch instruction opcode.
static CSA::CondCode GetCondFromBranchOpc(unsigned BrOpc)
{
  switch (BrOpc) {
  default: return CSA::COND_INVALID;
  case CSA::BT  : return CSA::COND_T;
  case CSA::BF  : return CSA::COND_F;
  }
}

// GetCondBranchFromCond - Return the Branch instruction
// opcode that matches the cc.
static unsigned GetCondBranchFromCond(CSA::CondCode CC)
{
  switch (CC) {
  default: llvm_unreachable("Illegal condition code!");
  case CSA::COND_T  : return CSA::BT;
  case CSA::COND_F  : return CSA::BF;
  }
}

// GetOppositeBranchCondition - Return the inverse of the specified condition
static CSA::CondCode GetOppositeBranchCondition(CSA::CondCode CC)
{
  switch (CC) {
  default: llvm_unreachable("Illegal condition code!");
  case CSA::COND_T  : return CSA::COND_F;
  case CSA::COND_F  : return CSA::COND_T;
  }
}

// Pin the vtable to this file.
void CSAInstrInfo::anchor() {}

CSAInstrInfo::CSAInstrInfo(CSASubtarget &STI)
  : CSAGenInstrInfo(CSA::ADJCALLSTACKDOWN, CSA::ADJCALLSTACKUP),
    RI(*this) {}

void CSAInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I,
                                  const DebugLoc &DL, unsigned DestReg,
                                  unsigned SrcReg, bool KillSrc) const {
  // This could determine the opcode based on the minimum size of the source
  // and destination
  // For now, just use MOV64 to make sure all bits are moved.
  // Ideally, this would be based on the actual bits in the value...
  unsigned Opc = CSA::MOV64;

  BuildMI(MBB, I, DL, get(Opc), DestReg)
    .addReg(SrcReg, getKillRegState(KillSrc));
}


void CSAInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator MI,
                                          unsigned SrcReg, bool isKill, int FrameIdx,
                                          const TargetRegisterClass *RC,
                                          const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();
  unsigned opc;

  if        (RC == &CSA::I0RegClass  || RC == &CSA::RI0RegClass  || RC == &CSA::CI0RegClass ||
             RC == &CSA::I1RegClass  || RC == &CSA::RI1RegClass  || RC == &CSA::CI1RegClass ||
             RC == &CSA::I8RegClass  || RC == &CSA::RI8RegClass  || RC == &CSA::CI8RegClass) {
    opc = CSA::ST8D;
  } else if (RC == &CSA::I16RegClass || RC == &CSA::RI16RegClass || RC == &CSA::CI16RegClass) {
    opc = CSA::ST16D;
  } else if (RC == &CSA::I32RegClass || RC == &CSA::RI32RegClass || RC == &CSA::CI32RegClass) {
    opc = CSA::ST32D;
  } else if (RC == &CSA::I64RegClass || RC == &CSA::RI64RegClass || RC == &CSA::CI64RegClass) {
    opc = CSA::ST64D;
  } else {
    llvm_unreachable("Unknown register class");
  }

  BuildMI(MBB, MI, DL, get(opc)).addFrameIndex(FrameIdx).addImm(0)
    .addReg(SrcReg, getKillRegState(isKill));

}

void CSAInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                           MachineBasicBlock::iterator MI,
                                           unsigned DestReg, int FrameIdx,
                                           const TargetRegisterClass *RC,
                                           const TargetRegisterInfo *TRI) const{
  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();
  unsigned opc;

  if        (RC == &CSA::I0RegClass  || RC == &CSA::RI0RegClass  || RC == &CSA::CI0RegClass ||
             RC == &CSA::I1RegClass  || RC == &CSA::RI1RegClass  || RC == &CSA::CI1RegClass ||
             RC == &CSA::I8RegClass  || RC == &CSA::RI8RegClass  || RC == &CSA::CI8RegClass) {
    opc = CSA::LD8D;
  } else if (RC == &CSA::I16RegClass || RC == &CSA::RI16RegClass || RC == &CSA::CI16RegClass) {
    opc = CSA::LD16D;
  } else if (RC == &CSA::I32RegClass || RC == &CSA::RI32RegClass || RC == &CSA::CI32RegClass) {
    opc = CSA::LD32D;
  } else if (RC == &CSA::I64RegClass || RC == &CSA::RI64RegClass || RC == &CSA::CI64RegClass) {
    opc = CSA::LD64D;
  } else {
    llvm_unreachable("Unknown register class");
  }

  BuildMI(MBB, MI, DL, get(opc), DestReg).addFrameIndex(FrameIdx).addImm(0);
}

unsigned CSAInstrInfo::removeBranch(MachineBasicBlock &MBB, int *BytesAdded) const {
  assert(!BytesAdded && "code size not handled");

  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;

  while (I != MBB.begin()) {
    --I;
    if (I->isDebugValue())
      continue;
    if (I->getOpcode() != CSA::BR &&
        I->getOpcode() != CSA::BT &&
        I->getOpcode() != CSA::BF)
      break;
    // Remove the branch.
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }

  return Count;
}

bool CSAInstrInfo::
reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 2 && "Invalid branch condition!");
  Cond[0].setImm(GetOppositeBranchCondition((CSA::CondCode)Cond[0].getImm()));
  return false;
}
/*
bool CSAInstrInfo::isUnpredicatedTerminator(const MachineInstr &MI) const {
  if (!MI.isTerminator()) return false;

  // Conditional branch is a special case.
  if (MI.isBranch() && !MI.isBarrier())
    return true;
  if (!MI.isPredicable())
    return true;
  return !isPredicated(MI);
}
*/
bool CSAInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
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
    if (I->getOpcode() == CSA::BR) {
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
    CSA::CondCode BranchCode = GetCondFromBranchOpc(I->getOpcode());
    if (BranchCode == CSA::COND_INVALID)
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

    CSA::CondCode OldBranchCode = (CSA::CondCode)Cond[0].getImm();
    // If the conditions are the same, we can leave them alone.
    if (OldBranchCode == BranchCode)
      continue;

    return true;
  }

  return false;
}

unsigned
CSAInstrInfo::insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                              MachineBasicBlock *FBB,
                              ArrayRef<MachineOperand> Cond,
                              const DebugLoc &DL,
                              int *BytesAdded) const {
  assert(!BytesAdded && "code size not handled");
  // Shouldn't be a fall through.
  assert(TBB && "insertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 2 || Cond.size() == 0) &&
         "CSA branch conditions have two components!");

  if (FBB == 0) { // One way branch.
    if (Cond.empty()) {
      // Unconditional branch?
      BuildMI(&MBB, DL, get(CSA::BR)).addMBB(TBB);
    } else {
      // Conditional branch.
      unsigned Opc = GetCondBranchFromCond((CSA::CondCode)Cond[0].getImm());
      BuildMI(&MBB, DL, get(Opc)).addReg(Cond[1].getReg()).addMBB(TBB);
    }
    return 1;
  }

  // Two-way Conditional branch.
  unsigned Opc = GetCondBranchFromCond((CSA::CondCode)Cond[0].getImm());
  BuildMI(&MBB, DL, get(Opc)).addReg(Cond[1].getReg()).addMBB(TBB);
  BuildMI(&MBB, DL, get(CSA::BR)).addMBB(FBB);

  return 2;
}
/*
/// GetInstSize - Return the number of bytes of code the specified
/// instruction may be.  This returns the maximum number of bytes.
///
unsigned CSAInstrInfo::GetInstSizeInBytes(const MachineInstr *MI) const {
  const MCInstrDesc &Desc = MI->getDesc();

  switch (Desc.TSFlags & CSAII::SizeMask) {
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
  case CSAII::SizeSpecial:
    switch (MI->getOpcode()) {
    default: llvm_unreachable("Unknown instruction size!");
    case CSA::SAR8r1c:
    case CSA::SAR16r1c:
      return 4;
    }
  case CSAII::Size2Bytes:
    return 2;
  case CSAII::Size4Bytes:
    return 4;
  case CSAII::Size6Bytes:
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
CSAInstrInfo::getPickSwitchOpcode(const TargetRegisterClass *RC, 
				    bool isPick) const {

  if (isPick) {
    switch (RC->getID()) {
    default: llvm_unreachable("Unknown Target register class!");
    case CSA::I1RegClassID: return CSA::PICK1;
    case CSA::I8RegClassID: return CSA::PICK8;
    case CSA::I16RegClassID: return CSA::PICK16;
    case CSA::I32RegClassID: return CSA::PICK32;
    case CSA::I64RegClassID: return CSA::PICK64;
    case CSA::RI1RegClassID: return CSA::PICK1;
    case CSA::RI8RegClassID: return CSA::PICK8;
    case CSA::RI16RegClassID: return CSA::PICK16;
    case CSA::RI32RegClassID: return CSA::PICK32;
    case CSA::RI64RegClassID: return CSA::PICK64;
    }
  }


  switch (RC->getID()) {
  default: llvm_unreachable("Unknown Target register class!");
  case CSA::I1RegClassID: return CSA::SWITCH1;
  case CSA::I8RegClassID: return CSA::SWITCH8;
  case CSA::I16RegClassID: return CSA::SWITCH16;
  case CSA::I32RegClassID: return CSA::SWITCH32;
  case CSA::I64RegClassID: return CSA::SWITCH64;

  case CSA::RI1RegClassID: return CSA::SWITCH1;
  case CSA::RI8RegClassID: return CSA::SWITCH8;
  case CSA::RI16RegClassID: return CSA::SWITCH16;
  case CSA::RI32RegClassID: return CSA::SWITCH32;
  case CSA::RI64RegClassID: return CSA::SWITCH64;
  }

}



unsigned
CSAInstrInfo::getPickanyOpcode(const TargetRegisterClass *RC) const {

  switch (RC->getID()) {
  default: llvm_unreachable("Unknown Target register class!");
  case CSA::I1RegClassID: return CSA::PICKANY1;
  case CSA::I8RegClassID: return CSA::PICKANY8;
  case CSA::I16RegClassID: return CSA::PICKANY16;
  case CSA::I32RegClassID: return CSA::PICKANY32;
  case CSA::I64RegClassID: return CSA::PICKANY64;
  case CSA::RI1RegClassID: return CSA::PICKANY1;
  case CSA::RI8RegClassID: return CSA::PICKANY8;
  case CSA::RI16RegClassID: return CSA::PICKANY16;
  case CSA::RI32RegClassID: return CSA::PICKANY32;
  case CSA::RI64RegClassID: return CSA::PICKANY64;
  }
}

// Convert opcode of LD/ST/ATM* into a corresponding opcode for OLD/OST/OATM*.
// Returns current_opcode if it is not a LD, ST, or ATM*.
unsigned
CSAInstrInfo::get_ordered_opcode_for_LDST(unsigned current_opcode)  const {
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


bool CSAInstrInfo::isLoad(MachineInstr *MI) const {
	return MI->getOpcode() >= CSA::LD1 && MI->getOpcode() <= CSA::LDx88I;
}

bool CSAInstrInfo::isStore(MachineInstr *MI) const {
	return MI->getOpcode() >= CSA::ST1 && MI->getOpcode() <= CSA::STx88I;
}

bool CSAInstrInfo::isOrderedLoad(MachineInstr *MI) const {
  return ((MI->getOpcode() >= CSA::OLD1) && (MI->getOpcode() <= CSA::OLDx88I));
}

bool CSAInstrInfo::isOrderedStore(MachineInstr *MI) const {
  return ((MI->getOpcode() >= CSA::OST1) && (MI->getOpcode() <= CSA::OSTx88I));
}

bool CSAInstrInfo::isMul(MachineInstr *MI) const {
	return MI->getOpcode() >= CSA::MUL16 && MI->getOpcode() <= CSA::MULF64;
}


bool CSAInstrInfo::isDiv(MachineInstr *MI) const {
	return MI->getOpcode() >= CSA::DIVF16 && MI->getOpcode() <= CSA::DIVU8;
}

bool CSAInstrInfo::isFMA(MachineInstr *MI) const {
	return MI->getOpcode() >= CSA::FMAF16 && MI->getOpcode() <= CSA::FMAF64;
}

bool CSAInstrInfo::isAdd(MachineInstr *MI) const {
	return MI->getOpcode() >= CSA::ADD16 && MI->getOpcode() <= CSA::ADDF64;
}

bool CSAInstrInfo::isSub(MachineInstr *MI) const {
	return MI->getOpcode() >= CSA::SUB16 && MI->getOpcode() <= CSA::SUBF64;
}

bool CSAInstrInfo::isShift(MachineInstr *MI) const {
        return (MI->getOpcode() >= CSA::SLL16 && MI->getOpcode() <= CSA::SLL8) ||
          (MI->getOpcode() >= CSA::SRA16 && MI->getOpcode() <= CSA::SRL8);
}

bool CSAInstrInfo::isCmp(MachineInstr *MI) const {
    return ((MI->getOpcode() >= CSA::CMPEQ16) &&
            (MI->getOpcode() <= CSA::CMPUOF64));
}


bool CSAInstrInfo::isSwitch(MachineInstr *MI) const {
	return MI->getOpcode() == CSA::SWITCH1 ||
         MI->getOpcode() == CSA::SWITCH8 ||
         MI->getOpcode() == CSA::SWITCH16 ||
         MI->getOpcode() == CSA::SWITCH32 ||
         MI->getOpcode() == CSA::SWITCH64;
}

bool CSAInstrInfo::isPick(MachineInstr *MI) const {
        return MI->getOpcode() == CSA::PICK1 ||
		MI->getOpcode() == CSA::PICK8 ||
		MI->getOpcode() == CSA::PICK16 ||
		MI->getOpcode() == CSA::PICK32 ||
		MI->getOpcode() == CSA::PICK64;
}

bool CSAInstrInfo::isPickany(MachineInstr *MI) const {
  return MI->getOpcode() == CSA::PICKANY1 ||
    MI->getOpcode() == CSA::PICKANY8 ||
    MI->getOpcode() == CSA::PICKANY16 ||
    MI->getOpcode() == CSA::PICKANY32 ||
    MI->getOpcode() == CSA::PICKANY64;
}

bool CSAInstrInfo::isCopy(MachineInstr *MI) const {
  return
    MI->getOpcode() == CSA::COPY0 ||
    MI->getOpcode() == CSA::COPY1 ||
    MI->getOpcode() == CSA::COPY8 ||
    MI->getOpcode() == CSA::COPY16 ||
    MI->getOpcode() == CSA::COPY32 ||
    MI->getOpcode() == CSA::COPY64;
}

bool CSAInstrInfo::isMOV(MachineInstr *MI) const {
  return MI->getOpcode() == CSA::MOV1 ||
    MI->getOpcode() == CSA::MOV8 ||
    MI->getOpcode() == CSA::MOV16 ||
    MI->getOpcode() == CSA::MOV32 ||
    MI->getOpcode() == CSA::MOV64;
}


bool CSAInstrInfo::isInit(MachineInstr *MI) const {
  return MI->getOpcode() == CSA::INIT1 ||
    MI->getOpcode() == CSA::INIT8 ||
    MI->getOpcode() == CSA::INIT16 ||
    MI->getOpcode() == CSA::INIT32 ||
    MI->getOpcode() == CSA::INIT64;
}

bool CSAInstrInfo::isAtomic(MachineInstr *MI) const {
    return MI->getOpcode() >= CSA::ATMADD16 && MI->getOpcode() <= CSA::ATMXOR8;
}

bool CSAInstrInfo::isOrderedAtomic(MachineInstr *MI) const {
    return MI->getOpcode() >= CSA::OATMADD16 && MI->getOpcode() <= CSA::OATMXOR8;
}

bool CSAInstrInfo::isSeqOT(MachineInstr *MI) const {
    return ((MI->getOpcode() >= CSA::SEQOTGE) &&
            (MI->getOpcode() <= CSA::SEQOTNE8));
}

unsigned CSAInstrInfo::getMemTokenMOVOpcode() const {
  return CSA::MOV0;
}

bool CSAInstrInfo::isMemTokenMOV(MachineInstr* MI) const {
  return MI->getOpcode() == CSA::MOV0;
}


unsigned
CSAInstrInfo::getCopyOpcode(const TargetRegisterClass *RC) const {
  if      (RC == &CSA::I1RegClass)  return CSA::COPY1;
  else if (RC == &CSA::I8RegClass)  return CSA::COPY8;
  else if (RC == &CSA::I16RegClass) return CSA::COPY16;
  else if (RC == &CSA::I32RegClass) return CSA::COPY32;
  else if (RC == &CSA::I64RegClass) return CSA::COPY64;
  else
    llvm_unreachable("Unknown Target LIC class!");
}

unsigned
CSAInstrInfo::getMoveOpcode(const TargetRegisterClass *RC) const {
  if (RC == &CSA::I1RegClass || RC == &CSA::CI1RegClass || RC == &CSA::RI1RegClass)  return CSA::MOV1;
  else if (RC == &CSA::I8RegClass || RC == &CSA::CI8RegClass || RC == &CSA::RI8RegClass)  return CSA::MOV8;
  else if (RC == &CSA::I16RegClass || RC == &CSA::CI16RegClass || RC == &CSA::RI16RegClass) return CSA::MOV16;
  else if (RC == &CSA::I32RegClass || RC == &CSA::CI32RegClass || RC == &CSA::RI32RegClass) return CSA::MOV32;
  else if (RC == &CSA::I64RegClass || RC == &CSA::CI64RegClass || RC == &CSA::RI64RegClass) return CSA::MOV64;
  else
    llvm_unreachable("Unknown Target LIC class!");
}

unsigned
CSAInstrInfo::getRepeatOpcode(const TargetRegisterClass *RC) const {
  if (RC == &CSA::I1RegClass || RC == &CSA::CI1RegClass || RC == &CSA::RI1RegClass)  return CSA::REPEAT1;
  else if (RC == &CSA::I8RegClass || RC == &CSA::CI8RegClass || RC == &CSA::RI8RegClass)  return CSA::REPEAT8;
  else if (RC == &CSA::I16RegClass || RC == &CSA::CI16RegClass || RC == &CSA::RI16RegClass) return CSA::REPEAT16;
  else if (RC == &CSA::I32RegClass || RC == &CSA::CI32RegClass || RC == &CSA::RI32RegClass) return CSA::REPEAT32;
  else if (RC == &CSA::I64RegClass || RC == &CSA::CI64RegClass || RC == &CSA::RI64RegClass) return CSA::REPEAT64;
  else
    llvm_unreachable("Unknown Target LIC class!");
}


unsigned
CSAInstrInfo::getInitOpcode(const TargetRegisterClass *RC) const {
  if (RC == &CSA::I1RegClass || RC == &CSA::CI1RegClass || RC == &CSA::RI1RegClass)  return CSA::INIT1;
  else if (RC == &CSA::I8RegClass || RC == &CSA::CI8RegClass || RC == &CSA::RI8RegClass)  return CSA::INIT8;
  else if (RC == &CSA::I16RegClass || RC == &CSA::CI16RegClass || RC == &CSA::RI16RegClass) return CSA::INIT16;
  else if (RC == &CSA::I32RegClass || RC == &CSA::CI32RegClass || RC == &CSA::RI32RegClass) return CSA::INIT32;
  else if (RC == &CSA::I64RegClass || RC == &CSA::CI64RegClass || RC == &CSA::RI64RegClass) return CSA::INIT64;
  else
    llvm_unreachable("Unknown Target LIC class!");
}


// TBD(jsukha): This table lookup works for now, but there must be a
// better way to implement this matching operation...
unsigned CSAInstrInfo::commuteNegateCompareOpcode(unsigned cmp_opcode,
                                                  bool commute_compare_operands,
                                                  bool negate_eq) const {

    // We need to a switch a "<" to a ">=" if we swap the operands,
    // and if we negate the output.  If we do both, then they cancel
    // each other out.
    bool swap_ltgt = commute_compare_operands ^ negate_eq;
      
    switch (cmp_opcode) {
      // == maps to == (self)
    case CSA::CMPEQ8:
      return negate_eq ? CSA::CMPNE8 :  CSA::CMPEQ8;
    case CSA::CMPEQ16:
      return negate_eq ? CSA::CMPNE16 :  CSA::CMPEQ16;      
    case CSA::CMPEQ32:
      return negate_eq ? CSA::CMPNE32 :  CSA::CMPEQ32;            
    case CSA::CMPEQ64:
      return negate_eq ? CSA::CMPNE64 :  CSA::CMPEQ64;
      
    // ">=" maps to "<"
    case CSA::CMPGES8:
      return swap_ltgt ? CSA::CMPLTS8 : CSA::CMPGES8;
    case CSA::CMPGES16:
      return swap_ltgt ? CSA::CMPLTS16 : CSA::CMPGES16;      
    case CSA::CMPGES32:
      return swap_ltgt ? CSA::CMPLTS32 : CSA::CMPGES32;      
    case CSA::CMPGES64:
      return swap_ltgt ? CSA::CMPLTS64 : CSA::CMPGES64;      
    case CSA::CMPGEU8:
      return swap_ltgt ? CSA::CMPLTU8 : CSA::CMPGEU8;
    case CSA::CMPGEU16:
      return swap_ltgt ? CSA::CMPLTU16 : CSA::CMPGEU16;      
    case CSA::CMPGEU32:
      return swap_ltgt ? CSA::CMPLTU32 : CSA::CMPGEU32;
    case CSA::CMPGEU64:
      return swap_ltgt ? CSA::CMPLTU64 : CSA::CMPGEU64;

    // ">" maps to "<="
    case CSA::CMPGTS8:
      return swap_ltgt ? CSA::CMPLES8 : CSA::CMPGTS8;
    case CSA::CMPGTS16:
      return swap_ltgt ? CSA::CMPLES16 : CSA::CMPGTS16;      
    case CSA::CMPGTS32:
      return swap_ltgt ? CSA::CMPLES32 : CSA::CMPGTS32;      
    case CSA::CMPGTS64:
      return swap_ltgt ? CSA::CMPLES64 : CSA::CMPGTS64;      
    case CSA::CMPGTU8:
      return swap_ltgt ? CSA::CMPLEU8 : CSA::CMPGTU8;
    case CSA::CMPGTU16:
      return swap_ltgt ? CSA::CMPLEU16 : CSA::CMPGTU16;      
    case CSA::CMPGTU32:
      return swap_ltgt ? CSA::CMPLEU32 : CSA::CMPGTU32;      
    case CSA::CMPGTU64:
      return swap_ltgt ? CSA::CMPLEU64 : CSA::CMPGTU64;      

    // "<=" maps to ">"
    case CSA::CMPLES8:
      return swap_ltgt ? CSA::CMPGTS8 : CSA::CMPLES8;
    case CSA::CMPLES16:
      return swap_ltgt ? CSA::CMPGTS16 : CSA::CMPLES16;      
    case CSA::CMPLES32:
      return swap_ltgt ? CSA::CMPGTS32 : CSA::CMPLES32;      
    case CSA::CMPLES64:
      return swap_ltgt ? CSA::CMPGTS64 : CSA::CMPLES64;      
    case CSA::CMPLEU8:
      return swap_ltgt ? CSA::CMPGTU8 : CSA::CMPLEU8;
    case CSA::CMPLEU16:
      return swap_ltgt ? CSA::CMPGTU16 : CSA::CMPLEU16;
    case CSA::CMPLEU32:
      return swap_ltgt ? CSA::CMPGTU32 : CSA::CMPLEU32;      
    case CSA::CMPLEU64:
      return swap_ltgt ? CSA::CMPGTU64 : CSA::CMPLEU64;            

    // "<" maps to ">="
    case CSA::CMPLTS8:
      return swap_ltgt ? CSA::CMPGES8 : CSA::CMPLTS8;
    case CSA::CMPLTS16:
      return swap_ltgt ? CSA::CMPGES16 : CSA::CMPLTS16;      
    case CSA::CMPLTS32:
      return swap_ltgt ? CSA::CMPGES32 : CSA::CMPLTS32;      
    case CSA::CMPLTS64:
      return swap_ltgt ? CSA::CMPGES64 : CSA::CMPLTS64;      
    case CSA::CMPLTU8:
      return swap_ltgt ? CSA::CMPGEU8 : CSA::CMPLTU8;
    case CSA::CMPLTU16:
      return swap_ltgt ? CSA::CMPGEU16 : CSA::CMPLTU16;
    case CSA::CMPLTU32:
      return swap_ltgt ? CSA::CMPGEU32 : CSA::CMPLTU32;      
    case CSA::CMPLTU64:
      return swap_ltgt ? CSA::CMPGEU64 : CSA::CMPLTU64;      

    // != maps to !=  (self)
    case CSA::CMPNE8:
      return negate_eq ? CSA::CMPEQ8 : CSA::CMPNE8;
    case CSA::CMPNE16:
      return negate_eq ? CSA::CMPEQ16 : CSA::CMPNE16;      
    case CSA::CMPNE32:
      return negate_eq ? CSA::CMPEQ32 : CSA::CMPNE32;      
    case CSA::CMPNE64:
      return negate_eq ? CSA::CMPEQ64 : CSA::CMPNE64;      

    // Floating-point equal: maps to self. 
    // == maps to ==
    case CSA::CMPOEQF16:
      return negate_eq ? CSA::CMPONEF16 : CSA::CMPOEQF16;
    case CSA::CMPOEQF32:
      return negate_eq ? CSA::CMPONEF32 : CSA::CMPOEQF32;      
    case CSA::CMPOEQF64:
      return negate_eq ? CSA::CMPONEF64 : CSA::CMPOEQF64;
    case CSA::CMPUEQF16:
      return negate_eq ? CSA::CMPUNEF16 : CSA::CMPUEQF16;
    case CSA::CMPUEQF32:
      return negate_eq ? CSA::CMPUNEF32 : CSA::CMPUEQF32;      
    case CSA::CMPUEQF64:
      return negate_eq ? CSA::CMPUNEF64 : CSA::CMPUEQF64;      

    // >= to <
    case CSA::CMPOGEF16:
      return swap_ltgt ? CSA::CMPOLTF16 : CSA::CMPOGEF16;
    case CSA::CMPOGEF32:
      return swap_ltgt ? CSA::CMPOLTF32 : CSA::CMPOGEF32;      
    case CSA::CMPOGEF64:
      return swap_ltgt ? CSA::CMPOLTF64 : CSA::CMPOGEF64;      
    case CSA::CMPUGEF16:
      return swap_ltgt ? CSA::CMPULTF16 : CSA::CMPUGEF16;
    case CSA::CMPUGEF32:
      return swap_ltgt ? CSA::CMPULTF32 : CSA::CMPUGEF32;
    case CSA::CMPUGEF64:
      return swap_ltgt ? CSA::CMPULTF64 : CSA::CMPUGEF64;      
      
    // > to <=
    case CSA::CMPOGTF16:
      return swap_ltgt ? CSA::CMPOLEF16 : CSA::CMPOGTF16;
    case CSA::CMPOGTF32:
      return swap_ltgt ? CSA::CMPOLEF32 : CSA::CMPOGTF32;      
    case CSA::CMPOGTF64:
      return swap_ltgt ? CSA::CMPOLEF64 : CSA::CMPOGTF64;      
    case CSA::CMPUGTF16:
      return swap_ltgt ? CSA::CMPULEF16 : CSA::CMPUGTF16;
    case CSA::CMPUGTF32:
      return swap_ltgt ? CSA::CMPULEF32 : CSA::CMPUGTF32;      
    case CSA::CMPUGTF64:
      return swap_ltgt ? CSA::CMPULEF64 : CSA::CMPUGTF64;      
      
    // <= to >
    case CSA::CMPOLEF16:
      return swap_ltgt ? CSA::CMPOGTF16 : CSA::CMPOLEF16;
    case CSA::CMPOLEF32:
      return swap_ltgt ? CSA::CMPOGTF32 : CSA::CMPOLEF32;
    case CSA::CMPOLEF64:
      return swap_ltgt ? CSA::CMPOGTF64 : CSA::CMPOLEF64;      
    case CSA::CMPULEF16:
      return swap_ltgt ? CSA::CMPUGTF16 : CSA::CMPULEF16;
    case CSA::CMPULEF32:
      return swap_ltgt ? CSA::CMPUGTF32 : CSA::CMPULEF32;      
    case CSA::CMPULEF64:
      return swap_ltgt ? CSA::CMPUGTF64 : CSA::CMPULEF64;            
      
    // < to >=
    case CSA::CMPOLTF16:
      return swap_ltgt ? CSA::CMPOGEF16 : CSA::CMPOLTF16;
    case CSA::CMPOLTF32:
      return swap_ltgt ? CSA::CMPOGEF32 : CSA::CMPOLTF32;
    case CSA::CMPOLTF64:
      return swap_ltgt ? CSA::CMPOGEF64 : CSA::CMPOLTF64;      
    case CSA::CMPULTF16:
      return swap_ltgt ? CSA::CMPUGEF16 : CSA::CMPULTF16;
    case CSA::CMPULTF32:
      return swap_ltgt ? CSA::CMPUGEF32 : CSA::CMPULTF32;      
    case CSA::CMPULTF64:
      return swap_ltgt ? CSA::CMPUGEF64 : CSA::CMPULTF64;            

    // != maps to !=
    case CSA::CMPONEF16:
      return negate_eq ? CSA::CMPOEQF16 : CSA::CMPONEF16;
    case CSA::CMPONEF32:
      return negate_eq ? CSA::CMPOEQF32 : CSA::CMPONEF32;
    case CSA::CMPONEF64:
      return negate_eq ? CSA::CMPOEQF64 : CSA::CMPONEF64;
    case CSA::CMPUNEF16:
      return negate_eq ? CSA::CMPUEQF16 : CSA::CMPUNEF16;
    case CSA::CMPUNEF32:
      return negate_eq ? CSA::CMPUEQF32 : CSA::CMPUNEF32;      
    case CSA::CMPUNEF64:
      return negate_eq ? CSA::CMPUEQF64 : CSA::CMPUNEF64;      

    // Die by default.  We should never call this method on any opcode
    // which is not a compare.
    default:
      assert(0);
      return cmp_opcode;
    }
}


unsigned CSAInstrInfo::
convertCompareOpToSeqOTOp(unsigned cmp_opcode) const {
    switch (cmp_opcode) {
    // ">="
    case CSA::CMPGES8:
      return CSA::SEQOTGES8;
    case CSA::CMPGES16:
      return CSA::SEQOTGES16;
    case CSA::CMPGES32:
      return CSA::SEQOTGES32;
    case CSA::CMPGES64:
      return CSA::SEQOTGES64;
    case CSA::CMPGEU8:
      return CSA::SEQOTGEU8;
    case CSA::CMPGEU16:
      return CSA::SEQOTGEU16;
    case CSA::CMPGEU32:
      return CSA::SEQOTGEU32;
    case CSA::CMPGEU64:
      return CSA::SEQOTGEU64;

    // ">" 
    case CSA::CMPGTS8:
      return CSA::SEQOTGTS8;
    case CSA::CMPGTS16:
      return CSA::SEQOTGTS16;
    case CSA::CMPGTS32:
      return CSA::SEQOTGTS32;
    case CSA::CMPGTS64:
      return CSA::SEQOTGTS64;
    case CSA::CMPGTU8:
      return CSA::SEQOTGTU8;
    case CSA::CMPGTU16:
      return CSA::SEQOTGTU16;
    case CSA::CMPGTU32:
      return CSA::SEQOTGTU32;
    case CSA::CMPGTU64:
      return CSA::SEQOTGTU64;

    // "<=" 
    case CSA::CMPLES8:
      return CSA::SEQOTLES8;
    case CSA::CMPLES16:
      return CSA::SEQOTLES16;
    case CSA::CMPLES32:
      return CSA::SEQOTLES32;
    case CSA::CMPLES64:
      return CSA::SEQOTLES64;
    case CSA::CMPLEU8:
      return CSA::SEQOTLEU8;
    case CSA::CMPLEU16:
      return CSA::SEQOTLEU16;
    case CSA::CMPLEU32:
      return CSA::SEQOTLEU32;
    case CSA::CMPLEU64:
      return CSA::SEQOTLEU64;

    // "<" 
    case CSA::CMPLTS8:
      return CSA::SEQOTLTS8;
    case CSA::CMPLTS16:
      return CSA::SEQOTLTS16;
    case CSA::CMPLTS32:
      return CSA::SEQOTLTS32;
    case CSA::CMPLTS64:
      return CSA::SEQOTLTS64;
    case CSA::CMPLTU8:
      return CSA::SEQOTLTU8;
    case CSA::CMPLTU16:
      return CSA::SEQOTLTU16;
    case CSA::CMPLTU32:
      return CSA::SEQOTLTU32;
    case CSA::CMPLTU64:
      return CSA::SEQOTLTU64;

    // !=
    case CSA::CMPNE8:
      return CSA::SEQOTNE8;
    case CSA::CMPNE16:
      return CSA::SEQOTNE16;
    case CSA::CMPNE32:
      return CSA::SEQOTNE32;
    case CSA::CMPNE64:
      return CSA::SEQOTNE64;
      

    // By default, return the same opcode. 
    default:
      return cmp_opcode;
    }
}


unsigned CSAInstrInfo::
promoteSeqOTOpBitwidth(unsigned seq_opcode,
                       int bitwidth) const {
    switch (seq_opcode) {
      // This code is relying on the fall-through of switch, to end up
      // picking the smallest size that is both larger than the size
      // specified in seq_opcode and >= bitwidth.

    //">="
    case CSA::SEQOTGES8:
      if (bitwidth <= 8) { return CSA::SEQOTGES8; }
    case CSA::SEQOTGES16:
      if (bitwidth <= 16) { return CSA::SEQOTGES16; }      
    case CSA::SEQOTGES32:
      if (bitwidth <= 32) { return CSA::SEQOTGES32; }            
    case CSA::SEQOTGES64:
      return CSA::SEQOTGES64;

    case CSA::SEQOTGEU8:
      if (bitwidth <= 8) { return CSA::SEQOTGEU8; }
    case CSA::SEQOTGEU16:
      if (bitwidth <= 16) { return CSA::SEQOTGEU16; }      
    case CSA::SEQOTGEU32:
      if (bitwidth <= 32) { return CSA::SEQOTGEU32; }            
    case CSA::SEQOTGEU64:
      return CSA::SEQOTGEU64;

    // ">"
    case CSA::SEQOTGTS8:
      if (bitwidth <= 8) { return CSA::SEQOTGTS8; }
    case CSA::SEQOTGTS16:
      if (bitwidth <= 16) { return CSA::SEQOTGTS16; }      
    case CSA::SEQOTGTS32:
      if (bitwidth <= 32) { return CSA::SEQOTGTS32; }            
    case CSA::SEQOTGTS64:
      return CSA::SEQOTGTS64;

    case CSA::SEQOTGTU8:
      if (bitwidth <= 8) { return CSA::SEQOTGTU8; }
    case CSA::SEQOTGTU16:
      if (bitwidth <= 16) { return CSA::SEQOTGTU16; }      
    case CSA::SEQOTGTU32:
      if (bitwidth <= 32) { return CSA::SEQOTGTU32; }            
    case CSA::SEQOTGTU64:
      return CSA::SEQOTGTU64;


    // "<="
    case CSA::SEQOTLES8:
      if (bitwidth <= 8) { return CSA::SEQOTLES8; }
    case CSA::SEQOTLES16:
      if (bitwidth <= 16) { return CSA::SEQOTLES16; }      
    case CSA::SEQOTLES32:
      if (bitwidth <= 32) { return CSA::SEQOTLES32; }            
    case CSA::SEQOTLES64:
      return CSA::SEQOTLES64;

    case CSA::SEQOTLEU8:
      if (bitwidth <= 8) { return CSA::SEQOTLEU8; }
    case CSA::SEQOTLEU16:
      if (bitwidth <= 16) { return CSA::SEQOTLEU16; }      
    case CSA::SEQOTLEU32:
      if (bitwidth <= 32) { return CSA::SEQOTLEU32; }            
    case CSA::SEQOTLEU64:
      return CSA::SEQOTLEU64;

    // "<"
    case CSA::SEQOTLTS8:
      if (bitwidth <= 8) { return CSA::SEQOTLTS8; }
    case CSA::SEQOTLTS16:
      if (bitwidth <= 16) { return CSA::SEQOTLTS16; }      
    case CSA::SEQOTLTS32:
      if (bitwidth <= 32) { return CSA::SEQOTLTS32; }            
    case CSA::SEQOTLTS64:
      return CSA::SEQOTLTS64;

    case CSA::SEQOTLTU8:
      if (bitwidth <= 8) { return CSA::SEQOTLTU8; }
    case CSA::SEQOTLTU16:
      if (bitwidth <= 16) { return CSA::SEQOTLTU16; }      
    case CSA::SEQOTLTU32:
      if (bitwidth <= 32) { return CSA::SEQOTLTU32; }            
    case CSA::SEQOTLTU64:
      return CSA::SEQOTLTU64;

    // !=
    case CSA::SEQOTNE8:
      if (bitwidth <= 8) { return CSA::SEQOTNE8; }
    case CSA::SEQOTNE16:
      if (bitwidth <= 16) { return CSA::SEQOTNE16; }      
    case CSA::SEQOTNE32:
      if (bitwidth <= 32) { return CSA::SEQOTNE32; }            
    case CSA::SEQOTNE64:
      return CSA::SEQOTNE64;
      
    // By default, return the same opcode. 
    default:
      return seq_opcode;
    }
}


bool
CSAInstrInfo::
convertAddToStrideOp(unsigned add_opcode,
                     unsigned* strideOpcode) const {
  switch (add_opcode) {
  case CSA::ADD64:
    *strideOpcode = CSA::STRIDE64;
    return true;

  case CSA::ADD32:
    *strideOpcode = CSA::STRIDE32;
    return true;
    
  case CSA::ADD16:
    *strideOpcode = CSA::STRIDE16;
    return true;

  case CSA::ADD8:
    *strideOpcode = CSA::STRIDE8;
    return true;
    
  default:
    // No match. return false. 
    return false;
  }
}

bool
CSAInstrInfo::
convertSubToStrideOp(unsigned sub_opcode,
                     unsigned* strideOpcode) const {
  switch (sub_opcode) {
  case CSA::SUB64:
    *strideOpcode = CSA::STRIDE64;
    return true;

  case CSA::SUB32:
    *strideOpcode = CSA::STRIDE32;
    return true;
    
  case CSA::SUB16:
    *strideOpcode = CSA::STRIDE16;
    return true;

  case CSA::SUB8:
    *strideOpcode = CSA::STRIDE8;
    return true;
    
  default:
    // No match. return false. 
    return false;
  }
}

bool
CSAInstrInfo::
negateOpForStride(unsigned strideOpcode,
                  unsigned* negOpcode) const {
  switch (strideOpcode) {
  case CSA::STRIDE64:
    *negOpcode = CSA::NEG64;
    return true;

  case CSA::STRIDE32:
    *negOpcode = CSA::NEG32;
    return true;
    
  case CSA::STRIDE16:
    *negOpcode = CSA::NEG16;
    return true;

  case CSA::STRIDE8:
    *negOpcode = CSA::NEG8;
    return true;

  default:
    // No match. return false. 
    return false;
  }
}

const TargetRegisterClass*
CSAInstrInfo::
getStrideInputRC(unsigned strideOpcode) const {
  switch (strideOpcode) {
  case CSA::STRIDE64:
    return &CSA::CI64RegClass;
  case CSA::STRIDE32:
    return &CSA::CI32RegClass;
  case CSA::STRIDE16:
    return &CSA::CI16RegClass;
  case CSA::STRIDE8:
    return &CSA::CI8RegClass;
  default:
    // No match. return false.
    return NULL;
  }
}


bool
CSAInstrInfo::
convertPickToRepeatOp(unsigned pick_opcode,
                      unsigned* repeat_opcode) const {
  switch (pick_opcode) {
  case CSA::PICK64:
    *repeat_opcode = CSA::REPEAT64;
    return true;
  case CSA::PICK32:
    *repeat_opcode = CSA::REPEAT32;
    return true;
  case CSA::PICK16:
    *repeat_opcode = CSA::REPEAT16;
    return true;
  case CSA::PICK8:
    *repeat_opcode = CSA::REPEAT8;
    return true;
  case CSA::PICK1:
    *repeat_opcode = CSA::REPEAT1;    
    return true;
  default:
    // No match. return false. 
    return false;
  }
}

bool
CSAInstrInfo::
isCommutingReductionTransform(MachineInstr* MI) const {
  unsigned opcode = MI->getOpcode();
  // NOTE: we are leaving out AND1, OR1, and XOR1.
  // We don't have a 1-bit reduction op...
  return (isAdd(MI) ||
          isMul(MI) ||
          ((CSA::AND16 <= opcode) && (opcode <= CSA::AND8)) ||
          ((CSA::OR16 <= opcode) && (opcode <= CSA::OR8)) ||
          ((CSA::XOR16 <= opcode) && (opcode <= CSA::XOR8)));
}


bool
CSAInstrInfo::
convertTransformToReductionOp(unsigned transform_opcode,
                              unsigned* reduction_opcode) const {
  switch (transform_opcode) {

  case CSA::FMAF64:
    *reduction_opcode = CSA::FMSREDAF64;
    return true;
  case CSA::FMAF32:
    *reduction_opcode = CSA::FMSREDAF32;
    return true;

  case CSA::ADDF64:
    *reduction_opcode = CSA::SREDADDF64;
    return true;
  case CSA::ADDF32:
    *reduction_opcode = CSA::SREDADDF32;
    return true;    
  case CSA::ADD64:
    *reduction_opcode = CSA::SREDADD64;
    return true;
  case CSA::ADD32:
    *reduction_opcode = CSA::SREDADD32;
    return true;
  case CSA::ADD16:
    *reduction_opcode = CSA::SREDADD16;
    return true;

  case CSA::SUBF64:
    *reduction_opcode = CSA::SREDSUBF64;
    return true;
  case CSA::SUBF32:
    *reduction_opcode = CSA::SREDSUBF32;
    return true;    
  case CSA::SUB64:
    *reduction_opcode = CSA::SREDSUB64;
    return true;
  case CSA::SUB32:
    *reduction_opcode = CSA::SREDSUB32;
    return true;
  case CSA::SUB16:
    *reduction_opcode = CSA::SREDSUB16;
    return true;

  case CSA::MULF64:
    *reduction_opcode = CSA::SREDMULF64;
    return true;
  case CSA::MULF32:
    *reduction_opcode = CSA::SREDMULF32;
    return true;    
  case CSA::MUL64:
    *reduction_opcode = CSA::SREDMUL64;
    return true;
  case CSA::MUL32:
    *reduction_opcode = CSA::SREDMUL32;
    return true;
  case CSA::MUL16:
    *reduction_opcode = CSA::SREDMUL16;
    return true;

  case CSA::AND64:
    *reduction_opcode = CSA::SREDAND64;
    return true;
  case CSA::AND32:
    *reduction_opcode = CSA::SREDAND32;
    return true;
  case CSA::AND16:
    *reduction_opcode = CSA::SREDAND16;
    return true;
  case CSA::AND8:
    *reduction_opcode = CSA::SREDAND8;
    return true;

  case CSA::OR64:
    *reduction_opcode = CSA::SREDOR64;
    return true;
  case CSA::OR32:
    *reduction_opcode = CSA::SREDOR32;
    return true;
  case CSA::OR16:
    *reduction_opcode = CSA::SREDOR16;
    return true;
  case CSA::OR8:
    *reduction_opcode = CSA::SREDOR8;
    return true;

  case CSA::XOR64:
    *reduction_opcode = CSA::SREDXOR64;
    return true;
  case CSA::XOR32:
    *reduction_opcode = CSA::SREDXOR32;
    return true;
  case CSA::XOR16:
    *reduction_opcode = CSA::SREDXOR16;
    return true;
  case CSA::XOR8:
    *reduction_opcode = CSA::SREDXOR8;
    return true;
    
  default:
    // No match. return false. 
    return false;
  }
}


// TBD(jsukha): My initial attempt at the implementation was to call
// TargetRegisterClass::getMinimalPhysRegClass.  But this method seems
// to end up picking the ANYC register class, which is not what we
// want...
const TargetRegisterClass*
CSAInstrInfo::lookupLICRegClass(unsigned reg) const {
  if (CSA::CI64RegClass.contains(reg)) {
    return &CSA::CI64RegClass;
  }
  else if (CSA::CI32RegClass.contains(reg)) {
    return &CSA::CI32RegClass;    
  }
  if (CSA::CI16RegClass.contains(reg)) {
    return &CSA::CI16RegClass;
  }
  else if (CSA::CI8RegClass.contains(reg)) {
    return &CSA::CI8RegClass;    
  }
  if (CSA::CI1RegClass.contains(reg)) {
    return &CSA::CI1RegClass;
  }
  else if (CSA::CI0RegClass.contains(reg)) {
    return &CSA::CI0RegClass;    
  }
  return nullptr;
}


int
CSAInstrInfo::
getMOVBitwidth(unsigned mov_opcode) const {
  switch (mov_opcode) {
  // Ordered from largest to smallest, under the assumption that
  // larger sizes are more common, and that the first cases are
  // faster.
  case CSA::MOV64:
    return 64;
  case CSA::MOV32:
    return 32;
  case CSA::MOV16:
    return 16;
  case CSA::MOV8:
    return 8;
  case CSA::MOV1:
    return 1;
  case CSA::MOV0:
    return 0;
  default:
    return -1;
  }
}

unsigned CSAInstrInfo::getMemOpAccessWidth(unsigned opcode) const {
  if ((opcode >= CSA::LD1 and opcode <= CSA::LD8X)
      or (opcode >= CSA::ST1 and opcode <= CSA::ST8X)
      or (opcode >= CSA::ATMADD and opcode <= CSA::ATMXOR8))
    return 1;
  else if ((opcode >= CSA::LDx81 and opcode <= CSA::LDx88I)
      or (opcode >= CSA::STx81 and opcode <= CSA::STx88I))
    return 8;
  else return 0;
}
