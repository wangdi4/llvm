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

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPOCODEGENHIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPOCODEGENHIR_H

#include "../IntelVPlanValue.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HIRVisitor.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/TargetLibraryInfo.h"

using namespace llvm::loopopt;

namespace llvm { // LLVM Namespace

namespace loopopt {
class HIRSafeReductionAnalysis;
} // namespace loopopt

namespace vpo {
class WRNVecLoopNode;

// VPOCodeGenHIR generates vector code by widening of scalars into
// appropriate length vectors.
class VPOCodeGenHIR {
public:
  VPOCodeGenHIR(TargetLibraryInfo *TLI, HIRSafeReductionAnalysis *SRA,
                Function &Fn, HLLoop *Loop, LoopOptReportBuilder &LORB,
                WRNVecLoopNode *WRLp, const bool IsSearchLoop)
      : TLI(TLI), SRA(SRA), Fn(Fn), OrigLoop(Loop), PeelLoop(nullptr),
        MainLoop(nullptr), CurMaskValue(nullptr), NeedRemainderLoop(false),
        TripCount(0), VF(0), LORBuilder(LORB), WVecNode(WRLp),
        IsSearchLoop(IsSearchLoop) {}

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

  Function &getFunction() const { return Fn; }
  HLLoop *getOrigLoop() const { return OrigLoop; }
  HLLoop *getMainLoop() const { return MainLoop; }
  int getVF() const { return VF; };
  bool getNeedRemainderLoop() const { return NeedRemainderLoop; }
  HLLoop *getRemainderLoop() const { return OrigLoop; }

  // Return true if Ref is a reduction
  bool isReductionRef(const RegDDRef *Ref, unsigned &Opcode);

  // Returns the HLLoop where it is safe to hoist the reduction initializer
  // statement. It is also the point after which the last value computation
  // instructions can be placed.
  HLLoop *findRednHoistInsertionPoint(HLLoop *Lp);

  // Widen the given instruction to a vector instruction using VF
  // as the vector length. The given Mask value overrides the
  // current mask value if non-null.
  HLInst *widenNode(const HLInst *Inst, RegDDRef *Mask = nullptr);

  HLInst *handleLiveOutLinearInEarlyExit(HLInst *Inst, RegDDRef *Mask);

  /// Collect live-out definitions reaching \p Goto's parent and insert a copy
  /// of them in the current insertion point of the main vector loop. Linear
  /// references in the live-out copies are shifted by an offset corresponding
  /// to the first vector lane taking the early exit. \p Goto must be an early
  /// exit of the loop we are vectorizing. \p Goto's parent must be an HLIf.
  void handleNonLinearEarlyExitLiveOuts(const HLGoto *Goto);

  HLInst *createBitCast(Type *Ty, RegDDRef *Ref,
                        const Twine &Name = "cast") {
    HLInst *BitCastInst =
        MainLoop->getHLNodeUtils().createBitCast(Ty, Ref->clone(), Name);
    addInstUnmasked(BitCastInst);
    return BitCastInst;
  }

  HLInst *createZExt(Type *Ty, RegDDRef *Ref,
                     const Twine &Name = "cast") {
    HLInst *ZExtInst =
        MainLoop->getHLNodeUtils().createZExt(Ty, Ref->clone(), Name);
    addInstUnmasked(ZExtInst);
    return ZExtInst;
  }

  HLInst *createCTTZCall(RegDDRef *Ref, const Twine &Name = "bsf");

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
    auto Itr = VPValWideRefMap.find(VPVal);
    if (Itr != VPValWideRefMap.end())
      return Itr->second;
    else
      return nullptr;
  }

  // Add WideVal as the widened vector value corresponding  to SCVal
  void addSCEVWideRefMapping(const SCEV *SCVal, RegDDRef *WideVal) {
    SCEVWideRefMap[SCVal] = WideVal;
  }

  // Return the widened vector value corresponding to SCVal if found
  // in SCEVWideRefMap, return null otherwise.
  RegDDRef *getWideRefForSCVal(const SCEV *SCVal) const {
    auto Itr = SCEVWideRefMap.find(SCVal);
    if (Itr != SCEVWideRefMap.end())
      return Itr->second;
    else
      return nullptr;
  }

  // Add the given instruction at the end of the main loop unmasked.
  // Currently used for predicate computation.
  void addInstUnmasked(HLInst *Inst) {
    addInst(Inst, nullptr);
  }

  void addInstUnmasked(HLInst *Inst, const bool IsThenChild) {
    addInst(Inst, nullptr, IsThenChild);
  }

  // Add the given instruction at the end of the main loop and mask
  // it with the provided mask value if non-null.
  void addInst(HLNode *Node, RegDDRef *Mask) {
    if (Mask) {
      HLInst *Inst = dyn_cast<HLInst>(Node);
      assert(Inst && "Only HLInst can have a mask.");
      Inst->setMaskDDRef(Mask->clone());
    }

    if (auto InsertRegion = dyn_cast<HLLoop>(getInsertRegion()))
      HLNodeUtils::insertAsLastChild(InsertRegion, Node);
    else if (isa<HLIf>(getInsertRegion()))
      addInst(Node, Mask, true);
  }

  // Insert instruction into HLIf region.
  void addInst(HLNode *Node, RegDDRef *Mask, const bool IsThenChild) {
    // Simply put MaskDDRef on each instruction under if. For innermost uniform
    // predicates it's responsibility of predicator to remove unnecessary
    // predicates.
    if (Mask) {
      HLInst *Inst = dyn_cast<HLInst>(Node);
      assert(Inst && "Only HLInst can have a mask.");
      Inst->setMaskDDRef(Mask->clone());
    }
    auto InsertRegion = dyn_cast<HLIf>(getInsertRegion());
    assert(InsertRegion && "HLIf is expected as insert region.");
    HLNodeUtils::insertAsLastChild(InsertRegion, Node, IsThenChild);
  }

