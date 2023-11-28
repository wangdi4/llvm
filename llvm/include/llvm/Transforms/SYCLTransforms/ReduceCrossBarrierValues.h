//==----------------- ReduceCrossBarrierValues.h ---------------- C++ -*---==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_REDUCE_CROSS_BARRIER_VALUES_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_REDUCE_CROSS_BARRIER_VALUES_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class BuiltinLibInfo;
class DataPerBarrier;
class DataPerValue;
class DominanceFrontier;
class DominatorTree;
class WIRelatedValue;

/// This pass reduce cross-barrier values by copying their definitions to where
/// they're used. E.g., given pseudo code as follows,
///
///   id = get_global_id(0);
///   i = id * 10;
///   ... a[i] ...
///   ....
///   barrier()
///   ... b[i] ...
///
/// we copy `id` and `i` below the barrier, and replace the `i` in `b[i]` to
/// the copied one,
///
///   id = get_global_id(0);
///   i = id * 10;
///   ... a[i] ...
///   ....
///   barrier()
///   id_copy = get_global_id(0);
///   i_copy = id_copy * 10;
///   ... b[i_copy] ...
///
/// Notes
/// =====
/// The reason to construct barrier region is to share a same copy of a def
/// among its uses within a region, and thus avoid duplicate copying.
///
/// Algorithm
/// =========
/// Collect all cross-barrier def-use's;
/// Sort all uses in topological order;
/// Found all barrier regions using DFS;
/// Init an empty map M to hold maps between original defs and their copies per
/// region;
/// For each cross-barrier use U:
///   Determine the region R it belongs to;
///   Iteratively copy U and its WI-related operands into the region (When
///   copying, if the region header is a dominance frontier, and all of its
///   predeceasing regions have already contained the instruction to copy or
///   its copies, just create a Phi node in R and stop copying instructions it
///   depends on instead of copying the instruction).
///
///
/// This pass was converted from a function pass to a module pass, because it
/// depends on module-level analysis results (DPV, WIRV, DPB) while those
/// analyses are not immutable (the new PM requires outer-level analysis results
/// must not be invalidated).
class ReduceCrossBarrierValuesPass
    : public PassInfoMixin<ReduceCrossBarrierValuesPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_REDUCE_CROSS_BARRIER_VALUES_H
