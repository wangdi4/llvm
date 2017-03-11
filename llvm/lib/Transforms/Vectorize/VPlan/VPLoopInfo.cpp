
#include "VPLoopInfo.h"
#include "IntelVPlan.h"

using namespace llvm;
using namespace llvm::vpo;

// Replicated from class Loop with minor changes (successors/predecessors
// iterators)
// TODO: Would it possible to move this to LoopBase?

bool VPLoop::hasDedicatedExits() const {
  // Each predecessor of each exit block of a normal loop is contained
  // within the loop.
  SmallVector<VPBlockBase *, 4> ExitBlocks;
  getExitBlocks(ExitBlocks);
  for (VPBlockBase *BB : ExitBlocks)
    for (VPBlockBase *Predecessor : BB->getPredecessors())
      if (!contains(Predecessor))
        return false;
  // All the requirements are met.
  return true;
}

void VPLoop::getUniqueExitBlocks(
    SmallVectorImpl<VPBlockBase *> &ExitBlocks) const {

  assert(hasDedicatedExits() &&
         "getUniqueExitBlocks assumes the loop has canonical form exits!");

  SmallVector<VPBlockBase *, 32> SwitchExitBlocks;
  for (VPBlockBase *BB : this->blocks()) {
    SwitchExitBlocks.clear();
    for (VPBlockBase *Successor : BB->getSuccessors()) {
      // If block is inside the loop then it is not an exit block.
      if (contains(Successor))
        continue;

      auto PI = Successor->getPredecessors().begin();
      VPBlockBase *FirstPred = *PI;

      // If current basic block is this exit block's first predecessor
      // then only insert exit block in to the output ExitBlocks vector.
      // This ensures that same exit block is not inserted twice into
      // ExitBlocks vector.
      if (BB != FirstPred)
        continue;

      // If a terminator has more then two successors, for example SwitchInst,
      // then it is possible that there are multiple edges from current block
      // to one exit block.
      if (std::distance(BB->getSuccessors().begin(),
                        BB->getSuccessors().end()) <= 2) {
        ExitBlocks.push_back(Successor);
        continue;
      }

      // In case of multiple edges from current block to exit block, collect
      // only one edge in ExitBlocks. Use switchExitBlocks to keep track of
      // duplicate edges.
      if (!is_contained(SwitchExitBlocks, Successor)) {
        SwitchExitBlocks.push_back(Successor);
        ExitBlocks.push_back(Successor);
      }
    }
  }
}

VPBlockBase *VPLoop::getUniqueExitBlock() const {
  SmallVector<VPBlockBase *, 8> UniqueExitBlocks;
  getUniqueExitBlocks(UniqueExitBlocks);
  if (UniqueExitBlocks.size() == 1)
    return UniqueExitBlocks[0];
  return nullptr;
}

const VPLoop *
VPLoopInfo::getLoopFromPreHeader(const VPBlockBase *PotentialPH) const {

  // VPLoop PH has a single successor
  const VPBlockBase *PotentialH = PotentialPH->getSingleSuccessor();

  if (!PotentialH || !isLoopHeader(PotentialH))
    return nullptr;

  // PotentialPH's successor is Loop H
  const VPLoop *VPL = getLoopFor(PotentialH);
  assert(VPL && "VPLoop is nullptr");

  // Returns VPL only if PotentialPH is VPL PH
  return (VPL->getLoopPreheader() == PotentialPH) ? VPL : nullptr;
}

VPLoop *VPLoopInfo::getLoopFromPreHeader(VPBlockBase *PotentialPH) {
  return const_cast<VPLoop *>(
      static_cast<const VPLoopInfo *>(this)->getLoopFromPreHeader(PotentialPH));
}

