//===---------- Intel_CallTreeCloning.cpp - Call Tree Cloning -------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file implements call tree-based cloning.
//
// An example of call tree-based cloning at depth 3:
//  caller -> foo -> bar -> zoo
//
// void caller(...) {
//   foo(2, 5);
//   foo(0, 1);
//     ->
//     void foo(int x, int y) {
//       bar((y+x)*2);
//       bar(10);
//         ->
//         void bar(int a) {
//           zoo(a, a+1);
//             ->
//             void zoo(int m, int n) {
//               for (int i = 0; i < 2*m; i++) {
//                 for (int j = 0; j < n/2; j++) {...}
//               }
//             }
//         }
//     }
// }
// ->
// void caller() {
//   foo_2_5();
//     ->
//     void foo_2_5() {
//       bar_14();
//         ->
//         void bar_14() {
//           zoo_14_15();
//             ->
//             void zoo_14_15() {
//               for (int i = 0; i < 28; i++) {
//                 for (int j = 0; j < 7; j++) { ... }
//               }
//             }
//         }
//       bar_10();
//         ->
//         void bar_10() {
//           zoo_10_11();
//             ->
//             void zoo_10_11() {
//               for (int i = 0; i < 20; i++) {
//                 for (int j = 0; j < 5; j++) { ... }
//               }
//             }
//         }
//     }
//   foo_0_1();
//     -> ...
// }
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_CallTreeCloning.h"
#include "llvm/Transforms/IPO/Utils/Intel_IPOUtils.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/PassSupport.h"

#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/OptBisect.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <bitset>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace llvm;

#define PASS_NAME_STR "call-tree-clone"

namespace {
const char *PASS_NAME = PASS_NAME_STR;
const char *PASS_DESC =
    "clone functions in call trees specializing by constant parameter sets of "
    "the calls at the top of the tree";
} // namespace

#define DEBUG_TYPE PASS_NAME_STR

STATISTIC(NumCTCClones, "Number of clone function(s) created");
STATISTIC(NumCTCClonedCalls, "Number of call(s) to a clone");
STATISTIC(NumPostCTCConstFold, "Number of constant(s) fold after CTC");
STATISTIC(NumPostCTCCallsiteReplace,
          "Number of callsite(s) replaced after CTC");
STATISTIC(NumPostCTCMVFCreate,
          "Number of multi-version function(s) created after CTC");

#ifndef NDEBUG
#define DBG(x) x
#define DBG_LAST_PARAM(x) , x
#define DBG_PARAM(x) x,
#else
#define DBG(x)
#define DBG_LAST_PARAM(x)
#define DBG_PARAM(x)
#endif // NDEBUG

// Option to disable call-tree cloning pass
static cl::opt<bool> DisableCallTreeCloning(
    PASS_NAME_STR "-disable", cl::init(false), cl::ReallyHidden,
    cl::desc("disable call-tree cloning and multiversioning"));

// Controls the maximum depth of a call tree for the call tree-based IP cloning
static cl::opt<unsigned>
    CTCloningMaxDepth(PASS_NAME_STR "-max-depth", cl::init(3), cl::ReallyHidden,
                      cl::desc("maximum depth of cloned call tree"));

static cl::opt<unsigned>
    CTCloningMaxClones(PASS_NAME_STR "-max-clones", cl::init(1024),
                       cl::ReallyHidden,
                       cl::desc("maximum number of cloned functions"));

// Maximum number of direct callsites allowed for CallTreeClone
static cl::opt<unsigned> CTCloningMaxDirectCallSiteCount(
    PASS_NAME_STR "-max-direct-callsites", cl::init(5120), cl::ReallyHidden,
    cl::desc("maximum allowed number of direct callsites in linked module"));

// Allows to specify "seed" functions and their parameter sets profitable to
// clone directly from the command line bypassing the cost-model.
// !!! NOTE: specification of parameter sets from the command line is
// !!!       unreliable since compiler may change parameters (e.g. remove some
// !!!       after constant propagation)
static cl::list<std::string> CCloneSeeds(
    PASS_NAME_STR "-seed", cl::ReallyHidden,
    cl::desc("clone seed specification - function name followed by "
             "':'-separated parameter index sets, e.g.: 'fname:0,1:1:2,3,4'"));

// controls verbosity of the debug output
static cl::opt<unsigned>
    CTCloningDbgLevel(PASS_NAME_STR "-dbg", cl::init(5), cl::ReallyHidden,
                      cl::desc("debug output verbosity level"));

static cl::opt<unsigned> CTCloningMaxIRSize(
    PASS_NAME_STR "-max-ir-size", cl::init(1024), cl::ReallyHidden,
    cl::desc("don't clone a function if the number of LLVM IR instructions "
             "exceeds this threshold"));

static cl::opt<bool> CTCloningLeafsOnly(
    PASS_NAME_STR "-leafs-only", cl::init(false), cl::ReallyHidden,
    cl::desc("don't clone functions containing non-intrinsic calls"));

// *** Options below control the behaviors of the function multiversioner ***

// Enable MultiVersion transformation. Default is ON.
static cl::opt<bool>
    EnableMV(PASS_NAME_STR "-do-mv", cl::init(true), cl::ReallyHidden,
             cl::desc("option to enable multi-version transformation"));

// Max Expected # of arguments in a function that may enable Multi-Version (MV)
// transformation for 2-variable clones
static cl::opt<unsigned> MV2VarCloneFMaxExpectedArgs(
    PASS_NAME_STR "-mv-2varclonef-max-expected-args", cl::init(9),
    cl::ReallyHidden,
    cl::desc("Max Expected # of arguments in a function that may enable "
             "Multi-Version (MV) transformation for 2-variable clones"));

// Min Expected # of arguments in a function that may enable Multi-Version (MV)
// transformation for 2-variable clones
static cl::opt<unsigned> MV2VarCloneFMinExpectedArgs(
    PASS_NAME_STR "-mv-2varclonef-min-expected-args", cl::init(8),
    cl::ReallyHidden,
    cl::desc("Min Expected # of arguments in a function that may enable "
             "Multi-Version (MV) transformation for 2-variable clones"));

// Min Expected # of integer arguments in a function that may enable
// Multi-Version (MV) transformation for 2-variable clones
static cl::opt<unsigned> MV2VarCloneFMinNumIntArgs(
    PASS_NAME_STR "-mv-2varclonef-min-num-int-args", cl::init(5),
    cl::ReallyHidden,
    cl::desc("Min Expected # of integer arguments in a function that may enable"
             " Multi-Version (MV) transformation for 2-variable clones"));

// Max Expected # of integer arguments in a function that may enable
// Multi-Version (MV) transformation for 2-variable clones
static cl::opt<unsigned> MV2VarCloneFMaxNumIntArgs(
    PASS_NAME_STR "-mv-2varclonef-max-num-int-args", cl::init(6),
    cl::ReallyHidden,
    cl::desc("Max Expected # of integer arguments in a function that may enable"
             " Multi-Version (MV) transformation for 2-variable clones"));

// Max Expected # of pointer arguments in a function that may enable
// Multi-Version (MV) transformation for 2-variable clones
static cl::opt<unsigned> MV2VarCloneFMaxPtrArgs(
    PASS_NAME_STR "-mv-2varclonef-max-ptr-args", cl::init(4), cl::ReallyHidden,
    cl::desc(
        "Max Expected # of pointer arguments in a function that may enable "
        "Multi-Version (MV) transformation for 2-variable clones"));

// Min Expected # of pointer arguments in a function that may enable
// Multi-Version (MV) transformation for 2-variable clones
static cl::opt<unsigned> MV2VarCloneFMinPtrArgs(
    PASS_NAME_STR "-mv-2varclonef-min-ptr-args", cl::init(2), cl::ReallyHidden,
    cl::desc(
        "Min Expected # of pointer arguments in a function that may enable "
        "Multi-Version (MV) transformation for 2-variable clones"));

// Max Expected # of double pointer arguments in a function that may enable
// Multi-Version (MV) transformation for 2-variable clones
static cl::opt<unsigned> MV2VarCloneFMaxDblPtrArgs(
    PASS_NAME_STR "-mv-2varclonef-max-dlbptr-args", cl::init(1),
    cl::ReallyHidden,
    cl::desc(
        "Max Expected # of pointer arguments in a function that may enable "
        "Multi-Version (MV) transformation for 2-variable clones"));

// Min Expected # of double pointer arguments in a function that may enable
// Multi-Version (MV) transformation for 2-variable clones
static cl::opt<unsigned> MV2VarCloneFMinDblPtrArgs(
    PASS_NAME_STR "-mv-2varclonef-min-dlbptr-args", cl::init(0),
    cl::ReallyHidden,
    cl::desc(
        "Max Expected # of pointer arguments in a function that may enable "
        "Multi-Version (MV) transformation for 2-variable clones"));

// Expected # of arguments in a function that may enable Multi-Version (MV)
// transformation for 1-variable clones
static cl::opt<unsigned> MV1VarCloneFExpectedArgs(
    PASS_NAME_STR "-mv-1varclonef-expected-args", cl::init(9), cl::ReallyHidden,
    cl::desc("Expected # of arguments in a function that may enable "
             "Multi-Version (MV) transformation for 1-variable clones"));

// Min Expected # of integer arguments in a function that may enable
// Multi-Version (MV) transformation for 11variable clones
static cl::opt<unsigned> MV1VarCloneFMinNumIntArgs(
    PASS_NAME_STR "-mv-1varclonef-min-num-int-args", cl::init(5),
    cl::ReallyHidden,
    cl::desc("Min Expected # of integer arguments in a function that may enable"
             " Multi-Version (MV) transformation for 2-variable clones"));

// Max Expected # of integer arguments in a function that may enable
// Multi-Version (MV) transformation for 2-variable clones
static cl::opt<unsigned> MV1VarCloneFMaxNumIntArgs(
    PASS_NAME_STR "-mv-1varclonef-max-num-int-args", cl::init(6),
    cl::ReallyHidden,
    cl::desc("Max Expected # of integer arguments in a function that may enable"
             " Multi-Version (MV) transformation for 2-variable clones"));

// Expected # of pointer arguments in a function that may enable Multi-Version
// (MV) transformation for 1-variable clones
static cl::opt<unsigned> MV1VarCloneFPtrArgs(
    PASS_NAME_STR "-mv-1varclonef-ptr-args", cl::init(4), cl::ReallyHidden,
    cl::desc("Expected # of pointer arguments in a function that may enable "
             "Multi-Version (MV) transformation for 1-variable clones"));

// Expected # of double pointer arguments in a function that may enable
// Multi-Version (MV) transformation for 1-variable clones
static cl::opt<unsigned> MV1VarCloneFDblPtrArgs(
    PASS_NAME_STR "-mv-1varclonef-dlbptr-args", cl::init(1), cl::ReallyHidden,
    cl::desc(
        "Expected # of double pointer arguments in a function that may enable "
        "Multi-Version (MV) transformation for 1-variable clones"));

static cl::opt<unsigned> MVMaxValueSetSize(
    PASS_NAME_STR "-mv-max-valueset-size", cl::init(2), cl::ReallyHidden,
    cl::desc("Maximum number of values over which a specific "
             "formal can be cloned. For example, we may be able to prove that "
             "the formal can have values {16, 8, 4, 2} but we only want to "
             "clone over 2 values, and so we may choose {16, 8}"));

static cl::opt<bool> MVBypassCollectionForLITTestOnly(
    PASS_NAME_STR "-mv-bypass-coll-for-littest", cl::init(false),
    cl::ReallyHidden,
    cl::desc("Allow to bypass collection in MultiVersion (MV) transformation."
             "This is specifically designed to demonstrate a LIT test case. "
             "This flag should be off at all other times."));

#define DBGX(n, x) LLVM_DEBUG(if (n <= CTCloningDbgLevel) { x; })

// Conduct Module Verifications:
// - verify the LLVM IR in the given Module is properly constructed
// - verify the debug info in the Module is well maintained
#ifndef NDEBUG
static void doVerification(bool ModuleChanged, Module &M) {
  if (!ModuleChanged)
    return;

  // Do a sanity check of the IR after the Call-Tree Cloning optimization
  // to catch any potential problem early.
  bool BrokenDebugInfo = false;
  if (verifyModule(M, &llvm::errs(), &BrokenDebugInfo))
    report_fatal_error("Module verifier failed after Call-tree cloning "
                       "(with Multiversioning) on Module: " +
        M.getName() + "()\n");

  // Do a sanity check of the debug info on the IR after the Call-Tree Cloning.
  // Force an assert if debug info is broken.
  assert(!BrokenDebugInfo &&
      "Invalid debug info found after Call-Tree Cloning Pass\n");
}
#endif

// Get a loop's bottom-test: an ICmpInst in form of: ICmp IV op UB
static ICmpInst *getLoopBottomTest(Loop *L) {
  auto ExitB = L->getExitingBlock();
  if (ExitB==nullptr)
    return nullptr;

  auto BI = dyn_cast<BranchInst>(ExitB->getTerminator());
  if (!BI || !BI->isConditional())
    return nullptr;

  auto ICmp = dyn_cast_or_null<ICmpInst>(BI->getCondition());
  return ICmp;
}

namespace {
// Estimations:
// - number of formal parameters in a function
const int EST_PARAM_LIST_SIZE = 8;
// - max depth of a call tree to be cloned (should be > CTCloningMaxDepth)
const int EST_CLONED_CALL_TREE_DEPTH = 8;
// - average number of call sites per function
const int EST_NUM_CALL_SITES_PER_FUNC = 8;
// - number of Value* instances in actual parameter "formula"
const int EST_ACT_PARAM_DEP_TREE_SIZE = 32;

// Represents a set of parameter indices
class ParamIndSet : public SmallBitVector {
public:
  ParamIndSet() {}
  ParamIndSet(const ParamIndSet &S) : SmallBitVector(S) {}
  ParamIndSet(const SmallBitVector &V) : SmallBitVector(V) {}
  ParamIndSet(size_t N) : SmallBitVector(N) {}

  ParamIndSet &set(unsigned Idx) {
    if (Idx >= size())
      resize(Idx + 1);

    SmallBitVector::set(Idx);
    return *this;
  }

  // Check: does ParamIndSet have Idx ON?
  bool haveIndex(const unsigned Idx) const {
    if (Idx >= size())
      return false;
    return this->operator[](Idx);
  }

#ifndef NDEBUG
  std::string toString() const {
    const unsigned Size = size();
    if (Size > 128)
      return "{too big}"; // too big to print

    std::ostringstream S;
    S << "{" << size() << "|";
    unsigned Cnt = 0;

    for (unsigned I = 0; I < Size; ++I)
      if (this->operator[](I)) {
        if (++Cnt > 1)
          S << " ";
        S << I;
      }
    S << "}";
    return S.str();
  }

  LLVM_DUMP_METHOD void dump() const { llvm::dbgs() << toString(); }
#endif // NDEBUG
};     // namespace

// ParamIndSet "less" comparator for STL containers
struct ParamIndSetLess {
  bool operator()(const ParamIndSet &Ps0, const ParamIndSet &Ps1) const {
    unsigned Size0 = Ps0.size();
    unsigned Size1 = Ps1.size();

    if (Size0!=Size1)
      return Size0 < Size1;

    // if they are exactly the same: false
    if (Ps0==Ps1)
      return false;

    assert((Size0==Size1) && "expect equal size\n");

    ParamIndSet Ps(Ps0);
    Ps ^= Ps1;
    // the most significant bit which is different:
    // (SmallBitVector::find_last is 1-based: 64-countLeadingZeros(bits) -
    // make it 0-based)
    int LastPos = Ps.find_last();
    assert(((LastPos >= 0) && (LastPos < (int) Size0)) &&
        "equal-size bitvectors can't have negative diff position\n");

    return Ps1[LastPos];
  }
};

// Check that Value* is either a ConstantInt, or an Argument, or can be traced
// back to either a ConstantInt or an Argument.
//
// All checkArgOrConst(.) functions are similar in checking ConstantInt or
// Argument, except each has a different starting point: Value*,
// BinaryOperator*, PHINODE*, or CastInt*.
//
// ValuePtrSet saves any visited Value*. This helps to recognize any loop
// during the back-tracking process, and can abort tracking when a previously
// visited Value* is encountered the 2nd time.
//
static bool checkArgOrConst(Value *, SmallPtrSetImpl<Value *> &, ParamIndSet &);
static bool checkArgOrConst(BinaryOperator *, SmallPtrSetImpl<Value *> &,
                            ParamIndSet &);
static bool checkArgOrConst(PHINode *, SmallPtrSetImpl<Value *> &,
                            ParamIndSet &);

// Check that both of a given BinaryOperator's operand is either a
// ConstantInt, an Argument, or can be traced to either a ConstantInt or an
// Argument.
static bool checkArgOrConst(BinaryOperator *BO,
                            SmallPtrSetImpl<Value *> &ValuePtrSet,
                            ParamIndSet &Pset) {
  assert(BO && "Expect a valid BinaryOperator *\n");

  for (unsigned I = 0, E = BO->getNumOperands(); I < E; ++I)
    if (!checkArgOrConst(BO->getOperand(I), ValuePtrSet, Pset))
      return false;

  return true;
}

// Check that each PHINode's incoming value is either a ConstantInt, an
// Argument, or can be traced to either a ConstantInt or an Argument.
static bool checkArgOrConst(PHINode *Phi, SmallPtrSetImpl<Value *> &ValuePtrSet,
                            ParamIndSet &Pset) {
  assert(Phi && "Expect a valid PHINode *\n");

  for (unsigned I = 0, E = Phi->getNumIncomingValues(); I < E; ++I)
    if (!checkArgOrConst(Phi->getIncomingValue(I), ValuePtrSet, Pset))
      return false;

  return true;
}

// Expect V be a ConstantInt, or an Argument, or can ultimately trace back to
// either a ConstantInt or an Argument.
//
// Support the following types only:
// - ConstantInt
// - Argument
// - BinaryOperation
// - PHINode
// - CastInst
//
static bool checkArgOrConst(Value *V, SmallPtrSetImpl<Value *> &ValuePtrSet,
                            ParamIndSet &Pset) {
  assert(V && "Expect a valid Value *\n");

  if (isa<ConstantInt>(V))
    return true;

  if (Argument *Arg = dyn_cast<Argument>(V)) {
    Pset.set(Arg->getArgNo());
    return true;
  }

  if (BinaryOperator *BO = dyn_cast<BinaryOperator>(V))
    return checkArgOrConst(BO, ValuePtrSet, Pset);

  if (CastInst *CI = dyn_cast<CastInst>(V))
    return checkArgOrConst(CI->getOperand(0), ValuePtrSet, Pset);

  // SelectInst: expect at least 1 operand to converge
  if (SelectInst *SI = dyn_cast<SelectInst>(V))
    return checkArgOrConst(SI->getTrueValue(), ValuePtrSet, Pset) ||
        checkArgOrConst(SI->getFalseValue(), ValuePtrSet, Pset);

  // PHINode: check current Phi form any potential cycle
  if (PHINode *Phi = dyn_cast<PHINode>(V))
    if (ValuePtrSet.find(V)==ValuePtrSet.end()) {
      ValuePtrSet.insert(V);
      return checkArgOrConst(Phi, ValuePtrSet, Pset);
    }

  return false;
}

// Check that in a given loop (L):
//
// - BottomTest (CmpInst) exists and is a supported predicate;
// - CmpInst has only 2 operands;
// - UpperBound (UB) ultimately refers to a function's argument or constant
//
static bool checkLoop(Loop *L, ParamIndSet &Pset) {
  assert(L && "Expect a valid Loop *\n");

  ICmpInst *CInst = getLoopBottomTest(L);
  if (!CInst)
    return false;

  if (!CInst->isIntPredicate())
    return false;

  if (CInst->getNumOperands()!=2)
    return false;

  // Check: comparison is <, <= or ==.
  // (since the loop has not been normalized yet)
  ICmpInst::Predicate Pred = CInst->getPredicate();
  if (!(Pred==ICmpInst::ICMP_EQ || Pred==ICmpInst::ICMP_ULT ||
      Pred==ICmpInst::ICMP_ULE || Pred==ICmpInst::ICMP_SLT ||
      Pred==ICmpInst::ICMP_SLE))
    return false;

  // Check: loop's UpperBound refers to a Function's Argument or a constant
  // Note: save all ArgNo during check into Pset.
  llvm::SmallPtrSet<Value *, 16> ValuePtrSet;
  if (!checkArgOrConst(CInst->getOperand(1), ValuePtrSet, Pset))
    return false;

  return true;
}

// Represents a set of constant values of a parameter set - basically maps
// parameter index to its concrete value
class ConstParamVec
    : public SmallVector<const ConstantInt *, EST_PARAM_LIST_SIZE> {
public:
  unsigned numConstants() const {
    unsigned Res = std::count_if(
        begin(), end(), [&](const ConstantInt *C) { return (C!=nullptr); });
    return Res;
  }

  // This function creates a vector of pairs, where the first item in the pair
  // is the position of the formal and the second is a constant value for that
  // formal.
  bool enumPosVal(
      SmallVectorImpl<std::pair<unsigned, ConstantInt *>> &PosValVec) const {
    for (unsigned I = 0, E = size(); I < E; ++I) {
      const ConstantInt *C = this->operator[](I);
      if (C)
        PosValVec.push_back(std::make_pair(I, const_cast<ConstantInt *>(C)));
    }

    return PosValVec.size();
  }

  ParamIndSet getParamIndSet() const {
    const unsigned Size = size();
    ParamIndSet Res(Size);

    for (unsigned I = 0; I < Size; ++I)
      if (this->operator[](I))
        Res.set(I);

    return Res;
  }

#ifndef NDEBUG
  std::string toString() const;
  LLVM_DUMP_METHOD void dump() const { llvm::dbgs() << toString(); }
#endif // NDEBUG
};

// ConstParamVec "less" comparator for STL containers
struct ConstParamVecLess {
  bool operator()(const ConstParamVec &Psv1, const ConstParamVec &Psv2) const {
    const unsigned MinSize = (unsigned) std::min(Psv1.size(), Psv2.size());

    for (unsigned I = 0; I < MinSize; ++I) {
      const ConstantInt *C1 = Psv1[I];
      const ConstantInt *C2 = Psv2[I];

      if ((C1==nullptr)!=(C2==nullptr))
        return C1==nullptr;

      if (C1==nullptr)
        // C2 is nullptr too then
        continue;

      const APInt &Val1 = C1->getValue();
      const APInt &Val2 = C2->getValue();

      if (Val1.getBitWidth()!=Val2.getBitWidth())
        return Val1.getBitWidth() < Val2.getBitWidth();

      if (Val1!=Val2)
        return Val1.slt(Val2);
    }
    bool Psv1Shorter = (MinSize==Psv1.size());
    const ConstParamVec &X = Psv1Shorter ? Psv2 : Psv1;

    for (unsigned I = MinSize; I < (unsigned) X.size(); ++I)
      if (X[I]!=nullptr)
        return Psv1Shorter;

    return false; // equal
  }
};

// "less" comparator for the clone map
struct CloneMapKeyLess {
  using key_t = std::pair<const Function *, ConstParamVec>;

  bool operator()(const key_t &K1, const key_t &K2) const {
    if (K1.first < K2.first)
      return true;

    if (K1.first > K2.first)
      return false;

    ConstParamVecLess CmpSets;
    return CmpSets(K1.second, K2.second);
  }
};

class DCGNode;

// Represents an edge within the detailed call graph
class DCGEdge : std::pair<DCGNode *, DCGNode *> {
public:
  DCGEdge(DCGNode *M, DCGNode *N) : std::pair<DCGNode *, DCGNode *>(M, N) {}
  const DCGNode *src() const { return first; }
  const DCGNode *dst() const { return second; }
  DCGNode *src() { return first; }
  DCGNode *dst() { return second; }

  friend class DetailedCallGraph;

#ifndef NDEBUG
  std::string toString() const;
  LLVM_DUMP_METHOD void dump() const { llvm::dbgs() << toString() << "\n"; }
#endif // NDEBUG
};     // namespace

// Detailed call graph node - based on a call site.
class DCGNode {
public:
  DCGNode() = delete;

  const ArrayRef<DCGEdge *> inEdgesView() const { return InEdges; }
  const ArrayRef<DCGEdge *> outEdgesView() const { return OutEdges; }
  int &cnt() { return Cnt; }
  void resetTraversalState() { cnt() = 0; }
  const CallInst *getCallInst() const { return Impl; }
  CallInst *getCallInst() { return Impl; }
  const Function *getCaller() const { return Impl->getFunction(); }
  Function *getCaller() { return Impl->getFunction(); }
  const Function *getCallee() const { return Impl->getCalledFunction(); }
  Function *getCallee() { return Impl->getCalledFunction(); }

  // Cloning transformation may remove an underlying call instruction from the
  // call graph.
  //
  // Replace it with a call to the cloned function, thus making the node
  // invalid.
  bool isValid() const {
    return Impl && Impl->getParent() && getCaller() && getCallee();
  }

  uint32_t id() const { return Id; }

  friend class DetailedCallGraph;

#ifndef NDEBUG
  std::string toString(bool PrintCallAddr = false) const;
  LLVM_DUMP_METHOD void dump() const { llvm::dbgs() << toString() << "\n"; }
#endif // NDEBUG

protected:
  DCGNode(CallInst *Impl_, uint32_t Id_) : Cnt(0), Impl(Impl_), Id(Id_) {}

private:
  // only friend classes can alter the edge lists:
  SmallVectorImpl<DCGEdge *> *inEdges() { return &InEdges; }
  SmallVectorImpl<DCGEdge *> *outEdges() { return &OutEdges; }

private:
  int Cnt;        // counter field used in traversal algorithms
  CallInst *Impl; // the direct call site
  uint32_t Id;    // node ID

  SmallVector<DCGEdge *, 8> InEdges;
  SmallVector<DCGEdge *, 8> OutEdges;
};

using DCGNodeList = SmallVector<DCGNode *, EST_NUM_CALL_SITES_PER_FUNC>;

struct CompareDCGNodePtr
    : public std::binary_function<DCGNode *, DCGNode *, bool> {
  bool operator()(const DCGNode *lhs, const DCGNode *rhs) const {
    if (lhs==nullptr || rhs==nullptr)
      return lhs < rhs;
    return lhs->id() < rhs->id();
  }
};

struct CompareFuncPtr
    : public std::binary_function<Function *, Function *, bool> {
  bool operator()(const Function *lhs, const Function *rhs) const {
    if (lhs==nullptr || rhs==nullptr)
      return lhs < rhs;
    return lhs->getName().compare(rhs->getName())==-1;
  }
};

// In a "detailed" call graph, nodes are call sites, an edge from node N to node
// M is present if N's callee is M's caller function. It also maps a function to
// nodes where the function is a caller.
class DetailedCallGraph
    : public std::map<const Function *, DCGNodeList, CompareFuncPtr> {
public:
  DetailedCallGraph() : NodeId(0) {}

  static DetailedCallGraph *build(Module &M) {
    DetailedCallGraph *DCG = new DetailedCallGraph();

    // Scan all instructions in the Module and add each CallInst
    // (regardless of whether it is a direct or indirect call) into the
    // CallGraph.
    for (auto &F : M)
      for (auto &I : instructions(F))
        if (auto *CI = dyn_cast<CallInst>(&I))
          DCG->addCallSite(CI);

    return DCG;
  }

  // Search the Call2Nodes map for Function *F
  const DCGNodeList *getNodesWithCallee(const Function *F) const {
    // TODO edges can be "compressed":
    // Nodes are grouped by the callee, e.g. a->b (1), a->b (2) go
    // in group X, a->c (1), a->c (2) go into group Y; there is a
    // single edge between the group X and echo group of b, and
    // between Y and each group of c

    // take any node of f and get all predecessors
    auto Nit = Callee2Nodes.find(F);
    if (Nit==Callee2Nodes.end())
      return nullptr;

    return &Nit->second;
  }

  // Search the inherited std::map (from DetailedCallGraph) for
  // Function *F
  const DCGNodeList *getNodesOf(const Function *F) const {
    auto I = find(F);
    if (I==end())
      return nullptr;

    return &(I->second);
  }

  // Reset the Cnt (counter) to 0 for each Node in Nodes list
  void resetTraversalState() {
    for (auto X : Nodes)
      X.resetTraversalState();
  }

#ifndef NDEBUG
  std::string toString() const;
  LLVM_DUMP_METHOD void dump() const { llvm::dbgs() << toString() << "\n"; }
#endif // NDEBUG

private:
  // grow the call graph with 1 CallInst and 1 or more edge(s)
  void addCallSite(CallInst *I);

private:
  std::list<DCGEdge> Edges;
  std::list<DCGNode> Nodes;
  // maps a function to nodes where it is the callee
  // - maps a function to all DCGNodes (calls) it has
  std::map<const Function *, DCGNodeList> Callee2Nodes;
  uint32_t NodeId;
};

void DetailedCallGraph::addCallSite(CallInst *CI) {
  // assume CI is Caller->Callee call site

  Function *Caller = CI->getParent()->getParent();
  Function *Callee = CI->getCalledFunction();

  // Skip any indirect call or va_arg call
  if (!Caller || !Callee || Caller->isVarArg() || Callee->isVarArg())
    // TODO support vararg functions or indirect calls
    return;

  // create and push a new node into Nodes list
  Nodes.emplace_front(DCGNode(CI, NodeId++));
  DCGNode *L = &Nodes.front();

  // add the new node to A's node list
  // (to caller's list obtained through inherited std::map)
  auto &ANodes = (*this)[Caller];
  ANodes.push_back(L);

  // add edges between Caller's existing callee nodes and L
  // This establishes one-direction sibling relationship over (K -> L).
  auto It = Callee2Nodes.find(Caller);

  if (It!=Callee2Nodes.end())
    for (auto *K : It->second) {
      Edges.emplace_front(DCGEdge(K, L));
      DCGEdge *Edge = &Edges.front();
      K->outEdges()->push_back(Edge);
      L->inEdges()->push_back(Edge);
    }

  // add L to B's caller list
  auto &BCallers = Callee2Nodes[Callee];
  BCallers.push_back(L);

  // add edges between L and all B's nodes
  auto It1 = find(Callee);

  if (It1!=end())
    for (auto *M : It1->second) {
      Edges.emplace_front(DCGEdge(L, M));
      DCGEdge *Edge = &Edges.front();
      L->outEdges()->push_back(Edge);
      M->inEdges()->push_back(Edge);
    }

  DBGX(1, dbgs() << "CALL " << Caller->getName() << " -> " << Callee->getName()
                 << "\n");
}

// Result of backwards parameter mapping
enum ParamMappingResult {
  // 0 actual parameter (set) is not a foldable function(s) of formals
      Params_cannot_fold,
  // 1 actual parameter (set) a potentially foldable function(s) of formals
      Params_foldable,
  // 2 actual parameter (set) is a constant
      Params_constant,
  // 3 actual parameter not yet back-mapped
      Params_unprocessed,
};

// A data dependence tree of an actual parameter flattened as a
// sequence of Value's (BinaryOperator's ConstantInt's,...) in
// normal Polish notation (operation preceeds operands)
class ActualParamFormula
    : public SmallVector<const Value *, EST_ACT_PARAM_DEP_TREE_SIZE> {
public:
  const ConstantInt *evaluate(const ConstParamVec &Formals) const;

  const ConstantInt *asConstantInt() {
    const ConstantInt *Res = dyn_cast<ConstantInt>(this->operator[](0));
    assert((!Res || size()==1) && "garbage in the formula");
    return Res;
  }

#ifndef NDEBUG
  unsigned dumpRec(StringRef Pref, raw_ostream &Os, unsigned Pos, int T) const;
  void print(raw_ostream &Os) const;
  LLVM_DUMP_METHOD void dump() const { print(llvm::dbgs()); }
#endif // NDEBUG

  friend class ParamTform;

private:
  const ConstantInt *
  evaluateRec(const ConstParamVec &Formals, std::list<const Value *> &ExprStack,
              std::list<const Value *>::iterator At,
              DenseMap<const Value *, const ConstantInt *> &Folded) const;
};

#ifndef NDEBUG
raw_ostream &operator<<(raw_ostream &Os, const ActualParamFormula &APF) {
  APF.dumpRec("\n...", Os, 0, 0);
  return Os;
}

// Convert the ParamMappingResult enum to its string representation
StringRef to_string(ParamMappingResult R) {
  switch (R) {
  case Params_cannot_fold: return "cantfold";
  case Params_foldable: return "foldable";
  case Params_constant: return "constant";
  case Params_unprocessed: return "unproc";
  default: return "unknown";
  }
}

// Print a std::set of DCGNode *
void print_node_set(const std::string &Msg,
                    std::set<DCGNode *, CompareDCGNodePtr> Nodes) {
  dbgs() << Msg << "\n";

  for (const auto X : Nodes)
    dbgs() << X->toString() << " ";

  dbgs() << "\n";
}

// Print a SmallVector of DCGNode *
void print_node_vector(const SmallVectorImpl<DCGNode *> &Nodes) {
  for (const auto X : Nodes)
    dbgs() << X->toString() << " ";
  dbgs() << "\n";
}

void print_node_vector(const SmallVectorImpl<DCGNode *> *Nodes) {
  for (const auto X : *Nodes)
    dbgs() << X->toString() << " ";
  dbgs() << "\n";
}

// Print a std::list of Value*
void print_value_list(const std::string &Msg,
                      std::list<const Value *> &ValueList) {
  dbgs() << Msg << "\n";

  for (const Value *V : ValueList) {
    if (V)
      dbgs() << *V << " ";
    else
      dbgs() << " null ";
  }

  dbgs() << "\n";
}
#endif // NDEBUG

using ActualParamFormulas = SmallVector<std::unique_ptr<ActualParamFormula>,
                                        EST_PARAM_LIST_SIZE>;

// Represents transformation(s) from formal parameters to actuals and back
class ParamTform {
public:
  ParamTform() : ParamTform(nullptr) {}
  ParamTform(const DCGNode *Impl_) : Impl(Impl_) {}

  void setDCGNode(const DCGNode *Impl_) {
    assert(!Impl && "node already set");
    Impl = Impl_;
  }

  void copyConstantParams(ConstParamVec &ConstParams) const;
  ParamIndSet getConstantParamInds() const;
  void evaluate(const ConstParamVec &Src, ConstParamVec &Dst) const;

  // 'back-maps' a set of actual parameters act_params to formal
  // parameters form_params through the (reverse) transform defined
  // by the actual SSA dependence chains ('formulas') of each
  // parameter from the Dst: foo(<formal_params>) {
  //   ...
  //   some_call(<act_params>);
  //   ...
  // }
  //
  // The result 'Res' is a set of formal parameter indices, which
  // participate in definitions of actual parameters in 'Dst'.
  //
  ParamMappingResult mapBack(ParamIndSet &Dst, ParamIndSet &Res);

  // Determine whether a given actual parameter of the underlying
  // call can be folded to a constant given that formal parameters
  // it depends on are constants.
  //
  // Returns:
  // - Params_cannot_fold
  //     if the parameters can't be folded
  //
  // - Params_foldable
  //     if it can; 'Res' will contain indices of the formal
  //     parameters
  //
  // - Params_constant
  //     if the parameter is constant
  //
  ParamMappingResult mapBack(int ActParamInd, ParamIndSet &Res);

  const DCGNode *getDCGNode() const { return Impl; }

  // get: # of arguments of the caller function
  unsigned getInputsSize() const { return Impl->getCaller()->arg_size(); }

  // get: # of arguments of the callee function
  unsigned getOutputsSize() const { return Impl->getCallee()->arg_size(); }

#ifndef NDEBUG
  std::string toString() const;
  LLVM_DUMP_METHOD void dump(void) const { llvm::dbgs() << toString(); }
#endif // NDEBUG

private:
  // the DCGraph edge defining this transform
  const DCGNode *Impl;

  // records the status of back-mapping for each of the actual
  // parameters
  SmallVector<ParamMappingResult, EST_PARAM_LIST_SIZE> ActParamStatus;

  // maps a formal parameter index to actuals that depend on it
  SmallVector<ParamIndSet, EST_PARAM_LIST_SIZE> Form2Act;

  // maps an actual parameter index to formals it depends on
  SmallVector<ParamIndSet, EST_PARAM_LIST_SIZE> Act2Form;

  // "formulas" of calculating actual parameters based on formal
  // parameters; each formula is a complete list of all transitive
  // data dependencies of the corresponding actual parameter laid
  // out in DFS order
  ActualParamFormulas ActParamFormulas;
};

// A set of parameter index sets.
class SetOfParamIndSets : public std::set<ParamIndSet, ParamIndSetLess> {
public:
  // Derives a set of parameter indices A set to a constant from the
  // given constant parameter vector. Then checks if there is subset
  // in this set of sets such that it is enclosed by A.
  bool hasSetCoveredBy(const ConstParamVec &ConstParams) const {
    ParamIndSet ConstParamsInds = ConstParams.getParamIndSet();

    for (auto S : *this) {
      if (S.size() < ConstParamsInds.size())
        S.resize(ConstParamsInds.size());

      if ((ConstParamsInds & S)==S)
        return true;
    }
    return false;
  }

  // Check: any ParamIndxSet has Idx?
  bool haveIndex(const unsigned Idx) {
    for (auto S : *this)
      if (S.haveIndex(Idx))
        return true;
    return false;
  }

#ifndef NDEBUG
  std::string toString() const;
  LLVM_DUMP_METHOD void dump() const { llvm::dbgs() << toString() << "\n"; }
#endif // NDEBUG
};

// Groups various information about parameter data flow through a
// call graph node; it is decoupled from the node itself so that the
// detailed call graph could be used externally (some day).
class DCGNodeParamFlow {
public:
  DCGNodeParamFlow(const DCGNode *Impl)
      : Tform(Impl), Marked(false), Visited(false) {}
  DCGNodeParamFlow() : Marked(false), Visited(false) {}

  // Getters + setters:
  void setMarked(bool Val, bool Force = false) {
    assert((Val!=Marked || Force) && "already set");
    Marked = Val;
  }

  void setVisited(bool val) { Visited = val; }
  bool isMarked() const { return Marked; }
  bool isVisited() const { return Visited; }

  SetOfParamIndSets &liveIn() { return LiveIn; }
  const SetOfParamIndSets &liveIn() const { return LiveIn; }
  SetOfParamIndSets &liveOut() { return LiveOut; }
  const SetOfParamIndSets &liveOut() const { return LiveOut; }
  SetOfParamIndSets &killedOut() { return KilledOut; }
  const SetOfParamIndSets &killedOut() const { return KilledOut; }
  ParamTform &tform() { return Tform; }
  const ParamTform &tform() const { return Tform; }

private:
  SetOfParamIndSets LiveIn;
  SetOfParamIndSets LiveOut;
  SetOfParamIndSets KilledOut;
  ParamTform Tform;
  // used externally
  bool Marked;
  bool Visited;

public:
#ifndef NDEBUG
  std::string toString() const;
  LLVM_DUMP_METHOD void dump() const { llvm::dbgs() << toString() << "\n"; }
#endif // NDEBUG
};

#ifndef NDEBUG
void printSeeds(
    const std::string &Msg,
    std::map<DCGNode *, SetOfParamIndSets, CompareDCGNodePtr> &Seeds) {

  // print msg
  dbgs() << Msg << "<" << Seeds.size() << ">\n";

  // print each Seed in the Seeds map
  for (auto &Seed : Seeds) {
    DCGNode *Node = Seed.first;
    const SetOfParamIndSets &Psets = Seed.second;
    dbgs() << Node->toString() << "->" << Psets.toString() << "\n";
  }

  dbgs() << "\n";
}

void printSeeds(
    const std::string &Msg,
    std::map<Function *, SetOfParamIndSets, CompareFuncPtr> &Seeds) {

  // Print msg
  dbgs() << Msg << ": <" << Seeds.size() << ">\n";

  // Print each Seed in the Seeds Map
  unsigned Count = 0;
  for (auto &Seed : Seeds) {
    Function *F = Seed.first;
    const SetOfParamIndSets &Psets = Seed.second;
    dbgs() << Count++ << " : " << F->getName() << " -> " << Psets.toString()
           << "\n";
  }

  dbgs() << "\n";
}

void print_FunctionMap(const std::string &Msg, std::map<Function *, bool> &Map,
                       bool PrintEachNewLine = true) {
  dbgs() << Msg << "<" << Map.size() << ">:\n";
  unsigned Count = 0;
  for (auto &Item : Map) {
    Function *F = Item.first;
    dbgs() << Count++ << "  " << F->getName();
    if (PrintEachNewLine)
      dbgs() << "\n";
    else
      dbgs() << "\t";
  }
  dbgs() << "\n";
}
#endif // NDEBUG

// Maps detailed call graph nodes to their parameter data flow information
class DCGParamFlows : public std::map<const DCGNode *, DCGNodeParamFlow> {
public:
  DCGNodeParamFlow *getOrCreate(const DCGNode *N) {
    auto res = &(this->operator[](N));

    if (!res->tform().getDCGNode())
      res->tform().setDCGNode(N);

    return res;
  }

  const DCGNodeParamFlow *get(const DCGNode *N) const {
    auto I = find(N);
    return I!=end() ? &I->second : nullptr;
  }

  DCGNodeParamFlow *get(const DCGNode *N) {
    auto I = find(N);
    return I!=end() ? &I->second : nullptr;
  }

#ifndef NDEBUG
  std::string toString() const {
    std::ostringstream S;
    S << "DCGParamFlows: <" << size() << ">{\n";

    for (auto &Item : *this) {
      S << "[ " << Item.first->toString() << " -> " << Item.second.toString()
        << " ]\n";
    }

    return S.str();
  }

  LLVM_DUMP_METHOD void dump() const { llvm::dbgs() << toString() << "\n"; }
#endif // NDEBUG
};

class Analyses {
public:
  std::function<LoopInfo &(Function &)> getLoopInfo;

  Analyses(decltype(getLoopInfo) GetLI) : getLoopInfo(GetLI) {}
};

// Cost model interface whose task is to assess whether there are some formal
// parameters sets in function F which, if set to constants, would generate
// profitable clone(s).
class CTCCostModel {
public:
  virtual SetOfParamIndSets assess(Function &F) = 0;
};

// Cost model used for debugging, engaged when the user specifies seeds from the
// command line.
class CTCDebugCostModel : public CTCCostModel {
public:
  template<typename It> CTCDebugCostModel(It Beg, It End);
  virtual SetOfParamIndSets assess(Function &F);

#ifndef NDEBUG
  std::string toString(void) const {
    std::ostringstream S;
    S << "CTCDebugCostModel: <" << SeedsFromCmdLine.size() << ">{\n";

    for (auto &Item : SeedsFromCmdLine) {
      S << Item.first << " -> " << Item.second.toString() << "\n";
    }

    return S.str();
  }

  LLVM_DUMP_METHOD void dump(void) const { llvm::dbgs() << toString(); }
#endif

private:
  std::map<std::string, SetOfParamIndSets> SeedsFromCmdLine;
};

// Real cost model based on loop presence.
class CTCLoopBasedCostModel : public CTCCostModel {
public:
  CTCLoopBasedCostModel(Analyses &AnlsP) : Anls(AnlsP) {}

  // For given function tells whether there are sets of formal parameters which
  // give significant performance benefits if replaced with constants. Returns
  // all such sets. E.g. for the function below these could be two sets - <0,1>
  // and <1,2>
  //
  // void foo(int a, int b, int c) {
  //   for (int i = 0; i < a + b; i++) {
  //     for (int j = 0; j < b + c; j++) {
  //       ...
  //     }
  //   }
  // }
  virtual SetOfParamIndSets assess(Function &F);

protected:
  /// Retrieves some statistics about given function \p F and fills in the
  /// output parameters:
  /// - \p IRSize - size of the IR  in IR instructions and
  /// - \p HasCalls - whether the function contains non-intrinsic calls.
  void getFunctionIRStats(const Function &F, size_t &IRSize, bool &HasCalls);

  /// Looks for loops within given function and applies the overloaded function
  /// below to each.
  void gatherParamDepsForFoldableLoopBounds(Function &F,
                                            SetOfParamIndSets &Psets);

  /// Checks if bounds of a given loop \p L are foldable functions of formal
  /// parameters, in which case constructs a set of the parameter indices and
  /// adds it to the output \p Pset parameter. \p Scev is used to determine the
  /// dependence on the formals.
  void gatherParamDepsForFoldableLoopBounds(Loop *L, SetOfParamIndSets &Psets);

private:
  Analyses &Anls;
};

using Call2ClonedFunc = DenseMap<CallInst *,
                                 std::pair<Function *, ConstParamVec>>;

// Instance of this class maintains registry of cloned functions to avoid
// cloning of functions with the same constant parameter sets
using CloneRegistry = std::map<std::pair<const Function *, ConstParamVec>,
                               Function *, CloneMapKeyLess>;

#ifndef NDEBUG
void print_CloneRegistry(const std::string &Msg, CloneRegistry &Clones) {
  dbgs() << Msg << "\n";
  unsigned Count = 0;

  for (auto &CloneItem : Clones) {
    auto Key = CloneItem.first;
    Function *ClonedF = CloneItem.second;

    // print counter:
    dbgs() << Count++ << "  ";

    // dump key:
    dbgs() << Key.first->getName() << " : ";
    Key.second.dump();

    dbgs() << " -> ";

    // dump value:
    dbgs() << ClonedF->getName();
    dbgs() << "\n";
  }

  dbgs() << "\n";
}
#endif // NDEBUG

// updates back-mapping result for a parameter set based on a result for a new
// element of the set
inline ParamMappingResult merge_result(ParamMappingResult SetResult,
                                       ParamMappingResult ElemResult) {
  assert(ElemResult!=Params_unprocessed && "");
  return static_cast<ParamMappingResult>(std::min(SetResult, ElemResult));
}

void ParamTform::copyConstantParams(ConstParamVec &ConstParams) const {
  unsigned N = getOutputsSize(); // N: number of arguments on callee
  ConstParams.resize(N);
  assert(ActParamFormulas.size()==N && "bad number of formulas");

  // scan each available index position in callee's argument:
  // convert to ConstantInt* if possible
  for (unsigned I = 0; I < N; ++I) {
    // no parallel access to ActParamFormulas, so get() is safe
    ActualParamFormula *F = ActParamFormulas[I].get();
    const ConstantInt *C = F ? F->asConstantInt() : nullptr;
    assert((!C || !ConstParams[I] ||
        C->equalsInt(ConstParams[I]->getValue().getZExtValue())) &&
        "tform inconsistent with input params");
    if (C)
      ConstParams[I] = C;
  }
}

inline ParamIndSet ParamTform::getConstantParamInds() const {
  ParamIndSet PIS;
  unsigned N = getOutputsSize(); // get number of arguments intcallee
  PIS.resize(N);

  // scan each argument-index position in callee:
  // if the matching index position is a ConstantInt*, turn the
  // matching index bit ON a ParamIndexSet, and return this
  // ParamIndexSet in the end.
  for (unsigned I = 0; I < N; ++I) {
    // no parallel access to ActParamFormulas, so get() is safe
    ActualParamFormula *F = ActParamFormulas[I].get();

    if (F && F->asConstantInt())
      PIS.set(I);
  }

  return PIS;
}

// Evaluate individual positions in Src ConstParamVec, and store the
// result into Dst ConstParamVec
void ParamTform::evaluate(const ConstParamVec &Src, ConstParamVec &Dst) const {
  // set Dst's number of bits to the number of arguments in callee
  Dst.resize(getOutputsSize());
  assert(ActParamFormulas.size()==Dst.size() && "bad number of formulas");
  DBGX(2, dbgs() << "\n");

  // Scan each bit-index position:
  // if the index position has a Formula, evaluate it
  for (unsigned I = 0, E = Dst.size(); I < E; ++I)
    if (auto &Formula = ActParamFormulas[I]) {
      DBGX(2, dbgs() << "Evaluating param " << I << "\n");
      Dst[I] = Formula->evaluate(Src);
    }

  // enrich the result with the constants this call site adds:
  copyConstantParams(Dst);
}

ParamMappingResult ParamTform::mapBack(ParamIndSet &Dst, ParamIndSet &Res) {
  // Early bail-out check:
  // if any position (on ActParamStatus) is Params_cannot_fold on
  // over-sized Dst return Params_cannot_fold
  for (unsigned I = 0, E = ActParamStatus.size(); I < E; ++I) {
    auto S = ActParamStatus[I];

    if (S==Params_cannot_fold && Dst.size() > I && Dst[I])
      // can't fold one of the parameters in the set =>
      // can't fold the entire set
      return Params_cannot_fold;
  }

  ParamMappingResult PMR = Params_unprocessed;
  const DCGNode &N = *getDCGNode();
  const Function *Caller = N.getCaller();
  const Function *Callee = N.getCallee();
  assert(!Caller->isVarArg() && !Callee->isVarArg() &&
      "vararg functions should've been skipped in the detailed "
      "call graph");
  size_t ResSize = Caller->arg_size();
  size_t DstSize = Callee->arg_size();

  if (ActParamStatus.size()==0) {
    assert(DstSize > 0 && "unexpected empty parameter set");
    ActParamStatus.resize(DstSize, Params_unprocessed);
    assert(ActParamFormulas.size()==0 && "");
    ActParamFormulas.resize(DstSize);
    Dst.resize(DstSize);
  }
  if (Res.size()==0)
    Res.resize(ResSize);

  assert(ActParamStatus.size()==DstSize && "Dst param set size mismatch");
  assert(Dst.size()==DstSize && "Dst param set size mismatch 1");
  assert(Res.size()==ResSize && "Res param set size mismatch");
  assert(ActParamFormulas.size()==DstSize && "formula set size mismatch");

  for (unsigned I = 0, E = Dst.size(); I < E; ++I) {
    if (!Dst[I])
      continue; // this actual parameter is not of interest

    auto Status = ActParamStatus[I];

    if (Status==Params_cannot_fold)
      // early bail-out
      return Params_cannot_fold;

    if (Status==Params_unprocessed) {
      // find how actual parameter maps to formals
      Status = mapBack(I, Res);
      ActParamStatus[I] = Status;
    } else if (Status==Params_foldable) {
      // parameter has been mapped back - just fill in from Act2Form cache
      const ParamIndSet &IparamDeps = Act2Form[I];

      if (Res.size() < IparamDeps.size())
        Res.resize(IparamDeps.size());

      Res |= IparamDeps;
    }
    // else the parameter is constant and no action is needed because it does
    // not add any dependencies on the caller's formal parameters (does not
    // extend the incoming 'Res' set)

    // dependencies for this actual has already been determined
    PMR = merge_result(PMR, Status);
  }

  return PMR;
}

// Tells whether given instruction can potentially be
// constant-folded. Used this function to discard unfoldable cases early.
//
// what it does:
// - check if the given instruction's operand is actually a valid
// Binary Opcode.
//
// TODO: should really be a part of ConstantFolding
// TODO: binary ops only for now - add everything supported by
// ConstantFolding
bool might_constant_fold_inst(const Instruction *I) {
  auto Opc = I->getOpcode();
  return ((Opc >= Instruction::BinaryOpsBegin) &&
      (Opc < Instruction::BinaryOpsEnd));
}

ParamMappingResult ParamTform::mapBack(int ActParamInd, ParamIndSet &Res) {
  // build data dependence chain for a given actual parameter and
  // see if its leaves are either constants or formal parameters of
  // the caller.

  // DFS work list:
  SmallVector<const Value *, EST_ACT_PARAM_DEP_TREE_SIZE/2> WrkList;

  // Create the container for resulting "formula" - dependencies in DFS order:
  assert(ActParamFormulas[ActParamInd]==nullptr);
  ActParamFormulas[ActParamInd] = std::make_unique<ActualParamFormula>();
  auto &Formula = ActParamFormulas[ActParamInd];

  // maps a value to its DFS number to detect cycles, also serves as 'visited'
  // attribute tracker
  DenseMap<const Value *, unsigned> V2N;
  unsigned DfsNum = 0;
  // seed the worklist with the actual argument
  const CallInst *CI = getDCGNode()->getCallInst();
  Value *Arg = CI->getArgOperand(ActParamInd);
  WrkList.push_back(Arg);
  ParamMappingResult Ret = Params_unprocessed;
  DBGX(2, dbgs() << "\n...map back actual param " << ActParamInd);

  while (!WrkList.empty()) {
    // values get popped from the work list in DFS order
    const Value *V = WrkList.pop_back_val();
    Formula->push_back(V);
    // assign DFS number
    auto I = V2N.insert(std::make_pair(V, DfsNum++));
    bool Met = !(I.second);
    const Instruction *Inst = dyn_cast<Instruction>(V);

    if (!Inst) {
      // not an instruction - A leaf in the dependence tree
      if (const Argument *A = dyn_cast<Argument>(V)) {
        unsigned ArgNum = A->getArgNo();
        // formal parameter contributes to this actual parameter's value
        Res.set(ArgNum);
        Ret = merge_result(Ret, Params_foldable);
        // update mapping:
        if (Form2Act.size()==0)
          Form2Act.resize(getInputsSize());

        if (Act2Form.size()==0)
          Act2Form.resize(getOutputsSize());

        assert(Form2Act.size()==Res.size() && "param set size mismatch");
        auto &FrmSet = Form2Act[ArgNum];
        FrmSet.set(ActParamInd);
        auto &ActSet = Act2Form[ActParamInd];
        ActSet.set(ArgNum);
      } else if (isa<ConstantInt>(V)) {
        Ret = merge_result(Ret, Params_constant);
      } else {
        // TODO: handle undefs
        // early bailout
        Ret = Params_cannot_fold;
        break;
      }
      continue;
    } else {
      if (!might_constant_fold_inst(Inst)) {
        // can't fold - early bailout
        Ret = Params_cannot_fold;
        break;
      }
      // don't change Ret, only leafs can tell whether the result is
      // "constant" or "foldable"
    }

    if (Met)
      // this instruction is an operand of more than one other visited - don't
      // recurse, as it was serialized together with its dependencies at the
      // time it was first met
      continue;

    // visit instruction operands; need to push in reverse order so that they
    // are popped and written to the formula in the direct order
    SmallVector<const Value *, 4> Opnds;

    for (const Value *Opnd : Inst->operands()) {
      // check for cycles first (possible only with PHIs)
      auto I = V2N.find(Opnd);

      if ((I!=V2N.end()) && (I->second < DfsNum - 1)) {
        // successor already visited and there is A dependence cycle - bail out
        Ret = Params_cannot_fold;
        break;
      }
      Opnds.push_back(Opnd);
    }
    if (Ret==Params_cannot_fold)
      break;

    std::copy(Opnds.rbegin(), Opnds.rend(), std::back_inserter(WrkList));
  }

  assert(((Ret!=Params_foldable) ||
      ((Res.count() > 0) && (Act2Form[ActParamInd].count() > 0))) &&
      "foldable actual must be a function of formals");

#ifndef NDEBUG
  if (Ret==Params_foldable || Ret==Params_constant)
    DBGX(2, dbgs() << *Formula << "\n");

  DBGX(2, dbgs() << "...RESULT: " << to_string(Ret) << "\n");
#endif // NDEBUG

  if (Ret==Params_cannot_fold)
    // de-allocate formula
    ActParamFormulas[ActParamInd] = nullptr;

  return Ret;
}

const ConstantInt *ActualParamFormula::evaluateRec(
    const ConstParamVec &Formals, std::list<const Value *> &ExprStack,
    std::list<const Value *>::iterator At,
    DenseMap<const Value *, const ConstantInt *> &Folded) const {

  assert(At!=ExprStack.end() && "invalid expr");
  const Value *V = *At;
  assert(V && "invalid expr");

#ifndef NDEBUG
  unsigned Depth = std::distance(ExprStack.begin(), At);
  bool IsArg = false;
#endif // NDEBUG

  // If the current Value *V is an argument: replace it with its
  // matching ConstantInt* value from Formals vector
  if (const Argument *Arg = dyn_cast<Argument>(V)) {
    LLVM_DEBUG(IsArg = true);

    // it is a formal parameter - replace it with a constant if possible
    const ConstantInt *C = Formals[Arg->getArgNo()];
    DBGX(2, dbgs().indent(Depth*2 + 2) << "arg[" << Arg->getArgNo() << "]=");

    if (!C) {
      DBGX(2, dbgs().indent(Depth*2 + 2) << "<null>\n");
      return nullptr;
    }
    *At = C;
    V = C;
    // ... and continue
  }

  // If the current Value *V is already an ConstantInt*: do nothing
  if (const ConstantInt *Res = dyn_cast<ConstantInt>(V)) {
    // a constant already - nothing to do
    DBGX(2, (IsArg ? dbgs().indent(0) : dbgs().indent(Depth*2 + 2))
        << *Res << "\n");
    return Res;
  }

  // Search the Folded map for V, and return the ContantInt* if
  // found (from Folded Map)
  auto It = Folded.find(V);

  if (It!=Folded.end())
    // operation is input to multiple other operations and has been folded
    return It->second;

  // Otherwise: try to fold the operation;
  const BinaryOperator *Op = dyn_cast<BinaryOperator>(V);

  if (!Op)
    // TODO: only binary operations are supported right now
    return nullptr;

  assert(std::distance(At, ExprStack.end()) > 2 && "2 operands expected");
  const ConstantInt *X0 =
      evaluateRec(Formals, ExprStack, std::next(At), Folded);
  // note: 2 evaluations w/o the intervening null check would be unsafe
  if (!X0)
    return nullptr;

  // the prior evaluation collapsed the dependence subtree of X0 into single
  // constant; hence, the second operand starts at offset 2:
  const ConstantInt *X1 =
      evaluateRec(Formals, ExprStack, std::next(At, 2), Folded);

  if (!X1)
    return nullptr;

  // the constant folding call below does not really change its arguments, so
  // use const_cast to satisfy the contract
  Constant *Opnds[2] = {const_cast<ConstantInt *>(X0),
                        const_cast<ConstantInt *>(X1)};
  const auto &DL = Op->getModule()->getDataLayout();
  ConstantInt *C = dyn_cast<ConstantInt>(
      ConstantFoldInstOperands(const_cast<BinaryOperator *>(Op), Opnds, DL));

  if (C) {
    // successfully folded the operation - replace the operation and its
    // operands on the expression stack with the result
    ExprStack.erase(std::next(At), std::next(At, 3));
    *At = C;
    Folded[Op] = C;
    DBGX(2, dbgs().indent(Depth*2) << *V << " = " << *C << "\n");
  } else
    DBGX(2, dbgs().indent(Depth*2) << *V << " = <null>\n");

  return C;
}

// Call evaluateRec() to evaluate the given ConstParamVec & Formals
const ConstantInt *
ActualParamFormula::evaluate(const ConstParamVec &Formals) const {
  // use list for quick erasure:
  std::list<const Value *> ExprStack;
  std::copy(begin(), end(), std::back_inserter(ExprStack));
  DenseMap<const Value *, const ConstantInt *> Folded;

  // recursively evaluate staring from the first Value:
  return evaluateRec(Formals, ExprStack, ExprStack.begin(), Folded);
}

// Constructs a dummy cost model which does not analyse functions, but
// constructs algorithm seeds from given set of textual specifications. The
// format is described in the CCloneSeeds option.
template<typename It> CTCDebugCostModel::CTCDebugCostModel(It Beg, It End) {
  std::for_each(Beg, End, [&](const std::string &Sstr) {
    StringRef S(Sstr);
    // parse name
    auto P = S.split(':');

    // since the option is hidden, don't bother doing nice error handling
    if (P.second.size()==0) {
      LLVM_DEBUG({
                   errs() << "invalid option: " << Sstr << "\n";
                   report_fatal_error("invalid option",
                                      false /*no crash dump*/);
                 });
    }
    StringRef FuncName = P.first;
    SetOfParamIndSets Psets;

    // parse parameter sets
    do {
      P = P.second.split(":");
      StringRef PsetStr = P.first;

      if (PsetStr.size()==0) {
        LLVM_DEBUG({
                     errs() << "invalid option: " << Sstr << "\n";
                     report_fatal_error("invalid option",
                                        false /*no crash dump*/);
                   });
      }
      ParamIndSet Pset;
      std::pair<StringRef, StringRef> P1;

      do {
        P1 = PsetStr.split(",");
        StringRef NStr = P1.first;
        unsigned N;

        if (NStr.getAsInteger(10, N) || N > 1023) {
          LLVM_DEBUG({
                       errs() << "invalid option: " << Sstr << "\n";
                       report_fatal_error("invalid option",
                                          false /*no crash dump*/);
                     });
        }
        if (N >= Pset.size())
          Pset.resize(N + 1);

        Pset.set(N);
        PsetStr = P1.second;
      } while (P1.second.size() > 0);
      Psets.insert(Pset);
    } while (P.second.size() > 0);

    SeedsFromCmdLine.insert(std::make_pair(FuncName, Psets));
  });
}

SetOfParamIndSets CTCDebugCostModel::assess(Function &F) {
  assert(!SeedsFromCmdLine.empty() && "seeds must have been provided");
  // 'It' references a pair <function name, sets of param index sets>:
  auto It = SeedsFromCmdLine.find(F.getName());

  if (It!=SeedsFromCmdLine.end()) {
    // function/param sets were added from the command line - make it a seed
    // unconditionally; but first check that parameter set is valid as compiler
    // may have constant-propagated and folded some of them
    SetOfParamIndSets ResPsets;

    for (const ParamIndSet &Pset : It->second) {
      if (Pset.size() <= F.arg_size())
        ResPsets.insert(Pset);
      else
        LLVM_DEBUG({
                     errs() << PASS_NAME_STR
                     "WARNING: seed not found in callgraph: "
                         << F.getName() << " " << Pset.toString();
                   });
    }
    return ResPsets;
  }
  return SetOfParamIndSets();
}

SetOfParamIndSets CTCLoopBasedCostModel::assess(Function &F) {
  // apply heuristics to assess profitability
  // 1. (aimed at CMPLRS-45545)
  //   Functions which are:
  //   - small
  //   - w/o non-intrinsic calls inside
  //   - containing loops whose bounds are foldable functions of formals
  SetOfParamIndSets Res;
  size_t IRSize = ULONG_MAX; // be conservative
  bool HasCalls = true;
  getFunctionIRStats(F, IRSize, HasCalls);

#ifndef NDEBUG
  std::string Msg = "";
#endif // NDEBUG

  if (IRSize > CTCloningMaxIRSize) {
    LLVM_DEBUG(std::ostringstream S;
                   S << "LLVM IR too big - " << IRSize;
                   Msg = S.str());
  } else if (HasCalls && CTCloningLeafsOnly) {
    LLVM_DEBUG(Msg = "contains non-intrinsic calls");
  } else {
    gatherParamDepsForFoldableLoopBounds(F, Res);
  }

  LLVM_DEBUG(if (Msg.size() > 0)
               dbgs()
                   << "Skip " << F.getName() << ":" << Msg << "\n";);

  return Res;
}

void CTCLoopBasedCostModel::getFunctionIRStats(const Function &F,
                                               size_t &IRSize, bool &HasCalls) {
  IRSize = 0;
  HasCalls = false;

  LLVM_DEBUG(errs() << "Function: " << F.getName() << "(.)\n;");

  for (const auto &I : instructions(&F)) {
    ++IRSize;

    if (isa<InvokeInst>(&I)) {
      HasCalls = true;
      continue;
    }
    const CallInst *Call = nullptr;

    if (!HasCalls && (Call = dyn_cast<CallInst>(&I))) {
      Function *Callee = Call->getCalledFunction();

      if (Callee && !Callee->isIntrinsic())
        HasCalls = true;
    }
  }
}

void CTCLoopBasedCostModel::gatherParamDepsForFoldableLoopBounds(
    Function &F, SetOfParamIndSets &Psets) {
  LoopInfo &LI = Anls.getLoopInfo(F);
  auto Loops = LI.getLoopsInPreorder();

  for (auto L : Loops)
    gatherParamDepsForFoldableLoopBounds(L, Psets);
}

// Psets is a set of ParamIndSet, each ParmIndSet contains a set of function's
// formal argument index that loop (L)'s upper bound can fold to.
void CTCLoopBasedCostModel::gatherParamDepsForFoldableLoopBounds(
    Loop *L, SetOfParamIndSets &Psets) {

  if (L->isInvalid())
    return;

  // Check: loop L's UpperBound composition is a constant, argument, or can
  // trace to a constant or argument. Formal argument's indexes are saved into
  // Pset
  ParamIndSet Pset;
  if (!checkLoop(L, Pset))
    return;

  if (Pset.count() > 0)
    Psets.insert(Pset);
}

// This class actually implements the call-tree cloner.
// Both CallTreeCloningPass and CallTreeCloningLegacyPass delegate to it.
class CallTreeCloningImpl {
  friend class PostProcessor;
  friend class MultiVersionImpl;

public:
  CallTreeCloningImpl() {}

