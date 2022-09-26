//==== DeleteFieldOP.cpp - Delete field with support for opaque pointers ====//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
// This file implements the DTrans delete field optimization pass with support
// for IR using either opaque or non-opaque pointers.
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DeleteFieldOP.h"

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"
#include "Intel_DTrans/Transforms/DTransOptUtils.h"
#include "llvm/ADT/PriorityWorklist.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Operator.h"

using namespace llvm;
using namespace dtransOP;

#define DEBUG_TYPE "dtrans-deletefieldop"

namespace {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// For testing purposes, this flag allows the types desired to be converted to
// be specified in a semicolon separated list of structure names. The fields to
// be deleted from these structures still need to meet the normal checks that
// the transformation supports deleting them. Specifying the name of a structure
// that has no candidate fields for deletion will allow exercising the code that
// performs type and variable remapping, but the replacement structure will be
// identical to the original structure.
cl::opt<std::string>
    DeleteFieldTypelist("dtrans-deletefieldop-typelist",
                        cl::desc("Specify structures for deletion"),
                        cl::ReallyHidden);

// For triaging the transformation, define an upper limit on the number of
// candidate structures that will be allowed to have fields deleted.
cl::opt<unsigned>
    DeleteFieldMaxStruct("dtrans-deletefieldop-max-struct",
                         cl::init(std::numeric_limits<unsigned>::max()),
                         cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

class DTransDeleteFieldOPWrapper : public ModulePass {
private:
  dtransOP::DeleteFieldOPPass Impl;

public:
  static char ID;

  DTransDeleteFieldOPWrapper() : ModulePass(ID) {
    initializeDTransDeleteFieldOPWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    DTransSafetyAnalyzerWrapper &DTAnalysisWrapper =
        getAnalysis<DTransSafetyAnalyzerWrapper>();
    DTransSafetyInfo &DTInfo = DTAnalysisWrapper.getDTransSafetyInfo(M);

    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    WholeProgramInfo &WPInfo =
        getAnalysis<WholeProgramWrapperPass>().getResult();
    bool Changed = Impl.runImpl(M, &DTInfo, WPInfo, GetTLI);
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransSafetyAnalyzerWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<TargetLibraryInfoWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

class DeleteFieldOPImpl : public DTransOPOptBase {
public:
  DeleteFieldOPImpl(LLVMContext &Ctx, DTransSafetyInfo *DTInfo,
                    StringRef DepTypePrefix, const DataLayout &DL,
                    DeleteFieldOPPass::GetTLIFn GetTLI)
      : DTransOPOptBase(Ctx, DTInfo, DepTypePrefix), DL(DL), GetTLI(GetTLI) {}

  bool prepareTypes(Module &M) override;
  void populateTypes(Module &M) override;
  GlobalVariable *createGlobalVariableReplacement(GlobalVariable *GV,
                                                  ValueMapper &Mapper) override;
  void initializeGlobalVariableReplacement(GlobalVariable *OrigGV,
                                           GlobalVariable *NewGV,
                                           ValueMapper &Mapper) override;
  void postprocessGlobalVariable(GlobalVariable *OrigGV,
                                 GlobalVariable *NewGV) override;
  void processFunction(Function &F) override;
  void postprocessFunction(Function &OrigFunc, bool isCloned) override;

private:
  void selectCandidates();
  void pruneCandidates();
  void pruneCandidatesByParentSafety();
  void pruneCandidatesForTriaging();

  void buildTypeEnclosingMapping(DTransTypeToTypeSetMap &TypeMap);
  bool checkParentStructure(dtrans::StructInfo *ParentStruct);

  bool processGEPIndex(GetElementPtrInst *GEP, ArrayRef<Value *> BaseIndices,
                       Value *Idx, uint64_t &NewIndex, bool &AffectedStructure,
                       bool &IsPacked, bool IsPreCloning);
  bool processGEPInst(GetElementPtrInst *GEP, bool IsPreCloning);
  bool processPossibleByteFlattenedGEP(GetElementPtrInst *GEP);
  void postprocessCall(CallBase *Call);
  void processSubInst(BinaryOperator *BinOp);

  bool typeContainsDeletedFields(llvm::Type *Ty);
  Constant *getReplacement(Constant *Init, ValueMapper &Mapper);
  Constant *getStructReplacement(ConstantStruct *StInit, ValueMapper &Mapper);
  Constant *getArrayReplacement(ConstantArray *ArInit, ValueMapper &Mapper);

  const DataLayout &DL;

  DeleteFieldOPPass::GetTLIFn GetTLI;

  // The pointers in this vector are owned by the DTransSafetyInfo class.
  // The list is populated during prepareTypes() and used in populateTypes().
  SmallVector<dtrans::StructInfo *, 4> StructsToConvert;

  // The list of original structure types that do not have fields being directly
  // deleted from, but need to have updates made on calls or instructions that
  // involve the structure size. For example:
  //   %struct.A = type{ i32, %struct.B, i32 }
  //   %struct.B = type{ i8, i16, i32 }
  // Deleting a field from %struct.B will cause a change to the size of
  // %struct.A.
  SmallPtrSet<llvm::StructType *, 4> OrigEnclosingTypes;

  // A mapping from the original structure type to the new structure type
  using LLVMStructTypeToStructTypeMap =
      DenseMap<llvm::StructType *, llvm::StructType *>;
  LLVMStructTypeToStructTypeMap OrigToNewTypeMapping;

  // A mapping from the llvm::Type to the DTransType for the original & new
  // structure types processed by this transformation.
  DenseMap<llvm::StructType *, DTransStructType *>
      LLVMStructTypeToDTransStructType;

  // A mapping from original types to a vector which can be used to lookup
  // the replacement index based on the original index. A replacement index
  // value of FIELD_DELETED indicates that the field was deleted.
  const uint64_t FIELD_DELETED = ~0ULL;
  DenseMap<llvm::StructType *, SmallVector<uint64_t, 16>> FieldIdxMap;
};
} // end anonymous namespace

// Return 'true' if the field can be deleted.
// We're only interested in fields that are never read or their value is
// unused. Fields that are written but not read can be deleted. Fields with
// complex uses (phi, select, icmp, memfuncs, etc.) cannot be deleted.
static bool canDeleteField(dtrans::FieldInfo &FI) {
  return (!FI.isRead() || FI.isValueUnused()) && !FI.hasComplexUse() &&
         !FI.hasNonGEPAccess() && !FI.getLLVMType()->isAggregateType();
}

static void safeEraseValueImpl(Value *V, SmallSetVector<Value *, 32> &Stack) {
  LLVM_DEBUG(dbgs() << "Handling: " << *V << "\n");

  if (!Stack.insert(V))
    return;

  for (Value *U : V->users())
    safeEraseValueImpl(U, Stack);
}

static void safeEraseValue(Value *V) {
  SmallSetVector<Value *, 32> Stack;
  safeEraseValueImpl(V, Stack);

  for (auto I = Stack.rbegin(), E = Stack.rend(); I != E; ++I) {
    if (auto *Inst = dyn_cast<Instruction>(*I)) {
      LLVM_DEBUG(dbgs() << "Delete field: erasing instruction:\n"
                        << *Inst << "\n");
      Inst->replaceAllUsesWith(UndefValue::get(Inst->getType()));
      Inst->eraseFromParent();
    } else if (auto *Const = dyn_cast<Constant>(*I)) {
      assert(!Const->isConstantUsed() &&
             "Attempting to delete const GEP that still has uses!");

      LLVM_DEBUG(dbgs() << "Delete field: destroying constant:\n"
                        << *Const << "\n");
      Const->destroyConstant();
    }
  }
}

bool DeleteFieldOPImpl::prepareTypes(Module &M) {
  selectCandidates();
  pruneCandidates();

  if (StructsToConvert.empty()) {
    LLVM_DEBUG(dbgs() << "  No candidates found.\n");
    return false;
  }

  LLVMContext &Context = M.getContext();
  for (auto *StInfo : StructsToConvert) {
    LLVM_DEBUG(dbgs() << "  Selected for deletion: " << *StInfo->getDTransType()
                      << "\n";);

    // Create an opaque structure as a placeholder to allow the base class to
    // compute all the dependent types that need to be created.
    StructType *OrigLLVMTy = cast<StructType>(StInfo->getLLVMType());
    assert(OrigLLVMTy->hasName() &&
           "Conversion is not supported for literal types");
    StructType *NewLLVMTy = StructType::create(
        Context, (Twine("__DFT_" + OrigLLVMTy->getName()).str()));

    // Also, create an opaque type in the DTransType representation.
    DTransStructType *DTransStructTy = TM.getStructType(OrigLLVMTy->getName());
    assert(DTransStructTy &&
           "Expected DTransStructType for original structure");
    DTransStructType *NewDTransStructTy = TM.getOrCreateStructType(NewLLVMTy);

    TypeRemapper.addTypeMapping(OrigLLVMTy, NewLLVMTy, DTransStructTy,
                                NewDTransStructTy);
    OrigToNewTypeMapping[OrigLLVMTy] = NewLLVMTy;
    LLVMStructTypeToDTransStructType[OrigLLVMTy] = DTransStructTy;
    LLVMStructTypeToDTransStructType[NewLLVMTy] = NewDTransStructTy;
  }

  return true;
}

void DeleteFieldOPImpl::selectCandidates() {
  LLVM_DEBUG(dbgs() << "Delete field for opaque pointers: looking for "
                       "candidate structures.\n");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Build the list of structures to convert based on a command line option
  // containing a list of structure names separated by semicolons.
  if (!DeleteFieldTypelist.empty()) {
    SmallVector<StringRef, 16> SubStrings;
    SplitString(DeleteFieldTypelist, SubStrings, ";");
    for (auto &TypeName : SubStrings) {
      DTransStructType *DTransTy = TM.getStructType(TypeName);
      if (!DTransTy) {
        LLVM_DEBUG(dbgs() << "No struct found for: " << TypeName << "\n");
        continue;
      }
      dtrans::TypeInfo *TI = DTInfo->getTypeInfo(DTransTy);
      if (!TI) {
        LLVM_DEBUG(dbgs() << "No type info found for: " << TypeName << "\n");
        continue;
      }
      dtrans::StructInfo *SI = cast<dtrans::StructInfo>(TI);
      StructsToConvert.push_back(SI);
    }
    return;
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // Build the list of candidate structures based on the structure safety data
  // and heuristics.
  for (dtrans::TypeInfo *TI : DTInfo->type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    // Don't try to delete fields from literal structures.
    if (cast<StructType>(StInfo->getLLVMType())->isLiteral())
      continue;

    LLVM_DEBUG(dbgs() << "LLVM Type: ";
               StInfo->getLLVMType()->print(dbgs(), true, true);
               dbgs() << "\n");

    size_t NumFields = StInfo->getNumFields();
    size_t NumFieldsDeleted = 0;
    uint64_t DeleteableBytes = 0;
    for (size_t i = 0; i < NumFields; ++i) {
      dtrans::FieldInfo &FI = StInfo->getField(i);
      if (canDeleteField(FI)) {
        LLVM_DEBUG({
          dbgs() << "  Can delete field: ";
          StInfo->getLLVMType()->print(dbgs(), true, true);
          dbgs() << " @ " << i << "\n";
        });

        // Get the field size in bytes. If the field is an i1 this will
        // return zero, but that keeps us from overestimating the size of
        // bitfields.
        DeleteableBytes += DL.getTypeSizeInBits(FI.getLLVMType()) / 8;
        ++NumFieldsDeleted;
      }
    }

    if (NumFieldsDeleted == 0)
      continue;

    if (DTInfo->testSafetyData(TI, dtrans::DT_DeleteField)) {
      LLVM_DEBUG({
        dbgs() << "  Rejecting ";
        StInfo->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " based on safety data.\n";
      });
      continue;
    }

    // In this case we really ought to be able to eliminate the type
    // completely, but that's more work than we want to do right now.
    if (NumFields == NumFieldsDeleted) {
      LLVM_DEBUG({
        dbgs() << "  Rejecting ";
        StInfo->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " because all fields would be deleted.\n";
      });
      continue;
    }

    // Reject type if deletable bytes < 1/8%.
    if ((800 * DeleteableBytes) < DL.getTypeAllocSize(StInfo->getLLVMType())) {
      LLVM_DEBUG({
        dbgs() << "  Rejecting ";
        StInfo->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " based on size.\n";
        dbgs() << "  Bytes to delete: " << DeleteableBytes << "\n";
        dbgs() << "  Struct size: "
               << DL.getTypeAllocSize(StInfo->getLLVMType()) << "\n";
      });
      continue;
    }

    StructsToConvert.push_back(StInfo);
  }
}

void DeleteFieldOPImpl::pruneCandidates() {
  LLVM_DEBUG(dbgs() << "Delete field: pruning candidate structures.\n");

  pruneCandidatesForTriaging();
  pruneCandidatesByParentSafety();

  // TODO: We need to extend delete fields for working with base and padded
  // structures. The basic idea is to make sure that both structures pass
  // the safety tests, and the fields selected for deletion can be removed
  // in both cases.
}

void DeleteFieldOPImpl::pruneCandidatesForTriaging() {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Check for triage limits on the number of structures to potentially modify.
  //
  // Note: This is done before checking whether any of the identified structures
  // should be skipped due to being nested in structures that cannot be changed.
  // This is necessary because otherwise we would need to re-identify the
  // OrigEnclosingTypes set after pruning the StructsToConvert based on the
  // triage limit.
  if (DeleteFieldMaxStruct != std::numeric_limits<unsigned>::max() &&
      StructsToConvert.size() > DeleteFieldMaxStruct) {
    // The set of structures that may be deleted from have been identified,
    // but the order of evaluation is not deterministic because the walk
    // of the type_info_entries is arbitrary. Sort the structures by name
    // so that triaging will be deterministic.
    LLVM_DEBUG(dbgs() << "Triaging: Reducing " << StructsToConvert.size()
                      << " candidates down to " << DeleteFieldMaxStruct
                      << " structures\n");
    std::sort(
        StructsToConvert.begin(), StructsToConvert.end(),
        [](dtrans::StructInfo *A, dtrans::StructInfo *B) {
          auto *AStTy = cast<StructType>(A->getLLVMType());
          auto *BStTy = cast<StructType>(B->getLLVMType());
          assert(
              AStTy->hasName() && BStTy->hasName() &&
              "Field deletion expected to be restricted to named structures");
          return AStTy->getName() < BStTy->getName();
        });

    LLVM_DEBUG({
      unsigned Count = 0;
      for (auto *StInfo : StructsToConvert) {
        dbgs() << Count << ":  Triaging will "
               << (Count >= DeleteFieldMaxStruct ? "NOT " : "") << "process: ";
        StInfo->getLLVMType()->print(dbgs(), true, true);
        dbgs() << "\n";
        ++Count;
      }
    });

    // Trim the vector to the number to transform based on the triage limit.
    StructsToConvert.resize(DeleteFieldMaxStruct);
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
}

// When a structure is nested within another structure, if may not be possible
// to delete fields from the structure because the structure it is nested within
// does not pass some safety checks that are required for adjusting the size of
// the parent structure. This routine will filter these structures out of the
// 'StructsToConvert' set.
void DeleteFieldOPImpl::pruneCandidatesByParentSafety() {
  DTransTypeToTypeSetMap TypeMap;
  buildTypeEnclosingMapping(TypeMap);

  SmallPtrSet<llvm::StructType *, 8> OrigEnclosingTypesTemp;
  SmallPtrSet<dtrans::StructInfo *, 4> ViolatingTIs;
  for (auto *StInfo : StructsToConvert) {
    auto *OrigDTransTy = cast<DTransStructType>(StInfo->getDTransType());
    auto It = TypeMap.find(OrigDTransTy);
    if (It == TypeMap.end())
      continue;

    for (auto *ParentTy : It->second) {
      auto *ParentStTy = dyn_cast<DTransStructType>(ParentTy);
      if (!ParentStTy)
        continue;

      auto *ParentTI =
          cast<dtrans::StructInfo>(DTInfo->getTypeInfo(ParentStTy));
      // Skip types that are already selected to be directly modified by the
      // transformation.
      if (std::find(StructsToConvert.begin(), StructsToConvert.end(),
                    ParentTI) != StructsToConvert.end())
        continue;

      // Test enclosing type for safety violations.
      if (!checkParentStructure(ParentTI)) {
        LLVM_DEBUG({
          dbgs() << "Rejecting ";
          StInfo->getLLVMType()->print(dbgs(), true, true);
          dbgs() << " based on safety data of enclosing type ";
          ParentTy->print(dbgs());
          dbgs() << "\n";
        });

        ViolatingTIs.insert(StInfo);
        break;
      }

      OrigEnclosingTypesTemp.insert(
          cast<llvm::StructType>(ParentStTy->getLLVMType()));
    }

    // Record enclosing types if none of them violate safety checks.
    if (!ViolatingTIs.count(StInfo))
      OrigEnclosingTypes.insert(OrigEnclosingTypesTemp.begin(),
                                OrigEnclosingTypesTemp.end());
  }

  if (ViolatingTIs.size() == StructsToConvert.size()) {
    StructsToConvert.clear();
    return;
  }

  // Remove rejected types from the 'StructsToConvert' set.
  StructsToConvert.erase(
      std::remove_if(StructsToConvert.begin(), StructsToConvert.end(),
                     [&ViolatingTIs](dtrans::StructInfo *SI) {
                       return ViolatingTIs.count(SI);
                     }),
      StructsToConvert.end());
}

// Identify dependent types. These are types that will not be changed directly,
// but their sizes could be changed due to a nested structure being transformed.
// These types need to be checked for safety flags which could prevent deletion
// of fields from types contained within them. For each type that has a
// dependency, populate 'TypeMap' with the list of affected types the type is
// contained within.
void DeleteFieldOPImpl::buildTypeEnclosingMapping(
    DTransTypeToTypeSetMap &TypeMap) {
  for (auto &KV : TypeToDirectDependentTypes) {
    DTransType *Ty = KV.first;
    PriorityWorklist<DTransType *> Worklist;
    Worklist.insert(KV.second);
    auto &CurSet = TypeMap[Ty];
    while (!Worklist.empty()) {
      DTransType *DepTy = Worklist.back();
      Worklist.pop_back();
      if (CurSet.insert(DepTy)) {
        // Add any direct dependents for DepTy to the worklist to form the
        // closure of all the dependent types.
        auto It = TypeToDirectDependentTypes.find(DepTy);
        if (It != TypeToDirectDependentTypes.end())
          Worklist.insert(It->second);
      }
    }
  }

  LLVM_DEBUG({
    dbgs() << "Type enclosing dependencies:\n";
    for (auto &KV : TypeMap) {
      KV.first->print(dbgs(), false);
      dbgs() << ":";
      for (auto *Ty : KV.second) {
        dbgs() << " ";
        Ty->print(dbgs(), false);
      }
      dbgs() << "\n";
    }
    dbgs() << "\n";
  });
}

// Check if there is any constraint in the structure that contains a candidate
// structure that prevents delete fields from being able to change the size of
// the parent structure. Return false if there is any constraint in the parent
// structure that prevents the delete fields optimization, else return true.
bool DeleteFieldOPImpl::checkParentStructure(dtrans::StructInfo *ParentStruct) {
  if (!ParentStruct)
    return false;

  // Go conservative if out of bounds is specified
  if (DTInfo->getDTransOutOfBoundsOK())
    return !(DTInfo->testSafetyData(ParentStruct, dtrans::DT_DeleteField));

  // Safety conditions in the enclosing structure that
  // prevents the optimization
  const dtrans::SafetyData DeleteFieldParent =
      dtrans::BadCasting | dtrans::BadAllocSizeArg |
      dtrans::BadPtrManipulation | dtrans::AmbiguousGEP | dtrans::VolatileData |
      dtrans::WholeStructureReference | dtrans::UnsafePointerStore |
      dtrans::BadMemFuncSize | dtrans::BadMemFuncManipulation |
      dtrans::AmbiguousPointerTarget | dtrans::UnsafePtrMerge |
      dtrans::AddressTaken | dtrans::NoFieldsInStruct | dtrans::SystemObject |
      dtrans::MismatchedArgUse | dtrans::HasVTable | dtrans::HasFnPtr |
      dtrans::HasZeroSizedArray | dtrans::HasFnPtr |
      dtrans::BadCastingConditional | dtrans::UnsafePointerStoreConditional;

  if (ParentStruct->testSafetyData(DeleteFieldParent))
    return false;

  // Check for safety issues in the fields
  unsigned Idx = 0;
  unsigned NumFields = ParentStruct->getNumFields();

  for (Idx = 0; Idx < NumFields; Idx++) {
    dtrans::FieldInfo &Field = ParentStruct->getField(Idx);
    DTransType *FieldTy = Field.getDTransType();
    dtrans::TypeInfo *FieldTI = DTInfo->getTypeInfo(FieldTy);
    assert(FieldTI && "Field TypeInfo must be created during analysis pass to "
                      "have safety data computed");

    // If the field is marked as address taken, then we need to make sure
    // that it passes the safety issues for delete fields.
    if (Field.isAddressTaken() &&
        DTInfo->testSafetyData(FieldTI, dtrans::DT_DeleteField))
      return false;
  }
  return true;
}

void DeleteFieldOPImpl::populateTypes(Module &M) {
  // Prepare a mapping for the dependent types. The base class created the
  // replacement types, but this class needs to have a mapping so that it can
  // determine when "sizeof" operations on those types need to be updated.
  for (auto *OrigParentTy : OrigEnclosingTypes) {
    auto *ReplParentTy =
        cast<llvm::StructType>(TypeRemapper.lookupTypeMapping(OrigParentTy));
    assert(ReplParentTy && "Parent type is not ready");
    OrigToNewTypeMapping[OrigParentTy] = ReplParentTy;
  }

  for (auto *StInfo : StructsToConvert) {
    auto *OrigLLVMTy = cast<StructType>(StInfo->getLLVMType());
    auto *NewLLVMTy = cast<StructType>(OrigToNewTypeMapping[OrigLLVMTy]);
    DTransStructType *OrigDTransTy =
        LLVMStructTypeToDTransStructType[OrigLLVMTy];
    DTransStructType *NewDTransTy = LLVMStructTypeToDTransStructType[NewLLVMTy];

    SmallVectorImpl<uint64_t> &NewIndices = FieldIdxMap[OrigLLVMTy];
    SmallVector<Type *, 8> LLVMDataTypes;
    SmallVector<DTransType *, 8> DTransDataTypes;
    size_t NumFields = StInfo->getNumFields();
    uint64_t NewIdx = 0;
    for (size_t i = 0; i < NumFields; ++i) {
      dtrans::FieldInfo &FI = StInfo->getField(i);
      if (!canDeleteField(FI)) {
        LLVM_DEBUG(dbgs() << OrigLLVMTy->getName() << "[" << i
                          << "] = " << NewIdx << "\n");
        NewIndices.push_back(NewIdx++);
        LLVMDataTypes.push_back(TypeRemapper.remapType(FI.getLLVMType()));

        DTransType *DTransFieldTy = OrigDTransTy->getFieldType(i);
        assert(DTransFieldTy && "DTransStructType is invalid");
        DTransDataTypes.push_back(TypeRemapper.remapType(DTransFieldTy));
      } else {
        LLVM_DEBUG(dbgs() << OrigLLVMTy->getName() << "[" << i
                          << "] = DELETED\n");
        NewIndices.push_back(FIELD_DELETED);
      }
    }

    NewLLVMTy->setBody(LLVMDataTypes, OrigLLVMTy->isPacked());
    NewDTransTy->setBody(DTransDataTypes);
    LLVM_DEBUG(dbgs() << "Delete field: New structure body: " << *NewLLVMTy
                      << "\nDTrans structure body:" << *NewDTransTy << "\n");
  }
}

GlobalVariable *
DeleteFieldOPImpl::createGlobalVariableReplacement(GlobalVariable *GV,
                                                   ValueMapper &Mapper) {
  Type *ValueType = GV->getValueType();

  // Let the base class handle replacing globals that are pointers. If the
  // original value type is a pointer, then the replacement type is also going
  // to be a pointer type, which can be handled by the base class.
  if (ValueType->isPointerTy())
    return nullptr;

  // If we aren't deleting fields from this type, let the base class handle it.
  if (!typeContainsDeletedFields(ValueType))
    return nullptr;

  Type *ReplValueTy = TypeRemapper.remapType(ValueType);
  assert(ValueType != ReplValueTy &&
         "createGlobalVariableReplacement called for unchanged type!");

  // Create and set the properties of the variable. The initialization of
  // the variable will not occur until all variables have been created
  // because there may be references to other variables being replaced in
  // the initializer list which have not been processed yet.
  GlobalVariable *NewGV = new GlobalVariable(
      *(GV->getParent()), ReplValueTy, GV->isConstant(), GV->getLinkage(),
      /*init=*/nullptr, GV->getName(),
      /*insertbefore=*/nullptr, GV->getThreadLocalMode(),
      GV->getType()->getAddressSpace(), GV->isExternallyInitialized());
  NewGV->setAlignment(MaybeAlign(GV->getAlignment()));
  NewGV->copyAttributesFrom(GV);
  NewGV->copyMetadata(GV, /*Offset=*/0);

  // The front-end may put DTrans metadata on Global variables that are
  // structure or arrays of structure types, even though those cases do not
  // strictly require it. In that case, the metadata needs to be updated to
  // reflect the type of the new variable based on the type mapping being done
  // because otherwise there could be a type left in the metadata that no longer
  // exists in the IR.
  remapDTransTypeMetadata(NewGV, Mapper);

  return NewGV;
}

// Check to see if we have deleted fields from this type at any level.
bool DeleteFieldOPImpl::typeContainsDeletedFields(llvm::Type *Ty) {
  if (!Ty->isAggregateType())
    return false;

  // For structure types, we need to consider types that directly have fields
  // being removed, and any container types that nest the type, which will have
  // to have their initializers rewritten.
  if (auto *StTy = dyn_cast<llvm::StructType>(Ty))
    return OrigToNewTypeMapping.count(StTy);
  if (Ty->isArrayTy())
    return typeContainsDeletedFields(Ty->getArrayElementType());
  llvm_unreachable("Unexpected aggregate type");
}

void DeleteFieldOPImpl::initializeGlobalVariableReplacement(
    GlobalVariable *OrigGV, GlobalVariable *NewGV, ValueMapper &Mapper) {
  Constant *OrigInit = OrigGV->getInitializer();
  NewGV->setInitializer(getReplacement(OrigInit, Mapper));
}

Constant *DeleteFieldOPImpl::getReplacement(Constant *Init,
                                            ValueMapper &Mapper) {
  if (auto *StInit = dyn_cast<ConstantStruct>(Init))
    return getStructReplacement(StInit, Mapper);
  if (auto *ArInit = dyn_cast<ConstantArray>(Init))
    return getArrayReplacement(ArInit, Mapper);
  return Mapper.mapConstant(*Init);
}

Constant *DeleteFieldOPImpl::getStructReplacement(ConstantStruct *StInit,
                                                  ValueMapper &Mapper) {
  llvm::StructType *OrigTy = StInit->getType();
  bool OuterType = OrigEnclosingTypes.count(OrigTy);

  // When traversing the fields of an outer type, a nested type that is not
  // changing may be encountered. In this case, don't walk the fields since
  // there will not be a field mapping. Since the nested type is not changing
  // the type remapper can handle it directly.
  if (!OuterType && !OrigToNewTypeMapping.count(OrigTy))
    return Mapper.mapConstant(*StInit);

  assert(OuterType ||
         FieldIdxMap.count(OrigTy) && "initializeGlobalVariableReplacement "
                                      "called for pointer-dependent type!");
  unsigned OrigNumFields = OrigTy->getStructNumElements();
  SmallVector<Constant *, 16> NewInitVals;
  for (unsigned Idx = 0; Idx < OrigNumFields; ++Idx)
    if (OuterType || FieldIdxMap[OrigTy][Idx] != FIELD_DELETED)
      NewInitVals.push_back(
          getReplacement(StInit->getAggregateElement(Idx), Mapper));
  auto *NewTy = OrigToNewTypeMapping[OrigTy];
  assert(NewTy->getStructNumElements() == NewInitVals.size() &&
         "Mismatched number of elements in struct initializer creation!");
  return ConstantStruct::get(cast<StructType>(NewTy), NewInitVals);
}

Constant *DeleteFieldOPImpl::getArrayReplacement(ConstantArray *ArInit,
                                                 ValueMapper &Mapper) {
  llvm::Type *OrigTy = ArInit->getType();
  unsigned OrigNumElements = OrigTy->getArrayNumElements();
  SmallVector<Constant *, 16> NewInitVals;
  for (unsigned Idx = 0; Idx < OrigNumElements; ++Idx)
    NewInitVals.push_back(
        getReplacement(ArInit->getAggregateElement(Idx), Mapper));
  Type *NewTy = TypeRemapper.remapType(OrigTy);
  assert(NewTy->getArrayNumElements() == NewInitVals.size() &&
         "Mismatched number of elements in array initializer creation!");
  return ConstantArray::get(cast<ArrayType>(NewTy), NewInitVals);
}

void DeleteFieldOPImpl::postprocessGlobalVariable(GlobalVariable *OrigGV,
                                                  GlobalVariable *NewGV) {
  // If we didn't delete fields from this type, don't do anything with the
  // global variable. This happens with dependent types.
  llvm::Type *OrigTy = OrigGV->getValueType();
  if (!typeContainsDeletedFields(OrigTy))
    return;

  llvm::Type *ReplTy = NewGV->getValueType();
  SmallVector<GEPOperator *, 4> GEPsToErase;

  for (auto *U : OrigGV->users()) {
    // Instructions will be processed elsewhere.
    if (isa<Instruction>(U))
      continue;

    if (auto *GEP = dyn_cast<GEPOperator>(U)) {
      assert(isa<ConstantExpr>(GEP) && "Expected constant GEP");
      assert(GEP->getOperand(0) == OrigGV && "Unexpected GV GEP use!");

      bool IsModified = false;
      bool AffectedStructure = false;
      bool IsPacked = false;

      SmallVector<Constant *, 8> OrigIndices;
      SmallVector<Constant *, 8> NewIndices;

      for (auto I = GEP->idx_begin(), E = GEP->idx_end(); I != E; ++I) {
        auto *Idx = cast<Constant>(*I);
        auto *NewIdx = Idx;

        if (!OrigIndices.empty()) {
          Type *IndexedType =
              GetElementPtrInst::getIndexedType(OrigTy, OrigIndices);
          assert(IndexedType && "Invalid type indexed");

          // Skip non-struct types
          if (auto *IndexedStTy = dyn_cast<llvm::StructType>(IndexedType)) {
            uint64_t FieldIdx = cast<ConstantInt>(Idx)->getLimitedValue();
            uint64_t NewFieldIdx = 0;

            // If the entry IndexedType in the FieldIdxMap map is empty,
            // then it means that the current structure (IndexedType) wasn't
            // modified, in this case we don't need to change the current
            // index (FieldIdx) for the GEP.
            if (FieldIdxMap[IndexedStTy].empty()) {
              NewFieldIdx = FieldIdx;
            } else {
              NewFieldIdx = FieldIdxMap[IndexedStTy][FieldIdx];
              AffectedStructure = true;
              IsPacked |= cast<llvm::StructType>(IndexedType)->isPacked();
            }

            // Although each operator GEP looks like it appears directly in an
            // instruction and therefore should have a single use, there is
            // actually only one instance of each unique set of indices for
            // the GEP because it is a constant.
            if (NewFieldIdx == FIELD_DELETED) {
              assert(I == std::prev(GEP->idx_end()) &&
                     "Unexpected removal of a GEP indexed into an aggregate");

              IsModified = false;
              GEPsToErase.push_back(GEP);
              break;
            }

            if (NewFieldIdx != FieldIdx) {
              IsModified = true;
              NewIdx = ConstantInt::get(Type::getInt32Ty(GEP->getContext()),
                                        NewFieldIdx);
            }
          }
        }

        NewIndices.push_back(NewIdx);
        OrigIndices.push_back(Idx);
      }

      GEPOperator *AffectedGEP = GEP;
      if (IsModified) {
        auto *NewGEP = ConstantExpr::getGetElementPtr(ReplTy, NewGV, NewIndices,
                                                      GEP->isInBounds());
        LLVM_DEBUG(dbgs() << "Delete field: Replacing GEP const expr: " << *GEP
                          << " with " << *NewGEP << "\n");
        // The functions have not been re-mapped yet at this point, so we
        // can just add an entry to the map that will replace the use of this
        // constant when the instruction is remapped.
        VMap[GEP] = NewGEP;
        AffectedGEP = cast<GEPOperator>(NewGEP);
      }

      if (AffectedStructure)
        dtrans::resetLoadStoreAlignment(AffectedGEP, DL, IsPacked);
    }
  }

  for_each(GEPsToErase, safeEraseValue);
}

// Before the functions are cloned, we need to find any GetElementPtr that
// is being used to access a deleted field (presumably for write purposes)
// and erase that GEP. If the GEP result is used by a store instruction
// we will also erase the store. Our safety checks should guarantee that
// there are no other uses of the GEP, so we'll assert that here.
void DeleteFieldOPImpl::processFunction(Function &F) {
  // When we delete GEPs, we will likely delete other instructions too,
  // so rather than delete them as we identify them we must keep a list
  // and delete everything after we've walked the function.
  SmallVector<GetElementPtrInst *, 4> GEPsToDelete;

  for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It) {
    switch (It->getOpcode()) {
    default:
      break;
    case Instruction::GetElementPtr: {
      auto *GEP = cast<GetElementPtrInst>(&*It);
      // If the GEP has a single index, it might be in the byte-flattened
      // form. Check that, and delete the field if necessary.
      // Otherwise, there's nothing more to do with this GEP.
      if (GEP->getNumIndices() == 1) {
        if (processPossibleByteFlattenedGEP(GEP))
          GEPsToDelete.push_back(GEP);
        continue;
      }

      if (processGEPInst(GEP, /*IsPreCloning=*/true))
        GEPsToDelete.push_back(GEP);
      break;
    }
    case Instruction::Sub:
      // Subtract instructions need to be processed prior to function
      // cloning because the pointer subtraction map to types is not kept
      // up-to-date for the cloned functions.
      processSubInst(cast<BinaryOperator>(&*It));
      break;
    }
  }

  for_each(GEPsToDelete, safeEraseValue);
}

// This function processes a single index GEP instruction to see if it is
// in the byte flattened form and accessing a field in a structure we are
// optimizing. If it is, this function will check the index of the field
// being accessed. If the field is being deleted, this function will return
// true, indicating that the caller should delete the GEP. If the field is
// not being deleted but its offset in the structure is changing, this function
// will update the GEP to reflect the new offset. Because the GEP is in
// byte-flattened form, the GEP can be updated prior to cloning.
bool DeleteFieldOPImpl::processPossibleByteFlattenedGEP(
    GetElementPtrInst *GEP) {
  auto InfoPair = DTInfo->getByteFlattenedGEPElement(GEP);
  if (!InfoPair.first)
    return false;

  // We'll typically only have a few types from which we are deleting
  // fields, so iterating over the map is a reasonable way to test
  // whether or not this GEP needs updating.
  llvm::Type *SrcTy = InfoPair.first->getLLVMType();
  for (auto &ONPair : OrigToNewTypeMapping) {
    llvm::StructType *OrigTy = ONPair.first;
    if (OrigTy != SrcTy)
      continue;

    llvm::Type *ReplTy = ONPair.second;
    uint64_t SrcIdx = InfoPair.second;
    uint64_t NewIdx = SrcIdx;

    // If it isn't an enclosing type then get a new index, otherwise update
    // offset using original index (SrcIdx).
    if (!OrigEnclosingTypes.count(OrigTy)) {
      assert(OrigTy->getStructNumElements() &&
             FieldIdxMap[OrigTy].size() > SrcIdx && "Unexpected GEP index");

      NewIdx = FieldIdxMap[OrigTy][SrcIdx];
      if (NewIdx == FIELD_DELETED)
        return true;

      // If the index isn't changing, we don't need to update or delete this
      // GEP.
      if (NewIdx == SrcIdx)
        return false;
    }

    // Otherwise, we need to get the offset of the updated index in the
    // replacement type.
    const StructLayout *SL = DL.getStructLayout(cast<StructType>(ReplTy));
    uint64_t NewOffset = SL->getElementOffset(NewIdx);
    Value *NewOffsetValue =
        ConstantInt::get(GEP->getOperand(1)->getType(), NewOffset);

    // Update GEP offset to match new ReplTy type.
    if (NewOffsetValue != GEP->getOperand(1)) {
      LLVM_DEBUG(dbgs() << "Delete field: Replacing instruction\n"
                        << *GEP << "\n");
      GEP->setOperand(1, NewOffsetValue);
      LLVM_DEBUG(dbgs() << "    with\n" << *GEP << "\n");
    }

    // We've updated this GEP and don't need to delete it.
    return false;
  }

  // This doesn't match any type we're updating.
  return false;
}

void DeleteFieldOPImpl::processSubInst(BinaryOperator *BinOp) {
  assert(BinOp->getOpcode() == Instruction::Sub &&
         "postProcessSubInst called for non-sub instruction!");
  DTransType *PtrSubTy = DTInfo->getResolvedPtrSubType(BinOp);
  if (!PtrSubTy)
    return;

  llvm::Type *PtrSubLLVMTy = PtrSubTy->getLLVMType();
  for (auto &ONPair : OrigToNewTypeMapping) {
    llvm::Type *OrigTy = ONPair.first;
    if (PtrSubLLVMTy != OrigTy)
      continue;

    // Update all users that divide this result by the structure size.
    llvm::Type *ReplTy = ONPair.second;
    llvm::dtrans::updatePtrSubDivUserSizeOperand(BinOp, OrigTy, ReplTy, DL);
  }
}

// In the pre-cloning case, the function will return true if the field being
// accessed should be deleted or false if not.
//
// In the post-cloning case, the function updates \p GEP instruction indices and
// returns true if they were modified.
bool DeleteFieldOPImpl::processGEPInst(GetElementPtrInst *GEP,
                                       bool IsPreCloning) {
  bool Modified = false;
  bool AffectedStructure = false;
  bool IsPacked = false;

  SmallVector<Value *, 8> IdxValues;
  for (auto I = GEP->idx_begin(), E = GEP->idx_end(); I != E; ++I) {
    Value *IdxValue = *I;

    uint64_t NewIndex;
    if (processGEPIndex(GEP, IdxValues, IdxValue, NewIndex, AffectedStructure,
                        IsPacked, IsPreCloning)) {
      if (IsPreCloning) {
        // Return true indicating that GEP should be removed.
        assert((I == std::prev(GEP->idx_end())) &&
               "Unexpected removal of a GEP indexed into an aggregate");
        return true;
      }

      if (!Modified)
        LLVM_DEBUG(dbgs() << "Delete field: Replacing instruction\n"
                          << *GEP << "\n");

      IdxValue =
          ConstantInt::get(Type::getInt32Ty(GEP->getContext()), NewIndex);

      GEP->setOperand(IdxValues.size() + 1, IdxValue);
      Modified = true;
    }

    IdxValues.push_back(IdxValue);
  }

  if (Modified)
    LLVM_DEBUG(dbgs() << "    with\n" << *GEP << "\n");

  if (AffectedStructure)
    dtrans::resetLoadStoreAlignment(cast<GEPOperator>(GEP), DL, IsPacked);
  return Modified;
}

// This function processes a GetElementPtr instruction to determine
// whether it needs to be updated based on our type re-mapping.
// When we are processing IR before function cloning, this function
// is called to test whether the GEP is accessing a deleted field.
// When we are processing IR after function cloning, this function is
// called to determine whether the GEP is accessing a type from which
// fields have been deleted and if so what its new index value should be.
//
// In the pre-cloning case, the function will return true if the field being
// accessed should be deleted or false if not.
//
// In the post-cloning case, the function will return true if the index
// argument should be updated and the \p NewIndex argument will be set to
// the new index value. If the index argument should not be updated the
// function will return false and the \p NewIndex argument will not be used.
//
// \p AffectedStructure and \p IsPacked will be updated if the GEP indexes a
// structure type that will have its size changed as a result of this
// transformation.
bool DeleteFieldOPImpl::processGEPIndex(GetElementPtrInst *GEP,
                                        ArrayRef<Value *> BaseIndices,
                                        Value *Idx, uint64_t &NewIndex,
                                        bool &AffectedStructure, bool &IsPacked,
                                        bool IsPreCloning) {
  if (BaseIndices.empty())
    return false;

  // If the GEP isn't accessing a structure, we can skip it.
  llvm::Type *IndexedTy = GetElementPtrInst::getIndexedType(
      GEP->getSourceElementType(), BaseIndices);

  assert(IndexedTy && "Invalid type indexed");

  if (!IndexedTy->isStructTy())
    return false;

  // We'll typically only have a few types from which we are deleting
  // fields, so iterating over the map is a reasonable way to test
  // whether or not this GEP needs updating.
  for (auto &ONPair : OrigToNewTypeMapping) {
    llvm::StructType *OrigTy = ONPair.first;
    llvm::StructType *ReplTy = ONPair.second;

    // Skip enclosing type, it isn't a type with deleted fields. However,
    // remember that it was encountered because alignments may change for
    // accesses due to the change in the nested type.
    if (OrigEnclosingTypes.count(OrigTy)) {
      AffectedStructure = true;
      IsPacked |= OrigTy->isPacked();
      continue;
    }

    // The original types should only be seen before cloning and the
    // replacement types should only be seen after cloning.
    assert((IsPreCloning || (OrigTy != IndexedTy)) &&
           "Pre-mapped type found in GEP post-cloning!");
    assert((!IsPreCloning || (ReplTy != IndexedTy)) &&
           "Mapped type found in GEP pre-cloning!");
    // Pre-cloning, we are looking for matches to the original type.
    if (IsPreCloning && OrigTy != IndexedTy)
      continue;
    // Post-cloning, we are looking for matches to the replacement type.
    if (!IsPreCloning && ReplTy != IndexedTy)
      continue;

    // Get the value of the Idx argument. The safety conditions
    // guarantee that this index is a constant.
    uint64_t GEPIdx = cast<ConstantInt>(Idx)->getLimitedValue();
    assert(OrigTy->getStructNumElements() &&
           FieldIdxMap[OrigTy].size() > GEPIdx && "Unexpected GEP index");

    // The GEP instruction would have its index in terms of the original
    // type. Get the corresponding index in the new type.
    uint64_t NewIdx = FieldIdxMap[OrigTy][GEPIdx];
    AffectedStructure = true;
    IsPacked |= cast<StructType>(OrigTy)->isPacked();

    // In the pre-cloning case, we only need to know whether the field was
    // deleted or not.
    if (IsPreCloning)
      return (NewIdx == FIELD_DELETED);

    // If this call is post-cloning, all access to deleted fields should
    // have already been removed.
    assert(NewIdx != FIELD_DELETED &&
           "Deleted field access found after cloning!");

    // If the index was changed, it should be because fields were deleted in
    // front of this field, so the new index should be smaller.
    assert(NewIdx <= GEPIdx && "Unexpected mapped field index!");
    NewIndex = NewIdx;
    return NewIdx != GEPIdx;
  }

  // If we get here, this isn't a type with deleted fields.
  return false;
}

void DeleteFieldOPImpl::postprocessFunction(Function &OrigFunc, bool isCloned) {
  Function *F = isCloned ? OrigFuncToCloneFuncMap[&OrigFunc] : &OrigFunc;
  for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It) {
    switch (It->getOpcode()) {
    default:
      break;
    case Instruction::GetElementPtr:
      processGEPInst(cast<GetElementPtrInst>(&*It), /*IsPreCloning=*/false);
      break;
    case Instruction::Call:
    case Instruction::Invoke:
      postprocessCall(cast<CallBase>(&*It));
      break;
    }
  }
}

void DeleteFieldOPImpl::postprocessCall(CallBase *Call) {
  auto CInfoVec =  DTInfo->getCallInfoVec(Call); 
  if (!CInfoVec || CInfoVec->size() != 1)
    return;
  auto *CInfo = DTInfo->getCallInfo(Call);
  assert(CInfo && "Expected unique CInfo");
  if (isa<dtrans::FreeCallInfo>(CInfo))
    return;

  // The number of types in the call element info and the number of types
  // in the OrigToNew type mapping should both be very small. We can use
  // the element_llvm_types here that returns llvm::Type objects, instead of
  // DTransTypes because the call info objects track the pointee type.
  auto CallElemTypes = CInfo->getElementTypesRef();
  for (auto *PointeeTy : CallElemTypes.element_llvm_types()) {
    // For an array of elements, identify the element type to see if it is
    // being transformed, and will require updating the size argument to be a
    // multiple of the new size.
    while (PointeeTy->isArrayTy())
      PointeeTy = PointeeTy->getArrayElementType();

    for (auto &ONPair : OrigToNewTypeMapping) {
      llvm::Type *OrigTy = ONPair.first;
      llvm::Type *ReplTy = ONPair.second;
      assert(PointeeTy != OrigTy &&
             "Original type found after type replacement!");
      if (PointeeTy != ReplTy)
        continue;

      // Do not adjust size if it's an incomplete access memfunc.
      // Fields within a partial memfunc range are not going to be removed.
      if (auto *MInfo = dyn_cast<dtrans::MemfuncCallInfo>(CInfo))
        if (!MInfo->getIsCompleteAggregate(0))
          continue;

      LLVM_DEBUG(dbgs() << "Found call involving type with deleted fields:\n"
                        << *Call << "\n"
                        << "  " << *OrigTy << "\n");
      const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
      dtrans::updateCallSizeOperand(Call, CInfo, OrigTy, ReplTy, TLI);
    }
  }
}

char DTransDeleteFieldOPWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransDeleteFieldOPWrapper, "dtrans-deletefieldop",
                      "DTrans delete field with opaque pointer support", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DTransSafetyAnalyzerWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransDeleteFieldOPWrapper, "dtrans-deletefieldop",
                    "DTrans delete field with opaque pointer support", false,
                    false)

ModulePass *llvm::createDTransDeleteFieldOPWrapperPass() {
  return new DTransDeleteFieldOPWrapper();
}

PreservedAnalyses dtransOP::DeleteFieldOPPass::run(Module &M,
                                                   ModuleAnalysisManager &AM) {
  DTransSafetyInfo *DTInfo = &AM.getResult<DTransSafetyAnalyzer>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  bool Changed = runImpl(M, DTInfo, WPInfo, GetTLI);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

bool dtransOP::DeleteFieldOPPass::runImpl(Module &M, DTransSafetyInfo *DTInfo,
                                          WholeProgramInfo &WPInfo,
                                          GetTLIFn GetTLI) {
  if (!DTInfo->useDTransSafetyAnalysis()) {
    LLVM_DEBUG(dbgs() << "  DTransSafetyAnalyzer results not available\n");
    return false;
  }

  DeleteFieldOPImpl Transformer(M.getContext(), DTInfo, "__DFDT_",
                                M.getDataLayout(), GetTLI);
  return Transformer.run(M);
}
