//===---------------- SOAToAOSEffects.h - Part of SOAToAOSPass ------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements iterators and other helper classes for
// SOAToAOSTransformImpl::CandidateSideEffectsInfo related for analysis of
// side-effects.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOSEFFECTS_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOSEFFECTS_H

#if !INTEL_INCLUDE_DTRANS
#error SOAToAOSEffects.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#include "Intel_DTrans/Analysis/DTransAnalysis.h"

#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/iterator.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/Support/FormattedStream.h"
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// DepCompute
#define DTRANS_SOADEP "dtrans-soatoaos-deps"

namespace llvm {
namespace dtrans {
namespace soatoaos {

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
  static inline bool isSupportedOpcode(unsigned OpCode) {
    // FIXME: FP exception handling.
    switch (OpCode) {
    case Instruction::Add:
    case Instruction::And:
    case Instruction::BitCast:
    case Instruction::ExtractValue:
    case Instruction::FMul:
    case Instruction::FPToUI:
    case Instruction::FPToSI:
    case Instruction::GetElementPtr:
    case Instruction::ICmp:
    case Instruction::InsertValue:
    case Instruction::IntToPtr:
    case Instruction::LShr:
    case Instruction::Mul:
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
  static inline bool shouldBeAnalyzed(const Use &U) {
    return !isa<Constant>(U.get());
  }
};

// This class is used in value_op_iterator and in GEPDepGraph to compute SCC
// containing GEPs, their base pointers and connected with PHIs.
struct GEPInstructionsTrait {
  static inline bool isSupportedOpcode(unsigned OpCode) {
    switch (OpCode) {
    case Instruction::GetElementPtr:
    case Instruction::PHI:
      return true;
    default:
      break;
    }
    return false;
  }
  static inline bool shouldBeAnalyzed(const Use &U) {
    auto *GEP = dyn_cast<GetElementPtrInst>(U.getUser());
    return GEP ? U.get() == GEP->getPointerOperand()
               : isa<PHINode>(U.getUser());
  }
};

struct AllInstructionsTrait {
  static inline bool isSupportedOpcode(unsigned OpCode) { return true; }
  static inline bool shouldBeAnalyzed(const Use &U) {
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
  static inline OpFilterIterTy setupOpIterators(reference Val,
                                                bool EndOfRange) {

    if (!isa<Instruction>(Val))
      return mkDefault();

    auto *Inst = cast<Instruction>(&Val);

    OpIterTy Begin, End;

    if (!FilterTrait::isSupportedOpcode(Inst->getOpcode()))
      return mkDefault();

    End = Inst->op_end();
    Begin = EndOfRange ? End : Inst->op_begin();
    return OpFilterIterTy(Begin, End, [](OpRefTy Use) -> bool {
      return FilterTrait::shouldBeAnalyzed(Use);
    });
  }
  static inline OpFilterIterTy mkDefault() {
    return OpFilterIterTy(OpIterTy(), OpIterTy(),
                          [](OpRefTy Use) -> bool { return false; });
  }
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

  static inline ptr_iter begin(value_type Val) { return ptr_iter(Val, false); }
  static inline ptr_iter end(value_type Val) { return ptr_iter(Val, true); }
  static inline iterator_range<ptr_iter> deps(value_type Val) {
    return make_range(begin(Val), end(Val));
  }

  value_type operator*() const { return &this->wrapped().operator*(); }

  static inline bool isSupportedOpcode(unsigned OpCode) {
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
} // namespace soatoaos
} // namespace dtrans

using namespace dtrans::soatoaos;

// GraphTraits to compute closures using scc_iterator
// and children enumeration using arith_inst_dep_iterator and
// all_inst_dep_iterator.
template <> struct GraphTraits<ArithDepGraph<Value *>> {
  using NodeRef = Value *;
  using ChildIteratorType = arith_inst_dep_iterator;

  static inline NodeRef getEntryNode(ArithDepGraph<Value *> G) {
    return G.ValuePtr;
  }
  static inline ChildIteratorType child_begin(NodeRef N) {
    return arith_inst_dep_iterator::begin(N);
  }
  static inline ChildIteratorType child_end(NodeRef N) {
    return arith_inst_dep_iterator::end(N);
  }
};

template <> struct GraphTraits<ArithDepGraph<const Value *>> {
  using NodeRef = const Value *;
  using ChildIteratorType = const_arith_inst_dep_iterator;

  static inline NodeRef getEntryNode(ArithDepGraph<const Value *> G) {
    return G.ValuePtr;
  }
  static inline ChildIteratorType child_begin(NodeRef N) {
    return const_arith_inst_dep_iterator::begin(N);
  }
  static inline ChildIteratorType child_end(NodeRef N) {
    return const_arith_inst_dep_iterator::end(N);
  }
};

template <> struct GraphTraits<AllDepGraph<Value *>> {
  using NodeRef = Value *;
  using ChildIteratorType = all_inst_dep_iterator;

  static inline NodeRef getEntryNode(AllDepGraph<Value *> G) {
    return G.ValuePtr;
  }
  static inline ChildIteratorType child_begin(NodeRef N) {
    return all_inst_dep_iterator::begin(N);
  }
  static inline ChildIteratorType child_end(NodeRef N) {
    return all_inst_dep_iterator::end(N);
  }
};

template <> struct GraphTraits<AllDepGraph<const Value *>> {
  using NodeRef = const Value *;
  using ChildIteratorType = const_all_inst_dep_iterator;

  static inline NodeRef getEntryNode(AllDepGraph<const Value *> G) {
    return G.ValuePtr;
  }
  static inline ChildIteratorType child_begin(NodeRef N) {
    return const_all_inst_dep_iterator::begin(N);
  }
  static inline ChildIteratorType child_end(NodeRef N) {
    return const_all_inst_dep_iterator::end(N);
  }
};

template <> struct GraphTraits<GEPDepGraph<Value *>> {
  using NodeRef = Value *;
  using ChildIteratorType = gep_inst_dep_iterator;

  static inline NodeRef getEntryNode(GEPDepGraph<Value *> G) {
    return G.ValuePtr;
  }
  static inline ChildIteratorType child_begin(NodeRef N) {
    return gep_inst_dep_iterator::begin(N);
  }
  static inline ChildIteratorType child_end(NodeRef N) {
    return gep_inst_dep_iterator::end(N);
  }
};

template <> struct GraphTraits<GEPDepGraph<const Value *>> {
  using NodeRef = const Value *;
  using ChildIteratorType = const_gep_inst_dep_iterator;

  static inline NodeRef getEntryNode(GEPDepGraph<const Value *> G) {
    return G.ValuePtr;
  }
  static inline ChildIteratorType child_begin(NodeRef N) {
    return const_gep_inst_dep_iterator::begin(N);
  }
  static inline ChildIteratorType child_end(NodeRef N) {
    return const_gep_inst_dep_iterator::end(N);
  }
};
} // namespace llvm

namespace llvm {
namespace dtrans {
namespace soatoaos {
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

  static inline base_scc_iterator begin(const ContainerTy &C) {
    return base_scc_iterator(C.begin(), C.end());
  }
  static inline base_scc_iterator end(const ContainerTy &C) {
    return base_scc_iterator(C.end(), C.end());
  }
  static inline iterator_range<base_scc_iterator> deps(const ContainerTy &C) {
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
      if (DCFIt.isEnd() || Visited.find(*DCFIt) == Visited.end())
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
} // namespace dtrans
} // namespace soatoaos

using dtrans::soatoaos::Dep;
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

namespace dtrans {
namespace soatoaos {
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
    DK_Store,    // Store val base
    DK_Load,     // Load base
    DK_GEP,      // Single index to access field: GEP base, 0, const
    DK_Alloc,    // alloc (size) remaining args
    DK_Free,     // free (ptr) remaining args
    DK_Function, // Unknown function depending only on its Args from union
                 // below. It represents arithmetic-related function
                 // depending on operands only. Resulting value is completely
                 // determined by explicit operands (as opposed to load, for
                 // example). If one consider execution in a loop, then
                 // functions implicitly depends on conditional branch, so
                 // control flow dependence is handled separately as data
                 // dependence for conditional branch.
    DK_Call,     // known call if Const is 0 and
                 // unknown if Const is non-0.
    // There are Kinds for DenseMapInfo.
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
  static inline const Dep *mkBottom(DepManager &M) { return M.intern(Dep()); }
  static inline const Dep *mkArg(DepManager &M, const Argument *Arg) {
    Dep Tmp;
    Tmp.Kind = DK_Argument;
    Tmp.Const = Arg->getArgNo();
    return M.intern(std::move(Tmp));
  }
  static inline const Dep *mkConst(DepManager &M) {
    Dep Tmp;
    Tmp.Kind = DK_Const;
    return M.intern(std::move(Tmp));
  }
  static inline const Dep *mkStore(DepManager &M, const Dep *Val,
                                   const Dep *Addr) {
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
  static inline const Dep *mkLoad(DepManager &M, const Dep *Addr) {
    if (Addr->Kind == DK_Bottom)
      return Addr;
    Dep Tmp;
    Tmp.Kind = DK_Load;
    Tmp.Arg1 = Addr;
    return M.intern(std::move(Tmp));
  }
  static inline const Dep *mkGEP(DepManager &M, const Dep *Addr,
                                 unsigned Index) {
    if (Addr->Kind == DK_Bottom)
      return Addr;

    Dep Tmp;
    Tmp.Kind = DK_GEP;
    Tmp.Const = Index;
    Tmp.Arg2 = Addr;
    return M.intern(std::move(Tmp));
  }
  static inline const Dep *mkAlloc(DepManager &M, const Dep *Size,
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
  static inline const Dep *mkFree(DepManager &M, const Dep *Ptr,
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
  static inline const Dep *mkFunction(DepManager &M, const Container &Args) {
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
  static inline const Dep *mkNonEmptyArgList(DepManager &M,
                                             const Container &Args) {
    assert(Args.size() != 0 && "Empty argument list in mkNonEmptyArgList");
    if (Args.size() == 1)
      return *Args.begin();
    return mkFunction(M, Args);
  }
  static inline const Dep *mkArgList(DepManager &M, const Container &Args) {
    if (Args.size() == 0)
      return mkConst(M);
    return mkNonEmptyArgList(M, Args);
  }
  static inline const Dep *mkCall(DepManager &M, const Dep *OtherArgs,
                                  bool IsKnown) {
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
        if (Other.Args->find(A) == Other.Args->end())
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
    if (It == ValDependencies.end())
      return nullptr;
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
// Lit-tests can be written with SOAToAOSApproximationDebug pass.
class DepCompute {
  const DTransAnalysisInfo &DTInfo;
  const DataLayout &DL;
  const TargetLibraryInfo &TLI;
  const Function *Method;
  const StructType *ClassType;

  // Output of computeDepApproximation.
  DepMap &DM;

  const Dep *computeValueDep(const Value *Val);

public:
  DepCompute(const DTransAnalysisInfo &DTInfo,
             // Need to compare pointers and integers of pointer size
             const DataLayout &DL,
             // Need to check free/malloc
             const TargetLibraryInfo &TLI,
             // Method to analyse.
             const Function *Method,
             // Class type of Method.
             const StructType *ClassType, DepMap &DM)
      : DTInfo(DTInfo), DL(DL), TLI(TLI), Method(Method), ClassType(ClassType),
        DM(DM) {}

  // Returned value 'true' means that all essential instructions
  // (store/load/ret/etc) have approximation computed (possibly Bottom in debug
  // configuration).
  //
  // Returned value 'false' means that there could be some instructions without
  // approximation.
  bool computeDepApproximation(
      std::function<bool(const Function *)> IsKnownCallCheck);
};

extern cl::opt<bool> DTransSOAToAOSComputeAllDep;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Structure name to use in SOAToAOSApproximationDebug.
extern cl::opt<std::string> DTransSOAToAOSApproxTypename;

// Calls to mark as known in SOAToAOSApproximationDebug.
extern cl::list<std::string> DTransSOAToAOSApproxKnown;

StructType *getStructTypeOfArray(Function &F);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace soatoaos

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
struct SOAToAOSApproximationDebugResult : public DepMap {};
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Special LLVM-type inconsistencies, which need to be processed in SOAToAOS.
namespace soatoaos {
// Check if bitcast can be ignored, because its consumers are load/stores
// and the size of accessed memory does not change due to bitcast.
//
// "load (bitcast(ptr))" is ok if load produces the same
// value as "load ptr". Similarly for store.
inline bool isSafeBitCast(const DataLayout &DL, const Value *V) {
  auto *BC = dyn_cast<BitCastInst>(V);
  if (!BC)
    return false;

  // Dereferenced value is the same.
  Type *STy = BC->getOperand(0)->getType();
  Type *DTy = BC->getType();
  if (!isa<PointerType>(STy) || !isa<PointerType>(DTy))
    return false;

  auto *D = DTy->getPointerElementType();
  auto *S = STy->getPointerElementType();
  if (!D->isSized() || !S->isSized() ||
      DL.getTypeStoreSize(D) != DL.getTypeStoreSize(S))
    return false;

  // Value is dereferenced.
  for (auto &U : BC->uses()) {
    if (isa<LoadInst>(U.getUser()))
      continue;
    else if (auto *S = dyn_cast<StoreInst>(U.getUser()))
      if (BC == S->getPointerOperand())
        continue;
    return false;
  }
  return true;
}

// It is analysis counter peephole transformation.
// Check if inttoptr can be ignored, because
//  it is obtained the following way:
//    %ptr  = bitcast <type>** %0 to intptr_t*
//    %ival = load %ptr
//    %val  = inttoptr %ival to <type>*
inline bool isSafeIntToPtr(const DataLayout &DL, const Value *V) {
  auto *I2P = dyn_cast<IntToPtrInst>(V);
  if (!I2P || I2P->getAddressSpace())
    return false;

  if (DL.getTypeStoreSize(I2P->getType()) !=
      DL.getTypeStoreSize(I2P->getOperand(0)->getType()))
    return false;

  auto *L = dyn_cast<LoadInst>(I2P->getOperand(0));
  if (!L)
    return false;

  auto *BC =
      dyn_cast<BitCastInst>(L->getPointerOperand());
  if (!BC)
    return false;

  auto *PtrType = dyn_cast<PointerType>(BC->getOperand(0)->getType());

  if (!PtrType)
    return false;

  return V->getType() == PtrType->getPointerElementType();
}

// It is analysis counter peephole transformation.
//  %ptr = bitcast %base* to <some type>*
//  store <val> %ptr,
// where <val> has size of first field.
inline bool isBitCastLikeGep(const DataLayout &DL, const Value *V) {
  if (!isa<BitCastInst>(V))
    return false;

  auto *BC = cast<BitCastInst>(V);
  auto *FromTy = BC->getOperand(0)->getType();
  auto *ToTy = BC->getType();
  if (!isa<PointerType>(ToTy) || !BC->hasOneUse() || !isa<PointerType>(FromTy))
    return false;

  auto *FromPointeeTy = dyn_cast<StructType>(FromTy->getPointerElementType());
  auto *ToPointeeTy = ToTy->getPointerElementType();

  if (!FromPointeeTy || FromPointeeTy->isOpaque() ||
      !FromPointeeTy->isSized() || FromPointeeTy->getNumElements() == 0)
    return false;

  if (DL.getTypeStoreSize(FromPointeeTy->getElementType(0)) !=
      DL.getTypeStoreSize(ToPointeeTy))
    return false;

  auto *U = BC->use_begin()->getUser();

  if (!isa<StoreInst>(U))
    return false;

  auto *S = cast<StoreInst>(U);
  if (S->getPointerOperand() != BC)
    return false;

  return true;
}
} // namespace soatoaos
} // namespace dtrans

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

#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSEFFECTS_H
