//===-- VPOAvrGenerate.cpp ------------------------------------------------===//
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
/// This file implements the AVR Decomposition Pass.
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

AVRContainerTy AVRLog;
// TODO
//iplist<Constant> ConstLog;

static AVRValueHIR *decomposeCoeff(Constant *Const) {
  AVRValueHIR *AVal = AVRUtilsHIR::createAVRValueHIR(Const, nullptr /*Parent*/);
  AVRLog.push_back(AVal);
  return AVal;
}

static AVRValueHIR *decomposeCoeff(int64_t Coeff, Type *Type) {
  // CHECKME: getSigned
  Constant *ConstCoeff = ConstantInt::getSigned(Type, Coeff);
  //ConstLog.push_back(ConstCoeff);
  return decomposeCoeff(ConstCoeff);
}

// It creates a new AVRExpressionHIR if LHS and RHS are not nullptr.
// Otherwise, it returns the AVR that is not nullptr
static AVR *combineSubTrees(AVR *LHS, AVR *RHS, Type *Ty, unsigned OpCode) {

  assert((LHS != nullptr || RHS != nullptr) && "LHS and RHS cannot be nullptr");

  if (LHS == nullptr)
    return RHS;
  else if (RHS == nullptr)
    return LHS;
  else {
    AVRExpressionHIR *AExpr =
        AVRUtilsHIR::createAVRExpressionHIR(LHS, RHS, Ty, OpCode);
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

  AVRValueHIR *decomposeSelfBlobSLEV(const SCEV *SC);
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

// It decomposes a blob DDRef given its SCEV
AVRValueHIR *BlobDecompVisitor::decomposeSelfBlobSLEV(const SCEV *SC) {

  unsigned BlobIndex = BlobUtils::findBlob(SC);
  assert(BlobIndex != InvalidBlobIndex && "SCEV is not a Blob");

  // Self blobs will always have a BlobDDRed
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
  auto Op = SCOperands.begin();
  AVR *ExprTree = visit(*Op);
  Op++;

  for (const auto OpEnd = SCOperands.end(); Op != OpEnd; Op++) {
    AVR *OpTree = visit(*Op);
    ExprTree = combineSubTrees(OpTree, ExprTree, SC->getType(), OpCode);
  }

  return ExprTree;
}

AVR *BlobDecompVisitor::visitConstant(const SCEVConstant *Constant) {
  return decomposeCoeff(Constant->getValue());
}

// TODO: Skipping conversions by now
AVR *BlobDecompVisitor::visitTruncateExpr(const SCEVTruncateExpr *Expr) {
  return visit(Expr->getOperand());
}

// TODO: Skipping conversions by now
AVR *BlobDecompVisitor::visitZeroExtendExpr(const SCEVZeroExtendExpr *Expr) {
  return visit(Expr->getOperand());
}

// TODO: Skipping conversions by now
AVR *BlobDecompVisitor::visitSignExtendExpr(const SCEVSignExtendExpr *Expr) {
  return visit(Expr->getOperand());
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
  llvm_unreachable("SMaxExpr not supported yet!");
}

AVR *BlobDecompVisitor::visitUMaxExpr(const SCEVUMaxExpr *Expr) {
  llvm_unreachable("UMaxExpr not supported yet!");
}

AVR *BlobDecompVisitor::visitUnknown(const SCEVUnknown *Expr) {
  return decomposeSelfBlobSLEV(Expr);
}

AVR *BlobDecompVisitor::visitCouldNotCompute(const SCEVCouldNotCompute *Expr) {
  llvm_unreachable("Attempt to use a SCEVCouldNotCompute object!");
}

// This class implements the main visitor of the AVRValueHIR decomposition.
// It has the coarse-grained functionality: decomposition of a RegDDRef, CanonExpr,
// IVs, and generic blobs.
class HIRDecomposer {

private:
  bool needsDecomposition(AVRValueHIR *AVal);
  AVR *decompose(AVRValueHIR *AVal);

  AVR *decomposeCanonExpr(RegDDRef *RDDR, CanonExpr *CE);
  AVR *decomposeMemoryOp() {return nullptr; /*TODO*/}
  AVR *decomposeIV(RegDDRef *RDDR, CanonExpr *CE, unsigned IVLevel, Type *Ty);
  AVR *decomposeBlob(RegDDRef *RDDR, unsigned BlobIdx, int64_t BlobCoeff);

public:
  HIRDecomposer() {}

  /// Visit Functions
  void visit(AVR *AVR){};
  void visit(AVRValueHIR *AVal);
  void postVisit(AVR *AVR){};
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }
};

bool HIRDecomposer::needsDecomposition(AVRValueHIR *AVal) {

  // We don't need to decompose:
  //   - Constants
  //   - BlobDDRefs (already decomposed)
  //   - IVs (already decomposed)
  if (AVal->isConstant() || isa<BlobDDRef>(AVal->getValue()) ||
      AVal->isIVValue())
    return false;

  // At this point, the incoming AVRValueHIR must have a RegDDRef pointer
  assert(isa<RegDDRef>(AVal->getValue()) && "Expected a RegDDRef" );
  RegDDRef * RDDR = cast<RegDDRef>(AVal->getValue());
 
  // We don't need to decompose:
  //   - Self blobs
  //   - Null pointers
  //   - Metadata
  // TODO:
  //   - Memory Ops (GEP + SingleCanonExpr)
  if (RDDR->isSelfBlob() || RDDR->isMetadata() || RDDR->isNull() ||
      RDDR->isLval() || RDDR->hasGEPInfo() || !RDDR->isSingleCanonExpr()) {
    return false;
  }

  return true;
}

AVR * HIRDecomposer::decompose(AVRValueHIR *AVal) {
  assert(isa<RegDDRef>(AVal->getValue()) && "Expected a RegDDRef" );
  RegDDRef * RDDR = cast<RegDDRef>(AVal->getValue());

  //TODO: Memory ops
  assert(RDDR->isSingleCanonExpr() && "Expected a single CanonExpr" );
  
  return decomposeCanonExpr(RDDR, RDDR->getSingleCanonExpr());
}

AVR *HIRDecomposer::decomposeCanonExpr(RegDDRef *RDDR, CanonExpr *CE) {
  AVR *CETree = nullptr;

  // TODO: The type of new AVRExpressionHIRs/AVRValueHIRs is not set
  // appropriately. Currently I'm mainly using the src type of the CE, which is
  // wrong. As AVRExpressionHIR/AVValueHIR will have type, I wonder if it's worth
  // moving Type to AVR even though some sub-classes will set it to nullptr.
  // This is to avoid:
  //
  // Type * type;
  // if (AVRExpression expr = dyn_cast<AVRExpression>(CETree))
  //   type = expr.getType();
  // else AVRValue val = dyn_cast...
  //   type = val.getType();
  // else
  //   llvm_unreachable(...);

  // Constant Additive
  int64_t AddCoeff = CE->getConstant();
  if (AddCoeff != 0) {
    // TODO: Type
    CETree = decomposeCoeff(AddCoeff, CE->getSrcType());
  }

  // Decompose inductive expression
  int i = 1;
  for (auto IVIt = CE->iv_begin(); IVIt != CE->iv_end(); IVIt++, i++) {
    int64_t IVConstCoeff = CE->getIVConstCoeff(IVIt);

    if (IVConstCoeff != 0) {
      // TODO: Type
      AVR *IVTree = decomposeIV(RDDR, CE, i, CE->getSrcType());
      CETree = combineSubTrees(CETree, IVTree, CE->getSrcType(),
                               Instruction::Add);
    }
  }

  // Decompose blobs
  for (auto BlobIt = CE->blob_begin(); BlobIt != CE->blob_end();
       BlobIt++, i++) {
    unsigned BlobIdx = CE->getBlobIndex(BlobIt);

    if (BlobIdx != InvalidBlobIndex) {
      int64_t BlobCoeff = CE->getBlobCoeff(BlobIdx);

      if (BlobCoeff != 0) {
        AVR *BlobTree = decomposeBlob(RDDR, BlobIdx, BlobCoeff);
        //TODO: Type
        CETree =
            combineSubTrees(CETree, BlobTree, CE->getSrcType(), Instruction::Add);
      }
    }
  }

  // Decompose denominator
  int64_t Denominator = CE->getDenominator();
  if (Denominator != 1) {
    // TODO: type
    AVR *DenomTree = decomposeCoeff(Denominator, CE->getSrcType());
    CETree =
        combineSubTrees(CETree, DenomTree, CE->getSrcType(), Instruction::UDiv);
  }

  return CETree;
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
    AVR *IVBlobValue = decomposeBlob(RDDR, IVBlobIndex, 1 /*BlobCoeff*/);
    IVSubTree = combineSubTrees(IVBlobValue, IVSubTree, Ty, Instruction::Mul);
  }

  if (IVConstCoeff != 1) {
    // CHECKME: SrcType
    AVR *IVCoeffValue = decomposeCoeff(IVConstCoeff, Ty);
    IVSubTree = combineSubTrees(IVCoeffValue, IVSubTree, Ty, Instruction::Mul);
  }

  return IVSubTree;
}

AVR *HIRDecomposer::decomposeBlob(RegDDRef *RDDR, unsigned BlobIdx,
                                  int64_t BlobCoeff) {
  BlobTy Blob = BlobUtils::getBlob(BlobIdx);
  AVR *BlobSubTree;

  // TODO: Reuse the visitor: One visitor for all blobs. RDDR is the same
  BlobDecompVisitor BlobDecomp(*RDDR);
  BlobSubTree = BlobDecomp.visit(Blob);

  // Create AVRExpression for Coeff * Blob
  if (BlobCoeff != 1) {
    // CHECKME: type
    AVR *BlobCoeffValue = decomposeCoeff(BlobCoeff, Blob->getType());
    BlobSubTree = combineSubTrees(BlobCoeffValue, BlobSubTree, Blob->getType(),
                                 Instruction::Mul);
  }
    
  return BlobSubTree;
}

void HIRDecomposer::visit(AVRValueHIR *AVal) {
  
  if (needsDecomposition(AVal)) {
    AVR *SubTree = decompose(AVal);

    assert(SubTree != nullptr && isa<AVRExpression>(SubTree) &&
           "Decomposition is unnecessary");
    AVRUtils::setDecompTree(AVal, SubTree);
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
bool AVRDecomposeHIR::runOnAvr(AVR *ANode) {

  HIRDecomposer HIRDecomp;
  AVRVisitor<HIRDecomposer> AVisitor(HIRDecomp);
  AVisitor.visit(ANode, true /*Recursive*/, true /*RecurseInsideLoops*/,
                 true /*RecurseInsideValues*/, true /*Forward*/);

  return false;
}

bool AVRDecomposeHIR::runOnFunction(Function &F) {

  AVRG = &getAnalysis<AVRGenerateHIR>();

  HIRDecomposer HIRDecomp;
  AVRVisitor<HIRDecomposer> AVisitor(HIRDecomp);
  AVisitor.forwardVisitAll(AVRG);

  return false;
}

void AVRDecomposeHIR::print(raw_ostream &OS, unsigned Depth,
                            VerbosityLevel VLevel) const {

  formatted_raw_ostream FOS(OS);

  for (auto I = AVRG->begin(), E = AVRG->end(); I != E; ++I) {
    I->print(FOS, Depth, VLevel);
  }
}

void AVRDecomposeHIR::print(raw_ostream &OS, const Module *M) const {
  this->print(OS, 1, PrintAvrDecomp);
}

void AVRDecomposeHIR::dump(VerbosityLevel VLevel) const {
  formatted_raw_ostream OS(dbgs());
  this->print(OS, 1, VLevel);
}

void AVRDecomposeHIR::releaseMemory() {
  AVRLog.clear();
  //ConstLog.clear();
}

