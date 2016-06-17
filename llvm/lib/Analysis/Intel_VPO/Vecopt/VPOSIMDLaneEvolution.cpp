//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOSIMDLaneEvolution.cpp -- Defines the SIMD Lane Evolution analysis.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOSIMDLaneEvolution.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/Passes.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrStmt.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrIf.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPODefUse.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/BlobUtils.h"

#include "llvm/PassSupport.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/GenericDomTreeConstruction.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/FileSystem.h"

#define DEBUG_TYPE "vpo-simd-lane-evolution"

using namespace llvm;
using namespace vpo;

INITIALIZE_PASS_BEGIN(SIMDLaneEvolution, "slev",
                      "VPO SIMD Lane Evolution Analysis",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(AvrDefUse)
INITIALIZE_PASS_DEPENDENCY(AvrCFG)
INITIALIZE_PASS_DEPENDENCY(AVRGenerate)
INITIALIZE_PASS_END(SIMDLaneEvolution, "slev",
                    "VPO SIMD Lane Evolution Analysis",
                    false, true)

char SIMDLaneEvolution::ID = 0;

const SLEV::Number SLEV::Zero(APSInt(APInt(64, 0), false)); 
const SLEV::Number SLEV::One(APSInt(APInt(64, 1), false));

const char* SLEV::SLEVKINDSTR[SLEVKind::RANDOM + 1] = {
  "BOTTOM", "CONSTANT", "UNIFORM", "STRIDED", "RANDOM"
};

SLEVKind SLEVAdd::Conversion[RANDOM + 1][RANDOM + 1] = {
  /*               BOTTOM,   CONSTANT, UNIFORM, STRIDED, RANDOM */
  /* BOTTOM   */  {BOTTOM,   BOTTOM,   BOTTOM,  BOTTOM,  RANDOM},
  /* CONSTANT */  {BOTTOM,   CONSTANT, UNIFORM, STRIDED, RANDOM},
  /* UNIFORM  */  {BOTTOM,   UNIFORM,  UNIFORM, STRIDED, RANDOM},
  /* STRIDED  */  {BOTTOM,   STRIDED,  STRIDED, STRIDED, RANDOM},
  /* RANDOM   */  {RANDOM,   RANDOM,   RANDOM,  RANDOM,  RANDOM},
};

SLEVKind SLEVSub::Conversion[RANDOM + 1][RANDOM + 1] = {
  /*               BOTTOM,   CONSTANT, UNIFORM, STRIDED, RANDOM */
  /* BOTTOM   */  {BOTTOM,   BOTTOM,   BOTTOM,  BOTTOM,  RANDOM},
  /* CONSTANT */  {BOTTOM,   CONSTANT, UNIFORM, STRIDED, RANDOM},
  /* UNIFORM  */  {BOTTOM,   UNIFORM,  UNIFORM, STRIDED, RANDOM},
  /* STRIDED  */  {BOTTOM,   STRIDED,  STRIDED, STRIDED, RANDOM},
  /* RANDOM   */  {RANDOM,   RANDOM,   RANDOM,  RANDOM,  RANDOM},
};

SLEVKind SLEVMul::Conversion[RANDOM + 1][RANDOM + 1] = {
  /*               BOTTOM,   CONSTANT, UNIFORM, STRIDED, RANDOM */
  /* BOTTOM   */  {BOTTOM,   BOTTOM,   BOTTOM,  BOTTOM,  RANDOM},
  /* CONSTANT */  {BOTTOM,   CONSTANT, UNIFORM, STRIDED, RANDOM},
  /* UNIFORM  */  {BOTTOM,   UNIFORM,  UNIFORM, RANDOM,  RANDOM},
  /* STRIDED  */  {BOTTOM,   STRIDED,  RANDOM,  STRIDED, RANDOM},
  /* RANDOM   */  {RANDOM,   RANDOM,   RANDOM,  RANDOM,  RANDOM},
};

SLEVKind SLEVDiv::Conversion[RANDOM + 1][RANDOM + 1] = {
  /*               BOTTOM,   CONSTANT, UNIFORM, STRIDED, RANDOM */
  /* BOTTOM   */  {BOTTOM,   BOTTOM,   BOTTOM,  BOTTOM,  RANDOM},
  /* CONSTANT */  {BOTTOM,   CONSTANT, UNIFORM, RANDOM,  RANDOM},
  /* UNIFORM  */  {BOTTOM,   UNIFORM,  UNIFORM, RANDOM,  RANDOM},
  /* STRIDED  */  {BOTTOM,   RANDOM,   RANDOM,  RANDOM,  RANDOM},
  /* RANDOM   */  {RANDOM,   RANDOM,   RANDOM,  RANDOM,  RANDOM},
};

SLEVKind SLEVCmp::Conversion[RANDOM + 1][RANDOM + 1] = {
  /*               BOTTOM,   CONSTANT, UNIFORM, STRIDED, RANDOM */
  /* BOTTOM   */  {BOTTOM,   BOTTOM,   BOTTOM,  BOTTOM,  RANDOM},
  /* CONSTANT */  {BOTTOM,   CONSTANT, UNIFORM, RANDOM,  RANDOM},
  /* UNIFORM  */  {BOTTOM,   UNIFORM,  UNIFORM, RANDOM,  RANDOM},
  /* STRIDED  */  {BOTTOM,   RANDOM,   RANDOM,  RANDOM,  RANDOM},
  /* RANDOM   */  {RANDOM,   RANDOM,   RANDOM,  RANDOM,  RANDOM},
};

unsigned long long SLEV::NextId = 0;

void SLEV::Number::print(raw_ostream& OS) const {
  if (!Defined) {
    OS << "<?>";
    return;
  }
  if (IsInteger)
    OS << IntN;
  else {
    SmallVector<char, 10> StrFloat;
    FloatN.toString(StrFloat);
    OS << StrFloat;
  }
}

bool SLEV::Number::operator==(const Number& O) const {

  if (!Defined)
    return !O.Defined;

  if (!O.Defined)
    return false;

  if (IsInteger != O.IsInteger)
    return false;

  if (IsInteger)
    return APInt::isSameValue(IntN, O.IntN);

  return FloatN.compare(O.FloatN) == APFloat::cmpEqual;
}

SLEV::Number SLEV::Number::operator+(const Number& RHS) const {

  assert(Defined && RHS.Defined && "Operating on undefined numbers");

  if (IsInteger && RHS.IsInteger) {
    return Number(IntN + RHS.IntN);
  }

  if (IsInteger) {
    APFloat AsFloat(RHS.FloatN.getSemantics());
    APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
    AsFloat.convertFromAPInt(IntN, IntN.isSigned(), rm);
    return Number(AsFloat + RHS.FloatN);
  }

  if (RHS.IsInteger) {
    APFloat AsFloat(FloatN.getSemantics());
    APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
    AsFloat.convertFromAPInt(RHS.IntN, RHS.IntN.isSigned(), rm);
    return Number(FloatN + AsFloat);
  }

  return Number(FloatN + RHS.FloatN);
}

SLEV::Number SLEV::Number::operator-(const Number& RHS) const {

  assert(Defined && RHS.Defined && "Operating on undefined numbers");

  if (IsInteger && RHS.IsInteger) {
    return Number(IntN - RHS.IntN);
  }

  if (IsInteger) {
    APFloat AsFloat(RHS.FloatN.getSemantics());
    APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
    AsFloat.convertFromAPInt(IntN, IntN.isSigned(), rm);
    return Number(AsFloat - RHS.FloatN);
  }

  if (RHS.IsInteger) {
    APFloat AsFloat(FloatN.getSemantics());
    APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
    AsFloat.convertFromAPInt(RHS.IntN, RHS.IntN.isSigned(), rm);
    return Number(FloatN - AsFloat);
  }

  return Number(FloatN - RHS.FloatN);
}

SLEV::Number SLEV::Number::operator*(const Number& RHS) const {

  assert(Defined && RHS.Defined && "Operating on undefined numbers");

  if (IsInteger && RHS.IsInteger) {
    return Number(IntN * RHS.IntN);
  }

  if (IsInteger) {
    APFloat AsFloat(RHS.FloatN.getSemantics());
    APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
    AsFloat.convertFromAPInt(IntN, IntN.isSigned(), rm);
    return Number(AsFloat * RHS.FloatN);
  }

  if (RHS.IsInteger) {
    APFloat AsFloat(FloatN.getSemantics());
    APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
    AsFloat.convertFromAPInt(RHS.IntN, RHS.IntN.isSigned(), rm);
    return Number(FloatN * AsFloat);
  }

  return Number(FloatN * RHS.FloatN);
}

SLEV::Number SLEV::Number::operator/(const Number& RHS) const {

  assert(Defined && RHS.Defined && "Operating on undefined numbers");

  if (IsInteger && RHS.IsInteger) {
    return Number(IntN / RHS.IntN);
  }

  if (IsInteger) {
    APFloat AsFloat(RHS.FloatN.getSemantics());
    APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
    AsFloat.convertFromAPInt(IntN, IntN.isSigned(), rm);
    return Number(AsFloat / RHS.FloatN);
  }

  if (RHS.IsInteger) {
    APFloat AsFloat(FloatN.getSemantics());
    APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
    AsFloat.convertFromAPInt(RHS.IntN, RHS.IntN.isSigned(), rm);
    return Number(FloatN / AsFloat);
  }

  return Number(FloatN / RHS.FloatN);
}

class SLEVConstructor {

private:
  SIMDLaneEvolutionBase& SLEV;

public:

  SLEVConstructor(SIMDLaneEvolutionBase& S) : SLEV(S) {}

  /// Visit Functions
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }
  void visit(AVR* ANode) {}
  void postVisit(AVR* ANode) {}
  void postVisit(AVRExpression* AExpr) { SLEV.construct(AExpr); }
  void visit(AVRValueIR* AValueIR) { SLEV.construct(AValueIR); }
  void visit(AVRPhiIR *APhiIR) { SLEV.construct(APhiIR); }
  void visit(AVRLabelIR *ALabelIR) { SLEV.construct(ALabelIR); }
  void visit(AVRCallIR *ACallIR) { SLEV.construct(ACallIR); }
  void visit(AVRReturnIR *AReturnIR) { SLEV.construct(AReturnIR); }
  void visit(AVRSelect *ASelect) { SLEV.construct(ASelect); }
  void visit(AVRCompareIR *ACompareIR) { SLEV.construct(ACompareIR); }
  void visit(AVRBranchIR *ABranchIR) { SLEV.construct(ABranchIR); }
  void visit(AVRIfIR *AIfIR) { SLEV.construct(AIfIR); }
  void visit(AVRValueHIR* AValueHIR) { SLEV.construct(AValueHIR); }
  void visit(AVRLoopIR *ALoopIR) { SLEV.entering(ALoopIR); }
  void postVisit(AVRLoopIR *ALoopIR) { SLEV.exiting(ALoopIR); }
  void visit(AVRLoopHIR *ALoopHIR) { SLEV.entering(ALoopHIR); }
  void postVisit(AVRLoopHIR *ALoopHIR) { SLEV.exiting(ALoopHIR); }
};

