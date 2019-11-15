//===----------- Intel_InlineLists.cpp - [No]Inline Lists  ----------------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass assignes attributes to the call sites that appear in inline and
// noinline lists.
//
//===----------------------------------------------------------------------===//
//
#include "llvm/Transforms/IPO/Intel_InlineLists.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO.h"
#include <set>

#if INTEL_CUSTOMIZATION

using namespace llvm;

#define DEBUG_TYPE "inlinelists"

// The functions below are aimed to force inlining and not inlining at the
// user's demand. The following two options are used to specify which
// functions/callsites should (should not) be inlined:
//   -inline-inline-list
//   -inline-noinline-list

// The option is used to force inlining of functions in the list.
// Syntax: the list should be a sequence of <[caller,]callee[,linenum]>
// separated by ';'
cl::list<std::string>
    IntelInlineLists("inline-inline-list",
                     cl::desc("Force inlining of functions/callsites"),
                     cl::ReallyHidden);

// The option is used to force not inlining of functions in the list
// Syntax: the list should be a sequence of <[caller,]callee[,linenum]>
// separated by ';'
cl::list<std::string>
    IntelNoinlineLists("inline-noinline-list",
                       cl::desc("Force not inlining of functions/callsites"),
                       cl::ReallyHidden);

typedef llvm::StringSet<> CalleeSetTy;
typedef llvm::StringMap<std::set<uint32_t>> CalleeMapTy;
typedef llvm::StringMap<CalleeMapTy> CallerCalleeMapTy;

namespace {
class InlineListsData {
public:
  InlineListsData() {}
  // functions to inline everywhere
  CalleeSetTy InlineCalleeList;
  // functions to inline on special occasions
  CallerCalleeMapTy InlineCallerList;
  // functions to not inline anywhere
  CalleeSetTy NoinlineCalleeList;
  // functions to not inline on special occasions
  CallerCalleeMapTy NoinlineCallerList;

  // Returns true if no inline-[no]inline-list options
  bool isEmpty() {
    if (!InlineCalleeList.empty())
      return false;
    if (!NoinlineCalleeList.empty())
      return false;
    if (!InlineCallerList.empty())
      return false;
    if (!NoinlineCallerList.empty())
      return false;
    return true;
  }
};
} // namespace

/// \brief Print inline lists gathered from options.
///
/// \param OS  Stream to emit the output to.
/// \param CalleeList List to print.
/// \param CallerList List to print.
void printLists(raw_ostream &OS, CalleeSetTy &CalleeList,
                CallerCalleeMapTy &CallerList) {
  OS << "\nIPO everywhere: ";
  for (auto &Member : CalleeList) {
    OS << "\n\t\t callee: " << Member.first() << "\n";
  }
  OS << "\nIPO selective: ";
  for (auto &First : CallerList) {
    OS << "\n\t\t caller: " << First.first();
    for (auto &Second : First.second) {
      OS << "\n\t\t\t callee: " << Second.first();
      for (auto &Third : Second.second)
        OS << "\t line: " << Third << "\n";
    }
  }
  OS << "\n";
}

