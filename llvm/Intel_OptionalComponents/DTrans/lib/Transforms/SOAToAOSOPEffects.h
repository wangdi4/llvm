//===-------------- SOAToAOSOPEffects.h - Part of SOAToAOSOPPass ----------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements iterators and other helper classes for
// SOAToAOSOPTransformImpl::CandidateSideEffectsInfo related for analysis of
// side-effects.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOSOPEFFECTS_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOSOPEFFECTS_H

#if !INTEL_FEATURE_SW_DTRANS
#error SOAToAOSOPEffects.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/Transforms/SOAToAOSOPExternal.h"

#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/iterator.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/Support/FormattedStream.h"
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// DepCompute
#define DTRANS_SOADEP "dtrans-soatoaosop-deps"

namespace llvm {
namespace dtransOP {
namespace soatoaosOP {

// Checking if iterator is dereferenceable.
template <typename WrappedIteratorTy, typename PredicateTy>
class filter_iterator_with_check
    : public filter_iterator<WrappedIteratorTy, PredicateTy> {
  using BaseTy = filter_iterator<WrappedIteratorTy, PredicateTy>;

public:
  filter_iterator_with_check(WrappedIteratorTy Begin, WrappedIteratorTy End,
                             PredicateTy Pred)
      : BaseTy(Begin, End, Pred) {}

  // Only end() iterators are valid and dereferenceable.
  bool isEnd() const { return this->wrapped() == this->End; }
};

struct ArithInstructionsTrait {
  static bool isSupportedOpcode(unsigned OpCode) {
    switch (OpCode) {
    case Instruction::Add:
    case Instruction::And:
    case Instruction::BitCast:
    case Instruction::ExtractValue:
    case Instruction::FCmp:
    case Instruction::FMul:
    case Instruction::FPToUI:
    case Instruction::FPToSI:
    case Instruction::GetElementPtr:
    case Instruction::ICmp:
    case Instruction::InsertValue:
    case Instruction::IntToPtr:
    case Instruction::LShr:
    case Instruction::Mul:
    case Instruction::Or:
    case Instruction::PHI:
    case Instruction::PtrToInt:
    case Instruction::SExt:
    case Instruction::Select:
    case Instruction::Shl:
    case Instruction::SIToFP:
    case Instruction::Sub:
    case Instruction::Trunc:
    case Instruction::UIToFP:
    case Instruction::Xor:
    case Instruction::ZExt:
      return true;
    default:
      break;
    }
    return false;
  }
  static bool shouldBeAnalyzed(const Use &U) { return !isa<Constant>(U.get()); }
};

// This class is used in value_op_iterator and in GEPDepGraph to compute SCC
// containing GEPs, their base pointers and connected with PHIs.
struct GEPInstructionsTrait {
  static bool isSupportedOpcode(unsigned OpCode) {
    switch (OpCode) {
    case Instruction::GetElementPtr:
    case Instruction::PHI:
      return true;
    default:
      break;
    }
    return false;
  }
  static bool shouldBeAnalyzed(const Use &U) {
    auto *GEP = dyn_cast<GetElementPtrInst>(U.getUser());
    return GEP ? U.get() == GEP->getPointerOperand()
               : isa<PHINode>(U.getUser());
  }
};

struct AllInstructionsTrait {
  static bool isSupportedOpcode(unsigned OpCode) { return true; }
  static bool shouldBeAnalyzed(const Use &U) {
    return !isa<Constant>(U.get()) && !isa<BasicBlock>(U.get());
  }
};

// OpIterTy is a derivative of op_iterator/const_op_iterator.
//
// 1. De-referencing returns Value&;
// 2. Only Instruction and Argument are processed.
template <typename OpIterTy, typename ValueTy, typename FilterTrait>
class value_op_iterator
    : public filter_iterator_with_check<
          OpIterTy, std::function<bool(
                        typename std::iterator_traits<OpIterTy>::reference)>>,
      public FilterTrait {

  static_assert(
      std::is_same<Value, typename std::remove_cv<ValueTy>::type>::value,
      "ValueTy must be a CV-qualified Value");

  using OpRefTy = typename std::iterator_traits<OpIterTy>::reference;
  using OpFilterIterTy =
      filter_iterator_with_check<OpIterTy, std::function<bool(OpRefTy)>>;

public:
  using value_type = ValueTy;
  using reference = ValueTy &;
  using pointer = ValueTy *;
  reference operator*() const { return *OpFilterIterTy::operator*().get(); }
  pointer operator->() const { return &operator*(); }

  value_op_iterator() : OpFilterIterTy(mkDefault()) {}
  value_op_iterator(reference Val, bool EndOfRange)
      : OpFilterIterTy(setupOpIterators(Val, EndOfRange)) {}

private:
  static OpFilterIterTy setupOpIterators(reference Val, bool EndOfRange) {

    if (!isa<Instruction>(Val))
      return mkDefault();

    auto *Inst = cast<Instruction>(&Val);

    if (!FilterTrait::isSupportedOpcode(Inst->getOpcode()))
      return mkDefault();

    OpIterTy End = Inst->op_end();
    OpIterTy Begin = EndOfRange ? End : Inst->op_begin();
    return OpFilterIterTy(Begin, End, [](OpRefTy Use) -> bool {
      return FilterTrait::shouldBeAnalyzed(Use);
    });
  }
  static OpFilterIterTy mkDefault() {
    return OpFilterIterTy(OpIterTy(), OpIterTy(),
                          [](OpRefTy Use) -> bool { return false; });
  }
};

// Allows to traverse from def to use to counter stripPointerCasts.
// See cast_use_iterator.
inline bool isCastUse(const Use &Use) {
  if (auto *GEP = dyn_cast<GetElementPtrInst>(Use.getUser()))
    if (Use.get() == GEP->getPointerOperand() && GEP->hasAllZeroIndices())
      return true;

  return isa<BitCastInst>(Use.getUser());
}

// Fix mess with value_type/reference/pointer in *use_iterator.
template <typename UseIterTy, typename UseTy>
struct ValIterTy : public UseIterTy {
  // Override typedefs.
  using value_type = UseTy;
  using reference = UseTy &;
  using pointer = UseTy *;

  reference operator*() const { return this->UseIterTy::operator*(); }

  pointer operator->() const { return &this->UseIterTy::operator*(); }

  ValIterTy() {}
  ValIterTy(const UseIterTy &B) : UseIterTy(B) {}
};

// UseIterTy is a derivative of use_iterator/const_use_iterator through
// ValIterTy.
//
// 1. De-referencing returns Value* (more accurately User*);
// 2. Only Instruction and Argument are processed.
// 3. Allows to revert stripPointerCasts.
template <typename UseIterTy, typename ValueTy, typename UseTy>
class cast_use_iterator
    : public filter_iterator<UseIterTy, std::function<bool(UseTy &)>> {

  static_assert(std::is_same<Use, typename std::remove_cv<UseTy>::type>::value,
                "UseTy must be a CV-qualified Value");

  static_assert(
      std::is_same<Value, typename std::remove_cv<ValueTy>::type>::value,
      "ValueTy must be a CV-qualified Value");

  using UseFilterIterTy =
      filter_iterator<UseIterTy, std::function<bool(UseTy &)>>;

  using UseFilterIterTy::operator->;

public:
  using value_type = UseTy;
  using reference = UseTy &;
  using pointer = UseTy *;
  using iterator_category = typename UseFilterIterTy::iterator_category;

  cast_use_iterator() : UseFilterIterTy(mkDefault()) {}
  cast_use_iterator(ValueTy *Val, bool EndOfRange)
      : UseFilterIterTy(setupUseIterators(Val, EndOfRange)) {}

private:
  static UseFilterIterTy setupUseIterators(ValueTy *Val, bool EndOfRange) {

    if (!isa<Instruction>(Val) && !isa<Argument>(Val))
      return mkDefault();

    UseIterTy End = Val->use_end();
    UseIterTy Begin = EndOfRange ? End : Val->use_begin();
    return UseFilterIterTy(
        Begin, End, [](reference Use) -> bool { return isCastUse(Use); });
  }
  static UseFilterIterTy mkDefault() {
    return UseFilterIterTy(UseIterTy(), UseIterTy(),
                           [](reference Use) -> bool { return false; });
  }
};

// From Use to Value using getUser().
template <typename IterTy, typename ValueTy>
class UserDerefIter
    : public iterator_adaptor_base<
          UserDerefIter<IterTy, ValueTy>, IterTy,
          typename std::iterator_traits<IterTy>::iterator_category, ValueTy *,
          typename std::iterator_traits<IterTy>::difference_type, ValueTy *,
          ValueTy *> {

  static_assert(
      std::is_same<Value, typename std::remove_cv<ValueTy>::type>::value,
      "ValueTy must be a CV-qualified Value");

  using BaseTy = iterator_adaptor_base<
      UserDerefIter<IterTy, ValueTy>, IterTy,
      typename std::iterator_traits<IterTy>::iterator_category, ValueTy *,
      typename std::iterator_traits<IterTy>::difference_type, ValueTy *,
      ValueTy *>;

  using BaseTy::operator->;

public:
  using pointer = typename BaseTy::pointer;

  static UserDerefIter begin(pointer Val) { return UserDerefIter(Val, false); }
  static UserDerefIter end(pointer Val) { return UserDerefIter(Val, true); }

  pointer operator*() const { return this->wrapped().operator*().getUser(); }

  UserDerefIter() : BaseTy() {}
  UserDerefIter(pointer ValPtr, bool EndOfRange)
      : BaseTy(IterTy(ValPtr, EndOfRange)) {}
};

// Returns Value* instead of Value& from operator*().
// operator->() is hidden.
template <typename IterTy>
class ptr_iter : public iterator_adaptor_base<
                     ptr_iter<IterTy>, IterTy,
                     typename std::iterator_traits<IterTy>::iterator_category,
                     typename std::iterator_traits<IterTy>::pointer,
                     typename std::iterator_traits<IterTy>::difference_type,
                     void, typename std::iterator_traits<IterTy>::pointer> {

  using BaseTy = iterator_adaptor_base<
      ptr_iter<IterTy>, IterTy,
      typename std::iterator_traits<IterTy>::iterator_category,
      typename std::iterator_traits<IterTy>::pointer,
      typename std::iterator_traits<IterTy>::difference_type, void,
      typename std::iterator_traits<IterTy>::pointer>;

  using BaseTy::operator->;

public:
  using value_type = typename BaseTy::value_type;

  static ptr_iter begin(value_type Val) { return ptr_iter(Val, false); }
  static ptr_iter end(value_type Val) { return ptr_iter(Val, true); }
  static iterator_range<ptr_iter> deps(value_type Val) {
    return make_range(begin(Val), end(Val));
  }

  value_type operator*() const { return &this->wrapped().operator*(); }

  static bool isSupportedOpcode(unsigned OpCode) {
    return IterTy::isSupportedOpcode(OpCode);
  }

  bool isEnd() const { return this->wrapped().isEnd(); }

  ptr_iter() : BaseTy() {}
  ptr_iter(value_type ValPtr, bool EndOfRange)
      : BaseTy(IterTy(*ValPtr, EndOfRange)) {}
};

using arith_inst_dep_iterator = ptr_iter<
    value_op_iterator<User::op_iterator, Value, ArithInstructionsTrait>>;
using const_arith_inst_dep_iterator =
    ptr_iter<value_op_iterator<User::const_op_iterator, const Value,
                               ArithInstructionsTrait>>;

using all_inst_dep_iterator =
    ptr_iter<value_op_iterator<User::op_iterator, Value, AllInstructionsTrait>>;
using const_all_inst_dep_iterator =
    ptr_iter<value_op_iterator<User::const_op_iterator, const Value,
                               AllInstructionsTrait>>;

using gep_inst_dep_iterator =
    ptr_iter<value_op_iterator<User::op_iterator, Value, GEPInstructionsTrait>>;
using const_gep_inst_dep_iterator =
    ptr_iter<value_op_iterator<User::const_op_iterator, const Value,
                               GEPInstructionsTrait>>;

using cast_dep_iterator = UserDerefIter<
    cast_use_iterator<ValIterTy<Value::use_iterator, Use>, Value, Use>, Value>;
using const_cast_dep_iterator = UserDerefIter<
    cast_use_iterator<ValIterTy<Value::const_use_iterator, const Use>,
                      const Value, const Use>,
    const Value>;

// Specialization to use *arith_inst_dep_iterator.
template <typename ValuePtrTy> struct ArithDepGraph {
  ValuePtrTy ValuePtr;
  ArithDepGraph(ValuePtrTy V) : ValuePtr(V) {}
};

// Specialization to use *all_inst_dep_iterator.
template <typename ValuePtrTy> struct AllDepGraph {
  ValuePtrTy ValuePtr;
  AllDepGraph(ValuePtrTy V) : ValuePtr(V) {}
};

// Specialization to use *gep_inst_dep_iterator.
template <typename ValuePtrTy> struct GEPDepGraph {
  ValuePtrTy ValuePtr;
  GEPDepGraph(ValuePtrTy V) : ValuePtr(V) {}
};

// Specialization to use *cast_use_iterator.
template <typename ValuePtrTy> struct CastDepGraph {
  ValuePtrTy ValuePtr;
  CastDepGraph(ValuePtrTy V) : ValuePtr(V) {}
};
} // namespace soatoaosOP
} // namespace dtransOP

using namespace dtransOP::soatoaosOP;

// GraphTraits to compute closures using scc_iterator
// and children enumeration using arith_inst_dep_iterator and
// all_inst_dep_iterator.
template <> struct GraphTraits<ArithDepGraph<Value *>> {
  using NodeRef = Value *;
  using ChildIteratorType = arith_inst_dep_iterator;

  static NodeRef getEntryNode(ArithDepGraph<Value *> G) { return G.ValuePtr; }
  static ChildIteratorType child_begin(NodeRef N) {
    return arith_inst_dep_iterator::begin(N);
  }
  static ChildIteratorType child_end(NodeRef N) {
    return arith_inst_dep_iterator::end(N);
  }
};

template <> struct GraphTraits<ArithDepGraph<const Value *>> {
  using NodeRef = const Value *;
  using ChildIteratorType = const_arith_inst_dep_iterator;

  static NodeRef getEntryNode(ArithDepGraph<const Value *> G) {
    return G.ValuePtr;
  }
  static ChildIteratorType child_begin(NodeRef N) {
    return const_arith_inst_dep_iterator::begin(N);
  }
  static ChildIteratorType child_end(NodeRef N) {
    return const_arith_inst_dep_iterator::end(N);
  }
};

template <> struct GraphTraits<AllDepGraph<Value *>> {
  using NodeRef = Value *;
  using ChildIteratorType = all_inst_dep_iterator;

  static NodeRef getEntryNode(AllDepGraph<Value *> G) { return G.ValuePtr; }
  static ChildIteratorType child_begin(NodeRef N) {
    return all_inst_dep_iterator::begin(N);
  }
  static ChildIteratorType child_end(NodeRef N) {
    return all_inst_dep_iterator::end(N);
  }
};

template <> struct GraphTraits<AllDepGraph<const Value *>> {
  using NodeRef = const Value *;
  using ChildIteratorType = const_all_inst_dep_iterator;

  static NodeRef getEntryNode(AllDepGraph<const Value *> G) {
    return G.ValuePtr;
  }
  static ChildIteratorType child_begin(NodeRef N) {
    return const_all_inst_dep_iterator::begin(N);
  }
  static ChildIteratorType child_end(NodeRef N) {
    return const_all_inst_dep_iterator::end(N);
  }
};

template <> struct GraphTraits<GEPDepGraph<Value *>> {
  using NodeRef = Value *;
  using ChildIteratorType = gep_inst_dep_iterator;

  static NodeRef getEntryNode(GEPDepGraph<Value *> G) { return G.ValuePtr; }
  static ChildIteratorType child_begin(NodeRef N) {
    return gep_inst_dep_iterator::begin(N);
  }
  static ChildIteratorType child_end(NodeRef N) {
    return gep_inst_dep_iterator::end(N);
  }
};

template <> struct GraphTraits<GEPDepGraph<const Value *>> {
  using NodeRef = const Value *;
  using ChildIteratorType = const_gep_inst_dep_iterator;

  static NodeRef getEntryNode(GEPDepGraph<const Value *> G) {
    return G.ValuePtr;
  }
  static ChildIteratorType child_begin(NodeRef N) {
    return const_gep_inst_dep_iterator::begin(N);
  }
  static ChildIteratorType child_end(NodeRef N) {
    return const_gep_inst_dep_iterator::end(N);
  }
};

template <> struct GraphTraits<CastDepGraph<Value *>> {
  using NodeRef = Value *;
  using ChildIteratorType = cast_dep_iterator;

  static NodeRef getEntryNode(CastDepGraph<Value *> G) { return G.ValuePtr; }
  static ChildIteratorType child_begin(NodeRef N) {
    return cast_dep_iterator::begin(N);
  }
  static ChildIteratorType child_end(NodeRef N) {
    return cast_dep_iterator::end(N);
  }
};

template <> struct GraphTraits<CastDepGraph<const Value *>> {
  using NodeRef = const Value *;
  using ChildIteratorType = const_cast_dep_iterator;

  static NodeRef getEntryNode(CastDepGraph<const Value *> G) {
    return G.ValuePtr;
  }
  static ChildIteratorType child_begin(NodeRef N) {
    return const_cast_dep_iterator::begin(N);
  }
  static ChildIteratorType child_end(NodeRef N) {
    return const_cast_dep_iterator::end(N);
  }
};
} // namespace llvm

