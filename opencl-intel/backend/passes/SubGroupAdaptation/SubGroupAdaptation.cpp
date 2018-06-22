// INTEL CONFIDENTIAL
//
// Copyright 2015-2018 Intel Corporation.
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

#include "SubGroupAdaptation.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "CompilationUtils.h"
#include "NameMangleAPI.h"
#include "Mangler.h"

#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DataLayout.h"
#include <assert.h>

#include <utility>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

extern "C" {
/// @brief Creates new SubGroupAdaptationPass module pass
/// @returns new SubGroupAdaptationPass module pass
llvm::ModulePass *createSubGroupAdaptationPass() {
  return new intel::SubGroupAdaptation();
}
}

namespace intel {

char SubGroupAdaptation::ID = 0;

OCL_INITIALIZE_PASS(SubGroupAdaptation, "sub-group-adaptation",
                    "Replace sub-group built-ins with appropriate IR sequence",
                    false, false)

bool SubGroupAdaptation::runOnModule(Module &M) {

  m_pModule = &M;
  m_pLLVMContext = &M.getContext();
  unsigned pointerSizeInBits = M.getDataLayout().getPointerSizeInBits(0);
  assert((32 == pointerSizeInBits || 64 == pointerSizeInBits) &&
         "Unsupported pointer size");
  m_pSizeT = IntegerType::get(*m_pLLVMContext, pointerSizeInBits);

  for (Module::iterator mi = M.begin(), me = M.end(); mi != me;) {
    Function *pFunc = &*(mi++);

    // OCL built-ins must not be defined in the module at the moment the pass is
    // running
    if (!pFunc || !pFunc->isDeclaration())
      continue;
    llvm::StringRef func_name = pFunc->getName();
    if (CompilationUtils::isSubGroupBarrier(func_name)) {
      replaceFunction(pFunc, CompilationUtils::WG_BARRIER_FUNC_NAME);
    } else if (CompilationUtils::isGetSubGroupLocalID(func_name)) {

      BasicBlock *entry = BasicBlock::Create(*m_pLLVMContext, "entry", pFunc);
      std::string fName = func_name.str();
      reflection::FunctionDescriptor fd = demangle(fName.c_str());
      fd.name = CompilationUtils::NAME_GET_LINEAR_LID;
      fName = mangle(fd);

      FunctionType *pNewFuncType = FunctionType::get(m_pSizeT, false);
      Function *newF = cast<Function>(m_pModule->getOrInsertFunction(
          fName, pNewFuncType, pFunc->getAttributes()));
      CallInst *pNewCall = CallInst::Create(newF, "callInst", entry);
      pNewCall->setCallingConv(pFunc->getCallingConv());
      newF->setCallingConv(pFunc->getCallingConv());
      Value *linid = pNewCall;
      if (pFunc->getReturnType() != newF->getReturnType())
        linid = CastInst::CreateIntegerCast(linid, pFunc->getReturnType(),
                                            false, "cast", entry);
      ReturnInst::Create(*m_pLLVMContext, linid, entry);
    } else if (CompilationUtils::isGetSubGroupSize(func_name)) {

      BasicBlock *entry = BasicBlock::Create(*m_pLLVMContext, "entry", pFunc);

      // Replace get_sub_group_size() with the following sequence.
      // get_local_size(0) * get_local_size(1) * get_local_size(2)
      CallInst *local_size_0 =
          getWICall(entry, "lsz0", CompilationUtils::mangledGetLocalSize(), 0);
      CallInst *local_size_1 =
          getWICall(entry, "lsz1", CompilationUtils::mangledGetLocalSize(), 1);
      CallInst *local_size_2 =
          getWICall(entry, "lsz2", CompilationUtils::mangledGetLocalSize(), 2);

      Instruction *pRetVal =
          BinaryOperator::CreateMul(local_size_0, local_size_1, "op0", entry);
      pRetVal = BinaryOperator::CreateMul(pRetVal, local_size_2, "res", entry);

      if (pRetVal->getType() != pFunc->getReturnType()) {
        pRetVal = CastInst::CreateIntegerCast(pRetVal, pFunc->getReturnType(),
                                              false, "cast", entry);
      }
      ReturnInst::Create(*m_pLLVMContext, pRetVal, entry);
    } else if (CompilationUtils::isGetMaxSubGroupSize(func_name)) {

      BasicBlock *entry = BasicBlock::Create(*m_pLLVMContext, "entry", pFunc);

      // Replace get_mux_sub_group_size() with the following sequence.
      // get_enqueued_local_size(0) * get_enqueued_local_size(1) *
      // get_enqueued_local_size(2)
      CallInst *enqueued_local_size_0 = getWICall(
          entry, "elsz0", CompilationUtils::mangledGetEnqueuedLocalSize(), 0);
      CallInst *enqueued_local_size_1 = getWICall(
          entry, "elsz1", CompilationUtils::mangledGetEnqueuedLocalSize(), 1);
      CallInst *enqueued_local_size_2 = getWICall(
          entry, "elsz2", CompilationUtils::mangledGetEnqueuedLocalSize(), 2);

      Instruction *pRetVal = BinaryOperator::CreateMul(
          enqueued_local_size_0, enqueued_local_size_1, "op0", entry);
      pRetVal = BinaryOperator::CreateMul(pRetVal, enqueued_local_size_2, "res",
                                          entry);

      if (pRetVal->getType() != pFunc->getReturnType()) {
        pRetVal = CastInst::CreateIntegerCast(pRetVal, pFunc->getReturnType(),
                                              false, "cast", entry);
      }
      ReturnInst::Create(*m_pLLVMContext, pRetVal, entry);
    } else if (CompilationUtils::isSubGroupAll(func_name))
      replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_ALL);
    else if (CompilationUtils::isSubGroupAny(func_name))
      replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_ANY);
    else if (CompilationUtils::isGetNumSubGroups(func_name))
      replaceWithConst(pFunc, 1, false);
    else if (CompilationUtils::isGetEnqueuedNumSubGroups(func_name))
      replaceWithConst(pFunc, 1, false);
    else if (CompilationUtils::isGetSubGroupId(func_name))
      replaceWithConst(pFunc, 0, false);
    else if (CompilationUtils::isSubGroupBroadCast(func_name))
      defineSubGroupBroadcast(pFunc);
    else if (CompilationUtils::isSubGroupReduceAdd(func_name))
      replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_REDUCE_ADD);
    else if (CompilationUtils::isSubGroupReduceMax(func_name))
      replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_REDUCE_MAX);
    else if (CompilationUtils::isSubGroupReduceMin(func_name))
      replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_REDUCE_MIN);
    else if (CompilationUtils::isSubGroupScanExclusiveAdd(func_name))
      replaceFunction(pFunc,
                      CompilationUtils::NAME_WORK_GROUP_SCAN_EXCLUSIVE_ADD);
    else if (CompilationUtils::isSubGroupScanExclusiveMax(func_name))
      replaceFunction(pFunc,
                      CompilationUtils::NAME_WORK_GROUP_SCAN_EXCLUSIVE_MAX);
    else if (CompilationUtils::isSubGroupScanExclusiveMin(func_name))
      replaceFunction(pFunc,
                      CompilationUtils::NAME_WORK_GROUP_SCAN_EXCLUSIVE_MIN);
    else if (CompilationUtils::isSubGroupScanInclusiveAdd(func_name))
      replaceFunction(pFunc,
                      CompilationUtils::NAME_WORK_GROUP_SCAN_INCLUSIVE_ADD);
    else if (CompilationUtils::isSubGroupScanInclusiveMax(func_name))
      replaceFunction(pFunc,
                      CompilationUtils::NAME_WORK_GROUP_SCAN_INCLUSIVE_MAX);
    else if (CompilationUtils::isSubGroupScanInclusiveMin(func_name))
      replaceFunction(pFunc,
                      CompilationUtils::NAME_WORK_GROUP_SCAN_INCLUSIVE_MIN);
    else if (CompilationUtils::isSubGroupReserveReadPipe(func_name))
      replaceFunction(pFunc,
                      CompilationUtils::NAME_WORK_GROUP_RESERVE_READ_PIPE);
    else if (CompilationUtils::isSubGroupCommitReadPipe(func_name))
      replaceFunction(pFunc,
                      CompilationUtils::NAME_WORK_GROUP_COMMIT_READ_PIPE);
    else if (CompilationUtils::isSubGroupReserveWritePipe(func_name))
      replaceFunction(pFunc,
                      CompilationUtils::NAME_WORK_GROUP_RESERVE_WRITE_PIPE);
    else if (CompilationUtils::isSubGroupCommitWritePipe(func_name))
      replaceFunction(pFunc,
                      CompilationUtils::NAME_WORK_GROUP_COMMIT_WRITE_PIPE);
  }
  return true;
}

