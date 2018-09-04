//=== DDRefGatherer.h - Gathers DDRefs attached to HLNodes ----*-- C++ --*-===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// DDRefGatherer is an utility class to create map: Symbase -> DDRef
//
// Because of variety of DDRef types, there some modes of gathering:
//   MemRefs         - collect GEP references, excluding IsAddressOfRefs
//   TerminalRefs    - collect terminal references
//   IsAddressOfRefs - collect GEP references for getting a ref. address
//   ConstantRefs    - collect constants
//   UndefRefs       - collect undefined DDRefs
//
// These modes can be combined using bitwise "or" operator.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFGATHERER_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFGATHERER_H

#include <map>
#include <type_traits>

#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/BlobDDRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

namespace llvm {

namespace loopopt {

enum DDRefGatherMode : unsigned int {
  MemRefs = 1 << 0,
  TerminalRefs = 1 << 1,
  IsAddressOfRefs = 1 << 2,
  BlobRefs = 1 << 3,
  ConstantRefs = 1 << 4,
  GenericRValRefs = 1 << 5,
  FakeRefs = 1 << 6,

  AllRefs = ~0U,
};

template <typename RefTy> using RefVectorTy = SmallVector<RefTy *, 32>;

// Data Structure to store mapping of symbase to memory references. We are using
// std::map here instead of DenseMap because of a large vector size.
template <typename RefTy>
using SymToRefTy = std::map<unsigned int, RefVectorTy<RefTy>>;

template <typename ContainerTy, typename RefTy>
struct DDRefGathererVisitorTraits {
  static void addRef(ContainerTy &, RefTy *);
};

template <typename RefTy>
struct DDRefGathererVisitorTraits<RefVectorTy<RefTy>, RefTy> {
  static void addRef(RefVectorTy<RefTy> &Vector, RefTy *Ref) {
    Vector.push_back(Ref);
  }
};

template <typename RefTy>
struct DDRefGathererVisitorTraits<SymToRefTy<RefTy>, RefTy> {
  static void addRef(SymToRefTy<RefTy> &Map, RefTy *Ref) {
    unsigned SB = Ref->getSymbase();
    Map[SB].push_back(Ref);
  }
};

template <typename RefTy, typename ContainerTy, typename Predicate>
class DDRefGathererVisitor final : public HLNodeVisitorBase {
  ContainerTy &Container;
  Predicate Pred;

  template <typename T>
  void
  addRef(T *Ref,
         typename std::enable_if<std::is_convertible<T *, RefTy *>::value>::type
             * = 0) {
    if (Pred(Ref)) {
      addRefImpl(Ref);
    }
  }

  template <typename T>
  void addRef(T *Ref,
              typename std::enable_if<
                  !std::is_convertible<T *, RefTy *>::value>::type * = 0) {}

  void addRefImpl(RefTy *Ref) {
    DDRefGathererVisitorTraits<ContainerTy, RefTy>::addRef(Container, Ref);
  }

public:
  DDRefGathererVisitor(ContainerTy &Container, Predicate Pred = Predicate())
      : Container(Container), Pred(Pred) {}

  void visit(const RegDDRef *Ref) {
    for (auto I = Ref->blob_cbegin(), E = Ref->blob_cend(); I != E; ++I) {
      addRef<BlobDDRef>(const_cast<BlobDDRef *>(*I));
    }
  }

  void visit(const HLDDNode *Node) {
    // Collect rvals before lval to be in strict lexical order.
    for (auto I = Node->rval_op_ddref_begin(), E = Node->rval_op_ddref_end();
         I != E; ++I) {
      addRef<RegDDRef>(const_cast<RegDDRef *>(*I));
      visit(*I);
    }

    if (auto LvalRef = Node->getLvalDDRef()) {
      addRef<RegDDRef>(const_cast<RegDDRef *>(LvalRef));
      visit(LvalRef);
    }

    for (auto I = Node->fake_ddref_begin(), E = Node->fake_ddref_end(); I != E;
         ++I) {
      addRef<RegDDRef>(const_cast<RegDDRef *>(*I));
      visit(*I);
    }
  }

