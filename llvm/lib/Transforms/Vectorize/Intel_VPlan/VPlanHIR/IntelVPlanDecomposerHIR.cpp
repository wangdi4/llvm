//===-- VPlanDecomposeHIR.cpp ---------------------------------------------===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRParser.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "vplan-decomposer"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

static Type *getBaseTypeForSemiPhiOp(ArrayRef<VPValue *> Operands) {
  assert(Operands.size() >= 1 &&
         "Expecting atleast one operand expected for Semi-Phi Op");
  return Operands[0]->getBaseType();
}

// Splice the instruction list of the VPBB where \p Phi belongs, by moving the
// VPPhi instruction to the front of the list
static void moveSemiPhiToFront(VPInstruction *Phi) {
  assert(Phi->getOpcode() == VPInstruction::SemiPhi &&
         "move operation called on a non-semiphi instruction!");
  VPBasicBlock *BB = Phi->getParent();
  BB->getInstList().splice((BB->front()).getIterator(), BB->getInstList(),
                           Phi->getIterator());
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

  auto *NewVPI = cast<VPInstruction>(
      Builder.createNaryOp(OpCode, {LHS, RHS}, LHS->getBaseType()));
  return NewVPI;
}

// Create a VPConstant for an integer coefficient.
VPConstant *VPDecomposerHIR::decomposeCoeff(int64_t Coeff, Type *Ty) {
  assert((Ty->isIntegerTy() || Ty->isPointerTy()) &&
         "Expected integer or pointer type for coefficient.");
  // Null value for pointer types needs special treatment
  if (Coeff == 0 && Ty->isPointerTy())
    return Plan->getVPConstant(Constant::getNullValue(Ty));

  return Plan->getVPConstant(ConstantInt::getSigned(Ty, Coeff));
}

// Create a VPInstruction with \p Src as source operand, \p ConvOpCode as
// conversion opcode and \p DestType as destination type.
VPInstruction *VPDecomposerHIR::decomposeConversion(VPValue *Src,
                                                    unsigned ConvOpCode,
                                                    Type *DestType) {
  auto *NewConv = cast<VPInstruction>(
      Builder.createNaryOp(ConvOpCode, {Src}, DestType));
  return NewConv;
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
  Type *SrcTy = Src->getBaseType();
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

  // Decompose Blob.
  VPBlobDecompVisitor BlobDecomp(*RDDR, *this);
  VPValue *DecompBlob = BlobDecomp.visit(Blob);
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

  VPValue *VPIndVar = HLLp2IVSemiPhi[getHLLoopForLevel(RDDR, IVLevel)];
  if (!VPIndVar)
    // If there is no semi-phi in the map, it means that the IV is an external
    // definition.
    // TODO: We could be creating redundant external definitions here because
    // this external definition cannot be mapped to an HLInst. Add check at the
    // beginning of this function to return an existing external definition in
    // the VPlan pool.
    VPIndVar = Plan->getVPExternalDefForIV(IVLevel, Ty);

  auto IVTy = VPIndVar->getBaseType();

  // Add a conversion for VPIndVar if its type does not match canon expr
  // type specified in Ty. We mimic the code from HIR CG here.
  if (Ty != IVTy) {
    assert(Ty->isIntegerTy() && "Expected integer type");
    if (Ty->getPrimitiveSizeInBits() > IVTy->getPrimitiveSizeInBits()) {
      bool IsNSW = OutermostHLp->isNSW();
      VPIndVar = IsNSW ? decomposeConversion(VPIndVar, Instruction::SExt, Ty)
                       : decomposeConversion(VPIndVar, Instruction::ZExt, Ty);
    } else
      VPIndVar = decomposeConversion(VPIndVar, Instruction::Trunc, Ty);
  }

  DecompIV = combineDecompDefs(DecompIV, VPIndVar, Ty, Instruction::Mul);
  return DecompIV;
}

// Decompose a CanonExpr. Return the last VPValue resulting from its
// decomposition.
VPValue *VPDecomposerHIR::decomposeCanonExpr(RegDDRef *RDDR, CanonExpr *CE) {
  LLVM_DEBUG(dbgs() << "  Decomposing CanonExpr: "; CE->dump(); dbgs() << "\n");
  VPValue *DecompDef = nullptr;

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

  // Decompose IV expression.
  for (auto IVIt = CE->iv_begin(), E = CE->iv_end(); IVIt != E; ++IVIt) {
    int64_t IVConstCoeff = CE->getIVConstCoeff(IVIt);

    if (IVConstCoeff != 0) {
      VPValue *DecompIV =
          decomposeIV(RDDR, CE, CE->getLevel(IVIt), CE->getSrcType());
      DecompDef = combineDecompDefs(DecompDef, DecompIV, CE->getSrcType(),
                                    Instruction::Add);
    }
  }

  // Decompose constant additive. If it's 0, we ignore it when the CE has more
  // components (e.g., X + 0). Otherwise, CE is representing the constant 0 and
  // we have to generate a VPValue for it.
  int64_t AddCoeff = CE->getConstant();
  if (AddCoeff != 0 || !DecompDef) {
    VPValue *DecompCoeff = decomposeCoeff(AddCoeff, CE->getSrcType());
    DecompDef = combineDecompDefs(DecompDef, DecompCoeff, CE->getSrcType(),
                                  Instruction::Add);
  }

  // Decompose denominator.
  int64_t Denominator = CE->getDenominator();
  if (Denominator != 1) {
    VPValue *DecompDenom = decomposeCoeff(Denominator, CE->getSrcType());
    DecompDef = combineDecompDefs(DecompDef, DecompDenom, CE->getSrcType(),
                                  CE->isUnsignedDiv() ? Instruction::UDiv
                                                      : Instruction::SDiv);
  }

  // Decompose conversions.
  DecompDef = decomposeCanonExprConv(CE, DecompDef);

  assert(DecompDef && "CanonExpr has not been decomposed");
  return DecompDef;
}

// Decompose the incoming HIR memory reference \p Ref. Return the last VPValue
// resulting from its decomposition.
//
// The incoming DDRef can either be a LHS or RHS operand for the parent DDNode.
// In case of a RHS operand we generate an additional `load` VPInstruction along
// with decomposing the DDRef into GEP (and bitcast if needed). For a LHS
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
//    %vp0 = gep %b, 0, %vpi1
//    %vpl = load %vp0
//
//    createVPInstruction will later pick the load (always the last generated
//    "operand" for a load HLInst) as master VPI.
//
// 2. %0 = (@b)[0][i1]  +  (@c)[0][i1]
//
//    Both the RHS memory operands will be decomposed as -
//    %vp0 = gep %b, 0, %vpi1
//    %vp1 = load %vp0
//    %vp2 = gep %c, 0, %vpi1
//    %vp3 = load %vp2
//
//    createVPInstruction will be responsible for generating the `add`
//    VPInstruction using %vp1 and %vp3 as operands and set as master VPI.
//
// 3. (@a)[0][i1] = %0
//
//    The LHS memory operand will be decomposed as -
//    %vp0 = gep %a, 0, %vpi1
//
//    createVPInstruction will then generate a `store` VPInstruction using %vp0
//    as operand.
//
VPValue *VPDecomposerHIR::decomposeMemoryOp(RegDDRef *Ref) {
  LLVM_DEBUG(dbgs() << "VPDecomp: Decomposing memory operand: "; Ref->dump();
             dbgs() << "\n");

  assert(Ref->hasGEPInfo() &&
         "Expected a GEP RegDDRef to decompose memory operand.");

  // Final VPInstruction obtained on decomposing the MemOp
  VPValue *MemOpVPI;

  VPValue *DecompBaseCE = decomposeCanonExpr(Ref, Ref->getBaseCE());

  LLVM_DEBUG(dbgs() << "VPDecomp: Memop DecompBaseCE: "; DecompBaseCE->dump();
             dbgs() << "\n");

  unsigned NumDims = Ref->getNumDimensions();
  assert(NumDims > 0 && "Number of dimensions in memory operand is 0.");

  // If Ref is of the form a[0] or &a[0], GEP instructions are not needed.
  // TODO: This is needed for correctness of any analysis with opaque types.
  // However HIR codegen should be aware of this lack of GEP for Refs of form
  // a[0] and &a[0] and generate them if needed during codegen. For example
  // check Transforms/Intel_VPO/Vecopt/hir_vector_opaque_type.ll
  if (NumDims == 1 && !Ref->hasTrailingStructOffsets() &&
      (*Ref->canon_begin())->isZero())
    MemOpVPI = DecompBaseCE;
  else {
    // Determine resulting type of the GEP instruction
    //
    // Example: float a[1024];
    //
    // @a[0][i] will be decomposed as -
    // float* %vp = getelementptr [1024 x float]* %a, i64 0, i64 %i
    //
    // Here -
    // GepResultType = float*
    //
    // NOTE: Type information about pointer type or pointer element type can be
    // retrieved anytime from the first operand of GEP instruction
    //

    Type *GepResultType = Ref->getSrcType();

    // For store/load references we need to convert to PointerType
    if (!Ref->isAddressOf())
      GepResultType =
          PointerType::get(GepResultType, Ref->getPointerAddressSpace());

    // The VPGEPInstruction generated for given memory operand.
    // NOTE: At this point, the instruction is built with only the base pointer
    // operand. The indices operands are added subsequently below.
    VPGEPInstruction *MemOpVPGEP;
    if (Ref->isInBounds())
      MemOpVPGEP = cast<VPGEPInstruction>(
          Builder.createInBoundsGEP(GepResultType, DecompBaseCE));
    else
      MemOpVPGEP = cast<VPGEPInstruction>(
          Builder.createGEP(GepResultType, DecompBaseCE));

    { // This scope is for the Guard (RAII)

      VPBuilder::InsertPointGuard Guard(Builder);
      // Ensure that all the VPInstructions created for decomposition of the
      // index DDRefs are inserted before the GEP VPInstruction
      Builder.setInsertPoint(MemOpVPGEP);

      // Process indices for each dimension and update operands of VPGEP.
      for (unsigned I = NumDims; I > 0; --I) {
        VPValue *DecompIndex =
            decomposeCanonExpr(Ref, Ref->getDimensionIndex(I));
        LLVM_DEBUG(dbgs() << "VPDecomp: Memop DecompIndex: ";
                   DecompIndex->dump(); dbgs() << "\n");
        MemOpVPGEP->addOperand(DecompIndex);

        // Add indices for trailing struct offsets and record it in the struct
        // offset tracker
        auto HIRDimOffsets = Ref->getTrailingStructOffsets(I);

        // Add the offsets for the corresponding dimension operand only if it is
        // non-empty
        if (!HIRDimOffsets.empty()) {
          // Trailing struct offsets are always I32 type constants
          auto I32Ty = Type::getInt32Ty(*Plan->getLLVMContext());
          for (auto OffsetVal : HIRDimOffsets) {
            auto OffsetIndex = ConstantInt::get(I32Ty, OffsetVal);
            // Build a VPConstant to represent the offset value
            VPConstant *VPOffset = Plan->getVPConstant(OffsetIndex);
            LLVM_DEBUG(dbgs() << "VPDecomp: Struct Offset: "; VPOffset->dump();
                       dbgs() << "\n");
            MemOpVPGEP->addOperand(VPOffset, true /*IsStructOffset*/);
          }
        }
      }

    } // End Guard scope

    MemOpVPI = MemOpVPGEP;
  }

  // Create a bitcast instruction if needed
  auto BitCastDestTy = Ref->getBitCastDestType();
  if (BitCastDestTy) {
    LLVM_DEBUG(dbgs() << "VPDecomp: BitCastDestTy: "; BitCastDestTy->dump();
               dbgs() << "\n");
    MemOpVPI =
        Builder.createNaryOp(Instruction::BitCast, {MemOpVPI}, BitCastDestTy);
  }

  // If memory reference is AddressOf type, return the last generated
  // VPInstruction
  if (Ref->isAddressOf())
    return MemOpVPI;

  if (Ref->isRval()) {
    // If memory reference is an RVal, then it corresponds to a load. Create a
    // new load VPInstruction to represent it.
    assert(isa<PointerType>(MemOpVPI->getBaseType()) &&
           "Base type of load is not a pointer.");
    // Result type of load will be element type of the pointer
    MemOpVPI = Builder.createNaryOp(
        Instruction::Load, {MemOpVPI},
        cast<PointerType>(MemOpVPI->getBaseType())->getElementType());

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
      cast<VPInstruction>(MemOpVPI)->HIR.setOperandDDR(Ref);
  }

  LLVM_DEBUG(dbgs() << "VPDecomp: MemOpVPI: "; MemOpVPI->dump();
             dbgs() << "\n");

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

// Utility function that returns a CmpInst::Predicate for a given DDNode. The
// return value is in the context of the *plain* CFG construction:
//   1) HLInst representing a CmpInst -> CmpInst's opcode.
//   2) HLLoop -> ICMP_SLE or ICMP_ULE (bottom test).
// NOTE: Decomposition of HLIf nodes currently don't use this utility function
static CmpInst::Predicate getPredicateFromHIR(HLDDNode *DDNode) {
  assert((isa<HLInst>(DDNode) || isa<HLLoop>(DDNode)) &&
         "Expected HLInst or HLLoop.");

  if (auto *HInst = dyn_cast<HLInst>(DDNode)) {
    assert(isa<CmpInst>(HInst->getLLVMInstruction()) && "Expected CmpInst.");
    return cast<CmpInst>(HInst->getLLVMInstruction())->getPredicate();
  }

  // Get the predicate for the HLLoop bottom test condition.
  auto *HLp = cast<HLLoop>(DDNode);
  assert((HLp->isDo() || HLp->isDoMultiExit()) && HLp->isNormalized() &&
         "Expected single-exit normalized DO HLLoop.");
  assert(HLp->getLowerCanonExpr()->getDestType()->isIntegerTy() &&
         HLp->getUpperCanonExpr()->getDestType()->isIntegerTy() &&
         "HLLoops only support integer IVs.");

  // HLLoop upper-bound is inclusive so we return the proper less-equal
  // predicate based on the sign bit of the comparison type.
  // TODO: Does HIR perform any normalization regarding sign/unsigned types?
  if (cast<IntegerType>(HLp->getLowerCanonExpr()->getDestType())->getSignBit())
    return CmpInst::ICMP_SLE;

  return CmpInst::ICMP_ULE;
}

// Return true if \p Def is considered an external definition. An external
// definition is a definition that happens outside of the outermost HLLoop,
// including its preheader and exit.
bool VPDecomposerHIR::isExternalDef(DDRef *UseDDR) {
  // TODO: We are pushing outermost loop PH and Exit outside of the VPlan region
  // for now so this code won't be valid until we bring them back. return
  // !Def->getHLNodeUtils().contains(OutermostHLp, Def,
  //                                 true /*include preheader/exit*/);
  assert(UseDDR->isRval() && "DDRef must be an RValue!");
  return OutermostHLp->isLiveIn(UseDDR->getSymbase());
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
  return std::distance(BlobInEdges.begin(), BlobInEdges.end()) +
         OutermostHLp->isLiveIn(UseDDR->getSymbase());
}

// Return a pointer to the last VPInstruction of \p VPBB. Return nullptr if \p
// VPBB is empty.
static VPInstruction *getLastVPI(VPBasicBlock *VPBB) {
  assert((VPBB->empty() || isa<VPInstruction>(&VPBB->back())) &&
         "VPRecipes in HIR?");
  return VPBB->empty() ? nullptr : cast<VPInstruction>(&VPBB->back());
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
  for (auto &Recipe : make_range(DecompVPIStart, DecompVPIEnd)) {
    assert(isa<VPInstruction>(Recipe) && "VPRecipes in HIR?");
    auto *DecompVPI = cast<VPInstruction>(&Recipe);
    // DecompVPI is a new VPInstruction at this point. To be marked as
    // decomposed VPInstruction with the following 'setMaster'.
    assert(DecompVPI->isNew() && "Expected new VPInstruction!");
    DecompVPI->HIR.setMaster(MasterVPI);
  }
}

// Create VPInstruction for \p Node and insert it in VPBuilder's insertion
// point. If \p Node is an HLIf, we create VPCmpInsts to handle multiple
// predicates. HLLoop are not expected.
VPInstruction *
VPDecomposerHIR::createVPInstruction(HLNode *Node,
                                     ArrayRef<VPValue *> VPOperands) {
  assert(Node && "Expected Node to create a VPInstruction.");
  assert(!isa<HLLoop>(Node) && "HLLoop shouldn't be processed here!");

  if (isa<HLGoto>(Node)) {
    Type *BaseTy = Type::getInt1Ty(*Plan->getLLVMContext());
    return Builder.createBr(BaseTy, cast<HLGoto>(Node));
  }

  // Create VPCmpInst for HLInst representing a CmpInst.
  VPInstruction *NewVPInst;
  auto DDNode = cast<HLDDNode>(Node);

  // There should't be any VPValue for Node at this point. Otherwise, we
  // visited Node when we shouldn't, breaking the RPO traversal order.
  assert(!HLDef2VPValue.count(DDNode) && "Node shouldn't have been visited.");

  if (auto *HInst = dyn_cast<HLInst>(Node)) {
    const Instruction *LLVMInst = HInst->getLLVMInstruction();
    assert(LLVMInst && "Missing LLVM Instruction for HLInst.");

    if (isa<CmpInst>(LLVMInst)) {
      assert(VPOperands.size() == 2 && "Expected 2 operands in CmpInst.");
      CmpInst::Predicate CmpPredicate = getPredicateFromHIR(DDNode);
      NewVPInst = Builder.createCmpInst(CmpPredicate, VPOperands[0],
                                        VPOperands[1], DDNode);
    } else if (isa<SelectInst>(LLVMInst)) {
      // Handle decomposition of select instruction
      assert(VPOperands.size() == 4 &&
             "Invalid number of operands for HIR select instruction.");

      VPValue *CmpLHS = VPOperands[0];
      VPValue *CmpRHS = VPOperands[1];
      VPValue *TVal = VPOperands[2];
      VPValue *FVal = VPOperands[3];

      // Decompose first 2 operands into a CmpInst used as predicate for select
      VPCmpInst *Pred =
          Builder.createCmpInst(HInst->getPredicate(), CmpLHS, CmpRHS);
      // Set underlying DDNode for the select VPInstruction since it's the
      // master VPInstruction
      NewVPInst = cast<VPInstruction>(
          Builder.createNaryOp(Instruction::Select, {Pred, TVal, FVal},
                               TVal->getBaseType(), DDNode));
    } else if (isa<LoadInst>(LLVMInst)) {
      // No-op behavior for load nodes since the VPInstruction is already added
      // by decomposeMemoryOp. Check comments in decomposeMemoryOp definition
      // for more details.
      assert(VPOperands.size() == 1 &&
             "Load instruction must have single operand only.");
      NewVPInst = cast<VPInstruction>(VPOperands.back());
      assert(NewVPInst->getOpcode() == Instruction::Load &&
             "Incorrect instruction added for load.");
      // Set the underlying DDNode for the load instruction since it will be
      // master VPI for this node
      NewVPInst->HIR.setUnderlyingNode(DDNode);
    } else
      // Generic VPInstruction.
      NewVPInst = cast<VPInstruction>(Builder.createNaryOp(
          LLVMInst->getOpcode(), VPOperands, LLVMInst->getType(), DDNode));

    if (RegDDRef *LvalDDR = HInst->getLvalDDRef()) {
      // Set Lval DDRef as VPOperandHIR for this VPInstruction. This includes
      // standalone loads.
      NewVPInst->HIR.setOperandDDR(LvalDDR);

      if (OutermostHLp->isLiveOut(LvalDDR->getSymbase())) {
        VPExternalUse *User = Plan->getVPExternalUseForDDRef(LvalDDR);
        User->addOperand(NewVPInst);
      }
    } else if (RegDDRef *RvalDDR = HInst->getRvalDDRef())
      // Set single Rval as VPOperandHIR for HLInst without Lval DDRef.
      NewVPInst->HIR.setOperandDDR(RvalDDR);
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
  // Check if number of operands for a HLIf is twice the number of its
  // predicates
  assert((HIf->getNumPredicates() * 2 == VPOperands.size()) &&
         "Incorrect number of operands for a HLIf");

  auto FirstPred = HIf->pred_begin();
  // Create a new VPCmpInst corresponding to the first predicate
  VPInstruction *CurVPInst =
      Builder.createCmpInst(FirstPred->Kind, VPOperands[0], VPOperands[1]);
  LLVM_DEBUG(dbgs() << "VPDecomp: First Pred VPInst: "; CurVPInst->dump());

  unsigned OperandIt = 2;

  // Create VPInstructions for remaining predicates in the HLIf
  for (auto It = HIf->pred_begin() + 1, E = HIf->pred_end(); It != E; ++It) {
    assert(OperandIt + 1 < VPOperands.size() &&
           "Out-of-range access on VPOperands.");
    // Create a new VPCmpInst for the current predicate
    VPInstruction *NewVPInst = Builder.createCmpInst(
        It->Kind, VPOperands[OperandIt], VPOperands[OperandIt + 1]);
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
  CurVPInst->HIR.setUnderlyingNode(HIf);
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

  // Collect operands necessary to build a VPInstruction out of an HLInst and
  // translate them into VPValue's.
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
    // HIR operand of a store is identified by checking if the Lval of a DDNode
    // is memref.
    // HLInst: (%A)[i1] = %add;
    // VPInstruction: store %add, %decompAi1
    if (Lval->isMemRef())
      VPOperands.push_back(decomposeVPOperand(Lval));
  }
}

// Create or retrieve an existing VPValue that represents the definition of the
// use \p UseDDR (R-value DDRef representing a use). Return an external
// definition if such definition happens outside of the outermost loop
// represented in VPlan. Otherwise, return a VPInstruction representing the
// definition.
void VPDecomposerHIR::createOrGetVPDefsForUse(
    DDRef *UseDDR, SmallVectorImpl<VPValue *> &VPDefs) {

  assert(UseDDR->isRval() && "DDRef must be an RValue!");

  // Process metadata definitions.
  MetadataAsValue *MDAsValue;
  if (UseDDR->isMetadata(&MDAsValue))
    VPDefs.push_back(Plan->getVPMetadataAsValue(MDAsValue));

  // Process external definitions.
  if (isExternalDef(UseDDR))
    VPDefs.push_back(Plan->getVPExternalDefForDDRef(UseDDR));

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
      // create a SemiPhiOp without any operands. We might not have all the
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
         isa<VPBasicBlock>(LpPH->getSingleSuccessor()) &&
         "Loop PH must have one successor VPBasicBlock.");
  VPBasicBlock *LpH = cast<VPBasicBlock>(LpPH->getSingleSuccessor());

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
  Builder.setInsertPoint(LpH, LpH->begin());
  SmallVector<VPValue *, 4> PhiArgs;
  PhiArgs.push_back(IVStart);
  Type *BaseTy = getBaseTypeForSemiPhiOp(PhiArgs);
  VPInstruction *IndSemiPhi =
      cast<VPInstruction>(Builder.createSemiPhiOp(BaseTy, PhiArgs, HLp));
  assert(!HLLp2IVSemiPhi.count(HLp) && "HLLoop has multiple IVs?");
  HLLp2IVSemiPhi[HLp] = IndSemiPhi;
  IndSemiPhi->HIR.setValid();
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
  //    %vp40944 = semi-phi i64 0 %vp43488
  //    ...
  //    %vp43488 = add %vp40944 i64 1 // IVNext (master VPI)
  //    %vp44352 = icmp %vp43488 %vp44016 // BottomTest (master VPI)
  //   SUCCESSORS(2):BB4(%vp44352), BB5(!%vp44352)

  // Retrieve the inductive semi-phi (IV) generated for this HLLoop.
  assert(HLLp2IVSemiPhi.count(HLp) &&
         "Expected semi-phi VPInstruction for HLLoop.");
  VPInstruction *IndSemiPhi = HLLp2IVSemiPhi[HLp];

  // Create add VPInstruction for IV next. HLp is set as underlying HIR of the
  // created VPInstruction. Only normalized loops are expected so we use step 1.
  assert(HLp->getStrideCanonExpr()->isOne() &&
         "Expected positive unit-stride HLLoop.");
  Builder.setInsertPoint(LpLatch);
  auto *IVNext = cast<VPInstruction>(Builder.createAdd(
      IndSemiPhi,
      Plan->getVPConstant(ConstantInt::getSigned(HLp->getIVType(), 1)), HLp));

  // Add IVNext to induction semi-phi.
  IndSemiPhi->addOperand(IVNext);

  // Create VPValue for bottom test condition. If decomposition is needed:
  //   1) decompose UB operand. Decomposed VPInstructions are inserted into the
  //      loop PH since they are loop invariant,
  //   2) create master VPInstruction for the bottom test condition with IVNext
  //      and decomposed UB as operands, and,
  //   3) set the last created VPInstruction for UB as master VPInstruction for
  //      that UB group of decomposed VPInstructions.
  assert(HLp->getUpperDDRef() && "Expected a valid upper DDRef for HLLoop.");
  SmallVector<VPValue *, 2> VPOperands;

  // Keep last instruction before decomposition. We will need it to set the
  // master VPInstruction of all the created decomposed VPInstructions.
  VPInstruction *LastVPIBeforeDec = getLastVPI(LpPH);
  VPValue *DecompUB;
  VPOperands.push_back(IVNext);
  { // #1. This scope is for Guard (RAII).
    VPBuilder::InsertPointGuard Guard(Builder);
    Builder.setInsertPoint(LpPH);
    DecompUB = decomposeVPOperand(HLp->getUpperDDRef());
    VPOperands.push_back(DecompUB);
  }

  // #2.
  CmpInst::Predicate CmpPredicate = getPredicateFromHIR(HLp);
  auto *BottomTest = Builder.createCmpInst(CmpPredicate, VPOperands[0],
                                           VPOperands[1], HLp);

  if (auto *DecompUBVPI = dyn_cast<VPInstruction>(DecompUB)) {
    // #3. Turn last decomposed VPInstruction of UB as master VPInstruction of
    // the decomposed group.
    DecompUBVPI->HIR.setUnderlyingNode(HLp);

    // Set DecompUBVPI as master VPInstruction of any other decomposed
    // VPInstruction of UB.
    setMasterForDecomposedVPIs(DecompUBVPI, LastVPIBeforeDec, LpPH);
    DecompUBVPI->HIR.setValid();
  }

  // Set the underlying HIR of the new VPInstructions (and its potential
  // decomposed VPInstructions) to valid.
  IndSemiPhi->HIR.setValid();
  IVNext->HIR.setValid();
  BottomTest->HIR.setValid();
  return BottomTest;
}

// Add operands to VPInstructions representing phi nodes from the input IR.
// PhisToFix contains a pair with VPPhi and its associated sink DDRef. We get
// the source of each incoming edge of DDRef and set the VPValue associated to
// that source as operand of the VPPhi.
// TODO: Above documentation is incorrect. Update after new algorithm.
void VPDecomposerHIR::fixPhiNodes() {
  for (auto &VPPhiMapPair : PhisToFix) {
    VPInstruction *VPPhi = VPPhiMapPair.second.first;
    // Move the VPPhi node to the top of its VPBB
    moveSemiPhiToFront(VPPhi);
    DDRef *UseDDR = VPPhiMapPair.second.second;
    assert(VPPhi->getNumOperands() == 0 &&
           "Expected VPInstruction with no operands.");

    SmallVector<VPValue *, 4> VPOperands;
    createOrGetVPDefsForUse(UseDDR, VPOperands);
    assert(VPOperands.size() > 1 && "Expected multiple definitions for VPPhi!");
    for (auto *VPOp : VPOperands)
      VPPhi->addOperand(VPOp);

    // Set the master VPInstruction of this VPPhi as valid after the fix.
    VPPhi->HIR.getMaster()->HIR.setValid();
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

  // Set the insertion point in the builder for the VPInstructions that we are
  // going to create for this Node.
  Builder.setInsertPoint(InsPointVPBB);
  // Keep last instruction before decomposition. We will need it to set the
  // master VPInstruction of all the decomposed VPInstructions created.
  VPInstruction *LastVPIBeforeDec = getLastVPI(InsPointVPBB);

  // Create and decompose the operands of the future new VPInstruction.
  // They will be inserted (obviously) before the new VPInstruction.
  SmallVector<VPValue *, 4> VPOperands;
  createVPOperandsForMasterVPInst(Node, VPOperands);

  // Create new VPInstruction with previous operands.
  VPInstruction *NewVPInst = createVPInstruction(Node, VPOperands);

  // Set NewVPInst as master VPInstruction of any decomposed VPInstruction
  // resulting from decomposing its operands.
  setMasterForDecomposedVPIs(NewVPInst, LastVPIBeforeDec, InsPointVPBB);

  // Set the underlying HIR of the new VPInstruction (and its potential
  // decomposed VPInstructions) to valid.
  NewVPInst->HIR.setValid();
  return NewVPInst;
}

// Create a VPValue for a non-integer constant \p Blob. A non-integer constant
// blob can be a floating point or an undef.
VPConstant *VPDecomposerHIR::VPBlobDecompVisitor::decomposeNonIntConstBlob(
    const SCEVUnknown *Blob) {
  BlobUtils &BlUtils = RDDR.getBlobUtils();
  assert(BlUtils.isConstantDataBlob(Blob) && "Expected a ConstantDataBlob.");
  (void)BlUtils;

  ConstantFP *FPConst;
  if (BlUtils.isConstantFPBlob(Blob, &FPConst))
    return Decomposer.Plan->getVPConstant(FPConst);

  if (BlUtils.isUndefBlob(Blob))
    return Decomposer.Plan->getVPConstant(UndefValue::get(Blob->getType()));

  llvm_unreachable("Unsupported non-integer HIR Constant.");
}

// Create a VPValue for a standalone blob given its SCEV. A standalone blob is
// unitary and doesn't need decomposition.
VPValue *VPDecomposerHIR::VPBlobDecompVisitor::decomposeStandAloneBlob(
    const SCEVUnknown *Blob) {

  if (RDDR.getBlobUtils().isConstantDataBlob(Blob))
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
  // definitions, in addition, we introduce a semi-phi operation that "blends"
  // all the VPValue definitions.
  SmallVector<VPValue *, 2> VPDefs;
  if (BlobNumReachDefs == 1) {
    // Single definition.
    Decomposer.createOrGetVPDefsForUse(DDR, VPDefs);
    assert(VPDefs.size() == 1 && "Expected single definition.");
    return VPDefs.front();
  } else {
    // The operands of the semi-phi are not set right now since some of them
    // might not have been created yet. They will be set by fixPhiNodes.
    // Map the corresponding Instruction to <DDRef's Symbase, VPBlockID> in
    // PhisToFix if not found.
    Decomposer.createOrGetVPDefsForUse(DDR, VPDefs);
    Type *BaseTy = VPDefs.front()->getBaseType();

    // Build the key pair to look-up PhiToFix map
    PhiFixMapKey SymVPBBPair = std::make_pair(
        DDR->getSymbase(), Decomposer.Builder.getInsertBlock()->getVPBlockID());

    auto VPPhiMapIt = Decomposer.PhisToFix.find(SymVPBBPair);
    // If a VPPhi node was already created for this sink DDRef's Symbase in
    // current VPBB, then reuse that.
    if (VPPhiMapIt != Decomposer.PhisToFix.end())
      return (VPPhiMapIt->second).first;

    // If no entry is found in PhisToFix then create a new VPPhi node and add it
    // to the map
    auto *SemiPhi = cast<VPInstruction>(Decomposer.Builder.createSemiPhiOp(
        BaseTy, {} /*No operands*/, nullptr));
    LLVM_DEBUG(dbgs() << "VPDecomp: Empty SemiPhi "; SemiPhi->dump();
               dbgs() << "\n");
    Decomposer.PhisToFix[SymVPBBPair] = std::make_pair(SemiPhi, DDR);
    return SemiPhi;
  }
}

// Helper function to decomposed an SCEVNAryExpr using the same \p OpCode to
// combine all the sub-expressions.
VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::decomposeNAryOp(const SCEVNAryExpr *Blob,
                                                      unsigned OpCode) {
  VPValue *DecompDef = nullptr;
  Type *ExprTy = Blob->getType();
  for (auto *SCOp : Blob->operands()) {
    VPValue *VPOp = Decomposer.decomposeBlobImplicitConv(visit(SCOp), ExprTy);
    DecompDef =
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
VPDecomposerHIR::VPBlobDecompVisitor::visitUnknown(const SCEVUnknown *Expr) {
  return decomposeStandAloneBlob(Expr);
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitCouldNotCompute(
    const SCEVCouldNotCompute *Expr) {
  llvm_unreachable("Attempt to use a SCEVCouldNotCompute object.");
}
