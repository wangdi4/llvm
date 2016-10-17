//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPODefUse.cpp -- Implements the AVR-level Def-Use information.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/Passes.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPODefUse.h"
#include "llvm/IR/Intel_LoopIR/BlobDDRef.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassSupport.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "vpo-def-use"

using namespace llvm;
using namespace vpo;

void IR2AVRVisitor::print(raw_ostream &OS) const {

  static std::string Indent(TabLength, ' ');
  formatted_raw_ostream FOS(dbgs());

  FOS << "IR2AVR Mapping:\n";
  FOS << Indent << "DDRef  --->  AVR:\n";
  for (const auto &It : DDRef2AVR) {
    FOS << Indent << Indent;
    It.first->print(FOS, true);
    FOS << "  --->  ";
    It.second->shallowPrint(FOS);
    FOS << "\n";
  }

  FOS << Indent << "Value  --->  Def:\n";
  for (const auto &It : Value2DefAVR) {
    FOS << Indent << Indent << "" << *(It.first);
    FOS << "  --->  ";
    It.second->shallowPrint(FOS);
    FOS << "\n";
  }

  FOS << Indent << "Value  --->  Uses:\n";
  for (const auto &It : Value2UsesAVR) {
    FOS << Indent << Indent << "" << *(It.first);
    FOS << "  --->\n";
    for (const AVR *Use : It.second) {
      FOS << Indent << Indent << Indent;
      Use->shallowPrint(FOS);
    }
    FOS << "\n";
  }
}

void IR2AVRVisitor::visit(AVRValueHIR *AValueHIR) {

  // TODO: AVRValueHIR that represents an IV does't have a DDRef
  if (DDRef *DDR = AValueHIR->getValue()) {
    // Register this value as the AVR holding this DDRef.
    DDRef2AVR[DDR] = AValueHIR;

    if (RegDDRef *RDDR = dyn_cast<RegDDRef>(DDR)) {
      // Register this value as the AVR holding any blob used by its RegDDRef.
      for (auto I = RDDR->blob_cbegin(), E = RDDR->blob_cend(); I != E; ++I)
        DDRef2AVR[*I] = AValueHIR;
    }
  } else if (AValueHIR->isIVValue()) {
    DEBUG(dbgs() << "Warning: Ignoring IV (AVRValueHIR):");
    DEBUG(AValueHIR->dump());
  } else {
    llvm_unreachable("AVRValueHIR doesn't have DDRef or IVValueInfo");
  }
}

void IR2AVRVisitor::visit(AVRValueIR *AValueIR) {

  if (AValueIR->isConstant())
    return;

  const Value *Val = AValueIR->getLLVMValue();

  if (Val == AValueIR->getLLVMInstruction()) {

    // Register this value as the AVR holding this DDRef.
    Value2DefAVR[Val] = AValueIR;
  } else {

    // Register this AVR a user of this value.
    Value2UsesAVR[Val].insert(AValueIR);
  }
}

void IR2AVRVisitor::visit(AVRPhiIR *APhiIR) {

  registerDefAndUses(APhiIR->getLLVMInstruction(), APhiIR);
}

void IR2AVRVisitor::visit(AVRCallIR *ACallIR) {

  const CallInst *Call = cast<CallInst>(ACallIR->getLLVMInstruction());
  FunctionType *CallType = Call->getFunctionType();

  if (!CallType->getReturnType()->isVoidTy())
    registerDef(Call, ACallIR);

  registerUses(Call, ACallIR);
}

void IR2AVRVisitor::visit(AVRReturnIR *AReturnIR) {

  registerUses(AReturnIR->getLLVMInstruction(), AReturnIR);
}

void IR2AVRVisitor::visit(AVRSelectIR *ASelectIR) {

  registerDefAndUses(ASelectIR->getLLVMInstruction(), ASelectIR);
}

void IR2AVRVisitor::visit(AVRCompareIR *ACompareIR) {

  registerDefAndUses(ACompareIR->getLLVMInstruction(), ACompareIR);
}

void IR2AVRVisitor::visit(AVRBranchIR *ABranchIR) {

  registerUses(ABranchIR->getLLVMInstruction(), ABranchIR);
}

void IR2AVRVisitor::visit(AVRIfIR *AIfIR) {

  // In LLVM-IR, an IF replaces a BRANCH and as such is a Use of the COMPARE
  // feeding it (but unlike BRANCHs it does not posses an underlying LLVM
  // Instruction that is a Use of the IF's condition).
  assert(isa<AVRCompareIR>(AIfIR->getCondition()) &&
         "Expected condition to be a COMPARE");
  AVRCompareIR *Condition = cast<AVRCompareIR>(AIfIR->getCondition());
  registerUser(Condition->getLLVMInstruction(), AIfIR);
}

