/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  SmartGVN.h

\*****************************************************************************/
#ifndef __SMART_GVN_H__
#define __SMART_GVN_H__

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/Analysis/LoopInfo.h>

namespace intel {

using namespace llvm;

/// @brief SmartGVN pass is used to configure and run GVN optimization pass
///  depending on  the function we are running optimization on. This pass is
///  intended to analyse function loops and disable "GVN-PRE for loads" if it's
///  considered to be non-profitable.

class SmartGVN : public ModulePass {
public:
  static char ID;
  /// @brief Constructor
  /// @param doNoLoadAnalysis enables SmartGVN pass heuristic which decides
  ///        if GVN should hoist load operations out of loops.
  /// @param memDependencyBBThreshold size of the basic block for memory denedency analysis.
  ///        if GVN should hoist load operations out of loops.
  SmartGVN(bool doNoLoadAnalysis = false);
  /// @brief execute pass on given module
  /// @param M module to optimize
  /// @returns True if module was modified
  virtual bool runOnModule(Module &M);
  /// @brief Provides name of pass
  virtual llvm::StringRef getPassName() const {
    return "Smart GVN";
  }
  /// @brief Inform about usage/mofication/dependency of this pass
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    if (noLoadAnalysis) {
      AU.addRequired<LoopInfoWrapperPass>();
    }
  }
protected:
  /// @brief Analyse the function body and returns true if disabling GVN-PRE
  /// for loads seems to be profitable. This will reduce register pressure in
  /// some cases.
  /// @param func  Function to analyse.
  bool isNoLoadsCandidate(Function *func);
  /// noLoadAnalysis - true if SmartGVN is supposed to do additional analysis
  /// and configure NoLoad parameter of GVN pass.
  bool noLoadAnalysis;
};

} // namespace intel

#endif // __SMART_GVN_H__

