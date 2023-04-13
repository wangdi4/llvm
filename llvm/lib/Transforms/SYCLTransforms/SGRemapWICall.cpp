//==------- SGRemapWICall.cpp - Remap work-item function calls ------ C++ -==//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// ===--------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/SGRemapWICall.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"

using namespace llvm;
using namespace CompilationUtils;

#define DEBUG_TYPE "sycl-kernel-sg-remap-wi-call"

static cl::opt<SubGroupConstructionMode> SYCLRemapWIMode(
    "sycl-sg-construction-mode", cl::init(SubGroupConstructionMode::X),
    cl::Hidden, cl::desc("Sub-group construction mode"),
    cl::values(clEnumValN(SubGroupConstructionMode::Linear, "linear",
                          "Construct sub-groups in linearized work-group"),
               clEnumValN(SubGroupConstructionMode::X, "0",
                          "Construct sub-groups in 0th dimension"),
               clEnumValN(SubGroupConstructionMode::Y, "1",
                          "Construct sub-groups in 1st dimension"),
               clEnumValN(SubGroupConstructionMode::Z, "2",
                          "Construct sub-groups in 2nd dimension")));

/// Whether to skip processing this module.
static bool canSkip(Module &M, SubGroupConstructionMode RemapMode) {
  // FIXME:
  // Check whether none of the kernels requires a special remapping mode.
  (void)M;

  // Default mode, we can skip running this pass.
  return RemapMode == SubGroupConstructionMode::X;
}

static Function *getOrInsertWIFunction(Module &M, FunctionType *FTy,
                                       StringRef Name) {
  auto *F = cast<Function>(M.getOrInsertFunction(Name, FTy).getCallee());
  F->setDoesNotAccessMemory();
  return F;
}

/// Translate "get_global_id(dim)" as
/// "get_local_id(dim) + get_group_id(dim) * get_local_size(dim)".
static bool translateGetGID(Module &M) {
  auto *GetGIDFunc = M.getFunction(mangledGetGID());
  if (!GetGIDFunc)
    return false;
  bool Changed = false;
  auto *FuncType = GetGIDFunc->getFunctionType();
  Function *GetLIDFunc = nullptr;
  Function *GetGroupIDFunc = nullptr;
  Function *GetLocalSizeFunc = nullptr;
  for (auto *U : make_early_inc_range(GetGIDFunc->users())) {
    auto *CI = dyn_cast<CallInst>(U);
    if (!CI)
      continue;
    if (!GetLIDFunc)
      GetLIDFunc = getOrInsertWIFunction(M, FuncType, mangledGetLID());
    if (!GetGroupIDFunc)
      GetGroupIDFunc = getOrInsertWIFunction(M, FuncType, mangledGetGroupID());
    if (!GetLocalSizeFunc)
      GetLocalSizeFunc =
          getOrInsertWIFunction(M, FuncType, mangledGetLocalSize());
    auto *DimArg = CI->getArgOperand(0);
    IRBuilder<> Builder(CI);
    auto Name = CI->getName();
    auto *LID = Builder.CreateCall(GetLIDFunc, DimArg, "lid." + Name);
    auto *GroupID =
        Builder.CreateCall(GetGroupIDFunc, DimArg, "groupid." + Name);
    auto *LocalSize =
        Builder.CreateCall(GetLocalSizeFunc, DimArg, "lsize." + Name);
    auto *GroupOffset =
        Builder.CreateNUWMul(GroupID, LocalSize, "groupoffset." + Name);
    auto *Replacement =
        Builder.CreateNUWAdd(LID, GroupOffset, "translated." + Name);
    CI->replaceAllUsesWith(Replacement);
    CI->eraseFromParent();
    Changed = true;
  }
  return Changed;
}

/// Linearize get_local_id or get_group_id calls.
/// Formula:
/// - get_local_id(i32 dim)
///   --> get_local_id(0) / Prod{get_local_size(i) for i in [0, dim)}
///       % get_local_size(dim)
/// - get_group_id(i32 dim)
///   --> get_group_id(0) / Prod{get_num_groups(i) for i in [0, dim)}
///       % get_num_groups(dim)
static bool linearizeGetLIDOrGetGroupIDCalls(Module &M, bool IsGetLID) {
  auto *GetIDFunc =
      M.getFunction(IsGetLID ? mangledGetLID() : mangledGetGroupID());
  if (!GetIDFunc)
    return false;

  bool Changed = false;
  auto *FuncType = GetIDFunc->getFunctionType();
  Function *GetSizeFunc = nullptr;
  SmallVector<CallInst *, 16> CallInsts;
  for (auto *U : GetIDFunc->users())
    if (auto *CI = dyn_cast<CallInst>(U))
      CallInsts.push_back(CI);
  for (auto *CI : CallInsts) {
    if (!GetSizeFunc)
      GetSizeFunc = getOrInsertWIFunction(M, FuncType,
                                          IsGetLID ? mangledGetLocalSize()
                                                   : mangledGetNumGroups());
    // Variable dimindx argument should have been resolved by
    // ResolveVarTIDCall pass.
    auto *DimArg = cast<ConstantInt>(CI->getArgOperand(0));
    unsigned Dim = DimArg->getZExtValue();
    IRBuilder<> Builder(CI);
    auto Name = CI->getName();
    auto *LinearizedID = Builder.CreateCall(GetIDFunc, Builder.getInt32(0),
                                            "linearized." + Name);
    Value *LeadingDimSize = nullptr;
    for (unsigned I = 0; I < Dim; ++I) {
      auto *Size =
          Builder.CreateCall(GetSizeFunc, Builder.getInt32(I), "size." + Name);
      LeadingDimSize = LeadingDimSize
                           ? Builder.CreateNUWMul(LeadingDimSize, Size,
                                                  "leading.dim.size." + Name)
                           : Size;
    }
    auto *CurrentDimSize =
        Builder.CreateCall(GetSizeFunc, DimArg, "current.dim.size." + Name);
    auto *RecoveredCall = LeadingDimSize
                              ? Builder.CreateUDiv(LinearizedID, LeadingDimSize)
                              : LinearizedID;
    RecoveredCall =
        Builder.CreateURem(RecoveredCall, CurrentDimSize, "recovered." + Name);
    CI->replaceAllUsesWith(RecoveredCall);
    CI->eraseFromParent();
    Changed = true;
  }
  return Changed;
}

