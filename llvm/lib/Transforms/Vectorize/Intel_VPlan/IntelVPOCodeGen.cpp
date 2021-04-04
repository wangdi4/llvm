//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
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

static cl::opt<bool> VPlanUseDAForUnitStride(
    "vplan-use-da-unit-stride-accesses", cl::init(true), cl::Hidden,
    cl::desc("Use DA knowledge in VPlan for unit-stride accesses."));

static cl::opt<bool> PredicateSafeValueDivision(
    "vplan-predicate-safe-value-div", cl::init(false), cl::Hidden,
    cl::desc("Always serialize masked integer division, even if divisor is "
             "known to be safe for speculation."));

static void addBlockToParentLoop(Loop *L, BasicBlock *BB, LoopInfo &LI) {
  if (auto *ParentLoop = L->getParentLoop())
    ParentLoop->addBasicBlockToLoop(BB, LI);
}

// TODO: Consolidate and move this function to a IntelVPlanUtils.h
static bool LLVM_ATTRIBUTE_UNUSED isScalarPointeeTy(const VPValue *Val) {
  assert(isa<PointerType>(Val->getType()) &&
         "Expect a pointer-type argument to isScalarPointeeTy function.");
  Type *PointeeTy = Val->getType()->getPointerElementType();
  return (!(PointeeTy->isAggregateType() || PointeeTy->isVectorTy() ||
            PointeeTy->isPointerTy()));
}

/// Helper function to check if given VPValue has consecutive pointer stride(1
/// or -1) and return true if this is the case. IsNegOneStride is set to true if
/// stride is -1 and false otherwise.
static bool isVPValueConsecutivePtrStride(const VPValue *Ptr,
                                          const VPlanVector *Plan,
                                          bool &IsNegOneStride) {
  return cast<VPlanDivergenceAnalysis>(Plan->getVPlanDA())
      ->isUnitStridePtr(Ptr, IsNegOneStride);
}

// Variant of isVPValueConsecutivePtrStride where the client does not care about
// IsNegOneStride value.
static bool isVPValueConsecutivePtrStride(const VPValue *Ptr,
                                          const VPlanVector *Plan) {
  bool IsNegOneStride = false;
  return isVPValueConsecutivePtrStride(Ptr, Plan, IsNegOneStride);
}

/// Helper function to check if given VPValue is uniform based on DA.
static bool isVPValueUniform(VPValue *V, const VPlanVector *Plan) {
  return !cast<VPlanDivergenceAnalysis>(Plan->getVPlanDA())->isDivergent(*V);
}

/// Helper function to check if VPValue is linear and return linear step in \p
/// Step.
// TODO: Use new VPO legality infra to get this information.
static bool isVPValueLinear(VPValue *V, int *Step = nullptr) {
  if (Step)
    *Step = 0;
  return false;
}

/// Helper function that returns widened type of given type \p VPInstTy.
static Type *getVPInstVectorType(Type *VPInstTy, unsigned VF) {
  auto *VPInstVecTy = dyn_cast<VectorType>(VPInstTy);
  unsigned NumElts = VPInstVecTy ? VPInstVecTy->getNumElements() * VF : VF;
  return FixedVectorType::get(VPInstTy->getScalarType(), NumElts);
}

/// Return true if \p Var a variable identified for SOA-layout.
static bool isSOAAccess(const VPValue *Var, const VPlanVector *Plan) {
  return cast<VPlanDivergenceAnalysis>(Plan->getVPlanDA())->isSOAShape(Var);
}

/// Return true if \p Var has a SOA unit-stride access.
static bool isSOAUnitStride(const VPValue *Var, const VPlanVector *Plan) {
  return cast<VPlanDivergenceAnalysis>(Plan->getVPlanDA())
      ->isSOAUnitStride(Var);
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
                                unsigned CStep) {
  // We need to generate the expression for the part of the loop that the
  // vectorized body will execute. Step is equal to the vectorization factor
  // (number of SIMD elements) times the unroll factor (number of SIMD
  // instructions). If the Step is a power of 2, then the vector trip count is
  // calculated using the following formula: &(OrigTC, ~(Step-1)). Otherwise, we
  // substract the remainder of OrigTC/Step from OrigTC.
  auto *Step = ConstantInt::get(OrigTC->getType(), CStep);
  if (isPowerOf2_32(CStep))
    return Builder.CreateAnd(OrigTC,
                             ConstantInt::get(OrigTC->getType(), ~(CStep - 1)));
  auto *Rem = Builder.CreateURem(OrigTC, Step, "n.mod.vf");
  return Builder.CreateSub(OrigTC, Rem, "n.vec", /*HasNUW=*/true,
                           /*HasNSW=*/true);
}

