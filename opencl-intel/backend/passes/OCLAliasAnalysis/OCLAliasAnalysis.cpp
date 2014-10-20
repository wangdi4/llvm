/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "OCLAliasAnalysis.h"
#include "OCLPassSupport.h"
#include "OCLAddressSpace.h"
#include "llvm/ADT/SmallPtrSet.h"
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
OCLAliasAnalysis::ResolveResult OCLAliasAnalysis::resolveAddressSpace(const Value& value) {
  std::queue<const Value*> nextPointerValues;
  SmallPtrSet<const Value*, 16> visitedPointerValues;

  // Assume this is the default namespace.
  unsigned int addressSpace = OCLAddressSpace::Private;
  // Keep track of whether this pointer originates from or gets casted to an int.
  bool isEscapingToInt = false;

  nextPointerValues.push(&value);
  do {
    const Value* pointerValue = nextPointerValues.front();
    nextPointerValues.pop();
    if (!visitedPointerValues.insert(pointerValue))
      continue;

    Type* type = pointerValue->getType();
    PointerType *pointerType = dyn_cast<PointerType>(type);
    assert(pointerType && "Value is not of a pointer type");
    unsigned int valueAddressSpace = pointerType->getAddressSpace();

    assert((addressSpace == OCLAddressSpace::Private || addressSpace == valueAddressSpace) &&
	   "Address used for more than one address space");
    addressSpace = valueAddressSpace;

    // Check whether the pointer value stems from or gets casted to an int.
    if (isa<IntToPtrInst>(pointerValue) || isa<PtrToIntInst>(pointerValue))
      isEscapingToInt = true;

    // We assume address space consistency: one explicit relation of the
    // address to an non-default address space is enough, as there is no
    // way in OpenCL to escapse out of an address space.
    if (addressSpace != OCLAddressSpace::Private)
      break;

    // Mark this value's users to be visited.
    for (Value::const_use_iterator it = pointerValue->use_begin(), end = pointerValue->use_end(); it != end; ++it)
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

  return ResolveResult(!isEscapingToInt, addressSpace);
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
    ResolveResult rrV1 = resolveAddressSpace(*V1);
    ResolveResult rrV2 = resolveAddressSpace(*V2);
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
