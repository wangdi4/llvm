//===------- HIRArraySectionAnalysis.cpp ----------------------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRArraySectionAnalysis.h"

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#include "llvm/Support/CommandLine.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

using namespace llvm;
using namespace llvm::loopopt;

#define OPT_SWITCH "hir-array-section-analysis"
#define OPT_DESCR "HIR Array Section Analysis"
#define DEBUG_TYPE OPT_SWITCH

AnalysisKey HIRArraySectionAnalysisPass::Key;
HIRArraySectionAnalysis
HIRArraySectionAnalysisPass::run(Function &F, FunctionAnalysisManager &AM) {
  return HIRArraySectionAnalysis(AM.getResult<HIRFrameworkAnalysis>(F),
                                 AM.getResult<HIRDDAnalysisPass>(F));
}

FunctionPass *llvm::createHIRArraySectionAnalysisPass() {
  return new HIRArraySectionAnalysisWrapperPass();
}

char HIRArraySectionAnalysisWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRArraySectionAnalysisWrapperPass, OPT_SWITCH, OPT_DESCR,
                      false, true)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRArraySectionAnalysisWrapperPass, OPT_SWITCH, OPT_DESCR,
                    false, true)

void HIRArraySectionAnalysisWrapperPass::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
}

bool HIRArraySectionAnalysisWrapperPass::runOnFunction(Function &F) {
  ASA.reset(new HIRArraySectionAnalysis(
      getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
      getAnalysis<HIRDDAnalysisWrapperPass>().getDDA()));
  return false;
}

HIRArraySectionAnalysis::HIRArraySectionAnalysis(HIRFramework &HIRF,
                                                 HIRDDAnalysis &DDA)
    : HIRAnalysis(HIRF), DDA(DDA) {}

void HIRArraySectionAnalysis::markLoopBodyModified(const HLLoop *Lp) {
  while (Lp && Cache.erase(Lp)) {
    Lp = Lp->getParentLoop();
  }
}

void HIRArraySectionAnalysis::markLoopBoundsModified(const HLLoop *Lp) {
  markLoopBodyModified(Lp);
}

void HIRArraySectionAnalysis::print(formatted_raw_ostream &OS,
                                    const HLLoop *Lp) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  auto &Sections = getOrCompute(Lp);
  Sections.print(OS, Lp);
  OS << "\n";
#endif
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void ArraySectionInfo::print(raw_ostream &OS) const {
  if (Lowers.empty()) {
    OS << "*";
    return;
  }

  OS << "(";
  auto UseDef = getUseDefFlags();
  if (UseDef == UDFlagTy::UNKNOWN) {
    OS << "UNKNOWN";
  } else {
    if (isUse()) {
      OS << "USE";
    }
    if (isDef()) {
      OS << "DEF";
    }
  }
  OS << ") ";

  for (auto Pair : reverse(zip(Lowers, Uppers))) {
    OS << "[";
    auto *LCE = std::get<0>(Pair);
    if (LCE) {
      LCE->print(OS, false);
    } else {
      OS << "*";
    }

    OS << ":";

    auto *UCE = std::get<1>(Pair);
    if (UCE) {
      UCE->print(OS, false);
    } else {
      OS << "*";
    }
    OS << "]";
  }
}
#endif

ArraySectionInfo ArraySectionInfo::clone() const {
  ArraySectionInfo CloneInfo;

  CloneInfo.UDFlag = UDFlag;

  auto CloneCE = [](const CanonExpr *CE) { return CE ? CE->clone() : nullptr; };

  std::transform(Lowers.begin(), Lowers.end(),
                 std::back_inserter(CloneInfo.Lowers), CloneCE);
  std::transform(Uppers.begin(), Uppers.end(),
                 std::back_inserter(CloneInfo.Uppers), CloneCE);

  return CloneInfo;
}

