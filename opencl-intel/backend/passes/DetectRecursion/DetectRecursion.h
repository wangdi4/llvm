/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __INSTTOFUNCCALL_H__
#define __INSTTOFUNCCALL_H__
#include <llvm/Pass.h>
#include <llvm/Module.h>
#include "llvm/Analysis/CallGraph.h"
//===----------------------------------------------------------------------===//
//
// This pass detects recursions in the module.
// The detection is not performed for dead code
// e.g. user functions which are unused by any kernel (entry point)
//
//===----------------------------------------------------------------------===//

namespace intel{

  using namespace llvm;

  class DetectRecursion : public ModulePass
  {
  public:
    static char ID; // Pass identification, replacement for typeid

    DetectRecursion();

    bool runOnModule(Module &M);

		/// @brief LLVM Interface
		/// @param AU Analysis
		virtual void getAnalysisUsage(AnalysisUsage &AU) const {
		  // Depends on CallGraph for SCC
		  AU.addRequired<CallGraph>();
		}

    bool hasRecursion() { return m_recursionExists; }

    void print(raw_ostream &O, const Module *M) const;

	private:
		bool m_recursionExists;

		 /// function handling
		bool DetectRecursionInFunction(Function* fn);
  };
}

#endif
