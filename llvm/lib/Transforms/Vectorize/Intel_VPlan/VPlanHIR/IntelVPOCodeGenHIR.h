//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017-2021 Intel Corporation. All rights reserved.
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
#include "../IntelVPlanOptrpt.h"
#include "../IntelVPlanValue.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HIRVisitor.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
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
                const VPlanVector *Plan, Function &Fn, HLLoop *Loop,
                OptReportBuilder &ORB, const VPLoopEntityList *VPLoopEntities,
                const HIRVectorizationLegality *HIRLegality,
                const VPlanIdioms::Opcode SearchLoopType,
                const RegDDRef *SearchLoopPeelArrayRef, bool IsOmpSIMD)
      : TLI(TLI), TTI(TTI), SRA(SRA), Plan(Plan), VLSA(VLSA), Fn(Fn),
        Context(*Plan->getLLVMContext()), OrigLoop(Loop), PeelLoop(nullptr),
        MainLoop(nullptr), CurMaskValue(nullptr), NeedRemainderLoop(false),
        TripCount(0), VF(0), UF(1), ORBuilder(ORB),
        VPLoopEntities(VPLoopEntities), HIRLegality(HIRLegality),
        SearchLoopType(SearchLoopType),
        SearchLoopPeelArrayRef(SearchLoopPeelArrayRef),
        BlobUtilities(Loop->getBlobUtils()),
        CanonExprUtilities(Loop->getCanonExprUtils()),
        DDRefUtilities(Loop->getDDRefUtils()),
        HLNodeUtilities(Loop->getHLNodeUtils()), IsOmpSIMD(IsOmpSIMD) {
    assert(Plan->getVPLoopInfo()->size() == 1 && "Expected one loop");
    VLoop = *(Plan->getVPLoopInfo()->begin());
  }

  ~VPOCodeGenHIR() {
    SCEVWideRefMap.clear();
    WidenMap.clear();
    VPValWideRefMap.clear();
    SCEVWideRefMap.clear();
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
  unsigned getUF() const { return UF; };
  VPlanVLSAnalysis *getVLS() const { return VLSA; }
  const VPlan *getPlan() const { return Plan; }
  bool getNeedRemainderLoop() const { return NeedRemainderLoop; }
  HLLoop *getRemainderLoop() const { return OrigLoop; }
  bool getIsOmpSIMD() const { return IsOmpSIMD; }

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

  // Propagate metadata using elements of MdSrcVec which are expected to be
  // either RegDDRefs or VPLoadStoreInsts to NewRef.
  template <class MDSource>
  void propagateMetadata(RegDDRef *NewRef, const MDSource * SrcMD);

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
  // current mask value if non-null.
  void widenNode(const VPInstruction *VPInst, RegDDRef *Mask = nullptr);

  /// Vectorize the given instruction that cannot be widened using
  /// serialization. This is done using a sequence of possible extractelements,
  /// Scalar Op, InsertElement instructions. Additionally if Mask is non-null
  /// then predicated serialization is done on-the-fly by creating a HLIf for
  /// each vector lane's mask value and inserting the generated extractelements,
  /// scalar op and insertelement instructions into the then branch.
  void serializeInstruction(const VPInstruction *VPInst, RegDDRef *Mask);

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

  /// Widen call instruction \p VPCall using vector library function. \p Mask is
  /// used to generate masked version of widened calls, if needed.
  HLInst *widenLibraryCall(const VPCallInstruction *VPCall, RegDDRef *Mask);

  /// Widen call instruction \p VPCall using vector intrinsic.
  HLInst *widenTrivialIntrinsic(const VPCallInstruction *VPCall);

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
  RegDDRef *concatenateVectors(ArrayRef<RegDDRef *> VecsArray, RegDDRef *Mask);

  /// Helper method to create a shuffle using \p Input and undef vectors based
  /// on given \p Mask vector. Shuffle's result is writen to \p WLvalRef if
  /// non-null.
  HLInst *createShuffleWithUndef(RegDDRef *Input, ArrayRef<Constant *> Mask,
                                 const StringRef &Name,
                                 RegDDRef *WLvalRef = nullptr);

  /// Extend the length of incoming vector \p Input to \p TargetLength using
  /// undefs. This function mimics the equivalent LLVM-IR version in
  /// VectorUtils.cpp. Example -
  /// {0, 1, 2, 3} -> TargetLen = 8 -> { 0, 1, 2, 3, undef, undef, undef, undef}
  HLInst *extendVector(RegDDRef *Input, unsigned TargetLength);

  /// Helper method to replicate contents of \p Input vector by \p
  /// ReplicationFactor number of times. This function mimics the equivalent
  /// LLVM-IR version in VectorUtils.cpp. Example -
  /// {v0, v1, v2, v3} -> RF = 2 -> { v0, v1, v2, v3, v0, v1, v2, v3 }
  HLInst *replicateVector(RegDDRef *Input, unsigned ReplicationFactor);

  /// Helper method to replicate each element of \p Input vector by \p
  /// ReplicationFactor number of times. This function mimics the equivalent
  /// LLVM-IR version in VectorUtils.cpp. Example -
  /// {v0, v1, v2, v3} -> RF = 2 -> { v0, v0, v1, v1, v2, v2, v3, v3 }
  HLInst *replicateVectorElts(RegDDRef *Input, unsigned ReplicationFactor);

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

    assert(Lane < getVF() && "Invalid lane ID.");
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

  // Generate and return an HLInst to initialize the given reference with undef
  // value. The generated HLInst is used to avoid unnecessary false dependences.
  HLInst *generateInitWithUndef(const RegDDRef *Ref) {
    RegDDRef *UndefRef = DDRefUtilities.createUndefDDRef(Ref->getDestType());
    return HLNodeUtilities.createCopyInst(UndefRef, "undef.init", Ref->clone());
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
        auto InitInst = generateInitWithUndef(LvalRef);
        // We handle only innermost loop vectorization today, so initialize
        // temp in MainLoop header.
        HLNodeUtils::insertAsFirstChild(MainLoop, InitInst);
      }
    }

    // TODO - not needed once search loop representation is made explicit
    if (!InsertPoint) {
      if (auto InsertRegion = dyn_cast<HLLoop>(getInsertRegion()))
        HLNodeUtils::insertAsLastChild(InsertRegion, Node);
      else if (isa<HLIf>(getInsertRegion()))
        addInst(Node, Mask, true);
      return;
    }

    HLNodeUtils::insertAfter(InsertPoint, Node);
    InsertPoint = Node;
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

  // Add the HLNodes in the list at the current insertion point. HLNodes are
  // expected to be masked appropriately coming in if necessary.
  void addInst(HLContainerTy *List) {
    HLNode *Save = &List->back();
    HLNodeUtils::insertAfter(InsertPoint, List);
    InsertPoint = Save;
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

  // Widen Ref to specified VF if needed and return the widened ref.
  RegDDRef *widenRef(const RegDDRef *Ref, unsigned VF,
                     bool LaneZeroOnly = false);

  // Return the widened DDRef corresponding to VPVal - when we enable full
  // VPValue based codegen, this function will generate the widened DDRef
  // if one is not found.
  RegDDRef *widenRef(const VPValue *VPVal, unsigned VF);

  // Returns the expression IV + <0, 1, .., VF-1> where IV is the current loop's
  // main induction variable.
  RegDDRef *generateLoopInductionRef(Type *RefDestTy);

  // Given a pointer ref that is a selfblob, create and return memory reference
  // for PtrRef[Index]. NumElements if greater than 1 is used to set the
  // destination type of canon expr corresponding to Index appropriately.
  RegDDRef *createMemrefFromBlob(RegDDRef *PtrRef, int Index,
                                 unsigned NumElements);

  // Returns the widened address-of DDRef for a pointer. The base pointer is
  // replicated and flattened if we are dealing with re-vectorization scenarios.
  // In the generated code, the returned DDRef is itself used as operand for
  // Scatter/Gather memrefs.
  RegDDRef *getWidenedAddressForScatterGather(const VPValue *VPPtr,
                                              Type *ScallarAccessType);

  // Given a load/store instruction, setup and return the memory ref to use in
  // generating the load/store HLInst. The given load/store instruction is also
  // used to get the symbase and alignment to set for the returned ref. It is
  // also used to set the alias analysis metadata for the returned ref.
  // Lane0Value specifies if we need memory ref corresponding to just vector
  // lane 0. This is used to handle uniform memory accesses. If the pointer
  // operand of the load/store instruction is unit strided(stride of 1/-1), the
  // ref returned is properly adjusted to enable wide load/store.
  // TODO: Do we need this function to generate scalar memrefs for other lanes
  // during serialization?
  RegDDRef *getMemoryRef(const VPLoadStoreInst *VPLdSt,
                         bool Lane0Value = false);

  /// Given VPVLSLoad/VPVLSStore setup the memory ref to be used as wide memory
  /// operation.
  template <class VLSOpTy>
  RegDDRef *getVLSMemoryRef(const VLSOpTy *LoadStore);

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
  bool targetHasIntelAVX512() const;

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

  // Return true if the call will be ignored by CG i.e. not emitted in outgoing
  // code.
  // TODO: This is a temporary workaround to handle deficiencies between
  // VPValue-based CG and mixed CG. It should be removed when mixed CG is
  // completely retired.
  bool isIgnoredCall(const CallInst *Call) {
    Intrinsic::ID ID = getVectorIntrinsicIDForCall(Call, TLI);
    return ID == Intrinsic::lifetime_start || ID == Intrinsic::lifetime_end;
  }

private:
  // Target Library Info is used to check for svml.
  TargetLibraryInfo *TLI;

  // Target Transform Info is used to check whether first iteration peeling is
  // needed
  TargetTransformInfo *TTI;

  HIRSafeReductionAnalysis *SRA;

  // VPlan for which vector code is being generated.
  const VPlanVector *Plan;

  // VPLoop being vectorized - assumes VPlan contains one loop.
  const VPLoop *VLoop;

  // OPTVLS analysis.
  VPlanVLSAnalysis *VLSA;

  // Current function.
  Function &Fn;

  // LLVMContext associated with current function.
  LLVMContext &Context;

  // Current insertion point
  HLNode *InsertPoint = nullptr;

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

  OptReportBuilder &ORBuilder;
  OptReportStatsTracker OptRptStats;

  // Map of DDRef symbase and widened ref
  DenseMap<unsigned, RegDDRef *> WidenMap;
  DenseMap<const VPValue *, RegDDRef *> VPValWideRefMap;

  // Map of scalar refs for VPValue + vector Lane
  DenseMap<const VPValue *, DenseMap<unsigned, RegDDRef *>> VPValScalRefMap;

  // Map of SCEV expression and widened DDRef.
  DenseMap<const SCEV *, RegDDRef *> SCEVWideRefMap;

  // The insertion points for reduction initializer and reduction last value
  // compute instructions.
  HLLoop *RednHoistLp = nullptr;
  HLNode *RednInitInsertPoint = nullptr;
  HLNode *RednFinalInsertPoint = nullptr;

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
  // and unique symbase created to represent accesses within vector loop.
  DenseMap<const VPAllocatePrivate *, std::pair<BlobDDRef *, unsigned>>
      PrivateMemBlobRefs;

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

  // Track HIR temp created for each deconstructed PHI ID.
  SmallDenseMap<int, RegDDRef *> PhiIdLValTempsMap;

  // True if #pragma omp simd defined for OrigLoop
  bool IsOmpSIMD;

  void setOrigLoop(HLLoop *L) { OrigLoop = L; }
  void setPeelLoop(HLLoop *L) { PeelLoop = L; }
  void setMainLoop(HLLoop *L) { MainLoop = L; }
  void setNeedPeelLoop(bool NeedPeel) { NeedPeelLoop = NeedPeel; }
  void setNeedRemainderLoop(bool NeedRem) { NeedRemainderLoop = NeedRem; }
  void setTripCount(uint64_t TC) { TripCount = TC; }
  void setVF(unsigned V) { VF = V; }
  void setUF(unsigned U) { UF = U == 0 ? 1 : U; }

  void insertReductionInit(HLContainerTy *List);
  void insertReductionFinal(HLContainerTy *List);

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
    case Instruction::Trunc:
    case Instruction::ZExt:
      return true;
    default:
      return false;
    }
  }

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
  void widenNodeImpl(const HLInst *Inst, RegDDRef *Mask,
                     const VPInstruction *VPInst);

  // Helper utility to get result type corresponding to \p RefTy based on \p
  // Widen.
  Type *getResultRefTy(Type *RefTy, unsigned VF, bool Widen) {
    return Widen ? getWidenedType(RefTy, VF) : RefTy;
  }

  // Helper utility to make a DDRef \p Ref  which is not attached to a HLDDNode
  // consistent and map the same to \p VPInst based on \p Widen. If the
  // instruction for which the Ref was created is also live-out, then we emit an
  // explicit copy operation at the current insertion point in HLLoop to prevent
  // invalid folding during liveout finalization. Note that this utility is
  // expected to be used whenever VPInst is lowered to a standalone DDRef that
  // will be used to fold operations.
  void makeConsistentAndAddToMap(RegDDRef *Ref, const VPInstruction *VPInst,
                                 SmallVectorImpl<const RegDDRef *> &AuxRefs,
                                 bool Widen, unsigned ScalarLaneID);

  // Implementation of generating needed HIR constructs for the given
  // VPInstruction. We generate new RegDDRefs or HLInsts that correspond to
  // the given VPInstruction. Widen parameter is used to specify if we are
  // generating VF wide constructs. If Widen is false, we generate scalar
  // constructs for lane given in ScalarLaneID.
  void generateHIR(const VPInstruction *VPInst, RegDDRef *Mask, bool Widen,
                   unsigned ScalarLaneID = -1);

  // Wrapper used to call generateHIR appropriately based on nature of given
  // VPInstruction.
  void widenNodeImpl(const VPInstruction *VPInst, RegDDRef *Mask);

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
  // uniform. VPLoad is the load instruction and Mask specifies the load Mask. A
  // scalar load is done using the pointer operand and the value is
  // broadcast to generate the vector value. If Mask is non-null, the load
  // is done conditionally only if Mask is non-zero.
  void widenUniformLoadImpl(const VPLoadStoreInst *VPLoad, RegDDRef *Mask);

  // Implementation of store where the pointer operand being stored to is
  // uniform and the store is unmasked. VPStore is the store instruction. A
  // scalar store is done using the pointer operand and the value to be stored
  // is extracted from the last vector lane if the value is divergent.If the
  // value being stored is uniform, we simply store the scalar uniform value.
  void widenUnmaskedUniformStoreImpl(const VPLoadStoreInst *VPStore);

  // Implementation of load/store widening.
  void widenLoadStoreImpl(const VPLoadStoreInst *VPLoadStore, RegDDRef *Mask);

  // Implementation of codegen for subscript instruction.
  void generateHIRForSubscript(const VPSubscriptInst *VPSubscript,
                               RegDDRef *Mask, bool Widen,
                               unsigned ScalarLaneID);

  // Get a vector of pointers corresponding to the private variable for each
  // vector lane.
  RegDDRef *createVectorPrivatePtrs(const VPAllocatePrivate *VPPvt);

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
  // if (IsLinearIndex) {
  //   // here parent is main component of min/max + index idiom
  //   %bcst = broadcast parent_final
  //   %cmp_v = cmp eq %bcst, parent_exit
  //   %ndx_fixup = select %cmp_v  %reduce_val, (is_min(pred) ? MAX_INT:MIN_INT)
  //   %result = (is_min(pred) ? reduce_min: reduce_max)(%ndx_fixup)
  // } else {
  //   // here parent is linear index reduction
  //   %bcst = broadcast parent_final
  //   %cmp_v = cmp eq %bcst, parent_exit
  //   %ndx = cttz(cmp_v)
  //   %result =  extract_element reduce_val, %ndx
  // }
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

  // Helper utility to generate a widened Call HLInst for given VPCall
  // instruction. Call arguments are widened in this utility (scalarized where
  // applicable) and Mask is used to adjust the arguments accordingly. Currently
  // this utility can handle call vectorization scenarios based on vector
  // library or trivial vector intrinsics.
  // TODO: Extend this utility to support call vectorization using
  // vector-variants.
  HLInst *generateWideCall(const VPCallInstruction *VPCall, RegDDRef *Mask,
                           Intrinsic::ID VectorIntrinID);

  // Helper utility to generate a scalar Call HLInst for given VPCall
  // instruction. Scalar values of the call arguments for given vector lane are
  // obtained in this utility.
  HLInst *generateScalarCall(const VPCallInstruction *VPCall, unsigned LaneID);

  // Get scalar version of RegDDRef that represents the VPValue \p VPVal
  // which is either an external definition or a constant. We can build the
  // scalar version from underlying HIR operand attached to VPValue.
  RegDDRef *getUniformScalarRef(const VPValue *VPVal);

  // Get scalar version of RegDDRef that represents the VPValue \p VPVal for
  // lane \p ScalarLaneID from VPValScalRefMap. If not found in the map,  we can
  // obtain the scalar ref using getUniformScalarRef for external definitions
  // and constants. For others, we create an extract element instruction for
  // lane ScalarLaneID from the wide reference and return the result of the
  // extract.
  RegDDRef *getOrCreateScalarRef(const VPValue *VPVal, unsigned ScalarLaneID);

  // Wrapper utility method to obtain RegDDRef corresponding to given VPValue
  // based on the Widen boolean.
  RegDDRef *getOrCreateRefForVPVal(const VPValue *VPVal, bool Widen,
                                   unsigned ScalarLaneID) {
    return Widen ? widenRef(VPVal, getVF())
                 : getOrCreateScalarRef(VPVal, ScalarLaneID);
  }

  RegDDRef *getVLSLoadStoreMask(VectorType *WideValueType, int GroupSize);

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

  void makeSymLiveInForParentLoops(unsigned Sym) {
    auto *ParentLoop = MainLoop->getParentLoop();
    while (ParentLoop != nullptr) {
      ParentLoop->addLiveInTemp(Sym);
      ParentLoop = ParentLoop->getParentLoop();
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
