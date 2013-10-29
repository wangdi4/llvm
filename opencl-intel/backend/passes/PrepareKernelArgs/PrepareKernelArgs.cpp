/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "PrepareKernelArgs.h"
#include "TypeAlignment.h"
#include "CompilationUtils.h"
#include "ImplicitArgsUtils.h"
#include "MetaDataApi.h"
#include "OCLPassSupport.h"
#include "llvm/Attributes.h"
#include "llvm/Support/ValueHandle.h"
#include "llvm/Version.h"
#include "llvm/ADT/SetVector.h"

#include <algorithm>
#include <sstream>
#include <memory>

extern "C"{
  /// @brief Creates new PrepareKernelArgs module pass
  /// @returns new PrepareKernelArgs module pass
  ModulePass* createPrepareKernelArgsPass() {
    return new intel::PrepareKernelArgs();
  }

}
using namespace Intel::OpenCL::DeviceBackend;

namespace intel{

  char PrepareKernelArgs::ID = 0;

  /// Register pass to for opt
  OCL_INITIALIZE_PASS(PrepareKernelArgs, "prepare-kernel-args", "changes the way arguments are passed to kernels", false, false)

  PrepareKernelArgs::PrepareKernelArgs() : ModulePass(ID) {
  }

  bool PrepareKernelArgs::runOnModule(Module &M) {
    m_DL = getAnalysisIfAvailable<DataLayout>();
    m_pModule = &M;
    m_pLLVMContext = &M.getContext();
    Intel::MetaDataUtils mdUtils(&M);
    m_mdUtils = &mdUtils;
    m_PtrSizeInBytes = M.getPointerSize() * 4;
    m_SizetTy = IntegerType::get(*m_pLLVMContext, m_PtrSizeInBytes*8);
    m_I32Ty = Type::getInt32Ty(*m_pLLVMContext);
    m_I8Ty = Type::getInt8Ty(*m_pLLVMContext);
    // Get all kernels (original scalar kernels and vectorized kernels)
    CompilationUtils::FunctionSet kernelsFunctionSet;
    CompilationUtils::getAllKernels(kernelsFunctionSet, m_pModule);

    // Run on all kernels for handling and handle them
    for ( CompilationUtils::FunctionSet::iterator fi = kernelsFunctionSet.begin(),
      fe = kernelsFunctionSet.end(); fi != fe; ++fi ) {
        runOnFunction(*fi);
    }
    //Save Metadata to the module
    m_mdUtils->save(*m_pLLVMContext);
    return true;
  }
  
  Function* PrepareKernelArgs::createWrapper(Function* pFunc) {
    // Create new function's argument type list
    // The new function receives one argument: i8* pBuffer
    std::vector<llvm::Type *> newArgsVec;
    newArgsVec.push_back(PointerType::get(m_I8Ty, 0));

    // Create new functions return type
    FunctionType *FTy = FunctionType::get( pFunc->getReturnType(), newArgsVec, false);

    // Create a new function
    Function *pNewF = Function::Create(FTy, pFunc->getLinkage(), pFunc->getName());
    pNewF->setCallingConv(pFunc->getCallingConv());
    
    return pNewF;
  }
  
  std::vector<Value*> PrepareKernelArgs::createArgumentLoads(IRBuilder<>& builder, Function* pFunc, Argument *pArgsBuffer) {

    // Get old function's arguments list in the OpenCL level from its metadata
    std::vector<cl_kernel_argument> arguments;
    std::vector<unsigned int>       memoryArguments;
    CompilationUtils::parseKernelArguments(m_pModule, pFunc, arguments, memoryArguments);
    
    Intel::KernelInfoMetaDataHandle kimd = m_mdUtils->getKernelsInfoItem(pFunc);
    assert(kimd.get() && "Function info should be available at this point");
    std::vector<Value*> params;
    // assuming pBuffer is initially aligned to TypeAlignment::MAX_ALIGNMENT and 
    // therefore currOffset = 0
    // TODO : Can we check this assumption here?
    size_t currOffset = 0;    
    
    llvm::Function::arg_iterator callIt = pFunc->arg_begin();
    
    // TODO :  get common code from the following 2 for loops into a function

    // Handle explicit arguments
    for (unsigned ArgNo = 0; ArgNo < arguments.size(); ++ArgNo) {
        
      cl_kernel_argument arg = arguments[ArgNo];
      
      // Align the current pArgsBuffer offset based on the alignment of the argument we are about to load
      currOffset = TypeAlignment::align(TypeAlignment::getAlignment(arg), currOffset);
      
      //  %0 = getelementptr i8* %pBuffer, i32 currOffset
      Value* pGEP = builder.CreateGEP(pArgsBuffer, ConstantInt::get(m_I32Ty, currOffset));
      
      Value* pArg;
      
      if (arg.type == CL_KRNL_ARG_COMPOSITE || arg.type == CL_KRNL_ARG_VECTOR_BY_REF) {
        // If this is a struct argument, then the struct itself is passed by value inside pArgsBuffer
        // and the original kernel signature was:
        // foo(..., MyStruct* byval myStruct, ...)
        // meaning pGEP already points to the structure and we do not need to load it
        // we just need to have a bitcast from i8* to MyStruct* and pass the pointer (!!!) to foo
        
        // %myStruct = bitcast i8* to MyStruct*
        // foo(..., %myStruct, ...)

        Value* pBitCast = builder.CreateBitCast(pGEP, callIt->getType());
        pArg = pBitCast;
        //TODO: Remove this #ifndef when apple pass local memory buffer size instead of pointer to buffer
#ifndef __APPLE__
      } else if (arg.type == CL_KRNL_ARG_PTR_LOCAL) {
        // The argument is actually the size of the buffer
        Value *pBitCast = builder.CreateBitCast(pGEP, PointerType::get(m_SizetTy, 0));
        LoadInst *BufferSize = builder.CreateLoad(pBitCast);
        // TODO: when buffer size is 0, we might want to set dummy address for debugging!
        AllocaInst *Allocation = builder.CreateAlloca(m_I8Ty, BufferSize);
        // Set alignment of buffer to type size.
        unsigned Alignment = 16; // Cacheline
        if (m_DL) {
          Type* EltTy = callIt->getType()->getPointerElementType();
          // If the kernel was vectorized, choose an alignment that is good for the *vectorized* type. This can
          // be good for unaligned loads on targets that support instructions such as MOVUPS
          unsigned VecSize = kimd->isVectorizedWidthHasValue() ? kimd->getVectorizedWidth() : 1;
          if (VecSize != 1 && VectorType::isValidElementType(EltTy))
            EltTy = VectorType::get(EltTy, VecSize);
          Alignment = llvm::NextPowerOf2(m_DL->getTypeAllocSize(EltTy) - 1);
        }
        Allocation->setAlignment(Alignment);
        pArg = builder.CreateBitCast(Allocation, callIt->getType());
#endif
      } else {
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
        pArg = pLoad;
      }
     
      // Here we mark the load instructions from the struct that are the actual parameters for 
      // the original kernel's restricted formal parameters  
      // This info is used later on in OpenCLAliasAnalysis to overcome the fact that inlining 
      // does not maintain the restrict information.
      Instruction* pArgInst = cast<Instruction>(pArg);
#if LLVM_VERSION == 3200
      if (pFunc->getParamAttributes(ArgNo + 1).hasAttribute(Attributes::NoAlias)) {
#elif LLVM_VERSION == 3425
      if (pFunc->paramHasAttr(ArgNo + 1, Attribute::NoAlias)) {
#else
      if (pFunc->getAttributes().hasAttribute(ArgNo + 1, Attribute::NoAlias)) {
#endif
        pArgInst->setMetadata("restrict", llvm::MDNode::get(*m_pLLVMContext, 0)); 
      }

      params.push_back(pArg);
      
      // Advance the pArgsBuffer offset based on the size
      currOffset += TypeAlignment::getSize(arg);
      ++callIt;
    }
    
    // Handle implicit arguments
    // Set to the Work Group Info implicit arg, as soon as it is known. Used for
    // computing other arg values
    Value *WorkInfo = 0;
    ImplicitArgsUtils::initImplicitArgProps(m_PtrSizeInBytes);
    for(unsigned int i=0; i< ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS; ++i) {
      Value* pArg = NULL;
      switch(i) {
      case ImplicitArgsUtils::IA_SLM_BUFFER:
        {
          uint64_t slmSizeInBytes = kimd->getLocalBufferSize();
          //TODO: when slmSizeInBytes equal 0, we might want to set dummy address for debugging!
          Type *slmType = ArrayType::get(m_I8Ty, slmSizeInBytes);
          AllocaInst *slmBuffer = builder.CreateAlloca(slmType);
          //Set alignment of implicit local buffer to max alignment.
          //TODO: we should choose the min required alignment size
          slmBuffer->setAlignment(TypeAlignment::MAX_ALIGNMENT);
          pArg = builder.CreateBitCast(slmBuffer, PointerType::get(m_I8Ty, 3));
        }
        break;
        //TODO: Remove this #ifndef when apple no longer pass barrier memory buffer
#ifndef __APPLE__
      case ImplicitArgsUtils::IA_BARRIER_BUFFER: 
        {
          // We obtain the number of bytes needed per item from the Metadata
          // which is set by the Barrier pass
          uint64_t SizeInBytes = kimd->getBarrierBufferSize();
          // BarrierBufferSize := BytesNeededPerWI*GroupSize(0)*GroupSize(1)*GroupSize(2)
          // GroupSize(i) is loaded directly from the WGInfo implicit argument,
          // which was already determined in an earlier iteration.
          Value *BarrierBufferSize = ConstantInt::get(m_SizetTy, SizeInBytes);
          assert(WorkInfo && "Work Group Info was not initialized");
          SmallVector<Value *, 4> params(3);
          params[0] = ConstantInt::get(m_I32Ty, 0);
          // Element with index=3 in struct is LocalSize
          // TODO: Make this code a library function
          params[1] = ConstantInt::get(m_I32Ty, 3);
          std::stringstream DimStr;
          for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
            params[2] = ConstantInt::get(m_I32Ty, Dim);
            DimStr.str("");
            DimStr << "LocalSize_dim" << Dim << "_";
            Value *pAddr = builder.CreateGEP(WorkInfo, params, "p" + DimStr.str());
            LoadInst *LocalSizeDim = builder.CreateLoad(pAddr, DimStr.str());
            BarrierBufferSize = builder.CreateMul(BarrierBufferSize, LocalSizeDim);
          }
          BarrierBufferSize->setName("BarrierBufferSize");
          // alloca i8, %BarrierBufferSize
          AllocaInst *BarrierBuffer = builder.CreateAlloca(m_I8Ty, BarrierBufferSize, "BarrierBuffer");
          //TODO: we should choose the min required alignment size
          BarrierBuffer->setAlignment(TypeAlignment::MAX_ALIGNMENT);
          pArg = BarrierBuffer;
        }
        break;
#endif
      case ImplicitArgsUtils::IA_CURRENT_WORK_ITEM:
        pArg = builder.CreateAlloca(m_SizetTy);
        break;
      default:
        {
          const ImplicitArgProperties& implicitArgProp = ImplicitArgsUtils::getImplicitArgProps(i);
          size_t alignment = implicitArgProp.m_alignment;
          // assuming pBuffer is aligned at maximum alignment
          currOffset = TypeAlignment::align(alignment, currOffset);

          // %0 = getelementptr i8* %pBuffer, i32 currOffset  
          Value* pGEP = builder.CreateGEP(pArgsBuffer, ConstantInt::get(m_I32Ty, currOffset));
          // %1 = bitcast i8* %0 to type *
          Value* pBitCast = builder.CreateBitCast(pGEP, PointerType::get(callIt->getType(), 0));
          //load type * %1
          LoadInst* pLoad = builder.CreateLoad(pBitCast);
          if (alignment > 0) {
            //load type * %1, align alignment
            pLoad->setAlignment(alignment);
          }
          pArg = pLoad;
          // WorkInfo is used as base for retrieving local size which is used
          // above for computing size of the barrier buffer
          if (ImplicitArgsUtils::IA_WORK_GROUP_INFO == i)
            WorkInfo = pArg;

          // Advance the pArgsBuffer offset based on the size
          currOffset += implicitArgProp.m_size;
        }
        break;
      }

      assert(pArg && "No value was created for this implicit argument!");
      params.push_back(pArg);
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

  bool PrepareKernelArgs::runOnFunction(Function *pFunc) {
  
    // Create wrapper function
    Function *pWrapper = createWrapper(pFunc);
    
    // Change name of old function
    pFunc->setName("__" + pFunc->getName() + "_separated_args");
    // Make sure old function always inlined
    // We want to do inlining pass after PrepareKernelArgs pass to gain performance
#if LLVM_VERSION == 3200
    pFunc->addFnAttr(llvm::Attributes::AlwaysInline);
#else
    pFunc->addFnAttr(llvm::Attribute::AlwaysInline);
#endif

    createWrapperBody(pWrapper, pFunc);

    // Add declaration of original function with its signature
    m_pModule->getFunctionList().push_back(pWrapper);

    m_mdUtils->getOrInsertKernelsInfoItem(pFunc)->setKernelWrapper(pWrapper);

    return true;
  }

} // namespace intel
