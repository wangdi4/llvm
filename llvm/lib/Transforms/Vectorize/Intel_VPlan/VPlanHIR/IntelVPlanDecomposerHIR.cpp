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
#include "../IntelVPlanIDF.h"
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

// Splice the instruction list of the VPBB where \p Phi belongs, by moving the
// VPPhi instruction to the front of the list
static void movePhiToFront(VPPHINode *Phi) {
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
      Builder.createNaryOp(OpCode, {LHS, RHS}, LHS->getType()));
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

  VPValue *VPIndVar = HLLp2IVPhi[getHLLoopForLevel(RDDR, IVLevel)];
  if (!VPIndVar)
    // If there is no PHI in the map, it means that the IV is an external
    // definition.
    // TODO: We could be creating redundant external definitions here because
    // this external definition cannot be mapped to an HLInst. Add check at the
    // beginning of this function to return an existing external definition in
    // the VPlan pool.
    VPIndVar = Plan->getVPExternalDefForIV(IVLevel, Ty);

  auto IVTy = VPIndVar->getType();

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
    assert(isa<PointerType>(MemOpVPI->getType()) &&
           "Base type of load is not a pointer.");
    // Result type of load will be element type of the pointer
    MemOpVPI = Builder.createNaryOp(
        Instruction::Load, {MemOpVPI},
        cast<PointerType>(MemOpVPI->getType())->getElementType());

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
bool VPDecomposerHIR::isExternalDef(const DDRef *UseDDR) {
  // TODO: We are pushing outermost loop PH and Exit outside of the VPlan region
  // for now so this code won't be valid until we bring them back. return
  // !Def->getHLNodeUtils().contains(OutermostHLp, Def,
  //                                 true /*include preheader/exit*/);
  assert(UseDDR->isRval() && "DDRef must be an RValue!");
  return OutermostHLp->isLiveIn(UseDDR->getSymbase());
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
      NewVPInst = cast<VPInstruction>(Builder.createNaryOp(
          Instruction::Select, {Pred, TVal, FVal}, TVal->getType(), DDNode));
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
  // Base type for the VPPHINode is obtained from IVStart
  Type *BaseTy = IVStart->getType();
  VPPHINode *IndVPPhi = Builder.createPhiInstruction(BaseTy, HLp);
  IndVPPhi->addIncoming(IVStart, LpPH);
  assert(!HLLp2IVPhi.count(HLp) && "HLLoop has multiple IVs?");
  HLLp2IVPhi[HLp] = IndVPPhi;
  IndVPPhi->HIR.setValid();
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
  Builder.setInsertPoint(LpLatch);
  VPConstant *One =
      Plan->getVPConstant(ConstantInt::getSigned(HLp->getIVType(), 1));
  auto *IVNext = cast<VPInstruction>(Builder.createAdd(IndVPPhi, One, HLp));

  // Add IVNext to induction PHI.
  IndVPPhi->addIncoming(IVNext, LpLatch);

  // Add to the induction descriptors. Push it at the beginning as main
  // induction.
  std::unique_ptr<VPInductionHIRList> &IndList = Inductions[HLp];
  if (!IndList)
    IndList.reset(new VPInductionHIRList);
  IndList->insert(
      IndList->begin(),
      std::make_unique<VPInductionHIR>(
          IVNext, One,
          Plan->getVPConstant(Constant::getNullValue(HLp->getIVType()))));

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
  IndVPPhi->HIR.setValid();
  IVNext->HIR.setValid();
  BottomTest->HIR.setValid();
  return BottomTest;
}


/// Determine which blocks the current symbase \p CurSymbase is live in.
///
///  These are the blocks that lead to actual use of the symbase. This knowlegde
///  will help us avoid inserting PHI nodes into blocks without any uses (no
///  dead PHI nodes)
void VPDecomposerHIR::computeLiveInBlocks(
    unsigned CurSymbase, const SmallPtrSetImpl<VPBlockBase *> &DefBlocks,
    const SmallPtrSetImpl<VPBlockBase *> &UsingBlocks,
    SmallPtrSetImpl<VPBlockBase *> &LiveInBlocks) {
  // Liveness of the current symbase is determined by iterating over
  // predecessors of the blocks where definition is live. These blocks are
  // tracked via a worklist

  // Initially this worklist contains all the using blocks of the current
  // symbase
  SmallVector<VPBlockBase *, 16> LiveInBlockWorklist(UsingBlocks.begin(),
                                                     UsingBlocks.end());

  // If a VPBB is both a defining and using block of the symbase, then we need
  // to check if the definition comes before or after the use. If definition
  // happens before use, the symbase is not really live-in to VPBB
  for (unsigned I = 0, E = LiveInBlockWorklist.size(); I != E; ++I) {
    VPBlockBase *VPBB = LiveInBlockWorklist[I];
    if (!DefBlocks.count(VPBB))
      continue;

    // VPBB has both use and definition of symbase, iterate over it's VPIs to
    // find out which comes first
    assert(isa<VPBasicBlock>(VPBB) && "HCFG block is not a VPBasicBlock");
    for (auto &VPI : cast<VPBasicBlock>(VPBB)->vpinstructions()) {
      // Underlying HIR is not attached to non-master VPInstructions
      if (!VPI.HIR.isMaster())
        continue;
      // We don't need to analyze non DDNode nodes like HLGoto
      if (!isa<loopopt::HLDDNode>(VPI.HIR.getUnderlyingNode()))
        continue;

      HLDDNode *DDNode = cast<loopopt::HLDDNode>(VPI.HIR.getUnderlyingNode());
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
    VPBlockBase *VPBB = LiveInBlockWorklist.pop_back_val();

    // Insert the VPBB into the proper LiveInBlocks set. If it already in the
    // set then we have also processed its predecessors
    if (!LiveInBlocks.insert(VPBB).second)
      continue;

    // Add predecessors of VPBB unless they are defining blocks of current
    // symbase
    for (VPBlockBase *Pred : VPBB->getPredecessors()) {
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
  DenseMap<unsigned, SmallPtrSet<VPBlockBase *, 8>> SymbaseDefBlocks;
  DenseMap<unsigned, SmallPtrSet<VPBlockBase *, 8>> SymbaseUsingBlocks;
  VPRegionBlock *Region = Builder.getInsertBlock()->getParent();
  Region->computeDT();
  VPDominatorTree &DT = *(Region->getDT());
  VPBlockBase *HCFGEntry = Region->getEntry();

  ///// Populate use-def blocks of each tracked symbase //////

  // Initialize all keys for the Symbase(Def|Using)Blocks map. If Symbase has an
  // external def then the HCFGEntry block is noted as one of its defining block
  for (auto Sym : TrackedSymbases) {
    if (Plan->getVPExternalDefForSymbase(Sym)) {
      SymbaseDefBlocks[Sym] = {HCFGEntry};
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
  for (VPBlockBase *VPBB :
       make_range(df_iterator<VPBlockBase *>::begin(HCFGEntry),
                  df_iterator<VPBlockBase *>::end(HCFGEntry))) {
    assert(isa<VPBasicBlock>(VPBB) && "HCFG block is not a VPBasicBlock");
    for (auto &VPI : cast<VPBasicBlock>(VPBB)->vpinstructions()) {
      if (!VPI.HIR.isMaster())
        continue;
      // We don't need to analyze non DDNode nodes like HLGoto
      if (!isa<loopopt::HLDDNode>(VPI.HIR.getUnderlyingNode()))
        continue;

      HLDDNode *DDNode = cast<loopopt::HLDDNode>(VPI.HIR.getUnderlyingNode());
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
    SmallPtrSet<VPBlockBase *, 8> SymLiveInBlocks;
    SmallVector<VPBlockBase *, 8> IDFPHIBlocks;

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

    for (auto IPB : IDFPHIBlocks) {
      VPBasicBlock *NewPhiBB = cast<VPBasicBlock>(IPB);
      LLVM_DEBUG(dbgs() << "VPDecomp: IDF decided to add a PHI in "
                        << NewPhiBB->getName() << " for the tracked symbase "
                        << Sym << "\n");
      std::pair<VPBasicBlock *, unsigned> VPBBSymPair =
          std::make_pair(NewPhiBB, Sym);
      if (PhisToFix.find(VPBBSymPair) == PhisToFix.end()) {
        // IDF suggests to add a new PHI node in IPB basic block, no entry was
        // found in PhisToFix

        VPBuilder::InsertPointGuard Guard(Builder);
        Builder.setInsertPoint(NewPhiBB, NewPhiBB->begin());

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

  // Preparations for fixPhiNodePass

  // 1. Make sure that all new PHI nodes added by decomposition are moved to the
  // top of the VPBB
  for (auto PHIMapIt : PhiToSymbaseMap)
    movePhiToFront(PHIMapIt.first);

  // 2. Set the incoming values of all tracked Symbases to their ExternalDef
  // values or nullptr before HCFG entry
  PhiNodePassData::VPValMap VPValues;
  for (auto Sym : TrackedSymbases) {
    LLVM_DEBUG(dbgs() << "Sym: " << Sym << "\n");

    VPValue *ExtDef = Plan->getVPExternalDefForSymbase(Sym);
    if (ExtDef) {
      LLVM_DEBUG(dbgs() << "ExtDef: "; ExtDef->dump(); dbgs() << "\n");
    } else {
      assert(!OutermostHLp->isLiveIn(Sym) &&
             "External def not found for a live-in symbase.");
      LLVM_DEBUG(dbgs() << "ExtDef: nullptr\n");
    }

    VPValues[Sym] = ExtDef;
  }

  VPBasicBlock *CurrentVPBB = Builder.getInsertBlock();
  assert(CurrentVPBB && "Current insertion VPBB for builder cannot be null.");
  VPBasicBlock *HCFGEntry =
      cast<VPBasicBlock>(CurrentVPBB->getParent()->getEntry());
  assert(HCFGEntry && "Entry VPBB for HCFG cannot be null.");
  LLVM_DEBUG(dbgs() << "HCFGEntry: "; HCFGEntry->dump(); dbgs() << "\n");

  // Walk all VPBasicBlocks in the HCFG of current VPlan and perform the PHI
  // node fixing algorithm, updating the incoming values based on underlying HIR
  SmallVector<PhiNodePassData, 32> PhiNodePassWorkList;
  PhiNodePassWorkList.emplace_back(HCFGEntry, nullptr /* No pred to entry*/,
                                   VPValues);

  LLVM_DEBUG(dbgs() << "Starting fixPhiNodePass algorithm\n");

  do {
    PhiNodePassData PNPD = std::move(PhiNodePassWorkList.back());
    PhiNodePassWorkList.pop_back();

    fixPhiNodePass(PNPD.VPBB, PNPD.VPBBPred, PNPD.VPValues,
                   PhiNodePassWorkList);
  } while (!PhiNodePassWorkList.empty());

  // TODO: validate correctness of the PHI nodes after fixing, also set their
  // master VPI's HIR to valid (VPPhi->HIR.getMaster()->HIR.setValid())
  for (auto PhiMapIt : PhisToFix) {
    VPPHINode *FixedPhi = PhiMapIt.second.first;
    if (FixedPhi->getNumIncomingValues() ==
        FixedPhi->getParent()->getNumPredecessors())
      continue;
    // This fixed PHI node has an empty/null incoming value from one of its
    // predecessors. This could happen due to inaccuracies in DDG. In most
    // cases such PHI nodes are not even needed. Following are possible cases :
    // 1. The PHI node has just a single incoming value.
    //    Solution : We replace all uses of this PHI with its operand, and
    //    remove the PHI.
    // 2. The PHI node has no incoming value.
    //    Solution : This means there was no definition of the ambiguous symbase
    //    before visiting the VPBB i.e. the VPBB defines this symbase for the
    //    first time. Iterate over the underlying HIR instructions of the VPBB,
    //    get the first HLInst (and VPInstruction)  that defines the symbase,
    //    replace all uses of PHI with this instruction thereby removing the
    //    PHI.
    if (FixedPhi->getNumIncomingValues() > 1)
      llvm_unreachable(
          "Only expecting incorrect PHIs with single or no incoming values.");

    LLVM_DEBUG(dbgs() << "VPDecomp fixPhiNodes : The fixed PHI node will be "
                         "replaced and removed:";
               FixedPhi->dump(); dbgs() << "\n");

    if (FixedPhi->getNumIncomingValues() == 1) {
      // Solution for case 1
      unsigned Idx = 0;
      // HIR should not be invalidated, we are still building initial HCFG.
      FixedPhi->replaceAllUsesWith(FixedPhi->getIncomingValue(Idx),
                                   false /*InvalidateIR*/);
      FixedPhi->getParent()->eraseRecipe(FixedPhi);
    } else {
      // Solution for case 2
      HLDDNode *FirstDefNode = nullptr;
      for (auto &VPI : FixedPhi->getParent()->vpinstructions()) {
        if (!VPI.HIR.isMaster())
          continue;

        if (!isa<loopopt::HLDDNode>(VPI.HIR.getUnderlyingNode()))
          continue;

        HLDDNode *DDNode =
            cast<loopopt::HLDDNode>(VPI.HIR.getUnderlyingNode());
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
      FixedPhi->getParent()->eraseRecipe(FixedPhi);
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
      unsigned NumEdges =
          std::count(Pred->succ_begin(), Pred->succ_end(), VPBB);
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
  for (auto &VPI : VPBB->vpinstructions()) {
    LLVM_DEBUG(dbgs() << "fixPhiNodePass: VPI: "; VPI.dump());

    if (VPI.HIR.isMaster()) {
      if (auto *HInst =
              dyn_cast<loopopt::HLInst>(VPI.HIR.getUnderlyingNode())) {
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

  for (VPBlockBase::succ_reverse_iterator RI = VPBB->succ_rbegin(),
                                          RE = VPBB->succ_rend();
       RI != RE; ++RI) {
    // Keep track of successors so that same successor is not added to worklist
    // twice
    if (VisitedSuccs.insert(cast<VPBasicBlock>(*RI)).second)
      Worklist.emplace_back(cast<VPBasicBlock>(*RI), VPBB, IncomingVPVals);
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

  // If this Node is a HLInst, check if it potentially updates any HIRLegality's
  // SIMD descriptors (linear/reduction).
  if (auto *HInst = dyn_cast<HLInst>(Node))
    HIRLegality.recordPotentialSIMDDescrUpdate(HInst);

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

    // Build the key pair to look-up PhisToFix map
    PhiFixMapKey VPBBSymPair =
        std::make_pair(Decomposer.Builder.getInsertBlock(), DDR->getSymbase());

    auto VPPhiMapIt = Decomposer.PhisToFix.find(VPBBSymPair);
    // If a VPPhi node was already created for this sink DDRef's Symbase in
    // current VPBB, then reuse that.
    if (VPPhiMapIt != Decomposer.PhisToFix.end())
      return (VPPhiMapIt->second).first;

    // If no entry is found in PhisToFix then create a new VPPhi node and add it
    // to the map
    auto *VPPhi = Decomposer.Builder.createPhiInstruction(BaseTy);
    Decomposer.PhisToFix[VPBBSymPair] = std::make_pair(VPPhi, DDR);

    LLVM_DEBUG(dbgs() << "Adding a new empty PHI node for:\n");
    LLVM_DEBUG(dbgs() << "Symbase: " << DDR->getSymbase() << "\n");
    LLVM_DEBUG(dbgs() << "DDR: "; DDR->dump(); dbgs() << "\n");

    // Add the Symbase of sink DDRef to be tracked for fixing PHI nodes
    Decomposer.TrackedSymbases.insert(DDR->getSymbase());

    // Add the type of the PHI node that was added for this tracked Symbase
    if (Decomposer.TrackedSymTypes.count(DDR->getSymbase())) {
      assert(Decomposer.TrackedSymTypes[DDR->getSymbase()] == BaseTy &&
             "Different type PHI node was inserted for same symbase.");
    } else {
      Decomposer.TrackedSymTypes[DDR->getSymbase()] = BaseTy;
    }

    // Add entry to map the PHI node to the sink DDRef Symbase it was generated
    // for
    assert(Decomposer.PhiToSymbaseMap.find(VPPhi) ==
               Decomposer.PhiToSymbaseMap.end() &&
           "The PHI node is already mapped to a Symbase?");
    Decomposer.PhiToSymbaseMap[VPPhi] = DDR->getSymbase();

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
VPDecomposerHIR::VPBlobDecompVisitor::visitSMinExpr(const SCEVSMinExpr *Expr) {
  return decomposeNAryOp(Expr, VPInstruction::SMin);
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitUMinExpr(const SCEVUMinExpr *Expr) {
  return decomposeNAryOp(Expr, VPInstruction::UMin);
}

VPValue *
VPDecomposerHIR::VPBlobDecompVisitor::visitUnknown(const SCEVUnknown *Expr) {
  return decomposeStandAloneBlob(Expr);
}

VPValue *VPDecomposerHIR::VPBlobDecompVisitor::visitCouldNotCompute(
    const SCEVCouldNotCompute *Expr) {
  llvm_unreachable("Attempt to use a SCEVCouldNotCompute object.");
}
