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

#ifndef __INSTTOFUNCCALL_H__
#define __INSTTOFUNCCALL_H__

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
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
		  AU.addRequired<CallGraphWrapperPass>();
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
