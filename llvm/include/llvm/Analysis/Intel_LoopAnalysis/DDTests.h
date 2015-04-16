//===-------- DDTest.h - Provides Data Dependence Analysis-*-- C++ --*-===//
//
//                     The LLVM Compiler Infrastructure
//
// TODO LICENSE
//===----------------------------------------------------------------------===//
//  This provides the implementations of data dependence tests between a pair
//  of ddrefs given an input direction vector. 
//  Only function available is findDependences which is test driver. It will
//  find and invoke appropriate test, providing forward and backwards DV
//  Requires DDrefs and HIR to which they are attached to be valid
//===----------------------------------------------------------------------===//

#ifndef INTEL_LOOPANALYSIS_DDTEST
#define INTEL_LOOPANALYSIS_DDTEST

#include "llvm/Analysis/Intel_LoopAnalysis/DDGraph.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
namespace llvm {
namespace loopopt {
class DDRef;
class DirectionVector;

// \brief Test driver. It will determine appropriate specific DD tests(ie
// GACD, zero SIV, banerjee etc) and invoke it. Returns true if either forward
// or backwards dependence found, with direction vector filled in.
// For inputDV, only = and * is valid. 
// For ouput dvs, forward and reversedv, a DV of all UNINIT means no dependence
// IE test determines only a forward dep exists, this function returns true 
// with forwarddv filled in, and a reverse dv of uninit
bool findDependences(DDRef *SrcRef, DDRef *SinkRef, DirectionVector InputDV,
                     DirectionVector &ForwardDV, DirectionVector &ReverseDV);
}
}
#endif
