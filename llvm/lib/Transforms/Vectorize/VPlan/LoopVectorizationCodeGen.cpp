//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOAvrLLVMCodeGen.cpp -- LLVM IR Code generation from AVR
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "LoopVectorizationCodeGen.h"
#include <tuple>

using namespace llvm;

#define DEBUG_TYPE "vpo-ir-loop-vectorize"


/// \brief Return scalar result of horizontal vector binary operation.
/// Horizontal binary operation splits the vector recursively
/// into 2 parts until the VF becomes 2. Then we extract elements from the
/// vector and perform scalar operation.
static Value* buildReductionTail(Value *VectorVal,
                                 Instruction::BinaryOps BOpcode,
                                 IRBuilder<>& Builder) {

  // Take Vector Length from the WideRedInst type
  Type *InstTy = VectorVal->getType();

  unsigned VF = cast<VectorType>(InstTy)->getNumElements();
  if (VF == 2) {
    Value *Lo =
        Builder.CreateExtractElement(VectorVal, Builder.getInt32(0), "Lo");
    Value *Hi =
        Builder.CreateExtractElement(VectorVal, Builder.getInt32(1), "Hi");
    return Builder.CreateBinOp(BOpcode, Lo, Hi, "Reduced");
  }
  SmallVector<uint32_t, 16> LoMask, HiMask;
  for (unsigned i = 0; i < VF / 2; ++i)
    LoMask.push_back(i);
  for (unsigned i = VF / 2; i < VF; ++i)
    HiMask.push_back(i);

  Value *Lo = Builder.CreateShuffleVector(VectorVal, UndefValue::get(InstTy),
                                          LoMask, "Lo");
  Value *Hi = Builder.CreateShuffleVector(VectorVal, UndefValue::get(InstTy),
                                          HiMask, "Hi");
  Value *Result = Builder.CreateBinOp(BOpcode, Lo, Hi, "Reduced");
  return buildReductionTail(Result, BOpcode, Builder);
}

/// \brief Check that the instruction has outside loop users and is not an
/// identified reduction variable.
static bool hasOutsideLoopUser(const Loop *TheLoop, Instruction *Inst,
                               SmallPtrSetImpl<Value *> &AllowedExit) {
  // Reduction and Induction instructions are allowed to have exit users. All
  // other instructions must not have external users.
  if (!AllowedExit.count(Inst))
    // Check that all of the users of the loop are inside the BB.
    for (User *U : Inst->users()) {
      Instruction *UI = cast<Instruction>(U);
      // This user may be a reduction exit value.
      if (!TheLoop->contains(UI)) {
        DEBUG(dbgs() << "LV: Found an outside user for : " << *UI << '\n');
        return true;
      }
    }
  return false;
}

bool VPOVectorizationLegality::canVectorize() {
  BasicBlock *Header = TheLoop->getHeader();
  // For each block in the loop.
  for (BasicBlock *BB : TheLoop->blocks()) {
    // Scan the instructions in the block and look for hazards.
    for (Instruction &I : *BB) {
      if (auto *Phi = dyn_cast<PHINode>(&I)) {
        Type *PhiTy = Phi->getType();
        // Check that this PHI type is allowed.
        if (!PhiTy->isIntegerTy() && !PhiTy->isFloatingPointTy() &&
            !PhiTy->isPointerTy()) {
          return false;
        }

        // If this PHINode is not in the header block, then we know that we
        // can convert it to select during if-conversion. No need to check if
        // the PHIs in this block are induction or reduction variables.
        if (BB != Header) {
          // Check that this instruction has no outside users or is an
          // identified reduction value with an outside user.
          if (!hasOutsideLoopUser(TheLoop, Phi, AllowedExit))
            continue;
          DEBUG(dbgs() << "LV: PHI value could not be identified as" <<
                " an induction or reduction \n");
          return false;
        }

        // We only allow if-converted PHIs with exactly two incoming values.
        if (Phi->getNumIncomingValues() != 2) {
          DEBUG(dbgs() << "LV: Found an invalid PHI.\n");
          return false;
        }
        /*
        RecurrenceDescriptor RedDes;
        if (RecurrenceDescriptor::isReductionPHI(Phi, TheLoop, RedDes)) {
          AllowedExit.insert(RedDes.getLoopExitInstr());
          Reductions[Phi] = RedDes;
          continue;
        }*/

        InductionDescriptor ID;
        if (InductionDescriptor::isInductionPHI(Phi, TheLoop, PSE, ID)) {
          addInductionPhi(Phi, ID, AllowedExit);
          continue;
        }

        DEBUG(dbgs() << "LV: Found an unidentified PHI." << *Phi << "\n");
        return false;
      } // end of PHI handling
    }
  }
  return Induction != nullptr;
}

