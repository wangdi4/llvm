//===--- Intel_MDInlineReport.cpp  --Inlining report vis metadata --------===//
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements InliningReport class using metadata. The base class is
// InliningReport. FunctionInliningReport and CallSiteInliningReport represent
// inlining reports for function and call site.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_MDInlineReport.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"

using namespace llvm;
using namespace MDInliningReport;

#define DEBUG_TYPE "mdinlinereport"

std::string llvm::getLinkageStr(Function *F) {
  std::string LinkageChar =
      (F->hasLocalLinkage()
           ? "L"
           : (F->hasLinkOnceODRLinkage()
                  ? "O"
                  : (F->hasAvailableExternallyLinkage() ? "X" : "A")));
  return LinkageChar;
}

//
// Constructor for function inlining report metadata and object.
// Ex.:
// !21 = distinct !{!"intel.function.inlining.report", !"name: a", !22,
//       !"name: test1.c", !"isDead: 0", !"isDeclaration: 0", !"A"}
// !22 = distinct !{!"intel.callsites.inlining.report"}
FunctionInliningReport::FunctionInliningReport(LLVMContext *C,
                                               std::string FuncName,
                                               std::vector<MDTuple *> *CSs,
                                               std::string ModuleName,
                                               bool IsDead, bool IsDeclaration,
                                               std::string LinkageStr) {
  // Create inlining report metadata list for call sites inside function.
  SmallVector<Metadata *, 100> Ops;
  Ops.push_back(llvm::MDString::get(*C, CallSitesTag));
  if (CSs)
    for (auto *MD : *CSs)
      Ops.push_back(MD);
  auto MDCSs = MDTuple::getDistinct(*C, Ops);
  Ops.clear();
  // Now create function metadata inlining report.
  // Op 0: 'intel.function.inlining.report' tag
  Ops.push_back(llvm::MDString::get(*C, FunctionTag));
  // Op 1: function name
  FuncName.insert(0, "name: ");
  Ops.push_back(llvm::MDString::get(*C, FuncName));
  // Op 2: callsites
  Ops.push_back(MDCSs);
  // Op 3: module name
  ModuleName.insert(0, "moduleName: ");
  Ops.push_back(llvm::MDString::get(*C, ModuleName));
  // Op 4: is dead?
  std::string IsDeadStr = "isDead: ";
  IsDeadStr.append(std::to_string(IsDead));
  Ops.push_back(llvm::MDString::get(*C, IsDeadStr));
  // Op 5: is declaration?
  std::string IsDeclStr = "isDeclaration: ";
  IsDeclStr.append(std::to_string(IsDeclaration));
  Ops.push_back(llvm::MDString::get(*C, IsDeclStr));
  // Op 6: linkage string
  LinkageStr.insert(0, "linkage: ");
  Ops.push_back(llvm::MDString::get(*C, LinkageStr));
  Report = MDTuple::getDistinct(*C, Ops);
}

bool FunctionInliningReport::isFunctionInliningReportMetadata(
    const Metadata *R) {
  const MDTuple *T = dyn_cast<MDTuple>(R);
  if (!T)
    return false;

  if (T->getNumOperands() == 0)
    return false;

  const MDString *S = dyn_cast<MDString>(T->getOperand(0));
  if (!S)
    return false;

  return S->getString() == FunctionTag;
}

//
// Call site inlining report initialization function.
// Ex.:
// !31 = distinct !{!"intel.callsite.inlining.report", !"name: z", !32,
//       !"isInlined: 0", !"reason: 25", !"inlineCost: -1",
//       !"outerInlineCost: -1", !"inlineThreshold: -1",
//       !"earlyExitCost: 2147483647", !"earlyExitThreshold: 2147483647",
//       !"line: 23 col: 3"}
//
MDTuple *CallSiteInliningReport::initCallSite(
    LLVMContext *C, std::string Name, std::vector<MDTuple *> *CSs,
    InlineReason Reason, bool IsInlined, int InlineCost, int OuterInlineCost,
    int InlineThreshold, int EarlyExitInlineCost, int EarlyExitInlineThreshold,
    unsigned Line, unsigned Col, std::string ModuleName) {
  // Create a list of inlining reports for call sites that appear from inlining
  // of the current call site.
  SmallVector<Metadata *, 100> Ops;
  Ops.push_back(llvm::MDString::get(*C, CallSitesTag));
  if (CSs)
    for (auto *MD : *CSs)
      Ops.push_back(MD);
  auto MDCSs = MDTuple::getDistinct(*C, Ops);
  Ops.clear();
  // Now create call site inlining report.
  // Op 0: 'intel.callsite.inlining.report' tag
  Ops.push_back(llvm::MDString::get(*C, CallSiteTag));
  // Op 1: callee name
  Name.insert(0, "name: ");
  Ops.push_back(llvm::MDString::get(*C, Name));
  // Op 2: dependent callsites
  Ops.push_back(MDCSs);
  // Op 3: is inlined?
  std::string IsInlinedStr = "isInlined: ";
  IsInlinedStr.append(std::to_string(IsInlined));
  Ops.push_back(llvm::MDString::get(*C, IsInlinedStr));
  // Op 4: inline reason
  std::string ReasonStr = "reason: ";
  ReasonStr.append(std::to_string(Reason));
  Ops.push_back(llvm::MDString::get(*C, ReasonStr));
  // Op 5: inline cost
  std::string InlineCostStr = "inlineCost: ";
  InlineCostStr.append(std::to_string(InlineCost));
  Ops.push_back(llvm::MDString::get(*C, InlineCostStr));
  // Op 6: outer inline cost
  std::string OuterInlineCostStr = "outerInlineCost: ";
  OuterInlineCostStr.append(std::to_string(OuterInlineCost));
  Ops.push_back(llvm::MDString::get(*C, OuterInlineCostStr));
  // Op 7: inline threshold
  std::string InlineThresholdStr = "inlineThreshold: ";
  InlineThresholdStr.append(std::to_string(InlineThreshold));
  Ops.push_back(llvm::MDString::get(*C, InlineThresholdStr));
  // Op 8: early exit cost
  std::string EarlyExitCostStr = "earlyExitCost: ";
  EarlyExitCostStr.append(std::to_string(EarlyExitInlineCost));
  Ops.push_back(llvm::MDString::get(*C, EarlyExitCostStr));
  // Op 9: early exit threshold
  std::string EarlyExitThresholdStr = "earlyExitThreshold: ";
  EarlyExitThresholdStr.append(std::to_string(EarlyExitInlineThreshold));
  Ops.push_back(llvm::MDString::get(*C, EarlyExitThresholdStr));
  // Op 10: line and column
  std::string LineAndColStr = "line: ";
  LineAndColStr.append(std::to_string(Line));
  LineAndColStr.append(" col: ");
  LineAndColStr.append(std::to_string(Col));
  Ops.push_back(llvm::MDString::get(*C, LineAndColStr));
  // Op 11: module name
  ModuleName.insert(0, "moduleName: ");
  Ops.push_back(llvm::MDString::get(*C, ModuleName));
  return MDTuple::getDistinct(*C, Ops);
}

CallSiteInliningReport::CallSiteInliningReport(
    LLVMContext *C, std::string Name, std::vector<MDTuple *> *CSs,
    InlineReason Reason, bool IsInlined, int InlineCost, int OuterInlineCost,
    int InlineThreshold, int EarlyExitInlineCost, int EarlyExitInlineThreshold,
    unsigned Line, unsigned Col, std::string ModuleName) {
  Report = initCallSite(C, Name, CSs, Reason, IsInlined, InlineCost,
                        OuterInlineCost, InlineThreshold, EarlyExitInlineCost,
                        EarlyExitInlineThreshold, Line, Col, ModuleName);
}

