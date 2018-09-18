//===-- CSACvtCFDFPass.cpp - CSA convert control flow to data flow --------===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file "reexpresses" the code containing traditional control flow
// into a basically data flow representation suitable for the CSA.
//
//===----------------------------------------------------------------------===//

#include "CSACvtCFDFPass.h"
#include "CSASubtarget.h"
#include "CSATargetMachine.h"
#include "CSAUtils.h"
#include "InstPrinter/CSAInstPrinter.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SparseBitVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/Debug.h"
#include <stack>

using namespace llvm;

static cl::opt<int>
  CvtCFDFPass("csa-cvt-cf-df-pass", cl::Hidden, cl::ZeroOrMore,
              cl::desc("CSA Specific: Convert control flow to data flow pass"),
              cl::init(1));

static cl::opt<int> RunSXU("csa-run-sxu", cl::Hidden,
                           cl::desc("CSA Specific: run on sequential unit"),
                           cl::init(0));

static cl::opt<int> UseDynamicPred(
  "csa-dynamic-pred", cl::Hidden,
  cl::desc(
    "CSA Specific: use solely dynamic predicate to generate data flow code"),
  cl::init(0));

static cl::opt<bool> ILPLWaitForAllIncoming(
  "csa-ilpl-all0-incoming", cl::Hidden,
  cl::desc("CSA Specific: ILPL codegen: pick based on all0 of new cohorts"),
  cl::init(true));

static cl::opt<bool> ILPLWaitForAllBack(
  "csa-ilpl-all0-backedges", cl::Hidden,
  cl::desc(
    "CSA Specific: ILPL codegen: pick based on all0 of backedge cohorts"),
  cl::init(false));

#define DEBUG_TYPE "csa-cvt-cf-df-pass"

//  Because of the namespace-related syntax limitations of gcc, we need
//  To hoist init out of namespace blocks.
char CSACvtCFDFPass::ID = 0;
// declare CSACvtCFDFPass Pass
INITIALIZE_PASS_BEGIN(CSACvtCFDFPass, "csa-cvt-cfdf",
                      "CSA Convert Control Flow to Data Flow", true, false)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_DEPENDENCY(ControlDependenceGraph)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_DEPENDENCY(MachinePostDominatorTree)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(CSACvtCFDFPass, "csa-cvt-cfdf",
                    "CSA Convert Control Flow to Data Flow", true, false)

CSACvtCFDFPass::CSACvtCFDFPass() : MachineFunctionPass(ID) {
  initializeCSACvtCFDFPassPass(*PassRegistry::getPassRegistry());
}

MachineFunctionPass *llvm::createCSACvtCFDFPass() {
  return new CSACvtCFDFPass();
}

void CSACvtCFDFPass::releaseMemory() {
  GenSwitches.clear();

  DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *>::iterator
    itmp = bb2pick.begin();
  while (itmp != bb2pick.end()) {
    DenseMap<unsigned, MachineInstr *> *reg2pick = itmp->getSecond();
    ++itmp;
    delete reg2pick;
  }
  bb2pick.clear();

  DenseMap<MachineBasicBlock *, SmallVectorImpl<unsigned> *>::iterator itedge =
    edgepreds.begin();
  while (itedge != edgepreds.end()) {
    SmallVectorImpl<unsigned> *edges = itedge->getSecond();
    ++itedge;
    delete edges;
  }
  edgepreds.clear();
  bb2predcpy.clear();
  bb2predmerge.clear();
  bbpreds.clear();
  bb2pick.clear();
  bb2rpo.clear();
  dcgBBs.clear();
  multiInputsPick.clear();
  loopInfo.clear();
}

void CSACvtCFDFPass::replacePhiWithPICK() {
  replaceLoopHdrPhi();
  replaceIfFooterPhiSeq();
}


bool CSACvtCFDFPass::runOnMachineFunction(MachineFunction &MF) {

  if (CvtCFDFPass == 0)
    return false;
  thisMF = &MF;

  TII = static_cast<const CSAInstrInfo *>(
    thisMF->getSubtarget<CSASubtarget>().getInstrInfo());
  MRI  = &thisMF->getRegInfo();
  TRI  = thisMF->getSubtarget<CSASubtarget>().getRegisterInfo();
  LMFI = thisMF->getInfo<CSAMachineFunctionInfo>();

  DT  = &getAnalysis<MachineDominatorTree>();
  PDT = &getAnalysis<MachinePostDominatorTree>();
  if (PDT->getRootNode() == nullptr)
    return false;
  else {
    //
    // CMPLRS-48822: workaround for infinite loops.
    //
    // Look for children of the PDT's root node and bail
    // out if any of them is not a normal exit (i.e. have
    // a CF successor).
    //
    bool normalExitBlockSeen = false;
    for (auto &child : children<MachineDomTreeNode *>(PDT->getRootNode())) {
      auto *MBB = child->getBlock();
      if (MBB->succ_empty()) {
        // Bail out on functions with multiple exits.
        //
        // TODO (vzakhari 2/21/2018): figure out, why this is needed.
        if (normalExitBlockSeen) {
          if (csa_utils::isAlwaysDataFlowLinkageSet())
            report_fatal_error("Function with multiple exits!!\n");
          else
            return false;
        }

        normalExitBlockSeen = true;
        continue;
      }
      else {
        if (csa_utils::isAlwaysDataFlowLinkageSet())
          report_fatal_error("Function with multiple exits!!\n");
        else
          return false;
      }
    }
  }
  CDG = &getAnalysis<ControlDependenceGraph>();
  MLI = &getAnalysis<MachineLoopInfo>();

  // Give up and run on SXU if there is dynamic stack activity we don't handle.
  if (thisMF->getFrameInfo().hasVarSizedObjects()) {
    errs() << "WARNING: dataflow conversion not attempting to handle dynamic "
              "stack allocation.\n";
    errs() << "Function \"" << thisMF->getName() << "\" will run on the SXU.\n";
    if (csa_utils::isAlwaysDataFlowLinkageSet())
      report_fatal_error("Compilation terminated!!\n");
    else
      return false;
  }

  // Move reads of index references, which turn into invariant uses of
  // implicitly live-in registers, into the function entry. This is done so
  // that dataflow conversion recognizes these non-loop value defs as ones
  // which need to be flowed in.
  createFIEntryDefs();

  replaceUndefWithIgn();
  handleAllConstantInputs();

  // Before going any further, set up a baseline information about LIC grouping.
  // This ensures that there will be no confusion resulting from code being
  // generated to control dataflow. Note that this code needs to run after
  // moving the constant MOV entries to the entry block to avoid issues with
  // constants in PHIs.
  licGrouping.grow(MRI->getNumVirtRegs());
  basicBlockRegs.resize(thisMF->size(), ~0U);
  switchOuts.resize(thisMF->size(), EdgeRegs(~0U, ~0U));
  findLICGroups(true);

  ReversePostOrderTraversal<MachineFunction *> RPOTStack(thisMF);
  RPOT = &RPOTStack;
  unsigned i = 0;
  for (MachineBasicBlock *MBB : *RPOT) {
    bb2rpo[MBB] = i++;
  }

  // Insert switches.
  switchNormalRegisters();

  // At this point, the code should obey the property that the define and uses
  // of all registers are in the same control-dependent region. We don't assert
  // that at this point, although there is effectively an assert of this
  // property in assignLicFrequencies later on.

  LLVM_DEBUG(dbgs() << "Function after switch generation:\n"; MF.print(dbgs()));

  // We are now exiting SSA mode. At the moment, we can't actually tell MRI that
  // we are doing so, since we need to remain in SSA mode for register
  // allocation.

  if (needDynamicPreds() || UseDynamicPred) {
    generateDynamicPreds();
  } else {
    replacePhiWithPICK();
  }

  // Convert LIC classes from I* to CI* and add NonSequential flag where
  // possible.
  assignLicForDF();

  LLVM_DEBUG(dbgs() << "Function after pick insertion:\n"; MF.print(dbgs()));

  // Recompute LIC groups for newly-added dataflow instructions. This time, it's
  // not safe to assume that instructions that get added necessarily correspond
  // to the execution counts of the blocks, but we can still do equivalence
  // class propagations that's accurate.
  findLICGroups(false);
  assignLicFrequencies(getAnalysis<MachineBlockFrequencyInfo>());

  if (!RunSXU) {
    removeBranch();
    linearizeCFG();
  }
  // destroy all the data structures for current function,
  // after branches are removed, BB pointer are no longer valid
  releaseMemory();
  RPOT = nullptr;

  RunSXU = false;

  return true;
}

static unsigned getLoopExitNumber(MachineLoop *Loop,
    MachineBasicBlock *ExitingBlock) {
  SmallVector<MachineBasicBlock *, 4> LoopExits;
  Loop->getExitingBlocks(LoopExits);
  return std::find(LoopExits.begin(), LoopExits.end(), ExitingBlock) -
    LoopExits.begin();
}

MachineInstr *CSACvtCFDFPass::insertSWITCHForReg(unsigned Reg,
                                                 MachineBasicBlock *cdgpBB) {
  // generate and insert SWITCH or copy
  const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
  MachineInstr *result           = nullptr;
  if (cdgpBB->succ_size() > 1) {
    MachineBasicBlock::iterator loc = cdgpBB->getFirstTerminator();
    MachineInstr *bi                = &*loc;
    unsigned switchFalseReg         = MRI->createVirtualRegister(TRC);
    unsigned switchTrueReg          = MRI->createVirtualRegister(TRC);
    nameLIC(switchTrueReg, "", Reg, ".switch.", cdgpBB, ".true");
    nameLIC(switchFalseReg, "", Reg, ".switch.", cdgpBB, ".false");
    assert(bi->getOperand(0).isReg());
    // generate switch op
    const unsigned switchOpcode = TII->makeOpcode(CSA::Generic::SWITCH, TRC);
    MachineInstr *switchInst;
    switchInst = BuildMI(*cdgpBB, loc, bi->getDebugLoc(),
        TII->get(switchOpcode), switchFalseReg)
        .addReg(switchTrueReg, RegState::Define)
        .addReg(bi->getOperand(0).getReg())
        .addReg(Reg);

    switchInst->setFlag(MachineInstr::NonSequential);
    result = switchInst;
  } else {
    assert(false && "Should not try to insert a switch for a block with only "
        "one successor");
  }

  // Note the switch as exiting in all blocks it's exiting in.
  MachineLoop *Loop = MLI->getLoopFor(cdgpBB);
  for (; Loop && Loop->isLoopExiting(cdgpBB); Loop = Loop->getParentLoop()) {
    loopInfo[Loop].addExitSwitch(getLoopExitNumber(Loop, cdgpBB), result);
  }

  LLVM_DEBUG(dbgs() << "  Inserted switch in BB#" << cdgpBB->getNumber()
      << ": " << *result);
  return result;
}

unsigned CSACvtCFDFPass::findSwitchingDstForReg(unsigned Reg,
                                                MachineBasicBlock *mbb) {
  auto SwitchMapIter = GenSwitches.find(mbb);
  if (SwitchMapIter == GenSwitches.end()) {
    return 0;
  }
  auto RegSwitchIter = SwitchMapIter->second->find(Reg);
  if (RegSwitchIter == SwitchMapIter->second->end()) {
    return 0;
  }
  MachineInstr *defSwitchInstr = RegSwitchIter->second;
  LLVM_DEBUG(dbgs() << *defSwitchInstr << '\n');
  unsigned switchFalseReg      = defSwitchInstr->getOperand(0).getReg();
  unsigned switchTrueReg       = defSwitchInstr->getOperand(1).getReg();
  if (MRI->use_empty(switchFalseReg)) {
    return switchFalseReg;
  } else if (MRI->use_empty(switchTrueReg)) {
    return switchTrueReg;
  }
  return 0;
}

MachineInstr *
CSACvtCFDFPass::getOrInsertSWITCHForReg(unsigned Reg, MachineBasicBlock *MBB) {
  auto SwitchMapIter = GenSwitches.find(MBB);
  if (SwitchMapIter == GenSwitches.end()) {
    SwitchMapIter = GenSwitches.try_emplace(MBB,
        new DenseMap<unsigned, MachineInstr *>()).first;
  }
  auto RegSwitchIter = SwitchMapIter->second->find(Reg);
  if (RegSwitchIter == SwitchMapIter->second->end()) {
    RegSwitchIter = SwitchMapIter->second->try_emplace(Reg,
      insertSWITCHForReg(Reg, MBB)).first;
  }
  return RegSwitchIter->second;
}

unsigned CSACvtCFDFPass::getSwitchIndexForEdge(MachineBasicBlock *Parent,
    MachineBasicBlock *Child) {
  switch (CDG->getEdgeType(Parent, Child)) {
  case ControlDependenceNode::TRUE:
    return 1;
  case ControlDependenceNode::FALSE:
    return 0;
  default:
    assert(false && "Unknown edge type for basic blocks");
    return 0;
  }
}

void CSACvtCFDFPass::prefillLoopInfo(MachineLoop *L) {
  for (auto Subloop : *L)
    prefillLoopInfo(Subloop);

  // Allocate the datastructure.
  CSALoopInfo &DFLoop = loopInfo[L];

  // Fill in the index for the exiting edges.
  SmallVector<MachineBasicBlock *, 2> ExitingBlocks;
  L->getExitingBlocks(ExitingBlocks);
  for (auto ExitingBlock : ExitingBlocks) {
    // Get the target exit block.
    MachineBasicBlock *ExitBlock = nullptr;
    for (auto Succ : ExitingBlock->successors())
      if (!L->contains(Succ)) {
        ExitBlock = Succ;
        break;
      }

    DFLoop.addExit(1 - getSwitchIndexForEdge(ExitingBlock, ExitBlock));
  }
}

void CSACvtCFDFPass::switchNormalRegisters() {
  // In the future, when we start having dataflow LICs that exist in the IR,
  // we are going to need to be more selective about which registers we switch.
  // For now, though, switch all of them.

  // Prefill the dataflow loop info with information about loops.
  for (auto L : *MLI) {
    prefillLoopInfo(L);
  }

  // It's important that we save the number of registers to switch before
  // switching any of them--switching creates new registers that we don't want
  // to switch.
  for (unsigned i = 0, e = MRI->getNumVirtRegs(); i != e; i++) {
    switchRegister(TargetRegisterInfo::index2VirtReg(i), false);
  }
}

void CSACvtCFDFPass::switchRegister(unsigned Reg, bool StrictLive) {
  // Do a live variables analysis for the given register. This is modified from
  // the regular LLVM live variable analysis in a few ways:
  //
  // 1. We don't keep track of kills.
  // 2. We modify the notion of alive to refer to liveness at the beginning of
  //    a basic block rather than through the entire basic block.
  // 3. Liveness tracking terminates at any node in the same control-dependent
  //    region as the define (if StrictLive is false).
  //
  // With these modifications, we need to generate switch statements precisely
  // where the successor block is marked as alive.

  MachineInstr *Def = MRI->getVRegDef(Reg);
  if (!Def)
    return;
  MachineBasicBlock *DefBB = Def->getParent();

  // Compute the canonical blocks for every node.
  std::vector<MachineBasicBlock *> CanonicalMap{thisMF->size(), nullptr};
  if (StrictLive) {
    for (auto &Node : *thisMF)
      CanonicalMap[Node.getNumber()] = &Node;
  } else {
    for (auto &Region : CDG->getRegions()) {
      MachineBasicBlock *Head = nullptr;
      for (auto Node : Region->nodes) {
        // Every node dominates all of its following nodes in the region list,
        // so when we find the first node in the region that is dominated by the
        // definition, then that node will be the canonical for every other node
        // in the region.
        if (!Head && DT->dominates(DefBB, Node))
          Head = Node;
        if (Head)
          CanonicalMap[Node->getNumber()] = Head;
      }
    }
  }

  SparseBitVector<> AliveBlocks;
  std::vector<MachineBasicBlock *> WorkList;
  bool HasPHI = false;

  for (auto &UseInstr : MRI->use_instructions(Reg)) {
    MachineBasicBlock *UseBB = UseInstr.getParent();

    // PHIs: live in their actual block, but trace their parent blocks instead.
    if (UseInstr.isPHI()) {
      for (unsigned i = 1, e = UseInstr.getNumOperands(); i != e; i += 2) {
        if (UseInstr.getOperand(i).getReg() == Reg)
          WorkList.push_back(UseInstr.getOperand(i + 1).getMBB());
      }
      HasPHI = true;
      continue;
    }

    // Ignore uses in the same block.
    if (UseBB == DefBB) {
      continue;
    }
    WorkList.push_back(UseBB);
  }

  // Mark the value as live in the given block. Recurse through predecessors
  // until we reach the definition block.
  while (!WorkList.empty()) {
    MachineBasicBlock *LiveBB = CanonicalMap[WorkList.back()->getNumber()];
    WorkList.pop_back();

    // Is this a define block? If so, stop recursion.
    if (LiveBB == DefBB)
      continue;

    // Do we already know that it's live? If so, stop.
    unsigned BBNum = LiveBB->getNumber();
    if (AliveBlocks.test(BBNum))
      continue;

    AliveBlocks.set(BBNum); // Live.

    // Go through our predecessors to mark it live there.
    assert(LiveBB != &thisMF->front() && "Can't find reaching def for virtreg");
    WorkList.insert(WorkList.end(), LiveBB->pred_rbegin(), LiveBB->pred_rend());
  }

  // Is there anything to do?
  if (!HasPHI && AliveBlocks.empty())
    return;

  LLVM_DEBUG(dbgs() << "Inserting switches for " << printReg(Reg) << "\n");

  // At this point, we've collected where the blocks are live. Now go through
  // and insert switches across all of those basic blocks. This will fill a list
  // mapping blocks to the proper version (switched or via PHI) in that block.
  const TargetRegisterClass *RegClass = MRI->getRegClass(Reg);
  std::vector<unsigned> RegValues(thisMF->size());
  std::vector<MachineInstr *> PHIs(thisMF->size());

  auto getBlockNumber = [&](MachineBasicBlock *MBB) {
    return CanonicalMap[MBB->getNumber()]->getNumber();
  };

  // Start the register with the initial value.
  RegValues[DefBB->getNumber()] = Reg;

  // Get the value that's live on the edge. If the parent has two children, it
  // needs to insert a switch; otherwise, it copies the parent's value.
  auto getRegOnEdge = [&](MachineBasicBlock *Parent, MachineBasicBlock *Child) {
    if (Parent->succ_size() == 1)
      return RegValues[getBlockNumber(Parent)];

    // Insert a switch in the parent and return the appropriate output edge.
    MachineInstr *NewSwitch = getOrInsertSWITCHForReg(Reg, Parent);
    return NewSwitch->getOperand(getSwitchIndexForEdge(Parent, Child)).getReg();
  };

  // Iterate through the graph in RPO, adding switches to the parent blocks or
  // PHIs as necessary. PHIs will be filled in via a second pass, to handle
  // cycles correctly.
  for (auto MBB : *RPOT) {
    unsigned BBNo = MBB->getNumber();
    // Skip blocks that are not live.
    if (!AliveBlocks.test(BBNo))
      continue;
    // Add the switch.
    if (MBB->pred_size() == 1) {
      RegValues[BBNo] = getRegOnEdge(*MBB->pred_begin(), MBB);
    } else {
      assert(!PHIs[BBNo] && "Trying to build too many phis");
      unsigned NewReg = MRI->createVirtualRegister(RegClass);
      auto NewPhi = BuildMI(*MBB, MBB->getFirstNonPHI(),
        MBB->front().getDebugLoc(), TII->get(TargetOpcode::PHI), NewReg);
      PHIs[BBNo] = NewPhi;
      RegValues[BBNo] = NewReg;
    }
  }

  // Now go through and fill in values for all of the PHI nodes.
  for (unsigned BBNo : AliveBlocks) {
    MachineBasicBlock *MBB = CanonicalMap[BBNo];
    if (MBB->pred_size() > 1) {
      auto PHI = PHIs[BBNo];
      assert(PHI && "How did we not insert a PHI yet?");
      for (auto Pred : MBB->predecessors()) {
        unsigned NewReg = getRegOnEdge(Pred, MBB);
        PHI->addOperand(MachineOperand::CreateReg(NewReg, false));
        PHI->addOperand(MachineOperand::CreateMBB(Pred));
      }
      LLVM_DEBUG(dbgs() << "  Inserted PHI in BB#" << BBNo << ": " << *PHI);
    }
  }

  // Rewrite all uses of the original register.
  for (auto I = MRI->use_begin(Reg), E = MRI->use_end(); I != E; ) {
    MachineOperand &MO = *I;
    ++I;
    MachineBasicBlock *MBB = MO.getParent()->getParent();
    if (MO.getParent()->isPHI()) {
      MachineBasicBlock *UseBB = (&MO + 1)->getMBB();
      MO.setReg(getRegOnEdge(UseBB, MBB));
    } else {
      MO.setReg(RegValues[getBlockNumber(MBB)]);
    }
  }
}

void CSACvtCFDFPass::replaceLoopHdrPhi() {
  for (MachineLoopInfo::iterator LI = MLI->begin(), LE = MLI->end(); LI != LE;
       ++LI) {
    replaceLoopHdrPhi(*LI);
  }
}

void CSACvtCFDFPass::replaceLoopHdrPhi(MachineLoop *L) {
  for (MachineLoop::iterator LI = L->begin(), LE = L->end(); LI != LE; ++LI) {
    replaceLoopHdrPhi(*LI);
  }
  MachineLoop *mloop      = L;
  MachineBasicBlock *lhdr = mloop->getHeader();

  unsigned pipeliningDegree = getInnerLoopPipeliningDegree(L);
  if (pipeliningDegree > 1) {
    replaceCanonicalLoopHdrPhiPipelined(lhdr, pipeliningDegree);
  } else {
    replaceCanonicalLoopHdrPhi(lhdr);
  }
}

// sequence OPT is targeting at this transform
// single entry, single exiting, single latch, exiting blk post dominates loop
// hdr(always execute)
void CSACvtCFDFPass::replaceCanonicalLoopHdrPhi(MachineBasicBlock *mbb) {
  MachineLoop *mloop = MLI->getLoopFor(mbb);
  assert(mloop->getHeader() == mbb);

  assert(mloop->getExitingBlock() &&
         "can't handle multi exiting blks in this funciton");
  MachineBasicBlock *headerBB = mloop->getHeader();
  MachineBasicBlock *latchBB         = mloop->getLoopLatch();
  ControlDependenceNode *latchNode   = CDG->getNode(latchBB);
  MachineBasicBlock *exitingBB       = mloop->getExitingBlock();
  ControlDependenceNode *exitingNode = CDG->getNode(exitingBB);
  MachineBasicBlock *exitBB          = mloop->getExitBlock();
  assert(latchBB);
  bool needCmpExitCond = false;
  //assert(latchBB && exitingBB && (latchBB == exitingBB || headerBB == exitingBB));
  if (!exitingBB) {
    needCmpExitCond = true;
  } else if (latchBB != exitingBB && headerBB != exitingBB) {
    ControlDependenceNode *exitingNode = CDG->getNode(exitingBB);
    for (ControlDependenceNode::node_iterator pnode = exitingNode->parent_begin(),
      pend = exitingNode->parent_end();
      pnode != pend; ++pnode) {
      ControlDependenceNode *ctrlNode = *pnode;
      MachineBasicBlock *ctrlBB = ctrlNode->getBlock();
      if (MLI->getLoopFor(ctrlBB) == mloop) {
        needCmpExitCond = true;
        break;
      }
    }
  }

  MachineInstr *bi                = &*exitingBB->getFirstInstrTerminator();
  MachineBasicBlock::iterator loc = exitingBB->getFirstTerminator();
  unsigned predReg                = bi->getOperand(0).getReg();
  if (needCmpExitCond) {
    predReg = findLoopExitCondition(mloop);
  }
  const TargetRegisterClass *TRC = MRI->getRegClass(predReg);
  CSAMachineFunctionInfo *LMFI   = thisMF->getInfo<CSAMachineFunctionInfo>();
  // Look up target register class corresponding to this register.
  const TargetRegisterClass *new_LIC_RC =
    LMFI->licRCFromGenRC(MRI->getRegClass(predReg));
  assert(new_LIC_RC && "Can't determine register class for register");
  unsigned cpyReg =
    LMFI->allocateLIC(new_LIC_RC, Twine("loop_") + mbb->getName() + "_phi");
  assert(mloop->isLoopExiting(latchBB) ||
         headerBB == exitingBB         ||
         latchNode->isParent(exitingNode));
  _unused(latchNode);
  _unused(exitingNode);
  _unused(headerBB);
  const unsigned moveOpcode = TII->getMoveOpcode(TRC);
  MachineInstr *cpyInst =
    BuildMI(*exitingBB, loc, DebugLoc(), TII->get(moveOpcode), cpyReg)
      .addReg(predReg);
  cpyInst->setFlag(MachineInstr::NonSequential);

  MachineBasicBlock *lphdr           = mloop->getHeader();
  MachineBasicBlock::iterator hdrloc = lphdr->begin();
  const unsigned InitOpcode          = TII->getInitOpcode(TRC);
  MachineInstr *initInst             = nullptr;
  bool pickCtrlInverted = needCmpExitCond ? 1 :
    CDG->getEdgeType(exitingBB, exitBB, true) != ControlDependenceNode::FALSE;
  if (pickCtrlInverted) {
    initInst = BuildMI(*lphdr, hdrloc, DebugLoc(), TII->get(InitOpcode), cpyReg)
                 .addImm(1);
  } else {
    initInst = BuildMI(*lphdr, hdrloc, DebugLoc(), TII->get(InitOpcode), cpyReg)
                 .addImm(0);
  }
  initInst->setFlag(MachineInstr::NonSequential);

  // Set up the loop info for the current loop.
  CSALoopInfo &DFLoop = loopInfo[mloop];
  DFLoop.setPickBackedgeIndex(pickCtrlInverted ? 0 : 1);

  MachineBasicBlock::iterator iterI = mbb->begin();
  while (iterI != mbb->end()) {
    MachineInstr *MI = &*iterI;
    ++iterI;
    if (!MI->isPHI())
      continue;

    unsigned numUse               = 0;
    MachineOperand *backEdgeInput = nullptr;
    MachineOperand *initInput     = nullptr;
    unsigned numOpnd              = 0;
    unsigned backEdgeIndex        = 0;
    unsigned dst                  = MI->getOperand(0).getReg();

    for (MIOperands MO(*MI); MO.isValid(); ++MO, ++numOpnd) {
      if (!MO->isReg())
        continue;
      // process use at loop level
      if (MO->isUse()) {
        ++numUse;
        MachineOperand &mOpnd = *MO;
        ++MO;
        ++numOpnd;
        MachineBasicBlock *inBB = MO->getMBB();
        if (inBB == latchBB) {
          backEdgeInput = &mOpnd;
          backEdgeIndex = numOpnd - 1;
        } else {
          initInput = &mOpnd;
        }
      }
    } // end for MO
    if (numUse > 2) {
      // loop hdr phi has more than 2 init inputs,
      // remove backedge input reduce it to if-foot phi case to be handled by
      // if-footer phi pass
      initInput = &MI->getOperand(0);
      const TargetRegisterClass *TRC =
        MRI->getRegClass(MI->getOperand(0).getReg());
      unsigned renameReg = MRI->createVirtualRegister(TRC);
      initInput->setReg(renameReg);
    }
    MachineOperand *pickFalse;
    MachineOperand *pickTrue;
    if (pickCtrlInverted) {
      pickFalse = backEdgeInput;
      pickTrue  = initInput;
    } else {
      pickFalse = initInput;
      pickTrue  = backEdgeInput;
    }
    TRC                       = MRI->getRegClass(dst);
    const unsigned pickOpcode = TII->makeOpcode(CSA::Generic::PICK, TRC);
    // generate PICK, and insert before MI
    MachineInstr *pickInst = nullptr;
    predReg                = cpyReg;
    if (pickFalse->isReg() && pickTrue->isReg()) {
      pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst)
                   .addReg(predReg)
                   .addReg(pickFalse->getReg())
                   .addReg(pickTrue->getReg());
    } else if (pickFalse->isReg()) {
      pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst)
                   .addReg(predReg)
                   .addReg(pickFalse->getReg())
                   .add(*pickTrue);
    } else if (pickTrue->isReg()) {
      pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst)
                   .addReg(predReg)
                   .add(*pickFalse)
                   .addReg(pickTrue->getReg());
    } else {
      pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst)
                   .addReg(predReg)
                   .add(*pickFalse)
                   .add(*pickTrue);
    }

    DFLoop.addHeaderPick(pickInst);
    pickInst->setFlag(MachineInstr::NonSequential);
    MI->removeFromParent();
    if (numUse > 2) {
      // move phi before the pick
      MachineBasicBlock::iterator tmpI = pickInst;
      mbb->insert(tmpI, MI);
      MI->RemoveOperand(backEdgeIndex);
      MI->RemoveOperand(backEdgeIndex);
    }
  }
}

// This version uses additional operators in order to allow multiple incoming
// "gangs" of data to flow through the loop at once. The number of gangs
// allowed to be in the pipeline at once is determined by the "completion"
// buffer operators: no new gangs will be admitted if these do not have
// available storage to reorder the loop's outputs. The number of "gangs"
// admitted is bounded on two sides: it is not correct to admit more than we
// have backedge and completion buffering for, and it does not increase
// performance to admit more than the pipeline depth of the body would fit.
void CSACvtCFDFPass::replaceCanonicalLoopHdrPhiPipelined(MachineBasicBlock *mbb,
                                                         unsigned numTokensSpecified) {
  MachineLoop *mloop = MLI->getLoopFor(mbb);
  MachineBasicBlock *lphdr = mloop->getHeader();
  replaceCanonicalLoopHdrPhi(lphdr);
  assert(numTokensSpecified >= 1);

  // The completionN operators we will insert are limited to a maximum depth of
  // 2**8-1==255 by the VISA.
  unsigned numTokens = std::min(255U, numTokensSpecified);

  // ...and they're further limited to a maximum depth of 32 according to V1
  // expectations, which is what the simulator is currently intepreting
  // compiler output as.
  numTokens = std::min(32U, numTokens);

  CSALoopInfo &DFLoop = loopInfo[mloop];
  assert(DFLoop.getNumExits() == 1 &&
    "Can only pipeline loops with single exit blocks");
  pipelineLoop(lphdr, DFLoop, numTokens);
}