void DDRef2AVR::visit(AVRValueHIR* AValueHIR) {

  RegDDRef * RDDF = AValueHIR->getValue();

  if (AvrDefUseHIR::isDef(AValueHIR)) {

    // This value is a Def - map its underlying RefDDRef to it.
    Map[RDDF] = AValueHIR;
  }
  else {

    // This value is a Use - map all its underlying RegDDRef's blobs to it.
    for (auto I = RDDF->blob_cbegin(), E = RDDF->blob_cend(); I != E; ++I)
      Map[*I] = AValueHIR;
  }
}

SIMDLaneEvolutionBase::SIMDLaneEvolutionBase(char &ID) : FunctionPass(ID) {

  reset();
}

void SIMDLaneEvolutionBase::reset() {

  Affected1.clear();
  Affected2.clear();
  AffectedOld = &Affected1;
  AffectedNew = &Affected2;

  std::set<SLEV*> AllSlevs;

  for (auto It : SLEVs)
    It.second->collectGarbage(AllSlevs);

  for (SLEV* S : AllSlevs)
    delete S;

  SLEVs.clear();
}

SIMDLaneEvolutionBase::InfluenceRegion
SIMDLaneEvolutionBase::calculateInfluenceRegion(AVR* Avr) {

  assert(CFG->isBranchCondition(Avr) && "AVR is not a branch condition");

  AvrBasicBlock* BasicBlock = CFG->getBasicBlock(Avr);
  AvrBasicBlock* PostDom =
    PostDominatorTree->getNode(BasicBlock)->getIDom()->getBlock();

  InfluenceRegion IR(BasicBlock, PostDom);
  SmallVector<AvrBasicBlock*, 3> Worklist;
  Worklist.push_back(BasicBlock);

  while (!Worklist.empty()) {
    AvrBasicBlock* Current = Worklist.back();
    Worklist.pop_back();
    for (AvrBasicBlock* Successor : Current->getSuccessors()) {
      if (Successor == PostDom || IR.InfluencedBasicBlocks.count(Successor))
        continue;
      IR.InfluencedBasicBlocks.insert(Successor);
      Worklist.push_back(Successor);
    }
  }

  return IR;
}

void SIMDLaneEvolutionBase::runOnAvr(AVRGenerateBase* A,
                                     AvrCFGBase* C,
                                     AvrDefUseBase* DU,
                                     IRValuePrinterBase* VP) {

  reset();

  AV = A;
  CFG = C;
  DefUseBase = DU;
  ValuePrinter = VP;

  DEBUG(dbgs() << "SLEV: Analyzing AVR:\n";
        AV->print(dbgs(), 1, PrintNumber);
        dbgs() << "SLEV: "; CFG->print(dbgs());
        dbgs() << "SLEV: "; DefUseBase->print(dbgs()));

  DominatorTree = new AvrDominatorTree(false);
  DominatorTree->recalculate(*CFG);
  PostDominatorTree = new AvrDominatorTree(true);
  PostDominatorTree->recalculate(*CFG);

  DEBUG(dbgs() << "SLEV: Dominator Tree:\n"; DominatorTree->print(dbgs()));
  DEBUG(dbgs() << "SLEV: PostDominator Tree:\n";
        PostDominatorTree->print(dbgs()));

  FirstCalcQueue = new std::vector<SLEV*>();

  SLEVConstructor Constructor(*this);
  AVRVisitor<SLEVConstructor> AVisitor(Constructor);
  AVisitor.forwardVisit(AV->begin(), AV->end(), true, true);

  // Add any reaching SLEVs that were not present during construction due to
  // visit order.
  for (auto& It : UsesPendingDefs) {
    SLEVUse* SU = It.first;
    for (auto& AVRVarIt : It.second) {
      AVR* ReachingDef = AVRVarIt.first;
      const void* IRUse = AVRVarIt.second;
      SU->addReaching(SLEVs[ReachingDef], IRUse);
      DEBUG(formatted_raw_ostream FOS(dbgs());
            FOS << "SLEV: Added pending SLEV:\n";
            FOS << "SLEV: ... Reaching Def: ";
            ReachingDef->shallowPrint(FOS);
            FOS << "\n";
            FOS << "SLEV: ... IR Use: ";
            ValuePrinter->print(FOS, IRUse);
            FOS << "\n";
            FOS << "SLEV: ... Now: ";
            SU->print(FOS, false);
            FOS << "\n");
    }
  }
  UsesPendingDefs.clear();

  // First calculation of all AVR SLEVs (users will be affected).
  for (SLEV* S : *FirstCalcQueue)
    calculate(S);
  delete FirstCalcQueue;
  FirstCalcQueue = nullptr;

  // Run the data-flow analysis to exhaustion.
  while (!AffectedNew->empty()) {

    // Swap between the affected-SLEVs sets.
    std::swap(AffectedNew, AffectedOld);

    // Prepare new affected-SLEVs set to receive affected SLEV.
    AffectedNew->clear();

    for (SLEV* Slev : *AffectedOld) {
      calculate(Slev);
    }
  }

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "SLEV: Done\n");

  delete DominatorTree;
  DominatorTree = nullptr;
  delete PostDominatorTree;
  PostDominatorTree = nullptr;

  AV = nullptr;
  CFG = nullptr;
  DefUseBase = nullptr;
  ValuePrinter = nullptr;
}

template<typename SLEVT, typename... OTHERS>
SLEVT* SIMDLaneEvolutionBase::createBinarySLEV(AVRExpression* AExpr, OTHERS... Tail) {
  assert(AExpr->getNumOperands() == 2 && "Not a binary expression");
  AVR *LHS = AExpr->getOperand(0);
  SLEV *LHSSlev = SLEVs[LHS];
  assert(LHSSlev && "LHS has no SLEV");
  AVR *RHS = AExpr->getOperand(1);
  SLEV *RHSSlev = SLEVs[RHS];
  assert(RHSSlev && "RHS has no SLEV");
  return new SLEVT(*LHSSlev, *RHSSlev, Tail...);
}

SLEVPreserveUniformity*
SIMDLaneEvolutionBase::createPreservingUniformitySLEV(AVRExpression* AExpr) {

  SLEVPreserveUniformity* SPU = new SLEVPreserveUniformity();
  for (unsigned I = 0; I < AExpr->getNumOperands(); ++I) {
    AVR *Op = AExpr->getOperand(0);
    SLEV *OpSlev = SLEVs[Op];
    assert(OpSlev && "Op has no SLEV");
    SPU->addDependency(OpSlev);
  }

  return SPU;
}

void SIMDLaneEvolutionBase::construct(AVRExpression* AExpr) {

  if (AExpr->isLHSExpr()) {

    // AVR LHS EXPR are (ironically) not Defs. We therefore do not
    // construct a SLEV for them - their value's SLEV will be
    // propagated to them later.
    return;
  }

  SLEV* ExprSLEV;
  switch (AExpr->getOperation()) {
  case Instruction::SExt:
  case Instruction::ZExt:
  case Instruction::FPExt:
    assert(SLEVs.count(AExpr->getOperand(0)) && "Operand has no SLEV");
    ExprSLEV = new SLEVIdentity(*SLEVs[AExpr->getOperand(0)]);
    break;
  case Instruction::Add:
  case Instruction::FAdd:
    ExprSLEV = createBinarySLEV<SLEVAdd>(AExpr);
    break;
  case Instruction::Sub:
  case Instruction::FSub:
    ExprSLEV = createBinarySLEV<SLEVSub>(AExpr);
    break;
  case Instruction::Mul:
  case Instruction::FMul:
    ExprSLEV = createBinarySLEV<SLEVMul>(AExpr);
    break;
  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::FDiv:
    ExprSLEV = createBinarySLEV<SLEVDiv>(AExpr);
    break;
  case Instruction::ICmp:
  case Instruction::FCmp:
    ExprSLEV = createBinarySLEV<SLEVCmp>(AExpr);
    break;
  case Instruction::URem:
  case Instruction::SRem:
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor:
  case Instruction::Trunc:
  case Instruction::FPTrunc:
  case Instruction::FPToUI:
  case Instruction::FPToSI:
  case Instruction::UIToFP:
  case Instruction::SIToFP:
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
  case Instruction::BitCast:
    // TODO
    ExprSLEV = createPreservingUniformitySLEV(AExpr);
    break;
  case Instruction::AddrSpaceCast:
    assert(SLEVs.count(AExpr->getOperand(0)) && "Operand has no SLEV");
    ExprSLEV = new SLEVIdentity(*SLEVs[AExpr->getOperand(0)]);
    break;
  case Instruction::GetElementPtr:
    {
      assert(SLEVs.count(AExpr->getOperand(0)) && "Base has no SLEV");
      SLEVAddress* AddressSLEV =
        new SLEVAddress(SLEVs[AExpr->getOperand(0)], 1 /*TODO*/);
      for (unsigned Index = 1; Index < AExpr->getNumOperands(); ++Index) {
        assert(SLEVs.count(AExpr->getOperand(Index)) && "Index has no SLEV");
        AddressSLEV->addIndex(SLEVs[AExpr->getOperand(Index)], 1 /*TODO*/);
      }
      ExprSLEV = AddressSLEV;
    }
    break;
  case Instruction::Load:
  case Instruction::Store:
    // We do not support SLEV memory addresses. Still, if all arguments are
    // UNIFORM then so is the operation and for loads - so is the result.
    ExprSLEV = createPreservingUniformitySLEV(AExpr);
    break;
  case Instruction::Call: // TODO: common logic with AVRCall, for now:
    // Can't assume anything about non-pure functions
    ExprSLEV = createPredefinedSLEV(RANDOM);
    break;
  default:
    llvm_unreachable("unsupported expression kind");
  }

  setSLEV(AExpr, ExprSLEV);
}

void SIMDLaneEvolutionBase::setSLEV(AVR* Avr, SLEV* Slev) {

  Slev->setAVR(Avr);

  if (CFG->isBranchCondition(Avr))
    Slev->setBranchCondition();

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "SLEV: Setting ";
        Slev->print(FOS, false);
        FOS << " for ";
        Avr->shallowPrint(FOS);
        FOS << "\n");

  SLEVs[Avr] = Slev;
}

void SIMDLaneEvolutionBase::calculate(SLEV* Slev) {

  DEBUG(dbgs() << "SLEV: Calculating ";
        Slev->print(dbgs(), false);
        dbgs() << "\n");

  SLEV LastSLEV(*Slev);

  Slev->calculate();

  if (LastSLEV == *Slev) {

    DEBUG(dbgs() << "SLEV: ... No change\n");
    return;
  }

  // This SLEV has changed - insert all its users into the work list.
  DEBUG(dbgs() << "SLEV: ... Now: ";
        Slev->print(dbgs(), true);
        dbgs() << "\n";
        dbgs() << "SLEV: ... Change affects:";
        for (SLEV* User : Slev->Users) {
          dbgs() << " ";
          User->print(dbgs(), true);
        }
        dbgs() << "\n");

  for (SLEV* User : Slev->Users)
    if (!User->isRANDOM())
      AffectedNew->insert(User);

  if (Slev->isDivergingControlFlow() & !Slev->isUniform())
    handleControlDivergence(Slev);
}

void SIMDLaneEvolutionBase::handleControlDivergence(SLEV* Slev) {

  AVR* Avr = Slev->getAVR();
  assert(Avr && "Control-diverging SLEV has no AVR");
  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "SLEV: ... Control-flow is now diverging at: ";
        Avr->shallowPrint(FOS);
        FOS << "\n");

  AvrBasicBlock* ConditionBB = getCFG()->getBasicBlock(Avr);
  InfluenceRegion IR = calculateInfluenceRegion(Avr);

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "SLEV: ...... Influence Region: ";
        IR.print(FOS);
        FOS << "\n");

  // For each Def AVR in the Influence Region, check for Uses tainted by
  // the control-flow divergence introduced by the non-UNIFORM branch condition.