// The function parses a list and creates data structures that store information
// about callsites/functions which should or should not be inlined.
static void parseList(const StringRef &List, CalleeSetTy &CalleeList,
                      CallerCalleeMapTy &CallerList) {

  StringRef Option = List;
  // Option was empty - skip parsing.
  if (List.empty()) {
    return;
  }

  // The list consists of records separated by ';'
  SmallVector<StringRef, 8> ListRecords;
  Option.split(ListRecords, ';');
  for (auto ListRecord : ListRecords) {
    // Each record should be in the form of [caller,]callee[,line]
    SmallVector<StringRef, 3> RecordItem;
    StringRef Caller = "", Callee = "";
    int64_t LineNum = -1;

    ListRecord.split(RecordItem, ',');

    // Nothing to parse.
    if (RecordItem.size() == 0)
      return;

    // Record has too many fields.
    if (RecordItem.size() > 3) {
      LLVM_DEBUG(
          dbgs()
          << "IPO: error 1: record <" << ListRecord
          << "> has a wrong format. Should be <[caller,]callee[,line]>\n");
      return;
    }

    if (RecordItem.size() == 1) {
      // <callee> case
      // Inline it everywhere
      Callee = RecordItem[0];
      CalleeList.insert(RecordItem[0]);
    } else if (RecordItem.size() >= 2) {
      // <caller, callee> or <caller,callee,line> cases
      Caller = RecordItem[0];
      Callee = RecordItem[1];
      if (RecordItem.size() == 3) {
        // <caller, callee, linenum> case
        // Inline specified callsites
        if (RecordItem[2].getAsInteger(10, LineNum)) {
          LLVM_DEBUG(
              dbgs()
              << "IPO: error 1: record <" << ListRecord
              << "> has a wrong format. Should be <[caller,]callee[,line]>\n");
        }
      }

      auto CallerIt = CallerList.find(Caller);
      if (CallerIt != CallerList.end()) {
        // The caller is already in the map. Add callsites to it.
        auto CalleeIt = CallerIt->second.find(Callee);
        if (CalleeIt != CallerIt->second.end()) {
          // This callee is already in caller map.
          if (!CalleeIt->second.empty()) {
            // Update only if current inlining strategy is selective for this
            // pair of caller and callee.
            if (LineNum < 0) {
              // Change selective inlining to 'inline all callsites'.
              CalleeIt->second.clear();
            } else {
              // Add one more line number to inline current callee to the
              // caller.
              CalleeIt->second.insert(LineNum);
            }
          }
        } else {
          // No such callee for this caller yet: add a new one.
          std::set<uint32_t> NewLineSet;
          if (LineNum >= 0)
            NewLineSet.insert(LineNum);
          CallerIt->second.insert(
              std::make_pair(Callee, std::move(NewLineSet)));
        }
      } else {
        // The first appearance of the function in the CallerList.
        std::set<uint32_t> NewLineSet;
        if (LineNum >= 0)
          NewLineSet.insert(LineNum);
        CalleeMapTy NewCalleeMap;
        NewCalleeMap.insert(std::make_pair(Callee, std::move(NewLineSet)));
        CallerList.insert(std::make_pair(Caller, std::move(NewCalleeMap)));
      }

      if (Callee.empty())
        LLVM_DEBUG(
            dbgs()
            << "IPO: error 2: \t record <" << ListRecord
            << "> has a wrong format. Should be <[caller,]callee[,line]>\n");
    }
  }

  LLVM_DEBUG(printLists(dbgs(), CalleeList, CallerList));
}

// Function to parse inline and noinline lists.
static void parseOptions(InlineListsData &Data) {
  if (!IntelInlineLists.empty()) {
    LLVM_DEBUG(dbgs() << "IPO: Inline Lists \n");
    for (auto &InlineList : IntelInlineLists) {
      LLVM_DEBUG(dbgs() << "\tList: " << InlineList << "\n");
      parseList(StringRef(InlineList), Data.InlineCalleeList,
                Data.InlineCallerList);
    }
  }

  if (!IntelNoinlineLists.empty()) {
    LLVM_DEBUG(dbgs() << "IPO: Noinline Lists \n");
    for (auto &NoinlineList : IntelNoinlineLists) {
      LLVM_DEBUG(dbgs() << "\tList: " << NoinlineList << "\n");
      parseList(StringRef(NoinlineList), Data.NoinlineCalleeList,
                Data.NoinlineCallerList);
    }
  }
}

// The function checks if current triple <caller,callee,linenum> was in the
// list.
static bool isCallsiteInList(StringRef Caller, StringRef Callee,
                             int32_t LineNum, CalleeSetTy &CalleeList,
                             CallerCalleeMapTy &CallerList) {
  if (Callee.empty())
    return false;

  auto CalleeIt = CalleeList.find(Callee);
  if (CalleeIt != CalleeList.end()) {
    // Callee is in the list without any restrictions.
    return true;
  }

  if (Caller.empty())
    return false;

  auto CallerIt = CallerList.find(Caller);
  if (CallerIt == CallerList.end()) {
    // No such caller in the list.
    return false;
  }

  auto NewCalleeIt = CallerIt->second.find(Callee);
  if (NewCalleeIt == CallerIt->second.end()) {
    // No such callee paired with the caller.
    return false;
  }

  if (NewCalleeIt->second.empty()) {
    // Inline all appearances of the callee in the caller.
    return true;
  }

  if (LineNum < 0) {
    LLVM_DEBUG(dbgs() << "IPO warning: no line numbers available. "
                      << "Try compiling with -gline-tables-only.\n");
    return false;
  }

  auto LineIt = NewCalleeIt->second.find(LineNum);
  if (LineIt != NewCalleeIt->second.end()) {
    // <caller,callee,linenum> triple is in the list
    return true;
  }

  return false;
}

