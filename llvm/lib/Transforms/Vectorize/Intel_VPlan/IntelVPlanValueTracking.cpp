//===- IntelVPlanValueTracking.cpp ------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanValueTracking.h"

#include "IntelVPlan.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanScalarEvolution.h"

#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <llvm/Analysis/ValueTracking.h>

#define DEBUG_TYPE "vplan-value-tracking"

using namespace llvm;
using namespace llvm::vpo;

namespace llvm {
namespace vpo {

/// Static class containing the implementation of IR-agnostic ValueTracking.
/// This would be a namespace, but is instead a static class (no members, only
/// static functions) so that it can be befriend VPlanValueTrackingLLVM and
/// VPlanValueTrackingHIR.
class VPlanValueTrackingImpl {
public:
  /// A struct representing the query being executed, containing all
  /// of the necessary context to evaluate that query.
  struct Query {
    const DataLayout &DL;
    VPAssumptionCache &VPAC;
    const VPDominatorTree &VPDT;
    const DominatorTree &DT;

    const VPInstruction *CtxI;
  };

  template <typename ValueTracking>
  static Query query(const ValueTracking *VPVT, const VPInstruction *CtxI) {
    if (const auto *VT = dyn_cast<VPlanValueTrackingLLVM>(VPVT))
      return Query{*VT->DL, *VT->VPAC, *VT->VPDT, *VT->DT, CtxI};
    if (const auto *VT = dyn_cast<VPlanValueTrackingHIR>(VPVT))
      return Query{*VT->DL, *VT->VPAC, *VT->VPDT, *VT->DT, CtxI};
    llvm_unreachable("unknown value tracking kind!");
  }

  /// How deep to recurse before we assume unknown bits. This value is copied
  /// from LLVM's ValueTracking.
  static constexpr const unsigned MaxRecursionDepth = 6;

  /// Try to compute known bits for the given VPValue \p V, at \p Depth, with
  /// the query context \p Q. This tries to either identify a special case, or
  /// recurse on the structure of the value (i.e. for a VPInstruction) and
  /// combine the results. If Depth is greater than our MaxRecursionDepth, we
  /// return a unknown KnownBits of the appropriate bitwidth.
  static KnownBits computeKnownBits(const VPValue *V, unsigned Depth,
                                    const Query &Q) {
    assert(V->getType()->isIntOrPtrTy() &&
           "can't calculate known bits for non-integral types!");

    const unsigned BitWidth = static_cast<unsigned>(
        Q.DL.getTypeSizeInBits(V->getType()).getKnownMinSize());

    // Constants are a special base case for which all known bits are known.
    if (const auto *C = dyn_cast<VPConstant>(V)) {
      if (const auto *CI = dyn_cast<ConstantInt>(C->getConstant()))
        return KnownBits::makeConstant(CI->getValue());
      if (isa<ConstantPointerNull>(C->getConstant()))
        return KnownBits::makeConstant(APInt::getNullValue(BitWidth));
    }

    // Otherwise, we know nothing about V's known bits.
    KnownBits KB{BitWidth};

    // If we've reached our recursion depth, assume unknown.
    if (Depth++ >= MaxRecursionDepth)
      return KB;

    // Otherwise, try to recurse on the structure of V.
    switch (V->getVPValueID()) {
    case VPValue::VPInstructionSC: {
      computeKnownBitsFromInst(cast<VPInstruction>(V), KB, Depth, Q);
      break;
    }
    case VPValue::VPExternalDefSC: {
      if (VPlanValueTracking::getUseUnderlyingValues()) {
        // Generally, we can't say much about an external def. However, if it
        // has an underlying value, we can try to use LLVM's ValueTracking to
        // obtain an answer.
        if (const Value *IRVal = V->getUnderlyingValue())
          llvm::computeKnownBits(IRVal, KB, Q.DL, Depth, Q.VPAC.getLLVMCache(),
                                 tryToGetUnderlyingInst(Q.CtxI), &Q.DT);
      }
      break;
    }
    }

    // Now, try to refine the known bits using any assumptions.
    computeKnownBitsFromAssume(V, KB, Q);

    assert(!KB.hasConflict() && "Bits known to be one AND zero?");
    return KB;
  }

  static void computeKnownBitsFromInst(const VPInstruction *I, KnownBits &KB,
                                       unsigned Depth, const Query &Q) {
    const unsigned BitWidth = KB.getBitWidth();

    // Helper to compute known bits of the operand at the given Idx.
    const auto Op = [I, Depth, Q](unsigned Idx) {
      return computeKnownBits(I->getOperand(Idx), Depth, Q);
    };

    switch (I->getOpcode()) {
    case Instruction::Add:
      KB = KnownBits::computeForAddSub(
          /*Add:*/ true, /*NSW:*/ I->hasNoSignedWrap(), Op(0), Op(1));
      break;
    case Instruction::Sub:
      KB = KnownBits::computeForAddSub(
          /*Add:*/ false, /*NSW:*/ I->hasNoSignedWrap(), Op(0), Op(1));
      return;
    case Instruction::Mul:
      KB = KnownBits::mul(Op(0), Op(1));
      return;
    case Instruction::UDiv:
      KB = KnownBits::udiv(Op(0), Op(1));
      return;
    case Instruction::And:
      KB = Op(0) & Op(1);
      return;
    case Instruction::Or:
      KB = Op(0) | Op(1);
      return;
    case Instruction::Xor:
      KB = Op(0) ^ Op(1);
      return;
    case Instruction::LShr:
      KB = KnownBits::lshr(Op(0), Op(1));
      return;
    case Instruction::AShr:
      KB = KnownBits::ashr(Op(0), Op(1));
      return;
    case Instruction::Shl:
      KB = KnownBits::shl(Op(0), Op(1));
      return;
    case Instruction::GetElementPtr:
      computeKnownBitsFromGEP(cast<VPGEPInstruction>(I), KB, Depth, Q);
      return;
    case Instruction::SRem:
      KB = KnownBits::srem(Op(0), Op(1));
      return;
    case Instruction::URem:
      KB = KnownBits::urem(Op(0), Op(1));
      return;
    case Instruction::Trunc:
      KB = Op(0).trunc(BitWidth);
      return;
    case Instruction::SExt:
      KB = Op(0).sext(BitWidth);
      return;
    case Instruction::ZExt:
      KB = Op(0).zext(BitWidth);
      return;
    case VPInstruction::Subscript:
      computeKnownBitsFromSubscript(cast<VPSubscriptInst>(I), KB, Depth, Q);
      return;
    default:
      break;
    }
  }

  static void computeKnownBitsFromGEP(const VPGEPInstruction *GEP,
                                      KnownBits &KB, unsigned Depth,
                                      const Query &Q) {
    // First, compute known bits of the base expression.
    KB = computeKnownBits(GEP->getPointerOperand(), Depth, Q);

    // Now, iterate over the indices of the GEP and compute the total offset,
    // accumulating constant offsets separately so we can efficiently add them
    // at the end.
    const unsigned BitWidth = KB.getBitWidth();
    vp_gep_type_iterator GTI = gep_type_begin(GEP);
    int64_t AccConstOffset = 0;
    for (auto I = GEP->idx_begin(), E = GEP->idx_end(); I != E; ++I, ++GTI) {
      const VPValue *Index = *I;

      // If the computed offset becomes unknown at any point, bail out --
      // further offsets will only result in more unknown bits.
      if (KB.isUnknown())
        return;

      // Skip zero offsets, as these have no effect.
      const auto *Constant = dyn_cast<VPConstantInt>(Index);
      if (Constant && Constant->getConstantInt()->isZeroValue())
        continue;

      // If this is an offset into a struct, compute the resulting element
      // offset in bytes, and add it to our accumulated constant offsets.
      if (StructType *STy = GTI.getStructTypeOrNull()) {
        assert(Constant && "struct offsets must be constant!");
        const uint64_t ElemOffset = Q.DL.getStructLayout(STy)->getElementOffset(
            Constant->getZExtValue());
        AccConstOffset += (int64_t)ElemOffset;
        continue;
      }

      // Otherwise, this is an offset into a sequential type (i.e. ptr/array).
      // Compute the known bits of the index, multiply by the indexed type's
      // size, sign extend the resulting value, and then add it to our total.
      auto OffsetKB = computeKnownBits(Index, Depth, Q);
      const unsigned TypeSizeInBytes =
          Q.DL.getTypeAllocSize(GTI.getIndexedType()).getKnownMinSize();

      // As an optimization, if the index is constant, compute the final offset
      // and add it to our accumulated constant offsets instead.
      if (OffsetKB.isConstant()) {
        AccConstOffset +=
            OffsetKB.getConstant().getSExtValue() * TypeSizeInBytes;
        continue;
      }

      computeMulConst(OffsetKB, TypeSizeInBytes);
      KB = KnownBits::computeForAddSub(/*Add: */ true, /*NSW: */ false, KB,
                                       OffsetKB.sextOrTrunc(BitWidth));
    }

    // Lastly, add all our accumulated constant offsets.
    computeAddConst(KB, AccConstOffset);
  }

  static void computeKnownBitsFromSubscript(const VPSubscriptInst *Sub,
                                            KnownBits &KB, unsigned Depth,
                                            const Query &Q) {
    // First, compute known bits of the base expression.
    KB = computeKnownBits(Sub->getPointerOperand(), Depth, Q);

    // Now, for each dimension, compute its known bits, and add the total offset
    // to the base KB.
    //
    // For example, if `%base` is [4 x [256 x {i8, i8}]], and we have an
    // expression like `&base[N][2].1`, then
    //
    //   KB(&(%base)[0:%N:512][0:2:1].1)
    //     -> KB(%base)
    //        + (KB(%N) - KB(0))  * 512
    //        + (KB( 2) - KB(0))  *   1 + KB(&{i8, i8}[1])
    //
    // Accumulate constant offsets separately so they can be effiently added all
    // at once after non-constant offsets are processed.
    int64_t AccConstOffset = 0;
    for (int I = Sub->getNumDimensions() - 1; I >= 0; --I) {
      // If the computed offset becomes unknown at any point, bail out --
      // further offsets will only result in more unknown bits.
      if (KB.isUnknown())
        return;

      const auto Dim = Sub->dim(I);

      // First, handle any struct offsets (which are always constant)
      AccConstOffset += (int64_t)computeStructOffset(Dim.DimElementType,
                                                     Dim.StructOffsets, Q.DL);

      // Next, compute known bits for the lower bound and index.
      const auto LowerKB = computeKnownBits(Dim.LowerBound, Depth, Q);
      auto IndexKB = computeKnownBits(Dim.Index, Depth, Q);

      // If the lower bound and index are zero, the whole offset is zero, and we
      // can skip this dimension.
      if (LowerKB.isZero() && IndexKB.isZero())
        continue;

      // Now, compute known bits for the element's stride. Using this, the
      // index, and the lower bound, we compute the total dimension offset:
      //
      //   KB(Dim) = (KB(Index) - KB(LowerBound)) * KB(Stride)
      //
      // being careful to sign-extend each quantity, as necessary, to the
      // indexed type's width.
      const auto StrideKB = computeKnownBits(Dim.StrideInBytes, Depth, Q);
      const unsigned IndexedTyBits = Q.DL.getIndexTypeSizeInBits(Dim.DimType);

      // If all quantities involved are constant, compute the constant offset
      // and add it to the accumulated constant offsets.
      if (LowerKB.isConstant() && IndexKB.isConstant() &&
          StrideKB.isConstant()) {
        AccConstOffset += StrideKB.getConstant().getSExtValue() *
                          (IndexKB.getConstant().getSExtValue() -
                           LowerKB.getConstant().getSExtValue());
        continue;
      }

      // At least one of the quantities is non-constant. Handle those here.
      KnownBits DimKB{IndexedTyBits};
      if (LowerKB.isConstant() && StrideKB.isConstant()) {
        // In most cases, the lower bound and stride are constant. As an
        // optimization, handle this case specifically (in case lower is 0 or
        // stride is 1).
        DimKB = std::move(IndexKB);
        computeSubConst(DimKB, LowerKB.getConstant().getSExtValue());
        computeMulConst(DimKB, StrideKB.getConstant().getSExtValue());
      } else {
        // Slow path -- compute using KnownBits interfaces.
        DimKB = KnownBits::mul(StrideKB.sext(IndexedTyBits),
                               KnownBits::computeForAddSub(
                                   /*Add:*/ false, /*NSW:*/ true,
                                   IndexKB.sext(IndexedTyBits),
                                   LowerKB.sext(IndexedTyBits)));
      }

      // KB(Base) += KB(Dim)
      KB =
          KnownBits::computeForAddSub(/*Add:*/ true, /*NSW:*/ false, KB, DimKB);
    }

    // Now add all accumulated constant offsets in one go.
    computeAddConst(KB, AccConstOffset);
  }

  /// Multiply the given KnownBits by a constant (possibly-signed) integer
  /// factor.
  template <typename T> static void computeMulConst(KnownBits &KB, T Val) {
    static_assert(std::is_integral<T>::value,
                  "can only multiply by an integral value");

    // Fold 1 * %KB => %KB
    if (Val == 1)
      return;

    APInt ConstantInt{KB.getBitWidth(), static_cast<uint64_t>(Val),
                      std::is_signed<T>::value};
    KB = KnownBits::mul(KnownBits::makeConstant(ConstantInt), std::move(KB));
  }

  /// Add to/sub from the given KnownBits a constant (possibly-signed) integer
  /// factor.
  template <bool Add, typename T>
  static void computeAddSubConst(KnownBits &KB, T Val) {
    static_assert(std::is_integral<T>::value, "can only add an integral value");
    // Fold %KB +/- 0 => %KB
    if (Val == 0)
      return;
    APInt ConstantInt{KB.getBitWidth(), static_cast<uint64_t>(Val),
                      std::is_signed<T>::value};
    KB = KnownBits::computeForAddSub(/*Add:*/ Add, /*NSW:*/ false,
                                     KnownBits::makeConstant(ConstantInt),
                                     std::move(KB));
  }

  template <typename ConstInt, typename AssumeT>
  static MaybeAlign extractAlignmentFromAssumption(const AssumeT *A,
                                                   unsigned Idx) {
    const auto Bundle = A->getOperandBundleAt(Idx);
    if (Bundle.getTagName() != "align")
      return std::nullopt;

    return cast<ConstInt>(Bundle.Inputs[1])->getAlignValue();
  }

  static MaybeAlign extractAlignmentFromAssumption(
      const VPAssumptionCache::ResultElem &Assumption) {
    if (const auto *AssumeLLVM = dyn_cast<AssumeInst>(Assumption))
      return extractAlignmentFromAssumption<ConstantInt>(AssumeLLVM,
                                                         Assumption.Index);
    if (const auto *VPAssume = dyn_cast<VPCallInstruction>(Assumption))
      return extractAlignmentFromAssumption<VPConstantInt>(VPAssume,
                                                           Assumption.Index);
    return std::nullopt;
  }

  static void computeKnownBitsFromAssume(const VPValue *V, KnownBits &KB,
                                         const Query &Q) {
    // No support for non-ptr values at this time: we only care about alignment
    // assumptions.
    if (!V->getType()->isPointerTy())
      return;

    // Iterate over the assumptions for this value, trying to find alignment
    // assumptions. Stop at the first one we find, and update our known bits
    // to be a multiple of the assumed alignment.
    //
    // NOTE: We assume values will not be affected by more than one alignment
    // assumption.
    for (const auto &Assumption : Q.VPAC.assumptionsFor(V)) {
      if (!isValidAssumeForContext(Assumption, Q))
        continue;

      if (MaybeAlign Assumed = extractAlignmentFromAssumption(Assumption)) {
        KB.Zero.setLowBits(Log2(Assumed.value()));
        return;
      }
    }
  }

  static bool
  isValidAssumeForContext(const VPAssumptionCache::ResultElem &Assume,
                          const Query &Q) {
    // External assumes are always valid.
    if (isa<AssumeInst>(Assume))
      return true;

    // For an internal assume, check that it dominates our context instruction.
    return Q.VPDT.properlyDominates(cast<VPCallInstruction>(Assume), Q.CtxI);
  }

  /// Add to the given KnownBits a constant (possibly-signed) integer factor.
  template <typename T> static void computeAddConst(KnownBits &KB, T Val) {
    return computeAddSubConst</*Add:*/ true>(KB, Val);
  }

