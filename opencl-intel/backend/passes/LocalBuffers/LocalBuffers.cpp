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

#include "LocalBuffers.h"
#include "CompilationUtils.h"
#include "ImplicitArgsUtils.h"
#include "InitializePasses.h"
#include "common_dev_limits.h"
#include "MetadataAPI.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DataLayout.h"

extern "C"
{
ModulePass *createLocalBuffersPass(bool isNativeDBG, bool useTLSGlobals) {
  return new intel::LocalBuffers(isNativeDBG, useTLSGlobals);
  }
}

using namespace Intel::OpenCL::DeviceBackend;

namespace intel{

  char LocalBuffers::ID = 0;

  LocalBuffers::LocalBuffers(bool isNativeDBG, bool useTLSGlobals)
      : ModulePass(ID), m_pModule(nullptr), m_pLLVMContext(nullptr),
        m_localBuffersAnalysis(nullptr), m_isNativeDBG(isNativeDBG),
        m_useTLSGlobals(useTLSGlobals) {
      initializeLocalBuffAnalysisPass(*llvm::PassRegistry::getPassRegistry());
  }

  bool LocalBuffers::runOnModule(Module &M) {
    using namespace Intel::MetadataAPI;

    m_pModule = &M;
    m_pLLVMContext = &M.getContext();

    // Get all kernels
    CompilationUtils::FunctionSet kernelsFunctionSet;
    CompilationUtils::getAllKernels(kernelsFunctionSet, &M);

    m_localBuffersAnalysis = &getAnalysis<LocalBuffAnalysis>();

    // Run on all defined function in the module
    for ( auto &F : M ) {
      Function *pFunc = &F;
      if ( !pFunc || pFunc->isDeclaration () ) {
        // Function is not defined inside module
        continue;
      }

      // pipes ctor is not a kernel
      if (CompilationUtils::isGlobalCtorDtor(pFunc))
        continue;

      runOnFunction(pFunc);
      if (kernelsFunctionSet.count(pFunc) ) {
        //We have a kernel, update metadata
        KernelInternalMetadataAPI(pFunc).LocalBufferSize.set(m_localBuffersAnalysis->getLocalsSize(pFunc));
      }
    }

    return true;
  }

  // TODO: LLVM has a special method to create Instruction from ConstExpr, we should use it instead.
  Instruction* LocalBuffers::CreateInstrFromConstant(Constant *pCE, Value *From, Value *To,
    std::vector<Instruction*> *InstInsert) {

    Instruction *Replacement = 0;

    if (ConstantExpr *CEx = dyn_cast<ConstantExpr>(pCE)) {
      if ( CEx->getOpcode() == Instruction::GetElementPtr ) {
        SmallVector<Value*, 8> Indices;
        Value *Pointer = pCE->getOperand(0);
        Indices.reserve(pCE->getNumOperands()-1);
        if (Pointer == From) Pointer = To;

        for ( unsigned i = 1, e = pCE->getNumOperands(); i != e; ++i ) {
          Value *Val = CEx->getOperand(i);
          if (Val == From) Val = To;
          Indices.push_back(Val);
        }
        // [LLVM 3.8 UPGRADE] ToDo: Replace nullptr for pointer type with actual type
        // (not using type from pointer as this functionality is planned to be removed.
        Replacement = GetElementPtrInst::Create(nullptr, Pointer, ArrayRef<Value*>(Indices));

      } else if ( CEx->getOpcode() == Instruction::ExtractValue ) {
        Value *Agg = CEx->getOperand(0);
        if (Agg == From) Agg = To;

        Replacement = ExtractValueInst::Create(Agg,  CEx->getIndices());

      } else if (CEx->getOpcode() == Instruction::InsertValue) {
        Value *Agg = CEx->getOperand(0);
        Value *Val = CEx->getOperand(1);
        if (Agg == From) Agg = To;
        if (Val == From) Val = To;

        Replacement = InsertValueInst::Create(Agg, Val, CEx->getIndices());

      } else if (CEx->isCast()) {
        assert(CEx->getOperand(0) == From && "Cast only has one use!");
        Replacement = CastInst::Create((Instruction::CastOps)CEx->getOpcode(), To, CEx->getType());

      } else if (CEx->getOpcode() == Instruction::Select) {
        Value *C1 = CEx->getOperand(0);
        Value *C2 = CEx->getOperand(1);
        Value *C3 = CEx->getOperand(2);
        if (C1 == From) C1 = To;
        if (C2 == From) C2 = To;
        if (C3 == From) C3 = To;
        Replacement = SelectInst::Create(C1, C2, C3);

      } else if (CEx->getOpcode() == Instruction::ExtractElement) {
        Value *C1 = CEx->getOperand(0);
        Value *C2 = CEx->getOperand(1);
        if (C1 == From) C1 = To;
        if (C2 == From) C2 = To;
        Replacement = ExtractElementInst::Create(C1, C2);

      } else if (CEx->getOpcode() == Instruction::InsertElement) {
        Value *C1 = CEx->getOperand(0);
        Value *C2 = CEx->getOperand(1);
        Value *C3 = CEx->getOperand(1);
        if (C1 == From) C1 = To;
        if (C2 == From) C2 = To;
        if (C3 == From) C3 = To;
        Replacement = InsertElementInst::Create(C1, C2, C3);

      } else if (CEx->getOpcode() == Instruction::ShuffleVector) {
        Value *C1 = CEx->getOperand(0);
        Value *C2 = CEx->getOperand(1);
        Value *C3 = CEx->getOperand(2);
        if (C1 == From) C1 = To;
        if (C2 == From) C2 = To;
        if (C3 == From) C3 = To;
        Replacement = new ShuffleVectorInst(C1, C2, C3);

      } else if (CEx->isCompare()) {
        Value *C1 = CEx->getOperand(0);
        Value *C2 = CEx->getOperand(1);
        if (C1 == From) C1 = To;
        if (C2 == From) C2 = To;
        Replacement = CmpInst::Create(
                (Instruction::OtherOps)CEx->getOpcode(),
                (llvm::CmpInst::Predicate)CEx->getPredicate(),
                C1,
                C2);

      } else {
        assert(0 && "Unknown ConstantExpr type!");
        return nullptr;
      }

      InstInsert->push_back(Replacement);
      return Replacement;
    }

    if (ConstantVector *CV = dyn_cast<ConstantVector>(pCE)) {

      VectorType *VT = CV->getType();
      unsigned numElem = VT->getNumElements();

      Value *UpdatedVec = UndefValue::get(CV->getType());
      ConstantInt *Idx = nullptr;
      Instruction *IE = nullptr;
      for (unsigned i = 0; i< numElem; i++) {
        Value *toInsert = nullptr;
        if (CV->getOperand(i) == From) {
          toInsert = To;
        } else {
          toInsert = CV->getOperand(i);
        }
        Idx = ConstantInt::get(IntegerType::get(CV->getContext(), 32) , i);
        IE = InsertElementInst::Create(UpdatedVec, toInsert, Idx, "Insert");
        UpdatedVec = IE;
        InstInsert->push_back(IE);
      }
      assert(IE  && "Unable to find source constant");
      return IE;
    }

    if (isa<ConstantArray>(pCE)) {
      // Right now, the only case in which this happens is when replacing annotations
      // This means we can simply avoid replacing it.
      // However, this may become relevant in the future, so leaving a special case.
      return 0;
    }

    // No need to check for ConstantDataVector here - it is composed of data,
    // so none of the elements can ever be the replaced val.

    assert(0  && "Unknown constant type");
    return 0;
  }

  bool LocalBuffers::ChangeConstant(Value *pTheValue, Value *pUser, Instruction *pBC, Instruction *Where) {

    // We need substitute constant expression with real instruction
    Constant *pCE = cast<Constant>(pUser);

    std::vector<Instruction*> InstInsert;
    Instruction *pInst = CreateInstrFromConstant(pCE, pTheValue, pBC, &InstInsert);
    if( nullptr == pInst)
    {
        return false;
    }
    //Klocwork add assert to prevent warnings
    assert(pInst && InstInsert.size());

    // Change all non-constant references recursively
    SmallVector<User *, 8> users(pCE->users());
    for (User * user : users) {
      if (isa<Constant>(user)) {
        ChangeConstant(pUser, user, pInst, Where);
      }
      // Check if user is an instruction that belongs to the same function
      else if (Instruction *Inst = dyn_cast<Instruction>(user)) {
          if (Inst->getParent()->getParent() == Where->getParent()->getParent()) {
              Inst->replaceUsesOfWith(pUser, pInst);
          }
      } else {
        // Currently, the only possible users of llvm::ContantExpr are llvm::Constant, llvm::Instruction and llvm::Operator
        // llvm::Operator is an internal class, so we do not expect it to be a user of ConstantExp
        assert(0 && "Unknown user of constant expression");
      }
    }

    // Check if the instruction was not used
    if ( pInst->use_empty() ) {
       for (Instruction * inst : InstInsert) {
         if (!inst->getType()->isVoidTy())
          inst->replaceAllUsesWith(UndefValue::get(inst->getType()));
       inst->dropAllReferences();
       }
    }

    // Add instruction to the block, only the first time and only if it has uses
    else if ( !pInst->getParent() ) {
      for (Instruction * inst : InstInsert) {
        inst->insertAfter(Where);
        Where = inst;
      }
    }

    if ( !pCE->use_empty() ) {
      return false;
    }

    return true;
  }

  // Substitutes a pointer to local buffer, with argument passed within kernel parameters
  void LocalBuffers::parseLocalBuffers(Function *pFunc, Value *pLocalMem) {

    LocalBuffAnalysis::TUsedLocals localsSet = m_localBuffersAnalysis->getDirectLocals(pFunc);
    IRBuilder<> builder(*m_pLLVMContext);

    unsigned int currLocalOffset = 0;
    // Create code for local buffer
    Instruction *pFirstInst = dyn_cast<Instruction>(pFunc->getEntryBlock().begin());

    if (m_useTLSGlobals) {
      builder.SetInsertPoint(pFirstInst);
      pLocalMem = builder.CreateLoad(pLocalMem);
    }
    // Iterate through local buffers
    for ( LocalBuffAnalysis::TUsedLocals::const_iterator gi = localsSet.begin(),
      ge = localsSet.end(); gi != ge; ++gi ) {
        GlobalValue *pLclBuff = dyn_cast<GlobalValue>(*gi);

        if(nullptr == pLclBuff) {
          continue;
        }

        // Calculate required buffer size
        llvm::DataLayout DL(m_pModule);
        size_t uiArraySize = DL.getTypeAllocSize(pLclBuff->getType()->getElementType());
        assert(0 != uiArraySize && "zero array size!");
        // Now retrieve to the offset of the local buffer
        // [LLVM 3.8 UPGRADE] ToDo: Replace nullptr for pointer type with actual type
        // (not using type from pointer as this functionality is planned to be removed.
        GetElementPtrInst *pLocalAddr =
          GetElementPtrInst::Create(
                  nullptr,
                  pLocalMem,
                  ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32),currLocalOffset), "", pFirstInst);

        // Now add bitcast to required/original pointer type
        CastInst *pPointerCast = CastInst::CreatePointerCast(pLocalAddr, pLclBuff->getType(), "", pFirstInst);

        // Advance total implicit size
        currLocalOffset += ADJUST_SIZE_TO_MAXIMUM_ALIGN(uiArraySize);

