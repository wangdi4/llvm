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
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#define OPT_SWITCH "hir-cond-ldst-motion"
#define OPT_DESC "HIR Conditional Load/Store Motion"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass{"disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass")};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// This flag enables detailed DDEdge output in order to make it easier to track
/// down everything preventing a ref from being hoisted/sunk by this pass.
static cl::opt<bool> PrintEdgeTests{
  OPT_SWITCH "-print-edge-tests", cl::init(false), cl::Hidden,
  cl::desc("Print detailed DDEdge debug output when evaluating refs for "
           "hoisting/sinking")};
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

namespace {

/// A type for keeping track of merged sets of loads or stores that should be
/// hoisted or sunk from both branches of an if.
class HoistOrSinkSet {

  /// Refs from the then side of the if that should be hoisted/sunk. These
  /// should all be equivalent and in control flow order (/reverse control flow
  /// for stores).
  SmallVector<RegDDRef *, 8> ThenRefs;

  /// Refs from the else side of the if, also equivalent, equivalent to the refs
  /// in ThenRefs, and in control flow (/reverse control flow) order.
  SmallVector<RegDDRef *, 8> ElseRefs;

  /// If set, use this ref when hoisting/sinking to enable using common temps
  /// between different sets.
  RegDDRef *CommonTemp = nullptr;

  /// True if we have memory references in the Then branch
  bool HasUnconditionalRefsThenBranch = false;

  /// True if we have memory references in the Else branch
  bool HasUnconditionalRefsElseBranch = false;

public:
  /// A default constructor creating an empty set.
  HoistOrSinkSet() = default;

  /// A constructor for creating a set containing only \p InitialRef, the first
  /// ref on the then side.
  HoistOrSinkSet(RegDDRef *InitialRef, bool IsThen)
      : ThenRefs{InitialRef}, HasUnconditionalRefsThenBranch(IsThen),
        HasUnconditionalRefsElseBranch(!IsThen) {}

  /// Constructor that will copy the refs from SrcSet, which represents a new
  /// ref found in an inner If, into the Then or Else refs vector depending
  /// on the IsThen parameter.
  HoistOrSinkSet(HoistOrSinkSet &SrcSet, bool IsThen) {
    CommonTemp = SrcSet.CommonTemp;
    appendDataFromSet(SrcSet, IsThen);
  }

  /// Prints a HoistOrSinkSet.
  void print(formatted_raw_ostream &) const;

  /// Dumps a HoistOrSinkSet.
  void dump() const { print(fdbgs()); }

  /// Return true if the current set is unconditional.
  bool isUnconditional() const {
    return HasUnconditionalRefsThenBranch && HasUnconditionalRefsElseBranch;
  }

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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// Prints the edge tests performed to determine whether refs can be
  /// hoisted/sunk from \p If according to \p DDG using \p Out.
  void printEdgeTests(formatted_raw_ostream &Out, const DDGraph &DDG) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  /// Filters refs based on whether they are hoistable or sinkable from \p If
  /// according to \p DDG.
  bool isLegallyHoistableOrSinkable(const DDGraph &DDG);

  /// Checks whether \p HoistSet (a set of loads to hoist) and \p SinkSet (a set
  /// of stores to sink) should use a common temp. If so, this sets \ref
  /// CommonTemp for both sets and returns true.
  static bool checkAndAssignCommonTemp(HoistOrSinkSet &HoistSet,
                                       HoistOrSinkSet &SinkSet);

  /// Hoists/sinks loads/stores from \p If.
  ///
  /// This HoistOrSinkSet is required to be non-empty.
  void hoistOrSinkFrom(HLIf *If);

  /// Copy all the Refs in the Then and Else branches from the input Set into
  /// the Then or Else set of the current Set. Basically, this function
  /// collects the result of analyzing an inner If and copies it into the
  /// branch needed for the outer if. Return true if the merge was possible,
  /// else return false.
  bool mergeInformationFromInnerSet(HoistOrSinkSet &Set, bool IsThen);

  /// Return the first memref stored in the ThenRefs or ElseRefs.
  RegDDRef *getFirstMemRef() const {
    assert(isUnconditional() &&
           "Trying to access a memref in a conditional set");
    return ThenRefs.front();
  }

