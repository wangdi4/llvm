//===-------- HLInst.cpp - Implements the HLInst class --------------------===//
//
// Copyright (C) 2015-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLInst class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSparseArrayReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    ShowLLVMInst("hir-details-llvm-inst", cl::init(false), cl::Hidden,
                 cl::desc("Show LLVM instructions instead of dummy HLInst"));

void HLInst::initialize() {
  /// This call is to get around calling virtual functions in the constructor.
  unsigned NumOp = getNumOperandsInternal();

  /// Number of operands stays the same over the lifetime of HLInst so make
  /// that the min size.
  RegDDRefs.resize(NumOp, nullptr);
}

HLInst::HLInst(HLNodeUtils &HNU, Instruction *Inst)
    : HLDDNode(HNU, HLNode::HLInstVal), Inst(Inst),
      CmpOrSelectPred(PredicateTy::FCMP_TRUE), IsSinked(false) {
  assert(Inst && "LLVM Instruction for HLInst cannot be null!");
  initialize();
}

HLInst::HLInst(const HLInst &HLInstObj)
    : HLDDNode(HLInstObj), Inst(HLInstObj.Inst),
      CmpOrSelectPred(HLInstObj.CmpOrSelectPred), IsSinked(HLInstObj.IsSinked) {

  unsigned NumOp, Count = 0;

  initialize();
  NumOp = getNumOperandsInternal();

  /// Clone DDRefs
  for (auto I = HLInstObj.ddref_begin(), E = HLInstObj.ddref_end(); I != E;
       I++, Count++) {
    if (Count < NumOp) {
      setOperandDDRef((*I)->clone(), Count);
    } else {
      auto Ref = (*I);
      auto CloneRef = Ref->clone();

      // Set MaskDDRef to appropriate clone
      if (HLInstObj.MaskDDRef == Ref) {
        MaskDDRef = CloneRef;
      }

      // Every fake ref is added as rval. The number of fake lval ddrefs will be
      // copied by the base class so we will end up with the same number of lval
      // and rval ddrefs.
      addFakeRvalDDRef(CloneRef);
    }
  }
}

HLInst *HLInst::cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                          HLNodeMapper *NodeMapper) const {
  // Call the Copy Constructor
  return new HLInst(*this);
}

HLInst *HLInst::clone(HLNodeMapper *NodeMapper) const {
  return cast<HLInst>(cloneBaseImpl(this, nullptr, nullptr, NodeMapper));
}

bool HLInst::isCopyInst() const {
  if (auto CInst = dyn_cast<CallInst>(Inst))
    if (CInst->getIntrinsicID() == Intrinsic::ssa_copy)
      return true;

  return false;
}

/// TODO: Add beautification logic based on instruction types.
bool HLInst::checkSeparator(formatted_raw_ostream &OS, bool Print) const {

  bool Ret = true;
  unsigned Opcode = Inst->getOpcode();

  if (isa<BinaryOperator>(Inst)) {
    if (Print) {
      if ((Opcode == Instruction::Add) || (Opcode == Instruction::FAdd)) {
        OS << "  +  ";
      } else if ((Opcode == Instruction::Sub) ||
                 (Opcode == Instruction::FSub)) {
        OS << "  -  ";
      } else if ((Opcode == Instruction::Mul) ||
                 (Opcode == Instruction::FMul)) {
        OS << "  *  ";
      } else if (Opcode == Instruction::UDiv) {
        OS << "  /u  ";
      } else if ((Opcode == Instruction::SDiv) ||
                 (Opcode == Instruction::FDiv)) {
        OS << "  /  ";
      } else if (Opcode == Instruction::URem) {
        OS << "  %u  ";
      } else if ((Opcode == Instruction::SRem) ||
                 (Opcode == Instruction::FRem)) {
        OS << "  %  ";
      } else if (Opcode == Instruction::Shl) {
        OS << "  <<  ";
      } else if ((Opcode == Instruction::LShr) ||
                 (Opcode == Instruction::AShr)) {
        OS << "  >>  ";
      } else if (Opcode == Instruction::And) {
        OS << "  &  ";
      } else if (Opcode == Instruction::Or) {
        OS << "  |  ";
      } else if (Opcode == Instruction::Xor) {
        OS << "  ^  ";
      } else {
        llvm_unreachable("Unexpected binary operator!");
      }
    }
  } else if (isa<CmpInst>(Inst)) {
    if (Print) {
      printPredicate(OS, CmpOrSelectPred);
    }
  } else {
    if (!isCallInst()) {
      Ret = false;
    }
    if (Print && !isa<SelectInst>(Inst)) {
      OS << ",  ";
    }
  }

  return Ret;
}

void HLInst::printBeginOpcode(formatted_raw_ostream &OS,
                              bool HasSeparator) const {
#if !INTEL_PRODUCT_RELEASE

  if (auto CInst = dyn_cast<CastInst>(Inst)) {
    OS << CInst->getOpcodeName() << ".";
    OS << *(CInst->getSrcTy());
    OS << ".";
    OS << *(CInst->getDestTy());
    OS << "(";
  } else if (auto *FInst = getCallInst()) {
    if (isIndirectCallInst()) {
      // Use the last operand which is the function pointer.
      (*op_ddref_rbegin())->print(OS, false);
    } else {
      FInst->getCalledOperand()->printAsOperand(OS, false);
    }
    OS << "(";
  } else if (isa<SelectInst>(Inst)) {
    OS << "(";
  } else if (Inst->getOpcode() == Instruction::FNeg) {
    OS << " - ";
  } else if (isa<FreezeInst>(Inst)) {
    OS << "freeze(";
  } else if (!HasSeparator && !isa<LoadInst>(Inst) && !isa<StoreInst>(Inst) &&
             !isa<GEPOrSubsOperator>(Inst) && !isa<CmpInst>(Inst) &&
             !isCopyInst()) {
    OS << Inst->getOpcodeName() << " ";
  }
#endif // !INTEL_PRODUCT_RELEASE
}

void HLInst::printEndOpcode(formatted_raw_ostream &OS) const {
#if !INTEL_PRODUCT_RELEASE
  if (isCallInst() || isa<FreezeInst>(Inst) || isa<CastInst>(Inst)) {
    OS << ")";
  }
#endif // !INTEL_PRODUCT_RELEASE
}

void HLInst::print(formatted_raw_ostream &OS, unsigned Depth,
                   bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE
  bool HasSeparator = checkSeparator(OS, false);

  indent(OS, Depth);

  if (ShowLLVMInst) {
    getLLVMInstruction()->print(OS, true);
    OS << "\n";
    return;
  }

  bool HasLval = hasLval();

  unsigned Count = 0;
  unsigned NumNonBundleOperands = getNumNonBundleOperands();
  auto OpIt = op_ddref_begin();
  auto E = op_ddref_end();

  // Do not print function pointer value of an indirect call as an argument.
  if (isIndirectCallInst()) {
    --E;
  }

  for (; Count < NumNonBundleOperands; ++OpIt, ++Count) {

    if ((Count > 1) || (!HasLval && (Count > 0))) {
      checkSeparator(OS, true);
    }

    if (Count == 0) {
      if (HasLval) {
        *OpIt ? (*OpIt)->print(OS, false) : (void)(OS << *OpIt);

        OS << " = ";
        printBeginOpcode(OS, HasSeparator);

      } else {
        printBeginOpcode(OS, HasSeparator);

        *OpIt ? (*OpIt)->print(OS, false) : (void)(OS << *OpIt);
      }
    } else {
      *OpIt ? (*OpIt)->print(OS, false) : (void)(OS << *OpIt);

      if (isa<SelectInst>(Inst)) {
        if (Count == 1) {
          printPredicate(OS, CmpOrSelectPred);
        } else if (Count == 2) {
          OS << ") ? ";
        } else if (Count == 3) {
          OS << " : ";
        }
      }
    }
  }

  // Print ExtractValueInst\InsertValueInst indices
  auto Inds1 = getExtractValueIndices();
  for (auto I : Inds1)
    OS << ", " << I;
  auto Inds2 = getInsertValueIndices();
  for (auto J : Inds2)
    OS << ", " << J;

  // Print function which no operands (no arguments or return value).
  if (!Count) {
    printBeginOpcode(OS, HasSeparator);
  }

  printEndOpcode(OS);

  if (MaskDDRef) {
    OS << ", Mask = @{";
    MaskDDRef->print(OS, false);
    OS << "}";
  }

  OS << ";";

  unsigned NumOperandBundles = getNumOperandBundles();

  if (NumOperandBundles > 0) {
    OS << " [ ";

    for (unsigned Count = 0; Count < NumOperandBundles; ++Count) {
      if (Count != 0) {
        checkSeparator(OS, true);
      }

      OperandBundleUse OB = getOperandBundleAt(Count);
      OS << OB.getTagName() << "(";

      for (unsigned I = 0, NumOperands = OB.Inputs.size(); I < NumOperands;
           ++I, ++OpIt) {
        *OpIt ? (*OpIt)->print(OS, false) : (void)(OS << *OpIt);
      }

      OS << ")";
    }

    OS << " ] ";
  }

  assert((OpIt == E) && "We missed printing HLInst operand(s)!");

  if (Detailed) {
    FastMathFlags FMF;

    if (isa<SelectInst>(Inst)) {
      FMF = CmpOrSelectPred.FMF;
    } else if (isa<FPMathOperator>(Inst)) {
      FMF = Inst->getFastMathFlags();
    }

    if (FMF.any()) {
      OS << " ";
      printFMF(OS, FMF);
    }
  }

  printDistributePoint(OS);
  printReductionInfo(OS, Detailed);

  OS << "\n";

  HLDDNode::print(OS, Depth, Detailed);
#endif // !INTEL_PRODUCT_RELEASE
}

void HLInst::printReductionInfo(formatted_raw_ostream &OS,
                                bool Detailed) const {
  HIRSafeReductionAnalysis *SRA = this->getHLNodeUtils()
                                      .getHIRFramework()
                                      .getHIRAnalysisProvider()
                                      .get<HIRSafeReductionAnalysis>();
  if (SRA) {
    if (const SafeRedInfo *const RedInfo = SRA->getSafeRedInfo(this)) {
      OS << " ";
      RedInfo->printMarkings(OS, Detailed);
    }
  }

  HIRSparseArrayReductionAnalysis *SARA =
      this->getHLNodeUtils()
          .getHIRFramework()
          .getHIRAnalysisProvider()
          .get<HIRSparseArrayReductionAnalysis>();
  if (SARA && SARA->isSparseArrayReduction(this)) {
    OS << " <Sparse Array Reduction>";
  }
}

RegDDRef *HLInst::getLvalDDRef() {
  if (hasLval()) {
    return getOperandDDRefImpl(0);
  }

  return nullptr;
}

unsigned HLInst::getLvalBlobIndex() const {
    auto *LRef = getLvalDDRef();

    if (!LRef || !LRef->isTerminalRef()) {
      return InvalidBlobIndex;
    }

    auto &BU = getBlobUtils();

    return LRef->isSelfBlob() ? LRef->getSelfBlobIndex()
                        : BU.findTempBlobIndex(LRef->getSymbase());

}

RegDDRef *HLInst::removeLvalDDRef() {
  auto TRef = getLvalDDRef();

  setLvalDDRef(nullptr);

  return TRef;
}

RegDDRef *HLInst::getRvalDDRef() {
  if (hasRval()) {
    return getOperandDDRefImpl(1);
  }

  return nullptr;
}

RegDDRef *HLInst::removeRvalDDRef() {
  auto TRef = getRvalDDRef();

  setRvalDDRef(nullptr);

  return TRef;
}

unsigned HLInst::getNumOperandsInternal() const {
  unsigned NumOp = 0;

  if (isa<GEPOrSubsOperator>(Inst)) {
    // GEP is represented as an assignment of address: %t = &A[i];
    NumOp = 1;
  } else if (auto CInst = dyn_cast<CallInst>(Inst)) {
    // Last operand of call is the function itself. We only count it for
    // indirect calls.
    NumOp = CInst->getNumOperands() - 1;
    // For indirect calls, we add function pointer as an operand.
    if (!CInst->getCalledFunction()) {
      ++NumOp;
    }
  } else {
    NumOp = Inst->getNumOperands();
  }

  if (hasLval() && !isa<StoreInst>(Inst)) {
    ++NumOp;
  }
  // Select instruction gains an extra operand due to inclusion of the
  // predicate.
  if (isa<SelectInst>(Inst)) {
    ++NumOp;
  }

  // ShuffleVectorInst gains an extra operand due to inclusion of the
  // shuffle mask. ShuffleVectorInst stores the mask as a vector of
  // integers internally instead of as an operand. The mask value
  // is converted to a vector in the ShuffleVectorInst constructor.
  if (isa<ShuffleVectorInst>(Inst)) {
    ++NumOp;
  }

  return NumOp;
}

HLDDNode::ddref_iterator HLInst::bundle_op_ddref_begin(unsigned BundleNum) {
  assert(BundleNum < getNumOperandBundles() && "Invalid BundleNum!");

  unsigned BundleOperandCount = 0;

  for (unsigned I = 0; I < BundleNum; ++I) {
    BundleOperandCount += getNumBundleOperands(I);
  }

  return op_ddref_begin() + getNumNonBundleOperands() + BundleOperandCount;
}

bool HLInst::isInPreheaderPostexitImpl(bool Preheader,
                                       const HLLoop *ParLoop) const {

  if (!ParLoop) {
    // Parent of preheader/postexit instructions has to be a loop.
    ParLoop = dyn_cast<HLLoop>(getParent());

  } else {
    assert(ParLoop == getParentLoop() && "Invalid parent loop!");
  }

  if (!ParLoop) {
    return false;
  }

  auto I = Preheader ? ParLoop->pre_begin() : ParLoop->post_begin();
  auto E = Preheader ? ParLoop->pre_end() : ParLoop->post_end();

  // Preheader or postexit is empty.
  if (I == E) {
    return false;
  }

  // If top sort number is available, use it instead.
  if (unsigned TSNum = getTopSortNum()) {
    assert(isAttached() && "It is illegal to call top sort number dependent "
                           "utility on disconnected node!");
    return Preheader ? (TSNum <= std::prev(E)->getTopSortNum())
                     : (TSNum >= I->getTopSortNum());
  }

  for (; I != E; I++) {
    if (cast<HLInst>(I) == this) {
      return true;
    }
  }

  return false;
}

void HLInst::verify() const {
  bool IsSelectCmpTrueFalse = (isa<SelectInst>(Inst) || isa<CmpInst>(Inst)) &&
                              isPredicateTrueOrFalse(CmpOrSelectPred);

  assert((CmpInst::isFPPredicate(CmpOrSelectPred) ||
          CmpInst::isIntPredicate(CmpOrSelectPred) ||
          CmpOrSelectPred == UNDEFINED_PREDICATE) &&
         "Invalid predicate value, should be one of PredicateTy");

  HLDDNode::verify();

  if (IsSelectCmpTrueFalse) {
    assert(getOperandDDRef(1)->isStandAloneUndefBlob() &&
           getOperandDDRef(2)->isStandAloneUndefBlob() &&
           "DDRefs for Select or Cmp Instruction with "
           "True or False predicate must be undefined");

  } else if (isa<LoadInst>(Inst)) {
    assert(getRvalDDRef()->isMemRef() &&
           "Rval of load instruction is not a memref!");
    assert(getLvalDDRef()->isTerminalRef() &&
           "Lval of load instruction is not a terminal ref!");

  } else if (isa<StoreInst>(Inst)) {
    assert(getLvalDDRef()->isMemRef() &&
           "Lval of store instruction is not a memref!");

  } else if (isa<GetElementPtrInst>(Inst)) {
    assert(getRvalDDRef()->isAddressOf() &&
           "Rval of GEP instruction is not an AddressOf ref!");
    assert(getLvalDDRef()->isTerminalRef() &&
           "Lval of GEP instruction is not a terminal ref!");

  } else if (isCopyInst()) {
    assert(getLvalDDRef()->isTerminalRef() &&
           "Lval of copy instruction is not a terminal!");
    assert((getRvalDDRef()->isTerminalRef() || getRvalDDRef()->isAddressOf()) &&
           "Rval of copy instruction is not a terminal or an AddressOf ref!");
  } else if (isCallInst()) {
    for (auto FakeIter = fake_ddref_begin(), FakeEndIter = fake_ddref_end();
         FakeIter != FakeEndIter; ++FakeIter) {
      auto *FakeRef = *FakeIter;
      if (FakeRef->isMemRef()) {
        bool hasMatchingOpRef = false;
        (void)hasMatchingOpRef;
        for (auto OpIter = op_ddref_begin(), OpEndIter = op_ddref_end();
             OpIter != OpEndIter; ++OpIter) {
          auto *OpRef = *OpIter;
          if (OpRef->isAddressOf()) {
            if (DDRefUtils::areEqualWithoutAddressOf(FakeRef, OpRef)) {
              hasMatchingOpRef = true;
              break;
            }
          }
        }
        assert(hasMatchingOpRef && "Fake ref has no matching op dd ref!");
      }
    }
  }
}

bool HLInst::isIntrinCall(Intrinsic::ID &IntrinID) const {
  if (auto *Intrin = getIntrinCall()) {
    IntrinID = Intrin->getIntrinsicID();
    return true;
  }

  return false;
}

bool HLInst::isDirective(int DirectiveID) const {
  auto *Call = getIntrinCall();
  if (!Call)
    return false;

  return vpo::VPOAnalysisUtils::getRegionDirectiveID(Call) == DirectiveID;
}

bool HLInst::isAutoVecDirective() const {
  return isDirective(DIR_VPO_AUTO_VEC);
}

bool HLInst::isValidReductionOpCode(unsigned OpCode) {
  // Start with these initially - when adding a new opcode ensure
  // that we also add changes to get reduction identity in
  // getRecurrenceIdentity below.
  switch (OpCode) {
  case Instruction::FAdd:
  case Instruction::FSub:
  case Instruction::FMul:
  case Instruction::Add:
  case Instruction::Sub:
  case Instruction::Mul:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor:
  case Instruction::Select:
  case Instruction::SDiv:
  case Instruction::UDiv:
  case Instruction::FDiv:
    return true;
  default:
    return false;
  }
}

bool HLInst::isReductionOp(unsigned *OpCode) const {
  const Instruction *LLVMInst = getLLVMInstruction();

  if (isa<BinaryOperator>(LLVMInst)) {
    unsigned OpC = LLVMInst->getOpcode();

    if (OpCode) {
      *OpCode = OpC;
    }

    return isValidReductionOpCode(OpC);

  } else if (isMinOrMax()) {

    if (OpCode) {
      *OpCode = Instruction::Select;
    }

    return true;
  }

  return false;
}

bool HLInst::checkMinMax(bool IsMin, bool IsMax) const {
  Intrinsic::ID Id;

  if (isIntrinCall(Id)) {
    if (IsMin) {
      switch (Id) {
      case Intrinsic::minnum:
      case Intrinsic::minimum:
      case Intrinsic::smin:
      case Intrinsic::umin:
        return true;
      default:
        break;
      }
    }

    if (IsMax) {
      switch (Id) {
      case Intrinsic::maxnum:
      case Intrinsic::maximum:
      case Intrinsic::smax:
      case Intrinsic::umax:
        return true;
      default:
        break;
      }
    }

    return false;
  }

  if (!isa<SelectInst>(Inst)) {
    return false;
  }

  // Get operands and predicate
  const RegDDRef *Operand1, *Operand2, *Operand3, *Operand4;
  Operand1 = getOperandDDRef(1);
  Operand2 = getOperandDDRef(2);
  Operand3 = getOperandDDRef(3);
  Operand4 = getOperandDDRef(4);

  PredicateTy Pred = getPredicate();

  // Operand pattern: x .. y ? x : y
  bool OneAndThree = DDRefUtils::areEqual(Operand1, Operand3) &&
                     DDRefUtils::areEqual(Operand2, Operand4);

  // Operand pattern: OneAndThree OR x .. y ? y : x (OneAndFour)
  if (OneAndThree || (DDRefUtils::areEqual(Operand1, Operand4) &&
                      DDRefUtils::areEqual(Operand2, Operand3))) {
    // min pattern: x >(=) y ? y : x    max pattern: x >(=) y ? x : y
    if ((!OneAndThree && IsMin) || (OneAndThree && IsMax)) {
      if (Pred == PredicateTy::ICMP_SGE || Pred == PredicateTy::ICMP_SGT ||
          Pred == PredicateTy::FCMP_OGE || Pred == PredicateTy::FCMP_OGT ||
          Pred == PredicateTy::ICMP_UGE || Pred == PredicateTy::ICMP_UGT) {
        return true;
      }
    }
    // min pattern: x <(=) y ? x : y    max pattern: x <(=) y ? y : x
    if ((OneAndThree && IsMin) || (!OneAndThree && IsMax)) {
      if (Pred == PredicateTy::ICMP_SLE || Pred == PredicateTy::ICMP_SLT ||
          Pred == PredicateTy::FCMP_OLE || Pred == PredicateTy::FCMP_OLT ||
          Pred == PredicateTy::ICMP_ULE || Pred == PredicateTy::ICMP_ULT) {
        return true;
      }
    }
  }

  return false;
}

bool HLInst::isAbs() const {

  if (!isa<SelectInst>(Inst)) {
    return false;
  }

  PredicateTy Pred = getPredicate();

  if (!CmpInst::isIntPredicate(Pred)) {
    return false;
  }

  // Assume canonical form, which means in the compare instruction constant will
  // always be the second operand. Here are the possibilities-
  // 1) T > 0 ? T : -T
  // 2) T >= 0 ? T : -T
  // 3) T < 0 ? -T : T  (operand swap needed)
  // 4) T <= 0 ? -T : T (operand swap needed)
  // 5) T > -1 ? T : -T
  // 6) T < 1 ? -T : T (operand swap needed)
  auto Operand1 = getOperandDDRef(1);

  if (!Operand1->isTerminalRef()) {
    return false;
  }

  auto Operand2 = getOperandDDRef(2);

  int64_t ConstVal;

  if (!Operand2->isIntConstant(&ConstVal) &&
      !Operand2->isIntConstantSplat(&ConstVal)) {
    return false;
  }

  bool SwapOperands = false;

  if (ConstVal == 0) {
    if ((Pred != PredicateTy::ICMP_SGT) && (Pred != PredicateTy::ICMP_SGE)) {
      if ((Pred == PredicateTy::ICMP_SLT) || (Pred == PredicateTy::ICMP_SLE)) {
        SwapOperands = true;
      } else {
        return false;
      }
    }
  } else if (ConstVal == -1) {
    if (Pred != PredicateTy::ICMP_SGT) {
      return false;
    }
  } else if (ConstVal == 1) {
    if (Pred != PredicateTy::ICMP_SLT) {
      return false;
    }
    SwapOperands = true;

  } else {
    return false;
  }

  auto Operand3 = getOperandDDRef(3);
  auto Operand4 = getOperandDDRef(4);

  if (SwapOperands) {
    std::swap(Operand3, Operand4);
  }

  if (!DDRefUtils::areEqual(Operand1, Operand3)) {
    return false;
  }

  if (!Operand4->isTerminalRef()) {
    return false;
  }

  auto CE1 = Operand1->getSingleCanonExpr();
  auto CE4 = Operand4->getSingleCanonExpr();

  // We are cheating here by modifying CE4 to avoid cloning but we will restore
  // it before returning.
  // We also assume that negating a CanonExpr twice yields the original CE.
  CanonExpr *NonConstCE4 = const_cast<CanonExpr *>(CE4);

  NonConstCE4->negate();

  bool Ret = CanonExprUtils::areEqual(CE1, NonConstCE4);

  NonConstCE4->negate();

  return Ret;
}

Constant *HLInst::getRecurrenceIdentity(unsigned RednOpCode, Type *Ty,
                                        FastMathFlags FMF, bool IsMin,
                                        bool IsSigned) {
  RecurKind RDKind;

  assert(isValidReductionOpCode(RednOpCode) &&
         "Expected a valid reduction opcode");

  switch (RednOpCode) {
  case Instruction::FAdd:
  case Instruction::FSub:
    RDKind = RecurKind::FAdd;
    break;

  case Instruction::Add:
  case Instruction::Sub:
    RDKind = RecurKind::Add;
    break;

  case Instruction::FMul:
  case Instruction::FDiv:
    RDKind = RecurKind::FMul;
    break;

  case Instruction::Mul:
  case Instruction::SDiv:
  case Instruction::UDiv:
    RDKind = RecurKind::Mul;
    break;

  case Instruction::And:
    RDKind = RecurKind::And;
    break;

  case Instruction::Or:
    RDKind = RecurKind::Or;
    break;

  case Instruction::Xor:
    RDKind = RecurKind::Xor;
    break;

  case Instruction::Select:
    if (Ty->isIntegerTy()) {
      if (IsSigned)
        RDKind = IsMin? RecurKind::SMin : RecurKind::SMax;
      else
        RDKind = IsMin? RecurKind::UMin : RecurKind::UMax;
    } else {
      assert(Ty->isFloatingPointTy() &&
             "Floating point type expected at this point!");
      RDKind = IsMin? RecurKind::FMin : RecurKind::FMax;
    }
    break;

  default:
    llvm_unreachable("Unexpected reduction opcode");
    break;
  }

  return RecurrenceDescriptor::getConstRecurrenceIdentity(RDKind, Ty, FMF);
}

const DebugLoc HLInst::getDebugLoc() const {
  auto *LRef = getLvalDDRef();
  if (LRef && LRef->hasGEPInfo()) {
    return LRef->getDebugLoc();
  }

  auto *RRef = getRvalDDRef();
  if (RRef && RRef->hasGEPInfo()) {
    return RRef->getDebugLoc();
  }

  return Inst->getDebugLoc();
}

bool HLInst::isSameOperationAs(const HLInst *HInst, bool Relaxed) const {
  auto *Inst1 = getLLVMInstruction();
  auto *Inst2 = HInst->getLLVMInstruction();

  // Instruction::isSameOperationAs() will check opcodes, type, operand
  // types and haveSameSpecialState() of the underlying LLVMInstruction
  if (!Inst1->isSameOperationAs(Inst2)) {

    // GEP/Subscript and copy instructions can correspond to each other in HIR
    // so we should allow them. Inside HIR clients create copy instructions
    // instead of GEPs due to its ease. Allow correspondance in relaxed mode
    // when the operand types are equal.
    //
    // For example-
    //
    // %t1 = &(%A)[4*i1]  // can be copy inst
    //
    // %t2 = &(%A)[4*i1 + 1] // can be GEP inst
    if (!Relaxed ||
        (!this->isCopyInst() || !isa<GEPOrSubsOperator>(Inst2)) &&
            (!HInst->isCopyInst() || !isa<GEPOrSubsOperator>(Inst1))) {
      return false;
    }

    // Do type check here and do not continue for Relaxed mode.
    return (Inst1->getType() == Inst2->getType());
  }

  if (isa<CmpInst>(Inst1) || isa<SelectInst>(Inst1)) {
    if (this->getPredicate() != HInst->getPredicate()) {
      return false;
    }
  }

  if (auto *FPInst1 = dyn_cast<FPMathOperator>(Inst1)) {
    auto *FPInst2 = cast<FPMathOperator>(Inst2);

    if (FPInst1->getFastMathFlags() != FPInst2->getFastMathFlags()) {
      return false;
    }
  }

  if (auto *BinOpInst1 = dyn_cast<OverflowingBinaryOperator>(Inst1)) {
    auto *BinOpInst2 = cast<OverflowingBinaryOperator>(Inst2);

    if ((BinOpInst1->hasNoUnsignedWrap() != BinOpInst2->hasNoUnsignedWrap()) ||
        (BinOpInst1->hasNoSignedWrap() != BinOpInst2->hasNoSignedWrap())) {
      return false;
    }
  }

  return true;
}

int OMPRegionProxy::getOmpRegionEntryDir(const HLInst *I) {
  bool IsEntry = false;
  const Instruction *LI = I->getLLVMInstruction();
  int Res = vpo::VPOAnalysisUtils::getRegionDirectiveID(LI, &IsEntry);
  return IsEntry ? Res : -1;
}

int OMPRegionProxy::getOmpRegionExitDir(const HLInst *Exit,
                                        const HLInst *Entry) {
  int RegEntryDir = Entry ? getOmpRegionEntryDir(Entry) : -1;
  (void)RegEntryDir;
  assert((!Entry || (RegEntryDir >= 0)) && "must be region entry");
  const Instruction *LI = Exit->getLLVMInstruction();
  bool IsEntry = false;
  int RegExitDir = vpo::VPOAnalysisUtils::getRegionDirectiveID(LI, &IsEntry);

  if (RegExitDir < 0 || IsEntry)
    return -1;
  if (!Entry)
    return RegExitDir;
  // Entry is not NULL - additionally check it matches the exit:
  if (!DDRefUtils::areEqual(Entry->getLvalDDRef(),
                            *Exit->rval_op_ddref_begin()))
    return -1;
  assert(RegExitDir ==
             vpo::VPOAnalysisUtils::getMatchingEndDirective(RegEntryDir) &&
         "OMP region entry/exit mismatch");
  return RegExitDir;
}

int OMPRegionProxy::getOmpClauseID(unsigned I) const {
  const OperandBundleUse &OBU = Impl->getOperandBundleAt(I + 1);
  return vpo::VPOAnalysisUtils::getClauseID(OBU.getTagName());
}
