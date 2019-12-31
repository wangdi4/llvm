//===-------DTransFieldModRef.h - DTrans Field ModRef Analysis-------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#if !INTEL_INCLUDE_DTRANS
#error DTransFieldModRef.h include in an non-INTEL_INCLUDE_DTRANS build.
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

  // Clear all stored information.
  void reset();

  // Check whether \p Loc is known to be Mod, Ref, ModRef or NoModRef when the
  // function call is made. If Loc is a GEP that is getting the address of a
  // field, then this will check the information stored in the candidate sets to
  // try to compute the result. Otherwise, the conservative answer of ModRef is
  // returned.
  ModRefInfo getModRefInfo(const CallBase *Call, const MemoryLocation &Loc);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
  void print(raw_ostream &OS) const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
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
};

// This class performs the safety analysis to identify candidates, and save
// the results of structure/field pairs that pass the analysis into a
// FieldModRefResult object.
class DTransModRefAnalyzer {
public:
  // The function that is invoked from either the new pass manager or the legacy
  // pass manager to perform all the analysis needed.
  bool runAnalysis(Module &M, DTransAnalysisInfo &DTransInfo,
                   WholeProgramInfo &WPInfo, FieldModRefResult &Result);

private:
  void initialize(Module &M);
  void analyzeModule(Module &M);
  void analyzeFunction(Function &F);
  void populateResults(FieldModRefResult &Result);

  bool analyzeFieldForEscapes(GetElementPtrInst *GEP, llvm::StructType *Ty,
                              size_t FieldNum, dtrans::FieldInfo &FI);

  bool checkAllValuesUsingIndirectAddress(llvm::StructType *StTy,
                                          size_t FieldNum, Value *V);
  void gatherValueAliases(Value *V, bool IncludeNonPointers,
                          SmallPtrSetImpl<Value *> &Aliases);

  Value *traceToAllocation(Value *V, SmallVectorImpl<Value *> &Aliases);
  bool checkStoredValueSafe(llvm::StructType *StTy, size_t FieldNum,
                            StoreInst *SI, Value *V);

  void setAllFieldsToBottom(dtrans::StructInfo *StInfo);

  void addIndirectReader(llvm::StructType *Ty, size_t FieldNum, Function *F);
  void addIndirectWriter(llvm::StructType *Ty, size_t FieldNum, Function *F);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printCandidateInfo(StringRef Header);
  void printQueryResults(Module &M, FieldModRefResult &Result);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  DTransAnalysisInfo *DTInfo = nullptr;

  // List of structures that have fields that passed the initial safety
  // analysis.
  SmallPtrSet<llvm::StructType *, 8> Candidates;

  // For dynamically allocated array fields, keep track of the functions that
  // read/write elements of the array.
  using FunctionSet = llvm::SmallPtrSet<Function *, 2>;
  using CandFieldTy = std::pair<llvm::StructType *, size_t>;
  DenseMap<CandFieldTy, FunctionSet> IndirectFieldReaders;
  DenseMap<CandFieldTy, FunctionSet> IndirectFieldWriters;
};

// Old pass manager style pass that does the analysis, and saves the results
// into a FieldModRefResult object.
class DTransFieldModRefAnalysisWrapper : public ModulePass {
private:
  DTransModRefAnalyzer Impl;

public:
  static char ID;

  DTransFieldModRefAnalysisWrapper();

  bool runOnModule(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
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

// This class is for communicating results to LoopOpt by having an ImmutablePass
// that LoopOpt can make calls to. This may be removed in the future with use
// of an alias analysis pass, but that will require modifying all the existing
// passes to mark them as preserving the information.
class DTransFieldModRefResultWrapper : public ImmutablePass {
  FieldModRefResult Result;

public:
  static char ID;

  DTransFieldModRefResultWrapper();
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  FieldModRefResult &getResult() { return Result; }
};

class DTransFieldModRefResult
    : public AnalysisInfoMixin<DTransFieldModRefResult> {
  friend AnalysisInfoMixin<DTransFieldModRefResult>;
  static AnalysisKey Key;

public:
  typedef FieldModRefResult Result;
  Result run(Module &M, ModuleAnalysisManager &AM);
  Result run(Function &F, FunctionAnalysisManager &AM);
};

ModulePass *createDTransFieldModRefAnalysisWrapperPass();
ImmutablePass *createDTransFieldModRefResultWrapperPass();

} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSFIELDMODREF_H
