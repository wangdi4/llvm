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
// \file
/// This file provides VPLoop-based analysis. Right now VPLoopAnalysisBase can
/// only be used to compute min, known, estimated or max trip counts for a
/// VPLoopRegion. In addition Min,max and average tripcounts are set based
/// on the values specified by the user in pragma loop_count
//
//===----------------------------------------------------------------------===//

#include "IntelVPLoopAnalysis.h"
#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "llvm/ADT/GraphTraits.h"

#define DEBUG_TYPE "vploop-analysis"

using namespace llvm;
using namespace llvm::vpo;

// Flag to enable printing of loop entities.
static cl::opt<bool> DumpVPlanEntities("vplan-entities-dump", cl::init(false),
                                       cl::Hidden,
                                       cl::desc("Print VPlan entities"));

// Temporary flag to disable loop entities import until CMPLRLLVM-9026 is
// fixed.
cl::opt<bool>
    LoopEntityImportEnabled("vplan-import-entities", cl::init(true),
                            cl::Hidden,
                            cl::desc("Enable VPloop entities import"));

cl::opt<bool> VPlanUseVPEntityInstructions(
    "vplan-use-entity-instr", cl::init(false), cl::Hidden,
    cl::desc("Generate VPInstructions for VPEntities"));

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
}

void VPPrivate::dump(raw_ostream &OS) const {
}

void VPLoopEntityMemoryDescriptor::dump(raw_ostream &OS) const {
  MemoryPtr->dump(OS);
}
#endif // NDEBUG

VPLoopEntity::~VPLoopEntity() {}

static VPValue *getLiveInOrConstOperand(const VPInstruction *Instr,
                                        const VPLoop &Loop) {
  auto Iter = llvm::find_if(Instr->operands(), [&Loop](const VPValue *Operand) {
    return Loop.isLiveIn(Operand) || isa<VPConstant>(Operand);
  });
  return Iter != Instr->op_end() ? *Iter : nullptr;
}

static bool hasLiveInOrConstOperand(const VPInstruction *Instr,
                                    const VPLoop &Loop) {
  return getLiveInOrConstOperand(Instr, Loop) != nullptr;
}

