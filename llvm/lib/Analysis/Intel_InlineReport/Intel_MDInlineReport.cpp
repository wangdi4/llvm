//===--- Intel_MDInlineReport.cpp  --Inlining report vis metadata --------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
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
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace MDInliningReport;

#define DEBUG_TYPE "mdinlinereport"

static cl::opt<bool>
    DumpFunctionNameIndexMap("dump-function-name-index-map", cl::Hidden,
                             cl::init(0), cl::Optional,
                             cl::desc("Dump function name index map"));

std::string llvm::getLinkageStr(Function *F) {
  std::string LinkageChar =
      (F->hasLocalLinkage()
           ? "L"
           : (F->hasLinkOnceODRLinkage()
                  ? "O"
                  : (F->hasAvailableExternallyLinkage() ? "X" : "A")));
  return LinkageChar;
}

std::string llvm::getLanguageStr(Function *F) {
  std::string LanguageChar = F->isFortran() ? "F" : "C";
  return LanguageChar;
}

//
// Constructor for function inlining report metadata and object.
// Ex.:
// !21 = distinct !{!"intel.function.inlining.report", !"name: a", !22,
//       !"name: test1.c", !"isDead: 0", !"isDeclaration: 0", !"linkage: A",
//       !"language: F", !"isSuppressPrint: 0"}
// !22 = distinct !{!"intel.callsites.inlining.report"}
FunctionInliningReport::FunctionInliningReport(
    LLVMContext *C, std::string FuncName, std::vector<MDTuple *> *CSs,
    std::string ModuleName, bool IsDead, bool IsDeclaration,
    bool isSuppressPrint, bool IsCompact, std::string LinkageStr,
    std::string LanguageStr) {
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
  // Op 7: language string
  LanguageStr.insert(0, "language: ");
  auto LanguageMD = MDNode::get(*C, llvm::MDString::get(*C, LanguageStr));
  Ops.push_back(LanguageMD);
  // Op 8: isSuppressPrint
  std::string IsSuppressPrintStr = "isSuppressPrint: ";
  if (isSuppressPrint) {
    LLVM_DEBUG(dbgs() << "isSuppressPrint is ON\n";);
    setSuppressPrint(true);
  }
  IsSuppressPrintStr.append(std::to_string(isSuppressPrint));
  auto IsSuppressPrintMD =
      MDNode::get(*C, llvm::MDString::get(*C, IsSuppressPrintStr));
  Ops.push_back(IsSuppressPrintMD);
  // Op 9: is compact?
  std::string IsCompactStr = "isCompact: ";
  IsCompactStr.append(std::to_string(IsCompact));
  auto IsCompactMD = MDNode::get(*C, llvm::MDString::get(*C, IsCompactStr));
  Ops.push_back(IsCompactMD);
  // Op 10: List of compact inline indexes (starts out null)
  Ops.push_back(nullptr);
  // Op 11: List of compact inline counts (starts out null)
  Ops.push_back(nullptr);
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
    InlineReason Reason, bool IsInlined, bool IsSuppressPrint, int InlineCost,
    int OuterInlineCost, int InlineThreshold, int EarlyExitInlineCost,
    int EarlyExitInlineThreshold, bool IsCostBenefit, int CBPairCost,
    int CBPairBenefit, InlICSType ICSMethod, bool IsCompact, unsigned Line,
    unsigned Col, std::string ModuleName) {
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
  // Op 13: has cost benefit?
  std::string IsCostBenefitStr = "isCostBenefit: ";
  IsCostBenefitStr.append(std::to_string(IsCostBenefit));
  auto IsCostBenefitMD =
      MDNode::get(*C, llvm::MDString::get(*C, IsCostBenefitStr));
  Ops.push_back(IsCostBenefitMD);
  // Op 14: CBPairCost
  std::string CBPairCostStr = "CBPairCost: ";
  CBPairCostStr.append(std::to_string(CBPairCost));
  auto CBPairCostMD = MDNode::get(*C, llvm::MDString::get(*C, CBPairCostStr));
  Ops.push_back(CBPairCostMD);
  // Op 15: CBPairBenefit
  std::string CBPairBenefitStr = "CBPairBenefit: ";
  CBPairBenefitStr.append(std::to_string(CBPairBenefit));
  auto CBPairBenefitMD =
      MDNode::get(*C, llvm::MDString::get(*C, CBPairBenefitStr));
  Ops.push_back(CBPairBenefitMD);
  // Op 16: icsMethod
  std::string ICSMethodStr = "icsMethod: ";
  ICSMethodStr.append(std::to_string(ICSMethod));
  auto ICSMethodMD = MDNode::get(*C, llvm::MDString::get(*C, ICSMethodStr));
  Ops.push_back(ICSMethodMD);
  // Op 17: isCompact
  std::string IsCompactStr = "isCompact: ";
  IsCompactStr.append(std::to_string(IsCompact));
  auto IsCompactMD = MDNode::get(*C, llvm::MDString::get(*C, IsCompactStr));
  Ops.push_back(IsCompactMD);
  // Op 18: broker target name
  Name.insert(0, "name: ");
  auto BTNameMD = MDNode::get(*C, llvm::MDString::get(*C, ""));
  Ops.push_back(BTNameMD);
  return MDTuple::getDistinct(*C, Ops);
}

CallSiteInliningReport::CallSiteInliningReport(
    LLVMContext *C, std::string Name, std::vector<MDTuple *> *CSs,
    InlineReason Reason, bool IsInlined, bool IsSuppressPrint, int InlineCost,
    int OuterInlineCost, int InlineThreshold, int EarlyExitInlineCost,
    int EarlyExitInlineThreshold, bool IsCostBenefit, int CBPairCost,
    int CBPairBenefit, InlICSType ICSMethod, bool IsCompact, unsigned Line,
    unsigned Col, std::string ModuleName) {
  Report =
      initCallSite(C, Name, CSs, Reason, IsInlined, IsSuppressPrint, InlineCost,
                   OuterInlineCost, InlineThreshold, EarlyExitInlineCost,
                   EarlyExitInlineThreshold, IsCostBenefit, CBPairCost,
                   CBPairBenefit, ICSMethod, IsCompact, Line, Col, ModuleName);
}

