//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOAvrHIRCodeGen.h -- HIR Vector Code generation from AVR
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_VECOPT_VPOAVRHIRCODEGEN_H
#define LLVM_TRANSFORMS_VPO_VECOPT_VPOAVRHIRCODEGEN_H

#include <map>
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrGenerate.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

// AVRCodeGen generates vector code by widening of scalars into
// appropriate length vectors.
// TBI - In stress mode generate scalar code by cloning
// instructions.
class AVRCodeGenHIR {
public:
  AVRCodeGenHIR(AVR *Avr)
    : Avr(Avr), ALoop(nullptr), OrigLoop(nullptr), TripCount(0), VL(0) {}

  ~AVRCodeGenHIR() {}

  // Perform the actual loop widening (vectorization).
  bool vectorize();

private:
  AVR *Avr;

  // AVRLoop in AVR region
  AVRLoop *ALoop;

  // Original HIR loop corresponding to this Avr region
  HLLoop *OrigLoop;

  // Loop trip count
  unsigned int TripCount;

  // Vector factor or vector length to use. Each scalar instruction is widened
  // to operate on this number of operands.
  int VL;

  // Map of DDRef symbase and widened HLInst
  std::map<int, HLInst *> WidenMap;

  void setALoop(AVRLoop *L) { ALoop = L; }
  void setOrigLoop(HLLoop *L) { OrigLoop = L; }
  void setTripCount(unsigned int TC) { TripCount = TC; }
  void setVL(int V) { VL = V; }

  // Check for currently handled loops. Initial implementations
  // punts on seeing any control flow.
  bool loopIsHandled();
  void widenNode(const HLNode *Node, HLNode *Anchor);
  bool processLoop();
  bool unitStrideRef(const RegDDRef *Ref);

};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_TRANSFORMS_VPO_VECOPT_VPOAVRHIRCODEGEN_H
