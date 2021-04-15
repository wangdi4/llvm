//===---------------- HIRCrossLoopArrayContraction.cpp --------------------===//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRCrossLoopArrayContraction.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRArrayContractionUtils.h"

#include "llvm/ADT/SparseBitVector.h"
#include "llvm/ADT/Statistic.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRArraySectionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#define OPT_SWITCH "hir-cross-loop-array-contraction"
#define OPT_DESC "HIR Cross-Loop Array Contraction"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;
using namespace arraycontractionutils;

STATISTIC(HIRLoopsWithArrayContraction,
          "Number of HIR loop(s) activated with array contraction");
STATISTIC(HIRArrayRefsContracted,
          "Number of unique HIR array reference(s) contracted");

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(true),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

static cl::opt<unsigned> MinMemRefNumDimension(
    OPT_SWITCH "-min-memref-num-dimension", cl::init(5), cl::Hidden,
    cl::desc(OPT_DESC " Minimal MemRef Number of Dimensions"));

static cl::opt<bool> PerformIdentitySubstitution(
    OPT_SWITCH "-identity-substitution", cl::init(false), cl::Hidden,
    cl::desc("Enables identity substitution in " OPT_DESC " pass"));

namespace {

class HIRCrossLoopArrayContractionLegacyPass : public HIRTransformPass {
public:
  static char ID;
  HIRCrossLoopArrayContractionLegacyPass() : HIRTransformPass(ID) {
    initializeHIRCrossLoopArrayContractionLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRArraySectionAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.setPreservesAll();
  }
};

} // namespace

char HIRCrossLoopArrayContractionLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRCrossLoopArrayContractionLegacyPass, OPT_SWITCH,
                      OPT_DESC, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRArraySectionAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRCrossLoopArrayContractionLegacyPass, OPT_SWITCH,
                    OPT_DESC, false, false)

FunctionPass *llvm::createHIRCrossLoopArrayContractionLegacyPass() {
  return new HIRCrossLoopArrayContractionLegacyPass();
}

template <typename T> class HLVariant {
  T *OriginalNode;
  T *OptimizedNode;

  // Contains operations executed at the end of the transformation.
  // Note: this is used to completely unroll the inner loops and do the scalar
  // replacement.
  SmallVector<std::function<bool(T *)>> PostProcessors;

public:
  HLVariant(T *Node) : OriginalNode(Node->clone()), OptimizedNode(Node) {}

  HLVariant(const HLVariant &) = delete;

  HLVariant(HLVariant &&Var)
      : OriginalNode(Var.OriginalNode), OptimizedNode(Var.OptimizedNode),
        PostProcessors(std::move(Var.PostProcessors)) {
    Var.commit();
  }

  bool runPostProcessors() {
    for (auto &Func : PostProcessors) {
      if (!Func(OptimizedNode)) {
        return false;
      }
    }

    PostProcessors.clear();
    return true;
  }

  void commit() {
    assert(PostProcessors.empty() && "There are post-processors not run.");
    OriginalNode = nullptr;
  }

  ~HLVariant() {
    if (OriginalNode) {
      HLNodeUtils::replace(OptimizedNode, OriginalNode);
    }
  }

  T *getOriginal() const { return OriginalNode; }
  T *getOptimized() const { return OptimizedNode; }

  void addPostProcessor(std::function<bool(T *)> Func) {
    PostProcessors.push_back(std::move(Func));
  }
};

class HIRCrossLoopArrayContraction {
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;
  HIRArraySectionAnalysis &ASA;
  HIRLoopStatistics &HLS;

  SmallVector<HLVariant<HLLoop>, 4> Optimizations;

public:
  HIRCrossLoopArrayContraction(HIRFramework &HIRF, HIRDDAnalysis &DDA,
                               HIRArraySectionAnalysis &ASA,
                               HIRLoopStatistics &HLS)
      : HIRF(HIRF), DDA(DDA), ASA(ASA), HLS(HLS) {}

  bool run();

private:
  bool mergeLoops(HLLoop *DefLoop, HLLoop *UseLoop, unsigned Levels);
  bool runOnRegion(HLRegion &Reg);

  // Contract relevant memref(s) from a given loop, using the provided
  // symbase(s) and number of continuous dimensions contracted.
  bool contractMemRefs(HLLoop *DefLp, HLLoop *UseLp,
                       const SparseBitVector<> &CommonBases,
                       const unsigned NumDimsContracted, HLRegion &Reg,
                       unsigned UseRefMappedDim,
                       const CanonExpr *UseRefMappedCE,
                       SmallSet<unsigned, 4> &AfterContractSBS,
                       unsigned &NumRefsContracted);

  HLVariant<HLLoop> &addOptimized(HLLoop *Loop);
};

HLVariant<HLLoop> &HIRCrossLoopArrayContraction::addOptimized(HLLoop *Loop) {
  for (auto &Var : Optimizations) {
    if (Var.getOptimized() == Loop) {
      return Var;
    }
  }

  Optimizations.push_back(Loop);
  return Optimizations.back();
}

bool HIRCrossLoopArrayContraction::run() {
  bool Modified = false;
  for (auto &Reg : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    Modified = runOnRegion(cast<HLRegion>(Reg)) || Modified;
  }
  return Modified;
}