AvrDefUse::AvrDefUse() : AvrDefUseBase(ID) {}

AvrDefUse::~AvrDefUse() {}

void AvrDefUseBase::reset() {

  DefUses.clear();
  ReachingDefs.clear();
}

void AvrDefUseBase::print(raw_ostream &OS, const Module *M) const {

  static std::string Indent(TabLength, ' ');
  formatted_raw_ostream FOS(OS);

  FOS << "AVR Def-Use Information:\n" << Indent << "Def -> Use -> Var:\n";

  for (auto &It : DefUses) {
    FOS << Indent << Indent;
    It.first->shallowPrint(FOS);
    FOS << "\n";
    for (auto &VarUses : It.second) {
      FOS << Indent << Indent << Indent;
      VarUses.first->shallowPrint(FOS);
      FOS << ", via:\n";
      for (const void *Use : VarUses.second) {
        printVar(Use, FOS, 4);
        FOS << "\n";
      }
    }
  }
  FOS << Indent << "Reaching Defs:\n";
  for (auto &It : ReachingDefs) {
    FOS << Indent << Indent;
    It.first->shallowPrint(FOS);
    FOS << "\n";
    for (auto &ReachingVars : It.second) {
      FOS << Indent << Indent << Indent << "via: ";
      printVar(ReachingVars.first, FOS, 0);
      FOS << "\n";
      for (AVR *ReachingDef : ReachingVars.second) {
        FOS << Indent << Indent << Indent << Indent;
        ReachingDef->shallowPrint(FOS);
        FOS << "\n";
      }
    }
  }
}

AvrDefUseBase::AvrDefUseBase(char &ID) : FunctionPass(ID) { reset(); }

