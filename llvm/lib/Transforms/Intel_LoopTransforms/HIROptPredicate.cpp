//===--- HIROptPredicate.cpp - Implements OptPredicate class --------------===//
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
// This file implements HIROptPredicate class which performs if switching for
// a HIR loop.
//
// Before:
// for(i=0; i<n; i++) {
//   A;
//   if(Cond1) {
//    B;
//   }
//   C;
// }
//
// After:
// if(Cond1) {
//  for(i=0; i<n; i++) {
//   A; B; C;
//  }
// } else {
//  for(i=0; i<n; i++) {
//   A; C;
//  }
// }
//
//===----------------------------------------------------------------------===//

// TODO:
// 1. Handle loops with internal branches and update gotos and label within
//    the parent loop of opt-predicate.
// 2. Handle IV's inside the predicate. This will need to create separate blocks
//    of ranged loops depending on the loop condition.
// 3. Do deep predicate analysis where conditions can be proved to be always
//    true or always false. There may be many other mathematical simplification
//    possible and extensions.
// 4. Possible future extensions would be to support memory references inside
//    the HLIf condition for OptPredicate.
// 5. Add opt report messages.

#include "llvm/Transforms/Intel_LoopTransforms/HIROptPredicatePass.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#if INTEL_FEATURE_CSA
#include "llvm/TargetParser/Triple.h"
#endif // INTEL_FEATURE_CSA

#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNodeMapper.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#define OPT_SWITCH "hir-opt-predicate"
#define OPT_DESC "HIR OptPredicate"
#define DEBUG_TYPE OPT_SWITCH
#define LLVM_DEBUG_DETAIL(X) DEBUG_WITH_TYPE(OPT_SWITCH "-detail", X)

using namespace llvm;
using namespace llvm::loopopt;

STATISTIC(ConditionsUnswitched,
          "Number of conditional branches optimized (hoisted)");
STATISTIC(ConditionsPUUnswitched,
          "Number of conditional branches partially optimized (hoisted)");
// This will also count if the loop has been analyzed multiple times
// due to nested/multiple if.
STATISTIC(ConditionsAnalyzed,
          "Number of conditional branches analyzed for loop unswitching");

constexpr unsigned DefaultNumPredicateThreshold = 3;

static cl::opt<unsigned> NumPredicateThreshold(
    OPT_SWITCH "-threshold", cl::init(DefaultNumPredicateThreshold), cl::Hidden,
    cl::desc("Don't opt predicate a loop which has been transformed greater"
             "than this threshold."));

static cl::opt<bool>
    DisableLoopUnswitch("disable-" OPT_SWITCH, cl::init(false), cl::Hidden,
                        cl::desc("Disable HIR Loop unswitching"));

static cl::opt<bool> DisableCostModel("disable-" OPT_SWITCH "-cost-model",
                                      cl::init(false), cl::Hidden,
                                      cl::desc("Disable " OPT_DESC
                                               " cost model related checks"));

static cl::opt<bool> DisablePartialUnswitch("disable-" OPT_SWITCH "-pu",
                                            cl::init(false), cl::Hidden,
                                            cl::desc("Disable " OPT_DESC
                                                     " partial unswitch"));

static cl::opt<bool>
    EarlyPredicateOptOption(OPT_SWITCH "-early-opt", cl::init(false),
                            cl::Hidden,
                            cl::desc(OPT_DESC " with special options during "
                                     "early pass"));

constexpr unsigned DefaultMaxCasesThreshold = 8;

static cl::opt<unsigned> MaxCasesThreshold(
    OPT_SWITCH "-max-cases-threshold", cl::init(DefaultMaxCasesThreshold),
    cl::Hidden, cl::desc("Don't apply opt predicate in a switch instruction if"
    "the number of cases is larger than this threshold."));

constexpr unsigned DefaultMaxIfsInLoopThreshold = 7;

static cl::opt<unsigned> MaxIfsInLoopThreshold(
    OPT_SWITCH "-max-ifs-in-loop-threshold",
    cl::init(DefaultMaxIfsInLoopThreshold), cl::Hidden, cl::desc("Don't apply "
    "opt predicate in a switch instruction if the number of Ifs in the parent "
    "loop is larger than this threshold."));

constexpr unsigned DefaultRegionCostThreshold = 3;

static cl::opt<unsigned> RegionCostThreshold(
    OPT_SWITCH "-max-region-cost", cl::init(DefaultRegionCostThreshold),
    cl::Hidden, cl::desc("Don't apply opt predicate of an If instruction if "
    "the number of inner loops is larger than this threshold."));

// NOTE: This reduces the cost for applying unswitching for Switch
// instructions.
static cl::opt<bool> UseReducedSwitchCost(
    OPT_SWITCH "-use-reduced-switch-cost", cl::init(false), cl::Hidden,
    cl::desc("Reduce the cost of switches for the unswitching in "
             "order to enable more unswitching"));

// Disable converting Select instructions into If/Else to perform unswitching
static cl::opt<bool> DisableUnswitchSelect("disable-" OPT_SWITCH "-select",
                                            cl::init(false), cl::Hidden,
                                            cl::desc("Disable " OPT_DESC
                                                     " for select "
                                                     "instructions"));

// Disable the optimization for SIMD cases
static cl::opt<bool> DisableUnswitchSIMD("disable-" OPT_SWITCH "-simd",
                                         cl::init(false), cl::Hidden,
                                         cl::desc("Disable " OPT_DESC
                                                  " when the loop is "
                                                  "inside SIMD directives"));

namespace {

// Structure to handle the information if the condition can be partially
// unswitched. Current cases that can be handled:
//
//   LoadPUC: This partial unswitching will be considered when a temp that
//            loads from the same memref inside a loop is used in the candidate
//            condition, and the memref can be modified inside the Then or Else
//            branch. In this case the branch that won't modify the memref will
//            be unswitched. The new condition will only contain the predicate
//            with the candidate temp, the loop with branch that won't modify
//            the memref in one branch of the new condition, and the original
//            loop in the other branch. For example:
//
//     HIR before transformation:
//
//       DO i1 = 0, 5
//         %0 = a[0]
//         if (%0 != 1) {
//           a[i1] = 1
//           ...
//         } else {
//           %t = %t + i1
//         }
//       END DO
//
//     HIR after transformation:
//
//       %0 = a[0]
//       if (%0 == 1) {
//         DO i1 = 0, 5
//           %t = %t + i1
//         END DO
//       } else {
//         DO i1 = 0, 5
//           %0 = a[0]
//           if (%0 != 1) {
//             a[i1] = i1
//             ...
//           } else {
//             %t = %t + i1
//           }
//         END DO
//       }
//
//   InvariantPredPUC: The whole condition can't be hoisted, but one
//                     predicate can be moved hoisted. If there are multiple
//                     predicates, then the predicate that can be hoisted to
//                     the outermost loop will be selected. Also, all
//                     predicates should be bitwise 'AND'd. For example:
//
//     HIR before transformation:
//
//       BEGIN REGION { }
//             + DO i1 = 0, 99, 1   <DO_LOOP>
//             |   if (i1 != 0 & %t > 1)
//             |   {
//             |      (%A)[i1] = i1;
//             |   }
//             + END LOOP
//       END REGION
//
//     HIR after transformation
//
//       BEGIN REGION { modified }
//             if (%t > 1)
//             {
//                + DO i1 = 0, 99, 1   <DO_LOOP>
//                |   if (i1 != 0)
//                |   {
//                |      (%A)[i1] = i1;
//                |   }
//                + END LOOP
//             }
//       END REGION
//
struct PUCandidate {

  // Handle the possible types that the PUCandidate can be. Only one type can
  // be set.
  enum PUCType {
    LoadPUC,          // Load used for condition is modified in one branch
    InvariantPredPUC, // One predicate in the condition can be hoisted
    None              // Not candidate for partial unswitch
  };

  PUCType Type;

  SmallPtrSet<HLInst *, 8> LoadInstructions;

  bool IsLoadUpdatedInThenBranch;
  bool IsLoadUpdatedInElseBranch;

  // Position of the predicate the could be removed if the type is
  // InvariantPredPUC. The numbering starts at 0 (first predicate).
  unsigned InvariantPredPos;

public:
  PUCandidate() { reset(); }

  PUCandidate(const PUCandidate &Arg, const HLNodeMapper &Mapper)
      : Type(Arg.Type),
        IsLoadUpdatedInThenBranch(Arg.IsLoadUpdatedInThenBranch),
        IsLoadUpdatedInElseBranch(Arg.IsLoadUpdatedInElseBranch),
        InvariantPredPos(Arg.InvariantPredPos) {
    for (HLInst *Inst : Arg.LoadInstructions) {
      // Skip instructions that are already removed.
      if (!Inst->getParent()) {
        continue;
      }

      HLInst *ClonedInst = Mapper.getMapped(Inst);
      assert(ClonedInst && "Nullptr instruction");
      LoadInstructions.insert(ClonedInst);
    }
  }

  SmallPtrSetImpl<HLInst *> &getLoadInstructions() {
    assert(Type == PUCType::LoadPUC &&
           "Trying to collect Load instructions from a non-Load PUC");
    return LoadInstructions;
  }

  void setLoadPUC(SmallPtrSetImpl<HLInst *> &LoadPUCInstructions) {
    // An If condition can have multiple prodicates that can be candidates for
    // Load PU.
    assert(Type == PUCType::None && "Trying to change LoadPUC type");
    Type = PUCType::LoadPUC;
    LoadInstructions.insert(LoadPUCInstructions.begin(),
                            LoadPUCInstructions.end());
  }

  void setInvariantPredPUC(unsigned PredPos) {
    assert(Type == PUCType::None && "Trying to change InvariantPredPUC type");
    Type = PUCType::InvariantPredPUC;
    InvariantPredPos = PredPos;
  }

  void reset() {
    Type = PUCType::None;
    InvariantPredPos = 0;
    LoadInstructions.clear();
    IsLoadUpdatedInThenBranch = false;
    IsLoadUpdatedInElseBranch = false;
  }

  bool isSet() const { return Type != PUCType::None; }
  bool isInvariantPredPUC() const { return Type == PUCType::InvariantPredPUC; }
  bool isLoadPUC() const { return Type == PUCType::LoadPUC; }

  PUCType getType() const { return Type; }
  unsigned getInvariantPredicatePos() const {
    assert(Type == PUCType::InvariantPredPUC &&
           "Trying to get the invariant predicate in an invalid type");
    return InvariantPredPos;
  }

#ifndef NDEBUG
  LLVM_DUMP_METHOD
  void dump() {
    dbgs() << "[ ";

    dbgs() << (IsLoadUpdatedInThenBranch ? "T" : "F");
    dbgs() << "/";
    dbgs() << (IsLoadUpdatedInElseBranch ? "T" : "F");
    dbgs() << " ";

    for (HLInst *Inst : LoadInstructions) {
      dbgs() << "<" << Inst->getNumber() << "> ";
    }
    dbgs() << "]";
  }
#endif
};

struct PUContext : PUCandidate {
  SmallPtrSet<const RegDDRef *, 8> VisitedRefs;
};

struct HoistCandidate {

  // Enum to handle which type of condition is the candidate
  enum UnswitchType {
    If,           // HLIf
    Switch,       // HLSwitch
    Select,       // HLInst where the LLVM instruction is a SelectInst
    Bottom        // Anything else that is not supported
  };

  HLDDNode *Node;
  unsigned Level;
  UnswitchType UType;

  PUCandidate PUC;
  bool CreatedFromSelect;
  bool HoistingIncreasesCodeSize;

  HoistCandidate(HLDDNode *Node, unsigned Level, const PUCandidate &PUC,
                 bool HoistingIncreasesCodeSize = true)
      : Node(Node), Level(Level), PUC(PUC), CreatedFromSelect(false),
        HoistingIncreasesCodeSize(HoistingIncreasesCodeSize) {
    UType = Bottom;
    if (isa<HLIf>(Node)) {
      UType = UnswitchType::If;
    } else if (isa<HLSwitch>(Node)) {
      UType = UnswitchType::Switch;
    } else {
      auto Inst = cast<HLInst>(Node);
      assert(isa<SelectInst>(Inst->getLLVMInstruction()) &&
             "Trying to create a unswitch hoist candidate that is not a "
             "select instruction");
      (void) Inst;
      UType = UnswitchType::Select;
    }
  }

  HoistCandidate(const HoistCandidate &Orig, HLDDNode *CloneNode,
                 const HLNodeMapper &Mapper)
      : Node(CloneNode), Level(Orig.Level), UType(Orig.UType),
        PUC(Orig.PUC, Mapper), CreatedFromSelect(Orig.CreatedFromSelect),
        HoistingIncreasesCodeSize(Orig.HoistingIncreasesCodeSize) {}

  bool isIf() const { return UType == UnswitchType::If; }
  bool isSwitch() const { return UType == UnswitchType::Switch; }
  bool isSelect() const { return UType == UnswitchType::Select; }
  bool isValidCandidate() const { return UType != UnswitchType::Bottom; }
  HLDDNode *getNode() const { return Node; }
  HLIf *getIf() const { return cast<HLIf>(Node); }
  HLSwitch *getSwitch() const { return cast<HLSwitch>(Node); }
  HLInst *getSelect() const { return cast<HLInst>(Node); }
  bool createdFromSelect() const { return CreatedFromSelect; }

  // If the candidate is a Select instruction, then replace the node with an
  // HLIf and set the candidate's type as UnswitchType::If. This function is
  // used when converting a Select instruction into If/Else to do unswitching.
  void convertSelectToIf();

  // If the candidate is for partial unswitching with type
  // PUCType::InvariantPredPUC, then generate an if condition with the
  // predicate that will be hoisted.
  void generatePUCInvariantPredicateIf();

  // Return true if the candidate may contain any side effect that could
  // possibly increase the code size.
  bool mayIncreasesCodeSize() const { return HoistingIncreasesCodeSize; }

