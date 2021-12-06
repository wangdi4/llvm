// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

#include "AddTLSGlobals.h"
#include "CompilationUtils.h"
#include "ImplicitArgsUtils.h"
#include "InitializePasses.h"
#include "OCLPassSupport.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

extern "C" {
/// @brief Creates new AddTLSGlobals module pass
/// @returns new AddTLSGlobals module pass
ModulePass *createAddTLSGlobalsPass() { return new intel::AddTLSGlobals(); }
}

// Command line option for other passes to use TLS mode
bool OptUseTLSGlobals;

static cl::opt<bool, true> OptUseTLSGlobalsOpt(
    "use-tls-globals", cl::Hidden,
    cl::desc("Use TLS globals instead of implicit arguments"),
    cl::location(OptUseTLSGlobals),
    cl::init(false));

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

char AddTLSGlobals::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(AddTLSGlobals, "add-tls-globals",
                          "Adds TLS global variables to the module", false,
                          false)
OCL_INITIALIZE_PASS_DEPENDENCY(LocalBufferAnalysisLegacy)
OCL_INITIALIZE_PASS_DEPENDENCY(ImplicitArgsAnalysisLegacy)
OCL_INITIALIZE_PASS_END(AddTLSGlobals, "add-tls-globals",
                        "Adds TLS global variables to the module", false, false)

AddTLSGlobals::AddTLSGlobals()
    : ModulePass(ID), m_pModule(nullptr), m_LBInfo(nullptr),
      m_IAA(nullptr), m_pLLVMContext(nullptr) {
  initializeAddTLSGlobalsPass(*llvm::PassRegistry::getPassRegistry());
}

bool AddTLSGlobals::runOnModule(Module &M) {
  m_pModule = &M;
  m_pLLVMContext = &M.getContext();
  m_LBInfo =
      &getAnalysis<LocalBufferAnalysisLegacy>().getResult();
  m_IAA = &getAnalysis<ImplicitArgsAnalysisLegacy>();
  ImplicitArgsInfo &IAInfo = m_IAA->getResult();

  // Create TLS globals
  for (unsigned i = 0; i < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++i) {
    // TODO handle name conflicts
    assert(!M.getGlobalVariable(ImplicitArgsUtils::getArgName(i)));
    Type *ArgType = IAInfo.getArgType(i);
    GlobalVariable *GV = new GlobalVariable(
        M, ArgType, false, GlobalValue::LinkOnceODRLinkage,
        UndefValue::get(ArgType), ImplicitArgsUtils::getArgName(i), nullptr,
        GlobalValue::GeneralDynamicTLSModel);
    GV->setAlignment(M.getDataLayout().getPreferredAlign(GV));
    if (i == ImplicitArgsUtils::IA_SLM_BUFFER) {
      m_pLocalMemBase = GV;
    }
  }

  // Collect all module functions that are not declarations for handling
  std::vector<Function *> FunctionsToHandle;
  for (Function &Func : M) {
    if (Func.isDeclaration()) {
      // Function is not defined inside module
      continue;
    }
    // No need to handle global ctors/dtors
    if (CompilationUtils::isGlobalCtorDtorOrCPPFunc(&Func))
      continue;

    FunctionsToHandle.push_back(&Func);
  }

  // Run on all collected functions for handling and handle them
  for (Function *pFunc : FunctionsToHandle) {
    runOnFunction(pFunc);
  }

  return true;
}

void AddTLSGlobals::runOnFunction(Function *pFunc) {
  llvm::DataLayout DL(m_pModule);

  // Calculate pointer to the local memory buffer
  unsigned int directLocalSize =
      (unsigned int)m_LBInfo->getDirectLocalsSize(pFunc);

  // Go through function instructions and search for calls
  for (BasicBlock &BB : *pFunc) {
    for (Instruction &Inst : BB) {
      if (auto *pCall = dyn_cast<CallInst>(&Inst)) {
        // Check call for not inlined module function
        // TODO Updating pLocalMemBase in this manner renders debugger unable to
        // call functions that use local buffers properly, they should be
        // handled
        // differently
        Function *pCallee = pCall->getCalledFunction();
        if (nullptr != pCallee && !pCallee->isDeclaration()) {
          IRBuilder<> B(pCall);
          Value *Load =
              B.CreateLoad(m_pLocalMemBase->getValueType(), m_pLocalMemBase);
          std::string ValName("pLocalMem_");
          ValName += pCallee->getName();
          Value *NewLocalMem = B.CreateGEP(
              Load->getType()->getScalarType()->getPointerElementType(), Load,
              ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32),
                               directLocalSize),
              ValName);

          // Now that the local memory buffer is rebased, the memory location of
          // directly used local values should be passed to the callee so that
          // local values can be loaded/stored correctly by callee. In this way,
          // the pointers to local values are pushed to new local memory buffer
          // base, and then callee can access local values via the pointers.
          auto &CalleeLocalSet = m_LBInfo->getDirectLocals(pCallee);
          Type *Ty =
              NewLocalMem->getType()->getScalarType()->getPointerElementType();
          unsigned int CurrLocalOffset = 0;
          for (auto *Local : CalleeLocalSet) {
            GlobalVariable *GV = cast<GlobalVariable>(Local);
            size_t ArraySize = DL.getTypeAllocSize(GV->getValueType());
            assert(0 != ArraySize && "zero array size!");
            // Get the position of Local in NewLocalMem.
            auto *Idx = ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32),
                                         CurrLocalOffset);
            auto *LocalPtr = B.CreateGEP(Ty, NewLocalMem, Idx);
            // Cast to pointer type of Local.
            auto *Cast =
                B.CreatePointerCast(LocalPtr, GV->getType()->getPointerTo());
            // Store Local to NewLocalMem.
            B.CreateStore(GV, Cast);
            // Calculate the offset of next direct used local value.
            CurrLocalOffset += ADJUST_SIZE_TO_MAXIMUM_ALIGN(ArraySize);
          }

          B.CreateStore(NewLocalMem, m_pLocalMemBase);
          B.SetInsertPoint(pCall->getNextNode());
          B.SetCurrentDebugLocation(pCall->getDebugLoc());
          B.CreateStore(Load, m_pLocalMemBase);
        }
      }
    }
  }
}

} // namespace intel
