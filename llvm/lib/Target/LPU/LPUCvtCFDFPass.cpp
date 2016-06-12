//===-- LPUCvtCFDFPass.cpp - LPU convert control flow to data flow --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file "reexpresses" the code containing traditional control flow
// into a basically data flow representation suitable for the LPU.
//
//===----------------------------------------------------------------------===//

#include <map>
#include "LPU.h"
#include "InstPrinter/LPUInstPrinter.h"
#include "LPUInstrInfo.h"
#include "LPUTargetMachine.h"
#include "LPULicAllocation.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineSSAUpdater.h"
#include "llvm/Pass.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "MachineCDG.h"
#include "LPUInstrInfo.h"

using namespace llvm;

static cl::opt<int>
CvtCFDFPass("lpu-cvt-cf-df-pass", cl::Hidden,
               cl::desc("LPU Specific: Convert control flow to data flow pass"),
               cl::init(1));

#define DEBUG_TYPE "lpu-cvt-ctl-df"

namespace llvm {
  class LPUCvtCFDFPass : public MachineFunctionPass {
  public:
    static char ID;
    LPUCvtCFDFPass();

    const char* getPassName() const override {
      return "LPU Convert Control Flow to Data Flow";
    }

    bool runOnMachineFunction(MachineFunction &MF) override;
    MachineInstr* insertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgpBB);
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      //AU.addRequired<MachineLoopInfo>();
      AU.addRequired<ControlDependenceGraph>();
      //AU.addRequired<LiveVariables>();
      AU.addRequired<MachineDominatorTree>();
      AU.setPreservesAll();
      MachineFunctionPass::getAnalysisUsage(AU);
    }
    void insertSWITCHForIf();
    void insertSWITCHForLoop();
  private:
    MachineFunction *thisMF;
    MachineDominatorTree *DT;
    ControlDependenceGraph *CDG;
    DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *>> bb2switch;  //switch for Reg added in bb
  };


  char LPUCvtCFDFPass::ID = 0;
  //declare LPUCvtCFDFPass Pass
  INITIALIZE_PASS(LPUCvtCFDFPass, "lpu-cvt-cfdf", "LPU Convert Control Flow to Data Flow", true, true)

  LPUCvtCFDFPass::LPUCvtCFDFPass() : MachineFunctionPass(ID) {
    initializeLPUCvtCFDFPassPass(*PassRegistry::getPassRegistry());
  }
}

MachineFunctionPass *llvm::createLPUCvtCFDFPass() {
  return new LPUCvtCFDFPass();
}


bool LPUCvtCFDFPass::runOnMachineFunction(MachineFunction &MF) {

  if (CvtCFDFPass == 0) return false;
  thisMF = &MF;

  DT = &getAnalysis<MachineDominatorTree>();
  CDG = &getAnalysis<ControlDependenceGraph>();

  bool Modified = false;
#if 0
  // for now only well formed innermost loop regions are processed in this pass
  MachineLoopInfo *MLI = &getAnalysis<MachineLoopInfo>();
  if (!MLI) {
    DEBUG(errs() << "no loop info.\n");
    return false;
  }
#endif
  insertSWITCHForIf();
  return Modified;

}

MachineInstr* LPUCvtCFDFPass::insertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgpBB) {
  // generate and insert SWITCH in dominating block
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
  unsigned switchFalseReg = MRI->createVirtualRegister(TRC);
  unsigned switchTrueReg = MRI->createVirtualRegister(TRC);

  MachineBasicBlock::iterator loc = cdgpBB->getFirstTerminator();
  MachineInstr* bi = loc;
  // generate switch op
  const unsigned switchOpcode = TII.getPickSwitchOpcode(TRC, false /*not pick op*/);
  MachineInstr *switchInst = BuildMI(*cdgpBB, loc,
                                     DebugLoc(),
                                     TII.get(switchOpcode),
                                     switchFalseReg).addReg(switchTrueReg,
                                     RegState::Define).
                                     addReg(bi->getOperand(0).getReg()).
                                     addReg(Reg);
  return switchInst;
}

