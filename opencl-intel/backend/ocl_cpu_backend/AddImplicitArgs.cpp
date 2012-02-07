/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  AddImplicitArgs.cpp

\*****************************************************************************/

#include "AddImplicitArgs.h"
#include "CompilationUtils.h"
#include "TLLVMKernelInfo.h"

#include "llvm/Support/InstIterator.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/Transforms/Utils/Cloning.h"

extern "C" int getVectorizerFunctions(Pass *V, SmallVectorImpl<Function*> &Functions);

namespace Intel { namespace OpenCL { namespace DeviceBackend {
  
  /// @brief Creates new AddImplicitArgs module pass
  /// @returns new AddImplicitArgs module pass  
  ModulePass* createAddImplicitArgsPass(Pass *pVect, SmallVectorImpl<Function*> &vectFunctions) {
    return new AddImplicitArgs(pVect, vectFunctions);
  }

  char AddImplicitArgs::ID = 0;

  AddImplicitArgs::AddImplicitArgs(
    Pass *pVectorizer, SmallVectorImpl<Function*> &vectFunctions) :
    ModulePass(ID), m_pVectorizer(pVectorizer), m_pVectFunctions(&vectFunctions) {
  }

  bool AddImplicitArgs::runOnModule(Module &M) {
    m_pModule = &M;
    m_pLLVMContext = &M.getContext();
    m_localBuffersAnalysis = &getAnalysis<LocalBuffersAnalysis>();

    // Clear call instruction to fix container
    m_fixupCalls.clear();

    // Add WI structure declarations to the module
    addWIInfoDeclarations();

    // Check that vectorizer pass is enabled
    if ( m_pVectorizer ) {
      // Get vectorized functions from vectorizer pass
      getVectorizerFunctions(m_pVectorizer, *m_pVectFunctions);
    }
    std::set<Function*> kernelsFunctionSet;
    CompilationUtils::getAllScalarKernels(kernelsFunctionSet, m_pModule);
    //Now add all vectorized kernels
    //TODO: should we take care of NULL values?
    kernelsFunctionSet.insert(m_pVectFunctions->begin(), m_pVectFunctions->end());

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

    // Check that vectorizer pass is enabled
    if ( m_pVectorizer ) {
      // Get vectorized functions from vectorizer pass
      getVectorizerFunctions(m_pVectorizer, *m_pVectFunctions);
      // Update vectorized functions with the new functions
      for ( unsigned int i=0; i < m_pVectFunctions->size(); ++i  ) {
        // Check if this function is a vectorized function
        Function *pVecFunc = (*m_pVectFunctions)[i];
        if ( pVecFunc ) {
          // It is a vectorized function, update it with its new function
          (*m_pVectFunctions)[i] = m_oldToNewFunctionMap[pVecFunc];
        }
      }
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
      for ( unsigned int i = 0; i < pCall->getNumArgOperands() - CompilationUtils::NUMBER_IMPLICIT_ARGS; ++i ) {
        params.push_back(pCall->getArgOperand(i));
      }

      // Add implicit parameters
      params.push_back(pCallArgs[0]);
      params.push_back(pCallArgs[1]);
      params.push_back(pCallArgs[2]);
      params.push_back(pCallArgs[3]);
      params.push_back(pCallArgs[4]);
      params.push_back(pCallArgs[5]);
      params.push_back(pCallArgs[6]);
      params.push_back(pCallArgs[7]);
      params.push_back(pCallArgs[8]);

      CallInst *pNewCall = CallInst::Create(pCallee, ArrayRef<Value*>(params), "", pCall);
      pNewCall->setCallingConv(pCall->getCallingConv());

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
    
    unsigned int uiSizeT = m_pModule->getPointerSize()*32;

    newArgsVec.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 3));
    newArgsVec.push_back(PointerType::get(m_struct_WorkDim, 0));
    newArgsVec.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
    //newArgsVec.push_back(PointerType::get(m_pModule->getTypeByName("struct.PaddedDimId"), 0));
    //newArgsVec.push_back(PointerType::get(m_pModule->getTypeByName("struct.PaddedDimId"), 0));
    newArgsVec.push_back(PointerType::get(m_struct_PaddedDimId, 0));
    newArgsVec.push_back(PointerType::get(m_struct_PaddedDimId, 0));
    newArgsVec.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
    newArgsVec.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    newArgsVec.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
    newArgsVec.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));

