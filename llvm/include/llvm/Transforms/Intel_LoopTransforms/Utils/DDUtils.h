//===-------- DDUtils.h - Utilities for DD  -------------------------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the utilities for DDUtils class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDUTILS_H
#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRUtils.h"

namespace llvm {
namespace loopopt {

/// \brief Defines utilities for DDUtils Class
///
/// It contains a functions which analyze and manipulate DD
/// It does not store any state.
///
class HIRFramework;
class DDGraph;

class DDUtils : public HIRUtils {
private:
  /// \brief Do not allow instantiation
  DDUtils() = delete;
  friend class HIRParser;
  friend class HLNodeUtils;

  /// \brief Return true if a load can move into the loop
  ///  t0 = A[i1]; loop { };
  ///  In some case, moving a load into a loop requires a corresponding store
  ///  A[i1] = t0 to be moved into the loop also as in the case of sum reduction
  static bool canMoveLoadIntoLoop(const DDRef *Lref, const DDRef *Rref,
                                  HLLoop *InnermostLoop,
                                  SmallVectorImpl<HLInst *> &PostLoopInsts,
                                  HLInst **StoreInst, DDGraph DDG);
  /// \brief Return true if a corresponding load is found
  static bool findLoadInst(const DDRef *RRef,
                           SmallVectorImpl<HLInst *> &PreLoopInsts,
                           DDGraph DDG);

  // \Brief Gather Pre / Post Nodes in Vectors
  // Called from EnablePerfectLoopNest Util
  // Return false if unwanted nodes are encountered (e.g  if)
  static bool
  enablePerfectLPGatherPrePostInsts(HLLoop *InnermostLoop, DDGraph DDG,
                                    SmallVectorImpl<HLInst *> &PreLoopInsts,
                                    SmallVectorImpl<HLInst *> &PostLoopInsts,
                                    SmallVectorImpl<HLInst *> &ForwardSubInsts);

  // \Brief Legality Check for nodes before Loop.
  // Called from EnablePerfectLoopNest Util.
  // Return true if legal
  static bool
  enablePerfectLPLegalityCheckPre(HLLoop *InnermostLoop, DDGraph DDG,
                                  SmallVectorImpl<HLInst *> &PreLoopInsts,
                                  SmallVectorImpl<HLInst *> &PostLoopInsts,
                                  SmallVectorImpl<HLInst *> &ForwardSubInsts,
                                  SmallVectorImpl<HLInst *> &ValidatedStores);

  // \Brief Legality Check for nodes after Loop.
  // Called from EnablePerfectLoopNest Util.
  // Return true if legal
  static bool
  enablePerfectLPLegalityCheckPost(HLLoop *InnermostLoop, DDGraph DDG,
                                   SmallVectorImpl<HLInst *> &PostLoopInsts,
                                   SmallVectorImpl<HLInst *> &ValidatedStores);

public:
  /// \brief Any incoming/outgoing edge into Loop?
  static bool anyEdgeToLoop(DDGraph DDG, const DDRef *Ref, HLLoop *Loop);

  ///  \brief Update the linearity of DDRef when it becomes part of the
  ///  innermost loop
  ///  (as a result of ld/st movement or complete unrolling)
  ///  Current code only work on stmts inside the innermost loop
  static void updateDDRefsLinearity(SmallVectorImpl<HLInst *> &HLInsts,
                                    DDGraph DDG);

  /// \brief  Enables Perfect Loop Nests
  /// Takes care of simple cases that are needed for Interchange
  static bool enablePerfectLoopNest(HLLoop *InnermostLoop, DDGraph DDG);
};
} // End namespace loopopt
} // End namespace llvm

#endif
