//===----------------- HIRConditionalLoadStoreMotion.cpp ------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements an HIR conditional load/store motion pass in order to
/// hoist/sink loads/stores from HLIfs in cases where the analysis available to
/// the LLVM transforms is insufficient.
///
/// For example:
///
/// \code
/// + DO i1 = ...
/// |   if (...)
/// |   {
/// |      %lt = (%addr_l)[i1];
/// |      <use %lt>
/// |      (%addr_s)[i1] = %st;
/// |   }
/// |   else
/// |   {
/// |      %le = (%addr_l)[i1];
/// |      <use %le>
/// |      (%addr_s)[i1] = %se;
/// |   }
/// + END LOOP
///
/// ===>
///
/// + DO i1 = ...
/// |   %l = (%addr_l)[i1];
/// |   if (...)
/// |   {
/// |      <use %l>
/// |      %s = %st;
/// |   }
/// |   else
/// |   {
/// |      <use %l>
/// |      %s = %se;
/// |   }
/// |   (%addr_s)[i1] = %s;
/// + END LOOP
/// \endcode
///
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRConditionalLoadStoreMotion.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeVisitor.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#define OPT_SWITCH "hir-cond-ldst-motion"
#define OPT_DESC "HIR Conditional Load/Store Motion"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass{"disable-" OPT_SWITCH, cl::init(true),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass")};

/// This flag enables detailed DDEdge output in order to make it easier to track
/// down everything preventing a ref from being hoisted/sunk by this pass.
static cl::opt<bool> PrintEdgeTests{
  OPT_SWITCH "-print-edge-tests", cl::init(false), cl::Hidden,
  cl::desc("Print detailed DDEdge debug output when evaluating refs for "
           "hoisting/sinking")};

namespace {

/// A wrapper for running HIRConditionalLoadStoreMotion with the old pass
/// manager.
class HIRConditionalLoadStoreMotionLegacyPass : public HIRTransformPass {
public:
  static char ID;
  HIRConditionalLoadStoreMotionLegacyPass() : HIRTransformPass{ID} {
    initializeHIRConditionalLoadStoreMotionLegacyPassPass(
      *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<HIRFrameworkWrapperPass>();
    AU.addRequired<HIRDDAnalysisWrapperPass>();
    AU.addRequired<HIRLoopStatisticsWrapperPass>();
    AU.setPreservesAll();
  }
};

/// A type for keeping track of merged sets of loads/stores that should be
/// hoisted/sunk from both branches of an if.
class HoistSinkSet {

  /// Refs from the then side of the if that should be hoisted/sunk. These
  /// should all be equivalent and in control flow order (/reverse control flow
  /// for stores).
  SmallVector<RegDDRef *, 8> ThenRefs;

  /// Refs from the else side of the if, also equivalent, equivalent to the refs
  /// in ThenRefs, and in control flow (/reverse control flow) order.
  SmallVector<RegDDRef *, 8> ElseRefs;

public:
  /// A default constructor creating an empty set.
  HoistSinkSet() = default;

  /// A constructor for creating a set containing only \p InitialRef, the first
  /// ref on the then side.
  HoistSinkSet(RegDDRef *InitialRef) : ThenRefs{InitialRef} {}

  /// Prints a HoistSinkSet.
  void print(formatted_raw_ostream &) const;

  /// Dumps a HoistSinkSet.
  void dump() const { print(fdbgs()); }

  /// Tests whether the set is empty.
  ///
  /// The set is considered empty if either of the two branches is empty because
  /// we shouldn't hoist/sink references from only one side.
  bool empty() const { return ThenRefs.empty() || ElseRefs.empty(); }

  /// Reverses the refs to make sure stores are in the expected order.
  void reverse();

  /// Tests whether the set contains a particular ref.
  bool contains(const RegDDRef *Ref) const {
    return find(ThenRefs, Ref) != std::end(ThenRefs) ||
           find(ElseRefs, Ref) != std::end(ElseRefs);
  }

  /// Attempts to add a ref to the this set, on the then side if \p IsThen is
  /// set or to the else side otherwise.
  ///
  /// The ref is added and the method returns true if it is equivalent to the
  /// existing refs; otherwise, the method will return false.
  bool addRefIfEquivalent(RegDDRef *, bool IsThen);

  /// Prints the edge tests performed to determine whether refs can be
  /// hoisted/sunk from \p If according to \p DDG using \p Out.
  void printEdgeTests(formatted_raw_ostream &Out, const HLIf *If,
                      const DDGraph &DDG) const;

  /// Filters refs based on whether they are hoistable or sinkable from \p If
  /// according to \p DDG.
  void filterHoistableOrSinkable(const HLIf *If, const DDGraph &DDG);

  /// Hoists/sinks loads/stores from \p If.
  ///
  /// This HoistSinkSet is required to be non-empty.
  void hoistOrSinkFrom(HLIf *If);
};

} // namespace

/// Prints \p Ref with its HLDDNode number for disambiguation.
///
/// This is helpful for this pass specifically because almost all of the refs
/// that it prints are equivalent and look the same.
static void printWithNodeNumber(const RegDDRef *Ref,
                                formatted_raw_ostream &Out) {
  if (const auto *const Node = Ref->getHLDDNode()) {
    Out << "<" << Node->getNumber() << ">";
  }
  Ref->print(Out);
}

/// Prints a list of refs with node numbers and separated by commas.
static void printWithNodeNumbers(const SmallVectorImpl<RegDDRef *> &Refs,
                                 formatted_raw_ostream &Out) {
  bool first = true;
  for (const RegDDRef *const Ref : Refs) {
    if (!first)
      Out << ", ";
    printWithNodeNumber(Ref, Out);
    first = false;
  }
}

void HoistSinkSet::print(formatted_raw_ostream &Out) const {
  Out << "(";
  printWithNodeNumbers(ThenRefs, Out);
  Out << " | ";
  printWithNodeNumbers(ElseRefs, Out);
  Out << ")";
}

void HoistSinkSet::reverse() {
  std::reverse(std::begin(ThenRefs), std::end(ThenRefs));
  std::reverse(std::begin(ElseRefs), std::end(ElseRefs));
}

/// Determines whether \p A and \p B should be considered equivalent for the
/// purposes of this pass.
///
/// Their access types can differ, but it must be possible to bitcast between
/// them.
static bool areEquivalentAccesses(const RegDDRef *A, const RegDDRef *B) {
  return A->isLval() == B->isLval() && DDRefUtils::areEqual(A, B, true) &&
         CastInst::isBitCastable(A->getDestType(), B->getDestType());
}

bool HoistSinkSet::addRefIfEquivalent(RegDDRef *NewRef, bool IsThen) {
  assert(!ThenRefs.empty() &&
         "No existing refs; construct a new set for this ref instead!");
  assert(NewRef->isLval() == ThenRefs.front()->isLval());

  // Check whether this new ref is equivalent to the existing ones; return false
  // if not.
  if (!areEquivalentAccesses(NewRef, ThenRefs.front()))
    return false;

  // Otherwise, this ref does belong in this set and should be added.
  (IsThen ? ThenRefs : ElseRefs).push_back(NewRef);
  return true;
}

/// Collects loads and stores from a given range of HLInsts \p Insts in \p Loads
/// and \p Stores, on the then side if \p IsThen is set or the else side
/// otherwise.
template <bool IsThen, typename HLInstRange>
static void collectLoadsAndStores(HLInstRange &&Insts,
                                  SmallVectorImpl<HoistSinkSet> &Loads,
                                  SmallVectorImpl<HoistSinkSet> &Stores) {

  // Iterate memory accesses in the range.
  for (HLNode &InstNode : Insts) {
    auto *const Inst = cast<HLInst>(&InstNode);
    for (RegDDRef *const Ref :
         make_range(Inst->ddref_begin(), Inst->ddref_end())) {
      if (!Ref->isMemRef())
        continue;

      // Don't handle non-linear refs as these are not expected to be
      // profitable and need extra handling for blob refs.
      if (!Ref->isLinear())
        continue;

      // Don't touch volatile or fake refs.
      if (Ref->isVolatile() || Ref->isFake())
        continue;

      // Choose which sets to add this ref to if appropriate.
      auto &LoadStoreSets = Ref->isRval() ? Loads : Stores;

      // Add it to the appropriate load/store set if it is equivalent.
      bool Unique = true;
      for (HoistSinkSet &LoadStoreSet : LoadStoreSets) {
        if (LoadStoreSet.addRefIfEquivalent(Ref, IsThen)) {
          Unique = false;
          break;
        }
      }

      // If it is not equivalent to any existing refs, start a new set for it if
      // it is on the then side. If it's on the else side, ignore it because
      // there are no equivalent then refs.
      if (Unique && IsThen)
        LoadStoreSets.emplace_back(Ref);
    }
  }
}

/// Removes empty hoist/sink sets from \p Sets.
static void removeEmptySets(SmallVectorImpl<HoistSinkSet> &Sets) {
  Sets.erase(
    remove_if(Sets, [](const HoistSinkSet &Set) { return Set.empty(); }),
    std::end(Sets));
}

/// Collects loads and stores from \p If into \p Loads and \p Stores.
///
/// Loads will be listed in execution order and stores in reverse execution
/// order in order to ensure that legality checks are done in the right order
/// later and that the hoisted/sunk operations appear in the expected order.
static void collectLoadsAndStores(HLIf *If,
                                  SmallVectorImpl<HoistSinkSet> &Loads,
                                  SmallVectorImpl<HoistSinkSet> &Stores) {

  // Collect from both branches.
  collectLoadsAndStores<true>(make_range(If->then_begin(), If->then_end()),
                              Loads, Stores);
  collectLoadsAndStores<false>(make_range(If->else_begin(), If->else_end()),
                               Loads, Stores);

  // Remove any empty sets that are not present in the else branch.
  removeEmptySets(Loads);
  removeEmptySets(Stores);

  // Reverse the store list, as required to put the stores in reverse execution
  // order.
  std::reverse(std::begin(Stores), std::end(Stores));
  for (HoistSinkSet &StoreSet : Stores)
    StoreSet.reverse();
}

/// Determines whether \p Edge is entirely within one branch of \p If.
///
/// \p ThisRef is a DDRef known to be at one end of \p Edge and also inside of
/// \p If.
static bool isIntraIfEdge(const DDEdge *Edge, const HLIf *If,
                          const DDRef *ThisRef) {
  assert(Edge->getSrc() == ThisRef || Edge->getSink() == ThisRef);
  assert(HLNodeUtils::contains(If, ThisRef->getHLDDNode()));

  // Determine where each end of the edge is relative to the if.
  const DDRef *const OtherRef =
    (Edge->getSrc() == ThisRef) ? Edge->getSink() : Edge->getSrc();
  const HLDDNode *const ThisNode  = ThisRef->getHLDDNode();
  const HLDDNode *const OtherNode = OtherRef->getHLDDNode();
  const bool OtherInThen          = If->isThenChild(OtherNode);
  const bool OtherInElse          = If->isElseChild(OtherNode);

  // If the other end of the edge is not in the if, filter it.
  if (!OtherInThen && !OtherInElse)
    return false;

  // If the ends are in different branches of the if, filter it.
  const bool ThisInThen = If->isThenChild(ThisNode);
  if (ThisInThen != OtherInThen)
    return false;

  // Filter out deps that are only loop-carried and not intra-iteration.
  const HLLoop *const LoopParent = If->getParentLoop();
  assert(LoopParent);
  const unsigned NestLevel = LoopParent->getNestingLevel();
  if ((Edge->getDVAtLevel(NestLevel) & DVKind::EQ) == DVKind::NONE ||
      Edge->getDV().isIndepFromLevel(NestLevel))
    return false;

  // Also filter out backwards deps.
  if (Edge->isBackwardDep())
    return false;

  // All checks passed; keep this edge.
  return true;
}

/// Determines whether \p Edge should prevent \p Ref from \p RefSet from being
/// hoisted/sunk from \p If according to \p DDG.
static bool edgeBlocksMotion(const DDEdge *Edge, const RegDDRef *Ref,
                             const HoistSinkSet &RefSet, const HLIf *If,
                             const DDGraph &DDG) {

  // If the edge is not intra-if, it isn't a problem for hoisting/sinking this
  // ref.
  if (!isIntraIfEdge(Edge, If, Ref))
    return false;

  // If the other end of the edge is equivalent and part of the same hoist/sink
  // set, it will be hoisted/sunk along with this ref and so is also safe.
  // Both ends of the edge will always be RegDDRefs because they are MemRefs.
  const auto *const Src  = cast<RegDDRef>(Edge->getSrc());
  const auto *const Sink = cast<RegDDRef>(Edge->getSink());
  if (areEquivalentAccesses(Src, Sink)) {
    const RegDDRef *const Other = (Ref == Src) ? Sink : Src;
    if (RefSet.contains(Other))
      return false;
  }

  // Otherwise, this edge should block the ref from being hoisted/sunk.
  return true;
}

/// Prints the edge tests performed when attempting to hoist/sink \p Refs from
/// \p RefSet out of \p If according to \p DDG using \p Out.
static void printEdgeTests(formatted_raw_ostream &Out,
                           const SmallVectorImpl<RegDDRef *> &Refs,
                           const HoistSinkSet &RefSet, const HLIf *If,
                           const DDGraph &DDG) {
  for (const RegDDRef *const Ref : Refs) {
    const bool Incoming = Ref->isRval();
    if (Incoming)
      Out << "Checking incoming edges for ";
    else
      Out << "Checking outgoing edges for ";
    printWithNodeNumber(Ref, Out);
    Out << ":\n";

    for (const DDEdge *const Edge :
         Incoming ? DDG.incoming(Ref) : DDG.outgoing(Ref)) {
      const bool EdgeBad = edgeBlocksMotion(Edge, Ref, RefSet, If, DDG);

      if (EdgeBad)
        dbgs() << "BAD: ";
      else
        dbgs() << "     ";
      Edge->print(dbgs());
    }
  }
}

void HoistSinkSet::printEdgeTests(formatted_raw_ostream &Out, const HLIf *If,
                                  const DDGraph &DDG) const {
  ::printEdgeTests(Out, ThenRefs, *this, If, DDG);
  ::printEdgeTests(Out, ElseRefs, *this, If, DDG);
}

/// Determines whether \p Ref can be hoisted/sunk from \p If according to \p
/// DDG.
static bool canHoistOrSink(const RegDDRef *Ref, const HoistSinkSet &RefSet,
                           const HLIf *If, const DDGraph &DDG) {
  const bool Incoming = Ref->isRval();

  for (const DDEdge *const Edge :
       Incoming ? DDG.incoming(Ref) : DDG.outgoing(Ref)) {
    if (edgeBlocksMotion(Edge, Ref, RefSet, If, DDG))
      return false;
  }

  return true;
}

/// Removes non-hoistable/sinkable refs from \p Refs.
///
/// Due to the ordering of refs, it is assumed that if one ref is
/// non-hoistable/sinkable all of the following ones are too. Loads are listed
/// in control flow order, so generally something that would block one load from
/// being hoisted would also block the following loads (which appear after that
/// load in control flow). Similarly, stores are listed in reverse control flow
/// order, so generally something that would block one store from being hoisted
/// would also block the following stores (which appear before that store in
/// control flow).
static void removeNonHoistableOrSinkable(SmallVectorImpl<RegDDRef *> &Refs,
                                         HoistSinkSet &RefSet, const HLIf *If,
                                         const DDGraph &DDG) {
  const auto FirstNonHoistableOrSinkable =
    find_if(Refs, [&RefSet, If, &DDG](const RegDDRef *Ref) {
      return !canHoistOrSink(Ref, RefSet, If, DDG);
    });
  Refs.erase(FirstNonHoistableOrSinkable, std::end(Refs));
}

void HoistSinkSet::filterHoistableOrSinkable(const HLIf *If,
                                             const DDGraph &DDG) {
  removeNonHoistableOrSinkable(ThenRefs, *this, If, DDG);
  removeNonHoistableOrSinkable(ElseRefs, *this, If, DDG);
}

/// Replaces an operand \p Ref of \p Node with a non-memory ref \p NewRef and
/// returns the updated node which may be different from \p Node if node
/// replacement was needed.
///
/// While most HIR instructions can accept memref or temp operands
/// interchangeably, LoadInsts and StoreInsts are special and replacing certain
/// of their operands are required to be memrefs and replacing them directly can
/// break them. This function detects these cases and replaces these
/// instructions completely in order to avoid this issue. When this happens,
/// \p Node will be invalidated and the new node should be accessed via the
/// return value of this function instead. However, the operand DDRefs besides
/// \p Ref will be transferred and should remain valid.
static HLDDNode *replaceOperandWithNonMemoryRef(HLDDNode *Node, RegDDRef *Ref,
                                                RegDDRef *NewRef) {
  assert(!NewRef->isMemRef());

  // If this node is not an HLInst, it's safe to just use replaceOperandDDRef.
  auto *const Inst = dyn_cast<HLInst>(Node);
  if (!Inst) {
    Node->replaceOperandDDRef(Ref, NewRef);
    return Node;
  }

  // If this is a store inst and we're replacing the Lval ref, it needs to be
  // replaced with a load or copy inst depending on what the Rval is.
  HLNodeUtils &HNU                  = Inst->getHLNodeUtils();
  const Instruction *const LLVMInst = Inst->getLLVMInstruction();
  if (isa<StoreInst>(LLVMInst) && Ref->isLval()) {
    RegDDRef *const Rval = Inst->removeRvalDDRef();
    if (Rval->isMemRef()) {
      HLInst *const NewLoad = HNU.createLoad(Rval, "", NewRef);
      HLNodeUtils::replace(Inst, NewLoad);
      return NewLoad;
    } else {
      HLInst *const NewCopy = HNU.createCopyInst(Rval, "", NewRef);
      HLNodeUtils::replace(Inst, NewCopy);
      return NewCopy;
    }
  }

  // If this is a load inst and we're replacing the Rval ref, it needs to be
  // replaced with a copy.
  if (isa<LoadInst>(LLVMInst) && Ref->isRval()) {
    RegDDRef *const Lval  = Inst->removeLvalDDRef();
    HLInst *const NewCopy = HNU.createCopyInst(NewRef, "", Lval);
    HLNodeUtils::replace(Inst, NewCopy);
    return NewCopy;
  }

  // Otherwise, it's safe to just use replaceOperandDDRef.
  Inst->replaceOperandDDRef(Ref, NewRef);
  return Inst;
}

/// Inserts a bitcast if needed to convert between the type of \p Ref and the
/// hoist/sink type \p HSType. A new Lval/Rval ref with the correct type is
/// returned.
static RegDDRef *insertBitcastIfNeeded(RegDDRef *Ref, Type *HSType) {

  // If these are the same type, no change is needed.
  if (Ref->getDestType() == HSType)
    return Ref;

  // Otherwise, prepare to add a bitcast. The direction and location are
  // determined based on whether the ref is an Lval or Rval.
  const bool IsLval = Ref->isLval();
  HLDDNode *RefNode = Ref->getHLDDNode();
  HLNodeUtils &HNU  = RefNode->getHLNodeUtils();

  // If it is an Lval, add a bitcast after it.
  if (IsLval) {

    // Convert Ref to a temp value if needed:
    //
    // %newref = ...;
    if (Ref->isMemRef()) {
      RegDDRef *const NewRef = HNU.createTemp(Ref->getDestType());
      RefNode = replaceOperandWithNonMemoryRef(RefNode, Ref, NewRef);
      Ref     = NewRef;
    }

    // Create the bitcast:
    //
    // %newref = ...;
    // %cldst.cast = bitcast.i64.double(%newref);
    HLInst *const Bitcast =
      HNU.createBitCast(HSType, Ref->clone(), "cldst.cast");
    HLNodeUtils::insertAfter(RefNode, Bitcast);

    return Bitcast->getLvalDDRef();
  }

  // If it is an Rval, add a bitcast before it.
  else {

    // Create the bitcast:
    //
    // %cldst.cast = bitcast.double.i64(%newref);
    // ... = (%A)[i1];
    RegDDRef *const NewRef = HNU.createTemp(HSType);
    HLInst *const Bitcast =
      HNU.createBitCast(Ref->getDestType(), NewRef, "cldst.cast");
    HLNodeUtils::insertBefore(RefNode, Bitcast);

    // Replace the original operand:
    //
    // %cldst.cast = bitcast.double.i64(%newref);
    // ... = %cldst.cast;
    replaceOperandWithNonMemoryRef(RefNode, Ref,
                                   Bitcast->getLvalDDRef()->clone());

    return NewRef;
  }
}

void HoistSinkSet::hoistOrSinkFrom(HLIf *If) {
  assert(!empty());
  const bool IsHoist = ThenRefs.front()->isRval();
  HLNodeUtils &HNU   = If->getHLNodeUtils();

  // Add a load/store ahead/behind the If:
  //
  // %cldst.hoisted = (%A)[i1];
  // if (...)
  // {
  //    (%B)[i1] = (%A)[i1];
  //    ...
  //    (%B)[i1] = (%A)[i1] * ...;
  // }
  // else
  // {
  //    (%B)[i1] = (%A)[i1];
  // }
  // (%B)[i1] = %cldst.sunk;
  const RegDDRef *HoistedOrSunk;
  if (IsHoist) {
    HLInst *const HoistLoadInst =
      HNU.createLoad(ThenRefs.front()->clone(), "cldst.hoisted");
    HLNodeUtils::insertBefore(If, HoistLoadInst);
    HoistedOrSunk = HoistLoadInst->getLvalDDRef();
  } else {
    RegDDRef *const NewSunkRef =
      HNU.createTemp(ThenRefs.front()->getDestType(), "cldst.sunk");
    HLInst *const SunkStoreInst =
      HNU.createStore(NewSunkRef, "", ThenRefs.front()->clone());
    HLNodeUtils::insertAfter(If, SunkStoreInst);
    HoistedOrSunk = NewSunkRef;
  }

  // Replace all of the refs in the set with the hoisted/sunk load/store:
  //
  // %cldst.hoisted = (%A)[i1];
  // if (...)
  // {
  //    %cldst.sunk = %cldst.hoisted;
  //    ...
  //    %cldst.sunk = %cldst.hoisted * ...;
  // }
  // else
  // {
  //    %cldst.sunk = %cldst.hoisted;
  // }
  // (%B)[i1] = %cldst.sunk;
  for (RegDDRef *Ref : ThenRefs) {
    Ref = insertBitcastIfNeeded(Ref, HoistedOrSunk->getDestType());
    replaceOperandWithNonMemoryRef(Ref->getHLDDNode(), Ref,
                                   HoistedOrSunk->clone());
  }
  for (RegDDRef *Ref : ElseRefs) {
    Ref = insertBitcastIfNeeded(Ref, HoistedOrSunk->getDestType());
    replaceOperandWithNonMemoryRef(Ref->getHLDDNode(), Ref,
                                   HoistedOrSunk->clone());
  }
}

/// Performs conditional load/store motion on \p If using \p HDDA.
///
/// This will return true if any memory operations were hoisted/sunk. \p If must
/// be inside of a loop and \p ParentLoop must be its immediate parent loop.
static bool runOnIf(HLIf *If, HIRDDAnalysis &HDDA, const HLLoop *ParentLoop) {
  assert(ParentLoop);
  assert(If->getParentLoop() == ParentLoop);
  LLVM_DEBUG({
    dbgs() << "Examining this HLIf for conditional load/store motion:\n\n";
    If->print(fdbgs(), 0);
  });

  // Collect loads and stores from the if.
  SmallVector<HoistSinkSet, 8> HoistLoads, SinkStores;
  collectLoadsAndStores(If, HoistLoads, SinkStores);

  LLVM_DEBUG({
    dbgs() << "\nCandidate load sets:\n";
    for (const HoistSinkSet &Loads : HoistLoads) {
      dbgs() << "  ";
      Loads.print(fdbgs());
      dbgs() << "\n";
    }
    dbgs() << "\nCandidate store sets:\n";
    for (const HoistSinkSet &Stores : SinkStores) {
      dbgs() << "  ";
      Stores.print(fdbgs());
      dbgs() << "\n";
    }
  });

  // If there aren't any of these, we can give up here.
  if (HoistLoads.empty() && SinkStores.empty()) {
    LLVM_DEBUG(dbgs() << "\nNo matching loads/stores!\n\n");
    return false;
  }

  // If requested, print the edge tests.
  const DDGraph &DDG = ParentLoop ? HDDA.getGraph(ParentLoop)
                                  : HDDA.getGraph(If->getParentRegion());
  LLVM_DEBUG({
    if (PrintEdgeTests) {
      dbgs() << "\nChecking edges:\n";
      for (HoistSinkSet &Loads : HoistLoads)
        Loads.printEdgeTests(fdbgs(), If, DDG);
      for (HoistSinkSet &Stores : SinkStores)
        Stores.printEdgeTests(fdbgs(), If, DDG);
    }
  });

  // Determine which loads/stores can be hoisted/sunk.
  for (HoistSinkSet &Loads : HoistLoads)
    Loads.filterHoistableOrSinkable(If, DDG);
  removeEmptySets(HoistLoads);
  for (HoistSinkSet &Stores : SinkStores)
    Stores.filterHoistableOrSinkable(If, DDG);
  removeEmptySets(SinkStores);

  // If there aren't any, give up here.
  if (HoistLoads.empty() && SinkStores.empty()) {
    LLVM_DEBUG(dbgs() << "\nNo loads/stores to hoist/sink!\n\n");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "\nWill hoist loads:\n";
    for (const HoistSinkSet &Loads : HoistLoads) {
      dbgs() << "  ";
      Loads.print(fdbgs());
      dbgs() << "\n";
    }
    dbgs() << "\nWill sink stores:\n";
    for (const HoistSinkSet &Stores : SinkStores) {
      dbgs() << "  ";
      Stores.print(fdbgs());
      dbgs() << "\n";
    }
    dbgs() << "\n";
  });

