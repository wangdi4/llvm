/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#define DEBUG_TYPE "BlockToFuncPtr"

#include "BlockToFuncPtrPass.h"
#include "CompilationUtils.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"

#include <llvm/IR/Module.h>

using namespace std;
using namespace llvm;

extern "C" {
  // Creates new BlockToFuncPtr pass
  void* createBlockToFuncPtrPass() {
    return new intel::BlockToFuncPtr();
  }
}

namespace intel {
namespace {
  // Functions to proccess by this pass
  const SmallVector<const char *, 5> FNames =
                                    {"Z14enqueue_kernel",
                                     "Z26get_kernel_work_group_size",
                                     "Z45get_kernel_preferred_work_group_size_multiple",
                                     "Z38get_kernel_sub_group_count_for_ndrange",
                                     "Z41get_kernel_max_sub_group_count_for_ndrange"};
}
  char BlockToFuncPtr::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(BlockToFuncPtr, "block-to-func-ptr",
                  "Cast pointers to OpenCL 2.0 blocks to function pointers", false, false)
  OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
  OCL_INITIALIZE_PASS_END(BlockToFuncPtr, "block-to-func-ptr",
                  "Cast pointers to OpenCL 2.0 blocks to function pointers", false, false)

  static bool isDeviceExecutionBI(const Function* F ) {
    StringRef FName = F->getName();
    for (auto *N : FNames) {
      if (FName.find(N) != std::string::npos)
        return true;
    }
    return false;
  }

  static Function* getFunctionDef(const Function* F,
                                  SmallVectorImpl<Module *> &RTLModules) {
    for (auto *M : RTLModules) {
      assert(M && "Invalid module");
      Function* Def = M->getFunction(F->getName());

      if (Def && !Def->isDeclaration())
        return Def;
    }
    return nullptr;
  }

  bool BlockToFuncPtr::runOnModule(Module &M) {

    BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
    SmallVector<Module *, 2> RTLModules = BLI.getBuiltinModules();

    // Find device execution builtin in user module and replace it with new one
    // casting block argument with pointer to %opencl.block opaque type to function pointer.
    // It is needed to preserve consistency between user and BIs module function arguments
    // and avoid function type bitcasts for device execution BIs in BIImport.
    // This inconsistency can occur when a module translates from SPIR to LLVM.
    // FIXME: Need to replace this functionality to SPIR20BlocksToObjCBlocks Pass.
    SmallVector<Function *, 2> SrcFs;
    SmallVector<Function *, 2> DstFs;
    for (auto &F : M) {
      if (!F.isDeclaration())
        continue;

      if (!isDeviceExecutionBI(&F))
        continue;

      Function *FDef = getFunctionDef(&F, RTLModules);

      // F might be a declaration without usage not removed yet
      assert((FDef || (!FDef && F.use_empty())) &&
            "Invalid function. Used BI not found in BI modules");

      if (!FDef)
        continue;

      Function *NewF = nullptr;
      SmallVector<Instruction *, 8> InstToRemove;
      for (auto FUser : F.users()) {
        CallInst *CI = llvm::dyn_cast<llvm::CallInst>(FUser);
        if (!CI)
          continue;
        bool isChanged = false;
        SmallVector<Value *, 8> NewPars;
        SmallVector<Type *, 8> NewParTys;

        // iterate over call parameters and create bitcast for
        // opencl.block if it was founded here
        Function::arg_iterator DefArg = FDef->arg_begin();
        for (auto &CallPar : CI->arg_operands()) {
          using namespace Intel::OpenCL::DeviceBackend;
          auto ParTy = CompilationUtils::getStructFromTypePtr(CallPar->getType());
          auto ArgTy = CompilationUtils::getStructFromTypePtr(DefArg->getType());
          if (CallPar->getType() == DefArg->getType() ||
             (ParTy && ArgTy && CompilationUtils::isSameStructType(ParTy,ArgTy))) {
            NewPars.push_back(CallPar);
            NewParTys.push_back(CallPar->getType());
            ++DefArg;
            continue;
          }
          assert((ParTy && ParTy->hasName()) &&
                  "Invalid parameter. Call and definition arg types doesn't match");
          StringRef STName = ParTy->getName();
          if (STName.startswith("opencl.block")) {
            NewPars.push_back(BitCastInst::CreatePointerCast(CallPar, DefArg->getType(), "", CI));
            NewParTys.push_back(DefArg->getType());
            ++DefArg;
            isChanged = true;
            continue;
          }
          llvm_unreachable("Invalid parameter. Call and definition arg types doesn't match");
        }
        // Nothing changes, functions have the same types
        if(!isChanged)
          continue;
        // Create new function, if haven't created yet
        if (!NewF) {
          auto *FT = FunctionType::get(F.getReturnType(), NewParTys, F.isVarArg());
          NewF = Function::Create(FT, F.getLinkage(), F.getName());
          NewF->copyAttributesFrom(&F);
          NewF->copyMetadata(&F, 0);
          SrcFs.push_back(&F);
          DstFs.push_back(NewF);
        }
        // Create new callinst for every callinst was founded
        CallInst *NewCI = CallInst::Create(NewF, NewPars, "", CI);
        NewCI->setCallingConv(CI->getCallingConv());
        NewCI->setAttributes(CI->getAttributes());
        if (CI->isTailCall())
          NewCI->setTailCall();
        NewCI->setDebugLoc(CI->getDebugLoc());
        CI->replaceAllUsesWith(NewCI);
        InstToRemove.push_back(CI);
      }
      // Remove callinsts without usage
      for (auto *Inst : InstToRemove) {
        assert(Inst->use_empty() && "Cannot erase used instruction");
        Inst->eraseFromParent();
      }
    }
    // Replace functions without usage with new one
    auto DstFI = DstFs.begin();
    for (auto *S : SrcFs) {
      assert(S->use_empty() && "Cannot erase used function");
      S->eraseFromParent();
      M.getFunctionList().push_front(*DstFI++);
    }

    return true;
  }
} // namespace intel:
