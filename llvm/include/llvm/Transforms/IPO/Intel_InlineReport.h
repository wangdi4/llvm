//===- Intel_InlineReport.h - Implement inlining report ---------*- C++ -*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines various classes needed to represent an inlining report.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_INLINEREPORT_H
#define LLVM_TRANSFORMS_IPO_INTEL_INLINEREPORT_H

#include "llvm/ADT/MapVector.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/LazyCallGraph.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/Transforms/IPO/Intel_InlineReportCommon.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <climits>

#define DEBUG_TYPE "inlinereport"

namespace llvm {

using namespace InlineReportTypes;

class InlineReportCallSite;

typedef std::vector<InlineReportCallSite *> InlineReportCallSiteVector;

class InlineReportFunction;

///
/// Represents a CallSite in the inlining report
///
class InlineReportCallSite {
public:
  // Constructor for InlineReportCallSite
  // The source file is given by 'M'.  The line and column info by 'Dloc'
  explicit InlineReportCallSite(InlineReportFunction *IRCallee, bool IsInlined,
                                InlineReason Reason, Module *Module,
                                DebugLoc *DLoc, CallBase *CB,
                                bool SuppressPrint = false)
      : IRCallee(IRCallee), IRCaller(nullptr), IRParent(nullptr),
        IRBrokerTarget(nullptr), IsInlined(IsInlined), IsCompact(false),
        Reason(Reason), InlineCost(-1), OuterInlineCost(-1),
        InlineThreshold(-1), EarlyExitInlineCost(INT_MAX),
        EarlyExitInlineThreshold(INT_MAX), CostBenefit(std::nullopt),
        ICSMethod(InlICSNone), Call(CB), M(Module),
        SuppressPrint(SuppressPrint) {
    Line = DLoc && DLoc->get() ? DLoc->getLine() : 0;
    Col = DLoc && DLoc->get() ? DLoc->getCol() : 0;
    Children.clear();
  };

  ~InlineReportCallSite(void);
  InlineReportCallSite(const InlineReportCallSite &) = delete;
  void operator=(const InlineReportCallSite &) = delete;

  // Return a pointer to a copy of Base with an empty Children vector
  // Use 'NewIRCallee' for the IRCallee if specified.
  InlineReportCallSite *copyBase(CallBase *CB,
                                 InlineReportFunction *NewIRCallee = nullptr);

  InlineReportFunction *getIRCallee() const { return IRCallee; }
  void setIRCallee(InlineReportFunction *IRF) { IRCallee = IRF; }

  InlineReportFunction *getIRCaller() const { return IRCaller; }
  void setIRCaller(InlineReportFunction *IRF) { IRCaller = IRF; }

  InlineReportCallSite *getIRParent() const { return IRParent; }
  void setIRParent(InlineReportCallSite *IRCS) { IRParent = IRCS; }

  InlineReportFunction *getIRBrokerTarget() const { return IRBrokerTarget; }
  void setIRBrokerTarget(InlineReportFunction *IRF) { IRBrokerTarget = IRF; }
  void printBrokerTargetName(raw_ostream &OS, unsigned ReportLevel);

  InlineReason getReason() const { return Reason; }
  void setReason(InlineReason MyReason) { Reason = MyReason; }
  bool getIsInlined() const { return IsInlined; }
  void setIsInlined(bool Inlined) { IsInlined = Inlined; }

  // Indicates that inlining for this call site will be summarized in
  // compact form.
  bool getIsCompact(void) const { return IsCompact; }
  void setIsCompact(bool V) { IsCompact = V; }

  /// Return true if in the original inlining process there would be
  /// early exit due to high cost.
  bool isEarlyExit() const { return EarlyExitInlineCost != INT_MAX; }

  /// Return the vector of InlineReportCallSites which represent
  /// the calls made from the section of inlined code represented by
  /// this InlineReportCallSite.
  InlineReportCallSiteVector &getChildren() { return Children; }

  /// Inlining is inhibited if the inline cost is greater than
  /// the threshold.
  int getInlineCost() const { return InlineCost; }
  void setInlineCost(int Cost) { InlineCost = Cost; }

