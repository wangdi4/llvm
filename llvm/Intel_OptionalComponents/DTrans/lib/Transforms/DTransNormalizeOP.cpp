//==== DTransNormalizeOP.cpp - Normalize IR for the DTransSafetyAnalyzer ====//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
// This file defines the pass that normalizes the IR to reduce instances where
// safety flags will be set by the DTransSafetyAnalyzer.
//
// 1: Conversion of element zero accesses to be GEP based.
// Example:
// From:
//   %struct.test = type { i32 }
//   %ptr_to_struct.test = alloca %struct.test
//   %x = load i32, ptr %ptr_to_struct.test
// To:
//   %ptr_to_struct.test = alloca %struct.test
//   %dtnorm = getelementptr %struct.test, ptr %ptr_to_struct.test, i64 0, i32 0
//   %x = load i32, ptr %dtnorm
//
// 2: This pass could be extended in the future to support other cases, such as
//    replacing byte-flattened GEPs.
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DTransNormalizeOP.h"

#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

using namespace llvm;
using namespace dtransOP;

#define DEBUG_TYPE "dtrans-normalizeop"

namespace {

class DTransNormalizeOPWrapper : public ModulePass {
private:
  dtransOP::DTransNormalizeOPPass Impl;

public:
  static char ID;

  DTransNormalizeOPWrapper() : ModulePass(ID) {
    initializeDTransNormalizeOPWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto &WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    bool Changed = Impl.runImpl(M, WPInfo, GetTLI);
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<TargetLibraryInfoWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

class DTransNormalizeImpl {
public:
  bool run(Module &M, DTransNormalizeOPPass::GetTLIFn GetTLI) {
    LLVMContext &Ctx = M.getContext();
    const DataLayout &DL = M.getDataLayout();
    std::unique_ptr<DTransTypeManager> TM =
        std::make_unique<DTransTypeManager>(Ctx);
    std::unique_ptr<TypeMetadataReader> MDReader =
        std::make_unique<TypeMetadataReader>(*TM);
    if (!MDReader->initialize(M)) {
      LLVM_DEBUG(
          dbgs() << "DTransSafetyInfo: Type metadata reader did not find "
                    "structure type metadata\n");
      return false;
    }

    PtrAnalyzer =
        std::make_unique<PtrTypeAnalyzer>(Ctx, *TM, *MDReader, DL, GetTLI);
    PtrAnalyzer->run(M);

    Zero32 = ConstantInt::get(llvm::Type::getInt32Ty(Ctx), 0);
    ZeroPtrSizedInt = ConstantInt::get(
        llvm::Type::getIntNTy(Ctx, M.getDataLayout().getPointerSizeInBits()),
        0);

    for (auto &F : M) {
      for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It) {
        Instruction *I = &*It;
        if (isa<LoadInst>(I) || isa<StoreInst>(I))
          checkPointer(I, getLoadStorePointerOperand(I));
      }
    }

    if (InstructionsToGepify.empty())
      return false;

    for (auto &KV : InstructionsToGepify)
      gepify(std::get<0>(KV), std::get<1>(KV), std::get<2>(KV));

    return true;
  }

private:
  static bool isCompilerConstantData(Value *V) { return isa<ConstantData>(V); }

  void checkPointer(Instruction *I, Value *Ptr) {
    if (isCompilerConstantData(Ptr))
      return;

    ValueTypeInfo *PtrInfo = PtrAnalyzer->getValueTypeInfo(Ptr);
    assert(PtrInfo &&
           "PtrTypeAnalyzer failed to construct ValueTypeInfo for load "
           "pointer operand");

    if (PtrInfo->getUnhandled() || PtrInfo->getDependsOnUnhandled())
      return;

    PtrTypeAnalyzer::ElementZeroInfo Info =
        PtrAnalyzer->getElementZeroPointer(I);
    DTransType *Ty = Info.Ty;
    if (!Ty)
      return;

    if ((Ty->isStructTy() &&
         cast<DTransStructType>(Ty)->getNumContainedElements() == 0) ||
        (Ty->isArrayTy() &&
         cast<DTransArrayType>(Ty)->getNumContainedElements() == 0))
      return;

    InstructionsToGepify.insert({I, Info.Ty, Info.Depth});
  }

  void gepify(Instruction *I, DTransType *Ty, unsigned int Depth) {
    assert(isa<LoadInst>(I) || isa<StoreInst>(I));
    Value *Ptr = getLoadStorePointerOperand(I);
    SmallVector<Value *, 2> IdxList;
    IdxList.push_back(ZeroPtrSizedInt);
    for (unsigned int Count = 0; Count < Depth; ++Count)
      IdxList.push_back(Zero32);
    auto *GEP =
        GetElementPtrInst::Create(Ty->getLLVMType(), Ptr, IdxList, "dtnorm", I);
    if (auto *LI = dyn_cast<LoadInst>(I)) {
      LLVM_DEBUG(dbgs() << "Replacing pointer operand in: " << *LI
                        << "\nWith: " << *GEP);
      LI->replaceUsesOfWith(LI->getPointerOperand(), GEP);
    } else if (auto *SI = dyn_cast<StoreInst>(I)) {
      if (SI->getValueOperand() == SI->getPointerOperand())
        return;
      LLVM_DEBUG(dbgs() << "Replacing pointer operand in: " << *SI
                        << "\nWith: " << *GEP);
      SI->replaceUsesOfWith(SI->getPointerOperand(), GEP);
    } else {
      llvm_unreachable("Only Load/Store currently allowed");
    }
  }

  ConstantInt *Zero32 = nullptr;
  ConstantInt *ZeroPtrSizedInt = nullptr;

  std::unique_ptr<PtrTypeAnalyzer> PtrAnalyzer;
  SetVector<std::tuple<Instruction *, DTransType *, unsigned int>>
      InstructionsToGepify;
};

} // end anonymous namespace

PreservedAnalyses
dtransOP::DTransNormalizeOPPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  bool Changed = runImpl(M, WPInfo, GetTLI);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<TargetLibraryAnalysis>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

bool dtransOP::DTransNormalizeOPPass::runImpl(Module &M,
                                              WholeProgramInfo &WPInfo,
                                              GetTLIFn GetTLI) {
  // This pass requires opaque pointers because when it updates instructions it
  // does not insert bitcasts to match differing pointer types.
  if (M.getContext().supportsTypedPointers())
    return false;

  if (!WPInfo.isWholeProgramSafe())
    return false;

  DTransNormalizeImpl Impl;
  return Impl.run(M, GetTLI);
}

char DTransNormalizeOPWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransNormalizeOPWrapper, "dtrans-normalizeop",
                      "Normalize IR for DTrans", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransNormalizeOPWrapper, "dtrans-normalizeop",
                    "Normalize IR for DTrans", false, false)

ModulePass *llvm::createDTransNormalizeOPWrapperPass() {
  return new DTransNormalizeOPWrapper();
}