namespace llvm {
namespace dtransOP {
namespace soatoaosOP {
template <typename DepIterTy, typename ContainerTy> class base_scc_iterator {
  // Not going to modify ContainerTy.
  using SCCIterTy = typename ContainerTy::const_iterator;

  static_assert(
      std::is_same<DepIterTy, arith_inst_dep_iterator>::value ||
          std::is_same<DepIterTy, const_arith_inst_dep_iterator>::value,
      "DepIterTy must be a arith_inst_dep_iterator or "
      "const_arith_inst_dep_iterator ");
  static_assert(
      std::is_base_of<
          std::forward_iterator_tag,
          typename std::iterator_traits<DepIterTy>::iterator_category>::value,
      "DepIterTy should be forward_iterator");
  static_assert(
      std::is_base_of<
          std::forward_iterator_tag,
          typename std::iterator_traits<SCCIterTy>::iterator_category>::value,
      "SCCIterTy should be forward_iterator");

public:
  using value_type = typename DepIterTy::value_type;
  using reference = typename DepIterTy::reference;
  using pointer = typename DepIterTy::pointer;
  using iterator_category = std::forward_iterator_tag;
  using difference_type = typename DepIterTy::difference_type;

  base_scc_iterator &operator++() {
    assert(!DCFIt.isEnd() &&
           "Incorrect base_scc_iterator state in pre-increment");
    ++DCFIt;
    skip();
    return *this;
  }
  base_scc_iterator operator++(int) {
    base_scc_iterator retval = *this;
    ++(*this);
    return std::move(retval);
  }

  // We do not have operator->() due to arith_inst_dep_iterator.
  reference operator*() const { return *DCFIt; }

  bool operator==(const base_scc_iterator &It) const {
    auto E1 = DCFIt.isEnd();
    auto E2 = It.DCFIt.isEnd();
    if (!E1 && !E2)
      return this->DCFIt == It.DCFIt;
    return E1 == E2;
  }
  bool operator!=(const base_scc_iterator &It) const {
    return !(operator==(It));
  }

  static base_scc_iterator begin(const ContainerTy &C) {
    return base_scc_iterator(C.begin(), C.end());
  }
  static base_scc_iterator end(const ContainerTy &C) {
    return base_scc_iterator(C.end(), C.end());
  }
  static iterator_range<base_scc_iterator> deps(const ContainerTy &C) {
    return make_range(begin(C), end(C));
  }

private:
  SmallPtrSet<typename DepIterTy::value_type, 32> Visited;

  DepIterTy DCFIt = DepIterTy();

  SCCIterTy SCCIt = SCCIterTy();
  SCCIterTy SCCEnd = SCCIterTy();

  // DCFIt and SCCIt are valid, but may not be dereferenceable.
  void skip() {
    do {
      while (DCFIt.isEnd() && SCCIt != SCCEnd) {
        ++SCCIt;
        if (SCCIt != SCCEnd)
          DCFIt = DepIterTy::begin(*SCCIt);
      }
      if (DCFIt.isEnd() || Visited.count(*DCFIt) == 0)
        break;

      ++DCFIt;
    } while (true);

    if (!DCFIt.isEnd())
      Visited.insert(*DCFIt);
  }

  base_scc_iterator(SCCIterTy Begin, SCCIterTy End)
      : SCCIt(Begin), SCCEnd(End) {

    if (SCCIt == SCCEnd) {
      return;
    }

    DCFIt = DepIterTy::begin(*SCCIt);
    Visited.insert(Begin, End);
    skip();
  }
};

using scc_arith_inst_dep_iterator =
    base_scc_iterator<GraphTraits<ArithDepGraph<Value *>>::ChildIteratorType,
                      scc_iterator<ArithDepGraph<Value *>>::value_type>;

using const_scc_arith_inst_dep_iterator = base_scc_iterator<
    GraphTraits<ArithDepGraph<const Value *>>::ChildIteratorType,
    scc_iterator<ArithDepGraph<const Value *>>::value_type>;

struct Idioms;
struct ArrayIdioms;
struct StructIdioms;
struct DepCmp;
// The following class is owned by class DepManager.
// It is declared at top level for DenseMapInfo specialization.
// DenseMapInfo specialization is needed for DepManager::intern method.
class Dep;
} // namespace soatoaosOP
} // namespace dtransOP

using dtransOP::soatoaosOP::Dep;
// Forward specialization of DenseMapInfo for DepManager.
template <> struct DenseMapInfo<Dep *> {
  typedef Dep *Ptr;
  typedef const Dep *CPtr;

  /// Specialized methods.
  static inline Ptr getEmptyKey();
  static inline Ptr getTombstoneKey();
  static inline unsigned getHashValue(CPtr PtrVal);
  static inline bool isEqual(CPtr LHS, CPtr RHS);
};

namespace dtransOP {
namespace soatoaosOP {
class DepManager {
  // All Deps are owned by DepManager.
  DenseSet<Dep *> Deps;
  unsigned DepId = 0;
  unsigned Queries = 0;

public:
  const Dep *intern(Dep &&Tmp);
  ~DepManager();
};

// Array object interacts with remaining program:
//  - reading and updating its fields relying only on values of methods
//    argument or corresponding value of field in another object;
//    It corresponds to lvalue and rvalue primitives of accessing array:
//    A[i].
//  - returning pointer to its fields (needed to be immediately be
//  dereferenced);
//  - throwing exceptions;
//  - calling MemoryInterface for allocation/deallocation.
class Dep {
public:
  using Container = SmallSet<const Dep *, 5>;

private:
  enum KindTy : uint8_t {
    DK_Bottom,
    DK_Argument, // Function's Argument access.
    DK_Const,    // Some constant.
    DK_Store,    // Store Arg1 Arg2
    DK_Load,     // Load Arg1
    DK_GEP,      // Single index to access field: 'GEP Arg2, 0, Const'
    DK_Alloc,    // alloc (Arg1 = size) (Arg2 = remaining args)
    DK_Free,     // free (Arg1 = ptr) (Arg2 = remaining args)
    DK_Function, // Unknown function depending only on its Args from union
                 // below. It represents arithmetic-related function
                 // depending on operands only. Resulting value is completely
                 // determined by explicit operands (as opposed to load, for
                 // example). If one consider execution in a loop, then
                 // functions implicitly depends on conditional branch, so
                 // control flow dependence is handled separately as data
                 // dependence for conditional branch.
                 //
                 // See ComputeArrayMethodClassification::classify()
                 // for example re. BranchInst handling.
                 //
    DK_Call,     // Call (Const ? unknown : known) (Arg2 = remaining args)
                 //
                 // Known calls are methods of the same class as determined by
                 // 'this' parameter. It is coupled with method collection in
                 // populateCFGInformation in SOAToAOSOP.cpp.
    // These are Kinds for DenseMapInfo. Only set for 2 global instances.
    DK_Empty,
    DK_Tomb
  };

  KindTy Kind = DK_Bottom;
  union {
    const Dep *Arg1;
    // DK_Function
    const Container *Args;
    // DK_GEP, DK_Argument, DK_Call
    unsigned Const;
  };
  const Dep *Arg2 = nullptr;
  unsigned Id = 0;

  // Analysis of computed approximations for array's methods.
  friend struct ArrayIdioms;
  // Analysis of computed approximations for struct's methods.
  friend struct StructIdioms;
  // Analysis of fields common to ArrayIdioms and StructIdioms.
  friend struct Idioms;
  // Deterministic order in debug printing.
  friend struct DepCmp;

public:
  static const Dep *mkBottom(DepManager &M) { return M.intern(Dep()); }
  static const Dep *mkArg(DepManager &M, const Argument *Arg) {
    Dep Tmp;
    Tmp.Kind = DK_Argument;
    Tmp.Const = Arg->getArgNo();
    return M.intern(std::move(Tmp));
  }
  static const Dep *mkConst(DepManager &M) {
    Dep Tmp;
    Tmp.Kind = DK_Const;
    return M.intern(std::move(Tmp));
  }
  static const Dep *mkStore(DepManager &M, const Dep *Val, const Dep *Addr) {
    if (Val->Kind == DK_Bottom)
      return Val;
    if (Addr->Kind == DK_Bottom)
      return Addr;
    Dep Tmp;
    Tmp.Kind = DK_Store;
    Tmp.Arg1 = Val;
    Tmp.Arg2 = Addr;
    return M.intern(std::move(Tmp));
  }
  static const Dep *mkLoad(DepManager &M, const Dep *Addr) {
    if (Addr->Kind == DK_Bottom)
      return Addr;
    Dep Tmp;
    Tmp.Kind = DK_Load;
    Tmp.Arg1 = Addr;
    return M.intern(std::move(Tmp));
  }
  static const Dep *mkGEP(DepManager &M, const Dep *Addr, unsigned Index) {
    if (Addr->Kind == DK_Bottom)
      return Addr;

    Dep Tmp;
    Tmp.Kind = DK_GEP;
    Tmp.Const = Index;
    Tmp.Arg2 = Addr;
    return M.intern(std::move(Tmp));
  }
  static const Dep *mkAlloc(DepManager &M, const Dep *Size,
                            const Dep *OtherArgs) {
    if (Size->Kind == DK_Bottom)
      return Size;
    if (OtherArgs->Kind == DK_Bottom)
      return OtherArgs;
    Dep Tmp;
    Tmp.Kind = DK_Alloc;
    Tmp.Arg1 = Size;
    Tmp.Arg2 = OtherArgs;
    return M.intern(std::move(Tmp));
  }
  static const Dep *mkFree(DepManager &M, const Dep *Ptr,
                           const Dep *OtherArgs) {
    if (Ptr->Kind == DK_Bottom)
      return Ptr;
    if (OtherArgs->Kind == DK_Bottom)
      return OtherArgs;
    Dep Tmp;
    Tmp.Kind = DK_Free;
    Tmp.Arg1 = Ptr;
    Tmp.Arg2 = OtherArgs;
    return M.intern(std::move(Tmp));
  }
  static const Dep *mkFunction(DepManager &M, const Container &Args) {
    Dep Tmp;
    Tmp.Kind = DK_Function;
    auto *C = new Container();
    Tmp.Args = C;

    for (auto *E : Args)
      if (E->Kind == DK_Function)
        C->insert(E->Args->begin(), E->Args->end());
      else if (E->Kind == DK_Bottom)
        return E;
      // "Hide" constant in DK_Function
      else if (E->Kind == DK_Const)
        continue;
      else
        C->insert(E);
    if (C->empty())
      return mkConst(M);
    return M.intern(std::move(Tmp));
  }
  static const Dep *mkNonEmptyArgList(DepManager &M, const Container &Args) {
    assert(Args.size() != 0 && "Empty argument list in mkNonEmptyArgList");
    if (Args.size() == 1)
      return *Args.begin();
    return mkFunction(M, Args);
  }
  static const Dep *mkArgList(DepManager &M, const Container &Args) {
    if (Args.size() == 0)
      return mkConst(M);
    return mkNonEmptyArgList(M, Args);
  }
  static const Dep *mkCall(DepManager &M, const Dep *OtherArgs, bool IsKnown) {
    if (OtherArgs->Kind == DK_Bottom)
      return OtherArgs;
    Dep Tmp;
    Tmp.Kind = DK_Call;
    Tmp.Const = IsKnown ? 0 : 1;
    Tmp.Arg2 = OtherArgs;
    return M.intern(std::move(Tmp));
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, unsigned Indent, unsigned ClosingParen) const;
  LLVM_DUMP_METHOD void dump() const {
    dbgs() << "; ";
    print(dbgs(), 0, 0);
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  bool isBottom() const { return Kind == DK_Bottom; }

private:
  Dep(KindTy K = DK_Bottom) : Kind(K), Arg1(nullptr) {}

  // 'private:' clause related to:
  //  - Args management for DK_Function (Args is owned by given instance of
  //  Dep);
  //  - intern() and DenseMapInfo<> support for canonization in
  //  DepManager, all dynamically-allocated instances are owned
  //  by DepManager, see Deps field.
  friend struct DenseMapInfo<Dep *>;
  friend class DepManager;

  Dep(const Dep &) = delete;
  Dep(const Dep &&) = delete;
  Dep &operator=(const Dep &) = delete;
  Dep &operator=(const Dep &&) = delete;

  // Can be called from intern only.
  Dep(Dep &&Copy) {
    Kind = Copy.Kind;
    Arg2 = Copy.Arg2;
    Id = Copy.Id;
    switch (Kind) {
    case Dep::DK_Argument:
    case Dep::DK_GEP:
    case Dep::DK_Call:
      Const = Copy.Const;
      break;
    case Dep::DK_Function:
      Args = Copy.Args;
      break;
    default:
      Arg1 = Copy.Arg1;
      break;
    }
    assert(this->isEqual(Copy) && getHashValue() == Copy.getHashValue() &&
           "Inconsistent source/destination in move-ctor of Dep");
    Copy.Kind = DK_Tomb;
    Copy.Arg1 = nullptr;
  }

  ~Dep() {
    if (Kind == DK_Function)
      delete Args;
    Kind = DK_Tomb;
    Arg1 = reinterpret_cast<Dep *>(0x1a1a);
    Arg2 = reinterpret_cast<Dep *>(0x1a1a1e);
  }

  // Only one level dereference is needed, see Dep::intern and Dep::mk*
  // functions for explanation. No need to use Ids here.
  bool isEqual(const Dep &Other) const {
    if (this == &Other)
      return true;
    if (Kind != Other.Kind || Arg2 != Other.Arg2)
      return false;

    // Process union.
    switch (Kind) {
    case Dep::DK_Argument:
    case Dep::DK_GEP:
    case Dep::DK_Call:
      return Const == Other.Const;
    case Dep::DK_Function:
      // Missing SmallSet::operator ==
      if (Args->size() != Other.Args->size())
        return false;
      for (auto A : *Args)
        if (Other.Args->count(A) == 0)
          return false;
      return true;
    default:
      break;
    }
    return Arg1 == Other.Arg1;
  }

  // Only one level dereference is needed, see Dep::intern and Dep::mk*
  // functions for explanation.
  unsigned getHashValue() const {
    unsigned Key = 0;
    switch (Kind) {
    case Dep::DK_Argument:
    case Dep::DK_GEP:
    case Dep::DK_Call:
      Key = Const;
      break;
    case Dep::DK_Function:
      Key = Args->size() + (*Args->begin())->Id;
      break;
    default:
      Key = Arg1 ? Arg1->Id : -1;
      break;
    }
    return DenseMapInfo<unsigned>::getHashValue(Kind) +
           DenseMapInfo<std::pair<unsigned, unsigned>>::getHashValue(
               std::make_pair(Key, Arg2 ? Arg2->Id : 0));
  }

  // Used in DenseMapInfo. Reserved address.
  static Dep Empty;
  // Used in DenseMapInfo. Reserved address.
  static Dep Tombstone;
};

// Class performing actual computations of Dep.
class DepCompute;

class DepMap : DepManager {
  friend class DepCompute;