#ifdef HAS_LOOP_INFO
  AVRLoop* DivergingBranchLoop = Avr->getParentLoop(); // TODO getLexicalParentloop?
#endif
  for (AvrBasicBlock* IRBB : IR.getInfluencedBasicBlocks()) {

    bool CheckedBlockForBeingPartiallyKilling = false;
    AvrCFGBase::PathTy PartiallyKillingPath;

    for (AVR* Def : IRBB->getInstructions()) {

      auto DefSlevIt = SLEVs.find(Def);
      if (DefSlevIt != SLEVs.end() && DefSlevIt->second->isRANDOM())
        continue; // Any user already is (or will be soon) RANDOM anyway.

      const auto& Uses = getDefUse()->getUses(Def);
      if (Uses.empty())
        continue; // Not a Def.

      AvrBasicBlock* DefBB = getCFG()->getBasicBlock(Def);
      const auto& DefDom = DominatorTree->getNode(DefBB);

#ifdef HAS_LOOP_INFO
      AVRLoop* DefLoop = Def->getParentLoop();
      if (DefLoop != DivergingBranchLoop)
        continue; // Diverging branch and Def not in the same loop.
#endif

      for (auto& It : Uses) {

        AVR* Use = It.first;

        // Get the SLEV of this Use if there is one.
        auto UseSlevIt = SLEVs.find(Use);
        if (UseSlevIt == SLEVs.end())
          continue; // to next Use.
        SLEV* UseSlev = UseSlevIt->second;

        // No point in tainting already random/tainted Uses.
        if (UseSlev->isRANDOM() || UseSlev->isTainted())
          continue; // to next Use.

        AvrBasicBlock* UseBB = getCFG()->getBasicBlock(Use);

        // Uses in the Def's basic block cannot be tainted by control flow.
        if (UseBB == DefBB)
          continue; // to next Use.

        // Check if Use is tainted by this Def and another Reaching Def.
        if (isUseTaintedByTwoReachingDefs(Avr, Def, Use)) {

          taint(Use, It.second);
          continue; // to next Use.
        }

        // Check if this Def is partially-killing w.r.t this Use.
        const auto& UseDom = DominatorTree->getNode(UseBB);
        bool DefDominatesUse = DominatorTree->dominates(DefDom, UseDom);
        if (!DefDominatesUse) {

          // Use is not shielded from the control divergence by being dominated
          // by Def: check wether the Def itself is partially-killing w.r.t the
          // diverging condition.

          if (!CheckedBlockForBeingPartiallyKilling) {
            findPartiallyKillingPath(ConditionBB, DefBB, PartiallyKillingPath);
            CheckedBlockForBeingPartiallyKilling = true;
          }

          if (!PartiallyKillingPath.empty()) {

            // Def's block is partially-killing: taint Use.
            DEBUG(formatted_raw_ostream FOS(dbgs());
                  FOS << "SLEV: ...... Partially-killing Def detected:\n";
                  FOS << "SLEV: ......... Def: ";
                  Def->shallowPrint(FOS);
                  FOS << "\n";
                  FOS << "SLEV: ......... Use: ";
                  Use->shallowPrint(FOS);
                  FOS << "\n";
                  FOS << "SLEV: ......... Path: ";
                  getCFG()->print(FOS, PartiallyKillingPath);
                  FOS << "\n");

            taint(Use, It.second);
            continue; // to next Use
          }
        }

        if (isUseTaintedByLeakingIterations(Avr, Def, Use)) {

          taint(Use, It.second);
          continue; // to next Use.
        }
      }
    }
  }
}

void SIMDLaneEvolutionBase::taint(AVR* Use,
        const AvrDefUseBase::VarSetTy& Vars) {

  assert(SLEVs.count(Use) && "Use without a SLEV?");
  SLEV* Slev = SLEVs[Use];
  std::set<SLEV*> Affected;

  for (const void* Var : Vars)
    Slev->taint(Var, Affected);

  DEBUG(formatted_raw_ostream FOS(dbgs());
        FOS << "SLEV: ......... Tainted: ";
        Slev->print(FOS, true);
        FOS << "\n";
        FOS << "SLEV: ............ Change affects:";
        for (SLEV* T : Affected) {
          FOS << " ";
          T->print(FOS, true);
        }
        FOS << "\n");

  // Every SLEV actually tainted (unless already RANDOM) needs recalculation.
  for (SLEV* T : Affected) {

    if (!T->isRANDOM())
      AffectedNew->insert(T);
  }
}

void SIMDLaneEvolutionBase::findPartiallyKillingPath(AvrBasicBlock* ConditionBB,
        AvrBasicBlock* DefBB,
        AvrCFGBase::PathTy& Result) {

  // Look for a simple path Successor -> ConditionBB -> DefBB

  for (AvrBasicBlock* Successor : ConditionBB->getSuccessors()) {

    AvrCFGBase::PathTy PartiallyKillingDefSchema;
    PartiallyKillingDefSchema.push_back(Successor);
    PartiallyKillingDefSchema.push_back(nullptr); // allow a gap here
    PartiallyKillingDefSchema.push_back(ConditionBB);
    PartiallyKillingDefSchema.push_back(nullptr); // allow a gap here
    PartiallyKillingDefSchema.push_back(DefBB);

    Result = getCFG()->findSimplePath(PartiallyKillingDefSchema, true);
    if (!Result.empty()) {
      return;
    }
  }
}

bool SIMDLaneEvolutionBase::isUseTaintedByLeakingIterations(AVR* Condition,
                                                            AVR* Def,
                                                            AVR* Use) {

  // Does Def's loop leak iterations onto Use?

  AvrBasicBlock* ConditionBB = getCFG()->getBasicBlock(Condition);
  AvrBasicBlock* DefBB = getCFG()->getBasicBlock(Def);
  AvrBasicBlock* UseBB = getCFG()->getBasicBlock(Use);
  assert(ConditionBB && "Condition not in CFG?");
  assert(DefBB && "Def not in CFG?");
  assert(UseBB && "Use not in CFG?");

#ifdef HAS_LOOP_INFO
  bool UseInDefsLoop = false;
  for (AVRLoop* UseLoop = Use->getParentLoop(); UseLoop != nullptr;
      UseLoop = UseLoop->getParentLoop()) {

      if (UseLoop == DefLoop) {
      UseInDefsLoop = true;
      break;
    }
  }
#else
  bool UseInDefsLoop = false;
#endif

  if (UseInDefsLoop)
    return false; // Use is inside Def's loop

  for (AvrBasicBlock* Successor : ConditionBB->getSuccessors()) {

    AvrCFGBase::PathTy LeakingIterationsSchema;
    LeakingIterationsSchema.push_back(Successor);
    if (DefBB != Successor) {
      LeakingIterationsSchema.push_back(nullptr); // allow a gap here
      LeakingIterationsSchema.push_back(DefBB);
    }
    LeakingIterationsSchema.push_back(nullptr); // allow a gap here
    LeakingIterationsSchema.push_back(ConditionBB);
    LeakingIterationsSchema.push_back(nullptr); // allow a gap here
    LeakingIterationsSchema.push_back(UseBB);

    AvrCFGBase::PathTy Path =
      getCFG()->findSimplePath(LeakingIterationsSchema, true);

    if (!Path.empty()) {

      // Yes. Taint this use.
      DEBUG(formatted_raw_ostream FOS(dbgs());
            FOS << "SLEV: ...... Leaking iterations detected:\n";
            FOS << "SLEV: ......... Def: ";
            Def->shallowPrint(FOS);
            FOS << "\n";
            FOS << "SLEV: ......... Use: ";
            Use->shallowPrint(FOS);
            FOS << "\n";
            FOS << "SLEV: ......... Path: ";
            getCFG()->print(FOS, Path);
            FOS << "\n");
      return true;
    }
  }

  return false;
}