CallSiteInliningReport::CallSiteInliningReport(
    CallSite MainCS, std::vector<MDTuple *> *CSs, InlineReason Reason,
    bool IsInlined, int InlineCost, int OuterInlineCost, int InlineThreshold,
    int EarlyExitInlineCost, int EarlyExitInlineThreshold) {
  Function *Callee = MainCS.getCalledFunction();
  std::string Name = Callee ? (Callee->hasName() ? Callee->getName() : "") : "";
  const DebugLoc &DL = MainCS->getDebugLoc();
  StringRef ModuleName =
      MainCS->getParent()->getParent()->getParent()->getName();
  Report = initCallSite(&(MainCS->getParent()->getParent()->getContext()), Name,
                        CSs, Reason, IsInlined, InlineCost, OuterInlineCost,
                        InlineThreshold, EarlyExitInlineCost,
                        EarlyExitInlineThreshold, (DL ? DL.getLine() : 0),
                        (DL ? DL.getCol() : 0), ModuleName.str());
}

bool CallSiteInliningReport::isCallSiteInliningReportMetadata(
    const Metadata *R) {
  const MDTuple *T = dyn_cast<MDTuple>(R);
  if (!T)
    return false;

  if (T->getNumOperands() == 0)
    return false;

  const MDString *S = dyn_cast<MDString>(T->getOperand(0));
  if (!S)
    return false;

  return S->getString() == CallSiteTag;
}

void llvm::setMDReasonNotInlined(const CallSite CS, const InlineCost &IC) {
  InlineReason Reason = IC.getInlineReason();
  assert(IsNotInlinedReason(Reason));
  llvm::setMDReasonNotInlined(CS, Reason);
  Metadata *CSMD = CS->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert(CSIR && (CSIR->getNumOperands() == CallSiteMDSize) &&
         "Incorrect call site inline report metadata");
  LLVMContext &Ctx = CS->getParent()->getParent()->getParent()->getContext();
  std::string ReasonStr = "reason: ";
  ReasonStr.append(std::to_string(Reason));
  CSIR->replaceOperandWith(CSMDIR_InlineReason,
                           llvm::MDString::get(Ctx, ReasonStr));
  std::string InlineCostStr = "inlineCost: ";
  InlineCostStr.append(std::to_string(IC.getCost()));
  CSIR->replaceOperandWith(CSMDIR_InlineCost,
                           llvm::MDString::get(Ctx, InlineCostStr));
  std::string InlineThresholdStr = "inlineThreshold: ";
  InlineThresholdStr.append(std::to_string(IC.getCost() + IC.getCostDelta()));
  CSIR->replaceOperandWith(CSMDIR_InlineThreshold,
                           llvm::MDString::get(Ctx, InlineThresholdStr));
  std::string EarlyExitCostStr = "earlyExitCost: ";
  EarlyExitCostStr.append(std::to_string(IC.getEarlyExitCost()));
  CSIR->replaceOperandWith(CSMDIR_EarlyExitCost,
                           llvm::MDString::get(Ctx, EarlyExitCostStr));
  std::string EarlyExitThresholdStr = "earlyExitThreshold: ";
  EarlyExitThresholdStr.append(std::to_string(IC.getEarlyExitThreshold()));
  CSIR->replaceOperandWith(CSMDIR_EarlyExitThreshold,
                           llvm::MDString::get(Ctx, EarlyExitThresholdStr));
}

void llvm::setMDReasonNotInlined(const CallSite CS, InlineReason Reason) {
  assert(IsNotInlinedReason(Reason));
  Metadata *CSMD = CS->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert(CSIR && (CSIR->getNumOperands() == CallSiteMDSize) &&
         "Incorrect call site inline report metadata");
  LLVMContext &Ctx = CS->getParent()->getParent()->getParent()->getContext();
  std::string ReasonStr = "reason: ";
  ReasonStr.append(std::to_string(Reason));
  CSIR->replaceOperandWith(CSMDIR_InlineReason,
                           llvm::MDString::get(Ctx, ReasonStr));
}

