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

#include "PrepareKernelArgs.h"

#include "TypeAlignment.h"
#include "CompilationUtils.h"
#include "ImplicitArgsUtils.h"
#include "MetadataAPI.h"
#include "OCLAddressSpace.h"
#include "OCLPassSupport.h"
#include "OclTune.h"

#include "llvm/IR/Attributes.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include <sstream>
#include <memory>

#define STACK_PADDING_BUFFER (DEV_MAXIMUM_ALIGN*1)

extern "C"{
  /// @brief Creates new PrepareKernelArgs module pass
  /// @returns new PrepareKernelArgs module pass
ModulePass *createPrepareKernelArgsPass(bool useTLSGlobals) {
  return new intel::PrepareKernelArgs(useTLSGlobals);
  }
}

extern cl::opt<bool> OptUseTLSGlobals;

using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::MetadataAPI;

namespace intel{

  char PrepareKernelArgs::ID = 0;

  /// Register pass to for opt
  OCL_INITIALIZE_PASS(PrepareKernelArgs, "prepare-kernel-args", "changes the way arguments are passed to kernels", false, false)

  PrepareKernelArgs::PrepareKernelArgs(bool useTLSGlobals)
      : ModulePass(ID), m_pModule(nullptr), m_DL(nullptr),
        m_pLLVMContext(nullptr), m_IAA(nullptr), m_PtrSizeInBytes(0),
        m_SizetTy(nullptr), m_I8Ty(nullptr), m_I32Ty(nullptr),
        m_useTLSGlobals(useTLSGlobals || OptUseTLSGlobals) {
    initializeImplicitArgsAnalysisPass(*llvm::PassRegistry::getPassRegistry());
  }

  bool PrepareKernelArgs::runOnModule(Module &M) {
    m_DL = &M.getDataLayout();
    m_pModule = &M;
    m_pLLVMContext = &M.getContext();
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
    for (auto *F : kernelsFunctionSet) {
      runOnFunction(F);
    }

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
    pNewF->copyMetadata(pFunc, 0);

    // pFunc is expected to be inlined anyway,
    // so no need to duplicate DISubprogram.
    pFunc->setSubprogram(nullptr);

    return pNewF;
  }

  std::vector<Value *> PrepareKernelArgs::createArgumentLoads(
      IRBuilder<> &builder, Function *WrappedKernel, Argument *pArgsBuffer,
      Argument *pArgGID, Argument *RuntimeContext) {
    // Get old function's arguments list in the OpenCL level from its metadata
    std::vector<cl_kernel_argument> arguments;
    std::vector<unsigned int>       memoryArguments;
    CompilationUtils::parseKernelArguments(
        m_pModule, WrappedKernel, m_useTLSGlobals, arguments, memoryArguments);

    auto kimd = KernelInternalMetadataAPI(WrappedKernel);
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
      } else if (arg.type == CL_KRNL_ARG_PTR_LOCAL) {
        // The argument is actually the size of the buffer
        Value *pPointerCast = builder.CreatePointerCast(pGEP, PointerType::get(m_SizetTy, 0));
        LoadInst *BufferSize = builder.CreateLoad(pPointerCast);
        // TODO: when buffer size is 0, we might want to set dummy address for debugging!
        const auto AllocaAddrSpace = m_DL->getAllocaAddrSpace();
        // We can't use overload without explicit alloca addrspace because the BB does
        // not have a parent yet.
        AllocaInst *Allocation = builder.CreateAlloca(m_I8Ty, AllocaAddrSpace, BufferSize);
        // Set alignment of buffer to type size.
        unsigned Alignment = 16; // Cacheline
        Type* EltTy = callIt->getType()->getPointerElementType();
        // If the kernel was vectorized, choose an alignment that is good for the *vectorized* type. This can
        // be good for unaligned loads on targets that support instructions such as MOVUPS
        unsigned VecSize = kimd.VectorizedWidth.hasValue() ? kimd.VectorizedWidth.get() : 1;
        if (VecSize != 1 && VectorType::isValidElementType(EltTy))
          EltTy = VectorType::get(EltTy, VecSize);
        Alignment = llvm::NextPowerOf2(m_DL->getTypeAllocSize(EltTy) - 1);
        Allocation->setAlignment(MaybeAlign(Alignment));
        pArg = builder.CreatePointerCast(Allocation, callIt->getType());
      } else if (arg.type == CL_KRNL_ARG_PTR_BLOCK_LITERAL) {
          pArg = builder.CreateAddrSpaceCast(pGEP, callIt->getType());
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
          pLoad->setAlignment(MaybeAlign(TypeAlignment::getAlignment(arg)));
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
          currOffset, m_DL->getPointerABIAlignment(m_DL->getAllocaAddrSpace()).value());
    }
    // Handle implicit arguments
    // Set to the Work Group Info implicit arg, as soon as it is known. Used for
    // computing other arg values
    Value* WGInfo = 0;
    // LocalSize for each dimension. Used several times below.
    SmallVector<Value*, 4> LocalSize;
    ImplicitArgsUtils::initImplicitArgProps(m_PtrSizeInBytes);
    for(unsigned int i=0; i< ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS; ++i) {
      Value* pArg = nullptr;
      if (!m_useTLSGlobals)
        assert(callIt->getType() == m_IAA->getArgType(i) &&
               "Mismatch in arg found in function and expected arg type");
      switch(i) {
      case ImplicitArgsUtils::IA_SLM_BUFFER: {
          uint64_t slmSizeInBytes = kimd.LocalBufferSize.get();
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
          const auto AllocaAddrSpace = m_DL->getAllocaAddrSpace();
          AllocaInst *slmBuffer = builder.CreateAlloca(slmType, AllocaAddrSpace);
          // Set alignment of implicit local buffer to max alignment.
          // TODO: we should choose the min required alignment size
          slmBuffer->setAlignment(MaybeAlign(TypeAlignment::MAX_ALIGNMENT));
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
        assert(m_IAA->getArgType(i) == pArgGID->getType() &&
               "Unmatching types");
        pArg = pArgGID;
        break;
      case ImplicitArgsUtils::IA_RUNTIME_HANDLE:
        // Runtime Context is passed by value as an argument to the wrapper
        assert(m_IAA->getArgType(i) == RuntimeContext->getType() &&
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
      case ImplicitArgsUtils::IA_BARRIER_BUFFER:
        {
          // We obtain the number of bytes needed per item from the Metadata
          // which is set by the Barrier pass
          uint64_t SizeInBytes = kimd.BarrierBufferSize.get();
          // BarrierBufferSize := BytesNeededPerWI*GroupSize(0)*GroupSize(1)*GroupSize(2)
          Value *BarrierBufferSize = ConstantInt::get(m_SizetTy, SizeInBytes);
          assert(WGInfo && "Work Group Info was not initialized");
          assert(!LocalSize.empty() && "Local group sizes are assumed to be computed already");
          for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim)
            BarrierBufferSize = builder.CreateMul(BarrierBufferSize, LocalSize[Dim]);
          BarrierBufferSize->setName("BarrierBufferSize");
          // alloca i8, %BarrierBufferSize
          const auto AllocaAddrSpace = m_DL->getAllocaAddrSpace();
          AllocaInst *BarrierBuffer = builder.CreateAlloca(
            m_I8Ty, AllocaAddrSpace, BarrierBufferSize);
          //TODO: we should choose the min required alignment size
          BarrierBuffer->setAlignment(MaybeAlign(TypeAlignment::MAX_ALIGNMENT));
          pArg = BarrierBuffer;
        }
        break;
      case ImplicitArgsUtils::IA_WORK_GROUP_INFO: {
        // These values are pointers that just need to be loaded from the
        // UniformKernelArgs structure and passed on to the kernel
        const ImplicitArgProperties &implicitArgProp =
            ImplicitArgsUtils::getImplicitArgProps(i);
        // %0 = getelementptr i8* %pBuffer, i32 currOffset
        Value *pGEP = builder.CreateGEP(pArgsBuffer,
                                        ConstantInt::get(m_I32Ty, currOffset));
        pArg = builder.CreatePointerCast(pGEP, m_IAA->getArgType(i));
        WGInfo = pArg;
        // Advance the pArgsBuffer offset based on the size
        currOffset += implicitArgProp.m_size;
      } break;
      default:
        assert(false && "Unknown implicit argument");
      }

      if (m_useTLSGlobals) {
        assert(pArg && "No value was created for this TLS global!");
        GlobalVariable *GV = CompilationUtils::getTLSGlobal(m_pModule, i);
        builder.CreateStore(pArg, GV);
      } else {
        assert(pArg && "No value was created for this implicit argument!");
        pArg->setName(ImplicitArgsUtils::getArgName(i));
        params.push_back(pArg);
        ++callIt;
      }
    }

    return params;
  }

  void PrepareKernelArgs::createWrapperBody(Function* pWrapper, Function* WrappedKernel) {
    // Set new function's argument name
    Function::arg_iterator DestI = pWrapper->arg_begin();
    DestI->setName("pUniformArgs");
    DestI->addAttr(Attribute::NoAlias);
    Argument *pArgsBuffer = &*(DestI++);
    DestI->setName("pWGId");
    DestI->addAttr(Attribute::NoAlias);
    Argument *pArgGID = &*(DestI++);
    DestI->setName("RuntimeHandle");
    DestI->addAttr(Attribute::NoAlias);
    Argument *RuntimeContext = &*(DestI++);
    assert(DestI == pWrapper->arg_end() && "Expected to be past last arg");

    // Create wrapper function
    BasicBlock* block = BasicBlock::Create(*m_pLLVMContext, "wrapper_entry", pWrapper);
    IRBuilder<> builder(block);

    std::vector<Value*> params = createArgumentLoads(builder, WrappedKernel, pArgsBuffer, pArgGID, RuntimeContext);

    CallInst* call = builder.CreateCall(WrappedKernel, ArrayRef<Value*>(params));
    // inlinable function call in a function with debug info
    // must have a !dbg location
    if(DISubprogram *SP = WrappedKernel->getSubprogram())
        call->setDebugLoc(DebugLoc::get(SP->getScopeLine(), 0, SP));
    call->setCallingConv(WrappedKernel->getCallingConv());

    // Preserve debug info for a kernel return instruction
    auto WrapperRet = builder.CreateRetVoid();
    for (auto &BB : *WrappedKernel) {
      auto *Term = BB.getTerminator();
      assert(Term && "Ill-formed BasicBlock");
      if (!isa<ReturnInst>(Term))
        continue;

      WrapperRet->setDebugLoc(Term->getDebugLoc());
      break;
    }
  }

  void PrepareKernelArgs::replaceFunctionPointers(Function* Wrapper,
                                                  Function* WrappedKernel) {
  // BIs like enqueue_kernel and kernel query have a function pointer to a
  // block invoke kernel as an argument.
  // Replace these arguments by a pointer to the wrapper function.
    for (auto &EEF : *m_pModule) {
      if (!EEF.isDeclaration())
        continue;

      StringRef EEFName = EEF.getName();
      if (!(EEFName.startswith("ocl20_enqueue_kernel_") ||
            EEFName.equals("ocl20_get_kernel_wg_size") ||
            EEFName.equals("ocl20_get_kernel_preferred_wg_size_multiple")))
        continue;

      unsigned BlockInvokeIdx = (EEFName.startswith("ocl20_enqueue_kernel_"))
            ? (EEFName.contains("_events") ? 6 : 3)
            : 0;

      for (auto *U : EEF.users()) {
        if (auto *EECall = dyn_cast<CallInst>(U)) {
          Value *BlockInvoke =
            EECall->getArgOperand(BlockInvokeIdx)->stripPointerCasts();
          if (BlockInvoke != WrappedKernel)
            continue;
          auto *Int8PtrTy = PointerType::get(
            IntegerType::getInt8Ty(m_pModule->getContext()),
            Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Generic);

          auto *NewCast =
            CastInst::CreatePointerCast(Wrapper, Int8PtrTy, "", EECall);
          EECall->getArgOperand(BlockInvokeIdx)->replaceAllUsesWith(NewCast);
        }
      }
    }

  }

  bool PrepareKernelArgs::runOnFunction(Function *pFunc) {
    using namespace Intel::MetadataAPI;

    // Create wrapper function
    Function *pWrapper = createWrapper(pFunc);

    // Change name of old function
    pFunc->setName("__" + pFunc->getName() + "_separated_args");

    // Make sure old function always inlined
    // We want to do inlining pass after PrepareKernelArgs pass to gain performance
    pFunc->removeFnAttr(llvm::Attribute::NoInline);
    pFunc->addFnAttr(llvm::Attribute::AlwaysInline);

    createWrapperBody(pWrapper, pFunc);

    // Add declaration of original function with its signature
    m_pModule->getFunctionList().push_back(pWrapper);

    // Replace function pointers to the original function (occures in case of
    // a call of a device execution built-in) by ones to the wrapper
    replaceFunctionPointers(pWrapper, pFunc);

    auto kernelWrapperMetadata = KernelInternalMetadataAPI(pFunc).KernelWrapper;
    kernelWrapperMetadata.set(pWrapper);

    // move stats from original kernel to the wrapper
    intel::Statistic::moveFunctionStats(*pFunc, *pWrapper);

    return true;
  }

} // namespace intel
