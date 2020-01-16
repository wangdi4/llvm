#if INTEL_COLLAB // -*- C++ -*-
//===- DebugInfoTransform.cpp - Debug Information Transformations ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Performs per-module transformations on debug info metadata.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/DebugInfoTransform.h"
#include "llvm/Support/Debug.h"

// DEBUG_TYPE
//
// The DEBUG_TYPE is used to enable trace output from this component.
//
// To enable trace output the flag "-debug-only debug-info-transform" must be
// passed to the compiler like this:
//
//   $ clang -c -g inputfile.cpp -mllvm -debug-only -mllvm debug-info-transform
//                               ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// The trace output can be extended by adding lines like the following:
//   LLVM_DEBUG(dbgs() << "My trace output goes here!\n");
//
#define DEBUG_TYPE "debug-info-transform"

namespace llvm {

DebugInfoTransform::DebugInfoTransform() : Map() {}
DebugInfoTransform::~DebugInfoTransform() { Map.clear(); }

void DebugInfoTransform::mapMetadata(MDNode *Original, MDNode *Clone) {
  Map.insert({Original, Clone});
}

MDNode *DebugInfoTransform::cloneMetadata(MDNode *Original) {
  if (Original == nullptr)
    return nullptr;

  if (ignore(Original))
    return Original;

  // We should probably find a way to reuse ValueToValueMap for the map.
  // I tried briefly but never found a way to make it work for this.
  auto Cached = Map.find(Original);
  if (Cached != Map.end())
    return Cached->second;

  // Clone this node.
  TempMDNode Temp = Original->clone();

  // Recursively (deep) clone the metadata operands.
  bool Changed = false;
  for (unsigned I = 0, E = Original->getNumOperands(); I != E; ++I) {
    Metadata *Old = Original->getOperand(I);
    Metadata *New;
    if (Old && isa<MDNode>(Old)) {
      New = cloneMetadata(dyn_cast_or_null<MDNode>(Old));
      if (Old != New) {
        Temp->replaceOperandWith(I, New);
        Changed = true;
      }
    }
  }
  MDNode *Clone;
  if (Changed)
    Clone = MDNode::replaceWithPermanent(std::move(Temp));
  else
    Clone = Original;

  if (Changed) {
    LLVM_DEBUG(dbgs() << "CLONE: "; Original->dump(););
  }

  // Insert the cloned entry into the map.
  mapMetadata(Original, Clone);

  return Clone;
}

} // namespace llvm

#endif // INTEL_COLLAB