bool SIMDLaneEvolutionBase::isUseTaintedByTwoReachingDefs(AVR* Condition,
                                                          AVR* Def,
                                                          AVR* Use) {

  // Is there a second Def (Def') reaching Use s.t. Def and Def' reach Use
  // by distinct paths (except the diverging condition and Use itself)?

  AvrBasicBlock* ConditionBB = getCFG()->getBasicBlock(Condition);
  AvrBasicBlock* DefBB = getCFG()->getBasicBlock(Def);
  AvrBasicBlock* UseBB = getCFG()->getBasicBlock(Use);
  assert(ConditionBB && "Condition not in CFG?");
  assert(DefBB && "Def not in CFG?");
  assert(UseBB && "Use not in CFG?");

  // Find all paths: Condition -> .. -> Def
  AvrCFGBase::PathTy ConditionDefUseSchema;
  ConditionDefUseSchema.push_back(ConditionBB);
  ConditionDefUseSchema.push_back(nullptr); // allow a gap here
  ConditionDefUseSchema.push_back(DefBB);

  std::set<AvrCFGBase::PathTy> ConditionDefUsePaths =
    getCFG()->findSimplePaths(ConditionDefUseSchema, false, true);

  SmallPtrSet<const AvrBasicBlock*, 3> AllowedCommonBasicBlocksInPath;
  AllowedCommonBasicBlocksInPath.insert(ConditionBB);
  AllowedCommonBasicBlocksInPath.insert(UseBB);

  const auto& UsedVarsMap = getDefUse()->getUses(Def);
  assert(UsedVarsMap.count(Use) && "Def not related to Use?");
  for (const void* Var : UsedVarsMap.find(Use)->second) {

    const AvrSetTy& ReachingDefs = getDefUse()->getReachingDefs(Use, Var);
    for (AVR* ReachingDef : ReachingDefs) {

      if (ReachingDef == Def)
        continue;

      AvrBasicBlock* ReachingDefBB = getCFG()->getBasicBlock(ReachingDef);

      // Find all paths: Condition -> .. -> Reaching Def -> .. -> Use
      AvrCFGBase::PathTy C_RD_USchema;
      C_RD_USchema.push_back(ConditionBB);
      C_RD_USchema.push_back(nullptr); // allow a gap
      C_RD_USchema.push_back(ReachingDefBB);
      C_RD_USchema.push_back(nullptr); // allow a gap
      C_RD_USchema.push_back(UseBB);
      std::set<AvrCFGBase::PathTy> C_RD_UPaths =
        getCFG()->findSimplePaths(C_RD_USchema, false, true);

      // Check if the paths are distinct excluding Condition and Use.
      DistinctPathsTy C_RD_UConflict =
        findDistinctPaths(ConditionDefUsePaths,
                          C_RD_UPaths,
                          AllowedCommonBasicBlocksInPath);

      if (std::get<0>(C_RD_UConflict)) {

        // Yes. Taint this Use.
        DEBUG(formatted_raw_ostream FOS(dbgs());
              FOS << "SLEV: ...... Conflicting Defs detected:\n";
              FOS << "SLEV: ......... Def: ";
              Def->shallowPrint(FOS);
              FOS << "\n";
              FOS << "SLEV: ......... 2nd Def: ";
              ReachingDef->shallowPrint(FOS);
              FOS << "\n";
              FOS << "SLEV: ......... Use: ";
              Use->shallowPrint(FOS);
              FOS << "\n";
              FOS << "SLEV: ......... Path: ";
              getCFG()->print(FOS, *std::get<1>(C_RD_UConflict));
              FOS << "\n";
              FOS << "SLEV: ......... 2nd Path: ";
              getCFG()->print(FOS, *std::get<2>(C_RD_UConflict));
              FOS << "\n");
        return true;
      }

      // Find all paths: Reaching Def -> .. -> Condition -> .. -> Use
      AvrCFGBase::PathTy RD_C_USchema;
      RD_C_USchema.push_back(ConditionBB);
      RD_C_USchema.push_back(nullptr); // allow a gap
      RD_C_USchema.push_back(ReachingDefBB);
      RD_C_USchema.push_back(nullptr); // allow a gap
      RD_C_USchema.push_back(UseBB);
      std::set<AvrCFGBase::PathTy> RD_C_UPaths =
        getCFG()->findSimplePaths(RD_C_USchema, false, true);

      // Check if the paths are distinct excluding Condition and Use.
      DistinctPathsTy RD_C_UConflict =
        findDistinctPaths(ConditionDefUsePaths,
                          RD_C_UPaths,
                          AllowedCommonBasicBlocksInPath);

      if (std::get<0>(RD_C_UConflict)) {

        // Yes. Taint this Use.
        DEBUG(formatted_raw_ostream FOS(dbgs());
              FOS << "SLEV: ...... Conflicting Defs detected:\n";
              FOS << "SLEV: ......... Def: ";
              Def->shallowPrint(FOS);
              FOS << "\n";
              FOS << "SLEV: ......... 2nd Def: ";
              Def->shallowPrint(FOS);
              FOS << "\n";
              FOS << "SLEV: ......... Use: ";
              Use->shallowPrint(FOS);
              FOS << "\n";
              FOS << "SLEV: ......... Path: ";
              getCFG()->print(FOS, *std::get<1>(RD_C_UConflict));
              FOS << "\n";
              FOS << "SLEV: ......... 2nd Path: ";
              getCFG()->print(FOS, *std::get<2>(RD_C_UConflict));
              FOS << "\n");

        return true;
      }
    }
  }

  return false;
}

SIMDLaneEvolutionBase::DistinctPathsTy SIMDLaneEvolutionBase::findDistinctPaths(
    const std::set<AvrCFGBase::PathTy>& Lefts,
    const std::set<AvrCFGBase::PathTy>& Rights,
    const SmallPtrSet<const AvrBasicBlock*, 3>& Except) const {

  for (const AvrCFGBase::PathTy& L : Lefts) {

    SmallPtrSet<const AvrBasicBlock*, 32> Ls;
    for (const AvrBasicBlock* BB : L)
      if (!Except.count(BB))
        Ls.insert(BB);

    for (const AvrCFGBase::PathTy& R : Rights) {

      bool FoundCommonNode = false;
      for (const AvrBasicBlock* BB : R)
        if (Ls.count(BB)) {
          FoundCommonNode = true;
          break;
        }

      if (!FoundCommonNode)
        return std::make_tuple(true, &L, &R);
    }
  }

  return std::make_tuple(false, nullptr, nullptr);
}

void SIMDLaneEvolutionBase::print(raw_ostream &OS, const Module*) const {

  formatted_raw_ostream FOS(OS);

  FOS << "AVR SIMD Lane Evolution Analysis:\n";

  for (auto& Entry : SLEVs) {
    Entry.first->shallowPrint(FOS);
    FOS << " ===> ";
    Entry.second->print(FOS, true);
    FOS << "\n";
  }
}

SIMDLaneEvolution::SIMDLaneEvolution() : SIMDLaneEvolutionBase(ID) {
  llvm::initializeSIMDLaneEvolutionPass(*PassRegistry::getPassRegistry());
}

SIMDLaneEvolution::~SIMDLaneEvolution() {}

SLEV* SIMDLaneEvolution::constructValueSLEV(const Value* Val,
                                            const ReachingAvrsTy& ReachingVars) {

  if (const ConstantInt* Const = dyn_cast<ConstantInt>(Val)) {
    return createPredefinedSLEV(SLEV(CONSTANT, APSInt(Const->getValue())));
  }

  if (const ConstantFP* Const = dyn_cast<ConstantFP>(Val)) {
    return createPredefinedSLEV(SLEV(CONSTANT, Const->getValueAPF()));
  }

  const auto& ReachingDefs = ReachingVars.find(Val);
  if (ReachingDefs != ReachingVars.end()) {
    SLEVUse* SU = new SLEVUse();
    for (AVR* ReachingDef : ReachingDefs->second)
      addReaching(SU, ReachingDef, Val);
    return SU;
  }

  return createPredefinedSLEV(UNIFORM);
}

void SIMDLaneEvolution::construct(AVRValueIR* AValueIR) {

  const Value* Val = AValueIR->getLLVMValue();

  if (InductionVariableDefs.count(AValueIR)) {

    // This is a value feeding the phi which is the canonical induction variable
    // of the candidate loop: we inject the SIMD stride into the loop by
    // predefining the incoming constant as STRIDED<1>.
    setSLEV(AValueIR, createPredefinedSLEV(SLEV(STRIDED, SLEV::One)));
    return;
  }

  if (isDef(AValueIR)) {

    // LHS values are (ironically) not Defs - the RHS expression is. They
    // therefore share its SLEV value (will be propagated later).
    return;
  }

  if (isa<AVRPhiIR>(AValueIR->getParent()) && !AValueIR->isConstant()) {

    // Incoming values of phi nodes which are not Defs (i.e. are constants) do
    // not get their own SLEVs: their reaching Defs propagate to the AVRPhi
    // which blends them together, so such a SLEV value would have no use and
    // computing it is a waste of compile-time (note that being "partial blends"
    // of the Defs reaching the AVRPhi means they may be tainted by single-Def
    // control-flow divergence effects (while the AVRPhi would be tained by the
    // cheaper two-reaching-Defs logic).
    return;
  }

  const auto& ReachingVars = getDefUse()->getReachingDefs(AValueIR);

  setSLEV(AValueIR, constructValueSLEV(Val, ReachingVars));
}

void SIMDLaneEvolution::construct(AVRLabelIR *ALabelIR) {

  // TODO: we probably don't need this.
  setSLEV(ALabelIR, createPredefinedSLEV(UNIFORM));
}

void SIMDLaneEvolution::construct(AVRPhiIR *APhiIR) {

  assert(isa<PHINode>(APhiIR->getLLVMInstruction()) && "Not a PHINode?");
  PHINode* Phi = cast<PHINode>(APhiIR->getLLVMInstruction());
  SLEVUse* SU = new SLEVUse();

  for (const auto& Use : getDefUse()->getReachingDefs(APhiIR))
    for (AVR* ReachingDef : Use.second)
      addReaching(SU, ReachingDef, Phi);

  setSLEV(APhiIR, SU);
}

void SIMDLaneEvolution::construct(AVRCallIR *ACallIR) {

  SLEV* S;

  assert(isa<CallInst>(ACallIR->getLLVMInstruction()) && "Not a CallInst?");
  const CallInst* Call = cast<CallInst>(ACallIR->getLLVMInstruction());

  if (Call->doesNotAccessMemory() ||
      (Call->onlyAccessesArgMemory() && Call->onlyReadsMemory())) {

    // Functions that have no side effects are UNIFORM iff all their arguments
    // are.
    const auto& ReachingVars = getDefUse()->getReachingDefs(ACallIR);
    SLEVPreserveUniformity *SPU = new SLEVPreserveUniformity();
    for (Value* Arg : Call->arg_operands())
      SPU->addDependency(constructValueSLEV(Arg, ReachingVars));
    S = SPU;
  }
  else {

    // Can't assume anything about non-pure functions
    S = createPredefinedSLEV(RANDOM);
  }

  Value2SLEV[Call] = S;
  setSLEV(ACallIR, S);
}

void SIMDLaneEvolution::construct(AVRReturnIR *AReturnIR) {

  const auto& ReachingVars = getDefUse()->getReachingDefs(AReturnIR);
  const ReturnInst *Return = cast<ReturnInst>(AReturnIR->getLLVMInstruction());
  Value *ReturnValue = Return->getReturnValue();

  if (ReturnValue)
    setSLEV(AReturnIR, constructValueSLEV(ReturnValue, ReachingVars));
}

void SIMDLaneEvolution::construct(AVRSelect *ASelect) {
}

void SIMDLaneEvolution::construct(AVRCompareIR *ACompareIR) {

  if (ACompareIR == LatchCondition) {

    // This is the comparison controlling the latch of the vector loop
    // candidate. Since it compares a STRIDED slev (the induction variable),
    // the usual slev-computing construction will cause the vector loop
    // candidate to diverge, which is incorrect (the SIMD lanes which may
    // diverge are only defined within the loop; the loop itself is UNIFORM just
    // like any part of the program outside it). We therefore create a
    // pre-defined UNIFORM slev for this AVR.

    setSLEV(ACompareIR, createPredefinedSLEV(UNIFORM));
    return;
  }

  const auto& ReachingVars = getDefUse()->getReachingDefs(ACompareIR);
  Instruction* Inst = ACompareIR->getLLVMInstruction();

  Value* LHS = Inst->getOperand(0);
  SLEV* LHSSlev = constructValueSLEV(LHS, ReachingVars);
  Value* RHS = Inst->getOperand(1);
  SLEV* RHSSlev = constructValueSLEV(RHS, ReachingVars);

  setSLEV(ACompareIR, new SLEVCmp(*LHSSlev, *RHSSlev));
}

void SIMDLaneEvolution::construct(AVRBranchIR *ABranchIR) {

  const BranchInst *Branch = cast<BranchInst>(ABranchIR->getLLVMInstruction());
  if (!Branch->isConditional())
    return;

  const auto& ReachingVars = getDefUse()->getReachingDefs(ABranchIR);
  Value *Condition = Branch->getCondition();

  setSLEV(ABranchIR, constructValueSLEV(Condition, ReachingVars));
}

void SIMDLaneEvolution::construct(AVRIfIR *AIfIR) {

  AVR* Condition = AIfIR->getCondition();

  assert(SLEVs.count(Condition) && "Expected condition to have a SLEV by now");

  setSLEV(AIfIR, new SLEVIdentity(*SLEVs[Condition]));
}

void SIMDLaneEvolution::entering(AVRLoopIR *ALoopIR) {

  // TODO: remove this when there is actually just one vector candidate.
  if (VectorCandidate)
    return;

  if (!ALoopIR->isVectorCandidate())
    return;

  assert(!VectorCandidate && "Nested vector candidates");

  VectorCandidate = ALoopIR;

  const Loop *IRLoop = ALoopIR->getLoop();
  assert(IRLoop && "Missing underlying loop");

  PHINode *IndVar = IRLoop->getCanonicalInductionVariable();
  assert(IndVar && "Underlying loop has no canonical induction variable");

  const IR2AVRVisitor& IR2AVR = DefUse->getIR2AVR();
  AVRPhi* AIndVar = cast<AVRPhi>(IR2AVR.getDefAVR(IndVar));

  for (auto& Incoming : AIndVar->getIncomingValues()) {

    AVRValue* IncomingValue = Incoming.first;
    if (IncomingValue->isConstant())
      InductionVariableDefs.insert(IncomingValue);
  }

  assert(InductionVariableDefs.size() &&
         "Candidate loop without a canonical induction variable");

  // Since this COMPARE is fed by the STRIDED induction variable
  // it gets a pre-defined UNIFORM slev rather than the usual computed slev
  // constructed for

  AVR *LastChild = ALoopIR->getLastChild();
  assert(LastChild && "Candidate loop is empty");
  assert(isa<AVRBranch>(LastChild) && "Loop does not terminate with a branch");

  AVRBranch *Latch = cast<AVRBranch>(LastChild);
  assert(Latch->isConditional() &&
         "Loop terminates with an unconditional branch");

  AVR *Condition = Latch->getCondition();
  assert(isa<AVRCompare>(Condition) && "Latch condition is not an AVRCompare");
  LatchCondition = cast<AVRCompare>(Condition);
}

