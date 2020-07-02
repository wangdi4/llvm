//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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

#include "IntelVPlan.h"
#include "IntelVPlanOptrpt.h"

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

extern llvm::cl::opt<bool> EnableVPValueCodegen;

namespace llvm {

class TargetTransformInfo;
class TargetLibraryInfo;
class LoopInfo;
class Function;
class VectorVariant;
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
             TargetLibraryInfo *TLI, const TargetTransformInfo *TTI,
             unsigned VecWidth, unsigned UnrollFactor,
             VPOVectorizationLegality *LVL, VPlanVLSAnalysis *VLSA,
             const VPlan *Plan)
      : OrigLoop(OrigLoop), PSE(PSE), LI(LI), DT(DT), TLI(TLI), TTI(TTI),
        Legal(LVL), VLSA(VLSA), Plan(Plan), VF(VecWidth), UF(UnrollFactor),
        Builder(Context), PreferredPeeling(Plan->getPreferredPeeling(VF)) {}

  ~VPOCodeGen() {}

  /// Initiate the scalar selects set.
  void initOpenCLScalarSelectSet(ArrayRef<const char *> OpenCLScalarSelects);

  // Take care of phi's to fix: reduction, 1st-order-recurrence, loop-closed.
  void finalizeLoop();

  // Create the vector loop skeleton which iterates from StartIndex
  // to StartIndex +  VF * Stride * TripCount. We also setup the control
  // flow such that the scalar loop is skipped.
  void createEmptyLoop();

  // Set current debug location for vector loop's IRBuilder. This location is
  // set for all instructions that are subsequently created using the Builder.
  void setBuilderDebugLoc(DebugLoc L) { Builder.SetCurrentDebugLocation(L); }

  // Widen the given instruction to VF wide vector instruction
  void vectorizeInstruction(VPInstruction *VPInst);

  // Vectorize the given instruction that cannot be widened using serialization.
  // This is done using a sequence of extractelement, Scalar Op, InsertElement
  // instructions.
  void serializeInstruction(VPInstruction *VPInst);

  // Attach metadata to \p Memref instruction to indicate that for the best
  // performance it needs to be aligned by at least \p PreferredAlignment bytes.
  // The main purpose of the metadata is to let OpenCL compiler peel kernel
  // iterations for the best memory access alignment.
  void attachPreferredAlignmentMetadata(Instruction *Memref,
                                        Align PreferredAlignment);

  IRBuilder<> &getBuilder() { return Builder; }

  BasicBlock *getLoopVectorPH() { return LoopVectorPreHeader; }

  Loop *getMainLoop() const { return NewLoop; }
  unsigned getVF() const { return VF; }
  bool getNeedRemainderLoop() const { return false; }
  Loop *getRemainderLoop() const { return nullptr; }

  // Get the widened vector value for given value V. If the scalar value
  // has not been widened, we widen it by VF and store it in WidenMap
  // before returning the widened value
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
  void vectorizeCallArgs(VPCallInstruction *VPCall, VectorVariant *VecVariant,
                         Intrinsic::ID VectorIntrinID, unsigned PumpPart,
                         unsigned PumpFactor, SmallVectorImpl<Value *> &VecArgs,
                         SmallVectorImpl<Type *> &VecArgTys);

  // Return true if the argument at position /p Idx for function /p FnName is
  // scalar.
  bool isScalarArgument(StringRef FnName, unsigned Idx);

  /// Set transform state
  void setTransformState(struct VPTransformState *SP) { State = SP; }

  VPlanVLSAnalysis *getVLS() { return VLSA; }

  void setVPlan(const VPlan *P, const VPLoopEntityList *Entities) {
    Plan = P;
    VPEntities = Entities;
  }

  OptReportStatsTracker &getOptReportStatsTracker() { return OptRptStats; }

