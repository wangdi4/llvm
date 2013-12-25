/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#define DEBUG_TYPE "cloneblockinvokefunctokernel"

#include "BlockUtils.h"
#include "CloneBlockInvokeFuncToKernel.h"
#include "OCLPassSupport.h"
#include "MetaDataApi.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Metadata.h"
#include "llvm/IRBuilder.h"
#include "llvm/Attributes.h"
#include "llvm/Version.h"

#include <assert.h>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

extern "C" {
  llvm::ModulePass* createCloneBlockInvokeFuncToKernelPass() {
      return new intel::CloneBlockInvokeFuncToKernel();
  }
}


// Create function - OpenCL kernel which calls block_invoke function
static Function *createKernelCallingBlock(Function *pBlock,
  llvm::LLVMContext &Context)
{
  assert(pBlock->arg_size() > 0 && "No arguments to block function");

  // create types for kernel
  // copy them from block_invoke function arguments
  std::vector<Type*> ArgTypes;
  for (Function::const_arg_iterator I = pBlock->arg_begin(), E = pBlock->arg_end();
    I != E; ++I)
      ArgTypes.push_back(I->getType());

  // Create a new function type...
  FunctionType *FTy = FunctionType::get(pBlock->getFunctionType()->getReturnType(),
    ArgTypes, false);

  // Create the new function...
  Function *NewF = Function::Create(FTy, GlobalValue::ExternalLinkage, "");

  // Create wrapper function
  BasicBlock* BB = BasicBlock::Create(Context, "entry", NewF);
  IRBuilder<> builder(BB);

  // CallInst fill in arguments
  std::vector<Value*> params;
  for (Function::arg_iterator I = NewF->arg_begin(),
                              E = NewF->arg_end(),
                              DestI = pBlock->arg_begin();
                              I != E;
                              ++I, ++DestI) {
      I->setName(DestI->getName());
      params.push_back(I);
  }
  // create CallInst to block
  CallInst* call = builder.CreateCall(pBlock, ArrayRef<Value*>(params));
  call->setCallingConv(pBlock->getCallingConv());
  builder.CreateRetVoid();

  return NewF;
}


namespace intel {

char CloneBlockInvokeFuncToKernel::ID = 0;
OCL_INITIALIZE_PASS(CloneBlockInvokeFuncToKernel, "cloneblockinvokefunctokernel",
                "Clones Block Invoke Functions to OpenCL kernels",
                false,
                false
                )


bool CloneBlockInvokeFuncToKernel::runOnModule(Module &M)
{
  m_pModule = &M;
  m_pContext = &M.getContext();
  m_pTD = getAnalysisIfAvailable<DataLayout>();
  if(!m_pTD)
    return false;
  Intel::MetaDataUtils MDU(&M);
  bool Changed = false;

  SmallVector<Function*,16> blockInvokeFuncs;

  // loop over functions in module
  for (Module::iterator F = M.begin(); F != M.end(); ++F) {
    // if F is block invoke function
    // AND returns void
    // then it may be enqueued in enqueue_kernel() BIs as kernel
    // and we need to create kernel from the block invoke function
    if(BlockUtils::isBlockInvokeFunction(*F) &&
       F->getReturnType()->isVoidTy())
      blockInvokeFuncs.push_back(F);
  }

  // obtain node with kernels
  NamedMDNode *OpenCLKernelMetadata = M.getNamedMetadata("opencl.kernels");
  if( NULL == OpenCLKernelMetadata )
  {
      // workaround to overcome klockwork issue
      return false;
  }

  // Create context and IRBuilder
  llvm::LLVMContext &Context = M.getContext();
  llvm::IRBuilder<> Builder(Context);

  // interate over block invoke functions and create kernels from them
  for(SmallVector<Function*,16>::const_iterator I = blockInvokeFuncs.begin(),
     E = blockInvokeFuncs.end(); I != E; ++I) {

    // hack: make block function not "internal"
    // this enables to see block function in global symbol table
    // we use for resolving call address
    (*I)->setLinkage(GlobalValue::ExternalLinkage);
    // copy function to new
    ValueToValueMapTy VMap;
    Function *NewFn = llvm::CloneFunction(*I, VMap,
                                              /*ModuleLevelChanges=*/false);
    assert(NewFn && "CloneFunction returned NULL");
    
    // obtain name for kernel
    std::string newName = BlockUtils::CreateBlockInvokeKernelName((*I)->getName());
    // set name of kernel
    NewFn->setName(newName);
    // Make sure function is always inlined
#if LLVM_VERSION == 3200
    NewFn->addFnAttr(llvm::Attributes::AlwaysInline);
#else
    NewFn->addFnAttr(llvm::Attribute::AlwaysInline);
#endif

    // add kernel func to module
    M.getFunctionList().push_back(NewFn);

    // compute block_literal size
    size_t blockLiteralSize = computeBlockLiteralSize(NewFn);
    // Set in metadata so it can be used later when preparing calls to enqueue_kernel_*
    MDU.getOrInsertKernelsInfoItem(NewFn)->setBlockLiteralSize(blockLiteralSize);
    
    // splitBlockInvokeArgument(NewFn);

    //
    // Updating of metadata with new kernel
    //
    // Derived from llvm/tools/clang/lib/CodeGen/CodeGenFunction::EmitOpenCLKernelMetadata()
    // Example of metadata for kernel:
    // !opencl.kernels = !{!0}
    // !0 = metadata !{void (i32 addrspace(1)*)* @enqueue_simple_block, metadata !1}
    // !1 = metadata !{metadata !"argument_attribute", i32 0}

    llvm::SmallVector <llvm::Value*, 2> kernelMDArgs;
    kernelMDArgs.push_back(NewFn);

    llvm::SmallVector<llvm::Value*, 5> kernelArgsAttr;
    kernelArgsAttr.push_back(llvm::MDString::get(Context, "argument_attribute"));

    Function::ArgumentListType::iterator argIt = NewFn->getArgumentList().begin();
    while ( argIt != NewFn->getArgumentList().end() ) {
          const int argAttr = 0;
          // !!! image types and sampler types are not handled properly
          // !!! they should not get here
          kernelArgsAttr.push_back(Builder.getInt32(argAttr)); // i32 0
          ++argIt;
    }

    kernelMDArgs.push_back(llvm::MDNode::get(Context, kernelArgsAttr));
    llvm::MDNode *kernelMDNode = llvm::MDNode::get(Context, kernelMDArgs);

    OpenCLKernelMetadata->addOperand(kernelMDNode);
    Changed = true;
  }
  if (Changed)
    MDU.save(M.getContext());

  return Changed;
}

// compute block_literal size
size_t CloneBlockInvokeFuncToKernel::computeBlockLiteralSize(Function *F)
{
  // get 1st and only argument
  Argument *blockLiteralPtr  = F->getArgumentList().begin();
  assert(blockLiteralPtr->getType()->isPointerTy());
  assert(blockLiteralPtr->hasOneUse() && "handle only one use of argument");
  
  BitCastInst *pBC = dyn_cast<BitCastInst>(*(blockLiteralPtr->use_begin()));
  assert( pBC && "use is not bitcast instruction!" );

  PointerType *pPTy = dyn_cast<PointerType>(pBC->getDestTy());
  assert( pPTy && "expected bitcast to pointer type");
  
  StructType * pStructBlockLiteralTy = dyn_cast<StructType>(pPTy->getPointerElementType());
  assert( pStructBlockLiteralTy && "expected bitcast to struct type");

  unsigned int const BLOCK_DESCRIPTOR_INDX = 4;
  PointerType *pBlockDescPtr = dyn_cast<PointerType>(pStructBlockLiteralTy->getElementType(BLOCK_DESCRIPTOR_INDX));
  assert( pBlockDescPtr && "expected pointer field");

  StructType * pBlockDescTy = dyn_cast<StructType>(pBlockDescPtr->getPointerElementType());
  assert( pBlockDescTy && "expected struct");

  // sum block_literal itself and block_descriptor size
  //return static_cast<size_t>(m_pTD->getStructLayout(pStructType)->getSizeInBytes() + 
//    m_pTD->getStructLayout(pBlockDescTy)->getSizeInBytes() );
  return static_cast<size_t>(m_pTD->getStructLayout(pStructBlockLiteralTy)->getSizeInBytes());
}

} // namespace intel