unsigned VPReduction::getReductionOpcode() const {
  switch (getRecurrenceKind()) {
  case RecurrenceKind::RK_IntegerMinMax:
  case RecurrenceKind::RK_FloatMinMax:
    switch (getMinMaxRecurrenceKind()) {
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
    return RDTempl::getRecurrenceBinOp(getRecurrenceKind());
  }
}

unsigned int VPInduction::getInductionOpcode() const {
  return getInductionBinOp() ? getInductionBinOp()->getOpcode() : BinOpcode;
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

VPPHINode *
VPLoopEntityList::findInductionStartPhi(const VPInduction *Induction) const {
  VPPHINode *StartPhi = nullptr;
  auto BinOp = Induction->getInductionBinOp();
  if (BinOp)
    for (auto User : BinOp->users())
      if (auto Instr = dyn_cast<VPInstruction>(User))
        if (isa<VPPHINode>(Instr) &&
            Loop.contains(cast<VPBlockBase>(Instr->getParent())) &&
            hasLiveInOrConstOperand(Instr, Loop)) {
          StartPhi = cast<VPPHINode>(Instr);
          break;
        }
  return StartPhi;
}

VPReduction *VPLoopEntityList::addReduction(
    VPInstruction *Instr, VPValue *Incoming, VPInstruction *Exit,
    RecurrenceKind Kind, FastMathFlags FMF, MinMaxRecurrenceKind MKind,
    Type *RedTy, bool Signed, VPValue *AI, bool ValidMemOnly) {
  VPReduction *Red = new VPReduction(Incoming, Exit, Kind, FMF, MKind, RedTy,
                                     Signed, ValidMemOnly);
  ReductionList.emplace_back(Red);
  linkValue(ReductionMap, Red, Instr);
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
  MinMaxIndexes[Parent] = Red;
  createMemDescFor(Red, AI);
  return Red;
}
VPInduction *VPLoopEntityList::addInduction(VPInstruction *Start,
                                            VPValue *Incoming,
                                            InductionKind Kind, VPValue *Step,
                                            VPInstruction *InductionBinOp,
                                            unsigned int Opc, VPValue *AI,
                                            bool ValidMemOnly) {
  //  assert(Start && "null starting instruction");
  VPInduction *Ind =
      new VPInduction(Incoming, Kind, Step, InductionBinOp, ValidMemOnly, Opc);
  InductionList.emplace_back(Ind);
  linkValue(InductionMap, Ind, Start);
  if (InductionBinOp) {
    linkValue(InductionMap, Ind, InductionBinOp);
  }
  createMemDescFor(Ind, AI);
  return Ind;
}
VPPrivate *VPLoopEntityList::addPrivate(VPInstruction *Assign,
                                        bool isConditional, bool Explicit,
                                        VPValue *AI, bool ValidMemOnly) {
  assert(Assign && "null assign");
  VPPrivate *Priv =
      new VPPrivate(Assign, isConditional, Explicit, ValidMemOnly);
  PrivateList.emplace_back(Priv);
  linkValue(PrivateMap, Priv, Assign);
  createMemDescFor(Priv, AI);
  return Priv;
}

void VPLoopEntityList::createMemDescFor(VPLoopEntity *E, VPValue *AI) {
  if (!AI)
    return;
  std::unique_ptr<VPLoopEntityMemoryDescriptor> &Ptr = MemoryDescriptors[E];
  if (!Ptr)
    Ptr.reset(new VPLoopEntityMemoryDescriptor(AI));
  else
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
//TODO - not implemented yet
bool VPLoopEntityList::isMinMaxInclusive(const VPReduction &Red) {
  return false;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPLoopEntityList::dump(raw_ostream &OS,
                          const VPBlockBase *LoopHeader) const {
  if (!DumpVPlanEntities)
    return;
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
  if (!PrivateList.empty())
    dumpList("\nPrivate list\n", PrivateList, OS);
  OS << "\n";
}
#endif //NDEBUG

void VPLoopEntityList::finalizeImport() {
  for (auto &Red: ReductionList) {
    VPReduction* R = Red.get();
    if (auto StartVal = R->getRecurrenceStartValue())
      linkValue(ReductionMap, R, StartVal);
    if (auto ExInstr = R->getLoopExitInstr())
      linkValue(ReductionMap, R, ExInstr);
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
  bool MakeSOA = MemDescr->isSafeSOA() && MemDescr->isProfitableSOA();
  return Builder.createAllocaPrivate(AI->getType(), MakeSOA);
}

void VPLoopEntityList::processInitValue(VPLoopEntity &E, VPValue *AI,
                                        VPValue *PrivateMem, VPBuilder &Builder,
                                        VPValue &Init, Type *Ty,
                                        VPValue &Start) {
  if (PrivateMem) {
    Builder.createNaryOp(Instruction::Store, Ty, {&Init, PrivateMem});
    AI->replaceAllUsesWithInLoop(PrivateMem, Loop);
  }
  // Now replace Start by Init inside the loop. It may be a
  // constant or something else and can be used in instructions not related to
  // this entity calculation. We should replace it only where it's needed.
  if (!E.getIsMemOnly()) {
    const SmallVectorImpl<VPValue *> &LinkedVals = E.getLinkedVPValues();
    for (auto *Val : LinkedVals)
      if (auto *Instr = dyn_cast<VPInstruction>(Val))
        Instr->replaceUsesOfWith(&Start, &Init);
  }
}

void VPLoopEntityList::processFinalValue(VPLoopEntity &E, VPValue *AI,
                                       VPBuilder &Builder, VPValue &Final,
                                       Type *Ty, VPValue *Exit) {
  if (AI)
    Builder.createNaryOp(Instruction::Store, Ty, {&Final, AI});
  if (!E.getIsMemOnly())
    // Replace external uses of the Exit by the calculated last value. Code
    // generation should transform those uses to the final IR
    for (auto *User : Exit->users())
      if (User != &Final && Loop.isLiveOut(User))
        User->replaceUsesOfWith(Exit, &Final);
}

void VPLoopEntityList::insertVPInstructions(VPBuilder &Builder) {
  if (!VPlanUseVPEntityInstructions)
    return;
  VPBuilder::InsertPointGuard Guard(Builder);
  VPBasicBlock *Preheader = dyn_cast<VPBasicBlock>(Loop.getLoopPreheader());
  VPBasicBlock *PostExit = dyn_cast<VPBasicBlock>(Loop.getUniqueExitBlock());
  assert((Preheader && PostExit) && "Loop preheader and exit expected");

  for (auto &RedPtr : ReductionList) {
    VPReduction *Reduction = RedPtr.get();
    Builder.setInsertPoint(Preheader);
    VPValue *Identity = getReductionIdentity(Reduction);
    Type *Ty = Reduction->getRecurrenceType();
    VPValue *AI = nullptr;
    VPValue *PrivateMem = createPrivateMemory(*Reduction, Builder, AI);
    if (Reduction->getIsMemOnly())
      if (!isa<VPConstant>(Identity))
        // min/max in-memory reductions. Need to generate a load.
        Identity = Builder.createNaryOp(Instruction::Load, Ty, {AI});

    VPInstruction *Init = Builder.createReductionInit(Identity);
    processInitValue(*Reduction, AI, PrivateMem, Builder, *Init, Ty,
                     *Reduction->getRecurrenceStartValue());

    // Create instruction for last value
    Builder.setInsertPoint(PostExit);
    VPValue *Exit =
        Reduction->getIsMemOnly()
            ? Builder.createNaryOp(Instruction::Load, Ty, {PrivateMem})
            : Reduction->getLoopExitInstr();

    VPInstruction *Final = Builder.createReductionFinal(
        Reduction->getReductionOpcode(), Reduction->getRecurrenceStartValue(),
        Exit, Reduction->isSigned());
    processFinalValue(*Reduction, AI, Builder, *Final, Ty, Exit);
  }
  for (auto &IndPtr : InductionList) {
    VPInduction *Induction = IndPtr.get();
    Builder.setInsertPoint(Preheader);
    VPValue *AI = nullptr;
    VPValue *PrivateMem = createPrivateMemory(*Induction, Builder, AI);
    VPValue *Start = Induction->getStartValue();
    Type *Ty = Start->getType();
    if (Induction->getIsMemOnly())
      Start = Builder.createNaryOp(Instruction::Load, Ty, {AI});

    Instruction::BinaryOps Opc =
        static_cast<Instruction::BinaryOps>(Induction->getInductionOpcode());
    VPInstruction *Init =
        Builder.createInductionInit(Start, Induction->getStep(), Opc);
    processInitValue(*Induction, AI, PrivateMem, Builder, *Init, Ty,
                     *Start);
    VPInstruction *InitStep =
        Builder.createInductionInitStep(Induction->getStep(), Opc);
    if (!Induction->needCloseForm()) {
      const SmallVectorImpl<VPValue *> &LinkedVals =
          Induction->getLinkedVPValues();
      for (auto *Val : LinkedVals)
        if (auto *Instr = dyn_cast<VPInstruction>(Val))
          Instr->replaceUsesOfWith(Induction->getStep(), InitStep);
    } else {
      createInductionCloseForm(Induction, Builder, *InitStep, *PrivateMem);
    }
    VPInstruction *ExitInstr = getInductionLoopExitInstr(Induction);
    if (Induction->getIsMemOnly() || ExitInstr) {
      // Create instruction for last value
      Builder.setInsertPoint(PostExit);
      VPValue *Exit =
          Induction->getIsMemOnly()
              ? Builder.createNaryOp(Instruction::Load, Ty, {PrivateMem})
              : ExitInstr;
      VPInstruction *Final = Builder.createInductionFinal(Exit, Start);
      processFinalValue(*Induction, AI, Builder, *Final, Ty, Exit);
    }
  }
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
void VPLoopEntityList::createInductionCloseForm(
    VPInduction *Induction, VPBuilder &Builder, VPValue &InitStep,
    VPValue &PrivateMem) {
  VPBuilder::InsertPointGuard Guard(Builder);
  auto Opc = Induction->getInductionOpcode();
  Type *Ty = Induction->getStartValue()->getType();
  if (Induction->getInductionBinOp()) {
    // Non-memory induction.
    VPPHINode *StartPhi = findInductionStartPhi(Induction);
    VPBasicBlock *Block = Induction->getInductionBinOp()->getParent();
    Builder.setInsertPoint(Block);
    VPValue *NewInd = nullptr;
    // TODO. Replace by VPInstruction::clone
    if ((Opc == Instruction::Add && Ty->isPointerTy()) ||
        Opc == Instruction::GetElementPtr)
      NewInd = Builder.createInBoundsGEP(StartPhi, &InitStep, nullptr);
    else
      NewInd = Builder.createNaryOp(Opc, Ty, {StartPhi, &InitStep});
    auto Ndx = StartPhi->getOperandIndex(Induction->getInductionBinOp());
    StartPhi->setOperand(Ndx, NewInd);
  } else {
    // First insert phi and store at the beginning of the block.
    VPBasicBlock *Block = cast<VPBasicBlock>(Loop.getHeader());
    Builder.setInsertPoint(Block, Block->begin());
    VPPHINode *IndPhi = cast<VPPHINode>(Builder.createPhiInstruction(Ty));
    Builder.insert(IndPhi);
    Builder.createNaryOp(Instruction::Store, Ty, {IndPhi, &PrivateMem});
    // Then insert increment of induction and update phi.
    Block = cast<VPBasicBlock>(Loop.getLoopLatch());
    Builder.setInsertPoint(Block);
    VPValue *NewInd;
    if ((Opc == Instruction::Add && Ty->isPointerTy()) ||
        Opc == Instruction::GetElementPtr)
      NewInd = Builder.createInBoundsGEP(IndPhi, &InitStep, nullptr);
    else
      NewInd = Builder.createNaryOp(Opc, Ty, {IndPhi, &InitStep});
    VPBasicBlock *InitParent =
        cast<VPBasicBlock>(IndPhi->getParent()->getSinglePredecessor());
    IndPhi->addIncoming(&InitStep, InitParent);
    IndPhi->addIncoming(NewInd, Block);
  }
}

static bool checkInstructionInLoop(const VPValue *V, const VPlan *Plan,
                                   const VPLoop *Loop) {
  // Check for null and VPInstruction here to avoid these checks at caller(s)
  // side
  return V == nullptr || !isa<VPInstruction>(V) ||
         Loop->contains(cast<VPBlockBase>(cast<VPInstruction>(V)->getParent()));
}

void ReductionDescr::checkParentVPLoop(const VPlan *Plan,
                                       const VPLoop *Loop) const {
  assert((checkInstructionInLoop(StartPhi, Plan, Loop) &&
          checkInstructionInLoop(Exit, Plan, Loop) &&
          checkInstructionInLoop(LinkPhi, Plan, Loop)) &&
         "Parent loop does not match instruction");
}

bool ReductionDescr::isIncomplete() const {
  return StartPhi == nullptr || Start == nullptr;
}

void ReductionDescr::tryToCompleteByVPlan(const VPlan *Plan,
                                          const VPLoop *Loop) {
  if (StartPhi == nullptr && Exit != nullptr)
    for (auto User : Exit->users())
      if (auto Instr = dyn_cast<VPInstruction>(User))
        if (isa<VPPHINode>(Instr) &&
            checkInstructionInLoop(Instr, Plan, Loop) &&
            hasLiveInOrConstOperand(Instr, *Loop)) {
          StartPhi = Instr;
          break;
        }
  if (StartPhi == nullptr) {
    // The phi was not found. That means we have an explicit reduction.
    assert(isa<VPExternalDef>(Start) && "Reduction is not properly defined");
    Start = findMemoryUses(Start, Loop);
  } else if (Start == nullptr) {
    Start = getLiveInOrConstOperand(StartPhi, *Loop);
    assert(Start && "Can't identify reduction start value");
  }
}

void ReductionDescr::passToVPlan(VPlan *Plan, const VPLoop *Loop) {
  VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(Loop);
  if (LinkPhi == nullptr)
    LE->addReduction(StartPhi, Start, Exit, K, FastMathFlags::getFast(), MK, RT,
                     Signed, AllocaInst, ValidMemOnly);
  else {
    const VPReduction *Parent = LE->getReduction(LinkPhi);
    bool ForLast = LE->isMinMaxInclusive(*Parent);
    LE->addIndexReduction(StartPhi, Parent, Start, Exit, RT, Signed, ForLast,
                          AllocaInst, ValidMemOnly);
  }
}

bool VPEntityImportDescr::isDuplicate(const VPlan *Plan,
                                      const VPLoop *Loop) const {
  const VPLoopEntityList *LE = Plan->getLoopEntities(Loop);
  // It's not first (LE exists) and already have the same alloca instruction
  return LE && AllocaInst && LE->getMemoryDescriptor(AllocaInst);
}

// We need to avoid type inconsistency. That inconsistency
// arises from that omp directive contains pointer to the real variable
// used in the clause. So we have a pointer type for Start but entity type
// is really of a pointed-to type. We replace Start with a load in this case
// and Start should not be used at initialization, use AI in this case to
// generate a load.
VPValue *VPEntityImportDescr::findMemoryUses(VPValue *Start,
                                             const VPLoop *Loop) {
  Importing = Start->getNumUsers() != 0;
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
        if (Loop->contains(cast<VPBlockBase>(Instr->getParent()))) {
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
          checkInstructionInLoop(InductionBinOp, Plan, Loop)) &&
         "Parent loop does not match instruction");
}

bool InductionDescr::isIncomplete() const {
  return StartPhi == nullptr || InductionBinOp == nullptr || Step == nullptr;
}

void InductionDescr::passToVPlan(VPlan *Plan, const VPLoop *Loop) {
  if (Importing)
    Plan->getOrCreateLoopEntities(Loop)->addInduction(StartPhi, Start, K, Step,
                                                      InductionBinOp, BinOpcode,
                                                      AllocaInst, ValidMemOnly);
}

void InductionDescr::tryToCompleteByVPlan(const VPlan *Plan,
                                          const VPLoop *Loop) {
  if (StartPhi == nullptr) {
    VPValue *V = InductionBinOp ? InductionBinOp : Start;
    for (auto User : V->users())
      if (auto Instr = dyn_cast<VPInstruction>(User))
        if (isa<VPPHINode>(Instr) &&
            Loop->contains(cast<VPBlockBase>(Instr->getParent())) &&
            hasLiveInOrConstOperand(Instr, *Loop)) {
          StartPhi = Instr;
          break;
        }
    if (StartPhi == nullptr) {
      // No phi was found. That can happen only for explicit inductions.
      // Start should represent AllocaIns.
      assert((isa<VPExternalDef>(Start) && InductionBinOp == nullptr) &&
             "Induction is not properly defined");
      Start = findMemoryUses(Start, Loop);
    }
  }

  if (StartPhi != nullptr)
    if (isa<VPPHINode>(StartPhi)) {
      if (InductionBinOp == nullptr)
        InductionBinOp = dyn_cast<VPInstruction>(
            StartPhi->getOperand(0) == Start ? StartPhi->getOperand(1)
                                             : StartPhi->getOperand(0));
      else if (Start == nullptr)
        Start = (StartPhi->getOperand(0) == InductionBinOp
                     ? StartPhi->getOperand(1)
                     : StartPhi->getOperand(0));
    }

  if (Step == nullptr) {
    // Induction variable with variable step
    assert((StartPhi && InductionBinOp) &&
           "Variable step occurs only for auto-recognized inductions.");
    int PhiOpIdx = InductionBinOp->getOperandIndex(StartPhi);
    assert(PhiOpIdx != -1 && "InductionBinOp does not use starting PHI node.");
    unsigned StepOpIdx = PhiOpIdx == 0 ? 1 : 0;
    Step = InductionBinOp->getOperand(StepOpIdx);
  }
}

namespace llvm {
namespace vpo {

void VPLoopAnalysisBase::setTripCountsFromPragma(const VPLoopRegion *Lp,
                                                 uint64_t MinTripCount,
                                                 uint64_t MaxTripCount,
                                                 uint64_t AvgTripCount) {
  bool IsMaxTakenFromPragma = false;
  bool IsMinTakenFromPragma = false;
  bool IsAverageTakenFromPragma = false;

  if (MaxTripCount) {
    setMaxTripCountFor(Lp, MaxTripCount);
    IsMaxTakenFromPragma = true;
  } else
    setMaxTripCountFor(Lp, DefaultTripCount);

  if (MinTripCount) {
    setMinTripCountFor(Lp, MinTripCount);
    IsMinTakenFromPragma = true;
  } else
    setMinTripCountFor(Lp, 0);

  if (AvgTripCount) {
    setEstimatedTripCountFor(Lp, AvgTripCount);
    IsAverageTakenFromPragma = true;
  } else if (IsMaxTakenFromPragma && IsMinTakenFromPragma)
    setEstimatedTripCountFor(Lp, (MaxTripCount + MinTripCount) >> 1);
  else if (IsMaxTakenFromPragma)
    setEstimatedTripCountFor(Lp, MaxTripCount);
  else if (IsMinTakenFromPragma)
    setEstimatedTripCountFor(Lp, MinTripCount);
  else
    setEstimatedTripCountFor(Lp, DefaultTripCount);

  (void)IsMaxTakenFromPragma;
  (void)IsMinTakenFromPragma;
  (void)IsAverageTakenFromPragma;

  LLVM_DEBUG(
      dbgs()
      << "Max trip count is " << getMaxTripCountFor(Lp)
      << (IsMaxTakenFromPragma
              ? " updated by loop opt upon retrieving loop count from pragma"
              : " assumed default trip count by vectorizer")
      << '\n');
  LLVM_DEBUG(dbgs() << "Average trip count is " << getTripCountFor(Lp)
                    << (IsAverageTakenFromPragma
                            ? " set by pragma loop count"
                            : " assumed default trip count by vectorizer")
                    << '\n');
  LLVM_DEBUG(dbgs() << "Min trip count is " << getMinTripCountFor(Lp)
                    << (IsMinTakenFromPragma
                            ? " set by pragma loop count"
                            : " assumed default trip count by vectorizer")
                    << '\n');
}

// Metadata is attached to the loop latch. Loop through the VPBasicBlocks to
// find the underlying original basic block and get the LoopID.
// Set the max, min and average trip counts from the metadata.
// Fix Me: This fails when there is an outerloop and there are different
// pragma values for inner and outer loop.
void VPLoopAnalysis::computeTripCountImpl(const VPLoopRegion *Lp) {

  StringRef MaxInfo = "llvm.loop.intel.loopcount_maximum";
  StringRef MinInfo = "llvm.loop.intel.loopcount_minimum";
  StringRef AvgInfo = "llvm.loop.intel.loopcount_average";

  uint64_t MinTripCount = 0, MaxTripCount = 0, AvgTripCount = 0;

  LoopTripCounts[Lp] = TripCountInfo();

  for (const VPBlockBase *VPB :
       make_range(df_iterator<const VPBlockBase *>::begin(Lp->getEntry()),
                  df_iterator<const VPBlockBase *>::end(Lp->getExit()))) {
    const VPBasicBlock *CurrentVPBB = cast<const VPBasicBlock>(VPB);

    if (BasicBlock *OriginalBB = CurrentVPBB->getOriginalBB()) {
      const Loop *Loop = LI->getLoopFor(OriginalBB);
      if (Loop) {
        MDNode *LoopID = Loop->getLoopID();
        if (LoopID) {
          for (unsigned i = 1, ie = LoopID->getNumOperands(); i < ie; ++i) {
            MDNode *MD = dyn_cast<MDNode>(LoopID->getOperand(i));
            if (MD) {
              const MDString *S = dyn_cast<MDString>(MD->getOperand(0));
              if (!S)
                continue;
              if (MaxInfo.equals(S->getString()))
                MaxTripCount = mdconst::extract<ConstantInt>(MD->getOperand(1))
                                   ->getZExtValue();
              if (MinInfo.equals(S->getString()))
                MinTripCount = mdconst::extract<ConstantInt>(MD->getOperand(1))
                                   ->getZExtValue();
              if (AvgInfo.equals(S->getString()))
                AvgTripCount = mdconst::extract<ConstantInt>(MD->getOperand(1))
                                   ->getZExtValue();
            }
          }
        }
      }
      break;
    }
  }
  setTripCountsFromPragma(Lp, MinTripCount, MaxTripCount, AvgTripCount);
}
} // namespace vpo
} // namespace llvm
