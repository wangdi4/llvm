//===--- Intel_MDInlineReport.h ----------------------------------*- C++
//-*-===//
//
// Copyright (C) 2019-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares InliningReport class using metadata.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_MDINLINEREPORT_H
#define LLVM_TRANSFORMS_IPO_INTEL_MDINLINEREPORT_H

#include "llvm/ADT/iterator_range.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Analysis/LazyCallGraph.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Transforms/IPO/Intel_InlineReportCommon.h"
#include "llvm/Transforms/IPO/Utils/Intel_IPOUtils.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#include <vector>

namespace llvm {

using namespace InlineReportTypes;

std::string getLinkageStr(Function *F);

std::string getLanguageStr(Function *F);

// Inlining report metadata tags and constants.
namespace MDInliningReport {

// Enumeration of the call site inlining report metadata fields.
enum CallSiteField {
  CSMDIR_Tag = 0,
  CSMDIR_CalleeName,
  CSMDIR_CSs,
  CSMDIR_IsInlined,
  CSMDIR_InlineReason,
  CSMDIR_InlineCost,
  CSMDIR_OuterInlineCost,
  CSMDIR_InlineThreshold,
  CSMDIR_EarlyExitCost,
  CSMDIR_EarlyExitThreshold,
  CSMDIR_LineAndColumn,
  CSMDIR_ModuleName,
  CSMDIR_SuppressPrintReport,
  CSMDIR_Last,
};

// Enumeration of the function inlining report metadata fields.
enum FuncField {
  FMDIR_Tag = 0,
  FMDIR_FuncName,
  FMDIR_CSs,
  FMDIR_ModuleName,
  FMDIR_IsDead,
  FMDIR_IsDeclaration,
  FMDIR_LinkageStr,
  FMDIR_LanguageStr,
  FMDIR_SuppressPrintReport,
  FMDIR_Last,
};

static constexpr const char *ModuleTag = "intel.module.inlining.report";
static constexpr const char *FunctionTag = "intel.function.inlining.report";
static constexpr const char *CallSitesTag = "intel.callsites.inlining.report";
static constexpr const char *CallSiteTag = "intel.callsite.inlining.report";
static constexpr const int FunctionMDSize = FMDIR_Last;
static constexpr const int CallSiteMDSize = CSMDIR_Last;
} // namespace MDInliningReport

// This class is needed to store Callback vector for functions and instructions
// of the current SCC during succeeding optimizations to keep inlining report
// consistent.
class InlineReportBuilder {
  // InlineFunction() fills this in with callsites which we are cloning from
  // the callee.  This is used only for inline report.
  SmallVector<Value *, 20> ActiveOriginalCalls;

  // InlineFunction() fills this in with callsites that were cloned from
  // the callee. This is used only for inline report. Each of these corresponds
  // 1-1 with an entry in ActiveOriginalCalls from which it is cloned.
  SmallVector<Value *, 20> ActiveInlinedCalls;

public:
  explicit InlineReportBuilder(unsigned MyLevel)
      : Level(MyLevel), CurrentCallInstr(nullptr),
        CurrentCallInstReport(nullptr), CurrentCallee(nullptr){};

  virtual ~InlineReportBuilder(void) {
    for (auto &IRCBEntry : IRCallbackMap)
      delete IRCBEntry.second;

    IRCallbackMap.clear();
  }

  // Walk over inlining reports for call instructions in current function to add
  // them to the callback vector.
  void beginFunction(Function *F);
  // Walk over inlining reports for functions in current SCC to add them to the
  // callback vector.
  void beginSCC(LazyCallGraph::SCC &SCC);
  void beginSCC(CallGraphSCC &SCC);

  // Mark function as dead in its inlining report.
  void setDead(Function *F);

  bool isMDIREnabled() { return Level & BasedOnMetadata; }
  // Create a clone of function inlining report.
  Metadata *cloneInliningReport(Function *F, ValueToValueMapTy &VMap);
  // Update inlining report for the inlined call site.
  void updateInliningReport();

  // Add a pair of old and new call sites.  The 'NewCall' is a clone of
  // the 'OldCall' produced by InlineFunction().
  void addActiveCallSitePair(Instruction *OldCall, Instruction *NewCall) {
    // If there were no metadata on the original instruction, we have nothing
    // to assign to the new instruction. Skip them.
    if (!OldCall->getMetadata(MDInliningReport::CallSiteTag))
      return;
    if (!NewCall)
      return;
    ActiveOriginalCalls.push_back(OldCall);
    ActiveInlinedCalls.push_back(NewCall);
    addCallback(NewCall);
  }