  bool operator==(const HoistCandidate &Arg) const { return Node == Arg.Node; }

#ifndef NDEBUG
  LLVM_DUMP_METHOD
  void dump() {
    dbgs() << "{";
    if (isIf()) {
      getIf()->dumpHeader();
    } else if (isSwitch()) {
      getSwitch()->dumpHeader();
    } else if (isSelect()) {
      getSelect()->dump();
    } else {
      llvm_unreachable("Trying to print an incorrect hoist candidate");
    }
    dbgs() << ", L: " << Level << ", PU: ";
    PUC.dump();
    if (CreatedFromSelect)
      dbgs() << ", S";
    dbgs() << "}";
  }
#endif
};

template <typename IterT>
static void updateDefLevels(HLDDNode *Node, IterT Begin, IterT End) {
  unsigned Level = Node->getNodeLevel();

  // Update Node's refs.
  for (auto Ref : make_range(Node->ddref_begin(), Node->ddref_end())) {
    Ref->updateDefLevel(Level);
  }

  // Keep the set of Refs whose BlobDDRefs were updated.
  SmallPtrSet<RegDDRef *, 16> RefsToUpdate;

  // handle each loop clone separately.
  ForEach<HLLoop>::visitRange<false>(Begin, End, [&](HLLoop *Loop) {
    SmallSet<unsigned, 32> DefSymbases;
    SmallSet<unsigned, 32> UseSymbases;

    SmallVector<DDRef *, 32> Refs;
    DDRefGatherer<DDRef, TerminalRefs | BlobRefs>::gatherRange(
        Loop->child_begin(), Loop->child_end(), Refs);

    for (DDRef *Ref : Refs) {
      auto Symbase = Ref->getSymbase();

      // Skip non-temps. Also skip non-livein references as their level may not
      // be changed by the predicate optimization.
      if ((Ref->isRval() && !Ref->isSelfBlob()) || !Loop->isLiveIn(Symbase)) {
        continue;
      }

      if (Ref->isLval()) {
        DefSymbases.insert(Symbase);
      } else if (Ref->getDefinedAtLevel() > Level) {
        // Account only non-linear @ Level references.
        UseSymbases.insert(Symbase);
      }
    }

    // Remove symbases that have definitions inside the loop. They should
    // preserve their non-linear definition level.
    for (unsigned Symbase : DefSymbases) {
      UseSymbases.erase(Symbase);
    }

    if (UseSymbases.empty()) {
      // Nothing to update.
      return;
    }

    for (auto *Ref : Refs) {
      // Update only the refs with linear uses inside the loop.
      if (!UseSymbases.count(Ref->getSymbase())) {
        continue;
      }

      if (auto *BRef = dyn_cast<BlobDDRef>(Ref)) {
        BRef->setDefinedAtLevel(Level);
        RefsToUpdate.insert(BRef->getParentDDRef());
      } else {
        RegDDRef *RRef = cast<RegDDRef>(Ref);
        if (RRef->isSelfBlob()) {
          RRef->getSingleCanonExpr()->setDefinedAtLevel(Level);
        }
      }
    }
  });

  // Update RegDDRefs those BlobDDRefs were updated.
  for (auto *Ref : RefsToUpdate) {
    Ref->updateDefLevel();
  }
}

// The following map is used to store children nodes of switches and ifs,
// indexed by a case or a branch.
//
// {Label: {Node: [List of children nodes]}}
//
// Where Label is an int64_t constant which may represent either a switch
// case or an if branch, and Node may be HLIf or HLSwitch.
using NodeContainerMapTy = SmallDenseMap<HLDDNode *, HLContainerTy>;
using CaseNodeContainerMapTy = SmallDenseMap<int64_t, NodeContainerMapTy>;

class HIROptPredicate {
public:

  HIROptPredicate(HIRFramework &HIRF, HIRDDAnalysis &DDA,
                  HIRLoopStatistics &HLS, bool EnablePartialUnswitch,
                  bool EarlyPredicateOpt)
      : HIRF(HIRF), DDA(DDA), HLS(HLS), BU(HIRF.getBlobUtils()),
        EnablePartialUnswitch(EnablePartialUnswitch && !DisablePartialUnswitch),
        EarlyPredicateOpt(EarlyPredicateOpt || EarlyPredicateOptOption) {}

  bool run();

private:
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;
  HIRLoopStatistics &HLS;
  BlobUtils &BU;

  bool EnablePartialUnswitch;
  bool EarlyPredicateOpt;

  struct CandidateLookup;
  struct RegionSizeCalculator;
  class LoopUnswitchNodeMapper;

  SmallDenseMap<const HLLoop *, unsigned, 16> ThresholdMap;
  SmallVector<HoistCandidate, 16> Candidates;

  // Store the HLIfs that are in outer loops and maybe hoisted. The boolean
  // entry is to identify if applying the transformation will increase code
  // size or not.
  SmallDenseMap<const HLIf *, bool, 4> OuterLoopIfs;

  // Set used to track which regions has been modified by the optimization.
  SmallSetVector<const HLRegion *, 16> RegionThresholdSet;

  // Opt-report chunk number
  unsigned VNum = 1;

  // Sets including visited TargetLoop, visited NewElseLoop and If line nums
  SmallPtrSet<HLLoop *, 8> OptReportVisitedSet;

  /// Returns true if the non-linear rval Ref of the If condition may be
  /// unswitched together with its definition. Also, set the PU as required.
  bool
  checkForLoadPUCandidate(const HLIf *If, const RegDDRef *Ref, PUContext &PU,
                          SmallPtrSet<HLInst *, 8> &LoadInstructions) const;

  /// Returns true of false whatever \p Edge prevents unswitching of non-linear
  /// condition. Populates PU and RefsStack.
  bool processPUEdge(const HLIf *If, DDEdge *Edge, PUContext &PU,
                     SmallVectorImpl<const RegDDRef *> &RefsStack, DDGraph &DDG,
                     SmallPtrSetImpl<HLInst *> &LoadInstructions) const;

  /// Sorts candidates in top sort number descending.
  /// They will be handled from right to left.
  void sortCandidates();

  /// Clears opt-report structures between HIR regions.
  void clearOptReportState();

  /// This routine will loop through the candidate loop to look
  /// for HLIf candidates. If all conditions are met, it will transform
  /// the loop. Returns true if transformation happened.
  bool processOptPredicate();

  /// Returns the deepest level at which any of the If/Ref operands is defined.
  unsigned getPossibleDefLevel(const HLIf *If, PUContext &PUC);
  unsigned getPossibleDefLevel(const HLSwitch *Switch);
  unsigned getPossibleDefLevel(const HLDDNode *Node, const RegDDRef *Ref,
                               bool *NonLinearRef = nullptr);

  /// Returns the possible level where CE is defined.
  ///
  /// UDivBlob is set to true whenever CE contains a non-constant division.
  unsigned getPossibleCEDefLevel(const CanonExpr *CE, bool &UDivBlob);

  // Return true if we should reduce the cost of doing unswitching for Switch
  bool shouldUseReducedSwitchCost(const HLLoop *ParentLoop,
                                  const HLLoop *TargetLoop) const;

  void transformSwitch(HLLoop *TargetLoop,
                       iterator_range<HoistCandidate *> SwitchCandidates,
                       CaseNodeContainerMapTy &Containers,
                       NodeContainerMapTy &DefaultContainer,
                       SmallPtrSetImpl<HLNode *> &TrackClonedNodes,
                       SmallVectorImpl<HoistCandidate> &NewCandidates);

  void transformIf(HLLoop *TargetLoop,
                   iterator_range<HoistCandidate *> IfCandidates,
                   CaseNodeContainerMapTy &CaseContainers,
                   SmallPtrSet<HLNode *, 32> TrackClonedNodes,
                   SmallVectorImpl<HoistCandidate> &NewCandidates);

  /// Transform the original loop.
  void transformCandidate(HLLoop *TargetLoop, HoistCandidate &Candidate);

  /// Extracts the children of HLIf or HLSwitch and stores them
  /// in the containers.
  void extractChildren(HLDDNode *Node, CaseNodeContainerMapTy &Containers,
                       NodeContainerMapTy &DefaultContainer);

  /// Performs the OptPredicate transformation where condition is
  /// hoisted outside and loops are moved inside the condition and updates the
  /// DDRefs inside the HLIf.
  void removeOrHoistIf(HoistCandidate &Candidate, HLLoop *TargetLoop,
                       HLIf *FirstIf, HLIf *If, HLIf *&PivotIf);
  void hoistIf(HLIf *&If, HLLoop *OrigLoop);

  void addPredicateOptReportOrigin(HLLoop *TargetLoop);

  void addPredicateOptReportRemark(
      HLLoop *TargetLoop,
      const iterator_range<HoistCandidate *> &IfOrSwitchCandidates,
      OptRemarkID RemarkID);

  /// Return true if all candidates belong to outer loops. It will be used for
  /// disabling the GenCode flag since unswitching by itself may not be
  /// profitable. If another transformation benefits from it, then it will
  /// enable the GenCode flag.
  bool mustCodeGen() const;

#ifndef NDEBUG
  LLVM_DUMP_METHOD
  void dumpCandidates() {
    dbgs() << "Candidates, count: " << Candidates.size() << "\n";
    for (HoistCandidate &Candidate : Candidates) {
      Candidate.dump();
      dbgs() << "\n";
    }
  }
#endif
};

class HIROptPredicate::LoopUnswitchNodeMapper : public HLNodeToNodeMapperImpl {
  SmallPtrSetImpl<HLNode *> &TrackedNodes;
  decltype(Candidates) &CandidatesRef;
  decltype(Candidates) NewCandidates;

public:
  LoopUnswitchNodeMapper(SmallPtrSetImpl<HLNode *> &TrackedNodes,
                         decltype(Candidates) &CandidatesRef)
      : TrackedNodes(TrackedNodes), CandidatesRef(CandidatesRef) {}

  void map(const HLNode *Node, HLNode *MappedNode) override {
    const HLInst *Inst = dyn_cast<HLInst>(Node);

    if (Inst && TrackedNodes.count(Inst)) {
      NodeMap[Node] = MappedNode;
      return;
    }

    bool IsIfOrSwitch = isa<HLIf>(Node) || isa<HLSwitch>(Node);
    if ((IsIfOrSwitch && TrackedNodes.count(Node)) || isa<HLLabel>(Node)) {
      NodeMap[Node] = MappedNode;
      return;
    }

    if (!IsIfOrSwitch) {
      return;
    }

    HLDDNode *CloneNode = cast<HLDDNode>(MappedNode);
    auto CandidateI = std::find_if(CandidatesRef.begin(), CandidatesRef.end(),
                                   [Node](const HoistCandidate &Candidate) {
                                     return Candidate.getNode() == Node;
                                   });

    // Original condition was found as a candidate, this means that we are still
    // interested in this node - have to save original node and update
    // candidate clone list.
    if (CandidateI != CandidatesRef.end()) {
      NewCandidates.emplace_back(*CandidateI, CloneNode, *this);

      LLVM_DEBUG(dbgs() << "Found new candidate: ");
      LLVM_DEBUG(NewCandidates.back().dump());
      LLVM_DEBUG(dbgs() << "\n");
    }
  }

  decltype(Candidates) &getNewCandidates() { return NewCandidates; }
};

static bool isUnswitchDisabled(HLSwitch *) { return false; }
static bool isUnswitchDisabled(HLIf *If) { return If->isUnswitchDisabled(); }

struct HIROptPredicate::CandidateLookup final : public HLNodeVisitorBase {

  // Helper structure to handle the constraints that wouldn't allow a condition
  // be a candidate.
  struct CandidateConstraints {
    unsigned MinLevel;       // Outermost level allowed to apply unswitching
    bool CanTransformParent; // Parent loop is allowed to be transformed
    bool PUCAllowed;         // Partial unswitching is allowed for the
                             //   current candidate

    // NOTE: This field could be improved by making it as MaxLevel, which
    // represents the innermost loop that could be hoisted into. For now,
    // we only check in some cases if the condition can be fully hoisted.
    bool RequiresLoopnestUnswitch; // Candidates needs to be unswitched to the
                                   //   outermost loop

    CandidateConstraints(unsigned MinLevel, bool CanTransformParent,
                         bool PUCAllowed, bool RequiresLoopnestUnswitch)
        : MinLevel(MinLevel), CanTransformParent(CanTransformParent),
          PUCAllowed(PUCAllowed),
          RequiresLoopnestUnswitch(RequiresLoopnestUnswitch) {}

    CandidateConstraints(CandidateConstraints &Constraints)
        : MinLevel(Constraints.MinLevel),
          CanTransformParent(Constraints.CanTransformParent),
          PUCAllowed(Constraints.PUCAllowed),
          RequiresLoopnestUnswitch(Constraints.RequiresLoopnestUnswitch) {}

    CandidateConstraints &operator=(const CandidateConstraints &Constraints) {
      MinLevel = Constraints.MinLevel;
      CanTransformParent = Constraints.CanTransformParent;
      PUCAllowed = Constraints.PUCAllowed;
      RequiresLoopnestUnswitch = Constraints.RequiresLoopnestUnswitch;
      return *this;
    }
  };

  HLNode *SkipNode;
  HIROptPredicate &Pass;
  HIRLoopStatistics &HLS;
  CandidateConstraints Constraints;
  HLLoop *CurrLoop;
  unsigned CostOfRegion;
  // If we are analyzing a loop, then we are going to keep the first Select
  // instruction that we found. All other Select instructions will be
  // candidates if they have the same condition as this one.
  HLInst *FirstSelectCandidate;

  CandidateLookup(HIROptPredicate &Pass, HIRLoopStatistics &HLS,
                  HLLoop *CurrLoop = nullptr, bool CanTransformParent = true,
                  unsigned MinLevel = 0, unsigned CostOfRegion = 0)
      : SkipNode(nullptr), Pass(Pass), HLS(HLS),
        Constraints(MinLevel, CanTransformParent, true /*PUCAllowed*/,
                    false /*RequiresLoopnestUnswitch*/),
        CurrLoop(CurrLoop), CostOfRegion(CostOfRegion),
        FirstSelectCandidate(nullptr) {}

  CandidateLookup(HIROptPredicate &Pass, HIRLoopStatistics &HLS,
                  CandidateConstraints &NewConstraints,
                  HLLoop *CurrLoop = nullptr, unsigned CostOfRegion = 0)
      : SkipNode(nullptr), Pass(Pass), HLS(HLS), Constraints(NewConstraints),
        CurrLoop(CurrLoop), CostOfRegion(CostOfRegion),
        FirstSelectCandidate(nullptr) {}

  template <typename NodeTy> void visitIfOrSwitch(NodeTy *Node);
  void visit(HLIf *If);
  void visit(HLSwitch *Switch);
  void visit(HLRegion *Reg);
  void visit(HLLoop *Loop);
  void visit(HLInst *Inst);
  bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

private:
  bool isProfitableOuterLoop(ArrayRef<const HLNode *> ChildNodes);
  bool canUnswitchInnerIf(HLIf *If, HLLoop *Loop);
  bool isRestrictedByConstraints(HLNode *Node, unsigned Level, PUContext &PUC,
                                 CandidateConstraints &Constraints);
  bool isTargetLoopValid(HLNode *Node, unsigned Level);
};

// Helper structure used to count the number of nested loops in a region.
// If an HLIf is provided to the constructor then it will collect how many
// times the same If condition is repeated in the nodes.
struct
HIROptPredicate::RegionSizeCalculator final : public HLNodeVisitorBase {
  HLNode *SkipNode;
  HIROptPredicate &Pass;
  unsigned RegionSize;
  HLIf *IfToCompare;
  unsigned NumSameIfs;
  unsigned IfWithElse;

  RegionSizeCalculator(HIROptPredicate &Pass, HLIf *IfToCompare = nullptr)
      : SkipNode(nullptr), Pass(Pass), RegionSize(0),
        IfToCompare(IfToCompare), NumSameIfs(0), IfWithElse(0) {}

  void visit(HLLoop *Loop) { RegionSize++; }

  bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }
  void visit(const HLNode *Node) { }
  void postVisit(const HLNode *Node) {}
  void visit(HLIf *If) {
    // If an HLIf was provided to the constructor than count how many times
    // the same If condition appears in the region
    if (IfToCompare && HLNodeUtils::areEqualConditions(IfToCompare,If))
      NumSameIfs++;

    if (If->hasElseChildren())
      IfWithElse++;
  }

  unsigned getNumSameIfs() { return NumSameIfs; }
  unsigned getRegionSize() { return RegionSize; }
  unsigned getNumOfIfWithElse() { return IfWithElse; }
};
} // namespace