  unsigned getNumMemRefs() const { return ThenRefs.size() + ElseRefs.size(); }

private:
  /// Return true if the refs in the input Set access the same memory space
  /// as the current set
  bool hasEquivalentAccess(HoistOrSinkSet &Set);

  /// Append into the ThenRefs or ElseRefs the ThenRefs and ElseRefs of SrcSet,
  /// depending on the value of IsThen,. The value of
  /// HasUnconditionalRefsThenBranch or HasUnconditionalRefsElseBranch will be
  /// updated if SrcSet is unconditional.
  void appendDataFromSet(HoistOrSinkSet &SrcSet, bool IsThen) {
    auto &DestVect = IsThen ? ThenRefs : ElseRefs;
    DestVect.append(SrcSet.ThenRefs);
    DestVect.append(SrcSet.ElseRefs);

    if (SrcSet.isUnconditional()) {
      if (IsThen)
        HasUnconditionalRefsThenBranch = true;
      else
        HasUnconditionalRefsElseBranch = true;
    }
  }
};

/// Helper class that finds the possible If instructions and memory references
/// that could be candidates for the optimization.
class HoistSinkSetBuilder : public HLNodeVisitorBase {
private:
  /// SmallVector that contains all the refs that will be hoisted or sinked for
  /// a particular If
  typedef SmallVector<HoistOrSinkSet, 8> HoistSinkRefsVector;

  /// Pair all the load with all the store for particular one If
  struct HoistRefsAndSinkRefsVec {
    HoistSinkRefsVector Loads;
    HoistSinkRefsVector Stores;

    bool isEmpty() { return Loads.empty() && Stores.empty(); }

    // Stores need to be organized. We want to order them by distance in
    // ascending order. The transformation will go from the end to begin,
    // and inserting the new stores in the HIR will prevent backward
    // dependencies.
    void sortStores() {
      if (Stores.empty())
        return;

      std::sort(Stores.begin(), Stores.end(),
                [](HoistOrSinkSet &LHS, HoistOrSinkSet &RHS) {
                  auto *LHSRef = LHS.getFirstMemRef();
                  auto *RHSRef = RHS.getFirstMemRef();
                  return DDRefUtils::compareMemRefAddress(LHSRef, RHSRef);
                });
    }
  };

  /// Map an HLIf with the refs that could be hoisted or sinked from it.
  typedef std::pair<HLIf *, HoistRefsAndSinkRefsVec> HoistSinkCandidate;

  HLLoop *ParentLoop;
  SmallVector<HoistSinkCandidate, 8> HoistSinkCandidatesVector;

  /// Vector that will be used to keep tracking of all memrefs starting from
  /// the outermost If.
  SmallVector<HoistRefsAndSinkRefsVec, 6> IfHoistSinkRefsStack;
  HLNode *SkipNode = nullptr;

public:
  HoistSinkSetBuilder(HLLoop *ParentLoop) : ParentLoop(ParentLoop) {}

  bool skipRecursion(HLNode *Node) { return Node == SkipNode; }

  void visit(HLNode *Node) {
    SkipNode = Node;
    // HLIf and HLInst are handled in their own visitor, anything else that
    // lands here should be considered as a bad node.
    disableOutermostIf();
  }

  void visit(HLInst *Inst) {
    if (IfHoistSinkRefsStack.empty())
      return;

    if (Inst->isUnsafeSideEffectsCallInst()) {
      disableOutermostIf();
      return;
    }

    // If IfHoistSinkRefsStack is not empty then it means that we are
    // collecting from an If. Therefore the parent of the instructions must be
    // an If.
    auto IfParent = cast<HLIf>(Inst->getParent());
    bool IsThen = IfParent->isThenChild(Inst);
    auto &CurrHostSinkRefs = IfHoistSinkRefsStack.back();

    for (RegDDRef *const Ref :
         make_range(Inst->ddref_begin(), Inst->ddref_end())) {
      if (!Ref->isMemRef())
        continue;

      // Don't handle non-linear refs as these are not expected to be
      // profitable and need extra handling for blob refs.
      if (!Ref->isLinear())
        continue;

      // Don't touch fake refs.
      if (Ref->isFake())
        continue;

      // Choose which sets to add this ref to if appropriate.
      auto &LoadStoreSets =
          Ref->isRval() ? CurrHostSinkRefs.Loads : CurrHostSinkRefs.Stores;

      // Add it to the appropriate load/store set if it is equivalent.
      bool Unique = true;
      for (HoistOrSinkSet &LoadStoreSet : LoadStoreSets) {
        if (LoadStoreSet.addRefIfEquivalent(Ref, IsThen)) {
          Unique = false;
          break;
        }
      }

      // If it is not equivalent to any existing refs, start a new set for it.
      if (Unique)
        LoadStoreSets.emplace_back(Ref, IsThen);
    }
  }

