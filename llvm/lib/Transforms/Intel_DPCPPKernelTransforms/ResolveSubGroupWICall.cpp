//= ResolveSubGroupWICall.cpp - Resolve DPC++ kernel subgroup work-item call =//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ResolveSubGroupWICall.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ResolveWICall.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/WGBoundDecoder.h"
#include <map>
#include <utility>

#define DEBUG_TYPE "dpcpp-kernel-resolve-sub-group-wi-call"

using namespace llvm;
using namespace DPCPPKernelMetadataAPI;
using namespace DPCPPKernelCompilationUtils;

namespace {
/// Legacy ResolveSubGroupWICall pass.
class ResolveSubGroupWICallLegacy : public ModulePass {
public:
  static char ID;

  ResolveSubGroupWICallLegacy(const SmallVector<Module *, 2> &BuiltinModules =
                                  SmallVector<Module *, 2>(),
                              bool ResolveSGBarrier = true);

  StringRef getPassName() const override {
    return "ResolveSubGroupWICallLegacy";
  }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
  }

private:
  ResolveSubGroupWICallPass Impl;
};
} // namespace

char ResolveSubGroupWICallLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(ResolveSubGroupWICallLegacy, DEBUG_TYPE,
                      "Resolve Sub Group WI functions", false, false)
INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
INITIALIZE_PASS_END(ResolveSubGroupWICallLegacy, DEBUG_TYPE,
                    "Resolve Sub Group WI functions", false, false)

ResolveSubGroupWICallLegacy::ResolveSubGroupWICallLegacy(
    const SmallVector<Module *, 2> &BuiltinModules, bool ResolveSGBarrier)
    : ModulePass(ID), Impl(BuiltinModules, ResolveSGBarrier) {
  initializeResolveSubGroupWICallLegacyPass(*PassRegistry::getPassRegistry());
}

bool ResolveSubGroupWICallLegacy::runOnModule(Module &M) {
  BuiltinLibInfo *BLI =
      &getAnalysis<BuiltinLibInfoAnalysisLegacy>().getResult();
  return Impl.runImpl(M, BLI);
}

ModulePass *llvm::createResolveSubGroupWICallLegacyPass(
    const SmallVector<Module *, 2> &BuiltinModules, bool ResolveSGBarrier) {
  return new ResolveSubGroupWICallLegacy(BuiltinModules, ResolveSGBarrier);
}

ResolveSubGroupWICallPass::ResolveSubGroupWICallPass(
    const SmallVector<Module *, 2> &, bool ResolveSGBarrier)
    : ResolveSGBarrier(ResolveSGBarrier) {}