void CSACvtCFDFPass::pipelineLoop(MachineBasicBlock *lphdr, CSALoopInfo &DFLoop,
    unsigned numTokens) {
  DebugLoc mloopLoc = MLI->getLoopFor(lphdr)->getStartLoc();

  SmallVector<MachineOperand *, 4> newGang, backGang;

  // Convert the 0/1 indexes of the pick to operand numbers of the pick
  // instruction itself.
  unsigned pickIncomingIdx = DFLoop.getPickInitialIndex() + 2;
  unsigned pickBackedgeIdx = DFLoop.getPickBackedgeIndex() + 2;
  unsigned switchBackedgeIdx = DFLoop.getSwitchBackedgeIndex(0);
  for (MachineInstr *pick : DFLoop.getHeaderPicks()) {
    newGang.push_back(&pick->getOperand(pickIncomingIdx));
    backGang.push_back(&pick->getOperand(pickBackedgeIdx));
  }

  assert(!newGang.empty() && !backGang.empty() &&
    "We have a loop with nothing flowing around the loop?");

  MachineOperand *newPulse  = newGang[0];
  MachineOperand *backPulse = backGang[0];

  if (ILPLWaitForAllIncoming) {
    newPulse = createUseTree(lphdr, lphdr->begin(), CSA::ALL0, newGang);
    LMFI->setLICName(newPulse->getReg(), "newAll");
  }

  if (ILPLWaitForAllBack) {
    backPulse = createUseTree(lphdr, lphdr->begin(), CSA::ALL0, backGang);
    LMFI->setLICName(backPulse->getReg(), "backAll");
  }

  // Look for loop outputs.
  SmallVector<MachineOperand *, 4> loopOutputs;
  unsigned OutgoingIdx = DFLoop.getSwitchLastIndex(0);
  for (MachineInstr *ExitSwitch : DFLoop.getExitSwitches(0)) {
    MachineOperand *LoopOutput = &ExitSwitch->getOperand(OutgoingIdx);
    if (LoopOutput->getReg() == CSA::IGN)
      continue;
    if (MRI->use_nodbg_empty(LoopOutput->getReg()))
      continue;

    loopOutputs.push_back(LoopOutput);
  }

  // If there were no loop outputs, conceptually create a null operand which
  // will get a trivially-used completion1 buffer to limit new cohorts with.
  if (loopOutputs.size() == 0) {
    loopOutputs.push_back(nullptr);
  }

  // For each output, create and hook up completion buffers.
  // The completion buffers' indices (indicating space available) are used to
  // control admission into the pipeline.
  SmallVector<MachineOperand*, 4> newTokens;
  unsigned cpyReg = (*DFLoop.getHeaderPicks().begin())->getOperand(1).getReg();
  unsigned predReg =
    (*DFLoop.getExitSwitches(0).begin())->getOperand(2).getReg();
  for (MachineOperand *g : loopOutputs) {
    unsigned newToken = LMFI->allocateLIC(&CSA::CI8RegClass, "newToken");
    unsigned bodyToken = LMFI->allocateLIC(&CSA::CI8RegClass, "bodyToken");
    unsigned backToken = LMFI->allocateLIC(&CSA::CI8RegClass, "backToken");
    unsigned outToken = LMFI->allocateLIC(&CSA::CI8RegClass, "outToken");

    // If g is null, then we're just flowing around indices with no
    // corresponding data. The data in/out for the completion op will both be
    // IGN. TODO: don't waste a "completion1" on this.
    // Otherwise (if g is a real operand) it needs to be reordered.
    unsigned orderedOut = g ? g->getReg() : static_cast<unsigned>(CSA::IGN);
    const TargetRegisterClass *RC = g ? MRI->getRegClass(g->getReg()) : &CSA::CI1RegClass;
    unsigned unorderedOut = g ? LMFI->allocateLIC(RC, "unorderedOut") :
                                                           static_cast<unsigned>(CSA::IGN);
    if (g)
      g->setReg(unorderedOut);

    // The index/token edges need buffering.
    LMFI->setLICDepth(newToken, numTokens);
    LMFI->setLICDepth(backToken, numTokens);
    // Advise the simulator not to be concerned if the this has values in it on
    // exit; this is expected.
    LMFI->addLICAttribute(newToken, "csasim_ignore_on_exit");

    // TODO: Do not need to reorder 0-bit channels; we should be able to just
    // do rate-limiting with no reordering/storage once the compiler starts
    // emitting them.
    const TargetRegisterClass *compRC =
      TII->getSizeOfRegisterClass(RC) < 1 ? &CSA::CI1RegClass : RC;

    MachineInstrBuilder compBuffer =
      BuildMI(*lphdr, lphdr->begin(), mloopLoc,
          TII->get(TII->makeOpcode(CSA::Generic::COMPLETION, compRC)))
          .addDef(newToken)
          .addDef(orderedOut)
          .addReg(outToken)
          .addReg(unorderedOut)
          .addImm(numTokens);
    newTokens.push_back(&compBuffer->getOperand(0));

    // Pick and switch the token around the loop.
    MachineInstr *tokPick =
      BuildMI(*lphdr, lphdr->begin(), mloopLoc, TII->get(CSA::PICK8), bodyToken)
          .addReg(cpyReg)
          .addReg(DFLoop.getPickInitialIndex() == 1 ? backToken : newToken)
          .addReg(DFLoop.getPickInitialIndex() == 1 ? newToken : backToken);
    MachineInstr *tokSwitch =
      BuildMI(*lphdr, lphdr->begin(), mloopLoc, TII->get(CSA::SWITCH8))
          .addDef(switchBackedgeIdx == 0 ? backToken : outToken)
          .addDef(switchBackedgeIdx == 0 ? outToken : backToken)
          .addReg(predReg)
          .addReg(bodyToken);
    DFLoop.addHeaderPick(tokPick);
    DFLoop.addExitSwitch(0, tokSwitch);
  }
  MachineOperand *haveTokens;
  if (newTokens.size()) {
    SmallVector<MachineInstr *, 4> useTreeInstrs;
    haveTokens = createUseTree(lphdr, lphdr->begin(), CSA::ALL0, newTokens, &useTreeInstrs);
    for (MachineInstr *newInst : useTreeInstrs)
      LMFI->addLICAttribute(std::begin(newInst->defs())->getReg(), "csasim_ignore_on_exit");
  } else {
    // This can happen, right?
    assert(0 && "ILPL on loops with no outputs not yet supported");
  }

  // Add buffering to backedges.
  for (MachineOperand *g : backGang) {
    assert(g->isReg() && "Unexpected non-LIC backedge in inner loop pipeline");
    LMFI->setLICDepth(g->getReg(), numTokens);
    LMFI->setLICName(g->getReg(), "backEdge");
  }

  // Set up the criteria for accepting new iterations. First, delete the old
  // loop instructions.
  SmallVector<MachineInstr*, 2> oldDefs;
  for (auto &MI : MRI->def_instructions(cpyReg))
    oldDefs.push_back(&MI);
  assert(oldDefs.size() == 2 && "Should have MOV+INIT for loop");
  for (auto MI : oldDefs)
    MI->eraseFromParent();

  // Check to see if we both have sufficient space to execute a new iteration
  // (which comes from haveTokens) and have a new iteration that is ready to
  // execute (newPulse).
  MachineInstrBuilder newGated =
    BuildMI(*lphdr, lphdr->begin(), mloopLoc, TII->get(CSA::GATE0),
            LMFI->allocateLIC(&CSA::CI0RegClass, "newGated"))
      .addReg(haveTokens->getReg())
      .addReg(newPulse->getReg())
      .setMIFlag(MachineInstr::NonSequential);

  // Choose whether or not to accept a new iteration or to continue an earlier
  // one. The ANY here is a priority any--we'll prefer accepting new iterations
  // if we have a choice.
  unsigned firstPrio          = newGated->getOperand(0).getReg();
  unsigned secondPrio         = backPulse->getReg();
  MachineInstrBuilder any = BuildMI(*lphdr, lphdr->begin(), mloopLoc,
      TII->get(CSA::ANY0))
    .addDef(cpyReg)
    .addReg(firstPrio)
    .addReg(secondPrio)
    .addReg(CSA::NA)
    .addReg(CSA::NA)
    .setMIFlag(MachineInstr::NonSequential);

  // If the new iteration is selected when pick index is 1, we need to negate
  // the result of the ANY0 to select the corresponding values on the picks.
  if (DFLoop.getPickInitialIndex() == 1) {
    any->getOperand(0).setReg(
      LMFI->allocateLIC(&CSA::CI1RegClass, "notLoopCtl"));
    BuildMI(*lphdr, lphdr->begin(), mloopLoc, TII->get(CSA::NOT1), cpyReg)
      .addReg(any->getOperand(0).getReg())
      .setMIFlag(MachineInstr::NonSequential);
  }
}

// Utility function to create a tree of uses. For example, it can create
// ANY/ALL/MERGE trees from a collection of operands. The "unusedReg" value
// defaults to IGN.
MachineOperand *CSACvtCFDFPass::createUseTree(
    MachineBasicBlock *mbb, MachineBasicBlock::iterator before,
    unsigned opcode, const SmallVector<MachineOperand *, 4> vals,
    SmallVector<MachineInstr *, 4> *created, unsigned unusedReg) {

  unsigned n = vals.size();
  assert(n && "Can't combine 0 values");
  MCInstrDesc id = TII->get(opcode);
  // Ensure that this is an op that we can build some kind of 1-to-N tree out
  // of.
  assert(id.getNumDefs() == 1 && id.getNumOperands() > 2 &&
         "Don't know how to build a tree out of this opcode");
  unsigned radix                    = id.getNumOperands() - id.getNumDefs();
  const TargetRegisterClass *outTRC = LMFI->licRCFromGenRC(
    TII->getRegClass(id, 0, TRI, *mbb->getParent()));

  // If one left, we're done.
  if (n == 1)
    return vals[0];

  // Make our own set of vals to track.
  SmallVector<MachineOperand *, 4> fewerVals;

  // Create a new element to combine some of the others.
  MachineInstrBuilder next =
    BuildMI(*mbb, before, before->getDebugLoc(),
            TII->get(opcode), LMFI->allocateLIC(outTRC));
  next.setMIFlag(MachineInstr::NonSequential);

  // vals will be partitioned into those items being combined and those items
  // which remain.
  for (unsigned j = 0; j < n; ++j) {
    if (j < radix)
      next.addReg(vals[j]->getReg());
    else
      fewerVals.push_back(vals[j]);
  }

  // Ensure the new instruction has enough ops
  while (next->getNumOperands() < radix + 1)
    next.addReg(CSA::IGN);

  // Add the new item to be combined. Putting this at the back of the list
  // helps balance.
  fewerVals.push_back(&next->getOperand(0));

  // Note new instruction for caller if requested.
  if (created)
    created->push_back(next);

  // Run again on the smaller vector.
  return createUseTree(mbb, before, opcode, fewerVals, created, unusedReg);
}

/* Do a sweep over all instructions, looking for direct frame index uses. The
 * use will be replaced with a vreg defined in the entry of the function so
 * that they will be recognized as defs going into any dataflow. This is only
 * intended to handle fixed-size frame objects. The result should still be
 * suitable for SXU execution. */
void CSACvtCFDFPass::createFIEntryDefs() {
  // Build a set of FI users which should be moved to the function entry.
  std::set<MachineOperand *> toReplace;
  // The set of FIs which need vregs created for them in this function.
  std::set<int> entryFIs;
  // A map of FIs to their corresponding virtual registers in the function's
  // entry BB.
  std::map<int, unsigned> fiVRegMap;

  // Collect the FI users which need to be modified.
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end();
       BB != E; ++BB) {
    ControlDependenceNode *mNode = CDG->getNode(&*BB);
    bool hasCDGParent =
      (mNode->getNumParents() > 1 ||
       (mNode->getNumParents() == 1 && (*mNode->parent_begin())->getBlock()));

    // Only worry about uses which aren't already in control flow entry.
    if (!hasCDGParent)
      continue;

    for (MachineBasicBlock::iterator MI = BB->begin(), EI = BB->end(); MI != EI;
         ++MI) {
      for (MIOperands MO(*MI); MO.isValid(); ++MO) {
        if (MO->isFI()) {
          int index = MO->getIndex();
          toReplace.insert(&*MO);
          entryFIs.insert(index);
        }
      }
    }
  }

  // If no interesting uses were found, then nothing needs to be done.
  if (toReplace.empty())
    return;

  // Find the entry block. (Surely there's an easier way to do this?)
  ControlDependenceNode *rootN = CDG->getRoot();
  assert(rootN && *rootN->begin());
  MachineBasicBlock *rootBB = (*rootN->begin())->getBlock();
  assert(rootBB);

  // Def a virtual register for this FI index in the function entry.
  for (int index : entryFIs) {
    unsigned vReg = MRI->createVirtualRegister(&CSA::I64RegClass);
    BuildMI(*rootBB, rootBB->getFirstInstrTerminator(), DebugLoc(),
            TII->get(CSA::MOV64), vReg)
      .addFrameIndex(index);

    fiVRegMap[index] = vReg;
  }

  // Try to eliminate the in-loop def, in case it's now just a redundant mov of
  // the entry def. If the use of the value wasn't as simple as a mov, then
  // just replace the operand.
  for (MachineOperand *mo : toReplace) {
    int index = mo->getIndex();
    assert(fiVRegMap.find(index) != fiVRegMap.end());
    unsigned vReg         = fiVRegMap[index];
    MachineInstr *oldInst = mo->getParent();

    if (oldInst->getOpcode() == CSA::MOV64 && oldInst->getOperand(0).isReg() &&
        TargetRegisterInfo::isVirtualRegister(
          oldInst->getOperand(0).getReg())) {
      // If the value we're replacing is just being moved into another virtual
      // register, then replace the whole instruction.
      unsigned oldVReg = oldInst->getOperand(0).getReg();
      MRI->replaceRegWith(oldVReg, vReg);
      oldInst->eraseFromParent();
    } else {
      // Replace the FI operand with the new vReg.
      mo->ChangeToRegister(vReg, false);
    }
  }
}

void CSACvtCFDFPass::assignLicForDF() {
  // Mark anything that can run on the dataflow array as being able to do so.
  // This entails remapping the I* classes to CI*, and adding the NonSequential
  // flag where appropriate.
  for (auto &BB : *thisMF) {
    for (auto &MI : BB) {
      // Is the opcode something that must run on the SXU?
      switch (MI.getOpcode()) {
      case CSA::JSR: case CSA::JSRi:
      case CSA::JTR: case CSA::JTRi:
        // If we have a function call, and the -csa-df-calls=0 is not passed in,
        // then we need to prevent the CFG from being destroyed.
        if (!csa_utils::isAlwaysDataFlowLinkageSet())
          RunSXU = true;
      case CSA::JMP: case CSA::RET:
      case CSA::BT: case CSA::BF: case CSA::BR:
      case TargetOpcode::PHI:
        continue;
      }

      // For other instructions, check if any of the registers are SXU-specific.
      bool HasSXUReg = false;
      for (auto &Op : MI.operands()) {
        if (Op.isFI()) {
          HasSXUReg = true;
          break;
        }
        if (!Op.isReg())
          continue;
        unsigned Reg = Op.getReg();
        if (TargetRegisterInfo::isVirtualRegister(Reg)) {
          const TargetRegisterClass *RC = MRI->getRegClass(Reg);
          switch (RC->getID()) {
          case CSA::RI0RegClassID:
          case CSA::RI1RegClassID:
          case CSA::RI8RegClassID:
          case CSA::RI16RegClassID:
          case CSA::RI32RegClassID:
          case CSA::RI64RegClassID:
            HasSXUReg = true;
          }
        } else {
          if (!CSA::ANYCRegClass.contains(Reg))
            HasSXUReg = true;
        }
      }

      if (HasSXUReg)
        continue;

      // Move the instruction to the dataflow array.
      MI.setFlag(MachineInstr::NonSequential);
      for (auto &Op : MI.operands()) {
        if (!Op.isReg())
          continue;
        unsigned Reg = Op.getReg();
        if (TargetRegisterInfo::isPhysicalRegister(Reg))
          continue;
        // In the case of registers without uses, replace them without ignore
        // instead of switching them to an unused LIC.
        if (Op.isDef() && MRI->use_empty(Reg))
          MI.substituteRegister(Reg, CSA::IGN, 0, *TRI);
        else {
          const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
          MRI->setRegClass(Reg,
                     TII->getLicClassForSize(TII->getSizeOfRegisterClass(TRC)));
        }
      }
    }
  }
}

