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
// SOAToAOSTransformImpl::CandidateSideEffectsInfo included to SOAToAOSPass.cpp
// only.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/SCCIterator.h"
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/Support/FormattedStream.h"
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

namespace {
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

// OpIterTy is a derivative of op_iterator/const_op_iterator.
//
// 1. De-referencing returns Value&;
// 2. Only Instruction and Argument are processed.
template <typename OpIterTy, typename ValueTy>
class value_op_iterator
    : public filter_iterator_with_check<
          OpIterTy, std::function<bool(
                        typename std::iterator_traits<OpIterTy>::reference)>> {

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

  static inline bool isSupportedOpcode(unsigned OpCode) {
    switch (OpCode) {
    case Instruction::Add:
    case Instruction::BitCast:
    case Instruction::ExtractValue:
    case Instruction::FMul:
    case Instruction::FPToUI:
    case Instruction::GetElementPtr:
    case Instruction::ICmp:
    case Instruction::Mul:
    case Instruction::PHI:
    case Instruction::SExt:
    case Instruction::Select:
    case Instruction::Shl:
    case Instruction::Trunc:
    case Instruction::UIToFP:
    case Instruction::ZExt:
      return true;
    default:
      break;
    }
    return false;
  }

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

    switch (Inst->getOpcode()) {
    // Computations.
    // FIXME: FP exception handling.
    case Instruction::Add:
    case Instruction::BitCast:
    case Instruction::ExtractValue:
    case Instruction::FMul:
    case Instruction::FPToUI:
    case Instruction::GetElementPtr:
    case Instruction::ICmp:
    case Instruction::Mul:
    case Instruction::PHI:
    case Instruction::SExt:
    case Instruction::Select:
    case Instruction::Shl:
    case Instruction::Trunc:
    case Instruction::UIToFP:
    case Instruction::ZExt:
      Begin = Inst->op_begin();
      End = Inst->op_end();
      break;
    default:
      return mkDefault();
    }
    return OpFilterIterTy(
        (EndOfRange ? End : Begin), End, [](OpRefTy Use) -> bool {
          return !isa<Constant>(Use.get());
        });
  }
  static inline OpFilterIterTy mkDefault() {
    return OpFilterIterTy(OpIterTy(), OpIterTy(),
                          [](OpRefTy Use) -> bool { return false; });
  }
};
// There are only 2 instances of the template above.
template class value_op_iterator<User::op_iterator, Value>;
template class value_op_iterator<User::const_op_iterator, const Value>;

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
// There are only 2 instances of the template above.
template class ptr_iter<value_op_iterator<User::op_iterator, Value>>;
template class ptr_iter<
    value_op_iterator<User::const_op_iterator, const Value>>;

using dep_iterator = ptr_iter<value_op_iterator<User::op_iterator, Value>>;
using const_dep_iterator =
    ptr_iter<value_op_iterator<User::const_op_iterator, const Value>>;

// Specialization to use *dep_iterator.
template <typename ValuePtrTy> struct DCFGraph {
  ValuePtrTy ValuePtr;
  DCFGraph(ValuePtrTy V) : ValuePtr(V) {}
};
} // namespace

// GraphTraits to compute closures using scc_iterator
// and children enumeration using dep_iterator.
namespace llvm {
template <> struct GraphTraits<DCFGraph<Value *>> {
  using NodeRef = Value *;
  using ChildIteratorType = dep_iterator;

  static inline NodeRef getEntryNode(DCFGraph<Value *> G) { return G.ValuePtr; }
  static inline ChildIteratorType child_begin(NodeRef N) {
    return dep_iterator::begin(N);
  }
  static inline ChildIteratorType child_end(NodeRef N) {
    return dep_iterator::end(N);
  }
};

template <> struct GraphTraits<DCFGraph<const Value *>> {
  using NodeRef = const Value *;
  using ChildIteratorType = const_dep_iterator;

  static inline NodeRef getEntryNode(DCFGraph<const Value *> G) {
    return G.ValuePtr;
  }
  static inline ChildIteratorType child_begin(NodeRef N) {
    return const_dep_iterator::begin(N);
  }
  static inline ChildIteratorType child_end(NodeRef N) {
    return const_dep_iterator::end(N);
  }
};
} // namespace llvm

namespace {
template <typename DepIterTy, typename ContainerTy> class base_scc_iterator {
  // Not going to modify ContainerTy.
  using SCCIterTy = typename ContainerTy::const_iterator;

  static_assert(std::is_same<DepIterTy, dep_iterator>::value ||
                    std::is_same<DepIterTy, const_dep_iterator>::value,
                "DepIterTy must be a dep_iterator or const_dep_iterator ");
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

  // We do not have operator->() due to dep_iterator.
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

using scc_dep_iterator =
    base_scc_iterator<GraphTraits<DCFGraph<Value *>>::ChildIteratorType,
                      scc_iterator<DCFGraph<Value *>>::value_type>;

using const_scc_dep_iterator =
    base_scc_iterator<GraphTraits<DCFGraph<const Value *>>::ChildIteratorType,
                      scc_iterator<DCFGraph<const Value *>>::value_type>;

// The following class is owned by class DepManager.
// It is declared at top level for DenseMapInfo specialization.
// DenseMapInfo specialization is needed for DepManager::intern method.
class Dep;
} // namespace

namespace llvm {
template <> struct DenseMapInfo<Dep *> {
  typedef Dep *Ptr;
  typedef const Dep *CPtr;

  /// Specialized methods.
  static inline Ptr getEmptyKey();
  static inline Ptr getTombstoneKey();
  static inline unsigned getHashValue(CPtr PtrVal);
  static inline bool isEqual(CPtr LHS, CPtr RHS);
};
} // namespace llvm

namespace {
class DepManager {
  // All Deps are owned by DepManager.
  DenseSet<Dep *> Deps;
  unsigned DepId = 0;

public:
  inline const Dep *intern(Dep &&Tmp);
  inline ~DepManager();
};

struct Idioms;
struct DepCmp;

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

  // Analysis of computed approximations.
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

  Dep(KindTy K = DK_Bottom) : Kind(K), Arg1(nullptr) {}

private:

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

// Empty.Id == 0;
Dep Dep::Empty(DK_Empty);
// Tombstone.Id == 0;
Dep Dep::Tombstone(DK_Tomb);

const Dep *DepManager::intern(Dep &&Tmp) {
  assert(DepId == Deps.size() && "Inconsistent state of DepManager::Deps");
  Tmp.Id = ++DepId;
  auto It = Deps.find(&Tmp);
  if (It != Deps.end()) {
    --DepId;
    return *It;
  }
  return *Deps.insert(new Dep(std::move(Tmp))).first;
}

DepManager::~DepManager() {
  DepId -= Deps.size();
  assert(DepId == 0 && "Inconsistent state of DepManager::Deps");

  // Deps checks Dep's internal while removing elements.
  std::vector<const Dep *> Temp;
  Temp.insert(Temp.end(), Deps.begin(), Deps.end());
  Deps.clear();
  for (auto *Ptr : Temp)
    delete Ptr;
}

// Class performing actual computations of Dep.
class DepCompute;

class DepMap : DepManager {
  friend class DepCompute;

protected:
  DenseMap<const Value *, const Dep *> ValDependencies;

public:
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

// Check if bitcast can be ignored, because its consumers are load/stores
// and the size of accessed memory does not change due to bitcast.
//
// "load (bitcast(ptr))" is ok if load produces the same
// value as "load ptr". Similarly for store.
bool isSafeBitCast(const DataLayout &DL, const Value *V) {
  if (!isa<BitCastInst>(V))
    return false;

  // Dereferenced value is the same.
  Type *STy = cast<BitCastInst>(V)->getOperand(0)->getType();
  if (!isa<PointerType>(STy))
    return false;

  if (DL.getTypeStoreSize(V->getType()->getPointerElementType()) !=
      DL.getTypeStoreSize(STy->getPointerElementType()))
    return false;

  // Value is dereferenced.
  for (auto &U : cast<BitCastInst>(V)->uses()) {
    if (isa<LoadInst>(U.getUser()))
      continue;
    else if (auto *S = dyn_cast<StoreInst>(U.getUser()))
      if (V == S->getPointerOperand())
        continue;
    return false;
  }
  return true;
}

// Class performing actual computations of Dep.
// Suitable for analysis of methods (Method field) of
// class (ClassType field).
//
// Exposes memory accesses and calls with emphasis on accesses to ClassType
// fields.
//
// Encapsulates short-living state.
class DepCompute {
  const DTransAnalysisInfo &DTInfo;
  const DataLayout &DL;
  const TargetLibraryInfo &TLI;
  const Function *Method;
  const StructType *ClassType;

