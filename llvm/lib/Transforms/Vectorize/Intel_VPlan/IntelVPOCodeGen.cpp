//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2019-2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   IntelVPOCodeGen.cpp -- VPValue-based LLVM IR code generation from VPlan.
//
//===----------------------------------------------------------------------===//

#include "IntelVPOCodeGen.h"
#include "IntelLoopVectorizationLegality.h"
#include "IntelVPlan.h"
#include "IntelVPlanCallVecDecisions.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanVLSAnalysis.h"
#include "IntelVPlanVectorizeIndirectCalls.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/SaveAndRestore.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"
#include <numeric>
#include <tuple>

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-ir-loop-vectorize"

extern bool Usei1MaskForSimdFunctions;

static cl::opt<bool> PredicateSafeValueDivision(
    "vplan-predicate-safe-value-div", cl::init(false), cl::Hidden,
    cl::desc("Always serialize masked integer division, even if divisor is "
             "known to be safe for speculation."));

static void addBlockToParentLoop(Loop *L, BasicBlock *BB, LoopInfo &LI) {
  if (auto *ParentLoop = L->getParentLoop())
    ParentLoop->addBasicBlockToLoop(BB, LI);
}

/// Return true if \p Var a variable identified for SOA-layout.
static bool isSOAAccess(const VPValue *Var, const VPlanVector *Plan) {
  return Plan->getVPlanDA()->isSOAShape(Var) ||
         Plan->getVPlanDA()->hasBeenSOAConverted(Var);
}

/// Return true if \p Var has a SOA unit-stride access.
static bool isSOAUnitStride(const VPValue *Var, const VPlanVector *Plan) {
  return Plan->getVPlanDA()->isSOAUnitStride(Var);
}

// Generate SOA-type for the given input-type.
// The following are the transformed for the SOA-layout.
// Ty -> <VF x Ty>.
// [NumElts x Ty] -> [NumElts x <VF x Ty>].
// Other input-types are currently not supported for SOA-layout.
static Type *getSOAType(Type *InTy, unsigned VF) {
  if (auto ArrTy = dyn_cast<ArrayType>(InTy))
    return ArrayType::get(getSOAType(ArrTy->getArrayElementType(), VF),
                          ArrTy->getArrayNumElements());
  else if (!(InTy->isAggregateType() || InTy->isVectorTy())) // Scalar-type.
    return FixedVectorType::get(InTy, VF);
  else
    // Any other type should be unsupported. Structure-types would be supported
    // in future.
    llvm_unreachable("Unexpected type encountered.");
}

static Value *calculateVectorTC(Value *OrigTC, IRBuilder<> &Builder,
                                unsigned CStep, Value* PeelAdjust = nullptr) {
  // We need to generate the expression for the part of the loop that the
  // vectorized body will execute. Step is equal to the vectorization factor
  // (number of SIMD elements) times the unroll factor (number of SIMD
  // instructions). If the Step is a power of 2, then the vector trip count is
  // calculated using the following formula: &(OrigTC, ~(Step-1)). Otherwise, we
  // substract the remainder of OrigTC/Step from OrigTC.
  auto *Step = ConstantInt::get(OrigTC->getType(), CStep);
  if (isPowerOf2_32(CStep) && PeelAdjust == nullptr)
    return Builder.CreateAnd(OrigTC,
                             ConstantInt::get(OrigTC->getType(), ~(CStep - 1)));
  // If we have an adjustment for peel, the formula is changed into
  //   vector_ub = OrigTC - (OrigTC - Adjustment) % CStep.
  Value *Rem;
  if (PeelAdjust) {
    auto AdjOrig =
        Builder.CreateSub(OrigTC, PeelAdjust, "n.adjst", /*HasNUW=*/true,
                          /*HasNSW=*/true);
    Rem = Builder.CreateURem(AdjOrig, Step, "n.mod.vf");
  } else {
    Rem = Builder.CreateURem(OrigTC, Step, "n.mod.vf");
  }
  return Builder.CreateSub(OrigTC, Rem, "n.vec", /*HasNUW=*/true,
                           /*HasNSW=*/true);
}

// TODO: this method should be extended in future to preserve all required
// metadata for memory operations.
static void
propagateLoadStoreInstAliasMetadata(Instruction *LoadStore,
                                    const VPLoadStoreInst *VPMemInst) {
  if (auto *MD = VPMemInst->getMetadata(LLVMContext::MD_noalias))
    LoadStore->setMetadata(LLVMContext::MD_noalias, MD);
  if (auto *MD = VPMemInst->getMetadata(LLVMContext::MD_alias_scope))
    LoadStore->setMetadata(LLVMContext::MD_alias_scope, MD);
}

// Helper to check if the given value/instruction is used only in a
// lifetime.start/end intrinsic.
static bool isOnlyUsedInLifetimeIntrinsics(const VPValue *Val) {
  return all_of(Val->users(), [&](const VPUser *U) {
    if (auto *VPCall = dyn_cast<VPCallInstruction>(U))
      return VPCall->isLifetimeStartOrEndIntrinsic();
    return false;
  });
}

static bool requiresUnsupportedSVAFeatures(const VPInstruction *VInst,
                                           const VPlanVector *Plan) {
  auto DA = Plan->getVPlanDA();
  auto SVA = Plan->getVPlanSVA();
  return !DA->isUniform(*VInst) && !SVA->instNeedsVectorCode(VInst) &&
         SVA->instNeedsFirstScalarCode(VInst) &&
         SVA->instNeedsLastScalarCode(VInst);
}

Value *VPOCodeGen::generateSerialInstruction(VPInstruction *VPInst,
                                             ArrayRef<Value *> Ops) {
  Value *SerialInst = nullptr;
  if (Instruction::isBinaryOp(VPInst->getOpcode())) {
    assert(Ops.size() == 2 &&
           "Binop VPInstruction has incorrect number of operands.");
    SerialInst = Builder.CreateBinOp(
        static_cast<Instruction::BinaryOps>(VPInst->getOpcode()), Ops[0],
        Ops[1]);
  } else if (Instruction::isUnaryOp(VPInst->getOpcode())) {
    assert(Ops.size() == 1 &&
           "Unop VPInstruction has incorrect number of operands.");
    SerialInst = Builder.CreateUnOp(
        static_cast<Instruction::UnaryOps>(VPInst->getOpcode()), Ops[0]);
  } else if (VPInst->getOpcode() == Instruction::Load) {
    assert(Ops.size() == 1 &&
           "Load VPInstruction has incorrect number of operands.");
    SerialInst = Builder.CreateLoad(VPInst->getType(), Ops[0]);
    auto *NewLoad = cast<LoadInst>(SerialInst);
    auto *VPLoad = cast<VPLoadStoreInst>(VPInst);
    NewLoad->setVolatile(VPLoad->isVolatile());
    NewLoad->setOrdering(VPLoad->getOrdering());
    if (VPLoad->isAtomic())
      NewLoad->setSyncScopeID(VPLoad->getSyncScopeID());
    NewLoad->setAlignment(VPLoad->getAlignment());
    propagateLoadStoreInstAliasMetadata(NewLoad, VPLoad);
  } else if (VPInst->getOpcode() == Instruction::Store) {
    assert(Ops.size() == 2 &&
           "Store VPInstruction has incorrect number of operands.");
    SerialInst = Builder.CreateStore(Ops[0], Ops[1]);
    auto *NewStore = cast<StoreInst>(SerialInst);
    auto *VPStore = cast<VPLoadStoreInst>(VPInst);
    NewStore->setVolatile(VPStore->isVolatile());
    NewStore->setOrdering(VPStore->getOrdering());
    if (VPStore->isAtomic())
      NewStore->setSyncScopeID(VPStore->getSyncScopeID());
    NewStore->setAlignment(VPStore->getAlignment());
    propagateLoadStoreInstAliasMetadata(NewStore, VPStore);
  } else if (VPInst->getOpcode() == Instruction::Call) {
    assert(Ops.size() > 0 &&
           "Call VPInstruction should have atleast one operand.");
    auto *VPCall = cast<VPCallInstruction>(VPInst);

    if (auto *ScalarF = dyn_cast<Function>(Ops.back())) {
      Ops = Ops.drop_back();
      if (ScalarF->getIntrinsicID() == Intrinsic::assume) {
        SmallVector<OperandBundleDef, 1> OpBundles;
        auto *VPCall = cast<VPCallInstruction>(VPInst);
        auto UnderlyingCI = VPCall->getUnderlyingCallInst();
        assert (UnderlyingCI && "Underlying call instruction expected here");
        UnderlyingCI->getOperandBundlesAsDefs(OpBundles);
        int CurrentIndex = 1;
        for (OperandBundleDef &BD : OpBundles) {
          StringRef ClauseString = BD.getTag();
          BD = OperandBundleDef(ClauseString.str(),
                                makeArrayRef(Ops).slice(CurrentIndex, BD.input_size()));
          CurrentIndex += BD.input_size();
        }
        SerialInst = Builder.CreateAssumption(Ops.front(), OpBundles);
      } else {
        SerialInst = Builder.CreateCall(ScalarF, Ops);
      }
    } else {
      // Indirect call (via function pointer).
      Value *FuncPtr = Ops.back();
      Ops = Ops.drop_back();

      SerialInst = Builder.CreateCall(VPCall->getFunctionType(), FuncPtr, Ops);
    }
    auto *SerialCall = cast<CallInst>(SerialInst);
    // Copy fast math flags represented in VPInstruction to new call.
    if (isa<FPMathOperator>(SerialCall))
       VPCall->copyOperatorFlagsTo(SerialCall);
    SerialCall->setCallingConv(VPCall->getOrigCallingConv());
    SerialCall->setAttributes(VPCall->getOrigCallAttrs());
    SerialCall->setTailCall(VPCall->isOrigTailCall());
  } else if (VPGEPInstruction *VPGEP = dyn_cast<VPGEPInstruction>(VPInst)) {
    assert(Ops.size() > 1 &&
           "VPGEPInstruction should have atleast two operands.");
    Value *GepBasePtr = Ops[0];
    Ops = Ops.drop_front();

    Type *SourceElementType = VPGEP->getSourceElementType();
    // TODO: SOAMemRef transformations needs to be updated to correctly set
    // SourceElementType which can then be used here.
    if (!VPGEP->isOpaque()) {
      SourceElementType =
          GepBasePtr->getType()->getScalarType()->getPointerElementType();
    } else if (isSOAAccess(VPGEP, Plan)) {
      SourceElementType = getSOAType(SourceElementType, VF);
    }

    if (VPGEP->isInBounds())
      SerialInst =
          Builder.CreateInBoundsGEP(SourceElementType, GepBasePtr, Ops);
    else
      SerialInst = Builder.CreateGEP(SourceElementType, GepBasePtr, Ops);
    StringRef GepName =
        isSOAAccess(VPGEP, Plan) ? "soa.scalar.gep" : "scalar.gep";
    SerialInst->setName(GepName);
  } else if (VPInst->getOpcode() == Instruction::InsertElement) {
    assert(Ops.size() == 3 &&
           "InsertElement instruction should have three operands.");
    SerialInst = Builder.CreateInsertElement(Ops[0], Ops[1], Ops[2]);
  } else if (VPInst->getOpcode() == Instruction::ExtractElement) {
    assert(Ops.size() == 2 &&
           "ExtractElement instruction should have two operands.");
    SerialInst = Builder.CreateExtractElement(Ops[0], Ops[1]);
  } else if (VPInst->getOpcode() == Instruction::Alloca) {
    assert(Ops.size() == 1 && "Alloca instruction should have one operand.");
    // TODO: We don't represent alloca attributes in VPInstruction, so
    // underlying instruction must exist!
    auto *OrigAlloca = cast<AllocaInst>(VPInst->getUnderlyingValue());
    AllocaInst *SerialAlloca = Builder.CreateAlloca(
        OrigAlloca->getAllocatedType(),
        cast<PointerType>(VPInst->getType())->getAddressSpace(), Ops[0]);
    SerialAlloca->setAlignment(OrigAlloca->getAlign());
    SerialAlloca->setUsedWithInAlloca(OrigAlloca->isUsedWithInAlloca());
    SerialAlloca->setSwiftError(OrigAlloca->isSwiftError());
    SerialInst = SerialAlloca;
  } else if (VPInst->getOpcode() == Instruction::AtomicRMW) {
    assert(Ops.size() == 2 &&
           "AtomicRMW instruction should have two operands.");
    // BinOp for atomicrmw instruction is needed, so underlying instruction must
    // exist. Should we have VPlan specialization of this opcode for capturing
    // BinOp?
    auto *OrigAtomicRMW = cast<AtomicRMWInst>(VPInst->getUnderlyingValue());
    AtomicRMWInst *SerialAtomicRMW = Builder.CreateAtomicRMW(
        OrigAtomicRMW->getOperation(), Ops[0], Ops[1],
        OrigAtomicRMW->getAlign(), OrigAtomicRMW->getOrdering(),
        OrigAtomicRMW->getSyncScopeID());
    SerialAtomicRMW->setVolatile(OrigAtomicRMW->isVolatile());
    SerialInst = SerialAtomicRMW;
  } else if (VPInst->getOpcode() == Instruction::AtomicCmpXchg) {
    assert(Ops.size() == 3 &&
           "AtomicCmpXchg instruction should have just three operands.");

    // Get the underlying instruction. We assume that it always exists.
    auto *OrigAtomicCmpXchg =
        cast<AtomicCmpXchgInst>(VPInst->getUnderlyingValue());

    // Create a Scalar variant copying over the attributes from the original
    // instruction.
    AtomicCmpXchgInst *SerialAtomicCmpXchg = Builder.CreateAtomicCmpXchg(
        Ops[0], Ops[1], Ops[2], OrigAtomicCmpXchg->getAlign(),
        OrigAtomicCmpXchg->getSuccessOrdering(),
        OrigAtomicCmpXchg->getFailureOrdering(),
        OrigAtomicCmpXchg->getSyncScopeID());

    // Copy other properties from the original instruction.
    SerialAtomicCmpXchg->setVolatile(OrigAtomicCmpXchg->isVolatile());
    SerialAtomicCmpXchg->setWeak(OrigAtomicCmpXchg->isWeak());
    SerialAtomicCmpXchg->setAlignment(OrigAtomicCmpXchg->getAlign());
    SerialInst = SerialAtomicCmpXchg;
    SerialInst->setName("serial.cmpxchg");
  } else if (VPInst->getOpcode() == Instruction::ExtractValue) {
    // TODO: Currently, 'extractvalue' VPInstruction drops the last argument.
    // This is an issue similar to the dropped mask-value for shufflevector
    // instruction. An assert on Ops size seems unnecessary till
    // then.
    auto *OrigExtractValueInst =
        cast<ExtractValueInst>(VPInst->getUnderlyingValue());
    assert(OrigExtractValueInst &&
           "Expect a valid underlying extractvalue instruction.");
    SerialInst = Builder.CreateExtractValue(
        Ops[0], OrigExtractValueInst->getIndices(), "serial.extractvalue");
  } else if (VPInst->getOpcode() == Instruction::InsertValue) {
    // TODO: Currently, 'insertvalue' VPInstruction drops the last argument.
    // This is an issue similar to the dropped mask-value for shufflevector
    // instruction. An assert on Ops size seems unnecessary till
    // then.
    auto *OrigInsertValueInst =
        cast<InsertValueInst>(VPInst->getUnderlyingValue());
    assert(OrigInsertValueInst &&
           "Expect a valid underlying insertvalue instruction.");
    SerialInst = Builder.CreateInsertValue(Ops[0], Ops[1],
                                           OrigInsertValueInst->getIndices(),
                                           "serial.insertvalue");
  } else if (VPInst->getOpcode() == Instruction::ShuffleVector) {
    SerialInst = Builder.CreateShuffleVector(Ops[0], Ops[1], Ops[2]);
  } else if (VPInst->getOpcode() == Instruction::Select) {
    assert(Ops.size() == 3 && "Select instruction should have three operands.");
    SerialInst = Builder.CreateSelect(Ops[0], Ops[1], Ops[2]);
  } else if (VPInst->getOpcode() == Instruction::BitCast ||
             VPInst->getOpcode() == Instruction::AddrSpaceCast) {
    assert(Ops.size() == 1 &&
           "BitCast/AddrspaceCast should have only one operand.");
    Type *DestTy = VPInst->getType();
    // Cast's destination type on operands with SOA accesses need to be set up
    // with correct type.
    if (isSOAAccess(VPInst, Plan)) {
      auto *DestPtrTy = cast<PointerType>(DestTy);
      if (!DestPtrTy->isOpaque()) {
        Type *ElemTy = DestPtrTy->getPointerElementType();
        DestTy = PointerType::get(getSOAType(ElemTy, VF),
                                  DestPtrTy->getAddressSpace());
      }
    }
    SerialInst = Builder.CreateCast(
        static_cast<Instruction::CastOps>(VPInst->getOpcode()), Ops[0], DestTy);
  } else if (VPInst->getOpcode() == Instruction::PHI) {
    assert(Ops.size() == 0 && "No operands expected for serialized PHIs.");
    SerialInst = Builder.CreatePHI(VPInst->getType(), VPInst->getNumOperands(),
                                   "serial.phi");
  } else {
    LLVM_DEBUG(dbgs() << "VPInst: "; VPInst->dump());
    llvm_unreachable("Serialization support for opcode is not implemented.");
  }

  return SerialInst;
}

void VPOCodeGen::createEmptyLoop() {

  LoopScalarBody = OrigLoop->getHeader();
  BasicBlock *LoopPreHeader = OrigLoop->getLoopPreheader();
  LoopExitBlock = OrigLoop->getExitBlock();

  assert(LoopPreHeader && "Must have loop preheader");
  assert(LoopExitBlock && "Must have an exit block");

  // There is an issue calling SCEV when we create TripCount instruction(s).
  // The call to SCEV should be done before any new basic block is created
  // otherwise it may create some additional instructions like lcssa phis
  // in the unexpected places, reflecting that not all basic blocks are linked
  // to their successors (at least the last one).
  IRBuilder<> LBuilder(LoopPreHeader->getTerminator());
  getOrCreateTripCount(OrigLoop, LBuilder);
}

void VPOCodeGen::unlinkOrigHeaderPhis() {
  BasicBlock *Header = OrigLoop->getHeader();
  for (auto &Phi: Header->phis())
    Phi.removeIncomingValue(OrigPreHeader, false);
}

void VPOCodeGen::dropExternalValsFromMaps() {
  for (auto V : VPValsToFlushForVF) {
    assert((isa<VPExternalDef>(V) || isa<VPConstant>(V) ||
            isa<VPMetadataAsValue>(V)) &&
           "Unknown external VPValue.");

    VPWidenMap.erase(V);
    VPScalarMap.erase(V);
  }
}

void VPOCodeGen::finalizeLoop() {
  // Fix phis.
  fixNonInductionVPPhis();

  if (!OrigLoopUsed) {
    // Remove uses of incoming values in original header.
    unlinkOrigHeaderPhis();
    // Unlink the exit block from the original latch. So we don't have uses of
    // the old loop body at all.
    BasicBlock *Header = OrigLoop->getHeader();
    BasicBlock *Latch = OrigLoop->getLoopLatch();
    auto CurrTerm = Latch->getTerminator();
    auto Br = BranchInst::Create(Header);
    ReplaceInstWithInst(CurrTerm, Br);
  }

  // Attach the new loop to the original preheader
  auto *Plan = const_cast<VPlanVector *>(this->Plan);

  cast<BranchInst>(OrigPreHeader->getTerminator())
      ->setOperand(0, getScalarValue(
                          // FIXME: Better consts everywhere.
                          &Plan->getEntryBlock(), 0));
  // Find last block in cfg.
  auto LastVPBB = Plan->getExitBlock();
  BasicBlock *LastBB = cast<BasicBlock>(getScalarValue(&*LastVPBB, 0));

  // Update external scalar uses.
  for (auto *VPExtUse : Plan->getExternals().externalUses()) {
    if (!VPExtUse->hasUnderlying())
      continue; // fake external use, nothing to fixup

    auto *ExtUse = cast<PHINode>(VPExtUse->getUnderlyingValue());
    assert(ExtUse->getNumOperands() == 1 && "Not in LCSSA form!");
    ExtUse->removeIncomingValue(0u, false /* Don't remove empty phi */);
    ExtUse->addIncoming(getScalarValue(VPExtUse->getOperand(0), 0), LastBB);
  }
  // Update instructins that should be genarated under predicates.
  predicateInstructions();

  VPLoopInfo *VPLI = Plan->getVPLoopInfo();
  VPBasicBlock *VHeader = (*VPLI->begin())->getHeader();
  LoopVectorBody = cast<BasicBlock>(getScalarValue(VHeader, 0));
  LoopVectorBody->setName("vector.body");

  // Anchor point to emit/lower remarks from VPLoops to outgoing llvm::Loops.
  // This should be done before LoopInfo gets invalidated/recomputed.
  // TODO: Currently only scalar loop remarks are emitted here. Move invocation
  // of lowerRemarksForVectorLoops here as well.
  emitRemarksForScalarLoops();

  DT->recalculate(*LoopVectorBody->getParent());
  LI->releaseMemory();
  LI->analyze(*DT);

  NewLoop = LI->getLoopFor(LoopVectorBody);
  OrigLoop = LI->getLoopFor(LoopScalarBody);

  // Preserve LocRange info in outgoing LoopID after LoopInfo is recomputed.
  preserveLoopIDDbgMDs();
}

void VPOCodeGen::updateAnalysis() {
  // Forget the original basic block.
  PSE.getSE()->forgetLoop(OrigLoop);

  // Update the dominator tree information.
  assert(DT->properlyDominates(LoopBypassBlocks.front(), LoopExitBlock) &&
         "Entry does not dominate exit.");

  if (!DT->getNode(LoopVectorBody))
    DT->addNewBlock(LoopVectorBody, LoopVectorPreHeader);

  DT->addNewBlock(LoopMiddleBlock, LoopVectorBody);
  DT->addNewBlock(LoopScalarPreHeader, LoopBypassBlocks[0]);
  DT->changeImmediateDominator(LoopScalarBody, LoopScalarPreHeader);
  DT->changeImmediateDominator(LoopExitBlock, LoopBypassBlocks[0]);

  // LLVM_DEBUG(DT->verifyDomTree());
}

BasicBlock &VPOCodeGen::getFunctionEntryBlock() const {
  return OrigLoop->getHeader()->getParent()->front();
}

/// Reverse vector \p Vec. \p OriginalVL specifies the original vector length
/// of the value before vectorization.
/// If the original value was scalar, a vector <A0, A1, A2, A3> will be just
/// reversed to <A3, A2, A1, A0>. If the original value was a vector
/// (OriginalVL > 1), the function will do the following:
/// <A0, B0, A1, B1, A2, B2, A3, B3> -> <A3, B3, A2, B2, A1, B1, A0, B0>
Value *VPOCodeGen::reverseVector(Value *Vec, unsigned OriginalVL) {
  unsigned NumElts = cast<FixedVectorType>(Vec->getType())->getNumElements();
  SmallVector<Constant *, 8> ShuffleMask;
  for (unsigned i = 0; i < NumElts; i += OriginalVL)
    for (unsigned j = 0; j < OriginalVL; j++)
      ShuffleMask.push_back(
          Builder.getInt32(NumElts - (i + 1) * OriginalVL + j));

  return Builder.CreateShuffleVector(Vec, UndefValue::get(Vec->getType()),
                                     ConstantVector::get(ShuffleMask),
                                     "reverse");
}

Value *VPOCodeGen::getStepVector(Value *Val, int StartIdx, Value *Step,
                                 Instruction::BinaryOps BinOp) {
  // Create and check the types.
  assert(Val->getType()->isVectorTy() && "Must be a vector");
  int VLen = cast<FixedVectorType>(Val->getType())->getNumElements();

  Type *STy = Val->getType()->getScalarType();
  assert((STy->isIntegerTy() || STy->isFloatingPointTy()) &&
         "Induction Step must be an integer or FP");
  assert(Step->getType() == STy && "Step has wrong type");

  SmallVector<Constant *, 8> Indices;

  if (STy->isIntegerTy()) {
    // Create a vector of consecutive numbers from zero to VF.
    for (int i = 0; i < VLen; ++i)
      Indices.push_back(ConstantInt::get(STy, StartIdx + i));

    // Add the consecutive indices to the vector value.
    Constant *Cv = ConstantVector::get(Indices);
    assert(Cv->getType() == Val->getType() && "Invalid consecutive vec");
    Step = Builder.CreateVectorSplat(VLen, Step);
    assert(Step->getType() == Val->getType() && "Invalid step vec");
    // FIXME: The newly created binary instructions should contain nsw/nuw
    // flags, which can be found from the original scalar operations.
    Step = Builder.CreateMul(Cv, Step);
    return Builder.CreateAdd(Val, Step, "induction");
  }

  // Floating point induction.
  assert((BinOp == Instruction::FAdd || BinOp == Instruction::FSub) &&
         "Binary Opcode should be specified for FP induction");
  // Create a vector of consecutive numbers from zero to VF.
  for (int i = 0; i < VLen; ++i)
    Indices.push_back(ConstantFP::get(STy, (double)(StartIdx + i)));

  // Add the consecutive indices to the vector value.
  Constant *Cv = ConstantVector::get(Indices);

  Step = Builder.CreateVectorSplat(VLen, Step);

  // Floating point operations had to be 'fast' to enable the induction.
  FastMathFlags Flags;
  Flags.setFast();

  Value *MulOp = Builder.CreateFMul(Cv, Step);
  if (isa<Instruction>(MulOp))
    // Have to check, MulOp may be a constant
    cast<Instruction>(MulOp)->setFastMathFlags(Flags);

  Value *BOp = Builder.CreateBinOp(BinOp, Val, MulOp, "induction");
  if (isa<Instruction>(BOp))
    cast<Instruction>(BOp)->setFastMathFlags(Flags);
  return BOp;
}

static bool isOrUsesVPInduction(VPInstruction *VPI) {
  auto IsVPInductionRelated = [](const VPValue *V) {
    return isa<VPInductionInit>(V) || isa<VPInductionInitStep>(V);
  };

  // Really, need to recurse, but that is not required at the moment.
  // This is a temporary fix, will be removed after scalar/vector analysis
  // implemented.
  return IsVPInductionRelated(VPI) ||
         llvm::any_of(VPI->operands(), IsVPInductionRelated);
}

// TODO: replace with a query to a real analysis.
bool VPOCodeGen::needScalarCode(VPInstruction *V) {
  return isOrUsesVPInduction(V);
}

bool VPOCodeGen::isScalarArgument(StringRef FnName, unsigned Idx) {
  if (isOpenCLReadChannel(FnName) || isOpenCLWriteChannel(FnName)) {
    return (Idx == 0);
  }
  return false;
}

