//===----  Intel_IPOPrefetch.cpp - Intel IPO Prefetch   --------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass conducts IPO-based (not loop-based) prefetching.
//
// This includes prefetching locations that have high-density Delinquent Loads
// (DL). These DLs are either not located inside a loop, or the DLs are
// indeed located inside a loop, but the said loop is in a poor shape that
// makes loop-based prefetching impossible.
//
//===----------------------------------------------------------------------===//
// *** An simplified example of IPO Prefetch ***
//
// The function in which a DL resides:
//
// ProbeTT(..){ // big function
//   ...
//   for (i = 0; i < 4; i++) { // big loop, multiple side exits, low trip count
//     if (temp->buckets[i].hash == .) {
//         ^^^^ DL ^^^^^
//     ..
//     }
//     ...
//     if(C0) return 1;
//     ...
//     if(C1) return 2;
//     ...
//     big loop body with multiple side exits
//   }
// ...
//   return 0;
// }
//
// A prefetch function computes DL address(es), and generates 1 or 2 prefetch
// instruction(s) on the DL address(es)
//
//define internal void @Prefetch.Backbone(%struct.state_t* nocapture %0) {
// ...
// ... compute address of DL[0], result in %18 ...
// call void @llvm.prefetch.p0i8(i8* %18, i32 0, i32 3, i32 1) ; 1st prefetch
//
// .. compute the need and address of DL[3], result in %20 ...
// call void @llvm.prefetch.p0i8(i8* %20, i32 0, i32 3, i32 1); 2nd prefetch
//
// ret void
//}
//
// A host function with a newly inserted call to prefetch function:
// qsearch(node * N){ // big and recursive function
//   ...
//   Prefetch.Backbone(N);
//   ...
// }
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_IPOPrefetch.h"
#include "llvm/Transforms/IPO/Utils/Intel_IPOUtils.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Intel_WP_utils.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/DeadArgumentElimination.h"
#include "llvm/Transforms/IPO/Intel_IPOPrefetch.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/EarlyCSE.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils.h"

#include <cstdarg>

using namespace llvm;

#define PASS_NAME_STR "ipo-prefetch"
#define DEBUG_TYPE "ipoprefetch"

STATISTIC(NumIPOPrefetch, "Number of IPO Prefetches");

// Option to enable IPO Prefetch pass, default to true.
static cl::opt<bool> EnableIPOPrefetch(
    PASS_NAME_STR "-enable", cl::init(true), cl::ReallyHidden,
    cl::desc("enable ipo prefetching"));

// Option to enable code generation of the 2nd prefetch instruction.
// Experiments show that allowing to generate the 2nd prefetch instruction can
// further improve performance. As a result, default is set to true (1).
static cl::opt<bool> Generate2ndPrefetchInst(
    PASS_NAME_STR "-gen-2ndprefetchinst", cl::init(true),
    cl::ReallyHidden,
    cl::desc("Generate the 2nd prefetch instruction"
    ));

// Min Expected # of arguments in a function that a Delinquent Load (DL) may
// reside
static cl::opt<unsigned> DLFuncMinExpectedArgs(
    PASS_NAME_STR "-dlf-min-expected-args", cl::init(10),
    cl::ReallyHidden,
    cl::desc("Min Expected # of arguments in a function that a Delinquent Load "
             "(DL) may reside"));

// Max Expected # of arguments in a function that a Delinquent Load (DL) may
// reside
static cl::opt<unsigned> DLFuncMaxExpectedArgs(
    PASS_NAME_STR "-dlf-max-expected-args", cl::init(10),
    cl::ReallyHidden,
    cl::desc("Max Expected # of arguments in a function that a Delinquent Load "
             "(DL) may reside"));

// Min Expected # of integer arguments in a function that a Delinquent Load
// (DL) may reside
static cl::opt<unsigned> DLFuncMinIntArgs(
    PASS_NAME_STR "-dlf-min-int-args", cl::init(3),
    cl::ReallyHidden,
    cl::desc("Min Expected # of integer arguments in a function that a "
             "Delinquent Load (DL) may reside"));

// Max Expected # of integer arguments in a function that a Delinquent Load
// (DL) may reside
static cl::opt<unsigned> DLFuncMaxIntArgs(
    PASS_NAME_STR "-dlf-max-int-args", cl::init(3),
    cl::ReallyHidden,
    cl::desc("Max Expected # of integer arguments in a function that a "
             "Delinquent Load (DL) may reside"));

// Min Expected # of integer ptr arguments in a function that a Delinquent Load
// (DL) may reside
static cl::opt<unsigned> DLFuncMinIntPtrArgs(
    PASS_NAME_STR "-dlf-min-intptr-args", cl::init(7),
    cl::ReallyHidden,
    cl::desc("Min Expected # of integer pointer arguments in a function that "
             "a Delinquent Load (DL) may reside"));

// Max Expected # of integer ptr arguments in a function that a Delinquent Load
// (DL) may reside
static cl::opt<unsigned> DLFuncMaxIntPtrArgs(
    PASS_NAME_STR "-dlf-max-intptr-args", cl::init(7),
    cl::ReallyHidden,
    cl::desc("Max Expected # of integer pointer arguments in a function that "
             "a Delinquent Load (DL) may reside"));

// Min Expected # of double integer ptr (int**) arguments in a function that
// a Delinquent Load (DL) may reside
static cl::opt<unsigned> DLFuncMinDltIntPtrArgs(
    PASS_NAME_STR "-dlf-min-dblintptr-args", cl::init(0),
    cl::ReallyHidden,
    cl::desc("Min Expected # of double integer pointer arguments in a function "
             "that a Delinquent Load (DL) may reside"));

// Max Expected # of double integer ptr (int**) arguments in a function that
// a Delinquent Load (DL) may reside
static cl::opt<unsigned> DLFuncMaxDltIntPtrArgs(
    PASS_NAME_STR "-dlf-max-dblintptr-args", cl::init(0),
    cl::ReallyHidden,
    cl::desc("Max Expected # of double integer pointer arguments in a function "
             "that a Delinquent Load (DL) may reside"));

// Number of expected function(s) that has delinquent load(s)
static cl::opt<unsigned> ExpectedNumDLFunctions(
    PASS_NAME_STR "-expected-num-dl-funcs", cl::init(1),
    cl::ReallyHidden,
    cl::desc("Expected Number of DL Functions "));

// Number of expected positions where a call to prefetch function will be
// inserted
static cl::opt<unsigned> ExpectedNumPrefetchInsertPositions(
    PASS_NAME_STR "-expected-num-prefetch-insert-positions", cl::init(2),
    cl::ReallyHidden,
    cl::desc("Expected Number of Prefetch Insert Positions"));

// Max Expected # of arguments in a function that will host prefetch function
static cl::opt<unsigned> DL0HostFuncMaxArgs(
    PASS_NAME_STR "-dl0-host-max-args", cl::init(5),
    cl::ReallyHidden,
    cl::desc("Max Expected # of max arguments in a DL host0 function"));

// Min Expected # of arguments in a function that will host prefetch function
static cl::opt<unsigned> DL0HostFuncMinArgs(
    PASS_NAME_STR "-dl0-host-min-args", cl::init(5),
    cl::ReallyHidden,
    cl::desc("Min Expected # of max arguments in a DL host0 function"));

// Max Expected # of integer arguments in a function that will host prefetch function
static cl::opt<unsigned> DL0HostFuncMaxIntArgs(
    PASS_NAME_STR "-dl0-host-max-int-args", cl::init(4),
    cl::ReallyHidden,
    cl::desc("Max Expected # of max integer arguments in a DL host0 function"));

// Min Expected # of integer arguments in a function that will host prefetch function
static cl::opt<unsigned> DL0HostFuncMinIntArgs(
    PASS_NAME_STR "-dl0-host-min-int-args", cl::init(4),
    cl::ReallyHidden,
    cl::desc("Min Expected # of max integer arguments in a DL host0 function"));

// Max Expected # of int* arguments in a function that will host prefetch function
static cl::opt<unsigned> DL0HostFuncMaxIntPtrArgs(
    PASS_NAME_STR "-dl0-host-max-intptr-args", cl::init(1),
    cl::ReallyHidden,
    cl::desc("Max Expected # of max int* arguments in a DL host0 function"));

// Min Expected # of int* arguments in a function that will host prefetch function
static cl::opt<unsigned> DL0HostFuncMinIntPtrArgs(
    PASS_NAME_STR "-dl0-host-min-intptr-args", cl::init(1),
    cl::ReallyHidden,
    cl::desc("Min Expected # of max int* arguments in a DL host0 function"));

// Max Expected # of arguments in a function that will host prefetch function
static cl::opt<unsigned> DL1HostFuncMaxArgs(
    PASS_NAME_STR "-dl1-host-max-args", cl::init(6),
    cl::ReallyHidden,
    cl::desc("Max Expected # of max arguments in a DL host1 function"));

// Min Expected # of arguments in a function that will host prefetch function
static cl::opt<unsigned> DL1HostFuncMinArgs(
    PASS_NAME_STR "-dl1-host-min-args", cl::init(6),
    cl::ReallyHidden,
    cl::desc("Min Expected # of max arguments in a DL host1 function"));

// Max Expected # of integer arguments in a function that will host prefetch function
static cl::opt<unsigned> DL1HostFuncMaxIntArgs(
    PASS_NAME_STR "-dl1-host-max-int-args", cl::init(5),
    cl::ReallyHidden,
    cl::desc("Max Expected # of max integer arguments in a DL host1 function"));

// Min Expected # of integer arguments in a function that will host prefetch function
static cl::opt<unsigned> DL1HostFuncMinIntArgs(
    PASS_NAME_STR "-dl1-host-min-int-args", cl::init(5),
    cl::ReallyHidden,
    cl::desc("Min Expected # of max integer arguments in a DL host1 function"));

// Max Expected # of int* arguments in a function that will host prefetch function
static cl::opt<unsigned> DL1HostFuncMaxIntPtrArgs(
    PASS_NAME_STR "-dl1-host-max-intptr-args", cl::init(1),
    cl::ReallyHidden,
    cl::desc("Max Expected # of max int* arguments in a DL host1 function"));

// Min Expected # of int* arguments in a function that will host prefetch function
static cl::opt<unsigned> DL1HostFuncMinIntPtrArgs(
    PASS_NAME_STR "-dl1-host-min-intptr-args", cl::init(1),
    cl::ReallyHidden,
    cl::desc("Min Expected # of max int* arguments in a DL host1 function"));

// Flag to do fine-tune control with LIT test
static cl::opt<bool> BeLITFriendly(
    PASS_NAME_STR "-be-lit-friendly", cl::init(false),
    cl::ReallyHidden,
    cl::desc("Be LIT Friendly"));

// Number of items in an array for applicable test once 2 expected struct types
// are identified. This value will be used to compute the stride distance leading
// to generating the 2nd prefetch intrinsic.
static cl::opt<unsigned> AppTestArraySize(
    PASS_NAME_STR "-AppTestArraySize", cl::init(4),
    cl::ReallyHidden,
    cl::desc("App Test Array Size"));

namespace {

static const StringRef PrefetchFunctionName = "Prefetch.Backbone";

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void dumpInstVec(SmallVectorImpl<Instruction *> &InstV, StringRef Msg) {
  dbgs() << Msg << ": <" << InstV.size() << ">:\n";

  unsigned Count = 0;
  for (auto I: InstV) {
    LLVM_DEBUG(dbgs() << Count++ << "\t" << *I << "\n";);
    (void) I;
  }
  (void) Count;
}

static bool verifyModule(Module &M, StringRef Msg) {
  bool BrokenDebugInfo = false;
  if (verifyModule(M, &llvm::errs(), &BrokenDebugInfo))
    report_fatal_error(Msg + M.getName() + "()\n");
  return true;
}

static bool verifyFunction(Function *F, StringRef Msg) {
  if (verifyFunction(*F))
    report_fatal_error(Msg + F->getName() + "()\n");
  return true;
}
#endif

// This class describes an LLVM Instruction's position inside an LLVM Function.
// This helps to precisely identify a position inside an LLVM function.
class PositionDescription {
  Function *F = nullptr;
  unsigned NumLoad = 0;
  unsigned NumStore = 0;
  unsigned NumBranch = 0;
  unsigned NumCmp = 0;
  unsigned NumCall = 0;
  unsigned NumIntrinsic = 0;
  Instruction *PreciseInst = nullptr;   // the located inst
  Instruction *InsertPosition = nullptr;// PreciseInst's next inst

public:
  PositionDescription(Function *F = nullptr, unsigned NumLoad = 0,
                      unsigned NumStore = 0, unsigned NumBranch = 0,
                      unsigned NumCmp = 0, unsigned NumCall = 0,
                      unsigned NumIntrinsic = 0)
      : F(F), NumLoad(NumLoad),
        NumStore(NumStore), NumBranch(NumBranch), NumCmp(NumCmp),
        NumCall(NumCall), NumIntrinsic(NumIntrinsic),
        PreciseInst(nullptr), InsertPosition(nullptr) {}

  // getters and setters
  void setF(Function *F) { this->F = F; }
  Instruction *getInst(void) const { return PreciseInst; }
  Instruction *getInsertPos(void) const { return InsertPosition; }

  // Analyze the given Instruction*, update relevant counters for
  // certain types of instructions.
  void analyze(Instruction *I) {
    // Skip any DebugInfo Intrinsic: don't count it
    if (isa<DbgInfoIntrinsic>(I))
      return;

    if (isa<LoadInst>(I))  // Count LoadInst
      ++NumLoad;
    else if (isa<StoreInst>(I))   // Count StoreInst
      ++NumStore;
    else if (isa<BranchInst>(I))  // Count BranchInst
      ++NumBranch;
    else if (isa<CmpInst>(I))  // Count CmpInst
      ++NumCmp;
    else if (isa<IntrinsicInst>(I))  // Count IntrinsicInst
      ++NumIntrinsic;
    else if (isa<CallBase>(I))  // Count CallInst
      ++NumCall;
  }

  // Analyze the current Function using PositionDescription info, and
  // obtain the target Instruction * at a precise location in F.
  void analyze(void) {
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      LLVM_DEBUG(dbgs() << *I << "\n";);
      analyze(&*I);
    }
  }

  bool compare(PositionDescription &PosDes) {
    return (F == PosDes.F)
        && (NumLoad == PosDes.NumLoad)
        && (NumStore == PosDes.NumStore)
        && (NumBranch == PosDes.NumBranch)
        && (NumCmp == PosDes.NumCmp)
        && (NumIntrinsic == PosDes.NumIntrinsic)
        && (NumCall == PosDes.NumCall);
  }

  bool operator==(PositionDescription &PD) {
    return compare(PD);
  }

  // Identity Function F's Insertion Position using pre-set counters.
  // - set Inst to point to the insertPosition if a precise match is obtained;
  //
  // Return: boolean
  // -true:  if a precise match is found;
  // -false: otherwise
  //
  bool identifyInsertPosition(Function *F) {
    assert(F && "Expect a valid Function F");
    this->F = F;
    PositionDescription Pos(F);
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      Instruction *Ins = &*I;
      LLVM_DEBUG(dbgs() << *Ins << "\n";);
      Pos.analyze(Ins);
      if (Pos.compare(*this)) {
        PreciseInst = &*I;        // mark current inst
        InsertPosition = &*(++I); // save next inst to allow insert before
        LLVM_DEBUG({
                     dbgs() << "PreciseInst: " << *PreciseInst << "\n"
                            << "InsertPosition: " << *InsertPosition << "\n";
                   });
        return true;
      }
    }
    return false;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD
  void dump() {
    if (F)
      dbgs() << "Function: " << F->getName() << "():\n";
    else
      dbgs() << "Function: nullptr\n";

    dbgs() << "Load: " << NumLoad << "\n"
           << "Store: " << NumStore << "\n"
           << "Branch: " << NumBranch << "\n"
           << "Compare: " << NumCmp << "\n"
           << "Intrinsic: " << NumIntrinsic << "\n"
           << "Call: " << NumCall << "\n";

    dbgs() << "Insert Position: ";
    InsertPosition ? dbgs() << *InsertPosition << "\n" : dbgs() << "nullptr\n";
  }
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

};

// This class analyzes the current module to ensure that the struct types
// equivalent to the following two exist on the module level:
//
// - %struct.ttbucket_t = type { i32, i16, i16, i8, i8 }
//  . 5 fields, all ints, order matters
//
// - %struct.ttentry_t = type { [4 x %struct.ttbucket_t] }
//  . 1 field, an array type , 4 * ttbucket_t
//  . refer to the ttbucket_t type the class has already recognized
//
class TypeAnalyzer {
  const Module &M;
  const IntegerType *IntegerTypeVec[5] = { //index
      Type::getInt1Ty(M.getContext()),  //  0
      Type::getInt8Ty(M.getContext()),  //  1
      Type::getInt16Ty(M.getContext()), //  2
      Type::getInt32Ty(M.getContext()), //  3
      Type::getInt64Ty(M.getContext()), //  4
  };

public:
  TypeAnalyzer(Module &M) : M(M) {}

  bool searchStructType(StructType *&MatchedType, // matched result type
                        const unsigned NumFields, ...) {
    // Unpack the variadic arguments into SmallVector FieldTypeIdxVec
    SmallVector<unsigned, 8> FieldTypeIdxVec;
    va_list arguments;
    va_start (arguments, NumFields);
    for (unsigned I = 0; I < NumFields; ++I) {
      unsigned Val = va_arg (arguments, unsigned);
      FieldTypeIdxVec.push_back(Val);
      LLVM_DEBUG(dbgs() << "Val: " << Val << "\n";);
      (void) I;
    }
    va_end (arguments);

    LLVM_DEBUG({
                 dbgs() << "FieldTypeIdxVec:\n";
                 for (auto V: FieldTypeIdxVec)
                   dbgs() << V << ", ";
                 dbgs() << "\n";
               });

    // try to match the TTBucketType:
    // %struct.ttbucket_t = type { i32, i16, i16, i8, i8 } type
    // index:                      3    2    2    1   1
    for (StructType *StructT: M.getIdentifiedStructTypes())
      if (matchStructType(StructT, NumFields, FieldTypeIdxVec)) {
        MatchedType = StructT;
        return true;
      }

    return false;
  }

  bool matchStructType(StructType *StructT, unsigned NumFields,
                       SmallVectorImpl<unsigned> &FieldTypeIdxVec) {
    if (NumFields != StructT->getNumElements())
      return false;
    for (unsigned I = 0, E = StructT->getNumElements(); I < E; ++I) {
      Type *Ty = StructT->getElementType(I);
      IntegerType *IntT = dyn_cast<IntegerType>(Ty);
      if (!IntT || IntT != IntegerTypeVec[FieldTypeIdxVec[I]])
        return false;
    }
    return true;
  }

  bool searchArrayType(StructType *&MatchedType, // matched result type
                       const unsigned NumFields, ...) {

    // Unpack the variadic arguments according to the table below
    // ------------------------------------------------------
    // |idx |  type      | value       |  meaning            |
    // ------------------------------------------------------
    // |0   | unsigned   | 1           | # types in struct   |
    // |1   | unsigned   | 4           | #elements in array  |
    // |2   | StructType*| TTBucketTy* | array element type  |
    // ------------------------------------------------------
    va_list arguments;
    va_start (arguments, NumFields);
    LLVM_DEBUG(dbgs() << "Index: 0, NumFields: " << NumFields << "\n";);

    // index 1: unsigned, array size
    const unsigned ArraySize = va_arg (arguments, unsigned);
    LLVM_DEBUG(dbgs() << "Index: 1, ArraySize: " << ArraySize << "\n";);

    // index 2: StructType *, TTBucketType *
    StructType *ArrayElemTy = va_arg (arguments, StructType*);
    LLVM_DEBUG(dbgs() << "Index: 2, ArrayElemTy: " << *ArrayElemTy << "\n";);

    va_end (arguments);

    LLVM_DEBUG({
                 dbgs() << "3 Unpacked Items (from VA-ARG list):\n"
                        << "NumFields: " << NumFields << "\n"
                        << "ArraySize: " << ArraySize << "\n"
                        << "ArrayElemTy: " << *ArrayElemTy << "\n";
               });

    // try to match the TTEntryType *:
    // %struct.ttentry_t = type { [4 x %struct.ttbucket_t] }
    for (StructType *StructT: M.getIdentifiedStructTypes())
      if (matchStructType(StructT, NumFields, ArraySize, ArrayElemTy)) {
        MatchedType = StructT;
        return true;
      }
    return false;
  }

  bool matchStructType(StructType *StructT, unsigned NumFields,
                       unsigned ArraySize, StructType *ArrayElemTy) {
    // Check: number of fields
    if (NumFields != StructT->getNumElements())
      return false;
    // Check: the only ArrayType field
    Type *Ty = StructT->getElementType(0);
    ArrayType *ArrTy = dyn_cast<ArrayType>(Ty);
    if (!ArrTy || (ArrTy->getArrayNumElements() != ArraySize)
        || (ArrTy->getArrayElementType() != ArrayElemTy))
      return false;
    return true;
  }

};

// This class is designed to conduct IPO-based prefetching for Delinquent Loads
// (DL).
// These DLs are not suitable for loop-based prefeching, because they are
// either not located inside a loop, or they are inside poorly formed loops
// that make loop-based prefetch difficult.
//
// The IPOPrefetch class will:
// - identify the DL(s) inside a LLVM Function according to pre-defined pattern
//   matching;
// - generate and optimize a prefetch function;
// - inside host function(s), insert call(s) to the prefetch function to enable
//   prefetching
//
class IPOPrefetcher {
private:
  SmallVector<Function *, 4> DLCandidates; // Functions where the DLs reside
  // Host function and its insert position(s):
  DenseMap<Function *, SmallVector<PositionDescription, 4>> PrefetchPositions;
  Function *PrefetchFunction = nullptr; // Place holder for prefetch function
  Module &M;
  LoadInst *DL = nullptr; // the delinquent load
  Function *MainF = nullptr; // the main function from the module
  function_ref<TargetLibraryInfo &(Function &)> GetTLI;
  function_ref<DominatorTree &(Function &)> LookupDomTree;
  function_ref<PostDominatorTree &(Function &)> LookupPostDomTree;
  unsigned ArrayElemSize = 0;
  unsigned NumArrayElem = 0;
  // the above 2 members refer to the following struct type:
  // %struct.ttentry_t = type { [4 x %struct.ttbucket_t] }
  // expect ArrayElemSize: 12 (B), NumArrayElem: 4
  WholeProgramInfo &WPInfo;

public:
  IPOPrefetcher(Module &M,
                function_ref<TargetLibraryInfo &(Function &)> GetTLI,
                function_ref<DominatorTree &(Function &)> LookupDomTree,
                function_ref<PostDominatorTree &(Function &)> LookupPostDomTree,
                WholeProgramInfo &WPInfo)
      : M(M), GetTLI(GetTLI), LookupDomTree(LookupDomTree),
        LookupPostDomTree(LookupPostDomTree), ArrayElemSize(0),
        NumArrayElem(0), WPInfo(WPInfo) {}

  bool run(Module &M);

  // getters + setters:
  LoadInst *getDL(void) const { return DL; }
  void setDL(LoadInst *DL) { this->DL = DL; }
  void setArrayElemSize(unsigned S) { ArrayElemSize = S; }
  unsigned getArrayElemSize(void) const { return ArrayElemSize; }
  void setNumArrayElem(unsigned N) { NumArrayElem = N; }
  unsigned getNumArrayElem(void) const { return NumArrayElem; }

private:
  // identify and collect pass-relevant info:
  // - functions where DL resides
  // - host functions into which calls to prefetch function will be inserted
  bool doCollection(void);

  // do both legal and applicable analysis
  bool doAnalysis(void);

  // Transform the module to enable IPO prefetching.
  // This includes code generation of:
  // - the prefetch function
  // - calls to prefetch functions in host(s)
  bool doTransformation(void);

  bool identifyDLFunctions(void);
  bool identifyPrefetchPositions(Function *F);
  bool identifyMainFunction(void);

  // Ensure that insert a call to prefetch function is legal, by:
  // - in case of using dynamically allocated memory, the malloc dominates
  //   prefetch inside a function that both closures join. (case1)
  // - if case1 and a call to free is available, ensure free post dominates
  //   prefetch inside a function that both closures join. (case2)
  // - ensure that it is legal to dereference the Node* inside a host function
  //   because the host function has already dereferenced the Node* before
  //   calling prefetch. (case3)
  //
  bool isLegal(void);
  bool isDominateProper(void);  // implementation of case1 and case2
  bool isAddrDereferenced(void);// implementation of case3

  // On the module level, 2 struct types are directly related to computing
  // the prefetch distance and generating the 2nd prefetch intrinsic inside
  // the prefetch function.
  // The applicable test ensures that these 2 struct types exist, and thus the
  // distance to the 2nd prefetch address can be reliably calculated.
  bool isApplicable(void);

  bool createPrefetchFunction(void);
  bool insertPrefetchFunction(void);

public:
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void dumpDLCandidate(void) {
    dbgs() << "DLCandidates: <" << DLCandidates.size() << "> \n";
    for (auto F: DLCandidates)
      dbgs() << F->getName() << "(), ";
    dbgs() << "\n";
  }

  LLVM_DUMP_METHOD void dumpPrefetchPositions(void) {
    dbgs() << "Prefetch Position(s): " << PrefetchPositions.size() << "\n";
    for (auto Pair: PrefetchPositions) {
      Function *F = Pair.first;
      auto &PosDesVec = Pair.second;
      dbgs() << F->getName() << "(): -> \n";

      unsigned Count = 0;
      for (auto PosDes: PosDesVec) {
        dbgs() << "\t" << Count++;
        PosDes.dump();
      }
    }
  }

  LLVM_DUMP_METHOD void dumpPrefetchFunction(bool PrintDetail = true) {
    dbgs() << "Prefetch Function: \n";
    if (!PrefetchFunction)
      return;
    dbgs() << PrefetchFunction->getName() << "(): \n";
    if (PrintDetail)
      dbgs() << *PrefetchFunction << "\n";
  }

  LLVM_DUMP_METHOD void dump(void) {
    dumpDLCandidate();
    dumpPrefetchPositions();
    dumpPrefetchFunction();
  }
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
};

bool IPOPrefetcher::run(Module &M) {
  // Check if AVX2 is supported.
  LLVM_DEBUG(dbgs() << "IPO Prefetch: BEGIN\n");
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
  if (!WPInfo.isAdvancedOptEnabled(TTIAVX2)) {
    LLVM_DEBUG(dbgs() << "NOT AVX2\n");
    LLVM_DEBUG(dbgs() << "IPO Prefetch: END\n");
    return false;
  }

  // Collect DL-resident function(s) and potential DL-host functions
  if (!doCollection())
    return false;

  // Analyze current module to ensure that the transformation is
  // -legal: correct and won't trigger any core-dump/seg fault issues,
  // -applicable: can generate the 2nd prefetch intrinsic.
  if (!doAnalysis())
    return false;

  // Transform the module to generate a fully optimized prefetch function,
  // and generate calls to the prefetch function in host function(s).
  if (!doTransformation())
    return false;

  // Ensure the module is good after transformation
  LLVM_DEBUG({
               if (!verifyModule(M,
                                 "Module verifier failed after IPOPrefetch "))
                 return false;
             });

  NumIPOPrefetch += DLCandidates.size();
  LLVM_DEBUG(dbgs() << "IPO Prefetch Triggered\n";);

  return true;
}

bool IPOPrefetcher::doCollection() {
  // Identify any DL function: a function that a DL resides
  if (!identifyDLFunctions()) {
    LLVM_DEBUG(dbgs() << "identify DLFunctions failed\n";);
    return false;
  }

  // Identify any DL-insert location
  for (auto *F: DLCandidates) {
    if (!identifyPrefetchPositions(F)) {
      LLVM_DEBUG(dbgs() << "identifyPrefetchPosition failed on "
                        << F->getName() << "()\n";);
      return false;
    }
  }

  // Identify the main() function: needed for legal analysis
  if (!identifyMainFunction()) {
    LLVM_DEBUG(dbgs() << "identify main Function failed\n";);
    return false;
  }

  // marker for IPO-Prefetch LIT test
  if (BeLITFriendly)
    LLVM_DEBUG(
        dbgs() << PrefetchFunctionName << " prefetch function identified\n";);

  return true;
}