static bool replaceIVsByBound(CanonExpr *CE, unsigned Level,
                              const HLLoop *ParentLoop, bool IsLowerCE) {
  if (!CE || !CE->hasIV(Level)) {
    return true;
  }

  if (ParentLoop->isUnknown() || CE->hasIVBlobCoeff(Level)) {
    return false;
  }

  if (CE->getIVConstCoeff(Level) < 0) {
    IsLowerCE = !IsLowerCE;
  }

  const HLLoop *Lp = ParentLoop->getNestingLevel() == Level
                         ? ParentLoop
                         : ParentLoop->getParentLoopAtLevel(Level);

  return CanonExprUtils::replaceIVByCanonExpr(
      CE, Level, IsLowerCE ? Lp->getLowerCanonExpr() : Lp->getUpperCanonExpr(),
      Lp->hasSignedIV(), true);
}

static ArraySectionInfo
computeSectionsFromGroup(ArrayRef<const RegDDRef *> Group,
                         unsigned OuterLoopLevel) {
  const RegDDRef *SeedRef = Group.front();
  ArraySectionInfo Info(SeedRef->getNumDimensions());

  // Classify group with USE/DEF.
  for (auto *Ref : Group) {
    if (Info.isUse() && Info.isDef()) {
      break;
    }

    if (Ref->isLval()) {
      Info.setDef();
    } else {
      Info.setUse();
    }
  }

  // Find MinCE and MaxCE across every dimension within a group.
  for (int I = 0, E = SeedRef->getNumDimensions(); I < E; ++I) {
    auto IsDimLinearAtLevel = [&](const RegDDRef *Ref) {
      return Ref->getDimensionStride(I + 1)->isLinearAtLevel(OuterLoopLevel) &&
             Ref->getDimensionLower(I + 1)->isLinearAtLevel(OuterLoopLevel) &&
             Ref->getDimensionIndex(I + 1)->isLinearAtLevel(OuterLoopLevel);
    };

    // If there are non-linear CEs return conservative result.
    if (!std::all_of(Group.begin(), Group.end(), IsDimLinearAtLevel)) {
      Info.lowers()[I] = nullptr;
      Info.uppers()[I] = nullptr;
      continue;
    }

    auto MinMax = std::minmax_element(
        Group.begin(), Group.end(), [&](const RegDDRef *A, const RegDDRef *B) {
          auto *ACE = A->getDimensionIndex(I + 1);
          auto *BCE = B->getDimensionIndex(I + 1);

          return CanonExprUtils::compare(ACE, BCE);
        });

    const RegDDRef *MinRef = *MinMax.first;
    const RegDDRef *MaxRef = *MinMax.second;

    CanonExpr *MinCE = MinRef->getDimensionIndex(I + 1)->clone();
    CanonExpr *MaxCE = MaxRef->getDimensionIndex(I + 1)->clone();

    if (!replaceIVsByBound(MinCE, OuterLoopLevel,
                           MinRef->getLexicalParentLoop(), true)) {
      MinCE = nullptr;
    }

    if (!replaceIVsByBound(MaxCE, OuterLoopLevel,
                           MaxRef->getLexicalParentLoop(), false)) {
      MaxCE = nullptr;
    }

    Info.lowers()[I] = MinCE;
    Info.uppers()[I] = MaxCE;
  }

  return Info;
}

static void groupRefs(ArrayRef<const RegDDRef *> Refs,
                      DDRefGrouping::RefGroupVecTy<const RegDDRef *> &Groups) {
  DenseMap<unsigned, unsigned> GroupIndex;

  for (auto *Ref : Refs) {
    unsigned &GroupNo = GroupIndex[Ref->getBasePtrBlobIndex()];
    if (GroupNo == 0) {
      GroupNo = GroupIndex.size();
      Groups.emplace_back();
    }

    Groups[GroupNo - 1].push_back(Ref);
  }
}

static void replaceIVInSection(ArraySectionInfo &Info, const HLLoop *Loop) {
  unsigned LoopLevel = Loop->getNestingLevel();

  for (auto *&CE : Info.lowers()) {
    if (!replaceIVsByBound(CE, LoopLevel, Loop, true)) {
      CE = nullptr;
    }
  }

  for (auto *&CE : Info.uppers()) {
    if (!replaceIVsByBound(CE, LoopLevel, Loop, false)) {
      CE = nullptr;
    }
  }
}

// The function merges sections of \p InResult into \p OutResult.
//
// 1. If a section from InResult doesn't exist in OutResult it will be copied
// into OutResult.
// 2. If section present in both results: (ex.: [0, 10] and [50, 100]), the
// section in OutResult will be replaced with the new one, covering both ranges
// (ex. [0, 100]).
// 3. Any IVs of the \p Loop in InResult will be replaced by the Loop bounds.
static void mergeResult(ArraySectionAnalysisResult &OutResult,
                        const ArraySectionAnalysisResult &InResult,
                        const HLLoop *Loop) {
  for (unsigned BaseIndex : InResult.knownBaseIndices()) {
    ArraySectionInfo InSectionClone = InResult.get(BaseIndex)->clone();
    replaceIVInSection(InSectionClone, Loop);

    auto *OutSection = OutResult.get(BaseIndex);

    // If no section for BaseIndex exist in OutResult then just use
    // InSectionClone.
    if (!OutSection) {
      OutResult.create(BaseIndex) = std::move(InSectionClone);
      continue;
    }

    // Now merge incoming section with outgoing.
    if (InSectionClone.isDef()) {
      OutSection->setDef();
    }

    if (InSectionClone.isUse()) {
      OutSection->setUse();
    }

    // If incoming section is not consistent with current section, clear the
    // section to provide conservative answer.
    if (InSectionClone.getNumDimensions() != OutSection->getNumDimensions()) {
      OutSection->clear();
      continue;
    }

    // For each dimension select new lower and upper CE.
    for (unsigned I = 0, E = OutSection->getNumDimensions(); I < E; ++I) {
      int64_t Distance;

      auto *&LCE1 = OutSection->lowers()[I];
      auto *LCE2 = InSectionClone.lowers()[I];

      if (LCE1 != nullptr && LCE2 != nullptr &&
          CanonExprUtils::getConstDistance(LCE1, LCE2, &Distance, true)) {
        if (Distance > 0) {
          LCE1 = LCE2;
        }
      } else {
        LCE1 = nullptr;
      }

      auto *&UCE1 = OutSection->uppers()[I];
      auto *UCE2 = InSectionClone.uppers()[I];

      if (UCE1 != nullptr && UCE2 != nullptr &&
          CanonExprUtils::getConstDistance(UCE1, UCE2, &Distance, true)) {
        if (Distance < 0) {
          UCE1 = UCE2;
        }
      } else {
        UCE1 = nullptr;
      }
    }
  }
}

// Do the merge for each \p InnerLoopResults into \p OutResult, replacing \p
// Loop IVs with the bounds.
static void
mergeResults(ArraySectionAnalysisResult &OutResult,
             ArrayRef<const ArraySectionAnalysisResult *> InnerLoopResults,
             const HLLoop *Loop) {
  for (auto *InnerResult : InnerLoopResults) {
    mergeResult(OutResult, *InnerResult, Loop);
  }
}