void SubGroupAdaptation::replaceFunction(Function *oldFunc,
                                         std::string newFuncName) {
  std::string m_name = oldFunc->getName();
  std::string newName(newFuncName);
  if (!CompilationUtils::isPipeBuiltin(m_name)) {
    reflection::FunctionDescriptor fd = demangle(m_name.c_str());
    fd.name = newFuncName;
    newName = mangle(fd);
  }
  Function *newF = cast<Function>(m_pModule->getOrInsertFunction(
      newName, oldFunc->getFunctionType(), oldFunc->getAttributes()));
  newF->setCallingConv(oldFunc->getCallingConv());
  oldFunc->replaceAllUsesWith(newF);
  oldFunc->eraseFromParent();
}

void SubGroupAdaptation::replaceWithConst(Function *oldFunc, unsigned constInt,
                                          bool isSigned) {
  std::vector<Instruction *> callSgFunc;

  for (const auto &ui : oldFunc->users()) {
    CallInst *pCallInst = dyn_cast<CallInst>(ui);
    // Found a call instruction to sub-group built-in, collect it.
    if (pCallInst != nullptr)
      callSgFunc.push_back(pCallInst);
  }

  for (auto CI : callSgFunc) {
    Value *pConstInt = ConstantInt::get(CI->getType(), constInt, isSigned);
    CI->replaceAllUsesWith(pConstInt);
    CI->eraseFromParent();
  }
  oldFunc->eraseFromParent();
}