  void setCurMaskValue(RegDDRef *V) { CurMaskValue = V; }

  // Return widened instruction if Symbase is in WidenMap, return nullptr
  // otherwise.
  HLInst *getWideInst(unsigned Symbase) const {
    auto Itr = WidenMap.find(Symbase);
    if (Itr != WidenMap.end())
      return Itr->second;
    else
      return nullptr;
  }

  void addUnitStrideRef(const RegDDRef *Ref) { UnitStrideRefSet.insert(Ref); }

  bool isUnitStrideRef(const RegDDRef *Ref) {
    return UnitStrideRefSet.count(Ref);
  }

  // Widen Ref if needed and return the widened ref.
  RegDDRef *widenRef(const RegDDRef *Ref);

  // Delete intel intrinsic directives before and after the loop.
  void eraseLoopIntrins();

  bool isSearchLoop() const { return IsSearchLoop; }

  HLDDNode *getInsertRegion() const { return InsertRegionsStack.back(); }
  void addInsertRegion(HLDDNode *Node) { InsertRegionsStack.push_back(Node); }
  void popInsertRegion() { InsertRegionsStack.pop_back(); }

  void createHLIf(const CmpInst::Predicate Pred, RegDDRef *LhsRef,
                  RegDDRef *RhsRef) {
    HLDDNode *If = MainLoop->getHLNodeUtils().createHLIf(Pred, LhsRef, RhsRef);
    addInst(If, nullptr);
    addInsertRegion(If);
  }

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

  // Peel loop
  HLLoop *PeelLoop;

  // Main vector loop
  HLLoop *MainLoop;

  // Mask value to add for instructions being added to MainLoop
  RegDDRef *CurMaskValue;

  // Is first iteration peel loop needed?
  bool NeedFirstItPeelLoop = false;

  // Is a remainder loop needed?
  bool NeedRemainderLoop;

  // Loop trip count if constant. Set to zero for non-constant trip count loops.
  uint64_t TripCount;

  // Vector factor or vector length to use. Each scalar instruction is widened
  // to operate on this number of operands.
  unsigned VF;

  LoopOptReportBuilder &LORBuilder;

  // Map of DDRef symbase and widened HLInst
  DenseMap<unsigned, HLInst *> WidenMap;
  DenseMap<VPValue *, RegDDRef *> VPValWideRefMap;

  // Map of SCEV expression and widened DDRef.
  DenseMap<const SCEV *, RegDDRef *> SCEVWideRefMap;

  // The loop for which it is safe to hoist the reduction initializer and sink
  // reduction last value compute instructions.
  HLLoop *RednHoistLp;

  // WRegion VecLoop Node corresponding to AVRLoop
  WRNVecLoopNode *WVecNode;

  // Set of unit-stride Refs
  SmallPtrSet<const RegDDRef *, 4> UnitStrideRefSet;
  // The loop meets search loop idiom criteria.
  bool IsSearchLoop;
  SmallVector<HLDDNode *, 8> InsertRegionsStack;

  void setOrigLoop(HLLoop *L) { OrigLoop = L; }
  void setPeelLoop(HLLoop *L) { PeelLoop = L; }
  void setMainLoop(HLLoop *L) { MainLoop = L; }
  void setNeedFirstItPeelLoop(bool NeedFirstItPeel) {
    NeedFirstItPeelLoop = NeedFirstItPeel;
  }
  void setNeedRemainderLoop(bool NeedRem) { NeedRemainderLoop = NeedRem; }
  void setTripCount(uint64_t TC) { TripCount = TC; }
  void setVF(unsigned V) { VF = V; }

  // Erase loop intrinsics implementation - delete intel intrinsic directives
  // before/after the loop based on the BeginDir which determines where we start
  // the lookup.
  void eraseLoopIntrinsImpl(bool BeginDir);

  /// \brief Analyzes the memory references of \p OrigCall to determine
  /// stride. The resulting stride information is attached to the arguments
  /// of \p WideCall in the form of attributes.
  void analyzeCallArgMemoryReferences(const HLInst *OrigCall, HLInst *WideCall,
                                      SmallVectorImpl<RegDDRef *> &Args);

  // Given reduction operator identity value, insert vector reduction operand
  // initialization to a vector of length VF identity values. Return the
  // initialization instruction. The initialization is added before RednHoistLp
  // and the LVAL of this instruction is used as the widened reduction ref.
  HLInst *insertReductionInitializer(Constant *Iden, RegDDRef *ScalarRednRef);

  // Add entry to WidenMap and handle generating code for liveout/reduction at
  // the end of loop specified by /p HoistLp.
  void addToMapAndHandleLiveOut(const RegDDRef *ScalRef, HLInst *WideInst,
                                HLLoop *HoistLp);

  // Find users of OrigRef and replaces them with NewRef.
  void replaceOrigRef(RegDDRef *OrigRef, RegDDRef *NewRef);

  // Replace math library calls in the remainder loop with the vectorized one
  // used in the main vector loop.
  void replaceLibCallsInRemainderLoop(HLInst *HInst);

  // The small loop trip count and body thresholds used to determine where it
  // is appropriate for complete unrolling. May eventually need to be moved to
  // the cost model.
  static constexpr unsigned SmallTripThreshold = 16;
  static constexpr unsigned SmallLoopBodyThreshold = 10;

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

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPOCODEGENHIR_H
