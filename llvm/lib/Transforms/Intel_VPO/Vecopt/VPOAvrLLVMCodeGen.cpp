//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
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

#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_VPO/Vecopt/VPOAvrLLVMCodeGen.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/IR/Intrinsics.h"
#include <tuple>

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-ir-loop-vectorize"


// Reduction Manager initialization includes analysis of reduction clause.
// ReductionClause contains address of the reduction variable at this moment.
// Starting from the address, we look for reduction PHI and for the binary
// operation that should be vectorized.
// Reduction PHI is saved as "Initializer", the binary operation is saved as
// "Combiner".
ReductionMngr::ReductionMngr(AVR *Avr) {
  ReductionClause &RC = cast<AVRWrn>(Avr)->getWrnNode()->getRed();
  for (ReductionItem *Ri : RC.items()) {
    auto usedInOnlyOnePhiNode = [](Value *V) {
      PHINode *Phi = 0;
      for (auto U : V->users())
        if (isa<PHINode>(U)) {
          if (Phi) // More than one Phi node
            return (PHINode *)nullptr;
          Phi = cast<PHINode>(U);
        }
      return Phi;
    };

    Value *RedVarPtr = Ri->getOrig();
    assert(isa<PointerType>(RedVarPtr->getType()) &&
           "Variable specified in Reduction directive should be a pointer");

    for (auto U : RedVarPtr->users()) {
      if (!isa<LoadInst>(U))
        continue;
      if (auto PhiNode = usedInOnlyOnePhiNode(U)) {
        Ri->setInitializer(U);
        if (PhiNode->getIncomingValue(0) == U)
          Ri->setCombiner(PhiNode->getIncomingValue(1));
        else
          Ri->setCombiner(PhiNode->getIncomingValue(0));
        break;
      }
    }

    ReductionMap[Ri->getCombiner()] = Ri;
  }
}

// Loop Pre-header and Loop-Exit allows to insert code before and after loop.
void ReductionMngr::saveLoopEntryExit(BasicBlock *Preheader,
                                      BasicBlock *ExitBlock) {
  LoopPreheader = Preheader;
  LoopExit = ExitBlock;
}

/// \brief Return scalar result of horizontal vector binary operation.
/// Horizontal binary operation splits the vector recursively
/// into 2 parts until the VL becomes 2. Then we extract elements from the
/// vector and perform scalar operation.
static Value* buildReductionTail(Value *VectorVal,
                                 Instruction::BinaryOps BOpcode,
                                 IRBuilder<>& Builder) {

  // Take Vector Length from the WideRedInst type
  Type *InstTy = VectorVal->getType();

  unsigned VL = cast<VectorType>(InstTy)->getNumElements();
  if (VL == 2) {
    Value *Lo = Builder.CreateExtractElement(VectorVal, Builder.getInt32(0), "Lo");
    Value *Hi = Builder.CreateExtractElement(VectorVal, Builder.getInt32(1), "Hi");
    return Builder.CreateBinOp(BOpcode, Lo, Hi, "Reduced");
  }
  SmallVector<uint32_t, 16> LoMask, HiMask;
  for (unsigned i = 0; i < VL / 2; ++i)
    LoMask.push_back(i);
  for (unsigned i = VL / 2; i < VL; ++i)
    HiMask.push_back(i);

  Value *Lo = Builder.CreateShuffleVector(VectorVal, UndefValue::get(InstTy), LoMask, "Lo");
  Value *Hi = Builder.CreateShuffleVector(VectorVal, UndefValue::get(InstTy), HiMask, "Hi");
  Value *Result = Builder.CreateBinOp(BOpcode, Lo, Hi, "Reduced");
  return buildReductionTail(Result, BOpcode, Builder);
}


// Complete edges of the reduction Phi and build the horizontal
// loop tail.
void ReductionMngr::completeReductionPhis(
    std::map<Value *, Value *> &WidenMap) {
  for (auto Itr : ReductionMap) {
    ReductionItem *RI = Itr.second;
    BinaryOperator *OrigInst = cast<BinaryOperator>(RI->getCombiner());
    BinaryOperator *VecInst = cast<BinaryOperator>(WidenMap[OrigInst]);
    Value *InitVal = RI->getInitializer();
    PHINode *OrigPhi = cast <PHINode>(*InitVal->user_begin());
    PHINode *VectorPhi = cast <PHINode>(WidenMap[OrigPhi]);

    // Set the second incoming edge for the vectorized phi-node
    VectorPhi->addIncoming(VecInst, VecInst->getParent());

    // Create loop tail
    IRBuilder<> Builder(&*(LoopExit->getFirstInsertionPt()));
    Value *ScalarVal = buildReductionTail(VecInst, OrigInst->getOpcode(), Builder);

    // Take the init value
    Value *Res = Builder.CreateBinOp(OrigInst->getOpcode(), ScalarVal, InitVal);
      
    // Replace all uses of "Combiner" except the block of scalar loop
    BasicBlock *OrigBB = OrigInst->getParent();
    OrigInst->replaceUsesOutsideBlock(Res, OrigBB);
  }
}

bool ReductionMngr::isReductionVariable(const Value *Val) {
  return ReductionMap.find(Val) != ReductionMap.end();
}

ReductionItem *ReductionMngr::getReductionInfo(const Value *Val) {
  if (isReductionVariable(Val))
    return ReductionMap[Val];
  return nullptr;
}

bool ReductionMngr::isReductionPhi(const PHINode *PhiInst) {
  if (PhiInst->getNumIncomingValues() != 2)
    return false;
  const Value *In0 = PhiInst->getIncomingValue(0);
  const Value *In1 = PhiInst->getIncomingValue(1);
  return isReductionVariable(In0) || isReductionVariable(In1);
}

Value *
ReductionMngr::getRecurrenceIdentityVector(ReductionItem *RedItem,
                                           Type *Ty, unsigned VL) {

  assert((Ty->isFloatTy() || Ty->isIntegerTy()) &&
         "Expected FP or Integer scalar type");
  ReductionItem::WRNReductionKind RKind = RedItem->getType();
  RecurrenceDescriptor::RecurrenceKind RDKind;
  switch (RKind) {
  case ReductionItem::WRNReductionBxor:
    RDKind = RecurrenceDescriptor::RK_IntegerXor;
    break;
  case ReductionItem::WRNReductionBand:
    RDKind = RecurrenceDescriptor::RK_IntegerAnd;
    break;
  case ReductionItem::WRNReductionBor:
    RDKind = RecurrenceDescriptor::RK_IntegerOr;
    break;
  case ReductionItem::WRNReductionSum:
    RDKind = Ty->isFloatTy() ? RecurrenceDescriptor::RK_FloatAdd
                             : RecurrenceDescriptor::RK_IntegerAdd;
    break;
  case ReductionItem::WRNReductionMult:
    RDKind = Ty->isFloatTy() ? RecurrenceDescriptor::RK_FloatMult
                             : RecurrenceDescriptor::RK_IntegerMult;
    break;
  default:
    llvm_unreachable("Unknown recurrence kind");
  }
  Constant *Iden = RecurrenceDescriptor::getRecurrenceIdentity(RDKind, Ty);
  return ConstantVector::getSplat(VL, Iden);
}

Instruction *ReductionMngr::vectorizePhiNode(PHINode *RdxPhi, unsigned VL) {

  // Reduction Phi has 2 incoming edges - one from initial value
  // and the second from loop body. Now we are setting only the
  // first incoming edge.
  Value *In0 = RdxPhi->getIncomingValue(0);
  Value *In1 = RdxPhi->getIncomingValue(1);

  ReductionItem *RI = getReductionInfo(In0);
  if (RI)
    assert(In1 == RI->getInitializer() && "Unexpected reduction phi node");
  else {
    RI = getReductionInfo(In1);
    assert(RI && In0 == RI->getInitializer() &&
           "Unexpected reduction phi node");
  }

  // Get Identity vector according to reduction operation
  // Min/Max reductions are not handled here
  Value *Identity = getRecurrenceIdentityVector(RI, RdxPhi->getType(), VL);

  // Create vectorized Phi node and set the first edge
  IRBuilder<> Builder(PhiInsertPt);
  PHINode *VecRdxPhi = Builder.CreatePHI(VectorType::get(RdxPhi->getType(), VL),
                                         2, "vec.rdx.phi");
  VecRdxPhi->addIncoming(Identity, LoopPreheader);

  return VecRdxPhi;
}

void AVRCodeGen::vectorizeReductionPHI(PHINode *RdxPhi) {
  Instruction *VecRdxPhi = RM.vectorizePhiNode(RdxPhi, VL);
  WidenMap[RdxPhi] = VecRdxPhi;
}

void AVRCodeGen::completeReductions() {
  RM.completeReductionPhis(WidenMap);
}

bool AVRLoopVectorizationLegality::canVectorizeLoop(AVRLoopIR *ALoop,
                                                    ReductionMngr *RM) {
  for (AVR &Itr : ALoop->nodes()) {
    switch (Itr.getAVRID()) {

    case AVR::AVRAssignIRNode:
    case AVR::AVRLabelIRNode:
    case AVR::AVRCallIRNode:
    case AVR::AVRCompareIRNode:
    case AVR::AVRBranchIRNode:
      break;
    case AVR::AVRPhiIRNode: {
      AVRPhiIR *Phi = cast<AVRPhiIR>(&Itr);
      PHINode *PhiInstr = cast<PHINode>(Phi->getLLVMInstruction());
      assert(TheLoop == LI->getLoopFor(PhiInstr->getParent()) &&
             "Unexpected Phi node");
      InductionDescriptor ID;
      if (InductionDescriptor::isInductionPHI(PhiInstr, TheLoop, PSE, ID)) {
        addInductionPhi(PhiInstr, ID, AllowedExit);
        continue;
      }
      if (RM->isReductionPhi(PhiInstr))
        continue;
      /*
      FIXME: Reduction Auto-detection will be added later.
      RecurrenceDescriptor RedDes;
      if (RecurrenceDescriptor::isReductionPHI(PhiInstr, TheLoop, RedDes)) {
        AllowedExit.insert(RedDes.getLoopExitInstr());
        Reductions[PhiInstr] = RedDes;
        continue;
      }*/
    }
    }
  }
  if (!Induction)
    return false;
  return true;
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

void AVRLoopVectorizationLegality::addInductionPhi(
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

// TODO: Take as input a VPOVecContext that indicates which AVRLoop(s)
// is (are) to be vectorized, as identified by the vectorization scenario
// evaluation.
// FORNOW there is only one AVRLoop per region, so we will re-discover
// the same AVRLoop that the vecScenarioEvaluation had "selected".
bool AVRCodeGen::loopIsHandled(unsigned int VF) {

  AVRWrn *AWrn = nullptr;
  AVRBranchIR *LoopBackEdge = nullptr;

  // We expect avr to be a AVRWrn node
  if (!(AWrn = dyn_cast<AVRWrn>(Avr))) {
    return false;
  }

  if (!ALoop)
    ALoop = AVRUtils::findAVRLoop(AWrn);

  // Check that we have an AVRLoop
  if (!ALoop) {
    return false;
  }

  AVRLoopIR *ALoopIR = cast<AVRLoopIR>(ALoop);
  OrigLoop = ALoopIR->getLoop();

  Legal = new AVRLoopVectorizationLegality(OrigLoop, SE, TLI, F, LI);

  if (!Legal->canVectorizeLoop(ALoopIR, &RM))
    return false;

  // Currently we only handle AVRAssignIR, AVRPhiIR, AVRIf,
  // AVRBranchIR, AVRLabelIR. We alse expect to see one AVRIf
  // for the induction var compare, one branch for loop
  // backedge and one AVRPhiIR for loop induction variable.
  // AVRLabelIRs are ignored for now.
  for (AVR &Itr : ALoop->nodes()) {
    switch (Itr.getAVRID()) {

    case AVR::AVRAssignIRNode:
    case AVR::AVRLabelIRNode:
    case AVR::AVRCallIRNode:
    case AVR::AVRCompareIRNode:
      break;
    case AVR::AVRPhiIRNode: {
      AVRPhiIR *Phi = cast<AVRPhiIR>(&Itr);
      PHINode *PhiInst = cast<PHINode>(Phi->getLLVMInstruction());
      if (PhiInst == Legal->getInduction()) {
        setInductionPhi(Phi);
        if (OrigLoop->contains(PhiInst->getIncomingBlock(0)))
          setStartValue(PhiInst->getIncomingValue(1));
        else
          setStartValue(PhiInst->getIncomingValue(0));
      }
    } break;
    case AVR::AVRBranchIRNode:
      LoopBackEdge = dyn_cast<AVRBranchIR>(&Itr);
      break;
    default:
      return false;
    }
  }

  assert(InductionPhi && "Expected AVRPhiIR for Induction Variable!");

  // Check that loop trip count is a multiple of vector length
  BasicBlock *ExitingBlock = OrigLoop->getLoopLatch();

  // Give up if we fail to get loop latch
  if (!ExitingBlock) {
    return false;
  }

  unsigned ConstTripCount = SE->getSmallConstantTripCount(OrigLoop,
                                                          ExitingBlock);

  if (!ConstTripCount) {
    errs() << "VPO_OPTREPORT: Vectorization failed: "
              "failed to compute loop trip count\n";
#ifndef NDEBUG
    OrigLoop->dump();
#endif
  }
  if (VF && ConstTripCount % VF)
    return false;
  setTripCount(ConstTripCount);
  setALoop(ALoop);
  setLoopBackEdge(LoopBackEdge);
  setStartValue(StartValue);

  return true;
}

void AVRCodeGen::createEmptyLoop() {
  BasicBlock *LoopPreHeader = OrigLoop->getLoopPreheader();
  BasicBlock *LoopExit = OrigLoop->getExitBlock();
  const PHINode *PhiInst;

  assert(LoopPreHeader && "Must have loop preheader");
  assert(LoopExit && "Must have an exit block");

  PhiInst = dyn_cast<const PHINode>(InductionPhi->getLLVMInstruction());

  LoopPreHeader->splitBasicBlock(LoopPreHeader->getTerminator(), "scalar.loop");

  // Create vector loop body
  BasicBlock *VecBody = BasicBlock::Create(F->getContext(), "vector.body", F);
  Builder.SetInsertPoint(VecBody);

  // Create vector loop induction variable
  Type *IdxTy = PhiInst->getType();
  PHINode *Induction = Builder.CreatePHI(IdxTy, 2, "index");
  Constant *Stride = ConstantInt::get(IdxTy, VL);

  // Create index increment
  Value *NextIdx = Builder.CreateAdd(Induction, Stride, "index.next");

  // Setup induction phi incoming values
  Value *StartValue;
  for (unsigned i = 0; i != 2; ++i) {
    if (!(OrigLoop->contains(PhiInst->getIncomingBlock(i)))) {
      // Starting loop induction value
      StartValue = PhiInst->getIncomingValue(i);
      break;
    }
  }

  Induction->addIncoming(StartValue, LoopPreHeader);
  Induction->addIncoming(NextIdx, VecBody);

  // Setup new induction var
  setNewInductionVal(cast<Value>(Induction));

  // Create the loop termination check.
  Value *ICmp =
      Builder.CreateICmpEQ(NextIdx, ConstantInt::get(IdxTy, TripCount));
  Builder.CreateCondBr(ICmp, LoopExit, VecBody);

  // Replace LoopPreheader terminator with an unconditional branch that
  // always jumps to vector loop body. The branch must be unconditional
  // because values that are live outside of vector.body must reside in
  // basic blocks that dominate their uses. We previously relied on the
  // CFG simplification pass to clean up "conditional always true" branches,
  // but this code has been removed from the end of VPODriver because it
  // is not safe to call another transform pass within a transform pass.
  Instruction *Term = LoopPreHeader->getTerminator();
  BranchInst *VecBodyBranch = BranchInst::Create(VecBody);
  ReplaceInstWithInst(Term, VecBodyBranch);
  Builder.SetInsertPoint(cast<Instruction>(NextIdx));

  // Inform SCEV analysis to forget original loop
  SE->forgetLoop(OrigLoop);

  RM.saveInsertPointForReductionPhis(Induction);
  RM.saveLoopEntryExit(LoopPreHeader, LoopExit);
}

Value *AVRCodeGen::getVectorValue(Value *V) {
  assert(!V->getType()->isVectorTy() && "Can't widen a vector");

  // If we have this scalar in the map, return it.
  if (WidenMap.find(V) != WidenMap.end())
    return WidenMap[V];

  // If this scalar is unknown, assume that it is a constant or that it is
  // loop invariant. Broadcast V and save the value for future uses.
  Value *B = Builder.CreateVectorSplat(VL, V, "broadcast");
  WidenMap[V] = B;

  return B;
}

Value *AVRCodeGen::getScalarValue(Value *V) {
  // If the value is not an instruction contained in the loop, it should
  // already be scalar.
  if (OrigLoop->isLoopInvariant(V))
    return V;

  Value *VecV = getVectorValue(V);
  return Builder.CreateExtractElement(VecV, Builder.getInt32(0));
}

Value *AVRCodeGen::reverseVector(Value *Vec) {
  assert(Vec->getType()->isVectorTy() && "Invalid type");
  SmallVector<Constant *, 8> ShuffleMask;
  for (unsigned i = 0; i < VL; ++i)
    ShuffleMask.push_back(Builder.getInt32(VL - i - 1));

  return Builder.CreateShuffleVector(Vec, UndefValue::get(Vec->getType()),
                                     ConstantVector::get(ShuffleMask),
                                     "reverse");
}

void AVRCodeGen::vectorizeLoopInvariantLoad(Instruction *Inst) {
  Instruction *Cloned = Inst->clone();
  Cloned->setName(Inst->getName() + ".cloned");
  Builder.Insert(Cloned);
  Value *Broadcast = Builder.CreateVectorSplat(VL, Cloned, "broadcast");
  WidenMap[Inst] = Broadcast;
}

void AVRCodeGen::vectorizeLoadInstruction(Instruction *Inst,
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

  Type *DataTy = VectorType::get(LI->getType(), VL);
  unsigned Alignment = LI->getAlignment();
  // An alignment of 0 means target abi alignment. We need to use the scalar's
  // target abi alignment in such a case.
  const DataLayout &DL = Inst->getModule()->getDataLayout();
  if (!Alignment)
    Alignment = DL.getABITypeAlignment(LI->getType());
  unsigned AddressSpace = Ptr->getType()->getPointerAddressSpace();


  // Handle consecutive loads/stores.
  GetElementPtrInst *Gep = dyn_cast<GetElementPtrInst>(Ptr);
  if (ConsecutiveStride != 0) {
    if (Gep) {
      GetElementPtrInst *Gep2 = cast<GetElementPtrInst>(Gep->clone());
      Gep2->setName("gep.indvar");

      for (unsigned i = 0; i < Gep->getNumOperands(); ++i)
        Gep2->setOperand(i, getScalarValue(Gep->getOperand(i)));
      Ptr = Builder.Insert(Gep2);
    }
    else // No GEP
      Ptr = getScalarValue(Ptr);

    Value *PartPtr = Reverse ?
      Builder.CreateGEP(nullptr, Ptr, Builder.getInt32(1 - VL)) : Ptr;   

    Value *VecPtr =
      Builder.CreateBitCast(PartPtr, DataTy->getPointerTo(AddressSpace));
    Value *NewLI = Builder.CreateAlignedLoad(VecPtr, Alignment, "wide.load");
    if (Reverse)
      NewLI = reverseVector(NewLI);
    WidenMap[cast<Value>(Inst)] = NewLI;
    return;
  }
  Instruction *NewLI;
  Value *VecPtrOp;

  VecPtrOp = getVectorValue(Inst->getOperand(0));
  Intrinsic::ID IntrinsicID = Intrinsic::masked_gather;

  Type *i1Ty = Type::getInt1Ty(F->getContext()); // Mask type
  Type *i32Ty = Type::getInt32Ty(F->getContext());

  // Unmasked for now
  Value *Mask = ConstantVector::getSplat(VL, ConstantInt::get(i1Ty, 1));
  Type *VectorOfPointersType = VecPtrOp->getType();
  Type *ElementPointerType = VectorOfPointersType->getVectorElementType();
  Type *ElementType = ElementPointerType->getPointerElementType();
  Type *VectorOfElementsType = VectorType::get(ElementType, VL);

  std::vector<Type *> ArgumentTypes;
  std::vector<Value *> Arguments;

  ArgumentTypes.push_back(VectorOfElementsType);

  // Vector of pointers to load
  Arguments.push_back(VecPtrOp);

  // Alignment argument
  Arguments.push_back(ConstantInt::get(i32Ty, LI->getAlignment()));

  // Mask argument
  Arguments.push_back(Mask);

  // Passthru argument
  Arguments.push_back(UndefValue::get(VectorOfElementsType));

  Function *Intrinsic = Intrinsic::getDeclaration(
      F->getParent(), IntrinsicID, ArrayRef<Type *>(ArgumentTypes));
  assert(Intrinsic &&
         "Expected to have an intrinsic for this memory operation");
  NewLI = CallInst::Create(Intrinsic, ArrayRef<Value *>(Arguments), "",
                           &*(Builder.GetInsertPoint()));
  WidenMap[cast<Value>(Inst)] = cast<Value>(NewLI);
}

void AVRCodeGen::vectorizeStoreInstruction(Instruction *Inst,
                                           bool EmitIntrinsic) {
  if (!EmitIntrinsic) {
    serializeInstruction(Inst);
    return;
  }

  StoreInst *SI = dyn_cast<StoreInst>(Inst);
  Instruction *NewSI;
  Value *VecPtrOp, *VecDataOp;

  VecDataOp = getVectorValue(SI->getValueOperand());
  VecPtrOp = getVectorValue(SI->getPointerOperand());

  Intrinsic::ID IntrinsicID = Intrinsic::masked_scatter;

  Type *i1Ty = Type::getInt1Ty(F->getContext()); // Mask type
  Type *i32Ty = Type::getInt32Ty(F->getContext());

  // Unmasked for now
  Value *Mask = ConstantVector::getSplat(VL, ConstantInt::get(i1Ty, 1));
  Type *VectorOfPointersType = VecPtrOp->getType();
  Type *ElementPointerType = VectorOfPointersType->getVectorElementType();
  Type *ElementType = ElementPointerType->getPointerElementType();
  Type *VectorOfElementsType = VectorType::get(ElementType, VL);

  std::vector<Type *> ArgumentTypes;
  std::vector<Value *> Arguments;

  ArgumentTypes.push_back(VectorOfElementsType);

  // Vector of data, pointers to store
  Arguments.push_back(VecDataOp);
  Arguments.push_back(VecPtrOp);

  // Alignment argument
  Arguments.push_back(ConstantInt::get(i32Ty, SI->getAlignment()));

  // Mask argument
  Arguments.push_back(Mask);

  Function *Intrinsic = Intrinsic::getDeclaration(
      F->getParent(), IntrinsicID, ArrayRef<Type *>(ArgumentTypes));
  assert(Intrinsic &&
         "Expected to have an intrinsic for this memory operation");
  NewSI = CallInst::Create(Intrinsic, ArrayRef<Value *>(Arguments), "",
                           &*(Builder.GetInsertPoint()));

  // Is this needed??
  WidenMap[cast<Value>(Inst)] = cast<Value>(NewSI);
}

void AVRCodeGen::serializeInstruction(Instruction *Inst) {
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
                        : UndefValue::get(VectorType::get(Inst->getType(), VL));

  // Create a new entry in the WidenMap and initialize it to Undef or Null.
  Value *VecResults = UndefVec;

  for (unsigned ElemNum = 0; ElemNum < VL; ++ElemNum) {
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

Value *AVRCodeGen::getStrideVector(Value *Val, Value *Stride) {
  assert(Val->getType()->isVectorTy() && "Must be a vector");
  assert(Val->getType()->getScalarType()->isIntegerTy() &&
         "Elem must be an integer");
  assert(Stride->getType() == Val->getType()->getScalarType() &&
         "Stride has wrong type");

  // Create the types.
  Type *ITy = Val->getType()->getScalarType();
  SmallVector<Constant *, 8> Indices;

  // Create a vector of consecutive numbers from zero to VL.
  for (unsigned i = 0; i < VL; ++i) {
    Indices.push_back(ConstantInt::get(ITy, i));
  }

  // Add the consecutive indices to the vector value.
  Constant *Cv = ConstantVector::get(Indices);
  assert(Cv->getType() == Val->getType() && "Invalid consecutive vec");
  Stride = Builder.CreateVectorSplat(VL, Stride);
  assert(Stride->getType() == Val->getType() && "Invalid stride type");

  // TBD: The newly created binary instructions should contain nsw/nuw flags,
  // which can be found from the original scalar operations.
  Stride = Builder.CreateMul(Cv, Stride);
  return Builder.CreateAdd(Val, Stride, "induction");
}

Value *AVRCodeGen::getBroadcastInstrs(Value *V) {
  // TBD: We need to place the broadcast of invariant variables
  // outside the loop. Broadcast the scalar into all locations
  // in the vector.
  Value *Shuf = Builder.CreateVectorSplat(VL, V, "broadcast");

  return Shuf;
}

void AVRCodeGen::vectorizePHIInstruction(Instruction *Inst) {
  Type *PhiTy;
  Value *Broadcasted;

  assert(InductionPhi->getLLVMInstruction() == Inst &&
         "Unexpected PHI instruction");

  PhiTy = NewInductionVal->getType();

  Broadcasted = getBroadcastInstrs(NewInductionVal);
  Constant *Stride = ConstantInt::getSigned(PhiTy, 1);
  Value *TempVal = getStrideVector(Broadcasted, Stride);

  WidenMap[cast<Value>(Inst)] = TempVal;
}

void AVRCodeGen::vectorizeCallInstruction(CallInst *Call) {

  SmallVector<Value*, 2> VecArgs;
  SmallVector<Type*, 2> VecArgTys;

  for (Value *Arg : Call->arg_operands()) {
    // TODO: some args may be scalar
    Value *VecArg = getVectorValue(Arg);
    VecArgs.push_back(VecArg);
    VecArgTys.push_back(VecArg->getType());
  }

  Function *VectorF = getOrInsertVectorFunction(Call, VL, VecArgTys, TLI,
                                                Intrinsic::not_intrinsic,
                                                nullptr/*simd function*/,
                                                false/*non-masked*/);
  assert(VectorF && "Can't create vector function.");
  CallInst *VecCall = Builder.CreateCall(VectorF, VecArgs);

  if (isa<FPMathOperator>(VecCall))
    VecCall->copyFastMathFlags(Call);

  Loop *Lp = LI->getLoopFor(Call->getParent());
  analyzeCallArgMemoryReferences(Call, VecCall, TLI, SE, Lp);

  WidenMap[cast<Value>(Call)] = VecCall;
}

void AVRCodeGen::vectorizeInstruction(Instruction *Inst) {
  switch (Inst->getOpcode()) {
  case Instruction::GetElementPtr: {
    // Create the vector GEP, keeping all constant arguments scalar.
    GetElementPtrInst *GI = dyn_cast<GetElementPtrInst>(Inst);
    Value *Base = GI->getPointerOperand();
    Value *VecBase = getVectorValue(Base);

    GetElementPtrInst::op_iterator IdxIt = GI->idx_begin();
    GetElementPtrInst::op_iterator IdxEnd = GI->idx_end();
    SmallVector<Value *, 8> Indices;

    for (; IdxIt != IdxEnd; ++IdxIt) {
      Value *Index;

      Index = getVectorValue(*IdxIt);
      Indices.push_back(Index);
    }
    GetElementPtrInst *VectorGEP = cast<GetElementPtrInst>(
        Builder.CreateGEP(VecBase, Indices, "mm_vectorGEP"));
    VectorGEP->setIsInBounds(GI->isInBounds());
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
    Type *VecTy = VectorType::get(CI->getType(), VL);

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
    vectorizeStoreInstruction(Inst, true);
    break;
  }

  case Instruction::PHI: {
    PHINode *Phi = cast<PHINode>(Inst);
    if (RM.isReductionPhi(Phi))
      vectorizeReductionPHI(Phi);
    else
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

// TODO: Change all VL occurences to VF.
// TODO: Have this method take a VecContext as input, which indicates which
// AVRLoops in the region to vectorize, and how (using what VF).
bool AVRCodeGen::vectorize(unsigned int VL) {
  setVL(VL);
  assert(VL >= 1);
  if (VL == 1)
    return false;

  if (!loopIsHandled(VL)) {
    return false;
  }
  createEmptyLoop();

  for (auto Itr = ALoop->child_begin(), E = ALoop->child_end(); Itr != E;
       ++Itr) {
    // Only widen assign and phis. Branch and compares are implicitly
    // handled. Other AVR types are not handled currently.
    if (AVRAssignIR *AssignAvr = dyn_cast<AVRAssignIR>(Itr)) {
      Instruction *Inst;

      Inst = const_cast<Instruction *>(AssignAvr->getLLVMInstruction());
      vectorizeInstruction(Inst);
    }

    if (AVRPhiIR *PhiAvr = dyn_cast<AVRPhiIR>(Itr)) {
      Instruction *Inst;

      Inst = const_cast<Instruction *>(PhiAvr->getLLVMInstruction());
      vectorizeInstruction(Inst);
    }

    if (AVRCallIR *CallAvr = dyn_cast<AVRCallIR>(Itr)) {
      Instruction *Inst;
      Inst = const_cast<Instruction *>(CallAvr->getLLVMInstruction());
      vectorizeInstruction(Inst);
    }
  }
  completeReductions();
  return true;
}

bool AVRLoopVectorizationLegality::isLoopInvariant(Value *V) {
  return (PSE.getSE()->isLoopInvariant(PSE.getSCEV(V), TheLoop));
}

bool AVRLoopVectorizationLegality::isConsecutivePtr(Value *Ptr) {
  const ValueToValueMap &Strides = ValueToValueMap();

  int Stride = getPtrStride(PSE, Ptr, TheLoop, Strides, false);
  if (Stride == 1 || Stride == -1)
    return Stride;
  return 0;
}

bool AVRLoopVectorizationLegality::isInductionVariable(const Value *V) {
  Value *In0 = const_cast<Value *>(V);
  PHINode *PN = dyn_cast_or_null<PHINode>(In0);
  if (!PN)
    return false;
  return Inductions.count(PN);
}
