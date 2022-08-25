//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   IntelLoopVectorizerCodeGen.h -- LLVM IR Code generation
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPOCODEGEN_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPOCODEGEN_H

#include "IntelLoopVectorizationPlanner.h"
#include "IntelVPlan.h"
#include "IntelVPlanOptrpt.h"

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Vectorize.h"

namespace llvm {

class TargetTransformInfo;
class TargetLibraryInfo;
class LoopInfo;
class Function;
struct VFInfo;
class LLVMContext;
class OVLSGroup;

namespace vpo {

class VPlanVLSAnalysis;

// LVCodeGen generates vector code by widening of scalars into
// appropriate length vectors.

class VPOCodeGen {
public:
  VPOCodeGen(Loop *OrigLoop, LLVMContext &Context,
             PredicatedScalarEvolution &PSE, LoopInfo *LI, DominatorTree *DT,
             TargetLibraryInfo *TLI, unsigned VecWidth, unsigned UnrollFactor,
             VPOVectorizationLegality *LVL, VPlanVLSAnalysis *VLSA,
             const VPlanVector *Plan, OptReportBuilder &ORBuilder,
             bool IsOmpSIMD = false,
             FatalErrorHandlerTy FatalErrorHandler = nullptr)
      : OrigLoop(OrigLoop), PSE(PSE), LI(LI), DT(DT), TLI(TLI), Legal(LVL),
        VLSA(VLSA), VPAA(*Plan->getVPSE(), *Plan->getVPVT(), VecWidth),
        Plan(Plan), VF(VecWidth), UF(UnrollFactor), Builder(Context),
        OrigPreHeader(OrigLoop->getLoopPreheader()), ORBuilder(ORBuilder),
        IsOmpSIMD(IsOmpSIMD), FatalErrorHandler(FatalErrorHandler) {}

  ~VPOCodeGen() { assert(VFStack.empty() && "expected empty VF stack"); }

  /// Initiate the scalar selects set.
  void initOpenCLScalarSelectSet(ArrayRef<const char *> OpenCLScalarSelects);

  // Take care of phi's to fix: reduction, 1st-order-recurrence, loop-closed.
  void finalizeLoop();

  // Create the vector loop skeleton which iterates from StartIndex
  // to StartIndex +  VF * Stride * TripCount. We also setup the control
  // flow such that the scalar loop is skipped.
  void createEmptyLoop();

  // Central entry point into codegen for lowering a VPInstruction into outgoing
  // LLVM-IR.
  void processInstruction(VPInstruction *VPInst);

  // Set current debug location for vector loop's IRBuilder. This location is
  // set for all instructions that are subsequently created using the Builder.
  void setBuilderDebugLoc(DebugLoc L) { Builder.SetCurrentDebugLocation(L); }

  // Attach metadata to \p Memref instruction to indicate that for the best
  // performance it needs to be aligned by at least \p PreferredAlignment bytes.
  // The main purpose of the metadata is to let OpenCL compiler peel kernel
  // iterations for the best memory access alignment.
  void attachPreferredAlignmentMetadata(Instruction *Memref,
                                        Align PreferredAlignment);

  IRBuilder<> &getBuilder() { return Builder; }

  BasicBlock *getLoopVectorPH() { return LoopVectorPreHeader; }
  BasicBlock *getOrigScalarExit() { return LoopExitBlock; }

  Loop *getMainLoop() const { return NewLoop; }
  Loop *getOrigLoop() const { return OrigLoop; }
  unsigned getVF() const { return VF; }
  unsigned getUF() const { return UF; }
  bool getIsOmpSIMD() const { return IsOmpSIMD; }
  bool getNeedRemainderLoop() const { return false; }
  Loop *getRemainderLoop() const { return nullptr; }

  // Get the widened vector value for given value V. If the scalar value
  // has not been widened, we widen it by VF and store it in WidenMap
  // before returning the widened value.
  Value *getVectorValue(VPValue *V);

  // Get a vector of pointers corresponding to the private variable for each
  // vector lane.
  Value *createVectorPrivatePtrs(VPAllocatePrivate *V);

  /// Return a value in the new loop corresponding to \p V from the original
  /// loop at vector index \p Lane. If the value has
  /// been vectorized but not scalarized, the necessary extractelement
  /// instruction will be generated.
  Value *getScalarValue(VPValue *V, unsigned Lane);

  /// MaskValue setter
  void setMaskValue(Value *MV) { MaskValue = MV; }

  /// Vectorize call arguments, or for simd functions and trivial vector
  /// intrinsics scalarize if the arg is linear/uniform/always scalar. If the
  /// call is being pumped by \p PumpFactor times, then the appropriate
  /// sub-vector is extracted for given \p PumpPart.
  void vectorizeCallArgs(VPCallInstruction *VPCall, const VFInfo *VecVariant,
                         Intrinsic::ID VectorIntrinID, unsigned PumpPart,
                         unsigned PumpFactor, SmallVectorImpl<Value *> &VecArgs,
                         SmallVectorImpl<Type *> &VecArgTys,
                         SmallVectorImpl<AttributeSet> &VecArgAttrs);

  /// Promote provided mask to a proper type and add it into
  /// vector arguments/vector argument types arrays.
  void createVectorMaskArg(VPCallInstruction *VPCall, const VFInfo *VecVariant,
                           SmallVectorImpl<Value *> &VecArgs,
                           SmallVectorImpl<Type *> &VecArgTys,
                           unsigned PumpedVF, Value *MaskToUse);

  // Return true if the argument at position /p Idx for function /p FnName is
  // scalar.
  bool isScalarArgument(StringRef FnName, unsigned Idx);

  /// Set transform state
  void setTransformState(struct VPTransformState *SP) { State = SP; }

  /// Add to WidenMap
  void addToWidenMap(VPValue *Key, Value *Data) { VPWidenMap[Key] = Data; }

  /// \Returns the widened value that corresponds to key or nullptr if not
  /// found.
  Value *getWidenedValue(VPValue *Key) {
    auto It = VPWidenMap.find(Key);
    return It != VPWidenMap.end() ? It->second : nullptr;
  }

  VPlanVLSAnalysis *getVLS() { return VLSA; }

  void setVPlan(const VPlanVector *P) { Plan = P; }

  OptReportStatsTracker &getOptReportStats(VPInstruction *I) {
    auto *BB = I->getParent();
    auto *VPLp = Plan->getVPLoopInfo()->getLoopFor(BB);
    // TODO: For instructions that don't belong to loop, need to find associated
    // concrete VPLoop and report stats there. For now we ignore these
    // instructions by collecting the stats in NonLoopInstStats.
    if (!VPLp)
      return NonLoopInstStats;

    return Plan->getOptRptStatsForLoop(VPLp);
  }

  /// Lower opt-report remarks collected in VPlan data structures to outgoing
  /// IR.
  void lowerVPlanOptReportRemarks() { lowerRemarksForVectorLoops(); }

  /// Clone the given scalar loop \p OrigLP and insert the cloned loop on the
  /// edge between NewLoopPred and NewLoopSucc. The NewLoopPred and NewLoopSucc
  /// should be connected directly and that is asserted by the function.
  //  TODO: This function will most likely not work for multi-exit loops. The
  //  full support for cloning such loops would have to be tested before this
  //  function is used for cloning such loops.
  template <class VPPeelRemainderTy>
  Loop *cloneScalarLoop(Loop *OrigLP, BasicBlock *NewLoopPred,
                        BasicBlock *NewLoopSucc, VPPeelRemainderTy *LoopInst,
                        const Twine &Name = "cloned.loop");

private:
  /// Helper function to process SOA layout for private final arrays and
  /// generate exit basic block as a result.
  BasicBlock *processSOALayout(VPAllocatePrivate *Priv, Value *Orig,
                               Type *ElementType, Value *ElementPosition);
  /// Return true if instruction \p V needs scalar code generated, i.e. is
  /// used in scalar context after vectorization.
  bool needScalarCode(VPInstruction *V);

  /// Return true if instruction \p V needs vector generated, i.e. is
  /// used in vector context after vectorization.
  bool needVectorCode(VPValue *V) { return true; }

  // Return the trip count for the scalar loop.
  Value *getTripCount() const { return TripCount; }

  /// Returns (and creates if needed) the original loop trip count.
  Value *getOrCreateTripCount(Loop *L, IRBuilder<> &Builder);

  /// Returns (and creates if needed) the trip count of the widened loop.
  Value *getOrCreateVectorTripCount(Loop *L, IRBuilder<> &Builder);

  // Generate vector code for given instruction based on results from VPlan
  // ScalVec analysis.
  void generateVectorCode(VPInstruction *VPInst);

  // Vectorize the given instruction that cannot be widened using serialization.
  // This is done using a sequence of extractelement, Scalar Op, InsertElement
  // instructions.
  void serializeInstruction(VPInstruction *VPInst);

  // Generate scalar code for given instruction for first/last lanes (0 / VF-1)
  // based on results from VPlan ScalVec analysis.
  void generateScalarCode(VPInstruction *VPInst);

  /// Helper function to generate and insert a scalar LLVM instruction from
  /// VPInstruction based on its opcode and scalar versions of its operands.
  // TODO: Currently we don't populate IR flags/metadata information for the
  // instructions generated below. Update after VPlan has internal
  // representation for them.
  Value *generateSerialInstruction(VPInstruction *VPInst,
                                   ArrayRef<Value *> Ops);

  /// Wrapper helper for generateSerialInstruction to accept range of Values
  /// instead of ArrayRef.
  template <class RangeTy>
  Value *generateSerialInstruction(VPInstruction *VPInst, RangeTy Ops) {
    SmallVector<Value *, 6> OpsVec(Ops.begin(), Ops.end());
    return generateSerialInstruction(VPInst, ArrayRef<Value *>(OpsVec));
  }

  /// Serialize instruction that requires predication.
  void serializeWithPredication(VPInstruction *VPInst);

  /// Create a series of instruction resulting in !all-zero(current-block-mask).
  Value *getMaskNotAllZero();

  /// Specialized method to handle predication of a uniform instruction. This
  /// function generates a single scalar instruction predicated by a
  /// not all-zero check.
  /// of the current MaskValue.
  // Example for Load instruction:
  //
  // Incoming scalar pseudo IR -
  // loop.body:
  //   ...
  //   %cond = icmp ...
  //   br %cond, %if.then, %loop.latch
  //
  // if.then:
  //   %uniform.gep = getelementptr ...
  //   %uniform.load = load %uniform.gep
  //   %user = add %uniform.load, %loop.iv
  //   br %loop.latch
  //
  // ...
  //
  // Vector pseudo IR emitted by this method -
  // vector.body:
  //   ...
  //   %wide.cond = icmp <VF x Ty>
  //   %uniform.gep = getelementptr ...
  //   %bitcast.cond = bitcast <VF x i1> %wide.cond to iVF
  //   %not.all.zero = icmp ne %bitcast.cond, 0
  //   %load = load %uniform.gep
  //   %bcast = <insert + shuffle> bcast %load
  //   %wide.user = add <VF x Ty> %bcast, %wide.loop.iv
  //   ...
  //
  // This is finally modified by predicateInstructions as -
  // vector.body:
  //   ...
  //   %wide.cond = icmp <VF x Ty>
  //   %uniform.gep = getelementptr ...
  //   %bitcast.cond = bitcast <VF x i1> %wide.cond to iVF
  //   %not.all.zero = icmp ne %bitcast.cond, 0
  //   br %not.all.zero, %pred.load.if, %merge
  //
  // pred.load.if:
  //   %load = load %uniform.gep
  //   %insert = insert load to undef vector
  //
  // merge:
  //   %merge.phi = [undef, %vector.body], [%insert, %pred.load.if]
  //   br %pred.load.continue
  //
  // pred.load.continue:
  //   %bcast = shufflevector %merge.phi
  //   %wide.user = add <VF x Ty> %bcast, %wide.loop.iv
  //
  void serializePredicatedUniformInstruction(VPInstruction *VPInst);

  /// Specialized method to handle uniform calls that are not widened during CG,
  /// for example kernel-convergent and kernel-uniform call and uniform call
  /// with no side-effects. Generates single call instruction predicated by a
  /// not all-zero check of the current mask value.
  void processPredicatedNonWidenedUniformCall(VPInstruction *VPInst);

  /// Predicate conditional instructions that require predication on their
  /// respective conditions.
  void predicateInstructions();

  /// Reverse vector elements
  Value *reverseVector(Value *Vec, unsigned Stride = 1);

  /// Insert the new loop to the loop hierarchy and pass manager
  /// and update the analysis passes.
  void updateAnalysis();

  /// Get the Function-entry block.
  BasicBlock &getFunctionEntryBlock() const;

  /// This function adds (StartIdx, StartIdx + Step, StartIdx + 2*Step, ...)
  /// to each vector element of Val. The sequence starts at StartIndex.
  Value *getStepVector(Value *Val, int StartIdx, Value *Step,
                       Instruction::BinaryOps BinOp);

  void fixNonInductionVPPhis();

  /// Return true if \p FnName is the name of an OpenCL scalar select and \p Idx
  /// is the position of the mask argument.
  bool isOpenCLSelectMask(StringRef FnName, unsigned Idx);

  /// Return the right vector mask for a OpenCL vector select build-in.
  Value *getOpenCLSelectVectorMask(VPValue *ScalarMask);

  /// Generate code for the VPPeelCount instruction.
  Value *codeGenVPInvSCEVWrapper(VPInvSCEVWrapper *SW);

  /// Vectorize VPInstruction that corresponds to scalar peel/remainder.
  template <class VPPeelRemainderTy>
  void vectorizeScalarPeelRem(VPPeelRemainderTy *LoopReuse);

  /// Generate vector code for reduction finalization.
  /// The final vector reduction value is reduced horizontally using
  /// "llvm.experimental.vector.reduce" intrinsics.
  /// If accumulator value is present, then a final scalar operation is
  /// also performed.
  // Example -
  // i32 % vp1 = reduction-final{u_add} i32 %vp.red.add
  //
  // Generated instruction-
  // %red.lvc = @llvm.experimental.vector.reduce.add.v4i32 (%vec.red.add)
  void vectorizeReductionFinal(VPReductionFinal *RedFinal);

  /// Generate vector code for reduction finalization of select-compare.
  /// There is no horizontal reduction intrinsic for this case, so it
  /// requires separate handling from vectorizeReductionFinal.
  // Example -
  // i32 %vp1 = reduction-final{u_icmp} i32 %vpexit i32 %vpstart i32 %vpchg
  //
  // Generated instructions for VF=4 -
  // %cmp = icmp ne <4 x i32> %vpexit', <i32 %vpstart, ..., i32 %vpstart>
  // %or = call i1 @llvm.vector.reduce.or.v4i1(<4 x i1> %cmp)
  // %sel = select i1 %or, i32 %vpchg, i32 %vpstart
  void vectorizeSelectCmpReductionFinal(VPReductionFinal *RedFinal);

  /// Generate vector code for induction initialization.
  /// InductionInit has two arguments {Start, Step} and keeps the operation
  /// opcode. We generate
  /// For +/-   : broadcast(start) +/GEP step*{0, 1,..,VL-1} (GEP for pointers)
  /// For */div : broadcast(start) * pow(step,{0, 1,..,VL-1})
  /// Other binary operations are not induction-compatible.
  void vectorizeInductionInit(VPInductionInit *VPInst);

  /// Generate vector code for induction-step initialization.
  /// Induction's step for vector loop will be
  /// For +/-    : VF * step
  /// For */div  : pow(step, VF)
  /// This value is broadcasted on all lanes.
  void vectorizeInductionInitStep(VPInductionInitStep *VPInst);

  /// Generate vector code for induction finalization.
  /// Last-value computation for inductions depends on its opcode. We generate
  /// For */div         : extractelement(ind.vec, VF-1)
  /// For all others    : start OP step*rounded_tc
  /// TODO: need to decide about masked calculation for early-exit loops.
  void vectorizeInductionFinal(VPInductionFinal *VPInst);

  /// Vectorize private memory creating alloca.
  /// Create a new widened alloca in the function entry BB. We allocate VF
  /// elements of the private element type.
  void vectorizeAllocatePrivate(VPAllocatePrivate *V);

  /// Vectorize unconditional last private final value calculation.
  void vectorizePrivateFinalUncond(VPInstruction *VPInst);

  /// Vectorize running inclusive reduction.
  void vectorizeRunningInclusiveReduction(
    VPRunningInclusiveReduction *InscanRed);

  /// Vectorize extract last vector lane instruction.
  Value *vectorizeExtractLastVectorLane(VPInstruction *VPInst);

  /// Vectorize blend instructions using selects.
  void vectorizeBlend(VPBlendInst *Blend);

  // In the original loop header phis remove operands coming from original
  // preheader.
  void unlinkOrigHeaderPhis();

  // Drop values generated for externals (VPExternalDef, VPConstant etc) from
  // value maps. This is done when we encounter VPPushVF/VPPopVF so we don't
  // use values geneated for different loops/VPlans.
  void dropExternalValsFromMaps();

  // Create broadcast of the external into vector with CurVF length. The
  // broadcast is created at the current insertion point.
  // No internal VPValue->Value maps are updated.
  Value* getVectorValueForExternal(VPValue* V, unsigned CurVF);

  /// The original loop.
  Loop *OrigLoop;

  /// Vectorized loop
  Loop *NewLoop = nullptr;

  /// A wrapper around ScalarEvolution used to add runtime SCEV checks. Applies
  /// dynamic knowledge to simplify SCEV expressions and converts them to a
  /// more usable form.
  PredicatedScalarEvolution &PSE;
  /// Loop Info.
  LoopInfo *LI;
  /// Dominator Tree.
  DominatorTree *DT;

  /// Target Library Info.
  TargetLibraryInfo *TLI;

  /// Vectorization Legality.
  VPOVectorizationLegality *Legal;

  /// Variable-length stridedness analysis.
  VPlanVLSAnalysis *VLSA;

  /// Alignment Analysis.
  VPlanAlignmentAnalysis VPAA;

  /// VPlan for which vector code is generated, need to finalize external uses.
  const VPlanVector *Plan;

  // Loop trip count
  Value *TripCount = nullptr;
  // Vectorized loop trip count.
  Value *VectorTripCount = nullptr;

  // Vector factor or vector length to use. Each scalar instruction is widened
  // to operate on this number of operands.
  unsigned VF;

  // Unroll factor
  unsigned UF;

  // Last VPPushVF encountered.
  VPInstruction *LastPushVF = nullptr;

  // Stack of triple <vector factor, unroll vactor, VPInstruction *>
  SmallVector<std::tuple<unsigned, unsigned, VPInstruction *>, 2> VFStack;

  // Set of VPValues which we should erase from the maps of already generated
  // values when we encounter VPPushVF/VPPopVF instructions. The same
  // VPConstants and VPExternalDefs can be used in different VPlans. But
  // generating code for a VPlan we can't use the values generated in another
  // VPlan, due to VFs mismatch and/or domination reasons. The VPPushVF/VPPopVF
  // define the bounds between VPlans so we use them to drop those maps
  SmallSet<VPValue*, 16> VPValsToFlushForVF;

  // IR Builder to use to generate instructions
  IRBuilder<> Builder;

  // Flag to indicate that the original loop is used either as peel or
  // remainder.
  // In case it's not used, in the end of CG we need to update original header
  // block phi-s removing incoming values from the loop preheader. E.g. the
  // phi-s will remain in the form
  //   %indvars.iv = phi i64 [ %indvars.iv.next, %for.end ], [ 0, %preheader ]
  // but the %preheader is not a predecessor of the header anymore as we
  // replaced it by the vector loop.
  // When the original loop is used somehow we update those phis incoming values
  // in a different way (see fixNonInductionVPPhis()).
  bool OrigLoopUsed = false;

  /// Holds a mapping between the VPPHINode and the corresponding vector/scalar
  /// LLVM IR PHI node that needs fix up. The operands of the VPPHINode are used
  /// to setup the operands of the LLVM IR PHI node.
  DenseMap<PHINode *,
           std::pair<VPPHINode *, int /*Lane will be -1 for vectorized PHI*/>>
      PhisToFix;

  // Map of scalar VPValue and widened LLVM IR value.
  DenseMap<VPValue *, Value *> VPWidenMap;

  // Map of scalar values for VPValues.
  std::map<VPValue *, DenseMap<unsigned, Value *>> VPScalarMap;

  // Map to track widened alloca created for a private memory entity introduced
  // by VPlan. VPScalarMap cannot be reused for this since we want to track the
  // base pointer to widened memory that was allocated during CG. The scalar map
  // will contain corresponding pointer for each lane. Some motivating usecases
  // where base pointer to widened alloca is needed include unit-stride
  // load/store and OpenCL sincos vectorization. An example -
  //
  // Incoming IR - "QUAL.OMP.PRIVATE"(i32* %priv)
  // VPlan private memory - i32* %vp0 = allocate-private i32*
  //
  // Data structures status in CG for VF=4 -
  // LoopPrivateVPWidenMap[%vp0] ---> %ptr = alloca <4 x i32>
  // VPWidenMap[%vp0] ---> %vptr = <i32* ptr0, i32* ptr1, i32* ptr2, i32* ptr3>
  // VPScalarMap[%vp0][0] ---> extract %vptr, 0
  //
  // Suppose there is a user of %priv in incoming IR like -
  // %call = non_vectorizable_call(i32* %priv)
  // Then to serialize this call, we need i32* parameters from VPScalarMap. The
  // generated %ptr alloca above will be <4 x i32>*.
  DenseMap<VPValue *, Value *> LoopPrivateVPWidenMap;

  // Holds finalization VPInstructions generated for loop entities.
  MapVector<const VPLoopEntity *, VPInstruction *> EntitiesFinalVPInstMap;

  // --- Vectorization state ---

  /// The vector-loop preheader.
  BasicBlock *LoopVectorPreHeader = nullptr;
  /// The scalar-loop preheader.
  BasicBlock *LoopScalarPreHeader = nullptr;
  /// Middle Block between the vector and the scalar.
  BasicBlock *LoopMiddleBlock = nullptr;
  /// The ExitBlock of the scalar loop.
  BasicBlock *LoopExitBlock = nullptr;
  /// The vector loop body.
  BasicBlock *LoopVectorBody = nullptr;
  /// The scalar loop body.
  BasicBlock *LoopScalarBody = nullptr;
  /// Original preheader of the original loop. During CG, we can't use
  /// OrigLoop->getPreheader as we insert some basic blocks.
  BasicBlock *OrigPreHeader;
  /// Current mask value for instructions being generated
  Value *MaskValue = nullptr;
  /// A list of all bypass blocks. The first block is the entry of the loop.
  SmallVector<BasicBlock *, 4> LoopBypassBlocks;

  // Pointer to current transformation state - used to obtain VPBasicBlock to
  // BasicBlock mapping.
  struct VPTransformState *State = nullptr;

  OptReportStatsTracker NonLoopInstStats;

  // Set of scalar loop header blocks generated in outgoing vector code. We also
  // track the scalar loop VPInstruction to identify its type i.e. peel or
  // remainder.
  SmallVector<std::pair<VPInstruction *, BasicBlock *>, 2>
      OutgoingScalarLoopHeaders;

  // Get alignment for VPLoadStoreInst using underlying llvm::Instruction if it
  // exists, otherwise use the alignment of the instruction itself.
  Align getLoadStoreAlignment(const VPLoadStoreInst *VPInst);

  // Get alignment for the gather/scatter intrinsic when widening load/store
  // VPInstruction \p VPInst.
  // When we widen a load/store to gather/scatter, if the value to load/store
  // is a vector, then the alignment for gather/scatter should be adjusted
  // according to the alignment of the vector element. E.g., given VF=2,
  //   store <3 x i32> %val, <3 x i32>* %ptr, align 16
  // can be widened to
  //   call void @llvm.scatter(<6 x i32> %wide.val, <6 x i32*> %wide.ptr, ...)
  // Note that there is no <2 x <3 x i32>>, so %wide.ptr is firstly casted
  // from <2 x <3 x i32>*> to <6 x i32*>, i.e., two 16-byte aligned stores are
  // split to six 4-byte ones. And thus, the alignment for the scatter should
  // be adjusted to 4.
  Align getAlignmentForGatherScatter(const VPLoadStoreInst *VPInst);

  // Widen the given load instruction. EmitIntrinsic needs to be set to true
  // when we can start emitting masked_gather intrinsic once we have support
  // in code gen. Without code gen support, we will serialize the intrinsic.
  // As a result, we simply serialize the instruction for now.
  void vectorizeLoadInstruction(VPLoadStoreInst *VPLoad,
                                bool EmitIntrinsic = false);

  // Generate a wide (un)masked load for a given consecutive stride load.
  Value *vectorizeUnitStrideLoad(VPLoadStoreInst *VPLoad, bool IsNegOneStride,
                                 bool IsPvtPtr);

  // Widen the given store instruction. EmitIntrinsic needs to be set to true
  // when we can start emitting masked_scatter intrinsic once we have support
  // in code gen. Without code gen support, we will serialize the intrinsic.
  // As a result, we simply serialize the instruction for now.
  void vectorizeStoreInstruction(VPLoadStoreInst *VPStore,
                                 bool EmitIntrinsic = false);

  // Generate a wide (un)masked store for a given consecutive stride store.
  void vectorizeUnitStrideStore(VPLoadStoreInst *VPStore, bool IsNegOneStride,
                                bool IsPvtPtr);

  // Widen a BitCast/AddrSpaceCast instructions
  template <typename CastInstTy>
  void vectorizeCast(typename std::enable_if<
                     std::is_same<CastInstTy, BitCastInst>::value ||
                         std::is_same<CastInstTy, AddrSpaceCastInst>::value,
                     VPInstruction>::type *VPInst);

  // Widen ExtractElt instruction - loop re-vectorization.
  void vectorizeExtractElement(VPInstruction *VPInst);

  // Widen InsertElt instruction - loop re-vectorization.
  void vectorizeInsertElement(VPInstruction *VPInst);

  // Widen Shuffle instruction - loop re-vectorization.
  void vectorizeShuffle(VPInstruction *VPInst);
  // Get the mask of a VPInstruction representing shuffle (before widening) as a
  // vector of ints.
  SmallVector<int, 16> getVPShuffleOriginalMask(const VPInstruction *VPI);
  // Utility to get VPValue that is broadcasted if the input \p V is a splat
  // vector. Functionality is same as VectorUtils::getSplatValue.
  const VPValue *getOrigSplatVPValue(const VPValue *V);

  /// Adjust arguments passed to SVML functions to handle masks. \p
  /// CallMaskValue defines the mask being applied to the current SVML call
  /// instruction that is processed.
  void addMaskToSVMLCall(Function *OrigF, Value *CallMaskValue,
                         AttributeList OrigAttrs,
                         SmallVectorImpl<Value *> &VecArgs,
                         SmallVectorImpl<Type *> &VecArgTys,
                         SmallVectorImpl<AttributeSet> &VecArgAttrs);

  /// Generate instructions to extract two results of a sincos call, and store
  /// them to locations designated in the original call.
  void generateStoreForSinCos(VPCallInstruction *VPCall, Value *CallResult);

  // Helper utility to generate vector call(s) for given \p VPCall, using vector
  // library function, matched SIMD vector variant or vector intrinsics. The
  // generated call(s) are returned via \p CallResults.
  void generateVectorCalls(VPCallInstruction *VPCall, unsigned PumpFactor,
                           bool IsMasked, const VFInfo *MatchedVariant,
                           Intrinsic::ID VectorIntrinID,
                           SmallVectorImpl<Value *> &CallResults);

  // Helper utility to combine pumped call results into a single vector for
  // current VF context. If pumping is not done then the single element of \p
  // CallResults is trivially returned.
  Value *getCombinedCallResults(ArrayRef<Value *> CallResults);

