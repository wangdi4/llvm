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
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrIfHIR.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include <map>

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

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
  AVRCodeGenHIR(AVR *Avr, TargetLibraryInfo *TLI, HIRSafeReductionAnalysis *SRA,
                Function &Fn)
      : Avr(Avr), TLI(TLI), SRA(SRA), Fn(Fn), ALoop(nullptr), OrigLoop(nullptr),
        MainLoop(nullptr), NeedRemainderLoop(false), TripCount(0), VL(0),
        RHM(Avr), WVecNode(nullptr) {}

  ~AVRCodeGenHIR() {}

  // Perform the actual loop widening (vectorization) using VL as the
  // vectorization factor.
  bool vectorize(unsigned int VF);

  // Check if loop is currently suported by AVRCodeGen.
  bool loopIsHandled(unsigned int VF);

  // Return the trip count for the scalar loop. Returns 0 for unknown trip
  // count loops
  uint64_t getTripCount() const { return TripCount; }

  // Return true if \p Ref is a constant stride reference at loop
  // nesting level \p Level. Return stride coefficient in \p CoeffPtr
  // if not null.
  static bool isConstStrideRef(const RegDDRef *Ref, unsigned Level,
                               int64_t *CoeffPtr = nullptr);

  Function &getFunction() const { return Fn; }
  HLLoop *getMainLoop() const { return MainLoop; }
  int getVL() const { return VL; };

  // Return widened instruction if Symbase is in WidenMap, return nullptr
  // otherwise.
  HLInst *findWideInst(unsigned Symbase) {
    if (WidenMap.find(Symbase) != WidenMap.end())
      return WidenMap[Symbase];
    else
      return nullptr;
  }

  void setWideAvrRef(int AvrNum, RegDDRef *Ref) { AvrWideMap[AvrNum] = Ref; }

  // Return widened ref if AvrNum is in AvrWideMap, return nullptr
  // otherwise.
  RegDDRef *findWideAvrRef(int AvrNum) {
    if (AvrWideMap.find(AvrNum) != AvrWideMap.end())
      return AvrWideMap[AvrNum];
    else
      return nullptr;
  }

  // Return widened ref for the given avr number
  RegDDRef *getWideAvrRef(int AvrNum) {
    auto WRef = findWideAvrRef(AvrNum);
    assert(WRef && "Expected to find widened ref for avr");
    return WRef;
  }

  // Widen Ref if needed and return the widened ref.
  RegDDRef *widenRef(const RegDDRef *Ref);

  // Return true if Ref is a reduction
  bool isReductionRef(const RegDDRef *Ref, unsigned &Opcode);

private:
  AVR *Avr;

  // Target Library Info is used to check for svml.
  TargetLibraryInfo *TLI;

  HIRSafeReductionAnalysis *SRA;

  // Current function
  Function &Fn;

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

  // Map of avr number and widened DDRef. TODO - look into combining the two
  // maps
  std::map<int, RegDDRef *> AvrWideMap;

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

  HLInst *widenNode(AVRAssignHIR *AvrNode);
  RegDDRef *getVectorValue(const RegDDRef *Op);
  HLInst *widenReductionNode(const HLNode *Node);

  // Erase loop intrinsics implementation - delete intel intrinsic directives
  // before/after the loop based on the BeginDir which determines where we start
  // the lookup.
  void eraseLoopIntrinsImpl(bool BeginDir);

  void eraseLoopIntrins();
  void processLoop();

  /// \brief Analyzes the memory references of \p OrigCall to determine
  /// stride. The resulting stride information is attached to the arguments
  /// of \p WideCall in the form of attributes.
  void analyzeCallArgMemoryReferences(const HLInst *OrigCall, HLInst *WideCall,
                                      SmallVectorImpl<RegDDRef *> &Args);

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