const ArraySectionAnalysisResult &
HIRArraySectionAnalysis::getOrCompute(const HLLoop *Loop) {
  auto CachedResult = Cache.find(Loop);
  if (CachedResult != Cache.end()) {
    return *CachedResult->second;
  }

  using Gatherer = DDRefGatherer<const RegDDRef, MemRefs>;

  Gatherer::VectorTy Refs;
  Gatherer::gatherRange<true, false>(Loop->child_begin(), Loop->child_end(),
                                     Refs);

  unsigned LoopLevel = Loop->getNestingLevel();

  SmallDenseSet<unsigned> InvalidBaseIndices;

  // Invalidate bases that may alias with each other.
  auto DDG = DDA.getGraph(Loop);
  SmallPtrSet<const DDRef *, 32> Visited;
  for (auto *Ref : Refs) {
    SmallVector<const RegDDRef *, 8> Worklist;
    Worklist.push_back(Ref);

    while (!Worklist.empty()) {
      auto *NextRef = Worklist.pop_back_val();
      if (Visited.count(NextRef)) {
        continue;
      }

      Visited.insert(NextRef);

      for (auto *Edge : DDG.outgoing(Ref)) {
        auto *SinkRef = cast<const RegDDRef>(Edge->getSink());
        Worklist.push_back(SinkRef);

        if (!DDRefUtils::haveConstDimensionDistances(Ref, SinkRef, true)) {
          InvalidBaseIndices.insert(Ref->getBasePtrBlobIndex());
          InvalidBaseIndices.insert(SinkRef->getBasePtrBlobIndex());
        }
      }
    }
  }

  // Compute results for inner loops
  SmallVector<const ArraySectionAnalysisResult *, 4> InnerLoopsResults;
  if (!Loop->isInnermost()) {
    SmallVector<const HLLoop *, 4> InnerLoops;
    HLNodeUtils::gatherLoopsWithLevel(Loop, InnerLoops, LoopLevel + 1);

    for (auto *Lp : InnerLoops) {
      InnerLoopsResults.push_back(&getOrCompute(Lp));
    }
  }

  // Group loop references.
  DDRefGrouping::RefGroupVecTy<const RegDDRef *> Groups;
  groupRefs(Refs, Groups);

  auto Result = std::make_unique<ArraySectionAnalysisResult>();
  for (auto &Group : Groups) {
    ArraySectionInfo &Info =
        Result->create(Group.front()->getBasePtrBlobIndex());

    // Check that group is consistent across references.
    if (!InvalidBaseIndices.count(Group.front()->getBasePtrBlobIndex())) {
      Info = computeSectionsFromGroup(Group, LoopLevel);
    }
  }

  // Merge results of children loops into the Result.
  mergeResults(*Result, InnerLoopsResults, Loop);

  // Reset results for invalid bases.
  for (unsigned BaseIndex : InvalidBaseIndices) {
    auto *Section = Result->get(BaseIndex);
    if (Section) {
      Section->clear();
    }
  }

  // Cache and return the result
  return *(Cache[Loop] = std::move(Result));
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void ArraySectionAnalysisResult::print(formatted_raw_ostream &OS,
                                       const HLLoop *Lp) const {
  if (ArraySections.empty()) {
    if (Lp) {
      Lp->indent(OS, Lp->getNestingLevel());
    }
    OS << "< Empty Array Section Info >";
    return;
  }

  std::function<void(raw_ostream &, unsigned)> PrintBlobFn =
      [](raw_ostream &OS, unsigned BlobIndex) { OS << BlobIndex; };

  if (Lp) {
    PrintBlobFn = [Lp](raw_ostream &OS, unsigned BlobIndex) {
      Lp->getBlobUtils().printBlob(OS, Lp->getBlobUtils().getBlob(BlobIndex));
    };
  } else {
    auto Array = ArraySections.begin()->second.lowers();
    if (!Array.empty() && Array.front() != nullptr) {
      PrintBlobFn = [Array](raw_ostream &OS, unsigned BlobIndex) {
        Array.front()->getBlobUtils().printBlob(
            OS, Array.front()->getBlobUtils().getBlob(BlobIndex));
      };
    }
  }

  for (auto &Pair : enumerate(ArraySections)) {
    if (Lp) {
      Lp->indent(OS, Lp->getNestingLevel());
    }

    PrintBlobFn(OS, Pair.value().first);
    OS << ": ";

    Pair.value().second.print(OS);
    if (Pair.index() != ArraySections.size() - 1) {
      OS << "\n";
    }
  }
}
#endif
