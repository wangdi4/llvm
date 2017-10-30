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

// These are the register flags that ought to be used when specifying %ign as
// an memory ordering output channel when constructing memory operations.
// RegState::Dead is used here because values stored into %ign are dead but
// LLVM won't know that unless they're explicitly marked.
constexpr unsigned ISSUED_REGSTATE = RegState::Define | RegState::Dead;

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

  BuildMI(MBB, MI, DL, get(opc)).addReg(CSA::IGN, ISSUED_REGSTATE)
    .addFrameIndex(FrameIdx).addImm(0).addReg(SrcReg, getKillRegState(isKill))
    .addImm(CSA::MEMLEVEL_T0).addReg(CSA::IGN);

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

  BuildMI(MBB, MI, DL, get(opc), DestReg).addReg(CSA::IGN, ISSUED_REGSTATE)
    .addFrameIndex(FrameIdx).addImm(0).addImm(CSA::MEMLEVEL_T0).addReg(CSA::IGN);
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

struct OpcGenericMap {
  CSA::Generic genericOpcode;
  unsigned opSize;
  unsigned opClassification;
};

struct GenericOpcMap {
  CSA::Generic genericOpcode;
  unsigned opcode;
  unsigned opSize;
  unsigned opClassification;
};

#define GET_OPC_GENERIC_MAP
#include "CSAGenCSAOpInfo.inc"

CSA::Generic CSAInstrInfo::getGenericOpcode(unsigned opcode) const {
  assert(opcode < CSA::INSTRUCTION_LIST_END && "Illegal opcode");
  return opcode_to_generic_map[opcode].genericOpcode;
}

unsigned CSAInstrInfo::getLicSize(unsigned opcode) const {
  assert(opcode < CSA::INSTRUCTION_LIST_END && "Illegal opcode");
  return opcode_to_generic_map[opcode].opSize;
}

CSA::OpcodeClass CSAInstrInfo::getOpcodeClass(unsigned opcode) const {
  assert(opcode < CSA::INSTRUCTION_LIST_END && "Illegal opcode");
  return static_cast<CSA::OpcodeClass>(
      opcode_to_generic_map[opcode].opClassification);
}

unsigned CSAInstrInfo::makeOpcode(CSA::Generic generic, unsigned size,
    CSA::OpcodeClass opcodeClass, bool *exists) const {
  unsigned startIndex = generic_index_map[static_cast<unsigned>(generic)];
  assert(startIndex != ~0U && "Generic opcode has no valid opcodes");
  for (unsigned index = startIndex;
      generic_to_opcode_map[index].genericOpcode == generic;
      index++) {
    auto &entry = generic_to_opcode_map[index];
    if (entry.opSize == size &&
        (opcodeClass == CSA::VARIANT_DONTCARE ||
         opcodeClass == entry.opClassification)) {
      if (exists)
        *exists = true;
      return entry.opcode;
    }
  }

  assert(exists && "No valid opcode could be found");
  *exists = false;
  return 0;
}

unsigned
CSAInstrInfo::adjustOpcode(unsigned oldOpcode, CSA::Generic opcode) const {
  bool exists = false;
  unsigned newOpcode = makeOpcode(opcode, getLicSize(oldOpcode),
      getOpcodeClass(oldOpcode), &exists);
  if (!exists)
    newOpcode = oldOpcode;
  return newOpcode;
}

unsigned
CSAInstrInfo::getSizeOfRegisterClass(const TargetRegisterClass *RC) const {
  switch (RC->getID()) {
    default: llvm_unreachable("Unknown Target register class!");
    case CSA::CI0RegClassID: case CSA::I0RegClassID: case CSA::RI0RegClassID:
      return 0;
    case CSA::CI1RegClassID: case CSA::I1RegClassID: case CSA::RI1RegClassID:
      return 1;
    case CSA::CI8RegClassID: case CSA::I8RegClassID: case CSA::RI8RegClassID:
      return 8;
    case CSA::CI16RegClassID: case CSA::I16RegClassID: case CSA::RI16RegClassID:
      return 16;
    case CSA::CI32RegClassID: case CSA::I32RegClassID: case CSA::RI32RegClassID:
      return 32;
    case CSA::CI64RegClassID: case CSA::I64RegClassID: case CSA::RI64RegClassID:
      return 64;
  }
}