        SmallVector<User *, 8> users(pLclBuff->users());
        for (User * user : users)  {
          if (isa<Constant>(user))  {
            ChangeConstant(pLclBuff, user, pPointerCast, pPointerCast);
          }
           // Check if user is an instruction that belongs to the same function
          else if (Instruction *Inst = dyn_cast<Instruction>(user)) {
            if (Inst->getParent()->getParent() == pFunc) {
              // pPointerCast was already added to a basic block during it's creation
              Inst->replaceUsesOfWith(pLclBuff, pPointerCast);
              // Only if debugging, copy from local memory buffer to thread
              // specific global buffer.
              if (m_isNativeDBG) {
                // Get the next instruction so we can insert the copy to global
                // after the Inst instruction.
                Instruction *pNextInst = cast<Instruction>(&*(++BasicBlock::iterator(Inst)));
                builder.SetInsertPoint(pNextInst);
                builder.CreateMemCpy(pLclBuff, pLclBuff->getAlignment(),
                                     pLocalAddr, pLclBuff->getAlignment(),
                                     uiArraySize, false);
              }
            }
          } else {
            // Currently, the only possible users of llvm::ContantExpr are llvm::Constant, llvm::Instruction and llvm::Contant
            // llvm::Operator is an internal class, so we do not expect it to be a user of ConstantExp
            assert(0 && "Unknown user of constant expression");
          }
        }
        // Special debugging handling for native (gdb) debugging
        if (m_isNativeDBG) {
          // Add copying from local memory buffer to thread global buffer so that the
          // thread global (__local) has valid data at the start of the basic block.
          if (!m_basicBlockSet.empty()) {
            for (std::set<llvm::BasicBlock*>::iterator vi = m_basicBlockSet.begin(),
              ve = m_basicBlockSet.end(); vi != ve; ++vi ) {
                BasicBlock *BB = dyn_cast<BasicBlock>(*vi);
                Instruction &insertBeforeEnd = BB->back();
                //Constant *pSizeToCopy = ConstantExpr::getSizeOf(pLclBuff->getType());
                // Create copy to thread global (from local memory)
                builder.SetInsertPoint(&insertBeforeEnd);
                builder.CreateMemCpy(pLclBuff, pLclBuff->getAlignment(),
                                     pLocalAddr, pLclBuff->getAlignment(),
                                     uiArraySize, false);
            }
          }
        }
    }

    assert( currLocalOffset == m_localBuffersAnalysis->getDirectLocalsSize(pFunc) &&
      "CurrLocalOffset is not equal to local buffer size!" );
  }


  void LocalBuffers::runOnFunction(Function *pFunc) {
    // Getting the implicit arguments
    Value *pLocalMem = 0;

    if (m_useTLSGlobals) {
      pLocalMem = CompilationUtils::getTLSGlobal(
          m_pModule, ImplicitArgsUtils::IA_SLM_BUFFER);
    } else {
    CompilationUtils::getImplicitArgs(
        pFunc, &pLocalMem, nullptr, nullptr, nullptr, nullptr, nullptr);
    }

    // Apple LLVM-IR workaround
    // 1.  Pass WI information structure as the next parameter after given function parameters
    // 2.  We don't want to use TLS for local memory.
    //    Our solution to move all internal local memory blocks to be allocated
    //    by the execution engine and passed within additional parameters to the kernel,
    //    those parameters are not exposed to the user

    // If we have a debug build, need to get the basic blocks where we need to do
    // special debug handling for globals (only for native (gdb) debugging).
    if (m_isNativeDBG)
      updateUsageBlocks(pFunc);

    parseLocalBuffers(pFunc, pLocalMem);
  }

  void LocalBuffers::updateUsageBlocks(Function *pFunc) {
    TInstVector instructionsToDelete;
    instructionsToDelete.clear();
    m_basicBlockSet.clear();

    // Only process functions with IR
    if ( !pFunc || pFunc->isDeclaration())
      return;

    // Look through all basic blocks for calls to DebugCopy.()
    for (Function::iterator vi = pFunc->begin(), ve = pFunc->end(); vi != ve; ++vi) {
      BasicBlock *pBB = dyn_cast<BasicBlock>(&*vi);
      for (BasicBlock::iterator vii = pBB->begin(), vee = pBB->end(); vii != vee; ++vii) {
        if (CallInst* call_instr = dyn_cast<CallInst>(vii)) {
          Function* called_func = call_instr->getCalledFunction();
          assert(called_func && "Unexpected indirect function invocation");
          if (called_func->getName() == "DebugCopy.") {
            // These are dummy calls so we know which blocks need special handling
            // for __locals. These calls are saved to a vector to be deleted later on.
            m_basicBlockSet.insert(pBB);
            instructionsToDelete.push_back(call_instr);
          }
        }
      }
    }
    for (TInstVector::iterator vi = instructionsToDelete.begin(),
      ve = instructionsToDelete.end(); vi != ve; ++vi) {
      Instruction *pInst = cast<Instruction>(*vi);
      pInst->eraseFromParent();
    }
  }

} // namespace intel
