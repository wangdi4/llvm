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
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
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
    return Impl.runImpl(M, DTInfo,
                        getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(),
                        getAnalysis<WholeProgramWrapperPass>().getResult());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
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
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransDynCloneWrapper, "dtrans-dynclone",
                    "DTrans dynamic cloning", false, false)

ModulePass *llvm::createDTransDynCloneWrapperPass() {
  return new DTransDynCloneWrapper();
}

namespace llvm {

namespace dtrans {

class DynCloneImpl {
  using DynField = std::pair<llvm::Type *, size_t>;
  using DynFieldList = SmallVector<DynField, 16>;
  // Even though it will be very small set,  using std::set (instead of
  // SmallSet/SetVector) to do set operations like union/intersect.
  using DynFieldSet = std::set<DynField>;

public:
  DynCloneImpl(Module &M, const DataLayout &DL, DTransAnalysisInfo &DTInfo)
      : M(M), DL(DL), DTInfo(DTInfo),
        ShrinkedIntTy(Type::getInt32Ty(M.getContext())),
        InitRoutine(nullptr) {};
  bool run(void);

private:
  Module &M;
  const DataLayout &DL;
  DTransAnalysisInfo &DTInfo;

  // Holds result Type after shrinking 64-bit to 32-bit integer values.
  llvm::Type *ShrinkedIntTy;

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

  bool gatherPossibleCandidateFields(void);
  bool prunePossibleCandidateFields(void);
  bool isCandidateField(DynField &DField) const;
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

    if (!ConstantInt::isValueValidForType(ShrinkedIntTy,
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

  for (Function &F : M) {
    if (F.isDeclaration() || F.isIntrinsic())
      continue;

    LLVM_DEBUG(dbgs() << "  Pruning in routine: " << F.getName() << "\n");

    for (inst_iterator II = inst_begin(F), E = inst_end(F); II != E; ++II) {
      Instruction &Inst = *II;
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

bool DynCloneImpl::run(void) {

  LLVM_DEBUG(dbgs() << "DynCloning Transformation \n");

  if (!gatherPossibleCandidateFields())
    return false;

  LLVM_DEBUG(dbgs() << "    Possible Candidate fields: \n";
             printCandidateFields(dbgs()));

  if (!prunePossibleCandidateFields())
    return false;

  LLVM_DEBUG(dbgs() << "  Final Candidate fields: \n";
             printCandidateFields(dbgs()));

  return true;
}

bool DynClonePass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                           TargetLibraryInfo &TLI, WholeProgramInfo &WPInfo) {

  if (!WPInfo.isWholeProgramSafe())
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  auto &DL = M.getDataLayout();

  DynCloneImpl DynCloneI(M, DL, DTInfo);
  return DynCloneI.run();
}

PreservedAnalyses DynClonePass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  if (!runImpl(M, DTransInfo, AM.getResult<TargetLibraryAnalysis>(M), WPInfo))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // namespace dtrans

} // namespace llvm