  DenseMap<const Value *, const Dep *> ValDependencies;

public:
  const Dep *getApproximation(const Value *V) const {
    auto It = ValDependencies.find(V);
    assert(It != ValDependencies.end() && "Dependency should be found");
    return It->second;
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  class DepAnnotatedWriter : public AssemblyAnnotationWriter {
    const DepMap &DM;

  public:
    DepAnnotatedWriter(const DepMap &DM) : DM(DM) {}

    void emitInstructionAnnot(const Instruction *I,
                              formatted_raw_ostream &OS) override {
      auto It = DM.ValDependencies.find(I);
      if (It == DM.ValDependencies.end()) {
        OS << " ; ERROR: deps are not computed\n";
        return;
      }
      OS << "; ";
      It->second->print(OS, 0, 0);
    }
  };
#endif
};

// Class performing actual computations of Dep.
// Suitable for analysis of methods (Method field) of
// class (ClassType field).
//
// Exposes memory accesses and calls with emphasis on accesses to ClassType
// fields.
//
// Encapsulates short-living state.
//
// Lit-tests can be written with SOAToAOSOPApproximationDebug pass.
//
// Essentially class encapsulates parameters and result (DM) for
// computeDepApproximation.
class DepCompute {
  // allocation/deallocation recognition.
  DTransSafetyInfo &DTInfo;
  // Layout information, like pointer-sized integer, etc.
  // See isSafeBitCast.
  const DataLayout &DL;
  // allocation/deallocation recognition.
  const TargetLibraryInfo &TLI;