  void postVisit(HLNode *) {}

  void visit(HLIf *If) {
    auto IfParent = If->getParent();
    bool IfIsValid = false;

    if (IfParent == ParentLoop) {
      // If the parent of the If is the parent loop, then it means that we are
      // starting to analyze a new outermost If. In this case, we are going to
      // start a new IfHoistSinkRefsStack with one entry.
      IfIsValid = If->hasElseChildren();
    } else if (!IfHoistSinkRefsStack.empty()) {
      // If the parent of the If is another If, and IfHoistSinkRefsStack is not
      // empty, then it means that we have nested If. In this case, we are
      // going to create a new entry in IfHoistSinkRefsStack to handle all the
      // refs at the current level.
      IfIsValid = true;
    }

    // If the parent is not the loop that is being visited, or an If
    // (nested If), then disable the collection for the current If.
    if (!IfIsValid) {
      SkipNode = If;
      disableOutermostIf();
      return;
    }

    // Start the new sets for saving the loads and stores in the current If
    IfHoistSinkRefsStack.emplace_back();
  }

  void postVisit(HLIf *If) {
    if (IfHoistSinkRefsStack.empty())
      return;

    auto CurrHostSinkRefs = IfHoistSinkRefsStack.back();
    IfHoistSinkRefsStack.pop_back();

    if (CurrHostSinkRefs.isEmpty())
      return;

    if (If->getParent() == ParentLoop) {
      // Remove any set that we didn't collected anything from the Else branch.
      removeConditionalSets(CurrHostSinkRefs.Loads);
      removeConditionalSets(CurrHostSinkRefs.Stores);

      if (CurrHostSinkRefs.isEmpty())
        return;

      // If the parent node of the If is the loop, then it means that we
      // already collected and merged all the information.

      // Sort the stores to prevent backward dependencies
      CurrHostSinkRefs.sortStores();

      // Store the new candidate
      HoistSinkCandidatesVector.emplace_back(
          std::make_pair(If, CurrHostSinkRefs));
    } else {
      // Else update the outer If with the information found in the inner If
      auto &DestHoistSinkPair = IfHoistSinkRefsStack.back();
      auto *ParentIf = cast<HLIf>(If->getParent());
      bool IsThen = ParentIf->isThenChild(If);
      for (auto &LoadSet : CurrHostSinkRefs.Loads)
        updateHoistSinkSetsRefs(LoadSet, DestHoistSinkPair.Loads, IsThen);

      for (auto &StoreSet : CurrHostSinkRefs.Stores)
        updateHoistSinkSetsRefs(StoreSet, DestHoistSinkPair.Stores, IsThen);
    }
  }

  // Return the candidates collected
  SmallVectorImpl<HoistSinkCandidate> &getHoistSinkSetsCollected() {
    return HoistSinkCandidatesVector;
  }

private:
  /// Clear the data for the current chain of nested Ifs if we found something
  /// wrong
  void disableOutermostIf() { IfHoistSinkRefsStack.clear(); }

  /// Update the destination sets with the references collected in the source
  /// set.
  void updateHoistSinkSetsRefs(HoistOrSinkSet &SrcSet,
                               SmallVectorImpl<HoistOrSinkSet> &DstSets,
                               bool IsThen);

