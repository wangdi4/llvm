//===-------- HLInst.cpp - Implements the HLInst class --------------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
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
      CmpOrSelectPred(PredicateTy::FCMP_TRUE) {
  assert(Inst && "LLVM Instruction for HLInst cannot be null!");
  initialize();
}

HLInst::HLInst(const HLInst &HLInstObj)
    : HLDDNode(HLInstObj), Inst(HLInstObj.Inst),
      CmpOrSelectPred(HLInstObj.CmpOrSelectPred) {

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

  if (auto BCInst = dyn_cast<BitCastInst>(Inst)) {
    if (BCInst->getSrcTy() == BCInst->getDestTy()) {
      return true;
    }
  }

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
        OS << "  &&  ";
      } else if (Opcode == Instruction::Or) {
        OS << "  ||  ";
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
    if (!isa<CallInst>(Inst)) {
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

  if (auto CInst = dyn_cast<CastInst>(Inst)) {
    if (!isCopyInst()) {
      OS << CInst->getOpcodeName() << ".";
      OS << *(CInst->getSrcTy());
      OS << ".";
      OS << *(CInst->getDestTy());
      OS << "(";
    }
  } else if (auto FInst = dyn_cast<CallInst>(Inst)) {
    FInst->getCalledValue()->printAsOperand(OS, false);
    OS << "(";
  } else if (isa<SelectInst>(Inst)) {
    OS << "(";
  } else if (!HasSeparator && !isa<LoadInst>(Inst) && !isa<StoreInst>(Inst) &&
             !isa<GetElementPtrInst>(Inst) && !isa<CmpInst>(Inst)) {
    OS << Inst->getOpcodeName() << " ";
  }
}

void HLInst::printEndOpcode(formatted_raw_ostream &OS) const {
  if (isa<CallInst>(Inst) || (isa<CastInst>(Inst) && !isCopyInst())) {
    OS << ")";
  }
}

void HLInst::print(formatted_raw_ostream &OS, unsigned Depth,
                   bool Detailed) const {
  unsigned Count = 0;
  bool HasSeparator = checkSeparator(OS, false);

  indent(OS, Depth);

  if (ShowLLVMInst) {
    getLLVMInstruction()->print(OS, true);
    OS << "\n";
    return;
  }

  for (auto I = op_ddref_begin(), E = op_ddref_end(); I != E; ++I, ++Count) {
    if ((Count > 1) || (!hasLval() && (Count > 0))) {
      checkSeparator(OS, true);
    }

    if (Count == 0) {
      if (hasLval()) {
        *I ? (*I)->print(OS, false) : (void)(OS << *I);

        OS << " = ";
        printBeginOpcode(OS, HasSeparator);

      } else {
        printBeginOpcode(OS, HasSeparator);

        *I ? (*I)->print(OS, false) : (void)(OS << *I);
      }
    } else {
      *I ? (*I)->print(OS, false) : (void)(OS << *I);

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

  // Print function which no operands (no arguments or return value).
  if (!Count) {
    printBeginOpcode(OS, HasSeparator);
  }

  printEndOpcode(OS);

  OS << ";";

  if (MaskDDRef) {
    OS << " Mask = @{";
    MaskDDRef->print(OS, false);
    OS << "}";
  }

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

  OS << "\n";

  HLDDNode::print(OS, Depth, Detailed);
}

bool HLInst::hasLval() const {
  /// The following logic is copied from AssemblyWriter::printInstruction().
  /// TODO: Is there a better way to determine this, probably by checking
  /// non-zero uses?
  return (Inst->hasName() || !Inst->getType()->isVoidTy() ||
          isa<StoreInst>(Inst));
}

RegDDRef *HLInst::getLvalDDRef() {
  if (hasLval()) {
    return getOperandDDRefImpl(0);
  }

  return nullptr;
}

const RegDDRef *HLInst::getLvalDDRef() const {
  return const_cast<HLInst *>(this)->getLvalDDRef();
}

void HLInst::setLvalDDRef(RegDDRef *RDDRef) {
  assert(hasLval() && "This instruction does not have an lval!");

  setOperandDDRefImpl(RDDRef, 0);
}

RegDDRef *HLInst::removeLvalDDRef() {
  auto TRef = getLvalDDRef();

  setLvalDDRef(nullptr);

  return TRef;
}

bool HLInst::hasRval() const {
  return (isa<StoreInst>(Inst) || isa<GetElementPtrInst>(Inst) ||
          (hasLval() && isa<UnaryInstruction>(Inst)));
}

RegDDRef *HLInst::getRvalDDRef() {
  if (hasRval()) {
    return getOperandDDRefImpl(1);
  }

  return nullptr;
}

const RegDDRef *HLInst::getRvalDDRef() const {
  return const_cast<HLInst *>(this)->getRvalDDRef();
}

void HLInst::setRvalDDRef(RegDDRef *Ref) {
  assert(hasRval() && "This instruction does not have a rval!");

  setOperandDDRefImpl(Ref, 1);
}

RegDDRef *HLInst::removeRvalDDRef() {
  auto TRef = getRvalDDRef();

  setRvalDDRef(nullptr);

  return TRef;
}

bool HLInst::isLval(const RegDDRef *Ref) const {
  assert((this == Ref->getHLDDNode()) && "Ref does not belong to this node!");

  return ((getLvalDDRef() == Ref) || isFakeLval(Ref));
}

bool HLInst::isRval(const RegDDRef *Ref) const { return !isLval(Ref); }

unsigned HLInst::getNumOperands() const { return getNumOperandsInternal(); }

unsigned HLInst::getNumOperandsInternal() const {
  unsigned NumOp = 0;

  if (isa<GetElementPtrInst>(Inst)) {
    // GEP is represented as an assignment of address: %t = &A[i];
    // TODO: GEP accessing structure elements
    NumOp = 1;
  } else if (auto CInst = dyn_cast<CallInst>(Inst)) {
    NumOp = CInst->getNumArgOperands();
  } else {
    NumOp = Inst->getNumOperands();
  }

  if (hasLval() && !isa<StoreInst>(Inst)) {
    NumOp++;
  }
  // Select instruction gains an extra operand due to inclusion of the
  // predicate.
  if (isa<SelectInst>(Inst)) {
    NumOp++;
  }

  return NumOp;
}

bool HLInst::isInPreheaderPostexitImpl(bool Preheader) const {
  auto HLoop = getParentLoop();

  if (!HLoop) {
    return false;
  }

  auto I = Preheader ? HLoop->pre_begin() : HLoop->post_begin();
  auto E = Preheader ? HLoop->pre_end() : HLoop->post_end();

  for (; I != E; I++) {
    if (cast<HLInst>(I) == this) {
      return true;
    }
  }

  return false;
}

bool HLInst::isInPreheader() const { return isInPreheaderPostexitImpl(true); }

bool HLInst::isInPostexit() const { return isInPreheaderPostexitImpl(false); }

bool HLInst::isInPreheaderOrPostexit() const {
  return (isInPreheader() || isInPostexit());
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
  }

  if (isa<LoadInst>(Inst)) {
    assert(getRvalDDRef()->isMemRef() &&
           "Rval of load instruction is not a memref!");
  }

  if (isa<StoreInst>(Inst)) {
    assert(getLvalDDRef()->isMemRef() &&
           "Lval of store instruction is not a memref!");
  }

  if (isa<GetElementPtrInst>(Inst)) {
    assert(getRvalDDRef()->isAddressOf() &&
           "Rval of GEP instruction is not an AddressOf ref!");
  }
}

bool HLInst::isIntrinCall(Intrinsic::ID &IntrinID) const {
  auto Call = dyn_cast<IntrinsicInst>(getLLVMInstruction());
  if (!Call) {
    return false;
  }
  IntrinID = Call->getIntrinsicID();
  return true;
}

bool HLInst::isIntelDirective(int DirectiveID) const {
  Intrinsic::ID IntrinID;

  if (!isIntrinCall(IntrinID) ||
      !vpo::VPOAnalysisUtils::isIntelDirective(IntrinID)) {
    return false;
  }

  auto Call = dyn_cast<IntrinsicInst>(getLLVMInstruction());
  auto DirStr = vpo::VPOAnalysisUtils::getDirectiveMetadataString(Call);

  return vpo::VPOAnalysisUtils::getDirectiveID(DirStr) == DirectiveID;
}

bool HLInst::isSIMDDirective() const { return isIntelDirective(DIR_OMP_SIMD); }

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
    return true;
  default:
    return false;
  }
}

bool HLInst::isReductionOp(unsigned *OpCode) const {
  const Instruction *LLVMInst = getLLVMInstruction();

  if (isa<BinaryOperator>(LLVMInst)) {
    *OpCode = LLVMInst->getOpcode();
    return isValidReductionOpCode(*OpCode);
  } else if (isa<SelectInst>(LLVMInst)) {
    *OpCode = Instruction::Select;
    return isMinOrMax();
  } else {
    *OpCode = 0;
    return false;
  }
}

bool HLInst::checkMinMax(bool IsMin, bool IsMax) const {

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
          Pred == PredicateTy::FCMP_OGE || Pred == PredicateTy::FCMP_OGT) {
        return true;
      }
    }
    // min pattern: x <(=) y ? x : y    max pattern: x <(=) y ? y : x
    if ((OneAndThree && IsMin) || (!OneAndThree && IsMax)) {
      if (Pred == PredicateTy::ICMP_SLE || Pred == PredicateTy::ICMP_SLT ||
          Pred == PredicateTy::FCMP_OLE || Pred == PredicateTy::FCMP_OLT) {
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

  if (!Operand2->isIntConstant(&ConstVal)) {
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

Constant *HLInst::getRecurrenceIdentity(unsigned RednOpCode, Type *Ty) {
  RecurrenceDescriptor::RecurrenceKind RDKind;

  assert(isValidReductionOpCode(RednOpCode) &&
         "Expected a valid reduction opcode");

  switch (RednOpCode) {
  case Instruction::FAdd:
  case Instruction::FSub:
    RDKind = RecurrenceDescriptor::RK_FloatAdd;
    break;

  case Instruction::Add:
  case Instruction::Sub:
    RDKind = RecurrenceDescriptor::RK_IntegerAdd;
    break;

  case Instruction::FMul:
    RDKind = RecurrenceDescriptor::RK_FloatMult;
    break;

  case Instruction::Mul:
    RDKind = RecurrenceDescriptor::RK_IntegerMult;
    break;

  case Instruction::And:
    RDKind = RecurrenceDescriptor::RK_IntegerAnd;
    break;

  case Instruction::Or:
    RDKind = RecurrenceDescriptor::RK_IntegerOr;
    break;

  case Instruction::Xor:
    RDKind = RecurrenceDescriptor::RK_IntegerXor;
    break;

  case Instruction::Select:
    if (Ty->isIntegerTy()) {
      RDKind = RecurrenceDescriptor::RK_IntegerMinMax;
    } else {
      assert(Ty->isFloatingPointTy() &&
             "Floating point type expected at this point!");
      RDKind = RecurrenceDescriptor::RK_FloatMinMax;
    }
    break;

  default:
    llvm_unreachable("Unexpected reduction opcode");
    break;
  }

  return RecurrenceDescriptor::getRecurrenceIdentity(RDKind, Ty);
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
