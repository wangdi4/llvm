//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Intrinsics.h"
#include <tuple>

using namespace llvm;
using namespace llvm::vpo;

ReductionMngr::ReductionMngr(AVR *Avr) {
  ReductionClause *RC = cast<AVRWrn>(Avr)->getWrnNode()->getRed();
  if (!RC)
    return;
  for (ReductionItem *Ri : RC->items())
    ReductionMap[Ri->getCombiner()] = Ri;
}

void ReductionMngr::saveLoopEntryExit(BasicBlock *Preheader,
                                      BasicBlock *ExitBlock) {
  LoopPreheader = Preheader;
  LoopExit = ExitBlock;
}

static Value *CreateBinOp(ReductionItem::WRNReductionKind RKind,
                          IRBuilder<> &Builder, Value *V1, Value *V2) {
  const char *Name = "reduction.tail";
  switch (RKind) {
  case ReductionItem::WRNReductionBxor:
    return Builder.CreateXor(V1, V2, Name);
  case ReductionItem::WRNReductionBor:
    return Builder.CreateOr(V1, V2, Name);
  case ReductionItem::WRNReductionSum:
    return V1->getType()->isFloatTy() ? Builder.CreateFAdd(V1, V2, Name)
                                      : Builder.CreateAdd(V1, V2, Name);
  case ReductionItem::WRNReductionMult:
    return V1->getType()->isFloatTy() ? Builder.CreateFMul(V1, V2, Name)
                                      : Builder.CreateMul(V1, V2, Name);
  case ReductionItem::WRNReductionBand:
    return Builder.CreateAnd(V1, V2, Name);
  default:
    llvm_unreachable("Unknown recurrence kind");
  }
}

// Complete edges of the reduction Phi and build the horizontal
// loop tail.
void ReductionMngr::completeReductionPhis(
    std::map<Value *, Value *> &WidenMap) {
  for (auto Itr : ReductionPhiMap) {
    ReductionItem *RI = Itr.second;
    Value *OrigValue = RI->getCombiner();
    Value *VecValue = WidenMap[OrigValue];
    PHINode *VecPhi = Itr.first;
    VecPhi->addIncoming(VecValue, cast<Instruction>(VecValue)->getParent());

    // loop tail, non-optimized meanwhile
    unsigned VL = VecValue->getType()->getVectorNumElements();
    IRBuilder<> Builder(&*(LoopExit->getFirstInsertionPt()));

    Value *Res = Builder.CreateExtractElement(VecValue, Builder.getInt32(0));
    for (unsigned i = 1; i < VL; ++i) {
      Value *Op = Builder.CreateExtractElement(VecValue, Builder.getInt32(i));
      Res = CreateBinOp(RI->getType(), Builder, Res, Op);
    }
    // Replace all uses of "Combiner" except the block of scalar loop
    BasicBlock *OrigBB = cast<Instruction>(OrigValue)->getParent();
    OrigValue->replaceUsesOutsideBlock(Res, OrigBB);
  }
}

bool ReductionMngr::isReductionVariable(const Value *Val) {
  return ReductionMap.find(Val) != ReductionMap.end();
}

ReductionItem *ReductionMngr::getReductionInfo(const Value *Val) {
  return ReductionMap[Val];
}

bool ReductionMngr::isReductionPhi(const PHINode *PhiInst) {
  if (PhiInst->getNumIncomingValues() != 2)
    return false;
  const Value *In0 = PhiInst->getIncomingValue(0);
  const Value *In1 = PhiInst->getIncomingValue(1);
  return isReductionVariable(In0) || isReductionVariable(In1);
}

/// This function returns the identity element (or neutral element) for
/// the operation K.
Constant *
ReductionMngr::getRecurrenceIdentity(ReductionItem::WRNReductionKind RKind,
                                     Type *Ty) {
  switch (RKind) {
  case ReductionItem::WRNReductionBxor:
  case ReductionItem::WRNReductionBor:
    // Adding, Xoring, Oring zero to a number does not change it.
    return ConstantInt::get(Ty, 0);
  case ReductionItem::WRNReductionSum:
    return Ty->isFloatTy() ? ConstantFP::get(Ty, 0.0L)
                           : ConstantInt::get(Ty, 0);
  case ReductionItem::WRNReductionMult:
    // Multiplying a number by 1 does not change it.
    return Ty->isFloatTy() ? ConstantInt::get(Ty, 1)
                           : ConstantFP::get(Ty, 1.0L);
  case ReductionItem::WRNReductionBand:
    // AND-ing a number with an all-1 value does not change it.
    return ConstantInt::get(Ty, -1, true);
  default:
    llvm_unreachable("Unknown recurrence kind");
  }
}

Instruction *ReductionMngr::vectorizePhiNode(PHINode *RdxPhi, unsigned VL) {
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

  Type *ScalarTy = RdxPhi->getType();
  ReductionItem::WRNReductionKind RKind = RI->getType();
  Constant *Iden = getRecurrenceIdentity(RKind, ScalarTy);
  Value *Identity = ConstantVector::getSplat(VL, Iden);

  // Reduction Phi has 2 incoming adges - one from initial value
  // and the second from loop body. Now we are setting only the
  // first incoming edge.
  IRBuilder<> Builder(LoopPreheader->getTerminator());
  // This vector is the Identity vector where the first element is the
  // incoming scalar reduction.
  Value *VectorStart = Builder.CreateInsertElement(
      Identity, RI->getInitializer(), Builder.getInt32(0));

  Builder.SetInsertPoint(PhiInsertPt);
  PHINode *VecRdxPhi =
      Builder.CreatePHI(VectorType::get(ScalarTy, VL), 2, "vec.rdx.phi");
  VecRdxPhi->addIncoming(VectorStart, LoopPreheader);
  ReductionPhiMap[VecRdxPhi] = RI;
  return VecRdxPhi;
}

void AVRCodeGen::vectorizeReductionPHI(PHINode *RdxPhi) {
  Instruction *VecRdxPhi = RM.vectorizePhiNode(RdxPhi, VL);
  WidenMap[RdxPhi] = VecRdxPhi;
}

void AVRCodeGen::completeReductions() { RM.completeReductionPhis(WidenMap); }

bool AVRCodeGen::loopIsHandled() {
  AVRWrn *AWrn = nullptr;
  AVRLoop *ALoop = nullptr;
  int VL = 0;
  AVRBranchIR *LoopBackEdge = nullptr;
  AVRPhiIR *InductionPhi = nullptr;
  AVRCompare *InductionCmp = nullptr;

  // We expect avr to be a AVRWrn node
  if (!(AWrn = dyn_cast<AVRWrn>(Avr))) {
    return false;
  }

  // An AVRWrn node is expected to have only one AVRLoop child
  for (auto Itr = AWrn->child_begin(), E = AWrn->child_end(); Itr != E; ++Itr) {
    if (AVRLoop *TempALoop = dyn_cast<AVRLoop>(Itr)) {
      if (ALoop) {
        return false;
      }
      ALoop = TempALoop;
    }
  }

  // Check that we have an AVRLoop
  if (!ALoop) {
    return false;
  }

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
      break;
    case AVR::AVRPhiIRNode: {
      AVRPhiIR *Phi = cast<AVRPhiIR>(&Itr);
      if (RM.isReductionPhi(cast<PHINode>(Phi->getLLVMInstruction())))
        break;
      else if (InductionPhi)
        return false;
      else
        InductionPhi = Phi;
    } break;
    case AVR::AVRCompareIRNode:
      if (InductionCmp)
        return false;
      InductionCmp = dyn_cast<AVRCompare>(&Itr);
      break;
    case AVR::AVRBranchIRNode:
      LoopBackEdge = dyn_cast<AVRBranchIR>(&Itr);
      break;
    default:
      return false;
    }
  }

  assert(ALoop && "Expected AVRLoop!");
  assert(InductionPhi && "Expected AVRPhiIR for Induction Variable!");
  assert(InductionCmp && "Expected AVRIf for loop exit check!");

  const PHINode *PhiInst;
  unsigned int NumPhiValues;

  PhiInst = dyn_cast<const PHINode>(InductionPhi->getLLVMInstruction());
  NumPhiValues = PhiInst->getNumIncomingValues();
  if (NumPhiValues != 2) {
    return false;
  }

  // Only support integer type induction vars for now
  if (!PhiInst->getType()->isIntegerTy()) {
    return false;
  }

  ConstantInt *StrideValue;

  // Only handle constant integer stride of 1 for now
  InductionDescriptor ID;
  if (!InductionDescriptor::isInductionPHI(const_cast<PHINode *>(PhiInst), SE,
                                           ID) ||
      !(StrideValue = ID.getStepValue()) || !StrideValue->equalsInt(1)) {
    return false;
  }

  Loop *L;
  Value *StartValue;

  L = LI->getLoopFor(PhiInst->getParent());
  if (!L) {
    return false;
  }

  for (unsigned i = 0; i != NumPhiValues; ++i) {
    if (!(L->contains(PhiInst->getIncomingBlock(i)))) {
      // Starting loop induction value
      StartValue = PhiInst->getIncomingValue(i);
      break;
    }
  }

  // Only StartValue of zero for now
  if (ConstantInt *CStart = dyn_cast<ConstantInt>(StartValue)) {
    if (!CStart->isZero()) {
      return false;
    }
  } else {
    return false;
  }

  // Check that loop trip count is a multiple of vector length
  BasicBlock *ExitingBlock = L->getLoopLatch();

  // Give up if we fail to get loop latch
  if (!ExitingBlock) {
    return false;
  }

  unsigned int TripCount = SE->getSmallConstantTripCount(L, ExitingBlock);

  if (!TripCount) {
    errs() << "VPO_OPTREPORT: Vectorization failed: "
              "failed to compute loop trip count\n";
#ifndef NDEBUG
    L->dump();
#endif
  }

  VL = AWrn->getSimdVectorLength();

  // Assume a default vectorization factor of 4
  if (VL == 0) {
    VL = 4;
  }

  // Check that trip count is a multiple of vector length. No remainder loop
  // is generated currently.
  if (TripCount % VL) {
    return false;
  }

  setALoop(ALoop);
  setOrigLoop(L);
  setTripCount(TripCount);
  setVL(VL);
  setLoopBackEdge(LoopBackEdge);
  setInductionPhi(InductionPhi);
  setInductionCmp(InductionCmp);
  setStartValue(StartValue);
  setStrideValue(StrideValue);

  // errs() << "Legal loop\n";
  return true;
}

