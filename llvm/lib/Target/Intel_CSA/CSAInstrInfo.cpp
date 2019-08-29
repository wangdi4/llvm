//===-- CSAInstrInfo.cpp - CSA Instruction Information --------------------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
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
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

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
static CSA::CondCode GetCondFromBranchOpc(unsigned BrOpc) {
  switch (BrOpc) {
  default:
    return CSA::COND_INVALID;
  case CSA::BT:
    return CSA::COND_T;
  case CSA::BF:
    return CSA::COND_F;
  }
}

// GetCondBranchFromCond - Return the Branch instruction
// opcode that matches the cc.
static unsigned GetCondBranchFromCond(CSA::CondCode CC) {
  switch (CC) {
  default:
    llvm_unreachable("Illegal condition code!");
  case CSA::COND_T:
    return CSA::BT;
  case CSA::COND_F:
    return CSA::BF;
  }
}

// GetOppositeBranchCondition - Return the inverse of the specified condition
static CSA::CondCode GetOppositeBranchCondition(CSA::CondCode CC) {
  switch (CC) {
  default:
    llvm_unreachable("Illegal condition code!");
  case CSA::COND_T:
    return CSA::COND_F;
  case CSA::COND_F:
    return CSA::COND_T;
  }
}

// Pin the vtable to this file.
void CSAInstrInfo::anchor() {}

CSAInstrInfo::CSAInstrInfo(CSASubtarget &STI)
    : CSAGenInstrInfo(CSA::ADJCALLSTACKDOWN, CSA::ADJCALLSTACKUP), RI(*this) {}

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
                                       unsigned SrcReg, bool isKill,
                                       int FrameIdx,
                                       const TargetRegisterClass *RC,
                                       const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (MI != MBB.end())
    DL = MI->getDebugLoc();
  unsigned opc;

  if (RC == &CSA::I0RegClass || RC == &CSA::RI0RegClass ||
      RC == &CSA::CI0RegClass || RC == &CSA::I1RegClass ||
      RC == &CSA::RI1RegClass || RC == &CSA::CI1RegClass ||
      RC == &CSA::I8RegClass || RC == &CSA::RI8RegClass ||
      RC == &CSA::CI8RegClass) {
    opc = CSA::ST8D;
  } else if (RC == &CSA::I16RegClass || RC == &CSA::RI16RegClass ||
             RC == &CSA::CI16RegClass) {
    opc = CSA::ST16D;
  } else if (RC == &CSA::I32RegClass || RC == &CSA::RI32RegClass ||
             RC == &CSA::CI32RegClass) {
    opc = CSA::ST32D;
  } else if (RC == &CSA::I64RegClass || RC == &CSA::RI64RegClass ||
             RC == &CSA::CI64RegClass) {
    opc = CSA::ST64D;
  } else {
    llvm_unreachable("Unknown register class");
  }

  BuildMI(MBB, MI, DL, get(opc))
    .addReg(CSA::IGN, ISSUED_REGSTATE)
    .addFrameIndex(FrameIdx)
    .addImm(0)
    .addReg(SrcReg, getKillRegState(isKill))
    .addImm(CSA::MEMLEVEL_T0)
    .addReg(CSA::IGN);
}

void CSAInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        unsigned DestReg, int FrameIdx,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (MI != MBB.end())
    DL = MI->getDebugLoc();
  unsigned opc;

  if (RC == &CSA::I0RegClass || RC == &CSA::RI0RegClass ||
      RC == &CSA::CI0RegClass || RC == &CSA::I1RegClass ||
      RC == &CSA::RI1RegClass || RC == &CSA::CI1RegClass ||
      RC == &CSA::I8RegClass || RC == &CSA::RI8RegClass ||
      RC == &CSA::CI8RegClass) {
    opc = CSA::LD8D;
  } else if (RC == &CSA::I16RegClass || RC == &CSA::RI16RegClass ||
             RC == &CSA::CI16RegClass) {
    opc = CSA::LD16D;
  } else if (RC == &CSA::I32RegClass || RC == &CSA::RI32RegClass ||
             RC == &CSA::CI32RegClass) {
    opc = CSA::LD32D;
  } else if (RC == &CSA::I64RegClass || RC == &CSA::RI64RegClass ||
             RC == &CSA::CI64RegClass) {
    opc = CSA::LD64D;
  } else {
    llvm_unreachable("Unknown register class");
  }

  BuildMI(MBB, MI, DL, get(opc), DestReg)
    .addReg(CSA::IGN, ISSUED_REGSTATE)
    .addFrameIndex(FrameIdx)
    .addImm(0)
    .addImm(CSA::MEMLEVEL_T0)
    .addReg(CSA::IGN);
}

