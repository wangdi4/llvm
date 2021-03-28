// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "ResolveSubGroupWICall.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "LoopUtils/LoopUtils.h"
#include "MetadataAPI.h"
#include "OCLPassSupport.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"

#include <map>
#include <utility>

#define DEBUG_TYPE "resolve-sub-group-wi-call"

using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::MetadataAPI;

extern "C" {
void *createResolveSubGroupWICallPass(bool ResolveSGBarrier) {
  return new intel::ResolveSubGroupWICall(ResolveSGBarrier);
}
}

namespace intel {

char ResolveSubGroupWICall::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(ResolveSubGroupWICall, DEBUG_TYPE,
                          "Resolve Sub Group WI functions", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(ResolveSubGroupWICall, DEBUG_TYPE,
                        "Resolve Sub Group WI functions", false, false)

ResolveSubGroupWICall::ResolveSubGroupWICall(bool ResolveSGBarrier)
    : ModulePass(ID), ResolveSGBarrier(ResolveSGBarrier), m_zero(nullptr),
      m_one(nullptr), m_two(nullptr), m_ret(nullptr), m_rtServices(nullptr) {}

bool ResolveSubGroupWICall::runOnModule(Module &M) {

  m_pModule = &M;

  m_rtServices = getAnalysis<BuiltinLibInfo>().getRuntimeServices();
  assert(m_rtServices && "m_rtServices should exist!");

  m_zero = ConstantInt::get(Type::getInt32Ty(M.getContext()), 0);
  m_one = ConstantInt::get(Type::getInt32Ty(M.getContext()), 1);
  m_two = ConstantInt::get(Type::getInt32Ty(M.getContext()), 2);

  m_ret = IntegerType::get(M.getContext(),
                           M.getDataLayout().getPointerSizeInBits(0));
  // Get all kernels.
  CompilationUtils::FunctionSet Kernels;
  CompilationUtils::getAllKernels(Kernels, &M);

  std::map<Function *, Function *> EEFuncToKernel;

  for (Function *Kernel : Kernels) {
    std::string EEFuncName =
        CompilationUtils::WG_BOUND_PREFIX + Kernel->getName().str();
    auto *EEFunc = M.getFunction(EEFuncName);
    if (EEFunc) {
      EEFuncToKernel[EEFunc] = Kernel;
      LLVM_DEBUG(dbgs() << "Found boundary function: " << EEFuncName << "\n");
    }
  }

  typedef Value *(ResolveSubGroupWICall::*ResolverFunc)(Instruction *, Value *,
                                                        int32_t);
  std::map<std::string, ResolverFunc> SubGroupFuncToResolver = {
      {CompilationUtils::mangledNumSubGroups(),
       &ResolveSubGroupWICall::replaceGetNumSubGroups},
      {CompilationUtils::mangledGetSubGroupId(),
       &ResolveSubGroupWICall::replaceGetSubGroupId},
      {CompilationUtils::mangledEnqueuedNumSubGroups(),
       &ResolveSubGroupWICall::replaceGetEnqueuedNumSubGroups},
      {CompilationUtils::mangledGetSubGroupSize(),
       &ResolveSubGroupWICall::replaceGetSubGroupSize},
      {CompilationUtils::mangledGetMaxSubGroupSize(),
       &ResolveSubGroupWICall::replaceGetMaxSubGroupSize}};

  LLVM_DEBUG(dbgs() << "ResolveSGBarrier = " << ResolveSGBarrier << '\n');
  if (ResolveSGBarrier) {
    SubGroupFuncToResolver.insert(
        {CompilationUtils::mangledSGBarrier(
             CompilationUtils::BARRIER_WITH_SCOPE),
         &ResolveSubGroupWICall::replaceSubGroupBarrier});
    SubGroupFuncToResolver.insert(
        {CompilationUtils::mangledSGBarrier(CompilationUtils::BARRIER_NO_SCOPE),
         &ResolveSubGroupWICall::replaceSubGroupBarrier});
  }

  // Collect all called subgroup functions.
  std::set<Function *> SubGroupFunctions;
  for (const auto &SGItem : SubGroupFuncToResolver) {
    if (Function *F = M.getFunction(SGItem.first))
      SubGroupFunctions.insert(F);
  }

  std::set<Function *> FuncsToBePatched;
  std::set<CallInst *> CallsToBeFixed;
  // Hold the VF for every kernel.
  std::map<Function *, std::pair<Value *, int32_t>> KernelToVecInfo;
  // Hold the map from original function to patched function.
  std::map<Function *, Function *> OrigF2PatchedF;

  // Collect all functions which directly/indiectly call subgroups functions.
  LoopUtils::fillFuncUsersSet(SubGroupFunctions, FuncsToBePatched);

  for (Function *OrigFunc : FuncsToBePatched) {

    if (EEFuncToKernel.count(OrigFunc))
      OrigFunc = EEFuncToKernel[OrigFunc];
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
      KernelToVecInfo.insert(
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
    Function *PatchedFunc = CompilationUtils::AddMoreArgsToFunc(
        OrigFunc, m_ret, "vf", None, "ResolveSubGroupWICall");
    OrigF2PatchedF[OrigFunc] = PatchedFunc;
  }

  // Fix the calls for the patched functions.
  for (CallInst *CI : CallsToBeFixed) {
    Function *Caller = CI->getCaller();
    Function *Callee = CI->getCalledFunction();
    assert(OrigF2PatchedF.find(Callee) != OrigF2PatchedF.end() &&
           "Patching unexpected call");

    Value *ArgVF = nullptr;
    if (Kernels.count(Caller)) {
      ArgVF = KernelToVecInfo[Caller].first;
    } else if (EEFuncToKernel.count(Caller)) {
      ArgVF = KernelToVecInfo[EEFuncToKernel[Caller]].first;
    } else {
      ArgVF = &*(Caller->arg_end() - 1);
    }
    LLVM_DEBUG(dbgs() << "Patching call: " << *CI << "\n");
    SmallVector<Value *, 1> NewArgs{ArgVF};
    CompilationUtils::AddMoreArgsToCall(CI, NewArgs, OrigF2PatchedF[Callee]);
  }

  std::vector<std::pair<Instruction *, Value *>> InstRepVec;
  // Replace the subgroup calls.
  for (Function *SubGroupFunc : SubGroupFunctions) {
    ResolverFunc resolver =
        SubGroupFuncToResolver[std::string(SubGroupFunc->getName())];
    for (User *U : SubGroupFunc->users()) {
      assert(isa<CallInst>(U) && "Subgroup builtin isn't used in a call");
      CallInst *CI = cast<CallInst>(U);
      Function *Caller = CI->getCaller();
      LLVM_DEBUG(dbgs() << "Replacing sub-group call: " << *CI);
      LLVM_DEBUG(dbgs() << " in function: " << Caller->getName() << "\n");

      Value *VFVal = nullptr;
      if (Kernels.count(Caller)) {
        VFVal = KernelToVecInfo[Caller].first;
      } else if (EEFuncToKernel.count(Caller)) {
        VFVal = KernelToVecInfo[EEFuncToKernel[Caller]].first;
      } else {
        VFVal = &*(Caller->arg_end() - 1);
      }
      // TODO: Get VD from KernelToVecInfo.
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

  if (InstRepVec.empty())
    return false;

  return true;
}

Value *ResolveSubGroupWICall::replaceGetSubGroupSize(Instruction *InsertBefore,
                                                     Value *VFVal, int32_t VD) {
  // get_sub_group_size will be replaced with:
  // non_uniform_size = get_local_size(VD) % VF
  // max_uniform_id = get_local_size(VD) - non_uniform_size
  // sub_group_size = get_local_id(VD) < max_uniform_id ? VF :
  //                                       non_uniform_size
  // Though get_local_id will be resolved as the id of first WI in the
  // sub group, it does not matter. Because we just use the id to
  // compare with max_uniform_id, any id within a sub group is ok.

  Type *Int32Ty = Type::getInt32Ty(m_pModule->getContext());
  std::string LocalSizeName = CompilationUtils::mangledGetLocalSize();
  std::string LocalIdName = CompilationUtils::mangledGetLID();

  auto *VecDimVar = ConstantInt::get(Int32Ty, VD);

  auto *LocalSize = createWIFunctionCall(m_pModule, "", LocalSizeName,
                                         InsertBefore, VecDimVar);

  auto *MinusVFVal =
      BinaryOperator::Create(Instruction::Sub, ConstantInt::get(m_ret, 0),
                             VFVal, "minus.vf", InsertBefore);
  auto *MaxUniformId = BinaryOperator::Create(
      Instruction::And, MinusVFVal, LocalSize, "uniform.id.max", InsertBefore);
  auto *NonUniformSize =
      BinaryOperator::Create(Instruction::Sub, LocalSize, MaxUniformId,
                             "nonuniform.size", InsertBefore);
  auto *LocalId =
      createWIFunctionCall(m_pModule, "", LocalIdName, InsertBefore, VecDimVar);
  auto *Cond =
      new ICmpInst(InsertBefore, CmpInst::ICMP_ULT, LocalId, MaxUniformId, "");
  auto *SGSize =
      SelectInst::Create(Cond, VFVal, NonUniformSize, "", InsertBefore);
  auto *Ret = CastInst::CreateTruncOrBitCast(SGSize, Int32Ty, "subgroup.size",
                                             InsertBefore);
  return Ret;
}

Value *
ResolveSubGroupWICall::replaceGetMaxSubGroupSize(Instruction *InsertBefore,
                                                 Value *VFVal, int32_t /*VD*/) {
  return CastInst::CreateTruncOrBitCast(
      VFVal, IntegerType::get(m_pModule->getContext(), 32), "max.sg.size",
      InsertBefore);
}

Value *ResolveSubGroupWICall::replaceGetEnqueuedNumSubGroups(
    Instruction *InsertBefore, Value *VFVal, int32_t VD) {
  // Replace get_enqueued_num_sub_groups with the following sequence:
  // Let x be the dimension over which the vectorization has happened. x is in
  // {0,1,2}. Let y and z be the over simensions. y,z are in {0,1,2}\{x}. Let VF
  // be the vector factor. Then the number of enqueued sub groups is:
  // ((get_enqueued_local_size(x) - 1) / VF ) + 1)
  //   * get_enqueued_local_size(y) * get_enqueued_local_size(z).
  std::string EnqdLocalSizeName =
      CompilationUtils::mangledGetEnqueuedLocalSize();

  auto *EnqdLsz0 = createWIFunctionCall(m_pModule, "enqdlz0", EnqdLocalSizeName,
                                        InsertBefore, m_zero);
  auto *EnqdLsz1 = createWIFunctionCall(m_pModule, "enqdlz1", EnqdLocalSizeName,
                                        InsertBefore, m_one);
  auto *EnqdLsz2 = createWIFunctionCall(m_pModule, "enqdlz2", EnqdLocalSizeName,
                                        InsertBefore, m_two);

  Value *ValOne = ConstantInt::get(m_ret, 1);
  std::vector<Value *> EnqdLszs = {EnqdLsz0, EnqdLsz1, EnqdLsz2};
  auto *Op0 = BinaryOperator::Create(Instruction::Sub, EnqdLszs[VD], ValOne, "",
                                     InsertBefore);
  auto *Op1 = BinaryOperator::CreateUDiv(Op0, VFVal, "", InsertBefore);
  auto *Op2 = BinaryOperator::Create(Instruction::Add, Op1, ValOne,
                                     "sg.num.vecdim.enqd", InsertBefore);
  EnqdLszs[VD] = Op2;

  auto *SGNumOp0 = BinaryOperator::Create(Instruction::Mul, EnqdLszs[0],
                                          EnqdLszs[1], "", InsertBefore);
  auto *SGNumOp1 = BinaryOperator::Create(Instruction::Mul, SGNumOp0,
                                          EnqdLszs[2], "", InsertBefore);
  auto *Res = CastInst::CreateTruncOrBitCast(
      SGNumOp1, Type::getInt32Ty(m_pModule->getContext()), "sg.num.enqd",
      InsertBefore);
  return Res;
}

Value *ResolveSubGroupWICall::replaceGetNumSubGroups(Instruction *InsertBefore,
                                                     Value *VFVal, int32_t VD) {
  // Replace get_num_sub_groups() with the following sequence:
  // Improve the sequence for 2D/3D case:
  // Let x be the dimension over which the vectorization has happened. x is in
  // {0,1,2}. Let y and z be the over simensions. y,z are in {0,1,2}\{x}. Let VF
  // be the vector factor. Then the number of sub groups is:
  // ((get_local_size(x) - 1) / VF ) + 1) * get_local_size(y) *
  // get_local_size(z).
  std::string LocalSizeName = CompilationUtils::mangledGetLocalSize();

  auto *Lsz0 = createWIFunctionCall(m_pModule, "lsz0", LocalSizeName,
                                    InsertBefore, m_zero);
  auto *Lsz1 = createWIFunctionCall(m_pModule, "lsz1", LocalSizeName,
                                    InsertBefore, m_one);
  auto *Lsz2 = createWIFunctionCall(m_pModule, "lsz2", LocalSizeName,
                                    InsertBefore, m_two);

  Value *ValOne = ConstantInt::get(m_ret, 1);
  std::vector<Value *> Lszs = {Lsz0, Lsz1, Lsz2};
  auto *Op0 = BinaryOperator::Create(Instruction::Sub, Lszs[VD], ValOne, "",
                                     InsertBefore);
  auto *Op1 = BinaryOperator::CreateUDiv(Op0, VFVal, "", InsertBefore);
  auto *Op2 = BinaryOperator::Create(Instruction::Add, Op1, ValOne,
                                     "sg.num.vecdim", InsertBefore);
  Lszs[VD] = Op2;

  auto *SGNumOp0 = BinaryOperator::Create(Instruction::Mul, Lszs[0], Lszs[1],
                                          "", InsertBefore);
  auto *SGNumOp1 = BinaryOperator::Create(Instruction::Mul, SGNumOp0, Lszs[2],
                                          "", InsertBefore);
  auto *Res = CastInst::CreateTruncOrBitCast(
      SGNumOp1, Type::getInt32Ty(m_pModule->getContext()), "sg.num",
      InsertBefore);
  return Res;
}

Value *ResolveSubGroupWICall::replaceGetSubGroupId(Instruction *InsertBefore,
                                                   Value *VFVal, int32_t VD) {
  // Assume vectorization dimension is 0
  // Replace get_sub_group_id() with the following sequence:
  // (get_local_id(2) * get_local_size(1) + get_local_id(1)) *
  // ((get_local_size(0) - 1) / VF + 1) + get_local_id(0) / VF
  std::vector<int32_t> Dims;
  Dims.push_back(VD);
  for (int32_t Dim = 0; Dim < 3; ++Dim) {
    if (Dim != VD) {
      Dims.push_back(Dim);
    }
  }

  std::string IdName = CompilationUtils::mangledGetLID();
  std::string SizeName = CompilationUtils::mangledGetLocalSize();

  // call get_local_id(2), get_local_id(1), get_local_id(0)
  CallInst *Lid2 =
      createWIFunctionCall(m_pModule, "lid2", IdName, InsertBefore, m_two);
  CallInst *Lid1 =
      createWIFunctionCall(m_pModule, "lid1", IdName, InsertBefore, m_one);
  CallInst *Lid0 =
      createWIFunctionCall(m_pModule, "lid0", IdName, InsertBefore, m_zero);
  assert(Lid0 && Lid1 && Lid2 && "Can't create get_local_id calls");

  // call get_local_size(2), get_local_size(1), get_local_size(0)
  CallInst *Lsz2 =
      createWIFunctionCall(m_pModule, "lsz2", SizeName, InsertBefore, m_two);
  CallInst *Lsz1 =
      createWIFunctionCall(m_pModule, "lsz1", SizeName, InsertBefore, m_one);
  CallInst *Lsz0 =
      createWIFunctionCall(m_pModule, "lsz0", SizeName, InsertBefore, m_zero);
  assert(Lsz0 && Lsz1 && Lsz2 && "Can't create get_local_size calls");

  std::vector<Value *> Lids = {Lid0, Lid1, Lid2};
  Lids = {Lids[Dims[0]], Lids[Dims[1]], Lids[Dims[2]]};

  std::vector<Value *> Lszs = {Lsz0, Lsz1, Lsz2};
  Lszs = {Lszs[Dims[0]], Lszs[Dims[1]], Lszs[Dims[2]]};

  auto *Op0 = BinaryOperator::Create(Instruction::Mul, Lids[2], Lszs[1],
                                     "sg.id.op0", InsertBefore);
  auto *Op1 = BinaryOperator::Create(Instruction::Add, Op0, Lids[1],
                                     "sg.id.op1", InsertBefore);

  Value *ValOne = ConstantInt::get(m_ret, 1);

  auto *Op2 = BinaryOperator::Create(Instruction::Sub, Lszs[0], ValOne,
                                     "sg.id.op2", InsertBefore);
  auto *Op3 = BinaryOperator::CreateUDiv(Op2, VFVal, "sg.id.op3", InsertBefore);
  auto *Op4 = BinaryOperator::Create(Instruction::Add, Op3, ValOne, "sg.id.op4",
                                     InsertBefore);

  auto *Op5 = BinaryOperator::Create(Instruction::Mul, Op4, Op1, "sg.id.op5",
                                     InsertBefore);

  auto *Op6 =
      BinaryOperator::CreateUDiv(Lids[0], VFVal, "sg.id.op6", InsertBefore);

  auto *Op7 = BinaryOperator::Create(Instruction::Add, Op5, Op6, "sg.id.res",
                                     InsertBefore);
  auto *Res = CastInst::CreateTruncOrBitCast(
      Op7, Type::getInt32Ty(m_pModule->getContext()), "sg.id.res.trunc",
      InsertBefore);
  return Res;
}

Value *ResolveSubGroupWICall::replaceSubGroupBarrier(Instruction *InsertBefore,
                                                     Value *, int32_t) {
  CallInst *CI = cast<CallInst>(InsertBefore);
  std::string AtomicWIFenceName =
      CompilationUtils::mangledAtomicWorkItemFence();
  auto *AtomicWIFenceBIF = m_rtServices->findInRuntimeModule(AtomicWIFenceName);
  assert(AtomicWIFenceBIF && "atomic_work_item_fence not found in BI library!");

  auto *AtomicWIFenceF =
      CompilationUtils::importFunctionDecl(m_pModule, AtomicWIFenceBIF);
  assert(AtomicWIFenceF && "Failed generating function in current module");

  // take mem_fence from the barrier
  assert((CI->getNumArgOperands() >= 1) &&
         "Expect sub_group_barrier to have at least mem fence argument!");
  Value *MemFence = CI->getArgOperand(0);
  // Obtain MemoryOrder.
  // must be aligned with clang preprocessor foir
  // __ATOMIC_ACQ_REL
  const uint64_t MemoryOrderAcqRel = 4;
  Value *MemOrder = ConstantInt::get(Type::getInt32Ty(m_pModule->getContext()),
                                     MemoryOrderAcqRel);
  // obtain mem_scope.
  Value *MemScope = nullptr;
  if (CI->getNumArgOperands() == 2)
    // take memory scope from the barrier.
    MemScope = CI->getArgOperand(1);
  else {
    // must be aligned with clang preprocessor for
    // __OPENCL_MEMORY_SCOPE_SUB_GROUP.
    const uint64_t memory_scope_sub_group = 4;
    MemScope = ConstantInt::get(Type::getInt32Ty(m_pModule->getContext()),
                                memory_scope_sub_group);
  }

  SmallVector<Value *, 3> Args;
  Args.push_back(MemFence);
  Args.push_back(MemOrder);
  Args.push_back(MemScope);

  auto *AtomicWIFenceCall =
      CallInst::Create(AtomicWIFenceF, Args, "", InsertBefore);

  AtomicWIFenceCall->setDebugLoc(InsertBefore->getDebugLoc());

  return AtomicWIFenceCall;
}

ConstantInt *ResolveSubGroupWICall::createVFConstant(LLVMContext &C,
                                                     const DataLayout &DL,
                                                     size_t VF) {
  assert(((DL.getPointerSizeInBits() == 64) ||
          (DL.getPointerSizeInBits() == 32)) &&
         "Unexpected ptr size!");
  return ConstantInt::get(Type::getIntNTy(C, DL.getPointerSizeInBits()), VF);
}

CallInst *ResolveSubGroupWICall::createWIFunctionCall(
    Module *M, char const *ValueName, std::string const &FuncName,
    Instruction *InsertBefore, Value *ActPar) {

  Function *Func = M->getFunction(FuncName);

  // if the function does not exist then we need to create it
  if (!Func) {
    std::vector<Type *> Args(1, ActPar->getType());

    FunctionType *FType = FunctionType::get(m_ret, Args, false);
    Func = dyn_cast<Function>(
        m_pModule->getOrInsertFunction(FuncName, FType).getCallee());
    assert(Func && "Failed creating function");
  }

  return CallInst::Create(Func, ActPar, ValueName, InsertBefore);
}

} // namespace intel
