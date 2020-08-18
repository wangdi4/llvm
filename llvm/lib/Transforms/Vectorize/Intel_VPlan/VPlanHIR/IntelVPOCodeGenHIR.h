//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017-2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   IntelVPOCodeGenHIR.h -- HIR Vector Code generation from VPlan
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPOCODEGENHIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPOCODEGENHIR_H

#include "../IntelVPlanIdioms.h"
#include "../IntelVPlanLoopUnroller.h"
#include "../IntelVPlanValue.h"
#include "../IntelVPlanOptrpt.h"
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
class OVLSGroup;

namespace loopopt {
class HIRSafeReductionAnalysis;
} // namespace loopopt

namespace vpo {
class WRNVecLoopNode;
class HIRVectorizationLegality;
class VPlan;
class VPlanVLSAnalysis;
class VPInstruction;

extern bool EnableVPValueCodegenHIR;

// VPOCodeGenHIR generates vector code by widening of scalars into
// appropriate length vectors.
class VPOCodeGenHIR {
public:
  VPOCodeGenHIR(TargetLibraryInfo *TLI, TargetTransformInfo *TTI,
                HIRSafeReductionAnalysis *SRA, VPlanVLSAnalysis *VLSA,
                const VPlan *Plan, Function &Fn, HLLoop *Loop,
                LoopOptReportBuilder &LORB,
                const VPLoopEntityList *VPLoopEntities,
                const HIRVectorizationLegality *HIRLegality,
                const VPlanIdioms::Opcode SearchLoopType,
                const RegDDRef *SearchLoopPeelArrayRef,
                const VPlanLoopUnroller::VPInstUnrollPartTy &VPInstUnrollPart)
      : TLI(TLI), TTI(TTI), SRA(SRA), Plan(Plan), VLSA(VLSA), Fn(Fn),
        Context(*Plan->getLLVMContext()), OrigLoop(Loop), PeelLoop(nullptr),
        MainLoop(nullptr), CurMaskValue(nullptr), NeedRemainderLoop(false),
        TripCount(0), VF(0), UF(1), LORBuilder(LORB),
        VPLoopEntities(VPLoopEntities), HIRLegality(HIRLegality),
        SearchLoopType(SearchLoopType),
        SearchLoopPeelArrayRef(SearchLoopPeelArrayRef),
        BlobUtilities(Loop->getBlobUtils()),
        CanonExprUtilities(Loop->getCanonExprUtils()),
        DDRefUtilities(Loop->getDDRefUtils()),
        HLNodeUtilities(Loop->getHLNodeUtils()),
        VPInstUnrollPart(VPInstUnrollPart) {
    assert(Plan->getVPLoopInfo()->size() == 1 && "Expected one loop");
    VLoop = *(Plan->getVPLoopInfo()->begin());
  }

  ~VPOCodeGenHIR() {
    SCEVWideRefMap.clear();
    WidenMap.clear();
    VPValWideRefMap.clear();
    SCEVWideRefMap.clear();
    VLSGroupLoadMap.clear();

    for (auto StMap : VLSGroupStoreMap)
      delete StMap.second;

    VLSGroupStoreMap.clear();
  }

  // Setup vector loop to perform the actual loop widening (vectorization) using
  // VF as the vectorization factor.
  bool initializeVectorLoop(unsigned int VF, unsigned int UF);

  // Perform and cleanup/final actions after vectorizing the loop
  void finalizeVectorLoop(void);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dumpFinalHIR();
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Fixup gotos in GotoTargetVPBBPairVector using VPBBLabelMap
  void finalizeGotos(void);

  // Check if loop is currently suported by CodeGen.
  bool loopIsHandled(HLLoop *Loop, unsigned int VF);

  // Return the trip count for the scalar loop. Returns 0 for unknown trip
  // count loops
  uint64_t getTripCount() const { return TripCount; }

  Function &getFunction() const { return Fn; }
  HLLoop *getOrigLoop() const { return OrigLoop; }
  HLLoop *getMainLoop() const { return MainLoop; }
  unsigned getVF() const { return VF; };
  VPlanVLSAnalysis *getVLS() const { return VLSA; }
  const VPlan *getPlan() const { return Plan; }
  bool getNeedRemainderLoop() const { return NeedRemainderLoop; }
  HLLoop *getRemainderLoop() const { return OrigLoop; }

  OptReportStatsTracker &getOptReportStatsTracker() { return OptRptStats; }

  void setForceMixedCG(bool MixedCG) { ForceMixedCG = MixedCG; }
  bool getForceMixedCG() const { return ForceMixedCG; }

  // Return true if Ref is a reduction
  bool isReductionRef(const RegDDRef *Ref, unsigned &Opcode);

  // Returns the HLLoop where it is safe to hoist the reduction initializer
  // statement. It is also the point after which the last value computation
  // instructions can be placed.
  HLLoop *findRednHoistInsertionPoint(HLLoop *Lp);

