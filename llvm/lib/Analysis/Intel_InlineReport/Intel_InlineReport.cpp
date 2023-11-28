//===- Intel_InlineReport.cpp - Inline report ------- ---------------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the mechanics of the inlining report.
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/IPO/Intel_InlineReport.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Transforms/IPO/Intel_InlineReportCommon.h"
#include "llvm/Transforms/IPO/Utils/Intel_IPOUtils.h"
#include <iomanip>

using namespace llvm;
using namespace InlineReportTypes;

#define DEBUG_TYPE "inlinereport"

//
// Member functions for class InlineReportCallSite
//

InlineReportCallSite::~InlineReportCallSite(void) {
  while (!Children.empty()) {
    InlineReportCallSite *cs = Children.back();
    Children.pop_back();
    delete cs;
  }
}

InlineReportCallSite *
InlineReportCallSite::copyBase(CallBase *CB,
                               InlineReportFunction *NewIRCallee) {
  InlineReportFunction *NewIRF = NewIRCallee ? NewIRCallee : IRCallee;
  InlineReportCallSite *NewCS =
      new InlineReportCallSite(NewIRF, IsInlined, Reason, M, nullptr, CB);
  NewCS->Reason = Reason;
  NewCS->InlineCost = InlineCost;
  NewCS->OuterInlineCost = OuterInlineCost;
  NewCS->InlineThreshold = InlineThreshold;
  NewCS->CostBenefit = CostBenefit;
  NewCS->Line = Line;
  NewCS->Col = Col;
  NewCS->Children.clear();
  NewCS->IsCompact = IsCompact;
  NewCS->ICSMethod = ICSMethod;
  NewCS->IRBrokerTarget = IRBrokerTarget;
  return NewCS;
}

InlineReportCallSite *InlineReport::cloneBase(InlineReportCallSite *OldIRCS,
                                              const ValueToValueMapTy &IIMap,
                                              CallBase *ActiveInlineCallBase) {
  if (OldIRCS->getIsInlined()) {
    InlineReportCallSite *IRCSk = OldIRCS->copyBase(nullptr);
    return IRCSk;
  }
  const Value *OldCall = OldIRCS->getCall();
  if (!OldCall)
    return nullptr;
  // If the OldCall was the ActiveInlineCallBase, we placed a nullptr
  // into the IIMap for it.
  InlineReportCallSite *IRCSk = nullptr;
  bool IsRecursiveCopy = OldCall == ActiveInlineCallBase;
  if (IsRecursiveCopy)
    OldCall = nullptr;
  auto VMI = IIMap.find(OldCall);
  CallBase *CB = nullptr;
  if (VMI != IIMap.end() && VMI->second)
    CB = cast<CallBase>(VMI->second);
  if (IsRecursiveCopy && CB) {
    // Start with a clean copy, as this is a newly created callsite produced
    // by recursive inlining.
    IRCSk =
        new InlineReportCallSite(OldIRCS->getIRCallee(), false, NinlrNoReason,
                                 CB->getFunction()->getParent(), nullptr, CB);
    IRCSk->setLine(OldIRCS->getLine());
    IRCSk->setCol(OldIRCS->getCol());
  } else {
    // The inliner can convert an indirect call into a direct call.
    // If so, update the IRCallee appropriately.
    InlineReportFunction *NewIRCallee = OldIRCS->getIRCallee();
    if (!NewIRCallee && CB && CB->getCalledFunction())
      NewIRCallee = getOrAddFunction(CB->getCalledFunction());
    IRCSk = OldIRCS->copyBase(CB, NewIRCallee);
    if (!CB)
      IRCSk->setReason(NinlrDeleted);
  }
  return IRCSk;
}

void InlineReport::setBrokerTarget(CallBase *CB, Function *F) {
  if (!isClassicIREnabled())
    return;
  InlineReportCallSite *IRCS = getOrAddCallSite(CB);
  InlineReportFunction *IRF = getOrAddFunction(F);
  IRCS->setIRBrokerTarget(IRF);
  IRCS->setReason(NinlrBrokerFunction);
}

void InlineReport::updateName(Function *F) {
  if (!isClassicIREnabled())
    return;
  InlineReportFunction *IRF = getOrAddFunction(F);
  IRF->setName(std::string(F->getName()));
}

///
/// Print a simple message.
///
/// OS: The output stream to print to
/// message: The message being printed
/// indentCount: The number of indentations before printing the message
/// level: The level N from '-inline-report=N'
///
static void printSimpleMessage(raw_ostream &OS, const char *Message,
                               unsigned IndentCount, unsigned Level,
                               bool IsInline) {
#if !INTEL_PRODUCT_RELEASE
  if (Level & InlineReportOptions::Reasons) {
    if (Level & InlineReportOptions::SameLine) {
      OS << " ";
    } else {
      OS << "\n";
      printIndentCount(OS, IndentCount + 1);
    }
    OS << (IsInline ? "<<" : "[[");
    OS << Message;
    OS << (IsInline ? ">>" : "]]");
    OS << "\n";
    return;
  }
#endif // !INTEL_PRODUCT_RELEASE
  OS << "\n";
}

///
/// Print the inlining cost and benefit values
///
void InlineReportCallSite::printCostAndBenefit(raw_ostream &OS,
                                               unsigned Level) {
  if (!(Level & InlineReportOptions::EarlyExitCost))
    return;
  CostBenefitPair CBP = *getCostBenefit();
  OS << " (" << CBP.getCost().getLimitedValue(INT64_MAX);
  if (getIsInlined())
    OS << "<=";
  else
    OS << ">";
  OS << CBP.getBenefit().getLimitedValue(INT64_MAX);
  OS << ")";
}

///
/// Print the inlining cost and threshold values
///
void InlineReportCallSite::printCostAndThreshold(raw_ostream &OS,
                                                 unsigned Level) {
  if (!(Level & InlineReportOptions::EarlyExitCost))
    return;
  OS << " (" << getInlineCost();
  if (getIsInlined())
    OS << "<=";
  else
    OS << ">";
  OS << getInlineThreshold();
  if (((Level & InlineReportOptions::RealCost) != 0) && isEarlyExit() &&
      !getIsInlined()) {
    // Under RealCost flag we compute both real and "early exit" costs and
    // thresholds of inlining.
    OS << " [EE:" << getEarlyExitInlineCost();
    OS << ">";
    OS << getEarlyExitInlineThreshold() << "]";
  }
  OS << ")";
}

///
/// Print the outer inlining cost and threshold values
///
void InlineReportCallSite::printOuterCostAndThreshold(raw_ostream &OS,
                                                      unsigned Level) {
  if (!(Level & InlineReportOptions::EarlyExitCost))
    return;
  OS << " (" << getOuterInlineCost() << ">" << getInlineCost() << ">"
     << getInlineThreshold() << ")";
}

