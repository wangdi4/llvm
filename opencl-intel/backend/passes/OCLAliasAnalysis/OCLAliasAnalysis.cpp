/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "OCLAliasAnalysis.h"
#include "OCLPassSupport.h"
#include "OCLAddressSpace.h"
#include <queue>

using namespace Intel::OpenCL::DeviceBackend::Utils;
using namespace llvm;

namespace intel {

OCL_INITIALIZE_AG_PASS_BEGIN(OCLAliasAnalysis, AliasAnalysis, "ocl-asaa",
                   "OpenCL Address Space Alias Analysis",
                   false, true, false)
OCL_INITIALIZE_AG_PASS_END(OCLAliasAnalysis, AliasAnalysis, "ocl-asaa",
                   "OpenCL Address Space Alias Analysis",
                   false, true, false)

OCLAliasAnalysis::OCLAliasAnalysis() : ImmutablePass(ID) {
  initializeOCLAliasAnalysisPass(*PassRegistry::getPassRegistry());
  m_disjointAddressSpaces = getAddressSpaceMask(OCLAddressSpace::Private) |
                            getAddressSpaceMask(OCLAddressSpace::Global) |
                            getAddressSpaceMask(OCLAddressSpace::Constant) |
                            getAddressSpaceMask(OCLAddressSpace::Local);
}

void OCLAliasAnalysis::deleteValue (Value *V) {
  m_cachedResults.erase(V);
}
void OCLAliasAnalysis::copyValue (Value *From, Value *To) {
  // Do nothing, i.e. let new value compute in resolveAddressSpace.
}
void OCLAliasAnalysis::addEscapingUse (Use &U) {
  resolveAddressSpace(U.get(), true);
}

OCLAliasAnalysis::ResolveResult
OCLAliasAnalysis::cacheResult(SmallValueSet& values,
			      ResolveResult resolveResult) {
  for (SmallValueSet::iterator it = values.begin(); it != values.end(); ++it) {
    m_cachedResults.insert(std::make_pair(*it, resolveResult));
  }
  return resolveResult;
}

// We may be given a value which by itself does not explicitly specify an OpenCL address
// space although the address does belong to one (e.g. alloca instructions are used for
// both Local and Private memory, but do not explicitly specify the addrspace.
// Since we cannot distinguish OpenCL's Private address space from LLVM's default (i.e. no)
// address space, we do not trust the value's apparent Private address-space and attempt
// to find the explicit namespace among the value's users and used base address
// value (e.g. the pointer operand of a GEP).
// Since OpenCL's address spaces are disjoint, a single relation of an address to a non-default
// address space is enough to determine the address space. We therefore declare the address
// space to be Private only if:
// - we cannot find any explicit relation of it to another address space, and
// - the pointer is not escaping our analysis via casting to/from int
// (note that when we do find a non-default address space we don't care if the pointer is casted
// to/from int as no pointer arithmetic can escape the OpenCL address space).
OCLAliasAnalysis::ResolveResult OCLAliasAnalysis::resolveAddressSpace(const Value* value, bool force) {
  std::queue<const Value*> nextPointerValues;
  SmallValueSet visitedPointerValues;

  // Assume this is the default namespace.
  unsigned int addressSpace = OCLAddressSpace::Private;
  // Keep track of whether this pointer originates from or gets casted to an int.
  bool isEscaping = false;

  nextPointerValues.push(value);
  do {
    const Value* pointerValue = nextPointerValues.front();
    nextPointerValues.pop();

    if(!force) {
      std::map<const Value*, ResolveResult>::iterator it = m_cachedResults.find(pointerValue);
      if (it != m_cachedResults.end()) {
        return cacheResult(visitedPointerValues, it->second);
      }
    }

    if (!visitedPointerValues.insert(pointerValue).second)
      continue;

    Type* type = pointerValue->getType();
    PointerType *pointerType = dyn_cast<PointerType>(type);
    assert(pointerType && "Value is not of a pointer type");
    unsigned int valueAddressSpace = pointerType->getAddressSpace();

    if(addressSpace != OCLAddressSpace::Private && valueAddressSpace != OCLAddressSpace::Private && addressSpace != valueAddressSpace)
      isEscaping = true;

    if(valueAddressSpace != OCLAddressSpace::Private)
      addressSpace = valueAddressSpace;

    // Check whether the pointer value stems from or gets casted to an int.
    if (isa<IntToPtrInst>(pointerValue) || isa<PtrToIntInst>(pointerValue))
      isEscaping = true;

    // Mark this value's users to be visited.
    for (Value::const_user_iterator it = pointerValue->user_begin(), end = pointerValue->user_end(); it != end; ++it)
      if (isa<BitCastInst>(*it) || isa<GetElementPtrInst>(*it))
	nextPointerValues.push(*it);

    // Also mark the base address this value is using to be visited, in case the
    // address space was for some reason stripped.
    if (const BitCastInst* bitCastInst = dyn_cast<BitCastInst>(pointerValue)) {
      nextPointerValues.push(bitCastInst->getOperand(0));
    } else if (const GetElementPtrInst* gepInst = dyn_cast<GetElementPtrInst>(pointerValue)) {
      nextPointerValues.push(gepInst->getPointerOperand());
    }
  } while (!nextPointerValues.empty());

  ResolveResult resolveResult = ResolveResult(!isEscaping, addressSpace);
  return cacheResult(visitedPointerValues, resolveResult);
}

// Check aliasing using the fact that in openCL pointers from different
// memory addresses do not alias.
AliasAnalysis::AliasResult OCLAliasAnalysis::alias(const Location &LocA,
                                                   const Location &LocB) {

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

  return AliasAnalysis::alias(LocA, LocB);
}

bool OCLAliasAnalysis::pointsToConstantMemory(const Location &Loc, bool OrLocal) {

  const Value *V = Loc.Ptr;

  // Remove casts if exists
  V = V->stripPointerCasts();

  if (PointerType *VP = dyn_cast<PointerType>(V->getType())) {
    // If V is a pointer to the OCLAddressSpace::Constant address space
    // then it points to __constant OpenCL memory.
    if (VP->getAddressSpace() == OCLAddressSpace::Constant)
      return true;
  }

  return AliasAnalysis::pointsToConstantMemory(Loc, OrLocal);
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
