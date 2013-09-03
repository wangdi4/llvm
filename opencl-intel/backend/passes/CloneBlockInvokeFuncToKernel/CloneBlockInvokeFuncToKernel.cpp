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

#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Metadata.h"
#include "llvm/IRBuilder.h"

#include <assert.h>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

extern "C" {
  llvm::ModulePass* createCloneBlockInvokeFuncToKernelPass() {
      return new intel::CloneBlockInvokeFuncToKernel();
  }
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
  SmallVector<Function*,16> blockInvokeFuncs;

  // loop over functions in module find block invoke and add to list
  for (Module::iterator F = M.begin(); F != M.end(); ++F) {
    if(BlockUtils::isBlockInvokeFunction(*F))
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
  for(SmallVector<Function*,16>::const_iterator EI = blockInvokeFuncs.begin(),
     EE = blockInvokeFuncs.end(); EI != EE; ++EI) {

    // hack: make block function not "internal"
    // this enables to see block function in global symbol table
    // we use for resolving call address
    (*EI)->setLinkage(GlobalValue::ExternalLinkage);
    // copy function to new
    ValueToValueMapTy VMap;
    Function *NewFn = llvm::CloneFunction(*EI, VMap,
                                              /*ModuleLevelChanges=*/false);
    assert(NewFn && "CloneFunction returned NULL");
    
    // obtain name for kernel
    std::string newName = BlockUtils::CreateBlockInvokeKernelName((*EI)->getName());
    // set name of kernel
    NewFn->setName(newName);
    // to drop internal linkage type in block_invoke
    NewFn->setLinkage(GlobalValue::ExternalLinkage);
    
    // TODO: Do we need to assign global address space to implicit argument 
    //  - block_literal pointer ?
    // 
    
    // add kernel func to module
    M.getFunctionList().push_back(NewFn);

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
  }
  return true;
}

} // namespace intel



