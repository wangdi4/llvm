//===-- CSALicGrouping.cpp - CSA convert control flow to data flow --------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains the code that passes information from the CFG phase of
// the compiler to the dataflow phase of the compiler. It contains some of the
// methods of CSACvtCFDFPass.
//
//===----------------------------------------------------------------------===//

#include "CSACvtCFDFPass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "csa-cvt-cf-df-pass"

static unsigned getVregIndex(const MachineOperand &MO) {
  if (MO.isReg() && Register::isVirtualRegister(MO.getReg()))
    return Register::virtReg2Index(MO.getReg());
  return ~0U;
}

void CSACvtCFDFPass::findLICGroups(bool preDFConversion) {
  LLVM_DEBUG(dbgs() << "Mapping groups for LICs" <<
             (preDFConversion ? " and blocks" : "") << "\n");
  // Ensure that we can handle all of the virtual registers in the function.
  licGrouping.grow(MRI->getNumVirtRegs());

  auto joinWithGroup = [&](unsigned operand, unsigned &groupOperand) {
    if (operand == UNMAPPED_REG)
      return;

    if (groupOperand == UNMAPPED_REG) {
      groupOperand = operand;
    } else {
      licGrouping.join(groupOperand, operand);
    }
 };

  auto joinVregs = [&](const MachineInstr &MI, ArrayRef<unsigned> OpIndexes) {
    unsigned GroupOperand = UNMAPPED_REG;
    for (unsigned Index : OpIndexes) {
      joinWithGroup(getVregIndex(MI.getOperand(Index)), GroupOperand);
    }
  };

  // Keep track of parallel incoming edges for picks. Note that the pick inputs
  // can be grouped due to identical control even if coming from different
  // basic blocks, so we track the two groups across the whole function.
  SmallVector<unsigned, 0> pickConditions;
  SmallVector<unsigned, 0> pickTrueRegs;
  SmallVector<unsigned, 0> pickFalseRegs;

  for (auto &BB : *thisMF) {
    unsigned joinGroupVreg = UNMAPPED_REG;

    // Keep track of parallel outgoing edges for switches. It may be the case
    // that outgoing switches on more than one condition show up, so we build
    // a vector to keep track of the various options. Note that we only need to
    // keep track of switches with distinct condition registers--in structured
    // code, this should only be at most one of these per basic block.
    SmallVector<unsigned, 2> switchConditions;
    SmallVector<unsigned, 2> switchTrueRegs;
    SmallVector<unsigned, 2> switchFalseRegs;

    for (auto &MI : BB) {
      // If we are after dataflow conversion, then we can only group registers
      // according to the current instruction, not the block as a whole. Reset
      // the vreg to keep track of that.
      if (!preDFConversion)
        joinGroupVreg = UNMAPPED_REG;

      if (TII->isMOV(&MI) || MI.getOpcode() == CSA::COPY) {
        // While these are regular ops, these could be inserted in between
        // operations that push into or out of high-level LICs. We should tie
        // the operands together, but we shouldn't tie the frequencies directly
        // to the group of the current block. Before dataflow conversion, we
        // can't guarantee that the source value has been switched yet, so we
        // can't do anything at all.
        if (!preDFConversion)
          joinVregs(MI, {0, 1});
      } else if (!TII->isMultiTriggered(&MI) ||
          TII->getGenericOpcode(MI.getOpcode()) == CSA::Generic::COMPLETION) {
        // Non-multi-triggered operations are easy: all LIC operands must
        // execute the same number of times.
        // Completion buffers are multi-triggered operations in general, but
        // they do eventually spit out as many values as they take in (just in a
        // different order), so the execution counts are guaranteed to be the
        // same.
        for (auto &op : MI.operands()) {
          // Is this operand a LIC?
          unsigned opIndex = getVregIndex(op);
          if (opIndex == UNMAPPED_REG)
            continue;

          // Before dataflow conversion, an operation in an inner loop may be
          // using a vreg set in a loop preheader. Dataflow conversion would
          // insert extra steering operations to make sure that the counts are
          // equal, but that hasn't happened yet. Therefore, we can only
          // properly worry about these registers at their definition, not site
          // of use.
          if (preDFConversion && !op.isDef())
            continue;

          joinWithGroup(opIndex, joinGroupVreg);
        }
      } else if (TII->isInit(&MI)) {
        // Do nothing. The resulting output is already picked up by another
        // definition, so we shouldn't need to do anything here (not that it's
        // clear we can do anything).
      } else if (TII->isPick(&MI)) {
        // Join the define and the control operand to the same group. We don't
        // need to attempt to join it with the basic block, since it shouldn't
        // exist before we run dataflow conversion.
        unsigned opDef = getVregIndex(MI.getOperand(0));
        unsigned opCtl = getVregIndex(MI.getOperand(1));
        unsigned opFalse = getVregIndex(MI.getOperand(2));
        unsigned opTrue = getVregIndex(MI.getOperand(3));
        if (opDef != UNMAPPED_REG && opCtl != UNMAPPED_REG)
          joinWithGroup(opDef, opCtl);

        // Only map the true/false legs if the control operand has a meaningful
        // value.
        if (opCtl != UNMAPPED_REG) {
          auto it = std::find(pickConditions.begin(),
              pickConditions.end(), opCtl);
          if (it == pickConditions.end()) {
            pickConditions.push_back(opCtl);
            pickTrueRegs.push_back(opTrue);
            pickFalseRegs.push_back(opFalse);
          } else {
            auto index = it - pickConditions.begin();
            joinWithGroup(opTrue, pickTrueRegs[index]);
            joinWithGroup(opFalse, pickFalseRegs[index]);
          }
        }
      } else if (TII->isSwitch(&MI)) {
        // Switches are the most complex case to deal with. Like PICK ops,
        // we can join the control and incoming edge to the same group. However,
        // we also need to assign edge frequency to LICs on the edge between
        // picks and switches (as in a pick-switch loop). To this end, we group
        // the pairs of outgoing edges into the same group as well.
        unsigned opFalse = getVregIndex(MI.getOperand(0));
        unsigned opTrue = getVregIndex(MI.getOperand(1));
        unsigned opCtl = getVregIndex(MI.getOperand(2));
        unsigned opUse = getVregIndex(MI.getOperand(3));
        if (opUse != UNMAPPED_REG && opCtl != UNMAPPED_REG)
          joinWithGroup(opCtl, opUse);

        // Only map the true/false legs if the control operand has a meaningful
        // value.
        if (opCtl != UNMAPPED_REG) {
          auto it = std::find(switchConditions.begin(),
              switchConditions.end(), opCtl);
          if (it == switchConditions.end()) {
            switchConditions.push_back(opCtl);
            switchTrueRegs.push_back(opTrue);
            switchFalseRegs.push_back(opFalse);
          } else {
            auto index = it - switchConditions.begin();
            joinWithGroup(opTrue, switchTrueRegs[index]);
            joinWithGroup(opFalse, switchFalseRegs[index]);
          }
        }
      } else if (MI.getOpcode() == CSA::LAND1 || MI.getOpcode() == CSA::LOR1) {
        // The operation is guaranteed to always query at least the first
        // operand, so the first operand must execute as many times as the
        // result.
        licGrouping.join(getVregIndex(MI.getOperand(0)),
            getVregIndex(MI.getOperand(1)));
      } else if (MI.getOpcode() == CSA::PREDPROP) {
        // PREDPROP: merge the two edge outputs with the input block predicate
        joinVregs(MI, {0, 1, 2});
      } else if (MI.getOpcode() == CSA::PREDMERGE) {
        // PREDMERGE: merge the two edge inputs with the output block predicate
        joinVregs(MI, {0, 2, 3});
      } else {
        LLVM_DEBUG({
          errs() << "Multi-triggered-op: ";
          MI.dump();
        });
      }
    }

    // If we're pre-dataflow conversion, then note the representative vreg for
    // the basic block now.
    if (preDFConversion && joinGroupVreg != UNMAPPED_REG) {
      LLVM_DEBUG(dbgs() << "Assigning " << BB.getName() << " to group " <<
                 joinGroupVreg << "\n");
      basicBlockRegs[BB.getNumber()] = joinGroupVreg;
    }

    // If there are switch outputs, analyze the block to figure out which edge
    // of the switch corresponds to which edge of the basic block. Only apply
    // this to blocks with two outgoing edges.
    if (BB.succ_size() == 2 && !switchConditions.empty()) {
      MachineBasicBlock *TBB = nullptr, *FBB = nullptr;
      SmallVector<MachineOperand, 4> Cond;
      bool analyzed = !TII->analyzeBranch(BB, TBB, FBB, Cond, false);
      (void)analyzed;
      assert((analyzed && TBB && FBB && Cond.size() == 2) &&
          "Could not analyze branch of machine code");
      unsigned branchCondReg = getVregIndex(Cond[1]);
      // Confusingly, we do not set the result of TBB and FBB according to which
      // one is true and which one is false. The MachineCDG code already worked
      // out which edge is true and which edge is false, so use that to
      // determine of the outgoing edges are [T, F] or [F, T].
      bool isTrueFirst =
        CDG->getEdgeType(&BB, *BB.succ_begin()) == ControlDependenceNode::TRUE;

      // Find the condition that corresponds to our outgoing edge. The assertion
      // that it might exist may be too strict, but we should be doing something
      // appropriate when reading switch outputs.
      auto it = std::find(switchConditions.begin(), switchConditions.end(),
          branchCondReg);
      assert(it != switchConditions.end() &&
          "No switches found for output edges.");
      auto index = it - switchConditions.begin();

      // Now we have the index of the switch condition in a lists of conditions.
      // Report which edge is which in the graph.
      auto &switchInfo = switchOuts[BB.getNumber()];
      joinWithGroup((isTrueFirst ? switchTrueRegs : switchFalseRegs)[index],
          switchInfo.first);
      joinWithGroup((isTrueFirst ? switchFalseRegs : switchTrueRegs)[index],
          switchInfo.second);
    }
  }

  // For debug output, print the mapping of blocks and edges to LIC groups that
  // we have found.
  LLVM_DEBUG({
    for (auto &BB : *thisMF) {
      unsigned map = basicBlockRegs[BB.getNumber()];
      if (map == UNMAPPED_REG) {
        dbgs() << "BB#" << BB.getNumber() << " is unmapped\n";
        continue;
      }
      map = licGrouping.findLeader(map);
      dbgs() << "BB#" << BB.getNumber() << " has vregs";
      for (unsigned index = 0; index < MRI->getNumVirtRegs(); index++) {
        auto vreg = Register::index2VirtReg(index);
        if (MRI->use_nodbg_empty(vreg))
          continue;
        if (licGrouping.findLeader(index) == map)
          dbgs() << " " << index;
      }
      dbgs() << "\n";

      if (BB.succ_size() == 2) {
        for (unsigned i = 0; i < 2; i++) {
          auto destBB = *(BB.succ_begin() + i);
          dbgs() << "Edge BB#" << BB.getNumber() << " -> BB#" <<
            destBB->getNumber();
          auto index = BB.getNumber();
          auto map = i == 0 ? switchOuts[index].first : switchOuts[index].second;
          if (map == UNMAPPED_REG) {
            dbgs() << " is unmapped\n";
            continue;
          }
          map = licGrouping.findLeader(map);
          dbgs() << " has vregs";
          for (unsigned index = 0; index < MRI->getNumVirtRegs(); index++) {
            auto vreg = Register::index2VirtReg(index);
            if (MRI->use_nodbg_empty(vreg))
              continue;
            if (licGrouping.findLeader(index) == map)
              dbgs() << " " << index;
          }
          dbgs() << "\n";
        }
      }
    }
  });
}

static void addLoop(MachineLoop *L, SmallVectorImpl<MachineLoop *> &Loops) {
  Loops.push_back(L);
  for (auto SubLoop : *L)
    addLoop(SubLoop, Loops);
}

static void collectLoops(MachineLoopInfo *MLI, SmallVectorImpl<MachineLoop *> &Loops) {
  for (auto L : *MLI)
    addLoop(L, Loops);
}

static unsigned findLoopId(SmallVectorImpl<MachineLoop *> &Loops, MachineLoop* loop) {
  return std::find(Loops.begin(), Loops.end(), loop) - Loops.begin() + 1;
}

unsigned CSACvtCFDFPass::getLoopId(MachineLoop* loop) {
  SmallVector<MachineLoop *, 8> Loops;
  collectLoops(MLI, Loops);
  return findLoopId(Loops, loop);
}

void CSACvtCFDFPass::assignLicFrequencies(MachineBlockFrequencyInfo &MBFI) {
  LLVM_DEBUG(dbgs() <<
             "Propagating block frequency information to LIC groups.\n");

  // We've assigned everything into equivalence classes. Now, we're going to
  // convert the groups into objects that represent those groups, and we're
  // going to propagate branch probability/block frequency information into
  // these classes.
  licGrouping.compress();
  std::vector<std::shared_ptr<CSALicGroup>> groups(licGrouping.getNumClasses());
  typedef ScaledNumber<uint64_t> Scaled64;
  Scaled64 entryFreq(MBFI.getEntryFreq(), 0);

  // Lambda for assigning the frequency to a LIC equivalence class. Note that it
  // will assert if you try to set an inconsistent frequency (this would
  // probably be indicative of a bug in dataflow conversion somewhere).
  auto setGroupInfo = [&](unsigned licClass, bool preciseFreq, BlockFrequency freq, unsigned LoopId, unsigned LoopDepth) {
    auto licGroup = std::make_shared<CSALicGroup>();
    ScaledNumber<uint64_t> newExecFreq = Scaled64(freq.getFrequency(), 0) / entryFreq;

    // Do some sanity checking when possible
    if (groups[licClass]) {
      if (groups[licClass]->execFreqIsPrecise && preciseFreq) {
        assert(groups[licClass]->executionFrequency == newExecFreq);
      }
      if (groups[licClass]->LoopId && LoopId) {
        assert(groups[licClass]->LoopId == LoopId);
        assert(groups[licClass]->LoopDepth == LoopDepth);
      }
    }

    if (!groups[licClass] || (groups[licClass] && !groups[licClass]->execFreqIsPrecise)) {
      // If we didn't have an estimate before or we did and it was not precise,
      // use the new estimate.
      licGroup->executionFrequency = newExecFreq;
    } else {
      // Otherwise, keep the old estimate.
      licGroup->executionFrequency = groups[licClass]->executionFrequency;
    }

    // Inherit Loop info. This intent is that loop information can be added to
    // the group in a second call as long as the first call specified 0
    // ID/depth. Note that we are not changing or losing loop info due to the
    // assertion above.
    if (LoopId) {
      licGroup->LoopId = LoopId;
      licGroup->LoopDepth = LoopDepth;
    }
    groups[licClass] = std::move(licGroup);
  };

  // Get a list of loops in preorder traversal. This helps with mapping to loop
  // ID in lic groups.
  SmallVector<MachineLoop *, 8> Loops;
  collectLoops(MLI, Loops);

  for (auto &BB: *thisMF) {
    auto index = BB.getNumber();
    auto nominalVreg = basicBlockRegs[index];
    auto blockFreq = MBFI.getBlockFreq(&BB);
    unsigned loopDepth = 0;
    unsigned loopId = 0;
    MachineLoop *L = MLI->getLoopFor(&BB);
    if (L) {
      loopDepth = L->getLoopDepth();
      loopId = findLoopId(Loops, L);
    }
    // If there is a class for the basic block, propagate block frequency.
    if (nominalVreg != ~0U) {
      auto groupId = licGrouping[nominalVreg];
      setGroupInfo(groupId, true, blockFreq, 0, 0);
      auto &licGroup = groups[groupId];

      // Propagate loop information too. There's no assertion here that the
      // information is consistent, unlike frequency, since the frequency being
      // wrong should catch it.
      if (L) {
        licGroup->LoopDepth = loopDepth;
        licGroup->LoopId = loopId;
      }
    }

    // Propagate the branch probability to LICs on the edges.
    SmallVector<unsigned, 2> outRegs =
      {switchOuts[index].first, switchOuts[index].second};
    int i = 0;
    for (auto succ = BB.succ_begin(), E = BB.succ_end(); succ != E; ++succ) {
      auto destBB = *succ;
      unsigned vreg = outRegs[i++];
      if (vreg == UNMAPPED_REG)
        continue;

      // Ignore destinations we can assign frequencies to by their own basic
      // blocks.
      unsigned destBBReg = basicBlockRegs[destBB->getNumber()];
      if (destBBReg != UNMAPPED_REG &&
          licGrouping[destBBReg] == licGrouping[vreg])
        continue;

      // If we didn't have a basic block association, then we are unlikely to
      // have had a loop association. Make an effort to recover this. One
      // observation is that edges effecting control flow within the same loop
      // are part of that loop.
      unsigned groupLoopDepth = 0;
      unsigned groupLoopId = 0;
      if (L) {
        MachineLoop *dstLoop = MLI->getLoopFor(destBB);
        if (dstLoop && L == dstLoop) {
          groupLoopDepth = loopDepth;
          groupLoopId = loopId;
        }
      }

      // Block frequency is more accurate than doing the calculation ourself
      // (which may round differently than block frequency calculations).
      // If the target has no other predecessor, its execution count is
      // identical to the edge's count, so use its block frequency instead.
      if (destBB->pred_size() == 1) {
        setGroupInfo(licGrouping[vreg], true, MBFI.getBlockFreq(destBB),
            groupLoopId, groupLoopDepth);
      } else {
        setGroupInfo(licGrouping[vreg], false,
          blockFreq * MBFI.getMBPI()->getEdgeProbability(&BB, succ),
          groupLoopId, groupLoopDepth);
      }
    }
  }

  // Assign all of the groups we generated to the LMFI side table.
  unsigned empty = 0, count = 0;
  LLVM_DEBUG(dbgs() << "Unassigned lics:");
  for (unsigned i = 0, e = MRI->getNumVirtRegs(); i != e; i++) {
    auto vreg = Register::index2VirtReg(i);
    if (MRI->use_nodbg_empty(vreg))
      continue;
    auto group = groups[licGrouping[i]];
    if (group) {
      LMFI->setLICGroup(vreg, group);
    } else {
      LLVM_DEBUG(dbgs() << " " << i);
    }
    count++;
    empty += !group;
  }
  LLVM_DEBUG(dbgs() << "\n" << empty << " of " << count <<
             " LICs were not assigned to groups.\n");

  // We don't need the equivalence classes anymore, since it's all propagated
  // to LMFI. Clear this so it doesn't interfere with the next function in the
  // module.
  licGrouping.clear();
  basicBlockRegs.clear();
  switchOuts.clear();
}