void SubGroupAdaptation::defineSubGroupBroadcast(Function *pFunc) {
  BasicBlock *entry = BasicBlock::Create(*m_pLLVMContext, "entry", pFunc);
  SmallVector<Value *, 4> params;

  CallInst *local_size_0 =
      getWICall(entry, "lsz0", CompilationUtils::mangledGetLocalSize(), 0);
  CallInst *local_size_1 =
      getWICall(entry, "lsz1", CompilationUtils::mangledGetLocalSize(), 1);

  assert((pFunc->arg_size() > 1) && "Expect at least 2 args!");
  auto firstArg = pFunc->arg_begin();
  params.push_back(&*firstArg);
  auto secondArg = ++firstArg;

  // For 1-dim workgroup - return get_local_id(0)
  // <3-dimensional Linear-ID> % get local_size(0)
  Value *linid = &*secondArg;
  if (linid->getType() != local_size_0->getType())
    linid = CastInst::CreateIntegerCast(linid, local_size_0->getType(), false,
                                        "linid", entry);
  Instruction *lid0 =
      BinaryOperator::CreateURem(linid, local_size_0, "lid0", entry);
  params.push_back(lid0);

  // Calculate local_id(1):
  //  ((<3-dimensional Linear-ID> / get_local_size(0)) % get_local_size(1)
  BinaryOperator *rd1 =
      BinaryOperator::CreateUDiv(linid, local_size_0, "lid1.op", entry);
  BinaryOperator *lid1 =
      BinaryOperator::CreateURem(rd1, local_size_1, "lid1", entry);
  params.push_back(lid1);

  // Calculate local_id(2):
  // (<3-dimensional Linear-ID> / get_local_size(0) / get_local_size(1)
  BinaryOperator *lid2 =
      BinaryOperator::CreateUDiv(rd1, local_size_1, "lid2", entry);
  params.push_back(lid2);

  std::string strFuncName = pFunc->getName();
  reflection::FunctionDescriptor fd = demangle(strFuncName.c_str());

  // replace built-in name
  fd.name = CompilationUtils::NAME_WORK_GROUP_BROADCAST.c_str();
  fd.parameters.pop_back();

  reflection::RefParamType F;
  if (m_pSizeT->getPrimitiveSizeInBits() == 64)
    F = new reflection::PrimitiveType(reflection::PRIMITIVE_ULONG);
  else
    F = new reflection::PrimitiveType(reflection::PRIMITIVE_UINT);

  for (unsigned int i = 0; i < 3; ++i) {
    fd.parameters.push_back(F);
  }
  std::string newFuncName = mangle(fd);

  std::vector<Type *> types;
  types.push_back(firstArg->getType());
  for (unsigned int i = 0; i < 3; ++i)
    types.push_back(m_pSizeT);

  FunctionType *pNewFuncType =
      FunctionType::get(pFunc->getReturnType(), types, false);
  Function *newF = cast<Function>(m_pModule->getOrInsertFunction(
      newFuncName, pNewFuncType, pFunc->getAttributes()));
  CallInst *pNewCall = CallInst::Create(newF, ArrayRef<Value *>(params),
                                        "CallWGBroadCast", entry);

  pNewCall->setCallingConv(pFunc->getCallingConv());
  newF->setCallingConv(pFunc->getCallingConv());
  ReturnInst::Create(*m_pLLVMContext, pNewCall, entry);
}

CallInst *SubGroupAdaptation::getWICall(BasicBlock *pAtEnd,
                                        char const *instName,
                                        std::string funcName, unsigned dimIdx) {
  Type *pInt32Type = Type::getInt32Ty(*m_pLLVMContext);
  ;
  FunctionType *pFuncType = FunctionType::get(m_pSizeT, pInt32Type, false);
  Function *func =
      cast<Function>(m_pModule->getOrInsertFunction(funcName, pFuncType));
  func->setCallingConv(CallingConv::SPIR_FUNC);
  CallInst *pCall = CallInst::Create(
      func, ArrayRef<Value *>(ConstantInt::get(pInt32Type, dimIdx)), instName,
      pAtEnd);
  assert(pCall && "Couldn't create CALL instruction!");
  pCall->setCallingConv(CallingConv::SPIR_FUNC);
  return pCall;
}
}
