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

OCL_INITIALIZE_PASS(AddTLSGlobals, "add-tls-globals",
                    "Adds TLS global variables to the module", false, false)

AddTLSGlobals::AddTLSGlobals()
    : ModulePass(ID), m_pModule(nullptr), m_localBuffersAnalysis(nullptr),
      m_IAA(nullptr), m_pLLVMContext(nullptr) {
  initializeLocalBuffAnalysisPass(*llvm::PassRegistry::getPassRegistry());
  initializeImplicitArgsAnalysisPass(*llvm::PassRegistry::getPassRegistry());
}

bool AddTLSGlobals::runOnModule(Module &M) {
  m_pModule = &M;
  m_pLLVMContext = &M.getContext();
  m_localBuffersAnalysis = &getAnalysis<LocalBuffAnalysis>();
  m_IAA = &getAnalysis<ImplicitArgsAnalysis>();
  m_IAA->initDuringRun(M.getDataLayout().getPointerSizeInBits(0));

  // Create TLS globals
  for (unsigned i = 0; i < ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS; ++i) {
    // TODO handle name conflicts
    assert(!M.getGlobalVariable(ImplicitArgsUtils::getArgName(i)));
    Type *ArgType = m_IAA->getArgType(i);
    GlobalVariable *GV = new GlobalVariable(
        M, ArgType, false, GlobalValue::LinkOnceODRLinkage,
        UndefValue::get(ArgType), ImplicitArgsUtils::getArgName(i), nullptr,
        GlobalValue::GeneralDynamicTLSModel);
    GV->setAlignment(M.getDataLayout().getPreferredAlignment(GV));
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

  // Calculate pointer to the local memory buffer
  unsigned int directLocalSize =
      (unsigned int)m_localBuffersAnalysis->getDirectLocalsSize(pFunc);

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
          Value *Load = B.CreateLoad(m_pLocalMemBase);
          std::string ValName("pLocalMem_");
          ValName += pCallee->getName();
          Value *GEP = B.CreateGEP(
              Load, ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32),
                                     directLocalSize),
              ValName);
          B.CreateStore(GEP, m_pLocalMemBase);
          B.SetInsertPoint(pCall->getNextNode());
          B.CreateStore(Load, m_pLocalMemBase);
        }
      }
    }
  }
}

} // namespace intel