// Clone loop body, w/o preheader. The existing llvm utilities are making
// many things that we don't need (like updateing DOM) and require preheader and
// some other data/info. This utility is much simpler. It clones all basic
// blocks and loops that belong to the passed \p OrigLoop and inserts the cloned
// blocks before \p Before. Nothing else is needed for us during CG - the loop
// info and DOM/PDOM will be rebuilt after CG.
static Loop *cloneLoopBody(BasicBlock *Before, Loop *OrigLoop,
                           ValueToValueMapTy &VMap, const Twine &NameSuffix,
                           LoopInfo *LI,
                           SmallVectorImpl<BasicBlock *> &Blocks) {
  Function *F = OrigLoop->getHeader()->getParent();
  Loop *ParentLoop = OrigLoop->getParentLoop();
  DenseMap<Loop *, Loop *> LMap;

  Loop *NewLoop = LI->AllocateLoop();
  LMap[OrigLoop] = NewLoop;
  if (ParentLoop)
    ParentLoop->addChildLoop(NewLoop);
  else
    LI->addTopLevelLoop(NewLoop);

  // Clone the loop structure.
  for (Loop *CurLoop : OrigLoop->getLoopsInPreorder()) {
    Loop *&NewLoop = LMap[CurLoop];
    if (!NewLoop) {
      NewLoop = LI->AllocateLoop();
      LMap[CurLoop] = NewLoop;

      // Establish the parent/child relationship.
      Loop *OrigParent = CurLoop->getParentLoop();
      assert(OrigParent && "Could not find the original parent loop");
      Loop *NewParentLoop = LMap[OrigParent];
      assert(NewParentLoop && "Could not find the new parent loop");

      NewParentLoop->addChildLoop(NewLoop);
    }
  }

  // Clone basic blocks.
  for (BasicBlock *BB : OrigLoop->getBlocks()) {
    Loop *CurLoop = LI->getLoopFor(BB);
    Loop *&NewLoop = LMap[CurLoop];
    assert(NewLoop && "Expecting new loop to be allocated");

    BasicBlock *NewBB = CloneBasicBlock(BB, VMap, NameSuffix, F);
    VMap[BB] = NewBB;

    // Update LoopInfo.
    NewLoop->addBasicBlockToLoop(NewBB, *LI);
    Blocks.push_back(NewBB);
  }

  for (BasicBlock *BB : OrigLoop->getBlocks()) {
    // Update loop headers.
    Loop *CurLoop = LI->getLoopFor(BB);
    assert(CurLoop && "Value should not be nullptr!");
    if (BB == CurLoop->getHeader())
      LMap[CurLoop]->moveToHeader(cast<BasicBlock>(VMap[BB]));
  }

  F->getBasicBlockList().splice(Before->getIterator(), F->getBasicBlockList(),
                                NewLoop->getHeader()->getIterator(), F->end());

  return NewLoop;
}

// Clone the given Scalar-loop and connect it to nodes NewLoopPred and
// NewLoopSucc in the CFG.
//
//  NewLoopPred                              NewLoopPred
//      |                                         |
//      V                                         V
//  NewLoopSucc       cloneScalarLoop()      OrigLp.clone
//      |           ------------------->          |
//      .                                         V
//      .                                    MewLoopSucc
//      .                                         |
//   OrigLp                                       .
//                                                .
//                                                .
//                                             OrigLp
//
//
template <class VPPeelRemainderTy>
Loop *VPOCodeGen::cloneScalarLoop(Loop *OrigLP, BasicBlock *NewLoopPred,
                                  BasicBlock *NewLoopSucc,
                                  VPPeelRemainderTy *LoopInst,
                                  const Twine &Name) {
  // Determine live-out type based on scalar peel/remainder.
  using OrigLiveOutTy = typename std::conditional<
      std::is_same<VPPeelRemainderTy, VPScalarPeel>::value, VPPeelOrigLiveOut,
      VPRemainderOrigLiveOut>::type;

  // Make sure that NewLoopPred and NewLoopSucc are connected.
  assert(count(successors(NewLoopPred), NewLoopSucc) == 1 &&
         "Expect NewLoopPred and NewLoopSucc to be connected.");

  // Make sure that the original loop has a unique exit-block.
  assert(OrigLP->getUniqueExitBlock() &&
         "The original loop should have a valid unique exit block.");

  // This is a map that holds mapping of values of new-loop that correspond
  // to the old-loop.
  ValueToValueMapTy VMap;

  // This is the vector of blocks that would belong to the newly cloned loop.
  SmallVector<BasicBlock *, 16> ClonedLoopBlocks;

  // Clone the loop body.
  Loop *NewLoop =
      cloneLoopBody(NewLoopSucc, OrigLP /*The original loop.*/,
                    VMap /*Value2Value Map*/, Name, LI, ClonedLoopBlocks);

  // Remap everything cloned.
  remapInstructionsInBlocks(ClonedLoopBlocks, VMap);

  if (LoopInst) {
    // Update VPPeelReainder instruction.
    LoopInst->setClonedLoop(NewLoop);
    // Remap uses in VPPeelRemainder.
    for (unsigned I = 0; I < LoopInst->getNumOperands(); I++) {
      Use *OrigU = LoopInst->getLiveIn(I);
      User *NewUser = cast<User>(MapValue(OrigU->getUser(), VMap));
      LoopInst->setClonedLiveIn(I,
                                &NewUser->getOperandUse(OrigU->getOperandNo()));
    }

    // Remap liveouts.
    for (auto U : LoopInst->users()) {
      auto *LO = cast<OrigLiveOutTy>(U);
      LO->setClonedLiveOutVal(MapValue(LO->getLiveOutVal(), VMap));
    }
  }

  // Connect the NewLoopPred to the new-loop preheader.
  NewLoopPred->getTerminator()->replaceUsesOfWith(NewLoopSucc,
                                                  NewLoop->getLoopPreheader());

  // Connect the new loops exit to the provided  NewLoopSucc node.
  NewLoop->getLoopLatch()->getTerminator()->replaceUsesOfWith(
      OrigLP->getUniqueExitBlock(), NewLoopSucc);

  return NewLoop;
}

template Loop *VPOCodeGen::cloneScalarLoop(Loop *OrigLP,
                                           BasicBlock *NewLoopPred,
                                           BasicBlock *NewLoopSucc,
                                           VPScalarRemainder *LoopInst,
                                           const Twine &Name);
template Loop *VPOCodeGen::cloneScalarLoop(Loop *OrigLP,
                                           BasicBlock *NewLoopPred,
                                           BasicBlock *NewLoopSucc,
                                           VPScalarPeel *LoopInst,
                                           const Twine &Name);

void VPOCodeGen::addMaskToSVMLCall(Function *OrigF, Value *CallMaskValue,
                                   AttributeList OrigAttrs,
                                   SmallVectorImpl<Value *> &VecArgs,
                                   SmallVectorImpl<Type *> &VecArgTys,
                                   SmallVectorImpl<AttributeSet> &VecArgAttrs) {
  assert(CallMaskValue && "Expected mask to be present");
  auto *VecTy = cast<FixedVectorType>(VecArgTys[0]);
  assert(
      VecTy->getNumElements() ==
          cast<FixedVectorType>(CallMaskValue->getType())->getNumElements() &&
      "Re-vectorization of SVML functions is not supported yet");

  if (VecTy->getPrimitiveSizeInBits().getFixedSize() < 512) {
    // For 128-bit and 256-bit masked calls, mask value is appended to the
    // parameter list. For example:
    //
    //  %sin.vec = call <4 x float> @__svml_sinf4_mask(<4 x float>, <4 x i32>)
    VectorType *MaskTyExt = VectorType::get(
        IntegerType::get(OrigF->getContext(), VecTy->getScalarSizeInBits()),
        VecTy->getElementCount());
    Value *MaskValueExt = Builder.CreateSExt(CallMaskValue, MaskTyExt);
    VecArgTys.push_back(MaskTyExt);
    VecArgs.push_back(MaskValueExt);
    VecArgAttrs.push_back(AttributeSet());
  } else {
    // Compared with 128-bit and 256-bit calls, 512-bit masked calls need extra
    // pass-through source parameters. We don't care about masked-out lanes, so
    // just pass undef for that parameter. For example:
    //
    // %sin.vec = call <16 x float> @__svml_sinf16_mask(<16 x float>, <16 x i1>,
    //            <16 x float>)
    SmallVector<Type *, 1> NewArgTys;
    SmallVector<Value *, 1> NewArgs;
    SmallVector<AttributeSet, 1> NewArgAttrs;

    Type *SourceTy = VecTy;
    StringRef FnName = OrigF->getName();
    if (FnName == "sincos" || FnName == "sincosf")
      SourceTy = StructType::get(VecTy, VecTy);

    Constant *Undef = UndefValue::get(SourceTy);

    NewArgTys.push_back(SourceTy);
    NewArgs.push_back(Undef);
    NewArgAttrs.push_back(AttributeSet());

    NewArgTys.push_back(CallMaskValue->getType());
    NewArgs.push_back(CallMaskValue);
    NewArgAttrs.push_back(AttributeSet());

    NewArgTys.append(VecArgTys.begin(), VecArgTys.end());
    NewArgs.append(VecArgs.begin(), VecArgs.end());
    NewArgAttrs.append(VecArgAttrs.begin(), VecArgAttrs.end());

    VecArgTys = std::move(NewArgTys);
    VecArgs = std::move(NewArgs);
    VecArgAttrs = std::move(NewArgAttrs);
  }
}

void VPOCodeGen::initOpenCLScalarSelectSet(
    ArrayRef<const char *> ScalarSelects) {

  for (const char *SelectFuncName : ScalarSelects) {
    ScalarSelectSet.insert(SelectFuncName);
  }
}

bool VPOCodeGen::isOpenCLSelectMask(StringRef FnName, unsigned Idx) {
  return Idx == 2 && ScalarSelectSet.count(std::string(FnName));
}

Value *VPOCodeGen::getVLSLoadStoreMask(VectorType *WideValueType, int GroupSize) {
  unsigned NumElements = cast<FixedVectorType>(WideValueType)->getNumElements();
  if (NumElements == VF * GroupSize && !MaskValue)
    return nullptr;

  Value *MaskToUse = MaskValue;
  if (!MaskToUse)
    MaskToUse = ConstantInt::getTrue(
        FixedVectorType::get(Type::getInt1Ty(WideValueType->getContext()), VF));

  SmallVector<int, 32> ShuffleMask;
  for (unsigned Lane = 0; Lane < VF; ++Lane)
    for (int Elt = 0; Elt < GroupSize; ++Elt)
      ShuffleMask.push_back(Lane);

  // Remaining elements are not read (were created to make type a power-of-two
  // sized so that it's allocated size matched the bit-width of the type
  // itself on all the platforms).
  for (unsigned Idx = VF * GroupSize; Idx < NumElements; ++Idx)
    ShuffleMask.push_back(VF);

  auto *False = ConstantInt::getFalse(MaskToUse->getType());

  return Builder.CreateShuffleVector(MaskToUse, False, ShuffleMask);
}

VPlanPeelingVariant *VPOCodeGen::getGuaranteedPeeling() const {
  VPlanPeelingVariant *PreferredPeeling = Plan->getPreferredPeeling(VF);

  // Absence of any peeling information means there will be no peel loop, which
  // is the same as VPlanStaticPeeling{0}.
  if (!PreferredPeeling)
    return &VPlanStaticPeeling::NoPeelLoop;

  // As of now, dynamic peel loop can be skipped at run-time at least because of
  // missed targetAlignment (i.e. when peeled pointer is not aligned on element
  // boundary).
  // With the new cfg merger, the static peeling is executed always except cases
  // when it does not leave room for any vector iterations (i.e. when
  // peel_tc + vf*uf > ub). In these cases we don't execute main loop but
  // execute remainder.
  // So we can guaranty that any static peeling will be executed before main
  // vector looop but can't do the same for dynamic peeling.
  if (isa<VPlanStaticPeeling>(PreferredPeeling))
    return PreferredPeeling;

  return nullptr;
}

Value *VPOCodeGen::codeGenVPInvSCEVWrapper(VPInvSCEVWrapper *SW) {
  VPlanSCEV *VPScev = SW->getSCEV();
  VPlanScalarEvolutionLLVM *VPSE =
      static_cast<VPlanScalarEvolutionLLVM *>(Plan->getVPSE());
  auto *Scev = VPSE->toSCEV(VPScev);
  SCEVExpander Exp(VPSE->getSE(), *Plan->getDataLayout(), ".Ind.");
  Value *InvBase = Exp.expandCodeFor(
      Scev, Scev->getType(), OrigLoop->getLoopPreheader()->getTerminator());

  return InvBase;
}

template <class VPPeelRemainderTy>
void VPOCodeGen::vectorizeScalarPeelRem(VPPeelRemainderTy *LoopReuse) {
  if (LoopReuse->isCloningRequired()) {
    // Clone before processing if needed.
    VPBasicBlock *ParentSucc = LoopReuse->getParent()->getSingleSuccessor();
    BasicBlock *SuccBB = cast<BasicBlock>(getScalarValue(ParentSucc, 0));
    ReplaceInstWithInst(Builder.GetInsertBlock()->getTerminator(),
                        BranchInst::Create(SuccBB));
    cloneScalarLoop(LoopReuse->getLoop(), Builder.GetInsertBlock(), SuccBB,
                    LoopReuse, ".sl.clone");
  }

  // Capture outgoing scalar loop.
  OutgoingScalarLoopHeaders.push_back(
      std::make_pair(LoopReuse, LoopReuse->getLoop()->getHeader()));

  // Make the current block predecessor of the original loop header.
  ReplaceInstWithInst(Builder.GetInsertBlock()->getTerminator(),
                      BranchInst::Create(LoopReuse->getLoop()->getHeader()));
  // Replace operands (original incoming values) with the new ones from VPlan.
  // This includes the exit block.
  for (unsigned Idx = 0; Idx < LoopReuse->getNumOperands(); ++Idx) {
    Use *OrigUse = LoopReuse->getLiveIn(Idx);
    OrigUse->set(getScalarValue(LoopReuse->getOperand(Idx), 0));
    if (auto *Phi = dyn_cast<PHINode>(OrigUse->getUser()))
      Phi->setIncomingBlock(OrigUse->getOperandNo(), Builder.GetInsertBlock());
  }
  OrigLoopUsed = true;
}

static bool isOpcodeCGSVADriven(VPInstruction *VPInst) {
  // TODO: Temporary switch to track list of opcodes that have been uplifted to
  // do SVA-driven scalarization. This will be used as staging area until all
  // scalarization related code duplication is removed in every opcode's vector
  // CG.
  switch (VPInst->getOpcode()) {
  default: {
    return false;
  }
  case Instruction::GetElementPtr:
  case Instruction::BitCast:
  case Instruction::AddrSpaceCast:
    return true;
  }
}

void VPOCodeGen::processInstruction(VPInstruction *VPInst) {
  setBuilderDebugLoc(VPInst->getDebugLocation());
  generateScalarCode(VPInst);
  if (isOpcodeCGSVADriven(VPInst)) {
    // TODO: Drop this check when needVectorCode is updated to use SVA.
    if (Plan->getVPlanSVA()->instNeedsVectorCode(VPInst) ||
        requiresUnsupportedSVAFeatures(VPInst, Plan))
      generateVectorCode(VPInst);
  } else {
    // Opcode not uplifted - perform ad-hoc checks before code generation.
    generateVectorCode(VPInst);
  }
}

void VPOCodeGen::generateScalarCode(VPInstruction *VPInst) {
  if (!isOpcodeCGSVADriven(VPInst)) {
    LLVM_DEBUG(
        dbgs()
        << "[VPOCG] SVA-based scalarization is not supported for opcode.\n");
    return;
  }

  // Helper lambda to scalarize the VPInstruction for a specific lane.
  auto GenerateScalarInstForLane = [this, VPInst](unsigned Lane) {
    Value *ScalarInst = generateSerialInstruction(
        VPInst, map_range(VPInst->operands(), [this, Lane](VPValue *Op) {
          return getScalarValue(Op, Lane);
        }));
    VPScalarMap[VPInst][Lane] = ScalarInst;
  };

  // Use SVA to drive scalarization decisions for first and last lane.
  if (Plan->getVPlanSVA()->instNeedsFirstScalarCode(VPInst))
    GenerateScalarInstForLane(0);

  if (Plan->getVPlanSVA()->instNeedsLastScalarCode(VPInst))
    GenerateScalarInstForLane(VF - 1);
}

void VPOCodeGen::generateVectorCode(VPInstruction *VPInst) {
  auto *SVA = Plan->getVPlanSVA();

  // Don't vectorize if VPInst is only used in scalar context.
  if (!needVectorCode(VPInst))
    return;

  auto *DA = Plan->getVPlanDA();
  auto &OptRptStats = getOptReportStats(VPInst);

  switch (VPInst->getOpcode()) {
  case Instruction::PHI: {
    vectorizeVPPHINode(cast<VPPHINode>(VPInst));
    return;
  }

  case VPInstruction::Blend: {
    vectorizeBlend(cast<VPBlendInst>(VPInst));
    return;
  }

  case Instruction::Alloca: {
    serializeWithPredication(VPInst);
    return;
  }

  case Instruction::AtomicCmpXchg:
  case Instruction::AtomicRMW: {
    serializeWithPredication(VPInst);
    return;
  }

  case VPInstruction::ConstStepVector: {
    // This would be a vector <i64 0, ..., i64 UB-1>.
    VPConstStepVector *CV = cast<VPConstStepVector>(VPInst);
    SmallVector<Constant *, 16> Indices;
    for (int I = CV->getStart(), J = 0; J != CV->getNumSteps();
         I += CV->getStep(), ++J)
      Indices.push_back(
          ConstantInt::get(Type::getInt64Ty(*Plan->getLLVMContext()), I));

    Constant *Cv = ConstantVector::get(Indices);
    VPWidenMap[CV] = Cv;
    return;
  }

  case Instruction::GetElementPtr: {
    VPGEPInstruction *GEP = cast<VPGEPInstruction>(VPInst);

    auto GetOrigVL = [](Type *Type) -> unsigned {
      auto *VecType = dyn_cast<VectorType>(Type);
      if (!VecType)
        return 1;
      assert(!VecType->getElementCount().isScalable() &&
             "Re-vectorizing scalable vector type isn't supported!");
      return VecType->getElementCount().getKnownMinValue();
    };

    unsigned MaxVL =
        std::accumulate(GEP->op_begin(), GEP->op_end(), 1,
                        [GetOrigVL](unsigned Max, VPValue *Op) {
                          return std::max(Max, GetOrigVL(Op->getType()));
                        });

    auto GetVectorOp = [=](VPValue *V) {
      return replicateVectorElts(getVectorValue(V),
                                 MaxVL / GetOrigVL(V->getType()), Builder);
    };

    VPValue *GepBasePtr = GEP->getPointerOperand();
    // Widen the base-pointer if needed.
    Value *WideGepBasePtr;
    if (SVA->operandNeedsVectorCode(GEP, 0 /*PtrOp*/))
      WideGepBasePtr = GetVectorOp(GepBasePtr);
    else {
      assert(SVA->operandNeedsFirstScalarCode(GEP, 0 /*PtrOp*/) &&
             "Only Vector or FirstScalar pointer operand is expected during "
             "GEP vectorization.");
      WideGepBasePtr = getScalarValue(GepBasePtr, 0);
    }

    // Widen the indices.
    assert((requiresUnsupportedSVAFeatures(GEP, Plan) ||
            all_of(GEP->indices(),
                   [GEP, SVA](VPValue *Idx) {
                     return SVA->operandNeedsVectorCode(
                         GEP, GEP->getOperandIndex(Idx));
                   })) &&
           "Trying to vectorize a strictly scalar index.");
    SmallVector<Value *, 4> OpsV;
    llvm::transform(GEP->indices(), std::back_inserter(OpsV),
                    [GetVectorOp](VPValue *Op) { return GetVectorOp(Op); });

    // TODO: SOAMemRef transformations needs to be updated to correctly set
    // SourceElementType which can then be used here.
    StringRef GepName =
        isSOAAccess(GEP, Plan) ? "soa_vectorGEP" : "mm_vectorGEP";
    Type *SourceElementType =
        GEP->isOpaque() ? isSOAAccess(GEP, Plan)
                              ? getSOAType(GEP->getSourceElementType(), VF)
                              : GEP->getSourceElementType()
                        : WideGepBasePtr->getType()
                              ->getScalarType()
                              ->getPointerElementType();

    Value *VectorGEP = Builder.CreateGEP(SourceElementType,
                                         WideGepBasePtr, OpsV, GepName);
    cast<GetElementPtrInst>(VectorGEP)->setIsInBounds(GEP->isInBounds());

    VPWidenMap[GEP] = VectorGEP;

    return;
  }

  case Instruction::ZExt:
  case Instruction::SExt:
  case Instruction::FPToUI:
  case Instruction::FPToSI:
  case Instruction::FPExt:
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
  case Instruction::SIToFP:
  case Instruction::UIToFP:
  case Instruction::Trunc:
  case Instruction::FPTrunc: {
    /// Vectorize casts.
    auto Opcode = static_cast<Instruction::CastOps>(VPInst->getOpcode());
    Type *ScalTy = VPInst->getType();
    Type *VecTy = getWidenedType(ScalTy, VF);
    VPValue *ScalOp = VPInst->getOperand(0);
    Value *VecOp = getVectorValue(ScalOp);
    VPWidenMap[VPInst] = Builder.CreateCast(Opcode, VecOp, VecTy);
    return;
  }

  case Instruction::FNeg: {
    if (DA->isUniform(*VPInst)) {
      serializeInstruction(VPInst);
      return;
    }
    // Widen operand.
    Value *Src = getVectorValue(VPInst->getOperand(0));

    // Create wide instruction.
    auto UnOpCode = static_cast<Instruction::UnaryOps>(VPInst->getOpcode());
    Value *V = Builder.CreateUnOp(UnOpCode, Src);

    // Copy operator flags stored in VPInstruction (example FMF, wrapping
    // flags).
    if (auto *UnaryInst = dyn_cast<Instruction>(V))
      VPInst->copyOperatorFlagsTo(UnaryInst);

    VPWidenMap[VPInst] = V;
    return;
  }

  case Instruction::Freeze: {
    // Widen operand.
    Value *Src = getVectorValue(VPInst->getOperand(0));
    Value *V = Builder.CreateFreeze(Src);
    VPWidenMap[VPInst] = V;
    return;
  }

  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::URem:
  case Instruction::SRem: {
    bool DivisorIsSafe = isDivisorSpeculationSafeForDivRem(
        VPInst->getOpcode(), VPInst->getOperand(1));
    if (MaskValue && !DivisorIsSafe) {
      if (DA->isUniform(*VPInst)) {
        serializePredicatedUniformInstruction(VPInst);
        return;
      } else if (!EnableIntDivRemBlendWithSafeValue) {
        serializeWithPredication(VPInst);
        // Remark: division was scalarized due to fp-model requirements
        OptRptStats.SerializedInstRemarks.emplace_back(
            15566, Instruction::getOpcodeName(VPInst->getOpcode()));
        return;
      }
    }
    LLVM_FALLTHROUGH;
  }
  case Instruction::Add:
  case Instruction::FAdd:
  case Instruction::Sub:
  case Instruction::FSub:
  case Instruction::Mul:
  case Instruction::FMul:
  case Instruction::FDiv:
  case Instruction::FRem:
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor: {

    if (DA->isUniform(*VPInst)) {
      serializeInstruction(VPInst);
      return;
    }
    // Widen binary operands.
    Value *A = getVectorValue(VPInst->getOperand(0));
    Value *B = getVectorValue(VPInst->getOperand(1));

    // Create wide instruction.
    auto BinOpCode = static_cast<Instruction::BinaryOps>(VPInst->getOpcode());
    Value *V = Builder.CreateBinOp(BinOpCode, A, B);

    // Copy operator flags stored in VPInstruction (example FMF, wrapping
    // flags).
    if (auto *BinaryInst = dyn_cast<Instruction>(V))
      VPInst->copyOperatorFlagsTo(BinaryInst);

    VPWidenMap[VPInst] = V;
    // TODO: Need to check for scalar code generation for the most of
    // VPInstructions.
    if (needScalarCode(VPInst)) {
      Value *AScal = getScalarValue(VPInst->getOperand(0), 0);
      Value *BScal = getScalarValue(VPInst->getOperand(1), 0);
      Value *VScal = Builder.CreateBinOp(
          static_cast<Instruction::BinaryOps>(VPInst->getOpcode()), AScal,
          BScal);
      VPScalarMap[VPInst][0] = VScal;
      if (auto Inst = dyn_cast<Instruction>(VScal))
        VPInst->copyOperatorFlagsTo(Inst);
    }
    return;
  }
  case Instruction::ICmp: {
    // FIXME: Proper SVA-driven scalar ICMP codegen (for needLastScalar).
    if (!MaskValue && Plan->getVPlanSVA()->instNeedsFirstScalarCode(VPInst)) {
      Value *A = getScalarValue(VPInst->getOperand(0), 0);
      Value *B = getScalarValue(VPInst->getOperand(1), 0);
      auto *Cmp = cast<VPCmpInst>(VPInst);
      VPScalarMap[VPInst][0] = Builder.CreateICmp(Cmp->getPredicate(), A, B);
    }

    if (MaskValue || Plan->getVPlanSVA()->instNeedsVectorCode(VPInst) ||
        Plan->getVPlanSVA()->instNeedsLastScalarCode(VPInst)) {
      Value *A = getVectorValue(VPInst->getOperand(0));
      Value *B = getVectorValue(VPInst->getOperand(1));
      auto *Cmp = cast<VPCmpInst>(VPInst);
      VPWidenMap[VPInst] = Builder.CreateICmp(Cmp->getPredicate(), A, B);
      // TODO: We need something like below for the lastScalarCode with
      // the correction to use the last lane. At the moment we don't process SVA
      // bits so that scalar value is not used anyway.
#if 0
      if (!MaskValue && Plan->getVPlanSVA()->instNeedsLastScalarCode(VPInst)) {
        Constant *LastL = ConstantInt::get(
            Type::getInt32Ty(VPInst->getType()->getContext()), VF - 1);
        VPScalarMap[VPInst][VF - 1] =
            Builder.CreateExtractElement(getVectorValue(VPInst), LastL);
      }
#endif
    }
    return;
  }
  case Instruction::FCmp: {
    Value *A = getVectorValue(VPInst->getOperand(0));
    Value *B = getVectorValue(VPInst->getOperand(1));
    auto *FCmp = cast<VPCmpInst>(VPInst);
    Value *VecFCmp = Builder.CreateFCmp(FCmp->getPredicate(), A, B);
    // Copy fast math flags.
    if (auto *VecFCmpInst = dyn_cast<Instruction>(VecFCmp))
      VPInst->copyOperatorFlagsTo(VecFCmpInst);

    VPWidenMap[VPInst] = VecFCmp;
    return;
  }
  case Instruction::Load: {
    vectorizeLoadInstruction(cast<VPLoadStoreInst>(VPInst), true);
    return;
  }
  case Instruction::Store: {
    vectorizeStoreInstruction(cast<VPLoadStoreInst>(VPInst), true);
    return;
  }
  case Instruction::Select: {
    vectorizeSelectInstruction(VPInst);
    return;
  }
  case Instruction::BitCast: {
    vectorizeCast<BitCastInst>(VPInst);
    return;
  }
  case Instruction::AddrSpaceCast: {
    vectorizeCast<AddrSpaceCastInst>(VPInst);
    return;
  }
  case Instruction::ExtractElement: {
    vectorizeExtractElement(VPInst);
    return;
  }
  case Instruction::InsertElement: {
    vectorizeInsertElement(VPInst);
    return;
  }
  case Instruction::InsertValue:
  case Instruction::ExtractValue: {
    serializeInstruction(VPInst);
    return;
  }
  case Instruction::ShuffleVector: {
    if (DA->isUniform(*VPInst)) {
      serializeInstruction(VPInst);
      return;
    }
    vectorizeShuffle(VPInst);
    return;
  }

  case Instruction::Call: {
    VPCallInstruction *VPCall = cast<VPCallInstruction>(VPInst);
    auto *UnderlyingCI =
        dyn_cast_or_null<CallInst>(VPCall->getUnderlyingValue());

    // Ignore dbg intrinsics. This might change after discussion on
    // CMPLRLLVM-10839.
    // TODO: Check if DbgInfoIntrinsic can be identified without underlying CI.
    if (UnderlyingCI && isa<DbgInfoIntrinsic>(UnderlyingCI))
      return;

    // For all other calls vectorization scenario should be available for
    // current VF.
    assert(VPCall->getVFForScenario() == VF &&
           "Cannot find call vectorization scenario for VF.");

    // TODO: Update the following assertion when we implement function pointers
    // for OpenMP.
    if (VPCall->isIntelIndirectCall() &&
        VPCall->getVectorizationScenario() !=
        VPCallInstruction::CallVecScenariosTy::VectorVariant) {
      if (FatalErrorHandler)
        FatalErrorHandler(OrigLoop->getHeader()->getParent());
      else
        llvm_unreachable("Intel indirect call should have vector-variants!");
    }

    // Drop lifetime_start/end intrinsics operating on private memory
    // inside the loop leaving those that are in preheader/exit blocks.
    if (VPCall->isLifetimeStartOrEndIntrinsic()) {
      if (auto *PrivPtr = dyn_cast_or_null<VPAllocatePrivate>(
              getVPValuePrivateMemoryPtr(VPCall->getOperand(1)))) {
        const VPBasicBlock *VPCallBB = VPCall->getParent();
        VPLoop *VPLp = Plan->getVPLoopInfo()->getLoopFor(VPCallBB);
        if (!VPLp || VPLp->getLoopPreheader() == VPCallBB ||
            VPLp->getUniqueExitBlock() == VPCallBB) {
          vectorizeLifetimeStartEndIntrinsic(VPCall);
        }
        return;
      }
    }

    switch (VPCall->getVectorizationScenario()) {
    case VPCallInstruction::CallVecScenariosTy::DoNotWiden: {
      // Currently only kernel convergent uniform calls and uniform calls
      // without side-effects are strictly marked to be not widened.
      // TODO: this case must be handled via VPlan to VPlan bypass
      // infrastructure.
      processPredicatedNonWidenedUniformCall(VPCall);
      return;
    }
    case VPCallInstruction::CallVecScenariosTy::LibraryFunc: {
      // Call that can be vectorized using vector library for current VF or
      // pumped multi-way using a lower VF.
      vectorizeLibraryCall(VPCall);
      ++OptRptStats.VectorMathCalls;
      return;
    }
    case VPCallInstruction::CallVecScenariosTy::TrivialVectorIntrinsic: {
      // Call that can be vectorized trviailly using vector overload of
      // intrinsics.
      vectorizeTrivialIntrinsic(VPCall);
      ++OptRptStats.VectorMathCalls;
      return;
    }
    case VPCallInstruction::CallVecScenariosTy::UnmaskedWiden: {
      auto *NotAllZero = getMaskNotAllZero();
      const VFInfo *MatchedVariant = VPCall->getVectorVariant();
      if (!MatchedVariant)
        report_fatal_error("No matching vector variant for an unmasked call!");
      SmallVector<Value *, 4> CallResults;
      generateVectorCalls(VPCall, 1 /* PumpFactor */, false /* IsMasked */, MatchedVariant,
                          Intrinsic::not_intrinsic /*No vector intrinsic*/,
                          CallResults);
      assert(CallResults.size() == 1 && "Pumping is unexpected for unmasked functions!");
      VPWidenMap[VPCall] = CallResults[0];

      PredicatedInstructions.emplace_back(cast<Instruction>(CallResults[0]),
                                          NotAllZero);
      // TODO: Is that good enough?
      ++OptRptStats.VectorVariantCalls;
      return;
    }
    case VPCallInstruction::CallVecScenariosTy::VectorVariant: {
      // Call that can be vectorized using SIMD vector-variants.
      vectorizeVecVariant(VPCall);
      ++OptRptStats.VectorVariantCalls;
      return;
    }
    case VPCallInstruction::CallVecScenariosTy::Serialization: {
      // Call cannot be vectorized for current context, emulate with
      // serialization
      auto *Func = VPCall->getCalledFunction();
      serializeWithPredication(VPCall);
      // Skip reporting lifetime markers
      if (!VPCall->isLifetimeStartOrEndIntrinsic()) {
        ++OptRptStats.SerializedCalls;
        // Below we add in OptReport remarks about serialized calls.
        // Check if called function is indirect call
        assert(VPCall->getSerialReason() !=
                   VPCallInstruction::SerializationReasonTy::UNDEFINED &&
               "Serialization reason is undefined");
        if (Func == nullptr)
          OptRptStats.SerializedInstRemarks.emplace_back(
              15557 + VPCall->getSerialReasonNum(), "");
        else
          OptRptStats.SerializedInstRemarks.emplace_back(
              15557 + VPCall->getSerialReasonNum(), (Func->getName()).str());
      }
      return;
    }
    default:
      llvm_unreachable(
          "VPCallInstruction does not have a valid decision for VF.");
    }
  }

  case VPInstruction::TransformLibraryCall: {
    auto *TransformedCall = cast<VPTransformLibraryCall>(VPInst);
    vectorizeLibraryCall(TransformedCall);
    ++OptRptStats.VectorMathCalls;
    return;
  }

  case VPInstruction::AllZeroCheck: {
    Value *A = getVectorValue(VPInst->getOperand(0));
    Type *Ty = A->getType();

    if (MaskValue) {
      // Consider the inner loop that is executed under the mask, after loop CFU
      // transformation and predication it looks something like this:
      //
      //   REGION: loop19 (BP: NULL)
      //   BB16 (BP: NULL) :
      //    i1 %vp24064 = block-predicate i1 %loop_incoming_mask
      //   SUCCESSORS(1):BB11
      //   no PREDECESSORS

      //   BB11 (BP: NULL) :
      //    i1 %vp24384 = block-predicate i1 %loop_incoming_mask
      //    i64 %vp54864 = phi  [ i64 %vp20544, BB23 ],  [ i64 1, BB16 ]
      //    i32 %vp55072 = phi  [ i32 %vp20704, BB23 ],  [ i32 %vp49616, BB16 ]
      //    i1 %inner_loop_specific_mask =
      //         phi  [ i1 true, BB16 ],
      //              [ i1 %inner_loop_specific_mask.next, BB23 ]
      //   SUCCESSORS(1):mask_region24
      //   PREDECESSORS(2): BB16 BB23

      //   REGION: mask_region24 (BP: NULL)
      //   BB21 (BP: NULL) :
      //    i1 %vp24544 = block-predicate i1 %loop_incoming_mask
      //   SUCCESSORS(1):BB22
      //   no PREDECESSORS

      //   BB22 (BP: NULL) :
      //    i1 %real_mask =
      //        and i1 %loop_incoming_mask i1, %inner_loop_specific_mask
      //    i1 %real_mask_predicate = block-predicate i1 %real_mask
      //    i32 addrspace(1)* %vp38496 =
      //        getelementptr inbounds i32 addrspace(1)* %input i64 %vp54864
      //    i32 %ld = load i32 addrspace(1)* %vp38496
      //    i1 %vp55440 = icmp i32 %vp55072 i32 %ld
      //    i32 %vp55616 =  select i1 %vp55440 i32 %ld i32 %vp55072
      //    i64 %vp55888 = add i64 %vp54864 i64 1
      //   SUCCESSORS(1):BB17
      //   PREDECESSORS(1): BB21

      //   BB17 (BP: NULL) :
      //    i1 %vp25472 = not i1 %inner_loop_specific_mask
      //    i1 %vp25632 = and i1 %loop_incoming_mask i1 %vp25472
      //    i1 %vp25920 = block-predicate i1 %loop_incoming_mask
      //    i1 %vp56048 = icmp i64 %vp55888 i64 %vp1824
      //
      //    ;; This "not" is because original latch was at false successor
      //    i1 %continue_cond = not i1 %vp56048
      //    i1 %inner_loop_specific_mask.next =
      //       and i1 %continue_cond i1  %inner_loop_specific_mask
      //
      //    ;; Live-outs updates
      //
      //   i1 %vp20864 = all-zero-check i1 %inner_loop_specific_mask.next
      //   no SUCCESSORS
      //   PREDECESSORS(1): BB22
      //
      // After vectorizing the loop above we create something like
      //
      //    %wide.ld = gather %vector_gep, %real_mask, undef_vector
      //
      // All-zero-check is dependent on the result of this gather, including the
      // lanes that were masked out by %real_mask (which includes
      // %loop_incoming_mask). That means that for lanes masked out by
      // %loop_incoming_mask we can only have undef values and naive
      //
      //   %vp1 bitcast <VF x i1> %innerl_loop_specific_mask.next to iVF
      //   %should_exit %cmp %vp1, 0
      //   br i1 %should_exit, %exit_bb, %header_bb
      //
      // would result in branching based on that undef, which is UB. To avoid
      // this, use only the active lanes when calculating the all-zero-check.
      // Note, that technically we can use either %loop_incoming_mask or
      // %real_mask. The former is easily available, so use it. Also,
      //
      //   and undef, %vpval
      //
      // semantics isn't immediately obvious for the reader.
      //
      // Another approach to this issue is to modify Predicator/LoopCFU to have
      // a single phi/value for the mask, but it looks like much more work.
      // Changing the semantics of the all-zero-check VPInstruction to reflect
      // the mask (similar to loads/stores/calls) doesn't seem to have any
      // drawbacks and is much easier to do.
      A = Builder.CreateAnd(A, MaskValue);
    }

    // Bitcast <VF x i1> to an integer value VF bits long.
    Type *IntTy =
        IntegerType::get(Ty->getContext(), Ty->getPrimitiveSizeInBits());
    auto *BitCastInst = Builder.CreateBitCast(A, IntTy);

    // Compare the bitcast value to zero. The compare will be true if all
    // the i1 masks in <VF x i1> are false.
    auto *CmpInst =
        Builder.CreateICmpEQ(BitCastInst, Constant::getNullValue(IntTy));

    VPScalarMap[VPInst][0] = CmpInst;
    if (SVA->instNeedsBroadcast(VPInst)) {
      // Broadcast the compare and set as the widened value.
      auto *V = Builder.CreateVectorSplat(VF, CmpInst, "broadcast");
      VPWidenMap[VPInst] = V;
    }
    return;
  }
  case VPInstruction::Not: {
    Value *A = getVectorValue(VPInst->getOperand(0));
    Value *V = Builder.CreateNot(A);
    VPWidenMap[VPInst] = V;
    if (needScalarCode(VPInst)) {
      Value *AScal = getScalarValue(VPInst->getOperand(0), 0);
      Value *VScal = Builder.CreateNot(AScal);
      VPScalarMap[VPInst][0] = VScal;
    }
    return;
  }
  case VPInstruction::Pred: {
    // Pred instruction just marks the block mask.
    Value *A = getVectorValue(VPInst->getOperand(0));
    setMaskValue(A);
    return;
  }
  case VPInstruction::ReductionInit: {
    // Generate a broadcast/splat of reduction's identity. If a start value is
    // specified for reduction, then insert into lane 0 after broadcast.
    // Example -
    // i32 %vp0 = reduction-init i32 0 i32 %red.init
    //
    // Generated instructions-
    // %0 = insertelement <4 x i32> zeroinitializer, i32 %red.init, i32 0
    //
    // If the instruction is marked as Scalar, we need only lane 0.

    auto *Init = cast<VPReductionInit>(VPInst);
    if (Init->isScalar()) {
      Value *Result = nullptr;
      if (VPInst->getNumOperands() > 1)
        // Use start value.
        Result = getScalarValue(VPInst->getOperand(1), 0);
      else
        Result = getScalarValue(VPInst->getOperand(0), 0);
      VPScalarMap[VPInst][0] = Result;
    } else {
      Value *Identity = getVectorValue(VPInst->getOperand(0));
      if (VPInst->getNumOperands() > 1) {
        auto *StartVPVal = VPInst->getOperand(1);
        auto *StartVal = getScalarValue(StartVPVal, 0);
        Identity = Builder.CreateInsertElement(
          Identity, StartVal, Builder.getInt32(0), "red.init.insert");
      }
      VPWidenMap[VPInst] = Identity;
    }

    return;
  }
  case VPInstruction::ReductionFinal: {
    vectorizeReductionFinal(cast<VPReductionFinal>(VPInst));
    return;
  }
  case VPInstruction::ReductionFinalUdr: {
    // Call combiner for each pointer in private memory and accumulate the
    // results in original variable corresponding to the UDR.
    Value *Orig = getScalarValue(VPInst->getOperand(1), 0);
    auto *Priv = cast<VPAllocatePrivate>(VPInst->getOperand(0));
    Function *CombinerFn = cast<VPReductionFinalUDR>(VPInst)->getCombiner();

    for (unsigned Lane = 0; Lane < getVF(); Lane++) {
      Value *LanePvtPtr = getScalarValue(Priv, Lane);
      Builder.CreateCall(CombinerFn, {Orig, LanePvtPtr});
    }
    return;
  }
  case VPInstruction::ReductionFinalInscan: {
    // We need the last vector lane as the final reduction value is there.
    // ReductionFinalInscan opcode is needed for correct work of CFG merger.
    VPScalarMap[VPInst][0] = vectorizeExtractLastVectorLane(VPInst);
    return;
  }
  case VPInstruction::InductionInit: {
    vectorizeInductionInit(cast<VPInductionInit>(VPInst));
    return;
  }
  case VPInstruction::InductionInitStep: {
    vectorizeInductionInitStep(cast<VPInductionInitStep>(VPInst));
    return;
  }
  case VPInstruction::InductionFinal: {
    vectorizeInductionFinal(cast<VPInductionFinal>(VPInst));
    return;
  }
  case VPInstruction::AllocatePrivate: {
    vectorizeAllocatePrivate(cast<VPAllocatePrivate>(VPInst));
    return;
  }
  case VPInstruction::OrigTripCountCalculation: {
    // TODO: Inline getOrCreateTripCount's implementation to here once we
    // complete transition to an explicit CFG representation in VPlan.
    VPScalarMap[VPInst][0] = getOrCreateTripCount(
        cast<VPOrigTripCountCalculation>(VPInst)->getOrigLoop(), Builder);
    return;
  }
  case VPInstruction::VectorTripCountCalculation: {
    auto *VTCCalc = cast<VPVectorTripCountCalculation>(VPInst);
    Value *OrigTC = getScalarValue(VTCCalc->getOperand(0), 0);
    // Get adjustment, which can be added when we have a peel loop.
    // In this case we calculate by the following formula:
    //   vector_ub = orig_ub - (orig_ub - adjustment) % VF.
    Value *Adjustment = VTCCalc->getNumOperands() > 1
                            ? getScalarValue(VTCCalc->getOperand(1), 0)
                            : nullptr;
    VectorTripCount = calculateVectorTC(OrigTC, Builder, UF * VF, Adjustment);
    VPScalarMap[VPInst][0] = VectorTripCount;
    // Meanwhile, assert that the UF implicitly used by the CG is the same as
    // represented explicitly.
    assert(VTCCalc->getUF() == UF && "Mismatch in UFs!");
    return;
  }
  case VPInstruction::ActiveLane: {
    assert(!MaskValue && "ActiveLane calculation is expected to be unmasked!");
    assert(DA->isUniform(*VPInst) &&
           "ActiveLane instruction is expected to be uniform!");
    VPValue *MaskOp = VPInst->getOperand(0);
    Value *WidenedMaskOp = getVectorValue(MaskOp);
    Type *Ty = WidenedMaskOp->getType();
    Type *IntTy = IntegerType::get(Ty->getContext(), VF);
    auto *CastedMask = Builder.CreateBitCast(WidenedMaskOp, IntTy);
    Module *M = OrigLoop->getHeader()->getModule();
    Function *F =
        Intrinsic::getDeclaration(M, Intrinsic::cttz, CastedMask->getType());
    // TODO: We don't branch on any uniform condition inside linearized control
    // flow, so there is no issue ActiveLane returning undef for the all-false mask,
    // technically. Still, ask for the non-undef value to ease debugging. Note
    // that currently this is a temporary solution and we don't try to get the
    // most performance out of it.
    Value *CTTZ =
        Builder.CreateCall(F, {CastedMask, Builder.getFalse()}, "cttz");
    VPScalarMap[VPInst][0] = CTTZ;
    return;
  }
  case VPInstruction::ActiveLaneExtract: {
    if (!VPInst->getType()->isVectorTy()) {
      VPScalarMap[VPInst][0] = Builder.CreateExtractElement(
          getVectorValue(VPInst->getOperand(0)),
          getScalarValue(VPInst->getOperand(1), 0));
      return;
    }
    // Original type was vector. Suppose it was <2 x type>, VF == 2 and active
    // lane is equals to "1" (in runtime). We'd need to extract elements (2, 3)
    // from the wide vector in this case:
    //
    // VecValue: |0.1, 0.2 | 1.1, 1.2 |
    //
    // ActiveScalar: <1.1, 1.2>
    auto *Val = getVectorValue(VPInst->getOperand(0));
    auto *ActiveLane = getScalarValue(VPInst->getOperand(1), 0);
    auto *I32Ty = IntegerType::getInt32Ty(ActiveLane->getType()->getContext());
    ActiveLane = Builder.CreateZExtOrTrunc(ActiveLane, I32Ty);
    unsigned NumElt = cast<FixedVectorType>(VPInst->getType())->getNumElements();
    Value *Result = UndefValue::get(VPInst->getType());
    auto *IndexBase =
        Builder.CreateMul(ActiveLane, ConstantInt::get(I32Ty, NumElt));
    for (unsigned EltIdx = 0; EltIdx < NumElt; ++EltIdx) {
      auto *Index =
          Builder.CreateAdd(IndexBase, ConstantInt::get(I32Ty, EltIdx));
      auto *Extract = Builder.CreateExtractElement(Val, Index);
      Result = Builder.CreateInsertElement(Result, Extract, EltIdx);
    }
    VPScalarMap[VPInst][0] = Result;
    return;
  }
  case VPInstruction::PushVF: {
    unsigned NewVF = cast<VPPushVF>(VPInst)->getVF();
    unsigned NewUF = cast<VPPushVF>(VPInst)->getUF();
    assert((NewVF != 0 && NewUF != 0) && "expected nonzero VF and UF");
    VFStack.emplace_back(VF, UF, LastPushVF);
    dropExternalValsFromMaps();
    VF = NewVF;
    UF = NewUF;
    LastPushVF = VPInst;
    return;
  }
  case VPInstruction::PopVF: {
    assert(!VFStack.empty() && "unexpected PopVF");
    auto V = VFStack.pop_back_val();
    dropExternalValsFromMaps();
    VF = std::get<0>(V);
    UF = std::get<1>(V);
    LastPushVF = std::get<2>(V);
    return;
  }
  case Instruction::Br:
    // Do nothing.
    return;
  case VPInstruction::ScalarRemainder:
    vectorizeScalarPeelRem(cast<VPScalarRemainder>(VPInst));
    return;
  case VPInstruction::ScalarPeel: {
    auto LoopReuse = cast<VPScalarPeel>(VPInst);
    vectorizeScalarPeelRem(LoopReuse);
    // In scalar peel replace original preheader with the new one in the header
    // phis.
    auto NewPH = Builder.GetInsertBlock();
    for (auto &Phi : LoopReuse->getLoop()->getHeader()->phis())
      Phi.replaceIncomingBlockWith(OrigPreHeader, NewPH);
    return;
  }
  case VPInstruction::PeelOrigLiveOut: {
    auto *LiveOut = cast<VPPeelOrigLiveOut>(VPInst);
    VPScalarMap[LiveOut][0] = const_cast<Value *>(LiveOut->getLiveOutVal());
    return;
  }
  case VPInstruction::RemOrigLiveOut: {
    auto *LiveOut = cast<VPRemainderOrigLiveOut>(VPInst);
    VPScalarMap[LiveOut][0] = const_cast<Value *>(LiveOut->getLiveOutVal());
    return;
  }
  case VPInstruction::InvSCEVWrapper: {
    auto *SCEVWrapper = cast<VPInvSCEVWrapper>(VPInst);
    assert(DA->isUniform(*SCEVWrapper) &&
           "Expect the inv-scev-wrapper instruction to be uniform.");
    assert(Plan->getVPlanSVA()->instNeedsFirstScalarCode(SCEVWrapper) &&
           "Expected inv-scev-wrapper instruction to be marked first-scalar by "
           "SVA.");
    VPScalarMap[SCEVWrapper][0] = codeGenVPInvSCEVWrapper(SCEVWrapper);
    return;
  }
  case VPInstruction::PrivateFinalUncondMem:
  case VPInstruction::PrivateFinalUncond: {
    vectorizePrivateFinalUncond(VPInst);
    return;
  }
  case VPInstruction::PrivateFinalCondMem:
  case VPInstruction::PrivateFinalCond: {
    // Pseudo IR generated to finalize conditional last private entity -
    // %priv.final = private-final-c %exit, %idx, %orig
    //
    // ; Find max index where condition was true
    // %idx.reduce = llvm.vector.reduce.smax.v2i64(%idx.vec)
    // ; Identify where max index is set in final vector
    // %max.idx.cmp = %idx.reduce == %idx.vec
    // ; Obtain lane for extraction
    // %bsfintmask = bitcast.<2 x i1>.i2(%max.idx.cmp);
    // %lane = @llvm.cttz.i2(%bsfintmask,  1);
    // ; Extract final value and store back to original
    // %orig = extractelement %exit.vec, %lane
    //
    Value *VecExit = getVectorValue(VPInst->getOperand(0));
    Value *VecIndex = getVectorValue(VPInst->getOperand(1));

    Value *IdxReduceCall = Builder.CreateIntMaxReduce(VecIndex, true);
    Value *CmpInst = Builder.CreateICmp(
        ICmpInst::ICMP_EQ, VecIndex,
        Builder.CreateVectorSplat(VF, IdxReduceCall, "broadcast"),
        "priv.idx.cmp");

    Type *IntTy = IntegerType::get(CmpInst->getContext(), VF);
    Value *CastedMask = Builder.CreateBitCast(CmpInst, IntTy);

    Module *M = OrigLoop->getHeader()->getModule();
    Function *CTTZ =
        Intrinsic::getDeclaration(M, Intrinsic::cttz, CastedMask->getType());
    Value *BsfCall =
        Builder.CreateCall(CTTZ, {CastedMask, Builder.getTrue()}, "cttz");

    // TODO:  if (VPInst->getOperand(0)->getType()->isVectorTy())
    Value *PrivExtract =
        Builder.CreateExtractElement(VecExit, BsfCall, "priv.extract");
    VPScalarMap[VPInst][0] = PrivExtract;
    return;
  }
  case VPInstruction::PrivateFinalMasked:
  case VPInstruction::PrivateFinalMaskedMem: {
    // Pseudo IR generated to finalize unconditional last private in masked mode
    // loop.
    // %priv.final = private-final-masked %exit, %mask, %orig
    //  ==>
    // %bsfintmask = bitcast.<2 x i1>.i2(%mask); Obtain lane for extraction
    // %lz = @llvm.ctlz.i2(%bsfintmask, 1);
    // %lane = VF - 1 - %lz;
    // %last_val = extractelement %exit, %lane ; extract final value
    //
    // NOTE: The block above is encapsulated into the check of mask is zero in
    // VPlan, before CG, so the code will be generated in the mask_nonzero block
    // below.
    //
    // orig_bb:
    //   %pred = all_zero_check(%mask)
    //   br %pred label %mask_zero, label %mask_nonzero
    // mask_nonzero:
    //   ; this one is transformed by this routine.
    //   %priv.final = private-final-masked %exit, %mask, %orig
    //   %br label %mask_zero
    // mask_zero:
    //   %last_v = phi [%priv.final, %mask_nonzero], [%orig, %orig_bb]
    //
    Value *VecMask = getVectorValue(VPInst->getOperand(1));
    Type *IntTy = IntegerType::get(Builder.getContext(), VF);
    Value *CastedMask = Builder.CreateBitCast(VecMask, IntTy);

    Module *M = OrigLoop->getHeader()->getModule();
    Function *CTLZ =
        Intrinsic::getDeclaration(M, Intrinsic::ctlz, CastedMask->getType());
    Value *BsfCall =
        Builder.CreateCall(CTLZ, {CastedMask, Builder.getTrue()}, "ctlz");
    Value *Lane = Builder.CreateSub(ConstantInt::get(IntTy, VF - 1), BsfCall);
    Value *VecExit = getVectorValue(VPInst->getOperand(0));
    // TODO:  if (VPInst->getOperand(0)->getType()->isVectorTy())
    Value *PrivExtract =
        Builder.CreateExtractElement(VecExit, Lane, "priv.extract");
    VPScalarMap[VPInst][0] = PrivExtract;
    return;
  }
  case VPInstruction::PrivateFinalArray: {
    // We need to copy array from last private allocated memory into the
    // original array location.
    Value *Orig = getScalarValue(VPInst->getOperand(1), 0);

    VPAllocatePrivate *Priv = cast<VPAllocatePrivate>(VPInst->getOperand(0));
    Type *ElementType = Priv->getAllocatedType();
    if (Priv->isSOALayout()) {

      BasicBlock *BBExit =
          processSOALayout(Priv, Orig, ElementType, Builder.getInt64(VF - 1));

      State->CFG.PrevBB = BBExit;

    } else {

      // In case of non-SOA layout it will be enough to copy memory from last
      // private into the original array.
      Value *Res = getScalarValue(Priv, VF - 1);
      const DataLayout &DL =
          OrigLoop->getHeader()->getModule()->getDataLayout();
      Builder.CreateMemCpy(Orig, DL.getPrefTypeAlign(Orig->getType()), Res,
                           Priv->getOrigAlignment(),
                           DL.getTypeAllocSize(ElementType));
    }

    return;
  }

  case VPInstruction::PrivateFinalArrayMasked: {
    VPAllocatePrivate *Priv = cast<VPAllocatePrivate>(VPInst->getOperand(0));
    Type *ElementType = Priv->getAllocatedType();

    Value *Orig = getScalarValue(VPInst->getOperand(1), 0);
    Value *VecMask = getVectorValue(VPInst->getOperand(2));
    Type *IntTy = IntegerType::get(Builder.getContext(), VF);
    Value *CastedMask = Builder.CreateBitCast(VecMask, IntTy);

    Module *M = OrigLoop->getHeader()->getModule();
    Function *CTLZ =
        Intrinsic::getDeclaration(M, Intrinsic::ctlz, CastedMask->getType());
    Value *BsfCall =
        Builder.CreateCall(CTLZ, {CastedMask, Builder.getTrue()}, "ctlz");
    Value *Lane = Builder.CreateSub(ConstantInt::get(IntTy, VF - 1), BsfCall);
    if (Priv->isSOALayout()) {
      BasicBlock *BBExit = processSOALayout(Priv, Orig, ElementType, Lane);

      State->CFG.PrevBB = BBExit;
    } else {
      Value *VecExit = getVectorValue(VPInst->getOperand(0));
      Value *PrivExtract =
          Builder.CreateExtractElement(VecExit, Lane, "priv.extract");

      const DataLayout &DL =
          OrigLoop->getHeader()->getModule()->getDataLayout();
      Builder.CreateMemCpy(Orig, DL.getPrefTypeAlign(Orig->getType()),
                           PrivExtract, Priv->getOrigAlignment(),
                           DL.getTypeAllocSize(ElementType));
    }
    return;
  }

  case VPInstruction::PrivateLastValueNonPOD: {
    // We need to copy private from last private allocated memory into the
    // original private location.
    Value *Orig = getScalarValue(VPInst->getOperand(1), 0);
    VPAllocatePrivate *Priv = cast<VPAllocatePrivate>(VPInst->getOperand(0));
    Value *Res = getScalarValue(Priv, VF - 1);
    auto *CopyAssignFn =
        cast<VPPrivateLastValueNonPODInst>(VPInst)->getCopyAssign();
    Builder.CreateCall(CopyAssignFn, {Orig, Res});
    return;
  }

  case VPInstruction::PrivateLastValueNonPODMasked: {
    // Pseudo IR generated to last non-POD private in masked mode
    // loop.
    // private-last-value-nonpod-masked %exit, %orig, %mask
    //  ==>
    // %bsfintmask = bitcast.<2 x i1>.i2(%mask); Obtain lane for extraction
    // %lz = @llvm.ctlz.i2(%bsfintmask, 1);
    // %lane = VF - 1 - %lz;
    // %priv_extract = extractelement %exit, %lane ; extract final value
    // call omp.copy_assign(%orig, %priv_extract)
    //
    // NOTE: The block above is encapsulated into the check of mask is zero in
    // VPlan, before CG, so the code will be generated in the mask_nonzero block
    // below.
    //
    // orig_bb:
    //   %pred = all_zero_check(%mask)
    //   br %pred label %mask_zero, label %mask_nonzero
    // mask_nonzero:
    //   ; this one is transformed by this routine.
    //   private-last-value-nonpod-masked %exit, %orig, %mask
    //   %br label %mask_zero
    // mask_zero:
    //
    Value *Orig = getScalarValue(VPInst->getOperand(1), 0);
    Value *VecMask = getVectorValue(VPInst->getOperand(2));
    Type *IntTy = IntegerType::get(Builder.getContext(), VF);
    Value *CastedMask = Builder.CreateBitCast(VecMask, IntTy);

    Module *M = OrigLoop->getHeader()->getModule();
    Function *CTLZ =
        Intrinsic::getDeclaration(M, Intrinsic::ctlz, CastedMask->getType());
    Value *BsfCall =
        Builder.CreateCall(CTLZ, {CastedMask, Builder.getTrue()}, "ctlz");
    Value *Lane = Builder.CreateSub(ConstantInt::get(IntTy, VF - 1), BsfCall);
    Value *VecExit = getVectorValue(VPInst->getOperand(0));
    Value *PrivExtract =
        Builder.CreateExtractElement(VecExit, Lane, "priv.extract");
    auto *CopyAssignFn =
        cast<VPPrivateLastValueNonPODMaskedInst>(VPInst)->getCopyAssign();
    Builder.CreateCall(CopyAssignFn, {Orig, PrivExtract});
    return;
  }
  case VPInstruction::VLSLoad: {
    auto *VLSLoad = cast<VPVLSLoad>(VPInst);
    assert(DA->isUniform(*VLSLoad) &&
           "VLSLoad must produce a uniform value!");
    auto *Base = getScalarValue(VLSLoad->getOperand(0), 0);
    auto *VecTy = cast<VectorType>(VLSLoad->getType());
    auto *CastedBase = Builder.CreateBitCast(
        Base, VecTy->getPointerTo(
                  cast<PointerType>(Base->getType())->getAddressSpace()));
    auto GroupSize = VLSLoad->getGroupSize();
    auto *LoadMask = getVLSLoadStoreMask(VecTy, GroupSize);
    if (!LoadMask) {
      auto *WideLoad = cast<LoadInst>(Builder.CreateAlignedLoad(
          VecTy, CastedBase, VLSLoad->getAlignment(), "vls.load"));

      for (std::pair<unsigned, MDNode *> It : VLSLoad->getMetadata())
        WideLoad->setMetadata(It.first, It.second);

      VPScalarMap[VLSLoad][0] = WideLoad;
      OptRptStats.UnmaskedVLSLoads += VLSLoad->getNumOrigLoads();
      return;
    }

    auto *WideLoad =
        Builder.CreateMaskedLoad(VecTy, CastedBase, VLSLoad->getAlignment(),
                                 LoadMask, nullptr /* PassThru */, "vls.load");
    VPScalarMap[VLSLoad][0] = WideLoad;
    OptRptStats.MaskedVLSLoads += VLSLoad->getNumOrigLoads();
    return;
  }
  case VPInstruction::VLSExtract: {
    auto *Extract = cast<VPVLSExtract>(VPInst);
    assert(DA->isUniform(*Extract->getOperand(0)) &&
           "Operand of VLSExtract must be a uniform value!");

    auto NumEltsPerValue = Extract->getNumGroupEltsPerValue();

    auto Offset = Extract->getOffset();
    auto GroupSize = Extract->getGroupSize();
    SmallVector<int, 32> ShuffleMask;
    for (unsigned Lane = 0; Lane < VF; ++Lane)
      for (unsigned Part = 0; Part < NumEltsPerValue; ++Part)
        ShuffleMask.push_back(Lane * GroupSize + Offset + Part);

    auto *WideValue = getScalarValue(Extract->getOperand(0), 0);
    auto *Result =
        Builder.CreateShuffleVector(WideValue, WideValue, ShuffleMask);
    Result->setName(Extract->getName());
    VPWidenMap[Extract] = Result;
    return;
  }
  case VPInstruction::VLSInsert: {
    auto *Insert = cast<VPVLSInsert>(VPInst);
    assert(DA->isUniform(*Insert) && "VLSInsert must produce a uniform value!");
    assert(DA->isUniform(*Insert->getOperand(0)) &&
           "Orig wide value operand of VLSInsert must be a uniform vlaue!");
    auto *OrigWideValue = getScalarValue(Insert->getOperand(0), 0);
    auto *ValueToInsert = getVectorValue(Insert->getOperand(1));

    auto NumEltsPerValue = Insert->getNumGroupEltsPerValue();
    auto *GroupType = cast<FixedVectorType>(Insert->getOperand(0)->getType());
    auto NumEltsInGroup = GroupType->getNumElements();
    auto *CastedExtended = extendVector(ValueToInsert, NumEltsInGroup, Builder);

    auto Offset = Insert->getOffset();
    auto GroupSize = Insert->getGroupSize();
    SmallVector<int, 32> ShuffleMask;
    // Initialize as if we'd want to keep OrigWideValue only.
    for (unsigned Idx = 0; Idx < NumEltsInGroup; ++Idx)
      ShuffleMask.push_back(Idx);

    // Now update indices where we'd like to change the data.
    for (unsigned Lane = 0; Lane < VF; ++Lane)
      for (unsigned Part = 0; Part < NumEltsPerValue; ++Part) {
        auto TargetIdx = GroupSize * Lane + Offset + Part;
        auto SrcIdx = NumEltsInGroup + Lane * NumEltsPerValue + Part;
        ShuffleMask[TargetIdx] = SrcIdx;
      }

    auto *Result =
        Builder.CreateShuffleVector(OrigWideValue, CastedExtended, ShuffleMask);

    VPScalarMap[Insert][0] = Result;
    return;
  }
  case VPInstruction::VLSStore: {
    auto *VLSStore = cast<VPVLSStore>(VPInst);
    assert(DA->isUniform(*VLSStore->getOperand(0)) &&
           "Value operand of VLSStore must be uniform!");
    auto *Base = getScalarValue(VLSStore->getOperand(1), 0);
    auto *StoredValue = getScalarValue(VLSStore->getOperand(0), 0);
    auto *VecTy = cast<VectorType>(VLSStore->getOperand(0)->getType());
    auto *CastedBase = Builder.CreateBitCast(
        Base, VecTy->getPointerTo(
                  cast<PointerType>(Base->getType())->getAddressSpace()));
    auto GroupSize = VLSStore->getGroupSize();
    auto *StoreMask = getVLSLoadStoreMask(VecTy, GroupSize);
    if (!StoreMask) {
      auto *WideStore = cast<StoreInst>(Builder.CreateAlignedStore(
          StoredValue, CastedBase, VLSStore->getAlignment()));

      for (std::pair<unsigned, MDNode *> It : VLSStore->getMetadata())
        WideStore->setMetadata(It.first, It.second);

      OptRptStats.UnmaskedVLSStores += VLSStore->getNumOrigStores();
      return;
    }

    auto *WideStore = Builder.CreateMaskedStore(
        StoredValue, CastedBase, VLSStore->getAlignment(), StoreMask);
    (void)WideStore;
    OptRptStats.MaskedVLSStores += VLSStore->getNumOrigStores();
    return;
  }
  case VPInstruction::RunningInclusiveReduction: {
    auto RunningReduction = cast<VPRunningInclusiveReduction>(VPInst);
    vectorizeRunningInclusiveReduction(RunningReduction);
    return;
  }
  case VPInstruction::RunningExclusiveReduction: {
    llvm_unreachable("RunningExclusiveReduction is not implemented yet!");
    return;
  }
  case VPInstruction::ExtractLastVectorLane: {
    VPScalarMap[VPInst][0] = vectorizeExtractLastVectorLane(VPInst);
    return;
  }
  case VPInstruction::SOAExtractValue: {
    // We know our first operand is an aggregate in SOA layout. Emit a single
    // ExtractValue to get the widened value.

    // First operand is our SOA aggregate
    Value *Aggregate = getVectorValue(VPInst->getOperand(0));

    // Subsequent operands are ExtractValue indices
    SmallVector<unsigned, 1> Indices(
        map_range(drop_begin(VPInst->operands(), 1), [](const VPValue *Arg) {
          return cast<VPConstantInt>(Arg)->getZExtValue();
        }));

    VPWidenMap[VPInst] = Builder.CreateExtractValue(Aggregate, Indices);
    return;
  }
  default: {
    LLVM_DEBUG(dbgs() << "VPInst: "; VPInst->dump());
    llvm_unreachable("VPVALCG: Opcode not uplifted yet.");
  }
  }
}

Value *VPOCodeGen::vectorizeExtractLastVectorLane(VPInstruction *VPInst) {
  return getScalarValue(VPInst->getOperand(0), VF - 1);
}

BasicBlock *VPOCodeGen::processSOALayout(VPAllocatePrivate *Priv, Value *Orig,
                                         Type *ElementType,
                                         Value *ElementPosition) {
  assert(LoopPrivateVPWidenMap.count(Priv) > 0 &&
         "Expected widened alloca for SOA last private.");
  Value *Res = LoopPrivateVPWidenMap[Priv];

  // In case of SOA layout we need to extract array elements one by one from
  // the each vector and then store it to the original array. To do so we
  // create a fixed trip count loop.
  BasicBlock *BBLoop =
      SplitBlock(Builder.GetInsertBlock(), &*Builder.GetInsertPoint(), DT, LI,
                 nullptr, "array.last.private.loop");
  BasicBlock *BBExit = SplitBlock(BBLoop, BBLoop->getTerminator(), DT, LI,
                                  nullptr, "array.last.private.loop.exit");
  Builder.SetInsertPoint(BBLoop->getTerminator());

  // Creating phi node to count loop iterations.
  PHINode *Phi = Builder.CreatePHI(Type::getInt64Ty(Builder.getContext()), 2);
  Phi->addIncoming(Builder.getInt64(0), BBLoop->getSinglePredecessor());

  // Loop body. Copying element in ElementPosition position from each vector.
  Value *Ptr = Builder.CreateGEP(getSOAType(ElementType, VF), Res,
                                 {Builder.getInt64(0), Phi, ElementPosition});
  Value *Val = Builder.CreateLoad(
      cast<GetElementPtrInst>(Ptr)->getResultElementType(), Ptr);
  Value *Target =
      Builder.CreateGEP(ElementType, Orig, {Builder.getInt64(0), Phi});
  Builder.CreateStore(Val, Target);

  // Increment of loop variable.
  Value *Index = Builder.CreateAdd(Phi, Builder.getInt64(1));
  Phi->addIncoming(Index, BBLoop);
  Value *Cond = Builder.CreateICmpULT(
      Index, Builder.getInt64(cast<ArrayType>(ElementType)->getNumElements()));

  Builder.CreateCondBr(Cond, BBLoop, BBExit);
  BBLoop->getTerminator()->eraseFromParent();
  return BBExit;
}

Value *VPOCodeGen::getOrCreateVectorTripCount(Loop *L, IRBuilder<> &IBuilder) {
  if (VectorTripCount)
    return VectorTripCount;

  assert(L && "Unexpected null loop for trip count create");
  Value *TC = getOrCreateTripCount(L, IBuilder);

  VectorTripCount = calculateVectorTC(TC, IBuilder, UF * VF);
  return VectorTripCount;
}

Value *VPOCodeGen::getOrCreateTripCount(Loop *L, IRBuilder<> &IBuilder) {
  if (TripCount)
    return TripCount;

  // Find the loop boundaries.
  PredicatedScalarEvolution &PSE = Legal->getPSE();
  const SCEV *BackedgeTakenCount = PSE.getBackedgeTakenCount();
  assert(BackedgeTakenCount != PSE.getSE()->getCouldNotCompute() &&
         "Invalid loop count");

  Type *IdxTy = Legal->getWidestInductionType();

  // The exit count might have the type of i64 while the phi is i32. This can
  // happen if we have an induction variable that is sign extended before the
  // compare. The only way that we get a backedge taken count is that the
  // induction variable was signed and as such will not overflow. In such a case
  // truncation is legal.
  if (BackedgeTakenCount->getType()->getPrimitiveSizeInBits() >
      IdxTy->getPrimitiveSizeInBits())
    BackedgeTakenCount =
        PSE.getSE()->getTruncateOrNoop(BackedgeTakenCount, IdxTy);
  BackedgeTakenCount =
      PSE.getSE()->getNoopOrZeroExtend(BackedgeTakenCount, IdxTy);

  // Get the total trip count from the count by adding 1.
  const SCEV *ExitCount = PSE.getSE()->getAddExpr(
      BackedgeTakenCount, PSE.getSE()->getOne(BackedgeTakenCount->getType()));

  const DataLayout &DL = L->getHeader()->getModule()->getDataLayout();

  // Expand the trip count and place the new instructions in the preheader.
  // Notice that the pre-header does not change, only the loop body.
  SCEVExpander Exp(*PSE.getSE(), DL, "induction");

  // Count holds the overall loop count (N).
  TripCount = Exp.expandCodeFor(ExitCount, ExitCount->getType(),
                                &*IBuilder.GetInsertPoint());

  if (TripCount->getType()->isPointerTy())
    TripCount =
        CastInst::CreatePointerCast(TripCount, IdxTy, "exitcount.ptrcnt.to.int",
                                    &*IBuilder.GetInsertPoint());

  return TripCount;
}

void VPOCodeGen::predicateInstructions() {

  // For each instruction I marked for predication on value C, split I into its
  // own basic block to form an if-then construct over C. Since I may be fed by
  // an extractelement instruction or other scalar operand, we try to
  // iteratively sink its scalar operands into the predicated block. If I feeds
  // an insertelement instruction, we try to move this instruction into the
  // predicated block as well. For non-void types, a phi node will be created
  // for the resulting value (either vector or scalar).
  //
  // So for some predicated instruction, e.g. the conditional sdiv in:
  //
  // for.body:
  //  ...
  //  %add = add nsw i32 %mul, %0
  //  %cmp5 = icmp sgt i32 %2, 7
  //  br i1 %cmp5, label %if.then, label %if.end
  //
  // if.then:
  //  %div = sdiv i32 %0, %1
  //  br label %if.end
  //
  // if.end:
  //  %x.0 = phi i32 [ %div, %if.then ], [ %add, %for.body ]
  //
  // the sdiv at this point is scalarized and if-converted using a select.
  // The inactive elements in the vector are not used, but the predicated
  // instruction is still executed for all vector elements, essentially:
  //
  // vector.body:
  //  ...
  //  %17 = add nsw <2 x i32> %16, %wide.load
  //  %29 = extractelement <2 x i32> %wide.load, i32 0
  //  %30 = extractelement <2 x i32> %wide.load51, i32 0
  //  %31 = sdiv i32 %29, %30
  //  %32 = insertelement <2 x i32> undef, i32 %31, i32 0
  //  %35 = extractelement <2 x i32> %wide.load, i32 1
  //  %36 = extractelement <2 x i32> %wide.load51, i32 1
  //  %37 = sdiv i32 %35, %36
  //  %38 = insertelement <2 x i32> %32, i32 %37, i32 1
  //  %predphi = select <2 x i1> %26, <2 x i32> %38, <2 x i32> %17
  //
  // Predication will now re-introduce the original control flow to avoid false
  // side-effects by the sdiv instructions on the inactive elements, yielding
  // (after cleanup):
  //
  // vector.body:
  //  ...
  //  %5 = add nsw <2 x i32> %4, %wide.load
  //  %8 = icmp sgt <2 x i32> %wide.load52, <i32 7, i32 7>
  //  %9 = extractelement <2 x i1> %8, i32 0
  //  br i1 %9, label %pred.sdiv.if, label %pred.sdiv.continue
  //
  // pred.sdiv.if:
  //  %10 = extractelement <2 x i32> %wide.load, i32 0
  //  %11 = extractelement <2 x i32> %wide.load51, i32 0
  //  %12 = sdiv i32 %10, %11
  //  %13 = insertelement <2 x i32> undef, i32 %12, i32 0
  //  br label %pred.sdiv.continue
  //
  // pred.sdiv.continue:
  //  %14 = phi <2 x i32> [ undef, %vector.body ], [ %13, %pred.sdiv.if ]
  //  %15 = extractelement <2 x i1> %8, i32 1
  //  br i1 %15, label %pred.sdiv.if54, label %pred.sdiv.continue55
  //
  // pred.sdiv.if54:
  //  %16 = extractelement <2 x i32> %wide.load, i32 1
  //  %17 = extractelement <2 x i32> %wide.load51, i32 1
  //  %18 = sdiv i32 %16, %17
  //  %19 = insertelement <2 x i32> %14, i32 %18, i32 1
  //  br label %pred.sdiv.continue55
  //
  // pred.sdiv.continue55:
  //  %20 = phi <2 x i32> [ %14, %pred.sdiv.continue ], [ %19, %pred.sdiv.if54 ]
  //  %predphi = select <2 x i1> %8, <2 x i32> %20, <2 x i32> %5

  for (auto KV : PredicatedInstructions) {
    BasicBlock::iterator I(KV.first);
    BasicBlock *Head = I->getParent();
    auto *BB = SplitBlock(Head, &*std::next(I), DT, LI);
    auto *T = SplitBlockAndInsertIfThen(KV.second, &*I, /*Unreachable=*/false,
                                        /*BranchWeights=*/nullptr, DT, LI);
    I->moveBefore(T);
    // sinkScalarOperands(&*I);

    I->getParent()->setName(Twine("pred.") + I->getOpcodeName() + ".if");
    BB->setName(Twine("pred.") + I->getOpcodeName() + ".continue");

    // If the instruction is non-void create a Phi node at reconvergence point.
    if (!I->getType()->isVoidTy()) {
      Value *IncomingTrue = nullptr;
      Value *IncomingFalse = nullptr;

      if (I->hasOneUse() && isa<InsertElementInst>(*I->user_begin()) &&
          // If InsertElementInst is predicated itself, the movement isn't safe.
          none_of(PredicatedInstructions,
                  [&I](std::pair<Instruction *, Value *> InstPredicatePair) {
                    return InstPredicatePair.first == *I->user_begin();
                  })) {
        // If the predicated instruction is feeding an insert-element, move it
        // into the Then block; Phi node will be created for the vector.
        InsertElementInst *IEI = cast<InsertElementInst>(*I->user_begin());
        IEI->moveBefore(T);
        IncomingTrue = IEI; // the new vector with the inserted element.
        IncomingFalse = IEI->getOperand(0); // the unmodified vector
      } else {
        // Phi node will be created for the scalar predicated instruction.
        IncomingTrue = &*I;
        IncomingFalse = UndefValue::get(I->getType());
      }

      BasicBlock *PostDom = I->getParent()->getSingleSuccessor();
      assert(PostDom && "Then block has multiple successors");
      PHINode *Phi =
          PHINode::Create(IncomingTrue->getType(), 2, "", &PostDom->front());
      IncomingTrue->replaceAllUsesWith(Phi);
      Phi->addIncoming(IncomingFalse, Head);
      Phi->addIncoming(IncomingTrue, I->getParent());
    }
  }
}

SmallVector<int, 16>
VPOCodeGen::getVPShuffleOriginalMask(const VPInstruction *VPI) {
  assert(VPI->getOpcode() == Instruction::ShuffleVector &&
         "getVPShuffleOriginalMask called on non-shuffle instruction.");
  // The last operand of shufflevector is the mask and it is expected to always
  // be a constant.
  VPConstant *ShufMask =
      cast<VPConstant>(VPI->getOperand(VPI->getNumOperands() - 1));
  Constant *ShufMaskConst = ShufMask->getConstant();
  SmallVector<int, 16> Result;

  unsigned NumElts =
      cast<FixedVectorType>(ShufMaskConst->getType())->getNumElements();
  if (auto *CDS = dyn_cast<ConstantDataSequential>(ShufMaskConst)) {
    for (unsigned I = 0; I != NumElts; ++I)
      Result.push_back(CDS->getElementAsInteger(I));
    return Result;
  }
  for (unsigned I = 0; I != NumElts; ++I) {
    Constant *C = ShufMaskConst->getAggregateElement(I);
    Result.push_back(isa<UndefValue>(C) ? -1
                                        : cast<ConstantInt>(C)->getZExtValue());
  }

  return Result;
}

const VPValue *VPOCodeGen::getOrigSplatVPValue(const VPValue *V) {
  if (auto *C = dyn_cast<VPConstant>(V)) {
    if (isa<VectorType>(V->getType())) {
      Constant *SplatC =
          cast<Constant>(
              getScalarValue(const_cast<VPConstant *>(C), 0 /*Lane*/))
              ->getSplatValue();
      // We need to create a new VPConstant to represent a splat constant
      // vector.
      return const_cast<VPlanVector *>(Plan)->getVPConstant(SplatC);
    }
  }

  auto *VPInst = dyn_cast<VPInstruction>(V);
  if (!VPInst || VPInst->getOpcode() != Instruction::ShuffleVector)
    return nullptr;

  // All-zero or undef shuffle mask elements.
  if (any_of(getVPShuffleOriginalMask(VPInst),
             [](int MaskElt) -> bool { return MaskElt != 0 && MaskElt != -1; }))
    return nullptr;

  // The first shuffle source is 'insertelement' with index 0.
  auto *InsertEltVPInst = dyn_cast<VPInstruction>(VPInst->getOperand(0));
  if (!InsertEltVPInst ||
      InsertEltVPInst->getOpcode() != Instruction::InsertElement)
    return nullptr;

  if (auto *ConstIdx = dyn_cast<VPConstant>(InsertEltVPInst->getOperand(2))) {
    Value *ConstIdxV = getScalarValue(ConstIdx, 0 /*Lane*/);
    if (!isa<ConstantInt>(ConstIdxV) || !cast<ConstantInt>(ConstIdxV)->isZero())
      return nullptr;
  } else {
    return nullptr;
  }

  return InsertEltVPInst->getOperand(1);
}

Value *VPOCodeGen::createVectorPrivatePtrs(VPAllocatePrivate *V) {
  assert(LoopPrivateVPWidenMap.count(V) &&
         "Private memory pointer not widened.");

  Value *PtrToVec = LoopPrivateVPWidenMap[V];
  PointerType *PrivTy = cast<PointerType>(V->getType());

  // If PtrToVec is of an widened array-type,
  // e.g., [2 x [NumElts x Ty]],
  // %privaddr = bitcast [2 x [NumElts x Ty]]* %s.vec to [NumElts x Ty]*
  // %addr.vec = getelementptr [NumElts x Ty], [NumElts x Ty]* %privaddr,
  //                                           <2 x i32> <i32 0, i32 1>
  // We will create a vector GEP with scalar base and a vector of indices.

  SmallVector<Constant *, 16> Indices;
  // Create a vector of consecutive numbers from zero to VF-1.
  // TODO: For allocas of the format -
  //     %priv = alloca i32, i32 %n
  // generating consecutive numbers is incorrect. We need -
  //     <%n * 0, %n * 1,..., %n * VF-1>
  // to correctly compute base addresses.
  for (unsigned I = 0; I < VF; ++I)
    Indices.push_back(
        ConstantInt::get(Type::getInt32Ty(PrivTy->getContext()), I));

  Constant *Cv = ConstantVector::get(Indices);

  auto Base =
      Builder.CreateBitCast(PtrToVec, PrivTy, PtrToVec->getName() + ".bc");
  return Builder.CreateGEP(V->getAllocatedType(), Base, Cv,
                           PtrToVec->getName() + ".base.addr");
}

template <typename CastInstTy>
void VPOCodeGen::vectorizeCast(
    typename std::enable_if<
        std::is_same<CastInstTy, BitCastInst>::value ||
            std::is_same<CastInstTy, AddrSpaceCastInst>::value,
        VPInstruction>::type *VPInst) {
  auto Opcode = static_cast<Instruction::CastOps>(VPInst->getOpcode());
  bool IsBitCastInst = std::is_same<CastInstTy, BitCastInst>::value;
  bool IsNonSerializedAllocaPointer = LoopPrivateVPWidenMap.count(
      VPInst->getOperand(0)); // Is this AOS-widened ptr,
                              // TODO: Add SOA-pointer check here.

  // If the pointer is a bitcast and is exclusively used in
  // lifetime_start/end intrinsics, use the correct operands.
  // TODO: Move these checks to SVA, FirstScalar should be propagated from
  // lifetime_start/end intrinsics.
  if (IsBitCastInst && IsNonSerializedAllocaPointer &&
      isOnlyUsedInLifetimeIntrinsics(VPInst)) {
    Value *ScalarOp = LoopPrivateVPWidenMap[VPInst->getOperand(0)];
    Type *ScalarTy = VPInst->getType();
    Value *ScalarCast = Builder.CreateCast(Opcode, ScalarOp, ScalarTy);
    VPScalarMap[VPInst][0] = ScalarCast;
    return;
  }

  Value *WidenedOp = getVectorValue(VPInst->getOperand(0));
  Type *WidenedTy = getWidenedType(VPInst->getType(), VF);
  Value *WidenedCast = Builder.CreateCast(Opcode, WidenedOp, WidenedTy);
  VPWidenMap[VPInst] = WidenedCast;
}