  /// Results from inlining cost/benefit analysis.
  std::optional<CostBenefitPair> getCostBenefit() const { return CostBenefit; }
  void setCostBenefit(const std::optional<CostBenefitPair> &TheCostBenefit) {
    CostBenefit = TheCostBenefit;
  }

  /// Stored "early exit" cost of inlining.
  int getEarlyExitInlineCost() const { return EarlyExitInlineCost; }
  void setEarlyExitInlineCost(int EECost) { EarlyExitInlineCost = EECost; }

  /// Get and set the outer inlining cost.
  /// Since inlining is bottom up, always selecting the leaf-most
  /// call sites for inlining is not always best, as it may inhibit inlining
  /// further up the call tree.  Therefore, in addition to an inlining cost,
  /// the inliner computes an outer inlining cost as well.  Inlining is also
  /// inhibited if the outer inlining cost is greater than the inline
  /// threshold.
  int getOuterInlineCost() const { return OuterInlineCost; }
  void setOuterInlineCost(int Cost) { OuterInlineCost = Cost; }

  int getInlineThreshold() const { return InlineThreshold; }
  void setInlineThreshold(int Threshold) { InlineThreshold = Threshold; }

  bool getSuppressPrint(void) const { return SuppressPrint; }
  void setSuppressPrint(bool V) { SuppressPrint = V; }

  InlICSType getICSMethod(void) const { return ICSMethod; }
  void setICSMethod(InlICSType V) { ICSMethod = V; }

  /// Stored "early exit" threshold of inlining.
  int getEarlyExitInlineThreshold() const { return EarlyExitInlineThreshold; }
  void setEarlyExitInlineThreshold(int EEThreshold) {
    EarlyExitInlineThreshold = EEThreshold;
  }
  CallBase *getCall() const { return Call; }
  void setCall(CallBase *Call) { this->Call = Call; }
  void addChild(InlineReportCallSite *IRCS) {
    IRCS->setIRCaller(IRCaller);
    IRCS->setIRParent(this);
    Children.push_back(IRCS);
  }

  /// Print the info in the inlining instance to 'OS' for the inlining report
  /// indenting 'indentCount' indentations, assuming an inlining report
  /// level of 'ReportLevel'.
  void print(raw_ostream &OS, unsigned IndentCount, unsigned ReportLevel);

  /// Load the call represented by '*this' and all of its descendant
  /// calls into the map 'Lmap'.
  void loadCallsToMap(std::map<CallBase *, bool> &LMap);

  /// Clone the callsites recursively in and under 'IRCSV' using 'VMap'
  /// and place the clones under 'NewIRCS'.
  void cloneCallSites(InlineReportCallSiteVector &IRCSV,
                      ValueToValueMapTy &VMap, InlineReportCallSite *NewIRCS);

  /// Move the callsites of 'OldIRF' to 'NewIRF'.
  void move(InlineReportFunction *OldIRF, InlineReportFunction *NewIRF);

  /// Move the callsites of 'OldIRCS' to 'NewIRCS'.
  void move(InlineReportCallSite *OldIRCS, InlineReportCallSite *NewIRCS);

  /// Move the callsite recursively in and under 'IRCSV' and attach
  /// them to 'NewIRCS' if they appear in 'OutFCBSet'.
  void moveOutlinedChildren(InlineReportCallSiteVector &IRCSV,
                            SmallPtrSetImpl<InlineReportCallSite *> &OutFCBSet,
                            InlineReportCallSite *NewIRCS);

  // Initialize the inlining reason based on characteristics of 'Callee'.
  void initReason(Function *Callee);

  unsigned getLine() { return Line; }
  void setLine(unsigned V) { Line = V; }
  unsigned getCol() { return Col; }
  void setCol(unsigned V) { Col = V; }

private:
  InlineReportFunction *IRCallee;
  InlineReportFunction *IRCaller;
  InlineReportCallSite *IRParent;
  InlineReportFunction *IRBrokerTarget;
  bool IsInlined;
  bool IsCompact;
  InlineReason Reason;
  int InlineCost;
  int OuterInlineCost;
  int InlineThreshold;
  int EarlyExitInlineCost;
  int EarlyExitInlineThreshold;
  std::optional<CostBenefitPair> CostBenefit;
  InlICSType ICSMethod;
  InlineReportCallSiteVector Children;
  CallBase *Call;
  ///
  /// Used to get the file name when we print the report
  Module *M;
  ///
  /// The line and column number of the call site.  These are 0 if
  /// we are not compiling with -g or the lighter weight version
  /// -gline-tables-only
  unsigned Line;
  unsigned Col;
  bool SuppressPrint; // suppress inline-report print info
  void printICSMethod(raw_ostream &OS, unsigned Level);
  void printCostAndBenefit(raw_ostream &OS, unsigned Level);
  void printCostAndThreshold(raw_ostream &OS, unsigned Level);
  void printOuterCostAndThreshold(raw_ostream &OS, unsigned Level);
  void printCalleeNameModuleLineCol(raw_ostream &OS, unsigned Level);

  /// Search 'OldIRCSV' for 'this' and if it is found, move it to 'NewIRCSV'.
  void moveCalls(InlineReportCallSiteVector &OldIRCSV,
                 InlineReportCallSiteVector &NewIRCSV);
};

///
/// Represents a routine (compiled or dead) in the inlining report
///
class InlineReportFunction {
public:
  explicit InlineReportFunction(const Function *F, bool SuppressPrint = false)
      : IsDead(false), IsCurrent(false), IsDeclaration(false), LinkageChar(' '),
        LanguageChar(' '), SuppressPrint(SuppressPrint), IsCompact(false),
        InlineCount(0){};
  ~InlineReportFunction(void);
  InlineReportFunction(const InlineReportFunction &) = delete;
  void operator=(const InlineReportFunction &) = delete;

  /// A vector of InlineReportCallSites representing the top-level
  /// call sites in a function (i.e. those which appear in the source code
  /// of the function).
  InlineReportCallSiteVector &getCallSites() { return CallSites; }

  /// Add an InlineReportCallSite to the list of top-level calls for
  /// this function.
  void addCallSite(InlineReportCallSite *IRCS) {
    IRCS->setIRCaller(this);
    IRCS->setIRParent(nullptr);
    CallSites.push_back(IRCS);
  }

  /// Return true if the function has been dead code eliminated.
  bool getDead() const { return IsDead; }

  /// Set whether the function is dead code eliminated.
  void setDead(bool Dead) { IsDead = Dead; }

  /// Return true if the inline report for this routine reflects
  /// the changes that have been made to the routine since the last call
  /// to Inliner::runOnSCC()
  bool getCurrent(void) const { return IsCurrent; }

  /// Set whether the inline report for the routine is current
  void setCurrent(bool Current) { IsCurrent = Current; }

  bool getIsDeclaration(void) const { return IsDeclaration; }

  void setIsDeclaration(bool Declaration) { IsDeclaration = Declaration; }

  /// Get a single character indicating the linkage type
  char getLinkageChar(void) { return LinkageChar; }

  /// Get a single character indicating the language
  char getLanguageChar(void) { return LanguageChar; }

  /// Get and set SuppressPrint
  bool getSuppressPrint(void) const { return SuppressPrint; }
  void setSuppressPrint(bool V) { SuppressPrint = V; }

  /// The number of times inlining has been applied to this function.
  /// If the number is high enough, inlining of this function will be
  /// reported in compact form.
  unsigned getInlineCount(void) const { return InlineCount; }
  void setInlineCount(unsigned V) { InlineCount = V; }

  /// Indicates that all information needed to report inlining of this
  /// function in compact form has been collected.
  bool getIsCompact(void) const { return IsCompact; }
  void setIsCompact(bool V) { IsCompact = V; }

  /// Returns 'true' if inlining of this function will be reported in
  /// compact form.
  bool getIsSummarized() { return IsCompact && TotalInlines.size(); }

  /// Set a single character indicating the linkage type
  /// L: Local
  /// O: One definition rule (ODR)
  /// X: External
  /// A: Other
  void setLinkageChar(Function *F) {
    LinkageChar =
        (F->hasLocalLinkage()
             ? 'L'
             : (F->hasLinkOnceODRLinkage()
                    ? 'O'
                    : (F->hasAvailableExternallyLinkage() ? 'X' : 'A')));
  }

