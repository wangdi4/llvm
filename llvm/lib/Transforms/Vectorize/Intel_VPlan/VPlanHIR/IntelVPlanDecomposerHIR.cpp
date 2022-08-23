//===-- VPlanDecomposeHIR.cpp ---------------------------------------------===//
//
//   Copyright (C) 2018-2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// TODO: Port AVR documentation.
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanDecomposerHIR.h"
#include "../IntelVPlanIDF.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRParVecAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "vplan-decomposer"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

static cl::opt<bool> VPlanForceInvariantDecomposition(
    "vplan-force-invariant-decomposition", cl::init(false), cl::Hidden,
    cl::desc("Force decomposition of invariants"));

static cl::opt<bool> VPlanAvoidRedundantInst(
    "vplan-avoid-redundant-inst", cl::init(true), cl::Hidden,
    cl::desc("Avoid generating redundant instructions in a basic block"));

// Splice the instruction list of the VPBB where \p Phi belongs, by moving the
// VPPhi instruction to the front of the list
static void movePhiToFront(VPPHINode *Phi) {
  VPBasicBlock *BB = Phi->getParent();
  BB->getInstructions().splice((BB->front()).getIterator(),
                               BB->getInstructions(), Phi->getIterator());
}

VPInstruction *VPDecomposerHIR::getOrCreateNaryOp(unsigned Opcode,
                                                  ArrayRef<VPValue *> Operands,
                                                  Type *BaseTy) {
  if (!VPlanAvoidRedundantInst)
    return cast<VPInstruction>(
        Builder.createNaryOp(Opcode, Operands, BaseTy, nullptr /* DDNode */));

  // Use FoldingSetNodeID to compute a hash from the opcode, base type and the
  // first two operands at most.
  FoldingSetNodeID Id;
  unsigned NumOps = Operands.size();
  Id.AddInteger(Opcode);
  Id.AddPointer(BaseTy);
  Id.AddPointer(Operands[0]);
  if (NumOps > 1)
    Id.AddPointer(Operands[1]);
  else
    Id.AddPointer(nullptr);
  unsigned InstHash = Id.ComputeHash();

  // Search InstrMap to see if we can find an equivalent instruction and reuse
  // the same if one is found.
  auto InstRange = InstrMap.equal_range(InstHash);
  for (auto Start = InstRange.first; Start != InstRange.second; ++Start) {
    VPInstruction *Inst = Start->second;
    if (Inst->getType() != BaseTy)
      continue;
    if (Inst->getOpcode() != Opcode)
      continue;
    if (Inst->getNumOperands() != NumOps)
      continue;
    for (unsigned OpIdx = 0; OpIdx < NumOps; OpIdx++)
      if (Inst->getOperand(OpIdx) != Operands[OpIdx])
        continue;

    // Found a matching instruction to reuse
    return Inst;
  }

  // Create a new instruction for the specified opcode/operands.
  auto *NewInst = cast<VPInstruction>(
      Builder.createNaryOp(Opcode, Operands, BaseTy, nullptr /* DDNode */));

  // We avoid reusing instructions that may have side effects by not adding
  // it to InstrMap.
  if (!NewInst->mayHaveSideEffects())
    InstrMap.insert(std::pair<unsigned, VPInstruction *>(InstHash, NewInst));

  return NewInst;
}

// Creates a decomposed VPInstruction that combines \p LHS and \p RHS VPValues
// using \p OpCode as operator and \p MasterVPI as master VPInstruction. If \p
// LHS or \p RHS is null, it returns the non-null VPValue.
VPValue *VPDecomposerHIR::combineDecompDefs(VPValue *LHS, VPValue *RHS,
                                            Type *Ty, unsigned OpCode) {
  assert((LHS != nullptr || RHS != nullptr) && "LHS and RHS cannot be nullptr");

  if (LHS == nullptr)
    return RHS;
  if (RHS == nullptr)
    return LHS;

  return getOrCreateNaryOp(OpCode, {LHS, RHS}, LHS->getType());
}

// Create a VPConstant for an integer coefficient.
VPConstant *VPDecomposerHIR::decomposeCoeff(int64_t Coeff, Type *Ty) {
  assert((Ty->isIntOrIntVectorTy() || Ty->isPtrOrPtrVectorTy()) &&
         "Expected scalar/vector integer or pointer type for coefficient.");
  // Null value for pointer types needs special treatment
  if (Coeff == 0 && Ty->isPtrOrPtrVectorTy())
    return Plan->getVPConstant(Constant::getNullValue(Ty));

  return Plan->getVPConstant(ConstantInt::getSigned(Ty, Coeff));
}

// Create a VPInstruction with \p Src as source operand, \p ConvOpCode as
// conversion opcode and \p DestType as destination type.
VPInstruction *VPDecomposerHIR::decomposeConversion(VPValue *Src,
                                                    unsigned ConvOpCode,
                                                    Type *DestType) {
  return getOrCreateNaryOp(ConvOpCode, {Src}, DestType);
}

// Create a VPInstruction for the conversion of \p CE, if any, using \p Src as a
// source operand. Return \p Src if \p CE doesn't have conversion.
VPValue *VPDecomposerHIR::decomposeCanonExprConv(CanonExpr *CE, VPValue *Src) {
  if (CE->getDestType() == CE->getSrcType())
    return Src;

  if (CE->isZExt())
    return decomposeConversion(Src, Instruction::ZExt, CE->getDestType());
  if (CE->isSExt())
    return decomposeConversion(Src, Instruction::SExt, CE->getDestType());
  if (CE->isTrunc())
    return decomposeConversion(Src, Instruction::Trunc, CE->getDestType());

  llvm_unreachable("Unsupported conversion in VPlan decomposer!");
}

// Create a VPInstruction for the conversion from \p Src's type to \p DestTy.
VPValue *VPDecomposerHIR::decomposeBlobImplicitConv(VPValue *Src,
                                                    Type *DestTy) {
  Type *SrcTy = Src->getType();
  if (SrcTy == DestTy)
    return Src;

  if (SrcTy->isPointerTy() && DestTy->isIntegerTy())
    return decomposeConversion(Src, Instruction::PtrToInt, DestTy);
  if (SrcTy->isIntegerTy() && DestTy->isPointerTy())
    return decomposeConversion(Src, Instruction::IntToPtr, DestTy);

  llvm_unreachable("Unexpected blob implicit conversion!");
}

// Decompose a blob given its \p BlobIdx and \p BlobCoeff. Return the last
// VPValue resulting from its decomposition.
VPValue *VPDecomposerHIR::decomposeBlob(RegDDRef *RDDR, unsigned BlobIdx,
                                        int64_t BlobCoeff) {
  BlobTy Blob = RDDR->getBlobUtils().getBlob(BlobIdx);
  VPValue *DecompBlob;

  // Avoid invariant blob decomposition if the same is enabled. However, for
  // constant data blobs, and standalone blobs continue regular decomposition.
  if (VPlanForceInvariantDecomposition ||
      RDDR->getBlobUtils().isConstantDataBlob(Blob) ||
      RDDR->isNonDecomposable() || RDDR->getBlobDDRef(BlobIdx) ||
      RDDR->findMaxBlobLevel(BlobIdx) >= OutermostHLp->getNestingLevel()) {
    // Decompose Blob.
    VPBlobDecompVisitor BlobDecomp(*RDDR, *this);
    DecompBlob = BlobDecomp.visit(Blob);
  } else
    DecompBlob = Plan->getVPExternalDefForBlob(RDDR, BlobIdx);

  assert(DecompBlob && "Blob was not decomposed into valid VPValues");

  if (BlobCoeff != 1) {
    // Create VPInstruction for Coeff * blob.
    Type *BlobTy = Blob->getType();
    Type *CoeffType;

    if (BlobTy->isPointerTy()) {
      // If coeff != 1 and blob type is pointer, only -1 coeff is allowed for
      // now.
      assert((BlobCoeff == -1) &&
             "Unexpected blob coefficient for pointer type.");

      unsigned PointerSize =
          RDDR->getDDRefUtils().getDataLayout().getPointerTypeSizeInBits(
              BlobTy);
      switch (PointerSize) {
      case 64:
        CoeffType = Type::getInt64Ty(*Plan->getLLVMContext());
        break;
      case 32:
        CoeffType = Type::getInt32Ty(*Plan->getLLVMContext());
        break;
      default:
        llvm_unreachable("Unexpected pointer size.");
      }

      // Generate pointer-to-int comparison for pointer blob with -1 coefficient
      // (-1 * PtrToInt(p)).
      DecompBlob = decomposeBlobImplicitConv(DecompBlob, CoeffType);
    } else
      CoeffType = Blob->getType();

    VPValue *BlobCoeffValue = decomposeCoeff(BlobCoeff, CoeffType);
    DecompBlob = combineDecompDefs(DecompBlob, BlobCoeffValue, Blob->getType(),
                                   Instruction::Mul);
  }

  return DecompBlob;
}

// Return the HLLoop given an IV level by traversing HLLoops starting from the
// HLLoop parent of \p DDR.
static HLLoop *getHLLoopForLevel(DDRef *DDR, unsigned IVLevel) {
  HLLoop *ParentLp = DDR->getParentLoop();
  assert(ParentLp && "Expected parent HLLoop for DDRef!");

  while (ParentLp->getNestingLevel() != IVLevel) {
    ParentLp = ParentLp->getParentLoop();
    assert(ParentLp && "HLLoop not found for IV level!");
  }

  return ParentLp;
}

// Decompose the IV sub-expression of a CanonExpr. Return the last VPValue
// resulting from its decomposition.
VPValue *VPDecomposerHIR::decomposeIV(RegDDRef *RDDR, CanonExpr *CE,
                                      unsigned IVLevel, Type *Ty) {
  int64_t IVConstCoeff;
  unsigned IVBlobIndex;
  CE->getIVCoeff(IVLevel, &IVBlobIndex, &IVConstCoeff);
  VPValue *DecompIV = nullptr;

  if (IVBlobIndex != InvalidBlobIndex)
    // Create VPInstruction for blob * IV.
    // The blob coefficient 1 because the IV coefficient is IVConstCoeff and
    // it's processed later.
    DecompIV = decomposeBlob(RDDR, IVBlobIndex, 1 /*BlobCoeff*/);

  if (IVConstCoeff != 1) {
    VPValue *DecompIVCoeff = decomposeCoeff(IVConstCoeff, Ty);
    DecompIV = combineDecompDefs(DecompIVCoeff, DecompIV, Ty, Instruction::Mul);
  }

  HLLoop *IVLevelLoop = getHLLoopForLevel(RDDR, IVLevel);
  auto IVTy = IVLevelLoop->getIVType();
  VPValue *VPIndVar = HLLp2IVPhi[IVLevelLoop];
  if (!VPIndVar)
    // If there is no PHI in the map, it means that the IV is an external
    // definition. When getting the external definition for the IV use the
    // IVLevelLoop's IV type.
    // TODO: We could be creating redundant external definitions here because
    // this external definition cannot be mapped to an HLInst. Add check at the
    // beginning of this function to return an existing external definition in
    // the VPlan pool.
    VPIndVar = Plan->getVPExternalDefForIV(IVLevel, IVTy);

  // Add a conversion for VPIndVar if its type does not match canon expr
  // type specified in Ty. We mimic the code from HIR CG here.
  if (Ty != IVTy) {
    assert(Ty->isIntegerTy() && "Expected integer type");
    if (Ty->getPrimitiveSizeInBits() > IVTy->getPrimitiveSizeInBits()) {
      bool IsNSW = OutermostHLp->hasSignedIV();
      VPIndVar = IsNSW ? decomposeConversion(VPIndVar, Instruction::SExt, Ty)
                       : decomposeConversion(VPIndVar, Instruction::ZExt, Ty);
    } else
      VPIndVar = decomposeConversion(VPIndVar, Instruction::Trunc, Ty);

    // Mark that the IV convert needs to be folded into the containing canon
    // expression. This is important for preserving linear canon expressions in
    // generated vector code.
    // TODO - IV tracking for inner loops
    if (IVLevel == OutermostHLp->getNestingLevel())
      cast<VPInstruction>(VPIndVar)->HIR().setFoldIVConvert(true);
  }

  DecompIV = combineDecompDefs(DecompIV, VPIndVar, Ty, Instruction::Mul);
  return DecompIV;
}

// When removing IV and forming external def, we force denominator to 1 and set
// dest type to be the same as source type. Consider the following canon
// expression whose source type is i32 and the destination type is i64(sext).
//
//    (IV1 * C1 * b1 + IV2 * C2 * b2 + IV3 * C3 * b3 + B1 + B2 + C4) / Denom
//
// C1/C2/C3 are constant IV coefficients. C4 is the constant additive. The blob
// coefficients b1/b2 are invariant at level 3 where we are trying to
// vectorize. The goal here is to avoid decomposing the invariant part of this
// canon expression. This is done by removing IV at level 3 as stated above.
// Once this is done, the invariant part of the canon expression looks like the
// following whose source and dest types are i32.
//
//    InvCE = (IV1 * C1 * b1 + IV2 * C2 * b2 + B1 + B2 + C4)
//
// The decomposition for the original CE then looks like the following:
//
//    t1 = mul IV3, C3
//    t2 = mul t1, b3
//    t3 = add InvCE, t2
//    t4 = div t3, Denom
//    origce = sext t4 to i64
//
// Also, consider the following canon expressions where %n1 and %n2 are
// invariant. This also allows us to create one invariant for %n1 + %n2.
//
//     (i1 + %n1 + %n2)  /  9 (source and dest types are i64)
//     i1 + %n1 + %n2         (source type is i64 and dest type is i32)
//
// Remove IV from CE at specified LoopLevel and return the original denominator
// and destination type in DenomP/DestTypeP respectively if non-null.
static void processIVRemoval(CanonExpr *CE, unsigned LoopLevel,
                             int64_t *DenomP = nullptr,
                             Type **DestTypeP = nullptr) {
  if (DenomP)
    *DenomP = CE->getDenominator();
  if (DestTypeP)
    *DestTypeP = CE->getDestType();
  CE->setDenominator(1);
  CE->setDestType(CE->getSrcType());
  CE->removeIV(LoopLevel);
}