void VPOCodeGen::vectorizeOpenCLSinCos(VPCallInstruction *VPCall,
                                       bool IsMasked) {
  // If we encounter a call to OpenCL sincos function, i.e., a call to
  // _Z6sincosfPf, the code in Intel_SVMLEmitter.cpp, currently maps that call
  // to _Z6sincosDv<VF>_fPS_ variant. The following code correctly
  // sets up the input/output arguments for that function.  '_Z6sincosfPf' has
  // the form,
  //
  // %sinVal = call float @_Z6sincosfPf(float %input, float* %cosPtr)
  // %cosVal = load float, float* %cosPtr
  //
  // The following code replaces that function call with
  // %wide.sinVal = call <8 x float> @_Z6sincosDv8_fPS_(<8 x float> %wide.input,
  //                                                    <8 x float>*
  //                                                    %cosPtr.vec)
  // %wide.load2 = load <8 x float>, <8 x float>* %cosPtr.vec, align 4

  // TODO: This is a temporary solution to fix performance issues with DPC++
  // MonteCarlo simulation code. The desirable solution would be to write
  // separate pre-processing pass that replaces 'SinCos' and other similar
  // function with their scalar equivalent 'correct' SVML functions (which
  // haven't been decided yet).  That way the vector code-generation does not
  // have to shuffle function arguments.

  SmallVector<Value *, 3> VecArgs;
  SmallVector<Type *, 3> VecArgTys;
  // For CallInsts represented as VPInstructions, arg operands are added first.
  Value *Arg1 = getVectorValue(VPCall->getOperand(0));
  VPValue *CosPtr = VPCall->getOperand(1);
  assert(
      (isa<VPAllocatePrivate>(CosPtr) && LoopPrivateVPWidenMap.count(CosPtr)) &&
      "CosPtr is expected to be loop private.");

  // Get the base-pointer for the widened CosPtr, i.e., <8 x float>*.
  // While vectorizing VPAllocatePrivate created for CosPtr, the base pointer to
  // wide alloca was added to LoopPrivateVPWidenMap.
  auto *WideCosPtr = cast<AllocaInst>(LoopPrivateVPWidenMap[CosPtr]);
  VecArgs.push_back(Arg1);
  VecArgs.push_back(WideCosPtr);
  VecArgTys.push_back(Arg1->getType());
  VecArgTys.push_back(WideCosPtr->getType());

  Function *CalledFunc = VPCall->getCalledFunction();
  assert(CalledFunc && "Unexpected null call function.");
  Function *VectorF =
      getOrInsertVectorLibFunction(CalledFunc, VF, VecArgTys, TLI,
                                   Intrinsic::not_intrinsic, IsMasked);
  assert(VectorF && "Vector function not created.");
  CallInst *VecCall = Builder.CreateCall(VectorF, VecArgs);
  // Copy fast math flags represented in VPInstruction to VecCall.
  if (isa<FPMathOperator>(VecCall))
    VPCall->copyOperatorFlagsTo(VecCall);

  // Make sure we don't lose attributes at the call site. E.g., IMF
  // attributes are taken from call sites in MapIntrinToIml to refine
  // SVML calls for precision.
  setRequiredAttributes(VPCall->getOrigCallAttrs(), VecCall);

  // Set calling convention for SVML function calls
  if (isSVMLFunction(TLI, CalledFunc->getName(), VectorF->getName()))
    VecCall->setCallingConv(CallingConv::SVML);

    // TODO: Need a VPValue based analysis for call arg memory references.
    // VPValue-based stride info also needed.
#if 0
  Loop *Lp = LI->getLoopFor(Call->getParent());
  analyzeCallArgMemoryReferences(Call, VecCall, TLI, PSE.getSE(), Lp);
#endif

  VPWidenMap[VPCall] = VecCall;
}

void VPOCodeGen::vectorizeCallArgs(VPCallInstruction *VPCall,
                                   const VFInfo *VecVariant,
                                   Intrinsic::ID VectorIntrinID,
                                   unsigned PumpPart, unsigned PumpFactor,
                                   SmallVectorImpl<Value *> &VecArgs,
                                   SmallVectorImpl<Type *> &VecArgTys,
                                   SmallVectorImpl<AttributeSet> &VecArgAttrs) {
  unsigned PumpedVF = VF / PumpFactor;
  ArrayRef<VFParameter> Parms;
  if (VecVariant) {
    Parms = VecVariant->getParameters();
  }

  Function *F = VPCall->getCalledFunction();
  assert(F && "Function not found for call instruction");
  StringRef FnName = F->getName();

  auto ProcessCallArg = [&](unsigned OrigArgIdx,
                            unsigned ParamsIdx) -> Value * {
    if (isOpenCLWriteChannelSrc(FnName, OrigArgIdx)) {
      llvm_unreachable(
          "VPVALCG: OpenCL write channel vectorization not uplifted.");
    }

    if ((!VecVariant || Parms[ParamsIdx].isVector()) &&
        !isScalarArgument(FnName, OrigArgIdx) &&
        !isVectorIntrinsicWithScalarOpAtArg(VectorIntrinID, OrigArgIdx)) {
      // This is a vector call arg, so vectorize it.
      VPValue *Arg = VPCall->getOperand(OrigArgIdx);

      // Generate the right mask for OpenCL vector 'select' intrinsic
      if (isOpenCLSelectMask(FnName, OrigArgIdx))
        return getOpenCLSelectVectorMask(Arg);

      Value *VecArg = getVectorValue(Arg);
      VecArg = generateExtractSubVector(VecArg, PumpPart, PumpFactor, Builder);
      assert(VecArg && "Vectorized call arg cannot be nullptr.");

      if (VecVariant) {
        // If this is a generated vector variant, we need to check if we had an
        // `i1` or `<N x i1>` param that we promoted to `i8`. If so, zero
        // extend the value, so that users observe the correct type.
        VectorType *VecArgTy = cast<VectorType>(VecArg->getType());
        if (VecArgTy->getElementType()->isIntegerTy(1)) {
          Type *PromotedTy = VecArgTy->getWithNewBitWidth(8);
          VecArg = Builder.CreateZExt(VecArg, PromotedTy);
        }
      }

      return VecArg;
    }
    // Linear and uniform parameters for simd functions must be passed as
    // scalars according to the vector function abi. CodeGen currently
    // vectorizes all instructions, so the scalar arguments for the vector
    // function must be extracted from them. For both linear and uniform
    // args, extract from lane 0. Linear args can use the value at lane 0
    // because this will be the starting value for which the stride will be
    // added. The same method applies to built-in functions for args that
    // need to be treated as uniform (example trivial vector intrinsics with
    // always scalar operands).

    // TODO: To support pumping for linear arguments of vector-variants, special
    // processing is needed to correctly update the linear argument for each
    // PumpPart.
    assert(PumpFactor == 1 && "Pumping feature is not implemented for SIMD "
                              "functions with vector-variants.");

    assert(!isOpenCLSelectMask(FnName, OrigArgIdx) &&
           "OpenCL select mask parameter is linear/uniform?");

    VPValue *Arg = VPCall->getOperand(OrigArgIdx);
    Value *ScalarArg = getScalarValue(Arg, 0);

    return ScalarArg;
  };

  AttributeList Attrs = VPCall->getOrigCallAttrs();

  unsigned NumArgOperands = VPCall->getNumArgOperands();

  for (unsigned OrigArgIdx = VPCall->isIntelIndirectCall() ? 1 : 0,
                ParamsIdx = 0;
       OrigArgIdx < NumArgOperands; OrigArgIdx++, ParamsIdx++) {
    if (isOpenCLReadChannelDest(FnName, OrigArgIdx))
      continue;

    Value *VecArg = ProcessCallArg(OrigArgIdx, ParamsIdx);
    VecArgs.push_back(VecArg);
    VecArgTys.push_back(VecArg->getType());
    VecArgAttrs.push_back(Attrs.getParamAttrs(ParamsIdx));
  }

  // Process mask parameters for current part being pumped. Masked intrinsics
  // will not have explicit mask parameter. They are handled like other BinOp
  // instructions i.e. execute on all lanes.
  bool IsMasked =
      MaskValue != nullptr && VectorIntrinID == Intrinsic::not_intrinsic;
  // NOTE: We can potentially cache the subvector extracted for MaskValue with a
  // map from {MaskValue, PumpPart, PumpFactor} to SubMaskValue. Since same mask
  // might be used for multiple calls, this would prevent dead code. However
  // this can be extended in general to all call arguments and might need a more
  // central map similar to VectorMap/ScalarMap. This cache can be implemented
  // in the future if we need very clean outgoing vector code.
  Value *PumpPartMaskValue =
      generateExtractSubVector(MaskValue, PumpPart, PumpFactor, Builder);
  StringRef VecFnName =
      TLI->getVectorizedFunction(FnName, ElementCount::getFixed(PumpedVF),
                                 IsMasked);
  if (IsMasked && !VecFnName.empty() &&
      isSVMLFunction(TLI, FnName, VecFnName)) {
    addMaskToSVMLCall(F, PumpPartMaskValue, Attrs, VecArgs, VecArgTys,
                      VecArgAttrs);
    return;
  }
  if (!VecVariant || !VecVariant->isMasked())
    return;

  assert(PumpFactor == 1 && "Pumping feature is not implemented for SIMD "
                            "functions with vector-variants.");
  Value *MaskToUse = PumpPartMaskValue
                         ? PumpPartMaskValue
                         : Constant::getAllOnesValue(FixedVectorType::get(
                               Type::getInt1Ty(F->getContext()), PumpedVF));

  createVectorMaskArg(VPCall, VecVariant, VecArgs, VecArgTys, PumpedVF,
                      MaskToUse);
}

void VPOCodeGen::createVectorMaskArg(VPCallInstruction *VPCall,
                                     const VFInfo *VecVariant,
                                     SmallVectorImpl<Value *> &VecArgs,
                                     SmallVectorImpl<Type *> &VecArgTys,
                                     unsigned PumpedVF, Value *MaskToUse) {

  // Add the mask parameter for masked simd functions.
  // Mask should already be vectorized as i1 type.
  VectorType *MaskTy = cast<VectorType>(MaskToUse->getType());
  assert(MaskTy->getElementType()->isIntegerTy(1) &&
         "Mask parameter is not vector of i1");

  // Incorrect code is generated by backend codegen when using i1 mask.
  // Therefore, the mask is promoted to the characteristic type of the
  // function, unless we're specifically told not to do so.
  if (Usei1MaskForSimdFunctions) {
    VecArgs.push_back(MaskToUse);
    VecArgTys.push_back(MaskTy);
    return;
  }

  llvm::createVectorMaskArg(
      Builder,
      VPlanCallVecDecisions::calcCharacteristicType(VPCall, *VecVariant),
      VecVariant, VecArgs, VecArgTys, PumpedVF, MaskToUse);
}

void VPOCodeGen::vectorizeSelectInstruction(VPInstruction *VPInst) {
  // If the selector is loop invariant we can create a select
  // instruction with a scalar condition. Otherwise, use vector-select.
  VPValue *Cond = VPInst->getOperand(0);
  if (!isVectorizableTy(VPInst->getOperand(1)->getType())) {
    return serializeWithPredication(VPInst);
  }
  Value *Op0 = getVectorValue(VPInst->getOperand(1));
  Value *Op1 = getVectorValue(VPInst->getOperand(2));
  bool UniformCond = Plan->getVPlanDA()->isUniform(*Cond);

  // Table to summarize special handling done for condition operand. Row heading
  // indicates DA nature of the condition, while column heading indicates the
  // incoming datatype of (condition/result) of select -
  //
  //          | scal/scal |    scal/vec      |      vec/vec     |
  // ---------|-----------|------------------|------------------|
  //    UNI   |  getScal  |      getScal     | getScal + repVec |
  //    DIV   |  getVec   | getVec + repElem |      getVec      |
  Value *VCond = nullptr;
  if (UniformCond) {
    VCond = getScalarValue(Cond, 0);
    if (Cond->getType()->isVectorTy()) {
      // For uniform vector cond variable, replicate it VF times as following
      //                        <0, 1>
      //                           |
      //                           | VF = 2,
      //                           | OriginalVL = 2
      //                           |
      //                           V
      //                      <0, 1, 0, 1>
      VCond = replicateVector(VCond, VF, Builder);
    }
  } else {
    VCond = getVectorValue(Cond);
    if (!Cond->getType()->isVectorTy() && VPInst->getType()->isVectorTy()) {
      unsigned OriginalVL =
          cast<FixedVectorType>(VPInst->getType())->getNumElements();
      // Widen the cond variable as following
      //                        <0, 1, 0, 1>
      //                             |
      //                             | VF = 4,
      //                             | OriginalVL = 2
      //                             |
      //                             V
      //                  <0, 0, 1, 1, 0, 0, 1, 1>

      VCond = replicateVectorElts(VCond, OriginalVL, Builder);
    }
  }

  Value *NewSelect = Builder.CreateSelect(VCond, Op0, Op1);

  VPWidenMap[VPInst] = NewSelect;
}

Align VPOCodeGen::getLoadStoreAlignment(const VPLoadStoreInst *LoadStore) {
  if (LoadStore->getUnderlyingValue() == nullptr)
    return LoadStore->getAlignment();

  const DataLayout &DL = OrigLoop->getHeader()->getModule()->getDataLayout();
  Type *OrigTy = LoadStore->getValueType();

  // Absence of alignment means target abi alignment. We need to use the
  // scalar's target abi alignment in such a case.
  return DL.getValueOrABITypeAlignment(LoadStore->getAlignment(), OrigTy);
}

Align VPOCodeGen::getAlignmentForGatherScatter(const VPLoadStoreInst *LoadStore) {
  Align Alignment = getLoadStoreAlignment(LoadStore);

  Type *OrigTy = LoadStore->getValueType();
  VectorType *VectorTy = dyn_cast<VectorType>(OrigTy);
  if (!VectorTy)
    return Alignment;

  const DataLayout &DL = OrigLoop->getHeader()->getModule()->getDataLayout();
  Type *EltTy = VectorTy->getElementType();
  assert(DL.getTypeAllocSizeInBits(EltTy).isKnownMultipleOf(8) &&
         "Only types with multiples of 8 bits are supported.");
  Align EltAlignment(DL.getTypeAllocSizeInBits(EltTy).getFixedSize() / 8);

  return std::min(EltAlignment, Alignment);
}

Value *VPOCodeGen::vectorizeUnitStrideLoad(VPLoadStoreInst *VPLoad,
                                           bool IsNegOneStride, bool IsPvtPtr) {
  Instruction *WideLoad = nullptr;
  VPValue *Ptr = VPLoad->getPointerOperand();
  Type *LoadType = VPLoad->getValueType();
  unsigned OriginalVL = LoadType->isVectorTy()
                            ? cast<FixedVectorType>(LoadType)->getNumElements()
                            : 1;
  Align Alignment =
      VPAA.getAlignmentUnitStride(*VPLoad, getGuaranteedPeeling());
  Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(
      Ptr, VPLoad->getValueType(), IsNegOneStride);
  Type *WidenedType = getWidenedType(LoadType, VF);

  // Masking not needed for privates.
  // TODO: This needs to be generalized for all "dereferenceable" pointers
  // identified in incoming LLVM-IR. Check CMPLRLLVM-10714.
  if (MaskValue && !IsPvtPtr) {
    // Replicate the mask if VPLoad is a vector instruction.
    Value *RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                              "replicatedMaskElts.");
    // We need to reverse the mask for -1 stride.
    if (IsNegOneStride)
      RepMaskValue = reverseVector(RepMaskValue, OriginalVL);

    ++getOptReportStats(VPLoad).MaskedUnalignedUnitStrideLoads;
    WideLoad =
        Builder.CreateMaskedLoad(WidenedType, VecPtr, Alignment, RepMaskValue,
                                 nullptr, "wide.masked.load");
  } else {
    ++getOptReportStats(VPLoad).UnmaskedUnalignedUnitStrideLoads;
    WideLoad =
        Builder.CreateAlignedLoad(WidenedType, VecPtr, Alignment, "wide.load");
  }

  // We don't need GuaranteedPeeling here. PreferredAlignmentMetadata is just a
  // preference, not a requirement.
  VPlanPeelingVariant *PreferredPeeling = Plan->getPreferredPeeling(VF);
  if (auto *DynPeeling =
          dyn_cast_or_null<VPlanDynamicPeeling>(PreferredPeeling))
    if (VPLoad == DynPeeling->memref())
      attachPreferredAlignmentMetadata(WideLoad, DynPeeling->targetAlignment());

  propagateLoadStoreInstAliasMetadata(WideLoad, VPLoad);

  if (IsNegOneStride) // Reverse
    return reverseVector(WideLoad);
  return WideLoad;
}

void VPOCodeGen::vectorizeLoadInstruction(VPLoadStoreInst *VPLoad,
                                          bool EmitIntrinsic) {
  Type *LoadType = VPLoad->getValueType();
  auto *LoadVecType = dyn_cast<FixedVectorType>(LoadType);
  assert((!LoadVecType || LoadVecType->getElementType()->isSingleValueType()) &&
         "Re-vectorization supports simple vectors only!");

  VPValue *Ptr = VPLoad->getPointerOperand();

  // Loads that are non-vectorizable should be serialized.
  if (!isVectorizableLoadStore(VPLoad)) {
    // Remark: serilalized due to operating on non-vectorizable types
    getOptReportStats(VPLoad).SerializedInstRemarks.emplace_back(15563, "");
    return serializeWithPredication(VPLoad);
  }

  // TODO: Using DA for loop invariance.
  if (Plan->getVPlanDA()->isUniform(*Ptr)) {
    if (MaskValue)
      serializePredicatedUniformInstruction(VPLoad);
    else
      serializeInstruction(VPLoad);
    return;
  }

  unsigned OriginalVL = LoadVecType ? LoadVecType->getNumElements() : 1;

  // Try to handle consecutive loads.
  bool IsNegOneStride = false;
  bool ConsecutiveStride = Plan->getVPlanDA()->isUnitStridePtr(
      Ptr, VPLoad->getValueType(), IsNegOneStride);
  if (ConsecutiveStride) {
    bool IsPvtPtr = getVPValuePrivateMemoryPtr(Ptr) != nullptr;
    VPWidenMap[VPLoad] = vectorizeUnitStrideLoad(VPLoad, IsNegOneStride, IsPvtPtr);
    return;
  }

  // Replicate the mask if VPInst is a vector instruction originally.
  Value *RepMaskValue = nullptr;
  if (MaskValue)
    RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                       "replicatedMaskElts.");
  Value *GatherAddress = getWidenedAddressForScatterGather(Ptr, LoadType);
  Align Alignment = getAlignmentForGatherScatter(VPLoad);
  ++(RepMaskValue ? getOptReportStats(VPLoad).MaskedGathers
                  : getOptReportStats(VPLoad).UnmaskedGathers);

  auto *ScalarPtrTy = cast<PointerType>(GatherAddress->getType()->getScalarType());
  if (ScalarPtrTy->isOpaque()) {
    // FIXME: Revisit when community updates the gather intrinsic for opaque
    // pointers.
    Type *ElemTy = VPLoad->getValueType()->getScalarType();
    VectorType *DesiredDataTy = FixedVectorType::get(ElemTy, VF * OriginalVL);
    GatherAddress = Builder.CreateBitCast(
        GatherAddress, DesiredDataTy->getWithNewType(
                        DesiredDataTy->getScalarType()->getPointerTo(
                            ScalarPtrTy->getAddressSpace())));
  }

  Type *VecTy =  getWidenedType(LoadType, VF);
  Instruction *NewLI =
      Builder.CreateMaskedGather(VecTy, GatherAddress, Alignment, RepMaskValue,
                                 nullptr, "wide.masked.gather");
  propagateLoadStoreInstAliasMetadata(NewLI, VPLoad);

  VPWidenMap[VPLoad] = NewLI;
}

void VPOCodeGen::vectorizeUnitStrideStore(VPLoadStoreInst *VPStore,
                                          bool IsNegOneStride, bool IsPvtPtr) {
  VPValue *Ptr = VPStore->getPointerOperand();
  Value *VecDataOp = getVectorValue(VPStore->getOperand(0));
  Type *StoreType = VPStore->getValueType();
  unsigned OriginalVL = StoreType->isVectorTy()
                            ? cast<FixedVectorType>(StoreType)->getNumElements()
                            : 1;
  Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(
      Ptr, VPStore->getValueType(), IsNegOneStride);
  Align Alignment =
      VPAA.getAlignmentUnitStride(*VPStore, getGuaranteedPeeling());

  if (IsNegOneStride) // Reverse
    // If we store to reverse consecutive memory locations, then we need
    // to reverse the order of elements in the stored value.
    VecDataOp = reverseVector(VecDataOp);

  Instruction *Store;
  if (MaskValue) {
    // Replicate the mask if VPStore is a vector instruction originally.
    Value *RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                              "replicatedMaskElts.");
    // We need to reverse the mask for -1 stride.
    if (IsNegOneStride)
      RepMaskValue = reverseVector(RepMaskValue, OriginalVL);

    ++getOptReportStats(VPStore).MaskedUnalignedUnitStrideStores;
    Store =
        Builder.CreateMaskedStore(VecDataOp, VecPtr, Alignment, RepMaskValue);
  } else {
    ++getOptReportStats(VPStore).UnmaskedUnalignedUnitStrideStores;
    Store = Builder.CreateAlignedStore(VecDataOp, VecPtr, Alignment);
  }

  const DataLayout &DL = OrigLoop->getHeader()->getModule()->getDataLayout();
  if (Alignment == DL.getTypeAllocSize(VecDataOp->getType()))
    if (auto *NtmpMD = VPStore->getMetadata(LLVMContext::MD_nontemporal))
      Store->setMetadata(LLVMContext::MD_nontemporal, NtmpMD);

  // We don't need GuaranteedPeeling here. PreferredAlignmentMetadata is just a
  // preference, not a requirement.
  VPlanPeelingVariant *PreferredPeeling = Plan->getPreferredPeeling(VF);
  if (auto *DynPeeling =
          dyn_cast_or_null<VPlanDynamicPeeling>(PreferredPeeling))
    if (VPStore == DynPeeling->memref())
      attachPreferredAlignmentMetadata(Store, DynPeeling->targetAlignment());

  propagateLoadStoreInstAliasMetadata(Store, VPStore);
}

void VPOCodeGen::vectorizeStoreInstruction(VPLoadStoreInst *VPStore,
                                           bool EmitIntrinsic) {
  Type *StoreType = VPStore->getValueType();
  auto *StoreVecType = dyn_cast<FixedVectorType>(StoreType);
  assert(
      (!StoreVecType || StoreVecType->getElementType()->isSingleValueType()) &&
      "Re-vectorization supports simple vectors only!");

  VPValue *Ptr = VPStore->getPointerOperand();

  // Stores that are non-vectorizable should be serialized.
  if (!isVectorizableLoadStore(VPStore)) {
    // Remark: serilalized due to operating on non-vectorizable types
    getOptReportStats(VPStore).SerializedInstRemarks.emplace_back(15563, "");
    return serializeWithPredication(VPStore);
  }

  // Stores to uniform pointers can be optimally generated as a scalar store in
  // vectorized code.
  // TODO: Extend the optimization for masked uniform stores too. Will need
  // all-zero check (like masked uniform load) and functionality to find out
  // last unmasked lane for divergent data operand. Update SVA after
  // implementing this optimization.
  if (Plan->getVPlanDA()->isUniform(*Ptr) && !MaskValue) {
    Value *ScalarPtr = getScalarValue(Ptr, 0);
    VPValue *DataOp = VPStore->getOperand(0);
    Align Alignment = getLoadStoreAlignment(VPStore);
    // Extract last lane of data operand to generate scalar store. For uniform
    // data operand, the same value is present on all lanes.
    auto *Inst = Builder.CreateAlignedStore(getScalarValue(DataOp, VF - 1),
                                            ScalarPtr, Alignment);
    propagateLoadStoreInstAliasMetadata(Inst, VPStore);
    return;
  }

  unsigned OriginalVL = StoreVecType ? StoreVecType->getNumElements() : 1;
  Value *VecDataOp = getVectorValue(VPStore->getOperand(0));

  // Try to handle consecutive stores.
  bool IsNegOneStride = false;
  bool ConsecutiveStride = Plan->getVPlanDA()->isUnitStridePtr(
      Ptr, VPStore->getValueType(), IsNegOneStride);
  if (ConsecutiveStride) {
    // TODO: VPVALCG: Special handling for mask value is also needed for
    // conditional last privates.
    bool IsPvtPtr = getVPValuePrivateMemoryPtr(Ptr) != nullptr;
    vectorizeUnitStrideStore(VPStore, IsNegOneStride, IsPvtPtr);
    return;
  }

  Value *ScatterPtr = getWidenedAddressForScatterGather(Ptr, StoreType);
  Type *ElemTy = VPStore->getValueType()->getScalarType();
  VectorType *DesiredDataTy = FixedVectorType::get(ElemTy, VF * OriginalVL);
  // TODO: Verify if this bitcast should be done this late. Maybe an earlier
  // transform can introduce it, if needed.
  VecDataOp = Builder.CreateBitCast(VecDataOp, DesiredDataTy, "cast");

  // Replicate the mask if VPStore is a vector instruction originally.
  Value *RepMaskValue = nullptr;
  if (MaskValue)
    RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                       "replicatedMaskElts.");
  Align Alignment = getAlignmentForGatherScatter(VPStore);
  ++(RepMaskValue ? getOptReportStats(VPStore).MaskedScatters
                  : getOptReportStats(VPStore).UnmaskedScatters);

  auto *ScalarPtrTy = cast<PointerType>(ScatterPtr->getType()->getScalarType());
  if (ScalarPtrTy->isOpaque()) {
    // FIXME: Revisit when community updates the scatter intrinsic for opaque
    // pointers.
    ScatterPtr = Builder.CreateBitCast(
        ScatterPtr, DesiredDataTy->getWithNewType(
                        DesiredDataTy->getScalarType()->getPointerTo(
                            ScalarPtrTy->getAddressSpace())));
  }

  auto *Inst = Builder.CreateMaskedScatter(VecDataOp, ScatterPtr, Alignment,
                                           RepMaskValue);
  propagateLoadStoreInstAliasMetadata(Inst, VPStore);
}

// This function returns computed addresses of memory locations which should be
// accessed in the vectorized code. These addresses, take the form of a GEP
// instruction, and this GEP is used as pointer operand of the resulting
// scatter/gather intrinsic.
Value *VPOCodeGen::getWidenedAddressForScatterGather(VPValue *VPBasePtr,
                                                     Type *ScalarAccessType) {
  assert(VPBasePtr->getType()->isPointerTy() &&
         "Expect 'VPBasePtr' to be a PointerType");

  // Vectorize BasePtr.
  Value *BasePtr = getVectorValue(VPBasePtr);

  // No replication is needed for non-vector types.
  if (!ScalarAccessType->isVectorTy())
    return BasePtr;

  unsigned AddrSpace =
      cast<PointerType>(VPBasePtr->getType())->getAddressSpace();

  // Cast the inner vector-type to it's elemental scalar type.
  // e.g. - <VF x <OriginalVL x Ty> addrspace(x)*>
  //                          |
  //                          |
  //                          V
  //                <VF x Ty addrspace(x)*>
  auto *VecType = cast<FixedVectorType>(ScalarAccessType);
  Value *TypeCastBasePtr = Builder.CreateBitCast(
      BasePtr, FixedVectorType::get(
                   VecType->getElementType()->getPointerTo(AddrSpace), VF));
  // Replicate the base-address OriginalVL times
  //                <VF x Ty addrspace(x)*>
  //                          |
  //                          |
  //                          V
  //      < 0, 1, .., OriginalVL-1, ..., 0, 1, ..., OriginalVL-1>
  unsigned OriginalVL = VecType->getNumElements();
  Value *VecBasePtr =
      replicateVectorElts(TypeCastBasePtr, OriginalVL, Builder, "vecBasePtr.");

  // Create a vector of consecutive numbers from zero to OriginalVL-1 repeated
  // VF-times.
  SmallVector<Constant *, 32> Indices;
  for (unsigned J = 0; J < VF; ++J)
    for (unsigned I = 0; I < OriginalVL; ++I) {
      Indices.push_back(
          ConstantInt::get(Type::getInt64Ty(VecType->getContext()), I));
    }

  // Add the consecutive indices to the vector value.
  Constant *Cv = ConstantVector::get(Indices);

  // Create a GEP that would return the address of each elements that is to be
  // accessed.
  Value *WidenedVectorGEP = Builder.CreateGEP(VecType->getElementType(),
                                              VecBasePtr, Cv, "elemBasePtr.");
  return WidenedVectorGEP;
}