// This function will traverse through the candidates that are Select
// instructions, will convert them into If/Else, and will add the new
// conditional into the Candidates list for unswitching. For example:
//
//   Original Select instruction :
//     %sel = (operand1 PREDICATE operand2) ? true result : false result
//
//   Converted:
//     if (operand1 PREDICATE operand2)
//       %sel = true result
//     else
//       %sel = false result
void HoistCandidate::convertSelectToIf() {
  assert(isSelect() && "Trying to convert a candidate that is not a Select");
  HLInst *Select = getSelect();
  auto *Loop = Select->getParentLoop();
  LLVM_DEBUG({
    dbgs() << "Candidate for converting Select:\n";
    Select->dump();
  });

  auto &HLNU = Loop->getHLNodeUtils();
  // Create the true instruction
  auto *RvalRef = Select->removeOperandDDRef(3);
  auto *LvalRef = Select->removeOperandDDRef(0u);
  HLInst *TrueInst =
      RvalRef->isMemRef() ? HLNU.createLoad(RvalRef, "", LvalRef) :
                            HLNU.createCopyInst(RvalRef, "", LvalRef);

  // Create the false instruction
  RvalRef = Select->removeOperandDDRef(4);
  LvalRef = LvalRef->clone();
  HLInst *FalseInst =
      RvalRef->isMemRef() ? HLNU.createLoad(RvalRef, "", LvalRef) :
                            HLNU.createCopyInst(RvalRef, "", LvalRef);

  auto *OP1 = Select->removeOperandDDRef(1);
  auto *OP2 = Select->removeOperandDDRef(2);
  auto &Pred = Select->getPredicate();
  HLIf *NewIf = HLNU.createHLIf(Pred, OP1, OP2);
  NewIf->setDebugLoc(cast<HLInst>(Node)->getDebugLoc());

  // Update the Select instruction with the new If/Else and set the
  // children. Basically, create:
  //
  // if (operand1 PREDICATE operand2)
  //   %sel = true result
  // else
  //   %sel = false result
  HLNU.insertAsFirstThenChild(NewIf, TrueInst);
  HLNU.insertAsFirstElseChild(NewIf, FalseInst);
  HLNU.replace(Node, NewIf);
  LLVM_DEBUG({
    dbgs() << "Into If/Else:\n";
    NewIf->dump();
  });

  Node = NewIf;
  UType = UnswitchType::If;
  CreatedFromSelect = true;
}

// Create a new If instruction that contains the invariant predicate. The
// new If will replace the node to convert and hoisting will be applied.
// For example, the condition %t > 1 in the following loop can be hoisted:
//
//   + DO i1 = 0, 99, 1   <DO_LOOP>
//   |   if (i1 != 0 && %t > 1)
//   |   {
//   |      (%A)[i1] = i1;
//   |   }
//   + END LOOP
//
// Before applying the hoist, we need to separate the invariant predicate.
// The loop will be cleanup as follows:
//
//   + DO i1 = 0, 99, 1   <DO_LOOP>
//   |   if (%t > 1)
//   |   {
//   |      if (i1 != 0)
//   |      {
//   |         (%A)[i1] = i1;
//   |      }
//   |   }
//   + END LOOP
//
// The transformation phase will use the new If to do the hoisting.
//
//   if (%t > 1)
//   {
//      + DO i1 = 0, 99, 1   <DO_LOOP>
//      |   if (i1 != 0)
//      |   {
//      |      (%A)[i1] = i1;
//      |   }
//      + END LOOP
//   }
//
void HoistCandidate::generatePUCInvariantPredicateIf() {
  assert(PUC.isInvariantPredPUC() &&
         "Trying to generate an HLIf for a non-invariant "
         "predicate condition");

  auto *If = cast<HLIf>(Node);

  // Find the invariant predicate
  auto CurrPred = If->pred_begin() + PUC.getInvariantPredicatePos();

  // Move the condition
  RegDDRef *Ref1 = If->removeLHSPredicateOperandDDRef(CurrPred);
  RegDDRef *Ref2 = If->removeRHSPredicateOperandDDRef(CurrPred);
  auto &Pred = *CurrPred;

  // Generate the new If condition
  auto &HLNU = If->getHLNodeUtils();
  HLIf *NewIf = HLNU.createHLIf(Pred, Ref1, Ref2);

  // We need to clone the Else branch. The reason is that if the condition
  // have an Else, then we need to make sure it is taken. For example:
  //
  //   + DO i1 = 0, 99, 1   <DO_LOOP>
  //   |   if (i1 != 0 && %t > 1)
  //   |   {
  //   |      (%A)[i1] = i1;
  //   |   }
  //   |   else
  //   |   {
  //   |      (%A)[i1] = i1 + 1;
  //   |   }
  //   + END LOOP
  //
  // If %t is less or equal than 1, or if i1 is not equal 0, then the Else
  // branch will be taken. We need to generate the cleanup as follows:
  //
  //   + DO i1 = 0, 99, 1   <DO_LOOP>
  //   |   if (%t > 1)
  //   |   {
  //   |      if (i1 != 0)
  //   |      {
  //   |         (%A)[i1] = i1;
  //   |      }
  //   |      else
  //   |      {
  //   |         (%A)[i1] = i1 + 1;
  //   |      }
  //   |   }
  //   |   else
  //   |   {
  //   |      (%A)[i1] = i1 + 1;
  //   |   }
  //   + END LOOP
  //
  // This means, if the condition %t > 1 is true, but i1 != 0 is false, then we
  // need to respect the Else branch.
  //
  // TODO: The clone sequence needs to use the LoopUnswitchNodeMapper to map
  // any node that is a candidate and is inside the Else branch.
  HLContainerTy ElseClones;
  if (If->hasElseChildren())
    HLNodeUtils::cloneSequence(&ElseClones, If->getFirstElseChild(),
                               If->getLastElseChild());

  // Remove the invariant predicate from the old If
  If->removePredicate(CurrPred);

  // Set the Else branch if needed
  if (!ElseClones.empty())
    HLNodeUtils::insertAsFirstElseChildren(NewIf, &ElseClones);

  HLNodeUtils::insertBefore(If, NewIf);

  // Set the old If as the Then branch in the new If
  HLNodeUtils::moveAsFirstThenChild(NewIf, If);

  // Reset the information for partial opt-predicate since the new If doesn't
  // have any more predicates.
  PUC.reset();

  // Update the node to transform
  Node = NewIf;
}

bool HIROptPredicate::processPUEdge(
    const HLIf *If, DDEdge *Edge, PUContext &PU,
    SmallVectorImpl<const RegDDRef *> &RefsStack, DDGraph &DDG,
    SmallPtrSetImpl<HLInst *> &LoadInstructions) const {
  if (!Edge->isFlow()) {
    // May ignore non-flow edges.
    return true;
  }

  RegDDRef *SrcRef = cast<RegDDRef>(Edge->getSrc());

  // Source of the flow edge is always an Inst.
  HLInst *Inst = cast<HLInst>(SrcRef->getHLDDNode());

  if (Edge->isBackwardDep()) {
    // VRef is updated from the previous loop iteration

    if (If->isThenChild(Inst)) {
      PU.IsLoadUpdatedInThenBranch = true;
    } else if (If->isElseChild(Inst)) {
      PU.IsLoadUpdatedInElseBranch = true;
    } else {
      // Is updated outside of the If construct
      return false;
    }

    // Is updated in both If branches
    if (PU.IsLoadUpdatedInThenBranch && PU.IsLoadUpdatedInElseBranch) {
      return false;
    }

    return true;
  }

  if (!SrcRef->isTerminalRef()) {
    // To support memory references we also have to take into account output
    // edges.
    // Here is an example:
    //
    // char *%a, int *%b, float *%c
    //
    // DO i1 = 0, %n, 1
    // |   (%c)[i1] = 1.000000e+00;
    // |   (%a)[0] = 1;
    // |   if ((%b)[0] == 100) {
    // |      (%b)[0] = 1;
    // |   }
    // + END LOOP
    //
    // %a-%c, %a-%b may alias
    // %c-%b may not alias
    //
    // To unswitch this loop, both %c and %a assignments should be hoisted.
    return false;
  }

  if (SrcRef->isLiveOutOfParentLoop()) {
    // This is conservative check to handle cases where the flow edge from 2nd
    // definition of t1 may not exist because it's killed by the first
    // definition.
    //
    // DO i1 = 0, %n, 1
    // |   t1 = ...
    // |   if (t1) {
    // |      ...
    // |   }
    // |   if (...) {
    // |      t1 = ...
    // |   }
    // + END LOOP
    return false;
  }

  if (Inst->isUnsafeSideEffectsCallInst()) {
    // Unsafe to speculate
    return false;
  }

  if (SrcRef->isFakeLval()) {
    // Do not handle cases when the condition may be changed by a call.
    return false;
  }

  if (!HLNodeUtils::dominates(Inst, If)) {
    // Handle only simple CFG.
    return false;
  }

  for (auto &SrcEdge : DDG.outgoing(SrcRef)) {
    if (SrcEdge->isOutput()) {
      return false;
    }
  }

  for (auto &SrcEdge : DDG.incoming(SrcRef)) {
    if (SrcEdge->isOutput()) {
      return false;
    }
  }

  LoadInstructions.insert(Inst);

  auto RefsRange = concat<RegDDRef *>(
      make_range(Inst->rval_op_ddref_begin(), Inst->rval_op_ddref_end()),
      make_range(Inst->rval_fake_ddref_begin(), Inst->rval_fake_ddref_end()));

  for (RegDDRef *RVal : RefsRange) {
    if (!PU.VisitedRefs.count(RVal) && !RVal->isConstant()) {
      RefsStack.push_back(RVal);
    }
  }

  return true;
}

// The HLIf may be unswitched partially if one of
// the branches may update value of the condition.
//
// DO i1 = 0, 5
//   %0 = a[0]
//   if (%0 == 1) {
//     {code}
//   } else {
//     a[i] = ...
//   }
// END DO
//
// %0 = a[0]
// if (%0 == 1) {
//   DO i1 = 0, 5
//     {code}
//   END DO
// } else {
//   DO i1 = 0, 5
//     %0 = a[0]
//     if (%0 == 1) {
//       {code}
//     } else {
//       a[i] = ...
//     }
//   END DO
// }
//
bool HIROptPredicate::checkForLoadPUCandidate(
    const HLIf *If, const RegDDRef *Ref, PUContext &PU,
    SmallPtrSet<HLInst *, 8> &LoadInstructions) const {
  assert(isa<HLIf>(Ref->getHLDDNode()) && "Ref parent is not an HLIf");
  assert(Ref->getHLDDNode() == If && "HLIf should be a parent of Ref");

  if (!EnablePartialUnswitch) {
    LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: skipping <" << If->getNumber()
                      << "> candidate due to optlevel\n");
    return false;
  }

  // Try to hoist non-linear condition with all its definition instructions.

  HLLoop *ParentLoop = If->getParentLoop();
  unsigned ParentLoopLevel = ParentLoop->getNestingLevel();

  if (Ref->hasIV(ParentLoopLevel))
    return false;

  DDGraph DDG = DDA.getGraph(ParentLoop);

#ifndef NDEBUG
  static HLLoop *DDGShownForLoop = nullptr;
  if (DDGShownForLoop != ParentLoop) {
    LLVM_DEBUG(DDG.dump());
    DDGShownForLoop = ParentLoop;
  }
#endif

  SmallVector<const RegDDRef *, 32> RefsStack;

  RefsStack.push_back(Ref);
  while (!RefsStack.empty()) {
    const RegDDRef *VRef = RefsStack.pop_back_val();
    PU.VisitedRefs.insert(VRef);

    // Check that the VRef value is invariant from ParentLoop's IV.
    if (VRef->hasIV(ParentLoopLevel)) {
      return false;
    }

    // Look for all incoming flow edges.
    for (const BlobDDRef *BDDRef :
         make_range(VRef->blob_begin(), VRef->blob_end())) {
      for (auto &Edge : DDG.incoming(BDDRef)) {
        if (!processPUEdge(If, Edge, PU, RefsStack, DDG, LoadInstructions)) {
          return false;
        }
      }
    }

    for (auto &Edge : DDG.incoming(VRef)) {
      if (!processPUEdge(If, Edge, PU, RefsStack, DDG, LoadInstructions)) {
        return false;
      }
    }
  }

  return true;
}

class UnsafeCallFinder : public HLNodeVisitorBase {
  bool HasUnsafeCall = false;
  HLNode *SkipNode;

public:
  UnsafeCallFinder(HLNode *SkipNode = nullptr) : SkipNode(SkipNode) {}

  void visit(const HLNode *) {}
  void postVisit(const HLNode *) {}

  void visit(const HLInst *Inst) {
    if (auto *CInst = Inst->getCallInst()) {
      if (!CInst->onlyReadsMemory() && HLInst::hasUnknownAliasing(CInst)) {
        HasUnsafeCall = true;
      }
    }
  }

  bool skipRecursion(const HLNode *Node) const { return SkipNode == Node; }
  bool isDone() const { return HasUnsafeCall; }

  bool hasUnsafeCall() const { return HasUnsafeCall; }
};

// Helper class that traverses the outer loop of an HLIf, and all it's child
// nodes to identify if there is any side effect in the loop but outside of
// of the condition. This will help to identify is applying the transformation
// will increase code size or not.
class SideEffectChecker : public HLNodeVisitorBase {
  const HLLoop *HoistToLoop;
  const HLIf *MainIf;
  const HLNode *SkipNode;
  bool SideEffectFound;

public:
  SideEffectChecker(const HLIf *If, const HLLoop *HoistToLoop)
      : HoistToLoop(HoistToLoop), MainIf(If), SkipNode(If),
        SideEffectFound(false) {

    if (MainIf && MainIf->hasElseChildren()) {
      SideEffectChecker SideEffectsThen(nullptr, HoistToLoop);
      HLNodeUtils::visitRange(SideEffectsThen, MainIf->then_begin(),
                              MainIf->then_end());

      SideEffectChecker SideEffectsElse(nullptr, HoistToLoop);
      HLNodeUtils::visitRange(SideEffectsElse, MainIf->else_begin(),
                              MainIf->else_end());

      // If we didn't found side effect in both sides, then we assume that
      // hoisting the condition will increase code size since there should be
      // an instruction that causes side effects later.
      if (SideEffectsThen.conditionMayHaveSideEffects() ==
          SideEffectsElse.conditionMayHaveSideEffects())
        SideEffectFound = true;
    }
  }

  void visit(const HLNode *) {}
  void postVisit(const HLNode *) {}

  void visit(const HLInst *Inst) {
    if (Inst->isSideEffectsCallInst()) {
      SideEffectFound = true;
      return;
    }

    auto *LVal = Inst->getLvalDDRef();
    if (LVal) {
      if (LVal->isMemRef()) {
        SideEffectFound = true;
        return;
      }

      if (HoistToLoop->isLiveOut(LVal->getSymbase())) {
        SideEffectFound = true;
        return;
      }
    }
  }

  bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }

  bool isDone() const { return conditionMayHaveSideEffects(); }

  bool conditionMayHaveSideEffects() const { return SideEffectFound; }
};