// Decompose a CanonExpr. Return the last VPValue resulting from its
// decomposition.
VPValue *VPDecomposerHIR::decomposeCanonExpr(RegDDRef *RDDR, CanonExpr *CE) {

  // Note: the insert location guard also guards builder debug location.
  VPBuilder::InsertPointGuard Guard(Builder);
  LLVM_DEBUG(dbgs() << "  Decomposing CanonExpr: "; CE->dump(); dbgs() << "\n");
  VPValue *DecompDef = nullptr;
  // Set debug locations for all newly created VPIs from decomposition of this
  // CE.
  Builder.setCurrentDebugLocation(CE->getDebugLoc());
  // Return true if we want to force regular decomposition of given
  // canon expression.
  auto forceRegularDecomposition = [](CanonExpr *CE, unsigned VecLoopLevel) {
    // Do regular decomposition if invariant decomposition is forced.
    if (VPlanForceInvariantDecomposition)
      return true;

    // We want to force regular decomposition for canon expressions that are
    // constants or self blobs once we ignore the IV at vectorization loop
    // level. Remove IV if needed and mark for later restore of IV.
    bool RestoreIV = false, UseRegular = false;
    int64_t IVConstCoeff;
    unsigned IVBlobIndex;
    CE->getIVCoeff(VecLoopLevel, &IVBlobIndex, &IVConstCoeff);

    int64_t Denom;
    Type *DestType;
    if (IVConstCoeff) {
      RestoreIV = true;
      processIVRemoval(CE, VecLoopLevel, &Denom, &DestType);
    }

    // Avoid creating an external def for constants - continue generating a
    // VPConstant.
    if (CE->isConstant())
      UseRegular = true;

    // Continue creating a VPBlob external def for standalone blob canon
    // expressions which includes self blobs.
    if (CE->isStandAloneBlob())
      UseRegular = true;

    // Continue creating an VPIndVar if the CE is a stand alone IV.
    if (CE->isStandAloneIV(false /* AllowConversion */))
      UseRegular = true;

    // Restore IV at vectorization loop level if needed.
    if (RestoreIV) {
      CE->addIV(VecLoopLevel, IVBlobIndex, IVConstCoeff);
      CE->setDestType(DestType);
      CE->setDenominator(Denom);
    }

    return UseRegular;
  };

  unsigned VecLoopLevel = OutermostHLp->getNestingLevel();
  bool ForceRegularDecomposition = forceRegularDecomposition(CE, VecLoopLevel);

  // Avoid decomposing invariant canon expressions.
  if (!ForceRegularDecomposition && CE->isInvariantAtLevel(VecLoopLevel))
    return Plan->getVPExternalDefForCanonExpr(CE, RDDR);

  // Check to see if we can avoid decomposition of canon expression portion
  // after removing the IV at level being vectorized. Boolean flag is set to
  // true if we can do so. TODO - revisit for multi level vectorization when
  // we support this in future.
  bool InvariantWithoutIV = false;

  // If the loop-level is max loop-nest level, then there are no more inner
  // loops to check.
  bool IsInvariantAtInnerLoopLevel =
      VecLoopLevel == loopopt::MaxLoopNestLevel
          ? true
          : CE->isInvariantAtLevel(VecLoopLevel + 1);

  // The canon expression is a candidate if we are not forcing regular
  // decomposition for this canon expression, it is linear at vec loop level and
  // invariant at any inner loop level.
  if (!ForceRegularDecomposition && CE->isLinearAtLevel(VecLoopLevel) &&
      IsInvariantAtInnerLoopLevel) {
    auto *CEClone = CE->clone();
    processIVRemoval(CEClone, VecLoopLevel);
    assert(CEClone->isInvariantAtLevel(VecLoopLevel) &&
           "Expected invariant CEClone");

    DecompDef = Plan->getVPExternalDefForCanonExpr(CEClone, RDDR);
    InvariantWithoutIV = true;
  }

  bool IsIdiomCE = Idioms->isCEIdiom(CE);

  // If the canon expression is invariant once we ignore IV at the vectorization
  // level, we do not need to decompose blobs.
  if (!InvariantWithoutIV)
    // Decompose blobs.
    for (auto BlobIt = CE->blob_begin(); BlobIt != CE->blob_end(); ++BlobIt) {
      unsigned BlobIdx = CE->getBlobIndex(BlobIt);
      assert(BlobIdx != InvalidBlobIndex && "Invalid blob index!");

      int64_t BlobCoeff = CE->getBlobCoeff(BlobIdx);
      assert(BlobCoeff != 0 && "Invalid blob coefficient!");

      VPValue *DecompBlob = decomposeBlobImplicitConv(
          decomposeBlob(RDDR, BlobIdx, BlobCoeff), CE->getSrcType());
      DecompDef = combineDecompDefs(DecompDef, DecompBlob, CE->getSrcType(),
                                    Instruction::Add);
    }

  // Decompose IV expression. If the canon expression is invariant without
  // vectorization level IV, we only need to decompose vectorization level
  // IV.
  for (auto IVIt = CE->iv_begin(), E = CE->iv_end(); IVIt != E; ++IVIt) {
    int64_t IVConstCoeff = CE->getIVConstCoeff(IVIt);
    if (IVConstCoeff != 0 &&
        (!InvariantWithoutIV ||
         CE->getLevel(IVIt) == OutermostHLp->getNestingLevel())) {
      assert((!IsIdiomCE ||
              CE->getLevel(IVIt) != OutermostHLp->getNestingLevel()) &&
             "Unexpected IV in compress/expand index");
      VPValue *DecompIV =
          decomposeIV(RDDR, CE, CE->getLevel(IVIt), CE->getSrcType());
      DecompDef = combineDecompDefs(DecompDef, DecompIV, CE->getSrcType(),
                                    Instruction::Add);
    }
  }

  // If the canon expression is invariant without vectorization level IV,
  // the constant additive will be part of the invariant and should be skipped.
  if (!InvariantWithoutIV) {
    // Decompose constant additive. If it's 0, we ignore it when the CE has more
    // components (e.g., X + 0). Otherwise, CE is representing the constant 0
    // and we have to generate a VPValue for it.
    int64_t AddCoeff = CE->getConstant();
    if (AddCoeff != 0 || !DecompDef) {
      VPValue *DecompCoeff = decomposeCoeff(AddCoeff, CE->getSrcType());
      DecompDef = combineDecompDefs(DecompDef, DecompCoeff, CE->getSrcType(),
                                    Instruction::Add);
    }
  }

  // Decompose denominator.
  int64_t Denominator = CE->getDenominator();
  if (Denominator != 1) {
    assert(!IsIdiomCE && "Unexpected denominator in compress/expand index");
    VPValue *DecompDenom = decomposeCoeff(Denominator, CE->getSrcType());
    DecompDef = combineDecompDefs(DecompDef, DecompDenom, CE->getSrcType(),
                                  CE->isUnsignedDiv() ? Instruction::UDiv
                                                      : Instruction::SDiv);
  }

  // Decompose conversions.
  DecompDef = decomposeCanonExprConv(CE, DecompDef);

  assert(DecompDef && "CanonExpr has not been decomposed");
  if (IsIdiomCE)
    addVPValueForCEIdiom(CE, DecompDef);
  return DecompDef;
}

/// Utility to get alignment information attached to given memref. When the ref
/// does not have alignment information we default to the ABI alignment for the
/// ref's type, similar to LLVM behavior.
static Align getAlignForMemref(RegDDRef *Ref) {
  assert(Ref->isMemRef() && "Memref expected to compute alignment.");
  unsigned Alignment = Ref->getAlignment();
  if (!Alignment) {
    auto DL = Ref->getDDRefUtils().getDataLayout();
    Alignment = DL.getABITypeAlignment(Ref->getDestType());
  }

  return Align(Alignment);
}

// Decompose the incoming HIR memory reference \p Ref. Return the last VPValue
// resulting from its decomposition.
//
// The incoming DDRef can either be a LHS or RHS operand for the parent DDNode.
// In case of a RHS operand we generate an additional `load` VPInstruction along
// with decomposing the DDRef into subscript (and bitcast if needed). For a LHS
// operand no additional instruction is generated, since VPInstructions
// representing the RHS must be generated first before generating the `store`.
// This technique also simplifies the decomposition code to handle loads cleaned
// up by HIR's Temp Cleanup pass (example 2). NOTE: If RHS operand is AddressOf
// type then load will not be generated.
//
// Examples -
// 1. %0 = (@b)[0][i1]
//
//    The RHS memory operand will be decomposed as -
//    %vp0 = subscript %b, 0, %vpi1
//    %vpl = load %vp0
//
//    createVPInstruction will later pick the load (always the last generated
//    "operand" for a load HLInst) as master VPI.
//
// 2. %0 = (@b)[0][i1]  +  (@c)[0][i1]
//
//    Both the RHS memory operands will be decomposed as -
//    %vp0 = subscript %b, 0, %vpi1
//    %vp1 = load %vp0
//    %vp2 = subscript %c, 0, %vpi1
//    %vp3 = load %vp2
//
//    createVPInstruction will be responsible for generating the `add`
//    VPInstruction using %vp1 and %vp3 as operands and set as master VPI.
//
// 3. (@a)[0][i1] = %0
//
//    The LHS memory operand will be decomposed as -
//    %vp0 = subscript %a, 0, %vpi1
//
//    createVPInstruction will then generate a `store` VPInstruction using %vp0
//    as operand.
//
VPValue *VPDecomposerHIR::decomposeMemoryOp(RegDDRef *Ref) {
  // Check if Ref is related to VConflict idiom. In legality, we mark the store
  // instructions, but we cannot do the same for load instructions since the
  // load memory references might not be in separate instructions. For this
  // reason, in legality, we mark the DDRef that includes VConflict load memory
  // references. Here, we collect load instructions along with their index.
  bool IsVConflictLoad = Idioms->isVConflictLoad(Ref);

  // Note: the insert location guard also guards builder debug location.
  VPBuilder::InsertPointGuard Guard(Builder);
  LLVM_DEBUG(dbgs() << "VPDecomp: Decomposing memory operand: "; Ref->dump();
             dbgs() << "\n");

  assert(Ref->hasGEPInfo() &&
         "Expected a GEP RegDDRef to decompose memory operand.");

  // Final VPInstruction obtained on decomposing the MemOp
  VPValue *MemOpVPI;
  Builder.setCurrentDebugLocation(Ref->getGepDebugLoc());
  VPValue *DecompBaseCE = decomposeCanonExpr(Ref, Ref->getBaseCE());

  LLVM_DEBUG(dbgs() << "VPDecomp: Memop DecompBaseCE: "; DecompBaseCE->dump();
             dbgs() << "\n");

  unsigned NumDims = Ref->getNumDimensions();
  assert(NumDims > 0 && "Number of dimensions in memory operand is 0.");

  // If Ref is of the form a[0] or &a[0], subscript instructions are not needed.
  // TODO: This is needed for correctness of any analysis with opaque types.
  // However HIR codegen should be aware of this lack of subscript for Refs of
  // form a[0] and &a[0] and generate them if needed during codegen. For example
  // check Transforms/Intel_VPO/Vecopt/hir_vector_opaque_type.ll
  if (NumDims == 1 && !Ref->hasTrailingStructOffsets() &&
      (*Ref->canon_begin())->isZero())
    MemOpVPI = DecompBaseCE;
  else {
    // Determine resulting type of the subscript instruction
    //
    // Example: float a[1024];
    //
    // @a[0][i] will be decomposed as -
    // float* %vp = subscript [1024 x float]* %a, i64 0, i64 %i
    //
    // Here -
    // SubscriptResultType = float*
    //
    // NOTE: Type information about pointer type or pointer element type can be
    // retrieved anytime from the pointer operand of subscript instruction.
    //

    Type *SubscriptResultType = Ref->getSrcType();

    // For store/load references we need to convert to PointerType
    if (!Ref->isAddressOf())
      SubscriptResultType =
          PointerType::get(SubscriptResultType, Ref->getPointerAddressSpace());

    SmallVector<VPSubscriptInst::DimInfo, 4> Dimensions;
    for (unsigned I = NumDims; I > 0; --I) {
      VPValue *DecompLower = decomposeCanonExpr(Ref, Ref->getDimensionLower(I));
      VPValue *DecompStride =
          decomposeCanonExpr(Ref, Ref->getDimensionStride(I));
      VPValue *DecompIndex = decomposeCanonExpr(Ref, Ref->getDimensionIndex(I));

      if (IsVConflictLoad) {
        assert(NumDims == 1 &&
               "VConflict is only supported for one-dimensional arrays.");
        // Add the conflicting index in the map.
        RefToConflictingIndex[Ref] = DecompIndex;
      }

      LLVM_DEBUG(dbgs() << "VPDecomp: Memop DecompLower: "; DecompLower->dump();
                 dbgs() << "\n");
      LLVM_DEBUG(dbgs() << "VPDecomp: Memop DecompStride: ";
                 DecompStride->dump(); dbgs() << "\n");
      LLVM_DEBUG(dbgs() << "VPDecomp: Memop DecompIndex: "; DecompIndex->dump();
                 dbgs() << "\n");

      // Get trailing struct offsets for dimension.
      auto HIRDimOffsets = Ref->getTrailingStructOffsets(I);

      Dimensions.emplace_back(I - 1, DecompLower, DecompStride, DecompIndex,
                              Ref->getDimensionType(I),
                              Ref->getDimensionElementType(I), HIRDimOffsets);
    }
    auto *Subscript = Builder.create<VPSubscriptInst>(
        "subscript", SubscriptResultType, DecompBaseCE, Dimensions);
    Subscript->setIsInBounds(Ref->isInBounds());
    MemOpVPI = Subscript;
  }

  // Create a bitcast instruction if needed
  auto *BitCastDestElemTy = Ref->getBitCastDestVecOrElemType();
  if (BitCastDestElemTy &&
      BitCastDestElemTy->getContext().supportsTypedPointers()) {
    LLVM_DEBUG(dbgs() << "VPDecomp: BitCastDestElemTy: ";
               BitCastDestElemTy->dump(); dbgs() << "\n");
    MemOpVPI = Builder.createNaryOp(
        Instruction::BitCast, {MemOpVPI},
        PointerType::get(BitCastDestElemTy, Ref->getBaseCE()
                                                ->getDestType()
                                                ->getScalarType()
                                                ->getPointerAddressSpace()));
  }

  // If memory reference is AddressOf type, return the last generated
  // VPInstruction
  if (Ref->isAddressOf())
    return MemOpVPI;

  Builder.setCurrentDebugLocation(Ref->getMemDebugLoc());

  if (Ref->isRval()) {
    // If memory reference is an RVal, then it corresponds to a load. Create a
    // new load VPInstruction to represent it.
    assert(cast<PointerType>(MemOpVPI->getType())
               ->isOpaqueOrPointeeTypeMatches(Ref->getDestType()) &&
           "Incompatible types!");
    MemOpVPI = Builder.createLoad(Ref->getDestType(), MemOpVPI);

    // Copy metadata for the created load instruction.
    auto *MemOpVPInst = cast<VPLoadStoreInst>(MemOpVPI);
    MemOpVPInst->readUnderlyingMetadata(Ref);

    if (IsVConflictLoad)
      // Add newly created load instruction to the map that we keep VConflict
      // load instructions.
      RefToVPLoadConflict[Ref] = MemOpVPInst;

    // Save away scalar memref symbase and original alignment for later use.
    MemOpVPInst->HIR().setSymbase(Ref->getSymbase());
    MemOpVPInst->setAlignment(getAlignForMemref(Ref));

    // FIXME: This special-casing for loads that are HLInst is becoming more
    // complicated than expected since we also have to special-case
    // createVPInstruction. I think that special-case this code to avoid
    // creating the VPInstruction load for load HLInsts is starting to make more
    // sense now.
    // Attach the DDRef definition operand for loads that are not standalone
    // HLInst. Standalone loads are handled in createVPInstruction.
    auto *HInst = dyn_cast<HLInst>(Ref->getHLDDNode());
    if (!HInst || !isa<LoadInst>(HInst->getLLVMInstruction()) ||
        HInst->getRvalDDRef() != Ref)
      cast<VPInstruction>(MemOpVPI)->HIR().setOperandDDR(Ref);
  }

  LLVM_DEBUG(dbgs() << "VPDecomp: MemOpVPI: "; MemOpVPI->dump();
             dbgs() << "\n");

  if (Idioms->isCEIdiom(Ref))
    addVPValueForCEIdiom(Ref, MemOpVPI);

  return MemOpVPI;
}