/// Rename the following get-size functions to their "user." variants:
/// - get_global_size
/// - get_local_size
/// - get_enqueued_local_size
/// - get_num_groups
static bool renameGetSizeFunctions(Module &M) {
  const static std::string FuncNames[] = {
      mangledGetGlobalSize(), mangledGetLocalSize(),
      mangledGetEnqueuedLocalSize(), mangledGetNumGroups()};

  bool Changed = false;
  for (auto &Name : FuncNames) {
    if (auto *F = M.getFunction(Name)) {
      LLVM_DEBUG(dbgs() << "Renaming function " << Name << " to ");
      assert(!M.getFunction(UserVariantPrefix.str() + Name) &&
             "Unexpected 'user.' variant!");
      Changed = true;
      F->setName(UserVariantPrefix + Name);
      LLVM_DEBUG(dbgs() << F->getName() << '\n');
    }
  }
  return Changed;
}

/// Change the dimindx argument of the following calls:
/// - get_global_id
/// - get_local_id
/// - get_group_id
static bool swapDimForGetIDCalls(Module &M, unsigned SwapWithZeroDim) {
  const static std::string GetIDFuncNames[] = {mangledGetGID(), mangledGetLID(),
                                               mangledGetGroupID()};
  if (SwapWithZeroDim == 0)
    return false;
  bool Changed = false;
  for (auto &FuncName : GetIDFuncNames) {
    auto *F = M.getFunction(FuncName);
    if (!F)
      continue;
    // Save all call instructions because we will create calls in the loop.
    SmallVector<CallInst *, 16> CallInsts;
    for (auto *U : make_early_inc_range(F->users())) {
      if (auto *CI = dyn_cast<CallInst>(U)) {
        // Variable dimindx argument should have been resolved by
        // ResolveVarTIDCall pass.
        auto *DimArg = cast<ConstantInt>(CI->getArgOperand(0));
        unsigned Dim = DimArg->getZExtValue();
        if (Dim != 0 && Dim != SwapWithZeroDim)
          continue;
        CallInsts.push_back(CI);
      }
    }
    for (auto *CI : CallInsts) {
      auto *DimArg = cast<ConstantInt>(CI->getArgOperand(0));
      unsigned Dim = DimArg->getZExtValue();
      unsigned TargetDim = Dim == 0 ? SwapWithZeroDim : 0;
      LLVM_DEBUG(dbgs() << "Swapping dimension of " << *CI << " to "
                        << TargetDim << '\n');
      IRBuilder<> Builder(CI);
      auto *SwappedCall = Builder.CreateCall(CI->getCalledFunction(),
                                             Builder.getInt32(TargetDim),
                                             "swapped." + CI->getName());
      CI->replaceAllUsesWith(SwappedCall);
      CI->eraseFromParent();
      Changed = true;
    }
  }
  return Changed;
}

static void updateKernelMetadata(Module &M,
                                 SubGroupConstructionMode RemapMode) {
  auto Kernels = getAllKernels(M);
  for (auto *K : Kernels) {
    auto KIMD = SYCLKernelMetadataAPI::KernelInternalMetadataAPI(K);
    KIMD.SubGroupConstructionMode.set(static_cast<int>(RemapMode));
  }
}

PreservedAnalyses SGRemapWICallPass::run(Module &M, ModuleAnalysisManager &AM) {
  if (SYCLRemapWIMode.getNumOccurrences())
    RemapMode = SYCLRemapWIMode;

  assert((RemapMode == SubGroupConstructionMode::Linear ||
          RemapMode == SubGroupConstructionMode::X ||
          RemapMode == SubGroupConstructionMode::Y ||
          RemapMode == SubGroupConstructionMode::Z) &&
         "Unknown subgroup construction mode!");

  if (canSkip(M, RemapMode))
    return PreservedAnalyses::all();

  bool Changed = false;
  if (RemapMode == SubGroupConstructionMode::Linear) {
    Changed |= translateGetGID(M);
    Changed |= linearizeGetLIDOrGetGroupIDCalls(M, true);
    Changed |= linearizeGetLIDOrGetGroupIDCalls(M, false);
  } else {
    Changed |= swapDimForGetIDCalls(M, static_cast<unsigned>(RemapMode));
  }
  // Replace get_*_size and get_num_groups with their "user." variant.
  Changed |= renameGetSizeFunctions(M);

  if (Changed)
    updateKernelMetadata(M, RemapMode);

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
