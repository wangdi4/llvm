//===---------------- DynClone.cpp - DTransDynClonePass -------------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Dynamic Cloning optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DynClone.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
using namespace llvm;

#define DEBUG_TYPE "dtrans-dynclone"

namespace {

class DTransDynCloneWrapper : public ModulePass {
private:
  dtrans::DynClonePass Impl;

public:
  static char ID;

  DTransDynCloneWrapper() : ModulePass(ID) {
    initializeDTransDynCloneWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    DTransAnalysisInfo &DTInfo =
        getAnalysis<DTransAnalysisWrapper>().getDTransInfo();

    dtrans::LoopInfoFuncType GetLI = [this](Function &F) -> LoopInfo & {
      return this->getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    };

    return Impl.runImpl(
        M, DTInfo, getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(),
        getAnalysis<WholeProgramWrapperPass>().getResult(), GetLI);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransDynCloneWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransDynCloneWrapper, "dtrans-dynclone",
                      "DTrans dynamic cloning", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransDynCloneWrapper, "dtrans-dynclone",
                    "DTrans dynamic cloning", false, false)

ModulePass *llvm::createDTransDynCloneWrapperPass() {
  return new DTransDynCloneWrapper();
}

namespace llvm {

namespace dtrans {

// This class is used to reorder fields after shrinking fields to
// eliminate gaps due to shrinking.
class FieldData {
public:
  uint64_t Align;
  uint64_t Size;
  unsigned Index;

  FieldData(uint64_t Align, uint64_t Size, unsigned Index)
      : Align(Align), Size(Size), Index(Index) {}

  // Sort fields based on below compare function to avoid gaps between fields.
  //    Ascending order based on Alignment, then
  //    Ascending order based on Size, then
  //    Descending order based on Index.
  bool operator<(const FieldData &RHS) const {
    return std::make_tuple(RHS.Align, RHS.Size, -(RHS.Index + 1)) <
           std::make_tuple(Align, Size, -(Index + 1));
  }
};

class DynCloneImpl {
  using DynField = std::pair<llvm::Type *, size_t>;
  using DynFieldList = SmallVector<DynField, 16>;
  // Even though it will be very small set,  using std::set (instead of
  // SmallSet/SetVector) to do set operations like union/intersect.
  using DynFieldSet = std::set<DynField>;
  using FunctionSet = SmallPtrSet<Function *, 16>;
  using CallInstSet = SmallPtrSet<CallInst *, 8>;

public:
  DynCloneImpl(Module &M, const DataLayout &DL, DTransAnalysisInfo &DTInfo,
               LoopInfoFuncType &GetLI)
      : M(M), DL(DL), DTInfo(DTInfo), GetLI(GetLI),
        ShrunkenIntTy(Type::getInt32Ty(M.getContext())), InitRoutine(nullptr),
        ShrinkHappenedVar(nullptr){};
  bool run(void);

private:
  Module &M;
  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;
  LoopInfoFuncType &GetLI;

  // Holds result Type after shrinking 64-bit to 32-bit integer values.
  llvm::Type *ShrunkenIntTy;

  // List of candidate fields which are represented as <Type of struct,
  // field_index>.
  DynFieldList CandidateFields;

  // This is used to maintain mapping between candidate field and set of
  // other fields that are assigned to the candidate field.
  DenseMap<DynField, DynFieldSet> DependentFieldSet;

  // Mapping between candidate field and the function where unknown values
  // are assigned to the candidate field.
  DenseMap<DynField, Function *> DynFieldFuncMap;

  // Function that has unknown assignments to all qualified candidate fields.
  Function *InitRoutine;

  // It is used to check at runtime if shrinking of structs occurred.
  GlobalVariable *ShrinkHappenedVar;

  // List of cloned routines.
  FunctionSet ClonedFunctionList;

  // List of allocation calls for candidate structs in InitRoutine.
  SmallVector<std::pair<AllocCallInfo *, Type *>, 4> AllocCalls;

  // Map of original and cloned routines.
  DenseMap<Function *, Function *> CloningMap;

  // Map of <Original struct, Vector of new indexes to indicate new
  // locations in shrunken struct>
  DenseMap<StructType *, std::vector<uint32_t>> TransformedIndexes;

  // Map of <Original Struct, Shrunken Struct>
  DenseMap<StructType *, StructType *> TransformedTypeMap;

  // For each candidate type, it collects all routines where the type is
  // accessed. This set is used to decide which routines need to be
  // cloned during transformation.
  DenseMap<llvm::Type *, FunctionSet> TypeAccessedInFunctions;

  // List of CallSites for AddressTaken routines.
  DenseMap<Function *, CallInstSet> FunctionCallInsts;

