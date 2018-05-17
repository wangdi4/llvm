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

private:
  // The pointers in this vector are owned by the DTransAnalysisInfo.
  // The list is populated during prepareTypes() and used in populateTypes().
  SmallVector<dtrans::StructInfo *, 4> StructsToConvert;

  // A mapping from the original structure type to the new structure type
  TypeToTypeMap OrigToNewTypeMapping;
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
      dtrans::SystemObject;

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
  // In the initial implementation we are simply renaming types without
  // changing anything within the body of the type other than renaming
  // any dependent types. Therefore, it can rely on the base class
  // functionality to fill in the body for the new type.
  DTransOptBase::populateDependentTypes(M, OrigToNewTypeMapping);
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
