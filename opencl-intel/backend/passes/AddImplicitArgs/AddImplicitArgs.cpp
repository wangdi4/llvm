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
#include "llvm/Support/InstIterator.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Attributes.h"
#include "llvm/Version.h"

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
    m_IAA->initDuringRun(M.getPointerSize() * 32);

    // Clear call instruction to fix container
    m_fixupCalls.clear();

    CompilationUtils::FunctionSet kernelsFunctionSet;
    CompilationUtils::getAllKernels(kernelsFunctionSet, m_pModule);

    // Collect all module functions that are not declarations into for handling
    std::vector<Function*> toHandleFunctions;
    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(&*fi);
      if ( !pFunc || pFunc->isDeclaration () ) {
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

    unsigned int directLocalSize =
      (unsigned int) m_localBuffersAnalysis->getDirectLocalsSize(pFunc);

    std::vector<llvm::Type *> newArgsVec;

    // Go over all explicit arguments in the function sugnature
    Function::ArgumentListType::iterator argIt = pFunc->getArgumentList().begin();
    while ( argIt != pFunc->getArgumentList().end() ) {
      newArgsVec.push_back(argIt->getType());
      argIt++;
    }

    // Add implicit arguments to the signature
    for (unsigned i=0; i < ImplicitArgsUtils::IA_NUMBER; ++i)
      newArgsVec.push_back(m_IAA->getArgType(i));

    FunctionType *FTy = FunctionType::get( pFunc->getReturnType(),newArgsVec, false);

    // Change original function name.
    std::string name = pFunc->getName().str();
    pFunc->setName("__" + pFunc->getName() + "_original");
    // Create a new function with explicit and implict arguments types
    Function *pNewF = Function::Create(FTy, pFunc->getLinkage(), name, m_pModule);
    // Copy old function attributes (including attributes on original arguments) to new function.
    pNewF->copyAttributesFrom(pFunc);
    if ( isAKernel ) {
      // Only those who are kernel functions need to get this calling convention
      pNewF->setCallingConv(CallingConv::C);
    }

    // Set explict arguments' names
    Function::arg_iterator DestI = pNewF->arg_begin();
    for ( Function::arg_iterator I = pFunc->arg_begin(),
      E = pFunc->arg_end(); I != E; ++I, ++DestI ) {
        DestI->setName(I->getName());
    }

    // Save location of first Implicit arg
    Function::arg_iterator ImplicitBegin = DestI;
    // Set implict arguments' names
#if LLVM_VERSION == 3200
    Attributes NoAlias = Attributes::get(*m_pLLVMContext, Attributes::NoAlias);
#elif LLVM_VERSION == 3425
    Attributes NoAlias = Attributes::get(*m_pLLVMContext, Attribute::NoAlias);
#else
    AttributeSet NoAlias = AttributeSet::get(*m_pLLVMContext, 0, Attribute::NoAlias);
#endif
    for (unsigned i=0; i < ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS; ++i, ++DestI) {
      assert(DestI != pNewF->arg_end());
      DestI->setName(ImplicitArgsUtils::getArgName(i));
      // We know that all implicit args that are pointers are non-aliasing, but
      // we must be careful are review this assumption every time we add a new implicit arg.
      if (DestI->getType()->isPointerTy())
        DestI->addAttr(NoAlias);
    }
    assert(DestI == pNewF->arg_end());

    // Since we have now created the new function, splice the body of the old
    // function right into the new function, leaving the old body of the function empty.
    pNewF->getBasicBlockList().splice(pNewF->begin(), pFunc->getBasicBlockList());
    assert(pFunc->isDeclaration() && "splice did not work, original function body is not empty!");
    // Delete original function body - this is needed to remove linkage (if exists)
    pFunc->deleteBody();


    // Loop over the argument list, transferring uses of the old arguments over to
    // the new arguments.
    Function::arg_iterator currArg = pNewF->arg_begin();
    for (Function::arg_iterator I = pFunc->arg_begin(), E = pFunc->arg_end(); I != E; ++I, ++currArg) {
      // Replace the users to the new version.
      I->replaceAllUsesWith(currArg);
    }

    // Apple LLVM-IR workaround
    // 1.  Pass WI information structure as the next parameter after given function parameters
    // 2.  We don't want to use TLS for local memory.
    //    Our solution to move all internal local memory blocks to be allocated
    //    by the execution engine and passed within additional parameters to the kernel,
    //    those parameters are not exposed to the user

    // Go through new function instructions and search calls
    for ( inst_iterator ii = inst_begin(pNewF), ie = inst_end(pNewF); ii != ie; ++ii ) {

      CallInst *pCall = dyn_cast<CallInst>(&*ii);
      if ( !pCall ) {
        continue;
      }
      // Call instruction

      // Check call for not inlined module function
      Function *pCallee = pCall->getCalledFunction();
      if ( NULL != pCallee && !pCallee->isDeclaration() ) {
        Value **pCallArgs = new Value*[ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS];
        Function::arg_iterator IA = ImplicitBegin;
        for (unsigned I = 0; I < ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS;
             ++I, ++IA) {
          pCallArgs[I] = static_cast<Value*>(IA);
          //pCallArgs[I] = cast<Value>(IA);
        }
        assert(IA == pNewF->arg_end());
        // Calculate pointer to the local memory buffer for callee
        Value *pLocalMem = pCallArgs[ImplicitArgsUtils::IA_SLM_BUFFER];
        std::string ValName("pLocalMem_");
        ValName += pCallee->getName();
        GetElementPtrInst *pNewLocalMem = GetElementPtrInst::Create(
          pLocalMem, ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), directLocalSize), ValName, pCall);
        pCallArgs[ImplicitArgsUtils::IA_SLM_BUFFER] = pNewLocalMem;

        // Memory leak in here. Who deletes this ? Seriously!
        m_fixupCalls[pCall] = pCallArgs;

      }
    }

    std::vector<Value*> uses(pFunc->use_begin(), pFunc->use_end());
    for (std::vector<Value*>::const_iterator it = uses.begin(), e = uses.end(); it != e; ++it) {
      // handle constant expression with bitcast of function pointer 
      // it handles cases like block_literal global variable definitions
      // Example of case:
      // @__block_literal_global = internal constant { ..., i8*, ... } 
      //    { ..., i8* bitcast (i32 (i8*, i32)* @globalBlock_block_invoke to i8*), ... }
      if(ConstantExpr *CE = dyn_cast<ConstantExpr>(*it)){
        if(CE->getOpcode() == Instruction::BitCast &&
          CE->getType()->isPointerTy()){
            // this case happens when global block variable is used
            Constant *newCE = ConstantExpr::getBitCast(pNewF, CE->getType());
            CE->replaceAllUsesWith(newCE);
            continue;
        }
      }
      // handle call instruction
      else if (CallInst *CI = dyn_cast<CallInst>(*it)){
        replaceCallInst(CI, newArgsVec, pNewF);
      }
      // handle metadata
      else if (isa<MDNode>(*it)){
        // do nothing 
      }
      else{
        // we should not be here
        // unhandled case
        assert(0 && "Unhandled function reference");
      }
    }

    for (Value::use_iterator UI = pNewF->use_begin(), UE = pNewF->use_end(); UI != UE; ++UI) {
      if (CallInst *I = dyn_cast<CallInst>(*UI)) {
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

    for(; MDIter != EndMDIter; MDIter++) {
      for(int ui = 0, ue = MDIter->getNumOperands(); ui < ue; ui++) {
        // Replace metadata with metada containing information about the wrapper
        MDNode* pMetadata = MDIter->getOperand(ui);
        std::set<MDNode *> visited;
        iterateMDTree(pMetadata, visited);
      }
    }

    return pNewF;
  }

  void AddImplicitArgs::iterateMDTree(MDNode* pMetadata, std::set<MDNode*> &visited) {
    // exit condition, avoid inifinte loops due to cyclic metadata
    if (visited.count(pMetadata)) return;
    visited.insert(pMetadata);

    SmallVector<Value *, 16> values;
    for (int i = 0, e = pMetadata->getNumOperands(); i < e; ++i) {
      Value *elem = pMetadata->getOperand(i);
      if (elem) {
        if (MDNode *Node = dyn_cast<MDNode>(elem))
          iterateMDTree(Node, visited);
        // Elem needs to be set again otherwise changes will be undone.
        elem = pMetadata->getOperand(i);
        if (m_pFunc == dyn_cast<Function>(elem))
          elem = m_pNewF;
      }
      values.push_back(elem);
    }
    MDNode* pNewMetadata = MDNode::get(*m_pLLVMContext, ArrayRef<Value*>(values));
    // TODO: Why may pMetadata and pNewMetadata be the same value ?
    if (pMetadata != pNewMetadata)
      pMetadata->replaceAllUsesWith(pNewMetadata);
  }

  void AddImplicitArgs::replaceCallInst(CallInst *CI, const std::vector<Type *>& newArgsVec, Function * pNewF) {
    
    assert(CI && "CallInst is NULL" );
    assert(newArgsVec.size() && "size of arguments vector is 0" );
    assert(pNewF && "function is NULL" );
    
    std::vector<Value*> arguments;
    // Push explicit arguments
    for (unsigned i=0; i < CI->getNumArgOperands(); ++i)
    {
      arguments.push_back(CI->getArgOperand(i));
    }

    // Push undefs for new implicit arguments
    unsigned int numOriginalParams = CI->getNumArgOperands();
    for (unsigned i = numOriginalParams; i < numOriginalParams + ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS; ++i) {
      arguments.push_back(UndefValue::get(newArgsVec[i]));
    }

    // Replace the original function with a call
    CallInst* pNewCall = CallInst::Create(pNewF, ArrayRef<Value*>(arguments), "", CI);

    // Copy debug metadata to new function if available
    if (CI->hasMetadata()) {
      pNewCall->setDebugLoc(CI->getDebugLoc());
    }

    // Update CI in the m_fixupCalls, now the map needs to have only the newly created call
    if (m_fixupCalls.count(CI) > 0) {
      Value **pCallArgs = m_fixupCalls[CI];
      m_fixupCalls.erase(CI);
      m_fixupCalls[pNewCall] = pCallArgs;
    }

    CI->replaceAllUsesWith(pNewCall);
    // Need to erase from parent to make sure there are no uses for the called function when we delete it
    CI->eraseFromParent();
  }

} // namespace intel
