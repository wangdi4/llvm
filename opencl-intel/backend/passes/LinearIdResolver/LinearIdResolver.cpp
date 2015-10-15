/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "LinearIdResolver.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "CompilationUtils.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/DataLayout.h"

#include <utility>

using namespace Intel::OpenCL::DeviceBackend;

extern "C" {
  /// @brief Creates new LinearIdResolverPass function pass
  /// @returns new LinearIdResolverPass function pass
  void* createLinearIdResolverPass() {
    return new intel::LinearIdResolver();
  }
}

namespace intel {

  char LinearIdResolver::ID = 0;

  OCL_INITIALIZE_PASS(LinearIdResolver, "linear-id-resolver", "Resolve linear id related WI functions", false, false)

  bool LinearIdResolver::runOnFunction(Function &F) {

    std::vector<std::pair<Instruction *, Instruction *> > InstRepVec;

    Module * M = F.getParent();

    m_zero = ConstantInt::get(Type::getInt32Ty(F.getParent()->getContext()), 0);
    m_one  = ConstantInt::get(Type::getInt32Ty(F.getParent()->getContext()), 1);
    m_two  = ConstantInt::get(Type::getInt32Ty(F.getParent()->getContext()), 2);
    m_ret = IntegerType::get(M->getContext(), M->getDataLayout()->getPointerSizeInBits(0));

    // looking for get_{global,local}_linear_id()
    for (inst_iterator itr = inst_begin(F);
        itr != inst_end(F);
        ++itr) {

      CallInst * CI = dyn_cast<CallInst>(&(*itr));
      if (!CI) continue;

      Function * func = CI->getCalledFunction();
      if (!func) continue;

      std::string funcName = func->getName().str();

      if(CompilationUtils::isGetGlobalLinearId(funcName)) {
        InstRepVec.push_back(std::pair<Instruction *, Instruction *> (CI, replaceGetGlobalLinearId(M, CI)));
      }
      else if(CompilationUtils::isGetLocalLinearId(funcName)) {
        InstRepVec.push_back(std::pair<Instruction *, Instruction *> (CI, replaceGetLocalLinearId(M, CI)));
      }
    }

    // Updating the usage of the new calculation
    for (std::vector<std::pair<Instruction *, Instruction *> >::iterator itr = InstRepVec.begin();
        itr != InstRepVec.end();
        ++itr) {
      Instruction * from = itr->first;
      Instruction * to   = itr->second;

      from->replaceAllUsesWith(to);
      from->eraseFromParent();
    }

    if (InstRepVec.empty())
      return false;

    return true;
  }

  Instruction * LinearIdResolver::replaceGetGlobalLinearId(Module * M, Instruction * insertBefore) {
    // Replace get_global_linear_id() with the following sequence
    // (get_global_id(2) – get_global_offset(2)) * get_global_size(1) * get_global_size(0) +
    // (get_global_id(1) – get_global_offset(1)) * get_global_size(0) +
    // (get_global_id(0) – get_global_offset(0))
    //    ==
    // ((get_global_id(2) – get_global_offset(2))
    //  * get_global_size(1)
    //  + (get_global_id(1) – get_global_offset(1)))
    // * get_global_size(0)
    // + (get_global_id(0) – get_global_offset(0))
    std::string idName   = CompilationUtils::mangledGetGID();
    std::string sizeName = CompilationUtils::mangledGetGlobalSize();
    std::string offName  = CompilationUtils::mangledGetGlobalOffset();

    // call get_global_id(0), get_global_id(1), get_global_id(2)
    CallInst * gid2 = createWIFunctionCall(M, "gid2", idName, insertBefore, m_two);
    CallInst * gid1 = createWIFunctionCall(M, "gid1", idName, insertBefore, m_one);
    CallInst * gid0 = createWIFunctionCall(M, "gid0", idName, insertBefore, m_zero);
    assert (gid0 && gid1 && gid2 && "Can't create get_global_id calls");

    // call get_global_offset(0), get_global_offset(1), get_global_offset(2)
    CallInst * gof2 = createWIFunctionCall(M, "gof2", offName, insertBefore, m_two);
    CallInst * gof1 = createWIFunctionCall(M, "gof1", offName, insertBefore, m_one);
    CallInst * gof0 = createWIFunctionCall(M, "gof0", offName, insertBefore, m_zero);
    assert (gof0 && gof1 && gof2 && "Can't create get_global_offset calls");

    // call get_global_size(0), get_global_size(1)
    CallInst * gsz1 = createWIFunctionCall(M, "gsz1", sizeName, insertBefore, m_one);
    CallInst * gsz0 = createWIFunctionCall(M, "gsz0", sizeName, insertBefore, m_zero);
    assert (gsz0 && gsz1 && "Can't create get_global_size calls");

    // ((get_global_id(2) – get_global_offset(2))
    BinaryOperator * op0 = BinaryOperator::Create(Instruction::Sub, gid2, gof2, "lgid.op0", insertBefore);
    //  * get_global_size(1)
    BinaryOperator * op1 = BinaryOperator::Create(Instruction::Mul, op0,  gsz1, "lgid.op1", insertBefore);
    //  + (get_global_id(1) – get_global_offset(1)))
    BinaryOperator * op2 = BinaryOperator::Create(Instruction::Sub, gid1, gof1, "lgid.op2", insertBefore);
    BinaryOperator * op3 = BinaryOperator::Create(Instruction::Add, op1,  op2,  "lgid.op3", insertBefore);
    // * get_global_size(0)
    BinaryOperator * op4 = BinaryOperator::Create(Instruction::Mul, op3,  gsz0, "lgid.op4", insertBefore);
    // + (get_global_id(0) – get_global_offset(0))
    BinaryOperator * op5 = BinaryOperator::Create(Instruction::Sub, gid0, gof0, "lgid.op5", insertBefore);
    BinaryOperator * res = BinaryOperator::Create(Instruction::Add, op4,  op5,  "lgid.res", insertBefore);
    return res;
  }

  Instruction * LinearIdResolver::replaceGetLocalLinearId(Module * M, Instruction * insertBefore) {
    // Replace get_local_linear_id() with the following sequence.
    // get_local_id(2) * get_local_size(1) * get_local_size(0) +
    // get_local_id(1) * get_local_size(0) +
    // get_local_id(0)
    //    ==
    // (get_local_id(2) * get_local_size(1)
    //  + get_local_id(1))
    // * get_local_size(0)
    // + get_local_id(0)
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
    BinaryOperator * op0 = BinaryOperator::Create(Instruction::Mul, lid2, lsz1, "llid.op0", insertBefore);
    //  + get_local_id(1))
    BinaryOperator * op1 = BinaryOperator::Create(Instruction::Add, op0, lid1, "llid.op1", insertBefore);
    // * get_local_size(0)
    BinaryOperator * op2 = BinaryOperator::Create(Instruction::Mul, op1, lsz0, "llid.op2", insertBefore);
    // + get_local_id(0)
    BinaryOperator * res = BinaryOperator::Create(Instruction::Add, op2, lid0, "llid.res", insertBefore);
    return res;
  }

  CallInst * LinearIdResolver::createWIFunctionCall(Module * M, char const* twine, std::string const& name, Instruction * insertBefore, Value *actPar) {

    Function * func = M->getFunction(name);

    // if the function does not exist then we need to create it
    if (!func) {
      std::vector<Type *> args(1, actPar->getType());

      FunctionType * fType = FunctionType::get(m_ret, args, false);
      func = dyn_cast<Function>(M->getOrInsertFunction(name, fType));
      assert(func && "Failed creating function");
    }

    return CallInst::Create(func, actPar, twine, insertBefore);
  }

}
