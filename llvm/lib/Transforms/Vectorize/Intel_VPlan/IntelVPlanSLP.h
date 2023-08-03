//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPlanSlp class which incorporates SLP related methods
/// that are run on top of VPlan IR.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANSLP_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANSLP_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"

namespace llvm {

namespace vpo {

class VPBasicBlock;
class VPValue;
class VPInstruction;
class VPInstructionCost;
class VPlanTTICostModel;
class VPLoadStoreInst;

class VPlanSlp {
  VPlanTTICostModel *CM;
  // Only instructions belonging BB are subject for SLP.
  const VPBasicBlock *BB;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Used in debugging dumps only.
  unsigned BundleID;
  unsigned GraphID;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Return true if at least two values from Values array can be represented
  // with a vector instruction.
  //
  // The utility populates Insts with vectoriable Values casted to
  // VPInstruction type.
  bool areVectorizable(ArrayRef<const VPValue *> Values,
                       SmallVectorImpl<const VPInstruction *> &Insts) const;

  // Returns true iff VPInstruction FromInst can be moved to ToInst.
  // TODO: It might be duplication of VPVLSClientMemref::canMoveTo().
  // We want these utilities to be merged once VPlanSlp matures.
  bool canMoveTo(const VPLoadStoreInst *FromInst,
                 const VPLoadStoreInst *ToInst) const;

  // Tries to build an SLP tree starting at 'Seed' list and returns the
  // difference (vectorized_graph_cost - scalar_graph_cost). Thereby if the
  // cost is negative, it is profitable to vectorize. If the cost is Invalid,
  // the graph is not vectorizable with SLP.
  VPInstructionCost buildGraph(ArrayRef<const VPInstruction *> Seed);

  // Searches for SLP patters that start at 'Seed' as seed instructions.
  // The utility modifies input Seed vector sorting it and it is truncated
  // at function's exit.
  //
  // Return the sum of the gain costs of profitable to SLP subgraphs only.
  VPInstructionCost searchSLPPatterns(
      SmallVectorImpl<const VPInstruction *> &Seed);

  // Utility collects the constant distances between Memrefs in Insts array
  // and 'BaseMem' memory reference.
  //
  // Only constant distances are collected. Only load and stores are expected
  // in Insts array. Every load/store in input array is expected to have
  // DDRef node associated.
  static void collectMemRefDistances(const VPLoadStoreInst *BaseMem,
                                     ArrayRef<const VPInstruction *> Insts,
                                     SmallVectorImpl<ssize_t> &Distances);

  // The utility sorts 'Distances' array and checks if it is indexes for
  // consequent in memory elements. Such, it return true for
  // 0, 1, 2, 3
  // -2, -1, 0, 1
  // 3, 2, 1, 0
  //
  // but false for:
  // 0, 2, 4, 6
  // 3, 2, 1, -1
  //
  // The number of elements is an arbitrary (it is not nessesary power of two).
  bool isUnitStrideMemRef(SmallVectorImpl<ssize_t> &Distances) const;

  // Calculates the cost of a vector instruction that would be produced from
  // scalar instruction Base extending it for VF elements. If Base is a memory
  // reference 'IsUnitMemref' indicates when the extention of the memory
  // results a whole chunk of memory access. VF which is a power of two is
  // supported only.
  // Non power-of-two memref size is indicated with IsMasked.
  VPInstructionCost getVectorCost(const VPInstruction *Base, unsigned VF,
                                  bool IsUnitMemref, bool IsMasked) const;

  // Returns the difference (vectorized_cost - scalar_cost), where
  // scalar_cost - the sum of all TTI costs of interuction in Insts array,
  // vectorized_cost - TTI cost of vector instruction that implements
  // operation equivalent to what all Insts do.
  VPInstructionCost estimateSLPCostDifference(
      ArrayRef<const VPInstruction *> Insts) const;

  // The method tries to bundle 'similar' instructions in groups and see if
  // those groups are profitable to SLP. The group is discarded (not stored
  // in OutSeed) if it is vectorizable. Not vectorized groups are stored in
  // OutSeed array if OutSeed is not null.
  //
  // 'similarity' of instructions inside InSeed is determined by Compare
  // comparator, which return true when two instructions are similar enough to
  // have a potential for vectorization in a single vector. Please note that
  // this is a quick heuristics based check rather than a legality check.
  //
  // The costs of profitable to vectorize groups is accumulated and returned.
  VPInstructionCost formAndCostBundles(
    ArrayRef<const VPInstruction *> InSeed,
    std::function<bool(const VPInstruction *,
                       const VPInstruction *) > Compare,
    SmallVectorImpl<const VPInstruction *> *OutSeed = nullptr);

public:
  VPlanSlp(VPlanTTICostModel *CM, const VPBasicBlock *BB) :
    CM(CM), BB(BB)
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    , BundleID(0), GraphID(0)
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  {}

  ~VPlanSlp() = default;

  // Tries to detect SLP patterns and return the cost gain if found. The
  // result is either a negative cost, which is a gain for SLP, or 0 if
  // no pattern is found.
  VPInstructionCost estimateSLPCostDifference();
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANSLP_H