// This function return an appropriate BasePtr for cases where we are dealing
// with load/store to consecutive memory locations
Value *VPOCodeGen::createWidenedBasePtrConsecutiveLoadStore(VPValue *Ptr,
                                                            Type *ScalarAccessType,
                                                            bool Reverse) {
  unsigned AddrSpace = Ptr->getType()->getPointerAddressSpace();
  auto *WideDataTy =
      cast<FixedVectorType>(getWidenedType(ScalarAccessType, VF));
  Value *VecPtr = nullptr;

  if (isa<VPAllocatePrivate>(Ptr))
    // For pointers that are private to the loop (privates, memory
    // reductions/inductions), we can use the base pointer to the widened
    // alloca. This would be more optimal than extracting 0th lane address from
    // vector of pointers (done by getScalarValue below).
    // TODO: Extend support for unit-stride loads/stores coming from casts on
    // loop private pointers. For example -
    // %1 = bitcast i32* %priv to i8*
    // %2 = load i8* %1
    // Wide unit-stride load can be optimally implemented for this example by
    // directly using bitcast to vector type instead of extract + bitcast
    // sequence.
    VecPtr = LoopPrivateVPWidenMap[Ptr];
  else
    // We do not care whether the 'Ptr' operand comes from a GEP or any other
    // source. We just fetch the first element and then create a
    // bitcast  which assumes the 'consecutive-ness' property and return the
    // correct operand for widened load/store.
    VecPtr = getScalarValue(Ptr, 0);

  VecPtr = Reverse ? Builder.CreateGEP(
                         WideDataTy->getScalarType(), VecPtr,
                         Builder.getInt32(1 - WideDataTy->getNumElements()))
                   : VecPtr;
  VecPtr = Builder.CreateBitCast(VecPtr, WideDataTy->getPointerTo(AddrSpace));
  return VecPtr;
}

// Return the right vector mask for a OpenCL vector select built-in.
//
// Definition of OpenCL select intrinsic:
//   gentype select ( gentype a, gentype b, igentype c)
//
//   For each component of a vector type, result[i] = if MSB of c[i] is set ?
//   b[i] : a[i] For scalar type, result = c ? b : a.
//
// Scalar select built-in uses integer mask (integer != 0 means true). However,
// vector select built-in uses the MSB of each vector element.
//
// Returned vector mask depends on ScalarMask as follows:
//   1) if ScalarMask == ZExt(i1), return widened SExt.
//   2) if ScalarMask == SExt(i1), return widened SExt.
//   3) Otherwise, return SExt(VectorMask != 0).
//
Value *VPOCodeGen::getOpenCLSelectVectorMask(VPValue *ScalarMask) {

  Type *ScTy = ScalarMask->getType();
  assert(!ScTy->isVectorTy() && ScTy->isIntegerTy() &&
         "Scalar integer type expected.");
  Type *VecTy = getWidenedType(ScTy, VF);

  // Special cases for i1 type.
  VPInstruction *VPInst = dyn_cast<VPInstruction>(ScalarMask);
  if (VPInst && VPInst->isCast() &&
      VPInst->getOperand(0)->getType()->isIntegerTy(1 /*i1*/)) {
    if (VPInst->getOpcode() == Instruction::SExt) {
      // SExt mask doesn't need to be fixed.
      return getVectorValue(ScalarMask);
    } else if (VPInst->getOpcode() == Instruction::ZExt) {
      // ZExt is replaced by an SExt.
      Value *Val = getVectorValue(VPInst->getOperand(0));
      return Builder.CreateSExt(Val, VecTy);
    }
  }

  // General case. We generate a SExt(VectorMask != 0).
  // TODO: Look at Volcano vectorizer, file OCLBuiltinPreVectorizationPass.cpp.
  // It is doing something different, creating a fake call to a built-in
  // intrinsic. I don't know if that approach is applicable here at this point.
  Value *VectorMask = getVectorValue(ScalarMask);
  Constant *Zero = Constant::getNullValue(VecTy);

  // Only integer mask is supported.
  Value *Cmp = Builder.CreateICmpNE(VectorMask, Zero);
  return Builder.CreateSExt(Cmp, VecTy);
}

void VPOCodeGen::generateStoreForSinCos(VPCallInstruction *VPCall,
                                        Value *CallResult) {
  // Extract results in the structure returned from SVML sincos call and store
  // into the pointers provided by the scalar call, for example:
  // %sincos = call svml_cc { <16 x float>, <16 x float> } @__svml_sincosf16(
  //           <16 x float> %src)
  // %sincos.sin = extractvalue { <16 x float>, <16 x float> } %sincos, 0
  // %sincos.cos = extractvalue { <16 x float>, <16 x float> } %sincos, 1
  // %sin.vector.ptr = bitcast float* %sin.ptr to <16 x float>*
  // store <16 x float> %sincos.sin, <16 x float>* %sin.vector.ptr, align 4
  // %cos.vector.ptr = bitcast float* %cos.ptr to <16 x float>*
  // store <16 x float> %sincos.cos, <16 x float>* %cos.vector.ptr, align 4
  //
  // Note we may generate other instructions such as masked store, shuffle +
  // store, scatter for storing the results depending on the optimizations
  // applied.

  auto *ExtractSinInst =
      Builder.CreateExtractValue(CallResult, {0}, "sincos.sin");
  auto *ExtractCosInst =
      Builder.CreateExtractValue(CallResult, {1}, "sincos.cos");

  const DataLayout &DL = *Plan->getDataLayout();
  Align Alignment = Align(DL.getABITypeAlignment(
      cast<VectorType>(ExtractSinInst->getType())->getElementType()));

  // Widen ScalarPtr and generate instructions to store VecValue into it.
  auto storeVectorValue = [this, VPCall](Value *VecValue, VPValue *ScalarPtr,
                                         Align Alignment) {
    assert(cast<FixedVectorType>(VecValue->getType())->getNumElements() == VF &&
           "Invalid vector width of value");

    bool IsNegOneStride = false;
    Type *OpTy = VPCall->getOperand(0)->getType();
    bool ConsecutiveStride =
        Plan->getVPlanDA()->isUnitStridePtr(ScalarPtr, OpTy, IsNegOneStride);

    // TODO: Currently only address with stride = 1 can be optimized. Need to
    // handle other cases.
    if (ConsecutiveStride && !IsNegOneStride) {
      Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(
          ScalarPtr, OpTy, false /*Reverse*/);
      if (MaskValue)
        Builder.CreateMaskedStore(VecValue, VecPtr, Alignment, MaskValue);
      else
        Builder.CreateAlignedStore(VecValue, VecPtr, Alignment);
    } else {
      Value *VectorPtr = getWidenedAddressForScatterGather(ScalarPtr, OpTy);
      VectorType *DesiredDataTy = FixedVectorType::get(OpTy, VF);
      VecValue = Builder.CreateBitCast(VecValue, DesiredDataTy, "cast");

      Builder.CreateMaskedScatter(VecValue, VectorPtr, Alignment, MaskValue);
    }
  };

  storeVectorValue(ExtractSinInst, VPCall->getOperand(1), Alignment);
  storeVectorValue(ExtractCosInst, VPCall->getOperand(2), Alignment);
}

void VPOCodeGen::generateVectorCalls(VPCallInstruction *VPCall,
                                     unsigned PumpFactor, bool IsMasked,
                                     const VFInfo *MatchedVariant,
                                     Intrinsic::ID VectorIntrinID,
                                     SmallVectorImpl<Value *> &CallResults) {
  Function *CalledFunc = VPCall->getCalledFunction();
  assert(CalledFunc && "Unexpected null called function.");

  LLVM_DEBUG(dbgs() << "Function " << CalledFunc->getName() << " is pumped "
                    << PumpFactor << "-way.\n");
  for (unsigned PumpPart = 0; PumpPart < PumpFactor; ++PumpPart) {
    LLVM_DEBUG(dbgs() << "Pumping part " << PumpPart << "/" << PumpFactor
                      << "\n");
    SmallVector<Value *, 2> VecArgs;
    SmallVector<Type *, 2> VecArgTys;
    SmallVector<AttributeSet, 2> VecArgAttrs;

    vectorizeCallArgs(VPCall, MatchedVariant, VectorIntrinID, PumpPart,
                      PumpFactor, VecArgs, VecArgTys, VecArgAttrs);

    Function *VectorF = nullptr;
    if (MatchedVariant) {
      VectorF = getOrInsertVectorVariantFunction(
            CalledFunc, VF / PumpFactor, VecArgTys, MatchedVariant, IsMasked);
    } else {
      VectorF = getOrInsertVectorLibFunction(
        CalledFunc, VF / PumpFactor, VecArgTys, TLI, VectorIntrinID, IsMasked);
    }
    assert(VectorF && "Can't create vector function.");
    CallInst *VecCall = Builder.CreateCall(VectorF, VecArgs);
    VecCall->setCallingConv(VectorF->getCallingConv());
    if (VPCall->getType()->isIntegerTy(1)) {
      // Trunc the result back to i1 after it has been promoted to i8,
      // in order not to conflict with ret type users.
      Value *Trunc = Builder.CreateTrunc(
        VecCall,
        getWidenedType(Type::getInt1Ty(Builder.getContext()), VF / PumpFactor),
        VecCall->getName() + ".trunc");
      CallResults.push_back(Trunc);
    } else
      CallResults.push_back(VecCall);

    // Copy fast math flags represented in VPInstruction.
    // TODO: investigate why attempting to copy fast math flags for __read_pipe
    // fails. For now, just don't do the copy.
    if (isa<FPMathOperator>(VecCall) &&
        !isOpenCLReadChannel(CalledFunc->getName())) {
      VPCall->copyOperatorFlagsTo(VecCall);
    }

    // Make sure we don't lose attributes at the call site. E.g., IMF
    // attributes are taken from call sites in MapIntrinToIml to refine
    // SVML calls for precision.
    setRequiredAttributes(VPCall->getOrigCallAttrs(), VecCall, VecArgAttrs);

#if 0
    // TODO: Need a VPValue based analysis for call arg memory references.
    // VPValue-based stride info also needed.
    Loop *Lp = LI->getLoopFor(Call->getParent());
    analyzeCallArgMemoryReferences(Call, VecCall, TLI, PSE.getSE(), Lp);
#endif

    // No blending is required here for masked simd function calls as of now for
    // two reasons:
    //
    // 1) A select is already generated for call results that are live outside
    //    of the predicated region by using the predicated region's mask. See
    //    vectorizeVPPHINode().
    //
    // 2) Currently, masked stores are always generated for call results stored
    //    to memory within a predicated region. See vectorizeStoreInstruction().

    if (isOpenCLReadChannel(CalledFunc->getName())) {
      llvm_unreachable(
          "VPVALCG: OpenCL read channel vectorization not uplifted.");
#if 0
      vectorizeOpenCLReadChannelDest(Call, VecCall, Call->getArgOperand(1));
#endif
    }
  }
}

Value *VPOCodeGen::getCombinedCallResults(ArrayRef<Value *> CallResults) {
  if (CallResults.size() == 1)
    return CallResults[0];

  // Combine the pumped call results to be used in original VF context.
  assert(CallResults.size() >= 2 && isPowerOf2_32(CallResults.size()) &&
         "Number of pumped vector calls to combine must be a power of 2 "
         "(atleast 2^1)");
  Type *ReturnType = CallResults[0]->getType();
  if (ReturnType->isVectorTy()) {
    return joinVectors(CallResults, Builder, "combined");
  } else {
    llvm_unreachable("Expect vector result from vector calls");
  }
}

void VPOCodeGen::vectorizeLibraryCall(VPCallInstruction *VPCall) {
  assert(VPCall->getVectorizationScenario() ==
             VPCallInstruction::CallVecScenariosTy::LibraryFunc &&
         "vectorizeLibraryCall called for mismatched scenario.");
  Function *CalledFunc = VPCall->getCalledFunction();
  assert(CalledFunc && "Unexpected null called function.");
  unsigned PumpFactor = VPCall->getPumpFactor();
  bool IsMasked = MaskValue != nullptr;

  // Special handling for OpenCL SinCos.
  if (isOpenCLSinCos(CalledFunc->getName())) {
    assert(PumpFactor == 1 &&
           "Pumping feature is not supported for OpenCL sincos.");
    vectorizeOpenCLSinCos(VPCall, IsMasked);
    return;
  }

  // Generate vector calls using vector library function.
  SmallVector<Value *, 4> CallResults;
  generateVectorCalls(
      VPCall, PumpFactor, IsMasked, nullptr /*No vector variant*/,
      Intrinsic::not_intrinsic /*No vector intrinsic*/, CallResults);

  // Set calling convention for SVML function calls.
  for (auto *Result : CallResults) {
    CallInst *VecCall = cast<CallInst>(Result);
    if (isSVMLFunction(TLI, CalledFunc->getName(),
                       VecCall->getCalledFunction()->getName())) {
      VecCall->setCallingConv(CallingConv::SVML);
    }
  }

  // Post process generated vector calls.
  Type *ReturnType = CallResults[0]->getType();
  if (PumpFactor > 1 && ReturnType->isStructTy()) {
    // If the return type is not a vector, then it must be a struct of vectors
    // (returned by sincos function). Create a widened struct type by widening
    // every element type.
    // For example, if CallResults[0] is { <2 x float>, <2 x float> } and
    // PumpFactor is 2, the combined type will be
    // { <4 x float>, <4 x float> }.
    StructType *ReturnStructType = cast<StructType>(ReturnType);
    SmallVector<Type *, 2> ElementTypes;
    for (unsigned I = 0; I < ReturnStructType->getStructNumElements(); I++) {
      VectorType *ElementType =
          cast<VectorType>(ReturnStructType->getStructElementType(I));
      ElementTypes.push_back(
          VectorType::get(ElementType->getElementType(),
                          ElementType->getElementCount() * CallResults.size()));
    }
    StructType *CombinedReturnType =
        StructType::get(ReturnType->getContext(), ElementTypes);

    // Then combine the pumped call results: for each vector element of
    // struct, extract the result from the return value of pumped calls,
    // combine them and insert to the combined struct:
    //
    // ; extract and combine sin field of the return values
    // %sin0 = extractvalue { <2 x float>, <2 x float> } %callResults0, 0
    // %sin1 = extractvalue { <2 x float>, <2 x float> } %callResults1, 0
    // %sin.combined = shufflevector <2 x float> %sin0, <2 x float> %sin1,
    //                               <4 x i32> <i32 0, i32 1, i32 2, i32 3>
    // %result.tmp = insertvalue { <4 x float>, <4 x float> } undef,
    //                           <4 x float> %sin.combined, 0
    //
    // ; extract and combine cos field of the return values
    // %cos0 = extractvalue { <2 x float>, <2 x float> } %callResults0, 1
    // %cos1 = extractvalue { <2 x float>, <2 x float> } %callResults1, 1
    // %cos.combined = shufflevector <2 x float> %cos0, <2 x float> %cos1,
    //                               <4 x i32> <i32 0, i32 1, i32 2, i32 3>
    // %result = insertvalue { <4 x float>, <4 x float> } %result.tmp,
    //                       <4 x float> %cos.combined, 1
    Value *Combined = UndefValue::get(CombinedReturnType);
    for (unsigned I = 0; I < CombinedReturnType->getNumElements(); I++) {
      SmallVector<Value *, 4> Parts;
      for (unsigned J = 0; J < CallResults.size(); J++)
        Parts.push_back(
            Builder.CreateExtractValue(CallResults[J], I, "extract.result"));

      Value *CombinedVector = joinVectors(Parts, Builder, "combined");
      Combined = Builder.CreateInsertValue(Combined, CombinedVector, I,
                                           "insert.result");
    }
    VPWidenMap[VPCall] = Combined;
  } else {
    VPWidenMap[VPCall] = getCombinedCallResults(CallResults);
  }
}

void VPOCodeGen::vectorizeTrivialIntrinsic(VPCallInstruction *VPCall) {
  assert(VPCall->getVectorizationScenario() ==
             VPCallInstruction::CallVecScenariosTy::TrivialVectorIntrinsic &&
         "vectorizeTrivialIntrinsic called for mismatched scenario.");
  unsigned PumpFactor = VPCall->getPumpFactor();
  assert(PumpFactor == 1 &&
         "Pumping feature is not expected for trivial vector intrinsics.");
  bool IsMasked = MaskValue != nullptr;
  Intrinsic::ID VectorIntrinID = VPCall->getVectorIntrinsic();
  assert(VectorIntrinID != Intrinsic::not_intrinsic &&
         "Unexpected non-intrinsic call.");

  // Generate vector call using vector intrinsic.
  SmallVector<Value *, 4> CallResults;
  generateVectorCalls(VPCall, PumpFactor, IsMasked,
                      nullptr /*No vector variant*/, VectorIntrinID,
                      CallResults);

  // Post process generated vector calls.
  VPWidenMap[VPCall] = getCombinedCallResults(CallResults);
}

void VPOCodeGen::vectorizeVecVariant(VPCallInstruction *VPCall) {
  assert(VPCall->getVectorizationScenario() ==
             VPCallInstruction::CallVecScenariosTy::VectorVariant &&
         "vectorizeVecVariant called for mismatched scenario.");
  if (VPCall->isIntelIndirectCall()) {
    IndirectCallCodeGenerator IndirectCall(this, LI, VF, State, MaskValue, Plan);
    IndirectCall.vectorize(VPCall);
    return;
  }
  unsigned PumpFactor = VPCall->getPumpFactor();
  assert(PumpFactor == 1 && "Pumping feature is not supported for SIMD "
                            "functions with vector variants.");
  bool IsMasked = MaskValue != nullptr;
  const VFInfo *MatchedVariant = VPCall->getVectorVariant();
  assert(MatchedVariant && "Unexpected null matched vector variant");

  // TLI is not used to check for SIMD functions for two reasons:
  // 1) A more sophisticated interface is needed to determine the most
  //    appropriate match.
  // 2) A SIMD function is not a library function.
  if (VPCall->shouldUseMaskedVariantForUnmasked()) {
    // If non-masked version isn't available, try running the masked version
    // with all-ones mask.
    IsMasked = true;
  }
  LLVM_DEBUG(dbgs() << "Matched Variant: " << MatchedVariant->VectorName << "\n");

  // Generate vector calls using matched vector variant.
  SmallVector<Value *, 4> CallResults;
  generateVectorCalls(VPCall, PumpFactor, IsMasked, MatchedVariant,
                      Intrinsic::not_intrinsic /*No vector intrinsic*/,
                      CallResults);

  // Post process generated vector calls.
  VPWidenMap[VPCall] = getCombinedCallResults(CallResults);
}

// Widen or Serialize lifetime_start/end intrinsic call.
void VPOCodeGen::vectorizeLifetimeStartEndIntrinsic(VPCallInstruction *VPCall) {
  // If this is a private, determine if the call can be widened with the widened
  // pointer, else serialie.
  if (VPValue *PrivPtr = const_cast<VPValue *>(
          getVPValuePrivateMemoryPtr(VPCall->getOperand(1)))) {
    if (LoopPrivateVPWidenMap.count(PrivPtr)) {
      Value *WidePriv = LoopPrivateVPWidenMap[PrivPtr];
      AllocaInst *AI = dyn_cast<AllocaInst>(WidePriv);
      if (!AI) {
        assert(isa<AddrSpaceCastInst>(WidePriv) &&
               "Expected alloca or addrspacecast instruction.");
        AI = cast<AllocaInst>(
            cast<AddrSpaceCastInst>(WidePriv)->getPointerOperand());
      }
      ConstantInt *Size = Builder.getInt64(-1);
      if (!cast<VPConstantInt>(VPCall->getOperand(0))->isMinusOne()) {
        const DataLayout &DL =
            OrigLoop->getHeader()->getModule()->getDataLayout();
        Size =
            Builder.getInt64(AI->getAllocationSizeInBits(DL).getValue() >> 3);
      }
      // If the pointer argument is not i8* type for this function, insert a
      // bitcast to convert it to i8*. This inserts duplicate bitcasts, but, we
      // expect CSE following up to take care of this.
      Value *PointerArg = getScalarValue(VPCall->getOperand(1), 0);
      auto *PointerArgType = cast<PointerType>(PointerArg->getType());
      if (!PointerArgType->isOpaque() &&
          !PointerArgType->getElementType()->isIntegerTy(8))
        PointerArg = Builder.CreateBitCast(
            PointerArg, Type::getInt8PtrTy(*Plan->getLLVMContext()));

      SmallVector<Value *, 3> ScalarArgs = {
          Size, PointerArg, getScalarValue(VPCall->getOperand(2), 0)};
      auto *ScalarInstrinsic = generateSerialInstruction(VPCall, ScalarArgs);
      VPScalarMap[VPCall][0] = ScalarInstrinsic;
      return;
    }
  }

  // This call is either not operating on privates, or is not vectorizable. So,
  // serialize.
  serializeWithPredication(VPCall);
}

Value *VPOCodeGen::getVectorValueForExternal(VPValue *V, unsigned CurVF) {
  assert((isa<VPExternalDef>(V) || isa<VPConstant>(V) ||
          isa<VPMetadataAsValue>(V)) &&
         "Unknown external VPValue.");
  assert(Plan->getVPlanDA()->isUniform(*V) && "External value is not uniform.");
  Value *UnderlyingV = getScalarValue(V, 0 /*Lane*/);
  assert(UnderlyingV &&
         "External VPValues are expected to have underlying IR value set.");

  Value *Widened;
  // Broadcast V and save the value for future uses.
  if (auto *ValVecTy = dyn_cast<VectorType>(V->getType())) {
    assert(ValVecTy->getElementType()->isSingleValueType() &&
           "Re-vectorization is supported for simple vectors only");
    (void)ValVecTy;
    // Widen the uniform vector variable as following
    //                        <i32 0, i32 1>
    //                             |
    //                             |VF = 4
    //                             |
    //                             V
    //          <i32 0, i32 1,i32 0, i32 1,i32 0, i32 1,i32 0, i32 1>
    Widened = replicateVector(UnderlyingV, CurVF, Builder,
                              "replicatedVal." + UnderlyingV->getName());
  } else {
    Widened = Builder.CreateVectorSplat(CurVF, UnderlyingV, "broadcast");
  }
  return Widened;
}

Value *VPOCodeGen::getVectorValue(VPValue *V) {
  // If we have this scalar in the map, return it.
  if (VPWidenMap.count(V))
    return VPWidenMap[V];

  // TODO: Probable improvement is to "inline" liveouts after CFG merge. Then we
  // don't need this code. Same for getScalarValue.
  if (auto LiveOut = dyn_cast<VPLiveOutValue>(V))
    return getVectorValue(LiveOut->getOperand(0));

  auto getInsertPointPH = [this]() -> Instruction * {
    // Get insertion point for broadcasting externals.
    // First try to get the block when we encounter the last VPPushVF.
    BasicBlock *B = nullptr;
    if (LastPushVF)
      B = State->CFG
              .VPBB2IRBB[const_cast<VPBasicBlock *>(LastPushVF->getParent())];
    if (!B) {
      // If it is not defined then try first executable basic block.
      auto LoopPH = State->CFG.FirstExecutableVPBB;
      B = State->CFG.VPBB2IRBB[const_cast<VPBasicBlock *>(LoopPH)];
    }
    return B->getTerminator();
  };

  // If the VPValue has not been vectorized, check if it has been scalarized
  // instead. If it has been scalarized, and we actually need the value in
  // vector form, we will construct the vector values on demand.
  if (VPScalarMap.count(V)) {
    bool IsUniform = Plan->getVPlanDA()->isUniform(*V);

    Value *VectorValue = nullptr;
    IRBuilder<>::InsertPointGuard Guard(Builder);

    auto UpdateInsertPoint = [=](Value *ScalarValue) -> void {
      // ScalarValue can be a constant, so insertion point setting is not needed
      // for that case.
      auto *ScalarInst = dyn_cast<Instruction>(ScalarValue);
      if (!ScalarInst) {
        Builder.SetInsertPoint(getInsertPointPH());
        return;
      }

      // If the scalar value is the result of an invoke instruction which should
      // be the last instruction in the block, set the insertion point to PH.
      if (isa<InvokeInst>(ScalarInst)) {
        Builder.SetInsertPoint(getInsertPointPH());
        return;
      }

      auto It = ++(ScalarInst->getIterator());

      while (isa<PHINode>(*It))
        ++It;

      Builder.SetInsertPoint(ScalarInst->getParent(), It);
    };

    // TODO: Temporarily allow bcast of strictly scalar non-uniform instruction
    // as it can be used in other instructions whose opcodes haven't been
    // uplifted to be scalarized via SVA. For example -
    // [DA: Div, SVA: (F  )] %gep1 = getelementptr %src i64 0 i64 %iv
    // [DA: Div, SVA: (F  )] %gep2 = getelementptr %src i64 1 i64 %iv
    // [DA: Div, SVA: (F  )] %ptr = select %cond i32* %gep1 i32* %gep2
    // [DA: Div, SVA: ( V )] %unit.stride.load = load i32* %ptr
    //
    // If "select" opcode has not been uplifted to be scalarized via SVA, then
    // bcast of scalarized GEPs is still needed for stability of CG. This will
    // be dropped when all opcodes have been uplifted to use SVA for
    // scalarization/vectorization decisions along with simplification of
    // get{Vector|Scalar}Value interfaces.
    auto *VInst = dyn_cast<VPInstruction>(V);
    auto *SVA = Plan->getVPlanSVA();
    bool InstCGIsSVADriven = VInst && isOpcodeCGSVADriven(VInst);
    bool NeedsFirstLaneBcastForNonSVADrivenCG =
        InstCGIsSVADriven && SVA->instNeedsFirstScalarCode(VInst) &&
        !SVA->instNeedsLastScalarCode(VInst) &&
        !SVA->instNeedsVectorCode(VInst);
    bool NeedsLastLaneBcastForNonSVADrivenCG =
        InstCGIsSVADriven && SVA->instNeedsLastScalarCode(VInst) &&
        !SVA->instNeedsFirstScalarCode(VInst) &&
        !SVA->instNeedsVectorCode(VInst);
    assert(!VInst || !requiresUnsupportedSVAFeatures(VInst, Plan) &&
           "Bcast of (F L) sequence of SVA bits is not supported.");
    if (IsUniform || NeedsFirstLaneBcastForNonSVADrivenCG ||
        NeedsLastLaneBcastForNonSVADrivenCG) {
      unsigned Lane = NeedsLastLaneBcastForNonSVADrivenCG ? VF - 1 : 0;
      Value *ScalarValue = VPScalarMap[V][Lane];
      UpdateInsertPoint(ScalarValue);
      if (ScalarValue->getType()->isVectorTy()) {
        VectorValue =
            replicateVector(ScalarValue, VF, Builder,
                            "replicatedVal." + ScalarValue->getName());
      } else
        VectorValue =
            Builder.CreateVectorSplat(VF, ScalarValue, "broadcast");

      VPWidenMap[V] = VectorValue;
      return VectorValue;
    }

    if (V->getType()->isVectorTy()) {
      SmallVector<Value *, 8> Parts;
      for (unsigned Lane = 0; Lane < VF; ++Lane)
        Parts.push_back(VPScalarMap[V][Lane]);
      Value *ScalarValue = VPScalarMap[V][VF - 1];
      assert(isa<Instruction>(ScalarValue) &&
             "Expected instruction for scalar value");
      UpdateInsertPoint(ScalarValue);
      VectorValue = joinVectors(Parts, Builder);
    } else {
      VectorValue = UndefValue::get(FixedVectorType::get(V->getType(), VF));
      for (unsigned Lane = 0; Lane < VF; ++Lane) {
        Value *ScalarValue = VPScalarMap[V][Lane];
        assert(isa<Instruction>(ScalarValue) &&
               "Expected instruction for scalar value");
        Builder.SetInsertPoint((cast<Instruction>(ScalarValue))->getNextNode());
        VectorValue = Builder.CreateInsertElement(VectorValue, ScalarValue,
                                                  Builder.getInt32(Lane));
      }
    }

    VPWidenMap[V] = VectorValue;
    return VectorValue;
  }

  // VPInstructions should already be handled, only external values are expected
  // here.
  // Keep the VPValue for dropping when we switch VF.
  VPValsToFlushForVF.insert(V);

  // Place the code for broadcasting invariant variables in the new preheader.
  IRBuilder<>::InsertPointGuard Guard(Builder);
  Builder.SetInsertPoint(getInsertPointPH());

  Value *Widened = getVectorValueForExternal(V, VF);
  VPWidenMap[V] = Widened;

  return Widened;
}

Value *VPOCodeGen::getScalarValue(VPValue *V, unsigned Lane) {
  if (isa<VPExternalDef>(V) || isa<VPConstant>(V) || isa<VPMetadataAsValue>(V))
    return V->getUnderlyingValue();

  if (auto LiveOut = dyn_cast<VPLiveOutValue>(V)) {
    assert(Lane == 0 && "unexpected lane number to get scalar value");
    return getScalarValue(LiveOut->getOperand(0), Lane);
  }

  if (VPScalarMap.count(V)) {
    auto SV = VPScalarMap[V];
    if (Plan->getVPlanDA()->isUniform(*V))
      // For uniform instructions the mapping is updated for lane zero only.
      Lane = 0;

    if (SV.count(Lane))
      return SV[Lane];
  }

  if (isa<VPBasicBlock>(V)) {
    BasicBlock *InsertBefore = State->CFG.InsertBefore;
    StringRef Name = V->getName();
    if (VPBasicBlock::isDefaultName(Name))
      Name = "VPlannedBB";
    BasicBlock *NewBB =
        BasicBlock::Create(InsertBefore->getContext(), Name,
                           InsertBefore->getParent(), InsertBefore);
    LLVM_DEBUG(dbgs() << "LV: created " << NewBB->getName() << '\n');
    VPScalarMap[V][0] = NewBB;
    return NewBB;
  }

#if 0
  // TODO: This will be handled by reduction/induction cleanup patch.
  if (Legal->isInductionVariable(V))
    return buildScalarIVForLane(cast<PHINode>(V), Lane);
#endif

  // Get the scalar value by extracting from the vector instruction based on the
  // requested lane.
  Value *VecV = getVectorValue(V);
  IRBuilder<>::InsertPointGuard Guard(Builder);
  if (auto VecInst = dyn_cast<Instruction>(VecV)) {
    if (isa<PHINode>(VecInst))
      Builder.SetInsertPoint(&*(VecInst->getParent()->getFirstInsertionPt()));
    else
      Builder.SetInsertPoint(VecInst->getNextNode());
  }

  // This code assumes that the widened vector, that we are extracting from has
  // data in AOS layout. If OriginalVL = 2, VF = 4 the widened value would be
  // Wide.Val = <v1_0, v2_0, v1_1, v2_1, v1_2, v2_2, v1_3, v2_3>.
  // getScalarValue(Wide.Val, 1) would return <v1_1, v2_1>
  if (V->getType()->isVectorTy()) {
    unsigned OrigNumElts = cast<FixedVectorType>(V->getType())->getNumElements();
    SmallVector<int, 8> ShufMask;
    for (unsigned StartIdx = Lane * OrigNumElts,
                  EndIdx = (Lane * OrigNumElts) + OrigNumElts;
         StartIdx != EndIdx; ++StartIdx)
      ShufMask.push_back(StartIdx);

    Value *Shuff = Builder.CreateShuffleVector(
        VecV, UndefValue::get(cast<VectorType>(VecV->getType())), ShufMask,
        "extractsubvec.");

    VPScalarMap[V][Lane] = Shuff;

    return Shuff;
  }

  auto ScalarV = Builder.CreateExtractElement(VecV, Builder.getInt32(Lane),
                                              VecV->getName() + ".extract." +
                                                  Twine(Lane) + ".");

  // Add to scalar map.
  VPScalarMap[V][Lane] = ScalarV;
  return ScalarV;
}

void VPOCodeGen::vectorizeExtractElement(VPInstruction *VPInst) {
  // Vector operand will be first operand of extractelement, index will be
  // second operand.
  Value *ExtrFrom = getVectorValue(VPInst->getOperand(0));
  VPValue *OrigIndexVal = VPInst->getOperand(1);
  Type *VecTy = VPInst->getOperand(0)->getType();
  unsigned OriginalVL = cast<FixedVectorType>(VecTy)->getNumElements();

  // In case of a non-const index, we serialize the instruction.
  // We first get the actual index, for the vectorized data using
  // 'add', extract the element using the index and then finally insert it into
  // the narrower sub-vector.
  // Example -
  // %extract = extractelement <4 x float> %input, i32 %varidx
  // Vector IR for VF=2 -
  // %varidx1 = extractelement <2 x i32> %varidx.vec, i64 0
  // %offset1 = add i32 0, %varidx1
  // %res1 = extractelement <8 x float> %input.vec, i32 %offset1
  // %wide.extract1 = insertelement <2 x float> undef, float %res1, i64 0
  // %varidx2 = extractelement <2 x i32> %varidx.vec, i64 1
  // %offset2 = add i32 4, %varidx2
  // %res2 = extractelement <8 x float> %input.vec, i32 %offset2
  // %final = insertelement <2 x float> %wide.extract1, float %res2, i64 1
  if (!isa<VPConstant>(OrigIndexVal) ||
      !cast<VPConstant>(OrigIndexVal)->isConstantInt()) {

    if (MaskValue) {
      serializeWithPredication(VPInst);
      // Remark: masked instruction can't be vectorized
      getOptReportStats(VPInst).SerializedInstRemarks.emplace_back(15565, "");
      return;
    }

    Value *WideExtract =
        UndefValue::get(FixedVectorType::get(VPInst->getType(), VF));
    Value *IndexValVec = getVectorValue(OrigIndexVal);

    for (unsigned VIdx = 0; VIdx < VF; ++VIdx) {
      Value *IndexVal = Builder.CreateExtractElement(IndexValVec, VIdx);
      Value *VectorIdx = Builder.CreateAdd(
          ConstantInt::get(IndexVal->getType(), VIdx * OriginalVL), IndexVal);
      WideExtract = Builder.CreateInsertElement(
          WideExtract, Builder.CreateExtractElement(ExtrFrom, VectorIdx), VIdx);
    }
    VPWidenMap[VPInst] = WideExtract;
    // Remark: instruction was serialized due to non-const index
    getOptReportStats(VPInst).SerializedInstRemarks.emplace_back(15564, "");
    return;
  }

  VPConstant *OrigIndexVPConst = cast<VPConstant>(OrigIndexVal);
  assert(OrigIndexVPConst->isConstantInt() &&
         "Original index is not constant integer.");
  unsigned Index = OrigIndexVPConst->getZExtValue();

  // Extract subvector. The subvector should include VF elements.
  SmallVector<int, 8> ShufMask;
  unsigned WideNumElts = VF * OriginalVL;
  for (unsigned Idx = Index; Idx < WideNumElts; Idx += OriginalVL)
    ShufMask.push_back(Idx);
  Type *VTy = ExtrFrom->getType();
  VPWidenMap[VPInst] = Builder.CreateShuffleVector(
      ExtrFrom, UndefValue::get(VTy), ShufMask, "wide.extract");
}

void VPOCodeGen::vectorizeInsertElement(VPInstruction *VPInst) {
  Value *InsertTo = getVectorValue(VPInst->getOperand(0));
  Value *NewSubVec = getVectorValue(VPInst->getOperand(1));
  VPValue *OrigIndexVal = VPInst->getOperand(2);
  Type *VecTy = VPInst->getOperand(0)->getType();
  unsigned OriginalVL = cast<FixedVectorType>(VecTy)->getNumElements();

  // In case of an non-const index, we serialize the instruction.
  // We first get the actual index, for the vectorized data using
  // 'add' and then insert that scalar into the index.

  if (!isa<VPConstant>(OrigIndexVal) ||
      !cast<VPConstant>(OrigIndexVal)->isConstantInt()) {
    if (MaskValue) {
      serializeWithPredication(VPInst);
      // Remark: masked instruction can't be vectorized
      getOptReportStats(VPInst).SerializedInstRemarks.emplace_back(15565, "");
      return;
    }
    Value *WideInsert = InsertTo;
    Value *IndexValVec = getVectorValue(OrigIndexVal);

    for (unsigned VIdx = 0; VIdx < VF; ++VIdx) {
      Value *IndexVal = Builder.CreateExtractElement(IndexValVec, VIdx);
      Value *VectorIdx = Builder.CreateAdd(
          ConstantInt::get(IndexVal->getType(), VIdx * OriginalVL), IndexVal);

      // Insert the scalar value of second operand which can be vectorized
      // earlier.
      WideInsert = Builder.CreateInsertElement(
          WideInsert, getScalarValue(VPInst->getOperand(1), VIdx), VectorIdx);
    }
    VPWidenMap[VPInst] = WideInsert;
    // Remark: instruction was serialized due to non-const index
    getOptReportStats(VPInst).SerializedInstRemarks.emplace_back(15564, "");
    return;
  }

  // TODO: Need more test coverage for vectorizing insertelement with const
  // index, especially masked insert scenarios.

  VPConstant *OrigIndexVPConst = cast<VPConstant>(OrigIndexVal);
  assert(OrigIndexVPConst->isConstantInt() &&
         "Original index is not constant integer.");
  unsigned Index = OrigIndexVPConst->getZExtValue();
  unsigned WideNumElts =
      cast<FixedVectorType>(InsertTo->getType())->getNumElements();

  // Widen the insert into an empty, undef-vector
  // E.g. For OriginalVL = 4 and VF = 2, the following code,
  // %add13 = add i32 %scalar, %scalar9
  // %assembled.vect = insertelement <4 x i32> undef, i32 %add13, i32 0
  //
  // is transformed into,
  // %6 = add <2 x i32> %Wide.Extract12, %Wide.Extract
  // %wide.insert = shufflevector <2 x i32> %6, <2 x i32> undef,
  //                              <8 x i32> <i32 0, i32 undef, i32 undef, i32
  //                                         undef,
  //                                         i32 1, i32 undef, i32 undef, i32
  //                                         undef>

  if (isa<UndefValue>(InsertTo)) {
    SmallVector<Constant *, 8> ShufMask;
    ShufMask.resize(WideNumElts, UndefValue::get(Builder.getInt32Ty()));
    for (size_t Lane = 0; Lane < VF; Lane++)
      ShufMask[Lane * OriginalVL + Index] = Builder.getInt32(Lane);

    Value *Shuf = Builder.CreateShuffleVector(
        NewSubVec, UndefValue::get(NewSubVec->getType()),
        ConstantVector::get(ShufMask), "wide.insert");
    VPWidenMap[VPInst] = Shuf;
    return;
  }

  // Generate two shuffles. The first one is extending the Subvector to the
  // width of the source and the second one is for blending in the actual
  // values.
  // In continuation of the example above, the following code,
  // %assembled.vect17 = insertelement <4 x i32> %assembled.vect, i32 %add14,
  //                                                              i32 1
  //
  // is transformed into,
  // %extended. = shufflevector <2 x i32> %7,
  //                            <2 x i32> undef,
  //                            <8 x i32> <i32 0, i32 1, i32 2, i32 2,
  //                                       i32 2, i32 2, i32 2, i32 2>
  // %wide.insert16 = shufflevector <8 x i32> %wide.insert,
  //                                <8 x i32> %extended.,
  //                                <8 x i32> <i32 0, i32 8, i32 2, i32 3,
  //                                           i32 4, i32 9, i32 6, i32 7>

  Value *ExtendSubVec =
      extendVector(NewSubVec, WideNumElts, Builder, NewSubVec->getName());
  SmallVector<int, 8> ShufMask2;
  for (unsigned FirstVecIdx = 0, SecondVecIdx = WideNumElts;
       FirstVecIdx < WideNumElts; ++FirstVecIdx) {
    if ((FirstVecIdx % OriginalVL) == Index)
      ShufMask2.push_back(SecondVecIdx++);
    else
      ShufMask2.push_back(FirstVecIdx);
  }
  Value *SecondShuf = Builder.CreateShuffleVector(InsertTo, ExtendSubVec,
                                                  ShufMask2, "wide.insert");
  VPWidenMap[VPInst] = SecondShuf;
}

void VPOCodeGen::vectorizeShuffle(VPInstruction *VPInst) {
  unsigned OrigSrcVL =
      cast<FixedVectorType>(VPInst->getOperand(0)->getType())->getNumElements();
  int OrigDstVL = cast<FixedVectorType>(VPInst->getType())->getNumElements();

  Value *V0 = getVectorValue(VPInst->getOperand(0));
  Value *V1 = getVectorValue(VPInst->getOperand(1));
  auto *Mask = cast<VPConstant>(VPInst->getOperand(2))->getConstant();

  SmallVector<Constant *, 16> MaskIndices;
  for (unsigned LogicalLane = 0; LogicalLane < VF; ++LogicalLane) {
    for (int Idx = 0; Idx < OrigDstVL; ++Idx) {
      auto *MaskElt = Mask->getAggregateElement(Idx);
      if (isa<UndefValue>(MaskElt) || isa<PoisonValue>(MaskElt)) {
        // From the LangRef: If the shuffle mask selects an undefined element
        // from one of the input vectors, the resulting element is undefined. An
        // undefined element in the mask vector specifies that the resulting
        // element is undefined. An undefined element in the mask vector
        // prevents a poisoned vector element from propagating.
        //
        // For poison: Vector elements may be independently poisoned. Therefore,
        // transforms on instructions such as shufflevector must be careful to
        // propagate poison across values or elements only as allowed by the
        // original code... An instruction that depends on a poison value,
        // produces a poison value itself.
        MaskIndices.push_back(MaskElt);
        continue;
      }
      unsigned OrigIdx = cast<ConstantInt>(MaskElt)->getZExtValue();
      unsigned NewIdx;
      if (OrigIdx < OrigSrcVL) {
        NewIdx = OrigSrcVL * LogicalLane + OrigIdx;
      } else {
        NewIdx = OrigSrcVL * VF + OrigSrcVL * LogicalLane + (OrigIdx - OrigSrcVL);
      }
      MaskIndices.push_back(ConstantInt::get(MaskElt->getType(), NewIdx));
    }
  }
  VPWidenMap[VPInst] =
      Builder.CreateShuffleVector(V0, V1, ConstantVector::get(MaskIndices));
}

void VPOCodeGen::processPredicatedNonWidenedUniformCall(VPInstruction *VPInst) {
  if (MaskValue)
    return serializePredicatedUniformInstruction(VPInst);

  return serializeInstruction(VPInst);
}

Value *VPOCodeGen::getMaskNotAllZero() {
  assert(MaskValue && "Should only be called in masked context!");
  auto *MaskTy = dyn_cast<FixedVectorType>(MaskValue->getType());
  assert(MaskTy && MaskTy->getNumElements() == VF && "Unexpected Mask Type");
  // Emit not of all-zero check for mask
  Type *IntTy =
      IntegerType::get(MaskTy->getContext(), MaskTy->getPrimitiveSizeInBits());
  auto *MaskBitCast = Builder.CreateBitCast(MaskValue, IntTy);

  // Check if the bitcast value is not zero. The generated compare will be true
  // if atleast one of the i1 masks in <VF x i1> is true.
  auto *CmpInst =
      Builder.CreateICmpNE(MaskBitCast, Constant::getNullValue(IntTy));

  return CmpInst;
}

void VPOCodeGen::serializePredicatedUniformInstruction(VPInstruction *VPInst) {
  // Mask is needed before the predicated instruction, so generate the code for
  // it.
  auto *NotAllZero = getMaskNotAllZero();

  // Now create a scalar instruction, populating correct values for its operands.
  SmallVector<Value *, 4> ScalarOperands;
  for (unsigned Op = 0, e = VPInst->getNumOperands(); Op != e; ++Op) {
    auto *ScalarOp = getScalarValue(VPInst->getOperand(Op), 0 /*Lane*/);
    assert(ScalarOp && "Operand for serialized uniform instruction not found.");
    ScalarOperands.push_back(ScalarOp);
  }

  Value *SerialInstruction = generateSerialInstruction(VPInst, ScalarOperands);
  VPScalarMap[VPInst][0] = SerialInstruction;

  PredicatedInstructions.push_back(
      std::make_pair(cast<Instruction>(SerialInstruction), NotAllZero));
}

void VPOCodeGen::serializeWithPredication(VPInstruction *VPInst) {
  if (!MaskValue)
    return serializeInstruction(VPInst);

  assert(cast<FixedVectorType>(MaskValue->getType())->getNumElements() == VF &&
         "Unexpected Mask Type");

  for (unsigned Lane = 0; Lane < VF; ++Lane) {
    Value *Cmp = Builder.CreateExtractElement(MaskValue, Lane, "Predicate");
    Cmp = Builder.CreateICmp(ICmpInst::ICMP_EQ, Cmp,
                             ConstantInt::get(Cmp->getType(), 1));

    SmallVector<Value *, 4> ScalarOperands;
    // All operands to the serialized Instruction should be original loop
    // Values.
    for (unsigned Op = 0, e = VPInst->getNumOperands(); Op != e; ++Op) {
      auto *ScalarOp = getScalarValue(VPInst->getOperand(Op), Lane);
      assert(ScalarOp && "Operand for serialized instruction not found.");
      LLVM_DEBUG(dbgs() << "LVCG: Serialize scalar op: "; ScalarOp->dump());
      ScalarOperands.push_back(ScalarOp);
    }

    Value *SerialInst = generateSerialInstruction(VPInst, ScalarOperands);
    assert(SerialInst && "Instruction not serialized.");
    VPScalarMap[VPInst][Lane] = SerialInst;
    PredicatedInstructions.push_back(
        std::make_pair(cast<Instruction>(SerialInst), Cmp));
    LLVM_DEBUG(dbgs() << "LVCG: SerialInst: "; SerialInst->dump());
  }
}

void VPOCodeGen::serializeInstruction(VPInstruction *VPInst) {

  unsigned Lanes =
      (!VPInst->mayHaveSideEffects() &&
       Plan->getVPlanDA()->isUniform(*VPInst)) ||
              (isa<VPCallInstruction>(VPInst) &&
               cast<VPCallInstruction>(VPInst)->getVectorizationScenario() ==
                   VPCallInstruction::CallVecScenariosTy::DoNotWiden)
          ? 1
          : VF;

  auto *VPPhiInst = dyn_cast<VPPHINode>(VPInst);
  for (unsigned Lane = 0; Lane < Lanes; ++Lane) {
    SmallVector<Value *, 4> ScalarOperands;
    // All operands to the serialized Instruction should be scalar Values. Skip
    // PHIs since their operands may not be generated yet.
    if (!VPPhiInst) {
      for (unsigned Op = 0, e = VPInst->getNumOperands(); Op != e; ++Op) {
        auto *ScalarOp = getScalarValue(VPInst->getOperand(Op), Lane);
        assert(ScalarOp && "Operand for serialized instruction not found.");
        LLVM_DEBUG(dbgs() << "LVCG: Serialize scalar op: "; ScalarOp->dump());
        ScalarOperands.push_back(ScalarOp);
      }
    }

    Value *SerialInst = generateSerialInstruction(VPInst, ScalarOperands);
    assert(SerialInst && "Instruction not serialized.");
    VPScalarMap[VPInst][Lane] = SerialInst;
    if (VPPhiInst)
      PhisToFix[cast<PHINode>(SerialInst)] = std::make_pair(VPPhiInst, Lane);
    LLVM_DEBUG(dbgs() << "LVCG: SerialInst: "; SerialInst->dump());
  }
}

// Widen blend instruction. The implementation here generates a sequence
// of selects. Consider the following scalar blend in bb0:
//      vp1 = blend [vp2, bp2] [vp3, bp3] [vp4, bp4].
// The selects are generated as follows:
//      select1 = bp3_vec ? vp3_vec : vp2_vec
//      vp1_vec = bp4_vec ? vp4_vec : select1
void VPOCodeGen::vectorizeBlend(VPBlendInst *Blend) {
  unsigned NumIncomingValues = Blend->getNumIncomingValues();
  assert(NumIncomingValues > 0 && "Unexpected blend with zero values");

  // Generate a sequence of selects.
  Value *BlendVal = nullptr;
  for (unsigned Idx = 0, End = NumIncomingValues; Idx < End;
       ++Idx) {
    Value *IncomingVecVal = getVectorValue(Blend->getIncomingValue(Idx));
    if (!BlendVal) {
      BlendVal = IncomingVecVal;
      continue;
    }

    VPValue *BlockPred = Blend->getIncomingPredicate(Idx);
    assert(BlockPred && "block-predicate should not be null for select");
    Value *Cond = getVectorValue(BlockPred);
    if (Blend->getType()->isVectorTy()) {
      unsigned OriginalVL =
          cast<FixedVectorType>(Blend->getType())->getNumElements();
      Cond = replicateVectorElts(Cond, OriginalVL, Builder);
    }
    BlendVal =
        Builder.CreateSelect(Cond, IncomingVecVal, BlendVal, "predblend");
  }

  VPWidenMap[Blend] = BlendVal;
}

void VPOCodeGen::vectorizeVPPHINode(VPPHINode *VPPhi) {
  auto PhiTy = VPPhi->getType();
  if (!isVectorizableTy(PhiTy)) {
    // PHIs cannot be predicated so it should be safe to serialize without
    // predication.
    assert(!MaskValue && "PHIs cannot be predicated.");
    serializeInstruction(VPPhi);
    return;
  }

  PHINode *NewPhi;
  // PHI-arguments with SOA accesses need to be set up with correct-types.
  if (isSOAAccess(VPPhi, Plan)) {
    auto *PhiPtrTy = cast<PointerType>(PhiTy);
    if (!PhiPtrTy->isOpaque()) {
      Type *ElemTy = PhiTy->getPointerElementType();
      PhiTy = PointerType::get(getSOAType(ElemTy, VF),
                               PhiPtrTy->getAddressSpace());
    }
  }
  // FIXME: Replace with proper SVA.
  bool EmitScalarOnly = !Plan->getVPlanDA()->isDivergent(*VPPhi) && !MaskValue;
  if (needScalarCode(VPPhi) || EmitScalarOnly || isSOAUnitStride(VPPhi, Plan)) {
    NewPhi = Builder.CreatePHI(PhiTy, VPPhi->getNumOperands(), "uni.phi");
    VPScalarMap[VPPhi][0] = NewPhi;
    PhisToFix[NewPhi] = std::make_pair(VPPhi, 0);
  }
  if (EmitScalarOnly)
    return;
  if (needVectorCode(VPPhi) && !isSOAUnitStride(VPPhi, Plan)) {
    PhiTy = getWidenedType(PhiTy, VF);
    NewPhi = Builder.CreatePHI(PhiTy, VPPhi->getNumOperands(), "vec.phi");
    VPWidenMap[VPPhi] = NewPhi;
    PhisToFix[NewPhi] = std::make_pair(VPPhi, -1);
  }
}

void VPOCodeGen::vectorizePrivateFinalUncond(VPInstruction *VPInst) {

  Value *Ret;
  Value *Operand = getVectorValue(VPInst->getOperand(0));
  if (VPInst->getOperand(0)->getType()->isVectorTy())
    Ret = generateExtractSubVector(Operand, VF - 1, VF, Builder,
                                   "extracted.priv");
  else
    Ret = Builder.CreateExtractElement(Operand, VF - 1, "extracted.priv");

  VPScalarMap[VPInst][0] = Ret;
}

void VPOCodeGen::vectorizeRunningInclusiveReduction(
    VPRunningInclusiveReduction *RunningReduction) {
  // Here and below examples are for a case of integer sum reduction, VF = 4.
  // Operation as follows.
  // <4 x Ty> running-inclusive-reduction(<4 x Ty> vx, Ty x) {
  //   return [vx3 + vx2 + vx1 + vx0 + x,
  //           vx2 + vx1 + vx0 + x,
  //           vx1 + vx0 + x,
  //           vx0 + x];
  //  }

  // This can be optimized since some of the additions are repeated across
  // different vector lanes. The following approach asks for 2 shuffles and
  // 2 additions.
  // %shuf1 = shufflevector %vx, zeroinitalizer, <4, 0, 1, 2>
  //         ; yields <0, vx0, vx1, vx2>
  // %add1 = add %vx, %shuf1
  //         ; yields <vx0, vx0 + vx1, vx1 + vx2, vx2 + vx3>
  // %shuf2 = shufflevector %add1, zeroinitalizer <4, 5, 0, 1>
  //         ; <0, 0, vx0, vx0 + vx1>
  // %add2 = add %add1, %shuf2
  //         ; <vx0, vx0 + vx1, vx0 + vx1 + vx2, vx0 + vx1 + vx2 + vx3>
  //
  // If generalized, this approach asks for log(VF) shuffles and summations.
  // Each step involves shuffle with certain amount of the first elements
  // being Identity values, the rest of elements are shifted right.
  // The number of the first elements displaced with Identity values is
  // different on even and odd steps (assuming numbering starting from one).
  // For odd numbered shuffle+add steps, the number of displaced elements is
  // 2**i, where counter i starts from 0 and is incremented by 1.
  // For even numbered steps it is
  // VF / 2 ** j, where j counter starts from 1 and is incremented by 1.
  //
  // For example,
  // With VF = 4, on the first step we displace 2**0 = 1 elements,
  // on the second step we displace VF / 2 ** 1 = 2 elements.
  // We need only log(VF) = 2 steps to complete scan reduction.
  //
  Value *Input = getVectorValue(RunningReduction->getInputOperand());
  Value *CarryOver = getVectorValue(RunningReduction->getCarryOverOperand());
  Value *IdentityValue = getVectorValue(RunningReduction->getIdentityOperand());

  unsigned Step = 1;
  unsigned Steps = Log2_64(VF);
  unsigned i = 0, j = 1;
  Value *LastReduced = Input;
  auto BinOpCode =
    static_cast<Instruction::BinaryOps>(RunningReduction->getBinOpcode());

  // Utility to set FastMathFlags for generated instructions.
  auto SetFastMathFlags = [RunningReduction](Value *V) {
    if (isa<FPMathOperator>(V) && RunningReduction->hasFastMathFlags())
      cast<Instruction>(V)->setFastMathFlags(
        RunningReduction->getFastMathFlags());
  };

  while (Step <= Steps) {
    unsigned DisplacedElems = 0;
    if (Step % 2 == 1) {
      DisplacedElems = 1 << i;
      i++;
    } else {
      DisplacedElems = VF / (1 << j);
      j++;
    }
    Step++;

    // Prepare the shuffle mask for this step.
    SmallVector<int, 8> ShufMask;
    for(unsigned k = 0; k < DisplacedElems; k++)
      ShufMask.push_back(VF + k); // Pick from Identity vector;
    for (unsigned k = 0; k < VF - DisplacedElems; k++)
      ShufMask.push_back(k);  // Pick from the last reduced vector.
    Value *Shuff =
      Builder.CreateShuffleVector(LastReduced, IdentityValue, ShufMask);
    LastReduced = Builder.CreateBinOp(BinOpCode, LastReduced, Shuff);
    // Set FMF for generated reduction code.
    SetFastMathFlags(LastReduced);
  }

  // Reduce with the carry-over value from the previous iteration.
  LastReduced = Builder.CreateBinOp(BinOpCode, LastReduced, CarryOver);
  SetFastMathFlags(LastReduced);

  VPWidenMap[RunningReduction] = LastReduced;
  return;
}

