//===-- VPOAvrDecomposeHIR.cpp --------------------------------------------===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the AVRDecomposeHIR Pass. This pass decomposes AVRValue
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrDecomposeHIR.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/Passes.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrUtilsHIR.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrVisitor.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"

#define DEBUG_TYPE "avr-decomposition"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

char AVRDecomposeHIR::ID = 0;

INITIALIZE_PASS_BEGIN(AVRDecomposeHIR, "hir-avr-decompose", "AVR Decompose HIR",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(AVRGenerateHIR)
INITIALIZE_PASS_END(AVRDecomposeHIR, "hir-avr-decompose", "AVR Decompose HIR",
                    false, true)
//
// Pass Initialization
//

FunctionPass *llvm::createAVRDecomposeHIRPass() {
  return new AVRDecomposeHIR();
}

//
// Utility Functions
//

SmallVector<AVR *, 32> AVRLog;
SmallVector<Constant *, 16> ConstLog;

static AVRValue *decomposeCoeff(Constant *Const) {
  AVRValue *AVal = AVRUtils::createAVRValue(Const);
  AVRLog.push_back(AVal);
  return AVal;
}

static AVRValue *decomposeCoeff(int64_t Coeff, Type *Ty) {
  Constant *ConstCoeff;

  // Null value for pointer types needs special treatment
  if (Coeff == 0 && Ty->isPointerTy()) {
    ConstCoeff = Constant::getNullValue(Ty);
  } else {
    ConstCoeff = ConstantInt::getSigned(Ty, Coeff);
  }
 
  ConstLog.push_back(ConstCoeff);
  return decomposeCoeff(ConstCoeff);
}

static AVRExpression *decomposeConversion(AVR *Src, unsigned ConvOpCode,
                                          Type *DestType) {
  assert((ConvOpCode == Instruction::ZExt || ConvOpCode == Instruction::SExt ||
          ConvOpCode == Instruction::Trunc) &&
         "Unexpected conversion OpCode");

  AVRExpression *AExpr =
      AVRUtils::createAVRExpression(Src, ConvOpCode, DestType);
  AVRLog.push_back(AExpr);

  return AExpr;
}

// It creates a new AVRExpression if LHS and RHS are not nullptr.
// Otherwise, it returns the AVR that is not nullptr
static AVR *combineSubTrees(AVR *LHS, AVR *RHS, Type *Ty, unsigned OpCode) {

  assert((LHS != nullptr || RHS != nullptr) && "LHS and RHS cannot be nullptr");

  if (LHS == nullptr)
    return RHS;
  else if (RHS == nullptr)
    return LHS;
  else {
    AVRExpression *AExpr = AVRUtils::createAVRExpression(LHS, RHS, OpCode, Ty);
    AVRLog.push_back(AExpr);
    return AExpr;
  }
}

// This class implements the decomposition of a blob in a RegDDRef. The
// decomposition is based on the SCEV representation of the blob.
// Resulting AVRValues representing a self blob will have a pointer to the
// corresponding BlobDDRef attached to the RegDDRef.
class BlobDecompVisitor : public SCEVVisitor<BlobDecompVisitor, AVR *> {

private:
  RegDDRef &RDDR;

  AVRValueHIR *decomposeStandAloneBlob(const SCEV *SC);
  AVR *decomposeNAryOp(const SCEVNAryExpr *SC, unsigned OpCode);
  AVR *decomposeUDivOp(const SCEVUDivExpr *SC);

public:
  BlobDecompVisitor(RegDDRef &R) : RDDR(R) {}

  AVR *visitConstant(const SCEVConstant *Constant);
  AVR *visitTruncateExpr(const SCEVTruncateExpr *Expr);
  AVR *visitZeroExtendExpr(const SCEVZeroExtendExpr *Expr);
  AVR *visitSignExtendExpr(const SCEVSignExtendExpr *Expr);
  AVR *visitAddExpr(const SCEVAddExpr *Expr);
  AVR *visitMulExpr(const SCEVMulExpr *Expr);
  AVR *visitUDivExpr(const SCEVUDivExpr *Expr);
  AVR *visitAddRecExpr(const SCEVAddRecExpr *Expr);
  AVR *visitSMaxExpr(const SCEVSMaxExpr *Expr);
  AVR *visitUMaxExpr(const SCEVUMaxExpr *Expr);
  AVR *visitUnknown(const SCEVUnknown *Expr);
  AVR *visitCouldNotCompute(const SCEVCouldNotCompute *Expr);
};

// It decomposes a standalone blob given its SCEV. A standalone blob is unitary
// and doesn't need decomposition. However, a standalone blob can be part of a
// more complex CanonExpr. This function is simply generating an AVRValue for a
// standalone blob.
AVRValueHIR *BlobDecompVisitor::decomposeStandAloneBlob(const SCEV *SC) {

  unsigned BlobIndex = RDDR.getBlobUtils().findBlob(SC);
  assert(BlobIndex != InvalidBlobIndex && "SCEV is not a Blob");

  // Self blobs will always have a BlobDDRef at this point. Self blob RegDDRefs
  // won't reach this point as they do not need decomposition.
  BlobDDRef *BDDR = RDDR.getBlobDDRef(BlobIndex);
  assert(BDDR != nullptr && "BlobDDRef not found!");

  AVRValueHIR *AVal =
      AVRUtilsHIR::createAVRValueHIR(BDDR, nullptr /* parent */);
  AVRLog.push_back(AVal);

  return AVal;
}

AVR *BlobDecompVisitor::decomposeNAryOp(const SCEVNAryExpr *SC, unsigned OpCode) {
  auto SCOperands = SC->operands();

  // Initialize OpTree with the first operand
  AVR *ExprTree = visit(*SCOperands.begin());

  for (auto Op = std::next(SCOperands.begin()), OpEnd = SCOperands.end();
       Op != OpEnd; ++Op) {
    AVR *OpTree = visit(*Op);
    ExprTree = combineSubTrees(OpTree, ExprTree, SC->getType(), OpCode);
  }

  return ExprTree;
}

AVR *BlobDecompVisitor::visitConstant(const SCEVConstant *Constant) {
  return decomposeCoeff(Constant->getValue());
}

AVR *BlobDecompVisitor::visitTruncateExpr(const SCEVTruncateExpr *Expr) {

  AVR *Src = visit(Expr->getOperand());
  return decomposeConversion(Src, Instruction::Trunc, Expr->getType()); 
}

AVR *BlobDecompVisitor::visitZeroExtendExpr(const SCEVZeroExtendExpr *Expr) {

  AVR *Src = visit(Expr->getOperand());
  return decomposeConversion(Src, Instruction::ZExt, Expr->getType()); 
}

AVR *BlobDecompVisitor::visitSignExtendExpr(const SCEVSignExtendExpr *Expr) {

  AVR *Src = visit(Expr->getOperand());
  return decomposeConversion(Src, Instruction::SExt, Expr->getType()); 
}

AVR *BlobDecompVisitor::visitAddExpr(const SCEVAddExpr *Expr) {
  return decomposeNAryOp(Expr, Instruction::Add);
}

AVR *BlobDecompVisitor::visitMulExpr(const SCEVMulExpr *Expr) {
  return decomposeNAryOp(Expr, Instruction::Mul);
}

AVR *BlobDecompVisitor::visitUDivExpr(const SCEVUDivExpr *Expr) {
  AVR *DivLHS = visit (Expr->getLHS());
  AVR *DivRHS = visit (Expr->getRHS());

  return combineSubTrees(DivLHS, DivRHS, Expr->getType(), Instruction::UDiv);
}

AVR *BlobDecompVisitor::visitAddRecExpr(const SCEVAddRecExpr *Expr) {
  llvm_unreachable("Expected add-recs to be broken by canon-expr");
}

AVR *BlobDecompVisitor::visitSMaxExpr(const SCEVSMaxExpr *Expr) {
  return decomposeNAryOp(Expr, AVRExpression::SMax);
}

AVR *BlobDecompVisitor::visitUMaxExpr(const SCEVUMaxExpr *Expr) {
  return decomposeNAryOp(Expr, AVRExpression::UMax);
}

AVR *BlobDecompVisitor::visitUnknown(const SCEVUnknown *Expr) {
  return decomposeStandAloneBlob(Expr);
}

AVR *BlobDecompVisitor::visitCouldNotCompute(const SCEVCouldNotCompute *Expr) {
  llvm_unreachable("Attempt to use a SCEVCouldNotCompute object!");
}

// This class implements the main visitor of the AVRValueHIR decomposition.
// It has the coarse-grained functionality: decomposition of a RegDDRef, CanonExpr,
// IVs, and generic blobs.
class HIRDecomposer {

private:
  const DataLayout &DL;

  bool needsDecomposition(AVRValueHIR *AVal);
  AVR *decompose(AVRValueHIR *AVal);

  AVR *decomposeCanonExpr(RegDDRef *RDDR, CanonExpr *CE);
  AVR *decomposeCanonExprConv(CanonExpr *CE, AVR *SrcTree);
  AVRExpression *decomposeMemoryOp(AVRValueHIR *AVal);
  AVR *decomposeIV(RegDDRef *RDDR, CanonExpr *CE, unsigned IVLevel, Type *Ty);

public:
  HIRDecomposer(const DataLayout &D) : DL(D) {}

  /// Visit Functions
  void visit(AVR *AVR){};
  void visit(AVRValueHIR *AVal);
  void postVisit(AVR *AVR){};
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }
};

bool HIRDecomposer::needsDecomposition(AVRValueHIR *AVal) {

  // We don't need to decompose:
  //   - Constants (including null pointers)
  //   - BlobDDRefs (already decomposed)
  //   - IVs (already decomposed)
  if (AVal->isConstant() || AVal->isIVValue() ||
      isa<BlobDDRef>(AVal->getValue()))
    return false;

  // At this point, the incoming AVRValueHIR must have a RegDDRef pointer
  assert(AVal->isDDRefValue() && isa<RegDDRef>(AVal->getValue()) &&
         "Expected a RegDDRef");
  RegDDRef * RDDR = cast<RegDDRef>(AVal->getValue());
 
  // We don't need to decompose:
  //   - Unitary blobs and standalone IVs with the same Src and Dest types
  //   - Metadata
  if (RDDR->isMetadata() || RDDR->isUnitaryBlob() ||
      RDDR->isStandAloneIV(false /*AllowConversion*/)) {
    return false;
  }

  return true;
}

AVR *HIRDecomposer::decompose(AVRValueHIR *AVal) {
  assert(isa<RegDDRef>(AVal->getValue()) && "Expected a RegDDRef" );
  RegDDRef *RDDR = cast<RegDDRef>(AVal->getValue());

  if (RDDR->isTerminalRef()) {
    return decomposeCanonExpr(RDDR, RDDR->getSingleCanonExpr());
  } else {
    // Memory ops
    return decomposeMemoryOp(AVal);
  }
}