// Helper function that determines if hoisting the input If condition will
// increase code size or not. This is determined if the side effect checker
// identifies one of the following:
//
//   * Loop nest from parent loop to hoist level has a temp live out that is
//     not inside the condition
//   * Loop nest from parent loop to hoist level has an instruction that is not
//     inside the input If and it is set as side effect outside
//   * Loop nest from parent loop to hoist level has a store outside the
//     condition
//   * If the condition has Then and Else branches, then both branches will be
//     analyzed separately with the same rules as before (temp is loop live
//     out, instruction is set as side effect or store). If the result is
//     the same for both branches, then the condition will increase size.
//
// This analysis is used to determine if we should or shouldn't count the
// condition to increase the counters used for the thresholds since the code
// size will or will not increase.
static bool ifHoistingIncreasesCodeSize(const HLIf *If, unsigned HoistLevel) {
  assert(If && "trying to analyze side effects without If");
  assert(If->getNodeLevel() > HoistLevel &&
         "Node level is lower or equal that hoist level");

  HLLoop *HoistToLoop = If->getParentLoopAtLevel(HoistLevel + 1);
  SideEffectChecker SideEffectsResult(If, HoistToLoop);
  HLNodeUtils::visitRange(SideEffectsResult, HoistToLoop->child_begin(),
                          HoistToLoop->child_end());

  return SideEffectsResult.conditionMayHaveSideEffects();
}

void HIROptPredicate::CandidateLookup::visit(HLIf *If) {
  // Skip bottom test in unknown loops.
  if (!CurrLoop || CurrLoop->getBottomTest() == If) {
    return;
  }

  visitIfOrSwitch(If);
}

void HIROptPredicate::CandidateLookup::visit(HLSwitch *Switch) {
  visitIfOrSwitch(Switch);
}

// Return true if the input If, which isn't a candidate for unswitching,
// has an inner If that can be unswitched.
bool HIROptPredicate::CandidateLookup::canUnswitchInnerIf(HLIf *If,
                                                          HLLoop *Loop) {

  // Only apply it if we are at the innermost loop and the loop is a DO loop.
  //
  // NOTE: For now, this type of unswitching is allowed if the candidate can
  // be moved out of the loopnest. Therefore, the min-level should be 0. This
  // is conservative because it may affect other transformation like loop
  // collapsing and/or loop interchange.
  if (!Loop->isInnermost() || !Loop->isDo() || Constraints.MinLevel != 0)
    return false;

  // Currently, we support for single nested If. The reason is that we don't
  // want to hoist a condition that is deeply nested. It would increase the
  // chance for taking the branch when previously wasn't taken.
  if (If->getParent() != Loop)
    return false;

  return true;
}

// Return true if there is at least one constraints for the input Node that
// prevents the transformation.
bool HIROptPredicate::CandidateLookup::isRestrictedByConstraints(
    HLNode *Node, unsigned Level, PUContext &PUC,
    CandidateConstraints &Constraints) {

  // Check if partial unswitching is set and allowed
  if (PUC.isSet() && !Constraints.PUCAllowed)
    return true;

  // Condition needs to be fully hoisted
  if (Constraints.RequiresLoopnestUnswitch && Level != 0)
    return true;

  return false;
}

// Enum to handle the different cases of enclosing a loop with SIMD directives
enum SIMDType {
  None,           // Candidate is not inside a SIMD loop
  PreAndPostLoop, // SIMD directives are in pre-header and post exit of
                  //   the loop.
  Region,         // SIMD directives covers the whole region
  NotSupported    // SIMD directives aren't supported
};

// Return the type of SIMD directives that encapsulates the current loop.
// This result will be used to used to identify which type of analysis will
// be done.
static SIMDType getSupportedSIMDType(const HLLoop *Loop) {
  assert(Loop && "Trying to collect SIMD information from a null Loop");

  auto *SIMDBegin = Loop->getSIMDEntryIntrinsic();
  if (!SIMDBegin)
    return SIMDType::None;

  // Check if SIMD support is enabled
  if (DisableUnswitchSIMD)
    return SIMDType::NotSupported;

  // If the loop is not the innermost, nor the outermost, then we mark it as
  // not supported.
  //
  // NOTE: This can be improved when we add support for SIMD blocks (JR-47148).
  if (!Loop->isInnermost() && !isa<HLRegion>(Loop->getParent()))
    return SIMDType::NotSupported;

  const HLInst *SIMDEnd = Loop->getSIMDExitIntrinsic();
  // There is a chance that the SIMD directives are enclosing a sibling loop.
  // If that is the case, the SIMD end won't be found.
  //
  // NOTE: This problem is documented in CMPLRLLVM-49174. if this issue happens
  // then SIMDBegin should be null for the current loop. We return NotSupported
  // to keep the NFC behavior before PR #14212.
  if (!SIMDEnd)
    return SIMDType::NotSupported;

  assert(SIMDEnd->isSIMDEndDirective() && "SIMD end directive expected");

  if (Loop->getNumPreheader() == 1 && Loop->getNumPostexit() == 1) {
    // Check if the SIMD directives encapsulates the loop in the preheader and
    // postexit
    auto *PreHeaderInst = Loop->getFirstPreheaderNode();
    auto *PostExitInst = Loop->getFirstPostexitNode();
    if (PreHeaderInst == SIMDBegin && PostExitInst == SIMDEnd)
      return SIMDType::PreAndPostLoop;
  }

  if (isa<HLRegion>(SIMDBegin->getParent()) &&
      isa<HLRegion>(SIMDEnd->getParent()))
    return SIMDType::Region;

  // TODO: Add support to a SIMD block, for example:
  //
  // HLRegion begin
  //   HLNode
  //   SIMD begin
  //     Loop begin
  //       HLIf
  //     Loop end
  //   SIMD end
  //   HLNode
  // HLRegion end
  //
  // Assume that the condition in the If is invariant within the SIMD block,
  // but not invariant in the region (e.g. an instruction outside the SIMD
  // block modifies a reference in the condition), then we can still apply
  // the transformation. We just need to add the proof that all the memrefs
  // in the condition needs to be invariant between the SIMD clauses
  // (CMPLRLLVM-47148).

  return SIMDType::NotSupported;
}

// Return true if at least one of the refs is not invariant to the region.
static bool conditionIsNotInvariantToRegion(HLNode *Node) {
  // This is only supported for If conditions at the moment
  //
  // TODO: We can expand this check for Select instructions by making sure
  // all the refs are invariant to the region.
  HLIf *If = dyn_cast<HLIf>(Node);
  if (!If)
    return true;

  for (auto *PredOP : make_range(If->op_ddref_begin(), If->op_ddref_end())) {
    if (!PredOP->isStructurallyRegionInvariant())
      return true;
  }

  return false;
}

// This function will check if the condition (Node) can be hoisted out of the
// the target loop. The target loop will be the loop at input level + 1. The
// reason is that level represents where the condition will be moved to
// (0 means region, 1 is loop level 1, etc.), and target loop (level + 1) is
// the loop that will be enclosed inside the condition after the
// transformation.
bool HIROptPredicate::CandidateLookup::isTargetLoopValid(HLNode *Node,
                                                         unsigned Level) {

  unsigned CurLevel = Node->getParentLoop()->getNestingLevel();
  auto *TargetLoop = Node->getParentLoopAtLevel(Level + 1);
  auto SIMDTy = getSupportedSIMDType(TargetLoop);
  if (SIMDTy == SIMDType::NotSupported) {
    // Check if the SIMD directives are supported
    LLVM_DEBUG(
        dbgs()
        << "Outerloop is a simd loop, hoisting will inhibit simd pragma.\n");
    return false;
  } else if (SIMDTy == SIMDType::Region) {
    // Check that all the refs inside the condition are invariant to the
    // region. The reason is that we want to move the SIMD directives, and
    // all the instructions between the directives and the target loop inside
    // the condition too.
    if (conditionIsNotInvariantToRegion(Node)) {
      LLVM_DEBUG(dbgs() << "Predicate is not invariant to the region.\n");
      return false;
    }
  }

  if ((Level != CurLevel - 1) &&
      HLS.getTotalStatistics(TargetLoop).hasConvergentCalls()) {
    LLVM_DEBUG(
        dbgs()
        << "Hoisting disabled as target loop contains convergent calls.\n");
    return false;
  }

  return true;
}

template <typename NodeTy>
void HIROptPredicate::CandidateLookup::visitIfOrSwitch(NodeTy *Node) {
  if (!CurrLoop) {
    return;
  }

  SkipNode = Node;

  // Partial unswitch context
  PUContext PUC;

  bool IsCandidate =
      Constraints.CanTransformParent && !isUnswitchDisabled(Node);

  // Skip candidates that are already at the outer most possible level.
  unsigned Level;

  LLVM_DEBUG_DETAIL(dbgs() << "\n"; dbgs() << "CanTransformParent: ";
                    StringRef Answer =
                        Constraints.CanTransformParent ? "yes" : "no";
                    dbgs() << Answer << "\n");
  LLVM_DEBUG_DETAIL(dbgs() << "IsCandidate: ";
                    StringRef Answer = IsCandidate ? "yes" : "no";
                    dbgs() << Answer << "\n");
  LLVM_DEBUG_DETAIL(dbgs() << "MinLevel: " << Constraints.MinLevel << "\n");

  unsigned CurLevel = CurrLoop->getNestingLevel();

  if (IsCandidate) {
    // Determine target level to unswitch.
    if (auto *If = dyn_cast<HLIf>(Node))
      Level = std::max(Pass.getPossibleDefLevel(If, PUC), Constraints.MinLevel);
    else if (auto *Switch = dyn_cast<HLSwitch>(Node))
      Level = std::max(Pass.getPossibleDefLevel(Switch), Constraints.MinLevel);
    else
      llvm_unreachable("Trying to unswitch a Node that is not If or Switch");

    // The hoisting level needs to be less than the the current node level.
    IsCandidate = Level < CurLevel;
  } else {
    Level = CurLevel;
  }

  // Check if there is any restriction that we shouldn't do the transformation
  if (IsCandidate && isRestrictedByConstraints(Node, Level, PUC, Constraints)) {
    IsCandidate = false;
    Level = CurLevel;
  }

  if (IsCandidate && !isTargetLoopValid(Node, Level)) {
    IsCandidate = false;
    Level = CurLevel;
  }

  if (IsCandidate && Pass.EarlyPredicateOpt && Level != 0) {
    // Suppress single level predicate optimization for complete unroll
    // candidates as it may complicate the analysis for unrolling and subsequent
    // passes.
    uint64_t TC;
    bool IsCompleteUnrollCandidate =
        (CurrLoop->isInnermost() && (Level == CurLevel - 1) &&
         CurrLoop->isConstTripLoop(&TC) && TC < 4);

    // Check if unswitching breaks the existing loopnest perfectness.
    IsCandidate = !IsCompleteUnrollCandidate &&
                  (Node->getParentLoopAtLevel(Level)->getNumChildren() > 1);
  }

  if (IsCandidate && PUC.isLoadPUC()) {
    // HLIf should be unconditionally executed to unswitch.
    IsCandidate = HLNodeUtils::postDominates(Node, CurrLoop->getFirstChild());
  }

  // Check for unsafe calls in branches that can modify the condition.
  // TODO: Do we need HIRStatistics instead of LoopStatistics?
  if (IsCandidate && PUC.isLoadPUC()) {
    // Implemented for HLIfs only
    HLIf *If = cast<HLIf>(Node);
    UnsafeCallFinder LoopUnsafe(Node), ThenUnsafe, ElseUnsafe;
    HLNodeUtils::visitRange(LoopUnsafe, CurrLoop->child_begin(),
                            CurrLoop->child_end());
    HLNodeUtils::visitRange(ThenUnsafe, If->then_begin(), If->then_end());
    HLNodeUtils::visitRange(ElseUnsafe, If->else_begin(), If->else_end());
    PUC.IsLoadUpdatedInThenBranch = PUC.IsLoadUpdatedInThenBranch ||
                                    ThenUnsafe.hasUnsafeCall() ||
                                    LoopUnsafe.hasUnsafeCall();
    PUC.IsLoadUpdatedInElseBranch = PUC.IsLoadUpdatedInElseBranch ||
                                    ElseUnsafe.hasUnsafeCall() ||
                                    LoopUnsafe.hasUnsafeCall();
    IsCandidate =
        !PUC.IsLoadUpdatedInThenBranch || !PUC.IsLoadUpdatedInElseBranch;
  }

  // Disable the optimization for the current node if it represents a switch
  // statement, and the number of cases is larger than MaxCasesThreshold.
  if (IsCandidate && isa<HLSwitch>(Node)) {
    auto *Switch = cast<HLSwitch>(Node);
    IsCandidate = Switch->getNumCases() <= MaxCasesThreshold;
    LLVM_DEBUG({
      if (!IsCandidate) {
        dbgs() << "Disabling opportunity for ";
        Node->dumpHeader();
        dbgs() << " because the number of cases is larger than max allowed("
               << MaxCasesThreshold << ")\n";
      }
    });
  }

  LLVM_DEBUG({
    dbgs() << "Opportunity: ";
    Node->dumpHeader();
    StringRef PUCString = "";
    if (PUC.isLoadPUC())
      PUCString = " (PUC: Load)";
    else if (PUC.isInvariantPredPUC())
      PUCString = " (PUC: InvariantPred)";

    dbgs() << " --> Level " << Level
           << ", Candidate: " << (IsCandidate ? "Yes" : "No") << PUCString
           << "\n";
  });

  // Tell inner candidates that parent candidate will not be unswitched.
  // Do not unswitch inner candidates in case of partial unswitching.
  bool WillUnswitchParent = IsCandidate && !PUC.isSet();

  bool HoistingIncreaseCodeSize = true;
  if (WillUnswitchParent && isa<HLIf>(Node)) {
    auto *If = cast<HLIf>(Node);
    auto It = Pass.OuterLoopIfs.find(If);
    if (It != Pass.OuterLoopIfs.end())
      HoistingIncreaseCodeSize = It->second;
    else if (!ifHoistingIncreasesCodeSize(If, Level))
      HoistingIncreaseCodeSize = false;

    LLVM_DEBUG({
      StringRef ResultString = HoistingIncreaseCodeSize ? " " : " NOT ";
      dbgs() << "  - Code size will" << ResultString << "increase, thresholds"
             << ResultString << "needed\n";
    });
  }

  LLVM_DEBUG_DETAIL(dbgs() << "WillUnswitchParent: ";
                    StringRef Answer = WillUnswitchParent ? "yes" : "no";
                    dbgs() << Answer << "\n");
  LLVM_DEBUG_DETAIL(dbgs() << "Level: " << Level << "\n");
  LLVM_DEBUG_DETAIL(dbgs() << "CurrLoop: " << CurrLoop->getNumber() << "\n");
  LLVM_DEBUG_DETAIL(dbgs() << "Node: " << Node->getNumber() << "\n");
  LLVM_DEBUG_DETAIL(dbgs() << "CostOfRegion: " << CostOfRegion << "\n");

  // We use RequiresLoopnestUnswitch to identify if an If condition can be
  // unswitched while its parent If condition is not a candidate for
  // opt-predicate. If that is the case, the aren't going to allow any more
  // unswitching for inner conditions.
  if (!Constraints.RequiresLoopnestUnswitch) {
    // Collect candidates within HLIf branches.
    if (auto *If = dyn_cast<HLIf>(Node)) {
      CandidateConstraints ThenConstraints(
          Level, WillUnswitchParent, Constraints.PUCAllowed,
          Constraints.RequiresLoopnestUnswitch);

      // If the current If couldn't be hoisted, then check if there is an
      // opportunity to hoist an inner If.
      if (!IsCandidate && Constraints.CanTransformParent) {
        if (canUnswitchInnerIf(If, CurrLoop)) {
          ThenConstraints.MinLevel = Constraints.MinLevel;
          ThenConstraints.CanTransformParent = Constraints.CanTransformParent;
          ThenConstraints.PUCAllowed = false;
          ThenConstraints.RequiresLoopnestUnswitch = true;
        }
      }

      // If the current node is candidate for invariant predicate PUC, then
      // we will check for more unswitching in the Else branch in the current
      // loop.
      CandidateLookup ThenLookup(Pass, HLS, ThenConstraints, CurrLoop,
                                 CostOfRegion);
      HLNodeUtils::visitRange(ThenLookup, If->then_begin(), If->then_end());

      if (If->hasElseChildren()) {
        CandidateConstraints ElseConstraints(ThenConstraints);

        // NOTE: We can still apply a similar logic for LoadPUC, but in this
        // case we need to know which branch will be converted.
        if (IsCandidate && PUC.isInvariantPredPUC())
          ElseConstraints.CanTransformParent = true;

        CandidateLookup ElseLookup(Pass, HLS, ElseConstraints, CurrLoop,
                                   CostOfRegion);
        HLNodeUtils::visitRange(ElseLookup, If->else_begin(), If->else_end());
      }

    } else if (auto *Switch = dyn_cast<HLSwitch>(Node)) {
      CandidateLookup SwitchLookup(Pass, HLS, CurrLoop, WillUnswitchParent,
                                   Level, CostOfRegion);
      for (int I = 1, E = Switch->getNumCases(); I <= E; ++I) {
        HLNodeUtils::visitRange(SwitchLookup, Switch->case_child_begin(I),
                                Switch->case_child_end(I));
      }
      HLNodeUtils::visitRange(SwitchLookup, Switch->default_case_child_begin(),
                              Switch->default_case_child_end());
    } else {
      llvm_unreachable(
          "Trying to analyze the children of a non If or Switch node");
    }
  }

  if (!IsCandidate) {
    return;
  }

  Pass.Candidates.emplace_back(Node, Level, PUC, HoistingIncreaseCodeSize);
}

