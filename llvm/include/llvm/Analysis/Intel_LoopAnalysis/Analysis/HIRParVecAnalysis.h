//===------ HIRParVecAnalysis.h - Provides Parallel/Vector --*-- C++ --*---===//
//                                Candidate Analysis
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// The primary purpose of this pass is to provide a lazily evaluated
// parallelizability/vectorizability analysis for HIR. Clients
// specify the HLNodes (or hierarchy) for which analysis is required.
// We try to avoid recomputation whenever possible, even if the HIR has been
// been modified. In order to do this, clients must specify how they modify HIR
// at the region/loop level such that correct invalidation is performed.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_LOOPANALYSIS_PVA_H
#define INTEL_LOOPANALYSIS_PVA_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/Pass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRAnalysisPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"

namespace llvm {
class TargetLibraryInfo;

namespace loopopt {

class HIRFramework;
class HIRDDAnalysis;
class HIRSafeReductionAnalysis;
class DDEdge;
class DDGraph;
class HLRegion;
class HLSwitch;
class HLInst;

class ParVecInfo;
using HIRParVecInfoMapType =
    DenseMap<const HLLoop *, std::unique_ptr<ParVecInfo>>;

/// \brief Main data structure describing parallelizability/vectorizability
/// of a given loop. If not parallelizable/vectorizable, reason and source
/// location are stored for later reporting.
class ParVecInfo {

public:
  enum AnalysisMode {
    None,
    Parallel,                     // Parallelization analysis only
    ParallelForThreadizer,        // Prints out diagnostic
    Vector,                       // Vectorization analysis only
    VectorForVectorizerInnermost, // For O2. Prints out diagnostic
    VectorForVectorizer,          // Prints out diagnostic
    ParallelVector                // Generic
  };

  enum LoopType {
    Analyzing,
    ParOkay,
    VecOkay,
    SIMD,
    NOVECTOR_PRAGMA_LOOP = 15319,
    FE_DIAG_PAROPT_VEC_VECTOR_DEPENDENCE = 15344,
    FE_DIAG_VEC_FAIL_EMPTY_LOOP = 15414,
    SWITCH_STMT = 15535,
    UNKNOWN_CALL = 15537,
    NON_DO_LOOP = 15536,
    UNROLL_PRAGMA_LOOP = 15538,
    FE_DIAG_VEC_NOT_INNERMOST = 15553,
    EH,
    NON_NORMALIZED_LOOP
  };

private:
  // Per-loop data for ParVec analysis
  HLLoop *HLoop;

  // Analysis mode can be different from one loop to another.
  AnalysisMode Mode;

  // Parallelization analysis result and related source location.
  LoopType ParType;
  DebugLoc ParLoc;

  // Vectorization analysis result and related source location.
  LoopType VecType;
  DebugLoc VecLoc;

  // DD Edges preventing parallelization
  SmallVector<DDEdge *, 1> ParEdges;

  // DD Edges preventing vectorization
  SmallVector<DDEdge *, 1> VecEdges;

  // If set, at least one of the inner loops is an unknown loop.
  HLLoop *InnerUnknownLoop;

  // If set, at least one switch-case construct exists.
  HLSwitch *Switch;

  /// \brief Non-diagnostic strings. Analysis incomplete or
  /// diagnostic has to come from threadizer/vectorizer.
  static const std::string LoopTypeString[];

  /// \brief Returns true if Mode requires non-vectorization/
  /// non-parallelization diagnostic and analysis result
  /// shows non-vectorization/non-parallelization.
  bool isEmitMode() {
    return (Mode == ParallelForThreadizer && ParType != ParOkay) ||
           (Mode == VectorForVectorizer && SIMD < VecType) ||
           (Mode == VectorForVectorizerInnermost && SIMD < VecType);
  }

  /// \brief Emit non-vectorization/non-parallelization diagnostic.
  /// To be called when non-vectorization/non-parallelization
  /// reason is set.
  void emitDiag();

  /// \brief Print indentation for the loop level. Helper for print().
  void printIndent(raw_ostream &OS, bool ZeroBase) const;

public:
  ParVecInfo(AnalysisMode Mode, HLLoop *HLoop);

  // Field accessors
  LoopType getParType() const { return ParType; }
  LoopType getVecType() const { return VecType; }

  void setParType(LoopType T) {
    if (isParallelMode())
      ParType = T;
  }

  void setVecType(LoopType T) {
    if (isVectorMode())
      VecType = T;
  }

  DebugLoc getParLoc() { return ParLoc; }
  DebugLoc getVecLoc() { return VecLoc; }

  void setParLoc(DebugLoc Loc) { ParLoc = Loc; }
  void setVecLoc(DebugLoc Loc) { VecLoc = Loc; }

  void setLoc(DebugLoc Loc) {
    setParLoc(Loc);
    setVecLoc(Loc);
  }

  void cleanEdges() {
    ParEdges.clear();
    VecEdges.clear();
  };