void SIMDLaneEvolution::exiting(AVRLoopIR *ALoopIR) {

  if (ALoopIR == VectorCandidate) {
    VectorCandidate = nullptr;
    InductionVariableDefs.clear();
    LatchCondition = nullptr;
  }
}

bool SIMDLaneEvolution::runOnFunction(Function &F) {

  DefUse = nullptr;

  VectorCandidate = nullptr;
  InductionVariableDefs.clear();
  LatchCondition = nullptr;

  AVRGenerate* AV = &getAnalysis<AVRGenerate>();

  if (AV->isAbstractLayerEmpty()) {
    return false;
  }

  DEBUG(AV->dump(PrintNumber));

  AvrCFGBase* CFG = &getAnalysis<AvrCFG>();
  DefUse = &getAnalysis<AvrDefUse>();
  IRValuePrinter VP;

  runOnAvr(AV, CFG, DefUse, &VP);

  return false;
}

INITIALIZE_PASS_BEGIN(SIMDLaneEvolutionHIR, "slev-hir",
                      "VPO SIMD Lane Evolution Analysis for HIR",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(AvrDefUseHIR)
INITIALIZE_PASS_DEPENDENCY(AvrCFGHIR)
INITIALIZE_PASS_DEPENDENCY(AVRGenerateHIR)
INITIALIZE_PASS_END(SIMDLaneEvolutionHIR, "slev-hir",
                    "VPO SIMD Lane Evolution Analysis for HIR",
                    false, true)

char SIMDLaneEvolutionHIR::ID = 0;

SIMDLaneEvolutionHIR::SIMDLaneEvolutionHIR() : SIMDLaneEvolutionBase(ID) {
  llvm::initializeSIMDLaneEvolutionHIRPass(*PassRegistry::getPassRegistry());
}

SIMDLaneEvolutionHIR::~SIMDLaneEvolutionHIR() {}

bool SIMDLaneEvolutionHIR::runOnFunction(Function &F) {

  DefUseHIR = nullptr;

  AVRGenerateBase* AV = &getAnalysis<AVRGenerateHIR>();

  if (AV->isAbstractLayerEmpty()) {
    return false;
  }

  AvrCFGBase* CFG = &getAnalysis<AvrCFGHIR>();
  DefUseHIR = &getAnalysis<AvrDefUseHIR>();
  IRValuePrinterHIR VP;

  runOnAvr(AV, CFG, DefUseHIR, &VP);

  return false;
}

/// \brief Visitor class for SCEV expressions. Used for constructing SLEVs for
/// nested blobs. Such blobs are opaque to the canon-expr, but SLEV needs to take
/// into account the operations in the expression so it must dig into the SCEV
/// expression.
class NestedBLOBSLEVConstructor {

private:

  SIMDLaneEvolutionHIR& SLEVHIR;
  AVRValueHIR* AValueHIR;

  SLEV* GeneratedSLEV;

public:

  NestedBLOBSLEVConstructor(SIMDLaneEvolutionHIR& SH,
                        AVRValueHIR* AVH) : SLEVHIR(SH), AValueHIR(AVH) {

    GeneratedSLEV = nullptr;
  }

  SLEV* getGeneratedSLEV() { return GeneratedSLEV; }

  void constructBlobSLEV(const SCEV *SC) {

    unsigned BlobIndex = BlobUtils::findBlob(SC);
    assert(BlobIndex != InvalidBlobIndex && "SCEV is not a Blob");

    GeneratedSLEV = SLEVHIR.constructSLEV(AValueHIR, BlobIndex);
    assert(GeneratedSLEV && "Failed to get a SLEV for Blob");
  }

  template<typename T> void constructUnarySLEV(const SCEV *SC) {

    NestedBLOBSLEVConstructor OperandConstructor(SLEVHIR, AValueHIR);
    SCEVTraversal<NestedBLOBSLEVConstructor> Visitor(OperandConstructor);

    const SCEV* Operand = cast<SCEVCastExpr>(SC)->getOperand();
    Visitor.visitAll(Operand);

    GeneratedSLEV = new T(*OperandConstructor.GeneratedSLEV);
  }

  template<typename T> void constructBinarySLEV(const SCEV *SC) {

    SmallVector<SLEV*, 2> OperandSLEVs;
    for (const auto *Op : cast<SCEVNAryExpr>(SC)->operands()) {

      NestedBLOBSLEVConstructor OperandConstructor(SLEVHIR, AValueHIR);
      SCEVTraversal<NestedBLOBSLEVConstructor> Visitor(OperandConstructor);
      Visitor.visitAll(Op);
      OperandSLEVs.push_back(OperandConstructor.GeneratedSLEV);
    }

    assert(OperandSLEVs.size() == 2 && "SCEV type is not a binary operation");
    GeneratedSLEV = new T(*OperandSLEVs[0], *OperandSLEVs[1]);
  }

  void constructPSUSLEV(const SCEV* SC) {

    SLEVPreserveUniformity* PSU = new SLEVPreserveUniformity();
    for (const auto *Op : cast<SCEVNAryExpr>(SC)->operands()) {

      NestedBLOBSLEVConstructor OperandConstructor(SLEVHIR, AValueHIR);
      SCEVTraversal<NestedBLOBSLEVConstructor> Visitor(OperandConstructor);
      Visitor.visitAll(Op);
      PSU->addDependency(OperandConstructor.GeneratedSLEV);
    }

    GeneratedSLEV = PSU;
  }

  bool follow(const SCEV *SC) {

    switch (SC->getSCEVType()) {
    case scConstant:
    case scUnknown:
      constructBlobSLEV(SC);
      break;
    case scTruncate:
    case scZeroExtend:
    case scSignExtend:
      constructUnarySLEV<SLEVIdentity>(SC);
      break;
    case scAddExpr:
      constructBinarySLEV<SLEVAdd>(SC);
      break;
    case scMulExpr:
      constructBinarySLEV<SLEVMul>(SC);
      break;
    case scUDivExpr:
      constructBinarySLEV<SLEVDiv>(SC);
      break;
    case scSMaxExpr:
    case scUMaxExpr:
      constructPSUSLEV(SC);
      break;
    case scAddRecExpr:
      llvm_unreachable("Expected add-recs to be broken by canon-expr");
    case scCouldNotCompute:
      llvm_unreachable("Attempt to use a SCEVCouldNotCompute object!");
    default:
      llvm_unreachable("Unknown SCEV kind!");
    }

    return !isDone();
  }

  bool isDone() const { return true; }
};

SLEV* SIMDLaneEvolutionHIR::constructSLEV(AVRValueHIR* AValueHIR,
                                          unsigned BlobIndex) {

  BlobTy Blob = BlobUtils::getBlob(BlobIndex);
  int64_t ConstInt;

  if (BlobUtils::isConstantIntBlob(Blob, &ConstInt))
    return createPredefinedSLEV(SLEV(CONSTANT, toAPSInt(ConstInt)));
  
  if (BlobUtils::isConstantFPBlob(Blob)) {

    const SCEVUnknown* UnknownSCEV = dyn_cast<SCEVUnknown>(Blob);
    ConstantFP* ConstFP = cast<ConstantFP>(UnknownSCEV->getValue());
    return createPredefinedSLEV(SLEV(CONSTANT, ConstFP->getValueAPF()));
  }

  if (BlobUtils::isNestedBlob(Blob)) {

    // This is a SCEV expression kept as-is during canon-expr construction (i.e.
    // has an internal structure opaque to the canon-expr).
    NestedBLOBSLEVConstructor NBSC(*this, AValueHIR);
    SCEVTraversal<NestedBLOBSLEVConstructor> Visitor(NBSC);
    Visitor.visitAll(Blob);
    return NBSC.getGeneratedSLEV();
  }

  DDRef* DDR = nullptr;
  RegDDRef* RDDF = AValueHIR->getValue();
  if (RDDF->isSelfBlob()) {

    DDR = RDDF;
  }
  else {

    for (RegDDRef::const_blob_iterator It = RDDF->blob_cbegin(),
           E = RDDF->blob_cend(); It != E; ++It) {

      if ((*It)->getBlobIndex() == BlobIndex) {
        DDR = *It;
        break;
      }
    }
  }
  assert(DDR && "Blob has no BlobDDRef");
  const AvrSetTy& ReachingDefs = getDefUse()->getReachingDefs(AValueHIR, DDR);
  if (ReachingDefs.empty()) {

    // No reaching Defs means this symbase is defined outside the WRN, which
    // means it is UNIFORM within the WRN.
    return createPredefinedSLEV(UNIFORM);
  }
  else {

    // At least one Def exists.
    SLEVUse* SU = new SLEVUse();
    for (AVR* ReachingDef : ReachingDefs)
      addReaching(SU, ReachingDef, DDR);
    return SU;
  }
}

SLEV* SIMDLaneEvolutionHIR::constructSLEV(AVRValueHIR* AValueHIR,
      CanonExpr& CE,
      unsigned VectorizedDim) {

  SLEV* SLEVTree = nullptr;

  // build SLEVs for IV coefficients.

  for (unsigned Level = 1; Level < MaxLoopNestLevel; ++Level) {

    if (!CE.hasIV(Level))
      continue;

    SLEV* IVSlev;
    if (Level == VectorizedDim)
      IVSlev = createPredefinedSLEV(SLEV(STRIDED, SLEV::One));
    else
      IVSlev = createPredefinedSLEV(UNIFORM);

    unsigned BlobIndex;
    int64_t Coeff;
    CE.getIVCoeff(Level, &BlobIndex, &Coeff);

    if (BlobIndex != InvalidBlobIndex) {

      SLEV* BlobSlev = constructSLEV(AValueHIR, BlobIndex);
      IVSlev = new SLEVMul(*BlobSlev, *IVSlev);
    }

    if (Coeff != 1) {

      SLEV* CoeffSlev = createPredefinedSLEV(SLEV(CONSTANT, toAPSInt(Coeff)));
      IVSlev = new SLEVMul(*CoeffSlev, *IVSlev);
    }

    if (SLEVTree == nullptr)
      SLEVTree = IVSlev;
    else {
      SLEVTree = new SLEVAdd(*SLEVTree, *IVSlev);
    }
  }

  // Build SLEVs for Blob coefficients.

  for (auto It = CE.blob_begin(), E = CE.blob_end(); It != E; ++It) {

    unsigned BlobIndex = CE.getBlobIndex(It);
    SLEV* BlobSlev = constructSLEV(AValueHIR, BlobIndex);

    int64_t Coeff = CE.getBlobCoeff(It);
    if (Coeff != 1) {

      SLEV* CoeffSlev = createPredefinedSLEV(SLEV(CONSTANT, toAPSInt(Coeff)));
      BlobSlev = new SLEVMul(*CoeffSlev, *BlobSlev);
    }

    if (SLEVTree == nullptr)
      SLEVTree = BlobSlev;
    else {
      SLEVTree = new SLEVAdd(*SLEVTree, *BlobSlev);
    }
  }

  // Build a SLEV for the constant additive. We can skip a zero constant
  // additive unless the SLEVTree is empty (which means the constant additive is
  // the entire CE).

  int64_t ConstantAdditive = CE.getConstant();
  if (ConstantAdditive != 0 || SLEVTree == nullptr) {
    SLEV* ConstantAdditiveSlev =
      createPredefinedSLEV(SLEV(CONSTANT, toAPSInt(ConstantAdditive)));
    if (SLEVTree == nullptr)
      SLEVTree = ConstantAdditiveSlev;
    else
      SLEVTree = new SLEVAdd(*SLEVTree, *ConstantAdditiveSlev);
  }

  // Build a SLEV for the denominator.

  int64_t Denominator = CE.getDenominator();
  if (Denominator != 1) {
    SLEV* DenominatorSlev =
      createPredefinedSLEV(SLEV(CONSTANT, toAPSInt(Denominator)));
    SLEVTree = new SLEVDiv(*SLEVTree, *DenominatorSlev);
  }

  assert(SLEVTree && "NO Slev constructed for canon-expr");
  return SLEVTree;
}

void SIMDLaneEvolutionHIR::construct(AVRValueHIR* AValueHIR) {

  RegDDRef * RDDF = AValueHIR->getValue();
  unsigned VectorizedDim = 1; // TODO - current: outermost

  if (DefUseHIR->isDef(AValueHIR)) {

    // AVRValueHIRs are (ironically) not Defs if their RegDDRef is an HIR Def
    // (i.e. if they are scalar lvalues). The actual Def is the RHS
    // AVRExpression. We therefore do not construct a SLEV for them - the SLEV
    // of the RHS will be propagated to them later.
    return;
  }

  // This AVR-VALUE-HIR is a computation, and as such is a Def and a Use and
  // should have a SLEV tracking its computation.

  if (RDDF->isTerminalRef()) {

    CanonExpr* CE = RDDF->getSingleCanonExpr();
    setSLEV(AValueHIR, constructSLEV(AValueHIR, *CE, VectorizedDim));
  }
  else {

    // This is a memory access (either a[i] or *p) - we construct a SLEV for
    // the address it represents (the memory operation itself is represented
    // explicitly in AVR (e.g. EXPR(load (VALUE(RegDDRef)))).

    CanonExpr *BaseCE = RDDF->getBaseCE();
    assert(BaseCE && "Expected memref to have a base");

    SLEV* BaseSLEV = constructSLEV(AValueHIR, *BaseCE, VectorizedDim);
    unsigned BaseSize = 100; // TODO
    SLEVAddress* AddressSlev = new SLEVAddress(BaseSLEV, BaseSize);
    for (auto It = RDDF->canon_begin(), E = RDDF->canon_end(); It != E; ++It) {
      CanonExpr* DimCE = *It;
      SLEV* IndexSLEV = constructSLEV(AValueHIR, *DimCE, VectorizedDim);
      unsigned IndexSize = 100; // TODO
      AddressSlev->addIndex(IndexSLEV, IndexSize);
    }

    setSLEV(AValueHIR, AddressSlev);
  }
}

void SIMDLaneEvolutionHIR::entering(AVRLoopHIR *ALoopHIR) {
}

void SIMDLaneEvolutionHIR::exiting(AVRLoopHIR *ALoopHIR) {
}

FunctionPass *llvm::createSIMDLaneEvolutionPass() {
  return new SIMDLaneEvolution();
}

FunctionPass *llvm::createSIMDLaneEvolutionHIRPass() {
  return new SIMDLaneEvolutionHIR();
}