unsigned CSAInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                    int *BytesAdded) const {
  assert(!BytesAdded && "code size not handled");

  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count                = 0;

  while (I != MBB.begin()) {
    --I;
    if (I->isDebugValue())
      continue;
    if (I->getOpcode() != CSA::BR && I->getOpcode() != CSA::BT &&
        I->getOpcode() != CSA::BF)
      break;
    // Remove the branch.
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }

  return Count;
}

bool CSAInstrInfo::reverseBranchCondition(
  SmallVectorImpl<MachineOperand> &Cond) const {
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
      return true; // Can't handle weird stuff.

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

unsigned CSAInstrInfo::insertBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *TBB,
                                    MachineBasicBlock *FBB,
                                    ArrayRef<MachineOperand> Cond,
                                    const DebugLoc &DL, int *BytesAdded) const {
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
                                  CSA::OpcodeClass opcodeClass) const {
  unsigned startIndex = generic_index_map[static_cast<unsigned>(generic)];
  assert(startIndex != ~0U && "Generic opcode has no valid opcodes");
  for (unsigned index = startIndex;
       generic_to_opcode_map[index].genericOpcode == generic; index++) {
    auto &entry = generic_to_opcode_map[index];
    if (entry.opSize == size && (opcodeClass == CSA::VARIANT_DONTCARE ||
                                 opcodeClass == entry.opClassification)) {
      return entry.opcode;
    }
  }

  return CSA::INVALID_OPCODE;
}

unsigned CSAInstrInfo::adjustOpcode(unsigned oldOpcode,
                                    CSA::Generic opcode,
                                    CSA::OpcodeClass override_class) const {
  // Allow class override.
  override_class =
    (override_class == CSA::VARIANT_DONTCARE) ?
    getOpcodeClass(oldOpcode) : override_class;

  return makeOpcode(opcode, getLicSize(oldOpcode), override_class);
}

unsigned
CSAInstrInfo::getSizeOfRegisterClass(const TargetRegisterClass *RC) const {
  switch (RC->getID()) {
  default:
    llvm_unreachable("Unknown Target register class!");
  case CSA::CI0RegClassID:
  case CSA::I0RegClassID:
  case CSA::RI0RegClassID:
    return 0;
  case CSA::CI1RegClassID:
  case CSA::I1RegClassID:
  case CSA::RI1RegClassID:
    return 1;
  case CSA::CI8RegClassID:
  case CSA::I8RegClassID:
  case CSA::RI8RegClassID:
    return 8;
  case CSA::CI16RegClassID:
  case CSA::I16RegClassID:
  case CSA::RI16RegClassID:
    return 16;
  case CSA::CI32RegClassID:
  case CSA::I32RegClassID:
  case CSA::RI32RegClassID:
    return 32;
  case CSA::CI64RegClassID:
  case CSA::I64RegClassID:
  case CSA::RI64RegClassID:
    return 64;
  }
}

bool CSAInstrInfo::isLoad(const MachineInstr *MI) const {
  switch (getGenericOpcode(MI->getOpcode())) {
  case CSA::Generic::LD:
  case CSA::Generic::LDD:
  case CSA::Generic::LDX:
    return true;
  default:
    return false;
  }
}

bool CSAInstrInfo::isStore(const MachineInstr *MI) const {
  switch (getGenericOpcode(MI->getOpcode())) {
  case CSA::Generic::ST:
  case CSA::Generic::STD:
  case CSA::Generic::STX:
    return true;
  default:
    return false;
  }
}

bool CSAInstrInfo::isMOV(const MachineInstr *MI) const {
  return getGenericOpcode(MI->getOpcode()) == CSA::Generic::MOV;
}

bool CSAInstrInfo::isAtomic(const MachineInstr *MI) const {
  return MI->getOpcode() >= CSA::ATMADD16 && MI->getOpcode() <= CSA::ATMXOR8;
}

bool CSAInstrInfo::isSeqOT(const MachineInstr *MI) const {
  return ((MI->getOpcode() >= CSA::SEQOTGES16) &&
          (MI->getOpcode() <= CSA::SEQOTNE8));
}

