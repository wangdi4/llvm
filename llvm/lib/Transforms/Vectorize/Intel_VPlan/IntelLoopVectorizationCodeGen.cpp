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

  // Inform SCEV analysis to forget original loop
  PSE.getSE()->forgetLoop(OrigLoop);

  // Save the state.
  LoopVectorPreHeader = Lp->getLoopPreheader();

  // Get ready to start creating new instructions into the vector preheader.
  Builder.SetInsertPoint(&*LoopVectorPreHeader->getFirstInsertionPt());
}

void VPOCodeGen::finalizeLoop() {

  fixOutgoingValues();

  fixNonInductionVPPhis();

  updateAnalysis();

  fixLCSSAPHIs();

  predicateInstructions();
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

Value *VPOCodeGen::getVectorValue(VPValue *V) {
  return getVectorValueUplifted(V);
}

Value *VPOCodeGen::getScalarValue(VPValue *V, unsigned Lane) {
  return getScalarValueUplifted(V, Lane);
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
  // If the PHI is not being blended into a select, go ahead and create a PHI
  // and return after adding it to the PHIsToFix map.
  if (VPPhi->getBlend() == false) {
    auto PhiTy = VPPhi->getType();
    PHINode *NewPhi;
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

  VPWidenMap[VPPhi] = BlendVal;
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

void VPOCodeGen::addMaskToSVMLCall(Function *OrigF,
                                   SmallVectorImpl<Value *> &VecArgs,
                                   SmallVectorImpl<Type *> &VecArgTys) {
  assert(MaskValue && "Expected mask to be present");
  VectorType *VecTy = cast<VectorType>(VecArgTys[0]);
  assert(VecTy->getVectorNumElements() ==
             MaskValue->getType()->getVectorNumElements() &&
         "Re-vectorization of SVML functions is not supported yet");

  if (VecTy->getBitWidth() < 512) {
    // For 128-bit and 256-bit masked calls, mask value is appended to the
    // parameter list. For example:
    //
    //  %sin.vec = call <4 x float> @__svml_sinf4_mask(<4 x float>, <4 x i32>)
    VectorType *MaskTyExt = VectorType::get(
        IntegerType::get(OrigF->getContext(), VecTy->getScalarSizeInBits()),
        VecTy->getElementCount());
    Value *MaskValueExt = Builder.CreateSExt(MaskValue, MaskTyExt);
    VecArgTys.push_back(MaskTyExt);
    VecArgs.push_back(MaskValueExt);
  } else {
    // Compared with 128-bit and 256-bit calls, 512-bit masked calls need extra
    // pass-through source parameters. We don't care about masked-out lanes, so
    // just pass undef for that parameter. For example:
    //
    // %sin.vec = call <16 x float> @__svml_sinf16_mask(<16 x float>, <16 x i1>,
    //            <16 x float>)
    SmallVector<Type *, 1> NewArgTys;
    SmallVector<Value *, 1> NewArgs;

    Constant *Undef = UndefValue::get(VecTy);

    NewArgTys.push_back(VecTy);
    NewArgs.push_back(Undef);

    NewArgTys.push_back(MaskValue->getType());
    NewArgs.push_back(MaskValue);

    NewArgTys.append(VecArgTys.begin(), VecArgTys.end());
    NewArgs.append(VecArgs.begin(), VecArgs.end());

    VecArgTys = std::move(NewArgTys);
    VecArgs = std::move(NewArgs);
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

void VPOCodeGen::vectorizeInstruction(VPInstruction *VPInst) {
  if (auto *VPPhi = dyn_cast<VPPHINode>(VPInst)) {
    vectorizeVPPHINode(VPPhi);
    return;
  }

  vectorizeVPInstruction(VPInst);
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
