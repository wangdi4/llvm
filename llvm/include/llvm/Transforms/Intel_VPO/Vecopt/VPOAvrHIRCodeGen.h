//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015 - 2016 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer Namespace

// ReductionHIRMngr keeps information about reduction variables.
class ReductionHIRMngr {
public:
  ReductionHIRMngr(AVR *Avr);

  /// \brief Return true if the given \p Val is marked up as reduction.
  bool isReductionVariable(const Value *Val);

  /// \brief Return reduction information for the given \p Val
  ReductionItem *getReductionInfo(const Value *Val);

  /// \brief Return address, where initial value is stored.
  const RegDDRef *getReductionValuePtr(ReductionItem *RI);

  /// \brief Build internal maps for the given \p Region
  void mapHLNodes(const HLRegion *Region);

  /// \brief Return identity vector corresponding to the recurrence kind.
  /// The recurrence kind is taken from \p RI. \p VL - vector length of
  /// the identity vector to be created. \Ty - scalar data type, float
  /// or integer.
  static RegDDRef *getRecurrenceIdentityVector(ReductionItem *RI,
                                               Type *Ty, unsigned VL);

private:
  // Reduction map
  typedef std::map<const Value *, ReductionItem *> ValueToRedItemMap;
  ValueToRedItemMap ReductionMap;

  typedef std::map<ReductionItem *, const RegDDRef *> InitializersMap;
  InitializersMap Initializers;
};

// AVRCodeGen generates vector code by widening of scalars into
// appropriate length vectors.
// TBI - In stress mode generate scalar code by cloning
// instructions.
class AVRCodeGenHIR {
public:
  AVRCodeGenHIR(AVR *Avr)
    : Avr(Avr), ALoop(nullptr), OrigLoop(nullptr), TripCount(0), VL(0),
      RHM(Avr) {}

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

  typedef DDRefGatherer<RegDDRef, TerminalRefs> BlobRefGatherer;

  BlobRefGatherer::MapTy MemRefMap;

  // Reductions
  ReductionHIRMngr RHM;

  void setALoop(AVRLoop *L) { ALoop = L; }
  void setOrigLoop(HLLoop *L) { OrigLoop = L; }
  void setTripCount(unsigned int TC) { TripCount = TC; }
  void setVL(int V) { VL = V; }

  // Check for currently handled loops. Initial implementations
  // punts on seeing any control flow.
  bool loopIsHandled();
  void widenNode(const HLNode *Node, HLNode *Anchor);
  RegDDRef *getVectorValue(const RegDDRef *Op);
  HLInst *widenReductionNode(const HLNode *Node, HLNode *Anchor);
  bool processLoop();
  bool unitStrideRef(const RegDDRef *Ref);
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_TRANSFORMS_VPO_VECOPT_VPOAVRHIRCODEGEN_H