// Function implements "less" predicate for a pair of HLNodes.
// The order is expected to be [HLIfs..., HLSwitches...]. No other node kinds
// are expected.
static bool conditionalHLNodeLess(const HLNode *A, const HLNode *B) {
  // The node may be one of two kinds here: HLIf or HLSwitch.
  auto GetKind = [](const HLNode *Node) {
    if (isa<HLIf>(Node))
      return 0;

    assert(isa<HLSwitch>(Node) &&
           "Unexpected node kind: should be either HLIf or HLSwitch.");
    return 1;
  };

  unsigned KindA = GetKind(A);
  unsigned KindB = GetKind(B);
  if (KindA != KindB) {
    return KindA < KindB;
  }

  // Equal kind of nodes

  if (KindA == 0 &&
      HLNodeUtils::areEqualConditions(cast<HLIf>(A), cast<HLIf>(B))) {
    return false;
  }

  if (KindA == 1 &&
      HLNodeUtils::areEqualConditions(cast<HLSwitch>(A), cast<HLSwitch>(B))) {
    return false;
  }

  return A->getNumber() < B->getNumber();
}

static unsigned countMaxEqualConditions(ArrayRef<const HLNode *> In) {
  unsigned Count = 0;

  for (auto I = In.begin(), E = In.end(); I != E;) {
    auto FoundI = std::adjacent_find(I, E, conditionalHLNodeLess);
    if (FoundI != E) {
      ++FoundI;
    }

    unsigned NumPreds = std::distance(I, FoundI);
    Count = std::max(Count, NumPreds);

    I = FoundI;
  }

  return Count;
}

// Calculate the cost of a region
void HIROptPredicate::CandidateLookup::visit(HLRegion *Reg) {
  // NOTE: The cost of a region is the number of nested loops. This is
  // conservative. We can relax this in the future by computing the size of the
  // region versus the size of the actual function.
  RegionSizeCalculator RegionSize(Pass);
  HLNodeUtils::visit(RegionSize, Reg);
  CostOfRegion = RegionSize.getRegionSize();
}

// Return true if the input loop is not the innermost, and we can apply
// the transformation at the current loop level.
bool HIROptPredicate::CandidateLookup::isProfitableOuterLoop(
    ArrayRef<const HLNode *> ChildNodes) {

  // TODO: This needs to be checked. If the inner loop is not candidate for
  // hoist, it shouldn't invalidate the outer loop.
  if (ChildNodes.size() != 1)
    return false;

  auto *If = dyn_cast<const HLIf>(ChildNodes[0]);
  if (!If)
    return false;

  PUContext PUC;
  // Check if the condition can be hoisted to the outermost loop and if
  // partial unswitch won't be performed. Partial unswitch is disabled
  // at the moment since we don't want to create a branch with perfect loop
  // nest and the other branch without prefect loop nest. This is
  // conservative and can relaxed in the future through profiling and/or
  // branch analysis.
  //
  // TODO: If HoistingIncreaseCodeSize is false, code size won't increase,
  // then we can relax this by removing the restriction of HoistLevel == 0.
  unsigned HoistLevel = Pass.getPossibleDefLevel(If, PUC);
  if (!(HoistLevel == 0 && !PUC.isSet()))
    return false;

  // TODO: This needs to move when we are creating a candidate.
  bool HoistingIncreaseCodeSize = ifHoistingIncreasesCodeSize(If, HoistLevel);

  // Don't allow the unswitch if the cost of the region is larger than
  // the threshold. There is a chance that the size of the function grows
  // big enough to produce slowdowns.
  if (HoistingIncreaseCodeSize && CostOfRegion > RegionCostThreshold)
    return false;

  Pass.OuterLoopIfs.insert({If, HoistingIncreaseCodeSize});
  return true;
}

void HIROptPredicate::CandidateLookup::visit(HLLoop *Loop) {

  SkipNode = Loop;

  // NOTE: Probably we may need only to initialize PUCAllowed only for
  // innermost loops.
  CandidateConstraints NewLoopConstraints(
      Constraints.MinLevel, true /*CanTransformParent*/, true /*PUCAllowed*/,
      false /*RequiresLoopnestUnswitch*/);

  // Handle innermost loops and outer loops, but only if unswitching can make
  // the loopnest perfectly nested. In this case the loop will have only one
  // child.
  if (!DisableCostModel && !Loop->isInnermost() &&
      Loop->getNumChildren() != 1) {

    SmallVector<const HLNode *, 8> ChildNodes;
    auto ConditionalNodes =
        make_filter_range(make_range(Loop->child_begin(), Loop->child_end()),
                          [](const HLNode &Node) {
                            return isa<HLIf>(Node) || isa<HLSwitch>(Node);
                          });

    std::transform(ConditionalNodes.begin(), ConditionalNodes.end(),
                   std::back_inserter(ChildNodes),
                   [](const HLNode &Node) { return &Node; });

    std::sort(ChildNodes.begin(), ChildNodes.end(), conditionalHLNodeLess);

    if (countMaxEqualConditions(ChildNodes) < 3 &&
        !isProfitableOuterLoop(ChildNodes))
      NewLoopConstraints.CanTransformParent = false;
  }

  // Check for SIMD support if the current loop can be transformed
  if (NewLoopConstraints.CanTransformParent) {
    auto SIMDTy = getSupportedSIMDType(Loop);
    NewLoopConstraints.CanTransformParent = SIMDTy != SIMDType::NotSupported;
  }

  if (NewLoopConstraints.CanTransformParent &&
      HLS.getTotalStatistics(Loop).hasConvergentCalls()) {
    NewLoopConstraints.CanTransformParent = false;
  }

  CandidateLookup Lookup(Pass, HLS, NewLoopConstraints, Loop, CostOfRegion);
  Loop->getHLNodeUtils().visitRange(Lookup, Loop->child_begin(),
                                    Loop->child_end());
}

// If the instruction is a Select instruction, is in the innermost loop and the
// condition is loop invariant, then add it as a candidate. The Select
// instruction will be converted into an If/Else which will be unswitched.
void HIROptPredicate::CandidateLookup::visit(HLInst *Inst) {
  if (!isa<SelectInst>(Inst->getLLVMInstruction()) || DisableUnswitchSelect ||
      Pass.EarlyPredicateOpt || !Constraints.CanTransformParent)
    return;

  // Current loop should be innermost and a Do loop
  if (!FirstSelectCandidate) {
    if (!CurrLoop || !CurrLoop->isInnermost() || !CurrLoop->isDo())
      return;

    // Check that the immediate parent must be a Loop.
    auto *CurrParent = Inst->getParent();
    if (!isa<HLLoop>(CurrParent))
      return;

    // TODO: We don't support loops with Switch statements at the moment due to
    // compile time issues. For example,
    //
    //    BEGIN REGION { }
    //          + DO i1 = 0, 99, 1   <DO_LOOP>
    //          |   switch(%x)
    //          |   {
    //          |   case 0:
    //          |      (%p)[i1] = i1;
    //          |      break;
    //          |   case 2:
    //          |      (%q)[i1] = i1;
    //          |      break;
    //          |   default:
    //          |      (%q)[i1 + 1] = i1;
    //          |      break;
    //          |   }
    //          |   %4 = (%n > 0) ? 0 : 1;
    //          |   (%p)[i1] = i1 + %4;
    //          + END LOOP
    //    END REGION
    //
    // The example above contains a Select instruction (%4) that can be optimized
    // and the result HIR will be the following:
    //
    //    BEGIN REGION { modified }
    //        switch(%x)
    //        {
    //        case 0:
    //           if (%n > 0)
    //           {
    //              + DO i1 = 0, 99, 1   <DO_LOOP>
    //              |   (%p)[i1] = i1;
    //              |   %6 = 0;
    //              |   (%p)[i1] = i1 + %6;
    //              + END LOOP
    //           }
    //           else
    //           {
    //              + DO i1 = 0, 99, 1   <DO_LOOP>
    //              |   (%p)[i1] = i1;
    //              |   %6 = 1;
    //              |   (%p)[i1] = i1 + %6;
    //              + END LOOP
    //           }
    //           break;
    //        case 2:
    //           if (%n > 0)
    //           {
    //              + DO i1 = 0, 99, 1   <DO_LOOP>
    //              |   (%q)[i1] = i1;
    //              |   %6 = 0;
    //              |   (%p)[i1] = i1 + %6;
    //              + END LOOP
    //           }
    //           else
    //           {
    //              + DO i1 = 0, 99, 1   <DO_LOOP>
    //              |   (%q)[i1] = i1;
    //              |   %6 = 1;
    //              |   (%p)[i1] = i1 + %6;
    //              + END LOOP
    //           }
    //           break;
    //        default:
    //           if (%n > 0)
    //           {
    //              + DO i1 = 0, 99, 1   <DO_LOOP>
    //              |   (%q)[i1 + 1] = i1;
    //              |   %6 = 0;
    //              |   (%p)[i1] = i1 + %6;
    //              + END LOOP
    //           }
    //           else
    //           {
    //              + DO i1 = 0, 99, 1   <DO_LOOP>
    //              |   (%q)[i1 + 1] = i1;
    //              |   %6 = 1;
    //              |   (%p)[i1] = i1 + %6;
    //              + END LOOP
    //           }
    //           break;
    //        }
    //  END REGION
    //
    // The main problem is that if the cases contain more conditionals, the
    // region is very big, and/or there is a huge number of cases, then it
    // produces compile time issues because the opt-predicate will try to
    // find new opportunities every time the transformation is applied for each
    // case. For now we are going to disable optimizing the Select instructions
    // when Switch instructions are in the same loopnest (CMPLRLLVM-39714).

    // Check if the current loop contains any switch
    auto LoopStats = HLS.getTotalStatistics(CurrLoop);
    if (LoopStats.hasSwitches())
      return;

    // If the loop is inside a Switch instruction then it can't be a candidate.
    // There is a chance that the compile time increses due to the iterative
    // checks introduced by unswitching a Switch instruction (CMPLRLLVM-39714).
    HLNode *CurrNode = CurrLoop;
    while (CurrNode) {
      if (isa<HLSwitch>(CurrNode))
        return;
      CurrNode = CurrNode->getParent();
    }

    // If the loop has any extra IF then don't proceed with the optimization.
    // There is a chance that the function grows big enough and it may affect
    // code alignment.
    if (LoopStats.hasIfs())
      return;

    // The transformation will be disabled if there are calls to unsafe
    // functions. The reason is that we would perform unswitching to enable
    // legal vectorization, but vectorization can't be applied if there are
    // calls to unsafe functions.
    if (LoopStats.hasCallsWithUnsafeSideEffects() ||
        LoopStats.hasCallsWithUnknownAliasing())
      return;

    FirstSelectCandidate = Inst;
  } else if (!HLNodeUtils::areEqualConditions(Inst, FirstSelectCandidate)) {
    return;
  }

  // Select instructions have 5 DDRef:
  //   0 -> left hand side
  //   1 -> compare operand 1
  //   2 -> compare operand 2
  //   3 -> result if compare is true
  //   4 -> result if compare is false

  auto *LHSSel = Inst->getOperandDDRef(0);
  if (!LHSSel->isSelfBlob())
    return;

  // Make sure that the operands in the condition are loop invariant
  unsigned MaxLevel = 0;
  unsigned LoopLevel = CurrLoop->getNestingLevel();
  for (unsigned I = 1; I < 3; I++) {
    auto *CmpOp = Inst->getOperandDDRef(I);

    // The operand's type can't be vector type. It isn't supported by HLIf.
    if (CmpOp->getDestType()->isVectorTy())
      return;

    unsigned DefLevel = Pass.getPossibleDefLevel(Inst, CmpOp);
    if (DefLevel >= LoopLevel)
      return;

    MaxLevel = std::max(DefLevel, MaxLevel);
  }

  if (!isTargetLoopValid(Inst, MaxLevel))
    return;

  // Disable if the LHS is used in the true or false.
  // TODO: This is conservative. There are cases where it can produce a
  // temp self assignment, which aren't expected in the HIR. Also, the
  // Select instruction might be a good candidate for last value computation.
  // For example:
  //
  // * Original loop:
  //
  //   + DO i1 = 0, zext.i32.i64(%138) + -1, 1
  //   |   %397 = (%375 <u 1.000000e-01) ? %397 : 0;
  //   |   %396 = ((%3)[0].12[i1] <u 1.600000e+01) ? %396 : 1;
  //   + END LOOP
  //
  // Expected transformation:
  //
  //   + DO i1 = 0, zext.i32.i64(%138) + -1, 1
  //   |   %396 = ((%3)[0].12[i1] <u 1.600000e+01) ? %396 : 1;
  //   + END LOOP
  //      %397 = (%375 <u 1.000000e-01) ? %397 : 0;
  //
  // Notice that the select instruction can be moved out of the loop
  // (CMPLRLLVM-39738).
  auto *TrueOP = Inst->getOperandDDRef(3);
  auto *FalseOP = Inst->getOperandDDRef(4);
  if (DDRefUtils::areEqual(LHSSel, TrueOP) ||
      DDRefUtils::areEqual(LHSSel, FalseOP))
    return;

  PUContext PUC;
  Pass.Candidates.emplace_back(Inst, MaxLevel, PUC);
}

