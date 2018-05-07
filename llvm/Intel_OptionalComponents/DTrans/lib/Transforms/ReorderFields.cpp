//===---------------- ReorderFields.cpp - DTransReorderFieldsPass ---------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans fields reorder optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/ReorderFields.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;

#define DEBUG_TYPE "dtrans-reorderfields"

// Enable Padding based heuristic to select candidates for "Reorder Fields".
static cl::opt<bool> DTransReoderFieldsPaddingHeuristic(
    "dtrans-reorder-fields-padding-heuristic", cl::init(true),
    cl::ReallyHidden);

// Minimum unused space (padding) percent threshold to select candidate
// structure for reorder fields.
static cl::opt<unsigned> DTransReorderFieldsUnusedSpacePercentThreshold(
    "dtrans-reorder-fields-unused-space-percent-threshold", cl::init(20),
    cl::ReallyHidden);

// Limit size of structs that are selected for reordering fields based
// on padding heuristic. Field affinity info may be needed to apply
// reordering for structs that don’t fit cache Line.
// "TargetTransformInfo::getCacheLineSize" can be used instead MaxStructSize
// after doing more experiments.
static const unsigned MaxStructSize = 64;

namespace {

class DTransReorderFieldsWrapper : public ModulePass {
private:
  dtrans::ReorderFieldsPass Impl;

public:
  static char ID;

  DTransReorderFieldsWrapper() : ModulePass(ID) {
    initializeDTransReorderFieldsWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    DTransAnalysisInfo &DTInfo =
        getAnalysis<DTransAnalysisWrapper>().getDTransInfo();
    return Impl.runImpl(M, DTInfo);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransReorderFieldsWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransReorderFieldsWrapper, "dtrans-reorderfields",
                      "DTrans reorder fields", false, false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_END(DTransReorderFieldsWrapper, "dtrans-reorderfields",
                    "DTrans reorder fields", false, false)

ModulePass *llvm::createDTransReorderFieldsWrapperPass() {
  return new DTransReorderFieldsWrapper();
}

// Returns true if StructT is a candidate for field reordering based
// on unused space in the structure due to alignment
bool dtrans::ReorderFieldsPass::isCandidateTypeHasEnoughPadding(
    StructType *StructT, const DataLayout &DL) {
  if (!DTransReoderFieldsPaddingHeuristic)
    return false;

  size_t StructSize = DL.getTypeAllocSize(StructT);
  if (StructSize > MaxStructSize)
    return false;

  // Compute unused space due to alignment
  uint32_t ExpectedOffset = 0;
  int32_t UnusedSpace = 0;
  for (unsigned i = 0; i < StructT->getNumElements(); ++i) {
    Type *Ty = StructT->getElementType(i);
    uint32_t FieldOff = DL.getStructLayout(StructT)->getElementOffset(i);
    UnusedSpace += (FieldOff - ExpectedOffset);
    ExpectedOffset = FieldOff + DL.getTypeAllocSize(Ty);
  }
  UnusedSpace += (StructSize - ExpectedOffset);
  if ((UnusedSpace * 100) / DL.getTypeAllocSize(StructT) <
      DTransReorderFieldsUnusedSpacePercentThreshold)
    return false;

  DEBUG(dbgs() << "  Selected based on padding heuristic: "
               << StructT->getName() << " ( Size: " << StructSize
               << " UnusedSpace: " << UnusedSpace << " )\n");
  return true;
}

// Returns true if StructT is a candidate for field reordering based
// on heuristics.
bool dtrans::ReorderFieldsPass::isCandidateType(StructType *StructT,
                                                const DataLayout &DL) {
  // Check Padding heuristic
  if (isCandidateTypeHasEnoughPadding(StructT, DL))
    return true;

  // Check more heuristics here if needed.

  return false;
}

bool dtrans::ReorderFieldsPass::gatherCandidateTypes(DTransAnalysisInfo &DTInfo,
                                                     const DataLayout &DL) {
  // TODO: Create a safety mask for the conditions that are common to all
  //       DTrans optimizations.
  ReorderFieldsSafetyConditions =
      dtrans::BadCasting | dtrans::BadAllocSizeArg |
      dtrans::BadPtrManipulation | dtrans::AmbiguousGEP | dtrans::VolatileData |
      dtrans::MismatchedElementAccess | dtrans::WholeStructureReference |
      dtrans::UnsafePointerStore | dtrans::FieldAddressTaken |
      dtrans::HasInitializerList | dtrans::BadMemFuncSize |
      dtrans::BadMemFuncManipulation | dtrans::AmbiguousPointerTarget |
      dtrans::UnsafePtrMerge | dtrans::AddressTaken | dtrans::NoFieldsInStruct |
      dtrans::NestedStruct | dtrans::ContainsNestedStruct |
      dtrans::SystemObject;

  DEBUG(dbgs() << "Reorder fields: looking for candidate structures.\n");

  for (dtrans::TypeInfo *TI : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    if (StInfo->testSafetyData(ReorderFieldsSafetyConditions)) {
      DEBUG(dbgs() << "  Rejecting "
                   << cast<StructType>(StInfo->getLLVMType())->getName()
                   << " based on safety data.\n");
      continue;
    }
    StructType *StructT = cast<StructType>(StInfo->getLLVMType());

    if (!isCandidateType(StructT, DL))
      continue;

    CandidateTypes.push_back(StInfo);
  }

  DEBUG(if (CandidateTypes.empty()) dbgs() << "  No candidates found.\n");

  return !CandidateTypes.empty();
}

bool dtrans::ReorderFieldsPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo) {
  auto &DL = M.getDataLayout();
  if (!gatherCandidateTypes(DTInfo, DL))
    return false;

  // TODO: Implement the optimization.

  return false;
}

PreservedAnalyses dtrans::ReorderFieldsPass::run(Module &M,
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