CallSiteInliningReport::CallSiteInliningReport(
    CallBase *MainCB, std::vector<MDTuple *> *CSs, InlineReason Reason,
    bool IsInlined, bool IsSuppressPrint, int InlineCost, int OuterInlineCost,
    int InlineThreshold, int EarlyExitInlineCost, int EarlyExitInlineThreshold,
    bool IsCostBenefit, int CBPairCost, int CBPairBenefit,
    InlICSType ICSMethod, bool IsCompact) {
  Function *Callee = MainCB->getCalledFunction();
  std::string Name =
      Callee ? std::string(Callee->hasName() ? Callee->getName() : "") : "";
  const DebugLoc &DL = MainCB->getDebugLoc();
  StringRef ModuleName =
      MainCB->getParent()->getParent()->getParent()->getName();
  Report = initCallSite(
      &(MainCB->getParent()->getParent()->getContext()), Name, CSs, Reason,
      IsInlined,
      MainCB->getMetadata(IPOUtils::getSuppressInlineReportStringRef()),
      InlineCost, OuterInlineCost, InlineThreshold, EarlyExitInlineCost,
      EarlyExitInlineThreshold, IsCostBenefit, CBPairCost, CBPairBenefit,
      ICSMethod, IsCompact, (DL ? DL.getLine() : 0), (DL ? DL.getCol() : 0),
      ModuleName.str());
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

void CallSiteInliningReport::initReason(CallBase *CB) {
  Function *Callee = CB->getCalledFunction();
  if (Callee) {
    if (Callee->isDeclaration()) {
      if (Callee->isIntrinsic())
        llvm::setMDReasonNotInlined(CB, NinlrIntrinsic);
      else
        llvm::setMDReasonNotInlined(CB, NinlrExtern);
    } else {
      llvm::setMDReasonNotInlined(CB, NinlrNewlyCreated);
    }
  } else {
    llvm::setMDReasonNotInlined(CB, NinlrIndirect);
  }
}

void InlineReportBuilder::addCallSite(CallBase *CB) {
  if (!isMDIREnabled())
    return;
  CallSiteInliningReport CSIR(CB, nullptr, NinlrNoReason);
  Function *Caller = CB->getCaller();
  Function *Callee = CB->getCalledFunction();
  CSIR.initReason(CB);
  std::string FuncName = std::string(Callee ? Callee->getName() : "");
  FuncName.insert(0, "name: ");
  CB->setMetadata(CallSiteTag, CSIR.get());
  LLVMContext &Ctx = CB->getFunction()->getParent()->getContext();
  auto FuncNameMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, FuncName));
  CSIR.get()->replaceOperandWith(CSMDIR_CalleeName, FuncNameMD);
  // Recreate the call site list for the caller, making the new call site
  // the last call in the list.
  SmallVector<Metadata *, 100> Ops;
  Ops.push_back(llvm::MDString::get(Ctx, CallSitesTag));
  Metadata *CallerMD = Caller->getMetadata(FunctionTag);
  if (!CallerMD)
    return;
  auto *CallerMDTuple = cast<MDTuple>(CallerMD);
  if (Metadata *MDCSs = CallerMDTuple->getOperand(FMDIR_CSs).get()) {
    auto CSs = cast<MDTuple>(MDCSs);
    unsigned CSsNumOps = CSs->getNumOperands();
    for (unsigned I = 1; I < CSsNumOps; ++I)
      Ops.push_back(CSs->getOperand(I));
  }
  Ops.push_back(CSIR.get());
  MDNode *NewCSs = MDTuple::getDistinct(Ctx, Ops);
  CallerMDTuple->replaceOperandWith(FMDIR_CSs, NewCSs);
  addCallback(CB);
}

void InlineReportBuilder::addFunction(Function *F) {
  if (!isMDIREnabled())
    return;
  std::vector<MDTuple *> CSs;
  FunctionInliningReport FIR(F, &CSs, /*isDead=*/false, getLevel() & Compact);
  addCallback(F);
  initFunctionTemps(F);
  Module *M = F->getParent();
  NamedMDNode *ModuleInlineReport = M->getOrInsertNamedMetadata(ModuleTag);
  ModuleInlineReport->addOperand(FIR.get());
  F->setMetadata(FunctionTag, FIR.get());
}

void InlineReportBuilder::addMultiversionedCallSite(CallBase *CB) {
  if (!isMDIREnabled())
    return;
  addCallSite(CB);
  llvm::setMDReasonNotInlined(CB, NinlrMultiversionedCallsite);
}

void InlineReportBuilder::deleteFunctionBody(Function *F) {
  if (!isMDIREnabled())
    return;
  Module *M = F->getParent();
  NamedMDNode *ModuleInlineReport = M->getOrInsertNamedMetadata(ModuleTag);
  MDTuple *FIR = nullptr;
  for (unsigned I = 0, E = ModuleInlineReport->getNumOperands(); I < E; ++I) {
    MDNode *Node = ModuleInlineReport->getOperand(I);
    MDTuple *FuncReport = cast<MDTuple>(Node);
    StringRef MDSR = getOpStr(FuncReport->getOperand(FMDIR_FuncName), "name: ");
    if (F->getName() == MDSR) {
      FIR = FuncReport;
      break;
    }
  }
  assert(FIR);
  FIR->replaceOperandWith(FMDIR_CSs, nullptr);
  F->setMetadata(FunctionTag, FIR);
}

void llvm::setMDReasonNotInlined(CallBase *Call, const InlineCost &IC) {
  Metadata *CSMD = Call->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  InlineReason Reason = IC.getInlineReason();
  llvm::setMDReasonNotInlined(Call, Reason);
  assert(IsNotInlinedReason(Reason));
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert((CSIR && CSIR->getNumOperands() == CallSiteMDSize) &&
         "Incorrect call site inline report metadata");
  LLVMContext &Ctx = Call->getContext();
  if (IC.getCostBenefit()) {
    CostBenefitPair CBP = *IC.getCostBenefit();
    std::string CBPairCostStr = "CBPairCost: ";
    CBPairCostStr.append(
        std::to_string(CBP.getCost().getLimitedValue(INT64_MAX)));
    auto CBPairCostMD =
        MDNode::get(Ctx, llvm::MDString::get(Ctx, CBPairCostStr));
    CSIR->replaceOperandWith(CSMDIR_CBPairCost, CBPairCostMD);
    std::string CBPairBenefitStr = "CBPairBenefit: ";
    CBPairBenefitStr.append(
        std::to_string(CBP.getBenefit().getLimitedValue(INT64_MAX)));
    auto CBPairBenefitMD =
        MDNode::get(Ctx, llvm::MDString::get(Ctx, CBPairBenefitStr));
    CSIR->replaceOperandWith(CSMDIR_CBPairBenefit, CBPairBenefitMD);
    return;
  }
  if (IC.isNever())
    return;
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

void llvm::setMDReasonNotInlined(CallBase *Call, InlineReason Reason) {
  Metadata *CSMD = Call->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  assert(IsNotInlinedReason(Reason));
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert((CSIR && CSIR->getNumOperands() == CallSiteMDSize) &&
         "Incorrect call site inline report metadata");
  std::string ReasonStr = "reason: ";
  int64_t OldReason = 0;
  getOpVal(CSIR->getOperand(CSMDIR_InlineReason), ReasonStr, &OldReason);
  if (Reason == NinlrNotAlwaysInline &&
      IsNotInlinedReason((InlineReason)OldReason))
    return;
  LLVMContext &Ctx = Call->getContext();
  ReasonStr.append(std::to_string(Reason));
  auto ReasonMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, ReasonStr));
  CSIR->replaceOperandWith(CSMDIR_InlineReason, ReasonMD);
}

void llvm::setMDReasonNotInlined(CallBase *Call, const InlineCost &IC,
                                 int TotalSecondaryCost) {
  Metadata *CSMD = Call->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  assert(IC.getInlineReason() == NinlrOuterInlining);
  llvm::setMDReasonNotInlined(Call, IC);
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert((CSIR && CSIR->getNumOperands() == CallSiteMDSize) &&
         "Incorrect call site inline report metadata");
  LLVMContext &Ctx = Call->getContext();
  std::string OuterInlineCostStr = "outerInlineCost: ";
  OuterInlineCostStr.append(std::to_string(TotalSecondaryCost));
  auto OuterInlineCostMD =
      MDNode::get(Ctx, llvm::MDString::get(Ctx, OuterInlineCostStr));
  CSIR->replaceOperandWith(CSMDIR_OuterInlineCost, OuterInlineCostMD);
}

// Set inlining reason
void llvm::setMDReasonIsInlined(CallBase *Call, InlineReason Reason) {
  Metadata *CSMD = Call->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  assert(IsInlinedReason(Reason));
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert((CSIR && CSIR->getNumOperands() == CallSiteMDSize) &&
         "Incorrect call site inline report metadata");
  LLVMContext &Ctx = Call->getContext();
  std::string ReasonStr = "reason: ";
  ReasonStr.append(std::to_string(Reason));
  auto ReasonMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, ReasonStr));
  CSIR->replaceOperandWith(CSMDIR_InlineReason, ReasonMD);
}

// Set inlining reason
void llvm::setMDReasonIsInlined(CallBase *Call, const InlineCost &IC) {
  Metadata *CSMD = Call->getMetadata(CallSiteTag);
  if (!CSMD)
    return;
  InlineReason Reason = IC.getInlineReason();
  assert(IsInlinedReason(Reason));
  llvm::setMDReasonIsInlined(Call, Reason);
  auto *CSIR = dyn_cast<MDTuple>(CSMD);
  assert((CSIR && CSIR->getNumOperands() == CallSiteMDSize) &&
         "Incorrect call site inline report metadata");
  LLVMContext &Ctx = Call->getContext();
  if (IC.getCostBenefit()) {
    std::string IsCostBenefitStr = "isCostBenefit: ";
    IsCostBenefitStr.append(std::to_string(true));
    auto IsCostBenefitMD =
        MDNode::get(Ctx, llvm::MDString::get(Ctx, IsCostBenefitStr));
    CSIR->replaceOperandWith(CSMDIR_IsCostBenefit, IsCostBenefitMD);
    CostBenefitPair CBP = *IC.getCostBenefit();
    std::string CBPairCostStr = "CBPairCost: ";
    CBPairCostStr.append(std::to_string(CBP.getCost().getLimitedValue()));
    auto CBPairCostMD =
        MDNode::get(Ctx, llvm::MDString::get(Ctx, CBPairCostStr));
    CSIR->replaceOperandWith(CSMDIR_CBPairCost, CBPairCostMD);
    std::string CBPairBenefitStr = "CBPairBenefit: ";
    CBPairBenefitStr.append(std::to_string(CBP.getBenefit().getLimitedValue()));
    auto CBPairBenefitMD =
        MDNode::get(Ctx, llvm::MDString::get(Ctx, CBPairBenefitStr));
    CSIR->replaceOperandWith(CSMDIR_CBPairBenefit, CBPairBenefitMD);
    return;
  }
  if (IC.isAlways())
    return;
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

bool InlineReportBuilder::shouldCompactCallBase(Function *Caller,
                                                Function *Callee,
                                                bool ForceCompact) {
  if (!hasFunctionMetadata(Caller, Callee))
    return false;
  unsigned CalleeIndex = getFunctionIndex(Callee);
  Module *M = Caller->getParent();
  NamedMDNode *ModuleInlineReport = M->getOrInsertNamedMetadata(ModuleTag);
  MDNode *Node = ModuleInlineReport->getOperand(CalleeIndex);
  MDTuple *FuncReport = cast<MDTuple>(Node);
  int64_t IsCompact = 0;
  getOpVal(FuncReport->getOperand(FMDIR_IsCompact), "isCompact: ", &IsCompact);
  if (IsCompact)
    return false;
  if (ForceCompact)
    return true;
  if (InlineCount[CalleeIndex] > IntelInlineReportCompactThreshold)
    return true;
  return false;
}

void InlineReportBuilder::addCompactInlinedCallBase(unsigned CallerIndex, 
                                                    unsigned CalleeIndex,
                                                    unsigned Count) {
  auto MV = TotalInlines[CallerIndex];
  auto MapIt = MV->find(CalleeIndex);
  if (MapIt != MV->end()) {
    MapIt->second += Count;
    return;
  }
  MV->insert({CalleeIndex, Count});
}

void InlineReportBuilder::addForCompactInlinedCallBase(unsigned CallerIndex,
                                                       unsigned CalleeIndex,
                                                       unsigned Count) {
  auto MV = Inlines[CallerIndex];
  auto MapIt = MV->find(CalleeIndex);
    if (MapIt != MV->end()) {
    MapIt->second += Count;
    return;
  }
  MV->insert({CalleeIndex, Count});
}

void InlineReportBuilder::compactChildren(Function *F,
                                          MDTuple *MDTupleCS) {
  Module *M = F->getParent();
  unsigned CallerIndex = getFunctionIndex(F);
  if (Metadata *MDCSs = MDTupleCS->getOperand(CSMDIR_CSs).get()) {
    auto CSs = cast<MDTuple>(MDCSs);
    for (unsigned I = 1, E = CSs->getNumOperands(); I < E; ++I) {
      MDTuple *TMD = dyn_cast<MDTuple>(CSs->getOperand(I).get());
      int64_t IsInlined = 0;
      getOpVal(TMD->getOperand(CSMDIR_IsInlined), "isInlined: ", &IsInlined);
      if (IsInlined) {
        StringRef MDSR = getOpStr(TMD->getOperand(CSMDIR_CalleeName), "name: ");
        unsigned CalleeIndex = getFunctionIndexByName(M, MDSR);
        addCompactInlinedCallBase(CallerIndex, CalleeIndex);
        compactChildren(F, TMD);
      }
    }
  }
}

void InlineReportBuilder::inheritCompactCallBases(Function *Caller,
                                                  Function *Callee) {
  unsigned CallerIndex = getFunctionIndex(Caller);
  auto MV = TotalInlines[getFunctionIndex(Callee)];
  for (auto &InlinedPair : *MV) {
    addForCompactInlinedCallBase(CallerIndex, InlinedPair.first,
                                 InlinedPair.second);
    if (getIsCompact(Caller))
      addCompactInlinedCallBase(CallerIndex, InlinedPair.first,
                                InlinedPair.second);
  }
}

void InlineReportBuilder::compact(Function *F) {
  if (DumpFunctionNameIndexMap)
    dumpFunctionNameIndexMap(F);
  Module *M = F->getParent();
  unsigned CallerIndex = getFunctionIndex(F);
  Metadata *MDF = F->getMetadata(FunctionTag);
  auto *MDTupleF = cast<MDTuple>(MDF);
  if (Metadata *MDCSs = MDTupleF->getOperand(FMDIR_CSs).get()) {
    auto CSs = cast<MDTuple>(MDCSs);
    for (unsigned I = 1, E = CSs->getNumOperands(); I < E; ++I) {
      MDTuple *TMD = dyn_cast<MDTuple>(CSs->getOperand(I).get());
      int64_t IsInlined = 0;
      getOpVal(TMD->getOperand(CSMDIR_IsInlined), "isInlined: ", &IsInlined);
      if (IsInlined) {
        StringRef MDSR = getOpStr(TMD->getOperand(CSMDIR_CalleeName), "name: ");
        unsigned CalleeIndex = getFunctionIndexByName(M, MDSR);
        addCompactInlinedCallBase(CallerIndex, CalleeIndex);
        compactChildren(F, TMD);
      }
    }
  }
  setIsCompact(F, true);
}

bool InlineReportBuilder::getIsCompact(Function *F) {
  int64_t IsCompact = 0;
  auto FIR = cast<MDTuple>(F->getMetadata(FunctionTag));
  getOpVal(FIR->getOperand(FMDIR_IsCompact), "isCompact: ", &IsCompact);
  return IsCompact;
}

void InlineReportBuilder::setIsCompact(Function *F, bool Value) {
  Module *M = F->getParent();
  LLVMContext &Ctx = M->getContext();
  std::string IsCompactStr = "isCompact: ";
  IsCompactStr.append(std::to_string(Value));
  auto IsCompactMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, IsCompactStr));
  auto FMD = F->getMetadata(FunctionTag);
  if (!FMD)
    return;
  auto FIR = cast<MDTuple>(FMD);
  FIR->replaceOperandWith(FMDIR_IsCompact, IsCompactMD);
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

  LLVM_DEBUG(dbgs() << "MDIR inline: begin function " << F->getName() << "\n");
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
  // Op 8: language string
  std::string LanguageStr = "language: ";
  LanguageStr.append(llvm::getLanguageStr(F));
  auto LanguageMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, LanguageStr));
  FIR->replaceOperandWith(FMDIR_LanguageStr, LanguageMD);
  // Add callbacks to the Function and CallBases if they do not already have
  // them.
  addCallback(F);
  for (auto &I : instructions(*F))
    if (auto CB = dyn_cast<CallBase>(&I))
      if (CB->getMetadata(CallSiteTag))
        addCallback(CB);
}

void InlineReportBuilder::beginSCC(LazyCallGraph::SCC &SCC) {
  if (!isMDIREnabled())
    return;
  LazyCallGraph::Node &LCGN = *(SCC.begin());
  Module *M = LCGN.getFunction().getParent();
  NamedMDNode *ModuleInlineReport = M->getOrInsertNamedMetadata(ModuleTag);
  if (ModuleInlineReport->getNumOperands() == 0)
    return;
  for (auto &Node : SCC) {
    Function *F = &Node.getFunction();
    beginFunction(F);
  }
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
  FunctionIndexMap.erase(F);
}

bool InlineReportBuilder::getIsCompact(Metadata *CurrentCallInstReport) {
  int64_t IsCompact = 0;
  auto CSIR = cast<MDTuple>(CurrentCallInstReport);
  getOpVal(CSIR->getOperand(CSMDIR_IsCompact), "isCompact: ", &IsCompact);
  return IsCompact;
}

bool InlineReportBuilder::hasFunctionMetadata(Function *Caller,
                                              Function *Callee) {
  if (!Caller->getMetadata(FunctionTag))
    return false;
  if (!Callee->getMetadata(FunctionTag))
    return false;
  return true;
}

void InlineReportBuilder::setIsCompact(Metadata *CurrentCallInstReport,
                                       bool Value) {
  auto CSIR = cast<MDTuple>(CurrentCallInstReport);
  LLVMContext &Ctx = CSIR->getContext();
  std::string IsCompactStr = "isCompact: ";
  IsCompactStr.append(std::to_string(Value));
  auto IsCompactMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, IsCompactStr));
  CSIR->replaceOperandWith(CSMDIR_IsCompact, IsCompactMD);
}

// This function copies simple metadata node and recursively calls itself to
// copy metadata list node. It also creates a map between original and copied
// metadata nodes. \pLeafMDIR is a set of not-inlined call site nodes which have
// no children - those we expect to see in the function after inlining.

// CMPLRLLVM-43269: We need to keep track of the type of metadata nodes we are
// traversing through, so that if we need to access an operand, we are sure
// that we are getting the right type of metadata.
enum MDDescentType {
  MDDT_Terminal,    // A MDDT_Func or MDDT_CallSite with no children.
  MDDT_Func,        // Represents a Function
  MDDT_CallSite,    // Represents a CallSite
  MDDT_CallSiteList // Represents a list of CallSites
};
static Metadata *cloneInliningReportHelper(
    LLVMContext &C, Metadata *OldMD, DenseMap<Metadata *, Metadata *> &MDMap,
    std::set<MDTuple *> &LeafMDIR, Metadata *CurrentCallInstReport,
    MDDescentType MDDType) {
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
    CallSiteInliningReport NewRep(
        &C, std::string(OldRep.getName()), nullptr, NinlrNoReason, false,
        OldRep.getSuppressPrint(), -1, -1, -1, INT_MAX, INT_MAX, false, -1, -1,
        InlICSNone, false, LineNum, ColNum,
        std::string(OldRep.getModuleName()));
    NewMD = NewRep.get();
  } else if (MDTuple *OldTupleMD = dyn_cast<MDTuple>(OldMD)) {
    SmallVector<Metadata *, 20> Ops;
    int NumOps = OldTupleMD->getNumOperands();
    for (int I = 0; I < NumOps; ++I) {
      // Compute the metadata type of the operand
      MDDescentType NewMDDType = MDDT_Terminal;
      if (MDDType == MDDT_Func && I == FMDIR_CSs ||
          MDDType == MDDT_CallSite && I == FMDIR_CSs)
        NewMDDType = MDDT_CallSiteList;
      else if (MDDType == MDDT_CallSiteList)
        NewMDDType = MDDT_CallSite;
      Ops.push_back(cloneInliningReportHelper(
          C, OldTupleMD->getOperand(I), MDMap, LeafMDIR, CurrentCallInstReport,
          NewMDDType));
    }
    MDTuple *NewTupleMD = nullptr;
    if (OldTupleMD->isDistinct())
      NewTupleMD = MDTuple::getDistinct(C, Ops);
    else
      NewTupleMD = MDTuple::get(C, Ops);

    // Here is a place where we need to test for the metadata type.
    if (MDDType == MDDT_CallSite) {
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
  Metadata *NewFuncMD = cloneInliningReportHelper(
      C, FuncMD, MDMap, LeafMDIR, CurrentCallInstReport, MDDT_Func);
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
    addCallback(NewI);
  }

  // All leaf callsites which we expected to appear in the inlined code, but
  // were not actually seen, should be marked 'deleted'. As they were optimized
  // out due to internal inliner code optimization.
  std::set<MDTuple *>::iterator It, ItEnd = LeafMDIR.end();
  for (It = LeafMDIR.begin(); It != ItEnd; ++It) {
    auto *CSIR = dyn_cast<MDTuple>(*It);
    assert((CSIR && CSIR->getNumOperands() == CallSiteMDSize) &&
           "Incorrect call site inline report metadata");
    LLVMContext &Ctx = CSIR->getContext();
    std::string ReasonStr = "reason: ";
    ReasonStr.append(std::to_string(NinlrDeleted));
    auto ReasonMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, ReasonStr));
    CSIR->replaceOperandWith(CSMDIR_InlineReason, ReasonMD);
  }
  return NewFuncMD;
}

Metadata *InlineReportBuilder::cloneCompactCS(LLVMContext &C,
                                              ValueToValueMapTy &VMap) {
  SmallVector<Metadata *, 20> Ops;
  Ops.push_back(llvm::MDString::get(C, CallSitesTag));
  const char *const CSKind = MDInliningReport::CallSiteTag;
  for (const auto &Pair : VMap) {
    auto CB0 = dyn_cast<CallBase>(Pair.first);
    auto CB1 = dyn_cast<CallBase>(Pair.second);
    if (!CB0 || !CB1)
      continue;
    auto CB0CIR = dyn_cast_or_null<MDTuple>(CB0->getMetadata(CSKind));
    if (!CB0CIR)
      continue;
    CallSiteInliningReport OldRep(CB0CIR);
    unsigned LineNum = 0, ColNum = 0;
    OldRep.getLineAndCol(&LineNum, &ColNum);
    CallSiteInliningReport NewRep(
        &C, std::string(OldRep.getName()), nullptr, NinlrNoReason, false,
        OldRep.getSuppressPrint(), -1, -1, -1, INT_MAX, INT_MAX, false, -1, -1,
        InlICSNone, false, LineNum, ColNum,
        std::string(OldRep.getModuleName()));
    Ops.push_back(NewRep.get());
    CB1->setMetadata(CSKind, NewRep.get());
  }
  return MDTuple::getDistinct(C, Ops);
}

Metadata *InlineReportBuilder::cloneInliningReportHelperCompact(
    LLVMContext &C, Metadata *OldMD, ValueToValueMapTy &VMap, bool IsTerminal) {
  Metadata *NewMD = nullptr;
  if (MDString *OldStrMD = dyn_cast<MDString>(OldMD))
    NewMD = llvm::MDString::get(C, OldStrMD->getString());
  else if (MDTuple *OldTupleMD = dyn_cast<MDTuple>(OldMD)) {
    SmallVector<Metadata *, 20> Ops;
    for (unsigned I = 0, E = OldTupleMD->getNumOperands(); I < E; ++I) {
      Metadata *MDT = nullptr;
      if (!IsTerminal && I == FMDIR_CSs) {
        Ops.push_back(cloneCompactCS(C, VMap));
      } else if (IsTerminal ||
                 (I != FMDIR_CompactIndexes && I != FMDIR_CompactCounts)) {
        Metadata *MDNodeT = OldTupleMD->getOperand(I);
        MDT = cloneInliningReportHelperCompact(C, MDNodeT, VMap, true);
        Ops.push_back(MDT);
      }
    }
    NewMD = OldTupleMD->isDistinct() ? MDTuple::getDistinct(C, Ops)
                                     : MDTuple::get(C, Ops);
  }
  return NewMD;
}

Metadata *InlineReportBuilder::cloneInliningReportCompact(
    Function *Caller, Function *Callee, ValueToValueMapTy &VMap) {
  inheritCompactCallBases(Caller, Callee);
  if (getIsSummarized(Callee))
    setIsCompact(CurrentCallInstReport, true);
  LLVMContext &C = Caller->getParent()->getContext();
  addCompactInlinedCallBase(getFunctionIndex(Caller), getFunctionIndex(Callee));
  Metadata *CallerMD = Caller->getMetadata(FunctionTag);
  auto CallerMDTuple = cast<MDTuple>(CallerMD);
  Metadata *OldCalleeMD = Callee->getMetadata(FunctionTag);
  Metadata *NewCalleeMD =
      cloneInliningReportHelperCompact(C, OldCalleeMD, VMap, false);
  auto MV = Inlines[getFunctionIndex(Caller)];
  if (MV->empty())
    return NewCalleeMD;
  SmallVector<Metadata *, 20> Ops0;
  SmallVector<Metadata *, 20> Ops1;
  for (const auto &Pair : *MV) {
    std::string IsIndexStr = "Index: ";
    IsIndexStr.append(std::to_string(Pair.first));
    auto IsIndexMD = MDNode::get(C, llvm::MDString::get(C, IsIndexStr));
    Ops0.push_back(IsIndexMD);
    std::string IsCountStr = "Count: ";
    IsCountStr.append(std::to_string(Pair.second));
    auto IsCountMD = MDNode::get(C, llvm::MDString::get(C, IsCountStr));
    Ops1.push_back(IsCountMD);
  }
  MDNode *NewCompactIndexes = MDTuple::getDistinct(C, Ops0);
  MDNode *NewCompactCounts = MDTuple::getDistinct(C, Ops1);
  CallerMDTuple->replaceOperandWith(FMDIR_CompactIndexes, NewCompactIndexes);
  CallerMDTuple->replaceOperandWith(FMDIR_CompactCounts, NewCompactCounts);
  return NewCalleeMD;
}

// Update inlining report of the inlined call site.
void InlineReportBuilder::inlineCallSite() {
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
  // Update the name of the callee. This must be done in case an indirect
  // call was converted to a direct call before inlining it.
  MDTuple *OldCallSiteIR = cast<MDTuple>(CurrentCallInstReport);
  StringRef CalleeName = CurrentCallee ? CurrentCallee->getName() : "";
  std::string FuncName = std::string(CalleeName);
  FuncName.insert(0, "name: ");
  LLVMContext &Ctx = CurrentCaller->getContext();
  auto FuncNameMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, FuncName));
  OldCallSiteIR->replaceOperandWith(CSMDIR_CalleeName, FuncNameMD);
  // Test if the callee should be compacted before inlining.
  if (shouldCompactCallBase(CurrentCaller, CurrentCallee, Level & Compact))
    compact(CurrentCallee);
  Metadata *NewMD = nullptr;
  bool hasFMD = hasFunctionMetadata(CurrentCaller, CurrentCallee);
  if (hasFMD && getIsCompact(CurrentCallee))
    NewMD = cloneInliningReportCompact(CurrentCaller, CurrentCallee, VMap);
  else
    NewMD = cloneInliningReport(CurrentCallee, VMap);
  if (!NewMD)
    return;
  MDTuple *NewCallSiteIR = cast<MDTuple>(NewMD);
  OldCallSiteIR->replaceOperandWith(CSMDIR_CSs,
                                    NewCallSiteIR->getOperand(CSMDIR_CSs));
  std::string IsInlinedStr = "isInlined: ";
  IsInlinedStr.append(std::to_string(true));
  auto IsInlinedMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, IsInlinedStr));
  OldCallSiteIR->replaceOperandWith(CSMDIR_IsInlined, IsInlinedMD);
  if (hasFMD) {
    unsigned CallerIndex = getFunctionIndex(CurrentCaller);
    unsigned CalleeIndex = getFunctionIndex(CurrentCallee);
    InlineCount[CallerIndex] += InlineCount[CalleeIndex] + 1;
  }
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
  assert(OldFunction->getName() == "" && NewFunction->getName() != "" &&
         "Expecting name of OldFunction taken by NewFunction");
  unsigned FI = getFunctionIndex(OldFunction);
  FunctionIndexMap.erase(OldFunction);
  FunctionIndexMap.insert({NewFunction, FI});
  // Use the LLVMContext from the OldFunction, as the one for the NewFunction
  // may not be set yet.
  LLVMContext &Ctx = OldFunction->getParent()->getContext();
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
  // Op 8: language string
  std::string LanguageStr = "language: ";
  LanguageStr.append(llvm::getLanguageStr(NewFunction));
  auto LanguageMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, LanguageStr));
  OldFIR->replaceOperandWith(FMDIR_LanguageStr, LanguageMD);
  NewFunction->setMetadata(FunctionTag, OldFIR);
  removeCallback(OldFunction);
  addCallback(NewFunction);
}

// Copy the call metadata from 'OldCall' to 'NewCall' and update the callbacks
void InlineReportBuilder::replaceCallBaseWithCallBase(CallBase *OldCall,
                                                      CallBase *NewCall,
                                                      bool UpdateReason) {
  if (!isMDIREnabled())
    return;
  if (OldCall == NewCall)
    return;
  Metadata *OldCallMD = OldCall->getMetadata(MDInliningReport::CallSiteTag);
  if (!OldCallMD)
    return;
  auto *OldCallMDIR = dyn_cast<MDTuple>(OldCallMD);
  if (!OldCallMDIR)
    return;
  if (shouldSkipCallBase(NewCall)) {
    removeCallback(OldCall);
    return;
  }
  assert(OldCall->getCaller() == NewCall->getCaller());
  // Steal the callsite metadata from OldCall, giving it to NewCall.
  NewCall->setMetadata(MDInliningReport::CallSiteTag, OldCallMDIR);
  // Update the metdata for NewCall to reflect its callee.
  Function *Callee = NewCall->getCalledFunction();
  std::string FuncName = std::string(Callee ? Callee->getName() : "");
  FuncName.insert(0, "name: ");
  LLVMContext &Ctx = OldCall->getFunction()->getParent()->getContext();
  auto FuncNameMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, FuncName));
  OldCallMDIR->replaceOperandWith(CSMDIR_CalleeName, FuncNameMD);
  if (UpdateReason) {
    InlineReason Reason = NinlrIndirect;
    if (auto Callee = NewCall->getCalledFunction()) {
      if (Callee->isDeclaration()) {
        if (Callee->isIntrinsic())
          Reason = NinlrIntrinsic;
        else
          Reason = NinlrExtern;
      } else {
        Reason = NinlrNewlyCreated;
      }
    }
    std::string ReasonStr = "reason: ";
    ReasonStr.append(std::to_string(Reason));
    auto ReasonMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, ReasonStr));
    OldCallMDIR->replaceOperandWith(CSMDIR_InlineReason, ReasonMD);
  }
  //  Add a callback for NewCall
  addCallback(NewCall);
  // Move the inline report builder from the old to the new callback
  // information if it is available
  copyAndUpdateIRBuilder(OldCall, NewCall);
  // Remove the callback to the old call
  removeCallback(OldCall);
}

Metadata *InlineReportBuilder::copyMD(LLVMContext &C, Metadata *OldMD) {
  if (!OldMD)
    return nullptr;
  Metadata *NewMD = nullptr;
  if (MDString *OldStrMD = dyn_cast<MDString>(OldMD)) {
    NewMD = llvm::MDString::get(C, OldStrMD->getString());
  } else if (MDTuple *OldTupleMD = dyn_cast<MDTuple>(OldMD)) {
    SmallVector<Metadata *, 20> Ops;
    int NumOps = OldTupleMD->getNumOperands();
    for (int I = 0; I < NumOps; ++I)
      Ops.push_back(copyMD(C, OldTupleMD->getOperand(I)));
    NewMD = OldTupleMD->isDistinct() ? MDTuple::getDistinct(C, Ops)
                                     : MDTuple::get(C, Ops);
  }
  return NewMD;
}

Metadata *
InlineReportBuilder::copyMDWithMap(LLVMContext &C, Metadata *OldMD,
                                   DenseMap<Metadata *, Metadata *> &MDMap) {
  if (!OldMD)
    return nullptr;
  Metadata *NewMD = nullptr;
  if (MDString *OldStrMD = dyn_cast<MDString>(OldMD)) {
    NewMD = llvm::MDString::get(C, OldStrMD->getString());
  } else if (MDTuple *OldTupleMD = dyn_cast<MDTuple>(OldMD)) {
    SmallVector<Metadata *, 20> Ops;
    int NumOps = OldTupleMD->getNumOperands();
    for (int I = 0; I < NumOps; ++I)
      Ops.push_back(copyMD(C, OldTupleMD->getOperand(I)));
    NewMD = OldTupleMD->isDistinct() ? MDTuple::getDistinct(C, Ops)
                                     : MDTuple::get(C, Ops);
    MDMap[OldMD] = NewMD;
  }
  return NewMD;
}

void InlineReportBuilder::cloneCallBaseToCallBase(CallBase *OldCall,
                                                  CallBase *NewCall) {
  if (!isMDIREnabled())
    return;
  if (OldCall == NewCall || shouldSkipCallBase(NewCall))
    return;
  Metadata *OldCallMD = OldCall->getMetadata(MDInliningReport::CallSiteTag);
  if (!OldCallMD)
    return;
  auto *OldCallMDIR = dyn_cast<MDTuple>(OldCallMD);
  if (!OldCallMDIR)
    return;
  assert(OldCall->getCaller() == NewCall->getCaller());
  Function *Caller = OldCall->getCaller();
  Metadata *CallerMD = Caller->getMetadata(FunctionTag);
  if (!CallerMD)
    return;
  // Copy the metadata from OldCall to NewCall.
  LLVMContext &Ctx = OldCall->getFunction()->getParent()->getContext();
  auto *NewCallMDIR = cast<MDTuple>(copyMD(Ctx, OldCallMDIR));
  // Update the metdata for NewCall to reflect its callee.
  Function *Callee = NewCall->getCalledFunction();
  std::string FuncName = std::string(Callee ? Callee->getName() : "");
  FuncName.insert(0, "name: ");
  auto FuncNameMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, FuncName));
  NewCallMDIR->replaceOperandWith(CSMDIR_CalleeName, FuncNameMD);
  NewCall->setMetadata(MDInliningReport::CallSiteTag, NewCallMDIR);
  // Update the list of callsites for the caller.
  auto *CallerMDTuple = cast<MDTuple>(CallerMD);
  SmallVector<Metadata *, 100> Ops;
  Metadata *MDCSs = CallerMDTuple->getOperand(FMDIR_CSs).get();
  auto CSs = cast<MDTuple>(MDCSs);
  for (unsigned I = 0, E = CSs->getNumOperands(); I < E; ++I)
    Ops.push_back(CSs->getOperand(I).get());
  Ops.push_back(NewCallMDIR);
  MDNode *NewCSs = MDTuple::getDistinct(Ctx, Ops);
  CallerMDTuple->replaceOperandWith(FMDIR_CSs, NewCSs);
  // Add a callback for the new call.
  addCallback(NewCall);
}

void InlineReportBuilder::setCalledFunction(CallBase *CB, Function *F) {
  if (!isMDIREnabled())
    return;
  Metadata *CallMD = CB->getMetadata(MDInliningReport::CallSiteTag);
  if (!CallMD)
    return;
  auto *CallMDIR = dyn_cast<MDTuple>(CallMD);
  if (!CallMDIR)
    return;
  LLVMContext &Ctx = CB->getFunction()->getParent()->getContext();
  std::string FuncName = std::string(F->getName());
  FuncName.insert(0, "name: ");
  auto FuncNameMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, FuncName));
  CallMDIR->replaceOperandWith(CSMDIR_CalleeName, FuncNameMD);
}

void InlineReportBuilder::cloneFunction(Function *OldFunction,
                                        Function *NewFunction,
                                        ValueToValueMapTy &VMap) {
  if (!isMDIREnabled())
    return;
  if (OldFunction == NewFunction)
    return;
  Metadata *OldFunctionMD = OldFunction->getMetadata(FunctionTag);
  if (!OldFunctionMD)
    return;
  auto *OldFunctionMDTuple = dyn_cast<MDTuple>(OldFunctionMD);
  if (!OldFunctionMDTuple)
    return;
  LLVMContext &Ctx = OldFunction->getParent()->getContext();
  Metadata *NewFunctionMD = copyMD(Ctx, OldFunctionMD);
  auto *NewFunctionMDTuple = cast<MDTuple>(NewFunctionMD);
  // Update the function name to correspond to NewFunction.
  std::string FuncName = std::string(NewFunction->getName());
  FuncName.insert(0, "name: ");
  auto FuncNameMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, FuncName));
  NewFunctionMDTuple->replaceOperandWith(FMDIR_FuncName, FuncNameMD);
  // Update the linkage string to correspond to NewFunction.
  std::string LinkageStr = "linkage: ";
  LinkageStr.append(llvm::getLinkageStr(NewFunction));
  auto LinkageMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, LinkageStr));
  NewFunctionMDTuple->replaceOperandWith(FMDIR_LinkageStr, LinkageMD);
  NewFunction->setMetadata(FunctionTag, NewFunctionMDTuple);
  // Add a callback for the clone.
  addCallback(NewFunction);
  // Update the clone's list of callsites.
  Module *M = OldFunction->getParent();
  NamedMDNode *ModuleInlineReport = M->getOrInsertNamedMetadata(ModuleTag);
  initFunctionTemps(NewFunction, M);
  ModuleInlineReport->addOperand(NewFunctionMDTuple);
  SmallVector<Metadata *, 100> Ops;
  SmallPtrSet<Metadata *, 32> CopiedMD;
  Ops.push_back(llvm::MDString::get(Ctx, CallSitesTag));
  DenseMap<Metadata *, Metadata *> MDMap;
  // Clone the metadata from 'OldFunction' for 'NewFunction'.
  if (Metadata *MDCSs = OldFunctionMDTuple->getOperand(FMDIR_CSs).get()) {
    auto CSs = cast<MDTuple>(MDCSs);
    for (unsigned I = 1, E = CSs->getNumOperands(); I < E; ++I) {
      Metadata *OldMD = CSs->getOperand(I).get();
      auto OldMDTuple = cast<MDTuple>(OldMD);
      auto NewMDTuple = cast<MDTuple>(copyMDWithMap(Ctx, OldMDTuple, MDMap));
      Ops.push_back(NewMDTuple);
    }
  }
  // Attach cloned metadata to newly created callsites.
  for (auto &I : instructions(OldFunction))
    if (auto CBOld = dyn_cast<CallBase>(&I))
      if (auto CBNew = dyn_cast_or_null<CallBase>(VMap[CBOld]))
        if (Metadata *CBOldMD =
                CBOld->getMetadata(MDInliningReport::CallSiteTag)) {
          auto MapIt = MDMap.find(CBOldMD);
          if (MapIt != MDMap.end()) {
            auto NewMDTuple = cast<MDTuple>(MapIt->second);
            CBNew->setMetadata(MDInliningReport::CallSiteTag, NewMDTuple);
            addCallback(CBNew);
          }
        }
  MDNode *NewCSs = MDTuple::getDistinct(Ctx, Ops);
  NewFunctionMDTuple->replaceOperandWith(FMDIR_CSs, NewCSs);
}

void InlineReportBuilder::doOutlining(Function *OldF, Function *OutF,
                                      CallBase *OutCB) {
  if (!isMDIREnabled())
    return;
  Metadata *OldFMD = OldF->getMetadata(FunctionTag);
  assert(OldFMD && "Expecting OldF metadata");
  auto *OldFMDTuple = dyn_cast<MDTuple>(OldFMD);
  assert(OldFMDTuple && "Expecting OldF metadata tuple");
  LLVMContext &Ctx = OutF->getParent()->getContext();
  Metadata *OutFMD = copyMD(Ctx, OldFMD);
  auto *OutFMDTuple = cast<MDTuple>(OutFMD);
  // Update the function name to correspond to 'OutF'
  std::string FuncName = std::string(OutF->getName());
  FuncName.insert(0, "name: ");
  auto FuncNameMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, FuncName));
  OutFMDTuple->replaceOperandWith(FMDIR_FuncName, FuncNameMD);
  // Update the linkage string to correspond to 'OutF'.
  std::string LinkageStr = "linkage: ";
  LinkageStr.append(llvm::getLinkageStr(OutF));
  auto LinkageMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, LinkageStr));
  OutFMDTuple->replaceOperandWith(FMDIR_LinkageStr, LinkageMD);
  OutF->setMetadata(FunctionTag, OutFMDTuple);
  // Add 'OutF' to the list of functions.
  Module *M = OldF->getParent();
  NamedMDNode *ModuleInlineReport = M->getOrInsertNamedMetadata(ModuleTag);
  initFunctionTemps(OutF);
  ModuleInlineReport->addOperand(OutFMDTuple);
  addCallback(OutF);
  // Update the list of callsites for 'OldF'.
  SmallVector<Metadata *, 100> OldOps;
  OldOps.push_back(llvm::MDString::get(Ctx, CallSitesTag));
  for (auto &I : instructions(OldF))
    if (auto CB = dyn_cast<CallBase>(&I))
      if (Metadata *CBMD = CB->getMetadata(MDInliningReport::CallSiteTag))
        OldOps.push_back(cast<MDTuple>(CBMD));
  // Be sure to include 'OutCB' in the callsites for 'OldF'.
  CallSiteInliningReport CSIR(OutCB, nullptr, NinlrPreferPartialInline);
  OutCB->setMetadata(CallSiteTag, CSIR.get());
  setMDReasonNotInlined(OutCB, NinlrPreferPartialInline);
  setCalledFunction(OutCB, OutF);
  Metadata *OutCBMD = OutCB->getMetadata(MDInliningReport::CallSiteTag);
  OldOps.push_back(cast<MDTuple>(OutCBMD));
  MDNode *OldCSs = MDTuple::getDistinct(Ctx, OldOps);
  OldFMDTuple->replaceOperandWith(FMDIR_CSs, OldCSs);
  addCallback(OutCB);
  // Update the list of callsites for 'OutF'.
  SmallVector<Metadata *, 100> OutOps;
  OutOps.push_back(llvm::MDString::get(Ctx, CallSitesTag));
  for (auto &I : instructions(OutF))
    if (auto CB = dyn_cast<CallBase>(&I))
      if (Metadata *CBMD = CB->getMetadata(MDInliningReport::CallSiteTag))
        OutOps.push_back(cast<MDTuple>(CBMD));
  MDNode *OutCSs = MDTuple::getDistinct(Ctx, OutOps);
  OutFMDTuple->replaceOperandWith(FMDIR_CSs, OutCSs);
}

