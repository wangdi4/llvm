//===- Intel_InlineReport.cpp - Inline report ------- ---------------------===//
//
// Copyright (C) 2015-2022 Intel Corporation. All rights reserved.
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

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Transforms/IPO/Utils/Intel_IPOUtils.h"
#include "llvm/IR/InstIterator.h"

using namespace llvm;
using namespace InlineReportTypes;

#define DEBUG_TYPE "intel-inlinereport"

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

InlineReportCallSite *InlineReportCallSite::copyBase(CallBase *CB) {
  InlineReportCallSite *NewCS = new InlineReportCallSite(IRCallee, IsInlined,
      Reason, M, nullptr, CB);
  NewCS->Reason = Reason;
  NewCS->InlineCost = InlineCost;
  NewCS->OuterInlineCost = OuterInlineCost;
  NewCS->InlineThreshold = InlineThreshold;
  NewCS->Line = Line;
  NewCS->Col = Col;
  NewCS->Children.clear();
  return NewCS;
}

InlineReportCallSite *
InlineReportCallSite::cloneBase(const ValueToValueMapTy &IIMap,
                                CallBase *ActiveInlineCallBase) {
  if (IsInlined) {
    InlineReportCallSite *IRCSk = copyBase(nullptr);
    return IRCSk;
  }
  const Value *OldCall = this->getCall();
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
    IRCSk = new InlineReportCallSite(this->IRCallee, false, NinlrNoReason,
                                     CB->getFunction()->getParent(),
                                     nullptr, CB);
    IRCSk->Line = this->Line;
    IRCSk->Col = this->Col;
  } else {
    IRCSk = copyBase(CB);
    if (!CB)
      IRCSk->setReason(NinlrDeleted);
  }
  return IRCSk;
}

///
/// Print a simple message.
///
/// OS: The output stream to print to
/// message: The message being printed
/// indentCount: The number of indentations before printing the message
/// level: The level N from '-inline-report=N'
///
static void printSimpleMessage(formatted_raw_ostream &OS, const char *Message,
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
/// Print the inlining cost and threshold values
///
void InlineReportCallSite::printCostAndThreshold(formatted_raw_ostream &OS,
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
void InlineReportCallSite::printOuterCostAndThreshold(formatted_raw_ostream &OS,
                                                      unsigned Level) {
  if (!(Level & InlineReportOptions::EarlyExitCost))
    return;
  OS << " (" << getOuterInlineCost() << ">" << getInlineCost() << ">"
     << getInlineThreshold() << ")";
}

///
/// Print the linkage info for a function 'F' as a single letter,
/// if the 'Level' specifies InlineReportOptions::Linkage.
/// For an explanation of the meaning of these letters,
/// see Intel_InlineReport.h.
///
static void printFunctionLinkage(formatted_raw_ostream &OS, unsigned Level,
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
static void printFunctionLanguage(formatted_raw_ostream &OS, unsigned Level,
                                  InlineReportFunction *IRF) {
  if (!(Level & InlineReportOptions::Language))
    return;
  OS << IRF->getLanguageChar() << " ";
}

///
/// Print optionally the callee linkage and language, and then the
/// callee name, and if non-zero, the line and column number of the call site
///
void InlineReportCallSite::printCalleeNameModuleLineCol(
    formatted_raw_ostream &OS, unsigned Level) {
  if (getIRCallee()) {
    printFunctionLinkage(OS, Level, getIRCallee());
    printFunctionLanguage(OS, Level, getIRCallee());
    OS << getIRCallee()->getName();
  }
  if (Level & InlineReportOptions::File)
    OS << " " << M->getModuleIdentifier();
  if ((Level & InlineReportOptions::LineCol) && (Line != 0 || Col != 0))
    OS << " (" << Line << "," << Col << ")";
}

///
/// Print a representation of the inlining instance.
///
/// OS: The output stream to print to
/// indentCount: The number of indentations to print
/// level: The level N from '-inline-report=N'
///
void InlineReportCallSite::print(formatted_raw_ostream &OS,
                                 unsigned IndentCount, unsigned Level) {
  assert(InlineReasonText[getReason()].Type != InlPrtNone);
  if (getIsInlined()) {
    printIndentCount(OS, IndentCount);
    OS << "-> INLINE: ";
    printCalleeNameModuleLineCol(OS, Level);
    if (InlineReasonText[getReason()].Type == InlPrtCost)
      printCostAndThreshold(OS, Level);
    printSimpleMessage(OS, InlineReasonText[getReason()].Message, IndentCount,
                       Level, true);
  } else {
    if (InlineReasonText[getReason()].Type == InlPrtSpecial) {
      switch (getReason()) {
      case NinlrDeleted:
        printIndentCount(OS, IndentCount);
        OS << "-> DELETE: ";
        printCalleeNameModuleLineCol(OS, Level);
        OS << "\n";
        break;
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
          OS << "-> INDIRECT: ";
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
      default:
        assert(0);
      }
    } else {
      printIndentCount(OS, IndentCount);
      OS << "-> ";
      printCalleeNameModuleLineCol(OS, Level);
      if (InlineReasonText[getReason()].Type == InlPrtCost)
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

InlineReportFunction *InlineReport::addFunction(Function *F,
                                                bool MakeNewCurrent) {
  if (!isClassicIREnabled())
    return nullptr;
  if (!F)
    return nullptr;

  auto MapIt = IRFunctionMap.find(F);
  if (MapIt != IRFunctionMap.end()) {
    InlineReportFunction *IRF = MapIt->second;
    makeCurrent(F);
    return IRF;
  }

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
  if (MakeNewCurrent)
    makeCurrent(F);
  return IRF;
}

static void addOutlinedIRCSes(InlineReportCallSiteVector &IRCSV,
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
    SmallPtrSetImpl<InlineReportCallSite*> &OutFCBSet,
    InlineReportCallSite *NewIRCS) {
  for (InlineReportCallSite *IRCS : IRCSV) {
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

void InlineReportFunction::moveOutlinedCallSites(InlineReportFunction *NewIRF,
    SmallPtrSetImpl<InlineReportCallSite*> &OutFCBSet) {
  for (InlineReportCallSite *IRCS : getCallSites()) {
    if (OutFCBSet.count(IRCS)) {
      if (IRCS->getCall()) {
        IRCS->move(this, NewIRF);
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

InlineReportCallSite *InlineReport::addCallSite(CallBase *Call) {
  if (!isClassicIREnabled())
    return nullptr;

  bool SuppressInlRpt = false;
  if (Call->getMetadata(IPOUtils::getSuppressInlineReportStringRef())) {
    LLVM_DEBUG(dbgs() << "Suppress inline report on: \n" << *Call << "\n";);
    SuppressInlRpt = true;
  }

  DebugLoc DLoc = Call->getDebugLoc();
  Function *F = Call->getCaller();
  auto MapIt = IRFunctionMap.find(F);
  assert(MapIt != IRFunctionMap.end());
  InlineReportFunction *IRF = MapIt->second;
  Function *Callee = Call->getCalledFunction();
  InlineReportFunction *IRFC = nullptr;
  if (Callee) {
    auto MapItC = IRFunctionMap.find(Callee);
    IRFC =
        MapItC == IRFunctionMap.end() ? addFunction(Callee) : MapItC->second;
  }
  InlineReportCallSite *IRCS =
      new InlineReportCallSite(IRFC, false, NinlrNoReason,
                               Call->getFunction()->getParent(), &DLoc,
                               Call, SuppressInlRpt);
  IRF->addCallSite(IRCS);
  IRCallBaseCallSiteMap.insert(std::make_pair(Call, IRCS));
  addCallback(Call);
  return IRCS;
}

InlineReportCallSite *InlineReport::addNewCallSite(CallBase *Call) {
  if (!isClassicIREnabled())
    return nullptr;
  InlineReportCallSite *IRCS = getCallSite(Call);
  if (IRCS)
    return IRCS;
  return addCallSite(Call);
}

void InlineReport::beginSCC(CallGraphSCC &SCC, void *Inliner) {
  if (!isClassicIREnabled())
    return;
  ActiveInliners.insert(Inliner);
  M = &SCC.getCallGraph().getModule();
  for (CallGraphNode *Node : SCC) {
    Function *F = Node->getFunction();
    if (!F)
      continue;
    beginFunction(F);
  }
}

void InlineReport::beginSCC(LazyCallGraph::SCC &SCC, void *Inliner) {
  if (!isClassicIREnabled())
    return;
  ActiveInliners.insert(Inliner);
  LazyCallGraph::Node &LCGN = *(SCC.begin());
  M = LCGN.getFunction().getParent();
  for (auto &Node : SCC) {
    Function &F = Node.getFunction();
    beginFunction(&F);
  }
}

void InlineReport::beginFunction(Function *F) {
  if (!F || F->isDeclaration())
    return;
  InlineReportFunction *IRF = addFunction(F);
  assert(IRF);
  for (BasicBlock &BB : *F) {
    for (Instruction &I : BB) {
      CallBase *Call = dyn_cast<CallBase>(&I);
      // If this isn't a call, or it is a call to an intrinsic, it can
      // never be inlined.
      if (!Call)
        continue;
      if (isa<IntrinsicInst>(I) && !(Level & DontSkipIntrin) &&
          shouldSkipIntrinsic(cast<IntrinsicInst>(&I)))
        continue;
      addNewCallSite(Call);
      if (isa<IntrinsicInst>(I)) {
        setReasonNotInlined(Call, NinlrIntrinsic);
        continue;
      }
      // If this is a direct call to an external function, we can never
      // inline it.  If it is an indirect call, inlining may resolve it to be
      // a direct call, so we keep it.
      if (Function *Callee = Call->getCalledFunction())
        if (Callee->isDeclaration()) {
          setReasonNotInlined(Call, NinlrExtern);
          continue;
        }
    }
  }
  IRF->setCurrent(true);
}

void InlineReport::endSCC(void) {
  if (!isClassicIREnabled())
    return;
  makeAllNotCurrent();
}

void InlineReport::cloneChildren(
    InlineReportCallSiteVector &OldCallSiteVector,
    InlineReportCallSite *NewCallSite, ValueToValueMapTy &IIMap) {
  assert(NewCallSite->getChildren().empty());
  for (unsigned I = 0, E = OldCallSiteVector.size(); I < E; ++I) {
    InlineReportCallSite *IRCSj = OldCallSiteVector[I];
    //
    // Copy the old InlineReportCallSite and add it to the children of the
    // cloned InlineReportCallSite.
    InlineReportCallSite *IRCSk = IRCSj->cloneBase(IIMap,
                                                   ActiveInlineCallBase);
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
  // to make a clone of it.
  InlineReportFunction *INR = addFunction(ActiveCallee);
  //
  // Ensure that the report is up to date since the last call to
  // Inliner::runOnSCC
  makeCurrent(ActiveCallee);
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
    // deleted value. Putting it in the IIMAP could cause a ValueHandle
    // to be associated with it, which should not happen because it is
    // deleted.
    Value *OC = ActiveOriginalCalls[I] == ActiveInlineCallBase ?
                nullptr : ActiveOriginalCalls[I];
    Value *NC = ActiveInlinedCalls[I];
    IIMap.insert(std::make_pair(OC, NC));
  }
  //
  // Clone the inline report INR and attach it to the inlined call site IRCS.
  // Use IIMap to map the original calls to the new calls in the cloned
  // inline report.
  cloneChildren(INR->getCallSites(), ActiveIRCS, IIMap);
  // Indicate that the call has been inlined in the inline report
  ActiveIRCS->setIsInlined(true);
  //
  // Remove the inlined instruction from the IRCallBaseCallSiteMap
  auto MapIt = IRCallBaseCallSiteMap.find(ActiveInlineCallBase);
  assert(MapIt != IRCallBaseCallSiteMap.end());
  IRCallBaseCallSiteMap.erase(MapIt);
  ActiveIRCS->setCall(nullptr);
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

void InlineReport::setReasonIsInlined(CallBase *Call,
                                      const InlineCost &IC) {
  if (!isClassicIREnabled())
    return;
  assert(IsInlinedReason(IC.getInlineReason()));
  auto MapIt = IRCallBaseCallSiteMap.find(Call);
  // The new call site may have already been deleted by dead code elimination.
  if (MapIt == IRCallBaseCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapIt->second;
  IRCS->setReason(IC.getInlineReason());
  if (IC.isAlways())
    return;
  IRCS->setInlineCost(IC.getCost());
  IRCS->setInlineThreshold(IC.getCost() + IC.getCostDelta());
}

void InlineReport::setReasonNotInlined(CallBase *Call,
                                       InlineReason Reason) {
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

void InlineReport::setReasonNotInlined(CallBase *Call,
                                       const InlineCost &IC) {
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
printInlineReportCallSiteVector(formatted_raw_ostream &OS,
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

void InlineReportFunction::print(formatted_raw_ostream &OS,
                                 unsigned Level) const {
  if (!Level || (Level & InlineReportTypes::BasedOnMetadata))
    return;
  printInlineReportCallSiteVector(OS, CallSites, 1, Level);
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
      OS << IRF->getName() << "\n\n";
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
      OS << IRF->getName() << "\n";
      InlineReportFunction *IRF = Mit->second;
      IRF->print(OS, Level);
      OS << "\n";
    }
  }
  OS << "---- End Inlining Report ------\n";
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

void InlineReport::makeCurrent(Function *F) {
  auto MapIt = IRFunctionMap.find(F);
  assert(MapIt != IRFunctionMap.end());
  InlineReportFunction *IRF = MapIt->second;
  if (IRF->getCurrent())
    return;
  if (F->isDeclaration()) {
    IRF->setCurrent(true);
    return;
  }
  for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
    for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
      CallBase *Call = dyn_cast<CallBase>(I);
      if (!Call)
        continue;
      if (isa<IntrinsicInst>(I) && !(Level & DontSkipIntrin) &&
          shouldSkipIntrinsic(cast<IntrinsicInst>(I)))
        continue;
      auto MapItICS = IRCallBaseCallSiteMap.find(Call);
      if (MapItICS != IRCallBaseCallSiteMap.end())
        continue;
      InlineReportCallSite *IRCS = addCallSite(Call);
      assert(IRCS);
      IRCS->setReason(NinlrNewlyCreated);
    }
  }
  IRF->setCurrent(true);
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
      InlineReportCallSite *IRCS = getCallSite(CB);
      IRCS->setIRCallee(IRFNew);
    }
  }
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
  auto IrfIt = IRFunctionMap.find(OldFunction);
  if (IrfIt == IRFunctionMap.end())
    return;
  InlineReportFunction *IRF = IrfIt->second;
  int count = IRFunctionMap.erase(OldFunction);
  (void) count;
  assert(count == 1);
  IRFunctionMap.insert(std::make_pair(NewFunction, IRF));
  replaceAllUsesWith(OldFunction, NewFunction);
  IRF->setLinkageChar(NewFunction);
  IRF->setLanguageChar(NewFunction);
  IRF->setName(std::string(NewFunction->getName()));
  removeCallback(OldFunction);
  addCallback(NewFunction);
}

void InlineReport::replaceCallBaseWithCallBase(CallBase *CB0, CallBase *CB1) {
  if (!isClassicIREnabled())
    return;
  if (CB0 == CB1)
    return;
  assert(CB0->getCaller() == CB1->getCaller());
  auto MapItCS = IRCallBaseCallSiteMap.find(CB0);
  if (MapItCS == IRCallBaseCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapItCS->second;
  IRCS->setCall(CB1);
  if (Function *Callee = CB1->getCalledFunction()) {
    auto MapItF = IRFunctionMap.find(Callee);
    if (MapItF != IRFunctionMap.end()) {
      InlineReportFunction *IRCallee = MapItF->second;
      IRCS->setIRCallee(IRCallee);
    } else {
      IRCS->setIRCallee(nullptr);
    }
  } else {
    IRCS->setIRCallee(nullptr);
  }
  IRCallBaseCallSiteMap.erase(MapItCS);
  IRCallBaseCallSiteMap.insert(std::make_pair(CB1, IRCS));
  removeCallback(CB0);
  addCallback(CB1);
}

void InlineReport::cloneCallBaseToCallBase(CallBase *CB0, CallBase *CB1) {
  if (!isClassicIREnabled())
    return;
  if (CB0 == CB1)
    return;
  assert(CB0->getCaller() == CB1->getCaller());
  auto MapItCS = IRCallBaseCallSiteMap.find(CB0);
  if (MapItCS == IRCallBaseCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapItCS->second;
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
  IRCS->setReason(InlineReportTypes::NinlrMultiversionedCallsite);
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

void InlineReport::cloneFunction(Function *OldFunction,
                                 Function *NewFunction,
                                 ValueToValueMapTy &VMap) {
  if (!isClassicIREnabled())
    return;
  auto MapIt = IRFunctionMap.find(OldFunction);
  if (MapIt == IRFunctionMap.end())
    return;
  InlineReportFunction *OldIRF = MapIt->second;
  InlineReportFunction *NewIRF = addFunction(NewFunction);
  for (InlineReportCallSite *IRCS : OldIRF->getCallSites()) {
    InlineReportCallSite *NewIRCS = copyAndSetup(IRCS, VMap);
    NewIRF->addCallSite(NewIRCS);
    cloneCallSites(IRCS->getChildren(), VMap, IRCS, NewIRCS);
  }
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
  return PreservedAnalyses::all();
}
