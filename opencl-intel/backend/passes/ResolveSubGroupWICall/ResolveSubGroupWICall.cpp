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
#include "MetadataAPI.h"
#include "OCLPassSupport.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"

#include <tuple>
#include <utility>

using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::MetadataAPI;

extern "C" {
void *createResolveSubGroupWICallPass() {
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

ResolveSubGroupWICall::ResolveSubGroupWICall()
    : ModulePass(ID), m_zero(nullptr), m_one(nullptr), m_two(nullptr),
      m_ret(nullptr), m_rtServices(nullptr) {}

bool ResolveSubGroupWICall::runOnModule(Module &M) {
  m_rtServices = getAnalysis<BuiltinLibInfo>().getRuntimeServices();
  assert(m_rtServices && "m_rtServices should exist!");

  m_zero = ConstantInt::get(Type::getInt32Ty(M.getContext()), 0);
  m_one = ConstantInt::get(Type::getInt32Ty(M.getContext()), 1);
  m_two = ConstantInt::get(Type::getInt32Ty(M.getContext()), 2);

  m_ret = IntegerType::get(M.getContext(),
                           M.getDataLayout().getPointerSizeInBits(0));

  // Holds functions and respective vector factors.
  std::vector<std::tuple<Function *, size_t, int32_t>> WorkList;

  // Get all kernels
  CompilationUtils::FunctionSet Kernels;
  CompilationUtils::getAllKernels(Kernels, &M);

  for (auto *F : Kernels) {
    auto kimd = KernelInternalMetadataAPI(F);
    auto VF = kimd.VectorizedWidth.hasValue() ? kimd.VectorizedWidth.get() : 1;
    auto VecDim = kimd.VectorizationDimension.hasValue()
                      ? kimd.VectorizationDimension.get()
                      : 0;
    WorkList.push_back(std::make_tuple(F, VF, VecDim));

    // Get the VF of vectorized counterpart.
    auto VectorizedKernelMetadata = kimd.VectorizedKernel;
    auto *VecKernel = VectorizedKernelMetadata.hasValue()
                          ? VectorizedKernelMetadata.get()
                          : nullptr;
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

    // TODO: add processing of vectorized function calls both direct and
    // indirect. (like SYCL sub_group.invoke etc.). Scalarized function call
    // does not hold subgroups semantics.
  }

  bool Changed = false;
  for (auto WorkTuple : WorkList) {
    Changed |= runOnFunction(*std::get<0>(WorkTuple), std::get<1>(WorkTuple),
                             std::get<2>(WorkTuple));
  }

  return Changed;
}

bool ResolveSubGroupWICall::runOnFunction(Function &F, size_t VF, int32_t VD) {
  std::vector<std::pair<Instruction *, Value *>> InstRepVec;

  Module *M = F.getParent();

  for (auto &I : instructions(F)) {
    CallInst *CI = dyn_cast<CallInst>(&I);
    if (!CI)
      continue;

    auto *CalledF = CI->getCalledFunction();
    if (!CalledF)
      continue;

    std::string funcName = CalledF->getName().str();

    if (CompilationUtils::isGetNumSubGroups(funcName)) {
      InstRepVec.push_back(std::pair<Instruction *, Value *>(
          CI, replaceGetNumSubGroups(M, CI, VF, VD)));
    } else if (CompilationUtils::isGetSubGroupId(funcName)) {
      InstRepVec.push_back(std::pair<Instruction *, Value *>(
          CI, replaceGetSubGroupId(M, CI, VF, VD)));
    } else if (CompilationUtils::isGetEnqueuedNumSubGroups(funcName)) {
      InstRepVec.push_back(std::pair<Instruction *, Value *>(
          CI, replaceGetEnqueuedNumSubGroups(M, CI, VF, VD)));
    } else if (CompilationUtils::isGetSubGroupSize(funcName)) {
      InstRepVec.push_back(std::pair<Instruction *, Value *>(
          CI, replaceGetSubGroupSize(M, CI, VF, VD)));
    } else if (CompilationUtils::isGetMaxSubGroupSize(funcName)) {
      InstRepVec.push_back(std::pair<Instruction *, Value *>(
          CI, replaceGetMaxSubGroupSize(M, CI, VF)));
    } else if (CompilationUtils::isGetSubGroupLocalId(funcName)) {
      InstRepVec.push_back(std::pair<Instruction *, Value *>(
          CI, replaceGetSubGroupLocalId(M, CI, VF)));
    } else if ((funcName == CompilationUtils::mangledSGBarrier(
                                CompilationUtils::BARRIER_WITH_SCOPE)) ||
               (funcName == CompilationUtils::mangledSGBarrier(
                                CompilationUtils::BARRIER_NO_SCOPE))) {
      InstRepVec.push_back(
          std::pair<Instruction *, Value *>(CI, replaceSubGroupBarrier(M, CI)));
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

Value *ResolveSubGroupWICall::replaceGetSubGroupSize(Module *M,
                                                     Instruction *insertBefore,
                                                     size_t VF, int32_t VD) {
  // get_sub_group_size will be replaced with:
  // non_uniform_size = get_local_size(VD) % VF
  // max_uniform_id = get_local_size(VD) - non_uniform_size
  // sub_group_size = get_local_id(VD) < max_uniform_id ? VF :
  //                                       non_uniform_size
  // Though get_local_id will be resolved as the id of first WI in the
  // sub group, it does not matter. Because we just use the id to
  // compare with max_uniform_id, any id within a sub group is ok.

  Type *Int32Ty = Type::getInt32Ty(M->getContext());
  std::string LocalSizeName = CompilationUtils::mangledGetLocalSize();
  std::string LocalIdName = CompilationUtils::mangledGetLID();

  auto *vecDimVar = ConstantInt::get(Int32Ty, VD);

  auto *localSize =
      createWIFunctionCall(M, "", LocalSizeName, insertBefore, vecDimVar);
  auto *maxUniformId =
      BinaryOperator::Create(Instruction::And, ConstantInt::get(m_ret, -VF),
                             localSize, "uniform.id.max", insertBefore);
  auto *nonUniformSize =
      BinaryOperator::Create(Instruction::Sub, localSize, maxUniformId,
                             "nonuniform.size", insertBefore);
  auto *localId =
      createWIFunctionCall(M, "", LocalIdName, insertBefore, vecDimVar);
  auto *cond =
      new ICmpInst(insertBefore, CmpInst::ICMP_ULT, localId, maxUniformId, "");
  auto *VFVal = createVFConstant(M->getContext(), M->getDataLayout(), VF);
  auto *sgSize =
      SelectInst::Create(cond, VFVal, nonUniformSize, "", insertBefore);
  auto *ret = CastInst::CreateTruncOrBitCast(sgSize, Int32Ty, "subgroup.size",
                                             insertBefore);
  return ret;
}

Value *ResolveSubGroupWICall::replaceGetMaxSubGroupSize(
    Module *M, Instruction *insertBefore, size_t VF) {
  return ConstantInt::get(IntegerType::get(M->getContext(), 32), VF);
}

Value *ResolveSubGroupWICall::replaceGetSubGroupLocalId(
    Module *M, Instruction *insertBefore, size_t VF) {
  // Gets expanded to <0, 1, ..., VF - 1> by Vectorizer.
  return ConstantInt::get(IntegerType::get(M->getContext(), 32), 0);
}

Instruction *ResolveSubGroupWICall::replaceGetEnqueuedNumSubGroups(
    Module *M, Instruction *insertBefore, size_t VF, int32_t VD) {
  // Replace get_enqueued_num_sub_groups with the following sequence:
  // Let x be the dimension over which the vectorization has happened. x is in
  // {0,1,2}. Let y and z be the over simensions. y,z are in {0,1,2}\{x}. Let VF
  // be the vector factor. Then the number of enqueued sub groups is:
  // ((get_enqueued_local_size(x) - 1) / VF ) + 1)
  //   * get_enqueued_local_size(y) * get_enqueued_local_size(z).
  std::string EnqdLocalSizeName =
      CompilationUtils::mangledGetEnqueuedLocalSize();

  auto *enqdlsz0 = createWIFunctionCall(M, "enqdlz0", EnqdLocalSizeName,
                                        insertBefore, m_zero);
  auto *enqdlsz1 = createWIFunctionCall(M, "enqdlz1", EnqdLocalSizeName,
                                        insertBefore, m_one);
  auto *enqdlsz2 = createWIFunctionCall(M, "enqdlz2", EnqdLocalSizeName,
                                        insertBefore, m_two);

  Value *valOne = ConstantInt::get(m_ret, 1);
  std::vector<Value *> enqdlszs = {enqdlsz0, enqdlsz1, enqdlsz2};
  auto *op0 = BinaryOperator::Create(Instruction::Sub, enqdlszs[VD], valOne, "",
                                     insertBefore);
  auto *VFVal = createVFConstant(M->getContext(), M->getDataLayout(), VF);
  auto *op1 = BinaryOperator::CreateUDiv(op0, VFVal, "", insertBefore);
  auto *op2 = BinaryOperator::Create(Instruction::Add, op1, valOne,
                                     "sg.num.vecdim.enqd", insertBefore);
  enqdlszs[VD] = op2;

  auto *sgNumOp0 = BinaryOperator::Create(Instruction::Mul, enqdlszs[0],
                                          enqdlszs[1], "", insertBefore);
  auto *sgNumOp1 = BinaryOperator::Create(Instruction::Mul, sgNumOp0,
                                          enqdlszs[2], "", insertBefore);
  auto *res = CastInst::CreateTruncOrBitCast(
      sgNumOp1, Type::getInt32Ty(M->getContext()), "sg.num.enqd", insertBefore);
  return res;
}

Instruction *ResolveSubGroupWICall::replaceGetNumSubGroups(
    Module *M, Instruction *insertBefore, size_t VF, int32_t VD) {
  // Replace get_num_sub_groups() with the following sequence:
  // Improve the sequence for 2D/3D case:
  // Let x be the dimension over which the vectorization has happened. x is in
  // {0,1,2}. Let y and z be the over simensions. y,z are in {0,1,2}\{x}. Let VF
  // be the vector factor. Then the number of sub groups is:
  // ((get_local_size(x) - 1) / VF ) + 1) * get_local_size(y) *
  // get_local_size(z).
  std::string LocalSizeName = CompilationUtils::mangledGetLocalSize();

  auto *lsz0 =
      createWIFunctionCall(M, "lsz0", LocalSizeName, insertBefore, m_zero);
  auto *lsz1 =
      createWIFunctionCall(M, "lsz1", LocalSizeName, insertBefore, m_one);
  auto *lsz2 =
      createWIFunctionCall(M, "lsz2", LocalSizeName, insertBefore, m_two);

  Value *valOne = ConstantInt::get(m_ret, 1);
  std::vector<Value *> lszs = {lsz0, lsz1, lsz2};
  auto *op0 = BinaryOperator::Create(Instruction::Sub, lszs[VD], valOne, "",
                                     insertBefore);
  auto *VFVal = createVFConstant(M->getContext(), M->getDataLayout(), VF);
  auto *op1 = BinaryOperator::CreateUDiv(op0, VFVal, "", insertBefore);
  auto *op2 = BinaryOperator::Create(Instruction::Add, op1, valOne,
                                     "sg.num.vecdim", insertBefore);
  lszs[VD] = op2;

  auto *sgNumOp0 = BinaryOperator::Create(Instruction::Mul, lszs[0], lszs[1],
                                          "", insertBefore);
  auto *sgNumOp1 = BinaryOperator::Create(Instruction::Mul, sgNumOp0, lszs[2],
                                          "", insertBefore);
  auto *res = CastInst::CreateTruncOrBitCast(
      sgNumOp1, Type::getInt32Ty(M->getContext()), "sg.num", insertBefore);
  return res;
}

Instruction *ResolveSubGroupWICall::replaceGetSubGroupId(
    Module *M, Instruction *insertBefore, size_t VF, int32_t VD) {
  // Assume vectorization dimension is 0
  // Replace get_sub_group_id() with the following sequence:
  // (get_local_id(2) * get_local_size(1) + get_local_id(1)) *
  // ((get_local_size(0) - 1) / VF + 1) + get_local_id(0) / VF
  std::vector<int32_t> Dims;
  Dims.push_back(VD);
  for (int32_t dim = 0; dim < 3; ++dim) {
    if (dim != VD) {
      Dims.push_back(dim);
    }
  }

  std::string idName = CompilationUtils::mangledGetLID();
  std::string sizeName = CompilationUtils::mangledGetLocalSize();

  // call get_local_id(2), get_local_id(1), get_local_id(0)
  CallInst *lid2 = createWIFunctionCall(M, "lid2", idName, insertBefore, m_two);
  CallInst *lid1 = createWIFunctionCall(M, "lid1", idName, insertBefore, m_one);
  CallInst *lid0 =
      createWIFunctionCall(M, "lid0", idName, insertBefore, m_zero);
  assert(lid0 && lid1 && lid2 && "Can't create get_local_id calls");

  // call get_local_size(2), get_local_size(1), get_local_size(0)
  CallInst *lsz2 =
      createWIFunctionCall(M, "lsz2", sizeName, insertBefore, m_two);
  CallInst *lsz1 =
      createWIFunctionCall(M, "lsz1", sizeName, insertBefore, m_one);
  CallInst *lsz0 =
      createWIFunctionCall(M, "lsz0", sizeName, insertBefore, m_zero);
  assert(lsz0 && lsz1 && lsz2 && "Can't create get_local_size calls");

  std::vector<Value *> lids = {lid0, lid1, lid2};
  lids = {lids[Dims[0]], lids[Dims[1]], lids[Dims[2]]};

  std::vector<Value *> lszs = {lsz0, lsz1, lsz2};
  lszs = {lszs[Dims[0]], lszs[Dims[1]], lszs[Dims[2]]};

  auto *op0 = BinaryOperator::Create(Instruction::Mul, lids[2], lszs[1],
                                     "sg.id.op0", insertBefore);
  auto *op1 = BinaryOperator::Create(Instruction::Add, op0, lids[1], "sg.id.op1",
                                     insertBefore);
  auto *VFVal = createVFConstant(M->getContext(), M->getDataLayout(), VF);

  Value *valOne = ConstantInt::get(m_ret, 1);

  auto *op2 = BinaryOperator::Create(Instruction::Sub, lszs[0], valOne, "sg.id.op2",
                                     insertBefore);
  auto *op3 = BinaryOperator::CreateUDiv(op2, VFVal, "sg.id.op3", insertBefore);
  auto *op4 =
      BinaryOperator::Create(Instruction::Add, op3, valOne, "sg.id.op4", insertBefore);

  auto *op5 =
      BinaryOperator::Create(Instruction::Mul, op4, op1, "sg.id.op5", insertBefore);

  auto *op6 = BinaryOperator::CreateUDiv(lids[0], VFVal, "sg.id.op6", insertBefore);

  auto *op7 =
      BinaryOperator::Create(Instruction::Add, op5, op6, "sg.id.res", insertBefore);
  auto *res = CastInst::CreateTruncOrBitCast(
      op7, Type::getInt32Ty(M->getContext()), "sg.id.res.trunc", insertBefore);
  return res;
}

Instruction *
ResolveSubGroupWICall::replaceSubGroupBarrier(Module *M,
                                              CallInst *insertBefore) {
  std::string AtomicWIFenceName =
      CompilationUtils::mangledAtomicWorkItemFence();
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
  Value *MemOrder =
      ConstantInt::get(Type::getInt32Ty(M->getContext()), memory_order_acq_rel);
  // obtain mem_scope.
  Value *MemScope = nullptr;
  if (insertBefore->getNumArgOperands() == 2)
    // take memory scope from the barrier.
    MemScope = insertBefore->getArgOperand(1);
  else {
    // must be aligned with clang preprocessor for
    // __OPENCL_MEMORY_SCOPE_SUB_GROUP.
    const uint64_t memory_scope_sub_group = 4;
    MemScope = ConstantInt::get(Type::getInt32Ty(M->getContext()),
                                memory_scope_sub_group);
  }

  SmallVector<Value *, 3> args;
  args.push_back(MemFence);
  args.push_back(MemOrder);
  args.push_back(MemScope);

  auto *AtomicWIFenceCall =
      CallInst::Create(AtomicWIFenceF, args, "", insertBefore);

  AtomicWIFenceCall->setDebugLoc(insertBefore->getDebugLoc());

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

CallInst *ResolveSubGroupWICall::createWIFunctionCall(Module *M,
                                                      char const *twine,
                                                      std::string const &name,
                                                      Instruction *insertBefore,
                                                      Value *actPar) {

  Function *func = M->getFunction(name);

  // if the function does not exist then we need to create it
  if (!func) {
    std::vector<Type *> args(1, actPar->getType());

    FunctionType *fType = FunctionType::get(m_ret, args, false);
    func = dyn_cast<Function>(M->getOrInsertFunction(name, fType).getCallee());
    assert(func && "Failed creating function");
  }

  return CallInst::Create(func, actPar, twine, insertBefore);
}

} // namespace intel