void InlineReportBuilder::addIndirectCallBaseTarget(InlICSType ICSMethod,
                                                    CallBase *CBIndirect,
                                                    CallBase *CBDirect) {
  if (!isMDIREnabled())
    return;
  Metadata *CBIndirectMD = CBIndirect->getMetadata(CallSiteTag);
  if (!CBIndirectMD)
    return;
  CallSiteInliningReport CSIR(CBDirect, nullptr, NinlrNewlyCreated);
  Function *Callee = CBDirect->getCalledFunction();
  std::string FuncName = std::string(Callee ? Callee->getName() : "");
  FuncName.insert(0, "name: ");
  CBDirect->setMetadata(CallSiteTag, CSIR.get());
  LLVMContext &Ctx = CBDirect->getModule()->getContext();
  auto FuncNameMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, FuncName));
  CSIR.get()->replaceOperandWith(CSMDIR_CalleeName, FuncNameMD);
  std::string ICSMethodStr = "icsMethod: ";
  ICSMethodStr.append(std::to_string(ICSMethod));
  auto ICSMethodMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, ICSMethodStr));
  Metadata *CBDirectMD = CBDirect->getMetadata(CallSiteTag);
  auto *CBDirectMDTuple = cast<MDTuple>(CBDirectMD);
  CBDirectMDTuple->replaceOperandWith(CSMDIR_ICSMethod, ICSMethodMD);
  // Recreate the call site list for 'CBIndirect', making 'CBDirect'
  // the last call in the list.
  SmallVector<Metadata *, 100> Ops;
  Ops.push_back(llvm::MDString::get(Ctx, CallSitesTag));
  auto *CBIndirectMDTuple = cast<MDTuple>(CBIndirectMD);
  if (Metadata *MDCSs = CBIndirectMDTuple->getOperand(CSMDIR_CSs).get()) {
    auto CSs = cast<MDTuple>(MDCSs);
    unsigned CSsNumOps = CSs->getNumOperands();
    for (unsigned I = 1; I < CSsNumOps; ++I)
      Ops.push_back(CSs->getOperand(I));
  }
  Ops.push_back(CSIR.get());
  MDNode *NewCSs = MDTuple::getDistinct(Ctx, Ops);
  CBIndirectMDTuple->replaceOperandWith(CSMDIR_CSs, NewCSs);
  addCallback(CBDirect);
}

void InlineReportBuilder::replaceAllUsesWith(Function *OldFunction,
                                             Function *NewFunction) {
  //
  // NOTE: This should be called before replaceAllUsesWith() in Value.cpp,
  // because it uses the 'users' list to find the users of 'OldFunction'.
  //
  if (!isMDIREnabled())
    return;
  for (auto U : OldFunction->users())
    if (auto CB = dyn_cast<CallBase>(U))
      setCalledFunction(CB, NewFunction);
}

void InlineReportBuilder::replaceUsesWithIf(
    Function *OldFunction, Function *NewFunction,
    llvm::function_ref<bool(Use &U)> ShouldReplace) {
  //
  // NOTE: This should be called before replaceUsesWithIf() in Value.cpp,
  // because it uses the 'users' list to find the users of 'OldFunction'.
  //
  if (!isMDIREnabled())
    return;
  for (auto &U : OldFunction->uses())
    if (ShouldReplace(U))
      if (auto CB = dyn_cast<CallBase>(U.getUser()))
        setCalledFunction(CB, NewFunction);
}

void InlineReportBuilder::removeCallBasesInBasicBlocks(
    SmallSetVector<BasicBlock *, 8> &BlocksToRemove) {
  if (!isMDIREnabled())
    return;
  for (BasicBlock *BB : BlocksToRemove)
    for (Instruction &I : *BB)
      if (auto CB = dyn_cast<CallBase>(&I))
        removeCallBaseReference(*CB, NinlrDeletedDeadCode);
}

bool InlineReportBuilder::shouldSkipCallBase(CallBase *CB) {
  auto II = dyn_cast<IntrinsicInst>(CB);
  if (!II)
    return false;
  return !(getLevel() & DontSkipIntrin) && shouldSkipIntrinsic(II);
}

void InlineReportBuilder::setBrokerTarget(CallBase *CB, Function *F) {
  if (!isMDIREnabled())
    return;
  Metadata *CBMD = CB->getMetadata(CallSiteTag);
  if (!CBMD)
    return;
  auto CSIR = cast<MDTuple>(CBMD);
  std::string FuncName = std::string(F->hasName() ? F->getName() : "");
  FuncName.insert(0, "name: ");
  LLVMContext &Ctx = CB->getModule()->getContext();
  auto FuncNameMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, FuncName));
  CSIR->replaceOperandWith(CSMDIR_BrokerTargetName, FuncNameMD);
  setMDReasonNotInlined(CB, NinlrBrokerFunction);
}

void InlineReportBuilder::updateName(Function *F) {
  if (!isMDIREnabled())
    return;
  Metadata *FMD = F->getMetadata(FunctionTag);
  if (!FMD)
    return;
  auto FIR = cast<MDTuple>(FMD);
  std::string FuncName = std::string(F->getName());
  FuncName.insert(0, "name: ");
  LLVMContext &Ctx = F->getParent()->getContext();
  auto FuncNameMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, FuncName));
  FIR->replaceOperandWith(FMDIR_FuncName, FuncNameMD);
  for (User *U : F->users())
    if (auto CB = dyn_cast<CallBase>(U))
      if (CB->getCalledFunction() == F) {
        Metadata *CBMD = CB->getMetadata(CallSiteTag);
        if (!CBMD)
          continue;
        auto CSIR = cast<MDTuple>(CBMD);
        CSIR->replaceOperandWith(CSMDIR_CalleeName, FuncNameMD);
      }
}

extern cl::opt<unsigned> IntelInlineReportLevel;

InlineReportBuilder *llvm::getMDInlineReport() {
  static llvm::InlineReportBuilder *SavedInlineReportBuilder = nullptr;
  if (!SavedInlineReportBuilder)
    SavedInlineReportBuilder =
        new llvm::InlineReportBuilder(IntelInlineReportLevel);
  return SavedInlineReportBuilder;
}
