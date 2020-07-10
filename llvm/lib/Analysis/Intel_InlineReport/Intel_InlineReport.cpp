//===- Intel_InlineReport.cpp - Inline report ------- ---------------------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Transforms/IPO/Utils/Intel_IPOUtils.h"

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

InlineReportCallSite *
InlineReportCallSite::copyBase(const InlineReportCallSite &Base,
                               Instruction *NI) {
  InlineReportCallSite *NewCS = new InlineReportCallSite(
      Base.IRCallee, Base.IsInlined, Base.Reason, Base.M, nullptr, NI);
  NewCS->IsInlined = Base.IsInlined;
  NewCS->InlineCost = Base.InlineCost;
  NewCS->OuterInlineCost = Base.OuterInlineCost;
  NewCS->InlineThreshold = Base.InlineThreshold;
  NewCS->Line = Base.Line;
  NewCS->Col = Base.Col;
  NewCS->Children.clear();
  return NewCS;
}

InlineReportCallSite *
InlineReportCallSite::cloneBase(const ValueToValueMapTy &IIMap,
                                Instruction *ActiveInlineInstruction) {
  if (IsInlined) {
    InlineReportCallSite *IRCSk = copyBase(*this, nullptr);
    return IRCSk;
  }
  const Value *OldCall = this->getCall();
  if (OldCall == nullptr)
    return nullptr;
  // If the OldCall was the ActiveInlineInstruction, we placed a nullptr
  // into the IIMap for it.
  bool IsRecursiveCopy = OldCall == ActiveInlineInstruction;
  if (IsRecursiveCopy)
    OldCall = nullptr;
  ValueToValueMapTy::const_iterator VMI = IIMap.find(OldCall);
  if (VMI == IIMap.end())
    return nullptr;
  if (VMI->second == nullptr)
    return nullptr;
  Instruction *NI = cast<Instruction>(VMI->second);
  InlineReportCallSite *IRCSk = nullptr;
  if (IsRecursiveCopy) {
    // Start with a clean copy, as this is a newly created callsite produced
    // by recursive inlining.
    IRCSk = new InlineReportCallSite(this->IRCallee, false, NinlrNoReason,
                                     this->M, nullptr, NI);
    IRCSk->Line = this->Line;
    IRCSk->Col = this->Col;
  } else
    IRCSk = copyBase(*this, NI);
  return IRCSk;
}

///
/// \brief Print a simple message
///
/// message: The message being printed
/// indentCount: The number of indentations before printing the message
/// level: The level N from '-inline-report=N'
///
static void printSimpleMessage(const char *Message, unsigned IndentCount,
                               unsigned Level, bool IsInline) {
#if !INTEL_PRODUCT_RELEASE
  if (Level & InlineReportOptions::Reasons) {
    if (Level & InlineReportOptions::SameLine) {
      llvm::errs() << " ";
    } else {
      llvm::errs() << "\n";
      printIndentCount(IndentCount + 1);
    }
    llvm::errs() << (IsInline ? "<<" : "[[");
    llvm::errs() << Message;
    llvm::errs() << (IsInline ? ">>" : "]]");
    llvm::errs() << "\n";
    return;
  }
#endif // !INTEL_PRODUCT_RELEASE
  llvm::errs() << "\n";
}

///
/// \brief Print the inlining cost and threshold values
///
void InlineReportCallSite::printCostAndThreshold(unsigned Level) {
  llvm::errs() << " (" << getInlineCost();
  if (getIsInlined()) {
    llvm::errs() << "<=";
  } else {
    llvm::errs() << ">";
  }
  llvm::errs() << getInlineThreshold();
  if (((Level & InlineReportOptions::RealCost) != 0) && isEarlyExit() &&
      !getIsInlined()) {
    // Under RealCost flag we compute both real and "early exit" costs and
    // thresholds of inlining.
    llvm::errs() << " [EE:" << getEarlyExitInlineCost();
    llvm::errs() << ">";
    llvm::errs() << getEarlyExitInlineThreshold() << "]";
  }
  llvm::errs() << ")";
}

///
/// \brief Print the outer inlining cost and threshold values
///
void InlineReportCallSite::printOuterCostAndThreshold(void) {
  llvm::errs() << " (" << getOuterInlineCost() << ">" << getInlineCost() << ">"
               << getInlineThreshold() << ")";
}

///
/// \brief Print the linkage info for a function 'F' as a single letter,
/// if the 'Level' specifies InlineReportOptions::Linkage.
/// For an explanation of the meaning of these letters,
/// see Intel_InlineReport.h.
///
static void printFunctionLinkage(unsigned Level, InlineReportFunction *IRF) {
  if (!(Level & InlineReportOptions::Linkage))
    return;
  llvm::errs() << IRF->getLinkageChar() << " ";
}

///
/// \brief Print the source language for a function 'F' as a single letter,
/// if the 'Level' specifies InlineReportOptions::Language.
/// For an explanation of the meaning of these letters,
/// see Intel_InlineReport.h.
///
static void printFunctionLanguage(unsigned Level, InlineReportFunction *IRF) {
  if (!(Level & InlineReportOptions::Language))
    return;
  llvm::errs() << IRF->getLanguageChar() << " ";
}

///
/// \brief Print optionally the callee linkage and language, and then the
/// callee name, and if non-zero, the line and column number of the call site
///
void InlineReportCallSite::printCalleeNameModuleLineCol(unsigned Level) {
  if (getIRCallee() != nullptr) {
    printFunctionLinkage(Level, getIRCallee());
    printFunctionLanguage(Level, getIRCallee());
    llvm::errs() << getIRCallee()->getName();
  }
  if (Level & InlineReportOptions::File)
    llvm::errs() << " " << M->getModuleIdentifier();
  if ((Level & InlineReportOptions::LineCol) && (Line != 0 || Col != 0))
    llvm::errs() << " (" << Line << "," << Col << ")";
}

///
/// \brief Print a representation of the inlining instance.
///
/// indentCount: The number of indentations to print
/// level: The level N from '-inline-report=N'
///
void InlineReportCallSite::print(unsigned IndentCount, unsigned Level) {
  assert(InlineReasonText[getReason()].Type != InlPrtNone);
  printIndentCount(IndentCount);
  if (getIsInlined()) {
    llvm::errs() << "-> INLINE: ";
    printCalleeNameModuleLineCol(Level);
    if (InlineReasonText[getReason()].Type == InlPrtCost) {
      printCostAndThreshold(Level);
    }
    printSimpleMessage(InlineReasonText[getReason()].Message, IndentCount,
                       Level, true);
  } else {
    if (InlineReasonText[getReason()].Type == InlPrtSpecial) {
      switch (getReason()) {
      case NinlrDeleted:
        llvm::errs() << "-> DELETE: ";
        printCalleeNameModuleLineCol(Level);
        llvm::errs() << "\n";
        break;
      case NinlrExtern:
        llvm::errs() << "-> EXTERN: ";
        printCalleeNameModuleLineCol(Level);
        llvm::errs() << "\n";
        break;
      case NinlrIndirect:
        llvm::errs() << "-> INDIRECT: ";
        printCalleeNameModuleLineCol(Level);
        printSimpleMessage(InlineReasonText[getReason()].Message, IndentCount,
                           Level, false);
        break;
      case NinlrOuterInlining:
        llvm::errs() << "-> ";
        printCalleeNameModuleLineCol(Level);
        printOuterCostAndThreshold();
        printSimpleMessage(InlineReasonText[getReason()].Message, IndentCount,
                           Level, false);
        break;
      default:
        assert(0);
      }
    } else {
      llvm::errs() << "-> ";
      printCalleeNameModuleLineCol(Level);
      if (InlineReasonText[getReason()].Type == InlPrtCost) {
        printCostAndThreshold(Level);
      }
      printSimpleMessage(InlineReasonText[getReason()].Message, IndentCount,
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

InlineReportFunction *InlineReport::addFunction(Function *F, Module *M) {
  if (!isClassicIREnabled())
    return nullptr;
  if (!F)
    return nullptr;

  InlineReportFunctionMap::const_iterator MapIt = IRFunctionMap.find(F);
  if (MapIt != IRFunctionMap.end()) {
    InlineReportFunction *IRF = MapIt->second;
    makeCurrent(M, F);
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
  return IRF;
}

InlineReportCallSite *InlineReport::addCallSite(Function *F, CallBase *Call,
                                                Module *M) {
  if (!isClassicIREnabled())
    return nullptr;
  if (!F)
    return nullptr;

  bool SuppressInlRpt = false;
  if (Call->getMetadata(IPOUtils::getSuppressInlineReportStringRef())) {
    LLVM_DEBUG(dbgs() << "Suppress inline report on: \n" << *Call << "\n";);
    SuppressInlRpt = true;
  }

  DebugLoc DLoc = Call->getDebugLoc();
  InlineReportFunctionMap::const_iterator MapIt = IRFunctionMap.find(F);
  assert(MapIt != IRFunctionMap.end());
  InlineReportFunction *IRF = MapIt->second;
  Function *Callee = Call->getCalledFunction();
  InlineReportFunction *IRFC = nullptr;
  if (Callee != nullptr) {
    InlineReportFunctionMap::const_iterator MapItC = IRFunctionMap.find(Callee);
    IRFC =
        MapItC == IRFunctionMap.end() ? addFunction(Callee, M) : MapItC->second;
  }
  InlineReportCallSite *IRCS =
      new InlineReportCallSite(IRFC, false, NinlrNoReason, M, &DLoc,
                               Call, SuppressInlRpt);
  IRF->addCallSite(IRCS);
  IRInstructionCallSiteMap.insert(std::make_pair(Call, IRCS));
  addCallback(Call);
  return IRCS;
}

InlineReportCallSite *InlineReport::addNewCallSite(Function *F, CallBase *Call,
                                                   Module *M) {
  if (!isClassicIREnabled())
    return nullptr;
  InlineReportCallSite *IRCS = getCallSite(Call);
  if (IRCS != nullptr)
    return IRCS;
  return addCallSite(F, Call, M);
}

void InlineReport::beginSCC(CallGraph &CG, CallGraphSCC &SCC) {
  if (!isClassicIREnabled())
    return;
  M = &CG.getModule();
  for (CallGraphNode *Node : SCC) {
    Function *F = Node->getFunction();
    beginFunction(F);
  }
}

void InlineReport::beginSCC(LazyCallGraph &CG, LazyCallGraph::SCC &SCC) {
  if (!isClassicIREnabled())
    return;
  M = &CG.getModule();
  for (auto &Node : SCC) {
    Function &F = Node.getFunction();
    beginFunction(&F);
  }
}

void InlineReport::beginFunction(Function *F) {
  if (!F || F->isDeclaration())
    return;
  InlineReportFunction *IRF = addFunction(F, M);
  assert(IRF != nullptr);
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
      addNewCallSite(F, Call, M);
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
    const InlineReportCallSiteVector &OldCallSiteVector,
    InlineReportCallSite *NewCallSite, ValueToValueMapTy &IIMap) {
  assert(NewCallSite->getChildren().empty());
  for (unsigned I = 0, E = OldCallSiteVector.size(); I < E; ++I) {
    InlineReportCallSite *IRCSj = OldCallSiteVector[I];
    //
    // Copy the old InlineReportCallSite and add it to the children of the
    // cloned InlineReportCallSite.
    InlineReportCallSite *IRCSk = IRCSj->cloneBase(IIMap,
                                                   ActiveInlineInstruction);
    if (IRCSk == nullptr)
      continue;
    NewCallSite->addChild(IRCSk);
    //
    // We keep track of the new calls that are added added to the inline
    // report in case they themselves will be inlined.
    if (IRCSk->getCall() != nullptr) {
      IRInstructionCallSiteMap.insert(std::make_pair(IRCSk->getCall(), IRCSk));
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
  InlineReportFunction *INR = addFunction(ActiveCallee, M);
  //
  // Ensure that the report is up to date since the last call to
  // Inliner::runOnSCC
  makeCurrent(M, ActiveCallee);
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
    // ActiveInlineInstruction into the IIMap, as it is an already
    // deleted value. Putting it in the IIMAP could cause a ValueHandle
    // to be associated with it, which should not happen because it is
    // deleted.
    Value *OC = ActiveOriginalCalls[I] == ActiveInlineInstruction ?
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
  // Remove the inlined instruction from the IRInstructionCallSiteMap
  InlineReportInstructionCallSiteMap::const_iterator MapIt;
  MapIt = IRInstructionCallSiteMap.find(ActiveInlineInstruction);
  assert(MapIt != IRInstructionCallSiteMap.end());
  IRInstructionCallSiteMap.erase(MapIt);
  ActiveIRCS->setCall(nullptr);
}

void InlineReport::setReasonIsInlined(CallBase *Call, InlineReason Reason) {
  if (!isClassicIREnabled())
    return;
  assert(IsInlinedReason(Reason));
  InlineReportInstructionCallSiteMap::const_iterator MapIt =
      IRInstructionCallSiteMap.find(Call);
  // The new call site may have already been deleted by dead code elimination.
  if (MapIt == IRInstructionCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapIt->second;
  IRCS->setReason(Reason);
}

void InlineReport::setReasonIsInlined(CallBase *Call,
                                      const InlineCost &IC) {
  if (!isClassicIREnabled())
    return;
  assert(IsInlinedReason(IC.getInlineReason()));
  InlineReportInstructionCallSiteMap::const_iterator MapIt =
      IRInstructionCallSiteMap.find(Call);
  // The new call site may have already been deleted by dead code elimination.
  if (MapIt == IRInstructionCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapIt->second;
  IRCS->setReason(IC.getInlineReason());
  IRCS->setInlineCost(IC.getCost());
  IRCS->setInlineThreshold(IC.getCost() + IC.getCostDelta());
}

void InlineReport::setReasonNotInlined(CallBase *Call,
                                       InlineReason Reason) {
  if (!isClassicIREnabled())
    return;
  assert(IsNotInlinedReason(Reason));
  InlineReportInstructionCallSiteMap::const_iterator MapIt =
      IRInstructionCallSiteMap.find(Call);
  // The new call site may have already been deleted by dead code elimination.
  if (MapIt == IRInstructionCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapIt->second;
  IRCS->setReason(Reason);
}

void InlineReport::setReasonNotInlined(CallBase *Call,
                                       const InlineCost &IC) {
  if (!isClassicIREnabled())
    return;
  InlineReason Reason = IC.getInlineReason();
  assert(IsNotInlinedReason(Reason));
  InlineReportInstructionCallSiteMap::const_iterator MapIt =
      IRInstructionCallSiteMap.find(Call);
  // The new call site may have already been deleted by dead code elimination.
  if (MapIt == IRInstructionCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapIt->second;
  IRCS->setReason(Reason);
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
  InlineReportInstructionCallSiteMap::const_iterator MapIt =
      IRInstructionCallSiteMap.find(Call);
  // The new call site may have already been deleted by dead code elimination.
  if (MapIt == IRInstructionCallSiteMap.end())
    return;
  InlineReportCallSite *IRCS = MapIt->second;
  IRCS->setOuterInlineCost(TotalSecondaryCost);
}

///
/// \brief Print the callsites in the 'Vector'
///
/// indentCount: The number of indentations to print
/// level: The level N from '-inline-report=N'
///
static void
printInlineReportCallSiteVector(const InlineReportCallSiteVector &Vector,
                                unsigned IndentCount, unsigned Level) {
  for (unsigned I = 0, E = Vector.size(); I < E; ++I) {
    InlineReportCallSite *IRCS = Vector[I];
    if (IRCS->getSuppressPrint())
      continue;
    IRCS->print(IndentCount, Level);
    printInlineReportCallSiteVector(IRCS->getChildren(), IndentCount + 1,
                                    Level);
  }
}

void InlineReportFunction::print(unsigned Level) const {
  if (!Level || (Level & InlineReportTypes::BasedOnMetadata))
    return;
  printInlineReportCallSiteVector(CallSites, 1, Level);
}

void InlineReport::print(void) const {
  if (!isClassicIREnabled())
    return;
  llvm::errs() << "---- Begin Inlining Report ----\n";
  printOptionValues();
  for (unsigned I = 0, E = IRDeadFunctionVector.size(); I < E; ++I) {
    InlineReportFunction *IRF = IRDeadFunctionVector[I];
    // Suppress inline report on any Function with Suppress mark on
    if (IRF->getSuppressPrint())
      continue;
    llvm::errs() << "DEAD STATIC FUNC: ";
    printFunctionLinkage(Level, IRF);
    printFunctionLanguage(Level, IRF);
    llvm::errs() << IRF->getName() << "\n\n";
  }

  InlineReportFunctionMap::const_iterator Mit, E;
  for (Mit = IRFunctionMap.begin(), E = IRFunctionMap.end(); Mit != E; ++Mit) {
    Function *F = Mit->first;
    // Update the linkage info one last time before printing,
    // as it may have changed.
    InlineReportFunction *IRF = Mit->second;
    IRF->setLinkageChar(F);
    IRF->setLanguageChar(F);
    if (IRF->getSuppressPrint())
      continue;
    if (!IRF->getIsDeclaration()) {
      llvm::errs() << "COMPILE FUNC: ";
      printFunctionLinkage(Level, IRF);
      printFunctionLanguage(Level, IRF);
      llvm::errs() << IRF->getName() << "\n";
      InlineReportFunction *IRF = Mit->second;
      IRF->print(Level);
      llvm::errs() << "\n";
    }
  }
  llvm::errs() << "---- End Inlining Report ------\n";
}

void InlineReportCallSite::loadCallsToMap(std::map<Instruction *, bool> &LMap) {
  Instruction *NI = getCall();
  if (NI != nullptr)
    LMap.insert(std::make_pair(NI, true));
  for (unsigned I = 0, E = Children.size(); I < E; ++I)
    Children[I]->loadCallsToMap(LMap);
}

#ifndef NDEBUG
bool InlineReport::validateFunction(Function *F) {
  llvm::errs() << "Validating " << F->getName() << "\n";
  bool ReturnValue = true;
  InlineReportFunctionMap::const_iterator MapIt;
  MapIt = IRFunctionMap.find(F);
  if (MapIt == IRFunctionMap.end())
    return false;
  InlineReportFunction *IRF = MapIt->second;
  IRF->print(Level);
  std::map<Instruction *, bool> OriginalCalls;
  const InlineReportCallSiteVector &Vec = IRF->getCallSites();
  for (unsigned I = 0, E = Vec.size(); I < E; ++I)
    Vec[I]->loadCallsToMap(OriginalCalls);
  for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
    for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
      auto *Call = dyn_cast<CallBase>(&*I);
      if (!Call)
        continue;
      Instruction *NI = Call;
      std::map<Instruction *, bool>::const_iterator MapIt;
      MapIt = OriginalCalls.find(NI);
      if (MapIt == OriginalCalls.end()) {
        ReturnValue = false;
        llvm::errs() << "Cannot find " << NI << "\n";
        NI->dump();
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
    if (LocalRv) {
      llvm::errs() << " passed\n";
    } else {
      llvm::errs() << " failed\n";
    }
    GlobalRv &= LocalRv;
  }
  llvm::errs() << "End Validation Pass\n";
  return GlobalRv;
}
#endif // NDEBUG

void InlineReport::makeCurrent(Module *M, Function *F) {
  InlineReportFunctionMap::const_iterator MapIt = IRFunctionMap.find(F);
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
      if (!Call) {
        continue;
      }
      if (isa<IntrinsicInst>(I) && !(Level & DontSkipIntrin) &&
          shouldSkipIntrinsic(cast<IntrinsicInst>(I)))
        continue;
      InlineReportInstructionCallSiteMap::const_iterator MapItICS;
      MapItICS = IRInstructionCallSiteMap.find(Call);
      if (MapItICS != IRInstructionCallSiteMap.end()) {
        continue;
      }
      InlineReportCallSite *IRCS = addCallSite(F, Call, M);
      assert(IRCS != nullptr);
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

void InlineReport::replaceFunctionWithFunction(Function *OldFunction,
                                               Function *NewFunction) {
  InlineReportFunctionMap::const_iterator IrfIt;
  if (OldFunction == NewFunction)
    return;
  IrfIt = IRFunctionMap.find(OldFunction);
  if (IrfIt == IRFunctionMap.end())
    return;
  InlineReportFunction *IRF = IrfIt->second;
  int count = IRFunctionMap.erase(OldFunction);
  (void) count;
  assert(count == 1);
  IRFunctionMap.insert(std::make_pair(NewFunction, IRF));
  IRF->setLinkageChar(NewFunction);
  IRF->setLanguageChar(NewFunction);
  IRF->setName(std::string(NewFunction->getName()));
  addCallback(NewFunction);
}

InlineReportCallSite *InlineReport::getCallSite(CallBase *Call) {
  if (!isClassicIREnabled())
    return nullptr;
  InlineReportInstructionCallSiteMap::const_iterator MapItC =
      IRInstructionCallSiteMap.find(Call);
  if (MapItC == IRInstructionCallSiteMap.end())
    return nullptr;
  return MapItC->second;
}

InlineReport::~InlineReport(void) {
  while (!IRCallbackVector.empty()) {
    InlineReportCallback *IRCB = IRCallbackVector.back();
    IRCallbackVector.pop_back();
    delete IRCB;
  }
  InlineReportFunctionMap::const_iterator FI, FE;
  for (FI = IRFunctionMap.begin(), FE = IRFunctionMap.end(); FI != FE; ++FI)
    delete FI->second;
}
