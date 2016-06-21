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
#include "llvm/ADT/SparseSet.h"
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
    MachineInstr* getOrInsertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgBB);
    SmallVectorImpl<MachineInstr *>* insertPredCpy(MachineBasicBlock *);
    SmallVectorImpl<MachineInstr *>* getOrInsertPredCopy(MachineBasicBlock *cdgpBB);
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<MachineLoopInfo>();
      AU.addRequired<ControlDependenceGraph>();
      //AU.addRequired<LiveVariables>();
      AU.addRequired<MachineDominatorTree>();
      AU.setPreservesAll();
      MachineFunctionPass::getAnalysisUsage(AU);
    }
    void insertSWITCHForIf();
    void insertSWITCHForRepeat();
    void insertSWITCHForLoopExit();
    void replacePhiWithPICK();
    void replaceLoopHdrPhi();
    void replaceIfFooterPhi();
    void replace2InputsIfFooterPhi(MachineInstr* MI);
    void releaseMemory() override;
  private:
    MachineFunction *thisMF;
    MachineDominatorTree *DT;
    ControlDependenceGraph *CDG;
    MachineLoopInfo *MLI;
    DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *> bb2switch;  //switch for Reg added in bb
    DenseMap<MachineBasicBlock *, SmallVectorImpl<MachineInstr *>* > bb2predcpy;
  };
}

//  Because of the namespace-related syntax limitations of gcc, we need
//  To hoist init out of namespace blocks. 
char LPUCvtCFDFPass::ID = 0;
//declare LPUCvtCFDFPass Pass
INITIALIZE_PASS(LPUCvtCFDFPass, "lpu-cvt-cfdf", "LPU Convert Control Flow to Data Flow", true, true)

LPUCvtCFDFPass::LPUCvtCFDFPass() : MachineFunctionPass(ID) {
  initializeLPUCvtCFDFPassPass(*PassRegistry::getPassRegistry());
}


MachineFunctionPass *llvm::createLPUCvtCFDFPass() {
  return new LPUCvtCFDFPass();
}

void LPUCvtCFDFPass::releaseMemory() {
  DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *> ::iterator itm = bb2switch.begin();
  while (itm != bb2switch.end()) {
    DenseMap<unsigned, MachineInstr *>* reg2switch = itm->getSecond();
    ++itm;
    delete reg2switch;
  }

  DenseMap<MachineBasicBlock *, SmallVectorImpl<MachineInstr *> *> ::iterator itmv = bb2predcpy.begin();
  while (itmv != bb2predcpy.end()) {
    SmallVectorImpl<MachineInstr *>* instrv = itmv->getSecond();
    ++itmv;
    delete instrv;
  }
}


void LPUCvtCFDFPass::replacePhiWithPICK() {
  replaceLoopHdrPhi();
  replaceIfFooterPhi();
}

bool LPUCvtCFDFPass::runOnMachineFunction(MachineFunction &MF) {

  if (CvtCFDFPass == 0) return false;
  thisMF = &MF;

  DT = &getAnalysis<MachineDominatorTree>();
  CDG = &getAnalysis<ControlDependenceGraph>();
  MLI = &getAnalysis<MachineLoopInfo>();

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
  insertSWITCHForRepeat();
  insertSWITCHForLoopExit();
  replacePhiWithPICK();
 
  return Modified;

}

MachineInstr* LPUCvtCFDFPass::insertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgpBB) {
  // generate and insert SWITCH or copy
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
  MachineInstr* result;
  MachineBasicBlock::iterator loc = cdgpBB->getFirstTerminator();
  MachineInstr* bi = loc;
  if (cdgpBB->succ_size() > 1) {
    unsigned switchFalseReg = MRI->createVirtualRegister(TRC);
    unsigned switchTrueReg = MRI->createVirtualRegister(TRC);
    assert(bi->getOperand(0).isReg());
    // generate switch op
    const unsigned switchOpcode = TII.getPickSwitchOpcode(TRC, false /*not pick op*/);
    MachineInstr *switchInst = BuildMI(*cdgpBB, loc, DebugLoc(), TII.get(switchOpcode), 
                                                                     switchFalseReg).
                                                                     addReg(switchTrueReg, RegState::Define).
                                                                     addReg(bi->getOperand(0).getReg()).
                                                                     addReg(Reg);
    result = switchInst;
  }
  else {
    assert(MLI->getLoopFor(cdgpBB)->getLoopLatch() == cdgpBB);
    //LLVM 3.6 buggy latch with no exit edge
    //get a wierd latch with no exit edge from LLVM 3.6 buggy loop rotation
    const unsigned copyOpcode = TII.getCopyOpcode(TRC);
    unsigned cpyReg = MRI->createVirtualRegister(TRC);
    MachineInstr *cpyInst = BuildMI(*cdgpBB, loc, DebugLoc(), TII.get(copyOpcode), cpyReg).addReg(Reg);
    result = cpyInst;
  }
  return result;
}


