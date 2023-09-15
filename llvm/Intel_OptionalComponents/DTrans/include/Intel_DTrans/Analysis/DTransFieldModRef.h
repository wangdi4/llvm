//===-------DTransFieldModRef.h - DTrans Field ModRef Analysis-------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransFieldModRef.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSFIELDMODREF_H
#define INTEL_DTRANS_ANALYSIS_DTRANSFIELDMODREF_H

#include "Intel_DTrans/Analysis/DTrans.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

class Function;
class GetElementPtrInst;
class Module;
class StoreInst;
class WholeProgramInfo;

namespace dtransOP {
class DTransSafetyInfo;
} // end namespace dtransOP

// This class captures the final results of the field ModRef analysis that can
// be used to check whether a function reads/writes a field within some
// structure.
//
// In the future, this may be changed to derive from AAResultBase to be
// used as an AliasAnalysis pass. For expediency to use this with LoopOpt, this
// is being used as a standalone result by an immutable pass that LoopOpt will
// interface with.
class FieldModRefResult {
public:
  void addReader(llvm::StructType *Ty, size_t FieldNum, Function *F);
  void addWriter(llvm::StructType *Ty, size_t FieldNum, Function *F);
  bool isReader(llvm::StructType *Ty, size_t FieldNum, Function *F);
  bool isWriter(llvm::StructType *Ty, size_t FieldNum, Function *F);

  void addValueReader(llvm::StructType *Ty, size_t FieldNum, Function *F);
  void addValueWriter(llvm::StructType *Ty, size_t FieldNum, Function *F);
  bool isValueReader(llvm::StructType *Ty, size_t FieldNum, Function *F);
  bool isValueWriter(llvm::StructType *Ty, size_t FieldNum, Function *F);

  bool isCandidate(llvm::StructType *Ty, size_t FieldNum);

  // Add an element to the FunctionToCalleeSet mapping to indicate that \p
  // Callee may be reachable from \p F because the address of \p Callee is
  // passed as a parameter to \p F.
  void addFunctionParamBasedCallee(Function *F, Function *Callee);

  // Clear all stored information.
  void reset();

  // Check whether \p Loc is known to be Mod, Ref, ModRef or NoModRef when the
  // function call is made. If Loc is a GEP that is getting the address of a
  // field, then this will check the information stored in the candidate sets to
  // try to compute the result. Otherwise, the conservative answer of ModRef is
  // returned.
  ModRefInfo getModRefInfo(const CallBase *Call, const MemoryLocation &Loc);

  /// Results cannot be invalidated.
  bool invalidate(Module &, const PreservedAnalyses &,
                  ModuleAnalysisManager::Invalidator &) {
    return false;
  }

  /// Results cannot be invalidated.
  bool invalidate(Function &, const PreservedAnalyses &,
                  FunctionAnalysisManager::Invalidator &) {
    return false;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
  void print(raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  void unionModRefInfo(ModRefInfo &Status, const CallBase *Call,
                       llvm::StructType *StTy, unsigned FieldNum, bool Indirect,
                       SmallPtrSetImpl<Function *> &Visited,
                       unsigned Indent = 0);

  void unionModRefInfo(ModRefInfo &Status, Function *F, llvm::StructType *StTy,
                       unsigned FNum, bool Indirect,
                       SmallPtrSetImpl<Function *> &Visited,
                       unsigned Indent = 0);

  // This will track sets of functions that directly modify/reference the field
  // values of structures. If the field is a dynamically allocation memory
  // buffer, it will also track which functions read/write the elements of the
  // array.
  using FunctionSet = llvm::SmallPtrSet<Function *, 2>;
  struct FieldModRefCandidateInfo {
    // Note: These sets only contain the direct accessors. Not the closure that
    // includes information about called functions.
    FunctionSet FieldReaders; /* Readers of the structure field address */
    FunctionSet FieldWriters; /* Writers of the structure field address */
    FunctionSet ValueReaders; /* Readers of values stored within a dynamic array
                                 pointed to by the field */
    FunctionSet ValueWriters; /* Writes of values stored within a dynamic array
                                 pointed to by the field*/
  };

  // StructType:Field number pair.
  using CandFieldTy = std::pair<llvm::StructType *, size_t>;
  using CandidateTy = DenseMap<CandFieldTy, FieldModRefCandidateInfo>;
  CandidateTy Candidates;

  // Some function calls may be made based on the address of a function
  // passed as an input parameter. This will map a Function to the set of
  // Functions that may be called based on input parameters that correspond
  // to an address taken function. This is used to allow some address taken
  // functions to be present when identifying all the functions that are
  // reachable from some function.
  //
  // For example:
  //   call void @foo(void (%struct.info*)* @bar, %struct.info* %info)
  //
  //   define @foo(void (%struct.info*)* nocapture %1, %struct.info* %2) {
  //     call void %1(%struct.info* %2)
  //     ...
  //   }
  //
  // @bar is an address taken function because the address is passed to @foo.
  // However, because the address is nocapture and just used to make a function
  // call from @foo, @bar should be considered as reachable from @foo. If all
  // the address taken uses of @bar are for uses such as this, it will not be
  // necessary to disqualify the structure fields used within @bar. When
  // looking at the calls made by @foo during getModRefInfo, the information
  // about @bar will also be considered, even though it is an address taken
  // call, because it will be contained within this mapping.
  DenseMap<Function *, SmallPtrSet<Function *, 4>> FunctionToCalleeSet;
};

// This class performs the safety analysis to identify candidates, and save
// the results of structure/field pairs that pass the analysis into a
// FieldModRefResult object.
class DTransModRefAnalyzer {
public:
  // The function that is invoked from either the new pass manager or the legacy
  // pass manager to perform all the analysis needed based on results collected
  // by the DTransAnalysis pass.
  bool runAnalysis(Module &M, DTransAnalysisInfo &DTransInfo,
                   WholeProgramInfo &WPInfo, FieldModRefResult &Result);

  // The function that is invoked from either the new pass manager or the legacy
  // pass manager to perform all the analysis needed based on results collected
  // by the DTransSafetyAnalyzer pass.
  bool runAnalysis(Module &M, dtransOP::DTransSafetyInfo &DTransInfo,
                   WholeProgramInfo &WPInfo, FieldModRefResult &Result);
};

// New pass manager style pass.
class DTransFieldModRefAnalysis
    : public AnalysisInfoMixin<DTransFieldModRefAnalysis> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<DTransFieldModRefAnalysis>;
  static char PassID;

public:
  typedef FieldModRefResult Result;
  Result run(Module &M, ModuleAnalysisManager &AM);
};

// New pass manager style pass that does the field mod/ref analysis with support
// for IR containing opaque pointer types, and saves the results into a
// FieldModRefResult object.
class DTransFieldModRefOPAnalysis
    : public AnalysisInfoMixin<DTransFieldModRefOPAnalysis> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<DTransFieldModRefOPAnalysis>;
  static char PassID;

public:
  typedef FieldModRefResult Result;
  Result run(Module &M, ModuleAnalysisManager &AM);
};

class DTransFieldModRefResult
    : public AnalysisInfoMixin<DTransFieldModRefResult> {
  friend AnalysisInfoMixin<DTransFieldModRefResult>;
  static AnalysisKey Key;

public:
  typedef FieldModRefResult Result;
  Result run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSFIELDMODREF_H
