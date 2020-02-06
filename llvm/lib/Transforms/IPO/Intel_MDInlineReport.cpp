//===--- Intel_MDInlineReport.cpp  --Inlining report vis metadata --------===//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/IPO/Utils/Intel_IPOUtils.h"

#include "llvm/ADT/SmallPtrSet.h"
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
//       !"name: test1.c", !"isDead: 0", !"isDeclaration: 0", !"linkage: A",
//       !"isSuppressPrint: 0"}
// !22 = distinct !{!"intel.callsites.inlining.report"}
FunctionInliningReport::FunctionInliningReport(LLVMContext *C,
                                               std::string FuncName,
                                               std::vector<MDTuple *> *CSs,
                                               std::string ModuleName,
                                               bool IsDead, bool IsDeclaration,
                                               bool isSuppressPrint,
                                               std::string LinkageStr) {
  // Create inlining report metadata list for call sites inside function.
  SmallVector<Metadata *, 100> Ops;
  if (CSs && CSs->size()) {
    Ops.push_back(llvm::MDString::get(*C, CallSitesTag));
    for (auto *MD : *CSs)
      Ops.push_back(MD);
  }
  auto MDCSs = Ops.empty() ? nullptr : MDTuple::getDistinct(*C, Ops);
  Ops.clear();
  // Now create function metadata inlining report.
  // Op 0: 'intel.function.inlining.report' tag
  Ops.push_back(llvm::MDString::get(*C, FunctionTag));
  // Op 1: function name
  FuncName.insert(0, "name: ");
  auto FuncNameMD = MDNode::get(*C, llvm::MDString::get(*C, FuncName));
  Ops.push_back(FuncNameMD);
  // Op 2: callsites
  Ops.push_back(MDCSs);
  // Op 3: module name
  ModuleName.insert(0, "moduleName: ");
  auto ModuleNameMD = MDNode::get(*C, llvm::MDString::get(*C, ModuleName));
  Ops.push_back(ModuleNameMD);
  // Op 4: is dead?
  std::string IsDeadStr = "isDead: ";
  IsDeadStr.append(std::to_string(IsDead));
  auto IsDeadMD = MDNode::get(*C, llvm::MDString::get(*C, IsDeadStr));
  Ops.push_back(IsDeadMD);
  // Op 5: is declaration?
  std::string IsDeclStr = "isDeclaration: ";
  IsDeclStr.append(std::to_string(IsDeclaration));
  auto IsDeclMD = MDNode::get(*C, llvm::MDString::get(*C, IsDeclStr));
  Ops.push_back(IsDeclMD);
  // Op 6: linkage string
  LinkageStr.insert(0, "linkage: ");
  auto LinkageMD = MDNode::get(*C, llvm::MDString::get(*C, LinkageStr));
  Ops.push_back(LinkageMD);
  // Op 7: isSuppressPrint
  std::string IsSuppressPrintStr = "isSuppressPrint: ";
  if (isSuppressPrint) {
    LLVM_DEBUG(dbgs() << "isSuppressPrint is ON\n";);
    setSuppressPrint(true);
  }
  IsSuppressPrintStr.append(std::to_string(isSuppressPrint));
  auto IsSuppressPrintMD =
      MDNode::get(*C, llvm::MDString::get(*C, IsSuppressPrintStr));
  Ops.push_back(IsSuppressPrintMD);

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
    InlineReason Reason, bool IsInlined, bool IsSuppressPrint,
    int InlineCost, int OuterInlineCost,
    int InlineThreshold, int EarlyExitInlineCost, int EarlyExitInlineThreshold,
    unsigned Line, unsigned Col, std::string ModuleName) {
  // Create a list of inlining reports for call sites that appear from inlining
  // of the current call site.
  SmallVector<Metadata *, 100> Ops;
  if (CSs && CSs->size()) {
    Ops.push_back(llvm::MDString::get(*C, CallSitesTag));
    for (auto *MD : *CSs)
      Ops.push_back(MD);
  }
  auto MDCSs = Ops.empty() ? nullptr : MDTuple::getDistinct(*C, Ops);
  Ops.clear();
  // Now create call site inlining report.
  // Op 0: 'intel.callsite.inlining.report' tag
  Ops.push_back(llvm::MDString::get(*C, CallSiteTag));
  // Op 1: callee name
  Name.insert(0, "name: ");
  auto NameMD = MDNode::get(*C, llvm::MDString::get(*C, Name));
  Ops.push_back(NameMD);
  // Op 2: dependent callsites
  Ops.push_back(MDCSs);
  // Op 3: is inlined?
  std::string IsInlinedStr = "isInlined: ";
  IsInlinedStr.append(std::to_string(IsInlined));
  auto IsInlinedMD = MDNode::get(*C, llvm::MDString::get(*C, IsInlinedStr));
  Ops.push_back(IsInlinedMD);
  // Op 4: inline reason
  std::string ReasonStr = "reason: ";
  ReasonStr.append(std::to_string(Reason));
  auto ReasonMD = MDNode::get(*C, llvm::MDString::get(*C, ReasonStr));
  Ops.push_back(ReasonMD);
  // Op 5: inline cost
  std::string InlineCostStr = "inlineCost: ";
  InlineCostStr.append(std::to_string(InlineCost));
  auto InlineCostMD = MDNode::get(*C, llvm::MDString::get(*C, InlineCostStr));
  Ops.push_back(InlineCostMD);
  // Op 6: outer inline cost
  std::string OuterInlineCostStr = "outerInlineCost: ";
  OuterInlineCostStr.append(std::to_string(OuterInlineCost));
  auto OuterInlineCostMD =
      MDNode::get(*C, llvm::MDString::get(*C, OuterInlineCostStr));
  Ops.push_back(OuterInlineCostMD);
  // Op 7: inline threshold
  std::string InlineThresholdStr = "inlineThreshold: ";
  InlineThresholdStr.append(std::to_string(InlineThreshold));
  auto InlineThresholdMD =
      MDNode::get(*C, llvm::MDString::get(*C, InlineThresholdStr));
  Ops.push_back(InlineThresholdMD);
  // Op 8: early exit cost
  std::string EarlyExitCostStr = "earlyExitCost: ";
  EarlyExitCostStr.append(std::to_string(EarlyExitInlineCost));
  auto EECostMD = MDNode::get(*C, llvm::MDString::get(*C, EarlyExitCostStr));
  Ops.push_back(EECostMD);
  // Op 9: early exit threshold
  std::string EarlyExitThresholdStr = "earlyExitThreshold: ";
  EarlyExitThresholdStr.append(std::to_string(EarlyExitInlineThreshold));
  auto EEThresholdMD =
      MDNode::get(*C, llvm::MDString::get(*C, EarlyExitThresholdStr));
  Ops.push_back(EEThresholdMD);
  // Op 10: line and column
  std::string LineAndColStr = "line: ";
  LineAndColStr.append(std::to_string(Line));
  LineAndColStr.append(" col: ");
  LineAndColStr.append(std::to_string(Col));
  Ops.push_back(llvm::MDString::get(*C, LineAndColStr));
  // Op 11: module name
  ModuleName.insert(0, "moduleName: ");
  auto ModuleNameMD = MDNode::get(*C, llvm::MDString::get(*C, ModuleName));
  Ops.push_back(ModuleNameMD);

  // Op 12: isSuppressPrint
  std::string IsSuppressPrintStr = "isSuppressPrint: ";
  if (IsSuppressPrint) {
    LLVM_DEBUG(dbgs() << "IsSuppressPrint is ON\n";);
    setSuppressPrint(true);
  }
  IsSuppressPrintStr.append(std::to_string(IsSuppressPrint));
  auto IsSuppressPrintMD =
      MDNode::get(*C, llvm::MDString::get(*C, IsSuppressPrintStr));
  Ops.push_back(IsSuppressPrintMD);

  return MDTuple::getDistinct(*C, Ops);
}

CallSiteInliningReport::CallSiteInliningReport(
    LLVMContext *C, std::string Name, std::vector<MDTuple *> *CSs,
    InlineReason Reason, bool IsInlined, bool IsSuppressPrint, int InlineCost,
    int OuterInlineCost,
    int InlineThreshold, int EarlyExitInlineCost, int EarlyExitInlineThreshold,
    unsigned Line, unsigned Col, std::string ModuleName) {
  Report = initCallSite(C, Name, CSs, Reason, IsInlined, IsSuppressPrint,
                        InlineCost, OuterInlineCost, InlineThreshold,
                        EarlyExitInlineCost, EarlyExitInlineThreshold, Line,
                        Col, ModuleName);
}

CallSiteInliningReport::CallSiteInliningReport(
    CallBase *MainCB, std::vector<MDTuple *> *CSs, InlineReason Reason,
    bool IsInlined, bool IsSuppressPrint, int InlineCost, int OuterInlineCost,
    int InlineThreshold, int EarlyExitInlineCost,
    int EarlyExitInlineThreshold) {
  Function *Callee = MainCB->getCalledFunction();
  std::string Name =
      Callee ? std::string(Callee->hasName() ? Callee->getName() : "") : "";
  const DebugLoc &DL = MainCB->getDebugLoc();
  StringRef ModuleName =
      MainCB->getParent()->getParent()->getParent()->getName();
  Report = initCallSite(&(MainCB->getParent()->getParent()->getContext()), Name,
                        CSs, Reason, IsInlined,
                        MainCB->getMetadata(IPOUtils::getSuppressInlineReportStringRef()),
                        InlineCost, OuterInlineCost,
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
  assert((CSIR->getNumOperands() == CallSiteMDSize) &&
      "Incorrect call site inline report metadata");
  LLVMContext &Ctx = CS->getParent()->getParent()->getParent()->getContext();
  std::string InlineCostStr = "inlineCost: ";
  InlineCostStr.append(std::to_string(IC.getCost()));
  auto InlineCostMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, InlineCostStr));
  CSIR->replaceOperandWith(CSMDIR_InlineCost, InlineCostMD);
  std::string InlineThresholdStr = "inlineThreshold: ";
  InlineThresholdStr.append(std::to_string(IC.getCost() + IC.getCostDelta()));
  auto InlineThresholdMD =
      MDNode::get(Ctx, llvm::MDString::get(Ctx, InlineThresholdStr));
  CSIR->replaceOperandWith(CSMDIR_InlineThreshold, InlineThresholdMD);
  std::string EarlyExitCostStr = "earlyExitCost: ";
  EarlyExitCostStr.append(std::to_string(IC.getEarlyExitCost()));
  auto EECostMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, EarlyExitCostStr));
  CSIR->replaceOperandWith(CSMDIR_EarlyExitCost, EECostMD);
  std::string EarlyExitThresholdStr = "earlyExitThreshold: ";
  EarlyExitThresholdStr.append(std::to_string(IC.getEarlyExitThreshold()));
  auto EEThresholdMD =
      MDNode::get(Ctx, llvm::MDString::get(Ctx, EarlyExitThresholdStr));
  CSIR->replaceOperandWith(CSMDIR_EarlyExitThreshold, EEThresholdMD);
}

