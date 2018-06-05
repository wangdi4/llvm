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

  virtual bool prepareTypes(Module &M) override;
  virtual void populateTypes(Module &M) override;
  virtual void processFunction(Function &F) override;
  virtual void postprocessFunction(Function &OrigFunc, bool isCloned) override;

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
  void postProcessCallInst(Instruction *I);
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
      dtrans::HasInitializerList | dtrans::BadMemFuncSize |
      dtrans::BadMemFuncManipulation | dtrans::AmbiguousPointerTarget |
      dtrans::UnsafePtrMerge | dtrans::AddressTaken | dtrans::NoFieldsInStruct |
      dtrans::NestedStruct | dtrans::ContainsNestedStruct |
      dtrans::MemFuncPartialWrite | dtrans::SystemObject;

  DEBUG(dbgs() << "Delete field: looking for candidate structures.\n");

  for (dtrans::TypeInfo *TI : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    // We're only interested in fields that are never read. Fields that are
    // written but not read can be deleted.
    bool HasUnreadFields = false;
    size_t NumFields = StInfo->getNumFields();
    for (size_t i = 0; i < NumFields; ++i) {
      dtrans::FieldInfo &FI = StInfo->getField(i);
      if (!FI.isRead()) {
        DEBUG(dbgs() << "  Found unread field: "
                     << cast<StructType>(StInfo->getLLVMType())->getName()
                     << " @ " << i << "\n");
        HasUnreadFields = true;
#ifdef NDEBUG
        break;
#endif // NDEBUG
      }
    }

    if (!HasUnreadFields)
      continue;

    if (StInfo->testSafetyData(DeleteFieldSafetyConditions)) {
      DEBUG(dbgs() << "  Rejecting "
                   << cast<StructType>(StInfo->getLLVMType())->getName()
                   << " based on safety data.\n");
      continue;
    }

    DEBUG(dbgs() << "  Selected for deletion: "
                 << cast<StructType>(StInfo->getLLVMType())->getName() << "\n");

    StructsToConvert.push_back(StInfo);
  }

  if (StructsToConvert.empty()) {
    DEBUG(dbgs() << "  No candidates found.\n");
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
      if (FI.isRead()) {
        DEBUG(dbgs() << OrigTy->getName() << "[" << i << "] = " << NewIdx
                     << "\n");
        NewIndices.push_back(NewIdx++);
        DataTypes.push_back(TypeRemapper->remapType(FI.getLLVMType()));
      } else {
        DEBUG(dbgs() << OrigTy->getName() << "[" << i << "] = DELETED\n");
        NewIndices.push_back(FIELD_DELETED);
      }
    }
    NewTy->setBody(DataTypes, OrigTy->isPacked());
    DEBUG(dbgs() << "Delete field: New structure body: " << *NewTy << "\n");
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

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    if (auto *GEP = dyn_cast<GetElementPtrInst>(&*I)) {
      uint64_t Unused;
      if (processGEPInst(GEP, Unused, /*IsPreCloning=*/true))
        GEPsToDelete.push_back(GEP);
    }
  }

  for (auto *GEP : GEPsToDelete) {
    DEBUG(dbgs() << "Delete field: erasing GEP of deleted field:\n"
                 << *GEP << "\n");
    for (auto *U : GEP->users()) {
      assert(isa<StoreInst>(U) && "Unexpected use of deleted field!");
      DEBUG(dbgs() << "Delete field: erasing GEP user:\n" << *U << "\n");
      cast<Instruction>(U)->eraseFromParent();
    }
    GEP->eraseFromParent();
  }
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
  // TODO: Add code to update instructions using the size of our structs.
  Function *F = isCloned ? OrigFuncToCloneFuncMap[&OrigFunc] : &OrigFunc;
  for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It) {
    Instruction *I = &*It;
    if (auto *GEP = dyn_cast<GetElementPtrInst>(I)) {
      uint64_t NewIndex;
      if (processGEPInst(GEP, NewIndex, /*IsPreCloning=*/false)) {
        LLVM_DEBUG(dbgs() << "Delete field: Replacing instruction\n"
                          << *GEP << "\n");
        GEP->setOperand(
            2, ConstantInt::get(Type::getInt32Ty(GEP->getContext()), NewIndex));
        LLVM_DEBUG(dbgs() << "    with\n" << *GEP << "\n");
      }
      continue;
    }
    if (isa<CallInst>(I))
      postProcessCallInst(I);
  }
}

void DeleteFieldImpl::postProcessCallInst(Instruction *I) {
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
    // FIXME: For some reason MemfuncCallInfo seems to have an extra layer of
    //        indirection.
    if (isa<dtrans::MemfuncCallInfo>(CInfo) && PointeeTy->isPointerTy())
      PointeeTy = PointeeTy->getPointerElementType();
    for (auto &ONPair : OrigToNewTypeMapping) {
      llvm::Type *OrigTy = ONPair.first;
      llvm::Type *ReplTy = ONPair.second;
      // FIXME: Shouldn't the CallInfo have been updated?
      //      assert(PointeeTy != OrigTy &&
      //             "Original type found after type replacement!");
      //      if (PointeeTy != ReplTy)
      if (PointeeTy != OrigTy)
        continue;
      LLVM_DEBUG(dbgs() << "Found call involving type with deleted fields:\n"
                        << *I << "\n"
                        << "  " << *OrigTy << "\n");
      updateSizeOperand(I, CInfo, OrigTy, ReplTy);
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
