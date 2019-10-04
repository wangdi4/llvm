//===---DTransOptBaseTest.cpp - Test pass for DTransOptBase functionality--===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a test pass that exercises the basic functionality of
// the DTransOptBase class.
//
//===----------------------------------------------------------------------===//

// This file is only used for opt testing, do not include it as part of the
// product build.
#if !INTEL_PRODUCT_RELEASE

#include "Intel_DTrans/Transforms/DTransOptBaseTest.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOptBase.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"

using namespace llvm;

#define DEBUG_TYPE "dtrans-optbasetest"

// This option is used to supply a comma separated list of structure types that
// should be renamed as part of the DTransTransform class test to verify
// dependent objects get transformed appropriately.
static cl::opt<std::string>
    DTransOptBaseTestTypeList("dtrans-optbasetest-typelist", cl::ReallyHidden);

// This option controls whether the DTrans analysis info will be passed into
// the base class to allow testing the base class operation with and without
// having the DTrans analysis info available.
static cl::opt<bool>
    DTransOptBaseTestUseAnalysis("dtrans-optbasetest-use-analysis",
                                 cl::init(true), cl::ReallyHidden);

namespace {

class DTransOptBaseTestWrapper : public ModulePass {
private:
  dtrans::OptBaseTestPass Impl;

public:
  static char ID;

  DTransOptBaseTestWrapper() : ModulePass(ID) {
    initializeDTransOptBaseTestWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    DTransAnalysisInfo *DTInfo =
        DTransOptBaseTestUseAnalysis
            ? &getAnalysis<DTransAnalysisWrapper>().getDTransInfo(M)
            : nullptr;
    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };
    WholeProgramInfo &WPInfo =
      getAnalysis<WholeProgramWrapperPass>().getResult();
    return Impl.runImpl(M, DTInfo, GetTLI, WPInfo);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

// This class tests and demonstrates usage of the DTransOptBase class.
class DTransOptBaseTest : public DTransOptBase {
public:
  DTransOptBaseTest(
      DTransAnalysisInfo *DTInfo, LLVMContext &Context, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      StringRef DepTypePrefix, DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(DTInfo, Context, DL, GetTLI, DepTypePrefix,
                      TypeRemapper) {}

  virtual bool prepareTypes(Module &M) override {
    SmallVector<StringRef, 4> SubStrings;
    SplitString(DTransOptBaseTestTypeList, SubStrings, ",");

    SmallPtrSet<llvm::StructType *, 2> TypesToConvert;
    for (auto &Name : SubStrings) {
      Type *Ty = M.getTypeByName(Name);
      if (Ty) {
        if (auto *StructTy = dyn_cast<StructType>(Ty)) {
          LLVM_DEBUG(dbgs()
                     << "DTRANS-OPTBASETEST: Type marked for conversion: "
                     << Name << "\n");
          TypesToConvert.insert(StructTy);
        } else {
          errs() << "DTRANS-OPTBASETEST: Ignored: Type is not a struct type: "
                 << Name << "\n";
        }
      } else {
        errs() << "DTRANS-OPTBASETEST: Ignored: Invalid type name requested: "
               << Name << "\n";
      }
    }

    LLVMContext &Context = M.getContext();
    for (auto *StructTy : TypesToConvert) {
      // Create an Opaque type as a placeholder, until we know all the
      // types that need to be created.
      StructType *NewStructTy = StructType::create(
          Context, (Twine("__DTT_" + StructTy->getName()).str()));
      TypeRemapper->addTypeMapping(StructTy, NewStructTy);
      OrigToNewTypeMapping[StructTy] = NewStructTy;
    }

    return !TypesToConvert.empty();
  }

  virtual void populateTypes(Module &M) override {
    // Because this test pass is simply renaming an existing type without
    // changing anything within the body of the type other than renaming
    // any dependent types, it can rely on the base class functionality to
    // fill in the body for the new type.
    DTransOptBase::populateDependentTypes(M, OrigToNewTypeMapping);
  }

private:
  // A mapping from the original structure type to the new structure type
  TypeToTypeMap OrigToNewTypeMapping;
};
} // end anonymous namespace

char DTransOptBaseTestWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransOptBaseTestWrapper, "dtrans-optbasetest",
                      "DTrans optimization base class tester", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransOptBaseTestWrapper, "dtrans-optbasetest",
                    "DTrans optimization base class tester", false, false)

ModulePass *llvm::createDTransOptBaseTestWrapperPass() {
  return new DTransOptBaseTestWrapper();
}

bool dtrans::OptBaseTestPass::runImpl(
    Module &M, DTransAnalysisInfo *DTInfo,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
    WholeProgramInfo &WPInfo) {
  if (!WPInfo.isWholeProgramSafe())
    return false;

  DTransTypeRemapper TypeRemapper;
  DTransOptBaseTest Transformer(DTInfo, M.getContext(), M.getDataLayout(),
                                GetTLI, "__DDT_", &TypeRemapper);
  return Transformer.run(M);
}

PreservedAnalyses dtrans::OptBaseTestPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  DTransAnalysisInfo *DTInfo =
      DTransOptBaseTestUseAnalysis ? &AM.getResult<DTransAnalysis>(M) : nullptr;
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function*>(&F)));
  };
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  bool Changed = runImpl(M, DTInfo, GetTLI, WPInfo);

  if (!Changed)
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

#endif // !INTEL_PRODUCT_RELEASE
