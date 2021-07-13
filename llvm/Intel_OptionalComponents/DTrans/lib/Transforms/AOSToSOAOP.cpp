//==== AOSToSOAOP.cpp - AOS-to-SOA with support for opaque pointers ====//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
// This file implements the DTrans Array of Structures to Structure of Arrays
// data layout optimization pass with support for IR using either opaque or
// non-opaque pointers.
//
// The AOS-to-SOA transformation will convert an allocation of a structure
// into a form where pointers to the structures of the type transformed are
// no longer used, but instead accesses are made using an integer index into
// an array.
//
// The following example shows the use of non-opaque pointers, with opaque
// pointers things are similar except all pointer types will simply be 'ptr'.
//
// For example:
//   %struct.test = type { i64, %struct.test* }
//   %array = call i8* @malloc(i64 160)
//
// This would create an array of 10 instances of the structure. These elements
// can be accessed from the array. However, because a pointer to the type of the
// structure is stored, the address of an element of the array can be stored to
// memory allowing access an arbitrary element of the allocated array. To handle
// this, when the transformation takes place, pointers to the type transformed
// will be converted into integer indices of the allocated array. There are many
// conditions imposed to support this, such as, only allowing a single array of
// structures of the type being allocated in the program. This results in the
// transformation producing a new structure which will hold an array for each of
// the original field members of the structure type being transformed and a
// global variable of this new structure type, which will be initialized to hold
// the address where each of these arrays begins at the point when memory for
// the original array of structures was allocated.
//
// New structure:
//   %SOA_struct.test = type { i64*, i64* }
//   @SOA_VAR = internal global %SOA_struct.test zeroinitializer
//
// The original i64 type has been converted to a pointer to i64 elements.
// The %struct.test* has been converted into a pointer of i64 elements as well,
// because all load & stores of pointers to the type will be transformed to hold
// the integer index of the array during this transformation. Indexing will
// begin with 1 to allow the use of 0 to represent a nullptr element. When the
// allocation occurs, the global variable will be initialized with an address
// where each arrays begin.
//
// Note 1: This requires the allocation to allocate space for one more structure
// than the original allocation.
// Note 2: the size of the integer for the index in some cases will be 32-bits
// instead of 64-bits, when it is determined that the maximum number of elements
// will fit within a 32-bit value.
//
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/AOSToSOAOP.h"

#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/InitializePasses.h"

using namespace llvm;
using namespace dtransOP;
using dtrans::StructInfo;

#define DEBUG_TYPE "dtrans-aostosoaop"

namespace {
class DTransAOSToSOAOPWrapper : public ModulePass {
private:
  AOSToSOAOPPass Impl;

public:
  static char ID;

  DTransAOSToSOAOPWrapper() : ModulePass(ID) {
    initializeDTransAOSToSOAOPWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto &DTAnalysisWrapper = getAnalysis<DTransSafetyAnalyzerWrapper>();
    DTransSafetyInfo &DTInfo = DTAnalysisWrapper.getDTransSafetyInfo(M);
    auto &WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
    AOSToSOAOPPass::GetTLIFuncType GetTLI =
        [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    // This lambda function is to allow getting the DominatorTree analysis for a
    // specific function to allow analysis of loops when checking the dynamic
    // allocation of the structure type candidates of this transformation.
    AOSToSOAOPPass::DominatorTreeFuncType GetDT =
        [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };

    bool Changed = Impl.runImpl(M, &DTInfo, WPInfo, GetTLI, GetDT);
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransSafetyAnalyzerWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};
} // end anonymous namespace

char DTransAOSToSOAOPWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransAOSToSOAOPWrapper, "dtrans-aostosoaop",
                      "DTrans array of structures to structure of arrays with "
                      "opaque pointer support",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(DTransSafetyAnalyzerWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransAOSToSOAOPWrapper, "dtrans-aostosoaop",
                    "DTrans array of structures to structure of arrays with "
                    "opaque pointer support",
                    false, false)

ModulePass *llvm::createDTransAOSToSOAOPWrapperPass() {
  return new DTransAOSToSOAOPWrapper();
}

namespace llvm {
namespace dtransOP {
bool AOSToSOAOPPass::runImpl(Module &M, DTransSafetyInfo *DTInfo,
                             WholeProgramInfo &WPInfo,
                             AOSToSOAOPPass::GetTLIFuncType &GetTLI,
                             AOSToSOAOPPass::DominatorTreeFuncType &GetDT) {
  LLVM_DEBUG(dbgs() << "Running AOS-to-SOA for opaque pointers pass\n");

  // Make the pass do something
  return false;
}

PreservedAnalyses AOSToSOAOPPass::run(Module &M, ModuleAnalysisManager &AM) {
  DTransSafetyInfo *DTInfo = &AM.getResult<DTransSafetyAnalyzer>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  GetTLIFuncType GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  DominatorTreeFuncType GetDT = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };

  bool Changed = runImpl(M, DTInfo, WPInfo, GetTLI, GetDT);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.abandon<DTransSafetyAnalyzer>();
  return PA;
}

} // end namespace dtransOP
} // end namespace llvm