  // Output of computeDepApproximation.
  DepMap &DM;

  const Dep *computeValueDep(const Value *Val) {
    auto DIt = DM.ValDependencies.find(Val);
    if (DIt != DM.ValDependencies.end())
      return DIt->second;

    if (isa<Constant>(Val))
      return Dep::mkConst(DM);

    if (!isa<Instruction>(Val))
      return Dep::mkBottom(DM);

    if (!dep_iterator::isSupportedOpcode(cast<Instruction>(Val)->getOpcode()))
      return Dep::mkBottom(DM);

    auto ClassTy = ClassType;
    auto IsFieldAccessGEP = [ClassTy](const Value *V) -> unsigned {
      if (auto *GEP = dyn_cast<GetElementPtrInst>(V))
        if (GEP->getPointerOperand()->getType()->getPointerElementType() ==
            ClassTy)
          if (GEP->hasAllConstantIndices() && GEP->getNumIndices() == 2 &&
              cast<Constant>(*GEP->idx_begin())->isZeroValue())
            return cast<Constant>(*(GEP->idx_begin() + 1))
                ->getUniqueInteger()
                .getLimitedValue();
      return -1U;
    };

    const Dep *ValRep = nullptr;
    // Find closure for dependencies. SCC returned in post order.
    for (auto SCCIt = scc_begin(DCFGraph<const Value *>(Val)); !SCCIt.isAtEnd();
         ++SCCIt) {

      // Get first instruction in SCC.
      auto DIt = DM.ValDependencies.find(*SCCIt->begin());
      if (DIt != DM.ValDependencies.end()) {
        ValRep = DIt->second;
        assert(all_of(*SCCIt,
                      [this, ValRep](const Value *V) -> bool {
                        return DM.ValDependencies.find(V)->second == ValRep;
                      }) &&
               "Incorrect SCC traversal");
        continue;
      } else if (!dep_iterator::isSupportedOpcode(
                     cast<Instruction>(*SCCIt->begin())->getOpcode()))
        return Dep::mkBottom(DM);

      Dep::Container Args;
      for (auto *Inst : const_scc_dep_iterator::deps(*SCCIt)) {
        auto DIt = DM.ValDependencies.find(Inst);
        if (DIt == DM.ValDependencies.end()) {
          Args.clear();
          Args.insert(Dep::mkBottom(DM));
          break;
        }
        Args.insert(DIt->second);
      }

      const Dep *ThisRep = nullptr;
      if ((*SCCIt).size() > 1) {
        if (Args.size() == 0)
          Args.insert(Dep::mkConst(DM));
        ThisRep = Dep::mkFunction(DM, Args);
      } else {
        unsigned FieldInd = IsFieldAccessGEP(*(*SCCIt).begin());

        if (isSafeBitCast(DL, *(*SCCIt).begin()))
          ThisRep = Dep::mkArgList(DM, Args);
        else if (FieldInd != -1U && Args.size() != 0)
          ThisRep = Dep::mkGEP(DM, Dep::mkNonEmptyArgList(DM, Args), FieldInd);
        else if (Args.size() == 0)
          ThisRep = Dep::mkConst(DM);
        else
          ThisRep = Dep::mkFunction(DM, Args);
      }

      for (auto *V : *SCCIt)
        DM.ValDependencies[V] = ThisRep;

      // Last SCC corresponds to Val.
      ValRep = ThisRep;
    }
    assert(ValRep && "Invalid logic of computeValueDep");
    return ValRep;
  }

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

