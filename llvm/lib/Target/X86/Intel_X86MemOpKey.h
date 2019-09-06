//=- Intel_X86MemOpKey.h - A key based on instruction's memory operands. --*- C++ -*-=//
//
//      Copyright (c) 2019-2019 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//===----------------------------------------------------------------------===//
//
// A key based on instruction's memory operands.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_INTEL_X86MEMOPKEY_H
#define LLVM_LIB_TARGET_X86_INTEL_X86MEMOPKEY_H

#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

namespace {

static inline bool isIdenticalOp(const llvm::MachineOperand &MO1,
                                 const llvm::MachineOperand &MO2) {
  return MO1.isIdenticalTo(MO2) &&
         (!MO1.isReg() ||
          !llvm::Register::isPhysicalRegister(MO1.getReg()));
}

#ifndef NDEBUG
static bool isValidDispOp(const llvm::MachineOperand &MO) {
  return MO.isImm() || MO.isCPI() || MO.isJTI() || MO.isSymbol() ||
         MO.isGlobal() || MO.isBlockAddress() || MO.isMCSymbol() || MO.isMBB();
}
#endif

static bool isSimilarDispOp(const llvm::MachineOperand &MO1,
                            const llvm::MachineOperand &MO2) {
  assert(isValidDispOp(MO1) && isValidDispOp(MO2) &&
         "Address displacement operand is not valid");
  return (MO1.isImm() && MO2.isImm()) ||
         (MO1.isCPI() && MO2.isCPI() && MO1.getIndex() == MO2.getIndex()) ||
         (MO1.isJTI() && MO2.isJTI() && MO1.getIndex() == MO2.getIndex()) ||
         (MO1.isSymbol() && MO2.isSymbol() &&
          MO1.getSymbolName() == MO2.getSymbolName()) ||
         (MO1.isGlobal() && MO2.isGlobal() &&
          MO1.getGlobal() == MO2.getGlobal()) ||
         (MO1.isBlockAddress() && MO2.isBlockAddress() &&
          MO1.getBlockAddress() == MO2.getBlockAddress()) ||
         (MO1.isMCSymbol() && MO2.isMCSymbol() &&
          MO1.getMCSymbol() == MO2.getMCSymbol()) ||
         (MO1.isMBB() && MO2.isMBB() && MO1.getMBB() == MO2.getMBB());
}

/// A key based on instruction's memory operands.
class MemOpKey {
public:
  MemOpKey(const llvm::MachineOperand *Base, const llvm::MachineOperand *Scale,
           const llvm::MachineOperand *Index, const llvm::MachineOperand *Segment,
           const llvm::MachineOperand *Disp)
      : Disp(Disp) {
    Operands[0] = Base;
    Operands[1] = Scale;
    Operands[2] = Index;
    Operands[3] = Segment;
  }

  bool operator==(const MemOpKey &Other) const {
    // Addresses' bases, scales, indices and segments must be identical.
    for (int i = 0; i < 4; ++i)
      if (!isIdenticalOp(*Operands[i], *Other.Operands[i]))
        return false;

    // Addresses' displacements don't have to be exactly the same. It only
    // matters that they use the same symbol/index/address. Immediates' or
    // offsets' differences will be taken care of during instruction
    // substitution.
    return isSimilarDispOp(*Disp, *Other.Disp);
  }

  llvm::Register getBaseReg() const { return Operands[0]->getReg(); }
  llvm::Register getIndexReg() const { return Operands[2]->getReg(); }
  llvm::Register getSegmentReg() const { return Operands[3]->getReg(); }

  // Address' base, scale, index and segment operands.
  const llvm::MachineOperand *Operands[4];

  // Address' displacement operand.
  const llvm::MachineOperand *Disp;
};

} // end anonymous namespace


/// Provide DenseMapInfo for MemOpKey.
namespace llvm {

template <> struct DenseMapInfo<MemOpKey> {
  using PtrInfo = DenseMapInfo<const MachineOperand *>;

  static inline MemOpKey getEmptyKey() {
    return MemOpKey(PtrInfo::getEmptyKey(), PtrInfo::getEmptyKey(),
                    PtrInfo::getEmptyKey(), PtrInfo::getEmptyKey(),
                    PtrInfo::getEmptyKey());
  }

  static inline MemOpKey getTombstoneKey() {
    return MemOpKey(PtrInfo::getTombstoneKey(), PtrInfo::getTombstoneKey(),
                    PtrInfo::getTombstoneKey(), PtrInfo::getTombstoneKey(),
                    PtrInfo::getTombstoneKey());
  }

  static unsigned getHashValue(const MemOpKey &Val) {
    // Checking any field of MemOpKey is enough to determine if the key is
    // empty or tombstone.
    assert(Val.Disp != PtrInfo::getEmptyKey() && "Cannot hash the empty key");
    assert(Val.Disp != PtrInfo::getTombstoneKey() &&
           "Cannot hash the tombstone key");

    hash_code Hash = hash_combine(*Val.Operands[0], *Val.Operands[1],
                                  *Val.Operands[2], *Val.Operands[3]);

    // If the address displacement is an immediate, it should not affect the
    // hash so that memory operands which differ only be immediate displacement
    // would have the same hash. If the address displacement is something else,
    // we should reflect symbol/index/address in the hash.
    switch (Val.Disp->getType()) {
    case MachineOperand::MO_Immediate:
      break;
    case MachineOperand::MO_ConstantPoolIndex:
    case MachineOperand::MO_JumpTableIndex:
      Hash = hash_combine(Hash, Val.Disp->getIndex());
      break;
    case MachineOperand::MO_ExternalSymbol:
      Hash = hash_combine(Hash, Val.Disp->getSymbolName());
      break;
    case MachineOperand::MO_GlobalAddress:
      Hash = hash_combine(Hash, Val.Disp->getGlobal());
      break;
    case MachineOperand::MO_BlockAddress:
      Hash = hash_combine(Hash, Val.Disp->getBlockAddress());
      break;
    case MachineOperand::MO_MCSymbol:
      Hash = hash_combine(Hash, Val.Disp->getMCSymbol());
      break;
    case MachineOperand::MO_MachineBasicBlock:
      Hash = hash_combine(Hash, Val.Disp->getMBB());
      break;
    default:
      llvm_unreachable("Invalid address displacement operand");
    }

    return (unsigned)Hash;
  }

  static bool isEqual(const MemOpKey &LHS, const MemOpKey &RHS) {
    // Checking any field of MemOpKey is enough to determine if the key is
    // empty or tombstone.
    if (RHS.Disp == PtrInfo::getEmptyKey())
      return LHS.Disp == PtrInfo::getEmptyKey();
    if (RHS.Disp == PtrInfo::getTombstoneKey())
      return LHS.Disp == PtrInfo::getTombstoneKey();
    return LHS == RHS;
  }
};

} // end namespace llvmi

#endif // LLVM_LIB_TARGET_X86_INTEL_X86MEMOPKEY_H