  // Utility that sets the hoist loop for reductions in loop being vectorized
  // (OrigLoop). If there are no reductions in the loop, then insertion point is
  // set to OrigLoop. This utility is expected to be called only after knowledge
  // about reductions is imported into CG. It is also expected to work only on
  // unmodified HIR.
  void setRednHoistPtForVectorLoop();

  // Propagate metadata from memory references in either the group or old DDRef
  // to the new DDRef. If Group is non-null, references in Group are used to set
  // NewRef's metadata. Otherwise, OldRef is expected to be non-null and the
  // same is used to set NewRef's metadata.
  void propagateMetadata(RegDDRef *NewRef, const OVLSGroup *Group = nullptr,
                         const RegDDRef *OldRef = nullptr);

  // Propagate debug location information from VPInstruction to the generated
  // HIR constructs. This is a post processing approach i.e. we don't attach
  // debug location while generating the construct, but immediately after the
  // generation.
  // TODO: Some known caveats of this approach -
  // 1. If a single VPInst is lowered to multiple HIR constructs (like multiple
  // HLInsts) then we attach debug location only to the last generated
  // construct. For example - OptVLS : VPLoad becomes a load + shuffle. DbgLoc
  // is set only for shuffle, not the load. This should be rare since VPlan IR
  // is a decomposed version of incoming HIR.
  // 2. Some potential loss of debug location for mixed CG approach (non-default
  // today).
  void propagateDebugLocation(const VPInstruction *VPInst);

  // Widen the given VPInstruction to a vector instruction using VF
  // as the vector length. The given Mask value overrides the
  // current mask value if non-null. Group is non-null if Inst is part of a
  // VLS group and in such a case InterleaveFactor specifies the memory access
  // interleaving factor, InterleaveIndex specifies the index of the current
  // memory access reference in the group, and GrpStartInst specifies the HLInst
  // corresponding to the lowest memory address access in the group.
  void widenNode(const VPInstruction *VPInst, RegDDRef *Mask = nullptr,
                 const OVLSGroup *Group = nullptr, int64_t InterleaveFactor = 0,
                 int64_t InterleaveIndex = 0,
                 const HLInst *GrpStartInst = nullptr);

  /// Adjust arguments passed to SVML functions to handle masks
  void addMaskToSVMLCall(Function *OrigF, AttributeList OrigAttrs,
                         SmallVectorImpl<RegDDRef *> &VecArgs,
                         SmallVectorImpl<Type *> &VecArgTys,
                         SmallVectorImpl<AttributeSet> &VecArgAttrs,
                         RegDDRef *MaskValue);

  /// Generate instructions to extract two results of a widened sincos call
  /// \p WideInst, and store them to locations designated in the original call
  /// \p HInst. Instructions may be generated for both vector loop body and
  /// remainder loop, \p IsRemainderLoop is used to distinguish these scenarios.
  void generateStoreForSinCos(const HLInst *HInst, HLInst *WideInst,
                              RegDDRef *Mask, bool IsRemainderLoop);

  // Given the function being called and the widened operands, generate and
  // return the widened call. The call arguments are returned in CallRegs
  // if they need to be analyzed for stride information. The flag HasLvalArg
  // is used to indicate if we have the widened operand corresponding to
  // call return value in WideOps.
  HLInst *widenCall(const HLInst *INode, SmallVectorImpl<RegDDRef *> &WideOps,
                    RegDDRef *Mask, SmallVectorImpl<RegDDRef *> &CallRegs,
                    bool HasLvalArg);

  // Widen an interleaved memory access - operands correspond to operands of
  // WidenNode.
  HLInst *widenInterleavedAccess(const HLInst *INode, RegDDRef *Mask,
                                 const OVLSGroup *Group,
                                 int64_t InterleaveFactor,
                                 int64_t InterleaveIndex,
                                 const HLInst *GrpStartInst,
                                 const VPInstruction *VPInst);

  // A helper function for concatenating vectors. This function concatenates two
  // vectors having the same element type. If the second vector has fewer
  // elements than the first, it is padded with undefs. This function mimics
  // the interface in VectorUtils.cpp modified to work with HIR. Vector values
  // V1 and V2 are concatenated and masked using Mask.
  RegDDRef *concatenateTwoVectors(RegDDRef *V1, RegDDRef *V2, RegDDRef *Mask);

  // Concatenate a list of vectors. This function generates code that
  // concatenate the vectors in VecsArray into a single large vector. The number
  // of vectors should be greater than one, and their element types should be
  // the same. The number of elements in the vectors should also be the same;
  // however, if the last vector has fewer elements, it will be padded with
  // undefs. This function mimics the interface in VectorUtils.cpp modified to
  // work with HIR. Vector values are concatenated and masked using Mask.
  RegDDRef *concatenateVectors(RegDDRef **VecsArray, int64_t NumVecs,
                               RegDDRef *Mask);

  // Given the LvalRef of a load instruction belonging to an interleaved group,
  // and the result of the corresponding wide interleaved load WLoadRes,
  // generate a shuffle using WLoadRes and the interleave index of the
  // current load instruction within the group. As an example, consider the
  // following load group(factor = 2, VF = 4)
  //     R = Pic[2*i];             // Member of index 0
  //     G = Pic[2*i+1];           // Member of index 1
  // The following shuffle is generated where for interleave index 0 and
  // %wide.vec = load <8 x i32> Pic[2*i]
  //    %R.vec = shuffle %wide.vec, undef, <0, 2, 4, 6>   ; R elements
  HLInst *createInterleavedLoad(const RegDDRef *LvalRef, RegDDRef *WLoadRes,
                                int64_t InterleaveFactor,
                                int64_t InterleaveIndex, RegDDRef *Mask);

  // Given the vector of values being stored for an interleaved store group,
  // generate the sequence of shuffles to combine the stored values, interleave
  // the concatenated vector and generate the wide store using the interleaved
  // value.
  // Given something like (factor = 2, VF=4)
  //     Pic[2*i] = R;           // Member of index 0
  //     Pic[2*i+1] = G;         // Member of index 1
  // this function generates the following sequence
  //     %R_G.vec = shuffle %R.vec, %G.vec, <0, 1, 2, ..., 7>
  //     %interleaved.vec = shuffle %R_G.vec, undef,
  //                                 <0, 4, 1, 5, 2, 6, 3, 7>
  //     store <8 x i32> %interleaved.vec, Pic[2*i]
  HLInst *createInterleavedStore(RegDDRef **StoreVals, const RegDDRef *StoreRef,
                                 int64_t InterleaveFactor, RegDDRef *Mask);

  HLInst *createReverseVector(RegDDRef *ValRef);

  HLInst *handleLiveOutLinearInEarlyExit(HLInst *Inst, RegDDRef *Mask,
                                         bool MaskIsNonZero);

  /// Collect live-out definitions reaching \p Goto's parent and insert a copy
  /// of them in the current insertion point of the main vector loop. Linear
  /// references in the live-out copies are shifted by an offset corresponding
  /// to the first vector lane taking the early exit. \p Goto must be an early
  /// exit of the loop we are vectorizing. \p Goto's parent must be an HLIf.
  void handleNonLinearEarlyExitLiveOuts(const HLGoto *Goto);

  /// Create a BitCast HLInst and put it into \p Container if it is not null
  /// else insert at the default insertion point.
  HLInst *createBitCast(Type *Ty, RegDDRef *Ref,
                        HLContainerTy *Container = nullptr,
                        const Twine &Name = "cast");

  HLInst *createZExt(Type *Ty, RegDDRef *Ref, const Twine &Name = "cast") {
    HLInst *ZExtInst = HLNodeUtilities.createZExt(Ty, Ref->clone(), Name);
    addInstUnmasked(ZExtInst);
    return ZExtInst;
  }

  /// Create a call to a zero counting intrinsic defined by \p Id and put it
  /// either into \p Container (if it's not null) or at the default insertion
  /// point.
  HLInst *createCTZCall(RegDDRef *Ref, llvm::Intrinsic::ID Id,
                        bool MaskIsNonZero, HLContainerTy *Container = nullptr,
                        const Twine &Name = "bsf");

  // Generates wide compares using VF as the vector length. Multiple
  // predicates are handled by conjoining the results of generated
  // wide compares with an implicit wide AND. The given Mask value
  // overrides the current mask value if non-null. The last HLInst
  // generated for this HLIf node is returned.
  HLInst *widenIfNode(const HLIf *HIf, RegDDRef *Mask = nullptr);

  // Handle vector code generation when given Inst is a non-masked uniform
  // store. If the Rval of Inst is invariant then only a scalar store is
  // generated, else an extractelement operation is performed before the store
  HLInst *widenNonMaskedUniformStore(const HLInst *Inst);

  // Add WideVal as the widened vector value corresponding  to VPVal
  void addVPValueWideRefMapping(const VPValue *VPVal, RegDDRef *WideVal) {
    VPValWideRefMap[VPVal] = WideVal;
  }

  // Return the widened vector value corresponding to VPVal if found
  // in VPValWideRefMap, return null otherwise.
  RegDDRef *getWideRefForVPVal(const VPValue *VPVal) const {
    auto Itr = VPValWideRefMap.find(VPVal);
    if (Itr != VPValWideRefMap.end())
      return Itr->second;
    else
      return nullptr;
  }

  // Add ScalVal as the scalar ref corresponding to VPVal in VPValScalRefMap
  // for specified Lane.
  void addVPValueScalRefMapping(const VPValue *VPVal, RegDDRef *ScalVal,
                                unsigned Lane) {
    VPValScalRefMap[VPVal][Lane] = ScalVal;
  }