void LPUCvtCFDFPass::insertSWITCHForIf() {
  typedef po_iterator<ControlDependenceNode *> po_cdg_iterator;
  const TargetMachine &TM = thisMF->getTarget();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();

  ControlDependenceNode *root = CDG->getRoot();
  for (po_cdg_iterator DTN = po_cdg_iterator::begin(root), END = po_cdg_iterator::end(root); DTN != END; ++DTN) {
    MachineBasicBlock *mbb = DTN->getBlock();
    if (!mbb) continue; //root node has no bb
    //if (root->isChild(*DTN)) continue;  ??????
    // process each instruction in BB
    for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
      MachineInstr *MI = I;
      if (MI->isPHI()) continue; //care about forks, not joints
      for (MIOperands MO(MI); MO.isValid(); ++MO) {
        if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
        unsigned Reg = MO->getReg();
        // process uses
        if (MO->isUse()) {
          ControlDependenceNode *unode = CDG->getNode(mbb);
          CDGRegion *uregion = CDG->getRegion(unode);
          assert(uregion);
          MachineInstr *DefMI = MRI->getVRegDef(Reg);
          const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
      
          if (DefMI && (DefMI->getParent() != mbb)) { // live into MI BB
            MachineBasicBlock *dmbb = DefMI->getParent();
            ControlDependenceNode *dnode = CDG->getNode(dmbb);
            CDGRegion *dRegion = CDG->getRegion(dnode);
            assert(dRegion);
            //use, def in different region => need switch
            if (uregion != dRegion) {
              if (DefMI->getOpcode() == TII.getPickSwitchOpcode(TRC, false/*not pick op*/)) {
                //def already from a switch -- can only happen if use is an immediate child of def in CDG
                assert(dnode->isChild(unode));
                continue;
              }

              unsigned numIfParent = 0;
              for (ControlDependenceNode::node_iterator uparent = unode->parent_begin(), uparent_end = unode->parent_end();
                uparent != uparent_end; ++uparent) {
                ControlDependenceNode *upnode = *uparent;
                MachineBasicBlock *upbb = upnode->getBlock();
                if (!upbb) {
                  //this is tipical define inside loop, used outside loop on the main execution path
                  continue;
                }
                if (mbb == upbb) {
                  //mbb is a loop latch node, use inside a loop will be take care of in HandleUseInLopp
                  continue;
                }
                if (upnode->isLatchNode()) {
                  //no need to conside backedge for if-statements handling
                  continue;
                }
                if (DT->dominates(dmbb, upbb)) { //including dmbb itself
                  numIfParent++;
                  if (numIfParent > 1) {
                    assert(false && "TBD: support multiple if parents in CDG has not been implemented yet");
                  }
                  assert(!dnode->isLatchNode() && "latch node can't forward dominate nodes inside its own loop");
       
                  MachineInstr *defSwitchInstr = NULL;
                  if (bb2switch[upbb].find(Reg) == bb2switch[upbb].end()) {
                    defSwitchInstr = insertSWITCHForReg(Reg, upbb);
                    DenseMap<unsigned, MachineInstr *> reg2inst;
                    reg2inst[Reg] = defSwitchInstr;
                    bb2switch[upbb] = reg2inst;
                  } else {
                    defSwitchInstr = bb2switch[upbb][Reg];
                  }

                  unsigned switchFalseReg = defSwitchInstr->getOperand(0).getReg();
                  unsigned switchTrueReg = defSwitchInstr->getOperand(1).getReg();
                  unsigned newVReg;
                  if (upnode->isFalseChild(unode)) {
                    //rename Reg to switchFalseReg
                    newVReg = switchFalseReg;
                  } else {
                    //rename it to switchTrueReg
                    newVReg = switchTrueReg;
                  }

                  MachineRegisterInfo::use_iterator UI = MRI->use_begin(Reg);
                  while (UI != MRI->use_end()) {
                    MachineOperand &UseMO = *UI;
                    MachineInstr *UseMI = UseMO.getParent();
                    ++UI;

                    if (UseMI->getParent() == mbb) {
                      assert(mbb != upbb);
                      UseMO.setReg(newVReg);
                    }
                  }
                }
              }
            }
          }
        }
      }//end of for operand
    }//end of for MI
  }//end of for DTN(mbb)
}