  /// Subtract from the given KnownBits a constant (possibly-signed) integer factor.
  template <typename T> static void computeSubConst(KnownBits &KB, T Val) {
    return computeAddSubConst</*Add:*/ false>(KB, Val);
  }

  /// Given a (possibly-empty) set of struct \p Offsets, compute the resulting
  /// offset in bytes, starting from the base \p Ty.
  static uint64_t computeStructOffset(Type *Ty, ArrayRef<unsigned> Offsets,
                                      const DataLayout &DL) {
    uint64_t Total = 0;
    Type *Cursor = Ty;
    for (unsigned Offset : Offsets) {
      auto *STy = cast<StructType>(Cursor);
      Total += DL.getStructLayout(STy)->getElementOffset(Offset);
      Cursor = STy->getStructElementType(Offset);
    }
    return Total;
  }

  static const Instruction *
  tryToGetUnderlyingInst(const VPInstruction *VPInst) {
    // FIXME: CtxI == NULL should probably mean the entry to the main loop.
    return (VPInst && VPInst->isUnderlyingIRValid())
               ? cast<Instruction>(VPInst->getUnderlyingValue())
               : nullptr;
  }
};
} // namespace vpo
} // namespace llvm

KnownBits VPlanValueTracking::getKnownBits(const VPValue *Expr,
                                           const VPInstruction *CtxI) const {
  const auto Q = VPlanValueTrackingImpl::query(this, CtxI);
  const auto KB = VPlanValueTrackingImpl::computeKnownBits(Expr, 0, Q);
  LLVM_DEBUG(dbgs() << "computeKnownBits("; Expr->printAsOperand(dbgs());
             dbgs() << ")\n"; dbgs() << " -> "; KB.print(dbgs());
             dbgs() << '\n');
  return KB;
}

KnownBits VPlanValueTrackingLLVM::getKnownBits(VPlanSCEV *Expr,
                                               const VPInstruction *CtxI) {
  return getKnownBitsImpl(VPSE->toSCEV(Expr),
                          VPlanValueTrackingImpl::tryToGetUnderlyingInst(CtxI));
}

KnownBits VPlanValueTrackingLLVM::getKnownBitsImpl(const SCEV *Scev,
                                                   const Instruction *CtxI) {
  auto BitWidth = DL->getTypeSizeInBits(Scev->getType());

  if (auto *ScevConst = dyn_cast<SCEVConstant>(Scev)) {
    KnownBits KB(BitWidth);
    KB.One = ScevConst->getAPInt();
    KB.Zero = ~KB.One;
    return KB;
  }

  if (auto *ScevUnknown = dyn_cast<SCEVUnknown>(Scev)) {
    Value *V = ScevUnknown->getValue();
    return llvm::computeKnownBits(V, *DL, 0, VPAC->getLLVMCache(), CtxI, DT);
  }

  if (auto *ScevAdd = dyn_cast<SCEVAddExpr>(Scev)) {
    bool NSW = ScevAdd->hasNoSignedWrap();
    KnownBits KB(BitWidth);
    KB.setAllZero();

    for (auto *Op : ScevAdd->operands()) {
      KnownBits OpKB = getKnownBitsImpl(Op, CtxI);
      KB = KnownBits::computeForAddSub(true, NSW, KB, OpKB);
    }

    return KB;
  }

  if (auto *ScevMul = dyn_cast<SCEVMulExpr>(Scev)) {
    // There's no public KnownBits API to compute known bits for the result of
    // multiplication. So, the only thing we do ourselves here is just computing
    // the number of low zero bits.
    int NZero = 0;
    for (auto *Op : ScevMul->operands()) {
      KnownBits OpKB = getKnownBitsImpl(Op, CtxI);
      NZero += OpKB.Zero.countTrailingOnes();
    }

    KnownBits KB(BitWidth);
    KB.Zero.setBits(0, NZero);
    return KB;
  }

  if (auto *ScevPtrToInt = dyn_cast<SCEVPtrToIntExpr>(Scev)) {
    // SCEVPtrToIntExpr should not change the known bits number.
    return getKnownBitsImpl(ScevPtrToInt->getOperand(), CtxI);
  }

  return KnownBits(BitWidth);
}