  // Do the hoisting/sinking for eligible refs.
  for (HoistSinkSet &Loads : HoistLoads)
    Loads.hoistOrSinkFrom(If);
  for (HoistSinkSet &Stores : SinkStores)
    Stores.hoistOrSinkFrom(If);

  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(If);
  return true;
}

/// Determines whether \p If is an innermost if (one with no non-HLInst
/// children and no calls with unknown side-effects).
static bool isInnermostHLIf(const HLIf *If) {
  for (const HLNode &Child : make_range(If->child_begin(), If->child_end())) {
    const auto *const Inst = dyn_cast<HLInst>(&Child);
    if (!Inst)
      return false;
    if (Inst->isUnsafeSideEffectsCallInst())
      return false;
  }
  return true;
}

/// Performs conditional load/store motion using the given analysis results.
static bool runConditionalLoadStoreMotion(HIRFramework &HIRF,
                                          HIRDDAnalysis &HDDA,
                                          HIRLoopStatistics &HLS) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << OPT_DESC " Disabled\n");
    return false;
  }

  // Run conditional load/store motion on innermost ifs immediately within
  // innermost loops. These are thought to be the highest-leverage ifs for this
  // pass; support for other ifs can be added as needed.
  SmallVector<HLLoop *, 16> InnerLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(InnerLoops);
  bool Changed = false;
  for (HLLoop *const InnerLoop : InnerLoops) {
    const LoopStatistics &LoopStats = HLS.getSelfLoopStatistics(InnerLoop);
    if (!LoopStats.hasIfs())
      continue;
    bool LoopChanged = false;
    for (HLNode &Child :
         make_range(InnerLoop->child_begin(), InnerLoop->child_end())) {
      if (auto *const If = dyn_cast<HLIf>(&Child)) {
        if (isInnermostHLIf(If))
          if (runOnIf(If, HDDA, InnerLoop))
            LoopChanged = true;
      }
    }
    if (LoopChanged) {
      HIRInvalidationUtils::invalidateBody(InnerLoop);
      Changed = true;
    }
  }

  return Changed;
}

char HIRConditionalLoadStoreMotionLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRConditionalLoadStoreMotionLegacyPass, OPT_SWITCH,
                      OPT_DESC, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRConditionalLoadStoreMotionLegacyPass, OPT_SWITCH,
                    OPT_DESC, false, false)

FunctionPass *llvm::createHIRConditionalLoadStoreMotionPass() {
  return new HIRConditionalLoadStoreMotionLegacyPass{};
}

bool HIRConditionalLoadStoreMotionLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    return false;
  }

  return runConditionalLoadStoreMotion(
    getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
    getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
    getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS());
}

PreservedAnalyses
HIRConditionalLoadStoreMotionPass::run(Function &F,
                                       llvm::FunctionAnalysisManager &AM) {
  runConditionalLoadStoreMotion(AM.getResult<HIRFrameworkAnalysis>(F),
                                AM.getResult<HIRDDAnalysisPass>(F),
                                AM.getResult<HIRLoopStatisticsAnalysis>(F));
  return PreservedAnalyses::all();
}
