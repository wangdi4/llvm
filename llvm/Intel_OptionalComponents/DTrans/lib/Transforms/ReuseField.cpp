//===---------------- ReuseField.cpp - DTransReuseFieldPass -------------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans reuse field optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/ReuseField.h"
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
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;

#define DEBUG_TYPE "dtrans-reusefield"

namespace {

class DTransReuseFieldWrapper : public ModulePass {
private:
  dtrans::ReuseFieldPass Impl;

public:
  static char ID;

  DTransReuseFieldWrapper() : ModulePass(ID) {
    initializeDTransReuseFieldWrapperPass(*PassRegistry::getPassRegistry());
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

class ReuseFieldImpl : public DTransOptBase {
public:
  ReuseFieldImpl(
      DTransAnalysisInfo &DTInfo, LLVMContext &Context, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      StringRef DepTypePrefix, DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(&DTInfo, Context, DL, GetTLI, DepTypePrefix,
                      TypeRemapper) {}

  bool prepareTypes(Module &M) override;
  void populateTypes(Module &M) override;

private:

  // A mapping from the original structure type to the new structure type
  TypeToTypeMap OrigToNewTypeMapping;

  // The pointers in this vector are owned by the DTransAnalysisInfo.
  // The list is populated during prepareTypes() and used in populateTypes().
  SmallVector<dtrans::StructInfo *, 4> CandidateStructs;
};

} // end anonymous namespace

char DTransReuseFieldWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransReuseFieldWrapper, DEBUG_TYPE,
                      "DTrans reuse field", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransReuseFieldWrapper, DEBUG_TYPE,
                    "DTrans reuse field", false, false)

ModulePass *llvm::createDTransReuseFieldWrapperPass() {
  return new DTransReuseFieldWrapper();
}

bool ReuseFieldImpl::prepareTypes(Module &M) {
  LLVM_DEBUG(dbgs() << "Reuse field: looking for candidate structures.\n");

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

    if (DTInfo->testSafetyData(TI, dtrans::DT_ReuseField)) {
      LLVM_DEBUG({ dbgs() << "  Rejecting based on safety data.\n"; });
      continue;
    }

    if (DTInfo->getMaxTotalFrequency() != StInfo->getTotalFrequency()) {
      LLVM_DEBUG({ dbgs() << "  Rejecting based on heuristic.\n"; });
      continue;
    }

    CandidateStructs.push_back(StInfo);
  }

  if (CandidateStructs.size() != 1) {
    LLVM_DEBUG(dbgs() << "  No candidates found.\n");
    return false;
  }

  LLVMContext &Context = M.getContext();
  for (auto *StInfo : CandidateStructs) {
    LLVM_DEBUG({
      dbgs() << "  Selected for reuse: ";
      StInfo->getLLVMType()->print(dbgs(), true, true);
      dbgs() << "\n";
    });

    // Create an Opaque type as a placeholder, until the base class has
    // computed all the types that need to be created.
    StructType *OrigTy = cast<StructType>(StInfo->getLLVMType());
    StructType *NewStructTy = StructType::create(
        Context, (Twine(DepTypePrefix + OrigTy->getName()).str()));
    TypeRemapper->addTypeMapping(OrigTy, NewStructTy);
    OrigToNewTypeMapping[OrigTy] = NewStructTy;
  }

  return true;
}

void ReuseFieldImpl::populateTypes(Module &M) {
  for (auto *StInfo : CandidateStructs) {
    auto *OrigTy = cast<StructType>(StInfo->getLLVMType());
    auto *NewTy = cast<StructType>(OrigToNewTypeMapping[OrigTy]);

    SmallVector<Type *, 8> DataTypes;
    size_t NumFields = StInfo->getNumFields();
    for (size_t i = 0; i < NumFields; ++i) {
      dtrans::FieldInfo &FI = StInfo->getField(i);
      DataTypes.push_back(TypeRemapper->remapType(FI.getLLVMType()));
    }
    NewTy->setBody(DataTypes, OrigTy->isPacked());
  }
}

bool dtrans::ReuseFieldPass::runImpl(
    Module &M, DTransAnalysisInfo &DTInfo,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
    WholeProgramInfo &WPInfo) {

  if (!WPInfo.isWholeProgramSafe())
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  DTransTypeRemapper TypeRemapper;
  ReuseFieldImpl Transformer(DTInfo, M.getContext(), M.getDataLayout(), GetTLI,
                              "__RFDT_", &TypeRemapper);
  return Transformer.run(M);
}

PreservedAnalyses dtrans::ReuseFieldPass::run(Module &M,
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
