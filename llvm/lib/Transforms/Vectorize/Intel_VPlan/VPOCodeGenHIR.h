//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOCodeGenHIR.h -- HIR Vector Code generation from VPlan
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPOCODEGENHIR_H
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPOCODEGENHIR_H

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HIRVisitor.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include <map>

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

using namespace llvm::loopopt;

namespace llvm { // LLVM Namespace

namespace loopopt {
class HIRSafeReductionAnalysis;
} // namespace loopopt

namespace vpo {
class VPValue;
class WRNVecLoopNode;

// VPOCodeGenHIR generates vector code by widening of scalars into
// appropriate length vectors.
class VPOCodeGenHIR {
public:
  VPOCodeGenHIR(TargetLibraryInfo *TLI, HIRSafeReductionAnalysis *SRA,
                Function &Fn, HLLoop *Loop, WRNVecLoopNode *WRLp)
      : TLI(TLI), SRA(SRA), Fn(Fn), OrigLoop(Loop), MainLoop(nullptr),
        CurMaskValue(nullptr), NeedRemainderLoop(false), TripCount(0), VF(0),
        WVecNode(WRLp) {}

  ~VPOCodeGenHIR() {}

  // Setup vector loop to perform the actual loop widening (vectorization) using
  // VF as the vectorization factor.
  void initializeVectorLoop(unsigned int VF);

  // Perform and cleanup/final actions after vectorizing the loop
  void finalizeVectorLoop(void);

  // Check if loop is currently suported by CodeGen.
  bool loopIsHandled(HLLoop *Loop, unsigned int VF);

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
  int getVF() const { return VF; };

  // Return true if Ref is a reduction
  bool isReductionRef(const RegDDRef *Ref, unsigned &Opcode);

  // Widen the given instruction to a vector instruction using VF
  // as the vector length. The given Mask value overrides the
  // current mask value if non-null.
  HLInst *widenNode(const HLInst *Inst, RegDDRef *Mask = nullptr);

  // Generate a wide compare using VF as the vector length. Only
  // single if predicates are currently handled. The given Mask value
  // overrides the current mask value if non-null.
  HLInst *widenIfPred(const HLIf *HIf, RegDDRef *Mask = nullptr);

  // Add WideVal as the widened vector value corresponding  to VPVal
  void addVPValueWideRefMapping(VPValue *VPVal, RegDDRef *WideVal) {
    VPValWideRefMap[VPVal] = WideVal;
  }

  // Return the widened vector value corresponding to VPVal if found
  // in VPValWideRefMap, return null otherwise.
  RegDDRef *getWideRefForVPVal(VPValue *VPVal) const {
    if (VPValWideRefMap.find(VPVal) != VPValWideRefMap.end())
      return VPValWideRefMap.at(VPVal);
    else
      return nullptr;
  }

  // Add the given instruction at the end of the main loop unmasked.
  // Currently used for predicate computation.
  void addInstUnmasked(HLInst *Inst) {
    HLNodeUtils::insertAsLastChild(MainLoop, Inst);
  }

  // Add the given instruction at the end of the main loop and mask
  // it with the provided mask value if non-null.
  void addInst(HLInst *Inst, RegDDRef *Mask) {
    if (Mask)
      Inst->setMaskDDRef(Mask->clone());
    HLNodeUtils::insertAsLastChild(MainLoop, Inst);
  }

  void setCurMaskValue(RegDDRef *V) { CurMaskValue = V; }

private:
  // Target Library Info is used to check for svml.
  TargetLibraryInfo *TLI;

  HIRSafeReductionAnalysis *SRA;

  // Current function
  Function &Fn;

  // Original HIR loop corresponding to this VPlan region, if a remainder loop
  // is needed after vectorization, the original loop is used as the remainder
  // loop after updating loop bounds.
  HLLoop *OrigLoop;

  // Main vector loop
  HLLoop *MainLoop;

  // Mask value to add for instructions being added to MainLoop
  RegDDRef *CurMaskValue;

  // Is a remainder loop needed?
  bool NeedRemainderLoop;

  // Loop trip count if constant. Set to zero for non-constant trip count loops.
  uint64_t TripCount;

  // Vector factor or vector length to use. Each scalar instruction is widened
  // to operate on this number of operands.
  unsigned VF;

  // Map of DDRef symbase and widened HLInst
  std::map<unsigned, HLInst *> WidenMap;
  std::map<VPValue *, RegDDRef *> VPValWideRefMap;

  // Map of avr number and widened DDRef. TODO - look into combining the two
  // maps
  std::map<int, RegDDRef *> WideMap;

  typedef DDRefGatherer<RegDDRef, TerminalRefs> BlobRefGatherer;

  BlobRefGatherer::MapTy MemRefMap;

  // WRegion VecLoop Node corresponding to AVRLoop
  WRNVecLoopNode *WVecNode;

  void setOrigLoop(HLLoop *L) { OrigLoop = L; }
  void setMainLoop(HLLoop *L) { MainLoop = L; }
  void setNeedRemainderLoop(bool NeedRem) { NeedRemainderLoop = NeedRem; }
  void setTripCount(uint64_t TC) { TripCount = TC; }
  void setVF(unsigned V) { VF = V; }

  // Erase loop intrinsics implementation - delete intel intrinsic directives
  // before/after the loop based on the BeginDir which determines where we start
  // the lookup.
  void eraseLoopIntrinsImpl(bool BeginDir);

  void eraseLoopIntrins();

  /// \brief Analyzes the memory references of \p OrigCall to determine
  /// stride. The resulting stride information is attached to the arguments
  /// of \p WideCall in the form of attributes.
  void analyzeCallArgMemoryReferences(const HLInst *OrigCall, HLInst *WideCall,
                                      SmallVectorImpl<RegDDRef *> &Args);

  // Given reduction operator identity value, insert vector reduction operand
  // initialization to a vector of length VF identity values. Return the
  // initialization instruction. The initialization is added before the loop
  // and the LVAL of this instruction is used as the widened reduction ref.
  HLInst *insertReductionInitializer(Constant *Iden);

  // Add entry to WidenMap and handle generating code for liveout/reduction at
  // the end of loop.
  void addToMapAndHandleLiveOut(const RegDDRef *ScalRef, HLInst *WideInst);

  // Find users of OrigRef and replaces them with NewRef.
  void replaceOrigRef(RegDDRef *OrigRef, RegDDRef *NewRef);

  // Replace math library calls in the remainder loop with the vectorized one
  // used in the main vector loop.
  void replaceLibCallsInRemainderLoop(HLInst *HInst);

  // Return widened instruction if Symbase is in WidenMap, return nullptr
  // otherwise.
  HLInst *getWideInst(unsigned Symbase) const {
    if (WidenMap.find(Symbase) != WidenMap.end())
      return WidenMap.at(Symbase);
    else
      return nullptr;
  }

  // Widen Ref if needed and return the widened ref.
  RegDDRef *widenRef(const RegDDRef *Ref);

  class HIRLoopVisitor : public HIRVisitor<HIRLoopVisitor> {
  private:
    VPOCodeGenHIR *CG;
    SmallVector<HLInst *, 1> CallInsts;

  public:
    HIRLoopVisitor(HLLoop *L, VPOCodeGenHIR *CG) : CG(CG) { visitLoop(L); }
    void visitLoop(HLLoop *L);
    void visitIf(HLIf *If);
    void visitInst(HLInst *Inst);
    void replaceCalls();
  };
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPOCODEGENHIR_H
