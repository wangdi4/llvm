//===-------- HLNodeUtils.h - Utilities for HLNode class ----------*- C++ -*-===//
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


#ifndef LLVM_TRANSFORMS_INTEL_LOOPUTILS_HLNODEUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPUTILS_HLNODEUTILS_H

#include <set>

namespace llvm {

class BasicBlock;
class Instruction;

namespace loopopt {

class HLNode;
class HLRegion;
class HLSwitch;
class HLLabel;
class HLGoto;
class HLInst;
class HLIf;
class HLLoop;

class HLNodeUtils {
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
  
};

} // End namespace loopopt

} // End namespace llvm

#endif
