//===---------------- SOAToAOSEffects.cpp - Part of SOAToAOSPass ----------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements DepManager/DepCompute' methods for approximate IR
// computations.
// Debug functionality:
//  - debug analysis pass for approximate IR;
//  - iterators explicitly instantiated to catch compilation issue earlier.
//
//===----------------------------------------------------------------------===//

#include "SOAToAOSEffects.h"

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Transforms/SOAToAOS.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"

namespace llvm {
namespace dtrans {
namespace soatoaos {
cl::opt<bool>
    DTransSOAToAOSComputeAllDep("enable-dtrans-soatoaos-alldeps",
                                cl::init(false), cl::Hidden,
                                cl::desc("Enable DTrans SOAToAOS"));

// There are only 6 instances of the corresponding template.
template class value_op_iterator<User::op_iterator, Value,
                                 ArithInstructionsTrait>;
template class value_op_iterator<User::const_op_iterator, const Value,
                                 ArithInstructionsTrait>;
template class value_op_iterator<User::op_iterator, Value,
                                 AllInstructionsTrait>;
template class value_op_iterator<User::const_op_iterator, const Value,
                                 AllInstructionsTrait>;
template class value_op_iterator<User::op_iterator, Value,
                                 GEPInstructionsTrait>;
template class value_op_iterator<User::const_op_iterator, const Value,
                                 GEPInstructionsTrait>;

// There are only 6 instances of the corresponding template.
template class ptr_iter<
    value_op_iterator<User::op_iterator, Value, ArithInstructionsTrait>>;
template class ptr_iter<value_op_iterator<User::const_op_iterator, const Value,
                                          ArithInstructionsTrait>>;
template class ptr_iter<
    value_op_iterator<User::op_iterator, Value, AllInstructionsTrait>>;
template class ptr_iter<value_op_iterator<User::const_op_iterator, const Value,
                                          AllInstructionsTrait>>;
template class ptr_iter<
    value_op_iterator<User::op_iterator, Value, GEPInstructionsTrait>>;
template class ptr_iter<value_op_iterator<User::const_op_iterator, const Value,
                                          GEPInstructionsTrait>>;

// Empty.Id == 0;
Dep Dep::Empty(DK_Empty);
// Tombstone.Id == 0;
Dep Dep::Tombstone(DK_Tomb);

const Dep *DepManager::intern(Dep &&Tmp) {
  assert(DepId == Deps.size() && "Inconsistent state of DepManager::Deps");
  ++Queries;
  Tmp.Id = ++DepId;
  auto It = Deps.find(&Tmp);
  if (It != Deps.end()) {
    --DepId;
    return *It;
  }
  return *Deps.insert(new Dep(std::move(Tmp))).first;
}

DepManager::~DepManager() {
  DEBUG_WITH_TYPE(DTRANS_SOADEP, dbgs() << "; Deps computed: " << DepId
                                        << ", Queries: " << Queries << "\n");
  DepId -= Deps.size();
  assert(DepId == 0 && "Inconsistent state of DepManager::Deps");

  // Deps checks Dep's internal while removing elements.
  std::vector<const Dep *> Temp;
  Temp.insert(Temp.end(), Deps.begin(), Deps.end());
  Deps.clear();
  for (auto *Ptr : Temp)
    delete Ptr;
}

const Dep *DepCompute::computeValueDep(const Value *Val) {
  auto DIt = DM.ValDependencies.find(Val);
  if (DIt != DM.ValDependencies.end())
    return DIt->second;

  if (isa<Constant>(Val))
    return Dep::mkConst(DM);

  if (!isa<Instruction>(Val))
    return Dep::mkBottom(DM);

  if (!arith_inst_dep_iterator::isSupportedOpcode(
          cast<Instruction>(Val)->getOpcode()))
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
  for (auto SCCIt = scc_begin(ArithDepGraph<const Value *>(Val));
       !SCCIt.isAtEnd(); ++SCCIt) {

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
    } else if (!arith_inst_dep_iterator::isSupportedOpcode(
                   cast<Instruction>(*SCCIt->begin())->getOpcode()))
      return Dep::mkBottom(DM);

    Dep::Container Args;
    for (auto *Inst : const_scc_arith_inst_dep_iterator::deps(*SCCIt)) {
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
      // Conservative analysis here, but sufficient for ultimate purpose.
      if (Args.size() == 0)
        Args.insert(Dep::mkConst(DM));
      ThisRep = Dep::mkFunction(DM, Args);
    } else {
      unsigned FieldInd = IsFieldAccessGEP(*(*SCCIt).begin());

      auto *V = *(*SCCIt).begin();
      if (isSafeBitCast(DL, V) || isSafeIntToPtr(DL, V))
        ThisRep = Dep::mkArgList(DM, Args);
      else if (isBitCastLikeGep(DL, V))
        ThisRep = Dep::mkGEP(DM, Dep::mkNonEmptyArgList(DM, Args), 0);
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

void DepCompute::computeDepApproximation(
    std::function<bool(const Function *)> IsKnownCallCheck) {

  assert(IsKnownCallCheck(Method) &&
         "Unexpected predicate passed to computeDepApproximation");

  SmallVector<const Instruction *, 32> Sinks;
  // This set is needed, because there could be multiple traversals starting
  // from different Sinks.
  SmallPtrSet<const Value *, 32> Visited;

  for (auto &Arg : Method->args())
    DM.ValDependencies[&Arg] = Dep::mkArg(DM, &Arg);

  for (auto &I : instructions(*Method))
    if (I.hasNUses(0))
      Sinks.push_back(&I);

  for (auto *S : Sinks)
    // SCCs are returned in post-order, for example, first instruction is
    // processed first. Graph of SCC is a tree with multiple edges between 2
    // SCC nodes.
    //
    // It is expected that there is no cyclic dependencies involving
    // instructions processed in the switch below, for example,
    // pointer-chasing is prohibited:
    //  t = A[0]
    //  for ()
    //    t = A[t]
    //
    // Restriction is related to use of scc_arith_inst_dep_iterator in
    // computeValueDep: operands inside SCC are ignored.
    for (auto SCCIt = scc_begin(AllDepGraph<const Value *>(S));
         !SCCIt.isAtEnd(); ++SCCIt) {

      // SCC should is processed in all-or-nothing way.
      if (Visited.find(*SCCIt->begin()) != Visited.end())
        continue;

      for (auto *I : *SCCIt) {
        assert(Visited.find(I) == Visited.end() && "Traversal logic is broken");
        Visited.insert(I);
      }

      for (auto *PtrI : *SCCIt) {
        if (isa<Argument>(PtrI))
          continue;
        auto &I = cast<Instruction>(*PtrI);
        // Value dependencies involving pure arithmetic are computed on
        // demand.
        if (arith_inst_dep_iterator::isSupportedOpcode(I.getOpcode())) {
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
        case Instruction::LandingPad: {
          auto &LP = cast<LandingPadInst>(I);
          bool Supported = true;
          for (unsigned I = 0, E = LP.getNumClauses(); I != E; ++I)
            if (LP.isFilter(I)) {
              Supported = false;
              break;
            }

          if (!Supported) {
            Rep = Dep::mkBottom(DM);
            break;
          }
          // Explicitly embedded CFG.
          Dep::Container Preds;
          for (auto *BB : predecessors(I.getParent()))
            Preds.insert(computeValueDep(BB->getTerminator()));

          Rep = Dep::mkNonEmptyArgList(DM, Preds);
          break;
        }
        case Instruction::Br: {
          if (cast<BranchInst>(I).isConditional())
            Rep = computeValueDep(cast<BranchInst>(I).getCondition());
          else
            Rep = Dep::mkConst(DM);
          break;
        }
        case Instruction::Invoke:
        case Instruction::Call: {
          if (auto *M = dyn_cast<MemSetInst>(&I)) {
            Dep::Container Special;
            Special.insert(computeValueDep(M->getRawDest()));
            Special.insert(computeValueDep(M->getLength()));
            Rep = Dep::mkStore(DM, computeValueDep(M->getValue()),
                               Dep::mkNonEmptyArgList(DM, Special));
            break;
          }

          SmallPtrSet<const Value *, 3> Args;
          auto *Info = DTInfo.getCallInfo(&I);
          if (Info) {
            if (Info->getCallInfoKind() == dtrans::CallInfo::CIK_Alloc) {
              auto AK = cast<AllocCallInfo>(Info)->getAllocKind();
              collectSpecialAllocArgs(AK, ImmutableCallSite(&I), Args, TLI);
            } else if (Info->getCallInfoKind() == dtrans::CallInfo::CIK_Free) {
              auto FK = cast<FreeCallInfo>(Info)->getFreeKind();
              collectSpecialFreeArgs(FK, ImmutableCallSite(&I), Args, TLI);
            } else {
              Rep = Dep::mkBottom(DM);
              break;
            }
          }

          Dep::Container Special;
          Dep::Container Remaining;
          for (auto &Op : I.operands()) {
            // CFG is processed separately.
            if (isa<BasicBlock>(Op.get()))
              continue;
            if (Args.count(Op.get()))
              Special.insert(computeValueDep(Op.get()));
            else
              Remaining.insert(computeValueDep(Op.get()));
          }

          auto *F = dyn_cast<Function>(ImmutableCallSite(&I).getCalledValue());

          if (Info)
            // Relying on check that Realloc is forbidden.
            Rep = Info->getCallInfoKind() == dtrans::CallInfo::CIK_Alloc
                      ? Dep::mkAlloc(DM, Dep::mkNonEmptyArgList(DM, Special),
                                     Dep::mkArgList(DM, Remaining))
                      : Dep::mkFree(DM, Dep::mkNonEmptyArgList(DM, Special),
                                    Dep::mkArgList(DM, Remaining));
          else
            Rep = Dep::mkCall(DM, Dep::mkArgList(DM, Remaining),
                              F && IsKnownCallCheck(F));
          break;
        }
        default:
          Rep = Dep::mkBottom(DM);
          break;
        }

        assert(Rep && "Invalid switch in computeDepApproximation");
        if (Rep->isBottom() && DTransSOAToAOSComputeAllDep)
          return;
        DM.ValDependencies[&I] = Rep;
      }
    }
}

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
    OS << "Unknown Dep";
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
      Arg2->print(OS, Indent + 14, ClosingParen + 1);
    } else {
      OS << "Known call (";
      Arg2->print(OS, Indent + 12, ClosingParen + 1);
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

// Structure name to use in SOAToAOSApproximationDebug.
cl::opt<std::string> DTransSOAToAOSApproxTypename(
    "dtrans-soatoaos-approx-typename", cl::init(""), cl::ReallyHidden,
    cl::desc("Structure name for SOAToAOSApproximationDebug"));

// Calls to mark as known in SOAToAOSApproximationDebug.
cl::list<std::string>
    DTransSOAToAOSApproxKnown("dtrans-soatoaos-approx-known-func",
                              cl::ReallyHidden);

// Get StructType from command-line or from 'this' argument.
StructType *getStructTypeOfArray(Function &F) {
  auto *ClassType = F.getParent()->getTypeByName(DTransSOAToAOSApproxTypename);
  if (ClassType)
    return ClassType;
  if (F.arg_size() == 0)
    return nullptr;

  auto *ThisType = dyn_cast<PointerType>(F.arg_begin()->getType());
  if (!ThisType)
    return nullptr;
  return dyn_cast<StructType>(ThisType->getPointerElementType());
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace soatoaos

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
using namespace soatoaos;

char SOAToAOSApproximationDebug::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey SOAToAOSApproximationDebug::Key;

SOAToAOSApproximationDebug::Ignore::Ignore(
    SOAToAOSApproximationDebugResult *Ptr)
    : Ptr(Ptr) {}
SOAToAOSApproximationDebug::Ignore::Ignore(Ignore &&Other)
    : Ptr(std::move(Other.Ptr)) {}
const SOAToAOSApproximationDebugResult *
SOAToAOSApproximationDebug::Ignore::get() const {
  return Ptr.get();
}
SOAToAOSApproximationDebug::Ignore::~Ignore() {}

SOAToAOSApproximationDebug::Ignore
SOAToAOSApproximationDebug::run(Function &F, FunctionAnalysisManager &AM) {
  const ModuleAnalysisManager &MAM =
      AM.getResult<ModuleAnalysisManagerFunctionProxy>(F).getManager();
  auto *DTInfo = MAM.getCachedResult<DTransAnalysis>(*F.getParent());
  auto *TLI = MAM.getCachedResult<TargetLibraryAnalysis>(*F.getParent());

  // TODO: add diagnostic message.
  if (!DTInfo || !TLI)
    return Ignore(nullptr);

  std::unique_ptr<SOAToAOSApproximationDebugResult> Result(
      new SOAToAOSApproximationDebugResult());
  StructType *ClassType = getStructTypeOfArray(F);

  // TODO: add diagnostic message.
  if (!ClassType)
    return Ignore(nullptr);

  std::function<bool(const Function *)> IsKnown =
      [&F](const Function *Func) -> bool {
    return Func == &F || find(DTransSOAToAOSApproxKnown, Func->getName()) !=
                             DTransSOAToAOSApproxKnown.end();
  };

  // Do analysis.
  DepCompute DC(*DTInfo, F.getParent()->getDataLayout(), *TLI, &F, ClassType,
                // *Result is filled-in.
                *Result);
  DC.computeDepApproximation(IsKnown);

  // Dump results of analysis.
  DEBUG_WITH_TYPE(DTRANS_SOADEP, {
    dbgs() << "; Dump computed dependencies ";
    DepMap::DepAnnotatedWriter Annotate(*Result);
    F.print(dbgs(), &Annotate);
  });
  return Ignore(Result.release());
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // namespace dtrans
} // namespace llvm