  /// Removes the conditional sets from \p Sets.
  void removeConditionalSets(SmallVectorImpl<HoistOrSinkSet> &Sets) {
    Sets.erase(remove_if(Sets,
                         [](const HoistOrSinkSet &Set) {
                           bool RemoveSet = !Set.isUnconditional();
                           LLVM_DEBUG({
                             if (RemoveSet) {
                               dbgs() << "Removing conditional set: ";
                               Set.dump();
                               dbgs() << "\n";
                             }
                           });
                           return RemoveSet;
                         }),
               std::end(Sets));
  }
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

void HoistOrSinkSet::print(formatted_raw_ostream &Out) const {
  Out << "(";
  printWithNodeNumbers(ThenRefs, Out);
  Out << " | ";
  printWithNodeNumbers(ElseRefs, Out);
  Out << ")";
}

void HoistOrSinkSet::reverse() {
  std::reverse(std::begin(ThenRefs), std::end(ThenRefs));
  std::reverse(std::begin(ElseRefs), std::end(ElseRefs));
}

/// Determines whether \p A and \p B should be considered equivalent for the
/// purposes of this pass.
///
/// Their access types can differ, but it must be possible to bitcast between
/// them.
static bool areEquivalentAccesses(const RegDDRef *A, const RegDDRef *B) {
  return A->isLval() == B->isLval() &&
         DDRefUtils::areEqualWithoutBitCastDestType(A, B) &&
         CastInst::isBitCastable(A->getDestType(), B->getDestType());
}

bool HoistOrSinkSet::hasEquivalentAccess(HoistOrSinkSet &Set) {
  // We only compare with the Then branch because the Else branch will be
  // collected only if there are refs in the Then branch.
  if (ThenRefs.empty() || Set.ThenRefs.empty())
    return false;

  return areEquivalentAccesses(ThenRefs.front(), Set.ThenRefs.front());
}

bool HoistOrSinkSet::mergeInformationFromInnerSet(HoistOrSinkSet &Set,
                                                  bool IsThen) {

  if (!hasEquivalentAccess(Set))
    return false;

  appendDataFromSet(Set, IsThen);
  return true;
}

bool HoistOrSinkSet::addRefIfEquivalent(RegDDRef *NewRef, bool IsThen) {
  assert(!ThenRefs.empty() &&
         "No existing refs; construct a new set for this ref instead!");
  assert(NewRef->isLval() == ThenRefs.front()->isLval());

  // Check whether this new ref is equivalent to the existing ones; return false
  // if not.
  if (!areEquivalentAccesses(NewRef, ThenRefs.front()))
    return false;

  // Otherwise, this ref does belong in this set and should be added.
  if (IsThen) {
    ThenRefs.push_back(NewRef);
    HasUnconditionalRefsThenBranch = true;
  } else {
    ElseRefs.push_back(NewRef);
    HasUnconditionalRefsElseBranch = true;
  }
  return true;
}

/// Find the HoistOrSinkSet in DstSets that have the same memory reference as
/// SrcSet, and copy the Refs from SrcSet into the set found. Else, if
/// destination set not found and IsThen is true, insert SrcSet into DstSets.
/// Basically, we are copying the memory refs found by analyzing a conditional
/// into the proper branch of an outer condition. For example:
///
///   if() {
///     ref1
///   }
///   else {
///     if () {
///       ref1
///     }
///     else {
///       ref1
///     }
///   }
///
/// The RegDDRef ref1 will be collected first in the Then branch of the outer
/// If. Then, when the outer Else branch is analyzed, ref1 will be collected
/// for the inner If since it is in both branches. Since ref1 is in both
/// branches for the inner If, it can be seen that the outer Else branch have
/// a reference to ref1. This means that we can add ref1 from the inner If as
/// part of the Else branch for the outer If.
void HoistSinkSetBuilder::updateHoistSinkSetsRefs(
    HoistOrSinkSet &SrcSet, SmallVectorImpl<HoistOrSinkSet> &DstSets,
    bool IsThen) {
  // Find the HoistOrSinkSet in DstSets that the refs could be merged
  for (auto &DstSet : DstSets) {
    if (DstSet.mergeInformationFromInnerSet(SrcSet, IsThen))
      return;
  }

  if (IsThen)
    DstSets.emplace_back(SrcSet, IsThen);
}

/// Determines whether \p Edge is entirely within one branch of \p If.
///
/// \p ThisRef is a DDRef known to be at one end of \p Edge and also inside of
/// \p If.
static bool IsLegalIntraIfEdge(const DDEdge *Edge, const HLIf *If,
                               const DDRef *ThisRef) {

  assert(Edge->getSrc() == ThisRef || Edge->getSink() == ThisRef);

  // Determine where each end of the edge is relative to the if.
  const DDRef *const OtherRef =
    (Edge->getSrc() == ThisRef) ? Edge->getSink() : Edge->getSrc();
  const HLDDNode *const ThisNode  = ThisRef->getHLDDNode();
  const HLDDNode *const OtherNode = OtherRef->getHLDDNode();
  const bool OtherInThen = If->isThenChild(OtherNode);

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

/// Return the lowest common If condition that is parent of the input edge.
/// Else, return nullptr since the common parent will be the inner loop.
static const HLIf *getCommonParentIf(const DDEdge *Edge) {

  const HLDDNode *const SinkNode = Edge->getSink()->getHLDDNode();
  const HLDDNode *const SrcNode = Edge->getSrc()->getHLDDNode();

  auto *CommonParent =
      HLNodeUtils::getLexicalLowestCommonAncestorParent(SinkNode, SrcNode);

  return dyn_cast<HLIf>(CommonParent);
}

/// Determines whether \p Edge should prevent \p Ref from \p RefSet from being
/// hoisted/sunk from \p If according to \p DDG.
static bool edgeBlocksMotion(const DDEdge *Edge, const RegDDRef *Ref,
                             const HoistOrSinkSet &RefSet, const DDGraph &DDG) {

  // This function will check what is the relationship between the input edge
  // and the input ref with respect to the lowest common parent if.
  // For example:
  //
  //     if() {
  // <1>     a[i] =
  //     }
  //     else {
  // <2>   a[i] =
  //       if () {
  //       }
  //       else {
  // <3>     a[i] =
  //       }
  //     }
  //
  // Assume that we are analyzing a[i] in the outer else branch (<2>), and the
  // edge found is a[i] in the inner else branch (<3>). These two refs will be
  // considered as intra-if since the common If is the outer condition, and
  // both can be reached through the outer else. Since they are intra-if, we
  // need to check that a[i] from the edge (<3>) is in the RefSet. This edge
  // won't be in the set since the visitor won't collect it. Therefore, Ref
  // can't be sinked.

  // Collect the lowest common parent If (outer If refering to the example
  // above).
  auto *If = getCommonParentIf(Edge);
  if (!If)
    return false;

  // If the edge is not intra-if, it isn't a problem for hoisting/sinking this
  // ref.
  if (!IsLegalIntraIfEdge(Edge, If, Ref))
    return false;

  // If the other end of the edge is equivalent and part of the same hoist/sink
  // set, it will be hoisted/sunk along with this ref and so is also safe.
  // Both ends of the edge will always be RegDDRefs because they are MemRefs.
  const auto *const Src  = cast<RegDDRef>(Edge->getSrc());
  const auto *const Sink = cast<RegDDRef>(Edge->getSink());
  const RegDDRef *const Other = (Ref == Src) ? Sink : Src;
  if (RefSet.contains(Other))
    return false;

  // Otherwise, this edge should block the ref from being hoisted/sunk.
  return true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// Prints the edge tests performed when attempting to hoist/sink \p Refs from
/// \p RefSet out of \p If according to \p DDG using \p Out.
static void printEdgeTests(formatted_raw_ostream &Out,
                           const SmallVectorImpl<RegDDRef *> &Refs,
                           const HoistOrSinkSet &RefSet, const DDGraph &DDG) {
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
      const bool EdgeBad = edgeBlocksMotion(Edge, Ref, RefSet, DDG);

      if (EdgeBad)
        dbgs() << "BAD: ";
      else
        dbgs() << "     ";
      Edge->print(dbgs());
    }
  }
}

void HoistOrSinkSet::printEdgeTests(formatted_raw_ostream &Out,
                                    const DDGraph &DDG) const {
  ::printEdgeTests(Out, ThenRefs, *this, DDG);
  ::printEdgeTests(Out, ElseRefs, *this, DDG);
}

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

/// Determines whether \p Ref can be hoisted/sunk from \p If according to \p
/// DDG.
static bool canHoistOrSink(const RegDDRef *Ref, const HoistOrSinkSet &RefSet,
                           const DDGraph &DDG) {

  const bool Incoming = Ref->isRval();

  for (const DDEdge *const Edge :
       Incoming ? DDG.incoming(Ref) : DDG.outgoing(Ref)) {
    if (edgeBlocksMotion(Edge, Ref, RefSet, DDG))
      return false;
  }

  return true;
}

// Go through all the refs in the current set and check if they can be hoisted
// or sinked from the input If. If at least one ref can't be hoisted, then
// return false. This is used for checking nested Ifs. We can hoist or sink
// a reference if all the inner references can be hoisted or sinked.
static bool allRefsCanBeHoistedSinked(SmallVectorImpl<RegDDRef *> &Refs,
                                      HoistOrSinkSet &RefSet,
                                      const DDGraph &DDG) {
  for (auto *Ref : Refs) {
    if (!canHoistOrSink(Ref, RefSet, DDG))
      return false;
  }
  return true;
}

bool HoistOrSinkSet::isLegallyHoistableOrSinkable(const DDGraph &DDG) {
  // If all the refs can't be hoisted or sinked, then the set will be
  // considered as conditional
  if (!allRefsCanBeHoistedSinked(ThenRefs, *this, DDG) ||
      !allRefsCanBeHoistedSinked(ElseRefs, *this, DDG))
    return false;

  return true;
}

bool HoistOrSinkSet::checkAndAssignCommonTemp(HoistOrSinkSet &HoistSet,
                                              HoistOrSinkSet &SinkSet) {
  assert(HoistSet.isUnconditional());
  const RegDDRef *const HoistRef = HoistSet.ThenRefs.front();
  assert(HoistRef->isRval());
  assert(SinkSet.isUnconditional());

  // Stores are handled from the end to beginning. The reason is to reduce
  // bitcasts produced when the set have stores with different destination
  // types.
  const RegDDRef *const SinkRef = SinkSet.ThenRefs.back();
  assert(SinkRef->isLval());

  // Skip any sets that already have a common temp set.
  if (HoistSet.CommonTemp)
    return false;
  if (SinkSet.CommonTemp)
    return false;

  // Set a common temp if the refs are equivalent.
  //
  // TODO: This may need to be updated for areEquivalentAccesses() in the
  // future (CMPLRLLVM-45919).
  if (DDRefUtils::areEqual(HoistRef, SinkRef)) {
    HLNodeUtils &HNU = HoistRef->getHLDDNode()->getHLNodeUtils();
    HoistSet.CommonTemp =
        HNU.createTemp(HoistRef->getDestType(), "cldst.motioned");
    SinkSet.CommonTemp = HoistSet.CommonTemp->clone();
    return true;
  }

  return false;
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
      RefNode = HIRTransformUtils::replaceOperand(Ref, NewRef);
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
    HIRTransformUtils::replaceOperand(Ref, Bitcast->getLvalDDRef()->clone());

    return NewRef;
  }
}

void HoistOrSinkSet::hoistOrSinkFrom(HLIf *If) {
  assert(isUnconditional());
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
        HNU.createLoad(ThenRefs.front()->clone(), "cldst.hoisted", CommonTemp);
    HLNodeUtils::insertBefore(If, HoistLoadInst);
    HoistedOrSunk = HoistLoadInst->getLvalDDRef();
  } else {
    RegDDRef *const NewSunkRef =
        CommonTemp
            ? CommonTemp
            : HNU.createTemp(ThenRefs.back()->getDestType(), "cldst.sunk");
    HLInst *const SunkStoreInst =
        HNU.createStore(NewSunkRef, "", ThenRefs.back()->clone());
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
    HIRTransformUtils::replaceOperand(Ref, HoistedOrSunk->clone());
  }
  for (RegDDRef *Ref : ElseRefs) {
    Ref = insertBitcastIfNeeded(Ref, HoistedOrSunk->getDestType());
    HIRTransformUtils::replaceOperand(Ref, HoistedOrSunk->clone());
  }
}

