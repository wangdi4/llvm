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
#include "OCLPassSupport.h"
#include "common_dev_limits.h"

#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#define DEBUG_TYPE "local-buffers"

extern "C"
{
ModulePass *createLocalBuffersPass(bool useTLSGlobals) {
  return new intel::LocalBuffers(useTLSGlobals);
}
}

using namespace Intel::OpenCL::DeviceBackend;

extern bool OptUseTLSGlobals;

namespace intel{

  char LocalBuffers::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(LocalBuffers, DEBUG_TYPE, DEBUG_TYPE, false, false)
  OCL_INITIALIZE_PASS_DEPENDENCY(LocalBuffAnalysis)
  OCL_INITIALIZE_PASS_END(LocalBuffers, DEBUG_TYPE, DEBUG_TYPE, false, false)

  LocalBuffers::LocalBuffers(bool useTLSGlobals)
      : ModulePass(ID), m_pModule(nullptr), m_pLLVMContext(nullptr),
        m_localBuffersAnalysis(nullptr),
        m_useTLSGlobals(useTLSGlobals || OptUseTLSGlobals),
        m_pInsertPoint(nullptr), m_pSubprogram(nullptr) {}

  bool LocalBuffers::runOnModule(Module &M) {
    using namespace DPCPPKernelMetadataAPI;

    m_pModule = &M;
    m_pLLVMContext = &M.getContext();

    m_DIFinder = DebugInfoFinder();
    m_DIFinder.processModule(M);

    // Get all kernels
    CompilationUtils::FunctionSet kernelsFunctionSet;
    CompilationUtils::getAllKernels(kernelsFunctionSet, &M);

    m_localBuffersAnalysis = &getAnalysis<LocalBuffAnalysis>();

    m_GVEToRemove.clear();
    m_GVToRemove.clear();

    // Run on all defined function in the module
    for ( auto &F : M ) {
      Function *pFunc = &F;
      if ( !pFunc || pFunc->isDeclaration () ) {
        // Function is not defined inside module
        continue;
      }

      // pipes ctor is not a kernel
      if (CompilationUtils::isGlobalCtorDtorOrCPPFunc(pFunc))
        continue;

      runOnFunction(pFunc);
      if (kernelsFunctionSet.count(pFunc) ) {
        //We have a kernel, update metadata
        KernelInternalMetadataAPI(pFunc).LocalBufferSize.set(m_localBuffersAnalysis->getLocalsSize(pFunc));
      }
    }

    UpdateDICompileUnitGlobals();

    // Safely erase useless GVs
    for (auto *GV : m_GVToRemove)
      GV->eraseFromParent();

    return true;
  }

  Value *LocalBuffers::CreateInstructionFromConstantWithReplacement(Constant *C,
                                                                    Value *From,
                                                                    Value *To) {
    if (auto *CExpr = dyn_cast<ConstantExpr>(C)) {
      auto *Inst = CExpr->getAsInstruction();
      Inst->insertBefore(m_pInsertPoint);
      Inst->replaceUsesOfWith(From, To);
      LLVM_DEBUG(dbgs() << "[CreateInstructionFromConstantWithReplacement]\n"
                        << "  Constant: " << *C << '\n'
                        << "  Instruction equivalent: " << *Inst << '\n');
      return Inst;
    }

    IRBuilder<> B(m_pInsertPoint);
    if (m_pSubprogram)
      B.SetCurrentDebugLocation(
          DILocation::get(*m_pLLVMContext, 0, 0, m_pSubprogram));

    Value *V = UndefValue::get(C->getType());
    for (unsigned i = 0; i < C->getNumOperands(); ++i) {
      Value *Op = C->getOperand(i);
      if (Op == From)
        Op = To;

      if (isa<ConstantVector>(C))
        V = B.CreateInsertElement(V, Op, i, "Insert");
      else if (isa<ConstantStruct>(C))
        V = B.CreateInsertValue(V, Op, i, "Insert");
      else {
        assert(false && "Unimplemented constant type conversion");
        return C;
      }
    }
    return V;
  }

  void LocalBuffers::ReplaceAllUsesOfConstantWith(Constant *From, Value *To) {
    assert(From && To && "shouldn't be nullptr");
    LLVM_DEBUG(dbgs() << "[ReplaceAllUsesOfConstantWith]\n"
                      << "  From: " << *From << '\n'
                      << "  To: " << *To << '\n');

    // Make an explicit copy of From's users, as we may erase user in the loop
    SmallVector<User *, 2> Users(From->users());
    for (auto *U : Users) {
      LLVM_DEBUG(dbgs() << "    Checking user: " << *U << '\n');
      if (auto *Inst = dyn_cast<Instruction>(U)) {
        if (Inst->getFunction() == m_pInsertPoint->getFunction())
          Inst->replaceUsesOfWith(From, To);
      } else if (auto *CUser = dyn_cast<Constant>(U)) {
        if (CUser->use_empty())
          continue;

        Value *Inst =
            CreateInstructionFromConstantWithReplacement(CUser, From, To);
        ReplaceAllUsesOfConstantWith(CUser, Inst);
      }
    }
  }

