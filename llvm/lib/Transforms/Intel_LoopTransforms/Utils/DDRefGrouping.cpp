//===-------- DDRefGrouping.cpp - Implements DDRef Grouping utilities -----===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements DDRefGrouping class.
//
//===----------------------------------------------------------------------===//
#include "llvm/Support/Debug.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGrouping.h"

using namespace llvm;
using namespace llvm::loopopt;

// Used primarily for debugging.
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DDRefGrouping::dump(const RefGroupsTy &Groups) {
  dbgs() << "\n Reference Groups \n";
  for (auto SymVecPair = Groups.begin(), Last = Groups.end();
       SymVecPair != Last; ++SymVecPair) {
    auto &RefVec = SymVecPair->second;
    dbgs() << "Group " << SymVecPair->first
           << " {sb: " << RefVec.front()->getSymbase() << "} contains: \n";
    for (auto Ref = RefVec.begin(), E = RefVec.end(); Ref != E; ++Ref) {
      dbgs() << "\t";
      (*Ref)->dump();
      dbgs() << " -> isWrite:" << (*Ref)->isLval() << "\n";
    }
  }
}
#endif
