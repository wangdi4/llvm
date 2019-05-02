//===------ HIRReroll.h - Interface for common reroll utilities *-- C++ --*---//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file lists shared reroll utilities.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRREROLL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRREROLL_H

#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"

namespace llvm {

class LoopOptReportBuilder;

namespace loopopt {

class HLLoop;

namespace reroll {

// Hard to estimate the size: not using small vector.
typedef std::vector<const CanonExpr *> VecCEsTy;
typedef std::vector<const RegDDRef *> VecRefsTy;

class CEOpSequence {
  typedef unsigned PositionTy;
  typedef unsigned OpcodeTy;
  typedef std::pair<PositionTy, OpcodeTy> PosOpcodeTy;
  typedef std::vector<PosOpcodeTy> VecOpcodesTy;

  int NumDDRefs;
  void countRefs() { NumDDRefs++; }
  void add(PositionTy Pos, OpcodeTy Opcode) {
    Opcodes.emplace_back(Pos, Opcode);
  }

public:
  CEOpSequence() : NumDDRefs(0) {}

  VecCEsTy CEList;
  VecOpcodesTy Opcodes;
  VecRefsTy MemRefs;

  void add(const RegDDRef *Ref) {
    if (Ref->hasGEPInfo()) {
      CEList.push_back(Ref->getBaseCE());
      if (MemRefs.empty() || Ref != MemRefs.back()) {
        MemRefs.push_back(Ref);
      }
    }
    for (const CanonExpr *CE :
         make_range(Ref->canon_begin(), Ref->canon_end())) {
      CEList.push_back(CE);
    }
    countRefs();
  }
  void add(const CanonExpr *CE) { CEList.push_back(CE); }
  unsigned size() const { return CEList.size(); }
  unsigned opSize() const { return Opcodes.size(); }
  unsigned numRefs() const { return NumDDRefs; }
  void addOpcodeToSeq(unsigned OpCode) { add(CEList.size(), OpCode); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printCEs() {
    for (auto CE : CEList) {
      CE->dump();
      dbgs() << " ";
    }
    dbgs() << "\n";
  }
#endif
};

/// Debuging related dump functions
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
std::string getOpcodeString(unsigned Opcode);
LLVM_DUMP_METHOD void dumpOprdOpSequence(const HLInst *HInst,
                                         const CEOpSequence &Seq,
                                         bool Detail = false);
#endif

namespace rerollcomparator {

bool blobIndexLess(unsigned BI1, unsigned BI2);

struct BlobDDRefLess {
  bool operator()(const BlobDDRef *B1, const BlobDDRef *B2) {
    return blobIndexLess(B1->getSingleCanonExpr()->getSingleBlobIndex(),
                         B2->getSingleCanonExpr()->getSingleBlobIndex());
  }
};

struct RegDDRefLess {
  bool operator()(const RegDDRef *R1, const RegDDRef *R2) {
    bool IsMemRef1 = R1->isMemRef();
    bool IsMemRef2 = R2->isMemRef();

    if (IsMemRef1) {
      return IsMemRef2 ? DDRefUtils::compareMemRef(R1, R2) : false;
    }

    if (IsMemRef2) {
      return true;
    }

    // Neither is MemRef just use symbase
    return R1->getSymbase() < R2->getSymbase();
  }
};
} // namespace rerollcomparator

} // namespace reroll
} // namespace loopopt
} // namespace llvm

#endif /* LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRREROLL_H */