static void addForceNoinlineAttr(CallBase &CB, Function *Callee);

// Add AlwaysInline attribute to callsite.
static void addForceInlineAttr(CallBase &CB, Function *Callee) {
  // If Callee is noinline and we need to inline some of its calls then
  // noinline attributes goes to callsites from function definition.
  if (!Callee)
    return;

  if (Callee->hasFnAttribute(Attribute::NoInline)) {
    Callee->removeFnAttr(Attribute::AlwaysInline);
    Callee->removeFnAttr(Attribute::NoInline);
    if (Callee->hasFnAttribute(Attribute::OptimizeNone)) {
      Callee->removeFnAttr(Attribute::OptimizeNone);
    }
    for (auto I = Callee->use_begin(), E = Callee->use_end(); I != E; ++I) {
      CallInst *CI = dyn_cast_or_null<CallInst>(I->getUser());
      InvokeInst *II = dyn_cast_or_null<InvokeInst>(I->getUser());

      if (!(CI && CI->getCalledFunction() == Callee) &&
          !(II && II->getCalledFunction() == Callee))
        continue;

      auto NewCB = cast<CallBase>(I->getUser());
      addForceNoinlineAttr(*NewCB, Callee);
    }
  }

  if (CB.hasFnAttr(Attribute::NoInline)) {
    CB.removeAttribute(llvm::AttributeList::FunctionIndex, Attribute::NoInline);
  }
  CB.addAttribute(llvm::AttributeList::FunctionIndex, Attribute::AlwaysInline);
}

// Add NoInline attribute to callsite.
static void addForceNoinlineAttr(CallBase &CB, Function *Callee) {
  // If Callee is alwaysinline and we need to not inline some of its calls then
  // alwaysinline attributes goes to callsites from function definition.
  if (!Callee)
    return;

  if (Callee->hasFnAttribute(Attribute::AlwaysInline)) {
    Callee->removeFnAttr(Attribute::AlwaysInline);
    for (auto I = Callee->use_begin(), E = Callee->use_end(); I != E; ++I) {
      CallInst *CI = dyn_cast_or_null<CallInst>(I->getUser());
      InvokeInst *II = dyn_cast_or_null<InvokeInst>(I->getUser());

      if (!(CI && CI->getCalledValue() == Callee) &&
          !(II && II->getCalledValue() == Callee))
        continue;

      auto NewCB = cast<CallBase>(I->getUser());
      addForceInlineAttr(*NewCB, Callee);
    }
  }

  if (CB.hasFnAttr(Attribute::AlwaysInline)) {
    CB.removeAttribute(llvm::AttributeList::FunctionIndex,
                       Attribute::AlwaysInline);
  }
  CB.addAttribute(llvm::AttributeList::FunctionIndex, Attribute::NoInline);
}

// Add AlwaysInline attribute to function.
static bool addForceInlineAttr(Function &F) {
  if (F.hasFnAttribute(Attribute::AlwaysInline)) {
    return false;
  }

  if (F.hasFnAttribute(Attribute::NoInline)) {
    F.removeFnAttr(Attribute::NoInline);
    if (F.hasFnAttribute(Attribute::OptimizeNone)) {
      F.removeFnAttr(Attribute::OptimizeNone);
    }
  }
  F.addFnAttr(Attribute::AlwaysInline);
  return true;
}

// Add NoInline attribute to function.
static bool addForceNoinlineAttr(Function &F) {
  if (F.hasFnAttribute(Attribute::NoInline)) {
    return false;
  }

  if (F.hasFnAttribute(Attribute::AlwaysInline)) {
    F.removeFnAttr(Attribute::AlwaysInline);
  }
  F.addFnAttr(Attribute::NoInline);
  return true;
}

// First, assign AlwaysInline/NoInline attributes to functions.
bool addListAttributesToFunction(Function &F, InlineListsData &Data) {
  bool Changed = false;
  StringRef FuncName = F.getName();
  if (FuncName.empty())
    return false;

  bool inlineNeeded =
      Data.InlineCalleeList.find(FuncName) != Data.InlineCalleeList.end();
  bool noinlineNeeded =
      Data.NoinlineCalleeList.find(FuncName) != Data.NoinlineCalleeList.end();
  if (inlineNeeded && noinlineNeeded) {
    // Function has both inline and noinline attributes: skip it.
    LLVM_DEBUG(dbgs() << "IPO warning: ignoring '" << FuncName
                      << "' since it is in both inline and noinline lists\n");
    return false;
  } else if (inlineNeeded) {
    Changed |= addForceInlineAttr(F);
  } else if (noinlineNeeded) {
    Changed |= addForceNoinlineAttr(F);
  }

  return Changed;
}

// Check all callsites inside the function and assign corresponding attributes.
// Note: we do it after assigning attributes to functions to keep inline
// attributes consistent.
static bool addListAttributesToCallsites(Function &F, InlineListsData &Data) {
  bool Changed = false;
  StringRef CallerName = F.getName();

  // Go through each basic block and find all callsites.
  for (auto &BB : F) {
    for (auto &I : BB) {
      if (isa<CallInst>(&I) || isa<InvokeInst>(&I)) {
        CallInst *CI = dyn_cast_or_null<CallInst>(&I);
        InvokeInst *II = dyn_cast_or_null<InvokeInst>(&I);
        const DebugLoc DL = (dyn_cast<Instruction>(&I))->getDebugLoc();
        int64_t LineNum = DL ? DL.getLine() : -1;
        auto CalleeFunc =
            CI ? CI->getCalledFunction() : II->getCalledFunction();
        if (!CalleeFunc)
          continue;
        auto CalleeName = CalleeFunc->getName();

        bool NeedsInlineListAttr =
            isCallsiteInList(CallerName, CalleeName, LineNum,
                             Data.InlineCalleeList, Data.InlineCallerList);
        bool NeedsNoinlineListAttr =
            isCallsiteInList(CallerName, CalleeName, LineNum,
                             Data.NoinlineCalleeList, Data.NoinlineCallerList);

        if (auto CB = dyn_cast<CallBase>(&I)) {
          if (NeedsInlineListAttr && NeedsNoinlineListAttr) {
            // Callsite has both inline and noinline attributes: skip it.
            LLVM_DEBUG(dbgs()
                       << "IPO warning: ignoring triple <" << CallerName << ","
                       << CalleeName
                       << "> since it is in both inline and noinline lists\n");
          } else if (NeedsInlineListAttr) {
            // Assign InlineList attribute to callsite.
            addForceInlineAttr(*CB, CalleeFunc);
            Changed = true;
          } else if (NeedsNoinlineListAttr) {
            // Assign NoinlineList attribute to callsite.
            addForceNoinlineAttr(*CB, CalleeFunc);
            Changed = true;
          }
        }
      }
    }
  }

  return Changed;
}

static bool setInlineListsAttributes(Module &M) {
  InlineListsData Data;

  parseOptions(Data);

  if (Data.isEmpty()) {
    // No [no]inline list options in the compilation line - skip optimization
    return false;
  }

  bool Changed = false;

  for (Function &F : M)
    Changed |= addListAttributesToFunction(F, Data);

  for (Function &F : M)
    Changed |= addListAttributesToCallsites(F, Data);
  return Changed;
}

namespace {
struct InlineLists : public ModulePass {
  static char ID;
  InlineLists() : ModulePass(ID) {
    initializeInlineListsPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    return setInlineListsAttributes(M);
  }
};
} // namespace

char InlineLists::ID = 0;
INITIALIZE_PASS(InlineLists, "inlinelists",
                "Set attributes for callsites in [no]inline list", false, false)

ModulePass *llvm::createInlineListsPass() { return new InlineLists; }

PreservedAnalyses InlineListsPass::run(Module &M, ModuleAnalysisManager &AM) {
  setInlineListsAttributes(M);
  return PreservedAnalyses::all();
}

#endif // INTEL_CUSTOMIZATION
