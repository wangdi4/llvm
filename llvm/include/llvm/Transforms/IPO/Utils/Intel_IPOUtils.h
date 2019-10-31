//===----  Intel_IPOUtils.h - IPO Utility Functions   --------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
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

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"

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
  template<typename T> static bool isInRange(T V, T LB, T UB) {
    assert((LB <= UB) && "Expect LB <= UB\n");
    return (V >= LB) && (V <= UB);
  }

  // Count the # of Integer Argument(s) in a given Function
  static unsigned countIntArgs(const Function &F);

  // Count the # of Pointer Argument(s) in a given Function
  static unsigned countPtrArgs(const Function &F);

  // Check if there is any floating-point argument
  static bool hasFloatArg(const Function &F);

  // Check if there is any floating-point pointer argument
  static bool hasFloatPtrArg(const Function &F);

  // Count the # of Double-Pointer Argument(s) in a given Function
  // E.g.
  // int foo(int **p, ...);
  // p is a called a double-pointer argument.
  static unsigned countDoublePtrArgs(const Function &F);

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
      : MinNumArgs(0), MaxNumArgs(0),
        MinNumIntArgs(0), MaxNumIntArgs(0),
        MinNumDoublePtrArgs(0), MaxNumDoublePtrArgs(0),
        IsLeaf(false) {}

  // [Note]
  // Argument NumPtrArgs0 and NumPtrArgs1 represent individual values instead of
  // a value range found on NumArgs, NumIntArgs, and NumDoublePtrArgs.
  FunctionSignatureMatcher(unsigned MinNumArgs, unsigned MaxNumArgs,
                           unsigned MinNumIntArgs, unsigned MaxNumIntArgs,
                           unsigned NumPtrArgs0, unsigned NumPtrArgs1,
                           unsigned MinNumDoublePtrArgs,
                           unsigned MaxNumDoublePtrArgs, bool IsLeaf)
      : MinNumArgs(MinNumArgs), MaxNumArgs(MaxNumArgs),
        MinNumIntArgs(MinNumIntArgs), MaxNumIntArgs(MaxNumIntArgs),
        MinNumDoublePtrArgs(MinNumDoublePtrArgs),
        MaxNumDoublePtrArgs(MaxNumDoublePtrArgs), IsLeaf(IsLeaf) {

    assert((MinNumArgs <= MaxNumArgs) && "Expect MinNumArgs <= MaxNumArgs");
    assert((MinNumIntArgs <= MaxNumIntArgs) &&
        "Expect MinNumIntArgs <= MaxNumIntArgs");
    assert((MinNumDoublePtrArgs <= MaxNumDoublePtrArgs) &&
        "Expect MinNumDoublePtrArgs <= MaxNumDoublePtrArgs");

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
  //                              1 /*MinNumDblPtRaRG */,1 /*MaxNumDblPtrArg */,
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
  // - NumDblPtrArg matches: foo has 1 dbl-ptr argument (q is ** type)
  // and
  // - LeafFunc matches: foo is a leaf function
  //
  // Note:
  // NumArg, NumIntArg, and NumDblPtrArg are provided in pairs because the same
  // FSM object will be used to match multiple function calls. So the match is
  // not necessarily exact.
  //
  // NumArgs and NumDblPtrArgs are used as bounds of a range, so search within
  // a range is enough. However, for NumPtrArgs, it needs to be 2 concrete
  // values instead of a range. As a result, the 2 individual values are pushed
  // into NumPtrArgV vector, and searched linearly using std::find() or
  // std::find_if().
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
    if (!IPOUtils::isInRange<unsigned>(IntArgCount,
                                       MinNumIntArgs,
                                       MaxNumIntArgs))
      return false;

    // Check PtrArgCount matches by NumPtrArgs0 or NumPtrArgs1
    unsigned PtrArgCount = IPOUtils::countPtrArgs(*F);
    if (std::find(NumPtrArgsV.begin(), NumPtrArgsV.end(), PtrArgCount)==
        NumPtrArgsV.end())
      return false;

    // Check MinNumDoublePtrArgs, MaxNumDoublePtrArgs: strict compare
    unsigned DblPtrArgCount = IPOUtils::countDoublePtrArgs(*F);
    if (!IPOUtils::isInRange<unsigned>(DblPtrArgCount, MinNumDoublePtrArgs,
                                       MaxNumDoublePtrArgs))
      return false;

    // Not allowing any Float and Float* argument type
    if (IPOUtils::hasFloatArg(*F) || IPOUtils::hasFloatPtrArg(*F))
      return false;

    return true;
  }

private:
  unsigned MinNumArgs, MaxNumArgs;
  unsigned MinNumIntArgs, MaxNumIntArgs;
  std::vector<unsigned> NumPtrArgsV;
  unsigned MinNumDoublePtrArgs, MaxNumDoublePtrArgs;
  bool IsLeaf;

public:
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string toString(void) const;
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
  // A FunctionClosureTy container is used to hold a function closure, which is a
  // set of function *.
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
  AllocFreeAnalyzer(Module &M,
                    Function *MainF,
                    function_ref<TargetLibraryInfo &(Function &)> GetTLI,
                    function_ref<DominatorTree &(Function &)> LookupDomTree,
                    function_ref<PostDominatorTree &(Function &)> LookupPostDomTree)
      : M(M), MainF(MainF), GetTLI(GetTLI), LookupDomTree(LookupDomTree),
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
    return Closure.find(F)!=Closure.end();
  }

  bool InsertUsersIntoClosure(Function *F,
                              FunctionClosureTy &Closure,
                              FunctionClosureTy &NewFunctions,
                              bool DoRecurse, ClosureMapperTy &Mapper);

  bool GrowAndTest(Function *F,
                   FunctionClosureTy &FClosure,
                   FunctionClosureTy &TestClosure,
                   ClosureMapperTy &HostClosureMapper,
                   ClosureMapperTy &JointMapper,
                   FunctionVisitTy &Visited);

  bool collect(void);
  bool analyzeMallocClosure(void);
  bool analyzeFreeClosure(void);
  bool analyzeForAlloc(Function *F);
  bool analyzeForFree(Function *F);

private:
  // malloc related:
  SmallVector<CallBase *, 4> AllocVec; // detected malloc calls
  FunctionClosureTy AllocClosure; // malloc closure
  ClosureMapperTy AllocClosureMapper;

  // free related:
  SmallVector<CallBase *, 4> FreeVec;  // detected free calls
  FunctionClosureTy FreeClosure;  // free closure
  ClosureMapperTy FreeClosureMapper;

  Module &M;
  Function *MainF = nullptr;
  function_ref<TargetLibraryInfo &(Function &)> GetTLI;
  function_ref<DominatorTree &(Function &)> LookupDomTree;
  function_ref<PostDominatorTree &(Function &)> LookupPostDomTree;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dumpVec(SmallVectorImpl<CallBase *> &Vec, StringRef Msg);
  void dumpFunctionClosure(FunctionClosureTy &Closure, StringRef Msg);
  void dumpClosureMapper(ClosureMapperTy &ClosureMapper, StringRef Msg);
#endif

};

} // End namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_UTIILTY_H