  /// Set a single character indicating the language type
  /// F: Fortran
  /// C: C/C++
  void setLanguageChar(Function *F) {
    LanguageChar = F->isFortran() ? 'F' : 'C';
  }

  std::string &getName() { return Name; }

  void setName(std::string FunctionName) { Name = FunctionName; }

  void printName(raw_ostream &OS, unsigned Level) {
    if ((Level & InlineReportOptions::Demangle) && getLanguageChar() == 'C')
      OS << demangle(getName());
    else
      OS << getName();
  }

  void print(raw_ostream &OS, unsigned Level) const;

  /// Populate 'OutFIRCSSet' with the InlineReportCallSites corresponding
  /// to the CallBases in 'OutFCBSet'
  void findOutlinedIRCSes(SmallPtrSetImpl<CallBase *> &OutFCBSet,
                          SmallPtrSetImpl<InlineReportCallSite *> &OutFIRCSSet);

  /// Move the InlineReportCallSites 'OutFCBSet' under 'NewIRF'.
  void
  moveOutlinedCallSites(InlineReportFunction *NewIRF,
                        SmallPtrSetImpl<InlineReportCallSite *> &OutFCBSet);

  /// Return 'true' if 'IRFCallee' should be compacted when inlined into
  /// this InlineReportFunction. If 'ForceCompact', always return 'true'.
  bool shouldCompactCallBase(InlineReportFunction *IRFCallee,
                             bool ForceCompact);

  /// Indicate 'IRFCallee' was inlined into this InlineReportFunction
  /// 'Count' times in compact form.
  void addCompactInlinedCallBase(InlineReportFunction *IRFCallee,
                                 unsigned Count = 1);

  /// Indicate 'IRFCallee' was inlined into this InlineReportFunction
  /// 'Count' times in compact form for a specific callsite.
  void addForCompactInlinedCallBase(InlineReportFunction *IRFCallee,
                                    unsigned Count = 1);

  /// Add the compact inlined callsites from 'IRFCallee' to this
  /// InlineReportFunction.
  void inheritCompactCallBases(InlineReportFunction *IRFCallee);

  /// Compact this InlineReportFunction.
  void compact();

  /// Compact the children of 'IRCS' into this InlineReportFunction.
  void compactChildren(InlineReportCallSite *IRCS);

  /// Clone compact info from 'IRF' to the current InlineReportFunction.
  void cloneCompactInfo(InlineReportFunction *IRF);

private:
  bool IsDead;
  bool IsCurrent;
  bool IsDeclaration;
  char LinkageChar;
  char LanguageChar;
  std::string Name;
  bool SuppressPrint; // suppress inline-report print
  bool IsCompact;
  InlineReportCallSiteVector CallSites;
  unsigned InlineCount;
  // Records the inlines that will be summarized for the function
  MapVector<InlineReportFunction *, unsigned> Inlines;
  // Records the summarized inlines that will be inherited from the callee
  // to the caller when the inlining of a callsite is reported in compact
  // form.
  MapVector<InlineReportFunction *, unsigned> TotalInlines;
};

typedef MapVector<Function *, InlineReportFunction *> InlineReportFunctionMap;
struct IRFComparator {
  bool operator()(InlineReportFunction *IRF1,
                  InlineReportFunction *IRF2) const {
    return IRF1->getName() < IRF2->getName();
  }
};
typedef std::set<InlineReportFunction *, IRFComparator> InlineReportFunctionSet;
typedef std::map<CallBase *, InlineReportCallSite *>
    InlineReportCallBaseCallSiteMap;

///
/// The inlining report
///
class InlineReport {
public:
  explicit InlineReport(unsigned MyLevel)
      : Level(MyLevel), ActiveInlineCallBase(nullptr), ActiveCaller(nullptr),
        ActiveCallee(nullptr), ActiveIRCS(nullptr), M(nullptr),
        OS(OptReportOptions::getOutputStream()){};
  virtual ~InlineReport(void);

  // Indicate that we have begun inlining functions in the current
  // SCC of the CG.
  void beginSCC(LazyCallGraph::SCC &SCC, void *Inliner);

  // Indicate that we have begun inlining functions in the current Module.
  void beginModule(void *Inliner);

  // Indicate that we are done inlining functions in the current SCC.
  void endSCC();

  // Indicate that we are done inlining functions in the current Module.
  void endModule(void);

  void beginUpdate(CallBase *Call) {
    if (!isClassicIREnabled())
      return;
    ActiveCaller = Call->getCaller();
    ActiveCallee = Call->getCalledFunction();
    // New call sites can be added from inlining even if they are not a
    // cloned from the inlined callee.
    ActiveIRCS = getOrAddCallSite(Call);
    ActiveInlineCallBase = Call;
    ActiveOriginalCalls.clear();
    ActiveInlinedCalls.clear();
  }

  void endUpdate() {
    if (!isClassicIREnabled())
      return;
    ActiveCaller = nullptr;
    ActiveCallee = nullptr;
    ActiveIRCS = nullptr;
    ActiveInlineCallBase = nullptr;
    ActiveOriginalCalls.clear();
    ActiveInlinedCalls.clear();
  }

  /// Indicate that the current CallSite CS has been inlined in
  /// the inline report.  Use the InlineInfo collected during inlining
  /// to update the report.
  void inlineCallSite();

  // Indicate that the Function is dead
  void setDead(Function *F) {
    if (!isClassicIREnabled())
      return;
    auto MapIt = IRFunctionMap.find(F);
    assert(MapIt != IRFunctionMap.end());
    InlineReportFunction *INR = MapIt->second;
    INR->setDead(true);
  }

  /// Print the inlining report at the given level.
  void print() const;

  /// Test if 'Inliner' represents an active inliner, and if it is the last
  /// pending active inliner, print the inlining report. If 'Inliner' is
  /// nullptr, print unconditionally.
  void testAndPrint(void *Inliner);

  /// Check if report has data
  bool isEmpty() { return IRFunctionMap.empty(); }

  // The level of the inline report
  unsigned getLevel() { return Level; }

  // Check if classic inline report should be created
  bool isClassicIREnabled() const {
    return (Level && !(Level & BasedOnMetadata));
  }

  /// Record the reason a call site is or is not inlined.
  void setReasonNotInlined(CallBase *Call, InlineReason Reason);
  void setReasonNotInlined(CallBase *Call, const InlineCost &IC);
  void setReasonNotInlined(CallBase *Call, const InlineCost &IC,
                           int TotalSecondaryCost);
  void setReasonIsInlined(CallBase *Call, InlineReason Reason);
  void setReasonIsInlined(CallBase *Call, const InlineCost &IC);

  /// Replace 'OldFunction' with 'NewFunction' in the inlining report,
  /// so that 'NewFunction' inherits the properties of 'OldFunction'.
  void replaceFunctionWithFunction(Function *OldFunction,
                                   Function *NewFunction);

  /// Replace 'CB0' with 'CB1' in the inlining report, so that 'CB1'
  /// inherits the properties of 'CB0'. If 'UpdateReason', update
  /// the inlining reason based on the callee of 'CB1'.
  void replaceCallBaseWithCallBase(CallBase *CB0, CallBase *CB1,
                                   bool UpdateReason = false);

  /// Clone 'CB0' to produce 'CB1' in the inlining report, so that 'CB1'
  /// inherits the properties of 'CB0'.
  void cloneCallBaseToCallBase(CallBase *CB0, CallBase *CB1);

  /// Clone 'OldFunction' into 'New Function', using 'VMap' to get
  /// the mapping from old to new callsites.
  void cloneFunction(Function *OldFunction, Function *NewFunction,
                     ValueToValueMapTy &VMap);

  /// Replace all uses of 'OldFunction' with 'NewFunction' in the
  /// inlining report.
  void replaceAllUsesWith(Function *OldFunction, Function *NewFunction);

  /// Replace all uses of 'OldFunction' with 'NewFunction'
  /// where 'ShouldReplace' is true in the inlining report.
  void replaceUsesWithIf(Function *OldFunction, Function *NewFunction,
                         llvm::function_ref<bool(Use &U)> ShouldReplace);

  /// Ensure that 'F' and all Functions that call it directly are in the
  /// inlining report.
  void initFunctionClosure(Function *F);

  /// Ensure that all of the Functions in 'M' are in the inlining report.
  void initModule(Module *M);

  /// Record that outling of 'OldF' (original function) into 'OutF'
  /// (extracted or splinter function). 'OldF' calls 'OutF' via 'OutCB'.
  void doOutlining(Function *OldF, Function *OutF, CallBase *OutCB);

  /// Add a pair of old and new call sites.  The 'NewCall' is a clone of
  /// the 'OldCall' produced by InlineFunction().
  void addActiveCallSitePair(CallBase *OldCall, CallBase *NewCall) {
    if (!isClassicIREnabled() || !NewCall || shouldSkipCallBase(NewCall))
      return;
    ActiveOriginalCalls.push_back(OldCall);
    ActiveInlinedCalls.push_back(NewCall);
    addCallback(NewCall);
  }

  /// Update the 'OldCall' to 'NewCall' in the ActiveInlinedCalls.
  /// This needs to happen if we manually replace an active inlined
  /// call during InlineFunction().
  void updateActiveCallSiteTarget(Value *OldCall, Value *NewCall) {
    if (!isClassicIREnabled())
      return;
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
  void removeCallBaseReference(CallBase &CB, InlineReason Reason = NinlrDeleted,
                               bool FromCallback = false) {
    LLVM_DEBUG(dbgs() << "removeCallBaseReference: " << &CB << " ");
    if (!FromCallback)
      LLVM_DEBUG(dbgs() << CB.getCaller()->getName() << " TO "
                        << CB.getCalledFunction()->getName());
    LLVM_DEBUG(dbgs() << "\n");
    if (!isClassicIREnabled())
      return;
    if (!FromCallback && shouldSkipCallBase(&CB))
      return;
    if (ActiveInlineCallBase != &CB) {
      auto MapIt = IRCallBaseCallSiteMap.find(&CB);
      if (MapIt != IRCallBaseCallSiteMap.end() || !FromCallback) {
        InlineReportCallSite *IRCS = getOrAddCallSite(&CB);
        IRCallBaseCallSiteMap.erase(&CB);
        IRCS->setCall(nullptr);
        IRCS->setReason(Reason);
      }
    }
    // If necessary, remove any reference in the ActiveInlinedCalls
    for (unsigned II = 0, E = ActiveInlinedCalls.size(); II < E; ++II)
      if (ActiveInlinedCalls[II] == &CB)
        ActiveInlinedCalls[II] = nullptr;
    removeCallback(&CB);
  }

  // Indicate that 'F' has been eliminated as a dead static function.
  void removeFunctionReference(Function &F, bool FromCallback = false) {
    LLVM_DEBUG(dbgs() << "removeFunctionReference: " << &F << " ");
    if (!FromCallback)
      LLVM_DEBUG(dbgs() << F.getName());
    LLVM_DEBUG(dbgs() << "\n");
    if (!isClassicIREnabled())
      return;
    auto MapIt = IRFunctionMap.find(&F);
    if (MapIt != IRFunctionMap.end() || !FromCallback) {
      InlineReportFunction *IRF = getOrAddFunction(&F);
      setDead(&F);
      IRFunctionMap.erase(&F);
      IRDeadFunctionSet.insert(IRF);
    }
    removeCallback(&F);
  }

  // Indicate that 'F' has been replaced.
  void replaceFunctionReference(Function &F) {
    LLVM_DEBUG(dbgs() << "replaceFunctionReference: " << &F << " ");
    LLVM_DEBUG(dbgs() << F.getName());
    LLVM_DEBUG(dbgs() << "\n");
    if (!isClassicIREnabled())
      return;
    auto MapIt = IRFunctionMap.find(&F);
    if (MapIt != IRFunctionMap.end())
      IRFunctionMap.erase(&F);
    removeCallback(&F);
  }

  // Create or update the exiting representation of 'F'.
  InlineReportFunction *initFunction(Function *F);

  // Change the called Function of 'CB' to 'F'.
  void setCalledFunction(CallBase *CB, Function *F);

  // Delete all calls inside 'F'.
  void deleteFunctionBody(Function *F);

  // Indicate that 'CB' calls a specialized version of its caller under
  // a multiversioning test.
  void addMultiversionedCallSite(CallBase *CB);

  // Remove all of the CallBases in the 'BlocksToRemove' as dead code.
  void
  removeCallBasesInBasicBlocks(SmallSetVector<BasicBlock *, 8> &BlocksToRemove);

  // Return 'true' if there should not be an InlineReportCallSite for 'CB'
  // in the inline report.
  bool shouldSkipCallBase(CallBase *CB);

  /// Ensure that the inline report for this routine reflects the
  /// changes that have been made to that routine since the last call to
  /// Inliner::runOnSCC(). Return 'true' if 'F' is modified.
  bool makeCurrent(Function *F);

  /// Indicate 'CBDirect' has been specialized as an direct call for the
  /// indirect call 'CBIndirect' using the method 'ICSMethod'.
  void addIndirectCallBaseTarget(InlICSType ICSMethod, CallBase *CBIndirect,
                                 CallBase *CBDirect);

  // Return a clone of 'OldIRCS', but do not copy its children, and
  // use the IIMap to get a new value for the 'Call'.
  InlineReportCallSite *cloneBase(InlineReportCallSite *OldIRCS,
                                  const ValueToValueMapTy &IIMap,
                                  CallBase *ActiveInlineCallBase);

  // Set the target of the broken function bring called by 'CB' to 'F"
  void setBrokerTarget(CallBase *CB, Function *F);

  // Update the name of 'F' in the inlining report.
  void updateName(Function *F);

  // Create an InlineReportFunction to represent 'F'
  InlineReportFunction *addFunction(Function *F);

  // Create an InlineReportCallSite to represent Call
  InlineReportCallSite *addCallSite(CallBase *Call, bool AttachToCaller = true);

private:
  /// The Level is specified by the option -inline-report=N.
  /// See llvm/lib/Transforms/IPO/Inliner.cpp for details on Level.
  unsigned Level;

  // The CallBase for the call site currently being inlined
  CallBase *ActiveInlineCallBase;

  // The Caller currently being inlined into
  Function *ActiveCaller;

  // The Callee currently being inlined
  Function *ActiveCallee;

  // The InlineReportCallSite* of the CallSite currently being inlined
  InlineReportCallSite *ActiveIRCS;

  // Set of active inliners. When this goes to empty, we can print the
  // inlining report.
  SmallPtrSet<void *, 4> ActiveInliners;

  /// InlineFunction() fills this in with callsites that were cloned from
  /// the callee.  This is used only for inline report.
  SmallVector<Value *, 8> ActiveOriginalCalls;

  /// InlineFunction() fills this in with callsites that are cloned from
  /// the callee.  This is used only for inline report.
  SmallVector<Value *, 8> ActiveInlinedCalls;

  // The Module* of the SCC being tested for inlining
  Module *M;

  /// A mapping from Functions to InlineReportFunctions
  InlineReportFunctionMap IRFunctionMap;

  /// A mapping from CallBases to InlineReportCallSites
  InlineReportCallBaseCallSiteMap IRCallBaseCallSiteMap;

  /// A vector of InlineReportFunctions of Functions that have
  /// been eliminated by dead static function elimination
  InlineReportFunctionSet IRDeadFunctionSet;

  /// The output stream to print the inlining report to
  raw_ostream &OS;

  /// Clone the newly created callsites in 'IIMap' and append them to
  /// 'NewCallSite', while using 'IRFCallee' to update the compact form
  /// of 'IRFCaller'. Assumes that 'IRFCaller' is in compact form.
  void cloneChildrenCompact(InlineReportFunction *IRFCaller,
                            InlineReportFunction *IRFCallee,
                            InlineReportCallSite *NewCallSite,
                            ValueToValueMapTy &IIMap);

  /// Clone the vector of InlineReportCallSites for NewCallSite
  /// using the mapping of old calls to new calls IIMap
  void cloneChildren(InlineReportCallSiteVector &OldCallSiteVector,
                     InlineReportCallSite *NewCallSite,
                     ValueToValueMapTy &IIMap);

  /// Clone the call sites recursively in and under 'IRCSV' within
  /// 'OldIRCS' using 'VMap', placing them under 'NewIRCS'.
  void cloneCallSites(InlineReportCallSiteVector &IRCSV,
                      ValueToValueMapTy &VMap, InlineReportCallSite *OldIRCS,
                      InlineReportCallSite *NewIRCS);

  ///
  /// CallbackVM for Instructions and Functions in the InlineReport
  ///
  class InlineReportCallback : public CallbackVH {
    InlineReport *IR;
    void deleted() override {
      assert(IR);
      if (auto CB = dyn_cast<CallBase>(getValPtr())) {
        /// Indicate in the inline report that the call site
        /// corresponding to the Value has been deleted
        InlineReason Reason = NinlrDeleted;
        IR->removeCallBaseReference(*CB, Reason, true);
      } else if (auto F = dyn_cast<Function>(getValPtr())) {
        /// Indicate in the inline report that the function
        /// corresponding to the Value has been deleted
        IR->removeFunctionReference(*F, true);
      }
    }

  public:
    InlineReportCallback(Value *V, InlineReport *CBIR)
        : CallbackVH(V), IR(CBIR){};
    virtual ~InlineReportCallback(){};
  };

  DenseMap<Value *, InlineReportCallback *> CallbackMap;

  // Get the existing or create an InlineReportFunction to represent 'F'
  InlineReportFunction *getOrAddFunction(Function *F);

  // Get the existing or create an InlineReportCallSite to represent 'Call'
  InlineReportCallSite *getOrAddCallSite(CallBase *Call);

#ifndef NDEBUG
  /// Run some simple consistency checking on 'F'. For example,
  /// (1) Check that F is in the inline report's function map
  /// (2) Check that all of the call/invoke instructions in F's IR
  ///       appear in the inline report for F
  bool validateFunction(Function *F);
  /// Validate all of the functions in the IR function map
  bool validate(void);
#endif // NDEBUG

  /// Update the Functions in the call graph, so that all are current
  /// and all newly created callsites will appear in the inline report.
  /// Return 'true' if some function is modified.
  bool makeAllCurrent(void);

  /// Indicate that the inline reports may need to be made current
  /// with InlineReport::makeCurrent() before they are changed to indicate
  /// additional inlining.
  void makeAllNotCurrent(void);

  void addCallback(Value *V) {
    if (!V || CallbackMap.count(V))
      return;
    LLVM_DEBUG(dbgs() << "addCallback: " << V << "\n");
    CallbackMap[V] = new InlineReportCallback(V, this);
  }

  void removeCallback(Value *V) {
    if (!V || !CallbackMap.count(V))
      return;
    LLVM_DEBUG(dbgs() << "removeCallback: " << V << "\n");
    InlineReportCallback *IRCB = CallbackMap[V];
    CallbackMap.erase(V);
    delete IRCB;
  }

  InlineReportCallSite *getCallSite(CallBase *Call);
  InlineReportFunction *getFunction(Function *F);

  // Create a new InlineReportCallSite which corresponds to the 'VMap'ped
  // version of 'IRCS', insert it into the 'IRCallBaseCallSiteMap', and
  // create a callback for it.
  InlineReportCallSite *copyAndSetup(InlineReportCallSite *IRCS,
                                     ValueToValueMapTy &VMap);

  // Remove it from the IRCallSiteMap, remove its callback, and remove
  // all of its children if it represents an inlined call site.
  void removeIRCS(InlineReportCallSite *IRCS);
};

/// Pass to bring a Function up to date by including all its callsites in
/// the classic inlining report and excluding any which have been deleted.
class InlineReportMakeCurrentPass
    : public PassInfoMixin<InlineReportMakeCurrentPass> {
  static char PassID;

public:
  static bool isRequired() { return true; }
  InlineReportMakeCurrentPass(void);
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

/// Get the single, active classic inlining report.
InlineReport *getInlineReport();

class InlineReportPass : public PassInfoMixin<InlineReportPass> {
  static char PassID;

public:
  InlineReportPass(void);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#undef DEBUG_TYPE

#endif