private:
  /// Return true if instruction \p V needs scalar code generated, i.e. is
  /// used in scalar context after vectorization.
  bool needScalarCode(VPInstruction *V);

  /// Return true if instruction \p V needs vector generated, i.e. is
  /// used in vector context after vectorization.
  bool needVectorCode(VPValue *V) { return true; }

  /// Emit a bypass check to see if the vector trip count is nonzero.
  void emitVectorLoopEnteredCheck(Loop *L, BasicBlock *Bypass);

  /// Check whether the original loop trip count \p Count is equal to vector
  /// loop trip count \p CountRoundDown. In this case we can bypass the scalar
  /// remainder.
  void emitEndOfVectorLoop(Value *Count, Value *CountRoundDown);

  // Return the trip count for the scalar loop.
  Value *getTripCount() const { return TripCount; }

  /// Returns (and creates if needed) the original loop trip count.
  Value *getOrCreateTripCount(Loop *L);

  /// Returns (and creates if needed) the trip count of the widened loop.
  Value *getOrCreateVectorTripCount(Loop *L);

  /// Helper function to generate and insert a scalar LLVM instruction from
  /// VPInstruction based on its opcode and scalar versions of its operands.
  // TODO: Currently we don't populate IR flags/metadata information for the
  // instructions generated below. Update after VPlan has internal
  // representation for them.
  Value *generateSerialInstruction(VPInstruction *VPInst,
                                   ArrayRef<Value *> ScalarOperands);

  /// Serialize instruction that requires predication.
  void serializeWithPredication(VPInstruction *VPInst);

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

  /// Specialized method for kernel-convergent and kernel-uniform call
  /// Generates single call instruction predicated by a not all-zero check
  /// of the current mask value.
  void processPredicatedKernelConvergentUniformCall(VPInstruction *VPInst);

  /// Predicate conditional instructions that require predication on their
  /// respective conditions.
  void predicateInstructions();

  /// Reverse vector elements
  Value *reverseVector(Value *Vec, unsigned Stride = 1);

  /// Make the needed fixups for all live out values.
  void fixOutgoingValues();

  /// Fix up reduction last value (link with remainder etc).
  void fixReductionLastVal(const VPReduction &Red, VPReductionFinal *RedFinal);

  /// Fix up live out value for a loop entity with finalization instruction \p
  /// FinalVPInst. NOTE: This fixup assumes that all external uses of VPEntity
  /// related instructions are replaced by its corresponding finalization
  /// VPInstruction.
  void fixLiveOutValues(VPInstruction *FinalVPInst, Value *LastVal);

  /// A part of fix up of last value. Creates a needed phi in intermediate
  /// block and updates phi in remainder.
  void createLastValPhiAndUpdateOldStart(Value *OrigStartValue, PHINode *Phi,
                                         const Twine &NameStr, Value *LastVal);
  /// Fix up induction last value.
  void fixInductionLastVal(const VPInduction &Ind, VPInductionFinal *IndFinal);

  /// The Loop exit block may have single value PHI nodes where the incoming
  /// value is 'Undef'. While vectorizing we only handled real values that were
  /// defined inside the loop. Here we fix the 'undef case'.
  void fixLCSSAPHIs();

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

  /// Generate vector code for reduction finalization.
  /// The final vector reduction value is reduced horizontally using
  /// "llvm.experimental.vector.reduce" intrinsics. The scalar result is
  /// recorded in VPEntities last-value tracking map to update out-of-the-loop
  /// uses. If accumulator value is present, then a final scalar operation is
  /// also performed.
  // Example -
  // i32 % vp1 = reduction-final{u_add} i32 %vp.red.add
  //
  // Generated instruction-
  // %red.lvc = @llvm.experimental.vector.reduce.add.v4i32 (%vec.red.add)
  void vectorizeReductionFinal(VPReductionFinal *RedFinal);

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

  /// Vectorize blend instructions using selects.
  void vectorizeBlend(VPBlendInst *Blend);

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
  /// Target Transform Info.
  const TargetTransformInfo *TTI;

  /// Vectorization Legality.
  VPOVectorizationLegality *Legal;

  /// Variable-length stridedness analysis.
  VPlanVLSAnalysis *VLSA;

  /// VPlan for which vector code is generated, need to finalize external uses.
  const VPlan *Plan;

  // Loop entities, for correct reductions processing
  const VPLoopEntityList *VPEntities = nullptr;

  // Loop trip count
  Value *TripCount = nullptr;
  // Vectorized loop trip count.
  Value *VectorTripCount = nullptr;

  // Vector factor or vector length to use. Each scalar instruction is widened
  // to operate on this number of operands.
  unsigned VF;

  // Unroll factor
  unsigned UF;

  // IR Builder to use to generate instructions
  IRBuilder<> Builder;

  /// Holds a mapping between the VPPHINode and the corresponding vector LLVM
  /// IR PHI node that needs fix up. The operands of the VPPHINode are used
  /// to setup the operands of the LLVM IR PHI node.
  DenseMap<VPPHINode *, PHINode *> PhisToFix;
  DenseMap<VPPHINode *, PHINode *> ScalarPhisToFix;

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

  // The selected peeling variant for the current VPlan and VF.
  VPlanPeelingVariant *PreferredPeeling = nullptr;

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
  /// Current mask value for instructions being generated
  Value *MaskValue = nullptr;
  /// A list of all bypass blocks. The first block is the entry of the loop.
  SmallVector<BasicBlock *, 4> LoopBypassBlocks;

  // Pointer to current transformation state - used to obtain VPBasicBlock to
  // BasicBlock mapping.
  struct VPTransformState *State = nullptr;

  OptReportStatsTracker OptRptStats;

  // Get alignment for load/store VPInstruction using underlying
  // llvm::Instruction.
  Align getOriginalLoadStoreAlignment(const VPInstruction *VPInst);

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
  Align getAlignmentForGatherScatter(const VPInstruction *VPInst);

  // Widen the given load instruction. EmitIntrinsic needs to be set to true
  // when we can start emitting masked_gather intrinsic once we have support
  // in code gen. Without code gen support, we will serialize the intrinsic.
  // As a result, we simply serialize the instruction for now.
  void vectorizeLoadInstruction(VPInstruction *VPInst,
                                bool EmitIntrinsic = false);

  // Generate a wide (un)masked load for a given consecutive stride load.
  Value *vectorizeUnitStrideLoad(VPInstruction *VPInst, bool IsNegOneStride,
                                 bool IsPvtPtr);

  // Widen the given store instruction. EmitIntrinsic needs to be set to true
  // when we can start emitting masked_scatter intrinsic once we have support
  // in code gen. Without code gen support, we will serialize the intrinsic.
  // As a result, we simply serialize the instruction for now.
  void vectorizeStoreInstruction(VPInstruction *VPInst,
                                 bool EmitIntrinsic = false);

  // Generate a wide (un)masked store for a given consecutive stride store.
  void vectorizeUnitStrideStore(VPInstruction *VPInst, bool IsNegOneStride,
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
                         SmallVectorImpl<Value *> &VecArgs,
                         SmallVectorImpl<Type *> &VecArgTys);

  /// Generate instructions to extract two results of a sincos call, and store
  /// them to locations designated in the original call.
  void generateStoreForSinCos(VPCallInstruction *VPCall, Value *CallResult);

  // Helper utility to generate vector call(s) for given \p VPCall, using vector
  // library function, matched SIMD vector variant or vector intrinsics. The
  // generated call(s) are returned via \p CallResults.
  void generateVectorCalls(VPCallInstruction *VPCall, unsigned PumpFactor,
                           bool IsMasked, VectorVariant *MatchedVariant,
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

  /// Helper internal method to determine if given VPValue \p V is a
  /// vectorizable load/store. A load/store is not vectorizable if it's not
  /// simple or if it operates on non-vectorizable types.
  // TODO : Move this to VPlanUtils when volatile/atomic property is represented
  // in VPInstruction.
  bool isVectorizableLoadStore(const VPValue *V);

  /// This function returns the widened GEP instruction for a pointer. In the
  /// generated code, the returned GEP is itself used as an operand of a
  /// Scatter/Gather function.
  Value *getWidenedAddressForScatterGather(VPValue *VPBasePtr);

  /// This function return an appropriate BasePtr for cases where we are have
  /// load/store to consecutive memory locations
  Value *createWidenedBasePtrConsecutiveLoadStore(VPValue *Ptr, bool Reverse);

  /// Create a wide load for the \p Group (or get existing one).
  Value *getOrCreateWideLoadForGroup(OVLSGroup *Group);

  /// Vectorize \p VPLoad instruction that is part of a \p Group.
  Value *vectorizeInterleavedLoad(VPInstruction *VPLoad, OVLSGroup *Group);

  /// Vectorize \p VPStore instruction that is part of a \p Group.
  void vectorizeInterleavedStore(VPInstruction *VPStore, OVLSGroup *Group);

  DenseMap<AllocaInst *, Value *> ReductionEofLoopVal;
  DenseMap<AllocaInst *, Value *> ReductionVecInitVal;

  SmallDenseMap<const OVLSGroup *, Instruction *> VLSGroupLoadMap;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPOCODEGEN_H
