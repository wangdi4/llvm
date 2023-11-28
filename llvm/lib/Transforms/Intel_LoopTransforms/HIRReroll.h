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

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"

#include <stack>

namespace llvm {

class OptReportBuilder;

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
  void printCEs() const {
    for (auto CE : CEList) {
      CE->dump(1);
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

typedef std::vector<HLNode *> VecNodesTy;
typedef std::vector<const DDRef *> VecDDRefsTy;

typedef std::stack<const DDRef *, VecDDRefsTy> TempStackTy;

/// Expands CEOpSequence by tracing defs of a temp.
/// Maintains covered instructions met during expansion.
///
/// Example:
/// %n = ...
/// DO
///   %1  = B[2*i];       -- (1)
///   A[2*i] = %n * %1;   -- (2)
///
/// END DO
///
/// Inst (2) is a Store inst. This function collects
///  - its LVAL reference, A[2*i]
///  - its RVAL references
///     In case temp refs are present, it tracks the instruction defining
///     that temp. Then adds non-temp DDRefs to list. If another temp ref
///     is found, this process repeats. If that temp is defined outside the
///     loop interest (linear at innermost level), the repetition stop.
///     Thus, for the example code above, %n, B[2*i], are collected.
/// The final CEOpSequence will be {A[2*i], %n * %1, B[2*i]} and {("*",1)}.
/// "*" means mul operator at index 1.
template <typename BuilderTy, typename ContainerTy> class SequenceBuilder {
public:
  explicit SequenceBuilder(const ContainerTy *Container, DDGraph &G,
                           CEOpSequence &Seq, VecNodesTy &IL)
      : SequenceBuilder(nullptr, Container, G, Seq, IL) {}

  explicit SequenceBuilder(const TempStackTy *S, const ContainerTy *Container,
                           DDGraph &G, CEOpSequence &Seq, VecNodesTy &IL)
      : Container(Container), DDG(G), Seq(Seq), InstList(IL) {
    if (S) {
      TempStack = *S;
    }
  }

  /// Given a ref, extend CEOpSequence
  /// and replenish stack as needed.
  bool processRegDDRef(const RegDDRef *Ref) {

    if (Ref->isConstant()) {
      Seq.add(Ref);
      return true;
    }

    if (Ref->isSelfBlob()) {
      if (static_cast<BuilderTy *>(this)->stopTrackingTemp(Ref)) {
        // Output it to RefList and than return.
        Seq.add(Ref);
      } else if (pushIntoStack(Ref)) {

        if (VisitedRefSet.count(Ref)) {
          return false;
        }
        // If Ref is not linear-at-level then push it to stack and return.
        TempStack.push(Ref);
        VisitedRefSet.insert(Ref);
      }
      return true;
    }

    Seq.add(Ref);

    // Push non-LinearAtLevel temps among blob ddrefs into stack.
    // Check for LiveIn is used instead of isLinearAtLevel because
    // - Usually !isLiveIn implies !isLinearAtLevel
    // - If not, e.g. "K = 5" in the innermost loop, chances are
    //   the innermost loop is not a reroll pattern.
    // - Most importantly, current algorithm for straight line codes
    //   does not handle reductions, i.e. live-in but non-loop-invariant
    //   temps.
    //
    // Sort the blobs to handle a case like
    //   %1  = B[2*i];
    //   A[2*i] = %n * %1;
    //          blob ddrefs are in the order of %n, %1.
    //   %3  = B[2*i+1];
    //   A[2*i+1] = %n * %3;
    //          blob ddrefs are in the order of %3, %n.
    // Sort blobs to make both have
    //     %n, %1
    //     %n, %3
    SmallVector<const BlobDDRef *, 8> Blobs;
    const ContainerTy *Container = this->Container;
    std::copy_if(Ref->blob_begin(), Ref->blob_end(), std::back_inserter(Blobs),
                 [Container](const BlobDDRef *Blob) {
                   return !Container->isLiveIn(Blob->getSymbase());
                 });
    std::sort(Blobs.begin(), Blobs.end(), rerollcomparator::BlobDDRefLess());

    for (const DDRef *Blob : Blobs) {

      if (VisitedRefSet.count(Blob)) {
        return false;
      }

      TempStack.push(Blob);
      VisitedRefSet.insert(Blob);
    }

    return true;
  }

  /// Track the temp at the top and process RHS of the defining inst.
  bool trackTemps() {

    while (!TempStack.empty()) {
      const DDRef *TempRef = TempStack.top();
      TempStack.pop();

      assert(TempRef->isSelfBlob());

      HLInst *DefInst = static_cast<BuilderTy *>(this)->findTempDef(TempRef);
      if (!DefInst) {
        // DefInst should exists within the
        // loop when a temp is not loop-invariant. Otherwise, we don't know how
        // to reroll. Just bail out.
        //
        // + DO i1 = 0, %len.18.lcssa + -1, 1
        //   %cuv = %cuv  + trunc.i32.i1((%bits /u 128));
        //   %bits = %bits  <<  1;
        // + END LOOP
        // %bits --> %bits ANTI (=) (0)
        return false;
      }

      assert((isa<HLLoop>(Container) &&
              !TempRef->getSingleCanonExpr()->isLinearAtLevel(
                  Container->getNodeLevel())) ||
             (isa<HLRegion>(Container) &&
              !Container->isLiveIn(TempRef->getSymbase())));

      InstList.push_back(DefInst);
      processOpcode(DefInst);

      // Push Def's RHS, no LVAL
      SmallVector<const RegDDRef *, 4> ChildrenRvalDDRefs;
      preprocessRvals(DefInst, ChildrenRvalDDRefs);
      for (const RegDDRef *ChildDDRef :
           make_range(ChildrenRvalDDRefs.begin(), ChildrenRvalDDRefs.end())) {
        // TODO: Forward typecast in a CE?
        if (!processRegDDRef(ChildDDRef))
          return false;
      }
    }
    return true;
  }

private:
  void preprocessRvals(const HLInst *DefInst,
                       SmallVectorImpl<const RegDDRef *> &ChildrenRvals) const {
    std::copy(DefInst->rval_op_ddref_begin(), DefInst->rval_op_ddref_end(),
              std::back_inserter(ChildrenRvals));

    std::sort(ChildrenRvals.begin(), ChildrenRvals.end(),
              rerollcomparator::RegDDRefLess());
  }

  void processOpcode(const HLInst *DefInst) {
    // TODO: Predicate handle for icmp/select
    Seq.addOpcodeToSeq(DefInst->getLLVMInstruction()->getOpcode());
  }

  // Currently, the same logic is used both for Loop and Region.
  // Might be differentiated later on.
  bool pushIntoStack(const RegDDRef *Ref) const {
    return (!Container->isLiveIn(Ref->getSymbase()));
  }

  TempStackTy TempStack;
  std::set<const DDRef*> VisitedRefSet; // set of refs ever entered into stack.

protected:
  // Loop level where the seed is found
  const ContainerTy *Container;

  DDGraph &DDG;

  // Seq of CEs and OpCodes starting from a seed.
  // A seed is
  //  - RVAL DDRef of a store inst
  //  - Terms that are not reduction variable of a reduction inst. (float add)
  //  - SCEVs of a self reduction inst. (e.g. (%1*%2) or (%3*%4)
  CEOpSequence &Seq;

  // List of Insts that are visited for this Seq.
  VecNodesTy &InstList;
};

/// Information about a seed Instruction from which temp tracking starts.
/// Base type is for StoreInst.
struct SeedInfo {
  /// Instruction that contains a seed.
  /// Store-instruction only for now.
  HLNode *ContainingInst;

  /// A set of instructions tracked from ContainingInst.
  /// It includes ContainingInst also.
  VecNodesTy TrackedUpwardInsts;

  SeedInfo(HLNode *SeedInst) : ContainingInst(SeedInst) {
    TrackedUpwardInsts.push_back(SeedInst);
  }
};

typedef SmallVector<SeedInfo, 4> VecSeedInfoTy;
typedef std::vector<CEOpSequence> VecCEOpSeqTy;

template <typename BuilderTy, typename ContainerTy>
bool extendSeq(const RegDDRef *StartRef, const ContainerTy *Container,
               DDGraph &DDG, CEOpSequence &Seq, VecNodesTy &InstList) {

  BuilderTy Builder(Container, DDG, Seq, InstList);
  if (!Builder.processRegDDRef(StartRef))
    return false;

  return Builder.trackTemps();
}

/// RHS of Store is the children to push to the stack.
/// Children ddrefs of a ddref in this context
/// are ddrefs in the right hand side of the inst which
/// defines this ddref.
/// Example:
/// %m = A[i] + %q; -- (2)
///    = %m ..      -- (1)
/// Child of %m at (1) are A[i] and %q in (2)
/// Return the number of sequences: 1 if seccessful, otherwise, 0.
template <typename BuilderTy, typename ContainerTy>
bool buildFromStoreInst(HLInst *HInst, const ContainerTy *Container,
                        DDGraph &DDG, VecCEOpSeqTy &VecSeq,
                        VecSeedInfoTy &VecSeedInfo) {
  // Lval of Store is the root
  const RegDDRef *LVal = HInst->getLvalDDRef();

  if (LVal->hasTrailingStructOffsets()) {
    return false;
  }

  VecSeq.push_back(CEOpSequence());
  VecSeedInfo.push_back(SeedInfo(HInst));
  if (!extendSeq<BuilderTy, ContainerTy>(
          LVal, Container, DDG, VecSeq.back(),
          VecSeedInfo.back().TrackedUpwardInsts)) {
    return false;
  }

  const RegDDRef *RVal = HInst->getRvalDDRef();

  if (!extendSeq<BuilderTy, ContainerTy>(
          RVal, Container, DDG, VecSeq.back(),
          VecSeedInfo.back().TrackedUpwardInsts)) {
    return false;
  }

  return true;
}

} // namespace reroll
} // namespace loopopt
} // namespace llvm

#endif /* LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRREROLL_H */
