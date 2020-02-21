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
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "CompilationUtils.h"
#include "MetadataAPI.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Debug.h"

#include <tuple>
#include <utility>

using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::MetadataAPI;

extern "C" {
  void* createResolveSubGroupWICallPass() {
    return new intel::ResolveSubGroupWICall();
  }
}

namespace intel {

  char ResolveSubGroupWICall::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(ResolveSubGroupWICall, "resolve-sub-group-wi-call",
                      "Resolve Sub Group WI functions", false, false)
  OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
  OCL_INITIALIZE_PASS_END(ResolveSubGroupWICall, "resolve-sub-group-wi-call",
                      "Resolve Sub Group WI functions", false, false)

  ResolveSubGroupWICall::ResolveSubGroupWICall() :
    ModulePass(ID), m_zero(nullptr), m_one(nullptr), m_two(nullptr),
    m_ret(nullptr), m_rtServices(nullptr)
  { }

  bool ResolveSubGroupWICall::runOnModule(Module &M) {
    m_rtServices = getAnalysis<BuiltinLibInfo>().getRuntimeServices();
    assert(m_rtServices && "m_rtServices should exist!");

    m_zero = ConstantInt::get(Type::getInt32Ty(M.getContext()), 0);
    m_one  = ConstantInt::get(Type::getInt32Ty(M.getContext()), 1);
    m_two  = ConstantInt::get(Type::getInt32Ty(M.getContext()), 2);

    m_ret =
      IntegerType::get(M.getContext(), M.getDataLayout().getPointerSizeInBits(0));

    // Holds functions and respective vector factors.
    std::vector<std::tuple<Function*, size_t, int32_t>> WorkList;

    // Get all kernels
    CompilationUtils::FunctionSet Kernels;
    CompilationUtils::getAllKernels(Kernels, &M);

    for (auto *F : Kernels) {
      auto kimd = KernelInternalMetadataAPI(F);
      auto VF = kimd.VectorizedWidth.hasValue() ? kimd.VectorizedWidth.get() : 1;
      auto VecDim = kimd.VectorizationDimension.hasValue() ?
                      kimd.VectorizationDimension.get() : 0;
      WorkList.push_back(std::make_tuple(F, VF, VecDim));

      // Get the VF of vectorized counterpart.
      auto VectorizedKernelMetadata = kimd.VectorizedKernel;
      auto *VecKernel = VectorizedKernelMetadata.hasValue() ?
        VectorizedKernelMetadata.get() : nullptr;
      if (VecKernel) {
        auto vkimd = KernelInternalMetadataAPI(VecKernel);
        VF = vkimd.VectorizedWidth.hasValue() ? vkimd.VectorizedWidth.get() : 1;
      }

      // Add Loop Boundaries (if exist)
      std::string FuncName = F->getName().str();
      std::string EEFuncName = CompilationUtils::WG_BOUND_PREFIX + FuncName;
      auto *EEFunc = M.getFunction(EEFuncName);
      // EE function in general can be inlined into a scalar kernel as well as
      // into a vector one, in native subgroups scalar kernel does not make sense
      // and is not used, so we spoil EE with correct VF not equal to 1.
      if (EEFunc)
        WorkList.push_back(std::make_tuple(EEFunc, VF, VecDim));

      // TODO: add processing of vectorized function calls both direct and indirect.
      // (like SYCL sub_group.invoke etc.).
      // Scalarized function call does not hold subgroups semantics.
    }

    bool Changed = false;
    for (auto WorkTuple : WorkList) {
      Changed |= runOnFunction(*std::get<0>(WorkTuple), std::get<1>(WorkTuple), std::get<2>(WorkTuple));
    }

    return Changed;
  }

  bool ResolveSubGroupWICall::runOnFunction(Function &F, size_t VF, int32_t VD) {
    std::vector<std::pair<Instruction *, Value *> > InstRepVec;

    Module *M = F.getParent();

    for (auto &I : instructions(F)) {
      CallInst *CI = dyn_cast<CallInst>(&I);
      if (!CI) continue;

      auto *CalledF = CI->getCalledFunction();
      if (!CalledF) continue;

      std::string funcName = CalledF->getName().str();

      if (CompilationUtils::isGetNumSubGroups(funcName)) {
        InstRepVec.push_back(std::pair<Instruction*, Value*>(
          CI, replaceGetNumSubGroups(M, CI, VF, VD)));
      } else if (CompilationUtils::isGetSubGroupId(funcName)) {
        InstRepVec.push_back(std::pair<Instruction*, Value*>(
          CI, replaceGetSubGroupId(M, CI, VF)));
      } else if (CompilationUtils::isGetEnqueuedNumSubGroups(funcName)) {
        InstRepVec.push_back(std::pair<Instruction*, Value*>(
          CI, replaceGetEnqueuedNumSubGroups(M, CI, VF, VD)));
      } else if (CompilationUtils::isGetSubGroupSize(funcName)) {
        InstRepVec.push_back(std::pair<Instruction*, Value*>(
          CI, replaceGetSubGroupSize(M, CI, VF)));
      }  else if (CompilationUtils::isGetMaxSubGroupSize(funcName)) {
        InstRepVec.push_back(std::pair<Instruction*, Value*>(
          CI, replaceGetMaxSubGroupSize(M, CI, VF)));
      } else if (CompilationUtils::isGetSubGroupLocalId(funcName)) {
        InstRepVec.push_back(std::pair<Instruction*, Value*>(
          CI, replaceGetSubGroupLocalId(M, CI, VF)));
      } else if ((funcName == CompilationUtils::mangledSGBarrier(
                                CompilationUtils::BARRIER_WITH_SCOPE)) ||
                 (funcName == CompilationUtils::mangledSGBarrier(
                                CompilationUtils::BARRIER_NO_SCOPE))) {
        InstRepVec.push_back(std::pair<Instruction*, Value*>(
          CI, replaceSubGroupBarrier(M, CI)));
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

  Value* ResolveSubGroupWICall::replaceGetSubGroupSize(
      Module *M, Value *insertBefore, size_t VF) {
    // TODO: get_sub_group_size needs to be implemented smarter when
    // we'll have support for masked loop remainders for non-VF divisible WG sizes.
    return replaceGetMaxSubGroupSize(M, insertBefore, VF);
  }

  Value* ResolveSubGroupWICall::replaceGetMaxSubGroupSize(
      Module *M, Value *insertBefore, size_t VF) {
    return ConstantInt::get(IntegerType::get(M->getContext(), 32), VF);
  }

  Value* ResolveSubGroupWICall::replaceGetSubGroupLocalId(
      Module *M, Value *insertBefore, size_t VF) {
    // Gets expanded to <0, 1, ..., VF - 1> by Vectorizer.
    return ConstantInt::get(IntegerType::get(M->getContext(), 32), 0);
  }

  Instruction* ResolveSubGroupWICall::replaceGetEnqueuedNumSubGroups(
    Module *M, Instruction *insertBefore, size_t VF, int32_t VD) {
    // Replace get_enqueued_num_sub_groups with the following sequence:
    // Let x be the dimension over which the vectorization has happened. x is in {0,1,2}.
    // Let y and z be the over simensions. y,z are in {0,1,2}\{x}.
    // Let VF be the vector factor.
    // Then the number of enqueued sub groups is:
    // ((get_enqueued_local_size(x) - 1) / VF ) + 1)
    //   * get_enqueued_local_size(y) * get_enqueued_local_size(z).
    std::string EnqdLocalSizeName = CompilationUtils::mangledGetEnqueuedLocalSize();

    auto *enqdlsz0 =
      createWIFunctionCall(M, "enqdlz0", EnqdLocalSizeName, insertBefore, m_zero);
    auto *enqdlsz1 =
      createWIFunctionCall(M, "enqdlz1", EnqdLocalSizeName, insertBefore, m_one);
    auto *enqdlsz2 =
      createWIFunctionCall(M, "enqdlz2", EnqdLocalSizeName, insertBefore, m_two);

    Value *valOne = ConstantInt::get(m_ret, 1);
    std::vector<Value*> enqdlszs = {enqdlsz0, enqdlsz1, enqdlsz2};
    auto *op0 = BinaryOperator::Create(Instruction::Sub, enqdlszs[VD],
                                       valOne, "", insertBefore);
    auto *VFVal = createVFConstant(M->getContext(), M->getDataLayout(), VF);
    auto *op1 = BinaryOperator::CreateUDiv(op0, VFVal, "", insertBefore);
    auto *op2 = BinaryOperator::Create(Instruction::Add, op1, valOne,
                                       "sg.num.vecdim.enqd", insertBefore);
    enqdlszs[VD] = op2;

    auto *sgNumOp0 = BinaryOperator::Create(Instruction::Mul, enqdlszs[0],
                                            enqdlszs[1], "", insertBefore);
    auto *sgNumOp1 = BinaryOperator::Create(Instruction::Mul, sgNumOp0,
                                            enqdlszs[2], "", insertBefore);
    auto *res =
      CastInst::CreateTruncOrBitCast(sgNumOp1,
                                     Type::getInt32Ty(M->getContext()),
                                     "sg.num.enqd", insertBefore);
    return res;
  }

  Instruction* ResolveSubGroupWICall::replaceGetNumSubGroups(
      Module *M, Instruction *insertBefore, size_t VF, int32_t VD) {
    // Replace get_num_sub_groups() with the following sequence:
    // Improve the sequence for 2D/3D case:
    // Let x be the dimension over which the vectorization has happened. x is in {0,1,2}.
    // Let y and z be the over simensions. y,z are in {0,1,2}\{x}.
    // Let VF be the vector factor.
    // Then the number of sub groups is:
    // ((get_local_size(x) - 1) / VF ) + 1) * get_local_size(y) * get_local_size(z).
    std::string LocalSizeName = CompilationUtils::mangledGetLocalSize();

    auto * lsz0 = createWIFunctionCall(M, "lsz0", LocalSizeName, insertBefore, m_zero);
    auto * lsz1 = createWIFunctionCall(M, "lsz1", LocalSizeName, insertBefore, m_one);
    auto * lsz2 = createWIFunctionCall(M, "lsz2", LocalSizeName, insertBefore, m_two);

    Value *valOne = ConstantInt::get(m_ret, 1);
    std::vector<Value*> lszs = {lsz0, lsz1, lsz2};
    auto *op0 = BinaryOperator::Create(Instruction::Sub, lszs[VD],
                                       valOne, "", insertBefore);
    auto *VFVal = createVFConstant(M->getContext(), M->getDataLayout(), VF);
    auto *op1 = BinaryOperator::CreateUDiv(op0, VFVal, "", insertBefore);
    auto *op2 = BinaryOperator::Create(Instruction::Add, op1, valOne,
                                       "sg.num.vecdim", insertBefore);
    lszs[VD] = op2;

    auto *sgNumOp0 = BinaryOperator::Create(Instruction::Mul, lszs[0],
                                            lszs[1], "", insertBefore);
    auto *sgNumOp1 = BinaryOperator::Create(Instruction::Mul, sgNumOp0,
                                            lszs[2], "", insertBefore);
    auto *res =
      CastInst::CreateTruncOrBitCast(sgNumOp1,
                                     Type::getInt32Ty(M->getContext()),
                                     "sg.num", insertBefore);
    return res;
  }

  Instruction* ResolveSubGroupWICall::replaceGetSubGroupId(
      Module *M, Instruction *insertBefore, size_t VF) {
    // Replace get_sub_group_id() with the following sequence:
    // Let x be the dimension over which the vectorization has happened. x is in {0,1,2}.
    // get_local_linear_id(x) / VF,
    // where get_local_linear_id is:
    //   (get_local_id(2) * get_local_size(1)
    //     + get_local_id(1))
    //    * get_local_size(0)
    //    + get_local_id(0)
    std::string idName   = CompilationUtils::mangledGetLID();
    std::string sizeName = CompilationUtils::mangledGetLocalSize();

    // call get_local_id(2), get_local_id(1), get_local_id(0)
    CallInst * lid2 = createWIFunctionCall(M, "lid2", idName, insertBefore, m_two);
    CallInst * lid1 = createWIFunctionCall(M, "lid1", idName, insertBefore, m_one);
    CallInst * lid0 = createWIFunctionCall(M, "lid0", idName, insertBefore, m_zero);
    assert (lid0 && lid1 && lid2 && "Can't create get_local_id calls");

    // call get_local_size(1), get_local_size(0)
    CallInst * lsz1 = createWIFunctionCall(M, "lsz1", sizeName, insertBefore, m_one);
    CallInst * lsz0 = createWIFunctionCall(M, "lsz0", sizeName, insertBefore, m_zero);
    assert (lsz0 && lsz1 && "Can't create get_local_size calls");

    // (get_local_id(2) * get_local_size(1)
    auto *op0 = BinaryOperator::Create(Instruction::Mul, lid2, lsz1,
                                       "llid.op0", insertBefore);
    //  + get_local_id(1))
    auto *op1 = BinaryOperator::Create(Instruction::Add, op0, lid1,
                                       "llid.op1", insertBefore);
    // * get_local_size(0)
    auto *op2 = BinaryOperator::Create(Instruction::Mul, op1, lsz0,
                                       "llid.op2", insertBefore);
    // + get_local_id(0)
    auto *op3 = BinaryOperator::Create(Instruction::Add, op2, lid0,
                                       "llid.res", insertBefore);

    // VF constant
    auto *VFVal = createVFConstant(M->getContext(), M->getDataLayout(), VF);

    auto *op4 = BinaryOperator::CreateUDiv(op3, VFVal, "llid.res.div", insertBefore);

    auto *res = CastInst::CreateTruncOrBitCast(op4, Type::getInt32Ty(M->getContext()),
                                               "llid.res.div.trunc", insertBefore);

    return res;
  }

  Instruction* ResolveSubGroupWICall::replaceSubGroupBarrier(
      Module *M, CallInst *insertBefore) {
    std::string AtomicWIFenceName = CompilationUtils::mangledAtomicWorkItemFence();
    auto *AtomicWIFenceBIF = m_rtServices->findInRuntimeModule(AtomicWIFenceName);
    assert(AtomicWIFenceBIF && "atomic_work_item_fence not found in BI library!");

    auto *AtomicWIFenceF =
      CompilationUtils::importFunctionDecl(M, AtomicWIFenceBIF);
    assert(AtomicWIFenceF && "Failed generating function in current module");

    // take mem_fence from the barrier
    assert((insertBefore->getNumArgOperands() >= 1) &&
           "Expect sub_group_barrier to have at least mem fence argument!");
    Value *MemFence = insertBefore->getArgOperand(0);
    // Obtain MemoryOrder.
    // must be aligned with clang preprocessor foir
    // __ATOMIC_ACQ_REL
    const uint64_t memory_order_acq_rel = 4;
    Value *MemOrder = ConstantInt::get(Type::getInt32Ty(
                           M->getContext()),
                           memory_order_acq_rel);
    // obtain mem_scope.
    Value* MemScope = nullptr;
    if (insertBefore->getNumArgOperands() == 2)
      // take memory scope from the barrier.
      MemScope = insertBefore->getArgOperand(1);
    else {
      // must be aligned with clang preprocessor for
      // __OPENCL_MEMORY_SCOPE_SUB_GROUP.
      const uint64_t memory_scope_sub_group = 4;
      MemScope = ConstantInt::get(Type::getInt32Ty(
                   M->getContext()),
                   memory_scope_sub_group);
    }

    SmallVector<Value*, 3> args;
    args.push_back(MemFence);
    args.push_back(MemOrder);
    args.push_back(MemScope);

    auto *AtomicWIFenceCall = CallInst::Create(AtomicWIFenceF, args, "", insertBefore);

    AtomicWIFenceCall->setDebugLoc(insertBefore->getDebugLoc());

    return AtomicWIFenceCall;
  }

  ConstantInt* ResolveSubGroupWICall::createVFConstant(
      LLVMContext& C, const DataLayout& DL, size_t VF) {
    assert(((DL.getPointerSizeInBits() == 64) || (DL.getPointerSizeInBits() == 32))
      && "Unexpected ptr size!");
    return ConstantInt::get(Type::getIntNTy(C, DL.getPointerSizeInBits()), VF);
  }

  CallInst* ResolveSubGroupWICall::createWIFunctionCall(
            Module * M, char const* twine, std::string const& name,
            Instruction * insertBefore, Value *actPar) {

    Function * func = M->getFunction(name);

    // if the function does not exist then we need to create it
    if (!func) {
      std::vector<Type *> args(1, actPar->getType());

      FunctionType * fType = FunctionType::get(m_ret, args, false);
      func = dyn_cast<Function>(M->getOrInsertFunction(name, fType).getCallee());
      assert(func && "Failed creating function");
    }

    return CallInst::Create(func, actPar, twine, insertBefore);
  }

}
