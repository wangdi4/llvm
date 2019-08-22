//===-- CSACvtCFDFPass.cpp - CSA convert control flow to data flow --------===//
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
#include "llvm/Analysis/CFG.h"
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
INITIALIZE_PASS_DEPENDENCY(CSALoopInfoPass)
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

  PredMergeTrees.clear();
  EdgePredicates.clear();
  BlockPredicates.clear();
  bb2rpo.clear();
  multiInputsPick.clear();
  loopInfo.clear();
}

void CSACvtCFDFPass::replacePhiWithPICK() {
  for (auto Loop : *MLI)
    generateLoopHeader(Loop);
  replaceIfFooterPhiSeq();
}


bool CSACvtCFDFPass::runOnMachineFunction(MachineFunction &MF) {
  if (!shouldRunDataflowPass(MF))
    return false;


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

  if (csa_utils::isAlwaysDataFlowLinkageSet()) {
    generateEntryInstr();
    generateContinueInstrs();
  }
  generateSingleReturn();
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

  // Until we support pickany-based PHI conversion, we cannot support
  // irreducible graphs.
  if (containsIrreducibleCFG<MachineBasicBlock *, decltype(*RPOT),
      MachineLoopInfo>(*RPOT, *MLI)) {
    report_fatal_error("Cannot compile irreducible CFGs for CSA");
  }

  // Insert switches.
  switchNormalRegisters();

  // At this point, the code should obey the property that the define and uses
  // of all registers are in the same control-dependent region. We don't assert
  // that at this point, although there is effectively an assert of this
  // property in assignLicFrequencies later on.

  LLVM_DEBUG(dbgs() << "Function after switch generation:\n"; MF.print(dbgs()));

  // We are now exiting SSA mode.
  MRI->leaveSSA();
  MF.getProperties().set(MachineFunctionProperties::Property::NoPHIs);

  if (needDynamicPreds() || UseDynamicPred) {
    computeBlockPredicates();
    generateDynamicPreds();
  } else {
    replacePhiWithPICK();
  }

  processLoops();

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

  // Lower LIC queue intrinsics here. This prevents us from trying to add
  // switch/picks to the code in question.
  lowerLicQueue();

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

static MachineInstr *getEntryPseudoMI(MachineFunction *MF) {
  for (MachineFunction::iterator MBB = MF->begin(); MBB != MF->end(); MBB++) {
    for (MachineBasicBlock::iterator I = MBB->begin(); I != MBB->end(); I++) {
      MachineInstr *MI = &*I;
      if (MI->getOpcode() == CSA::CSA_ENTRYPSEUDO)
        return MI;
    }
  }
  assert(false && "Pseudo entry instruction not found!!");
  return nullptr;
}

// During ISelLowering, Function entry points are lowered to CSA_ENTRYPSEUDO
// followed by a list of CSA_GETVAL* instructions, one for each input argument
// This function merges these PSEUDO instruction to generate a single CSA_ENTRY instruction
void CSACvtCFDFPass::generateEntryInstr() {
  MachineInstr *EntryPseudoMI = getEntryPseudoMI(thisMF);
  MachineInstrBuilder MIB =
    BuildMI(*(EntryPseudoMI->getParent()), EntryPseudoMI, EntryPseudoMI->getDebugLoc(),
            TII->get(CSA::CSA_ENTRY));
  // Parsing through instructions to find the related GETVALs
  unsigned reg = EntryPseudoMI->getOperand(0).getReg();
  auto nextUI = MRI->use_begin(reg);
  for (auto UI = MRI->use_begin(reg), UE = MRI->use_end(); UI != UE; UI = nextUI) {
    MachineInstr *MI = UI->getParent();
    assert(TII->getGenericOpcode(MI->getOpcode()) == CSA::Generic::CSA_GETVAL);
    ++UI;
    nextUI = UI;
    MIB.addDef(MI->getOperand(0).getReg());
    MI->eraseFromParent();
  }
  MIB.setMIFlag(MachineInstr::NonSequential);
  LMFI->setEntryMI(&*MIB);
  EntryPseudoMI->eraseFromParent();
}

// During ISelLowering, call-sites are lowered to CSA_CALL and CSA_CONTINUEPSEUDO
// followed by a list of CSA_GETVAL* instructions, one for each returned value
// This function merges these PSEUDO instruction to generate a single CSA_CONTINUE instruction
void CSACvtCFDFPass::generateContinueInstrs() {
  // Loop through all instructions and find sites to include CSA_CONTINUE
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end();
       BB != E; ++BB) {
    for (MachineBasicBlock::iterator I = BB->begin(), EI = BB->end(); I != EI;) {
      MachineInstr *ContinuePseudoMI = &*I;
      if (ContinuePseudoMI->getOpcode() == CSA::CSA_CONTINUEPSEUDO) {
        MachineInstrBuilder MIB =
          BuildMI(*(ContinuePseudoMI->getParent()), ContinuePseudoMI, ContinuePseudoMI->getDebugLoc(),
              TII->get(CSA::CSA_CONTINUE));
        // Parsing through instructions to find the related GETVALs
        unsigned reg = ContinuePseudoMI->getOperand(0).getReg();
        auto nextUI = MRI->use_begin(reg);
        for (auto UI = MRI->use_begin(reg), UE = MRI->use_end(); UI != UE; UI = nextUI) {
          MachineInstr *MI = UI->getParent();
          ++UI;
          nextUI = UI;
          if (TII->getGenericOpcode(MI->getOpcode()) != CSA::Generic::CSA_GETVAL) continue;
          MIB.addDef(MI->getOperand(0).getReg());
          MI->eraseFromParent();
        }
        // End of parsing through instructions to find the related GETVALs
        MIB.setMIFlag(MachineInstr::NonSequential);
        ContinuePseudoMI->eraseFromParent();
        MachineBasicBlock::iterator nextI(&*MIB);
        I = nextI;
      } else
        ++I;
    }
  }
}