static Type *convertPointerToIntegerType(const DataLayout &DL, Type *Ty) {
  if (Ty->isPointerTy())
    return DL.getIntPtrType(Ty);

  // It is possible that char's or short's overflow when we ask for the loop's
  // trip count, work around this by changing the type size.
  if (Ty->getScalarSizeInBits() < 32)
    return Type::getInt32Ty(Ty->getContext());

  return Ty;
}

static Type *getWiderType(const DataLayout &DL, Type *Ty0, Type *Ty1) {
  Ty0 = convertPointerToIntegerType(DL, Ty0);
  Ty1 = convertPointerToIntegerType(DL, Ty1);
  if (Ty0->getScalarSizeInBits() > Ty1->getScalarSizeInBits())
    return Ty0;
  return Ty1;
}

void VPOVectorizationLegality::addInductionPhi(
    PHINode *Phi, const InductionDescriptor &ID,
    SmallPtrSetImpl<Value *> &AllowedExit) {

  Inductions[Phi] = ID;

  Type *PhiTy = Phi->getType();
  const DataLayout &DL = Phi->getModule()->getDataLayout();

  // Get the widest type.
  if (!PhiTy->isFloatingPointTy()) {
    if (!WidestIndTy)
      WidestIndTy = convertPointerToIntegerType(DL, PhiTy);
    else
      WidestIndTy = getWiderType(DL, PhiTy, WidestIndTy);
  }

  // Int inductions are special because we only allow one IV.
  if (ID.getKind() == InductionDescriptor::IK_IntInduction &&
      ID.getConstIntStepValue() && ID.getConstIntStepValue()->isOne() &&
      isa<Constant>(ID.getStartValue()) &&
      cast<Constant>(ID.getStartValue())->isNullValue()) {

    // Use the phi node with the widest type as induction. Use the last
    // one if there are multiple (no good reason for doing this other
    // than it is expedient). We've checked that it begins at zero and
    // steps by one, so this is a canonical induction variable.
    if (!Induction || PhiTy == WidestIndTy)
      Induction = Phi;
  }

  // Both the PHI node itself, and the "post-increment" value feeding
  // back into the PHI node may have external users.
  AllowedExit.insert(Phi);
  AllowedExit.insert(Phi->getIncomingValueForBlock(TheLoop->getLoopLatch()));

  DEBUG(dbgs() << "LV: Found an induction variable.\n");
  return;
}

static void addBlockToParentLoop(Loop *L, BasicBlock *BB, LoopInfo& LI) {
  if (auto *ParentLoop = L->getParentLoop())
    ParentLoop->addBasicBlockToLoop(BB, LI);
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
      // We know what the end value is.
      EndValue = CountRoundDown;

    } else {
      IRBuilder<> B(LoopBypassBlocks.back()->getTerminator());
      Type *StepType = II.getStep()->getType();
      Instruction::CastOps CastOp =
          CastInst::getCastOpcode(CountRoundDown, true, StepType, true);
      Value *CRD = B.CreateCast(CastOp, CountRoundDown, StepType, "cast.crd");
      const DataLayout &DL =
          OrigLoop->getHeader()->getModule()->getDataLayout();
      PredicatedScalarEvolution &PSE = Legal->getPSE();
      EndValue = II.transform(B, CRD, PSE.getSE(), DL);
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

void VPOCodeGen::emitVectorLoopEnteredCheck(Loop *L, BasicBlock *Bypass) {
  Value *TC = getOrCreateVectorTripCount(L);
  BasicBlock *BB = L->getLoopPreheader();
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
  LoopVectorPreHeader = NewBB;
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
  // Create the compare.
  Value *ICmp = Builder.CreateICmpEQ(Next, End);
  Builder.CreateCondBr(ICmp, L->getExitBlock(), Header);

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

  Loop *Lp = new Loop();
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
  
  Type *IdxTy = Legal->getInduction()->getType();
  Value *StartIdx = ConstantInt::get(IdxTy, 0);
  Constant *Step = ConstantInt::get(IdxTy, VF);

  // Create an induction variable in vector loop with a step equal to VF.
  Induction = createInductionVariable(Lp, StartIdx, CountRoundDown, Step);

  // Add a check in the middle block to see if we have completed
  // all of the iterations in the first vector loop.
  // If (N - N%VF) == N, then we *don't* need to run the remainder.
  emitEndOfVectorLoop(Count, CountRoundDown);

  // Resume from vector loop. If vector loop was executed, the remainder
  // is Count - CountRoundDown. Otherwise the remainder is Count.
  emitResume(CountRoundDown);

  // Get ready to start creating new instructions into the vectorized body.
  Builder.SetInsertPoint(&*LoopVectorBody->getFirstInsertionPt());

  // Inform SCEV analysis to forget original loop
  PSE.getSE()->forgetLoop(OrigLoop);

  // Save the state.
  LoopVectorPreHeader = Lp->getLoopPreheader();
}

void VPOCodeGen::updateAnalysis() {
  // Forget the original basic block.
  PSE.getSE()->forgetLoop(OrigLoop);

  // Update the dominator tree information.
  assert(DT->properlyDominates(LoopBypassBlocks.front(), LoopExitBlock) &&
         "Entry does not dominate exit.");

  // We don't predicate stores by this point, so the vector body should be a
  // single loop.
  DT->addNewBlock(LoopVectorBody, LoopVectorPreHeader);

  DT->addNewBlock(LoopMiddleBlock, LoopVectorBody);
  DT->addNewBlock(LoopScalarPreHeader, LoopBypassBlocks[0]);
  DT->changeImmediateDominator(LoopScalarBody, LoopScalarPreHeader);
  DT->changeImmediateDominator(LoopExitBlock, LoopBypassBlocks[0]);

  DEBUG(DT->verifyDomTree());
}

Value * VPOCodeGen::getVectorValue(Value *V) {
  assert(!V->getType()->isVectorTy() && "Can't widen a vector");

  // If we have this scalar in the map, return it.
  if (WidenMap.find(V) != WidenMap.end())
    return WidenMap[V];

  // If this scalar is unknown, assume that it is a constant or that it is
  // loop invariant. Broadcast V and save the value for future uses.
  Value *B = Builder.CreateVectorSplat(VF, V, "broadcast");
  WidenMap[V] = B;

  return WidenMap[V];
}

Value *VPOCodeGen::getScalarValue(Value *V) {
  // If the value is not an instruction contained in the loop, it should
  // already be scalar.
  if (OrigLoop->isLoopInvariant(V))
    return V;

  if (ScalarMap.count(V))
    return ScalarMap[V];

  Value *VecV = getVectorValue(V);
  return Builder.CreateExtractElement(VecV, Builder.getInt32(0));
}

Value *VPOCodeGen::reverseVector(Value *Vec) {
  assert(Vec->getType()->isVectorTy() && "Invalid type");
  SmallVector<Constant *, 8> ShuffleMask;
  for (unsigned i = 0; i < VF; ++i)
    ShuffleMask.push_back(Builder.getInt32(VF - i - 1));

  return Builder.CreateShuffleVector(Vec, UndefValue::get(Vec->getType()),
                                     ConstantVector::get(ShuffleMask),
                                     "reverse");
}

void VPOCodeGen::vectorizeLoopInvariantLoad(Instruction *Inst) {
  Instruction *Cloned = Inst->clone();
  Cloned->setName(Inst->getName() + ".cloned");
  Builder.Insert(Cloned);
  Value *Broadcast = Builder.CreateVectorSplat(VF, Cloned, "broadcast");
  WidenMap[Inst] = Broadcast;
}

void VPOCodeGen::vectorizeLoadInstruction(Instruction *Inst,
                                          bool EmitIntrinsic) {
  LoadInst *LI = cast<LoadInst>(Inst);
  Value *Ptr = LI->getPointerOperand();
  if (Legal->isLoopInvariant(Ptr)) {
    vectorizeLoopInvariantLoad(Inst);
    return;
  }

  int ConsecutiveStride = Legal->isConsecutivePtr(Ptr);
  bool Reverse = (ConsecutiveStride == -1);
  if (ConsecutiveStride == 0 && !EmitIntrinsic) {
    serializeInstruction(Inst);
    return;
  }

  Type *DataTy = VectorType::get(LI->getType(), VF);
  unsigned Alignment = LI->getAlignment();
  // An alignment of 0 means target abi alignment. We need to use the scalar's
  // target abi alignment in such a case.
  const DataLayout &DL = Inst->getModule()->getDataLayout();
  if (!Alignment)
    Alignment = DL.getABITypeAlignment(LI->getType());
  unsigned AddressSpace = Ptr->getType()->getPointerAddressSpace();

  // Handle consecutive loads.
  GetElementPtrInst *Gep = dyn_cast<GetElementPtrInst>(Ptr);
  if (ConsecutiveStride != 0) {
    if (Gep) {
      GetElementPtrInst *Gep2 = cast<GetElementPtrInst>(Gep->clone());
      Gep2->setName("gep.indvar");

      for (unsigned i = 0; i < Gep->getNumOperands(); ++i)
        Gep2->setOperand(i, getScalarValue(Gep->getOperand(i)));
      Ptr = Builder.Insert(Gep2);
    } else // No GEP
      Ptr = getScalarValue(Ptr);

    Ptr = Reverse ?
      Builder.CreateGEP(nullptr, Ptr, Builder.getInt32(1 - VF)) : Ptr;   

    Value *VecPtr =
      Builder.CreateBitCast(Ptr, DataTy->getPointerTo(AddressSpace));
    Value *NewLI = Builder.CreateAlignedLoad(VecPtr, Alignment, "wide.load");
    if (Reverse)
      NewLI = reverseVector(NewLI);
    WidenMap[cast<Value>(Inst)] = NewLI;
    return;
  }

  // GATHER
  Value *VectorPtr = getVectorValue(Ptr);
  Instruction *NewLI = Builder.CreateMaskedGather(VectorPtr, Alignment, nullptr,
                                                  0, "wide.masked.gather");

  WidenMap[cast<Value>(Inst)] = cast<Value>(NewLI);
}

void VPOCodeGen::vectorizeStoreInstruction(Instruction *Inst,
                                           bool EmitIntrinsic) {
  StoreInst *SI = cast<StoreInst>(Inst);
  Value *Ptr = SI->getPointerOperand();

  int ConsecutiveStride = Legal->isConsecutivePtr(Ptr);
  bool Reverse = (ConsecutiveStride == -1);
  if (ConsecutiveStride == 0 && !EmitIntrinsic) {
    serializeInstruction(Inst);
    return;
  }

  const DataLayout &DL = Inst->getModule()->getDataLayout();
  Type *ScalarDataTy = SI->getValueOperand()->getType();
  Type *DataTy = VectorType::get(ScalarDataTy, VF);

  unsigned Alignment = SI->getAlignment();
  if (!Alignment)
    Alignment = DL.getABITypeAlignment(ScalarDataTy);
  unsigned AddressSpace = Ptr->getType()->getPointerAddressSpace();
  Value *VecDataOp = getVectorValue(SI->getValueOperand());


  // Handle consecutive stores.
  GetElementPtrInst *Gep = dyn_cast<GetElementPtrInst>(Ptr);
  if (ConsecutiveStride) {
    if (Gep) {
      GetElementPtrInst *Gep2 = cast<GetElementPtrInst>(Gep->clone());
      Gep2->setName("gep.indvar");

      for (unsigned i = 0; i < Gep->getNumOperands(); ++i)
        Gep2->setOperand(i, getScalarValue(Gep->getOperand(i)));
      Ptr = Builder.Insert(Gep2);
    } else // No GEP
      Ptr = getScalarValue(Ptr);

    Ptr = Reverse ?
      Builder.CreateGEP(nullptr, Ptr, Builder.getInt32(1 - VF)) : Ptr;

    if (Reverse)
      // If we store to reverse consecutive memory locations, then we need
      // to reverse the order of elements in the stored value.
      VecDataOp = reverseVector(VecDataOp);

    Value *VecPtr =
      Builder.CreateBitCast(Ptr, DataTy->getPointerTo(AddressSpace));
    Builder.CreateAlignedStore(VecDataOp, VecPtr, Alignment, "wide.store");
    return;
  }

  // SCATTER
  Value *VectorPtr = getVectorValue(Ptr);
  Builder.CreateMaskedScatter(VecDataOp, VectorPtr, Alignment, nullptr);
}

void VPOCodeGen::serializeInstruction(Instruction *Inst) {
  assert(!Inst->getType()->isAggregateType() && "Can't handle vectors");

  // Holds vector parameters or scalars, in case of uniform vals.
  SmallVector<Value *, 4> Params;

  // Find all of the vectorized parameters.
  for (unsigned op = 0, e = Inst->getNumOperands(); op != e; ++op) {
    Value *SrcOp = Inst->getOperand(op);

    // Try using previously calculated values.
    Instruction *SrcInst = dyn_cast<Instruction>(SrcOp);

    // If the src is an instruction that appeared earlier in the loop
    // then it should already be vectorized.
    if (SrcInst && OrigLoop->contains(SrcInst)) {
      assert(WidenMap.find(SrcOp) != WidenMap.end() &&
             "Source operand is unavailable");

      // The parameter is a vector value from earlier.
      Params.push_back(WidenMap[SrcOp]);
    } else {
      // The parameter is a scalar from outside the loop. Maybe even a constant.
      Params.push_back(SrcOp);
    }
  }

  assert(Params.size() == Inst->getNumOperands() &&
         "Invalid number of operands");

  // Does this instruction return a value ?
  bool IsVoidRetTy = Inst->getType()->isVoidTy();

  Value *UndefVec = IsVoidRetTy
                        ? nullptr
                        : UndefValue::get(VectorType::get(Inst->getType(), VF));

  // Create a new entry in the WidenMap and initialize it to Undef or Null.
  Value *VecResults = UndefVec;

  for (unsigned ElemNum = 0; ElemNum < VF; ++ElemNum) {
    Instruction *Cloned = Inst->clone();

    if (!IsVoidRetTy)
      Cloned->setName(Inst->getName() + ".cloned");

    // Replace the operands of the cloned instructions with extracted scalars.
    for (unsigned op = 0, e = Inst->getNumOperands(); op != e; ++op) {
      Value *Op = Params[op];

      // Param is a vector. Need to extract the right lane.
      if (Op->getType()->isVectorTy()) {
        Op = Builder.CreateExtractElement(Op, Builder.getInt32(ElemNum));
      }

      Cloned->setOperand(op, Op);
    }

    // Place the cloned scalar in the new loop.
    Builder.Insert(Cloned);

    // If the original scalar returns a value we need to build up the return
    // vector value.
    if (!IsVoidRetTy) {
      VecResults = Builder.CreateInsertElement(VecResults, Cloned,
                                               Builder.getInt32(ElemNum));
    }
  }

  // If the original scalar returns a value we need to place the vector result
  // in WidenMap so that future users will be able to use it.
  if (!IsVoidRetTy) {
    WidenMap[cast<Value>(Inst)] = VecResults;
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

Value *VPOCodeGen::getStepVector(Value *Val, int StartIdx, Value *Step) {
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
    // FIXME: The newly created binary instructions should contain nsw/nuw
    // flags, which can be found from the original scalar operations.
    Step = Builder.CreateMul(Cv, Step);
    return Builder.CreateAdd(Val, Step, "induction");
  }
  llvm_unreachable("Non integer step is unsupported yet");
  return nullptr;
}

void VPOCodeGen::createVectorIntInductionPHI(PHINode *IV,
                                             Instruction *&VectorInd) {

  auto II = Legal->getInductionVars()->find(IV);
  auto ID = II->second;
  Value *Start = ID.getStartValue();
  ConstantInt *Step = ID.getConstIntStepValue();
  assert(Step && "Can not widen an IV with a non-constant step");

  // Construct the initial value of the vector IV in the vector loop preheader
  auto CurrIP = Builder.saveIP();
  Builder.SetInsertPoint(LoopVectorPreHeader->getTerminator());

  Value *SplatStart = Builder.CreateVectorSplat(VF, Start);
  Value *SteppedStart = getStepVector(SplatStart, 0, Step);
  Builder.restoreIP(CurrIP);

  Value *SplatVF = ConstantVector::getSplat(
      VF, ConstantInt::getSigned(Start->getType(), VF * Step->getSExtValue()));

  // We may need to add the step a number of times, depending on the unroll
  // factor. The last of those goes into the PHI.
  VectorInd = PHINode::Create(SteppedStart->getType(), 2, "vec.ind",
                              &*LoopVectorBody->getFirstInsertionPt());
  Instruction *LastInduction =
      cast<Instruction>(Builder.CreateAdd(VectorInd, SplatVF, "step.add"));

  // Move the last step to the end of the latch block. This ensures consistent
  // placement of all induction updates.
  auto *LoopVectorLatch = LI->getLoopFor(LoopVectorBody)->getLoopLatch();
  auto *Br = cast<BranchInst>(LoopVectorLatch->getTerminator());
  auto *ICmp = cast<Instruction>(Br->getCondition());
  LastInduction->moveBefore(ICmp);
  LastInduction->setName("vec.ind.next");

  cast<PHINode>(VectorInd)->addIncoming(SteppedStart, LoopVectorPreHeader);
  cast<PHINode>(VectorInd)->addIncoming(LastInduction, LoopVectorLatch);
}

static Value *getFpStepVector(IRBuilder<>& Builder, int StartIdx,
                              Value *Step, unsigned VF) {
  SmallVector<Constant *, 8> Indices;

  // Create a vector of consecutive numbers from zero to VF.
  for (unsigned i = 0; i < VF; ++i)
    Indices.push_back(ConstantFP::get(Step->getType(), (double)(StartIdx + i)));

  // Add the consecutive indices to the vector value.
  Constant *Cv = ConstantVector::get(Indices);

  Step = Builder.CreateVectorSplat(VF, Step);

  // Floating point operations had to be 'fast' to enable the induction.
  FastMathFlags Flags;
  Flags.setUnsafeAlgebra();

  Value *MulOp = Builder.CreateFMul(Cv, Step);
  if (isa<Instruction>(MulOp))
    // Have to check, MulOp may be a constant
    cast<Instruction>(MulOp)->setFastMathFlags(Flags);

  return MulOp;
}

void VPOCodeGen::widenFpInduction(PHINode *IV) {

  auto II = Legal->getInductionVars()->find(IV);
  auto ID = II->second;
  assert(IV->getType() == ID.getStartValue()->getType() &&
         "Types must match");
  // Handle other induction variables that are now based on the
  // canonical one.
  assert(IV != Legal->getInduction() && "Primary induction can be integer only");
  auto &DL = OrigLoop->getHeader()->getModule()->getDataLayout();

  Value *FpInd = Builder.CreateCast(Instruction::SIToFP, Induction, IV->getType());
  FpInd = ID.transform(Builder, FpInd, PSE.getSE(), DL);
  FpInd->setName("fp.offset.idx");
  // Now we have scalar op: %fp.offset.idx = StartVal +/- Induction*StepVal

  Value *VecFpInd = Builder.CreateVectorSplat(VF, FpInd);
  Value *StepVal = cast<SCEVUnknown>(ID.getStep())->getValue();

  IRBuilder<> VecPhBuilder(&*LoopVectorPreHeader->getFirstInsertionPt());
  Value *VecStepVal = getFpStepVector(VecPhBuilder, 0, StepVal, VF);
  Value *BOp = Builder.CreateBinOp(ID.getInductionOpcode(), VecFpInd,
                                   VecStepVal, "induction");

  // Floating point operations had to be 'fast' to enable the induction.
  FastMathFlags Flags;
  Flags.setUnsafeAlgebra();
  if (isa<Instruction>(BOp))
    cast<Instruction>(BOp)->setFastMathFlags(Flags);

  WidenMap[IV] = BOp;
}

void VPOCodeGen::widenIntInduction(PHINode *IV) {

  auto II = Legal->getInductionVars()->find(IV);
  assert(II != Legal->getInductionVars()->end() && "IV is not an induction");

  auto ID = II->second;
  assert(IV->getType() == ID.getStartValue()->getType() && "Types must match");

  // The step of the induction.
  Value *Step = nullptr;

  if (ID.getConstIntStepValue())
    Step = ID.getConstIntStepValue();

  assert(Step && "Non-constant step is not handled yet");

  if (IV->getType() == Induction->getType() && Step) {
    Instruction *VectorInd = nullptr;
    createVectorIntInductionPHI(IV, VectorInd);
    WidenMap[cast<Value>(IV)] = VectorInd;
  }

  Value *ScalarIV = Induction;
  auto &DL = OrigLoop->getHeader()->getModule()->getDataLayout();
  if (IV != Legal->getInduction()) {
    ScalarIV = Builder.CreateSExtOrTrunc(ScalarIV, IV->getType());
    ScalarIV = ID.transform(Builder, ScalarIV, PSE.getSE(), DL);
    ScalarIV->setName("offset.idx");
  }

  ScalarMap[cast<Value>(IV)] = ScalarIV;
}

void VPOCodeGen::vectorizePHIInstruction(Instruction *Inst) {

  PHINode *P = cast<PHINode>(Inst);
  // This PHINode must be an induction variable.
  // Make sure that we know about it.
  assert(Legal->getInductionVars()->count(P) && "Not an induction variable");

  InductionDescriptor II = Legal->getInductionVars()->lookup(P);

  switch (II.getKind()) {
  default:
    llvm_unreachable("Unknown induction");
  case InductionDescriptor::IK_IntInduction:
    return widenIntInduction(P);
  case InductionDescriptor::IK_FpInduction:
    return widenFpInduction(P);
  }
}

void VPOCodeGen::vectorizeCallInstruction(CallInst *Call) {

  SmallVector<Value *, 2> VecArgs;
  SmallVector<Type *, 2> VecArgTys;
  Function *CalledFunc = Call->getCalledFunction();

  for (Value *Arg : Call->arg_operands()) {
    // TODO: some args may be scalar
    Value *VecArg = getVectorValue(Arg);
    VecArgs.push_back(VecArg);
    VecArgTys.push_back(VecArg->getType());
  }

  Function *VectorF = getOrInsertVectorFunction(CalledFunc, VF, VecArgTys, TLI,
                                                Intrinsic::not_intrinsic,
                                                false/*non-masked*/);
  assert(VectorF && "Can't create vector function.");
  CallInst *VecCall = Builder.CreateCall(VectorF, VecArgs);

  if (isa<FPMathOperator>(VecCall))
    VecCall->copyFastMathFlags(Call);

  Loop *Lp = LI->getLoopFor(Call->getParent());
  analyzeCallArgMemoryReferences(Call, VecCall, TLI, PSE.getSE(), Lp);

  WidenMap[cast<Value>(Call)] = VecCall;
}

void VPOCodeGen::vectorizeInstruction(Instruction *Inst) {
  switch (Inst->getOpcode()) {
  case Instruction::GetElementPtr: {
    // Create the vector GEP, keeping all constant arguments scalar.
    GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Inst);
    SmallVector<Value*, 4> OpsV;
    for (Value *Op : GEP->operands()) {
      Instruction *SrcInst = dyn_cast<Instruction>(Op);
      if (SrcInst && OrigLoop->contains(SrcInst))
        OpsV.push_back(getVectorValue(Op));
      else
        OpsV.push_back(Op);
    }
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
  case Instruction::FPTrunc:
  case Instruction::BitCast: {
    CastInst *CI = dyn_cast<CastInst>(Inst);

    /// Vectorize casts.
    Type *VecTy = VectorType::get(CI->getType(), VF);

    Value *A = getVectorValue(Inst->getOperand(0));
    WidenMap[cast<Value>(Inst)] = Builder.CreateCast(CI->getOpcode(), A, VecTy);
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
    vectorizeStoreInstruction(Inst, false);
    break;
  }

  case Instruction::PHI: {
    vectorizePHIInstruction(Inst);
    break;
  }

  case Instruction::Call: {
    // Currently, the LLVM code gen side does not let AVRIf nodes flow through
    // during loop legalization (loopIsHandled()), so we don't need to worry
    // about masked vector function calls yet.
    CallInst *Call = cast<CallInst>(Inst);
    StringRef CalledFunc = Call->getCalledFunction()->getName();
    if (TLI->isFunctionVectorizable(CalledFunc)) {
      vectorizeCallInstruction(Call);
    } else {
      errs() << "Function is serialized\n";
      serializeInstruction(Inst);
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
      WidenMap[cast<Value>(Inst)] = tv;
      
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

bool VPOVectorizationLegality::isLoopInvariant(Value *V) {
  return (PSE.getSE()->isLoopInvariant(PSE.getSCEV(V), TheLoop));
}

bool VPOVectorizationLegality::isConsecutivePtr(Value *Ptr) {
  const ValueToValueMap &Strides = ValueToValueMap();

  int Stride = getPtrStride(PSE, Ptr, TheLoop, Strides, false);
  if (Stride == 1 || Stride == -1)
    return Stride;
  return 0;
}

bool VPOVectorizationLegality::isInductionVariable(const Value *V) {
  Value *In0 = const_cast<Value *>(V);
  PHINode *PN = dyn_cast_or_null<PHINode>(In0);
  if (!PN)
    return false;
  return Inductions.count(PN);
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
