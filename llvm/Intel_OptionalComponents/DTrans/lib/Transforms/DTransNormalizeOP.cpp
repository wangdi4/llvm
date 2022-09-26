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
// Example:
// From:
//   %struct.Y = type { [7 x ptr] }
//   %struct.X = type { i64, %struct.Y }
//   %x1 = getelementptr inbounds %struct.X, ptr %arg, i64 0, i32 1
//   %y2 = getelementptr inbounds [7 x ptr], ptr %w, i64 0, i64 0
//   %63 = phi ptr [ %x1, %B0 ], [ %y2, %B1 ]
// To:
//   %x1 = getelementptr inbounds %struct.X, ptr %arg, i64 0, i32 1
//   %dtnorm = getelementptr inbounds [7 x ptr], ptr %x1, i64 0, i32 0
//   %y2 = getelementptr inbounds [7 x ptr], ptr %w, i64 0, i64 0
//   %63 = phi ptr [ %dtnorm, %B0 ], [ %y2, %B1 ]
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
        else if (auto PHIN = dyn_cast<PHINode>(I))
          checkPHI(PHIN);
      }
    }

    if (InstructionsToGepify.empty() && PHIsToGepify.empty())
      return false;

    for (auto &KV : InstructionsToGepify)
      gepify(std::get<0>(KV), std::get<1>(KV), std::get<2>(KV));

    for (auto &KV : PHIsToGepify)
      gepifyPHI(KV.first, KV.second);

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

  void checkPHI(PHINode *PHIN) {

    auto GetCandidateType = [this](Value *V) -> Type * {
      if (auto PHI = dyn_cast<PHINode>(V))
        return PHIType.lookup(PHI);
      if (auto GEPI = dyn_cast<GetElementPtrInst>(V))
        return GEPI->getSourceElementType();
      return nullptr;
    };

    auto SimpleStructGEPI = [](Value *V) -> Type * {
      auto GEPI = dyn_cast<GetElementPtrInst>(V);
      if (!GEPI || GEPI->getNumIndices() != 2)
        return nullptr;
      auto CI0 = dyn_cast<ConstantInt>(GEPI->getOperand(1));
      if (!CI0 || !CI0->isZero())
        return nullptr;
      auto CI1 = dyn_cast<ConstantInt>(GEPI->getOperand(2));
      if (!CI1)
        return nullptr;
      auto Ty = dyn_cast<StructType>(GEPI->getSourceElementType());
      if (!Ty)
        return nullptr;
      auto STy = dyn_cast<StructType>(Ty->getTypeAtIndex(CI1->getZExtValue()));
      if (!STy || STy->getNumElements() == 0)
        return nullptr;
      return STy->getTypeAtIndex((unsigned)0);
    };

    auto TestAndInsert = [this, SimpleStructGEPI](PHINode *PHIN, unsigned Num,
                                                  Value *GEPI0, Type *Ty) -> bool {
      if (auto ATy0 = dyn_cast_or_null<ArrayType>(SimpleStructGEPI(GEPI0)))
        if (auto ATy1 = dyn_cast<ArrayType>(Ty))
          if (ATy0 == ATy1) {
            PHIsToGepify.insert({PHIN, Num});
            PHIType.insert({PHIN, Ty});
            return true;
          }
      return false;
    };

    if (PHIN->getNumIncomingValues() != 2)
      return;
    auto V0 = PHIN->getIncomingValue(0);
    auto V1 = PHIN->getIncomingValue(1);
    auto Ty0 = GetCandidateType(V0);
    auto Ty1 = GetCandidateType(V1);
    if (!Ty0 || !Ty1)
      return;
    if (Ty0 == Ty1) {
      PHIType.insert({PHIN, Ty0});
      return;
    }
    if (TestAndInsert(PHIN, 0, V0, Ty1))
      return;
    if (TestAndInsert(PHIN, 1, V1, Ty0))
      return;
  }

  void gepifyPHI(PHINode *PHIN, unsigned Num) {
    auto GEPI = dyn_cast<GetElementPtrInst>(PHIN->getIncomingValue(Num));
    if (!GEPI)
      return;
    Type *Ty = PHIType.lookup(PHIN);
    if (!Ty)
      return;

    SmallVector<Value *, 2> IdxList;
    IdxList.push_back(ZeroPtrSizedInt);
    IdxList.push_back(Zero32);
    auto *GEP = GetElementPtrInst::Create(Ty, GEPI, IdxList, "dtnorm",
        GEPI->getNextNonDebugInstruction());
    PHIN->replaceUsesOfWith(PHIN->getIncomingValue(Num), GEP);
  }

  ConstantInt *Zero32 = nullptr;
  ConstantInt *ZeroPtrSizedInt = nullptr;

  std::unique_ptr<PtrTypeAnalyzer> PtrAnalyzer;
  SetVector<std::tuple<Instruction *, DTransType *, unsigned int>>
      InstructionsToGepify;
  SmallDenseMap<PHINode *, Type *> PHIType;
  SetVector<std::pair<PHINode *, unsigned>> PHIsToGepify;
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
