//===- HIRSymbaseAssignment.cpp - Assigns symbase to ddrefs ---------------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Symbase assignment pass.
//
//===----------------------------------------------------------------------===//

#include "HIRSymbaseAssignment.h"

#include <map>

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRParser.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"

using namespace llvm;
using namespace llvm::loopopt;

enum class VaryingBaseMode { QueryAlias, QueryLoopCarriedAlias };

static cl::opt<VaryingBaseMode> VaryingBaseHandling(
    "hir-symbase-assignment-varying-base-mode",
    cl::desc("Influence how we use AA when encountering pointers with varying "
             "bases"),
    cl::init(VaryingBaseMode::QueryLoopCarriedAlias), cl::ReallyHidden,
    cl::values(clEnumValN(VaryingBaseMode::QueryAlias, "query-alias",
                          "Query the alias() interface, possibly incorrectly"),
               clEnumValN(VaryingBaseMode::QueryLoopCarriedAlias,
                          "query-loopcarried",
                          "Query the loopCarriedAlias() interface")));

#define DEBUG_TYPE "hir-symbase-assignment"

namespace {
typedef SmallVector<DDRef *, 16> RefsTy;
} // namespace

class HIRSymbaseAssignment::HIRSymbaseAssignmentVisitor
    : public HLNodeVisitorBase {
  // TODO: probably change to DenseMap by lowering size of RefsTy once we
  // disable llvm's complete unroll.
  typedef std::map<Value *, RefsTy> PtrToRefsTy;

  HIRSymbaseAssignment &SA;
  HybridAliasSetTracker AST;
  PtrToRefsTy PtrToRefs;

  void addToAST(RegDDRef *Ref);

public:
  HIRSymbaseAssignmentVisitor(HIRSymbaseAssignment &CurSA, AliasAnalysis &AA)
      : SA(CurSA), AST(AA) {}

  HybridAliasSetTracker &getAST() { return AST; }

  const RefsTy &getRefs(Value *Ptr) const {
    auto RefsIt = PtrToRefs.find(Ptr);
    assert((RefsIt != PtrToRefs.end()) && "Pointer not found!");
    return RefsIt->second;
  }

  void visit(HLNode *Node) {}
  void visit(HLDDNode *Node);
  void postVisit(HLNode *) {}
  void postVisit(HLDDNode *) {}
};

// TODO: add special handling for memrefs with undefined base pointers.
void HIRSymbaseAssignment::HIRSymbaseAssignmentVisitor::addToAST(
    RegDDRef *Ref) {
  assert(!Ref->isTerminalRef() && "Non terminal ref is expected.");

  Value *Ptr = SA.HIRP.getGEPRefPtr(Ref);
  assert(Ptr && "Could not find Value* ptr for mem load store ref");
  LLVM_DEBUG(dbgs() << "Got ptr " << *Ptr << "\n");

  PtrToRefs[Ptr].push_back(Ref);

  AAMDNodes AANodes;
  Ref->getAAMetadata(AANodes);

  if (Ref->isStructurallyRegionInvariant()) {
    // The entire pointer (base and indexing) is region invariant. A normal AST
    // will correctly disambiguate, even with precise size.
    LocationSize LocSize = MemoryLocation::UnknownSize;
    if (!Ref->isFake() &&
        (!Ref->isAddressOf() || Ref->isAddressOfSizedType())) {
      uint64_t RefSize = Ref->isAddressOf() ? Ref->getElementTypeSizeInBytes()
                                            : Ref->getDestTypeSizeInBytes();
      LocSize = LocationSize::precise(RefSize);
    }
    LLVM_DEBUG(
        dbgs() << "\tRegion invariant; adding to AST with precise size.\n");
    AST.add(Ptr, LocSize, AANodes);
  } else if (VaryingBaseHandling == VaryingBaseMode::QueryAlias ||
             Ref->getBaseCE()->isProperLinear()) {
    // The base pointer is invariant. We can add it to the normal AST but with
    // UnknownSize.
    LLVM_DEBUG(dbgs() << "\tInvariant base, but varying indexing; adding to "
                         "AST with UnknownSize.\n");
    AST.add(Ptr, MemoryLocation::UnknownSize, AANodes);
  } else {
    // We're trying to track a pointer which is not region invariant.  Add to
    // the AST with the "LoopCarried" requirement flag. The underlying pairwise
    // "loopCarriedAlias" interface requires that both pointers have
    // "UnknownSize" to guarantee strong enough semantics to break a
    // dependence, so we must not use precise sizes here.
    LLVM_DEBUG(
        dbgs() << "\tVarying base pointer; will use loop-carried AST.\n");
    AST.add(Ptr, MemoryLocation::UnknownSize, AANodes, true);
  }
}

void HIRSymbaseAssignment::HIRSymbaseAssignmentVisitor::visit(HLDDNode *Node) {
  for (auto I = Node->ddref_begin(), E = Node->ddref_end(); I != E; ++I) {
    if ((*I)->hasGEPInfo()) {
      addToAST(*I);
    }
  }
}

void HIRSymbaseAssignment::run() {
  // Create alias sets per region.
  for (auto I = HIRF.hir_begin(), E = HIRF.hir_end(); I != E; ++I) {
    HIRSymbaseAssignmentVisitor SV(*this, AA);
    HLNodeUtils::visit(SV, &*I);

    // Each ref in a set gets the same symbase
    for (auto &AliasSet : SV.getAST()) {
      unsigned CurSymbase = HIRF.getNewSymbase();
      LLVM_DEBUG(dbgs() << "Assigned following refs to Symbase " << CurSymbase
                        << "\n");

      for (auto AV : AliasSet) {
        Value *Ptr = AV.getValue();
        auto &Refs = SV.getRefs(Ptr);
        for (auto CurRef : Refs) {
          LLVM_DEBUG(CurRef->dump());
          LLVM_DEBUG(dbgs() << "\n");
          CurRef->setSymbase(CurSymbase);
        }
      }
    }
  }
}

void HIRSymbaseAssignment::print(raw_ostream &OS) const {
  typedef DDRefGatherer<const DDRef, AllRefs ^ ConstantRefs>
      NonConstantRefGatherer;

  NonConstantRefGatherer::MapTy SymToRefs;
  NonConstantRefGatherer::gatherRange(HIRF.hir_begin(), HIRF.hir_end(),
                                      SymToRefs);

  formatted_raw_ostream FOS(OS);
  FOS << "Symbase Reference Vector:";
  FOS << "\n";

  for (auto SymVecPair = SymToRefs.begin(), Last = SymToRefs.end();
       SymVecPair != Last; ++SymVecPair) {
    auto &RefVec = SymVecPair->second;
    FOS << "Symbase ";
    FOS << SymVecPair->first;
    FOS << ":\n";
    for (auto Ref = RefVec.begin(), E = RefVec.end(); Ref != E; ++Ref) {
      (*Ref)->print(FOS, true);
      FOS << "\n";
    }
  }
}
