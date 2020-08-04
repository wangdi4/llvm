//===- IntelVPLoopAnalysis.cpp --------------------------------------------===//
//
//   Copyright (C) 2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file provides VPLoop-based analyses.
///
//===----------------------------------------------------------------------===//

#include "IntelVPLoopAnalysis.h"
#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanValue.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "vploop-analysis"

using namespace llvm;
using namespace llvm::vpo;

// Flag to enable printing of loop entities.
static cl::opt<bool> DumpVPlanEntities("vplan-entities-dump", cl::init(false),
                                       cl::Hidden,
                                       cl::desc("Print VPlan entities"));

extern cl::opt<bool> EnableVPValueCodegen;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

void VPReduction::dump(raw_ostream &OS) const {
  OS << (isIntegerRecurrenceKind(getRecurrenceKind()) && isSigned() ? " signed "
                                                                    : " ");
  switch (getRecurrenceKind()) {
  default:
    OS << "unknown";
    break;
  case RK_IntegerAdd:
  case RK_FloatAdd:
    OS << "(+)";
    break;
  case RK_IntegerMult:
  case RK_FloatMult:
    OS << "(*)";
    break;
  case RK_IntegerOr:
    OS << "(|)";
    break;
  case RK_IntegerAnd:
    OS << "(&)";
    break;
  case RK_IntegerXor:
    OS << "(^)";
    break;
  case RK_IntegerMinMax:
  case RK_FloatMinMax:
    switch (getMinMaxRecurrenceKind()) {
    default:
      OS << "unknown";
      break;
    case MRK_UIntMin:
      OS << "(UIntMin)";
      break;
    case MRK_UIntMax:
      OS << "(UIntMax)";
      break;
    case MRK_SIntMin:
      OS << "(SIntMin)";
      break;
    case MRK_SIntMax:
      OS << "(SIntMax)";
      break;
    case MRK_FloatMin:
      OS << "(FloatMin)";
      break;
    case MRK_FloatMax:
      OS << "(FloatMax)";
      break;
    }
  }
  if (getRecurrenceStartValue()) {
    OS << " Start: ";
    getRecurrenceStartValue()->printAsOperand(OS);
  }
  if (getLoopExitInstr()) {
    OS << " Exit: ";
    getLoopExitInstr()->printAsOperand(OS);
  }
  printLinkedValues(OS);
}

void VPIndexReduction::dump(raw_ostream &OS) const {
  VPReduction::dump(OS);
  OS << " Parent exit: ";
  ParentRed->getLoopExitInstr()->printAsOperand(OS);
}

void VPInduction::dump(raw_ostream &OS) const {
  switch (getKind()) {
  default:
    OS << " Unknown induction ";
    break;
  case IK_IntInduction:
    OS << " IntInduction";
    break;
  case IK_PtrInduction:
    OS << " PtrInduction";
    break;
  case IK_FpInduction:
    OS << " FpInduction";
    break;
  }
  unsigned int Opc = getInductionOpcode();
  switch (Opc) {
  case Instruction::Add:
  case Instruction::FAdd:
  case Instruction::GetElementPtr:
    OS << "(+)";
    break;
  case Instruction::Sub:
  case Instruction::FSub:
    OS << "(-)";
    break;
  case Instruction::Mul:
  case Instruction::FMul:
    OS << "(*)";
    break;
  }

  if (getStartValue()) {
    OS << " Start: ";
    getStartValue()->printAsOperand(OS);
  }
  if (Step) {
    OS << " Step: ";
    Step->printAsOperand(OS);
  }
  if (getInductionBinOp()) {
    OS << " BinOp: ";
    getInductionBinOp()->print(OS);
  }
  if (NeedCloseForm) {
    OS << " need close form ";
  }
  printLinkedValues(OS);
}

void VPPrivate::dump(raw_ostream &OS) const { printLinkedValues(OS); }

void VPLoopEntityMemoryDescriptor::dump(raw_ostream &OS) const {
  MemoryPtr->print(OS);
}

void VPLoopEntity::printLinkedValues(raw_ostream &OS) const {
  if (LinkedVPValues.size() == 0)
    return;
  OS << "\n  Linked values: ";
  for (auto *V : LinkedVPValues) {
    V->printAsOperand(OS);
    OS << ", ";
  }
  OS << "\n";
}
#endif // NDEBUG || LLVM_ENABLE_DUMP

VPLoopEntity::~VPLoopEntity() {}

static VPValue *getLiveInOrConstOperand(const VPInstruction *Instr,
                                        const VPLoop &Loop) {
  auto Iter = llvm::find_if(Instr->operands(), [&Loop](const VPValue *Operand) {
    return Loop.isDefOutside(Operand) || isa<VPConstant>(Operand);
  });
  return Iter != Instr->op_end() ? *Iter : nullptr;
}

static bool hasLiveInOrConstOperand(const VPInstruction *Instr,
                                    const VPLoop &Loop) {
  return getLiveInOrConstOperand(Instr, Loop) != nullptr;
}

static bool
allUpdatesAreStores(SmallVectorImpl<VPInstruction *> &UpdateVPInsts) {
  assert(UpdateVPInsts.size() > 0 &&
         "No updates for this reduction descriptor.");
  if (llvm::any_of(UpdateVPInsts, [](VPInstruction *I) {
        return I->getOpcode() != Instruction::Store;
      }))
    return false;

  // All update VPInstructions are stores
  return true;
}

// A trivial bitcast VPInstruction is something like below-
// i32 %vp18288 = bitcast i32 %vp8624
static bool isTrivialBitcast(VPInstruction *VPI) {
  if (VPI->getOpcode() != Instruction::BitCast)
    return false;

  Type *SrcTy = VPI->getOperand(0)->getType();
  Type *DestTy = VPI->getType();

  if (SrcTy != DestTy)
    return false;

  return true;
}

unsigned VPReduction::getReductionOpcode(RecurrenceKind K,
                                         MinMaxRecurrenceKind MK) {
  switch (K) {
  case RecurrenceKind::RK_IntegerMinMax:
  case RecurrenceKind::RK_FloatMinMax:
    switch (MK) {
    default:
      llvm_unreachable("Unknown recurrence kind");
      return Instruction::BinaryOpsEnd;
    case MRK_UIntMin:
      return VPInstruction::UMin;
    case MRK_UIntMax:
      return VPInstruction::UMax;
    case MRK_SIntMin:
      return VPInstruction::SMin;
    case MRK_SIntMax:
      return VPInstruction::SMax;
    case MRK_FloatMin:
      return VPInstruction::FMin;
    case MRK_FloatMax:
      return VPInstruction::FMax;
    }
  default:
    return RDTempl::getRecurrenceBinOp(K);
  }
}

unsigned int VPInduction::getInductionOpcode() const {
  // If the opode was set explicitly return it.
  if (IndOpcode != Instruction::BinaryOpsEnd)
    return IndOpcode;

  // Otherwise get opcode from instruction.
  VPInstruction* IndUpdate = getInductionBinOp();
  assert (IndUpdate && "Induction update instruction is not set");
  return IndUpdate->getOpcode();
}

VPInstruction *VPLoopEntityList::getInductionLoopExitInstr(
    const VPInduction *Induction) const {
  auto BinOp = Induction->getInductionBinOp();
  if (BinOp && Loop.isLiveOut(BinOp)) {
    return BinOp;
  }
  for (auto *Val : Induction->getLinkedVPValues())
    if (auto *Instr = dyn_cast<VPInstruction>(Val))
      if (Loop.isLiveOut(Instr))
        return Instr;
  return nullptr;
}

bool VPLoopEntityList::isInductionLastValPreInc(const VPInduction *Ind) const {
  if (Ind->getIsMemOnly())
    return false;
  VPInstruction *Instr = getInductionLoopExitInstr(Ind);
  if (!Instr)
    return false;
  if (Instr == Ind->getInductionBinOp())
    return false;
  if (isa<VPPHINode>(Instr) && Instr->getParent() == Loop.getHeader())
    return true;

  return false;
}

VPPHINode *
VPLoopEntityList::findInductionStartPhi(const VPInduction *Induction) const {
  VPPHINode *StartPhi = nullptr;
  auto BinOp = Induction->getInductionBinOp();
  if (BinOp)
    for (auto User : BinOp->users())
      if (auto Instr = dyn_cast<VPInstruction>(User))
        if (isa<VPPHINode>(Instr) && Loop.contains(Instr->getParent()) &&
            hasLiveInOrConstOperand(Instr, Loop)) {
          StartPhi = cast<VPPHINode>(Instr);
          break;
        }
  return StartPhi;
}

void VPLoopEntityList::replaceDuplicateInductionPHIs() {
  for (auto &PhiPair : DuplicateInductionPHIs) {
    VPPHINode *Duplicate = PhiPair.first;
    VPPHINode *Orig = PhiPair.second;
    LLVM_DEBUG(dbgs() << "VPLoopEntity: Replacing duplicate induction PHI ";
               Duplicate->dump(); dbgs() << " with original PHI ";
               Orig->dump());
    Duplicate->replaceAllUsesWithInLoop(Orig, Loop);
  }

  DuplicateInductionPHIs.clear();
}

VPReduction *VPLoopEntityList::addReduction(
    VPInstruction *Instr, VPValue *Incoming, VPInstruction *Exit,
    RecurrenceKind Kind, FastMathFlags FMF, MinMaxRecurrenceKind MKind,
    Type *RedTy, bool Signed, VPValue *AI, bool ValidMemOnly) {
  VPReduction *Red = new VPReduction(Incoming, Exit, Kind, FMF, MKind, RedTy,
                                     Signed, ValidMemOnly);
  ReductionList.emplace_back(Red);
  linkValue(ReductionMap, Red, Instr);
  linkValue(ReductionMap, Red, Exit);
  createMemDescFor(Red, AI);
  return Red;
}

VPIndexReduction *VPLoopEntityList::addIndexReduction(
    VPInstruction *Instr, const VPReduction *Parent, VPValue *Incoming,
    VPInstruction *Exit, Type *RedTy, bool Signed, bool ForLast, VPValue *AI,
    bool ValidMemOnly) {
  assert(Parent && "null parent in index-reduction");
  VPIndexReduction *Red = new VPIndexReduction(Parent, Incoming, Exit, RedTy,
                                               Signed, ForLast, ValidMemOnly);
  ReductionList.emplace_back(Red);
  linkValue(ReductionMap, Red, Instr);
  linkValue(ReductionMap, Red, Exit);
  MinMaxIndexes[Parent] = Red;
  createMemDescFor(Red, AI);
  return Red;
}
VPInduction *VPLoopEntityList::addInduction(VPInstruction *Start,
                                            VPValue *Incoming,
                                            InductionKind Kind, VPValue *Step,
                                            VPInstruction *InductionOp,
                                            unsigned int Opc, VPValue *AI,
                                            bool ValidMemOnly) {
  //  assert(Start && "null starting instruction");
  VPInduction *Ind =
      new VPInduction(Incoming, Kind, Step, InductionOp, ValidMemOnly, Opc);
  InductionList.emplace_back(Ind);
  linkValue(InductionMap, Ind, Start);
  if (InductionOp) {
    linkValue(InductionMap, Ind, InductionOp);
  }
  createMemDescFor(Ind, AI);
  return Ind;
}

VPPrivate *VPLoopEntityList::addPrivate(VPInstruction *FinalI,
                                        VPEntityAliasesTy &Aliases,
                                        bool IsConditional, bool IsLast,
                                        bool Explicit, VPValue *AI,
                                        bool ValidMemOnly) {
  VPPrivate *Priv = new VPPrivate(FinalI, std::move(Aliases), IsConditional,
                                  IsLast, Explicit, ValidMemOnly);
  PrivatesList.emplace_back(Priv);
  linkValue(PrivateMap, Priv, AI);
  createMemDescFor(Priv, AI);
  return Priv;
}

void VPLoopEntityList::createMemDescFor(VPLoopEntity *E, VPValue *AI) {
  if (!AI)
    return;
  std::unique_ptr<VPLoopEntityMemoryDescriptor> &Ptr = MemoryDescriptors[E];
  if (!Ptr) {
    assert(E && "Expect non-null VPLoopEntity 'E'");
    Ptr.reset(new VPLoopEntityMemoryDescriptor(E, AI));
  } else
    assert(Ptr.get()->getMemoryPtr() == AI &&
           "Secondary memory location for a VPLoopEntity");
  MemInstructions[AI] = Ptr.get();
}

/// Returns identity corresponding to the RecurrenceKind.
VPValue *VPLoopEntityList::getReductionIdentity(const VPReduction *Red) const {
  switch (Red->getRecurrenceKind()) {
  case RecurrenceKind::RK_IntegerXor:
  case RecurrenceKind::RK_IntegerAdd:
  case RecurrenceKind::RK_IntegerOr:
  case RecurrenceKind::RK_IntegerMult:
  case RecurrenceKind::RK_IntegerAnd:
  case RecurrenceKind::RK_FloatMult:
  case RecurrenceKind::RK_FloatAdd: {
    Constant *C = VPReduction::getRecurrenceIdentity(Red->getRecurrenceKind(),
                                                     Red->getRecurrenceType());
    return Plan.getVPConstant(C);
  }
  case RecurrenceKind::RK_IntegerMinMax:
  case RecurrenceKind::RK_FloatMinMax:
    return Red->getRecurrenceStartValue();
  default:
    llvm_unreachable("Unknown recurrence kind");
  }
}

