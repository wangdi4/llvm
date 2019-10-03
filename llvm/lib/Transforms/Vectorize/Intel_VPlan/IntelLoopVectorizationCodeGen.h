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
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Support/CommandLine.h"

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
        OldInduction(nullptr), VF(VecWidth), UF(UnrollFactor), Builder(Context),
        StartValue(nullptr), StrideValue(nullptr), LoopVectorPreHeader(nullptr),
        LoopScalarPreHeader(nullptr), LoopMiddleBlock(nullptr),
        LoopExitBlock(nullptr), LoopVectorBody(nullptr),
        LoopScalarBody(nullptr), MaskValue(nullptr) {}

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
  void vectorizeInstruction(Instruction *Inst);
  void vectorizeInstruction(VPInstruction *VPInst);
  void vectorizeVPInstruction(VPInstruction *VPInst);

  // Vectorize the given instruction that cannot be widened using serialization.
  // This is done using a sequence of extractelement, Scalar Op, InsertElement
  // instructions.
  void serializeInstruction(Instruction *Inst, bool HasLoopPrivateOperand = false);
  void serializeInstruction(VPInstruction *VPInst);

  /// Collect Uniform and Scalar values for the given \p VF.
  void collectUniformsAndScalars(unsigned VF);

  IRBuilder<>& getBuilder() { return Builder; }

  BasicBlock *getLoopVectorPH() { return LoopVectorPreHeader;  }

  Loop *getMainLoop() const { return NewLoop; }
  unsigned getVF() const { return VF; }
  bool getNeedRemainderLoop() const { return false; }
  Loop *getRemainderLoop() const { return nullptr; }

  static void collectTriviallyDeadInstructions(
    Loop *OrigLoop, VPOVectorizationLegality *Legal,
    SmallPtrSetImpl<Instruction *> &DeadInstructions);

  // Get the widened vector value for given value V. If the scalar value
  // has not been widened, we widen it by VF and store it in WidenMap
  // before returning the widened value
  Value *getVectorValue(Value *V);
  Value *getVectorValue(VPValue *V); // TODO: Remove after uplift
  Value *getVectorValueUplifted(VPValue *V);

  // Get widened base pointer of in-memory private variable
  Value *getVectorPrivateBase(Value *V);

  // Get widened base pointer(s) of in-memory private variable of aggregate-type
  Value *getVectorPrivateAggregateBase(Value *ArrayPriv);

  /// Return true if we are serializing privatized alloca for array-types
  bool isSerializedPrivateArray(Value *Priv) const;

  /// Get base pointer(s) of in-memory private variable of array-type
  // In case we have not allocated the pointers, do so and store them
  // in the ScalarMap data-structure.
  void createSerialPrivateArrayBase(Value *ArrPriv);

  // Get a vector of pointers corresponding to the private variable for each
  // vector lane.
  Value *getVectorPrivatePtrs(Value *ScalarPrivate);
  Value *createVectorPrivatePtrs(VPAllocatePrivate *V);

  /// Return a value in the new loop corresponding to \p V from the original
  /// loop at vector index \p Lane. If the value has
  /// been vectorized but not scalarized, the necessary extractelement
  /// instruction will be generated.
  Value *getScalarValue(Value *V, unsigned Lane);
  Value *getScalarValue(VPValue *V, unsigned Lane); // TODO: Remove after uplift
  Value *getScalarValueUplifted(VPValue *V, unsigned Lane);

  /// MaskValue setter
  void setMaskValue(Value *MV) { MaskValue = MV; }

  /// Write down the condition mask of the Edge between block \p From
  /// and block \p To.
  void setEdgeMask(BasicBlock *From, BasicBlock *To, Value *Mask);

  /// Get a condition mask between block \p From and block \p To.
  Value *getEdgeMask(BasicBlock *From, BasicBlock *To);

  /// Helper wrapper to find the best smid function variant for a given \p Call.
  /// \p Masked parameter tells whether we need a masked version or not.
  std::unique_ptr<VectorVariant> matchVectorVariant(const CallInst *Call,
                                                    bool Masked);

  /// Vectorize call arguments, or for simd functions scalarize if the arg
  /// is linear or uniform.
  void vectorizeCallArgs(CallInst *Call, VectorVariant *VecVariant,
                         SmallVectorImpl<Value*> &VecArgs,
                         SmallVectorImpl<Type*> &VecArgTys);
  void vectorizeCallArgs(VPInstruction *VPCall, VectorVariant *VecVariant,
                         SmallVectorImpl<Value *> &VecArgs,
                         SmallVectorImpl<Type *> &VecArgTys);

  // Return true if the argument at position /p Idx for function /p FnName is
  // scalar.
  bool isScalarArgument(StringRef FnName, unsigned Idx);
  
  /// Add an in memory linear to the vector of linear values.
  void addUnitStepLinear(Value *LinVal, Value *NewVal, int Step);

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

  /// Emit blocks of vector loop
  /// Emit a bypass check to see if we have enough iterations \p Count to
  /// execute one vector loop.
  void  emitMinimumIterationCountCheck(Loop *L, Value *Count);

  /// Compute the transformed value of Index at offset StartValue using step
  /// StepValue.
  /// For integer induction, returns StartValue + Index * StepValue.
  /// For pointer induction, returns StartValue[Index * StepValue].
  /// FIXME: The newly created binary instructions should contain nsw/nuw
  /// flags, which can be found from the original scalar operations.
  Value *emitTransformedIndex(IRBuilder<> &B, Value *Index, ScalarEvolution *SE,
                              const DataLayout &DL,
                              const InductionDescriptor &ID) const;

  /// Emit a bypass check to see if the vector trip count is nonzero.
  void  emitVectorLoopEnteredCheck(Loop *L, BasicBlock *Bypass);

  /// Emit resume block for combining bypassed values and the values coming
  /// from the vector loop.
  void  emitResume(Value *CountRoundDown);

  /// Check whether the original loop trip count \p Count is equal to vector
  /// loop trip count \p CountRoundDown. In this case we can bypass the scalar
  /// remainder.
  void  emitEndOfVectorLoop(Value *Count, Value *CountRoundDown);

  // Return the trip count for the scalar loop. Returns 0 for non-constant trip
  // count loops.
  uint64_t getConstTripCount() const;

  // Return the trip count for the scalar loop. 
  Value *getTripCount() const { return TripCount; }

  /// Returns (and creates if needed) the original loop trip count.
  Value *getOrCreateTripCount(Loop *L);

  /// Returns (and creates if needed) the trip count of the widened loop.
  Value *getOrCreateVectorTripCount(Loop *L);

  /// Serialize instruction that requires predication.
  void serializeWithPredication(Instruction *Inst);
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
  void serializePredicatedUniformLoad(Instruction *Inst);
  void serializePredicatedUniformLoad(VPInstruction *VPInst);

  /// Predicate conditional instructions that require predication on their
  /// respective conditions.
  void predicateInstructions();

  /// Reverse vector elements
  Value *reverseVector(Value *Vec, unsigned Stride = 1);

  /// Create the primary induction variable for vector loop.
  PHINode *createInductionVariable(Loop *L, Value *Start,
                                   Value *End, Value *Step);

  /// Load initial linear value before the loop and do the linear value
  /// update at the end of the loop.
  void initLinears(PHINode *Induction, Loop *VecLoop);

  /// Handle all cross-iteration phis in the header.
  void fixCrossIterationPHIs();

  /// The result of reduction is in register only.
  void fixReductionInReg(PHINode *Phi, RecurrenceDescriptor& RdxDesc);
  
  /// Feed reduction result into LCSSA Phi node
  void fixReductionLCSSA(Value *LoopExitInst, Value *NewV);

  /// Fix a reduction cross-iteration phi. This is the second phase of
  /// vectorizing this phi node.
  void fixReductionPhi(PHINode *Phi, Value *VectorStart);

  /// Set a Phi to merge In-Reg reduction value.
  void mergeReductionControlFlow(PHINode *Phi, RecurrenceDescriptor& RdxDesc,
                                 Value *EndV);

  /// Build a tail code for in-memory reduction.
  Value *buildInMemoryReductionTail(Value *OrigRedV,
      RecurrenceDescriptor::RecurrenceKind Kind,
      RecurrenceDescriptor::MinMaxRecurrenceKind Mrk);

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

  /// Set up the values of the IVs correctly when exiting the vector loop.
  void fixupIVUsers(PHINode *OrigPhi, const InductionDescriptor &II,
                    Value *CountRoundDown, Value *EndValue,
                    BasicBlock *MiddleBlock);

  /// Fix instructions that use loop private values outside the vectorized loop.
  void fixupLoopPrivates();

  /// Write last value of unconditional private variable.
  void writePrivateValAfterLoop(Value *OrigPrivate);

  /// Write last value of conditional private.
  void writeCondPrivateValAfterLoop(Value *OrigPrivate);

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

  /// Return the alignment to be set on the 'alloca' inst.
  unsigned getPrivateVarAlignment(Value *V);

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

  /// Create vector and scalar version of the same induction variable.
  void widenIntOrFpInduction(PHINode *IV, VPPHINode *VPIV);

  /// Widen Phi node, which is not an induction variable. This Phi node
  /// is a result of merging blocks ruled out by uniform branch.
  void widenNonInductionPhi(VPPHINode *Phi);

  void fixNonInductionPhis();
  void fixNonInductionVPPhis();

  /// Build a tail code for all reductions going through the
  /// memory.
  void completeInMemoryReductions();

  /// Given a PHI node which is an induction and the corresponding induction
  /// descriptor, return the Value for induction step.
  Value *getIVStep(PHINode *IV, const InductionDescriptor &ID);

  /// Compute the scalar induction value of induction \p OrigIV for the
  /// the vector lane \p Lane 
  Value *buildScalarIVForLane(PHINode *OrigIV, unsigned Lane);

  /// Create a vector version of induction.
  void createVectorIntOrFpInductionPHI(const InductionDescriptor &II,
                                       Value *Step, Instruction *&VectorInd);

  /// Collect the instructions that are uniform after vectorization. An
  /// instruction is uniform if we represent it with a single scalar value in
  /// the vectorized loop corresponding to each vector iteration. Examples of
  /// uniform instructions include pointer operands of consecutive or
  /// interleaved memory accesses. Note that although uniformity implies an
  /// instruction will be scalar, the reverse is not true. In general, a
  /// scalarized instruction will be represented by VF scalar values in the
  /// vectorized loop, each corresponding to an iteration of the original
  /// scalar loop.
  void collectLoopUniforms(unsigned VF);

  /// Returns true if \p I is known to be uniform after vectorization.
  bool isUniformAfterVectorization(Instruction *I, unsigned VF) const;

  /// Return true if \p FnName is the name of an OpenCL scalar select and \p Idx
  /// is the position of the mask argument.
  bool isOpenCLSelectMask(StringRef FnName, unsigned Idx);

  /// Return the right vector mask for a OpenCL vector select build-in.
  Value *getOpenCLSelectVectorMask(Value *ScalarMask);
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
  /// The induction variable of the old basic block.
  PHINode *OldInduction;

  // Vector factor or vector length to use. Each scalar instruction is widened
  // to operate on this number of operands.
  unsigned VF;

  // Unroll factor
  unsigned UF;

  // IR Builder to use to generate instructions
  IRBuilder<> Builder;

  // Starting value of loop induction variable
  Value *StartValue;

  // Loop increment
  ConstantInt *StrideValue;

  /// Holds the instructions known to be uniform after vectorization.
  /// The data is collected per VF.
  DenseMap<unsigned, SmallPtrSet<Instruction *, 4>> Uniforms;

  /// Holds the instructions known to be scalar after vectorization.
  /// The data is collected per VF.
  DenseMap<unsigned, SmallPtrSet<Instruction *, 4>> Scalars;

  /// Holds a mapping between the VPPHINode and the corresponding vector LLVM
  /// IR PHI node that needs fix up. The operands of the VPPHINode are used
  /// to setup the operands of the LLVM IR PHI node.
  DenseMap<VPPHINode *, PHINode *> PhisToFix;
  DenseMap<VPPHINode *, PHINode *> ScalarPhisToFix;

  // Map of scalar LLVM IR value and widened LLVM IR value.
  DenseMap<Value *, Value *> WidenMap;

  // Map of scalar VPValue and widened LLVM IR value.
  DenseMap<VPValue *, Value *> VPWidenMap;

  // Map of scalar values.
  std::map<Value *, DenseMap<unsigned, Value *>> ScalarMap;
  // Map of scalar values for VPValues.
  std::map<VPValue *, DenseMap<unsigned, Value *>> VPScalarMap;

  // Map of widened private values. Unlike WidenMap, this is
  // pointer-to-pointer map.
  std::map<Value *, Value *> LoopPrivateWidenMap;
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

  // Keeps last non-zero mask
  std::map<Value *, Value *> LoopPrivateLastMask;

  // Holds the end values for each induction variable. We save the end values
  // so we can later fix-up the external users of the induction variables.
  DenseMap<PHINode *, Value *> IVEndValues;

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

  // Widen the load of a linear value. We do a scalar load and generate a vector
  // value using the linear \p Step 
  void vectorizeLinearLoad(Instruction *Inst, int Step);
  
  // Widen the given load instruction. EmitIntrinsic needs to be set to true
  // when we can start emitting masked_gather intrinsic once we have support
  // in code gen. Without code gen support, we will serialize the intrinsic.
  // As a result, we simply serialize the instruction for now.
  void vectorizeLoadInstruction(Instruction *Inst, bool EmitIntrinsic = false);
  void vectorizeLoadInstruction(VPInstruction *VPInst,
                                bool EmitIntrinsic = false);

  // Generate a wide (un)masked load for a given consecutive stride load.
  Value *vectorizeUnitStrideLoad(VPInstruction *VPInst, int StrideVal,
                                 bool IsPvtPtr);

  // Widen the store of a linear value. We do a scalar store of the value in the
  // first vector lane.
  void vectorizeLinearStore(Instruction *Inst);
  
  // Widen the given store instruction. EmitIntrinsic needs to be set to true
  // when we can start emitting masked_scatter intrinsic once we have support
  // in code gen. Without code gen support, we will serialize the intrinsic.
  // As a result, we simply serialize the instruction for now.
  void vectorizeStoreInstruction(Instruction *Inst, bool EmitIntrinsic = false);
  void vectorizeStoreInstruction(VPInstruction *VPInst,
                                 bool EmitIntrinsic = false);

  // Generate a wide (un)masked store for a given consecutive stride store.
  void vectorizeUnitStrideStore(VPInstruction *VPInst, int StrideVal,
                                bool IsPvtPtr);

  // Re-vectorize the given vector load instruction. The function handles 
  // only simple vectors.
  void widenVectorLoad(LoadInst *Inst);

  // Re-vectorize the given vector store instruction. The function handles 
  // only simple vectors.
  void widenVectorStore(StoreInst *Inst);

  // Widen a BitCast/AddrSpaceCast instructions
  template <typename CastInstTy> void vectorizeCast(Instruction *Inst);
  template <typename CastInstTy>
  void vectorizeCast(typename std::enable_if<
                     std::is_same<CastInstTy, BitCastInst>::value ||
                         std::is_same<CastInstTy, AddrSpaceCastInst>::value,
                     VPInstruction>::type *VPInst);

  // Widen ExtractElt instruction - loop re-vectorization.
  void vectorizeExtractElement(Instruction *Inst);
  void vectorizeExtractElement(VPInstruction *VPInst);

  // Widen InsertElt instruction - loop re-vectorization.
  void vectorizeInsertElement(Instruction *Inst);
  void vectorizeInsertElement(VPInstruction *VPInst);

  // Widen Shuffle instruction - loop re-vectorization.
  void vectorizeShuffle(Instruction *Inst);
  void vectorizeShuffle(VPInstruction *VPInst);
  // Get the mask of a VPInstruction representing shuffle (before widening) as a
  // vector of ints.
  SmallVector<int, 16> getVPShuffleOriginalMask(const VPInstruction *VPI);
  // Utility to get VPValue that is broadcasted if the input \p V is a splat
  // vector. Functionality is same as VectorUtils::getSplatValue.
  const VPValue *getOrigSplatVPValue(const VPValue *V);

  // Widen call instruction parameters and return. Currently, this is limited
  // to svml function support that is hooked in to TLI. Later, this can be
  // extended to user-defined vector functions.
  void vectorizeCallInstruction(CallInst *Call);
  void vectorizeCallInstruction(VPInstruction *VPCall);

  // Widen Select instruction.
  void vectorizeSelectInstruction(Instruction *Inst);
  void vectorizeSelectInstruction(VPInstruction *VPInst);

  // Widen the given PHI instruction. For now we assume this corresponds to
  // the Induction PHI.
  void vectorizePHIInstruction(VPPHINode *VPPhi);

  // Generate code for given VPPHINode transforming it either into PHINode or
  // into Select instruction (for blends).
  void vectorizeVPPHINode(VPPHINode *VPPhi);

  // Create a PHINode for VPPHINode that represent reduction.
  void vectorizeReductionPHI(VPPHINode *Inst, PHINode *UnderlyingPhi = nullptr);

  // Vectorize the call to OpenCL SinCos function with the vector-variant from
  // SVML
  void vectorizeOpenCLSinCos(CallInst *Call, bool isMasked);
  void vectorizeOpenCLSinCos(VPInstruction *VPCall, bool IsMasked);

  // Vectorize the write channel source argument for an OpenCL write channel
  // call. The source is the data that will be written to the channel.
  Value* vectorizeOpenCLWriteChannelSrc(CallInst *Call, unsigned ArgNum);

  // Vectorize the read channel destination for an OpenCL read channel call.
  // The destination is the location where the data from the channel call will
  // be written to.
  void vectorizeOpenCLReadChannelDest(CallInst *Call, CallInst *VecCall,
                                      Value *CallOp);

  // Return a vector Vl wide: <Val, Val + Stride,
  // ... VAL + (VF - 1) * Stride>
  Value *getStrideVector(Value *Val, Value *Stride);

  // Map Edge between blocks to a mask value.
  std::map< std::pair<BasicBlock *, BasicBlock *>, Value *> EdgeToMaskMap;

  /// Store instructions that should be predicated, as a pair
  ///   <StoreInst, Predicate>
  SmallVector<std::pair<Instruction *, Value *>, 4> PredicatedInstructions;

  /// Hold names of scalar select builtins
  SmallSet<std::string, 20> ScalarSelectSet;

  /// This function returns the widened GEP instruction that is used
  /// as a pointer-operand in a load-store instruction. In the generated code,
  /// the returned GEP is itself used as an operand of a Scatter/Gather
  /// function.
  Value *createWidenedGEPForScatterGather(Instruction *I);
  Value *getWidenedAddressForScatterGather(VPInstruction *VPI);

  /// This function return an appropriate BasePtr for cases where we are have
  /// load/store to consecutive memory locations
  Value *createWidenedBasePtrConsecutiveLoadStore(Instruction *I, Value *Ptr,
                                                  bool Reverse);
  Value *createWidenedBasePtrConsecutiveLoadStore(VPValue *Ptr, bool Reverse);

  /// Create a wide load for the \p Group (or get existing one).
  Value *getOrCreateWideLoadForGroup(OVLSGroup *Group);

  /// Vectorize \p VPLoad instruction that is part of a \p Group.
  Value *vectorizeInterleavedLoad(VPInstruction *VPLoad, OVLSGroup *Group);

  /// Vectorize \p VPStore instruction that is part of a \p Group.
  void vectorizeInterleavedStore(VPInstruction *VPStore, OVLSGroup *Group);

  DenseMap<AllocaInst *, Value *> ReductionEofLoopVal;
  DenseMap<AllocaInst *, Value *> ReductionVecInitVal;

  SmallDenseMap<const OVLSGroup *, LoadInst *> VLSGroupLoadMap;
};

} // end vpo namespace
} // end llvm namespace

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELLOOPVECTORIZERCODEGEN_H
