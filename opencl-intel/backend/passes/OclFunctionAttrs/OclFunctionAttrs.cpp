/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "OclFunctionAttrs.h"
#include "OCLPassSupport.h"
#include "CompilationUtils.h"
#include "LoopUtils/LoopUtils.h"
#include "OCLAddressSpace.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/InstIterator.h"

extern "C" {
  void* createOclFunctionAttrsPass() {
    return new intel::OclFunctionAttrs();
  }
}

extern "C" {
  void* createOclSyncFunctionAttrsPass() {
    return new intel::OclSyncFunctionAttrs();
  }
}

using namespace Intel::OpenCL::DeviceBackend;
using namespace Utils::OCLAddressSpace;
namespace intel{

/*****************************************************************/
/*                       OclFunctionAttrs                        */
/*****************************************************************/
  char OclFunctionAttrs::ID = 0;

  OclFunctionAttrs::OclFunctionAttrs() : ModulePass(ID) {}
  OCL_INITIALIZE_PASS(OclFunctionAttrs, "ocl-functionattrs", "Deduce function attributes for OpenCL", false, false)

  // isFunctionMayBeCalled - Returns true if the function may be called from
  // with the module
  static bool isFunctionMayBeCalled(const Function &F) {
    // If there may be Clang Blocks present in the module, pessimistically
    // assume this function is a callee
    if (CompilationUtils::getCLVersionFromModuleOrDefault(*F.getParent()) ==
        OclVersion::CL_VER_2_0)
      return true;
    for (Function::const_user_iterator U = F.user_begin(), UE = F.user_end();
         U != UE; ++U)
      if (isa<CallInst>(*U))
        return true;
    return false;
  }

  // setNoAlias - Sets the NoAlias attribute to one or all arguments in 'V'. If
  // ReturnAfterFirst is true, sets the NoAlias attribute to the first argument
  // in V without this attribute, otherwise sets all arguments in V with the
  // attribute. 'NoAlias' is passed in for efficiency.
  static void setNoAlias(SmallVectorImpl<Argument *> &V,
                         const AttributeSet &NoAlias, bool ReturnAfterFirst) {
    for (SmallVectorImpl<Argument *>::iterator AI = V.begin(), AE = V.end();
         AI != AE; ++AI) {
      if (!(*AI)->hasNoAliasAttr()) {
        (*AI)->addAttr(NoAlias);
        if (ReturnAfterFirst)
          return;
      }
    }
  }

  // AddNoAliasAttrs - Attempts to add the 'NoAlias' attribute to function
  // arguments
  static bool AddNoAliasAttrs(Function &F, bool isKernel) {
    // Rules:
    // 1. If exists a kernel function, F, s.t. F is not called by other functions,
    // then any local memory argument of F is safe to be marked as NoAlias
    // 2. If the kernel is called by other function, treat local memory
    // arguments the same as other address spaces using the following rules
    // TODO: for now rules #1-2 are disabled in OpenCL 2.0 mode until we understand
    // how are they affected by dynamic NDRange
    // 3. Given there are N arguments from address space A and N-1 of these
    // arguments are marked as NoAlias, then mark the remaining argument as
    // NoAlias (holds true also for N=1)
    // 4. Constant and Global address spaces are considered to be same, since two buffers
    // may be initialized from the same host memory
    // 5. If there exist any arguments of the generic address space, we must
    // pessistically assume they will collide with other pointer arguments
    if (F.isDeclaration())
      return false;
    // Keeps the function's pointer arguments in address space buckets
    SmallVector<Argument *, 16> Args[LastStaticAddrSpace + 1];
    // Keeps the number of function args with NoAlias per addrspace
    unsigned NumArgsNoAlias[LastStaticAddrSpace + 1] = {0};
    // Analyze the function arguments
    for (Function::arg_iterator AI = F.arg_begin(), AE = F.arg_end(); AI != AE;
         ++AI) {
      Argument &A = *AI;
      if (!A.getType()->isPointerTy())
        continue;
      PointerType *T = cast<PointerType>(A.getType());
      unsigned AS = T->getAddressSpace();
      if (AS == Generic) {
        if (!A.hasNoAliasAttr())
          return false;
        continue;
      }
      // Global and Constant address spaces will be sharing same bucket. See
      // comments above.
      if (AS == Utils::OCLAddressSpace::Constant)
        AS = Global;
      Args[AS].push_back(&A);
      if (A.hasNoAliasAttr())
        NumArgsNoAlias[AS]++;
    }
    // Modify the arguments
    AttributeSet NoAlias = AttributeSet::get(F.getContext(), 0, Attribute::NoAlias);
    bool Changed = false;
    for (unsigned AS = 0; AS < LastStaticAddrSpace + 1; ++AS) {
      if (Args[AS].size() && Args[AS].size() - 1 == NumArgsNoAlias[AS]) {
        setNoAlias(Args[AS], NoAlias, true);
        Changed = true;
      }
    }
    // local addrspace has its additional special rules as listed above
    if (!Args[Local].size())
      return Changed;
    // If kernel function and is not called by any other functions, can set all
    // local args to NoAlias
    if (isKernel && !isFunctionMayBeCalled(F)) {
      setNoAlias(Args[Local], NoAlias, false);
      Changed = true;
    }
    return Changed;
  }

  static bool runOnFunction(Function &F, bool isKernel) {
    return AddNoAliasAttrs(F, isKernel);
  }

  bool OclFunctionAttrs::runOnModule(Module &M) {
    CompilationUtils::FunctionSet Kernels;
    CompilationUtils::getAllKernels(Kernels, &M);

    bool Changed = false;
    for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I)
      Changed = runOnFunction(*I, Kernels.count(&*I)) || Changed;
    return Changed;
  }

/*****************************************************************/
/*                     OclSyncFunctionAttrs                      */
/*****************************************************************/

  char OclSyncFunctionAttrs::ID = 0;

  OclSyncFunctionAttrs::OclSyncFunctionAttrs() : ModulePass(ID) {}
  OCL_INITIALIZE_PASS(OclSyncFunctionAttrs, "ocl-syncfunctionattrs",
    "Mark and propogate synchronize function with relevent attributes", false, false)

  bool OclSyncFunctionAttrs::runOnModule(Module &M) {
    CompilationUtils::FunctionSet oclSyncBuiltins;
    std::set<Function *> oclSyncBuiltinsSet;
    std::set<Function *> oclSyncFunctions;

    // Get all synchronize built-ins declared in module
    CompilationUtils::getAllSyncBuiltinsDcls(oclSyncBuiltins, &M);
    if (oclSyncBuiltins.empty()) {
      // No synchronize functions to mark
      return false;
    }
    // Get all function that calls synchronize built-ins in/direct
    oclSyncBuiltinsSet.insert(oclSyncBuiltins.begin(), oclSyncBuiltins.end());
    LoopUtils::fillFuncUsersSet(oclSyncBuiltinsSet, oclSyncFunctions);

    bool Changed = false;
    for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
      Function* pFunc = &*I;
      if (oclSyncFunctions.count(pFunc) || oclSyncBuiltins.count(pFunc)) {
        pFunc->setAttributes(pFunc->getAttributes().addAttribute(
          pFunc->getContext(), AttributeSet::FunctionIndex, Attribute::NoDuplicate));
        Changed = true;
      }
    }
    return Changed;
  }

} // namespace intel
