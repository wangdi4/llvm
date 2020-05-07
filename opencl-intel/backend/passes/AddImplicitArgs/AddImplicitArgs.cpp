// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "AddImplicitArgs.h"
#include "CompilationUtils.h"
#include "ImplicitArgsUtils.h"
#include "InitializePasses.h"
#include "common_dev_limits.h"
#include "OCLPassSupport.h"
#include "ImplicitArgsUtils.h"

#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/ValueMap.h"

extern "C"{
  /// @brief Creates new AddImplicitArgs module pass
  /// @returns new AddImplicitArgs module pass
  ModulePass* createAddImplicitArgsPass() {
    return new intel::AddImplicitArgs();
  }
}

using namespace Intel::OpenCL::DeviceBackend;

namespace intel{

  char AddImplicitArgs::ID = 0;

  /// Register pass to for opt
  OCL_INITIALIZE_PASS(AddImplicitArgs, "add-implicit-args", "Adds the implicit arguments to signature of all functions of the module (that are defined inside the module)", false, false)

  AddImplicitArgs::AddImplicitArgs() :
    ModulePass(ID), m_pModule(nullptr), m_localBuffersAnalysis(nullptr),
    m_IAA(nullptr), m_pLLVMContext(nullptr), m_struct_WorkDim(nullptr) {
      initializeLocalBuffAnalysisPass(*llvm::PassRegistry::getPassRegistry());
      initializeImplicitArgsAnalysisPass(*llvm::PassRegistry::getPassRegistry());
  }

  bool AddImplicitArgs::runOnModule(Module &M) {
    m_pModule = &M;
    m_pLLVMContext = &M.getContext();
    m_localBuffersAnalysis = &getAnalysis<LocalBuffAnalysis>();
    m_IAA = &getAnalysis<ImplicitArgsAnalysis>();
    m_IAA->initDuringRun(M.getDataLayout().getPointerSizeInBits(0));

    // Clear call instruction to fix container
    m_fixupCalls.clear();
    // Clear functions refs to fix container
    m_fixupFunctionsRefs.clear();

    // Collect all module functions that are not declarations into for handling
    std::vector<Function*> toHandleFunctions;
    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(&*fi);
      if ( !pFunc || pFunc->isDeclaration() ) {
        // Function is not defined inside module
        continue;
      }

      // global ctor's and dtor's don't need implicit arguments, as it only
      // called once by runStaticConstructorsDestructors function.
      // C++ function does not need implicit arguments.
      if (CompilationUtils::isGlobalCtorDtorOrCPPFunc(pFunc))
        continue;

      toHandleFunctions.push_back(pFunc);
    }

    // Run on all collected functions for handlin and handle them
    for (auto *pFunc : toHandleFunctions) {
      runOnFunction(pFunc);
    }

    // update Metadata now
    CompilationUtils::updateFunctionMetadata(m_pModule, m_fixupFunctionsRefs);

    // Indirect calls are not users of any functions - we need to collect them
    // and add stubs for implicit arguments here
    SmallPtrSet<CallInst *, 16> IndirectCalls;
    for (const auto &It : m_fixupCalls) {
      CallInst *CI = It.first;
      if (CI->getCalledFunction())
        continue;
      IndirectCalls.insert(CI);
    }

    for (auto *CI : IndirectCalls) {
      Value **Args = m_fixupCalls[CI];
      SmallVector<Type *, 16> ArgTys;
      for (unsigned I = 0; I < ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS; ++I)
        ArgTys.push_back(Args[I]->getType());

      replaceIndirectCallInst(CI, ArgTys);
    }

    // Go over all call instructions that need to be changed
    // and add implicit arguments to them
    // Call instructions already contain extra agruments, but all of them are
    // Undef's - we need to replace Undef's with the correct values
    while ( !m_fixupCalls.empty() ) {
      CallInst *pCall = m_fixupCalls.begin()->first;
      Value  **pCallArgs = m_fixupCalls.begin()->second;
      m_fixupCalls.erase(m_fixupCalls.begin());

      bool IsDirectCall = pCall->getCalledFunction() != nullptr;

      // Create new call instruction with extended parameters
      SmallVector<Value*, 16> params;

      // Go over explicit parameters, they are currently undef and need to be assigned their actual value.
      for ( unsigned int i = 0; i < pCall->getNumArgOperands() - ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS; ++i ) {
        params.push_back(pCall->getArgOperand(i));
      }

      // Add implicit parameters
      for ( unsigned int i = 0; i < ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS; ++i ) {
        params.push_back(pCallArgs[i]);
      }

      CallInst *pNewCall = CallInst::Create(
          IsDirectCall ? pCall->getCalledFunction() : pCall->getCalledValue(),
          ArrayRef<Value *>(params), "", pCall);
      pNewCall->setCallingConv(pCall->getCallingConv());
      // Copy attributes from the callee which contains aligment for parameters
      if (IsDirectCall)
        pNewCall->setAttributes(pCall->getCalledFunction()->getAttributes());

      // Copy debug metadata to new function if available
      if (pCall->hasMetadata()) {
        pNewCall->setDebugLoc(pCall->getDebugLoc());
      }

      delete [] pCallArgs;

      pCall->replaceAllUsesWith(pNewCall);
      pCall->eraseFromParent();

    }

