//===------ HIRParVecAnalysis.h - Provides Parallel/Vector --*-- C++ --*---===//
//                                Candidate Analysis
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRVecIdioms.h"

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
    UNKNOWN_CALL = 15527,
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
  SmallVector<const DDEdge *, 1> ParEdges;

  // DD Edges preventing vectorization
  SmallVector<const DDEdge *, 1> VecEdges;

  // Maximum safe vectorization length
  unsigned Safelen = UINT_MAX;

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
  // If set, at least one of the inner loops is an unknown loop.
  HLLoop *InnerUnknownLoop;
  // If set, at least one switch-case construct exists.
  HLSwitch *Switch;

  ParVecInfo(AnalysisMode Mode, HLLoop *HLoop);

  // Field accessors
  LoopType getParType() const { return ParType; }
  LoopType getVecType() const { return VecType; }

  void setSafelen(unsigned Slen) {
    // If the allowed vectorization safe length is smaller than
    // the current allowed safe length, update Safelen value.
    if (Slen < Safelen)
      Safelen = Slen;
  }
  unsigned getSafelen() const { return Safelen; }

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

  const SmallVectorImpl<const DDEdge *> &getParEdges() const {
    return ParEdges;
  }
  const SmallVectorImpl<const DDEdge *> &getVecEdges() const {
    return VecEdges;
  }

  void addParEdge(const DDEdge *Edge) { ParEdges.push_back(Edge); };
  void addVecEdge(const DDEdge *Edge) { VecEdges.push_back(Edge); };
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
  void analyze(HLLoop *Loop, const TargetTransformInfo *TTI,
               TargetLibraryInfo *TLI, HIRDDAnalysis *DDA,
               HIRSafeReductionAnalysis *SRA);

  /// \brief Print the analysis result.
  void print(raw_ostream &OS, bool WithLoop = true) const;

  /// \brief Main accessor for the ParVecInfo.
  static ParVecInfo *get(AnalysisMode Mode, HIRParVecInfoMapType &InfoMap,
                         const TargetTransformInfo *TTI, TargetLibraryInfo *TLI,
                         HIRDDAnalysis *DDA, HIRSafeReductionAnalysis *SRA,
                         HLLoop *Loop) {

    auto &Info = InfoMap[Loop];

    if (!Info)
      Info.reset(new ParVecInfo(Mode, Loop));
    // TODO: Query Mode and cached Mode may be different. Add code
    //       to deal with such situation.

    if (!Info->isDone())
      Info->analyze(Loop, TTI, TLI, DDA, SRA);

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
  const TargetTransformInfo *TTI;
  TargetLibraryInfo *TLI;
  HIRDDAnalysis *DDA;
  HIRSafeReductionAnalysis *SRA;
  HIRParVecInfoMapType InfoMap;

public:
  HIRParVecAnalysis(bool Enabled, const TargetTransformInfo *TTI,
                    TargetLibraryInfo *TLI, HIRFramework *HIRF,
                    HIRDDAnalysis *DDA, HIRSafeReductionAnalysis *SRA)
      : HIRAnalysis(*HIRF), Enabled(Enabled), TTI(TTI), TLI(TLI), DDA(DDA),
        SRA(SRA) {}

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
  HIRParVecAnalysisWrapperPass();

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

class HIRVectorIdiomAnalysis {
public:
  HIRVectorIdiomAnalysis() = default;

  void gatherIdioms(const TargetTransformInfo *TTI, HIRVectorIdioms &IList,
                    const DDGraph &DDG, HIRSafeReductionAnalysis &SRA,
                    HLLoop *Loop);
};

} // namespace loopopt
} // namespace llvm

#endif // INTEL_LOOPANALYSIS_PVA_H