PreservedAnalyses ResolveSubGroupWICallPass::run(Module &M,
                                                 ModuleAnalysisManager &AM) {
  BuiltinLibInfo *BLI = &AM.getResult<BuiltinLibInfoAnalysis>(M);
  if (!runImpl(M, BLI))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

bool ResolveSubGroupWICallPass::runImpl(Module &M, BuiltinLibInfo *BLI) {
  RTService = BLI->getRuntimeService();
  assert(RTService && "Invalid runtime service");

  // Get all kernels.
  FuncSet Kernels = getAllKernels(M);

  std::map<Function *, Function *> EarlyExitFuncToKernelMap;

  for (Function *Kernel : Kernels) {
    std::string EarlyExitFuncName =
        WGBoundDecoder::encodeWGBound(Kernel->getName());
    auto *EarlyExitFunc = M.getFunction(EarlyExitFuncName);
    if (EarlyExitFunc) {
      EarlyExitFuncToKernelMap[EarlyExitFunc] = Kernel;
      LLVM_DEBUG(dbgs() << "Found boundary function: " << EarlyExitFuncName
                        << "\n");
    }
  }

  typedef Value *(ResolveSubGroupWICallPass::*ResolverFunc)(Instruction *,
                                                            Value *, int32_t);
  std::map<std::string, ResolverFunc> SubGroupFuncToResolverMap = {
      {mangledNumSubGroups(),
       &ResolveSubGroupWICallPass::replaceGetNumSubGroups},
      {mangledGetSubGroupId(),
       &ResolveSubGroupWICallPass::replaceGetSubGroupId},
      {mangledEnqueuedNumSubGroups(),
       &ResolveSubGroupWICallPass::replaceGetEnqueuedNumSubGroups},
      {mangledGetSubGroupSize(),
       &ResolveSubGroupWICallPass::replaceGetSubGroupSize},
      {mangledGetMaxSubGroupSize(),
       &ResolveSubGroupWICallPass::replaceGetMaxSubGroupSize}};

  // Add resolvers for subgroup slice builtins.
  SmallPtrSet<Function *, 2> SubGroupRowSliceInsertElementFuncs;
  for (auto &F : M) {
    if (!F.isDeclaration())
      continue;
    StringRef FnName = F.getName();
    if (isGetSubGroupSliceLength(FnName))
      SubGroupFuncToResolverMap.insert(
          {FnName.str(),
           &ResolveSubGroupWICallPass::replaceGetSubGroupSliceLength});
    if (isSubGroupRowSliceExtractElement(FnName))
      SubGroupFuncToResolverMap.insert(
          {FnName.str(),
           &ResolveSubGroupWICallPass::replaceSubGroupRowSliceExtractElement});
    if (isSubGroupInsertRowSliceToMatrix(FnName))
      SubGroupFuncToResolverMap.insert(
          {FnName.str(),
           &ResolveSubGroupWICallPass::replaceSubGroupInsertRowSliceToMatrix});
    // We don't parse sub_group_rowslice_insertelement directly, but we need to
    // clean-up dead sub_group_rowslice_insertelement calls (who receive an
    // undef value as the rowslice id argument) at the end.
    if (isSubGroupRowSliceInsertElement(FnName))
      SubGroupRowSliceInsertElementFuncs.insert(&F);
  }

  LLVM_DEBUG(dbgs() << "ResolveSGBarrier = " << ResolveSGBarrier << '\n');
  if (ResolveSGBarrier) {
    SubGroupFuncToResolverMap.insert(
        {mangledSGBarrier(BarrierType::WithScope),
         &ResolveSubGroupWICallPass::replaceSubGroupBarrier});
    SubGroupFuncToResolverMap.insert(
        {mangledSGBarrier(BarrierType::NoScope),
         &ResolveSubGroupWICallPass::replaceSubGroupBarrier});
  }

  // Collect all called subgroup functions.
  FuncSet SubGroupFunctions;
  for (const auto &SGItem : SubGroupFuncToResolverMap) {
    if (Function *F = M.getFunction(SGItem.first))
      SubGroupFunctions.insert(F);
  }

  FuncSet FuncsToBePatched;
  std::set<CallInst *> CallsToBeFixed;
  // Hold the VF for every kernel.
  std::map<Function *, std::pair<Value *, int32_t>> KernelToVecInfoMap;
  // Hold the map from original function to patched function.
  std::map<Function *, Function *> OrigToPatchedFuncMap;

  // Collect all functions which directly/indiectly call subgroups functions.
  DPCPPKernelLoopUtils::fillFuncUsersSet(SubGroupFunctions, FuncsToBePatched);

  for (Function *OrigFunc : FuncsToBePatched) {
    if (EarlyExitFuncToKernelMap.count(OrigFunc))
      OrigFunc = EarlyExitFuncToKernelMap[OrigFunc];
    if (Kernels.count(OrigFunc)) {
      // Get the VF for kernel.
      auto KIMD = KernelInternalMetadataAPI(OrigFunc);
      auto VF = 1;
      if (KIMD.VectorizedWidth.hasValue())
        VF = KIMD.VectorizedWidth.get();
      else if (KIMD.SubgroupEmuSize.hasValue())
        VF = KIMD.SubgroupEmuSize.get();

      auto VecDim = KIMD.VectorizationDimension.hasValue()
                        ? KIMD.VectorizationDimension.get()
                        : 0;
      // TODO: After moving the computation related to VD to runtime, we should
      // remove this check and support other VDs in ChooseVectorizationDimension
      // when subgroups existing.
      assert(VecDim == 0 && "Only support vectorization at Dim 0");
      auto VectorizedKernelMetadata = KIMD.VectorizedKernel;
      auto *VecKernel = VectorizedKernelMetadata.hasValue()
                            ? VectorizedKernelMetadata.get()
                            : nullptr;
      if (VecKernel) {
        auto vkimd = KernelInternalMetadataAPI(VecKernel);
        VF = vkimd.VectorizedWidth.hasValue() ? vkimd.VectorizedWidth.get() : 1;
      }
      KernelToVecInfoMap.insert(
          {OrigFunc,
           {createVFConstant(M.getContext(), M.getDataLayout(), VF), VecDim}});
      LLVM_DEBUG(dbgs() << "Insert Vectorization Info for: ");
      LLVM_DEBUG(dbgs() << OrigFunc->getName());
      LLVM_DEBUG(dbgs() << " VF: " << VF << " VD: " << VecDim << "\n");
      // Kernels are not needed to be patched.
      continue;
    }

    // Collect the calls to be fixed. No need to handle kernels here since
    // DuplicateCalledKernelsPass runs before this pass.
    for (User *U : OrigFunc->users()) {
      if (CallInst *CI = dyn_cast<CallInst>(U)) {
        CallsToBeFixed.insert(CI);
        LLVM_DEBUG(dbgs() << "Call to be patched: " << *CI << "\n");
      }
    }

    // Patch the function by adding an arg to pass down the vectorization
    // factor.
    LLVM_DEBUG(dbgs() << "Patching function: " << OrigFunc->getName() << "\n");
    Type *IntTy = IntegerType::get(M.getContext(),
                                   M.getDataLayout().getPointerSizeInBits(0));
    Function *PatchedFunc =
        AddMoreArgsToFunc(OrigFunc, IntTy, "vf", None, "ResolveSubGroupWICall");
    OrigToPatchedFuncMap[OrigFunc] = PatchedFunc;
  }

  // Fix the calls for the patched functions.
  for (CallInst *CI : CallsToBeFixed) {
    Function *Caller = CI->getCaller();
    Function *Callee = CI->getCalledFunction();
    assert(OrigToPatchedFuncMap.find(Callee) != OrigToPatchedFuncMap.end() &&
           "Patching unexpected call");

    Value *ArgVF = nullptr;
    if (Kernels.count(Caller)) {
      assert(KernelToVecInfoMap.count(Caller) != 0 &&
             "No map for the kernel to vectorizer information!");
      ArgVF = KernelToVecInfoMap[Caller].first;
    } else if (EarlyExitFuncToKernelMap.count(Caller)) {
      assert(KernelToVecInfoMap.count(EarlyExitFuncToKernelMap[Caller]) != 0 &&
             "No map for the kernel to vectorizer information!");

      ArgVF = KernelToVecInfoMap[EarlyExitFuncToKernelMap[Caller]].first;
    } else {
      ArgVF = &*(Caller->arg_end() - 1);
    }
    LLVM_DEBUG(dbgs() << "Patching call: " << *CI << "\n");
    SmallVector<Value *, 1> NewArgs{ArgVF};
    AddMoreArgsToCall(CI, NewArgs, OrigToPatchedFuncMap[Callee]);
  }

  std::vector<std::pair<Instruction *, Value *>> InstRepVec;
  // Replace the subgroup calls.
  for (Function *SubGroupFunc : SubGroupFunctions) {
    ResolverFunc resolver =
        SubGroupFuncToResolverMap[std::string(SubGroupFunc->getName())];
    for (User *U : SubGroupFunc->users()) {
      assert(isa<CallInst>(U) && "Subgroup builtin isn't used in a call");
      CallInst *CI = cast<CallInst>(U);
      Function *Caller = CI->getCaller();
      LLVM_DEBUG(dbgs() << "Replacing sub-group call: " << *CI);
      LLVM_DEBUG(dbgs() << " in function: " << Caller->getName() << "\n");

      Value *VFVal = nullptr;
      if (Kernels.count(Caller)) {
        VFVal = KernelToVecInfoMap[Caller].first;
      } else if (EarlyExitFuncToKernelMap.count(Caller)) {
        VFVal = KernelToVecInfoMap[EarlyExitFuncToKernelMap[Caller]].first;
      } else {
        VFVal = &*(Caller->arg_end() - 1);
      }
      // TODO: Get VD from KernelToVecInfoMap.
      int32_t VD = 0;
      InstRepVec.emplace_back(CI, (this->*resolver)(CI, VFVal, VD));
    }
  }

  // Updating the usage of the new calculation
  for (const auto &pair : InstRepVec) {
    Instruction *from = pair.first;
    Value *to = pair.second;
    from->replaceAllUsesWith(to);
    from->eraseFromParent();
  }

  for (auto *I : ExtraInstToRemove)
    I->eraseFromParent();

  // sub_group_rowslice_insertelement calls are well parsed if they are linked
  // to an sub_group_insert_rowslice_to_matrix call via the rowslice id.
  // However, there might be some sub_group_rowslice_insertelement calls left
  // over, because they have an "undef" rowslice id argument.
  // e.g.
  // call void @_ZGVbM8uv_sub_group_rowslice_insertelement.i32(i64 undef, ...)
  // These calls are just no-ops. We need to make sure they are also removed.
  for (auto *F : SubGroupRowSliceInsertElementFuncs)
    for (auto *U : make_early_inc_range(F->users()))
      cast<CallInst>(U)->eraseFromParent();

  return !InstRepVec.empty();
}

Value *
ResolveSubGroupWICallPass::replaceGetSubGroupSize(Instruction *InsertBefore,
                                                  Value *VFVal, int32_t VD) {
  // get_sub_group_size will be replaced with:
  // non_uniform_size = get_local_size(VD) % VF
  // max_uniform_id = get_local_size(VD) - non_uniform_size
  // sub_group_size = get_local_id(VD) < max_uniform_id ? VF :
  //                                       non_uniform_size
  // Though get_local_id will be resolved as the id of first WI in the
  // sub group, it does not matter. Because we just use the id to
  // compare with max_uniform_id, any id within a sub group is ok.
  Module *M = InsertBefore->getModule();
  IRBuilder<> Builder(InsertBefore);
  Type *Int32Ty = Builder.getInt32Ty();
  std::string LocalSizeName = mangledGetLocalSize();
  std::string LocalIdName = mangledGetLID();

  auto *VecDimVar = ConstantInt::get(Int32Ty, VD);
  auto *LocalSize =
      createWIFunctionCall(M, "", LocalSizeName, InsertBefore, VecDimVar);
  ConstantInt *Zero =
      Builder.getIntN(M->getDataLayout().getPointerSizeInBits(), 0);
  auto *MinusVFVal =
      Builder.CreateBinOp(Instruction::Sub, Zero, VFVal, "minus.vf");
  auto *MaxUniformId = Builder.CreateBinOp(Instruction::And, MinusVFVal,
                                           LocalSize, "uniform.id.max");
  auto *NonUniformSize = Builder.CreateBinOp(Instruction::Sub, LocalSize,
                                             MaxUniformId, "nonuniform.size");
  auto *LocalId =
      createWIFunctionCall(M, "", LocalIdName, InsertBefore, VecDimVar);
  auto *Cond = Builder.CreateICmpULT(LocalId, MaxUniformId);
  auto *SGSize = Builder.CreateSelect(Cond, VFVal, NonUniformSize);
  auto *Ret = Builder.CreateTruncOrBitCast(SGSize, Int32Ty, "subgroup.size");
  return Ret;
}

Value *ResolveSubGroupWICallPass::replaceGetMaxSubGroupSize(
    Instruction *InsertBefore, Value *VFVal, int32_t /*VD*/) {
  // Replace get_max_sub_group_size with the constant (like "i32 16" if the max
  // subgroup size is 16), which is equal to trunc instruction:
  //   %max.sg.size = trunc i64 16 to i32
  IRBuilder<> Builder(InsertBefore);
  return Builder.CreateTruncOrBitCast(VFVal, Builder.getInt32Ty(),
                                      "max.sg.size");
}

Value *ResolveSubGroupWICallPass::replaceGetEnqueuedNumSubGroups(
    Instruction *InsertBefore, Value *VFVal, int32_t VD) {
  Module *M = InsertBefore->getModule();

  // Replace get_enqueued_num_sub_groups with the following sequence:
  // Let x be the dimension over which the vectorization has happened. x is in
  // {0,1,2}. Let y and z be the other dimensions. y,z are in {0,1,2}\{x}. Let
  // VF be the vector factor. Then the number of enqueued subgroups is:
  // ((get_enqueued_local_size(x) - 1) / VF ) + 1)
  //   * get_enqueued_local_size(y) * get_enqueued_local_size(z).
  std::string EnqdLocalSizeName = mangledGetEnqueuedLocalSize();

  IRBuilder<> Builder(InsertBefore);
  auto *EnqdLsz0 = createWIFunctionCall(M, "enqdlz0", EnqdLocalSizeName,
                                        InsertBefore, Builder.getInt32(0));
  auto *EnqdLsz1 = createWIFunctionCall(M, "enqdlz1", EnqdLocalSizeName,
                                        InsertBefore, Builder.getInt32(1));
  auto *EnqdLsz2 = createWIFunctionCall(M, "enqdlz2", EnqdLocalSizeName,
                                        InsertBefore, Builder.getInt32(2));
  std::vector<Value *> EnqdLszs = {EnqdLsz0, EnqdLsz1, EnqdLsz2};
  ConstantInt *One =
      Builder.getIntN(M->getDataLayout().getPointerSizeInBits(), 1);
  auto *Op0 = Builder.CreateBinOp(Instruction::Sub, EnqdLszs[VD], One);
  auto *Op1 = Builder.CreateBinOp(Instruction::UDiv, Op0, VFVal);
  auto *Op2 =
      Builder.CreateBinOp(Instruction::Add, Op1, One, "sg.num.vecdim.enqd");
  EnqdLszs[VD] = Op2;
  auto *SGNumOp0 =
      Builder.CreateBinOp(Instruction::Mul, EnqdLszs[0], EnqdLszs[1]);
  auto *SGNumOp1 = Builder.CreateBinOp(Instruction::Mul, SGNumOp0, EnqdLszs[2]);
  auto *Res = Builder.CreateTruncOrBitCast(
      SGNumOp1, Type::getInt32Ty(M->getContext()), "sg.num.enqd");
  return Res;
}

Value *
ResolveSubGroupWICallPass::replaceGetNumSubGroups(Instruction *InsertBefore,
                                                  Value *VFVal, int32_t VD) {
  // Replace get_num_sub_groups() with the following sequence:
  // Improve the sequence for 2D/3D case:
  // Let x be the dimension over which the vectorization has happened. x is in
  // {0,1,2}. Let y and z be the other dimensions. y,z are in {0,1,2}\{x}. Let
  // VF be the vector factor. Then the number of subgroups is:
  // ((get_local_size(x) - 1) / VF ) + 1) * get_local_size(y) *
  // get_local_size(z).
  std::string LocalSizeName = mangledGetLocalSize();
  Module *M = InsertBefore->getModule();
  IRBuilder<> Builder(InsertBefore);
  auto *Lsz0 = createWIFunctionCall(M, "lsz0", LocalSizeName, InsertBefore,
                                    Builder.getInt32(0));
  auto *Lsz1 = createWIFunctionCall(M, "lsz1", LocalSizeName, InsertBefore,
                                    Builder.getInt32(1));
  auto *Lsz2 = createWIFunctionCall(M, "lsz2", LocalSizeName, InsertBefore,
                                    Builder.getInt32(2));

  std::vector<Value *> Lszs = {Lsz0, Lsz1, Lsz2};
  ConstantInt *One =
      Builder.getIntN(M->getDataLayout().getPointerSizeInBits(), 1);
  auto *Op0 = Builder.CreateBinOp(Instruction::Sub, Lszs[VD], One);
  auto *Op1 = Builder.CreateBinOp(Instruction::UDiv, Op0, VFVal);
  auto *Op2 = Builder.CreateBinOp(Instruction::Add, Op1, One, "sg.num.vecdim");
  Lszs[VD] = Op2;
  auto *SGNumOp0 = Builder.CreateBinOp(Instruction::Mul, Lszs[0], Lszs[1]);
  auto *SGNumOp1 = Builder.CreateBinOp(Instruction::Mul, SGNumOp0, Lszs[2]);
  auto *Res = Builder.CreateTruncOrBitCast(
      SGNumOp1, Type::getInt32Ty(M->getContext()), "sg.num");
  return Res;
}

Value *
ResolveSubGroupWICallPass::replaceGetSubGroupId(Instruction *InsertBefore,
                                                Value *VFVal, int32_t VD) {
  Module *M = InsertBefore->getModule();
  IRBuilder<> Builder(InsertBefore);

  // Assume vectorization dimension is 0
  // Replace get_sub_group_id() with the following sequence:
  // (get_local_id(2) * get_local_size(1) + get_local_id(1)) *
  // ((get_local_size(0) - 1) / VF + 1) + get_local_id(0) / VF
  std::vector<int32_t> Dims;
  Dims.push_back(VD);
  for (int32_t Dim = 0; Dim < 3; ++Dim)
    if (Dim != VD)
      Dims.push_back(Dim);

  std::string IdName = mangledGetLID();
  std::string SizeName = mangledGetLocalSize();

  // call get_local_id(2), get_local_id(1), get_local_id(0)
  CallInst *Lid2 = createWIFunctionCall(M, "lid2", IdName, InsertBefore,
                                        Builder.getInt32(2));
  CallInst *Lid1 = createWIFunctionCall(M, "lid1", IdName, InsertBefore,
                                        Builder.getInt32(1));
  CallInst *Lid0 = createWIFunctionCall(M, "lid0", IdName, InsertBefore,
                                        Builder.getInt32(0));
  assert(Lid0 && Lid1 && Lid2 && "Can't create get_local_id calls");

  // call get_local_size(2), get_local_size(1), get_local_size(0)
  CallInst *Lsz2 = createWIFunctionCall(M, "lsz2", SizeName, InsertBefore,
                                        Builder.getInt32(2));
  CallInst *Lsz1 = createWIFunctionCall(M, "lsz1", SizeName, InsertBefore,
                                        Builder.getInt32(1));
  CallInst *Lsz0 = createWIFunctionCall(M, "lsz0", SizeName, InsertBefore,
                                        Builder.getInt32(0));
  assert(Lsz0 && Lsz1 && Lsz2 && "Can't create get_local_size calls");

  std::vector<Value *> Lids = {Lid0, Lid1, Lid2};
  Lids = {Lids[Dims[0]], Lids[Dims[1]], Lids[Dims[2]]};

  std::vector<Value *> Lszs = {Lsz0, Lsz1, Lsz2};
  Lszs = {Lszs[Dims[0]], Lszs[Dims[1]], Lszs[Dims[2]]};

  auto *Op0 =
      Builder.CreateBinOp(Instruction::Mul, Lids[2], Lszs[1], "sg.id.op0");
  auto *Op1 = Builder.CreateBinOp(Instruction::Add, Op0, Lids[1], "sg.id.op1");

  ConstantInt *One =
      Builder.getIntN(M->getDataLayout().getPointerSizeInBits(), 1);
  auto *Op2 = Builder.CreateBinOp(Instruction::Sub, Lszs[0], One, "sg.id.op2");
  auto *Op3 = Builder.CreateBinOp(Instruction::UDiv, Op2, VFVal, "sg.id.op3");
  auto *Op4 = Builder.CreateBinOp(Instruction::Add, Op3, One, "sg.id.op4");
  auto *Op5 = Builder.CreateBinOp(Instruction::Mul, Op4, Op1, "sg.id.op5");
  auto *Op6 =
      Builder.CreateBinOp(Instruction::UDiv, Lids[0], VFVal, "sg.id.op6");
  auto *Op7 = Builder.CreateBinOp(Instruction::Add, Op5, Op6, "sg.id.res");
  auto *Res = Builder.CreateTruncOrBitCast(Op7, Builder.getInt32Ty(),
                                           "sg.id.res.trunc");
  return Res;
}

Value *
ResolveSubGroupWICallPass::replaceSubGroupBarrier(Instruction *InsertBefore,
                                                  Value *, int32_t) {
  Module *M = InsertBefore->getModule();
  IRBuilder<> Builder(InsertBefore);
  CallInst *CI = cast<CallInst>(InsertBefore);
  std::string AtomicWIFenceName = mangledAtomicWorkItemFence();
  auto *AtomicWIFenceBIF =
      RTService->findFunctionInBuiltinModules(AtomicWIFenceName);
  assert(AtomicWIFenceBIF && "atomic_work_item_fence not found in BI library!");

  auto *AtomicWIFenceF = importFunctionDecl(M, AtomicWIFenceBIF);
  assert(AtomicWIFenceF && "Failed generating function in current module");

  // take mem_fence from the barrier
  assert((CI->arg_size() >= 1) &&
         "Expect sub_group_barrier to have at least mem fence argument!");
  Value *MemFence = CI->getArgOperand(0);
  // Obtain MemoryOrder.
  // must be aligned with clang preprocessor foir
  // __ATOMIC_ACQ_REL
  const uint64_t MemoryOrderAcqRel = 4;
  Value *MemOrder = Builder.getInt32(MemoryOrderAcqRel);
  // obtain mem_scope.
  Value *MemScope = nullptr;
  if (CI->arg_size() == 2)
    // take memory scope from the barrier.
    MemScope = CI->getArgOperand(1);
  else {
    // must be aligned with clang preprocessor for
    // __OPENCL_MEMORY_SCOPE_SUB_GROUP.
    const uint64_t MemoryScopeSubGroup = 4;
    MemScope = Builder.getInt32(MemoryScopeSubGroup);
  }

  SmallVector<Value *, 3> Args;
  Args.push_back(MemFence);
  Args.push_back(MemOrder);
  Args.push_back(MemScope);

  return Builder.CreateCall(AtomicWIFenceF, Args, "");
}

// FIXME: Support subgroup rowslice emulation (-O0), so that the resolved value
// of get_sub_group_slice_length.() matches the actual widen size.
// Currently in the subgroup emulation path, VFVal would be emulation size,
// while other slice builtins will assume VF == RowSliceLength == 1.
Value *ResolveSubGroupWICallPass::replaceGetSubGroupSliceLength(
    Instruction *InsertBefore, Value *VFVal, int32_t) {
  auto *CI = cast<CallInst>(InsertBefore);
  unsigned VF = cast<ConstantInt>(VFVal)->getZExtValue();
  unsigned TotalElementCount =
      cast<ConstantInt>(CI->getArgOperand(0))->getZExtValue();
  assert(VF != 0);
  // ceil(TotalElementCount / VF)
  return ConstantInt::get(CI->getType(), (TotalElementCount + VF - 1) / VF);
}

void ResolveSubGroupWICallPass::resolveGetSubGroupRowSliceId(
    Value *RowSliceId, unsigned RowSliceLength, IRBuilder<> &Builder,
    SmallVectorImpl<Value *> &ParsedArgs) {
  CallInst *CI = nullptr;
  // After vectorization, the rowslice id might become a PHI node selecting
  // between the get_sub_group_rowslice_id call and an undef value.
  // clang-format off
  // e.g.
  // pred.call.if69vector_func:                        ; preds = %VPlannedBB30vector_func
  //   %57 = call i64 @get_sub_group_rowslice_id.v256i32.i64(<256 x i32> %extractsubvec.vector_func, i32 16, i32 16, i64 %uni.phi18vector_func) #8
  //   br label %pred.call.continue70vector_func
  // pred.call.continue70vector_func:                  ; preds = %pred.call.if69vector_func, %VPlannedBB30vector_func
  //   %58 = phi i64 [ undef, %VPlannedBB30vector_func ], [ %57, %pred.call.if69vector_func ]
  //   %59 = call <8 x i32> @_ZGVbM8u_sub_group_rowslice_extractelement.i32(i64 %58, <8 x i32> %maskext32vector_func) #10
  // clang-format on
  if (auto *PHI = dyn_cast<PHINode>(RowSliceId)) {
    // If we ignore the undef value, the result of the PHI must be the the
    // get_sub_group_rowslice_id call,
    assert(PHI->hasConstantOrUndefValue() &&
           "The incoming value of the PHI representing the rowslice id must be "
           "either 'undef' or a get_sub_group_rowslice_id call");
    // Find the get_sub_group_rowslice_id call.
    for (auto &Incoming : PHI->incoming_values())
      if (CI = dyn_cast<CallInst>(Incoming.get()))
        break;
    assert(CI && "get_sub_group_rowslice_id call is not found");
    // We need to remove the PHI node (before removing the
    // get_sub_group_rowslice_id call inst).
    ExtraInstToRemove.push_back(PHI);
  } else {
    CI = cast<CallInst>(RowSliceId);
  }
  // %id = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32
  //   12, i32 12, i64 %element.index)
  assert(CI->arg_size() == 4 &&
         "A get_sub_group_rowslice_id call must have exactly 4 args.");
  auto *Matrix = CI->getArgOperand(0);
  assert(DPCPPKernelCompilationUtils::isValidMatrixType(
             cast<FixedVectorType>(Matrix->getType())) &&
         "Unsupported matrix type");
  unsigned R = cast<ConstantInt>(CI->getArgOperand(1))->getZExtValue();
  unsigned C = cast<ConstantInt>(CI->getArgOperand(2))->getZExtValue();
  auto *Index = CI->getArgOperand(3);
  auto *Index32 = Builder.CreateSExtOrTrunc(Index, Builder.getInt32Ty());
  auto *BaseId = Builder.CreateNSWMul(Index32, Builder.getInt32(RowSliceLength),
                                      "rowslice.baseid");
  auto *RowIndex =
      Builder.CreateUDiv(BaseId, Builder.getInt32(C), "rowslice.row.index");
  auto *ColIndex =
      Builder.CreateURem(BaseId, Builder.getInt32(C), "rowslice.col.index");
  ParsedArgs.append({Matrix, RowIndex, ColIndex,
                     Builder.getInt32(RowSliceLength), Builder.getInt32(R),
                     Builder.getInt32(C)});
  // FIXME: Pass layout metadata from SPIR-V. Hardcode as rowmajor for now.
  ParsedArgs.push_back(MetadataAsValue::get(
      CI->getContext(), MDString::get(CI->getContext(), "matrix.rowmajor")));
  // We need to remove the get_sub_group_rowslice_id call.
  ExtraInstToRemove.push_back(CI);
}

Value *ResolveSubGroupWICallPass::replaceSubGroupRowSliceExtractElement(
    Instruction *InsertBefore, Value *, int32_t) {
  auto *ValType = InsertBefore->getType();
  unsigned RowSliceLength = 1;
  if (auto *VecValType = dyn_cast<FixedVectorType>(ValType))
    RowSliceLength = VecValType->getNumElements();
  // %id = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32
  //   12, i32 12, i64 %element.index)
  // %elem = call i32 @sub_group_rowslice_extractelement.i32(i64 %id)
  // or
  // %elem = call <16 x i32>
  //   @_ZGVbN16u_sub_group_rowslice_extractelement.i32(i64 %id)
  auto *CI = cast<CallInst>(InsertBefore);
  auto *RowSliceId = CI->getArgOperand(0);
  assert(RowSliceId->hasNUses(1) &&
         "Each rowslice id value must be used only once");
  IRBuilder<> Builder(InsertBefore);
  SmallVector<Value *, 8> Args;
  resolveGetSubGroupRowSliceId(RowSliceId, RowSliceLength, Builder, Args);
  auto *RowSliceType =
      FixedVectorType::get(ValType->getScalarType(), RowSliceLength);
  auto *MatrixType = Args.front()->getType();
  auto *RowSlice =
      Builder.CreateIntrinsic(Intrinsic::experimental_matrix_extract_row_slice,
                              {RowSliceType, MatrixType}, Args);
  // Add an extra extractelement from <1 x T> for the scalar case.
  if (RowSliceLength == 1)
    return Builder.CreateExtractElement(RowSlice, Builder.getInt32(0));
  return RowSlice;
}

Value *ResolveSubGroupWICallPass::replaceSubGroupInsertRowSliceToMatrix(
    Instruction *InsertBefore, Value *, int32_t) {
  // %id = call i64 @get_sub_group_rowslice_id.v144i32.i64(<144 x i32> %mat, i32
  //   12, i32 12, i64 %element.index)
  // call void @sub_group_rowslice_insertelement.i32(i64 %id, i32 %val)
  // %mat.update = call <144 x i32>
  //   @sub_group_insert_rowslice_to_matrix.v144i32(i64 %id)
  // or
  // call void
  //   @_ZGVbN16u_sub_group_rowslice_insertelement.i32(i64 %id, <16 x i32> %val)
  // %mat.update = call <144 x i32>
  //   @sub_group_insert_rowslice_to_matrix.v144i32(i64 %id)
  auto *CI = cast<CallInst>(InsertBefore);
  auto *RowSliceId = CI->getArgOperand(0);
  assert(RowSliceId->hasNUses(2) &&
         "Each rowslice id value must be used exactly twice");
  CallInst *SGRowSliceInsertElementCall = nullptr;
  for (auto *U : RowSliceId->users()) {
    auto *I = cast<CallInst>(U);
    if (DPCPPKernelCompilationUtils::isSubGroupRowSliceInsertElement(
            I->getCalledFunction()->getName())) {
      SGRowSliceInsertElementCall = I;
      break;
    }
  }
  assert(SGRowSliceInsertElementCall &&
         "Doesn't find sub_group_rowslice_insertelement use on rowslice id");
  // The sub_group_rowslice_insertelement call needs to be removed first.
  ExtraInstToRemove.push_back(SGRowSliceInsertElementCall);

  auto *SliceData = SGRowSliceInsertElementCall->getArgOperand(1);
  unsigned RowSliceLength = 1;
  auto *ValType = SliceData->getType();
  if (auto *VecValType = dyn_cast<FixedVectorType>(ValType))
    RowSliceLength = VecValType->getNumElements();
  IRBuilder<> Builder(InsertBefore);
  auto *RowSliceType =
      FixedVectorType::get(ValType->getScalarType(), RowSliceLength);
  // Create an extra insertelement into <1 x T> in the scalar case.
  if (RowSliceLength == 1)
    SliceData = Builder.CreateInsertElement(UndefValue::get(RowSliceType),
                                            SliceData, Builder.getInt32(0));
  SmallVector<Value *, 8> Args;
  resolveGetSubGroupRowSliceId(RowSliceId, RowSliceLength, Builder, Args);
  // The second arg should be the data to be inserted.
  Args.insert(Args.begin() + 1, SliceData);
  auto *MatrixType = InsertBefore->getType();
  auto *Matrix =
      Builder.CreateIntrinsic(Intrinsic::experimental_matrix_insert_row_slice,
                              {MatrixType, RowSliceType}, Args);
  return Matrix;
}

ConstantInt *ResolveSubGroupWICallPass::createVFConstant(LLVMContext &C,
                                                         const DataLayout &DL,
                                                         size_t VF) {
  assert(((DL.getPointerSizeInBits() == 64) ||
          (DL.getPointerSizeInBits() == 32)) &&
         "Unexpected ptr size!");
  return ConstantInt::get(Type::getIntNTy(C, DL.getPointerSizeInBits()), VF);
}

CallInst *ResolveSubGroupWICallPass::createWIFunctionCall(
    Module *M, char const *ValueName, std::string const &FuncName,
    Instruction *InsertBefore, Value *ActPar) {
  Function *Func = M->getFunction(FuncName);
  IRBuilder<> Builder(InsertBefore);
  // if the function does not exist then we need to create it
  if (!Func) {
    std::vector<Type *> Args(1, ActPar->getType());
    Type *IntTy = Builder.getIntNTy(M->getDataLayout().getPointerSizeInBits(0));
    FunctionType *FType = FunctionType::get(IntTy, Args, false);
    Func = cast<Function>(M->getOrInsertFunction(FuncName, FType).getCallee());
  }

  return Builder.CreateCall(Func, ActPar, ValueName);
}