Value *VPOCodeGen::generateSerialInstruction(VPInstruction *VPInst,
                                             ArrayRef<Value *> ScalarOperands) {
  SmallVector<Value *, 4> Ops(ScalarOperands.begin(), ScalarOperands.end());
  Value *SerialInst = nullptr;
  if (Instruction::isBinaryOp(VPInst->getOpcode())) {
    assert(ScalarOperands.size() == 2 &&
           "Binop VPInstruction has incorrect number of operands.");
    SerialInst = Builder.CreateBinOp(
        static_cast<Instruction::BinaryOps>(VPInst->getOpcode()), Ops[0],
        Ops[1]);
  } else if (Instruction::isUnaryOp(VPInst->getOpcode())) {
    assert(ScalarOperands.size() == 1 &&
           "Unop VPInstruction has incorrect number of operands.");
    SerialInst = Builder.CreateUnOp(
        static_cast<Instruction::UnaryOps>(VPInst->getOpcode()), Ops[0]);
  } else if (VPInst->getOpcode() == Instruction::Load) {
    assert(ScalarOperands.size() == 1 &&
           "Load VPInstruction has incorrect number of operands.");
    SerialInst = Builder.CreateLoad(Ops[0]);
    auto *NewLoad = cast<LoadInst>(SerialInst);
    auto *VPLoad = cast<VPLoadStoreInst>(VPInst);
    NewLoad->setVolatile(VPLoad->isVolatile());
    NewLoad->setOrdering(VPLoad->getOrdering());
    if (VPLoad->isAtomic())
      NewLoad->setSyncScopeID(VPLoad->getSyncScopeID());
    NewLoad->setAlignment(VPLoad->getAlignment());
  } else if (VPInst->getOpcode() == Instruction::Store) {
    assert(ScalarOperands.size() == 2 &&
           "Store VPInstruction has incorrect number of operands.");
    SerialInst = Builder.CreateStore(Ops[0], Ops[1]);
    auto *NewStore = cast<StoreInst>(SerialInst);
    auto *VPStore = cast<VPLoadStoreInst>(VPInst);
    NewStore->setVolatile(VPStore->isVolatile());
    NewStore->setOrdering(VPStore->getOrdering());
    if (VPStore->isAtomic())
      NewStore->setSyncScopeID(VPStore->getSyncScopeID());
    NewStore->setAlignment(VPStore->getAlignment());
  } else if (VPInst->getOpcode() == Instruction::Call) {
    assert(ScalarOperands.size() > 0 &&
           "Call VPInstruction should have atleast one operand.");
    if (auto *ScalarF = dyn_cast<Function>(Ops.back())) {
      Ops.pop_back();
      SerialInst = Builder.CreateCall(ScalarF, Ops);
    } else {
      // Indirect call (via function pointer).
      Value *FuncPtr = Ops.back();
      Ops.pop_back();

      Type *FuncPtrTy = FuncPtr->getType();
      assert(isa<PointerType>(FuncPtrTy) &&
             "Function pointer operand is not pointer type.");
      auto *FT = cast<FunctionType>(
          cast<PointerType>(FuncPtrTy)->getPointerElementType());

      SerialInst = Builder.CreateCall(FT, FuncPtr, Ops);
    }
    auto *SerialCall = cast<CallInst>(SerialInst);
    auto *VPCall = cast<VPCallInstruction>(VPInst);
    SerialCall->setCallingConv(VPCall->getOrigCallingConv());
    SerialCall->setAttributes(VPCall->getOrigCallAttrs());
    SerialCall->setTailCall(VPCall->isOrigTailCall());
  } else if (VPGEPInstruction *VPGEP = dyn_cast<VPGEPInstruction>(VPInst)) {
    assert(ScalarOperands.size() > 1 &&
           "VPGEPInstruction should have atleast two operands.");
    Value *GepBasePtr = Ops[0];
    Ops.erase(Ops.begin());
    SerialInst = Builder.CreateGEP(GepBasePtr, Ops);
    cast<GetElementPtrInst>(SerialInst)->setIsInBounds(VPGEP->isInBounds());
  } else if (VPInst->getOpcode() == Instruction::InsertElement) {
    assert(ScalarOperands.size() == 3 &&
           "InsertElement instruction should have three operands.");
    SerialInst = Builder.CreateInsertElement(Ops[0], Ops[1], Ops[2]);
  } else if (VPInst->getOpcode() == Instruction::ExtractElement) {
    assert(ScalarOperands.size() == 2 &&
           "ExtractElement instruction should have two operands.");
    SerialInst = Builder.CreateExtractElement(Ops[0], Ops[1]);
  } else if (VPInst->getOpcode() == Instruction::Alloca) {
    assert(ScalarOperands.size() == 1 &&
           "Alloca instruction should have one operand.");
    auto *Ty = cast<PointerType>(VPInst->getType());
    AllocaInst *SerialAlloca = Builder.CreateAlloca(
        Ty->getElementType(), Ty->getAddressSpace(), Ops[0]);
    // TODO: We don't represent alloca attributes in VPInstruction, so
    // underlying instruction must exist!
    auto *OrigAlloca = cast<AllocaInst>(VPInst->getUnderlyingValue());
    SerialAlloca->setAlignment(OrigAlloca->getAlign());
    SerialAlloca->setUsedWithInAlloca(OrigAlloca->isUsedWithInAlloca());
    SerialAlloca->setSwiftError(OrigAlloca->isSwiftError());
    SerialInst = SerialAlloca;
  } else if (VPInst->getOpcode() == Instruction::AtomicRMW) {
    assert(ScalarOperands.size() == 2 &&
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
    if (SerialAtomicRMW->isFloatingPointOperation())
      SerialAtomicRMW->setFastMathFlags(OrigAtomicRMW->getFastMathFlags());
    SerialInst = SerialAtomicRMW;
  } else if (VPInst->getOpcode() == Instruction::AtomicCmpXchg) {
    assert(ScalarOperands.size() == 3 &&
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
    // instruction. An assert on ScalarOperands size seems unnecessary till
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
    // instruction. An assert on ScalarOperands size seems unnecessary till
    // then.
    auto *OrigInsertValueInst =
        cast<InsertValueInst>(VPInst->getUnderlyingValue());
    assert(OrigInsertValueInst &&
           "Expect a valid underlying insertvalue instruction.");
    SerialInst = Builder.CreateInsertValue(Ops[0], Ops[1],
                                           OrigInsertValueInst->getIndices(),
                                           "serial.insertvalue");
  } else {
    LLVM_DEBUG(dbgs() << "VPInst: "; VPInst->dump());
    llvm_unreachable("Currently serialization of only binop instructions, "
                     "load, store, call, gep, insert/extract-element, alloca, "
                     "atomicrmw is supported.");
  }

  return SerialInst;
}

void VPOCodeGen::emitEndOfVectorLoop(Value *Count, Value *CountRoundDown) {
  // Add a check in the middle block to see if we have completed
  // all of the iterations in the first vector loop.
  // If (N - N%VF) == N, then we *don't* need to run the remainder.
  Value *CmpN = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_EQ, Count,
                                CountRoundDown, "cmp.n",
                                LoopMiddleBlock->getTerminator());
  ReplaceInstWithInst(
      LoopMiddleBlock->getTerminator(),
      BranchInst::Create(LoopExitBlock, LoopScalarPreHeader, CmpN));
}

void VPOCodeGen::emitVectorLoopEnteredCheck(Loop *L, BasicBlock *Bypass) {
  BasicBlock *BB = L->getLoopPreheader();
  assert(BB && "Loop does not have preheader block.");
  IRBuilder<> IBuilder(BB->getTerminator());
  Value *TC = getOrCreateVectorTripCount(L, IBuilder);

  // Now, compare the new count to zero. If it is zero skip the vector loop and
  // jump to the scalar loop.
  Value *Cmp = IBuilder.CreateICmpEQ(TC, Constant::getNullValue(TC->getType()),
                                    "cmp.zero");

  // Generate code to check that the loop's trip count that we computed by
  // adding one to the backedge-taken count will not overflow.
  BasicBlock *NewBB = BB->splitBasicBlock(BB->getTerminator(), "vector.ph");
  // Update dominator tree immediately if the generated block is a
  // LoopBypassBlock because SCEV expansions to generate loop bypass
  // checks may query it before the current function is finished.
  DT->addNewBlock(NewBB, BB);
  addBlockToParentLoop(L, NewBB, *LI);
  ReplaceInstWithInst(BB->getTerminator(),
                      BranchInst::Create(Bypass, NewBB, Cmp));
  LoopBypassBlocks.push_back(BB);
}

void VPOCodeGen::createEmptyLoop() {

  LoopScalarBody = OrigLoop->getHeader();
  BasicBlock *LoopPreHeader = OrigLoop->getLoopPreheader();
  LoopExitBlock = OrigLoop->getExitBlock();

  assert(LoopPreHeader && "Must have loop preheader");
  assert(LoopExitBlock && "Must have an exit block");

  if (Plan->hasExplicitRemainder()) {
    // There is an issue calling SCEV when we create TripCount instruction(s).
    // The call to SCEV should be done before any new basic block is created
    // otherwise it may create some additional instructions like lcssa phis
    // in the unexpected places, reflecting that not all basic blocks are linked
    // to their successors (at least the last one).
    IRBuilder<> LBuilder(LoopPreHeader->getTerminator());
    getOrCreateTripCount(OrigLoop, LBuilder);
    return;
  }
  // Create vector loop body.
  LoopVectorBody = LoopPreHeader->splitBasicBlock(
      LoopPreHeader->getTerminator(), "vector.body");

  // Middle block comes after vector loop is done. It contains reduction tail
  // and checks if we need a scalar remainder.
  LoopMiddleBlock = LoopVectorBody->splitBasicBlock(
      LoopVectorBody->getTerminator(), "middle.block");

  // Scalar preheader contains phi nodes with incoming from vector version and
  // vector loop bypass blocks.
  LoopScalarPreHeader = LoopMiddleBlock->splitBasicBlock(
      LoopMiddleBlock->getTerminator(), "scalar.ph");

  Loop *Lp = LI->AllocateLoop();

  // Initialize NewLoop member
  NewLoop = Lp;

  Loop *ParentLoop = OrigLoop->getParentLoop();

  // Insert the new loop into the loop nest and register the new basic blocks
  // before calling any utilities such as SCEV that require valid LoopInfo.
  if (ParentLoop) {
    ParentLoop->addChildLoop(Lp);
    ParentLoop->addBasicBlockToLoop(LoopScalarPreHeader, *LI);
    ParentLoop->addBasicBlockToLoop(LoopMiddleBlock, *LI);
  } else
    LI->addTopLevelLoop(Lp);

  Lp->addBasicBlockToLoop(LoopVectorBody, *LI);

  // Now, compare the new count to zero. If it is zero skip the vector loop and
  // jump to the scalar loop.
  emitVectorLoopEnteredCheck(Lp, LoopScalarPreHeader);

  // Find the loop boundaries.
  IRBuilder<> LBuilder(Lp->getLoopPreheader()->getTerminator());
  Value *Count = getOrCreateTripCount(Lp, LBuilder);
  // CountRoundDown is a counter for the vectorized loop.
  // CountRoundDown = Count - Count % VF.
  Value *CountRoundDown = getOrCreateVectorTripCount(Lp, LBuilder);
  // Add a check in the middle block to see if we have completed
  // all of the iterations in the first vector loop.
  // If (N - N%VF) == N, then we *don't* need to run the remainder.
  emitEndOfVectorLoop(Count, CountRoundDown);

  // Inform SCEV analysis to forget original loop
  PSE.getSE()->forgetLoop(OrigLoop);

  // Save the state.
  LoopVectorPreHeader = Lp->getLoopPreheader();

  // Get ready to start creating new instructions into the vector preheader.
  Builder.SetInsertPoint(&*LoopVectorPreHeader->getFirstInsertionPt());
}

void VPOCodeGen::unlinkOrigHeaderPhis() {
  BasicBlock *Header = OrigLoop->getHeader();
  for (auto &Phi: Header->phis())
    Phi.removeIncomingValue(OrigPreHeader, false);
}

void VPOCodeGen::finalizeLoop() {
  if (Plan->hasExplicitRemainder()) {
    // Fix phis.
    fixNonInductionVPPhis();

    if (!OrigLoopUsed)
      unlinkOrigHeaderPhis();

    // Attach the new loop to the original preheader
    auto *Plan = const_cast<VPlanVector *>(this->Plan);

    cast<BranchInst>(OrigPreHeader->getTerminator())
        ->setOperand(0, getScalarValue(
                            // FIXME: Better consts everywhere.
                            Plan->getEntryBlock(), 0));
    // Find last block in cfg.
    auto LastVPBB = find_if(*Plan, [](const VPBasicBlock &BB) {
      return BB.getNumSuccessors() == 0;
    });
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
  } else {
    fixOutgoingValues();
    fixNonInductionVPPhis();
    updateAnalysis();
    fixLCSSAPHIs();
    predicateInstructions();
  }

  DT->recalculate(*LoopVectorBody->getParent());
  LI->releaseMemory();
  LI->analyze(*DT);

  NewLoop = LI->getLoopFor(LoopVectorBody);
  OrigLoop = LI->getLoopFor(LoopScalarBody);
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
  unsigned NumElts = cast<VectorType>(Vec->getType())->getNumElements();
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
  int VLen = cast<VectorType>(Val->getType())->getNumElements();

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
Loop *VPOCodeGen::cloneScalarLoop(Loop *OrigLP, BasicBlock *NewLoopPred,
                                  BasicBlock *NewLoopSucc, const Twine &Name) {

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

  // Clone the loop.
  Loop *NewLoop = cloneLoopWithPreheader(
      NewLoopSucc,
      NewLoopPred /*Block that you want to dominate the new loop.*/,
      OrigLP /*The original loop.*/, VMap /*Value2Value Map*/, Name, LI, DT,
      ClonedLoopBlocks);

  // Adjust the target blocks in the newly cloned loops.
  remapInstructionsInBlocks(ClonedLoopBlocks, VMap);

  // Connect the NewLoopPred to the new-loop preheader.
  NewLoopPred->getTerminator()->replaceUsesOfWith(NewLoopSucc,
                                                  NewLoop->getLoopPreheader());

  // Connect the new loops exit to the provided  NewLoopSucc node.
  NewLoop->getLoopLatch()->getTerminator()->replaceUsesOfWith(
      OrigLP->getUniqueExitBlock(), NewLoopSucc);

  return NewLoop;
}

void VPOCodeGen::addMaskToSVMLCall(Function *OrigF, Value *CallMaskValue,
                                   AttributeList OrigAttrs,
                                   SmallVectorImpl<Value *> &VecArgs,
                                   SmallVectorImpl<Type *> &VecArgTys,
                                   SmallVectorImpl<AttributeSet> &VecArgAttrs) {
  assert(CallMaskValue && "Expected mask to be present");
  VectorType *VecTy = cast<VectorType>(VecArgTys[0]);
  assert(VecTy->getNumElements() ==
             cast<VectorType>(CallMaskValue->getType())->getNumElements() &&
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
  for (unsigned Idx = VF * GroupSize; Idx < WideValueType->getNumElements();
       ++Idx)
    ShuffleMask.push_back(VF);

  auto *False = ConstantInt::getFalse(MaskToUse->getType());

  return Builder.CreateShuffleVector(MaskToUse, False, ShuffleMask);
}

void VPOCodeGen::vectorizeInstruction(VPInstruction *VPInst) {
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
    // For consecutive load/store we create a scalar GEP.
    // TODO: Extend support for private pointers and VLS-based unit-stride
    // optimization.
    VPGEPInstruction *GEP = cast<VPGEPInstruction>(VPInst);
    auto IsAddrSpaceCast = [](VPValue *V) -> bool {
      auto *I = dyn_cast<VPInstruction>(V);
      return I && I->getOpcode() == Instruction::AddrSpaceCast;
    };

    auto SkipAddrSpaceCasts = [IsAddrSpaceCast](VPValue *V) {
      while (IsAddrSpaceCast(V))
        V = cast<VPInstruction>(V)->getOperand(0);
      return V;
    };

    // Check for load/stores users and addrspace casts with their users.
    // Check if vectorizable laod/store uses GEP's result or address space cast
    // as pointer operand.
    // TODO: When CG would rely completely on SVA, this traversal with a
    // worklist can be removed.
    SmallVector<VPUser *, 16> Worklist(GEP->user_begin(), GEP->user_end());
    bool CanBeScalar = true;
    while (!Worklist.empty() && CanBeScalar) {
      auto *UserI = cast<VPInstruction>(Worklist.pop_back_val());
      if (IsAddrSpaceCast(UserI)) {
        Worklist.insert(Worklist.end(), UserI->user_begin(), UserI->user_end());
        continue;
      } else if (isVectorizableLoadStore(UserI))
          if (SkipAddrSpaceCasts(getLoadStorePointerOperand(UserI)) == GEP)
            continue;

      CanBeScalar = false;
    }

    if ((CanBeScalar && isVPValueConsecutivePtrStride(GEP, Plan) &&
         VPlanUseDAForUnitStride) ||
        isSOAUnitStride(GEP, Plan)) {
      SmallVector<Value *, 6> ScalarOperands;
      for (unsigned Op = 0; Op < GEP->getNumOperands(); ++Op) {
        auto *ScalarOp = getScalarValue(GEP->getOperand(Op), 0 /*Lane*/);
        assert(ScalarOp && "Operand for scalar GEP not found.");
        ScalarOperands.push_back(ScalarOp);
      }

      Value *ScalarGep = generateSerialInstruction(GEP, ScalarOperands);
      StringRef GepName =
          isSOAAccess(GEP, Plan) ? "soa.scalar.gep" : "scalar.gep";
      ScalarGep->setName(GepName);
      VPScalarMap[GEP][0] = ScalarGep;
      break;
    }
    // Serialize if all users of GEP are uniform load/store.
    if (all_of(GEP->users(), [&](VPUser *U) -> bool {
          return getLoadStorePointerOperand(U) == GEP &&
                 isVPValueUniform(U, Plan);
        })) {
      serializeInstruction(GEP);
      return;
    }
    VPValue *GepBasePtr = GEP->getPointerOperand();
    bool AllGEPIndicesUniform =
        all_of(GEP->indices(), [&](VPValue *Op) -> bool {
          // TODO: Using DA for loop invariance.
          return isVPValueUniform(Op, Plan);
        });

    bool AllGEPOpsUniform =
        isVPValueUniform(GepBasePtr, Plan) && AllGEPIndicesUniform;

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

    // We check the pointer-operand and the indices seperately. For the
    // pointer-operand, decision on whether to use the scalar pointer or the
    // vector pointer depends on the pointer itself. For the decision to widen
    // the indices, we look at the the uniformity as well as whether the GEP
    // itself is an unit-stride GEP or not.

    // To correctly widen the  base-pointer, we check if all the operands of
    // the GEP are uniform. If they are uniform, we retain the scalar-pointer.
    // Another scenario where this is true is when we have pointer which are
    // SOA-unit stride. In case of SOA-unit stride pointer, we retain the
    // scalar-type pointer, typically <VF x Ty>*. Otherwise, we get the vector
    // version of the pointer, which is typically a vector of pointers, i.e.,
    // <VF x Ty*>.
    // TODO: When CG would rely completely on SVA, this check can be removed.

    // Widen the base-pointer.
    Value *WideGepBasePtr =
        AllGEPOpsUniform || isSOAUnitStride(GepBasePtr, Plan)
            ? getScalarValue(GepBasePtr, 0)
            : GetVectorOp(GepBasePtr);

    // Widen the indices.
    SmallVector<Value *, 4> OpsV;
    if (AllGEPOpsUniform)
      llvm::transform(GEP->indices(), std::back_inserter(OpsV),
                      [this](VPValue *Op) { return getScalarValue(Op, 0); });
    else
      llvm::transform(
          GEP->indices(), std::back_inserter(OpsV),
          [GetVectorOp](VPValue *Op) { return GetVectorOp(Op); });


    StringRef GepName =
        isSOAAccess(GEP, Plan) ? "soa_vectorGEP" : "mm_vectorGEP";
    Value *VectorGEP = Builder.CreateGEP(WideGepBasePtr, OpsV, GepName);
    cast<GetElementPtrInst>(VectorGEP)->setIsInBounds(GEP->isInBounds());

    // We need to bcast the scalar GEP to all lanes if all its operands were
    // uniform.
    if (AllGEPOpsUniform)
      VectorGEP = Builder.CreateVectorSplat(VF, VectorGEP);

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
    auto Opcode = static_cast<Instruction::CastOps>(VPInst->getOpcode());

    /// Vectorize casts.
    Type *ScalTy = VPInst->getType();
    Type *VecTy = getWidenedType(ScalTy, VF);
    VPValue *ScalOp = VPInst->getOperand(0);
    Value *VecOp = getVectorValue(ScalOp);
    VPWidenMap[VPInst] = Builder.CreateCast(Opcode, VecOp, VecTy);
    return;
  }

  case Instruction::FNeg: {
    if (isVPValueUniform(VPInst, Plan)) {
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
    bool DivisorIsSafe = false;
    auto *Const = dyn_cast<VPConstant>(VPInst->getOperand(1));
    if (!PredicateSafeValueDivision && Const && Const->isConstantInt()) {
      int64_t Val = Const->getSExtValue();
      if (Val != 0 && Val != -1)
        DivisorIsSafe = true;
    }
    if (MaskValue && !DivisorIsSafe) {
      if (isVPValueUniform(VPInst, Plan))
        serializePredicatedUniformInstruction(VPInst);
      else
        serializeWithPredication(VPInst);
      return;
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

    if (isVPValueUniform(VPInst, Plan)) {
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
    vectorizeLoadInstruction(VPInst, true);
    return;
  }
  case Instruction::Store: {
    vectorizeStoreInstruction(VPInst, true);
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
    // Handle lifetime_start/end intrinsics operating on private-memory.
    // We use the following mechanism to handle the intrinsic:
    // If the array-private is widened (AOS/SOA) and not serialized, do not
    // serialize the intrinsic. Just use the widened-alloca pointer, i.e., its
    // cast'ed version ( correct casting is handled in then vectorizeCast
    // function), and pass it to the intrinsic. Along with this, if the first
    // argument in the original call is not -1 (used to denote variable-size),
    // compute the size of the widened copy and pass it to the intrinsic.
    // If the array-private is not widened, and serialized, just bypass this
    // block and serialize this call.
    if (VPCall->isLifetimeStartOrEndIntrinsic()) {
      vectorizeLifetimeStartEndIntrinsic(VPCall);
      return;
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
      VectorVariant *MatchedVariant =
          const_cast<VectorVariant *>(VPCall->getVectorVariant());
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
      // serialization.
      serializeWithPredication(VPCall);
      ++OptRptStats.SerializedCalls;
      return;
    }
    default:
      llvm_unreachable(
          "VPCallInstruction does not have a valid decision for VF.");
    }
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

    // Broadcast the compare and set as the widened value.
    auto *V = Builder.CreateVectorSplat(VF, CmpInst, "broadcast");
    VPWidenMap[VPInst] = V;
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

    Value *Identity = getVectorValue(VPInst->getOperand(0));
    if (VPInst->getNumOperands() > 1) {
      auto *StartVPVal = VPInst->getOperand(1);
      assert((isa<VPExternalDef>(StartVPVal) || isa<VPConstant>(StartVPVal)) &&
             "Unsupported reduction StartValue");
      auto *StartVal = getScalarValue(StartVPVal, 0);
      Identity = Builder.CreateInsertElement(
          Identity, StartVal, Builder.getInt32(0), "red.init.insert");
    }
    VPWidenMap[VPInst] = Identity;
    return;
  }
  case VPInstruction::ReductionFinal: {
    vectorizeReductionFinal(cast<VPReductionFinal>(VPInst));
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
    VectorTripCount = calculateVectorTC(OrigTC, Builder, UF * VF);
    VPScalarMap[VPInst][0] = VectorTripCount;
    // Meanwhile, assert that the UF implicitly used by the CG is the same as
    // represented explicitly.
    assert(VTCCalc->getUF() == UF && "Mismatch in UFs!");
    return;
  }
  case VPInstruction::ActiveLane: {
    assert(!MaskValue && "ActiveLane calculation is expected to be unmasked!");
    assert(isVPValueUniform(VPInst, Plan) &&
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
    VPScalarMap[VPInst][0] =
        Builder.CreateExtractElement(getVectorValue(VPInst->getOperand(0)),
                                     getScalarValue(VPInst->getOperand(1), 0));
    return;
  }
  case VPInstruction::PushVF: {
    unsigned NewVF = cast<VPPushVF>(VPInst)->getVF();
    unsigned NewUF = cast<VPPushVF>(VPInst)->getUF();
    assert((NewVF != 0 && NewUF != 0) && "expected nonzero VF and UF");
    VFStack.emplace_back(VF, UF);
    VF = NewVF;
    UF = NewUF;
    return;
  }
  case VPInstruction::PopVF: {
    assert(!VFStack.empty() && "unexpected PopVF");
    auto V = VFStack.pop_back_val();
    VF = V.first;
    UF = V.second;
    return;
  }
  case Instruction::Br:
    // Do nothing.
    return;
  case VPInstruction::ScalarRemainder: {
    auto *LoopReuse = cast<VPScalarRemainder>(VPInst);
    // Make the current block predecessor of the original loop header.
    ReplaceInstWithInst(Builder.GetInsertBlock()->getTerminator(),
                        BranchInst::Create(LoopReuse->getLoop()->getHeader()));
    // Replace operands (original incoming values) with the new ones from VPlan.
    // This includes the exit block.
    for (unsigned Idx = 0; Idx < LoopReuse->getNumOperands(); ++Idx) {
      Use *OrigUse = LoopReuse->getOrigUse(Idx);
      OrigUse->set(getScalarValue(LoopReuse->getOperand(Idx), 0));
      if (auto *Phi = dyn_cast<PHINode>(OrigUse->getUser()))
        Phi->setIncomingBlock(OrigUse->getOperandNo(),
                              Builder.GetInsertBlock());
    }
    OrigLoopUsed = true;
    return;
  }
  case VPInstruction::OrigLiveOut: {
    auto *LiveOut = cast<VPOrigLiveOut>(VPInst);
    VPScalarMap[LiveOut][0] = const_cast<Value*>(LiveOut->getLiveOutVal());
    return;
  }
  case VPInstruction::PrivateFinalUncondMem:
  case VPInstruction::PrivateFinalUncond: {
    vectorizePrivateFinalUncond(VPInst);
    return;
  }
  case VPInstruction::VLSLoad: {
    auto *VLSLoad = cast<VPVLSLoad>(VPInst);
    assert(isVPValueUniform(VLSLoad, Plan) &&
           "VLSLoad must produce a uniform value!");
    auto *Base = getScalarValue(VLSLoad->getOperand(0), 0);
    auto *VecTy = cast<VectorType>(VLSLoad->getType());
    auto *CastedBase = Builder.CreateBitCast(
        Base, VecTy->getPointerTo(
                  cast<PointerType>(Base->getType())->getAddressSpace()));
    auto GroupSize = VLSLoad->getGroupSize();
    if (VecTy->getNumElements() == VF * GroupSize && !MaskValue) {
      auto *WideLoad = cast<LoadInst>(Builder.CreateAlignedLoad(
          CastedBase, VLSLoad->getAlignment(), "vls.load"));
      VPScalarMap[VLSLoad][0] = WideLoad;
      OptRptStats.UnmaskedVLSLoads += VLSLoad->getNumOrigLoads();
      return;
    }

    auto *LoadMask = getVLSLoadStoreMask(VecTy, GroupSize);
    auto *WideLoad =
        Builder.CreateMaskedLoad(CastedBase, VLSLoad->getAlignment(), LoadMask,
                                 nullptr /* PassThru */, "vls.load");
    VPScalarMap[VLSLoad][0] = WideLoad;
    OptRptStats.MaskedVLSLoads += VLSLoad->getNumOrigLoads();
    return;
  }
  case VPInstruction::VLSExtract: {
    auto *Extract = cast<VPVLSExtract>(VPInst);
    assert(isVPValueUniform(Extract->getOperand(0), Plan) &&
           "Operand of VLSExtract must be a uniform value!");

    auto NumEltsPerValue = Extract->getNumGroupEltsPerValue();

    auto Offset = Extract->getOffset();
    auto GroupSize = Extract->getGroupSize();
    SmallVector<int, 32> ShuffleMask;
    for (unsigned Lane = 0; Lane < VF; ++Lane)
      for (unsigned Part = 0; Part < NumEltsPerValue; ++Part)
        ShuffleMask.push_back(Lane * GroupSize + Offset + Part);

    auto *WideValue = getScalarValue(Extract->getOperand(0), 0);
    auto *ExtractedData =
        Builder.CreateShuffleVector(WideValue, WideValue, ShuffleMask);
    auto *ResultType = getWidenedType(Extract->getType(), VF);
    auto *Result = Builder.CreateBitCast(ExtractedData, ResultType);
    Result->setName(Extract->getName());
    VPWidenMap[Extract] = Result;
    return;
  }
  case VPInstruction::VLSInsert: {
    auto *Insert = cast<VPVLSInsert>(VPInst);
    assert(isVPValueUniform(Insert, Plan) && "VLSInsert must produce a uniform value!");
    assert(isVPValueUniform(Insert->getOperand(0), Plan) &&
           "Orig wide value operand of VLSInsert must be a uniform vlaue!");
    auto *OrigWideValue = getScalarValue(Insert->getOperand(0), 0);
    auto *ValueToInsert = getVectorValue(Insert->getOperand(1));

    auto NumEltsPerValue = Insert->getNumGroupEltsPerValue();
    auto *GroupType = cast<VectorType>(Insert->getOperand(0)->getType());
    Type *GroupEltType = GroupType->getElementType();
    auto NumEltsInGroup = GroupType->getNumElements();
    auto *Casted = Builder.CreateBitCast(
        ValueToInsert, getWidenedType(GroupEltType, VF * NumEltsPerValue));

    auto *CastedExtended = extendVector(Casted, NumEltsInGroup, Builder);

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
    assert(isVPValueUniform(VLSStore->getOperand(0), Plan) &&
           "Value operand of VLSStore must be uniform!");
    auto *Base = getScalarValue(VLSStore->getOperand(1), 0);
    auto *StoredValue = getScalarValue(VLSStore->getOperand(0), 0);
    auto *VecTy = cast<VectorType>(VLSStore->getOperand(0)->getType());
    auto *CastedBase = Builder.CreateBitCast(
        Base, VecTy->getPointerTo(
                  cast<PointerType>(Base->getType())->getAddressSpace()));
    auto GroupSize = VLSStore->getGroupSize();
    if (VecTy->getNumElements() == VF * GroupSize && !MaskValue) {
      auto *WideStore = cast<StoreInst>(Builder.CreateAlignedStore(
          StoredValue, CastedBase, VLSStore->getAlignment()));
      (void)WideStore;
      OptRptStats.UnmaskedVLSStores += VLSStore->getNumOrigStores();
      return;
    }

    auto *StoreMask = getVLSLoadStoreMask(VecTy, GroupSize);
    auto *WideStore = Builder.CreateMaskedStore(
        StoredValue, CastedBase, VLSStore->getAlignment(), StoreMask);
    (void)WideStore;
    OptRptStats.MaskedVLSStores += VLSStore->getNumOrigStores();
    return;
  }
  default: {
    LLVM_DEBUG(dbgs() << "VPInst: "; VPInst->dump());
    llvm_unreachable("VPVALCG: Opcode not uplifted yet.");
  }
  }
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

void VPOCodeGen::fixLCSSAPHIs() {
  for (Instruction &LEI : *LoopExitBlock) {
    auto *LCSSAPhi = dyn_cast<PHINode>(&LEI);
    if (!LCSSAPhi)
      break;
    if (LCSSAPhi->getNumIncomingValues() == 1)
      LCSSAPhi->addIncoming(UndefValue::get(LCSSAPhi->getType()),
                            LoopMiddleBlock);
  }
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
      cast<VectorType>(ShufMaskConst->getType())->getNumElements();
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
  return Builder.CreateGEP(Base, Cv, PtrToVec->getName() + ".base.addr");
}

template <typename CastInstTy>
void VPOCodeGen::vectorizeCast(
    typename std::enable_if<
        std::is_same<CastInstTy, BitCastInst>::value ||
            std::is_same<CastInstTy, AddrSpaceCastInst>::value,
        VPInstruction>::type *VPInst) {

  bool IsScalarized = false;
  Value *WidenedOp = nullptr;
  Type *WidenedTy = nullptr;
  Value *WidenedCast = nullptr;
  bool IsBitCastInst = std::is_same<CastInstTy, BitCastInst>::value;
  bool IsNonSerializedAllocaPointer = LoopPrivateVPWidenMap.count(
      VPInst->getOperand(0)); // Is this AOS-widened ptr,
                              // TODO: Add SOA-pointer check here.
  bool IsOnlyUsedInLifetimeIntrinsics =
      all_of(VPInst->users(),
             [&](VPValue *V) { // All users of this cast should be the
                               // lifetime_start and lifetime_end intrinsics.
               if (auto *VPCall = dyn_cast<VPCallInstruction>(V))
                 return VPCall->isLifetimeStartOrEndIntrinsic();
               return false;
             });

  // If the pointer is a bitcast and is exclusively used in
  // lifetime_start/end intrinsics, use the correct operands.
  if (IsBitCastInst && IsNonSerializedAllocaPointer &&
      IsOnlyUsedInLifetimeIntrinsics) {
    WidenedOp = LoopPrivateVPWidenMap[VPInst->getOperand(0)];
    WidenedTy = VPInst->getType();
    IsScalarized = true;
  } else if (!IsBitCastInst && VPScalarMap.count(VPInst->getOperand(0)) &&
             !VPWidenMap.count(VPInst->getOperand(0)) &&
             !isSerialized(VPInst->getOperand(0))) {
    // For addrspace cast check if operand is a scalar already
    // TODO: Replace with proper SVA check
    WidenedOp = getScalarValue(VPInst->getOperand(0), 0 /* Lane */);
    WidenedTy = VPInst->getType();
    IsScalarized = true;
  } else {
    WidenedOp = getVectorValue(VPInst->getOperand(0));
    WidenedTy = getVPInstVectorType(VPInst->getType(), VF);
  }

  // Create the widened-cast instruction.
  WidenedCast =
      Builder.CreateCast(static_cast<Instruction::CastOps>(VPInst->getOpcode()),
                         WidenedOp, WidenedTy);

  if (IsScalarized)
    VPScalarMap[VPInst][0] = WidenedCast;
  else
    VPWidenMap[VPInst] = WidenedCast;
}

void VPOCodeGen::vectorizeOpenCLSinCos(VPCallInstruction *VPCall,
                                       bool IsMasked) {
  // If we encounter a call to OpenCL sincos function, i.e., a call to
  // _Z6sincosfPf, the code in Intel_SVMLEmitter.cpp, currently maps that call
  // to _Z14sincos_ret2ptrDv<VF>_fPS_S1_ variant. The following code correctly
  // sets up the input/output arguments for that function.  '_Z6sincosfPf' has
  // the form,
  //
  // %sinVal = call float @_Z6sincosfPf(float %input, float* %cosPtr)
  // %cosVal = load float, float* %cosPtr
  //
  // The following code replaces that function call with
  // %16 = call <8 x float> @_Z14sincos_ret2ptrDv8_fPS_S1_(<8 x float>
  //                                                       %wide.input,
  //                                                       <8 x float>*
  //                                                       %sinPtr.vec,
  //                                                       <8 x float>*
  //                                                       %cosPtr.vec)
  // %wide.sin.InitVal = load <8 x float>, <8 x float>* %SinPtr.vec
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
  AllocaInst *WideCosPtr = cast<AllocaInst>(LoopPrivateVPWidenMap[CosPtr]);
  Instruction *WideSinPtr = WideCosPtr->clone();
  WideSinPtr->insertAfter(WideCosPtr);
  WideSinPtr->setName("sinPtr.vec");
  VecArgs.push_back(Arg1);
  VecArgs.push_back(WideSinPtr);
  VecArgs.push_back(WideCosPtr);
  VecArgTys.push_back(Arg1->getType());
  VecArgTys.push_back(WideSinPtr->getType());
  VecArgTys.push_back(WideCosPtr->getType());

  Function *CalledFunc = VPCall->getCalledFunction();
  assert(CalledFunc && "Unexpected null call function.");
  Function *VectorF =
      getOrInsertVectorFunction(CalledFunc, VF, VecArgTys, TLI,
                                Intrinsic::not_intrinsic, nullptr, IsMasked);
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

  Value *WideSinLoad = Builder.CreateAlignedLoad(
      WideSinPtr, cast<AllocaInst>(WideCosPtr)->getAlign(), "wide.sin.InitVal");
  VPWidenMap[VPCall] = WideSinLoad;
}

void VPOCodeGen::vectorizeCallArgs(VPCallInstruction *VPCall,
                                   VectorVariant *VecVariant,
                                   Intrinsic::ID VectorIntrinID,
                                   unsigned PumpPart, unsigned PumpFactor,
                                   SmallVectorImpl<Value *> &VecArgs,
                                   SmallVectorImpl<Type *> &VecArgTys,
                                   SmallVectorImpl<AttributeSet> &VecArgAttrs) {
  unsigned PumpedVF = VF / PumpFactor;
  std::vector<VectorKind> Parms;
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
        !hasVectorInstrinsicScalarOpd(VectorIntrinID, OrigArgIdx)) {
      // This is a vector call arg, so vectorize it.
      VPValue *Arg = VPCall->getOperand(OrigArgIdx);

      // Generate the right mask for OpenCL vector 'select' intrinsic
      if (isOpenCLSelectMask(FnName, OrigArgIdx))
        return getOpenCLSelectVectorMask(Arg);

      Value *VecArg = getVectorValue(Arg);
      VecArg = generateExtractSubVector(VecArg, PumpPart, PumpFactor, Builder);
      assert(VecArg && "Vectorized call arg cannot be nullptr.");

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

  // glibc scalar sincos function has 2 pointer out parameters, but SVML sincos
  // functions return the results directly in a struct. The pointers should be
  // omitted in vectorized call.
  if ((FnName == "sincos" || FnName == "sincosf") && !VecVariant)
    NumArgOperands -= 2;

  for (unsigned OrigArgIdx = VPCall->isIntelIndirectCall() ? 1 : 0,
                ParamsIdx = 0;
       OrigArgIdx < NumArgOperands; OrigArgIdx++, ParamsIdx++) {
    if (isOpenCLReadChannelDest(FnName, OrigArgIdx))
      continue;

    Value *VecArg = ProcessCallArg(OrigArgIdx, ParamsIdx);
    VecArgs.push_back(VecArg);
    VecArgTys.push_back(VecArg->getType());
    VecArgAttrs.push_back(Attrs.getParamAttributes(ParamsIdx));
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
                                     VectorVariant *VecVariant,
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

  // Promote to characteristic type.
  Type *CharacteristicType =
      VPlanCallVecDecisions::calcCharacteristicType(VPCall, *VecVariant);
  unsigned CharacteristicTypeSize =
      CharacteristicType->getPrimitiveSizeInBits();

  // Promote the i1 to an integer type that has the same size as the
  // characteristic type.
  Type *ScalarToType =
      IntegerType::get(MaskTy->getContext(), CharacteristicTypeSize);
  VectorType *VecToType = FixedVectorType::get(ScalarToType, PumpedVF);
  Value *MaskExt = Builder.CreateSExt(MaskToUse, VecToType, "maskext");

  // Bitcast if the promoted type is not the same as the characteristic
  // type.
  if (ScalarToType != CharacteristicType) {
    Type *MaskCastTy = FixedVectorType::get(CharacteristicType, PumpedVF);
    Value *MaskCast = Builder.CreateBitCast(MaskExt, MaskCastTy, "maskcast");
    VecArgs.push_back(MaskCast);
    VecArgTys.push_back(MaskCastTy);
  } else {
    VecArgs.push_back(MaskExt);
    VecArgTys.push_back(VecToType);
  }
}

void VPOCodeGen::vectorizeSelectInstruction(VPInstruction *VPInst) {
  // If the selector is loop invariant we can create a select
  // instruction with a scalar condition. Otherwise, use vector-select.
  VPValue *Cond = VPInst->getOperand(0);
  Value *VCond = getVectorValue(Cond);
  Value *Op0 = getVectorValue(VPInst->getOperand(1));
  Value *Op1 = getVectorValue(VPInst->getOperand(2));
  auto *VPInstVecTy = dyn_cast<VectorType>(VPInst->getType());

  // TODO: Using DA for loop invariance.
  bool UniformCond = isVPValueUniform(Cond, Plan);

  // The condition can be loop invariant  but still defined inside the
  // loop. This means that we can't just use the original 'cond' value.

  if (UniformCond) {
    // TODO: Handle uniform vector condition in selects.
    assert(!Cond->getType()->isVectorTy() &&
           "Uniform vector condition is not supported.");
    VCond = getScalarValue(Cond, 0);
  } else if (!Cond->getType()->isVectorTy() && VPInstVecTy) {
    unsigned OriginalVL = VPInstVecTy->getNumElements();
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

  Value *NewSelect = Builder.CreateSelect(VCond, Op0, Op1);

  VPWidenMap[VPInst] = NewSelect;
}

Align VPOCodeGen::getOriginalLoadStoreAlignment(const VPInstruction *VPInst) {
  assert((VPInst->getOpcode() == Instruction::Load ||
          VPInst->getOpcode() == Instruction::Store) &&
         "Alignment helper called on non load/store instruction.");
  // TODO: Using align 1 for new loads/stores introduced by VPlan-to-VPlan
  // transforms.
  if (VPInst->getUnderlyingValue() == nullptr)
    return Align(1);

  const DataLayout &DL = OrigLoop->getHeader()->getModule()->getDataLayout();
  Type *OrigTy = getLoadStoreType(VPInst);

  // Absence of alignment means target abi alignment. We need to use the
  // scalar's target abi alignment in such a case.
  return DL.getValueOrABITypeAlignment(
      cast<VPLoadStoreInst>(VPInst)->getAlignment(), OrigTy);
}

Align VPOCodeGen::getAlignmentForGatherScatter(const VPInstruction *VPInst) {
  assert((VPInst->getOpcode() == Instruction::Load ||
          VPInst->getOpcode() == Instruction::Store) &&
         "Alignment helper called on non load/store instruction.");

  Align Alignment = getOriginalLoadStoreAlignment(VPInst);

  Type *OrigTy = getLoadStoreType(VPInst);
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

Value *VPOCodeGen::getOrCreateWideLoadForGroup(OVLSGroup *Group) {
  auto FoundIter = VLSGroupLoadMap.find(Group);
  if (FoundIter != VLSGroupLoadMap.end())
    return FoundIter->second;

  // Check if the group is valid for this VF.
  assert(Group->getNumElems() == VF &&
         "Group number of elements must match VF");

  // The wide load can be based on InsertPoint only. Any other memory reference
  // in the group (e.g. Group->getFirstMemref()) may be not ready at this point
  // in code generation yet.
  OVLSMemref *InsertPoint = Group->getInsertPoint();
  int InterleaveIndex = computeInterleaveIndex(InsertPoint, Group);
  const VPInstruction *Leader =
      cast<VPVLSClientMemref>(InsertPoint)->getInstruction();

  Type *SingleAccessType = getLoadStoreType(Leader);
  Type *GroupType = getWidenedType(SingleAccessType, VF * Group->size());

  Value *GatherAddress = getVectorValue(Leader->getOperand(0));
  Value *ScalarAddress = Builder.CreateExtractElement(
      GatherAddress, (uint64_t)0, GatherAddress->getName() + "_0");

  // If the group leader (lexically first load) does not access the lowest
  // memory address address (InterleaveIndex != 0), we should adjust
  // ScalarAddress to make it point to the beginning of the Group.
  if (InterleaveIndex != 0)
    ScalarAddress = Builder.CreateConstInBoundsGEP1_64(
        ScalarAddress, -InterleaveIndex, "groupStart");

  auto AddressSpace =
      cast<PointerType>(ScalarAddress->getType())->getAddressSpace();
  Value *GroupPtr = Builder.CreateBitCast(
      ScalarAddress, GroupType->getPointerTo(AddressSpace), "groupPtr");

  // The alignment for the wide load needs to be set using the group's first
  // memory(lowest offset) reference.
  const VPInstruction *FirstGroupInst =
      (cast<VPVLSClientMemref>(Group->getFirstMemref()))->getInstruction();
  Align Alignment = getOriginalLoadStoreAlignment(FirstGroupInst);

  Instruction *GroupLoad;
  if (MaskValue) {
    auto *SingleAccessVecType = dyn_cast<VectorType>(SingleAccessType);
    auto OriginalVL =
        SingleAccessVecType ? SingleAccessVecType->getNumElements() : 1;
    Value *LoadMask = replicateVectorElts(MaskValue, OriginalVL * Group->size(),
                                          Builder, "groupLoadMask");
    OptRptStats.MaskedVLSLoads += Group->size();
    GroupLoad = Builder.CreateMaskedLoad(GroupPtr, Alignment, LoadMask, nullptr,
                                         "groupLoad");
  } else {
    OptRptStats.UnmaskedVLSLoads += Group->size();
    GroupLoad =
        Builder.CreateAlignedLoad(GroupType, GroupPtr, Alignment, "groupLoad");
  }

  DEBUG_WITH_TYPE(
      "ovls", dbgs() << "Emitted a group-wide vector LOAD for Group#"
                     << Group->getDebugId() << ":\n  " << *GroupLoad << "\n\n");

  VLSGroupLoadMap.insert(std::make_pair(Group, GroupLoad));
  return GroupLoad;
}

Value *VPOCodeGen::vectorizeInterleavedLoad(VPInstruction *VPLoad,
                                            OVLSGroup *Group) {
  auto MemrefIter = find_if(*Group, [VPLoad](OVLSMemref *Iter) {
    return cast<VPVLSClientMemref>(Iter)->getInstruction() == VPLoad;
  });
  assert(MemrefIter != Group->end() &&
         "Instruction does not belong to the group");
  auto *Memref = cast<VPVLSClientMemref>(*MemrefIter);

  auto InterleaveIndex = computeInterleaveIndex(Memref, Group);
  auto InterleaveFactor = computeInterleaveFactor(Memref);
  assert(InterleaveIndex < InterleaveFactor &&
         "InterleaveIndex must be less than InterleaveFactor");

  assert((Group->getInsertPoint() == Memref ||
          VLSGroupLoadMap.find(Group) != VLSGroupLoadMap.end()) &&
         "Wide load must be emitted at the group insertion point");
  Value *GroupLoad = getOrCreateWideLoadForGroup(Group);

  // No shuffling is needed for unit-stride loads.
  if (InterleaveFactor == 1)
    return GroupLoad;

  // Extract a proper widened value from the wide load. Generally, an extraction
  // mask looks like <0, 2, 4, 6>, but in case of vector input types (for
  // example, <2 x i32>) we would need somewhat more sophisticated mask:
  // <0, 1, 4, 5, 8, 9, 12, 13>.
  Type *GroupLeaderTy = cast<VPVLSClientMemref>(Group->getInsertPoint())
                            ->getInstruction()
                            ->getType();
  auto *GroupLeaderVecTy = dyn_cast<VectorType>(GroupLeaderTy);
  unsigned OriginalVL =
      GroupLeaderVecTy ? GroupLeaderVecTy->getNumElements() : 1;
  SmallVector<int, 64> ShuffleMask =
      createVectorStrideMask(InterleaveIndex, InterleaveFactor, VF, OriginalVL);
  Value *GroupShuffle = Builder.CreateShuffleVector(
      GroupLoad, UndefValue::get(GroupLoad->getType()), ShuffleMask,
      "groupShuffle");
  return Builder.CreateBitCast(
      GroupShuffle, getWidenedType(Memref->getInstruction()->getType(), VF),
      "groupCast");
}

Value *VPOCodeGen::vectorizeUnitStrideLoad(VPInstruction *VPInst,
                                           bool IsNegOneStride, bool IsPvtPtr) {
  Instruction *WideLoad = nullptr;
  VPValue *Ptr = getLoadStorePointerOperand(VPInst);
  Type *LoadType = getLoadStoreType(VPInst);
  auto *LoadVecType = dyn_cast<VectorType>(LoadType);
  unsigned OriginalVL = LoadVecType ? LoadVecType->getNumElements() : 1;
  Align Alignment = getOriginalLoadStoreAlignment(VPInst);
  Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(Ptr, IsNegOneStride);

  // Masking not needed for privates.
  // TODO: This needs to be generalized for all "dereferenceable" pointers
  // identified in incoming LLVM-IR. Check CMPLRLLVM-10714.
  if (MaskValue && !IsPvtPtr) {
    // Replicate the mask if VPInst is a vector instruction.
    Value *RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                              "replicatedMaskElts.");
    // We need to reverse the mask for -1 stride.
    if (IsNegOneStride)
      RepMaskValue = reverseVector(RepMaskValue, OriginalVL);

    ++OptRptStats.MaskedUnalignedUnitStrideLoads;
    WideLoad = Builder.CreateMaskedLoad(VecPtr, Alignment, RepMaskValue,
                                        nullptr, "wide.masked.load");
  } else {
    ++OptRptStats.UnmaskedUnalignedUnitStrideLoads;
    WideLoad = Builder.CreateAlignedLoad(VecPtr, Alignment, "wide.load");
  }

  if (auto *DynPeeling =
          dyn_cast_or_null<VPlanDynamicPeeling>(PreferredPeeling))
    if (VPInst == DynPeeling->memref())
      attachPreferredAlignmentMetadata(WideLoad, DynPeeling->targetAlignment());

  if (IsNegOneStride) // Reverse
    return reverseVector(WideLoad);
  return WideLoad;
}

void VPOCodeGen::vectorizeLoadInstruction(VPInstruction *VPInst,
                                          bool EmitIntrinsic) {
  Type *LoadType = VPInst->getType();
  auto *LoadVecType = dyn_cast<VectorType>(LoadType);
  assert((!LoadVecType || LoadVecType->getElementType()->isSingleValueType()) &&
         "Re-vectorization supports simple vectors only!");

  // Pointer operand of Load is always the first operand.
  VPValue *Ptr = VPInst->getOperand(0);
  int LinStride = 0;

  // Loads that are non-vectorizable should be serialized.
  if (!isVectorizableLoadStore(VPInst)) {
    return serializeWithPredication(VPInst);
  }

  // Handle vectorization of a linear value load.
  if (isVPValueLinear(Ptr, &LinStride)) {
    llvm_unreachable("VPVALCG: Vectorization of linear load not uplifted.");
#if 0
    vectorizeLinearLoad(Inst, LinStride);
    return;
#endif
  }

  // TODO: Using DA for loop invariance.
  if (isVPValueUniform(Ptr, Plan)) {
    if (MaskValue)
      serializePredicatedUniformInstruction(VPInst);
    else
      serializeInstruction(VPInst);
    return;
  }

  unsigned OriginalVL = LoadVecType ? LoadVecType->getNumElements() : 1;

  Value *NewLI = nullptr;

  // Try to handle consecutive loads without VLS.
  if (VPlanUseDAForUnitStride) {
    bool IsNegOneStride = false;
    bool ConsecutiveStride =
        isVPValueConsecutivePtrStride(Ptr, Plan, IsNegOneStride);
    if (ConsecutiveStride) {
      bool IsPvtPtr = getVPValuePrivateMemoryPtr(Ptr) != nullptr;
      NewLI = vectorizeUnitStrideLoad(VPInst, IsNegOneStride, IsPvtPtr);
      VPWidenMap[VPInst] = NewLI;
      return;
    }
  }

  // Try to do GATHER-to-SHUFFLE optimization.
  if (OVLSGroup *Group = VLSA->getGroupsFor(Plan, VPInst)) {
    Optional<int64_t> GroupStride = Group->getConstStride();
    assert(GroupStride && "Indexed loads are not supported");
    // Groups with gaps are not supported either.
    APInt AccessMask = Group->computeByteAccessMask();
    if (AccessMask.isAllOnesValue() && AccessMask.getBitWidth() == *GroupStride)
      NewLI = vectorizeInterleavedLoad(VPInst, Group);
  }

  // If VLS failed to emit a wide load, we have to emit a GATHER instruction.
  if (!NewLI) {
    // Replicate the mask if VPInst is a vector instruction originally.
    Value *RepMaskValue = nullptr;
    if (MaskValue)
      RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                         "replicatedMaskElts.");
    Value *GatherAddress = getWidenedAddressForScatterGather(Ptr);
    Align Alignment = getAlignmentForGatherScatter(VPInst);
    ++(RepMaskValue ? OptRptStats.MaskedGathers : OptRptStats.UnmaskedGathers);
    NewLI = Builder.CreateMaskedGather(GatherAddress, Alignment, RepMaskValue,
                                       nullptr, "wide.masked.gather");
  }

  VPWidenMap[VPInst] = NewLI;
}

void VPOCodeGen::vectorizeInterleavedStore(VPInstruction *VPStoreArg,
                                           OVLSGroup *Group) {
  // Don't do anything unless we're vectorizing an instruction that is the group
  // insertion point.
  if (VPStoreArg !=
      cast<VPVLSClientMemref>(Group->getInsertPoint())->getInstruction())
    return;

  // Now we forget about VPStoreArg and work with group leader only.
  auto *Leader = cast<VPVLSClientMemref>(Group->getFirstMemref());
  const VPInstruction *LeaderInst = Leader->getInstruction();
  assert(LeaderInst->getOpcode() == Instruction::Store &&
         "Unexpected instruction in OVLSGroup");
  Type *LeaderAccessType = LeaderInst->getOperand(0)->getType();
  auto *LeaderAccessVecType = dyn_cast<VectorType>(LeaderAccessType);
  unsigned OriginalVL =
      LeaderAccessVecType ? LeaderAccessVecType->getNumElements() : 1;

  // Values to be stored.
  SmallVector<Value *, 8> GrpValues;

  // Interleave indexes of the values/memrefs.
  SmallVector<int, 8> GrpIndexes;

  // Populate GrpValues and GrpIndexes.
  for (OVLSMemref *Mrf : *Group) {
    VPValue *V = cast<VPVLSClientMemref>(Mrf)->getInstruction()->getOperand(0);
    Type *LeaderAccessType = LeaderInst->getOperand(0)->getType();
    GrpValues.push_back(Builder.CreateBitCast(
        getVectorValue(V), getWidenedType(LeaderAccessType, VF), "groupCast"));
    GrpIndexes.push_back(computeInterleaveIndex(Mrf, Group));
  }

  // For now indexes are assumed to be <0, 1, 2, ... N-1>. If it turns out not
  // true, we can sort the arrays and/or insert undefs instead of gaps.
  for (int i = 0, ie = GrpIndexes.size(); i < ie; ++i)
    assert(GrpIndexes[i] == i && "Unsupported memory references sequence");

  // Check that all memory references have the same interleave factor.
  auto InterleaveFactor = computeInterleaveFactor(Leader);
  assert(all_of(*Group,
                [InterleaveFactor](OVLSMemref *x) {
                  return computeInterleaveFactor(x) == InterleaveFactor;
                }) &&
         "Cannot compute shuffle mask for groups with different access sizes");

  // Check if the group is valid for this VF.
  assert(Group->getNumElems() == VF &&
         "Group number of elements must match VF");

  Value *StoredValue;
  if (InterleaveFactor != 1) {
    // Concatenate all the values being stored into a single wide vector.
    Value *ConcatValue = concatenateVectors(Builder, GrpValues);

    // Shuffle values into the correct order. A bit more sophisticated shuffle
    // mask is required if the original type is itself a vector type.
    SmallVector<int, 64> ShuffleMask =
        createVectorInterleaveMask(VF, InterleaveFactor, OriginalVL);
    StoredValue = Builder.CreateShuffleVector(
        ConcatValue, UndefValue::get(ConcatValue->getType()), ShuffleMask,
        "groupShuffle");
  } else {
    // No shuffling is needed for unit-stride stores.
    StoredValue = GrpValues[0];
  }

  // Compute address for the wide store.
  Value *ScatterAddress = getVectorValue(LeaderInst->getOperand(1));
  Value *ScalarAddress = Builder.CreateExtractElement(
      ScatterAddress, (uint64_t)0, ScatterAddress->getName() + "_0");
  auto AddressSpace =
      cast<PointerType>(ScalarAddress->getType())->getAddressSpace();
  Type *GroupPtrTy = StoredValue->getType()->getPointerTo(AddressSpace);
  Value *GroupPtr =
      Builder.CreateBitCast(ScalarAddress, GroupPtrTy, "groupPtr");

  // Create the wide store.
  Align Alignment = getOriginalLoadStoreAlignment(LeaderInst);
  Instruction *GroupStore;
  if (MaskValue) {
    Value *StoreMask = replicateVectorElts(
        MaskValue, OriginalVL * Group->size(), Builder, "groupStoreMask");
    OptRptStats.MaskedVLSStores += Group->size();
    GroupStore =
        Builder.CreateMaskedStore(StoredValue, GroupPtr, Alignment, StoreMask);
  } else {
    OptRptStats.UnmaskedVLSStores += Group->size();
    GroupStore = Builder.CreateAlignedStore(StoredValue, GroupPtr, Alignment);
  }

  DEBUG_WITH_TYPE("ovls", dbgs()
                              << "Emitted a group-wide vector STORE for Group#"
                              << Group->getDebugId() << ":\n  " << *GroupStore
                              << "\n\n");
  (void) GroupStore;
}

void VPOCodeGen::vectorizeUnitStrideStore(VPInstruction *VPInst,
                                          bool IsNegOneStride, bool IsPvtPtr) {
  VPValue *Ptr = getLoadStorePointerOperand(VPInst);
  Value *VecDataOp = getVectorValue(VPInst->getOperand(0));
  Type *StoreType = getLoadStoreType(VPInst);
  auto *StoreVecType = dyn_cast<VectorType>(StoreType);
  unsigned OriginalVL = StoreVecType ? StoreVecType->getNumElements() : 1;
  Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(Ptr, IsNegOneStride);

  Align Alignment;
  if (!PreferredPeeling) {
    // No peeling means static peeling with peel count = 0.
    VPlanStaticPeeling Peeling(0);
    Alignment =
        VPAA.getAlignmentUnitStride(*cast<VPLoadStoreInst>(VPInst), Peeling);
  } else {
    Alignment = getOriginalLoadStoreAlignment(VPInst);
  }

  if (IsNegOneStride) // Reverse
    // If we store to reverse consecutive memory locations, then we need
    // to reverse the order of elements in the stored value.
    VecDataOp = reverseVector(VecDataOp);

  Instruction *Store;
  if (MaskValue) {
    // Replicate the mask if VPInst is a vector instruction originally.
    Value *RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                              "replicatedMaskElts.");
    // We need to reverse the mask for -1 stride.
    if (IsNegOneStride)
      RepMaskValue = reverseVector(RepMaskValue, OriginalVL);

    ++OptRptStats.MaskedUnalignedUnitStrideStores;
    Store =
        Builder.CreateMaskedStore(VecDataOp, VecPtr, Alignment, RepMaskValue);
  } else {
    ++OptRptStats.UnmaskedUnalignedUnitStrideStores;
    Store = Builder.CreateAlignedStore(VecDataOp, VecPtr, Alignment);
  }

  const DataLayout &DL = OrigLoop->getHeader()->getModule()->getDataLayout();
  if (Alignment == DL.getTypeAllocSize(VecDataOp->getType()))
    if (auto *NtmpMD = cast<VPLoadStoreInst>(VPInst)->getMetadata(
            LLVMContext::MD_nontemporal))
      Store->setMetadata(LLVMContext::MD_nontemporal, NtmpMD);

  if (auto *DynPeeling =
          dyn_cast_or_null<VPlanDynamicPeeling>(PreferredPeeling))
    if (VPInst == DynPeeling->memref())
      attachPreferredAlignmentMetadata(Store, DynPeeling->targetAlignment());
}

void VPOCodeGen::vectorizeStoreInstruction(VPInstruction *VPInst,
                                           bool EmitIntrinsic) {
  Type *StoreType = VPInst->getOperand(0)->getType();
  auto *StoreVecType = dyn_cast<VectorType>(StoreType);
  assert(
      (!StoreVecType || StoreVecType->getElementType()->isSingleValueType()) &&
      "Re-vectorization supports simple vectors only!");

  // Pointer operand of Store will always be second operand.
  VPValue *Ptr = VPInst->getOperand(1);

  // Stores that are non-vectorizable should be serialized.
  if (!isVectorizableLoadStore(VPInst))
    return serializeWithPredication(VPInst);

  // Handle vectorization of a linear value store.
  if (isVPValueLinear(Ptr)) {
    llvm_unreachable("VPVALCG: Vectorization of linear store not uplifted.");
#if 0
    vectorizeLinearStore(Inst);
    return;
#endif
  }

  // Stores to uniform pointers can be optimally generated as a scalar store in
  // vectorized code.
  // TODO: Extend the optimization for masked uniform stores too. Will need
  // all-zero check (like masked uniform load) and functionality to find out
  // last unmasked lane for divergent data operand.
  if (isVPValueUniform(Ptr, Plan) && !MaskValue) {
    Value *ScalarPtr = getScalarValue(Ptr, 0);
    VPValue *DataOp = VPInst->getOperand(0);
    Align Alignment = getOriginalLoadStoreAlignment(VPInst);
    // Extract last lane of data operand to generate scalar store. For uniform
    // data operand, the same value is present on all lanes.
    Builder.CreateAlignedStore(getScalarValue(DataOp, VF - 1), ScalarPtr,
                               Alignment);
    return;
  }

  unsigned OriginalVL = StoreVecType ? StoreVecType->getNumElements() : 1;
  Value *VecDataOp = getVectorValue(VPInst->getOperand(0));

  // Try to handle consecutive stores without VLS.
  if (VPlanUseDAForUnitStride) {
    bool IsNegOneStride = false;
    bool ConsecutiveStride =
        isVPValueConsecutivePtrStride(Ptr, Plan, IsNegOneStride);
    if (ConsecutiveStride) {
      // TODO: VPVALCG: Special handling for mask value is also needed for
      // conditional last privates.
      bool IsPvtPtr = getVPValuePrivateMemoryPtr(Ptr) != nullptr;
      vectorizeUnitStrideStore(VPInst, IsNegOneStride, IsPvtPtr);
      return;
    }
  }

  // Try to do SCATTER-to-SHUFFLE optimization.
  if (OVLSGroup *Group = VLSA->getGroupsFor(Plan, VPInst)) {
    Optional<int64_t> GroupStride = Group->getConstStride();
    assert(GroupStride && "Indexed loads are not supported");
    // Groups with gaps are not supported either.
    APInt AccessMask = Group->computeByteAccessMask();
    if (AccessMask.isAllOnesValue() &&
        AccessMask.getBitWidth() == *GroupStride) {
      vectorizeInterleavedStore(VPInst, Group);
      return;
    }
  }

  // If VLS failed to emit a wide store, we have to emit a SCATTER
  // instruction.
  Value *ScatterPtr = getWidenedAddressForScatterGather(Ptr);
  Type *PtrToElemTy = cast<VectorType>(ScatterPtr->getType())->getElementType();
  Type *ElemTy = PtrToElemTy->getPointerElementType();
  VectorType *DesiredDataTy = FixedVectorType::get(ElemTy, VF * OriginalVL);
  // TODO: Verify if this bitcast should be done this late. Maybe an earlier
  // transform can introduce it, if needed.
  VecDataOp = Builder.CreateBitCast(VecDataOp, DesiredDataTy, "cast");

  // Replicate the mask if VPInst is a vector instruction originally.
  Value *RepMaskValue = nullptr;
  if (MaskValue)
    RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                       "replicatedMaskElts.");
  Align Alignment = getAlignmentForGatherScatter(VPInst);
  ++(RepMaskValue ? OptRptStats.MaskedScatters : OptRptStats.UnmaskedScatters);
  Builder.CreateMaskedScatter(VecDataOp, ScatterPtr, Alignment, RepMaskValue);
}

// This function returns computed addresses of memory locations which should be
// accessed in the vectorized code. These addresses, take the form of a GEP
// instruction, and this GEP is used as pointer operand of the resulting
// scatter/gather intrinsic.
Value *VPOCodeGen::getWidenedAddressForScatterGather(VPValue *VPBasePtr) {
  assert(VPBasePtr->getType()->isPointerTy() &&
         "Expect 'VPBasePtr' to be a PointerType");

  // Vectorize BasePtr.
  Value *BasePtr = getVectorValue(VPBasePtr);

  // No replication is needed for non-vector types.
  Type *LSIType = cast<PointerType>(VPBasePtr->getType())->getElementType();
  if (!isa<VectorType>(LSIType))
    return BasePtr;

  unsigned AddrSpace =
      cast<PointerType>(VPBasePtr->getType())->getAddressSpace();

  // Cast the inner vector-type to it's elemental scalar type.
  // e.g. - <VF x <OriginalVL x Ty> addrspace(x)*>
  //                          |
  //                          |
  //                          V
  //                <VF x Ty addrspace(x)*>
  Value *TypeCastBasePtr = Builder.CreateBitCast(
      BasePtr,
      FixedVectorType::get(
          cast<VectorType>(LSIType)->getElementType()->getPointerTo(AddrSpace),
          VF));
  // Replicate the base-address OriginalVL times
  //                <VF x Ty addrspace(x)*>
  //                          |
  //                          |
  //                          V
  //      < 0, 1, .., OriginalVL-1, ..., 0, 1, ..., OriginalVL-1>
  unsigned OriginalVL = cast<VectorType>(LSIType)->getNumElements();
  Value *VecBasePtr =
      replicateVectorElts(TypeCastBasePtr, OriginalVL, Builder, "vecBasePtr.");

  // Create a vector of consecutive numbers from zero to OriginalVL-1 repeated
  // VF-times.
  SmallVector<Constant *, 32> Indices;
  for (unsigned J = 0; J < VF; ++J)
    for (unsigned I = 0; I < OriginalVL; ++I) {
      Indices.push_back(
          ConstantInt::get(Type::getInt64Ty(LSIType->getContext()), I));
    }

  // Add the consecutive indices to the vector value.
  Constant *Cv = ConstantVector::get(Indices);

  // Create a GEP that would return the address of each elements that is to be
  // accessed.
  Value *WidenedVectorGEP =
      Builder.CreateGEP(nullptr, VecBasePtr, Cv, "elemBasePtr.");
  return WidenedVectorGEP;
}

// This function return an appropriate BasePtr for cases where we are dealing
// with load/store to consecutive memory locations
Value *VPOCodeGen::createWidenedBasePtrConsecutiveLoadStore(VPValue *Ptr,
                                                            bool Reverse) {
  Type *VecTy = Ptr->getType()->getPointerElementType();
  unsigned AddrSpace = Ptr->getType()->getPointerAddressSpace();
  VectorType *WideDataTy = getWidenedType(VecTy, VF);
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

  VecPtr = Reverse
               ? Builder.CreateGEP(
                     VecPtr, Builder.getInt32(1 - WideDataTy->getNumElements()))
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
  auto storeVectorValue = [this](Value *VecValue, VPValue *ScalarPtr,
                                 Align Alignment) {
    assert(cast<VectorType>(VecValue->getType())->getNumElements() == VF &&
           "Invalid vector width of value");

    bool IsNegOneStride = false;
    bool ConsecutiveStride =
        isVPValueConsecutivePtrStride(ScalarPtr, Plan, IsNegOneStride);

    // TODO: Currently only address with stride = 1 can be optimized. Need to
    // handle other cases.
    if (ConsecutiveStride && !IsNegOneStride) {
      Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(
          ScalarPtr, false /*Reverse*/);
      if (MaskValue)
        Builder.CreateMaskedStore(VecValue, VecPtr, Alignment, MaskValue);
      else
        Builder.CreateAlignedStore(VecValue, VecPtr, Alignment);
    } else {
      Value *VectorPtr = getWidenedAddressForScatterGather(ScalarPtr);
      Type *PtrToElemTy =
          cast<VectorType>(VectorPtr->getType())->getElementType();
      Type *ElemTy = PtrToElemTy->getPointerElementType();
      VectorType *DesiredDataTy = FixedVectorType::get(ElemTy, VF);
      VecValue = Builder.CreateBitCast(VecValue, DesiredDataTy, "cast");

      Builder.CreateMaskedScatter(VecValue, VectorPtr, Alignment, MaskValue);
    }
  };

  storeVectorValue(ExtractSinInst, VPCall->getOperand(1), Alignment);
  storeVectorValue(ExtractCosInst, VPCall->getOperand(2), Alignment);
}

void VPOCodeGen::generateVectorCalls(VPCallInstruction *VPCall,
                                     unsigned PumpFactor, bool IsMasked,
                                     VectorVariant *MatchedVariant,
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

    Function *VectorF =
        getOrInsertVectorFunction(CalledFunc, VF / PumpFactor, VecArgTys, TLI,
                                  VectorIntrinID, MatchedVariant, IsMasked);
    assert(VectorF && "Can't create vector function.");
    CallInst *VecCall = Builder.CreateCall(VectorF, VecArgs);
    VecCall->setCallingConv(VectorF->getCallingConv());
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

  // Handle results of SVML sincos function calls
  // sincos function has two return values. The scalar sincos function uses
  // pointers as out-parameters. SVML sincos function, instead, returns them in
  // a struct directly. Here we store the results in the struct to the pointer
  // given in parameters to bridge the gap between these two approaches.
  if (cast<CallInst>(CallResults[0])
          ->getCalledFunction()
          ->getName()
          .startswith("__svml_sincos"))
    generateStoreForSinCos(VPCall, VPWidenMap[VPCall]);
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
  VectorVariant *MatchedVariant =
      const_cast<VectorVariant *>(VPCall->getVectorVariant());
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
  LLVM_DEBUG(dbgs() << "Matched Variant: " << MatchedVariant->encode() << "\n");

  // Generate vector calls using matched vector variant.
  SmallVector<Value *, 4> CallResults;
  generateVectorCalls(VPCall, PumpFactor, IsMasked, MatchedVariant,
                      Intrinsic::not_intrinsic /*No vector intrinsic*/,
                      CallResults);

  // Post process generated vector calls.
  VPWidenMap[VPCall] = getCombinedCallResults(CallResults);
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
    auto LoopPH = State->CFG.FirstExecutableVPBB;
    return State->CFG.VPBB2IRBB[const_cast<VPBasicBlock *>(LoopPH)]
        ->getTerminator();
  };

  // If the VPValue has not been vectorized, check if it has been scalarized
  // instead. If it has been scalarized, and we actually need the value in
  // vector form, we will construct the vector values on demand.
  if (VPScalarMap.count(V)) {
    // Use DA to check if VPValue is uniform.
    bool IsUniform = isVPValueUniform(V, Plan);

    Value *VectorValue = nullptr;
    IRBuilder<>::InsertPointGuard Guard(Builder);

    auto UpdateInsertPoint = [=](Value *ScalarValue) -> void {
      // ScalarValue can be a constant, so insertion point setting is not needed
      // for that case.
      auto *ScalarInst = dyn_cast<Instruction>(ScalarValue);
      if (!ScalarInst) {
        if (Plan->hasExplicitRemainder())
          Builder.SetInsertPoint(getInsertPointPH());
        return;
      }
      auto It = ++(ScalarInst->getIterator());

      while (isa<PHINode>(*It))
        ++It;

      Builder.SetInsertPoint(ScalarInst->getParent(), It);
    };

    if (IsUniform) {
      Value *ScalarValue = VPScalarMap[V][0];
      UpdateInsertPoint(ScalarValue);
      if (ScalarValue->getType()->isVectorTy()) {
        VectorValue =
            replicateVector(ScalarValue, VF, Builder,
                            "replicatedVal." + ScalarValue->getName());
      } else
        VectorValue = Builder.CreateVectorSplat(VF, ScalarValue, "broadcast");

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
  assert((isa<VPExternalDef>(V) || isa<VPConstant>(V) ||
          isa<VPMetadataAsValue>(V)) &&
         "Unknown external VPValue.");
  assert(isVPValueUniform(V, Plan) && "External value is not uniform.");
  Value *UnderlyingV = getScalarValue(V, 0 /*Lane*/);
  assert(UnderlyingV &&
         "External VPValues are expected to have underlying IR value set.");

  // Place the code for broadcasting invariant variables in the new preheader.
  IRBuilder<>::InsertPointGuard Guard(Builder);
  if (Plan->hasExplicitRemainder())
    Builder.SetInsertPoint(getInsertPointPH());
  else
    Builder.SetInsertPoint(LoopVectorPreHeader->getTerminator());

  // Broadcast V and save the value for future uses.
  Value *Widened;
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
    Widened = replicateVector(UnderlyingV, VF, Builder,
                                    "replicatedVal." + UnderlyingV->getName());
  } else {
    Widened = Builder.CreateVectorSplat(VF, UnderlyingV, "broadcast");
  }
  VPWidenMap[V] = Widened;

  return Widened;
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
      SmallVector<Value *, 3> ScalarArgs = {
          Size, getScalarValue(VPCall->getOperand(1), 0),
          getScalarValue(VPCall->getOperand(2), 0)};
      auto *ScalarInstrinsic = generateSerialInstruction(VPCall, ScalarArgs);
      VPScalarMap[VPCall][0] = ScalarInstrinsic;
      return;
    }
  }

  // This call is either not operating on privates, or is not vectorizable. So,
  // serialize.
  serializeWithPredication(VPCall);
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
    if (isVPValueUniform(V, Plan))
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
  if (auto *ValVecTy = dyn_cast<VectorType>(V->getType())) {
    unsigned OrigNumElts = ValVecTy->getNumElements();
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
  unsigned OriginalVL = cast<VectorType>(VecTy)->getNumElements();

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
  unsigned OriginalVL = cast<VectorType>(VecTy)->getNumElements();

  // In case of an non-const index, we serialize the instruction.
  // We first get the actual index, for the vectorized data using
  // 'add' and then insert that scalar into the index.

  if (!isa<VPConstant>(OrigIndexVal) ||
      !cast<VPConstant>(OrigIndexVal)->isConstantInt()) {
    if (MaskValue) {
      serializeWithPredication(VPInst);
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
    return;
  }

  // TODO: Need more test coverage for vectorizing insertelement with const
  // index, especially masked insert scenarios.

  VPConstant *OrigIndexVPConst = cast<VPConstant>(OrigIndexVal);
  assert(OrigIndexVPConst->isConstantInt() &&
         "Original index is not constant integer.");
  unsigned Index = OrigIndexVPConst->getZExtValue();
  unsigned WideNumElts =
      cast<VectorType>(InsertTo->getType())->getNumElements();

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
      cast<VectorType>(VPInst->getOperand(0)->getType())->getNumElements();
  int OrigDstVL = cast<VectorType>(VPInst->getType())->getNumElements();

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
  auto *MaskTy = dyn_cast<VectorType>(MaskValue->getType());
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

  assert(cast<VectorType>(MaskValue->getType())->getNumElements() == VF &&
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

bool VPOCodeGen::isSerialized(VPValue *V) const {
  auto It = VPScalarMap.find(V);
  if (It != VPScalarMap.end()) {
    const auto &LaneMap = It->second;
    return LaneMap.count(1 /* Lane */);
  }
  return false;
}

void VPOCodeGen::serializeInstruction(VPInstruction *VPInst) {

  unsigned Lanes =
      (!VPInst->mayHaveSideEffects() && isVPValueUniform(VPInst, Plan)) ||
              (isa<VPCallInstruction>(VPInst) &&
               cast<VPCallInstruction>(VPInst)->isKernelUniformCall())
          ? 1
          : VF;

  for (unsigned Lane = 0; Lane < Lanes; ++Lane) {
    SmallVector<Value *, 4> ScalarOperands;
    // All operands to the serialized Instruction should be scalar Values.
    for (unsigned Op = 0, e = VPInst->getNumOperands(); Op != e; ++Op) {
      auto *ScalarOp = getScalarValue(VPInst->getOperand(Op), Lane);
      assert(ScalarOp && "Operand for serialized instruction not found.");
      LLVM_DEBUG(dbgs() << "LVCG: Serialize scalar op: "; ScalarOp->dump());
      ScalarOperands.push_back(ScalarOp);
    }

    Value *SerialInst = generateSerialInstruction(VPInst, ScalarOperands);
    assert(SerialInst && "Instruction not serialized.");
    VPScalarMap[VPInst][Lane] = SerialInst;
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
    if (auto *BlendVecTy = dyn_cast<VectorType>(Blend->getType())) {
      unsigned OriginalVL = BlendVecTy->getNumElements();
      Cond = replicateVectorElts(Cond, OriginalVL, Builder);
    }
    BlendVal = Builder.CreateSelect(Cond, IncomingVecVal, BlendVal, "predblend");
  }

  VPWidenMap[Blend] = BlendVal;
}

void VPOCodeGen::vectorizeVPPHINode(VPPHINode *VPPhi) {
  auto PhiTy = VPPhi->getType();
  PHINode *NewPhi;
  // FIXME: Replace with proper SVA.
  bool EmitScalarOnly = !Plan->getVPlanDA()->isDivergent(*VPPhi) && !MaskValue;
  if (needScalarCode(VPPhi) || EmitScalarOnly) {
    NewPhi = Builder.CreatePHI(PhiTy, VPPhi->getNumOperands(), "uni.phi");
    VPScalarMap[VPPhi][0] = NewPhi;
    ScalarPhisToFix[VPPhi] = NewPhi;
  }
  if (EmitScalarOnly)
    return;
  if (needVectorCode(VPPhi)) {
    PhiTy = getWidenedType(PhiTy, VF);
    NewPhi = Builder.CreatePHI(PhiTy, VPPhi->getNumOperands(), "vec.phi");
    VPWidenMap[VPPhi] = NewPhi;
    PhisToFix[VPPhi] = NewPhi;
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

  if (!Plan->hasExplicitRemainder() &&
      VPInst->getOpcode() != VPInstruction::PrivateFinalUncondMem) {
    // Add info to update scalar loop livein and liveouts. We don't
    // need this for in-memory privates.
    const VPLoopEntity *Entity = VPEntities->getPrivate(VPInst);
    assert(Entity && "Unexpected: private last value is not for entity");
    EntitiesFinalVPInstMap[Entity] = VPInst;
  }
}

void VPOCodeGen::vectorizeReductionFinal(VPReductionFinal *RedFinal) {
  Value *VecValue = getVectorValue(RedFinal->getOperand(0));
  Intrinsic::ID Intrin = RedFinal->getVectorReduceIntrinsic();
  Type *ElType = RedFinal->getOperand(0)->getType();
  if (isa<VectorType>(ElType))
    // TODO: can implement as shufle/OP sequence for vectors.
    llvm_unreachable("Unsupported vector data type in reduction");

  auto *StartVPVal =
      RedFinal->getNumOperands() > 1 ? RedFinal->getOperand(1) : nullptr;
  Value *Acc = nullptr;
  if (StartVPVal) {
    assert((isa<VPExternalDef>(StartVPVal) || isa<VPConstant>(StartVPVal)) &&
           "Unsupported reduction StartValue");
    Acc = getScalarValue(StartVPVal, 0);
  }
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

  if (!Plan->hasExplicitRemainder()) {
    const VPLoopEntity *Entity = VPEntities->getReduction(RedFinal);
    assert(Entity && "Unexpected: reduction last value is not for entity");
    EntitiesFinalVPInstMap[Entity] = RedFinal;
  }
}

void VPOCodeGen::vectorizeAllocatePrivate(VPAllocatePrivate *V) {
  // Private memory is a pointer. We need to get element type
  // and allocate VF elements.
  Type *OrigTy = V->getType()->getPointerElementType();

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
    if (auto *OrigVecTy = dyn_cast<VectorType>(OrigTy)) {
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
    if (V->isSOALayout())
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
          ? Builder.CreateInBoundsGEP(BcastStart, {VectorStep}, "vector_gep")
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
    //  for post increment liveouts LV = start + step*rounded_tc,
    //  for pre increment liveouts LV = start + step*(rounded_tc-1)
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
    Value *TripCnt = VectorTripCount;
    if (VPInst->isLastValPreIncrement())
      TripCnt =
          Builder.CreateSub(TripCnt, ConstantInt::get(TripCnt->getType(), 1));
    Instruction::CastOps CastOp =
        CastInst::getCastOpcode(TripCnt, true, StepType, true);
    Value *CRD = Builder.CreateCast(CastOp, TripCnt, StepType, "cast.crd");
    Value *MulV = Builder.CreateBinOp(
        static_cast<Instruction::BinaryOps>(StepOpc), Step, CRD);
    auto *VPStart = VPInst->getOperand(0);
    auto *Start = getScalarValue(VPStart, 0);
    LastValue =
        (VPInst->getType()->isPointerTy() || Opc == Instruction::GetElementPtr)
            ? Builder.CreateInBoundsGEP(Start, {MulV}, "final_gep")
            : Builder.CreateBinOp(static_cast<Instruction::BinaryOps>(Opc),
                                  Start, MulV);
  }
  // The value is scalar
  VPScalarMap[VPInst][0] = LastValue;
  if (!Plan->hasExplicitRemainder()) {
    const VPLoopEntity *Entity = VPEntities->getInduction(VPInst);
    assert(Entity && "Induction last value is not for entity");
    EntitiesFinalVPInstMap[Entity] = VPInst;
  }
}

void VPOCodeGen::fixOutgoingValues() {
  for (auto &LastValPair : EntitiesFinalVPInstMap) {
    if (auto *Reduction = dyn_cast<VPReduction>(LastValPair.first))
      fixReductionLastVal(*Reduction,
                          cast<VPReductionFinal>(LastValPair.second));
    if (auto *Induction = dyn_cast<VPInduction>(LastValPair.first))
      fixInductionLastVal(*Induction,
                          cast<VPInductionFinal>(LastValPair.second));
    if (isa<VPPrivate>(LastValPair.first)) {
      fixPrivateLastVal(cast<VPInstruction>(LastValPair.second));
    }
  }
}

void VPOCodeGen::attachPreferredAlignmentMetadata(Instruction *Memref,
                                                  Align PreferredAlignment) {
  auto &C = Builder.getContext();
  auto *CI = ConstantInt::get(Type::getInt32Ty(C), PreferredAlignment.value());
  SmallVector<Metadata *, 1> Ops{ConstantAsMetadata::get(CI)};
  Memref->setMetadata("intel.preferred_alignment", MDTuple::get(C, Ops));
}

void VPOCodeGen::fixLiveOutValues(VPInstruction *FinalVPInst, Value *LastVal) {
  assert(isa<VPReductionFinal>(FinalVPInst) ||
         FinalVPInst->getOpcode() == VPInstruction::PrivateFinalUncond ||
         isa<VPInductionFinal>(FinalVPInst) &&
             "Only loop entity finalization instructions can be live-out.");
  for (VPUser *User : FinalVPInst->users()) {
    if (auto LI = dyn_cast<VPLiveOutValue>(User)) {
      // Get VPExternalUse and restore its operand.
      // TODO: make VPExternalUse purely descriptional, w/o operands and
      // use VPLiveOutValue to keep them.
      const VPExternalUse *EUse =
          Plan->getExternals().getVPExternalUse(LI->getMergeId());
      User = cast<VPUser>(const_cast<VPExternalUse*>(EUse));
      User->addOperand(FinalVPInst);
    }
    if (isa<VPExternalUse>(User)) {
      Value *ExtVal = User->getUnderlyingValue();
      if (!ExtVal)
        continue;
      if (auto Phi = dyn_cast<PHINode>(ExtVal)) {
        int Ndx = Phi->getBasicBlockIndex(LoopMiddleBlock);
        if (Ndx == -1)
          Phi->addIncoming(LastVal, LoopMiddleBlock);
        else
          Phi->setIncomingValue(Ndx, LastVal);
      } else {
        int Ndx = User->getOperandIndex(FinalVPInst);
        assert(Ndx != -1 && "Operand not found in User");
        Value *Operand = const_cast<Value *>(
            cast<VPExternalUse>(User)->getUnderlyingOperand(Ndx));
        cast<Instruction>(ExtVal)->replaceUsesOfWith(Operand, LastVal);
      }
    }
  }
}

void VPOCodeGen::createLastValPhiAndUpdateOldStart(Value *OrigStartValue,
                                                   PHINode *Phi,
                                                   const Twine &NameStr,
                                                   Value *LastVal) {
  PHINode *BCBlockPhi = PHINode::Create(OrigStartValue->getType(), 2, NameStr,
                                        LoopScalarPreHeader->getTerminator());
  for (unsigned I = 0, E = LoopBypassBlocks.size(); I != E; ++I)
    BCBlockPhi->addIncoming(OrigStartValue, LoopBypassBlocks[I]);
  BCBlockPhi->addIncoming(LastVal, LoopMiddleBlock);

  // Fix the scalar loop reduction variable.
  int IncomingEdgeBlockIdx = Phi->getBasicBlockIndex(OrigLoop->getLoopLatch());
  assert(IncomingEdgeBlockIdx >= 0 && "Invalid block index");
  // Pick the other block.
  int SelfEdgeBlockIdx = (IncomingEdgeBlockIdx ? 0 : 1);
  Phi->setIncomingValue(SelfEdgeBlockIdx, BCBlockPhi);
}

void VPOCodeGen::fixReductionLastVal(const VPReduction &Red,
                                     VPReductionFinal *RedFinal) {
  if (Red.getIsMemOnly()) {
#if 0
    // TODO: Implement last value fixing for in-memory reductions.
    auto OrigPtr = VPEntities->getOrigMemoryPtr(&Red);
    assert(OrigPtr && "Unexpected nullptr original memory");
    auto ScalarPtr = OrigPtr->getUnderlyingValue();
    Builder.SetInsertPoint(LoopScalarPreHeader->getTerminator());
    MergedVal = Builder.CreateLoad(ScalarPtr, ScalarPtr->getName() + ".reload");
#endif
  } else {
    // Reduction final value should be mapped only in scalar map always. TODO:
    // Use getScalarValue instead?
    Value *LastVal = VPScalarMap[RedFinal][0];
    VPValue *VPStart = Red.getRecurrenceStartValue();
    Value *OrigStartValue = VPStart->getUnderlyingValue();
    VPPHINode *VPHi = VPEntities->getRecurrentVPHINode(Red);
    assert(VPHi && "nullptr is not expected");
    PHINode *Phi = cast<PHINode>(VPHi->getUnderlyingValue());
    createLastValPhiAndUpdateOldStart(OrigStartValue, Phi, "bc.merge.reduction",
                                      LastVal);
    fixLiveOutValues(RedFinal, LastVal);
  }
}

void VPOCodeGen::fixInductionLastVal(const VPInduction &Ind,
                                     VPInductionFinal *IndFinal) {
  if (Ind.getIsMemOnly()) {
    // TODO: Implement last value fixing for in-memory inductions.
  } else {
    // Induction final value should be mapped only in scalar map always. TODO:
    // Use getScalarValue instead?
    Value *LastVal = VPScalarMap[IndFinal][0];
    VPValue *VPStart = Ind.getStartValue();
    Value *OrigStartValue = VPStart->getUnderlyingValue();
    VPPHINode *VPHi = VPEntities->getRecurrentVPHINode(Ind);
    assert(VPHi && "nullptr is not expected");
    PHINode *Phi = cast<PHINode>(VPHi->getUnderlyingValue());
    createLastValPhiAndUpdateOldStart(OrigStartValue, Phi, "bc.resume.val",
                                      LastVal);
    fixLiveOutValues(IndFinal, LastVal);
  }
}

void VPOCodeGen::fixPrivateLastVal(VPInstruction *PrivFinal) {
  Value *LastVal = VPScalarMap[PrivFinal][0];
  fixLiveOutValues(PrivFinal, LastVal);
}

void VPOCodeGen::fixNonInductionVPPhis() {
  std::function<void(DenseMap<VPPHINode *, PHINode *> &)> fixInductions =
      [&](DenseMap<VPPHINode *, PHINode *> &Table) -> void {
    bool IsScalar = &Table == &ScalarPhisToFix;
    for (auto PhiToFix : Table) {
      auto *VPPhi = PhiToFix.first;
      auto *Phi = PhiToFix.second;
      const unsigned NumPhiValues = VPPhi->getNumIncomingValues();
      for (unsigned I = 0; I < NumPhiValues; ++I) {
        auto *VPVal = VPPhi->getIncomingValue(I);
        auto *VPBB = VPPhi->getIncomingBlock(I);
        Value *IncValue =
            IsScalar ? getScalarValue(VPVal, 0) : getVectorValue(VPVal);
        if (Plan->hasExplicitRemainder()) {
          BasicBlock *BB = State->CFG.VPBB2IREndBB[VPBB];
          if (auto *LiveOut = dyn_cast<VPOrigLiveOut>(VPVal))
            BB = cast<VPScalarRemainder>(LiveOut->getOperand(0))
                     ->getLoop()
                     ->getLoopLatch();
          Phi->addIncoming(IncValue, BB);
        } else {
          Phi->addIncoming(IncValue, State->CFG.VPBB2IREndBB[VPBB]);
        }
      }
    }
    return;
  };
  fixInductions(ScalarPhisToFix);
  fixInductions(PhisToFix);
}