  // Update the 'OldCall' to 'NewCall' in the ActiveInlinedCalls.
  // This needs to happen if we manually replace an active inlined
  // call during InlineFunction().
  void updateActiveCallSiteTarget(Instruction *OldCall, Instruction *NewCall) {
    for (unsigned I = 0; I < ActiveInlinedCalls.size(); ++I)
      if (ActiveInlinedCalls[I] == OldCall) {
        ActiveInlinedCalls[I] = NewCall;
        removeCallback(OldCall);
        addCallback(NewCall);
        break;
      }
  }

  // Indicate that 'CB' has been eliminated as dead code with the
  // indicated reason.
  void removeCallBaseReference(
      CallBase &CB, InlineReason Reason = InlineReportTypes::NinlrDeleted) {
    MDNode *MDIR = CB.getMetadata(MDInliningReport::CallSiteTag);
    if (MDIR && CurrentCallInstr != &CB)
      if (auto *CSIR = dyn_cast<MDTuple>(MDIR)) {
        LLVMContext &Ctx = MDIR->getContext();
        std::string ReasonStr = "reason: ";
        ReasonStr.append(std::to_string(Reason));
        auto ReasonMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, ReasonStr));
        CSIR->replaceOperandWith(MDInliningReport::CSMDIR_InlineReason,
                                 ReasonMD);
        CB.setMetadata(MDInliningReport::CallSiteTag, nullptr);
      }
    // If necessary, remove any reference in the ActiveInlinedCalls
    for (unsigned II = 0, E = ActiveInlinedCalls.size(); II < E; ++II)
      if (ActiveInlinedCalls[II] == &CB)
        ActiveInlinedCalls[II] = nullptr;
  }

  // Indicate that 'F' has been eliminated as a dead static function.
  void removeFunctionReference(Function &F) {
    MDNode *MDIR = F.getMetadata(MDInliningReport::FunctionTag);
    if (!MDIR)
      return;
    if (auto *FIR = dyn_cast<MDTuple>(MDIR)) {
      LLVMContext &Ctx = MDIR->getContext();
      std::string IsDeadStr = "isDead: ";
      IsDeadStr.append(std::to_string(true));
      auto IsDeadMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, IsDeadStr));
      FIR->replaceOperandWith(MDInliningReport::FMDIR_IsDead, IsDeadMD);
    }
  }

  // Setup initial values for the single inlining step
  void beginUpdate(CallBase *Call) {
    CurrentCallee = Call->getCalledFunction();
    CurrentCallInstReport = Call->getMetadata(MDInliningReport::CallSiteTag);
    CurrentCallInstr = Call;
    ActiveOriginalCalls.clear();
    ActiveInlinedCalls.clear();
  }

  void endUpdate() {
    CurrentCallee = nullptr;
    CurrentCallInstReport = nullptr;
    CurrentCallInstr = nullptr;
    ActiveOriginalCalls.clear();
    ActiveInlinedCalls.clear();
  }
  // The level of the inline report
  unsigned getLevel() { return Level; }
  // The level of the inline report
  void setLevel(unsigned L) { Level = L; }

  // Replace 'OldFunction' with 'NewFunction'.
  void replaceFunctionWithFunction(Function *OldFunction,
                                   Function *NewFunction);

  // Replace 'OldFunction' with 'NewFunction'.
  void cloneFunction(Function *OldFunction, Function *NewFunction,
                     ValueToValueMapTy &VMap);

  // Replace 'OldCall' with 'NewCall'. If 'UpdateReason', update
  // the inlining reason based on the callee of 'NewCall'.
  void replaceCallBaseWithCallBase(CallBase *OldCall, CallBase *NewCall,
                                   bool UpdateReason = false);

  // Clone 'OldCall' to 'NewCall'.
  void cloneCallBaseToCallBase(CallBase *OldCall, CallBase *NewCall);

  // Set the callee of 'CB' to 'F'.
  void setCalledFunction(CallBase *CB, Function *F);

  // Delete all calls inside 'F'.
  void deleteFunctionBody(Function *F);

  // Indicate that 'CB' calls a specialized version of its caller under
  // a multiversioning test.
  void addMultiversionedCallSite(CallBase *CB);

  // Remove all of the CallBases in the 'BlocksToRemove' as dead code.
  void
  removeCallBasesInBasicBlocks(SmallSetVector<BasicBlock *, 8> &BlocksToRemove);

