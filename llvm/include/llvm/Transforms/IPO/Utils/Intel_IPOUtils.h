//===----  Intel_IPOUtils.h - IPO Utility Functions   --------===//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file collects utility functions shared by various IPO passes, and
// their implementations may be split between the .h file and .cpp file.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_UTIILTY_H
#define LLVM_TRANSFORMS_IPO_INTEL_UTIILTY_H

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"

#include <sstream>

namespace llvm {

class DominatorTree;
class PostDominatorTree;
class TargetLibraryInfo;

class IPOUtils {
public:
  // Check: is the given Function a leaf function?
  // A leaf function has NO function call(s) in it.
  static bool isLeafFunction(const Function &F);

  // Check: is the given value V within the range of [LB, UB]?
  template <typename T> static bool isInRange(T V, T LB, T UB) {
    assert((LB <= UB) && "Expect LB <= UB\n");
    return (V >= LB) && (V <= UB);
  }

  // Count the # of Integer Argument(s) in a given Function
  static unsigned countIntArgs(const Function &F);

  // Count the # of Pointer Argument(s) in a given Function
  static unsigned countPtrArgs(const Function &F);

  // Check if there is any floating-point argument
  static bool hasFloatArg(const Function &F);

  // Provide a StringRef used as MDNode key to suppress inline report
  static StringRef getSuppressInlineReportStringRef(void) {
    return StringRef("InlRpt.Suppress");
  }

  static bool preserveOrSuppressInlineReport(Instruction *I, Instruction *NI);

  static bool isProdBuild(void) {
#if defined(NDEBUG)
    return true;
#endif
    return false;
  }
};

// This helper class is used to find candidates for multiversioning based on
// function signature. It is now extended as a generic filter to identify
// function candidates.
//
// 2 current users are:
// - Intel Call Tree Cloning pass,
// - Intel IPO Prefetch pass.
//
class FunctionSignatureMatcher {
public:
  // Default constructor, needed for MultiVersionImpl's type declaration
  FunctionSignatureMatcher()
      : MinNumArgs(0), MaxNumArgs(0), MinNumIntArgs(0), MaxNumIntArgs(0),
        IsLeaf(false) {}

  // [Note]
  // Argument NumPtrArgs0 and NumPtrArgs1 represent individual values instead of
  // a value range found on NumArgs, NumIntArgs, and NumDoublePtrArgs.
  FunctionSignatureMatcher(unsigned MinNumArgs, unsigned MaxNumArgs,
                           unsigned MinNumIntArgs, unsigned MaxNumIntArgs,
                           unsigned NumPtrArgs0, unsigned NumPtrArgs1,
                           bool IsLeaf)
      : MinNumArgs(MinNumArgs), MaxNumArgs(MaxNumArgs),
        MinNumIntArgs(MinNumIntArgs), MaxNumIntArgs(MaxNumIntArgs),
        IsLeaf(IsLeaf) {

    assert((MinNumArgs <= MaxNumArgs) && "Expect MinNumArgs <= MaxNumArgs");

    // Note: NumPtrArgs are treated as individual values, rather than a range!
    NumPtrArgsV.push_back(NumPtrArgs0);
    NumPtrArgsV.push_back(NumPtrArgs1);
  }

  // Try to match Function* F's signature with the signature provided inside a
  // FunctionSignatureMatcher object.
  //
  // E.g.
  // FunctionSignatureMatcher FSM(3 /*MinNumArgs*/, 3 /*MaxNumArgs*/,
  //                              1 /*MinNumIntArgs*/, 1 /*MaxNumIntArgs*/,
  //                              2 /*NumPtrArg0*/, 2 /*NumPtrArg1*/,
  //                              true /*IsLeaf*/);
  //
  // The above FSM object is used to match function foo(), where
  // foo's prototype is: void foo(int, int *, int **),
  // foo's callsite is: foo(1, p, q), assuming the following types defined:
  //   int * p;
  //   int **q;
  // and foo is a leaf function.
  //
  // FSM.match(F) will return true, because
  // - NumArg matches: foo has 3 arguments
  // - NumIntArg matches: foo has 1 integer argument
  // - NumPtrArg matches: foo has 2 ptr arg (both p and q are ptr type)
  // - NumIntArg + NumPtrArg == NumArg: foo has 3 arguments, 1 is a integer
  //                                    and 2 are pointers
  // and
  // - LeafFunc matches: foo is a leaf function
  //
  // Note:
  // NumArg, and NumIntArg are provided in pairs because the same FSM object
  // will be used to match multiple function calls. So the match is
  // not necessarily exact.
  //
  // NumArgs are used as bounds of a range, so search within a range is enough.
  // However, for NumPtrArgs, it needs to be 2 concrete values instead of a
  // range. As a result, the 2 individual values are pushed into NumPtrArgV
  // vector, and searched linearly using std::find() or std::find_if().
  //
  bool match(Function *F) const {
    assert(F && "Expect Function* F be a valid pointer\n");

    // Check IsLeaf:
    if (IsLeaf && !IPOUtils::isLeafFunction(*F))
      return false;

    // Check MinNumArgs, MaxNumArgs: strict compare
    unsigned ArgCount = F->arg_size();
    if (!IPOUtils::isInRange<unsigned>(ArgCount, MinNumArgs, MaxNumArgs))
      return false;

    // Check MinNumIntAgs, MaxNumIntArgs: strict compare
    unsigned IntArgCount = IPOUtils::countIntArgs(*F);
    if (!IPOUtils::isInRange<unsigned>(IntArgCount, MinNumIntArgs,
                                       MaxNumIntArgs))
      return false;

    // Check PtrArgCount matches by NumPtrArgs0 or NumPtrArgs1
    unsigned PtrArgCount = IPOUtils::countPtrArgs(*F);
    if (std::find(NumPtrArgsV.begin(), NumPtrArgsV.end(), PtrArgCount) ==
        NumPtrArgsV.end())
      return false;

    // Not allowing any Float argument type
    if (IPOUtils::hasFloatArg(*F))
      return false;

    // The number of arguments that are integer and arguments that are pointers
    // should be equal to the number of arguments
    if (IntArgCount + PtrArgCount != ArgCount)
      return false;

    return true;
  }

private:
  unsigned MinNumArgs, MaxNumArgs;
  unsigned MinNumIntArgs, MaxNumIntArgs;
  std::vector<unsigned> NumPtrArgsV;
  bool IsLeaf;

public:
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD std::string toString(void) const {
    std::ostringstream S;
    S << "FunctionSignature Object: \n";

    // unsigned MinNumArgs, MaxNumArgs:
    S << "MinNumArgs: " << MinNumArgs << ", MaxNumArgs: " << MaxNumArgs << "\n";

    // unsigned MinNumIntArgs, MaxNumIntArgs:
    S << "MinNumIntArgs: " << MinNumIntArgs
      << ", MaxNumIntArgs: " << MaxNumIntArgs << "\n";

    // std::vector<unsigned> NumPtrArgsV:
    S << "NumPtrArgs: ";
    for (auto V : NumPtrArgsV)
      S << V << ", ";
    S << "\n";

    // IsLeaf:
    S << "IsLeaf: " << std::boolalpha << IsLeaf << "\n";

    return S.str();
  }

  LLVM_DUMP_METHOD void dump(void) const { llvm::dbgs() << toString(); }
#endif
};

// Class to analyze any malloc-like and/or free calls on the module level.
//
// For any malloc-like (malloc, calloc, realloc, new, etc.) call, it will
// identify the host functions where the malloc-like calls reside, and
// construct a closure that the malloc may reach in the call graph.
//
// Similar analysis is also conducted for any free-like (free, delete, etc.)
// call.
//
class AllocFreeAnalyzer {
  // A FunctionClosureTy container is used to hold a function closure, which is
  // a set of function *.
  typedef SmallPtrSet<Function *, 8> FunctionClosureTy;

  // A CloseMapperTy type maps a Function* to a SmallVector of CallBase*.
  // This is used to record the entry point(s) that a growing closure may reach.
  // E.g.
  // foo(){
  //  ...
  //  bar();  // (1)
  //  ...
  //  bar();  // (2)
  // ...
  //}
  // In the above example, ClosureMap[bar] = [bar (1), bar(2)]. They are the
  // 2 joins in function foo() that function bar() can grow into.
  typedef DenseMap<Function *, SmallVector<CallBase *, 4>> ClosureMapperTy;

  // A FunctionVisitTy type maps a Function* to a boolean.
  // It is used as a visit map, recording if a function has previously been
  // visited or processed.
  typedef DenseMap<Function *, bool> FunctionVisitTy;

public:
  Function *MainF = nullptr;
  AllocFreeAnalyzer(
      Module &M, Function *MainF,
      function_ref<TargetLibraryInfo &(Function &)> GetTLI,
      function_ref<DominatorTree &(Function &)> LookupDomTree,
      function_ref<PostDominatorTree &(Function &)> LookupPostDomTree)
      : MainF(MainF), M(M), GetTLI(GetTLI), LookupDomTree(LookupDomTree),
        LookupPostDomTree(LookupPostDomTree) {
    // Collect any malloc-like and/or free call.
    // If there is no malloc-like call or free call, there is no need for
    // further analysis.
    if (!collect())
      return;

    // Analyze any malloc-like call.
    // [Note]
    // if there is no malloc-like call, there is no need to analyze free.
    if (!analyzeMallocClosure())
      return;

    analyzeFreeClosure();
  }