  void addParEdge(DDEdge *Edge) { ParEdges.push_back(Edge); };
  void addVecEdge(DDEdge *Edge) { VecEdges.push_back(Edge); };
  bool isParallelMode() const { return isParallelMode(Mode); }
  bool isVectorMode() const { return isVectorMode(Mode); }

  bool isDone() const {

    if (!isParallelMode()) {
      return Analyzing != VecType;
    }

    if (!isVectorMode()) {
      return Analyzing != ParType;
    }
    return (Analyzing != VecType) && (Analyzing != ParType);
  }

  static bool isParallelMode(AnalysisMode Mode) {
    return Mode == Parallel || Mode == ParallelForThreadizer ||
           Mode == ParallelVector;
  }

  static bool isVectorMode(AnalysisMode Mode) {
    return Mode == Vector || Mode == VectorForVectorizer ||
           Mode == VectorForVectorizerInnermost || Mode == ParallelVector;
  }

  /// \brief Main analysis function.
  void analyze(HLLoop *Loop, TargetLibraryInfo *TLI, HIRDDAnalysis *DDA,
               HIRSafeReductionAnalysis *SRA);

  /// \brief Print the analysis result.
  void print(raw_ostream &OS, bool WithLoop = true) const;

  /// \brief Main accessor for the ParVecInfo.
  static ParVecInfo *get(AnalysisMode Mode, HIRParVecInfoMapType &InfoMap,
                         TargetLibraryInfo *TLI, HIRDDAnalysis *DDA,
                         HIRSafeReductionAnalysis *SRA, HLLoop *Loop) {

    auto &Info = InfoMap[Loop];

    if (!Info)
      Info.reset(new ParVecInfo(Mode, Loop));
    // TODO: Query Mode and cached Mode may be different. Add code
    //       to deal with such situation.

    if (!Info->isDone())
      Info->analyze(Loop, TLI, DDA, SRA);

    return Info.get();
  }

  // TODO: This function doesn't handle vectorizable but not parallelizable.
  static void set(AnalysisMode Mode, HIRParVecInfoMapType &InfoMap,
                  HLLoop *Loop, AnalysisMode InfoMode, LoopType T,
                  DebugLoc Loc) {

    auto &Info = InfoMap[Loop];

    if (!Info)
      Info.reset(new ParVecInfo(Mode, Loop));

    if (isParallelMode(InfoMode)) {
      Info->setParLoc(Loc);
      Info->setParType(T);
    }

    if (isVectorMode(InfoMode)) {
      Info->setVecLoc(Loc);
      Info->setVecType(T);
    }
  }
};

class HIRParVecAnalysis : public HIRAnalysis {
  bool Enabled;
  TargetLibraryInfo *TLI;
  HIRDDAnalysis *DDA;
  HIRSafeReductionAnalysis *SRA;
  HIRParVecInfoMapType InfoMap;

public:
  HIRParVecAnalysis(bool Enabled, TargetLibraryInfo *TLI, HIRFramework *HIRF,
                    HIRDDAnalysis *DDA, HIRSafeReductionAnalysis *SRA)
      : HIRAnalysis(*HIRF), Enabled(Enabled), TLI(TLI), DDA(DDA), SRA(SRA) {}

  /// \brief Analyze (if invalid) the loop and return the info.
  const ParVecInfo *getInfo(ParVecInfo::AnalysisMode Mode, HLLoop *Loop);

  /// \brief Analyze the entire function to make all loops to have valid info.
  void analyze(ParVecInfo::AnalysisMode Mode);

  /// \brief Analyze the region to make all loops in the region to have valid
  /// info.
  void analyze(ParVecInfo::AnalysisMode Mode, HLRegion *Region);

  /// \brief Analyze the loop nest to make all loops in the nest to have valid
  /// info. For analyzing just this loop, use getInfo() instead.
  void analyze(ParVecInfo::AnalysisMode Mode, HLLoop *Loop);

  void markLoopBodyModified(const HLLoop *L) override;

  void printAnalysis(raw_ostream &OS) const override;