void llvm::setMDReasonNotInlined(const CallSite CS, const InlineCost &IC,
                                 int TotalSecondaryCost) {
  assert(IC.getInlineReason() == NinlrOuterInlining);
  llvm::setMDReasonNotInlined(CS, IC);
  Metadata *CSMD = CS->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert(CSIR && (CSIR->getNumOperands() == CallSiteMDSize) &&
         "Incorrect call site inline report metadata");
  LLVMContext &Ctx = CS->getParent()->getParent()->getParent()->getContext();
  std::string OuterInlineCostStr = "outerInlineCost: ";
  OuterInlineCostStr.append(std::to_string(TotalSecondaryCost));
  CSIR->replaceOperandWith(CSMDIR_OuterInlineCost,
                           llvm::MDString::get(Ctx, OuterInlineCostStr));
}

// Set inlining reason
void llvm::setMDReasonIsInlined(const CallSite CS, InlineReason Reason) {
  assert(IsInlinedReason(Reason));
  Metadata *CSMD = CS->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert(CSIR && (CSIR->getNumOperands() == CallSiteMDSize) &&
         "Incorrect call site inline report metadata");
  LLVMContext &Ctx = CS->getParent()->getParent()->getParent()->getContext();
  std::string ReasonStr = "reason: ";
  ReasonStr.append(std::to_string(Reason));
  CSIR->replaceOperandWith(CSMDIR_InlineReason,
                           llvm::MDString::get(Ctx, ReasonStr));
}

// Set inlining reason
void llvm::setMDReasonIsInlined(const CallSite CS, const InlineCost &IC) {
  InlineReason Reason = IC.getInlineReason();
  assert(IsInlinedReason(Reason));
  Metadata *CSMD = CS->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert(CSIR && (CSIR->getNumOperands() == CallSiteMDSize) &&
         "Incorrect call site inline report metadata");
  LLVMContext &Ctx = CS->getParent()->getParent()->getParent()->getContext();
  std::string IsInlinedStr = "isInlined: ";
  IsInlinedStr.append(std::to_string(true));
  CSIR->replaceOperandWith(CSMDIR_IsInlined,
                           llvm::MDString::get(Ctx, IsInlinedStr));
  std::string ReasonStr = "reason: ";
  ReasonStr.append(std::to_string(Reason));
  CSIR->replaceOperandWith(CSMDIR_InlineReason,
                           llvm::MDString::get(Ctx, ReasonStr));
  std::string InlineCostStr = "inlineCost: ";
  InlineCostStr.append(std::to_string(IC.getCost()));
  CSIR->replaceOperandWith(CSMDIR_InlineCost,
                           llvm::MDString::get(Ctx, InlineCostStr));
  std::string InlineThresholdStr = "inlineThreshold: ";
  InlineThresholdStr.append(std::to_string(IC.getCost() + IC.getCostDelta()));
  CSIR->replaceOperandWith(CSMDIR_InlineThreshold,
                           llvm::MDString::get(Ctx, InlineThresholdStr));
}

// In the beginning of the inlining process put all functions and call
// instructions together with their inlining reports into callback list.
void InlineReportBuilder::beginFunction(Function *F) {
  if (!isMDIREnabled())
    return;
  if (!F)
    return;
  if (!F->getMetadata(FunctionTag)) {
    // no inlining report set up.
    return;
  }

  LLVM_DEBUG(dbgs() << "\nMDIR inline: begin function " << F->getName()
                    << "\n");

  std::vector<MDTuple *> CSs;
  for (BasicBlock &BB : *F) {
    for (Instruction &I : BB) {
      CallSite CS(cast<Value>(&I));
      // If this isn't a call it can never be inlined.
      if (!CS)
        continue;
      if (auto CSMD = CS->getMetadata(CallSiteTag))
        addCallback(&I, CSMD);
    }
  }

  auto *FMD = F->getMetadata(FunctionTag);
  addCallback(F, FMD);

  // Update function linkage char
  auto *FIR = dyn_cast<MDTuple>(FMD);
  assert(FIR && "Bad function inline report format");
  LLVMContext &Ctx = F->getParent()->getContext();
  // Op 7: linkage string
  std::string LinkageStr = "linkage: ";
  LinkageStr.append(llvm::getLinkageStr(F));
  FIR->replaceOperandWith(FMDIR_LinkageStr,
                          llvm::MDString::get(Ctx, LinkageStr));
  return;
}