private:
  /// The Level is specified by the option -inline-report=N.
  /// See llvm/lib/Transforms/IPO/Inliner.cpp for details on Level.
  unsigned Level;
  // Call instruction which is considered on the current inlining step.
  Instruction *CurrentCallInstr;
  // Metadata inlining report for the current call instruction.
  Metadata *CurrentCallInstReport;
  // Function called in the current call instruction.
  Function *CurrentCallee;
  // Copy metadata 'OldMD' for use my another Function or CallBase.
  Metadata *copyMD(LLVMContext &C, Metadata *OldMD);

  ///
  /// CallbackVM for Instructions and Functions in the InlineReport
  ///
  class InliningReportCallback : public CallbackVH {
    InlineReportBuilder *IRB;
    void deleted() override {
      assert(IRB);
      if (auto CB = dyn_cast<CallBase>(getValPtr())) {
        /// Indicate in the inline report that the call site
        /// corresponding to the Value has been deleted
        IRB->removeCallBaseReference(*CB);
      } else if (auto F = dyn_cast<Function>(getValPtr())) {
        /// Indicate in the inline report that the function
        /// corresponding to the Value has been deleted
        IRB->removeFunctionReference(*F);
      }
      setValPtr(nullptr);
    }

  public:
    InliningReportCallback(Value *V, InlineReportBuilder *IRBPtr)
        : CallbackVH(V), IRB(IRBPtr){};

    void updateIRBuilder(InlineReportBuilder *NewIRB) { IRB = NewIRB; }
    InlineReportBuilder *getIRBuilder() { return IRB; }
    virtual ~InliningReportCallback(){};
  };

  SmallDenseMap<Value *, InliningReportCallback *, 16> IRCallbackMap;

public:
  // Add callback for function or instruction.
  void addCallback(Value *V) {
    if (!V || IRCallbackMap.count(V))
      return;
    InliningReportCallback *IRCB = new InliningReportCallback(V, this);
    IRCallbackMap.insert({V, IRCB});
  }

  // Remove callback from the map
  void removeCallback(Value *V) {
    if (!V || !IRCallbackMap.count(V))
      return;
    InliningReportCallback *IRCB = IRCallbackMap[V];
    IRCallbackMap.erase(V);
    delete IRCB;
  }

  // Copy the IRBuilder from SrcV to DstV, and update the active inlined calls
  void copyAndUpdateIRBuilder(Value *SrcV, Value *DstV) {
    if (!SrcV || !DstV)
      return;

    if (SrcV == DstV)
      return;

    if (IRCallbackMap.count(SrcV) == 0 || IRCallbackMap.count(DstV) == 0)
      return;

    IRCallbackMap[DstV]->updateIRBuilder(IRCallbackMap[SrcV]->getIRBuilder());

    // Update the active inlined calls if needed
    auto IRB = IRCallbackMap[DstV]->getIRBuilder();
    unsigned ActiveInlinedCallsSize = IRB->ActiveInlinedCalls.size();
    for (unsigned II = 0; II < ActiveInlinedCallsSize; ++II)
      if (IRB->ActiveInlinedCalls[II] == SrcV)
        IRB->ActiveInlinedCalls[II] = DstV;
  }
};

// Basic class for inlining report metadata
class InliningReport {
protected:
  MDTuple *Report;
  bool SuppressPrint; // flag to suppress inline-report print when true

public:
  InliningReport(MDTuple *R = nullptr, bool SuppressPrint = false)
      : Report(R), SuppressPrint(SuppressPrint) {}

  MDTuple *get() const { return Report; }

  StringRef getName() const {
    if (Report->getNumOperands() < 2)
      return "";
    return llvm::getOpStr(Report->getOperand(1), "name: ");
  }

  /// Get and set SuppressPrint
  bool getSuppressPrint(void) const { return SuppressPrint; }
  void setSuppressPrint(bool V) { SuppressPrint = V; }
};

// Class representing inlining report for function
class FunctionInliningReport : public InliningReport {
public:
  FunctionInliningReport(MDTuple *R = nullptr, bool SuppressPrint = false)
      : InliningReport(R, SuppressPrint) {
    assert((!R || isFunctionInliningReportMetadata(R)) &&
           "Bad function inlining report metadata");
  }

  FunctionInliningReport(LLVMContext *C, std::string FuncName,
                         std::vector<MDTuple *> *CSs, std::string ModuleName,
                         bool IsDead, bool isDeclaration, bool isSuppressPrint,
                         std::string LinkageChar, std::string LanguageChar);

  FunctionInliningReport(Function *F, std::vector<MDTuple *> *CSs, bool IsDead)
      : FunctionInliningReport(
            &(F->getParent()->getContext()),
            std::string(F->hasName() ? F->getName() : ""), CSs,
            std::string(F->getParent()->getName()), IsDead, F->isDeclaration(),
            F->getMetadata(IPOUtils::getSuppressInlineReportStringRef()),
            std::string(getLinkageStr(F)), std::string(getLanguageStr(F))) {}

  static bool isFunctionInliningReportMetadata(const Metadata *R);
};

// Class representing inlining report for call site
class CallSiteInliningReport : public InliningReport {
  MDTuple *
  initCallSite(LLVMContext *C, std::string Name, std::vector<MDTuple *> *CSs,
               InlineReason Reason = NinlrNoReason, bool IsInlined = false,
               bool IsSuppressPrint = false, int InlineCost = -1,
               int OuterInlineCost = -1, int InlineThreshold = -1,
               int EarlyExitInlineCost = INT_MAX,
               int EarlyExitInlineThreshold = INT_MAX, unsigned Line = 0,
               unsigned Col = 0, std::string ModuleName = "");

public:
  CallSiteInliningReport(MDTuple *R = nullptr, bool SuppressPrint = false)
      : InliningReport(R, SuppressPrint) {
    assert((!R || isCallSiteInliningReportMetadata(R)) &&
           "Bad function inlining report metadata");
  }

  CallSiteInliningReport(
      LLVMContext *C, std::string Name, std::vector<MDTuple *> *CSs,
      InlineReason Reason = NinlrNoReason, bool IsInlined = false,
      bool IsSuppressPrint = false, int InlineCost = -1,
      int OuterInlineCost = -1, int InlineThreshold = -1,
      int EarlyExitInlineCost = INT_MAX, int EarlyExitInlineThreshold = INT_MAX,
      unsigned Line = 0, unsigned Col = 0, std::string ModuleName = "");

  CallSiteInliningReport(CallBase *MainCS, std::vector<MDTuple *> *CSs,
                         InlineReason Reason = NinlrNoReason,
                         bool IsInlined = false, bool IsSuppressPrint = false,
                         int InlineCost = -1, int OuterInlineCost = -1,
                         int InlineThreshold = -1,
                         int EarlyExitInlineCost = INT_MAX,
                         int EarlyExitInlineThreshold = INT_MAX);

  static bool isCallSiteInliningReportMetadata(const Metadata *R);

  // Extracts line and column numbers from metadata node.
  bool getLineAndCol(unsigned *Line, unsigned *Col) {
    assert(Line && Col && "empty line or column args");
    if (Report->getNumOperands() < MDInliningReport::CallSiteMDSize)
      return false;
    Metadata *M = Report->getOperand(MDInliningReport::CSMDIR_LineAndColumn);
    StringRef LineAndColStr = cast<MDString>(M)->getString();
    // MDString is expected to be in the form of 'line: X col: Y'
    SmallVector<StringRef, 4> Elems;
    LineAndColStr.split(Elems, ' ');
    if (Elems.size() == 4) {
      Elems[1].getAsInteger(10, *Line);
      Elems[3].getAsInteger(10, *Col);
      return true;
    }
    return false;
  }

  // Return module name of the call site inline report
  StringRef getModuleName() {
    assert((Report->getNumOperands() == MDInliningReport::CallSiteMDSize) &&
           "bad metadata for callsite inline report");
    return llvm::getOpStr(
        Report->getOperand(MDInliningReport::CSMDIR_ModuleName),
        "moduleName: ");
  }
};

// Set of functions which set not-inlined reason to call site
void setMDReasonNotInlined(CallBase *Call, InlineReason Reason);
void setMDReasonNotInlined(CallBase *Call, const InlineCost &IC);
void setMDReasonNotInlined(CallBase *Call, const InlineCost &IC,
                           int TotalSecondaryCost);
// Set of functions which set inlined reason to call site
void setMDReasonIsInlined(CallBase *Call, InlineReason Reason);
void setMDReasonIsInlined(CallBase *Call, const InlineCost &IC);

/// Get the single, active metadata-based inlining report.
InlineReportBuilder *getMDInlineReport();

} // namespace llvm
#endif // LLVM_TRANSFORMS_IPO_INTEL_MDINLINEREPORT_H