static void removeIllegalSets(SmallVectorImpl<HoistOrSinkSet> &Sets,
                              const DDGraph &DDG) {
  Sets.erase(remove_if(Sets,
                       [DDG](HoistOrSinkSet &Set) {
                         bool RemoveSet =
                             !Set.isLegallyHoistableOrSinkable(DDG);
                         LLVM_DEBUG({
                           if (RemoveSet) {
                             dbgs() << "Removing illegal set: ";
                             Set.dump();
                             dbgs() << "\n";
                           }
                         });
                         return RemoveSet;
                       }),
             std::end(Sets));
}

// Add the optimization report remarks if the transformation will be applied
// to the input HoistSinkSets. RemarkID used:
//
//   25589u = remark for load hoisted
//   25590u = remark for store sunk
static void addOptReportRemark(HLIf *If, HLLoop *ParentLoop,
                               SmallVectorImpl<HoistOrSinkSet> &HoistSinkSets,
                               OptRemarkID RemarkID) {

  if (HoistSinkSets.empty())
    return;

  OptReportBuilder &ORBuilder =
      If->getHLNodeUtils().getHIRFramework().getORBuilder();

  if (!ORBuilder.isOptReportOn())
    return;

  unsigned NumLoadsOrStores = 0;
  for (auto &Set : HoistSinkSets) {
    unsigned NumRefs = Set.getNumMemRefs();
    assert(NumRefs != 0 && "Found a HoistOrSinkSet with empty refs");
    NumLoadsOrStores += NumRefs;
  }

  auto &DebugLoc = If->getDebugLoc();
  unsigned LineNumber = DebugLoc ? DebugLoc.getLine() : 0;
  ORBuilder(*ParentLoop)
      .addRemark(OptReportVerbosity::Low, RemarkID, NumLoadsOrStores,
                 LineNumber);
}

