//===- MachineCDG.cpp ---------------------*- C++ -*-===//
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
// This file defines the ControlDependenceGraph class, which allows fast and
// efficient control dependence queries. It is based on Ferrante et al's "The
// Program Dependence Graph and Its Use in Optimization."
//
//===----------------------------------------------------------------------===//
#include "MachineCDG.h"
#include "CSA.h"
#include "CSATargetMachine.h"

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegionInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include <deque>
#include <set>

using namespace llvm;

static cl::opt<int>
  CSADumpDotGraph("csa-dump-dot-graph", cl::Hidden,
                  cl::desc("CSA Specific: dump CFG, CDG, PDT, DT dot graphs"),
                  cl::init(0));

static cl::opt<bool> CSAViewMachineCDG(
  "csa-view-machine-cdg", cl::Hidden,
  cl::desc("CSA Specific: View machine control dependence graph of function"),
  cl::init(false));

static cl::opt<bool> CSAViewMachineCFG(
  "csa-view-machine-cfg", cl::Hidden,
  cl::desc("CSA Specific: View machine control flow graph of function"),
  cl::init(false));

static cl::opt<bool> CSAViewMachinePDT(
  "csa-view-machine-pdt", cl::Hidden,
  cl::desc("CSA Specific: View machine post-dominator tree of function"),
  cl::init(false));

static cl::opt<bool> CSAViewMachineDT(
  "csa-view-machine-dt", cl::Hidden,
  cl::desc("CSA Specific: View machine dominator tree of function"),
  cl::init(false));

//  Because of the namespace-related syntax limitations of gcc, we need
//  To hoist init out of namespace blocks.
char ControlDependenceGraph::ID = 0;
// declare ControlDependenceGraph Pass
INITIALIZE_PASS(ControlDependenceGraph, "machine-cdg",
                "Machine Control Dependence Graph Construction", true, true)