void llvm::setMDReasonNotInlined(const CallSite CS, InlineReason Reason) {
  assert(IsNotInlinedReason(Reason));
  Metadata *CSMD = CS->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert((CSIR->getNumOperands() == CallSiteMDSize) &&
      "Incorrect call site inline report metadata");
  LLVMContext &Ctx = CS->getParent()->getParent()->getParent()->getContext();
  std::string ReasonStr = "reason: ";
  ReasonStr.append(std::to_string(Reason));
  auto ReasonMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, ReasonStr));
  CSIR->replaceOperandWith(CSMDIR_InlineReason, ReasonMD);
}

void llvm::setMDReasonNotInlined(const CallSite CS, const InlineCost &IC,
                                 int TotalSecondaryCost) {
  assert(IC.getInlineReason() == NinlrOuterInlining);
  llvm::setMDReasonNotInlined(CS, IC);
  Metadata *CSMD = CS->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert((CSIR->getNumOperands() == CallSiteMDSize) &&
      "Incorrect call site inline report metadata");
  LLVMContext &Ctx = CS->getParent()->getParent()->getParent()->getContext();
  std::string OuterInlineCostStr = "outerInlineCost: ";
  OuterInlineCostStr.append(std::to_string(TotalSecondaryCost));
  auto OuterInlineCostMD =
      MDNode::get(Ctx, llvm::MDString::get(Ctx, OuterInlineCostStr));
  CSIR->replaceOperandWith(CSMDIR_OuterInlineCost, OuterInlineCostMD);
}

// Set inlining reason
void llvm::setMDReasonIsInlined(const CallSite CS, InlineReason Reason) {
  assert(IsInlinedReason(Reason));
  Metadata *CSMD = CS->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert((CSIR->getNumOperands() == CallSiteMDSize) &&
      "Incorrect call site inline report metadata");
  LLVMContext &Ctx = CS->getParent()->getParent()->getParent()->getContext();
  std::string ReasonStr = "reason: ";
  ReasonStr.append(std::to_string(Reason));
  auto ReasonMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, ReasonStr));
  CSIR->replaceOperandWith(CSMDIR_InlineReason, ReasonMD);
}

// Set inlining reason
void llvm::setMDReasonIsInlined(const CallSite CS, const InlineCost &IC) {
  InlineReason Reason = IC.getInlineReason();
  assert(IsInlinedReason(Reason));
  llvm::setMDReasonIsInlined(CS, Reason);
  Metadata *CSMD = CS->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert((CSIR->getNumOperands() == CallSiteMDSize) &&
      "Incorrect call site inline report metadata");
  LLVMContext &Ctx = CS->getParent()->getParent()->getParent()->getContext();
  std::string InlineCostStr = "inlineCost: ";
  InlineCostStr.append(std::to_string(IC.getCost()));
  auto InlineCostMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, InlineCostStr));
  CSIR->replaceOperandWith(CSMDIR_InlineCost, InlineCostMD);
  std::string InlineThresholdStr = "inlineThreshold: ";
  InlineThresholdStr.append(std::to_string(IC.getCost() + IC.getCostDelta()));
  auto InlineThresholdMD =
      MDNode::get(Ctx, llvm::MDString::get(Ctx, InlineThresholdStr));
  CSIR->replaceOperandWith(CSMDIR_InlineThreshold, InlineThresholdMD);
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
  auto *FMD = F->getMetadata(FunctionTag);
  // Update function linkage char
  auto *FIR = dyn_cast<MDTuple>(FMD);
  assert(FIR && "Bad function inline report format");
  LLVMContext &Ctx = F->getParent()->getContext();
  // Op 7: linkage string
  std::string LinkageStr = "linkage: ";
  LinkageStr.append(llvm::getLinkageStr(F));
  auto LinkageMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, LinkageStr));
  FIR->replaceOperandWith(FMDIR_LinkageStr, LinkageMD);
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
  auto IsDeadMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, IsDeadStr));
  FIR->replaceOperandWith(FMDIR_IsDead, IsDeadMD);
}

// This function copies simple metadata node and recursively calls itself to
// copy metadata list node. It also creates a map between original and copied
// metadata nodes. \pLeafMDIR is a set of not-inlined call site nodes which have
// no children - those we expect to see in the function after inlining.
static Metadata *
cloneInliningReportHelper(LLVMContext &C, Metadata *OldMD,
                          DenseMap<Metadata *, Metadata *> &MDMap,
                          std::set<MDTuple *> &LeafMDIR,
                          Metadata *CurrentCallInstReport) {
  if (!OldMD)
    return nullptr;
  Metadata *NewMD = nullptr;
  if (MDString *OldStrMD = dyn_cast<MDString>(OldMD))
    NewMD = llvm::MDString::get(C, OldStrMD->getString());
  else if (OldMD == CurrentCallInstReport) {
    // It means that we have a recursive inlining call. Make a default inline
    // report out of the stored inlining report since call instruction is
    // already invalid.
    CallSiteInliningReport OldRep(cast<MDTuple>(CurrentCallInstReport));
    unsigned LineNum = 0, ColNum = 0;
    OldRep.getLineAndCol(&LineNum, &ColNum);
    CallSiteInliningReport *NewRep = new CallSiteInliningReport(
        &C, std::string(OldRep.getName()), nullptr, NinlrNoReason, false,
        OldRep.getSuppressPrint(),
        -1, -1, -1,
        INT_MAX, INT_MAX, LineNum, ColNum, std::string(OldRep.getModuleName()));
    NewMD = NewRep->get();
  } else if (MDTuple *OldTupleMD = dyn_cast<MDTuple>(OldMD)) {
    SmallVector<Metadata *, 20> Ops;
    int NumOps = OldTupleMD->getNumOperands();
    for (int I = 0; I < NumOps; ++I)
      Ops.push_back(cloneInliningReportHelper(C, OldTupleMD->getOperand(I),
                                              MDMap, LeafMDIR,
                                              CurrentCallInstReport));
    MDTuple *NewTupleMD = nullptr;
    if (OldTupleMD->isDistinct())
      NewTupleMD = MDTuple::getDistinct(C, Ops);
    else
      NewTupleMD = MDTuple::get(C, Ops);

    if (NumOps == CallSiteMDSize) {
      int64_t IsInlined = 0;
      getOpVal(OldTupleMD->getOperand(CSMDIR_IsInlined),
               "isInlined: ", &IsInlined);
      if (!IsInlined && OldTupleMD->getOperand(CSMDIR_CSs) == nullptr) {
        LeafMDIR.insert(NewTupleMD);
      }
    }
    NewMD = NewTupleMD;
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
  std::set<MDTuple *> LeafMDIR;
  LLVMContext &C = F->getParent()->getContext();
  Metadata *NewFuncMD = cloneInliningReportHelper(C, FuncMD, MDMap, LeafMDIR,
                                                  CurrentCallInstReport);
  if (!NewFuncMD)
    return nullptr;

  for (ValueToValueMapTy::iterator InstIt = VMap.begin(),
           InstItEnd = VMap.end();
       InstIt != InstItEnd; InstIt++) {
    Metadata *OldMetadata = nullptr;
    if (!InstIt->first) {
      // It means that we have a recursive inlining call. Make a default inline
      // report out of the stored inlining report since call instruction is
      // already invalid.
      OldMetadata = CurrentCallInstReport;
    } else {
      const Instruction *OldI = dyn_cast<Instruction>(InstIt->first);
      if (!OldI || !isa<CallBase>(OldI))
        continue;
      OldMetadata = OldI->getMetadata(CallSiteTag);
    }
    if (!OldMetadata || !InstIt->second)
      continue;
    Instruction *NewI = dyn_cast_or_null<Instruction>(InstIt->second);
    if (!NewI)
      continue;
    MDTuple *NewMetadata = dyn_cast_or_null<MDTuple>(MDMap[OldMetadata]);
    if (!NewMetadata)
      continue;
    NewI->setMetadata(CallSiteTag, NewMetadata);
    std::set<MDTuple *>::iterator It = LeafMDIR.find(NewMetadata);
    if (It != LeafMDIR.end())
      LeafMDIR.erase(It);
    addCallback(NewI, NewMetadata);
  }

  // All leaf callsites which we expected to appear in the inlined code, but
  // were not actually seen, should be marked 'deleted'. As they were optimized
  // out due to internal inliner code optimization.
  std::set<MDTuple *>::iterator It, ItEnd = LeafMDIR.end();
  for (It = LeafMDIR.begin(); It != ItEnd; ++It) {
    auto *CSIR = dyn_cast<MDTuple>(*It);
    assert((CSIR->getNumOperands() == CallSiteMDSize) &&
        "Incorrect call site inline report metadata");
    LLVMContext &Ctx = CSIR->getContext();
    std::string ReasonStr = "reason: ";
    ReasonStr.append(std::to_string(NinlrDeleted));
    auto ReasonMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, ReasonStr));
    CSIR->replaceOperandWith(CSMDIR_InlineReason, ReasonMD);
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
    // deleted value. Putting it in the IIMap could cause a ValueHandle
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
  auto IsInlinedMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, IsInlinedStr));
  OldCallSiteIR->replaceOperandWith(CSMDIR_IsInlined, IsInlinedMD);
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
  std::string FuncName = std::string(NewFunction->getName());
  FuncName.insert(0, "name: ");
  auto FuncNameMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, FuncName));
  OldFIR->replaceOperandWith(FMDIR_FuncName, FuncNameMD);
  // Op 7: linkage string
  std::string LinkageStr = "linkage: ";
  LinkageStr.append(llvm::getLinkageStr(NewFunction));
  auto LinkageMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, LinkageStr));
  OldFIR->replaceOperandWith(FMDIR_LinkageStr, LinkageMD);
  NewFunction->setMetadata(FunctionTag, OldFIR);
  addCallback(NewFunction, OldFIR);
}