MachineInstr* LPUCvtCFDFPass::getOrInsertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgpBB) {
  MachineInstr *defSwitchInstr = nullptr;
  DenseMap<unsigned, MachineInstr *>* reg2switch = nullptr;
  if (bb2switch.find(cdgpBB) == bb2switch.end()) {
    reg2switch = new DenseMap<unsigned, MachineInstr*>();
    bb2switch[cdgpBB] = reg2switch;
  }
  else {
    reg2switch = bb2switch[cdgpBB];
  }

  if (reg2switch->find(Reg) == reg2switch->end()) {
    defSwitchInstr = insertSWITCHForReg(Reg, cdgpBB);
    (*reg2switch)[Reg] = defSwitchInstr;
  }
  else {
    defSwitchInstr = (*reg2switch)[Reg];
  }
  
  return defSwitchInstr;
}

SmallVectorImpl<MachineInstr *>* LPUCvtCFDFPass::insertPredCpy(MachineBasicBlock *cdgpBB) {
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  assert(MLI->getLoopFor(cdgpBB)->getLoopLatch() == cdgpBB);
  MachineInstr* bi;
  MachineLoop* mloop = MLI->getLoopFor(cdgpBB);
  assert(mloop);
  //get the condition where the backedge is always taken
  if (cdgpBB->succ_size() == 1) {
    //LLVM 3.6 buggy loop latch with no exit edge from latch, fixed in 3.9
    ControlDependenceNode* latchNode = CDG->getNode(cdgpBB);
    //closed edge latchNode has loop hdr as control parent, 
    //nesting loop controls its loop hdr first, then the closed latch
    assert(latchNode->getNumParents() == 1);
    MachineBasicBlock* ctrlBB = (*latchNode->parent_begin())->getBlock();

    assert(ctrlBB);
    bi = ctrlBB->getFirstInstrTerminator();
  }
  else {
    bi = cdgpBB->getFirstInstrTerminator();
  }
  MachineBasicBlock::iterator loc = cdgpBB->getFirstTerminator();
  unsigned predReg = bi->getOperand(0).getReg();

  const TargetRegisterClass *TRC = MRI->getRegClass(predReg);
#if 0
  unsigned cpyReg = MRI->createVirtualRegister(TRC);
#else
  LPUMachineFunctionInfo *LMFI = thisMF->getInfo<LPUMachineFunctionInfo>();
  // Look up target register class corresponding to this register.
  const TargetRegisterClass* new_LIC_RC = LMFI->licRCFromGenRC(MRI->getRegClass(predReg));
  assert(new_LIC_RC && "Can't determine register class for register");
  unsigned cpyReg = LMFI->allocateLIC(new_LIC_RC);
#endif

  const unsigned copyOpcode = TII.getCopyOpcode(TRC);
  MachineInstr *cpyInst = BuildMI(*cdgpBB, loc, DebugLoc(),TII.get(copyOpcode), cpyReg).addReg(bi->getOperand(0).getReg());
  MachineBasicBlock *lphdr = MLI->getLoopFor(cdgpBB)->getHeader();
  MachineBasicBlock::iterator hdrloc = lphdr->begin();
  const unsigned InitOpcode = TII.getInitOpcode(TRC);

  ControlDependenceNode* hdrNode = CDG->getNode(lphdr);
  ControlDependenceNode* latchNode = CDG->getNode(cdgpBB);
  MachineInstr *initInst = nullptr;
  if (cdgpBB->succ_size() > 1) {
    if (latchNode->isTrueChild(hdrNode)) {
      initInst = BuildMI(*lphdr, hdrloc, DebugLoc(), TII.get(InitOpcode), cpyReg).addImm(0);
    }
    else {
      initInst = BuildMI(*lphdr, hdrloc, DebugLoc(), TII.get(InitOpcode), cpyReg).addImm(1);
    }
  }
  else {
    //LLVM 3.6 buggy latch
    //closed latch
    SmallVector<MachineOperand, 4> brCond;
    MachineBasicBlock *currTBB = nullptr, *currFBB = nullptr;
    if (TII.AnalyzeBranch(*bi->getParent(), currTBB, currFBB, brCond, true)) {
      assert(false && "BB branch not analyzable \n");
    }
    if (CDG->getNode(bi->getParent())->isTrueChild(CDG->getNode(cdgpBB))) {
      initInst = BuildMI(*lphdr, hdrloc, DebugLoc(), TII.get(InitOpcode), cpyReg).addImm(0);
    }
    else {
      initInst = BuildMI(*lphdr, hdrloc, DebugLoc(), TII.get(InitOpcode), cpyReg).addImm(1);
    }
  }
  SmallVector<MachineInstr *, 2>* predVec = new SmallVector<MachineInstr *, 2>();
  predVec->push_back(cpyInst);
  predVec->push_back(initInst);
  return predVec;
}


SmallVectorImpl<MachineInstr *>* LPUCvtCFDFPass::getOrInsertPredCopy(MachineBasicBlock *cdgpBB) {
  assert(MLI->getLoopFor(cdgpBB)->getLoopLatch() == cdgpBB);
  MachineInstr *predcpy = nullptr;
  SmallVectorImpl<MachineInstr *>* predcpyVec = nullptr;
  if (bb2predcpy.find(cdgpBB) == bb2predcpy.end()) {
    predcpyVec = insertPredCpy(cdgpBB);
    bb2predcpy[cdgpBB] = predcpyVec;
  }
  else {
    predcpyVec = bb2predcpy[cdgpBB];
  }
  return predcpyVec;
}

//focus on uses
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
              if (TII.isSwitch(DefMI)) {
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
       
                  MachineInstr *defSwitchInstr = getOrInsertSWITCHForReg(Reg, upbb);
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


//focus on def
void LPUCvtCFDFPass::insertSWITCHForLoopExit() {
  typedef po_iterator<ControlDependenceNode *> po_cdg_iterator;
  const TargetMachine &TM = thisMF->getTarget();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  
  ControlDependenceNode *root = CDG->getRoot();
  for (po_cdg_iterator DTN = po_cdg_iterator::begin(root), END = po_cdg_iterator::end(root); DTN != END; ++DTN) {
    MachineBasicBlock *mbb = DTN->getBlock();
    if (!mbb) continue; //root node has no bb
    MachineLoop* mloop = MLI->getLoopFor(mbb);
    //not inside a loop
    if (!mloop) continue;
    MachineBasicBlock *latchBB = mloop->getLoopLatch();
    //LLVM3.6 buggy latch without exit edge
    if (latchBB->succ_size() == 1) {
     //no SWITCH is needed is loop latch has no exit edge
      continue;
    }
    ControlDependenceNode *mLatch = CDG->getNode(latchBB);
    //inside a loop
    for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
      MachineInstr *MI = I;

      //avoid infinitive recursive
      if (TII.isSwitch(MI) && mbb == latchBB) {
        continue; 
      }
      for (MIOperands MO(MI); MO.isValid(); ++MO) {
        if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
        unsigned Reg = MO->getReg();
        // process defs
        if (MO->isDef()) {
          ControlDependenceNode *dnode = CDG->getNode(mbb);
          MachineRegisterInfo::use_iterator UI = MRI->use_begin(Reg);
          while (UI != MRI->use_end()) {
            MachineOperand &UseMO = *UI;
            MachineInstr *UseMI = UseMO.getParent();
            ++UI;
            MachineBasicBlock *UseBB = UseMI->getParent();
            if (UseBB == mbb) {
              if (UseBB != mloop->getHeader() || !UseMI->isPHI()) {
                //not a loop hdr Phi
                continue;
              }
              //for loop hdr Phi, we still need to handle back to back instructions in same block:
              // %y = Phi(%x0, %x)
              // %x = ...
            }
            MachineLoop* useLoop = MLI->getLoopFor(UseBB);
            if (mloop != useLoop || !DT->properlyDominates(mbb, UseBB)) {
              if (DT->properlyDominates(mbb, UseBB) && mloop == useLoop) continue;
              //can only have one nesting level difference
              else if (!DT->properlyDominates(mbb, UseBB) && mloop == useLoop) {
                //insertSWITCHForBackEdge();
                //def, use must in same loop, use must be loop hdr PHI, def come from backedge to loop hdr PHI
                assert(UseMI->isPHI());
                if (UseBB != mloop->getHeader()) {
                  //no need to attend if-footer Phi inside the loop, still need to attend those outside the loop
                  continue;
                }
                MachineInstr *defSwitchInstr = getOrInsertSWITCHForReg(Reg, latchBB);
              
                unsigned switchFalseReg = defSwitchInstr->getOperand(0).getReg();
                unsigned switchTrueReg = defSwitchInstr->getOperand(1).getReg();
                MachineBasicBlock* mlphdr = mloop->getHeader();
                unsigned newVReg;
                if (mLatch->isFalseChild(CDG->getNode(mlphdr))) {
                  //rename Reg to switchFalseReg
                  newVReg = switchFalseReg;
                } else {
                  //rename it to switchTrueReg
                  newVReg = switchTrueReg;
                }
                MachineBasicBlock* lphdr = mloop->getHeader();
                // Rewrite uses that outside of the original def's block, inside the loop
                MachineRegisterInfo::use_iterator UI = MRI->use_begin(Reg);
                while (UI != MRI->use_end()) {
                  MachineOperand &UseMO = *UI;
                  MachineInstr *UseMI = UseMO.getParent();
                  ++UI;
                  if (MLI->getLoopFor(UseMI->getParent()) == mloop &&
                    UseMI->getParent() == lphdr &&
                    UseMI->isPHI()) {
                    //rename loop header Phi
                    UseMO.setReg(newVReg);
                  }
                }
              } else {   //mloop != defLoop
                //two possibilites: a) def dom use;  b) def !dom use;
                //two cases: each can only have one nesting level difference
                // 1) def inside a loop, use outside the loop as LCSSA Phi with single input
                // 2) def outside a loop, use inside the loop, not handled here
                //use, def in different region cross latch
                bool isUseEnclosingDef = MLI->getLoopFor(UseBB) == NULL ||
                  MLI->getLoopFor(UseBB) == MLI->getLoopFor(mbb)->getParentLoop();
                //only need to handle use's loop immediately encloses def's loop, otherwise, reduced to case 2 which should already have been run
                if (isUseEnclosingDef) {
                  //this is case 1, can only have one level nesting difference 
                  MachineInstr *defSwitchInstr = getOrInsertSWITCHForReg(Reg, latchBB);

                  unsigned switchFalseReg = defSwitchInstr->getOperand(0).getReg();
                  unsigned switchTrueReg = defSwitchInstr->getOperand(1).getReg();
                  MachineBasicBlock* mlphdr = mloop->getHeader();
                  unsigned newVReg;
                  if (mLatch->isFalseChild(CDG->getNode(UseBB))) {
                    //rename Reg to switchFalseReg
                    newVReg = switchFalseReg;
                  } else {
                    //rename it to switchTrueReg
                    newVReg = switchTrueReg;
                  }
                  // Rewrite uses that outside of the original def's block, inside the loop
                  MachineRegisterInfo::use_iterator UI = MRI->use_begin(Reg);
                  while (UI != MRI->use_end()) {
                    MachineOperand &UseMO = *UI;
                    MachineInstr *UseMI = UseMO.getParent();
                    ++UI;
                    if (UseMI->getParent() == UseBB) {
                      //renameLCSSAPhi or other cross boundary uses
                      UseMO.setReg(newVReg);
                    }
                  }
                } //use enclosing def
                else {
                  // use not enclosing def, def and use in different regions
                  //assert(use have to be a switch from the repeat handling pass, or def is a switch from the if handling pass  
                  ControlDependenceNode* unode = CDG->getNode(UseBB);
                  ControlDependenceNode* dnode = CDG->getNode(mbb);
                  assert(TII.isSwitch(UseMI) && MLI->getLoopFor(UseBB)->getLoopLatch() == UseBB || 
                         TII.isSwitch(MI) && unode->isParent(dnode) ||
                         //loop hdr Phi generated by SSAUpdater in handling repeat case
                         UseMI->isPHI() && MLI->getLoopFor(UseBB)->getHeader() == UseBB);
                }
              }
            }
          }//end of while (use)
        }
      }//end of for operand
    }//end of for MI
  }//end of for DTN(mbb)
}


//focus on uses
void LPUCvtCFDFPass::insertSWITCHForRepeat() {
  typedef po_iterator<ControlDependenceNode *> po_cdg_iterator;
  const TargetMachine &TM = thisMF->getTarget();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  ControlDependenceNode *root = CDG->getRoot();
  for (po_cdg_iterator DTN = po_cdg_iterator::begin(root), END = po_cdg_iterator::end(root); DTN != END; ++DTN) {
    MachineBasicBlock *mbb = DTN->getBlock();
    if (!mbb) continue; //root node has no bb
    MachineLoop* mloop = MLI->getLoopFor(mbb);
    //not inside a loop
    if (!mloop) continue;
    MachineBasicBlock *latchBB = mloop->getLoopLatch();
    ControlDependenceNode *mLatch = CDG->getNode(latchBB);
    //make sure loop is properly formed with exit edge from latch block. 
    //LLVM 3.6 has this buggy issue that was subsequently fixed in 3.9
    //TBD:: reenable it:
    //assert(latchBB->succ_size() == 2);
    for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
      MachineInstr *MI = I;
      if (MI->isPHI()) continue; //care about forks, not joints
      //To avoid infinitive recursive since the newly add SWITCH always use Reg
      if (TII.isSwitch(MI) && CDG->getNode(mbb)->isLatchNode()) {
        //we are workin from inner most out, no need to revisit the switch after it is inserted into the latch
        continue;
      }
      for (MIOperands MO(MI); MO.isValid(); ++MO) {
        if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
        unsigned Reg = MO->getReg();
        // process use at loop level
        if (MO->isUse()) {
          MachineInstr* dMI = MRI->getVRegDef(Reg);
          MachineBasicBlock* DefBB = dMI->getParent();
          if (DefBB == mbb) continue;
          //use, def in different region cross latch
          bool isDefEnclosingUse = MLI->getLoopFor(DefBB) == NULL ||
                                   MLI->getLoopFor(mbb)->getParentLoop() == MLI->getLoopFor(DefBB);
          const TargetRegisterClass *TRC = MRI->getRegClass(Reg);

          if (isDefEnclosingUse && DT->dominates(DefBB, mbb)) {
            unsigned newVReg;  
            MachineInstr *defInstr = getOrInsertSWITCHForReg(Reg, latchBB);
            if (TII.isSwitch(defInstr)) {
              unsigned switchFalseReg = defInstr->getOperand(0).getReg();
              unsigned switchTrueReg = defInstr->getOperand(1).getReg();
              MachineBasicBlock* mlphdr = mloop->getHeader();
              if (mLatch->isFalseChild(CDG->getNode(mlphdr))) {
                //rename Reg to switchFalseReg
                newVReg = switchFalseReg;
              } else {
                //rename it to switchTrueReg
                newVReg = switchTrueReg;
              }
            } else {
              //LLVM3.6 buggy latch
              assert(TII.isCopy(defInstr));
              newVReg = defInstr->getOperand(0).getReg();
            }
      
            SmallVector<MachineInstr*, 8> NewPHIs;
            MachineSSAUpdater SSAUpdate(*thisMF, &NewPHIs);
            SSAUpdate.Initialize(newVReg);
            SSAUpdate.AddAvailableValue(DefBB, Reg);
            SSAUpdate.AddAvailableValue(latchBB, newVReg);
            // Rewrite uses that outside of the original def's block, inside the loop
            MachineRegisterInfo::use_iterator UI = MRI->use_begin(Reg);
            while (UI != MRI->use_end()) {
              MachineOperand &UseMO = *UI;
              MachineInstr *UseMI = UseMO.getParent();
              ++UI;
              if (UseMI->isDebugValue()) {
                UseMI->eraseFromParent();
                continue;
              }
              if (MLI->getLoopFor(UseMI->getParent()) == mloop) {
                SSAUpdate.RewriteUse(UseMO);
              }
            }
          }
        }
      }
    }
  }
}


void LPUCvtCFDFPass::replaceLoopHdrPhi() {
  typedef po_iterator<ControlDependenceNode *> po_cdg_iterator;
  const TargetMachine &TM = thisMF->getTarget();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  ControlDependenceNode *root = CDG->getRoot();
  for (po_cdg_iterator DTN = po_cdg_iterator::begin(root), END = po_cdg_iterator::end(root); DTN != END; ++DTN) {
    MachineBasicBlock *mbb = DTN->getBlock();
    if (!mbb) continue; //root node has no bb
    MachineLoop* mloop = MLI->getLoopFor(mbb);
    //not inside a loop
    if (!mloop) continue;
    //only scan loop header
    if (mbb != mloop->getHeader()) continue;
    MachineBasicBlock *latchBB = mloop->getLoopLatch();

    SmallVectorImpl<MachineInstr *>* predCpy = getOrInsertPredCopy(latchBB);

    ControlDependenceNode *mLatch = CDG->getNode(latchBB);
    MachineBasicBlock::iterator iterI = mbb->begin();
    while(iterI != mbb->end()) {
      MachineInstr *MI = iterI;
      ++iterI;
      if (!MI->isPHI()) continue;
      assert(MI->getNumOperands() == 5 && "loop header Phi can't have more than 2 inputs");
      unsigned pickSrc[2] = { 0 };
      for (MIOperands MO(MI); MO.isValid(); ++MO) {
        //if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
        if (!MO->isReg()) continue;
        unsigned Reg = MO->getReg();
        // process use at loop level
        if (MO->isUse()) {
          //move to its incoming block operand
          //++MO;
          MachineInstr* dMI = MRI->getVRegDef(Reg);
          MachineBasicBlock* DefBB = dMI->getParent();
          bool defInsideLoop = false;
          if (MLI->getLoopFor(DefBB)) {
            if (MLI->getLoopFor(DefBB) == mloop) {
              defInsideLoop = true;
            } else {
              //switch dst is defined inside a loop, but used outside loop
              MachineLoop* defLoop = MLI->getLoopFor(DefBB);
              while (defLoop->getParentLoop()) {
                defLoop = defLoop->getParentLoop();
                if (defLoop == mloop) {
                  defInsideLoop = true;
                  break;
                }
              }
            }
          }
          //if (DefBB == mbb) continue;
          //0 index is for init value, 1 for backedge value
          if (!defInsideLoop) {
            //def out side the loop
            pickSrc[0] = Reg;
          } else {
            //nothing can be assumed due to the Latch block without exit edge
            //inside loop def must come from a switch in the latch
            //assert(DefBB == mloop->getLoopLatch() && (TII.isSwitch(dMI)|| TII.isCopy(dMI)));
            pickSrc[1] = Reg;
          }
        }
      } //end for MO
      assert(pickSrc[0] && pickSrc[1]);
      unsigned pickFalseReg, pickTrueReg;
      if (latchBB->succ_size() > 1) {
        assert(mLatch->isChild(*DTN));
        if (mLatch->isFalseChild(*DTN)) {
          pickFalseReg = pickSrc[1];
          pickTrueReg = pickSrc[0];
        } else {
          pickTrueReg = pickSrc[1];
          pickFalseReg = pickSrc[0];
        }
      } else {
        //LLVM 3.6 buggy latch, loop with > 1 exits
        //LLVM 3.6 buggy loop latch with no exit edge from latch, fixed in 3.9
        ControlDependenceNode* latchNode = CDG->getNode(latchBB);
        assert(latchNode->getNumParents() == 1);
        ControlDependenceNode* ctrlNode = *latchNode->parent_begin();
        MachineInstr* bi = ctrlNode->getBlock()->getFirstInstrTerminator();
        SmallVector<MachineOperand, 4> brCond;
        MachineBasicBlock *currTBB = nullptr, *currFBB = nullptr;
        if (TII.AnalyzeBranch(*bi->getParent(), currTBB, currFBB, brCond, true)) {
          assert(false && "BB branch not analyzable \n");
        }
        if (CDG->getNode(bi->getParent())->isFalseChild(CDG->getNode(latchBB))) {
          pickFalseReg = pickSrc[1];
          pickTrueReg = pickSrc[0];
        } else {
          pickTrueReg = pickSrc[1];
          pickFalseReg = pickSrc[0];
        }
      }
      unsigned predReg = (*predCpy)[0]->getOperand(0).getReg();
      unsigned dst = MI->getOperand(0).getReg();
      const TargetRegisterClass *TRC = MRI->getRegClass(dst);
      const unsigned pickOpcode = TII.getPickSwitchOpcode(TRC, true /*pick op*/);
      //generate PICK, and insert before MI
      MachineInstr *pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII.get(pickOpcode), dst).
                                        addReg(predReg).
                                        addReg(pickFalseReg).addReg(pickTrueReg);
      MI->removeFromParent();
    }
  }
}


void LPUCvtCFDFPass::replace2InputsIfFooterPhi(MachineInstr* MI) {
  const TargetMachine &TM = thisMF->getTarget();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  MachineBasicBlock* mbb = MI->getParent();
  unsigned pickFalseReg = 0, pickTrueReg = 0, fallThroughReg = 0;
  MachineBasicBlock* controlBB = nullptr;
  for (MIOperands MO(MI); MO.isValid(); ++MO) {
    if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
    unsigned Reg = MO->getReg();
    if (MO->isUse()) {
      //move to its incoming block operand
      ++MO;
      MachineBasicBlock* inBB = nullptr;
      MachineInstr* dMI = MRI->getVRegDef(Reg);
      MachineBasicBlock* DefBB = dMI->getParent();
      if (DT->dominates(DefBB, mbb)) {  
        //Triangle fall through case
        controlBB = nullptr;
        inBB = nullptr;
        fallThroughReg = Reg;
      } else { 
        //Diamond case
        ControlDependenceNode* defNode = CDG->getNode(DefBB);
        ControlDependenceNode* defParent = CDG->getNonLatchParent(defNode, true);
        assert(defParent);
        controlBB = defParent->getBlock();
        inBB = DefBB;
      }
      if (controlBB) {
        ControlDependenceNode* controlNode = CDG->getNode(controlBB);
        ControlDependenceNode* inNode = CDG->getNode(inBB);
        if (controlNode->isFalseChild(inNode)) {
          pickFalseReg = Reg;
        } else {
          //has to be a conditional branch
          assert(!controlNode->isOtherChild(inNode));
          pickTrueReg = Reg;
        }
      }
    }
  }
  assert(controlBB && (pickFalseReg != 0 || pickTrueReg != 0));
  if (fallThroughReg != 0) {
    if (pickFalseReg == 0) {
      pickFalseReg = fallThroughReg;
    } else {
      assert(pickTrueReg == 0);
      pickTrueReg = fallThroughReg;
    }
  }
  MachineInstr* bi = controlBB->getFirstInstrTerminator();
  unsigned predReg = bi->getOperand(0).getReg();
  unsigned dst = MI->getOperand(0).getReg();
  const TargetRegisterClass *TRC = MRI->getRegClass(dst);
  const unsigned pickOpcode = TII.getPickSwitchOpcode(TRC, true /*pick op*/);
  //generate PICK, and insert before MI
  MachineInstr *pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII.get(pickOpcode), dst).addReg(predReg).
                                                                                          addReg(pickFalseReg).
                                                                                          addReg(pickTrueReg);
  MI->removeFromParent();
}



void LPUCvtCFDFPass::replaceIfFooterPhi() {
	typedef po_iterator<ControlDependenceNode *> po_cdg_iterator;
	const TargetMachine &TM = thisMF->getTarget();
	const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
	const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
	MachineRegisterInfo *MRI = &thisMF->getRegInfo();
	ControlDependenceNode *root = CDG->getRoot();
	for (po_cdg_iterator DTN = po_cdg_iterator::begin(root), END = po_cdg_iterator::end(root); DTN != END; ++DTN) {
		MachineBasicBlock *mbb = DTN->getBlock();
		if (!mbb) continue; //root node has no bb
		MachineBasicBlock::iterator iterI = mbb->begin();
		while (iterI != mbb->end()) {
			MachineInstr *MI = iterI;
			++iterI;
			if (!MI->isPHI()) continue;

			//for two inputs value, we can generate better code
			if (MI->getNumOperands() == 5) {
				replace2InputsIfFooterPhi(MI);
				continue;
			}

			unsigned pickFalseReg = 0, pickTrueReg = 0, fallThroughReg = 0;
			MachineBasicBlock* controlBB = nullptr;
			unsigned dst = MI->getOperand(0).getReg();
			const TargetRegisterClass *TRC = MRI->getRegClass(dst);
			const unsigned pickOpcode = TII.getPickSwitchOpcode(TRC, true /*pick op*/);

			for (MIOperands MO(MI); MO.isValid(); ++MO) {
				if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
				unsigned Reg = MO->getReg();
				if (MO->isUse()) {
					//move to its incoming block operand
					++MO;
					MachineBasicBlock* inBB = nullptr;
					MachineInstr* dMI = MRI->getVRegDef(Reg);
					MachineBasicBlock* DefBB = dMI->getParent();
					if (DT->dominates(DefBB, mbb)) {
						//Triangle fall through case
						controlBB = nullptr;
						inBB = nullptr;
						fallThroughReg = Reg;
					}
				}
			}

			unsigned pickReg;
			if (fallThroughReg == 0) {
				//start with %ign
				pickReg = LPU::IGN;
			} else {
				pickReg = fallThroughReg;
			}
			//assume only can have one fallthrough reg in Phi's inputs
			for (MIOperands MO(MI); MO.isValid(); ++MO) {
				if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg()) || !MO->isUse()) continue;
				if (MO->getReg() == fallThroughReg) continue;
				MIOperands Opd = MO;
				//move MO to incoming blk
				++MO;
				MachineBasicBlock* inBB = MO->getMBB();
				MachineInstr *defMI = MRI->getVRegDef(Opd->getReg());
				MachineBasicBlock* defBB = defMI->getParent();
				ControlDependenceNode* defNode = CDG->getNode(defBB);
				assert(defNode->getNumParents() == 1 || defNode->getNumParents() == 2);
				ControlDependenceNode* ctrlNode = CDG->getNonLatchParent(defNode, true);
				MachineBasicBlock* ctrlBB = ctrlNode->getBlock();
				MachineInstr* bi = ctrlBB->getFirstInstrTerminator();
				unsigned predReg = bi->getOperand(0).getReg();
				if (ctrlNode->isTrueChild(defNode)) {
					pickTrueReg = Opd->getReg();
					pickFalseReg = pickReg;
				} else {
					pickFalseReg = Opd->getReg();
					pickTrueReg = pickReg;
				}

				unsigned newdst = MRI->createVirtualRegister(MRI->getRegClass(dst));
				//generate PICK, and insert before MI, so that new PICK is after the previously generated PICKS
				MachineInstr *pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII.get(pickOpcode), newdst).addReg(predReg).
					addReg(pickFalseReg).
					addReg(pickTrueReg);
				pickReg = newdst;
			}
			const unsigned copyOpcode = TII.getCopyOpcode(TRC);
			MachineInstr *cpyInst = BuildMI(*mbb, MI, DebugLoc(), TII.get(copyOpcode), dst).addReg(pickReg);
			MI->removeFromParent();
		}
	}
}


#if 0
void LPUCvtCFDFPass::replaceIfFooterPhi() {
	typedef po_iterator<ControlDependenceNode *> po_cdg_iterator;
	const TargetMachine &TM = thisMF->getTarget();
	const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
	const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
	MachineRegisterInfo *MRI = &thisMF->getRegInfo();
	ControlDependenceNode *root = CDG->getRoot();
	for (po_cdg_iterator DTN = po_cdg_iterator::begin(root), END = po_cdg_iterator::end(root); DTN != END; ++DTN) {
		MachineBasicBlock *mbb = DTN->getBlock();
		if (!mbb) continue; //root node has no bb
		MachineBasicBlock::iterator iterI = mbb->begin();
		while (iterI != mbb->end()) {
			MachineInstr *MI = iterI;
			++iterI;
			if (!MI->isPHI()) continue;

			//for two inputs value, we can generate better code
			if (MI->getNumOperands() == 5) {
				replace2InputsIfFooterPhi(MI);
				continue;
			}

			std::set<ControlDependenceNode *> InNodes;
			SmallVector<ControlDependenceNode *, 4> CtrlNodes;
			InNodes.clear();
			CtrlNodes.clear();
			unsigned pickFalseReg = 0, pickTrueReg = 0, fallThroughReg = 0;
			MachineBasicBlock* controlBB = nullptr;
			unsigned dst = MI->getOperand(0).getReg();
			const TargetRegisterClass *TRC = MRI->getRegClass(dst);
			const unsigned pickOpcode = TII.getPickSwitchOpcode(TRC, true /*pick op*/);

			for (MIOperands MO(MI); MO.isValid(); ++MO) {
				if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
				unsigned Reg = MO->getReg();
				if (MO->isUse()) {
					//move to its incoming block operand
					++MO;
					MachineBasicBlock* inBB = nullptr;
					MachineInstr* dMI = MRI->getVRegDef(Reg);
					MachineBasicBlock* DefBB = dMI->getParent();
					if (DT->dominates(DefBB, mbb)) {
						//Triangle fall through case
						controlBB = nullptr;
						inBB = nullptr;
						fallThroughReg = Reg;
					}
					else {
						//Diamond case
						ControlDependenceNode* inNode = CDG->getNode(DefBB);
						ControlDependenceNode* ctrlNode = CDG->getNonLatchParent(inNode, true);
						assert(ctrlNode);
						controlBB = ctrlNode->getBlock();
						inBB = DefBB;
						InNodes.insert(inNode);
					}

					ControlDependenceNode* inNode = CDG->getNode(inBB);
					ControlDependenceNode* ctrlNode = CDG->getNode(controlBB);
					InNodes.insert(inNode);
					CtrlNodes.push_back(ctrlNode);
				}
			}// end of for(MO

			unsigned pickReg;
			MachineBasicBlock* visitedBB[2] = { 0 };
			if (fallThroughReg == 0) {
				//CtrlNodes forms a linear parent relationship of each other
				for (SmallVectorImpl<ControlDependenceNode *>::iterator iterCtrl = CtrlNodes.begin(); iterCtrl != CtrlNodes.end(); iterCtrl++) {
					ControlDependenceNode* ctrlNode = *iterCtrl;
					if (ctrlNode->getNumChildren() == 2 &&
						ctrlNode->true_begin() != ctrlNode->true_end() &&
						ctrlNode->false_begin() != ctrlNode->false_end()) {
						//ctrl's true and fasle child are all in phi's incoming blocks
						if (InNodes.find(*ctrlNode->false_begin()) != InNodes.end() &&
							InNodes.find(*ctrlNode->true_begin()) != InNodes.end()) {
							MachineInstr* bi = ctrlNode->getBlock()->getFirstInstrTerminator();
							unsigned predReg = bi->getOperand(0).getReg();
							MachineBasicBlock* TBB = (*ctrlNode->true_begin())->getBlock();
							MachineBasicBlock* FBB = (*ctrlNode->false_begin())->getBlock();
							for (MIOperands MO(MI); MO.isValid(); ++MO) {
								if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg()) || !MO->isUse()) continue;
								MIOperands Opd = MO;
								++MO;
								MachineBasicBlock* inBB = MO->getMBB();
								if (inBB == TBB) {
									pickTrueReg = Opd->getReg();
									visitedBB[0] = inBB;
								}
								else if (inBB == FBB) {
									pickFalseReg = Opd->getReg();
									visitedBB[1] = inBB;
								}
							}
							//generate PICK, and insert before MI
							pickReg = MRI->createVirtualRegister(MRI->getRegClass(dst));
							MachineInstr *pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII.get(pickOpcode), pickReg).addReg(predReg).
								addReg(pickFalseReg).
								addReg(pickTrueReg);
							break;
						}
					}
				}
			}
			else {
				pickReg = fallThroughReg;
			}

			//either a fall through or a diamond shaple at the lowest level of if's
			assert(fallThroughReg || visitedBB[0] && visitedBB[1]);

			//assume only can have one fallthrough reg in Phi's inputs
			for (MIOperands MO(MI); MO.isValid(); ++MO) {
				if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg()) || !MO->isUse()) continue;
				MIOperands Opd = MO;
				//move MO to incoming blk
				++MO;
				MachineBasicBlock* inBB = MO->getMBB();
				if (inBB != visitedBB[0] && inBB != visitedBB[1] && Opd->getReg() != fallThroughReg) {
					MachineInstr *defMI = MRI->getVRegDef(Opd->getReg());
					MachineBasicBlock* defBB = defMI->getParent();
					ControlDependenceNode* defNode = CDG->getNode(defBB);
					ControlDependenceNode* ctrlNode = CDG->getNonLatchParent(defNode, true);
					MachineBasicBlock* ctrlBB = ctrlNode->getBlock();
					MachineInstr* bi = ctrlBB->getFirstInstrTerminator();
					unsigned predReg = bi->getOperand(0).getReg();
					if (ctrlNode->isTrueChild(defNode)) {
						pickTrueReg = Opd->getReg();
						pickFalseReg = pickReg;
					}
					else {
						pickFalseReg = Opd->getReg();
						pickTrueReg = pickReg;
					}

					unsigned newdst = MRI->createVirtualRegister(MRI->getRegClass(dst));
					//generate PICK, and insert before MI, so that new PICK is after the previously generated PICKS
					MachineInstr *pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII.get(pickOpcode), newdst).addReg(predReg).
						addReg(pickFalseReg).
						addReg(pickTrueReg);

					pickReg = newdst;
				}
			}
			const unsigned copyOpcode = TII.getCopyOpcode(TRC);
			MachineInstr *cpyInst = BuildMI(*mbb, MI, DebugLoc(), TII.get(copyOpcode), dst).addReg(pickReg);
			MI->removeFromParent();
		}
	}
}
#endif