#define DEBUG_TYPE "csa-cdg-pass"
namespace llvm {

void ControlDependenceNode::addTrue(ControlDependenceNode *Child) {
  TrueChildren.insert(Child);
}

void ControlDependenceNode::addFalse(ControlDependenceNode *Child) {
  FalseChildren.insert(Child);
}

void ControlDependenceNode::addOther(ControlDependenceNode *Child) {
  OtherChildren.insert(Child);
}

void ControlDependenceNode::addParent(ControlDependenceNode *Parent) {
  assert(std::find(Parent->begin(), Parent->end(), this) != Parent->end() &&
         "Must be a child before adding the parent!");
  Parents.insert(Parent);
}

void ControlDependenceNode::removeTrue(ControlDependenceNode *Child) {
  node_iterator CN = TrueChildren.find(Child);
  if (CN != TrueChildren.end())
    TrueChildren.erase(CN);
}

void ControlDependenceNode::removeFalse(ControlDependenceNode *Child) {
  node_iterator CN = FalseChildren.find(Child);
  if (CN != FalseChildren.end())
    FalseChildren.erase(CN);
}

void ControlDependenceNode::removeOther(ControlDependenceNode *Child) {
  node_iterator CN = OtherChildren.find(Child);
  if (CN != OtherChildren.end())
    OtherChildren.erase(CN);
}

void ControlDependenceNode::removeParent(ControlDependenceNode *Parent) {
  node_iterator PN = Parents.find(Parent);
  if (PN != Parents.end())
    Parents.erase(PN);
}

const ControlDependenceNode *ControlDependenceNode::enclosingRegion() const {
  if (this->isRegion()) {
    return this;
  } else {
    assert(this->Parents.size() == 1);
    const ControlDependenceNode *region = *this->Parents.begin();
    assert(region->isRegion());
    return region;
  }
}

ControlDependenceNode::EdgeType ControlDependenceGraphBase::getEdgeType(
  MachineBasicBlock *A, MachineBasicBlock *B, bool confirmAnalysiable) {
  SmallVector<MachineOperand, 4> Cond; // For analyzeBranch.
  Cond.clear();
  MachineBasicBlock *TBB = nullptr, *FBB = nullptr; // For analyzeBranch
  assert(A->isSuccessor(B) &&
         "Asking for edge type between unconnected basic blocks!");
  if (TII->analyzeBranch(*A, TBB, FBB, Cond)) {
    if (confirmAnalysiable) {
      assert(false && "can't analyze branch");
    }
    // no branch, just fall through
    return ControlDependenceNode::OTHER;
  } else if (!FBB && Cond.empty()) {
    // unconditional jump
    return ControlDependenceNode::OTHER;
  } else if (!FBB && !Cond.empty() && TBB) {
    // branch followed by a fall through
    if (TBB == B) {
      if (A->getFirstTerminator()->getOpcode() == CSA::BT)
        return ControlDependenceNode::TRUE;
      else
        return ControlDependenceNode::FALSE;
    } else {
      if (A->getFirstTerminator()->getOpcode() == CSA::BT)
        return ControlDependenceNode::FALSE;
      else
        return ControlDependenceNode::TRUE;
    }
  } else if (TBB && !Cond.empty() && FBB) {
    // a two-way branch
    if (TBB == B) {
      if (A->getFirstTerminator()->getOpcode() == CSA::BT) {
        return ControlDependenceNode::TRUE;
      } else {
        return ControlDependenceNode::FALSE;
      }
    } else {
      if (A->getFirstTerminator()->getOpcode() == CSA::BT) {
        return ControlDependenceNode::FALSE;
      } else {
        return ControlDependenceNode::TRUE;
      }
    }
  } else {
    assert(false && "unexpected case");
    return ControlDependenceNode::OTHER;
  }
}

void ControlDependenceGraphBase::computeDependencies(
  MachineFunction &F, MachinePostDominatorTree &pdt) {
  root = new ControlDependenceNode();
  nodes.insert(root);
  for (MachineFunction::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
    MachineBasicBlock *mbb    = &*BB;
    ControlDependenceNode *bn = new ControlDependenceNode(mbb);
    nodes.insert(bn);
    bb2cdg[mbb] = bn;
  }

  for (MachineFunction::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
    MachineBasicBlock *A      = &*BB;
    ControlDependenceNode *AN = bb2cdg[A];

    for (MachineBasicBlock::succ_iterator succ = A->succ_begin(),
                                          end  = A->succ_end();
         succ != end; ++succ) {
      MachineBasicBlock *B = *succ;
      assert(A && B);
      if (A == B || !pdt.dominates(B, A)) {
        MachineBasicBlock *L = pdt.findNearestCommonDominator(A, B);
        ControlDependenceNode::EdgeType type =
          ControlDependenceGraphBase::getEdgeType(A, B);
        if (A == L) {
          switch (type) {
          case ControlDependenceNode::TRUE:
            AN->addTrue(AN);
            break;
          case ControlDependenceNode::FALSE:
            AN->addFalse(AN);
            break;
          case ControlDependenceNode::OTHER:
            AN->addOther(AN);
            break;
          }
          AN->addParent(AN);
        }
        for (MachineDomTreeNode *cur = pdt[B]; cur && cur != pdt[L];
             cur                     = cur->getIDom()) {
          ControlDependenceNode *CN = bb2cdg[cur->getBlock()];
          switch (type) {
          case ControlDependenceNode::TRUE:
            AN->addTrue(CN);
            break;
          case ControlDependenceNode::FALSE:
            AN->addFalse(CN);
            break;
          case ControlDependenceNode::OTHER:
            AN->addOther(CN);
            break;
          }
          assert(CN);
          CN->addParent(AN);
        }
      }
    }
  }

  // ENTRY -> START
  for (MachineDomTreeNode *cur = pdt[&F.front()]; cur; cur = cur->getIDom()) {
    if (cur->getBlock()) {
      ControlDependenceNode *CN = bb2cdg[cur->getBlock()];
      assert(CN);
      root->addOther(CN);
      CN->addParent(root);
    }
  }
}

void ControlDependenceGraphBase::graphForFunction(
  MachineFunction &F, MachinePostDominatorTree &pdt) {
  computeDependencies(F, pdt);
  computeCDRegions(F, pdt);

  LLVM_DEBUG(dumpRegions());
}

void ControlDependenceGraphBase::computeCDRegions(MachineFunction &F,
    MachinePostDominatorTree &PDT) {
  // This algorithm for computing control-dependent regions is based on the one
  // presented in section 4 of Compact Representations for Control Dependence,
  // by Cytron, Ferrante and Sarkar.
  //
  // In essence, the algorithm works by initially lumping all of the nodes into
  // one region, and then iterating through every edge in the graph, splitting
  // regions into multiple regions if not all nodes in the region share the
  // control-dependence on the current edge. One useful property of the regions
  // is that every region is sorted such that the postdominators of a node
  // follows it in the list--in other words, at the end of the algorithm, the
  // list is sorted such that the first node dominates all other nodes and the
  // last node postdominates all other nodes in the region.
  // reset region for each funciton
  Regions.clear();

  Regions.emplace_back(new CDGRegion);
  RegionIndexes.clear();
  RegionIndexes.resize(F.size(), 0);

  // Insert all of the nodes into the starting region, such that the
  // postdominators of a node follow it in the list.
  for (auto DTN : post_order(&PDT)) {
    if (!DTN->getBlock())
      continue;
    Regions[0]->nodes.push_back(DTN->getBlock());
  }

  std::vector<unsigned> SplitRegions{0};

  // Process a chain of immediate post dominators from the start node to its
  // ending ancestor (the end is exclusive). This chain represents a set of
  // node that are control-dependent on some edge.
  auto processCD = [&](const MachineDomTreeNode *Start,
                       const MachineDomTreeNode *End) {
    assert(PDT.dominates(End, Start));
    unsigned T = Regions.size() - 1;
    for (auto Node = Start; Node != End; Node = Node->getIDom()) {
      MachineBasicBlock *Y = Node->getBlock();
      unsigned RegionNum = RegionIndexes[Y->getNumber()];
      CDGRegion *Region = Regions[RegionNum].get();
      // If the following condition fails, then some node in Y's region is not
      // control-dependent on the edge we're considering. Split this into a new
      // region.
      if (!PDT.dominates(PDT[Region->nodes.front()], Start) ||
          !PDT.properlyDominates(End, PDT[Region->nodes.back()])) {
        // If the old region for the node hasn't had a new region split to
        // include this control dependence, then make a new region.
        if (SplitRegions[RegionNum] <= T) {
          SplitRegions.push_back(Regions.size());
          SplitRegions[RegionNum] = Regions.size();
          Regions.emplace_back(new CDGRegion);
        }
        Region->nodes.erase(
            std::find(Region->nodes.begin(), Region->nodes.end(), Y));
        Regions[SplitRegions[RegionNum]]->nodes.push_back(Y);
        RegionIndexes[Y->getNumber()] = SplitRegions[RegionNum];
      }
    }
  };

  for (MachineBasicBlock &Block : F) {
    for (MachineBasicBlock *Succ : Block.successors()) {
      // The set of nodes from [Succ, postdom(Pred)) in the PDT are the set of
      // nodes that are control dependent on Pred -> Succ.
      processCD(PDT[Succ], PDT[&Block]->getIDom());
    }
  }

  // Include the dummy edge from the entry to the exit.
  processCD(PDT[&F.front()], PDT.getRootNode());

#ifndef NDEBUG
  // Verify that regions are defined such that every node is followed by its
  // postdominator.
  for (auto &Region : Regions) {
    MachineBasicBlock *Prev = nullptr;
    for (MachineBasicBlock *Node : Region->nodes) {
      if (Prev) {
        assert(PDT.dominates(Node, Prev) && "Region is not in dominator order");
      }
      Prev = Node;
    }
  }
#endif
}

void ControlDependenceGraphBase::dumpRegions() {
  for (unsigned I = 0; I < Regions.size(); I++) {
    errs() << "Region" << I << ": ";
    for (auto Node : Regions[I]->nodes) {
      errs() << "BB#" << Node->getNumber() << ", ";
    }
    errs() << "\n";
  }
}

bool ControlDependenceGraphBase::controls(MachineBasicBlock *A,
                                          MachineBasicBlock *B) const {
  const ControlDependenceNode *n = getNode(B);
  assert(n && "Basic block not in control dependence graph!");
  while (n->getNumParents() == 1) {
    n = *n->parent_begin();
    if (n->getBlock() == A)
      return true;
  }
  return false;
}

bool ControlDependenceGraphBase::influences(MachineBasicBlock *A,
                                            MachineBasicBlock *B) const {
  const ControlDependenceNode *n = getNode(B);
  assert(n && "Basic block not in control dependence graph!");

  std::deque<ControlDependenceNode *> worklist;
  worklist.insert(worklist.end(), n->parent_begin(), n->parent_end());

  while (!worklist.empty()) {
    n = worklist.front();
    worklist.pop_front();
    if (n->getBlock() == A)
      return true;
    worklist.insert(worklist.end(), n->parent_begin(), n->parent_end());
  }

  return false;
}

const ControlDependenceNode *
ControlDependenceGraphBase::enclosingRegion(MachineBasicBlock *BB) const {
  if (const ControlDependenceNode *node = this->getNode(BB)) {
    return node->enclosingRegion();
  } else {
    return NULL;
  }
}

ControlDependenceGraph::ControlDependenceGraph()
    : MachineFunctionPass(ID), ControlDependenceGraphBase() {
  initializeControlDependenceGraphPass(*PassRegistry::getPassRegistry());
}


bool ControlDependenceGraph::runOnMachineFunction(MachineFunction &F) {
  thisMF                        = &F;
  TII                           = thisMF->getSubtarget().getInstrInfo();

  MachinePostDominatorTree &pdt = getAnalysis<MachinePostDominatorTree>();
  if (pdt.getRootNode() == nullptr) {
    return false;
  }

  thisPDT = &pdt;
  graphForFunction(F, pdt);

  if (CSADumpDotGraph) {
    writeDotGraph(F.getName());
  }
  if (CSAViewMachineCDG) {
    viewMachineCDG();
  }
  if (CSAViewMachineCFG) {
    viewMachineCFG();
  }
  if (CSAViewMachinePDT) {
    viewMachinePDT();
  }
  if (CSAViewMachineDT) {
    viewMachineDT();
  }
  return false;
}

void ControlDependenceGraph::viewMachineCDG(void) {
  llvm::ViewGraph(this, "mCDG");
}

void ControlDependenceGraph::viewMachineCFG(void) {
  llvm::ViewGraph(thisMF, "mCFG");
}

void ControlDependenceGraph::viewMachinePDT(void) {
  MachinePostDominatorTree &pdt = getAnalysis<MachinePostDominatorTree>();
  llvm::ViewGraph(&pdt, "mPDT");
}

void ControlDependenceGraph::viewMachineDT(void) {
  MachineDominatorTree &dt = getAnalysis<MachineDominatorTree>();
  llvm::ViewGraph(&dt, "mDT");
}

void ControlDependenceGraph::writeDotGraph(StringRef fname) {
  std::string Filename = fname.str() + "_CDG" + ".dot";
  std::error_code EC;

  LLVM_DEBUG(errs() << "Writing '" << Filename << "'...");

  raw_fd_ostream File(Filename, EC, sys::fs::F_Text);
  GraphWriter<ControlDependenceGraph *> gwr(File, this, false);
  gwr.writeGraph();

  Filename = fname.str() + "_CFG" + ".dot";
  raw_fd_ostream File1(Filename, EC, sys::fs::F_Text);
  GraphWriter<MachineFunction *> gwr1(File1, thisMF, false);
  gwr1.writeGraph();

  MachinePostDominatorTree &pdt = getAnalysis<MachinePostDominatorTree>();
  Filename = fname.str() + "_PDT" + ".dot";
  raw_fd_ostream File2(Filename, EC, sys::fs::F_Text);
  GraphWriter<MachinePostDominatorTree *> gwr2(File2, &pdt, false);
  gwr2.writeGraph();
  //pdt.print(File2);

  MachineDominatorTree &dt = getAnalysis<MachineDominatorTree>();
  Filename = fname.str() + "_DT" + ".dot";
  raw_fd_ostream File3(Filename, EC, sys::fs::F_Text);
  GraphWriter<MachineDominatorTree *> gwr3(File3, &dt, false);
  gwr3.writeGraph();
}

void CSASSAGraph::BuildCSASSAGraph(MachineFunction &F, bool ignCtrl) {
  MachineRegisterInfo *MRI = &F.getRegInfo();
  root                     = new CSASSANode(nullptr);
  for (MachineFunction::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      MachineInstr *minstr = &*I;
      CSASSANode *sn;
      // skip mem-dependence artifical cycle
      if (minstr->getOpcode() == CSA::ALL0 ||
          // TII->getGenericOpcode(minstr->getOpcode()) == CSA::Generic::MERGE
          // ||
          TII->isLoad(minstr) || TII->isStore(minstr) ||
          TII->getGenericOpcode(minstr->getOpcode()) == CSA::Generic::REPEAT ||
          TII->getGenericOpcode(minstr->getOpcode()) == CSA::Generic::REPEATO)
        continue;
      if (instr2ssan.find(minstr) == instr2ssan.end()) {
        sn                 = new CSASSANode(minstr);
        instr2ssan[minstr] = sn;
        root->children.push_back(sn);
      } else {
        sn = instr2ssan[minstr];
      }
      unsigned i = 0;
      for (MIOperands MO(*minstr); MO.isValid(); ++MO, ++i) {
        if (ignCtrl && TII->isSwitch(minstr) && i == 2)
          // skip ctrl sig for pick/switch
          continue;
        if (TII->isPick(minstr) && i == 1)
          continue;
        if (TII->isPick(minstr) && i > 1) {
          unsigned pickCtrl = minstr->getOperand(1).getReg();
          if (!MRI->hasOneDef(pickCtrl)) {
            MachineInstr *lpInit = nullptr;
            for (MachineInstr &DefMI : MRI->def_instructions(pickCtrl)) {
              MachineInstr *dinstr = &DefMI;
              if (TII->isInit(dinstr)) {
                lpInit = dinstr;
                break;
              }
            }
            unsigned initIdx = lpInit->getOperand(1).getImm();
            // skip loop initial value in the pick instr
            if (i == initIdx + 2)
              continue;
          }
        }

        if (MO->isReg() && MO->isUse()) {
          unsigned reg = MO->getReg();
          for (MachineInstr &DefMI : MRI->def_instructions(reg)) {
            MachineInstr *dinstr = &DefMI;
            CSASSANode *cnode;
            if (instr2ssan.find(dinstr) == instr2ssan.end()) {
              cnode              = new CSASSANode(dinstr);
              instr2ssan[dinstr] = cnode;
            } else {
              cnode = instr2ssan[dinstr];
            }
            sn->children.push_back(cnode);
          }
        }
      }
    }
  }
}

} // namespace llvm

MachineFunctionPass *llvm::createControlDepenceGraph() {
  return new ControlDependenceGraph();
}