/// Performs conditional load/store motion on \p If using \p HDDA.
///
/// This will return true if any memory operations were hoisted/sunk. \p If must
/// be inside of a loop and \p ParentLoop must be its immediate parent loop.
static bool runOnIf(HLIf *If, SmallVectorImpl<HoistOrSinkSet> &HoistLoads,
                    SmallVectorImpl<HoistOrSinkSet> &SinkStores,
                    HIRDDAnalysis &HDDA, HLLoop *ParentLoop) {
  assert(ParentLoop);
  assert(If->getParentLoop() == ParentLoop);
  assert(!(HoistLoads.empty() && SinkStores.empty()) &&
         "Both sets can't be empty");
  LLVM_DEBUG({
    dbgs() << "Examining this HLIf for conditional load/store motion:\n\n";
    If->print(fdbgs(), 0);
  });

  LLVM_DEBUG({
    dbgs() << "\nCandidate load sets:\n";
    for (const HoistOrSinkSet &Loads : HoistLoads) {
      dbgs() << "  ";
      Loads.print(fdbgs());
      dbgs() << "\n";
    }
    dbgs() << "\nCandidate store sets:\n";
    for (const HoistOrSinkSet &Stores : SinkStores) {
      dbgs() << "  ";
      Stores.print(fdbgs());
      dbgs() << "\n";
    }
  });

  const DDGraph &DDG = HDDA.getGraph(ParentLoop);
  // If requested, print the edge tests.
  LLVM_DEBUG({
    if (PrintEdgeTests) {
      dbgs() << "\nChecking edges:\n";
      for (HoistOrSinkSet &Loads : HoistLoads)
        Loads.printEdgeTests(fdbgs(), DDG);
      for (HoistOrSinkSet &Stores : SinkStores)
        Stores.printEdgeTests(fdbgs(), DDG);
    }
  });

  // Determine which loads/stores can be hoisted/sunk.
  removeIllegalSets(HoistLoads, DDG);
  removeIllegalSets(SinkStores, DDG);

  // If there aren't any, give up here.
  if (HoistLoads.empty() && SinkStores.empty()) {
    LLVM_DEBUG(dbgs() << "\nNo loads/stores to hoist/sink!\n\n");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "\nWill hoist loads:\n";
    for (const HoistOrSinkSet &Loads : HoistLoads) {
      dbgs() << "  ";
      Loads.print(fdbgs());
      dbgs() << "\n";
    }
    dbgs() << "\nWill sink stores:\n";
    for (const HoistOrSinkSet &Stores : SinkStores) {
      dbgs() << "  ";
      Stores.print(fdbgs());
      dbgs() << "\n";
    }
    dbgs() << "\n";
  });

  // Match up any equivalent hoist/sink sets and assign common temps.
  for (HoistOrSinkSet &Stores : SinkStores) {
    for (HoistOrSinkSet &Loads : HoistLoads)
      if (HoistOrSinkSet::checkAndAssignCommonTemp(Loads, Stores))
        break;
  }

  // Add the opt-report remarks
  addOptReportRemark(If, ParentLoop, HoistLoads,
                     OptRemarkID::HoistedConditionalLoads);
  addOptReportRemark(If, ParentLoop, SinkStores,
                     OptRemarkID::SunkConditionalStores);

  // Do the hoisting/sinking for eligible refs.
  for (HoistOrSinkSet &Loads : HoistLoads)
    Loads.hoistOrSinkFrom(If);
  for (HoistOrSinkSet &Stores : SinkStores)
    Stores.hoistOrSinkFrom(If);

  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(If);
  return true;
}

