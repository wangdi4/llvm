//===- Intel_InlineReport.h - Implement inlining report ---------*- C++ -*-===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_CallGraphReport.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Analysis/LazyCallGraph.h"
#include "llvm/Transforms/IPO/Intel_InlineReportCommon.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <climits>

namespace llvm {

class InlineReportCallSite;

typedef std::vector<InlineReportCallSite *> InlineReportCallSiteVector;

class InlineReportFunction;

///
/// \brief Represents a CallSite in the inlining report
///
class InlineReportCallSite {
public:
  // \brief Constructor for InlineReportCallSite
  // The source file is given by 'M'.  The line and column info by 'Dloc'
  explicit InlineReportCallSite(InlineReportFunction *IRCallee, bool IsInlined,
                                InlineReportTypes::InlineReason Reason,
                                Module *Module, DebugLoc *DLoc, Instruction *I)
      : IRCallee(IRCallee), IsInlined(IsInlined), Reason(Reason),
        InlineCost(-1), OuterInlineCost(-1), InlineThreshold(-1),
        EarlyExitInlineCost(INT_MAX), EarlyExitInlineThreshold(INT_MAX),
        Call(I), M(Module) {
    Line = DLoc && DLoc->get() ? DLoc->getLine() : 0;
    Col = DLoc && DLoc->get() ? DLoc->getCol() : 0;
    Children.clear();
  };

  ~InlineReportCallSite(void);
  InlineReportCallSite(const InlineReportCallSite &) = delete;
  void operator=(const InlineReportCallSite &) = delete;

  // \brief Return a clone of *this, but do not copy its children, and
  // use the IIMap to get a new value for the 'Call'.
  InlineReportCallSite *cloneBase(const ValueToValueMapTy &IIMap,
                                  Instruction *ActiveInlineInstruction);

  InlineReportFunction *getIRCallee() const { return IRCallee; }
  InlineReportTypes::InlineReason getReason() const { return Reason; }
  void setReason(InlineReportTypes::InlineReason MyReason) {
    Reason = MyReason;
  }
  bool getIsInlined() const { return IsInlined; }
  void setIsInlined(bool Inlined) { IsInlined = Inlined; }

  /// \brief Return true if in the original inlining process there would be
  /// early exit due to high cost.
  bool isEarlyExit() const { return EarlyExitInlineCost != INT_MAX; }

  /// \brief Return the vector of InlineReportCallSites which represent
  /// the calls made from the section of inlined code represented by
  /// this InlineReportCallSite.
  const InlineReportCallSiteVector &getChildren() { return Children; }

  /// \brief Inlining is inhibited if the inline cost is greater than
  /// the threshold.
  int getInlineCost() const { return InlineCost; }
  void setInlineCost(int Cost) { InlineCost = Cost; }

  /// \brief Stored "early exit" cost of inlining.
  int getEarlyExitInlineCost() const { return EarlyExitInlineCost; }
  void setEarlyExitInlineCost(int EECost) { EarlyExitInlineCost = EECost; }

  /// \brief Since inlining is bottom up, always selecting the leaf-most
  /// call sites for inlining is not always best, as it may inhibit inlining
  /// further up the call tree.  Therefore, in addition to an inlining cost,
  /// the inliner computes an outer inlining cost as well.  Inlining is also
  /// inhibited if the outer inlining cost is greater than the inline
  /// threshold.
  int getOuterInlineCost() const { return OuterInlineCost; }
  void setOuterInlineCost(int Cost) { OuterInlineCost = Cost; }

  int getInlineThreshold() const { return InlineThreshold; }
  void setInlineThreshold(int Threshold) { InlineThreshold = Threshold; }

  /// \brief Stored "early exit" threshold of inlining.
  int getEarlyExitInlineThreshold() const { return EarlyExitInlineThreshold; }
  void setEarlyExitInlineThreshold(int EEThreshold) {
    EarlyExitInlineThreshold = EEThreshold;
  }
  Instruction *getCall() const { return Call; }
  void setCall(Instruction *Call) { this->Call = Call; }
  void addChild(InlineReportCallSite *IRCS) { Children.push_back(IRCS); }

  /// \brief Print the info in the inlining instance for the inling report
  /// indenting 'indentCount' indentations, assuming an inlining report
  /// level of 'ReportLevel'.
  void print(unsigned IndentCount, unsigned ReportLevel);

  /// \brief Load the call represented by '*this' and all of its descendant
  /// calls into the map 'Lmap'.
  void loadCallsToMap(std::map<Instruction *, bool> &LMap);

private:
  InlineReportFunction *IRCallee;
  bool IsInlined;
  InlineReportTypes::InlineReason Reason;
  int InlineCost;
  int OuterInlineCost;
  int InlineThreshold;
  int EarlyExitInlineCost;
  int EarlyExitInlineThreshold;
  InlineReportCallSiteVector Children;
  Instruction *Call;
  ///
  /// \brief Used to get the file name when we print the report
  Module *M;
  ///
  /// \brief The line and column number of the call site.  These are 0 if
  /// we are not compiling with -g or the lighter weight version
  /// -gline-tables-only
  unsigned Line;
  unsigned Col;
  void printCostAndThreshold(unsigned Level);
  void printOuterCostAndThreshold(void);
  void printCalleeNameModuleLineCol(unsigned Level);
  // \brief Return a pointer to a copy of Base with an empty Children vector
  InlineReportCallSite *copyBase(const InlineReportCallSite &Base,
                                 Instruction *NI);
};

///
/// \brief Represents a routine (compiled or dead) in the inlining report
///
class InlineReportFunction {
public:
  explicit InlineReportFunction(const Function *F)
      : IsDead(false), IsCurrent(false), IsDeclaration(false),
        LinkageChar(' '){};
  ~InlineReportFunction(void);
  InlineReportFunction(const InlineReportFunction &) = delete;
  void operator=(const InlineReportFunction &) = delete;

  /// \brief A vector of InlineReportCallSites representing the top-level
  /// call sites in a function (i.e. those which appear in the source code
  /// of the function).
  const InlineReportCallSiteVector &getCallSites() { return CallSites; }

  /// \brief Add an InlineReportCallSite to the list of top-level calls for
  /// this function.
  void addCallSite(InlineReportCallSite *IRCS) { CallSites.push_back(IRCS); }

  /// \brief Return true if the function has been dead code eliminated.
  bool getDead() const { return IsDead; }

  /// \brief Set whether the function is dead code eliminated.
  void setDead(bool Dead) { IsDead = Dead; }

  /// \brief Return true if the inline report for this routine reflects
  /// the changes that have been made to the routine since the last call
  /// to Inliner::runOnSCC()
  bool getCurrent(void) const { return IsCurrent; }

  /// \brief Set whether the inline report for the routine is current
  void setCurrent(bool Current) { IsCurrent = Current; }

  bool getIsDeclaration(void) const { return IsDeclaration; }

  void setIsDeclaration(bool Declaration) { IsDeclaration = Declaration; }

  /// brief Get a single character indicating the linkage type
  char getLinkageChar(void) { return LinkageChar; }

  /// brief Set a single character indicating the linkage type
  void setLinkageChar(Function *F) {
    LinkageChar =
      (F->hasLocalLinkage()
           ? 'L'
           : (F->hasLinkOnceODRLinkage()
                  ? 'O'
                  : (F->hasAvailableExternallyLinkage() ? 'X' : 'A')));
  }

  std::string &getName() { return Name; }

  void setName(std::string FunctionName) { Name = FunctionName; }

  void print(unsigned Level) const;

private:
  bool IsDead;
  bool IsCurrent;
  bool IsDeclaration;
  char LinkageChar;
  std::string Name;
  InlineReportCallSiteVector CallSites;
};

typedef MapVector<Function *, InlineReportFunction *> InlineReportFunctionMap;
typedef std::vector<InlineReportFunction *> InlineReportFunctionVector;
typedef std::map<Instruction *, InlineReportCallSite *>
    InlineReportInstructionCallSiteMap;

///
/// \brief The inlining report
///
class InlineReport : public CallGraphReport {
public:
  explicit InlineReport(unsigned MyLevel)
      : Level(MyLevel), ActiveInlineInstruction(nullptr),
        ActiveCallee(nullptr), ActiveIRCS(nullptr), M(nullptr) {};
  virtual ~InlineReport(void);

  // \brief Indicate that we have begun inlining functions in the current
  // SCC of the CG.
  void beginSCC(CallGraph &CG, CallGraphSCC &SCC);
  void beginSCC(LazyCallGraph &CG, LazyCallGraph::SCC &SCC);
  void beginFunction(Function *F);

  // \brief Indicate that we are done inlining functions in the current SCC.
  void endSCC();

  void beginUpdate(CallSite CS) {
    ActiveCallee = CS.getCalledFunction();
    // New call sites can be added from inlining even if they are not a
    // cloned from the inlined callee.
    ActiveIRCS = addNewCallSite(CS.getCaller(), CS, M);
    ActiveInlineInstruction = CS.getInstruction();
    ActiveOriginalCalls.clear();
    ActiveInlinedCalls.clear();
  }

  void endUpdate() {
    ActiveCallee = nullptr;
    ActiveIRCS = nullptr;
    ActiveInlineInstruction = nullptr;
    ActiveOriginalCalls.clear();
    ActiveInlinedCalls.clear();
  }

  /// \brief Indicate that the current CallSite CS has been inlined in
  /// the inline report.  Use the InlineInfo collected during inlining
  /// to update the report.
  void inlineCallSite();

  // \brief Indicate that the Function is dead
  void setDead(Function *F) {
    if (!isClassicIREnabled())
      return;
    InlineReportFunctionMap::const_iterator MapIt = IRFunctionMap.find(F);
    assert(MapIt != IRFunctionMap.end());
    InlineReportFunction *INR = MapIt->second;
    INR->setDead(true);
  }

  /// \brief Print the inlining report at the given level.
  void print() const;

  /// \brief Check if report has data
  bool isEmpty() { return IRFunctionMap.empty(); }

  // The level of the inline report
  unsigned getLevel() { return Level; }

  // \brief Check if classic inline report should be created
  bool isClassicIREnabled() const {
    return (Level && !(Level & InlineReportTypes::BasedOnMetadata));
  }

  /// \brief Record the reason a call site is or is not inlined.
  void setReasonNotInlined(const CallSite CS,
                           InlineReportTypes::InlineReason Reason);
  void setReasonNotInlined(const CallSite CS, const InlineCost &IC);
  void setReasonNotInlined(const CallSite CS, const InlineCost &IC,
                           int TotalSecondaryCost);
  void setReasonIsInlined(const CallSite CS,
                          InlineReportTypes::InlineReason Reason);
  void setReasonIsInlined(const CallSite CS, const InlineCost &IC);

  void replaceFunctionWithFunction(Function *OldFunction,
                                   Function *NewFunction) override;

  /// Add a pair of old and new call sites.  The 'NewCall' is a clone of
  /// the 'OldCall' produced by InlineFunction().
  void addActiveCallSitePair(Value *OldCall, Value *NewCall) {
    ActiveOriginalCalls.push_back(OldCall);
    ActiveInlinedCalls.push_back(NewCall);
    addCallback(NewCall);
  }

  /// Update the 'OldCall' to 'NewCall' in the ActiveInlinedCalls.
  /// This needs to happen if we manually replace an active inlined
  /// call during InlineFunction().
  void updateActiveCallSiteTarget(Value *OldCall, Value *NewCall) {
    for (unsigned I = 0; I < ActiveInlinedCalls.size(); ++I)
      if (ActiveInlinedCalls[I] == OldCall) {
        ActiveInlinedCalls[I] = NewCall;
        addCallback(NewCall);
        break;
      }
  }

  // Indicate that the CallSite whose Instruction is 'I' has been
  // eliminated as dead code.
  void removeCallSiteReference(Instruction &I) {
    if (ActiveInlineInstruction != &I) {
      InlineReportInstructionCallSiteMap::const_iterator MapIt;
      MapIt = IRInstructionCallSiteMap.find(&I);
      if (MapIt != IRInstructionCallSiteMap.end()) {
        InlineReportCallSite *IRCS = MapIt->second;
        IRInstructionCallSiteMap.erase(MapIt);
        IRCS->setReason(InlineReportTypes::NinlrDeleted);
      }
    }
    // If necessary, remove any reference in the ActiveInlinedCalls
    for (unsigned II = 0, E = ActiveInlinedCalls.size(); II < E; ++II)
      if (ActiveInlinedCalls[II] == &I)
        ActiveInlinedCalls[II] = nullptr;
  }

