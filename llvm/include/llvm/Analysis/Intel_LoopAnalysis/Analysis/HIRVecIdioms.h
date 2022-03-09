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

namespace llvm {
namespace loopopt {

class HLInst;
class DDRef;

// Class to keep a list of vector idioms. The idiom is IdiomSubject marked with
// a IdiomId from a predefined list. Each idiom can have a list of linked
// idioms. For printing purposes, the IdiomId-s are split into master idioms,
// standalone idioms, and others.
template <typename IdiomSubject> class VectorIdioms {
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
    else
      IdiomData[ISubj] = Id;
  }

  /// Return IdiomId if \p ISubj is registered as idiom or NoIdiom if
  /// it is not.
  IdiomId isIdiom(IdiomSubject ISubj) const {
    const_iterator Iter = IdiomData.find(ISubj);
    return Iter == IdiomData.end() ? NoIdiom : Iter->second;
  }

  /// Add \p Linked as an idiom marked with \p Id, and link it to \p Master.
  void addLinked(IdiomSubject Master, IdiomSubject Linked, IdiomId Id) {
    assert(isIdiom(Master) != NoIdiom && "Expected master idiom registered");
    addIdiom(Linked, Id);
    IdiomLinks[Master].insert(Linked);
  }

  /// Link already inserted idioms.
  void linkIdiom(IdiomSubject Master, IdiomSubject Linked, IdiomId Id) {
    assert(isIdiom(Master) != NoIdiom && "Expected master idiom registered");
    assert((Id != NoIdiom && isIdiom(Linked) == Id) &&
           "Expected linked idiom registered");
    IdiomLinks[Master].insert(Linked);
  }

  const_iterator begin() const { return IdiomData.begin(); }
  const_iterator end() const { return IdiomData.end(); }

  /// Return list of idioms linked to \p Master
  const LinkedIdiomListTy *getLinkedIdioms(IdiomSubject Master) const {
    auto Iter = IdiomLinks.find(Master);
    return Iter == IdiomLinks.end() ? nullptr : &Iter->second;
  }

  /// Predicate whether \p Id means a standalone idiom.
  static bool isStandaloneIdiom(IdiomId Id) { return false; }

  /// Predicate whether \p Id marks idioms that require the linked ones.
  static bool isMasterIdiom(IdiomId Id) { return Id == MinOrMax; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const {
    raw_ostream &OS = dbgs();
    OS << "Idiom List\n";
    if (IdiomData.empty()) {
      OS << "  No idioms detected.\n";
      return;
    }
    for (auto &Idiom : IdiomData)
      if (isMasterIdiom(Idiom.second) || isStandaloneIdiom(Idiom.second)) {
        OS << getIdiomName(Idiom.second) << ": ";
        Idiom.first->dump();
        if (const LinkedIdiomListTy *LinkedList = getLinkedIdioms(Idiom.first))
          for (auto Linked : *LinkedList) {
            auto IdiomCode = isIdiom(Linked);
            OS << "  " << getIdiomName(IdiomCode) << ": ";
            Linked->dump();
          }
      }
  }

  static const char *getIdiomName(IdiomId Id) {
    static const char *Names[] = {"NoIdiom", "MinOrMax", "MMFirstLastIdx",
                                  "MMFirstLastVal", "VConflictLikeStore"};
    return Names[Id];
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

private:
  IdiomListTy IdiomData;
  IdiomLinksTy IdiomLinks;
};

// HIR vector idiom can represent either HLInst or DDRef.
struct HIRVecIdiom : public PointerUnion<const HLInst *, const DDRef *> {
  using Base = PointerUnion<const HLInst *, const DDRef *>;
  using HI = const HLInst *;
  using DD = const DDRef *;

  const HIRVecIdiom *operator->() const { return this; }
  HIRVecIdiom(const HLInst *H) { Base::operator=(H); }
  HIRVecIdiom(const DDRef *D) { Base::operator=(D); }
  HIRVecIdiom(const Base &U) : Base(U) {}

  operator const HLInst *() const { return get<HI>(); }
  operator const DDRef *() const { return get<DD>(); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
#endif
};

using HIRVectorIdioms = VectorIdioms<HIRVecIdiom>;

// Deleter, to use with std::unique_ptr<HIRVectorIdioms> when this header is
// not included and HIRVectorIdioms is forward declared as incomplete.
// This declaration is unnecessary here, keeping just to able to copy it in the
// needed place.
extern void deleteHIRVectorIdioms(HIRVectorIdioms *);

} // namespace loopopt

// DenseMap requires specialization
template <>
struct DenseMapInfo<loopopt::HIRVecIdiom>
    : public DenseMapInfo<
          PointerUnion<const loopopt::HLInst *, const loopopt::DDRef *>> {
  using Base = DenseMapInfo<
      PointerUnion<const loopopt::HLInst *, const loopopt::DDRef *>>;

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