  bool gatherPossibleCandidateFields(void);
  bool prunePossibleCandidateFields(void);
  bool verifyLegalityChecksForInitRoutine(void);
  void createShrunkenTypes(void);
  void transformInitRoutine(void);
  bool createCallGraphClone(void);
  bool isCandidateField(DynField &DField) const;
  bool isCandidateStruct(Type *Ty);
  Type *getGEPStructType(GetElementPtrInst *GEP) const;
  Type *getCallInfoElemTy(CallInfo *CInfo) const;
  void printCandidateFields(raw_ostream &OS) const;
  void printDynField(raw_ostream &OS, const DynField &DField) const;
};

// Collects possible candidate fields for Dynamic cloning.
bool DynCloneImpl::gatherPossibleCandidateFields(void) {

  // Allow only Int64 type fields as candidates for now.
  auto IsCandidateType = [&](Type *Ty) { return Ty->isIntegerTy(64); };

  LLVM_DEBUG(dbgs() << "  Looking for candidate structures.\n");

  for (TypeInfo *TI : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<StructInfo>(TI);
    if (!StInfo)
      continue;

    if (DTInfo.testSafetyData(TI, dtrans::DT_DynClone)) {
      LLVM_DEBUG(dbgs() << "    Rejecting "
                        << getStructName(StInfo->getLLVMType())
                        << " based on safety data.\n");
      continue;
    }
    StructType *StTy = cast<StructType>(StInfo->getLLVMType());
    for (unsigned I = 0; I < StTy->getNumElements(); ++I) {
      Type *Ty = StTy->getElementType(I);
      if (!IsCandidateType(Ty))
        continue;
      CandidateFields.push_back(std::make_pair(StTy, I));
    }
  }

  LLVM_DEBUG(if (CandidateFields.empty()) dbgs()
             << "    No possible candidates found.\n");

  return !CandidateFields.empty();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DynCloneImpl::printDynField(raw_ostream &OS,
                                 const DynField &DField) const {
  OS << "struct: " << getStructName(DField.first) << " Index: " << DField.second
     << "\n";
}

// Print candidate fields
void DynCloneImpl::printCandidateFields(raw_ostream &OS) const {
  for (auto &CandidatePair : CandidateFields) {
    OS << "    ";
    printDynField(OS, CandidatePair);
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Returns true if \p DField is in CandidateFields.
bool DynCloneImpl::isCandidateField(DynField &DField) const {
  for (auto &CandidatePair : CandidateFields)
    if (CandidatePair == DField)
      return true;
  return false;
}

// Returns true if \p Ty is a struct that has candidate fields for
// shrinking.
bool DynCloneImpl::isCandidateStruct(Type *Ty) {
  if (!Ty->isStructTy())
    return false;
  for (auto &CPair : CandidateFields)
    if (CPair.first == Ty)
      return true;
  return false;
}

// Returns Type of struct accessed in GEP if the GEP is in allowed format
// to enable DynClone for the struct. Otherwise, returns nullptr.
Type *DynCloneImpl::getGEPStructType(GetElementPtrInst *GEP) const {
  int32_t NumIndices = GEP->getNumIndices();
  if (NumIndices > 2)
    return nullptr;
  if (NumIndices == 1) {
    auto FPair = DTInfo.getByteFlattenedGEPElement(GEP);
    return FPair.first;
  }
  auto ElemTy = GEP->getSourceElementType();
  if (!isa<StructType>(ElemTy))
    return nullptr;
  return ElemTy;
}

// Returns struct type associated with \p CInfo of either Alloc/Memfunc.
Type *DynCloneImpl::getCallInfoElemTy(CallInfo *CInfo) const {
  if (!CInfo)
    return nullptr;
  if (CInfo->getCallInfoKind() != CallInfo::CIK_Alloc &&
      CInfo->getCallInfoKind() != CallInfo::CIK_Memfunc)
    return nullptr;
  auto &CallTypes = CInfo->getPointerTypeInfoRef().getTypes();
  if (CallTypes.size() != 1)
    return nullptr;
  Type *PtrTy = *CallTypes.begin();
  if (!PtrTy->isPointerTy())
    return nullptr;
  Type *ElemTy = PtrTy->getPointerElementType();
  if (!isa<StructType>(ElemTy))
    return nullptr;
  return ElemTy;
}

// This routine tries to reduce possible candidate fields by analyzing IR
// and also finds InitRoutine, where unknown values are assigned to the
// all candidate fields. A candidate field is eliminated if we can't prove
// that value of the field always fits in 32-bit.
// Analysis does look at memset and store instructions.
//
// memset: For now, only zero value is considered as safe. For all other
// values, fields in structure are not qualified for DynClone transformation.
//
// Store Inst:
//   If Store operand is
//     1. Constant Value: If it doesn't fit in 32-bit, it will be not be
//        qualified.
//
//     2. Load Value:
//           a. Same Field: No issues.
//           b. Some other Candidate field: Will add to dependence set and
//              verify it later.
//           c. Non-Candidate field: Not qualified.
//
//     TODO: Support for Args will be added later.
//
//     3. Other (Unknown value): Map between field and the function where it
//        is assigned is recorded. All unknown assignments should be in the
//        same routine (i.e InitRoutine).
//
//  Dependence Set Analysis: Disable DynClone transformation conservatively
//  if dependent field of any candidate field is not qualified.
//
//  Init routine detection: All qualified candidates should have unknown
//  assignments in the same routine.
//
bool DynCloneImpl::prunePossibleCandidateFields(void) {

  DynFieldSet InvalidFields;

  // Analyze constant assignments:  str->field = 200;
  // Field will be added to invalid fields if constant doesn't fit
  // in 32-bits.
  auto CheckConstInt = [&](Value *V, DynField &StElem) {
    assert(isa<ConstantInt>(V) && "Expected ConstantInt");
    auto *CInt = cast<ConstantInt>(V);

    if (!ConstantInt::isValueValidForType(ShrunkenIntTy,
                                          CInt->getValue().getLimitedValue())) {
      InvalidFields.insert(StElem);
      LLVM_DEBUG(dbgs() << "    Large constant... Added to Invalid Field:";
                 printDynField(dbgs(), StElem));
    }
  };

  // Analyze assignment of Load value:  str1->fielda = str2->fieldb
  // 1. fielda and fieldb are same: No Issue.
  // 2. fieldb is candidate field: Add fieldb as dependent for fielda
  // 3. fieldb is non-candidate: Add fielda to invalid fields.
  auto CheckLoadValue = [&](Value *V, DynField &StElem, Function *F) {
    assert(isa<LoadInst>(V) && "Expected LoadInst");
    LoadInst *LI = cast<LoadInst>(V);

    auto LdElem = DTInfo.getLoadElement(LI);
    if (StElem == LdElem)
      return;
    if (isCandidateField(LdElem)) {
      DependentFieldSet[StElem].insert(LdElem);
      LLVM_DEBUG(dbgs() << "    Adding to dependence-set:\n";
                 dbgs() << "        "; printDynField(dbgs(), StElem);
                 dbgs() << "        "; printDynField(dbgs(), LdElem));
      return;
    }
    auto It = DynFieldFuncMap.find(StElem);
    if (It == DynFieldFuncMap.end()) {
      DynFieldFuncMap.insert({StElem, F});
    } else if (It->second != F) {
      InvalidFields.insert(StElem);
      LLVM_DEBUG(dbgs() << "    Invalid...More than one init routine:";
                 printDynField(dbgs(), StElem));
    }
  };

  // Analyze memset here: memset(ptr2str, 0, size)
  //   Any call/ memset with zero value: No issues
  //
  //   memset with non-zero value: If there are any candidate fields in the
  //   struct, make them as invalid fields.
  auto WrittenUnknownValue = [&](Instruction *I) {
    auto *CInfo = DTInfo.getCallInfo(I);
    if (!CInfo || CInfo->getCallInfoKind() != dtrans::CallInfo::CIK_Memfunc)
      return;
    if (cast<MemfuncCallInfo>(CInfo)->getMemfuncCallInfoKind() !=
        MemfuncCallInfo::MK_Memset)
      return;
    MemSetInst &Inst = cast<MemSetInst>(*I);
    if (isValueEqualToSize(Inst.getValue(), 0))
      return;

    auto &CallTypes = CInfo->getPointerTypeInfoRef().getTypes();
    if (CallTypes.size() != 1)
      return;
    Type *PtrTy = *CallTypes.begin();
    if (!PtrTy->isPointerTy())
      return;
    Type *ElemTy = PtrTy->getPointerElementType();
    for (auto &CandidatePair : CandidateFields) {
      if (CandidatePair.first == ElemTy) {
        InvalidFields.insert(CandidatePair);
        LLVM_DEBUG(dbgs() << "    Invalid...written by memset: ";
                   printDynField(dbgs(), CandidatePair));
      }
    }
  };

  // If \p I is accessing a candidate struct, add \p F to the list
  // of routines where the candidate struct is accessed.
  auto UpdateTypeAccessInfo = [&](Instruction *I, Function *F) {
    Type *StTy = nullptr;
    if (auto *GEP = dyn_cast<GetElementPtrInst>(I))
      StTy = getGEPStructType(GEP);

    if (!StTy)
      return;
    if (isCandidateStruct(StTy))
      TypeAccessedInFunctions[StTy].insert(F);
  };

  // Returns false if \p I is not supported instruction.
  auto IsInstructionSupported = [&](Instruction *I) {
    // TODO: Add more unsupported instructions
    if (isa<InvokeInst>(I))
      return false;
    return true;
  };

  // Returns true if \p CalledValue is a BitCast to convert a function
  // to the FunctionType of \p CI.
  auto IsSimpleFPtrBitCast = [&](Value *CalledValue, CallInst *CI) {
    if (CalledValue->getType() != CI->getFunctionType())
      return false;
    if (auto *BC = dyn_cast<BitCastOperator>(CalledValue))
      if (isa<Function>(BC->getOperand(0)))
        return true;
    return false;
  };

  for (Function &F : M) {
    if (F.isDeclaration() || F.isIntrinsic())
      continue;

    LLVM_DEBUG(dbgs() << "  Pruning in routine: " << F.getName() << "\n");

    for (inst_iterator II = inst_begin(F), E = inst_end(F); II != E; ++II) {
      Instruction &Inst = *II;
      if (!IsInstructionSupported(&Inst)) {
        LLVM_DEBUG(dbgs() << "    Unsupported Inst ... Skip DynClone\n");
        return false;
      }
      if (CallInst *CI = dyn_cast<CallInst>(&Inst)) {
        Value *CalledValue = CI->getCalledValue();
        // Calls using aliases are treated as indirect calls.
        if (!isa<Function>(CalledValue) &&
            !IsSimpleFPtrBitCast(CalledValue, CI)) {
          LLVM_DEBUG(dbgs()
                     << "    Indirect Call ... Skip DynClone :" << *CI << "\n");
          return false;
        }
        Function *Callee = dyn_cast<Function>(CalledValue->stripPointerCasts());
        // Collecting all uses of routines that are marked with AddressTaken.
        // Use info of AddressTaken functions are not properly updated after
        // IPSCCP. So, we can't get all CallSites of AddressTaken function
        // by walking over uses of the function.
        // FunctionCallInsts will be used later during transformation.
        if (Callee->hasAddressTaken())
          FunctionCallInsts[Callee].insert(cast<CallInst>(&Inst));
      }

      UpdateTypeAccessInfo(&Inst, &F);

      if (auto *StInst = dyn_cast<StoreInst>(&Inst)) {
        auto StElem = DTInfo.getStoreElement(StInst);
        if (!StElem.first)
          continue;
        if (!isCandidateField(StElem))
          continue;

        Value *V = StInst->getValueOperand();
        if (isa<ConstantInt>(V)) {
          CheckConstInt(V, StElem);
        } else if (isa<LoadInst>(V)) {
          CheckLoadValue(V, StElem, &F);
        } else {
          auto It = DynFieldFuncMap.find(StElem);
          if (It == DynFieldFuncMap.end()) {
            DynFieldFuncMap.insert({StElem, &F});
            LLVM_DEBUG(dbgs() << "    unknown value assigned in init routine:";
                       printDynField(dbgs(), StElem));
          } else if (It->second != &F) {
            InvalidFields.insert(StElem);
            LLVM_DEBUG(dbgs() << "    Invalid...More than one init routine:";
                       printDynField(dbgs(), StElem));
          }
        }
      } else if (isa<CallInst>(Inst)) {
        WrittenUnknownValue(&Inst);
      }
    }
  }

  // Eliminate invalid candidate fields from CandidateFields.
  DynFieldList ValidCandidates;
  for (auto &CandidatePair : CandidateFields)
    if (!InvalidFields.count(CandidatePair))
      ValidCandidates.push_back(CandidatePair);

  std::swap(CandidateFields, ValidCandidates);
  if (CandidateFields.empty()) {
    LLVM_DEBUG(dbgs() << "    No Candidate is qualified ... Skip DynClone\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << "  Candidate fields after Pruning: \n";
             printCandidateFields(dbgs()));

  // Check Dependency sets here. If a field in dependence set of any
  // candidate field is not qualified, remove all fields from CandidateFields
  // conservatively. union/intersect operations are used to do this.
  LLVM_DEBUG(dbgs() << "  Processing Dependency Sets: \n");
  LLVM_DEBUG(dbgs() << "    Dependency Sets for Candidate fields: \n");
  DynFieldSet AllDepFieldsSet;
  for (auto &CPair : CandidateFields) {
    auto It = DependentFieldSet.find(CPair);
    if (It == DependentFieldSet.end()) {
      LLVM_DEBUG(dbgs() << "      Candidate Field doesn't have Dep Set:";
                 printDynField(dbgs(), CPair));
      continue;
    }
    set_union(AllDepFieldsSet, It->second);
    LLVM_DEBUG(dbgs() << "      Candidate Field:";
               printDynField(dbgs(), It->first); for (auto &SetField
                                                      : It->second) {
                 dbgs() << "        ";
                 printDynField(dbgs(), SetField);
               } dbgs() << "\n";);
  }
  set_intersect(AllDepFieldsSet, InvalidFields);

  if (!AllDepFieldsSet.empty()) {
    LLVM_DEBUG(
        dbgs() << "    No Candidate is qualified after Dep...Skip DynClone\n");
    CandidateFields.clear();
    return false;
  }

  // Verify that all qualified candidate fields have unknown assignments
  // in the same routine. Otherwise, clear CandidateFields.
  assert(!InitRoutine && "Unexpected InitRoutine");
  for (auto &CPair : CandidateFields) {
    LLVM_DEBUG(dbgs() << "    Checking init routine for: ";
               printDynField(dbgs(), CPair));
    auto It = DynFieldFuncMap.find(CPair);
    if (It == DynFieldFuncMap.end()) {
      LLVM_DEBUG(dbgs() << "    Failed ... Field doesn't have Init routine\n");
      InitRoutine = nullptr;
      break;
    }
    if (!InitRoutine) {
      InitRoutine = It->second;
      continue;
    }
    if (InitRoutine != It->second) {
      LLVM_DEBUG(dbgs() << "    Failed ... Init routine doesn't match\n");
      InitRoutine = nullptr;
      break;
    }
  }

  if (!InitRoutine) {
    LLVM_DEBUG(dbgs() << "  No Init Routine identified...Skip DynClone\n");
    CandidateFields.clear();
    return false;
  }

  LLVM_DEBUG(dbgs() << "  Init Routine: " << InitRoutine->getName() << "\n");

  return !CandidateFields.empty();
}

// Check legality issues for InitRoutine here. DynClone will be disabled
// if any legality check is failed for InitRoutine. Basically, it proves
// that structs with candidate fields are not accessed before
// InitRoutine is called. It does following checks:
//   1. InitRoutine is called only once in "main" routine
//   2. Call to InitRoutine is not in Loop
//   3. No access to a struct with candidate fields before InitRoutine is
//      called.
//
bool DynCloneImpl::verifyLegalityChecksForInitRoutine(void) {

  std::function<bool(BasicBlock * CurrentBB,
                     SmallPtrSetImpl<BasicBlock *> & Visited,
                     Instruction * InitInst)>
      CandidateFieldAccessAfterBB;

  // Recursive lambda function to check legality issues for InitRoutine
  // in CurrentBB and all of the successors until InitInst.
  CandidateFieldAccessAfterBB = [this, &CandidateFieldAccessAfterBB](
                                    BasicBlock *CurrentBB,
                                    SmallPtrSetImpl<BasicBlock *> &Visited,
                                    Instruction *InitInst) -> bool {
    if (!Visited.insert(CurrentBB).second)
      return true;

    // If CurrentBB is the BasicBlock that has call to InitRoutine, check
    // legality issues until the call. Otherwise, check for all instructions
    // in CurrentBB.
    BasicBlock *InitBB = InitInst->getParent();
    BasicBlock::iterator EndIt;
    if (InitBB == CurrentBB)
      EndIt = InitInst->getIterator();
    else
      EndIt = CurrentBB->end();

    BasicBlock::iterator It = CurrentBB->begin();
    for (; It != EndIt; ++It) {
      Instruction &I = *It;
      if (auto CS = CallSite(&I)) {
        // Treat InitRoutine as invalid if there are any indirect calls or
        // calls to user defined routines.
        const Function *Callee = CS.getCalledFunction();
        assert(Callee && "Expected only direct calls");
        // Since WholeProgramSafe is true, just check if Callee is defined
        // to prove it is a user defined routine.
        if (!Callee->isDeclaration()) {
          LLVM_DEBUG(dbgs() << "    InitRoutine failed...User routine called: "
                            << I << "\n");
          return false;
        }
      } else if (auto *GEP = dyn_cast<GetElementPtrInst>(&I)) {
        // DynClone is disabled if a struct with candidate fields is
        // accessed before InitRoutine is called.
        auto StType = getGEPStructType(GEP);
        if (!StType)
          continue;
        for (auto &CandidatePair : CandidateFields)
          if (CandidatePair.first == StType) {
            LLVM_DEBUG(dbgs() << "    InitRoutine failed...Struct accessed "
                                 "before InitRoutine:"
                              << getStructName(StType) << "\n");
            return false;
          }
      }
    }
    // Stop checking successors of CurrentBB if it is the BasicBlock that
    // has call to InitRoutine.
    if (InitBB == CurrentBB)
      return true;

    bool Result = true;
    for (auto *BB : successors(CurrentBB))
      Result = Result && CandidateFieldAccessAfterBB(BB, Visited, InitInst);

    return Result;
  };

  assert(InitRoutine && "Expected InitRoutine");

  // Check if InitRoutine is called only once from main and it is not in
  // Loop.
  if (!InitRoutine->hasOneUse()) {
    LLVM_DEBUG(dbgs() << "    InitRoutine failed...More than single use \n");
    return false;
  }
  CallSite CS(InitRoutine->user_back());
  if (InitRoutine->hasAddressTaken()) {
    LLVM_DEBUG(dbgs() << "    InitRoutine failed...AddressTaken \n");
    return false;
  }
  Function *Caller = CS.getCaller();
  if (Caller->getName() != "main" || !Caller->use_empty()) {
    LLVM_DEBUG(dbgs() << "    InitRoutine failed...Not called from main \n");
    return false;
  }

  // TODO: Irreducible CFG check needs to be added here.

  BasicBlock *InitBB = CS.getInstruction()->getParent();
  LoopInfo &LI = (GetLI)(*Caller);
  if (!LI.empty() && LI.getLoopFor(InitBB)) {
    LLVM_DEBUG(dbgs() << "    InitRoutine failed...call in Loop \n");
    return false;
  }

  // Check legality issues in all BasicBlocks starting from Entry block
  // to the place where InitRoutine is called.
  SmallPtrSet<BasicBlock *, 32> Visited;
  BasicBlock *StartBB = &Caller->getEntryBlock();
  if (!CandidateFieldAccessAfterBB(StartBB, Visited, CS.getInstruction()))
    return false;

  int32_t BBCount = 0;
  // Collect Alloc calls related to candidate structs and count return
  // stmts.
  for (BasicBlock &BB : *InitRoutine) {
    if (isa<ReturnInst>(BB.getTerminator()))
      BBCount++;
    for (Instruction &II : BB)
      if (isa<CallInst>(&II)) {
        auto *CInfo = DTInfo.getCallInfo(&II);
        Type *StTy = getCallInfoElemTy(CInfo);
        if (!StTy || !isCandidateStruct(StTy) ||
            CInfo->getCallInfoKind() != CallInfo::CIK_Alloc)
          continue;
        AllocCalls.push_back(std::make_pair(cast<AllocCallInfo>(CInfo), StTy));
      }
  }

  // Allow only single Return for now.
  if (BBCount != 1) {
    LLVM_DEBUG(dbgs() << "    InitRoutine failed...More than one Return\n");
    return false;
  }
  return true;
}

// Create new shrunken structs for all candidate structs and maintain
// mapping between old and new layouts.
//
// New shrunken type is created using the following steps:
//  1. Reduce size of shrunken fields (ex: i64 to i32)
//  2. Reorder fields to eliminate padding created due to shrinking.
//     This step is really not needed since "isPacked" is applied for
//     entire new struct.
//  3. Eliminate padding by applying "isPacked" attribute.
//
void DynCloneImpl::createShrunkenTypes(void) {

  // Return shrunken type for given struct and field index.
  auto GetShrunkenType = [&](StructType *StTy, unsigned Idx) {
    Type *Ty = StTy->getElementType(Idx);
    // Only i64 is supported for now.
    if (Ty->isIntegerTy(64)) {
      return ShrunkenIntTy;
    }
    llvm_unreachable("Unexpected type for shrinking");
  };

  // For given Struct type and field index, returns type of the field
  // in shrunken record.
  auto GetTypeInShrunkenStruct = [&, GetShrunkenType](StructType *StTy,
                                                      unsigned Idx) {
    DynField Field(std::make_pair(StTy, Idx));
    Type *ShrunkenTy;
    // Get shrunken field type if it is candidate field.
    if (isCandidateField(Field))
      ShrunkenTy = GetShrunkenType(StTy, Idx);
    else
      ShrunkenTy = StTy->getElementType(Idx);
    return ShrunkenTy;
  };

  // First, create new struct types without any fields for each
  // candidate struct.
  // No need to fix types of pointer fields that point to their own
  // struct or any other candidate struct since it will be type-casted
  // to original struct types before accessing from memory.
  for (auto &CandidatePair : CandidateFields) {
    StructType *StructT = cast<StructType>(CandidatePair.first);
    if (TransformedTypeMap[StructT])
      continue;
    StructType *NewSt = StructType::create(StructT->getContext(),
                                           "__DYN_" + StructT->getName().str());
    TransformedTypeMap[StructT] = NewSt;
  }

  SmallVector<FieldData, 8> Fields;
  for (auto &CPair : TransformedTypeMap) {
    Fields.clear();
    StructType *StructT = CPair.first;
    StructType *NewSt = CPair.second;
    // Shrink candidate fields and then apply reordering.
    for (unsigned I = 0; I < StructT->getNumElements(); ++I) {
      Type *ShrunkenTy = GetTypeInShrunkenStruct(StructT, I);
      FieldData FD(DL.getABITypeAlignment(ShrunkenTy),
                   DL.getTypeStoreSize(ShrunkenTy), I);
      Fields.push_back(FD);
    }
    llvm::sort(Fields.begin(), Fields.end());

    // Create new layout of fields and maintain mapping of old and new
    // layouts.
    std::vector<Type *> EltTys(StructT->getNumElements());
    unsigned Index = 0;
    std::vector<unsigned> NewIdxVec(Fields.size());
    for (FieldData &FD : Fields) {
      unsigned NewIdx = FD.Index;
      EltTys[Index] = GetTypeInShrunkenStruct(StructT, NewIdx);
      NewIdxVec[NewIdx] = Index++;
    }
    TransformedIndexes[StructT] = NewIdxVec;

    // Set body of new struct and set isPacked to true.
    NewSt->setBody(EltTys, true);

    LLVM_DEBUG(dbgs() << "After dynamic shrinking " << getStructName(StructT)
                      << " :" << *NewSt << "\n");
  }
}

// Transform InitRoutine:
//
//   1. Generates Runtime checks to detect whether all values assigned
//      to candidate fields fit in shrunken type. "__Shrink__Happened__"
//      new GlobalVariable is created to indicate if sizes of candidate
//      fields are reduced. "__Shrink__Happened__" flag is used by other
//      routines to decide whether to use original data-layout or shrunken
//      data-layout. "__Shrink__Happened__" is set to zero when it is
//      created. This flag is set to 1 if all assigned values of candidate
//      fields fit in shrunken type. Collects min and max values that are
//      assigned to candidate fields and then decides at the end of routine
//      if the min and max values fits in shrunken types.
//
//   2. Copy Contents from old layout to new layout before ReturnInst
//
//      Ex:
//      Before:
//              InitRoutine() {
//                  ptr = calloc(); // Memory allocation for candidate struct
//                  ...
//                  stp->candidate1 = value1;
//                  ...
//                  stp->candidate2 = value2;
//                  ...
//                  return;
//              }
//
//      After:
//              __Shrink__Happened__ = 0;
//
//              InitRoutine() {
//                  L_i64_Max = 0xffffffff80000000;
//                  L_i64_Min = 0x000000007fffffff;
//                  ptr = calloc();
//                  ...
//                  stp->candidate1 = value1;
//                  L_i64_Max = (L_i64_Max < value1) ? value1 : L_i64_Max;
//                  L_i64_Min = (L_i64_Min > value1) ? value1 : L_i64_Min;
//                  ...
//                  stp->candidate2 = value2;
//                  L_i64_Max = (L_i64_Max < value2) ? value2 : L_i64_Max;
//                  L_i64_Min = (L_i64_Min > value2) ? value2 : L_i64_Min;
//                  ...
//                  if (L_i64_Max > 0x000000007fffffff ||
//                      L_i64_Min < 0xffffffff80000000) {
//                    // TODO: Fix pointers of shrunken records if needed
//                    OldType* SPtr = ptr;
//                    NewType* DPtr = ptr;
//                    for (i = 0; i < alloc_size_of_calloc; i++) {
//                      t0 = (SPtr + i)->field0;
//                      t1 = (SPtr + i)->field1;
//                      ...
//                      (DPtr + i)->new_field0 = t0;
//                      (DPtr + i)->new_field1 = t1;
//                      ...
//                    }
//                    __Shrink__Happened__ = 1;
//                  }
//                  return;
//              }
//
//
void DynCloneImpl::transformInitRoutine(void) {

  // Returns ConstantInt of max value that fits in shrunken type for
  // the given Ty.
  //    i64  ==>  Max value that fits in int32_t
  auto GetShrunkenMaxValue = [&](Type *Ty) -> Value * {
    if (Ty->isIntegerTy(64)) {
      return ConstantInt::get(Ty, std::numeric_limits<int32_t>::max());
    }
    llvm_unreachable("Unexpected shrunken type for Max Value");
  };

  // Returns ConstantInt of min value that fits in shrunken type for
  // the given Ty.
  //    i64  ==>  Max value that fits in int32_t
  auto GetShrunkenMinValue = [&](Type *Ty) -> Value * {
    if (Ty->isIntegerTy(64))
      return ConstantInt::get(Ty, std::numeric_limits<int32_t>::min());
    llvm_unreachable("Unexpected shrunken type for Min Value");
  };

  // Generates instructions to find  min / max values.
  auto GenerateMinMaxInsts =
      [&](DynField &StElem, StoreInst *Inst, CmpInst::Predicate Pred,
          SmallDenseMap<Type *, AllocaInst *> &TypeAllocIMap) {
        assert(isa<StoreInst>(Inst) && "Expected StoreInst");
        StructType *StTy = cast<StructType>(StElem.first);
        Type *Ty = StTy->getElementType(StElem.second);

        AllocaInst *AI = TypeAllocIMap[Ty];
        assert(AI && "Expected Local var for Ty");

        Value *LI = new LoadInst(AI, "d.ld", Inst);
        Value *SOp = Inst->getValueOperand();
        ICmpInst *ICmp = new ICmpInst(Inst, Pred, LI, SOp, "d.cmp");
        SelectInst *Sel = SelectInst::Create(ICmp, LI, SOp, "d.sel", Inst);
        StoreInst *SI = new StoreInst(Sel, AI, Inst);
        (void)SI;
        LLVM_DEBUG(dbgs() << "      " << *LI << "\n");
        LLVM_DEBUG(dbgs() << "      " << *ICmp << "\n");
        LLVM_DEBUG(dbgs() << "      " << *Sel << "\n");
        LLVM_DEBUG(dbgs() << "      " << *SI << "\n");
        return;
      };

  // Generate final condition and "or" with previous condition if available.
  auto GenerateFinalCond = [&](AllocaInst *AI, Value *V,
                               CmpInst::Predicate Pred, Value *PrevCond,
                               ReturnInst *RI) -> Value * {
    Value *LI = new LoadInst(AI, "d.ld", RI);
    ICmpInst *ICmp = new ICmpInst(RI, Pred, LI, V, "d.cmp");
    LLVM_DEBUG(dbgs() << "      " << *LI << "\n");
    LLVM_DEBUG(dbgs() << "      " << *ICmp << "\n");
    if (!PrevCond)
      return ICmp;

    Value *FinalCond = BinaryOperator::CreateOr(PrevCond, ICmp, "d.or", RI);
    LLVM_DEBUG(dbgs() << "      " << *FinalCond << "\n");
    return FinalCond;
  };

  // Creates GEP instruction like below using \p IRB and returns it.
  //   getelementptr inbounds  %StructTy, %StructTy* %BPtr, %AIdx, FieldIdx
  //
  auto CreateFieldAccessGEP = [&](Type *StructTy, Value *BPtr, PHINode *AIdx,
                                  unsigned FieldIdx,
                                  IRBuilder<> &IRB) -> Value * {
    SmallVector<Value *, 2> Indices;
    Indices.push_back(AIdx);
    Value *Op2 = ConstantInt::get(Type::getInt32Ty(M.getContext()), FieldIdx);
    Indices.push_back(Op2);
    Value *GEP = IRB.CreateInBoundsGEP(StructTy, BPtr, Indices);
    LLVM_DEBUG(dbgs() << "  " << *GEP << "\n");
    return GEP;
  };

  // \p CI is expected to be calloc instruction. Computes Allocation
  // size of \p CI using IRB and returns it.
  //
  // For "calloc(arg1, arg2)", it computes allocation size like
  //
  //    %t1 = mul i64 %arg1, %arg2
  //    %t2 = sdiv %t1, size_of_StTy
  //
  auto ComputeAllocCount = [&](CallInst *CI, Type *StTy,
                               IRBuilder<> &IRB) -> Value * {
    Value *Arg1 = CI->getArgOperand(0);
    Value *Arg2 = CI->getArgOperand(1);
    Value *TSize = IRB.CreateMul(Arg1, Arg2);
    Value *SSize =
        ConstantInt::get(TSize->getType(), DL.getTypeAllocSize(StTy));
    Value *ACount = IRB.CreateSDiv(TSize, SSize);
    LLVM_DEBUG(dbgs() << "  " << *TSize << "\n"
                      << "  " << *ACount << "\n");
    return ACount;
  };

  // Copy data from old layout to new layout for shrunken record by
  // generating loop. \p CI is expected to be calloc instruction.
  // \p Memory is allocated for OrigTy with the calloc instruction.
  // Copy loop is generated before \p InsertBefore.
  //
  // Ex:   %ptr = calloc(asize, ssize)
  //
  // Assume NewTy is new shrunken record type for OrigTy.
  //
  // PreLoop:
  //  LCount = (asize * ssize) / size_of_OrigTy;
  //  LoopIdx = 0;
  //  SrcPtr = (OrigTy*) %ptr;
  //  DstPtr = (NewTy*) %ptr;
  //  Goto Loop:
  // Loop:
  //   SGEP0 = getelementptr inbounds %OrigTy, %SrcPtr, LoopIdx, 0
  //   L0 = Load %SGEP0
  //   SGEP1 = getelementptr inbounds %OrigTy, %SrcPtr, LoopIdx, 1
  //   L1 = Load %SGEP1
  //   ...
  //
  //   DGEP0 = getelementptr inbounds %NewTy, %DstPtr, LoopIdx, new_idx_of_0
  //   %t1 = Trunk %L0
  //   Store %t1, %DGEP0
  //   DGEP1 = getelementptr inbounds %NewTy, %DstPtr, LoopIdx, new_idx_of_1
  //   Store %L1, %DGEP1
  //   ...
  //   LoopIdx++
  //   if (LoopIdx < LCount) goto Loop:
  // PostLoop:
  //   __Shrink__Happened__ = 1;
  //
  auto CopyDataFromOrigLayoutToNewLayout = [this, &CreateFieldAccessGEP,
                                            &ComputeAllocCount](
                                               CallInst *CI, Type *OrigTy,
                                               Instruction *InsertBefore) {
    // Get shrunken type.
    Type *NewTy = TransformedTypeMap[cast<StructType>(OrigTy)];
    LLVM_DEBUG(dbgs() << "Copy data for " << getStructName(OrigTy) << "  at "
                      << *CI << "\n");

    // Create blocks for PreLoop, Loop and PostLoop and fix CFG.
    BasicBlock *PreLoopBB = InsertBefore->getParent();
    BasicBlock *PostLoopBB =
        PreLoopBB->splitBasicBlock(InsertBefore, "postloop");
    BasicBlock *LoopBB = BasicBlock::Create(PreLoopBB->getContext(), "copydata",
                                            InitRoutine, PostLoopBB);

    IRBuilder<> PLB(PreLoopBB->getTerminator());

    Value *LCount = ComputeAllocCount(CI, OrigTy, PLB);
    Value *SrcPtr = PLB.CreateBitCast(CI, OrigTy->getPointerTo());
    Value *DstPtr = PLB.CreateBitCast(CI, NewTy->getPointerTo());
    PLB.CreateBr(LoopBB);
    PreLoopBB->getTerminator()->eraseFromParent();
    LLVM_DEBUG(dbgs() << "  " << *SrcPtr << "\n"
                      << "  " << *DstPtr << "\n");

    // Create loop index.
    IRBuilder<> LB(LoopBB);
    Type *IdxTy = LCount->getType();
    PHINode *LoopIdx = LB.CreatePHI(IdxTy, 2, "lindex");
    LoopIdx->addIncoming(ConstantInt::get(IdxTy, 0U), PreLoopBB);

    SmallVector<LoadInst *, 8> LoadVec;
    StructType *OrigSt = cast<StructType>(OrigTy);
    StructType *NewSt = cast<StructType>(NewTy);
    // Generate Loads for all element of OrigTy at LoopIdx location.
    for (unsigned I = 0; I < OrigSt->getNumElements(); ++I) {
      Value *SrcGEP = CreateFieldAccessGEP(OrigTy, SrcPtr, LoopIdx, I, LB);
      LoadInst *LI = LB.CreateLoad(SrcGEP);
      LoadVec.push_back(LI);
      LLVM_DEBUG(dbgs() << "  " << *LI << "\n");
    }
    // Generate Stores for all element of NewTy at LoopIdx location.
    for (unsigned I = 0; I < OrigSt->getNumElements(); ++I) {
      unsigned NewI = TransformedIndexes[OrigSt][I];
      Value *DstGEP = CreateFieldAccessGEP(NewTy, DstPtr, LoopIdx, NewI, LB);
      Type *NewElemTy = NewSt->getElementType(NewI);
      Value *LI = LoadVec[I];
      if (LI->getType() != NewElemTy) {
        LI = LB.CreateTruncOrBitCast(LI, NewElemTy);
        LLVM_DEBUG(dbgs() << "  " << *LI << "\n");
      }
      StoreInst *SI = LB.CreateStore(LI, DstGEP);
      LLVM_DEBUG(dbgs() << "  " << *SI << "\n");
      (void)SI;
    }
    // Increment LoopIdx.
    Value *NewIdx = LB.CreateAdd(LoopIdx, ConstantInt::get(IdxTy, 1U));
    LoopIdx->addIncoming(NewIdx, LoopBB);
    auto *Br =
        LB.CreateCondBr(LB.CreateICmpULT(NewIdx, LCount), LoopBB, PostLoopBB);
    LLVM_DEBUG(dbgs() << "  " << *NewIdx << "\n");
    LLVM_DEBUG(dbgs() << "  " << *Br << "\n");
    (void)Br;
  };

  SmallVector<BasicBlock *, 2> RetBBs;
  SmallVector<StoreInst *, 16> RuntimeCheckStores;

  // Collect all StoreInsts that assign values to candidate fields and
  // BasicBlocks's with ReturnInst..
  for (BasicBlock &BB : *InitRoutine) {
    // Collect BasicBlocks's with ReturnInst.
    if (isa<ReturnInst>(BB.getTerminator()))
      RetBBs.push_back(&BB);

    // Collect StoreInsts
    for (auto I = BB.begin(), E = BB.end(); I != E; I++) {
      auto *Inst = &*I;
      if (auto *StInst = dyn_cast<StoreInst>(Inst)) {
        auto StElem = DTInfo.getStoreElement(StInst);
        if (!StElem.first)
          continue;
        if (!isCandidateField(StElem))
          continue;
        Value *V = StInst->getValueOperand();
        // Ignore constant values here since they are already verified.
        if (isa<ConstantInt>(V))
          continue;
        RuntimeCheckStores.push_back(StInst);
      }
    }
  }

  assert(RetBBs.size() == 1 && "Expected only one Return");

  // Create GlobalVariable to indicate whether DynClone occurred or not.
  Type *FlagType = Type::getInt8Ty(M.getContext());
  assert(!ShrinkHappenedVar && "Expected nullptr for ShrinkHappenedVar");
  ShrinkHappenedVar = new GlobalVariable(
      M, FlagType, false, GlobalVariable::CommonLinkage,
      ConstantInt::get(FlagType, 0), Twine("__Shrink__Happened__"));

  LLVM_DEBUG(dbgs() << "    ShrinkHappenedVar: " << *ShrinkHappenedVar << "\n");

  // Map of Type and AllocaInst where max value of the type is saved.
  SmallDenseMap<Type *, AllocaInst *> TypeMaxAllocIMap;
  // Map of Type and AllocaInst where min value of the type is saved.
  SmallDenseMap<Type *, AllocaInst *> TypeMinAllocIMap;

  // Create Local variables to save max and min values for each candidate
  // type.
  //  entry:
  //      %d.max = alloca i64
  //      store i64 -2147483648, i64* %d.max
  //      %d.min = alloca i64
  //      store i64 2147483647, i64* %d.min
  //
  LLVM_DEBUG(dbgs() << "    Create and Initialize min and max variables: \n");
  for (auto &CPair : CandidateFields) {
    StructType *StTy = cast<StructType>(CPair.first);
    Type *Ty = StTy->getElementType(CPair.second);
    // Create new Local max and min variables for Ty if not already there.
    if (TypeMaxAllocIMap[Ty]) {
      assert(TypeMinAllocIMap[Ty] &&
             " Expected local min variable already created");
      continue;
    }
    AllocaInst *AI =
        new AllocaInst(Ty, DL.getAllocaAddrSpace(), nullptr, "d.max",
                       &InitRoutine->getEntryBlock().front());
    TypeMaxAllocIMap[Ty] = AI;
    StoreInst *SI =
        new StoreInst(GetShrunkenMinValue(Ty), AI, AI->getNextNode());
    LLVM_DEBUG(dbgs() << "      " << *AI << "\n");
    LLVM_DEBUG(dbgs() << "      " << *SI << "\n");

    AI = new AllocaInst(Ty, DL.getAllocaAddrSpace(), nullptr, "d.min",
                        SI->getNextNode());
    TypeMinAllocIMap[Ty] = AI;
    SI = new StoreInst(GetShrunkenMaxValue(Ty), AI, AI->getNextNode());
    (void)SI;
    LLVM_DEBUG(dbgs() << "      " << *AI << "\n");
    LLVM_DEBUG(dbgs() << "      " << *SI << "\n");
  }

  // For each StoreInst, it generates instructions like
  //   Before:
  //     store i64 %g1, i64* %F1, align 8
  //
  //   After:
  //       %d.ld = load i64, i64* %d.min
  //       %d.cmp = icmp slt i64 %d.ld, %g1
  //       %d.sel = select i1 %d.cmp, i64 %d.ld, i64 %g1
  //       store i64 %d.sel, i64* %d.min
  //       %d.ld1 = load i64, i64* %d.max
  //       %d.cmp2 = icmp sgt i64 %d.ld1, %g1
  //       %d.sel3 = select i1 %d.cmp2, i64 %d.ld1, i64 %g1
  //       store i64 %d.sel3, i64* %d.max
  //       store i64 %g1, i64* %F1, align 8
  //
  for (auto StInst : RuntimeCheckStores) {
    auto StElem = DTInfo.getStoreElement(StInst);
    LLVM_DEBUG(dbgs() << "    Find min and max for " << *StInst << "\n");
    GenerateMinMaxInsts(StElem, StInst, ICmpInst::ICMP_SLT, TypeMinAllocIMap);
    GenerateMinMaxInsts(StElem, StInst, ICmpInst::ICMP_SGT, TypeMaxAllocIMap);
  }

  // Generates instructions to compute final condition to detect
  // whether the assigned values fit in shrunken types like below.
  //
  //   %d.ld16 = load i64, i64* %d.min
  //   %d.cmp17 = icmp slt i64 %d.ld16, -2147483648
  //   %d.ld18 = load i64, i64* %d.max
  //   %d.cmp19 = icmp sgt i64 %d.ld18, 2147483647
  //   %d.or = or i1 %d.cmp17, %d.cmp19
  //
  LLVM_DEBUG(dbgs() << "    Generate Final condition \n");
  BasicBlock *OrigBB = RetBBs.front();
  ReturnInst *RetI = cast<ReturnInst>(OrigBB->getTerminator());
  Value *FinalCond = nullptr;
  for (auto &Pair : TypeMinAllocIMap)
    FinalCond = GenerateFinalCond(Pair.second, GetShrunkenMinValue(Pair.first),
                                  ICmpInst::ICMP_SLT, FinalCond, RetI);
  for (auto &Pair : TypeMaxAllocIMap)
    FinalCond = GenerateFinalCond(Pair.second, GetShrunkenMaxValue(Pair.first),
                                  ICmpInst::ICMP_SGT, FinalCond, RetI);

  // TODO: It is better not to set __Shrink__Happened__ when none of
  // StoreInst of candidate fields is executed at runtime in InitRoutine.
  // Add a another check for FinalCond to do the same.

  assert(FinalCond && "Expected non-null Final condition");

  // Before:
  //   original:
  //     ...
  //     ret void
  //
  // After:
  //   original:
  //   ...
  //   br i1 %d.or, label %1, label %d.set_happened
  //
  //   d.set_happened:
  //     store i8 1, i8* @__Shrink__Happened__
  //     br label %1
  //
  //   <label>:1:
  //     ret void
  //
  // Split OrigBB just before RetI.
  LLVM_DEBUG(dbgs() << "    Set __Shrink__Happened__ to 1 \n");
  BasicBlock *LastBB = OrigBB->splitBasicBlock(RetI);

  BasicBlock *NewBB = BasicBlock::Create(OrigBB->getContext(), "d.set_happened",
                                         OrigBB->getParent(), LastBB);
  OrigBB->getTerminator()->eraseFromParent();
  BranchInst *BI = BranchInst::Create(LastBB, NewBB, FinalCond, OrigBB);
  LLVM_DEBUG(dbgs() << "      " << *BI << "\n");

  StoreInst *SI =
      new StoreInst(ConstantInt::get(ShrinkHappenedVar->getValueType(), 1),
                    ShrinkHappenedVar, NewBB);
  LLVM_DEBUG(dbgs() << "      " << *SI << "\n");
  BranchInst::Create(LastBB, NewBB);
  (void)BI;
  // Copy data from old layout to new layout.
  for (auto &AllocPair : AllocCalls) {
    auto *AInfo = AllocPair.first;
    assert(AInfo->getAllocKind() == AK_Calloc && " Unexpected alloc kind");
    CallInst *CI = cast<CallInst>(AInfo->getInstruction());
    CopyDataFromOrigLayoutToNewLayout(CI, AllocPair.second, SI);
  }
}

//  Before:
//      main() {
//        ...
//        InitRoutine();
//        ...
//        foo();
//        ...
//        bar();
//        ..
//      }
//
//      foo() {
//        ...
//        baz();
//      }
//      bar() { // Assume no accesses to shrunken structs
//        ...
//      }
//      baz() { // Assume shrunken structs are accessed in this routine.
//        ...
//        baz();
//      }
//
// No user call is allowed before calling InitRoutine. Except InitRoutine,
// all other routines, which have accesses to shrunken structs,  are called
// from main are call-graph cloned. Calls to those routines in main are
// specialized with "__Shrink__Happened__ == 0" check.
//
//  After:
//      main() {
//        ...
//        InitRoutine();
//        ...
//        if (__Shrink__Happened__ == 0) {
//          foo.1();
//        }
//        else {
//          foo();
//        }
//
//        ...
//        bar();  // No specialization
//        ..
//      }
//
//      foo() {
//        ...
//        baz();
//      }
//
//      foo.1() { // foo is cloned as foo.1
//        ...
//        baz.1();  // baz.1 is clone of baz
//      }
//
//      bar() { // bar is not cloned.
//      }
//      baz() {
//        ...
//        baz();
//      }
//      baz.1() {
//        ...
//        baz.1();
//      }
//
// TODO: Main restriction for the above solution is that shrunken structs
// shouldn't be accessed in main routine directly. We could use
// CodeExtractor class to create a new function, which will be cloned
// and controlled under __Shrink__Happened__, for all BasicBlocks that can
// be reached from "InitRoutine()" call.
//
bool DynCloneImpl::createCallGraphClone(void) {

  std::function<void(Function * F, Function * CallerMain,
                     CallInstSet & SpecializedCallsInMain)>
      FindCloningCallChains;

  // We can't set NoInline for all cloned routines due to performance reasons.
  // That means, we need to clone callers of cloned routines also to generate
  // correct code. This recursive lambda function finds all call-chains of
  // cloned routines till "main" is reached. NoInline attribute is set for
  // calls of cloned routines in "main" later.
  FindCloningCallChains =
      [this, &FindCloningCallChains](Function *F, Function *CallerMain,
                                     CallInstSet &SpecializedCallsInMain) {
        LLVM_DEBUG(dbgs() << "    Finding cloning call-chain..." << F->getName()
                          << "\n");
        // Add to the list of cloned routines.
        if (!ClonedFunctionList.insert(F).second) {
          LLVM_DEBUG(dbgs() << "    Already processed ... \n");
          return;
        }

        if (F->hasAddressTaken()) {
          for (CallInst *CI : FunctionCallInsts[F]) {
            LLVM_DEBUG(dbgs() << "    Processing CallSite1: " << *CI << "\n");
            Function *CalledF = CI->getFunction();
            if (CalledF == CallerMain) {
              // Specialize these calls later.
              SpecializedCallsInMain.insert(CI);
              continue;
            }
            FindCloningCallChains(CalledF, CallerMain, SpecializedCallsInMain);
          }
        } else {
          for (auto *U : F->users()) {
            assert(isa<CallInst>(U) && "Expected CallInst");
            CallInst *CI = cast<CallInst>(U);
            LLVM_DEBUG(dbgs() << "    Processing CallSite2: " << *CI << "\n");
            Function *CalledF = CI->getFunction();
            if (CalledF == CallerMain) {
              // Specialize these calls later.
              SpecializedCallsInMain.insert(CI);
              continue;
            }
            FindCloningCallChains(CalledF, CallerMain, SpecializedCallsInMain);
          }
        }
      };

  // Specialize CallInstruction:
  //
  //   Before:
  //      main() {
  //        ...
  //        t = foo();  // F
  //      }
  //
  //   After:
  //      main() {
  //        ...
  //        if (__Shrink__Happened__ == 0) {
  //          t0 = foo.1();  // NewF
  //        }
  //        else {
  //          t1 = foo();   // F
  //        }
  //        t = PHI(t0, t1);
  //      }
  //
  auto SpecializeCallInst = [&](CallInst *CI, Function *F, Function *NewF) {
    // Create ICmp inst
    Instruction *OrigInst = cast<Instruction>(CI);
    LLVM_DEBUG(dbgs() << "    Specializing call: " << *OrigInst << "\n");
    Type *SHVarTy = ShrinkHappenedVar->getValueType();
    Constant *ConstantZero = ConstantInt::get(SHVarTy, 0);
    IRBuilder<> Builder(OrigInst);
    LoadInst *Load = Builder.CreateLoad(ShrinkHappenedVar, "d.gld");
    auto *Cond = Builder.CreateICmpEQ(Load, ConstantZero, "d.gc");
    LLVM_DEBUG(dbgs() << "      Load: " << *Load << "\n");
    LLVM_DEBUG(dbgs() << "      Cmp: " << *Cond << "\n");

    // Fix CFG and create new CallInst
    TerminatorInst *TrueT = nullptr;
    TerminatorInst *FalseT = nullptr;
    SplitBlockAndInsertIfThenElse(Cond, OrigInst, &TrueT, &FalseT);
    BasicBlock *TrueB = TrueT->getParent();
    BasicBlock *FalseB = FalseT->getParent();
    BasicBlock *MergeBlock = OrigInst->getParent();
    TrueB->setName("d.t");
    FalseB->setName("d.f");
    MergeBlock->setName("d.m");
    Instruction *NewInst = OrigInst->clone();
    OrigInst->moveBefore(FalseT);
    NewInst->insertBefore(TrueT);
    CallInst *NewCI = cast<CallInst>(NewInst);
    NewCI->setCalledFunction(NewF);

    LLVM_DEBUG(dbgs() << "      New Call (False): " << *NewInst << "\n");
    LLVM_DEBUG(dbgs() << "    Set NoInline for both orig and new calls\n");
    CI->setIsNoInline();
    NewCI->setIsNoInline();

    if (OrigInst->getType()->isVoidTy() || OrigInst->use_empty())
      return;

    // Unify returns values of original and new call instructions
    Builder.SetInsertPoint(&MergeBlock->front());
    PHINode *Phi = Builder.CreatePHI(OrigInst->getType(), 0, "d.p");
    SmallVector<User *, 16> UsersToUpdate;
    for (User *U : OrigInst->users())
      UsersToUpdate.push_back(U);
    for (User *U : UsersToUpdate)
      U->replaceUsesOfWith(OrigInst, Phi);
    Phi->addIncoming(NewInst, NewInst->getParent());
    Phi->addIncoming(OrigInst, OrigInst->getParent());
    LLVM_DEBUG(dbgs() << "      PHI: " << *Phi << "\n");
  };

  CallSite CS(InitRoutine->user_back());
  Function *CallerMain = CS.getCaller();

  FunctionSet CandidateAccessRoutines;
  // TODO: Move the below checks before generating any runtime checks.
  for (auto TPair : TypeAccessedInFunctions) {
    if (!isCandidateStruct(TPair.first))
      continue;
    FunctionSet FSet = TPair.second;
    // 1. Candidate struct should be accessed in at least two
    //    routines.
    // 2. One of them should be InitRoutine.
    // 3. Candidate struct shouldn't be accessed in main routine.
    if (FSet.size() < 2 || FSet.count(InitRoutine) == 0 ||
        FSet.count(CallerMain) != 0) {
      LLVM_DEBUG(dbgs() << "    Unexpected AccessSet .. Skip DynClone\n");
      return false;
    }
    CandidateAccessRoutines.insert(FSet.begin(), FSet.end());
  }
  if (CandidateAccessRoutines.empty()) {
    LLVM_DEBUG(dbgs() << "    Empty AccessSet .. Skip DynClone\n");
    return false;
  }

  // List of CallInsts that need to be specialized in main.
  CallInstSet SpecializedCallsInMain;
  // Collect all routines that need to be cloned.
  for (auto *F : CandidateAccessRoutines) {
    // Shouldn't clone InitRoutine
    if (F == InitRoutine)
      continue;
    LLVM_DEBUG(dbgs() << "    Collect call-chains: " << F->getName() << "\n");
    FindCloningCallChains(F, CallerMain, SpecializedCallsInMain);
  }

  // Create cloned routines
  for (auto *F : ClonedFunctionList) {
    ValueToValueMapTy VMap;
    LLVM_DEBUG(dbgs() << "    Cloning ..." << F->getName() << "\n");
    Function *NewF = CloneFunction(F, VMap);
    CloningMap[F] = NewF;
    LLVM_DEBUG(dbgs() << "    New Cloned entry: " << NewF->getName() << "\n");
  }

  for (auto *F : ClonedFunctionList) {
    Function *NewF = CloningMap[F];
    for (inst_iterator II = inst_begin(NewF), E = inst_end(NewF); II != E;
         ++II) {
      Instruction &Inst = *II;
      if (!isa<CallInst>(&Inst))
        continue;
      CallInst *CI = cast<CallInst>(&Inst);
      Value *CalledValue = CI->getCalledValue();
      Function *Callee = dyn_cast<Function>(CalledValue->stripPointerCasts());
      if (!ClonedFunctionList.count(Callee))
        continue;
      Function *NewCallee = CloningMap[Callee];
      LLVM_DEBUG(dbgs() << "    Inst Before ..." << Inst << " in "
                        << NewF->getName() << "\n");

      Constant *BitcastNew = NewCallee;
      if (!isa<Function>(CalledValue)) {
        BitcastNew =
            ConstantExpr::getBitCast(NewCallee, CalledValue->getType());
      }
      CI->replaceUsesOfWith(CalledValue, BitcastNew);

      LLVM_DEBUG(dbgs() << "    Inst After ..." << Inst << "\n");
    }
  }
  // Specialize calls in "main".
  for (auto *CI : SpecializedCallsInMain) {
    Function *F = CI->getCalledFunction();
    Function *NewF = CloningMap[F];
    SpecializeCallInst(CI, F, NewF);
  }
  return true;
}

bool DynCloneImpl::run(void) {

  LLVM_DEBUG(dbgs() << "DynCloning Transformation \n");

  if (!gatherPossibleCandidateFields())
    return false;

  LLVM_DEBUG(dbgs() << "    Possible Candidate fields: \n";
             printCandidateFields(dbgs()));

  if (!prunePossibleCandidateFields())
    return false;

  if (!verifyLegalityChecksForInitRoutine())
    return false;

  LLVM_DEBUG(dbgs() << "    Verified InitRoutine ... \n");

  // TODO: Lot more checks are needed before generating runtime checks.
  // Most of those checks are basically to prove it is legal to rematerialize
  // struct pointers after shrining the struct. These checks will be
  // implemented later.

  createShrunkenTypes();

  transformInitRoutine();

  LLVM_DEBUG(dbgs() << "    Generated Runtime checks in InitRoutine ... \n");

  LLVM_DEBUG(dbgs() << "  Final Candidate fields: \n";
             printCandidateFields(dbgs()));

  if (!createCallGraphClone())
    return false;

  return true;
}

bool DynClonePass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                           TargetLibraryInfo &TLI, WholeProgramInfo &WPInfo,
                           LoopInfoFuncType &GetLI) {

  if (!WPInfo.isWholeProgramSafe())
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  auto &DL = M.getDataLayout();

  DynCloneImpl DynCloneI(M, DL, DTInfo, GetLI);
  return DynCloneI.run();
}

PreservedAnalyses DynClonePass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  LoopInfoFuncType GetLI = [&FAM](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };

  if (!runImpl(M, DTransInfo, AM.getResult<TargetLibraryAnalysis>(M), WPInfo,
               GetLI))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // namespace dtrans

} // namespace llvm