///
/// Print the indirect call specialization method.
///
void InlineReportCallSite::printICSMethod(raw_ostream &OS, unsigned Level) {
  if (!(Level & InlineReportOptions::Indirects))
    return;
  switch (getICSMethod()) {
  case InlICSNone:
    break;
  case InlICSGPT:
    OS << "(GPT) ";
    break;
  case InlICSSFA:
    OS << "(SFA) ";
    break;
  case InlICSPGO:
    OS << "(PGO) ";
    break;
  }
}

///
/// Print the linkage info for a function 'F' as a single letter,
/// if the 'Level' specifies InlineReportOptions::Linkage.
/// For an explanation of the meaning of these letters,
/// see Intel_InlineReport.h.
///
static void printFunctionLinkage(raw_ostream &OS, unsigned Level,
                                 InlineReportFunction *IRF) {
  if (!(Level & InlineReportOptions::Linkage))
    return;
  OS << IRF->getLinkageChar() << " ";
}

///
/// Print the source language for a function 'F' as a single letter,
/// if the 'Level' specifies InlineReportOptions::Language.
/// For an explanation of the meaning of these letters,
/// see Intel_InlineReport.h.
///
static void printFunctionLanguage(raw_ostream &OS, unsigned Level,
                                  InlineReportFunction *IRF) {
  if (!(Level & InlineReportOptions::Language))
    return;
  OS << IRF->getLanguageChar() << " ";
}

///
/// Print optionally the callee linkage and language, and then the
/// callee name, and if non-zero, the line and column number of the call site
///
void InlineReportCallSite::printCalleeNameModuleLineCol(raw_ostream &OS,
                                                        unsigned Level) {
  if (auto IRF = getIRCallee()) {
    printICSMethod(OS, Level);
    printFunctionLinkage(OS, Level, getIRCallee());
    printFunctionLanguage(OS, Level, getIRCallee());
    IRF->printName(OS, Level);
  }
  if (Level & InlineReportOptions::File)
    OS << " " << M->getModuleIdentifier();
  if ((Level & InlineReportOptions::LineCol) && (Line != 0 || Col != 0))
    OS << " (" << Line << "," << Col << ")";
}

void InlineReportCallSite::printBrokerTargetName(raw_ostream &OS,
                                                 unsigned Level) {
  OS << "(";
  getIRBrokerTarget()->printName(OS, Level);
  OS << ")\n";
}

///
/// Print a representation of the inlining instance.
///
/// OS: The output stream to print to
/// indentCount: The number of indentations to print
/// level: The level N from '-inline-report=N'
///
void InlineReportCallSite::print(raw_ostream &OS, unsigned IndentCount,
                                 unsigned Level) {
  assert(InlineReasonText[getReason()].Type != InlPrtNone);
  if (getIsInlined()) {
    printIndentCount(OS, IndentCount);
    if (getIsCompact())
      OS << "-> <C> INLINE: ";
    else
      OS << "-> INLINE: ";
    printCalleeNameModuleLineCol(OS, Level);
    if (InlineReasonText[getReason()].Type == InlPrtCost)
      if (getCostBenefit())
        printCostAndBenefit(OS, Level);
      else
        printCostAndThreshold(OS, Level);
    printSimpleMessage(OS, InlineReasonText[getReason()].Message, IndentCount,
                       Level, true);
  } else {
    if (InlineReasonText[getReason()].Type == InlPrtSpecial) {
      switch (getReason()) {
      case NinlrExtern:
        if (Level & InlineReportOptions::Externs) {
          printIndentCount(OS, IndentCount);
          OS << "-> EXTERN: ";
          printCalleeNameModuleLineCol(OS, Level);
          OS << "\n";
        }
        break;
      case NinlrIndirect:
        if (Level & InlineReportOptions::Indirects) {
          printIndentCount(OS, IndentCount);
          OS << "-> INDIRECT:";
          printCalleeNameModuleLineCol(OS, Level);
          printSimpleMessage(OS, InlineReasonText[getReason()].Message,
                             IndentCount, Level, false);
        }
        break;
      case NinlrDeletedIndCallConv:
        if (Level & InlineReportOptions::Indirects) {
          printIndentCount(OS, IndentCount);
          OS << "-> INDIRECT: DELETE:";
          printCalleeNameModuleLineCol(OS, Level);
          printSimpleMessage(OS, InlineReasonText[getReason()].Message,
                             IndentCount, Level, false);
        }
        break;
      case NinlrOuterInlining:
        printIndentCount(OS, IndentCount);
        OS << "-> ";
        printCalleeNameModuleLineCol(OS, Level);
        printOuterCostAndThreshold(OS, Level);
        printSimpleMessage(OS, InlineReasonText[getReason()].Message,
                           IndentCount, Level, false);
        break;
      case NinlrBrokerFunction:
        printIndentCount(OS, IndentCount);
        OS << "-> BROKER: ";
        printCalleeNameModuleLineCol(OS, Level);
        printBrokerTargetName(OS, Level);
        break;
      default:
        assert(0);
      }
    } else if (InlineReasonText[getReason()].Type == InlPrtDeleted) {
      printIndentCount(OS, IndentCount);
      OS << "-> DELETE: ";
      printCalleeNameModuleLineCol(OS, Level);
      if (InlineReasonText[getReason()].Message)
        printSimpleMessage(OS, InlineReasonText[getReason()].Message,
                           IndentCount, Level, false);
      else
        OS << "\n";
    } else {
      printIndentCount(OS, IndentCount);
      OS << "-> ";
      printCalleeNameModuleLineCol(OS, Level);
      if (InlineReasonText[getReason()].Type == InlPrtCost)
        if (getCostBenefit())
          printCostAndBenefit(OS, Level);
        else
          printCostAndThreshold(OS, Level);
      printSimpleMessage(OS, InlineReasonText[getReason()].Message, IndentCount,
                         Level, false);
    }
  }
}

//
// Member functions for class InlineReportFunction
//

InlineReportFunction::~InlineReportFunction(void) {
  while (!CallSites.empty()) {
    InlineReportCallSite *CS = CallSites.back();
    CallSites.pop_back();
    delete CS;
  }
}

//
// Member functions for class InlineReport
//

InlineReportFunction *InlineReport::addFunction(Function *F) {
  if (!isClassicIREnabled())
    return nullptr;
  if (!F)
    return nullptr;
  bool SuppressInlRpt = false;
  if (F->getMetadata(IPOUtils::getSuppressInlineReportStringRef())) {
    LLVM_DEBUG(dbgs() << "Suppress inline report for Function: " << F->getName()
                      << "() \n";);
    SuppressInlRpt = true;
  }
  InlineReportFunction *IRF = new InlineReportFunction(F, SuppressInlRpt);
  IRFunctionMap.insert(std::make_pair(F, IRF));
  IRF->setName(std::string(F->getName()));
  IRF->setIsDeclaration(F->isDeclaration());
  IRF->setLinkageChar(F);
  IRF->setLanguageChar(F);
  addCallback(F);
  return IRF;
}

InlineReportFunction *InlineReport::getFunction(Function *F) {
  auto MapIt = IRFunctionMap.find(F);
  if (MapIt != IRFunctionMap.end())
    return MapIt->second;
  return nullptr;
}

InlineReportFunction *InlineReport::getOrAddFunction(Function *F) {
  if (auto IRF = getFunction(F))
    return IRF;
  return addFunction(F);
}

static void
addOutlinedIRCSes(InlineReportCallSiteVector &IRCSV,
                  SmallVectorImpl<InlineReportCallSite *> &EnclosingIRCSes,
                  SmallPtrSetImpl<CallBase *> &OutFCBSet,
                  SmallPtrSetImpl<InlineReportCallSite *> &OutFIRCSSet) {
  for (InlineReportCallSite *IRCS : IRCSV) {
    if (IRCS->getCall() && OutFCBSet.count(cast<CallBase>(IRCS->getCall()))) {
      OutFIRCSSet.insert(IRCS);
      for (InlineReportCallSite *IRCSX : EnclosingIRCSes)
        OutFIRCSSet.insert(IRCSX);
    }
    EnclosingIRCSes.push_back(IRCS);
    addOutlinedIRCSes(IRCS->getChildren(), EnclosingIRCSes, OutFCBSet,
                      OutFIRCSSet);
    EnclosingIRCSes.pop_back();
  }
}

void InlineReportFunction::findOutlinedIRCSes(
    SmallPtrSetImpl<CallBase *> &OutFCBSet,
    SmallPtrSetImpl<InlineReportCallSite *> &OutFIRCSSet) {
  SmallVector<InlineReportCallSite *, 4> EnclosingIRCSes;
  addOutlinedIRCSes(getCallSites(), EnclosingIRCSes, OutFCBSet, OutFIRCSSet);
}

void InlineReportCallSite::moveOutlinedChildren(
    InlineReportCallSiteVector &IRCSV,
    SmallPtrSetImpl<InlineReportCallSite *> &OutFCBSet,
    InlineReportCallSite *NewIRCS) {
  InlineReportCallSiteVector TIRCSV = IRCSV;
  for (InlineReportCallSite *IRCS : TIRCSV) {
    if (OutFCBSet.count(IRCS)) {
      if (IRCS->getCall()) {
        IRCS->move(this, NewIRCS);
      } else {
        InlineReportCallSite *NewIRCSX = IRCS->copyBase(nullptr);
        NewIRCS->addChild(NewIRCSX);
        IRCS->moveOutlinedChildren(IRCS->getChildren(), OutFCBSet, NewIRCSX);
      }
    }
  }
}

void InlineReportFunction::moveOutlinedCallSites(
    InlineReportFunction *NewIRF,
    SmallPtrSetImpl<InlineReportCallSite *> &OutFCBSet) {
  InlineReportCallSiteVector TIRCSV = getCallSites();
  for (InlineReportCallSite *IRCS : TIRCSV) {
    if (OutFCBSet.count(IRCS)) {
      if (IRCS->getCall()) {
        IRCS->move(this, NewIRF);
        IRCS->setIRCaller(NewIRF);
      } else {
        InlineReportCallSite *NewIRCS = IRCS->copyBase(nullptr);
        NewIRF->addCallSite(NewIRCS);
        IRCS->moveOutlinedChildren(IRCS->getChildren(), OutFCBSet, NewIRCS);
      }
    }
  }
}

void InlineReportCallSite::moveCalls(InlineReportCallSiteVector &OldIRCSV,
                                     InlineReportCallSiteVector &NewIRCSV) {
  for (auto I = OldIRCSV.begin(), E = OldIRCSV.end(); I != E; ++I) {
    if (*I == this) {
      OldIRCSV.erase(I);
      NewIRCSV.push_back(this);
      return;
    }
  }
}

void InlineReportCallSite::move(InlineReportFunction *OldIRF,
                                InlineReportFunction *NewIRF) {
  moveCalls(OldIRF->getCallSites(), NewIRF->getCallSites());
}

void InlineReportCallSite::move(InlineReportCallSite *OldIRCS,
                                InlineReportCallSite *NewIRCS) {
  moveCalls(OldIRCS->getChildren(), NewIRCS->getChildren());
}

void InlineReport::doOutlining(Function *OldF, Function *OutF,
                               CallBase *OutCB) {
  if (!isClassicIREnabled())
    return;
  auto MapIt = IRFunctionMap.find(OldF);
  assert(MapIt != IRFunctionMap.end());
  InlineReportFunction *OldIRF = MapIt->second;
  InlineReportFunction *OutIRF = addFunction(OutF);
  SmallPtrSet<CallBase *, 4> OutFCBSet;
  SmallPtrSet<InlineReportCallSite *, 4> OutFIRCSSet;
  for (auto &I : instructions(OutF))
    if (auto CB = dyn_cast<CallBase>(&I))
      OutFCBSet.insert(CB);
  OldIRF->findOutlinedIRCSes(OutFCBSet, OutFIRCSSet);
  OldIRF->moveOutlinedCallSites(OutIRF, OutFIRCSSet);
  addCallSite(OutCB);
  setReasonNotInlined(OutCB, NinlrPreferPartialInline);
  addCallback(OutCB);
}

InlineReportCallSite *InlineReport::addCallSite(CallBase *Call,
                                                bool AttachToCaller) {
  if (!isClassicIREnabled())
    return nullptr;
  assert(IRCallBaseCallSiteMap.find(Call) == IRCallBaseCallSiteMap.end());
  bool SuppressInlRpt = false;
  if (Call->getMetadata(IPOUtils::getSuppressInlineReportStringRef())) {
    LLVM_DEBUG(dbgs() << "Suppress inline report on: \n" << *Call << "\n";);
    SuppressInlRpt = true;
  }
  DebugLoc DLoc = Call->getDebugLoc();
  Function *Callee = Call->getCalledFunction();
  InlineReportFunction *IRFC = Callee ? getOrAddFunction(Callee) : nullptr;
  InlineReportCallSite *IRCS = new InlineReportCallSite(
      IRFC, false, NinlrNoReason, Call->getFunction()->getParent(), &DLoc, Call,
      SuppressInlRpt);
  IRCS->initReason(Callee);
  IRCallBaseCallSiteMap.insert(std::make_pair(Call, IRCS));
  addCallback(Call);
  if (AttachToCaller) {
    Function *F = Call->getCaller();
    InlineReportFunction *IRF = getOrAddFunction(F);
    IRF->addCallSite(IRCS);
  }
  return IRCS;
}

InlineReportCallSite *InlineReport::getOrAddCallSite(CallBase *Call) {
  if (!isClassicIREnabled())
    return nullptr;
  InlineReportCallSite *IRCS = getCallSite(Call);
  if (IRCS)
    return IRCS;
  return addCallSite(Call);
}

void InlineReport::beginSCC(LazyCallGraph::SCC &SCC, void *Inliner) {
  if (!isClassicIREnabled())
    return;
  ActiveInliners.insert(Inliner);
  LazyCallGraph::Node &LCGN = *(SCC.begin());
  M = LCGN.getFunction().getParent();
  for (auto &Node : SCC)
    initFunction(&Node.getFunction());
}

void InlineReport::beginModule(void *Inliner) {
  ActiveInliners.insert(Inliner);
}

void InlineReport::endModule(void) { makeAllNotCurrent(); }

void InlineReport::endSCC(void) {
  if (!isClassicIREnabled())
    return;
  makeAllNotCurrent();
}

void InlineReport::cloneChildrenCompact(InlineReportFunction *IRFCaller,
                                        InlineReportFunction *IRFCallee,
                                        InlineReportCallSite *NewCallSite,
                                        ValueToValueMapTy &IIMap) {
  IRFCaller->inheritCompactCallBases(IRFCallee);
  if (IRFCallee->getIsSummarized())
    NewCallSite->setIsCompact(true);
  for (const auto &Pair : IIMap)
    if (auto OldCall = dyn_cast<const CallBase>(Pair.first))
      if (auto IRCSj = getCallSite(const_cast<CallBase *>(OldCall))) {
        //
        // Copy the old InlineReportCallSite and add it to the children
        // of the cloned InlineReportCallSite.
        auto IRCSk = cloneBase(IRCSj, IIMap, ActiveInlineCallBase);
        if (!IRCSk)
          continue;
        NewCallSite->addChild(IRCSk);
        //
        // We keep track of the new calls that are added added to the
        // inline report in case they themselves will be inlined.
        if (IRCSk->getCall()) {
          auto NewPair = std::make_pair(IRCSk->getCall(), IRCSk);
          IRCallBaseCallSiteMap.insert(NewPair);
          addCallback(IRCSk->getCall());
        }
      }
  if (IRFCaller->getIsCompact())
    IRFCaller->addCompactInlinedCallBase(IRFCallee);
}

void InlineReport::cloneChildren(InlineReportCallSiteVector &OldCallSiteVector,
                                 InlineReportCallSite *NewCallSite,
                                 ValueToValueMapTy &IIMap) {
  for (unsigned I = 0, E = OldCallSiteVector.size(); I < E; ++I) {
    InlineReportCallSite *IRCSj = OldCallSiteVector[I];
    //
    // Copy the old InlineReportCallSite and add it to the children of the
    // cloned InlineReportCallSite.
    InlineReportCallSite *IRCSk = cloneBase(IRCSj, IIMap, ActiveInlineCallBase);
    if (!IRCSk)
      continue;
    NewCallSite->addChild(IRCSk);
    //
    // We keep track of the new calls that are added added to the inline
    // report in case they themselves will be inlined.
    if (IRCSk->getCall()) {
      IRCallBaseCallSiteMap.insert(std::make_pair(IRCSk->getCall(), IRCSk));
      addCallback(IRCSk->getCall());
    }
    //
    // Recursively copy the InlineReportCallSites for the children.
    if (IRCSj->getIsInlined())
      cloneChildren(IRCSj->getChildren(), IRCSk, IIMap);
  }
}

void InlineReport::inlineCallSite() {
  if (!isClassicIREnabled())
    return;
  //
  // Get the inline report for the routine being inlined.  We are going
  // to make a clone of it and ensure that the report is up to date
  // since the last call to Inliner::runOnSCC
  InlineReportFunction *IRFCaller = getOrAddFunction(ActiveCaller);
  InlineReportFunction *IRFCallee = initFunction(ActiveCallee);
  // 'IRCallee' in 'ActiveIRCS' may be nullptr because it was originally
  // an indirect call that was converted to a direct call later. Update
  // the value now, since we know it.
  ActiveIRCS->setIRCallee(IRFCallee);
  //
  // Create InlineReportCallSites "new calls" which appear in the inlined
  // code.  Also, create a mapping from the "original calls" which appeared
  // in the routine that was inlined, to the "new calls". When we clone the
  // inline report for the routine being inlined, we need to replace the
  // original calls with the new calls in the cloned inline report.
  // We use 'IIMap' to do that mapping.
  ValueToValueMapTy IIMap;
  for (unsigned I = 0, E = ActiveOriginalCalls.size(); I < E; ++I) {
    // In the case of a directly recursive call, avoid putting the
    // ActiveInlineCallBase into the IIMap, as it is an already
    // deleted value. Putting it in the IIMap could cause a ValueHandle
    // to be associated with it, which should not happen because it is
    // deleted.
    Value *OC = ActiveOriginalCalls[I] == ActiveInlineCallBase
                    ? nullptr
                    : ActiveOriginalCalls[I];
    Value *NC = ActiveInlinedCalls[I];
    IIMap.insert(std::make_pair(OC, NC));
  }
  //
  // Clone the inline report IRFCallee and attach it to the inlined call
  // site IRCS. Use IIMap to map the original calls to the new calls in the
  // cloned inline report.
  bool ForceCompact = Level & Compact;
  if (IRFCaller->shouldCompactCallBase(IRFCallee, ForceCompact))
    IRFCallee->compact();
  if (IRFCallee->getIsCompact())
    cloneChildrenCompact(IRFCaller, IRFCallee, ActiveIRCS, IIMap);
  else
    cloneChildren(IRFCallee->getCallSites(), ActiveIRCS, IIMap);
  // Indicate that the call has been inlined in the inline report
  ActiveIRCS->setIsInlined(true);

  //
  // Remove the inlined instruction from the IRCallBaseCallSiteMap
  auto MapIt = IRCallBaseCallSiteMap.find(ActiveInlineCallBase);
  assert(MapIt != IRCallBaseCallSiteMap.end());
  IRCallBaseCallSiteMap.erase(MapIt);
  ActiveIRCS->setCall(nullptr);
  unsigned NewInlineCount = IRFCallee->getInlineCount() + 1;
  IRFCaller->setInlineCount(IRFCaller->getInlineCount() + NewInlineCount);
}

void InlineReport::setReasonIsInlined(CallBase *Call, InlineReason Reason) {
  if (!isClassicIREnabled())
    return;
  assert(IsInlinedReason(Reason));
  auto MapIt = IRCallBaseCallSiteMap.find(Call);
  // The new call site may have already been deleted by dead code elimination.
  if (MapIt == IRCallBaseCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapIt->second;
  IRCS->setReason(Reason);
}

void InlineReport::setReasonIsInlined(CallBase *Call, const InlineCost &IC) {
  if (!isClassicIREnabled())
    return;
  assert(IsInlinedReason(IC.getInlineReason()));
  auto MapIt = IRCallBaseCallSiteMap.find(Call);
  // The new call site may have already been deleted by dead code elimination.
  if (MapIt == IRCallBaseCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapIt->second;
  IRCS->setReason(IC.getInlineReason());
  IRCS->setCostBenefit(IC.getCostBenefit());
  if (IC.isAlways())
    return;
  IRCS->setInlineCost(IC.getCost());
  IRCS->setInlineThreshold(IC.getCost() + IC.getCostDelta());
}

void InlineReport::setReasonNotInlined(CallBase *Call, InlineReason Reason) {
  if (!isClassicIREnabled())
    return;
  assert(IsNotInlinedReason(Reason));
  auto MapIt = IRCallBaseCallSiteMap.find(Call);
  // The new call site may have already been deleted by dead code elimination.
  if (MapIt == IRCallBaseCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapIt->second;
  if (Reason == NinlrNotAlwaysInline && IsNotInlinedReason(IRCS->getReason()))
    return;
  IRCS->setReason(Reason);
}

void InlineReport::setReasonNotInlined(CallBase *Call, const InlineCost &IC) {
  if (!isClassicIREnabled())
    return;
  InlineReason Reason = IC.getInlineReason();
  assert(IsNotInlinedReason(Reason));
  auto MapIt = IRCallBaseCallSiteMap.find(Call);
  // The new call site may have already been deleted by dead code elimination.
  if (MapIt == IRCallBaseCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapIt->second;
  if (Reason == NinlrNotAlwaysInline && IsNotInlinedReason(IRCS->getReason()))
    return;
  IRCS->setReason(Reason);
  IRCS->setCostBenefit(IC.getCostBenefit());
  if (IC.isNever())
    return;
  IRCS->setInlineCost(IC.getCost());
  IRCS->setInlineThreshold(IC.getCost() + IC.getCostDelta());
  IRCS->setEarlyExitInlineCost(IC.getEarlyExitCost());
  IRCS->setEarlyExitInlineThreshold(IC.getEarlyExitThreshold());
}

void InlineReport::setReasonNotInlined(CallBase *Call, const InlineCost &IC,
                                       int TotalSecondaryCost) {
  if (!isClassicIREnabled())
    return;
  assert(IC.getInlineReason() == NinlrOuterInlining);
  setReasonNotInlined(Call, IC);
  auto MapIt = IRCallBaseCallSiteMap.find(Call);
  // The new call site may have already been deleted by dead code elimination.
  if (MapIt == IRCallBaseCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapIt->second;
  IRCS->setOuterInlineCost(TotalSecondaryCost);
}

///
/// Print the callsites in the 'Vector'.
///
/// OS: The output stream to print to
/// indentCount: The number of indentations to print
/// level: The level N from '-inline-report=N'
///
static void
printInlineReportCallSiteVector(raw_ostream &OS,
                                const InlineReportCallSiteVector &Vector,
                                unsigned IndentCount, unsigned Level) {
  for (unsigned I = 0, E = Vector.size(); I < E; ++I) {
    InlineReportCallSite *IRCS = Vector[I];
    if (IRCS->getSuppressPrint())
      continue;
    IRCS->print(OS, IndentCount, Level);
    printInlineReportCallSiteVector(OS, IRCS->getChildren(), IndentCount + 1,
                                    Level);
  }
}

void InlineReportFunction::print(raw_ostream &OS, unsigned Level) const {
  if (!Level || (Level & BasedOnMetadata))
    return;
  printInlineReportCallSiteVector(OS, CallSites, 1, Level);
  if (!Inlines.empty()) {
    OS << "  SUMMARIZED INLINED CALL SITE COUNTS\n";
    for (auto &Pair : Inlines) {
      std::stringstream SS;
      OS << "    ";
      SS << std::setw(5) << " " << Pair.second << " ";
      OS << SS.str() << Pair.first->getName() << "\n";
    }
  }
}

void InlineReport::print() const {
  if (!isClassicIREnabled())
    return;
  OS << "---- Begin Inlining Report ----\n";
  if (Level & InlineReportOptions::Options)
    printOptionValues(OS);
  auto &IRDFS = IRDeadFunctionSet;
  if (Level & InlineReportOptions::DeadStatics) {
    for (auto I = IRDFS.begin(), E = IRDFS.end(); I != E; ++I) {
      InlineReportFunction *IRF = *I;
      // Suppress inline report on any Function with Suppress mark on
      if (IRF->getSuppressPrint())
        continue;
      OS << "DEAD STATIC FUNC: ";
      printFunctionLinkage(OS, Level, IRF);
      printFunctionLanguage(OS, Level, IRF);
      IRF->printName(OS, Level);
      OS << "\n\n";
    }
  }

  InlineReportFunctionMap::const_iterator Mit, E;
  for (Mit = IRFunctionMap.begin(), E = IRFunctionMap.end(); Mit != E; ++Mit) {
    Function *F = Mit->first;
    // Update the linkage info one last time before printing,
    // as it may have changed.
    InlineReportFunction *IRF = Mit->second;
    IRF->setLinkageChar(F);
    if (IRF->getSuppressPrint())
      continue;
    if (!IRF->getIsDeclaration()) {
      OS << "COMPILE FUNC: ";
      printFunctionLinkage(OS, Level, IRF);
      printFunctionLanguage(OS, Level, IRF);
      IRF->printName(OS, Level);
      OS << "\n";
      InlineReportFunction *IRF = Mit->second;
      IRF->print(OS, Level);
      OS << "\n";
    }
  }
  OS << "---- End Inlining Report ------\n";
}

bool InlineReportFunction::shouldCompactCallBase(
    InlineReportFunction *IRFCallee, bool ForceCompact) {
  if (IRFCallee->getIsCompact())
    return false;
  if (ForceCompact)
    return true;
  if (IRFCallee->getInlineCount() > IntelInlineReportCompactThreshold)
    return true;
  return false;
}

void InlineReportFunction::addCompactInlinedCallBase(
    InlineReportFunction *IRFCallee, unsigned Count) {
  auto MapIt = TotalInlines.find(IRFCallee);
  if (MapIt != TotalInlines.end()) {
    MapIt->second += Count;
    return;
  }
  TotalInlines.insert({IRFCallee, Count});
}

void InlineReportFunction::addForCompactInlinedCallBase(
    InlineReportFunction *IRFCallee, unsigned Count) {
  auto MapIt = Inlines.find(IRFCallee);
  if (MapIt != Inlines.end()) {
    MapIt->second += Count;
    return;
  }
  Inlines.insert({IRFCallee, Count});
}

void InlineReportFunction::compactChildren(InlineReportCallSite *IRCS) {
  for (auto IRCS0 : IRCS->getChildren())
    if (IRCS0->getIsInlined()) {
      addCompactInlinedCallBase(IRCS0->getIRCallee());
      compactChildren(IRCS0);
    }
}

void InlineReportFunction::compact() {
  for (auto IRCS : getCallSites())
    if (IRCS->getIsInlined()) {
      addCompactInlinedCallBase(IRCS->getIRCallee());
      compactChildren(IRCS);
    }
  setIsCompact(true);
}

void InlineReportFunction::inheritCompactCallBases(
    InlineReportFunction *IRFCallee) {
  for (auto &InlinedPair : IRFCallee->TotalInlines) {
    addForCompactInlinedCallBase(InlinedPair.first, InlinedPair.second);
    if (getIsCompact())
      addCompactInlinedCallBase(InlinedPair.first, InlinedPair.second);
  }
}

void InlineReportFunction::cloneCompactInfo(InlineReportFunction *IRF) {
  InlineCount = IRF->InlineCount;
  for (auto &Pair : IRF->Inlines)
    Inlines.insert(Pair);
  for (auto &Pair : IRF->TotalInlines)
    TotalInlines.insert(Pair);
}

void InlineReport::testAndPrint(void *Inliner) {
  if (!Inliner) {
    print();
    return;
  }
  if (!ActiveInliners.count(Inliner))
    return;
  ActiveInliners.erase(Inliner);
  if (!ActiveInliners.empty())
    return;
  print();
}

void InlineReportCallSite::loadCallsToMap(std::map<CallBase *, bool> &LMap) {
  CallBase *CB = getCall();
  if (CB)
    LMap.insert(std::make_pair(CB, true));
  for (unsigned I = 0, E = Children.size(); I < E; ++I)
    Children[I]->loadCallsToMap(LMap);
}

void InlineReportCallSite::initReason(Function *Callee) {
  if (Callee) {
    if (Callee->isDeclaration()) {
      if (Callee->isIntrinsic())
        setReason(NinlrIntrinsic);
      else
        setReason(NinlrExtern);
    } else {
      setReason(NinlrNewlyCreated);
    }
  } else {
    setReason(NinlrIndirect);
  }
}

#ifndef NDEBUG
bool InlineReport::validateFunction(Function *F) {
  llvm::errs() << "Validating " << F->getName() << "\n";
  bool ReturnValue = true;
  auto MapIt = IRFunctionMap.find(F);
  if (MapIt == IRFunctionMap.end())
    return false;
  InlineReportFunction *IRF = MapIt->second;
  IRF->print(llvm::ferrs(), Level);
  std::map<CallBase *, bool> OriginalCalls;
  const InlineReportCallSiteVector &Vec = IRF->getCallSites();
  for (unsigned I = 0, E = Vec.size(); I < E; ++I)
    Vec[I]->loadCallsToMap(OriginalCalls);
  for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
    for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
      auto *Call = dyn_cast<CallBase>(&*I);
      if (!Call)
        continue;
      auto MapIt = OriginalCalls.find(Call);
      if (MapIt == OriginalCalls.end()) {
        ReturnValue = false;
        llvm::errs() << "Cannot find " << Call << "\n";
        Call->dump();
      }
    }
  }
  llvm::errs() << "Done Validating " << F->getName() << "\n";
  return ReturnValue;
}

bool InlineReport::validate(void) {
  bool GlobalRv = true;
  InlineReportFunctionMap::const_iterator MI, ME;
  llvm::errs() << "Start Validation Pass\n";
  for (MI = IRFunctionMap.begin(), ME = IRFunctionMap.end(); MI != ME; ++MI) {
    Function *F = MI->first;
    bool LocalRv = validateFunction(F);
    llvm::errs() << "Validated " << F->getName();
    if (LocalRv)
      llvm::errs() << " passed\n";
    else
      llvm::errs() << " failed\n";
    GlobalRv &= LocalRv;
  }
  llvm::errs() << "End Validation Pass\n";
  return GlobalRv;
}
#endif // NDEBUG

void InlineReport::addIndirectCallBaseTarget(InlICSType ICSMethod,
                                             CallBase *CBIndirect,
                                             CallBase *CBDirect) {
  if (!isClassicIREnabled())
    return;
  InlineReportCallSite *IRCSIndirect = getOrAddCallSite(CBIndirect);
  InlineReportCallSite *IRCSDirect = addCallSite(CBDirect,
                                                 /*AttachToCaller=*/false);
  IRCSIndirect->addChild(IRCSDirect);
  IRCSDirect->initReason(CBDirect->getCalledFunction());
  IRCSDirect->setICSMethod(ICSMethod);
}

bool InlineReport::makeCurrent(Function *F) {
  auto MapIt = IRFunctionMap.find(F);
  InlineReportFunction *IRF =
      (MapIt == IRFunctionMap.end()) ? addFunction(F) : MapIt->second;
  if (IRF->getCurrent())
    return false;
  if (IRF->getIsDeclaration()) {
    IRF->setCurrent(true);
    return false;
  }
  bool Changed = false;
  SmallPtrSet<CallBase *, 16> SeenCallBases;
  // Ensure that every CallBase in F is in the IRCallBaseCallSiteMap.
  for (auto &I : instructions(*F)) {
    CallBase *Call = dyn_cast<CallBase>(&I);
    if (!Call || shouldSkipCallBase(Call))
      continue;
    SeenCallBases.insert(Call);
    if (IRCallBaseCallSiteMap.count(Call))
      continue;
    Changed = true;
    InlineReportCallSite *IRCS = addCallSite(Call);
    IRCS->initReason(Call->getCalledFunction());
  }

  // Ensure that any CallBase in the IRCallBaseCallSiteMap which is
  // no longer in F is marked as deleted.
  SmallVector<CallBase *, 16> RemovableCallBases;
  for (const auto &CBI : IRCallBaseCallSiteMap) {
    CallBase *CB = CBI.first;
    InlineReportCallSite *IRCS = CBI.second;
    if (CB == ActiveInlineCallBase || SeenCallBases.count(CB))
      continue;
    InlineReportCallSite *IRCS0 = IRCS;
    while (IRCS0->getIRParent())
      IRCS0 = IRCS0->getIRParent();
    if (IRCS0->getIRCaller() == IRF)
      RemovableCallBases.push_back(CB);
  }
  for (auto CB : RemovableCallBases) {
    Changed = true;
    removeCallBaseReference(*CB);
  }
  IRF->setCurrent(true);
  return Changed;
}

bool InlineReport::makeAllCurrent(void) {
  if (!isClassicIREnabled())
    return false;
  std::vector<Function *> FuncVector;
  for (auto &IRFME : IRFunctionMap)
    FuncVector.push_back(IRFME.first);
  bool Changed = false;
  for (Function *F : FuncVector)
    Changed |= makeCurrent(F);
  return Changed;
}

void InlineReport::makeAllNotCurrent(void) {
  if (!isClassicIREnabled())
    return;
  InlineReportFunctionMap::const_iterator It, E;
  for (It = IRFunctionMap.begin(), E = IRFunctionMap.end(); It != E; ++It) {
    InlineReportFunction *IRF = It->second;
    IRF->setCurrent(false);
  }
}

void InlineReport::replaceAllUsesWith(Function *OldFunction,
                                      Function *NewFunction) {
  //
  // NOTE: This should be called before replaceAllUsesWith() in Value.cpp,
  // because it uses the 'users' list to find the users of 'OldFunction'.
  //
  if (!isClassicIREnabled())
    return;
  auto MapIt = IRFunctionMap.find(NewFunction);
  assert(MapIt != IRFunctionMap.end());
  InlineReportFunction *IRFNew = MapIt->second;
  for (auto U : OldFunction->users()) {
    if (auto CB = dyn_cast<CallBase>(U)) {
      InlineReportCallSite *IRCS = getOrAddCallSite(CB);
      IRCS->setIRCallee(IRFNew);
    }
  }
}

void InlineReport::replaceUsesWithIf(
    Function *OldFunction, Function *NewFunction,
    llvm::function_ref<bool(Use &U)> ShouldReplace) {
  //
  // NOTE: This should be called before replaceUsesWithIf() in Value.cpp,
  // because it uses the 'users' list to find the users of 'OldFunction'.
  //
  if (!isClassicIREnabled())
    return;
  auto MapIt = IRFunctionMap.find(NewFunction);
  assert(MapIt != IRFunctionMap.end());
  InlineReportFunction *IRFNew = MapIt->second;
  for (auto &U : OldFunction->uses())
    if (ShouldReplace(U))
      if (auto CB = dyn_cast<CallBase>(U.getUser())) {
        InlineReportCallSite *IRCS = getOrAddCallSite(CB);
        IRCS->setIRCallee(IRFNew);
      }
}

InlineReportFunction *InlineReport::initFunction(Function *F) {
  InlineReportFunction *IRF = getOrAddFunction(F);
  assert(IRF);
  IRF->setCurrent(false);
  (void)makeCurrent(F);
  if (Level & Compact)
    IRF->setIsCompact(true);
  return IRF;
}

void InlineReport::initFunctionClosure(Function *F) {
  if (!isClassicIREnabled())
    return;
  initFunction(F);
  for (auto U : F->users())
    if (auto CB = dyn_cast<CallBase>(U))
      initFunction(CB->getCaller());
}

void InlineReport::replaceFunctionWithFunction(Function *OldFunction,
                                               Function *NewFunction) {
  if (!isClassicIREnabled())
    return;
  if (OldFunction == NewFunction)
    return;
  assert(OldFunction->getName() == "" && NewFunction->getName() != "" &&
         "Expecting name of OldFunction taken by NewFunction");
  InlineReportFunction *IRF = getOrAddFunction(OldFunction);
  IRFunctionMap.insert(std::make_pair(NewFunction, IRF));
  replaceAllUsesWith(OldFunction, NewFunction);
  IRF->setLinkageChar(NewFunction);
  IRF->setLanguageChar(NewFunction);
  IRF->setName(std::string(NewFunction->getName()));
  replaceFunctionReference(*OldFunction);
  addCallback(NewFunction);
}

void InlineReport::replaceCallBaseWithCallBase(CallBase *CB0, CallBase *CB1,
                                               bool UpdateReason) {
  if (!isClassicIREnabled())
    return;
  if (CB0 == CB1 || shouldSkipCallBase(CB0) && shouldSkipCallBase(CB1))
    return;
  if (shouldSkipCallBase(CB1)) {
    IRCallBaseCallSiteMap.erase(CB0);
    removeCallback(CB0);
    return;
  }
  assert(CB0->getCaller() == CB1->getCaller());
  InlineReportCallSite *IRCS = nullptr;
  if (shouldSkipCallBase(CB0))
    IRCS = addCallSite(CB1);
  else
    IRCS = getOrAddCallSite(CB0);
  IRCS->setCall(CB1);
  if (Function *Callee = CB1->getCalledFunction()) {
    InlineReportFunction *IRFC = getOrAddFunction(Callee);
    IRCS->setIRCallee(IRFC);
    if (UpdateReason) {
      if (Callee->isDeclaration()) {
        if (Callee->isIntrinsic())
          IRCS->setReason(NinlrIntrinsic);
        else
          IRCS->setReason(NinlrExtern);
      } else {
        IRCS->setReason(NinlrNewlyCreated);
      }
    }
  } else {
    IRCS->setIRCallee(nullptr);
    if (UpdateReason)
      IRCS->setReason(NinlrIndirect);
  }
  IRCallBaseCallSiteMap.erase(CB0);
  removeCallback(CB0);
  IRCallBaseCallSiteMap.insert(std::make_pair(CB1, IRCS));
  addCallback(CB1);
}

void InlineReport::cloneCallBaseToCallBase(CallBase *CB0, CallBase *CB1) {
  if (!isClassicIREnabled())
    return;
  if (CB0 == CB1 || shouldSkipCallBase(CB1))
    return;
  assert(CB0->getCaller() == CB1->getCaller());
  InlineReportCallSite *IRCS = getOrAddCallSite(CB0);
  InlineReportCallSite *NewIRCS = IRCS->copyBase(nullptr);
  NewIRCS->setCall(CB1);
  InlineReportFunction *IRCaller = IRCS->getIRCaller();
  NewIRCS->setIRCaller(IRCaller);
  if (Function *Callee = CB1->getCalledFunction()) {
    auto MapItF = IRFunctionMap.find(Callee);
    if (MapItF != IRFunctionMap.end()) {
      InlineReportFunction *IRCallee = MapItF->second;
      NewIRCS->setIRCallee(IRCallee);
    } else {
      NewIRCS->setIRCallee(nullptr);
    }
  } else {
    NewIRCS->setIRCallee(nullptr);
  }
  InlineReportCallSite *IRParent = IRCS->getIRParent();
  NewIRCS->setIRParent(IRParent);
  if (IRParent)
    IRParent->addChild(NewIRCS);
  else
    IRCaller->addCallSite(NewIRCS);
  IRCallBaseCallSiteMap.insert(std::make_pair(CB1, NewIRCS));
  addCallback(CB1);
}

void InlineReport::removeIRCS(InlineReportCallSite *IRCS) {
  if (IRCS->getIsInlined()) {
    for (auto *LIRCS : IRCS->getChildren())
      removeIRCS(LIRCS);
    IRCS->getChildren().clear();
  } else {
    auto MapIt = IRCallBaseCallSiteMap.find(IRCS->getCall());
    if (MapIt != IRCallBaseCallSiteMap.end())
      IRCallBaseCallSiteMap.erase(MapIt);
    removeCallback(IRCS->getCall());
  }
}

void InlineReport::deleteFunctionBody(Function *F) {
  if (!isClassicIREnabled())
    return;
  auto MapIt = IRFunctionMap.find(F);
  assert(MapIt != IRFunctionMap.end());
  InlineReportFunction *IRF = MapIt->second;
  for (auto *IRCS : IRF->getCallSites())
    removeIRCS(IRCS);
  IRF->getCallSites().clear();
}

void InlineReport::addMultiversionedCallSite(CallBase *CB) {
  if (!isClassicIREnabled())
    return;
  InlineReportCallSite *IRCS = addCallSite(CB);
  IRCS->setReason(NinlrMultiversionedCallsite);
}

void InlineReport::initModule(Module *M) {
  if (!isClassicIREnabled())
    return;
  for (auto &F : M->functions())
    initFunction(&F);
}

InlineReportCallSite *InlineReport::copyAndSetup(InlineReportCallSite *IRCS,
                                                 ValueToValueMapTy &VMap) {
  InlineReportCallSite *NewIRCS = IRCS->copyBase(nullptr);
  if (Instruction *I = IRCS->getCall()) {
    if (CallBase *CB = dyn_cast_or_null<CallBase>(VMap[I])) {
      NewIRCS->setCall(CB);
      IRCallBaseCallSiteMap.insert(std::make_pair(CB, NewIRCS));
      addCallback(CB);
    }
  }
  return NewIRCS;
}

void InlineReport::cloneCallSites(InlineReportCallSiteVector &IRCSV,
                                  ValueToValueMapTy &VMap,
                                  InlineReportCallSite *OldIRCS,
                                  InlineReportCallSite *NewIRCS) {
  for (InlineReportCallSite *IRCS : IRCSV) {
    InlineReportCallSite *ChildIRCS = copyAndSetup(IRCS, VMap);
    NewIRCS->addChild(ChildIRCS);
    cloneCallSites(ChildIRCS->getChildren(), VMap, IRCS, ChildIRCS);
  }
}

void InlineReport::cloneFunction(Function *OldFunction, Function *NewFunction,
                                 ValueToValueMapTy &VMap) {
  if (!isClassicIREnabled())
    return;
  auto MapIt = IRFunctionMap.find(OldFunction);
  if (MapIt == IRFunctionMap.end())
    return;
  InlineReportFunction *OldIRF = MapIt->second;
  InlineReportFunction *NewIRF = addFunction(NewFunction);
  // Clone callsites.
  for (InlineReportCallSite *IRCS : OldIRF->getCallSites()) {
    InlineReportCallSite *NewIRCS = copyAndSetup(IRCS, VMap);
    NewIRF->addCallSite(NewIRCS);
    cloneCallSites(IRCS->getChildren(), VMap, IRCS, NewIRCS);
  }
  NewIRF->cloneCompactInfo(OldIRF);
}

InlineReportCallSite *InlineReport::getCallSite(CallBase *Call) {
  if (!isClassicIREnabled())
    return nullptr;
  auto MapItC = IRCallBaseCallSiteMap.find(Call);
  if (MapItC == IRCallBaseCallSiteMap.end())
    return nullptr;
  return MapItC->second;
}

void InlineReport::setCalledFunction(CallBase *CB, Function *F) {
  auto MapItC = IRCallBaseCallSiteMap.find(CB);
  if (MapItC == IRCallBaseCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapItC->second;
  auto MapIt = IRFunctionMap.find(F);
  if (MapIt == IRFunctionMap.end())
    return;
  InlineReportFunction *IRF = MapIt->second;
  IRCS->setIRCallee(IRF);
}

InlineReport::~InlineReport(void) {
  while (!CallbackMap.empty()) {
    auto MapIt = CallbackMap.begin();
    CallbackMap.erase(MapIt);
    delete MapIt->second;
  }
  InlineReportFunctionMap::const_iterator FI, FE;
  for (FI = IRFunctionMap.begin(), FE = IRFunctionMap.end(); FI != FE; ++FI)
    delete FI->second;
}

void InlineReport::removeCallBasesInBasicBlocks(
    SmallSetVector<BasicBlock *, 8> &BlocksToRemove) {
  if (!isClassicIREnabled())
    return;
  for (BasicBlock *BB : BlocksToRemove)
    for (Instruction &I : *BB)
      if (auto CB = dyn_cast<CallBase>(&I))
        removeCallBaseReference(*CB, NinlrDeletedDeadCode);
}

bool InlineReport::shouldSkipCallBase(CallBase *CB) {
  return isa<IntrinsicInst>(CB) && !(Level & DontSkipIntrin) &&
         shouldSkipIntrinsic(cast<IntrinsicInst>(CB));
}

extern cl::opt<unsigned> IntelInlineReportLevel;

InlineReport *llvm::getInlineReport() {
  static llvm::InlineReport *SavedInlineReport = nullptr;
  if (!SavedInlineReport)
    SavedInlineReport = new llvm::InlineReport(IntelInlineReportLevel);
  return SavedInlineReport;
}

InlineReportPass::InlineReportPass(void) {}

char InlineReportPass::PassID;

PreservedAnalyses InlineReportPass::run(Module &M, ModuleAnalysisManager &AM) {
  for (Function &F : M)
    getInlineReport()->initFunction(&F);
  getInlineReport()->print();
  getInlineReport()->~InlineReport();
  return PreservedAnalyses::all();
}

InlineReportMakeCurrentPass::InlineReportMakeCurrentPass(void) {}

PreservedAnalyses
InlineReportMakeCurrentPass::run(Function &F, FunctionAnalysisManager &AM) {
  InlineReport *IR = getInlineReport();
  if (!IR->isClassicIREnabled())
    return PreservedAnalyses::all();
  if (IR->makeCurrent(&F))
    return PreservedAnalyses::none();
  return PreservedAnalyses::all();
}