bool CSAInstrInfo::isSeqZT(const MachineInstr *MI) const {
  return ((MI->getOpcode() >= CSA::SEQGES16) &&
          (MI->getOpcode() <= CSA::SEQNE8));
}

bool CSAInstrInfo::isReduction(const MachineInstr *MI) const {
  switch (getGenericOpcode(MI->getOpcode())) {
  case CSA::Generic::REDADD:
  case CSA::Generic::REDSUB:
  case CSA::Generic::REDMUL:
  case CSA::Generic::FMREDA:
    return true;
  default:
    return false;
  }
}

unsigned CSAInstrInfo::getMemTokenMOVOpcode() const { return CSA::MOV0; }

bool CSAInstrInfo::isMemTokenMOV(const MachineInstr *MI) const {
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
  CSA::Generic new_generic;
  switch (getGenericOpcode(cmp_opcode)) {
  case CSA::Generic::CMPEQ: // "==" maps to "=="
    new_generic = negate_eq ? CSA::Generic::CMPNE : CSA::Generic::CMPEQ;
    break;
  case CSA::Generic::CMPGE: // ">=" maps to "<"
    new_generic = negate_eq ? CSA::Generic::CMPLT : CSA::Generic::CMPGE;
    break;
  case CSA::Generic::CMPGT: // ">" maps to "<="
    new_generic = negate_eq ? CSA::Generic::CMPLE : CSA::Generic::CMPGT;
    break;
  case CSA::Generic::CMPLE: // "<=" maps to ">"
    new_generic = negate_eq ? CSA::Generic::CMPGT : CSA::Generic::CMPLE;
    break;
  case CSA::Generic::CMPLT: // "<" maps to ">="
    new_generic = negate_eq ? CSA::Generic::CMPGE : CSA::Generic::CMPLT;
    break;
  case CSA::Generic::CMPNE: // "!=" maps to "!="
    new_generic = negate_eq ? CSA::Generic::CMPEQ : CSA::Generic::CMPNE;
    break;
  default:
    new_generic = getGenericOpcode(cmp_opcode);
  }

  switch (new_generic) {
  case CSA::Generic::CMPGE: // ">=" maps to "<="
    new_generic =
      commute_compare_operands ? CSA::Generic::CMPLE : CSA::Generic::CMPGE;
    break;
  case CSA::Generic::CMPGT: // ">" maps to "<"
    new_generic =
      commute_compare_operands ? CSA::Generic::CMPLT : CSA::Generic::CMPGT;
    break;
  case CSA::Generic::CMPLE: // "<=" maps to ">="
    new_generic =
      commute_compare_operands ? CSA::Generic::CMPGE : CSA::Generic::CMPLE;
    break;
  case CSA::Generic::CMPLT: // "<" maps to ">"
    new_generic =
      commute_compare_operands ? CSA::Generic::CMPGT : CSA::Generic::CMPLT;
    break;
  default:
    break;
  }

  return adjustOpcode(cmp_opcode, new_generic);
}

unsigned CSAInstrInfo::convertCompareOpToSeqOTOp(unsigned cmp_opcode) const {
  CSA::Generic seq_opcode;
  switch (getGenericOpcode(cmp_opcode)) {
  case CSA::Generic::CMPGE:
    seq_opcode = CSA::Generic::SEQOTGE;
    break;
  case CSA::Generic::CMPGT:
    seq_opcode = CSA::Generic::SEQOTGT;
    break;
  case CSA::Generic::CMPLE:
    seq_opcode = CSA::Generic::SEQOTLE;
    break;
  case CSA::Generic::CMPLT:
    seq_opcode = CSA::Generic::SEQOTLT;
    break;
  case CSA::Generic::CMPNE:
    seq_opcode = CSA::Generic::SEQOTNE;
    break;
  default:
    return CSA::INVALID_OPCODE;
  }
  return adjustOpcode(cmp_opcode, seq_opcode);
}

unsigned CSAInstrInfo::convertSeqOTToSeqOp(unsigned seqot_opcode) const {
  CSA::Generic seq_opcode;
  switch (getGenericOpcode(seqot_opcode)) {
  case CSA::Generic::SEQOTGE:
    seq_opcode = CSA::Generic::SEQGE;
    break;
  case CSA::Generic::SEQOTGT:
    seq_opcode = CSA::Generic::SEQGT;
    break;
  case CSA::Generic::SEQOTLE:
    seq_opcode = CSA::Generic::SEQLE;
    break;
  case CSA::Generic::SEQOTLT:
    seq_opcode = CSA::Generic::SEQLT;
    break;
  case CSA::Generic::SEQOTNE:
    seq_opcode = CSA::Generic::SEQNE;
    break;
  default:
    return CSA::INVALID_OPCODE;
  }
  return adjustOpcode(seqot_opcode, seq_opcode);
}

