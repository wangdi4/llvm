//=== DDRefGatherer.h - Gathers DDRefs attached to HLNodes ----*-- C++ --*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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
//   MemRefs       - collect GEP references
//   TerminalRefs  - collect terminal references
//   BlobRefs      - collect BlobDDRefs
//   ConstantRefs  - collect constants
//   UndefRefs     - collect undefined DDRefs
//
// These modes can be combined using bitwise "or" operator.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_DDREFGATHERER_H
#define LLVM_IR_INTEL_LOOPIR_DDREFGATHERER_H

#include <type_traits>

#include "llvm/Support/Debug.h"

#include "llvm/IR/Intel_LoopIR/RegDDRef.h"
#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

namespace llvm {

namespace loopopt {

enum DDRefGatherMode {
  MemRefs = 1 << 0,
  TerminalRefs = 1 << 1,
  BlobRefs = 1 << 2,
  ConstantRefs = 1 << 3,
  UndefRefs = 1 << 4,
  AllRefs = (1 << 5) - 1,
};

// Data Structure to store mapping of symbase to memory references.
template <typename RefTy>
using SymToRefTy = std::map<unsigned int, SmallVector<RefTy *, 32>>;

template <typename RefTy, unsigned Mode>
class DDRefGathererVisitor final : public HLNodeVisitorBase {
protected:
  SymToRefTy<RefTy> &SymToMemRef;

  template <typename T>
  void
  addRef(T *Ref,
         typename std::enable_if<std::is_convertible<T *, RefTy *>::value>::type
             * = 0) {
    addRefImpl(Ref);
  }

  template <typename T>
  void addRef(T *Ref,
              typename std::enable_if<
                  !std::is_convertible<T *, RefTy *>::value>::type * = 0) {}

  void addRefImpl(RefTy *Ref) {
    if (!(Mode & UndefRefs) && Ref->containsUndef()) {
      return;
    }

    decltype(Ref->getSymbase()) SB = Ref->getSymbase();
    SymToMemRef[SB].push_back(Ref);
  }

public:
  typedef SymToRefTy<RefTy> MapTy;

  DDRefGathererVisitor(SymToRefTy<RefTy> &SymToMemRef)
      : SymToMemRef(SymToMemRef) {}

  void visit(const HLDDNode *RefNode) {
    for (auto I = RefNode->ddref_begin(), E = RefNode->ddref_end(); I != E;
         ++I) {
      if (!(Mode & ConstantRefs) && (*I)->getSymbase() == CONSTANT_SYMBASE) {
        continue;
      }

      bool IsTerminal = (*I)->isScalarRef();
      if (((Mode & TerminalRefs) && IsTerminal) ||
          ((Mode & MemRefs) && !IsTerminal)) {
        addRef<RegDDRef>(*I);
      }

      if (Mode & BlobRefs) {
        for (auto II = (*I)->blob_cbegin(), EE = (*I)->blob_cend(); II != EE;
             ++II) {
          addRef<BlobDDRef>(*II);
        }
      }
    }
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
};

template <typename RefTy, unsigned Mode> struct DDRefGatherer {
  DDRefGatherer() = delete;
  ~DDRefGatherer() = delete;
  DDRefGatherer(const DDRefGatherer &) = delete;
  DDRefGatherer &operator=(const DDRefGatherer &) = delete;

  typedef typename DDRefGathererVisitor<RefTy, Mode>::MapTy MapTy;
  static void gather(const HLNode *Node, SymToRefTy<RefTy> &SymToMemRef) {
    DDRefGathererVisitor<RefTy, Mode> VImpl(SymToMemRef);
    HLNodeUtils::visit(VImpl, Node);
  }

  template <template <typename> class It>
  static void gatherRange(It<const HLNode> Begin, It<const HLNode> End,
                          SymToRefTy<RefTy> &SymToMemRef) {
    DDRefGathererVisitor<RefTy, Mode> VImpl(SymToMemRef);
    HLNodeUtils::visitRange(VImpl, Begin, End);
  }
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
template <typename RefTy> void dumpRefMap(const SymToRefTy<RefTy> &RefMap) {
  for (auto SymVecPair = RefMap.begin(), Last = RefMap.end();
       SymVecPair != Last; ++SymVecPair) {
    auto &RefVec = SymVecPair->second;
    dbgs() << "Symbase " << SymVecPair->first << " contains: \n";
    for (auto Ref = RefVec.begin(), E = RefVec.end(); Ref != E; ++Ref) {
      dbgs() << "\t";
      (*Ref)->dump();
      // dbgs() << " -> isWrite:" << (*Ref)->isLval();
      dbgs() << "\n";
    }
  }
}
#endif

typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;
typedef DDRefGatherer<DDRef, TerminalRefs | MemRefs | BlobRefs>
    DefinedRefGatherer;
}
}

#endif
