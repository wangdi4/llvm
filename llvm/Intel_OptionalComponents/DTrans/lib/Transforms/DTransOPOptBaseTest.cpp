//===-----------------DTransOptBaseOpaquePtrTest.cpp-----------------------===//
// Test pass for DTransOPOptBase functionality
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a test pass that exercises the basic functionality of
// the DTransOPOptBase class.
//
//===----------------------------------------------------------------------===//

// This file is only used for opt testing, do not include it as part of the
// product build.
#if !INTEL_PRODUCT_RELEASE
#include "Intel_DTrans/Transforms/DTransOPOptBaseTest.h"

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace dtransOP;

#define DEBUG_TYPE "dtransop-optbasetest"

// This option is used to supply a comma separated list of structure types that
// should be renamed as part of the DTransOPOptBaseTestPass class test to verify
// dependent objects get transformed appropriately.
static cl::opt<std::string>
    DTransOPOptBaseOpaquePtrTestTypeList("dtransop-optbasetest-typelist",
                                         cl::ReallyHidden);

namespace {
class DTransOPOptBaseTestWrapper : public ModulePass {
private:
  DTransOPOptBaseTestPass Impl;

public:
  static char ID;

  DTransOPOptBaseTestWrapper() : ModulePass(ID) {
    initializeDTransOPOptBaseTestWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    DTransSafetyAnalyzerWrapper &DTAnalysisWrapper =
        getAnalysis<DTransSafetyAnalyzerWrapper>();
    DTransSafetyInfo &DTInfo = DTAnalysisWrapper.getDTransSafetyInfo(M);
    return Impl.runImpl(M, &DTInfo);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransSafetyAnalyzerWrapper>();
  }
};

// This class tests and demonstrates usage of the DTransOptBase class.
class DTransOptBaseTest : public DTransOPOptBase {
public:
  DTransOptBaseTest(LLVMContext &Ctx, DTransSafetyInfo *DTInfo,
                    bool UsingOpaquePtrs, StringRef DepTypePrefix)
      : DTransOPOptBase(Ctx, DTInfo, UsingOpaquePtrs, DepTypePrefix) {}

  virtual bool prepareTypes(Module &M) override {
    SmallVector<StringRef, 16> SubStrings;
    SplitString(DTransOPOptBaseOpaquePtrTestTypeList, SubStrings, ",");

    // Gether the types to be converted by this class.
    SmallPtrSet<DTransStructType *, 16> TypesToConvert;
    for (auto *DTransSructTy : KnownStructTypes) {
      if (!DTransSructTy->hasName())
        continue;

      if (std::find(SubStrings.begin(), SubStrings.end(),
                    DTransSructTy->getName()) == SubStrings.end())
        continue;

      LLVM_DEBUG(dbgs() << "DTRANS-OPTBASETEST: Type marked for conversion: "
                        << DTransSructTy->getName() << "\n");
      TypesToConvert.insert(DTransSructTy);
    }

    if (TypesToConvert.empty())
      return false;

    LLVMContext &Ctx = M.getContext();
    for (auto *DTransSructTy : TypesToConvert) {
      // Create an Opaque type as a placeholder, until we know all the
      // types that need to be created.
      llvm::Type *LLVMTy = DTransSructTy->getLLVMType();
      assert(LLVMTy && "Failed to convert DTransStructType to llvm type");
      auto *LLVMStructTy = cast<llvm::StructType>(LLVMTy);
      llvm::StructType *NewStructTy = StructType::create(
          Ctx, (Twine("__DTT_" + LLVMStructTy->getName()).str()));

      // Also, create an opaque type in the DTransType representation.
      DTransType *NewDTransStructTy = TM.getOrCreateStructType(NewStructTy);
      getTypeRemapper()->addTypeMapping(LLVMStructTy, NewStructTy,
                                        DTransSructTy, NewDTransStructTy);
      OrigToNewTypeMapping[LLVMStructTy] = NewStructTy;

      LLVM_DEBUG(dbgs() << "DTransOPOptBaseTest: New type created: "
                        << *NewStructTy << " as replacement for "
                        << *LLVMStructTy << "\n");
    }

    return true;
  }

  virtual void populateTypes(Module &M) override {
    // Because this test pass is simply renaming an existing type without
    // changing anything within the body of the type other than renaming
    // any dependent types, it can rely on the base class functionality to
    // fill in the body for the new type.
    DTransOPOptBase::populateDependentTypes(M, OrigToNewTypeMapping);
  }

private:
  // A mapping from the original structure type to the new structure type
  LLVMTypeToTypeMap OrigToNewTypeMapping;
};

} // end anonymous namespace

char DTransOPOptBaseTestWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransOPOptBaseTestWrapper, "dtransop-optbasetest",
                      "DTrans base class tester for opaque pointers", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DTransSafetyAnalyzerWrapper)
INITIALIZE_PASS_END(DTransOPOptBaseTestWrapper, "dtransop-optbasetest",
                    "DTrans base class tester for opaque pointers", false,
                    false)

ModulePass *llvm::createDTransOPOptBaseTestWrapperPass() {
  return new DTransOPOptBaseTestWrapper();
}

// Implementation for new pass manager
PreservedAnalyses
dtransOP::DTransOPOptBaseTestPass::run(Module &M, ModuleAnalysisManager &AM) {
  DTransSafetyInfo *DTInfo = &AM.getResult<DTransSafetyAnalyzer>(M);
  bool Changed = runImpl(M, DTInfo);
  if (!Changed)
    return PreservedAnalyses::all();

  // This pass is only used for testing the base class, so there is no need to
  // preserve anything when the IR changes.
  return PreservedAnalyses::none();
}

bool dtransOP::DTransOPOptBaseTestPass::runImpl(Module &M,
                                                DTransSafetyInfo *DTInfo) {
  auto CheckForOpaquePointers = [](Module &M, DTransSafetyInfo *DTInfo) {
    if (DTInfo->useDTransSafetyAnalysis())
      return DTInfo->getPtrTypeAnalyzer().sawOpaquePointer();

    // If the DTrans pointer type analyzer did not run, then we need to scan for
    // opaque pointers being present here.
    for (auto &GV : M.globals())
      if (GV.getType()->isOpaquePointerTy())
        return true;

    for (auto &F : M) {
      if (F.getType()->isOpaquePointerTy())
        return true;
      for (auto &I : instructions(F))
        if (I.getType()->isOpaquePointerTy())
          return true;
    }
    return false;
  };

  assert(DTInfo && "DTransSafetyInfo is required");
  DTransOptBaseTest Transformer(M.getContext(), DTInfo,
                                CheckForOpaquePointers(M, DTInfo), "__DDT_");
  return Transformer.run(M);
}

#endif // !INTEL_PRODUCT_RELEASE
