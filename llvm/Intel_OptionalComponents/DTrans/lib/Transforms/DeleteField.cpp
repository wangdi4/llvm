//===---------------- DeleteField.cpp - DTransDeleteFieldPass -------------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans delete field optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DeleteField.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOptBase.h"
#include "Intel_DTrans/Transforms/DTransOptUtils.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;

#define DEBUG_TYPE "dtrans-deletefield"

namespace {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// For triaging this transformation, define an upper limit on the number of
// candidate structures that will be allowed to have fields deleted.
cl::opt<unsigned>
    DeleteFieldMaxStruct("dtrans-deletefield-max-struct",
                         cl::init(std::numeric_limits<unsigned>::max()),
                         cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

class DTransDeleteFieldWrapper : public ModulePass {
private:
  dtrans::DeleteFieldPass Impl;

public:
  static char ID;

  DTransDeleteFieldWrapper() : ModulePass(ID) {
    initializeDTransDeleteFieldWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    DTransAnalysisWrapper &DTAnalysisWrapper =
        getAnalysis<DTransAnalysisWrapper>();
    DTransAnalysisInfo &DTInfo = DTAnalysisWrapper.getDTransInfo(M);
    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };
    WholeProgramInfo &WPInfo =
        getAnalysis<WholeProgramWrapperPass>().getResult();
    bool Changed = Impl.runImpl(M, DTInfo, GetTLI, WPInfo);
    if (Changed)
      DTAnalysisWrapper.setInvalidated();
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // TODO: Mark the actual required and preserved analyses.
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<DTransAnalysisWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

class DeleteFieldImpl : public DTransOptBase {
public:
  DeleteFieldImpl(
      DTransAnalysisInfo &DTInfo, LLVMContext &Context, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      StringRef DepTypePrefix, DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(&DTInfo, Context, DL, GetTLI, DepTypePrefix,
                      TypeRemapper) {}

  bool prepareTypes(Module &M) override;
  void populateTypes(Module &M) override;
  void processFunction(Function &F) override;
  void postprocessFunction(Function &OrigFunc, bool isCloned) override;

  GlobalVariable *createGlobalVariableReplacement(GlobalVariable *GV) override;
  void initializeGlobalVariableReplacement(GlobalVariable *OrigGV,
                                           GlobalVariable *NewGV,
                                           ValueMapper &Mapper) override;
  void postprocessGlobalVariable(GlobalVariable *OrigGV,
                                 GlobalVariable *NewGV) override;

private:
  // The pointers in this vector are owned by the DTransAnalysisInfo.
  // The list is populated during prepareTypes() and used in populateTypes().
  SmallVector<dtrans::StructInfo *, 4> StructsToConvert;

  // The list of original enclosing types to update their sizes.
  SmallPtrSet<Type *, 4> OrigEnclosingTypes;

  const uint64_t FIELD_DELETED = ~0ULL;

  // A mapping from the original structure type to the new structure type
  TypeToTypeMap OrigToNewTypeMapping;

  // A mapping from original types to a vector which can be used to lookup
  // the replacement index based on the original index. A replacement index
  // value of FIELD_DELETED indicates that the field was deleted.
  DenseMap<llvm::Type *, SmallVector<uint64_t, 16>> FieldIdxMap;

  bool typeContainsDeletedFields(llvm::Type *Ty);

  // Check if there is any constraint in the enclosed structure that prevents
  // delete fields
  bool checkParentStructure(dtrans::StructInfo *ParentStruct);

  Constant *getStructReplacement(ConstantStruct *StInit, ValueMapper &Mapper);
  Constant *getArrayReplacement(ConstantArray *ArInit, ValueMapper &Mapper);
  Constant *getReplacement(Constant *Init, ValueMapper &Mapper);

  bool processGEPIndex(GetElementPtrInst *GEP, ArrayRef<Value *> BaseIndices,
                      Value *Idx, uint64_t &NewIndex, bool IsPreCloning);
  bool processGEPInst(GetElementPtrInst *GEP, bool IsPreCloning);
  bool processPossibleByteFlattenedGEP(GetElementPtrInst *GEP);
  void postprocessCall(CallBase *Call);
  void processSubInst(Instruction *I);
};

} // end anonymous namespace

char DTransDeleteFieldWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransDeleteFieldWrapper, "dtrans-deletefield",
                      "DTrans delete field", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransDeleteFieldWrapper, "dtrans-deletefield",
                    "DTrans delete field", false, false)

ModulePass *llvm::createDTransDeleteFieldWrapperPass() {
  return new DTransDeleteFieldWrapper();
}

static bool canDeleteField(dtrans::FieldInfo &FI) {
  return (!FI.isRead() || FI.isValueUnused()) && !FI.hasComplexUse() &&
         !FI.getLLVMType()->isAggregateType();
}

// Return false if there is any constraint in the parent structure
// that prevents the delete fields optimization, else return true.
bool DeleteFieldImpl::checkParentStructure(dtrans::StructInfo *ParentStruct) {

  if (!ParentStruct)
    return false;

  // Go conservative if out of bounds is specified
  if (DTInfo->getDTransOutOfBoundsOK())
    return !(DTInfo->testSafetyData(ParentStruct, dtrans::DT_DeleteField));

  // Safety conditions in the enclosing structure that
  // prevents the optimization
  const dtrans::SafetyData DeleteFieldParent =
      dtrans::BadCasting | dtrans::BadAllocSizeArg |
      dtrans::BadPtrManipulation | dtrans::AmbiguousGEP |
      dtrans::VolatileData | dtrans::WholeStructureReference |
      dtrans::UnsafePointerStore | dtrans::BadMemFuncSize |
      dtrans::BadMemFuncManipulation | dtrans::AmbiguousPointerTarget |
      dtrans::UnsafePtrMerge | dtrans::AddressTaken |
      dtrans::NoFieldsInStruct | dtrans::SystemObject |
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
    llvm::Type *FieldTy = Field.getLLVMType();
    dtrans::TypeInfo *FieldTI = DTInfo->getOrCreateTypeInfo(FieldTy);

    // If the field is marked as address taken, then we need to make sure
    // that it passes the safety issues for delete fields.
    if (Field.isAddressTaken() &&
        DTInfo->testSafetyData(FieldTI, dtrans::DT_DeleteField))
      return false;
  }

  return true;
}

bool DeleteFieldImpl::prepareTypes(Module &M) {
  LLVM_DEBUG(dbgs() << "Delete field: looking for candidate structures.\n");

  for (dtrans::TypeInfo *TI : DTInfo->type_info_entries()) {
    uint64_t DeleteableBytes = 0;

    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    // Don't try to delete fields from literal structures.
    if (cast<StructType>(StInfo->getLLVMType())->isLiteral())
      continue;

    LLVM_DEBUG(dbgs() << "LLVM Type: ";
               StInfo->getLLVMType()->print(dbgs(), true, true);
               dbgs() << "\n");

    // We're only interested in fields that are never read or their value is
    // unused. Fields that are written but not read can be deleted. Fields with
    // complex uses (phi, select, icmp, memfuncs, etc.) cannot be deleted.
    bool CanDeleteField = false;
    size_t NumFields = StInfo->getNumFields();
    size_t NumFieldsDeleted = 0;
    for (size_t i = 0; i < NumFields; ++i) {
      dtrans::FieldInfo &FI = StInfo->getField(i);
      if (canDeleteField(FI)) {
        auto *FieldTy = FI.getLLVMType();

        LLVM_DEBUG({
          dbgs() << "  Can delete field: ";
          StInfo->getLLVMType()->print(dbgs(), true, true);
          dbgs() << " @ " << i << "\n";
        });

        // Get the field size in bytes. If the field is an i1 this will
        // return zero, but that keeps us from overestimating the size of
        // bitfields.
        DeleteableBytes += DL.getTypeSizeInBits(FieldTy) / 8;
        CanDeleteField = true;
        ++NumFieldsDeleted;
      }
    }

    if (!CanDeleteField)
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
    // so that triaging can be applied.
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

  // Scan for dependent types, they will not be changed directly, but their
  // sizes could be updated. Also collect types whose dependencies violate
  // safety checks.
  SmallPtrSet<dtrans::StructInfo *, 4> ViolatingTIs;
  for (auto *StInfo : StructsToConvert) {
    StructType *OrigTy = cast<StructType>(StInfo->getLLVMType());

    SmallPtrSet<Type *, 8> OrigEnclosingTypesTemp;
    for (Type *ParentTy : getEnclosingTypes(OrigTy)) {
      // Skip non-struct types - arrays.
      if (!isa<llvm::StructType>(ParentTy))
        continue;

      auto *ParentTI = cast<dtrans::StructInfo>(DTInfo->getTypeInfo(ParentTy));

      // Skip types that are already considered.
      if (std::find(StructsToConvert.begin(), StructsToConvert.end(),
                    ParentTI) != StructsToConvert.end())
        continue;

      // Test enclosing type for safety violations.
      if (!checkParentStructure(ParentTI)) {
        LLVM_DEBUG({
          dbgs() << "Rejecting ";
          StInfo->getLLVMType()->print(dbgs(), true, true);
          dbgs() << " based on safety data of enclosing type ";
          ParentTy->print(dbgs(), true, true);
          dbgs() << "\n";
        });

        ViolatingTIs.insert(StInfo);
        break;
      }

      OrigEnclosingTypesTemp.insert(ParentTy);
    }

    // Record enclosing types if none of them violate safety checks.
    if (!ViolatingTIs.count(StInfo))
      OrigEnclosingTypes.insert(OrigEnclosingTypesTemp.begin(),
                                OrigEnclosingTypesTemp.end());
  }

  if (StructsToConvert.empty() ||
      ViolatingTIs.size() == StructsToConvert.size()) {
    LLVM_DEBUG(dbgs() << "  No candidates found.\n");
    return false;
  }

  // Remove rejected types whose dependencies violate safety checks.
  StructsToConvert.erase(
      std::remove_if(StructsToConvert.begin(), StructsToConvert.end(),
                     [&ViolatingTIs](dtrans::StructInfo *SI) {
                       return ViolatingTIs.count(SI);
                     }),
      StructsToConvert.end());

  LLVMContext &Context = M.getContext();
  for (auto *StInfo : StructsToConvert) {
    LLVM_DEBUG({
      dbgs() << "  Selected for deletion: ";
      StInfo->getLLVMType()->print(dbgs(), true, true);
      dbgs() << "\n";
    });

    // Create an Opaque type as a placeholder, until the base class has
    // computed all the types that need to be created.
    StructType *OrigTy = cast<StructType>(StInfo->getLLVMType());
    StructType *NewStructTy = StructType::create(
        Context, (Twine("__DFT_" + OrigTy->getName()).str()));
    TypeRemapper->addTypeMapping(OrigTy, NewStructTy);
    OrigToNewTypeMapping[OrigTy] = NewStructTy;
  }

  return true;
}

void DeleteFieldImpl::populateTypes(Module &M) {
  // Prepare mapping for the dependent types.
  for (Type *OrigParentTy : OrigEnclosingTypes) {
    Type *ReplParentTy = TypeRemapper->lookupTypeMapping(OrigParentTy);
    assert(ReplParentTy && "Parent type is not ready");
    OrigToNewTypeMapping[OrigParentTy] = ReplParentTy;
  }

  for (auto *StInfo : StructsToConvert) {
    auto *OrigTy = cast<StructType>(StInfo->getLLVMType());
    auto *NewTy = cast<StructType>(OrigToNewTypeMapping[OrigTy]);

    SmallVectorImpl<uint64_t> &NewIndices = FieldIdxMap[OrigTy];
    SmallVector<Type *, 8> DataTypes;
    size_t NumFields = StInfo->getNumFields();
    uint64_t NewIdx = 0;
    for (size_t i = 0; i < NumFields; ++i) {
      dtrans::FieldInfo &FI = StInfo->getField(i);
      if (!canDeleteField(FI)) {
        LLVM_DEBUG(dbgs() << OrigTy->getName() << "[" << i << "] = " << NewIdx
                          << "\n");
        NewIndices.push_back(NewIdx++);
        DataTypes.push_back(TypeRemapper->remapType(FI.getLLVMType()));
      } else {
        LLVM_DEBUG(dbgs() << OrigTy->getName() << "[" << i << "] = DELETED\n");
        NewIndices.push_back(FIELD_DELETED);
      }
    }
    NewTy->setBody(DataTypes, OrigTy->isPacked());
    LLVM_DEBUG(dbgs() << "Delete field: New structure body: " << *NewTy
                      << "\n");
  }
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

// Before the functions are cloned, we need to find any GetElementPtr that
// is being used to access a deleted field (presumably for write purposes)
// and erase that GEP. If the GEP result is used by a store instruction
// we will also erase the store. Our safety checks should guarantee that
// there are no other uses of the GEP, so we'll assert that here.
void DeleteFieldImpl::processFunction(Function &F) {
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
      processSubInst(&*It);
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
bool DeleteFieldImpl::processPossibleByteFlattenedGEP(GetElementPtrInst *GEP) {
  auto InfoPair = DTInfo->getByteFlattenedGEPElement(GEP);
  if (!InfoPair.first)
    return false;

  // We'll typically only have a few types from which we are deleting
  // fields, so iterating over the map is a reasonable way to test
  // whether or not this GEP needs updating.
  llvm::Type *SrcTy = InfoPair.first;
  for (auto &ONPair : OrigToNewTypeMapping) {
    llvm::Type *OrigTy = ONPair.first;
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
bool DeleteFieldImpl::processGEPIndex(GetElementPtrInst *GEP,
                                      ArrayRef<Value *> BaseIndices, Value *Idx,
                                      uint64_t &NewIndex, bool IsPreCloning) {
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
    llvm::Type *OrigTy = ONPair.first;
    llvm::Type *ReplTy = ONPair.second;

    // Skip enclosing type, it isn't a type with deleted fields.
    if (OrigEnclosingTypes.count(OrigTy))
      continue;

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

// In the pre-cloning case, the function will return true if the field being
// accessed should be deleted or false if not.
//
// In the post-cloning case, the function updates \p GEP instruction indices and
// returns true if they were modified.
bool DeleteFieldImpl::processGEPInst(GetElementPtrInst *GEP,
                                     bool IsPreCloning) {
  bool Modified = false;
  SmallVector<Value *, 8> IdxValues;
  for (auto I = GEP->idx_begin(), E = GEP->idx_end(); I != E; ++I) {
    Value *IdxValue = *I;

    uint64_t NewIndex;
    if (processGEPIndex(GEP, IdxValues, IdxValue, NewIndex, IsPreCloning)) {
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

  return Modified;
}

// After functions are cloned, we need to find any GEP instructions that are
// accessing elements whose index was changing by field deletion and any
// instructions that are referencing the structure size.
void DeleteFieldImpl::postprocessFunction(Function &OrigFunc, bool isCloned) {
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

void DeleteFieldImpl::postprocessCall(CallBase *Call) {
  auto *CInfo = DTInfo->getCallInfo(Call);
  if (!CInfo || isa<dtrans::FreeCallInfo>(CInfo))
    return;

  // The number of types in the pointer info and the number of types
  // in the OrigToNew type mapping should both be very small.
  auto &PtrInfo = CInfo->getPointerTypeInfoRef();
  for (auto *CallTy : PtrInfo.getTypes()) {
    if (!CallTy->isPointerTy())
      continue;

    auto *PointeeTy = CallTy->getPointerElementType();

    for (auto &ONPair : OrigToNewTypeMapping) {
      llvm::Type *OrigTy = ONPair.first;
      llvm::Type *ReplTy = ONPair.second;
      assert(PointeeTy != OrigTy &&
             "Original type found after type replacement!");
      if (PointeeTy != ReplTy)
        continue;

      // Do not adjust size if it's an incomplete access memfunc.
      // Fields within a partial memfunc range are not going to be removed.
      if (auto *MInfo = dyn_cast<dtrans::MemfuncCallInfo>(CInfo)) {
        if (!MInfo->getIsCompleteAggregate(0))
          continue;
      }

      LLVM_DEBUG(dbgs() << "Found call involving type with deleted fields:\n"
                        << *Call << "\n"
                        << "  " << *OrigTy << "\n");
      const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
      updateCallSizeOperand(Call, CInfo, OrigTy, ReplTy, TLI);
    }
  }
}

void DeleteFieldImpl::processSubInst(Instruction *I) {
  auto *BinOp = cast<BinaryOperator>(I);
  assert(BinOp->getOpcode() == Instruction::Sub &&
         "postProcessSubInst called for non-sub instruction!");
  llvm::Type *PtrSubTy = DTInfo->getResolvedPtrSubType(BinOp);
  if (!PtrSubTy)
    return;
  for (auto &ONPair : OrigToNewTypeMapping) {
    llvm::Type *OrigTy = ONPair.first;
    llvm::Type *ReplTy = ONPair.second;
    if (PtrSubTy != OrigTy)
      continue;
    // Call the base class to find and update all users that divide this result
    // by the structure size.
    llvm::dtrans::updatePtrSubDivUserSizeOperand(BinOp, OrigTy, ReplTy, DL);
  }
}

// Check to see if we have deleted fields from this type at any level.
bool DeleteFieldImpl::typeContainsDeletedFields(llvm::Type *Ty) {
  if (!Ty->isAggregateType())
    return false;
  if (Ty->isStructTy())
    return FieldIdxMap.count(Ty);
  if (Ty->isArrayTy())
    return typeContainsDeletedFields(Ty->getArrayElementType());
  llvm_unreachable("Unexpected aggregate type");
}

GlobalVariable *
DeleteFieldImpl::createGlobalVariableReplacement(GlobalVariable *GV) {
  Type *GVTy = GV->getType();

  // If we aren't deleting fields from this type, let the base class handle it.
  if (!typeContainsDeletedFields(GVTy->getPointerElementType()))
    return nullptr;

  Type *ReplTy = TypeRemapper->remapType(GVTy);

  assert(GVTy != ReplTy &&
         "createGlobalVariableReplacement called for unchanged type!");

  // Globals are always pointers, so the variable we want to create is
  // the element type of the pointer.
  Type *RemapType = ReplTy->getPointerElementType();

  // Let the base class handle replacing globals that are pointers.
  if (RemapType->isPointerTy())
    return nullptr;

  // Create and set the properties of the variable. The initialization of
  // the variable will not occur until all variables have been created
  // because there may be references to other variables being replaced in
  // the initializer list which have not been processed yet.
  GlobalVariable *NewGV = new GlobalVariable(
      *(GV->getParent()), RemapType, GV->isConstant(), GV->getLinkage(),
      /*init=*/nullptr, GV->getName(),
      /*insertbefore=*/nullptr, GV->getThreadLocalMode(),
      GV->getType()->getAddressSpace(), GV->isExternallyInitialized());
  NewGV->setAlignment(MaybeAlign(GV->getAlignment()));
  NewGV->copyAttributesFrom(GV);
  NewGV->copyMetadata(GV, /*Offset=*/0);

  return NewGV;
}

Constant *DeleteFieldImpl::getArrayReplacement(ConstantArray *ArInit,
                                               ValueMapper &Mapper) {
  llvm::Type *OrigTy = ArInit->getType();
  unsigned OrigNumElements = OrigTy->getArrayNumElements();
  SmallVector<Constant *, 16> NewInitVals;
  for (unsigned Idx = 0; Idx < OrigNumElements; ++Idx)
    NewInitVals.push_back(
        getReplacement(ArInit->getAggregateElement(Idx), Mapper));
  Type *NewTy = TypeRemapper->remapType(OrigTy);
  assert(NewTy->getArrayNumElements() == NewInitVals.size() &&
         "Mismatched number of elements in array initializer creation!");
  return ConstantArray::get(cast<ArrayType>(NewTy), NewInitVals);
}

Constant *DeleteFieldImpl::getStructReplacement(ConstantStruct *StInit,
                                                ValueMapper &Mapper) {
  llvm::Type *OrigTy = StInit->getType();
  assert(FieldIdxMap.count(OrigTy) &&
         "initializeGlobalVariableReplacement called for dependent type!");
  unsigned OrigNumFields = OrigTy->getStructNumElements();
  SmallVector<Constant *, 16> NewInitVals;
  for (unsigned Idx = 0; Idx < OrigNumFields; ++Idx)
    if (FieldIdxMap[OrigTy][Idx] != FIELD_DELETED)
      NewInitVals.push_back(
          getReplacement(StInit->getAggregateElement(Idx), Mapper));
  auto *NewTy = OrigToNewTypeMapping[OrigTy];
  assert(NewTy->getStructNumElements() == NewInitVals.size() &&
         "Mismatched number of elements in struct initializer creation!");
  return ConstantStruct::get(cast<StructType>(NewTy), NewInitVals);
}

Constant *DeleteFieldImpl::getReplacement(Constant *Init, ValueMapper &Mapper) {
  if (auto *StInit = dyn_cast<ConstantStruct>(Init))
    return getStructReplacement(StInit, Mapper);
  if (auto *ArInit = dyn_cast<ConstantArray>(Init))
    return getArrayReplacement(ArInit, Mapper);
  return Mapper.mapConstant(*Init);
}

void DeleteFieldImpl::initializeGlobalVariableReplacement(
    GlobalVariable *OrigGV, GlobalVariable *NewGV, ValueMapper &Mapper) {
  Constant *OrigInit = OrigGV->getInitializer();
  NewGV->setInitializer(getReplacement(OrigInit, Mapper));
}

void DeleteFieldImpl::postprocessGlobalVariable(GlobalVariable *OrigGV,
                                                GlobalVariable *NewGV) {
  // If we didn't delete fields from this type, don't do anything with the
  // global variable. This happens with dependent types.
  llvm::Type *OrigTy = OrigGV->getType()->getPointerElementType();
  if (!typeContainsDeletedFields(OrigTy))
    return;

  llvm::Type *ReplTy = NewGV->getType()->getPointerElementType();
  SmallVector<GEPOperator *, 4> GEPsToErase;
  for (auto *U : OrigGV->users()) {
    // Instructions will be processed elsewhere.
    if (isa<Instruction>(U))
      continue;

    if (auto *GEP = dyn_cast<GEPOperator>(U)) {
      assert(isa<ConstantExpr>(GEP) && "Expected constant GEP");
      assert(GEP->getOperand(0) == OrigGV && "Unexpected GV GEP use!");

      bool IsModified = false;

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
          if (IndexedType->isStructTy()) {
            uint64_t FieldIdx = cast<ConstantInt>(Idx)->getLimitedValue();
            uint64_t NewFieldIdx = FieldIdxMap[IndexedType][FieldIdx];

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

      if (IsModified) {
        auto *NewGEP = ConstantExpr::getGetElementPtr(ReplTy, NewGV, NewIndices,
                                                      GEP->isInBounds());
        LLVM_DEBUG(dbgs() << "Delete field: Replacing GEP const expr: " << *GEP
                          << " with " << *NewGEP << "\n");
        // The functions have not been re-mapped yet at this point, so we
        // can just add an entry to the map that will replace the use of this
        // constant when the instruction is remapped.
        VMap[GEP] = NewGEP;
      }
    }
  }

  for_each(GEPsToErase, safeEraseValue);
}

bool dtrans::DeleteFieldPass::runImpl(
    Module &M, DTransAnalysisInfo &DTInfo,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
    WholeProgramInfo &WPInfo) {

  if (!WPInfo.isWholeProgramSafe())
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  DTransTypeRemapper TypeRemapper;
  DeleteFieldImpl Transformer(DTInfo, M.getContext(), M.getDataLayout(), GetTLI,
                              "__DFDT_", &TypeRemapper);
  return Transformer.run(M);
}

PreservedAnalyses dtrans::DeleteFieldPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function*>(&F)));
  };
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  if (!runImpl(M, DTransInfo, GetTLI, WPInfo))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}