bool HIROptPredicate::mustCodeGen() const {
  if (OuterLoopIfs.empty())
    return true;

  for (auto &Candidate : Candidates) {
    if (Candidate.isIf()) {
      auto *If = cast<HLIf>(Candidate.getNode());
      if (OuterLoopIfs.find(If) == OuterLoopIfs.end())
        return true;
    } else {
      return true;
    }
  }

  return false;
}

void HIROptPredicate::sortCandidates() {
  std::sort(Candidates.begin(), Candidates.end(),
            [](const HoistCandidate &A, const HoistCandidate &B) {
              return B.getNode()->getTopSortNum() <
                     A.getNode()->getTopSortNum();
            });
}

bool HIROptPredicate::run() {
  if (DisableLoopUnswitch) {
    return false;
  }

#if INTEL_FEATURE_CSA
  // When compiling for offload target = CSA:
  //
  // If the user does not specify the option -mllvm
  // disable-hir-opt-predicate at all on the command line, then
  // HIROptPredicate (i.e., loop unswitching) will be disabled by
  // default. HIROptPredicate will be enabled only when the user
  // specifies -mllvm disable-hir-opt-predicate=false (or = 0) on the
  // command line.

  bool IsCsaTarget =
      Triple(HIRF.getFunction().getParent()->getTargetTriple()).getArch() ==
      Triple::ArchType::csa;

  if (IsCsaTarget && (DisableLoopUnswitch.getNumOccurrences() == 0)) {
    return false;
  }
#endif // INTEL_FEATURE_CSA

  LLVM_DEBUG(dbgs() << "Opt Predicate for Function: "
                    << HIRF.getFunction().getName() << "\n");

  bool Modified = false;
  for (HLNode &Node : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    HLRegion *Region = cast<HLRegion>(&Node);

    LLVM_DEBUG(dbgs() << "Region: " << Region->getNumber() << ":\n");

    CandidateLookup Lookup(*this, HLS);
    HLNodeUtils::visit(Lookup, Region);
    sortCandidates();

    LLVM_DEBUG(dumpCandidates());

    bool MustCodeGen = mustCodeGen();
    if (processOptPredicate()) {
      if (MustCodeGen)
        Region->setGenCode();
      HLNodeUtils::removeRedundantNodes(Region, false);
      Modified = true;
    }

    clearOptReportState();
    Candidates.clear();
    ThresholdMap.clear();
    OuterLoopIfs.clear();
  }

  return Modified;
}

unsigned HIROptPredicate::getPossibleCEDefLevel(const CanonExpr *CE,
                                                bool &UDivBlob) {
  unsigned IVMaxLevel = 0;
  unsigned IVLevel = 0;
  for (auto I = CE->iv_begin(), E = CE->iv_end(); I != E; ++I) {
    // TODO: Traverse in reverse order and break as soon as a non-zero
    // constant coeff found. Also, the IV level can be accessed using
    // CE->getLevel(I). But before it will be possible getIVConstCoeff()
    // and getLevel() should support reverse iterators.
    IVLevel++;
    if (CE->getIVConstCoeff(I)) {
      IVMaxLevel = IVLevel;
    }

    if (I->Index != InvalidBlobIndex &&
        BlobUtils::mayContainUDivByZero(BU.getBlob(I->Index))) {
      UDivBlob = true;
    }
  }

  for (auto I = CE->blob_begin(), E = CE->blob_end(); I != E && !UDivBlob;
       ++I) {
    if (I->Index != InvalidBlobIndex &&
        BlobUtils::mayContainUDivByZero(BU.getBlob(I->Index))) {
      UDivBlob = true;
    }
  }

  unsigned DefLevel = CE->getDefinedAtLevel();
  assert(DefLevel != NonLinearLevel && "Non linear CE found");
  return std::max(IVMaxLevel, DefLevel);
}

// Given a Ref that is used in Node, return at which loop level it is defined.
// If NonLinearRef is passed, then store true or false if the Ref is considered
// non-linear.
unsigned HIROptPredicate::getPossibleDefLevel(const HLDDNode *Node,
                                              const RegDDRef *Ref,
                                              bool *NonLinearRef) {

  // If the input Ref is NonLinear or a memref, then return the current node
  // level
  if (Ref->isMemRef() || Ref->isNonLinear()) {
    if (NonLinearRef)
      *NonLinearRef = true;

    return Node->getNodeLevel();
  }

  unsigned Level = 0;
  bool UDivBlob = false;
  bool HasGEPInfo = Ref->hasGEPInfo();

  if (HasGEPInfo) {
    Level = getPossibleCEDefLevel(Ref->getBaseCE(), UDivBlob);
  }

  for (unsigned I = 1, NumDims = Ref->getNumDimensions(); I <= NumDims; ++I) {
    Level = std::max(
        Level, getPossibleCEDefLevel(Ref->getDimensionIndex(I), UDivBlob));

    if (HasGEPInfo) {
      Level = std::max(
          Level, getPossibleCEDefLevel(Ref->getDimensionLower(I), UDivBlob));
      Level = std::max(
          Level, getPossibleCEDefLevel(Ref->getDimensionStride(I), UDivBlob));
    }
  }

  unsigned NodeLevel = Node->getNodeLevel();
  if (NodeLevel == Level) {
    // Reference is dependent on If's nesting level. Ex.: a[i1] or i1 + %b
    return Level;
  }

  if (UDivBlob) {
    // Allow only one level unswitching in case of UDivBlob.
    Level = NodeLevel - 1;
  }

  return Level;
}

unsigned HIROptPredicate::getPossibleDefLevel(const HLIf *If, PUContext &PUC) {
  unsigned Level = 0;
  unsigned NodeLevel = If->getParentLoop()->getNestingLevel();
  unsigned PartialLevel = NodeLevel;
  unsigned OuterMostPartialPred = 0;
  bool IsLoadPUC = false;

  SmallPtrSet<HLInst *, 8> LoadInstructions;

  for (auto PI = If->pred_begin(), PE = If->pred_end(); PI != PE; ++PI) {
    const RegDDRef *LRef = If->getLHSPredicateOperandDDRef(PI);
    const RegDDRef *RRef = If->getRHSPredicateOperandDDRef(PI);

    // Collect the possible levels where the Refs were defined
    bool NonLinearLRef = false;
    unsigned LRefLevel = getPossibleDefLevel(If, LRef, &NonLinearLRef);

    bool NonLinearRRef = false;
    unsigned RRefLevel = getPossibleDefLevel(If, RRef, &NonLinearRRef);

    // If the current level is less that the partial level, it means that
    // the current predicate can be hoisted at an outer level than the
    // previous predicate found.
    unsigned CurrPredLevel = std::max(LRefLevel, RRefLevel);
    if (CurrPredLevel < PartialLevel && !LRef->containsUndef() &&
        !RRef->containsUndef()) {
      PartialLevel = CurrPredLevel;
      OuterMostPartialPred = std::distance(If->pred_begin(), PI);
    }

    if (NonLinearLRef &&
        checkForLoadPUCandidate(If, LRef, PUC, LoadInstructions)) {
      IsLoadPUC = true;
      --LRefLevel;
    }

    Level = std::max(Level, LRefLevel);

    if (NonLinearRRef &&
        checkForLoadPUCandidate(If, RRef, PUC, LoadInstructions)) {
      IsLoadPUC = true;
      --RRefLevel;
    }

    Level = std::max(Level, RRefLevel);
  }

  // If we can't hoist the whole condition, then check if there is one
  // predicate that we can partially hoist.
  //
  // TODO: There are two cases we can improve here.
  //
  //   Case 1:
  //
  //     BEGIN REGION { }
  //          + DO i1 = 0, 99, 1   <DO_LOOP>
  //          |   + DO i2 = 0, 99, 1   <DO_LOOP>
  //          |   |   if (i1 != 0 && %t > 1)
  //          |   |   {
  //          |   |      (%A)[i2] = i2;
  //          |   |   }
  //          |   + END LOOP
  //          + END LOOP
  //     END REGION
  //
  //    In this case, the condition will be hoisted to the i1 loop since we
  //    can hoist the whole condition. There is an opportunity to partial
  //    hoist both conditions and produce something as follows.
  //
  //     BEGIN REGION { }
  //          if (%t > 1)
  //          {
  //             + DO i1 = 0, 99, 1   <DO_LOOP>
  //             |   if (i1 != 0)
  //             |   {
  //             |      + DO i2 = 0, 99, 1   <DO_LOOP>
  //             |      |      (%A)[i2] = i2;
  //             |      + END LOOP
  //             |   }
  //             + END LOOP
  //          }
  //     END REGION
  //
  //    Also, this case can be expanded if we have profile data. If we know
  //    that i1 != 0 will always be false, but %t > 1 will be true, then we
  //    can hoist only i1 != 0.
  //
  //   Case 2:
  //
  //    BEGIN REGION { }
  //          + DO i1 = 0, 99, 1   <DO_LOOP>
  //          |   + DO i2 = 0, 99, 1   <DO_LOOP>
  //          |   |   if (i2 != 5 && %t > 1 && %t < 5)
  //          |   |   {
  //          |   |      (%A)[i2] = i2;
  //          |   |   }
  //          |   + END LOOP
  //          + END LOOP
  //    END REGION
  //
  //   In this case we have two candidates to hoist at the same level. We need
  //   to be able to identify that '%t > 1' and '%t < 5' can be hoisted.
  //
  //    BEGIN REGION { }
  //          if (%t > 1 && t < %5)
  //          {
  //             + DO i1 = 0, 99, 1   <DO_LOOP>
  //             |   + DO i2 = 0, 99, 1   <DO_LOOP>
  //             |   |   if (i2 != 5)
  //             |   |   {
  //             |   |      (%A)[i2] = i2;
  //             |   |   }
  //             |   + END LOOP
  //             + END LOOP
  //          }
  //    END REGION
  //
  if (!DisablePartialUnswitch) {
    assert(!PUC.isSet() &&
           "Trying to set PU type in a candidate that was set before");
    if (Level < NodeLevel) {
      // Check if the condition is candidate for load PU
      if (IsLoadPUC)
        PUC.setLoadPUC(LoadInstructions);
    } else if (PartialLevel < Level) {
      // Check if the condition is candidate for invariant PU
      PUC.setInvariantPredPUC(OuterMostPartialPred);
      Level = PartialLevel;
    }
  }

  return Level;
}

unsigned HIROptPredicate::getPossibleDefLevel(const HLSwitch *Switch) {
  return getPossibleDefLevel(Switch, Switch->getConditionDDRef());
}

void HIROptPredicate::clearOptReportState() {
  VNum = 1;
  OptReportVisitedSet.clear();
}

/// processOptPredicate - Main routine to perform opt predicate
/// transformation.
bool HIROptPredicate::processOptPredicate() {
  bool Modified = false;

  while (!Candidates.empty()) {
    HoistCandidate &Candidate = Candidates.back();

    // There shouldn't be any Candidate set as Bottom.
    assert(Candidate.isValidCandidate() &&
           "Invalid instruction selected as candidate");

    HLDDNode *PilotIfOrSwitch = Candidate.getNode();

    ConditionsAnalyzed++;

    HLLoop *ParentLoop = PilotIfOrSwitch->getParentLoop();
    assert(ParentLoop && "Candidate should have a parent loop");

    LLVM_DEBUG(dbgs() << "Unswitching loop <" << ParentLoop->getNumber()
                      << ">:\n");

    // TODO: Implement partial predicate hoisting: if (%b > 50 && i < 50),
    // %b > 50 may be hoisted.
    HLLoop *TargetLoop =
        PilotIfOrSwitch->getParentLoopAtLevel(Candidate.Level + 1);
    assert(TargetLoop && "Target loop should always exist for the candidate");

    if (ThresholdMap[TargetLoop] >= NumPredicateThreshold) {
      LLVM_DEBUG(dbgs() << "Skipped due to NumPredicateThreshold\n");
      Candidates.pop_back();

      if (Candidate.isIf()) {
        Candidate.getIf()->setUnswitchDisabled();
      }
      continue;
    }

    HLRegion *ParentRegion = TargetLoop->getParentRegion();
    auto LoopStats = HLS.getTotalStatistics(TargetLoop);
    bool SkipLoop = false;
    unsigned NumOfIfs = LoopStats.getNumIfs();
    if (!RegionThresholdSet.insert(ParentRegion) && Candidate.isIf() &&
        NumOfIfs > MaxIfsInLoopThreshold) {
      // If the region was transformed once, the candidate is an If and the
      // number of If conditions in the loopnest is larger than
      // MaxIfsInLoopThreshold, then check how many times the candidate
      // condition repeats in the loopnest. If the number of times repeated
      // is less than half of the conditionals, and more than 40% of the If
      // statements have an Else child node in the loop nest, then disable
      // the current candidate. The reason is that it will introduce a large
      // number of new candidates which increases the compile time, and many
      // of these candidates won't necessarily be hoisted to the outermost
      // loop. Else, if there is a large count of the candidate condition then
      // it means that most likely the loop was unrolled.
      RegionSizeCalculator PredCounter(*this, Candidate.getIf());
      TargetLoop->getHLNodeUtils().visitRange(PredCounter,
                                              TargetLoop->child_begin(),
                                              TargetLoop->child_end());

#if INTEL_FEATURE_SW_ADVANCED
      // TODO: Disabling the If condition (just leaving the computation of
      // SkipLoop) provides an extra 5% for CPU2017/538.imagick
      // (CMPLRLLVM-38999).
#endif // INTEL_FEATURE_SW_ADVANCED
      if (PredCounter.getNumOfIfWithElse() > ((double)NumOfIfs * 0.4))
        SkipLoop =
            (double)PredCounter.getNumSameIfs() < ((double)NumOfIfs / 2.0);
    }

    if (SkipLoop) {
      // If the parent loop was converted already and there is a large
      // number of conditionals, then do not apply it again.
      LLVM_DEBUG({
        dbgs() << "Skipping opportunity because the number of Ifs in loop "
               << "is larger than max allowed ("
               << LoopStats.getNumIfs()
               << " > " << MaxIfsInLoopThreshold << ")\n";
      });
      Candidates.pop_back();
      if (Candidate.isIf()) {
        Candidate.getIf()->setUnswitchDisabled();
      }
      continue;
    }

    HIRInvalidationUtils::invalidateBody(ParentLoop);
    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(TargetLoop);

    // If the loop is SIMD then do not extract the pre-header and post-exit
    // nodes of the loop. We need to duplicate the SIMD intrinsics. This will
    // only happen if the SIMD intrinsics are part of the loop pre-header and
    // loop post-exit, and these are the only instructions in both lists.
    if (getSupportedSIMDType(TargetLoop) != SIMDType::PreAndPostLoop) {
      // TODO: check if candidate is defined in preheader
      TargetLoop->extractZttPreheaderAndPostexit();
    }

    // Calculate statistics
    if (Candidate.PUC.isSet()) {
      ConditionsPUUnswitched++;
    }

    ConditionsUnswitched++;

    // TransformLoop and its clones.
    transformCandidate(TargetLoop, Candidate);

    LLVM_DEBUG(dbgs() << "While " OPT_DESC ":\n");
    LLVM_DEBUG(ParentLoop->getParentRegion()->dump());
    LLVM_DEBUG(dbgs() << "\n");

    LLVM_DEBUG(dumpCandidates());

    Modified = true;
  }

  return Modified;
}