// It decomposes a CanonExpr. CanonExpr's SrcType is used on purpose for coeffs,
// IVs, etc. as conversions (DestType), if any, are handled independently.
AVR *HIRDecomposer::decomposeCanonExpr(RegDDRef *RDDR, CanonExpr *CE) {

  DEBUG(dbgs() << "  Decomposing CanonExpr: ");
  DEBUG(CE->dump());
  DEBUG(dbgs() << "\n");

  // Special case for constant CanonExprs
  if (CE->isConstant()) {
    // Constant CanonExpr may have a hidden conversion
    return decomposeCanonExprConv(
        CE, decomposeCoeff(CE->getConstant(), CE->getSrcType()));
  }

  AVR *CETree = nullptr;

  // Constant Additive
  int64_t AddCoeff = CE->getConstant();
  if (AddCoeff != 0) {
    CETree = decomposeCoeff(AddCoeff, CE->getSrcType());
  }

  // Decompose inductive expression
  for (auto IVIt = CE->iv_begin(), E = CE->iv_end(); IVIt != E; ++IVIt) {
    int64_t IVConstCoeff = CE->getIVConstCoeff(IVIt);

    if (IVConstCoeff != 0) {
      AVR *IVTree = decomposeIV(RDDR, CE, CE->getLevel(IVIt), CE->getSrcType());
      CETree = combineSubTrees(CETree, IVTree, CE->getSrcType(),
                               Instruction::Add);
    }
  }

  // Decompose blobs
  for (auto BlobIt = CE->blob_begin(); BlobIt != CE->blob_end(); ++BlobIt) {
    
    unsigned BlobIdx = CE->getBlobIndex(BlobIt);
    assert(BlobIdx != InvalidBlobIndex && "Invalid blob index!");

    int64_t BlobCoeff = CE->getBlobCoeff(BlobIdx);
    assert(BlobCoeff != 0 && "Invalid blob coefficient!");

    AVR *BlobTree = decomposeBlob(RDDR, BlobIdx, BlobCoeff, DL);
    CETree =
        combineSubTrees(CETree, BlobTree, CE->getSrcType(), Instruction::Add);
  }

  // Decompose denominator
  int64_t Denominator = CE->getDenominator();
  if (Denominator != 1) {
    AVR *DenomTree = decomposeCoeff(Denominator, CE->getSrcType());
    CETree = combineSubTrees(CETree, DenomTree, CE->getSrcType(),
                             CE->isUnsignedDiv() ? Instruction::UDiv
                                                 : Instruction::SDiv);
  }

  assert (CETree && "CanonExpr has not been decomposed");

  // Decompose conversions
  CETree = decomposeCanonExprConv(CE, CETree);

  return CETree;
}

AVR *HIRDecomposer::decomposeCanonExprConv(CanonExpr *CE, AVR *SrcTree) {
  if (CE->isZExt())
    return decomposeConversion(SrcTree, Instruction::ZExt, CE->getDestType());
  else if (CE->isSExt())
    return decomposeConversion(SrcTree, Instruction::SExt, CE->getDestType());
  else if (CE->isTrunc())
    return decomposeConversion(SrcTree, Instruction::Trunc, CE->getDestType());

  return SrcTree;
}

AVRExpression *HIRDecomposer::decomposeMemoryOp(AVRValueHIR *AVal) {
 
  assert(isa<RegDDRef>(AVal->getValue()) && "Expected a RegDDRef" );
  RegDDRef *RDDR = cast<RegDDRef>(AVal->getValue());

  DEBUG(dbgs() << "  Decomposing MemOp:  ");
  DEBUG(RDDR->dump());
  DEBUG(dbgs() << "\n");

  assert (RDDR->hasGEPInfo() && "Expected a GEP RegDDRef");
  SmallVector<AVR *, 2> GepOperands;

  DEBUG(dbgs() << "  Base: ");
  DEBUG(RDDR->getBaseCE()->dump());
  DEBUG(dbgs() << "\n");
 
  // Decompose Base
  AVR *BaseTree = decomposeCanonExpr(RDDR, RDDR->getBaseCE());
  GepOperands.push_back(BaseTree);

  unsigned numDims = RDDR->getNumDimensions();
  DEBUG(dbgs() << "  NumDims: " << numDims << "\n");
  assert (numDims > 0 && "The number of dimensions is 0");

  // Decompose Subscripts
  for (unsigned i = numDims; i > 0; --i) {
    DEBUG(dbgs() << "  Decomposing Dim: " << i << "\n");
    AVR *DimIdx = decomposeCanonExpr(RDDR, RDDR->getDimensionIndex(i));
    GepOperands.push_back(DimIdx);
  }

  // This expression is representing a GEP so we use the type of the BaseCE
  // (pointer)
  DEBUG(dbgs() << "  Creating GEP\n");
  AVRExpression *Result = AVRUtils::createAVRExpression(GepOperands, Instruction::GetElementPtr,
                                       RDDR->getBaseCE()->getDestType());

  // So far, loads always had explicit HLInsts in HIR that were translated to
  // explicit load operations (expressions) in AVR. For example, in the code
  // below, arrays 'b' and 'c' are explicit loads that load data to %0
  // and %1 respectively.
  //
  // + DO i1 = 0, zext.i32.i64(%max_allocno) + -1, 1
  // |   %0 = (@b)[0][i1];
  // |   %1 = (@c)[0][i1];
  // |   (@a)[0][i1] = %0 + %1;
  // + END LOOP
  //
  // LOOP( IV )
  // {
  //   ASSIGN{EXPR{i32 VALUE{i32 %0}} = EXPR{i32 load VALUE{i32* (@b)[0][i1]}}}
  //   ASSIGN{EXPR{i32 VALUE{i32 %1}} = EXPR{i32 load VALUE{i32* (@c)[0][i1]}}}
  //   ...
  // }
  //
  // However, Temp Cleanup in HIR introduced "implicit loads" by removing
  // explicit load HLInsts in some cases. For example, in the code below, arrays
  // 'b' and 'c' are implicit loads. As you can see, there is no load
  // instruction generated in AVR for them.
  //
  // + DO i1 = 0, zext.i32.i64(%max_allocno) + -1, 1
  // |   %add = (@b)[0][i1]  +  (@c)[0][i1];
  // |   (@a)[0][i1] = %add;
  // + END LOOP
  //
  // LOOP( IV )
  // {
  //   ASSIGN{EXPR{float VALUE{float %add}} =
  //       EXPR{float VALUE{float (@b)[0][i1]} fadd VALUE{float (@c)[0][i1]}}}
  //   ...
  // }
  //
  // These implicit loads need special treatment in decomposer as an explicit
  // load has to be generated to have a consistent decomposition. The example
  // below shows how explicit loads are introduced in decomposer.
  //
  // LOOP( IV )
  // {
  //   ASSIGN{(4)EXPR{float (5)VALUE{float %add}} = ...
  //       EXPR{float load (16)EXPR{[64 x float]* getelementptr ... }}}
  //       fadd
  //       EXPR{float load (21)EXPR{[64 x float]* getelementptr ... }}}
  //   ...
  // }
  //

  // Is this an implicit load? We need to add an explicit load.
  // CHECKME: I don't find a cleaner way to know if a load is necessary.
  AVRExpression *Parent;
  if (RDDR->isRval() && !RDDR->isAddressOf() &&
      (Parent = dyn_cast<AVRExpression>(AVal->getParent())) &&
      Parent->getOperation() != Instruction::Load) {
    DEBUG(dbgs() << "  Creating Load\n");
    Result = AVRUtils::createAVRExpression(Result, Instruction::Load,
                                           RDDR->getDestType());
  }

  return Result;
}

AVR *HIRDecomposer::decomposeIV(RegDDRef *RDDR, CanonExpr *CE, unsigned IVLevel,
                                Type *Ty) {
  int64_t IVConstCoeff;
  unsigned IVBlobIndex;
 
  CE->getIVCoeff(IVLevel, &IVBlobIndex, &IVConstCoeff);

  AVR *IVSubTree =
      AVRUtilsHIR::createAVRValueHIR(CE, IVLevel, Ty, nullptr /* parent */);
  AVRLog.push_back(IVSubTree);

  // Create AVRExpression for Blob * IV
  if (IVBlobIndex != InvalidBlobIndex) {
    AVR *IVBlobValue = decomposeBlob(RDDR, IVBlobIndex, 1 /*BlobCoeff*/, DL);
    IVSubTree = combineSubTrees(IVBlobValue, IVSubTree, Ty, Instruction::Mul);
  }

  if (IVConstCoeff != 1) {
    AVR *IVCoeffValue = decomposeCoeff(IVConstCoeff, Ty);
    IVSubTree = combineSubTrees(IVCoeffValue, IVSubTree, Ty, Instruction::Mul);
  }

  return IVSubTree;
}

namespace llvm {
namespace vpo {
AVR *decomposeBlob(RegDDRef *RDDR, unsigned BlobIdx, int64_t BlobCoeff,
                   const DataLayout &DL) {
  BlobTy Blob = RDDR->getBlobUtils().getBlob(BlobIdx);
  AVR *BlobSubTree;

  // Decompose Blob
  BlobDecompVisitor BlobDecomp(*RDDR);
  BlobSubTree = BlobDecomp.visit(Blob);

  // Create AVRExpression for Coeff * Blob
  if (BlobCoeff != 1) {
    Type *BlobTy = Blob->getType();
    Type *CoeffType = Blob->getType();

    // If blob has pointer type, coeff must be integer
    if (BlobTy->isPointerTy()) {
      // If coeff != 1 and blob type is pointer, only -1 coeff is allowed by
      // now.
      assert((BlobCoeff == -1) &&
             "Unexpected blob coefficient for pointer type");

      unsigned PointerSize = DL.getPointerTypeSizeInBits(BlobTy);

      if (PointerSize == 64) {
        CoeffType = Type::getInt64Ty(BlobTy->getContext()); 
      }
      else if (PointerSize == 32) {
        CoeffType = Type::getInt32Ty(BlobTy->getContext()); 
      }
      else {
        llvm_unreachable("Unexpected pointer size!");
      }
    }

    AVR *BlobCoeffValue = decomposeCoeff(BlobCoeff, CoeffType);
    BlobSubTree = combineSubTrees(BlobCoeffValue, BlobSubTree, Blob->getType(),
                                 Instruction::Mul);
  }
    
  return BlobSubTree;
}
}
}

void HIRDecomposer::visit(AVRValueHIR *AVal) {

  DEBUG(dbgs() << "Visiting AVRValueHIR: ");
  DEBUG(AVal->dump());
  DEBUG(dbgs() << "\n");
  DEBUG(dbgs() << "  Needs decomp: " << (needsDecomposition(AVal) ? "Yes\n" : "No\n"));
 
  if (needsDecomposition(AVal)) {
    AVR *SubTree = decompose(AVal);

    assert(SubTree && isa<AVRExpression>(SubTree) &&
           "Decomposition is unnecessary");
    AVRUtils::setDecompTree(AVal, cast<AVRExpression>(SubTree));
    AVRUtils::setParent(SubTree, AVal);
  }
}

// Analysis Pass
AVRDecomposeHIR::AVRDecomposeHIR() : FunctionPass(ID) {
  llvm::initializeAVRDecomposeHIRPass(*PassRegistry::getPassRegistry());
}

void AVRDecomposeHIR::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<AVRGenerateHIR>();
}

// Analysis Utility
bool AVRDecomposeHIR::runOnAvr(AVR *ANode, const DataLayout& DL) {

  HIRDecomposer HIRDecomp(DL);
  AVRVisitor<HIRDecomposer> AVisitor(HIRDecomp);
  AVisitor.visit(ANode, true /*Recursive*/, true /*RecurseInsideLoops*/,
                 false /*RecurseInsideValues*/, true /*Forward*/);

  return false;
}

bool AVRDecomposeHIR::runOnFunction(Function &F) {

  AVRG = &getAnalysis<AVRGenerateHIR>();

  DEBUG(dbgs() << "AVRDecomposerHIR\n");

  const DataLayout& DL = F.getParent()->getDataLayout();

  for (auto I = AVRG->begin(), E = AVRG->end(); I != E; ++I) {
    runOnAvr(&*I, DL);
  }

  DEBUG(dbgs() << "Abstract Layer After Decomposition:\n");
  DEBUG(this->dump(PrintAvrDecomp));

  return false;
}

void AVRDecomposeHIR::print(raw_ostream &OS, unsigned Depth,
                            VerbosityLevel VLevel) const {
#if !INTEL_PRODUCT_RELEASE

  formatted_raw_ostream FOS(OS);

  for (auto I = AVRG->begin(), E = AVRG->end(); I != E; ++I) {
    I->print(FOS, Depth, VLevel);
  }
#endif // !INTEL_PRODUCT_RELEASE
}

void AVRDecomposeHIR::print(raw_ostream &OS, const Module *M) const {
#if !INTEL_PRODUCT_RELEASE
  this->print(OS, 1, PrintAvrDecomp);
#endif // !INTEL_PRODUCT_RELEASE
}

void AVRDecomposeHIR::dump(VerbosityLevel VLevel) const {
  formatted_raw_ostream OS(dbgs());
  this->print(OS, 1, VLevel);
}

void AVRDecomposeHIR::releaseMemory() {
  AVRLog.clear();
  ConstLog.clear();
}