    return true;
  }

  Function* AddImplicitArgs::runOnFunction(Function *pFunc) {

    SmallVector<llvm::Type *, 16> NewTypes;
    SmallVector<const char *, 16> NewNames;
    SmallVector<AttributeSet, 16> NewAttrs;
    unsigned NumExplicitArgs = pFunc->arg_size();

    // Calculate pointer to the local memory buffer. Do this before pFunc is deleted.
    unsigned int directLocalSize =
        (unsigned int)m_localBuffersAnalysis->getDirectLocalsSize(pFunc);
    AttrBuilder B;
    B.addAttribute(Attribute::NoAlias);
    AttributeSet NoAlias = AttributeSet::get(*m_pLLVMContext, B);
    AttributeSet NoAttr;
    // For each implicit arg, setup its type, name and attributes
    for (unsigned i=0; i < ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS; ++i) {
      Type* ArgType = m_IAA->getArgType(i);
      NewTypes.push_back(ArgType);
      NewNames.push_back(ImplicitArgsUtils::getArgName(i));
      // We know that all implicit args that are pointers are non-aliasing, but
      // we must be careful are review this assumption every time we add a new implicit arg.
      if (ArgType->isPointerTy())
        NewAttrs.push_back(NoAlias);
      else
        NewAttrs.push_back(NoAttr);
    }
    // Create the new function with appended implicit attributes
    Function *pNewF = CompilationUtils::AddMoreArgsToFunc(
        pFunc, NewTypes, NewNames, NewAttrs, "AddImplicitArgs");

    // maintain this map to preserve original/modified relation for functions.
    m_fixupFunctionsRefs[pFunc] = pNewF;

    // Apple LLVM-IR workaround
    // 1.  Pass WI information structure as the next parameter after given function parameters
    // 2.  We don't want to use TLS for local memory.
    //    Our solution to move all internal local memory blocks to be allocated
    //    by the execution engine and passed within additional parameters to the kernel,
    //    those parameters are not exposed to the user

    // Go through new function instructions and search calls
    for (inst_iterator ii = inst_begin(pNewF), ie = inst_end(pNewF); ii != ie;
         ++ii) {
      CallInst *pCall = dyn_cast<CallInst>(&*ii);
      if (!pCall) {
        continue;
      }
      // Call instruction

      // Check call for not inlined module function
      Function *pCallee = pCall->getCalledFunction();
      bool IsDirectCall = (pCallee != nullptr);
      // C++ Callee will be skipped as C++ func does not add implicit arguments.
      // TODO: handle C++ function as indirect callee
      if (IsDirectCall && (pCallee->isDeclaration() ||
          CompilationUtils::isGlobalCtorDtorOrCPPFunc(pCallee)))
        continue;
      StringRef Name = IsDirectCall ? pCallee->getName()
                                    : pCall->getCalledValue()->getName();
      Value **pCallArgs = new Value*[ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS];
      Function::arg_iterator IA = pNewF->arg_begin();
      // Skip over explicit args
      for (unsigned I = 0; I < NumExplicitArgs; ++I, ++IA)
        ;
      // Copy over implicit args
      for (unsigned I = 0; I < ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS;
           ++I, ++IA) {
        pCallArgs[I] = static_cast<Value*>(&*IA);
      }
      assert(IA == pNewF->arg_end());
      // Calculate pointer to the local memory buffer for callee
      Value *pLocalMem = pCallArgs[ImplicitArgsUtils::IA_SLM_BUFFER];
      std::string ValName("pLocalMem_");
      ValName += Name;
      // [LLVM 3.8 UPGRADE] ToDo: Replace nullptr for pointer type with actual
      // type (not using type from pointer as this functionality is planned to
      // be removed.
      Value *pNewLocalMem = GetElementPtrInst::Create(
          nullptr, pLocalMem,
          ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32),
                           directLocalSize), ValName, pCall);
      pCallArgs[ImplicitArgsUtils::IA_SLM_BUFFER] = pNewLocalMem;

      // Memory leak in here. Who deletes this ? Seriously!
      m_fixupCalls[pCall] = pCallArgs;
    }

    SmallVector<User *, 16> Users(pFunc->users());
    // All users which need to be replaced are handled here
    for (User *user : Users) {
      // handle constant expression with bitcast of function pointer
      // it handles cases like block_literal global variable definitions
      // Example of case:
      // @__block_literal_global = internal constant { ..., i8*, ... }
      //    { ..., i8* bitcast (i32 (i8*, i32)* @globalBlock_block_invoke to i8*), ... }
      if(ConstantExpr *CE = dyn_cast<ConstantExpr>(user)){
        if ((CE->getOpcode() == Instruction::BitCast ||
             CE->getOpcode() == Instruction::AddrSpaceCast) &&
            CE->getType()->isPointerTy()) {
          // this case happens when global block variable is used
          Constant *newCE = ConstantExpr::getPointerCast(pNewF, CE->getType());
          CE->replaceAllUsesWith(newCE);
        } else if (CE->getOpcode() == Instruction::PtrToInt) {
          // function pointer is converted into an int
          Constant *NewCE = ConstantExpr::getPtrToInt(pNewF, CE->getType());
          CE->replaceAllUsesWith(NewCE);
        }
      }
      // handle call instruction
      else if (CallInst *CI = dyn_cast<CallInst>(user)){
        replaceCallInst(CI, NewTypes, pNewF);
      }
      else if (auto *CI = dyn_cast<PtrToIntInst>(user)) {
        auto *NewCE = CastInst::CreatePointerCast(pNewF, CI->getType(), "", CI);
        CI->replaceAllUsesWith(NewCE);
        CI->eraseFromParent();
      }
      else if (auto *SI = dyn_cast<StoreInst>(user)) {
        assert(isa<AllocaInst>(SI->getPointerOperand()) &&
               "Expected store of function pointer into an alloca");
        // this function was stored as a function pointer, but the type of
        // function was changed (implicit args were added) - to avoid changing
        // types let's just cast new function type into the old one before
        // storing
        auto *OldAllocaTy =
            cast<PointerType>(SI->getPointerOperand()->getType());
        Type *OldFPtrTy = OldAllocaTy->getElementType();

        auto *Cast = CastInst::CreatePointerCast(pNewF, OldFPtrTy, "", SI);
        auto *NewSI = new StoreInst(Cast, SI->getPointerOperand(), SI);
        SI->replaceAllUsesWith(NewSI);
        SI->eraseFromParent();
      }
      else{
        // we should not be here
        // unhandled case except for SelectInst - they will be handled later
        assert(isa<SelectInst>(user) && "Unhandled function reference");
      }
    }

    // All users which are not to be replaced are handled here
    for (Use &U : pFunc->uses()) {
      User *Usr = U.getUser();
      unsigned OpNo = U.getOperandNo();
      if (auto *SI = dyn_cast<SelectInst>(Usr)) {
        // This function goes though a select instruction, but the type of the
        // function was changed (implicit args were added) - to avoid changing
        // types let's just cast new function type into the old one before
        // select
        if (SI->getOperand(OpNo)->getType() != pNewF->getType()) {
          auto *Cast = CastInst::CreatePointerCast(
              pNewF, SI->getOperand(OpNo)->getType(), "", SI);
          SI->setOperand(OpNo, Cast);
        }
      }
    }

    for (User *user : pNewF->users()) {
      if (CallInst *I = dyn_cast<CallInst>(user)) {
        // Change the calling convention of the call site to match the
        // calling convention of the called function.
        I->setCallingConv(pNewF->getCallingConv());
      }
    }

    return pNewF;
  }

  void AddImplicitArgs::replaceCallInst(CallInst *CI, ArrayRef<Type *> implicitArgsTypes, Function * pNewF) {
    bool FixupCallsNeedsUpdate = m_fixupCalls.count(CI) > 0;
    Value **pCallArgs = 0;
    if (FixupCallsNeedsUpdate) {
      // Remove the old call from the mapping
      pCallArgs = m_fixupCalls[CI];
      m_fixupCalls.erase(CI);
    }
    SmallVector<Value *, 16> NewArgs;
    // Push undefs for new arguments
    assert(implicitArgsTypes.size() == ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS);
    for (unsigned i = 0; i < ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS; ++i) {
      NewArgs.push_back(UndefValue::get(implicitArgsTypes[i]));
    }
    CallInst *pNewCall =
        CompilationUtils::AddMoreArgsToCall(CI, NewArgs, pNewF);

    if (FixupCallsNeedsUpdate) {
      // Place the new call into the mapping
      m_fixupCalls[pNewCall] = pCallArgs;
    }
  }

  void
  AddImplicitArgs::replaceIndirectCallInst(CallInst *CI,
                                           ArrayRef<Type *> implicitArgsTypes) {
    bool FixupCallsNeedsUpdate = m_fixupCalls.count(CI) > 0;
    Value **pCallArgs = nullptr;
    if (FixupCallsNeedsUpdate) {
      // Remove the old call from the mapping
      pCallArgs = m_fixupCalls[CI];
      m_fixupCalls.erase(CI);
    }
    SmallVector<Value *, 16> NewArgs;
    // Push undefs for new arguments
    assert(implicitArgsTypes.size() == ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS);
    for (unsigned i = 0; i < ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS; ++i)
      NewArgs.push_back(UndefValue::get(implicitArgsTypes[i]));

    CallInst *pNewCall =
        CompilationUtils::AddMoreArgsToIndirectCall(CI, NewArgs);

    if (FixupCallsNeedsUpdate) {
      // Place the new call into the mapping
      m_fixupCalls[pNewCall] = pCallArgs;
    }
  }

} // namespace intel
