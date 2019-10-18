//===--- HIRFramework.h - public interface of HIR framework ---*-- C++ --*-===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis is the public interface for the HIR framework. HIR
// transformations should add it as a dependency.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRFRAMEWORK_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRFRAMEWORK_H

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRAnalysisPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

class DominatorTree;
class PostDominatorTree;
class LoopInfo;
class ScalarEvolution;
class AAResults;

namespace loopopt {

class HIRCreation;
class HIRCleanup;
class HIRLoopFormation;
class HIRScalarSymbaseAssignment;
class HIRParser;
class HIRRegionIdentification;
class HIRSCCFormation;

const unsigned InvalidBlobIndex = 0;

/// Invalid symbase.
const unsigned InvalidSymbase = 0;

/// Symbase for constants.
const unsigned ConstantSymbase = 1;

/// Symbase assigned to non-constant rvals which do not create data
/// dependencies.
const unsigned GenericRvalSymbase = 2;

class HIRDDAnalysis;
class HIRLoopLocality;
class HIRLoopResource;
class HIRLoopStatistics;
class HIRParVecAnalysis;
class HIRSafeReductionAnalysis;
class HIRSparseArrayReductionAnalysis;

typedef HIRAnalysisProviderBase<HIRDDAnalysis, HIRLoopLocality, HIRLoopResource,
                                HIRLoopStatistics, HIRParVecAnalysis,
                                HIRSafeReductionAnalysis,
                                HIRSparseArrayReductionAnalysis>
    HIRAnalysisProvider;

/// This analysis is the public interface for the HIR framework.
///
/// The overall sequence of building the HIR is as follows-
///
/// 1) HIRRegionIdentification - identifies regions in IR.
/// 2) HIRSCCFormation - identifies SCCs in regions.
/// 3) HIRSSADeconstruction - deconstructs SSA for HIR by inserting copies.
/// 4) HIRCreation - populates HIR regions with a sequence of HLNodes (without
///    HIR loops).
/// 5) HIRCleanup - removes redundant gotos/labels from HIR.
/// 6) HIRLoopFormation - Forms HIR loops within HIR regions.
/// 7) HIRScalarSymbaseAssignment - Assigns symbases to livein/liveout values.
/// 8) HIRParser - Creates DDRefs and parses SCEVs into CanonExprs. Also assigns
///    symbases to non livein/liveout scalars using ScalarSymbaseAssignment's
///    interface.
/// 9) HIRSymbaseAssignment - Assigns symbases to memory DDRefs.
///
class HIRFramework {
public:
  /// Iterators to iterate over regions
  typedef HLContainerTy::iterator iterator;
  typedef HLContainerTy::const_iterator const_iterator;
  typedef HLContainerTy::reverse_iterator reverse_iterator;
  typedef HLContainerTy::const_reverse_iterator const_reverse_iterator;

private:
  /// The function we are operating on.
  Function &Func;

  DominatorTree &DT;
  PostDominatorTree &PDT;
  LoopInfo &LI;
  ScalarEvolution &SE;
  AAResults &AA;
  HIRRegionIdentification &RI;
  HIRSCCFormation &SCCF;
  LoopOptReportBuilder LORBuilder;

  /// HLNodeUtils object for the framework.
  std::unique_ptr<HLNodeUtils, HLNodeUtils::HLNodeUtilsDeleter> HNU;

  HIRAnalysisProvider AnalysisProvider;

  /// Regions - HLRegions formed out of incoming LLVM IR.
  HLContainerTy Regions;

  /// Scalar symbase assignment facility.
  std::unique_ptr<HIRCreation> PhaseCreation;
  std::unique_ptr<HIRCleanup> PhaseCleanup;
  std::unique_ptr<HIRLoopFormation> PhaseLoopFormation;
  std::unique_ptr<HIRScalarSymbaseAssignment> PhaseScalarSA;
  std::unique_ptr<HIRParser> PhaseParser;

  // Latest requested symbase.
  unsigned MaxSymbase;

  struct MaxTripCountEstimator;
  void estimateMaxTripCounts();

  void runImpl();

public:
  HIRFramework(Function &F, DominatorTree &DT, PostDominatorTree &PDT,
               LoopInfo &LI, ScalarEvolution &SE, AAResults &AA,
               HIRRegionIdentification &RI, HIRSCCFormation &SCCF,
               OptReportVerbosity::Level VerbosityLevel,
               HIRAnalysisProvider AnalysisProvider);
  HIRFramework(const HIRFramework &) = delete;
  HIRFramework(HIRFramework &&);
  ~HIRFramework();

