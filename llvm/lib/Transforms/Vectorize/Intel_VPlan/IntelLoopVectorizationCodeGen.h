//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZERCODEGEN_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZERCODEGEN_H

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

class VPValue;
class VPInstruction;
class VPPHINode;
class VPlanVLSAnalysis;
struct VPTransformState;
class VPOVectorizationLegality;
class VPlan;
class VPReductionFinal;
class VPPrivateMemory;
class VPInductionInit;
class VPInductionInitStep;
class VPInductionFinal;
class VPAllocatePrivate;
class VPLoopEntityList;
class VPLoopEntity;
class VPReduction;
class VPInduction;
class VPGEPInstruction;

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
        Legal(LVL), VLSA(VLSA), Plan(Plan), VPEntities(nullptr),
        TripCount(nullptr), VectorTripCount(nullptr), Induction(nullptr),
        VF(VecWidth), UF(UnrollFactor), Builder(Context),
        LoopVectorPreHeader(nullptr), LoopScalarPreHeader(nullptr),
        LoopMiddleBlock(nullptr), LoopExitBlock(nullptr),
        LoopVectorBody(nullptr), LoopScalarBody(nullptr), MaskValue(nullptr) {}

  ~VPOCodeGen() {}

  /// Initiate the scalar selects set.
  void initOpenCLScalarSelectSet(ArrayRef<const char *> OpenCLScalarSelects);

  // Take care of phi's to fix: reduction, 1st-order-recurrence, loop-closed.
  void finalizeLoop();

  // Create the vector loop skeleton which iterates from StartIndex
  // to StartIndex +  VF * Stride * TripCount. We also setup the control
  // flow such that the scalar loop is skipped.
  void createEmptyLoop();

  // Widen the given instruction to VF wide vector instruction
  void vectorizeInstruction(VPInstruction *VPInst);
  void vectorizeVPInstruction(VPInstruction *VPInst);

  // Vectorize the given instruction that cannot be widened using serialization.
  // This is done using a sequence of extractelement, Scalar Op, InsertElement
  // instructions.
  void serializeInstruction(VPInstruction *VPInst);

  IRBuilder<> &getBuilder() { return Builder; }

  BasicBlock *getLoopVectorPH() { return LoopVectorPreHeader; }

  Loop *getMainLoop() const { return NewLoop; }
  unsigned getVF() const { return VF; }
  bool getNeedRemainderLoop() const { return false; }
  Loop *getRemainderLoop() const { return nullptr; }

  // Get the widened vector value for given value V. If the scalar value
  // has not been widened, we widen it by VF and store it in WidenMap
  // before returning the widened value
  Value *getVectorValue(VPValue *V); // TODO: Remove after uplift
  Value *getVectorValueUplifted(VPValue *V);

  // Get a vector of pointers corresponding to the private variable for each
  // vector lane.
  Value *createVectorPrivatePtrs(VPAllocatePrivate *V);

  /// Return a value in the new loop corresponding to \p V from the original
  /// loop at vector index \p Lane. If the value has
  /// been vectorized but not scalarized, the necessary extractelement
  /// instruction will be generated.
  Value *getScalarValue(VPValue *V, unsigned Lane); // TODO: Remove after uplift
  Value *getScalarValueUplifted(VPValue *V, unsigned Lane);

  /// MaskValue setter
  void setMaskValue(Value *MV) { MaskValue = MV; }

  /// Helper wrapper to find the best smid function variant for a given \p Call.
  /// \p Masked parameter tells whether we need a masked version or not.
  std::unique_ptr<VectorVariant> matchVectorVariant(const CallInst *Call,
                                                    bool Masked);

  /// Vectorize call arguments, or for simd functions scalarize if the arg
  /// is linear or uniform.
  void vectorizeCallArgs(VPInstruction *VPCall, VectorVariant *VecVariant,
                         SmallVectorImpl<Value *> &VecArgs,
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

private:
  /// Find the best simd function variant.
  std::unique_ptr<VectorVariant>
  matchVectorVariantImpl(StringRef VecVariantStringValue, bool Masked);

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

  /// Serialize instruction that requires predication.
  void serializeWithPredication(VPInstruction *VPInst);

  /// Specialized method to handle predication of uniform load instruction. This
  /// function generates a single scalar load predicated by a not all-zero check
  /// of the current MaskValue.
  // Example :
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
  void serializePredicatedUniformLoad(VPInstruction *VPInst);

  /// Predicate conditional instructions that require predication on their
  /// respective conditions.
  void predicateInstructions();

  /// Reverse vector elements
  Value *reverseVector(Value *Vec, unsigned Stride = 1);

  /// Create the primary induction variable for vector loop.
  PHINode *createInductionVariable(Loop *L, Value *Start, Value *End,
                                   Value *Step);

  /// Make the needed fixups for all live out values.
  void fixOutgoingValues();

  /// Fix up reduction last value (link with remainder etc).
  void fixReductionLastVal(const VPReduction &Red, Value *LastVal);

  /// Fix up live out value for a loop entity \p Ent.
  void fixLiveOutValues(const VPLoopEntity &Ent, Value *LastVal);

  /// A part of fix up of last value. Creates a needed phi in intermediate
  /// block and updates phi in remainder.
  void createLastValPhiAndUpdateOldStart(Value *OrigStartValue, PHINode *Phi,
                                         const Twine &NameStr, Value *LastVal);
  /// Fix up induction last value.
  void fixInductionLastVal(const VPInduction &Ind, Value *LastVal);

  /// Get an index of last written lane using Mask value.
  Value *getLastLaneFromMask(Value *MaskPtr);

  /// \brief The Loop exit block may have single value PHI nodes where the
  /// incoming value is 'Undef'. While vectorizing we only handled real values
  /// that were defined inside the loop. Here we fix the 'undef case'.
  void fixLCSSAPHIs();

  /// Insert the new loop to the loop hierarchy and pass manager
  /// and update the analysis passes.
  void updateAnalysis();

  /// Get the Function-entry block.
  BasicBlock &getFunctionEntryBlock() const {
    return OrigLoop->getHeader()->getParent()->front();
  }

  /// This function adds (StartIdx, StartIdx + Step, StartIdx + 2*Step, ...)
  /// to each vector element of Val. The sequence starts at StartIndex.
  Value *getStepVector(Value *Val, int StartIdx, Value *Step,
                       Instruction::BinaryOps BinOp);

  /// Create a broadcast instruction. This method generates a broadcast
  /// instruction (shuffle) for loop invariant values and for the induction
  /// value. If this is the induction variable then we extend it to N, N+1, ...
  /// this is needed because each iteration in the loop corresponds to a SIMD
  /// element.
  Value *getBroadcastInstrs(Value *V);

  /// Widen Phi node, which is not an induction variable. This Phi node
  /// is a result of merging blocks ruled out by uniform branch.
  void widenNonInductionPhi(VPPHINode *Phi);

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
  const VPLoopEntityList *VPEntities;

  // Loop trip count
  Value *TripCount;
  // Vectorized loop trip count.
  Value *VectorTripCount;

  /// The new Induction variable which was added to the new block.
  PHINode *Induction;

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

  // Holds last values generated for loop entities.
  DenseMap<const VPLoopEntity *, Value *> EntitiesLastValMap;

  // --- Vectorization state ---

  /// The vector-loop preheader.
  BasicBlock *LoopVectorPreHeader;
  /// The scalar-loop preheader.
  BasicBlock *LoopScalarPreHeader;
  /// Middle Block between the vector and the scalar.
  BasicBlock *LoopMiddleBlock;
  /// The ExitBlock of the scalar loop.
  BasicBlock *LoopExitBlock;
  /// The vector loop body.
  BasicBlock *LoopVectorBody;
  /// The scalar loop body.
  BasicBlock *LoopScalarBody;
  /// Current mask value for instructions being generated
  Value *MaskValue;
  /// A list of all bypass blocks. The first block is the entry of the loop.
  SmallVector<BasicBlock *, 4> LoopBypassBlocks;

  // Pointer to current transformation state - used to obtain VPBasicBlock to
  // BasicBlock mapping.
  struct VPTransformState *State = nullptr;

  // Get alignment for load/store VPInstruction using underlying
  // llvm::Instruction.
  unsigned getOriginalLoadStoreAlignment(const VPInstruction *VPInst);

  // Widen the given load instruction. EmitIntrinsic needs to be set to true
  // when we can start emitting masked_gather intrinsic once we have support
  // in code gen. Without code gen support, we will serialize the intrinsic.
  // As a result, we simply serialize the instruction for now.
  void vectorizeLoadInstruction(VPInstruction *VPInst,
                                bool EmitIntrinsic = false);

  // Generate a wide (un)masked load for a given consecutive stride load.
  Value *vectorizeUnitStrideLoad(VPInstruction *VPInst, int StrideVal,
                                 bool IsPvtPtr);

  // Widen the given store instruction. EmitIntrinsic needs to be set to true
  // when we can start emitting masked_scatter intrinsic once we have support
  // in code gen. Without code gen support, we will serialize the intrinsic.
  // As a result, we simply serialize the instruction for now.
  void vectorizeStoreInstruction(VPInstruction *VPInst,
                                 bool EmitIntrinsic = false);

  // Generate a wide (un)masked store for a given consecutive stride store.
  void vectorizeUnitStrideStore(VPInstruction *VPInst, int StrideVal,
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

  /// Adjust arguments passed to SVML functions to handle masks
  void addMaskToSVMLCall(Function *OrigF, SmallVectorImpl<Value *> &VecArgs,
                         SmallVectorImpl<Type *> &VecArgTys);

  // Widen call instruction parameters and return. Currently, this is limited
  // to svml function support that is hooked in to TLI. Later, this can be
  // extended to user-defined vector functions.
  void vectorizeCallInstruction(VPInstruction *VPCall);

  // Widen Select instruction.
  void vectorizeSelectInstruction(VPInstruction *VPInst);

  // Generate code for given VPPHINode transforming it either into PHINode or
  // into Select instruction (for blends).
  void vectorizeVPPHINode(VPPHINode *VPPhi);

  // Create a PHINode for VPPHINode that represent reduction.
  void vectorizeReductionPHI(VPPHINode *Inst, PHINode *UnderlyingPhi = nullptr);

  // Vectorize the call to OpenCL SinCos function with the vector-variant from
  // SVML
  void vectorizeOpenCLSinCos(VPInstruction *VPCall, bool IsMasked);

  /// Store instructions that should be predicated, as a pair
  ///   <StoreInst, Predicate>
  SmallVector<std::pair<Instruction *, Value *>, 4> PredicatedInstructions;

  /// Hold names of scalar select builtins
  SmallSet<std::string, 20> ScalarSelectSet;

  /// This function returns the widened GEP instruction that is used
  /// as a pointer-operand in a load-store instruction. In the generated code,
  /// the returned GEP is itself used as an operand of a Scatter/Gather
  /// function.
  Value *getWidenedAddressForScatterGather(VPInstruction *VPI);

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

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZERCODEGEN_H
