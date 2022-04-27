//==----------------- ReduceCrossBarrierValues.h ---------------- C++ -*---==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_REDUCE_CROSS_BARRIER_VALUES_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_REDUCE_CROSS_BARRIER_VALUES_H

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
/// The algorithm is described as follows.
///
/// Assumption
/// ==========
/// All barriers must be at the beginning of basic blocks. This is already done
/// by SplitBBonBarrier pass.
///
/// Terms
/// =====
/// - Sync basic block
///   A sync basic block (Sync BB) is a basic block who contains a barrier or a
///   dummy barrier. And the barrier is the first instruction of the basic
///   block according to the assumption above.
///
/// - Barrier region header
///   A basic block is a barrier region header iff it's
///     1) the entry block of a function, or
///     2) a sync BB, or
///     3) a dominance frontier of another region header.
///
/// - Barrier region
///   A barrier region starts with a region header, and contains a set of basic
///   blocks. A basic block (not a header) belongs to a barrier region iff its
///   idom
///     1) is the region header, or
///     2) belongs to the region.
///
/// Example
/// =======
/// Given a CFG as follows,
///
///       A
///   ,.-'|
///  |   *B
///  |   / \
///  |  C   D-.
///  |  |   |  | (backedge from F to D)
///  | *E  *F-'
///  |   \ /
///  |    G
///   `.  |
///     '-H
///       |
///      *I
///   (Blocks with * are sync BBs, i.e.,
///    block B, E, F and I start with barrier.)
///
/// barrier region headers are
///  - A (entry block)
///  - B, E, F, I (sync BB)
///  - D (dominance frontier of F)
///  - G (dominance frontier of A)
///  - H (dominance frontier of G)
///
/// so there are 8 barrier regions in total:
///  {A}, {B C}, {D}, {E}, {F}, {G}, {H} and {I}
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
/// Author: Senran Zhang
///
/// This pass was converted from a function pass to a module pass, because it
/// depends on module-level analysis results (DPV, WIRV, DPB) while those
/// analyses are not immutable (the new PM requires outer-level analysis results
/// must not be invalidated).
class ReduceCrossBarrierValuesPass
    : public PassInfoMixin<ReduceCrossBarrierValuesPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  // Glue for old PM.
  bool runImpl(Module &M, BuiltinLibInfo *BLI, DataPerValue *DPV,
               WIRelatedValue *WIRV, DataPerBarrier *DPB,
               function_ref<DominanceFrontier &(Function &)> GetDF,
               function_ref<DominatorTree &(Function &)> GetDT);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_REDUCE_CROSS_BARRIER_VALUES_H
