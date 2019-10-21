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
//   IntelLoopVectorizationCodeGen.cpp -- LLVM IR Code generation from VPlan
//
//===----------------------------------------------------------------------===//

#include "IntelLoopVectorizationCodeGen.h"
#include "IntelLoopVectorizationLegality.h"
#include "IntelVPlan.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanVLSAnalysis.h"
#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-ir-loop-vectorize"

static cl::opt<bool>
    VPlanSerializeAlloca("vplan-serialize-alloca", cl::init(false), cl::Hidden,
                         cl::desc("Serialize alloca for private array-types"));

// Force VPValue based code generation path.
cl::opt<bool> EnableVPValueCodegen("enable-vp-value-codegen", cl::init(false),
                                   cl::Hidden,
                                   cl::desc("Enable VPValue based codegen"));

static cl::opt<bool> VPlanTeachLegalFromDA(
    "vplan-teach-legal-from-da", cl::init(true), cl::Hidden,
    cl::desc("Teach legal about uniforms recognized by DA"));

/// A helper function that returns GEP instruction and knows to skip a
/// 'bitcast'. The 'bitcast' may be skipped if the source and the destination
/// pointee types of the 'bitcast' have the same size.
/// For example:
///   bitcast double** %var to i64* - can be skipped
///   bitcast double** %var to i8*  - can not
static GetElementPtrInst *getGEPInstruction(Value *Ptr) {

  if (isa<GetElementPtrInst>(Ptr))
    return cast<GetElementPtrInst>(Ptr);
  GetElementPtrInst *GEP = nullptr;
  GEP = dyn_cast<GetElementPtrInst>(getPtrThruCast<BitCastInst>(Ptr));
  if (!GEP)
    GEP = dyn_cast<GetElementPtrInst>(getPtrThruCast<AddrSpaceCastInst>(Ptr));
  return GEP;
}

/// Reduce vector \p Vec to a scalar value according to the
/// recurrence descriptor.
static Value *reduceVector(Value *Vec,
                           RecurrenceDescriptor::RecurrenceKind RK,
                           RecurrenceDescriptor::MinMaxRecurrenceKind MinMaxKind,
                           IRBuilder<>& Builder) {
  unsigned VF = Vec->getType()->getVectorNumElements();
  // Reduce all of the unrolled parts into a single vector.
  unsigned Op = RecurrenceDescriptor::getRecurrenceBinOp(RK);
  // VF is a power of 2 so we can emit the reduction using log2(VF) shuffles
  // and vector ops, reducing the set of values being computed by half each
  // round.
  assert(isPowerOf2_32(VF) &&
         "Reduction emission only supported for pow2 vectors!");
  SmallVector<Constant *, 32> ShuffleMask(VF, nullptr);
  for (unsigned i = VF; i != 1; i >>= 1) {
    // Move the upper half of the vector to the lower half.
    for (unsigned j = 0; j != i / 2; ++j)
      ShuffleMask[j] = Builder.getInt32(i / 2 + j);

    // Fill the rest of the mask with undef.
    std::fill(&ShuffleMask[i / 2], ShuffleMask.end(),
              UndefValue::get(Builder.getInt32Ty()));

    Value *Shuf = Builder.CreateShuffleVector(
      Vec, UndefValue::get(Vec->getType()),
      ConstantVector::get(ShuffleMask), "rdx.shuf");

    if (Op != Instruction::ICmp && Op != Instruction::FCmp)
      Vec = Builder.CreateBinOp((Instruction::BinaryOps)Op,
                                Vec, Shuf, "bin.rdx");
    else
      Vec = createMinMaxOp(Builder, MinMaxKind, Vec, Shuf);
  }

  // The result is in the first element of the vector.
  return Builder.CreateExtractElement(Vec, Builder.getInt32(0));
}

static void addBlockToParentLoop(Loop *L, BasicBlock *BB, LoopInfo& LI) {
  if (auto *ParentLoop = L->getParentLoop())
    ParentLoop->addBasicBlockToLoop(BB, LI);
}

unsigned VPOCodeGen::getPrivateVarAlignment(Value *V) {
  // Get the alignment value based on what the type of V is.
  // We always set the alignment-value for the new, widened alloca as the
  // alignment of the original alloca. This means that in some cases, when the
  // original alloca has no alignment value, we end up setting up the alignment
  // based on the value computed from DataLayout for a particular pointer-type.
  // This might not be the most optimal value, but safe. One issue we might have
  // to deal with is in cost-modeling, where we compute costs based on
  // alignment.
  if (isa<GlobalValue>(V))
    return cast<GlobalValue>(V)->getAlignment();
  else if (isa<AllocaInst>(V))
    return cast<AllocaInst>(V)->getAlignment();
  else {
    const DataLayout &DL = OrigLoop->getHeader()->getModule()->getDataLayout();
    return DL.getPrefTypeAlignment(V->getType());
  }
}

bool VPOCodeGen::isSerializedPrivateArray(Value *Priv) const {
  if (VPlanSerializeAlloca && Legal->isLoopPrivate(Priv)) {
    Type *PrivTy = Priv->getType();
    return isa<PointerType>(PrivTy) &&
           isa<ArrayType>(cast<PointerType>(PrivTy)->getPointerElementType());
  }
  return false;
}

void VPOCodeGen::emitEndOfVectorLoop(Value *Count, Value *CountRoundDown) {
  // Add a check in the middle block to see if we have completed
  // all of the iterations in the first vector loop.
  // If (N - N%VF) == N, then we *don't* need to run the remainder.
  Value *CmpN = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_EQ, Count,
                                CountRoundDown, "cmp.n",
                                LoopMiddleBlock->getTerminator());
  ReplaceInstWithInst(LoopMiddleBlock->getTerminator(),
                      BranchInst::Create(LoopExitBlock, LoopScalarPreHeader,
                                         CmpN));
}

void VPOCodeGen::emitResume(Value *CountRoundDown) {
  // We are going to resume the execution of the scalar loop.
  // Go over all of the induction variables that we found and fix the
  // PHIs that are left in the scalar version of the loop.
  // The starting values of PHI nodes depend on the counter of the last
  // iteration in the vectorized loop.
  // If we come from a bypass edge then we need to start from the original
  // start value.

  // This variable saves the new starting index for the scalar loop. It is used
  // to test if there are any tail iterations left once the vector loop has
  // completed.
  VPOVectorizationLegality::InductionList *List = Legal->getInductionVars();
  for (auto &InductionEntry : *List) {
    PHINode *OrigPhi = InductionEntry.first;
    InductionDescriptor II = InductionEntry.second;

    // Create phi nodes to merge from the  backedge-taken check block.
    PHINode *BCResumeVal =
        PHINode::Create(OrigPhi->getType(), 3, "bc.resume.val",
                        LoopScalarPreHeader->getTerminator());
    Value *&EndValue = IVEndValues[OrigPhi];
    if (OrigPhi == Legal->getInduction()) {
      // We already know what the end value is.
      IRBuilder<> B(LoopMiddleBlock->getTerminator());
      EndValue = B.CreateZExtOrTrunc(CountRoundDown, OrigPhi->getType(),
                                     "cast.endval");
    } else {
      // We must compute what the end value is.
      IRBuilder<> B(LoopBypassBlocks.back()->getTerminator());
      Type *StepType = II.getStep()->getType();
      Instruction::CastOps CastOp =
          CastInst::getCastOpcode(CountRoundDown, true, StepType, true);
      Value *CRD = B.CreateCast(CastOp, CountRoundDown, StepType, "cast.crd");
      const DataLayout &DL =
          OrigLoop->getHeader()->getModule()->getDataLayout();
      PredicatedScalarEvolution &PSE = Legal->getPSE();
      EndValue = emitTransformedIndex(B, CRD, PSE.getSE(), DL, II);
      assert(EndValue && "Unexpected null return from transform");
      EndValue->setName("ind.end");
    }

    // The new PHI merges the original incoming value, in case of a bypass,
    // or the value at the end of the vectorized loop.
    BCResumeVal->addIncoming(EndValue, LoopMiddleBlock);

    // Fix the scalar body counter (PHI node).
    unsigned BlockIdx = OrigPhi->getBasicBlockIndex(LoopScalarPreHeader);

    // The old induction's phi node in the scalar body needs the truncated
    // value.
    for (BasicBlock *BB : LoopBypassBlocks)
      BCResumeVal->addIncoming(II.getStartValue(), BB);
    OrigPhi->setIncomingValue(BlockIdx, BCResumeVal);
  }
}

void VPOCodeGen::emitMinimumIterationCountCheck(Loop *L, Value *Count) {

  BasicBlock *VLoopFirstBB = L->getLoopPreheader();
  assert(VLoopFirstBB && "Loop does not have a preheader block.");
  IRBuilder<> Builder(VLoopFirstBB->getTerminator());

  // Generate code to check that the loop's trip count that we computed by
  // adding one to the backedge-taken count will not overflow.
  Value *CheckMinIters = Builder.CreateICmpULT(
      Count, ConstantInt::get(Count->getType(), VF), "min.iters.check");

  BasicBlock *NewBB =
    VLoopFirstBB->splitBasicBlock(VLoopFirstBB->getTerminator(),
                                 "min.iters.checked");
  // Update dominator tree immediately if the generated block is a
  // LoopBypassBlock because SCEV expansions to generate loop bypass
  // checks may query it before the current function is finished.
  DT->addNewBlock(NewBB, VLoopFirstBB);
  addBlockToParentLoop(L, NewBB, *LI);

  BranchInst *Branch = BranchInst::Create(LoopScalarPreHeader, NewBB,
                                          CheckMinIters);
  ReplaceInstWithInst(VLoopFirstBB->getTerminator(), Branch);

  LoopBypassBlocks.push_back(VLoopFirstBB);
}

Value *VPOCodeGen::emitTransformedIndex(IRBuilder<> &B, Value *Index,
                                        ScalarEvolution *SE,
                                        const DataLayout &DL,
                                        const InductionDescriptor &ID) const {

  SCEVExpander Exp(*SE, DL, "induction");
  auto Step = ID.getStep();
  auto StartValue = ID.getStartValue();
  assert(Index->getType() == Step->getType() &&
         "Index type does not match StepValue type");
  switch (ID.getKind()) {
  case InductionDescriptor::IK_IntInduction: {
    assert(Index->getType() == StartValue->getType() &&
           "Index type does not match StartValue type");

    // FIXME: Theoretically, we can call getAddExpr() of ScalarEvolution
    // and calculate (Start + Index * Step) for all cases, without
    // special handling for "isOne" and "isMinusOne".
    // But in the real life the result code getting worse. We mix SCEV
    // expressions and ADD/SUB operations and receive redundant
    // intermediate values being calculated in different ways and
    // Instcombine is unable to reduce them all.

    if (ID.getConstIntStepValue() && ID.getConstIntStepValue()->isMinusOne())
      return B.CreateSub(StartValue, Index);
    if (ID.getConstIntStepValue() && ID.getConstIntStepValue()->isOne())
      return B.CreateAdd(StartValue, Index);
    const SCEV *S = SE->getAddExpr(SE->getSCEV(StartValue),
                                   SE->getMulExpr(Step, SE->getSCEV(Index)));
    return Exp.expandCodeFor(S, StartValue->getType(), &*B.GetInsertPoint());
  }
  case InductionDescriptor::IK_PtrInduction: {
    assert(isa<SCEVConstant>(Step) &&
           "Expected constant step for pointer induction");
    const SCEV *S = SE->getMulExpr(SE->getSCEV(Index), Step);
    Index = Exp.expandCodeFor(S, Index->getType(), &*B.GetInsertPoint());
    return B.CreateGEP(nullptr, StartValue, Index);
  }
  case InductionDescriptor::IK_FpInduction: {
    assert(Step->getType()->isFloatingPointTy() && "Expected FP Step value");
    auto InductionBinOp = ID.getInductionBinOp();
    assert(InductionBinOp &&
           (InductionBinOp->getOpcode() == Instruction::FAdd ||
            InductionBinOp->getOpcode() == Instruction::FSub) &&
           "Original bin op should be defined for FP induction");

    Value *StepValue = cast<SCEVUnknown>(Step)->getValue();

    // Floating point operations had to be 'fast' to enable the induction.
    FastMathFlags Flags;
    Flags.setFast();

    Value *MulExp = B.CreateFMul(StepValue, Index);
    if (isa<Instruction>(MulExp))
      // We have to check, the MulExp may be a constant.
      cast<Instruction>(MulExp)->setFastMathFlags(Flags);

    Value *BOp = B.CreateBinOp(InductionBinOp->getOpcode(), StartValue, MulExp,
                               "induction");
    if (isa<Instruction>(BOp))
      cast<Instruction>(BOp)->setFastMathFlags(Flags);

    return BOp;
  }
  case InductionDescriptor::IK_NoInduction:
    return nullptr;
  }
  llvm_unreachable("invalid enum");
}

