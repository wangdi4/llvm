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
#include "llvm/Pass.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "MachineCDG.h"

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

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      //AU.addRequired<MachineLoopInfo>();
      AU.addRequired<ControlDependenceGraph>();
      //AU.addRequired<LiveVariables>();
      AU.addRequired<MachineDominatorTree>();
      AU.setPreservesAll();
      MachineFunctionPass::getAnalysisUsage(AU);
    }
    void insertSWITCHForIf();
  private:
    MachineFunction *thisMF;
    MachineDominatorTree *DT;
    ControlDependenceGraph *CDG;
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
    // process each instruction in BB
    for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
      MachineInstr *MI = I;
      if (MI->isPHI()) continue; //care about forks, not joints
      for (MIOperands MO(MI); MO.isValid(); ++MO) {
        if (!MO->isReg()) continue;
        unsigned Reg = MO->getReg();
        // process uses
        if (MO->isUse()) {
          ControlDependenceNode *unode = CDG->getNode(mbb);
          CDGRegion *uregion = CDG->getRegion(unode);
          MachineInstr *DefMI = MRI->getVRegDef(Reg);
          if (DefMI && (DefMI->getParent() != mbb)) { // live into MI BB
            MachineBasicBlock *dmbb = DefMI->getParent();
            ControlDependenceNode *dnode = CDG->getNode(dmbb);
            CDGRegion *dRegion = CDG->getRegion(dnode);
            if (uregion != dRegion) {
              for (ControlDependenceNode::node_iterator uparent = unode->parent_begin(), uparent_end = unode->parent_end();
                uparent != uparent_end; ++uparent) {
                MachineBasicBlock *upbb = (*uparent)->getBlock();
                if (DT->dominates(dmbb, upbb)) {
                  assert(!dnode->isLatchNode());
                  //insertSWITCHForRegInBB(Reg, upbb);
                }
              }
            }
          }
        }
      }//end of for operand
    }//end of for MI
  }//end of for DTN(mbb)
}
