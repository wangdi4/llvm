// Copyright (c) 2015 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#define DEBUG_TYPE "SPIR20BlocksToObjCBlocks"

#include <llvm/ADT/Twine.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Module.h>
#include <llvm/PassManager.h>

#include <stdint.h>

#include <OCLPassSupport.h>
#include <OCLAddressSpace.h>

#include "SPIR20BlocksToObjCBlocks.h"

using namespace llvm;
using namespace intel;
using namespace Intel::OpenCL::DeviceBackend::Utils;

extern "C" {
  ModulePass *createSPIR20BlocksToObjCBlocks() {
    return new intel::SPIR20BlocksToObjCBlocks();
  }
}

namespace intel {
  namespace {
    char const * spirBlockBindFuncName = "spir_block_bind";
  }

  // Register pass to for opt
  OCL_INITIALIZE_PASS(SPIR20BlocksToObjCBlocks, "spir20-to-objc-blocks",
                      "Convert SPIR 2.0 blocks to Objective-C blocks", false, false)

  char SPIR20BlocksToObjCBlocks::ID = 0;

  bool SPIR20BlocksToObjCBlocks::runOnModule(Module &M) {
    Function * spirBlockBindFunc = M.getFunction(spirBlockBindFuncName);
    // Check if the module has spir_block_bind function.
    if(!spirBlockBindFunc) return false;

    initPass(M);

    // Go over call sites of spir_block_bind and replace it with allocation and
    // initialization  of Objective-C blocks which are to be passes to device execution
    // built-ins afterwards.
    // All blocks' invoke functions must be patched accordingly to new blocks layout
    // and captured context.
    SmallVector<Instruction *, 8> toErase;
    for(User * user: spirBlockBindFunc->users()) {
      CallInst * spirBlockBindCI = dyn_cast<CallInst>(user);
      if(!spirBlockBindCI) continue;
      // Create and initialize Objective-C block.
      Value * objcBlock = getOrCreateObjCBlock(M, spirBlockBindCI);
      spirBlockBindCI->replaceAllUsesWith(objcBlock);
      toErase.push_back(spirBlockBindCI);
    }
    // Erase SPIR's remnants
    for(Instruction * inst : toErase)
      inst->eraseFromParent();
    spirBlockBindFunc->eraseFromParent();

    return true;
  }

  // *****************************************************************
  // Create Objective-C block descriptor type and initialize the pass.
  // *****************************************************************
  void SPIR20BlocksToObjCBlocks::initPass(Module &M) {
    m_objcBlocks.clear();

    // Create block descriptor type which contains block size in the last element.
    // See the examples below.
    // %struct.__block_descriptor = type { i64, i64 }
    // @__block_descriptor_tmp = internal constant { i64, i64 } { i64 0, i64 40 }
    LLVMContext & ctx = M.getContext();
    m_int64Ty = Type::getInt64Ty(ctx);
    m_int32Ty = Type::getInt32Ty(ctx);
    Type * int8PtrTy = Type::getInt8Ty(ctx)->getPointerTo(OCLAddressSpace::Private);

    // Create Objective-C block desriptor w\o elements what aren't requred by CPU BE
    Type * objcBlockDescrElements [2] = { m_int64Ty,   // reserved
                                          m_int64Ty }; // Block_size
    m_objcBlockDescrTy = StructType::create(ctx, objcBlockDescrElements,
                                            "struct.__block_descriptor");
    // Prepare structure elements which are common for all block types.
    // <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>
    // Each concreete block must append context elemenents this list.
    PointerType * objcBlockDescrPtrTy = m_objcBlockDescrTy->getPointerTo(OCLAddressSpace::Private);
    m_objcBlockContextElements.resize(ObjCBlockElementsNumWithoutContext);
    m_objcBlockContextElements[0] = int8PtrTy;           // isa
    m_objcBlockContextElements[1] = m_int32Ty;           // flags
    m_objcBlockContextElements[2] = m_int32Ty;           // reserved
    m_objcBlockContextElements[3] = int8PtrTy;           // invoke
    m_objcBlockContextElements[4] = objcBlockDescrPtrTy; // block descriptor
  }

  //************************************************************************
  // Helper function to replace all GEPs from SPIR 2.0 w\ ObjC GEPs
  //************************************************************************
  void SPIR20BlocksToObjCBlocks::replaceGEPs(Value * spirContextPtr,
                                             Value * objcContextPtr) {
    Value::user_iterator userIt = spirContextPtr->user_begin();
    for(;userIt != spirContextPtr->user_end();) {
      // Notice users are erased per iteration which invalidates current iterator
      User * user = *(userIt++);
      // Go over all GEPs and subsequent stores to recapture the contexts.
      GetElementPtrInst * spirCapturedGEP = dyn_cast<GetElementPtrInst>(user);
      if(!spirCapturedGEP) continue;
      // Get the second index of GEP from SPIR 2.0 captured context and
      // add ObjCBlockElementsNumWithoutContext to it. The first index
      // is 0.
      assert(spirCapturedGEP->getNumIndices() == 2 &&
             "unexpected number of indices into SPIR 2.0 context");
      GetElementPtrInst::op_iterator idxIter = spirCapturedGEP->idx_begin();
      int64_t spirCapturedValueIdx =
        cast<ConstantInt>((++idxIter)->get())->getSExtValue();

      Value * objcCapturedGEPIndices[2] =
        { ConstantInt::get(m_int32Ty, 0),
          ConstantInt::get(m_int32Ty,
                           ObjCBlockElementsNumWithoutContext + spirCapturedValueIdx) };

      Instruction* objcCapturedGEP =
        GetElementPtrInst::Create(objcContextPtr, objcCapturedGEPIndices,
                                  "objc.block.captured", spirCapturedGEP);
      spirCapturedGEP->replaceAllUsesWith(objcCapturedGEP);
      spirCapturedGEP->eraseFromParent();
    }
  }

  // ****************************************************************
  // Create and initialize a specific Objective-C block using call to
  // "spir_block_bind" to gather necessary data.
  // ****************************************************************
  Value * SPIR20BlocksToObjCBlocks::getOrCreateObjCBlock(Module &M,
                                                         CallInst * spirBlockBindCI) {
    // First check if ObjectiveC block for this invoke has been created already.
    Value * spirBlockInvoke = spirBlockBindCI->getArgOperand(0);
    auto foundBlock = m_objcBlocks.find(spirBlockInvoke);
    if(foundBlock != m_objcBlocks.end()) return foundBlock->second;

    LLVMContext & ctx = M.getContext();
    Function * parentFunc = spirBlockBindCI->getParent()->getParent();
    Instruction * insertBefore = parentFunc->getEntryBlock().getFirstInsertionPt();

    // Create a packed block structure type like in the example below;
    // <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, ... #CapturedContextTypes }>
    // If the captured context isn't empty then cast to i8* is expected
    CastInst * spirBlockContextCast = dyn_cast<CastInst>(spirBlockBindCI->getArgOperand(3));
    if(spirBlockContextCast) {
      m_objcBlockContextElements.resize(ObjCBlockElementsNumWithoutContext);
      PointerType * spirBlockCtxPtrTy = cast<PointerType>(spirBlockContextCast->getSrcTy());
      StructType * spirBlockContextTy = cast<StructType>(spirBlockCtxPtrTy->getElementType());
      m_objcBlockContextElements.append(spirBlockContextTy->elements().begin(),
                                        spirBlockContextTy->elements().end());
    }
    StructType * objcBlockContextTy = StructType::get(ctx, m_objcBlockContextElements,
                                                      /* isPacked */ true);
    // Create a block descritptor for this block:
    // @__block_descriptor = internal constant { i64, i64 } { i64 #Reserved, i64 #Size }
    const DataLayout * DL = M.getDataLayout();
    uint64_t objcSizeOfBlock =
      DL->getTypeAllocSize(objcBlockContextTy);

    Constant * objcBlockDescrInitElements[2] = {
      ConstantInt::get(m_int64Ty, 0),                // reserved
      ConstantInt::get(m_int64Ty, objcSizeOfBlock)}; // block size
    Constant * objcBlockDescrInit =
      ConstantStruct::get(m_objcBlockDescrTy, objcBlockDescrInitElements);

    GlobalVariable * objcBlockDescrGV =
      new GlobalVariable(M, m_objcBlockDescrTy, true, GlobalValue::InternalLinkage,
                         objcBlockDescrInit, "__block_descriptor.objc");

    // Allocate Objective-C block on the stack.
    // %objc.block = alloca <{ i8*, i32, i32, i8*, %struct.__block_descriptor*, ... #CapturedContextTypes }>
    AllocaInst * objcBlockAlloca =
      new AllocaInst(objcBlockContextTy, "objc.block", insertBefore);
    objcBlockAlloca->setAlignment(DL->getPrefTypeAlignment(objcBlockContextTy));
    // Initialize invoke, block descriptor, and context elements; others aren't used
    // by OpenCL CPU BE
    // 1. Store invoke passed to spir_block_bind to Objective-C block
    Value * objcInvokeGEPIndices[2] = { ConstantInt::get(m_int32Ty, 0),
                                        ConstantInt::get(m_int32Ty, 3) };
    Instruction* objcInvokePtr =
      GetElementPtrInst::Create(objcBlockAlloca, objcInvokeGEPIndices,
                                "objc.block.invoke", insertBefore);
    new StoreInst(spirBlockInvoke, objcInvokePtr, insertBefore);

    // 2. Store a pointer to the block desriptor global value created earlier
    Value * objcBlockDescrGEPIndices[2] = { ConstantInt::get(m_int32Ty, 0),
                                            ConstantInt::get(m_int32Ty, 4) };
    Instruction* objcBlockDescrPtr =
      GetElementPtrInst::Create(objcBlockAlloca, objcBlockDescrGEPIndices,
                                "objc.block.descriptor", insertBefore);
    new StoreInst(objcBlockDescrGV, objcBlockDescrPtr, insertBefore);

    // 3. Store captured context if any and fix block invoke function
    if(spirBlockContextCast) {
      Value * spirContextAlloca = spirBlockContextCast->getOperand(0);
      assert(isa<AllocaInst>(spirContextAlloca) &&
             "alloca for SPIR 2.0 captured context is expected");
      replaceGEPs(spirContextAlloca, objcBlockAlloca);
    }
    // Fix the invoke which still expects captured data as an agrument
    // and all GEPs from it (even if nothing have been captured).
    fixBlockInvoke(M, spirBlockInvoke, objcBlockContextTy);

    // Ideally next what has to be done is to replace all uses of "opencl.block"
    // opaque type with this new block type but this is tedious and error prone.
    // It is easier to cast the poiner to Objective-C block to SPIR 2.0 block and
    // rely on BIImport pass which is able to properly map opaque struct types while
    // importing built-ins to a user module.
    CastInst * objcBlock =
      CastInst::CreatePointerCast(objcBlockAlloca, spirBlockBindCI->getType(),
                                  "objc.to.spir.cast", insertBefore);
    return m_objcBlocks[spirBlockInvoke] = objcBlock;
  }

  //************************************************************************
  // Helper function to look for a specific use in LLVM IR even if it wasn't
  // optimized which means local variables are accessed through the stack.
  //************************************************************************
  template<class InstT>
  static Instruction * findFirstDistantUse(Value * usedValue) {
    InstT * theDistantUse = nullptr;
    for(User * user : usedValue->users())
      if(isa<InstT>(user)) theDistantUse = cast<InstT>(user);
    // This method is intended to work with unoptimized LLVM IR which means a value
    // could be stored onto the stack and loaded from it afterwards.
    if(!theDistantUse) {
      StoreInst * usedValueStore = nullptr;
      for(User * user : usedValue->users())
        if(isa<StoreInst>(user)) usedValueStore = cast<StoreInst>(user);
      // Stop right there if there is no any store instruction.
      if(!usedValueStore) return nullptr;
      // Otherwise look for the first load.
      Value * usedValuePtr = usedValueStore->getPointerOperand();
      Instruction * usedValueLoad = nullptr;
      for(User * user : usedValuePtr->users())
        if(isa<LoadInst>(user)) usedValueLoad = cast<LoadInst>(user);
      // Stop right there if there is no any load instruction.
      if(!usedValueLoad) return nullptr;
      // Check if the caller is looking for a LoadInst
      if(isa<InstT>(usedValueLoad)) return usedValueLoad;
      // Finally take a look for InstT user of the found load instruciton.
      for(User * user : usedValueLoad->users())
        if(isa<InstT>(user)) theDistantUse = cast<InstT>(user);
    }
    return theDistantUse;
  }
  // *************************************************************************
  // SPIR 2.0 block invoke functions expects only captured data while
  // Objecitve-C blocks expects pointer to the block. So, the invoke functions
  // should be fixed accordingly.
  // *************************************************************************
  void SPIR20BlocksToObjCBlocks::fixBlockInvoke(llvm::Module &M,
                                                Value * spirBlockInvoke,
                                                StructType * objcBlockContextTy) {
    Function * invokeFunc = cast<Function>(spirBlockInvoke->stripPointerCasts());
    // The invoke function must have an argument which is a pointer to opencl.block type
    // but it actually might be the second argument if this invoke returns a struct
    auto aIter = invokeFunc->arg_begin();
    Argument * invokeArgument = aIter;
    // Skip struct return
    if (invokeArgument->hasStructRetAttr())
      invokeArgument = ++aIter;

    // Get pointer to Objective-C context
    PointerType * objcBlockContextPtrTy =
      PointerType::get(objcBlockContextTy,
                       cast<PointerType>(invokeArgument->getType())->getAddressSpace());
    // Replace cast to SPIR captured context with Objective-C captured context.
    Instruction * spirCastToCaptured = findFirstDistantUse<CastInst>(invokeArgument);
    // If no bitcast from void* to SPIR 2.0 context found yet then there is no
    // context captured so nothing to fix.
    if(!spirCastToCaptured) return;
    // Check if this bitcast is used through the stack
    Instruction * spirCastLoad = findFirstDistantUse<LoadInst>(spirCastToCaptured);
    if(spirCastLoad) spirCastToCaptured = spirCastLoad;

    // Create cast to Objective-C block context
    CastInst * obcjCastToCaptured =
      CastInst::CreatePointerCast(invokeArgument, objcBlockContextPtrTy,
                                  "spir.to.ojbc.cast", spirCastToCaptured);
    replaceGEPs(spirCastToCaptured, obcjCastToCaptured);

    if(spirCastToCaptured->hasNUses(0))
      spirCastToCaptured->eraseFromParent();
  }

} // namespace intel
