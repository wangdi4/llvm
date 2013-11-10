/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "DetectRecursion.h"
#include "OCLPassSupport.h"
#include "MetaDataApi.h"

#include "llvm/ADT/SCCIterator.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace intel{

  char DetectRecursion::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(DetectRecursion, "detect-recursion", "detects whether there are recursions", false, false)
  OCL_INITIALIZE_AG_DEPENDENCY(CallGraph)
  OCL_INITIALIZE_PASS_END(DetectRecursion, "detect-recursion", "detects whether there are recursions", false, false)

  DetectRecursion::DetectRecursion() : ModulePass(ID), m_recursionExists(false) {
   initializeDetectRecursionPass(*PassRegistry::getPassRegistry());
  }

  bool DetectRecursion::runOnModule(Module &M){
    m_recursionExists = false;

    Intel::MetaDataUtils mdUtils(&M);

    // for each function
    Module::FunctionListType &FL = M.getFunctionList();
    for (Module::iterator fn = FL.begin(), fne = FL.end(); fn != fne; ++fn){
      if(DetectRecursionInFunction(fn)){
        mdUtils.getOrInsertFunctionsInfoItem(fn)->setHasRecursion(true);
        m_recursionExists = true;
      }
    }
    if(m_recursionExists){
      mdUtils.save(M.getContext());
    }
    return false;
  }

  bool DetectRecursion::DetectRecursionInFunction(Function* fn){

    CallGraph& cg = getAnalysis<CallGraph>();

    CallGraphNode * node = cg[&(*fn)];

    // Recursion exists if there is a cycle in the call graph. A directed graph
    // is acyclic if and only if it has no (nontrivial) strongly connected
    // subgraphs (because a cycle is strongly connected, and every strongly
    // connected graph contains at least one cycle).
    for (scc_iterator<CallGraphNode*> SCCI = scc_begin(node), E = scc_end(node);
            SCCI != E; ++SCCI) {
      const std::vector<CallGraphNode*> &nextSCC = *SCCI;
      if (nextSCC.size() > 1 || SCCI.hasLoop()) {
        return true;
      }
    }
    return false;
  }

  // print out results
  void DetectRecursion::print(raw_ostream &O, const Module *M) const{
    using namespace Intel;

    if(m_recursionExists){
      O << "DetectRecursion: Found recursive calls.\n";
      O << "DetectRecursion: Functions with recursive calls:\n";

      MetaDataUtils mdUtils(const_cast<Module*>(M));
      MetaDataUtils::FunctionsInfoMap::iterator i = mdUtils.begin_FunctionsInfo();
      MetaDataUtils::FunctionsInfoMap::iterator e = mdUtils.end_FunctionsInfo();
      for(; i != e; ++i )
      {
        llvm::Function * pFunc = i->first;
        Intel::FunctionInfoMetaDataHandle kimd = i->second;
        if(kimd->isHasRecursionHasValue() && kimd->getHasRecursion()){
          O << pFunc->getName() << ".\n";
        }
      }
    }
    else
      O << "DetectRecursion: No recursion found.\n";
  }
} // intel namespace

extern "C" void* createDetectRecursionPass() {
	return new intel::DetectRecursion();
}