void CSACvtCFDFPass::generateSingleReturn() {
  // Gather all return instructions in the function. Note that a block
  // immediately postdominated by the exit pseudonode might not correspond to a
  // return node. Calls to noreturn functions or infinite loops will generate
  // fake edges to this node for the purposes of postdomination, and we can
  // ignore those situations.
  SmallVector<MachineInstr *, 4> ReturnInsts;
  bool SeenSXURet = false;
  for (auto ChildNode : PDT->getRootNode()->getChildren()) {
    MachineBasicBlock *ChildBB = ChildNode->getBlock();
    auto Terminator = ChildBB->getFirstInstrTerminator();
    if (Terminator == ChildBB->end())
      continue;
    if (Terminator->getOpcode() == CSA::CSA_RETURN)
      ReturnInsts.push_back(&*Terminator);
    else if (!csa_utils::isAlwaysDataFlowLinkageSet() &&
        Terminator->getOpcode() == CSA::RET) {
      if (SeenSXURet) {
        report_fatal_error("Cannot handle multiple returns with legacy RET");
      }
      SeenSXURet = true;
    }
  }

  if (SeenSXURet)
    return;

  if (ReturnInsts.empty()) {
    // We have no return instructions. Our function ought to be noreturn.
    //report_fatal_error("Need clarification on how to handle noreturn");
    // TODO: Handle return more cleanly?
    MachineBasicBlock &MBB = *thisMF->begin();
    unsigned DummyOutReg = LMFI->allocateLIC(&CSA::CI0RegClass);
    BuildMI(MBB, MBB.end(), DebugLoc{}, TII->get(CSA::MOV0), DummyOutReg)
      .addUse(CSA::NA);
    MachineInstr *RetInstr = BuildMI(MBB, MBB.end(), DebugLoc{},
        TII->get(CSA::CSA_RETURN))
      .addUse(DummyOutReg);
    LMFI->setReturnMI(RetInstr);
  } else if (ReturnInsts.size() > 1) {
    // We have multiple return instructions. Make a basic block that generates
    // a PHI of all of the used values.
    LLVM_DEBUG(dbgs() << "Inserting new basic block to merge returns.");
    MachineBasicBlock *RetBB = thisMF->CreateMachineBasicBlock();
    thisMF->insert(thisMF->end(), RetBB);
    for (MachineInstr *RetInstr : ReturnInsts) {
      RetInstr->getParent()->addSuccessor(RetBB);
    }

    // Collect PHIs for all of the operands.
    auto NewRet = BuildMI(*RetBB, RetBB->end(), ReturnInsts[0]->getDebugLoc(),
        TII->get(CSA::CSA_RETURN));

    unsigned NumOperands = ReturnInsts[0]->getNumOperands();
    for (unsigned i = 0; i < NumOperands; i++) {
      const MachineOperand &Op = ReturnInsts[0]->getOperand(i);
      assert(Op.getReg() && "CSA_RETURN needs register operands");
      unsigned Reg = Op.getReg();
      assert(TargetRegisterInfo::isVirtualRegister(Reg) &&
        "CSA_RETURN should not have physical registers");

      // Add the result register for the PHI.
      unsigned ResultReg = MRI->createVirtualRegister(MRI->getRegClass(Reg));
      NewRet.addReg(ResultReg);

      // Create a PHI using all of the input instructions.
      auto NewPHI = BuildMI(*RetBB, NewRet.getInstr(), DebugLoc{},
          TII->get(CSA::PHI), ResultReg);
      for (MachineInstr *MI : ReturnInsts) {
        NewPHI.add(MI->getOperand(i));
        NewPHI.add(MachineOperand::CreateMBB(MI->getParent()));
      }
    }

    // Replace all of the return instructions with branches.
    for (MachineInstr *RetInstr : ReturnInsts) {
      RetInstr->setDesc(TII->get(CSA::BR));
      for (unsigned i = 0; i < NumOperands; i++)
        RetInstr->RemoveOperand(0);
      RetInstr->addOperand(MachineOperand::CreateMBB(RetBB));
    }

    // Update our datastructures with the new basic block. The immediate post
    // dominator is the pseudo exit node; the immediate dominator is the mutual
    // post dominator of the control-dependence region of the entry node.
    CDGRegion *EntryRegion = CDG->getRegion(DT->getRoot());
    // PDT->addNewBlock(RetBB, PDT->getRoot()); -- this isn't exposed?
    DT->addNewBlock(RetBB, EntryRegion->nodes.back());
    CDG->addNewBlock(RetBB, EntryRegion);

    // Recalculate the frequency information.
    getAnalysis<MachineBlockFrequencyInfo>().calculate(*thisMF,
        getAnalysis<MachineBranchProbabilityInfo>(), *MLI);

    // Set the LMFI information for the return instruction.
    LMFI->setReturnMI(NewRet);
  } else {
    // We have just one return instruction. Tell LMFI about it.
    LMFI->setReturnMI(ReturnInsts[0]);
  }
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
    StringRef Name = LMFI->getLICName(Reg);
    if (!Name.empty()) {
      LMFI->setLICName(switchTrueReg, Name);
      LMFI->setLICName(switchFalseReg, Name);
    }
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
  auto getRegOnEdge = [&](MachineBasicBlock *Parent,
                          MachineBasicBlock *Child) -> unsigned {
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
          MBB->empty() ? DebugLoc() : MBB->front().getDebugLoc(),
          TII->get(TargetOpcode::PHI), NewReg);
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

void CSACvtCFDFPass::processLoops() {
  for (auto Loop : *MLI)
    processLoop(Loop);
}

void CSACvtCFDFPass::processLoop(MachineLoop *L) {
  for (auto Subloop : *L)
    processLoop(Subloop);

  CSALoopInfo &DFLoop = loopInfo[L];

  // Attempt to pipeline the loop.
  unsigned pipeliningDegree = getInnerLoopPipeliningDegree(L);
  if (pipeliningDegree > 1) {
    // The completionN operators we will insert are limited to a maximum depth
    // of 2**8-1==255 by the VISA.
    unsigned numTokens = std::min(255U, pipeliningDegree);

    // ...and they're further limited to a maximum depth of 64 according to V1
    // expectations. This limit will seemingly be exposed to the vISA.
    numTokens = std::min(64U, numTokens);

    assert(DFLoop.getNumExits() == 1 &&
      "Can only pipeline loops with single exit blocks");
    bufferPipelinedLoopBypass(L, numTokens);
    pipelineLoop(L->getHeader(), DFLoop, numTokens);
  }

  // Annotate all the edges on the pick backedge with csasim_backedge
  auto PickBackedge = DFLoop.getPickBackedgeIndex();
  for (MachineInstr *Pick : DFLoop.getHeaderPicks()) {
    unsigned PickReg = Pick->getOperand(2 + PickBackedge).getReg();
    LMFI->addLICAttribute(PickReg, "csasim_backedge");
  }

  // If pipeline depth has been specified, add token control
  limitPipelineDepth(L);

  // Move the loop into the LoopInfo for use after dataflow conversion.
  // Skip pipelined loops for now--we generally can't do later optimizations on
  // them anyways because the iterations are out of order.
  if (pipeliningDegree <= 1)
    getAnalysis<CSALoopInfoPass>().addLoop(std::move(DFLoop));
}

void CSACvtCFDFPass::limitPipelineDepth(MachineLoop *L)
{
  // If pipeline depth has been specified, add token control
  MachineInstr *tokenTake = 0;
  MachineInstr *tokenReturn = 0;
  if (checkIfDepthLimitedLoop(L, tokenTake, tokenReturn)) {
    auto frameSize = tokenTake->getOperand(3).getImm();
    auto pipelineDepth = tokenTake->getOperand(4).getImm();
    LLVM_DEBUG(dbgs() <<
      "Retrieved pipeline depth-limited loop with pool address " <<
      printReg(tokenTake->getOperand(1).getReg()) <<
      ", frameSize " << frameSize <<
      ", depth " << pipelineDepth <<
      " setting " << printReg(tokenTake->getOperand(0).getReg()) << "\n");

    unsigned frameOffset =
      LMFI->allocateLIC(&CSA::CI64RegClass, "pd.ls.offsets");
    LMFI->setLICDepth(frameOffset, pipelineDepth);


    // Initialize slot-offsets lic with frame offsets
    // The initial values are:
    //    slot-offsets = offset_of(slot0), ..., offset_of(slot'n-1)
    MachineBasicBlock &LH = *tokenTake->getParent();
    auto DebugLoc = L->getStartLoc();
    for (int i = 0; i < pipelineDepth; ++i) {
      BuildMI(LH, tokenTake, DebugLoc, TII->get(CSA::INIT64))
        .addDef(frameOffset).addImm(i*frameSize);
    }

    // TOKEN_TAKE looks like this:
    // slot, take_outord = CSA_PIPELINE_DEPTH_TOKEN_TAKE pool, slot-size, depth, take_inord

    // Convert token_take to an ADD to generate frame address
    // pool_gated = GATE64 take_inord, pool
    // slot-addr = ADD64 pool_gated, slot-offsets
    // take_outord = MOV0 slot-addr
    unsigned take_inord = tokenTake->getOperand(5).getReg();
    unsigned take_outord = tokenTake->getOperand(1).getReg();
    unsigned pool = tokenTake->getOperand(2).getReg();
    unsigned slot = tokenTake->getOperand(0).getReg();
    unsigned pool_gated =
      LMFI->allocateLIC(&CSA::CI64RegClass, "pd.ls.pool.take");
    BuildMI(LH, tokenTake, DebugLoc, TII->get(CSA::GATE64), pool_gated)
      .addReg(take_inord)
      .addReg(pool);
    BuildMI(LH, tokenTake, DebugLoc, TII->get(CSA::ADD64), slot)
      .addReg(pool_gated)
      .addReg(frameOffset);
    BuildMI(LH, tokenTake, DebugLoc, TII->get(CSA::MOV0), take_outord)
      .addReg(slot);

    // TOKEN_RETURN looks like this:
    // ret_outord = CSA_PIPELINE_DEPTH_TOKEN_RETURN pool, slot-addr, ret_inord
    // Return a frame slot to the pool
    // slot-offsets = GATE64 ret_inord, slot-offsets
    // ret_outord = MOV0 ret_inord
    MachineBasicBlock &LL = *tokenReturn->getParent();
    unsigned ret_inord = tokenReturn->getOperand(3).getReg();
    unsigned ret_outord = tokenReturn->getOperand(0).getReg();
    BuildMI(LL, tokenReturn, DebugLoc, TII->get(CSA::GATE64), frameOffset)
      .addReg(ret_inord).addReg(frameOffset);
    BuildMI(LL, tokenReturn, DebugLoc, TII->get(CSA::MOV0), ret_outord)
      .addReg(ret_inord);

    // Delete pseudoinstrs
    tokenTake->eraseFromParent();
    tokenReturn->eraseFromParent();
  }
}

void CSACvtCFDFPass::generateLoopHeader(MachineLoop *Loop) {
  for (auto Subloop : *Loop)
    generateLoopHeader(Subloop);

  MachineBasicBlock *Header = Loop->getHeader();
  auto DebugLoc = Loop->getStartLoc();
  CSALoopInfo &DFLoop = loopInfo[Loop];

  unsigned LoopExitPredicate = LMFI->allocateLIC(&CSA::CI1RegClass);
  MachineBasicBlock *ExitingBB = Loop->getExitingBlock();
  if (ExitingBB && CDG->getRegion(ExitingBB) == CDG->getRegion(Header)) {
    // When there is only one exiting block, then we select a new loop iteration
    // when the previous iteration exits.
    MachineBasicBlock *ExitBB = Loop->getExitBlock();
    assert(ExitBB && "How do we have two exit blocks with one exiting block?");

    unsigned ExitIndex = getSwitchIndexForEdge(ExitingBB, ExitBB);
    DFLoop.setPickBackedgeIndex(1 - ExitIndex);
    // Generate a MOV so that only the header picks see the initial value.
    BuildMI(*ExitingBB, ExitingBB->getFirstTerminator(), DebugLoc,
        TII->get(CSA::MOV1), LoopExitPredicate)
      .add(ExitingBB->getFirstTerminator()->getOperand(0));
  } else {
    DFLoop.setPickBackedgeIndex(1);

    // For now, fail to handle this kind of condition. It should be dealt with
    // by the dynamic predication selector.
    assert(false && "This condition is not handlable at the moment");
  }

  bool InitialIsFirst = DFLoop.getPickInitialIndex() == 0;

  LLVM_DEBUG(dbgs() << "Inserting header for BB#" << Header->getNumber() <<
      ", loop back when " << printReg(LoopExitPredicate) << " == " <<
      DFLoop.getPickBackedgeIndex() << "\n");

  // Generate the initial value for the initial PHIs in the loop.
  auto InsertPoint = Header->getFirstNonPHI();
  BuildMI(*Header, InsertPoint, DebugLoc, TII->get(CSA::INIT1),
      LoopExitPredicate)
    .addImm(DFLoop.getPickInitialIndex());

  unsigned NumLatches = 0;
  for (auto Pred : Header->predecessors())
    NumLatches += Loop->contains(Pred);
  unsigned NumInitial = Header->pred_size() - NumLatches;
  assert(NumLatches >= 1 && NumInitial >= 1 && "How does this loop work?");

  // Partition the PHIs in the header between incoming and latch edges.
  SmallVector<MachineInstr *, 16> ToDelete;
  for (auto &PHI : make_range(Header->begin(), Header->getFirstNonPHI())) {
    unsigned DestReg = PHI.getOperand(0).getReg();
    auto RegClass = MRI->getRegClass(DestReg);
    unsigned LatchReg, InitialReg;
    MachineInstr *LatchPHI = nullptr, *InitialPHI = nullptr;

    // Set the partitions appropriately.
    auto buildPHI = [&](unsigned &OutReg, MachineInstr *&OutPhi) {
      OutReg = LMFI->allocateLIC(RegClass);
      OutPhi = BuildMI(*Header, PHI, PHI.getDebugLoc(),
        TII->get(CSA::PHI), OutReg);
    };
    if (NumLatches > 1)
      buildPHI(LatchReg, LatchPHI);
    if (NumInitial > 1)
      buildPHI(InitialReg, InitialPHI);

    // Assign values of the PHI operand into the latch or initial PHI/reg as
    // necessary.
    for (unsigned O = 1, E = PHI.getNumOperands(); O != E; O += 2) {
      MachineOperand &InOp = PHI.getOperand(O);
      MachineBasicBlock *Pred = PHI.getOperand(O + 1).getMBB();

      if (Loop->contains(Pred)) {
        if (NumLatches == 1)
          LatchReg = InOp.getReg();
        else {
          LatchPHI->addOperand(InOp);
          LatchPHI->addOperand(MachineOperand::CreateMBB(Pred));
        }
      } else {
        if (NumInitial == 1)
          InitialReg = InOp.getReg();
        else {
          InitialPHI->addOperand(InOp);
          InitialPHI->addOperand(MachineOperand::CreateMBB(Pred));
        }
      }
    }

    if (InitialPHI)
      LLVM_DEBUG(dbgs() << "  Adding for initial: " << PHI);
    if (LatchPHI)
      LLVM_DEBUG(dbgs() << "  Added for latch: " << PHI);

    // Pick between the initial and the latch.
    auto Pick = BuildMI(*Header, InsertPoint, PHI.getDebugLoc(),
        TII->get(TII->makeOpcode(CSA::Generic::PICK, RegClass)), DestReg)
      .addReg(LoopExitPredicate)
      .addReg(InitialIsFirst ? InitialReg : LatchReg)
      .addReg(InitialIsFirst ? LatchReg : InitialReg);
    DFLoop.addHeaderPick(Pick);
    LLVM_DEBUG(dbgs() << "  Inserted pick: " << *Pick);

    // Delete the old PHI.
    ToDelete.push_back(&PHI);
  }

  for (auto PHI : ToDelete)
    PHI->eraseFromParent();
}

// This version uses additional operators in order to allow multiple incoming
// "gangs" of data to flow through the loop at once. The number of gangs
// allowed to be in the pipeline at once is determined by the "completion"
// buffer operators: no new gangs will be admitted if these do not have
// available storage to reorder the loop's outputs. The number of "gangs"
// admitted is bounded on two sides: it is not correct to admit more than we
// have backedge and completion buffering for, and it does not increase
// performance to admit more than the pipeline depth of the body would fit.
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
    unsigned orderedOut = g ? g->getReg() : Register(CSA::IGN);
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

    // This is also a backedge from a dataflow perspective.
    LMFI->addLICAttribute(newToken, "csasim_backedge");

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

void CSACvtCFDFPass::bufferPipelinedLoopBypass(MachineLoop *L, unsigned Depth) {
  // Find the parent of this block in the control dependence graph.
  MachineBasicBlock *Header = L->getHeader();
  ControlDependenceNode *HeaderNode = CDG->getNode(Header);
  ControlDependenceNode *CDParent = nullptr;
  for (auto Parent = HeaderNode->parent_begin(), E = HeaderNode->parent_end();
      Parent != E; ++Parent) {
    MachineBasicBlock *MBB = (*Parent)->getBlock();
    if (L->contains(MBB))
      continue;
    if (CDParent) {
      LLVM_DEBUG(dbgs()<< "Too many control dependences for loop header, "
          "skipping buffering\n");
      return;
    }
    CDParent = *Parent;
  }
  assert(CDParent &&
      "How do we have an inner loop pipeline with nothing outside?");
  ControlDependenceNode *CDChild = HeaderNode;
  do {
    // Get the index of the switch results that corresponds to the bypass of the
    // loop--the direction *other* than that indicated by the control
    // dependence.
    unsigned OpNum = CDParent->isTrueChild(CDChild) ? 0 : 1;
    for (auto SwitchPair : *GenSwitches[CDParent->getBlock()]) {
      MachineInstr *Switch = SwitchPair.second;
      unsigned Reg = Switch->getOperand(OpNum).getReg();
      if (Reg == CSA::IGN)
        continue;
      LMFI->setLICDepth(Reg, Depth);
      LLVM_DEBUG(dbgs() << "Adding buffering to " << printReg(Reg) <<
          " since it is bypassing a pipelined loop.\n");
    }

    // Crawl up the parent chain.
    CDChild = CDParent;
    CDParent = *CDParent->parent_begin();
  } while (CDChild->getNumParents() == 1 && CDParent->getNumParents() != 0);
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
  assert(id.getNumDefs() == 1 &&
         "Must have exactly one output to use createUseTree");

  const TargetRegisterClass *outTRC = LMFI->licRCFromGenRC(
    TII->getRegClass(id, 0, TRI, *mbb->getParent()));

  // If the instruction is variadic, we don't really need to make a tree.
  if (id.isVariadic()) {
    MachineInstrBuilder MIB = BuildMI(
        *mbb, before, before == mbb->end() ? DebugLoc() : before->getDebugLoc(),
        TII->get(opcode), LMFI->allocateLIC(outTRC));
    MIB.setMIFlag(MachineInstr::NonSequential);
    for (unsigned j = 0; j < n; ++j)
      MIB.addUse(vals[j]->getReg());
    if (created)
      created->push_back(MIB);
    return &MIB->getOperand(0);
  }

  // Ensure that this is an op that we can build some kind of 1-to-N tree out
  // of.
  assert(id.getNumOperands() > 2 &&
         "Don't know how to build a tree out of this opcode");
  unsigned radix                    = id.getNumOperands() - id.getNumDefs();

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
        if (Op.isDef() && MRI->use_empty(Reg)
            && (MI.getOpcode() != CSA::CSA_ENTRY)
            && (MI.getOpcode() != CSA::CSA_CONTINUE))
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
  MachineBasicBlock *Entry = &*thisMF->begin();
  for (auto SI = Entry->succ_begin(); SI != Entry->succ_end(); ) {
    SI = Entry->removeSuccessor(SI);
  }
  for (MachineBasicBlock *MBB : *RPOT) {
    if (MBB == Entry)
      continue;
    Entry->splice(Entry->end(), MBB, MBB->begin(), MBB->end());
    MBB->eraseFromParent();
  }

  // Move the return instruction to the end.
  MachineInstr *ReturnMI = LMFI->getReturnMI();
  if (ReturnMI) {
    ReturnMI->removeFromParent();
    Entry->insert(Entry->end(), ReturnMI);
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

static unsigned &getEdge(std::pair<unsigned, unsigned> &Pair,
    ControlDependenceNode::EdgeType ChildType) {
  switch (ChildType) {
  case ControlDependenceNode::TRUE:
    return Pair.first;
  case ControlDependenceNode::FALSE:
    return Pair.second;
  default:
    assert(false && "Illegal edge type");
  }
  return Pair.first;
}

unsigned
CSACvtCFDFPass::getEdgePred(MachineBasicBlock *mbb,
                            ControlDependenceNode::EdgeType childType) {
  // In the case that the source node has only one child, return that node's
  // block predicate.
  if (mbb->succ_size() == 1) {
    assert(childType == ControlDependenceNode::OTHER &&
        "Invalid relationship for control dependence");
    assert(BlockPredicates.find(mbb) != BlockPredicates.end() &&
        "Must compute block predication before calling this function");
    return BlockPredicates[mbb];
  }
  if (EdgePredicates.find(mbb) == EdgePredicates.end())
    return 0;
  return getEdge(EdgePredicates[mbb], childType);
}

void CSACvtCFDFPass::setEdgePred(MachineBasicBlock *mbb,
                                 ControlDependenceNode::EdgeType childType,
                                 unsigned ch) {
  assert(ch && "0 is not a valid vreg number");
  if (EdgePredicates.find(mbb) == EdgePredicates.end()) {
    EdgePredicates[mbb] = std::make_pair(0, 0);
  }
  getEdge(EdgePredicates[mbb], childType) = ch;
  LMFI->setLICName(ch, "edgePred");
  LLVM_DEBUG(dbgs() << "  Edge predicate of BB#" << mbb->getNumber() << "->BB#"
      << (*(mbb->succ_begin() + childType))->getNumber()
      << " is " << printReg(ch) << "\n");
}

// also set up edge pred
MachineInstr *CSACvtCFDFPass::InsertPredProp(MachineBasicBlock *mbb,
                                             unsigned bbPred) {
  assert(mbb->succ_size() == 2);
  unsigned falseEdge = MRI->createVirtualRegister(&CSA::I1RegClass);
  unsigned trueEdge  = MRI->createVirtualRegister(&CSA::I1RegClass);
  MachineBasicBlock::iterator loc = mbb->getFirstTerminator();
  MachineInstr *bi                = &*loc;
  MachineInstr *predPropInstr =
    BuildMI(*mbb, loc, DebugLoc(), TII->get(CSA::PREDPROP), falseEdge)
      .addReg(trueEdge, RegState::Define)
      .addReg(bbPred)
      .addReg(bi->getOperand(0).getReg());
  LLVM_DEBUG(dbgs() << "  Inserting into BB#" << mbb->getNumber() << ": " <<
      *predPropInstr);
  setEdgePred(mbb, ControlDependenceNode::FALSE, falseEdge);
  setEdgePred(mbb, ControlDependenceNode::TRUE, trueEdge);
  return predPropInstr;
}

void CSACvtCFDFPass::computeBlockPredicates() {
  DenseMap<MachineLoop *, unsigned> LoopExitPredicates;

  auto mergeEdgePredicates = [&](MachineBasicBlock *MBB,
      ArrayRef<MachineBasicBlock *> Blocks) {
    // Get the predicates for the set of blocks;
    SmallVector<unsigned, 4> Predicates;
    for (auto Pred : Blocks)
      Predicates.push_back(getEdgePred(Pred, CDG->getEdgeType(Pred, MBB)));

    return makePickTree(MBB, MBB->begin(), Blocks, Predicates);
  };

  LLVM_DEBUG(dbgs() << "Computing block and edge predicates:\n");

  for (auto MBB : *RPOT) {
    // Generate a block predicate for the block.
    unsigned BlockPred = MBB->pred_size() == 1 ?
      BlockPredicates[*MBB->pred_begin()] : 0;

    MachineLoop *ML = MLI->getLoopFor(MBB);
    if (MBB->pred_size() == 0) {
      // Entry block. Gate the value on the input memory ordering edge.
      assert(MBB == &*thisMF->begin() && "Unreachable non-entry block");
      BlockPred = LMFI->allocateLIC(&CSA::CI1RegClass);
      BuildMI(*MBB, MBB->getFirstTerminator(), DebugLoc(),
          TII->get(CSA::GATE1), BlockPred)
        .addReg(LMFI->getInMemoryLic())
        .addImm(1);
    } else if (ML && ML->getHeader() == MBB) {
      // This is a loop header. Generate a loop exit predicate and block
      // predicate. These will actually get computed in a separate pass, once
      // the backedges are computed.
      LoopExitPredicates.insert(std::make_pair(ML,
        LMFI->allocateLIC(&CSA::CI1RegClass)));
      BlockPred = LMFI->allocateLIC(&CSA::CI1RegClass);

      LLVM_DEBUG(dbgs() << "  Loop exit predicate for BB#" << MBB->getNumber()
          << " is " << printReg(LoopExitPredicates[ML]) << "\n");
    } else {
      // Insert PREDMERGE chains for our inputs.
      auto PredMergePair = mergeEdgePredicates(MBB,
        ArrayRef<MachineBasicBlock *>(&*MBB->pred_begin(), &*MBB->pred_end()));
      PredMergeTrees[MBB] = std::move(PredMergePair.first);
      BlockPred = PredMergePair.second;
    }
    BlockPredicates.insert(std::make_pair(MBB, BlockPred));

    LLVM_DEBUG(dbgs() << "  Block predicate for BB#" << MBB->getNumber()
        << " is " << printReg(BlockPred) << "\n");

    // Generate an edge predicate for each outgoing edge.
    if (MBB->succ_size() > 1) {
      InsertPredProp(MBB, BlockPred);

      // If this is a loop exiting block, we need to filter out the edge
      // predicates.
      if (ML && ML->isLoopExiting(MBB)) {
        // Find the successor edge that's not in the loop.
        MachineBasicBlock *ExitBlock = nullptr;
        for (auto &Succ : MBB->successors()) {
          if (!ML->contains(Succ)) {
            ExitBlock = Succ;
            break;
          }
        }
        assert(ExitBlock && "None of the successors are outside the loop?");

        // Exiting a loop requires filtering the outgoing predicate so that the
        // exit block doesn't receive spurious not-executed predicates when the
        // loop continues for another iteration. A block can exit multiple loops
        // at once, so we need to insert filters for all of them in turn. The
        // exit block could also be in a sibling loop to the one that is being
        // converted, so this code goes up to the common ancestor of the current
        // loop and the loop containing the exit block.
        MachineLoop *OuterLoop = MLI->getLoopFor(ExitBlock);
        while (OuterLoop && !OuterLoop->contains(ML))
          OuterLoop = OuterLoop->getParentLoop();
        auto EdgeType = CDG->getEdgeType(MBB, ExitBlock);
        unsigned ExitingEdge = getEdgePred(MBB, EdgeType);
        for (MachineLoop *ExitingLoop = ML; ExitingLoop != OuterLoop;
            ExitingLoop = ExitingLoop->getParentLoop()) {
          unsigned FilteredEdge = LMFI->allocateLIC(&CSA::CI1RegClass);
          MachineInstr *FilterInstr = BuildMI(*MBB, MBB->getFirstTerminator(),
              MBB->getFirstTerminator()->getDebugLoc(), TII->get(CSA::FILTER1),
              FilteredEdge)
            .addReg(LoopExitPredicates[ExitingLoop])
            .addReg(ExitingEdge);
          LLVM_DEBUG(dbgs() << "  Filtering via loop at BB#" <<
              ExitingLoop->getHeader()->getNumber() << ": " <<
              *FilterInstr);
          (void)FilterInstr;
          ExitingEdge = FilteredEdge;
        }
        setEdgePred(MBB, EdgeType, ExitingEdge);
      }
    }
  }

  // Go through the loops and wire up the block predicates and loop phis for
  // the loop.
  for (auto &Pair : LoopExitPredicates) {
    MachineLoop *L = Pair.first;
    MachineBasicBlock *MBB = L->getHeader();
    unsigned LoopExitPredicate = Pair.second;
    unsigned BlockPred = BlockPredicates[MBB];
    auto InsertPoint = MBB->begin();

    // Create single values for the loop latch predicate and the loop start
    // predicate.
    SmallVector<MachineBasicBlock *, 4> LoopEntryBlocks;
    SmallVector<MachineBasicBlock *, 4> LoopLatchBlocks;
    for (auto Pred : MBB->predecessors()) {
      (L->contains(Pred) ? LoopLatchBlocks : LoopEntryBlocks).push_back(Pred);
    }
    auto LoopStartPair = mergeEdgePredicates(MBB, LoopEntryBlocks);
    auto LoopLatchPair = mergeEdgePredicates(MBB, LoopLatchBlocks);
    unsigned LoopStartPredicate = LoopStartPair.second;
    unsigned LoopLatched = LoopLatchPair.second;

    // We exited if latched is not 0.
    BuildMI(*MBB, InsertPoint, L->getStartLoc(), TII->get(CSA::NOT1),
        LoopExitPredicate)
      .addReg(LoopLatched);

    // Copy the loop latch predicate so that we can initialize it to 0.
    unsigned LoopInitPredicate = LMFI->allocateLIC(&CSA::CI1RegClass);
    BuildMI(*MBB, InsertPoint, L->getStartLoc(), TII->get(CSA::MOV1),
        LoopInitPredicate)
      .addReg(LoopLatched);
    BuildMI(*MBB, InsertPoint, L->getStartLoc(), TII->get(CSA::INIT1),
        LoopInitPredicate)
      .addImm(0);

    // This is a backedge, too.
    LMFI->addLICAttribute(LoopInitPredicate, "csasim_backedge");

    // Block predicate is Loop || Start.
    BuildMI(*MBB, InsertPoint, L->getStartLoc(), TII->get(CSA::LOR1), BlockPred)
      .addReg(LoopInitPredicate)
      .addReg(LoopStartPredicate)
      .addImm(0)
      .addImm(0);

    // Compute the pick control value for the header: it's the Loop predicate,
    // but only when the block is executed.
    unsigned HeaderPickReg = LMFI->allocateLIC(&CSA::CI1RegClass);
    BuildMI(*MBB, InsertPoint, L->getStartLoc(), TII->get(CSA::FILTER1),
        HeaderPickReg)
      .addReg(BlockPred)
      .addReg(LoopInitPredicate);

    PredMergeTrees[MBB] = std::make_unique<PickTreeNode>(
      std::move(LoopStartPair.first), std::move(LoopLatchPair.first),
      HeaderPickReg);

    // Add loop info information.
    CSALoopInfo &DFLoop = loopInfo[L];
    DFLoop.setPickBackedgeIndex(1);
  }
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

  if (!L->getExitingBlock()) {
    LLVM_DEBUG(dbgs() << L
        << " has multiple exiting blocks, requires dynamic predication.\n");
    return true;
  }

  if (CDG->getRegion(L->getExitingBlock()) != CDG->getRegion(L->getHeader())) {
    LLVM_DEBUG(dbgs() << L << " has a control-dependent exit block, "
        "requires dynamic predication.\n");
    return true;
  }

  return false;
}

bool CSACvtCFDFPass::checkIfDepthLimitedLoop(MachineLoop *L,
  MachineInstr* &tokenTake,
  MachineInstr* &tokenReturn)
{
  for (MachineBasicBlock *MBB : L->blocks()) {
    // Skip blocks in nested loops
    if (MLI->getLoopFor(MBB) != L)
      continue;
    for (MachineInstr &MI : *MBB) {
      if (MI.getOpcode() == CSA::CSA_PIPELINE_DEPTH_TOKEN_TAKE) {
        tokenTake = &MI;
        if (tokenReturn)
          return true;
      }
      else if (MI.getOpcode() == CSA::CSA_PIPELINE_DEPTH_TOKEN_RETURN) {
        tokenReturn = &MI;
        if (tokenTake)
          return true;
      }
    }
  }
  return false;
}

unsigned CSACvtCFDFPass::getInnerLoopPipeliningDegree(MachineLoop *L) {
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
  for (auto &MBB : *thisMF) {
    // Grab the loop info so we can note the header picks, only if this is the
    // header of a loop.
    CSALoopInfo *DFLoopInfo = nullptr;
    MachineLoop *Loop = MLI->getLoopFor(&MBB);
    if (Loop && Loop->getHeader() == &MBB) {
      DFLoopInfo = &loopInfo[Loop];
    }

    for (auto I = MBB.begin(), E = MBB.end(); I != E; ) {
      MachineInstr *MI = &*I;
      ++I;
      if (!MI->isPHI())
        continue;

      // Use the predmerge tree to generate the pick tree for the PHI
      // instruction. The resulting instruction will be generated right before
      // the PHI instruction, which we will immediately delete thereafter.
      auto PMTree = PredMergeTrees.find(&MBB);
      assert(PMTree != PredMergeTrees.end() &&
          "How did we not find a pred merge tree?");
      unsigned OutputReg = PMTree->second->convertToPick(MI, MI,
          *LMFI, *MRI, *TII);
      MI->eraseFromParent();

      // If this is a loop header, add the final pick in the tree to the loop
      // header picks list.
      if (DFLoopInfo) {
        MachineInstr *NewHeaderPick = MRI->getVRegDef(OutputReg);
        DFLoopInfo->addHeaderPick(NewHeaderPick);
      }
    }
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

std::pair<std::unique_ptr<PickTreeNode>, unsigned>
CSACvtCFDFPass::makePickTree(
    MachineBasicBlock *MBB, MachineBasicBlock::iterator InsertPoint,
    ArrayRef<MachineBasicBlock *> Blocks, ArrayRef<unsigned> Predicates) {
  assert(!Blocks.empty() && "Cannot make a tree of no nodes");
  if (Blocks.size() == 1) {
    return std::make_pair(std::make_unique<PickTreeNode>(Blocks[0]),
        Predicates[0]);
  } else {
    unsigned ChopPoint = Blocks.size() / 2;
    auto FalsePair = makePickTree(MBB, InsertPoint,
        Blocks.take_front(ChopPoint), Predicates.take_front(ChopPoint));
    auto TruePair = makePickTree(MBB, InsertPoint,
        Blocks.drop_front(ChopPoint), Predicates.drop_front(ChopPoint));

    unsigned MergedPred = LMFI->allocateLIC(&CSA::CI1RegClass);
    unsigned PickIndex = LMFI->allocateLIC(&CSA::CI1RegClass);
    MachineInstr *PredMerge = BuildMI(*MBB, InsertPoint,
        InsertPoint == MBB->end() ? DebugLoc() : InsertPoint->getDebugLoc(),
        TII->get(CSA::PREDMERGE), MergedPred)
      .addReg(PickIndex, RegState::Define)
      .addReg(FalsePair.second)
      .addReg(TruePair.second);
    (void)PredMerge;
    LLVM_DEBUG(dbgs() << "  Inserted block predicate merge: " <<
        *PredMerge);
    return std::make_pair(std::make_unique<PickTreeNode>(
          std::move(FalsePair.first), std::move(TruePair.first), PickIndex),
        MergedPred);
  }
}

unsigned PickTreeNode::convertToPick(MachineInstr *Phi,
    MachineBasicBlock::iterator InsertPoint, CSAMachineFunctionInfo &LMFI,
    MachineRegisterInfo &MRI, const CSAInstrInfo &TII,
    bool UseOutput) {
  // If we're a leaf node, return the register that corresponds to our current
  // key. If it's not present, just use %ign instead.
  if (isLeafNode()) {
    assert(!UseOutput && "Pick trees should be larger than one node.");
    for (unsigned I = 1, E = Phi->getNumOperands(); I != E; I += 2) {
      if (Phi->getOperand(I + 1).getMBB() == LeafKey)
        return Phi->getOperand(I).getReg();
    }
    return CSA::IGN;
  }

  // Not a leaf node--get the results of our parent nodes.
  unsigned FalseReg = PickFalse->convertToPick(Phi, InsertPoint, LMFI, MRI,
      TII, false);
  unsigned TrueReg = PickTrue->convertToPick(Phi, InsertPoint, LMFI, MRI,
      TII, false);

  // Optimization: when the two inputs are the same, we don't need to insert a
  // pick for those values. This probably only applies when the inputs are all
  // %ign.
  if (FalseReg == TrueReg && !UseOutput) {
    return FalseReg;
  }

  const TargetRegisterClass *RC = MRI.getRegClass(Phi->getOperand(0).getReg());
  unsigned OutputReg = UseOutput ? Phi->getOperand(0).getReg() :
    Register(LMFI.allocateLIC(RC));

  BuildMI(*Phi->getParent(), InsertPoint, Phi->getDebugLoc(),
      TII.get(TII.makeOpcode(CSA::Generic::PICK, RC)), OutputReg)
    .addReg(PickReg)
    .addReg(FalseReg)
    .addReg(TrueReg);

  return OutputReg;
}

void CSACvtCFDFPass::lowerLicQueue() {
  std::vector<unsigned> LicToReg; // maps from licNum -> register number
  SmallVector<MachineInstr *, 8> ToDelete;
  for (auto BB : *RPOT) {
    for (auto &MI : *BB) {
      if (MI.getOpcode() == CSA::CSA_LIC_INIT){
        unsigned licNum = MI.getOperand(0).getImm();
        const TargetRegisterClass *RC =
          TII->getLicClassForSize(MI.getOperand(1).getImm()*8);
        unsigned LicReg = LMFI->allocateLIC(RC, Twine("lic_queue") +
                                            Twine(licNum));
        LMFI->setLICDepth(LicReg, MI.getOperand(3).getImm());
        if (licNum >= LicToReg.size()) LicToReg.resize(licNum+1);
        LicToReg[licNum] = LicReg;
        ToDelete.push_back(&MI);
      }
      else if (MI.getOpcode() == CSA::CSA_LIC_WRITE) {
        unsigned licNum = MI.getOperand(0).getImm();
        unsigned reg =  LicToReg[licNum];
        const TargetRegisterClass *RC = MRI->getRegClass(reg);
        unsigned MovOpcode = TII->getMoveOpcode(RC);
        MachineInstr *movInst = BuildMI(*BB, MI, MI.getDebugLoc(),
                   TII->get(MovOpcode), reg).addReg(MI.getOperand(1).getReg());
        movInst->setFlag(MachineInstr::NonSequential);
        ToDelete.push_back(&MI);
      }
      else if (MI.getOpcode() == CSA::CSA_LIC_READ) {
        unsigned licNum = MI.getOperand(1).getImm();
        unsigned reg =  LicToReg[licNum];
        const TargetRegisterClass *RC = MRI->getRegClass(reg);
        unsigned MovOpcode = TII->getMoveOpcode(RC);
        MachineInstr *movInst = BuildMI(*BB, MI, MI.getDebugLoc(),
                  TII->get(MovOpcode ), MI.getOperand(0).getReg()).addReg(reg);
        movInst->setFlag(MachineInstr::NonSequential);
        ToDelete.push_back(&MI);
      }
    }
  }
  for (auto MI : ToDelete)
    MI->eraseFromParent();
}
