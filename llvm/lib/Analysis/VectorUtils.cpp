//===----------- VectorUtils.cpp - Vectorizer utility functions -----------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021-2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines vectorizer utilities.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VectorUtils.h"
#include "llvm/ADT/EquivalenceClasses.h"
#include "llvm/Analysis/DemandedBits.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "vectorutils"

using namespace llvm;
using namespace llvm::PatternMatch;

/// Maximum factor for an interleaved memory access.
static cl::opt<unsigned> MaxInterleaveGroupFactor(
    "max-interleave-group-factor", cl::Hidden,
    cl::desc("Maximum factor for an interleaved access group (default = 8)"),
    cl::init(8));

#if INTEL_CUSTOMIZATION
static char encodeISAClass(VFISAKind Isa) {
  switch (Isa) {
  case VFISAKind::SSE:
    return 'b';
  case VFISAKind::AVX:
    return 'c';
  case VFISAKind::AVX2:
    return 'd';
  case VFISAKind::AVX512:
    return 'e';
  case VFISAKind::Unknown:
    return 'x';
  case VFISAKind::AdvancedSIMD:
  case VFISAKind::SVE:
  case VFISAKind::LLVM:
    llvm_unreachable("unsupported kind!");
  }
}

static char encodeMask(bool Mask) {
  return Mask ? 'M' : 'N';
}

static void encodeParam(raw_ostream &OS, const VFParameter &Param) {
  // Encode param kind
  constexpr const char *LUT[] = {
      "v",  // Vector
      "l",  // OMP_Linear
      "R",  // OMP_LinearRef
      "L",  // OMP_LinearVal
      "U",  // OMP_LinearUVal
      "ls", // OMP_LinearPos
      "Ls", // OMP_LinearValPos
      "Rs", // OMP_LinearRefPos
      "Us", // OMP_LinearUValPos
      "u"   // OMP_Uniform
  };
  assert((unsigned)Param.ParamKind < sizeof(LUT) &&
         "unsupported parameter kind!");
  OS << LUT[(unsigned)Param.ParamKind];

  // Encode associated data (for kinds that have it).
  switch (Param.ParamKind) {
    default:
      break;
    case VFParamKind::OMP_Linear:
    case VFParamKind::OMP_LinearRef:
    case VFParamKind::OMP_LinearVal:
    case VFParamKind::OMP_LinearUVal:
      // Encode linear step
      if (Param.LinearStepOrPos == 1);
        // Skip step if unit-strided
      else if (Param.LinearStepOrPos < 0)
        OS << 'n' << -Param.LinearStepOrPos;
      else
        OS << Param.LinearStepOrPos;
      break;
    case VFParamKind::OMP_LinearPos:
    case VFParamKind::OMP_LinearRefPos:
    case VFParamKind::OMP_LinearValPos:
    case VFParamKind::OMP_LinearUValPos:
      // Encode linear pos
      OS << Param.LinearStepOrPos;
      break;
  }

  // Encode alignment if present
  if (Param.Alignment)
    OS << 'a' << Param.Alignment->value();
}

std::string llvm::VFInfo::encodeFromParts(VFISAKind Isa, bool Mask, unsigned VF,
                                          ArrayRef<VFParameter> Parameters,
                                          StringRef ScalarName) {
  std::string VectorName;
  raw_string_ostream OS(VectorName);

  OS << VFInfo::PREFIX << encodeISAClass(Isa) << encodeMask(Mask) << VF;

  const auto *It = Parameters.begin();
  const auto *End = Parameters.end();

  if (Mask)
    --End; // mask parameter is not encoded

  for (; It != End; ++It)
    encodeParam(OS, *It);

  OS << "_" << ScalarName;
  
  return VectorName;
}

/// Describes the caller side argument to callee side parameter matching
/// score for simd functions. Scoring is based on the performance implications
/// of the matching. E.g., uniform pointer arg -> vector parameter would
/// result in gather/scatter, uniform -> vector would result in broadcast,
/// etc.
namespace scores {
// Scalar2VectorScore represents either a uniform or linear match with
// vector.
static constexpr unsigned Scalar2VectorScore = 2;
static constexpr unsigned Uniform2UniformScore = 3;
static constexpr unsigned UniformPtr2UniformPtrScore = 4;
static constexpr unsigned Vector2VectorScore = 4;
static constexpr unsigned Linear2LinearScore = 4;

// Indicate that a match was not found for a particular variant when doing
// caller/callee variant matching.
static constexpr int NoMatch = -1;

int matchParameters(const VFInfo &V1, const VFInfo &V2, int &MaxArg,
                    const Module *M) {

  // 'V1' refers to the variant for the call. Match parameters with 'V2',
  // which represents some available variant.
  ArrayRef<VFParameter> Params = V1.getParameters();
  ArrayRef<VFParameter> OtherParams = V2.getParameters();

  assert(Params.size() == OtherParams.size() &&
         "Number of parameters do not match");

  Function *F = M->getFunction(V1.ScalarName);
  assert(F && "Function not found in module");

  LLVM_DEBUG(dbgs() << "Attempting parameter matching of " << V1.prefix()
                    << " with " << V2.prefix() << "\n");
  int ParamScore = 0;

  std::vector<int> ArgScores;
  unsigned ArgIdx = (F->getName().startswith("__intel_indirect_call")) ? 1 : 0;
  for (unsigned I = 0; I < OtherParams.size(); ++I, ++ArgIdx) {
    // Linear and uniform arguments can always safely be put into vectors, but
    // reduce score in those cases because scalar is optimal.
    int ArgScore;
    if (OtherParams[I].isVector()) {
      if (Params[I].isVector())
        ArgScore = Vector2VectorScore;
      else
        ArgScore = Scalar2VectorScore; // uniform/linear -> vector
      ArgScores.push_back(ArgScore);
      ParamScore += ArgScore;
      continue;
    }

    // linear->linear matches occur when both args are linear and have same
    // stride.
    if (OtherParams[I].isLinear() && Params[I].isLinear() &&
        OtherParams[I].isConstantStrideLinear() &&
        Params[I].isConstantStrideLinear() &&
        OtherParams[I].getStride() == Params[I].getStride()) {
      ArgScore = Linear2LinearScore;
      ArgScores.push_back(ArgScore);
      ParamScore += ArgScore;
      continue;
    }

    if (OtherParams[I].isUniform() && Params[I].isUniform()) {
      // Uniform ptr arguments are more beneficial for performance, so weight
      // them accordingly.
      if (isa<PointerType>(F->getArg(ArgIdx)->getType()))
        ArgScore = UniformPtr2UniformPtrScore;
      else
        ArgScore = Uniform2UniformScore;
      ArgScores.push_back(ArgScore);
      ParamScore += ArgScore;
      continue;
    }

    LLVM_DEBUG(dbgs() << "Arg did not match variant parameter!\n");
    return NoMatch;
  }

  LLVM_DEBUG(dbgs() << "Args matched variant parameters\n");
  // If two args have the same max score, the 1st is selected.
  MaxArg =
      std::max_element(ArgScores.begin(), ArgScores.end()) - ArgScores.begin();
  LLVM_DEBUG(dbgs() << "MaxArg: " << MaxArg << "\n");
  LLVM_DEBUG(dbgs() << "Score: " << ParamScore << "\n");
  return ParamScore;
}
} // namespace scores

int VFInfo::getMatchingScore(const VFInfo &Other, int &MaxArg,
                             const Module *M) const {
  if (getVF() != Other.getVF())
    return scores::NoMatch;
  if (isMasked() != Other.isMasked())
    return scores::NoMatch;
  return scores::matchParameters(*this, Other, MaxArg, M);
}
#endif // INTEL_CUSTOMIZATION

/// Return true if all of the intrinsic's arguments and return type are scalars
/// for the scalar form of the intrinsic, and vectors for the vector form of the
/// intrinsic (except operands that are marked as always being scalar by
/// isVectorIntrinsicWithScalarOpAtArg).
bool llvm::isTriviallyVectorizable(Intrinsic::ID ID) {
  switch (ID) {
  case Intrinsic::abs:   // Begin integer bit-manipulation.
  case Intrinsic::bswap:
  case Intrinsic::bitreverse:
  case Intrinsic::ctpop:
  case Intrinsic::ctlz:
  case Intrinsic::cttz:
  case Intrinsic::fshl:
  case Intrinsic::fshr:
  case Intrinsic::smax:
  case Intrinsic::smin:
  case Intrinsic::umax:
  case Intrinsic::umin:
  case Intrinsic::sadd_sat:
  case Intrinsic::ssub_sat:
  case Intrinsic::uadd_sat:
  case Intrinsic::usub_sat:
  case Intrinsic::smul_fix:
  case Intrinsic::smul_fix_sat:
  case Intrinsic::umul_fix:
  case Intrinsic::umul_fix_sat:
  case Intrinsic::sqrt: // Begin floating-point.
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
  case Intrinsic::minimum:
  case Intrinsic::maximum:
  case Intrinsic::copysign:
  case Intrinsic::floor:
  case Intrinsic::ceil:
  case Intrinsic::trunc:
  case Intrinsic::rint:
  case Intrinsic::nearbyint:
  case Intrinsic::round:
  case Intrinsic::roundeven:
  case Intrinsic::pow:
  case Intrinsic::fma:
  case Intrinsic::fmuladd:
  case Intrinsic::powi:
  case Intrinsic::canonicalize:
  case Intrinsic::fptosi_sat:
  case Intrinsic::fptoui_sat:
    return true;
  default:
    return false;
  }
}

/// Identifies if the vector form of the intrinsic has a scalar operand.
bool llvm::isVectorIntrinsicWithScalarOpAtArg(Intrinsic::ID ID,
                                              unsigned ScalarOpdIdx) {
  switch (ID) {
  case Intrinsic::abs:
  case Intrinsic::ctlz:
  case Intrinsic::cttz:
  case Intrinsic::powi:
    return (ScalarOpdIdx == 1);
  case Intrinsic::smul_fix:
  case Intrinsic::smul_fix_sat:
  case Intrinsic::umul_fix:
  case Intrinsic::umul_fix_sat:
    return (ScalarOpdIdx == 2);
  default:
    return false;
  }
}

bool llvm::isVectorIntrinsicWithOverloadTypeAtArg(Intrinsic::ID ID,
                                                  unsigned OpdIdx) {
  switch (ID) {
  case Intrinsic::fptosi_sat:
  case Intrinsic::fptoui_sat:
    return OpdIdx == 0;
  case Intrinsic::powi:
    return OpdIdx == 1;
  default:
    return false;
  }
}