// The main goal of beginSCC() and beginFunction() routines is to fill in the
// list of callbacks which is stored in InlineReportBuilder object.
void InlineReportBuilder::beginSCC(CallGraph &CG, CallGraphSCC &SCC) {
  if (!isMDIREnabled())
    return;
  Module &M = CG.getModule();
  NamedMDNode *ModuleInlineReport = M.getNamedMetadata(ModuleTag);
  if (!ModuleInlineReport || ModuleInlineReport->getNumOperands() == 0)
    return;
  for (CallGraphNode *Node : SCC) {
    Function *F = Node->getFunction();
    if (!F)
      continue;
    beginFunction(F);
  }
}

void InlineReportBuilder::beginSCC(LazyCallGraph &CG, LazyCallGraph::SCC &SCC) {
  if (!isMDIREnabled())
    return;
  Module &M = CG.getModule();
  NamedMDNode *ModuleInlineReport = M.getNamedMetadata(ModuleTag);
  if (!ModuleInlineReport || ModuleInlineReport->getNumOperands() == 0)
    return;
  for (auto &Node : SCC)
    beginFunction(&(Node.getFunction()));
}

// Function set dead
void InlineReportBuilder::setDead(Function *F) {
  if (!isMDIREnabled())
    return;
  Metadata *FMD = F->getMetadata(FunctionTag);
  if (!FMD)
    return;
  auto *FIR = dyn_cast<MDTuple>(FMD);
  if (!FIR)
    return;
  LLVMContext &Ctx = F->getParent()->getContext();
  std::string IsDeadStr = "isDead: ";
  IsDeadStr.append(std::to_string(true));
  FIR->replaceOperandWith(FMDIR_IsDead, llvm::MDString::get(Ctx, IsDeadStr));
}

// This function copies simple metadata node and recursively calls itself to
// copy metadata list node. It also creates a map between original and copied
// metadata nodes.
static Metadata *
cloneInliningReportHelper(LLVMContext &C, Metadata *OldMD,
                          DenseMap<Metadata *, Metadata *> &MDMap) {
  Metadata *NewMD = nullptr;
  if (MDString *OldStrMD = dyn_cast<MDString>(OldMD))
    NewMD = llvm::MDString::get(C, OldStrMD->getString());
  else if (MDTuple *OldTupleMD = dyn_cast<MDTuple>(OldMD)) {
    SmallVector<Metadata *, 20> Ops;
    int NumOps = OldTupleMD->getNumOperands();
    for (int I = 0; I < NumOps; ++I)
      Ops.push_back(
          cloneInliningReportHelper(C, OldTupleMD->getOperand(I), MDMap));
    NewMD = MDTuple::getDistinct(C, Ops);
  }
  MDMap[OldMD] = NewMD;
  return NewMD;
}