  bool run(Module &M, Analyses &Anl,
           std::function<const TargetLibraryInfo &(Function &F)> GetTLI,
           PreservedAnalyses &PA);

protected:
  // -count the number of CallInst(s) and InvokeInst(s)
  // -check if it is within the allowed threshold
  bool checkThreshold(Module &M) {
    uint64_t NumCallInst = 0;

    for (auto &F : M.functions())
      for (BasicBlock &BB : F)
        for (Instruction &I : BB)
          // count direct calls only: support both CallInst and InvokeInst
          if (CallBase *CB = dyn_cast<CallBase>(&I))
            if (!CB->isIndirectCall())
              ++NumCallInst;

    LLVM_DEBUG(dbgs() << "NumCallInst:\t" << NumCallInst << "\n");

    if (NumCallInst > CTCloningMaxDirectCallSiteCount) {
      LLVM_DEBUG(dbgs() << "Intel_CallTreeCloning: Potential Call graph too "
                           "large, CallTreeClone pass disabled\n");
      return false;
    }
    return true;
  }

  // The main algorithm - performs bottom-up parameter sets propagation and then
  // top-down parameter sets propagation/evaluation and function cloning.
  bool findAndCloneCallSubtrees(
      DetailedCallGraph *Cgraph,
      std::map<DCGNode *, SetOfParamIndSets, CompareDCGNodePtr> &AlgSeeds,
      CloneRegistry &Clones);

  // Clone a given function 'F' by replacing some of the parameters with a given
  // constants 'ConstParams'. Non-null at position i means i'th parameter is
  // replaced by this constant. 'Call2Clone' maps all callsites within the
  // source function to <Function, <constant parameter set>> pairs. For every
  // callsite a counterpart within the clone function is found, and updated to
  // call the mapped function with constant parameter removed. 'clones' keeps
  // record of cloned functions. The clone is created only if it is not found in
  // this map.
  Function *cloneFunction(Function *F, const ConstParamVec &ConstParams,
                          const Call2ClonedFunc &Call2Clone,
                          CloneRegistry &Clones);

  // The recursive bottom-up pass of the algorithm, which determines clone
  // roots, live-in and live-out parameter sets, parameter transform formulas
  // for detailed call graph nodes met along all paths from seeds to roots
  // (along reverse edges of the graph)
  void findParamDepsRec(DCGNode *Top,
                        std::set<DCGNode *, CompareDCGNodePtr> &CloneRoots,
                        SmallVectorImpl<DCGNode *> *CallStack,
                        DCGParamFlows &Flows);

  // The recursive top-down pass which does actual cloning
  Function *cloneCallSubtreeRec(
      DCGNode *Root, SmallVectorImpl<DCGNode *> *CallStack,
      const ConstParamVec &ConstParams,
      std::map<DCGNode *, SetOfParamIndSets, CompareDCGNodePtr> &Seeds,
      CloneRegistry &Clones, const DCGParamFlows &Flows);
};

// Do post-process cleanup after the recursive Call-Tree Cloning
// finished the module. New opportunities are created. E.g.
// ...
// %10 = shl 4, 2;
// %11 = shl 2, 2;
// %12 = @get_ref(%10, %11) ; a call to a seed function
// ...
//
// Post processor (PP) will turn this into:
// ...
// %10 = shl 4, 2; if dead code, next GVN will clean it up
// %11 = shl 2, 2; same
// %12 = @get_ref(8, 16) ; a call to a seed function with constants on certain
//                         arguments
// ...
//
// and then into:
// ...
// %10 = shl 4, 2; dead code, next GVN will clean it up
// %11 = shl 2, 2; same
// %12 = @get_ref|8.16() ; a call to a cloned function
// ...
class PostProcessor {
public:
  PostProcessor(
      Module &M,
      std::map<Function *, SetOfParamIndSets, CompareFuncPtr> &LeafSeeds,
      CloneRegistry &Clones,
      std::function<const TargetLibraryInfo &(Function &F)> GetTLI,
      CallTreeCloningImpl *CTCI)
      : M(M), LeafSeeds(LeafSeeds), Clones(Clones), DL(M.getDataLayout()),
        GetTLI(std::move(GetTLI)), CTCI(CTCI) {}

  // Run post processing on the Module
  bool run();

private:
  bool doPreliminaryTest(void);
  bool collectPPCallInst(CallInst *);
  bool doCollection(void);

  // do constant folding and seed-function replacement with its matching clone
  bool foldConstantAndReplWithClone(CallInst *&, unsigned);
  bool doTransformation(void);

public:
#ifndef NDEBUG
  std::string toString(void) const;
  LLVM_DUMP_METHOD void dump(void) const { llvm::dbgs() << toString(); }
#endif

private:
  Module &M;
  std::map<Function *, SetOfParamIndSets, CompareFuncPtr> &LeafSeeds;
  CloneRegistry &Clones;
  const DataLayout &DL;
  std::function<const TargetLibraryInfo &(Function &F)> GetTLI;
  CallTreeCloningImpl *CTCI;

  std::map<CallInst *, unsigned> PPCandidates;
  // includes both original and populated seed functions:
  std::map<Function *, bool> ExtSeedFunctions;
  std::map<Function *, bool> ClonedFunctions;
};

struct ConstantIntGreaterThan { // Descent-order comparator
  bool operator()(const ConstantInt *C0, const ConstantInt *C1) const {
    return C0->getSExtValue() > C1->getSExtValue();
  }
};

// All information needed to conduct multi-version transformation for a given
// Function* F.
//
// This includes:
// -F: Function itself
// -Psets: sets of all formals
// -Size: # of LLVM Instructions inside F
// -Param2CloneMap: map (F, ConstParamVec) -> CloneF
// -Pos2ValMap:     map: formal position -> sorted ConstantInt* set
// - bool Gen2VarClones: true if 2-variable clones will be generated
// - bool Gen1VarClones: true if 1-variable clones will be generated
//
struct MVFunctionInfo {
  MVFunctionInfo() : F(nullptr), Size(0) {}

  void set(Function *Fn) {
    if (F && (Fn!=F))
      assert(0 && "Wrong Fn given");
    if (!F)
      F = Fn;
  }

  void set(SetOfParamIndSets &SPsets) {
    for (auto S : SPsets)
      Psets.insert(S);
  }

  void set(ConstParamVec &ConstParams, Function *ClonedF) {
    Param2CloneMap[ConstParams] = ClonedF;
  }

  void set(unsigned Idx, ConstantInt *C) { Pos2ValMap[Idx].insert(C); }
  void set(unsigned S) { Size = S; }
  void setGen2VarC(bool V) { Gen2VarClones = V; }
  void setGen1VarC(bool V) { Gen1VarClones = V; }

#ifndef NDEBUG
  std::string toString(void) const;
  LLVM_DUMP_METHOD void dump(void) const { llvm::dbgs() << toString(); }
#endif

  Function *F;
  SetOfParamIndSets Psets;
  unsigned Size;
  std::map<ConstParamVec, Function *> Param2CloneMap;
  std::map<unsigned, std::set<ConstantInt *, ConstantIntGreaterThan>>
      Pos2ValMap;
  bool Gen2VarClones = false;
  bool Gen1VarClones = false;
};

// Declaration of Multi-version Implementation Class
class MultiVersionImpl {
public:
  MultiVersionImpl(
      Module &M,
      std::map<Function *, SetOfParamIndSets, CompareFuncPtr> &LeafSeeds,
      CloneRegistry &Clones, CallTreeCloningImpl *CTCI)
      : M(M), LeafSeeds(LeafSeeds), Clones(Clones), CTCI(CTCI) {

    // 2VarMV function matcher:
    Match2VarMV = FunctionSignatureMatcher(
        MV2VarCloneFMinExpectedArgs /* MinNumArg:8 */,
        MV2VarCloneFMaxExpectedArgs /* MaxNumArg:9 */,
        MV2VarCloneFMinNumIntArgs,  /* MinNumIntArg: 5 */
        MV2VarCloneFMaxNumIntArgs,  /* MaxNumIntArg: 6 */
        MV2VarCloneFMinPtrArgs /* NumPtrArg0:2 */,
        MV2VarCloneFMaxPtrArgs /* NumPtrArg1:4 */,
        MV2VarCloneFMinDblPtrArgs /* MinNumDoublePtrArgs:0 */,
        MV2VarCloneFMaxDblPtrArgs /* MaxNumDoublePtrArgs:1 */,
        true /* LeafFunc */);

    // 1VarMV function matcher:
    Match1VarMV = FunctionSignatureMatcher(
        MV1VarCloneFExpectedArgs /* MinNumArg:9 */,
        MV1VarCloneFExpectedArgs /* MaxNumArg:9 */,
        MV1VarCloneFMinNumIntArgs,  /* MinNumIntArg: 5 */
        MV1VarCloneFMaxNumIntArgs,  /* MaxNumIntArg: 6 */
        MV1VarCloneFPtrArgs /* NumPtrArg0:4 */,
        MV1VarCloneFPtrArgs /* NumPtrArg1:4 */,
        MV1VarCloneFDblPtrArgs /* MinNumDoublePtrArgs:1 */,
        MV1VarCloneFDblPtrArgs /* MaxNumDoublePtrArgs:1 */,
        true /* LeafFunc */);
  }

  bool run();

private:
  // Collect all function candidates for multi-version transformation
  // - candidates are from cloner's leaf-seed functions;
  // - filter the candidates by function's size, number of desired arguments,
  // etc.
  //   This produces multi-version candidates (MVSeeds).
  bool doCollection(void);

  // Analyze the MVSeeds, and produce:
  // 1. map<Function *, std::set<ConstParamsVec>>: map between
  // OrigF* and its potentially multiple ConstParamsVec(s);
  //
  // 2. map<unsigned idx, std::set<ConstantInt*>: map between
  // OrigF's argument position to all possible constants that position may have;
  //
  bool doAnalysis(void);

  // clone a given function with
  // - an all-empty ConstParams
  // - single-variable clones on each qualified arg position
  bool createAdditionalClones(Function *F);

  // Multi-version code generation:
  //
  // - Generate if_then style LLVM code blocks for 2 given (Arg0,C0) and
  // (Arg1,C1) pairs, as:
  //
  //[C: 1 2-variable clone]
  // if((i_width == C0) && (i_height == C1)) {
  //   matched_clone();
  // }
  //
  //[LLVM: 1 2-variable clone]
  //------------------------------------------------------
  // %cmp = icmp eq i32 %6, 16                        CommonBB
  // %cmp1 = icmp eq i32 %7,
  // 16 %and.cond = and i1 %cmp, %cmp1
  // br i1 %and.cond, label %if.then, label %if.end
  //
  // if.then: ---------------------------------------------
  //
  // tail call void (...) @pixel_avg16x16() #2         ThenBB
  // ret
  //
  // if.end: ---------------------------------------------
  //                                                   MergeBB
  //
  // ---------------------------------------------
  //
  // Note:
  // - Current clause's MergeBB is the CommonBB for the next clause.
  //
  bool doCodeGenMV2VarClone(Function *F, unsigned Pos0, ConstantInt *C0,
                            unsigned Pos1, ConstantInt *C1,
                            BasicBlock *&CommonBB, BasicBlock *&ThenBB,
                            BasicBlock *&MergeBB);

  // Generate a "ret" instruction;
  //
  // Note:
  // it can be a simple return of void or it can also be a return with a proper
  // value;
  //
  void doCodeGenRet(CallInst *CI, IRBuilder<> &Builder);

  // Generate
  // - a call to the original clone (with empty ConstParamsVector)
  //   E.g. mc_chroma's original clone is mc_chroma|_._..._.()
  //
  // - a "ret" instruction
  bool doCodeGenOrigClone(Function *F, BasicBlock *CallBB);

