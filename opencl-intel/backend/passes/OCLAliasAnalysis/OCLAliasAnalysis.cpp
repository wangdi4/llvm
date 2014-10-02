/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "OCLAliasAnalysis.h"
#include "OCLPassSupport.h"
#include "OCLAddressSpace.h"

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

// Check aliasing using the fact that in openCL pointers from different
// memory addresses do not alias.
AliasAnalysis::AliasResult OCLAliasAnalysis::alias(const Location &LocA,
                                                   const Location &LocB) {

  const Value *V1 = LocA.Ptr;
  const Value *V2 = LocB.Ptr;
  // Remove casts if exists
  V1 = V1->stripPointerCasts();
  V2 = V2->stripPointerCasts();

  PointerType *V1P = dyn_cast<PointerType>(V1->getType());
  PointerType *V2P = dyn_cast<PointerType>(V2->getType());
  if (V1P && V2P) {
    // If V1 and V2 are pointers to different address spaces then they do not alias.
    // This is not true in general, however, in openCL the memory addresses
    // for private, local, and global are disjoint.
    int V1PAddressSpace = V1P->getAddressSpace();
    int V2PAddressSpace = V2P->getAddressSpace();
    if (isInSpace(V1PAddressSpace, m_disjointAddressSpaces) &&
        isInSpace(V2PAddressSpace, m_disjointAddressSpaces) &&
        V1PAddressSpace != V2PAddressSpace)
      return NoAlias;
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