  void visit(HLNode *Node) {}
  void visit(const HLNode *Node) {}
  void postVisit(HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
};

class DDRefGathererUtils {

  DDRefGathererUtils() = delete;
  ~DDRefGathererUtils() = delete;
  DDRefGathererUtils(const DDRefGathererUtils &) = delete;
  DDRefGathererUtils &operator=(const DDRefGathererUtils &) = delete;

public:
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  template <typename RefTy> static void dump(const SymToRefTy<RefTy> &RefMap) {
    for (auto &RefVec : RefMap) {
      dbgs() << "Symbase " << RefVec.first << " contains: \n";
      dump(RefVec.second);
    }
  }
#endif

public:
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  template <typename RefTy> static void dump(const RefVectorTy<RefTy> &RefVec) {
    for (auto *Ref : RefVec) {
      dbgs() << "\t";
      Ref->dump();
      dbgs() << "\n";
    }
  }
#endif

  template <typename RefTy>
  static void makeUnique(RefVectorTy<RefTy> &RefVec, bool RelaxedMode = false) {
    RefVec.erase(
        std::unique(RefVec.begin(), RefVec.end(),
                    std::bind(DDRefUtils::areEqual, std::placeholders::_1,
                              std::placeholders::_2, RelaxedMode)),
        RefVec.end());
  }

  /// Removes the duplicates by comparing the Ref's in sorted order.
  template <typename RefTy>
  static void makeUnique(SymToRefTy<RefTy> &RefMap, bool RelaxedMode = false) {
    for (auto &SymVecPair : RefMap) {
      auto &RefVec = SymVecPair.second;

      makeUnique(RefVec, RelaxedMode);
    }
  }

  template <typename RefTy> static void sort(RefVectorTy<RefTy> &RefVec) {
    sort(RefVec, DDRefUtils::compareMemRef);
  }

  template <typename RefTy> static void sort(SymToRefTy<RefTy> &MemRefMap) {
    sort(MemRefMap, DDRefUtils::compareMemRef);
  }

  template <typename RefTy, typename Compare>
  static void sort(RefVectorTy<RefTy> &RefVec, Compare Cmp) {
    std::sort(RefVec.begin(), RefVec.end(), Cmp);
  }

  template <typename RefTy, typename Compare>
  static void sort(SymToRefTy<RefTy> &MemRefMap, Compare Cmp) {
    // Sorts the memory reference based on the comparison provided
    // by compareMemRef.
    for (auto SymVecPair = MemRefMap.begin(), Last = MemRefMap.end();
         SymVecPair != Last; ++SymVecPair) {
      auto &RefVec = SymVecPair->second;

#if 0
      // The code below is for checking the conformance of compareMemRef to a
      // Compare concept.
      // It's O(n^3) and wouldn't be enabled neither in prod nor in debug modes.
      // Un-comment if you believe that something is wrong with the sorting, it
      // could help to catch refs on which compareMemRef fails.
      auto comp = [](const RegDDRef *R1, const RegDDRef *R2) {
        return DDRefUtils::compareMemRef(R1, R2);
      };
      auto equv = [](const RegDDRef *R1, const RegDDRef *R2) {
        return !DDRefUtils::compareMemRef(R1, R2) &&
               !DDRefUtils::compareMemRef(R2, R1);
      };

      for (auto I = RefVec.begin(), IE = RefVec.end(); I != IE; ++I) {
        assert(comp(*I, *I) == false);
        assert(equv(*I, *I) == true);
        for (auto J = RefVec.begin(), JE = RefVec.end(); J != JE; ++J) {
          if (comp(*I, *J) == true) {
            assert(comp(*J, *I) == false);
          }
          if (equv(*I, *J) == true) {
            assert(equv(*J, *I) == true);
          }
          for (auto C = RefVec.begin(), CE = RefVec.end(); C != CE; ++C) {
            if (comp(*I, *J) == true && comp(*J, *C) == true) {
              assert(comp(*I, *C) == true);
            }
            if (equv(*I, *J) == true && equv(*J, *C) == true) {
              assert(equv(*I, *C) == true);
            }
          }
        }
      }
#endif

      sort(RefVec, Cmp);
    }
  }

