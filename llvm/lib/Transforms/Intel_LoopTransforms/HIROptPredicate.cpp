//===--- HIROptPredicate.cpp - Implements OptPredicate class --------------===//
//
// Copyright (C) 2015-2022 Intel Corporation. All rights reserved.
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
#include "llvm/ADT/Triple.h"
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

// Disable converting Select instructions into If/Else to perform unswitching
static cl::opt<bool> DisableUnswitchSelect("disable-" OPT_SWITCH "-select",
                                            cl::init(false), cl::Hidden,
                                            cl::desc("Disable " OPT_DESC
                                                     " for select "
                                                     "instructions"));

namespace {

// Partial Unswitch context
struct PUCandidate {
  bool IsRequired = false;

  SmallPtrSet<HLInst *, 8> Instructions;

  bool IsUpdatedInThenBranch = false;
  bool IsUpdatedInElseBranch = false;

public:
  SmallPtrSetImpl<HLInst *> &getInstructions() { return Instructions; }

  bool isPUCandidate() const {
    return !(IsUpdatedInThenBranch && IsUpdatedInElseBranch);
  }

  bool isPURequired() const { return IsRequired; }

  void setPURequired() { IsRequired = true; }

  PUCandidate() {}

  PUCandidate(const PUCandidate &Arg, const HLNodeMapper &Mapper)
      : IsRequired(Arg.IsRequired),
        IsUpdatedInThenBranch(Arg.IsUpdatedInThenBranch),
        IsUpdatedInElseBranch(Arg.IsUpdatedInElseBranch) {
    for (HLInst *Inst : Arg.Instructions) {
      // Skip instructions that are already removed.
      if (!Inst->getParent()) {
        continue;
      }

      HLInst *ClonedInst = Mapper.getMapped(Inst);
      assert(ClonedInst && "Nullptr instruction");
      Instructions.insert(ClonedInst);
    }
  }

#ifndef NDEBUG
  LLVM_DUMP_METHOD
  void dump() {
    dbgs() << "[ ";

    dbgs() << (IsUpdatedInThenBranch ? "T" : "F");
    dbgs() << "/";
    dbgs() << (IsUpdatedInElseBranch ? "T" : "F");
    dbgs() << " ";

    for (HLInst *Inst : Instructions) {
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

  HoistCandidate(HLDDNode *Node, unsigned Level, const PUCandidate &PUC)
      : Node(Node), Level(Level), PUC(PUC), CreatedFromSelect(false) {
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
        PUC(Orig.PUC, Mapper), CreatedFromSelect(Orig.CreatedFromSelect) {}

  HoistCandidate() : Level(0) {}

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
  SmallPtrSet<const HLIf *, 4> OuterLoopIfs;

  // Set used to track which regions has been modified by the optimization.
  SmallSetVector<const HLRegion *, 16> RegionThresholdSet;

  // Opt-report chunk number
  unsigned VNum = 1;

  // Sets including visited TargetLoop, visited NewElseLoop and If line nums
  SmallPtrSet<HLLoop *, 8> OptReportVisitedSet;

  /// Returns true if the non-linear rval Ref of the If condition may be
  /// unswitched together with its definition.
  bool isPUCandidate(const HLIf *If, const RegDDRef *Ref, PUContext &PU) const;

  /// Returns true of false whatever \p Edge prevents unswitching of non-linear
  /// condition. Populates PU and RefsStack.
  bool processPUEdge(const HLIf *If, DDEdge *Edge, PUContext &PU,
                     SmallVectorImpl<const RegDDRef *> &RefsStack,
                     DDGraph &DDG) const;

  /// Sorts candidates in top sort number descending.
  /// They will be handled from right to left.
  void sortCandidates();

  /// Clears opt-report structures between HIR regions.
  void clearOptReportState();

  /// This routine will loop through the candidate loop to look
  /// for HLIf candidates. If all conditions are met, it will transform
  /// the loop. Returns true if transformation happened.
  bool processOptPredicate(bool &HasMultiexitLoop);

  /// Returns the deepest level at which any of the If/Ref operands is defined.
  unsigned getPossibleDefLevel(const HLIf *If, PUContext &PUC);
  unsigned getPossibleDefLevel(const HLSwitch *Switch, PUContext &PUC);
  unsigned getPossibleDefLevel(const HLDDNode *Node, const RegDDRef *Ref,
                               PUContext &PUC);

  /// Returns the possible level where CE is defined.
  ///
  /// NonLinearBlob is set true whenever CE contains a non-linear blob.
  /// UDivBlob is set to true whenever CE contains a non-constant division.
  unsigned getPossibleDefLevel(const CanonExpr *CE, bool &NonLinearBlob,
                               bool &UDivBlob);

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
      unsigned RemarkID);

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

template <typename VisitorTy>
static void visitChildren(VisitorTy &Visitor, HLIf *If) {
  HLNodeUtils::visitRange(Visitor, If->then_begin(), If->then_end());
  HLNodeUtils::visitRange(Visitor, If->else_begin(), If->else_end());
}

template <typename VisitorTy>
static void visitChildren(VisitorTy &Visitor, HLSwitch *Switch) {
  for (int I = 1, E = Switch->getNumCases(); I <= E; ++I) {
    HLNodeUtils::visitRange(Visitor, Switch->case_child_begin(I),
                            Switch->case_child_end(I));
  }
  HLNodeUtils::visitRange(Visitor, Switch->default_case_child_begin(),
                          Switch->default_case_child_end());
}

static bool isUnswitchDisabled(HLSwitch *) { return false; }
static bool isUnswitchDisabled(HLIf *If) { return If->isUnswitchDisabled(); }

struct HIROptPredicate::CandidateLookup final : public HLNodeVisitorBase {
  HLNode *SkipNode;
  HIROptPredicate &Pass;
  HIRLoopStatistics &HLS;
  unsigned MinLevel;
  bool TransformLoop;
  unsigned CostOfRegion;
  // If we are analyzing a loop, then we are going to keep the first Select
  // instruction that we found. All other Select instructions will be
  // candidates if they have the same condition as this one.
  HLInst *FirstSelectCandidate;

  CandidateLookup(HIROptPredicate &Pass, HIRLoopStatistics &HLS,
      bool TransformLoop = true, unsigned MinLevel = 0,
      unsigned CostOfRegion = 0) : SkipNode(nullptr), Pass(Pass), HLS(HLS),
      MinLevel(MinLevel), TransformLoop(TransformLoop),
      CostOfRegion(CostOfRegion), FirstSelectCandidate(nullptr) {}

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
  bool isCandidateForPerfectLoopNest(
      SmallVectorImpl<const HLNode *> &ChildNodes);
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

bool HIROptPredicate::processPUEdge(
    const HLIf *If, DDEdge *Edge, PUContext &PU,
    SmallVectorImpl<const RegDDRef *> &RefsStack, DDGraph &DDG) const {
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
      PU.IsUpdatedInThenBranch = true;
    } else if (If->isElseChild(Inst)) {
      PU.IsUpdatedInElseBranch = true;
    } else {
      // Is updated outside of the If construct
      return false;
    }

    // Is updated in both If branches
    if (PU.IsUpdatedInThenBranch && PU.IsUpdatedInElseBranch) {
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

  PU.getInstructions().insert(Inst);

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
bool HIROptPredicate::isPUCandidate(const HLIf *If, const RegDDRef *Ref,
                                    PUContext &PU) const {
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

    // Check that the VRef value is independent from ParentLoop's IV.
    if (VRef->hasIV(ParentLoopLevel)) {
      return false;
    }

    // Look for all incoming flow edges.

    for (const BlobDDRef *BDDRef :
         make_range(VRef->blob_begin(), VRef->blob_end())) {
      for (auto &Edge : DDG.incoming(BDDRef)) {
        if (!processPUEdge(If, Edge, PU, RefsStack, DDG)) {
          return false;
        }
      }
    }

    for (auto &Edge : DDG.incoming(VRef)) {
      if (!processPUEdge(If, Edge, PU, RefsStack, DDG)) {
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

void HIROptPredicate::CandidateLookup::visit(HLIf *If) {
  // Skip bottom test in unknown loops.
  HLLoop *ParentLoop = If->getParentLoop();
  if (!ParentLoop || ParentLoop->getBottomTest() == If) {
    return;
  }

  visitIfOrSwitch(If);
}

void HIROptPredicate::CandidateLookup::visit(HLSwitch *Switch) {
  visitIfOrSwitch(Switch);
}

template <typename NodeTy>
void HIROptPredicate::CandidateLookup::visitIfOrSwitch(NodeTy *Node) {
  HLLoop *ParentLoop = Node->getParentLoop();
  if (!ParentLoop) {
    return;
  }

  SkipNode = Node;

  // Partial unswitch context
  PUContext PUC;

  bool IsCandidate = TransformLoop && !isUnswitchDisabled(Node);

  // Skip candidates that are already at the outer most possible level.
  unsigned Level;

  if (IsCandidate) {
    // Determine target level to unswitch.
    Level = std::max(Pass.getPossibleDefLevel(Node, PUC), MinLevel);

    if (Level < ParentLoop->getNestingLevel()) {
      // Check if condition does not depend on both T/F branches at the same
      // time.
      IsCandidate = PUC.isPUCandidate();
    } else {
      IsCandidate = false;
    }
  } else {
    Level = ParentLoop->getNestingLevel();
  }

  if (IsCandidate && Pass.EarlyPredicateOpt && Level != 0) {
    // Suppress single level predicate optimization for complete unroll
    // candidates as it may complicate the analysis for unrolling and subsequent
    // passes.
    uint64_t TC;
    bool IsCompleteUnrollCandidate =
        (ParentLoop->isInnermost() &&
         (Level == ParentLoop->getNestingLevel() - 1) &&
         ParentLoop->isConstTripLoop(&TC) && TC < 4);

    // Check if unswitching breaks the existing loopnest perfectness.
    IsCandidate = !IsCompleteUnrollCandidate &&
                  (Node->getParentLoopAtLevel(Level)->getNumChildren() > 1);
  }

  if (IsCandidate && PUC.isPURequired()) {
    // HLIf should be unconditionally executed to unswitch.
    IsCandidate = HLNodeUtils::postDominates(Node, ParentLoop->getFirstChild());
  }

  // Check for unsafe calls in branches that can modify the condition.
  // TODO: Do we need HIRStatistics instead of LoopStatistics?
  if (IsCandidate && PUC.isPURequired()) {
    // Implemented for HLIfs only
    HLIf *If = cast<HLIf>(Node);
    UnsafeCallFinder LoopUnsafe(Node), ThenUnsafe, ElseUnsafe;
    HLNodeUtils::visitRange(LoopUnsafe, ParentLoop->child_begin(),
                            ParentLoop->child_end());
    HLNodeUtils::visitRange(ThenUnsafe, If->then_begin(), If->then_end());
    HLNodeUtils::visitRange(ElseUnsafe, If->else_begin(), If->else_end());
    PUC.IsUpdatedInThenBranch = PUC.IsUpdatedInThenBranch ||
                                ThenUnsafe.hasUnsafeCall() ||
                                LoopUnsafe.hasUnsafeCall();
    PUC.IsUpdatedInElseBranch = PUC.IsUpdatedInElseBranch ||
                                ElseUnsafe.hasUnsafeCall() ||
                                LoopUnsafe.hasUnsafeCall();
    IsCandidate = PUC.isPUCandidate();
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

  LLVM_DEBUG(dbgs() << "Opportunity: ");
  LLVM_DEBUG(Node->dumpHeader());
  LLVM_DEBUG(dbgs() << " --> Level " << Level << ", Candidate: " << IsCandidate
                    << (PUC.isPURequired() ? "(PU)" : "") << "\n");

  // Tell inner candidates that parent candidate will not be unswitched.
  // Do not unswitch inner candidates in case of partial unswitching.
  bool WillUnswitchParent = IsCandidate && !PUC.isPURequired();

  // Collect candidates within HLIf branches.
  CandidateLookup Lookup(Pass, HLS, WillUnswitchParent, Level, CostOfRegion);
  visitChildren(Lookup, Node);

  if (!IsCandidate) {
    return;
  }

  Pass.Candidates.emplace_back(Node, Level, PUC);
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

// Return true if the input vector has only one HLNode entry, the node
// is HLIf, and the definition level is 0
bool HIROptPredicate::CandidateLookup::isCandidateForPerfectLoopNest(
    SmallVectorImpl<const HLNode *> &ChildNodes) {
  if (ChildNodes.size() != 1)
    return false;

  auto *If = dyn_cast<const HLIf>(ChildNodes[0]);
  if (!If)
    return false;

  // Don't allow the unswitch if the cost of the region is larger than
  // the threshold. There is a chance that the size of the function grows
  // big enough to produce slowdowns.
  if (CostOfRegion > RegionCostThreshold)
    return false;

  PUContext PUC;
  // Check if the condition can be hoisted to the outermost loop and if
  // partial unswitch won't be performed. Partial unswitch is disabled
  // at the moment since we don't want to create a branch with perfect loop
  // nest and the other branch without prefect loop nest. This is
  // conservative and can relaxed in the future through profiling and/or
  // branch analysis.
  bool Result = Pass.getPossibleDefLevel(If, PUC) == 0 &&
                               !PUC.isPURequired();
  if (Result)
    Pass.OuterLoopIfs.insert(If);

  return Result;
}

void HIROptPredicate::CandidateLookup::visit(HLLoop *Loop) {

  SkipNode = Loop;

  bool TransformCurrentLoop = true;
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
        !isCandidateForPerfectLoopNest(ChildNodes))
      TransformCurrentLoop = false;
  }

  // We will only process SIMD loops if the SIMD intrinsics are the only
  // instructions in the pre-header and post-exit nodes of the loop. The
  // reason is because they get cloned automatically without handling any
  // special transformation.
  if (Loop->isSIMD()) {
    if (!Loop->isInnermost()) {
      TransformCurrentLoop = false;
    } else if (Loop->getNumPreheader() != 1 || Loop->getNumPostexit() != 1) {
      TransformCurrentLoop = false;
    } else {
      auto *PreHeaderInst = cast<HLInst>(Loop->getFirstPreheaderNode());
      auto *PostExitInst = cast<HLInst>(Loop->getFirstPostexitNode());
      if (!PreHeaderInst->isSIMDDirective() ||
          !PostExitInst->isSIMDEndDirective())
        TransformCurrentLoop = false;
    }
  }

  CandidateLookup
      Lookup(Pass, HLS, TransformCurrentLoop, MinLevel, CostOfRegion);
  Loop->getHLNodeUtils().visitRange(Lookup, Loop->child_begin(),
                                    Loop->child_end());
}

// If the instruction is a Select instruction, is in the innermost loop and the
// condition is loop invariant, then add it as a candidate. The Select
// instruction will be converted into an If/Else which will be unswitched.
void HIROptPredicate::CandidateLookup::visit(HLInst *Inst) {
  if (!isa<SelectInst>(Inst->getLLVMInstruction()) || DisableUnswitchSelect ||
      Pass.EarlyPredicateOpt || !TransformLoop)
    return;

  // Current loop should be innermost and a Do loop
  HLLoop *CurrLoop = Inst->getLexicalParentLoop();
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
    auto LoopStats = HLS.getTotalLoopStatistics(CurrLoop);
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
    auto *InstDDRef = Inst->getOperandDDRef(I);
    PUContext PUC;
    unsigned DefLevel = Pass.getPossibleDefLevel(Inst, InstDDRef, PUC);
    if (DefLevel >= LoopLevel)
      return;

    MaxLevel = std::max(DefLevel, MaxLevel);
  }

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

  for (auto Candidate : Candidates) {
    if (Candidate.isIf()) {
      auto *If = cast<HLIf>(Candidate.getNode());
      if (!OuterLoopIfs.contains(If))
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

  for (HLNode &Node : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    HLRegion *Region = cast<HLRegion>(&Node);

    LLVM_DEBUG(dbgs() << "Region: " << Region->getNumber() << ":\n");

    CandidateLookup Lookup(*this, HLS);
    HLNodeUtils::visit(Lookup, Region);
    sortCandidates();

    LLVM_DEBUG(dumpCandidates());

    bool MustCodeGen = mustCodeGen();
    bool HasMultiexitLoop;
    if (processOptPredicate(HasMultiexitLoop)) {
      if (MustCodeGen)
        Region->setGenCode();
      HLNodeUtils::removeRedundantNodes(Region, false);

      if (HasMultiexitLoop) {
        HLNodeUtils::updateNumLoopExits(Region);
      }
    }

    clearOptReportState();
    Candidates.clear();
    ThresholdMap.clear();
    OuterLoopIfs.clear();
  }

  return false;
}

unsigned HIROptPredicate::getPossibleDefLevel(const CanonExpr *CE,
                                              bool &NonLinearBlob,
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
  if (DefLevel == NonLinearLevel) {
    // The caller uses max IV level if there was non linear blobs.
    NonLinearBlob = true;
    return IVMaxLevel;
  }

  return std::max(IVMaxLevel, DefLevel);
}

unsigned HIROptPredicate::getPossibleDefLevel(const HLDDNode *Node,
                                              const RegDDRef *Ref,
                                              PUContext &PUC) {
  unsigned Level = 0;
  bool NonLinearRef = false;
  bool UDivBlob = false;
  bool HasGEPInfo = Ref->hasGEPInfo();

  if (HasGEPInfo) {
    Level = getPossibleDefLevel(Ref->getBaseCE(), NonLinearRef, UDivBlob);
  }

  for (unsigned I = 1, NumDims = Ref->getNumDimensions(); I <= NumDims; ++I) {
    Level = std::max(Level, getPossibleDefLevel(Ref->getDimensionIndex(I),
                                                NonLinearRef, UDivBlob));

    if (HasGEPInfo) {
      Level = std::max(Level, getPossibleDefLevel(Ref->getDimensionLower(I),
                                                  NonLinearRef, UDivBlob));
      Level = std::max(Level, getPossibleDefLevel(Ref->getDimensionStride(I),
                                                  NonLinearRef, UDivBlob));
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

  if (NonLinearRef || Ref->isMemRef()) {
    // Return current level of attachment.
    Level = NodeLevel;

    const HLIf *If = dyn_cast<HLIf>(Node);

    // May hoist one level up only.
    if (If && isPUCandidate(If, Ref, PUC)) {
      PUC.setPURequired();
      Level -= 1;
    }
  }

  return Level;
}

unsigned HIROptPredicate::getPossibleDefLevel(const HLIf *If, PUContext &PUC) {
  unsigned Level = 0;

  for (auto PI = If->pred_begin(), PE = If->pred_end(); PI != PE; ++PI) {
    const RegDDRef *Ref1 = If->getLHSPredicateOperandDDRef(PI);
    const RegDDRef *Ref2 = If->getRHSPredicateOperandDDRef(PI);

    Level = std::max(Level, getPossibleDefLevel(If, Ref1, PUC));
    if (Level == NonLinearLevel) {
      return Level;
    }

    Level = std::max(Level, getPossibleDefLevel(If, Ref2, PUC));
    if (Level == NonLinearLevel) {
      return Level;
    }
  }

  return Level;
}

unsigned HIROptPredicate::getPossibleDefLevel(const HLSwitch *Switch,
                                              PUContext &PUC) {
  return getPossibleDefLevel(Switch, Switch->getConditionDDRef(), PUC);
}

void HIROptPredicate::clearOptReportState() {
  VNum = 1;
  OptReportVisitedSet.clear();
}

/// processOptPredicate - Main routine to perform opt predicate
/// transformation.
bool HIROptPredicate::processOptPredicate(bool &HasMultiexitLoop) {
  bool Modified = false;
  HasMultiexitLoop = false;

  while (!Candidates.empty()) {
    HoistCandidate &Candidate = Candidates.back();

    // There shouldn't be any Candidate set as Bottom.
    assert(Candidate.isValidCandidate() &&
           "Invalid instruction selected as candidate");

    HLDDNode *PilotIfOrSwitch = Candidate.getNode();

    ConditionsAnalyzed++;

    HLLoop *ParentLoop = PilotIfOrSwitch->getParentLoop();
    assert(ParentLoop && "Candidate should have a parent loop");

    if (ParentLoop->isMultiExit()) {
      // It will be used in the caller to update exit counts.
      HasMultiexitLoop = true;
    }

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
    auto LoopStats = HLS.getTotalLoopStatistics(TargetLoop);
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
    if (!TargetLoop->isSIMD()) {
      // TODO: check if candidate is defined in preheader
      TargetLoop->extractZttPreheaderAndPostexit();
    }

    // TransformLoop and its clones.
    transformCandidate(TargetLoop, Candidate);

    LLVM_DEBUG(dbgs() << "While " OPT_DESC ":\n");
    LLVM_DEBUG(ParentLoop->getParentRegion()->dump());
    LLVM_DEBUG(dbgs() << "\n");

    // Calculate statistics
    if (Candidate.PUC.isPURequired()) {
      ConditionsPUUnswitched++;
    }

    ConditionsUnswitched++;

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

static bool hasEqualParentNode(HLNode *FromNode, HLLoop *ToLoop) {
  HLIf *FromIf = dyn_cast<HLIf>(FromNode);
  HLSwitch *FromSwitch = dyn_cast<HLSwitch>(FromNode);

  for (HLNode *Node = FromNode->getParent(); Node != ToLoop;
       Node = Node->getParent()) {
    assert(Node && "FromNode should be a child of ToLoop");

    HLIf *ParentIf = dyn_cast<HLIf>(Node);
    if (FromIf && ParentIf) {
      if (HLNodeUtils::areEqualConditions(FromIf, ParentIf)) {
        return true;
      }
    }

    HLSwitch *ParentSwitch = dyn_cast<HLSwitch>(Node);
    if (FromSwitch && ParentSwitch) {
      if (HLNodeUtils::areEqualConditions(FromSwitch, ParentSwitch)) {
        return true;
      }
    }
  }

  return false;
}

void HIROptPredicate::transformSwitch(
    HLLoop *TargetLoop, iterator_range<HoistCandidate *> SwitchCandidates,
    CaseNodeContainerMapTy &CaseContainers,
    NodeContainerMapTy &DefaultContainer,
    SmallPtrSetImpl<HLNode *> &TrackClonedNodes,
    SmallVectorImpl<HoistCandidate> &NewCandidates) {
  HLSwitch *OriginalSwitch = SwitchCandidates.begin()->getSwitch();

  auto &HNU = OriginalSwitch->getHLNodeUtils();
  auto &DRU = HNU.getDDRefUtils();

  HLSwitch *OuterSwitch =
      HNU.createHLSwitch(OriginalSwitch->getConditionDDRef()->clone());

  bool IsRemarkAdded = false;

  // Populate cases of the outer Switch with loop clones.
  for (auto &Case : CaseContainers) {
    auto CaseDDRef = DRU.createConstDDRef(
        OriginalSwitch->getConditionDDRef()->getDestType(), Case.first);
    OuterSwitch->addCase(CaseDDRef);

    LoopUnswitchNodeMapper CloneMapper(TrackClonedNodes, Candidates);
    HLLoop *NewLoop = TargetLoop->clone(&CloneMapper);

    addPredicateOptReportOrigin(NewLoop);
    if (!IsRemarkAdded) {
      // Invariant Switch condition%s hoisted out of this loop
      addPredicateOptReportRemark(NewLoop, SwitchCandidates, 25424u);
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

void HIROptPredicate::transformIf(
    HLLoop *TargetLoop, iterator_range<HoistCandidate *> IfCandidates,
    CaseNodeContainerMapTy &CaseContainers,
    SmallPtrSet<HLNode *, 32> TrackClonedNodes,
    SmallVectorImpl<HoistCandidate> &NewCandidates) {

  // Create the else loop by cloning the main loop.
  LoopUnswitchNodeMapper CloneMapper(TrackClonedNodes, Candidates);
  HLLoop *NewElseLoop = TargetLoop->clone(&CloneMapper);

  HLNodeUtils::insertAfter(TargetLoop, NewElseLoop);

  ThresholdMap[NewElseLoop] = ++ThresholdMap[TargetLoop];

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

  // If the If/Else was created from a select instruction then we use a
  // generic opt-report remark
  unsigned OptReportRemark = 25423u;
  for (auto &Candidate : IfCandidates) {
    if (Candidate.createdFromSelect()) {
      OptReportRemark = 25422u;
      break;
    }
  }

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

    if (C.PUC.IsUpdatedInThenBranch) {
      // Place original HLIf block
      HLNodeUtils::insertAsFirstThenChildren(If, &ThenContainer);
      HLNodeUtils::insertAsFirstElseChildren(If, &ElseContainer);
    } else {
      // Place *then* branch unconditionally.
      if (!ThenContainer.empty()) {
        HLContainerTy *ThenContainerPtr = &ThenContainer;
        HLContainerTy CloneContainer;
        if (C.PUC.IsUpdatedInElseBranch) {
          // If true then nodes will be needed for second loop.
          HLNodeUtils::cloneSequence(&CloneContainer, &ThenContainer.front(),
                                     &ThenContainer.back());
          ThenContainerPtr = &CloneContainer;
        }

        HLNodeUtils::insertAfter(If, ThenContainerPtr);
      }

      removeOrHoistIf(C, TargetLoop, FirstIf, If, PivotIf);

      for (HLInst *RedundantInst : C.PUC.getInstructions()) {
        // Remove instruction if it's still attached. May be removed by
        // another candidate.
        if (RedundantInst->getParent()) {
          addLvalAsLivein(RedundantInst->getLvalDDRef(), TargetLoop);
          HLNodeUtils::remove(RedundantInst);
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
    if (C.PUC.IsUpdatedInElseBranch) {
      // Place original HLIf block
      HLNodeUtils::insertAsFirstThenChildren(ClonedIf, &ThenContainer);
      HLNodeUtils::insertAsFirstElseChildren(ClonedIf, &ElseContainer);
    } else {
      // Place *else* branch unconditionally.
      if (!ElseContainer.empty()) {
        HLNodeUtils::insertAfter(ClonedIf, &ElseContainer);
      }

      removeOrHoistIf(C, TargetLoop, FirstIfClone, ClonedIf, PivotIf);

      for (HLInst *RedundantInst : C.PUC.getInstructions()) {
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

  NewCandidates.append(CloneMapper.getNewCandidates().begin(),
                       CloneMapper.getNewCandidates().end());

  updateDefLevels(PivotIf, PivotIf->child_begin(), PivotIf->child_end());
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
        if (C == Candidate) {
          return true;
        }

        bool IsEquiv =
            C.Level == Candidate.Level && C.isIf() == Candidate.isIf() &&
            // Have same condition
            ((C.isIf() &&
              HLNodeUtils::areEqualConditions(C.getIf(), Candidate.getIf())) ||
             (C.isSwitch() && HLNodeUtils::areEqualConditions(
                               C.getSwitch(), Candidate.getSwitch()))) &&
          // And are in the same target loop
          HLNodeUtils::contains(TargetLoop, C.getNode(), false);

        // Check if the current candidate is a Select instruction that will be
        // converted into an If. If so, then collect all the possible equal
        // conditions
        if (!IsEquiv && C.Level == Candidate.Level && C.isSelect() &&
            Candidate.isIf()) {
          IsEquiv =
              HLNodeUtils::areEqualConditions(C.getSelect(),
                                              Candidate.getIf()) &&
              HLNodeUtils::contains(TargetLoop, C.getSelect(), false);
        }

        if (!IsEquiv) {
          return false;
        }

        // Can not handle candidate if there is an equal parent node.
        return !hasEqualParentNode(C.getNode(), TargetLoop);
      };


  // The Candidate that is a Select instruction must have a temporary If
  // at this point, convert it
  if (Candidate.isSelect())
    Candidate.convertSelectToIf();

  auto EquivCandidatesI = std::stable_partition(
      Candidates.begin(), Candidates.end(), std::not1(IsEquivCandidate));

  for (auto Iter = EquivCandidatesI, E = Candidates.end(); Iter != E; ++Iter) {
    LLVM_DEBUG(dbgs() << "H: ");
    LLVM_DEBUG(Iter->dump());
    LLVM_DEBUG(dbgs() << "\n");

    // The equivalent candidate that is a Select instruction must have a
    // temporary If at this point, convert it
    if (Iter->isSelect())
      Iter->convertSelectToIf();

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
    TrackClonedNodes.insert(C.PUC.getInstructions().begin(),
                            C.PUC.getInstructions().end());
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

    SmallVector<HLInst *, 8> DefInstructions(
        Candidate.PUC.getInstructions().begin(),
        Candidate.PUC.getInstructions().end());

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
    ORBuilder(*TargetLoop).addOrigin(25476u, VNum);
    VNum++;
    OptReportVisitedSet.insert(TargetLoop);
  }
}

void HIROptPredicate::addPredicateOptReportRemark(
    HLLoop *TargetLoop,
    const iterator_range<HoistCandidate *> &IfOrSwitchCandidates,
    unsigned RemarkID) {
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
  HIROptPredicate(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                  AM.getResult<HIRLoopStatisticsAnalysis>(F),
                  EnablePartialUnswitch, EarlyPredicateOpt)
      .run();

  return PreservedAnalyses::all();
}

class HIROptPredicateLegacyPass : public HIRTransformPass {
  bool EnablePartialUnswitch;
  bool EarlyPredicateOpt;

public:
  static char ID;

  HIROptPredicateLegacyPass(bool EnablePartialUnswitch = true,
                            bool EarlyPredicateOpt = false)
      : HIRTransformPass(ID), EnablePartialUnswitch(EnablePartialUnswitch),
        EarlyPredicateOpt(EarlyPredicateOpt) {
    initializeHIROptPredicateLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    // Loop Statistics is not used by this pass directly but it used by
    // HLNodeUtils::dominates() utility. This is a workaround to keep the pass
    // manager from freeing it.
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIROptPredicate(getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
                           getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
                           getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS(),
                           EnablePartialUnswitch, EarlyPredicateOpt)
        .run();
  }
};

char HIROptPredicateLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIROptPredicateLegacyPass, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIROptPredicateLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIROptPredicatePass(bool EnablePartialUnswitch,
                                              bool EarlyPredicateOpt) {
  return new HIROptPredicateLegacyPass(EnablePartialUnswitch,
                                       EarlyPredicateOpt);
}
