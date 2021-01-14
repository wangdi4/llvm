//===---------------- HIRCrossLoopArrayContraction.cpp --------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRCrossLoopArrayContraction.h"

#include "llvm/ADT/SparseBitVector.h"
#include "llvm/ADT/Statistic.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRArraySectionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
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

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(true),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

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
INITIALIZE_PASS_END(HIRCrossLoopArrayContractionLegacyPass, OPT_SWITCH,
                    OPT_DESC, false, false)

FunctionPass *llvm::createHIRCrossLoopArrayContractionLegacyPass() {
  return new HIRCrossLoopArrayContractionLegacyPass();
}

template <typename T> class HLVariant {
  HLIf *If;
  T *OriginalNode;
  T *OptimizedNode;

public:
  HLVariant(T *Node) : OriginalNode(Node), OptimizedNode(Node->clone()) {
    auto &HNU = Node->getHLNodeUtils();
    If = HNU.createHLIf(PredicateTy::FCMP_TRUE, nullptr, nullptr);
    HLNodeUtils::replace(Node, If);
    HLNodeUtils::insertAsFirstThenChild(If, OptimizedNode);
    HLNodeUtils::insertAsFirstElseChild(If, OriginalNode);
  }

  HLVariant(const HLVariant &) = delete;

  HLVariant(HLVariant &&Var)
      : If(Var.If), OriginalNode(Var.OriginalNode),
        OptimizedNode(Var.OptimizedNode) {
    Var.If = nullptr;
  }

  void invalidate(HLLoop *Lp) {
    HIRInvalidationUtils::invalidateLoopNestBody(Lp);
  }

  void commit() {
    if (If) {
      invalidate(OriginalNode);
      HLNodeUtils::replaceNodeWithBody(If, true);
      If = nullptr;
    }
  }

  ~HLVariant() {
    if (If) {
      HLNodeUtils::replaceNodeWithBody(If, false);
    }
  }

  T *getOriginal() const { return OriginalNode; }
  T *getOptimized() const { return OptimizedNode; }
};

class HIRCrossLoopArrayContraction {
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;
  HIRArraySectionAnalysis &ASA;

  SmallVector<HLVariant<HLLoop>, 4> Optimizations;

public:
  HIRCrossLoopArrayContraction(HIRFramework &HIRF, HIRDDAnalysis &DDA,
                               HIRArraySectionAnalysis &ASA)
      : HIRF(HIRF), DDA(DDA), ASA(ASA) {}

  bool run();

private:
  bool mergeLoops(HLLoop *DefLoop, HLLoop *UseLoop, unsigned Levels);
  bool runOnRegion(HLRegion &Reg);
};

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

static unsigned computeDependencyMaxTopSortNumber(
    const HLLoop *Loop, ArrayRef<const HLLoop *> IgnoreLoops, DDGraph &DDG) {
  using Gatherer = DDRefGatherer<DDRef, TerminalRefs | MemRefs | FakeRefs>;
  Gatherer::VectorTy Refs;
  Gatherer::gather(Loop, Refs);

  unsigned MaxTopSortNumber = std::numeric_limits<unsigned>::max();
  unsigned LoopMaxTopSortNumber = Loop->getMaxTopSortNum();

  for (auto *Ref : Refs) {
    for (auto &Edge : DDG.outgoing(Ref)) {
      if (!Edge->isFlow() && !Edge->isOutput()) {
        continue;
      }

      HLNode *SinkNode = Edge->getSink()->getHLDDNode();
      auto TopSortNumber = SinkNode->getTopSortNum();
      if (TopSortNumber <= LoopMaxTopSortNumber) {
        continue;
      }

      bool SinkIsInIgnoreLoop =
          std::any_of(IgnoreLoops.begin(), IgnoreLoops.end(),
                      [&](const HLLoop *IgnoreLoop) {
                        return HLNodeUtils::contains(IgnoreLoop, SinkNode);
                      });

      if (SinkIsInIgnoreLoop) {
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

static unsigned computeNumDimsContracted(const ArraySectionInfo &InfoA,
                                         const ArraySectionInfo &InfoB) {
  assert(InfoA.getNumDimensions() == InfoB.getNumDimensions() &&
         "Unexpected number of dimensions");

  int Dim = InfoA.getNumDimensions();

  // Determine dimensions that can be contracted.
  for (; Dim > 0; --Dim) {
    auto IndA = InfoA.indices(Dim - 1);
    auto IndB = InfoB.indices(Dim - 1);

    if (IndA.size() != 1 || IndB.size() != 1) {
      break;
    }

    if (!CanonExprUtils::areEqual(IndA.front(), IndB.front())) {
      break;
    }
  }

  return Dim;
}

static DirectionVector computeDirectionVector(const ArraySectionInfo &InfoA,
                                              const ArraySectionInfo &InfoB,
                                              unsigned StartLevel) {
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

    if (IVSeen.test(I - 1)) {
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

bool HIRCrossLoopArrayContraction::runOnRegion(HLRegion &Reg) {
  // Find array contraction candidates.
  SmallVector<RegDDRef *, 32> Refs;
  DDRefGathererLambda<RegDDRef>::gather(&Reg, Refs, [](const RegDDRef *Ref) {
    return Ref->isMemRef() && Ref->accessesAlloca() &&
           Ref->getNumDimensions() >= 5 && Ref->hasIV();
  });

  // Get their loops and Base pointers.
  std::map<HLLoop *, SparseBitVector<>, TopSortComparator> LoopToSymbase;
  for (auto *Ref : Refs) {
    unsigned ParentLoopLevel = getRefMinLevel(Ref);
    HLLoop *ParentLoop =
        Ref->getHLDDNode()->getParentLoopAtLevel(ParentLoopLevel);

    LoopToSymbase[ParentLoop].set(Ref->getSymbase());
  }

  DDGraph DDG = DDA.getGraph(&Reg);

  for (auto DefPair : LoopToSymbase) {
    auto *DefLp = DefPair.first;

    LLVM_DEBUG(dbgs() << "\nTaking Def-loop <" << DefLp->getNumber() << ">\n");

    SmallVector<const HLLoop *, 4> IgnoreLoops;

    if (DefLp->hasLiveOutTemps()) {
      // Can't move loops with temp live-outs.
      LLVM_DEBUG(dbgs() << "[SKIP] Def loop has live-outs.\n");
      continue;
    }

    auto &DefASAR = ASA.getOrCompute(DefLp);

    unsigned LoopDependencyLimits =
        computeDependencyMaxTopSortNumber(DefLp, {}, DDG);

    LLVM_DEBUG(dbgs() << "LoopDependencyLimit: " << LoopDependencyLimits
                      << "\n");

    for (auto UsePair : make_range(std::next(LoopToSymbase.find(DefLp)),
                                   LoopToSymbase.end())) {
      HLLoop *UseLp = UsePair.first;
      LLVM_DEBUG(dbgs() << "\t* Trying Use-loop <" << UseLp->getNumber()
                        << ">\n");

      // Check if there are common array sections.
      auto CommonSymbases = DefPair.second & UsePair.second;
      if (CommonSymbases.empty()) {
        LLVM_DEBUG(dbgs() << "\t[SKIP] No common array uses.\n");
        continue;
      }

      if (DefLp->getNestingLevel() != UseLp->getNestingLevel() &&
          DefLp->getNestingLevel() + 1 != UseLp->getNestingLevel()) {
        // This is a limitation of the current implementation.
        LLVM_DEBUG(
            dbgs() << "\t[SKIP] Use loop must be nested at the same level or "
                      "one level below.\n");
        continue;
      }

      // Check if DEF loop dominates USE loop.
      if (!HLNodeUtils::dominates(DefLp, UsePair.first)) {
        LLVM_DEBUG(dbgs() << "\t[SKIP] Def-loop does not dominate Use-loop.\n");
        continue;
      }

      // Check move DD legality.
      if (UsePair.first->getTopSortNum() >= LoopDependencyLimits) {
        LLVM_DEBUG(
            dbgs() << "\t[SKIP] Can not move here because of DD legality.\n");

        // No reasons to look further for use-loops because they are all placed
        // lexically after the current one.
        break;
      }

      auto &UseASAR = ASA.getOrCompute(UsePair.first);

      bool CommonArraySectionsAreEqual = true;
      unsigned NumDimsContracted = 0;
      unsigned FusionLevel = MaxLoopNestLevel + 1;

      for (auto Bit : CommonSymbases) {
        auto *DefSection = DefASAR.get(Bit);
        auto *UseSection = UseASAR.get(Bit);

        if (!DefSection->isDef() || !UseSection->isUse() ||
            !DefSection->equals(*UseSection)) {
          CommonArraySectionsAreEqual = false;

          LLVM_DEBUG(dbgs()
                     << "\tIncompatible array section found for base ptr: ");
          LLVM_DEBUG(DefLp->getBlobUtils().printBlob(
              dbgs(), DefLp->getBlobUtils().getBlob(Bit)));
          LLVM_DEBUG(dbgs() << ":\n");
          LLVM_DEBUG(dbgs() << "\tDef-loop: ");
          LLVM_DEBUG(DefSection->dump());
          LLVM_DEBUG(dbgs() << "\tUse-loop: ");
          LLVM_DEBUG(UseSection->dump());

          break;
        }

        ArraySectionInfo TmpDefSection;
        if (DefLp->getNestingLevel() != UseLp->getNestingLevel()) {
          TmpDefSection = DefSection->clone();
          promoteSectionIVs(TmpDefSection, DefLp->getNestingLevel());
          DefSection = &TmpDefSection;
        }

        NumDimsContracted =
            std::max(NumDimsContracted,
                     computeNumDimsContracted(*DefSection, *UseSection));

        auto DV = computeDirectionVector(
            *DefSection, *UseSection, UseLp->getNestingLevel());

        unsigned CurFusionLevel = 0;
        while (CurFusionLevel < MaxLoopNestLevel &&
               DV[CurFusionLevel] == DVKind::EQ) {
          ++CurFusionLevel;
        }

        FusionLevel = std::min(FusionLevel, CurFusionLevel);
      }

      LLVM_DEBUG(dbgs() << "\tCommonArraySectionsAreEqual: "
                        << CommonArraySectionsAreEqual << "\n");
      if (!CommonArraySectionsAreEqual) {
        LLVM_DEBUG(dbgs() << "\t[SKIP] Incompatible array sections found.\n");
        continue;
      }

      Optimizations.emplace_back(UsePair.first);
      HLLoop *UseLoopClone = Optimizations.back().getOptimized();

      // TODO: here is a place to clone loop with array contraction.
      HLLoop *DefLoopClone = DefLp->clone();

      HLNodeUtils::insertBefore(UseLoopClone, DefLoopClone);
      if (DefLp->getNestingLevel() != UseLoopClone->getNestingLevel()) {
        LLVM_DEBUG(dbgs() << "\t+ Need to promote IVs\n");

        DefLoopClone->promoteNestingLevel(DefLp->getNestingLevel());

        if (auto *ParentLoop = DefLoopClone->getParentLoop()) {
          for (unsigned LiveInSB : make_range(DefLoopClone->live_in_begin(),
                                              DefLoopClone->live_in_end())) {
            ParentLoop->addLiveInTemp(LiveInSB);
          }
        }
      }

      // Merge loops
      if (!mergeLoops(DefLoopClone, UseLoopClone,
                      FusionLevel - UseLoopClone->getNestingLevel() + 1)) {
        LLVM_DEBUG(dbgs() << "\t[SKIP] Can not merge loops.\n");

        Optimizations.pop_back();
        continue;
      }

      HLNodeUtils::remove(DefLoopClone);

      IgnoreLoops.push_back(UsePair.first);

      // Recompute limits
      LoopDependencyLimits =
          computeDependencyMaxTopSortNumber(DefLp, IgnoreLoops, DDG);

      LLVM_DEBUG(dbgs() << "\t[OK] DEF->USE Loops Merged\n");
    }
  }

  for (auto &Opt : Optimizations) {
    Reg.setGenCode();

    HLLoop *ParentLoop = Opt.getOptimized()->getParentLoop();
    if (ParentLoop) {
      HIRInvalidationUtils::invalidateBody(ParentLoop);
    } else {
      HIRInvalidationUtils::invalidateNonLoopRegion(&Reg);
    }

    Opt.commit();
  }

  return !Optimizations.empty();
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
             getAnalysis<HIRArraySectionAnalysisWrapperPass>().getASA())
      .run();
}

PreservedAnalyses
HIRCrossLoopArrayContractionPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {
  return PreservedAnalyses::all();
}
