//=---- OCLVecClone.cpp - Vector function to loop transform -*- C++ -*----=//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
///
/// OCLVecClone pass is an OpenCL specialization of the VecClone pass which does
/// the following:
/// 1. Emits the vector-variant attributes (languageSpecificInitializations)
///    that activates VecClone.
/// 2. Updates all the uses of the TID calls with TID + new induction variable
///    and moves the TID call ut of the loop that is emitted by VecClone
///    (handleLanguageSpecifics).
///
/// Example:
/// original kernel:
///   i = get_global_id();
///   A[i] = ...
///
/// after OCLVecClone pass:
///   i = get_global_id();
///   for (j = 0; j < VF; j++){
///      A[i+j] = ...
///   }
///
/// 3. Updates the metadata that later passes use.
// ===--------------------------------------------------------------------=== //
#include "OCLVecClone.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "LoopUtils/LoopUtils.h"
#include "MetadataAPI.h"
#include "OCLPrepareKernelForVecClone.h"

#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LegacyPassManager.h"

#define DEBUG_TYPE "OCLVecClone"
#define SV_NAME "ocl-vecclone"

using namespace llvm;
using namespace Intel::MetadataAPI;

namespace intel {

char OCLVecClone::ID = 0;
static const char lv_name[] = "OCLVecClone";
OCL_INITIALIZE_PASS_BEGIN(OCLVecClone, SV_NAME, lv_name,
                          false /* modifies CFG */, false /* transform pass */)
OCL_INITIALIZE_PASS_END(OCLVecClone, SV_NAME, lv_name,
                        false /* modififies CFG */, false /* transform pass */)

OCLVecClone::OCLVecClone(const Intel::CPUId *CPUId,
                         bool EnableVPlanVecForOpenCL)
    : VecClone(), CPUId(CPUId),
      EnableVPlanVecForOpenCL(EnableVPlanVecForOpenCL) {
  V_INIT_PRINT;
}

OCLVecClone::OCLVecClone() : VecClone(), EnableVPlanVecForOpenCL(true) {}

// Remove the "ocl_recommended_vector_length" metadata from the original kernel.
// "ocl_recommened_vector_length" metadata is used only by OCLVecClone. The rest
// of the Volcano passes recognize the "vector_width" metadata. Thus, we add
// "vector_width" metadata to the original kernel and the cloned kernel.
static void updateMetadata(Function &F, Function *Clone) {
  auto FMD = KernelInternalMetadataAPI(&F);
  auto CloneMD = KernelInternalMetadataAPI(Clone);
  // Get VL from the "ocl_recommended_vector_length" metadata from the original
  // kernel.
  unsigned VectorLength = FMD.OclRecommendedVectorLength.get();
  // Set the "vector_width" metadata to the cloned kernel.
  CloneMD.VectorizedKernel.set(nullptr);
  CloneMD.VectorizedWidth.set(VectorLength);
  // For now, only x dimension is vectorized. For this reason, the chosen
  // vectorization dimension is 0.
  CloneMD.VectorizationDimension.set(0);
  // Set the metadata that points to the orginal kernel of the clone.
  CloneMD.ScalarizedKernel.set(&F);
  // TODO: for now, it is false.
  CloneMD.CanUniteWorkgroups.set(false);

  // Set "vector_width" for the original kernel.
  FMD.VectorizedWidth.set(1);
  FMD.ScalarizedKernel.set(nullptr);
  FMD.VectorizedKernel.set(Clone);
  // Remove "ocl_recommended_vector_length" metadata
  MDValueGlobalObjectStrategy::unset(&F, "ocl_recommended_vector_length");
}

// Updates all the uses of TID calls with TID + new induction variable.
static void updateTID(IVecVec &TIDCalls, PHINode *Phi, BasicBlock *EntryBlock) {
  IRBuilder<> IRB(Phi);
  IRB.SetInsertPoint(Phi->getNextNode());
  // Currently, only zero dimension is vectorized.
  IVec zeroDimTIDCalls = TIDCalls[0];
  for (IVec::iterator tidIt = zeroDimTIDCalls.begin(),
                      tidE = zeroDimTIDCalls.end();
       tidIt != tidE; ++tidIt) {
    Instruction *TIDCallInstr = *tidIt;
    // Update the uses of the TID with TID+ind.
    Instruction *InductionSExt =
        cast<Instruction>(IRB.CreateSExtOrTrunc(Phi, TIDCallInstr->getType()));
    Instruction *Add = BinaryOperator::CreateNUWAdd(
        InductionSExt, UndefValue::get(InductionSExt->getType()), "add");
    Add->insertAfter(InductionSExt);
    TIDCallInstr->replaceAllUsesWith(Add);
    Add->setOperand(1, TIDCallInstr);
    // Move TID call outside of the loop.
    TIDCallInstr->moveBefore(EntryBlock->getTerminator());
  }
}

void OCLVecClone::handleLanguageSpecifics(Function &F, PHINode *Phi,
                                          Function *Clone,
                                          BasicBlock *EntryBlock) {
  IVecVec GlobalIDCalls;
  IVecVec LocalIDCalls;
  // Collect all get_local_id() and get_global_id() of all dimensions.
  std::string GID = CompilationUtils::mangledGetGID();
  std::string LID = CompilationUtils::mangledGetLID();
  LoopUtils::collectTIDCallInst(GID.c_str(), GlobalIDCalls, Clone);
  LoopUtils::collectTIDCallInst(LID.c_str(), LocalIDCalls, Clone);
  // Update the uses of the TID calls.
  updateTID(GlobalIDCalls, Phi, &*EntryBlock);
  updateTID(LocalIDCalls, Phi, &*EntryBlock);
  updateMetadata(F, Clone);
}

void OCLVecClone::languageSpecificInitializations(Module &M) {
  OCLPrepareKernelForVecClone PK(CPUId);

  auto Kernels = KernelList(*&M).getList();

  // Checks for some common module errors.
  if (Kernels.empty()) {
    V_PRINT(wrapper, "Failed to find kernel annotation. Aborting!\n");
    return;
  }

  unsigned NumOfKernels = Kernels.size();
  if (NumOfKernels == 0) {
    V_PRINT(wrapper, "Num of kernels is 0. Aborting!\n");
    return;
  }

  for (Function *F : Kernels) {
    auto MD = KernelMetadataAPI(F);
    auto VecTypeHint = MD.VecTypeHint;
    auto VecLenHint = MD.VecLenHint;
    bool EnableVect = true;

    if (isSimpleFunction(F))
      EnableVect = false;

    // Looks for vector type in metadata.
    if (!VecLenHint.hasValue() && VecTypeHint.hasValue()) {
      Type *VTHTy = VecTypeHint.getType();
      if (!VTHTy->isFloatTy() && !VTHTy->isDoubleTy() &&
          !VTHTy->isIntegerTy(8) && !VTHTy->isIntegerTy(16) &&
          !VTHTy->isIntegerTy(32) && !VTHTy->isIntegerTy(64)) {
        EnableVect = false;
      }
    }

    if (EnableVect)
      PK.run(F);
  }
}
} // namespace intel

extern "C" Pass *createOCLVecClonePass(const Intel::CPUId *CPUId,
                                       bool EnableVPlanVecForOpenCL) {
  return new intel::OCLVecClone(CPUId, EnableVPlanVecForOpenCL);
}
