#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPLOOPINFO_H 
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPLOOPINFO_H

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopInfoImpl.h"

namespace llvm {

class VPBlockBase;

namespace vpo {


/// A VPLoop holds analysis information for every loop detected by VPLoopInfo.
/// It is a specialization of LoopBase class that also implements some
/// functionality already implemented in Loop (BasicBlock specialization).
class VPLoop : public LoopBase<VPBlockBase, VPLoop> {

public:
  VPLoop(VPBlockBase *Header) : LoopBase(Header) {}

  /// Return true if no exit block for the loop has a predecessor that is
  /// outside the loop.
  bool hasDedicatedExits() const;

  /// Return all unique successor blocks of this loop.
  /// These are the blocks _outside of the current loop_ which are branched to.
  /// This assumes that loop exits are in canonical form.
  void getUniqueExitBlocks(SmallVectorImpl<VPBlockBase *> &ExitBlocks) const;

  /// If getUniqueExitBlocks would return exactly one block, return that block.
  /// Otherwise return null.
  VPBlockBase *getUniqueExitBlock() const;
};

/// VPLoopInfo provides analysis of natural loop for VPBlockBase-based
/// Hierarchical CFG. It is a specialization of LoopInfoBase class. 
class VPLoopInfo : public LoopInfoBase<VPBlockBase, VPLoop> {

public:
  VPLoopInfo() {}

  size_t getNumTopLevelLoops() const {
    // TODO: TopLevelLoops is private
    //return TopLevelLoops.size();
   
    return std::distance(begin(), end());
  }

  const VPLoop *getLoopFromPreHeader(const VPBlockBase *PotentialPH) const;
  VPLoop *getLoopFromPreHeader(VPBlockBase *PotentialPH);
};

} // End VPO Vectorizer Namespace 
}  // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_VPLAN_VPLOOPINFO_H 

