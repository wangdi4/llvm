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
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanValue.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"

#define DEBUG_TYPE "vploop-analysis"

using namespace llvm;
using namespace llvm::vpo;

// Flag to enable printing of loop entities.
static cl::opt<bool> DumpVPlanEntities("vplan-entities-dump", cl::init(false),
                                       cl::Hidden,
                                       cl::desc("Print VPlan entities"));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

void VPReduction::dump(raw_ostream &OS) const {
  OS << (isIntegerRecurrenceKind(getRecurrenceKind()) && isSigned() ? " signed "
                                                                    : " ");
  switch (getRecurrenceKind()) {
  default:
    OS << "unknown";
    break;
  case RecurKind::Add:
  case RecurKind::FAdd:
    OS << "(+)";
    break;
  case RecurKind::Mul:
  case RecurKind::FMul:
    OS << "(*)";
    break;
  case RecurKind::Or:
    OS << "(|)";
    break;
  case RecurKind::And:
    OS << "(&)";
    break;
  case RecurKind::Xor:
    OS << "(^)";
    break;
  case RecurKind::UMin:
    OS << "(UIntMin)";
    break;
  case RecurKind::UMax:
    OS << "(UIntMax)";
    break;
  case RecurKind::SMin:
    OS << "(SIntMin)";
    break;
  case RecurKind::SMax:
    OS << "(SIntMax)";
    break;
  case RecurKind::FMin:
    OS << "(FloatMin)";
    break;
  case RecurKind::FMax:
    OS << "(FloatMax)";
    break;
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
  OS << "IsLinearIndex: " << isLinearIndex() << " Parent exit: ";
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
  OS << " StartVal: ";
  if (StartVal)
    StartVal->printAsOperand(OS);
  else
    OS << "?";
  OS << " EndVal: ";
  if (EndVal)
    EndVal->printAsOperand(OS);
  else
    OS << "?";
  if (getInductionBinOp()) {
    OS << " BinOp: ";
    getInductionBinOp()->print(OS);
  }
  if (NeedCloseForm) {
    OS << " need close form ";
  }
  printLinkedValues(OS);
}

void VPPrivate::dump(raw_ostream &OS) const {
  if (hasExitInstr()) {
    OS << "\n  Exit instr: ";
    getExitInst()->print(OS);
  }
  if (hasPrivateTag()) {
    OS << "\n  Private tag: ";
    switch (getPrivateTag()) {
    case PrivateTag::PTRegisterized:
      OS << "Registerized";
      break;
    case PrivateTag::PTArray:
      OS << "Array";
      break;
    case PrivateTag::PTInMemory:
      OS << "InMemory";
      break;
    case PrivateTag::PTNonPod:
      OS << "Non-POD";
      break;
    }
  }
  printLinkedValues(OS);
}

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

VPPrivate::~VPPrivate() {
  for (auto &AliasMapIt : aliases())
    // Drop references to alias instruction if it is not attached to VPlan CFG.
    if (!AliasMapIt.second->getParent())
      AliasMapIt.second->dropAllReferences();
}

static VPValue *getLiveInOrConstOperand(const VPInstruction *Instr,
                                        const VPLoop &Loop) {
  auto Iter = llvm::find_if(Instr->operands(), [&Loop](const VPValue *Operand) {
    return Loop.isDefOutside(Operand) || isa<VPConstant>(Operand);
  });
  return Iter != Instr->op_end() ? *Iter : nullptr;
}

static bool
allUpdatesAreStores(SmallVectorImpl<VPInstruction *> &UpdateVPInsts) {
  assert(UpdateVPInsts.size() > 0 &&
         "No updates for this reduction descriptor.");
  return all_of(UpdateVPInsts, [](VPInstruction *I) {
    return I->getOpcode() == Instruction::Store;
  });
}

// A trivial bitcast VPInstruction is something like below-
// i32 %vp18288 = bitcast i32 %vp8624
static bool isTrivialBitcast(VPInstruction *VPI) {
  if (VPI->getOpcode() != Instruction::BitCast)
    return false;

  Type *SrcTy = VPI->getOperand(0)->getType();
  Type *DestTy = VPI->getType();

  return SrcTy == DestTy;
}

unsigned VPReduction::getReductionOpcode(RecurKind K) {
  switch (K) {
  case RecurKind::UMin:
    return VPInstruction::UMin;
  case RecurKind::UMax:
    return VPInstruction::UMax;
  case RecurKind::SMin:
    return VPInstruction::SMin;
  case RecurKind::SMax:
    return VPInstruction::SMax;
  case RecurKind::FMin:
    return VPInstruction::FMin;
  case RecurKind::FMax:
    return VPInstruction::FMax;
  default:
    return RDTempl::getOpcode(K);
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
  if (BinOp && Loop.isLiveOut(BinOp))
    return BinOp;

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
  auto BinOp = Induction->getInductionBinOp();
  if (!BinOp)
    return nullptr;

  for (auto User : BinOp->users())
    if (auto *Phi = dyn_cast<VPPHINode>(User))
      if (Loop.contains(Phi) && getLiveInOrConstOperand(Phi, Loop))
        return Phi;

  return nullptr;
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

VPReduction *VPLoopEntityList::addReduction(VPInstruction *Instr,
                                            VPValue *Incoming,
                                            VPInstruction *Exit, RecurKind Kind,
                                            FastMathFlags FMF, Type *RedTy,
                                            bool Signed, VPValue *AI,
                                            bool ValidMemOnly) {
  VPReduction *Red =
      new VPReduction(Incoming, Exit, Kind, FMF, RedTy, Signed, ValidMemOnly);
  ReductionList.emplace_back(Red);
  linkValue(ReductionMap, Red, Instr);
  linkValue(ReductionMap, Red, Exit);
  createMemDescFor(Red, AI);
  return Red;
}

VPIndexReduction *VPLoopEntityList::addIndexReduction(
    VPInstruction *Instr, const VPReduction *Parent, VPValue *Incoming,
    VPInstruction *Exit, Type *RedTy, bool Signed, bool ForLast,
    bool IsLinIndex, VPValue *AI, bool ValidMemOnly) {

  assert(Parent && "null parent in index-reduction");
  VPIndexReduction *Red = new VPIndexReduction(
      Parent, Incoming, Exit, RedTy, Signed, ForLast, IsLinIndex, ValidMemOnly);
  ReductionList.emplace_back(Red);
  linkValue(ReductionMap, Red, Instr);
  linkValue(ReductionMap, Red, Exit);
  createMemDescFor(Red, AI);

  // Remember the first index for parent
  if (IsLinIndex && !getMinMaxIndex(Parent))
    MinMaxIndexes[Parent] = Red;

  return Red;
}
VPInduction *VPLoopEntityList::addInduction(
    VPInstruction *Start, VPValue *Incoming, InductionKind Kind, VPValue *Step,
    VPValue *StartVal, VPValue *EndVal, VPInstruction *InductionOp,
    unsigned int Opc, VPValue *AI, bool ValidMemOnly) {
  //  assert(Start && "null starting instruction");
  VPInduction *Ind =
      new VPInduction(Incoming, Kind, Step, StartVal, EndVal, InductionOp,
                      ValidMemOnly, Opc);
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
                                        VPPrivate::PrivateKind K, bool Explicit,
                                        Type *AllocatedTy, VPValue *AI,
                                        bool ValidMemOnly) {
  auto *Priv = new VPPrivate(FinalI, std::move(Aliases), K, Explicit,
                             AllocatedTy, ValidMemOnly);
  PrivatesList.emplace_back(Priv);
  linkValue(PrivateMap, Priv, FinalI);
  linkValue(PrivateMap, Priv, AI);
  createMemDescFor(Priv, AI);
  return Priv;
}

VPPrivate *VPLoopEntityList::addPrivate(VPPrivate::PrivateTag Tag,
                                        VPEntityAliasesTy &Aliases,
                                        VPPrivate::PrivateKind K, bool Explicit,
                                        Type *AllocatedTy, VPValue *AI,
                                        bool ValidMemOnly) {
  auto *Priv = new VPPrivate(Tag, std::move(Aliases), K, Explicit, AllocatedTy,
                             ValidMemOnly);
  PrivatesList.emplace_back(Priv);
  linkValue(PrivateMap, Priv, AI);
  createMemDescFor(Priv, AI);
  return Priv;
}

VPPrivateNonPOD *VPLoopEntityList::addNonPODPrivate(
    VPEntityAliasesTy &Aliases, VPPrivate::PrivateKind K, bool Explicit,
    Function *Ctor, Function *Dtor, Function *CopyAssign, Type *AllocatedTy,
    VPValue *AI) {
  VPPrivateNonPOD *Priv = new VPPrivateNonPOD(
    std::move(Aliases), K, Explicit, Ctor, Dtor, AllocatedTy, CopyAssign);
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
  case RecurKind::Xor:
  case RecurKind::Add:
  case RecurKind::Or:
  case RecurKind::Mul:
  case RecurKind::And:
  case RecurKind::FMul:
  case RecurKind::FAdd: {
    Constant *C = VPReduction::getConstRecurrenceIdentity(Red->getRecurrenceKind(),
                                                     Red->getRecurrenceType(),
                                                     Red->getFastMathFlags());
    return Plan.getVPConstant(C);
  }
  case RecurKind::SMin:
  case RecurKind::SMax:
  case RecurKind::UMin:
  case RecurKind::UMax:
  case RecurKind::FMin:
  case RecurKind::FMax:
  case RecurKind::SelectICmp:
  case RecurKind::SelectFCmp:
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
  if (!Red.isMinMax())
    return false;

  bool IsMin;
  switch (Red.getRecurrenceKind()) {
  case RecurKind::SMin:
  case RecurKind::UMin:
  case RecurKind::FMin:
    IsMin = true;
    break;
  case RecurKind::SMax:
  case RecurKind::UMax:
  case RecurKind::FMax:
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
  if (!MemDescr || MemDescr->canRegisterize())
    return nullptr;
  AI = MemDescr->getMemoryPtr();
  assert((isa<VPExternalDef>(AI) || isa<VPConstant>(AI)) &&
         "Original AI for private is not external.");

  // Capture alignment of original alloca/global from incoming LLVM-IR.
  Align OrigAlignment(1);
  if (auto *OrigAI = dyn_cast_or_null<AllocaInst>(AI->getUnderlyingValue()))
    OrigAlignment = OrigAI->getAlign();
  if (auto *OrigGlobal =
          dyn_cast_or_null<GlobalVariable>(AI->getUnderlyingValue()))
    OrigAlignment = OrigGlobal->getAlign().valueOrOne();

  if (OrigAlignment == 1) {
    // Set default alignment.
    Type *ElemTy = E.getAllocatedType();
    OrigAlignment = Plan.getDataLayout()->getPrefTypeAlign(ElemTy);
  }

  auto *Ret = Builder.create<VPAllocatePrivate>(
    AI->getName(), AI->getType(), E.getAllocatedType(), OrigAlignment);
  // We do not set debug location on allocates.
  Ret->setDebugLocation({});
  linkValue(&E, Ret);
  return Ret;
}

void VPLoopEntityList::processInitValue(VPLoopEntity &E, VPValue *AI,
                                        VPValue *PrivateMem, VPBuilder &Builder,
                                        VPValue &Init, Type *Ty,
                                        VPValue &Start) {
  if (PrivateMem) {
    assert(AI && "Expected non-null original pointer");
    Builder.createStore(&Init, PrivateMem);
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
    if (User != To &&
        (isa<VPExternalUse>(User) || !Loop.contains(cast<VPInstruction>(User))))
      User->replaceUsesOfWith(From, To);
}

void VPLoopEntityList::processFinalValue(VPLoopEntity &E, VPValue *AI,
                                         VPBuilder &Builder, VPValue &Final,
                                         Type *Ty, VPValue *Exit) {
  if (AI) {
    VPLoadStoreInst *V = Builder.createStore(&Final, AI);
#if INTEL_CUSTOMIZATION
    if (auto *ExtDef = dyn_cast<VPExternalDef>(AI)) {
      if (ExtDef->getOperandHIR()) {
        unsigned ExtDefSym =
            cast<VPBlob>(ExtDef->getOperandHIR())->getBlob()->getSymbase();
        V->HIR().setSymbase(ExtDefSym);
      }
    }
#endif // INTEL_CUSTOMIZATION
    linkValue(&E, V);
  }
  if (Exit && !E.getIsMemOnly())
    relinkLiveOuts(Exit, &Final, Loop);
  linkValue(&E, &Final);
}

void VPLoopEntityList::insertOneReductionVPInstructions(
    VPReduction *Reduction, VPBuilder &Builder, VPBasicBlock *PostExit,
    VPBasicBlock *Preheader,
    DenseMap<const VPReduction *,
             std::pair<VPReductionFinal *, VPInstruction *>> &RedFinalMap,
    SmallPtrSetImpl<const VPReduction *> &ProcessedReductions) {

  // Note: the insert location guard also guards builder debug location.
  VPBuilder::InsertPointGuard Guard(Builder);

  VPValue *AI = nullptr;
  Builder.setInsertPoint(Preheader);
  Builder.setCurrentDebugLocation(
      Preheader->getTerminator()->getDebugLocation());

  VPValue *Identity = getReductionIdentity(Reduction);
  Type *Ty = Reduction->getRecurrenceType();
  VPValue *PrivateMem = createPrivateMemory(*Reduction, Builder, AI);
  if (Reduction->getIsMemOnly() && !isa<VPConstant>(Identity))
    // min/max in-memory reductions. Need to generate a load.
    Identity = Builder.createLoad(Ty, AI);

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
  bool StartIncluded = !Ty->isFloatingPointTy() && !Reduction->isMinMax();
  VPValue *StartValue =
      StartIncluded ? Reduction->getRecurrenceStartValue() : nullptr;
  bool UseStart = StartValue != nullptr || Reduction->isMinMax();

  // We need to emit an extra load for in-memory reductions with start value for
  // initialization.
  if (Reduction->getIsMemOnly() && StartValue &&
      StartValue->getType() != Ty) { // Ty is recurrence type
    assert(isa<PointerType>(StartValue->getType()) &&
           "Expected pointer type here.");
    StartValue = Builder.createLoad(Ty, StartValue);
  }

  VPInstruction *Init =
      Builder.createReductionInit(Identity, StartValue, UseStart,
                                  Name + Reduction->getNameSuffix() + ".init");

  processInitValue(*Reduction, AI, PrivateMem, Builder, *Init, Ty,
                   *Reduction->getRecurrenceStartValue());

  // Create instruction for last value. If a register reduction does not have
  // a liveout loop exit instruction (store to reduction variable after
  // update), then last value computation should be done by loading from
  // private memory created for the reduction.
  Builder.setInsertPoint(PostExit);

  // For reduction the liveout instruction basically refers to the reduction op
  // that is in the loop. Reduction-final represents horizontal
  // (i.e. across lanes) reduction of the same op in such case. So we attach
  // same debug location to it. If it is load from private memory then
  // we use loop exit block debug location (i.e. its terminator instruction).
  VPInstruction *Exit = Reduction->getLoopExitInstr();
  if (Exit)
    Builder.setCurrentDebugLocation(Exit->getDebugLocation());
  else
    Builder.setCurrentDebugLocation(
        PostExit->getTerminator()->getDebugLocation());

  if (Reduction->getIsMemOnly() || !Exit)
    Exit = Builder.createLoad(Ty, PrivateMem);

  VPReductionFinal *Final = nullptr;
  std::string FinName = (Name + Reduction->getNameSuffix() + ".final").str();
  if (auto IndexRed = dyn_cast<VPIndexReduction>(Reduction)) {
    const VPReduction *Parent = IndexRed->getParentReduction();
    VPInstruction *ParentExit;
    VPReductionFinal *ParentFinal;
    std::tie(ParentFinal, ParentExit) = RedFinalMap[Parent];
    Final = Builder.create<VPReductionFinal>(
        FinName, Reduction->getReductionOpcode(), Exit, ParentExit, ParentFinal,
        Reduction->isSigned());
    if (IndexRed->isLinearIndex())
      Final->setIsLinearIndex();
  } else {
    if (StartIncluded || Reduction->isMinMax()) {
      Final = Builder.create<VPReductionFinal>(
          FinName, Reduction->getReductionOpcode(), Exit);
    } else {
      // Create a load for Start value if it's a pointer.
      VPValue *FinalStartValue = Reduction->getRecurrenceStartValue();
      if (FinalStartValue->getType() != Ty) { // Ty is recurrence type
        assert(isa<PointerType>(FinalStartValue->getType()) &&
               "Expected pointer type here.");
        FinalStartValue = Builder.createLoad(Ty, FinalStartValue);
      }
      Final = Builder.create<VPReductionFinal>(
          FinName, Reduction->getReductionOpcode(), Exit, FinalStartValue,
          Reduction->isSigned());
    }
  }
  // Attach FastMathFlags to reduction-final.
  FastMathFlags FMF = Reduction->getFastMathFlags();
  if (FMF.any())
    Final->setFastMathFlags(FMF);

  processFinalValue(*Reduction, AI, Builder, *Final, Ty, Exit);

  RedFinalMap[Reduction] = std::make_pair(Final, Exit);
  ProcessedReductions.insert(Reduction);
}

// Insert VPInstructions related to VPReductions.
void VPLoopEntityList::insertReductionVPInstructions(VPBuilder &Builder,
                                                     VPBasicBlock *Preheader,
                                                     VPBasicBlock *PostExit) {

  assert(Preheader && "Expect valid Preheader to be passed as input argument.");
  assert(PostExit && "Expect valid PostExit to be passed as input argument.");

  DenseMap<const VPReduction *, std::pair<VPReductionFinal *, VPInstruction *>>
      RedFinalMap;
  SmallPtrSet<const VPReduction *, 4> ProcessedReductions;

  // Set the insert-guard-point.
  VPBuilder::InsertPointGuard Guard(Builder);

  // Process the list of Reductions.
  for (VPReduction *Reduction : vpreductions()) {
    // For index part of min/max+index idioms, it's essential that
    // parent reductions are processed before. As we don't care about
    // reductions order during import we should make additional checks here
    // and process parent reductions first.
    SmallVector<VPReduction *, 3> WorkList;
    while (!is_contained(ProcessedReductions, Reduction)) {
      WorkList.push_back(Reduction);
      auto *IndexRed = dyn_cast<VPIndexReduction>(Reduction);
      if (!IndexRed)
        break;

      Reduction = const_cast<VPReduction *>(IndexRed->getParentReduction());
    }

    for (auto *Reduction : reverse(WorkList))
      insertOneReductionVPInstructions(Reduction, Builder, PostExit, Preheader,
                                       RedFinalMap, ProcessedReductions);
  }
}

void VPLoopEntityList::preprocess() {
  identifyMinMaxLinearIdxs();
}

void VPLoopEntityList::identifyMinMaxLinearIdxs() {
  // Create the absent linear indexes.
  VPDominatorTree DomTree;
  DomTree.recalculate(Plan);

  // Collect reductions to process in advance.
  // Note that we cannot process directly iterating over vpreductions
  // because the range is being invalidated as new entries added.
  SmallVector<VPIndexReduction *> NonLinearIndexReds;
  for (VPReduction *Red : vpreductions()) {
    auto *IndexRed = dyn_cast<VPIndexReduction>(Red);
    if (IndexRed && !IndexRed->isLinearIndex())
      NonLinearIndexReds.push_back(IndexRed);
  }

  for (VPIndexReduction *Reduction : NonLinearIndexReds) {
    auto *Parent = Reduction->getParentReduction();
    auto Index = getMinMaxIndex(Parent);
    if (!Index)
      Index = createLinearIndexReduction(Reduction, DomTree);

    assert(Index && "linear index is not set for reduction");
    assert(Index != Reduction && "unexpected linear index");

    Reduction->replaceParentReduction(Index);
  }
}

// Create linear index for min/max + index idiom.
// The new code is created to handle min/max+index idioms that look like below.
//    for(i = 0....) {
//       mm = (mm > a[i]) ? mm : a[i];
//       v2 = (mm > a[i]) ? v2 : b[i];
//    }
// Here, we can't calculate v2 last value directly using mm last value. We need
// to know on which iteration the assignment occurs, select the maximum index
// from all lanes, and get the corresponding value. We don't have anything to
// calculate the maximum index as v2 value is not linear. Thus we create fake
// linear index, effectively creating the following code.
//    for(i = 0....) {
//       mm = (mm > a[i]) ? mm : a[i];
//       fake_i = (mm > a[i]) ? fake_i : i;
//       v2 = (mm > a[i]) ? v2 : b[i];
//    }
// After that, fake_i is used to calculate maximum index which, in turn, allows
// us to get the final value for v2.
//
// 1) Emit the following set of VPInstructions:
//    ; in the loop header
//    %phi_val = phi [-1, preheader], [%select, loop_latch]
//    ; in the loop body, just after NonLinNdx update instruction.
//    ; this corresponds to fake_i assignment in the source above.
//    %select = select NonLinNdx.cond, %loop_induction, %phi_val
// 2) Create VPIndexReduction descriptor for these instructions, setting
//    parent reduction to the parent of NonLinNdx.
// 3) Set "IsLinearIndex" flag on the new reduction and update parent's index
//    reduction.
VPIndexReduction *
VPLoopEntityList::createLinearIndexReduction(VPIndexReduction *NonLinNdx,
                                             VPDominatorTree &DomTree) {
  assert(!NonLinNdx->isLinearIndex() && "Expected non-linear index reduction");
  assert(!getMinMaxIndex(NonLinNdx->getParentReduction()) &&
         "Linear index already exists");

  VPBasicBlock *Header = Loop.getHeader();
  const VPInduction *LoopIndex = getLoopInduction();

  // First create the needed VPInstructions.
  VPPHINode *LoopIVPhi = getRecurrentVPHINode(*LoopIndex);

  assert(LoopIVPhi && "Expected getRecurrentVPHINode() to return a non-null "
                      "value in this context.");

  // Create init value. This should be the corresponding constant, either max or
  // min integer value of the corresponding type. Using paropt utility to obtain
  // it.
  unsigned Opc = NonLinNdx->getReductionOpcode();
  bool NeedMaxIntVal =
      (Opc == VPInstruction::FMin || Opc == VPInstruction::SMin ||
       Opc == VPInstruction::UMin);

  Constant *MinMaxInt = VPOParoptUtils::getMinMaxIntVal(
      LoopIVPhi->getType(), !NonLinNdx->isSigned(), NeedMaxIntVal);

  VPBuilder Builder;
  Builder.setInsertPointFirstNonPhi(Header);
  VPConstant *IncomingVal = Plan.getVPConstant(MinMaxInt);
  VPPHINode *StartPhi = Builder.createPhiInstruction(LoopIVPhi->getType());
  StartPhi->addIncoming(IncomingVal, Loop.getLoopPreheader());
  VPInstruction *OrigExit = NonLinNdx->getLoopExitInstr();
  // TODO. Currently we support only select instructions for min/max+index.
  // Need to enhance that and then the code below will not work.
  assert(OrigExit->getOpcode() == Instruction::Select &&
         "Expected select instruction");

  VPInstruction *OrigPhi = getRecurrentVPHINode(*NonLinNdx);
  bool IsPhiFirst = OrigExit->getOperand(1) == OrigPhi;
  Builder.setInsertPoint(OrigExit);
  VPInstruction *NewExit = Builder.createSelect(
      OrigExit->getOperand(0), IsPhiFirst ? StartPhi : LoopIVPhi,
      IsPhiFirst ? LoopIVPhi : StartPhi);

  // TODO. Check whether the current block dominates latch. If not
  // then we need a phi in the latch.
  assert(DomTree.dominates(NewExit->getParent(), Loop.getLoopLatch()) &&
         "Unsupported non-dominating update");
  StartPhi->addIncoming(NewExit, Loop.getLoopLatch());
  const auto *LatchCmp = cast<VPCmpInst>(Loop.getLoopLatch()->getCondBit());
  assert(LatchCmp->getOpcode() == Instruction::ICmp &&
         "Expected ICmp in latch cond");
  // Next, create LoopEntity.
  VPIndexReduction *Ret = addIndexReduction(
      StartPhi, NonLinNdx->getParentReduction(), IncomingVal, NewExit,
      LoopIVPhi->getType(), ICmpInst::isSigned(LatchCmp->getPredicate()),
      NonLinNdx->isForLast(), true /*IsLinNdx*/);
  MinMaxIndexes[NonLinNdx->getParentReduction()] = Ret;
  return Ret;
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

  // Note: the insert location guard also guards builder debug location.
  VPBuilder::InsertPointGuard Guard(Builder);

  // Process the list of Inductions.
  for (VPInduction *Induction : vpinductions()) {
    VPValue *AI = nullptr;
    Builder.setInsertPoint(Preheader);
    Builder.setCurrentDebugLocation(
        Preheader->getTerminator()->getDebugLocation());
    VPValue *PrivateMem = createPrivateMemory(*Induction, Builder, AI);
    VPValue *Start = Induction->getStartValue();
    Type *Ty = Start->getType();
    if (Induction->getIsMemOnly())
      Start = Builder.createLoad(Ty, AI);

    Instruction::BinaryOps Opc =
        static_cast<Instruction::BinaryOps>(Induction->getInductionOpcode());
    StringRef Name;
    if (AI)
      Name = AI->getName();
    else if (VPPHINode *PhiN = getRecurrentVPHINode(*Induction))
      Name = PhiN->getName();

    VPInstruction *Init = Builder.create<VPInductionInit>(
        Name + ".ind.init", Start, Induction->getStep(),
        Induction->getStartVal(), Induction->getEndVal(), Opc);
    processInitValue(*Induction, AI, PrivateMem, Builder, *Init, Ty, *Start);
    VPInstruction *InitStep = Builder.create<VPInductionInitStep>(
        Name + ".ind.init.step", Induction->getStep(), Opc);

    if (Induction->needCloseForm()) {
      createInductionCloseForm(Induction, Builder, *Init, *InitStep,
                               *PrivateMem);
    } else {
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
    }

    VPInstruction *ExitInstr = getInductionLoopExitInstr(Induction);
    // Create instruction for last value
    Builder.setInsertPoint(PostExit);
    if (ExitInstr)
      Builder.setCurrentDebugLocation(ExitInstr->getDebugLocation());
    else
      Builder.setCurrentDebugLocation(
          PostExit->getTerminator()->getDebugLocation());

    unsigned OpT = static_cast<unsigned>(Opc);
    bool IsExtract = OpT != Instruction::Add && OpT != Instruction::FAdd &&
                     OpT != Instruction::GetElementPtr;
    VPValue *Exit = Induction->getIsMemOnly()
                        ? Builder.createLoad(Ty, PrivateMem)
                        : ExitInstr;
    auto *Final =
        IsExtract && Exit
            ? Builder.create<VPInductionFinal>(Name + ".ind.final", Exit)
            : Builder.create<VPInductionFinal>(Name + ".ind.final", Start,
                                               Induction->getStep(), Opc);
    // Check if induction's last value is live-out of penultimate loop
    // iteration.
    Final->setLastValPreIncrement(isInductionLastValPreInc(Induction));
    processFinalValue(*Induction, AI, Builder, *Final, Ty, Exit);
  }
}

// Helper method to create ctor/dtor VPCalls for given non-POD private memory.
static void createNonPODPrivateCtorDtorCalls(Function *F, VPValue *NonPODMemory,
                                             VPBuilder &Builder,
                                             VPlanVector &Plan) {
  assert(F->arg_size() == 1 &&
         "Expected ctor/dtor functions to accept single argument.");
  assert(NonPODMemory &&
         "Expected private memory allocated for non-POD private.");
  auto *VPFunc = Plan.getVPConstant(cast<Constant>(F));

  auto *VPCall =
      new VPCallInstruction(VPFunc, F->getFunctionType(), {NonPODMemory});
  Builder.insert(VPCall);
}

const VPInduction *
VPLoopEntityList::getLoopInduction() const {
  // There are two ways to find main loop induction:
  // 1) Get latch -> condbit -> follow operand chain which is involved in
  //    induction. This relies on that we always have canonical loops with
  //    bottom test and will give exactly loop IV.
  // 2) Get header, scan phis until the induction is found. That is more
  //    reliable but will give the first IV which is not necessary the main
  //    loop IV.
  // The method implements the first way.

  // Look into the def-use chain starting from latch condition via a worklist
  // approach.
  const auto *LatchCond = cast<VPCmpInst>(Loop.getLatchComparison());
  SmallVector<const VPInstruction *, 4> Worklist;
  SmallPtrSet<const VPInstruction *, 4> Visited;

  // Initiate the worklist with latch condition.
  Worklist.push_back(LatchCond);

  while (!Worklist.empty()) {
    const VPInstruction *CurrI = Worklist.pop_back_val();
    if (const VPInduction *Ret = getInduction(CurrI))
      return Ret;

    // Avoid cyclic-phi chains.
    if (!Visited.insert(CurrI).second)
      continue;

    for (auto *Op : CurrI->operands()) {
      if (auto *OpInst = dyn_cast<VPInstruction>(Op))
        Worklist.push_back(OpInst);
    }
  }

  llvm_unreachable("Expected non-null induction for main loop IV.");
}

// See comment in the header file.
void VPLoopEntityList::insertConditionalLastPrivateInst(
    VPPrivate &Private, VPBuilder &Builder, VPBasicBlock *Preheader,
    VPBasicBlock *PostExit, VPValue *PrivateMem, VPValue *AI) {
  assert(Private.isConditional() && "Expected conditional private");

  const VPInduction *LoopIndex = getLoopInduction();
  VPPHINode *InductionHeaderPhi = getRecurrentVPHINode(*LoopIndex);
  assert(InductionHeaderPhi && "Value should not be nullptr!");
  VPConstant *IncomingVal =
      Plan.getVPConstant(ConstantInt::get(InductionHeaderPhi->getType(), -1));

  VPBuilder::InsertPointGuard Guard(Builder);
  if (Private.hasExitInstr()) {

    // We have registerized private.
    VPPHINode *HeaderPhi = getRecurrentVPHINode(Private);
    assert(HeaderPhi && "Expected non-null header phi");

    // First insert phi in the loop header, with one operand, starting -1.
    Builder.setInsertPoint(InductionHeaderPhi);
    VPPHINode *IndexStartPhi = Builder.createPhiInstruction(
        InductionHeaderPhi->getType(), "priv.idx.hdr");
    IndexStartPhi->addIncoming(IncomingVal, Loop.getLoopPreheader());

    DenseMap<VPValue *, VPInstruction *> CreatedVPInsts;
    std::function<VPInstruction *(VPValue *)> getOrCreateIndexInst =
        [&](VPValue *VPVal) -> VPInstruction * {

      if (VPVal == HeaderPhi)
        return IndexStartPhi;
      if (CreatedVPInsts.count(VPVal))
        return CreatedVPInsts[VPVal];

      if (auto *VPPhi = dyn_cast<VPPHINode>(VPVal)) {

        Builder.setInsertPoint(VPPhi);
        VPPHINode *VPPhiIndex =
            Builder.createPhiInstruction(InductionHeaderPhi->getType());
        VPPhiIndex->setName("priv.idx." + VPPhi->getParent()->getName());
        CreatedVPInsts[VPVal] = VPPhiIndex;

        for (unsigned I = 0; I < VPPhi->getNumIncomingValues(); I++) {

          VPValue *Val = VPPhi->getIncomingValue(I);
          VPBasicBlock *BB = VPPhi->getIncomingBlock(I);
          VPPhiIndex->addIncoming(getOrCreateIndexInst(Val), BB);
        }

        return VPPhiIndex;
      }

      if (auto VPInst = dyn_cast<VPInstruction>(VPVal))
        if (VPInst->getOpcode() == Instruction::Select) {

          Builder.setInsertPoint(VPInst);
          VPInstruction *VPInstIndex = Builder.createSelect(
              VPInst->getOperand(0), InductionHeaderPhi, InductionHeaderPhi);
          VPInstIndex->setName("priv.idx." + VPInst->getParent()->getName());
          CreatedVPInsts[VPInst] = VPInstIndex;

          for (int I = 1; I <= 2; I++)
            VPInstIndex->setOperand(
                I, getOrCreateIndexInst(VPInst->getOperand(I)));

          return VPInstIndex;
        }

      return InductionHeaderPhi;
    };

    // Add latch phi as operand to the header phi.
    VPInstruction *PrivExitInst = Private.getExitInst();
    VPInstruction *LastPhi = getOrCreateIndexInst(PrivExitInst);
    VPBasicBlock *Latch = Loop.getLoopLatch();
    assert(Plan.getDT()->dominates(LastPhi->getParent(), Latch) &&
           "Exit PHI's parent block should dominate loop latch.");
    IndexStartPhi->addIncoming(LastPhi, Latch);

    // Create last value calculation instruction.
    Builder.setInsertPoint(PostExit);
    VPValue *Exit = Private.getIsMemOnly()
                        ? cast<VPValue>(Builder.createLoad(
                              Private.getAllocatedType(), PrivateMem, nullptr,
                              "loaded.priv"))
                        : cast<VPValue>(PrivExitInst);
    StringRef ExitName = PrivExitInst ? PrivExitInst->getName() : "";
    Twine Name = ExitName + ".priv.final";
    VPValue *Orig = HeaderPhi->getIncomingValue(Loop.getLoopPreheader());
    VPInstruction *Final;
    if (Private.getIsMemOnly())
      Final = Builder.create<VPPrivateFinalCondMem>(Name, Exit, LastPhi, Orig);
    else
      Final = Builder.create<VPPrivateFinalCond>(Name, Exit, LastPhi, Orig);

    VPValue* StoreMem = Private.getIsMemOnly() ? AI : nullptr;
    processFinalValue(Private, StoreMem, Builder, *Final, Exit->getType(), Exit);
    return;
  }

  // For in-memory privates, create a variable for index and insert stores
  // of the induciton variable at each store to private memory.
  assert(Private.getPrivateTag() == VPPrivate::PrivateTag::PTInMemory &&
         "Expected non null private mem");
  Type *ElemTy = InductionHeaderPhi->getType();
  Builder.setInsertPoint(Preheader);
  // In preheader assign initial value.
  // For inductions, the allocated type is same as ElemTy.
  VPValue *IdxMem = Builder.create<VPAllocatePrivate>(
      "priv.idx.mem", PointerType::get(ElemTy, 0), ElemTy,
      Plan.getDataLayout()->getPrefTypeAlign(ElemTy));
  Builder.createStore(IncomingVal, IdxMem);

  // Go through all stores and create stores to index variable.
  for (auto U : PrivateMem->users()) {
    auto *StoreI = dyn_cast<VPLoadStoreInst>(U);
    if (!StoreI || StoreI->getOpcode() == Instruction::Load)
      continue;
    Builder.setInsertPoint(StoreI);
    Builder.createStore(InductionHeaderPhi, IdxMem);
  }

  Builder.setInsertPoint(PostExit);
  VPValue *Exit =
      Builder.createLoad(Private.getAllocatedType(), PrivateMem, nullptr,
                         "loaded.priv");
  auto IdxLoad = Builder.createLoad(ElemTy, IdxMem, nullptr, "loaded.priv.idx");
  VPInstruction *Final =
      Builder.create<VPPrivateFinalCondMem>(".priv.final", Exit, IdxLoad, AI);
  processFinalValue(Private, AI, Builder, *Final, Exit->getType(), Exit);
}

// Insert VPInstructions related to VPPrivates.
void VPLoopEntityList::insertPrivateVPInstructions(VPBuilder &Builder,
                                                   VPBasicBlock *Preheader,
                                                   VPBasicBlock *PostExit) {

  assert(Preheader && "Expect valid Preheader to be passed as input argument.");
  assert(PostExit && "Expect valid PostExit to be passed as input argument.");

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

    // For non-POD privates we create explicit VPCalls to constructor and
    // destructor functions using the new memory created for the private as
    // operand.
    if (auto *PrivateNonPOD = dyn_cast<VPPrivateNonPOD>(Private)) {
      if (auto *CtorFn = PrivateNonPOD->getCtor())
        createNonPODPrivateCtorDtorCalls(CtorFn, PrivateMem, Builder, Plan);

      if (auto *DtorFn = PrivateNonPOD->getDtor()) {
        // Destructor calls should be emitted in PostExit BB, set insert point
        // of Builder accordingly.
        VPBuilder::InsertPointGuard Guard(Builder);
        Builder.setInsertPoint(PostExit, PostExit->begin());
        createNonPODPrivateCtorDtorCalls(DtorFn, PrivateMem, Builder, Plan);
      }

      if (PrivateNonPOD->isLast()) {
        assert(PrivateNonPOD->getCopyAssign() &&
               "CopyAssign cannot be nullptr");
        VPBuilder::InsertPointGuard Guard(Builder);
        Builder.setInsertPoint(PostExit, PostExit->begin());
        // Below instruction creates call for CopyAssign function. It requires pointer
        // to CopyAssign function to be passed, hence new Instruction type had
        // to be used. Example IR generrated with -vplan-print-after-vpentity-instrs:
        // private-last-value-nonpod CopyAssign:
        // _ZTS3str.omp.copy_assign %struct.str* %vp.alloca.priv %struct.str* %x.lpriv
        // TODO: Consider using regular VPCall instead of adding new
        // Instruction. Currently this approach has been skippped, because for
        // this to work, we'd need to use DoNotWiden scenario, for which value from
        // first lane is used by default. Such approach is not correct for last value
        // calculation for non-POD as there one of the operand requires values
        // from first lane while the other operand requires values from last
        // lane.
        Builder.create<VPPrivateLastValueNonPODInst>(
            ".priv.lastval.nonpod", Type::getVoidTy(*Plan.getLLVMContext()),
            ArrayRef<VPValue *>{PrivateMem, AI},
            PrivateNonPOD->getCopyAssign());
      }

    } else if (Private->isLast()) {

      // Handling of last privates generating last value calculation.
      if (Private->hasPrivateTag()) {
        // No last value for non-pod types and arrays
        assert(Private->getPrivateTag() != VPPrivate::PrivateTag::PTNonPod &&
               "Unsupported aggregate type");

        if (Private->getPrivateTag() == VPPrivate::PrivateTag::PTInMemory) {
          // No last value for unused in-memory privates. This is an explicit
          // private which was completely registerized.
          if (!VPEntityImportDescr::hasRealUserInLoop(PrivateMem, &Loop))
            continue;
        }

        if (Private->getPrivateTag() == VPPrivate::PrivateTag::PTArray) {
          VPBuilder::InsertPointGuard Guard(Builder);
          Builder.setInsertPoint(PostExit);
          Builder.createNaryOp(VPInstruction::PrivateFinalArray,
                               Type::getVoidTy(*Plan.getLLVMContext()),
                               {PrivateMem, AI});
          continue;
        }
      }

      if (Private->isConditional()) {
        insertConditionalLastPrivateInst(*Private, Builder, Preheader, PostExit,
                                         PrivateMem, AI);
        continue;
      }

      assert((PrivateMem || !Private->getIsMemOnly()) &&
             "MemOnly private is expected to have PrivateMem.");

      VPBuilder::InsertPointGuard Guard(Builder);
      Builder.setInsertPoint(PostExit);
      VPValue *Exit =
          Private->getIsMemOnly()
              ? Builder.createLoad(Private->getAllocatedType(), PrivateMem)
              : Private->getExitInst();
      StringRef Name = Private->hasExitInstr()
                           ? Private->getExitInst()->getName()
                           : "loaded";
      unsigned Opc = Private->getIsMemOnly()
                         ? VPInstruction::PrivateFinalUncondMem
                         : VPInstruction::PrivateFinalUncond;
      auto *Final = Builder.createNaryOp(Opc, Exit->getType(), {Exit});
      Final->setName(Name + ".priv.final");
      processFinalValue(*Private, AI, Builder, *Final, Exit->getType(), Exit);
    }
  }
  LLVM_DEBUG(
      dbgs()
      << "After replacement of private and aliases within the preheader.\n");
  LLVM_DEBUG(Preheader->dump());
}

// Check whether last private phi has correct users. The correct user is
// either phi or select instructions, or hir-copy, or \p ExitI. Other uses mean
// that we have a dependency between loop iterations and thus we can't vectorize
// the loop.
static bool checkLastPrivPhiUsers(const VPPHINode *VPhi,
                                  const VPInstruction *ExitI) {
  SmallVector<const VPInstruction *, 4> Worklist;
  SmallPtrSet<const VPValue *, 4> Visited;

  Worklist.push_back(VPhi);
  Visited.insert(ExitI);

  while (!Worklist.empty()) {
    const VPInstruction *Cur = Worklist.pop_back_val();
    // If it's not ExitI and not phi and not hir-copy and not select with 0th
    // operand not in the visited set then it's an incorrect use.
    if (Cur != ExitI && !isa<VPPHINode>(Cur) && !isa<VPHIRCopyInst>(Cur) &&
        !(Cur->getOpcode() == Instruction::Select &&
          Visited.count(Cur->getOperand(0)) == 0)) {
      LLVM_DEBUG(dbgs() << "Incorrect use of private: \n");
      LLVM_DEBUG(Cur->dump());
      return false;
    }
    if (!Visited.insert(Cur).second)
      continue;
    for (auto *U : Cur->users()) {
      if (auto *I = dyn_cast<VPInstruction>(U)) {
        Worklist.push_back(I);
      } else {
        // This means it's a second liveout.
        LLVM_DEBUG(dbgs() << "Second liveout of conditional private phi: \n");
        LLVM_DEBUG(Cur->dump());
        return false;
      }
    }
  }
  return true;
}

// Return true if the \p ExitI does not have any recurrent phi in its operand
// chain, except inductions.
//
static bool checkUncondLastPrivOperands(const VPInstruction *ExitI,
                                        VPLoopEntityList *LE) {
  SmallVector<const VPInstruction *, 4> Worklist;
  SmallPtrSet<const VPInstruction *, 4> Visited;

  const VPBasicBlock *HeaderBB = LE->getLoop().getHeader();

  Worklist.push_back(ExitI);
  while (!Worklist.empty()) {
    const VPInstruction *Cur = Worklist.pop_back_val();
    if (auto VPhi = dyn_cast<VPPHINode>(Cur)) {
      if (VPhi->getParent() == HeaderBB && !LE->getInduction(VPhi)) {
        LLVM_DEBUG(
            dbgs()
            << "Incorrect recurrent operand of unconditional private: \n");
        LLVM_DEBUG(Cur->dump());
        return false;
      }
    }
    if (!Visited.insert(Cur).second)
      continue;
    for (auto *Op : Cur->operands())
      if (auto *VInst = dyn_cast<VPInstruction>(Op)) {
        if (VInst->getOpcode() == Instruction::Load) {
          // Skip a load, it produces a new value that does not use operands
          // directly.
          continue;
        }
        Worklist.push_back(VInst);
      }
  }
  return true;
}

// Calculate kind of a last private, using its exit instruction \p Inst. Returns
// optional pair <VPValue*, PrivateKind>. For conditonal last privates the
// pair.first is the VPHINode from the loop header and PrivateKind is
// VPPrivate::PrivateKind::Conditional. For unconditional last privates the pair
// is <nullptr, PrivateKind::Last>, and for non-vectorizable cases the return
// value is None.

using PrivKindPair = Optional<std::pair<VPValue *, VPPrivate::PrivateKind>>;

static PrivKindPair getPrivateKind(VPInstruction *Inst, VPLoopEntityList *LE) {
  const VPBasicBlock *HeaderBB = LE->getLoop().getHeader();
  auto IsHeaderPhi = [HeaderBB](const VPValue *V) {
    auto *I = dyn_cast<VPPHINode>(V);
    return I && I->getParent() == HeaderBB;
  };

  if (IsHeaderPhi(Inst)) {
    if (LE->getInduction(Inst)) {
      // Second exit of the induction can be classified as non conditional
      // lastprivate.
      return PrivKindPair(std::make_pair(Inst, VPPrivate::PrivateKind::Last));
    }
    // TODO: create a check for operands of the header phi, there should be no
    // cross iteration dependencies.
    return None;
  }

  auto Iter = llvm::find_if(Inst->users(), IsHeaderPhi);
  if (Iter != Inst->user_end()) {
    // If it's used in a header phi then it should be conditional. In such cases
    // we allow the uses only in the header and outside of the loop, i.e. only
    // two uses. Preventing, e.g., case like below (%exitI is declared as last
    // private).
    //
    //    %header_phi = phi [%start_val], [%exitI]
    //    %cond = some_comparison ...
    //    %exitI = select %cond, %some_other, %header_phi
    //    store %exitI, %some_ptr
    //
    // 1) According to OMP standard, the %exitI has undefined values in the
    //    masked lanes (at least on the first vector itertaion).
    // 2) Consider the following mask at some vector interation and
    //    the values that will be stored by scalar execution.
    //    iter#    0, 1,  2,  3,  4,  5,  6, 7
    //    mask  =  0, 0,  1,  0,  0,  1,  0, 1
    //    exitI =  p, p, v2, v2, v2, v5, v5, v7
    //    Here p is value from the last lane of the previous vector iteration,
    //    vN are the values calculated in the corresponding lanes)
    //
    // I.e. the values in the masked lanes should be filled in with the values
    // from the latest previous unmasked lane. It's not simple to place such
    // values correctly in the vector register, and even that is possible to
    // implement the resulting code will be very slow. Until we don't have
    // support for such code generation (and the proper cost model) we bail out.
    //
    if (Inst->getNumUsers() != 2)
      return PrivKindPair(
          std::make_pair(nullptr, VPPrivate::PrivateKind::NonLast));

    if (checkLastPrivPhiUsers(cast<VPPHINode>(*Iter), Inst))
      return PrivKindPair(
          std::make_pair(*Iter, VPPrivate::PrivateKind::Conditional));

    return None;
  }

  // A value which is assigned uconditionally. We consider it as a safe
  // last private if does not use any recurrence, except known inductions.
  if (checkUncondLastPrivOperands(Inst, LE))
    return PrivKindPair(std::make_pair(nullptr, VPPrivate::PrivateKind::Last));

  return None;
}

void VPLoopEntityList::analyzeImplicitLastPrivates() {
  for (auto *BB : Loop.blocks())
    for (VPInstruction &Inst : *BB) {

      if (!Loop.isLiveOut(&Inst) || getReduction(&Inst) || getPrivate(&Inst))
        continue;

      // Induction can have more than one liveout linked with it. E.g. we can
      // have the header phi and the increment instruction both liveout. In such
      // cases, we create a private for instructions that will not be handled by
      // VPInductionFinal.
      // TODO: Find a more efficient way. Having a private will cause having
      // vector value always and last value will be done by extracting value
      // from last lane. We can try to improve that having a second VPInduction
      // or just second VPInductionFinal.
      if (const VPInduction *Ind = getInduction(&Inst))
        if (&Inst == getInductionLoopExitInstr(Ind))
          continue;
      PrivKindPair PrivPair = getPrivateKind(&Inst, this);
      if (!PrivPair)
        // Liveout is not recognized as last private, will bailout later.
        continue;

      VPPrivate::PrivateKind Kind;
      VPValue *HeaderPhi;
      std::tie(HeaderPhi, Kind) = *PrivPair;

      // Add new private with empty alias list
      VPEntityAliasesTy EmptyAliases;
      VPPrivate *Priv =
          addPrivate(&Inst, EmptyAliases, Kind,
                     /* Explicit */ false, Inst.getType(), /* AI */ nullptr,
                     /* MemOnly */ false);
      linkValue(Priv, HeaderPhi);
    }
}

// Insert VPInstructions corresponding to the VPLoopEntities like
// VPInductions, VPReductions and VPPrivates.
void VPLoopEntityList::insertVPInstructions(VPBuilder &Builder) {
  // If the loop is multi-exit then the code gen for it is done using
  // underlying IR and we don't need to emit anything here.
  if (!Loop.getUniqueExitBlock())
    return;

  preprocess();

  VPBasicBlock *PostExit = Loop.getUniqueExitBlock();
  VPBasicBlock *Preheader = Loop.getLoopPreheader();

  // Insert VPInstructions related to VPReductions.
  insertReductionVPInstructions(Builder, Preheader, PostExit);

  // Insert VPInstructions related to VPInductions.
  insertInductionVPInstructions(Builder, Preheader, PostExit);

  // Insert VPInstructions related to VPPrivates.
  insertPrivateVPInstructions(Builder, Preheader, PostExit);
}

// Create close-form calculation for induction.
// The close-form calculation is the one by the formula:
//      v = (i MUL step) OP v0
// where 'v0' is its initial value, 'i' is primary induction variable (IV),
// 'step' is per iteration increment value of induction variable 'v'.
// The need of close-form (see also InductionDescr::inductionNeedsCloseForm)
// means that we need up-to-date induction value at the beginning of each loop
// iteration. That can be achieved in two ways (assuming "+" induction):
// either (1) insert calculation exactly by the formula at the beginning of
// the loop, or (2) insert increment of the induction in the end of the loop
// (see examples below). In both cases the Init and InitStep are generated.
//
// The initial loop (primary IV not shown):
// DO
//   %ind = phi(init, %inc_ind)
//   ... uses of %ind
//   %inc_ind = %ind OP step
//   ... uses of inc_ind
// ENDDO
//
// Possible transformations.
// Init = Initialize(identity, VF, OP)      ! generated always
// InitStep = InitializeStep(Step, VF, OP)  ! generated always
// ...
// variant 1 (exact close form)
// DO
//   %temp = InitStep MUL %primary_IV       ! new instruction
//   %Induction = %temp OP Init             ! new instruction
//   ... uses of %Induction (replacing %ind)
//   %inc_ind = %Induction OP step          ! step is not replaced
//   ... uses of %inc_ind
// ENDDO
//
// variant 2 (simplest case)
// DO
//   %ind = phi(Init, %Induction)       ! %ind replaced by %Induction
//   ... uses of %ind
//   %inc_ind = %ind OP step            ! step is not replaced
//   ... uses of %inc_ind
//   %Induction = %ind OP %InitStep     ! new instruction
// ENDDO
//
// For loop with in-memory induction (no start-phi), before transformation:
// DO
//   %ind = load %induction_mem
//   ... uses of %ind
//   %inc_ind = %ind OP step
//   ... uses of %inc_ind
//   store %inc_ind, %induction_mem
// ENDDO
// (skipping variant 1, it will not be generated anyway)
// variant 2:
// DO
//   %Induction_phi= phi(Init,%Induction)       ! new instruction
//   store %Induction_phi, %Induction_priv_mem  ! new instruction
//   %ind = load %Induction_priv_mem
//   ... uses of %ind
//   %inc_ind = %ind OP step                    ! step is not replaced
//   ... uses of %inc_ind
//   store %inc_ind, %Induction_priv_mem        ! this is redundant
//   %Induction = %Induction_phi OP %InitStep   ! new instruction
// ENDDO
//
// The routine generates variant 2 as preferable.
//
void VPLoopEntityList::createInductionCloseForm(VPInduction *Induction,
                                                VPBuilder &Builder,
                                                VPValue &Init,
                                                VPValue &InitStep,
                                                VPValue &PrivateMem) {
  VPBuilder::InsertPointGuard Guard(Builder);

  auto CreateNewInductionOp =
      [&Builder](VPPHINode *Phi, VPValue *Step,
                 const VPInduction *Ind) -> VPInstruction * {
    unsigned int Opc = Ind->getInductionOpcode();
    Type *Ty = Ind->getStartValue()->getType();

    if ((Opc == Instruction::Add && Ty->isPointerTy()) ||
        Opc == Instruction::GetElementPtr) {
      // FIXME: Don't reference getElementType as it won't exist for opaque
      // pointers. Propagate it through whole VPEntities framework.
      auto *GEP =
          Builder.createGEP(cast<PointerType>(Phi->getType())->getElementType(),
                            cast<PointerType>(Phi->getType())->getElementType(),
                            Phi, Step, nullptr);
      GEP->setIsInBounds(true); // TODO: Why is that correct?
      return GEP;
    }
    return Builder.createNaryOp(Opc, Ty, {Phi, Step});
  };

  VPBasicBlock *LatchBlock = Loop.getLoopLatch();
  assert(LatchBlock && "expected non-null latch");

  if (auto BinOp = Induction->getInductionBinOp()) {
    // Non-memory induction.
    VPBranchInst *Br = LatchBlock->getTerminator();
    auto *LatchCond = cast<VPInstruction>(Br->getCondition());
    VPPHINode *StartPhi = findInductionStartPhi(Induction);
    assert(StartPhi && "null induction StartPhi");

    if (isa<VPPHINode>(BinOp))
      Builder.setInsertPointFirstNonPhi(BinOp->getParent());
    else
      Builder.setInsertPoint(BinOp);
    VPInstruction *NewInd =
        CreateNewInductionOp(StartPhi, &InitStep, Induction);
    // TODO: add copying of other attributes.
    NewInd->setDebugLocation(BinOp->getDebugLocation());

    StartPhi->replaceUsesOfWith(BinOp, NewInd);
    if (LatchCond->getNumOperandsFrom(BinOp) != 0 &&
        LatchCond->getNumUsers() > 1) {
      // If the latch condition uses this induction and is used somewhere else
      // we need to duplicate it and leave all old uses untouched except one
      // in the back edge.
      VPInstruction *LatchCond2 = LatchCond->clone();
      Builder.setInsertPoint(LatchCond);
      Builder.insert(LatchCond2);
      LatchBlock->setCondBit(LatchCond2);
      LatchCond = LatchCond2;
    }
    LatchCond->replaceUsesOfWith(BinOp, NewInd);

    VPInstruction *ExitIns = getInductionLoopExitInstr(Induction);
    if (ExitIns == BinOp)
      relinkLiveOuts(ExitIns, BinOp, Loop);
    linkValue(Induction, NewInd);
    return;
  }

  // In-memory induction.
  // First insert phi and store after existing PHIs in loop header block.
  // See comment before the routine.
  Builder.setInsertPointFirstNonPhi(Loop.getHeader());
  VPPHINode *IndPhi =
      Builder.createPhiInstruction(Induction->getStartValue()->getType());
  Builder.createStore(IndPhi, &PrivateMem);
  // Then insert increment of induction and update phi.
  Builder.setInsertPoint(LatchBlock);
  VPInstruction *NewInd = CreateNewInductionOp(IndPhi, &InitStep, Induction);
  // Step is always initialized in loop preheader.
  IndPhi->addIncoming(&Init, Loop.getLoopPreheader());
  IndPhi->addIncoming(NewInd, LatchBlock);
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

static bool checkInstructionInLoop(const VPValue *V, const VPLoop *Loop) {
  // Check for null and VPInstruction here to avoid these checks at caller(s)
  // side
  return V == nullptr || !isa<VPInstruction>(V) ||
         Loop->contains(cast<VPInstruction>(V));
}

void ReductionDescr::checkParentVPLoop(const VPLoop *Loop) const {
  assert((checkInstructionInLoop(StartPhi, Loop) &&
          checkInstructionInLoop(Exit, Loop) &&
          checkInstructionInLoop(LinkPhi, Loop)) &&
         "Parent loop does not match instruction");
}

bool ReductionDescr::isIncomplete() const {
  return StartPhi == nullptr || Start == nullptr || RT == nullptr ||
         Exit == nullptr;
}

bool ReductionDescr::isDuplicate(const VPlanVector *Plan, const VPLoop *Loop) const {
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

VPPHINode *ReductionDescr::getLastNonheaderPHIUser(VPInstruction *VPInst,
                                                   const VPLoop *Loop) {
  SetVector<VPPHINode *> Worklist;
  auto AddPHIUsersToWorklist = [&Worklist, Loop](VPInstruction *VPI) -> void {
    for (auto *U : VPI->users()) {
      if (auto *PhiUser = dyn_cast<VPPHINode>(U))
        if (checkInstructionInLoop(PhiUser, Loop) &&
            PhiUser->getParent() != Loop->getHeader())
          Worklist.insert(PhiUser);
    }
  };
  AddPHIUsersToWorklist(VPInst);

  VPPHINode *LastNonheaderPHI = nullptr;
  while (!Worklist.empty()) {
    VPPHINode *CurrPHI = Worklist.pop_back_val();
    // Add CurrPHI to linked VPValues and recurse on its PHI users.
    LinkedVPVals.push_back(CurrPHI);
    LastNonheaderPHI = CurrPHI;
    AddPHIUsersToWorklist(CurrPHI);
  }

  return LastNonheaderPHI;
}

// This method tries to populate missing data in a ReductionDescr by using
// additional knowledge from the VPlan CFG (def-use chains) that this reduction
// belongs to. Following is the series of checks and cases handled -
//
// 1. Unknown loop exit instruction
//    - In case of explicit reductions, additional analysis is performed to
//      identify reduction's loop exit instruction.
//    - Replacing original descriptor with an alias is done here if needed.
//    - Exit instruction is expected to be live-out (nullptr otherwise).
//    - Masked reduction scenario is also accounted for here.
//
// 2. Unknown recurrence PHI when loop exit instruction is known
//    - Given that Exit is known we try to find recurrence PHI for this
//      reduction based on users of Exit.
//    - Exit can be non-liveout in cases like masked HIR SafeReductions.
//    - Masked reduction pattern is also accounted for here.
//
// 3. Unknown recurrence PHI based on linked VPValues
//    - If checks in Case 2 fail then we try to identify recurrence PHI based on
//      users of reduction's linked VPValues.
//
// 4. Pure in-memory reduction pattern
//    - If checks in both Case 2 and Case 3 fail, then we are dealing with an
//      in-memory reduction.
//    - Memory uses (load/store) of reduction's Start value inside the loop are
//      identified.
//
// 5. Unknown Start value when recurrence PHI is known
//    - Live-in or const operand of recurrence PHI is identified as reduction's
//      Start value.
//
// 6. Unknown recurrence type
//    - Determined based on type of reduction's Exit/recurrence PHI/Start value.
void ReductionDescr::tryToCompleteByVPlan(const VPlanVector *Plan,
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
    if (!Loop->isLiveOut(Exit)) {
      // Check if loop exit is used by any non-header PHIs (masked reduction
      // scenarios).
      if (auto *Phi = getLastNonheaderPHIUser(Exit, Loop)) {
        LinkedVPVals.push_back(Exit);
        Exit = Phi;
      }
    }

    // Find out if Exit is used in a potential StartPhi.
    for (auto *U : Exit->users()) {
      auto *PhiUser = dyn_cast<VPPHINode>(U);
      if (!PhiUser)
        continue;

      // Reduction's StartPhi will be in loop's header block and blends a
      // live-in or const operand.
      if (PhiUser->getParent() == Loop->getHeader() &&
          getLiveInOrConstOperand(PhiUser, *Loop)) {
        StartPhi = PhiUser;
        break;
      }
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
        if (auto *Instr = dyn_cast<VPPHINode>(User))
          if (Instr->getParent() == Loop->getHeader() &&
              getLiveInOrConstOperand(Instr, *Loop) &&
              is_contained(Instr->operands(), Start)) {
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

void ReductionDescr::passToVPlan(VPlanVector *Plan, const VPLoop *Loop) {
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
    VPRed = LE->addReduction(StartPhi, Start, Exit, K, RedFMF, RT, Signed,
                             AllocaInst, ValidMemOnly);
  else {
    const VPReduction *Parent = LE->getReduction(LinkPhi);
    assert(Parent && "nullptr is unexpected");
    bool ForLast = LE->isMinMaxLastItem(*Parent);
    VPRed =
        LE->addIndexReduction(StartPhi, Parent, Start, Exit, RT, Signed,
                              ForLast, IsLinearIndex, AllocaInst, ValidMemOnly);
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
    if (UpdateVPInsts[0]->getOpcode() == Instruction::Store)
      // In-memory reduction, no more analysis needed
      return nullptr;
    LoopExitVPI = UpdateVPInsts[0];
  }

  // Case where descriptor has multiple update instructions
  if (UpdateVPInsts.size() > 1) {
    if (allUpdatesAreStores(UpdateVPInsts))
      return nullptr; // In-memory reduction

    auto IsLiveOut = [Loop](const VPInstruction *I) {
      return Loop->isLiveOut(I);
    };

    auto Iter = llvm::find_if(UpdateVPInsts, IsLiveOut);
    if (Iter != UpdateVPInsts.end()) {
      assert(llvm::count_if(UpdateVPInsts, IsLiveOut) == 1 &&
             "non-single liveout reduction value");
      LoopExitVPI = *Iter;
    }
  }

  // Live-out analysis tests for LoopExit
  if (!LoopExitVPI)
    return nullptr;

  while (!Loop->isLiveOut(LoopExitVPI) &&
         (isTrivialBitcast(LoopExitVPI) || isa<VPHIRCopyInst>(LoopExitVPI))) {
    // Add the trivial copy to linked VPVals for safety
    LinkedVPVals.push_back(LoopExitVPI);
    LoopExitVPI = dyn_cast<VPInstruction>(
        LoopExitVPI->getOperand(0)); // Copy has only one operand
    assert(LoopExitVPI && "Input for copy is not a VPInstruction.");
  }

  // Check if loop exit VPI is used by any non-header PHIs (masked reduction
  // scenarios).
  if (!Loop->isLiveOut(LoopExitVPI)) {
    if (auto *Phi = getLastNonheaderPHIUser(LoopExitVPI, Loop)) {
      LinkedVPVals.push_back(LoopExitVPI);
      LoopExitVPI = Phi;
    }
  }

  // If the final loop exit VPI is still not live-out then store the VPI to
  // linked VPVals and return null, as private memory will be needed to
  // perform this reduction. Example test -
  // Transforms/Intel_VPO/Vecopt/hir_simd_descr_vpentities_priv_memory.ll
  if (!Loop->isLiveOut(LoopExitVPI)) {
    LinkedVPVals.push_back(LoopExitVPI);
    LoopExitVPI = nullptr;
  }

  return LoopExitVPI;
}

bool PrivateDescr::updateKind(VPLoopEntityList *LE) {
  if (ExitInst && ExitInst->getOpcode() != Instruction::Store) {
    setIsMemOnly(false);
    PrivKindPair PrivPair = getPrivateKind(ExitInst, LE);
    if (!PrivPair)
      return false;
    VPPrivate::PrivateKind Kind = VPPrivate::PrivateKind::Last;
    std::tie(std::ignore, Kind) = *PrivPair;
    IsLast = Kind >= VPPrivate::PrivateKind::Last;
    IsConditional = Kind == VPPrivate::PrivateKind::Conditional;
  }
  return true;
}

void PrivateDescr::passToVPlan(VPlanVector *Plan, const VPLoop *Loop) {
  using PrivKind = VPPrivate::PrivateKind;

  VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(Loop);
  if (!updateKind(LE))
    return;

  PrivKind K = IsLast ? (IsConditional ? PrivKind::Conditional : PrivKind::Last)
                      : PrivKind::NonLast;
  // If private has non-null constructor/destructor fields, then it is expected
  // to be of non-POD type. TODO: Add check for CopyAssign when support is added
  // to insertPrivateVPInstructions.
  Type *AllocatedTy = getAllocatedType();
  if (Ctor || Dtor)
    LE->addNonPODPrivate(PtrAliases, K, IsExplicit, Ctor, Dtor, CopyAssign,
                         AllocatedTy, AllocaInst);
  else if (PTag == VPPrivate::PrivateTag::PTRegisterized) {
    assert(ExitInst && "ExitInst is expected to be non-null here.");
    if (LE->getReduction(ExitInst)) {
      // In case a reduction (even auto-recognized) is declared as a
      // last private we will have a duplication and assert. Skipping private
      // importing for now.
      // TODO: 1) implement a check for such duplication before importing, 2)
      // pass a message to optreport.
      LLVM_DEBUG(dbgs() << "Skipping already imported private for ";
                 AllocaInst->printAsOperand(dbgs()););
      return;
    }
    auto Priv = LE->addPrivate(ExitInst, PtrAliases, K, IsExplicit, AllocatedTy,
                               AllocaInst, isMemOnly());
    for (VPInstruction *I : UpdateVPInsts)
      LE->linkValue(Priv, I);
  } else
    LE->addPrivate(PTag, PtrAliases, K, IsExplicit, AllocatedTy, AllocaInst,
                   isMemOnly());
}

void PrivateDescr::checkParentVPLoop(const VPLoop *Loop) const {
  // TODO: Add robust check for FinalI and some more checks related to
  // AllocaInst in this function.
}

void PrivateDescr::tryToCompleteByVPlan(const VPlanVector *Plan,
                                        const VPLoop *Loop) {
  setIsMemOnly(true);

  VPPHINode *VPhi = nullptr;
  if (HasAlias)
    for (VPInstruction *It : Alias.getValue().UpdateVPInsts)
      UpdateVPInsts.push_back(It);

  // Go through update instructions to find liveout and/or header phi.
  const VPBasicBlock *LHeader = Loop->getHeader();
  for (VPInstruction *It : UpdateVPInsts) {
    if (Loop->isLiveOut(It)) {
      // Check that we don't have two different liveouts.
      assert((!ExitInst || It == ExitInst) && "Second liveout of private");
      ExitInst = It;
    }
    auto HdrUseIt = llvm::find_if(It->users(), [LHeader](VPUser *U) {
      auto Phi = dyn_cast<VPPHINode>(U);
      return Phi && Phi->getParent() == LHeader;
    });
    if (HdrUseIt != It->user_end()) {
      // Check that we don't have two different recurrent phis.
      assert((!VPhi || VPhi == *HdrUseIt) && "second recurrent phi");
      VPhi = cast<VPPHINode>(*HdrUseIt);
    }
  }
  if (VPhi) {
    UpdateVPInsts.push_back(VPhi);
    // check for is conditional??
  }
  if (ExitInst) {
    PTag = VPPrivate::PrivateTag::PTRegisterized;
    setIsMemOnly(false);
    return;
  }

  if (!isScalarTy(AllocatedType) &&
      !AllocatedType->isVectorTy()) {
    PTag = VPPrivate::PrivateTag::PTArray;
    return;
  }

  PTag = VPPrivate::PrivateTag::PTInMemory;
}

bool VPEntityImportDescr::isDuplicate(const VPlanVector *Plan,
                                      const VPLoop *Loop) const {
  const VPLoopEntityList *LE = Plan->getLoopEntities(Loop);
  // It's not first (LE exists) and already have the same alloca instruction
  return LE && AllocaInst && LE->getMemoryDescriptor(AllocaInst);
}

bool VPEntityImportDescr::hasRealUserInLoop(VPValue *Val, const VPLoop *Loop) {
  SmallVector<VPValue *, 4> WorkList(Val->users());
  while (!WorkList.empty()) {
    VPValue *Cur = WorkList.pop_back_val();
    if (isa<VPExternalUse>(Cur))
      continue;
    auto I = cast<VPInstruction>(Cur);
    if (!Loop->contains(I) && I->getParent() != Loop->getLoopPreheader())
      continue;
    if (I->getOpcode() == Instruction::BitCast ||
        I->getOpcode() == Instruction::AddrSpaceCast) {
      WorkList.append(I->user_begin(), I->user_end());
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
    for (auto *User : Start->users()) {
      auto *Instr = dyn_cast<VPLoadStoreInst>(User);
      if (!Instr || !Loop->contains(Instr))
        continue;

      if (Instr->getOpcode() == Instruction::Load)
        LdStInstr = Instr;
      else
        LdStInstr = Instr->getOperand(0);
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

void InductionDescr::checkParentVPLoop(const VPLoop *Loop) const {
  assert((checkInstructionInLoop(StartPhi, Loop) &&
          checkInstructionInLoop(Start, Loop) &&
          checkInstructionInLoop(InductionOp, Loop)) &&
         "Parent loop does not match instruction");
}

bool InductionDescr::isIncomplete() const {
  return StartPhi == nullptr || InductionOp == nullptr || Step == nullptr;
}

bool InductionDescr::isDuplicate(const VPlanVector *Plan,
                                 const VPLoop *Loop) const {
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
  if (!LE || !InductionOp)
    return false;

  const VPInduction *OtherInd = LE->getInduction(InductionOp);
  if (!OtherInd)
    return false;

  if (OtherInd->getStartValue() == Start) {
    // FIXME: At least rename the method... Modifying state isn't the behavior
    // expected from a "const" method with the name looking like a const
    // method name.

    // Record that current induction descriptor's PHI node duplicates another
    // induction descriptor (OtherInd)
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

    auto *UserVPI = cast<VPInstruction>(User);

    // Ignore this user if it is -
    // 1. Non-loop user of increment instruction.
    // 2. Already processed in recursion chain.
    // 3. One of the whitelist instruction listed above.
    if (!Loop->contains(UserVPI) || AnalyzedVPIs.count(UserVPI) ||
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
  } else {
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

void InductionDescr::passToVPlan(VPlanVector *Plan, const VPLoop *Loop) {
  if (!Importing)
    return;

  VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(Loop);
  VPInduction *VPInd =
      LE->addInduction(StartPhi, Start, K, Step, StartVal, EndVal, InductionOp,
                       IndOpcode, AllocaInst, ValidMemOnly);
  if (inductionNeedsCloseForm(Loop))
    VPInd->setNeedCloseForm(true);
}

void InductionDescr::tryToCompleteByVPlan(const VPlanVector *Plan,
                                          const VPLoop *Loop) {
  if (StartPhi == nullptr) {
    VPValue *V = InductionOp ? InductionOp : Start;
    for (auto User : V->users())
      if (auto Instr = dyn_cast<VPPHINode>(User))
        if (Loop->contains(Instr) && getLiveInOrConstOperand(Instr, *Loop)) {
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

  if (isa_and_nonnull<VPPHINode>(StartPhi)) {
    if (InductionOp == nullptr)
      InductionOp = dyn_cast<VPInstruction>(StartPhi->getOperand(0) == Start
                                                ? StartPhi->getOperand(1)
                                                : StartPhi->getOperand(0));
    else if (Start == nullptr)
      Start =
          (StartPhi->getOperand(0) == InductionOp ? StartPhi->getOperand(1)
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