void CSACvtCFDFPass::handleAllConstantInputs() {
  std::deque<unsigned> renameQueue;
  MachineBasicBlock *entry = &*thisMF->begin();
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end();
       BB != E; ++BB) {
    MachineBasicBlock *mbb       = &*BB;
    ControlDependenceNode *unode = CDG->getNode(mbb);
    unsigned domIf               = 0;
    for (ControlDependenceNode::node_iterator uparent = unode->parent_begin(),
                                              uparent_end = unode->parent_end();
         uparent != uparent_end; ++uparent) {
      ControlDependenceNode *upnode = *uparent;
      MachineBasicBlock *upbb       = upnode->getBlock();
      if (!upbb) {
        // this is typical define inside loop, used outside loop on the main
        // execution path
        continue;
      }
      if (bb2rpo[upbb] >= bb2rpo[mbb]) {
        continue;
      }
      domIf++;
    }
    MachineBasicBlock::iterator iterMI = BB->begin();
    while (iterMI != BB->end()) {
      MachineInstr *MI = &*iterMI;
      ++iterMI;
      if (!TII->isMOV(MI) || !hasAllConstantInputs(MI))
        continue;

      if (mbb != entry && !domIf) {
        MI->removeFromParent();
        entry->insertAfter(entry->begin(), MI);
      }
      // Replace the MOV with a GATE on the in memory edge.
      if (MI->getParent() == entry) {
        BuildMI(*entry, MI, MI->getDebugLoc(),
            TII->get(TII->adjustOpcode(MI->getOpcode(), CSA::Generic::GATE)),
            MI->getOperand(0).getReg())
          .addReg(LMFI->getInMemoryLic())
          .add(MI->getOperand(1))
          .setMIFlag(MachineInstr::NonSequential);
        MI->eraseFromParent();
      }
    }
  }
}

bool CSACvtCFDFPass::hasAllConstantInputs(MachineInstr *MI) {
  if (!TII->isMOV(MI))
    return false;

  for (auto &MO : MI->uses()) {
    // These are the known types of operands which will always be available
    // to the instruction/operator.
    bool isConst = MO.isImm() or MO.isCImm() or MO.isFPImm() or
                   MO.isGlobal() or MO.isSymbol();

    if (not isConst)
      return false;
  }
  return true;
}

void CSACvtCFDFPass::removeBranch() {
  std::deque<unsigned> renameQueue;
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end();
       BB != E; ++BB) {
    MachineBasicBlock::iterator iterMI = BB->begin();
    while (iterMI != BB->end()) {
      MachineInstr *MI = &*iterMI;
      ++iterMI;
      if (MI->isBranch()) {
        MI->removeFromParent();
      }
    }
  }
}

void CSACvtCFDFPass::linearizeCFG() {
  std::stack<MachineBasicBlock *> mbbStack;
  MachineBasicBlock *root = &*thisMF->begin();
  for (scc_iterator<MachineFunction *> I  = scc_begin(thisMF),
                                       IE = scc_end(thisMF);
       I != IE; ++I) {
    // Obtain the vector of BBs in this SCC
    const std::vector<MachineBasicBlock *> &SCCBBs = *I;
    for (std::vector<MachineBasicBlock *>::const_iterator BBI  = SCCBBs.begin(),
                                                          BBIE = SCCBBs.end();
         BBI != BBIE; ++BBI) {
      mbbStack.push(*BBI);
    }
  }
  MachineBasicBlock *x = mbbStack.top();
  assert(x == root);
  _unused(x);
  MachineBasicBlock::succ_iterator SI = root->succ_begin();
  while (SI != root->succ_end()) {
    SI = root->removeSuccessor(SI);
  }
  mbbStack.pop();
  while (!mbbStack.empty()) {
    MachineBasicBlock *mbb = mbbStack.top();
    mbbStack.pop();
    root->splice(root->end(), mbb, mbb->begin(), mbb->end());
    mbb->eraseFromParent();
  }
}

MachineInstr *CSACvtCFDFPass::PatchOrInsertPickAtFork(
  MachineBasicBlock *ctrlBB, // fork
  unsigned dst,              // the SSA value
  unsigned Reg,              // input of phi
  MachineBasicBlock *inBB,   // incoming blk
  MachineInstr *phi,         // the multi-input phi
  unsigned pickReg)          // pick output
{
  const TargetRegisterInfo &TRI =
    *thisMF->getSubtarget<CSASubtarget>().getRegisterInfo();
  MachineInstr *pickInstr                      = nullptr;
  bool patched                                 = false;
  DenseMap<unsigned, MachineInstr *> *reg2pick = nullptr;
  if (bb2pick.find(ctrlBB) == bb2pick.end()) {
    reg2pick        = new DenseMap<unsigned, MachineInstr *>();
    bb2pick[ctrlBB] = reg2pick;
  } else {
    reg2pick = bb2pick[ctrlBB];
  }

  if (reg2pick->find(dst) == reg2pick->end()) {
    pickInstr        = insertPICKForReg(ctrlBB, Reg, inBB, phi, pickReg);
    (*reg2pick)[dst] = pickInstr;
  } else {
    // find existing PICK, patch its %ign with Reg
    pickInstr             = (*reg2pick)[dst];
    unsigned pickFalseReg = 0, pickTrueReg = 0;
    assignPICKSrcForReg(pickFalseReg, pickTrueReg, Reg, ctrlBB, inBB, phi);
    unsigned ignIndex = 0;
    if (pickFalseReg == CSA::IGN) {
      // reg assigned to pickTrue => make sure the original pick has %IGN for
      // pickTrue;
      assert(pickTrueReg && pickTrueReg != CSA::IGN);
      assert(pickInstr->getOperand(3).getReg() == CSA::IGN);
      ignIndex = 3;
    } else {
      // reg assigned to pickFalse
      assert(pickTrueReg == CSA::IGN);
      assert(pickFalseReg && pickFalseReg != CSA::IGN);
      assert(pickInstr->getOperand(2).getReg() == CSA::IGN);
      ignIndex = 2;
    }
    MachineOperand &MO = pickInstr->getOperand(ignIndex);
    MO.substVirtReg(Reg, 0, TRI);
    MachineInstr *DefMI = MRI->getVRegDef(Reg);
    // if (TII->isPick(DefMI) && DefMI->getParent() == pickInstr->getParent()) {
    if (multiInputsPick.find(DefMI) != multiInputsPick.end()) {
      // make sure input src is before the pick
      // assert(DefMI->getParent() == pickInstr->getParent());
      pickInstr->removeFromParent();
      DefMI->getParent()->insertAfter(DefMI, pickInstr);
    }
    patched = true;
  }

  LLVM_DEBUG(dbgs() << "  Picking " << printReg(Reg) << " on the basis of BB#"
      << ctrlBB->getNumber() << " when coming from BB#" << inBB->getNumber()
      << " to register " << printReg(pickInstr->getOperand(0).getReg())
      << "\n");
  if (patched) {
    return NULL;
  } else {
    return pickInstr;
  }
}

MachineInstr *CSACvtCFDFPass::insertPICKForReg(MachineBasicBlock *ctrlBB,
                                               unsigned Reg,
                                               MachineBasicBlock *inBB,
                                               MachineInstr *phi,
                                               unsigned pickReg) {
  const TargetRegisterClass *TRC  = MRI->getRegClass(Reg);
  MachineBasicBlock::iterator loc = ctrlBB->getFirstTerminator();
  MachineInstr *bi                = &*loc;
  if (!pickReg) {
    pickReg = MRI->createVirtualRegister(TRC);
  }
  assert(bi->getOperand(0).isReg());
  unsigned predReg      = bi->getOperand(0).getReg();
  unsigned pickFalseReg = 0, pickTrueReg = 0;
  assignPICKSrcForReg(pickFalseReg, pickTrueReg, Reg, ctrlBB, inBB, phi);
  const unsigned pickOpcode = TII->makeOpcode(CSA::Generic::PICK, TRC);
  MachineInstr *pickInst =
    BuildMI(*phi->getParent(), phi, DebugLoc(), TII->get(pickOpcode), pickReg)
      .addReg(predReg)
      .addReg(pickFalseReg)
      .addReg(pickTrueReg);
  pickInst->setFlag(MachineInstr::NonSequential);
  // multiInputsPick.insert(pickInst);
  // bookkeeping pickInst and its predBB
  multiInputsPick[pickInst] = ctrlBB;
  return pickInst;
}

void CSACvtCFDFPass::assignPICKSrcForReg(unsigned &pickFalseReg,
                                         unsigned &pickTrueReg, unsigned Reg,
                                         MachineBasicBlock *ctrlBB,
                                         MachineBasicBlock *inBB,
                                         MachineInstr *phi) {
  if (inBB != ctrlBB) {
    ControlDependenceNode *inNode   = CDG->getNode(inBB);
    ControlDependenceNode *ctrlNode = CDG->getNode(ctrlBB);
    if (ctrlNode->isFalseChild(inNode)) {
      pickFalseReg = Reg;
      pickTrueReg  = CSA::IGN;
    } else {
      pickTrueReg  = Reg;
      pickFalseReg = CSA::IGN;
    }
  } else {
    MachineBasicBlock *mbb = phi->getParent();
    // assert(DT->dominates(ctrlBB, mbb));
    if (CDG->getEdgeType(ctrlBB, mbb, true) == ControlDependenceNode::TRUE) {
      pickTrueReg  = Reg;
      pickFalseReg = CSA::IGN;
    } else {
      pickFalseReg = Reg;
      pickTrueReg  = CSA::IGN;
    }
  }
}

void CSACvtCFDFPass::TraceCtrl(MachineBasicBlock *inBB, MachineBasicBlock *mbb,
                               unsigned Reg, unsigned dst, unsigned src,
                               MachineInstr *MI) {
  MachineBasicBlock *ctrlBB = nullptr;
  if (!DT->dominates(inBB, mbb)) {
    // If the operand we're tracing is itself a PHI instruction, pretend that
    // this PHI were instead merged with the original PHI instruction. In some
    // cases, this lets us generate clean pick trees even if the control-flow
    // graph itself isn't clean.
    // XXX: Do we need the PHI instruction to come specifically from this
    // basic block?
    MachineInstr *DefMI = MRI->getVRegDef(src);
    if (DefMI->getParent() == inBB && DefMI->isPHI())
      TraceThroughPhi(DefMI, mbb, dst);
    else {
      // Not a PHI instruction. This operand is generated from the parent node
      // (or some other basic block in the control-dependent region thereof).
      // Build a pick tree that goes through the control-dependent parents,
      // until we reach the dominator of the original PHI, in order.
      ControlDependenceNode *inNode = CDG->getNode(inBB);
      for (ControlDependenceNode::node_iterator pnode = inNode->parent_begin(),
                                                pend  = inNode->parent_end();
           pnode != pend; ++pnode) {
        ControlDependenceNode *ctrlNode = *pnode;
        ctrlBB                          = ctrlNode->getBlock();
        // Ignore loop-related control dependence edges.
        if (MLI->getLoopFor(ctrlBB) &&
            MLI->getLoopFor(ctrlBB)->isLoopExiting(ctrlBB))
          continue;

        // If the parent is the dominator, the pick we want will be the final
        // one, so generate the target output. Otherwise, create a new register
        // for the pick result that will be used as the pick input for the
        // next level.
        unsigned pickReg = 0;
        if (DT->dominates(ctrlBB, mbb)) {
          pickReg = dst;
        }
        MachineInstr *pickInstr =
          PatchOrInsertPickAtFork(ctrlBB, dst, Reg, inBB, MI, pickReg);
        // If the PICK instruction still has some missing nodes, continue
        // walking up the chain. Otherwise, it's just duplicating work we've
        // already done, so abort the recursion now.
        if (pickInstr) {
          TraceCtrl(ctrlBB, mbb, pickInstr->getOperand(0).getReg(), dst, src,
                    MI);
        }
      }
    }
  }
}

void CSACvtCFDFPass::TraceThroughPhi(MachineInstr *iphi, MachineBasicBlock *mbb,
                                     unsigned dst) {
  for (MIOperands MO(*iphi); MO.isValid(); ++MO) {
    if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg()))
      continue;
    if (MO->isUse()) {
      unsigned Reg = MO->getReg();
      unsigned src = MO->getReg();
      // move to its incoming block operand
      ++MO;
      MachineBasicBlock *inBB = MO->getMBB();
      if (DT->dominates(inBB, mbb)) {
        // We've reached the immediate dominator for the final phi via this
        // edge. Since we're processing a phi edge, this means that there is
        // an if statement with no else:
        // inBB -> (if code) -> iphi
        //   \__________________/  <-- the edge in question
        // If we postdominate the inBB as well, then no switches will have been
        // generated during switch generation, so we may need to generate one
        // right now.
        MachineInstr *dMI        = MRI->getVRegDef(Reg);
        MachineBasicBlock *DefBB = dMI->getParent();
        unsigned switchingDef    = findSwitchingDstForReg(Reg, DefBB);
        if (switchingDef) {
          Reg = switchingDef;
        }

        // Add the value to the pick tree.
        PatchOrInsertPickAtFork(inBB, dst, Reg, inBB, iphi, dst);
        continue;
      } else {
        // Recurse up the control-dependent chain of the graph to build the
        // pick tree. If the edge is a critical edge, then we have to do the
        // first step of pick tree construction right now, since we're
        // control-dependent on this block.
        // XXX: really wants to be control-dependent check?
        bool inBBFork = inBB->succ_size() > 1 &&
                        (!MLI->getLoopFor(inBB) ||
                         !MLI->getLoopFor(inBB)->isLoopExiting(inBB));
        if (inBBFork) {
          MachineInstr *pickInstr =
            PatchOrInsertPickAtFork(inBB, dst, Reg, inBB, iphi, 0);
          if (!pickInstr) {
            // patched
            continue; // to next MO
          } else {
            Reg = pickInstr->getOperand(0).getReg();
          }
        }
        TraceCtrl(inBB, mbb, Reg, dst, src, iphi);
      }
    }
  } // end of for MO
}

void CSACvtCFDFPass::generateCompletePickTreeForPhi(MachineBasicBlock *mbb) {
  MachineBasicBlock::iterator iterI = mbb->begin();
  while (iterI != mbb->end()) {
    MachineInstr *MI = &*iterI;
    ++iterI;
    if (!MI->isPHI())
      continue;
    multiInputsPick.clear();
    LLVM_DEBUG(dbgs() << "Generating pick tree for " << *MI);
    unsigned dst = MI->getOperand(0).getReg();
    TraceThroughPhi(MI, mbb, dst);
    MI->removeFromParent();

    LLVM_DEBUG(for (auto &pickInfo : multiInputsPick) {
      dbgs() << "  " << *pickInfo.first;
    });
    PatchCFGLeaksFromPickTree(dst, mbb);
  }
}

void CSACvtCFDFPass::TraceLeak(MachineBasicBlock *inBB, MachineBasicBlock *mbb,
                               SmallVectorImpl<unsigned> &landOpnds) {
  // XXX: This code is broken in the case where nodes have multiple
  // control-dependent parents.
  if (!DT->dominates(inBB, mbb)) {
    ControlDependenceNode *inNode = CDG->getNode(inBB);
    for (ControlDependenceNode::node_iterator pnode = inNode->parent_begin(),
                                              pend  = inNode->parent_end();
         pnode != pend; ++pnode) {
      ControlDependenceNode *ctrlNode = *pnode;
      MachineBasicBlock *ctrlBB       = ctrlNode->getBlock();
      // Ignore control-dependences due to loops.
      if (MLI->getLoopFor(ctrlBB) &&
          MLI->getLoopFor(ctrlBB)->isLoopExiting(ctrlBB))
        continue;
      MachineInstr *bi = &*ctrlBB->getFirstInstrTerminator();
      unsigned ec      = bi->getOperand(0).getReg();
      if (ctrlNode->isFalseChild(inNode)) {
        unsigned notReg = MRI->createVirtualRegister(&CSA::I1RegClass);
        BuildMI(*ctrlBB, ctrlBB->getFirstTerminator(), DebugLoc(),
                TII->get(CSA::NOT1), notReg)
          .addReg(ec);
        ec = notReg;
      }
      landOpnds.push_back(ec);

      TraceLeak(ctrlBB, mbb, landOpnds);
    }
  }
}

unsigned CSACvtCFDFPass::generateLandSeq(SmallVectorImpl<unsigned> &landOpnds,
                                         MachineBasicBlock *mbb,
                                         MachineInstr *MI) {
  MachineBasicBlock::instr_iterator loc = mbb->getFirstInstrTerminator();
  unsigned landResult                   = 0;
  unsigned landSrc                      = 0;
  MachineInstr *landInstr;
  unsigned i = 0;
  assert(landOpnds.size() > 0 && "Invalid number of operands.");
  for (; i < landOpnds.size(); i++) {
    if (!landSrc) {
      landSrc = landOpnds[i];
    } else if (!landResult) {
      landResult = MRI->createVirtualRegister(&CSA::I1RegClass);
      landInstr =
        BuildMI(*mbb, loc, DebugLoc(), TII->get(CSA::LAND1), landResult)
          .addReg(landSrc)
          .addReg(landOpnds[i]);
      if (MI) {
        // move landInst to after MI
        landInstr->removeFromParent();
        mbb->insert(MI, landInstr);
      }
      landInstr->setFlag(MachineInstr::NonSequential);
    } else {
      if (i % 4) {
        landInstr->addOperand(MachineOperand::CreateReg(landOpnds[i], false));
      } else {
        unsigned newResult = MRI->createVirtualRegister(&CSA::I1RegClass);
        landInstr =
          BuildMI(*mbb, loc, DebugLoc(), TII->get(CSA::LAND1), newResult)
            .addReg(landResult)
            .addReg(landOpnds[i]);
        if (MI) {
          landInstr->removeFromParent();
          mbb->insert(MI, landInstr);
        }
        landInstr->setFlag(MachineInstr::NonSequential);
        landResult = newResult;
      }
    }
  }
  // If only one LAND is reqired, then we have to fill up to 4
  // source operands.  If multiple LAND are required, then
  // we have already encoded one with the other LAND being its
  // first source operand - we need to fill up to 3 source
  // operands in this case.
  if (i == 1) {
    // CMPLRLLVM-5595: one operand is possible.
    return landSrc;
  }
  else if (i < 4 && (i % 4) != 0) {
    for (unsigned j = i % 4; j < 4; j++)
      landInstr->addOperand(MachineOperand::CreateImm(1));
  } else if (i > 4 && ((i - 4) % 3) != 0) {
    for (unsigned j = (i - 4) % 3; j < 3; j++)
      landInstr->addOperand(MachineOperand::CreateImm(1));
  }
  return landInstr->getOperand(0).getReg();
}

unsigned CSACvtCFDFPass::generateOrSeq(SmallVectorImpl<unsigned> &orOpnds,
                                       MachineBasicBlock *mbb,
                                       MachineInstr *ploc) {
  unsigned orSrc                  = 0;
  unsigned orResult               = 0;
  MachineInstr *orInstr           = nullptr;
  MachineBasicBlock::iterator loc = ploc ? *ploc : mbb->getFirstTerminator();
  for (unsigned i = 0; i < orOpnds.size(); ++i) {
    unsigned ec = orOpnds[i];
    if (!orSrc) {
      orSrc = ec;
    } else if (!orResult) {
      orResult = MRI->createVirtualRegister(&CSA::I1RegClass);
      orInstr  = BuildMI(*mbb, loc, DebugLoc(), TII->get(CSA::OR1), orResult)
                  .addReg(orSrc)
                  .addReg(ec);
      orInstr->setFlag(MachineInstr::NonSequential);
    } else {
      unsigned newResult = MRI->createVirtualRegister(&CSA::I1RegClass);
      orInstr = BuildMI(*mbb, loc, DebugLoc(), TII->get(CSA::OR1), newResult)
                  .addReg(orResult)
                  .addReg(ec);
      orInstr->setFlag(MachineInstr::NonSequential);
      orResult = newResult;
    }
  }
  unsigned pred = orResult ? orResult : orSrc;
  return pred;
}

void CSACvtCFDFPass::CombineDuplicatePickTreeInput() {
  SmallVector<unsigned, 4> landOpnds;
  DenseMap<MachineInstr *, MachineBasicBlock *>::iterator itmp =
    multiInputsPick.begin();
  while (itmp != multiInputsPick.end()) {
    MachineInstr *pickInstr = itmp->getFirst();
    ++itmp;
    unsigned dst                         = pickInstr->getOperand(0).getReg();
    MachineRegisterInfo::use_iterator UI = MRI->use_begin(dst);
    // assert single use???
    while (UI != MRI->use_end()) {
      MachineOperand &UseMO    = *UI;
      MachineInstr *UseMI      = UseMO.getParent();
      MachineBasicBlock *UseBB = UseMI->getParent();
      ++UI;
      if (multiInputsPick.find(UseMI) != multiInputsPick.end()) {
        unsigned useIndex = (dst == UseMI->getOperand(2).getReg()) ? 2 : 3;
        assert(dst != UseMI->getOperand(1).getReg());
        unsigned otherIndex = 5 - useIndex;
        unsigned otherReg   = UseMI->getOperand(otherIndex).getReg();
        if (otherReg == pickInstr->getOperand(2).getReg() ||
            otherReg == pickInstr->getOperand(3).getReg()) {
          unsigned dupIndex =
            (otherReg == pickInstr->getOperand(2).getReg()) ? 2 : 3;
          unsigned singleIndex = 5 - dupIndex;
          unsigned c1          = UseMI->getOperand(1).getReg();
          if (useIndex == 2) {
            // c1 = not c1
            unsigned notReg = MRI->createVirtualRegister(&CSA::I1RegClass);
            BuildMI(*UseBB, UseMI, DebugLoc(), TII->get(CSA::NOT1), notReg)
              .addReg(c1);
            c1 = notReg;
          }
          unsigned c2 = pickInstr->getOperand(1).getReg();
          if (singleIndex == 2) {
            // c2 = not c2
            unsigned notReg = MRI->createVirtualRegister(&CSA::I1RegClass);
            BuildMI(*UseBB, UseMI, DebugLoc(), TII->get(CSA::NOT1), notReg)
              .addReg(c2);
            c2 = notReg;
          }
          landOpnds.push_back(c1);
          landOpnds.push_back(c2);
          unsigned c0 = generateLandSeq(landOpnds, UseBB, UseMI);
          landOpnds.clear();
          // c0 = land c1, c2, 1, 1, and insert before UseMI
          // rewrite UseMI's c to c0, opnd2 to pickInstr's dupIndex, opnd3
          // pickInstr's singleIndex remove pickInstr
          UseMI->RemoveOperand(3);
          UseMI->RemoveOperand(2);
          UseMI->RemoveOperand(1);
          UseMI->addOperand(MachineOperand::CreateReg(c0, false));
          UseMI->addOperand(MachineOperand::CreateReg(
            pickInstr->getOperand(dupIndex).getReg(), false));
          UseMI->addOperand(MachineOperand::CreateReg(
            pickInstr->getOperand(singleIndex).getReg(), false));

          LLVM_DEBUG(dbgs() << "Patching " << *pickInstr);
          pickInstr->removeFromParent();
          // remove pickInstr from MultiInputsPick
          multiInputsPick.erase(pickInstr);
        }
      }
    }
  }
}


unsigned CSACvtCFDFPass::findLoopExitCondition(MachineLoop* mloop) {
  SmallVector<unsigned, 4> landOpnds;
  SmallVector<unsigned, 4> orOpnds;
  SmallVector<MachineBasicBlock*, 4> exitingBBs;
  mloop->getExitingBlocks(exitingBBs);
  for (unsigned i = 0; i < exitingBBs.size(); i++) {
    MachineBasicBlock* exitingBB = exitingBBs[i];
    MachineBasicBlock*exitBB = nullptr;
    for (MachineBasicBlock::succ_iterator psucc = exitingBB->succ_begin();
      psucc != exitingBB->succ_end(); psucc++) {
      MachineBasicBlock *msucc = *psucc;
      if (MLI->getLoopFor(msucc) != mloop) {
        exitBB = msucc;
        break;
      }
    }
    MachineInstr *bi = &*exitingBB->getFirstInstrTerminator();
    MachineBasicBlock::iterator loc = exitingBB->getFirstTerminator();
    unsigned predReg = bi->getOperand(0).getReg();
    const TargetRegisterClass *new_LIC_RC =
      LMFI->licRCFromGenRC(MRI->getRegClass(predReg));
    assert(new_LIC_RC && "Can't determine register class for register");
    unsigned cpyReg =
      LMFI->allocateLIC(new_LIC_RC, Twine("loop_") + exitingBB->getName() + "_exiting");

    const TargetRegisterClass *TRC = MRI->getRegClass(predReg);
    if (CDG->getEdgeType(exitingBB, exitBB, true) != ControlDependenceNode::FALSE) {
      unsigned int notOpcode = TII->makeOpcode(CSA::Generic::NOT, TRC);
      MachineInstr* notInstr = BuildMI(*exitingBB, loc, DebugLoc(),
                                       TII->get(notOpcode), cpyReg).addReg(predReg);
      notInstr->setFlag(MachineInstr::NonSequential);
      predReg = cpyReg;
    }
    landOpnds.clear();
    landOpnds.push_back(predReg);
    TraceLeak(exitingBB, mloop->getHeader(), landOpnds);
    std::reverse(landOpnds.begin(), landOpnds.end());
    unsigned landSeq = generateLandSeq(landOpnds, exitingBB);
    orOpnds.push_back(landSeq);
  }

  unsigned orSeq = generateOrSeq(orOpnds, mloop->getLoopLatch());
  return orSeq;
}





// for each IGN remaining in the multiInputPick, generate an land
void CSACvtCFDFPass::PatchCFGLeaksFromPickTree(unsigned phiDst, MachineBasicBlock *phiHome) {
  CombineDuplicatePickTreeInput();

  SmallVector<unsigned, 4> landOpnds;
  SmallVector<unsigned, 4> orOpnds;
  for (DenseMap<MachineInstr *, MachineBasicBlock *>::iterator itmp =
         multiInputsPick.begin();
       itmp != multiInputsPick.end(); itmp++) {
    MachineInstr *pickInstr = itmp->getFirst();
    assert(pickInstr->getOperand(2).getReg() != CSA::IGN ||
           pickInstr->getOperand(3).getReg() != CSA::IGN);
    unsigned pickIGNInd = 0;
    if (pickInstr->getOperand(2).getReg() == CSA::IGN)
      pickIGNInd = 2;
    else if (pickInstr->getOperand(3).getReg() == CSA::IGN)
      pickIGNInd = 3;

    if (!pickIGNInd)
      continue;
    landOpnds.clear();
    ControlDependenceNode::EdgeType childType = pickIGNInd == 2
                                                  ? ControlDependenceNode::FALSE
                                                  : ControlDependenceNode::TRUE;
    // all pick instructions in the pick tree are all in phi's home block
    MachineBasicBlock *ctrlBB = itmp->getSecond();
    // start building land from ctrlBB and its child
    unsigned ec = pickInstr->getOperand(1).getReg();
    if (childType == ControlDependenceNode::FALSE) {
      unsigned notReg = MRI->createVirtualRegister(&CSA::I1RegClass);
      BuildMI(*ctrlBB, ctrlBB->getFirstTerminator(), DebugLoc(),
              TII->get(CSA::NOT1), notReg)
        .addReg(ec);
      ec = notReg;
    }
    landOpnds.push_back(ec);
    // now trace from CtrlBB upward till the entry of the if-tree
    TraceLeak(ctrlBB, phiHome, landOpnds);
    // replace IGN with 0 to be ignored later one
    if (pickIGNInd == 3) {
      pickInstr->RemoveOperand(3);
      pickInstr->addOperand(MachineOperand::CreateImm(0));
    } else {
      unsigned pickSrc = pickInstr->getOperand(3).getReg();
      pickInstr->RemoveOperand(3);
      pickInstr->RemoveOperand(2);
      pickInstr->addOperand(MachineOperand::CreateImm(0));
      pickInstr->addOperand(MachineOperand::CreateReg(pickSrc, false));
    }
    std::reverse(landOpnds.begin(), landOpnds.end());
    unsigned landSeq = generateLandSeq(landOpnds, phiHome);
    orOpnds.push_back(landSeq);
  }

  // If we do not have any leaks, do nothing.
  if (orOpnds.empty())
    return;

  unsigned orSeq = generateOrSeq(orOpnds, phiHome);
  ////////
  MachineInstr *finalPick = MRI->getVRegDef(phiDst);
  assert(TII->isPick(finalPick));
  unsigned tmpResult = MRI->createVirtualRegister(MRI->getRegClass(phiDst));
  finalPick->getOperand(0).setReg(tmpResult);
  const unsigned switchOpcode =
    TII->makeOpcode(CSA::Generic::SWITCH, MRI->getRegClass(phiDst));
  MachineInstr *switchInst = BuildMI(*phiHome, phiHome->getFirstTerminator(),
                                     DebugLoc(), TII->get(switchOpcode), phiDst)
                               .addReg(CSA::IGN, RegState::Define)
                               .addReg(orSeq)
                               .addReg(tmpResult);
  switchInst->setFlag(MachineInstr::NonSequential);

  // Note the switch output as being set to the current basic block.
  unsigned bbnum = phiHome->getNumber();
  unsigned vregNo = TargetRegisterInfo::virtReg2Index(phiDst);
  licGrouping.grow(vregNo + 1);
  if (basicBlockRegs[bbnum] == UNMAPPED_REG)
    basicBlockRegs[bbnum] = vregNo;
  else {
    licGrouping.join(vregNo, basicBlockRegs[bbnum]);
  }
}

unsigned
CSACvtCFDFPass::getEdgePred(MachineBasicBlock *mbb,
                            ControlDependenceNode::EdgeType childType) {
  if (edgepreds.find(mbb) == edgepreds.end())
    return 0;
  return (*edgepreds[mbb])[childType];
}

void CSACvtCFDFPass::setEdgePred(MachineBasicBlock *mbb,
                                 ControlDependenceNode::EdgeType childType,
                                 unsigned ch) {
  assert(ch && "0 is not a valid vreg number");
  if (edgepreds.find(mbb) == edgepreds.end()) {
    SmallVectorImpl<unsigned> *childVect = new SmallVector<unsigned, 2>;
    childVect->push_back(0);
    childVect->push_back(0);
    edgepreds[mbb] = childVect;
  }
  (*edgepreds[mbb])[childType] = ch;
  nameLIC(ch, "", 0, "", mbb,
    (childType == ControlDependenceNode::FALSE ? ".false.pred" :
     childType == ControlDependenceNode::TRUE ? ".true.pred" : ".other.pred"));
  LLVM_DEBUG(dbgs() << "Edge predicate of "
      << mbb->getName() << "->" << (*(mbb->succ_begin() + childType))->getName()
      << " is " << printReg(ch) << "\n");
}

unsigned CSACvtCFDFPass::getBBPred(MachineBasicBlock *mbb) {
  if (bbpreds.find(mbb) == bbpreds.end())
    return 0;
  return bbpreds[mbb];
}

void CSACvtCFDFPass::setBBPred(MachineBasicBlock *mbb, unsigned ch) {
  assert(ch && "0 is not a valid vreg number");
  // don't set it twice
  assert(bbpreds.find(mbb) == bbpreds.end() && "CSA: Try to set bb pred twice");
  bbpreds[mbb] = ch;
}

unsigned CSACvtCFDFPass::computeEdgePred(MachineBasicBlock *fromBB,
                                         MachineBasicBlock *toBB,
                                         std::list<MachineBasicBlock *> &path) {
  assert(toBB);
  ControlDependenceNode::EdgeType childType = CDG->getEdgeType(fromBB, toBB);
  // make sure each edge is computed only once to avoid cycling on backedge
  if (unsigned edgeReg = getEdgePred(fromBB, childType)) {
    return edgeReg;
  }
  unsigned bbPredReg = 0;
  if (CDG->getNode(fromBB)->getNumParents() == 1 &&
      *CDG->getNode(fromBB)->parent_begin() == CDG->getRoot()) {
    // root level, out side any loop
    // mov 1
    if (!(bbPredReg = getBBPred(fromBB))) {
      MachineBasicBlock *entryBB = &*thisMF->begin();
      unsigned cpyReg            = MRI->createVirtualRegister(&CSA::I1RegClass);
      BuildMI(*entryBB, entryBB->getFirstTerminator(), DebugLoc(),
              TII->get(CSA::GATE1), cpyReg)
        .addReg(LMFI->getInMemoryLic())
        .addImm(1)
        .setMIFlag(MachineInstr::NonSequential);
      bbPredReg = cpyReg;
      setBBPred(fromBB, bbPredReg);
    }
  } else if (MLI->getLoopFor(toBB) &&
             MLI->getLoopFor(toBB)->getHeader() == toBB &&
             !MLI->getLoopFor(toBB)->contains(fromBB)) {
    // fromBB outside current loop, using loop as the unit of the region,
    // reaching the boundary, use pre-assigned BB pred for init input of loop
    if (bb2predcpy.find(fromBB) == bb2predcpy.end()) {
      bbPredReg          = MRI->createVirtualRegister(&CSA::I1RegClass);
      bb2predcpy[fromBB] = bbPredReg;
    } else {
      bbPredReg = bb2predcpy[fromBB];
    }
    if (!MLI->getLoopFor(fromBB)) {
      // post process after all loops are handled
      dcgBBs.insert(fromBB);
    }
  } else {
    // non boundary -- need to compute
    bbPredReg = computeBBPred(fromBB, path);
  }
  unsigned result = 0;
  if (fromBB->succ_size() == 1) {
    result = bbPredReg;
  } else {
    // calling omputeBBPred => calling computeEdgePred
    //=> might have insert prop due to self cycle
    if (unsigned edgeReg = getEdgePred(fromBB, childType)) {
      result = edgeReg;
    } else {
      InsertPredProp(fromBB, bbPredReg);
      result = getEdgePred(fromBB, childType);
    }
  }
  return result;
}

unsigned
CSACvtCFDFPass::mergeIncomingEdgePreds(MachineBasicBlock *inBB,
                                       std::list<MachineBasicBlock *> &path) {
  unsigned mergeOp, numInputs;
  if (MLI->getLoopFor(inBB) && MLI->getLoopFor(inBB)->getHeader() == inBB) {
    mergeOp   = CSA::LOR1;
    numInputs = 4;
  } else {
    mergeOp   = CSA::OR1;
    numInputs = 2;
  }

  unsigned predBB           = 0; // lor Result
  MachineBasicBlock *ctrlBB = nullptr;
  unsigned ctrlEdge;
  unsigned lorSrc        = 0;
  MachineInstr *lorInstr = nullptr;
  unsigned i             = 0;
  unsigned backedgeCtrl  = 0;
  for (MachineBasicBlock::pred_iterator ipred = inBB->pred_begin();
       ipred != inBB->pred_end(); ipred++, i++) {
    ctrlBB = *ipred;
    // no need to (bb2rpo[ctrlBB] < bb2rpo[inBB]), as long as each edge is
    // computed once.  handle single edge as well as fork edge
    ctrlEdge = computeEdgePred(ctrlBB, inBB, path);
    if (MLI->getLoopFor(ctrlBB) &&
        MLI->getLoopFor(ctrlBB)->isLoopLatch(ctrlBB)) {
      backedgeCtrl = ctrlEdge;
    }
    MachineBasicBlock::iterator loc = inBB->getFirstTerminator();
    // merge predecessor if needed
    if (!lorSrc) {
      lorSrc = ctrlEdge;
    } else if (!predBB) {
      predBB = MRI->createVirtualRegister(&CSA::I1RegClass);
      assert(TargetRegisterInfo::isVirtualRegister(ctrlEdge));
      assert(TargetRegisterInfo::isVirtualRegister(lorSrc));
      lorInstr = BuildMI(*inBB, loc, DebugLoc(), TII->get(mergeOp), predBB)
                   .addReg(lorSrc)
                   .addReg(ctrlEdge);
      lorInstr->setFlag(MachineInstr::NonSequential);
    } else {
      if ((i % numInputs) && (numInputs > 2)) {
        lorInstr->addOperand(MachineOperand::CreateReg(ctrlEdge, false));
      } else {
        unsigned newResult = MRI->createVirtualRegister(&CSA::I1RegClass);
        lorInstr = BuildMI(*inBB, loc, DebugLoc(), TII->get(mergeOp), newResult)
                     .addReg(predBB)
                     .addReg(ctrlEdge);
        lorInstr->setFlag(MachineInstr::NonSequential);
        predBB = newResult;
      }
    }
  }
  if (i == 1) {
    predBB = lorSrc;
  } else {
    if ((i % numInputs) && (numInputs > 2)) {
      for (unsigned j = i % numInputs; j < numInputs; j++) {
        lorInstr->addOperand(MachineOperand::CreateImm(0));
      }
    }
  }

  // make sure back edge pred is the first operand of lor, otherwise exchang
  // operands;  to avoid repeat operand
  MachineLoop *mloop = MLI->getLoopFor(inBB);
  if (mloop && mloop->getHeader() == inBB) {
    MachineInstr *lorInstr = MRI->getVRegDef(predBB);
    if (inBB->pred_size() == 2) {
      unsigned reg = lorInstr->getOperand(2).getReg();
      if (reg == backedgeCtrl) {
        // exchnage operands
        MachineOperand &MO2 = lorInstr->getOperand(2);
        MachineOperand &MO1 = lorInstr->getOperand(1);
        MO2.setReg(MO1.getReg());
        MO1.setReg(reg);
      }
    } else {
      // loop has more than one entrance edge
      // replace current lor with two lors with firt lor consolidate all
      // entrance edge,  second lor has backegde and consolidated init value
      SmallVector<unsigned, 4> opnds;
      for (unsigned i = 1; i < lorInstr->getNumOperands(); i++) {
        if (lorInstr->getOperand(i).isReg()) {
          unsigned reg = lorInstr->getOperand(i).getReg();
          if (reg != backedgeCtrl) {
            opnds.push_back(reg);
          }
        }
      }
      unsigned initCombined =
        generateOrSeq(opnds, lorInstr->getParent(), lorInstr);
      MachineInstr *newLorInstr =
        BuildMI(*lorInstr->getParent(), lorInstr, DebugLoc(),
                TII->get(lorInstr->getOpcode()),
                lorInstr->getOperand(0).getReg())
          .addReg(backedgeCtrl)
          .addReg(initCombined)
          .addImm(0)
          .addImm(0);
      newLorInstr->setFlag(MachineInstr::NonSequential);
      lorInstr->removeFromParent();
    }
  }
  return predBB;
}

unsigned CSACvtCFDFPass::computeBBPred(MachineBasicBlock *inBB,
                                       std::list<MachineBasicBlock *> &path) {
  if (unsigned c = getBBPred(inBB)) {
    return c;
  } else {
    if (std::find(path.begin(), path.end(), inBB) != path.end()) {
      // find a cycle
      return bb2predcpy[inBB];
    }

    path.push_back(inBB);

    unsigned result;
    if (bb2predcpy.find(inBB) == bb2predcpy.end()) {
      result = MRI->createVirtualRegister(&CSA::I1RegClass);
      nameLIC(result, "", 0, "", inBB, ".pred");
      bb2predcpy[inBB] = result;
    } else {
      result = bb2predcpy[inBB];
    }

    unsigned predBB = mergeIncomingEdgePreds(inBB, path);

    setBBPred(inBB, predBB);

    path.pop_back();
    // copy predBB to result for cycle of depencence; dead code it if not used
    if (!MRI->use_empty(result)) {
      MachineInstr *movPred = BuildMI(*inBB, inBB->getFirstTerminator(),
                                      DebugLoc(), TII->get(CSA::MOV1), result)
                                .addReg(predBB);
      MachineRegisterInfo::use_iterator UI = MRI->use_begin(result);
      MachineInstr *usePred                = UI->getParent();
      if (usePred->getOpcode() == CSA::PREDPROP) {
        assert(usePred->getParent() == inBB);
        usePred->removeFromParent();
        inBB->insertAfter(movPred, usePred);
      }
      return result;
    }
    return predBB;
  }
}

MachineInstr *CSACvtCFDFPass::getOrInsertPredMerge(MachineBasicBlock *mbb,
                                                   MachineInstr *loc,
                                                   unsigned e1, unsigned e2) {
  MachineInstr *predMergeInstr = nullptr;
  if (bb2predmerge.find(mbb) == bb2predmerge.end()) {
    unsigned indexReg = MRI->createVirtualRegister(&CSA::I1RegClass);
    predMergeInstr    = BuildMI(*mbb, loc, DebugLoc(), TII->get(CSA::PREDMERGE),
                             CSA::IGN)
                       . // in a two-way merge, it is %IGN to eat the BB's pred,
                         // they will be computed using "or" consistently
                     addReg(indexReg, RegState::Define)
                       .addReg(e1)
                       .         // last processed edge
                     addReg(e2); // current edge
    bb2predmerge[mbb] = predMergeInstr;
  } else {
    predMergeInstr = bb2predmerge[mbb];
  }
  return predMergeInstr;
}

// also set up edge pred
MachineInstr *CSACvtCFDFPass::InsertPredProp(MachineBasicBlock *mbb,
                                             unsigned bbPred) {
  assert(mbb->succ_size() == 2);
  if (bbPred == 0) {
    bbPred = getBBPred(mbb);
  }
  unsigned falseEdge = MRI->createVirtualRegister(&CSA::I1RegClass);
  unsigned trueEdge  = MRI->createVirtualRegister(&CSA::I1RegClass);
  MachineBasicBlock::iterator loc = mbb->getFirstTerminator();
  MachineInstr *bi                = &*loc;
  MachineInstr *predPropInstr =
    BuildMI(*mbb, loc, DebugLoc(), TII->get(CSA::PREDPROP), falseEdge)
      .addReg(trueEdge, RegState::Define)
      .addReg(bbPred)
      .addReg(bi->getOperand(0).getReg());
  setEdgePred(mbb, ControlDependenceNode::FALSE, falseEdge);
  setEdgePred(mbb, ControlDependenceNode::TRUE, trueEdge);
  return predPropInstr;
}

bool CSACvtCFDFPass::parentsLinearInCDG(MachineBasicBlock *mbb) {
  std::list<ControlDependenceNode *> parents;
  parents.clear();
  ControlDependenceNode *inNode = CDG->getNode(mbb);
  parents.clear();
  for (ControlDependenceNode::node_iterator pnode = inNode->parent_begin(),
                                            pend  = inNode->parent_end();
       pnode != pend; ++pnode) {
    ControlDependenceNode *ctrlNode = *pnode;
    MachineBasicBlock *ctrlBB       = ctrlNode->getBlock();
    if (!ctrlBB)
      continue;
    // Ignore control-dependences due to loops.
    if (MLI->getLoopFor(ctrlBB) &&
        MLI->getLoopFor(ctrlBB)->isLoopExiting(ctrlBB))
      continue;
    // sort parents using the non-transitive parent relationship
    std::list<ControlDependenceNode *>::iterator parent = parents.begin();
    while (parent != parents.end()) {
      if (ctrlNode->isParent(*parent)) {
        // insert before parent
        break;
      } else if ((*parent)->isParent(ctrlNode)) {
        // insert after parent
        parent++;
        break;
      }
      parent++;
    }
    parents.insert(parent, ctrlNode);
  }
  // all parents have to form a linear control dependence relationship
  if (parents.size() > 1) {
    std::list<ControlDependenceNode *>::iterator parent = parents.begin();
    std::list<ControlDependenceNode *>::iterator next   = parent;
    next++;
    while (next != parents.end()) {
      if (!(*parent)->isParent(*next))
        return false;
      parent = next;
      next++;
    }
  }
  return true;
}

//need to use dynamic predication for loops with multiple exits;
//and for jump-thread style branches where nested braches
//have shared or interleaving if-footers. In CDG, those are nodes
//with more than one parents with each parents unrelated to each other
//JIRA CMPLRS-50024 CMPLRS-50410 have such CFGs.
//one such example is:
/*
                                 |--|
                                 +---
                               --/ |
                           |--/    |
                           +-|-    |
                             |    /
                             |    |
                             |  |-|-|
                              \ +----
                              |  /  \-
                              |-/     \-
                            |-|-|      |\--|
                            +----      +-/--
                                \-     -/
                                  \   /
                                  |-/-|
                                  +----
*/
bool CSACvtCFDFPass::needDynamicPreds() {
  for (MachineLoopInfo::iterator LI = MLI->begin(), LE = MLI->end(); LI != LE;
       ++LI) {
    if (needDynamicPreds(*LI)) {
      return true;
    }
  }
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end();
       BB != E; ++BB) {
    MachineBasicBlock *mbb = &*BB;
    if (!parentsLinearInCDG(mbb)) {
      LLVM_DEBUG(dbgs() << "Using dynamic predication because parents of BB#"
          << mbb->getNumber() << " are nonlinear.\n");
      return true;
    }
    unsigned nParent              = 0;
    ControlDependenceNode *inNode = CDG->getNode(mbb);
    for (ControlDependenceNode::node_iterator pnode = inNode->parent_begin(),
           pend  = inNode->parent_end();
         pnode != pend; ++pnode) {
      ControlDependenceNode *ctrlNode = *pnode;
      MachineBasicBlock *ctrlBB       = ctrlNode->getBlock();
      if (!ctrlBB)
        continue;
      // A loop is control dependent on the exiting blocks of the loop. Ignore
      // nodes that are exiting control blocks.
      if (MLI->getLoopFor(ctrlBB) &&
          MLI->getLoopFor(ctrlBB)->isLoopExiting(ctrlBB))
        continue;
      nParent++;
    }
    if (nParent > 1 && mbb->succ_size() > 1) {
      // the phi value will be used by the switch to define new values
      LLVM_DEBUG(dbgs() << "Using dynamic predication because BB#"
          << mbb->getNumber() << " has too many control-dependent parents.\n");
      return true;
    }
  }
  return false;
}

bool CSACvtCFDFPass::needDynamicPreds(MachineLoop *L) {
  for (MachineLoop::iterator LI = L->begin(), LE = L->end(); LI != LE; ++LI) {
    if (needDynamicPreds(*LI))
      return true;
  }
  MachineLoop *mloop = L;
  // multiple exiting blocks
  if (!L->getExitingBlock()) {
    LLVM_DEBUG(dbgs() << L
        << " has multiple exiting blocks, requires dynamic predication.\n");
    return true;
  }
  MachineBasicBlock *lhdr = mloop->getHeader();

  if (lhdr->pred_size() > 2) {
    LLVM_DEBUG(dbgs() << L << " has a header with more than two predecessors, "
        "requires dynamic predication.\n");
    return true;
  }
  // multiple backedges
  MachineBasicBlock *latch = mloop->getLoopLatch();
  if (!latch) {
    LLVM_DEBUG(dbgs() << L << " has multiple backedges, "
        "requires dynamic predication.\n");
    return true;
  }
  return false;
}

unsigned CSACvtCFDFPass::getInnerLoopPipeliningDegree(MachineLoop *L) {
  // No pipelining if predprop/predmerge may be around. (At least until we
  // are sure about how they interact.)
  if (UseDynamicPred)
    return 1;

  // No pipelining if this loop requires dynamic predication.
  if (needDynamicPreds(L))
    return 1;

  // No pipelining if we can't identify a loop header.
	MachineBasicBlock *header = L->getHeader();
	if (not header)
		return 1;

  // Look for a marker left by the IR prep pass in the loop header. Sometimes
  // it gets moved here.
	for (MachineInstr &headerInst : *header) {
		if (headerInst.getOpcode() == CSA::CSA_PIPELINEABLE_LOOP) {
			unsigned maxDOP = headerInst.getOperand(0).getImm();
			// Remove the directive so that we know it was acted upon.
			headerInst.eraseFromParentAndMarkDBGValuesForRemoval();
			return maxDOP;
		}
	}

  // Also check the latch. This is where it's inserted and usually found.
  MachineBasicBlock *latch = L->getLoopLatch();
  if (not latch)
    return 1;

	for (MachineInstr &latchInst : *latch) {
    if (latchInst.getOpcode() == CSA::CSA_PIPELINEABLE_LOOP) {
      unsigned maxDOP = latchInst.getOperand(0).getImm();
      // Remove the directive so that we know it was acted upon.
      latchInst.eraseFromParentAndMarkDBGValuesForRemoval();
      return maxDOP;
    }
  }

  // Finally, if there's no indication from the prep pass that we can pipeline,
  // don't.
  return 1;
}

void CSACvtCFDFPass::generateDynamicPreds() {
  for (MachineLoopInfo::iterator LI = MLI->begin(), LE = MLI->end(); LI != LE;
       ++LI) {
    generateDynamicPreds(*LI);
  }

  for (std::set<MachineBasicBlock *>::iterator ibb = dcgBBs.begin();
       ibb != dcgBBs.end(); ibb++) {
    MachineBasicBlock *mbb = *ibb;
    std::list<MachineBasicBlock *> path;
    computeBBPred(mbb, path);
  }
  for (MachineFunction::iterator ibb = thisMF->begin(); ibb != thisMF->end();
       ibb++) {
    MachineBasicBlock *mbb = &*ibb;
    if (!MLI->getLoopFor(mbb) && mbb->getFirstNonPHI() != mbb->begin()) {
      generateDynamicPickTreeForFooter(mbb);
    }
  }
}

void CSACvtCFDFPass::generateDynamicPreds(MachineLoop *L) {
  for (MachineLoop::iterator LI = L->begin(), LE = L->end(); LI != LE; ++LI) {
    generateDynamicPreds(*LI);
  }
  MachineLoop *mloop      = L;
  MachineBasicBlock *lhdr = mloop->getHeader();

  generateDynamicPickTreeForHeader(lhdr);

  for (MachineLoop::block_iterator BI = mloop->block_begin(),
                                   BE = mloop->block_end();
       BI != BE; ++BI) {
    MachineBasicBlock *mbb = *BI;
    // only conside blocks in the current loop level, blocks in the nested level
    // are done before.
    if (MLI->getLoopFor(mbb) != mloop)
      continue;
    for (MachineBasicBlock::succ_iterator psucc = mbb->succ_begin();
         psucc != mbb->succ_end(); psucc++) {
      MachineBasicBlock *msucc = *psucc;
      // need to drive the compute for BB that is nested loop's preheader, in
      // case the current loop missed it
      if (MLI->getLoopFor(msucc) && MLI->getLoopFor(msucc) != mloop &&
          MLI->getLoopFor(msucc)->getHeader() == msucc) {
        std::list<MachineBasicBlock *> path;
        computeBBPred(mbb, path);
        break;
      }
    }

    if (mbb->getFirstNonPHI() == mbb->begin())
      continue;
    generateDynamicPickTreeForFooter(mbb);
  }
}

void CSACvtCFDFPass::repeatOperandInLoopUsePred(MachineLoop *mloop,
                                                MachineInstr *initInst,
                                                unsigned backedgePred,
                                                unsigned exitPred) {
  unsigned predReg   = initInst->getOperand(0).getReg();
  unsigned predConst = initInst->getOperand(1).getImm();
  assert(!predConst);
  _unused(predConst);
  MachineBasicBlock *lphdr   = mloop->getHeader();
  MachineBasicBlock *latchBB = mloop->getLoopLatch();
  assert(latchBB);

  std::set<MachineInstr *> repeats;

  for (MachineLoop::block_iterator BI = mloop->block_begin(),
                                   BE = mloop->block_end();
       BI != BE; ++BI) {
    MachineBasicBlock *mbb = *BI;
    // only conside blocks in the current loop level, blocks in the nested level
    // are done before.
    if (MLI->getLoopFor(mbb) != mloop)
      continue;
    MachineBasicBlock::iterator I = mbb->begin();
    while (I != mbb->end()) {
      MachineInstr *MI = &*I;
      I++;
      if ((MI->isPHI() && mloop->getHeader() == mbb) ||
          (repeats.find(MI) != repeats.end())) {
        // loop hdr phi's init input is used only once, no need to repeat
        continue;
      }
      for (MIOperands MO(*MI); MO.isValid(); ++MO) {
        if (!MO->isReg() ||
            !TargetRegisterInfo::isVirtualRegister(MO->getReg()))
          continue;
        unsigned Reg = MO->getReg();
        if (MO->isUse()) {
          MachineInstr *dMI = MRI->getUniqueVRegDef(Reg);
          if (!dMI || dMI->getOpcode() == CSA::PREDPROP) {
            continue;
          }
          MachineBasicBlock *DefBB = dMI->getParent();
          if (DefBB == mbb)
            continue;

          // use, def in different region cross latch
          bool isDefOutsideLoop =
            MLI->getLoopFor(DefBB) == NULL ||
            !MLI->getLoopFor(mbb)->contains(MLI->getLoopFor(DefBB));

          if (isDefOutsideLoop && DT->dominates(DefBB, mbb)) {
            assert(
              (!hasAllConstantInputs(dMI) || MLI->getLoopFor(DefBB) == NULL) &&
              "const prop failed");
#if 0
            if (hasAllConstantInputs(dMI)) {
			  //has to be root mov 1 pred instr
              assert(!dMI->getFlag(MachineInstr::NonSequential));
              continue;
            }
#endif
            const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
            unsigned rptIReg               = MRI->createVirtualRegister(TRC);
            unsigned rptOReg               = MRI->createVirtualRegister(TRC);
            const unsigned pickOpcode =
              TII->makeOpcode(CSA::Generic::PICK, TRC);
            MachineInstr *pickInst =
              BuildMI(*lphdr, lphdr->getFirstTerminator(), DebugLoc(),
                      TII->get(pickOpcode), rptOReg)
                .addReg(predReg)
                .addReg(Reg)
                .addReg(rptIReg);
            pickInst->setFlag(MachineInstr::NonSequential);
            repeats.insert(pickInst);

            unsigned notExit = MRI->createVirtualRegister(&CSA::I1RegClass);
            BuildMI(*latchBB, latchBB->getFirstTerminator(), DebugLoc(),
                    TII->get(CSA::NOT1), notExit)
              .addReg(exitPred);

            SmallVector<unsigned, 4> landOpnds;
            landOpnds.push_back(notExit);
            landOpnds.push_back(backedgePred);
            unsigned rptPred = generateLandSeq(landOpnds, latchBB);

            const unsigned switchOpcode =
              TII->makeOpcode(CSA::Generic::SWITCH, TRC);
            MachineInstr *switchInst =
              BuildMI(*latchBB, latchBB->getFirstTerminator(), DebugLoc(),
                      TII->get(switchOpcode), CSA::IGN)
                .addReg(rptIReg, RegState::Define)
                .addReg(rptPred)
                .addReg(rptOReg);
            switchInst->setFlag(MachineInstr::NonSequential);
            repeats.insert(switchInst);
            MachineRegisterInfo::use_iterator UI = MRI->use_begin(Reg);
            while (UI != MRI->use_end()) {
              MachineOperand &UseMO    = *UI;
              MachineInstr *UseMI      = UseMO.getParent();
              MachineBasicBlock *UseBB = UseMI->getParent();
              ++UI;
              if (UseMI != pickInst && MLI->getLoopFor(UseBB) &&
                  MLI->getLoopFor(UseBB) == mloop &&
                  // loop hdr phi's init input is used only once, no need to
                  // repeat
                  !(UseMI->isPHI() && UseBB == mloop->getHeader())) {
                if (mloop->isLoopExiting(UseBB) &&
                    !mloop->isLoopExiting(latchBB)) {
                  // closed loop latch can cause CDG relationship reversed --
                  // exits control everything connected to latch,  need switch to
                  // guard channel overflow
                  unsigned pbb    = getBBPred(UseBB);
                  unsigned rptReg = MRI->createVirtualRegister(TRC);
                  MachineInstr *switchInst =
                    BuildMI(*UseBB, UseBB->getFirstTerminator(), DebugLoc(),
                            TII->get(switchOpcode), CSA::IGN)
                      .addReg(rptReg, RegState::Define)
                      .addReg(pbb)
                      .addReg(rptOReg);
                  switchInst->setFlag(MachineInstr::NonSequential);
                  UseMO.setReg(rptReg);
                } else {
                  UseMO.setReg(rptOReg);
                }
              }
            }
          }
        }
      }
    }
  }
}

void CSACvtCFDFPass::generateDynamicPickTreeForHeader(MachineBasicBlock *mbb) {
  SmallVector<std::pair<unsigned, unsigned> *, 4> pred2values;
  std::list<MachineBasicBlock *> path;
  MachineLoop *mloop = MLI->getLoopFor(mbb);
  // a loop header phi
  assert(mloop && mloop->getHeader() == mbb);
  MachineBasicBlock *latchBB = mloop->getLoopLatch();
  assert(latchBB && "TODO:: handle multiple backedges not implemented yet");

  unsigned backedgePred = 0;
  for (MachineBasicBlock::pred_iterator ipred = mbb->pred_begin();
       ipred != mbb->pred_end(); ipred++) {
    MachineBasicBlock *inBB = *ipred;
    // this will cause lhdr BBPred get evaluated due to cycle
    unsigned edgePred = computeEdgePred(inBB, mbb, path);
    if (inBB == latchBB) {
      backedgePred = edgePred;
    }
  }
  assert(backedgePred);
  // incoming edge pred evaluation doesn't result in the bb pred get evaluated
  // due to nested loop
  if (!getBBPred(mbb)) {
    computeBBPred(mbb, path);
  }

  unsigned pb1 = MRI->createVirtualRegister(&CSA::I1RegClass);
  // filter 0 value backedge
  const unsigned switchOpcode = TII->makeOpcode(CSA::Generic::SWITCH, 1);
  MachineInstr *switchInst =
    BuildMI(*latchBB, latchBB->getFirstTerminator(), DebugLoc(),
            TII->get(switchOpcode), CSA::IGN)
      .addReg(pb1, RegState::Define)
      .addReg(backedgePred)
      .addImm(1);
  switchInst->setFlag(MachineInstr::NonSequential);

  // combine exit condition's pred
  unsigned orSrc        = 0;
  unsigned orResult     = 0;
  MachineInstr *orInstr = nullptr;
  SmallVector<MachineBasicBlock *, 4> exitingBlks;
  mloop->getExitingBlocks(exitingBlks);
  for (unsigned i = 0; i < exitingBlks.size(); ++i) {
    MachineBasicBlock *exitingBlk = exitingBlks[i];
    assert(exitingBlk->succ_size() == 2);
    MachineBasicBlock *succ1   = *exitingBlk->succ_begin();
    MachineBasicBlock *succ2   = *exitingBlk->succ_rbegin();
    MachineBasicBlock *exitBlk = mloop->contains(succ1) ? succ2 : succ1;
    unsigned ec                = computeEdgePred(exitingBlk, exitBlk, path);
    if (!orSrc) {
      orSrc = ec;
    } else if (!orResult) {
      orResult = MRI->createVirtualRegister(&CSA::I1RegClass);
      orInstr  = BuildMI(*latchBB, latchBB->getFirstTerminator(), DebugLoc(),
                        TII->get(CSA::OR1), orResult)
                  .addReg(orSrc)
                  .addReg(ec);
      orInstr->setFlag(MachineInstr::NonSequential);
    } else {
      unsigned newResult = MRI->createVirtualRegister(&CSA::I1RegClass);
      orInstr            = BuildMI(*latchBB, latchBB->getFirstInstrTerminator(),
                        DebugLoc(), TII->get(CSA::OR1), newResult)
                  .addReg(orResult)
                  .addReg(ec);
      orInstr->setFlag(MachineInstr::NonSequential);
      orResult = newResult;
    }
  }
  unsigned exitPred = orResult ? orResult : orSrc;

  // assign loop pred and its reset value
  const TargetRegisterClass *new_LIC_RC =
    LMFI->licRCFromGenRC(MRI->getRegClass(backedgePred));
  assert(new_LIC_RC && "Can't determine register class for register");
  unsigned loopPred = LMFI->allocateLIC(new_LIC_RC);

  MachineInstr *pickPB1 = BuildMI(*latchBB, latchBB->getFirstInstrTerminator(),
                                  DebugLoc(), TII->get(CSA::PICK1), loopPred)
                            .addReg(exitPred)
                            .addReg(pb1)
                            .addImm(0);
  pickPB1->setFlag(MachineInstr::NonSequential);
  const unsigned InitOpcode          = TII->getInitOpcode(&CSA::I1RegClass);
  MachineBasicBlock::iterator hdrloc = mbb->begin();
  // init loopPred 0;
  MachineInstr *predInit =
    BuildMI(*mbb, hdrloc, DebugLoc(), TII->get(InitOpcode), loopPred).addImm(0);
  predInit->setFlag(MachineInstr::NonSequential);
#if 1
  unsigned hdrPred = getBBPred(mbb);
  assert(hdrPred);
  MachineInstr *lorInstr = MRI->getVRegDef(hdrPred);
  // for multi entrance loop, this is the second lor with consolidated init
  // value
  assert(lorInstr->getOpcode() == CSA::LOR1);
  lorInstr->substituteRegister(lorInstr->getOperand(1).getReg(), loopPred, 0,
                               *TRI);
  // TODO: ????
  // filter 0 value init edge
  unsigned pi1 = MRI->createVirtualRegister(&CSA::I1RegClass);
  MachineInstr *switchInit =
    BuildMI(*lorInstr->getParent(), lorInstr, DebugLoc(),
            TII->get(switchOpcode), CSA::IGN)
      .addReg(pi1, RegState::Define)
      .addReg(lorInstr->getOperand(2).getReg())
      .addImm(1);
  lorInstr->substituteRegister(lorInstr->getOperand(2).getReg(), pi1, 0, *TRI);
#endif

  // fiiltering exit edge pred, only keep the value when exit condition is true
  for (unsigned i = 0; i < exitingBlks.size(); ++i) {
    MachineBasicBlock *exitingBlk = exitingBlks[i];
    assert(exitingBlk->succ_size() == 2);
    MachineBasicBlock *succ1   = *exitingBlk->succ_begin();
    MachineBasicBlock *succ2   = *exitingBlk->succ_rbegin();
    MachineBasicBlock *exitBlk = mloop->contains(succ1) ? succ2 : succ1;
    // switch off the exit edge value generated when exit condition is false;
    unsigned ec =
      getEdgePred(exitingBlk, CDG->getEdgeType(exitingBlk, exitBlk, true));
    unsigned ec1 = MRI->createVirtualRegister(&CSA::I1RegClass);
    MachineInstr *switchEC =
      BuildMI(*exitingBlk, exitingBlk->getFirstInstrTerminator(), DebugLoc(),
              TII->get(CSA::SWITCH1), CSA::IGN)
        .addReg(ec1, RegState::Define)
        .addReg(exitPred)
        .addReg(ec);
    switchEC->setFlag(MachineInstr::NonSequential);

    // pick exit edge value from 0 when loop is not executed, and loop exiting
    // value(ec1) when loop is executed using loop initial condition
    unsigned exitEdgePred = MRI->createVirtualRegister(&CSA::I1RegClass);
    MachineInstr *pickEC1 =
      BuildMI(*exitingBlk, exitingBlk->getFirstInstrTerminator(), DebugLoc(),
              TII->get(CSA::PICK1), exitEdgePred)
        .addReg(switchInit->getOperand(2).getReg())
        .addImm(0)
        .addReg(ec1);
    pickEC1->setFlag(MachineInstr::NonSequential);
    // reset the exit edge pred to the value after filtering
    setEdgePred(exitingBlk, CDG->getEdgeType(exitingBlk, exitBlk, true),
                exitEdgePred);
  }

  repeatOperandInLoopUsePred(mloop, predInit, pb1, exitPred);

  MachineBasicBlock::iterator iterI = mbb->begin();
  while (iterI != mbb->end()) {
    MachineInstr *MI = &*iterI;
    ++iterI;
    if (!MI->isPHI())
      continue;
    unsigned initInput     = 0;
    unsigned backedgeInput = 0;
    for (MIOperands MO(*MI); MO.isValid(); ++MO) {
      if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg()))
        continue;
      if (MO->isUse()) {
        unsigned Reg = MO->getReg();
        // move to its incoming block operand
        ++MO;
        MachineBasicBlock *inBB = MO->getMBB();
        if (inBB == latchBB) {
          backedgeInput = Reg;
        } else {
          initInput = Reg;
        }
      }
    }
    unsigned dst = MI->getOperand(0).getReg();
    // if we have two-way predMerge available, use predmerge/pick combination to
    // generated pick directly
    const TargetRegisterClass *TRC = MRI->getRegClass(dst);
    const unsigned pickOpcode      = TII->makeOpcode(CSA::Generic::PICK, TRC);
    BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst)
      .addReg(loopPred)
      .addReg(initInput)
      .addReg(backedgeInput);
    MI->removeFromParent();
  }
}

void CSACvtCFDFPass::generateDynamicPickTreeForFooter(MachineBasicBlock *mbb) {
  SmallVector<std::pair<unsigned, unsigned> *, 4> pred2values;
  unsigned predBB              = 0;
  MachineInstr *predMergeInstr = nullptr;
  MachineBasicBlock *pickFalseBB = nullptr;
  std::list<MachineBasicBlock *> path;
  MachineBasicBlock::iterator iterI = mbb->begin();
  while (iterI != mbb->end()) {
    MachineInstr *MI = &*iterI;
    ++iterI;
    if (!MI->isPHI())
      continue;
    // not a loop header phi
    assert(!MLI->getLoopFor(mbb) || MLI->getLoopFor(mbb)->getHeader() != mbb);
    pred2values.clear();
    for (MIOperands MO(*MI); MO.isValid(); ++MO) {
      if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg()))
        continue;
      if (MO->isUse()) {
        unsigned Reg = MO->getReg();
        // move to its incoming block operand
        ++MO;
        MachineBasicBlock *inBB = MO->getMBB();
        unsigned edgePred       = computeEdgePred(inBB, mbb, path);
        std::pair<unsigned, unsigned> *pred2value =
          new std::pair<unsigned, unsigned>;
        pred2value->first  = edgePred;
        pred2value->second = Reg;
        pred2values.push_back(pred2value);
        // merge incoming edge pred to generate BB pred
        if (!predBB) {
          predBB = edgePred;
        } else if (MI->getNumOperands() == 5) {
          // two input phi: use PREDMERGE to avoid further lowering.
          predMergeInstr =
            getOrInsertPredMerge(mbb, MI, predBB, // last processed edge
                                 edgePred);       // current edge
          if (!pickFalseBB)
            pickFalseBB = MI->getOperand(2).getMBB();
        }
      }
    } // end of for MO

    unsigned dst = MI->getOperand(0).getReg();
    // if we have two-way predMerge available, use predmerge/pick combination to
    // generated pick directly
    if (predMergeInstr) {
      assert(MI->getNumOperands() == 5);
      unsigned reg1                  = MI->getOperand(1).getReg();
      unsigned reg2                  = MI->getOperand(3).getReg();
      // Swap the registers if the order of basic blocks is reversed.
      if (MI->getOperand(2).getMBB() != pickFalseBB)
        std::swap(reg1, reg2);
      const TargetRegisterClass *TRC = MRI->getRegClass(reg1);
      unsigned pickPred              = predMergeInstr->getOperand(1).getReg();
      const unsigned pickOpcode      = TII->makeOpcode(CSA::Generic::PICK, TRC);
      BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst)
        .addReg(pickPred)
        .addReg(reg1)
        .addReg(reg2);
    } else {
#if 0
      MachineInstr* xphi = nullptr;
      //TODO::generated xphi sequence
      for (unsigned i = 0; i < pred2values.size(); i++) {
        std::pair<unsigned, unsigned>* pred2value = pred2values[i];
        if (i == 0) {
          xphi = BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(CSA::XPHI), dst).addReg(pred2value->first).addReg(pred2value->second);
        }
        else {
          MachineOperand edgeOp = MachineOperand::CreateReg(pred2value->first, true);
          MachineOperand valueOp = MachineOperand::CreateReg(pred2value->second, true);
          xphi->add(edgeOp);
          xphi->add(valueOp);
        }
      }
#else
      CombineDuplicatePhiInputs(pred2values, MI);
      LowerXPhi(pred2values, MI);
#endif
    }
    // release memory
    for (unsigned i = 0; i < pred2values.size(); i++) {
      std::pair<unsigned, unsigned> *pred2value = pred2values[i];
      delete pred2value;
    }
    MI->removeFromParent();
  }
}

void CSACvtCFDFPass::CombineDuplicatePhiInputs(
  SmallVectorImpl<std::pair<unsigned, unsigned> *> &pred2values,
  MachineInstr *iPhi) {
  unsigned pairsLen = pred2values.size();
  for (unsigned i = 0; i < pairsLen; i++) {
    std::pair<unsigned, unsigned> *pair1 = pred2values[i];
    for (unsigned j = i + 1; j < pairsLen;) {
      std::pair<unsigned, unsigned> *pair2 = pred2values[j];
      if (pair1->second == pair2->second) {
        unsigned orResult     = MRI->createVirtualRegister(&CSA::I1RegClass);
        MachineInstr *orInstr = BuildMI(*iPhi->getParent(), iPhi, DebugLoc(),
                                        TII->get(CSA::OR1), orResult)
                                  .addReg(pair1->first)
                                  .addReg(pair2->first);
        orInstr->setFlag(MachineInstr::NonSequential);
        pair1->first = orResult;
        // remove pair2
        delete pair2;
        for (unsigned k = j; k < pairsLen - 1; k++) {
          pred2values[k] = pred2values[k + 1];
        }
        pairsLen--;
      } else {
        j++;
      }
    }
  }
  pred2values.set_size(pairsLen);
}

void CSACvtCFDFPass::LowerXPhi(
  SmallVectorImpl<std::pair<unsigned, unsigned> *> &pred2values,
  MachineInstr *loc) {
  if (pred2values.empty() || pred2values.size() == 1)
    return;
  SmallVector<std::pair<unsigned, unsigned> *, 4> vpair;
  unsigned j = pred2values.size() - 1;
  unsigned i = 0;
  while (i <= j) {
    if (i == j) {
      // singular
      vpair.push_back(pred2values[i]);
    } else {
      std::pair<unsigned, unsigned> *pair1 = pred2values[i];
      std::pair<unsigned, unsigned> *pair2 = pred2values[j];
      // const TargetRegisterClass *pTRC = MRI->getRegClass(pair1->first);
      // MachineInstr* predMerge = getOrInsertPredMerge(loc->getParent(), loc,
      // pair1->first, pair2->first);

      unsigned indexReg  = MRI->createVirtualRegister(&CSA::I1RegClass);
      unsigned bbpredReg = MRI->createVirtualRegister(&CSA::I1RegClass);
      BuildMI(*loc->getParent(), loc, DebugLoc(), TII->get(CSA::PREDMERGE),
              bbpredReg)
        .addReg(indexReg, RegState::Define)
        .addReg(pair1->first)
        .                     // last processed edge
        addReg(pair2->first); // current edge

      const TargetRegisterClass *vTRC = MRI->getRegClass(pair1->second);
      const unsigned pickOpcode = TII->makeOpcode(CSA::Generic::PICK, vTRC);
      unsigned pickDst;
      if (pred2values.size() == 2) {
        pickDst = loc->getOperand(0).getReg();
      } else {
        pickDst = MRI->createVirtualRegister(vTRC);
      }
      BuildMI(*loc->getParent(), loc, loc->getDebugLoc(), TII->get(pickOpcode),
              pickDst)
        .addReg(indexReg)
        .addReg(pair1->second)
        .addReg(pair2->second);
      pair1->first  = bbpredReg;
      pair1->second = pickDst;
      if (pred2values.size() > 2) {
        vpair.push_back(pair1);
      }
    }
    ++i;
    --j;
  }
  if (vpair.size() > 1) {
    LowerXPhi(vpair, loc);
  }
}


void CSACvtCFDFPass::replaceIfFooterPhiSeq() {
  typedef po_iterator<MachineBasicBlock *> po_cfg_iterator;
  MachineBasicBlock *root = &*thisMF->begin();
  for (po_cfg_iterator itermbb = po_cfg_iterator::begin(root),
                       END     = po_cfg_iterator::end(root);
       itermbb != END; ++itermbb) {
    MachineBasicBlock *mbb = *itermbb;
    generateCompletePickTreeForPhi(mbb);
  }
}

// make sure phi block post dominates all control points of all its inBBs
bool CSACvtCFDFPass::CheckPhiInputBB(MachineBasicBlock *inBB,
                                     MachineBasicBlock *mbb) {
  if (DT->dominates(inBB, mbb)) {
    return PDT->dominates(mbb, inBB);
  }
  ControlDependenceNode *inNode = CDG->getNode(inBB);
  unsigned numCtrl              = 0;
  for (ControlDependenceNode::node_iterator pnode = inNode->parent_begin(),
                                            pend  = inNode->parent_end();
       pnode != pend; ++pnode) {
    ControlDependenceNode *ctrlNode = *pnode;
    MachineBasicBlock *ctrlBB       = ctrlNode->getBlock();

    // Ignore loop-related control dependences.
    if (MLI->getLoopFor(ctrlBB) &&
        MLI->getLoopFor(ctrlBB)->isLoopExiting(ctrlBB))
      continue;

    ++numCtrl;
    if (numCtrl > 1)
      return false;
    if (!PDT->dominates(mbb, ctrlBB)) {
      return false;
    }
    if (!CheckPhiInputBB(ctrlBB, mbb)) {
      return false;
    }
  }
  return true;
}

/* Find all implicitly defined vregs. These are problematic with dataflow
 * conversion: LLVM will automatically expand them to registers (LICs, in our
 * case). While registers can be read without any value previously having been
 * written, LICs are different. We must replace the undef with a read from
 * %IGN, equivalent to reading 0. Note that we can do this even if we're not
 * sure that the instructions in question will be successfully converted to
 * data flow. Returns a boolean indicating modification.
 */
bool CSACvtCFDFPass::replaceUndefWithIgn() {
  bool modified            = false;
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  const CSAInstrInfo &TII  = *static_cast<const CSAInstrInfo *>(
    thisMF->getSubtarget<CSASubtarget>().getInstrInfo());
  SmallPtrSet<MachineInstr *, 4> implicitDefs;
  LLVM_DEBUG(errs() << "Finding implicit defs:\n");
  for (MachineFunction::iterator BB = thisMF->begin(); BB != thisMF->end();
       ++BB) {
    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      MachineInstr *MI = &*I;
      // We're looking for instructions like '%vreg26<def> = IMPLICIT_DEF;'.
      if (MI->isImplicitDef()) {
        implicitDefs.insert(MI);
        LLVM_DEBUG(errs() << "\tFound: " << *MI);
      }
    }
  }

  if (implicitDefs.empty()) {
    LLVM_DEBUG(errs() << "(No implicit defs found.)\n");
  }

  for (SmallPtrSet<MachineInstr *, 4>::iterator I = implicitDefs.begin(),
                                                E = implicitDefs.end();
       I != E; ++I) {
    MachineInstr *uMI  = *I;
    MachineOperand uMO = uMI->getOperand(0);
    // Ensure we're dealing with a register definition.
    assert(uMO.isDef() && uMO.isReg());
    (void) uMO;
    // Ensure SSA form and that we have right defining instruction.
    assert(MRI->getUniqueVRegDef(uMO.getReg()) &&
           MRI->getUniqueVRegDef(uMO.getReg()) == uMI);
    const TargetRegisterClass *TRC =
      MRI->getRegClass(uMI->getOperand(0).getReg());
    const unsigned moveOpcode = TII.getMoveOpcode(TRC);
    BuildMI(*uMI->getParent(), uMI, DebugLoc(), TII.get(moveOpcode),
            uMI->getOperand(0).getReg())
      .addImm(0);
    // Erase the implicit definition.
    uMI->removeFromParent();
    modified = true;
  }

  LLVM_DEBUG(errs() << "Finished converting implicit defs to %IGN reads.\n\n");
  return modified;
}

void CSACvtCFDFPass::nameLIC(unsigned vreg, const Twine &prefix,
    unsigned baseReg, const Twine &infix,
    const MachineBasicBlock *containingBlock, const Twine &suffix) {
  // Ensure that the base register has a printable name.
  if (baseReg && LMFI->getLICName(baseReg).empty())
    LMFI->setLICName(baseReg,
        "lic" + Twine(TargetRegisterInfo::virtReg2Index(vreg)));

  if (vreg && LMFI->getLICName(vreg).empty())
  LMFI->setLICName(vreg,
      prefix +
      (baseReg ? LMFI->getLICName(baseReg) : "") +
      infix +
      (containingBlock ? containingBlock->getName() : "") +
      suffix);
}