// Decompose the RegDDRef operand of an HLDDNode. Return the last VPValue
// resulting from its decomposition.
VPValue *VPDecomposerHIR::decomposeVPOperand(RegDDRef *RDDR) {
  assert(RDDR && "Expected a valid RegDDRef.");
  if (RDDR->isTerminalRef())
    return decomposeCanonExpr(RDDR, RDDR->getSingleCanonExpr());

  // Memory ops
  return decomposeMemoryOp(RDDR);
}

// Utility function that returns a CmpInst::Predicate for a given HLInst
// representing a CmpInst. Return value is CmpInst's opcode. NOTE: Decomposition
// of HLIf nodes currently don't use this utility function
static CmpInst::Predicate getPredicateFromHIR(HLInst *HInst) {
  assert(isa<CmpInst>(HInst->getLLVMInstruction()) && "Expected CmpInst.");
  return cast<CmpInst>(HInst->getLLVMInstruction())->getPredicate();
}

// Return true if \p Def is considered an external definition. An external
// definition is a definition that happens outside of the outermost HLLoop,
// including its preheader and exit.
bool VPDecomposerHIR::isExternalDef(const DDRef *DDR) {
  // TODO: We are pushing outermost loop PH and Exit outside of the VPlan region
  // for now so this code won't be valid until we bring them back. return
  // !Def->getHLNodeUtils().contains(OutermostHLp, Def,
  //                                 true /*include preheader/exit*/);
  return OutermostHLp->isLiveIn(DDR->getSymbase());
}

// Utility function to determine if the live-in external definition of ref \p
// UseDDR should be accounted for. This function assumes that \p UseDDR is
// live-in to the loop, whose data dependencies are represented in \p DDG.
static bool useLiveInDef(DDRef *UseDDR, const loopopt::DDGraph &DDG) {
  assert(UseDDR->isRval() && "Live-in use analysis is only for RValue refs.");
  auto InEdges = DDG.incoming(UseDDR);

  // For every reaching definition of UseDDR within the loop (DDG is constructed
  // only for loop being vectorized), check if it kills the live-in definition
  // of UseDDR. This happens when the source node of the dependency edge
  // strictly dominates the sink node where UseDDR is used.
  for (auto *Edge : InEdges) {
    HLDDNode *SrcNode = Edge->getSrc()->getHLDDNode();
    HLDDNode *SinkNode = UseDDR->getHLDDNode();
    if (HLNodeUtils::strictlyDominates(SrcNode, SinkNode))
      return false;
  }

  return true;
}

// Return the number of reaching definitions for \p DDR. Reaching definitions
// are computed based on the edges of the DDGraph of the outermost loop that we
// are representing, plus one, if \p DDR is a live-in value of the outermost
// loop.
unsigned VPDecomposerHIR::getNumReachingDefinitions(DDRef *UseDDR) {
  assert(UseDDR->isRval() && "DDRef must be an RValue!");

  if (UseDDR->isMetadata())
    // Metadata operands has a single definition since they are globally defined
    // only once.
    return 1;

  auto BlobInEdges = DDG.incoming(UseDDR);
  auto NumEdges = std::distance(BlobInEdges.begin(), BlobInEdges.end());

  if (isExternalDef(UseDDR))
    return NumEdges + useLiveInDef(UseDDR, DDG);
  else
    return NumEdges;
}

// Return a pointer to the last VPInstruction of \p VPBB (before terminator
// instruction). Return nullptr if \p VPBB is empty (or has only terminator
// instruction).
static VPInstruction *getLastVPI(VPBasicBlock *VPBB) {
  if (VPBB->empty())
    return nullptr;
  if (VPBB->size() == 1) {
    assert(isa<VPBranchInst>(VPBB->begin()));
    return nullptr;
  }
  return &*std::prev(VPBB->terminator());
}

// Set \p MasterVPI as master VPInstruction of all the decomposed VPInstructions
// between \p LastVPIBeforeDec and the own MasterVPI. If LastVPIBeforeDec is
// null, the first decomposed VPInstruction is the first one in \p VPBB.
void VPDecomposerHIR::setMasterForDecomposedVPIs(
    VPInstruction *MasterVPI, VPInstruction *LastVPIBeforeDec,
    VPBasicBlock *VPBB) {
  assert(MasterVPI->getParent() == VPBB && "MasterVPI must be in VPBB.");
  assert((!LastVPIBeforeDec || LastVPIBeforeDec->getParent() == VPBB) &&
         "LastVPIBeforeDec must be in VPBB.");

  VPBasicBlock::iterator DecompVPIStart =
      LastVPIBeforeDec == nullptr
          ? VPBB->begin()
          : std::next(VPBasicBlock::iterator(LastVPIBeforeDec));
  VPBasicBlock::iterator DecompVPIEnd = VPBasicBlock::iterator(MasterVPI);
  for (auto &DecompVPI : make_range(DecompVPIStart, DecompVPIEnd)) {
    // DecompVPI is a new VPInstruction at this point. To be marked as
    // decomposed VPInstruction with the following 'setMaster'.
    assert(DecompVPI.isNew() && "Expected new VPInstruction!");
    DecompVPI.HIR().setMaster(MasterVPI);
  }
}

VPValue *VPDecomposerHIR::getVPValueForNode(const loopopt::HLNode *Node) {
  if (auto DDNode = dyn_cast<HLDDNode>(Node)) {
    auto VPValIt = HLDef2VPValue.find(DDNode);
    if (VPValIt != HLDef2VPValue.end())
      return VPValIt->second;
  }
  return nullptr;
}

// Create VPInstruction for \p Node and insert it in VPBuilder's insertion
// point. If \p Node is an HLIf, we create VPCmpInsts to handle multiple
// predicates. HLLoop are not expected.
VPInstruction *
VPDecomposerHIR::createVPInstruction(HLNode *Node,
                                     ArrayRef<VPValue *> VPOperands) {
  assert(Node && "Expected Node to create a VPInstruction.");
  assert(!isa<HLLoop>(Node) && "HLLoop shouldn't be processed here!");

  // Create VPCmpInst for HLInst representing a CmpInst.
  VPInstruction *NewVPInst;
  auto DDNode = cast<HLDDNode>(Node);

  // There should't be any VPValue for Node at this point. Otherwise, we
  // visited Node when we shouldn't, breaking the RPO traversal order.
  assert(!HLDef2VPValue.count(DDNode) && "Node shouldn't have been visited.");

  // Generate a VPInstruction using LLVMInst opcode and the operands in
  // VPOperands. DDNode if non-null will be used to setup the underlying
  // node. HInst is used to obtain instruction specific information when
  // needed.
  auto genVPInst = [this](const Instruction *LLVMInst, HLDDNode *DDNode,
                          HLInst *HInst, ArrayRef<VPValue *> VPOperands) {
    VPInstruction *NewVPInst;
    if (HInst->isCopyInst()) {
      // Handle HIR copy instruction.
      assert(VPOperands.size() == 1 &&
             "Invalid number of operands for copy instruction.");
      NewVPInst = Builder.createHIRCopy(VPOperands[0], DDNode);
    } else if (isa<CmpInst>(LLVMInst)) {
      assert(VPOperands.size() == 2 && "Expected 2 operands in CmpInst.");
      CmpInst::Predicate CmpPredicate = getPredicateFromHIR(HInst);
      // NOTE: Decomposer::createCmpInst wrapper isn't used here because all the
      // underlying metadata will be obtained by processing of the original
      // HLInst which is a standalone Cmp operation.
      NewVPInst = Builder.createCmpInst(CmpPredicate, VPOperands[0],
                                        VPOperands[1], DDNode);
    } else if (isa<SelectInst>(LLVMInst)) {
      // Handle HLInst idioms such as Abs.
      if (HInst->isAbs()) {
        assert(VPOperands.size() == 1 &&
               "Invalid number of operands for abs instruction.");
        NewVPInst = Builder.createAbs(VPOperands[0], DDNode);
      } else {
        // Handle decomposition of generic select instruction
        assert(VPOperands.size() == 4 &&
               "Invalid number of operands for HIR select instruction.");

        VPValue *CmpLHS = VPOperands[0];
        VPValue *CmpRHS = VPOperands[1];
        VPValue *TVal = VPOperands[2];
        VPValue *FVal = VPOperands[3];

        // Decompose first 2 operands into a CmpInst used as predicate for
        // select
        VPCmpInst *Pred = createCmpInst(HInst->getPredicate(), CmpLHS, CmpRHS);
        // Set underlying DDNode for the select VPInstruction since it may be
        // the master VPInstruction which will be the case if DDNode is
        // non-null.
        NewVPInst = cast<VPInstruction>(Builder.createNaryOp(
            Instruction::Select, {Pred, TVal, FVal}, TVal->getType(), DDNode));
      }
    } else if (isa<LoadInst>(LLVMInst)) {
      // No-op behavior for load nodes since the VPInstruction is already added
      // by decomposeMemoryOp. Check comments in decomposeMemoryOp definition
      // for more details.
      assert(VPOperands.size() == 1 &&
             "Load instruction must have single operand only.");
      NewVPInst = cast<VPInstruction>(VPOperands.back());
      assert(NewVPInst->getOpcode() == Instruction::Load &&
             "Incorrect instruction added for load.");
      // Set the underlying DDNode for the load instruction since it may be
      // master VPI for this node which will be the case if DDNode is non-null.
      if (DDNode)
        NewVPInst->HIR().setUnderlyingNode(DDNode);
    } else if (isa<StoreInst>(LLVMInst)) {
      // Decompose store instruction into VPLoadStoreInst.
      assert(VPOperands.size() == 2 &&
             "Store instruction must have 2 operands only.");
      NewVPInst = Builder.createStore(VPOperands[0], VPOperands[1], DDNode);
    } else if (auto *Call = dyn_cast<CallInst>(LLVMInst)) {
      if (auto *IntrinCall = dyn_cast<IntrinsicInst>(Call)) {
        if (IntrinCall->getIntrinsicID() == Intrinsic::intel_subscript) {
          NewVPInst = cast<VPSubscriptInst>(VPOperands[0]);
          // Make subscript the master instruction since it was already created.
          NewVPInst->HIR().setUnderlyingNode(DDNode);
          return NewVPInst;
        }
      }

      assert(HInst->isCallInst() && "Underlying HLInst expected to be call.");
      VPValue *CalledValue;
      // For indirect calls, called value (function) is the last operand
      // DDRef.
      unsigned ArgOperandOffset = 0;
      if (HInst->isIndirectCallInst()) {
        CalledValue = VPOperands.back();
        ArgOperandOffset = 1;
      } else {
        Function *F = Call->getCalledFunction();
        assert(F && "Call HLInst does not have called function.");
        CalledValue = Plan->getVPConstant(F);
      }
      SmallVector<VPValue *, 4> ArgList(VPOperands.begin(),
                                        VPOperands.end() - ArgOperandOffset);
      NewVPInst = Builder.createCall(
          CalledValue, ArgList, HInst /*Used to get underlying call*/,
          DDNode /*Used to determine if this VPCall is master/slave*/);
    } else if (isa<GetElementPtrInst>(LLVMInst)) {
      // Don't create an additional single operand no-op GEP here. Re-use the
      // subscript instruction that was already created during decomposition of
      // corresponding memref.
      assert(VPOperands.size() == 1 &&
             "HLInst with underlying GEP is expected to have single operand.");
      NewVPInst = cast<VPSubscriptInst>(VPOperands[0]);
      // Make subscript the master instruction since it was already created.
      NewVPInst->HIR().setUnderlyingNode(DDNode);
    } else {
      // Generic VPInstruction.
      NewVPInst = cast<VPInstruction>(Builder.createNaryOp(
          LLVMInst->getOpcode(), VPOperands, LLVMInst->getType(), DDNode));
    }

    // Capture operator flags like FastMathFlags, overflowing flags (nsw/nuw)
    // and exact flag.
    // TODO: Check other parts of decomposer where operator instructions can be
    // emitted. Example NUW/NSW flags from CanonExprs (in future?).
    NewVPInst->copyOperatorFlagsFrom(LLVMInst);

    return NewVPInst;
  };

  if (auto *HInst = dyn_cast<HLInst>(Node)) {
    // Note: the insert location guard also guards builder debug location.
    VPBuilder::InsertPointGuard Guard(Builder);
    const Instruction *LLVMInst = HInst->getLLVMInstruction();
    assert(LLVMInst && "Missing LLVM Instruction for HLInst.");
    // Set debug location for VPInstruction generated for given HLInst.
    Builder.setCurrentDebugLocation(HInst->getDebugLoc());

    // Any LLVM instruction which semantically has a terminal lval/rval can
    // alternatively contain a memref operand in HIR. For example, an add
    // instruction can look like the following in HIR:
    //        A[i] = B[i] + C[i].
    // Rval memory references are handled properly when we create corresponding
    // VPOperands. However, instructions such as the above, need to be
    // decomposed into two separate instructions. In this case, LLVMInst will be
    // an add instruction and VPOperands will contain VPValues corresponding to
    // B[i], C[i], and &A[i]. We generate an add instruction using the first two
    // VPValues. The result of add becomes the value being stored and we
    // generate a store instruction using the result of the add and the VPValue
    // corresponding to &A[i]. The generated store instruction is set as the
    // master instruction.
    RegDDRef *LvalDDR = HInst->getLvalDDRef();

    if (LvalDDR && LvalDDR->isMemRef() &&
        LLVMInst->getOpcode() != Instruction::Store) {
      VPInstruction *StoreVal =
          genVPInst(LLVMInst, nullptr /* DDNode is only set on master */, HInst,
                    VPOperands.drop_back() /* Omit last operand */);
      NewVPInst = Builder.createStore(StoreVal, VPOperands.back(), DDNode);
    } else
      NewVPInst = genVPInst(LLVMInst, DDNode, HInst, VPOperands);

    if (LvalDDR) {
      // Set Lval DDRef as VPOperandHIR for this VPInstruction. This includes
      // standalone loads.
      NewVPInst->HIR().setOperandDDR(LvalDDR);

      // Save away scalar memref symbase and original alignment for later use.
      if (NewVPInst->getOpcode() == Instruction::Store) {
        assert(LvalDDR->isMemRef() &&
               "Expected Lval of a store HLInst to be a memref");
        NewVPInst->HIR().setSymbase(LvalDDR->getSymbase());
        cast<VPLoadStoreInst>(NewVPInst)->setAlignment(
            getAlignForMemref(LvalDDR));
      }

      if (OutermostHLp->isLiveOut(LvalDDR->getSymbase())) {
        VPExternalUse *User =
            Plan->getExternals().getOrCreateVPExternalUseForDDRef(LvalDDR);
        User->addOperand(NewVPInst);
      }
    } else if (RegDDRef *RvalDDR = HInst->getRvalDDRef())
      // Set single Rval as VPOperandHIR for HLInst without Lval DDRef.
      NewVPInst->HIR().setOperandDDR(RvalDDR);

    if (Idioms->isCEIdiom(HInst))
      addVPValueForCEIdiom(HInst, NewVPInst);

  } else if (auto *HIf = dyn_cast<HLIf>(DDNode))
    // Handle decomposition of HLIf node.
    NewVPInst = createVPInstsForHLIf(HIf, VPOperands);
  else
    llvm_unreachable("Unexpected DDNode!");

  HLDef2VPValue[DDNode] = NewVPInst;
  return NewVPInst;
}

// A specialized method to handle decomposition of HLIf nodes in HIR. The input
// \p HIf is expected to be an HLIf and \p VPOperands is a list of operands
// corresponding to each predicate in the HLIf. This method creates a new
// VPCmpInst for each predicate and combines multiple predicates using the
// implicit AND operator. The last VPInstruction created by this function is
// returned and it is set as MasterVPI for the other instructions created for
// this HLIf node.
VPInstruction *
VPDecomposerHIR::createVPInstsForHLIf(HLIf *HIf,
                                      ArrayRef<VPValue *> VPOperands) {
  // Note: the insert location guard also guards builder debug location.
  VPBuilder::InsertPointGuard Guard(Builder);
  // Check if number of operands for a HLIf is twice the number of its
  // predicates
  assert((HIf->getNumPredicates() * 2 == VPOperands.size()) &&
         "Incorrect number of operands for a HLIf");
  Builder.setCurrentDebugLocation(HIf->getDebugLoc());

  auto FirstPred = HIf->pred_begin();
  // Create a new VPCmpInst corresponding to the first predicate
  VPInstruction *CurVPInst =
      createCmpInst(*FirstPred, VPOperands[0], VPOperands[1]);

  LLVM_DEBUG(dbgs() << "VPDecomp: First Pred VPInst: "; CurVPInst->dump());

  unsigned OperandIt = 2;

  // Create VPInstructions for remaining predicates in the HLIf
  for (auto It = HIf->pred_begin() + 1, E = HIf->pred_end(); It != E; ++It) {
    assert(OperandIt + 1 < VPOperands.size() &&
           "Out-of-range access on VPOperands.");
    // Create a new VPCmpInst for the current predicate
    VPInstruction *NewVPInst =
        createCmpInst(*It, VPOperands[OperandIt], VPOperands[OperandIt + 1]);

    LLVM_DEBUG(dbgs() << "VPDecomp: NewVPInst: "; NewVPInst->dump());
    // Conjoin the new VPInst with current VPInst using implicit AND
    CurVPInst = cast<VPInstruction>(Builder.createAnd(CurVPInst, NewVPInst));
    LLVM_DEBUG(dbgs() << "VPDecomp: CurVPInst: "; CurVPInst->dump());
    // Increment OperandIt for next predicate
    OperandIt += 2;
  }

  assert(CurVPInst && "No VPInstruction generated for HLIf?");

  // Set underlying HLDDNode for current VPInst since it's the last created
  // instruction for decomposition of this HLIf
  CurVPInst->HIR().setUnderlyingNode(HIf);
  return CurVPInst;
}

// Return a sequence of VPValues (VPOperands) that represents Node's operands
// in VPlan. In addition to the RegDDRef to VPValue translation, operands are
// sorted in the way VPlan expects them. Some operands, such as the LHS operand
// in some HIR instructions, are ignored because they are not explicitly
// represented as an operand in VPlan.
void VPDecomposerHIR::createVPOperandsForMasterVPInst(
    HLNode *Node, SmallVectorImpl<VPValue *> &VPOperands) {
  auto DDNode = dyn_cast<HLDDNode>(Node);
  // Nothing to create for non-HLDDNode.
  if (!DDNode)
    return;

  HLInst *HInst = dyn_cast<HLInst>(DDNode);
  bool ProcessRvalOps = true;
  if (HInst && isa<SelectInst>(HInst->getLLVMInstruction())) {
    if (HInst->isAbs()) {
      // If we are dealing with an HLInst that is computing the absolute value
      // (a select instruction of the form t = v1 < 0 ? -v1 : v1), we only
      // decompose the first rval operand and generate an Abs VPInstruction.
      VPOperands.push_back(decomposeVPOperand(HInst->getOperandDDRef(1)));
      ProcessRvalOps = false;
    } else if (HInst->isMax() || HInst->isMin()) {
      // When decomposing a min/max HLInst, we can avoid decomposing the 3rd and
      // 4th operands of the instruction by reusing the 1st and 2nd operands
      // appropriately. This reduces number of VPInstructions and also helps
      // preserve min/max form in generated vector code.
      RegDDRef *Op1, *Op2, *Op3, *Op4;
      Op1 = HInst->getOperandDDRef(1);
      Op2 = HInst->getOperandDDRef(2);
      Op3 = HInst->getOperandDDRef(3);
      Op4 = HInst->getOperandDDRef(4);
      (void)Op4;

      VPValue *VPOp1 = decomposeVPOperand(Op1);
      VPValue *VPOp2 = decomposeVPOperand(Op2);
      VPOperands.push_back(VPOp1);
      VPOperands.push_back(VPOp2);

      // Op1 is expected to be equal to Op3 or Op4. Op2 is expected to be equal
      // to Op3 or Op4. Example min/max HLInsts:
      //     min = t1 < t2 ? t1 : t2
      //     max = t1 < t2 ? t2 : t1
      if (DDRefUtils::areEqual(Op1, Op3)) {
        // Push VPOp1 as 3rd VPValue operand
        VPOperands.push_back(VPOp1);
        // Op2 and Op4 should match
        assert(DDRefUtils::areEqual(Op2, Op4) &&
               "Inconsistent min/max operands");
        // Push VPOp2 as 4th VPValue operand
        VPOperands.push_back(VPOp2);
      } else {
        assert(DDRefUtils::areEqual(Op2, Op3) &&
               "Inconsistent min/max operands");
        // Push VPOp2 as 3rd VPValue operand
        VPOperands.push_back(VPOp2);
        assert(DDRefUtils::areEqual(Op1, Op4) &&
               "Inconsistent min/max operands");
        // Push VPOp1 as 4th VPValue operand
        VPOperands.push_back(VPOp1);
      }
      ProcessRvalOps = false;
    }
  }

  // Decompose Rval operands if not processed already.
  if (ProcessRvalOps)
    // Collect operands necessary to build a VPInstruction out of an HLInst
    // and translate them into VPValue's.
    for (RegDDRef *HIROp :
         make_range(DDNode->op_ddref_begin(), DDNode->op_ddref_end())) {
      // We skip LHS operands for all instructions. Lval for stores is handled
      // later.
      if (HIROp->isLval())
        continue;

      VPOperands.push_back(decomposeVPOperand(HIROp));
    }

  if (RegDDRef *Lval = DDNode->getLvalDDRef()) {
    // Insert the Lval operand of store for decomposition to the end. The Lval
    // HIR operand of a store is identified by checking if the Lval of a
    // DDNode is memref.
    //     HLInst: (%A)[i1] = %add;
    //     VPInstruction: store %add, %decompAi1
    if (Lval->isMemRef())
      VPOperands.push_back(decomposeVPOperand(Lval));
  }
}

// Create or retrieve an existing VPValue that represents the definition of the
// use \p UseDDR (R-value DDRef representing a use). Return an external
// definition if such definition happens outside of the outermost loop
// represented in VPlan. Otherwise, return a VPInstruction representing the
// definition.
void VPDecomposerHIR::getOrCreateVPDefsForUse(
    DDRef *UseDDR, SmallVectorImpl<VPValue *> &VPDefs) {

  assert(UseDDR->isRval() && "DDRef must be an RValue!");

  // Process metadata definitions.
  MetadataAsValue *MDAsValue;
  if (UseDDR->isMetadata(&MDAsValue))
    VPDefs.push_back(Plan->getVPMetadataAsValue(MDAsValue));

  // Process external definitions. Add live-in definition only if it is valid.
  if (isExternalDef(UseDDR) && useLiveInDef(UseDDR, DDG)) {
    VPExternalDef *ExtDef = Plan->getVPExternalDefForDDRef(UseDDR);
    VPDefs.push_back(ExtDef);
    // If UseDDR represents an external definition then update the corresponding
    // HIRLegality descriptor (linear/reduction), if any, that it is being
    // directly used inside the loop.
    HIRLegality.recordPotentialSIMDDescrUse(UseDDR);
  }

  // Process definitions coming from incoming DD edges. At this point, all
  // the sources of the incoming edges of UseDDR must have an associated
  // VPInstruction modeling the definition.
  auto InEdges = DDG.incoming(UseDDR);
  for (const DDEdge *Edge : InEdges) {
    // Get the HLDDNode causing the definition.
    HLDDNode *DefNode = Edge->getSrc()->getHLDDNode();

    auto VPValIt = HLDef2VPValue.find(DefNode);
    if (VPValIt == HLDef2VPValue.end()) {
      // Changing hard-assert.  This is because in 'decomposeStandAloneBlob' we
      // create a VPPHINode without any operands. We might not have all the
      // operands, but we have at least one operand at that time. We are peeking
      // into the operand list and just getting the BaseTy of that operand.
      // Deleting the hard-assert allows that.
      assert(getNumReachingDefinitions(UseDDR) > 1 &&
             "Missing VPInstruction for HLDDNode!");
      continue;
    }
    VPDefs.push_back(VPValIt->second);
  }
}

void VPDecomposerHIR::createLoopIVAndIVStart(HLLoop *HLp, VPBasicBlock *LpPH) {
  assert((HLp->isDo() || HLp->isDoMultiExit()) && HLp->isNormalized() &&
         "Only normalized single-exit DO loops are supported for now.");
  assert(LpPH->getSingleSuccessor() &&
         "Loop PH must have one successor VPBasicBlock.");
  VPBasicBlock *LpH = LpPH->getSingleSuccessor();

  // Create IV start (0). Only normalized loops are expected.
  CanonExpr *LowerCE = HLp->getLowerCanonExpr();
  assert(LowerCE->isZero() && "Expected normalized IV.");
  assert(LowerCE->getDestType() == HLp->getIVType() &&
         "Lower bound and IV type doesn't match.");
  VPConstant *IVStart = Plan->getVPConstant(
      ConstantInt::getSigned(LowerCE->getDestType(), LowerCE->getConstant()));

  // Create induction phi only with IVStart. We will add IVNext in a separate
  // step. Insert it at the beginning of the loop header and map it to the loop
  // level. HLp is set as underlying HIR of the induction phi.
  VPBuilder::InsertPointGuard Guard(Builder);
  setInsertPoint(LpH, LpH->begin());
  // Base type for the VPPHINode is obtained from IVStart
  Type *BaseTy = IVStart->getType();
  VPPHINode *IndVPPhi = Builder.createPhiInstruction(BaseTy, HLp);
  IndVPPhi->addIncoming(IVStart, LpPH);
  assert(!HLLp2IVPhi.count(HLp) && "HLLoop has multiple IVs?");
  HLLp2IVPhi[HLp] = IndVPPhi;
  IndVPPhi->HIR().setValid();
}

VPValue *VPDecomposerHIR::createLoopIVNextAndBottomTest(HLLoop *HLp,
                                                        VPBasicBlock *LpPH,
                                                        VPBasicBlock *LpLatch) {
  // This is an example of the master and decomposed VPInstructions that are
  // generated for an HLLoop by this method:
  // HLLoop:
  //   <26> + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
  //   ...
  //   <26> + END LOOP
  //
  // Output:
  //   BB3: // Loop PH
  //    %vp43808 = sext %vp43744 // UB decomposition.
  //    %vp44016 = add %vp43808 i64 -1 // UB decomposition (master VPI)
  //   SUCCESSORS(1):BB4
  //
  //   BB4: // Loop H and Latch
  //    %vp40944 = phi [ i64 0, BB3 ], [ i64 %vp43488, BB4]
  //    ...
  //    %vp43488 = add %vp40944 i64 1 // IVNext (master VPI)
  //    %vp44352 = icmp %vp43488 %vp44016 // BottomTest (master VPI)
  //   SUCCESSORS(2):BB4(%vp44352), BB5(!%vp44352)

  // Retrieve the inductive PHI (IV) generated for this HLLoop.
  assert(HLLp2IVPhi.count(HLp) && "Expected VPPHINode for HLLoop.");
  VPPHINode *IndVPPhi = HLLp2IVPhi[HLp];

  // Create add VPInstruction for IV next. HLp is set as underlying HIR of the
  // created VPInstruction. Only normalized loops are expected so we use step 1.
  assert(HLp->getStrideCanonExpr()->isOne() &&
         "Expected positive unit-stride HLLoop.");
  // Note: the insert location guard also guards builder debug location.
  VPBuilder::InsertPointGuard Guard(Builder);
  setInsertPoint(LpLatch);
  Builder.setCurrentDebugLocation(HLp->getCmpDebugLoc());
  VPConstant *One =
      Plan->getVPConstant(ConstantInt::getSigned(HLp->getIVType(), 1));
  auto *IVNext = cast<VPInstruction>(Builder.createAdd(IndVPPhi, One, HLp));

  // Add IVNext to induction PHI.
  IndVPPhi->addIncoming(IVNext, LpLatch);

  // Get the IV range.
  CanonExpr *LowerCE = HLp->getLowerCanonExpr();
  CanonExpr *UpperCE = HLp->getUpperCanonExpr();
  int64_t Lower;
  int64_t Upper;
  VPValue *StartVal = nullptr;
  VPValue *EndVal = nullptr;
  if (LowerCE->isIntConstant(&Lower))
    StartVal = Plan->getVPConstant(ConstantInt::get(HLp->getIVType(), Lower));

  if (UpperCE->isIntConstant(&Upper))
    EndVal = Plan->getVPConstant(ConstantInt::get(HLp->getIVType(), Upper));

  // Add to the induction descriptors. Push it at the beginning as main
  // induction.
  std::unique_ptr<VPInductionHIRList> &IndList = Inductions[HLp];
  if (!IndList)
    IndList.reset(new VPInductionHIRList);
  IndList->insert(
      IndList->begin(),
      std::make_unique<VPInductionHIR>(
          IVNext, One,
          Plan->getVPConstant(Constant::getNullValue(HLp->getIVType())),
          StartVal, EndVal));

  // Create VPValue for bottom test condition. If decomposition is needed:
  //   1) decompose UB operand. Decomposed VPInstructions are inserted into the
  //      loop PH since they are loop invariant,
  //   2) create master VPInstruction for the bottom test condition with IVNext
  //      and decomposed UB as operands, and,
  //   3) set the last created VPInstruction for UB as master VPInstruction for
  //      that UB group of decomposed VPInstructions.
  assert(HLp->getUpperDDRef() && "Expected a valid upper DDRef for HLLoop.");

  // Keep last instruction before decomposition. We will need it to set the
  // master VPInstruction of all the created decomposed VPInstructions.
  VPInstruction *LastVPIBeforeDec = getLastVPI(LpPH);
  VPValue *DecompUB;
  { // #1. This scope is for Guard (RAII).
    VPBuilder::InsertPointGuard Guard(Builder);
    setInsertPoint(LpPH);
    DecompUB = decomposeVPOperand(HLp->getUpperDDRef());
    // Increment UB value by 1 since HLLoop upper bounds are inclusive. This
    // allows to avoid off-by-one errors during vector TC computation and use
    // stricter predicate for backedge condition.
    if (auto *ConstUB = dyn_cast<VPConstant>(DecompUB))
      DecompUB = Plan->getVPConstant(ConstantExpr::getAdd(
          ConstUB->getConstant(), ConstantInt::get(ConstUB->getType(), 1)));
    else
      DecompUB = Builder.createAdd(
          DecompUB,
          Plan->getVPConstant(ConstantInt::get(DecompUB->getType(), 1)));
  }
  bool UBInstsGenerated = LastVPIBeforeDec != getLastVPI(LpPH);

  // #2.
  // Get the predicate for the HLLoop bottom test condition.
  // HLLoop upper-bound is inclusive and the UB used in VPlan will be
  // incremented by 1 for valid TC computation. So we return the proper
  // less-than predicate based on the sign bit of the comparison type.
  // TODO: Does HIR perform any normalization regarding sign/unsigned types?
  CmpInst::Predicate CmpPredicate =
      cast<IntegerType>(HLp->getLowerCanonExpr()->getDestType())->getSignBit()
          ? CmpInst::ICMP_SLT
          : CmpInst::ICMP_ULT;
  auto *BottomTest = Builder.createCmpInst(CmpPredicate, IVNext, DecompUB, HLp);

  if (UBInstsGenerated)
    if (auto *DecompUBVPI = dyn_cast<VPInstruction>(DecompUB)) {
      // #3. Turn last decomposed VPInstruction of UB as master VPInstruction of
      // the decomposed group.
      DecompUBVPI->HIR().setUnderlyingNode(HLp);

      // Set DecompUBVPI as master VPInstruction of any other decomposed
      // VPInstruction of UB.
      setMasterForDecomposedVPIs(DecompUBVPI, LastVPIBeforeDec, LpPH);
      DecompUBVPI->HIR().setValid();
    }

  // Set the underlying HIR of the new VPInstructions (and its potential
  // decomposed VPInstructions) to valid.
  IndVPPhi->HIR().setValid();
  IVNext->HIR().setValid();
  BottomTest->HIR().setValid();
  return BottomTest;
}

VPInstruction *VPDecomposerHIR::createLoopZtt(HLLoop *HLp,
                                              VPBasicBlock *ZttBlock) {
  VPBuilder::InsertPointGuard Guard(Builder);
  setInsertPoint(ZttBlock);
  Builder.setCurrentDebugLocation(HLp->getDebugLoc());

  // Keep last instruction before decomposition. We will need it to set the
  // master VPInstruction of all the created decomposed VPInstructions.
  VPInstruction *LastVPIBeforeDec = getLastVPI(ZttBlock);

  VPInstruction *CombinedVPInst = nullptr;
  auto PredBegin = HLp->ztt_pred_begin();
  auto PredEnd = HLp->ztt_pred_end();
  for (auto PredIt = PredBegin; PredIt != PredEnd; ++PredIt) {
    // Generate VPValues for the LHS and RHS of the DDRefs corresponding
    // to PredIt and generate Cmp instruction using predicate from PredIt.
    auto *VPOp1 =
        decomposeVPOperand(HLp->getLHSZttPredicateOperandDDRef(PredIt));
    auto *VPOp2 =
        decomposeVPOperand(HLp->getRHSZttPredicateOperandDDRef(PredIt));
    auto *CurVPInst = createCmpInst(*PredIt, VPOp1, VPOp2);

    // Combine using 'And' into CombinedVPInst if non-null
    if (CombinedVPInst)
      CombinedVPInst =
          cast<VPInstruction>(Builder.createAnd(CombinedVPInst, CurVPInst));
    else
      CombinedVPInst = CurVPInst;
  }

  assert(CombinedVPInst && "No VPInstruction generated for Ztt");

  // Set underlying HLDDNode for combined VPInst since it's the last created
  // instruction for decomposition of this HLIf
  CombinedVPInst->HIR().setUnderlyingNode(HLp);

  // Set CombinedVPInst as master VPInstruction of any decomposed VPInstruction
  // resulting from decomposing ztt.
  setMasterForDecomposedVPIs(CombinedVPInst, LastVPIBeforeDec, ZttBlock);
  return CombinedVPInst;
}

VPPHINode *VPDecomposerHIR::getOrCreateEmptyPhiForDDRef(Type *PhiTy,
                                                        VPBasicBlock *VPBB,
                                                        DDRef *DDR) {
  // Build the key pair to look-up PhisToFix map
  PhiFixMapKey VPBBSymPair = std::make_pair(VPBB, DDR->getSymbase());

  auto VPPhiMapIt = PhisToFix.find(VPBBSymPair);
  // If a VPPhi node was already created for this sink DDRef's Symbase in
  // current VPBB, then reuse that.
  if (VPPhiMapIt != PhisToFix.end())
    return (VPPhiMapIt->second).first;

  // If no entry is found in PhisToFix then create a new VPPhi node and add it
  // to the map
  VPBuilder::InsertPointGuard Guard(Builder);
  setInsertPoint(VPBB, VPBB->begin());
  auto *VPPhi = Builder.createPhiInstruction(PhiTy);
  PhisToFix[VPBBSymPair] = std::make_pair(VPPhi, DDR);

  LLVM_DEBUG(dbgs() << "Adding a new empty PHI node for:\n");
  LLVM_DEBUG(dbgs() << "Symbase: " << DDR->getSymbase() << "\n");
  LLVM_DEBUG(dbgs() << "DDR: "; DDR->dump(); dbgs() << "\n");

  // Add the Symbase of sink DDRef to be tracked for fixing PHI nodes
  TrackedSymbases.insert(DDR->getSymbase());

  // Add the type of the PHI node that was added for this tracked Symbase
  if (TrackedSymTypes.count(DDR->getSymbase())) {
    assert(TrackedSymTypes[DDR->getSymbase()] == PhiTy &&
           "Different type PHI node was inserted for same symbase.");
  } else {
    TrackedSymTypes[DDR->getSymbase()] = PhiTy;
  }

  // Add entry to map the PHI node to the sink DDRef Symbase it was generated
  // for
  assert(PhiToSymbaseMap.find(VPPhi) == PhiToSymbaseMap.end() &&
         "The PHI node is already mapped to a Symbase?");
  PhiToSymbaseMap[VPPhi] = DDR->getSymbase();

  return VPPhi;
}

/// Determine which blocks the current symbase \p CurSymbase is live in.
///
///  These are the blocks that lead to actual use of the symbase. This knowlegde
///  will help us avoid inserting PHI nodes into blocks without any uses (no
///  dead PHI nodes)
void VPDecomposerHIR::computeLiveInBlocks(
    unsigned CurSymbase, const SmallPtrSetImpl<VPBasicBlock *> &DefBlocks,
    const SmallPtrSetImpl<VPBasicBlock *> &UsingBlocks,
    SmallPtrSetImpl<VPBasicBlock *> &LiveInBlocks) {
  // Liveness of the current symbase is determined by iterating over
  // predecessors of the blocks where definition is live. These blocks are
  // tracked via a worklist

  // Initially this worklist contains all the using blocks of the current
  // symbase
  SmallVector<VPBasicBlock *, 16> LiveInBlockWorklist(UsingBlocks.begin(),
                                                     UsingBlocks.end());

  // If a VPBB is both a defining and using block of the symbase, then we need
  // to check if the definition comes before or after the use. If definition
  // happens before use, the symbase is not really live-in to VPBB
  for (unsigned I = 0, E = LiveInBlockWorklist.size(); I != E; ++I) {
    VPBasicBlock *VPBB = LiveInBlockWorklist[I];
    if (!DefBlocks.count(VPBB))
      continue;

    // VPBB has both use and definition of symbase, iterate over it's VPIs to
    // find out which comes first
    for (auto &VPI : *VPBB) {
      // Underlying HIR is not attached to non-master VPInstructions
      if (!VPI.HIR().isMaster())
        continue;
      // We don't need to analyze non DDNode nodes like HLGoto
      if (!isa<loopopt::HLDDNode>(VPI.HIR().getUnderlyingNode()))
        continue;

      HLDDNode *DDNode = cast<loopopt::HLDDNode>(VPI.HIR().getUnderlyingNode());
      // We reverse iterate over the DDRefs of the node to handle reduction
      // scenarios. For example - %t1 = %5 + %t1 If current symbase is for
      // %t1, forward iterating over DDRefs would incorrectly recognize it
      // as def before use and hence not live-in, while it is actually live
      HLDDNode::reverse_ddref_iterator DDRIt = std::find_if(
          DDNode->op_ddref_rbegin(), DDNode->op_ddref_rend(),
          [&](const RegDDRef *Ref) {
            // For Lval refs and SelfBlob Rval refs, we can get the Symbase
            // directly
            if (Ref->getSymbase() == CurSymbase)
              return true;
            // If ref is Rval which is not SelfBlob, then iterate over its Blobs
            // to find any use of CurSymbase
            // TODO: Is the second check redundant? Can we have non SelfBlob
            // refs in Lval?
            if (!Ref->isSelfBlob() && DDNode->isRval(Ref)) {
              for (auto BlobI = Ref->blob_begin(), BlobE = Ref->blob_end();
                   BlobI != BlobE; ++BlobI) {
                if ((*BlobI)->getSymbase() == CurSymbase)
                  return true;
              }
            }
            return false;
          });

      // If we find a RegDDRef, then liveness decision can be made. Lval refs
      // simulate store (or def) scenario, while Rval refs simulate a use
      // scenario. If def happens before use, then symbase is not live-in, else
      // it is actually live.
      if (DDRIt != DDNode->op_ddref_rend()) {
        const RegDDRef *RDDR = *DDRIt;
        if (DDNode->isLval(RDDR)) {
          // We are seeing a definition to this symbase before any use of it, so
          // the symbase is not really live in this VPBB
          LiveInBlockWorklist[I] = LiveInBlockWorklist.back();
          LiveInBlockWorklist.pop_back();
          --I;
          --E;
        }
        break;
      }
    }
  }

  // Now we have a list of blocks where the current symbase is actually live-in.
  // Recursively add their predecessors, until we find the full region where the
  // symbase is live.
  while (!LiveInBlockWorklist.empty()) {
    VPBasicBlock *VPBB = LiveInBlockWorklist.pop_back_val();

    // Insert the VPBB into the proper LiveInBlocks set. If it already in the
    // set then we have also processed its predecessors
    if (!LiveInBlocks.insert(VPBB).second)
      continue;

    // Add predecessors of VPBB unless they are defining blocks of current
    // symbase
    for (VPBasicBlock *Pred : VPBB->getPredecessors()) {
      if (DefBlocks.count(Pred))
        continue;

      // Predecessor is not a defining block, so symbase is live in there too
      LiveInBlockWorklist.push_back(Pred);
    }
  }
}

// 1. Do an RPOT traversal, and collect defining VPBBs for each tracked symbase
//    Map:
//    Symbase1 --> [VPBB1, VPBB2]
//
// 2. Iterate over the map and call IDF to find IDFPHIBlocks. If an entry in
//    IDFPHIBlocks does not already have a PHI for the current Symbase, then
//    add it.
void VPDecomposerHIR::addIDFPhiNodes() {
  DenseMap<unsigned, SmallPtrSet<VPBasicBlock *, 8>> SymbaseDefBlocks;
  DenseMap<unsigned, SmallPtrSet<VPBasicBlock *, 8>> SymbaseUsingBlocks;
  Plan->computeDT();
  VPDominatorTree &DT = *(Plan->getDT());
  VPBasicBlock *PlanEntry = &Plan->getEntryBlock();

  ///// Populate use-def blocks of each tracked symbase //////

  // Initialize all keys for the Symbase(Def|Using)Blocks map. If Symbase has an
  // external def then the PlanEntry block is noted as one of its defining block
  for (auto Sym : TrackedSymbases) {
    if (Plan->getVPExternalDefForSymbase(Sym)) {
      SymbaseDefBlocks[Sym] = {PlanEntry};
    } else {
      SymbaseDefBlocks[Sym] = {};
    }

    SymbaseUsingBlocks[Sym] = {};
  }

  // For using blocks we can reuse information available in the PhiToFix map,
  // since we know that Decomposer adds an empty PHI node only for an ambiguous
  // use of the Symbase
  for (auto PhiMapIt : PhisToFix) {
    unsigned Sym = PhiMapIt.first.second;
    assert(TrackedSymbases.count(Sym) && "Untracked symbase in PhisToFix.");
    SymbaseUsingBlocks[Sym].insert(PhiMapIt.first.first);
  }

  // For defining blocks we need to traverse the HCFG and check the Lval of each
  // underlying HIR nodes of each VPBB
  for (VPBasicBlock *VPBB : depth_first(PlanEntry)) {
    for (auto &VPI : *VPBB) {
      if (!VPI.HIR().isMaster())
        continue;
      // We don't need to analyze non DDNode nodes like HLGoto
      if (!isa<loopopt::HLDDNode>(VPI.HIR().getUnderlyingNode()))
        continue;

      HLDDNode *DDNode = cast<loopopt::HLDDNode>(VPI.HIR().getUnderlyingNode());
      if (DDNode->hasLval()) {
        unsigned Sym = DDNode->getLvalDDRef()->getSymbase();
        if (TrackedSymbases.count(Sym)) {
          LLVM_DEBUG(dbgs() << "DDNode: "; DDNode->dump();
                     dbgs() << "  defines the symbase: " << Sym << "\n");
          SymbaseDefBlocks[Sym].insert(VPBB);
        }
      }
    }
  }

  assert(SymbaseDefBlocks.size() == TrackedSymbases.size() &&
         "Should find defining blocks for all tracked symbases.");
  assert(SymbaseUsingBlocks.size() == TrackedSymbases.size() &&
         "Should find using blocks for all tracked symbases.");

  auto PrintDefUseBlocks = [&](raw_ostream &OS) {
    OS << "\nSymbaseDefBlocks:\n";
    for (auto MapIt : SymbaseDefBlocks) {
      OS << "Symbase " << MapIt.first << " -> [";
      for (auto B : MapIt.second) {
        OS << " " << B->getName() << " ";
      }
      OS << "]\n";
    }
    OS << "\nSymbaseUsingBlocks:\n";
    for (auto MapIt : SymbaseUsingBlocks) {
      OS << "Symbase " << MapIt.first << " -> [";
      for (auto B : MapIt.second) {
        OS << " " << B->getName() << " ";
      }
      OS << "]\n";
    }
  };

  LLVM_DEBUG(PrintDefUseBlocks(dbgs()));
  (void)PrintDefUseBlocks;

  // For each tracked symbase compute blocks where the Symbase is actually live
  // in. These are the blocks that lead to uses. Use DefBlocks and LiveInBlocks
  // to run IDF and determine additional PHI nodes that are needed in the HCFG
  for (auto Sym : TrackedSymbases) {
    VPlanForwardIDFCalculator IDF(DT);
    SmallPtrSet<VPBasicBlock *, 8> SymLiveInBlocks;
    SmallVector<VPBasicBlock *, 8> IDFPHIBlocks;

    assert(!SymbaseDefBlocks[Sym].empty() &&
           "Tracked symbase has no defining blocks.");
    assert(!SymbaseUsingBlocks[Sym].empty() &&
           "Tracked symbase has no defining blocks.");
    computeLiveInBlocks(Sym, SymbaseDefBlocks[Sym], SymbaseUsingBlocks[Sym],
                        SymLiveInBlocks);

    LLVM_DEBUG(dbgs() << "\n Results from IDF calculator: \n");

    // NOTE: SymLiveInBlocks can be empty for cases when DDG is inaccurate and
    // we place a unnecessary PHI node
    for (auto LIB : SymLiveInBlocks) {
      LLVM_DEBUG(dbgs() << "VPDecomp: " << LIB->getName()
                        << " is a live-in block for the tracked symbase " << Sym
                        << "\n");
      (void)LIB;
    }

    IDF.setDefiningBlocks(SymbaseDefBlocks[Sym]);
    IDF.setLiveInBlocks(SymLiveInBlocks);

    IDF.calculate(IDFPHIBlocks);

    for (VPBasicBlock *NewPhiBB : IDFPHIBlocks) {
      LLVM_DEBUG(dbgs() << "VPDecomp: IDF decided to add a PHI in "
                        << NewPhiBB->getName() << " for the tracked symbase "
                        << Sym << "\n");
      std::pair<VPBasicBlock *, unsigned> VPBBSymPair =
          std::make_pair(NewPhiBB, Sym);
      if (PhisToFix.find(VPBBSymPair) == PhisToFix.end()) {
        // IDF suggests to add a new PHI node in NewPhiBB basic block, no entry
        // was found in PhisToFix

        VPBuilder::InsertPointGuard Guard(Builder);
        setInsertPoint(NewPhiBB, NewPhiBB->begin());

        assert(TrackedSymTypes.count(Sym) &&
               "PHI type for tracked symbase not found.");
        auto *NewIDFPHI = Builder.createPhiInstruction(TrackedSymTypes[Sym]);
        // Add the new empty PHI to list of phis to fix
        PhisToFix[VPBBSymPair] = std::make_pair(NewIDFPHI, nullptr);

        LLVM_DEBUG(dbgs() << "Adding a new empty IDF PHI node for:\n");
        LLVM_DEBUG(dbgs() << "Symbase: " << Sym << "\n");

        // Update PhiToSymbaseMap
        PhiToSymbaseMap[NewIDFPHI] = Sym;
      }
    }
  }
}

void VPDecomposerHIR::createExitPhisForExternalUses(VPBasicBlock *ExitBB) {
  // TODO: Live-outs for multi-exit loops cannot be handled correctly because of
  // missing explicit representation for early-exit loops.
  if (OutermostHLp->isDoMultiExit())
    return;

  for (auto *ExtUse : Plan->getExternals().getVPExternalUsesHIR()) {
    const VPOperandHIR *HIROp = ExtUse->getOperandHIR();
    assert(HIROp && "Cannot find HIR operand for external use.");
    auto *HIROpBlob = cast<VPBlob>(HIROp);
    DDRef *DDR = const_cast<DDRef *>(HIROpBlob->getBlob());

    auto *ExitPhi = getOrCreateEmptyPhiForDDRef(ExtUse->getType(), ExitBB, DDR);
    LLVM_DEBUG(dbgs() << "Empty PHI was created for live out temp: ";
               DDR->dump();
               dbgs() << " in VPBB: " << ExitBB->getName() << "\n");
    (void)ExitPhi;
  }
}

// Add operands to VPInstructions representing PHI nodes inserted by HIR
// decomposer. PhisToFix represents a map of empty PHI nodes which need to be
// fixed. We implement a dataflow analysis algorithm which tracks the VPValue of
// ambiguous Symbases (corresponding to sink DDRefs) inside each VPBasicBlock of
// the HCFG. The dataflow analysis is described in detail in the function
// fixPhiNodePass.
void VPDecomposerHIR::fixPhiNodes() {
  LLVM_DEBUG(dbgs() << "New PHI node fix algorithm in progress...\n");

  LLVM_DEBUG(dbgs() << "The HIR being decomposed is:\n");
  LLVM_DEBUG(OutermostHLp->dump());
  LLVM_DEBUG(dbgs() << "\n");

  addIDFPhiNodes();

  // If there are no PHIs to fix, bypass the fixPhiNodePass
  if (PhisToFix.empty())
    return;

  // If the Symbase corresponding to a PHI is live-out then make the PHI
  // live-out by creating a VPExternalUse for it. This may not be completely
  // accurate since there could be post-dominating VPInstructions updating the
  // same Symbase. This will however be handled by fixExternalUses transform at
  // end of decomposer.
  for (auto PhiIt : PhisToFix) {
    unsigned Sym = PhiIt.first.second;
    if (OutermostHLp->isLiveOut(Sym)) {
      VPPHINode *Phi = PhiIt.second.first;
      DDRef *DDR = PhiIt.second.second;
      if (DDR == nullptr)
        DDR = getDDRefForTrackedSymbase(Sym);
      assert(DDR && "Sink DDRef for tracked symbase not found.");
      VPExternalUse *ExtUser =
          Plan->getExternals().getOrCreateVPExternalUseForDDRef(DDR);
      ExtUser->addOperand(Phi);
    }
  }

  // Preparations for fixPhiNodePass

  // 1. Make sure that all new PHI nodes added by decomposition are moved to the
  // top of the VPBB
  for (auto PHIMapIt : PhiToSymbaseMap)
    movePhiToFront(PHIMapIt.first);

  // 2. Set the incoming values of all tracked Symbases to their ExternalDef
  // values before CFG entry. If Symbase has no ExternalDef because of lack of
  // r-val use in the loop, then we create a new ExternalDef using the DDRef
  // associated with the Symbase. NOTE: This guarantees that a Symbase is always
  // defined at the entry block of VPlan CFG. Is this always valid to assume?
  PhiNodePassData::VPValMap VPValues;
  for (auto Sym : TrackedSymbases) {
    LLVM_DEBUG(dbgs() << "Sym: " << Sym << "\n");

    VPValue *ExtDef = Plan->getVPExternalDefForSymbase(Sym);
    if (ExtDef) {
      LLVM_DEBUG(dbgs() << "ExtDef was found for symbase.\n");
    } else {
      LLVM_DEBUG(dbgs() << "Create new ExtDef for symbase using DDRef.\n ");
      DDRef *DDR = getDDRefForTrackedSymbase(Sym);
      assert(DDR && "DDRef not found for tracked symbase.");
      ExtDef = Plan->getVPExternalDefForDDRef(DDR);
    }

    LLVM_DEBUG(dbgs() << "ExtDef: "; ExtDef->dump(); dbgs() << "\n");
    VPValues[Sym] = ExtDef;
  }

  assert(Builder.getInsertBlock() &&
         "Current insertion VPBB for builder cannot be null.");
  VPBasicBlock *PlanEntry = &Plan->getEntryBlock();
  assert(PlanEntry && "Entry VPBB for Plan cannot be null.");
  LLVM_DEBUG(dbgs() << "PlanEntry: "; PlanEntry->getParent()->dump();
             dbgs() << "\n");

  // Walk all VPBasicBlocks in the HCFG of current VPlan and perform the PHI
  // node fixing algorithm, updating the incoming values based on underlying HIR
  SmallVector<PhiNodePassData, 32> PhiNodePassWorkList;
  PhiNodePassWorkList.emplace_back(PlanEntry, nullptr /* No pred to entry*/,
                                   VPValues);

  LLVM_DEBUG(dbgs() << "Starting fixPhiNodePass algorithm\n");

  do {
    PhiNodePassData PNPD = std::move(PhiNodePassWorkList.back());
    PhiNodePassWorkList.pop_back();

    fixPhiNodePass(PNPD.VPBB, PNPD.VPBBPred, PNPD.VPValues,
                   PhiNodePassWorkList);
  } while (!PhiNodePassWorkList.empty());

  // TODO: validate correctness of the PHI nodes after fixing, also set their
  // master VPI's HIR to valid (VPPhi->HIR().getMaster()->HIR().setValid())
  for (auto PhiMapIt : PhisToFix) {
    VPPHINode *FixedPhi = PhiMapIt.second.first;

    // This fixed PHI node might have an empty/null incoming value from one of
    // its predecessors. This could happen due to inaccuracies in DDG. We also
    // consider PHIs with same incoming values along all edges here. In most
    // cases such PHI nodes are not even needed. Following are possible cases :
    // 1. The PHI node has just a single incoming value along all edges.
    //    Solution : We replace all uses of this PHI with its operand, and
    //    remove the PHI.
    // 2. The PHI node has only 2 incoming values and blends itself as incoming
    //    value along one of the edges. For example -
    //      %phi = phi [ %v1, BB1 ], [ %phi, BB2 ]
    //    Solution: Replace all uses of PHI with the actual incoming value, and
    //    remove the PHI. In the above example, we RAUW(%phi, %v1).
    // 3. The PHI node has no incoming value.
    //    Solution : This means there was no definition of the ambiguous symbase
    //    before visiting the VPBB i.e. the VPBB defines this symbase for the
    //    first time. Iterate over the underlying HIR instructions of the VPBB,
    //    get the first HLInst (and VPInstruction)  that defines the symbase,
    //    replace all uses of PHI with this instruction thereby removing the
    //    PHI.

    bool HasSameIncomingValues =
        FixedPhi->getNumIncomingValues() > 0 &&
        llvm::all_of(FixedPhi->incoming_values(), [FixedPhi](VPValue *InV) {
          return InV == FixedPhi->getIncomingValue(0u);
        });
    if (HasSameIncomingValues) {
      // Solution for case 1
      LLVM_DEBUG(dbgs() << "VPDecomp fixPhiNodes : The fixed PHI node will be "
                           "replaced and removed:";
                 FixedPhi->dump(); dbgs() << "\n");
      unsigned Idx = 0;
      // HIR should not be invalidated, we are still building initial HCFG.
      FixedPhi->replaceAllUsesWith(FixedPhi->getIncomingValue(Idx),
                                   false /*InvalidateIR*/);
      FixedPhi->getParent()->eraseInstruction(FixedPhi);
    } else if (FixedPhi->getNumIncomingValues() == 2 &&
               (FixedPhi->getIncomingValue(0u) == FixedPhi ||
                FixedPhi->getIncomingValue(1u) == FixedPhi)) {
      // Solution for case 2
      LLVM_DEBUG(dbgs() << "VPDecomp fixPhiNodes : The fixed PHI node will be "
                           "replaced and removed:";
                 FixedPhi->dump(); dbgs() << "\n");
      unsigned Idx = FixedPhi->getIncomingValue(0u) == FixedPhi ? 1 : 0;
      FixedPhi->replaceAllUsesWith(FixedPhi->getIncomingValue(Idx),
                                   false /*InvalidateIR*/);
      FixedPhi->getParent()->eraseInstruction(FixedPhi);
    } else if (FixedPhi->getNumIncomingValues() == 0) {
      // Solution for case 3
      LLVM_DEBUG(dbgs() << "VPDecomp fixPhiNodes : The fixed PHI node will be "
                           "replaced and removed:";
                 FixedPhi->dump(); dbgs() << "\n");
      HLDDNode *FirstDefNode = nullptr;
      for (auto &VPI : *(FixedPhi->getParent())) {
        if (!VPI.HIR().isMaster())
          continue;

        if (!isa<loopopt::HLDDNode>(VPI.HIR().getUnderlyingNode()))
          continue;

        HLDDNode *DDNode =
            cast<loopopt::HLDDNode>(VPI.HIR().getUnderlyingNode());
        if (DDNode->hasLval()) {
          unsigned Sym = DDNode->getLvalDDRef()->getSymbase();
          if (Sym == PhiToSymbaseMap[FixedPhi]) {
            FirstDefNode = DDNode;
            break;
          }
        }
      }

      assert(FirstDefNode &&
             "First defining HIR node for ambiguous symbase not found.");
      assert(HLDef2VPValue.count(FirstDefNode) &&
             "First defining HIR node not found in HLDef2VPValue map.");

      VPInstruction *FirstDefVPI =
          cast<VPInstruction>(HLDef2VPValue[FirstDefNode]);
      LLVM_DEBUG(dbgs() << "VPDecomp fixPhiNodes:\n FirstDefNode: ";
                 FirstDefNode->dump(); dbgs() << "\n FirstDefVPI: ";
                 FirstDefVPI->dump(); dbgs() << "\n");

      // Replace the PHI node and remove it from HCFG. HIR should not be
      // invalidated, we are still building initial HCFG.
      FixedPhi->replaceAllUsesWith(FirstDefVPI, false /*InvalidateIR*/);
      FixedPhi->getParent()->eraseInstruction(FixedPhi);
    } else {
      // Nothing to do, PHI has been fixed accurately.
      assert(FixedPhi->getNumIncomingValues() ==
                 FixedPhi->getParent()->getNumPredecessors() &&
             "PHI node has incorrect number of operands.");
    }
  }
}

// Empty PHI nodes are populated with appropriate incoming VPValues using a
// dataflow analysis algorithm borrowed from LLVM's mem2reg pass. More details
// about this algorithm in mem2reg can be found in the function
// PromoteMem2Reg::RenamePass().
//
//
// Consider the trivial scenario of if-else branching in HCFG:
//
//
//                      +-----------------+
//                      |             BB0 |
//                      |                 |
//                      |  %t = 0 (%vp1)  |
//            +---------+                 +----------+
//            |         +-----------------+          |
//            |                                      |
//            |                                      |
//            |                                      |
//   +--------v--------+                   +---------v-------+
//   |             BB1 |                   |             BB2 |
//   |                 |                   |                 |
//   |  %t = 2 (%vp2)  |                   |        ...      |
//   |                 |                   |                 |
//   +--------+--------+                   +---------+-------+
//            |                                      |
//            |                                      |
//            |                                      |
//            |         +-----------------+          |
//            |         |             BB3 |          |
//            +-------->+                 +<---------+
//                      |   %t.phi = phi  |
//                      |                 |
//                      +-----------------+
//
//
// In this example, HIR decomposer decides to add an empty PHI node in
// VPBasicBlock BB3 to resolve ambiguous reaching definitions of the DDRef %t in
// BB3. We use a modified DFS algorithm to traverse the HCFG which allows us to
// visit VPBasicBlocks multiple times in order to update incoming VPValues of
// the empty PHI nodes.
//
// Upon visiting a VPBB we first check if it has any PHI nodes that need to be
// fixed and appropriately update its incoming VPValue. Next we walk over the
// HIR instructions (through master VPInstructions) that are represented within
// that VPBB and check if they update any of the Symbases being tracked which is
// then recorded in the map IncomingVPVals. Finally we recurse the algorithm for
// the first successor of VPBB and add the remaining successors to a worklist.
//
//
// Tracing the algorithm for above example -
// 1. Visit BB0
//    VPBB = BB0, Pred = null, IncomingVPVals = { %t : null }
//
// 2. Visit BB1
//    VPBB = BB1, Pred = BB0, IncomingVPVals = { %t : %vp1 }
//
// 3. Visit BB3
//    VPBB = BB3, Pred = BB1, IncomingVPVals = { %t : %vp2 }
//    Update %t.phi = phi [ %vp2, BB1]
//
// 4. Visit BB2
//    VPBB = BB2, Pred = BB0, IncomingVPVals = { %t : %vp1 }
//
// 5. Visit BB3 (again, but only to update PHI node)
//    VPBB = BB3, Pred = BB2, IncomingVPVals = { %t : %vp1 }
//    Update %t.phi = phi [ %vp2, BB1 ], [ %vp1, BB2 ]
//
//
void VPDecomposerHIR::fixPhiNodePass(
    VPBasicBlock *VPBB, VPBasicBlock *Pred,
    PhiNodePassData::VPValMap &IncomingVPVals,
    SmallVectorImpl<PhiNodePassData> &Worklist) {
  LLVM_DEBUG(dbgs() << "\nEntering VPBB: " << VPBB->getName() << " from Pred: "
                    << (Pred ? Pred->getName() : "nullptr") << "\n");
  // First step is updating any PHI nodes in this VPBB, if it's incomplete
  for (VPPHINode &VPN : VPBB->getVPPhis()) {
    if (PhiToSymbaseMap.count(&VPN)) {
      assert(Pred && "VPBB has null predecessor.");

      // Number of incoming edges from Pred to VPBB
      auto Successors = Pred->getSuccessors();
      unsigned NumEdges =
          std::count(Successors.begin(), Successors.end(), VPBB);
      assert(NumEdges && "Atleast one edge must exist from Pred to VPBB.");

      // Find Symbase that this PHI node corresponds to in TrackedSymbases
      unsigned Symbase = PhiToSymbaseMap[&VPN];
      assert(TrackedSymbases.count(Symbase) &&
             "Empty PHI corresponds to a Symbase which is not being tracked.");

      // A single VPBB can have multiple incoming edges from a predecessor VPBB
      // (typically from switch statements). LLVM-IR handles this scenario by
      // introducing redundant PHI node entries like -
      // %phi = phi [%val1, BB0], [%val1, BB0], [%val2, BB1]
      // Hence we add the incoming value for this PHI node from Pred for all
      // edges.
      // TODO: Investigate if this is going to be needed from VPlan/HCFG
      // perspective.
      for (unsigned I = 0; I != NumEdges; ++I) {
        if (!IncomingVPVals[Symbase]) {
          // No incoming value was found for the Symbase along the edge from
          // Pred to current VPBB. This is potentially an incorrect placement of
          // empty PHI node due to inaccuracies in DDG. We dump more details of
          // such DDG here and ignore this edge to continue the analysis.
          // Example -
          // DO_LOOP i = 0, %n , 1
          //   %t = i + 1    <1>
          //   %x = %t + i   <2>
          // END LOOP
          //
          // For above loop, no PHI node is needed for %t, but DDG can
          // potentially state that there are 2 incoming edges to node <2>
          // (assume a more complex loop with large DDG). So the PHI node
          // inserted for %t has only one incoming value, the other would be
          // nullptr and should be ignored.
          LLVM_DEBUG(dbgs() << "Temporary fix to handle problems with DDG!\n");
          LLVM_DEBUG(dbgs() << "Symbase: " << Symbase << "\n");
          loopopt::DDRef *DDR = PhisToFix[std::make_pair(VPBB, Symbase)].second;
          LLVM_DEBUG(dbgs() << "DDRef:"; DDR->dump());
          LLVM_DEBUG(dbgs() << "\nHLNode: \n"; DDR->getHLDDNode()->dump());
          LLVM_DEBUG(dbgs()
                     << "Function name: "
                     << DDR->getDDRefUtils().getFunction().getName() << "\n");
          LLVM_DEBUG(dbgs() << "HLLoop:\n"; OutermostHLp->dump());
          LLVM_DEBUG(dbgs() << "DDG:\n"; DDG.dump());
          (void)DDR;
        } else {
          VPN.addIncoming(IncomingVPVals[Symbase], Pred);
          // Current value for the Symbase is now the result of PHI for control
          // flowing through this VPBB
          IncomingVPVals[Symbase] = &VPN;
        }
      }
    }
  }

  // If VPBB has already been visited, then return immediately
  if (!PhiNodePassVisited.insert(VPBB).second)
    return;

  // Iterate over instructions of VPBB and update the incoming value of Symbases
  // being tracked if any instruction in the VPBB writes into it
  for (auto &VPI : *VPBB) {
    LLVM_DEBUG(dbgs() << "fixPhiNodePass: VPI: "; VPI.dump());

    if (VPI.HIR().isMaster()) {
      if (auto *HInst =
              dyn_cast<loopopt::HLInst>(VPI.HIR().getUnderlyingNode())) {
        // If HLInst has no Lval then ignore the instruction and move to next
        // master VPI
        if (!HInst->hasLval())
          continue;
        unsigned Sym = HInst->getLvalDDRef()->getSymbase();
        LLVM_DEBUG(dbgs() << "fixPhiNodePass: HIR: "; HInst->dump());
        LLVM_DEBUG(dbgs() << "fixPhiNodePass: Sym: " << Sym << "\n");

        // If the Lval DDRef's Symbase is being tracked, then update the
        // IncomingVPVals map to use the corresponding VPI
        if (TrackedSymbases.count(Sym))
          IncomingVPVals[Sym] = &VPI;
      }
    }
  }

  // Recurse the traversal into successors of VPBB in reverse iterator order by
  // adding them to the worklist. The reverse order is done to maintain
  // consistency with the mem2reg version of this traversal algorithm.
  SmallPtrSet<VPBasicBlock *, 8> VisitedSuccs;

  for (VPBasicBlock *CurVPBB : reverse(VPBB->getSuccessors())) {
    // Keep track of successors so that same successor is not added to worklist
    // twice
    if (VisitedSuccs.insert(CurVPBB).second)
      Worklist.emplace_back(CurVPBB, VPBB, IncomingVPVals);
  }
}

// Post-processing initial VPlan CFG to fix VPExternalUses that were created for
// live-out VPInstructions during decomposition. In HIR every VPExternalUse is
// tied to a unique temp/symbase, so it is expected to have only one live-out
// VPInstruction as operand. We use VPlan's PostDomTree to find out this single
// post-dominating live-out VPInstruction for given ExternalUse and fix its
// operands accordingly.
void VPDecomposerHIR::fixExternalUses() {
  // TODO: Live-outs for multi-exit loops cannot be handled correctly because of
  // missing explicit representation for early-exit loops.
  if (OutermostHLp->isDoMultiExit())
    return;

  Plan->computePDT();
  const VPPostDominatorTree *PDT = Plan->getPDT();

  for (auto *ExtUse : Plan->getExternals().getVPExternalUsesHIR()) {
    // If VPExternalUse already has just one operand, then nothing to do.
    if (ExtUse->getNumOperands() == 1)
      continue;

    // Track index of VPExternalUse's operand that post-dominates all other
    // operands.
    int PostDomOpIdx = -1;
    for (unsigned Idx = 0; Idx < ExtUse->getNumOperands(); ++Idx) {
      auto *CurrInst = cast<VPInstruction>(ExtUse->getOperand(Idx));
      bool PostDominates =
          llvm::all_of(ExtUse->operands(), [PDT, CurrInst](VPValue *Op) {
            return PDT->dominates(CurrInst, cast<VPInstruction>(Op));
          });

      if (PostDominates) {
        assert(
            (PostDomOpIdx == -1 ||
             ExtUse->getOperand(Idx) == ExtUse->getOperand(PostDomOpIdx)) &&
            "Multiple different post-dominating operands for VPExternalUse.");
        PostDomOpIdx = Idx;
      }
    }

    assert(PostDomOpIdx != -1 &&
           "Could not find any post-dominating operand for VPExternalUse");

    // Cache the post-dominating operand to be used after clearing operand list.
    VPValue *PostDomOp = ExtUse->getOperand(PostDomOpIdx);
    ExtUse->removeAllOperands();
    ExtUse->addOperand(PostDomOp);
  }
}

VPInstruction *
VPDecomposerHIR::createVPInstructionsForNode(HLNode *Node,
                                             VPBasicBlock *InsPointVPBB) {
  LLVM_DEBUG(dbgs() << "Generating VPInstructions for "; Node->dump();
             dbgs() << "\n");
  // There should't be any VPValue for Node at this point. Otherwise, we
  // visited Node when we shouldn't, breaking the RPO traversal order.
  assert((isa<HLGoto>(Node) || !HLDef2VPValue.count(cast<HLDDNode>(Node))) &&
         "Node shouldn't have been visited.");

  // Don't need to create any new instructions for Goto nodes -- can use
  // existing terminator instead.
  if (isa<HLGoto>(Node))
    return InsPointVPBB->getTerminator();

  // Set the insertion point in the builder for the VPInstructions that we are
  // going to create for this Node.
  setInsertPoint(InsPointVPBB);
  // Keep last instruction before decomposition. We will need it to set the
  // master VPInstruction of all the decomposed VPInstructions created.
  VPInstruction *LastVPIBeforeDec = getLastVPI(InsPointVPBB);

  // Create and decompose the operands of the future new VPInstruction.
  // They will be inserted (obviously) before the new VPInstruction.
  SmallVector<VPValue *, 4> VPOperands;
  createVPOperandsForMasterVPInst(Node, VPOperands);

  // Create new VPInstruction with previous operands.
  VPInstruction *NewVPInst = createVPInstruction(Node, VPOperands);
  if (NewVPInst->getOpcode() == Instruction::Store)
    cast<VPLoadStoreInst>(NewVPInst)->readUnderlyingMetadata();

  // Set NewVPInst as master VPInstruction of any decomposed VPInstruction
  // resulting from decomposing its operands.
  setMasterForDecomposedVPIs(NewVPInst, LastVPIBeforeDec, InsPointVPBB);

  // If this Node is a HLInst, check if it potentially updates any HIRLegality's
  // SIMD descriptors (linear/reduction).
  if (auto *HInst = dyn_cast<HLInst>(Node))
    HIRLegality.recordPotentialSIMDDescrUpdate(HInst);

  // Set the underlying HIR of the new VPInstruction (and its potential
  // decomposed VPInstructions) to valid.
  NewVPInst->HIR().setValid();
  return NewVPInst;
}

// Create a VPValue for a non-integer constant \p Blob. A non-integer constant
// blob can be a floating point or an undef.
VPConstant *VPDecomposerHIR::VPBlobDecompVisitor::decomposeNonIntConstBlob(
    const SCEVUnknown *Blob) {
  BlobUtils &BlUtils = RDDR.getBlobUtils();
  assert((BlUtils.isConstantDataBlob(Blob) ||
          BlUtils.isConstantVectorBlob(Blob)) &&
         "Expected a ConstantData/ConstantVector Blob.");
  (void)BlUtils;

  ConstantFP *FPConst;
  if (BlUtils.isConstantFPBlob(Blob, &FPConst))
    return Decomposer.Plan->getVPConstant(FPConst);

  Constant *VecConst;
  if (BlUtils.isConstantVectorBlob(Blob, &VecConst))
    return Decomposer.Plan->getVPConstant(VecConst);

  if (BlUtils.isUndefBlob(Blob))
    return Decomposer.Plan->getVPConstant(UndefValue::get(Blob->getType()));

  llvm_unreachable("Unsupported non-integer HIR Constant.");
}

// Create a VPValue for a standalone blob given its SCEV. A standalone blob is
// unitary and doesn't need decomposition.
VPValue *VPDecomposerHIR::VPBlobDecompVisitor::decomposeStandAloneBlob(
    const SCEVUnknown *Blob) {

  if (RDDR.getBlobUtils().isConstantDataBlob(Blob) ||
      RDDR.getBlobUtils().isConstantVectorBlob(Blob))
    // Decompose constant blobs that are not integer values.
    return decomposeNonIntConstBlob(Blob);

  // If the RegDDRef is a standalone blob (including metadata), we use the
  // RegDDRef directly in the following steps since there is no BlobDDRef
  // associated to this Blob. Otherwise, we retrieve and use the BlobDDRef.
  DDRef *DDR;
  if (RDDR.isNonDecomposable())
    DDR = &RDDR;
  else {
    unsigned BlobIndex = RDDR.getBlobUtils().findBlob(Blob);
    assert(BlobIndex != InvalidBlobIndex && "SCEV is not a Blob");
    DDR = RDDR.getBlobDDRef(BlobIndex);
    assert(DDR != nullptr && "BlobDDRef not found!");
  }

  unsigned BlobNumReachDefs = Decomposer.getNumReachingDefinitions(DDR);
  assert(BlobNumReachDefs > 0 && "Blob without reaching definitions!");

  // Blob has reaching definitions. We need to retrieve (or create) the VPValues
  // associated to the sources DDRefs (definitions). If there are multiple
  // definitions, in addition, we introduce a VPPHINode that "blends" all the
  // VPValue definitions.
  SmallVector<VPValue *, 2> VPDefs;
  if (BlobNumReachDefs == 1) {
    // Single definition.
    Decomposer.getOrCreateVPDefsForUse(DDR, VPDefs);
    assert(VPDefs.size() == 1 && "Expected single definition.");
    return VPDefs.front();
  } else {
    // The operands of the PHI are not set right now since some of them
    // might not have been created yet. They will be set by fixPhiNodes.
    // Map the corresponding Instruction to <VPBasicBlock, Symbase> in
    // PhisToFix if not found.
    Decomposer.getOrCreateVPDefsForUse(DDR, VPDefs);
    Type *BaseTy = VPDefs.front()->getType();

    auto *VPPhi = Decomposer.getOrCreateEmptyPhiForDDRef(
        BaseTy, Decomposer.Builder.getInsertBlock(), DDR);
    return VPPhi;
  }
}

// Helper function to decomposed an SCEVNAryExpr using the same \p OpCode to
// combine all the sub-expressions.
VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::decomposeNAryOp(const SCEVNAryExpr *Blob,
                                                      unsigned OpCode) {
  VPValue *DecompDef = nullptr;
  Type *ExprTy = Blob->getType();

  // TODO:
  // VPInstruction::UMinSeq is NOT commutative so the original operand order has
  // to be honored as it is implemented for OpCode == VPInstruction::UMinSeq
  // case below.
  //
  // This method handled commutative operations only before UMinSeq was
  // introduced. Historically the code swaps the operands, which is unacceptable
  // for UMinSeq (the default code below). Technically we should be able to
  // remove 'not UMinSeq' code and handle both commutative and non commutative
  // operations with the same code. However VPlan misses canonicalization pass
  // for immediate operands of commutative operation thus such change would
  // produce instructions such as mul 3, %op, i.e. with the first immediate
  // operand.
  //
  // Thereby, before generalizing the code we might want to add canonicalization
  // to avoid possible negative impact on downstream transformations and
  // performance eventually.
  //
  for (auto *SCOp : Blob->operands()) {
    VPValue *VPOp = Decomposer.decomposeBlobImplicitConv(visit(SCOp), ExprTy);
    DecompDef = (OpCode == VPInstruction::UMinSeq) ?
      Decomposer.combineDecompDefs(DecompDef, VPOp, Blob->getType(), OpCode) :
      Decomposer.combineDecompDefs(VPOp, DecompDef, Blob->getType(), OpCode);
  }

  return DecompDef;
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitConstant(
    const SCEVConstant *Constant) {
  return Decomposer.Plan->getVPConstant(Constant->getValue());
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitTruncateExpr(
    const SCEVTruncateExpr *Expr) {
  VPValue *Src = visit(Expr->getOperand());
  return Decomposer.decomposeConversion(Src, Instruction::Trunc,
                                        Expr->getType());
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitZeroExtendExpr(
    const SCEVZeroExtendExpr *Expr) {
  VPValue *Src = visit(Expr->getOperand());
  return Decomposer.decomposeConversion(Src, Instruction::ZExt,
                                        Expr->getType());
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitSignExtendExpr(
    const SCEVSignExtendExpr *Expr) {
  VPValue *Src = visit(Expr->getOperand());
  return Decomposer.decomposeConversion(Src, Instruction::SExt,
                                        Expr->getType());
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitPtrToIntExpr(
    const SCEVPtrToIntExpr *Expr) {
  VPValue *Src = visit(Expr->getOperand());
  return Decomposer.decomposeConversion(Src, Instruction::PtrToInt,
                                        Expr->getType());
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitAddExpr(const SCEVAddExpr *Expr) {
  return decomposeNAryOp(Expr, Instruction::Add);
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitMulExpr(const SCEVMulExpr *Expr) {
  return decomposeNAryOp(Expr, Instruction::Mul);
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitUDivExpr(const SCEVUDivExpr *Expr) {
  VPValue *DivLHS = visit(Expr->getLHS());
  VPValue *DivRHS = visit(Expr->getRHS());
  return Decomposer.combineDecompDefs(DivLHS, DivRHS, Expr->getType(),
                                      Instruction::UDiv);
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitAddRecExpr(
    const SCEVAddRecExpr *Expr) {
  llvm_unreachable("Expected add-recs to be broken by canon-expr.");
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitSMaxExpr(const SCEVSMaxExpr *Expr) {
  return decomposeNAryOp(Expr, VPInstruction::SMax);
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitUMaxExpr(const SCEVUMaxExpr *Expr) {
  return decomposeNAryOp(Expr, VPInstruction::UMax);
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitSMinExpr(const SCEVSMinExpr *Expr) {
  return decomposeNAryOp(Expr, VPInstruction::SMin);
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitUMinExpr(const SCEVUMinExpr *Expr) {
  return decomposeNAryOp(Expr, VPInstruction::UMin);
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitSequentialUMinExpr(
    const SCEVSequentialUMinExpr *Expr) {
  return decomposeNAryOp(Expr, VPInstruction::UMinSeq);
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitUnknown(const SCEVUnknown *Expr) {
  return decomposeStandAloneBlob(Expr);
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitCouldNotCompute(
    const SCEVCouldNotCompute *Expr) {
  llvm_unreachable("Attempt to use a SCEVCouldNotCompute object.");
}
