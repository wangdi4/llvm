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

#include "llvm/Transforms/IPO/Utils/Intel_IPOUtils.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"

#include <sstream>

using namespace llvm;

#define DEBUG_TYPE "ipoutils"

enum AllocKind : uint8_t {
  AK_NotAlloc,
  AK_Malloc,
  AK_Calloc,
  AK_Realloc,
  AK_UserMalloc,
  AK_UserMalloc0,
  AK_New
};

// Check: is the given Function a leaf function?
// Intrinsics are allowed.
bool IPOUtils::isLeafFunction(const Function &F) {
  for (const auto &I : instructions(&F)) {
    if (isa<InvokeInst>(&I))
      return false;

    if (auto CI = dyn_cast<CallInst>(&I)) {
      Function *Callee = CI->getCalledFunction();
      if (!Callee || !Callee->isIntrinsic())
        return false;
    }
  }
  return true;
}

// Count the # of Integer Argument(s) in a given Function
unsigned IPOUtils::countIntArgs(const Function &F) {
  unsigned NumIntArgs = 0;
  for (auto &Arg : F.args())
    if (Arg.getType()->isIntegerTy())
      ++NumIntArgs;
  return NumIntArgs;
}

// Count the # of Pointer Argument(s) i n a given Function
unsigned IPOUtils::countPtrArgs(const Function &F) {
  unsigned NumPtrArgs = 0;
  for (auto &Arg : F.args())
    if (Arg.getType()->isPointerTy())
      ++NumPtrArgs;
  return NumPtrArgs;
}

// Check if there is any floating-point argument
bool IPOUtils::hasFloatArg(const Function &F) {
  for (auto &Arg : F.args())
    if (Arg.getType()->isFloatingPointTy())
      return true;
  return false;
}

// Check if there is any floating-point pointer argument
bool IPOUtils::hasFloatPtrArg(const Function &F) {
  for (auto &Arg : F.args())
    if (Arg.getType()->isPointerTy()) {
      auto PointeeTy =
          dyn_cast<PointerType>(Arg.getType())->getPointerElementType();
      if (!PointeeTy)
        continue;
      if (PointeeTy->isFloatingPointTy())
        return true;
    }
  return false;
}

// Count the # of Double-Pointer Argument(s) in a given Function
// E.g.
// int foo(int **p, ...);
// p is a called a double-pointer argument.
unsigned IPOUtils::countDoublePtrArgs(const Function &F) {
  unsigned NumDoublePtrArgs = 0;
  for (auto &Arg : F.args())
    if (Arg.getType()->isPointerTy()) {
      auto PointeeTy =
          dyn_cast<PointerType>(Arg.getType())->getPointerElementType();
      if (!PointeeTy)
        continue;
      if (PointeeTy->isPointerTy())
        ++NumDoublePtrArgs;
    }
  return NumDoublePtrArgs;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
std::string FunctionSignatureMatcher::toString(void) const {
  std::ostringstream S;
  S << "FunctionSignature Object: \n";

  // unsigned MinNumArgs, MaxNumArgs:
  S << "MinNumArgs: " << MinNumArgs << ", MaxNumArgs,: " << MaxNumArgs << "\n";

  // unsigned MinNumIntArgs, MaxNumIntArgs:
  S << "MinNumIntArgs: " << MinNumIntArgs << ", MaxNumIntArgs,: "
    << MaxNumIntArgs << "\n";

  // std::vector<unsigned> NumPtrArgsV:
  S << "NumPtrArgs: ";
  for (auto V : NumPtrArgsV)
    S << V << ", ";
  S << "\n";

  // unsigned MinNumDoublePtrArgs, MaxNumDoublePtrArgs;
  S << "MinNumDoublePtrArgs: " << MinNumDoublePtrArgs
    << ", MaxNumDoublePtrArgs,: " << MaxNumDoublePtrArgs << "\n";

  // IsLeaf:
  S << "IsLeaf: " << std::boolalpha << IsLeaf << "\n";

  return S.str();
}
#endif //!defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

static Function *getCalledFunction(const CallBase &Call) {
  Value *CalledValue = Call.getCalledOperand()->stripPointerCasts();
  if (auto *CalledF = dyn_cast<Function>(CalledValue))
    return CalledF;

  if (auto *GA = dyn_cast<GlobalAlias>(CalledValue))
    if (!GA->isInterposable())
      if (auto *AliasF =
          dyn_cast<Function>(GA->getAliasee()->stripPointerCasts()))
        return AliasF;

  return nullptr;
}

static AllocKind getAllocFnKind(const CallBase *Call,
                                const TargetLibraryInfo &TLI) {
  if (isNewLikeFn(Call, &TLI))
    return AK_New;
  if (isMallocLikeFn(Call, &TLI))
    return Call->arg_size()==1 ? AK_Malloc : AK_New;
  if (isCallocLikeFn(Call, &TLI))
    return AK_Calloc;
  if (isReallocLikeFn(Call, &TLI))
    return AK_Realloc;
  return AK_NotAlloc;
}

// Beginning of member functions for class AllocFreeAnalyzer

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void AllocFreeAnalyzer::dumpVec(SmallVectorImpl<CallBase *> &Vec,
                                StringRef Msg) {
  bool IsEmpty = Vec.empty();
  if (!IsEmpty) {
    dbgs() << Msg << " has " << Vec.size() << " item(s):\n";
    unsigned Count = 0;
    for (auto I: Vec)
      dbgs() << Count++ << "  " << *I << "\n";
  } else
    LLVM_DEBUG(dbgs() << Msg << " is empty\n";);
}

void AllocFreeAnalyzer::dumpFunctionClosure(FunctionClosureTy &Closure,
                                            StringRef Msg) {
  bool IsEmpty = Closure.empty();
  if (!IsEmpty) {
    dbgs() << Msg << " has " << Closure.size() << " item(s):\n";
    unsigned Count = 0;
    for (auto *F: Closure)
      dbgs() << Count++ << "  " << F->getName() << "(), \n";
  } else
    LLVM_DEBUG(dbgs() << Msg << " is empty\n";);
}

void AllocFreeAnalyzer::dumpClosureMapper(ClosureMapperTy &ClosureMapper,
                                          StringRef Msg) {
  if (!Msg.empty())
    dbgs() << Msg << "\t" << ClosureMapper.size() << "\n";
  for (auto &Pair: ClosureMapper) {
    Function *F = Pair.first;
    auto &Vec = Pair.second;
    dbgs() << F->getName() << "() :\n";
    unsigned Count = 0;
    for (auto *CB: Vec)
      dbgs() << "  " << Count++ << "  " << *CB << "\n";
  }
}

#endif

bool AllocFreeAnalyzer::hasFreeCall(BasicBlock &BB,
                                    SmallVectorImpl<CallBase *> &InstVec) {
  bool Result = false;
  for (auto BI = BB.rbegin(), BE = BB.rend(); BI!=BE;) {
    Instruction *I = &*BI++;
    if (auto *Call = dyn_cast<CallBase>(I)) {
      const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
      if (isFreeCall(Call, &TLI, false)) {
        Result = true;
        InstVec.push_back(Call); // save the CallBase*
      }
    }
  }
  return Result;
}

// Return true if 'BB' has a call to malloc(), calloc(), realloc(), new(), etc.
// If so, InstVec contains all such calls.
bool AllocFreeAnalyzer::hasMallocLikeCall(BasicBlock &BB,
                                          SmallVectorImpl<CallBase *> &InstVec) {
  bool Result = false;
  for (auto BI = BB.rbegin(), BE = BB.rend(); BI!=BE;) {
    Instruction *I = &*BI++;
    auto *Call = dyn_cast<CallBase>(I);
    if (!Call || !Call->getCalledFunction()) // skip any function pointer
      continue;

    const TargetLibraryInfo &TLI = GetTLI(*Call->getFunction());
    auto Kind = getAllocFnKind(Call, TLI);
    if (Kind==AK_Malloc || Kind==AK_New || Kind==AK_Calloc ||
        Kind==AK_Realloc) {
      Result = true;
      InstVec.push_back(Call); // save the CallBase*
    }
  }
  return Result;
}

// Collect all uses of 'malloc-like' and 'free' calls across all functions.
// A malloc-like call may include: malloc, calloc, realloc, new, etc.
// A free-like call may include: free, delete, etc.
bool AllocFreeAnalyzer::collect(void) {

  auto FindCallsites = [&](Function &F, bool IsMallocLike,
                           SmallVectorImpl<CallBase *> &ResultVec) -> bool {
    bool HasRelevantCall = false;
    const TargetLibraryInfo &TLI = GetTLI(F);

    if (IsMallocLike)
      //check malloc-like call: malloc, calloc, realloc, new
      HasRelevantCall = isMallocLikeFn(&F, &TLI) || isCallocLikeFn(&F, &TLI) ||
          isReallocLikeFn(&F, &TLI) || isNewLikeFn(&F, &TLI);
    else
      //check free-like call: free, delete, etc.
      HasRelevantCall = isFreeFn(&F, &TLI) || isDeleteFn(&F, &TLI);

    if (!HasRelevantCall)
      return false;

    // Find all uses of this function, and save each valid call site
    for (User *U : F.users())
      if (CallBase *CB = dyn_cast<CallBase>(U)) {
        LLVM_DEBUG(
            dbgs() << "a valid call to a requested function: " << *CB
                   << "\n";);
        ResultVec.push_back(CB);
      }

    return !ResultVec.empty();
  };

  for (auto &F : M.functions()) {
    // Skip any function with implementation
    if (!F.isDeclaration())
      continue;

    // Collect any malloc-like call:
    FindCallsites(F, true /* malloc-like */, AllocVec);

    // Collect any free call:
    FindCallsites(F, false /* free-like */, FreeVec);
  }

  // check the collection results:
  LLVM_DEBUG(dumpVec(AllocVec, "AllocVec:"););
  LLVM_DEBUG(dumpVec(FreeVec, "FreeVec:"););

  // only worth analyze if malloc-like collection is NOT empty!
  return !AllocVec.empty();
}

//
// Insert all Function * F's users into a given Closure.
// If DoRecurse is true, this algorithm will continue recursion until no new
// function is found.
//
// Return: bool
// -true: if the Closure changes;
// -false:otherwise
//
bool AllocFreeAnalyzer::InsertUsersIntoClosure(Function *F,
                                               FunctionClosureTy &Closure,
                                               FunctionClosureTy &NewFunctions,
                                               bool Recurse,
                                               ClosureMapperTy &Mapper) {
  bool Result = false;

  // act on current level:
  for (User *U : F->users()) {
    CallBase *Caller = dyn_cast<CallBase>(U);
    if (!Caller || Caller->isIndirectCall())
      continue;
    Function *HostF = Caller->getFunction();
    LLVM_DEBUG(dbgs() << "Caller: " << *Caller << "\n" << "Host: "
                      << HostF->getName() << "()\n";);
    // record into the mapper
    Mapper[HostF].push_back(Caller);

    if (!isInClosure(HostF, Closure)) {
      Closure.insert(HostF);
      NewFunctions.insert(HostF);
      Result = true;
    }
  }

  LLVM_DEBUG({
               dumpFunctionClosure(Closure, "Closure");
               dumpFunctionClosure(NewFunctions, "NewFunctions");
               dumpClosureMapper(Mapper, "ClosureMapper");
             });

  // Recurse: build closures on any newly discovered function(s)
  if (Result && Recurse)
    for (auto *F: NewFunctions) {
      FunctionClosureTy TempFuncs;
      InsertUsersIntoClosure(F, Closure, TempFuncs, Recurse, Mapper);
    }

  LLVM_DEBUG({
               dumpFunctionClosure(Closure, "Closure");
               dumpFunctionClosure(NewFunctions, "NewFunctions");
               dumpClosureMapper(Mapper, "ClosureMapper");
             });

  return Result;
}

// A closure is the max set of functions that a callsite can grow into,
// following only callee-caller relationship.
//
// In the example below, malloc's closure is {g, f, b, main, d, c,}, and
// prefetch's closure is {q, p, o, main, r, s}.
//
// A prefetch host (a.k.a host) is a function in which a call to prefetch will
// be inserted. E.g. in the partial call graph below, q is a host.
//
// The GrowAndTest function attempts to grow function F's closure and puts
// incremental results into FClosure. When a new caller is discovered, it is
// tested against the given TestClosure. Each match is recorded.
//
// When FClosure's growth is complete (can't find any new function to add to the
// FClosure), the recursion stops. The algorithm checks for intersection, and
// returns true if there is at least 1 intersection.
//
// E.g.
// Refer to IPO Prefetch Design, Slide #13: malloc dominates prefetch function.
//
//        main
//    /   |    \
//  b     c     o  -   s
//  |    /      |  \  /
//  f   d       p   r
//   \  /       \  /
//   g           q
//   |            \
//  malloc      prefetch
//
// In the above call graph, each node is a function, and a vertical or
// horizontal bar indicates a caller-callee relationship.
//
// E.g. main calls b: main is the caller, b is the callee.
// E.g. o calls s : o is the caller, s is the callee.
//
// In this case, TestClosure is already established as the closure for malloc:
// {g, f, d, c, b, main}.
//
// Function q is the host function thus FClosure begins with {q}.
// FClosure grows as new functions are discovered and inserted into it.
// Each time a new function appears, it is tested against TestClosure for a
// potential intersection, and is recorded if the intersection is not null.
//
// In this example, FClosure grows all the way up to {q, p, r, o, s, main} and
// only intersects with TestClosure at main.
//
// When FClosure stops growing, the recursive algorithm terminates. If there is
// any non-empty intersection, the algorithm will return true. All intersections
// are recorded in HostClosureMapper and JointMapper for testing dominance later.
//
// As the above example shows, TestClosure and FClosure intersects only at main
// function, thus the algorithm returns true.
//
bool AllocFreeAnalyzer::GrowAndTest(Function *F,
                                    FunctionClosureTy &FClosure,
                                    FunctionClosureTy &TestClosure,
                                    ClosureMapperTy &HostClosureMapper,
                                    ClosureMapperTy &JointMapper,
                                    FunctionVisitTy &Visited) {
  bool Result = false;
  FunctionClosureTy NewFunctions;

  // act on current level:
  for (User *U : F->users()) {
    CallBase *Caller = dyn_cast<CallBase>(U);
    if (!Caller || Caller->isIndirectCall())
      continue;
    Function *HostF = Caller->getFunction();
    // save into the ClosureMapper
    HostClosureMapper[HostF].push_back(Caller);

    LLVM_DEBUG(dbgs() << "Host: " << HostF->getName() << "():\t"
                      << "Caller: " << *Caller << "\n";);

    if (isInClosure(HostF, TestClosure)) {
      // Found the 1st joint, return
      JointMapper[HostF].push_back(Caller);
      Result = true;
    } else {
      //Save newly discovered HostF into both FClosure and NewFunctions
      FClosure.insert(HostF);
      NewFunctions.insert(HostF);
    }
  }

  LLVM_DEBUG({
               dumpFunctionClosure(TestClosure, "TestClosure -- ");
               dumpFunctionClosure(FClosure, "FClosure -- ");
               dumpFunctionClosure(NewFunctions, "NewFunctions --");
             });

  // Recurse: recursively add into FClosures on any newly discovered function(s)
  // Note: use depth-first recursion with a visit map.
  for (auto *NF: NewFunctions)
    if (!Visited[NF]) {
      Visited[NF] = true;
      Result |= GrowAndTest(NF, FClosure, TestClosure, HostClosureMapper,
                            JointMapper, Visited);
    }

  LLVM_DEBUG({
               dumpFunctionClosure(TestClosure, "TestClosure -- ");
               dumpFunctionClosure(FClosure, "FClosure -- ");
               dumpFunctionClosure(NewFunctions, "NewFunctions --");
             });

  return Result;
}

// Analyze the dominance relationship between malloc and prefetch-host function.
//
//- Build malloc closure: if there is at least 1 malloc-like callsite
// . start from each malloc-site, find its function, and find all places this
//   function is called.
// . grow into a closure, until it reaches main(), or won't grow any further.
//
//- Build prefetch closure: if there is at least 1 prefetch host function
// 1. start from each prefetch-host function, find all its callers.
// 2. find intersection point:
//  for each caller, check if this caller is in the malloc chain;
//  if yes: we find the lowest joint point;
//  otherwise, continue step 2.
//  In the worst cast, the worst join point is main().
//  hopefully, the algorithm can find a better intersection point.
//
// 3. Test dominance relationship in the intersection function:
// E.g.
//  call1: get the callsite leading to malloc
//  call2: get the callsite leading to prefetch host
//  verify: call1 DOMINATES call2
//
// 4. recurse step 1, until it won't grow any further, or reach main().
//
bool AllocFreeAnalyzer::analyzeMallocClosure(void) {

  // Build malloc closure: if there is at least 1 malloc-like callsite
  if (AllocVec.empty())
    return false;

  for (auto *CB: AllocVec) {
    Function *F = CB->getFunction();
    if (!F)
      continue;

    // build Closure for the current level
    FunctionClosureTy NewFunctions;
    InsertUsersIntoClosure(F,
                           AllocClosure,
                           NewFunctions,
                           true,
                           AllocClosureMapper);
    // show the current closure:
    LLVM_DEBUG(dumpFunctionClosure(AllocClosure, "AllocClosure"));
  }

  // examine contents of AllocClosure
  LLVM_DEBUG(dumpFunctionClosure(AllocClosure, "AllocClosure"););

  return !AllocClosure.empty();
}

// Analyze the dominance relationship between prefetch-host function and free.
// The algorithm is very similar to analyzeMallocClosure().
bool AllocFreeAnalyzer::analyzeFreeClosure(void) {
  // Build free chain: if there is at least 1 free callsite
  if (FreeVec.empty())
    return false;

  for (auto *CB: FreeVec) {
    Function *F = CB->getFunction();
    if (!F)
      continue;

    // build Closure for the current level
    FunctionClosureTy NewFunctions;
    InsertUsersIntoClosure(F,
                           FreeClosure,
                           NewFunctions,
                           true,
                           FreeClosureMapper);
    // show the current closure:
    LLVM_DEBUG(dumpFunctionClosure(FreeClosure, "FreeClosure"));
  }

  // examine contents of FreeClosure
  LLVM_DEBUG(dumpFunctionClosure(FreeClosure, "FreeClosure"););

  return !FreeClosure.empty();
}

bool AllocFreeAnalyzer::analyzeForAlloc(Function *F) {
  // Find lowest join between malloc closure and prefetch closure:
  FunctionClosureTy HostClosure;
  ClosureMapperTy HostClosureMapper;
  ClosureMapperTy JointMapper;
  FunctionVisitTy Visited;
  bool FindJointWithMalloc =
      GrowAndTest(F, HostClosure, AllocClosure, HostClosureMapper,
                  JointMapper, Visited);
  LLVM_DEBUG({
               dbgs() << "FindJoinWithMalloc: " << FindJointWithMalloc << "\n";
               dumpFunctionClosure(AllocClosure, "AllocClosure--");
               dumpFunctionClosure(HostClosure, "HostClosure--");
               dumpClosureMapper(HostClosureMapper, "HostClosureMapper--");
               dumpClosureMapper(JointMapper, "JointMapper--");
               dumpClosureMapper(AllocClosureMapper, "AllocClosureMapper--");
               dumpClosureMapper(FreeClosureMapper, "FreeClosureMapper--");
             });

  // sanity check:
  if (FindJointWithMalloc)
    assert(!HostClosure.empty() && "non empty FClosure");
  else
    assert(HostClosure.empty() && "empty FClosure");

  if (!FindJointWithMalloc)
    return false;
  if (JointMapper.empty())
    return false;

  // check: proper dominance relationship
  // each alloc joint dominates every host joint
  for (auto &Pair: JointMapper) {
    Function *F = Pair.first;
    auto &DT = LookupDomTree(*F);

    auto &AllocVec = AllocClosureMapper[F];
    auto &HostJoinVec = Pair.second;

    for (auto *AllocJP: AllocVec)
      for (auto *HostJP: HostJoinVec) {
        // ensure AllocJP dominates HostJP for each possible case
        bool DomRelationship = DT.dominates(AllocJP, HostJP);
        LLVM_DEBUG({
                     dbgs() << "AllocJP: " << AllocJP << "\n"
                            << "HostJP: " << HostJP << "\n"
                            << "Dominates: " << DomRelationship << "\n";
                   });
        if (!DomRelationship) {
          LLVM_DEBUG(dbgs() << "AllocJP NOT dominate HostJP\n!!!";);
          return false;
        }
      }
  }

  return true;
}

bool AllocFreeAnalyzer::analyzeForFree(Function *F) {
  // ok if free is never called: code is still correct, just bad programming.
  if (FreeClosure.empty())
    return true;

  // Find lowest join: between prefetch host and free.
  FunctionClosureTy HostClosure;
  ClosureMapperTy HostClosureMapper;
  ClosureMapperTy JointMapper;
  FunctionVisitTy Visited;
  bool FindJoinWithFree =
      GrowAndTest(F, HostClosure, FreeClosure, HostClosureMapper,
                  JointMapper, Visited);
  LLVM_DEBUG({
               dbgs() << "FindJoinWithFree: " << FindJoinWithFree << "\n";
               dumpFunctionClosure(FreeClosure, "FreeClosure--");
               dumpFunctionClosure(HostClosure, "HostClosure--");
               dumpClosureMapper(HostClosureMapper, "HostClosureMapper--");
               dumpClosureMapper(JointMapper, "JointMapper--");
               dumpClosureMapper(AllocClosureMapper, "AllocClosureMapper--");
               dumpClosureMapper(FreeClosureMapper, "FreeClosureMapper--");
             });

  // sanity check:
  if (FindJoinWithFree)
    assert(!HostClosure.empty() && "non empty FClosure");
  else
    assert(HostClosure.empty() && "empty FClosure");

  if (!FindJoinWithFree)
    return false;
  if (JointMapper.empty())
    return false;

  // check: proper dominance relationship
  // each free joint post dominates every host joint
  for (auto &Pair: JointMapper) {
    Function *F = Pair.first;
    PostDominatorTree &PDT = LookupPostDomTree(*F);

    auto &FreeVec = FreeClosureMapper[F];
    auto &HostJoinVec = Pair.second;

    for (auto *FreeJP: FreeVec)
      for (auto *HostJP: HostJoinVec) {
        // ensure FreeJP post dominates HostJP for each possible case
        bool
            IsPostDom = PDT.dominates(FreeJP->getParent(), HostJP->getParent());
        LLVM_DEBUG({
                     dbgs() << "FreeJP: " << FreeJP << "\n"
                            << "HostJP: " << HostJP << "\n"
                            << "Post Dominates: " << IsPostDom << "\n";
                   });
        if (!IsPostDom) {
          LLVM_DEBUG(
              dbgs() << "Error: FreeJP NOT post dominate HostJP\n!!!";);
          return false;
        }
      }
  }

  return true;
}

// End of member functions for class AllocFreeAnalyzer
