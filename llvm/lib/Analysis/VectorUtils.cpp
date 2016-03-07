//===----------- VectorUtils.cpp - Vectorizer utility functions -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines vectorizer utilities.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/EquivalenceClasses.h"
#include "llvm/Analysis/DemandedBits.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/Intel_VectorVariant.h" // INTEL

using namespace llvm;
using namespace llvm::PatternMatch;

/// \brief Identify if the intrinsic is trivially vectorizable.
/// This method returns true if the intrinsic's argument types are all
/// scalars for the scalar form of the intrinsic and all vectors for
/// the vector form of the intrinsic.
bool llvm::isTriviallyVectorizable(Intrinsic::ID ID) {
  switch (ID) {
  case Intrinsic::sqrt:
  case Intrinsic::sin:
  case Intrinsic::cos:
  case Intrinsic::exp:
  case Intrinsic::exp2:
  case Intrinsic::log:
  case Intrinsic::log10:
  case Intrinsic::log2:
  case Intrinsic::fabs:
  case Intrinsic::minnum:
  case Intrinsic::maxnum:
  case Intrinsic::copysign:
  case Intrinsic::floor:
  case Intrinsic::ceil:
  case Intrinsic::trunc:
  case Intrinsic::rint:
  case Intrinsic::nearbyint:
  case Intrinsic::round:
  case Intrinsic::bswap:
  case Intrinsic::ctpop:
  case Intrinsic::pow:
  case Intrinsic::fma:
  case Intrinsic::fmuladd:
  case Intrinsic::ctlz:
  case Intrinsic::cttz:
  case Intrinsic::powi:
  case Intrinsic::sincos:
    return true;
  default:
    return false;
  }
}

/// \brief Identifies if the intrinsic has a scalar operand. It check for
/// ctlz,cttz and powi special intrinsics whose argument is scalar.
bool llvm::hasVectorInstrinsicScalarOpd(Intrinsic::ID ID,
                                        unsigned ScalarOpdIdx) {
  switch (ID) {
  case Intrinsic::ctlz:
  case Intrinsic::cttz:
  case Intrinsic::powi:
    return (ScalarOpdIdx == 1);
  default:
    return false;
  }
}

/// \brief Check call has a unary float signature
/// It checks following:
/// a) call should have a single argument
/// b) argument type should be floating point type
/// c) call instruction type and argument type should be same
/// d) call should only reads memory.
/// If all these condition is met then return ValidIntrinsicID
/// else return not_intrinsic.
Intrinsic::ID
llvm::checkUnaryFloatSignature(const CallInst &I,
                               Intrinsic::ID ValidIntrinsicID) {
  if (I.getNumArgOperands() != 1 ||
      !I.getArgOperand(0)->getType()->isFloatingPointTy() ||
      I.getType() != I.getArgOperand(0)->getType() || !I.onlyReadsMemory())
    return Intrinsic::not_intrinsic;

  return ValidIntrinsicID;
}

/// \brief Check call has a binary float signature
/// It checks following:
/// a) call should have 2 arguments.
/// b) arguments type should be floating point type
/// c) call instruction type and arguments type should be same
/// d) call should only reads memory.
/// If all these condition is met then return ValidIntrinsicID
/// else return not_intrinsic.
Intrinsic::ID
llvm::checkBinaryFloatSignature(const CallInst &I,
                                Intrinsic::ID ValidIntrinsicID) {
  if (I.getNumArgOperands() != 2 ||
      !I.getArgOperand(0)->getType()->isFloatingPointTy() ||
      !I.getArgOperand(1)->getType()->isFloatingPointTy() ||
      I.getType() != I.getArgOperand(0)->getType() ||
      I.getType() != I.getArgOperand(1)->getType() || !I.onlyReadsMemory())
    return Intrinsic::not_intrinsic;

  return ValidIntrinsicID;
}

/// \brief Check call has the following signature:
/// a) call should have 3 arguments.
/// b) argument 1 should be floating point.
/// c) arguments 2 and 3 should be pointers to floating point.
/// d) does not matter that call arguments match return.
/// If all these conditions are met then return ValidIntrinsicID
/// else return not_intrinsic.
Intrinsic::ID
checkFloatBinaryFloatPtrSignature(const CallInst &I,
                                  Intrinsic::ID ValidIntrinsicID)
{
  Type *ArgOneType   = I.getArgOperand(0)->getType();
  Type *ArgTwoType   = I.getArgOperand(1)->getType();
  Type *ArgThreeType = I.getArgOperand(2)->getType();

  if (I.getNumArgOperands() != 3 ||
      !ArgOneType->isFPOrFPVectorTy() ||
      !ArgTwoType->isPtrOrPtrVectorTy() ||
      !ArgTwoType->getPointerElementType()->isFPOrFPVectorTy() ||
      !ArgThreeType->isPtrOrPtrVectorTy() ||
      !ArgThreeType->getPointerElementType()->isFPOrFPVectorTy()) {
    return Intrinsic::not_intrinsic;
  }

  return ValidIntrinsicID;
}

