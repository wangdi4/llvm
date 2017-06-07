/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"

#include "OCLAliasAnalysis.h"
#include "OCLPassSupport.h"
#include "OCLAddressSpace.h"

#include <queue>

using namespace Intel::OpenCL::DeviceBackend::Utils;
using namespace llvm;

namespace intel {

OCL_INITIALIZE_PASS_BEGIN(OCLAliasAnalysis, "ocl-asaa",
                   "OpenCL Address Space Alias Analysis",
                   false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BasicAAWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(CFLAndersAAWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(ExternalAAWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(GlobalsAAWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(SCEVAAWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(ScopedNoAliasAAWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(TypeBasedAAWrapperPass)
OCL_INITIALIZE_PASS_END(OCLAliasAnalysis, "ocl-asaa",
                   "OpenCL Address Space Alias Analysis",
                   false, true)

//===----------------------------------------------------------------------===//
//                     OCLAAACallbackVH Class Implementation
//===----------------------------------------------------------------------===//
OCLAAResults::OCLAAACallbackVH::OCLAAACallbackVH(Value *V, OCLAAResults *ocl_aar)
  : CallbackVH(V), OCLAAR(ocl_aar) {}

void OCLAAResults::OCLAAACallbackVH::deleted() {
  assert(OCLAAR && "OCLAAACallbackVH called with a null OCLAliasAnalysis!");
  OCLAAR->deleteValue(getValPtr());
}

void OCLAAResults::OCLAAACallbackVH::allUsesReplacedWith(Value *V) {
  assert(OCLAAR && "OCLAAACallbackVH called with a null OCLAliasAnalysis!");
  // It is not clear if it's safe to call OCLAliasAnalysis::copyValue here
  // as other passes including standart ones might call it.
  OCLAAR->rauwValue(getValPtr(), V);
}

//===----------------------------------------------------------------------===//
//                     OCLAliasAnalysis Class Implementation
//===----------------------------------------------------------------------===//

void OCLAliasAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<BasicAAWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();

  // We also need to mark all the alias analysis passes we will potentially
  // probe in runOnFunction as used here to ensure the legacy pass manager
  // preserves them. This hard coding of lists of alias analyses is specific to
  // the legacy pass manager.
  AU.addUsedIfAvailable<ScopedNoAliasAAWrapperPass>();
  AU.addUsedIfAvailable<TypeBasedAAWrapperPass>();
  AU.addUsedIfAvailable<GlobalsAAWrapperPass>();
  AU.addUsedIfAvailable<SCEVAAWrapperPass>();
  AU.addUsedIfAvailable<CFLAndersAAWrapperPass>();
}

/// [LLVM 3.8 Upgrade] Taken from LLVM and modified.
/// Perhaps I should use callback for an external AA instead.
/// Anyway, external AA seems to be the best way to go with new
/// pass manager.
///
/// Run the wrapper pass to rebuild an aggregation over known AA passes.
/// This is the legacy pass manager's interface to the new-style AA results
/// aggregation object. Because this is somewhat shoe-horned into the legacy
/// pass manager, we hard code all the specific alias analyses available into
/// it. While the particular set enabled is configured via commandline flags,
/// adding a new alias analysis to LLVM will require adding support for it to
/// this list.
bool OCLAliasAnalysis::runOnFunction(Function &F) {
  // NB! This *must* be reset before adding new AA results to the new
  // AAResults object because in the legacy pass manager, each instance
  // of these will refer to the *same* immutable analyses, registering and
  // unregistering themselves with them. We need to carefully tear down the
  // previous object first, in this case replacing it with an empty one, before
  // registering new results.
  auto &TLIWP = getAnalysis<TargetLibraryInfoWrapperPass>();
  OCLAAR.reset(new OCLAAResults(TLIWP.getTLI()));

  // BasicAA is always available for function analyses. Also, we add it first
  // so that it can trump TBAA results when it proves MustAlias.
  // FIXME: TBAA should have an explicit mode to support this and then we
  // should reconsider the ordering here.
  // if (!DisableBasicAA)
  OCLAAR->addAAResult(getAnalysis<BasicAAWrapperPass>().getResult());

  // Populate the results with the currently available AAs.
  if (auto *WrapperPass = getAnalysisIfAvailable<ScopedNoAliasAAWrapperPass>())
    OCLAAR->addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass = getAnalysisIfAvailable<TypeBasedAAWrapperPass>())
    OCLAAR->addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass = getAnalysisIfAvailable<GlobalsAAWrapperPass>())
    OCLAAR->addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass = getAnalysisIfAvailable<SCEVAAWrapperPass>())
    OCLAAR->addAAResult(WrapperPass->getResult());
  if (auto *WrapperPass = getAnalysisIfAvailable<CFLAndersAAWrapperPass>())
    OCLAAR->addAAResult(WrapperPass->getResult());

  // Analyses don't mutate the IR, so return false.
  return false;
}
//===----------------------------------------------------------------------===//
//                     OCLAAResults Class Implementation
//===----------------------------------------------------------------------===//

OCLAAResults::OCLAAResults(const TargetLibraryInfo &TLI) : AAResults(TLI) {
  m_disjointAddressSpaces = getAddressSpaceMask(OCLAddressSpace::Private) |
                            getAddressSpaceMask(OCLAddressSpace::Global) |
                            getAddressSpaceMask(OCLAddressSpace::Constant) |
                            getAddressSpaceMask(OCLAddressSpace::Local);
}

OCLAliasAnalysis::OCLAliasAnalysis() : FunctionPass(ID) {
  initializeOCLAliasAnalysisPass(*PassRegistry::getPassRegistry());
}

void OCLAAResults::deleteValue(Value *V) {
  ValueMap.erase(V);
}
void OCLAAResults::copyValue(Value *From, Value *To) {
  // Do nothing, i.e. let new value compute in resolveAddressSpace.
}
void OCLAAResults::addEscapingUse(Use &U) {
  resolveAddressSpace(U.get(), true);
}

void OCLAAResults::rauwValue(Value* oldVal, Value* newVal) {
  auto it = ValueMap.find_as(oldVal);
  if (it != ValueMap.end())
    ValueMap.insert(std::make_pair(OCLAAACallbackVH(const_cast<Value*>(newVal), this), it->second));
}

OCLAAResults::ResolveResult
OCLAAResults::cacheResult(SmallValueSet& values,
                              ResolveResult resolveResult) {
  for (SmallValueSet::iterator it = values.begin(); it != values.end(); ++it) {
    ValueMap.insert(std::make_pair(OCLAAACallbackVH(const_cast<Value*>(*it), this), resolveResult));
  }
  return resolveResult;
}

// We may be given a value which by itself does not explicitly specify an OpenCL address
// space although the address does belong to one (e.g. alloca instructions are used for
// both __local and __private memory, but do not explicitly specify the addrspace.
// Since we cannot distinguish OpenCL's __private address space from LLVM's default (i.e. no)
// address space, we do not trust the value's apparent __private address-space and attempt
// to find the explicit namespace among the value's users and used base address
// value (e.g. the pointer operand of a GEP). If at least one pointer is in __generic address space
// the OCL AAA can not say for sure that another pointer is in disjoint address space.
//
// Since OpenCL's address spaces are disjoint, a single relation of an address to a non-default
// address space is enough to determine the address space. We therefore declare the address
// space to be __private only if:
// - we cannot find any explicit relation of it to another address space, and
// - the pointer is not escaping our analysis via casting to/from int
// (note that when we do find a non-default address space we don't care if the pointer is casted
// to/from int as no pointer arithmetic can escape the OpenCL address space).
OCLAAResults::ResolveResult OCLAAResults::resolveAddressSpace(const Value* value, bool force) {
  std::queue<const Value*> nextPointerValues;
  SmallValueSet visitedPointerValues;

  // Assume this is the default namespace.
  Type* type = value->getType();
  PointerType *pointerType = dyn_cast<PointerType>(type);
  assert(pointerType && "Value is not of a pointer type");
  unsigned int addressSpace = pointerType->getAddressSpace();
  // Keep track of whether this pointer originates from or gets casted to an int.
  bool isEscaping = false;

  nextPointerValues.push(value);
  do {
    const Value* pointerValue = nextPointerValues.front();
    nextPointerValues.pop();

    if (!visitedPointerValues.insert(pointerValue).second) // no insertion happened.
      continue;

    if(!force) {
      ValueMapType::iterator it = ValueMap.find_as(pointerValue);
      if (it != ValueMap.end()) {
        return cacheResult(visitedPointerValues, it->second);
      }
    }

    type = pointerValue->getType();
    pointerType = dyn_cast<PointerType>(type);
    assert(pointerType && "Value is not of a pointer type");
    unsigned int valueAddressSpace = pointerType->getAddressSpace();

    if(!isEscaping && addressSpace != valueAddressSpace && valueAddressSpace != OCLAddressSpace::Private && valueAddressSpace != OCLAddressSpace::Generic) {
        // Check if the same pointer value was not resovled into different named address spaces. E.g. __local and __global.
        if(addressSpace != OCLAddressSpace::Private && addressSpace != OCLAddressSpace::Generic)
            isEscaping = true;
        addressSpace = valueAddressSpace;
    }

    // Check whether the pointer value stems from or gets casted to an int.
    if (!isEscaping && (isa<IntToPtrInst>(pointerValue) || isa<PtrToIntInst>(pointerValue)))
      isEscaping = true;

    // Mark this value's users to be visited.
    for (Value::const_user_iterator it = pointerValue->user_begin(), end = pointerValue->user_end(); it != end; ++it)

      if (isa<BitCastInst>(*it) || isa<GetElementPtrInst>(*it) || isa<AddrSpaceCastInst>(*it))
        nextPointerValues.push(*it);

    // Also mark the base address this value is using to be visited, in case the
    // address space was for some reason stripped.
    Value const* operand = nullptr;
    if (const BitCastInst* bitCastInst = dyn_cast<BitCastInst>(pointerValue)) {
      operand = bitCastInst->getOperand(0);
    } else if (const GetElementPtrInst* gepInst = dyn_cast<GetElementPtrInst>(pointerValue)) {
      operand = gepInst->getPointerOperand();
    } else if (const AddrSpaceCastInst* addrSpaceCastInst = dyn_cast<AddrSpaceCastInst>(pointerValue)) {
      operand = addrSpaceCastInst->getOperand(0);
    }

    if (operand)
      nextPointerValues.push(operand);

  } while (!nextPointerValues.empty());

  ResolveResult resolveResult = ResolveResult(!isEscaping, addressSpace);
  return cacheResult(visitedPointerValues, resolveResult);
}

// Check aliasing using the fact that in openCL pointers from different
// memory addresses do not alias.
AliasResult OCLAAResults::alias(const MemoryLocation &LocA,
                                                   const MemoryLocation &LocB) {
  const Value *V1 = LocA.Ptr;
  const Value *V2 = LocB.Ptr;

  PointerType *V1P = dyn_cast<PointerType>(V1->getType());
  PointerType *V2P = dyn_cast<PointerType>(V2->getType());
  if (V1P && V2P) {
    // If V1 and V2 are pointers to different address spaces then they do not alias.
    // This is not true in general, however, in openCL the memory addresses
    // for private, local, and global are disjoint.

    ResolveResult rrV1 = resolveAddressSpace(V1, false);
    ResolveResult rrV2 = resolveAddressSpace(V2, false);
    if (rrV1.isResolved() && rrV1.isResolved()) {
      unsigned int V1PAddressSpace = rrV1.getAddressSpace();
      unsigned int V2PAddressSpace = rrV2.getAddressSpace();
      if (isInSpace(V1PAddressSpace, m_disjointAddressSpaces) &&
          isInSpace(V2PAddressSpace, m_disjointAddressSpaces) &&
          V1PAddressSpace != V2PAddressSpace) {
        return NoAlias;
      }
    }
  }

  return alias(LocA, LocB);
}

bool OCLAAResults::pointsToConstantMemory(const MemoryLocation &Loc, bool OrLocal) {

  const Value *V = Loc.Ptr;

  // Remove casts if exists
  V = V->stripPointerCasts();

  if (PointerType *VP = dyn_cast<PointerType>(V->getType())) {
    // If V is a pointer to the OCLAddressSpace::Constant address space
    // then it points to __constant OpenCL memory.
    if (VP->getAddressSpace() == OCLAddressSpace::Constant)
      return true;
  }

  return pointsToConstantMemory(Loc, OrLocal);
}

char OCLAliasAnalysis::ID = 0;
}

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  FunctionPass* createOCLAliasAnalysisPass() {
    return new intel::OCLAliasAnalysis();
  }
}