  // Return the scalar ref corresponding to VPVal for specified Lane if found
  // in VPValScalRefMap, return null otherwise.
  RegDDRef *getScalRefForVPVal(const VPValue *VPVal, unsigned Lane) const {
    auto It = VPValScalRefMap.find(VPVal);
    if (It == VPValScalRefMap.end())
      return nullptr;

    auto SVMap = It->second;
    auto Itr = SVMap.find(Lane);
    if (Itr == SVMap.end())
      return nullptr;

    return Itr->second;
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
  void addInstUnmasked(HLInst *Inst) { addInst(Inst, nullptr); }

  void addInstUnmasked(HLInst *Inst, const bool IsThenChild) {
    addInst(Inst, nullptr, IsThenChild);
  }

  // Add the given instruction at the end of the main loop and mask
  // it with the provided mask value if non-null. If the masked instruction
  // writes into a loop private temp ref, then initialize the temp with undef
  // value in loop header to prevent any backedge flows introduced by selects
  // during lowering.
  void addInst(HLNode *Node, RegDDRef *Mask) {
    if (Mask) {
      HLInst *Inst = dyn_cast<HLInst>(Node);
      assert(Inst && "Only HLInst can have a mask.");
      Inst->setMaskDDRef(Mask->clone());

      // Initialize Lval temp to undef if it's private per loop iteration.
      RegDDRef *LvalRef = Inst->getLvalDDRef();
      if (LvalRef && LvalRef->isTerminalRef() &&
          !MainLoop->isLiveIn(LvalRef->getSymbase()) &&
          InitializedPrivateTempSymbases.insert(LvalRef->getSymbase()).second) {
        RegDDRef *Init =
            DDRefUtilities.createUndefDDRef(LvalRef->getDestType());
        auto InitInst =
            HLNodeUtilities.createCopyInst(Init, "priv.init", LvalRef->clone());
        // We handle only innermost loop vectorization today, so initialize
        // temp in MainLoop header.
        HLNodeUtils::insertAsFirstChild(MainLoop, InitInst);
      }
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
    IsThenChild ? HLNodeUtils::insertAsLastThenChild(InsertRegion, Node)
                : HLNodeUtils::insertAsLastElseChild(InsertRegion, Node);
  }

  void setCurMaskValue(RegDDRef *V) { CurMaskValue = V; }

  // Return widened ref if Symbase is in WidenMap, return nullptr
  // otherwise.
  RegDDRef *getWideRef(unsigned Symbase) const {
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

  // Widen Ref to specified VF if needed and return the widened ref. If
  // Interleaveaccess is true we widen the reference treating it as a unit
  // stride reference.
  RegDDRef *widenRef(const RegDDRef *Ref, unsigned VF,
                     bool InterleaveAccess = false);

  // Return the widened DDRef corresponding to VPVal - when we enable full
  // VPValue based codegen, this function will generate the widened DDRef
  // if one is not found.
  RegDDRef *widenRef(const VPValue *VPVal, unsigned VF);

  // Returns the expression IV + <0, 1, .., VF-1> where IV is the current loop's
  // main induction variable.
  RegDDRef *generateLoopInductionRef(Type *RefDestTy);

  // Return true if the given VPPtr has a stride of 1 or -1. IsNegOneStride is
  // set to true if stride is -1 and false otherwise.
  bool isUnitStridePtr(const VPValue *VPPtr, bool &IsNegOneStride) const {
    return Plan->getVPlanDA()->isUnitStridePtr(VPPtr, IsNegOneStride);
  }

  // Given the pointer operand of a load/store instruction, setup and return the
  // memory ref to use in generating the load/store HLInst. ScalSymbase
  // specifies the symbase to set for the returned ref. AANodes specify alias
  // analysis metadata to set for the returned ref. Lane0Value specifies if
  // we need memory ref corresponding to just vector lane 0. This is used to
  // handle uniform memory accesses. If VPPtr is unit strided(stride of 1/-1),
  // the ref returned is properly adjusted to enable wide load/store.
  RegDDRef *getMemoryRef(const VPValue *VPPtr, unsigned ScalSymbase,
                         const AAMDNodes &AANodes, bool Lane0Value = false);

  bool isSearchLoop() const {
    return VPlanIdioms::isAnySearchLoop(SearchLoopType);
  }
  VPlanIdioms::Opcode getSearchLoopType() const { return SearchLoopType; }

  HLDDNode *getInsertRegion() const { return InsertRegionsStack.back(); }
  void addInsertRegion(HLDDNode *Node) { InsertRegionsStack.push_back(Node); }
  void popInsertRegion() { InsertRegionsStack.pop_back(); }

  void createHLIf(const CmpInst::Predicate Pred, RegDDRef *LhsRef,
                  RegDDRef *RhsRef) {
    HLDDNode *If = HLNodeUtilities.createHLIf(Pred, LhsRef, RhsRef);
    addInst(If, nullptr);
    addInsertRegion(If);
  }

  // Create a new temp ref to represent VPLoopEntities inside the generated
  // vector loop. Type of this temp ref is VF x Entity's type. Additionally we
  // map instructions linked to a LoopEntity, to the new ref which will be used
  // during CG.
  void createAndMapLoopEntityRefs(unsigned VF);

  // Utility to check if target being compiled for has AVX512 Intel
  // optimizations.
  bool targetHasAVX512() const;

  void setUniformControlFlowSeen() {
    // Search loops do not go through predication currently and code generation
    // for this is handled separately for now. Until search loop representation
    // is made explicit we do not need any additional handling of the uniform
    // control flow case for them.
    if (!isSearchLoop())
      UniformControlFlowSeen = true;
  }

  bool getUniformControlFlowSeen() const { return UniformControlFlowSeen; }

  const VPLoop *getVPLoop() const { return VLoop; }

  // Emit a label to indicate the start of the given basic block if the
  // block is inside the loop being vectorized and add the label to
  // the VPBBLabelMap.
  void emitBlockLabel(const VPBasicBlock *VPBB);

  // Emit the needed gotos to appropriate labels if the block is inside
  // the loop being vectorized. Add the gotos generated to
  // GotoTargetVPBBPairVector so that the gotos are fixed up at the end of
  // vector code generation.
  void emitBlockTerminator(const VPBasicBlock *SourceBB);

  // Return the Lval temp that was generated to represent the deconstructed PHI
  // identified by its PhiId, if found in PhiIdLValTempsMap. Return null
  // otherwise.
  RegDDRef *getLValTempForPhiId(const int PhiId) const {
    auto Itr = PhiIdLValTempsMap.find(PhiId);
    if (Itr != PhiIdLValTempsMap.end())
      return Itr->second;
    else
      return nullptr;
  }

private:
  // Target Library Info is used to check for svml.
  TargetLibraryInfo *TLI;

  // Target Transform Info is used to check whether first iteration peeling is
  // needed
  TargetTransformInfo *TTI;

  HIRSafeReductionAnalysis *SRA;

  // VPlan for which vector code is being generated.
  const VPlan *Plan;

  // VPLoop being vectorized - assumes VPlan contains one loop.
  const VPLoop *VLoop;

  // OPTVLS analysis.
  VPlanVLSAnalysis *VLSA;

  // Current function.
  Function &Fn;

  // LLVMContext associated with current function.
  LLVMContext &Context;

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

  // Is a peel loop needed?
  bool NeedPeelLoop = false;

  // Is a remainder loop needed?
  bool NeedRemainderLoop;

  // Force mixed code generation - used when we see cases such as search loops,
  // live out privates, and Fortran subscript arrays
  bool ForceMixedCG = false;

  // Loop trip count if constant. Set to zero for non-constant trip count loops.
  uint64_t TripCount;

  // Vector factor or vector length to use. Each scalar instruction is widened
  // to operate on this number of operands.
  unsigned VF;

  // Unroll factor which was applied to VPlan before code generation.
  unsigned UF;

  LoopOptReportBuilder &LORBuilder;
  OptReportStatsTracker OptRptStats;

  // Map of DDRef symbase and widened ref
  DenseMap<unsigned, RegDDRef *> WidenMap;
  DenseMap<const VPValue *, RegDDRef *> VPValWideRefMap;

  // Map of scalar refs for VPValue + vector Lane
  DenseMap<const VPValue *, DenseMap<unsigned, RegDDRef *>> VPValScalRefMap;

  // Map of SCEV expression and widened DDRef.
  DenseMap<const SCEV *, RegDDRef *> SCEVWideRefMap;

  // Map of load OVLSGroup and the corresponding RegDDRef containing the result
  // of the interleaved wide store.
  SmallDenseMap<const OVLSGroup *, RegDDRef *> VLSGroupLoadMap;

  // Map of store OVLSGroup and the vector of values(RegDDRefs) being stored
  // into memrefs in this group.
  SmallDenseMap<const OVLSGroup *, RegDDRef **> VLSGroupStoreMap;

  // The insertion points for reduction initializer and reduction last value
  // compute instructions.
  HLLoop *RednHoistLp = nullptr;
  HLNode *RedInitInsertPoint = nullptr;
  HLNode *RedFinalInsertPoint = nullptr;

  // VPEntities present in current loop being vectorized. These include
  // reductions, inductions and privates.
  const VPLoopEntityList *VPLoopEntities;

  // HIR vectorization legality which contains reductions, inductions and
  // privates coming from SIMD clause descriptors.
  const HIRVectorizationLegality *HIRLegality;

  // Set of unit-stride Refs
  SmallPtrSet<const RegDDRef *, 4> UnitStrideRefSet;
  // The loop meets search loop idiom criteria.
  VPlanIdioms::Opcode SearchLoopType;
  // Array memref that needs to be aligned (if necessary) in the peel loop
  // generated for a vectorized search loop.
  const RegDDRef *SearchLoopPeelArrayRef = nullptr;

  // References to miscellaneous HIR creation utilities objects.
  BlobUtils &BlobUtilities;
  CanonExprUtils &CanonExprUtilities;
  DDRefUtils &DDRefUtilities;
  HLNodeUtils &HLNodeUtilities;

  SmallVector<HLDDNode *, 8> InsertRegionsStack;

  // Set of VPInsts involved in a reduction - used to avoid folding of
  // operations.
  SmallPtrSet<const VPInstruction *, 4> ReductionVPInsts;

  // Map of VPValues(reduction PHI and its operands) and their corresponding HIR
  // reduction variable(RegDDRef) used inside the generated vector loop.
  DenseMap<const VPValue *, RegDDRef *> ReductionRefs;

  // Collection of VPInstructions inside the loop that correspond to main loop
  // IV. This is expected to contain the PHI and incrementing add
  // instruction(s).
  SmallPtrSet<const VPValue *, 8> MainLoopIVInsts;
  // Map of VPlan's private memory objects and their corresponding HIR BlobDDRef
  // created to represent within vector loop.
  DenseMap<const VPAllocatePrivate *, BlobDDRef *> PrivateMemBlobRefs;

  // Set of masked private temp symbases that have been initialized to undef in
  // vector loop header.
  SmallSet<unsigned, 16> InitializedPrivateTempSymbases;

  // Boolean flag used to see if the loop being vectorized has any uniform
  // control flow.
  bool UniformControlFlowSeen = false;

  // Vector of <HLGoto, target basic block> pairs.
  SmallVector<std::pair<HLGoto *, const VPBasicBlock *>, 8>
      GotoTargetVPBBPairVector;

  // Map from a basic block to its starting label.
  SmallDenseMap<const VPBasicBlock *, HLLabel *> VPBBLabelMap;

  // Mapping of VPInstructions to their unrolled part numbers.
  const VPlanLoopUnroller::VPInstUnrollPartTy &VPInstUnrollPart;

  // Unrolled part number for the VPInstruction currently being processed.
  unsigned CurrentVPInstUnrollPart;

  // Track HIR temp created for each deconstructed PHI ID.
  SmallDenseMap<int, RegDDRef *> PhiIdLValTempsMap;

  void setOrigLoop(HLLoop *L) { OrigLoop = L; }
  void setPeelLoop(HLLoop *L) { PeelLoop = L; }
  void setMainLoop(HLLoop *L) { MainLoop = L; }
  void setNeedPeelLoop(bool NeedPeel) { NeedPeelLoop = NeedPeel; }
  void setNeedRemainderLoop(bool NeedRem) { NeedRemainderLoop = NeedRem; }
  void setTripCount(uint64_t TC) { TripCount = TC; }
  void setVF(unsigned V) { VF = V; }
  void setUF(unsigned U) { UF = U == 0 ? 1 : U; }

  void insertReductionInit(HLInst *Inst) {
    // RedInitInsertPoint is never updated after initial assignment, so it
    // should always be the reduction hoist loop.
    assert(isa<HLLoop>(RedInitInsertPoint) &&
           "Reduction init insert point is not a loop.");
    HLNodeUtils::insertAsLastPreheaderNode(cast<HLLoop>(RedInitInsertPoint),
                                           Inst);
  }
  void insertReductionInit(HLContainerTy *List) {
    assert(isa<HLLoop>(RedInitInsertPoint) &&
           "Reduction init insert point is not a loop.");
    HLNodeUtils::insertAsLastPreheaderNodes(cast<HLLoop>(RedInitInsertPoint),
                                            List);
  }
  void insertReductionFinal(HLInst *Inst) {
    if (auto *RedFinalInsertLp = dyn_cast<HLLoop>(RedFinalInsertPoint))
      HLNodeUtils::insertAsFirstPostexitNode(RedFinalInsertLp, Inst);
    else
      HLNodeUtils::insertAfter(RedFinalInsertPoint, Inst);

    RedFinalInsertPoint = Inst;
  }
  void insertReductionFinal(HLContainerTy *List) {
    HLNode *Save = &List->back();
    if (auto *RedFinalInsertLp = dyn_cast<HLLoop>(RedFinalInsertPoint))
      HLNodeUtils::insertAsFirstPostexitNodes(RedFinalInsertLp, List);
    else
      HLNodeUtils::insertAfter(RedFinalInsertPoint, List);
    RedFinalInsertPoint = Save;
  }

  /// Return true if the opcode is one for which we want to generate a scalar
  /// HLInst. Temporary solution until such decision is driven by the results
  /// of SVA analysis.
  bool isOpcodeForScalarInst(unsigned Opcode) {
    switch (Opcode) {
    case Instruction::Add:
    case Instruction::BitCast:
    case Instruction::Mul:
    case Instruction::GetElementPtr:
    case VPInstruction::Subscript:
    case Instruction::SExt:
    case Instruction::Trunc:;
    case Instruction::ZExt:
      return true;
    default:
      return false;
    }
  }

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

  // Helper function to generate a wide Cmp HLInst for given predicate
  // PredIt found in the HLIf node. VF is used as vector length.
  HLInst *widenPred(const HLIf *HIf, HLIf::const_pred_iterator PredIt,
                    RegDDRef *Mask);

  // Implementation of widening the given instruction to a vector instruction
  // using VF as the vector length. This interface is used by the public
  // interface when a VPInstruction has a valid underlying HLInst. This
  // function also adds the mapping between VPInst and the widened value.
  void widenNodeImpl(const HLInst *Inst, RegDDRef *Mask, const OVLSGroup *Group,
                     int64_t InterleaveFactor, int64_t InterleaveIndex,
                     const HLInst *GrpStartInst, const VPInstruction *VPInst);

  // Helper utility to get result type corresponding to \p RefTy based on \p
  // Widen.
  Type *getResultRefTy(Type *RefTy, unsigned VF, bool Widen) {
    return Widen ? FixedVectorType::get(RefTy, VF) : RefTy;
  }

  // Helper utility to make \p Ref consistent and map it to \p VPInst based on
  // \p Widen.
  void makeConsistentAndAddToMap(RegDDRef *Ref, const VPInstruction *VPInst,
                                 SmallVectorImpl<const RegDDRef *> &AuxRefs,
                                 bool Widen) {
    // Use AuxRefs if it is not empty to make Ref consistent
    if (!AuxRefs.empty())
      Ref->makeConsistent(AuxRefs, OrigLoop->getNestingLevel());
    if (Widen)
      addVPValueWideRefMapping(VPInst, Ref);
    else
      addVPValueScalRefMapping(VPInst, Ref, 0);
  }

  // Implementation of generating needed HIR constructs for the given
  // VPInstruction. We generate new RegDDRefs or HLInsts that correspond to
  // the given VPInstruction. Widen parameter is used to specify if we are
  // generating VF wide constructs. If Widen is false, we generate scalar
  // constructs for lane 0.
  void generateHIR(const VPInstruction *VPInst, RegDDRef *Mask,
                   const OVLSGroup *Group, int64_t InterleaveFactor,
                   int64_t InterleaveIndex, const HLInst *GrpStartInst,
                   bool Widen);

  // Wrapper used to call generateHIR appropriately based on nature of given
  // VPInstruction.
  void widenNodeImpl(const VPInstruction *VPInst, RegDDRef *Mask,
                     const OVLSGroup *Group, int64_t InterleaveFactor,
                     int64_t InterleaveIndex, const HLInst *GrpStartInst);

  // Implementation of blend widening.
  void widenBlendImpl(const VPBlendInst *Blend, RegDDRef *Mask);

  // Implementation of VPPhi widening.
  void widenPhiImpl(const VPPHINode *VPPhi, RegDDRef *Mask);

  // Generate instructions to compare Value against zero. Value's type is
  // expected to be a vector of i1. InstMask specifies the mask to add
  // for the generated instructions. The compare predicate is ICMP_EQ if Equal
  // is true and ICMP_NE otherwise.
  RegDDRef *generateCompareToZero(RegDDRef *Value, RegDDRef *InstMask,
                                  bool Equal);

  // Implementation of load where the pointer operand being loaded from is
  // uniform. VPInst is the load instruction and Mask specifies the load Mask. A
  // scalar load is done using the pointer operand and the value is
  // broadcast to generate the vector value. If Mask is non-null, the load
  // is done conditionally only if Mask is non-zero.
  void widenUniformLoadImpl(const VPInstruction *VPInst, RegDDRef *Mask);

  // Implementation of load/store widening.
  void widenLoadStoreImpl(const VPInstruction *VPInst, RegDDRef *Mask);

  // Implementation of codegen for subscript instruction.
  void generateHIRForSubscript(const VPSubscriptInst *VPSubscript,
                               RegDDRef *Mask, bool Widen);

  // Implementation of widening of VPLoopEntity specific instructions. Some
  // notes on opcodes supported so far -
  // 1. reduction-init  : We generate a broadcast/splat of reduction
  //                      identity. If a start value is specified for the
  //                      reduction, then it is inserted into lane 0 after
  //                      the broadcast.
  //
  //    Example -
  //    i64 %vp0 = reduction-init i32 0 i32 %red.init
  //
  //    Generated HIR -
  //    %red.ref = 0;
  //    %red.ref = insertelement %red.ref,  %red.init,  0
  //
  // 2. reduction-final : The final vector reduction RegDDRef is reduced
  //                      using "llvm.experimental.vector.reduce" intrinsics
  //                      and stored back to original reduction scalar RegDDRef.
  //                      If accumulator value is present, then a final scalar
  //                      operation is performed for last-value computation.
  //
  //    Example -
  //    i64 % vp1 = reduction-final{u_add} i64 % vp.red.ref
  //
  //    Generated HIR -
  //    %red.init = @llvm.experimental.vector.reduce.add.v4i64 (%red.ref);
  //
  void widenLoopEntityInst(const VPInstruction *VPInst);

  // The following code is generated supposing we have
  //    reduction-final (pred) reduce_val, parent_final, parent_exit
  // %bcst = broadcast parent_final
  // %cmp_v = cmp eq %bcst, parent_exit
  // %ndx_fixup = select %cmp_v  %reduce_val, (is_min(pred) ? MAX_INT:MIN_INT)
  // %result = (is_min(pred) ? reduce_min: reduce_max)(%ndx_fixup)
  //
  // For example, we have the following values for min+min_index.
  //  parent_exit:  {1,5,1,3}      // minimal values for 4 lanes
  //  reduce_val :  {100,20,85,55} // the indexes of those values for each lane
  //  parent_final: 1
  // We should choose the minimum index from those that correspond to the
  // parent_final. That is done by setting the non-relevant elements in the
  // reduce_val to MAX_INT. So we have {100,MAX_INT,85,MAX_INT}. Then we just
  // select the minimum from those values, 85.
  //
  void generateMinMaxIndex(const VPReductionFinal *RedFinal,
                           RegDDRef *RednDescriptor, HLContainerTy &RedTail,
                           HLInst *&WInst);

  // Get scalar version of RegDDRef that represents the VPValue \p VPVal
  // which is either an external definition or a constant. We can build the
  // scalar version from underlying HIR operand attached to VPValue.
  RegDDRef *getUniformScalarRef(const VPValue *VPVal);

  // Get scalar version of RegDDRef that represents the VPValue \p VPVal for
  // lane 0 from VPValScalRefMap. If not found in the map,  we can obtain the
  // scalar ref using getUniformScalarRef for external definitions and
  // constants. For others, we create an extract element instruction for lane 0
  // from the wide reference and return the result of the extract.
  RegDDRef *getOrCreateScalarRef(const VPValue *VPVal);

  // Wrapper utility method to obtain RegDDRef corresponding to given VPValue
  // based on the Widen boolean.
  RegDDRef *getOrCreateRefForVPVal(const VPValue *VPVal, bool Widen) {
    return Widen ? widenRef(VPVal, getVF()) : getOrCreateScalarRef(VPVal);
  }

  // For Generate PaddedCounter < 250 and insert it into the vector of runtime
  // checks if this is a search loop which needs the check.
  // Nothing is added if padding transformation is not required for this loop.
  void addPaddingRuntimeCheck(
      SmallVectorImpl<std::tuple<HLPredicate, RegDDRef *, RegDDRef *>>
          &RTChecks);

  // If VPInst has a corresponding reduction ref, create a copy instruction
  // copying RValRef to the same with given Mask and return LvalRef of the copy
  // instruction. Return RValRef otherwise.
  RegDDRef *createCopyForRednRef(const VPInstruction *VPInst, RegDDRef *RvalRef,
                                 RegDDRef *Mask) {
    auto Itr = ReductionRefs.find(VPInst);
    if (Itr == ReductionRefs.end())
      return RvalRef;
    auto *RedRef = Itr->second;
    HLInst *CopyInst =
        HLNodeUtilities.createCopyInst(RvalRef, "redval.copy", RedRef->clone());
    addInst(CopyInst, Mask);
    return CopyInst->getLvalDDRef();
  }

  // Internal helper utility to get operator overflow flags (nuw/nsw) for a
  // VPInstruction and the reduction ref if it participates in reduction
  // sequence.
  void getOverflowFlagsAndRednRef(const VPInstruction *VPInst, bool &HasNUW,
                                  bool &HasNSW, RegDDRef *&RedRef) {
    // Overflow flags should be preserved only for instructions that don't
    // participate in reduction sequence.
    bool PreserveOverflowFlags = ReductionVPInsts.count(VPInst) == 0;
    HasNUW = PreserveOverflowFlags && VPInst->hasNoUnsignedWrap();
    HasNSW = PreserveOverflowFlags && VPInst->hasNoSignedWrap();

    // If binop instruction corresponds to a reduction, then we need to write
    // the result back to the corresponding reduction variable. Overflow flags
    // should not be preserved for this instruction.
    RedRef = nullptr;
    if (ReductionRefs.count(VPInst)) {
      assert(!PreserveOverflowFlags &&
             "Overflow flags cannot be preserved for reduction instruction.");
      RedRef = ReductionRefs[VPInst];
    }
  }

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