INITIALIZE_PASS_BEGIN(AvrDefUse, "avr-def-use", "VPO AVR Def-Use Analysis",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(AVRGenerate)
INITIALIZE_PASS_END(AvrDefUse, "avr-def-use", "VPO AVR Def-Use", false, true)

char AvrDefUse::ID = 0;

void AvrDefUse::registerUsers(AVR *Def, AvrUsedVarsMapTy &UVs, const Value *Val,
                              VisitedPhisTy &VisitedPhis) {

  for (AVR *UsingAVR : IR2AVR.getUsingAVRs(Val)) {

    AVRPhiIR *APhiIR = dyn_cast<AVRPhiIR>(UsingAVR);
    VarSetTy &Vs = UVs[UsingAVR];

    // AVRPhis use their own phi instruction as the underlying variable since
    // all their arguments provide Defs to the phi itself. All other AVR nodes
    // use the given value.
    const Value *UnderlyingVariable =
        APhiIR ? APhiIR->getLLVMInstruction() : Val;

    // Register Use
    Vs.insert(UnderlyingVariable);

    // Register Reaching Def.
    ReachingDefs[UsingAVR][UnderlyingVariable].insert(Def);

    // If this Use happens to be an incoming value of an AVRPhi, register the
    // users of the IR Phi as users of this Def as well (rather than users of
    // the phi node, which we do not consider a Def).
    // Two possible Uses here: incoming constants are used directly by the
    // AVRPhi, incoming non-constants used by the icoming AVRValue.
    if (!APhiIR) {

      AVR *Parent = UsingAVR->getParent();
      if (Parent)
        APhiIR = dyn_cast<AVRPhiIR>(Parent);
    }
    if (APhiIR && !VisitedPhis.count(APhiIR)) {

      VisitedPhis.insert(APhiIR);
      Instruction *PhiInst = APhiIR->getLLVMInstruction();
      registerUsers(Def, UVs, PhiInst, VisitedPhis);
    }
  }
}

AvrDefUse::AvrUsedVarsMapTy &AvrDefUse::registerDef(AVR *Def) {

  return DefUses[Def]; // Initialize to no uses.
}

template <typename AIRT> void AvrDefUse::registerDefAndUses(AIRT *Def) {

  AvrUsedVarsMapTy &UVs = registerDef(Def);

  const Instruction *Inst = Def->getLLVMInstruction();
  VisitedPhisTy VisitedPhis;
  registerUsers(Def, UVs, Inst, VisitedPhis);
}

void AvrDefUse::visit(AVRValueIR *AValueIR) {

  AVRPhiIR *ParentPhiIR = dyn_cast<AVRPhiIR>(AValueIR->getParent());
  if (ParentPhiIR) {

    // Phi nodes are not Defs by themselves, but rather pass-thru for their
    // incoming-value Defs, therefore:
    // - The LHS value of the Phi node is of no interest.
    // - Non-constant incoming values are Uses and are being taken care of by
    //   their reaching Defs.
    // - Constant incoming values are their own Def: register the AVRValue
    //   wrapping constant values as Defs and register their Users in the usual
    //   manner

    if (AValueIR->isConstant()) {

      // Register the AVRValue as Def.
      AvrUsedVarsMapTy &UVs = registerDef(AValueIR);

      // Propagate this Def to its AVR users. We can't call registerUsers with
      // the IR value, since mutiple AVRValues may be using it (so following the
      // IR2AVR map would be incorrect). Instead, we handle the parent AVRPhi
      // here ourselves and use the IR phi as the IR value whose (AVR) users
      // should become users of this Def.

      VisitedPhisTy VisitedPhis;

      // AVRPhis use their own phi instruction as the underlying variable since
      // all their arguments provide Defs to the phi itself. All other AVR nodes
      // use the given value.
      const Value *UnderlyingVariable = ParentPhiIR->getLLVMInstruction();

      // Register Use
      VarSetTy &Vs = UVs[ParentPhiIR];
      Vs.insert(UnderlyingVariable);

      // Register Reaching Def.
      ReachingDefs[ParentPhiIR][UnderlyingVariable].insert(AValueIR);

      // Propage Def to the users of the phi.
      VisitedPhis.insert(ParentPhiIR);
      registerUsers(AValueIR, UVs, UnderlyingVariable, VisitedPhis);
    }

    return;
  }

  if (isDef(AValueIR)) {

    // This AVR value is a Def of its IR value. Extract the actual Def AVR and
    // register it as a Def of this Value's users.

    AVR *RHS = getActualDef(AValueIR);
    assert(isa<AVRExpressionIR>(RHS));

    registerDefAndUses<AVRExpressionIR>(cast<AVRExpressionIR>(RHS));
  }
}

void AvrDefUse::visit(AVRCallIR *ACallIR) {

  const CallInst *Call = cast<CallInst>(ACallIR->getLLVMInstruction());
  FunctionType *CallType = Call->getFunctionType();

  if (!CallType->getReturnType()->isVoidTy())
    registerDefAndUses<AVRCallIR>(ACallIR);
}

void AvrDefUse::visit(AVRSelectIR *ASelectIR) {

  registerDefAndUses<AVRSelectIR>(ASelectIR);
}

void AvrDefUse::visit(AVRCompareIR *ACompareIR) {
  registerDefAndUses<AVRCompareIR>(ACompareIR);
}

bool AvrDefUse::runOnFunction(Function &F) {

  reset();

  AV = &getAnalysis<AVRGenerate>();

  if (AV->isAbstractLayerEmpty()) {
    AV = nullptr;
    return false;
  }

  // Collect inverse-mapping from IR to AVR.
  AVRVisitor<IR2AVRVisitor> IRVisitor(IR2AVR);
  IRVisitor.forwardVisit(AV->begin(), AV->end(), true, true,
                         true /*RecurseInsideValues*/);

  // Walk down the AVR tree and gather DU information.
  AVRVisitor<AvrDefUse> DUVisitor(*this);
  DUVisitor.forwardVisit(AV->begin(), AV->end(), true, true,
                         true /*RecurseInsideValues*/);

  AV = nullptr;
  return false;
}

FunctionPass *llvm::createAvrDefUsePass() { return new AvrDefUse(); }

INITIALIZE_PASS_BEGIN(AvrDefUseHIR, "avr-def-use-hir",
                      "VPO AVR-HIR Control Flow Graph", false, true)
INITIALIZE_PASS_DEPENDENCY(AVRGenerateHIR)
INITIALIZE_PASS_END(AvrDefUseHIR, "avr-def-use-hir",
                    "VPO AVR-HIR Control Flow Graph", false, true)

char AvrDefUseHIR::ID = 0;

AvrDefUseHIR::AvrDefUseHIR()
    : AvrDefUseBase(ID), TopLevelLoop(nullptr), DDG(nullptr, nullptr) {}

AvrDefUseHIR::~AvrDefUseHIR() {}

AVR *AvrDefUseBase::getActualDef(AVRValue *LHSValue) const {

  AVR *Parent = LHSValue->getParent();
  assert(Parent && "Expected value to have a parent");
  assert(isa<AVRExpression>(Parent) && "Expected expression for parent");
  assert(cast<AVRExpression>(Parent)->isLHSExpr() &&
         "Expected Def to be on the LHS");
  AVR *Grandparent = Parent->getParent();
  assert(Grandparent && "Expected value to have a parent");
  assert(isa<AVRAssign>(Grandparent) && "Expected assign for grandparent");
  return cast<AVRAssign>(Grandparent)->getRHS();
}

void AvrDefUseHIR::visit(AVRValueHIR *AValueHIR) {

  if (!isDef(AValueHIR))
    return;

  // CHECKME: Not sure if an AVRValueHIR with a null DDRef (IV) can reach this
  // point and it needs special treatment
  if (DDRef *DDR = AValueHIR->getValue()) {

    auto HLoop = TopLevelLoop->getLoop();

    // If the def is outside the loop, all uses of the def in the loop can
    // be treated as uniform. We want to avoid problems for cases where
    // we do not find the use AVR(such as in loop bounds for which we do
    // not build AVR nodes).
    if (!HLNodeUtils::contains(HLoop, DDR->getHLDDNode()))
      return;

    // This AVR value is a Def of its Symbase. Extract the actual Def AVR and
    // register it as a Def of this Value's RegDDRef's FLOW dependencies.
    AVR *RHS = getActualDef(AValueHIR);
    AvrUsedVarsMapTy &UVs = DefUses[RHS]; // Initialize to no uses.

    for (auto II = DDG.outgoing_edges_begin(DDR),
              EE = DDG.outgoing_edges_end(DDR);
         II != EE; ++II) {
      const DDEdge *Edge = *II;
      // Skip non-FLOW dependencies.
      if (!Edge->isFLOWdep())
        continue;

      DDRef *SinkDDR = Edge->getSink();
      RegDDRef *SelfBlob = dyn_cast<RegDDRef>(SinkDDR);

      // Skip dependencies to DDRefs that are neither blobs or self-blobs.
      if (!(isa<BlobDDRef>(SinkDDR) || (SelfBlob && SelfBlob->isSelfBlob())))
        continue;

      // Skip dependencies outside TopLevelLoop
      if (!HLNodeUtils::contains(HLoop, SinkDDR->getHLDDNode()))
        continue;

      // Skip dependencies to DDRefs whose using AVR is a Def.
      AVR *UsingAVR = IR2AVR.getAVR(SinkDDR);
      AVRValueHIR *UsingAVRValue = dyn_cast<AVRValueHIR>(UsingAVR);
      if (UsingAVRValue && isDef(UsingAVRValue))
        continue;

      UVs[UsingAVR].insert(SinkDDR);               // Register Use.
      ReachingDefs[UsingAVR][SinkDDR].insert(RHS); // Register Reaching Def.
    }
  }
}

void AvrDefUseHIR::visit(AVRLoopHIR *ALoopHIR) {

  if (TopLevelLoop)
    return;

  TopLevelLoop = ALoopHIR;
  DDG = DDA->getGraph(ALoopHIR->getLoop(), false);

  DEBUG(formatted_raw_ostream FOS(dbgs()); FOS << "Top-Level loop DDG:\n";
        DDG.print(FOS); FOS << "\n");
}

void AvrDefUseHIR::postVisit(AVRLoopHIR *ALoopHIR) {

  if (TopLevelLoop != ALoopHIR)
    return;

  TopLevelLoop = nullptr;
  DDG = DDGraph(nullptr, nullptr);
}

void AvrDefUseHIR::visit(AVRWrn *AWrn) {
  //  DDG = DDA->getGraph(AWrn->getLoop(), false);
}

void AvrDefUseHIR::postVisit(AVRWrn *AWrn) {
  //  DDG = DDGraph(nullptr, nullptr);
}

bool AvrDefUseHIR::runOnFunction(Function &F) {

  reset();

  TopLevelLoop = nullptr;

  AVRGenerateHIR &AV = getAnalysis<AVRGenerateHIR>();
  DDA = &getAnalysis<HIRDDAnalysis>();

  if (AV.isAbstractLayerEmpty()) {
    return false;
  }

  // Collect inverse-mapping from IR to AVR.
  AVRVisitor<IR2AVRVisitor> IRVisitor(IR2AVR);
  IRVisitor.forwardVisit(AV.begin(), AV.end(), true, true,
                         true /*RecurseInsideValues*/);

  // Walk down the AVR tree and gather DU information.
  AVRVisitor<AvrDefUseHIR> DUVisitor(*this);
  DUVisitor.forwardVisit(AV.begin(), AV.end(), true, true,
                         true /*RecurseInsideValues*/);

  return false;
}

FunctionPass *llvm::createAvrDefUseHIRPass() { return new AvrDefUseHIR(); }
