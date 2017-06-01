/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#define DEBUG_TYPE "cloneblockinvokefunctokernel"

#include "BlockUtils.h"
#include "CloneBlockInvokeFuncToKernel.h"
#include "MetaDataApi.h"
#include "OCLPassSupport.h"

#include "llvm/IR/Metadata.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Attributes.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/Cloning.h"

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
      params.push_back(&*I);
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

static bool canBlockInvokeFunctionBeEnqueued(Function *F)
{
  if (!F->getReturnType()->isVoidTy())
    return false;

  // Check that the Function has at least one argument.
  if (F->arg_empty())
    return false;
  // First arg must be i8* %.block_descriptor
  // Check 1st arg is NOT struct return attribute
  // In this case block returns struct - not acceptable for enqueue
  Argument* Arg1= &*F->arg_begin();
  if (Arg1->hasStructRetAttr() || Arg1->getName() != ".block_descriptor")
    return false;

  return true;
}

bool CloneBlockInvokeFuncToKernel::runOnModule(Module &M)
{
  m_pModule = &M;
  m_pContext = &M.getContext();
  m_pTD = &M.getDataLayout();
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
       canBlockInvokeFunctionBeEnqueued(&*F))
      blockInvokeFuncs.push_back(&*F);
  }

  // obtain node with kernels
  NamedMDNode *OpenCLKernelMetadata = M.getNamedMetadata("opencl.kernels");
  assert(OpenCLKernelMetadata && "There is no \"opencl.kernels\" metadata.");
  // workaround to overcome klockwork issue
  if( !OpenCLKernelMetadata ) {
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
    Function *NewFn = llvm::CloneFunction(*I, VMap);
    assert(NewFn && "CloneFunction returned NULL");

    // obtain name for kernel
    std::string newName = BlockUtils::CreateBlockInvokeKernelName((*I)->getName());
    // set name of kernel
    NewFn->setName(newName);
    // Make sure function is always inlined
    NewFn->addFnAttr(llvm::Attribute::AlwaysInline);

    // add kernel func to module
    M.getFunctionList().push_back(NewFn);

    // compute block_literal size
    size_t blockLiteralSize = computeBlockLiteralSize(NewFn);
    // Set in metadata so it can be used later when preparing calls to enqueue_kernel_*
    MDU.getOrInsertKernelsInfoItem(NewFn)->setBlockLiteralSize(blockLiteralSize);

    //
    // Updating of metadata with new kernel
    //
    // TDerived from llvm/tools/clang/lib/CodeGen/CodeGenFunction::EmitOpenCLKernelMetadata()
    // Example of metadata for kernel:
    // !opencl.kernels = !{!0}
    // !0 = metadata !{void (i32 addrspace(1)*)* @enqueue_simple_block, metadata !1}
    // !1 = metadata !{metadata !"argument_attribute", i32 0}
    //
    // [LLVM 3.6 UPGRADE] Seems the comment above is not correct because SPIR 1.2
    // of "__kernel void test(__global float* arg)" looks like this:
    // !opencl.kernels = !{!0}
    // !0 = metadata !{void (float addrspace(1)*)* @test, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5, metadata !6}
    // !1 = metadata !{metadata !"kernel_arg_addr_space", i32 1}
    // !2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none"}
    // !3 = metadata !{metadata !"kernel_arg_type", metadata !"float*"}
    // !4 = metadata !{metadata !"kernel_arg_type_qual", metadata !""}
    // !5 = metadata !{metadata !"kernel_arg_base_type", metadata !"float*"}
    // !6 = metadata !{metadata !"kernel_arg_name", metadata !"arg"}
    //
    // FIXME: The right way is to use MetaDataUtils to handle this instead of the two lines below.
    OpenCLKernelMetadata->addOperand(
      llvm::MDNode::get(Context, llvm::ConstantAsMetadata::getConstant(NewFn)));
    // So I have no idea about the impact of the lecacy code below.

//    llvm::SmallVector <llvm::Value*, 2> kernelMDArgs;
//    kernelMDArgs.push_back(NewFn);
//
//    llvm::SmallVector<llvm::Value*, 5> kernelArgsAttr;
//    kernelArgsAttr.push_back(llvm::MDString::get(Context, "argument_attribute"));
//
//    Function::ArgumentListType::iterator argIt = NewFn->getArgumentList().begin();
//    while ( argIt != NewFn->getArgumentList().end() ) {
//          const int argAttr = 0;
//          // !!! image types and sampler types are not handled properly
//          // !!! they should not get here
//          kernelArgsAttr.push_back(Builder.getInt32(argAttr)); // i32 0
//          ++argIt;
//    }
//
//    kernelMDArgs.push_back(llvm::MDNode::get(Context, kernelArgsAttr));
//    llvm::MDNode *kernelMDNode = llvm::MDNode::get(Context, kernelMDArgs);
//
//    OpenCLKernelMetadata->addOperand(kernelMDNode);
    Changed = true;
  }
  if (Changed)
    MDU.save(M.getContext());

  return Changed;
}

static size_t getBlockLiteralDefaultSize() {
// http://clang.llvm.org/docs/Block-ABI-Apple.html
//  struct Block_literal_1 {
//    void *isa; // initialized to &_NSConcreteStackBlock or &_NSConcreteGlobalBlock
//    int flags;
//    int reserved;
//    void (*invoke)(void *, ...);
//    struct Block_descriptor_1 {
//    unsigned long int reserved;         // NULL
//        unsigned long int size;         // sizeof(struct Block_literal_1)
//        // optional helper functions
//        void (*copy_helper)(void *dst, void *src);     // IFF (1<<25)
//        void (*dispose_helper)(void *src);             // IFF (1<<25)
//        // required ABI.2010.3.16
//        const char *signature;                         // IFF (1<<30)
//    } *descriptor;
//    // imported variables
//};
  return sizeof(void*) + sizeof(int) + sizeof(int) + sizeof(void*) + sizeof(void*);
}
// compute block_literal size
size_t CloneBlockInvokeFuncToKernel::computeBlockLiteralSize(Function *F)
{
  // get 1st and only argument
  Function::arg_iterator ai = F->arg_begin();
  Argument* blockLiteralPtr  = &*ai;
  // workaround for blocks returning struct
  if ( blockLiteralPtr->hasStructRetAttr() ) {
      ++ai;
      blockLiteralPtr = &*ai;
  }

  // if no uses return default size
  if(!blockLiteralPtr->getNumUses())
    return getBlockLiteralDefaultSize();

  assert(blockLiteralPtr->getType()->isPointerTy());

  // search for specific bitcast
  // example bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, i64, i32 addrspace(1)*, i32 }>*
  for(Argument::user_iterator AI = blockLiteralPtr->user_begin(),
    E = blockLiteralPtr->user_end(); AI != E; ++AI){

    if(!(isa<BitCastInst>(*AI) || isa<AddrSpaceCastInst>(*AI)))
      continue;
    CastInst *pBC = cast<CastInst>(*AI);

    PointerType *pPTy = dyn_cast<PointerType>(pBC->getDestTy());
    if(!pPTy)
      continue;

    StructType * pStructBlockLiteralTy = dyn_cast<StructType>(pPTy->getPointerElementType());
    if(!pStructBlockLiteralTy)
      continue;

#ifndef NDEBUG
    unsigned int const BLOCK_DESCRIPTOR_INDX = 4;
    PointerType *pBlockDescPtr = dyn_cast<PointerType>(pStructBlockLiteralTy->getElementType(BLOCK_DESCRIPTOR_INDX));
    assert( pBlockDescPtr && "expected pointer field");

    StructType * pBlockDescTy = dyn_cast<StructType>(pBlockDescPtr->getPointerElementType());
    assert( pBlockDescTy && "expected struct");
#endif

    //block_literal itself
    return static_cast<size_t>(m_pTD->getStructLayout(pStructBlockLiteralTy)->getSizeInBytes());
  }

  assert(0 && "did not find bitcast to struct");
  return -1;
}

} // namespace intel