  // Create additional runtime values by interpolating existing values. E.g.
  // [Before] ValSet:{     16,     8}
  //
  // [After]  ValSet:{ 20, 16, 12, 8}
  //                   ^       ^
  // 20 and 12 are new values created through interpolation.
  bool interpolateForRTValues(
      std::set<ConstantInt *, ConstantIntGreaterThan> &ValSet);

  // - Generate if_then style LLVM code blocks for a single (Arg,C) pair, as:
  //
  //[C: 1 1-variable clone]
  // if(i_width == C0)
  //   matched_clone();
  // }
  //
  //[LLVM: 1 1-variable clone]
  // ------------------------------------------------------------
  // %cmp = icmp eq i32 %6, 16                    CommonBB Block
  // br i1 %cmp, label %if.then, label %if.end
  //
  // ------------------------------------------------------------
  // if.then:                                     ThenBB Block
  // tail call void (...) @pixel_avg16x_() #2
  // ret
  //
  // if.end: ----------------------------------------------------
  //                                              MergeBB Block
  //
  // ------------------------------------------------------------
  //
  bool doCodeGenMV1VarClone(Function *F, unsigned Pos, ConstantInt *C,
                            BasicBlock *&CommonBB, BasicBlock *&ThenBB,
                            BasicBlock *&MergeBB);

  // Main entry for Multi-Version Code generation for Function F:
  //- wipe F clean;
  //- code generation for 2-variable clones;
  //- code generation for 1-variable clones;
  //- code generation for the original function;
  bool doCodeGen(Function *F);

  bool doTransformation(void);

public:
#ifndef NDEBUG
  std::string toString(void) const;
  LLVM_DUMP_METHOD void dump(void) const { llvm::dbgs() << toString(); }
#endif

private:
  Module &M;
  std::map<Function *, SetOfParamIndSets, CompareFuncPtr> &LeafSeeds;
  CloneRegistry &Clones;
  CallTreeCloningImpl *CTCI;

  std::map<Function *, SetOfParamIndSets, CompareFuncPtr> MVSeeds;
  std::map<Function *, MVFunctionInfo, CompareFuncPtr> MVFIMap;

  // 2VarMV function matcher:
  FunctionSignatureMatcher Match2VarMV;
  // 1VarMV function matcher:
  FunctionSignatureMatcher Match1VarMV;

}; // namespace

} // end of anonymous namespace

#ifndef NDEBUG
std::string PostProcessor::toString(void) const {
  std::ostringstream S;

  // Data to print: std::map<CallInst *, unsigned> PPCandidates;
  S << "PPCandidates<" << PPCandidates.size() << ">:\n";
  unsigned Count = 0;

  for (auto &Item : PPCandidates) {
    S << "  " << Count++ << " ";
    CallInst *CI = Item.first;

    // print CallInst info:
    Function *Caller = CI->getParent()->getParent();
    Function *Callee = CI->getCalledFunction();
    S << "Caller: " << Caller->getName().str()
      << " -> Callee: " << Callee->getName().str() << "\t";

    // print Position info:
    unsigned Pos = Item.second;
    for (unsigned I = 0, E = sizeof(unsigned); I < E; ++I)
      if (Pos & (1 << I))
        S << I << ",";

    S << "\n";
  }

  return S.str();
}

std::string MVFunctionInfo::toString(void) const {
  std::ostringstream S;
  S << "MVFunctionInfo: \t";

  // print: Function * F:
  if (F)
    S << F->getName().str() << "()\n";
  else
    S << "null\n";

  // print: SetOfParamIndSets Psets;
  S << "Psets:<" << Psets.size() << ">:" << Psets.toString() << "\n";

  // print: unsigned Size;
  S << "Size: " << Size << "\n";

  // std::map<ConstParamVec, Function *> Param2CloneMap;
  S << "Param2CloneMap:<" << Param2CloneMap.size() << ">\n";
  for (auto &Pair : Param2CloneMap) {
    ConstParamVec ConstParams = Pair.first;
    Function *ClonedF = Pair.second;
    S << ConstParams.toString() << " -> " << ClonedF->getName().str() << "\n";
  }

  // print: std::map<unsigned, SmallSet<ConstantInt *, 4>> Pos2ValMap;
  S << "Pos2ValMap:<" << Pos2ValMap.size() << ">\n";
  for (auto &Pair : Pos2ValMap) {
    unsigned Index = Pair.first;
    auto &ConstSet = Pair.second;
    S << Index << " -> ";

    for (ConstantInt *C : ConstSet) {
      APInt Val = C->getValue();
      S << Val.toString(10, true) << "\t";
    }
    S << "\n";
  }

  // print: Gen2VarClones, Gen1VarClones
  S << std::boolalpha << "Gen2VarClones: " << Gen2VarClones << ",  "
    << "Gen1VarClones: " << Gen1VarClones << "\n";

  return S.str();
}

std::string MultiVersionImpl::toString(void) const {
  std::string S;

  for (auto &P : MVFIMap) {
    const MVFunctionInfo &MVFI = P.second;
    S += MVFI.toString();
  }
  return S;
}
#endif