  /// \brief Helper for skipping ParVec analysis on SIMD enabled functions.
  static bool isSIMDEnabledFunction(Function &F);
};

class HIRParVecAnalysisWrapperPass : public FunctionPass {
  std::unique_ptr<HIRParVecAnalysis> HPVA;

public:
  static char ID;
  HIRParVecAnalysisWrapperPass() : FunctionPass(ID) {
    initializeHIRParVecAnalysisWrapperPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnFunction(Function &F) override;
  void releaseMemory() override { HPVA.reset(); }

  void print(raw_ostream &OS, const Module * = nullptr) const override {
    getHPVA().printAnalysis(OS);
  }

  HIRParVecAnalysis &getHPVA() { return *HPVA; }
  const HIRParVecAnalysis &getHPVA() const { return *HPVA; }
};

class HIRParVecAnalysisPass : public AnalysisInfoMixin<HIRParVecAnalysisPass> {
  friend struct AnalysisInfoMixin<HIRParVecAnalysisPass>;
  static AnalysisKey Key;

public:
  using Result = HIRParVecAnalysis;
  HIRParVecAnalysis run(Function &F, FunctionAnalysisManager &AM);
};

template <class Instruction> class VectorIdioms {
public:
  enum IdiomId {
    NoIdiom = 0,
    // Min or Max main instruction in minmax+index idiom.
    MinOrMax,
    // Index instruction of minmax+index idiom.
    MMFirstLastLoc,
  };

private:
  using IdiomListTy = MapVector<const Instruction *, IdiomId>;
  using IdiomLinksTy =
      DenseMap<const Instruction *, SmallPtrSet<const Instruction *, 2>>;

public:
  using iterator = typename IdiomListTy::iterator;
  using const_iterator = typename IdiomListTy::const_iterator;
  using LinkedIdiomListTy = SmallPtrSetImpl<const Instruction *>;

  VectorIdioms() = default;
  VectorIdioms(const VectorIdioms &) = delete;
  VectorIdioms &operator=(const VectorIdioms &) = delete;

  /// Mark \p Instr with IdiomId \p Id.
  void addIdiom(const Instruction *Instr, IdiomId Id) {
    assert(Id != NoIdiom && "Expected idiom");
    iterator Iter = IdiomData.find(Instr);
    if (Iter != IdiomData.end())
      assert(Iter->second == Id && "Conflicting idiom");
    else
      IdiomData[Instr] = Id;
  }

  /// Return IdiomId if \p Instr is registered as idiom or NoIdiom if
  /// it is not.
  IdiomId isIdiom(const Instruction *Instr) const {
    const_iterator Iter = IdiomData.find(Instr);
    return Iter == IdiomData.end() ? NoIdiom : Iter->second;
  }

  /// Add \p Linked as an idiom and link it to \p Master.
  void addLinked(const Instruction *Master, const Instruction *Linked,
                 IdiomId Id) {
    assert(isIdiom(Master) != NoIdiom && "Expected master idiom registered");
    addIdiom(Linked, Id);
    IdiomLinks[Master].insert(Linked);
  }

  /// Link already inserted idioms.
  void linkIdiom(const Instruction *Master, const Instruction *Linked,
                 IdiomId Id) {
    assert(isIdiom(Master) != NoIdiom && "Expected master idiom registered");
    assert((Id != NoIdiom && isIdiom(Linked) == Id) &&
           "Expected linked idiom registered");
    IdiomLinks[Master].insert(Linked);
  }

  const_iterator begin() const { return IdiomData.begin(); }
  const_iterator end() const { return IdiomData.end(); }

  /// Return list of idioms linked to \p Master
  const LinkedIdiomListTy *getLinkedIdioms(const Instruction *Master) const {
    auto Iter = IdiomLinks.find(Master);
    return Iter == IdiomLinks.end() ? nullptr : &Iter->second;
  }

  /// Predicate whether \p Id means a standalone idiom.
  static bool isStandaloneIdiom(IdiomId Id) { return false; }

  /// Predicate whether \p Id marks idioms that require the linked ones.
  static bool isMasterIdiom(IdiomId Id) { return Id == MinOrMax; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const {
    raw_ostream &OS = dbgs();
    OS << "Idiom List\n";
    if (IdiomData.empty()) {
      OS << "  No idioms detected.\n";
      return;
    }
    for (auto &Idiom : IdiomData)
      if (isMasterIdiom(Idiom.second) || isStandaloneIdiom(Idiom.second)) {
        OS << getIdiomName(Idiom.second) << ": ";
        Idiom.first->dump();
        if (const LinkedIdiomListTy *LinkedList = getLinkedIdioms(Idiom.first))
          for (const Instruction *Linked : *LinkedList) {
            auto IdiomCode = isIdiom(Linked);
            OS << "  " << getIdiomName(IdiomCode) << ": ";
            Linked->dump();
          }
      }
  }

  static const char *getIdiomName(IdiomId Id) {
    static const char *Names[] = {"NoIdiom", "MinOrMax", "MMFirstLastLoc"};
    return Names[Id];
  }
#endif

private:
  IdiomListTy IdiomData;
  IdiomLinksTy IdiomLinks;
};

using HIRVectorIdioms = VectorIdioms<HLInst>;

// Deleter, to use with std::unique_ptr<HIRVectorIdioms> when this header is
// not included and HIRVectorIdioms is forward declared as incomplete.
// This declaration is unnecessary here, keeping just to able to copy it in the
// needed place.
extern void deleteHIRVectorIdioms(HIRVectorIdioms *);

class HIRVectorIdiomAnalysis {
public:
  HIRVectorIdiomAnalysis() = default;

  void gatherIdioms(HIRVectorIdioms &IList, const DDGraph &DDG,
                    HIRSafeReductionAnalysis &SRA, HLLoop *Loop);
};

} // namespace loopopt
} // namespace llvm

#endif // INTEL_LOOPANALYSIS_PVA_H