void VPOCodeGen::vectorizeReductionFinal(VPReductionFinal *RedFinal) {
  unsigned BinOpcode = RedFinal->getBinOpcode();
  if (BinOpcode == Instruction::ICmp || BinOpcode == Instruction::FCmp) {
    vectorizeSelectCmpReductionFinal(RedFinal);
    return;
  }

  Value *VecValue = getVectorValue(RedFinal->getOperand(0));
  Intrinsic::ID Intrin = RedFinal->getVectorReduceIntrinsic();
  Type *ElType = RedFinal->getOperand(0)->getType();
  if (isa<VectorType>(ElType))
    // TODO: can implement as shufle/OP sequence for vectors.
    llvm_unreachable("Unsupported vector data type in reduction");

  auto *StartVPVal =
      RedFinal->getNumOperands() > 1 ? RedFinal->getOperand(1) : nullptr;
  Value *Acc = nullptr;
  if (StartVPVal)
    Acc = getScalarValue(StartVPVal, 0);

  Value *Ret = nullptr;
  // TODO: Need meaningful processing for Acc for FP reductions, and NoNan
  // parameter.
  switch (Intrin) {
  case Intrinsic::vector_reduce_fadd:
    assert(Acc && "Expected initial value");
    Ret = Builder.CreateFAddReduce(Acc, VecValue);
    Acc = nullptr;
    break;
  case Intrinsic::vector_reduce_fmul:
    assert(Acc && "Expected initial value");
    Ret = Builder.CreateFMulReduce(Acc, VecValue);
    Acc = nullptr;
    break;
  case Intrinsic::vector_reduce_add:
    Ret = Builder.CreateAddReduce(VecValue);
    break;
  case Intrinsic::vector_reduce_mul:
    Ret = Builder.CreateMulReduce(VecValue);
    break;
  case Intrinsic::vector_reduce_and:
    Ret = Builder.CreateAndReduce(VecValue);
    break;
  case Intrinsic::vector_reduce_or:
    Ret = Builder.CreateOrReduce(VecValue);
    break;
  case Intrinsic::vector_reduce_xor:
    Ret = Builder.CreateXorReduce(VecValue);
    break;
  case Intrinsic::vector_reduce_umax:
    assert(!Acc && "Unexpected initial value");
    Ret = Builder.CreateIntMaxReduce(VecValue, false);
    break;
  case Intrinsic::vector_reduce_smax:
    assert(!Acc && "Unexpected initial value");
    Ret = Builder.CreateIntMaxReduce(VecValue, true);
    break;
  case Intrinsic::vector_reduce_umin:
    assert(!Acc && "Unexpected initial value");
    Ret = Builder.CreateIntMinReduce(VecValue, false);
    break;
  case Intrinsic::vector_reduce_smin:
    assert(!Acc && "Unexpected initial value");
    Ret = Builder.CreateIntMinReduce(VecValue, true);
    break;
  case Intrinsic::vector_reduce_fmax:
    assert(!Acc && "Unexpected initial value");
    Ret = Builder.CreateFPMaxReduce(VecValue);
    break;
  case Intrinsic::vector_reduce_fmin:
    assert(!Acc && "Unexpected initial value");
    Ret = Builder.CreateFPMinReduce(VecValue);
    break;
  default:
    llvm_unreachable("unsupported reduction");
    break;
  }
  // Utility to set FastMathFlags for generated instructions.
  auto SetFastMathFlags = [RedFinal](Value *V) {
    if (isa<FPMathOperator>(V) && RedFinal->hasFastMathFlags())
      cast<Instruction>(V)->setFastMathFlags(RedFinal->getFastMathFlags());
  };
  // Set FMF for generated vector reduce intrinsic.
  SetFastMathFlags(Ret);

  if (Acc) {
    Ret = Builder.CreateBinOp(
        static_cast<Instruction::BinaryOps>(RedFinal->getBinOpcode()), Acc, Ret,
        "final.red");
    SetFastMathFlags(Ret);
  }

  VPScalarMap[RedFinal][0] = Ret;
}

void VPOCodeGen::vectorizeSelectCmpReductionFinal(VPReductionFinal *RedFinal) {
  assert(RedFinal->getNumOperands() == 3
         && "Wrong operand count for SelectCmp reduction-final!");
  assert(!isa<VectorType>(RedFinal->getOperand(0)->getType())
         && "Unsupported vector data type in reduction");

  Value *VecValue = getVectorValue(RedFinal->getOperand(0));
  Value *StartVPVal = getScalarValue(RedFinal->getOperand(1), 0);
  Value *ChangeVPVal = getScalarValue(RedFinal->getOperand(2), 0);
  Value *Splat = Builder.CreateVectorSplat(VF, StartVPVal);
  // FIXME: Can handle floating-point reductions with FCmp UNE also.
  Value *Cmp = Builder.CreateICmpNE(VecValue, Splat);
  Value *Reduc = Builder.CreateOrReduce(Cmp);
  Value *Ret = Builder.CreateSelect(Reduc, ChangeVPVal, StartVPVal);
  VPScalarMap[RedFinal][0] = Ret;
}

void VPOCodeGen::vectorizeAllocatePrivate(VPAllocatePrivate *V) {
  // Private memory is a pointer. We need to get element type
  // and allocate VF elements.
  Type *OrigTy = V->getAllocatedType();

  Type *VecTyForAlloca;
  std::string VarName = Twine(V->getOrigName() + ".vec").str();
  // TODO. We should handle the case when original alloca has the size argument,
  // e.g. it's like alloca i32, i32 4.
  if (OrigTy->isAggregateType())
    if (V->isSOALayout()) {
      // TODO: Compute the correct alignment value.
      VarName = Twine(V->getOrigName() + ".soa.vec").str();
      VecTyForAlloca = getSOAType(OrigTy, VF);
    } else
      VecTyForAlloca = ArrayType::get(OrigTy, VF);
  else {
    // For non-aggregate types create a vector type.
    Type *EltTy = OrigTy;
    unsigned NumEls = VF;
    if (OrigTy->isVectorTy()) {
      auto *OrigVecTy = cast<FixedVectorType>(OrigTy);
      EltTy = OrigVecTy->getElementType();
      NumEls *= OrigVecTy->getNumElements();
    }
    VecTyForAlloca = FixedVectorType::get(EltTy, NumEls);
  }

  // Compute preferred alignment for vector alloca.
  const DataLayout &DL = OrigLoop->getHeader()->getModule()->getDataLayout();
  Align VecAllocaPrefAlignment = DL.getPrefTypeAlign(VecTyForAlloca);
  // Get original memory's alignment.
  Align OrigAlignment = V->getOrigAlignment();

  // Create an alloca in the appropriate block
  IRBuilder<>::InsertPointGuard Guard(Builder);
  Function *F = OrigLoop->getHeader()->getParent();
  BasicBlock &FirstBB = F->front();
  assert(FirstBB.getTerminator() &&
         "Expect the 'entry' basic-block to be well-formed.");
  Builder.SetInsertPoint(FirstBB.getTerminator());

  // TODO: We potentially need additional divisibility-based checks here to
  // ensure that correct alignment is set for each vector lane. Check JIRA :
  // CMPLRLLVM-11372.
  // TODO: Currently we are allowing all scalar privates to always have
  // vectorized private alloca because DA has been taught to allow unit-stride
  // accesses for scalar privates. However this assumption is not safe and DA
  // should be updated to use results from SOAAnalysis (which should internally
  // account for alignment for all types, before marking a private as SOASafe).
  // Again coverred by JIRA : CMPLRLLVM-11372.
  if (VecAllocaPrefAlignment >= OrigAlignment || V->isSOALayout()) {
    // Use widened alloca if preferred alignment can accommodate original
    // alloca's alignment or if we're using SOALayout.
    Value *WidenedPrivArr =
        Builder.CreateAlloca(VecTyForAlloca, nullptr, VarName);
    cast<AllocaInst>(WidenedPrivArr)->setAlignment(VecAllocaPrefAlignment);
    // If address space of widened alloca and private's data type don't match,
    // then emit an explicit addrspacecast. This casted value represents the
    // wide memory corresponding to the loop private variable.
    unsigned OrigAddrSpace = V->getType()->getPointerAddressSpace();
    if (WidenedPrivArr->getType()->getPointerAddressSpace() != OrigAddrSpace) {
      // Generate a cast from VecTyForAlloca* to VecTyForAlloca addrspace(x)*.
      Type *DestPtrTy = VecTyForAlloca->getPointerTo(OrigAddrSpace);
      WidenedPrivArr = Builder.CreateAddrSpaceCast(WidenedPrivArr, DestPtrTy,
                                                   VarName + ".ascast");
    }

    LoopPrivateVPWidenMap[V] = WidenedPrivArr;
    // If this is a scalar-private, do not store this in the VPScalarMap. All
    // operations on scalar-privates, even for SOA, should be done via the
    // VPWidenMap or the LoopPrivateWidenMap.
    if (V->isSOALayout() && !isScalarTy(OrigTy))
      VPScalarMap[V][0] = WidenedPrivArr;
    else
      VPWidenMap[V] = createVectorPrivatePtrs(V);
  } else {
    // If preferred alignment is less than original alignment, generate VF
    // copies of original alloca (one for each lane) and construct the
    // corresponding vector of pointers.
    Value *PtrsVector = UndefValue::get(getWidenedType(V->getType(), VF));
    for (unsigned I = 0; I < VF; ++I) {
      AllocaInst *SerialPrivArr = Builder.CreateAlloca(
          OrigTy, nullptr, V->getOrigName() + ".lane." + Twine(I));
      SerialPrivArr->setAlignment(OrigAlignment);
      PtrsVector =
          Builder.CreateInsertElement(PtrsVector, SerialPrivArr, I,
                                      V->getOrigName() + ".insert." + Twine(I));
    }
    VPWidenMap[V] = PtrsVector;
  }
}

// InductionInit has two arguments {Start, Step} and keeps the operation
// opcode. We generate
// For +/-   : broadcast(start) +/GEP step*{0, 1,..,VL-1} (GEP for pointers)
// For */div : broadcast(start) * pow(step,{0, 1,..,VL-1})
// In the current version, pow() is replaced with a series of multiplications.
void VPOCodeGen::vectorizeInductionInit(VPInductionInit *VPInst) {
  auto *StartVPVal = VPInst->getOperand(0);
  auto *StartVal = getScalarValue(StartVPVal, 0);
  Value *BcastStart =
      createVectorSplat(StartVal, VF, Builder, "ind.start.bcast");

  auto *StepVPVal = VPInst->getOperand(1);
  Value *StepVal = getScalarValue(StepVPVal, 0);
  unsigned Opc = VPInst->getBinOpcode();
  bool isMult = Opc == Instruction::Mul || Opc == Instruction::FMul ||
                Opc == Instruction::SDiv || Opc == Instruction::UDiv ||
                Opc == Instruction::FDiv;
  bool IsFloat = VPInst->getType()->isFloatingPointTy();
  int StartConst = isMult ? 1 : 0;
  Constant *StartCoeff =
      IsFloat ? ConstantFP::get(VPInst->getType(), StartConst)
              : ConstantInt::getSigned(StepVal->getType(), StartConst);
  Value *VectorStep;
  if (isMult) {
    // Generate series of mult and insert operations, to avoid calling pow(),
    // forming the following vector
    // {StartCoeff, StartCoeff*Step, StartCoeff*Step*Step, ...,
    //  StartCoeff{*Step}{VF-1 times}}
    unsigned StepOpc = IsFloat ? Instruction::FMul : Instruction::Mul;
    Value *Val = StartCoeff;
    VectorStep = createVectorSplat(UndefValue::get(Val->getType()), VF, Builder,
                                   "ind.step.vec");
    unsigned I = 0;
    for (I = 0; I < VF - 1; I++) {
      VectorStep = Builder.CreateInsertElement(VectorStep, Val, I);
      Val = Builder.CreateBinOp(static_cast<Instruction::BinaryOps>(StepOpc),
                                Val, StepVal);
    }
    // Here I = VF - 1.
    VectorStep = Builder.CreateInsertElement(VectorStep, Val, I);
  } else {
    // Generate sequence of vector operations:
    // %i_seq = {0, 1, 2, 3, ...VF-1}
    // %bcst_step = broadcast step
    // %vector_step = mul %i_seq, %bcst_step
    SmallVector<Constant *, 32> IndStep;
    IndStep.push_back(StartCoeff);
    for (unsigned I = 1; I < VF; I++) {
      Constant *ConstVal = IsFloat
                               ? ConstantFP::get(VPInst->getType(), I)
                               : ConstantInt::getSigned(StepVal->getType(), I);
      IndStep.push_back(ConstVal);
    }
    Value *VecConst = ConstantVector::get(IndStep);
    Value *BcstStep = createVectorSplat(StepVal, VF, Builder, "ind.step.vec");
    VectorStep = Builder.CreateBinOp(
        IsFloat ? Instruction::FMul : Instruction::Mul, BcstStep, VecConst);

    if (auto BinOp = dyn_cast<BinaryOperator>(VectorStep))
      // May be a constant.
      if (BinOp->getOpcode() == Instruction::FMul) {
        FastMathFlags Flags;
        Flags.setFast();
        BinOp->setFastMathFlags(Flags);
      }
  }
  Value *Ret =
      (VPInst->getType()->isPointerTy() || Opc == Instruction::GetElementPtr)
          ? Builder.CreateInBoundsGEP(
                getInt8OrPointerElementTy(
                    BcastStart->getType()->getScalarType()),
                BcastStart, VectorStep, "vector_gep")
          : Builder.CreateBinOp(static_cast<Instruction::BinaryOps>(Opc),
                                BcastStart, VectorStep);
  VPWidenMap[VPInst] = Ret;
  if (needScalarCode(VPInst)) {
    VPScalarMap[VPInst][0] = StartVal;
  }
}

void VPOCodeGen::vectorizeInductionInitStep(VPInductionInitStep *VPInst) {
  unsigned Opc = VPInst->getBinOpcode();
  bool isMult = Opc == Instruction::Mul || Opc == Instruction::FMul ||
                Opc == Instruction::SDiv || Opc == Instruction::UDiv ||
                Opc == Instruction::FDiv;
  bool IsFloat = VPInst->getType()->isFloatingPointTy();
  auto *StartVPVal = VPInst->getOperand(0);
  auto *StartVal = getScalarValue(StartVPVal, 0);

  unsigned StepOpc = IsFloat ? Instruction::FMul : Instruction::Mul;
  Value *MulVF = StartVal;
  if (isMult) {
    for (unsigned I = 1; I < VF; I *= 2)
      MulVF = Builder.CreateBinOp(static_cast<Instruction::BinaryOps>(StepOpc),
                                  MulVF, MulVF);
  } else {
    Constant *VFVal = IsFloat ? ConstantFP::get(VPInst->getType(), VF)
                              : ConstantInt::getSigned(StartVal->getType(), VF);
    MulVF = Builder.CreateBinOp(static_cast<Instruction::BinaryOps>(StepOpc),
                                MulVF, VFVal);
  }
  Value *Ret = createVectorSplat(MulVF, VF, Builder, "ind.step.init");
  VPWidenMap[VPInst] = Ret;

  if (needScalarCode(VPInst)) {
    VPScalarMap[VPInst][0] = MulVF;
  }
}

void VPOCodeGen::vectorizeInductionFinal(VPInductionFinal *VPInst) {
  Value *LastValue = nullptr;
  if (VPInst->getNumOperands() == 1) {
    // One operand - extract from vector
    Value *VecVal = getVectorValue(VPInst->getOperand(0));
    LastValue = Builder.CreateExtractElement(VecVal, Builder.getInt32(VF - 1));
  } else {
    // Otherwise calculate by formulas
    //  for post increment liveouts LV = start + step*upper_bound,
    //  for pre increment liveouts LV = start + step*(upper_bound-1)
    //
    assert(VPInst->getNumOperands() == 2 && "Incorrect number of operands");
    unsigned Opc = VPInst->getBinOpcode();
    assert(!(Opc == Instruction::Mul || Opc == Instruction::FMul ||
             Opc == Instruction::SDiv || Opc == Instruction::UDiv ||
             Opc == Instruction::FDiv) &&
           "Unsupported induction final form");

    bool IsFloat = VPInst->getType()->isFloatingPointTy();
    auto *VPStep = VPInst->getOperand(1);
    auto *Step = getScalarValue(VPStep, 0);

    unsigned StepOpc = IsFloat ? Instruction::FMul : Instruction::Mul;
    Type *StepType = Step->getType();

    // Try to find the latch comparison of the loop. We know that
    // VPInductionFinal is created in the loop exit so get its parent
    // predecessor to find a loop. The upper bound of the loop is used in the
    // latch comparison (this is ensured either by the check for normalized IV
    // or by emission of the new IV).
    // If the latch comparison is not found, use global VectorTripCount. This
    // can happen in case when we optimize the branch that uses the comparison,
    // when we know the upper bound is equal to VF. That means we don't create
    // peel/remainder loops and have only one main loop.
    // Then make the calculations by the formula above.
    VPLoop *L = nullptr;
    VPBasicBlock *VPIndFinalBB =
        *VPInst->getParent()->getPredecessors().begin();
    L = Plan->getVPLoopInfo()->getLoopFor(VPIndFinalBB);
    while (!L) {
      VPIndFinalBB = *VPIndFinalBB->getPredecessors().begin();
      L = Plan->getVPLoopInfo()->getLoopFor(VPIndFinalBB);
    }
    bool ExactUB = L->exactUB();
    VPCmpInst *Cond = L->getLatchComparison();
    Value *TripCnt = VectorTripCount;
    if (Cond) {
      VPValue *VPTripCount = L->isDefOutside(Cond->getOperand(0))
                                 ? Cond->getOperand(0)
                                 : Cond->getOperand(1);
      TripCnt = getScalarValue(VPTripCount, 0);
    } else {
      assert(ExactUB && "Expected exact UB");
    }
    if (VPInst->isLastValPreIncrement())
      TripCnt =
          Builder.CreateSub(TripCnt, ConstantInt::get(TripCnt->getType(), 1));
    if (!ExactUB)
      TripCnt =
          Builder.CreateAdd(TripCnt, ConstantInt::get(TripCnt->getType(), 1));
    Instruction::CastOps CastOp =
        CastInst::getCastOpcode(TripCnt, true, StepType, true);
    Value *CRD = Builder.CreateCast(CastOp, TripCnt, StepType, "cast.crd");
    Value *MulV = Builder.CreateBinOp(
        static_cast<Instruction::BinaryOps>(StepOpc), Step, CRD);
    auto *VPStart = VPInst->getOperand(0);
    auto *Start = getScalarValue(VPStart, 0);
    LastValue =
        (VPInst->getType()->isPointerTy() || Opc == Instruction::GetElementPtr)
            ? Builder.CreateInBoundsGEP(
                  getInt8OrPointerElementTy(Start->getType()->getScalarType()),
                  Start, MulV, "final_gep")
            : Builder.CreateBinOp(static_cast<Instruction::BinaryOps>(Opc),
                                  Start, MulV);
  }
  // The value is scalar
  VPScalarMap[VPInst][0] = LastValue;
}

void VPOCodeGen::attachPreferredAlignmentMetadata(Instruction *Memref,
                                                  Align PreferredAlignment) {
  auto &C = Builder.getContext();
  auto *CI = ConstantInt::get(Type::getInt32Ty(C), PreferredAlignment.value());
  SmallVector<Metadata *, 1> Ops{ConstantAsMetadata::get(CI)};
  Memref->setMetadata("intel.preferred_alignment", MDTuple::get(C, Ops));
}

void VPOCodeGen::fixNonInductionVPPhis() {
  auto fixPHI = [&](VPPHINode *VPPhi, PHINode *Phi, int Lane = -1) -> void {
    const unsigned NumPhiValues = VPPhi->getNumIncomingValues();
    for (unsigned I = 0; I < NumPhiValues; ++I) {
      auto *VPVal = VPPhi->getIncomingValue(I);
      auto *VPBB = VPPhi->getIncomingBlock(I);
      BasicBlock *BB = State->CFG.VPBB2IREndBB[VPBB];
      bool IsScalar = Lane != -1;
      Value *IncValue;
      if (IsScalar) {
        IncValue = getScalarValue(VPVal, Lane);
      } else {
        // At this point we have VF set to the main VPlan VF. So if we process a
        // phi that is in the peel/remainder VPlan which has another VF we
        // should be careful with uniform values that are fed into vector phis.
        // We need to broadcast them with the correct VF. Also, the broadcasts
        // should be placed correctly to avoid domination issues.

        // First, get the needed VF.
        Type *ValTy = Phi->getType();
        assert(ValTy->isVectorTy() && "expected vector type");
        unsigned CurVF = cast<FixedVectorType>(ValTy)->getNumElements();
        Type *VPTy = VPVal->getType();
        if (VPTy->isVectorTy()) {
          // vectors are re-vectorized into vectors with OldVF x VF length.
          unsigned NumElts = cast<FixedVectorType>(VPTy)->getNumElements();
          CurVF /= NumElts;
          assert(CurVF > 1 && "unexpected VF");
        }

        if (isa<VPExternalDef>(VPVal) || isa<VPConstant>(VPVal) ||
            isa<VPMetadataAsValue>(VPVal)) {
          // Externals: insert broadcast in the end of incoming block and don't
          // update value maps.
          IRBuilder<>::InsertPointGuard Guard(Builder);
          Builder.SetInsertPoint(BB->getTerminator());
          IncValue = getVectorValueForExternal(VPVal, CurVF);
        } else {
          // In case we have a uniform value here, we need to use the correct
          // VF.
          llvm::SaveAndRestore<unsigned> SvVF(VF, CurVF);
          IncValue = getVectorValue(VPVal);
        }
      }
      // We can have a scenario when dealing with SOA pointers
      // where it is not the same as the targeted type. We have to insert a
      // bitcast it to an appropriate type.
      // This is typically on account of phi's inserted by AZB where one of
      // the argument is of SOA-type and the other is a null. Since DA is
      // unaware of this, we should typecast the pointer argument to the
      // PHi-type.
      // i32* %vp1 = phi  [ i32* %vp2, BB16 ],  [ i32* null, BB17 ]
      if (isSOAAccess(VPPhi, Plan) && IncValue->getType() != Phi->getType()) {
        assert(isa<Constant>(IncValue) &&
               cast<Constant>(IncValue)->isNullValue() &&
               "Expect this argument to be a constant null-value.");
        // We expect this to be a constant nullptr value, so, no need of
        // setting up builder's insertion point.
        IncValue = Builder.CreateBitCast(IncValue, Phi->getType());
      }
      if (auto *LiveOut = dyn_cast<VPRemainderOrigLiveOut>(VPVal)) {
        // Add outgoing from the scalar loop.
        Loop *L = cast<VPScalarRemainder>(LiveOut->getOperand(0))->getLoop();
        BB = L->getLoopLatch();
        if (!any_of(predecessors(Phi->getParent()),
                    [BB](auto Pred) { return Pred == BB; })) {
          // If the latch is not a predecessor of the phi-block
          // try its non-loop-header successor.
          auto *Br = cast<BranchInst>(BB->getTerminator());
          BB = Br->getOperand(1) == L->getHeader()
                   ? cast<BasicBlock>(Br->getOperand(2))
                   : cast<BasicBlock>(Br->getOperand(1));
        }
        assert(any_of(predecessors(Phi->getParent()),
                      [BB](auto Pred) { return Pred == BB; }) &&
               "can't find correct incoming block");
      }
      Phi->addIncoming(IncValue, BB);
    }
  };

  for (auto &MapIt : PhisToFix) {
    int Lane = MapIt.second.second;
    fixPHI(MapIt.second.first, MapIt.first, Lane);
  }
}

void VPOCodeGen::lowerRemarksForVectorLoops() {
  auto LowerRemarksForLoop = [this](VPLoop *VPL) {
    // Emit statistics related remarks for the loop.
    Plan->getOptRptStatsForLoop(VPL).emitRemarks(
        ORBuilder, VPL, const_cast<VPLoopInfo *>(Plan->getVPLoopInfo()));

    auto OR = VPL->getOptReport();
    if (!OR)
      return;

    // Identify the IR loop that corresponds to current VPLoop.
    auto *VPLpHeader = VPL->getHeader();
    auto *LpHeader = cast<BasicBlock>(getScalarValue(VPLpHeader, 0));
    Loop *IRLp = LI->getLoopFor(LpHeader);
    assert(IRLp && "Could not find IR loop corresponding to VPLoop.");

    // Drop any existing opt-report metadata and re-add the opt-report tracked
    // for current VPLoop to corresponding IR loop. Remarks from prior
    // components are all captured in VPLoop's opt-report during VPlan CFG
    // construction.
    MDNode *NewLoopID = OptReport::replaceOptReportForLoopID(
        IRLp->getLoopID(), OR, *Plan->getLLVMContext());
    IRLp->setLoopID(NewLoopID);
  };

  for (VPLoop *OuterLoop : *Plan->getVPLoopInfo())
    for (auto *VLP : post_order(OuterLoop))
      LowerRemarksForLoop(VLP);
}

void VPOCodeGen::emitRemarksForScalarLoops() {
  auto RemoveOptReport = [this](Loop *Lp) {
    MDNode *NewLoopID = OptReport::eraseOptReportFromLoopID(
        Lp->getLoopID(), *Plan->getLLVMContext());
    Lp->setLoopID(NewLoopID);
  };

  for (auto &ScalarLoopPair : OutgoingScalarLoopHeaders) {
    auto *ScalarLpVPI = ScalarLoopPair.first;
    Loop *ScalarLp = LI->getLoopFor(ScalarLoopPair.second);
    assert(ScalarLp && "Loop not found.");

    // Remove all opt-reports from scalar loop nest. They have been moved to
    // vectorized loops.
    for (auto *Lp : post_order(ScalarLp))
      RemoveOptReport(Lp);

    // Emit remarks collected for scalar loop instruction into outgoing scalar
    // loop's opt-report.
    auto EmitScalarLpVPIRemarks = [this, ScalarLp](auto *LpVPI) {
      for (auto R : LpVPI->getOriginRemarks())
        ORBuilder(*ScalarLp, *LI).addOrigin(R.RemarkID);

      for (auto R : LpVPI->getGeneralRemarks())
        ORBuilder(*ScalarLp, *LI)
            .addRemark(R.MessageVerbosity, R.RemarkID, R.Arg);
    };

    if (auto *RemLp = dyn_cast<VPScalarRemainder>(ScalarLpVPI))
      EmitScalarLpVPIRemarks(RemLp);
    else
      EmitScalarLpVPIRemarks(cast<VPScalarPeel>(ScalarLpVPI));
  }
}

void VPOCodeGen::preserveLoopIDDbgMDs() {
  auto PreserveMDsForLoop = [this](VPLoop *VPL) {
    MDNode *LpID = VPL->getLoopID();
    if (!LpID)
      return;

    SmallVector<MDNode *, 2> DbgLocMDs;
    // Collect all DbgLoc metadata present in LoopID.
    for (unsigned i = 1, e = LpID->getNumOperands(); i < e; i++) {
      if (auto *DbgMD = dyn_cast<DILocation>(LpID->getOperand(i)))
        DbgLocMDs.push_back(DbgMD);
    }

    if (DbgLocMDs.empty())
      return;

    // Identify the IR loop that corresponds to current VPLoop.
    auto *VPLpHeader = VPL->getHeader();
    auto *LpHeader = cast<BasicBlock>(getScalarValue(VPLpHeader, 0));
    Loop *IRLp = LI->getLoopFor(LpHeader);
    assert(IRLp && "Could not find IR loop corresponding to VPLoop.");

    // Add the MDNodes to LoopID.
    MDNode *NewLoopID = makePostTransformationMetadata(
        IRLp->getHeader()->getContext(), IRLp->getLoopID(),
        {} /*No MDs to drop*/, DbgLocMDs);
    IRLp->setLoopID(NewLoopID);
  };

  for (VPLoop *OuterLoop : *Plan->getVPLoopInfo())
    for (auto *VLP : post_order(OuterLoop))
      PreserveMDsForLoop(VLP);
}
