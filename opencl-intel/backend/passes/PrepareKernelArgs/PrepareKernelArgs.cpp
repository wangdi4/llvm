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
#include "OclTune.h"

#include "llvm/IR/Attributes.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/ADT/SetVector.h"

#include <sstream>
#include <memory>

#define STACK_PADDING_BUFFER DEV_MAXIMUM_ALIGN*1

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

    PrepareKernelArgs::PrepareKernelArgs() : ModulePass(ID), m_DL(nullptr) {
    initializeImplicitArgsAnalysisPass(*llvm::PassRegistry::getPassRegistry());
  }

  bool PrepareKernelArgs::runOnModule(Module &M) {
    m_DL = &M.getDataLayout();
    m_pModule = &M;
    m_pLLVMContext = &M.getContext();
    Intel::MetaDataUtils mdUtils(&M);
    m_mdUtils = &mdUtils;
    m_IAA = &getAnalysis<ImplicitArgsAnalysis>();
    m_PtrSizeInBytes = M.getDataLayout().getPointerSize(0);
    m_IAA->initDuringRun(m_PtrSizeInBytes * 8);
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
    std::vector<llvm::Type *> newArgsVec;
    // The new function receives the following arguments:
    // i8* pBuffer
    newArgsVec.push_back(PointerType::get(m_I8Ty, 0));
    // GID argument
    newArgsVec.push_back(m_IAA->getArgType(ImplicitArgsUtils::IA_WORK_GROUP_ID));
    // Runtime context
    newArgsVec.push_back(m_IAA->getArgType(ImplicitArgsUtils::IA_RUNTIME_HANDLE));
    // Create new functions return type
    FunctionType *FTy = FunctionType::get( pFunc->getReturnType(), newArgsVec, false);

    // Create a new function
    Function *pNewF = Function::Create(FTy, pFunc->getLinkage(), pFunc->getName());
    pNewF->setCallingConv(pFunc->getCallingConv());

    // Set DISubprogram as an original function has
    pNewF->setSubprogram(pFunc->getSubprogram());

    return pNewF;
  }

  std::vector<Value *> PrepareKernelArgs::createArgumentLoads(
      IRBuilder<> &builder, Function *WrappedKernel, Argument *pArgsBuffer,
      Argument *pArgGID, Argument *RuntimeContext) {

    // Get old function's arguments list in the OpenCL level from its metadata
    std::vector<cl_kernel_argument> arguments;
    std::vector<unsigned int>       memoryArguments;
    CompilationUtils::parseKernelArguments(m_pModule, WrappedKernel, arguments, memoryArguments);

    Intel::KernelInfoMetaDataHandle kimd = m_mdUtils->getKernelsInfoItem(WrappedKernel);
    assert(kimd.get() && "Function info should be available at this point");
    std::vector<Value*> params;
    llvm::Function::arg_iterator callIt = WrappedKernel->arg_begin();

    // TODO :  get common code from the following 2 for loops into a function

    // Handle explicit arguments
    for (unsigned ArgNo = 0; ArgNo < arguments.size(); ++ArgNo) {
      cl_kernel_argument arg = arguments[ArgNo];
      //  %0 = getelementptr i8* %pBuffer, i32 currOffset
      Value* pGEP = builder.CreateGEP(pArgsBuffer, ConstantInt::get(m_I32Ty, arg.offset_in_bytes));

      Value* pArg;

      if (arg.type == CL_KRNL_ARG_COMPOSITE || arg.type == CL_KRNL_ARG_VECTOR_BY_REF) {
        // If this is a struct argument, then the struct itself is passed by value inside pArgsBuffer
        // and the original kernel signature was:
        // foo(..., MyStruct* byval myStruct, ...)
        // meaning pGEP already points to the structure and we do not need to load it
        // we just need to have a bitcast from i8* to MyStruct* and pass the pointer (!!!) to foo

        // %myStruct = bitcast i8* to MyStruct*
        // foo(..., %myStruct, ...)

        Value* pPointerCast = builder.CreatePointerCast(pGEP, callIt->getType());
        pArg = pPointerCast;
        //TODO: Remove this #ifndef when apple pass local memory buffer size instead of pointer to buffer
#ifndef __APPLE__
      } else if (arg.type == CL_KRNL_ARG_PTR_LOCAL) {
        // The argument is actually the size of the buffer
        Value *pPointerCast = builder.CreatePointerCast(pGEP, PointerType::get(m_SizetTy, 0));
        LoadInst *BufferSize = builder.CreateLoad(pPointerCast);
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
        pArg = builder.CreatePointerCast(Allocation, callIt->getType());
#endif
      } else if (arg.type == CL_KRNL_ARG_PTR_BLOCK_LITERAL) {
          pArg = pGEP;
      } else {
        // Otherwise this is some other type, lets say int4, then int4 itself is passed by value inside pArgsBuffer
        // and the original kernel signature was:
        // foo(..., int4 vec, ...)
        // meaning pGEP points to int4 and we just need to have a bitcast from i8* to int4*
        // load the int4 and pass the loaded value (!!!) to foo

        // %pVec = bitcast i8* %0 to int4 *
        // %vec = load int4 * %pVec {, align <alignment> }
        // foo(..., vec, ...)

        Value* pPointerCast = builder.CreatePointerCast(pGEP, PointerType::get(callIt->getType(), 0));
        LoadInst* pLoad = builder.CreateLoad(pPointerCast);
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
      if (WrappedKernel->getAttributes().hasAttribute(ArgNo + 1, Attribute::NoAlias)) {
        pArgInst->setMetadata("restrict", llvm::MDNode::get(*m_pLLVMContext, 0));
      }

      //TODO: Maybe get arg name from metadata?
      std::stringstream Name;
      Name << "explicit_" << ArgNo;
      pArg->setName(Name.str());
      params.push_back(pArg);

      ++callIt;
    }

    // Offset to after last explicit argument + adjusted alignment
    // Believe it or not, the conformance has a test kernel with 0 args...
    size_t currOffset = 0;
    if (!arguments.empty()) {
      assert(m_DL && "m_DL is nullptr!");
      currOffset = arguments.back().offset_in_bytes +
                   TypeAlignment::getSize(arguments.back());
      currOffset = ImplicitArgsUtils::getAdjustedAlignment(
          currOffset, m_DL->getPointerABIAlignment());
    }
    // Handle implicit arguments
    // Set to the Work Group Info implicit arg, as soon as it is known. Used for
    // computing other arg values
    Value* WGInfo = 0;
    // LocalSize for each dimension. Used several times below.
    SmallVector<Value*, 4> LocalSize;
    ImplicitArgsUtils::initImplicitArgProps(m_PtrSizeInBytes);
    for(unsigned int i=0; i< ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS; ++i) {
      Value* pArg = NULL;
      assert(callIt->getType() == m_IAA->getArgType(i) &&
             "Mismatch in arg found in function and expected arg type");
      switch(i) {
      case ImplicitArgsUtils::IA_SLM_BUFFER: {
          uint64_t slmSizeInBytes = kimd->getLocalBufferSize();
          // TODO: when slmSizeInBytes equal 0, we might want to set dummy
          // address for debugging!
        if (slmSizeInBytes == 0) { // no need to create of pad this buffer.
          pArg = Constant::getNullValue(PointerType::get(m_I8Ty, 3));
        }
        else {
          // add stack padding before and after this alloca, to allow unmasked wide loads
          // inside the vectorizer.
          Type *slmType = ArrayType::get(m_I8Ty,
            slmSizeInBytes+STACK_PADDING_BUFFER*2);
          AllocaInst *slmBuffer = builder.CreateAlloca(slmType);
          // Set alignment of implicit local buffer to max alignment.
          // TODO: we should choose the min required alignment size
          slmBuffer->setAlignment(TypeAlignment::MAX_ALIGNMENT);
          // move argument up over the lower side padding.
          Value* castBuf = builder.CreatePointerCast(slmBuffer,
                                                     PointerType::get(m_I8Ty, 3));
          pArg = builder.CreateGEP(castBuf,
                                   ConstantInt::get(m_I32Ty, STACK_PADDING_BUFFER));
        }
      }
        break;
      case ImplicitArgsUtils::IA_WORK_GROUP_ID:
        // WGID is passed by value as an argument to the wrapper
        assert(callIt->getType() == pArgGID->getType() && "Unmatching types");
        pArg = pArgGID;
        break;
      case ImplicitArgsUtils::IA_RUNTIME_HANDLE:
        // Runtime Context is passed by value as an argument to the wrapper
        assert(callIt->getType() == RuntimeContext->getType() &&
               "Unmatching types");
        pArg = RuntimeContext;
        break;
      case ImplicitArgsUtils::IA_GLOBAL_BASE_ID: {
        assert(WGInfo && "WGInfo should have already been initialized");
        // Obtain values of Local Size for each dimension
        assert(LocalSize.empty() &&
               "Assuming that we are computing Local Sizes here");
        for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim)
          LocalSize.push_back(
              m_IAA->GenerateGetEnqueuedLocalSize(WGInfo, Dim, builder));
        // Obtain values of NDRange Offsets for each dimension
        SmallVector<Value *, 4> GlobalOffsets;
        for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
          GlobalOffsets.push_back(
              m_IAA->GenerateGetGlobalOffset(WGInfo, Dim, builder));
        }
        // Obtain values of group ID for each dimension
        SmallVector<Value *, 4> GroupIDs;
        for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim)
          GroupIDs.push_back(m_IAA->GenerateGetGroupID(pArgGID, Dim, builder));
        // Compute the required value:
        // GlobalBaseId[i] = GroupId[i]*LocalSize[i]+GlobalOffset[i]
        SmallVector<Value *, 4> Computes;
        for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
          Value *V = builder.CreateBinOp(Instruction::Mul, LocalSize[Dim],
                                         GroupIDs[Dim]);
          V = builder.CreateBinOp(Instruction::Add, V, GlobalOffsets[Dim]);
          Computes.push_back(V);
        }
        // Collect all values to single array
        Value *U = UndefValue::get(m_IAA->getArgType(i));
        for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
          U = builder.CreateInsertValue(U, Computes[Dim],
                                        ArrayRef<unsigned>(Dim));
        }
        pArg = U;
        } break;
        //TODO: Remove this #ifndef when apple no longer pass barrier memory buffer