  void computeDepApproximation(
      std::function<bool(const Function *)> IsKnownCallCheck) {

    assert(IsKnownCallCheck(Method) &&
           "Unexpected predicate passed to computeDepApproximation");

    SmallVector<const BasicBlock *, 32> Returns;
    SmallPtrSet<const BasicBlock *, 32> Visited;

    for (auto &Arg : Method->args())
      DM.ValDependencies[&Arg] = Dep::mkArg(DM, &Arg);

    for (auto &BB : *Method)
      if (BB.getTerminator()->getNumSuccessors() == 0)
        Returns.push_back(&BB);

    for (auto *R : Returns)
      for (auto SCCIt = scc_begin(Inverse<const BasicBlock *>(R));
           !SCCIt.isAtEnd(); ++SCCIt)
        for (auto *BB : *SCCIt) {
          if (Visited.find(BB) != Visited.end())
            continue;
          Visited.insert(BB);

          for (auto &I : *BB) {

            // Value dependencies involving pure arithmetic are computed on
            // demand.
            if (dep_iterator::isSupportedOpcode(I.getOpcode())) {
              continue;
            }

            const Dep *Rep = nullptr;
            switch (I.getOpcode()) {
            case Instruction::Load:
              if (cast<LoadInst>(I).isVolatile()) {
                Rep = Dep::mkBottom(DM);
                break;
              }
              Rep = Dep::mkLoad(
                  DM, computeValueDep(cast<LoadInst>(I).getPointerOperand()));
              break;
            case Instruction::Store:
              if (cast<StoreInst>(I).isVolatile()) {
                Rep = Dep::mkBottom(DM);
                break;
              }
              Rep = Dep::mkStore(
                  DM, computeValueDep(cast<StoreInst>(I).getValueOperand()),
                  computeValueDep(cast<StoreInst>(I).getPointerOperand()));
              break;
            case Instruction::Unreachable:
              Rep = Dep::mkConst(DM);
              break;
            case Instruction::Ret:
              if (I.getNumOperands() == 0)
                Rep = Dep::mkConst(DM);
              else
                // Did not introduce Ret special kind.
                Rep = computeValueDep(I.getOperand(0));
              break;
            case Instruction::Resume:
              Rep = computeValueDep(I.getOperand(0));
              break;
            case Instruction::LandingPad:
              if (!cast<LandingPadInst>(I).isCleanup() ||
                  cast<LandingPadInst>(I).getNumClauses() != 0) {
                Rep = Dep::mkBottom(DM);
                break;
              }
              // Explicitly embedded CFG.
              Rep = computeValueDep(
                  I.getParent()->getSinglePredecessor()->getTerminator());
              break;
            case Instruction::Br: {
              if (cast<BranchInst>(I).isConditional())
                Rep = computeValueDep(cast<BranchInst>(I).getCondition());
              else
                Rep = Dep::mkConst(DM);
              break;
            }
            case Instruction::Invoke:
            case Instruction::Call: {
              auto *Info = DTInfo.getCallInfo(&I);
              if (auto *M = dyn_cast<MemSetInst>(&I)) {
                Dep::Container Special;
                Special.insert(computeValueDep(M->getDest()));
                Special.insert(computeValueDep(M->getLength()));
                Rep = Dep::mkStore(DM, computeValueDep(M->getValue()),
                                   Dep::mkNonEmptyArgList(DM, Special));
                break;
              }
              if (!Info) {
                ImmutableCallSite CS(&I);
                if (auto *F = dyn_cast<Function>(CS.getCalledValue())) {
                  Dep::Container Args;
                  for (auto &Op : I.operands()) {
                    if (isa<BasicBlock>(Op.get()))
                      continue;
                    Args.insert(computeValueDep(Op.get()));
                  }
                  Rep = Dep::mkCall(DM, Dep::mkArgList(DM, Args),
                                    IsKnownCallCheck(F));
                  break;
                }
                Rep = Dep::mkBottom(DM);
                break;
              }

              SmallPtrSet<const Value *, 3> Args;
              if (Info->getCallInfoKind() == dtrans::CallInfo::CIK_Alloc) {
                auto AK = cast<AllocCallInfo>(Info)->getAllocKind();
                collectSpecialAllocArgs(AK, ImmutableCallSite(&I), Args, TLI);
              } else if (Info->getCallInfoKind() ==
                         dtrans::CallInfo::CIK_Free) {
                auto FK = cast<FreeCallInfo>(Info)->getFreeKind();
                collectSpecialFreeArgs(FK, ImmutableCallSite(&I), Args, TLI);
              } else {
                Rep = Dep::mkBottom(DM);
                break;
              }

              Dep::Container Special;
              Dep::Container Remaining;
              for (auto &Op : I.operands())
                if (Args.count(Op.get()))
                  Special.insert(computeValueDep(Op.get()));
                else
                  Remaining.insert(computeValueDep(Op.get()));

              // Relying on check that Realloc is forbidden.
              Rep = Info->getCallInfoKind() == dtrans::CallInfo::CIK_Alloc
                        ? Dep::mkAlloc(DM, Dep::mkNonEmptyArgList(DM, Special),
                                       Dep::mkArgList(DM, Remaining))
                        : Dep::mkFree(DM, Dep::mkNonEmptyArgList(DM, Special),
                                      Dep::mkArgList(DM, Remaining));
              break;
            }
            default:
              Rep = Dep::mkBottom(DM);
              break;
            }

            assert(Rep && "Invalid switch in computeDepApproximation");
            DM.ValDependencies[&I] = Rep;
          }
        }
  }
};

} // namespace

// DenseMapInfo<Dep *> specialization.
namespace llvm {
Dep *DenseMapInfo<Dep *>::getEmptyKey() { return &Dep::Empty; }
Dep *DenseMapInfo<Dep *>::getTombstoneKey() { return &Dep::Tombstone; }
unsigned DenseMapInfo<Dep *>::getHashValue(CPtr Ptr) {
  return Ptr->getHashValue();
}
bool DenseMapInfo<Dep *>::isEqual(CPtr LHS, CPtr RHS) {
  return LHS->isEqual(*RHS);
}
} // namespace llvm

namespace {

// Various idioms relying on checks of Dep.

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Deterministic order for "const Dep *"
struct DepCmp {
  bool operator()(const Dep *A, const Dep *B) const {
    assert((A != B) == (A->Id != B->Id) &&
           "Dep's in DepManager should be comparable by Id");
    return A->Id < B->Id;
  }
};

void Dep::print(raw_ostream &OS, unsigned Indent, unsigned ClosingParen) const {
  auto EOL = [&OS](unsigned ClosingParen) -> void {
    for (unsigned I = 0; I < ClosingParen; ++I)
      OS << ")";
    OS << "\n";
  };
  switch (Kind) {
  case DK_Bottom:
    OS << "Unknown";
    EOL(ClosingParen);
    break;
  case DK_Argument:
    OS << "Arg " << Const;
    EOL(ClosingParen);
    break;
  case DK_Const:
    OS << "Const";
    EOL(ClosingParen);
    break;
  case DK_Store:
    OS << "Store(";
    Arg1->print(OS, Indent + 6, 1);
    OS << "; ";
    OS.indent(Indent + 5);
    OS << "(";
    Arg2->print(OS, Indent + 6, ClosingParen + 1);
    break;
  case DK_Load:
    OS << "Load(";
    Arg1->print(OS, Indent + 5, ClosingParen + 1);
    break;
  case DK_GEP:
    OS << "GEP(";
    Arg2->print(OS, Indent + 4, 1);
    OS << "; ";
    OS.indent(Indent + 4);
    OS << Const;
    EOL(ClosingParen);
    break;
  case DK_Alloc:
    OS << "Alloc size(";
    Arg1->print(OS, Indent + 11, 1);
    OS << "; ";
    OS.indent(Indent + 10);
    OS << "(";
    Arg2->print(OS, Indent + 11, ClosingParen + 1);
    break;
  case DK_Free:
    OS << "Free ptr(";
    Arg1->print(OS, Indent + 9, 1);
    OS << "; ";
    OS.indent(Indent + 8);
    OS << "(";
    Arg2->print(OS, Indent + 9, ClosingParen + 1);
    break;
  case DK_Call:
    if (Const) {
      OS << "Unknown call (";
      Arg2->print(OS, Indent + 14, 1);
    } else {
      OS << "Known call (";
      Arg2->print(OS, Indent + 12, 1);
    }
    break;
  case DK_Function: {
    OS << "Func(";

    std::set<const Dep *, DepCmp> Print;
    Print.insert(Args->begin(), Args->end());

    for (auto I = Print.begin(), E = Print.end(); I != E; ++I) {
      if (I != Print.begin()) {
        OS << "; ";
        OS.indent(Indent + 4);
        OS << "(";
      }
      auto T = I;
      ++T;
      if (T == Print.end())
        (*I)->print(OS, Indent + 5, ClosingParen + 1);
      else
        (*I)->print(OS, Indent + 5, 1);
    }
    break;
  }
  case DK_Empty:
  case DK_Tomb: {
    OS << "\n ERROR: Special Kind encountered\n";
    break;
  }
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace
