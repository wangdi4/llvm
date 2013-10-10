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

#include "llvm/Support/InstIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Version.h"
#if LLVM_VERSION == 3425
#include "llvm/Target/TargetData.h"
#else
#include "llvm/DataLayout.h"
#endif

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

    std::string idName, sizeName;

    Module * M = F.getParent();

    m_zero = ConstantInt::get(Type::getInt32Ty(F.getParent()->getContext()), 0);
    m_one  = ConstantInt::get(Type::getInt32Ty(F.getParent()->getContext()), 1);
    m_two  = ConstantInt::get(Type::getInt32Ty(F.getParent()->getContext()), 2);
#if LLVM_VERSION == 3425
    m_ret = IntegerType::get(M->getContext(), TargetData(M).getPointerSizeInBits());
#else
    m_ret = IntegerType::get(M->getContext(), DataLayout(M).getPointerSizeInBits());
#endif

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
        idName = CompilationUtils::mangledGetGID();
        sizeName = CompilationUtils::mangledGetGlobalSize();
      }
      else if(CompilationUtils::isGetLocalLinearId(funcName)) {
        idName = CompilationUtils::mangledGetLID();
        sizeName = CompilationUtils::mangledGetLocalSize();
      }
      else {
        continue;
      }

      // Making the new calculation.
      InstRepVec.push_back(std::pair<Instruction *, Instruction *> (CI, replaceGetLinearId(M, CI, idName, sizeName)));
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

  Instruction * LinearIdResolver::replaceGetLinearId(Module * M, Instruction * insertAfter, std::string idName, std::string sizeName) {
  // Replace get_{global,local}_linear_id()
  //  get_{global,local}_linear_id() =
  // (get_{global,local}_id(2) * get_{global,local}_size(1) * get_{global,local}_size(0)) +
  // (get_{global,local}_id(1) * get_{global,local}_size (0)) +
  //  get_{global,local}_id(0) =
  //  ((get_{global,local}_id(2) * get_{global,local}_size(1)) + get_{global,local}_id(1)) * get_{global,local}_size (0)) +
  //  get_{global,local}_id(0)

    // call id functions
    CallInst * id0 = createWIFunctionCall(M, idName, insertAfter, m_zero);
    CallInst * id1 = createWIFunctionCall(M, idName, insertAfter, m_one);
    CallInst * id2 = createWIFunctionCall(M, idName, insertAfter, m_two);

    assert (id0 && id1 && id2 && "Can't create id call");

    // call size functions
    CallInst * sz0 = createWIFunctionCall(M, sizeName, insertAfter, m_zero);
    CallInst * sz1 = createWIFunctionCall(M, sizeName, insertAfter, m_one);

    assert (sz0 && sz1 && "Can't create sz call");

    // (get_{global,local}_id(2) * get_{global,local}_size(1))
    BinaryOperator * id2sz1 = BinaryOperator::Create(Instruction::Mul, id2, sz1, "id2sz1", insertAfter);

    // ((get_{global,local}_id(2) * get_{global,local}_size(1)) + get_{global,local}_id(1))
    BinaryOperator * add1 = BinaryOperator::Create(Instruction::Add, id2sz1, id1, "add1", insertAfter);

    // ((get_{global,local}_id(2) * get_{global,local}_size(1)) + get_{global,local}_id(1)) * get_{global,local}_size (0))
    BinaryOperator * add1sz0 = BinaryOperator::Create(Instruction::Mul, add1, sz0 , "add1sz0", insertAfter);

    //  ((get_{global,local}_id(2) * get_{global,local}_size(1)) + get_{global,local}_id(1)) * get_{global,local}_size (0)) +
    //  get_{global,local}_id(0)
    BinaryOperator * add2 = BinaryOperator::Create(Instruction::Add, add1sz0, id0, "add2", insertAfter);

    return add2;
  }

  CallInst * LinearIdResolver::createWIFunctionCall(Module * M, std::string name, Instruction * insertAfter, Value *actPar) {

    Function * func = M->getFunction(name);

    // if the function does not exist then we need to create it
    if (!func) {
      std::vector<Type *> args(1, actPar->getType());

      FunctionType * fType = FunctionType::get(m_ret, args, false);
      func = dyn_cast<Function>(M->getOrInsertFunction(name, fType));
      assert(func && "Failed creating function");
    }

    return CallInst::Create(func, actPar, "gid", insertAfter);
  }

}