#ifndef __APPLE__
      case ImplicitArgsUtils::IA_BARRIER_BUFFER:
        {
          // We obtain the number of bytes needed per item from the Metadata
          // which is set by the Barrier pass
          uint64_t SizeInBytes = kimd->getBarrierBufferSize();
          // BarrierBufferSize := BytesNeededPerWI*GroupSize(0)*GroupSize(1)*GroupSize(2)
          Value *BarrierBufferSize = ConstantInt::get(m_SizetTy, SizeInBytes);
          assert(WGInfo && "Work Group Info was not initialized");
          assert(!LocalSize.empty() && "Local group sizes are assumed to be computed already");
          for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim)
            BarrierBufferSize = builder.CreateMul(BarrierBufferSize, LocalSize[Dim]);
          BarrierBufferSize->setName("BarrierBufferSize");
          // alloca i8, %BarrierBufferSize
          AllocaInst *BarrierBuffer = builder.CreateAlloca(m_I8Ty, BarrierBufferSize);
          //TODO: we should choose the min required alignment size
          BarrierBuffer->setAlignment(TypeAlignment::MAX_ALIGNMENT);
          pArg = BarrierBuffer;
        }
        break;
#endif
      case ImplicitArgsUtils::IA_WORK_GROUP_INFO: {
        // These values are pointers that just need to be loaded from the
        // UniformKernelArgs structure and passed on to the kernel
        const ImplicitArgProperties &implicitArgProp =
            ImplicitArgsUtils::getImplicitArgProps(i);
        // %0 = getelementptr i8* %pBuffer, i32 currOffset
        Value *pGEP = builder.CreateGEP(pArgsBuffer,
                                        ConstantInt::get(m_I32Ty, currOffset));
        pArg = builder.CreatePointerCast(pGEP, callIt->getType());
        WGInfo = pArg;
        // Advance the pArgsBuffer offset based on the size
        currOffset += implicitArgProp.m_size;
      } break;
      default:
        assert(false && "Unknown implicit argument");
      }

      assert(pArg && "No value was created for this implicit argument!");
      pArg->setName(ImplicitArgsUtils::getArgName(i));
      params.push_back(pArg);
      ++callIt;
    }

    return params;
  }

  void PrepareKernelArgs::createWrapperBody(Function* pWrapper, Function* WrappedKernel) {
    // Set new function's argument name
    AttributeSet NoAlias = AttributeSet::get(*m_pLLVMContext, 0, Attribute::NoAlias);
    Function::arg_iterator DestI = pWrapper->arg_begin();
    DestI->setName("pUniformArgs");
    DestI->addAttr(NoAlias);
    Argument *pArgsBuffer = &*(DestI++);
    DestI->setName("pWGID");
    DestI->addAttr(NoAlias);
    Argument *pArgGID = &*(DestI++);
    DestI->setName("RuntimeHandle");
    DestI->addAttr(NoAlias);
    Argument *RuntimeContext = &*(DestI++);
    assert(DestI == pWrapper->arg_end() && "Expected to be past last arg");

    // Create wrapper function
    BasicBlock* block = BasicBlock::Create(*m_pLLVMContext, "wrapper_entry", pWrapper);
    IRBuilder<> builder(block);

    std::vector<Value*> params = createArgumentLoads(builder, WrappedKernel, pArgsBuffer, pArgGID, RuntimeContext);

    CallInst* call = builder.CreateCall(WrappedKernel, ArrayRef<Value*>(params));
    call->setCallingConv(WrappedKernel->getCallingConv());

    builder.CreateRetVoid();
  }

  bool PrepareKernelArgs::runOnFunction(Function *pFunc) {

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

    m_mdUtils->getOrInsertKernelsInfoItem(pFunc)->setKernelWrapper(pWrapper);

    // move stats from original kernel to the wrapper
    intel::Statistic::moveFunctionStats(*pFunc, *pWrapper);

    return true;
  }

} // namespace intel
