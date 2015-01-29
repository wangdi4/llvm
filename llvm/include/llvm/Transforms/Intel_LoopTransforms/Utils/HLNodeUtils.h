//===-------- HLNodeUtils.h - Utilities for HLNode class --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the utilities for HLNode class.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLNODEUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_HLNODEUTILS_H

#include <set>
#include "llvm/Support/Compiler.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLSwitch.h"
#include "llvm/IR/Intel_LoopIR/HLLabel.h"
#include "llvm/IR/Intel_LoopIR/HLGoto.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"

namespace llvm {

class BasicBlock;
class Instruction;

namespace loopopt {

/// \brief Defines utilities for HLNode class
///
/// It contains a bunch of static member functions which manipulate HLNodes. 
/// It does not store any state.
///
class HLNodeUtils {
private:
  /// \brief Do not allow instantiation.
  HLNodeUtils() LLVM_DELETED_FUNCTION;

public:
  /// \brief Returns a new HLRegion.
  static HLRegion* createHLRegion(std::set< BasicBlock* >& OrigBBs, 
    BasicBlock* PredBB, BasicBlock* SuccBB);

  /// \brief Returns a new HLSwitch.
  static HLSwitch* createHLSwitch(HLNode* Par = nullptr);

  /// \brief Returns a new HLLabel.
  static HLLabel* createHLLabel(BasicBlock* SrcBB, HLNode* Par = nullptr);

  /// \brief Returns a new HLGoto.
  static HLGoto* createHLGoto(BasicBlock* TargetBB, HLLabel* TargetL = nullptr, 
    HLNode* Par = nullptr);

  /// \brief Returns a new HLInst.
  static HLInst* createHLInst(Instruction* In, HLNode* Par = nullptr);

  /// \brief Returns a new HLIf.
  static HLIf* createHLIf(HLNode* Par = nullptr);

  /// \brief Returns a new HLLoop.
  static HLLoop* createHLLoop(HLNode* Par = nullptr, HLIf* ZttIf = nullptr, 
    bool isDoWh = false, unsigned NumEx = 1);

  /// \brief Destroys the passed in HLNode.
  static void destroy(HLNode* Node);
  /// \brief Destroys all HLNodes. Should only be called after code gen.
  static void destroyAll();

  /// \brief Visits HLNodes in the forward direction in the range [begin, end).
  template<typename HV>
  static void forwardVisit(HV* Visitor, HLContainerTy::iterator Begin, 
    HLContainerTy::iterator End, bool Recursive = true);
  /// \brief Visits HLNodes in the backward direction in the range [begin, end).
  template<typename HV>
  static void backwardVisit(HV* Visitor, HLContainerTy::iterator Begin, 
    HLContainerTy::iterator End, bool Recursive = true);

  /// \brief Visits all HLNodes in the HIR in forward direction.
  template<typename HV>
  static void forwardVisitAll(HV* Visitor);
  /// \brief Visits all HLNodes in the HIR in backward direction.
  template<typename HV>
  static void backwardVisitAll(HV* Visitor); 
  
};

} // End namespace loopopt

} // End namespace llvm

#endif