void LPUCvtCFDFPass::insertSWITCHForLoop() {
  typedef po_iterator<ControlDependenceNode *> po_cdg_iterator;
  const TargetMachine &TM = thisMF->getTarget();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();

  ControlDependenceNode *root = CDG->getRoot();
  for (po_cdg_iterator DTN = po_cdg_iterator::begin(root), END = po_cdg_iterator::end(root); DTN != END; ++DTN) {
    ControlDependenceNode *latchParent;
    if (DTN->isLatchNode()) {
      latchParent = *DTN;
    } else {
      latchParent = CDG->getLatchParent(*DTN);
    }
    //inside a loop
    if (latchParent) {
      MachineBasicBlock *mbb = DTN->getBlock();
      if (!mbb) continue; //root node has no bb
      for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
        MachineInstr *MI = I;
        if (MI->isPHI()) continue; //care about forks, not joints
        for (MIOperands MO(MI); MO.isValid(); ++MO) {
          if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
          unsigned Reg = MO->getReg();
          // process defs
          if (MO->isDef()) {
            ControlDependenceNode *dnode = CDG->getNode(mbb);
            CDGRegion *dregion = CDG->getRegion(dnode);
            assert(dregion);
            MachineRegisterInfo::use_iterator UI = MRI->use_begin(Reg);
            while (UI != MRI->use_end()) {
              MachineOperand &UseMO = *UI;
              MachineInstr *UseMI = UseMO.getParent();
              ++UI;
              MachineBasicBlock *UseBB = UseMI->getParent();
              if (UseBB == mbb) continue;
              ControlDependenceNode *unode = CDG->getNode(UseBB);
              CDGRegion *uregion = CDG->getRegion(unode);
              assert(uregion);
              if (dregion != uregion || !DT->dominates(mbb, UseBB)) {
                //can only have one nesting level difference
                if (!DT->dominates(mbb, UseBB)) {
                  //def, use must in same loop, use must be loop hdr PHI, def come from backedge to loop hdr PHI
                  //reassure the previous set up condition
                  assert(mbb != UseBB);
                  //two cases: a) dregion == uregion; b) dregion != uregion
                  assert(UseMI->isPHI());
                  if (dregion == uregion) {
                    assert(CDG->getLatchParent(dnode) == CDG->getLatchParent(unode));
                  }
                  else {
                    assert(dnode->isLatchNode() && CDG->getLatchParent(unode) == dnode);
                  }
                  //insertSWITCHForBackEdge();
                  //renameLPHdrPhi();
                }
                else {
                  //def dom use but in different regions
                  //two possibilites: a) def dom use;  b) def !dom use; -- with b) already coveried in previous branch
                  assert(DT->dominates(mbb, UseBB));
                  //two cases: each can only have one nesting level difference
                  // 1) def inside a loop, use outside the loop as LCSSA Phi with single input
                  // 2) def outside a loop, use inside the loop, not handled here
                  assert(latchParent->getNumParents() == 1 && "loop latch has more than one CDG parent"); //assume single entry loop, and latch can't be the end node
                  ControlDependenceNode* loopParent = *latchParent->parent_begin();
                  assert(loopParent);
                  if (CDG->getRegion(loopParent) == CDG->getRegion(unode)) {
                    //this is case 1, can only have one level nesting difference 
                    //insertSWITCHForLPExit()
                    //renameLCSSAPhi()
                  }
                }
              }
            }//end of while (!use_end)
          }
        }//end of for operand
      }//end of for MI
    }//end of for DTN(mbb)
  }
}

