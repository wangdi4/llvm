//===- DDTests.cpp - Assigns symbase to ddrefs *- C++ -*---------===//
//
//                     The LLVM Compiler Infrastructure
//
// TODO License for Intel
//
//===----------------------------------------------------------------------===//
//
// This file implements the our dd tests.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/DDTests.h"

#include "llvm/Support/Debug.h"
using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "dd-tests"

namespace {
// test declarations go here

// this test is merely to ensure everything is linking up properly
bool sillyTest(DDRef *Ref1, DDRef *Ref2, DirectionVector InputDV);
}
bool llvm::loopopt::findDependences(DDRef *SrcRef, DDRef *SinkRef,
                                    DirectionVector InputDV,
                                    DirectionVector &ForwardDV,
                                    DirectionVector &ReverseDV) {

  ForwardDV = InputDV;
  ReverseDV = InputDV;

  DEBUG(dbgs() << "Testing Src: ");
  DEBUG(SrcRef->dump());

  DEBUG(dbgs() << " against Sink: ");
  DEBUG(SinkRef->dump());
  DEBUG(dbgs() << " with DirV ");
  DEBUG(InputDV.dump());
  DEBUG(dbgs() << "\n");

  return sillyTest(SrcRef, SinkRef, InputDV);
}

namespace {
// test definitions go here

// TODO replace this with a real test
bool sillyTest(DDRef *Ref1, DDRef *Ref2, DirectionVector InputDV) {
  return true;
}
}

