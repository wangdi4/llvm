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

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"
#include <map>

#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

namespace llvm { // LLVM Namespace

namespace loopopt {
class HIRSafeReductionAnalysis;
} // End loopopt namespace

namespace vpo { // VPO Vectorizer Namespace

class WRNVecLoopNode;

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

  /// \brief Build internal maps for the given loop \p OrigLoop.
  void mapHLNodes(const HLLoop *OrigLoop);

  /// \brief Return identity value corresponding to the recurrence kind.
  /// The recurrence kind is taken from \p RI. \Ty - scalar data type, float
  /// or integer.
  static Constant *getRecurrenceIdentity(ReductionItem *RI, Type *Ty);

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
  AVRCodeGenHIR(AVR *Avr, HIRSafeReductionAnalysis *SRA)
      : Avr(Avr), SRA(SRA), ALoop(nullptr), OrigLoop(nullptr),
        MainLoop(nullptr), NeedRemainderLoop(false), TripCount(0), VL(0),
        RHM(Avr) {}

  ~AVRCodeGenHIR() {}

  // Perform the actual loop widening (vectorization) using VL as the
  // vectorization factor.
  bool vectorize(unsigned int VF);

  // Check if loop is currently suported by AVRCodeGen.
  bool loopIsHandled(unsigned int VF);

  // Return the cost of remainder loop code, if a remainder loop is needed.
  int getRemainderLoopCost(HLLoop *Loop, unsigned int VF,
                           unsigned int &TripCount);

  // Return true if \p Ref is a constant stride reference at loop
  // nesting level \p Level. Return stride coefficient in \p CoeffPtr
  // if not null.
  static bool isConstStrideRef(const RegDDRef *Ref, unsigned Level,
                               int64_t *CoeffPtr = nullptr);

private:
  AVR *Avr;
  HIRSafeReductionAnalysis *SRA;

  // AVRLoop in AVR region
  AVRLoop *ALoop;

  // Original HIR loop corresponding to this Avr region, if a remainder loop is
  // needed after vectorization, the original loop is used as the remainder loop
  // after updating loop bounds.
  HLLoop *OrigLoop;

  // Main vector loop
  HLLoop *MainLoop;

  // Is a remainder loop needed?
  bool NeedRemainderLoop;

  // Loop trip count if constant. Set to zero for non-constant trip count loops.
  uint64_t TripCount;

  // Vector factor or vector length to use. Each scalar instruction is widened
  // to operate on this number of operands.
  int VL;

  // Map of DDRef symbase and widened HLInst
  std::map<int, HLInst *> WidenMap;

  typedef DDRefGatherer<RegDDRef, TerminalRefs> BlobRefGatherer;

  BlobRefGatherer::MapTy MemRefMap;

  // Reductions
  ReductionHIRMngr RHM;

  // WRegion VecLoop Node corresponding to AVRLoop
  WRNVecLoopNode *WVecNode;

  void setALoop(AVRLoop *L) { ALoop = L; }
  void setOrigLoop(HLLoop *L) { OrigLoop = L; }
  void setMainLoop(HLLoop *L) { MainLoop = L; }
  void setNeedRemainderLoop(bool NeedRem) { NeedRemainderLoop = NeedRem; }
  void setTripCount(uint64_t TC) { TripCount = TC; }
  void setVL(int V) { VL = V; }
  void setWVecNode(WRNVecLoopNode *Node) { WVecNode = Node; }

  // Check for currently handled loops. Initial implementations
  // punts on seeing any control flow.
  // The output parameter \p TripCount holds the tripCount of the loop if it is
  // a constant, zero otherwise.
  bool loopIsHandledImpl(int64_t &TripCount);

  void widenNode(const HLNode *Node);
  RegDDRef *getVectorValue(const RegDDRef *Op);
  HLInst *widenReductionNode(const HLNode *Node);

  // Erase loop intrinsics implementation - delete intel intrinsic directives
  // before/after the loop based on the BeginDir which determines where we start
  // the lookup.
  void eraseLoopIntrinsImpl(bool BeginDir);

  void eraseLoopIntrins();
  void processLoop();
  RegDDRef *widenRef(const RegDDRef *Ref);

  // Return true if the loop is a small loop(atmost 2 instructions)
  // with add reduction of 16-bit integer values into 32/64 bit
  // integer value.
  bool isSmallShortAddRedLoop();

  // Given reduction operator identity value, insert vector reduction operand
  // initialization to a vector of length VL identity values. Return the
  // initialization instruction. The initialization is added before the loop
  // and the LVAL of this instruction is used as the widened reduction ref.
  HLInst *insertReductionInitializer(Constant *Iden);

  // Add entry to WidenMap and handle generating code for liveout/reduction at
  // the end of loop.
  void addToMapAndHandleLiveOut(const RegDDRef *ScalRef, HLInst *WideInst);
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_TRANSFORMS_VPO_VECOPT_VPOAVRHIRCODEGEN_H