  void LocalBuffers::AttachDebugInfoToLocalMem(GlobalVariable *GV,
                                               Value *pLocalMem,
                                               unsigned offset) {
    SmallVector<DIGlobalVariableExpression *, 1> DIGVExprs;
    GV->getDebugInfo(DIGVExprs);

    DIBuilder DIB(*m_pModule, false);

    for (auto *DIGVExpr : DIGVExprs) {
      auto *DIGV = DIGVExpr->getVariable();
      if (DIGV->getScope() != m_pSubprogram)
        continue;
      auto *DIExpr = DIGVExpr->getExpression();
      DIExpr = DIExpression::prepend(DIExpr, DIExpression::DerefBefore, offset);

      // Create a function-level Debug local variable
      auto *DILV = DIB.createAutoVariable(
          DIGV->getScope(), DIGV->getName(), DIGV->getFile(), DIGV->getLine(),
          DIGV->getType(), true, DINode::FlagArtificial);

      DIB.insertDbgValueIntrinsic(
          pLocalMem, DILV, DIExpr,
          DILocation::get(*m_pLLVMContext, 0, 0, m_pSubprogram),
          m_pInsertPoint);
    }

    m_GVEToRemove.insert(DIGVExprs.begin(), DIGVExprs.end());
    m_GVToRemove.insert(GV);
  }

  void LocalBuffers::UpdateDICompileUnitGlobals() {
    SmallVector<Metadata *> LiveGVs;
    for (auto *DICU : m_DIFinder.compile_units()) {
      LiveGVs.clear();

      for (auto *DIG : DICU->getGlobalVariables()) {
        if (!m_GVEToRemove.count(DIG))
          LiveGVs.push_back(DIG);
      }

      DICU->replaceGlobalVariables(MDTuple::get(*m_pLLVMContext, LiveGVs));
    }
  }

  // Substitutes a pointer to local buffer, with argument passed within kernel parameters
  void LocalBuffers::parseLocalBuffers(Function *pFunc, Value *pLocalMem) {

    // Get all __local variables owned by pFunc
    auto &LocalSet = m_localBuffersAnalysis->getDirectLocals(pFunc);

    unsigned int currLocalOffset = 0;

    // Iterate through local buffers
    for (auto *Local : LocalSet) {
      GlobalVariable *GV = cast<GlobalVariable>(Local);

      // Calculate required buffer size
      llvm::DataLayout DL(m_pModule);
      size_t uiArraySize = DL.getTypeAllocSize(GV->getType()->getElementType());
      assert(0 != uiArraySize && "zero array size!");
      // Now retrieve to the offset of the local buffer
      GetElementPtrInst *pLocalAddr = GetElementPtrInst::Create(
          nullptr, pLocalMem,
          ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32),
                           currLocalOffset),
          "", m_pInsertPoint);

      // Now add bitcast to required/original pointer type
      CastInst *pPointerCast = CastInst::CreatePointerCast(
          pLocalAddr, GV->getType(), "", m_pInsertPoint);

      ReplaceAllUsesOfConstantWith(GV, pPointerCast);
      if (m_pSubprogram)
        AttachDebugInfoToLocalMem(GV, pLocalMem, currLocalOffset);

      // Advance total implicit size
      currLocalOffset += ADJUST_SIZE_TO_MAXIMUM_ALIGN(uiArraySize);
    }

    assert( currLocalOffset == m_localBuffersAnalysis->getDirectLocalsSize(pFunc) &&
      "CurrLocalOffset is not equal to local buffer size!" );
  }

  void LocalBuffers::runOnFunction(Function *pFunc) {
    m_pInsertPoint = &*inst_begin(pFunc);
    m_pSubprogram = pFunc->getSubprogram();

    // Getting the implicit arguments
    Value *pLocalMem = 0;
    if (m_useTLSGlobals) {
      pLocalMem = CompilationUtils::getTLSGlobal(
          m_pModule, ImplicitArgsUtils::IA_SLM_BUFFER);
      assert(pLocalMem && "TLS pLocalMem is not found.");
      // Load the LocalMem pointer from TLS GlobalVariable
      pLocalMem = new LoadInst(pLocalMem->getType()->getPointerElementType(),
                               pLocalMem, "LocalMemBase", m_pInsertPoint);
    } else {
    CompilationUtils::getImplicitArgs(
        pFunc, &pLocalMem, nullptr, nullptr, nullptr, nullptr, nullptr);
    }

    assert(pLocalMem && "pLocalMem should not be nullptr.");
    parseLocalBuffers(pFunc, pLocalMem);
  }
} // namespace intel
