//===------ HIRParVecAnalysis.h - Provides Parallel/Vector --*-- C++ --*---===//
//                                Candidate Analysis
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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
#include "llvm/IR/DebugLoc.h"
#include "llvm/Pass.h"

namespace llvm {
namespace loopopt {

class HIRFramework;
class HIRDDAnalysis;
class HIRSafeReductionAnalysis;
class DDEdge;
class HLRegion;
class HLLoop;
class HLSwitch;

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
    FE_DIAG_PAROPT_VEC_VECTOR_DEPENDENCE = 15344,
    FE_DIAG_VEC_FAIL_EMPTY_LOOP = 15414,
    SWITCH_STMT = 15535,
    NON_DO_LOOP = 15536,
    FE_DIAG_VEC_NOT_INNERMOST = 15553,
    EH
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
  void analyze(HLLoop *Loop, HIRDDAnalysis *DDA, HIRSafeReductionAnalysis *SRA);

  /// \brief Print the analysis result.
  void print(raw_ostream &OS, bool WithLoop = true) const;

  /// \brief Main accessor for the ParVecInfo.
  static ParVecInfo *get(AnalysisMode Mode,
                         DenseMap<HLLoop *, ParVecInfo *> &InfoMap,
                         HIRDDAnalysis *DDA, HIRSafeReductionAnalysis *SRA,
                         HLLoop *Loop) {

    auto Info = InfoMap[Loop];

    if (!Info)
      InfoMap[Loop] = Info = new ParVecInfo(Mode, Loop);
    // TODO: Query Mode and cached Mode may be different. Add code
    //       to deal with such situation.

    if (!Info->isDone())
      Info->analyze(Loop, DDA, SRA);

    return Info;
  }

  // TODO: This function doesn't handle vectorizable but not parallelizable.
  static void set(AnalysisMode Mode, DenseMap<HLLoop *, ParVecInfo *> &InfoMap,
                  HLLoop *Loop, AnalysisMode InfoMode, LoopType T,
                  DebugLoc Loc) {

    auto Info = InfoMap[Loop];

    if (!Info)
      InfoMap[Loop] = Info = new ParVecInfo(Mode, Loop);

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

class HIRParVecAnalysis : public FunctionPass {

private:
  bool Enabled;
  HIRFramework *HIRF;
  HIRDDAnalysis *DDA;
  HIRSafeReductionAnalysis *SRA;
  DenseMap<HLLoop *, ParVecInfo *> InfoMap;

public:
  HIRParVecAnalysis()
      : FunctionPass(ID), Enabled(false), HIRF(nullptr), DDA(nullptr),
        SRA(nullptr) {
    initializeHIRParVecAnalysisPass(*PassRegistry::getPassRegistry());
    InfoMap.clear();
  }

  static char ID;
  bool runOnFunction(Function &F) override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void releaseMemory() override;

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

  /// \brief Invalidate the cached result for the loop or the loop nest.
  void forget(HLLoop *Loop, bool Nest = false);

  /// \brief Invalidate the cached result for the region.
  void forget(HLRegion *Region);

  /// \brief Helper for skipping ParVec analysis on SIMD enabled functions.
  static bool isSIMDEnabledFunction(Function &F);
};

} // namespace loopopt
} // namespace llvm

#endif // INTEL_LOOPANALYSIS_PVA_H
