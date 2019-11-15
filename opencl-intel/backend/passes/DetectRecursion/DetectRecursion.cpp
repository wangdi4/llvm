// INTEL CONFIDENTIAL
//
// Copyright 2012-2019 Intel Corporation.
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

#include "DetectRecursion.h"
#include "MetadataAPI.h"
#include "OCLPassSupport.h"

#include "llvm/ADT/SCCIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace Intel::MetadataAPI;

namespace intel {

char DetectRecursion::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(DetectRecursion, "detect-recursion",
                          "detects whether there are recursions", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
OCL_INITIALIZE_PASS_END(DetectRecursion, "detect-recursion",
                        "detects whether there are recursions", false, false)

DetectRecursion::DetectRecursion() : ModulePass(ID), m_recursionExists(false) {
  initializeDetectRecursionPass(*PassRegistry::getPassRegistry());
}

bool DetectRecursion::runOnModule(Module &M) {
  m_recursionExists = false;

  // for each function
  Module::FunctionListType &FL = M.getFunctionList();
  for (auto &F : FL) {
    if (DetectRecursionInFunction(&F)) {
      FunctionMetadataAPI(&F).RecursiveCall.set(true);
      m_recursionExists = true;
    }
  }
  return false;
}

bool DetectRecursion::DetectRecursionInFunction(Function *fn) {

  CallGraph &cg = getAnalysis<CallGraphWrapperPass>().getCallGraph();

  CallGraphNode *node = cg[&(*fn)];

  // Recursion exists if there is a cycle in the call graph. A directed graph
  // is acyclic if and only if it has no (nontrivial) strongly connected
  // subgraphs (because a cycle is strongly connected, and every strongly
  // connected graph contains at least one cycle).
  for (scc_iterator<CallGraphNode *> SCCI = scc_begin(node), E = scc_end(node);
       SCCI != E; ++SCCI) {
    const std::vector<CallGraphNode *> &nextSCC = *SCCI;
    if (nextSCC.size() > 1 || SCCI.hasLoop()) {
      return true;
    }
  }
  return false;
}

// print out results
void DetectRecursion::print(raw_ostream &O, const Module *M) const {
  using namespace Intel;

  if (m_recursionExists) {
    O << "DetectRecursion: Found recursive calls.\n";
    O << "DetectRecursion: Functions with recursive calls:\n";

    for (auto &F : *const_cast<Module *>(M)) {
      auto recursiveCallInfo = FunctionMetadataAPI(&F).RecursiveCall;
      if (recursiveCallInfo.hasValue() && recursiveCallInfo.get()) {
        O << F.getName() << ".\n";
      }
    }
  } else
    O << "DetectRecursion: No recursion found.\n";
}
} // namespace intel

extern "C" void *createDetectRecursionPass() {
  return new intel::DetectRecursion();
}
