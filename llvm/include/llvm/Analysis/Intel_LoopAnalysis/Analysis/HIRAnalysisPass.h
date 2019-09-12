//===------- HIRAnalysisPass.h - Base class for HIR analyses -*- C++ -*----===//
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
// This header file declares the base class for HIR analyis passes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRANALYSISPASS_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRANALYSISPASS_H

#include "llvm/Pass.h"

#include "llvm/Support/Casting.h"

namespace llvm {

class formatted_raw_ostream;

namespace loopopt {

class HIRFramework;
class HLRegion;
class HLLoop;

/// \brief - All HIR analysis passes should derive from this class.
///
/// Pass setup requirements (see HIRDDAnalysis.cpp for ref)-
///
/// - Define under Intel_LoopAnalysis/Analysiss directory.
/// - Use the INITIALIZE_PASS* macros for initialization.
/// - Declare initialize<PassName>Pass() in llvm/InitializePasses.h and add a
///   call in Intel_LoopAnalysis/Intel_LoopAnalysis.cpp.
/// - Declare create<PassName>Pass() in Intel_LoopAnalysis/Passes.h, define
///   it in your file and add a call in llvm/LinkAllPasses.h (so it is not
///   optimized away) and PassManagerBuilder.cpp (to add it to clang opt
///   pipeline).
/// - Define pass under loopopt namespace.
/// - Declare HIRFramework pass as required to access HIR.
/// - Always call setPreservesAll() in getAnalysisUsage().
/// - Add a new value for the pass to HIRAnalysisVal enum before
///   HIRPassCountVal.
/// Pass this value in the constructor.
/// - Add function calls for the pass to
/// HIRInvalidationUtils::invalidateLoopBodyAnalysis(),
/// HIRInvalidationUtils::invalidateLoopBoundsAnalysis() and
/// HIRInvalidationUtils::invalidateTopLevelNodeAnalysis() in
/// HIRInvalidationUtils.h.
class HIRAnalysisBase {
protected:
  /// Used to print derived classes's results.
  struct PrintVisitor;

  /// Invoked by main print() function to print analysis results for region.
  /// This is intentionally non-const as on-demand analyses have to compute
  /// results for printing.
  virtual void print(formatted_raw_ostream &OS, const HLRegion *Reg) {}

  /// Invoked by main print() function to print analysis results for loop.
  /// This is intentionally non-const as on-demand analyses have to compute
  /// results for printing.
  virtual void print(formatted_raw_ostream &OS, const HLLoop *Lp) {}

public:
  virtual ~HIRAnalysisBase() {}
  virtual void printAnalysis(raw_ostream &OS) const = 0;

  /// Interface for derived classes to invalidate analysis for
  /// regions/loops/nodes.

  /// This method informs the analysis that the loop body has been
  /// modified. Most analysis would want to implement this function to
  /// invalidate results.
  virtual void markLoopBodyModified(const HLLoop *Lp) = 0;

  /// This method informs the analysis that the loop bounds has been
  /// modified. Default implementation is empty since most analysis would not
  /// care about changes to loop bounds alone.
  virtual void markLoopBoundsModified(const HLLoop *Lp) {}

  /// This methods informs the analysis that one or more nodes which lie
  /// outside any loop in the region have been modified. Default implementation
  /// is empty since most analysis would not care about changes to nodes
  /// outside loops.
  virtual void markNonLoopRegionModified(const HLRegion *Reg) {}
};

class HIRAnalysis : public HIRAnalysisBase {
protected:
  HIRFramework &HIRF;

public:
  HIRAnalysis(HIRFramework &HIRF) : HIRF(HIRF) {}
  HIRAnalysis(HIRAnalysis &&Arg) : HIRF(Arg.HIRF) {}
  HIRAnalysis(const HIRAnalysis &) = delete;
  virtual ~HIRAnalysis() {}

  void printAnalysis(raw_ostream &OS) const override;
};

// TODO: Remove the HIRAnalysisPass after we change all HIR analysis passes to
// support new pass manager.
class HIRAnalysisPass : public FunctionPass, public HIRAnalysisBase {
public:
  /// \brief An enumeration to keep track of the subclasses.
  enum HIRAnalysisVal {
    HIRDDAnalysisVal,
    HIRLocalityAnalysisVal,
    HIRLoopResourceVal,
    HIRLoopStatisticsVal,
    HIRSafeReductionAnalysisVal,
    HIRSparseArrayReductionAnalysisVal,
    // Should be kept last
    HIRPassCountVal
  };

private:
  /// ID to differentiate between concrete subclasses.
  const HIRAnalysisVal SubClassID;

protected:
  HIRAnalysisPass(char &ID, HIRAnalysisVal SCID)
      : FunctionPass(ID), SubClassID(SCID) {}

public:
  /// \brief Return an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't
  /// be used for any other purpose.
  HIRAnalysisVal getHIRAnalysisID() const { return SubClassID; }

  /// Prints analysis's results in 'opt -analyze' mode. This is a lightweight
  /// print which prints region's/loop's header/footer along with their analysis
  /// results.
  void printAnalysis(raw_ostream &OS) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override {
    printAnalysis(OS);
  }
};


template <typename T> using ProviderFunctionTy = std::function<T *(void)>;

template <typename... AnalysisTys> class HIRAnalysisProviderBase;

template <> class HIRAnalysisProviderBase<> {
public:
  template <typename T> T *get() {
    static_assert(!std::is_same<T, T>::value,
                  "Requested analysis is not registered in the provider");
    return nullptr;
  }

  template <typename... Except> struct Invoke {
    Invoke(HIRAnalysisProviderBase<> &Provider) {}
    template <typename F, typename... ArgsTy>
    void operator()(F &&Func, ArgsTy... Args) {}
  };

  template <typename... Except> Invoke<Except...> invoke() {
    return Invoke<Except...>(*this);
  }
};

template <typename AnalysisTy, typename... AnalysisTys>
class HIRAnalysisProviderBase<AnalysisTy, AnalysisTys...>
    : public HIRAnalysisProviderBase<AnalysisTys...> {
  typedef HIRAnalysisProviderBase<AnalysisTys...> Base;

  ProviderFunctionTy<AnalysisTy> AnalysisFunc;

public:
  HIRAnalysisProviderBase(ProviderFunctionTy<AnalysisTy> F,
                          ProviderFunctionTy<AnalysisTys>... Fs)
      : Base(Fs...), AnalysisFunc(F) {}

  template <typename T>
  typename std::enable_if<std::is_same<T, AnalysisTy>::value, T>::type *get() {
    return AnalysisFunc();
  }

  template <typename T>
  typename std::enable_if<!std::is_same<T, AnalysisTy>::value, T>::type *get() {
    return static_cast<Base &>(*this).template get<T>();
  }

  template <typename... Except> class Invoke {
    typedef HIRAnalysisProviderBase<AnalysisTy, AnalysisTys...> ProviderTy;
    ProviderTy &Provider;

  public:
    Invoke(ProviderTy &Provider) : Provider(Provider) {}

    template <typename F, typename... ArgsTy>
    void operator()(F &&Func, ArgsTy... Args) {
      AnalysisTy *AnalysisPtr = !is_one_of<AnalysisTy, Except...>::value
                                    ? Provider.get<AnalysisTy>()
                                    : nullptr;
      if (AnalysisPtr) {
        ((AnalysisPtr)->*Func)(std::forward<ArgsTy>(Args)...);
      }

      static_cast<Base &>(Provider).template invoke<Except...>()(Func, Args...);
    }
  };

  template <typename... Except> Invoke<Except...> invoke() {
    return Invoke<Except...>(*this);
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