namespace llvm {
// The call-tree cloning transformation ModulePass
class CallTreeCloningLegacyPass : public ModulePass {
public:
  CallTreeCloningLegacyPass();
  bool runOnModule(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  StringRef getPassName() const override {
    return "Call-Tree Cloning (with MultiVersioning)";
  }

public:
  static char ID;
};
} // end of namespace llvm

bool CallTreeCloningImpl::run(
    Module &M, Analyses &Anls,
    std::function<const TargetLibraryInfo &(Function &F)> GetTLI,
    PreservedAnalyses &PA) {

  if (!checkThreshold(M)) {
    LLVM_DEBUG(dbgs() << "Disable CallTreeClone pass due to potential "
                         "callgraph's size over threshold\n");
    return false;
  }

  // build the detailed call graph first
  std::unique_ptr<DetailedCallGraph> Cgraph(DetailedCallGraph::build(M));

  DBGX(1, dbgs() << "--- Call graph:\n");
  DBGX(1, Cgraph->dump());

  // now find "seed" nodes for the algorithm based on cost model and
  // filters - call sites with callees being functions profitable to clone
  std::map<DCGNode *, SetOfParamIndSets, CompareDCGNodePtr> AlgSeeds;
  std::unique_ptr<CTCCostModel> CM;
  std::map<Function *, SetOfParamIndSets, CompareFuncPtr> LeafSeeds;

  if (!CCloneSeeds.empty())
    CM = std::make_unique<CTCDebugCostModel>(CCloneSeeds.begin(),
                                             CCloneSeeds.end());
  else
    CM = std::make_unique<CTCLoopBasedCostModel>(Anls);

  LLVM_DEBUG(dbgs() << "--- Seeds for cloning:\n");

  for (auto &F : M) {
    if (!F.hasExactDefinition() || F.isDeclaration() || F.isIntrinsic())
      continue;

    SetOfParamIndSets Psets = CM->assess(F);

    if (Psets.empty())
      continue;

    if (const DCGNodeList *callers = Cgraph->getNodesWithCallee(&F)) {
      LLVM_DEBUG(dbgs() << "  " << F.getName() << Psets.toString() << "\n");

      for (auto *N : *callers) {
        if (N->getCallInst()->cannotDuplicate()) {
          LLVM_DEBUG(dbgs() << "  skip " << N->toString()
                            << ": can't duplicate call\n");
          continue;
        }
        assert(AlgSeeds.find(N)==AlgSeeds.end() && "seed duplication");
        AlgSeeds[N] = Psets;
        LLVM_DEBUG(dbgs() << "  added seed: " << N->toString() << "/"
                          << Psets.toString() << "\n");
      }

      // Collect leaf seed functions
      if (IPOUtils::isLeafFunction(F)) {
        LLVM_DEBUG(dbgs() << "leaf-seeds: " << F.getName() << Psets.toString()
                          << "\n");
        LeafSeeds[&F] = Psets;
      }
    }
  }

  LLVM_DEBUG({ printSeeds("Seeds: ", AlgSeeds); });
  LLVM_DEBUG({ printSeeds("LeafSeeds: ", LeafSeeds); });

  CloneRegistry Clones; // records all clones, needed for post processing (PP)
  bool CTCResult = findAndCloneCallSubtrees(Cgraph.get(), AlgSeeds, Clones);
  if (!CTCResult)
    return false;

  // Do Post Processing Cleanup:
  PostProcessor PostProc(M, LeafSeeds, Clones, GetTLI, this);
  bool PPResult = PostProc.run();

  // Check + Do Multi-version (MV) transformation:
  if (!EnableMV)
    return PPResult;

  MultiVersionImpl MV(M, LeafSeeds, Clones, this);
  bool MVResult = MV.run();

  // Return true if at least 1 of the CTC, PP or MV is triggered and modified
  // the module
  return CTCResult || PPResult || MVResult;
}

bool llvm::CallTreeCloningLegacyPass::runOnModule(Module &M) {
  if (skipModule(M) || (CTCloningMaxDepth==0) || DisableCallTreeCloning)
    return false;

  Analyses Anls([&](Function &F) -> LoopInfo & {
    return getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
  });
  auto GetTLI = [this](Function &F) -> TargetLibraryInfo & {
    return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  };
  PreservedAnalyses PA;
  CallTreeCloningImpl Impl;
  bool ModuleChanged = Impl.run(M, Anls, GetTLI, PA);

  // Verify Module if there is any change on the LLVM IR
#ifndef NDEBUG
  doVerification(ModuleChanged, M);
#endif

  return ModuleChanged;
}

namespace {
// Change the call site, so that it calls a cloned version of the
// function with specialized-to-constant and reduced-number actual
// argument list.
CallInst *specializeCallSite(CallInst *Call, Function *Clone,
                             const ParamIndSet &ConstArgs) {
  unsigned NumArgs = Call->getNumArgOperands();
  unsigned NumConstArgs = ConstArgs.count();
  SmallVector<Value *, EST_PARAM_LIST_SIZE> NewArgs;
  NewArgs.reserve(NumArgs - NumConstArgs);
  unsigned NumNewArgs = 0;
  AttributeList NewAttrs;
  AttributeList Attrs = Call->getAttributes();
  LLVMContext &Ctx = Clone->getContext();

  for (auto I : {AttributeList::ReturnIndex, AttributeList::FunctionIndex})
    if (Attrs.hasAttributes(I))
      NewAttrs = NewAttrs.addAttributes(Ctx, I, Attrs.getAttributes(I));

  for (unsigned I = 0; I < NumArgs; ++I) {
    auto *Arg = Call->getArgOperand(I);

    if ((I >= ConstArgs.size()) || !ConstArgs[I]) {
      NewArgs.push_back(Arg);

      if (Attrs.hasParamAttrs(I))
        NewAttrs = NewAttrs.addParamAttributes(Ctx, NumNewArgs,
                                               Attrs.getParamAttributes(I));
      ++NumNewArgs;
    }
  }
  assert(NewArgs.size()==(NumArgs - NumConstArgs) && "arg num mismatch");
  assert(!Call->hasOperandBundles() && "TODO: support operand bundles");
  CallInst *NewCall = CallInst::Create(Clone, NewArgs, "", Call);
  NewCall->setCallingConv(Call->getCallingConv());
  NewCall->setAttributes(NewAttrs);
  if (MDNode *MD = Call->getMetadata(LLVMContext::MD_dbg))
    NewCall->setMetadata(LLVMContext::MD_dbg, MD);
  Call->replaceAllUsesWith(NewCall);
  Call->dropAllReferences();
  Call->removeFromParent();
  ++NumCTCClonedCalls;

  return NewCall;
}

} // end of anonymous namespace

void CallTreeCloningImpl::findParamDepsRec(
    DCGNode *Top, std::set<DCGNode *, CompareDCGNodePtr> &CloneRoots,
    SmallVectorImpl<DCGNode *> *CallStack, DCGParamFlows &Flows) {

  DBGX(1, dbgs().indent(CallStack->size()*2) << Top->toString());
  DCGNodeParamFlow *Flow = Flows.getOrCreate(Top);

  if (Flow->isMarked())
    // skip marked nodes
    return;

  // call stack is CTCloningMaxDepth size max, so 'find' is
  // constant-time
  if (std::find(CallStack->begin(), CallStack->end(), Top)!=
      CallStack->end()) {
    // the edge creates a cycle - skip it
    DBGX(1, dbgs() << " CYCLE\n");
    Flow->setMarked(true, true /*Force*/);
    return;
  }
  Flow->setVisited(true);
  CallStack->push_back(Top);

  ParamTform &Tform = Flow->tform();
  bool NewLiveInMet = false;

  // back-map live-out sets through the (reverse) transform
  for (auto CurLiveOut : Flow->liveOut()) {
    CurLiveOut.resize(Tform.getOutputsSize());
    ParamIndSet CurLiveIn; // back-mapping result
    ParamMappingResult Res = Tform.mapBack(CurLiveOut, CurLiveIn);

    DBGX(1, dbgs() << " " << CurLiveOut.toString() << "<-");

    if (Res==Params_foldable) {
      // current parameter set can be constant-folded from caller's
      // formals if they are known constants
      auto InsRes = Flow->liveIn().insert(CurLiveIn);
      NewLiveInMet = NewLiveInMet || InsRes.second;
      DBGX(1, dbgs() << CurLiveIn.toString());
    } else if (Res==Params_constant) {
      // can clone current call tree starting from 'Top', as all the
      // parameters down to the bottom callsite are constant-foldable

      if (Top->cnt()==0) {
        // Clone root reachability
        // -----------------------
        //     X
        //     |
        //     Y
        //    / \
        //   A   B
        // if X and Y in the detailed call graph picture above are both found to
        // be roots (edge direction is down) then only X must remain as root.
        // More generally, all roots reachable from other roots must be removed.
        // Otherwise, top-down pass from X (or across X from some upper node)
        // won't clone Y's subtree. The code below keeps the set of clone roots
        // satisfying that property (none of the roots can be reacheable from
        // other roots along any path from a root to a seed).

        // Top->cnt() being non-zero means this node is on a path to some root;
        // here it is zero, which means Top is not on the path to any root yet.
        //
        // actions:
        // (1) mark all the node in the CallStack as such, since Top is
        // identified as a root
        std::for_each(CallStack->begin(), CallStack->end(),
                      [&](DCGNode *N) -> void { N->cnt() = 1; });

        // (2) record current node as a clone root
        CloneRoots.insert(Top);
        // see step (3) in the caller
        DBGX(1, dbgs() << "C");
      } else {
        DBGX(1, dbgs() << "C(met)");
      }
    } else {
      // parameters in current set can't be folded from caller's ones
      assert(Res==Params_cannot_fold && "invalid result");
      DBGX(1, dbgs() << "!");
      Flow->killedOut().insert(CurLiveOut);
    }
  }
  DBGX(1, dbgs() << " " << Tform.toString());

  // See if it is allowed to ascend the call tree even if there are callers
  // (less expensive tests first).
  //
  // Factors which can prevent that:
  // - call tree depth limit is reached;
  // - none of the parameter sets "survive" the edge's transform;
  //   E.g.
  //   the transform can't fold constant input parameters into the output
  //   parameters belonging to the set.
  // - the input parameter sets are constants (cloning can proceed).

  // - 'depth' test:
  bool Stop = CallStack->size() >= CTCloningMaxDepth;
  DBGX(1, if (Stop)
    dbgs() << " MAX DEPTH");

  if (!Stop) {
    // - 'survival'/'constant' test:
    Stop = Flow->liveIn().size()==0;
    DBGX(1, if (Stop)
      dbgs() << " EMPTY LIVE IN");
  }
  if (!Stop) {
    Stop = !NewLiveInMet;
    DBGX(1, if (Stop)
      dbgs() << " NO NEW LIVE IN");
  }
  DBGX(1, llvm::dbgs() << "\n");

  if (!Stop) {
    // recurse through the input nodes to the work list, but skip
    // those which don't get any new input for the back-propagation
    for (const auto &E : Top->inEdgesView()) {
      DCGNode *pred = E->src();
      DCGNodeParamFlow *PredFlow = Flows.getOrCreate(pred);
      bool NewLiveOut = false;

      for (auto &LiveIn : Flow->liveIn()) {
        auto &KilledOut = PredFlow->killedOut();

        if (KilledOut.find(LiveIn)!=KilledOut.end())
          // this set will be killed - not worth trying
          continue;

        auto InsRes = PredFlow->liveOut().insert(LiveIn);
        NewLiveOut = NewLiveOut || InsRes.second;
      }
      if (NewLiveOut)
        // insertion occurred - the predecessor has new data to
        // back-propagate - recurse through it
        findParamDepsRec(pred, CloneRoots, CallStack, Flows);
    }
  }
  // the node has been processed - pop it off the stack
  CallStack->pop_back();
}

// Starting from given "seed" <node,parameter set> pairs, propagates the
// parameter sets up the call tree until "root" nodes where all parameters of
// interest are fold-able to constants. Then goes back from roots to seeds along
// all possible paths calculating input constant parameter sets and cloning the
// callee functions.
//
// Algorithm of finding all paths from 'start' to leaves in a directed acyclic
// graph is DFS w/o tracking the 'visited' property.
//
bool CallTreeCloningImpl::findAndCloneCallSubtrees(
    DetailedCallGraph *Cgraph,
    std::map<DCGNode *, SetOfParamIndSets, CompareDCGNodePtr> &AlgSeeds,
    CloneRegistry &Clones) {

  // parameter data-flow information for each call graph node
  DCGParamFlows Flows;

  // storage of nodes which start call-trees leading to compile-time defined
  // parameter sets in any of the seed nodes
  std::set<DCGNode *, CompareDCGNodePtr> CloneRoots;

  DBGX(1, dbgs() << "\n--- Bottom-up pass (!-can't fold, C-constant)\n\n");

  // bottom-up pass finding paths to root calls with constant actual parameters
  // and parameter transformations performed by each involved call site
  for (auto &AlgSeed : AlgSeeds) {
    DCGNode *SeedNode = AlgSeed.first;
    const SetOfParamIndSets &SeedLiveOut = AlgSeed.second;

    DBGX(1, dbgs() << "SeedNode: " << SeedNode->toString() << "\n";);
    DBGX(1, dbgs() << "SeedLiveOut: " << SeedLiveOut.toString() << "\n";);

    DCGNodeParamFlow *Flow = Flows.getOrCreate(SeedNode);
    Flow->liveOut().insert(SeedLiveOut.begin(), SeedLiveOut.end());

    // current call tree (call stack) ending with a function found in current
    // seed_node
    SmallVector<DCGNode *, EST_CLONED_CALL_TREE_DEPTH> CallStack;

    // recursively/inter-procedurally find formal parameter data dependencies
    // until constants are met or the upward recursion is stopped otherwise
    findParamDepsRec(SeedNode, CloneRoots, &CallStack, Flows);
  }

  DBGX(1, print_node_set("--- Raw clone roots:", CloneRoots));

  // Step (3) of the "Clone root reachability" maintenance described in
  // findParamDepsRec: bust the clone roots which have a predecessor marked as
  // being on a way to another root. This handles the situation as shown below:
  //
  //   X----A
  //  / \  /
  // Z   \/
  //  \  /\
  //   \/  \
  //   Y----B
  //
  // If X and Z are both seeds, X could be cloned unnecessarily w/o this step

  auto CloneIter = CloneRoots.begin();
  while (CloneIter!=CloneRoots.end()) {
    DCGNode *N = *CloneIter;
    ++CloneIter; // Explicit increment here, because we may erase "N".
    for (const auto &E : N->inEdgesView()) {
      assert((E->dst()==N) && "bad call graph edge");
      DCGNode *N1 = E->src();
      if (N1->cnt() > 0) {
        CloneRoots.erase(N);
        break;
      }
    }
  }

  LLVM_DEBUG(print_node_set("--- Clone roots:", CloneRoots));
  LLVM_DEBUG({ print_CloneRegistry("Clones: ", Clones); });

  // At this point we have a detailed call graph annotated with:
  // - information about "clone roots" (see above) to start cloning with
  // - formal->actual parameter transformation function at some of the nodes,
  //   which have been visited during the bottom-up traversal.
  //
  // With this information, do subgraph cloning for each clone root, and record
  // all cloned functions in the clone registry.

  // the number of functions cloned by this pass
  unsigned NumClones = 0;

  // records all cloned functions
  // CloneRegistry Clones;

  LLVM_DEBUG(dbgs() << "\n--- Top-down pass\n\n");

  // Edges which introduce cycles (isMarked() == true) are skipped in
  // cloneCallSubtreeRec below, so it is safe not to track visited nodes.
  // This leads to all *paths *from roots to seeds having been traversed, rather
  // than just all nodes visited.
  for (auto *Root : CloneRoots) {
    DBGX(1, dbgs() << "ROOT " << Root->toString() << ": ");

    // fetch constant actual parameters of the call-site 'Root' represents
    ConstParamVec ConstParams;
    DCGNodeParamFlow *Flow = Flows.get(Root);
    Flow->tform().copyConstantParams(ConstParams);
#ifndef NDEBUG
    // make sure 'evaluate' yields the same result for a clone root:
    ConstParamVec ConstParams1;
    ConstParamVec ConstParams1Img;
    ConstParams1.resize(Flow->tform().getInputsSize());
    Flow->tform().evaluate(ConstParams1, ConstParams1Img);
    assert(ConstParams1Img==ConstParams && "evaluate() error");
#endif // NDEBUG
    ParamIndSet ConstParamsInds = ConstParams.getParamIndSet();
    DBGX(1, dbgs() << " live_out=" << ConstParamsInds.toString()
                   << " consts=" << ConstParams.toString() << "\n");

    SmallVector<DCGNode *, EST_CLONED_CALL_TREE_DEPTH> CallStack;
    // recursively clone the subgraph with these constants
    Function *Clone = cloneCallSubtreeRec(Root, &CallStack, ConstParams,
                                          AlgSeeds, Clones, Flows);

    if (Clone) {
      // cloning successful - update the call site
      CallInst *Call = Root->getCallInst();
      specializeCallSite(Call, Clone, ConstParamsInds);
    }
  }

  NumClones = static_cast<unsigned>(Clones.size()) - NumClones;
  LLVM_DEBUG(dbgs() << "Number of clones: " << NumClones << "\n");

  return NumClones > 0;
}

Function *CallTreeCloningImpl::cloneCallSubtreeRec(
    DCGNode *Root, SmallVectorImpl<DCGNode *> *CallStack,
    const ConstParamVec &ConstParams,
    std::map<DCGNode *, SetOfParamIndSets, CompareDCGNodePtr> &Seeds,
    CloneRegistry &Clones, const DCGParamFlows &Flows) {

#ifndef NDEBUG
  unsigned Depth = CallStack->size();
#endif // NDEBUG
  Call2ClonedFunc Call2Clone;
  LLVM_DEBUG(dbgs().indent(Depth*2)
                 << Root->toString() << " PARAMS:" << ConstParams.toString());
  DBGX(1, dbgs() << " || ");

  if (std::find(CallStack->begin(), CallStack->end(), Root)!=
      CallStack->end()) {
    // the edge creates a cycle - skip it
    DBGX(1, dbgs() << " CYCLE\n");
    return nullptr;
  }
  auto It = Seeds.find(Root);

  if (It!=Seeds.end()) {
    DBGX(1, dbgs() << "SEED ");
    // seed node, stop recursion and clone function
    // ...
    // but first see if constant parameters entirely cover at least one of the
    // seed parameter sets
    bool HasLiveSet = It->second.hasSetCoveredBy(ConstParams);
    DBGX(1, dbgs() << (HasLiveSet ? "" : "No LIVE set! "));
    Function *Clone = HasLiveSet ? cloneFunction(Root->getCallee(), ConstParams,
                                                 Call2Clone, Clones)
                                 : nullptr;
    LLVM_DEBUG(dbgs() << "\n");
    return Clone;
  }
  LLVM_DEBUG(dbgs() << "\n");
  CallStack->push_back(Root);

  // clone the subgraph starting with nodes where the callee is a
  // caller and update the call site to call the cloned callee
  Function *F = Root->getCallee();

  for (auto *Edge : Root->outEdgesView()) {
    DCGNode *N = Edge->dst();
    assert((!N->isValid() || (N->getCaller()==F)) &&
        "inconsistent detailed call graph");
    const DCGNodeParamFlow *Flow = Flows.get(N);

    if (!Flow || Flow->isMarked() || !Flow->isVisited() || !N->isValid())
      // The node isn't of interest and does not participate in cloning;
      // !N->isValid() means that N is one of the found clone roots and its
      // callsite has already been specialized.
      //
      // Replaced with a call to a cloned function; this, in turn, means that
      // all "live out" parameter sets are defined by constants in this call and
      // constants coming from root's invocation of N's caller are useless.
      continue;

    // see if any of parameter sets instantiated using the incoming constants
    // survive the node's transform
    ConstParamVec ConstParamsImg;
    Flow->tform().evaluate(ConstParams, ConstParamsImg);
    bool LiveSetFound = Flow->liveOut().hasSetCoveredBy(ConstParamsImg);

    DBGX(1, dbgs().indent(Depth*2)
        << "# " << N->toString()
        << ": img=" << ConstParamsImg.toString()
        << " live(b)=" << Flow->liveOut().toString());

    if (!LiveSetFound) {
      DBGX(1, dbgs() << " - can't fold\n");
      continue;
    }
    DBGX(1, dbgs() << "\n");

    // live set found - makes sense to recurse deeper along this edge
    Function *Clone =
        cloneCallSubtreeRec(N, CallStack, ConstParamsImg, Seeds, Clones, Flows);

    if (Clone)
      Call2Clone[N->getCallInst()] = std::make_pair(Clone, ConstParamsImg);
  }
  CallStack->pop_back();

  if (Call2Clone.size()==0)
    // none of the subgraphs were cloned, so makes no sense to clone current
    // node's function as no profitable constant parameter combinations will
    // reach the seed nodes
    return nullptr;

  // clone current node's function and update the call site to call the clone
  LLVM_DEBUG(dbgs().indent(Depth*2));
  Function *C = cloneFunction(F, ConstParams, Call2Clone, Clones);
  LLVM_DEBUG(dbgs() << "\n");
  return C;
}

Function *CallTreeCloningImpl::cloneFunction(Function *F,
                                             const ConstParamVec &ConstParams,
                                             const Call2ClonedFunc &Call2Clone,
                                             CloneRegistry &Clones) {
  // check if there is already a need to clone or if clone limit reached
  auto SearchKey = std::make_pair(F, ConstParams);
  auto It = Clones.find(SearchKey);

  if (It!=Clones.end()) {
    LLVM_DEBUG(dbgs() << "CLONE USED " << It->second->getName());
    return It->second;
  }
  if (Clones.size() >= CTCloningMaxClones) {
    dbgs() << CTCloningMaxClones.ArgStr << " limit reached ("
           << CTCloningMaxClones << ")\n";
    return nullptr;
  }

  // create:
  // - a new function type with constant parameters removed
  // - a unique name for the new function based on the constant argument(s)
  ValueToValueMapTy Old2New;
  FunctionType *FTy = F->getFunctionType();
  unsigned NParams = FTy->getNumParams();
  unsigned NRemoved = ConstParams.numConstants();
  SmallVector<Type *, EST_PARAM_LIST_SIZE> Tys;
  Tys.reserve(NParams - NRemoved);
  std::ostringstream NewName;
  NewName << F->getName().str();

  AttributeList NewAttrs;
  AttributeList Attrs = F->getAttributes();
  LLVMContext &Ctx = F->getContext();
  unsigned NumNewArgs = 0;

  for (auto I : {AttributeList::ReturnIndex, AttributeList::FunctionIndex})
    if (Attrs.hasAttributes(I))
      NewAttrs = NewAttrs.addAttributes(Ctx, I, Attrs.getAttributes(I));

  for (unsigned I = 0; I < NParams; ++I) {
    const ConstantInt *C = ConstParams[I];
    const char sep = I ? '.' : '|';

    if (C) {
      auto Arg = F->arg_begin() + I;
      // this is a constant parameter - map it
      assert((C->getType()==Arg->getType()) && "type mismatch");
      // C is not changed by ValueToValueMapTy - remove const is safe:
      Old2New[Arg] = const_cast<ConstantInt *>(C);
      APInt Val = C->getValue();
      NewName << sep << Val.toString(10, true);
    } else {
      Tys.push_back(FTy->getParamType(I));
      NewName << sep << "_";

      if (Attrs.hasParamAttrs(I)) {
        NewAttrs = NewAttrs.addParamAttributes(Ctx, NumNewArgs,
                                               Attrs.getParamAttributes(I));
      }
      ++NumNewArgs;
    }
  }
  FunctionType *NewFTy =
      FunctionType::get(FTy->getReturnType(), Tys, FTy->isVarArg());
  // do the cloning
  Function *Clone =
      Function::Create(NewFTy, F->getLinkage(), NewName.str(), F->getParent());
  Clone->setAttributes(NewAttrs);
  Clone->setCallingConv(F->getCallingConv());
  auto NewI = Clone->arg_begin();
  int ArgInd = 0;

  for (auto I = F->arg_begin(); I!=F->arg_end(); ++I, ++ArgInd)
    if (!ConstParams[ArgInd]) {
      NewI->setName(I->getName());
      Old2New[&*I] = &*NewI;
      ++NewI;
    }
  SmallVector<ReturnInst *, 8> Rets;
  CloneFunctionInto(Clone, F, Old2New, true, Rets);

  // Redirect the calls in the input map to the cloned functions they map to.
  // Also fix the actual parameter lists removing the constants
  for (auto &X : Call2Clone) {
    CallInst *OldCall = X.first;
    CallInst *NewCall = cast<CallInst>(Old2New[OldCall]);
    Function *Clone = X.second.first;
    const ConstParamVec &ConstArgs = X.second.second;
    specializeCallSite(NewCall, Clone, ConstArgs.getParamIndSet());
  }
  // enjoy the newly created clone
  Clones[SearchKey] = Clone;
  ++NumCTCClones;
  LLVM_DEBUG(dbgs() << "CLONED " << Clone->getName());

  return Clone;
}

// Collect the following items:
// -all Function(s) appears in CloneRegistry, both the source Function(s), and
//  their mapped Clone(s);
// -scan the current module, see if there is any candidate(s) suitable for PP
//
bool PostProcessor::doPreliminaryTest(void) {
  // Check: CloneRegistry is not empty
  if (!Clones.size())
    return false;

  return true;
}

bool PostProcessor::collectPPCallInst(CallInst *CI) {
  Function *Callee = CI->getCalledFunction();
  if (!Callee || !CI->getNumArgOperands())
    return false;

  // Bail out if the callee is in the seed-function list
  if (!ExtSeedFunctions[Callee])
    return false;

  SetOfParamIndSets Psets = LeafSeeds[Callee];

  // Check:
  // - on the Pset position, does the callee have any constant foldable
  //   BinaryOperator argument?
  // or
  // - any argument that is already constant?
  unsigned CFArgPos = 0;
  for (unsigned I = 0, E = CI->getNumArgOperands(); I < E; ++I) {
    if (!Psets.haveIndex(I))
      continue;

    Value *arg = CI->getArgOperand(I);
    bool IsBinOp = isa<BinaryOperator>(arg);
    bool IsConstant = isa<ConstantInt>(arg);
    if (!IsBinOp && !IsConstant)
      continue;

    // found a BinaryOperator or a constant on index I:
    if (IsConstant) {
      CFArgPos |= (1 << I);
      continue;
    }

    // BinOp case: check if the BinOp on index is subject to constant fold
    BinaryOperator *BOp = dyn_cast<BinaryOperator>(arg);
    if (ConstantFoldInstruction(BOp, DL, &GetTLI(*Callee))) {
      // record this index's position
      CFArgPos |= (1 << I);
    }
  }

  // If the CallInst has any constant fold-able argument, save this CallInst
  // with its matching CFArgPos.
  if (CFArgPos) {
    LLVM_DEBUG(dbgs() << "CI: " << *CI
                      << " has constant a fold-able argument or a "
                         "constant\t at ArgPos: "
                      << std::bitset<32>(CFArgPos).to_string()
                      << "  Dec: " << CFArgPos << "\n");

    PPCandidates[CI] = CFArgPos;
  }

  return CFArgPos;
}

// CloneRegistery is a map of: <Function*, ConstParamVec> -> Function*
bool PostProcessor::doCollection(void) {
  // Collect all Function(s) appears in CloneRegistry:
  //  . the source Function(s),
  //  . the ConstParamvVec,
  //  . and their mapped Clone(s).
  unsigned Count = 0;
  for (auto &CloneRegItem : Clones) {
    Function *OrigF = const_cast<Function *>(CloneRegItem.first.first);
    Function *ClonedF = CloneRegItem.second;
    LLVM_DEBUG(dbgs() << Count++ << "  OrigF: " << OrigF->getName() << "\t"
                      << "ClonedF: " << ClonedF->getName() << "\n");

    // Collect relevant functions:
    ExtSeedFunctions[OrigF] = true;
    ClonedFunctions[ClonedF] = true;
  }
  (void) Count;

  // - Populate Functions LeafSeeds into ExtSeedFunctions:
  for (auto Item : LeafSeeds) {
    Function *F = Item.first;
    if (!ExtSeedFunctions[F])
      ExtSeedFunctions[F] = true;
  }
  LLVM_DEBUG({ print_FunctionMap("ExtSeedFunction: ", ExtSeedFunctions); });

  // -Scan the current module, see if there is any candidate(s) suitable for PP
  for (auto &F : M.functions())
    for (BasicBlock &BB : F)
      for (Instruction &I : BB) {
        if (isa<IntrinsicInst>(I))
          continue;
        if (!isa<CallInst>(I))
          continue;

        // Only interested in user-defined calls
        collectPPCallInst(dyn_cast<CallInst>(&I));
      }

  // see what we have collected:
  LLVM_DEBUG(dump(););

  return PPCandidates.size();
}

// Actions:
// - Fold constant, replace the argument in Pos-index position with its
//   respective constant value;
// - Replace the original callee with its matching cloned version;
// - Create new clones if the to-replace call candidate has NO matching clone;
//
bool PostProcessor::foldConstantAndReplWithClone(CallInst *&CI, unsigned Pos) {
  LLVM_DEBUG(dbgs() << "CI: " << *CI
                    << "Pos<bit>: " << std::bitset<32>(Pos).to_string()
                    << "Pos<Dec<: " << Pos << "\n");

  // 1.Fold constant, replace the Pos-index argument with the constant value
  //   (construct a ConstParams object implicitly for 2nd-stage use)
  const unsigned NumArgs = CI->getNumArgOperands();
  ConstParamVec ConstParams;
  ConstParams.resize(NumArgs);
  bool IsBinOp = false;
  for (unsigned I = 0; I < NumArgs; ++I) {
    if (!(Pos & (1 << I)))
      continue;

    Value *arg = CI->getArgOperand(I);
    IsBinOp = isa<BinaryOperator>(arg);
    bool IsConstant = isa<ConstantInt>(arg);
    if (!IsBinOp && !IsConstant)
      assert(0 && "Expect the argument be either a ConstantInt or a "
                  "constant fold-able "
                  "BinaryOperator\n");

    if (IsConstant) { // constant case
      ConstantInt *ConstInt = dyn_cast<ConstantInt>(arg);
      assert(ConstInt && "Expect the arg be a valid ConstantInt\n");
      ConstParams[I] = ConstInt;
    } else { // BinOp case: do constant fold 1st
      BinaryOperator *BOp = dyn_cast<BinaryOperator>(arg);
      assert(BOp && "Expect the arg be a valid BinaryOperator\n");
      Function *Callee = CI->getCalledFunction();
      assert(Callee && "Called function is not valid\n");
      if (Constant *C = ConstantFoldInstruction(BOp, DL, &GetTLI(*Callee))) {
        CI->setArgOperand(I, C);
        ConstantInt *ConstInt = dyn_cast<ConstantInt>(C);
        assert(ConstInt && "Expect the Constant is actually a ConstantInt, not "
                           "supporting float yet\n");
        ConstParams[I] = ConstInt;
        ++NumPostCTCConstFold;
      }
    }
  }

  LLVM_DEBUG({
               if (IsBinOp)
                 dbgs() << "After Constant Fold: " << *CI << "\n";
             });

  // 2.Replace the original callee with its matching cloned version Source:
  // - CI, with constant parameter(s) folded;
  // - ConstParams;
  //
  // Mapping:
  // using CloneRegistry = std::map<std::pair<const Function *, ConstParamVec>,
  //                                Function *, CloneMapKeyLess>;
  Function *OrigF = CI->getCalledFunction();
  assert(OrigF && "Expect OrigF be valid\n");
  Call2ClonedFunc Call2Clone;
  Function *ClonedF =
      CTCI->cloneFunction(OrigF, ConstParams, Call2Clone, Clones);
  assert(ClonedF && "ClonedF must be available/valid now\n");

  LLVM_DEBUG({
               dbgs() << OrigF->getName().str();
               dbgs() << " : " << ConstParams.toString() << " -> ";
               dbgs() << ClonedF->getName().str() << "\n";
             });

  CI = specializeCallSite(CI, ClonedF, ConstParams.getParamIndSet());
  LLVM_DEBUG(dbgs() << "\nCI after Replace with Cloned callsite:\n" << *CI);

  ++NumPostCTCCallsiteReplace;
  // CI is implicitly returned
  return true;
}

// INPUT: std::map<CallInst *, unsigned> PPCandidates;
//
// For each collected CallInst:
// - Fold constant, replace the respective argument with the constant value;
// - Replace the original callee with the cloned copy of the callee;
//
bool PostProcessor::doTransformation(void) {
  unsigned Count = 0;
  for (auto &Item : PPCandidates) {
    CallInst *CI = Item.first;
    unsigned CFPos = Item.second;
    LLVM_DEBUG(dbgs() << "CI: " << *CI
                      << " has constant fold-able argument(s)\t at ArgPos: "
                      << std::bitset<32>(CFPos).to_string()
                      << "  Dec: " << CFPos << "\n");

    if (!foldConstantAndReplWithClone(CI, CFPos)) {
      LLVM_DEBUG(dbgs() << "foldConstAndReplWithClone(CI, Pos) failed: "
                        << *CI);
      continue;
    }
    ++Count;
  }

  return Count;
}

bool PostProcessor::run(void) {
  if (!doPreliminaryTest())
    return false;

  LLVM_DEBUG({ print_CloneRegistry("Clones: ", Clones); });
  LLVM_DEBUG({ printSeeds("LeafSeeds: ", LeafSeeds); });

  if (!doCollection())
    return false;

  if (!doTransformation())
    return false;

  return true;
}

// ---------------------------------------------------------------
// Multi-Version Code section
// ---------------------------------------------------------------

// Collect for MultiVersion.
// Action list:
// - filter all leaf-seed functions by size and other conditions;
//
//[INPUT]
// std::map<Function *, SetOfParamIndSets, CompareFuncPtr> &LeafSeeds;
//
//[OUTPUT]
// std::map<Function *, SetOfParamIndSets, CompareFuncPtr> MVSeeds;
//
bool MultiVersionImpl::doCollection(void) {

  if (MVBypassCollectionForLITTestOnly) {
    // Short-cut collection:
    // - Allow Functions to be collected through bypassing collection
    // testing.
    for (auto &LeafSeed : LeafSeeds) {
      Function *F = LeafSeed.first;
      MVSeeds[F] = LeafSeed.second;
    }
  } else {
    // Normal collection process: with function signature matching
    for (auto &LeafSeed : LeafSeeds) {
      Function *F = LeafSeed.first;

      if (Match2VarMV.match(F) || Match1VarMV.match(F)) {
        LLVM_DEBUG(dbgs() << "matched: " << F->getName() << "()\n");
        MVSeeds[F] = LeafSeed.second;
      }
    }
  }

  // see what we have collected in MVSeeds:
  LLVM_DEBUG({ printSeeds("[MVSeeds] ", MVSeeds); });

  return MVSeeds.size();
}

// Analyze the MVSeeds:
// 1. fill in MVFunctionInfo record for each unique Function* from MVSeed, and
// populate the MVFI map: Function * -> MVFunctionInfo, where MVFunctinInfo
// contains:
//  -Function *
//  -SetOfParamIdxSets
//  -map: ConstParamVec -> CloneF
//  -map: idxPos -> std::set<ConstantInt*, ConstantIntGreaterThan>
//
// 2. Reduce potential combinations of std::set<ConstantInt*> by
// sorting and limiting the max size of the set. Currently, the max
// size is set to 2.
//
bool MultiVersionImpl::doAnalysis(void) {
  // Check: does Seeds map contains a given Function *
  auto findFuncIn =
      [&](Function *Func,
          std::map<Function *, SetOfParamIndSets, CompareFuncPtr> &Seeds) {
        for (auto I = Seeds.begin(), E = Seeds.end(); I!=E; ++I) {
          const Function *F = (*I).first;
          if (Func==F)
            return true;
        }
        return false;
      };

  LLVM_DEBUG(dbgs() << "Match2VarMV: " << Match2VarMV.toString() << "\n");

  // 1. fill in MVFunctionInfo record for each unique Function* from MVSeed, and
  //    populate the map:
  //    std::map<Function *, MVFunctionInfo> MVFIMap
  //
  for (auto &Clone : Clones) {
    Function *OrigF = const_cast<Function *>(Clone.first.first);
    ConstParamVec ConstParams = Clone.first.second;
    Function *CloneF = Clone.second;

    LLVM_DEBUG(dbgs() << OrigF->getName() << " : " << ConstParams.toString()
                      << " -> " << CloneF->getName() << "\n");

    if (findFuncIn(OrigF, MVSeeds)) {
      MVFunctionInfo &MVFI = MVFIMap[OrigF];
      MVFI.set(OrigF);
      MVFI.set(OrigF->getInstructionCount());
      MVFI.set(ConstParams, CloneF);
      MVFI.set(MVSeeds[OrigF]);

      // set code-gen flags: 2var and 1var
      bool Gen2VarClone = Match2VarMV.match(OrigF);
      bool Gen1VarClone = Match1VarMV.match(OrigF);
      if (MVBypassCollectionForLITTestOnly) {
        Gen2VarClone = true;
        Gen1VarClone = true;
      }
      MVFI.setGen2VarC(Gen2VarClone);
      MVFI.setGen1VarC(Gen1VarClone);

      SmallVector<std::pair<unsigned, ConstantInt *>, 4> PosValVec;
      ConstParams.enumPosVal(PosValVec);
      for (auto &Pair : PosValVec) {
        unsigned PosIdx = Pair.first;
        ConstantInt *C = Pair.second;
        LLVM_DEBUG(dbgs() << "pos: " << PosIdx << " -> C: " << *C << "\n");
        MVFI.set(PosIdx, C);
      }
    }
  }

  // See what is inside MVFIMap now:
  LLVM_DEBUG({
               for (auto &Pair : MVFIMap) {
                 MVFunctionInfo &MVFI = Pair.second;
                 MVFI.dump();
               }
             });

  // 2. Reduce potential multi-versioning comparison combinations over
  // ConstantInt* values by sorting and limiting the max size of the set.
  // Currently, the max size is set to 2.
  //
  // Data in: std::map<unsigned, std::set<ConstantInt *, >> Pos2ValMap;
  for (auto &Pair : MVFIMap) {
    Function *F = Pair.first;
    MVFunctionInfo &MVFI = Pair.second;
    auto &Pos2ValMap = MVFI.Pos2ValMap;

    for (auto &Item : Pos2ValMap) {
      unsigned PosIdx = Item.first;
      auto &ValSet = Item.second;
      LLVM_DEBUG(dbgs() << F->getName() << " , PosIdx: " << PosIdx
                        << "\tBefore set-size reduction:\n");
      for (auto *V : ValSet) {
        LLVM_DEBUG(V->dump());
        (void) V;
      }
      (void) PosIdx;
      (void) F;

      unsigned SetSize = ValSet.size();
      unsigned MaxItems =
          (SetSize > MVMaxValueSetSize) ? MVMaxValueSetSize : SetSize;
      // purge set: remove additional items from the set
      ValSet.erase(std::next(ValSet.begin(), MaxItems), ValSet.end());

      LLVM_DEBUG(dbgs() << F->getName() << "() , PosIdx: " << PosIdx
                        << "\tAfter set-size reduction:\n");
      LLVM_DEBUG({
                   for (auto *V : ValSet)
                     V->dump();
                 });
    }
  }

  // See what is inside MVFIMap after set-size reduction:
  LLVM_DEBUG({
               for (auto &Pair : MVFIMap) {
                 MVFunctionInfo &MVFI = Pair.second;
                 MVFI.dump();
               }
             });

  bool AnalysisResult = MVFIMap.size();
  if (!AnalysisResult)
    LLVM_DEBUG(dbgs() << "Nothing to MultiVersion (MV)\n");

  return AnalysisResult;
}

bool MultiVersionImpl::createAdditionalClones(Function *F) {
  Call2ClonedFunc Call2Clone;
  ConstParamVec ConstParams;
  const unsigned FArgSize = F->arg_size();
  ConstParams.resize(FArgSize);

  // 1. Create a cloned Function for all-empty ConstParams
  // E.g.  get_ref|_._._._._._._._();
  Function *CloneF = CTCI->cloneFunction(F, ConstParams, Call2Clone, Clones);
  if (!CloneF) {
    LLVM_DEBUG(dbgs() << "Fail to create a clone on " << F->getName() << " : "
                      << ConstParams.toString() << "\n");
    return false;
  }

  // 2. Create single-variable argument clone(s)
  // E.g. get_ref|_._._._._._.4._();
  // 	  get_ref|_._._._._._.8._();
  //
  // Note:
  // Data Input is in std::map<unsigned, std::set<ConstantInt *>>Pos2ValMap;
  //
  MVFunctionInfo &MVFI = MVFIMap[F];
  bool Is1VarCloneCandidate = MVFI.Gen1VarClones;
  for (auto &Pos2ValPair : MVFIMap[F].Pos2ValMap) {
    unsigned PosIdx = Pos2ValPair.first;
    auto &ValSet = Pos2ValPair.second;

    if (!Is1VarCloneCandidate)
      continue;

    // Code block to deal with valid Is1VarCloneCandidate.
    //
    // Note:
    // - Just create additional clone functions, not to extend ValSet.
    // - IValSet begins with ValSet, maybe extended via interpolation, but will
    //   be thrown away once the 1-variable clone creation is done.
    // - The real ValueSet extension will be handled later in execution, here we
    //   only create additional clones.
    //
    auto IValSet = ValSet;
    if (!interpolateForRTValues(IValSet)) {
      LLVM_DEBUG(dbgs() << "Failed to interpolate Value Set\n");
      return false;
    }

    for (auto &Val : IValSet) {
      ConstParams.clear();
      ConstParams.resize(FArgSize);
      ConstParams[PosIdx] = Val;
      LLVM_DEBUG(dbgs() << "\nConstParams: " << ConstParams.toString() << "\n");

      CloneF = CTCI->cloneFunction(F, ConstParams, Call2Clone, Clones);
      if (!CloneF) {
        LLVM_DEBUG(dbgs() << "Fail to create a clone on " << F->getName()
                          << " : " << ConstParams.toString() << "\n");
        return false;
      }
    }
  }

  // See what Clones have now:
  LLVM_DEBUG({ print_CloneRegistry("Clones: ", Clones); });

  return true;
}

// generate a "ret" instruction properly.
// note:
// - either return void if CI's return type is void;
// or
// - return whatever CI's returned value is;
//
void MultiVersionImpl::doCodeGenRet(CallInst *CI, IRBuilder<> &Builder) {

  Function *Callee = CI->getCalledFunction();
  assert(Callee && "Expect Callee be a valid ptr\n");
  FunctionType *FTy = Callee->getFunctionType();
  assert(FTy && "Expect FTy be a valid ptr\n");
  Type *RetTy = FTy->getReturnType();
  assert(RetTy && "Expect RetTy be a valid ptr\n");

  // Create a proper "ret" instruction:
  if (RetTy->isVoidTy())
    Builder.CreateRetVoid();
  else
    Builder.CreateRet(CI);
}

bool MultiVersionImpl::doCodeGenMV2VarClone(
    Function *F, unsigned Pos0, ConstantInt *C0, unsigned Pos1, ConstantInt *C1,
    BasicBlock *&CommonBB, BasicBlock *&ThenBB, BasicBlock *&MergeBB) {

  unsigned static Count = 0;
  // Figure out the CloneF Function: mapped from F and
  // (Pos0,C0),(Pos1,C1)
  unsigned ArgSize = F->arg_size();
  assert((Pos0 < ArgSize) && "Pos0 is out of bound\n");
  assert((Pos1 < ArgSize) && "Pos1 is out of bound\n");
  ConstParamVec ConstParams;
  ConstParams.resize(ArgSize);
  ConstParams[Pos0] = C0;
  ConstParams[Pos1] = C1;
  Function *CloneF = Clones[std::make_pair(F, ConstParams)];
  if (!CloneF) {
    LLVM_DEBUG(dbgs() << F->getName() << " : " << ConstParams.toString()
                      << " -> CloneF not found\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << F->getName() << " : " << ConstParams.toString() << " -> "
                    << CloneF->getName() << "\n");

  // Figure out the Function's arguments:
  Value *Arg0 = F->arg_begin() + Pos0;
  Value *Arg1 = F->arg_begin() + Pos1;
  LLVM_DEBUG(dbgs() << "Arg0: " << *Arg0 << "Arg1: " << *Arg1 << "\n");

  // Code generation for 1 call to a 2-variable clone
  IRBuilder<> Builder(M.getContext());

  // *** CODE GENERATION TEMPLATE ***
  //
  // [C: 1 2-variable clone]
  // if((i_width == 16) && (i_height == 16)){
  //    pixel_avg16x16();
  // }
  //
  //[LLVM: 1 2-variable clone]
  //--------------------------------------------------------------------------
  // %cmp0 = icmp eq i32 %a, 16                      CommonBB Block
  // %cmp1 = icmp eq i32 %b, 16
  // Block %and01 = and i1 %cmp0, %cmp1
  // br i1 %and01, label %if.then.0, label %if.end.0
  // -------------------------------------------------------------------------
  // if.then.0:           ; preds = %entry           ThenBB Block
  // tail call void (...) @pixel_avg16x16() #2
  // ret
  //
  // -------------------------------------------------------------------------
  // if.end.0:                                       MergeBB Block
  //
  // -------------------------------------------------------------------------

  // Create/reuse the CommonBB
  if (CommonBB==nullptr) {
    // Note: only FIRST CommonBB is null
    CommonBB = BasicBlock::Create(M.getContext(), "Common.BB", F);
  }
  assert(CommonBB && "Expect valid CommonBB\n");
  CommonBB->setName("Common.BB." + Count);

  Builder.SetInsertPoint(CommonBB);
  Value *Cmp0 = Builder.CreateICmpEQ(Arg0, C0);
  Value *Cmp1 = Builder.CreateICmpEQ(Arg1, C1);
  Value *And01 = Builder.CreateAnd(Cmp0, Cmp1);

  // Create + organize BasicBlocks: ensure ThenBB appears earlier than MergeBB
  ThenBB = BasicBlock::Create(M.getContext(), "Then.BB", F);
  MergeBB = BasicBlock::Create(M.getContext(), "Merge.BB", F);
  ThenBB->setName("Then.BB." + Count);
  MergeBB->setName("Merge.BB." + Count);
  ThenBB->moveBefore(MergeBB);
  ++Count;

  // Create CondBr: ThenBB, MergeBB
  Builder.CreateCondBr(And01, ThenBB, MergeBB);

  // generate a call to CloneF() inside ThenBB:
  Builder.SetInsertPoint(ThenBB);
  SmallVector<Value *, 16> Args;
  int32_t Pos = -1;
  for (Argument &Arg : F->args()) {
    ++Pos;

    // Skip F's arg(s) on Pos0 and Pos1:
    if ((static_cast<unsigned>(Pos)==Pos0) ||
        (static_cast<unsigned>(Pos)==Pos1))
      continue;
    Args.push_back(&Arg);
  }
  CallInst *CI = Builder.CreateCall(CloneF, Args);
  CI->setCallingConv(CloneF->getCallingConv());

  // generate a "ret" inside ThenBB after the call to clone.
  // note:
  // - either return void if CI's return type is void;
  // or
  // - return whatever CI's returned value is.
  doCodeGenRet(CI, Builder);

  LLVM_DEBUG(dbgs() << " Generated a call block for 2-var clone\n" << *F);

  return true;
}

bool MultiVersionImpl::doCodeGenOrigClone(Function *F, BasicBlock *CallBB) {
  assert(CallBB && "Expect a valid CallBB\n");

  // Get the Original-Clone Function:
  ConstParamVec ConstParams;
  ConstParams.resize(F->arg_size());
  Function *OrigCloneF = Clones[std::make_pair(F, ConstParams)];
  if (!OrigCloneF) {
    LLVM_DEBUG(dbgs() << F->getName() << " : " << ConstParams.toString()
                      << " -> OrigCloneF not found\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << F->getName() << " : " << ConstParams.toString() << " -> "
                    << OrigCloneF->getName() << "\n");

  // Generate a call to OrigCloneF before the very end of F
  // - the default fail-safe path
  IRBuilder<> Builder(M.getContext());
  Builder.SetInsertPoint(CallBB);

  SmallVector<Value *, 16> Args;
  for (Argument &Arg : F->args()) {
    Args.push_back(&Arg);
  }
  CallInst *CI = Builder.CreateCall(OrigCloneF, Args);
  CI->setCallingConv(OrigCloneF->getCallingConv());

  // Generate a proper "ret" instruction inside CallBB
  Builder.SetInsertPoint(CallBB);
  doCodeGenRet(CI, Builder);

  LLVM_DEBUG(dbgs() << " Generated a call block for original clone\n" << *F);
  return true;
}

#ifndef NDEBUG
void dump_val_set(const std::string &Msg,
                  std::set<ConstantInt *, ConstantIntGreaterThan> &ValSet) {
  dbgs() << Msg << "<" << ValSet.size() << ">\n";

  for (auto *C : ValSet) {
    dbgs() << C->getSExtValue() << ", ";
  }

  dbgs() << "\n";
}
#endif

// interpolate the ValSet, and grow/guess additional run-time values.
// E.g. ValSet:
// before interpolation { 16, 8 }
// after  interpolation { 20, 16, 12, 8}
//                        ^_      ^_
bool MultiVersionImpl::interpolateForRTValues(
    std::set<ConstantInt *, ConstantIntGreaterThan> &ValSet) {
  LLVM_DEBUG({ dump_val_set("before interpolateForRTValues():", ValSet); });

  // Note: ValSet is already sorted in descending order!
  unsigned ValSetSize = ValSet.size();
  ConstantInt *MaxC = *ValSet.begin();
  int64_t MaxV = MaxC->getSExtValue();
  ConstantInt *MinC = *std::next(ValSet.begin(), ValSetSize - 1);
  int64_t MinV = MinC->getSExtValue();
  LLVM_DEBUG(dbgs() << "maxV: " << MaxV << "  minV: " << MinV
                    << " Size: " << ValSetSize << "\n");
  unsigned Dist = (MaxV - MinV)/ValSetSize;
  IRBuilder<> Builder(M.getContext());

  // Generate interpolated values into NewSet:
  std::set<ConstantInt *, ConstantIntGreaterThan> NewSet;
  for (auto *C : ValSet) {
    int64_t CurVal = C->getSExtValue();
    ConstantInt *V = Builder.getIntN(C->getBitWidth(), CurVal + Dist);
    NewSet.insert(V);
  }
  LLVM_DEBUG({ dump_val_set("NewSet:", NewSet); });

  // Merge NewSet into ValSet:
  std::copy(NewSet.begin(), NewSet.end(),
            std::inserter(ValSet, ValSet.begin()));

  LLVM_DEBUG({ dump_val_set("after interpolateForRTValues():", ValSet); });

  return true;
}

// *** CODE GENERATION TEMPLATE ***
//
// [C: 1 1-variable clone]
// if(i_width == 16){
//    pixel_avg16();
// }
//
//[LLVM: 1 1-variable clone]
// -------------------------------------------------------------------------
//  %cmp = icmp eq i32 %b, 16                       Common Block
//  br i1 %cmp, label %if.then, label %if.end
//
// -------------------------------------------------------------------------
// if.then:                                         Then Block
//  tail call void (...) @pixel_avg16() #2
//  return
//
// -------------------------------------------------------------------------
// if.end.0:                                        Merge Block
//
// -------------------------------------------------------------------------
bool MultiVersionImpl::doCodeGenMV1VarClone(Function *F, unsigned Pos,
                                            ConstantInt *C,
                                            BasicBlock *&CommonBB,
                                            BasicBlock *&ThenBB,
                                            BasicBlock *&MergeBB) {
  unsigned ArgSize = F->arg_size();
  assert((Pos < ArgSize) && "Pos is out of bound\n");
  IRBuilder<> Builder(M.getContext());

  // Figure out the CloneF Function:
  ConstParamVec ConstParams;
  ConstParams.resize(ArgSize);
  ConstParams[Pos] = C;
  Function *CloneF = Clones[std::make_pair(F, ConstParams)];
  if (!CloneF) {
    LLVM_DEBUG(dbgs() << F->getName() << " : " << ConstParams.toString()
                      << " -> CloneF not found\n");
    return false;
  }
  LLVM_DEBUG(dbgs() << F->getName() << " : " << ConstParams.toString() << " -> "
                    << CloneF->getName() << "\n");

  // Figure out the Function's matching argument:
  Value *Arg = F->arg_begin() + Pos;
  LLVM_DEBUG({ dbgs() << "Arg: " << *Arg; });

  // Code generation for 1 call to a 1-variable clone
  assert(CommonBB && "Expect valid CommonBB\n");
  Builder.SetInsertPoint(CommonBB);
  Value *Cmp0 = Builder.CreateICmpEQ(Arg, C);

  // Create+organize BasicBlocks: ensure ThenBB appears earlier than MergeBB
  ThenBB = BasicBlock::Create(M.getContext(), "Then.BB", F);
  MergeBB = BasicBlock::Create(M.getContext(), "Merge.BB", F);
  ThenBB->moveBefore(MergeBB);

  // Create CondBr: ThenBB, MergeBB
  Builder.CreateCondBr(Cmp0, ThenBB, MergeBB);

  // Generate a call to CloneF() inside ThenBB:
  Builder.SetInsertPoint(ThenBB);
  SmallVector<Value *, 16> Args;
  int32_t Idx = -1;
  for (Argument &Arg : F->args()) {
    ++Idx;

    // Skip F's arg on Pos
    if (static_cast<unsigned>(Idx)==Pos)
      continue;
    Args.push_back(&Arg);
  }
  CallInst *CI = Builder.CreateCall(CloneF, Args);
  CI->setCallingConv(CloneF->getCallingConv());

  // generate a "ret" inside ThenBB after the call to clone:
  doCodeGenRet(CI, Builder);

  LLVM_DEBUG(dbgs() << " Generated a call block for 1-var clone\n" << *F);

  return true;
}

bool MultiVersionImpl::doCodeGen(Function *F) {
  // Clean F's function body, prepare for MultiVersion code generation
  auto LT = F->getLinkage();
  F->deleteBody();
  F->setLinkage(LT);
  LLVM_DEBUG(F->dump());

  // Figure out the mapping between F's parameter list and C0,C1, etc.
  MVFunctionInfo &MVFI = MVFIMap[F];
  auto &Pos2ValsMap = MVFI.Pos2ValMap;

  // handle only 2 positions here
  const unsigned PosSize = 2;
  if (Pos2ValsMap.size()!=PosSize) {
    LLVM_DEBUG(dbgs() << "Wrong size of Pos2ValsMap\n");
    return false;
  }

  SmallVector<unsigned, 2> PosVec;
  PosVec.resize(PosSize);
  SmallVector<std::set<ConstantInt *, ConstantIntGreaterThan>, 2> ValSetVec;
  ValSetVec.resize(PosSize);
  auto It = Pos2ValsMap.begin();
  for (unsigned I = 0; I < PosSize; ++I, ++It) {
    PosVec[I] = (*It).first;
    ValSetVec[I] = (*It).second;
  }

  // Show details on PosVec and ValSetVec:
  LLVM_DEBUG({
               for (unsigned I = 0; I < PosSize; ++I) {
                 dbgs() << "PosSize[" << I << "]: " << PosVec[I] << "\n";
               }

               for (unsigned I = 0; I < PosSize; ++I) {
                 auto &ValSet = ValSetVec[I];
                 dbgs() << "ValSet[" << I << "]: ";
                 for (auto &V : ValSet) {
                   dbgs() << *V << "  ";
                 }
                 dbgs() << "\n";
               }
             });

  // For each valid 2-value pair, call
  // doCodeGenMV2VarClone2VarClone():
  std::set<ConstantInt *, ConstantIntGreaterThan> ValSet0 = ValSetVec[0];
  std::set<ConstantInt *, ConstantIntGreaterThan> ValSet1 = ValSetVec[1];
  unsigned Pos0 = PosVec[0];
  unsigned Pos1 = PosVec[1];

  BasicBlock *CommonBB = nullptr;
  BasicBlock *ThenBB = nullptr;
  BasicBlock *MergeBB = nullptr;

  // Force certain order by actually generating needed pairs and sort
  // the pairs in particular order!!
  using ConstantIntPair = std::pair<ConstantInt *, ConstantInt *>;
  SmallVector<ConstantIntPair, 8> CIPVec;

  for (auto &C0 : ValSet0) {
    for (auto &C1 : ValSet1) {
      CIPVec.push_back(std::make_pair(C0, C1));
    }
  }
  LLVM_DEBUG({
               dbgs() << "Before sorting: \n";
               for (auto &Pair : CIPVec) {
                 ConstantInt *C0 = Pair.first;
                 ConstantInt *C1 = Pair.second;
                 dbgs() << "  " << *C0 << " : " << *C1 << "\n";
               }
             });

  std::function<bool(const ConstantIntPair &)> isEqualValue;
  isEqualValue = [](const ConstantIntPair &P) -> bool {
    ConstantInt *C0 = P.first;
    ConstantInt *C1 = P.second;
    return (C0->getSExtValue()==C1->getSExtValue());
  };

  auto isSmallerEqualValue = [&](const ConstantIntPair &P0,
                                 const ConstantIntPair &P1) {
    if (!isEqualValue(P0))
      return false;
    if (!isEqualValue(P1))
      return false;

    ConstantInt *C0 = P0.first;
    ConstantInt *C1 = P1.second;
    return (C0->getSExtValue() < C1->getSExtValue());
  };

  // Force a particular sorting order!
  // This gives better performance of the multi-version function for certain
  // benchmarks with 2 conditionals.
  //
  // Note:
  // - This case only works for 2 conditionals.
  // - In case there are 3 or more conditionals, we may either rewrite the logic
  //   to handle those cases, or rebuild algorithm to make it more generic.
  //
  std::sort(CIPVec.begin(), CIPVec.end(),
            [&](const ConstantIntPair &P0, const ConstantIntPair &P1) -> bool {
              bool IsEq0 = isEqualValue(P0);
              bool IsEq1 = isEqualValue(P1);

              // - Comparison algorithm -
              // equal value (e.g. 4x4) takes precedence over non-equal value
              // (e.g. 4x8)
              //
              // smaller equal value (e.g. 4x4) takes precedence over bigger
              // equal value (e.g. 8x8)
              //
              // non equal value (e.g. 4x8) over non equal value (e.g. 8x4)?:
              // don't care

              // case1: both are equal values
              if (IsEq0 && IsEq1)
                return isSmallerEqualValue(P0, P1);
                // case2: P0 is EQ and P1 is NOT
              else if (IsEq0 && !IsEq1)
                return true;

              // case3: P0 is NOT EQ and P1 is EQ
              // case4: both are NOT EQ, doesn't matter, don't care!
              return false;
            });

  LLVM_DEBUG({
               dbgs() << "After sorting: \n";
               for (auto &Pair : CIPVec) {
                 ConstantInt *C0 = Pair.first;
                 ConstantInt *C1 = Pair.second;
                 dbgs() << "  " << *C0 << " : " << *C1 << "\n";
               }
             });

  unsigned Count = 0;

  // Do code generation for all 2-variable clones:
  for (auto &Pair : CIPVec) {
    bool IsFirst = (Count==0);
    ConstantInt *C0 = Pair.first;
    ConstantInt *C1 = Pair.second;

    // Generate code for 1 set of 2-variable clone.
    //
    // Note:
    // The very 1st call to doCodeGenMV2VarClone() is treated differently
    // because the reuse of MergeBB for CommonBB happens after CodeGen of 1st
    // 2-variable clone.
    //
    if (IsFirst) {
      if (!doCodeGenMV2VarClone(F, Pos0, C0, Pos1, C1, CommonBB, ThenBB,
                                MergeBB)) {
        LLVM_DEBUG(dbgs() << "doCodeGenMV2VarClone() failed: Pos0: " << Pos0
                          << " Pos1: " << Pos1 << "\n");
        return false;
      }
    } else {
      // rotate: MergeBB from the previous iteration becomes CommonBB
      // in current iteration.
      CommonBB = MergeBB;

      if (!doCodeGenMV2VarClone(F, Pos0, C0, Pos1, C1, CommonBB, ThenBB,
                                MergeBB)) {
        LLVM_DEBUG(dbgs() << "doCodeGenMV2VarClone() failed: Pos0: " << Pos0
                          << " Pos1: " << Pos1 << "\n");
        return false;
      }
    }

    ++Count;
  }
  LLVM_DEBUG(dbgs() << *F);

  // Do code generation for all 1-variable clones:
  bool Is1VarCloneCandidate = MVFI.Gen1VarClones;
  if (Is1VarCloneCandidate) {
    std::set<ConstantInt *, ConstantIntGreaterThan> ValSet = ValSet0;

    if (!interpolateForRTValues(ValSet)) {
      LLVM_DEBUG(dbgs() << "interpolateForRTValues() failed\n");
      return false;
    }

    for (auto &C : ValSet) {
      // rotate, same as before
      CommonBB = MergeBB;

      if (!doCodeGenMV1VarClone(F, Pos0, C, CommonBB, ThenBB, MergeBB)) {
        LLVM_DEBUG(dbgs() << "doCodeGenMV1VarClone() failed: Position: " << Pos0
                          << " ConstantInt: " << *C << "\n");
        return false;
      }
    }

    LLVM_DEBUG(dbgs() << *F);
  }

  // Create a call to the original clone (e.g. mc_chrome|_._..._()) at end of
  // the last MergeBB. This is the default fail-over protection path.
  if (!doCodeGenOrigClone(F, MergeBB))
    return false;

  LLVM_DEBUG(dbgs() << "Done code gen: \n" << *F);

  // Under DEBUG mode:
  // ensure the newly generated Multi-version Function is good.
#ifndef NDEBUG
  if (verifyFunction(*F))
    report_fatal_error("Function verification failed after Call-tree cloning "
                       "(with Multi versioning) on Function: " +
        F->getName() + "()\n");
#endif

  return true;
}

bool MultiVersionImpl::doTransformation(void) {
  // for each Function in MVSeeds: transform it for multiversion
  for (auto &MVSeed : MVSeeds) {
    Function *F = MVSeed.first;
    SetOfParamIndSets Psets = MVSeed.second;
    LLVM_DEBUG(dbgs() << F->getName() << " : " << Psets.toString() << "\n");

    // 1.Create additional clones:
    if (!createAdditionalClones(F))
      return false;

    // 2.Do multi-version code generation over Function F:
    if (!doCodeGen(F))
      return false;

    // Record the # of Multi-Version function(s) generated
    ++NumPostCTCMVFCreate;
  }

  // LLVM_DEBUG(M.dump());
  return true;
}

bool MultiVersionImpl::run(void) {
  // LLVM_DEBUG({ print_CloneRegistry("Clones: ", Clones); });
  // LLVM_DEBUG({ printSeeds("LeafSeeds: ", LeafSeeds); });

  if (!doCollection())
    return false;

  // analyze MVSeeds, build + populate all needed data for MV
  if (!doAnalysis())
    return false;

  // transform MV
  if (!doTransformation())
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Pass creation and registration code
////////////////////////////////////////////////////////////////////////////////

// ---------- Old Pass Manager

namespace llvm {
void initializeCallTreeCloningLegacyPassPass(PassRegistry &);
ModulePass *createCallTreeCloningPass();
} // end of namespace llvm

llvm::CallTreeCloningLegacyPass::CallTreeCloningLegacyPass() : ModulePass(ID) {
  initializeCallTreeCloningLegacyPassPass(*PassRegistry::getPassRegistry());
}

void CallTreeCloningLegacyPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addPreserved<WholeProgramWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
}

llvm::ModulePass *llvm::createCallTreeCloningPass() {
  CallTreeCloningLegacyPass *Res = new llvm::CallTreeCloningLegacyPass();
  return Res;
}

char llvm::CallTreeCloningLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(CallTreeCloningLegacyPass, PASS_NAME, PASS_DESC, false,
                      false)
  INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(CallTreeCloningLegacyPass, PASS_NAME, PASS_DESC, false,
                    false)

// ---------- New Pass Manager
PreservedAnalyses CallTreeCloningPass::run(Module &M,
                                           ModuleAnalysisManager &MAM) {
  auto &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  Analyses Anls([&](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  });
  auto GetTLI = [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };
  PreservedAnalyses PA;

  // TODO FIXME add preserved analyses
  CallTreeCloningImpl Impl;
  bool ModuleChanged = Impl.run(M, Anls, GetTLI, PA);

  // Verify Module if there is any change on the LLVM IR
#ifndef NDEBUG
  doVerification(ModuleChanged, M);
#endif
  (void) ModuleChanged;

  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

////////////////////////////////////////////////////////////////////////////////
// Debugging stuff
////////////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
std::string ConstParamVec::toString() const {
  std::ostringstream S;

  S << "{";
  unsigned Cnt = 0;

  for (unsigned I = 0, E = size(); I < E; ++I)
    if (const ConstantInt *C = this->operator[](I)) {
      if (++Cnt > 1)
        S << ",";

      S << I << ":" << C->getValue().toString(10, true);
    }
  S << "}";
  return S.str();
}

std::string SetOfParamIndSets::toString() const {
  std::ostringstream S;
  S << "[";

  for (const auto &Pset : *this)
    S << " " << Pset.toString();

  S << " ]";
  return S.str();
}

std::string DCGNode::toString(bool PrintCallAddr) const {
  std::ostringstream S;
  S << "[" << id();

  if (PrintCallAddr)
    S << "/" << getCallInst();

  S << "] ";

  // print caller, handle null case:
  if (getCaller())
    S << getCaller()->getName().str();
  else
    S << " null ";

  S << "->";

  // print callee, handle null case:
  if (getCallee())
    S << getCallee()->getName().str();
  else
    S << " null ";

  return S.str();
}

std::string DCGEdge::toString() const {
  std::ostringstream S;
  S << "DCGEdge[:\n";

  // src:
  const DCGNode *SrcNode = src();
  S << "  src: ";
  if (SrcNode)
    S << SrcNode->toString();
  else
    S << " null ";
  S << "\n";

  // dst:
  const DCGNode *DstNode = dst();
  S << "  dst: ";
  if (DstNode)
    S << DstNode->toString();
  else
    S << " null ";
  S << "\n";

  S << "]\n";
  return S.str();
}

std::string DetailedCallGraph::toString() const {
  std::ostringstream S;

  // Print each Node in the DetailedCallGraph (DCG):
  for (const auto &N : Nodes) {
    S << N.toString() << "\n";
    S << "  outgoing: ";
    unsigned Cnt = 0;

    // Print: outEdges if any
    if (N.outEdgesView().size() > 0) {
      S << "    ";

      for (const auto &E : N.outEdgesView()) {
        if (++Cnt%10==0)
          S << "\n    ";

        assert(E->src()==&N && "inconsistent call graph 0");
        S << "[" << E->dst()->id() << "] ";
      }
    }
    S << "\n";

    // Print: inEdges if any
    if (N.inEdgesView().size() > 0) {
      S << "  incoming: ";
      Cnt = 0;

      for (const auto &E : N.inEdgesView()) {
        if (++Cnt%10==0)
          S << "\n    ";

        assert(E->dst()==&N && "inconsistent call graph 1");
        S << "[" << E->src()->id() << "] ";
      }
      S << "\n";
    }
  }

  for (const auto &X : *this) {
    S << X.first->getName().str() << ":";

    for (const auto *N : X.second)
      S << " [" << N->id() << "]";

    S << "\n";
  }

  return S.str();
}

std::string DCGNodeParamFlow::toString() const {
  std::ostringstream S;

  S << "DCGNodeParamFlow:{\n";

  // LiveIn:
  S << " -LiveIn:    " << LiveIn.toString() << "\n";

  // LiveOut:
  S << " -LiveOut:   " << LiveOut.toString() << "\n";

  // KilledOut:
  S << " -killedOut: " << KilledOut.toString() << "\n";

  // ParamTform Tform:
  // note: have to comment it out since it will fire an assert!
  // S << " -Tform:     " << Tform.toString() << "\n";

  // Boolean variables:
  S << " -Marked:  " << std::boolalpha << Marked << "\n";
  S << " -Visited: " << std::boolalpha << Visited << "\n";

  S << "}\n";
  return S.str();
}

unsigned ActualParamFormula::dumpRec(StringRef Pref, raw_ostream &Os,
                                     unsigned Pos, int T) const {
  assert(Pos <= size() && "Pos is out of range");
  const Value *V = (*this)[Pos];
  Os << Pref;
  Os.indent(2*T);
  unsigned NewPos = Pos + 1;
  StringRef InstDumpShift = "  ";

  if (!V) {
    Os << InstDumpShift << "NULL";
  } else {
    if (const Argument *Arg = dyn_cast<Argument>(V)) {
      Os << InstDumpShift << "ARG[" << Arg->getArgNo() << "]";
    } else {
      if (isa<Instruction>(V)) {
        if (isa<BinaryOperator>(V)) {
          Os << *V << " (bin op)";
          NewPos = dumpRec(Pref, Os, NewPos, T + 1);
          NewPos = dumpRec(Pref, Os, NewPos, T + 1);
        } else {
          Os << *V << "unsupported";
        }
      } else {
        Os << InstDumpShift << *V;
      }
    }
  }
  return NewPos;
}

void ActualParamFormula::print(raw_ostream &Os) const { Os << *this << "\n"; }

std::string ParamTform::toString() const {
  std::ostringstream S;
  unsigned Cnt = 0;
  ConstParamVec ConstParams;
  copyConstantParams(ConstParams);
  S << "tform{ ";

  ParamIndSet Fpset;
  ParamIndSet Apset;

  for (unsigned I = 0; I < Form2Act.size(); ++I) {
    const auto &X = Form2Act[I];

    if (X.count() > 0) {
      Fpset.set(I);
      Apset.resize(std::max(Apset.size(), X.size()));
      Apset |= X;
    }
  }
  S << Fpset.toString() << "->" << Apset.toString() << ":";

  for (unsigned I = 0; I < Form2Act.size(); ++I) {
    const auto &Pset = Form2Act[I];

    if (Pset.count() > 0) {
      if (++Cnt > 1)
        S << " ";

      S << I << "->" << Pset.toString();
    }
  }
  for (unsigned I = 0; I < ConstParams.size(); ++I) {
    if (const ConstantInt *C = ConstParams[I])
      S << " \"" << C->getValue().toString(10, true) << "\"->" << I;
  }
  S << " }";
  return S.str();
}

extern "C" void dump_pset(const ParamIndSet *Pset) { Pset->dump(); }
extern "C" void dump_cvec(const ConstParamVec *Cvec) { Cvec->dump(); }
extern "C" void dump_psets(const SetOfParamIndSets *Psets) { Psets->dump(); }
extern "C" void dump_node(const DCGNode *N) { N->dump(); }
extern "C" void dump_formula(const ActualParamFormula *F) { F->dump(); }
extern "C" void dump_nodeset(const std::set<const DCGNode *> &NodeSet) {
  for (const auto X : NodeSet)
    dbgs() << X->toString() << " ";
  dbgs() << "\n";
}
extern "C" void dump_nodev(SmallVectorImpl<const DCGNode *> NodeV) {

  for (const auto X : NodeV)
    dbgs() << X->toString() << " ";
  dbgs() << "\n";
}

extern "C" void dump_valuev(SmallVectorImpl<const Value *> &ValueV) {
  if (ValueV.size()==0)
    return;

  for (const Value *V : ValueV) {
    V->dump();
    dbgs() << " ";
  }
  dbgs() << "\n";
}
#endif // NDEBUG