    FunctionType *FTy = FunctionType::get( pFunc->getReturnType(),newArgsVec, false);

    // Create a new function with explicit and implict arguments types
    Function *pNewF = Function::Create(FTy, pFunc->getLinkage(), pFunc->getName());
    if ( isAKernel ) {
      // Only those who are kernel functions need to get this calling convention
      pNewF->setCallingConv(CallingConv::C);
    }

    llvm::ValueToValueMapTy ValueMap;

    // Set explict arguments' names
    Function::arg_iterator DestI = pNewF->arg_begin();
    for ( Function::arg_iterator I = pFunc->arg_begin(),
      E = pFunc->arg_end(); I != E; ++I, ++DestI ) {
        DestI->setName(I->getName());
        ValueMap[I] = DestI;
    }

    // Set implict arguments' names
    DestI->setName("pLocalMem");
    Argument *pLocalMem = DestI;
    ++DestI;
    DestI->setName("pWorkDim");
    Argument *pWorkDim = DestI;
    ++DestI;
    DestI->setName("pWGId");
    Argument *pWGId = DestI;
    ++DestI;
    DestI->setName("pBaseGlbId");
    Argument *pBaseGlbId = DestI;
    ++DestI;
    DestI->setName("pLocalIds");
    Argument *pLocalId = DestI;
    ++DestI;
    DestI->setName("contextpointer");
    Argument *pctx = DestI;
    ++DestI;
    DestI->setName("iterCount");
    Argument *pIterCount = DestI;
    ++DestI;
    DestI->setName("pSpecialBuf");
    Argument *pSpecialBuf = DestI;
    ++DestI;
    DestI->setName("pCurrWI");
    Argument *pCurrWI = DestI;


    SmallVector<ReturnInst*, 8> Returns;
    CloneFunctionInto(pNewF, pFunc, ValueMap, true, Returns, "", NULL);

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