// Conservatively add Lval symbase as live-in to the loop.
static void addLvalAsLivein(const RegDDRef *LvalRef, HLLoop *Loop) {
  assert(LvalRef->isLval());
  if (LvalRef->isTerminalRef()) {
    Loop->addLiveInTemp(LvalRef->getSymbase());
  }
}

// Traverse through the parent nodes of the input Node to check if the
// condition already exists outside. If CheckPredPosOnly is true, then the
// predicate at position PredPos will be used for compare rather than the
// whole condition. This is used for partial unswitching.
static bool hasEqualParentNode(HLNode *FromNode, HLLoop *ToLoop,
                               unsigned PredPos, bool CheckPredPosOnly) {

  HLIf *FromIf = dyn_cast<HLIf>(FromNode);
  HLSwitch *FromSwitch = dyn_cast<HLSwitch>(FromNode);

  for (HLNode *Node = FromNode->getParent(); Node != ToLoop;
       Node = Node->getParent()) {
    assert(Node && "FromNode should be a child of ToLoop");

    HLIf *ParentIf = dyn_cast<HLIf>(Node);
    HLSwitch *ParentSwitch = dyn_cast<HLSwitch>(Node);

    if (FromIf && ParentIf) {
      // If CheckPredPosOnly is true, then check if the If contains one
      // condition and it matches with the predicate that will be partially
      // unswitched.
      bool IsValidIf = !CheckPredPosOnly
                           ? HLNodeUtils::areEqualConditions(FromIf, ParentIf)
                           : (ParentIf->getNumPredicates() == 1 &&
                              HLNodeUtils::areEqualConditionsAtPos(
                                  FromIf, PredPos, ParentIf, 0));
      if (IsValidIf)
        return true;
    } else if (FromSwitch && ParentSwitch &&
               HLNodeUtils::areEqualConditions(FromSwitch, ParentSwitch)) {
      return true;
    }
  }

  return false;
}

// Return true if the input loop has special patterns for reducing the
// cost of unswitching Switch instructions. This will allow to perform
// more loop unswitching. This function will check if there is a loop
// nest, the parent loop is the innermost, the target loop is the
// outermost, and both loops are multi-exit:
//
//   + DO i1 = 0, zext.i32.i64(%size) + -1, 1    <DO_MULTI_EXIT_LOOP>
//   |   + DO i2 = 0, zext.i32.i64(%size2) + -1, 1    <DO_MULTI_EXIT_LOOP>
//   |   |   switch(%n)
//   |   |   {
//   |   |   case 50:
//   |   |      ...
//   |   |      break;
//   |   |   case 20:
//   |   |      ...
//   |   |      break;
//   |   |   case 30:
//   |   |      ...
//   |   |      break;
//   |   |   case 40:
//   |   |      ...
//   |   |      break;
//   |   |   default:
//   |   |      ...
//   |   |      break;
//   |   |   }
//   |   + END LOOP
//   + END LOOP
bool HIROptPredicate::shouldUseReducedSwitchCost(
    const HLLoop *ParentLoop, const HLLoop *TargetLoop) const {

  assert(ParentLoop && "Parent loop not provided");
  assert(TargetLoop && "Target loop not provided");

  if (UseReducedSwitchCost)
    return true;

  if (TargetLoop == ParentLoop)
    return false;

  if (!ParentLoop->isDoMultiExit() || !TargetLoop->isDoMultiExit())
    return false;

  return TargetLoop->getNestingLevel() == 1 && ParentLoop->isInnermost();
}

void HIROptPredicate::transformSwitch(
    HLLoop *TargetLoop, iterator_range<HoistCandidate *> SwitchCandidates,
    CaseNodeContainerMapTy &CaseContainers,
    NodeContainerMapTy &DefaultContainer,
    SmallPtrSetImpl<HLNode *> &TrackClonedNodes,
    SmallVectorImpl<HoistCandidate> &NewCandidates) {
  HLSwitch *OriginalSwitch = SwitchCandidates.begin()->getSwitch();
  HLLoop *ParentLoop = OriginalSwitch->getParentLoop();

  bool ReduceSwitchCost = shouldUseReducedSwitchCost(ParentLoop, TargetLoop);

  auto &HNU = OriginalSwitch->getHLNodeUtils();
  auto &DRU = HNU.getDDRefUtils();

  HLSwitch *OuterSwitch =
      HNU.createHLSwitch(OriginalSwitch->getConditionDDRef()->clone());

  bool IsRemarkAdded = false;

  // If ReduceSwitchCost is true, then it means that there is some
  // extra pattern in the loop that is worthy to do keep doing the
  // unswitching. Therefore, the cost for applying the transformation
  // will be 1.
  if (ReduceSwitchCost)
    ++ThresholdMap[TargetLoop];

  // Populate cases of the outer Switch with loop clones.
  for (auto &Case : CaseContainers) {
    auto CaseDDRef = DRU.createConstDDRef(
        OriginalSwitch->getConditionDDRef()->getDestType(), Case.first);
    OuterSwitch->addCase(CaseDDRef);

    LoopUnswitchNodeMapper CloneMapper(TrackClonedNodes, Candidates);
    HLLoop *NewLoop = TargetLoop->clone(&CloneMapper);

    // If ReduceSwitchCost is false then it means that there is a
    // chance that unswitching the Switch condition may have some extra
    // risks (e.g. compile time, larger code, etc.). In this case, the
    // cost for unswitching will be the same as the number of cases.
    ThresholdMap[NewLoop] = !ReduceSwitchCost ? ++ThresholdMap[TargetLoop]
                                              : ThresholdMap[TargetLoop];

    addPredicateOptReportOrigin(NewLoop);
    if (!IsRemarkAdded) {
      // Invariant Switch condition%s hoisted out of this loop
      addPredicateOptReportRemark(NewLoop, SwitchCandidates,
                                  OptRemarkID::InvariantSwitchConditionHoisted);
      IsRemarkAdded = true;
    }

    HLNodeUtils::insertAsFirstChild(OuterSwitch, NewLoop,
                                    OuterSwitch->getNumCases());

    // Replace each Switch clone with a list of previously extracted children
    // nodes.
    for (auto &Candidate : SwitchCandidates) {
      HLSwitch *SwitchOrig = Candidate.getSwitch();
      HLSwitch *SwitchClone = CloneMapper.getMapped<HLSwitch>(SwitchOrig);

      auto CaseContainerI = Case.second.find(SwitchOrig);

      // Select the case container or a default container if case does not exist
      // in particular switch.
      bool UseDefaultContainer = CaseContainerI == Case.second.end();
      HLContainerTy *Container = UseDefaultContainer
                                     ? &DefaultContainer[SwitchOrig]
                                     : &CaseContainerI->second;

      if (!Container->empty()) {
        // Clone nodes in default case as they also be used to populate the
        // default case.
        HLContainerTy DefaultClones;
        if (UseDefaultContainer) {
          HLNodeUtils::cloneSequence(&DefaultClones, &Container->front(),
                                     &Container->back(), &CloneMapper);
          Container = &DefaultClones;
        }

        HIRTransformUtils::remapLabelsRange(CloneMapper, &Container->front(),
                                            &Container->back());
        HLNodeUtils::insertAfter(SwitchClone, Container);
      }
      HLNodeUtils::remove(SwitchClone);
    }

    NewCandidates.append(CloneMapper.getNewCandidates().begin(),
                         CloneMapper.getNewCandidates().end());
  }

  // Detach original loop for node manipulations.
  HLNode *Marker = HNU.getOrCreateMarkerNode();
  HLNodeUtils::replace(TargetLoop, Marker);

  // Default case
  {
    HLNodeUtils::insertAsFirstDefaultChild(OuterSwitch, TargetLoop);

    for (auto &Candidate : SwitchCandidates) {
      HLSwitch *SwitchOrig = Candidate.getSwitch();

      auto &CaseContainer = DefaultContainer[SwitchOrig];

      HLNodeUtils::insertAfter(SwitchOrig, &CaseContainer);
      HLNodeUtils::remove(SwitchOrig);
    }

    addPredicateOptReportOrigin(TargetLoop);
  }

  HLNodeUtils::replace(Marker, OuterSwitch);
  updateDefLevels(OuterSwitch, OuterSwitch->child_begin(),
                  OuterSwitch->child_end());
}

// Return true if the input loop has SIMD directives in the pre-header or
// post-exit.
static bool IsPreAndPostSIMDLoop(HLLoop *Loop) {
  if (!Loop->isSIMD())
    return false;

  if (Loop->hasPreheader()) {
    auto *CurrPre = Loop->getFirstPreheaderNode();
    while (CurrPre) {
      HLInst *Inst = dyn_cast<HLInst>(CurrPre);
      if (Inst && Inst->isSIMDDirective())
        return true;
      CurrPre = CurrPre->getNextNode();
    }
  }

  if (Loop->hasPostexit()) {
    auto *CurrPost = Loop->getFirstPostexitNode();
    while (CurrPost) {
      HLInst *Inst = dyn_cast<HLInst>(CurrPost);
      if (Inst && Inst->isSIMDEndDirective())
        return true;
      CurrPost = CurrPost->getNextNode();
    }
  }

  return false;
}

void HIROptPredicate::transformIf(
    HLLoop *TargetLoop, iterator_range<HoistCandidate *> IfCandidates,
    CaseNodeContainerMapTy &CaseContainers,
    SmallPtrSet<HLNode *, 32> TrackClonedNodes,
    SmallVectorImpl<HoistCandidate> &NewCandidates) {

  // Collect the SIMD clauses that aren't pre-header and post-exit if they are
  // available before transforming the loop. SIMD directives in pre and post
  // loop are handled differently.
  const HLInst *SIMDBegin = nullptr;
  const HLInst *SIMDEnd = nullptr;
  if (!DisableUnswitchSIMD && !IsPreAndPostSIMDLoop(TargetLoop)) {
    SIMDBegin = TargetLoop->getSIMDEntryIntrinsic();
    SIMDEnd = TargetLoop->getSIMDExitIntrinsic();
  }

  // Create the else loop by cloning the main loop.
  LoopUnswitchNodeMapper CloneMapper(TrackClonedNodes, Candidates);
  HLLoop *NewElseLoop = TargetLoop->clone(&CloneMapper);

  HLNodeUtils::insertAfter(TargetLoop, NewElseLoop);

  HLIf *PivotIf = nullptr;

  HLIf *FirstIf = IfCandidates.begin()->getIf();
  HLIf *FirstIfClone = CloneMapper.getMapped(FirstIf);

  // if (...) {  << PivotIf
  //   DO i1     << TargetLoop
  //     if(...) << FirstIf (Original)
  //     if(...)
  //     if(...)
  //   END DO
  // } else {
  //   DO i1     << NewElseLoop
  //     if(...) << FirstIf (Clone)
  //     if(...)
  //     if(...)
  //   END DO
  // }

  // Check if the candidate may increase code size to decide if we need to
  // count the current If toward the threshold not.
  bool ThresholdNeeded = false;

  // If the If/Else was created from a select instruction then we use a
  // generic opt-report remark
  OptRemarkID OptReportRemark = OptRemarkID::InvariantIfConditionHoisted;
  for (auto &Candidate : IfCandidates) {
    if (Candidate.createdFromSelect()) {
      OptReportRemark = OptRemarkID::InvariantConditionHoisted;
      // Select instructions will always create Else branch
      ThresholdNeeded = true;
      break;
    }

    if (!ThresholdNeeded && Candidate.mayIncreasesCodeSize())
      ThresholdNeeded = true;
  }

  ThresholdMap[NewElseLoop] =
      ThresholdNeeded ? ++ThresholdMap[TargetLoop] : ThresholdMap[TargetLoop];

  // Invariant [If condition%s | Condition%s] hoisted out of this loop
  addPredicateOptReportRemark(TargetLoop, IfCandidates, OptReportRemark);

  // Unswitch every equivalent candidate - a pair of HLIfs. First we handle
  // original If and then its clone. removeOrHoistIf() will move HLIf before
  // the loops and make it a pivot or will remove it as needed.
  for (auto &C : IfCandidates) {
    HLIf *If = C.getIf();
    auto &ThenContainer = CaseContainers[0][If];
    auto &ElseContainer = CaseContainers[1][If];

    // Handle TargetLoop first

    if (C.PUC.IsLoadUpdatedInThenBranch) {
      // Place original HLIf block
      HLNodeUtils::insertAsFirstThenChildren(If, &ThenContainer);
      HLNodeUtils::insertAsFirstElseChildren(If, &ElseContainer);
    } else {
      // Place *then* branch unconditionally.
      if (!ThenContainer.empty()) {
        HLContainerTy *ThenContainerPtr = &ThenContainer;
        HLContainerTy CloneContainer;
        if (C.PUC.IsLoadUpdatedInElseBranch) {
          // If true then nodes will be needed for second loop.
          HLNodeUtils::cloneSequence(&CloneContainer, &ThenContainer.front(),
                                     &ThenContainer.back());
          ThenContainerPtr = &CloneContainer;
        }

        HLNodeUtils::insertAfter(If, ThenContainerPtr);
      }

      removeOrHoistIf(C, TargetLoop, FirstIf, If, PivotIf);
      if (C.PUC.isLoadPUC()) {
        for (HLInst *RedundantInst : C.PUC.getLoadInstructions()) {
          // Remove instruction if it's still attached. May be removed by
          // another candidate.
          if (RedundantInst->getParent()) {
            addLvalAsLivein(RedundantInst->getLvalDDRef(), TargetLoop);
            HLNodeUtils::remove(RedundantInst);
          }
        }
      }
    }

    // Handle second loop - NewElseLoop

    if (!ThenContainer.empty()) {
      HIRTransformUtils::remapLabelsRange(CloneMapper, &ThenContainer.front(),
                                          &ThenContainer.back());
    }

    if (ElseContainer.empty() && If->hasElseChildren()) {
      // If true than ElseContainer was used for the first loop and we will
      // need it for the second loop.
      HLNodeUtils::cloneSequence(&ElseContainer, If->getFirstElseChild(),
                                 If->getLastElseChild());
    }

    if (!ElseContainer.empty()) {
      HIRTransformUtils::remapLabelsRange(CloneMapper, &ElseContainer.front(),
                                          &ElseContainer.back());
    }

    HLIf *ClonedIf = CloneMapper.getMapped(If);
    assert(ClonedIf && "Can not get mapped If");
    if (C.PUC.IsLoadUpdatedInElseBranch) {
      // Place original HLIf block
      HLNodeUtils::insertAsFirstThenChildren(ClonedIf, &ThenContainer);
      HLNodeUtils::insertAsFirstElseChildren(ClonedIf, &ElseContainer);
    } else {
      // Place *else* branch unconditionally.
      if (!ElseContainer.empty()) {
        HLNodeUtils::insertAfter(ClonedIf, &ElseContainer);
      }

      removeOrHoistIf(C, TargetLoop, FirstIfClone, ClonedIf, PivotIf);

      if (C.PUC.isLoadPUC()) {
        for (HLInst *RedundantInst : C.PUC.getLoadInstructions()) {
          // Remove instruction if it's still attached. May be removed by
          // another candidate.
          HLInst *RedundantInstClone = CloneMapper.getMapped(RedundantInst);
          if (RedundantInstClone && RedundantInstClone->getParent()) {
            addLvalAsLivein(RedundantInstClone->getLvalDDRef(), NewElseLoop);
            HLNodeUtils::remove(RedundantInstClone);
          }
        }
      }
    }
  }

  assert(PivotIf && "should be defined");

  // Allow further unswitching of the top *If* statement.
  PivotIf->setUnswitchDisabled(false);

  // Place TargetLoop under PivotIf.
  HLNodeUtils::moveAsFirstThenChild(PivotIf, TargetLoop);

  // Place NewElseLoop under PivotIf if it is non-empty, and
  // add opt-report origin for both TargetLoop and NewElseLoop.
  // Otherwise remove the loop.
  if (NewElseLoop->hasChildren()) {
    HLNodeUtils::moveAsFirstElseChild(PivotIf, NewElseLoop);
    addPredicateOptReportOrigin(TargetLoop);
    addPredicateOptReportOrigin(NewElseLoop);
  } else {
    HLNodeUtils::remove(NewElseLoop);
  }

  // If the PivotIf is inside a SIMD block, then we need to move everything
  // inside the Then and Else branches. The target loop's SIMD directives will
  // be enclosing the PivotIf at this point.
  if (SIMDBegin && SIMDEnd) {
    // If the Else branch was created then clone the nodes before and after the
    // condition, and insert them before and after the loop in the else part.
    if (PivotIf->hasElseChildren()) {
      HLContainerTy ElseClonesBeforeLoop;
      HLContainerTy ElseClonesAfterLoop;
      HLNodeUtils::cloneSequence(&ElseClonesBeforeLoop, SIMDBegin,
                                 PivotIf->getPrevNode());
      HLNodeUtils::cloneSequence(&ElseClonesAfterLoop, PivotIf->getNextNode(),
                                 SIMDEnd);

      HLNodeUtils::insertAsFirstElseChildren(PivotIf, &ElseClonesBeforeLoop);
      HLNodeUtils::insertAsLastElseChildren(PivotIf, &ElseClonesAfterLoop);
    }

    auto *SIMDStart = const_cast<HLInst *>(SIMDBegin);
    auto *SIMDExit = const_cast<HLInst *>(SIMDEnd);

    auto SIMDItStart = SIMDStart->getIterator();
    auto PivotIfIt = PivotIf->getIterator();
    auto AfterIfIt = PivotIf->getNextNode()->getIterator();
    auto SIMDItEnd = SIMDExit->getIterator();

    // Move [SIMD begin - pivot If) inside the then branch, before the target
    // loop.
    HLNodeUtils::moveAsFirstThenChildren(PivotIf, SIMDItStart, PivotIfIt);

    // Move [after the pivot If - SIMD end) inside the then branch, after the
    // target loop.
    HLNodeUtils::moveAsLastThenChildren(PivotIf, AfterIfIt, SIMDItEnd);

    // Move the SIMD end
    HLNodeUtils::moveAsLastThenChild(PivotIf, SIMDExit);
  }

  NewCandidates.append(CloneMapper.getNewCandidates().begin(),
                       CloneMapper.getNewCandidates().end());

  updateDefLevels(PivotIf, PivotIf->child_begin(), PivotIf->child_end());
}