static unsigned getRefMinLevel(const RegDDRef *Ref) {
  unsigned MinLevel = NonLinearLevel;

  for (auto *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {
    auto IVLevel = CE->getFirstIVLevel();

    if (IVLevel != 0) {
      MinLevel = std::min(IVLevel, MinLevel);
    }
  }

  auto DefLevel = Ref->getDefinedAtLevel();
  if (DefLevel != 0) {
    MinLevel = std::min(DefLevel, MinLevel);
  }

  return MinLevel;
}

struct TopSortComparator {
  bool operator()(const HLNode *A, const HLNode *B) const {
    return A->getTopSortNum() < B->getTopSortNum();
  }
};

static bool isPassedToMetadataIntrinsic(const RegDDRef *Ref) {
  auto *Inst = dyn_cast<HLInst>(Ref->getHLDDNode());
  Intrinsic::ID IntrinId;
  return Inst && Inst->isIntrinCall(IntrinId) &&
         (IntrinId == Intrinsic::lifetime_start ||
          IntrinId == Intrinsic::lifetime_end);
}

static bool areIdenticalInsts(const HLInst *HInst1, const HLInst *HInst2) {

  auto *Inst1 = HInst1->getLLVMInstruction();
  auto *Inst2 = HInst2->getLLVMInstruction();

  if (Inst1->getOpcode() != Inst2->getOpcode()) {
    return false;
  }

  if (HInst1->isCallInst()) {
    return false;
  }

  // Lval was already compared in caller, compare rvals.
  auto *RvalIt1 = HInst1->rval_op_ddref_begin();
  auto *RvalIt2 = HInst2->rval_op_ddref_begin();

  for (auto End = HInst1->rval_op_ddref_end(); RvalIt1 != End;
       ++RvalIt1, ++RvalIt2) {

    auto *Ref1 = *RvalIt1;
    auto *Ref2 = *RvalIt2;

    if (!DDRefUtils::areEqual(Ref1, Ref2)) {
      return false;
    }

    for (unsigned Level = 1; Level <= MaxLoopNestLevel; ++Level) {
      if (!Ref1->hasIV(Level)) {
        continue;
      }

      auto *OuterLp1 = HInst1->getParentLoopAtLevel(Level);
      auto *OuterLp2 = HInst2->getParentLoopAtLevel(Level);

      if (!OuterLp1->isDo() || !OuterLp2->isDo() || !OuterLp1->isNormalized() ||
          !OuterLp2->isNormalized() ||
          !CanonExprUtils::areEqual(OuterLp1->getUpperCanonExpr(),
                                    OuterLp2->getUpperCanonExpr())) {
        return false;
      }
    }
  }

  return true;
}

static unsigned computeDependencyMaxTopSortNumber(
    DDGraph &DDG, const HLLoop *Loop,
    const SmallPtrSetImpl<RegDDRef *> &CandidateRefs,
    SparseBitVector<> &OutUsesByNonCandidates) {
  assert(llvm::all_of(CandidateRefs,
                      [](const RegDDRef *Ref) { return Ref->isMemRef(); }) &&
         "Only MemRefs are expected in CandidateRefs");

  using Gatherer = DDRefGatherer<RegDDRef, TerminalRefs | MemRefs | FakeRefs>;
  Gatherer::VectorTy Refs;
  Gatherer::gather(Loop, Refs);
  LLVM_DEBUG({
    dbgs() << "Refs:<" << Refs.size() << ">\n";
    unsigned Count = 0;
    for (auto Ref : Refs) {
      dbgs() << Count++ << ": ";
      Ref->dump();
      dbgs() << "\t";
    }
    dbgs() << "\n";
  });

  unsigned MaxTopSortNumber = std::numeric_limits<unsigned>::max();
  unsigned LoopMaxTopSortNumber = Loop->getMaxTopSortNum();
  unsigned LoopLevel = Loop->getNestingLevel();

  for (auto *Ref : Refs) {
    bool SrcIsCandidate = CandidateRefs.count(dyn_cast<RegDDRef>(Ref));

    for (auto &Edge : DDG.outgoing(Ref)) {
      auto *SinkRef = Edge->getSink();

      if (SrcIsCandidate) {
        // May do cast<> because IgnoreRefs contain only mem refs.
        auto SinkRegDDRef = cast<RegDDRef>(SinkRef);

        // Ignore edges between candidates, they will ba handled using Array
        // Section Analysis.
        if (CandidateRefs.count(SinkRegDDRef)) {
          continue;
        }

        // Sink is not a candidate ref, need to record a non-candidate use.
        if (!isPassedToMetadataIntrinsic(SinkRegDDRef)) {
          OutUsesByNonCandidates.set(SinkRegDDRef->getBasePtrBlobIndex());
        }
      }

      HLNode *SinkNode = SinkRef->getHLDDNode();

      auto TopSortNumber = SinkNode->getTopSortNum();
      if (TopSortNumber <= LoopMaxTopSortNumber) {
        continue;
      }

      // Ignore anti-edges for temps if the temp is defined in the current
      // loopnest. The output edge check below can take care of it.
      if (Edge->isAnti() && Ref->isTerminalRef() &&
          (Ref->getDefinedAtLevel() >= LoopLevel)) {
        continue;
      }

      // Ignore identical temp redefinitions as it doesn't change legality of
      // def loop forward substitution. These are introduced by loop
      // distribution.
      if (Edge->isOutput() && Ref->isTerminalRef() &&
          areIdenticalInsts(cast<HLInst>(Ref->getHLDDNode()),
                            cast<HLInst>(SinkNode))) {
        continue;
      }

      MaxTopSortNumber = std::min(MaxTopSortNumber, TopSortNumber);
    }
  }

  return MaxTopSortNumber;
}

bool HIRCrossLoopArrayContraction::mergeLoops(HLLoop *DefLoop, HLLoop *UseLoop,
                                              unsigned Levels) {
  assert(Levels > 0 && "Merging zero loops is undefined");

  auto FindFirstNonInstr = [](HLLoop::child_iterator Begin,
                              HLLoop::child_iterator End) {
    for (auto I = Begin; I != End; ++I) {
      if (!isa<HLInst>(*I)) {
        return I;
      }
    }

    return End;
  };

  for (auto LiveInSB :
       make_range(DefLoop->live_in_begin(), DefLoop->live_in_end())) {
    UseLoop->addLiveInTemp(LiveInSB);
  }

  for (auto LiveOutSB :
       make_range(DefLoop->live_out_begin(), DefLoop->live_out_end())) {
    UseLoop->addLiveOutTemp(LiveOutSB);
  }

  HLNodeUtils::moveAsFirstPreheaderNodes(UseLoop, DefLoop->pre_begin(),
                                         DefLoop->pre_end());
  HLNodeUtils::moveAsFirstPostexitNodes(UseLoop, DefLoop->post_begin(),
                                        DefLoop->post_end());

  auto DefInnerLoopI =
      FindFirstNonInstr(DefLoop->child_begin(), DefLoop->child_end());
  auto UseInnerLoopI =
      FindFirstNonInstr(UseLoop->child_begin(), UseLoop->child_end());

  bool NoDefInnerLoop = DefInnerLoopI == DefLoop->child_end();
  bool NoUseInnerLoop = UseInnerLoopI == UseLoop->child_end();

  // Check if this is a last level to fuse or there is no inner loops in both
  // Use and Def loops.
  if (Levels == 1 || (NoDefInnerLoop && NoUseInnerLoop)) {
    // Innermost loop case
    HLNodeUtils::moveAsFirstChildren(UseLoop, DefLoop->child_begin(),
                                     DefLoop->child_end());
    return true;
  }

  if (NoDefInnerLoop != NoUseInnerLoop) {
    return false;
  }

  HLLoop *DefInnerLoop = dyn_cast<HLLoop>(&*DefInnerLoopI);
  HLLoop *UseInnerLoop = dyn_cast<HLLoop>(&*UseInnerLoopI);

  if (!DefInnerLoop || !UseInnerLoop) {
    return false;
  }

  // Check if there are non-instr nodes after the inner loop.
  if (FindFirstNonInstr(std::next(DefInnerLoopI), DefLoop->child_end()) !=
          DefLoop->child_end() ||
      FindFirstNonInstr(std::next(UseInnerLoopI), UseLoop->child_end()) !=
          UseLoop->child_end()) {
    return false;
  }

  // Move nodes into the Use loop.
  HLNodeUtils::moveAsFirstChildren(UseLoop, DefLoop->child_begin(),
                                   DefInnerLoopI);
  HLNodeUtils::moveAfter(UseInnerLoop, std::next(DefInnerLoopI),
                         DefLoop->child_end());

  return mergeLoops(DefInnerLoop, UseInnerLoop, Levels - 1);
}

static void promoteSectionIVs(ArraySectionInfo &Info, unsigned StartLevel) {
  for (int I = 0, E = Info.getNumDimensions(); I < E; ++I) {
    for (CanonExpr *CE : Info.indices(I)) {
      CE->promoteIVs(StartLevel);
    }
  }
}

// Class describing a dimension mapped from IV in def loop to some CEs in use
// loop.
class DefToUseMappedDimension {
  unsigned IVLevel;
  unsigned DimensionNum;
  ArrayRef<const CanonExpr *> MappedCEs;

public:
  DefToUseMappedDimension() : IVLevel(0), DimensionNum(0) {}

  bool empty() { return IVLevel == 0; }

  void populate(unsigned Level, unsigned DimNum,
                ArrayRef<const CanonExpr *> CEs) {
    IVLevel = Level;
    DimensionNum = DimNum;
    MappedCEs = CEs;
  }

  unsigned getIVLevel() const { return IVLevel; }
  unsigned getDimensionNum() const { return DimensionNum; }
  ArrayRef<const CanonExpr *> getMappedCEs() const { return MappedCEs; }
};

// Compute a number of outer dimensions to contract between two array sections
// described by \p DefInfo and \p UseInfo.
//
// Dimensions may be contracted if dimensions with the same number are
// accessed by the same index and within the same bounds. The function assumes
// dimension bounds are already checked to be equal.
//             4  3  2  1
// For ex.: %A[i][j][k][m]
//          %A[i][j][m][k]
//
// The function will return 2 because dimensions 3 and 4 are accessed with
// [i][j] in both cases.
static unsigned
computeNumDimsContracted(const ArraySectionInfo &DefInfo,
                         const ArraySectionInfo &UseInfo,
                         DefToUseMappedDimension &MappedDimInfo) {
  assert(DefInfo.getNumDimensions() == UseInfo.getNumDimensions() &&
         "Unexpected number of dimensions");

  int Dim = DefInfo.getNumDimensions();

  // Determine dimensions that can be contracted.
  for (; Dim > 0; --Dim) {
    auto DefIndices = DefInfo.indices(Dim - 1);
    auto UseIndices = UseInfo.indices(Dim - 1);

    if (DefIndices.size() != 1) {
      break;
    }

    auto *DefIndex = DefIndices.front();

    if (UseIndices.size() != 1) {
      // Already mapped a dimension.
      if (!MappedDimInfo.empty()) {
        break;
      }

      unsigned Level;
      if (!DefIndex->isStandAloneIV(false, &Level)) {
        break;
      }

      bool IsValidMapping = true;
      for (auto *UseIndex : UseIndices) {
        int64_t Dist;
        if (!CanonExprUtils::getConstDistance(DefIndex, UseIndex, &Dist)) {
          IsValidMapping = false;
          break;
        }
      }

      if (!IsValidMapping) {
        break;
      }

      MappedDimInfo.populate(Level, Dim, UseIndices);

    } else if (!CanonExprUtils::areEqual(DefIndex, UseIndices.front())) {
      break;
    }
  }

  return DefInfo.getNumDimensions() - Dim;
}

// Computes direction vector for a pair of sections \p InfoA and \p InfoB.
// It results in (=) or (*) for each used IV. The function is used to
// determine the nesting level where loop fusion is possible.
static DirectionVector computeDirectionVector(const ArraySectionInfo &InfoA,
                                              const ArraySectionInfo &InfoB,
                                              unsigned StartLevel,
                                              unsigned MappedLevel) {
  BitVector IVEqual;
  BitVector IVSeen;

  IVEqual.resize(MaxLoopNestLevel, true);
  IVSeen.resize(MaxLoopNestLevel, false);

  auto SetSeen = [&](ArrayRef<const CanonExpr *> CEs, bool Equal) {
    for (auto *CE : CEs) {
      for (auto IV : enumerate(make_range(CE->iv_begin(), CE->iv_end()))) {
        if (IV.value().Coeff != 0) {
          bool AlreadySeen = IVSeen.test(IV.index());
          IVSeen.set(IV.index());

          if (Equal && !AlreadySeen) {
            IVEqual.set(IV.index());
          } else {
            IVEqual.reset(IV.index());
          }
        }
      }
    }
  };

  for (int I = 0, E = InfoA.getNumDimensions(); I < E; ++I) {
    auto IndA = InfoA.indices(I);
    auto IndB = InfoB.indices(I);

    if (IndA.size() == 1 && IndB.size() == 1 &&
        CanonExprUtils::areEqual(IndA.front(), IndB.front())) {
      SetSeen(IndA, true);
    } else {
      SetSeen(IndA, false);
      SetSeen(IndB, false);
    }
  }

  DirectionVector DV;
  DV.setAsInput(StartLevel);

  for (unsigned I = StartLevel; I <= MaxLoopNestLevel; ++I) {
    DVKind Dir = DVKind::NONE;

    if (I == MappedLevel) {
      Dir = DVKind::EQ;
    } else if (IVSeen.test(I - 1)) {
      if (IVEqual.test(I - 1)) {
        Dir = DVKind::EQ;
      } else {
        Dir = DVKind::ALL;
      }
    }

    DV[I - 1] = Dir;
  }

  return DV;
}

static void dumpBasesBitVector(HLLoop *Loop, SparseBitVector<> Bases) {
  auto &BU = Loop->getBlobUtils();
  for (auto Bit : Bases) {
    BU.printBlob(dbgs(), BU.getBlob(Bit));
    dbgs() << " ";
  }
  dbgs() << "\n";
}

static void removeDeadStores(HLLoop *Loop, SparseBitVector<> Bases) {
  LLVM_DEBUG(dbgs() << "Unused bases: "; dumpBasesBitVector(Loop, Bases));

  ForEach<HLInst>::visit(Loop, [&](HLInst *Inst) {
    auto *LVal = Inst->getLvalDDRef();
    if (!LVal || !LVal->isMemRef() ||
        !Bases.test(LVal->getBasePtrBlobIndex())) {
      return;
    }

    HLNodeUtils::remove(Inst);
  });
}

template <bool IsDef>
static SparseBitVector<>
getDefUseBasesImpl(const ArraySectionAnalysisResult &ASAR,
                   SparseBitVector<> &BasePtrIndices) {
  SparseBitVector<> Output;
  for (unsigned BasePtr : BasePtrIndices) {
    auto *SectionInfo = ASAR.get(BasePtr);

    // ASAR may be missing a base, do not update those bases in Output.
    if (!SectionInfo) {
      continue;
    }

    if (IsDef ? SectionInfo->isDef() : SectionInfo->isUse()) {
      Output.set(BasePtr);
    }
  }
  LLVM_DEBUG(dump(Output, dbgs()););
  return Output;
}

static SparseBitVector<> getDefBases(const ArraySectionAnalysisResult &ASAR,
                                     SparseBitVector<> &BasePtrIndices) {
  return getDefUseBasesImpl<true>(ASAR, BasePtrIndices);
}

static SparseBitVector<> getUseBases(const ArraySectionAnalysisResult &ASAR,
                                     SparseBitVector<> &BasePtrIndices) {
  return getDefUseBasesImpl<false>(ASAR, BasePtrIndices);
}

class BaseUses {
  SmallDenseMap<unsigned, unsigned> Uses;

public:
  void add(const SparseBitVector<> &Bases) {
    for (auto Bit : Bases) {
      ++Uses[Bit];
    }
  }

  void remove(const SparseBitVector<> &Bases) {
    for (auto Bit : Bases) {
      --Uses[Bit];
    }
  }

  SparseBitVector<> getUnused(const SparseBitVector<> &Bases) {
    SparseBitVector<> UnusedBases;
    for (auto Bit : Bases) {
      auto I = Uses.find(Bit);
      if (I == Uses.end() || I->second == 0) {
        UnusedBases.set(Bit);
      }
    }
    return UnusedBases;
  }
};

static bool isArrayContractionCandidate(const RegDDRef *Ref) {
  return Ref->isMemRef() && Ref->accessesAlloca() &&
         Ref->getNumDimensions() >= MinMemRefNumDimension && Ref->hasIV();
}

bool HIRCrossLoopArrayContraction::contractMemRefs(
    HLLoop *DefLp, HLLoop *UseLp, const SparseBitVector<> &CommonBases,
    const unsigned NumDimsContracted, HLRegion &Reg, unsigned UseRefMappedDim,
    const CanonExpr *UseRefMappedCE, SmallSet<unsigned, 4> &AfterContractSBS,
    unsigned &NumRefsContracted) {

  LLVM_DEBUG({
    dbgs() << "Def Loop: \n";
    DefLp->dump();
    dbgs() << "Use Loop: \n";
    UseLp->dump();
    dbgs() << "CommonBases: ";
    dump(CommonBases, dbgs());
    dbgs() << "\nNumDimsContracted: " << NumDimsContracted << "\n";
  });

  // Collect refs for each candidate base and contract them one at a time.
  for (auto CommonBase : CommonBases) {

    SmallVector<RegDDRef *, 32> Refs;

    DDRefGathererLambda<RegDDRef>::gather(
        DefLp, Refs, [&](const RegDDRef *Ref) {
          return (Ref->hasGEPInfo() &&
                  Ref->getBasePtrBlobIndex() == CommonBase);
        });

    if (Refs.empty()) {
      llvm_unreachable("No memrefs found in def loop for contraction!");
    }

    auto NumDefRefs = Refs.size();

    DDRefGathererLambda<RegDDRef>::gather(
        UseLp, Refs, [&](const RegDDRef *Ref) {
          return (
              Ref->hasGEPInfo() && (Ref->getBasePtrBlobIndex() == CommonBase) &&
              (!UseRefMappedCE ||
               CanonExprUtils::areEqual(Ref->getDimensionIndex(UseRefMappedDim),
                                        UseRefMappedCE)));
        });

    if (Refs.size() == NumDefRefs) {
      llvm_unreachable("No memrefs found in use loop for contraction!");
    }

    LLVM_DEBUG({
      dbgs() << "Refs:<" << Refs.size() << ">\n";
      unsigned Count = 0;
      for (auto Ref : Refs) {
        dbgs() << "\t" << Count++ << ": ";
        Ref->dump();
        dbgs() << "\n";
      }
      dbgs() << "\n";
    });

    // Construct proper PreservedDims and ToContractDims:
    SmallSet<unsigned, 4> PreservedDims;
    SmallSet<unsigned, 4> ToContractDims;
    const unsigned ToPreserveDimSize =
        Refs[0]->getNumDimensions() - NumDimsContracted;

    for (unsigned I = 1; I <= ToPreserveDimSize; ++I) {
      PreservedDims.insert(I);
    }
    for (unsigned I = 1; I <= NumDimsContracted; ++I) {
      ToContractDims.insert(ToPreserveDimSize + I);
    }

    LLVM_DEBUG({
      dbgs() << "PreservedDims: <" << PreservedDims.size() << ">\t";
      for (auto I : PreservedDims) {
        dbgs() << I << ",";
      }
      dbgs() << "\n";

      dbgs() << "ToContactDims: <" << ToContractDims.size() << ">\t";
      for (auto I : ToContractDims) {
        dbgs() << I << ",";
      }
      dbgs() << "\n";
    });

    // Contract each qualified Ref:
    RegDDRef *AfterContractRef = nullptr;
    for (auto *Ref : Refs) {
      LLVM_DEBUG(dbgs() << "Ref: "; Ref->dump(); dbgs() << "\t HLDDNode: ";
                 Ref->getHLDDNode()->dump(); dbgs() << "\n";);

      if (!HIRArrayContractionUtil::contractMemRef(
              Ref, PreservedDims, ToContractDims, Reg, AfterContractRef)) {
        LLVM_DEBUG(dbgs() << "Fail to contract Ref:\t"; Ref->dump();
                   dbgs() << "\n";);
        return false;
      }

      LLVM_DEBUG(dbgs() << "Array Contraction -- \tFrom: "; Ref->dump();
                 dbgs() << "\tTo: "; AfterContractRef->dump();
                 dbgs() << "\tAfterSB:" << AfterContractRef->getSymbase()
                        << "\n";);

      AfterContractSBS.insert(AfterContractRef->getSymbase());
      ++NumRefsContracted;
    }
  }

  return true;
}

static void replaceIVByCE(HLLoop *Lp, unsigned IVLevel,
                          const CanonExpr *ReplaceCE) {

  if (ReplaceCE->hasIV(IVLevel)) {
    unsigned ShiftAmt = ReplaceCE->getConstant();

    if (ShiftAmt != 0) {
      ForEach<RegDDRef>::visit(
          Lp, [&](RegDDRef *Ref) { Ref->shift(IVLevel, ShiftAmt); });
    }

  } else {
    llvm_unreachable("Unexpected IV replacement!");
    /*
    * TODO: Enable later.
       ForEach<RegDDRef>::visit(Lp, [&](RegDDRef *Ref) {

         for (auto *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {
           if (!replaceIVByCanonExpr(CE, IVLevel, ReplaceCE, false)) {
             llvm_unreachable("CE merging failed");
           }
       });
   */
  }
}

bool HIRCrossLoopArrayContraction::runOnRegion(HLRegion &Reg) {
  if (!Reg.isFunctionLevel()) {
    LLVM_DEBUG(dbgs() << "Skipping non-function region.\n");
    return false;
  }

  // Find array contraction candidates.
  SmallVector<RegDDRef *, 32> Refs;
  DDRefGathererLambda<RegDDRef>::gather(&Reg, Refs,
                                        isArrayContractionCandidate);

  if (Refs.empty()) {
    LLVM_DEBUG(dbgs() << "No candidate refs found.\n");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "Refs:<" << Refs.size() << ">\n";
    unsigned Count = 0;
    for (auto Ref : Refs) {
      dbgs() << Count++ << ": ";
      Ref->dump();
      dbgs() << "\t";
    }
    dbgs() << "\n";
  });

  // Keep set of interesting references to ignore DD between them.
  SmallPtrSet<RegDDRef *, 32> RefsSet(Refs.begin(), Refs.end());
  SmallSet<unsigned, 4> AfterContractSBS;

  // Get their loops and Base pointers.
  std::map<HLLoop *, SparseBitVector<>, TopSortComparator> LoopToBasePtr;
  for (auto *Ref : Refs) {
    unsigned ParentLoopLevel = getRefMinLevel(Ref);
    HLLoop *ParentLoop =
        Ref->getHLDDNode()->getParentLoopAtLevel(ParentLoopLevel);
    const unsigned BlobIdx = Ref->getBasePtrBlobIndex();
    LoopToBasePtr[ParentLoop].set(BlobIdx);
  }

  DenseMap<const HLLoop *, ArraySectionAnalysisResult> ASARCache;
  auto GetASAR = [&](const HLLoop *Loop) -> ArraySectionAnalysisResult & {
    auto &DefASAR = ASARCache[Loop];
    if (DefASAR.empty()) {
      DefASAR.merge(ASA.getOrCompute(Loop));
    }
    return DefASAR;
  };

  // Find identity matrix to substitute later
  if (PerformIdentitySubstitution) {
    SmallVector<HLLoop *, 64> InnermostLoops;
    HIRF.getHLNodeUtils().gatherInnermostLoops(InnermostLoops, &Reg);
    unsigned IdentitySB = 0;

    for (auto &Lp : InnermostLoops) {
      SmallVector<const RegDDRef *, 2> Identity;
      HLNodeUtils::findInner2DIdentityMatrix(&HLS, Lp, Identity);
      if (!Identity.empty()) {
        IdentitySB = Identity.front()->getSymbase();
        LLVM_DEBUG(
            dbgs() << Lp->getNumber()
                   << ": Loop was found containing identity matrix - SB = "
                   << IdentitySB << "!\n ");
        break;
      }
    }
    (void)IdentitySB;
  }

  DDGraph DDG = DDA.getGraph(&Reg);

  for (auto &DefPair : LoopToBasePtr) {
    auto *DefLp = DefPair.first;

    LLVM_DEBUG(dbgs() << "\nTaking Def-loop <" << DefLp->getNumber() << ">\n");

    auto &DefASAR = GetASAR(DefLp);

    auto DefBases = getDefBases(DefASAR, DefPair.second);

    LLVM_DEBUG(dbgs() << "Uses: ";
               dumpBasesBitVector(DefLp, getUseBases(DefASAR, DefPair.second)));
    LLVM_DEBUG(dbgs() << "Defines: "; dumpBasesBitVector(DefLp, DefBases));

    if (DefBases.empty()) {
      LLVM_DEBUG(dbgs() << "+ Loop has no interesting defs\n");
      continue;
    }

    BaseUses UsesTracking;

    SparseBitVector<> NonCandidateUses;
    unsigned LoopDependencyLimits = computeDependencyMaxTopSortNumber(
        DDG, DefLp, RefsSet, NonCandidateUses);

    // Add uses by non-candidate refs.
    UsesTracking.add(NonCandidateUses);

    // Add self uses.
    UsesTracking.add(getUseBases(DefASAR, DefPair.second));

    LLVM_DEBUG(dbgs() << "+ LoopDependencyLimit: " << LoopDependencyLimits
                      << "\n");

    SmallVector<HLLoop *, 2> DefKillLoops;

    for (auto &UsePair : make_range(std::next(LoopToBasePtr.find(DefLp)),
                                    LoopToBasePtr.end())) {
      HLLoop *UseLp = UsePair.first;
      LLVM_DEBUG(dbgs() << "\t* Trying Use-loop <" << UseLp->getNumber()
                        << ">\n");

      // Check if there are common array sections.
      auto CommonBases = DefBases & UsePair.second;
      LLVM_DEBUG(dbgs() << " CommonBases: "; dump(CommonBases, dbgs());
                 dbgs() << "\n";);

      if (CommonBases.empty()) {
        LLVM_DEBUG(dbgs() << "\t[SKIP] No common array uses.\n");
        continue;
      }

      UsesTracking.add(CommonBases);

      // Check if DEF loop dominates USE loop.
      if (!HLNodeUtils::dominates(DefLp, UseLp)) {
        LLVM_DEBUG(dbgs() << "\t[SKIP] Def-loop does not dominate Use-loop.\n");
        continue;
      }

      auto &UseASAR = GetASAR(UseLp);
      LLVM_DEBUG(dbgs() << "\t* Uses: "; dumpBasesBitVector(
                     UseLp, getUseBases(UseASAR, UsePair.second)));
      LLVM_DEBUG(dbgs() << "\t* Defines: "; dumpBasesBitVector(
                     UseLp, getDefBases(UseASAR, UsePair.second)));

      bool OutputDependencyFound = false;
      bool FlowDependencyFound = false;

      bool CommonArraySectionsAreEqual = true;
      unsigned NumDimsContracted = -1u;
      unsigned FusionLevel = MaxLoopNestLevel + 1;

      unsigned DefUseLevelDiff =
          (UseLp->getNestingLevel() - DefLp->getNestingLevel());

      DefToUseMappedDimension MappedDim;

      for (auto BaseIndex : CommonBases) {
        auto *DefSection = DefASAR.get(BaseIndex);
        auto *UseSection = UseASAR.get(BaseIndex);

        CommonArraySectionsAreEqual =
            CommonArraySectionsAreEqual && DefSection->equals(*UseSection);

        if (!CommonArraySectionsAreEqual) {
          LLVM_DEBUG(dbgs()
                     << "\t+ Incompatible array section found for base ptr: ");
          LLVM_DEBUG(DefLp->getBlobUtils().printBlob(
              dbgs(), DefLp->getBlobUtils().getBlob(BaseIndex)));
          LLVM_DEBUG(dbgs() << ":\n");
          LLVM_DEBUG(dbgs() << "\tDef-loop: ");
          LLVM_DEBUG(DefSection->dump());
          LLVM_DEBUG(dbgs() << "\tUse-loop: ");
          LLVM_DEBUG(UseSection->dump());
        }
        if (DefSection->isDef() && UseSection->isDef()) {
          OutputDependencyFound = true;

          LLVM_DEBUG(dbgs() << "\t+ Output dependency found: ");
          LLVM_DEBUG(DefLp->getBlobUtils().printBlob(
              dbgs(), DefLp->getBlobUtils().getBlob(BaseIndex)));
          LLVM_DEBUG(dbgs() << "\n");
          break;
        }

        if (!DefSection->isDef() || !UseSection->isUse()) {
          LLVM_DEBUG(dbgs() << "\t+ Only flow dependencies are expected\n");
          break;
        }

        FlowDependencyFound = true;

        ArraySectionInfo TmpDefSection;
        // We bail out on difference greater than 1 later.
        if (DefUseLevelDiff == 1) {
          TmpDefSection = DefSection->clone();
          promoteSectionIVs(TmpDefSection, DefLp->getNestingLevel());
          DefSection = &TmpDefSection;
        }

        // Note: use a single number of dimensions to contract for all arrays.
        // We can extend this to "per array" if needed.
        NumDimsContracted = std::min(
            NumDimsContracted,
            computeNumDimsContracted(*DefSection, *UseSection, MappedDim));

        auto DV = computeDirectionVector(*DefSection, *UseSection,
                                         UseLp->getNestingLevel(),
                                         MappedDim.getIVLevel());

        unsigned CurFusionLevel = 0;
        while (CurFusionLevel < MaxLoopNestLevel &&
               DV[CurFusionLevel] == DVKind::EQ) {
          ++CurFusionLevel;
        }

        FusionLevel = std::min(FusionLevel, CurFusionLevel);
      }

      if (FusionLevel <= 1) {
        // mergeLoops() asserts on zero 'Levels' when FusionLevel is 1.
        LLVM_DEBUG(dbgs() << "\t[SKIP] Invalid merging.\n");
        continue;
      }

      if (!CommonArraySectionsAreEqual &&
          (CommonBases.count() != 1 || MappedDim.empty())) {
        LLVM_DEBUG(dbgs() << "\t[SKIP] Incompatible array sections found.\n");
        continue;
      }

      if (OutputDependencyFound) {
        // Do not count bases that are defined in the use-loop.
        UsesTracking.remove(DefBases & getDefBases(UseASAR, UsePair.second));
        DefKillLoops.push_back(UseLp);
        continue;
      }

      // Check if the value is killed by other def loop.
      bool DefLoopKilled = llvm::any_of(DefKillLoops, [&](const HLLoop *Loop) {
        return HLNodeUtils::dominates(Loop, UseLp);
      });

      if (DefLoopKilled) {
        // Do not count this as a USE and continue with other use loops.
        LLVM_DEBUG(
            dbgs() << "\t[SKIP] Def loop is killed by another def-loop\n");
        UsesTracking.remove(CommonBases);
        continue;
      }

      if (!FlowDependencyFound) {
        LLVM_DEBUG(dbgs() << "\t[SKIP] Not a candidate\n");
        continue;
      }

      if (DefUseLevelDiff != 0 && DefUseLevelDiff != 1) {
        // This is a limitation of the current implementation.
        LLVM_DEBUG(
            dbgs() << "\t[SKIP] Use loop must be nested at the same level or "
                      "one level below.\n");
        continue;
      }

      // Check move DD legality.
      if (UseLp->getMaxTopSortNum() >= LoopDependencyLimits) {
        LLVM_DEBUG(
            dbgs() << "\t[STOP] Can not move here because of DD legality.\n");

        // No reasons to look further for use-loops because they are all
        // placed lexically after the current one.
        break;
      }

      addOptimized(UseLp);

      bool HasMappedDim = !MappedDim.empty();
      unsigned UseRefMappedDim = HasMappedDim ? MappedDim.getDimensionNum() : 0;
      unsigned NumClones = HasMappedDim ? MappedDim.getMappedCEs().size() : 1;
      bool BailOut = false;

      for (unsigned I = 0; I < NumClones; ++I) {
        HLLoop *DefLoopClone = DefLp->clone();

        // Do array contraction on qualified memref(s) in a given loop:
        unsigned NumContractedRefs = 0;
        const CanonExpr *UseRefMappedCE =
            HasMappedDim ? MappedDim.getMappedCEs()[I] : nullptr;

        if (!contractMemRefs(DefLoopClone, UseLp, CommonBases,
                             NumDimsContracted, Reg, UseRefMappedDim,
                             UseRefMappedCE, AfterContractSBS,
                             NumContractedRefs)) {
          LLVM_DEBUG(dbgs() << "Fail to contract memref(s) in Def/Use loop "
                            << DefLoopClone->getNumber() << "\n";);
        } else {
          HIRLoopsWithArrayContraction += 2;
          HIRArrayRefsContracted += NumContractedRefs;
        }

        HLNodeUtils::insertBefore(UseLp, DefLoopClone);
        if (DefUseLevelDiff == 1) {
          LLVM_DEBUG(dbgs() << "\t+ Need to promote IVs\n");

          DefLoopClone->promoteNestingLevel(DefLp->getNestingLevel());

          if (auto *ParentLoop = DefLoopClone->getParentLoop()) {
            for (unsigned LiveInSB : make_range(DefLoopClone->live_in_begin(),
                                                DefLoopClone->live_in_end())) {
              ParentLoop->addLiveInTemp(LiveInSB);
            }
          }
        }

        if (HasMappedDim) {
          replaceIVByCE(DefLoopClone, MappedDim.getIVLevel(), UseRefMappedCE);
        }

        // Merge loops
        if (!mergeLoops(DefLoopClone, UseLp,
                        FusionLevel - UseLp->getNestingLevel() + 1)) {
          LLVM_DEBUG(dbgs() << "\t[SKIP] Can not merge loops.\n");
          Optimizations.pop_back();
          BailOut = true;
          break;
        }

        HLNodeUtils::remove(DefLoopClone);
      }

      if (BailOut) {
        continue;
      }

      // Merge array section analysis results.
      UseASAR.merge(DefASAR);

      // Merge base pts but remove common bases which have been contracted.
      UsePair.second |= DefPair.second;
      UsePair.second = UsePair.second - CommonBases;

      UsesTracking.remove(CommonBases);

      LLVM_DEBUG(dbgs() << "\t[OK] DEF->USE Loops Merged\n");

      LLVM_DEBUG(dbgs() << "While " OPT_DESC "\n");
      LLVM_DEBUG(Reg.dump());
    }

    // Remove array definitions if there is no usages.
    auto UnusedBases = UsesTracking.getUnused(DefBases);
    if (!UnusedBases.empty()) {
      addOptimized(DefLp);

      removeDeadStores(DefLp, UnusedBases);
      LLVM_DEBUG(dbgs() << "[DEAD] Unused candiadate DEF stores removed\n");
      LLVM_DEBUG(Reg.dump());
    }
  }

  if (Optimizations.empty()) {
    return false;
  }

  // Run post-processors for each optimized loop. Revert everything if any
  // post-optimization fails.
  for (auto &Opt : Optimizations) {
    if (!Opt.runPostProcessors()) {
      return false;
    }
  }

  for (auto &Opt : Optimizations) {
    Reg.setGenCode();

    Opt.commit();

    HLLoop *Loop = Opt.getOptimized();
    HLLoop *ParentLoop = Loop->getParentLoop();
    HLNodeUtils::removeRedundantNodes(Loop);

    if (ParentLoop) {
      HIRInvalidationUtils::invalidateBody(ParentLoop);
    } else {
      HIRInvalidationUtils::invalidateNonLoopRegion(&Reg);
    }
  }

  return true;
}

bool HIRCrossLoopArrayContractionLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    return false;
  }

  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Cross-Loop Array Contraction\n");
    return false;
  }

  return HIRCrossLoopArrayContraction(
             getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
             getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
             getAnalysis<HIRArraySectionAnalysisWrapperPass>().getASA(),
             getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS())
      .run();
}

PreservedAnalyses HIRCrossLoopArrayContractionPass::runImpl(
    Function &F, FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  HIRCrossLoopArrayContraction(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                               AM.getResult<HIRArraySectionAnalysisPass>(F),
                               AM.getResult<HIRLoopStatisticsAnalysis>(F))
      .run();
  return PreservedAnalyses::all();
}
