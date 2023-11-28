//===--- Intel_MDInlineReport.h ----------------------------------*- C++
//-*-===//
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

#define DEBUG_TYPE "mdinlinereport"

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
  CSMDIR_IsCostBenefit,
  CSMDIR_CBPairCost,
  CSMDIR_CBPairBenefit,
  CSMDIR_ICSMethod,
  CSMDIR_IsCompact,
  CSMDIR_BrokerTargetName,
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
  FMDIR_IsCompact,
  FMDIR_CompactIndexes,
  FMDIR_CompactCounts,
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

  // Maps Function to function index into temporary data structures:
  // Inlines, TotalInlines, and InlineCount. These are used to speed up
  // the translation process. They do not survive after the InlineReportBuilder
  // is destroyed. Before then, all relevant info must be written to metadata.
  MapVector<Function *, unsigned> FunctionIndexMap;
  // Similar to FunctionIndexMap, but uses the Function name which is still
  // valid after the Function is deleted.
  std::map<std::string, unsigned> FunctionNameIndexMap;
  // Maps function index to map of function index to inline counts for
  // summarized inlines
  MapVector<unsigned, MapVector<unsigned, unsigned> *> Inlines;
  // Maps function index to map of function index to inline counts for
  // total inlines in a callee
  MapVector<unsigned, MapVector<unsigned, unsigned> *> TotalInlines;
  // Maps function index to number of inlines in callee. Used to determine
  // when compacting threshold is crossed.
  MapVector<unsigned, unsigned> InlineCount;

public:
  explicit InlineReportBuilder(unsigned MyLevel)
      : Level(MyLevel), InitializedFromModuleTable(false),
        CurrentCallInstr(nullptr), CurrentCallInstReport(nullptr),
        CurrentCaller(nullptr), CurrentCallee(nullptr), IRBModule(nullptr) {}

  virtual ~InlineReportBuilder(void) {
    for (auto &IRCBEntry : IRCallbackMap)
      delete IRCBEntry.second;
    IRCallbackMap.clear();
    deleteAllFunctionTemps();
  }

  // Ensure that the temporary data structures are initialized to
  // correspond with data read in from an IR file. This happens most
  // commonly with LIT tests.
  void ensureModuleTableIsInitialized(Module *M) {
    if (!InitializedFromModuleTable) {
      NamedMDNode *MIR =
          M->getOrInsertNamedMetadata(MDInliningReport::ModuleTag);
      for (unsigned I = 0, E = MIR->getNumOperands(); I < E; ++I) {
        auto FR = cast<MDTuple>(MIR->getOperand(I));
        auto FF = MDInliningReport::FMDIR_FuncName;
        std::string Name = std::string(getOpStr(FR->getOperand(FF), "name: "));
        if (Function *LF = M->getFunction(Name))
          initFunctionTempsAtIndex(LF, I);
      }
      InitializedFromModuleTable = true;
    }
  }

  // Init the temporary data structures for the Function at the given index.
  void initFunctionTempsAtIndex(Function *F, unsigned Index) {
    if (!IRBModule)
      IRBModule = F->getParent();
    FunctionIndexMap.insert({F, Index});
    FunctionNameIndexMap.insert({std::string(F->getName()), Index});
    Inlines[Index] = new MapVector<unsigned, unsigned>;
    TotalInlines[Index] = new MapVector<unsigned, unsigned>;
    InlineCount[Index] = 0;
  }

  // Init the temporary data structures for the Function. Use the next
  // available index.
  void initFunctionTemps(Function *F, Module *M = nullptr) {
    if (!M)
      M = F->getParent();
    NamedMDNode *ModuleInlineReport =
        M->getOrInsertNamedMetadata(MDInliningReport::ModuleTag);
    unsigned Index = ModuleInlineReport->getNumOperands();
    initFunctionTempsAtIndex(F, Index);
  }

  // Delete temporary data structures.
  void deleteAllFunctionTemps() {
    FunctionIndexMap.clear();
    FunctionNameIndexMap.clear();
    for (auto &MV : Inlines)
      delete MV.second;
    Inlines.clear();
    for (auto &MV : TotalInlines)
      delete MV.second;
    TotalInlines.clear();
    InlineCount.clear();
  }

  // Search the function index for a function with name 'FunctionName'.
  // Normally it will be already in the FunctionIndexMap, but if we are running
  // the intermediate phases without first running inline report setup, the
  // function index will need to be generated on the fly, at which time its
  // temporary data structures will need to be initialized.
  unsigned searchForFunctionName(Module *M, StringRef FunctionName) {
    NamedMDNode *ModuleInlineReport =
        M->getOrInsertNamedMetadata(MDInliningReport::ModuleTag);
    for (unsigned I = 0, E = ModuleInlineReport->getNumOperands(); I < E; ++I) {
      auto FuncReport = cast<MDTuple>(ModuleInlineReport->getOperand(I));
      auto FN = MDInliningReport::FMDIR_FuncName;
      StringRef MDSR = getOpStr(FuncReport->getOperand(FN), "name: ");
      if (FunctionName == MDSR) {
        if (Function *F = M->getFunction(MDSR))
          initFunctionTempsAtIndex(F, I);
        return I;
      }
    }
    assert(false && "Expecting to find function in module metadata");
    // So that there is a return value on all paths.
    return ModuleInlineReport->getNumOperands();
  }

  // The function metdata may be present, but may not have been
  // inserted into the metadata inlining report table because the
  // specific optimization that created the function was not
  // explicitly updated by calling the metadata inlining report
  // update functions. If so, put it into the table and allocate
  // temps for it.
  unsigned fixRogueFunctionAndReturnIndex(Function *F, MDTuple *MDN) {
    Module *M = F->getParent();
    NamedMDNode *ModuleInlineReport =
        M->getOrInsertNamedMetadata(MDInliningReport::ModuleTag);
    unsigned Index = ModuleInlineReport->getNumOperands();
    initFunctionTemps(F);
    ModuleInlineReport->addOperand(MDN);
    return Index;
  }

  // Get the function index for 'F'.
  unsigned getFunctionIndex(Function *F) {
    ensureModuleTableIsInitialized(F->getParent());
    auto MapIt = FunctionIndexMap.find(F);
    if (MapIt != FunctionIndexMap.end())
      return MapIt->second;
    MDNode *MDN = F->getMetadata(MDInliningReport::FunctionTag);
    if (auto MDT = dyn_cast_or_null<MDTuple>(MDN))
      return fixRogueFunctionAndReturnIndex(F, MDT);
    return searchForFunctionName(F->getParent(), F->getName());
  }

  // Get the function index for a Function with name 'FunctionName'.
  // This must be used if the Function may have already been deleted.
  unsigned getFunctionIndexByName(Module *M, StringRef FunctionName) {
    ensureModuleTableIsInitialized(M);
    auto MapIt = FunctionNameIndexMap.find(std::string(FunctionName));
    if (MapIt != FunctionNameIndexMap.end())
      return MapIt->second;
    if (Function *F = M->getFunction(FunctionName)) {
      MDNode *MDN = F->getMetadata(MDInliningReport::FunctionTag);
      if (auto MDT = dyn_cast_or_null<MDTuple>(MDN))
        return fixRogueFunctionAndReturnIndex(F, MDT);
    }
    return searchForFunctionName(M, FunctionName);
  }

  // Return 'true' if the summarized form of 'F' should be used.
  bool getIsSummarized(Function *F) {
    return getIsCompact(F) && TotalInlines[getFunctionIndex(F)]->size();
  }

  // Return 'true' if both the caller and callee have function metadata.
  bool hasFunctionMetadata(Function *Caller, Function *Callee);

  bool getIsCompact(Function *F);
  void setIsCompact(Function *F, bool Value);

  bool getIsCompact(Metadata *CurrentCallInstReport);
  void setIsCompact(Metadata *CurrentCallInstReport, bool Value);

  void setBrokerTarget(CallBase *CB, Function *F);

  // Update the name of 'F' in the inlining report.
  void updateName(Function *F);

  // Return 'true' if the inlining of 'Callee' into 'Caller' should be done
  // after compacting the representation of 'Callee'. If 'ForceCompact',
  // ensure that the compacted form is always used.
  bool shouldCompactCallBase(Function *Caller, Function *Callee,
                             bool ForceCompact);

  // Indicate that the Function with 'CalleeIndex' was inlined into the
  // Function with 'CallerIndex' 'Count' times.
  void addCompactInlinedCallBase(unsigned CallerIndex, unsigned CalleeIndex,
                             unsigned Count = 1);

  // Indicate that the Function with 'CalleeIndex' was inlined into the
  // Function with 'CallerIndex' 'Count' times for a specific callsite.
  void addForCompactInlinedCallBase(unsigned CallerIndex,
                                    unsigned CalleeIndex,
                                    unsigned Count = 1);

  // Compact the children in the callsite whose metdata is 'MDTupleCS'
  // into the 'Caller'.
  void compactChildren(Function *Caller, MDTuple *MDTupleCS);

  // Compact the inlining report for 'F'.
  void compact(Function *F);

  // Add the compacted inlining report information from 'Callee' into 'Caller'.
  void inheritCompactCallBases(Function *Caller, Function *Callee);

  // Walk over inlining reports for call instructions in current function to add
  // them to the callback vector.
  void beginFunction(Function *F);

  // Walk over inlining reports for functions in current SCC to add them to the
  // callback vector.
  void beginSCC(LazyCallGraph::SCC &SCC);

  // Mark function as dead in its inlining report.
  void setDead(Function *F);

  bool isMDIREnabled() { return Level & BasedOnMetadata; }

  // Create a clone of function inlining report.
  Metadata *cloneInliningReport(Function *F, ValueToValueMapTy &VMap);

  // Cloning routine for callsites of function in compact form.
  Metadata *cloneCompactCS(LLVMContext &C, ValueToValueMapTy &VMap);

  // Helper traversal function for cloneInliningReportHelperCompact()
  Metadata *cloneInliningReportHelperCompact(LLVMContext &C, Metadata *OldMD,
                                             ValueToValueMapTy &VMap,
                                             bool IsTerminal);

  // Create a compact clone of function inlining report.
  Metadata *cloneInliningReportCompact(Function *Caller, Function *Callee,
                                       ValueToValueMapTy &VMap);
  // Update inlining report for the inlined call site.
  void inlineCallSite();

  // Add a pair of old and new call sites.  The 'NewCall' is a clone of
  // the 'OldCall' produced by InlineFunction().
  void addActiveCallSitePair(CallBase *OldCall, CallBase *NewCall) {
    // If there were no metadata on the original instruction, we have nothing
    // to assign to the new instruction. Skip them.
    if (!OldCall->getMetadata(MDInliningReport::CallSiteTag))
      return;
    if (!NewCall || shouldSkipCallBase(NewCall))
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

  void dumpFunctionNameIndexMap(Function *F) {
    static unsigned ICount = 0;
    dbgs() << "BEGIN DUMPING FunctionNameIndexMap: " << ICount++ << "\n";
    dbgs() << "COMPACTING: " << F->getName() << "\n";
    for (const auto &Pair : FunctionNameIndexMap)
      dbgs() << "  " << Pair.second << " " << Pair.first << "\n";
    dbgs() << "END DUMPING FunctionNameIndexMap\n";
  }

  // Indicate that 'CB' has been eliminated as dead code with the
  // indicated reason.
  void removeCallBaseReference(CallBase &CB, InlineReason Reason = NinlrDeleted,
                               bool FromCallback = false) {
    LLVM_DEBUG(dbgs() << "removeCallBaseReference: " << &CB << " ");
    if (!FromCallback)
      LLVM_DEBUG(dbgs() << CB.getCaller()->getName() << " TO "
                        << CB.getCalledFunction()->getName());
    LLVM_DEBUG(dbgs() << "\n");
    if (!FromCallback && shouldSkipCallBase(&CB))
      return;
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
    if (!FromCallback)
      removeCallback(&CB);
  }

  // Indicate that 'F' has been eliminated as a dead static function.
  void removeFunctionReference(Function &F, bool FromCallback = false) {
    LLVM_DEBUG(dbgs() << "removeFunctionReference: " << &F << " ");
    if (!FromCallback)
      LLVM_DEBUG(dbgs() << F.getName());
    LLVM_DEBUG(dbgs() << "\n");
    MDNode *MDIR = F.getMetadata(MDInliningReport::FunctionTag);
    if (!MDIR) {
      auto MapIt = FunctionIndexMap.find(&F);
      if (MapIt == FunctionIndexMap.end())
        return;
      NamedMDNode *ModuleInlineReport =
          IRBModule->getOrInsertNamedMetadata(MDInliningReport::ModuleTag);
      unsigned Index = MapIt->second;
      MDIR = ModuleInlineReport->getOperand(Index);
    }
    if (auto *FIR = dyn_cast<MDTuple>(MDIR)) {
      LLVMContext &Ctx = MDIR->getContext();
      std::string IsDeadStr = "isDead: ";
      IsDeadStr.append(std::to_string(true));
      auto IsDeadMD = MDNode::get(Ctx, llvm::MDString::get(Ctx, IsDeadStr));
      FIR->replaceOperandWith(MDInliningReport::FMDIR_IsDead, IsDeadMD);
    }
    FunctionIndexMap.erase(&F);
    if (!FromCallback)
      removeCallback(&F);
  }

  // Setup initial values for the single inlining step
  void beginUpdate(CallBase *Call) {
    CurrentCaller = Call->getCaller();
    CurrentCallee = Call->getCalledFunction();
    CurrentCallInstReport = Call->getMetadata(MDInliningReport::CallSiteTag);
    CurrentCallInstr = Call;
    ActiveOriginalCalls.clear();
    ActiveInlinedCalls.clear();
  }

  void endUpdate() {
    CurrentCaller = nullptr;
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

  // Indicate 'CBDirect' has been specialized as an direct call for the
  // indirect call 'CBIndirect'.
  void addIndirectCallBaseTarget(InlICSType ICSMethod, CallBase *CBIndirect,
                                 CallBase *CBDirect);

  // Replace 'OldFunction' with 'NewFunction'.
  void replaceFunctionWithFunction(Function *OldFunction,
                                   Function *NewFunction);

  // Replace 'OldFunction' with 'NewFunction'.
  void cloneFunction(Function *OldFunction, Function *NewFunction,
                     ValueToValueMapTy &VMap);

  // Outline 'OutF' from 'OldF', calling it with 'OutCB'.
  void doOutlining(Function *OldF, Function *OutF, CallBase *OutCB);

  // Replace uses of 'OldFunction' with 'NewFunction'.
  void replaceAllUsesWith(Function *OldFunction, Function *NewFunction);

  /// Replace all uses of 'OldFunction' with 'NewFunction'
  /// where 'ShouldReplace' is true in the inlining report.
  void replaceUsesWithIf(Function *OldFunction, Function *NewFunction,
                         llvm::function_ref<bool(Use &U)> ShouldReplace);

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

  // Return 'true' if there should not be inline report metadata for 'CB'.
  bool shouldSkipCallBase(CallBase *CB);

  // Add a call site corresponding to 'CB'.
  void addCallSite(CallBase *CB);

  // Add a function corresponding to 'F'.
  void addFunction(Function *F);

private:
  /// The Level is specified by the option -inline-report=N.
  /// See llvm/lib/Transforms/IPO/Inliner.cpp for details on Level.
  unsigned Level;
  /// Is 'true' if the module metadata table has been queried to determine if
  /// reading the IR has loaded entries into the module metadata table.
  bool InitializedFromModuleTable;
  // Call instruction which is considered on the current inlining step.
  Instruction *CurrentCallInstr;
  // Metadata inlining report for the current call instruction.
  Metadata *CurrentCallInstReport;
  Function *CurrentCaller;
  // Function called in the current call instruction.
  Function *CurrentCallee;
  /// The Module for the inlining report.
  Module *IRBModule;
  // Copy metadata 'OldMD' for use by another Function or CallBase.
  Metadata *copyMD(LLVMContext &C, Metadata *OldMD);
  // Copy metadata 'OldMD' for use by another Function or CallBase.
  // Save a map from the original to the copied metadata.
  Metadata *copyMDWithMap(LLVMContext &C, Metadata *OldMD,
                          DenseMap<Metadata *, Metadata *> &MDMap);

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
        InlineReason Reason = NinlrDeleted;
        IRB->removeCallBaseReference(*CB, Reason, true);
      } else if (auto F = dyn_cast<Function>(getValPtr())) {
        /// Indicate in the inline report that the function
        /// corresponding to the Value has been deleted
        IRB->removeFunctionReference(*F, true);
      }
      CallbackVH::deleted();
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
    LLVM_DEBUG(dbgs() << "addCallback: " << V << "\n");
    InliningReportCallback *IRCB = new InliningReportCallback(V, this);
    IRCallbackMap.insert({V, IRCB});
  }

  // Remove callback from the map
  void removeCallback(Value *V) {
    if (!V || !IRCallbackMap.count(V))
      return;
    LLVM_DEBUG(dbgs() << "removeCallback: " << V << "\n");
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
                         bool IsCompact, std::string LinkageChar,
                         std::string LanguageChar);

  FunctionInliningReport(Function *F, std::vector<MDTuple *> *CSs, bool IsDead,
                         bool IsCompact)
      : FunctionInliningReport(
            &(F->getParent()->getContext()),
            std::string(F->hasName() ? F->getName() : ""), CSs,
            std::string(F->getParent()->getName()), IsDead, F->isDeclaration(),
            F->getMetadata(IPOUtils::getSuppressInlineReportStringRef()),
            IsCompact, std::string(getLinkageStr(F)),
            std::string(getLanguageStr(F))) {}

  static bool isFunctionInliningReportMetadata(const Metadata *R);
};

// Class representing inlining report for call site
class CallSiteInliningReport : public InliningReport {
  MDTuple *initCallSite(
      LLVMContext *C, std::string Name, std::vector<MDTuple *> *CSs,
      InlineReason Reason = NinlrNoReason, bool IsInlined = false,
      bool IsSuppressPrint = false, int InlineCost = -1,
      int OuterInlineCost = -1, int InlineThreshold = -1,
      int EarlyExitInlineCost = INT_MAX, int EarlyExitInlineThreshold = INT_MAX,
      bool IsCostBenefit = false, int CBPairCost = -1, int CBPairBenefit = -1,
      InlICSType ICSMethod = InlICSNone, bool IsCompact = false,
      unsigned Line = 0, unsigned Col = 0, std::string ModuleName = "");

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
      bool IsCostBenefit = false, int CBPairCost = -1, int CBPairBenefit = -1,
      InlICSType ICSMethod = InlICSNone, bool IsCompact = false,
      unsigned Line = 0, unsigned Col = 0, std::string ModuleName = "");

  CallSiteInliningReport(CallBase *MainCS, std::vector<MDTuple *> *CSs,
                         InlineReason Reason = NinlrNoReason,
                         bool IsInlined = false, bool IsSuppressPrint = false,
                         int InlineCost = -1, int OuterInlineCost = -1,
                         int InlineThreshold = -1,
                         int EarlyExitInlineCost = INT_MAX,
                         int EarlyExitInlineThreshold = INT_MAX,
                         bool IsCostBenefit = false, int CBPairCost = -1,
                         int CBPairBenefit = -1,
                         InlICSType ICSMethod = InlICSNone,
                         bool IsCompact = false);

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

  // Set the inlining reason based on the type of the callee of 'CB'.
  void initReason(CallBase *CB);
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

#undef DEBUG_TYPE
#endif // LLVM_TRANSFORMS_IPO_INTEL_MDINLINEREPORT_H
