//===----------- Intel_InlineLists.cpp - [No]Inline Lists  ----------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
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
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Intel_MDInlineReport.h"
#include <set>

using namespace llvm;

#define DEBUG_TYPE "inlinelists"

// The functions below are aimed to force inlining and not inlining at the
// user's demand. The following three options are used to specify which
// functions/callsites should (should not) be inlined:
//   -inline-inline-list
//   -inline-noinline-list
//   -inline-recursive-list

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

// The option is used to force recursive inlining of functions in the list.
// Syntax: the list should be a sequence of <[caller,]callee[,linenum]>
// separated by ';'
cl::list<std::string> IntelInlineRecLists(
    "inline-recursive-list",
    cl::desc("Force recursive inlining of functions/callsites"),
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
  // functions to inline recursive everywhere
  CalleeSetTy InlineRecCalleeList;
  // functions to inline recursive on special occasions
  CallerCalleeMapTy InlineRecCallerList;

  // Returns true if no inline-[no]inline-list options
  bool isEmpty() {
    if (!InlineCalleeList.empty())
      return false;
    if (!NoinlineCalleeList.empty())
      return false;
    if (!InlineRecCalleeList.empty())
      return false;
    if (!InlineCallerList.empty())
      return false;
    if (!NoinlineCallerList.empty())
      return false;
    if (!InlineRecCallerList.empty())
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
  for (const auto &ListRecord : ListRecords) {
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

// Function to parse inline, inline recursive and noinline lists.
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

  if (!IntelInlineRecLists.empty()) {
    LLVM_DEBUG(dbgs() << "IPO: Inline Recursive Lists \n");
    for (auto &InlineRecursiveList : IntelInlineRecLists) {
      LLVM_DEBUG(dbgs() << "\tList: " << InlineRecursiveList << "\n");
      parseList(StringRef(InlineRecursiveList), Data.InlineRecCalleeList,
                Data.InlineRecCallerList);
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

// Add AlwaysInline attribute to callsite.
static void addForceInlineAttr(CallBase &CB, Function *Callee) {
  if (!Callee)
    return;

  if (CB.hasFnAttr(Attribute::NoInline))
    CB.removeFnAttr(Attribute::NoInline);

  if (CB.hasFnAttr(Attribute::AlwaysInlineRecursive))
    CB.removeFnAttr(Attribute::AlwaysInlineRecursive);

  CB.addFnAttr(Attribute::AlwaysInline);

  // used for inline report reason
  CB.addFnAttr("inline-list");
}

// Add ForceInlineRecursive attribute to callsite.
static void addForceInlineRecursiveAttr(CallBase &CB, Function *Callee) {
  if (!Callee)
    return;

  if (CB.hasFnAttr(Attribute::NoInline))
    CB.removeFnAttr(Attribute::NoInline);

  if (CB.hasFnAttr(Attribute::AlwaysInline))
    CB.removeFnAttr(Attribute::AlwaysInline);

  CB.addFnAttr(Attribute::AlwaysInlineRecursive);

  // used for inline report reason
  CB.addFnAttr("inline-recursive-list");
}

// Add NoInline attribute to callsite.
static void addForceNoinlineAttr(CallBase &CB, Function *Callee) {
  if (!Callee)
    return;

  if (CB.hasFnAttr(Attribute::AlwaysInline))
    CB.removeFnAttr(Attribute::AlwaysInline);

  if (CB.hasFnAttr(Attribute::AlwaysInlineRecursive))
    CB.removeFnAttr(Attribute::AlwaysInlineRecursive);

  CB.addFnAttr(Attribute::NoInline);

  // used for inline report reason
  CB.addFnAttr("noinline-list");
}

// Add AlwaysInline attribute to function.
static bool addForceInlineAttr(Function &F) {
  if (F.hasFnAttribute(Attribute::AlwaysInline))
    return false;

  if (F.hasFnAttribute(Attribute::NoInline)) {
    F.removeFnAttr(Attribute::NoInline);
    if (F.hasFnAttribute(Attribute::OptimizeNone))
      F.removeFnAttr(Attribute::OptimizeNone);
  }

  if (F.hasFnAttribute(Attribute::AlwaysInlineRecursive))
    F.removeFnAttr(Attribute::AlwaysInlineRecursive);

  F.addFnAttr(Attribute::AlwaysInline);
  return true;
}

// Add AlwaysInlineRecursive attribute to function.
static bool addForceInlineRecursiveAttr(Function &F) {
  if (F.hasFnAttribute(Attribute::AlwaysInlineRecursive))
    return false;

  if (F.hasFnAttribute(Attribute::NoInline)) {
    F.removeFnAttr(Attribute::NoInline);
    if (F.hasFnAttribute(Attribute::OptimizeNone))
      F.removeFnAttr(Attribute::OptimizeNone);
  }

  if (F.hasFnAttribute(Attribute::AlwaysInline))
    F.removeFnAttr(Attribute::AlwaysInline);

  F.addFnAttr(Attribute::AlwaysInlineRecursive);
  return true;
}

// Add NoInline attribute to function.
static bool addForceNoinlineAttr(Function &F) {
  if (F.hasFnAttribute(Attribute::NoInline))
    return false;

  if (F.hasFnAttribute(Attribute::AlwaysInline))
    F.removeFnAttr(Attribute::AlwaysInline);

  if (F.hasFnAttribute(Attribute::AlwaysInlineRecursive))
    F.removeFnAttr(Attribute::AlwaysInlineRecursive);

  F.addFnAttr(Attribute::NoInline);
  return true;
}

// First, assign AlwaysInline[Recursive]/NoInline attributes to functions.
bool addListAttributesToFunction(Function &F, InlineListsData &Data) {
  bool Changed = false;
  StringRef FuncName = F.getName();
  if (FuncName.empty())
    return false;

  bool inlineNeeded =
      Data.InlineCalleeList.find(FuncName) != Data.InlineCalleeList.end();
  bool noinlineNeeded =
      Data.NoinlineCalleeList.find(FuncName) != Data.NoinlineCalleeList.end();
  bool inlineRecNeeded =
      Data.InlineRecCalleeList.find(FuncName) != Data.InlineRecCalleeList.end();

  int NumAttrsToApply = 0;
  if (inlineNeeded)
    NumAttrsToApply++;
  if (noinlineNeeded)
    NumAttrsToApply++;
  if (inlineRecNeeded)
    NumAttrsToApply++;

  if (NumAttrsToApply > 1) {
    // Function has both inline and noinline attributes: skip it.
    LLVM_DEBUG(dbgs() << "IPO warning: ignoring '" << FuncName
                      << "' since it is in multiple lists\n");
    return false;
  } else if (inlineNeeded) {
    Changed |= addForceInlineAttr(F);
  } else if (noinlineNeeded) {
    Changed |= addForceNoinlineAttr(F);
  } else if (inlineRecNeeded) {
    Changed |= addForceInlineRecursiveAttr(F);
  }

  return Changed;
}

// Check all callsites inside the function and assign corresponding attributes.
// Note: we do it after assigning attributes to functions to keep inline
// attributes consistent.
static bool addListAttributesToCallsites(Function &F, InlineListsData &Data) {
  bool Changed = false;
  StringRef CallerName = F.getName();

  // Go through each instruction and find all callsites.
  for (auto &I : instructions(F)) {
    if (auto *CB = dyn_cast<CallBase>(&I)) {
      const DebugLoc DL = (dyn_cast<Instruction>(&I))->getDebugLoc();
      int64_t LineNum = DL ? DL.getLine() : -1;
      auto *CalleeFunc = CB->getCalledFunction();
      if (!CalleeFunc)
        continue;
      auto CalleeName = CalleeFunc->getName();

      bool NeedsInlineListAttr =
          isCallsiteInList(CallerName, CalleeName, LineNum,
                           Data.InlineCalleeList, Data.InlineCallerList);
      bool NeedsNoinlineListAttr =
          isCallsiteInList(CallerName, CalleeName, LineNum,
                           Data.NoinlineCalleeList, Data.NoinlineCallerList);
      bool NeedsInlineRecListAttr =
          isCallsiteInList(CallerName, CalleeName, LineNum,
                           Data.InlineRecCalleeList, Data.InlineRecCallerList);

      int NumAttrsToApply = 0;
      if (NeedsInlineListAttr)
        NumAttrsToApply++;
      if (NeedsNoinlineListAttr)
        NumAttrsToApply++;
      if (NeedsInlineRecListAttr)
        NumAttrsToApply++;

      if (NumAttrsToApply > 1) {
        // Callsite has both inline and noinline attributes: skip it.
        LLVM_DEBUG(dbgs() << "IPO warning: ignoring triple <" << CallerName
                          << "," << CalleeName
                          << "> since it is in multiple lists\n");
      } else if (NeedsInlineListAttr) {
        // Assign InlineList attribute to callsite.
        addForceInlineAttr(*CB, CalleeFunc);
        Changed = true;
      } else if (NeedsNoinlineListAttr) {
        // Assign NoinlineList attribute to callsite.
        addForceNoinlineAttr(*CB, CalleeFunc);
        Changed = true;
      } else if (NeedsInlineRecListAttr) {
        // assign InlineRecursiveList attribute to callsite.
        addForceInlineRecursiveAttr(*CB, CalleeFunc);
        Changed = true;
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

PreservedAnalyses InlineListsPass::run(Module &M, ModuleAnalysisManager &AM) {
  setInlineListsAttributes(M);
  return PreservedAnalyses::all();
}
