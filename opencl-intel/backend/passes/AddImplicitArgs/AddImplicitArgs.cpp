/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "AddImplicitArgs.h"
#include "CompilationUtils.h"
#include "ImplicitArgsUtils.h"
#include "InitializePasses.h"
#include "common_dev_limits.h"
#include "OCLPassSupport.h"
#include "ImplicitArgsUtils.h"
#include "ImplicitArgsAnalysis/ImplicitArgsAnalysis.h"

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

  AddImplicitArgs::AddImplicitArgs() : ModulePass(ID) {
        initializeLocalBuffAnalysisPass(*llvm::PassRegistry::getPassRegistry());
        initializeImplicitArgsAnalysisPass(*llvm::PassRegistry::getPassRegistry());
  }

  bool AddImplicitArgs::runOnModule(Module &M) {
    m_pModule = &M;
    m_pLLVMContext = &M.getContext();
    m_localBuffersAnalysis = &getAnalysis<LocalBuffAnalysis>();
    m_IAA = &getAnalysis<ImplicitArgsAnalysis>();
    m_IAA->initDuringRun(M.getDataLayout()->getPointerSizeInBits(0));

    // Clear call instruction to fix container
    m_fixupCalls.clear();

    CompilationUtils::FunctionSet kernelsFunctionSet;
    CompilationUtils::getAllKernels(kernelsFunctionSet, m_pModule);

    // Collect all module functions that are not declarations into for handling
    std::vector<Function*> toHandleFunctions;
    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(&*fi);
      if ( !pFunc || pFunc->isDeclaration() ) {
        // Function is not defined inside module
        continue;
      }
      toHandleFunctions.push_back(pFunc);
    }

    // Run on all collected functions for handlin and handle them
    for ( std::vector<Function*>::iterator fi = toHandleFunctions.begin(),
      fe = toHandleFunctions.end(); fi != fe; ++fi ) {
        Function *pFunc = dyn_cast<Function>(*fi);
        bool isAKernel = (kernelsFunctionSet.count(pFunc) > 0);
        runOnFunction(pFunc, isAKernel);
    }

    // Go over all call instructions that need to be changed
    // and add implicit arguments to them
    while ( !m_fixupCalls.empty() ) {
      CallInst *pCall = m_fixupCalls.begin()->first;
      Value  **pCallArgs = m_fixupCalls.begin()->second;
      m_fixupCalls.erase(m_fixupCalls.begin());

      Function *pCallee = pCall->getCalledFunction();
      assert(pCallee && "Indirect function call is not expected!");

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

      CallInst *pNewCall = CallInst::Create(pCallee, ArrayRef<Value*>(params), "", pCall);
      pNewCall->setCallingConv(pCall->getCallingConv());

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

  Function* AddImplicitArgs::runOnFunction(Function *pFunc, bool isAKernel) {

    SmallVector<llvm::Type *, 16> NewTypes;
    SmallVector<const char *, 16> NewNames;
    SmallVector<AttributeSet, 16> NewAttrs;
    unsigned NumExplicitArgs = pFunc->arg_size();

    // Calculate pointer to the local memory buffer. Do this before pFunc is deleted.
    unsigned int directLocalSize =
        (unsigned int)m_localBuffersAnalysis->getDirectLocalsSize(pFunc);
    AttributeSet NoAlias = AttributeSet::get(*m_pLLVMContext, 0, Attribute::NoAlias);
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
        pFunc, NewTypes, NewNames, NewAttrs, "AddImplicitArgs", isAKernel);

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
      if ( NULL != pCallee && !pCallee->isDeclaration() ) {
        Value **pCallArgs = new Value*[ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS];
        Function::arg_iterator IA = pNewF->arg_begin();
        // Skip over explicit args
        for (unsigned I = 0; I < NumExplicitArgs; ++I, ++IA)
          ;
        // Copy over implicit args
        for (unsigned I = 0; I < ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS;
             ++I, ++IA) {
          pCallArgs[I] = static_cast<Value*>(IA);
        }
        assert(IA == pNewF->arg_end());
        // Calculate pointer to the local memory buffer for callee
        Value *pLocalMem = pCallArgs[ImplicitArgsUtils::IA_SLM_BUFFER];
        std::string ValName("pLocalMem_");
        ValName += pCallee->getName();
        Value *pNewLocalMem = GetElementPtrInst::Create(
          pLocalMem, ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), directLocalSize), ValName, pCall);
        pCallArgs[ImplicitArgsUtils::IA_SLM_BUFFER] = pNewLocalMem;

        // Memory leak in here. Who deletes this ? Seriously!
        m_fixupCalls[pCall] = pCallArgs;

      }
    }

    std::vector<User*> users(pFunc->user_begin(), pFunc->user_end());
    for (User *user : users) {
      // handle constant expression with bitcast of function pointer
      // it handles cases like block_literal global variable definitions
      // Example of case:
      // @__block_literal_global = internal constant { ..., i8*, ... }
      //    { ..., i8* bitcast (i32 (i8*, i32)* @globalBlock_block_invoke to i8*), ... }
      if(ConstantExpr *CE = dyn_cast<ConstantExpr>(user)){
        if((CE->getOpcode() == Instruction::BitCast || CE->getOpcode() == Instruction::AddrSpaceCast) &&
          CE->getType()->isPointerTy()){
            // this case happens when global block variable is used
            Constant *newCE = ConstantExpr::getPointerCast(pNewF, CE->getType());
            CE->replaceAllUsesWith(newCE);
            continue;
        }
      }
      // handle call instruction
      else if (CallInst *CI = dyn_cast<CallInst>(user)){
        replaceCallInst(CI, NewTypes, pNewF);
      }
//      [LLVM 3.6 UPGRADE] TODO: MDNode does not derive from Value anymore while User does
//                               so it should be safe to remove the commented code below
//                               along with this comment.
//      // handle metadata
//      else if (isa<MDNode>(*it)){
//        // do nothing
//      }
      else{
        // we should not be here
        // unhandled case
        assert(0 && "Unhandled function reference");
      }
    }

    for (User *user : pNewF->users()) {
      if (CallInst *I = dyn_cast<CallInst>(user)) {
        // Change the calling convention of the call site to match the
        // calling convention of the called function.
        I->setCallingConv(pNewF->getCallingConv());
      }
    }

    Module *pModule = pFunc->getParent();
    Module::named_metadata_iterator MDIter = pModule->named_metadata_begin();
    Module::named_metadata_iterator EndMDIter = pModule->named_metadata_end();
    m_pFunc = pFunc;
    m_pNewF = pNewF;

    // FIXME:
    // This is suboptimal since the loop iterates over all named metadata again and
    // again per each patched function in the module while it can be only done once
    // after all the functions are patched w\ the implicit arguments.
    for(; MDIter != EndMDIter; MDIter++) {
      for(int ui = 0, ue = MDIter->getNumOperands(); ui < ue; ui++) {
        // Replace metadata with metada containing information about the wrapper
        MDNode* pMDNode = MDIter->getOperand(ui);
        std::set<MDNode *> visited;
        iterateMDTree(pMDNode, visited);
      }
    }

    return pNewF;
  }

  void AddImplicitArgs::iterateMDTree(MDNode* pMDNode, std::set<MDNode*> &visited) {
    // Avoid inifinite loops due to possible cycles in metadata
    if (visited.count(pMDNode)) return;
    visited.insert(pMDNode);

    for (int i = 0, e = pMDNode->getNumOperands(); i < e; ++i) {
      Metadata * mdOp = pMDNode->getOperand(i);
      if (mdOp) {
        if (MDNode * mdOpNode = dyn_cast<MDNode>(mdOp)) {
          iterateMDTree(mdOpNode, visited);
        }
        else if(ConstantAsMetadata * funcAsMet = dyn_cast<ConstantAsMetadata>(mdOp)) {
          if (m_pFunc == mdconst::dyn_extract<Function>(funcAsMet))
            pMDNode->replaceOperandWith(i, ConstantAsMetadata::get(m_pNewF));
          // TODO: Check if the old metadata has to bee deleted manually to avoid
          //       memory leaks.
        }
      }
    }
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

} // namespace intel