        // Calculate pointer to the local memory buffer
        GetElementPtrInst *pNewLocalMem = GetElementPtrInst::Create(
          pLocalMem, ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), directLocalSize), "", pCall);

        Value **pCallArgs = new Value*[CompilationUtils::NUMBER_IMPLICIT_ARGS];
        pCallArgs[0] = pNewLocalMem;
        pCallArgs[1] = pWorkDim;
        pCallArgs[2] = pWGId;
        pCallArgs[3] = pBaseGlbId;
        pCallArgs[4] = pLocalId;
        pCallArgs[5] = pctx;
        pCallArgs[6] = pIterCount;
        pCallArgs[7] = pSpecialBuf;
        pCallArgs[8] = pCurrWI;

        // Memory leak in here. Who deletes this ? Seriously! 
        m_fixupCalls[pCall] = pCallArgs;

      }
    }
    
    std::vector<Value*> uses(pFunc->use_begin(), pFunc->use_end());
    for (std::vector<Value*>::const_iterator it = uses.begin(), e = uses.end();
       it != e; ++it) {
      CallInst *CI = dyn_cast<CallInst>(*it);
      // We do not handle non CallInst users. Is this okay ?
      if (!CI) continue;

      std::vector<Value*> arguments;
      // Push existing arguments
      for (unsigned i=0; i < CI->getNumArgOperands(); ++i)
      {
        arguments.push_back(CI->getArgOperand(i));
      }
      
      // Push undefs for new arguments
      unsigned int numOriginalParams = CI->getNumArgOperands();
      for (unsigned i = numOriginalParams; i < numOriginalParams + CompilationUtils::NUMBER_IMPLICIT_ARGS; ++i) {
        arguments.push_back(UndefValue::get(newArgsVec[i]));
      }
        
      // Replace the original function with a call 
      CallInst* pNewCall = CallInst::Create(pNewF, ArrayRef<Value*>(arguments), "", CI);
            
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

    // Add declaration of original function with its signature
    //pFunc->replaceAllUsesWith(pNewF);
    pFunc->setName("__" + pFunc->getName() + "_original");
    m_pModule->getFunctionList().push_back(pNewF);

    // Delete original function body
    pFunc->deleteBody();

    // Map original function to new function
    
    m_oldToNewFunctionMap[pFunc] = pNewF;

    for ( Value::use_iterator UI = pNewF->use_begin(),
      UE = pNewF->use_end(); UI != UE; ++UI ) {
       if (CallInst *I = dyn_cast<CallInst>(*UI)) {
         // Change the calling convention of the call site to match the
         // calling convention of the called function.
         I->setCallingConv(pNewF->getCallingConv());
       }
    }

    Module *pModule = pFunc->getParent();
    Module::named_metadata_iterator MDIter = pModule->named_metadata_begin();
    Module::named_metadata_iterator EndMDIter = pModule->named_metadata_end();

    for(;MDIter != EndMDIter; MDIter++)
    {
      for(int ui = 0, ue = MDIter->getNumOperands(); ui < ue; ui++)
      { 
        // Replace metadata with metada containing information about the wrapper
        MDNode* pMetadata = MDIter->getOperand(ui);
        
        SmallVector<Value *, 16> values;
        for (int i = 0, e = pMetadata->getNumOperands(); i < e; ++i) {
          Value *elem = pMetadata->getOperand(i);

          if(pFunc == dyn_cast<Function>(elem))
            elem = pNewF;

          values.push_back(elem);
        }
          
        // &(values[0]) gets the pointer to the metadata values array
        MDNode* pNewMetadata = MDNode::get(*m_pLLVMContext, ArrayRef<Value*>(values));
        // TODO: Why may pMetadata and pNewMetadata be the same value ?
        if (pMetadata != pNewMetadata)
          pMetadata->replaceAllUsesWith(pNewMetadata);
      }
    }

    return pNewF;
  }

  void AddImplicitArgs::addWIInfoDeclarations() {
    // Detect size_t size
    unsigned int uiSizeT = m_pModule->getPointerSize()*32;
    /*
      struct sLocalId 
      {
        size_t  Id[MAX_WORK_DIM];
      };
    */
    // Create Work Group/Work Item info structures
    std::vector<Type*> members;
    assert(CPU_MAX_WI_DIM_POW_OF_2 == 4 && "CPU_MAX_WI_DIM_POW_OF_2 is not equal to 4!");
    //use 4 instead of MAX_WORK_DIM for alignment & for better calculation of offset in Local ID buffer
    members.push_back(ArrayType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), CPU_MAX_WI_DIM_POW_OF_2)); // Local Id's
    StructType *PaddedDimId = StructType::get(*m_pLLVMContext, members, true);
    //m_pModule->addTypeName("struct.PaddedDimId", PaddedDimId);
    m_struct_PaddedDimId = PaddedDimId;

    /*
      struct sWorkInfo
      {
        unsigned int  uiWorkDim;
        size_t        GlobalOffset[MAX_WORK_DIM];
        size_t        GlobalSize[MAX_WORK_DIM];
        size_t        LocalSize[MAX_WORK_DIM];
        size_t        WGNumber[MAX_WORK_DIM];
      };
    */
    members.clear();
    members.push_back(IntegerType::get(*m_pLLVMContext, 32));
    members.push_back(ArrayType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), MAX_WORK_DIM)); // Global offset
    members.push_back(ArrayType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), MAX_WORK_DIM)); // Global size
    members.push_back(ArrayType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), MAX_WORK_DIM)); // WG size/Local size
    members.push_back(ArrayType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), MAX_WORK_DIM)); // Number of groups
    StructType *pWorkDimType = StructType::get(*m_pLLVMContext, members, false);
    //m_pModule->addTypeName("struct.WorkDim", pWorkDimType);
    m_struct_WorkDim = pWorkDimType;
  }


}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {