//===-- DetectRecursion.cpp -----------------------------------------------===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DetectRecursion.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;
using namespace DPCPPKernelMetadataAPI;

#define DEBUG_TYPE "dpcpp-kernel-detect-recursion"

PreservedAnalyses DetectRecursionPass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  CallGraph *CG = &AM.getResult<CallGraphAnalysis>(M);
  if (!runImpl(M, CG))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

static bool detectRecursionInFunction(Function *Fn, CallGraph *CG) {
  CallGraphNode *Node = (*CG)[&(*Fn)];

  // Recursion exists if there is a cycle in the call graph. A directed graph
  // is acyclic if and only if it has no (nontrivial) strongly connected
  // subgraphs (because a cycle is strongly connected, and every strongly
  // connected graph contains at least one cycle).
  for (scc_iterator<CallGraphNode *> SCCI = scc_begin(Node), E = scc_end(Node);
       SCCI != E; ++SCCI) {
    const std::vector<CallGraphNode *> &NextScc = *SCCI;
    if (NextScc.size() > 1 || SCCI.hasCycle())
      return true;
  }
  return false;
}

bool DetectRecursionPass::runImpl(Module &M, CallGraph *CG) {
  bool RecursionExists = false;
  // for each function
  Module::FunctionListType &FL = M.getFunctionList();
  for (auto &F : FL) {
    if (!F.isDeclaration() &&
        detectRecursionInFunction(&F, CG)) {
      FunctionMetadataAPI(&F).RecursiveCall.set(true);
      RecursionExists = true;
    }
  }
  return RecursionExists;
}
