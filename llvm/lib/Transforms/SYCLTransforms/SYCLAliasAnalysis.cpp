//===- SYCLAliasAnalysis.cpp - addrspace based alias analysis ------------===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/SYCLAliasAnalysis.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

using namespace llvm;
using namespace CompilationUtils;

#define DEBUG_TYPE "sycl-kernel-aa"

AnalysisKey SYCLAliasAnalysis::Key;

SYCLAAResult SYCLAliasAnalysis::run(Function &, FunctionAnalysisManager &) {
  return SYCLAAResult{};
}

SYCLAAResult::AACallbackVH::AACallbackVH(Value *V, SYCLAAResult *AAR)
    : CallbackVH(V), AAR(AAR) {}

bool SYCLAAResult::invalidate(Function &, const PreservedAnalyses &PA,
                               FunctionAnalysisManager::Invalidator &) {
  // Check whether the analysis has been explicitly invalidated. Otherwise, it's
  // stateless and remains preserved.
  auto PAC = PA.getChecker<SYCLAliasAnalysis>();
  return !PAC.preservedWhenStateless();
}

void SYCLAAResult::AACallbackVH::deleted() {
  assert(AAR && "AACallbackVH called with a null OCLAliasAnalysis!");
  AAR->deleteValue(getValPtr());
}

void SYCLAAResult::AACallbackVH::allUsesReplacedWith(Value *V) {
  assert(AAR && "AACallbackVH called with a null OCLAliasAnalysis!");
  // It is not clear if it's safe to call OCLAliasAnalysis::copyValue here
  // as other passes including standart ones might call it.
  AAR->rauwValue(getValPtr(), V);
}

static int getAddressSpaceMask(int ID) { return 1 << ID; }

SYCLAAResult::SYCLAAResult() {
  DisjointASs = getAddressSpaceMask(ADDRESS_SPACE_PRIVATE) |
                getAddressSpaceMask(ADDRESS_SPACE_GLOBAL) |
                getAddressSpaceMask(ADDRESS_SPACE_CONSTANT) |
                getAddressSpaceMask(ADDRESS_SPACE_LOCAL);
}

void SYCLAAResult::deleteValue(Value *V) { ValueMap.erase(V); }
void SYCLAAResult::copyValue(Value * /*From*/, Value * /*To*/) {
  // Do nothing, i.e. let new value compute in resolveAddressSpace.
}
void SYCLAAResult::addEscapingUse(Use &U) {
  resolveAddressSpace(U.get(), true);
}

void SYCLAAResult::rauwValue(Value *OldV, Value *NewV) {
  auto It = ValueMap.find_as(OldV);
  if (It != ValueMap.end())
    ValueMap.insert(std::make_pair(
        AACallbackVH(const_cast<Value *>(NewV), this), It->second));
}

SYCLAAResult::ResolveResult SYCLAAResult::cacheResult(SmallValueSet &Values,
                                                        ResolveResult RR) {
  for (auto *V : Values)
    ValueMap.insert(
        std::make_pair(AACallbackVH(const_cast<Value *>(V), this), RR));

  return RR;
}

// We may be given a value which by itself does not explicitly specify an
// SYCL/OpenCL address space although the address does belong to one (e.g.
// alloca instructions are used for both __local and __private memory, but do
// not explicitly specify the addrspace. Since we cannot distinguish OpenCL's
// __private address space from LLVM's default (i.e. no) address space, we do
// not trust the value's apparent __private address-space and attempt to find
// the explicit address space among the value's users and used base address
// value (e.g. the pointer operand of a GEP). If at least one pointer is in
// __generic address space SYCLAA can not say for sure that another pointer
// is in disjoint address space.
//
// Since SYCL/OpenCL's address spaces are disjoint, a single relation of an
// address to a non-default address space is enough to determine the address
// space. We therefore declare the address space to be __private only if:
// - we cannot find any explicit relation of it to another address space, and
// - the pointer is not escaping our analysis via casting to/from int
// (note that when we do find a non-default address space we don't care if the
// pointer is casted to/from int as no pointer arithmetic can escape the
// SYCL/OpenCL address space).
SYCLAAResult::ResolveResult SYCLAAResult::resolveAddressSpace(const Value *V,
                                                                bool Force) {
  SmallVector<const Value *, 16> NextPointerValues;
  SmallValueSet Visited;

  // Assume this is the default address space.
  Type *Ty = V->getType();
  if (VectorType *VecType = dyn_cast<VectorType>(Ty))
    Ty = VecType->getElementType();
  PointerType *PTy = cast<PointerType>(Ty);
  unsigned AS = PTy->getAddressSpace();
  // Keep track of whether this pointer originates from or gets casted to an
  // int.
  bool IsEscaping = false;

  NextPointerValues.push_back(V);
  do {
    const Value *PointerValue = NextPointerValues.pop_back_val();
    if (!Visited.insert(PointerValue).second) // no insertion happened.
      continue;

    if (!Force) {
      auto It = ValueMap.find_as(PointerValue);
      if (It != ValueMap.end())
        return cacheResult(Visited, It->second);
    }

    Ty = PointerValue->getType();
    if (VectorType *VecType = dyn_cast<VectorType>(Ty))
      Ty = VecType->getElementType();
    PTy = cast<PointerType>(Ty);
    unsigned ValueAS = PTy->getAddressSpace();

    if (!IsEscaping && AS != ValueAS && ValueAS != ADDRESS_SPACE_PRIVATE &&
        ValueAS != ADDRESS_SPACE_GENERIC) {
      // Check if the same pointer value was not resovled into different named
      // address spaces. E.g. __local and __global.
      if (AS != ADDRESS_SPACE_PRIVATE && AS != ADDRESS_SPACE_GENERIC)
        IsEscaping = true;
      AS = ValueAS;
    }

    // Check whether the pointer value stems from or gets casted to an int.
    if (!IsEscaping && (isa<IntToPtrInst>(PointerValue)))
      IsEscaping = true;

    // Mark this value's users to be visited.
    for (const User *U : PointerValue->users())
      if (isa<BitCastInst>(U) || isa<GetElementPtrInst>(U) ||
          isa<AddrSpaceCastInst>(U))
        NextPointerValues.push_back(U);

    // Also mark the base address this value is using to be visited, in case the
    // address space was for some reason stripped.
    const Value *Op = nullptr;
    if (const BitCastInst *BC = dyn_cast<BitCastInst>(PointerValue))
      Op = BC->getOperand(0);
    else if (const GetElementPtrInst *GEP =
                 dyn_cast<GetElementPtrInst>(PointerValue))
      Op = GEP->getPointerOperand();
    else if (const AddrSpaceCastInst *ASC =
                 dyn_cast<AddrSpaceCastInst>(PointerValue))
      Op = ASC->getOperand(0);

    if (Op)
      NextPointerValues.push_back(Op);
  } while (!NextPointerValues.empty());

  return cacheResult(Visited, ResolveResult(!IsEscaping, AS));
}

// Check aliasing using the fact that in openCL pointers from different
// memory addresses do not alias.
AliasResult SYCLAAResult::alias(const MemoryLocation &LocA,
                                 const MemoryLocation &LocB, AAQueryInfo &AAQI,
                                 const Instruction *) {
  const Value *A = LocA.Ptr;
  const Value *B = LocB.Ptr;

  PointerType *AP = dyn_cast<PointerType>(A->getType());
  PointerType *BP = dyn_cast<PointerType>(B->getType());
  if (AP && BP) {
    // If A and B are pointers to different address spaces then they do not
    // alias. This is not true in general, however, in openCL the memory
    // addresses for private, local, and global are disjoint.

    ResolveResult RR1 = resolveAddressSpace(A, false);
    ResolveResult RR2 = resolveAddressSpace(B, false);
    if (RR1.isResolved() && RR1.isResolved()) {
      unsigned APAddressSpace = RR1.getAddressSpace();
      unsigned BPAddressSpace = RR2.getAddressSpace();
      auto IsInSpace = [](int ID, int Mask) {
        return ((getAddressSpaceMask(ID) & Mask) != 0);
      };
      if (IsInSpace(APAddressSpace, DisjointASs) &&
          IsInSpace(BPAddressSpace, DisjointASs) &&
          APAddressSpace != BPAddressSpace) {
        LLVM_DEBUG(dbgs() << "SYCLAAResult NoAlias: " << *A << ", " << *B
                          << "\n");
        return AliasResult::NoAlias;
      }
    }
  }

  return AAResultBase::alias(LocA, LocB, AAQI, nullptr);
}

ModRefInfo SYCLAAResult::getModRefInfoMask(const MemoryLocation &Loc,
                                           AAQueryInfo &AAQI, bool IgnoreLocals) {
  const Value *V = Loc.Ptr;

  // Remove casts if exists
  V = V->stripPointerCasts();

  if (PointerType *VP = dyn_cast<PointerType>(V->getType())) {
    // If V is a pointer to the OCLAddressSpace::Constant address space
    // then it points to __constant OpenCL memory.
    if (VP->getAddressSpace() == ADDRESS_SPACE_CONSTANT)
      return ModRefInfo::NoModRef;
  }

  return AAResultBase::getModRefInfoMask(Loc, AAQI, IgnoreLocals);
}