  void print(raw_ostream &OS) const;
  void print(bool FrameworkDetails, raw_ostream &OS) const;

  void verify() const;

  // The biggest symbase seen during compilation of this fuction
  unsigned getMaxSymbase() const { return MaxSymbase; }

  // Returns a new unused symbase ID.
  unsigned getNewSymbase() { return ++MaxSymbase; }

  // Returns the max symbase assigned to any scalar.
  unsigned getMaxScalarSymbase() const;

  /// Returns HLNodeUtils object.
  HLNodeUtils &getHLNodeUtils() { return *HNU; }
  const HLNodeUtils &getHLNodeUtils() const { return *HNU; }

  /// Returns DDRefUtils object.
  DDRefUtils &getDDRefUtils() const;

  /// Returns CanonExprUtils object.
  CanonExprUtils &getCanonExprUtils() const;

  /// Returns BlobUtils object.
  BlobUtils &getBlobUtils() const;

  /// Region iterator methods
  iterator hir_begin() { return Regions.begin(); }
  const_iterator hir_begin() const { return Regions.begin(); }
  iterator hir_end() { return Regions.end(); }
  const_iterator hir_end() const { return Regions.end(); }

  reverse_iterator hir_rbegin() { return Regions.rbegin(); }
  const_reverse_iterator hir_rbegin() const { return Regions.rbegin(); }
  reverse_iterator hir_rend() { return Regions.rend(); }
  const_reverse_iterator hir_rend() const { return Regions.rend(); }

  /// Returns true if \p HInst is a livein copy.
  bool isLiveinCopy(const HLInst *HInst);

  /// Returns true if \p HInst is a liveout copy.
  bool isLiveoutCopy(const HLInst *HInst);

  /// Returns Function object.
  Function &getFunction() const { return Func; }

  /// Returns Module object.
  Module &getModule() const { return *getFunction().getParent(); }

  /// Returns LLVMContext object.
  LLVMContext &getContext() const { return getFunction().getContext(); }

  /// Returns DataLayout object.
  const DataLayout &getDataLayout() const {
    return getModule().getDataLayout();
  }

  /// Returns LORBuilder
  LoopOptReportBuilder &getLORBuilder() { return LORBuilder; }

  HIRAnalysisProvider &getHIRAnalysisProvider() { return AnalysisProvider; }
};

class HIRFrameworkAnalysis : public AnalysisInfoMixin<HIRFrameworkAnalysis> {
  friend struct AnalysisInfoMixin<HIRFrameworkAnalysis>;

  static AnalysisKey Key;

public:
  using Result = HIRFramework;

  HIRFramework run(Function &F, FunctionAnalysisManager &AM);
};

class HIRFrameworkPrinterPass : public PassInfoMixin<HIRFrameworkPrinterPass> {
  raw_ostream &OS;
  bool PrintDetails;

public:
  HIRFrameworkPrinterPass(raw_ostream &OS, bool PrintDetails)
      : OS(OS), PrintDetails(PrintDetails) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    if (llvm::isFunctionInPrintList(F.getName())) {
      OS << "Function: " << F.getName() << "\n";

      AM.getResult<HIRFrameworkAnalysis>(F).print(PrintDetails, OS);
    }

    return PreservedAnalyses::all();
  }
};

class HIRFrameworkVerifierPass
    : public PassInfoMixin<HIRFrameworkVerifierPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    AM.getResult<HIRFrameworkAnalysis>(F).verify();
    return PreservedAnalyses::all();
  }
};

class HIRFrameworkWrapperPass : public FunctionPass {
  std::unique_ptr<HIRFramework> HIRF;

public:
  static char ID; // Pass identification

  HIRFrameworkWrapperPass();

  HIRFramework &getHIR() { return *HIRF; }
  const HIRFramework &getHIR() const { return *HIRF; }

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  void print(raw_ostream &OS, const Module * = nullptr) const override {
    HIRF->print(OS);
  }

  void verifyAnalysis() const override { HIRF->verify(); }

  void releaseMemory() override { HIRF.reset(); }
};

} // End namespace loopopt

} // End namespace llvm

#endif
