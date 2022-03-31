//===-DTransBadCastingAnalyzerOP.h - Specialized OP bad casting analyzer----===//
//
// Copyright (C) 2018-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransBadCastingAnalyzerOP.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSBADCASTINGANALYZEROP_H
#define INTEL_DTRANS_ANALYSIS_DTRANSBADCASTINGANALYZEROP_H

#include "llvm/IR/Operator.h"
#include "Intel_DTrans/Analysis/DTransAllocCollector.h"
#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include <map>


namespace llvm {

class TargetLibraryInfo;

namespace dtransOP {

class DTransBadCastingAnalyzerOP {

public:
  using GetTLIFnType =
      std::function<const TargetLibraryInfo &(const Function &)>;
  DTransBadCastingAnalyzerOP(LLVMContext &Ctx, DTransSafetyInfo &DTInfo,
                             PtrTypeAnalyzer &PTA, DTransTypeManager &TM,
                             GetTLIFnType GetTLI, const Module &M)
      : DTInfo(DTInfo), PTA(PTA), TM(TM), GetTLI(GetTLI), M(M),
        FoundViolation(false), CandidateRootType(nullptr) {
    LLVMI8Type = llvm::Type::getInt8Ty(Ctx);
    DTransI8Type = TM.getOrCreateAtomicType(LLVMI8Type);
    DTransI8PtrType = TM.getOrCreatePointerType(DTransI8Type);
  }
  DTransPointerType *getDTransI8PtrType() const { return DTransI8PtrType; }
  ~DTransBadCastingAnalyzerOP() {}
  bool analyzeBeforeVisit();
  bool analyzeLoad(dtrans::FieldInfo &FI, Instruction &I);
  bool analyzeStore(dtrans::FieldInfo &FI, Instruction &I);
  bool analyzeAfterVisit();
  bool gepiMatchesCandidateStruct(GetElementPtrInst *GEPI);
  bool gepiMatchesCandidateField(GetElementPtrInst *GEPI);
  bool isCandidateLoad(Instruction *I);
  bool isAllocStore(Instruction *I);
  void setSawBadCasting(Instruction *I);
  void setSawUnsafePointerStore(Instruction *I);
  void setSawMismatchedElementAccess(Instruction *I);

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
  DTransSafetyInfo &DTInfo;
  PtrTypeAnalyzer &PTA;
  DTransTypeManager &TM;
  GetTLIFnType GetTLI;
  llvm::Type *LLVMI8Type;
  DTransType *DTransI8Type;
  DTransPointerType *DTransI8PtrType;
  const Module &M;

  // A lattice which indicates:
  //   BCCondTop: a store is unconditionally assigned
  //   BCCondSpecial: a store is conditionally assigned
  //   BCCondBottom: we cannot tell whether a store is unconditionally
  //     or conditionally assigned.
  enum BCCondType { BCCondTop, BCCondSpecial, BCCondBottom };
  // Set to true if a bad casting or unsafe pointer store violation on
  // the candidate field has been detected, and the analysis cannot continue.
  bool FoundViolation;
  // The structure to which the candidate field belongs.
  DTransStructType *CandidateRootType;
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
  bool isGEPILastTypeCandidateRootType(GetElementPtrInst *GEPI);
  llvm::Type *findSingleGEPISourceElementType(StoreInst *STI,
                                              bool CheckPHIInputs);
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
  bool allUseBBsCondDeadOrAllocStoreDominated(Instruction *I,
                                              bool &IsCondDead);
  bool isValidZeroElementPtrAccess(LoadInst *LDI);
  StoreInst *allocStoreInst(BasicBlock *BB);
  bool condDeadOrAllocStoreDominated(BasicBlock *BB, bool &IsCondDead);
  void pruneCondLoadFunctions();
  bool violationIsConditional();
  void applySafetyCheckToCandidate(dtrans::SafetyData FindCondition,
                                   dtrans::SafetyData RemoveCondition,
                                   dtrans::SafetyData ReplaceByCondition);

public:
  void getConditionalFunctions(SetVector<Function *> &Funcs) const;
};

} // end namespace dtransOP
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSBADCASTINGANALYZEROP_H
