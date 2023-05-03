//===------ HIRVecIdioms.h ----------------------------------*-- C++ --*---===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// The file contains interface of vector idioms storage and its specialization
// for HIR data types.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_LOOPANALYSIS_VECIDIOMS_H
#define INTEL_LOOPANALYSIS_VECIDIOMS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PointerUnion.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"

namespace llvm {
namespace loopopt {

class HLInst;
class DDRef;

// Class to keep a list of vector idioms. The idiom is IdiomSubject marked with
// a IdiomId from a predefined list. Each idiom can have a list of linked
// idioms. For printing purposes, the IdiomId-s are split into master idioms,
// standalone idioms, and others.
template <typename IdiomSubject, typename IdiomTraits> class VectorIdioms {
public:
  enum IdiomId {
    NoIdiom = 0,
    // Min or Max main instruction in minmax+index idiom.
    MinOrMax,
    // Index instructions of minmax+index idiom.
    // Monotonic index, the last value can be calculated in one step.
    MMFirstLastIdx,
    // Non-monotonic value, the last value calculation requires a MMFirstLastIdx
    // to be present and uses its last value for final calculation.
    MMFirstLastVal,
    VConflictLikeStore,
    // Compress/expand IdiomIds
    // E.g. in the following loop
    // double *A,*B,*D,*E; int *C;
    // for (int i=0; i<size; ++i)
    //   if (C[i] != 0) {
    //     B[j] = A[i];
    //     j++; // 1st increment
    //     E[i] = D[j];
    //     j++; // 2nd increment
    //   }
    //
    // The statement commented as "1st increment" is marked as CEIndexIncFirst.
    // The statement commented as "2nd increment" is marked as CEIndexIncNext.
    // I.e. the first found index increment is CEIndexIncFirst all other
    // increments are CEIndexIncNext.
    // The statement with the store to B[j] is marked as CEStore.
    // The load from D[j] is marked as CELoad.
    // The 'j' in D[j] and in B[j] are marked as CELdStIndex.
    //
    // Main entry for idiom, first encountered index increment (HLInst)
    CEIndexIncFirst,
    // Additional index increments, are linked to the main entry (HLInst)
    CEIndexIncNext,
    // Compress store, linked to the main entry (HLInst)
    CEStore,
    // Expand load, linked to the main entry (DDRef)
    CELoad,
    // Index in the store/load, linked to a store or to a load (DDRef)
    CELdStIndex,
  };

private:
  using IdiomListTy = MapVector<IdiomSubject, IdiomId>;
  using LinkedIdiomListTy = SetVector<IdiomSubject>;
  using IdiomLinksTy = DenseMap<IdiomSubject, LinkedIdiomListTy>;
  SmallDenseMap<const HLInst *, DDRef *> VConflictStoreToLoadMap;

public:
  using iterator = typename IdiomListTy::iterator;
  using const_iterator = typename IdiomListTy::const_iterator;

  VectorIdioms() = default;
  VectorIdioms(const VectorIdioms &) = delete;
  VectorIdioms &operator=(const VectorIdioms &) = delete;

  /// Mark \p ISubj with IdiomId \p Id and put it to the storage.
  void addIdiom(IdiomSubject ISubj, IdiomId Id) {
    assert(Id != NoIdiom && "Expected idiom");
    iterator Iter = IdiomData.find(ISubj);
    if (Iter != IdiomData.end())
      assert(Iter->second == Id && "Conflicting idiom");
    else {
      assert(IdiomTraits::isValidIdForIdiom(ISubj, Id) &&
             "invalid idiom markup");
      IdiomData[ISubj] = Id;
    }
  }

  /// Return IdiomId if \p ISubj is registered as idiom or NoIdiom if
  /// it is not.
  IdiomId isIdiom(IdiomSubject ISubj) const {
    const_iterator Iter = IdiomData.find(ISubj);
    return Iter == IdiomData.end() ? NoIdiom : Iter->second;
  }

  /// Return true if \p ISubj is registered as a compress/expand idiom.
  bool isCEIdiom(IdiomSubject ISubj) const {
    IdiomId Id = isIdiom(ISubj);
    return Id == CEIndexIncFirst || Id == CEIndexIncNext || Id == CEStore ||
           Id == CELoad || Id == CELdStIndex;
  }

  /// Add \p Linked as an idiom marked with \p Id, and link it to \p Master.
  void addLinked(IdiomSubject Master, IdiomSubject Linked, IdiomId Id) {
    addIdiom(Linked, Id);
    insertLinked(Master, Linked, Id);
  }

  /// Link already inserted idioms.
  void linkIdiom(IdiomSubject Master, IdiomSubject Linked, IdiomId Id) {
    assert((Id != NoIdiom && isIdiom(Linked) == Id) &&
           "Expected linked idiom registered");
    insertLinked(Master, Linked, Id);
  }

  const_iterator begin() const { return IdiomData.begin(); }
  const_iterator end() const { return IdiomData.end(); }

  auto getIdiomsById(IdiomId Id) const {
    return make_filter_range(make_range(begin(), end()),
                             [Id](const auto &V) { return V.second == Id; });
  }

  /// Return list of idioms linked to \p Master
  const LinkedIdiomListTy *getLinkedIdioms(IdiomSubject Master) const {
    auto Iter = IdiomLinks.find(Master);
    return Iter == IdiomLinks.end() ? nullptr : &Iter->second;
  }

  /// Predicate whether \p Id means a standalone idiom.
  static bool isStandaloneIdiom(IdiomId Id) { return false; }

  /// Predicate whether \p Id marks idioms that require the linked ones.
  static bool isMasterIdiom(IdiomId Id) {
    return Id == MinOrMax || Id == VConflictLikeStore || Id == CEIndexIncFirst;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const {
    raw_ostream &OS = dbgs();
    OS << "Idiom List\n";
    if (IdiomData.empty()) {
      OS << "  No idioms detected.\n";
      return;
    }
    SmallSet<IdiomSubject, 4> Printed;
    std::function<void(raw_ostream &, IdiomSubject, int Indent)>
        DumpLinkedIdioms = [this, &Printed,
                            &DumpLinkedIdioms](raw_ostream &OS,
                                              IdiomSubject Idiom, int Indent) {
          if (const LinkedIdiomListTy *LinkedList = getLinkedIdioms(Idiom)) {
            std::string StrIndent = std::string(2 * Indent, ' ');
            for (auto Linked : *LinkedList) {
              auto IdiomCode = isIdiom(Linked);
              OS << StrIndent << getIdiomName(IdiomCode) << ": ";
              Linked->dump(OS);
              if (Printed.insert(Linked).second)
                DumpLinkedIdioms(OS, Linked, Indent + 1);
            }
          }
        };
    for (auto &Idiom : IdiomData)
      if (isMasterIdiom(Idiom.second) || isStandaloneIdiom(Idiom.second)) {
        if (!Printed.insert(Idiom.first).second)
          continue;
        OS << getIdiomName(Idiom.second) << ": ";
        Idiom.first->dump(OS);
        DumpLinkedIdioms(OS, Idiom.first, 1);
      }
  }

  static const char *getIdiomName(IdiomId Id) {
    switch (Id) {
    case NoIdiom:
      return "NoIdiom";
    case MinOrMax:
      return "MinOrMax";
    case MMFirstLastIdx:
      return "MMFirstLastIdx";
    case MMFirstLastVal:
      return "MMFirstLastVal";
    case VConflictLikeStore:
      return "VConflictLikeStore";
    case CEIndexIncFirst:
      return "CEIndexIncFirst";
    case CEIndexIncNext:
      return "CEIndexIncNext";
    case CEStore:
      return "CEStore";
    case CELoad:
      return "CELoad";
    case CELdStIndex:
      return "CELdStIndex";
    };
  }
#endif

  void recordVConflictIdiom(const HLInst *StoreInst, DDRef *LoadRef) {
    // Add the root of VConflict idiom (StoreInst) in idiom list.
    addIdiom(StoreInst, VectorIdioms::VConflictLikeStore);
    // Add load and store of VConflict idiom in VConflictStoreToLoadMap.
    VConflictStoreToLoadMap[StoreInst] = LoadRef;
  }

  DDRef *getVConflictLoad(const HLInst *StoreInst) const {
    return VConflictStoreToLoadMap.find(StoreInst)->second;
  }

  bool isVConflictLoad(DDRef *LoadRef) const {
    return llvm::find_if(VConflictStoreToLoadMap, [LoadRef](const auto &Pair) {
             return Pair.second == LoadRef;
           }) != VConflictStoreToLoadMap.end();
  }

  static bool isIncrementInst(const HLInst *Inst, int64_t &Stride) {

    if (isPointerIncrementInst(Inst, Stride))
      return true;

    if (Inst->getLLVMInstruction()->getOpcode() != Instruction::Add)
      return false;

    const RegDDRef *LhsDDRef = Inst->getLvalDDRef();
    assert(LhsDDRef && "Add instruction expected to have non-null lval DDRef");
    if (LhsDDRef->isMemRef() || !LhsDDRef->getSrcType()->isIntegerTy())
      return false;

    assert(Inst->getNumOperands() == 3 &&
           "Add instruction expected to have 3 operands");
    const RegDDRef *Op0 = Inst->getOperandDDRef(0);
    const RegDDRef *Op1 = Inst->getOperandDDRef(1);
    const RegDDRef *Op2 = Inst->getOperandDDRef(2);
    if (!Op1->isTerminalRef() || !Op2->isTerminalRef())
      return false;

    CanonExprUtils &CEU = Op0->getCanonExprUtils();
    CanonExpr *SumCE =
        CEU.cloneAndAdd(Op1->getSingleCanonExpr(), Op2->getSingleCanonExpr());
    if (!SumCE)
      return false;

    CanonExpr *StrideCE =
        CEU.cloneAndSubtract(SumCE, Op0->getSingleCanonExpr());
    if (!StrideCE)
      return false;

    // TODO: relax the restriction to check for invariant strides.
    return StrideCE->isIntConstant(&Stride);
  }

  static bool isPointerIncrementInst(const HLInst *Inst, int64_t &Stride) {

    const RegDDRef *RVal = Inst->getRvalDDRef();
    if (!RVal || !RVal->isAddressOf())
      return false;

    const RegDDRef *LVal = Inst->getLvalDDRef();
    if (!LVal->isSelfBlob())
      return false;

    const CanonExpr *BaseCE = RVal->getBaseCE();
    if (!BaseCE->containsStandAloneBlob(LVal->getSelfBlobIndex()))
      return false;

    if (RVal->getNumDimensions() != 1)
      return false;

    const CanonExpr *CE = RVal->getSingleCanonExpr();
    return CE->isIntConstant(&Stride);
  }

private:
  void insertLinked(IdiomSubject Master, IdiomSubject Linked, IdiomId Id) {
    assert(isValidLink(isIdiom(Master), Id) && "Invalid idiom linking");
    IdiomLinks[Master].insert(Linked);
  }

  // Return true if idiom with \p Id can be linked to idiom with \p MainId.
  static bool isValidLink(IdiomId MainId, IdiomId Id) {
    switch (MainId) {
    case MinOrMax:
      return Id == MMFirstLastIdx || Id == MMFirstLastVal;
    case VConflictLikeStore:
      return false;
    case CEIndexIncFirst:
      return Id == CEIndexIncNext || Id == CEStore || Id == CELoad;
    case CEStore:
    case CELoad:
      return Id == CELdStIndex;
    case NoIdiom:
    default:
      return false;
    }
  }

  IdiomListTy IdiomData;
  IdiomLinksTy IdiomLinks;
};

// HIR vector idiom can represent either HLInst or DDRef.
struct HIRVecIdiom
    : public PointerUnion<const HLInst *, const DDRef *, const CanonExpr *> {
  using Base = PointerUnion<const HLInst *, const DDRef *, const CanonExpr *>;
  const HIRVecIdiom *operator->() const { return this; }
  HIRVecIdiom(const HLInst *H) { Base::operator=(H); }
  HIRVecIdiom(const DDRef *D) { Base::operator=(D); }
  HIRVecIdiom(const CanonExpr *C) { Base::operator=(C); }
  HIRVecIdiom(const Base &U) : Base(U) {}

  operator const HLInst *() const { return get<const HLInst *>(); }
  operator const DDRef *() const { return get<const DDRef *>(); }
  operator const CanonExpr *() const { return get<const CanonExpr *>(); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(raw_ostream &OS) const;
#endif
};

class HIRVectorIdiomTraits {
  using Base = VectorIdioms<HIRVecIdiom, HIRVectorIdiomTraits>;

public:
  // Some of idioms can be HLInst and some of them can be DDRef. Return true if
  // \p Id can be assigned to the \p Idiom, depending on \p Idiom's class.
  static bool isValidIdForIdiom(HIRVecIdiom Idiom, Base::IdiomId Id) {
    switch (Id) {
    case Base::NoIdiom:
      return false;
    case Base::MinOrMax:
    case Base::MMFirstLastIdx:
    case Base::MMFirstLastVal:
    case Base::VConflictLikeStore:
    case Base::CEIndexIncFirst:
    case Base::CEIndexIncNext:
    case Base::CEStore:
      return Idiom.is<const HLInst *>();
    case Base::CELoad:
      return Idiom.is<const DDRef *>();
    case Base::CELdStIndex:
      return Idiom.is<const CanonExpr *>();
    }
    llvm_unreachable("unexpected IdiomId");
    return false;
  }
};

using HIRVectorIdioms = VectorIdioms<HIRVecIdiom, HIRVectorIdiomTraits>;

} // namespace loopopt

// DenseMap requires specialization
template <>
struct DenseMapInfo<loopopt::HIRVecIdiom>
    : public DenseMapInfo<
          PointerUnion<const loopopt::HLInst *, const loopopt::DDRef *,
                       const loopopt::CanonExpr *>> {
  using Base =
      DenseMapInfo<PointerUnion<const loopopt::HLInst *, const loopopt::DDRef *,
                                const loopopt::CanonExpr *>>;

  static inline loopopt::HIRVecIdiom getEmptyKey() {
    return Base::getEmptyKey();
  }

  static inline loopopt::HIRVecIdiom getTombstoneKey() {
    return Base::getTombstoneKey();
  }

  static unsigned getHashValue(const loopopt::HIRVecIdiom &UnionVal) {
    return Base::getHashValue(UnionVal);
  }

  static bool isEqual(const loopopt::HIRVecIdiom &LHS,
                      const loopopt::HIRVecIdiom &RHS) {
    return Base::isEqual(LHS, RHS);
  }
};

} // namespace llvm

#endif // INTEL_LOOPANALYSIS_VECIDIOMS_H