// Legality tests: ensure prefetch is valid w.r.t. the current process.
//
// -------------------------------------------------------------
// ** dominate proper **
//
// A call to malloc forms a call closure ending in main. Similarly,
// each host function (where a prefetch call is inserted) forms a use closure
// also ending in main.
//
// The work in dominate proper ensures that (i) the intersection of the 2
// closures is NOT empty and (ii) in each join point, the call leading to malloc
// dominates the call leading to prefetch. (1)
//
// Similarly, if a call to free exists, the legal test ensures that (i) the
// intersection of the 2 closures is NOT empty and (ii) in each join point, the
// call leading to free post dominates the call leading to prefetch. (2)
//
// Refer to IPO Prefetch Design, slide #13 for (1) and #15 for (2).
//
// -------------------------------------------------------------
// ** node* is dereferenced before dereference inside prefetch  **
//
// In the host function (where a call to prefetch function is inserted),
// the node* is dereferenced before the call to prefetch, where the DL address
// is obtained using the same node*.
//
// This guarantees the node* is valid before control goes into the prefetch
// function.
//
// Refer to IPO Prefetch Design, slide #16 for example.
//
bool IPOPrefetcher::isLegal(void) {
  if (!isDominateProper()) {
    LLVM_DEBUG(dbgs() << "isLegal() - Dominate proper test failed\n";);
    return false;
  }

  if (!isAddrDereferenced()) {
    LLVM_DEBUG(dbgs() << "isLegal() - Address dereference test failed\n";);
    return false;
  }

  return true;
}

// In a given function, collect all Load Instructions that use addresses
// from GEPs that originate from an argument(index).
//
// E.g.
// define internal i32 @foo(%struct.state_t* nocapture %0) {
//  %11 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !24
//  %12 = load i32, i32* %11, align 4, !tbaa !24
//  %13 = icmp eq i32 %12, 0
// ...
//
// For the above partial function, there is at least 1 load that uses address
// from arg0 (%0: arg index 0). The load is %12, the address GEP is %11.
//
static bool getLoadsFromArg(Function *F,
                            const unsigned ArgIdx,
                            SmallVectorImpl<LoadInst *> &LoadVecFromArg) {
  SmallVector<Instruction *, 8> InstVecUseArg;
  if (ArgIdx >= F->arg_size()) {
    LLVM_DEBUG(dbgs() << "ArgIdx " << ArgIdx << " is out of bound\n";);
    return false;
  }
  Argument *Arg = &*(F->arg_begin() + ArgIdx);
  for (User *U : Arg->users())
    if (Instruction *Inst = dyn_cast<Instruction>(U)) {
      LLVM_DEBUG(dbgs() << "valid instruction: " << *Inst << "\n";);
      InstVecUseArg.push_back(Inst);
    }
  if (InstVecUseArg.empty()) {// if Arg is not used, it can't dominate anything
    LLVM_DEBUG(dbgs() << "Arg: " << *Arg << "is not used inside function\n";);
    return false;
  }

  // Collect all Load(s) that are using Insts originated from the Arg(Index)
  // [Note]
  // - expect the use of arg be a GEP that will be used later on load(s).
  //
  for (Instruction *Inst : InstVecUseArg) {
    // only consider GEPs:
    GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Inst);
    if (!GEP)
      continue;
    LLVM_DEBUG(dbgs() << " GEP: " << *GEP << "\n";);

    // check each use of the GEP
    for (User *U : GEP->users()) {
      // only consider LoadInst:
      LoadInst *LI = dyn_cast<LoadInst>(U);
      if (!LI)
        continue;

      LoadVecFromArg.push_back(LI);
      LLVM_DEBUG(dbgs() << " Load: " << *LI << "\n";);
    }
  }

  return !LoadVecFromArg.empty();
}

//
// Get the argument index for a struct-type argument.
// Expect there is only 1 struct-type* argument.
// If so, return true, and the index is returned in ArgIdx.
//
static bool getStructArgIndex(Function *F, unsigned &ArgIdx) {
  SmallVector<unsigned, 4> StructTypeArgIdxVec;

  // collect all StructType* function arg(s)
  for (unsigned I = 0, E = F->arg_size(); I < E; ++I) {
    Argument *arg = F->getArg(I);
    if (PointerType *PtrTy = dyn_cast<PointerType>(arg->getType())) {
      auto PointeeTy = PtrTy->getPointerElementType();
      if (isa<StructType>(PointeeTy))
        StructTypeArgIdxVec.push_back(I);
    }
  }

  // return true if only 1 struct-type arg is available.
  if (StructTypeArgIdxVec.size() != 1)
    return false;

  ArgIdx = StructTypeArgIdxVec[0];
  return true;
}

// In a host function (where a call to the prefetch function is inserted), the
// arg0 (node *) is dereferenced before obtaining the DL address using the same
// node* inside the prefetch function.
// In this case, this function will return true.
//
// E.g.
// foo(node * N, ..){
//   ...
//   if(N->field0){...} // (1): use N->field0
//   ...
//   prefetch(N); // (2): use N->field1
//   ..
// }
//
// In the example above, because existing code in (1) dereferences node* N to
// access its field0, the compiler knew that node * N is valid at (1).
// Thus in (2) that is newly inserted code and being dominated by (1), it is
// safe for prefetch(N) to dereference N->field1.
//
bool IPOPrefetcher::isAddrDereferenced(void) {
  if (!PrefetchPositions.size())
    return false;

  for (auto Pair: PrefetchPositions) {
    Function *F = Pair.first;
    auto &PosDesVec = Pair.second;
    DominatorTree &DT = LookupDomTree(*F);

    for (auto Pos: PosDesVec) {
      Instruction *InsertPos = Pos.getInsertPos();
      if (!InsertPos) {
        LLVM_DEBUG(dbgs() << "Fail to obtain insert position on host: "
                          << F->getName() << "()\n";);
        return false;
      }

      // Obtain the arg index that is a StructType *
      SmallVector<LoadInst *, 8> LoadVecFromArg;
      unsigned ArgIdx = 0; // defaults to 0
      if (!getStructArgIndex(F, ArgIdx)) {
        LLVM_DEBUG(dbgs() << "Expect a single struct-type arg in Function's "
                             "argument list\n";);
        return false;
      }

      // Obtain all uses of F's Arg[ArgIdx] in load(s):
      if (!getLoadsFromArg(F, ArgIdx, LoadVecFromArg)) {
        LLVM_DEBUG(dbgs() << "NO load from arg(" << ArgIdx << ")\n";);
        return false;
      }

      // Check:
      // Is there at least 1 use of the Arg that is a load and this load
      // dominates the projected insert position of a call to the prefetch
      // function?
      bool FindDominance = false;
      for (auto *LI: LoadVecFromArg)
        if (DT.dominates(LI, InsertPos)) {
          LLVM_DEBUG(dbgs() << "Has dominating relationship between "
                            << *LI << "\nand\n" << *InsertPos << "\n";);
          FindDominance = true;
          break;
        }

      if (!FindDominance)
        return false;
    } //end: for loop on Pos
  } //end: for loop on Pair
  return true;
}

// Do legal test on the host.
//
// Each host needs to satisfy the following conditions:
// - malloc-like call:
//   malloc dominates the host.
//
// - free call:
//   free post dominates the host if free is called.
//
bool IPOPrefetcher::isDominateProper(void) {
  // Collect all prefetch hosts:
  SmallVector<Function *, 4> PrefetchHosts;

  for (auto Pair: PrefetchPositions) {
    Function *F = Pair.first;
    LLVM_DEBUG(dbgs() << "Function: " << F->getName() << "() : \n";);
    PrefetchHosts.push_back(F);
  }

  if (PrefetchHosts.empty())
    return false;

  AllocFreeAnalyzer MFA(M, MainF, GetTLI, LookupDomTree, LookupPostDomTree);

  for (auto *F: PrefetchHosts) {
    if (!MFA.analyzeForAlloc(F)) {
      LLVM_DEBUG(dbgs() << "analysis for malloc dominates host " << F->getName()
                        << "() failed";);
      return false;
    }

    if (!MFA.analyzeForFree(F)) {
      LLVM_DEBUG(
          dbgs() << "analysis for free post dominates host " << F->getName()
                 << "() failed";);
      return false;
    }
  }

  return true;
}

