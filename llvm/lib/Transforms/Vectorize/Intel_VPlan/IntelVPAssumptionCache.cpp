//===- IntelVPAssumptionCache.cpp -------------------------------*- C++ -*-===//
//
//   Copyright (C) 2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
#include "IntelVPAssumptionCache.h"
#include "IntelVPlan.h"
#include "IntelVPlanClone.h"
#include "llvm/Analysis/AssumeBundleQueries.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Intrinsics.h"

using namespace llvm;
using namespace vpo;

#define DEBUG_TYPE "IntelVPAssumptionCache"

static bool isAssumeCall(const VPCallInstruction &Call) {
  if (auto *Func = Call.getCalledFunction())
    return Func->getIntrinsicID() == Intrinsic::assume;
  return false;
}
static std::string idxToString(unsigned Idx) {
  if (Idx == VPAssumptionCache::ExprResultIdx)
    return std::string("<expr>");
  return std::to_string(Idx);
}

std::unique_ptr<VPAssumptionCache>
VPAssumptionCache::clone(const VPValueMapper &Mapper) const {
  auto *Clone = new VPAssumptionCache(*getLLVMCache(), DT);

  for (auto &It : AffectedValues)
    for (auto &Assumption : It.second) {
      if (auto *RemappedVal = Mapper.getRemappedValue(It.first))
        Clone->insertAssume(RemappedVal, Assumption.Assume, Assumption.Index);
    }

  return std::unique_ptr<VPAssumptionCache>(Clone);
}

void VPAssumptionCache::insertAssume(const VPValue *V, AssumeT Assume,
                                     unsigned Index) {
  auto GetOrInsertValues = [this](const VPValue *V) -> auto & {
    auto It = AffectedValues.find(V);
    if (It != AffectedValues.end())
      return It->second;

    It = AffectedValues.insert({V, SmallVector<ResultElem, 1>{}}).first;
    return It->second;
  };
  assert(V && "can't insert assume for null value!");

  // Check that we don't insert duplicates. It's sufficient to just do a linear
  // scan here: the number of elements should rarely exceed a small number (<5).
  if (llvm::any_of(Assumes, [=](const VPAssumptionCache::ResultElem &E) {
        return E.Assume == Assume && E.Index == Index;
      }))
    return;

  LLVM_DEBUG(dbgs() << "Inserting assumption cache elem for '";
             V->printAsOperand(dbgs()); dbgs() << "':\n";
             dbgs() << "  Assume: " << Assume << '\n';
             dbgs() << "  Index:  " << idxToString(Index) << '\n');

  ResultElem Elem{Assume, Index};
  Assumes.push_back(Elem);
  GetOrInsertValues(V).push_back(Elem);
}

void VPAssumptionCache::registerAssumption(const VPCallInstruction &Assume) {
  assert(isAssumeCall(Assume) && "Can only register calls to '@llvm.assume'!");

  LLVM_DEBUG(dbgs() << "Registering assumption: " << Assume << '\n');

  const auto AddValue = [this, &Assume](const VPValue *V, unsigned Idx) {
    if (isa<VPConstant>(V)) {
      // Don't add affected constant values (e.g. the `i1 true` dummy arg which
      // is used for unconditional assumes.)
      return;
    }
    insertAssume(V, &Assume, Idx);
  };

  SmallVector<VPOperandBundle, 1> Bundles;
  Assume.getOperandBundles(Bundles);
  for (const auto &[Idx, Bundle] : enumerate(Bundles))
    if (Bundle.Inputs.size() > ABA_WasOn && Bundle.Tag != IgnoreBundleTag)
      AddValue(Bundle.Inputs[ABA_WasOn], Idx);

  AddValue(Assume.getArgOperand(0), ExprResultIdx);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
raw_ostream &llvm::vpo::operator<<(llvm::raw_ostream &OS,
                                   const VPAssumptionCache::AssumeT &Assume) {
  if (auto *VPCall = Assume.dyn_cast<const VPCallInstruction *>())
    return OS << *VPCall;
  return OS << *Assume.get<const AssumeInst *>();
}
void VPAssumptionCache::dump() const {
  for (auto &It : AffectedValues) {
    dbgs() << *It.first << ": \n";
    for (const ResultElem &Elem : It.second) {
      dbgs() << "  " << Elem.Assume << '\n';
    }
  }
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