void AVRCodeGen::createEmptyLoop() {
  BasicBlock *LoopPreHeader = OrigLoop->getLoopPreheader();
  BasicBlock *LoopExit = OrigLoop->getExitBlock();
  const PHINode *PhiInst;

  assert(LoopPreHeader && "Must have loop preheader");
  assert(LoopExit && "Must have an exit block");

  PhiInst = dyn_cast<const PHINode>(InductionPhi->getLLVMInstruction());

  BasicBlock *ScalarLoopEntry = LoopPreHeader->splitBasicBlock(
      LoopPreHeader->getTerminator(), "scalar.loop");

  // Create vector loop body
  BasicBlock *VecBody = BasicBlock::Create(F->getContext(), "vector.body", F);
  Builder.SetInsertPoint(VecBody);

  // Create vector loop induction variable
  Type *IdxTy = PhiInst->getType();
  PHINode *Induction = Builder.CreatePHI(IdxTy, 2, "index");
  Constant *Stride = ConstantInt::get(IdxTy, VL);

  // Create index increcment
  Value *NextIdx = Builder.CreateAdd(Induction, Stride, "index.next");

  // Setup induction phi incoming values
  Induction->addIncoming(StartValue, LoopPreHeader);
  Induction->addIncoming(NextIdx, VecBody);

  // Setup new induction var
  setNewInductionVal(cast<Value>(Induction));

  // Create the loop termination check.
  Value *ICmp =
      Builder.CreateICmpEQ(NextIdx, ConstantInt::get(IdxTy, TripCount));
  Builder.CreateCondBr(ICmp, LoopExit, VecBody);

  // Replace LoopPreheader terminator with a conditional branch that always
  // jumps to vector loop body
  Instruction *Term = LoopPreHeader->getTerminator();
  BranchInst *VecBodyBranch = BranchInst::Create(
      VecBody, ScalarLoopEntry,
      ConstantInt::get(IntegerType::get(F->getContext(), 1), 1));
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

void AVRCodeGen::vectorizeLoadInstruction(Instruction *Inst,
                                          bool EmitIntrinsic) {
  if (!EmitIntrinsic) {
    serializeInstruction(Inst);
    return;
  }

  LoadInst *LI = dyn_cast<LoadInst>(Inst);
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

  for (int ElemNum = 0; ElemNum < VL; ++ElemNum) {
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
  for (int i = 0; i < VL; ++i) {
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

bool AVRCodeGen::vectorize() {
  if (!loopIsHandled()) {
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
  }
  completeReductions();
  return true;
}
