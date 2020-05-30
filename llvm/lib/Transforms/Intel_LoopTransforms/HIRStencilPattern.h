#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSTENCIPATTERN_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSTENCIPATTERN_H

#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"

/// Checks if the innermost loop body has a certain stencil pattern.
/// Checks
///  - kinds of binary operations.
///  - Per memref group (grouping is done by RefGrouper)
///    + Get the median by compareMemRefAddress.
///    + Check all other memrefs in the same group has
///      a constant distance.
///    + for a 3-D reference, upto 2 dimensions can have different
///      from those of median ref.
namespace llvm {

namespace loopopt {

namespace stencilpattern {

typedef DDRefGrouping::RefGroupVecTy<RegDDRef *> RefGroupVecTy;
typedef DDRefGrouping::RefGroupTy<RegDDRef *> RefGroupTy;

// If the group of refs consitute a structurally stencil pattern.
// It internally calls getMedianRef and isSymetricCenteredAt.
bool areStructuallyStencilRefs(RefGroupTy &Group);

// Get the median of all given refs.
// Assumes all refs are memrefs with the same base ptr.
// If not, the result is meaningless.
const RegDDRef *getMedianRef(RefGroupTy &Group);

// Given Center ref, see if all refs have symetric an opposite in the Group.
// Caveat: refs without no diffs from Center Ref are counted
//         as a stencil ref. E.G.) A[i][j] is center and all refs
//         are A[i][j], it is considered a stencil ref.
//         On the other hand, A[i+1][j-1] is not considred as
//         a stencil ref because numCEs = 2 >= numCEs with diff.
//         In general, A[i][j][k], intend was to make sure
//         A[i-1][j][k] (numCEs with diff == 1),
//         A[i+1][j][k-1] (numCEs with diff == 2) and so on
//         are stencil refs.
bool isSymetricCenteredAt(const RegDDRef *Center, const RefGroupTy &Group);
} // namespace stencilpattern

} // namespace loopopt

} // namespace llvm

#endif /* LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSTENCIPATTERN_H*/
