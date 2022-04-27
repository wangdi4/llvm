// INTEL CONFIDENTIAL
//
// Copyright 2012-2019 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/ScalarEvolutionAliasAnalysis.h"
#include "llvm/Analysis/CFLAndersAliasAnalysis.h"
#include "llvm/InitializePasses.h"

#include "OCLAliasAnalysis.h"
#include "OCLPassSupport.h"
#include "OCLAddressSpace.h"

#include <queue>

using namespace Intel::OpenCL::DeviceBackend::Utils;
using namespace llvm;

namespace intel {

OCL_INITIALIZE_PASS(OCLAliasAnalysis, "ocl-asaa",
                   "OpenCL Address Space Alias Analysis",
                   false, true)

//===----------------------------------------------------------------------===//
//                     OCLAAACallbackVH Class Implementation
//===----------------------------------------------------------------------===//
OCLAAResult::OCLAAACallbackVH::OCLAAACallbackVH(Value *V, OCLAAResult *ocl_aar)
  : CallbackVH(V), OCLAAR(ocl_aar) {}

void OCLAAResult::OCLAAACallbackVH::deleted() {
  assert(OCLAAR && "OCLAAACallbackVH called with a null OCLAliasAnalysis!");
  OCLAAR->deleteValue(getValPtr());
}

void OCLAAResult::OCLAAACallbackVH::allUsesReplacedWith(Value *V) {
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
bool OCLAliasAnalysis::doInitialization(Module &) {
  OCLAAR.reset(new OCLAAResult());
  return false;
}
//===----------------------------------------------------------------------===//
//                     OCLAAResult Class Implementation
//===----------------------------------------------------------------------===//

OCLAAResult::OCLAAResult() : AAResultBase() {
  m_disjointAddressSpaces = getAddressSpaceMask(OCLAddressSpace::Private) |
                            getAddressSpaceMask(OCLAddressSpace::Global) |
                            getAddressSpaceMask(OCLAddressSpace::Constant) |
                            getAddressSpaceMask(OCLAddressSpace::Local);
}

OCLAliasAnalysis::OCLAliasAnalysis() : ImmutablePass(ID) {
  initializeOCLAliasAnalysisPass(*PassRegistry::getPassRegistry());
}

void OCLAAResult::deleteValue(Value *V) {
  ValueMap.erase(V);
}
void OCLAAResult::copyValue(Value * /*From*/, Value * /*To*/) {
  // Do nothing, i.e. let new value compute in resolveAddressSpace.
}
void OCLAAResult::addEscapingUse(Use &U) {
  resolveAddressSpace(U.get(), true);
}

void OCLAAResult::rauwValue(Value* oldVal, Value* newVal) {
  auto it = ValueMap.find_as(oldVal);
  if (it != ValueMap.end())
    ValueMap.insert(std::make_pair(OCLAAACallbackVH(const_cast<Value*>(newVal), this), it->second));
}

OCLAAResult::ResolveResult
OCLAAResult::cacheResult(SmallValueSet& values,
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
// to find the explicit address space among the value's users and used base address
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
OCLAAResult::ResolveResult OCLAAResult::resolveAddressSpace(const Value* value, bool force) {
  std::queue<const Value*> nextPointerValues;
  SmallValueSet visitedPointerValues;

  // Assume this is the default address space.
  Type* type = value->getType();
  if (VectorType *VecType = dyn_cast<VectorType>(type))
    type = VecType->getElementType();
  PointerType *pointerType = cast<PointerType>(type);
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
    if (VectorType *VecType = dyn_cast<VectorType>(type))
      type = VecType->getElementType();
    pointerType = cast<PointerType>(type);
    unsigned int valueAddressSpace = pointerType->getAddressSpace();

    if(!isEscaping && addressSpace != valueAddressSpace && valueAddressSpace != OCLAddressSpace::Private && valueAddressSpace != OCLAddressSpace::Generic) {
        // Check if the same pointer value was not resovled into different named address spaces. E.g. __local and __global.
        if(addressSpace != OCLAddressSpace::Private && addressSpace != OCLAddressSpace::Generic)
            isEscaping = true;
        addressSpace = valueAddressSpace;
    }

    // Check whether the pointer value stems from or gets casted to an int.
    if (!isEscaping && (isa<IntToPtrInst>(pointerValue)))
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
AliasResult OCLAAResult::alias(const MemoryLocation &LocA,
                               const MemoryLocation &LocB,
                               AAQueryInfo &AAQI) {
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
        return AliasResult::NoAlias;
      }
    }
  }

  return AAResultBase::alias(LocA, LocB, AAQI);
}

bool OCLAAResult::pointsToConstantMemory(const MemoryLocation &Loc,
                                         AAQueryInfo &AAQI, bool OrLocal) {
  const Value *V = Loc.Ptr;

  // Remove casts if exists
  V = V->stripPointerCasts();

  if (PointerType *VP = dyn_cast<PointerType>(V->getType())) {
    // If V is a pointer to the OCLAddressSpace::Constant address space
    // then it points to __constant OpenCL memory.
    if (VP->getAddressSpace() == OCLAddressSpace::Constant)
      return true;
  }

  return AAResultBase::pointsToConstantMemory(Loc, AAQI, OrLocal);
}

char OCLAliasAnalysis::ID = 0;
}

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  ImmutablePass* createOCLAliasAnalysisPass() {
    return new intel::OCLAliasAnalysis();
  }
}