  // Indicate that the Function 'F' has been eliminated as a dead static
  // function.
  void removeFunctionReference(Function &F) {
    InlineReportFunctionMap::iterator MapIt;
    MapIt = IRFunctionMap.find(&F);
    if (MapIt != IRFunctionMap.end()) {
      InlineReportFunction *IRF = MapIt->second;
      setDead(&F);
      IRF->setLinkageChar(&F);
      IRFunctionMap.erase(MapIt);
      IRDeadFunctionVector.push_back(IRF);
    }
  }

private:
  /// \brief The Level is specified by the option -inline-report=N.
  /// See llvm/lib/Transforms/IPO/Inliner.cpp for details on Level.
  unsigned Level;

  // \brief The instruction for the call site currently being inlined
  Instruction *ActiveInlineInstruction;

  // \brief The Callee currently being inlined
  Function *ActiveCallee;

  // \brief The InlineReportCallSite* of the CallSite currently being inlined
  InlineReportCallSite *ActiveIRCS;

  /// InlineFunction() fills this in with callsites that were cloned from
  /// the callee.  This is used only for inline report.
  SmallVector<Value *, 8> ActiveOriginalCalls;

  /// InlineFunction() fills this in with callsites that are cloned from
  /// the callee.  This is used only for inline report.
  SmallVector<Value *, 8> ActiveInlinedCalls;

  // \brief The Module* of the SCC being tested for inlining
  Module *M;

  /// \brief A mapping from Functions to InlineReportFunctions
  InlineReportFunctionMap IRFunctionMap;

  /// \brief A mapping from Instructions to InlineReportCallSites
  InlineReportInstructionCallSiteMap IRInstructionCallSiteMap;

  /// \brief A vector of InlineReportFunctions of Functions that have
  /// been eliminated by dead static function elimination
  InlineReportFunctionVector IRDeadFunctionVector;

  /// \brief Clone the vector of InlineReportCallSites for NewCallSite
  /// using the mapping of old calls to new calls IIMap
  void cloneChildren(const InlineReportCallSiteVector &OldCallSiteVector,
                     InlineReportCallSite *NewCallSite,
                     ValueToValueMapTy &IIMap);

  ///
  /// \brief CallbackVM for Instructions and Functions in the InlineReport
  ///
  class InlineReportCallback : public CallbackVH {
    InlineReport *IR;
    void deleted() override {
      assert(IR != nullptr);
      if (isa<Instruction>(getValPtr())) {
        /// \brief Indicate in the inline report that the call site
        /// corresponding to the Value has been deleted
        Instruction *I = cast<Instruction>(getValPtr());
        IR->removeCallSiteReference(*I);
      } else if (isa<Function>(getValPtr())) {
        /// \brief Indicate in the inline report that the function
        /// corresponding to the Value has been deleted
        Function *F = cast<Function>(getValPtr());
        IR->removeFunctionReference(*F);
      }
      setValPtr(nullptr);
    }

  public:
    InlineReportCallback(Value *V, InlineReport *CBIR)
        : CallbackVH(V), IR(CBIR){};
    virtual ~InlineReportCallback(){};
  };

  SmallVector<InlineReportCallback *, 16> IRCallbackVector;

  // \brief Create an InlineReportFunction to represent F
  InlineReportFunction *addFunction(Function *F, Module *M);

  // \brief Create an InlineReportCallSite to represent CS
  InlineReportCallSite *addCallSite(Function *F, CallSite CS, Module *M);

  // \brief Create an InlineReportCallSite to represent CS, if one does
  // not already exist
  InlineReportCallSite *addNewCallSite(Function *F, CallSite CS, Module *M);

#ifndef NDEBUG
  /// \brief Run some simple consistency checking on 'F', e.g.
  /// (1) Check that F is in the inline report's function map
  /// (2) Check that all of the call/invoke instructions in F's IR
  ///       appear in the inline report for F
  bool validateFunction(Function *F);
  /// \brief Validate all of the functions in the IR function map
  bool validate(void);
#endif // NDEBUG

  /// \brief Ensure that the inline report for this routine reflects the
  /// changes thatr have been made to that routine since the last call to
  /// Inliner::runOnSCC()
  void makeCurrent(Module *M, Function *F);

  /// \brief Indicate that the inline reports may need to be made current
  /// with InlineReport::makeCurrent() before they are changed to indicate
  /// additional inlining.
  void makeAllNotCurrent(void);

  void addCallback(Value *V) {
    InlineReportCallback *IRCB = new InlineReportCallback(V, this);
    IRCallbackVector.push_back(IRCB);
  }

  InlineReportCallSite *getCallSite(CallSite CS);
};

} // namespace llvm

#endif