void VPOCodeGen::emitVectorLoopEnteredCheck(Loop *L, BasicBlock *Bypass) {
  Value *TC = getOrCreateVectorTripCount(L);
  BasicBlock *BB = L->getLoopPreheader();
  assert(BB && "Loop does not have preheader block.");
  IRBuilder<> Builder(BB->getTerminator());

  // Now, compare the new count to zero. If it is zero skip the vector loop and
  // jump to the scalar loop.
  Value *Cmp = Builder.CreateICmpEQ(TC, Constant::getNullValue(TC->getType()),
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

void VPOCodeGen::initLinears(PHINode *Induction, Loop *VecLoop) {
  // The first value of the Induction PHINode is the initial loop index and
  // the second value is the index value from the loop latch.
  auto NextIndex = Induction->getIncomingValueForBlock(VecLoop->getLoopLatch());

  // Add the linear inupdate for the next vector iteration of the loop before the
  // the loop latch terminator.
  IRBuilder<> Builder(cast<Instruction>(NextIndex)->getParent()->getTerminator());

  for (auto It : *(Legal->getLinears())) {
    Value *LinPtr = It.first;
    int LinStep = It.second;

    // Load the initial value of the linears at the end of the loop preheader
    auto CurrIP = Builder.saveIP();
    Builder.SetInsertPoint(LoopVectorPreHeader->getTerminator());
    auto LinInitVal = Builder.CreateLoad(LinPtr);
    Builder.restoreIP(CurrIP);

    // Cast NextIndex to LinValType
    auto LinValType = LinInitVal->getType();
    Instruction::CastOps CastOp =
      CastInst::getCastOpcode(NextIndex, true, LinValType, true);
    auto ConvIndex = Builder.CreateCast(CastOp, NextIndex, LinValType, "lin.cast");

    // Linear value increment is NextIndex * LinStep
    Value *LinIncr;
    if (LinStep != 1) {
      auto LinStepVal = ConstantInt::get(LinValType, LinStep);
      LinIncr = Builder.CreateMul(ConvIndex, LinStepVal);
    }
    else
      LinIncr = ConvIndex;

    Value *ValToStore = Builder.CreateAdd(LinInitVal, LinIncr);
    Builder.CreateStore(ValToStore, LinPtr);
  }
}

PHINode *VPOCodeGen::createInductionVariable(Loop *L, Value *Start, Value *End,
                                             Value *Step) {
  BasicBlock *Header = L->getHeader();
  BasicBlock *Latch = L->getLoopLatch();
  // As we're just creating this loop, it's possible no latch exists
  // yet. If so, use the header as this will be a single block loop.
  if (!Latch)
    Latch = Header;

  IRBuilder<> Builder(&*Header->getFirstInsertionPt());
  auto *Induction = Builder.CreatePHI(Start->getType(), 2, "index");

  Builder.SetInsertPoint(Latch->getTerminator());

  // Create i+1 and fill the PHINode.
  Value *Next = Builder.CreateAdd(Induction, Step, "index.next");
  Induction->addIncoming(Start, L->getLoopPreheader());
  Induction->addIncoming(Next, Latch);

  // Create the compare. Special case for 1-trip count vector loop by checking
  // for End == Step and start value of zero. We rely on later optimizations to
  // cleanup the loop. TODO: Consider modifying the vector code generation to
  // avoid the vector loop altogether for such cases.
  Value *ICmp;
  ConstantInt *ConstStart = dyn_cast<ConstantInt>(Start);
  if (End == Step && ConstStart && ConstStart->isZero())
    ICmp = Builder.getInt1(true);
  else
    ICmp = Builder.CreateICmpEQ(Next, End);

  BasicBlock *Exit = L->getExitBlock();
  assert(Exit && "Exit block not found for loop.");
  Builder.CreateCondBr(ICmp, Exit, Header);

  // Now we have two terminators. Remove the old one from the block.
  Latch->getTerminator()->eraseFromParent();

  return Induction;
}

void VPOCodeGen::createEmptyLoop() {

  LoopScalarBody = OrigLoop->getHeader();
  BasicBlock *LoopPreHeader = OrigLoop->getLoopPreheader();
  LoopExitBlock = OrigLoop->getExitBlock();

  assert(LoopPreHeader && "Must have loop preheader");
  assert(LoopExitBlock && "Must have an exit block");

  // Create vector loop body.
  LoopVectorBody =
    LoopPreHeader->splitBasicBlock(LoopPreHeader->getTerminator(),
                                   "vector.body");

  // Middle block comes after vector loop is done. It contains reduction tail
  // and checks if we need a scalar remainder.
  LoopMiddleBlock =
      LoopVectorBody->splitBasicBlock(LoopVectorBody->getTerminator(),
                                      "middle.block");

  // Scalar preheader contains phi nodes with incoming from vector version and
  // vector loop bypass blocks.
  LoopScalarPreHeader =
    LoopMiddleBlock->splitBasicBlock(LoopMiddleBlock->getTerminator(),
                                     "scalar.ph");

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

  // Find the loop boundaries.
  Value *Count = getOrCreateTripCount(Lp);

  emitMinimumIterationCountCheck(Lp, Count);

  // Now, compare the new count to zero. If it is zero skip the vector loop and
  // jump to the scalar loop.
  emitVectorLoopEnteredCheck(Lp, LoopScalarPreHeader);

  // CountRoundDown is a counter for the vectorized loop.
  // CountRoundDown = Count - Count % VF.
  Value *CountRoundDown = getOrCreateVectorTripCount(Lp);

  Type *IdxTy = Legal->getWidestInductionType();
  Value *StartIdx = ConstantInt::get(IdxTy, 0);
  Constant *Step = ConstantInt::get(IdxTy, VF);

  // Create an induction variable in vector loop with a step equal to VF.
  Induction = createInductionVariable(Lp, StartIdx, CountRoundDown, Step);

  // Add a check in the middle block to see if we have completed
  // all of the iterations in the first vector loop.
  // If (N - N%VF) == N, then we *don't* need to run the remainder.
  emitEndOfVectorLoop(Count, CountRoundDown);

  if (!EnableVPValueCodegen)
    // Resume from vector loop. If vector loop was executed, the remainder
    // is Count - CountRoundDown. Otherwise the remainder is Count. Emit
    // final values for inductions using the old infrastracture.
    // During VPValue-based code gen this is done by lowering VPEntities
    // instructions.
    emitResume(CountRoundDown);

  // Inform SCEV analysis to forget original loop
  PSE.getSE()->forgetLoop(OrigLoop);

  // Save the state.
  LoopVectorPreHeader = Lp->getLoopPreheader();

  if (!EnableVPValueCodegen)
    // Initialize loop linears. During VPValue-based code gen this is
    // done by lowering VPEntities instructions.
    initLinears(Induction, Lp);

  // Get ready to start creating new instructions into the vector preheader.
  Builder.SetInsertPoint(&*LoopVectorPreHeader->getFirstInsertionPt());
}

void VPOCodeGen::finalizeLoop() {

  if (!EnableVPValueCodegen) {
    // Should come before fixCrossIterationPHIs().
    completeInMemoryReductions();
    fixCrossIterationPHIs();
  } else
    fixOutgoingValues();

  fixNonInductionPhis();

  updateAnalysis();
  if (!EnableVPValueCodegen)
    // Fix-up external users of the induction variables.
    for (auto &Entry : *Legal->getInductionVars())
      fixupIVUsers(Entry.first, Entry.second,
                   getOrCreateVectorTripCount(LI->getLoopFor(LoopVectorBody)),
                   IVEndValues[Entry.first], LoopMiddleBlock);

  fixLCSSAPHIs();

  fixupLoopPrivates();

  predicateInstructions();
}

void VPOCodeGen::fixCrossIterationPHIs() {
  // In order to support recurrences we need to be able to vectorize Phi nodes.
  // Phi nodes have cycles, so we need to vectorize them in two stages. First,
  // we create a new vector PHI node with no incoming edges. We use this value
  // when we vectorize all of the instructions that use the PHI. Next, after
  // all of the instructions in the block are complete we add the new incoming
  // edges to the PHI. At this point all of the instructions in the basic block
  // are vectorized, so we can use them to construct the PHI.

  // At this point every instruction in the original loop is widened to a
  // vector form. Now we need to fix the recurrences. These PHI nodes are
  // currently empty because we did not want to introduce cycles.
  // This is the second stage of vectorizing recurrences.
  for (Instruction &I : *OrigLoop->getHeader()) {
    PHINode *Phi = dyn_cast<PHINode>(&I);
    if (!Phi)
      break;
    // Handle first-order recurrences and reductions that need to be fixed.
    // TODO: handle first-order recurrences
    if (Legal->isImplicitReductionVariable(Phi))
      fixReductionInReg(Phi, (*Legal->getReductionVars())[Phi]);

    else if (Legal->isExplicitReductionVariable(Phi)) {
      AllocaInst *Ptr = Legal->getReductionPtrByPhi(Phi);
      RecurrenceDescriptor& RD = Legal->getRecurrenceDescrByPhi(Phi);
      if (!Legal->isInMemoryReduction(Ptr))
        fixReductionInReg(Phi, RD);
      else {
        assert(ReductionVecInitVal.count(Ptr) &&
               ReductionEofLoopVal.count(Ptr) &&
               "Reduction is not handled properly");

        Value *VectorStart = ReductionVecInitVal[Ptr];
        fixReductionPhi(Phi, VectorStart);
        mergeReductionControlFlow(Phi, RD, ReductionEofLoopVal[Ptr]);
        Value *LoopExitInst = RD.getLoopExitInstr();
        fixReductionLCSSA(LoopExitInst, ReductionEofLoopVal[Ptr]);
      }
    }
  }
}

void VPOCodeGen::fixReductionPhi(PHINode *Phi, Value *VectorStart) {
  Value *VecRdxPhi = getVectorValue(Phi);
  BasicBlock *Latch = OrigLoop->getLoopLatch();
  Value *LoopVal = Phi->getIncomingValueForBlock(Latch);
  Value *VecLoopVal = getVectorValue(LoopVal);
  cast<PHINode>(VecRdxPhi)->addIncoming(VectorStart, LoopVectorPreHeader);
  auto *VecLp = LI->getLoopFor(LoopVectorBody);
  assert(VecLp && "Unexpected null vector loop");
  auto *LoopVectorLatch = VecLp->getLoopLatch();
  assert(LoopVectorLatch && "Unexpected null vector loop latch");
  cast<PHINode>(VecRdxPhi)->addIncoming(VecLoopVal, LoopVectorLatch);
}

void VPOCodeGen::mergeReductionControlFlow(PHINode *Phi,
                                           RecurrenceDescriptor& RdxDesc,
                                           Value *ReducedPartRdx) {
  Value *ReductionStartValue = RdxDesc.getRecurrenceStartValue();
  // Create a phi node that merges control-flow from the backedge-taken check
  // block and the middle block.
  PHINode *BCBlockPhi = PHINode::Create(ReductionStartValue->getType(), 2,
                                        "bc.merge.rdx",
                                        LoopScalarPreHeader->getTerminator());
  for (unsigned I = 0, E = LoopBypassBlocks.size(); I != E; ++I)
    BCBlockPhi->addIncoming(ReductionStartValue, LoopBypassBlocks[I]);
  BCBlockPhi->addIncoming(ReducedPartRdx, LoopMiddleBlock);

  // Fix the scalar loop reduction variable with the incoming reduction sum
  // from the vector body and from the backedge value.
  int IncomingEdgeBlockIdx = Phi->getBasicBlockIndex(OrigLoop->getLoopLatch());
  assert(IncomingEdgeBlockIdx >= 0 && "Invalid block index");
  // Pick the other block.
  int SelfEdgeBlockIdx = (IncomingEdgeBlockIdx ? 0 : 1);
  Phi->setIncomingValue(SelfEdgeBlockIdx, BCBlockPhi);
  Value *LoopExitInst = RdxDesc.getLoopExitInstr();
  Phi->setIncomingValue(IncomingEdgeBlockIdx, LoopExitInst);
}

void VPOCodeGen::fixReductionLCSSA(Value *LoopExitInst, Value *NewV) {
  // Now, we need to fix the users of the reduction variable
  // inside and outside of the scalar remainder loop.
  // We know that the loop is in LCSSA form. We need to update the
  // PHI nodes in the exit blocks.
  for (BasicBlock::iterator LEI = LoopExitBlock->begin(),
       LEE = LoopExitBlock->end();
       LEI != LEE; ++LEI) {
    PHINode *LCSSAPhi = dyn_cast<PHINode>(LEI);
    if (!LCSSAPhi)
      break;

    // All PHINodes need to have a single entry edge, or two if
    // we already fixed them.
    assert(LCSSAPhi->getNumIncomingValues() < 3 && "Invalid LCSSA PHI");

    // We found our reduction value exit-PHI. Update it with the
    // incoming bypass edge.
    if (LCSSAPhi->getIncomingValue(0) == LoopExitInst) {
      // Add an edge coming from the bypass.
      LCSSAPhi->addIncoming(NewV, LoopMiddleBlock);
      break;
    }
  } // end of the LCSSA phi scan.
}

void VPOCodeGen::fixReductionInReg(PHINode *Phi,
                                   RecurrenceDescriptor& RdxDesc) {
  Constant *Zero = Builder.getInt32(0);

  RecurrenceDescriptor::RecurrenceKind RK = RdxDesc.getRecurrenceKind();
  TrackingVH<Value> ReductionStartValue = RdxDesc.getRecurrenceStartValue();
  Instruction *LoopExitInst = RdxDesc.getLoopExitInstr();
  RecurrenceDescriptor::MinMaxRecurrenceKind MinMaxKind =
    RdxDesc.getMinMaxRecurrenceKind();

  // We need to generate a reduction vector from the incoming scalar.
  // To do so, we need to generate the 'identity' vector and override
  // one of the elements with the incoming scalar reduction. We need
  // to do it in the vector-loop preheader.
  Builder.SetInsertPoint(LoopBypassBlocks[1]->getTerminator());

  // This is the vector-clone of the value that leaves the loop.
  Value *VecExit = getVectorValue(LoopExitInst);
  Type *VecTy = VecExit->getType();

  // Find the reduction identity variable. Zero for addition, or, xor,
  // one for multiplication, -1 for And.
  Value *Identity;
  Value *VectorStart;
  if (RK == RecurrenceDescriptor::RK_IntegerMinMax ||
      RK == RecurrenceDescriptor::RK_FloatMinMax) {
    // MinMax reduction have the start value as their identify.
    VectorStart = Identity =
        Builder.CreateVectorSplat(VF, ReductionStartValue, "minmax.ident");
  } else {
    // Handle other reduction kinds:
    Constant *Iden =
      RecurrenceDescriptor::getRecurrenceIdentity(RK, VecTy->getScalarType());
    Identity = ConstantVector::getSplat(VF, Iden);

    // This vector is the Identity vector where the first element is the
    // incoming scalar reduction.
    VectorStart =
      Builder.CreateInsertElement(Identity, ReductionStartValue, Zero);
  }

  // Fix the vector-loop phi.
  fixReductionPhi(Phi, VectorStart);

  // Before each round, move the insertion point right between
  // the PHIs and the values we are going to write.
  // This allows us to write both PHINodes and the extractelement
  // instructions.
  Builder.SetInsertPoint(&*LoopMiddleBlock->getFirstInsertionPt());

  Value *ReducedPartRdx = reduceVector(VecExit, RK, MinMaxKind, Builder);

  // Create a phi node that merges control-flow from the backedge-taken check
  // block and the middle block.
  mergeReductionControlFlow(Phi, RdxDesc, ReducedPartRdx);

  // Now, we need to fix the users of the reduction variable
  // inside and outside of the scalar remainder loop.
  // We know that the loop is in LCSSA form. We need to update the
  // PHI nodes in the exit blocks.
  fixReductionLCSSA(LoopExitInst, ReducedPartRdx);

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

Value *VPOCodeGen::getBroadcastInstrs(Value *V) {
  // We need to place the broadcast of invariant variables outside the loop.
  Instruction *Instr = dyn_cast<Instruction>(V);
  bool NewInstr = (Instr && NewLoop->contains(Instr));
  bool Invariant = OrigLoop->isLoopInvariant(V) && !NewInstr;

  auto OldIP = Builder.saveIP();
  // Place the code for broadcasting invariant variables in the new preheader.
  IRBuilder<>::InsertPointGuard Guard(Builder);
  if (Invariant)
    Builder.SetInsertPoint(LoopVectorPreHeader->getTerminator());

  // Broadcast the scalar into all locations in the vector.
  Value *Shuf = Builder.CreateVectorSplat(VF, V, "broadcast");

  Builder.restoreIP(OldIP);
  return Shuf;
}

// Get base pointer(s) of in-memory private variable of array-type
// In case we have not allocated the pointers, do so and store them
// in the ScalarMap data-structure.
void VPOCodeGen::createSerialPrivateArrayBase(Value *ArrPriv) {
  assert(VPlanSerializeAlloca && "-vplan-serialize-alloca is FALSE");
  assert(Legal->isLoopPrivate(ArrPriv) && "Loop private value expected");

  if (ScalarMap.count(ArrPriv))
    return;
  auto OldIP = Builder.saveIP();
  Instruction *ArrPrivInst = cast<Instruction>(ArrPriv);
  Type *PointeeTy = ArrPriv->getType()->getPointerElementType();
  Builder.SetInsertPoint(ArrPrivInst->getNextNode());
  for (unsigned I = 0; I < VF; ++I) {
    AllocaInst *SPrivArr = Builder.CreateAlloca(
        PointeeTy, nullptr, ArrPrivInst->getName() + ".vec");
    // Alignment of the alloca should match the original alignment.
    SPrivArr->setAlignment(MaybeAlign(getPrivateVarAlignment(ArrPriv)));
    ScalarMap[ArrPriv][I] = SPrivArr;
  }
  Builder.restoreIP(OldIP);
}

Value *VPOCodeGen::getVectorPrivatePtrs(Value *ScalarPrivate) {
  assert(Legal->isLoopPrivate(ScalarPrivate) && "Loop private value expected");

  if (WidenMap.count(ScalarPrivate))
    return WidenMap[ScalarPrivate];

  auto PtrToVec = getVectorPrivateBase(ScalarPrivate);
  auto PtrType = cast<PointerType>(ScalarPrivate->getType());

  auto Base = Builder.CreateBitCast(PtrToVec, PtrType, "privaddr");

  // If PtrToVec is of an widened array-type,
  // e.g., [2 x [NumElts x Ty]],
  // %privaddr = bitcast [2 x [NumElts x Ty]]* %s.vec to [NumElts x Ty]*
  // %addr.vec = getelementptr [NumElts x Ty], [NumElts x Ty]* %privaddr,
  //                                           <2 x i32> <i32 0, i32 1>
  // We will create a vector GEP with scalar base and a vector of indices.

  SmallVector<Constant *, 8> Indices;
  // Create a vector of consecutive numbers from zero to VF.
  for (unsigned i = 0; i < VF; ++i) {
    Indices.push_back(
        ConstantInt::get(Type::getInt32Ty(PtrType->getContext()), i));
  }
  // Add the consecutive indices to the vector value.
  Constant *Cv = ConstantVector::get(Indices);

  return Builder.CreateGEP(nullptr, Base, Cv);
}

///////////////////////////////////////////////////////////////////////////////
// This function widens the array/struct 'alloca' by VF. It then returns
// the vector containing the base address of each private copy
//
//     %s = alloca [NElts x Ty]      |   %s = alloca {i32, <NElts x Ty>}
//           |                       |                  |
//           | VF = 2                |                  | VF = 8
//           |                       |                  |
//           V                       |                  V
// %s.vec = alloca [2 x [NElts x Ty]]| %s.vec = alloca [8 x {i32, <NElts x Ty>}]
//
// /////////////////////////////////////////////////////////////////////////////
Value *VPOCodeGen::getVectorPrivateAggregateBase(Value *AggrPriv) {
  assert(Legal->isLoopPrivate(AggrPriv) && "Loop private value expected");

  Type *OrigAggTy = AggrPriv->getType()->getPointerElementType();

  assert((isa<ArrayType>(OrigAggTy) || isa<StructType>(OrigAggTy)) &&
         "Expected the private variable to be of array type");

  // Create a VF-wide type. If OrigAggTy is [64 x Ty], VecTyForAlloca
  // is [VF x [64 x Ty]].
  Type *VecTyForAlloca = ArrayType::get(OrigAggTy, VF);

  // Create a PointerType for this
  Type *PtrToVecAllocaTy = PointerType::get(
      VecTyForAlloca,
      cast<PointerType>(AggrPriv->getType())->getAddressSpace());

  Value *V = getPtrThruCast<BitCastInst>(AggrPriv);

  // If we have already created a widened private Array, return it.
  if (LoopPrivateWidenMap.count(V)) {
    Value *BitCastPtr = LoopPrivateWidenMap[V];
    return Builder.CreateBitCast(BitCastPtr, PtrToVecAllocaTy);
  }

  // Create an alloca in the appropriate block
  auto OldIP = Builder.saveIP();
  Builder.SetInsertPoint(&*(getFunctionEntryBlock().getFirstInsertionPt()));
  AllocaInst *WidenedPrivArr = Builder.CreateAlloca(
      VecTyForAlloca, nullptr, AggrPriv->getName() + ".vec");
  // Alignment of the alloca should match the original alignment.
  WidenedPrivArr->setAlignment(MaybeAlign(getPrivateVarAlignment(AggrPriv)));

  // Save alloca's result
  LoopPrivateWidenMap[AggrPriv] = WidenedPrivArr;

  if (Legal->isCondLastPrivate(AggrPriv)) {
    Builder.SetInsertPoint(WidenedPrivArr->getNextNode());
    // Create a memory location for last non-zero mask
    // We save mask as an integer value
    Type *MaskTy = IntegerType::get(AggrPriv->getContext(), VF);
    Value *PtrToMask =
        Builder.CreateAlloca(MaskTy, nullptr, AggrPriv->getName() + ".mask");
    Builder.CreateStore(Constant::getAllOnesValue(MaskTy), PtrToMask);
    LoopPrivateLastMask[V] = PtrToMask;
  }

  Builder.restoreIP(OldIP);
  return WidenedPrivArr;
}

Value *VPOCodeGen::getVectorPrivateBase(Value *V) {
  assert(Legal->isLoopPrivate(V) && "Loop private value expected");
  bool IsConditional = Legal->isCondLastPrivate(V);

  Type *TypeBeforeBitcast = V->getType();
  Type *ValueTy = TypeBeforeBitcast->getPointerElementType();

  if (isa<ArrayType>(ValueTy) || isa<StructType>(ValueTy))
    return getVectorPrivateAggregateBase(V);

  Type *NewValueTy = ValueTy->isVectorTy()
                         ? VectorType::get(ValueTy->getScalarType(),
                                           ValueTy->getVectorNumElements() * VF)
                         : VectorType::get(ValueTy, VF);

  Type *NewType = PointerType::get(NewValueTy, 0);

  V = getPtrThruCast<BitCastInst>(V);

  if (LoopPrivateWidenMap.count(V)) {
    Value *PtrToVec = LoopPrivateWidenMap[V];
    return Builder.CreateBitCast(PtrToVec, NewType);
  }

  // If V is an alloca ptr for a loop private, alloca a VF wide vector and
  // use this alloca'd ptr as the vector value.
  auto OldIP = Builder.saveIP();
  auto OrigAllocaTy = V->getType()->getPointerElementType();
  unsigned OriginalVL = OrigAllocaTy->isVectorTy() ?
      OrigAllocaTy->getVectorNumElements() : 1;

  auto VecTyForAlloca = VectorType::get(OrigAllocaTy->getScalarType(),
                                        OriginalVL * VF);
  Builder.SetInsertPoint(&*(getFunctionEntryBlock().getFirstInsertionPt()));
  AllocaInst *PtrToVec = cast<AllocaInst>(
      Builder.CreateAlloca(VecTyForAlloca, nullptr, V->getName() + ".vec"));

  // Alignment of vector alloca should match the original alignment
  PtrToVec->setAlignment(MaybeAlign(getPrivateVarAlignment(V)));

  // Save alloca's result
  LoopPrivateWidenMap[V] = PtrToVec;

  Builder.SetInsertPoint(LoopVectorPreHeader->getTerminator());
  // Broadcast the initial value through the vector

  LoadInst *LoadInit = Builder.CreateLoad(V, V->getName() + "InitVal");

  if (IsConditional) {
    Builder.SetInsertPoint((cast<Instruction>(PtrToVec))->getNextNode());
    // Create a memory location for last non-zero mask
    // We save mask as an integer value
    Type *MaskTy = IntegerType::get(V->getContext(), VF);
    Value *PtrToMask =
        Builder.CreateAlloca(MaskTy, nullptr, V->getName() + ".mask");
    Builder.CreateStore(Constant::getAllOnesValue(MaskTy), PtrToMask);
    LoopPrivateLastMask[V] = PtrToMask;
  }
  // Spread the initial value over the vector for in-memory reduction as well.
  Builder.SetInsertPoint(LoopVectorPreHeader->getTerminator());
  if (Legal->isInMemoryReduction(V)) {
    Value *InitVec;
      RecurrenceDescriptor::RecurrenceKind RK =
          (*Legal->getInMemoryReductionVars())[cast<AllocaInst>(V)].first;
      if (RK == RecurrenceDescriptor::RK_IntegerMinMax ||
          RK == RecurrenceDescriptor::RK_FloatMinMax) {
        InitVec = createVectorSplat(LoadInit, VF, Builder, "InitVec");
      } else {
        Constant *Iden = RecurrenceDescriptor::getRecurrenceIdentity(
            RK, OrigAllocaTy->getScalarType());
        InitVec = ConstantVector::getSplat(VF, Iden);
        Constant *Zero = Builder.getInt32(0);
        InitVec = Builder.CreateInsertElement(InitVec, LoadInit, Zero);
      }
      Builder.CreateStore(InitVec, PtrToVec);
      ReductionVecInitVal[cast<AllocaInst>(V)] = InitVec;
  } else {
    // Private variable - spread initial value though the whole vector
    Value *InitVec = createVectorSplat(LoadInit, VF, Builder);
    Builder.CreateStore(InitVec, PtrToVec);
  }

  Value *BitCastPtrToVec = Builder.CreateBitCast(PtrToVec, NewType);

  Builder.restoreIP(OldIP);
  return BitCastPtrToVec;
}

template<typename CastInstTy>
void VPOCodeGen::vectorizeCast(Instruction *Inst) {

  // In-case we are dealing with loop-private arrays, serialize the execution of
  // the bitcast instruction. This would result in bitcast of each private-copy
  // to the target-type
  Value *SrcOp = Inst->getOperand(0);
  if (isSerializedPrivateArray(SrcOp)) {
    serializeInstruction(Inst, true);
    return;
  }
  Value *VecOp = getVectorValue(SrcOp);
  unsigned NumElts = Inst->getType()->isVectorTy()
                         ? Inst->getType()->getVectorNumElements() * VF
                         : VF;

  Type *VecTy = VectorType::get(Inst->getType()->getScalarType(), NumElts);
  if (std::is_same<CastInstTy, BitCastInst>::value)
    WidenMap[Inst] = Builder.CreateBitCast(VecOp, VecTy);
  else if (std::is_same<CastInstTy, AddrSpaceCastInst>::value)
    WidenMap[Inst] = Builder.CreateAddrSpaceCast(VecOp, VecTy);
  else
    llvm_unreachable("Expecting either a BitCastInst or AddrSpaceCastInst");
}

Value *VPOCodeGen::getVectorValue(VPValue *V) {
  if (EnableVPValueCodegen)
    return getVectorValueUplifted(V);

  if (V->isUnderlyingIRValid())
    return getVectorValue(V->getUnderlyingValue());

  Value *VecV = VPWidenMap[V];
  assert(VecV && "Value not in VPWidenMap");
  return VecV;
}

Value *VPOCodeGen::getVectorValue(Value *V) {
  if (EnableVPValueCodegen) {
    LLVM_DEBUG(dbgs() << "LV: Value: "; V->dump(); dbgs() << "LV: Parent: ";
               cast<Instruction>(V)->getParent()->dump());
    llvm_unreachable("Implementing VPValue codegen uplift.");
  }

  // If we have this scalar in the map, return it.
  if (WidenMap.count(V))
    return WidenMap[V];

  // Address of in memory private is needed. Construct a vector of addresses
  // on the fly.
  if (Legal->isLoopPrivate(V)) {
    Value *VectorValue = getVectorPrivatePtrs(V);
    WidenMap[V] = VectorValue;
    return VectorValue;
  }

  // If the value has not been vectorized, check if it has been scalarized
  // instead. If it has been scalarized, and we actually need the value in
  // vector form, we will construct the vector values on demand.
  if (ScalarMap.count(V)) {
    bool IsUniform = isUniformAfterVectorization(cast<Instruction>(V), VF) ||
      OrigLoop->hasLoopInvariantOperands(cast<Instruction>(V));

    Value *VectorValue = nullptr;
    IRBuilder<>::InsertPointGuard Guard(Builder);
    if (IsUniform) {
      Value *ScalarValue = ScalarMap[V][0];
      assert(isa<Instruction>(ScalarValue) && "Expected instruction for scalar value");
      Builder.SetInsertPoint((cast<Instruction>(ScalarValue))->getNextNode());
      if (ScalarValue->getType()->isVectorTy()) {
        VectorValue =
            replicateVector(ScalarValue, VF, Builder,
                                "replicatedVal." + ScalarValue->getName());
      } else
        VectorValue = Builder.CreateVectorSplat(VF, ScalarValue, "broadcast");
    } else if (V->getType()->isVectorTy()) {
      SmallVector<Value *, 8> Parts;
      for (unsigned Lane = 0; Lane < VF; ++Lane)
        Parts.push_back(ScalarMap[V][Lane]);
      Value *ScalarValue = ScalarMap[V][VF-1];
      assert(isa<Instruction>(ScalarValue) && "Expected instruction for scalar value");
      Builder.SetInsertPoint((cast<Instruction>(ScalarValue))->getNextNode());
      VectorValue = joinVectors(Parts, Builder);
    } else {
      VectorValue = UndefValue::get(VectorType::get(V->getType(), VF));
      for (unsigned Lane = 0; Lane < VF; ++Lane) {
        Value *ScalarValue = ScalarMap[V][Lane];
        assert(isa<Instruction>(ScalarValue) && "Expected instruction for scalar value");
        Builder.SetInsertPoint((cast<Instruction>(ScalarValue))->getNextNode());
        VectorValue = Builder.CreateInsertElement(VectorValue, ScalarValue,
                                                  Builder.getInt32(Lane));
      }
    }

    WidenMap[V] = VectorValue;
    return VectorValue;
  }

  // If this scalar is unknown, assume that it is a constant or that it is
  // loop invariant. Broadcast V and save the value for future uses.
  if (V->getType()->isVectorTy()) {
    assert(V->getType()->getVectorElementType()->isSingleValueType() &&
           "Re-vectorization is supported for simple vectors only");
    // Widen the uniform vector variable as following
    //                        <i32 0, i32 1>
    //                             |
    //                             |VF = 4
    //                             |
    //                             V
    //          <i32 0, i32 1,i32 0, i32 1,i32 0, i32 1,i32 0, i32 1>
    WidenMap[V] =
        replicateVector(V, VF, Builder, "replicatedVal." + V->getName());
  } else
    WidenMap[V] = getBroadcastInstrs(V);

  return WidenMap[V];
}

Value *VPOCodeGen::getScalarValue(VPValue *V, unsigned Lane) {
  if (EnableVPValueCodegen)
    return getScalarValueUplifted(V, Lane);

  if (V->isUnderlyingIRValid())
    return getScalarValue(V->getUnderlyingValue(), Lane);
  else {
    Value *VecV = getVectorValue(V);
    IRBuilder<>::InsertPointGuard Guard(Builder);
    if (auto VecInst = dyn_cast<Instruction>(VecV)) {
      if (isa<PHINode>(VecInst))
        Builder.SetInsertPoint(&*(VecInst->getParent()->getFirstInsertionPt()));
      else
        Builder.SetInsertPoint(VecInst->getNextNode());
    }
    return Builder.CreateExtractElement(VecV, Builder.getInt32(Lane));
  }
}

// TODO: Consider renaming this function to 'getScalarOrSubvectorValue' as this
// function can now return a sub-vector in addition to a scalar value
Value *VPOCodeGen::getScalarValue(Value *V, unsigned Lane) {
  if (EnableVPValueCodegen) {
    LLVM_DEBUG(dbgs() << "LV: Value: "; V->dump(); dbgs() << "LV: Parent: ";
               cast<Instruction>(V)->getParent()->dump());
    llvm_unreachable("Implementing VPValue codegen uplift.");
  }

  // If the value is not an instruction contained in the loop, it should
  // already be scalar.
  if (OrigLoop->isLoopInvariant(V) && !Legal->isLoopPrivate(V))
    return V;

  // InCase we have not created the serial alloca's for array-types already,
  // create them and populate the ScalarMap.
  if (!ScalarMap.count(V) && isSerializedPrivateArray(V))
    createSerialPrivateArrayBase(V);

  if (ScalarMap.count(V)) {
    auto SV = ScalarMap[V];

    if (auto *Inst = dyn_cast<Instruction>(V))
      if (isUniformAfterVectorization(Inst, VF))
        // For uniform instructions the mapping is updated for lane zero only.
        Lane = 0;

    if (SV.count(Lane))
      return SV[Lane];
  }

  if (Legal->isInductionVariable(V))
    return buildScalarIVForLane(cast<PHINode>(V), Lane);

  // Get the scalar value by extracting from the vector instruction based on the
  // requested lane.
  Value *VecV = getVectorValue(V);

  // This code assumes that the widened vector, that we are extracting from has
  // data in AOS layout. If OriginalVL = 2, VF = 4 the widened value would be
  // Wide.Val = <v1_0, v2_0, v1_1, v2_1, v1_2, v2_2, v1_3, v2_3>.
  // getScalarValue(Wide.Val, 1) would return <v1_1, v2_1>
  if (V->getType()->isVectorTy()) {
    unsigned OrigNumElts = V->getType()->getVectorNumElements();
    SmallVector<unsigned, 8> ShufMask;
    for (unsigned StartIdx = Lane * OrigNumElts,
                  EndIdx = (Lane * OrigNumElts) + OrigNumElts;
         StartIdx != EndIdx; ++StartIdx)
      ShufMask.push_back(StartIdx);

    Value *Shuff = Builder.CreateShuffleVector(
        VecV, UndefValue::get(VecV->getType()), ShufMask,
        "extractsubvec.");

    ScalarMap[V][Lane] = Shuff;

    return Shuff;
  }

  IRBuilder<>::InsertPointGuard Guard(Builder);
  if (auto VecInst = dyn_cast<Instruction>(VecV)) {
    if (isa<PHINode>(VecInst))
      Builder.SetInsertPoint(&*(VecInst->getParent()->getFirstInsertionPt()));
    else
      Builder.SetInsertPoint(VecInst->getNextNode());
  }
  auto ScalarV = Builder.CreateExtractElement(VecV, Builder.getInt32(Lane));

  // Add to scalar map
  ScalarMap[V][Lane] = ScalarV;
  return ScalarV;
}

/// Reverse vector \p Vec. \p OriginalVL specifies the original vector length
/// of the value before vectorization.
/// If the original value was scalar, a vector <A0, A1, A2, A3> will be just
/// reversed to <A3, A2, A1, A0>. If the original value was a vector
/// (OriginalVL > 1), the function will do the following:
/// <A0, B0, A1, B1, A2, B2, A3, B3> -> <A3, B3, A2, B2, A1, B1, A0, B0>
Value *VPOCodeGen::reverseVector(Value *Vec, unsigned OriginalVL) {
  unsigned NumElts = Vec->getType()->getVectorNumElements();
  SmallVector<Constant *, 8> ShuffleMask;
  for (unsigned i = 0; i < NumElts; i += OriginalVL)
    for (unsigned j = 0; j < OriginalVL; j++)
      ShuffleMask.push_back(
          Builder.getInt32(NumElts - (i + 1) * OriginalVL + j));

  return Builder.CreateShuffleVector(Vec, UndefValue::get(Vec->getType()),
                                     ConstantVector::get(ShuffleMask),
                                     "reverse");
}

// Return Value indicating that the mask is not all-zero
static Value *isNotAllZeroMask(IRBuilder<> &Builder, Value *MaskValue,
                               Value *&MaskInInt) {
  unsigned VF = MaskValue->getType()->getVectorNumElements();
  Type *IntTy = IntegerType::get(MaskValue->getContext(), VF);
  MaskInInt = Builder.CreateBitCast(MaskValue, IntTy);
  Value *NotAllZ = Builder.CreateICmp(CmpInst::ICMP_NE, MaskInInt,
                                      ConstantInt::get(IntTy, 0));
  return NotAllZ;
}

void storeMaskValue(Value *MaskValue, Value *Ptr, unsigned VF,
                    IRBuilder<> &Builder) {
  Value *MaskToStore = nullptr;
  if (MaskValue) {
    Value *MaskInInt = nullptr;
    Value *NotAllZero = isNotAllZeroMask(Builder, MaskValue, MaskInInt);

    // Store the last written lane
    // We store only non-zero mask.
    Value *PrevMask = Builder.CreateLoad(Ptr);
    MaskToStore = Builder.CreateSelect(NotAllZero, MaskInInt, PrevMask);
  } else {
    Type *MaskTy = IntegerType::get(Ptr->getContext(), VF);
    MaskToStore = Constant::getAllOnesValue(MaskTy);
  }
  Builder.CreateStore(MaskToStore, Ptr);
}

// This function returns computed addresses of memory locations which should be
// accessed in the vectorized code. These addresses, take the form of a GEP
// instruction, and this GEP is used as pointer operand of the resulting
// scatter/gather intrinsic.
Value *VPOCodeGen::createWidenedGEPForScatterGather(Instruction *I) {
  assert((isa<LoadInst>(I) || isa<StoreInst>(I)) &&
         "Expect 'I' to be either a LoadInst or a StoreInst");
  Type *LSIType = getLoadStoreType(I);

  assert(
      isa<VectorType>(LSIType) &&
      "Expect the original type of Load/Store instruction to be a vector-type");

  Value *BasePtr = getPointerOperand(I);

  unsigned AddrSpace = cast<PointerType>(BasePtr->getType())->getAddressSpace();

  // Vectorize BasePtr.
  BasePtr = getVectorValue(BasePtr);

  // Cast the inner vector-type to it's elemental scalar type
  // e.g. - <VF x <OriginalVL x Ty> addrspace(x)*>
  //                          |
  //                          |
  //                          V
  //                <VF x Ty addrspace(x)*>
  Value *TypeCastBasePtr = Builder.CreateBitCast(
      BasePtr,
      VectorType::get(LSIType->getVectorElementType()->getPointerTo(AddrSpace),
                      VF));
  // Replicate the base-address OriginalVL times
  //                <VF x Ty addrspace(x)*>
  //                          |
  //                          |
  //                          V
  //      < 0, 1, .., OriginalVL-1, ..., 0, 1, ..., OriginalVL-1>

  unsigned OriginalVL = LSIType->getVectorNumElements();
  Value *VecBasePtr =
      replicateVectorElts(TypeCastBasePtr, OriginalVL, Builder, "vecBasePtr.");

  // Create a vector of consecutive numbers from zero to OriginalVL-1 repeated
  // VF-times.
  SmallVector<Constant *, 32> Indices;
  for (unsigned J = 0; J < VF; ++J)
    for (unsigned I = 0; I < OriginalVL; ++I)
      Indices.push_back(
          ConstantInt::get(Type::getInt64Ty(LSIType->getContext()), I));

  // Add the consecutive indices to the vector value.
  Constant *Cv = ConstantVector::get(Indices);

  // Create a GEP that would return the address of each elements that is to be
  // accessed.
  Value *WidenedVectorGEP = Builder.CreateGEP(VecBasePtr, Cv, "elemBasePtr.");
  return WidenedVectorGEP;
}

// This function return an appropriate BasePtr for cases where we are dealing
// with load/store to consecutive memory locations
Value *VPOCodeGen::createWidenedBasePtrConsecutiveLoadStore(Instruction *I,
                                                            Value *Ptr,
                                                            bool Reverse) {
  Type *VecTy = getLoadStoreType(I);
  unsigned AddrSpace = getLoadStoreAddressSpace(I);
  unsigned OriginalVL =
      isa<VectorType>(VecTy) ? VecTy->getVectorNumElements() : 1;
  Type *SubTy = isa<VectorType>(VecTy) ? VecTy->getVectorElementType() : VecTy;
  Type *WideDataTy = VectorType::get(SubTy, VF * OriginalVL);
  Value *VecPtr = nullptr;
  if (Legal->isLoopPrivate(Ptr))
    // 'isLoopPrivate(Ptr)' returns true only for scalar privates.
    VecPtr = getVectorPrivateBase(Ptr);
  else
    // We do not care whether the 'Ptr' operand comes from a GEP or any other
    // source. We just fetch the first element and then create a
    // bitcast  which assumes the 'consecutive-ness' property and return the
    // correct operand for widened load/store.
    VecPtr = getScalarValue(Ptr, 0);

  VecPtr =
      Reverse ? Builder.CreateGEP(VecPtr, Builder.getInt32(1 - OriginalVL * VF),
                                  "reverse.ptr.")
              : VecPtr;
  VecPtr = Builder.CreateBitCast(VecPtr, WideDataTy->getPointerTo(AddrSpace));
  return VecPtr;
}

void VPOCodeGen::widenVectorStore(StoreInst *SI) {
  Value *Ptr = SI->getPointerOperand();
  unsigned Alignment = SI->getAlignment();
  // An alignment of 0 means target abi alignment. We need to use the scalar's
  // target abi alignment in such a case.
  const DataLayout &DL = SI->getModule()->getDataLayout();
  Value *DataOp = SI->getValueOperand();
  if (!Alignment)
    Alignment = DL.getABITypeAlignment(DataOp->getType());

  Value *VecDataOp = getVectorValue(DataOp);
  Type *WideDataTy = VecDataOp->getType();
  unsigned OriginalVL = WideDataTy->getVectorNumElements() / VF;

  int ConsecutiveStride = Legal->isConsecutivePtr(Ptr);
  if (ConsecutiveStride) {
    Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(
        SI, Ptr, ConsecutiveStride == -1);

    // We replicate the mask-value as {m0, m0, m1, m1, m2, m2, m3, m3}.
    Value *WidenedMask =
        MaskValue ? replicateVectorElts(MaskValue, OriginalVL, Builder,
                                        "replicatedMaskElts.")
                  : nullptr;

    if (ConsecutiveStride == -1) // Reverse
      VecDataOp = reverseVector(VecDataOp, OriginalVL);

    if (WidenedMask) {
      Builder.CreateMaskedStore(VecDataOp, VecPtr, Alignment, WidenedMask);
    } else
      Builder.CreateAlignedStore(VecDataOp, VecPtr, Alignment);

    // Store the mask value in case of a last-private.
    if (Legal->isCondLastPrivate(Ptr))
      storeMaskValue(MaskValue, LoopPrivateLastMask[getPtrThruCast<BitCastInst>(Ptr)], VF,
                     Builder);

  } else {
    // We replicate the mask-value as {m0, m1, m2, m3, m0, m1, m2, m3}.
    Value *WidenMask = MaskValue
                           ? replicateVectorElts(MaskValue, OriginalVL, Builder,
                                                 "replicatedMaskVecElts.")
                           : nullptr;
    Value *VectorGEP = createWidenedGEPForScatterGather(SI);
    Builder.CreateMaskedScatter(VecDataOp, VectorGEP, Alignment, WidenMask);
  }
}

void VPOCodeGen::widenVectorLoad(LoadInst *LI) {
  Value *Ptr = LI->getPointerOperand();
  unsigned Alignment = LI->getAlignment();
  // An alignment of 0 means target abi alignment. We need to use the scalar's
  // target abi alignment in such a case.
  const DataLayout &DL = LI->getModule()->getDataLayout();
  if (!Alignment)
    Alignment = DL.getABITypeAlignment(LI->getType());

  Type *VecTy = getLoadStoreType(LI);
  Type *ScalarTy = VecTy->getVectorElementType();
  unsigned OriginalVL = VecTy->getVectorNumElements();
  if (!ScalarTy->isSingleValueType())
    llvm_unreachable("Re-vectorization supports simple vectors only!");
  int ConsecutiveStride = Legal->isConsecutivePtr(Ptr);

  Value *NewLI = nullptr;
  if (ConsecutiveStride) {
    Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(
        LI, Ptr, ConsecutiveStride == -1);

    if (MaskValue && !Legal->isLoopPrivate(Ptr)) {
      // Masking not needed for privates.
      // Mask value should be replicated for each element.
      Value *RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                                "replicatedMaskElts.");
      NewLI = Builder.CreateMaskedLoad(VecPtr, Alignment, RepMaskValue, nullptr,
                                       "wide.masked.load");
    } else
      NewLI = Builder.CreateAlignedLoad(VecPtr, Alignment, "wide.load");
    if (ConsecutiveStride == -1)
      NewLI = reverseVector(NewLI, OriginalVL);
  } else {
    // We replicate the mask-value as {m0, m0, m1, m1, m2, m2, m3, m3}.
    Value *WidenMask = MaskValue
                           ? replicateVectorElts(MaskValue, OriginalVL, Builder,
                                                 "replicatedMaskVecElts.")
                           : nullptr;
    Value *VectorGEP = createWidenedGEPForScatterGather(LI);
    NewLI = Builder.CreateMaskedGather(VectorGEP, Alignment, WidenMask, nullptr,
                                       "wide.masked.gather");
  }
  WidenMap[LI] = NewLI;
}

void VPOCodeGen::vectorizeLinearLoad(Instruction *LinLdInst, int LinStep) {
  Instruction *LinLdClone = LinLdInst->clone();
  LinLdClone->setName(LinLdInst->getName() + "linload.clone");
  Builder.Insert(LinLdClone);

  // Generate vector value for the linear value loaded by broadcasting it and adding
  // LaneNum * LinStep
  auto LinValTy = LinLdClone->getType();
  Value *BroadcastVal = Builder.CreateVectorSplat(VF, LinLdClone);
  SmallVector<Constant *, 8> LinSteps;
  // Create the vector of steps from zero to VF in increments of LinStep.
  for (unsigned LaneNum = 0; LaneNum < VF; ++LaneNum) {
    LinSteps.push_back(ConstantInt::get(LinValTy, LaneNum * LinStep));
  }

  Constant *Cv = ConstantVector::get(LinSteps);

  auto LinVecValue = Builder.CreateAdd(BroadcastVal, Cv, "vec.linear");
  WidenMap[LinLdInst] = LinVecValue;

  // Add to UnitStepLinears if LinStep is 1/-1 - so that we can use it to infer
  // information about unit stride loads/stores
  if (LinStep == 1 || LinStep == -1) {
    addUnitStepLinear(LinLdInst, LinLdClone, LinStep);
  }
}

void VPOCodeGen::vectorizeLoadInstruction(Instruction *Inst,
                                          bool EmitIntrinsic) {
  LoadInst *LI = cast<LoadInst>(Inst);
  Value *Ptr = LI->getPointerOperand();
  int LinStride = 0;

  // Handle vectorization of a linear value load
  if (Legal->isLinear(Ptr, &LinStride)) {
    vectorizeLinearLoad(Inst, LinStride);
    return;
  }

  auto *GEP = getGEPInstruction(Ptr);

  if (!Legal->isLoopPrivateAggregate(GEP ? getPointerOperand(GEP) : Ptr) &&
      (Legal->isLoopInvariant(Ptr) || Legal->isUniformForTheLoop(Ptr))) {
    if (MaskValue)
      serializePredicatedUniformLoad(Inst);
    else
      serializeInstruction(Inst);
    return;
  }

  int ConsecutiveStride = Legal->isConsecutivePtr(Ptr);

  if (LI->getType()->isVectorTy())
    return widenVectorLoad(LI);

  unsigned Alignment = LI->getAlignment();
  // An alignment of 0 means target abi alignment. We need to use the scalar's
  // target abi alignment in such a case.
  const DataLayout &DL = Inst->getModule()->getDataLayout();
  if (!Alignment)
    Alignment = DL.getABITypeAlignment(LI->getType());

  Value *NewLI = nullptr;

  // Handle loads from privatized array-types
  if (GEP && isSerializedPrivateArray(getPointerOperand(GEP)) &&
      ScalarMap.count(GEP)) {
    Value *WideLoad = UndefValue::get(VectorType::get(Inst->getType(), VF));
    for (unsigned I = 0; I < VF; ++I) {
      Value *LI = Builder.CreateAlignedLoad(Inst->getType(), ScalarMap[GEP][I],
                                            Alignment, "LI");
      WideLoad = Builder.CreateInsertElement(WideLoad, LI, Builder.getInt64(I));
    }
    WidenMap[LI] = WideLoad;
    return;
  }

  // Handle consecutive loads.
  if (ConsecutiveStride) {
    Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(
        LI, Ptr, ConsecutiveStride == -1);

    if (MaskValue && !Legal->isLoopPrivate(Ptr)) {
      // Masking not needed for privates.
      NewLI = Builder.CreateMaskedLoad(VecPtr, Alignment, MaskValue, nullptr,
                                       "wide.masked.load");
    } else
      NewLI = Builder.CreateAlignedLoad(VecPtr, Alignment, "wide.load");

    if (ConsecutiveStride == -1) //Reverse
      NewLI = reverseVector(NewLI);

    WidenMap[cast<Value>(Inst)] = NewLI;
  } else {

    // GATHER
    Value *VectorPtr = getVectorValue(Ptr);
    NewLI = Builder.CreateMaskedGather(VectorPtr, Alignment, MaskValue, nullptr,
                                       "wide.masked.gather");
  }

  WidenMap[cast<Value>(Inst)] = NewLI;
}

void VPOCodeGen::vectorizeSelectInstruction(Instruction *Inst) {
  SelectInst *SelectI = cast<SelectInst>(Inst);
  // If the selector is loop invariant we can create a select
  // instruction with a scalar condition. Otherwise, use vector-select.
  Value *Cond = SelectI->getOperand(0);
  Value *VCond = getVectorValue(Cond);
  Value *Op0 = getVectorValue(SelectI->getOperand(1));
  Value *Op1 = getVectorValue(SelectI->getOperand(2));

  bool InvariantCond =
    Legal->isLoopInvariant(Cond);

  // The condition can be loop invariant  but still defined inside the
  // loop. This means that we can't just use the original 'cond' value.

  if (InvariantCond)
    VCond = getScalarValue(Cond, 0);
  else if (Inst->getType()->isVectorTy()) {
    unsigned OriginalVL = Inst->getType()->getVectorNumElements();

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

  WidenMap[Inst] = NewSelect;
}

void VPOCodeGen::vectorizeLinearStore(Instruction *Inst) {
  StoreInst *SI = cast<StoreInst>(Inst);
  Value *Ptr = SI->getPointerOperand();

  // Store the value that corresponds to lane 0 - any subsequent loads
  // will add in the linear step when generating the vector value for the load.
  Value *ValToStore =   getScalarValue(SI->getValueOperand(), 0);

  // If the store is masked, blend using the current linear value so that we can
  // do an unconditional store.
  if (MaskValue) {
    auto ScalMask =  Builder.CreateExtractElement(MaskValue, Builder.getInt32(VF-1), "lin.mask");
    auto CurrVal = Builder.CreateLoad(Ptr);

    ValToStore = Builder.CreateSelect(ScalMask, ValToStore,CurrVal);
  }

  Builder.CreateStore(ValToStore, Ptr);
}

void VPOCodeGen::vectorizeStoreInstruction(Instruction *Inst,
                                           bool EmitIntrinsic) {
  StoreInst *SI = cast<StoreInst>(Inst);
  Value *Ptr = SI->getPointerOperand();

  // Handle vectorization of a linear value store
  if (Legal->isLinear(Ptr)) {
    vectorizeLinearStore(Inst);
    return;
  }

  int ConsecutiveStride = Legal->isConsecutivePtr(Ptr);

  const DataLayout &DL = Inst->getModule()->getDataLayout();
  if (SI->getValueOperand()->getType()->isVectorTy())
    return widenVectorStore(SI);

  Type *ScalarDataTy = SI->getValueOperand()->getType();

  unsigned Alignment = SI->getAlignment();
  if (!Alignment)
    Alignment = DL.getABITypeAlignment(ScalarDataTy);
  Value *VecDataOp = getVectorValue(SI->getValueOperand());

  // Handle stores to privatized array-types
  Value *GEP = getGEPInstruction(getPointerOperand(Inst));
  if (GEP && isSerializedPrivateArray(getPointerOperand(GEP)) &&
      ScalarMap.count(GEP)) {
    for (unsigned I = 0; I < VF; ++I) {
      Builder.CreateAlignedStore(
          Builder.CreateExtractElement(VecDataOp, Builder.getInt64(I), "SV"),
          ScalarMap[GEP][I], Alignment);
    }
    return;
  }

  // Handle consecutive stores.
  if (ConsecutiveStride) {
    bool StoreMaskValue = Legal->isCondLastPrivate(Ptr);
    Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(
        SI, Ptr, ConsecutiveStride == -1);

    if (ConsecutiveStride == -1) // Reverse
      // If we store to reverse consecutive memory locations, then we need
      // to reverse the order of elements in the stored value.
      VecDataOp = reverseVector(VecDataOp);

    if (MaskValue)
      Builder.CreateMaskedStore(VecDataOp, VecPtr, Alignment, MaskValue);
    else
      Builder.CreateAlignedStore(VecDataOp, VecPtr, Alignment);

    if (StoreMaskValue)
      storeMaskValue(MaskValue, LoopPrivateLastMask[getPtrThruCast<BitCastInst>(Ptr)], VF,
                     Builder);
  } else {

    // SCATTER
    Value *VectorPtr = getVectorValue(Ptr);
    Type *PtrToElemTy = VectorPtr->getType()->getVectorElementType();
    Type *ElemTy = PtrToElemTy->getPointerElementType();
    VectorType *DesiredDataTy = VectorType::get(ElemTy, VF);
    VecDataOp = Builder.CreateBitCast(VecDataOp, DesiredDataTy, "cast");

    Builder.CreateMaskedScatter(VecDataOp, VectorPtr, Alignment, MaskValue);
  }
}

void VPOCodeGen::vectorizeExtractElement(Instruction *Inst) {
  ExtractElementInst *ExtrEltInst = cast<ExtractElementInst>(Inst);
  Value *ExtrFrom = getVectorValue(ExtrEltInst->getVectorOperand());
  Value *OrigIndexVal = ExtrEltInst->getIndexOperand();

  // In case of an non-const index, we serialize the instruction.
  // We first get the actual index, for the vectorized data using
  // 'add', extract the element using the index and then finally insert it into
  // the narrower sub-vector
  if (!isa<ConstantInt>(OrigIndexVal)) {
    Value *WideExtract =
        UndefValue::get(VectorType::get(ExtrEltInst->getType(), VF));
    Value *IndexValVec = getVectorValue(OrigIndexVal);
    for (unsigned VIdx = 0; VIdx < VF; ++VIdx) {
      Value *IndexVal = Builder.CreateExtractElement(IndexValVec, VIdx);
      Value *VectorIdx = Builder.CreateAdd(
          ConstantInt::get(IndexVal->getType(), VIdx * VF), IndexVal);
      WideExtract = Builder.CreateInsertElement(
          WideExtract, Builder.CreateExtractElement(ExtrFrom, VectorIdx), VIdx);
    }
    WidenMap[Inst] = WideExtract;
    return;
  }

  unsigned Index = cast<ConstantInt>(OrigIndexVal)->getZExtValue();

  // Extract subvector. The subvector should include VF elements.
  SmallVector<unsigned, 8> ShufMask;
  unsigned OriginalVL =
      ExtrEltInst->getOperand(0)->getType()->getVectorNumElements();
  unsigned WideNumElts = VF * OriginalVL;
  for (unsigned Idx = Index; Idx < WideNumElts; Idx += OriginalVL)
    ShufMask.push_back(Idx);
  Type *VTy = ExtrFrom->getType();
  WidenMap[Inst] = Builder.CreateShuffleVector(ExtrFrom, UndefValue::get(VTy),
                                               ShufMask, "wide.extract");
}

void VPOCodeGen::vectorizeShuffle(Instruction *Inst) {
  ShuffleVectorInst *Shuf = cast<ShuffleVectorInst>(Inst);
  unsigned OriginalVL = Shuf->getOperand(0)->getType()->getVectorNumElements();
  // Simple case - broadcast scalar elt into vector.
  if (getSplatValue(Inst)) {

    Value *SplVal = cast<InsertElementInst>(Shuf->getOperand(0))->getOperand(1);
    Value *Vec = getVectorValue(SplVal);
    SmallVector<unsigned, 8> ShufMask;
    for (unsigned i = 0; i < OriginalVL; ++i)
      for (unsigned j = 0; j < VF; ++j)
        ShufMask.push_back(j);

    WidenMap[Inst] = Builder.CreateShuffleVector(Vec,
                                                 UndefValue::get(Vec->getType()),
                                                 ShufMask);
    return;
  }

  Value *V0 = getVectorValue(Shuf->getOperand(0));

  Constant *Mask = Shuf->getMask();
  int InstVL = Inst->getType()->getVectorNumElements();
  // All-zero mask case
  if (isa<ConstantAggregateZero>(Mask)) {
    SmallVector<unsigned, 8> ShufMask;
    int Repeat = InstVL / OriginalVL;
    for (int k = 0; k < Repeat; k++)
      for (unsigned i = 0; i < OriginalVL; ++i)
        for (unsigned j = 0; j < VF; ++j)
          ShufMask.push_back(j + i);

    WidenMap[Inst] = Builder.CreateShuffleVector(V0,
                                                 UndefValue::get(V0->getType()),
                                                 ShufMask);
    return;
  }
  // General case - whole mask should be recalculated
  llvm_unreachable("Unsupported shuffle");
}

void VPOCodeGen::vectorizeInsertElement(Instruction *Inst) {
  InsertElementInst *InsEltInst = cast<InsertElementInst>(Inst);
  Value *InsertTo = getVectorValue(InsEltInst->getOperand(0));
  Value *NewSubVec = getVectorValue(InsEltInst->getOperand(1));
  Value *OrigIndexVal = InsEltInst->getOperand(2);

  // In case of an non-const index, we serialize the instruction.
  // We first get the actual index, for the vectorized data using
  // 'add' and then insert that scalar into the index

  if (!isa<ConstantInt>(OrigIndexVal)) {
    Value *WideInsert = InsertTo;
    Value *IndexValVec = getVectorValue(OrigIndexVal);
    for (unsigned VIdx = 0; VIdx < VF; ++VIdx) {
      Value *IndexVal = Builder.CreateExtractElement(IndexValVec, VIdx);
      Value *VectorIdx = Builder.CreateAdd(
          ConstantInt::get(IndexVal->getType(), VIdx * VF), IndexVal);
      // The scalar value to be inserted maybe vectorized. Get its scalar value
      // for current lane.
      WideInsert = Builder.CreateInsertElement(
          WideInsert, getScalarValue(InsEltInst->getOperand(1), VIdx),
          VectorIdx);
    }
    WidenMap[Inst] = WideInsert;
    return;
  }

  unsigned Index = cast<ConstantInt>(OrigIndexVal)->getZExtValue();
  unsigned WideNumElts = InsertTo->getType()->getVectorNumElements();
  unsigned OriginalVL =
      InsEltInst->getOperand(0)->getType()->getVectorNumElements();

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
    WidenMap[Inst] = Shuf;
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
  SmallVector<unsigned, 8> ShufMask2;
  for (unsigned FirstVecIdx = 0, SecondVecIdx = WideNumElts;
       FirstVecIdx < WideNumElts; ++FirstVecIdx) {
    if ((FirstVecIdx % OriginalVL) == Index)
      ShufMask2.push_back(SecondVecIdx++);
    else
      ShufMask2.push_back(FirstVecIdx);
  }
  Value *SecondShuf = Builder.CreateShuffleVector(InsertTo, ExtendSubVec,
                                                  ShufMask2, "wide.insert");
  WidenMap[Inst] = SecondShuf;
}

void VPOCodeGen::serializePredicatedUniformLoad(Instruction *Inst) {
  assert(MaskValue->getType()->isVectorTy() &&
         MaskValue->getType()->getVectorNumElements() == VF &&
         "Unexpected Mask Type");
  // Emit not of all-zero check for mask
  Type *MaskTy = MaskValue->getType();
  Type *IntTy =
      IntegerType::get(MaskTy->getContext(), MaskTy->getPrimitiveSizeInBits());
  auto *MaskBitCast = Builder.CreateBitCast(MaskValue, IntTy);

  // Check if the bitcast value is not zero. The generated compare will be true
  // if atleast one of the i1 masks in <VF x i1> is true.
  auto *CmpInst =
      Builder.CreateICmpNE(MaskBitCast, Constant::getNullValue(IntTy));

  // Now create a clone of the load, populating correct values for its operands.
  Instruction *Cloned = Inst->clone();
  if (!Inst->getType()->isVoidTy())
    Cloned->setName(Inst->getName() + ".cloned");

  // Replace the operands of the cloned instructions with their scalar
  // equivalents in the new loop.
  for (unsigned Op = 0, e = Inst->getNumOperands(); Op != e; ++Op) {
    auto *NewOp = getScalarValue(Inst->getOperand(Op), 0 /*Lane*/);
    Cloned->setOperand(Op, NewOp);
  }

  // Place the cloned scalar load in the new loop.
  Builder.InsertWithDbgLoc(Cloned);
  ScalarMap[Inst][0] = Cloned;

  PredicatedInstructions.push_back(std::make_pair(Cloned, CmpInst));
}

void VPOCodeGen::serializeWithPredication(Instruction *Inst) {
  if (!MaskValue)
    return serializeInstruction(Inst);

  assert(MaskValue->getType()->isVectorTy() &&
         MaskValue->getType()->getVectorNumElements() == VF &&
         "Unexpected Mask Type");
  for (unsigned Lane = 0; Lane < VF; ++Lane) {
    Value *Cmp = Builder.CreateExtractElement(MaskValue, Lane, "Predicate");
    Cmp = Builder.CreateICmp(ICmpInst::ICMP_EQ, Cmp,
                             ConstantInt::get(Cmp->getType(), 1));
    Instruction *Cloned = Inst->clone();
    if (!Inst->getType()->isVoidTy())
      Cloned->setName(Inst->getName() + ".cloned");

    // Replace the operands of the cloned instructions with their scalar
    // equivalents in the new loop.
    for (unsigned Op = 0, e = Inst->getNumOperands(); Op != e; ++Op) {
      auto *NewOp = getScalarValue(Inst->getOperand(Op), Lane);
      Cloned->setOperand(Op, NewOp);
    }

    // Place the cloned scalar in the new loop.
    Builder.InsertWithDbgLoc(Cloned);
    ScalarMap[Inst][Lane] = Cloned;

    PredicatedInstructions.push_back(std::make_pair(Cloned, Cmp));
  }
}

void VPOCodeGen::serializeInstruction(Instruction *Instr, bool HasLoopPrivateOperand) {
  if (EnableVPValueCodegen) {
    LLVM_DEBUG(dbgs() << "LV: Value: "; Instr->dump(); dbgs() << "LV: Parent: ";
               Instr->getParent()->dump());
    llvm_unreachable("Implementing VPValue codegen uplift.");
  }

  assert(!Instr->getType()->isAggregateType() && "Can't handle vectors");

  unsigned Lanes = !HasLoopPrivateOperand && !Instr->mayHaveSideEffects() &&
                           isUniformAfterVectorization(Instr, VF)
                       ? 1
                       : VF;
  // Does this instruction return a value ?
  bool IsVoidRetTy = Instr->getType()->isVoidTy();

  // For each scalar that we create:
  for (unsigned Lane = 0; Lane < Lanes; ++Lane) {

    Instruction *Cloned = Instr->clone();
    if (!IsVoidRetTy)
      Cloned->setName(Instr->getName() + ".cloned");

    // Replace the operands of the cloned instructions with their scalar
    // equivalents in the new loop.
    for (unsigned Op = 0, e = Instr->getNumOperands(); Op != e; ++Op) {
      auto *NewOp = getScalarValue(Instr->getOperand(Op), Lane);
      Cloned->setOperand(Op, NewOp);
    }
    // Place the cloned scalar in the new loop.
    Builder.InsertWithDbgLoc(Cloned);
    ScalarMap[Instr][Lane] = Cloned;
  }
}

Value *VPOCodeGen::getStrideVector(Value *Val, Value *Stride) {
  assert(Val->getType()->isVectorTy() && "Must be a vector");
  assert(Val->getType()->getScalarType()->isIntegerTy() &&
         "Elem must be an integer");
  assert(Stride->getType() == Val->getType()->getScalarType() &&
         "Stride has wrong type");

  // Create the types.
  Type *ITy = Val->getType()->getScalarType();
  SmallVector<Constant *, 8> Indices;

  // Create a vector of consecutive numbers from zero to VF.
  for (unsigned i = 0; i < VF; ++i) {
    Indices.push_back(ConstantInt::get(ITy, i));
  }

  // Add the consecutive indices to the vector value.
  Constant *Cv = ConstantVector::get(Indices);
  assert(Cv->getType() == Val->getType() && "Invalid consecutive vec");
  Stride = Builder.CreateVectorSplat(VF, Stride);
  assert(Stride->getType() == Val->getType() && "Invalid stride type");

  // TBD: The newly created binary instructions should contain nsw/nuw flags,
  // which can be found from the original scalar operations.
  Stride = Builder.CreateMul(Cv, Stride);
  return Builder.CreateAdd(Val, Stride, "induction");
}

Value *VPOCodeGen::getStepVector(Value *Val, int StartIdx, Value *Step,
                                          Instruction::BinaryOps BinOp) {
  // Create and check the types.
  assert(Val->getType()->isVectorTy() && "Must be a vector");
  int VLen = Val->getType()->getVectorNumElements();

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
    // FIXME: The newly created binary instructions should contain nsw/nuw flags,
    // which can be found from the original scalar operations.
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

/// A helper function that adds a 'fast' flag to floating-point operations.
static Value *addFastMathFlag(Value *V) {
  if (isa<FPMathOperator>(V)) {
    FastMathFlags Flags;
    Flags.setFast();
    cast<Instruction>(V)->setFastMathFlags(Flags);
  }
  return V;
}

/// A helper function that returns an integer or floating-point constant with
/// value C.
static Constant *getSignedIntOrFpConstant(Type *Ty, int64_t C) {
  return Ty->isIntegerTy() ? ConstantInt::getSigned(Ty, C)
    : ConstantFP::get(Ty, C);
}

void VPOCodeGen::createVectorIntOrFpInductionPHI(const InductionDescriptor &ID,
                                                 Value *Step,
                                                 Instruction *&VectorInd) {
  Value *Start = ID.getStartValue();

  // Construct the initial value of the vector IV in the vector loop preheader
  auto CurrIP = Builder.saveIP();
  Builder.SetInsertPoint(LoopVectorPreHeader->getTerminator());
  Value *SplatStart = Builder.CreateVectorSplat(VF, Start);
  Value *SteppedStart =
    getStepVector(SplatStart, 0, Step, ID.getInductionOpcode());

  // We create vector phi nodes for both integer and floating-point induction
  // variables. Here, we determine the kind of arithmetic we will perform.
  Instruction::BinaryOps AddOp;
  Instruction::BinaryOps MulOp;
  if (Step->getType()->isIntegerTy()) {
    AddOp = Instruction::Add;
    MulOp = Instruction::Mul;
  } else {
    AddOp = ID.getInductionOpcode();
    MulOp = Instruction::FMul;
  }

  // Multiply the vectorization factor by the step using integer or
  // floating-point arithmetic as appropriate.
  Value *ConstVF = getSignedIntOrFpConstant(Step->getType(), VF);
  Value *Mul = addFastMathFlag(Builder.CreateBinOp(MulOp, Step, ConstVF));

  // Create a vector splat to use in the induction update.
  //
  // FIXME: If the step is non-constant, we create the vector splat with
  //        IRBuilder. IRBuilder can constant-fold the multiply, but it doesn't
  //        handle a constant vector splat.
  Value *SplatVF = isa<Constant>(Mul)
    ? ConstantVector::getSplat(VF, cast<Constant>(Mul))
    : Builder.CreateVectorSplat(VF, Mul);
  Builder.restoreIP(CurrIP);

  // We may need to add the step a number of times, depending on the unroll
  // factor. The last of those goes into the PHI.
  VectorInd = PHINode::Create(SteppedStart->getType(), 2, "vec.ind",
                              &*LoopVectorBody->getFirstInsertionPt());

  Instruction *LastInduction = cast<Instruction>(addFastMathFlag(
    Builder.CreateBinOp(AddOp, VectorInd, SplatVF, "step.add")));

  // Move the last step to the end of the latch block. This ensures consistent
  // placement of all induction updates.
  auto *VecLp = LI->getLoopFor(LoopVectorBody);
  assert(VecLp && "Unexpected null vector loop");
  auto *LoopVectorLatch = VecLp->getLoopLatch();
  assert(LoopVectorLatch && "Unexpected null vector loop latch");
  auto *Br = cast<BranchInst>(LoopVectorLatch->getTerminator());
  LastInduction->moveBefore(Br);
  LastInduction->setName("vec.ind.next");

  cast<PHINode>(VectorInd)->addIncoming(SteppedStart, LoopVectorPreHeader);
  cast<PHINode>(VectorInd)->addIncoming(LastInduction, LoopVectorLatch);
}

Value *VPOCodeGen::getIVStep(PHINode *IV, const InductionDescriptor &ID) {
  Value *Step = nullptr;
  auto &DL = OrigLoop->getHeader()->getModule()->getDataLayout();

  if (PSE.getSE()->isSCEVable(IV->getType())) {
    SCEVExpander Exp(*PSE.getSE(), DL, "induction");
    Step = Exp.expandCodeFor(ID.getStep(), ID.getStep()->getType(),
                             LoopVectorPreHeader->getTerminator());
  } else
    Step = cast<SCEVUnknown>(ID.getStep())->getValue();

  return Step;
}

Value *VPOCodeGen::buildScalarIVForLane(PHINode *OrigIV, unsigned Lane) {
  auto II = Legal->getInductionVars()->find(OrigIV);
  auto ID = II->second;

  // This function should never get called for Lane 0. The ScalarIV value for
  // lane 0 is added during induction widening.
  assert(Lane > 0 && "Unexpected lane 0 in buildScalarIVForLane");

  // Get the scalar IV value for the vector loop by getting the value from
  // ScalarMap for lane 0
  assert(ScalarMap.count(OrigIV) &&
         ScalarMap[OrigIV].count(0) &&
         "Expected scalar value for lane 0 not found");
  Value *ScalarIV = ScalarMap[OrigIV][0];

  // Induction step
  Value *Step = getIVStep(OrigIV, ID);

  // Get the value type and ensure it and the step have the same integer type.
  Type *ScalarIVTy = ScalarIV->getType()->getScalarType();
  assert(ScalarIVTy == Step->getType() &&
         "Val and Step should have the same type");

  // We build scalar steps for both integer and floating-point induction
  // variables. Here, we determine the kind of arithmetic we will perform.
  Instruction::BinaryOps AddOp;
  Instruction::BinaryOps MulOp;
  if (ScalarIVTy->isIntegerTy()) {
    AddOp = Instruction::Add;
    MulOp = Instruction::Mul;
  } else {
    AddOp = ID.getInductionOpcode();
    MulOp = Instruction::FMul;
  }

  auto CurrIP = Builder.saveIP();
  auto ScalIVInst = cast<Instruction>(ScalarIV);

  Builder.SetInsertPoint(&*(ScalIVInst->getParent()->getFirstInsertionPt()));
  auto *StartIdx = getSignedIntOrFpConstant(ScalarIVTy, Lane);
  auto *Mul = addFastMathFlag(Builder.CreateBinOp(MulOp, StartIdx, Step));
  auto *Add = addFastMathFlag(Builder.CreateBinOp(AddOp, ScalarIV, Mul));
  ScalarMap[OrigIV][Lane] = Add;
  Builder.restoreIP(CurrIP);

  return Add;
}

void VPOCodeGen::widenIntOrFpInduction(PHINode *IV, VPPHINode *VPIV) {

  auto II = Legal->getInductionVars()->find(IV);
  assert(II != Legal->getInductionVars()->end() && "IV is not an induction");

  auto ID = II->second;
  assert(IV->getType() == ID.getStartValue()->getType() && "Types must match");
  auto &DL = OrigLoop->getHeader()->getModule()->getDataLayout();

  // The step of the induction.
  Value *Step = getIVStep(IV, ID);

  Instruction *VectorInd = nullptr;
  createVectorIntOrFpInductionPHI(ID, Step, VectorInd);
  WidenMap[cast<Value>(IV)] = VectorInd;
  VPWidenMap[VPIV] = VectorInd;

  Value *ScalarIV = Induction;
  if (IV != Legal->getInduction()) {
    ScalarIV = IV->getType()->isIntegerTy()
      ? Builder.CreateSExtOrTrunc(ScalarIV, IV->getType())
      : Builder.CreateCast(Instruction::SIToFP, Induction,
                           IV->getType());
    ScalarIV = emitTransformedIndex(Builder, ScalarIV, PSE.getSE(), DL, ID);
    assert(ScalarIV && "Unexpected null return from transform");
    ScalarIV->setName("offset.idx");
  }

  // Use ScalarIV as the scalar value for Lane 0 when needed
  ScalarMap[IV][0] = ScalarIV;
  VPScalarMap[VPIV][0] = ScalarIV;
}

void VPOCodeGen::fixNonInductionPhis() {
  // Fix up the PHIs in the PhisToFix map. We use the VPPHI operands to setup
  // the operands of the PHI.
  if (EnableVPValueCodegen) {
    fixNonInductionVPPhis();
    return;
  }
  else
    for (auto PhiToFix : PhisToFix) {
      auto *VPPhi = PhiToFix.first;
      auto *Phi = PhiToFix.second;
      auto *UnderlyingPhi = VPPhi->getInstruction();
      const unsigned NumPhiValues = VPPhi->getNumIncomingValues();
      bool IsUniform = isUniformAfterVectorization(UnderlyingPhi, VF);

      for (unsigned I = 0; I < NumPhiValues; ++I) {
        auto *VPValue = VPPhi->getIncomingValue(I);
        auto *VPBB = VPPhi->getIncomingBlock(I);
        Value *IncValue =
            IsUniform ? getScalarValue(VPValue, 0) : getVectorValue(VPValue);
        Phi->addIncoming(IncValue, State->CFG.VPBB2IRBB[VPBB]);
      }
    }
}

void VPOCodeGen::setEdgeMask(BasicBlock *From, BasicBlock *To, Value *Mask) {
  EdgeToMaskMap[std::make_pair(From, To)] = Mask;
}

Value *VPOCodeGen::getEdgeMask(BasicBlock *From, BasicBlock *To) {
  auto Edge = std::make_pair(From, To);
  if (EdgeToMaskMap.count(Edge))
    return EdgeToMaskMap[Edge];
  return nullptr;
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

void VPOCodeGen::widenNonInductionPhi(VPPHINode *VPPhi) {
  PHINode *UnderlyingPhi = cast_or_null<PHINode>(VPPhi->getInstruction());

  // If the PHI is not being blended into a select, go ahead and create a PHI
  // and return after adding it to the PHIsToFix map.
  if (VPPhi->getBlend() == false) {
    auto PhiTy = VPPhi->getType();
    PHINode *NewPhi;
    if (!EnableVPValueCodegen) {
      if (isUniformAfterVectorization(UnderlyingPhi, VF)) {
        NewPhi = Builder.CreatePHI(PhiTy, VPPhi->getNumOperands(), "uni.phi");
        ScalarMap[UnderlyingPhi][0] = NewPhi;
      } else {
        PhiTy = getWidenedType(PhiTy, VF);
        NewPhi = Builder.CreatePHI(PhiTy, VPPhi->getNumOperands(), "vec.phi");
        WidenMap[UnderlyingPhi] = NewPhi;
        VPWidenMap[VPPhi] = NewPhi;
      }
      // Set incoming values later, they may not be ready yet in case of
      // back-edges.
      PhisToFix[VPPhi] = NewPhi;
    } else {
      // TODO: move this code to IntelVPOCodeGen.cpp.
      if (needScalarCode(VPPhi)) {
        NewPhi = Builder.CreatePHI(PhiTy, VPPhi->getNumOperands(), "uni.phi");
        VPScalarMap[VPPhi][0] = NewPhi;
        ScalarPhisToFix[VPPhi] = NewPhi;
      }
      if (needVectorCode(VPPhi)) {
        PhiTy = getWidenedType(PhiTy, VF);
        NewPhi = Builder.CreatePHI(PhiTy, VPPhi->getNumOperands(), "vec.phi");
        VPWidenMap[VPPhi] = NewPhi;
        PhisToFix[VPPhi] = NewPhi;
      }
    }
    return;
  }

  unsigned NumIncomingValues = VPPhi->getNumIncomingValues();
  assert(NumIncomingValues > 0 && "Unexpected PHI with zero values");
  if (NumIncomingValues == 1) {
    // Blend phis should only be encountered in the linearized control flow.
    // However, currently some preceding transformations mark some single-value
    // phis as blends too (and codegen is probably relying on that as well).
    // Bail out right now because general processing of phis with multiple
    // incoming values relies on the control flow being linearized.
    Value *Val = getVectorValue(VPPhi->getOperand(0));
    WidenMap[UnderlyingPhi] = Val;
    VPWidenMap[VPPhi] = Val;
    return;
  }

  // Blend the PHIs using selects and incoming masks.
  VPPhi->sortIncomingBlocksForBlend();

  // Generate a sequence of selects.
  Value *BlendVal = nullptr;
  for (unsigned Idx = 0, End = VPPhi->getNumIncomingValues(); Idx < End;
       ++Idx) {
    VPBasicBlock *Block = VPPhi->getIncomingBlock(Idx);
    Value *IncomingVecVal = getVectorValue(VPPhi->getIncomingValue(Idx));
    if (!BlendVal) {
      BlendVal = IncomingVecVal;
      continue;
    }

    Value *Cond = getVectorValue(Block->getPredicate());
    if (VPPhi->getType()->isVectorTy()) {
      unsigned OriginalVL = VPPhi->getType()->getVectorNumElements();
      Cond = replicateVectorElts(Cond, OriginalVL, Builder);
    }
    BlendVal = Builder.CreateSelect(Cond, IncomingVecVal, BlendVal, "predphi");
  }

  WidenMap[UnderlyingPhi] = BlendVal;
  VPWidenMap[VPPhi] = BlendVal;
}

void VPOCodeGen::vectorizePHIInstruction(VPPHINode *VPPhi) {

  if (EnableVPValueCodegen)
    return vectorizeVPPHINode(VPPhi);

  PHINode *P = cast_or_null<PHINode>(VPPhi->getInstruction());

  // Handle recurrences.
  if (P && Legal->isReductionVariable(P))
    return vectorizeReductionPHI(VPPhi, P);

  if (!VPPhi->isUnderlyingIRValid() || !Legal->getInductionVars()->count(P))
    // We are assuming for now that any VPPHIs added without an underlying
    // PHI is not an induction.
    // The Phi node is not induction. It combines 2 basic blocks ruled out
    // by uniform branch.
    return widenNonInductionPhi(VPPhi);

  InductionDescriptor II = Legal->getInductionVars()->lookup(P);
  const DataLayout &DL = OrigLoop->getHeader()->getModule()->getDataLayout();

  switch (II.getKind()) {
  default:
    llvm_unreachable("Unknown induction");
  case InductionDescriptor::IK_IntInduction:
  case InductionDescriptor::IK_FpInduction:
    return widenIntOrFpInduction(P, VPPhi);
  case InductionDescriptor::IK_PtrInduction: {
    // Handle the pointer induction variable case.
    assert(P->getType()->isPointerTy() && "Unexpected type.");
    // This is the normalized GEP that starts counting at zero.
    Value *PtrInd = Induction;
    PtrInd = Builder.CreateSExtOrTrunc(PtrInd, II.getStep()->getType());
    // Determine the number of scalars we need to generate for each unroll
    // iteration. If the instruction is uniform, we only need to generate the
    // first lane. Otherwise, we generate all VF values.
    unsigned Lanes = VF;
    // These are the scalar results. Notice that we don't generate vector GEPs
    // because scalar GEPs result in better code.
    for (unsigned Lane = 0; Lane < Lanes; ++Lane) {
      Constant *Idx = ConstantInt::get(PtrInd->getType(), Lane);
      Value *GlobalIdx = Builder.CreateAdd(PtrInd, Idx);
      Value *SclrGep =
          emitTransformedIndex(Builder, GlobalIdx, PSE.getSE(), DL, II);
      SclrGep->setName("next.gep");
      ScalarMap[P][Lane] = SclrGep;
      VPScalarMap[VPPhi][Lane] = SclrGep;
    }
    return;
  }
  }
}

std::unique_ptr<VectorVariant>
VPOCodeGen::matchVectorVariantImpl(StringRef VecVariantStringValue, bool Masked) {
  assert(!VecVariantStringValue.empty() &&
         "VectorVariant string value shouldn't be empty!");

  LLVM_DEBUG(dbgs() << "Trying to find match for: " << VecVariantStringValue
                    << "\n");
  LLVM_DEBUG(dbgs() << "\nCall VF: " << VF << "\n");
  unsigned TargetMaxRegWidth = TTI->getRegisterBitWidth(true);
  LLVM_DEBUG(dbgs() << "Target Max Register Width: " << TargetMaxRegWidth
                    << "\n");

  VectorVariant::ISAClass TargetIsaClass;
  switch (TargetMaxRegWidth) {
    case 128:
      TargetIsaClass = VectorVariant::ISAClass::XMM;
      break;
    case 256:
      // Important Note: there is no way to inspect CPU or FeatureBitset from
      // the LLVM compiler middle end (i.e., lib/Analysis, lib/Transforms). This
      // can only be done from the front-end or from lib/Target. Thus, we select
      // avx2 by default for 256-bit vector register targets. Plus, I don't
      // think we currently have anything baked in to TTI to differentiate avx
      // vs. avx2. Namely, whether or not for 256-bit register targets there is
      // 256-bit integer support.
      TargetIsaClass = VectorVariant::ISAClass::YMM2;
      break;
    case 512:
      TargetIsaClass = VectorVariant::ISAClass::ZMM;
      break;
    default:
      llvm_unreachable("Invalid target vector register width");
  }
  LLVM_DEBUG(dbgs() << "Target ISA Class: "
                    << VectorVariant::ISAClassToString(TargetIsaClass)
                    << "\n\n");

  SmallVector<StringRef, 4> Variants;
  VecVariantStringValue.split(Variants, ",");
  VectorVariant::ISAClass SelectedIsaClass = VectorVariant::ISAClass::XMM;
  int VariantIdx = -1;
  for (unsigned i = 0; i < Variants.size(); i++) {
    VectorVariant Variant(Variants[i]);
    VectorVariant::ISAClass VariantIsaClass = Variant.getISA();
    LLVM_DEBUG(dbgs() << "Variant ISA Class: "
                      << VectorVariant::ISAClassToString(VariantIsaClass)
                      << "\n");
    unsigned IsaClassMaxRegWidth =
      VectorVariant::ISAClassMaxRegisterWidth(VariantIsaClass);
    LLVM_DEBUG(dbgs() << "Isa Class Max Vector Register Width: "
                      << IsaClassMaxRegWidth << "\n");
    (void) IsaClassMaxRegWidth;
    unsigned FuncVF = Variant.getVlen();
    LLVM_DEBUG(dbgs() << "Func VF: " << FuncVF << "\n\n");

    // Select the largest supported ISA Class for this target.
    if (FuncVF == VF && VariantIsaClass <= TargetIsaClass &&
      Variant.isMasked() == Masked && VariantIsaClass >= SelectedIsaClass) {
      LLVM_DEBUG(dbgs() << "Candidate Function: " << Variant.encode()
                        << "\n");
      SelectedIsaClass = VariantIsaClass;
      VariantIdx = i;
    }
  }

  if (VariantIdx >= 0)
    return std::make_unique<VectorVariant>(Variants[VariantIdx]);

  return nullptr;
}

std::unique_ptr<VectorVariant>
VPOCodeGen::matchVectorVariant(const CallInst *Call, bool Masked) {
  if (!Call->hasFnAttr("vector-variants"))
    return {};

  return matchVectorVariantImpl(
      Call->getFnAttr("vector-variants").getValueAsString(), Masked);
}

bool VPOCodeGen::isScalarArgument(StringRef FnName, unsigned Idx) {
  if (isOpenCLReadChannel(FnName) || isOpenCLWriteChannel(FnName)) {
    return (Idx == 0);
  }
  return false;
}

void VPOCodeGen::addUnitStepLinear(Value *LinVal, Value *NewVal, int Step) {
  Legal->addUnitStepLinear(LinVal, NewVal, Step);

  // Add NewVal as the new scalar value for lane 0
  ScalarMap[LinVal][0] = NewVal;
}

void VPOCodeGen::vectorizeOpenCLSinCos(CallInst *Call, bool isMasked) {
  // If we encounter a call to OpenCL sincos function, i.e., a call to
  // _Z6sincosfPf, the code in Intel_SVMLEitter.cpp, currently maps that call to
  // _Z14sincos_ret2ptrDv<VF>_fPS_S1_ variant. The following code correctly sets
  // up the input/output arguments for that function.  '_Z6sincosfPf' has the
  // form,
  //
  // %sinVal = call float @_Z6sincosfPf(float %input, float* %cosPtr)
  // %cosVal = load float, float* %cosPtr
  //
  // The following code replaces that function call with
  // %16 = call <8 x float> @_Z14sincos_ret2ptrDv8_fPS_S1_(<8 x float>
  //                                                       %wide.input,
  //                                                       <8 x float>*
  //                                                       %cosPtr.vec,
  //                                                       <8 x float>*
  //                                                       %sinPtr.vec)
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
  Value *Arg1 = getVectorValue(Call->getArgOperand(0));
  Value *CosPtr = Call->getArgOperand(1);
  // Get the base-pointer for the widened CosPtr, i.e., <8 x float>*.
  // 'getVectorValue' will itself return <8 x float*>. A call to
  // getVectorValue() makes sure that we have that in place.
  if (!LoopPrivateWidenMap.count(CosPtr))
    getVectorValue(CosPtr);

  Instruction *WideCosPtr = cast<Instruction>(LoopPrivateWidenMap[CosPtr]);
  Instruction *WideSinPtr = WideCosPtr->clone();
  WideSinPtr->insertAfter(WideCosPtr);
  WideSinPtr->setName("sinPtr.vec");
  VecArgs.push_back(Arg1);
  VecArgs.push_back(WideCosPtr);
  VecArgs.push_back(WideSinPtr);
  VecArgTys.push_back(Arg1->getType());
  VecArgTys.push_back(WideCosPtr->getType());
  VecArgTys.push_back(WideSinPtr->getType());
  Function *VectorF =
      getOrInsertVectorFunction(Call->getCalledFunction(), VF, VecArgTys, TLI,
                                Intrinsic::not_intrinsic, nullptr, isMasked);
  assert(VectorF && "Vector function not created.");
  CallInst *VecCall = Builder.CreateCall(VectorF, VecArgs);
  if (isa<FPMathOperator>(VecCall))
    VecCall->copyFastMathFlags(Call);

  // Make sure we don't lose attributes at the call site. E.g., IMF
  // attributes are taken from call sites in MapIntrinToIml to refine
  // SVML calls for precision.
  copyRequiredAttributes(Call, VecCall);

  // Set calling convention for SVML function calls
  Function *CalledFunc = Call->getCalledFunction();
  assert(CalledFunc && "Unexpected null call function.");
  if (isSVMLFunction(TLI, CalledFunc->getName(), VectorF->getName()))
    VecCall->setCallingConv(CallingConv::SVML);

  Loop *Lp = LI->getLoopFor(Call->getParent());
  analyzeCallArgMemoryReferences(Call, VecCall, TLI, PSE.getSE(), Lp);

  Value *WideSinLoad = Builder.CreateAlignedLoad(
      WideSinPtr, cast<AllocaInst>(WideCosPtr)->getAlignment(),
      "wide.sin.InitVal");
  WidenMap[Call] = WideSinLoad;
  WidenMap[CosPtr] = WideCosPtr;
}

Value* VPOCodeGen::vectorizeOpenCLWriteChannelSrc(CallInst *Call,
                                                  unsigned ArgNum) {
  // For the vector version of __write_pipe we need to get a vector for the
  // source of the write. Since the argument to the scalar version is a pointer
  // to the scalar alloca designated for the write source, we must trace back
  // to this alloca and find the store to it. Then, the vector store for this
  // instruction is found and used as the write source argument for the vector
  // call.

  Value *VecWriteSrc = nullptr;
  Value *WriteSrc = getOpenCLReadWriteChannelAlloc(Call);
  assert(WriteSrc && "Unexpected null write source in ReadWriteChannel call");
  unsigned NumStoresToWriteSrc = 0;

  // In scalar code, before the call to __write_pipe, there will be a store of
  // some value to the alloca pointed to by the 2nd argument of the __write_pipe
  // call. Once vectorized, the result of the call will be returned to a vector
  // register instead of written to memory through the argument.
  //
  // Examples:
  //
  // Case 1 (write through bitcast):
  //
  // entry:
  //   %write.src = alloca float, align 4
  //   <snip, snip ...>
  //   %4 = bitcast float* %write.src to i32*
  //   %5 = bitcast float* %write.src to i8*, !dbg !301
  //   %6 = addrspacecast i8* %5 to i8 addrspace(4)*
  //   br <snip, snip, ...>
  //
  // <snip, snip, ...>
  //
  // for.body:
  //   store i32 %18, i32* %4, align 4
  //   %19 = load %struct.__pipe_t <snip, snip, ...>
  //   %20 = call i32
  //         @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %19,
  //                                  i8 addrspace(4)* %6)
  //
  // Case 1a: sometimes only %5 bitcast is present and %4 is missing
  //
  // Case 2: store directly to %write.src
  //
  // In both cases, there should only be one store to the write src location
  // before the call to the __write_pipe function. The widened equivalent of the
  // value being stored is what becomes VecWriteSrc, and this becomes the 2nd
  // parameter to the vectorized __write_pipe call. Since the algorithm relies
  // heavily on finding a single store, make sure there are no others.
  // Alternatively, the widened value of %4 could be used, but we still rely on
  // finding the single store to know which value corresponding to %write.src
  // will be used for the vector call. So, either way, the algorithm is
  // dependent on finding that single store before the __write_pipe call.
  //
  SmallVector<Value*, 2> WriteSrcUsers;
  for (auto *U : WriteSrc->users()) {
    // Case 1: find store to %4 and get the widened value of %18.
    if (BitCastInst *BitcastWriteSrc = dyn_cast<BitCastInst>(U)) {
      for (auto *BU : BitcastWriteSrc->users()) {
        WriteSrcUsers.push_back(BU);
      }
    } else {
      WriteSrcUsers.push_back(U);
    }
  }

  for (auto *U : WriteSrcUsers) {
    if (StoreInst *StoreToWriteSrc = dyn_cast<StoreInst>(U)) {
      LLVM_DEBUG(dbgs() << "StoreToWriteSrc: " << *StoreToWriteSrc << "\n");
      Value *StoreVal = StoreToWriteSrc->getOperand(0);
      LLVM_DEBUG(dbgs() << "StoreVal: " << *StoreVal << "\n");
      VecWriteSrc = getVectorValue(StoreVal);
      LLVM_DEBUG(dbgs() << "VecWriteSrc: " << *VecWriteSrc << "\n");
      NumStoresToWriteSrc++;
    }
  }

  assert(VecWriteSrc && NumStoresToWriteSrc == 1 &&
         "Assumed single store to write src location");

  return VecWriteSrc;
}

void VPOCodeGen::vectorizeOpenCLReadChannelDest(CallInst *Call,
                                                CallInst *VecCall,
                                                Value *CallOp) {

  Value *ReadDst = getOpenCLReadWriteChannelAlloc(Call);
  LLVM_DEBUG(dbgs() << "ReadDst: " << *ReadDst << "\n");

  // Write the return value from the vector call to the widened private pointer
  // for the read destination.
  auto VecReadDst = getVectorPrivateBase(ReadDst);
  auto VecCallPtrType = PointerType::get(
      VecCall->getType(), VecReadDst->getType()->getPointerAddressSpace());
  VecReadDst =
      Builder.CreateBitCast(VecReadDst, VecCallPtrType, "read_dst_cast");
  Builder.CreateStore(VecCall, VecReadDst);
}

void VPOCodeGen::vectorizeCallArgs(CallInst *Call, VectorVariant *VecVariant,
                                   SmallVectorImpl<Value *> &VecArgs,
                                   SmallVectorImpl<Type *> &VecArgTys) {

  std::vector<VectorKind> Parms;
  if (VecVariant) {
    Parms = VecVariant->getParameters();
  }

  Function *F = Call->getCalledFunction();
  assert(F && "Function not found for call instruction");
  StringRef FnName = F->getName();

  auto ProcessCallArg = [&](unsigned OrigArgIdx) -> Value * {
    if (isOpenCLWriteChannelSrc(FnName, OrigArgIdx)) {
      Value *VecWriteSrc = vectorizeOpenCLWriteChannelSrc(Call, OrigArgIdx);
      assert(VecWriteSrc && "Vector value for channel write source not found!");
      return VecWriteSrc;
    }

    if ((!VecVariant || Parms[OrigArgIdx].isVector()) &&
        !isScalarArgument(FnName, OrigArgIdx)) {
      // This is a vector call arg, so vectorize it.
      Value *Arg = Call->getArgOperand(OrigArgIdx);

      // Generate the right mask for OpenCL vector 'select' intrinsic
      if (isOpenCLSelectMask(FnName, OrigArgIdx))
        return getOpenCLSelectVectorMask(Arg);

      return getVectorValue(Arg);
    }
    // Linear and uniform parameters for simd functions must be passed as
    // scalars according to the vector function abi. CodeGen currently
    // vectorizes all instructions, so the scalar arguments for the vector
    // function must be extracted from them. For both linear and uniform
    // args, extract from lane 0. Linear args can use the value at lane 0
    // because this will be the starting value for which the stride will be
    // added. The same method applies to built-in functions for args that
    // need to be treated as uniform.

    assert(!isOpenCLSelectMask(FnName, OrigArgIdx) &&
           "OpenCL select mask parameter is linear/uniform?");

    Value *Arg = Call->getArgOperand(OrigArgIdx);
    Value *ScalarArg = getScalarValue(Arg, 0);

    return ScalarArg;
  };

  for (unsigned OrigArgIdx = 0; OrigArgIdx < Call->getNumArgOperands(); OrigArgIdx++) {
    if (isOpenCLReadChannelDest(FnName, OrigArgIdx))
      continue;

    Value *VecArg = ProcessCallArg(OrigArgIdx);
    VecArgs.push_back(VecArg);
    VecArgTys.push_back(VecArg->getType());
  }

  // We're done, unless we have an additional mask parameter to process that
  // wasn't part of the original (scalar) call.
  if (!VecVariant || !VecVariant->isMasked())
    return;

  Value *MaskToUse = MaskValue ? MaskValue
                               : Constant::getAllOnesValue(VectorType::get(
                                     Type::getInt1Ty(F->getContext()), VF));

  // Add the mask parameter for masked simd functions.
  // Mask should already be vectorized as i1 type.
  VectorType *MaskTy = cast<VectorType>(MaskToUse->getType());
  assert(MaskTy->getVectorElementType()->isIntegerTy(1) &&
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
  Function *CalledFunc = Call->getCalledFunction();
  assert(CalledFunc && "Unexpected null called function");
  Type *CharacteristicType = calcCharacteristicType(*CalledFunc, *VecVariant);
  unsigned CharacteristicTypeSize =
      CharacteristicType->getPrimitiveSizeInBits();

  // Promote the i1 to an integer type that has the same size as the
  // characteristic type.
  Type *ScalarToType =
      IntegerType::get(MaskTy->getContext(), CharacteristicTypeSize);
  VectorType *VecToType = VectorType::get(ScalarToType, VF);
  Value *MaskExt = Builder.CreateSExt(MaskToUse, VecToType, "maskext");

  // Bitcast if the promoted type is not the same as the characteristic
  // type.
  if (ScalarToType != CharacteristicType) {
    Type *MaskCastTy = VectorType::get(CharacteristicType, VF);
    Value *MaskCast = Builder.CreateBitCast(MaskExt, MaskCastTy, "maskcast");
    VecArgs.push_back(MaskCast);
    VecArgTys.push_back(MaskCastTy);
  } else {
    VecArgs.push_back(MaskExt);
    VecArgTys.push_back(VecToType);
  }
}

void VPOCodeGen::initOpenCLScalarSelectSet(
    ArrayRef<const char *> ScalarSelects) {

  for (const char *SelectFuncName : ScalarSelects) {
    ScalarSelectSet.insert(SelectFuncName);
  }
}

bool VPOCodeGen::isOpenCLSelectMask(StringRef FnName, unsigned Idx) {
  return Idx == 2 && ScalarSelectSet.count(FnName);
}

// Return the right vector mask for a OpenCL vector select build-in.
//
// Definition of OpenCL select intrinsic:
//   gentype select ( gentype a, gentype b, igentype c)
//
//   For each component of a vector type, result[i] = if MSB of c[i] is set ?
//   b[i] : a[i] For scalar type, result = c ? b : a.
//
// Scalar select build-in uses integer mask (integer != 0 means true). However,
// vector select built-in uses the MSB of each vector element.
//
// Returned vector mask depends on ScalarMask as follows:
//   1) if ScalarMask == ZExt(i1), return widened SExt.
//   2) if ScalarMask == SExt(i1), return widened SExt.
//   3) Otherwise, return CmpInst != 0 + SExt.
//
Value *VPOCodeGen::getOpenCLSelectVectorMask(Value *ScalarMask) {

  Type *ScTy = ScalarMask->getType();
  Type *VecTy = VectorType::get(ScTy, VF);

  assert(!ScTy->isVectorTy() && ScTy->isIntegerTy() &&
         "Scalar integer type expected.");

  // Special cases for i1 type
  CastInst *CastI;
  if ((CastI = dyn_cast<CastInst>(ScalarMask)) &&
      CastI->getSrcTy()->isIntegerTy(1 /*i1*/)) {
    // SExt mask doesn't need to be fixed.
    if (isa<SExtInst>(CastI))
      return getVectorValue(ScalarMask);
    // ZExt is replaced by an SExt.
    else if (isa<ZExtInst>(ScalarMask)) {
      Value *Val = getVectorValue(CastI->getOperand(0));
      return Builder.CreateSExt(Val, VecTy);
    }
  }

  // General case. We generate a CmpInst != 0 + SExt
  // TODO: Look at Volcano vectorizer, file OCLBuiltinPreVectorizationPass.cpp.
  // It is doing something different, creating a fake buildin. I don't know if
  // that approach is applicable here at this point.
  Value *VectorMask = getVectorValue(ScalarMask);
  Constant *Zero = Constant::getNullValue(VecTy);
  Value *Cmp;

  // Only integer mask is supported.
  Cmp = Builder.CreateICmpNE(VectorMask, Zero);
  return Builder.CreateSExt(Cmp, VecTy);
}

void VPOCodeGen::vectorizeCallInstruction(CallInst *Call) {

  SmallVector<Value *, 2> VecArgs;
  SmallVector<Type *, 2> VecArgTys;
  Function *CalledFunc = Call->getCalledFunction();
  assert(CalledFunc && "Unexpected null called function");
  bool IsMasked = (MaskValue != nullptr) ? true : false;

  // Don't attempt vector function matching for SVML or built-in functions.
  std::unique_ptr<VectorVariant> MatchedVariant;

  // OpenCL SinCos, would have a 'nullptr' MatchedVariant
  if (isOpenCLSinCos(CalledFunc->getName())) {
    vectorizeOpenCLSinCos(Call, IsMasked);
    return;
  }

  if (!TLI->isFunctionVectorizable(CalledFunc->getName())
      && !isOpenCLReadChannel(CalledFunc->getName())
      && !isOpenCLWriteChannel(CalledFunc->getName())) {
    // TLI is not used to check for SIMD functions for two reasons:
    // 1) A more sophisticated interface is needed to determine the most
    //    appropriate match.
    // 2) A SIMD function is not a library function.
    MatchedVariant = matchVectorVariant(Call, IsMasked);
    if (!MatchedVariant && !IsMasked) {
      // If non-masked version isn't available, try running the masked version
      // with all-ones mask.
      MatchedVariant = matchVectorVariant(Call, true);
      IsMasked = true;
    }
    assert(MatchedVariant && "Unexpected null matched vector variant");
    LLVM_DEBUG(dbgs() << "Matched Variant: " << MatchedVariant->encode()
                      << "\n");
  }

  vectorizeCallArgs(Call, MatchedVariant.get(), VecArgs, VecArgTys);

  // Call is passed here to handle OpenCL read/write channel vectorization.
  Function *VectorF = getOrInsertVectorFunction(CalledFunc, VF, VecArgTys, TLI,
                                                Intrinsic::not_intrinsic,
                                                MatchedVariant.get(),
                                                IsMasked, Call);
  assert(VectorF && "Can't create vector function.");
  CallInst *VecCall = Builder.CreateCall(VectorF, VecArgs);

  // TODO: investigate why attempting to copy fast math flags for __read_pipe
  // fails. For now, just don't do the copy.
  if (isa<FPMathOperator>(VecCall)
      && !isOpenCLReadChannel(CalledFunc->getName()))
    VecCall->copyFastMathFlags(Call);

  // Make sure we don't lose attributes at the call site. E.g., IMF
  // attributes are taken from call sites in MapIntrinToIml to refine
  // SVML calls for precision.
  copyRequiredAttributes(Call, VecCall);

  // Set calling convention for SVML function calls
  if (isSVMLFunction(TLI, CalledFunc->getName(), VectorF->getName()))
    VecCall->setCallingConv(CallingConv::SVML);

  Loop *Lp = LI->getLoopFor(Call->getParent());
  analyzeCallArgMemoryReferences(Call, VecCall, TLI, PSE.getSE(), Lp);

  // No blending is required here for masked simd function calls as of now for
  // two reasons:
  //
  // 1) A select is already generated for call results that are live outside of
  //    the predicated region by using the predicated region's mask. See
  //    widenNonInductionPhi().
  //
  // 2) Currently, masked stores are always generated for call results stored
  //    to memory within a predicated region. See vectorizeStoreInstruction().

  if (isOpenCLReadChannel(CalledFunc->getName())) {
    vectorizeOpenCLReadChannelDest(Call, VecCall, Call->getArgOperand(1));
  }

  WidenMap[Call] = VecCall;
}

void VPOCodeGen::vectorizeInstruction(VPInstruction *VPInst) {
  if (auto *VPPhi = dyn_cast<VPPHINode>(VPInst)) {
    vectorizePHIInstruction(VPPhi);
    return;
  }

  if (EnableVPValueCodegen) {
    // Use uplifted VPValue-based codegen path.
    vectorizeVPInstruction(VPInst);
    return;
  }

  // Generate code by peeking at underlying IR, if valid.
  if (VPInst->isUnderlyingIRValid()) {
    auto *Inst = VPInst->getInstruction();
    assert(Inst &&
           "Underlying instruction cannot be null for valid VPInstruction.");

    // Temporary workaround to detect and handle uniform loads and unit-stride
    // loads/stores in codegen by transferring knowledge from DA to VPOLegal.
    // TODO: Is this too late to do a knowledge transfer. Other option is to
    // write a full HCFG traversal in collectLoopUniforms.
    if (VPlanTeachLegalFromDA) {
      VPlanDivergenceAnalysis *DA = Plan->getVPlanDA();
      // We do special handling for uniform loads, nothing is done in codegen
      // for uniform stores.
      if (isa<LoadInst>(Inst) && !DA->isDivergent(*VPInst)) {
        if (auto *PtrInst =
                dyn_cast<Instruction>(getLoadStorePointerOperand(Inst))) {
          Legal->UniformForAnyVF.insert(PtrInst);
          Uniforms[VF].insert(Inst);
        }
      }
      if (isa<GetElementPtrInst>(Inst)) {
        if (!DA->isDivergent(*VPInst)) {
          // A pointer identified by DA as uniform for outer-loop vectorization
          // was marked as unit-strided by legality based on inner-loop
          // vectorization. Fix legality by unsetting the stride.
          if (Legal->isConsecutivePtr(Inst)) {
            Legal->erasePtrStride(Inst);
          }
        }

        // Check for GEPs producing unit-stride pointers.
        VPVectorShape *VPPtrShape = DA->getVectorShape(VPInst);
        if (all_of(VPInst->users(),
                   [](const VPUser *U) -> bool {
                     if (auto *UserInst = dyn_cast<VPInstruction>(U))
                       if (!UserInst->isUnderlyingIRValid())
                         return false;

                     return true;
                   }) &&
            VPPtrShape->isUnitStridePtr()) {
          int StrideInBytes = VPPtrShape->getStrideVal();
          Legal->addPtrStride(Inst, StrideInBytes > 0 ? 1 : -1);
        }
      }
    }

    vectorizeInstruction(Inst);

    // Add the widened value to the VPValue widen map.
    if (WidenMap.count(Inst)) {
      VPWidenMap[VPInst] = WidenMap[Inst];
    }

    return;
  }

  switch (VPInst->getOpcode()) {
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
      //    i1 %real_mask = and i1 %loop_incoming_mask i1,  %inner_loop_specific_mask
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
    auto *V = getBroadcastInstrs(CmpInst);
    VPWidenMap[VPInst] = V;
    return;
  }
  case Instruction::And:
  case Instruction::Or: {
    Value *A = getVectorValue(VPInst->getOperand(0));
    Value *B = getVectorValue(VPInst->getOperand(1));
    Value *V = Builder.CreateBinOp(
        (Instruction::BinaryOps)(VPInst->getOpcode()), A, B);
    VPWidenMap[VPInst] = V;
    return;
  }
  case VPInstruction::Not: {
    Value *A = getVectorValue(VPInst->getOperand(0));
    Value *V = Builder.CreateNot(A);
    VPWidenMap[VPInst] = V;
    return;
  }
  case VPInstruction::Pred: {
    // Pred instruction just marks the block mask.
    Value *A = getVectorValue(VPInst->getOperand(0));
    setMaskValue(A);
    return;
  }
  case Instruction::Select: {
    Value *M = getVectorValue(VPInst->getOperand(0));
    if (VPInst->getType()->isVectorTy()) {
      unsigned OriginalVL = VPInst->getType()->getVectorNumElements();
      M = replicateVectorElts(M, OriginalVL, Builder);
    }
    Value *A = getVectorValue(VPInst->getOperand(1));
    Value *B = getVectorValue(VPInst->getOperand(2));
    Value *V = Builder.CreateSelect(M, A, B, "wide.select.");
    VPWidenMap[VPInst] = V;
    return;
  }
  case Instruction::ICmp: {
    Value *A = getVectorValue(VPInst->getOperand(0));
    Value *B = getVectorValue(VPInst->getOperand(1));
    auto *Cmp = cast<VPCmpInst>(VPInst);
    Value *V = Builder.CreateICmp(Cmp->getPredicate(), A, B);
    VPWidenMap[VPInst] = V;
    return;
  }
  case Instruction::Store: {
    Value *VecPtr = getVectorValue(VPInst->getOperand(1));
    Value *VecDataOp = getVectorValue(VPInst->getOperand(0));
    Type *PtrToElemTy = VecPtr->getType()->getVectorElementType();
    Type *ElemTy = PtrToElemTy->getPointerElementType();
    VectorType *DesiredDataTy = getWidenedType(ElemTy, VF);
    VecDataOp = Builder.CreateBitCast(VecDataOp, DesiredDataTy, "cast");

    // TODO: Without underlying store, we will choose align=1.
    unsigned Alignment = getOriginalLoadStoreAlignment(VPInst);
    Builder.CreateMaskedScatter(VecDataOp, VecPtr, Alignment, MaskValue);
    return;
  }
  case Instruction::Add:
  case Instruction::FAdd:
  case Instruction::Xor: {
    assert(VPInst->getUnderlyingValue() &&
           "Can't handle a newly generated add/xor VPInstruction.");

    // Widen binary operands.
    Value *A = getVectorValue(VPInst->getOperand(0));
    Value *B = getVectorValue(VPInst->getOperand(1));

    // Create wide instruction.
    auto BinOpCode = static_cast<Instruction::BinaryOps>(VPInst->getOpcode());
    Value *V = Builder.CreateBinOp(BinOpCode, A, B);

    // TODO: Can't set any IR flags since they are not stored in VPInstruction
    // (example FMF, wrapping flags).

    VPWidenMap[VPInst] = V;
    // Inserting into the WidenMap is dirty and illegal. This is a temporary
    // hack and should be retired when we transition completely to VPValue-based
    // CG approach.
    WidenMap[VPInst->getUnderlyingValue()] = V;
    return;
  }
  case Instruction::ZExt: {
    assert(VPInst->getUnderlyingValue() &&
           "Can't handle a newly generated zext VPInstruction.");

    // Widen source operands.
    Value *VecSrc = getVectorValue(VPInst->getOperand(0));

    // Create wide instruction.
    Value *WideZExt =
        Builder.CreateZExt(VecSrc, VectorType::get(VPInst->getType(), VF));

    VPWidenMap[VPInst] = WideZExt;
    // Inserting into the WidenMap is dirty and illegal. This is a temporary
    // hack and should be retired when we transition completely to VPValue-based
    // CG approach.
    WidenMap[VPInst->getUnderlyingValue()] = WideZExt;
    return;
  }
  case Instruction::Trunc: {
    assert(VPInst->getUnderlyingValue() &&
           "Can't handle a newly generated trunc VPInstruction.");

    // Widen source operands.
    Value *VecSrc = getVectorValue(VPInst->getOperand(0));

    // Create wide instruction.
    Value *WideTrunc =
        Builder.CreateTrunc(VecSrc, getWidenedType(VPInst->getType(), VF));

    VPWidenMap[VPInst] = WideTrunc;
    // Inserting into the WidenMap is dirty and illegal. This is a temporary
    // hack and should be retired when we transition completely to VPValue-based
    // CG approach.
    WidenMap[VPInst->getUnderlyingValue()] = WideTrunc;
    return;
  }
  case Instruction::FNeg: {
    assert(VPInst->getUnderlyingValue() &&
           "Can't handle a newly generated fneg VPInstruction.");

    // Widen the source operand.
    Value *VecSrc = getVectorValue(VPInst->getOperand(0));

    // Create wide instruction.
    auto UnOpCode = static_cast<Instruction::UnaryOps>(VPInst->getOpcode());
    Value *V = Builder.CreateUnOp(UnOpCode, VecSrc);

    VPWidenMap[VPInst] = V;
    // Inserting into the WidenMap is dirty and illegal. This is a temporary
    // hack and should be retired when we transition completely to VPValue-based
    // CG approach.
    WidenMap[VPInst->getUnderlyingValue()] = V;
    return;
  }
  default:
    llvm_unreachable("Unexpected VPInstruction");
  }
}

void VPOCodeGen::vectorizeInstruction(Instruction *Inst) {
  // Diego: Why are we blindly vectorizing any instruction?
  //if (isUniformAfterVectorization(Inst, VF)) {
  //  return;
  //}

  switch (Inst->getOpcode()) {
  case Instruction::GetElementPtr: {
    GetElementPtrInst *GEP = cast<GetElementPtrInst>(Inst);

    // For Consecutive Load/Store we will create a scalar-gep.
    if (all_of(Inst->users(),
               [&](User *U) -> bool {
                 return getLoadStorePointerOperand(U) == Inst;
               }) &&
        Legal->isConsecutivePtr(Inst)) {
      Value *NewGEPPtrOp = getScalarValue(GEP->getPointerOperand(), 0);
      SmallVector<Value *, 6> OpsV;
      for (unsigned I = 1; I < GEP->getNumOperands(); ++I)
        OpsV.push_back(getScalarValue(GEP->getOperand(I), 0));
      GetElementPtrInst *ScalarGEP = cast<GetElementPtrInst>(
          Builder.CreateGEP(NewGEPPtrOp, OpsV, "scalar.gep."));
      ScalarGEP->setIsInBounds(GEP->isInBounds());
      ScalarMap[GEP][0] = ScalarGEP;
      break;
    }
    if (!Legal->isLoopPrivateAggregate(getPointerOperand(GEP)) &&
        all_of(Inst->users(), [&](User *U) -> bool {
          return getLoadStorePointerOperand(U) == Inst &&
                 Legal->isUniformForTheLoop(U);
        })) {
      serializeInstruction(Inst);
      break;
    }

    Value *GepPtrOp = GEP->getPointerOperand();
    SmallVector<Value *, 3> OpsV;
    if (isSerializedPrivateArray(GepPtrOp)) {
      // Treat memory access to private array-types differently. Instead
      // of widening [NumElts x Ty] to [VF x [NumElts x Ty]], we serialize
      // the allocation as alloca [NumElts x Ty], ... alloca [NumElts x Ty]
      if (!ScalarMap.count(GepPtrOp))
        createSerialPrivateArrayBase(GepPtrOp);

      // Get appropriate values for the operands of the GEP.
      for (Value *Op : GEP->indices()) {
        if (Legal->isLoopInvariant(Op) || Legal->isLoopPrivate(Op))
          OpsV.push_back(Op);
        else
          OpsV.push_back(getVectorValue(Op));
      }

      for (unsigned I = 0; I < VF; ++I) {
        SmallVector<Value *, 2> AcOpsV;
        for (Value *Op : OpsV) {
          if (!Op->getType()->isVectorTy())
            AcOpsV.push_back(Op);
          else
            AcOpsV.push_back(
                Builder.CreateExtractElement(Op, Builder.getInt64(I)));
        }
        ScalarMap[GEP][I] =
            Builder.CreateGEP(ScalarMap[GepPtrOp][I], AcOpsV, "privBase.");
      }
      return;
    }

    for (Value *Op : GEP->operands())
    OpsV.push_back(getVectorValue(Op));

    Value *GepBasePtr = OpsV[0];
    OpsV.erase(OpsV.begin());
    GetElementPtrInst *VectorGEP = cast<GetElementPtrInst>(
        Builder.CreateGEP(GepBasePtr, OpsV, "mm_vectorGEP"));
    VectorGEP->setIsInBounds(GEP->isInBounds());
    WidenMap[cast<Value>(Inst)] = VectorGEP;
    break;
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
    CastInst *CI = cast<CastInst>(Inst);
    auto Opcode = CI->getOpcode();

    /// Vectorize casts.
    Type *ScalTy = CI->getType();
    Type *VecTy = VectorType::get(ScalTy, VF);
    Value *ScalOp = Inst->getOperand(0);
    Value *VecOp = getVectorValue(ScalOp);
    WidenMap[cast<Value>(Inst)] = Builder.CreateCast(Opcode, VecOp, VecTy);

    // If the cast is a SExt/ZExt of a unit step linear item, add the cast value to
    // UnitStepLinears - so that we can use it to infer information about unit stride
    // loads/stores. For the scalar cast value
    Value *NewScalar;
    int LinStep;

    if ((Opcode == Instruction::SExt || Opcode == Instruction::ZExt) &&
        Legal->isUnitStepLinear(ScalOp, &LinStep, &NewScalar)) {
      // NewScalar is the scalar linear iterm corresponding to ScalOp - apply cast
      auto ScalCast = Builder.CreateCast(Opcode, NewScalar, ScalTy);
      addUnitStepLinear(Inst, ScalCast, LinStep);
    }
    break;
  }

  case Instruction::BitCast:
    vectorizeCast<BitCastInst>(Inst);
    break;
  case Instruction::AddrSpaceCast:
    vectorizeCast<AddrSpaceCastInst>(Inst);
    break;
  case Instruction::FNeg: {
    if (isUniformAfterVectorization(Inst, VF)) {
      serializeInstruction(Inst);
      break;
    }
    // Widen operand
    UnaryOperator *UnOp = cast<UnaryOperator>(Inst);
    Value *Src = getVectorValue(Inst->getOperand(0));

    // Create wide instruction
    Value *V = Builder.CreateUnOp(UnOp->getOpcode(), Src);

    UnaryOperator *VecOp = cast<UnaryOperator>(V);
    VecOp->copyIRFlags(UnOp);

    WidenMap[Inst] = V;
    break;
  }

  case Instruction::Add:
  case Instruction::FAdd:
  case Instruction::Sub:
  case Instruction::FSub:
  case Instruction::Mul:
  case Instruction::FMul:
  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::FDiv:
  case Instruction::URem:
  case Instruction::SRem:
  case Instruction::FRem:
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor: {

    if (isUniformAfterVectorization(Inst, VF)) {
      serializeInstruction(Inst);
      break;
    }
    // Widen binary operands
    BinaryOperator *BinOp = dyn_cast<BinaryOperator>(Inst);
    Value *A = getVectorValue(Inst->getOperand(0));
    Value *B = getVectorValue(Inst->getOperand(1));

    // Create wide instruction
    Value *V = Builder.CreateBinOp(BinOp->getOpcode(), A, B);

    if (BinaryOperator *VecOp = dyn_cast<BinaryOperator>(V))
      VecOp->copyIRFlags(BinOp);

    WidenMap[cast<Value>(Inst)] = V;
    break;
  }

  case Instruction::Load: {
    vectorizeLoadInstruction(Inst, true);
    break;
  }

  case Instruction::Store: {
    vectorizeStoreInstruction(Inst, true);
    break;
  }

  case Instruction::PHI: {
    llvm_unreachable("Phi instruction should not reach here");
  }
  case Instruction::ExtractElement:
    vectorizeExtractElement(Inst);
    break;
  case Instruction::InsertElement:
    vectorizeInsertElement(Inst);
    break;
  case Instruction::ShuffleVector:
    vectorizeShuffle(Inst);
    break;
  case Instruction::ICmp: {
    auto *Cmp = cast<ICmpInst>(Inst);
    Value *A = getVectorValue(Cmp->getOperand(0));
    Value *B = getVectorValue(Cmp->getOperand(1));
    WidenMap[Inst] = Builder.CreateICmp(Cmp->getPredicate(), A, B);
    break;
  }

  case Instruction::FCmp: {
    auto *FCmp = cast<FCmpInst>(Inst);
    Value *A = getVectorValue(FCmp->getOperand(0));
    Value *B = getVectorValue(FCmp->getOperand(1));
    Value *NewFCmp = Builder.CreateFCmp(FCmp->getPredicate(), A, B);
    cast<FCmpInst>(NewFCmp)->copyFastMathFlags(FCmp);
    WidenMap[Inst] = NewFCmp;
    break;
  }

  case Instruction::Select: {
    vectorizeSelectInstruction(Inst);
    break;
  }

  case Instruction::Call: {
    // TODO: Masked vector function call support needs to be added.
    CallInst *Call = cast<CallInst>(Inst);
    Function *F = Call->getCalledFunction();

    if (!F) {
      serializeWithPredication(Call);
      break;
    }

    StringRef CalledFunc = F->getName();
    bool IsMasked = (MaskValue != nullptr) ? true : false;
    if (TLI->isFunctionVectorizable(CalledFunc, VF, IsMasked) ||
        ((matchVectorVariant(Call, IsMasked) ||
          (!IsMasked && matchVectorVariant(Call, true)))) ||
        (isOpenCLReadChannel(CalledFunc) || isOpenCLWriteChannel(CalledFunc))) {
      vectorizeCallInstruction(Call);
    } else {
      LLVM_DEBUG(dbgs() << "Function " << CalledFunc << " is serialized\n");
      serializeWithPredication(Call);
    }
    break;
  }

#if 0
  case Instruction::Trunc: {
    CastInst *CI = dyn_cast<CastInst>(Inst);
    /// Optimize the special case where the source is the induction
    /// variable. Notice that we can only optimize the 'trunc' case
    /// because: a. FP conversions lose precision, b. sext/zext may wrap,
    /// c. other casts depend on pointer size.
    if (CI->getOperand(0) == OldInduction) {
      Value *ScalarCast = Builder.CreateCast(CI->getOpcode(), Induction,
                                             CI->getType());
      Value *Broadcasted = getBroadcastInstrs(ScalarCast, Builder);
      Constant *Stride = ConstantInt::getSigned(CI->getType(), 1);
      Value *tv = getStrideVector(Broadcasted, 0, Stride, Builder);
      WidenMap[Value] = tv;
    }
    break;
  }
#endif

  default: {
    serializeInstruction(Inst);
    break;
  }
  }
}
Value *VPOCodeGen::getOrCreateVectorTripCount(Loop *L) {
  if (VectorTripCount)
    return VectorTripCount;

  assert(L && "Unexpected null loop for trip count create");
  Value *TC = getOrCreateTripCount(L);
  IRBuilder<> Builder(L->getLoopPreheader()->getTerminator());

  // Now we need to generate the expression for the part of the loop that the
  // vectorized body will execute. This is equal to N - (N % Step) if scalar
  // iterations are not required for correctness, or N - Step, otherwise. Step
  // is equal to the vectorization factor (number of SIMD elements) times the
  // unroll factor (number of SIMD instructions).
  Constant *Step = ConstantInt::get(TC->getType(), VF);
  Value *R = Builder.CreateURem(TC, Step, "n.mod.vf");

  VectorTripCount = Builder.CreateSub(TC, R, "n.vec");
  return VectorTripCount;
}

void VPOCodeGen::collectTriviallyDeadInstructions(
    Loop *OrigLoop, VPOVectorizationLegality *Legal,
    SmallPtrSetImpl<Instruction *> &DeadInstructions) {
  BasicBlock *Latch = OrigLoop->getLoopLatch();

  // We create new control-flow for the vectorized loop, so the original
  // condition will be dead after vectorization if it's only used by the
  // branch.
  auto *Cmp = dyn_cast<Instruction>(Latch->getTerminator()->getOperand(0));
  if (Cmp && Cmp->hasOneUse())
    DeadInstructions.insert(Cmp);

  // We create new "steps" for induction variable updates to which the original
  // induction variables map. An original update instruction will be dead if
  // all its users except the induction variable are dead.
  for (auto &Induction : *Legal->getInductionVars()) {
    PHINode *Ind = Induction.first;
    auto *IndUpdate = cast<Instruction>(Ind->getIncomingValueForBlock(Latch));
    if (all_of(IndUpdate->users(), [&](User *U) -> bool {
      return U == Ind || DeadInstructions.count(cast<Instruction>(U));
    }))
      DeadInstructions.insert(IndUpdate);
  }
}

uint64_t VPOCodeGen::getConstTripCount() const {
  if (auto *C = dyn_cast<ConstantInt>(TripCount))
    return C->getZExtValue();
  return 0;
}

Value *VPOCodeGen::getOrCreateTripCount(Loop *L) {
  if (TripCount)
    return TripCount;

  IRBuilder<> Builder(L->getLoopPreheader()->getTerminator());
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
    BackedgeTakenCount = PSE.getSE()->getTruncateOrNoop(BackedgeTakenCount, IdxTy);
  BackedgeTakenCount = PSE.getSE()->getNoopOrZeroExtend(BackedgeTakenCount, IdxTy);

  // Get the total trip count from the count by adding 1.
  const SCEV *ExitCount = PSE.getSE()->getAddExpr(
      BackedgeTakenCount, PSE.getSE()->getOne(BackedgeTakenCount->getType()));

  const DataLayout &DL = L->getHeader()->getModule()->getDataLayout();

  // Expand the trip count and place the new instructions in the preheader.
  // Notice that the pre-header does not change, only the loop body.
  SCEVExpander Exp(*PSE.getSE(), DL, "induction");

  // Count holds the overall loop count (N).
  TripCount = Exp.expandCodeFor(ExitCount, ExitCount->getType(),
                                L->getLoopPreheader()->getTerminator());

  if (TripCount->getType()->isPointerTy())
    TripCount =
        CastInst::CreatePointerCast(TripCount, IdxTy, "exitcount.ptrcnt.to.int",
                                    L->getLoopPreheader()->getTerminator());

  return TripCount;
}

void VPOCodeGen::collectLoopUniforms(unsigned VF) {

  // We should not collect Uniforms more than once per VF. Right now,
  // this function is called from collectUniformsAndScalars(), which
  // already does this check. Collecting Uniforms for VF=1 does not make any
  // sense.

  assert(VF >= 2 && !Uniforms.count(VF) &&
          "This function should not be visited twice for the same VF");

  // Visit the list of Uniforms. If we'll not find any uniform value, we'll
  // not analyze again.  Uniforms.count(VF) will return 1.
  Uniforms[VF].clear();

  // We now know that the loop is vectorizable!
  // Collect instructions inside the loop that will remain uniform after
  // vectorization.

  SetVector<Instruction *> Worklist;

  // Start from Uniforms that alerady collected for any VF.
  //for (Instruction *I : Legal->uniforms())
  Worklist.insert(Legal->UniformForAnyVF.begin(), Legal->UniformForAnyVF.end());

  // Holds consecutive and consecutive-like pointers. Consecutive-like pointers
  // are pointers that are treated like consecutive pointers during
  // vectorization. The pointer operands of interleaved accesses are an
  // example.
  SmallSetVector<Instruction *, 8> ConsecutiveLikePtrs;

  // Holds pointer operands of instructions that are possibly non-uniform.
  SmallPtrSet<Instruction *, 8> PossibleNonUniformPtrs;

  // Check if a value is loop invariant or has already been
  // accounted as uniform.
  auto isInWorklistOrOutOfScope = [&](Value *V) -> bool {
    return OrigLoop->isLoopInvariant(V) || Worklist.count(cast<Instruction>(V));
  };

  // Iterate over the instructions in the loop, and collect all
  // consecutive-like pointer operands in ConsecutiveLikePtrs. If it's possible
  // that a consecutive-like pointer operand will be scalarized, we collect it
  // in PossibleNonUniformPtrs instead. We use two sets here because a single
  // getelementptr instruction can be used by both vectorized and scalarized
  // memory instructions. For example, if a loop loads and stores from the same
  // location, but the store is conditional, the store will be scalarized, and
  // the getelementptr won't remain uniform.
  // We also collect call instructions that have a return value as they are
  // (seems) the only instructions that may not be uniform even with uniform
  // operands(or with no operands at all). Check whether a call instruction
  // may have a side effect to determine that. That actually is a bit
  // pessimistic approach since function determinism is only a subset of all
  // possible side effects.
  for (auto *BB : OrigLoop->blocks())
    for (auto &I : *BB) {

      if (I.getOpcode() == Instruction::Call &&
          (I.getType()->isVoidTy() || !I.mayHaveSideEffects()) &&
          all_of(I.operands(), isInWorklistOrOutOfScope)) {
        Worklist.insert(&I);
        LLVM_DEBUG(dbgs() << "LV: Found uniform instruction: " << I << "\n");
      }

      // If there's no pointer operand, there's nothing to do.
      auto *Ptr = dyn_cast_or_null<Instruction>(getLoadStorePointerOperand(&I));
      if (!Ptr)
        continue;

      // True if all users of Ptr are memory accesses that have Ptr as their
      // pointer operand.
      auto UsersAreMemAccesses = all_of(Ptr->users(), [&](User *U) -> bool {
        return getLoadStorePointerOperand(U) == Ptr;
      });

      // Ensure the memory instruction will not be scalarized or used by
      // gather/scatter, making its pointer operand non-uniform. If the pointer
      // operand is used by any instruction other than a memory access, we
      // conservatively assume the pointer operand may be non-uniform.
      if (!UsersAreMemAccesses || !Legal->isConsecutivePtr(Ptr))
        PossibleNonUniformPtrs.insert(Ptr);

      // If the memory instruction will be vectorized and its pointer operand
      // is consecutive-like, or interleaving - the pointer operand should
      // remain uniform.
      else
        ConsecutiveLikePtrs.insert(Ptr);
    }

  // Add to the Worklist all consecutive and consecutive-like pointers that
  // aren't also identified as possibly non-uniform.
  for (auto *V : ConsecutiveLikePtrs)
    if (!PossibleNonUniformPtrs.count(V)) {
      LLVM_DEBUG(dbgs() << "LV: Found uniform instruction: " << *V << "\n");
      Worklist.insert(V);
    }

  // Expand Worklist in topological order: whenever a new instruction
  // is added , its users should be either already inside Worklist, or
  // out of scope (loop invariant). It ensures a uniform instruction will only
  // be used by uniform instructions or out of scope instructions.
  unsigned idx = 0;
  while (idx != Worklist.size()) {
    Instruction *I = Worklist[idx++];

    for (auto OV : I->operand_values()) {
      if (auto *OI = dyn_cast<Instruction>(OV)) {
        if (all_of(OI->users(), isInWorklistOrOutOfScope)) {
          Worklist.insert(OI);
          LLVM_DEBUG(dbgs()
                     << "LV: Found uniform instruction: " << *OI << "\n");
        }
      }
    }
  }

  // Returns true if Ptr is the pointer operand of a memory access instruction
  // I, and I is known to not require scalarization.
  auto isVectorizedMemAccessUse = [&](Instruction *I, Value *Ptr) -> bool {
    return getLoadStorePointerOperand(I) == Ptr && Legal->isConsecutivePtr(Ptr);
  };

  // For an instruction to be added into Worklist above, all its users inside
  // the loop should also be in Worklist. However, this condition cannot be
  // true for phi nodes that form a cyclic dependence. We must process phi
  // nodes separately. An induction variable will remain uniform if all users
  // of the induction variable and induction variable update remain uniform.
  // The code below handles both pointer and non-pointer induction variables.
  for (auto &Induction : *Legal->getInductionVars()) {
    auto *Ind = Induction.first;
    BasicBlock *Latch = OrigLoop->getLoopLatch();
    auto *IndUpdate = cast<Instruction>(Ind->getIncomingValueForBlock(Latch));

    // Determine if all users of the induction variable are uniform after
    // vectorization.
    auto UniformInd = all_of(Ind->users(), [&](User *U) -> bool {
      auto *I = cast<Instruction>(U);
      return I == IndUpdate || !OrigLoop->contains(I) || Worklist.count(I) ||
        isVectorizedMemAccessUse(I, Ind);
    });
    if (!UniformInd)
      continue;

    // Determine if all users of the induction variable update instruction are
    // uniform after vectorization.
    auto UniformIndUpdate = all_of(IndUpdate->users(), [&](User *U) -> bool {
      auto *I = cast<Instruction>(U);
      return I == Ind || !OrigLoop->contains(I) || Worklist.count(I) ||
        isVectorizedMemAccessUse(I, IndUpdate);
    });
    if (!UniformIndUpdate)
      continue;

    // The induction variable and its update instruction will remain uniform.
    Worklist.insert(Ind);
    Worklist.insert(IndUpdate);
    LLVM_DEBUG(dbgs() << "LV: Found uniform instruction: " << *Ind << "\n");
    LLVM_DEBUG(dbgs() << "LV: Found uniform instruction: " << *IndUpdate
                      << "\n");
  }

  Uniforms[VF].insert(Worklist.begin(), Worklist.end());
}

void VPOCodeGen::collectUniformsAndScalars(unsigned VF) {
  collectLoopUniforms(VF);
}

/// Returns true if \p I is known to be uniform after vectorization.
bool VPOCodeGen::isUniformAfterVectorization(Instruction *I,
                                             unsigned VF) const {
  // TODO - we need to use uniformity information coming from DA. For now, we
  // assume non-uniform if we do not have underlying IR instruction.
  if (!I)
    return false;

  assert(Uniforms.count(VF) && "VF not yet analyzed for uniformity");
  auto UniformsPerVF = Uniforms.find(VF);
  return UniformsPerVF->second.count(I);
}

// Fix up external users of the induction variable. At this point, we are
// in LCSSA form, with all external PHIs that use the IV having one input value,
// coming from the remainder loop. We need those PHIs to also have a correct
// value for the IV when arriving directly from the middle block.
void VPOCodeGen::fixupIVUsers(PHINode *OrigPhi,
                              const InductionDescriptor &II,
                              Value *CountRoundDown, Value *EndValue,
                              BasicBlock *MiddleBlock) {
  // There are two kinds of external IV usages - those that use the value
  // computed in the last iteration (the PHI) and those that use the penultimate
  // value (the value that feeds into the phi from the loop latch).
  // We allow both, but they, obviously, have different values.

  assert(OrigLoop->getExitBlock() && "Expected a single exit block");

  DenseMap<Value *, Value *> MissingVals;

  // An external user of the last iteration's value should see the value that
  // the remainder loop uses to initialize its own IV.
  Value *PostInc = OrigPhi->getIncomingValueForBlock(OrigLoop->getLoopLatch());
  for (User *U : PostInc->users()) {
    Instruction *UI = cast<Instruction>(U);
    if (!OrigLoop->contains(UI)) {
      assert(isa<PHINode>(UI) && "Expected LCSSA form");
      MissingVals[UI] = EndValue;
    }
  }

  // An external user of the penultimate value need to see EndValue - Step.
  // The simplest way to get this is to recompute it from the constituent SCEVs,
  // that is Start + (Step * (CRD - 1)).
  for (User *U : OrigPhi->users()) {
    auto *UI = cast<Instruction>(U);
    if (!OrigLoop->contains(UI)) {
      const DataLayout &DL =
        OrigLoop->getHeader()->getModule()->getDataLayout();
      assert(isa<PHINode>(UI) && "Expected LCSSA form");

      IRBuilder<> B(MiddleBlock->getTerminator());
      Value *CountMinusOne = B.CreateSub(
        CountRoundDown, ConstantInt::get(CountRoundDown->getType(), 1));
      Value *CMO = B.CreateSExtOrTrunc(CountMinusOne, II.getStep()->getType(),
                                       "cast.cmo");
      Value *Escape = emitTransformedIndex(B, CMO, PSE.getSE(), DL, II);
      assert(Escape && "Unexpected null return from transform");
      Escape->setName("ind.escape");
      MissingVals[UI] = Escape;
    }
  }

  for (auto &I : MissingVals) {
    PHINode *PHI = cast<PHINode>(I.first);
    // One corner case we have to handle is two IVs "chasing" each-other,
    // that is %IV2 = phi [...], [ %IV1, %latch ]
    // In this case, if IV1 has an external use, we need to avoid adding both
    // "last value of IV1" and "penultimate value of IV2". So, verify that we
    // don't already have an incoming value for the middle block.
    if (PHI->getBasicBlockIndex(MiddleBlock) == -1)
      PHI->addIncoming(I.second, MiddleBlock);
  }
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
    //sinkScalarOperands(&*I);

    I->getParent()->setName(Twine("pred.") + I->getOpcodeName() + ".if");
    BB->setName(Twine("pred.") + I->getOpcodeName() + ".continue");

    // If the instruction is non-void create a Phi node at reconvergence point.
    if (!I->getType()->isVoidTy()) {
      Value *IncomingTrue = nullptr;
      Value *IncomingFalse = nullptr;

      if (I->hasOneUse() && isa<InsertElementInst>(*I->user_begin())) {
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

// Unconditional last private variable
void VPOCodeGen::writePrivateValAfterLoop(Value *OrigPrivate) {
  IRBuilder<> Builder(LoopMiddleBlock->getTerminator());
  Value *PtrToVec = LoopPrivateWidenMap[OrigPrivate];
  Type *Int64Ty = Type::getInt64Ty(LoopMiddleBlock->getContext());

  Value *LastUpdatedLane = ConstantInt::get(Int64Ty, VF - 1);
  Value *PtrToFirstElt = Builder.CreateBitCast(PtrToVec, OrigPrivate->getType());
  Value *PtrToLane = Builder.CreateGEP(PtrToFirstElt, LastUpdatedLane,
                                       "LastUpdatedLanePtr");
  Value *ValueToWriteIn = Builder.CreateLoad(PtrToLane, "LastVal");
  Builder.CreateStore(ValueToWriteIn, OrigPrivate);
}

void VPOCodeGen::completeInMemoryReductions() {
  for (auto V : (*Legal->getInMemoryReductionVars())) {
    AllocaInst *Ptr = V.first;
    RecurrenceDescriptor::RecurrenceKind Kind = V.second.first;
    RecurrenceDescriptor::MinMaxRecurrenceKind Mrk = V.second.second;
    Value *Res = buildInMemoryReductionTail(Ptr, Kind, Mrk);
    ReductionEofLoopVal[Ptr] = Res;
  }
}

Value *VPOCodeGen::buildInMemoryReductionTail(
    Value *OrigRedV,
    RecurrenceDescriptor::RecurrenceKind Kind,
    RecurrenceDescriptor::MinMaxRecurrenceKind Mrk) {

  IRBuilder<> Builder(&*LoopMiddleBlock->getFirstInsertionPt());
  Value *PtrToVec = LoopPrivateWidenMap[OrigRedV];
  Value *WideLoad = Builder.CreateLoad(PtrToVec, "Red.vec");
  Value *ScalarV = reduceVector(WideLoad, Kind, Mrk, Builder);
  Builder.CreateStore(ScalarV, OrigRedV);
  return ScalarV;
}


Value *VPOCodeGen::getLastLaneFromMask(Value *MaskPtr) {

  Value *MaskValue = Builder.CreateLoad(MaskPtr);
  assert(MaskValue->getType()->isIntegerTy() &&
         "Mask should be an integer value");
  // Count leading zeroes. Since we always write non-zero mask,
  // the number of leading zeroes should be smaller than VF.
  Module *M = LoopMiddleBlock->getParent()->getParent();
  Value *F = Intrinsic::getDeclaration(M, Intrinsic::ctlz, MaskValue->getType());
  Value *LeadingZeroes = Builder.CreateCall(F, { MaskValue, Builder.getTrue() },
                                            "ctlz");

  // Last written lane is most-significant '1' in the mask.
  return Builder.CreateSub(ConstantInt::get(MaskValue->getType(), VF - 1),
                           LeadingZeroes, "LaneToCopyFrom");
}

void VPOCodeGen::writeCondPrivateValAfterLoop(Value *OrigPrivate) {
  Builder.SetInsertPoint(LoopMiddleBlock->getTerminator());

  // Here we keep the vector value:
  Value *PtrToVec = LoopPrivateWidenMap[OrigPrivate];

  Value *LastLane = getLastLaneFromMask(LoopPrivateLastMask[OrigPrivate]);

  // Get the type of original private value.
  Type *OrigPrivateTy = OrigPrivate->getType()->getPointerElementType();

  // Load the last lane element.
  if (!OrigPrivateTy->isVectorTy()) {
    Type *ScalarTy = OrigPrivate->getType();
    Value *PtrToLastVal =
      Builder.CreateGEP(Builder.CreateBitCast(PtrToVec, ScalarTy), LastLane);
    Value *ValueToWiteIn = Builder.CreateLoad(PtrToLastVal, "LastVal");

    // Store the result in original location of the private variable.
    Builder.CreateStore(ValueToWiteIn, OrigPrivate);
    return;
  }
  // The private variable is a vector.
  Type *EltTy = OrigPrivateTy->getVectorElementType();
  Type *PtrToEltTy = PointerType::get(EltTy, 0);
  unsigned OriginalVL = OrigPrivateTy->getVectorNumElements();
  Value *PtrToFirstElt = Builder.CreateBitCast(PtrToVec, PtrToEltTy,
                                               "PtrToFirstEltInPrivateVec");
  Value *PtrToFirstOrigElt = Builder.CreateBitCast(OrigPrivate, PtrToEltTy,
                                                   "PtrToFirstEltInOrigPrivate");
  for (unsigned i = 0; i < OriginalVL; ++i) {
    Value *LaneToCopyFrom = (i == 0) ? LastLane :
      Builder.CreateAdd(LastLane, ConstantInt::get(LastLane->getType(), i*VF),
                        "LaneToCopyFrom");
    Value *PtrToLastVal =
      Builder.CreateGEP(PtrToFirstElt, LaneToCopyFrom, "PtrInsidePrivVec");
    Value *ValueToWiteIn = Builder.CreateLoad(PtrToLastVal, "LastVal");
    Type *IdxTy = Type::getInt32Ty(OrigPrivate->getContext());
    Value *PtrToOrigLoc = (i==0) ? PtrToFirstOrigElt :
      Builder.CreateGEP(PtrToFirstOrigElt, ConstantInt::get(IdxTy, i),
                        "PtrToNextEltInOrigPrivate");
    // Store the result in original location of the private variable.
    Builder.CreateStore(ValueToWiteIn, PtrToOrigLoc);
  }
}

void VPOCodeGen::fixupLoopPrivates() {
  for (auto It : LoopPrivateWidenMap) {
    Value *OrigV = It.first;
    if (Legal->isLastPrivate(OrigV))
      writePrivateValAfterLoop(OrigV);
    else if (Legal->isCondLastPrivate(OrigV))
      writeCondPrivateValAfterLoop(OrigV);
  }
}