// Basing on the MinMax kind and comparison predicate identify which
// index should be returned as last value, the last or the first one.
// The following rules apply for predicates (b stands for "found value", ai
// stands for array we are looking through).
// If we have MAX the following combinations can occur:
//   b  <  ai ? ai : b   => first index
//   ai <  b  ? b  : ai  => last index
//   b  <= ai ? ai : b   => last
//   ai <= b  ? b  : ai  => first
//   ai >  b  ? ai : b   => first
//   b  >  ai ? b  : ai  => last
//   ai >= b  ? ai : b   => last
//   b  >= ai ? b  : ai  => first
// If we have MIN the following combinations can occur:
//   b  <  ai ? b  : ai  => last  index
//   ai <  b  ? ai : b   => first index
//   b  <= ai ? b  : ai  => first
//   ai <= b  ? ai : b   => last
//   ai >  b  ? b  : ai  => last
//   b  >  ai ? ai : b   => first
//   ai >= b  ? b  : ai  => first
//   b  >= ai ? ai : b   => last
// So, we need to know at wich position in comparison is b,
// comparison predicate and kind of reduction (MIN or MAX).
//
bool VPLoopEntityList::isMinMaxLastItem(const VPReduction &Red) const {
  if (Red.getRecurrenceKind() != RecurrenceKind::RK_IntegerMinMax &&
      Red.getRecurrenceKind() != RecurrenceKind::RK_FloatMinMax)
    return false;

  bool IsMin;
  switch (Red.getMinMaxRecurrenceKind()) {
  case MinMaxRecurrenceKind::MRK_UIntMin:
  case MinMaxRecurrenceKind::MRK_SIntMin:
  case MinMaxRecurrenceKind::MRK_FloatMin:
    IsMin = true;
    break;
  case MinMaxRecurrenceKind::MRK_UIntMax:
  case MinMaxRecurrenceKind::MRK_SIntMax:
  case MinMaxRecurrenceKind::MRK_FloatMax:
    IsMin = false;
    break;
  default:
    llvm_unreachable("Unknown minmax predicate");
  }
  auto &LinkedVals = Red.getLinkedVPValues();
  for (auto *Val : LinkedVals)
    if (auto VPInst = dyn_cast<VPInstruction>(Val))
      if (VPInst->getOpcode() == Instruction::Select) {
        VPPHINode *BestValInst = getRecurrentVPHINode(Red);
        assert(BestValInst && "Phi node not found for min/max reduction");
        // Get condition
        auto PredInst = cast<VPCmpInst>(VPInst->getOperand(0));
        bool BestIsFirstCmpOperand = BestValInst == PredInst->getOperand(0);
        switch (PredInst->getPredicate()) {
        case CmpInst::FCMP_OGE:
        case CmpInst::FCMP_UGE:
        case CmpInst::ICMP_UGE:
        case CmpInst::ICMP_SGE:
          // max b  >= ai ? b  : ai  => first
          // min b  >= ai ? ai : b   => last
          // max ai >= b  ? ai : b   => last
          // min ai >= b  ? b  : ai  => first
          return BestIsFirstCmpOperand ? IsMin : !IsMin;
        case CmpInst::FCMP_OLE:
        case CmpInst::FCMP_ULE:
        case CmpInst::ICMP_ULE:
        case CmpInst::ICMP_SLE:
          // min b  <= ai ? b  : ai  => first
          // max b  <= ai ? ai : b   => last
          // min ai <= b  ? ai : b   => last
          // max ai <= b  ? b  : ai  => first
          return BestIsFirstCmpOperand ? !IsMin : IsMin;
        case CmpInst::FCMP_OGT:
        case CmpInst::FCMP_UGT:
        case CmpInst::ICMP_UGT:
        case CmpInst::ICMP_SGT:
          // max b  >  ai ? b  : ai  => last
          // min b  >  ai ? ai : b   => first
          // max ai >  b  ? ai : b   => first
          // min ai >  b  ? b  : ai  => last
          return BestIsFirstCmpOperand ? !IsMin : IsMin;
        case CmpInst::FCMP_OLT:
        case CmpInst::FCMP_ULT:
        case CmpInst::ICMP_ULT:
        case CmpInst::ICMP_SLT:
          // max b  <  ai ? ai : b   => first
          // min b  <  ai ? b  : ai  => last
          // max ai <  b  ? b  : ai  => last
          // min ai <  b  ? ai : b   => first
          return BestIsFirstCmpOperand ? IsMin : !IsMin;
        default:
          llvm_unreachable("Unknown minmax predicate");
          break;
        }
      }
  return false;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPLoopEntityList::dump(raw_ostream &OS,
                            const VPBasicBlock *LoopHeader) const {
  if (!DumpVPlanEntities)
    return;

  if (DumpVPlanEntities) {
    if (LoopHeader)
      OS << "Loop Entities of the loop with header " << LoopHeader->getName()
         << "\n";
    // TODO: different prints for different debug levels.
    // OS << "\nReductions count: " << ReductionMap.size()
    //   << " Inductions Count: " << InductionMap.size()
    //   << " Privates Count: " << PrivateMap.size() << "\n";
    if (!ReductionList.empty())
      dumpList("\nReduction list\n", ReductionList, OS);
    if (!InductionList.empty())
      dumpList("\nInduction list\n", InductionList, OS);
    if (!PrivatesList.empty())
      dumpList("\nPrivate list\n", PrivatesList, OS);
    OS << "\n";
  }
}
#endif // NDEBUG

void VPLoopEntityList::finalizeImport() {
  for (auto &Red: ReductionList) {
    VPReduction* R = Red.get();
    if (auto StartVal = R->getRecurrenceStartValue())
      linkValue(ReductionMap, R, StartVal);
  }
}

VPValue *VPLoopEntityList::createPrivateMemory(VPLoopEntity &E,
                                               VPBuilder &Builder,
                                               VPValue *&AI) {
  AI = nullptr;
  const VPLoopEntityMemoryDescriptor *MemDescr = getMemoryDescriptor(&E);
  if (!MemDescr)
    return nullptr;
  if (MemDescr->canRegisterize())
    return nullptr;
  AI = MemDescr->getMemoryPtr();
  VPValue *Ret = Builder.createAllocaPrivate(AI);
  linkValue(&E, Ret);
  return Ret;
}

void VPLoopEntityList::processInitValue(VPLoopEntity &E, VPValue *AI,
                                        VPValue *PrivateMem, VPBuilder &Builder,
                                        VPValue &Init, Type *Ty,
                                        VPValue &Start) {
  if (PrivateMem) {
    assert(AI && "Expected non-null original pointer");
    Builder.createNaryOp(Instruction::Store, Ty, {&Init, PrivateMem});
    AI->replaceAllUsesWithInLoop(PrivateMem, Loop);
  }
  // Now replace Start by Init inside the loop. It may be a
  // constant or something else and can be used in instructions not related to
  // this entity calculation. We should replace it only where it's needed.
  if (!E.getIsMemOnly()) {
    auto &LinkedVals = E.getLinkedVPValues();
    for (auto *Val : LinkedVals)
      if (auto *Phi = dyn_cast<VPPHINode>(Val))
        if (Loop.getHeader() == Phi->getParent())
          Phi->replaceUsesOfWith(&Start, &Init);
  }
  linkValue(&E, &Init);
}

// Replace out-of-the-loop uses of the \p From by the calculated last value (\p
// To). Code generation should transform those uses to the final IR.
static void relinkLiveOuts(VPValue *From, VPValue *To, const VPLoop &Loop) {
  for (auto *User : From->users())
    // Don't replace use in \To itself. It can use original value as, e.g.,
    // accumulator of reduction.
    if (User != To && (isa<VPExternalUse>(User) ||
                       !Loop.contains(cast<VPInstruction>(User)->getParent())))
      User->replaceUsesOfWith(From, To);
}

void VPLoopEntityList::processFinalValue(VPLoopEntity &E, VPValue *AI,
                                         VPBuilder &Builder, VPValue &Final,
                                         Type *Ty, VPValue *Exit) {
  if (AI) {
    VPValue *V = Builder.createNaryOp(Instruction::Store, Ty, {&Final, AI});
    linkValue(&E, V);
  }
  if (Exit && !E.getIsMemOnly())
    relinkLiveOuts(Exit, &Final, Loop);
  linkValue(&E, &Final);
}

// Insert VPInstructions related to VPReductions.
void VPLoopEntityList::insertReductionVPInstructions(VPBuilder &Builder,
                                                     VPBasicBlock *Preheader,
                                                     VPBasicBlock *PostExit) {

  assert(Preheader && "Expect valid Preheader to be passed as input argument.");
  assert(PostExit && "Expect valid PostExit to be passed as input argument.");

  DenseMap<const VPReduction *, std::pair<VPReductionFinal *, VPInstruction *>>
      RedFinalMap;

  // Set the insert-guard-point.
  VPBuilder::InsertPointGuard Guard(Builder);

  // Process the list of Reductions.
  for (VPReduction *Reduction : vpreductions()) {
    VPValue *AI = nullptr;
    Builder.setInsertPoint(Preheader);
    VPValue *Identity = getReductionIdentity(Reduction);
    Type *Ty = Reduction->getRecurrenceType();
    VPValue *PrivateMem = createPrivateMemory(*Reduction, Builder, AI);
    if (Reduction->getIsMemOnly())
      if (!isa<VPConstant>(Identity))
        // min/max in-memory reductions. Need to generate a load.
        Identity = Builder.createNaryOp(Instruction::Load, Ty, {AI});

    // We can initialize reduction either with broadcasted identity only or
    // inserting additionally the initial value into 0th element. In the
    // second case we don't need an additional instruction when reducing.
    // Currently, we use broadcast-only for FP data types and min/max
    // reductions. For integers and pointers we use the broadcast-and-insert
    // method.
    StringRef Name;
    if (AI)
      Name = AI->getName();
    else {
      VPPHINode *PhiN = getRecurrentVPHINode(*Reduction);
      Name = PhiN ? PhiN->getName() : "";
    }
    bool StartIncluded = !Ty->isFloatingPointTy();
    VPInstruction *Init =
        StartIncluded && !Reduction->isMinMax()
            ? Builder.createReductionInit(Identity,
                                          Reduction->getRecurrenceStartValue(),
                                          Name + ".red.init")
            : Builder.createReductionInit(Identity, nullptr /* Start */,
                                          Name + ".red.init");
    processInitValue(*Reduction, AI, PrivateMem, Builder, *Init, Ty,
                     *Reduction->getRecurrenceStartValue());

    // Create instruction for last value. If a register reduction does not have
    // a liveout loop exit instruction (store to reduction variable after
    // update), then last value computation should be done by loading from
    // private memory created for the reduction.
    Builder.setInsertPoint(PostExit);
    VPInstruction *Exit = cast<VPInstruction>(
        Reduction->getIsMemOnly() || !Reduction->getLoopExitInstr()
            ? Builder.createNaryOp(Instruction::Load, Ty, {PrivateMem})
            : Reduction->getLoopExitInstr());

    VPReductionFinal *Final = nullptr;
    if (auto IndexRed = dyn_cast<VPIndexReduction>(Reduction)) {
      const VPReduction *Parent = IndexRed->getParentReduction();
      VPInstruction *ParentExit;
      VPReductionFinal *ParentFinal;
      std::tie(ParentFinal, ParentExit) = RedFinalMap[Parent];
      Final = Builder.createReductionFinal(
          Reduction->getReductionOpcode(), Exit, ParentExit, ParentFinal,
          Reduction->isSigned(), Name + ".red.final");
    } else {
      if (StartIncluded || Reduction->isMinMax()) {
        Final = Builder.createReductionFinal(Reduction->getReductionOpcode(),
                                             Exit, Name + ".red.final");
      } else {
        // Create a load for Start value if it's a pointer.
        VPValue *FinalStartValue = Reduction->getRecurrenceStartValue();
        if (FinalStartValue->getType() != Ty) { // Ty is recurrence type
          assert(isa<PointerType>(FinalStartValue->getType()) &&
                 "Expected pointer type here.");
          FinalStartValue =
              Builder.createNaryOp(Instruction::Load, Ty, {FinalStartValue});
        }
        Final = Builder.createReductionFinal(
            Reduction->getReductionOpcode(), Exit, FinalStartValue,
            Reduction->isSigned(), Name + ".red.final");
      }
      RedFinalMap[Reduction] = std::make_pair(Final, Exit);

      // Attach FastMathFlags to reduction-final.
      FastMathFlags FMF = Reduction->getFastMathFlags();
      if (FMF.any())
        Final->setFastMathFlags(FMF);
    }
    processFinalValue(*Reduction, AI, Builder, *Final, Ty, Exit);
  }
}

// Check whether \p Inst is a consistent update of induction, i.e. it
// has correct opcode and contains induction step.
static bool isConsistentInductionUpdate(const VPInstruction *Inst,
                                        unsigned BinOpc, const VPValue *Step) {
  // Check whether update operator is consistent with induction definition,
  // i.e. it has the declared opcode and induction step as operand.
  // For auto-recognized inductions the BinOpc is set to BinaryOpsEnd
  // so we have special check for that.
  // TODO: It's better to have the BinOpc set for auto-recognized inductions
  // as well. This is planned for implementation in the fix for CMPLRLLVM-11590.
  bool IsAutoRecognizedInduction = BinOpc == Instruction::BinaryOpsEnd;
  return Instruction::isBinaryOp(Inst->getOpcode()) &&
         (IsAutoRecognizedInduction || Inst->getOpcode() == BinOpc) &&
         Inst->getOperandIndex(Step) != -1;
}

// Insert VPInstructions related to VPInductions.
void VPLoopEntityList::insertInductionVPInstructions(VPBuilder &Builder,
                                                     VPBasicBlock *Preheader,
                                                     VPBasicBlock *PostExit) {

  assert(Preheader && "Expect valid Preheader to be passed as input argument.");
  assert(PostExit && "Expect valid PostExit to be passed as input argument.");

  // Set the insert-guard-point.
  VPBuilder::InsertPointGuard Guard(Builder);

  // Process the list of Inductions.
  for (VPInduction *Induction : vpinductions()) {
    VPValue *AI = nullptr;
    Builder.setInsertPoint(Preheader);
    VPValue *PrivateMem = createPrivateMemory(*Induction, Builder, AI);
    VPValue *Start = Induction->getStartValue();
    Type *Ty = Start->getType();
    if (Induction->getIsMemOnly())
      Start = Builder.createNaryOp(Instruction::Load, Ty, {AI});

    Instruction::BinaryOps Opc =
        static_cast<Instruction::BinaryOps>(Induction->getInductionOpcode());
    StringRef Name;
    if (AI)
      Name = AI->getName();
    else {
      VPPHINode *PhiN = getRecurrentVPHINode(*Induction);
      Name = PhiN ? PhiN->getName() : "";
    }
    VPInstruction *Init = Builder.createInductionInit(
        Start, Induction->getStep(), Opc, Name + ".ind.init");
    processInitValue(*Induction, AI, PrivateMem, Builder, *Init, Ty, *Start);
    VPInstruction *InitStep = Builder.createInductionInitStep(
        Induction->getStep(), Opc, Name + ".ind.init.step");
    if (!Induction->needCloseForm()) {
      if (auto *Instr = Induction->getInductionBinOp()) {
        assert(isConsistentInductionUpdate(Instr,
                                           Induction->getInductionOpcode(),
                                           Induction->getStep()) &&
               "Inconsistent induction update");
        // This is the only instruction to replace step in induction
        // calculation, no other instruction should be affected. That is
        // important in case the step is used in other instructions linked
        // with induction.
        Instr->replaceUsesOfWith(Induction->getStep(), InitStep);
      }
    } else {
      createInductionCloseForm(Induction, Builder, *Init, *InitStep,
                               *PrivateMem);
    }
    VPInstruction *ExitInstr = getInductionLoopExitInstr(Induction);
    // Create instruction for last value
    Builder.setInsertPoint(PostExit);
    unsigned OpT = static_cast<unsigned>(Opc);
    bool IsExtract = OpT != Instruction::Add && OpT != Instruction::FAdd &&
                     OpT != Instruction::GetElementPtr;
    VPValue *Exit =
        Induction->getIsMemOnly()
            ? Builder.createNaryOp(Instruction::Load, Ty, {PrivateMem})
            : ExitInstr;
    VPInstruction *Final =
        IsExtract && Exit
            ? Builder.createInductionFinal(Exit, Name + ".ind.final")
            : Builder.createInductionFinal(Start, Induction->getStep(), Opc,
                                           Name + ".ind.final");
    // Check if induction's last value is live-out of penultimate loop
    // iteration.
    cast<VPInductionFinal>(Final)->setLastValPreIncrement(
        isInductionLastValPreInc(Induction));
    processFinalValue(*Induction, AI, Builder, *Final, Ty, Exit);
  }
}

// Insert VPInstructions related to VPPrivates.
void VPLoopEntityList::insertPrivateVPInstructions(VPBuilder &Builder,
                                                   VPBasicBlock *Preheader) {

  assert(Preheader && "Expect valid Preheader to be passed as input argument.");

  // Set the insert-guard-point.
  VPBuilder::InsertPointGuard Guard(Builder);

  Builder.setInsertPoint(Preheader, Preheader->begin());

  // Process the list of Privates.
  for (VPPrivate *Private : vpprivates()) {
    VPValue *AI = nullptr;
    VPValue *PrivateMem = createPrivateMemory(*Private, Builder, AI);
    if (PrivateMem)
      LLVM_DEBUG(dbgs() << "Replacing all instances of {" << AI << "} with "
                        << *PrivateMem << "\n");

    // Handle aliases in two passes.
    // Insert the aliases into the Loop preheader in the regular order first.
    for (auto const &ValInstPair : Private->aliases()) {
      auto *VPInst = ValInstPair.second;
      Builder.insert(VPInst);
    }

    // Now do the replacement. We first replace all instances of VPOperand
    // with VPInst within the preheader, where all aliases have been inserted.
    // Then replace all instances of VPOperand with VPInst in the loop.
    for (auto const &ValInstPair : Private->aliases()) {
      auto *VPOperand = ValInstPair.first;
      auto *VPInst = ValInstPair.second;
      VPOperand->replaceAllUsesWithInBlock(VPInst, *Preheader);
      VPOperand->replaceAllUsesWithInLoop(VPInst, Loop);
    }

    if (PrivateMem) {
      // The uses of this allocate-private could also be instruction outside
      // the loop. We have to replace instances which are in the pre-header,
      // along with the ones in the loop.
      AI->replaceAllUsesWithInBlock(PrivateMem, *Preheader);
      AI->replaceAllUsesWithInLoop(PrivateMem, Loop);
    }
    // Add special handling for 'Cond' and 'Last' - privates
  }
  LLVM_DEBUG(
      dbgs()
      << "After replacement of private and aliases within the preheader.\n");
  LLVM_DEBUG(Preheader->dump());
}

// Insert VPInstructions corresponding to the VPLoopEntities like
// VPInductions, VPReductions and VPPrivates.
void VPLoopEntityList::insertVPInstructions(VPBuilder &Builder) {
  // If the loop is multi-exit then the code gen for it is done using
  // underlying IR and we don't need to emit anything here.
  if (!Loop.getUniqueExitBlock())
    return;

  VPBasicBlock *PostExit = Loop.getUniqueExitBlock();
  VPBasicBlock *Preheader = Loop.getLoopPreheader();

  // Insert VPInstructions related to VPReductions.
  insertReductionVPInstructions(Builder, Preheader, PostExit);

  // Insert VPInstructions related to VPInductions.
  insertInductionVPInstructions(Builder, Preheader, PostExit);

  // Insert VPInstructions related to VPPrivates.
  insertPrivateVPInstructions(Builder, Preheader);

}

// Create so called "close-form calculation" for induction. The close-form
// calculation is calculation by the formula v = i MUL step OP v0. In case of
// the loop inductions, the need of the close-form means that we need up-to-date
// induction value at the beginning of each loop iteration. That can be achieved
// in two ways: insert calculation exactly by the formula at the beginning of
// the loop, or insert increment of the induction in the end of the loop (see
// examples below for the "+" induction). In both cases the Init and InitStep
// are generated. The initial loop is
// DO
//   %ind = phi(init, %inc_ind)
//   ... uses of %ind
//   %inc_ind = %ind OP step
//   ... uses of inc_ind
// ENDDO
//
// Possible transformations.
// Init = Initialize(identity, VF, Oper); --- generated always
// InitStep = InitializeStep(Step, VF, OPer); --- generated always
// ...
// case 1 (exact close form)
// DO
//   %temp = InitStep MUL %primary_IV         --- new instruction
//   %Induction = Init OP %temp               --- new instruction
//   ... uses of %Induction (replacing %ind)
//   %inc_ind = %Induction OP step --- step is not replaced in such cases
//   ... uses of %inc_ind
// ENDDO
//
// case 2 (second variant, simplest case)
// DO
//   %ind = phi(Init, %Induction) ; %ind replaced by %Induction
//   ... uses of %ind
//   %inc_ind = %ind OP step       --- step is not replaced in such cases
//   ... uses of %inc_ind
//   %Induction = %ind OP %InitStep           --- new instruction
// ENDDO
//
// Another loop, in memory induction
// DO
//   %ind = load %induction_mem ;
//   ... uses of %ind
//   %inc_ind = %ind OP step ; step is not replaced in such cases
//   ... uses of %inc_ind
//   store %inc_ind, %induction_mem
// ENDDO
// case 2 (second variant, in memory induction, no start-phi)
// DO
//   %Induction_phi= phi(Init,%Induction)        --- new instruction
//   store %Induction_phi, %Induction_priv_mem   --- new instruction
//   %ind = load %Induction_priv_mem ;
//   ... uses of %ind
//   %inc_ind = %ind OP step ; step is not replaced in such cases
//   ... uses of %inc_ind
//   store %inc_ind, %Induction_priv_mem --------- this is redundant
//   %Induction = %Induction_phi OP %InitStep    --- new instruction
// ENDDO

// The second variant looks preferrable.:
//
void VPLoopEntityList::createInductionCloseForm(VPInduction *Induction,
                                                VPBuilder &Builder,
                                                VPValue &Init,
                                                VPValue &InitStep,
                                                VPValue &PrivateMem) {
  VPBuilder::InsertPointGuard Guard(Builder);
  auto Opc = Induction->getInductionOpcode();
  Type *Ty = Induction->getStartValue()->getType();
  if (auto BinOp = Induction->getInductionBinOp()) {
    // Non-memory induction.
    VPPHINode *StartPhi = findInductionStartPhi(Induction);
    assert(StartPhi && "null induction StartPhi");
    VPBasicBlock *Block = BinOp->getParent();
    Builder.setInsertPoint(Block);
    VPValue *NewInd = nullptr;
    // TODO. Replace by VPInstruction::clone
    if ((Opc == Instruction::Add && Ty->isPointerTy()) ||
        Opc == Instruction::GetElementPtr)
      NewInd = Builder.createInBoundsGEP(StartPhi, &InitStep, nullptr);
    else
      NewInd = Builder.createNaryOp(Opc, Ty, {StartPhi, &InitStep});
    auto Ndx = StartPhi->getOperandIndex(BinOp);
    StartPhi->setOperand(Ndx, NewInd);
    VPInstruction *ExitIns = getInductionLoopExitInstr(Induction);
    if (ExitIns == BinOp)
      relinkLiveOuts(ExitIns, BinOp, Loop);
    linkValue(Induction, NewInd);
  } else {
    // In-memory induction.
    // First insert phi and store after existing PHIs in loop header block.
    // See comment before the routine.
    VPBasicBlock *Block = Loop.getHeader();
    Builder.setInsertPointFirstNonPhi(Block);
    VPPHINode *IndPhi = Builder.createPhiInstruction(Ty);
    Builder.createNaryOp(Instruction::Store, Ty, {IndPhi, &PrivateMem});
    // Then insert increment of induction and update phi.
    Block = Loop.getLoopLatch();
    Builder.setInsertPoint(Block);
    VPValue *NewInd;
    if ((Opc == Instruction::Add && Ty->isPointerTy()) ||
        Opc == Instruction::GetElementPtr)
      NewInd = Builder.createInBoundsGEP(IndPhi, &InitStep, nullptr);
    else
      NewInd = Builder.createNaryOp(Opc, Ty, {IndPhi, &InitStep});
    // Step will be initialized in loop preheader always.
    VPBasicBlock *InitParent = Loop.getLoopPreheader();
    IndPhi->addIncoming(&Init, InitParent);
    IndPhi->addIncoming(NewInd, Block);
  }
}

VPPHINode *VPLoopEntityList::getRecurrentVPHINode(const VPLoopEntity &E) const {
  for (auto *VPInst : E.getLinkedVPValues())
    if (auto *VPHi = dyn_cast<VPPHINode>(VPInst))
      if (Loop.getHeader() == VPHi->getParent())
        return VPHi;

  return nullptr;
}

bool VPLoopEntityList::isReductionPhi(const VPPHINode *VPhi) const {
  if (const VPReduction *Reduction = getReduction(VPhi))
    return getRecurrentVPHINode(*Reduction) == VPhi;
  return false;
}

static bool checkInstructionInLoop(const VPValue *V, const VPlan *Plan,
                                   const VPLoop *Loop) {
  // Check for null and VPInstruction here to avoid these checks at caller(s)
  // side
  return V == nullptr || !isa<VPInstruction>(V) ||
         Loop->contains(cast<VPInstruction>(V)->getParent());
}

void ReductionDescr::checkParentVPLoop(const VPlan *Plan,
                                       const VPLoop *Loop) const {
  assert((checkInstructionInLoop(StartPhi, Plan, Loop) &&
          checkInstructionInLoop(Exit, Plan, Loop) &&
          checkInstructionInLoop(LinkPhi, Plan, Loop)) &&
         "Parent loop does not match instruction");
}

bool ReductionDescr::isIncomplete() const {
  return StartPhi == nullptr || Start == nullptr || RT == nullptr ||
         Exit == nullptr;
}

bool ReductionDescr::isDuplicate(const VPlan *Plan, const VPLoop *Loop) const {
  if (VPEntityImportDescr::isDuplicate(Plan, Loop))
    return true;

  // Reduction specific checks for duplication.
  // A single SIMD reduction descriptor can be potentially identified by both
  // SafeReductionAnalysis (auto-recognized) and explicitly specified in clause.
  // This duplication is avoided by checking if a reduction entity is already
  // created for current descriptors StartPhi

  const VPLoopEntityList *LE = Plan->getLoopEntities(Loop);

  if (LE && StartPhi && LE->getReduction(StartPhi))
    return true;

  // All checks failed.
  return false;
}

void ReductionDescr::tryToCompleteByVPlan(const VPlan *Plan,
                                          const VPLoop *Loop) {
  if (!Exit) {
    // Explicit reduction descriptors need further analysis to identify Exit
    // VPInstruction. Auto-recognized reductions don't need this.
    bool AliasAnalysisSuccess = replaceOrigWithAlias();
    if (!AliasAnalysisSuccess)
      return;
    Exit = getLoopExitVPInstr(Loop);
  }
  if (StartPhi == nullptr && Exit != nullptr) {
    SetVector<VPPHINode *> Worklist;
    auto AddPHIUsersToWorklist = [&Worklist](VPInstruction *VPI) -> void {
      for (auto *U : VPI->users()) {
        if (auto *PhiUser = dyn_cast<VPPHINode>(U))
          Worklist.insert(PhiUser);
      }
    };
    AddPHIUsersToWorklist(Exit);

    while (!Worklist.empty()) {
      VPPHINode *CurrPHI = Worklist.pop_back_val();
      // Reduction's StartPhi will be in loop's header block and blends a
      // live-in or const operand.
      if (checkInstructionInLoop(CurrPHI, Plan, Loop) &&
          CurrPHI->getParent() == Loop->getHeader() &&
          hasLiveInOrConstOperand(CurrPHI, *Loop)) {
        StartPhi = CurrPHI;
        break;
      }

      // CurrPHI doesn't match StartPhi requirements, recurse on its PHI users.
      LinkedVPVals.push_back(Exit);
      Exit = CurrPHI;
      AddPHIUsersToWorklist(Exit);
    }

    if (StartPhi) {
      LLVM_DEBUG(dbgs() << "StartPhi: "; StartPhi->dump(); dbgs() << "Exit: ";
                 Exit->dump());
    }
  }
  if (StartPhi == nullptr) {
    // The start PHI could potentially be associated with one of the
    // LinkedVPVals of the reduction descriptor
    // TODO: Need a LIT test for this.
    assert(Start &&
           "Start is not available to check for PHIs via LinkedVPValues.");
    for (auto *LVPV : LinkedVPVals) {
      for (auto *User : LVPV->users()) {
        if (auto Instr = dyn_cast<VPInstruction>(User))
          if (isa<VPPHINode>(Instr) && Loop->contains(Instr->getParent()) &&
              hasLiveInOrConstOperand(Instr, *Loop) &&
              Instr->getNumOperandsFrom(Start) > 0) {
            StartPhi = Instr;
            break;
          }
      }
      if (StartPhi)
        break;
    }
  }
  if (StartPhi == nullptr) {
    // The phi was not found. That means we have an explicit reduction.
    assert(isa<VPExternalDef>(Start) && "Reduction is not properly defined");
    findMemoryUses(Start, Loop);
  } else if (Start == nullptr) {
    Start = getLiveInOrConstOperand(StartPhi, *Loop);
    assert(Start && "Can't identify reduction start value");
  }
  if (!RT) {
    RT = Exit ? Exit->getType()
              : (StartPhi ? StartPhi->getType() : Start->getType());
  }
}

void ReductionDescr::passToVPlan(VPlan *Plan, const VPLoop *Loop) {
  if (!Importing)
    return;

  VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(Loop);
  VPReduction *VPRed = nullptr;

  // FastMathFlags for this reduction maybe attached to Exit or any of the
  // LinkedVPValues.
  FastMathFlags RedFMF;
  if (Exit && Exit->hasFastMathFlags())
    RedFMF = Exit->getFastMathFlags();
  else {
    for (auto *V : LinkedVPVals) {
      if (auto *VPI = dyn_cast<VPInstruction>(V))
        if (VPI->hasFastMathFlags())
          RedFMF = VPI->getFastMathFlags();
    }
  }

  if (LinkPhi == nullptr)
    VPRed = LE->addReduction(StartPhi, Start, Exit, K, RedFMF, MK, RT, Signed,
                             AllocaInst, ValidMemOnly);
  else {
    const VPReduction *Parent = LE->getReduction(LinkPhi);
    assert(Parent && "nullptr is unexpected");
    bool ForLast = LE->isMinMaxLastItem(*Parent);
    VPRed = LE->addIndexReduction(StartPhi, Parent, Start, Exit, RT, Signed,
                                  ForLast, AllocaInst, ValidMemOnly);
  }

  // Add all linked VPValues collected during Phase 2 analysis
  for (auto *V : LinkedVPVals)
    VPRed->addLinkedVPValue(V);

  // Invalidate underlying IR of reduction instructions.
  invalidateReductionInstructions();
}

void ReductionDescr::invalidateReductionInstructions() {
  for (auto *V : LinkedVPVals)
    V->invalidateUnderlyingIR();

  if (Exit)
    Exit->invalidateUnderlyingIR();

  if (StartPhi)
    StartPhi->invalidateUnderlyingIR();
}

bool ReductionDescr::replaceOrigWithAlias() {
  auto PerformAliasReplace = [&]() {
    LLVM_DEBUG(
        dbgs()
        << "Reduction descr: Using alias instead of original descriptor.\n");

    // Overwrite start value of descriptor with that of the alias.
    Start = Alias.getValue().Start;
    // Add updates to linked VPValues before overwriting
    for (auto *U : UpdateVPInsts)
      LinkedVPVals.push_back(U);
    UpdateVPInsts = Alias.getValue().UpdateVPInsts;
  };

  // Cases where alias to descriptor is needed instead of original descriptor
  // 1. Original descriptor has no InitVPValue or UpdateVPInsts
  if (!Start || UpdateVPInsts.empty()) {
    if (!HasAlias) {
      LLVM_DEBUG(dbgs() << "Reduction descriptor is not used within the loop "
                           "and it does not have any valid alias.\n");
      // Set importing to false to ensure this descriptor is not imported into
      // VPlan
      setImporting(false);
      return false; // Not importing, analysis bailed
    }
    PerformAliasReplace();
  }
  // 2. Original descriptor has only stores as UpdateVPInsts
  else if (allUpdatesAreStores(UpdateVPInsts)) {
    if (HasAlias) {
      PerformAliasReplace();
    }
  }

  return true; // Successful analysis
}

VPInstruction *ReductionDescr::getLoopExitVPInstr(const VPLoop *Loop) {
  LLVM_DEBUG(dbgs() << "ReductionDescr: Start: "; Start->dump();
             dbgs() << "\n");
  for (auto &UVP : UpdateVPInsts) {
    LLVM_DEBUG(dbgs() << "ReductionDescr: UVP: "; UVP->dump());
    (void)UVP;
  }

  VPInstruction *LoopExitVPI = nullptr;

  if (UpdateVPInsts.size() == 1) {
    if (UpdateVPInsts[0]->getOpcode() != Instruction::Store)
      LoopExitVPI = UpdateVPInsts[0];
    else {
      // In-memory reduction, no more analysis needed
      return nullptr;
    }
  }

  // Case where descriptor has multiple update instructions
  if (UpdateVPInsts.size() > 1) {
    if (allUpdatesAreStores(UpdateVPInsts))
      return nullptr; // In-memory reduction

    // TODO: Multiple valid updates
    assert(0 && "Multiple valid updates code not fully implemented, unable to "
                "find test case.");
  }

  // Live-out analysis tests for LoopExit
  if (LoopExitVPI) {
    while (!Loop->isLiveOut(LoopExitVPI) &&
           (isTrivialBitcast(LoopExitVPI) || isa<VPHIRCopyInst>(LoopExitVPI))) {
      // Add the trivial copy to linked VPVals for safety
      LinkedVPVals.push_back(LoopExitVPI);
      LoopExitVPI = dyn_cast<VPInstruction>(
          LoopExitVPI->getOperand(0)); // Copy has only one operand
      assert(LoopExitVPI && "Input for copy is not a VPInstruction.");
    }

    // If the final loop exit VPI is still not live-out then store the VPI to
    // linked VPVals and return null, as private memory will be needed to
    // perform this reduction. Example test -
    // Transforms/Intel_VPO/Vecopt/hir_simd_descr_vpentities_priv_memory.ll
    if (!Loop->isLiveOut(LoopExitVPI)) {
      LinkedVPVals.push_back(LoopExitVPI);
      LoopExitVPI = nullptr;
    }
  }

  return LoopExitVPI;
}

void PrivateDescr::passToVPlan(VPlan *Plan, const VPLoop *Loop) {
  VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(Loop);
  LE->addPrivate(FinalInst, PtrAliases, IsConditional, IsLast, IsExplicit,
                 AllocaInst, isMemOnly());
}

void PrivateDescr::checkParentVPLoop(const VPlan *Plan,
                                     const VPLoop *Loop) const {
  // TODO: Add robust check for FinalI and some more checks related to
  // AllocaInst in this function.
}

void PrivateDescr::tryToCompleteByVPlan(const VPlan *Plan, const VPLoop *Loop) {

  for (auto User : AllocaInst->users()) {
    if (auto Inst = dyn_cast<VPInstruction>(User)) {
      // TODO: Need to revisit the logic. This is not quite correct. We have to
      // get the last, i.e., the store writing into the private which
      // post-dominates all other stores in the loop.
      if (Inst->getOpcode() == Instruction::Store && User->hasExternalUse())
        FinalInst = Inst;
    }
  }
}

bool VPEntityImportDescr::isDuplicate(const VPlan *Plan,
                                      const VPLoop *Loop) const {
  const VPLoopEntityList *LE = Plan->getLoopEntities(Loop);
  // It's not first (LE exists) and already have the same alloca instruction
  return LE && AllocaInst && LE->getMemoryDescriptor(AllocaInst);
}

static bool hasRealUserInLoop(VPValue *Val, const VPLoop *Loop) {
  SmallVector<VPValue *, 4> WorkList;
  for (auto *U : Val->users())
    WorkList.push_back(U);
  while (!WorkList.empty()) {
    VPValue *Cur = WorkList.pop_back_val();
    if (isa<VPExternalUse>(Cur))
      continue;
    auto I = cast<VPInstruction>(Cur);
    if (!Loop->contains(I))
      continue;
    if (I->getOpcode() == Instruction::BitCast ||
        I->getOpcode() == Instruction::AddrSpaceCast) {
      for (auto *U : I->users())
        WorkList.push_back(U);
      continue;
    }
    if (auto *VPCall = dyn_cast<VPCallInstruction>(I)) {
      Function *CalleeFunc = VPCall->getCalledFunction();
      // For indirect calls we will have a null CalledFunc.
      if (!CalleeFunc)
        return true;
      if (CalleeFunc->isIntrinsic())
        if (CalleeFunc->getIntrinsicID() == Intrinsic::lifetime_start ||
            CalleeFunc->getIntrinsicID() == Intrinsic::lifetime_end ||
            CalleeFunc->getIntrinsicID() == Intrinsic::invariant_start ||
            CalleeFunc->getIntrinsicID() == Intrinsic::invariant_end)
          continue;
    }
    return true;
  }
  return false;
}

// We need to avoid type inconsistency. That inconsistency
// arises from that omp directive contains pointer to the real variable
// used in the clause. So we have a pointer type for Start but entity type
// is really of a pointed-to type. We replace Start with a load in this case
// and Start should not be used at initialization, use AI in this case to
// generate a load.
VPValue *VPEntityImportDescr::findMemoryUses(VPValue *Start,
                                             const VPLoop *Loop) {
  Importing = hasRealUserInLoop(Start, Loop);
  ValidMemOnly = true;
  if (Importing) {
    // Try to find either load or store. Not having them we can't proceed
    // further. E.g., the code might be something like below. We can't guess
    // which/whether a call is an update and create a correct code.
    //  #omp linear(k:1)
    //  do
    //    foo(&k);
    //    goo(&k);
    //  enddo
    //  use(k);
    //
    // TODO: for inductions, this situation can be handled using close-form
    // re-calculation at the beginning of the loop.
    VPValue *LdStInstr = nullptr;
    for (auto User : Start->users())
      if (auto Instr = dyn_cast<VPInstruction>(User))
        if (Loop->contains(Instr->getParent())) {
          if (Instr->getOpcode() == Instruction::Load)
            LdStInstr = Instr;
          else if (Instr->getOpcode() == Instruction::Store)
            LdStInstr = Instr->getOperand(0);
          if (LdStInstr)
            break;
        }
    if (!LdStInstr) {
      // No able to find load/store. Assert in debug mode and don't
      // import it in product compiler. So far no cases detected.
      assert(false && "Can't handle explicit induction/reduction");
      Importing = false;
    } else
      Start = LdStInstr;
  }
  return Start;
}

void InductionDescr::checkParentVPLoop(const VPlan *Plan,
                                       const VPLoop *Loop) const {
  assert((checkInstructionInLoop(StartPhi, Plan, Loop) &&
          checkInstructionInLoop(Start, Plan, Loop) &&
          checkInstructionInLoop(InductionOp, Plan, Loop)) &&
         "Parent loop does not match instruction");
}

bool InductionDescr::isIncomplete() const {
  return StartPhi == nullptr || InductionOp == nullptr || Step == nullptr;
}

bool InductionDescr::isDuplicate(const VPlan *Plan, const VPLoop *Loop) const {
  if (VPEntityImportDescr::isDuplicate(Plan, Loop))
    return true;

  // Induction specific checks for duplication.
  // 2 cases are possible for induction PHIs because of optimizations of IR
  // before VPlan -
  // Case 1:
  // %phi1 = [%v, PreHeader], [%update, Latch]
  // %phi2 = [%v, PreHeader], [%update, Latch]
  // ...
  // %update = add %phi1, 1
  //
  // In this case the second induction %phi2 should not be imported, and all its
  // uses should be replaced by %phi1 inside the loop (collect in
  // DuplicateInductionPHIs)
  //
  // Case 2:
  // %phi1 = [%v1, PreHeader], [%update, Latch]
  // %phi2 = [%v2, PreHeader], [%phi1, Latch]
  // ...
  // %update = add %phi1, 1
  //
  // Here although %phi2 is an induction, it duplicates values of %phi1
  // partially. %phi2 should not be imported since VPLoopEnties framework cannot
  // handle such cases.

  const VPLoopEntityList *LE = Plan->getLoopEntities(Loop);

  if (LE && InductionOp) {
    if (const VPInduction *OtherInd = LE->getInduction(InductionOp)) {
      if (OtherInd->getStartValue() == Start) {
        // Record that current induction descriptor's PHI node duplicates
        // another induction descriptor (OtherInd)
        VPPHINode *OrigIndPHI = LE->findInductionStartPhi(OtherInd);
        assert(OrigIndPHI && StartPhi &&
               "Could not find PHI node for original induction and current "
               "duplicate.");
        const_cast<VPLoopEntityList *>(LE)->addDuplicateInductionPHIs(
            cast<VPPHINode>(StartPhi) /*Duplicate*/, OrigIndPHI);
      }

      // This is a duplicate induction descriptor, don't continue to import it
      return true;
    }
  }

  // All checks failed
  return false;
}

// If the induction's increment instruction has users which are not one of the
// following, then it has an invalid extra user which would need close-form
// code.
// 1. StartPhi
// 2. Store to the alloca of induction variable
// 3. Loop latch condbit (Bottom-test)
// 4. VPExternalUse
bool InductionDescr::hasUserOfIndIncrement(
    VPInstruction *IncrementVPI, SmallPtrSetImpl<VPInstruction *> &AnalyzedVPIs,
    const VPLoop *Loop) const {
  for (auto *User : IncrementVPI->users()) {
    if (!isa<VPInstruction>(User)) {
      assert(isa<VPExternalUse>(User) &&
             "Non-VPI user of increment instruction is not an external user.");
      continue;
    }

    VPInstruction *UserVPI = cast<VPInstruction>(User);

    // Ignore this user if it is -
    // 1. Non-loop user of increment instruction.
    // 2. Already processed in recursion chain.
    // 3. One of the whitelist instruction listed above.
    if (!Loop->contains(UserVPI->getParent()) || AnalyzedVPIs.count(UserVPI) ||
        UserVPI == StartPhi ||
        (UserVPI->getOpcode() == Instruction::Store &&
         UserVPI->getOperand(1) == AllocaInst) ||
        (isa<VPCmpInst>(UserVPI) && Loop->getLoopLatch() &&
         Loop->getLoopLatch()->getCondBit() == UserVPI))
      continue;

    if (isa<VPPHINode>(UserVPI))
      // Conservatively mark all non-Start PHI users as invalid users.
      // TODO: We need a more comprehensive analysis to find out if a non-Start
      // PHI user of an induction's increment represents an induction. Some
      // preliminary version could be to inspect the incoming values being
      // blended by the PHI node.
      return true;

    // Invalid user.
    return true;
  }

  // All whitelist checks passed.
  return false;
}

bool InductionDescr::inductionNeedsCloseForm(const VPLoop *Loop) const {
  if (UpdateVPInsts.size() > 1)
    // More than one updating instructions for linear, there will be a PHI to
    // blend them. There might be uses between the updates.
    return true;

  VPInstruction *IndIncrementVPI = InductionOp;

  if (IndIncrementVPI) {
    if (!isConsistentInductionUpdate(IndIncrementVPI, getIndOpcode(),
                                        getStep()))
      // The update instruction is inconsistent.
      return true;
  }
  if (!IndIncrementVPI) {
    // TODO: Currently we don't have analyses for memory inductions in
    // VPLoopEntities to determine final updating instruction. Temporarily using
    // close-form representation for them to be conservative. This will change
    // once additional analyses are added to converters and VPLoopEntities.
    return true;
    // In-memory induction scenario.
    assert(IsExplicitInduction && "in-memory induction is auto-recognized?");
    // The single update to descriptor should be a store to the original
    // descriptor.
    assert(UpdateVPInsts[0]->getOpcode() == Instruction::Store &&
           "Single update to in-memory induction is not store.");
    assert(AllocaInst && UpdateVPInsts[0]->getOperand(1) == AllocaInst &&
           "In-memory induction store not writing into alloca.");
    IndIncrementVPI = cast<VPInstruction>(UpdateVPInsts[0]->getOperand(0));
  }

  // Analyse all users of induction's increment instruction.
  // NOTE: If increment is a conversion operation, we recurse the analysis on
  // source instruction.
  SmallPtrSet<VPInstruction *, 8> AnalyzedIncrementVPIs;
  while (true) {
    if (hasUserOfIndIncrement(IndIncrementVPI, AnalyzedIncrementVPIs, Loop))
      // The increment instruction has a valid user, so close-form is needed.
      return true;

    if (!IndIncrementVPI->isCast())
      // We've reached and processed an actual incrementing instruction, stop
      // analysis.
      break;

    // Cast increment instruction, recurse on source.
    AnalyzedIncrementVPIs.insert(IndIncrementVPI);
    IndIncrementVPI = cast<VPInstruction>(IndIncrementVPI->getOperand(0));
  }

  // All checks failed.
  return false;
}

void InductionDescr::passToVPlan(VPlan *Plan, const VPLoop *Loop) {
  if (!Importing)
    return;

  VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(Loop);
  VPInduction *VPInd =
      LE->addInduction(StartPhi, Start, K, Step, InductionOp, IndOpcode,
                       AllocaInst, ValidMemOnly);
  if (inductionNeedsCloseForm(Loop))
    VPInd->setNeedCloseForm(true);
}

void InductionDescr::tryToCompleteByVPlan(const VPlan *Plan,
                                          const VPLoop *Loop) {
  if (StartPhi == nullptr) {
    VPValue *V = InductionOp ? InductionOp : Start;
    for (auto User : V->users())
      if (auto Instr = dyn_cast<VPInstruction>(User))
        if (isa<VPPHINode>(Instr) && Loop->contains(Instr->getParent()) &&
            hasLiveInOrConstOperand(Instr, *Loop)) {
          StartPhi = Instr;
          break;
        }
    if (StartPhi == nullptr) {
      // No phi was found. That can happen only for explicit inductions.
      // Start should represent AllocaIns.
      assert((isa<VPExternalDef>(Start) && InductionOp == nullptr) &&
             "Induction is not properly defined");
      Start = findMemoryUses(Start, Loop);
    }
  }

  if (StartPhi != nullptr)
    if (isa<VPPHINode>(StartPhi)) {
      if (InductionOp == nullptr)
        InductionOp = dyn_cast<VPInstruction>(
            StartPhi->getOperand(0) == Start ? StartPhi->getOperand(1)
                                             : StartPhi->getOperand(0));
      else if (Start == nullptr)
        Start = (StartPhi->getOperand(0) == InductionOp
                     ? StartPhi->getOperand(1)
                     : StartPhi->getOperand(0));
    }

  if (Step == nullptr) {
    // Induction variable with variable step
    assert((StartPhi && InductionOp) &&
           "Variable step occurs only for auto-recognized inductions.");
    int PhiOpIdx = InductionOp->getOperandIndex(StartPhi);
    assert(PhiOpIdx != -1 && "InductionOp does not use starting PHI node.");
    unsigned StepOpIdx = PhiOpIdx == 0 ? 1 : 0;
    Step = InductionOp->getOperand(StepOpIdx);
  }
}
