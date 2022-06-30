// INTEL CONFIDENTIAL
//
// Copyright 2012-2022 Intel Corporation.
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

#include "InstCounter.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "Logger.h"
#include "LoopUtils/LoopUtils.h"
#include "OCLPassSupport.h"
#include "OpenclRuntime.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/DPCPPStatistic.h"

#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#include <iomanip>
#include <sstream>

#define DEBUG_TYPE "vectorpossible"

extern bool DPCPPEnableDirectFunctionCallVectorization;
extern bool DPCPPEnableSubgroupDirectCallVectorization;

using namespace DPCPPKernelMetadataAPI;
using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

char VectorizationPossibilityPass::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(VectorizationPossibilityPass, "vectorpossible", "Check whether vectorization is possible", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(VectorizationPossibilityPass, "vectorpossible", "Check whether vectorization is possible", false, false)

bool VectorizationPossibilityPass::runOnFunction(Function & F)
{
  DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  RuntimeServices* services = getAnalysis<BuiltinLibInfo>().getRuntimeServices();
  m_canVectorize = CanVectorizeImpl::canVectorize(F, DT, services);
  return false;
}

bool CanVectorizeImpl::canVectorizeForVPO(Function &F, RuntimeServices *services)
{
  DPCPPStatistic::ActiveStatsT kernelStats;
  if (hasVariableGetTIDAccess(F, services)) {
    LLVM_DEBUG(dbgs() << "Variable TID access, can not vectorize\n");
    DPCPP_STAT_DEFINE(CantVectGIDMess,
                      "Unable to vectorize because get_global_id is messed up",
                      kernelStats);
    CantVectGIDMess++;
    DPCPPStatistic::pushFunctionStats(kernelStats, F, DEBUG_TYPE);
    return false;
  }

  if (!DPCPPEnableDirectFunctionCallVectorization) {
    auto KIMD = KernelInternalMetadataAPI(&F);
    bool HasSG =
        KIMD.KernelHasSubgroups.hasValue() && KIMD.KernelHasSubgroups.get();
    if (!(DPCPPEnableSubgroupDirectCallVectorization && HasSG)) {
      if (hasNonInlineUnsupportedFunctions(F)) {
        LLVM_DEBUG(
            dbgs() << "Call to unsupported functions, can not vectorize\n");
        DPCPP_STAT_DEFINE(
            CantVectNonInlineUnsupportedFunctions,
            "Unable to vectorize because of calls to functions that "
            "can't be inlined",
            kernelStats);
        CantVectNonInlineUnsupportedFunctions++;
        DPCPPStatistic::pushFunctionStats(kernelStats, F, DEBUG_TYPE);
        return false;
      }
    }
  }

  DPCPP_STAT_DEFINE(CanVect, "Code is vectorizable", kernelStats);
  CanVect++;
  DPCPPStatistic::pushFunctionStats(kernelStats, F, DEBUG_TYPE);
  return true;
}

bool CanVectorizeImpl::canVectorize(Function &F, DominatorTree &DT, RuntimeServices* services)
{
  DPCPPStatistic::ActiveStatsT kernelStats;

  if (hasVariableGetTIDAccess(F, services)) {
    LLVM_DEBUG(dbgs() << "Variable TID access, can not vectorize\n");
    DPCPP_STAT_DEFINE(CantVectGIDMess,
                      "Unable to vectorize because get_global_id is messed up",
                      kernelStats);
    CantVectGIDMess++;
    DPCPPStatistic::pushFunctionStats(kernelStats, F, DEBUG_TYPE);
    return false;
  }

  if (!isReducibleControlFlow(F, DT)) {
    LLVM_DEBUG(dbgs() << "Irreducible control flow, can not vectorize\n");
    DPCPP_STAT_DEFINE(
        CantVectNonReducable,
        "Unable to vectorize because the control flow is irreducible",
        kernelStats);
    CantVectNonReducable++;
    DPCPPStatistic::pushFunctionStats(kernelStats, F, DEBUG_TYPE);
    return false;
  }

  if (hasIllegalTypes(F)) {
    LLVM_DEBUG(dbgs() << "Types unsupported by codegen, can not vectorize\n");
    DPCPP_STAT_DEFINE(CantVectIllegalTypes,
                      "Unable to vectorize because of unsupported opcodes",
                      kernelStats);
    CantVectIllegalTypes++;
    DPCPPStatistic::pushFunctionStats(kernelStats, F, DEBUG_TYPE);
    return false;
  }

  if (hasNonInlineUnsupportedFunctions(F)) {
    LLVM_DEBUG(dbgs() << "Call to unsupported functions, can not vectorize\n");
    DPCPP_STAT_DEFINE(CantVectNonInlineUnsupportedFunctions,
                      "Unable to vectorize because of calls to functions that "
                      "can't be inlined",
                      kernelStats);
    CantVectNonInlineUnsupportedFunctions++;
    DPCPPStatistic::pushFunctionStats(kernelStats, F, DEBUG_TYPE);
    return false;
  }

  if (hasDirectStreamCalls(F, services)) {
    LLVM_DEBUG(dbgs() << "Has direct calls to stream functions, can not vectorize\n");
    DPCPP_STAT_DEFINE(
        CantVectStreamCalls,
        "Unable to vectorize because the code contains direct stream calls",
        kernelStats);
    CantVectStreamCalls++;
    DPCPPStatistic::pushFunctionStats(kernelStats, F, DEBUG_TYPE);
    return false;
  }

  if (hasUnreachableInstructions(F)) {
    LLVM_DEBUG(dbgs() << "Has unreachable instructions, can not vectorize\n");
    DPCPP_STAT_DEFINE(
        CantVectUnreachableCode,
        "Unable to vectorize because the code contains unreachable code",
        kernelStats);
    CantVectUnreachableCode++;
    DPCPPStatistic::pushFunctionStats(kernelStats, F, DEBUG_TYPE);
    return false;
  }

  DPCPP_STAT_DEFINE(CanVect, "Code is vectorizable", kernelStats);
  CanVect++;
  DPCPPStatistic::pushFunctionStats(kernelStats, F, DEBUG_TYPE);
  return true;
}

bool CanVectorizeImpl::hasVariableGetTIDAccess(Function &F, RuntimeServices* services) {
  assert(services && "Unable to get runtime services");
  for (Function::iterator bbit = F.begin(), bbe=F.end(); bbit != bbe; ++bbit) {
    for (BasicBlock::iterator it = bbit->begin(), e=bbit->end(); it!=e;++it) {
      if (CallInst* call = dyn_cast<CallInst>(it)) {
        bool err = false;
        unsigned dim = 0;
        services->isTIDGenerator(call, &err, &dim);
        // We are unable to vectorize this code because get_global_id is messed up
        if (err) return true;
      }
    }
  }
  // TID access is okay
  return false;
}

// Reference:
//  Book: High Performance Compilers for Parallel Computing / Michael Wolfe
//  Page 60, Section 3.2.4 - Finding Cycles in Directed Graphs
//
// Check if the graph is irreducible using the standard algorithm:
// 1. If you ignore backedges, graph is acyclic.
// 2. backedges are edges where the target dominates the src.
bool CanVectorizeImpl::isReducibleControlFlow(Function& F, DominatorTree& DT) {
  llvm::SmallSet<BasicBlock*, 16> removed;
  llvm::SmallSet<BasicBlock*, 16> toProcess;
  toProcess.insert(&F.getEntryBlock());

  while (!toProcess.empty()) {
    BasicBlock* next = nullptr;
    // Find a free node
    llvm::SmallSet<BasicBlock*, 16>::iterator bb(toProcess.begin());
    llvm::SmallSet<BasicBlock*, 16>::iterator bbe(toProcess.end());
    // for each of the toProcess blocks, find a block who's
    // preds are all removed
    for (; bb != bbe ; ++bb) {
      // Are all of its preds removed ?
      bool Removed = true;
      llvm::pred_iterator p = pred_begin(*bb), pe = pred_end(*bb);

      // for each pred
      for (; p != pe; ++p) {
        // This is a back-edge, ignore it
        // Note: nodes dominate themselves.
        // This is good because this is how we handle self-edges.
        if (DT.dominates(*bb, *p)) continue;
        // pred in removed list ?
        Removed &= removed.count(*p);
      }

      // Found a candidate to remove
      if (Removed) {
        next = *bb;
        break;
      }
    }

    // Did not find a free node to remove,
    // The graph has a cycle. This code is irreducible.
    if (!next) {
      return false;
    }

    // Remove this node
    toProcess.erase(next);
    removed.insert(next);
    // Insert all successors to the 'todo' queue
    llvm::succ_iterator s = succ_begin(next), se = succ_end(next);
    for (; s != se; ++s) {
      // did not visit this before
      if (!removed.count(*s)) toProcess.insert(*s);
    }
  }

  // Code is reducible
  return true;
}

bool CanVectorizeImpl::hasIllegalTypes(Function &F) {
  // For each BB
  for (Function::iterator bbit = F.begin(), bbe=F.end(); bbit != bbe; ++bbit) {
    // For each instruction
    for (BasicBlock::iterator it = bbit->begin(), e=bbit->end(); it!=e;++it) {
      Type* tp = it->getType();
      // strip vector types
      if (VectorType* VT = dyn_cast<VectorType>(tp)) {
        tp = VT->getElementType();
      }
      // check that integer types are legal
      if (IntegerType* IT = dyn_cast<IntegerType>(tp)) {
        unsigned BW = IT->getBitWidth();
        if (BW > 64)
          return true;
      }
    }
  }
  return false;
}

bool CanVectorizeImpl::hasNonInlineUnsupportedFunctions(Function &F) {
  using namespace DPCPPKernelMetadataAPI;

  Module *pM = F.getParent();
  std::set<Function *> unsupportedFunctions;
  std::set<Function *> roots;

  // Add all kernels to root functions
  // Kernels assumes to have implicit barrier
  auto kernels = KernelList(pM);
  roots.insert(kernels.begin(), kernels.end());

  // Add all functions that contains synchronize/get_local_id/get_global_id to root functions
  CompilationUtils::FunctionSet oclFunction;

  //Get all synchronize built-ins declared in module
  CompilationUtils::getAllSyncBuiltinsDcls(oclFunction, pM);

  //Get get_local_id built-in if declared in module
  if ( Function *pF = pM->getFunction(CompilationUtils::mangledGetLID()) ) {
    oclFunction.insert(pF);
  }
  //Get get_global_id built-in if declared in module
  if ( Function *pF = pM->getFunction(CompilationUtils::mangledGetGID()) ) {
    oclFunction.insert(pF);
  }

  for ( CompilationUtils::FunctionSet::iterator fi = oclFunction.begin(), fe = oclFunction.end(); fi != fe; ++fi ) {
    Function *F = *fi;
    for (Function::user_iterator ui = F->user_begin(), ue = F->user_end(); ui != ue; ++ui ) {
      CallInst *CI = dyn_cast<CallInst> (*ui);
      if (!CI) continue;
      Function *pCallingFunc = CI->getParent()->getParent();
      roots.insert(pCallingFunc);
    }
  }

  // Fill unsupportedFunctions set with all functions that calls directly or undirectly
  // functions from the root functions set
  LoopUtils::fillFuncUsersSet(roots, unsupportedFunctions);
  return unsupportedFunctions.count(&F);
}

bool CanVectorizeImpl::hasDirectStreamCalls(Function &F, RuntimeServices* services) {
  Module *pM = F.getParent();
  bool isPointer64 = pM->getDataLayout().getPointerSizeInBits(0) == 64;
  std::set<Function *> streamFunctions;
  std::set<Function *> unsupportedFunctions;

  Function* readStreamFunc = ((OpenclRuntime*)services)->getReadStream(isPointer64);
  if (readStreamFunc) {
    // This returns the read stream function *from the runtime module*.
    // We need a function in *this* module with the same name.
    readStreamFunc = pM->getFunction(readStreamFunc->getName());
    if (readStreamFunc)
      streamFunctions.insert(readStreamFunc);
  }

  Function* writeStreamFunc = ((OpenclRuntime*)services)->getWriteStream(isPointer64);
  if (writeStreamFunc) {
    // This returns the write stream function *from the runtime module*.
    // We need a function in *this* module with the same name.
    writeStreamFunc = pM->getFunction(writeStreamFunc->getName());
    if (writeStreamFunc)
      streamFunctions.insert(writeStreamFunc);
  }

  // If we have stream functions in the module, don't vectorize their users.
  if (streamFunctions.size())
    LoopUtils::fillFuncUsersSet(streamFunctions, unsupportedFunctions);

  return unsupportedFunctions.count(&F);

}

bool CanVectorizeImpl::hasUnreachableInstructions(Function &F) {
  for (Function::iterator bbit = F.begin(), bbe = F.end(); bbit != bbe; ++bbit) {
    if (isa<UnreachableInst>(bbit->getTerminator()))
      return true;
  }
  return false;
}

} // namespace intel
