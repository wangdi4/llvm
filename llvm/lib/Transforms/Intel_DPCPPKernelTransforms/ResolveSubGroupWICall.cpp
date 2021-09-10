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
#include "RuntimeService.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ResolveWICall.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
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

private:
  ResolveSubGroupWICallPass Impl;
};
} // namespace

char ResolveSubGroupWICallLegacy::ID = 0;

INITIALIZE_PASS(ResolveSubGroupWICallLegacy, DEBUG_TYPE,
                "Resolve Sub Group WI functions", false, false)

ResolveSubGroupWICallLegacy::ResolveSubGroupWICallLegacy(
    const SmallVector<Module *, 2> &BuiltinModules, bool ResolveSGBarrier)
    : ModulePass(ID), Impl(BuiltinModules, ResolveSGBarrier) {
  initializeResolveSubGroupWICallLegacyPass(*PassRegistry::getPassRegistry());
}

bool ResolveSubGroupWICallLegacy::runOnModule(Module &M) {
  return Impl.runImpl(M);
}

ModulePass *llvm::createResolveSubGroupWICallLegacyPass(
    const SmallVector<Module *, 2> &BuiltinModules, bool ResolveSGBarrier) {
  return new ResolveSubGroupWICallLegacy(BuiltinModules, ResolveSGBarrier);
}

ResolveSubGroupWICallPass::ResolveSubGroupWICallPass(
    const SmallVector<Module *, 2> &BuiltinModules, bool ResolveSGBarrier)
    : ResolveSGBarrier(ResolveSGBarrier), BuiltinModules(BuiltinModules) {}

PreservedAnalyses ResolveSubGroupWICallPass::run(Module &M,
                                                 ModuleAnalysisManager &) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

bool ResolveSubGroupWICallPass::runImpl(Module &M) {
  SmallVector<std::unique_ptr<Module>, 2> BIModuleList;
  if (BuiltinModules.empty()) {
    BIModuleList = loadBuiltinModulesFromCommandLine(M.getContext());
    transform(BIModuleList, std::back_inserter(BuiltinModules),
              [](auto &BIModule) { return BIModule.get(); });
  }

  // Get all kernels.
  FuncSet Kernels = getAllKernels(M);

  std::map<Function *, Function *> EarlyExitFuncToKernelMap;

  for (Function *Kernel : Kernels) {
    std::string EarlyExitFuncName =
        appendWorkGroupBoundariesPrefix(Kernel->getName());
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
      ArgVF = KernelToVecInfoMap[Caller].first;
    } else if (EarlyExitFuncToKernelMap.count(Caller)) {
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
  auto *AtomicWIFenceBIF = RuntimeService::findFunctionInBuiltinModules(
      BuiltinModules, AtomicWIFenceName);
  assert(AtomicWIFenceBIF && "atomic_work_item_fence not found in BI library!");

  auto *AtomicWIFenceF = importFunctionDecl(M, AtomicWIFenceBIF);
  assert(AtomicWIFenceF && "Failed generating function in current module");

  // take mem_fence from the barrier
  assert((CI->getNumArgOperands() >= 1) &&
         "Expect sub_group_barrier to have at least mem fence argument!");
  Value *MemFence = CI->getArgOperand(0);
  // Obtain MemoryOrder.
  // must be aligned with clang preprocessor foir
  // __ATOMIC_ACQ_REL
  const uint64_t MemoryOrderAcqRel = 4;
  Value *MemOrder = Builder.getInt32(MemoryOrderAcqRel);
  // obtain mem_scope.
  Value *MemScope = nullptr;
  if (CI->getNumArgOperands() == 2)
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