/// \brief Returns intrinsic ID for call.
/// For the input call instruction it finds mapping intrinsic and returns
/// its ID, in case it does not found it return not_intrinsic.
Intrinsic::ID llvm::getIntrinsicIDForCall(CallInst *CI,
                                          const TargetLibraryInfo *TLI) {
  // If we have an intrinsic call, check if it is trivially vectorizable.
  if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(CI)) {
    Intrinsic::ID ID = II->getIntrinsicID();
    if (isTriviallyVectorizable(ID) || ID == Intrinsic::lifetime_start ||
        ID == Intrinsic::lifetime_end || ID == Intrinsic::assume)
      return ID;
    return Intrinsic::not_intrinsic;
  }

  if (!TLI)
    return Intrinsic::not_intrinsic;

  LibFunc::Func Func;
  Function *F = CI->getCalledFunction();
  // We're going to make assumptions on the semantics of the functions, check
  // that the target knows that it's available in this environment and it does
  // not have local linkage.
  if (!F || F->hasLocalLinkage() || !TLI->getLibFunc(F->getName(), Func))
    return Intrinsic::not_intrinsic;

  // Otherwise check if we have a call to a function that can be turned into a
  // vector intrinsic.
  switch (Func) {
  default:
    break;
  case LibFunc::sin:
  case LibFunc::sinf:
  case LibFunc::sinl:
    return checkUnaryFloatSignature(*CI, Intrinsic::sin);
  case LibFunc::cos:
  case LibFunc::cosf:
  case LibFunc::cosl:
    return checkUnaryFloatSignature(*CI, Intrinsic::cos);
  case LibFunc::exp:
  case LibFunc::expf:
  case LibFunc::expl:
    return checkUnaryFloatSignature(*CI, Intrinsic::exp);
  case LibFunc::exp2:
  case LibFunc::exp2f:
  case LibFunc::exp2l:
    return checkUnaryFloatSignature(*CI, Intrinsic::exp2);
  case LibFunc::log:
  case LibFunc::logf:
  case LibFunc::logl:
    return checkUnaryFloatSignature(*CI, Intrinsic::log);
  case LibFunc::log10:
  case LibFunc::log10f:
  case LibFunc::log10l:
    return checkUnaryFloatSignature(*CI, Intrinsic::log10);
  case LibFunc::log2:
  case LibFunc::log2f:
  case LibFunc::log2l:
    return checkUnaryFloatSignature(*CI, Intrinsic::log2);
  case LibFunc::fabs:
  case LibFunc::fabsf:
  case LibFunc::fabsl:
    return checkUnaryFloatSignature(*CI, Intrinsic::fabs);
  case LibFunc::fmin:
  case LibFunc::fminf:
  case LibFunc::fminl:
    return checkBinaryFloatSignature(*CI, Intrinsic::minnum);
  case LibFunc::fmax:
  case LibFunc::fmaxf:
  case LibFunc::fmaxl:
    return checkBinaryFloatSignature(*CI, Intrinsic::maxnum);
  case LibFunc::copysign:
  case LibFunc::copysignf:
  case LibFunc::copysignl:
    return checkBinaryFloatSignature(*CI, Intrinsic::copysign);
  case LibFunc::floor:
  case LibFunc::floorf:
  case LibFunc::floorl:
    return checkUnaryFloatSignature(*CI, Intrinsic::floor);
  case LibFunc::ceil:
  case LibFunc::ceilf:
  case LibFunc::ceill:
    return checkUnaryFloatSignature(*CI, Intrinsic::ceil);
  case LibFunc::trunc:
  case LibFunc::truncf:
  case LibFunc::truncl:
    return checkUnaryFloatSignature(*CI, Intrinsic::trunc);
  case LibFunc::rint:
  case LibFunc::rintf:
  case LibFunc::rintl:
    return checkUnaryFloatSignature(*CI, Intrinsic::rint);
  case LibFunc::nearbyint:
  case LibFunc::nearbyintf:
  case LibFunc::nearbyintl:
    return checkUnaryFloatSignature(*CI, Intrinsic::nearbyint);
  case LibFunc::round:
  case LibFunc::roundf:
  case LibFunc::roundl:
    return checkUnaryFloatSignature(*CI, Intrinsic::round);
  case LibFunc::pow:
  case LibFunc::powf:
  case LibFunc::powl:
    return checkBinaryFloatSignature(*CI, Intrinsic::pow);
  case LibFunc::sincos:
  case LibFunc::sincosf:
    CI->setDoesNotAccessMemory();
    return checkFloatBinaryFloatPtrSignature(*CI, Intrinsic::sincos);
  }

  return Intrinsic::not_intrinsic;
}

/// \brief Find the operand of the GEP that should be checked for consecutive
/// stores. This ignores trailing indices that have no effect on the final
/// pointer.
unsigned llvm::getGEPInductionOperand(const GetElementPtrInst *Gep) {
  const DataLayout &DL = Gep->getModule()->getDataLayout();
  unsigned LastOperand = Gep->getNumOperands() - 1;
  unsigned GEPAllocSize = DL.getTypeAllocSize(
      cast<PointerType>(Gep->getType()->getScalarType())->getElementType());

  // Walk backwards and try to peel off zeros.
  while (LastOperand > 1 && match(Gep->getOperand(LastOperand), m_Zero())) {
    // Find the type we're currently indexing into.
    gep_type_iterator GEPTI = gep_type_begin(Gep);
    std::advance(GEPTI, LastOperand - 1);

    // If it's a type with the same allocation size as the result of the GEP we
    // can peel off the zero index.
    if (DL.getTypeAllocSize(*GEPTI) != GEPAllocSize)
      break;
    --LastOperand;
  }

  return LastOperand;
}

/// \brief If the argument is a GEP, then returns the operand identified by
/// getGEPInductionOperand. However, if there is some other non-loop-invariant
/// operand, it returns that instead.
Value *llvm::stripGetElementPtr(Value *Ptr, ScalarEvolution *SE, Loop *Lp) {
  GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Ptr);
  if (!GEP)
    return Ptr;

  unsigned InductionOperand = getGEPInductionOperand(GEP);

  // Check that all of the gep indices are uniform except for our induction
  // operand.
  for (unsigned i = 0, e = GEP->getNumOperands(); i != e; ++i)
    if (i != InductionOperand &&
        !SE->isLoopInvariant(SE->getSCEV(GEP->getOperand(i)), Lp))
      return Ptr;
  return GEP->getOperand(InductionOperand);
}

/// \brief If a value has only one user that is a CastInst, return it.
Value *llvm::getUniqueCastUse(Value *Ptr, Loop *Lp, Type *Ty) {
  Value *UniqueCast = nullptr;
  for (User *U : Ptr->users()) {
    CastInst *CI = dyn_cast<CastInst>(U);
    if (CI && CI->getType() == Ty) {
      if (!UniqueCast)
        UniqueCast = CI;
      else
        return nullptr;
    }
  }
  return UniqueCast;
}

/// \brief Get the stride of a pointer access in a loop. Looks for symbolic
/// strides "a[i*stride]". Returns the symbolic stride, or null otherwise.
Value *llvm::getStrideFromPointer(Value *Ptr, ScalarEvolution *SE, Loop *Lp) {
  auto *PtrTy = dyn_cast<PointerType>(Ptr->getType());
  if (!PtrTy || PtrTy->isAggregateType())
    return nullptr;

  // Try to remove a gep instruction to make the pointer (actually index at this
  // point) easier analyzable. If OrigPtr is equal to Ptr we are analzying the
  // pointer, otherwise, we are analyzing the index.
  Value *OrigPtr = Ptr;

  // The size of the pointer access.
  int64_t PtrAccessSize = 1;

  Ptr = stripGetElementPtr(Ptr, SE, Lp);
  const SCEV *V = SE->getSCEV(Ptr);

  if (Ptr != OrigPtr)
    // Strip off casts.
    while (const SCEVCastExpr *C = dyn_cast<SCEVCastExpr>(V))
      V = C->getOperand();

  const SCEVAddRecExpr *S = dyn_cast<SCEVAddRecExpr>(V);
  if (!S)
    return nullptr;

  V = S->getStepRecurrence(*SE);
  if (!V)
    return nullptr;

  // Strip off the size of access multiplication if we are still analyzing the
  // pointer.
  if (OrigPtr == Ptr) {
    const DataLayout &DL = Lp->getHeader()->getModule()->getDataLayout();
    DL.getTypeAllocSize(PtrTy->getElementType());
    if (const SCEVMulExpr *M = dyn_cast<SCEVMulExpr>(V)) {
      if (M->getOperand(0)->getSCEVType() != scConstant)
        return nullptr;

      const APInt &APStepVal = cast<SCEVConstant>(M->getOperand(0))->getAPInt();

      // Huge step value - give up.
      if (APStepVal.getBitWidth() > 64)
        return nullptr;

      int64_t StepVal = APStepVal.getSExtValue();
      if (PtrAccessSize != StepVal)
        return nullptr;
      V = M->getOperand(1);
    }
  }

  // Strip off casts.
  Type *StripedOffRecurrenceCast = nullptr;
  if (const SCEVCastExpr *C = dyn_cast<SCEVCastExpr>(V)) {
    StripedOffRecurrenceCast = C->getType();
    V = C->getOperand();
  }

  // Look for the loop invariant symbolic value.
  const SCEVUnknown *U = dyn_cast<SCEVUnknown>(V);
  if (!U)
    return nullptr;

  Value *Stride = U->getValue();
  if (!Lp->isLoopInvariant(Stride))
    return nullptr;

  // If we have stripped off the recurrence cast we have to make sure that we
  // return the value that is used in this loop so that we can replace it later.
  if (StripedOffRecurrenceCast)
    Stride = getUniqueCastUse(Stride, Lp, StripedOffRecurrenceCast);

  return Stride;
}

/// \brief Given a vector and an element number, see if the scalar value is
/// already around as a register, for example if it were inserted then extracted
/// from the vector.
Value *llvm::findScalarElement(Value *V, unsigned EltNo) {
  assert(V->getType()->isVectorTy() && "Not looking at a vector?");
  VectorType *VTy = cast<VectorType>(V->getType());
  unsigned Width = VTy->getNumElements();
  if (EltNo >= Width)  // Out of range access.
    return UndefValue::get(VTy->getElementType());

  if (Constant *C = dyn_cast<Constant>(V))
    return C->getAggregateElement(EltNo);

  if (InsertElementInst *III = dyn_cast<InsertElementInst>(V)) {
    // If this is an insert to a variable element, we don't know what it is.
    if (!isa<ConstantInt>(III->getOperand(2)))
      return nullptr;
    unsigned IIElt = cast<ConstantInt>(III->getOperand(2))->getZExtValue();

    // If this is an insert to the element we are looking for, return the
    // inserted value.
    if (EltNo == IIElt)
      return III->getOperand(1);

    // Otherwise, the insertelement doesn't modify the value, recurse on its
    // vector input.
    return findScalarElement(III->getOperand(0), EltNo);
  }

  if (ShuffleVectorInst *SVI = dyn_cast<ShuffleVectorInst>(V)) {
    unsigned LHSWidth = SVI->getOperand(0)->getType()->getVectorNumElements();
    int InEl = SVI->getMaskValue(EltNo);
    if (InEl < 0)
      return UndefValue::get(VTy->getElementType());
    if (InEl < (int)LHSWidth)
      return findScalarElement(SVI->getOperand(0), InEl);
    return findScalarElement(SVI->getOperand(1), InEl - LHSWidth);
  }

  // Extract a value from a vector add operation with a constant zero.
  Value *Val = nullptr; Constant *Con = nullptr;
  if (match(V, m_Add(m_Value(Val), m_Constant(Con))))
    if (Constant *Elt = Con->getAggregateElement(EltNo))
      if (Elt->isNullValue())
        return findScalarElement(Val, EltNo);

  // Otherwise, we don't know.
  return nullptr;
}

/// \brief Get splat value if the input is a splat vector or return nullptr.
/// This function is not fully general. It checks only 2 cases:
/// the input value is (1) a splat constants vector or (2) a sequence
/// of instructions that broadcast a single value into a vector.
///
const llvm::Value *llvm::getSplatValue(const Value *V) {

  if (auto *C = dyn_cast<Constant>(V))
    if (isa<VectorType>(V->getType()))
      return C->getSplatValue();

  auto *ShuffleInst = dyn_cast<ShuffleVectorInst>(V);
  if (!ShuffleInst)
    return nullptr;
  // All-zero (or undef) shuffle mask elements.
  for (int MaskElt : ShuffleInst->getShuffleMask())
    if (MaskElt != 0 && MaskElt != -1)
      return nullptr;
  // The first shuffle source is 'insertelement' with index 0.
  auto *InsertEltInst =
    dyn_cast<InsertElementInst>(ShuffleInst->getOperand(0));
  if (!InsertEltInst || !isa<ConstantInt>(InsertEltInst->getOperand(2)) ||
      !cast<ConstantInt>(InsertEltInst->getOperand(2))->isNullValue())
    return nullptr;

  return InsertEltInst->getOperand(1);
}

// The purpose of this function is to trace back to all GEPs involved in an
// address calculation (e.g., array subscripts) and determine if each, other
// than the level being vectorized (assuming innermost loop vectorization), is
// loop invariant. If so, this corresponds to a base + constant offset
// expression, where we can then use the stride from the SCEV of the base to
// determine the actual stride needed for vectorization.
//
// As an example (trace IR from bottom to top), the LLVM IR
// corresponding to an expression of the form A[i][j][k], where k is being
// vectorized is something like:
//
// for.cond.1.preheader:           ; preds = %for.inc.26, %entry
//   %indvars.iv119 = phi i64 [ 0, %entry ],
//                            [ %indvars.iv.next120, %for.inc.26 ]
//   ... some arbitrary IR here ...
//   %arrayidx14 = getelementptr inbounds double**, double*** %array2,
//                                                  i64 %indvars.iv119, !dbg !8
//   ... some arbitrary IR here ...
//   br label %for.cond.4.preheader, !dbg !10
//
// for.cond.4.preheader:           ; preds = %for.inc.23, %for.cond.1.preheader
//   %indvars.iv116 = phi i64 [ 0, %for.cond.1.preheader ],
//                            [ %indvars.iv.next117, %for.inc.23 ]
//   br label %for.body.6, !dbg !11
//
// for.body.6:                     ; preds = %for.body.6, %for.cond.4.preheader
//   %indvars.iv113 = phi i64 [ 0, %for.cond.4.preheader ],
//                            [ %indvars.iv.next114, %for.body.6 ]
//   ... some arbitrary IR here ...
//   %4 = load double**, double*** %arrayidx14, align 8, !dbg !8, !tbaa !12
//
//   %arrayidx15 = getelementptr inbounds double*, double** %4,
//                                                 i64 %indvars.iv116, !dbg !8
//
//   %5 = load double*, double** %arrayidx15, align 8, !dbg !8, !tbaa !12
//
//   %arrayidx16 = getelementptr inbounds double, double* %5,
//                                                i64 %indvars.iv113, !dbg !8
//   ... some arbitrary IR here ...
//
//   tail call void @sincos(double %3, double* %arrayidx16,
//                                     double* %arrayidx22) #1, !dbg !18
//
//
// The traceback begins with %arrayidx16, which is a gep using an IV from the
// loop being vectorized. All other geps involved with the address computation
// for A[i][j] involve geps that use IVs that are invariant to the loop being
// vectorized. So,
//
// the SCEV of %arrayidx16 = ({0,+,8}<nuw><nsw><%for.body.6> + %162)<nsw>
// The SCEV expression involves an unknown value of %162 for which we must trace
// back to determine loop invariance. The first operand is an add recurrence for
// the innermost loop, which of course, is not loop invariant. However, this is
// ok because this corresponds to the last subscript for which we are
// vectorizing and we can use the stride of this SCEV to determine what type of
// load/store to generate as long as all sub-expressions of %162 are loop
// invariant with respect to the innermost loop.
//
// There is no SCEV for load instructions, so we must trace back using operand 0
// of the load, which should be another gep. In this case, the gep for
// %arrayidx15 = ({0,+,8}<nuw><nsw><%for.cond.4.preheader> + %161)<nsw>. This
// time, the add recurrence part of the SCEV is related to outside the loop we
// are vectorizing (%for.cond.4.preheader), so this is invariant and we must
// then check %161.
//
// %161 is a load done outside of the innermost loop (not shown in the IR above)
// which traces back to %arrayidx14 (a gep). The SCEV for %arrayidx14 =
// {%array2,+,8}<nsw><%for.cond.1.preheader>, which is also innermost loop
// invariant.
//
// Thus, essentially, we have an expression of the form:
//
// base + offset1 + offset2,
//
// where offsets 1 and 2 are constants. So, we can take the stride of the add
// recurrence for base (the incoming SCEV to the getStrideExpr() function below
// to determine what type of load/store to generate. i.e., get the stride from
// SCEV {0,+,8}<nuw><nsw><%for.body.6>, which is 8 bytes. If the innermost loop
// were incremented as k+=2, then the SCEV would look like
// {0,+,16}<nuw><nsw><%for.body.6>.

bool llvm::referenceIsLoopInvariant(const SCEV *Scev, ScalarEvolution *SE,
                                    Loop *OrigLoop)
{
  bool IsLoopInvariant = false;

  if (const SCEVAddRecExpr *AddRecExpr = dyn_cast<SCEVAddRecExpr>(Scev)) {
    if (SE->isLoopInvariant(AddRecExpr, OrigLoop)) {
      IsLoopInvariant = true;
    }
  } else if (const SCEVAddExpr *ScevAddExpr = dyn_cast<SCEVAddExpr>(Scev)) {
    for (unsigned I = 0; I < ScevAddExpr->getNumOperands(); ++I) {
      const SCEV *ScevOp = ScevAddExpr->getOperand(I);
      IsLoopInvariant = referenceIsLoopInvariant(ScevOp, SE, OrigLoop);
    }
  } else if (const SCEVUnknown *Unknown = dyn_cast<SCEVUnknown>(Scev)) {
    // No SCEV for load instructions, so get the SCEV for the gep from which it
    // is loading.
    Value *UnknownVal = Unknown->getValue();
    LoadInst *Load = dyn_cast<LoadInst>(UnknownVal);
    assert(Load && "Expected unknown SCEV to be a load instruction");
    Value *LoadOp = Load->getOperand(0);
    const SCEV *LoadOpScev = SE->getSCEV(LoadOp);
    IsLoopInvariant = referenceIsLoopInvariant(LoadOpScev, SE, OrigLoop);
  }

  return IsLoopInvariant;
}

// This function returns the stride of a memory reference expression. If it is
// determined that all SCEVs analyzed in the trace back are loop invariant, then
// the stride from the initial add recurrence is returned. Otherwise, the stride
// is set to Undef.
Value* llvm::getExprStride(const SCEV *Scev, ScalarEvolution *SE,
                           Loop *OrigLoop)
{
  // UndefValue indicates non constant stride, which will lead to
  // gather/scatter.
  Value *Stride = UndefValue::get(IntegerType::getInt32Ty(SE->getContext()));
  const SCEVAddRecExpr *MemRef;
  bool SubExprIsInvariant = false;

  if (const SCEVAddRecExpr *AddRecExpr = dyn_cast<SCEVAddRecExpr>(Scev)) {
    MemRef = AddRecExpr;
    SubExprIsInvariant = true;
  } else if (const SCEVAddExpr *AddExpr = dyn_cast<SCEVAddExpr>(Scev)) {
    MemRef = dyn_cast<SCEVAddRecExpr>(AddExpr->getOperand(0));
    assert(MemRef && "Stride can only be computed from an add recurrence SCEV");

    // Check to make sure all offsets to the add recurrence are loop invariant.
    // If not, then return non-linear stride.
    SubExprIsInvariant = referenceIsLoopInvariant(AddExpr->getOperand(1), SE,
                                                  OrigLoop);
  }

  if (SubExprIsInvariant) {
    const SCEV *ScevStride = MemRef->getStepRecurrence(*SE);
    const SCEVConstant *ScevConst = dyn_cast<SCEVConstant>(ScevStride);
    Stride = ScevConst->getValue();
  }

  return Stride;
}

// This function marks the CallInst CI with the appropriate stride information
// determined by getExprStride(), which is used later in LLVM IR generation for
// loads/stores. Initial use of this information is used during SVML translation
// for sincos vectorization, but could be applicable to any situation where we
// need to analyze memory references.
void llvm::analyzeCallArgMemoryReferences(CallInst *CI, CallInst *VecCall,
                                          const TargetLibraryInfo *TLI,
                                          ScalarEvolution *SE, Loop *OrigLoop)
{
  for (unsigned I = 0; I < CI->getNumArgOperands(); ++I) {

    Value *CallArg = CI->getArgOperand(I);
    GetElementPtrInst *ArgGep = dyn_cast<GetElementPtrInst>(CallArg);

    if (ArgGep) {

      const SCEV *ArgScev = SE->getSCEV(ArgGep);
      Value *Stride = getExprStride(ArgScev, SE, OrigLoop);
      AttrBuilder AttrList;

      if (!isa<UndefValue>(Stride)) {

        // 2nd and 3rd args to sincos should always be pointers, but assert just
        // in case.
        PointerType *PtrArgType = dyn_cast<PointerType>(CallArg->getType());

        if (PtrArgType) {

          Type *ElemType = PtrArgType->getElementType();

          ConstantInt *StrideConst = dyn_cast<ConstantInt>(Stride);
          if (StrideConst) {

            int64_t ElemTypeSize = ElemType->getScalarSizeInBits() / 8;
            int64_t StrideVal = StrideConst->getSExtValue();

            // Mark the call argument with the stride value in number of
            // elements.
            unsigned ElemStride = StrideVal / ElemTypeSize;
            AttrList.addAttribute("stride",
                                  APInt(32, ElemStride).toString(10, false));
          }
        }
      } else {
        // Undef stride means that we must treat the memory reference as
        // gather/scatter or resort to store scalarization.
        AttrList.addAttribute("stride", "indirect");
      }

      if (AttrList.hasAttributes()) {
        VecCall->setAttributes(
            VecCall->getAttributes().addAttributes(
                VecCall->getContext(), I + 1,
                AttributeSet::get(VecCall->getContext(), I + 1, AttrList)));
      }
    }
  }
}

MapVector<Instruction *, uint64_t>
llvm::computeMinimumValueSizes(ArrayRef<BasicBlock *> Blocks, DemandedBits &DB,
                               const TargetTransformInfo *TTI) {

  // DemandedBits will give us every value's live-out bits. But we want
  // to ensure no extra casts would need to be inserted, so every DAG
  // of connected values must have the same minimum bitwidth.
  EquivalenceClasses<Value *> ECs;
  SmallVector<Value *, 16> Worklist;
  SmallPtrSet<Value *, 4> Roots;
  SmallPtrSet<Value *, 16> Visited;
  DenseMap<Value *, uint64_t> DBits;
  SmallPtrSet<Instruction *, 4> InstructionSet;
  MapVector<Instruction *, uint64_t> MinBWs;

  // Determine the roots. We work bottom-up, from truncs or icmps.
  bool SeenExtFromIllegalType = false;
  for (auto *BB : Blocks)
    for (auto &I : *BB) {
      InstructionSet.insert(&I);

      if (TTI && (isa<ZExtInst>(&I) || isa<SExtInst>(&I)) &&
          !TTI->isTypeLegal(I.getOperand(0)->getType()))
        SeenExtFromIllegalType = true;

      // Only deal with non-vector integers up to 64-bits wide.
      if ((isa<TruncInst>(&I) || isa<ICmpInst>(&I)) &&
          !I.getType()->isVectorTy() &&
          I.getOperand(0)->getType()->getScalarSizeInBits() <= 64) {
        // Don't make work for ourselves. If we know the loaded type is legal,
        // don't add it to the worklist.
        if (TTI && isa<TruncInst>(&I) && TTI->isTypeLegal(I.getType()))
          continue;

        Worklist.push_back(&I);
        Roots.insert(&I);
      }
    }
  // Early exit.
  if (Worklist.empty() || (TTI && !SeenExtFromIllegalType))
    return MinBWs;

  // Now proceed breadth-first, unioning values together.
  while (!Worklist.empty()) {
    Value *Val = Worklist.pop_back_val();
    Value *Leader = ECs.getOrInsertLeaderValue(Val);

    if (Visited.count(Val))
      continue;
    Visited.insert(Val);

    // Non-instructions terminate a chain successfully.
    if (!isa<Instruction>(Val))
      continue;
    Instruction *I = cast<Instruction>(Val);

    // If we encounter a type that is larger than 64 bits, we can't represent
    // it so bail out.
    if (DB.getDemandedBits(I).getBitWidth() > 64)
      return MapVector<Instruction *, uint64_t>();

    uint64_t V = DB.getDemandedBits(I).getZExtValue();
    DBits[Leader] |= V;

    // Casts, loads and instructions outside of our range terminate a chain
    // successfully.
    if (isa<SExtInst>(I) || isa<ZExtInst>(I) || isa<LoadInst>(I) ||
        !InstructionSet.count(I))
      continue;

    // Unsafe casts terminate a chain unsuccessfully. We can't do anything
    // useful with bitcasts, ptrtoints or inttoptrs and it'd be unsafe to
    // transform anything that relies on them.
    if (isa<BitCastInst>(I) || isa<PtrToIntInst>(I) || isa<IntToPtrInst>(I) ||
        !I->getType()->isIntegerTy()) {
      DBits[Leader] |= ~0ULL;
      continue;
    }

    // We don't modify the types of PHIs. Reductions will already have been
    // truncated if possible, and inductions' sizes will have been chosen by
    // indvars.
    if (isa<PHINode>(I))
      continue;

    if (DBits[Leader] == ~0ULL)
      // All bits demanded, no point continuing.
      continue;

    for (Value *O : cast<User>(I)->operands()) {
      ECs.unionSets(Leader, O);
      Worklist.push_back(O);
    }
  }

  // Now we've discovered all values, walk them to see if there are
  // any users we didn't see. If there are, we can't optimize that
  // chain.
  for (auto &I : DBits)
    for (auto *U : I.first->users())
      if (U->getType()->isIntegerTy() && DBits.count(U) == 0)
        DBits[ECs.getOrInsertLeaderValue(I.first)] |= ~0ULL;

  for (auto I = ECs.begin(), E = ECs.end(); I != E; ++I) {
    uint64_t LeaderDemandedBits = 0;
    for (auto MI = ECs.member_begin(I), ME = ECs.member_end(); MI != ME; ++MI)
      LeaderDemandedBits |= DBits[*MI];

    uint64_t MinBW = (sizeof(LeaderDemandedBits) * 8) -
                     llvm::countLeadingZeros(LeaderDemandedBits);
    // Round up to a power of 2
    if (!isPowerOf2_64((uint64_t)MinBW))
      MinBW = NextPowerOf2(MinBW);
    for (auto MI = ECs.member_begin(I), ME = ECs.member_end(); MI != ME; ++MI) {
      if (!isa<Instruction>(*MI))
        continue;
      Type *Ty = (*MI)->getType();
      if (Roots.count(*MI))
        Ty = cast<Instruction>(*MI)->getOperand(0)->getType();
      if (MinBW < Ty->getScalarSizeInBits())
        MinBWs[cast<Instruction>(*MI)] = MinBW;
    }
  }

  return MinBWs;
}

#if INTEL_CUSTOMIZATION
std::vector<Attribute> llvm::getVectorVariantAttributes(Function& F) {
  std::vector<Attribute> RetVal;
  AttributeSet Attributes = F.getAttributes().getFnAttributes();
  AttributeSet::iterator ItA = Attributes.begin(0);
  AttributeSet::iterator EndA = Attributes.end(0);
  for (; ItA != EndA; ++ItA) {
    if (!ItA->isStringAttribute())
      continue;
    StringRef AttributeKind = ItA->getKindAsString();
    if (VectorVariant::isVectorVariant(AttributeKind))
      RetVal.push_back(*ItA);
  }
  return RetVal;
}

Type* llvm::calcCharacteristicType(Function& F, VectorVariant& Variant)
{
  Type* ReturnType = F.getReturnType();
  Type* CharacteristicDataType = NULL;

  if (!ReturnType->isVoidTy())
    CharacteristicDataType = ReturnType;

  if (!CharacteristicDataType) {

    std::vector<VectorKind>& ParmKinds = Variant.getParameters();
    const Function::ArgumentListType& Args = F.getArgumentList();
    Function::ArgumentListType::const_iterator ArgIt = Args.begin();
    Function::ArgumentListType::const_iterator ArgEnd = Args.end();
    std::vector<VectorKind>::iterator VKIt = ParmKinds.begin();

    for (; ArgIt != ArgEnd; ++ArgIt, ++VKIt) {
      if (VKIt->isVector()) {
        CharacteristicDataType = (*ArgIt).getType();
        break;
      }
    }
  }

  // TODO except Clang's ComplexType
  if (!CharacteristicDataType || CharacteristicDataType->isStructTy()) {
    CharacteristicDataType = Type::getInt32Ty(F.getContext());
  }

  // Promote char/short types to int for Xeon Phi.
  CharacteristicDataType =
    VectorVariant::promoteToSupportedType(CharacteristicDataType, Variant);

  if (CharacteristicDataType->isPointerTy()) {
    // For such cases as 'int* foo(int x)', where x is a non-vector type, the
    // characteristic type at this point will be i32*. If we use the DataLayout
    // to query the supported pointer size, then a promotion to i64* is
    // incorrect because the mask element type will mismatch the element type
    // of the characteristic type.
    PointerType *PointerTy = cast<PointerType>(CharacteristicDataType);
    CharacteristicDataType = PointerTy->getElementType();
  }

  return CharacteristicDataType;
}

void llvm::getFunctionsToVectorize(llvm::Module &M,
                                   FunctionVariants& FuncVars) {
  for (auto It = M.begin(), End = M.end(); It != End; ++It) {
    Function& F = *It;
    auto VariantAttributes = getVectorVariantAttributes(F);
    if (VariantAttributes.empty())
      continue;
    FuncVars[&F] = DeclaredVariants();
    DeclaredVariants& DeclaredFuncVariants = FuncVars[&F];
    for (auto Attr : VariantAttributes)
      DeclaredFuncVariants.push_back(Attr.getKindAsString());
  }
}
#endif // INTEL_CUSTOMIZATION