/// Returns intrinsic ID for call.
/// For the input call instruction it finds mapping intrinsic and returns
/// its ID, in case it does not found it return not_intrinsic.
Intrinsic::ID llvm::getVectorIntrinsicIDForCall(const CallInst *CI,
                                                const TargetLibraryInfo *TLI) {
  Intrinsic::ID ID = getIntrinsicForCallSite(*CI, TLI);
  if (ID == Intrinsic::not_intrinsic)
    return Intrinsic::not_intrinsic;

  if (isTriviallyVectorizable(ID) || ID == Intrinsic::lifetime_start ||
      ID == Intrinsic::lifetime_end || ID == Intrinsic::assume ||
      ID == Intrinsic::experimental_noalias_scope_decl ||
      ID == Intrinsic::sideeffect || ID == Intrinsic::pseudoprobe)
    return ID;
  return Intrinsic::not_intrinsic;
}

/// Find the operand of the GEP that should be checked for consecutive
/// stores. This ignores trailing indices that have no effect on the final
/// pointer.
unsigned llvm::getGEPInductionOperand(const GetElementPtrInst *Gep) {
  const DataLayout &DL = Gep->getModule()->getDataLayout();
  unsigned LastOperand = Gep->getNumOperands() - 1;
  TypeSize GEPAllocSize = DL.getTypeAllocSize(Gep->getResultElementType());

  // Walk backwards and try to peel off zeros.
  while (LastOperand > 1 && match(Gep->getOperand(LastOperand), m_Zero())) {
    // Find the type we're currently indexing into.
    gep_type_iterator GEPTI = gep_type_begin(Gep);
    std::advance(GEPTI, LastOperand - 2);

    // If it's a type with the same allocation size as the result of the GEP we
    // can peel off the zero index.
    if (DL.getTypeAllocSize(GEPTI.getIndexedType()) != GEPAllocSize)
      break;
    --LastOperand;
  }

  return LastOperand;
}

/// If the argument is a GEP, then returns the operand identified by
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

/// If a value has only one user that is a CastInst, return it.
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

/// Get the stride of a pointer access in a loop. Looks for symbolic
/// strides "a[i*stride]". Returns the symbolic stride, or null otherwise.
#if INTEL_CUSTOMIZATION
/// This function was modified to also return constant strides for the purpose
/// of analyzing call arguments (specifically, sincos calls) in order to
/// generate more efficient stores to memory. Previously, this function only
/// returned loop invariant symbolic strides for loop versioning. This expands
/// the functionality of this function to a broader set of applications.
#endif // INTEL_CUSTOMIZATION
Value *llvm::getStrideFromPointer(Value *Ptr, ScalarEvolution *SE, Loop *Lp) {
  auto *PtrTy = dyn_cast<PointerType>(Ptr->getType());
  if (!PtrTy || PtrTy->isAggregateType())
    return nullptr;

  // Try to remove a gep instruction to make the pointer (actually index at this
  // point) easier analyzable. If OrigPtr is equal to Ptr we are analyzing the
  // pointer, otherwise, we are analyzing the index.
  Value *OrigPtr = Ptr;

  // The size of the pointer access.
  int64_t PtrAccessSize = 1;

  Ptr = stripGetElementPtr(Ptr, SE, Lp);
  const SCEV *V = SE->getSCEV(Ptr);

  if (Ptr != OrigPtr)
    // Strip off casts.
    while (const SCEVIntegralCastExpr *C = dyn_cast<SCEVIntegralCastExpr>(V))
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
  if (const SCEVIntegralCastExpr *C = dyn_cast<SCEVIntegralCastExpr>(V)) {
    StripedOffRecurrenceCast = C->getType();
    V = C->getOperand();
  }

#if INTEL_CUSTOMIZATION
  // Look for constant stride.
  const SCEVConstant *C = dyn_cast<SCEVConstant>(V);
  if (C) {
    return C->getValue();
  }
#endif // INTEL_CUSTOMIZATION

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

/// Given a vector and an element number, see if the scalar value is
/// already around as a register, for example if it were inserted then extracted
/// from the vector.
Value *llvm::findScalarElement(Value *V, unsigned EltNo) {
  assert(V->getType()->isVectorTy() && "Not looking at a vector?");
  VectorType *VTy = cast<VectorType>(V->getType());
  // For fixed-length vector, return undef for out of range access.
  if (auto *FVTy = dyn_cast<FixedVectorType>(VTy)) {
    unsigned Width = FVTy->getNumElements();
    if (EltNo >= Width)
      return UndefValue::get(FVTy->getElementType());
  }

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

    // Guard against infinite loop on malformed, unreachable IR.
    if (III == III->getOperand(0))
      return nullptr;

    // Otherwise, the insertelement doesn't modify the value, recurse on its
    // vector input.
    return findScalarElement(III->getOperand(0), EltNo);
  }

  ShuffleVectorInst *SVI = dyn_cast<ShuffleVectorInst>(V);
  // Restrict the following transformation to fixed-length vector.
  if (SVI && isa<FixedVectorType>(SVI->getType())) {
    unsigned LHSWidth =
        cast<FixedVectorType>(SVI->getOperand(0)->getType())->getNumElements();
    int InEl = SVI->getMaskValue(EltNo);
    if (InEl < 0)
      return UndefValue::get(VTy->getElementType());
    if (InEl < (int)LHSWidth)
      return findScalarElement(SVI->getOperand(0), InEl);
    return findScalarElement(SVI->getOperand(1), InEl - LHSWidth);
  }

  // Extract a value from a vector add operation with a constant zero.
  // TODO: Use getBinOpIdentity() to generalize this.
  Value *Val; Constant *C;
  if (match(V, m_Add(m_Value(Val), m_Constant(C))))
    if (Constant *Elt = C->getAggregateElement(EltNo))
      if (Elt->isNullValue())
        return findScalarElement(Val, EltNo);

  // If the vector is a splat then we can trivially find the scalar element.
  if (isa<ScalableVectorType>(VTy))
    if (Value *Splat = getSplatValue(V))
      if (EltNo < VTy->getElementCount().getKnownMinValue())
        return Splat;

  // Otherwise, we don't know.
  return nullptr;
}

int llvm::getSplatIndex(ArrayRef<int> Mask) {
  int SplatIndex = -1;
  for (int M : Mask) {
    // Ignore invalid (undefined) mask elements.
    if (M < 0)
      continue;

    // There can be only 1 non-negative mask element value if this is a splat.
    if (SplatIndex != -1 && SplatIndex != M)
      return -1;

    // Initialize the splat index to the 1st non-negative mask element.
    SplatIndex = M;
  }
  assert((SplatIndex == -1 || SplatIndex >= 0) && "Negative index?");
  return SplatIndex;
}

/// Get splat value if the input is a splat vector or return nullptr.
/// This function is not fully general. It checks only 2 cases:
/// the input value is (1) a splat constant vector or (2) a sequence
/// of instructions that broadcasts a scalar at element 0.
Value *llvm::getSplatValue(const Value *V) {
  if (isa<VectorType>(V->getType()))
    if (auto *C = dyn_cast<Constant>(V))
      return C->getSplatValue();

  // shuf (inselt ?, Splat, 0), ?, <0, undef, 0, ...>
  Value *Splat;
  if (match(V,
            m_Shuffle(m_InsertElt(m_Value(), m_Value(Splat), m_ZeroInt()),
                      m_Value(), m_ZeroMask())))
    return Splat;

  return nullptr;
}

bool llvm::isSplatValue(const Value *V, int Index, unsigned Depth) {
  assert(Depth <= MaxAnalysisRecursionDepth && "Limit Search Depth");

  if (isa<VectorType>(V->getType())) {
    if (isa<UndefValue>(V))
      return true;
    // FIXME: We can allow undefs, but if Index was specified, we may want to
    //        check that the constant is defined at that index.
    if (auto *C = dyn_cast<Constant>(V))
      return C->getSplatValue() != nullptr;
  }

  if (auto *Shuf = dyn_cast<ShuffleVectorInst>(V)) {
    // FIXME: We can safely allow undefs here. If Index was specified, we will
    //        check that the mask elt is defined at the required index.
    if (!is_splat(Shuf->getShuffleMask()))
      return false;

    // Match any index.
    if (Index == -1)
      return true;

    // Match a specific element. The mask should be defined at and match the
    // specified index.
    return Shuf->getMaskValue(Index) == Index;
  }

  // The remaining tests are all recursive, so bail out if we hit the limit.
  if (Depth++ == MaxAnalysisRecursionDepth)
    return false;

  // If both operands of a binop are splats, the result is a splat.
  Value *X, *Y, *Z;
  if (match(V, m_BinOp(m_Value(X), m_Value(Y))))
    return isSplatValue(X, Index, Depth) && isSplatValue(Y, Index, Depth);

  // If all operands of a select are splats, the result is a splat.
  if (match(V, m_Select(m_Value(X), m_Value(Y), m_Value(Z))))
    return isSplatValue(X, Index, Depth) && isSplatValue(Y, Index, Depth) &&
           isSplatValue(Z, Index, Depth);

  // TODO: Add support for unary ops (fneg), casts, intrinsics (overflow ops).

  return false;
}

void llvm::narrowShuffleMaskElts(int Scale, ArrayRef<int> Mask,
                                 SmallVectorImpl<int> &ScaledMask) {
  assert(Scale > 0 && "Unexpected scaling factor");

  // Fast-path: if no scaling, then it is just a copy.
  if (Scale == 1) {
    ScaledMask.assign(Mask.begin(), Mask.end());
    return;
  }

  ScaledMask.clear();
  for (int MaskElt : Mask) {
    if (MaskElt >= 0) {
      assert(((uint64_t)Scale * MaskElt + (Scale - 1)) <= INT32_MAX &&
             "Overflowed 32-bits");
    }
    for (int SliceElt = 0; SliceElt != Scale; ++SliceElt)
      ScaledMask.push_back(MaskElt < 0 ? MaskElt : Scale * MaskElt + SliceElt);
  }
}

bool llvm::widenShuffleMaskElts(int Scale, ArrayRef<int> Mask,
                                SmallVectorImpl<int> &ScaledMask) {
  assert(Scale > 0 && "Unexpected scaling factor");

  // Fast-path: if no scaling, then it is just a copy.
  if (Scale == 1) {
    ScaledMask.assign(Mask.begin(), Mask.end());
    return true;
  }

  // We must map the original elements down evenly to a type with less elements.
  int NumElts = Mask.size();
  if (NumElts % Scale != 0)
    return false;

  ScaledMask.clear();
  ScaledMask.reserve(NumElts / Scale);

  // Step through the input mask by splitting into Scale-sized slices.
  do {
    ArrayRef<int> MaskSlice = Mask.take_front(Scale);
    assert((int)MaskSlice.size() == Scale && "Expected Scale-sized slice.");

    // The first element of the slice determines how we evaluate this slice.
    int SliceFront = MaskSlice.front();
    if (SliceFront < 0) {
      // Negative values (undef or other "sentinel" values) must be equal across
      // the entire slice.
      if (!is_splat(MaskSlice))
        return false;
      ScaledMask.push_back(SliceFront);
    } else {
      // A positive mask element must be cleanly divisible.
      if (SliceFront % Scale != 0)
        return false;
      // Elements of the slice must be consecutive.
      for (int i = 1; i < Scale; ++i)
        if (MaskSlice[i] != SliceFront + i)
          return false;
      ScaledMask.push_back(SliceFront / Scale);
    }
    Mask = Mask.drop_front(Scale);
  } while (!Mask.empty());

  assert((int)ScaledMask.size() * Scale == NumElts && "Unexpected scaled mask");

  // All elements of the original mask can be scaled down to map to the elements
  // of a mask with wider elements.
  return true;
}

