//==== DeleteFieldOP.cpp - Delete field with support for opaque pointers ====//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
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
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"
#include "llvm/ADT/PriorityWorklist.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

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
  GlobalVariable *createGlobalVariableReplacement(GlobalVariable *GV) override;
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

  const DataLayout &DL;

  // TODO: Remove 'public' specifier when function call size argument processing
  // code is added. Temporarily, declared as public to prevent build warning
  // about unused private variable.
public:
  DeleteFieldOPPass::GetTLIFn GetTLI;
private:
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

bool DeleteFieldOPImpl::prepareTypes(Module &M) {
  selectCandidates();
  pruneCandidates();

  if (StructsToConvert.empty()) {
    LLVM_DEBUG(dbgs() << "  No candidates found.\n");
    return false;
  }

  for (auto *StInfo : StructsToConvert) {
    LLVM_DEBUG(dbgs() << "  Selected for deletion: " << *StInfo->getDTransType()
                      << "\n";);

    // TODO: Create the opaque structures for the new types.
    (void)StInfo;
  }

  return false;
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
  // TODO: Populate the structure bodies for the new types.
}
GlobalVariable *
DeleteFieldOPImpl::createGlobalVariableReplacement(GlobalVariable *GV) {
  // TODO: Check if the global variable needs to be changed.
  return nullptr;
}
void DeleteFieldOPImpl::initializeGlobalVariableReplacement(
    GlobalVariable *OrigGV, GlobalVariable *NewGV, ValueMapper &Mapper) {
  // TODO: Set the initializer for a replaced global variable
}

void DeleteFieldOPImpl::postprocessGlobalVariable(GlobalVariable *OrigGV,
                                                  GlobalVariable *NewGV) {
  // TODO: Process all the GEPOperators that use a global variable that is
  // being changed
}

void DeleteFieldOPImpl::processFunction(Function &F) {
  // TODO: Update instructions in the function before type remapping occurs
}

void DeleteFieldOPImpl::postprocessFunction(Function &OrigFunc, bool isCloned) {
  // TODO: Update instructions in the function that need to be proceed after
  // type remapping
}

char DTransDeleteFieldOPWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransDeleteFieldOPWrapper, "dtrans-deletefieldop",
                      "DTrans delete field", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransSafetyAnalyzerWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransDeleteFieldOPWrapper, "dtrans-deletefieldop",
                    "DTrans delete field", false, false)

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
  DeleteFieldOPImpl Transformer(M.getContext(), DTInfo, "__DFDT_",
                                M.getDataLayout(), GetTLI);
  return Transformer.run(M);
}