unsigned CSAInstrInfo::promoteSeqOTOpBitwidth(unsigned seq_opcode,
                                              int bitwidth) const {
  // Pick the larger of the sequence opcode and the bitwidth.
  unsigned licSize = std::max(getLicSize(seq_opcode), (unsigned)bitwidth);
  return makeOpcode(getGenericOpcode(seq_opcode), licSize,
                    getOpcodeClass(seq_opcode));
}

const TargetRegisterClass *
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

bool CSAInstrInfo::isCommutingReductionTransform(const MachineInstr *MI) const {
  using namespace CSA;
  // NOTE: we are leaving out AND1, OR1, and XOR1.
  // We don't have a 1-bit reduction op...
  switch (getGenericOpcode(MI->getOpcode())) {
  case Generic::ADD:
  case Generic::MUL:
    return true;
  case Generic::AND:
  case Generic::OR:
  case Generic::XOR:
    return getLicSize(MI->getOpcode()) > 1;
  default:
    return false;
  }
}

unsigned
CSAInstrInfo::convertTransformToReductionOp(unsigned transform_opcode) const {

  // Only floating-point/SIMD reduction ops are supported.
  const CSA::OpcodeClass Class = getOpcodeClass(transform_opcode);
  if (Class != CSA::VARIANT_FLOAT and Class != CSA::VARIANT_SIMD)
    return CSA::INVALID_OPCODE;

  CSA::Generic reductGeneric;
  switch (getGenericOpcode(transform_opcode)) {
  case CSA::Generic::FMA:
    reductGeneric = CSA::Generic::FMREDA;
    break;
  case CSA::Generic::ADD:
    reductGeneric = CSA::Generic::REDADD;
    break;
  case CSA::Generic::SUB:
    reductGeneric = CSA::Generic::REDSUB;
    break;
  case CSA::Generic::MUL:
    reductGeneric = CSA::Generic::REDMUL;
    break;
  default:
    return CSA::INVALID_OPCODE;
  }
  return adjustOpcode(transform_opcode, reductGeneric);
}

const TargetRegisterClass *
CSAInstrInfo::getLicClassForSize(unsigned size) const {
  if (size == 0)
    return &CSA::CI0RegClass;
  else if (size == 1)
    return &CSA::CI1RegClass;
  else if (size == 8)
    return &CSA::CI8RegClass;
  else if (size == 16)
    return &CSA::CI16RegClass;
  else if (size == 32)
    return &CSA::CI32RegClass;
  else if (size == 64)
    return &CSA::CI64RegClass;
  llvm_unreachable("Unknown size class");
}

bool CSAInstrInfo::isLICClass(const TargetRegisterClass *RC) const {
  if (!RC)
    return false;
  return RC->getID() == CSA::CI0RegClassID ||
         RC->getID() == CSA::CI1RegClassID ||
         RC->getID() == CSA::CI8RegClassID ||
         RC->getID() == CSA::CI16RegClassID ||
         RC->getID() == CSA::CI32RegClassID ||
         RC->getID() == CSA::CI64RegClassID ||
         RC->getID() == CSA::ANYCRegClassID;
}

const TargetRegisterClass *
CSAInstrInfo::getRegisterClass(unsigned reg,
                               const MachineRegisterInfo &MRI) const {
  if (Register::isVirtualRegister(reg))
    return MRI.getRegClass(reg);

  if (CSA::CI64RegClass.contains(reg)) {
    return &CSA::CI64RegClass;
  } else if (CSA::CI32RegClass.contains(reg)) {
    return &CSA::CI32RegClass;
  }
  if (CSA::CI16RegClass.contains(reg)) {
    return &CSA::CI16RegClass;
  } else if (CSA::CI8RegClass.contains(reg)) {
    return &CSA::CI8RegClass;
  }
  if (CSA::CI1RegClass.contains(reg)) {
    return &CSA::CI1RegClass;
  } else if (CSA::CI0RegClass.contains(reg)) {
    return &CSA::CI0RegClass;
  } else if (CSA::ANYCRegClass.contains(reg)) {
    return &CSA::ANYCRegClass;
  }
  return nullptr;
}