// Verify that the following 2 equivalent struct types exist on module level:
// - a ttbucket_t type
// - a ttentry_t type
//
// Details:
//
// [C level]
// struct ttbucket_t {
//   unsigned int   hash;
//   short          bound;
//   unsigned short bestmove;
//   unsigned char  depth;
//   unsigned char  threat     : 1,
//                  type       : 2,
//                  singular   : 1,
//                  nosingular : 1,
//                  age        : 2;
// };
//
// struct ttentry_t {
//   ttbucket_t buckets[BUCKETS]; // BUCKETS = 4
// };
//
// [LLVM level]
// %struct.ttbucket_t = type { i32, i16, i16, i8, i8 }
// %struct.ttentry_t = type { [4 x %struct.ttbucket_t] }
//
bool IPOPrefetcher::isApplicable(void) {
  StructType *TTBucketTy = nullptr; // matched BucketType
  TypeAnalyzer TA(M);

  // Identify TBucket Type:
  // %struct.ttbucket_t = type { i32, i16, i16, i8, i8 }
  bool Result = TA.searchStructType(TTBucketTy, 5, 3 /*i32*/, 2 /*i16*/,
                                    2 /*i16*/, 1 /*i8*/, 1 /*i8*/);
  if (!Result || !TTBucketTy) {
    LLVM_DEBUG(dbgs() << "Fail to find TTBucketTy\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << "TTBucketTy: " << *TTBucketTy << "\n";);

  // Identify TTEntry Type:
  // %struct.ttentry_t = type { [4 x %struct.ttbucket_t] }
  StructType *TTEntryTy = nullptr;  // matched EntryType
  const unsigned ArraySize = AppTestArraySize;
  Result = TA.searchArrayType(TTEntryTy,
                              1 /* NumFields */,
                              ArraySize /*: 4*/,
                              TTBucketTy /* ArrayElemTy */);
  if (!Result || !TTEntryTy) {
    LLVM_DEBUG(dbgs() << "Fail to find TTEntryTy\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << "TTEntryTy: " << *TTEntryTy << "\n";);

  // record the NumArrayItems and ArrayElemSize:
  auto &DL = M.getDataLayout();
  setArrayElemSize(DL.getTypeAllocSize(TTBucketTy));
  setNumArrayElem(AppTestArraySize /* 4 */);

  return true;
}

bool IPOPrefetcher::doAnalysis(void) {
  if (!BeLITFriendly && !isLegal()) {
    LLVM_DEBUG(dbgs() << "Legal test failed\n";);
    return false;
  }

  // If applicable test fails, flag to generate the 2nd prefetch intrinsic will
  // be off.
  // However, prefetch pass will continue and only 1 prefetch intrinsic will be
  // generated.
  if (!isApplicable()) {
    LLVM_DEBUG(dbgs() << "Applicable test failed\n";);
    Generate2ndPrefetchInst = false;
  }

  return true;
}

bool IPOPrefetcher::doTransformation(void) {
  // Generate a Prefetch Function
  if (!createPrefetchFunction()) {
    LLVM_DEBUG(dbgs() << "PrefetchFunction creation failed\n";);
    return false;
  }

  // Insert a call to the prefetch function at each prefetch position
  // inside all host function(s)
  if (!insertPrefetchFunction()) {
    LLVM_DEBUG(dbgs() << "PrefetchFunction insertion failed\n";);
    return false;
  }

  return true;
}

// On the module level, identify the function(s) that have delinquent load(s)
// using heuristics.
// Collect all such functions into DLCandidate.
//
// Identify DL Functions using Function Signature Matcher.
// In future, this may be refined into using DL-related profile info
// when such information becomes available.
//
// E.g. the DL Function we are looking for:
//
// define internal i32 @_Z7ProbeTTP7state_tPiiiPjS1_S1_S1_S1_i(
//   %struct.state_t* nocapture %0,
//   i32* nocapture %1,
//   i32 %2,
//   i32 %3,
//   i32* nocapture %4,
//   i32* nocapture %5,
//   i32* nocapture %6,
//   i32* nocapture %7,
//   i32* nocapture %8,
//   i32 %9);
//
// Statistics:
// total arguments: 10
//   int arg:        3
//     * arg:        7
//    ** arg:        0
//   ret:          i32
//
bool IPOPrefetcher::identifyDLFunctions(void) {
  static const FunctionSignatureMatcher
      DLFuncMatcher = FunctionSignatureMatcher(
      DLFuncMinExpectedArgs /* MinNumArg: 10 */,
      DLFuncMaxExpectedArgs /* MaxNumArg: 10*/,
      DLFuncMaxIntArgs   /* MinNumIntArg: 3 */,
      DLFuncMinIntArgs   /* MaxNumIntArg: 3 */,
      DLFuncMinIntPtrArgs   /* NumMinPtrArg: 7 */,
      DLFuncMaxIntPtrArgs   /* NumMaxPtrArg: 7 */,
      DLFuncMinDltIntPtrArgs   /* MinNumDoublePtrArgs: 0 */,
      DLFuncMaxDltIntPtrArgs   /* MaxNumDoublePtrArgs: 0 */,
      false /* LeafFunc */); // make it false for debugging only!

  auto isDLFunction = [&](Function *F) {
    return DLFuncMatcher.match(F);
  };

  for (auto &F : M.functions())
    if (isDLFunction(&F))
      DLCandidates.push_back(&F);

  LLVM_DEBUG({ dumpDLCandidate(); });

  // Sorting DLCandidates vector puts its contents in a predictable order.
  // This will simplify later matching.
  llvm::sort(DLCandidates.begin(), DLCandidates.end(),
            [&](const Function *F0, const Function *F1) -> bool {
              return F0->arg_size() > F1->arg_size();
            });

  LLVM_DEBUG({
               dbgs() << " Check collected Function order\n";
               dumpDLCandidate();
             });

  return (DLCandidates.size() == ExpectedNumDLFunctions);
}

//
// Identify all positions where a prefetch function will be inserted.
// A Prefetch-insert position may not be in the same function where its
// respective DL resides.
//
// One FunctionSignatureMatcher object per prefetch-insert position is
// constructed. The content detail of each such object is collected w.r.t. the
// experimentally inserted call to prefetch.
//
bool IPOPrefetcher::identifyPrefetchPositions(Function *F) {
  // An array of 2 FunctionSignatureMatcher objects, because there are 2
  // expected host functions to insert the prefetch function, one prefetch each.
  //
  static const FunctionSignatureMatcher FSMA[] = {
      FunctionSignatureMatcher(
          DL0HostFuncMinArgs   /* MinNumArg: 5 */,
          DL0HostFuncMaxArgs    /* MaxNumArg: 5*/,
          DL0HostFuncMinIntArgs /* MinNumIntArg: 4 */,
          DL0HostFuncMaxIntArgs /* MaxNumIntArg: 4 */,
          DL0HostFuncMinIntPtrArgs  /* NumPtrArg0: 1 */,
          DL0HostFuncMaxIntPtrArgs   /* NumPtrArg1: 1 */,
          0   /* MinNumDoublePtrArgs: 0 */,
          0   /* MaxNumDoublePtrArgs: 0 */,
          false /* LeafFunc */),

      FunctionSignatureMatcher(
          DL1HostFuncMinArgs    /* MinNumArg: 6 */,
          DL1HostFuncMaxArgs    /* MaxNumArg: 6*/,
          DL1HostFuncMinIntArgs   /* MinNumIntArg: 5 */,
          DL1HostFuncMaxIntArgs  /* MaxNumIntArg: 5 */,
          DL1HostFuncMinIntPtrArgs   /* NumPtrArg0: 1 */,
          DL1HostFuncMaxIntPtrArgs   /* NumPtrArg1: 1 */,
          0   /* MinNumDoublePtrArgs: 0 */,
          0   /* MaxNumDoublePtrArgs: 0 */,
          false /* LeafFunc */),
  };

  // Check: can any of the provided FunctionSignatureMatcher objects match
  // a given function?
  auto IsSuitableDLCaller = [&](Function *F) {
    for (unsigned I = 0, E = sizeof(FSMA)/sizeof(FunctionSignatureMatcher);
         I < E; ++I)
      if (FSMA[I].match(F))
        return true;
    return false;
  };

  // Functor: compare function by its #args
  struct CompareFunctionPtr :
      public std::binary_function<Function *, Function *, bool> {
    bool operator()(const Function *lhs, const Function *rhs) const {
      if (lhs == nullptr || rhs == nullptr)
        return lhs < rhs;
      return lhs->arg_size() < rhs->arg_size();
    }
  };

  SmallSet<Function *, 4, CompareFunctionPtr> DLCallers; // sorted by #arg

  // Identify hosts (prefetch-insert) functions:
  // - filtering from DL Function's callers
  for (User *U : F->users()) {
    CallBase *CB = dyn_cast<CallInst>(U);
    if (!CB || CB->isIndirectCall())
      continue;

    Function *CallerFunction = CB->getFunction();
    assert(CallerFunction && "Expect caller exist");
    LLVM_DEBUG(dbgs() << "Caller: " << CallerFunction->getName() << "()\n";);

    if (IsSuitableDLCaller(CallerFunction))
      DLCallers.insert(CallerFunction);
  }

  LLVM_DEBUG({
               dbgs() << "DLCallers: <" << DLCallers.size() << ">\n";
               for (auto *F: DLCallers)
                 dbgs() << F->getName() << "() \n";
             });


  // 2 Position Descriptions, each needs precise customization
  static PositionDescription PDA[] = {
      PositionDescription(nullptr    /* Function * */,
                          54   /* NumLoad: 54 */,
                          5    /* NumStore */,
                          68  /* NumBranch */,
                          55    /* NumCmp */,
                          23     /* NumCall */,
                          6   /* NumInstrinsic */),

      PositionDescription(nullptr     /* Function * */,
                          144   /* NumLoad: 144 */,
                          39    /* NumStore */,
                          133  /* NumBranch */,
                          141    /* NumCmp */,
                          62      /* NumCall */,
                          14   /* NumInstrinsic */),

  };

  // Finish construction of the 2 PositionDescription objects
  unsigned Count = 0;
  for (auto F: DLCallers) {
    PositionDescription &PD = PDA[Count++];
    if (PD.identifyInsertPosition(F)) {
      LLVM_DEBUG({
                   dbgs() << "Precise InsertPosition Identified:\t"
                          << F->getName() << "():\n";
                   PD.dump();
                 });
      auto &PosDesV = PrefetchPositions[F];
      PosDesV.push_back(PD);
    } else {
      LLVM_DEBUG({
                   dbgs() << "Precise InsertPosition NOT Identified:\t"
                          << F->getName() << "():\n";
                 });
    }
  }

  return (PrefetchPositions.size() == ExpectedNumPrefetchInsertPositions);
}

// Search module, find and save the main() function
bool IPOPrefetcher::identifyMainFunction(void) {
  return (MainF = WPInfo.getMainFunction(M));
}

// Check if a StoreInst is to write into a stack-allocated memory.
// (All non-local stores have global side effects and thus unsafe.)
static bool IsLocalStore(Value *V) {
  LLVM_DEBUG(dbgs() << "Value: : " << *V << "\n";);

  if (isa<Argument>(V) || isa<GlobalVariable>(V))
    return false;
  else if (isa<StoreInst>(V)) {
    StoreInst *SI = dyn_cast<StoreInst>(V);
    return IsLocalStore(SI->getPointerOperand());
  } else if (isa<GetElementPtrInst>(V)) {
    GetElementPtrInst *GEPT = dyn_cast<GetElementPtrInst>(V);
    return IsLocalStore(GEPT->getOperand(0));
  }
  return true;
}

// RemoveDeadThingsFromFunction:
// - remove any dead arguments;
// - change function's return type to void and change any return instruction to
//   ret.
//
// [Note]
// This function does similar work as Dead Argument Elimination (DAE) pass.
// Since DAE works on module level only, this function is created to do DAE-
// equivalent work on the prefetch function.
//
static bool RemoveDeadThingsFromFunction(Function *F, Function *&NF,
                                         unsigned &NumArgumentsEliminated) {

  // Build a new prototype for the function, which is the same as the old
  // function, but has fewer arguments and a different return type.
  FunctionType *FTy = F->getFunctionType();
  std::vector<Type *> Params;

  // Keep track of whether we have a live 'returned' argument
  bool HasLiveReturnedArg = false;

  // Set up to build a new list of parameter attributes.
  SmallVector<AttributeSet, 8> ArgAttrVec;
  const AttributeList &PAL = F->getAttributes();

  // Remember which arguments are live.
  SmallVector<bool, 8> ArgAlive(FTy->getNumParams(), false);

  // Construct the new parameter list from non-dead arguments. Also construct
  // a new set of parameter attributes to correspond. Skip the first parameter
  // attribute, since that belongs to the return value.
  unsigned i = 0;
  for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I, ++i) {
    if (I->getNumUses()) {
      Params.push_back(I->getType());
      ArgAlive[i] = true;
      ArgAttrVec.push_back(PAL.getParamAttributes(i));
      HasLiveReturnedArg |= PAL.hasParamAttribute(i, Attribute::Returned);
    } else {
      ++NumArgumentsEliminated;
      LLVM_DEBUG(dbgs() << "Removing argument "
                        << i << " (" << I->getName()
                        << ") from " << F->getName() << "()\n");
    }
  }

  // Find out the new return value
  Type *NRetTy = Type::getVoidTy(F->getContext());
  unsigned RetCount = 0; // new function will return void

  // -1 means unused, other numbers are the new index
  SmallVector<int, 5> NewRetIdxs(RetCount, -1);
  std::vector<Type *> RetTypes;

  // The existing function's return attributes.
  AttrBuilder RAttrs(PAL.getRetAttributes());

  // Remove any incompatible attributes, but only if we removed all return
  // values. Otherwise, ensure that we don't have any conflicting attributes
  // here. Currently, this should not be possible, but special handling might be
  // required when new return value attributes are added.
  RAttrs.remove(AttributeFuncs::typeIncompatible(NRetTy));

  AttributeSet RetAttrs = AttributeSet::get(F->getContext(), RAttrs);

  // Strip allocsize attributes. They might refer to the deleted arguments.
  AttributeSet FnAttrs = PAL.getFnAttributes().removeAttribute(
      F->getContext(), Attribute::AllocSize);

  // Reconstruct the AttributesList based on the vector we constructed.
  assert(ArgAttrVec.size() == Params.size());
  AttributeList NewPAL =
      AttributeList::get(F->getContext(), FnAttrs, RetAttrs, ArgAttrVec);

  // Create the new function type based on the recomputed parameters.
  FunctionType *NFTy = FunctionType::get(NRetTy, Params, FTy->isVarArg());

  // No change?
  if (NFTy == FTy)
    return false;

  // Create the new function body and insert it into the module...
  NF = Function::Create(NFTy, F->getLinkage(), F->getAddressSpace());
  NF->copyAttributesFrom(F);
  NF->setComdat(F->getComdat());
  NF->setAttributes(NewPAL);
  // Insert the new function before the old function, so we won't be processing
  // it again.
  F->getParent()->getFunctionList().insert(F->getIterator(), NF);
  NF->takeName(F);

  // Since we have now created the new function, splice the body of the old
  // function right into the new function, leaving the old rotting hulk of the
  // function empty.
  NF->getBasicBlockList().splice(NF->begin(), F->getBasicBlockList());

  // Loop over the argument list, transferring uses of the old arguments over to
  // the new arguments, also transferring over the names as well.
  i = 0;
  for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(),
           I2 = NF->arg_begin(); I != E; ++I, ++i)
    if (ArgAlive[i]) {
      // If this is a live argument, move the name and users over to the new
      // version.
      I->replaceAllUsesWith(&*I2);
      I2->takeName(&*I);
      ++I2;
    } else {
      // If this argument is dead, replace any uses of it with undef
      // (any non-debug value uses will get removed later on).
      if (!I->getType()->isX86_MMXTy())
        I->replaceAllUsesWith(UndefValue::get(I->getType()));
    }

  // If we need to change the return value of the function, we must rewrite any
  // return instructions.
  if (F->getReturnType() != NF->getReturnType())
    for (BasicBlock &BB : *NF)
      if (ReturnInst *RI = dyn_cast<ReturnInst>(BB.getTerminator())) {
        // Replace the return instruction with one returning the new return
        // value
        Value *RetVal = nullptr;
        ReturnInst::Create(F->getContext(), RetVal, RI);
        BB.getInstList().erase(RI);
      }

  // Clone metadata from the old function, including debug info descriptor.
  SmallVector<std::pair<unsigned, MDNode *>, 1> MDs;
  F->getAllMetadata(MDs);
  for (auto MD : MDs)
    NF->addMetadata(MD.first, *MD.second);

  // Delete the old function
  F->eraseFromParent();

  return true;
}

// A Prefetch function is a function that takes a single node_t* as argument,
// calculates the DL's address, issues 1 (or 2) prefetch of the DL address at
// the end of the function.
//
// The creation of the prefetch function follows a sequence of actions:
// - Clone from DL function:
//   Have a full clone of the function where DL resides. (ProbeTT). This builds
//   the initial PrefetchFunction. This serves the basis for the remaining steps.
//
// Prepare for cleanup:
// - Insert a ret:
//   Insert a "ret i32 0" instruction right before the DL load.
//   This helps to prepare for cleanup.
//
//  -Remove any store to non-local storage:
//   For any store into a global or function argument, remove this store
//   instruction and all instructions used by the store.
//
// - Cleanup1: (invoke -simplfycfg and -early-cse)
//   Run -simplifycfg and -early-cse. This will remove the loop and any control
//   flow, and vastly simply the prefetch function.
//
// - Cleanup2:
//   Cleanup arguments:
//   Keep the 1st argument, remove all other (6) arguments because they are not
//   used in the prefetch function.
//
//   Change return type:
//   Change function's return type to void, and update any ret instruction.
//
// - Insert prefetch instruction(s):
//   Insert the 1st prefetch intrinsic at the very end of the function.
//
//   Selectively insert the 2nd prefetch intrinsic after the 1st one if a
//   control flag is on.
//
// -----------------------------------------------------------------------
// [Note]
// INPUT is in: SmallVector<Function *, 4> DLCandidates after collection
// -----------------------------------------------------------------------
// The skeleton of the prefetch function may look like the following.
//
// void Prefetch(Node * N){
//   ... compute &Data[0] from N*, where Data[] is the array suffering from
//       frequent LLC miss, and Data[] is a field inside Node type.
//   prefetch(&Data[0]);
//
//   ... compute &Data[last-idx] from &Data[0]
//   prefetch(&Data[last-idx]);
// }
//
bool IPOPrefetcher::createPrefetchFunction(void) {

  // Clone from Function F*, with always_inline attribute
  auto CloneDLFunction = [&](Function *F) -> Function * {
    ValueToValueMapTy Old2New;
    FunctionType *FTy = F->getFunctionType();
    const unsigned NParams = FTy->getNumParams();
    SmallVector<Type *, 8> Tys;
    Tys.reserve(NParams);

    AttributeList NewAttrs;
    AttributeList Attrs = F->getAttributes();
    LLVMContext &Ctx = F->getContext();
    unsigned NumNewArgs = 0;

    for (auto I : {AttributeList::ReturnIndex, AttributeList::FunctionIndex})
      if (Attrs.hasAttributes(I))
        NewAttrs = NewAttrs.addAttributes(Ctx, I, Attrs.getAttributes(I));

    for (unsigned I = 0; I < NParams; ++I) {
      Tys.push_back(FTy->getParamType(I));
      if (Attrs.hasParamAttrs(I))
        NewAttrs = NewAttrs.addParamAttributes(Ctx, NumNewArgs,
                                               Attrs.getParamAttributes(I));
      ++NumNewArgs;
    }

    FunctionType
        *NewFTy = FunctionType::get(FTy->getReturnType(), Tys, FTy->isVarArg());

    // Do the cloning:
    Function *Clone = Function::Create(NewFTy,
                                       F->getLinkage(),
                                       PrefetchFunctionName,
                                       F->getParent());
    Clone->setAttributes(NewAttrs);
    Clone->setCallingConv(F->getCallingConv());

    auto NewI = Clone->arg_begin();
    for (auto I = F->arg_begin(); I != F->arg_end(); ++I) {
      NewI->setName(I->getName());
      Old2New[&*I] = &*NewI;
      ++NewI;
    }

    SmallVector<ReturnInst *, 8> Rets;
    CloneFunctionInto(Clone, F, Old2New, true, Rets);

    // Aim to have the prefetch function ALWAYS inlined later in IPO
    if (!Clone->hasFnAttribute(Attribute::AlwaysInline))
      Clone->addFnAttr(Attribute::AlwaysInline);

    LLVM_DEBUG(dbgs() << "Clone: " << *Clone << "\n";);

    return Clone;
  };

  // Clean up the clone to make it suitable as a prefetch function.
  // This includes:
  // - replace cond_br with a ret 0 in the DL BB;
  // - remove any non-local store;
  // - call existing -simply-cfg and -early-cse to do cleanup;
  //
  auto CleanupClone = [&](Function *F) -> bool {

    // Insert a ret instruction at the end of the BB where the delinquent load
    // (DL) resides. This replaces the original end-of-BB terminator (a
    // conditional branch).
    //
    // E.g.
    // [BEFORE]
    // 32:                                               ; preds = %30, %10
    // %33 = phi i64 [ 0, %10 ], [ %38, %30 ]          ; loop iv: i
    // %34 = getelementptr inbounds [4 x %struct.ttbucket_t],
    //       [4 x %struct.ttbucket_t]* %28, i64 0, i64 %33, !intel-tbaa !185
    // %35 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %34,
    //       i64 0, i32 0, !intel-tbaa !213
    // %36 = load i32, i32* %35, align 4, !tbaa !214   ; *** the DL !!!
    // %37 = icmp eq i32 %36, %29
    // %38 = add nuw nsw i64 %33, 1
    // br i1 %37, label %39, label %30 ; <<- original conditional branch
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //
    //[AFTER]
    // 32:                                               ; preds = %30, %10
    // %33 = phi i64 [ 0, %10 ], [ %38, %30 ]          ; loop iv: i
    // %34 = getelementptr inbounds [4 x %struct.ttbucket_t],
    //       [4 x %struct.ttbucket_t]* %28, i64 0, i64 %33, !intel-tbaa !185
    // %35 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %34,
    //       i64 0, i32 0, !intel-tbaa !213
    // %36 = load i32, i32* %35, align 4, !tbaa !214   ; *** the DL !!!
    // %37 = icmp eq i32 %36, %29
    // %38 = add nuw nsw i64 %33, 1
    // ret i32 0                        ; <<= replace the br with a ret 0
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //
    // PD for the DL:
    // (#Load:6, #Store: 2, #Cmp: 2, #Branch: 2, #Call: 0, #Intrinsic: 0)
    static PositionDescription PD(F   /* Function * */,
                                  6   /* NumLoad */,
                                  2   /* NumStore */,
                                  2  /* NumBranch */,
                                  2    /* NumCmp */,
                                  0     /* NumCall */,
                                  0  /* NumInstrinsic */);

    // Locate the DL:
    if (!PD.identifyInsertPosition(F)) {
      LLVM_DEBUG(dbgs() << "Identify Insert Position failed\n";);
      return false;
    }
    Instruction *DLInst = PD.getInst();
    assert(DLInst && "Expect a valid instruction");
    if (!isa<LoadInst>(DLInst)) {
      LLVM_DEBUG(dbgs() << "Expect a load on the DL position\n";);
      return false;
    }
    setDL(dyn_cast<LoadInst>(DLInst)); // Record the DL

    // Create an Ret 0 inst and replace the original terminator in the BB
    // where the DL resides.
    BasicBlock *BB = DLInst->getParent();
    Type *RetTy = F->getReturnType();
    assert(RetTy && "Expect RetTy be a valid ptr\n");
    ReturnInst *RetInst = nullptr;
    if (RetTy->isVoidTy())
      RetInst = ReturnInst::Create(M.getContext());
    else
      RetInst = ReturnInst::Create(M.getContext(), DLInst);
    ReplaceInstWithInst(BB->getTerminator(), RetInst);
    LLVM_DEBUG(dbgs() << "BB: " << *BB << "\n";);

    LLVM_DEBUG({
                 if (!verifyFunction(F,
                                     "Function verification failed after insert ret inst"))
                   return false;
                 dbgs() << "Clone after replace cond_br with a ret\n" << *F
                        << "\n";
               });

    // Simplify the prefetch function by calling existing scalar optimization
    // passes. In particular, the following ScalarOpt passes are called in order:
    // - Simplify CFG
    // - Early CSE
    //
    // This will vastly simplify the existing prefetch function by:
    // - remove the now-dead loop structure
    // - remove all existing control flows in loop
    //
    llvm::legacy::FunctionPassManager FPM(&M);
    FPM.add(createCFGSimplificationPass());
    FPM.add(createEarlyCSEPass());

    FPM.doInitialization();
    FPM.run(*F);
    FPM.doFinalization();

    LLVM_DEBUG({
                 if (!verifyFunction(F, "Function verification failed after "
                                        "simplifications"))
                   return false;
                 dbgs() << "Clone after simplification work\n" << *F << "\n";
               });

    return true;
  };

  // - 2nd Cleanups:
  //   Cleanup stores to non-local storage:
  //   remove any chains of instructions that form the address and/or value
  //   for any non-local store.
  //
  //   Cleanup arguments and return:
  //   - keep the 1st argument, remove all other (6) arguments because they are
  //     not used in the prefetch function.
  //   - change return type to void, and update any return instruction as well.
  //
  auto SimplifyPrefetchAgain = [&](Function *&F) -> bool {
    // Remove all stores into non-local memory:
    // - these are stores into globals or function arguments.
    SmallVector<Instruction *, 4> NonLocalStoreV;
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      Instruction *Inst = &*I;
      if (isa<StoreInst>(Inst) && !IsLocalStore(Inst))
        NonLocalStoreV.push_back(Inst);
    }

    if (NonLocalStoreV.empty()) {
      LLVM_DEBUG(dbgs() << "No NonLoacl Store collected\n";);
      return false;
    }

    LLVM_DEBUG(dumpInstVec(NonLocalStoreV, "All Non-Loacl Stores "););

    for (auto *I: NonLocalStoreV) {
      LLVM_DEBUG(
          dbgs() << "To DeleteLocalStoreChain: " << *I << "\n";);
      I->eraseFromParent();
    }

    // Run scalar optimization passses with a function pass manager
    llvm::legacy::FunctionPassManager FPM(&M);
    FPM.add(createDeadCodeEliminationPass());
    FPM.doInitialization();
    bool HasChanges = FPM.run(*F);
    FPM.doFinalization();

    if (HasChanges)
      LLVM_DEBUG(
          dbgs() << "Clone after simplification work\n: " << *F << "\n";);
    (void) HasChanges;

    // Remove all dead arguments and change the function's return type to void,
    // so as any ret instruction.
    unsigned NumArgumentsEliminated = 0;
    Function *NF = nullptr;
    if (!RemoveDeadThingsFromFunction(F, NF, NumArgumentsEliminated)) {
      LLVM_DEBUG(
          dbgs() << "Failure in RemoveDeadThingsFromFunction(.): "
                 << F->getName() << "()\n";);
      return false;
    }

    assert(NF && "Expect a valid NewFunction now");
    F = NF;

    LLVM_DEBUG({
                 if (!verifyFunction(F,
                                     "Function verification failed after simplifyAgain"))
                   return false;

                 dbgs() << "After dead-arg removal: " << F->getName() << "()\n"
                        << *F << "\n";
               });

    return true;
  };

  //At this point, the prefetch function will look like:
  //; Function Attrs: alwaysinline nofree norecurse nounwind uwtable
  // define internal void @Prefetch.Backbone(%struct.state_t* nocapture %0) #19 {
  //    %2 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 11, !intel-tbaa !24
  //    %3 = load i32, i32* %2, align 4, !tbaa !24
  //    %4 = icmp eq i32 %3, 0
  //    %5 = getelementptr inbounds %struct.state_t, %struct.state_t* %0, i64 0, i32 16
  //    %6 = load i64, i64* %5, align 8, !tbaa !61
  //    %7 = zext i1 %4 to i64
  //    %8 = add i64 %6, %7
  //    %9 = trunc i64 %8 to i32
  //    %10 = load %struct.ttentry_t*, %struct.ttentry_t** @TTable, align 8, !tbaa !179
  //    %11 = load i32, i32* @TTSize, align 4, !tbaa !50
  //    %12 = urem i32 %9, %11
  //    %13 = zext i32 %12 to i64
  //    %14 = getelementptr inbounds %struct.ttentry_t, %struct.ttentry_t* %10, i64 %13
  //    %15 = getelementptr inbounds %struct.ttentry_t, %struct.ttentry_t* %14, i64 0, i32 0, !intel-tbaa !181
  //    %16 = getelementptr inbounds [4 x %struct.ttbucket_t], [4 x %struct.ttbucket_t]* %15, i64 0, i64 0, !intel-tbaa !185
  //    %17 = getelementptr inbounds %struct.ttbucket_t, %struct.ttbucket_t* %16, i64 0, i32 0, !intel-tbaa !213
  //    %18 = load i32, i32* %17, align 4, !tbaa !214
  //    ret void
  // }
  //
  // Insert a prefetch intrinsic instruction and replace the original DL.
  // Insertion of the 2nd prefetch intrinsic is optional, and is controlled
  // by the Generate2ndPrefetchInst flag (set to true by default).
  //
  // as:
  //   ...
  //   %18 = bitcast %struct.bucket_t* %17 to i8*  ; new code
  //   call void @llvm.prefetch.p0i8(i8* %18, i32 0, i32 3, i32 1) ; new code
  //   %18 = load i32, i32* %17, align 4, !tbaa !214 ; old code
  //   ret void                                      ; old code
  //
  auto InsertPrefetchInsts = [&](Function *F) -> bool {
    LoadInst *DLInst = getDL();
    if (!DLInst) {
      LLVM_DEBUG(dbgs() << "expect a valid DL\n";);
      return false;
    }
    IRBuilder<> Builder(M.getContext());

    // ** Insert the 1st prefetch intrinsic **
    // insert a bitcast from address to i8*
    BasicBlock::iterator It(DLInst);
    ++It; // After this line, It refers to the instruction after DLLoad
    if (It == DLInst->getParent()->end())
      assert(0 && "reached end of BB");

    Builder.SetInsertPoint(&*It);
    Value *PrefetchAddr = Builder.CreateBitCast(DLInst->getPointerOperand(),
                                                Type::getInt8PtrTy(M.getContext()),
                                                "bitcast-for-prefetch0");

    // insert a prefetch(addr,3) intrinsic: prefetch into L3 cache
    Type *I32 = Type::getInt32Ty(M.getContext());
    Type *I64 = Type::getInt64Ty(M.getContext());
    Function *PrefetchFunc = Intrinsic::getDeclaration(
        &M, Intrinsic::prefetch, PrefetchAddr->getType());

    CallInst *PrefetchInst = Builder.CreateCall(
        PrefetchFunc,
        {PrefetchAddr,            // prefetch address
         ConstantInt::get(I32, 0),// rw: 0 for read
         ConstantInt::get(I32, 3),// L3
         ConstantInt::get(I32, 1) // 1: data cache
        });

    LLVM_DEBUG(dbgs() << "PrefetchInst: " << *PrefetchInst << "\n";);
    LLVM_DEBUG(
        dbgs() << "BasicBlock:\n" << *PrefetchInst->getParent() << "\n";);
    LLVM_DEBUG(dbgs() << "Function:\n" << *F << "\n";);
    (void) PrefetchInst;

    // Insert 2nd (optional) prefetch intrinsic
    if (Generate2ndPrefetchInst) {
      // Generate the prefetchaddr + offset address using the following sequence,
      // assuming prefetchaddr is in %10:
      //
      // %11 = ptr2int %10 to i64
      // %12 = add  i64 %11, offset
      // %13 = int2ptr i64 %12 to i8*
      //
      Value *Ptr2Int =
          Builder.CreatePtrToInt(PrefetchAddr,
                                 Type::getInt64Ty(M.getContext()),
                                 "ptr2i32");
      const unsigned NumArrayElem = getNumArrayElem();
      const unsigned ArrayElemSize = getArrayElemSize();
      if (!NumArrayElem || !ArrayElemSize) {
        LLVM_DEBUG(dbgs() << "Incorrect NumArrayElem or ArrayElemSize\n";);
        return false;
      }
      unsigned Offset = (NumArrayElem - 1)*ArrayElemSize;
      Value *IntPlusOffset = Builder.CreateAdd(Ptr2Int,
                                               ConstantInt::get(I64, Offset),
                                               "intplusoffset");

      Value *PrefetchAddr2 =
          Builder.CreateIntToPtr(IntPlusOffset,
                                 Type::getInt8PtrTy(M.getContext()),
                                 "prefetch2-addr");

      // insert the 2nd prefetch(addr2,3) intrinsic:
      CallInst *PrefetchInst2 = Builder.CreateCall(
          PrefetchFunc,
          {PrefetchAddr2,           // prefetch address
           ConstantInt::get(I32, 0),// rw: 0 for read
           ConstantInt::get(I32, 3),// L3
           ConstantInt::get(I32, 1) // 1: data cache
          });

      LLVM_DEBUG(dbgs() << "PrefetchInst2: " << *PrefetchInst2 << "\n";);
      LLVM_DEBUG(
          dbgs() << "BasicBlock:\n" << *PrefetchInst2->getParent() << "\n";);
      LLVM_DEBUG(dbgs() << "Function:\n" << *F << "\n";);
      (void) PrefetchInst2;
    }

    // Remove the original DL
    DL->eraseFromParent();

    LLVM_DEBUG({
                 dbgs() << "F:\t" << F->getName() << "()\n" << *F << "\n";
                 if (!verifyFunction(F, "Function verification failed after "
                                        "insertPrefetchInst"))
                   return false;
                 dbgs() << "After generating prefetch intrinsic(s): "
                        << F->getName() << "()\n" << *F << "\n";
               });

    return true;
  };

  // Clone from DL function:
  // This process begins with a full clone of the function where DL resides.
  // (E.g. ProbeTT()).
  // This clone creates the initial prefetch function. It is subject to a
  // sequence of optimizations until it is fully optimized and becomes the final
  // prefetch function.
  //
  assert(DLCandidates.size() == 1 && "Expect Single DL Function");
  Function *DLFunction = DLCandidates[0];
  LLVM_DEBUG(dbgs() << "DLFunction: " << DLFunction->getName() << "()\n";);
  PrefetchFunction = CloneDLFunction(DLFunction);
  if (!PrefetchFunction) {
    LLVM_DEBUG(dbgs() << "Clone DL Function failed\n";);
    return false;
  }
  LLVM_DEBUG(dbgs() << "PrefetchFunction: " << *PrefetchFunction << "\n";);

  // Cleanup clones to make it toward the prefetch function
  if (!CleanupClone(PrefetchFunction)) {
    LLVM_DEBUG(dbgs() << "CleanupClone() failed\n";);
    return false;
  }

  // More cleanups:
  // - cleanup any store into non-local storage (global or argument)
  // - cleanup arguments and return
  if (!SimplifyPrefetchAgain(PrefetchFunction)) {
    LLVM_DEBUG(dbgs() << "SimplifyPrefetchAgain() failed\n";);
    return false;
  }

  // Insert prefetch instruction(s)
  if (!InsertPrefetchInsts(PrefetchFunction)) {
    LLVM_DEBUG(dbgs() << "InsertPrefetchInsts() failed\n";);
    return false;
  }

  LLVM_DEBUG({
               if (!verifyFunction(PrefetchFunction,
                                   "Function verification failed after "
                                   "IPOPrefetcher::createPrefetchFunction(void)\n"))
                 return false;
               dbgs() << "After IPOPrefetcher::createPrefetchFunction(void): \n"
                      << PrefetchFunction->getName() << "()\n"
                      << *PrefetchFunction << "\n";
             });

  return true;
}

// Create a call to the prefetch function inside each host function.
bool IPOPrefetcher::insertPrefetchFunction(void) {
  LLVM_DEBUG({ dumpPrefetchPositions(); });
  IRBuilder<> Builder(M.getContext());

  for (auto Pair: PrefetchPositions) {
    Function *F = Pair.first;
    auto &PosDesVec = Pair.second;
    Function *Caller = F;

    for (auto Pos: PosDesVec) {
      if (!Pos.identifyInsertPosition(Caller)) {
        LLVM_DEBUG(dbgs() << "Fail to identify prefetch insert position in "
                          << F->getName() << "()\n";);
        return false;
      }
      Instruction *InsertPos = Pos.getInsertPos();
      assert(InsertPos && "Expect a valid InsertPos");

      // move to nextI, prepare to insert
      BasicBlock::iterator It(InsertPos);

      // create a call to PrefetchFunction
      Builder.SetInsertPoint(&*It);

      SmallVector<Value *, 4> Args;
      const unsigned NumArgs = PrefetchFunction->arg_size();
      unsigned ArgCount = 0;
      for (Argument &Arg : Caller->args()) {
        Args.push_back(&Arg);
        if (++ArgCount == NumArgs)
          break;
      }

      CallInst *CI = Builder.CreateCall(PrefetchFunction, Args);
      CI->setCallingConv(PrefetchFunction->getCallingConv());

      // Check the newly created call to PrefetchFunction:
      LLVM_DEBUG(dbgs() << *CI << "\n";);
    }

    // Check the entire revised caller:
    LLVM_DEBUG({
                 dbgs() << *Caller << "\n";
                 if (!verifyFunction(F, "Function verification failed after "
                                        "InsertPrefetchFunction()"))
                   return false;
               });
  }

  return true;
}

// Legacy pass manager implementation
class IntelIPOPrefetchWrapperPass : public ModulePass {
public:
  static char ID;
  IntelIPOPrefetchWrapperPass() : ModulePass(ID) {
    initializeIntelIPOPrefetchWrapperPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<PostDominatorTreeWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();

    AU.addPreserved<TargetLibraryInfoWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<PostDominatorTreeWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }

  bool runOnModule(Module &M) override {
    auto LookupDomTree = [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };

    auto LookupPostDomTree = [this](Function &F) -> PostDominatorTree & {
      return this->getAnalysis<PostDominatorTreeWrapperPass>(F).getPostDomTree();
    };

    auto GetTLI = [this](Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    if (skipModule(M) || !EnableIPOPrefetch) {
      LLVM_DEBUG(dbgs() << "IPO Prefetch disabled\n";);
      return false;
    }

    auto WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();

    IPOPrefetcher Impl(M, GetTLI, LookupDomTree, LookupPostDomTree, WPInfo);
    return Impl.run(M);
  }
};

} // End anonymous namespace

char IntelIPOPrefetchWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(IntelIPOPrefetchWrapperPass,
                      "intel-ipoprefetch", "Intel IPO Prefetch",
                      false, false)
  INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
  INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
  INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_END(IntelIPOPrefetchWrapperPass,
                    "intel-ipoprefetch",
                    "Intel IPO Prefetch", false, false)

namespace llvm {

ModulePass *createIntelIPOPrefetchWrapperPass() {
  return new IntelIPOPrefetchWrapperPass();
}

PreservedAnalyses IntelIPOPrefetchPass::run(Module &M,
                                            ModuleAnalysisManager &AM) {
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto LookupDomTree = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };
  auto LookupPostDomTree = [&FAM](Function &F) -> PostDominatorTree & {
    return FAM.getResult<PostDominatorTreeAnalysis>(F);
  };
  auto GetTLI = [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };

  if (!EnableIPOPrefetch) {
    LLVM_DEBUG(dbgs() << "IPO Prefetch disabled\n";);
    return PreservedAnalyses::all();
  }
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  IPOPrefetcher Impl(M, GetTLI, LookupDomTree, LookupPostDomTree, WPInfo);
  bool Changed = Impl.run(M);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // End namespace llvm
