//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
///  \file
///  VPOAvrLoopIR.h -- Defines the Abstract Vector Representation (AVR) loop
///  node for LLVM IR.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_LOOP_IR_H
#define LLVM_ANALYSIS_VPO_AVR_LOOP_IR_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrLoop.h"
#include "llvm/Analysis/ScalarEvolution.h"

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

/// \brief Loop node abstract vector representation
///
/// An AVRLoopIR node represents a loop found in LLVM IR.
class AVRLoopIR : public AVRLoop {

private:

  /// Pointer to original LLVM loop.
  const Loop *LLVMLoop;

//TODO: Currently investigating the best way to represent and maintain loop
//      info. Some loop info can simply be computed using existing LLVM
//      utilities (e.g., trip count) and would avoid storing in the AVR.
//      Other info we will want cached in the AVR (e.g., loopIVs).

  /// LoopIVs and strides
  SmallDenseMap<Instruction*, const SCEV*> LoopIVs;

  /// Loop upper bound
  SCEV *UpperBound;

  /// Loop lower bound
  SCEV *LowerBound;

protected:

  // Interface to create AVRLoop from LLVM Loop.
  AVRLoopIR(Loop *Lp);

  // AvrLoop copy constructor.
  AVRLoopIR(const AVRLoopIR &AVROrigLoop);

  virtual ~AVRLoopIR() override {}

  /// \brief Set Orig LLVM Loop
  void setLoop(const Loop *Lp) { LLVMLoop = Lp; }

  /// \brief Set loop lower bound
  void setLowerBound(SCEV *Lb) { LowerBound = Lb; }

  /// \brief Set loop upper bound
  void setUpperBound(SCEV *Ub) { UpperBound = Ub; }

  /// Only this utility class should be use to modify/delete AVR nodes.
  friend class AVRUtilsIR;

public:

  AVRLoopIR *clone() const override;

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const AVR *Node) {
    return Node->getAVRID() == AVR::AVRLoopIRNode;
  }

  /// \brief Returns Original LLVM Loop
  const Loop *getLoop() const { return LLVMLoop; }

  /// \brief Get loop lower bound
  SCEV* getLowerBound(Value *Lb) { return LowerBound; }

  /// \brief Get loop upper bound
  SCEV* getUpperBound(Value *Ub) { return UpperBound; }

  /// \brief Set loop induction variables and associated strides from the
  /// original LLVM loop.
  void setLoopIV(Instruction *I, const SCEV *S) {
    LoopIVs.insert(std::pair<Instruction *, const SCEV*>(I, S));
  }

  /// \brief Returns LoopIVs and corresponding strides.
  //SmallDenseMap<Instruction*, const SCEV*>& getLoopIVs() { return LoopIVs; }

  /// \brief Prints the AvrLoop node.
  void print(formatted_raw_ostream &OS, unsigned Depth,
             VerbosityLevel VLevel) const override;

  /// \brief Returns a constant StringRef for the type name of this node.
  virtual StringRef getAvrTypeName() const override;

  /// \brief Returns the value name of this node.
  virtual std::string getAvrValueName() const override;

  /// \brief Code generation for AVR loop.
  void codeGen() override;
};


} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif  // LLVM_ANALYSIS_VPO_AVR_LOOP_IR_H