  // Method to analyse.
  const Function *Method;
  // Structured accesses are computed with respect to ClassType.
  // See computeDepApproximation::IsFieldAccessGEP.
  const DTransStructType *ClassType;

  // Output of computeDepApproximation.
  DepMap &DM;

  // Compute IR approximation for Value, which obtained through arithmetic
  // instruction.
  const Dep *computeValueDep(const Value *Val) const;

  // Compute IR approximation for Instruction.
  // Helper function for computeDepApproximation.
  const Dep *computeInstDep(const Instruction *I) const;

public:
  DepCompute(DTransSafetyInfo &DTInfo,
             // Need to compare pointers and integers of pointer size
             const DataLayout &DL,
             // Need to check free/malloc
             const TargetLibraryInfo &TLI,
             // Method to analyse.
             const Function *Method,
             // Class type of Method.
             const DTransStructType *ClassType, DepMap &DM)
      : DTInfo(DTInfo), DL(DL), TLI(TLI), Method(Method), ClassType(ClassType),
        DM(DM) {}

  // Compute IR approximation using Dep:
  //  - loads/stores/arguments/allocation and deallocation calls are
  //  explicitly presented;
  //  - structured addresses, i.e. GEPs to fields are attempted to be
  //  preserved;
  //  - arithmetic instructions are abstracted away as much as possible using
  //  DK_Function.
  //
  // Returned value 'true' means that all essential instructions
  // (store/load/ret/etc) have approximation computed (possibly Bottom in debug
  // configuration).
  //
  // Returned value 'false' means that there could be some instructions without
  // approximation.
  bool computeDepApproximation() const;
};

extern cl::opt<bool> DTransSOAToAOSOPComputeAllDep;
} // namespace soatoaosOP

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
struct SOAToAOSOPApproximationDebugResult : public DepMap {};
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Special LLVM-type inconsistencies, which need to be processed in SOAToAOSOP.
namespace soatoaosOP {
// Check if bitcast can be ignored, because its consumers are load/stores
// and the size of accessed memory does not change due to bitcast.
//
// "load (bitcast(ptr))" is ok if load produces the same
// value as "load ptr". Similarly for store.
//
// TODO: IntToPtr and BitCastLikeGEP instructions can also be
// considered as safe instructions in some conditions. But, it is not
// needed to enable SOAToAOSOP for the benchmarks. If needed, isSafeIntToPtr
// and isBitCastLikeGep will be implemented to handle the below cases.
//
// isSafeIntToPtr:
//    %ptr  = bitcast <type>** %0 to intptr_t*
//    %ival = load %ptr
//    %val  = inttoptr %ival to <type>*
//
// isBitCastLikeGep:
//    %ptr = bitcast %base* to <some type>*
//    store <val> %ptr,
//     where <val> has size of first field.
//
inline bool isSafeBitCast(const DataLayout &DL, const Value *V,
                          const PtrTypeAnalyzer &PTA) {
  auto *BC = dyn_cast<BitCastInst>(V);
  if (!BC)
    return false;

  auto *BCInfo = PTA.getValueTypeInfo(BC);
  if (!BCInfo)
    return false;
  // Check first and use if BCInfo is dominant type.
  auto *BCTy = PTA.getDominantType(*BCInfo, ValueTypeInfo::VAT_Use);
  uint64_t ElemSize = -1ULL;
  if (BCTy) {
    if (!BCTy->isPointerTy())
      return false;
    DTransType *ETy = BCTy->getPointerElementType();
    Type *S = ETy->getLLVMType();
    if (!S->isSized())
      return false;
    ElemSize = DL.getTypeStoreSize(S);
  } else {
    // If it doesn't have dominant type, makes sure it is safe bitcast.
    auto *FromInfo = PTA.getValueTypeInfo(BC->getOperand(0));
    if (!FromInfo)
      return false;
    auto &AliasSet = FromInfo->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use);
    for (auto *FromTy : AliasSet) {
      if (!FromTy->isPointerTy())
        return false;
      DTransType *ETy = FromTy->getPointerElementType();
      Type *S = ETy->getLLVMType();
      if (!S->isSized())
        return false;
      if (ElemSize == -1ULL)
        ElemSize = DL.getTypeStoreSize(S);
      else if (ElemSize != DL.getTypeStoreSize(S))
        return false;
    }
  }
   if (ElemSize == -1ULL)
     return false;

  // Value is dereferenced.
  for (auto &U : BC->uses()) {
    if (auto *LI = dyn_cast<LoadInst>(U.getUser())) {
      if (BC == LI->getPointerOperand() &&
          DL.getTypeStoreSize(LI->getType()) == ElemSize)
        continue;
    } else if (auto *SI = dyn_cast<StoreInst>(U.getUser())) {
      if (BC == SI->getPointerOperand() &&
          DL.getTypeStoreSize(SI->getValueOperand()->getType()) == ElemSize)
        continue;
    }
    return false;
  }
  return true;
}

} // namespace soatoaosOP
} // namespace dtransOP

// DenseMapInfo<Dep *> specialization.
Dep *DenseMapInfo<Dep *>::getEmptyKey() { return &Dep::Empty; }
Dep *DenseMapInfo<Dep *>::getTombstoneKey() { return &Dep::Tombstone; }
unsigned DenseMapInfo<Dep *>::getHashValue(CPtr Ptr) {
  return Ptr->getHashValue();
}
bool DenseMapInfo<Dep *>::isEqual(CPtr LHS, CPtr RHS) {
  return LHS->isEqual(*RHS);
}
} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSOPEFFECTS_H