  // Return true if 'BB' has a call to malloc(), calloc(), realloc(), new, etc.
  // If so, InstVec contains all such calls.
  bool hasMallocLikeCall(BasicBlock &BB,
                         SmallVectorImpl<CallBase *> &CallBaseVec);

  // Return true if 'BB' has at least a call to free().
  // If so, InstVec contains all such calls.
  bool hasFreeCall(BasicBlock &BB, SmallVectorImpl<CallBase *> &CallBaseVec);

  // Check if a given CallBase* is inside a provided Closure
  bool isInClosure(Function *F, FunctionClosureTy &Closure) {
    return Closure.find(F) != Closure.end();
  }

  bool InsertUsersIntoClosure(Function *F, FunctionClosureTy &Closure,
                              FunctionClosureTy &NewFunctions, bool DoRecurse,
                              ClosureMapperTy &Mapper);

  bool GrowAndTest(Function *F, FunctionClosureTy &FClosure,
                   FunctionClosureTy &TestClosure,
                   ClosureMapperTy &HostClosureMapper,
                   ClosureMapperTy &JointMapper, FunctionVisitTy &Visited);

  bool collect(void);
  bool analyzeMallocClosure(void);
  bool analyzeFreeClosure(void);
  bool analyzeForAlloc(Function *F);
  bool analyzeForFree(Function *F);

private:
  // malloc related:
  SmallVector<CallBase *, 4> AllocVec; // detected malloc calls
  FunctionClosureTy AllocClosure;      // malloc closure
  ClosureMapperTy AllocClosureMapper;

  // free related:
  SmallVector<CallBase *, 4> FreeVec; // detected free calls
  FunctionClosureTy FreeClosure;      // free closure
  ClosureMapperTy FreeClosureMapper;

  Module &M;
  function_ref<TargetLibraryInfo &(Function &)> GetTLI;
  function_ref<DominatorTree &(Function &)> LookupDomTree;
  function_ref<PostDominatorTree &(Function &)> LookupPostDomTree;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dumpVec(SmallVectorImpl<CallBase *> &Vec, StringRef Msg);
  void dumpFunctionClosure(FunctionClosureTy &Closure, StringRef Msg);
  void dumpClosureMapper(ClosureMapperTy &ClosureMapper, StringRef Msg);
#endif
};

// Helper class to handle the shared utilities for argument alignment
class IntelArgumentAlignmentUtils {
public:
  bool valueRefersToArg(Value *Val, Value *ArgArray);
}; // end IntelArgumentAlignmentUtils

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Data structure that tracks the progress of a task
class ProgressLog {
  StringRef PassName;
  bool PassPreliminaryAnalysis, PassCollection, PassAnalysis,
      PassTransformation;
  bool DetailMode;

public:
  enum Stage {
    PreliminaryAnalysis = 0,
    Collection,
    Analysis,
    Transform,
    Stage_Last,
  };

  ProgressLog()
      : PassName("IPO Prefetch "), PassPreliminaryAnalysis(false),
        PassCollection(false), PassAnalysis(false), PassTransformation(false),
        DetailMode(false) {}

  ProgressLog(StringRef PassName)
      : PassName(PassName), PassPreliminaryAnalysis(false),
        PassCollection(false), PassAnalysis(false), PassTransformation(false),
        DetailMode(false) {}

  void setPassName(const StringRef PName) { PassName = PName; }
  void setDetailMode(const bool Mode) { DetailMode = Mode; }

  void setStage(Stage TheStage) {
    switch (TheStage) {
    case PreliminaryAnalysis:
      PassPreliminaryAnalysis = true;
      break;
    case Collection:
      PassCollection = true;
      break;
    case Analysis:
      PassAnalysis = true;
      break;
    case Transform:
      PassTransformation = true;
      break;
    default:
      break;
    }
  }

  void print(raw_ostream &OS) const {
    if (DetailMode) {
      OS << PassName
         << " preliminary analysis: " << (PassPreliminaryAnalysis ? "" : "NOT")
         << " good\n";
      OS << PassName << " collection: " << (PassCollection ? "" : "NOT")
         << " good\n";
      OS << PassName << " analysis: " << (PassAnalysis ? "" : "NOT")
         << " good\n";
      OS << PassName << " transformation: " << (PassTransformation ? "" : "NOT")
         << " good\n";
    }

    bool Summary = PassCollection && PassAnalysis && PassTransformation;
    StringRef Msg = Summary ? " Triggered " : " NOT Triggered";
    OS << PassName << Msg << "\n";
  }

  void dump() const {
    formatted_raw_ostream FOS(dbgs());
    print(FOS);
  }
};
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

} // End namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_UTIILTY_H