  // Widen call instruction using vector library function. If given \p VPCall
  // cannot be widened for current VF, but can be pumped with lower VF, then
  // vectorize the call by breaking down the operands and pumping it multiple
  // times. Pumped call results are subsequently combined back to current VF
  // context. For example -
  // %1 = call float @sinf(float %0)
  //
  // for a VF=128, this call will be pumped 2-way as below -
  // %shuffle1 = shufflevector %0.vec, undef, <0, 1, ... 63>
  // %pump1 = call <64 x float> @__svml_sinf64(%shuffle1)
  // %shuffle2 = shufflevector %0.vec, undef, <64, 65, ... 127>
  // %pump2 = call <64 x float> @__svml_sinf64(%shuffle2)
  // %combine = shufflevector %pump1, %pump2, <0, 1, ... 127>
  void vectorizeLibraryCall(VPCallInstruction *VPCall);

  // Widen call instruction using vector intrinsic.
  void vectorizeTrivialIntrinsic(VPCallInstruction *VPCall);

  // Widen call instruction using a matched SIMD vector variant.
  // TODO: Add support for pumping.
  void vectorizeVecVariant(VPCallInstruction *VPCall);

  // Widen or Serialize lifetime_start/end intrinsic call.
  void vectorizeLifetimeStartEndIntrinsic(VPCallInstruction *VPCall);

  // Widen Select instruction.
  void vectorizeSelectInstruction(VPInstruction *VPInst);

  // Generate code for given VPPHINode transforming it either into PHINode or
  // into Select instruction (for blends).
  void vectorizeVPPHINode(VPPHINode *VPPhi);

  // Vectorize the call to OpenCL SinCos function with the vector-variant from
  // SVML
  void vectorizeOpenCLSinCos(VPCallInstruction *VPCall, bool IsMasked);

  /// Store instructions that should be predicated, as a pair
  ///   <StoreInst, Predicate>
  SmallVector<std::pair<Instruction *, Value *>, 4> PredicatedInstructions;

  /// Hold names of scalar select builtins
  SmallSet<std::string, 20> ScalarSelectSet;

  /// This function returns the widened GEP instruction for a pointer. In the
  /// generated code, the returned GEP is itself used as an operand of a
  /// Scatter/Gather function.
  Value *getWidenedAddressForScatterGather(VPValue *VPBasePtr,
                                           Type *ScalarAccessType);

  /// This function return an appropriate BasePtr for cases where we are have
  /// load/store to consecutive memory locations
  Value *createWidenedBasePtrConsecutiveLoadStore(VPValue *Ptr,
                                                  Type *ScalarAccessType,
                                                  bool Reverse);

  /// Create a mask to be used in @llvm.masked.[load|store] for the wide VLS
  /// memory operation. Returns nullptr if operation is unmasked.
  Value *getVLSLoadStoreMask(VectorType *WidevalueType, int GroupSize);

  /// Helper method to visit all VPLoops in final VPlan CFG and lower
  /// opt-reports attached to them to their corresponding loops in outgoing IR.
  void lowerRemarksForVectorLoops();

  /// Helper method to visit all outgoing scalar loops and emit opt-report
  /// remarks for them explicitly.
  // TODO: We are not able to attach these remarks earlier in pipeline since
  // scalar loops don't have VPLoops in CFG. Explore alternative approaches for
  // such loops.
  void emitRemarksForScalarLoops();

  /// Preserve DbgLoc metatdata from incoming loop's LoopID in outgoing
  /// vectorized loops. This ensures loop's LocRange info is not lost.
  void preserveLoopIDDbgMDs();

  /// Return a guaranteed peeling variant. Null is returned if we are not sure
  /// that the peel loop will be executed at run-time.
  VPlanPeelingVariant *getGuaranteedPeeling() const;

  DenseMap<AllocaInst *, Value *> ReductionEofLoopVal;
  DenseMap<AllocaInst *, Value *> ReductionVecInitVal;

  SmallDenseMap<const OVLSGroup *, Instruction *> VLSGroupLoadMap;

  /// Opt-report builder object that will be used to emit/lower remarks into
  /// outgoing loops.
  OptReportBuilder &ORBuilder;

  // True if #pragma omp simd defined for OrigLoop
  bool IsOmpSIMD;

  FatalErrorHandlerTy FatalErrorHandler;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPOCODEGEN_H