unsigned CSAInstrInfo::getPickSwitchOpcode(const TargetRegisterClass *RC,
                                           bool isPick) const {
  return makeOpcode(isPick ? CSA::Generic::PICK : CSA::Generic::SWITCH, RC);
}

bool CSAInstrInfo::isLoad(const MachineInstr *MI) const {
	return MI->getOpcode() >= CSA::LD1 && MI->getOpcode() <= CSA::LD8X;
}

bool CSAInstrInfo::isStore(const MachineInstr *MI) const {
	return MI->getOpcode() >= CSA::ST1 && MI->getOpcode() <= CSA::ST8X;
}

bool CSAInstrInfo::isCmp(const MachineInstr *MI) const {
    return ((MI->getOpcode() >= CSA::CMPEQ16) &&
            (MI->getOpcode() <= CSA::CMPUOF64));
}

bool CSAInstrInfo::isMOV(const MachineInstr *MI) const {
  return MI->getOpcode() == CSA::MOV1 ||
    MI->getOpcode() == CSA::MOV8 ||
    MI->getOpcode() == CSA::MOV16 ||
    MI->getOpcode() == CSA::MOV32 ||
    MI->getOpcode() == CSA::MOV64;
}

bool CSAInstrInfo::isAtomic(const MachineInstr *MI) const {
    return MI->getOpcode() >= CSA::ATMADD16 && MI->getOpcode() <= CSA::ATMXOR8;
}

bool CSAInstrInfo::isSeqOT(const MachineInstr *MI) const {
    return ((MI->getOpcode() >= CSA::SEQOTGES16) &&
            (MI->getOpcode() <= CSA::SEQOTNE8));
}

bool CSAInstrInfo::isPure(const MachineInstr *MI) const {
  // TODO: This really should be generated from the tablegen. In theory, it
  // could be the negation of (mayLoad | mayStore | hasUnmodeledSideEffects),
  // but we don't set the latter at the moment, and it's not immediately clear
  // that this is the right flag to use.
  if (isAdd(MI) || isSub(MI) || isMul(MI) || isDiv(MI) || isFMA(MI))
    return true;
  unsigned opcode = MI->getOpcode();
  // ALL0 is the only dataflow operation that is safe.
  if (opcode == CSA::ALL0)
    return true;
  if (isShift(MI) || isCmp(MI))
    return true;
  if (CSA::NOT1 <= opcode && opcode <= CSA::NOT64) return true;
  if (CSA::NEG8 <= opcode && opcode <= CSA::NEG64) return true;
  if (CSA::CTLZ8 <= opcode && opcode <= CSA::CTLZ64) return true;
  if (CSA::CTTZ8 <= opcode && opcode <= CSA::CTTZ64) return true;
  if (CSA::CTPOP8 <= opcode && opcode <= CSA::CTPOP64) return true;
  if (CSA::PARITY8 <= opcode && opcode <= CSA::PARITY64) return true;
  if (CSA::AND1 <= opcode && opcode <= CSA::AND64) return true;
  if (CSA::OR1 <= opcode && opcode <= CSA::OR64) return true;
  if (CSA::XOR1 <= opcode && opcode <= CSA::XOR64) return true;
  if (CSA::ADC8 <= opcode && opcode <= CSA::ADC64) return true;
  if (CSA::SBB8 <= opcode && opcode <= CSA::SBB64) return true;
  if (CSA::SEXT8 <= opcode && opcode <= CSA::SEXT64) return true;
  if (CSA::SLADD8 <= opcode && opcode <= CSA::SLADD64) return true;
  if (CSA::COPY0 <= opcode && opcode <= CSA::COPY64) return true;
  if (CSA::NEGF32 <= opcode && opcode <= CSA::NEGF64) return true;
  if (CSA::ABSF32 <= opcode && opcode <= CSA::ABSF64) return true;

  return false;
}

unsigned CSAInstrInfo::getMemTokenMOVOpcode() const {
  return CSA::MOV0;
}

bool CSAInstrInfo::isMemTokenMOV(const MachineInstr* MI) const {
  return MI->getOpcode() == CSA::MOV0;
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

unsigned CSAInstrInfo::convertCompareOpToSeqOTOp(unsigned cmp_opcode) const {
  CSA::Generic seq_opcode;
  switch (getGenericOpcode(cmp_opcode)) {
  case CSA::Generic::CMPGE: seq_opcode = CSA::Generic::SEQOTGE; break;
  case CSA::Generic::CMPGT: seq_opcode = CSA::Generic::SEQOTGT; break;
  case CSA::Generic::CMPLE: seq_opcode = CSA::Generic::SEQOTLE; break;
  case CSA::Generic::CMPLT: seq_opcode = CSA::Generic::SEQOTLT; break;
  case CSA::Generic::CMPNE: seq_opcode = CSA::Generic::SEQOTNE; break;
  default: return cmp_opcode;
  }
  return adjustOpcode(cmp_opcode, seq_opcode);
}

unsigned CSAInstrInfo::promoteSeqOTOpBitwidth(unsigned seq_opcode,
                                              int bitwidth) const {
  // Pick the larger of the sequence opcode and the bitwidth.
  unsigned licSize = std::max(getLicSize(seq_opcode), (unsigned)bitwidth);
  return makeOpcode(getGenericOpcode(seq_opcode), licSize,
      getOpcodeClass(seq_opcode));
}

bool CSAInstrInfo::convertAddToStrideOp(unsigned add_opcode,
                                        unsigned* strideOpcode) const {
  if (getGenericOpcode(add_opcode) == CSA::Generic::ADD) {
    *strideOpcode = adjustOpcode(add_opcode, CSA::Generic::STRIDE);
    return true;
  }
  return false;
}

bool CSAInstrInfo::convertSubToStrideOp(unsigned sub_opcode,
                                        unsigned* strideOpcode) const {
  if (getGenericOpcode(sub_opcode) == CSA::Generic::SUB) {
    *strideOpcode = adjustOpcode(sub_opcode, CSA::Generic::STRIDE);
    return true;
  }
  return false;
}

bool CSAInstrInfo::negateOpForStride(unsigned strideOpcode,
                                     unsigned* negOpcode) const {
  if (getGenericOpcode(strideOpcode) == CSA::Generic::STRIDE) {
    *negOpcode = adjustOpcode(strideOpcode, CSA::Generic::NEG);
    return true;
  }
  return false;
}

const TargetRegisterClass*
CSAInstrInfo::getStrideInputRC(unsigned strideOpcode) const {
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

bool CSAInstrInfo::convertPickToRepeatOp(unsigned pick_opcode,
                                         unsigned* repeat_opcode) const {
  assert(getGenericOpcode(pick_opcode) == CSA::Generic::PICK &&
      "Input must be a PICK");
  *repeat_opcode = adjustOpcode(pick_opcode, CSA::Generic::REPEAT);
  return true;
}

bool
CSAInstrInfo::isCommutingReductionTransform(const MachineInstr* MI) const {
  using namespace CSA;
  // NOTE: we are leaving out AND1, OR1, and XOR1.
  // We don't have a 1-bit reduction op...
  switch (getGenericOpcode(MI->getOpcode())) {
  case Generic::ADD: case Generic::MUL:
    return true;
  case Generic::AND: case Generic::OR: case Generic::XOR:
    return getLicSize(MI->getOpcode()) > 1;
  default:
    return false;
  }
}

bool CSAInstrInfo::convertTransformToReductionOp(unsigned transform_opcode,
                                             unsigned* reduction_opcode) const {
  CSA::Generic reductGeneric;
  switch (getGenericOpcode(transform_opcode)) {
  case CSA::Generic::FMA: reductGeneric = CSA::Generic::FMSREDA; break;
  case CSA::Generic::ADD: reductGeneric = CSA::Generic::SREDADD; break;
  case CSA::Generic::SUB: reductGeneric = CSA::Generic::SREDSUB; break;
  case CSA::Generic::MUL: reductGeneric = CSA::Generic::SREDMUL; break;
  case CSA::Generic::AND: reductGeneric = CSA::Generic::SREDAND; break;
  case CSA::Generic::OR:  reductGeneric = CSA::Generic::SREDOR;  break;
  case CSA::Generic::XOR: reductGeneric = CSA::Generic::SREDXOR; break;
  default: return false;
  }
  *reduction_opcode = adjustOpcode(transform_opcode, reductGeneric);
  return *reduction_opcode != transform_opcode;
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