  /// Sorts and removes duplicates in MemRefMap.
  template <typename RefTy>
  static void sortAndUnique(RefVectorTy<RefTy> &RefVec,
                            bool RelaxedMode = false) {
    sort(RefVec);
    makeUnique(RefVec, RelaxedMode);
  }

  /// Sorts and removes duplicates in MemRefMap.
  template <typename RefTy>
  static void sortAndUnique(SymToRefTy<RefTy> &MemRefMap,
                            bool RelaxedMode = false) {
    sort(MemRefMap);
    makeUnique(MemRefMap, RelaxedMode);
  }
};

template <typename RefTy>
struct DDRefGathererLambda : public DDRefGathererUtils {

  DDRefGathererLambda() = delete;
  ~DDRefGathererLambda() = delete;
  DDRefGathererLambda(const DDRefGathererLambda &) = delete;
  DDRefGathererLambda &operator=(const DDRefGathererLambda &) = delete;

  typedef SymToRefTy<RefTy> MapTy;
  typedef RefVectorTy<RefTy> VectorTy;

  template <bool Recursive = true, typename Predicate, typename ContainerTy>
  static void gather(const HLNode *Node, ContainerTy &Container,
                     Predicate Pred) {
    DDRefGathererVisitor<RefTy, ContainerTy, Predicate> VImpl(Container, Pred);
    HLNodeUtils::visit<Recursive>(VImpl, Node);
  }

  template <bool Recursive = true, bool RecurseInsideLoops = true, typename It,
            typename Predicate, typename ContainerTy>
  static void gatherRange(It Begin, It End, ContainerTy &Container,
                          Predicate Pred) {
    DDRefGathererVisitor<RefTy, ContainerTy, Predicate> VImpl(Container, Pred);
    HLNodeUtils::visitRange<Recursive, RecurseInsideLoops>(VImpl, Begin, End);
  }
};

template <typename RefTy, unsigned Mode, bool CollectUndefs = true>
struct DDRefGatherer : DDRefGathererLambda<RefTy> {

  DDRefGatherer() = delete;
  ~DDRefGatherer() = delete;
  DDRefGatherer(const DDRefGatherer &) = delete;
  DDRefGatherer &operator=(const DDRefGatherer &) = delete;

  using typename DDRefGathererLambda<RefTy>::MapTy;

  struct ModeSelectorPredicate {
    bool operator()(const RegDDRef *Ref) {
      unsigned Symbase = Ref->getSymbase();

      if (!(Mode & ConstantRefs) && (Symbase == ConstantSymbase)) {
        return false;
      }

      if (!(Mode & GenericRValRefs) && (Symbase == GenericRvalSymbase)) {
        return false;
      }

      if (!(Mode & FakeRefs) && Ref->isFake()) {
        return false;
      }

      return ((Mode & TerminalRefs) && Ref->isTerminalRef()) ||
             ((Mode & IsAddressOfRefs) && Ref->isAddressOf()) ||
             ((Mode & MemRefs) && Ref->isMemRef()) ||
             (!CollectUndefs && !Ref->containsUndef());
    }

    bool operator()(const BlobDDRef *Ref) {
      return static_cast<bool>(Mode & BlobRefs);
    }
  };

  template <typename ContainerTy>
  static void gather(const HLNode *Node, ContainerTy &Container) {
    DDRefGathererLambda<RefTy>::gather(Node, Container,
                                       ModeSelectorPredicate());
  }

  template <typename It, typename ContainerTy, bool Recursive = true,
            bool RecurseInsideLoops = true>
  static void gatherRange(It Begin, It End, ContainerTy &Container) {
    DDRefGathererLambda<RefTy>::template gatherRange<
        Recursive, RecurseInsideLoops, It, ModeSelectorPredicate, ContainerTy>(
        Begin, End, Container, ModeSelectorPredicate());
  }
};

} // namespace loopopt
} // namespace llvm

#endif