/// Performs conditional load/store motion using the given analysis results.
/// We currently support hoisting and sinking references if they appear in all
/// paths of the If. In other words, the references must exists in the Then and
/// Else branches to apply the transformation. For example:
///
///   if() {
///     ref1
///   } else {
///     ref1
///   }
///
/// In the example above, we can hoist/sink ref1 because it is available in the
/// true and false paths.
///
/// This pass also support nested conditionals, while the reference has
/// unconditional references acress the whole chain of conditions. For example:
///
///   if() {
///     ref1
///   } else {
///     if () {
///       ref1
///     }
///     else {
///       ref1
///     }
///   }
///
/// The above case is supported for hoisting or sinking since ref1 can be found
/// in the then and else branches of the inner If, and the outer If contains a
// reference to ref1 too.
///
/// Another example:
///
///   if() {
///     ref1
///   } else {
///     if () {
///       ref1
///     }
///     ref1
///   }
///
/// This case is supported since the outer Else branch has an unconditional
/// reference to ref1.
///
/// TODO: The following case is not supported:
///
///   if() {
///   } else {
///     if () {
///       ref1
///     }
///     else {
///       ref1
///     }
///   }
///
/// The references to ref1 in the inner If can be hoisted or sinked at the
/// outer else level.
static bool runConditionalLoadStoreMotion(HIRFramework &HIRF,
                                          HIRDDAnalysis &HDDA,
                                          HIRLoopStatistics &HLS) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << OPT_DESC " Disabled\n");
    return false;
  }

  // Run conditional load/store motion on the innermost loops.
  SmallVector<HLLoop *, 16> InnerLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(InnerLoops);
  bool Changed = false;
  for (HLLoop *const InnerLoop : InnerLoops) {
    const LoopStatistics &LoopStats = HLS.getSelfStatistics(InnerLoop);
    if (!LoopStats.hasIfs())
      continue;
    HoistSinkSetBuilder HoistSinkCandidates(InnerLoop);
    HLNodeUtils::visitRange(HoistSinkCandidates, InnerLoop->child_begin(),
                            InnerLoop->child_end());

    bool LoopChanged = false;
    for (auto &Entry : HoistSinkCandidates.getHoistSinkSetsCollected()) {
      auto *If = Entry.first;
      auto &LoadsSet = Entry.second.Loads;
      auto &StoresSet = Entry.second.Stores;
      if (runOnIf(If, LoadsSet, StoresSet, HDDA, InnerLoop))
        LoopChanged = true;
    }
    if (LoopChanged) {
      HIRInvalidationUtils::invalidateBody(InnerLoop);
      Changed = true;
    }
  }

  return Changed;
}

PreservedAnalyses HIRConditionalLoadStoreMotionPass::runImpl(
    Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  ModifiedHIR =
      runConditionalLoadStoreMotion(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                                    AM.getResult<HIRLoopStatisticsAnalysis>(F));
  return PreservedAnalyses::all();
}
