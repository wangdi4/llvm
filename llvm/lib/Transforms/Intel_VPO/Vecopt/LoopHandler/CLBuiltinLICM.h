/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

// This pass is a work around to overcome problematic behaviour of LICM.
// LICM does not hoist calls instruction that are under if since they
// might not halt (all calls fails safeToSpeculativeExecute).
// This pass hoists built-ins that we know that are safe to hoist and have all
// argumnets invariant.
// The main drawback of this pass is that it may depend on LICM to hoist
// instructions in order to hoist the built-ins calls (LICM might make the
// arguments invariant) but LICM depends on it in order to deduce that the call
// result is invariant. However, it capture cases like get_image_height etc.. 
// which are the main reasons for it.
// A better solution is to add an attribute to function calls, and modify LICM 
// to hoist these call by itself, and also to add AliasAnalysis that will
// correctly deduce alias of image built-ins.
// Author: Ran Chachick

#ifndef __CL_BUILTIN_LICM_H_
#define __CL_BUILTIN_LICM_H_

#include "BuiltinLibInfo.h"
#include "OpenclRuntime.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Instructions.h"
using namespace llvm;
namespace intel {
class CLBuiltinLICM : public LoopPass {
public:
  ///@brief Pass identification.
  static char ID;

  /// @brief C'tor.
  CLBuiltinLICM();

  /// @brief destructor.
  ~CLBuiltinLICM() {}

  /// @brief LLVM interface.
  /// @param L - Loop to analyze.
  /// @param LPM - Loop Pass manager (unused).
  /// @returns true if the pass made changes.
  virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

  /// @brief LLVM interface.
  /// @param AU - usage of analysis.
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DominatorTree>();
    AU.addRequired<LoopInfo>();
    AU.addRequired<BuiltinLibInfo>();
    AU.setPreservesCFG();
  };



private:

  /// @brief computes the stride dependency of instruction in the basic block 
  ///        represented by the node, and recursively calls his children.
  /// @param N - dominator tree node of the current processed basic block.
  void ScanLoop(DomTreeNode *N);

  ///@brief hoist the call instruction if it is a known to have no side 
  ///       effect and  all it's operands are invariant.
  ///@param CI - call candidate for hoisting.
  bool hoistCLBuiltin(CallInst *CI);

  /// @brief current loop.
  Loop *m_curLoop;

  /// @brief dominator tree analysis.
  DominatorTree *m_DT;

  /// @breif pre header of the current loop.
  BasicBlock *m_preHeader;

  /// @brief header of the current loop.
  BasicBlock *m_header;

  /// @brief true if any call instruction was hoisted.
  bool m_changed;

  /// @brief tuntime serivces object.
  const OpenclRuntime *m_rtServices;

};
}

#endif //define __CL_BUILTIN_LICM_H_