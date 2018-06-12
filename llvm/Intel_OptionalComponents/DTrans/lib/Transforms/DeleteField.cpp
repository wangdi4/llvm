//===---------------- DeleteField.cpp - DTransDeleteFieldPass -------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;

#define DEBUG_TYPE "dtrans-deletefield"

namespace {

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
    DTransAnalysisInfo &DTInfo =
        getAnalysis<DTransAnalysisWrapper>().getDTransInfo();
    return Impl.runImpl(M, DTInfo);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // TODO: Mark the actual required and preserved analyses.
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

class DeleteFieldImpl : public DTransOptBase {
public:
  DeleteFieldImpl(DTransAnalysisInfo &DTInfo, LLVMContext &Context,
                  const DataLayout &DL, StringRef DepTypePrefix,
                  DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(DTInfo, Context, DL, DepTypePrefix, TypeRemapper) {}

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

  const uint64_t FIELD_DELETED = ~0ULL;

  // A mapping from the original structure type to the new structure type
  TypeToTypeMap OrigToNewTypeMapping;

  // A mapping from original types to a vector which can be used to lookup
  // the replacement index based on the original index. A replacement index
  // value of FIELD_DELETED indicates that the field was deleted.
  DenseMap<llvm::Type *, SmallVector<uint64_t, 16>> FieldIdxMap;

  bool processGEPInst(GetElementPtrInst *GEP, uint64_t &NewIndex,
                      bool IsPreCloning);
  bool processPossibleByteFlattenedGEP(GetElementPtrInst *GEP);
  void postprocessCallInst(Instruction *I);
  void processSubInst(Instruction *I);
};

} // end anonymous namespace

char DTransDeleteFieldWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransDeleteFieldWrapper, "dtrans-deletefield",
                      "DTrans delete field", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_END(DTransDeleteFieldWrapper, "dtrans-deletefield",
                    "DTrans delete field", false, false)

ModulePass *llvm::createDTransDeleteFieldWrapperPass() {
  return new DTransDeleteFieldWrapper();
}

bool DeleteFieldImpl::prepareTypes(Module &M) {
  // TODO: Create a safety mask for the conditions that are common to all
  //       DTrans optimizations.
  dtrans::SafetyData DeleteFieldSafetyConditions =
      dtrans::BadCasting | dtrans::BadAllocSizeArg |
      dtrans::BadPtrManipulation | dtrans::AmbiguousGEP | dtrans::VolatileData |
      dtrans::MismatchedElementAccess | dtrans::WholeStructureReference |
      dtrans::UnsafePointerStore | dtrans::FieldAddressTaken |
      dtrans::BadMemFuncSize | dtrans::BadMemFuncManipulation |
      dtrans::AmbiguousPointerTarget | dtrans::UnsafePtrMerge |
      dtrans::AddressTaken | dtrans::NoFieldsInStruct | dtrans::NestedStruct |
      dtrans::ContainsNestedStruct | dtrans::MemFuncPartialWrite |
      dtrans::SystemObject;

  LLVM_DEBUG(dbgs() << "Delete field: looking for candidate structures.\n");

  for (dtrans::TypeInfo *TI : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    // We're only interested in fields that are never read. Fields that are
    // written but not read can be deleted. Fields with complex uses
    // (phi, select, icmp, etc.) cannot be deleted.
    bool CanDeleteField = false;
    size_t NumFields = StInfo->getNumFields();
    for (size_t i = 0; i < NumFields; ++i) {
      dtrans::FieldInfo &FI = StInfo->getField(i);
      if (!FI.isRead() && !FI.hasComplexUse()) {
        LLVM_DEBUG({
          dbgs() << "  Found unread field: ";
          StInfo->getLLVMType()->print(dbgs(), true, true);
          dbgs() << " @ " << i << "\n";
        });
        CanDeleteField = true;
#ifdef NDEBUG
        break;
#endif // NDEBUG
      }
    }

    if (!CanDeleteField)
      continue;

    if (StInfo->testSafetyData(DeleteFieldSafetyConditions)) {
      LLVM_DEBUG({
        dbgs() << "  Rejecting ";
        StInfo->getLLVMType()->print(dbgs(), true, true);
        dbgs() << " based on safety data.\n";
      });
      continue;
    }

    LLVM_DEBUG({
      dbgs() << "  Selected for deletion: ";
      StInfo->getLLVMType()->print(dbgs(), true, true);
      dbgs() << "\n";
    });

    StructsToConvert.push_back(StInfo);
  }

  if (StructsToConvert.empty()) {
    LLVM_DEBUG(dbgs() << "  No candidates found.\n");
    return false;
  }

  LLVMContext &Context = M.getContext();
  for (auto *StInfo : StructsToConvert) {
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
  for (auto *StInfo : StructsToConvert) {
    auto *OrigTy = cast<StructType>(StInfo->getLLVMType());
    auto *NewTy = cast<StructType>(OrigToNewTypeMapping[OrigTy]);

    SmallVectorImpl<uint64_t> &NewIndices = FieldIdxMap[OrigTy];
    SmallVector<Type *, 8> DataTypes;
    size_t NumFields = StInfo->getNumFields();
    uint64_t NewIdx = 0;
    for (size_t i = 0; i < NumFields; ++i) {
      dtrans::FieldInfo &FI = StInfo->getField(i);
      if (FI.isRead() || FI.hasComplexUse()) {
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

      // Otherwise, check the normal form of the GEP to see if it should
      // be deleted.
      uint64_t Unused;
      if (processGEPInst(GEP, Unused, /*IsPreCloning=*/true))
        GEPsToDelete.push_back(GEP);
      break;
    }
    case Instruction::Sub:
      processSubInst(&*It);
      break;
    }
  }

  std::function<void(Value *)> deleteStoreAndCastUses =
      [&deleteStoreAndCastUses](Value *V) {
        SmallPtrSet<Instruction *, 4> InstsToDelete;
        for (auto *U : V->users()) {
          assert((isa<CastInst>(U) || isa<StoreInst>(U)) &&
                 "Unexpected use of deleted field!");
          // There may be a bitcast between the GEP and any store instruction.
          // If so, follow its uses and delete them (they must also be either
          // casts or stores) then delete the cast instruction.
          if (isa<CastInst>(U))
            deleteStoreAndCastUses(U);
          // Since this pointer is an iterator, we can't erase it here.
          InstsToDelete.insert(cast<Instruction>(U));
        }
        // The instructions we're deleting won't ever share users so
        // there's no reason to gather instructions to delete from the
        // recursive calls. We can delete each level's instructions as
        // we unwind.
        for (auto *I : InstsToDelete) {
          LLVM_DEBUG(dbgs() << "Delete field: erasing GEP user:\n"
                            << *I << "\n");
          I->eraseFromParent();
        }
      };

  for (auto *GEP : GEPsToDelete) {
    deleteStoreAndCastUses(GEP);
    LLVM_DEBUG(dbgs() << "Delete field: erasing GEP of deleted field:\n"
                      << *GEP << "\n");
    GEP->eraseFromParent();
  }
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
  auto InfoPair = DTInfo.getByteFlattenedGEPElement(GEP);
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
    assert(OrigTy->getStructNumElements() &&
           FieldIdxMap[OrigTy].size() > SrcIdx && "Unexpected GEP index");
    uint64_t NewIdx = FieldIdxMap[OrigTy][SrcIdx];
    if (NewIdx == FIELD_DELETED)
      return true;
    // If the index isn't changing, we don't need to update or delete this GEP.
    if (NewIdx == SrcIdx)
      return false;
    // Otherwise, we need to get the offset of the updated index in the
    // replacement type.
    const StructLayout *SL = DL.getStructLayout(cast<StructType>(ReplTy));
    uint64_t NewOffset = SL->getElementOffset(NewIdx);
    LLVM_DEBUG(dbgs() << "Delete field: Replacing instruction\n"
                      << *GEP << "\n");
    GEP->setOperand(1,
                    ConstantInt::get(GEP->getOperand(1)->getType(), NewOffset));
    LLVM_DEBUG(dbgs() << "    with\n" << *GEP << "\n");
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
// accessed should be deleted or false if not, and the \p NewIndex argument
// is ignored.
//
// In the post-cloning case, the function will return true if the index
// argument should be updated and the \p NewIndex argument will be set to
// the new index value. If the index argument should not be updated the
// function will return false and the \p NewIndex argument will not be used.
bool DeleteFieldImpl::processGEPInst(GetElementPtrInst *GEP, uint64_t &NewIndex,
                                     bool IsPreCloning) {
  // We don't optimize structures with nesting, and we don't need
  // to update GEPs that are indexing into a dynamic array of the
  // structure, so a quick check on the number of indices here may
  // avoid any further checking.
  if (GEP->getNumIndices() != 2)
    return false;

  // If the GEP isn't accessing a structure, we can skip it.
  llvm::Type *SrcTy = GEP->getSourceElementType();
  if (!SrcTy->isStructTy())
    return false;

  // We'll typically only have a few types from which we are deleting
  // fields, so iterating over the map is a reasonable way to test
  // whether or not this GEP needs updating.
  for (auto &ONPair : OrigToNewTypeMapping) {
    llvm::Type *OrigTy = ONPair.first;
    llvm::Type *ReplTy = ONPair.second;
    // The original types should only be seen before cloning and the
    // replacement types should only be seen after cloning.
    assert((IsPreCloning || (OrigTy != SrcTy)) &&
           "Pre-mapped type found in GEP post-cloning!");
    assert((!IsPreCloning || (ReplTy != SrcTy)) &&
           "Mapped type found in GEP pre-cloning!");
    // Pre-cloning, we are looking for matches to the original type.
    if (IsPreCloning && OrigTy != SrcTy)
      continue;
    // Post-cloning, we are looking for matches to the replacement type.
    if (!IsPreCloning && ReplTy != SrcTy)
      continue;

    // Get the value of the second index argument. The safety conditions
    // guarantee that this index is a constant.
    uint64_t GEPIdx = cast<ConstantInt>(GEP->getOperand(2))->getLimitedValue();
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

// After functions are cloned, we need to find any GEP instructions that are
// accessing elements whose index was changing by field deletion and any
// instructions that are referencing the structure size.
void DeleteFieldImpl::postprocessFunction(Function &OrigFunc, bool isCloned) {
  Function *F = isCloned ? OrigFuncToCloneFuncMap[&OrigFunc] : &OrigFunc;
  for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It) {
    switch (It->getOpcode()) {
    default:
      break;
    case Instruction::GetElementPtr: {
      auto *GEP = cast<GetElementPtrInst>(&*It);
      uint64_t NewIndex;
      if (processGEPInst(GEP, NewIndex, /*IsPreCloning=*/false)) {
        LLVM_DEBUG(dbgs() << "Delete field: Replacing instruction\n"
                          << *GEP << "\n");
        GEP->setOperand(
            2, ConstantInt::get(Type::getInt32Ty(GEP->getContext()), NewIndex));
        LLVM_DEBUG(dbgs() << "    with\n" << *GEP << "\n");
      }
    } break;
    case Instruction::Call:
      postprocessCallInst(&*It);
      break;
    }
  }
}

void DeleteFieldImpl::postprocessCallInst(Instruction *I) {
  auto *CInfo = DTInfo.getCallInfo(I);
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
      LLVM_DEBUG(dbgs() << "Found call involving type with deleted fields:\n"
                        << *I << "\n"
                        << "  " << *OrigTy << "\n");
      updateCallSizeOperand(I, CInfo, OrigTy, ReplTy);
    }
  }
}

void DeleteFieldImpl::processSubInst(Instruction *I) {
  auto *BinOp = cast<BinaryOperator>(I);
  assert(BinOp->getOpcode() == Instruction::Sub &&
         "postProcessSubInst called for non-sub instruction!");
  llvm::Type *PtrSubTy = DTInfo.getResolvedPtrSubType(BinOp);
  if (!PtrSubTy)
    return;
  for (auto &ONPair : OrigToNewTypeMapping) {
    llvm::Type *OrigTy = ONPair.first;
    llvm::Type *ReplTy = ONPair.second;
    if (PtrSubTy != OrigTy)
      continue;
    // Call the base class to find and update all users that divide this result
    // by the structure size.
    updatePtrSubDivUserSizeOperand(BinOp, OrigTy, ReplTy);
  }
}

GlobalVariable *
DeleteFieldImpl::createGlobalVariableReplacement(GlobalVariable *GV) {
  Type *GVTy = GV->getType();

  // If we aren't deleting fields from this type, let the base class handle it.
  if (!FieldIdxMap.count(GVTy->getPointerElementType()))
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
  NewGV->setAlignment(GV->getAlignment());
  NewGV->copyAttributesFrom(GV);
  NewGV->copyMetadata(GV, /*Offset=*/0);

  return NewGV;
}

void DeleteFieldImpl::initializeGlobalVariableReplacement(
    GlobalVariable *OrigGV, GlobalVariable *NewGV, ValueMapper &Mapper) {
  Constant *OrigInit = OrigGV->getInitializer();
  assert(
      (isa<ConstantAggregateZero>(OrigInit) || isa<ConstantStruct>(OrigInit)) &&
      "Unexpected global variable initializer!");
  // If the original initializer is a zero initializer just remap it.
  if (isa<ConstantAggregateZero>(OrigInit)) {
    NewGV->setInitializer(Mapper.mapConstant(*OrigGV->getInitializer()));
    return;
  }

  // Otherwise, we need to do an element by element copy.

  // Because globals are always pointers, we need the type it's pointing to.
  llvm::Type *OrigTy = OrigGV->getType()->getPointerElementType();

  assert(FieldIdxMap.count(OrigTy) &&
         "initializeGlobalVariableReplacement called for dependent type!");

  unsigned OrigNumFields = OrigTy->getStructNumElements();
  SmallVector<Constant *, 16> NewInitVals;
  for (unsigned Idx = 0; Idx < OrigNumFields; ++Idx)
    if (FieldIdxMap[OrigTy][Idx] != FIELD_DELETED)
      NewInitVals.push_back(
          Mapper.mapConstant(*OrigInit->getAggregateElement(Idx)));
  auto *NewTy = cast<StructType>(NewGV->getType()->getPointerElementType());
  assert(NewTy->getNumElements() == NewInitVals.size() &&
         "Mismatched number of elements in global initializer creation!");
  Constant *NewInit = ConstantStruct::get(NewTy, NewInitVals);
  NewGV->setInitializer(NewInit);
}

void DeleteFieldImpl::postprocessGlobalVariable(GlobalVariable *OrigGV,
                                                GlobalVariable *NewGV) {
  // If we didn't delete fields from this type, don't do anything with the
  // global variable. This happens with dependent types.
  llvm::Type *OrigTy = OrigGV->getType()->getPointerElementType();
  if (!FieldIdxMap.count(OrigTy))
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
      assert(GEP->getNumIndices() == 2 && "Unexpected GV GEP index count!");
      auto *FieldIdxConst = cast<ConstantInt>(GEP->getOperand(2));
      uint64_t FieldIdx = FieldIdxConst->getLimitedValue();
      uint64_t NewIdx = FieldIdxMap[OrigTy][FieldIdx];
      // If the index for this GEP hasn't changed we don't need to do anything.
      if (FieldIdx == NewIdx)
        continue;
      // Although each operator GEP looks like it appears directly in an
      // instruction and therefore should have a single use, there is
      // actually only one instance of each unique set of indices for
      // the GEP because it is a constant.
      if (NewIdx == FIELD_DELETED) {
        GEPsToErase.push_back(GEP);
      } else {
        Constant *Indices[] = {
            cast<Constant>(GEP->getOperand(1)),
            ConstantInt::get(Type::getInt32Ty(GEP->getContext()), NewIdx)};
        auto *NewGEP = ConstantExpr::getGetElementPtr(ReplTy, NewGV, Indices,
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

  std::function<void(User *)> eraseCastAndStoreUses = [&eraseCastAndStoreUses](
                                                          User *U) {
    assert((isa<StoreInst>(U) || isa<CastInst>(U) || isa<BitCastOperator>(U)) &&
           "Unexpected GEP operator user");
    if (isa<CastInst>(U) || isa<BitCastOperator>(U)) {
      while (!U->user_empty()) {
        auto *SubUser = U->user_back();
        eraseCastAndStoreUses(SubUser);
      }
    }
    if (auto *I = dyn_cast<Instruction>(U)) {
      LLVM_DEBUG(dbgs() << "Delete field: erasing GEP const expr user: " << *I
                        << "\n");
      I->eraseFromParent();
    } else {
      auto *Const = cast<Constant>(U);
      assert(!Const->isConstantUsed() &&
             "Attempting to delete const GEP that still has uses!");
      Const->destroyConstant();
    }
  };

  for (auto *GEP : GEPsToErase) {
    while (!GEP->user_empty()) {
      auto *U = GEP->user_back();
      eraseCastAndStoreUses(U);
    }
  }
}

bool dtrans::DeleteFieldPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo) {

  DTransTypeRemapper TypeRemapper;
  DeleteFieldImpl Transformer(DTInfo, M.getContext(), M.getDataLayout(),
                              "__DFDT_", &TypeRemapper);
  return Transformer.run(M);
}

PreservedAnalyses dtrans::DeleteFieldPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);

  if (!runImpl(M, DTransInfo))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<DTransAnalysis>();
  return PA;
}