// When inlining of the callee happens we need to update inlining report for
// current call instructions. To achieve that, we clone function inlining report
// of the callee and attach it to the call site, updating isInlined value.
Metadata *InlineReportBuilder::cloneInliningReport(Function *F,
                                                   ValueToValueMapTy &VMap) {
  if (!isMDIREnabled())
    return nullptr;
  DenseMap<Metadata *, Metadata *> MDMap;
  Metadata *FuncMD = F->getMetadata(FunctionTag);
  if (!FuncMD)
    return nullptr;
  Metadata *NewFuncMD =
      cloneInliningReportHelper(F->getParent()->getContext(), FuncMD, MDMap);
  for (ValueToValueMapTy::iterator InstIt = VMap.begin(),
                                   InstItEnd = VMap.end();
       InstIt != InstItEnd; InstIt++) {
    if (!InstIt->second)
      continue;

    const Instruction *OldI = dyn_cast<Instruction>(InstIt->first);
    if (!OldI || !isa<CallBase>(OldI))
      continue;
    Metadata *OldMetadata = OldI->getMetadata(CallSiteTag);
    if (!OldMetadata)
      continue;
    Instruction *NewI = dyn_cast_or_null<Instruction>(InstIt->second);
    if (!NewI)
      continue;
    MDTuple *NewMetadata = dyn_cast_or_null<MDTuple>(MDMap[OldMetadata]);
    if (!NewMetadata)
      continue;
    NewI->setMetadata(CallSiteTag, NewMetadata);
    addCallback(NewI, NewMetadata);
  }
  return NewFuncMD;
}

// Update inlining report of the inlined call site.
void InlineReportBuilder::updateInliningReport() {
  if (!isMDIREnabled())
    return;
  if (!CurrentCallee)
    return;
  if (!CurrentCallInstReport)
    return;
  ValueToValueMapTy VMap;
  for (unsigned I = 0, E = ActiveOriginalCalls.size(); I < E; ++I) {
    // In the case of a directly recursive call, avoid putting the
    // ActiveInlineInstruction into the IIMap, as it is an already
    // deleted value. Putting it in the IIMAP could cause a ValueHandle
    // to be associated with it, which should not happen because it is
    // deleted.
    Value *OldCall = ActiveOriginalCalls[I] == CurrentCallInstr
                         ? nullptr
                         : ActiveOriginalCalls[I];
    Value *NewCall = ActiveInlinedCalls[I];
    VMap.insert(std::make_pair(OldCall, NewCall));
  }

  Metadata *NewMD = cloneInliningReport(CurrentCallee, VMap);
  if (!NewMD)
    return;
  MDTuple *NewCallSiteIR = cast<MDTuple>(NewMD);
  MDTuple *OldCallSiteIR = cast<MDTuple>(CurrentCallInstReport);
  OldCallSiteIR->replaceOperandWith(CSMDIR_CSs,
                                    NewCallSiteIR->getOperand(CSMDIR_CSs));
  LLVMContext &Ctx = CurrentCallee->getParent()->getContext();
  std::string IsInlinedStr = "isInlined: ";
  IsInlinedStr.append(std::to_string(true));
  OldCallSiteIR->replaceOperandWith(CSMDIR_IsInlined,
                                    llvm::MDString::get(Ctx, IsInlinedStr));
}

void InlineReportBuilder::replaceFunctionWithFunction(Function *OldFunction,
                                                      Function *NewFunction) {
  if (!isMDIREnabled())
    return;
  if (OldFunction == NewFunction)
    return;
  Metadata *OldFMD = OldFunction->getMetadata(FunctionTag);
  if (!OldFMD)
    return;
  auto *OldFIR = dyn_cast<MDTuple>(OldFMD);
  if (!OldFIR)
    return;

  LLVMContext &Ctx = NewFunction->getParent()->getContext();
  // Op 1: function name
  std::string FuncName = NewFunction->getName();
  FuncName.insert(0, "name: ");
  OldFIR->replaceOperandWith(FMDIR_FuncName,
                             llvm::MDString::get(Ctx, FuncName));
  // Op 7: linkage string
  std::string LinkageStr = "linkage: ";
  LinkageStr.append(llvm::getLinkageStr(NewFunction));
  OldFIR->replaceOperandWith(FMDIR_LinkageStr,
                             llvm::MDString::get(Ctx, LinkageStr));
  NewFunction->setMetadata(FunctionTag, OldFIR);
  addCallback(NewFunction, OldFIR);
}