void llvm::processShuffleMasks(
    ArrayRef<int> Mask, unsigned NumOfSrcRegs, unsigned NumOfDestRegs,
    unsigned NumOfUsedRegs, function_ref<void()> NoInputAction,
    function_ref<void(ArrayRef<int>, unsigned, unsigned)> SingleInputAction,
    function_ref<void(ArrayRef<int>, unsigned, unsigned)> ManyInputsAction) {
  SmallVector<SmallVector<SmallVector<int>>> Res(NumOfDestRegs);
  // Try to perform better estimation of the permutation.
  // 1. Split the source/destination vectors into real registers.
  // 2. Do the mask analysis to identify which real registers are
  // permuted.
  int Sz = Mask.size();
  unsigned SzDest = Sz / NumOfDestRegs;
  unsigned SzSrc = Sz / NumOfSrcRegs;
  for (unsigned I = 0; I < NumOfDestRegs; ++I) {
    auto &RegMasks = Res[I];
    RegMasks.assign(NumOfSrcRegs, {});
    // Check that the values in dest registers are in the one src
    // register.
    for (unsigned K = 0; K < SzDest; ++K) {
      int Idx = I * SzDest + K;
      if (Idx == Sz)
        break;
      if (Mask[Idx] >= Sz || Mask[Idx] == UndefMaskElem)
        continue;
      int SrcRegIdx = Mask[Idx] / SzSrc;
      // Add a cost of PermuteTwoSrc for each new source register permute,
      // if we have more than one source registers.
      if (RegMasks[SrcRegIdx].empty())
        RegMasks[SrcRegIdx].assign(SzDest, UndefMaskElem);
      RegMasks[SrcRegIdx][K] = Mask[Idx] % SzSrc;
    }
  }
  // Process split mask.
  for (unsigned I = 0; I < NumOfUsedRegs; ++I) {
    auto &Dest = Res[I];
    int NumSrcRegs =
        count_if(Dest, [](ArrayRef<int> Mask) { return !Mask.empty(); });
    switch (NumSrcRegs) {
    case 0:
      // No input vectors were used!
      NoInputAction();
      break;
    case 1: {
      // Find the only mask with at least single undef mask elem.
      auto *It =
          find_if(Dest, [](ArrayRef<int> Mask) { return !Mask.empty(); });
      unsigned SrcReg = std::distance(Dest.begin(), It);
      SingleInputAction(*It, SrcReg, I);
      break;
    }
    default: {
      // The first mask is a permutation of a single register. Since we have >2
      // input registers to shuffle, we merge the masks for 2 first registers
      // and generate a shuffle of 2 registers rather than the reordering of the
      // first register and then shuffle with the second register. Next,
      // generate the shuffles of the resulting register + the remaining
      // registers from the list.
      auto &&CombineMasks = [](MutableArrayRef<int> FirstMask,
                               ArrayRef<int> SecondMask) {
        for (int Idx = 0, VF = FirstMask.size(); Idx < VF; ++Idx) {
          if (SecondMask[Idx] != UndefMaskElem) {
            assert(FirstMask[Idx] == UndefMaskElem &&
                   "Expected undefined mask element.");
            FirstMask[Idx] = SecondMask[Idx] + VF;
          }
        }
      };
      auto &&NormalizeMask = [](MutableArrayRef<int> Mask) {
        for (int Idx = 0, VF = Mask.size(); Idx < VF; ++Idx) {
          if (Mask[Idx] != UndefMaskElem)
            Mask[Idx] = Idx;
        }
      };
      int SecondIdx;
      do {
        int FirstIdx = -1;
        SecondIdx = -1;
        MutableArrayRef<int> FirstMask, SecondMask;
        for (unsigned I = 0; I < NumOfDestRegs; ++I) {
          SmallVectorImpl<int> &RegMask = Dest[I];
          if (RegMask.empty())
            continue;

          if (FirstIdx == SecondIdx) {
            FirstIdx = I;
            FirstMask = RegMask;
            continue;
          }
          SecondIdx = I;
          SecondMask = RegMask;
          CombineMasks(FirstMask, SecondMask);
          ManyInputsAction(FirstMask, FirstIdx, SecondIdx);
          NormalizeMask(FirstMask);
          RegMask.clear();
          SecondMask = FirstMask;
          SecondIdx = FirstIdx;
        }
        if (FirstIdx != SecondIdx && SecondIdx >= 0) {
          CombineMasks(SecondMask, FirstMask);
          ManyInputsAction(SecondMask, SecondIdx, FirstIdx);
          Dest[FirstIdx].clear();
          NormalizeMask(SecondMask);
        }
      } while (SecondIdx >= 0);
      break;
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

    if (!Visited.insert(Val).second)
      continue;

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
    DBits[I] = V;

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
    for (Value *M : llvm::make_range(ECs.member_begin(I), ECs.member_end()))
      LeaderDemandedBits |= DBits[M];

    uint64_t MinBW = (sizeof(LeaderDemandedBits) * 8) -
                     llvm::countLeadingZeros(LeaderDemandedBits);
    // Round up to a power of 2
    if (!isPowerOf2_64((uint64_t)MinBW))
      MinBW = NextPowerOf2(MinBW);

    // We don't modify the types of PHIs. Reductions will already have been
    // truncated if possible, and inductions' sizes will have been chosen by
    // indvars.
    // If we are required to shrink a PHI, abandon this entire equivalence class.
    bool Abort = false;
    for (Value *M : llvm::make_range(ECs.member_begin(I), ECs.member_end()))
      if (isa<PHINode>(M) && MinBW < M->getType()->getScalarSizeInBits()) {
        Abort = true;
        break;
      }
    if (Abort)
      continue;

    for (Value *M : llvm::make_range(ECs.member_begin(I), ECs.member_end())) {
      if (!isa<Instruction>(M))
        continue;
      Type *Ty = M->getType();
      if (Roots.count(M))
        Ty = cast<Instruction>(M)->getOperand(0)->getType();
      if (MinBW < Ty->getScalarSizeInBits())
        MinBWs[cast<Instruction>(M)] = MinBW;
    }
  }

  return MinBWs;
}

#if INTEL_CUSTOMIZATION
// This function marks the CallInst VecCall with the appropriate stride
// information determined by getStrideFromPointer(), which is used later in
// LLVM IR generation for loads/stores. Initial use of this information is
// used during SVML translation for sincos vectorization, but could be
// applicable to any situation where we need to analyze memory references.
void llvm::analyzeCallArgMemoryReferences(CallInst *CI, CallInst *VecCall,
                                          const TargetLibraryInfo *TLI,
                                          ScalarEvolution *SE, Loop *OrigLoop)
{
  for (unsigned I = 0; I < CI->arg_size(); ++I) {

    Value *CallArg = CI->getArgOperand(I);
    GetElementPtrInst *ArgGep = dyn_cast<GetElementPtrInst>(CallArg);

    if (ArgGep) {

      Value *Stride = getStrideFromPointer(CallArg, SE, OrigLoop);
      AttrBuilder AttrList(CI->getContext());

      if (Stride) {
        // 2nd and 3rd args to sincos should always be pointers, but assert just
        // in case.
        PointerType *PtrArgType = dyn_cast<PointerType>(CallArg->getType());

        if (PtrArgType) {

          ConstantInt *StrideConst = dyn_cast<ConstantInt>(Stride);
          if (StrideConst) {

            int64_t StrideVal = StrideConst->getSExtValue();

            // Mark the call argument with the stride value in number of
            // elements.
            AttrList.addAttribute("stride",
                                  toString(APInt(32, StrideVal), 10, false));
          }
        }
      } else {
        // Undef stride means that we must treat the memory reference as
        // gather/scatter or resort to store scalarization.
        AttrList.addAttribute("stride", "indirect");
      }

      if (AttrList.hasAttributes()) {
        VecCall->setAttributes(
            VecCall->getAttributes().addAttributesAtIndex(
                VecCall->getContext(), I + 1, AttrList));
      }
    }
  }
}

std::vector<Attribute> llvm::getVectorVariantAttributes(Function &F) {
  auto Variants = llvm::make_filter_range(
      F.getAttributes().getFnAttrs(), [](const Attribute &A) {
        return A.isStringAttribute() && VFInfo::isVectorVariant(A.getKindAsString());
      });
  return {Variants.begin(), Variants.end()};
}

Type *llvm::calcCharacteristicType(Function &F, const VFInfo &Variant) {
  return calcCharacteristicType(F.getReturnType(), F.args(), Variant,
                                F.getParent()->getDataLayout());
}

void llvm::createVectorMaskArg(IRBuilder<> &Builder, Type *CharacteristicType,
                               const VFInfo *VecVariant,
                               SmallVectorImpl<Value *> &VecArgs,
                               SmallVectorImpl<Type *> &VecArgTys,
                               unsigned VF, Value *MaskToUse) {

  // Add the mask parameter for masked simd functions.
  // Mask should already be vectorized as i1 type.
  VectorType *MaskTy = cast<VectorType>(MaskToUse->getType());
  assert(MaskTy->getElementType()->isIntegerTy(1) &&
         "Mask parameter is not vector of i1");

  // Promote the i1 to an integer type that has the same size as the
  // characteristic type.
  Type *ScalarToType = IntegerType::get(
      MaskTy->getContext(), CharacteristicType->getPrimitiveSizeInBits());
  VectorType *VecToType = FixedVectorType::get(ScalarToType, VF);
  Value *MaskExt = Builder.CreateSExt(MaskToUse, VecToType, "maskext");

  // Bitcast if the promoted type is not the same as the characteristic
  // type.
  if (ScalarToType != CharacteristicType) {
    Type *MaskCastTy = FixedVectorType::get(CharacteristicType, VF);
    Value *MaskCast = Builder.CreateBitCast(MaskExt, MaskCastTy, "maskcast");
    VecArgs.push_back(MaskCast);
    VecArgTys.push_back(MaskCastTy);
  } else {
    VecArgs.push_back(MaskExt);
    VecArgTys.push_back(VecToType);
  }
}

void llvm::getFunctionsToVectorize(
  llvm::Module &M, MapVector<Function*, std::vector<StringRef> > &FuncVars) {

  // FuncVars will contain a 1-many mapping between the original scalar
  // function and the vector variant encoding strings (represented as
  // attributes). The encodings correspond to functions that will be created by
  // the caller of this function as vector versions of the original function.
  // For example, if foo() is a function marked as a simd function, it will have
  // several vector variant encodings like: "_ZGVbM4_foo", "_ZGVbN4_foo",
  // "_ZGVcM8_foo", "_ZGVcN8_foo", "_ZGVdM8_foo", "_ZGVdN8_foo", "_ZGVeM16_foo",
  // "_ZGVeN16_foo". The caller of this function will then clone foo() and name
  // the clones using the above name manglings. The variant encodings correspond
  // to differences in masked/non-masked execution, vector length, and target
  // vector register size, etc. For more details, please refer to the following
  // reference for details on the vector function encodings.
  // https://www.cilkplus.org/sites/default/files/open_specifications/
  // Intel-ABI-Vector-Function-2012-v0.9.5.pdf

  for (auto It = M.begin(), End = M.end(); It != End; ++It) {
    Function &F = *It;
    if (F.hasFnAttribute("vector-variants") && !F.isDeclaration()) {
      Attribute Attr = F.getFnAttribute("vector-variants");
      StringRef VariantsStr = Attr.getValueAsString();
      SmallVector<StringRef, 8> Variants;
      VariantsStr.split(Variants, ',');
      for (unsigned i = 0; i < Variants.size(); i++) {
        FuncVars[&F].push_back(Variants[i]);
      }
    }
  }
}

bool llvm::isOpenCLSinCos(StringRef FcnName) {
  return (FcnName == "_Z6sincosfPf");
}

bool llvm::isOpenCLReadChannel(StringRef FnName) {
  return (FnName == "__read_pipe_2_bl_fpga");
}

bool llvm::isOpenCLWriteChannel(StringRef FnName) {
  return (FnName == "__write_pipe_2_bl_fpga");
}

bool llvm::isOpenCLReadChannelDest(StringRef FnName, unsigned i) {
  return (isOpenCLReadChannel(FnName) && i == 1);
}

bool llvm::isOpenCLWriteChannelSrc(StringRef FnName, unsigned i) {
  return (isOpenCLWriteChannel(FnName) && i == 1);
}

AllocaInst* llvm::getOpenCLReadWriteChannelAlloc(const CallInst *Call) {

  AddrSpaceCastInst *Arg = dyn_cast<AddrSpaceCastInst>(Call->getArgOperand(1));

  assert(Arg && "Expected addrspacecast in traceback of __read_pipe argument");

  BitCastInst *ArgCast = dyn_cast<BitCastInst>(Arg->getOperand(0));
  AllocaInst *ReadDst = nullptr;

  if (!ArgCast) {
    ReadDst = dyn_cast<AllocaInst>(Arg->getOperand(0));
  } else {
    ReadDst = dyn_cast<AllocaInst>(ArgCast->getOperand(0));
  }

  assert(ReadDst && "Expected alloca in traceback of __read_pipe argument");
  return ReadDst;
}

std::string llvm::typeToString(Type *Ty) {
  if (Ty->isFloatTy())
    // need to return "f32" instead of "float"
    return "f32";
  if (Ty->isDoubleTy())
    // need to return "f64" instead of "double"
    return "f64";
  if (Ty->isIntegerTy() && Ty->getPrimitiveSizeInBits() >= 8 &&
      Ty->getPrimitiveSizeInBits() <= 64) {
    // return i8, i16, i32, i64
    std::string typeStr;
    raw_string_ostream OS(typeStr);
    Ty->print(OS);
    return OS.str();
  }
  llvm_unreachable("Unsupported type for converting type to string");
}

bool llvm::isSVMLFunction(const TargetLibraryInfo *TLI, StringRef FnName,
                          StringRef VFnName) {
  return TLI->isFunctionVectorizable(FnName) && VFnName.startswith("__svml_");
}

unsigned llvm::getPumpFactor(StringRef FnName, bool IsMasked, unsigned VF,
                             const TargetLibraryInfo *TLI) {
  // Call can already be vectorized for current VF, pumping not needed.
  if (TLI->isFunctionVectorizable(FnName, ElementCount::getFixed(VF), IsMasked))
    return 1;

  // TODO: Pumping is supported only for simple SVML functions.
  if (isOpenCLSinCos(FnName))
    return 1;

  // Check if function can be vectorized for a dummy low VF value. This is
  // purely to identify and filter out non-SVML functions.
  // TODO: This filtering is temporary until we start supporting pumping feature
  // for SIMD functions with vector-variants.
  StringRef VecFnName =
      TLI->getVectorizedFunction(FnName, ElementCount::getFixed(4) /*dummy VF*/,
                                 IsMasked);
  if (VecFnName.empty() || !isSVMLFunction(TLI, FnName, VecFnName))
    return 1;

  // Pumping can be done if function can be vectorized for any LowerVF starting
  // from VF/2 -> 2.
  assert(isPowerOf2_32(VF) &&
         "Pumping analysis is not supported for non-power of two VF.");
  unsigned LowerVF;
  for (LowerVF = VF / 2; LowerVF > 1; LowerVF /= 2) {
    if (TLI->isFunctionVectorizable(FnName, ElementCount::getFixed(LowerVF),
                                    IsMasked))
      return VF / LowerVF;
  }

  return 1;
}

template <typename CastInstTy> Value *llvm::getPtrThruCast(Value *Ptr) {
  while (isa<CastInstTy>(Ptr)) {
    CastInstTy *CastPtr = cast<CastInstTy>(Ptr);
    Type *DestTy = CastPtr->getType();
    Type *SrcTy = CastPtr->getSrcTy();
    if (!isa<PointerType>(DestTy) || !isa<PointerType>(SrcTy))
      break;
    Ptr = CastPtr->getOperand(0);
  }
  return Ptr;
}

template Value *llvm::getPtrThruCast<BitCastInst>(Value *Ptr);
template Value *llvm::getPtrThruCast<AddrSpaceCastInst>(Value *Ptr);

void llvm::setRequiredAttributes(AttributeList Attrs, CallInst *VecCall) {
  VecCall->setAttributes(Attrs.removeAttribute(
      VecCall->getContext(), AttributeList::FunctionIndex, "vector-variants"));
}

void llvm::setRequiredAttributes(AttributeList Attrs, CallInst *VecCall,
                                 ArrayRef<AttributeSet> ArgAttrs) {
  AttributeSet FnAttrs = Attrs.getFnAttrs().removeAttribute(
      VecCall->getContext(), "vector-variants");

  VecCall->setAttributes(AttributeList::get(VecCall->getContext(), FnAttrs,
                                            Attrs.getRetAttrs(), ArgAttrs));
}

Function *llvm::getOrInsertVectorVariantFunction(
    Function *OrigF, unsigned VL,
    ArrayRef<Type *> ArgTys,
    const VFInfo *VecVariant,
    bool Masked) {
  // OrigF is the original scalar function being called.
  assert(OrigF && "Function not found for call instruction");
  assert(VecVariant && "Expect VectorVariant to be present");

  Module *M = OrigF->getParent();
  Type *RetTy = OrigF->getReturnType();
  Type *VecRetTy = RetTy;
  if (!RetTy->isVoidTy()) {
    // GEPs into vectors of i1 do not make sense, so promote it to i8
    // similar to its later processing in CodeGen.
    if (RetTy->isIntegerTy(1))
      RetTy = Type::getInt8Ty(RetTy->getContext());
    VecRetTy = getWidenedType(RetTy, VL);
  }

  std::string VFnName = VecVariant->VectorName;
  LLVM_DEBUG(dbgs() << "Getting or inserting " << VFnName << '\n');
  Function *VectorF = M->getFunction(VFnName);
  if (!VectorF) {
    FunctionType *FTy = FunctionType::get(VecRetTy, ArgTys, false);
    VectorF = Function::Create(FTy, OrigF->getLinkage(), VFnName, M);
    VectorF->copyAttributesFrom(OrigF);
    VectorF->setVisibility(OrigF->getVisibility());
  }

  return VectorF;
}


Function *llvm::getOrInsertVectorLibFunction(
    Function *OrigF, unsigned VL,
    ArrayRef<Type *> ArgTys,
    TargetLibraryInfo *TLI,
    Intrinsic::ID ID,
    bool Masked, const CallInst *Call) {

  // OrigF is the original scalar function being called. Widen the scalar
  // call to a vector call if it is known to be vectorizable as SVML or
  // an intrinsic.
  assert(OrigF && "Function not found for call instruction");
  StringRef FnName = OrigF->getName();
  if (TLI && !TLI->isFunctionVectorizable(
        FnName, ElementCount::getFixed(VL)) &&
      !ID && !isOpenCLReadChannel(FnName) && !isOpenCLWriteChannel(FnName))
    return nullptr;

  Module *M = OrigF->getParent();
  Type *RetTy = OrigF->getReturnType();
  Type *VecRetTy = RetTy;
  if (!RetTy->isVoidTy()) {
    VecRetTy = getWidenedType(RetTy, VL);
  }

  if (ID) {
    // Generate a vector intrinsic.
    assert(!RetTy->isVoidTy() && "Expected non-void function");
    SmallVector<Type *, 1> TysForDecl;
    TysForDecl.push_back(VecRetTy);
    for (auto &I : enumerate(ArgTys))
      if (isVectorIntrinsicWithOverloadTypeAtArg(ID, I.index()))
        TysForDecl.push_back(I.value());
    return Intrinsic::getDeclaration(M, ID, TysForDecl);
  }

  if (isOpenCLReadChannel(FnName) || isOpenCLWriteChannel(FnName)) {
    // TODO: Modify OpenCL read/write channel code to be CallInst independent.
    // Check JR https://jira.devtools.intel.com/browse/CORC-4838
    assert(Call && "VPVALCG: OpenCL read/write channels not uplifted to be "
                   "call independent.");
    AllocaInst *Alloca = getOpenCLReadWriteChannelAlloc(Call);
    std::string VLStr = toString(APInt(32, VL), 10, false);
    std::string TyStr = typeToString(Alloca->getAllocatedType());
    std::string VFnName = FnName.str() + "_v" + VLStr + TyStr;

    if (isOpenCLReadChannel(FnName)) {
      // The return type of the vector read channel call is a vector of the
      // pointer element type of the read destination pointer alloca. The
      // function call below traces back through bitcast instructions to
      // find the alloca.
      VecRetTy = FixedVectorType::get(Alloca->getAllocatedType(), VL);
    }
    if (isOpenCLWriteChannel(FnName)) {
      VecRetTy = RetTy;
    }

    Function *VectorF = M->getFunction(VFnName);
    if (!VectorF) {
      FunctionType *FTy = FunctionType::get(VecRetTy, ArgTys, false);
      VectorF = Function::Create(FTy, OrigF->getLinkage(), VFnName, M);
    }
    // Note: The function signature is different for the vector version of
    // these functions. E.g., in the case of __read_pipe the 2nd parameter
    // is dropped, and for __write_pipe the 2nd parameter becomes a vector
    // of instead of a pointer. Thus, the attributes cannot blindly be
    // copied because some attributes for the parameters on the original
    // scalar call will be incompatible with the vector parameter types.
    // Or, in the case of __read_pipe, the attribute for the 2nd parameter
    // will still be copied to the vector call site and will result in an
    // assert in the verifier because there is no longer a 2nd parameter.
    // TODO: determine if attributes really need to be copied for those
    // parameters that still match the scalar version.
    return VectorF;
  }

  assert(TLI && "TLI is expected to be initialized.");
  // Generate a vector library call.
  StringRef VFnName =
      TLI->getVectorizedFunction(FnName, ElementCount::getFixed(VL),
                                 Masked);
  Function *VectorF = M->getFunction(VFnName);
  if (!VectorF) {
    // isFunctionVectorizable() returned true, so it is guaranteed that
    // the svml function exists and the call is legal. Generate a declaration
    // for it if one does not already exist.

    // SVML sincos functions uses struct to return results.
    bool IsSinCos = VFnName.startswith("__svml_sincos");
    if (IsSinCos) {
      Type *ElementType = getWidenedType(OrigF->getArg(0)->getType(), VL);
      VecRetTy = StructType::get(ElementType, ElementType);
    }
    FunctionType *FTy = FunctionType::get(VecRetTy, ArgTys, false);
    VectorF = Function::Create(FTy, OrigF->getLinkage(), VFnName, M);

    if (IsSinCos) {
      LLVMContext &C = VectorF->getContext();
      AttributeSet NoUndefAttr =
          AttributeSet::get(C, {Attribute::get(C, Attribute::NoUndef)});
      AttributeList Attrs = AttributeList::get(
          C, VectorF->getAttributes().getFnAttrs(), NoUndefAttr, {NoUndefAttr});
      VectorF->setAttributes(Attrs);
    } else
      VectorF->copyAttributesFrom(OrigF);
  }
  return VectorF;
}

Value *llvm::joinVectors(ArrayRef<Value *> VectorsToJoin, IRBuilderBase &Builder,
                         Twine Name) {
  SmallVector<Value *, 8> VParts(VectorsToJoin.begin(), VectorsToJoin.end());
  unsigned VL = VParts.size();
  while (VL >= 2) {
    for (unsigned i = 0, j = 0; i < VL; i += 2, ++j) {
      unsigned NumElts =
          cast<FixedVectorType>(VParts[i]->getType())->getNumElements();
      SmallVector<int, 8> ShuffleMask(NumElts * 2);
      for (unsigned MaskInd = 0; MaskInd < NumElts * 2; ++MaskInd)
        ShuffleMask[MaskInd] = MaskInd;
      VParts[j] =
          Builder.CreateShuffleVector(VParts[i], VParts[i + 1], ShuffleMask);
    }
    VL /= 2;
  }
  VParts[0]->setName(Name);
  return VParts[0];
}

Value *llvm::extendVector(Value *OrigVal, unsigned TargetLength,
                          IRBuilderBase &Builder, const Twine &Name) {
  Type *OrigTy = OrigVal->getType();
  unsigned VectorElts = cast<FixedVectorType>(OrigTy)->getNumElements();
  assert(TargetLength >= VectorElts &&
         "TargetLength should be greater than or equal to VectorElts");
  if (VectorElts == TargetLength)
    return OrigVal;
  auto ShufMask = createSequentialMask(
      0, VectorElts, TargetLength - VectorElts /*No. of undef's*/);
  return Builder.CreateShuffleVector(OrigVal, UndefValue::get(OrigTy), ShufMask,
                                     "extended." + Name);
}

Value *llvm::replicateVectorElts(Value *OrigVal, unsigned OriginalVL,
                                 IRBuilderBase &Builder, const Twine &Name) {
  if (OriginalVL == 1)
    return OrigVal;
  auto ShuffleMask = createReplicatedMask(
      OriginalVL, cast<FixedVectorType>(OrigVal->getType())->getNumElements());
  return Builder.CreateShuffleVector(OrigVal,
                                     UndefValue::get(OrigVal->getType()),
                                     ShuffleMask, Name + OrigVal->getName());
}

Value *llvm::replicateVector(Value *OrigVal, unsigned OriginalVL,
                             IRBuilderBase &Builder, const Twine &Name) {
  if (OriginalVL == 1)
    return OrigVal;
  unsigned NumElts =
      cast<FixedVectorType>(OrigVal->getType())->getNumElements();
  SmallVector<int, 8> ShuffleMask;
  for (unsigned j = 0; j < OriginalVL; j++)
    for (unsigned i = 0; i < NumElts; ++i)
      ShuffleMask.push_back((signed)i);
  return Builder.CreateShuffleVector(OrigVal,
                                     UndefValue::get(OrigVal->getType()),
                                     ShuffleMask, Name + OrigVal->getName());
}

Value *llvm::createVectorSplat(Value *V, unsigned VF, IRBuilderBase &Builder,
                               const Twine &Name) {
  if (V->getType()->isVectorTy())
    return replicateVectorElts(V, VF, Builder, Name);
  return Builder.CreateVectorSplat(VF, V, V->getName() + Name);
}

Value *llvm::generateExtractSubVector(Value *V, unsigned Part,
                                      unsigned NumParts, IRBuilderBase &Builder,
                                      const Twine &Name) {
  // Example:
  // Consider the following vector code -
  // %1 = sitofp <4 x i32> %0 to <4 x double>
  //
  // If NumParts is 2, then shuffle values for %1 for different parts are -
  // If Part = 1, output value is -
  // %shuffle = shufflevector <4 x double> %1, <4 x double> undef,
  //                                           <2 x i32> <i32 0, i32 1>
  //
  // and if Part = 2, output is -
  // %shuffle7 =shufflevector <4 x double> %1, <4 x double> undef,
  //                                           <2 x i32> <i32 2, i32 3>

  if (!V)
    return nullptr; // No vector to extract from.

  assert(NumParts > 0 && "Invalid number of subparts of vector.");

  if (NumParts == 1) {
    // Return the original vector as there is only one Part.
    return V;
  }

  unsigned VecLen = cast<FixedVectorType>(V->getType())->getNumElements();
  assert(VecLen % NumParts == 0 &&
         "Vector cannot be divided into unequal parts for extraction");
  assert(Part < NumParts && "Invalid subpart to be extracted from vector.");

  unsigned SubVecLen = VecLen / NumParts;
  SmallVector<int, 4> ShuffleMask;
  Value *Undef = UndefValue::get(V->getType());

  unsigned ElemIdx = Part * SubVecLen;

  for (unsigned K = 0; K < SubVecLen; K++)
    ShuffleMask.push_back(ElemIdx + K);

  auto *ShuffleInst = Builder.CreateShuffleVector(
      V, Undef, ShuffleMask,
      !Name.isTriviallyEmpty() ? Name
                               : V->getName() + ".part." + Twine(Part) +
                                     ".of." + Twine(NumParts) + ".");

  return ShuffleInst;
}

#endif // INTEL_CUSTOMIZATION
/// Add all access groups in @p AccGroups to @p List.
template <typename ListT>
static void addToAccessGroupList(ListT &List, MDNode *AccGroups) {
  // Interpret an access group as a list containing itself.
  if (AccGroups->getNumOperands() == 0) {
    assert(isValidAsAccessGroup(AccGroups) && "Node must be an access group");
    List.insert(AccGroups);
    return;
  }

  for (const auto &AccGroupListOp : AccGroups->operands()) {
    auto *Item = cast<MDNode>(AccGroupListOp.get());
    assert(isValidAsAccessGroup(Item) && "List item must be an access group");
    List.insert(Item);
  }
}

MDNode *llvm::uniteAccessGroups(MDNode *AccGroups1, MDNode *AccGroups2) {
  if (!AccGroups1)
    return AccGroups2;
  if (!AccGroups2)
    return AccGroups1;
  if (AccGroups1 == AccGroups2)
    return AccGroups1;

  SmallSetVector<Metadata *, 4> Union;
  addToAccessGroupList(Union, AccGroups1);
  addToAccessGroupList(Union, AccGroups2);

  if (Union.size() == 0)
    return nullptr;
  if (Union.size() == 1)
    return cast<MDNode>(Union.front());

  LLVMContext &Ctx = AccGroups1->getContext();
  return MDNode::get(Ctx, Union.getArrayRef());
}

MDNode *llvm::intersectAccessGroups(const Instruction *Inst1,
                                    const Instruction *Inst2) {
  bool MayAccessMem1 = Inst1->mayReadOrWriteMemory();
  bool MayAccessMem2 = Inst2->mayReadOrWriteMemory();

  if (!MayAccessMem1 && !MayAccessMem2)
    return nullptr;
  if (!MayAccessMem1)
    return Inst2->getMetadata(LLVMContext::MD_access_group);
  if (!MayAccessMem2)
    return Inst1->getMetadata(LLVMContext::MD_access_group);

  MDNode *MD1 = Inst1->getMetadata(LLVMContext::MD_access_group);
  MDNode *MD2 = Inst2->getMetadata(LLVMContext::MD_access_group);
  if (!MD1 || !MD2)
    return nullptr;
  if (MD1 == MD2)
    return MD1;

  // Use set for scalable 'contains' check.
  SmallPtrSet<Metadata *, 4> AccGroupSet2;
  addToAccessGroupList(AccGroupSet2, MD2);

  SmallVector<Metadata *, 4> Intersection;
  if (MD1->getNumOperands() == 0) {
    assert(isValidAsAccessGroup(MD1) && "Node must be an access group");
    if (AccGroupSet2.count(MD1))
      Intersection.push_back(MD1);
  } else {
    for (const MDOperand &Node : MD1->operands()) {
      auto *Item = cast<MDNode>(Node.get());
      assert(isValidAsAccessGroup(Item) && "List item must be an access group");
      if (AccGroupSet2.count(Item))
        Intersection.push_back(Item);
    }
  }

  if (Intersection.size() == 0)
    return nullptr;
  if (Intersection.size() == 1)
    return cast<MDNode>(Intersection.front());

  LLVMContext &Ctx = Inst1->getContext();
  return MDNode::get(Ctx, Intersection);
}

/// \returns \p I after propagating metadata from \p VL.
Instruction *llvm::propagateMetadata(Instruction *Inst, ArrayRef<Value *> VL) {
  if (VL.empty())
    return Inst;
  Instruction *I0 = cast<Instruction>(VL[0]);
  SmallVector<std::pair<unsigned, MDNode *>, 4> Metadata;
  I0->getAllMetadataOtherThanDebugLoc(Metadata);

  for (auto Kind : {LLVMContext::MD_tbaa, LLVMContext::MD_alias_scope,
                    LLVMContext::MD_noalias, LLVMContext::MD_fpmath,
                    LLVMContext::MD_nontemporal, LLVMContext::MD_invariant_load,
                    LLVMContext::MD_access_group}) {
    MDNode *MD = I0->getMetadata(Kind);

    for (int J = 1, E = VL.size(); MD && J != E; ++J) {
      const Instruction *IJ = cast<Instruction>(VL[J]);
      MDNode *IMD = IJ->getMetadata(Kind);
      switch (Kind) {
      case LLVMContext::MD_tbaa:
        MD = MDNode::getMostGenericTBAA(MD, IMD);
        break;
      case LLVMContext::MD_alias_scope:
        MD = MDNode::getMostGenericAliasScope(MD, IMD);
        break;
      case LLVMContext::MD_fpmath:
        MD = MDNode::getMostGenericFPMath(MD, IMD);
        break;
      case LLVMContext::MD_noalias:
      case LLVMContext::MD_nontemporal:
      case LLVMContext::MD_invariant_load:
        MD = MDNode::intersect(MD, IMD);
        break;
      case LLVMContext::MD_access_group:
        MD = intersectAccessGroups(Inst, IJ);
        break;
      default:
        llvm_unreachable("unhandled metadata");
      }
    }

    Inst->setMetadata(Kind, MD);
  }

  return Inst;
}

Constant *
llvm::createBitMaskForGaps(IRBuilderBase &Builder, unsigned VF,
                           const InterleaveGroup<Instruction> &Group) {
  // All 1's means mask is not needed.
  if (Group.getNumMembers() == Group.getFactor())
    return nullptr;

  // TODO: support reversed access.
  assert(!Group.isReverse() && "Reversed group not supported.");

  SmallVector<Constant *, 16> Mask;
  for (unsigned i = 0; i < VF; i++)
    for (unsigned j = 0; j < Group.getFactor(); ++j) {
      unsigned HasMember = Group.getMember(j) ? 1 : 0;
      Mask.push_back(Builder.getInt1(HasMember));
    }

  return ConstantVector::get(Mask);
}

llvm::SmallVector<int, 16>
llvm::createReplicatedMask(unsigned ReplicationFactor, unsigned VF) {
  SmallVector<int, 16> MaskVec;
  for (unsigned i = 0; i < VF; i++)
    for (unsigned j = 0; j < ReplicationFactor; j++)
      MaskVec.push_back(i);

  return MaskVec;
}

llvm::SmallVector<int, 16> llvm::createInterleaveMask(unsigned VF,
                                                      unsigned NumVecs) {
  SmallVector<int, 16> Mask;
  for (unsigned i = 0; i < VF; i++)
    for (unsigned j = 0; j < NumVecs; j++)
      Mask.push_back(j * VF + i);

  return Mask;
}

llvm::SmallVector<int, 16>
llvm::createStrideMask(unsigned Start, unsigned Stride, unsigned VF) {
  SmallVector<int, 16> Mask;
  for (unsigned i = 0; i < VF; i++)
    Mask.push_back(Start + i * Stride);

  return Mask;
}

#if INTEL_CUSTOMIZATION
llvm::SmallVector<int, 64> llvm::createVectorInterleaveMask(unsigned VF,
                                                            unsigned NumVecs,
                                                            unsigned VecWidth) {
  SmallVector<int, 64> Mask;
  for (unsigned i = 0; i < VF; i++)
    for (unsigned j = 0; j < NumVecs; j++)
      for (unsigned k = 0; k < VecWidth; k++)
        Mask.push_back((j * VF + i) * VecWidth + k);

  return Mask;
}

llvm::SmallVector<int, 64> llvm::createVectorStrideMask(unsigned Start,
                                                        unsigned Stride,
                                                        unsigned VF,
                                                        unsigned VecWidth) {
  SmallVector<int, 64> Mask;
  for (unsigned i = 0; i < VF; i++)
    for (unsigned j = 0; j < VecWidth; j++)
      Mask.push_back((Start + i * Stride) * VecWidth + j);

  return Mask;
}
#endif // INTEL_CUSTOMIZATION

llvm::SmallVector<int, 16> llvm::createSequentialMask(unsigned Start,
                                                      unsigned NumInts,
                                                      unsigned NumUndefs) {
  SmallVector<int, 16> Mask;
  for (unsigned i = 0; i < NumInts; i++)
    Mask.push_back(Start + i);

  for (unsigned i = 0; i < NumUndefs; i++)
    Mask.push_back(-1);

  return Mask;
}

llvm::SmallVector<int, 16> llvm::createUnaryMask(ArrayRef<int> Mask,
                                                 unsigned NumElts) {
  // Avoid casts in the loop and make sure we have a reasonable number.
  int NumEltsSigned = NumElts;
  assert(NumEltsSigned > 0 && "Expected smaller or non-zero element count");

  // If the mask chooses an element from operand 1, reduce it to choose from the
  // corresponding element of operand 0. Undef mask elements are unchanged.
  SmallVector<int, 16> UnaryMask;
  for (int MaskElt : Mask) {
    assert((MaskElt < NumEltsSigned * 2) && "Expected valid shuffle mask");
    int UnaryElt = MaskElt >= NumEltsSigned ? MaskElt - NumEltsSigned : MaskElt;
    UnaryMask.push_back(UnaryElt);
  }
  return UnaryMask;
}

/// A helper function for concatenating vectors. This function concatenates two
/// vectors having the same element type. If the second vector has fewer
/// elements than the first, it is padded with undefs.
static Value *concatenateTwoVectors(IRBuilderBase &Builder, Value *V1,
                                    Value *V2) {
  VectorType *VecTy1 = dyn_cast<VectorType>(V1->getType());
  VectorType *VecTy2 = dyn_cast<VectorType>(V2->getType());
  assert(VecTy1 && VecTy2 &&
         VecTy1->getScalarType() == VecTy2->getScalarType() &&
         "Expect two vectors with the same element type");

  unsigned NumElts1 = cast<FixedVectorType>(VecTy1)->getNumElements();
  unsigned NumElts2 = cast<FixedVectorType>(VecTy2)->getNumElements();
  assert(NumElts1 >= NumElts2 && "Unexpect the first vector has less elements");

  if (NumElts1 > NumElts2) {
    // Extend with UNDEFs.
    V2 = Builder.CreateShuffleVector(
        V2, createSequentialMask(0, NumElts2, NumElts1 - NumElts2));
  }

  return Builder.CreateShuffleVector(
      V1, V2, createSequentialMask(0, NumElts1 + NumElts2, 0));
}

Value *llvm::concatenateVectors(IRBuilderBase &Builder,
                                ArrayRef<Value *> Vecs) {
  unsigned NumVecs = Vecs.size();
  assert(NumVecs > 1 && "Should be at least two vectors");

  SmallVector<Value *, 8> ResList;
  ResList.append(Vecs.begin(), Vecs.end());
  do {
    SmallVector<Value *, 8> TmpList;
    for (unsigned i = 0; i < NumVecs - 1; i += 2) {
      Value *V0 = ResList[i], *V1 = ResList[i + 1];
      assert((V0->getType() == V1->getType() || i == NumVecs - 2) &&
             "Only the last vector may have a different type");

      TmpList.push_back(concatenateTwoVectors(Builder, V0, V1));
    }

    // Push the last vector if the total number of vectors is odd.
    if (NumVecs % 2 != 0)
      TmpList.push_back(ResList[NumVecs - 1]);

    ResList = TmpList;
    NumVecs = ResList.size();
  } while (NumVecs > 1);

  return ResList[0];
}

bool llvm::maskIsAllZeroOrUndef(Value *Mask) {
  assert(isa<VectorType>(Mask->getType()) &&
         isa<IntegerType>(Mask->getType()->getScalarType()) &&
         cast<IntegerType>(Mask->getType()->getScalarType())->getBitWidth() ==
             1 &&
         "Mask must be a vector of i1");

  auto *ConstMask = dyn_cast<Constant>(Mask);
  if (!ConstMask)
    return false;
  if (ConstMask->isNullValue() || isa<UndefValue>(ConstMask))
    return true;
  if (isa<ScalableVectorType>(ConstMask->getType()))
    return false;
  for (unsigned
           I = 0,
           E = cast<FixedVectorType>(ConstMask->getType())->getNumElements();
       I != E; ++I) {
    if (auto *MaskElt = ConstMask->getAggregateElement(I))
      if (MaskElt->isNullValue() || isa<UndefValue>(MaskElt))
        continue;
    return false;
  }
  return true;
}

bool llvm::maskIsAllOneOrUndef(Value *Mask) {
  assert(isa<VectorType>(Mask->getType()) &&
         isa<IntegerType>(Mask->getType()->getScalarType()) &&
         cast<IntegerType>(Mask->getType()->getScalarType())->getBitWidth() ==
             1 &&
         "Mask must be a vector of i1");

  auto *ConstMask = dyn_cast<Constant>(Mask);
  if (!ConstMask)
    return false;
  if (ConstMask->isAllOnesValue() || isa<UndefValue>(ConstMask))
    return true;
  if (isa<ScalableVectorType>(ConstMask->getType()))
    return false;
  for (unsigned
           I = 0,
           E = cast<FixedVectorType>(ConstMask->getType())->getNumElements();
       I != E; ++I) {
    if (auto *MaskElt = ConstMask->getAggregateElement(I))
      if (MaskElt->isAllOnesValue() || isa<UndefValue>(MaskElt))
        continue;
    return false;
  }
  return true;
}

/// TODO: This is a lot like known bits, but for
/// vectors.  Is there something we can common this with?
APInt llvm::possiblyDemandedEltsInMask(Value *Mask) {
  assert(isa<FixedVectorType>(Mask->getType()) &&
         isa<IntegerType>(Mask->getType()->getScalarType()) &&
         cast<IntegerType>(Mask->getType()->getScalarType())->getBitWidth() ==
             1 &&
         "Mask must be a fixed width vector of i1");

  const unsigned VWidth =
      cast<FixedVectorType>(Mask->getType())->getNumElements();
  APInt DemandedElts = APInt::getAllOnes(VWidth);
  if (auto *CV = dyn_cast<ConstantVector>(Mask))
    for (unsigned i = 0; i < VWidth; i++)
      if (CV->getAggregateElement(i)->isNullValue())
        DemandedElts.clearBit(i);
  return DemandedElts;
}

bool InterleavedAccessInfo::isStrided(int Stride) {
  unsigned Factor = std::abs(Stride);
  return Factor >= 2 && Factor <= MaxInterleaveGroupFactor;
}

void InterleavedAccessInfo::collectConstStrideAccesses(
    MapVector<Instruction *, StrideDescriptor> &AccessStrideInfo,
    const ValueToValueMap &Strides) {
  auto &DL = TheLoop->getHeader()->getModule()->getDataLayout();

  // Since it's desired that the load/store instructions be maintained in
  // "program order" for the interleaved access analysis, we have to visit the
  // blocks in the loop in reverse postorder (i.e., in a topological order).
  // Such an ordering will ensure that any load/store that may be executed
  // before a second load/store will precede the second load/store in
  // AccessStrideInfo.
  LoopBlocksDFS DFS(TheLoop);
  DFS.perform(LI);
  for (BasicBlock *BB : make_range(DFS.beginRPO(), DFS.endRPO()))
    for (auto &I : *BB) {
      Value *Ptr = getLoadStorePointerOperand(&I);
      if (!Ptr)
        continue;
      Type *ElementTy = getLoadStoreType(&I);

      // We don't check wrapping here because we don't know yet if Ptr will be
      // part of a full group or a group with gaps. Checking wrapping for all
      // pointers (even those that end up in groups with no gaps) will be overly
      // conservative. For full groups, wrapping should be ok since if we would
      // wrap around the address space we would do a memory access at nullptr
      // even without the transformation. The wrapping checks are therefore
      // deferred until after we've formed the interleaved groups.
      int64_t Stride = getPtrStride(PSE, ElementTy, Ptr, TheLoop, Strides,
                                    /*Assume=*/true, /*ShouldCheckWrap=*/false);

      const SCEV *Scev = replaceSymbolicStrideSCEV(PSE, Strides, Ptr);
      uint64_t Size = DL.getTypeAllocSize(ElementTy);
      AccessStrideInfo[&I] = StrideDescriptor(Stride, Scev, Size,
                                              getLoadStoreAlignment(&I));
    }
}

// Analyze interleaved accesses and collect them into interleaved load and
// store groups.
//
// When generating code for an interleaved load group, we effectively hoist all
// loads in the group to the location of the first load in program order. When
// generating code for an interleaved store group, we sink all stores to the
// location of the last store. This code motion can change the order of load
// and store instructions and may break dependences.
//
// The code generation strategy mentioned above ensures that we won't violate
// any write-after-read (WAR) dependences.
//
// E.g., for the WAR dependence:  a = A[i];      // (1)
//                                A[i] = b;      // (2)
//
// The store group of (2) is always inserted at or below (2), and the load
// group of (1) is always inserted at or above (1). Thus, the instructions will
// never be reordered. All other dependences are checked to ensure the
// correctness of the instruction reordering.
//
// The algorithm visits all memory accesses in the loop in bottom-up program
// order. Program order is established by traversing the blocks in the loop in
// reverse postorder when collecting the accesses.
//
// We visit the memory accesses in bottom-up order because it can simplify the
// construction of store groups in the presence of write-after-write (WAW)
// dependences.
//
// E.g., for the WAW dependence:  A[i] = a;      // (1)
//                                A[i] = b;      // (2)
//                                A[i + 1] = c;  // (3)
//
// We will first create a store group with (3) and (2). (1) can't be added to
// this group because it and (2) are dependent. However, (1) can be grouped
// with other accesses that may precede it in program order. Note that a
// bottom-up order does not imply that WAW dependences should not be checked.
void InterleavedAccessInfo::analyzeInterleaving(
                                 bool EnablePredicatedInterleavedMemAccesses) {
  LLVM_DEBUG(dbgs() << "LV: Analyzing interleaved accesses...\n");
  const ValueToValueMap &Strides = LAI->getSymbolicStrides();

  // Holds all accesses with a constant stride.
  MapVector<Instruction *, StrideDescriptor> AccessStrideInfo;
  collectConstStrideAccesses(AccessStrideInfo, Strides);

  if (AccessStrideInfo.empty())
    return;

  // Collect the dependences in the loop.
  collectDependences();

  // Holds all interleaved store groups temporarily.
  SmallSetVector<InterleaveGroup<Instruction> *, 4> StoreGroups;
  // Holds all interleaved load groups temporarily.
  SmallSetVector<InterleaveGroup<Instruction> *, 4> LoadGroups;

  // Search in bottom-up program order for pairs of accesses (A and B) that can
  // form interleaved load or store groups. In the algorithm below, access A
  // precedes access B in program order. We initialize a group for B in the
  // outer loop of the algorithm, and then in the inner loop, we attempt to
  // insert each A into B's group if:
  //
  //  1. A and B have the same stride,
  //  2. A and B have the same memory object size, and
  //  3. A belongs in B's group according to its distance from B.
  //
  // Special care is taken to ensure group formation will not break any
  // dependences.
  for (auto BI = AccessStrideInfo.rbegin(), E = AccessStrideInfo.rend();
       BI != E; ++BI) {
    Instruction *B = BI->first;
    StrideDescriptor DesB = BI->second;

    // Initialize a group for B if it has an allowable stride. Even if we don't
    // create a group for B, we continue with the bottom-up algorithm to ensure
    // we don't break any of B's dependences.
    InterleaveGroup<Instruction> *Group = nullptr;
    if (isStrided(DesB.Stride) &&
        (!isPredicated(B->getParent()) || EnablePredicatedInterleavedMemAccesses)) {
      Group = getInterleaveGroup(B);
      if (!Group) {
        LLVM_DEBUG(dbgs() << "LV: Creating an interleave group with:" << *B
                          << '\n');
        Group = createInterleaveGroup(B, DesB.Stride, DesB.Alignment);
      }
      if (B->mayWriteToMemory())
        StoreGroups.insert(Group);
      else
        LoadGroups.insert(Group);
    }

    for (auto AI = std::next(BI); AI != E; ++AI) {
      Instruction *A = AI->first;
      StrideDescriptor DesA = AI->second;

      // Our code motion strategy implies that we can't have dependences
      // between accesses in an interleaved group and other accesses located
      // between the first and last member of the group. Note that this also
      // means that a group can't have more than one member at a given offset.
      // The accesses in a group can have dependences with other accesses, but
      // we must ensure we don't extend the boundaries of the group such that
      // we encompass those dependent accesses.
      //
      // For example, assume we have the sequence of accesses shown below in a
      // stride-2 loop:
      //
      //  (1, 2) is a group | A[i]   = a;  // (1)
      //                    | A[i-1] = b;  // (2) |
      //                      A[i-3] = c;  // (3)
      //                      A[i]   = d;  // (4) | (2, 4) is not a group
      //
      // Because accesses (2) and (3) are dependent, we can group (2) with (1)
      // but not with (4). If we did, the dependent access (3) would be within
      // the boundaries of the (2, 4) group.
      if (!canReorderMemAccessesForInterleavedGroups(&*AI, &*BI)) {
        // If a dependence exists and A is already in a group, we know that A
        // must be a store since A precedes B and WAR dependences are allowed.
        // Thus, A would be sunk below B. We release A's group to prevent this
        // illegal code motion. A will then be free to form another group with
        // instructions that precede it.
        if (isInterleaved(A)) {
          InterleaveGroup<Instruction> *StoreGroup = getInterleaveGroup(A);

          LLVM_DEBUG(dbgs() << "LV: Invalidated store group due to "
                               "dependence between " << *A << " and "<< *B << '\n');

          StoreGroups.remove(StoreGroup);
          releaseGroup(StoreGroup);
        }

        // If a dependence exists and A is not already in a group (or it was
        // and we just released it), B might be hoisted above A (if B is a
        // load) or another store might be sunk below A (if B is a store). In
        // either case, we can't add additional instructions to B's group. B
        // will only form a group with instructions that it precedes.
        break;
      }

      // At this point, we've checked for illegal code motion. If either A or B
      // isn't strided, there's nothing left to do.
      if (!isStrided(DesA.Stride) || !isStrided(DesB.Stride))
        continue;

      // Ignore A if it's already in a group or isn't the same kind of memory
      // operation as B.
      // Note that mayReadFromMemory() isn't mutually exclusive to
      // mayWriteToMemory in the case of atomic loads. We shouldn't see those
      // here, canVectorizeMemory() should have returned false - except for the
      // case we asked for optimization remarks.
      if (isInterleaved(A) ||
          (A->mayReadFromMemory() != B->mayReadFromMemory()) ||
          (A->mayWriteToMemory() != B->mayWriteToMemory()))
        continue;

      // Check rules 1 and 2. Ignore A if its stride or size is different from
      // that of B.
      if (DesA.Stride != DesB.Stride || DesA.Size != DesB.Size)
        continue;

      // Ignore A if the memory object of A and B don't belong to the same
      // address space
      if (getLoadStoreAddressSpace(A) != getLoadStoreAddressSpace(B))
        continue;

      // Calculate the distance from A to B.
      const SCEVConstant *DistToB = dyn_cast<SCEVConstant>(
          PSE.getSE()->getMinusSCEV(DesA.Scev, DesB.Scev));
      if (!DistToB)
        continue;
      int64_t DistanceToB = DistToB->getAPInt().getSExtValue();

      // Check rule 3. Ignore A if its distance to B is not a multiple of the
      // size.
      if (DistanceToB % static_cast<int64_t>(DesB.Size))
        continue;

      // All members of a predicated interleave-group must have the same predicate,
      // and currently must reside in the same BB.
      BasicBlock *BlockA = A->getParent();
      BasicBlock *BlockB = B->getParent();
      if ((isPredicated(BlockA) || isPredicated(BlockB)) &&
          (!EnablePredicatedInterleavedMemAccesses || BlockA != BlockB))
        continue;

      // The index of A is the index of B plus A's distance to B in multiples
      // of the size.
#if INTEL_CUSTOMIZATION
      assert(Group && "Group is expected to be non-null");
#endif // INTEL_CUSTOMIZATION
      int IndexA =
          Group->getIndex(B) + DistanceToB / static_cast<int64_t>(DesB.Size);

      // Try to insert A into B's group.
      if (Group->insertMember(A, IndexA, DesA.Alignment)) {
        LLVM_DEBUG(dbgs() << "LV: Inserted:" << *A << '\n'
                          << "    into the interleave group with" << *B
                          << '\n');
        InterleaveGroupMap[A] = Group;

        // Set the first load in program order as the insert position.
        if (A->mayReadFromMemory())
          Group->setInsertPos(A);
      }
    } // Iteration over A accesses.
  }   // Iteration over B accesses.

  auto InvalidateGroupIfMemberMayWrap = [&](InterleaveGroup<Instruction> *Group,
                                            int Index,
                                            std::string FirstOrLast) -> bool {
    Instruction *Member = Group->getMember(Index);
    assert(Member && "Group member does not exist");
    Value *MemberPtr = getLoadStorePointerOperand(Member);
    Type *AccessTy = getLoadStoreType(Member);
    if (getPtrStride(PSE, AccessTy, MemberPtr, TheLoop, Strides,
                     /*Assume=*/false, /*ShouldCheckWrap=*/true))
      return false;
    LLVM_DEBUG(dbgs() << "LV: Invalidate candidate interleaved group due to "
                      << FirstOrLast
                      << " group member potentially pointer-wrapping.\n");
    releaseGroup(Group);
    return true;
  };

  // Remove interleaved groups with gaps whose memory
  // accesses may wrap around. We have to revisit the getPtrStride analysis,
  // this time with ShouldCheckWrap=true, since collectConstStrideAccesses does
  // not check wrapping (see documentation there).
  // FORNOW we use Assume=false;
  // TODO: Change to Assume=true but making sure we don't exceed the threshold
  // of runtime SCEV assumptions checks (thereby potentially failing to
  // vectorize altogether).
  // Additional optional optimizations:
  // TODO: If we are peeling the loop and we know that the first pointer doesn't
  // wrap then we can deduce that all pointers in the group don't wrap.
  // This means that we can forcefully peel the loop in order to only have to
  // check the first pointer for no-wrap. When we'll change to use Assume=true
  // we'll only need at most one runtime check per interleaved group.
  for (auto *Group : LoadGroups) {
    // Case 1: A full group. Can Skip the checks; For full groups, if the wide
    // load would wrap around the address space we would do a memory access at
    // nullptr even without the transformation.
    if (Group->getNumMembers() == Group->getFactor())
      continue;

    // Case 2: If first and last members of the group don't wrap this implies
    // that all the pointers in the group don't wrap.
    // So we check only group member 0 (which is always guaranteed to exist),
    // and group member Factor - 1; If the latter doesn't exist we rely on
    // peeling (if it is a non-reversed accsess -- see Case 3).
    if (InvalidateGroupIfMemberMayWrap(Group, 0, std::string("first")))
      continue;
    if (Group->getMember(Group->getFactor() - 1))
      InvalidateGroupIfMemberMayWrap(Group, Group->getFactor() - 1,
                                     std::string("last"));
    else {
      // Case 3: A non-reversed interleaved load group with gaps: We need
      // to execute at least one scalar epilogue iteration. This will ensure
      // we don't speculatively access memory out-of-bounds. We only need
      // to look for a member at index factor - 1, since every group must have
      // a member at index zero.
      if (Group->isReverse()) {
        LLVM_DEBUG(
            dbgs() << "LV: Invalidate candidate interleaved group due to "
                      "a reverse access with gaps.\n");
        releaseGroup(Group);
        continue;
      }
      LLVM_DEBUG(
          dbgs() << "LV: Interleaved group requires epilogue iteration.\n");
      RequiresScalarEpilogue = true;
    }
  }

  for (auto *Group : StoreGroups) {
    // Case 1: A full group. Can Skip the checks; For full groups, if the wide
    // store would wrap around the address space we would do a memory access at
    // nullptr even without the transformation.
    if (Group->getNumMembers() == Group->getFactor())
      continue;

    // Interleave-store-group with gaps is implemented using masked wide store.
    // Remove interleaved store groups with gaps if
    // masked-interleaved-accesses are not enabled by the target.
    if (!EnablePredicatedInterleavedMemAccesses) {
      LLVM_DEBUG(
          dbgs() << "LV: Invalidate candidate interleaved store group due "
                    "to gaps.\n");
      releaseGroup(Group);
      continue;
    }

    // Case 2: If first and last members of the group don't wrap this implies
    // that all the pointers in the group don't wrap.
    // So we check only group member 0 (which is always guaranteed to exist),
    // and the last group member. Case 3 (scalar epilog) is not relevant for
    // stores with gaps, which are implemented with masked-store (rather than
    // speculative access, as in loads).
    if (InvalidateGroupIfMemberMayWrap(Group, 0, std::string("first")))
      continue;
    for (int Index = Group->getFactor() - 1; Index > 0; Index--)
      if (Group->getMember(Index)) {
        InvalidateGroupIfMemberMayWrap(Group, Index, std::string("last"));
        break;
      }
  }
}

void InterleavedAccessInfo::invalidateGroupsRequiringScalarEpilogue() {
  // If no group had triggered the requirement to create an epilogue loop,
  // there is nothing to do.
  if (!requiresScalarEpilogue())
    return;

  bool ReleasedGroup = false;
  // Release groups requiring scalar epilogues. Note that this also removes them
  // from InterleaveGroups.
  for (auto *Group : make_early_inc_range(InterleaveGroups)) {
    if (!Group->requiresScalarEpilogue())
      continue;
    LLVM_DEBUG(
        dbgs()
        << "LV: Invalidate candidate interleaved group due to gaps that "
           "require a scalar epilogue (not allowed under optsize) and cannot "
           "be masked (not enabled). \n");
    releaseGroup(Group);
    ReleasedGroup = true;
  }
  assert(ReleasedGroup && "At least one group must be invalidated, as a "
                          "scalar epilogue was required");
  (void)ReleasedGroup;
  RequiresScalarEpilogue = false;
}

template <typename InstT>
void InterleaveGroup<InstT>::addMetadata(InstT *NewInst) const {
  llvm_unreachable("addMetadata can only be used for Instruction");
}

namespace llvm {
template <>
void InterleaveGroup<Instruction>::addMetadata(Instruction *NewInst) const {
  SmallVector<Value *, 4> VL;
  std::transform(Members.begin(), Members.end(), std::back_inserter(VL),
                 [](std::pair<int, Instruction *> p) { return p.second; });
  propagateMetadata(NewInst, VL);
}
}

std::string VFABI::mangleTLIVectorName(StringRef VectorName,
                                       StringRef ScalarName, unsigned numArgs,
                                       ElementCount VF) {
  SmallString<256> Buffer;
  llvm::raw_svector_ostream Out(Buffer);
  Out << "_ZGV" << VFABI::_LLVM_ << "N";
  if (VF.isScalable())
    Out << 'x';
  else
    Out << VF.getFixedValue();
  for (unsigned I = 0; I < numArgs; ++I)
    Out << "v";
  Out << "_" << ScalarName << "(" << VectorName << ")";
  return std::string(Out.str());
}

void VFABI::getVectorVariantNames(
    const CallInst &CI, SmallVectorImpl<std::string> &VariantMappings) {
  const StringRef S = CI.getFnAttr(VFABI::MappingsAttrName).getValueAsString();
  if (S.empty())
    return;

  SmallVector<StringRef, 8> ListAttr;
  S.split(ListAttr, ",");

  for (const auto &S : SetVector<StringRef>(ListAttr.begin(), ListAttr.end())) {
#ifndef NDEBUG
    LLVM_DEBUG(dbgs() << "VFABI: adding mapping '" << S << "'\n");
    Optional<VFInfo> Info = VFABI::tryDemangleForVFABI(S, *(CI.getModule()));
    assert(Info && "Invalid name for a VFABI variant.");
    assert(CI.getModule()->getFunction(Info.value().VectorName) &&
           "Vector function is missing.");
#endif
    VariantMappings.push_back(std::string(S));
  }
}

bool VFShape::hasValidParameterList(bool Permissive) const { // INTEL
  for (unsigned Pos = 0, NumParams = Parameters.size(); Pos < NumParams;
       ++Pos) {
    assert(Parameters[Pos].ParamPos == Pos && "Broken parameter list.");

    switch (Parameters[Pos].ParamKind) {
    default: // Nothing to check.
      break;
    case VFParamKind::OMP_Linear:
    case VFParamKind::OMP_LinearRef:
    case VFParamKind::OMP_LinearVal:
    case VFParamKind::OMP_LinearUVal:
      // Compile time linear steps must be non-zero.
      if (Parameters[Pos].LinearStepOrPos == 0)
        return false;
      break;
    case VFParamKind::OMP_LinearPos:
    case VFParamKind::OMP_LinearRefPos:
    case VFParamKind::OMP_LinearValPos:
    case VFParamKind::OMP_LinearUValPos:
      // The runtime linear step must be referring to some other
      // parameters in the signature.
      if (Parameters[Pos].LinearStepOrPos >= int(NumParams))
        return false;
#if INTEL_CUSTOMIZATION
      if (Permissive && Parameters[Pos].LinearStepOrPos == int(Pos)) {
        // This is currently explicitly allowed because we generate
        // variable-strided params that refer to themselves during VPlan call
        // vec decisions (when generating vector variants from a given call).
        // While this is technically incorrect, at the present time, this is
        // fine, because we don't actually support variable-strided params in
        // any subsequent processing that might use the vector variant.
        //
        // TODO: Once we properly support variable-strided params and properly
        // generate variable-strided params during VPlan call vec decisions,
        // this case should be disallowed again.
        return true;
      }
#endif
      // The linear step parameter must be marked as uniform.
      if (Parameters[Parameters[Pos].LinearStepOrPos].ParamKind !=
          VFParamKind::OMP_Uniform)
        return false;
      // The linear step parameter can't point at itself.
      if (Parameters[Pos].LinearStepOrPos == int(Pos))
        return false;
      break;
    case VFParamKind::GlobalPredicate:
      // The global predicate must be the unique. Can be placed anywhere in the
      // signature.
      for (unsigned NextPos = Pos + 1; NextPos < NumParams; ++NextPos)
        if (Parameters[NextPos].ParamKind == VFParamKind::GlobalPredicate)
          return false;
      break;
    }
  }
  return true;
}