// Return true if CandA and CandB can be treated as equal conditions at
// loop TargetLoop.
static bool areEquivalentCandidates(const HoistCandidate &CandA,
                                    const HoistCandidate &CandB,
                                    HLLoop *TargetLoop) {

  if (CandB == CandA)
    return true;

  if (CandB.Level != CandA.Level)
    return false;

  unsigned PUCPredPos = 0;
  bool CheckPredPosOnly = false;
  bool IsEquiv =
      CandB.isIf() == CandA.isIf() &&
      // Have same condition
      ((CandB.isIf() &&
        HLNodeUtils::areEqualConditions(CandB.getIf(), CandA.getIf())) ||
       (CandB.isSwitch() && HLNodeUtils::areEqualConditions(
                                CandB.getSwitch(), CandA.getSwitch()))) &&
      // And are in the same target loop
      HLNodeUtils::contains(TargetLoop, CandB.getNode(), false);

  // Check the special cases
  if (!IsEquiv) {
    if (CandB.isSelect() && CandA.isIf()) {
      // Check if the current candidate is a Select instruction that will
      // be converted into an If. If so, then collect all the possible
      // equal conditions
      IsEquiv =
          HLNodeUtils::areEqualConditions(CandB.getSelect(), CandA.getIf()) &&
          HLNodeUtils::contains(TargetLoop, CandB.getSelect(), false);

    } else if (CandB.PUC.isInvariantPredPUC() && CandA.isIf() &&
               CandA.getIf()->getNumPredicates() == 1) {
      // Check if the candidate will be partially unswitched, or if the
      // condition can be mixed with one If that will be partially
      // unswitched
      IsEquiv = (HLNodeUtils::areEqualConditionsAtPos(
                     CandA.getIf(), 0, CandB.getIf(),
                     CandB.PUC.getInvariantPredicatePos()) &&
                 HLNodeUtils::contains(TargetLoop, CandB.getNode(), false));

      CheckPredPosOnly = IsEquiv;
      PUCPredPos = CandB.PUC.getInvariantPredicatePos();
    }
  }

  if (!IsEquiv)
    return false;

  // Can not handle candidate if there is an equal parent node.
  return !hasEqualParentNode(CandB.getNode(), TargetLoop, PUCPredPos,
                             CheckPredPosOnly);
}

// transformLoop - Perform the OptPredicate transformation for the given loop.
void HIROptPredicate::transformCandidate(HLLoop *TargetLoop,
                                         HoistCandidate &Candidate) {
  SmallPtrSet<HLNode *, 32> TrackClonedNodes;

  // Find other equivalent candidates that may be handled under the same
  // TargetLoop. They should be in the same Target loop and have same
  // conditions.
  std::function<bool(const HoistCandidate &)> IsEquivCandidate =
      [TargetLoop, &Candidate](const HoistCandidate &C) {
        return areEquivalentCandidates(Candidate, C, TargetLoop);
      };

  // Clean any candidate needed before checking if we can merge them. Ideally,
  // we would prefer to clean all the candidates and then let the equivalent
  // candidates run by itself. The issue is that we are giving up in some
  // candidates if the thresholds are reached.
  if (Candidate.isSelect()) {
    // The Candidate that is a Select instruction must have a temporary If
    // at this point, convert it
    Candidate.convertSelectToIf();
  } else if (Candidate.PUC.isInvariantPredPUC()) {
    // Clean up the candidate if it for partial unswitching
    Candidate.generatePUCInvariantPredicateIf();
  }

  auto EquivCandidatesI = std::stable_partition(
      Candidates.begin(), Candidates.end(), std::not_fn(IsEquivCandidate));

  for (auto Iter = EquivCandidatesI, E = Candidates.end(); Iter != E; ++Iter) {
    LLVM_DEBUG(dbgs() << "H: ");
    LLVM_DEBUG(Iter->dump());
    LLVM_DEBUG(dbgs() << "\n");

    if (Iter->isSelect()) {
      // The equivalent candidate that is a Select instruction must have a
      // temporary If at this point, convert it
      Iter->convertSelectToIf();
    } else if (Iter->PUC.isInvariantPredPUC()) {
      // Extract the partial entry that we want to convert
      Iter->generatePUCInvariantPredicateIf();
    }

    // Disable further unswitching in case of pass will be called more than
    // once.
    if (Iter->isIf()) {
      Iter->getIf()->setUnswitchDisabled();
    }

    // Set HLIfs that will be tracked during the cloning.
    TrackClonedNodes.insert(Iter->getNode());
  }

  for (auto &C : Candidates) {
    // Also track the condition definition instructions.
    if (C.PUC.isLoadPUC())
      TrackClonedNodes.insert(C.PUC.getLoadInstructions().begin(),
                              C.PUC.getLoadInstructions().end());
  }

  // [Label: [Node: Container]]
  CaseNodeContainerMapTy Containers;
  NodeContainerMapTy DefaultContainer;
  // Containers.reserve(std::distance(EquivCandidatesI, Candidates.end()));

  // Extract children, they will be inserted after cloning.
  // This is done to avoid cloning of children when creating the new loop.
  for (auto Iter = EquivCandidatesI, E = Candidates.end(); Iter != E; ++Iter) {
    HoistCandidate &C = *Iter;
    extractChildren(C.getNode(), Containers, DefaultContainer);
  }

  SmallVector<HoistCandidate, 8> NewCandidates;
  if (Candidate.isIf()) {
    transformIf(TargetLoop, make_range(EquivCandidatesI, Candidates.end()),
                Containers, TrackClonedNodes, NewCandidates);
  } else {
    assert(Candidate.getSwitch() && "Switch candidate expected");
    transformSwitch(TargetLoop, make_range(EquivCandidatesI, Candidates.end()),
                    Containers, DefaultContainer, TrackClonedNodes,
                    NewCandidates);
  }

  Candidates.erase(EquivCandidatesI, Candidates.end());

  // Append new candidates.
  if (!NewCandidates.empty()) {
    Candidates.append(NewCandidates.begin(), NewCandidates.end());
    sortCandidates();
  }
}

void HIROptPredicate::extractChildren(HLDDNode *Node,
                                      CaseNodeContainerMapTy &Containers,
                                      NodeContainerMapTy &DefaultContainer) {

  if (HLIf *If = dyn_cast<HLIf>(Node)) {
    // Collect Then Children.
    if (If->hasThenChildren()) {
      HLNodeUtils::remove(&Containers[0][If], If->getFirstThenChild(),
                          If->getLastThenChild());
    }

    // Collect Else Children
    if (If->hasElseChildren()) {
      HLNodeUtils::remove(&Containers[1][If], If->getFirstElseChild(),
                          If->getLastElseChild());
    }
  } else {
    HLSwitch *Switch = cast<HLSwitch>(Node);

    HLNodeUtils::remove(&DefaultContainer[Switch],
                        Switch->default_case_child_begin(),
                        Switch->default_case_child_end());

    for (int I = 1, E = Switch->getNumCases(); I <= E; ++I) {
      auto Key = Switch->getConstCaseValue(I);
      HLNodeUtils::remove(&Containers[Key][Switch], Switch->case_child_begin(I),
                          Switch->case_child_end(I));
    }
  }
}

// The method is called for a series of HLIfs with the same conditions and
// same TargetLoop. FirstIf is the first HLIf in the series. PivotIf is a
// reference to HLIf * that is used to store hoisted HLIf instance.
void HIROptPredicate::removeOrHoistIf(HoistCandidate &Candidate,
                                      HLLoop *TargetLoop, HLIf *FirstIf,
                                      HLIf *If, HLIf *&PivotIf) {
  if (!PivotIf && If == FirstIf) {
    // Move the If condition outside.

    // If the candidate is set as LoadPUC, then we need to move the definition
    // of the temps too.
    if (Candidate.PUC.isLoadPUC()) {
      SmallVector<HLInst *, 8> DefInstructions(
          Candidate.PUC.getLoadInstructions().begin(),
          Candidate.PUC.getLoadInstructions().end());

      std::sort(DefInstructions.begin(), DefInstructions.end(),
                [](const HLInst *A, const HLInst *B) {
                  return A->getTopSortNum() < B->getTopSortNum();
                });

      unsigned Level = TargetLoop->getNestingLevel();
      for (HLInst *DefInst : DefInstructions) {
        // Skip instructions that may already be removed by other candidates.
        if (!DefInst->getParent()) {
          continue;
        }

        HLInst *DefInstClone = DefInst->clone();
        HLNodeUtils::insertBefore(TargetLoop, DefInstClone);

        // Update def levels in DefInstructions.
        for (RegDDRef *Ref : make_range(DefInstClone->ddref_begin(),
                                        DefInstClone->ddref_end())) {
          Ref->updateDefLevel(Level - 1);
        }
      }
    }

    hoistIf(FirstIf, TargetLoop);
    PivotIf = If;
  } else {
    HLNodeUtils::remove(If);
  }
}

void HIROptPredicate::hoistIf(HLIf *&If, HLLoop *OrigLoop) {
  // TODO: remove loop live-ins

  // Hoist the If outside the loop.
  HLNodeUtils::moveBefore(OrigLoop, If);
}

static SmallString<32>
constructLineNumberString(SmallVector<unsigned, 8> LineNumbers) {
  SmallString<32> LoopNum;
  raw_svector_ostream VOS(LoopNum);
  auto It = LineNumbers.begin();

  if (LineNumbers.size() == 1) {
    VOS << " at line " << *It;
  } else if (LineNumbers.size() == 2) {
    VOS << " at lines ";
    VOS << *It << " and " << *std::next(It);
  } else {
    VOS << " at lines ";
    while (std::next(It) != LineNumbers.end()) {
      VOS << *It << ", ";
      ++It;
    }
    VOS << "and " << *It;
  }

  return LoopNum;
}

void HIROptPredicate::addPredicateOptReportOrigin(HLLoop *TargetLoop) {
  OptReportBuilder &ORBuilder =
      TargetLoop->getHLNodeUtils().getHIRFramework().getORBuilder();

  bool IsReportOn = ORBuilder.isOptReportOn();

  if (!IsReportOn) {
    return;
  }

  if (!OptReportVisitedSet.count(TargetLoop)) {
    // Predicate Optimized v%d
    ORBuilder(*TargetLoop).addOrigin(OptRemarkID::PredicateOptimized, VNum);
    VNum++;
    OptReportVisitedSet.insert(TargetLoop);
  }
}

void HIROptPredicate::addPredicateOptReportRemark(
    HLLoop *TargetLoop,
    const iterator_range<HoistCandidate *> &IfOrSwitchCandidates,
    OptRemarkID RemarkID) {
  OptReportBuilder &ORBuilder =
      TargetLoop->getHLNodeUtils().getHIRFramework().getORBuilder();

  bool IsReportOn = ORBuilder.isOptReportOn();

  if (!IsReportOn) {
    return;
  }

  SmallVector<unsigned, 8> LineNumbers;
  for (auto &Cand : IfOrSwitchCandidates) {
    auto &DebugLoc = Cand.isIf() ? Cand.getIf()->getDebugLoc()
                                 : Cand.getSwitch()->getDebugLoc();
    LineNumbers.push_back(DebugLoc ? DebugLoc.getLine() : 0);
  }

  auto LoopNum = constructLineNumberString(LineNumbers);

  ORBuilder(*TargetLoop).addRemark(OptReportVerbosity::Low, RemarkID, LoopNum);
}

PreservedAnalyses HIROptPredicatePass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  ModifiedHIR = HIROptPredicate(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                                AM.getResult<HIRLoopStatisticsAnalysis>(F),
                                EnablePartialUnswitch, EarlyPredicateOpt)
                    .run();

  return PreservedAnalyses::all();
}
