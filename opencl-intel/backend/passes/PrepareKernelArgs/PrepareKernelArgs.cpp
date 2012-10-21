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

File Name:  PrepareKernelArgs.cpp

\*****************************************************************************/

#include "PrepareKernelArgs.h"

#include "TypeAlignment.h"
#include "CompilationUtils.h"
#include "ImplicitArgsUtils.h"

#include "llvm/Support/ValueHandle.h"

#include <map>
#include <memory>

namespace Intel { namespace OpenCL { namespace DeviceBackend {
  
  /// @brief Creates new PrepareKernelArgs module pass
  /// @returns new PrepareKernelArgs module pass  
  ModulePass* createPrepareKernelArgsPass(SmallVectorImpl<Function*> &vectFunctions) {
    return new PrepareKernelArgs(vectFunctions);
  }

  char PrepareKernelArgs::ID = 0;

  PrepareKernelArgs::PrepareKernelArgs(SmallVectorImpl<Function*> &vectFunctions) :
    ModulePass(ID), m_pVectFunctions(&vectFunctions) {
  }

  bool PrepareKernelArgs::runOnModule(Module &M) {
    m_pModule = &M;
    m_pLLVMContext = &M.getContext();

    NamedMDNode *KernelsMD = m_pModule->getNamedMetadata("opencl.kernels");

    if (!KernelsMD)
      return false;

    NamedMDNode *WrapperMD = m_pModule->getOrInsertNamedMetadata("opencl.wrappers");
    for(int i = 0, e = KernelsMD->getNumOperands(); i < e ; i++) {
      MDNode *elem = KernelsMD->getOperand(i);
      WrapperMD->addOperand(elem);
    }
    
    // Map functions and their metadata
    CompilationUtils::getKernelsMetadata(m_pModule, *m_pVectFunctions, m_kernelsMetadata);

    // Get all scalar kernels (original kernels, before vectorization)
    std::set<Function*> kernelsFunctionSet;
    CompilationUtils::getAllScalarKernels(kernelsFunctionSet, m_pModule);

    // Run on all scalar functions for handling and handle them
    for ( std::set<Function*>::iterator fi = kernelsFunctionSet.begin(),
      fe = kernelsFunctionSet.end(); fi != fe; ++fi ) {
        runOnFunction(*fi, false);
    }
    
    kernelsFunctionSet.clear();
    
    // Run on all vectorized functions and handle them
    kernelsFunctionSet.insert(m_pVectFunctions->begin(), m_pVectFunctions->end());
    for ( std::set<Function*>::iterator fi = kernelsFunctionSet.begin(),
      fe = kernelsFunctionSet.end(); fi != fe; ++fi ) {
        
        Function* pVectFunc = *fi;
        
        if (pVectFunc != NULL) {
          runOnFunction(pVectFunc, true);
        }
    }

    // TODO : is it ok?
    // Update vectorized functions map with the new functions
    for ( unsigned int i=0; i < m_pVectFunctions->size(); ++i  ) {
      // Check if this function is a vectorized function
      Function *pVecFunc = (*m_pVectFunctions)[i];
      if ( pVecFunc ) {
        // It is a vectorized function, update it with its new function
        (*m_pVectFunctions)[i] = m_oldToNewFunctionMap[pVecFunc];
      }
    }

    return true;
  }
  
  Function* PrepareKernelArgs::createWrapper(Function* pFunc) {
    // Create new function's argument type list
    // The new function receives one argument: i8* pBuffer
    std::vector<llvm::Type *> newArgsVec;
    newArgsVec.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));

    // Create new functions return type
    FunctionType *FTy = FunctionType::get( pFunc->getReturnType(),newArgsVec, false);

    // Create a new function
    Function *pNewF = Function::Create(FTy, pFunc->getLinkage(), pFunc->getName());
    pNewF->setCallingConv(pFunc->getCallingConv());
    
    return pNewF;
  }
  
  std::vector<Value*> PrepareKernelArgs::createArgumentLoads(IRBuilder<>& builder, Function* pFunc, Argument *pArgsBuffer) {

    // Get old function's arguments list in the OpenCL level from its metadata
    std::vector<cl_kernel_argument> arguments;
    CompilationUtils::parseKernelArguments(m_pModule, pFunc, arguments);

    
    std::vector<Value*> params;
    // assuming pBuffer is initially aligned to TypeAlignment::MAX_ALIGNMENT and 
    // therefore currOffset = 0
    // TODO : Can we check this assumption here?
    size_t currOffset = 0;    
    
    llvm::Function::arg_iterator callIt = pFunc->arg_begin();
    
    // TODO :  get common code from the following 2 for loops into a function

    // Handle explicit arguments
    for(unsigned ArgNo = 0; ArgNo <  arguments.size(); ++ArgNo) {
        
      cl_kernel_argument arg = arguments[ArgNo];
      
      // Align the current pArgsBuffer offset based on the alignment of the argument we are about to load
      currOffset = TypeAlignment::align(TypeAlignment::getAlignment(arg), currOffset);
      
      //  %0 = getelementptr i8* %pBuffer, i32 currOffset
      Value* pGEP = builder.CreateGEP(pArgsBuffer, ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), currOffset));
      
      Value* pLoadedValue;
      
      if (arg.type == CL_KRNL_ARG_COMPOSITE) {
        // If this is a struct argument, then the struct itself is passed by value inside pArgsBuffer
        // and the original kernel signature was:
        // foo(..., MyStruct* byval myStruct, ...)
        // meaning pGEP already points to the structure and we do not need to load it
        // we just need to have a bitcast from i8* to MyStruct* and pass the pointer (!!!) to foo
        
        // %myStruct = bitcast i8* to MyStruct*
        // foo(..., %myStruct, ...)

        Value* pBitCast = builder.CreateBitCast(pGEP, callIt->getType());
        pLoadedValue= pBitCast;
      } 
      else
      {
        // Otherwise this is some other type, lets say int4, then int4 itself is passed by value inside pArgsBuffer
        // and the original kernel signature was:
        // foo(..., int4 vec, ...)
        // meaning pGEP points to int4 and we just need to have a bitcast from i8* to int4*
        // load the int4 and pass the loaded value (!!!) to foo
        
        // %pVec = bitcast i8* %0 to int4 *
        // %vec = load int4 * %pVec {, align <alignment> }
        // foo(..., vec, ...)
        
        Value* pBitCast = builder.CreateBitCast(pGEP, PointerType::get(callIt->getType(), 0));
        LoadInst* pLoad = builder.CreateLoad(pBitCast);
        size_t alignment = TypeAlignment::getAlignment(arg);
        if (alignment > 0) {
          pLoad->setAlignment(TypeAlignment::getAlignment(arg));
        }
        pLoadedValue = pLoad;
      }
     
      // Here we mark the load instructions from the struct that are the actual parameters for 
      // the original kernel's restricted formal parameters  
      // This info is used later on in OpenCLAliasAnalysis to overcome the fact that inlining 
      // does not maintain the restrict information.
      Instruction* pLoadedValueInst = cast<Instruction>(pLoadedValue);
      if (pFunc->paramHasAttr(ArgNo+1, Attribute::NoAlias)) {
        pLoadedValueInst->setMetadata("restrict", llvm::MDNode::get(*m_pLLVMContext, 0)); 
      }

      params.push_back(pLoadedValue);
      
      // Advance the pArgsBuffer offset based on the size
      currOffset += TypeAlignment::getSize(arg);
      ++callIt;
    }
    
    // Handle implicit arguments
    for(unsigned int i=0; i< ImplicitArgsUtils::m_numberOfImplicitArgs; ++i) {
      const ImplicitArgProperties& implicitArgProp = ImplicitArgsUtils::getImplicitArgProps(i);

      size_t alignment = implicitArgProp.m_alignment;
      
      // assuming pBuffer is aligned at maximum alignment
      currOffset = TypeAlignment::align(alignment, currOffset);
      
       //  %0 = getelementptr i8* %pBuffer, i32 currOffset  
      Value* pGEP = builder.CreateGEP(pArgsBuffer, ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), currOffset));
      // %1 = bitcast i8* %0 to type *
      Value* pBitCast = builder.CreateBitCast(pGEP, PointerType::get(callIt->getType(), 0));
      //load type * %1
      LoadInst* pLoad = builder.CreateLoad(pBitCast);
      if (alignment > 0) {
        //load type * %1, align alignment
        pLoad->setAlignment(alignment);
      }

      params.push_back(pLoad);
      
      // Advance the pArgsBuffer offset based on the size
      currOffset += implicitArgProp.m_size;
      ++callIt;
    }
    
    return params;
  }
  
  void PrepareKernelArgs::createWrapperBody(Function* pWrapper, Function* pFunc) {
  // Set new function's argument name
    Function::arg_iterator DestI = pWrapper->arg_begin();
    DestI->setName("pBuffer");
    Argument *pArgsBuffer = DestI;
    
    // Create wrapper function
    BasicBlock* block = BasicBlock::Create(*m_pLLVMContext, "entry", pWrapper);
    IRBuilder<> builder(block);
    
    std::vector<Value*> params = createArgumentLoads(builder, pFunc, pArgsBuffer);
    
    CallInst* call = builder.CreateCall(pFunc, ArrayRef<Value*>(params));
    call->setCallingConv(pFunc->getCallingConv());
    
    builder.CreateRetVoid();
  }

  Function* PrepareKernelArgs::runOnFunction(Function *pFunc, bool isVectorized) {
  
    // Create wrapper function
    Function *pWrapper = createWrapper(pFunc);
    
    // Change name of old function
    pFunc->setName("__" + pFunc->getName() + "_separated_args");
    // Make sure old function always inlined
    // We want to do inlining pass after PrepareKernelArgs pass to gain performance
    pFunc->addFnAttr(llvm::Attribute::AlwaysInline);

    createWrapperBody(pWrapper, pFunc);

    // Add declaration of original function with its signature
    m_pModule->getFunctionList().push_back(pWrapper);

    // Map original function to new function
    m_oldToNewFunctionMap[pFunc] = pWrapper;
      
    if (!isVectorized) {
      NamedMDNode *WrapperMD = m_pModule->getNamedMetadata("opencl.wrappers");

      SmallVector<llvm::Value*, 5> Operands;

      for(int i = 0, e = WrapperMD->getNumOperands(); i < e ; i++) {
        Value *op = WrapperMD->getOperand(i)->getOperand(0);
        // We expect the metadata nodes to be llvm::Function
        // In case the cast is wrong an assertion failure will be thrown
        if(pFunc == cast<Function>(op))
          Operands.push_back(pWrapper);
        else
          Operands.push_back(WrapperMD->getOperand(i)->getOperand(0));
      }

      WrapperMD->eraseFromParent();

      WrapperMD = m_pModule->getOrInsertNamedMetadata("opencl.wrappers");

      for(int i = 0, e = Operands.size(); i < e ; i++) {
        WrapperMD->addOperand(llvm::MDNode::get(*m_pLLVMContext, Operands[i]));
      }
    }

   return pWrapper;
  }

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {
