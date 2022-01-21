//===-----DTransBadCastingAnalyzer.h - Specialized bad casting analyzer----===//
//
// Copyright (C) 2018-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransBadCastingAnalyzer.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSBADCASTINGANALYZER_H
#define INTEL_DTRANS_ANALYSIS_DTRANSBADCASTINGANALYZER_H

#include "Intel_DTrans/Analysis/DTransAllocAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include <map>

namespace llvm {

class TargetLibraryInfo;

namespace dtrans {

class DTransBadCastingAnalyzer {

public:
  using GetTLIFnType =
      std::function<const TargetLibraryInfo &(const Function &)>;
  DTransBadCastingAnalyzer(DTransAnalysisInfo &DTInfo,
                           dtrans::DTransAllocAnalyzer &DTAA,
                           GetTLIFnType GetTLI, const Module &M)
      : DTInfo(DTInfo), DTAA(DTAA), GetTLI(GetTLI), M(M), FoundViolation(false),
        CandidateRootType(nullptr) {
    Int8PtrTy = llvm::Type::getInt8PtrTy(M.getContext());
  }
  ~DTransBadCastingAnalyzer() {}
  bool analyzeBeforeVisit();
  bool analyzeLoad(dtrans::FieldInfo &FI, Instruction &I);
  bool analyzeStore(dtrans::FieldInfo &FI, Instruction &I);
  bool analyzeAfterVisit();
  bool isBadCastTypeAndFieldCandidate(llvm::Type *SrcType, unsigned Index);
  bool isBitCastFromBadCastCandidate(BitCastOperator *I);
  bool gepiMatchesCandidate(GetElementPtrInst *GEPI);
  bool isPotentialBitCastOfAllocStore(BitCastOperator *BCI);
  void setSawBadCastBitCast(BitCastOperator *BCI);
  void setSawUnsafePointerStore(StoreInst *SI, llvm::Type *AliasType);
  void noteUnsafeCastOfAliasedPtr(BitCastOperator *I);

public:
  // Constants
  // The candidate field. This is a void* (i8* in LLVM IR) field to which
  // can be assigned pointers to various types of structures, which this
  // analysis attempts to disambiguate.
  static const unsigned CandidateVoidField = 0;

  // Argument position of functions to which the field referred to above
  // may be assigned.
  static const unsigned VoidArgumentIndex = 0;

private:
  // Accessed class objects
  DTransAnalysisInfo &DTInfo;
  dtrans::DTransAllocAnalyzer &DTAA;
  GetTLIFnType GetTLI;

  const Module &M;

  // A lattice which indicates:
  //   BCCondTop: a store is unconditionally assigned
  //   BCCondSpecial: a store is conditionally assigned
  //   BCCondBottom: we cannot tell whether a store is unconditionally
  //     or conditionally assigned.
  enum BCCondType { BCCondTop, BCCondSpecial, BCCondBottom };
  // Data structures
  // The type for a pointer to a 8-bit integer
  PointerType *Int8PtrTy;
  // Set to true if a bad casting or unsafe pointer store violation on
  // the candidate field has been detected, and the analysis cannot continue.
  bool FoundViolation;
  // The structure to which the candidate field belongs.
  llvm::StructType *CandidateRootType;
  // A map of stores to pairs of a bool and a type.  Each store is the
  // target of the return of a call to an allocation function, which is a
  // pointer value and can be assigned to the candidate field.  The bool
  // indicates whether the store was conditional. The Type is the type of
  // the pointer to the structure which is stored.
  MapVector<StoreInst *, std::pair<bool, llvm::Type *>> AllocStores;
  // A set of stores which may be eventually proved to be "alloc stores".
  // After having visited all stores, all of the pending stores need to
  // be proved to be "alloc stores", or we assume a potential violation has
  // been found.
  SmallPtrSet<StoreInst *, 10> PendingStores;
  // Bit cast operators which could be the source of bad casting.
  SmallPtrSet<BitCastOperator *, 10> BadCastOperators;
  // A map from store instructions, which could be the source of an unsafe
  // pointer store and the type that can be assigned to each store.
  std::map<StoreInst *, llvm::Type *> UnsafePtrStores;
  // Functions which must have conditionals inserted to avoid bad casting
  // on loads in those Functions.
  SetVector<Function *> CondLoadFunctions;
  // Member functions
  // For all but the most trivial, these are documented in the lines above
  // the function definitions.
  bool foundViolation() const { return FoundViolation; }
  void setFoundViolation(bool B) { FoundViolation = B; }
  std::pair<bool, llvm::Type *> findSpecificArgType(Function *F,
                                                    unsigned Index);
  llvm::Type *getLastType(GetElementPtrInst *GEPI);
  BitCastInst *findSingleBitCastAlloc(StoreInst *STI);
  std::pair<bool, llvm::Type *>
  findStoreTypeForwardCall(CallInst *CI, GetElementPtrInst *GEPI);
  llvm::Type *foundStoreType(Instruction *TI, GetElementPtrInst *GEPI);
  GetElementPtrInst *getRootGEPIFromConditional(BasicBlock *BB);
  BasicBlock *getTakenPathOfSpecialGuardConditional(BasicBlock *BB);
  BasicBlock *getNotTakenPathOfSpecialGuardConditional(BasicBlock *BB);
  bool isSpecialGuardConditional(BasicBlock *BB);
  BasicBlock *getStoreForwardAltNextBB(BasicBlock *BB, GetElementPtrInst *GEPI);
  llvm::Type *findStoreTypeBack(Instruction *TI, GetElementPtrInst *GEPI);
  std::pair<bool, llvm::Type *> findStoreTypeForward(Instruction *TI,
                                                     GetElementPtrInst *GEPI);
  BCCondType isConditionalBlock(BasicBlock *SBB);
  void recordAllocStore(StoreInst *SI, llvm::Type *StType);
  std::pair<bool, llvm::Type *> findStoreType(Instruction *TI,
                                              GetElementPtrInst *GEPI);
  void handlePotentialAllocStore(StoreInst *SI);
  bool isInnocuousLoadOfCall(CallInst *CI, LoadInst *LI,
                             GetElementPtrInst *GEPI);
  bool allUseBBsConditionallyDead(Instruction *I);
  void pruneCondLoadFunctions();
  void processPotentialBitCastsOfAllocStores();
  void processPotentialUnsafePointerStores();
  bool violationIsConditional();
  void applySafetyCheckToCandidate(dtrans::SafetyData FindCondition,
                                   dtrans::SafetyData RemoveCondition,
                                   dtrans::SafetyData ReplaceByCondition);

public:
  void getConditionalFunctions(SetVector<Function *> &Funcs) const;
};

} // end namespace dtrans
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSBADCASTINGANALYZER_H
