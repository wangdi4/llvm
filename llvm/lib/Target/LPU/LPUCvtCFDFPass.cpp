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
#include <stack>
#include "LPU.h"
#include "InstPrinter/LPUInstPrinter.h"
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
#include "llvm/Target/TargetInstrInfo.h"
#include "MachineCDG.h"
#include "LPUInstrInfo.h"

using namespace llvm;

static cl::opt<int>
CvtCFDFPass("lpu-cvt-cf-df-pass", cl::Hidden,
               cl::desc("LPU Specific: Convert control flow to data flow pass"),
               cl::init(1));

static cl::opt<int>
RunSXU("lpu-run-sxu", cl::Hidden,
	cl::desc("LPU Specific: run on sequential unit"),
	cl::init(0));


#define DEBUG_TYPE "lpu-cvt-cf-df-pass"

namespace llvm {
  class LPUCvtCFDFPass : public MachineFunctionPass {
  public:
    static char ID;
    LPUCvtCFDFPass();

    const char* getPassName() const override {
      return "LPU Convert Control Flow to Data Flow";
    }

    bool runOnMachineFunction(MachineFunction &MF) override;
	ControlDependenceNode* getNonLatchParent(ControlDependenceNode* anode, bool oneAndOnly = false);
    MachineInstr* insertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgpBB);
    MachineInstr* getOrInsertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgBB);
    MachineInstr* insertPICKForReg(MachineBasicBlock* ctrlBB, unsigned Reg, MachineBasicBlock* inBB, MachineInstr* phi, unsigned pickReg = 0);
    void assignPICKSrcForReg(unsigned &pickFalseReg, unsigned &pickTrueReg, unsigned Reg, MachineBasicBlock* ctrlBB, MachineBasicBlock* inBB, MachineInstr* phi);
    //generate a PICK for SSA value dst at fork of ctrlBB with source input Reg from inBB, and output in pickReg
    MachineInstr* PatchOrInsertPickAtFork(MachineBasicBlock* ctrlBB, unsigned dst, unsigned Reg, MachineBasicBlock* inBB, MachineInstr* phi, unsigned pickReg = 0);
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
		void insertSWITCHForOperand(MachineOperand& MO, MachineBasicBlock* mbb, MachineInstr* phiIn = nullptr);
    void insertSWITCHForIf();
    void insertSWITCHForRepeat();
    void insertSWITCHForLoopExit();
    void replacePhiWithPICK();
    void replaceLoopHdrPhi();
    void replaceIfFooterPhiSeq();
  	void assignLicForDF();
  	void removeBranch();
    void linearizeCFG();
    unsigned findSwitchingDstForReg(unsigned Reg, MachineBasicBlock* mbb);
    void handleAllConstantInputs();
    void releaseMemory() override;
  private:
    MachineFunction *thisMF;
    MachineDominatorTree *DT;
    ControlDependenceGraph *CDG;
    MachineLoopInfo *MLI;
    DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *> bb2switch;  //switch for Reg added in bb
    DenseMap<MachineBasicBlock *, SmallVectorImpl<MachineInstr *>* > bb2predcpy;
    DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *> bb2pick;  //switch for Reg added in bb
    std::set<MachineInstr *> multiInputsPick;
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

  DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *> ::iterator itmp = bb2pick.begin();
  while (itmp != bb2pick.end()) {
    DenseMap<unsigned, MachineInstr *>* reg2pick = itmp->getSecond();
    ++itmp;
    delete reg2pick;
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
#if 0
  replaceIfFooterPhi();
#else
  replaceIfFooterPhiSeq();
#endif
}

//return the first non latch parent found or NULL
ControlDependenceNode* LPUCvtCFDFPass::getNonLatchParent(ControlDependenceNode* anode, bool oneAndOnly ) {
	ControlDependenceNode* pcdn = nullptr;
	if (anode->getNumParents() == 0) return pcdn;
	for (ControlDependenceNode::node_iterator pnode = anode->parent_begin(), pend = anode->parent_end(); pnode != pend; ++pnode) {
		MachineBasicBlock* pbb = (*pnode)->getBlock();
		if (!pbb) continue; //root of CDG is a fake node
		if (MLI->getLoopFor(pbb) == NULL ||
			MLI->getLoopFor(pbb)->getLoopLatch() != pbb) {
			if (oneAndOnly && pcdn) {
				assert(false && "CDG node has more than one if parent");
			}
			pcdn = *pnode;
		}
	}
	return pcdn;
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
  handleAllConstantInputs();
  assignLicForDF();
  if (!RunSXU) {
    removeBranch();
    linearizeCFG();
  }
  return Modified;

}

MachineInstr* LPUCvtCFDFPass::insertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgpBB) {
  // generate and insert SWITCH or copy
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
  MachineInstr* result = nullptr;
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
    switchInst->setFlag(MachineInstr::NonSequential);
    result = switchInst;
  } else {
    assert(MLI->getLoopFor(cdgpBB)->getLoopLatch() == cdgpBB);
    //LLVM 3.6 buggy latch with no exit edge
    //get a wierd latch with no exit edge from LLVM 3.6 buggy loop rotation
    const unsigned moveOpcode = TII.getMoveOpcode(TRC);
    unsigned cpyReg = MRI->createVirtualRegister(TRC);
    MachineInstr *cpyInst = BuildMI(*cdgpBB, loc, DebugLoc(), TII.get(moveOpcode), cpyReg).addReg(Reg);
    cpyInst->setFlag(MachineInstr::NonSequential);
    result = cpyInst;
  }
  return result;
}


unsigned LPUCvtCFDFPass::findSwitchingDstForReg(unsigned Reg, MachineBasicBlock* mbb) {
  if (bb2switch.find(mbb) == bb2switch.end()) {
    return 0;
  }
  DenseMap<unsigned, MachineInstr *>* reg2switch = bb2switch[mbb];
  if (reg2switch->find(Reg) == reg2switch->end()) {
    return 0;
  }
  MachineInstr *defSwitchInstr = (*reg2switch)[Reg];
  unsigned switchFalseReg = defSwitchInstr->getOperand(0).getReg();
  unsigned switchTrueReg = defSwitchInstr->getOperand(1).getReg();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  if (MRI->use_empty(switchFalseReg)) {
    return switchFalseReg;
  }
  else if (MRI->use_empty(switchTrueReg)) {
    return switchTrueReg;
  }
  return 0;
}



MachineInstr* LPUCvtCFDFPass::getOrInsertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgpBB) {
  MachineInstr *defSwitchInstr = nullptr;
  DenseMap<unsigned, MachineInstr *>* reg2switch = nullptr;
  if (bb2switch.find(cdgpBB) == bb2switch.end()) {
    reg2switch = new DenseMap<unsigned, MachineInstr*>();
    bb2switch[cdgpBB] = reg2switch;
  } else {
    reg2switch = bb2switch[cdgpBB];
  }

  if (reg2switch->find(Reg) == reg2switch->end()) {
    defSwitchInstr = insertSWITCHForReg(Reg, cdgpBB);
    (*reg2switch)[Reg] = defSwitchInstr;
  } else {
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
  } else {
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

  const unsigned moveOpcode = TII.getMoveOpcode(TRC);
  MachineInstr *cpyInst = BuildMI(*cdgpBB, loc, DebugLoc(),TII.get(moveOpcode), cpyReg).addReg(bi->getOperand(0).getReg());
  cpyInst->setFlag(MachineInstr::NonSequential);
  MachineBasicBlock *lphdr = MLI->getLoopFor(cdgpBB)->getHeader();
  MachineBasicBlock::iterator hdrloc = lphdr->begin();
  const unsigned InitOpcode = TII.getInitOpcode(TRC);

  ControlDependenceNode* hdrNode = CDG->getNode(lphdr);
  ControlDependenceNode* latchNode = CDG->getNode(cdgpBB);
  MachineInstr *initInst = nullptr;
  if (cdgpBB->succ_size() > 1) {
    if (latchNode->isTrueChild(hdrNode)) {
      initInst = BuildMI(*lphdr, hdrloc, DebugLoc(), TII.get(InitOpcode), cpyReg).addImm(0);
    } else {
      initInst = BuildMI(*lphdr, hdrloc, DebugLoc(), TII.get(InitOpcode), cpyReg).addImm(1);
    }
    initInst->setFlag(MachineInstr::NonSequential);
  } else {
    //LLVM 3.6 buggy latch
    //closed latch
    if (CDG->getNode(bi->getParent())->isTrueChild(CDG->getNode(cdgpBB))) {
      initInst = BuildMI(*lphdr, hdrloc, DebugLoc(), TII.get(InitOpcode), cpyReg).addImm(0);
    } else {
      initInst = BuildMI(*lphdr, hdrloc, DebugLoc(), TII.get(InitOpcode), cpyReg).addImm(1);
    }
    initInst->setFlag(MachineInstr::NonSequential);
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
  } else {
    predcpyVec = bb2predcpy[cdgpBB];
  }
  return predcpyVec;
}



void LPUCvtCFDFPass::insertSWITCHForOperand(MachineOperand& MO, MachineBasicBlock* mbb, MachineInstr* phiIn) {
	const TargetMachine &TM = thisMF->getTarget();
	const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
	const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
	MachineRegisterInfo *MRI = &thisMF->getRegInfo();
	if (!MO.isReg() || !TargetRegisterInfo::isVirtualRegister(MO.getReg())) return;
	unsigned Reg = MO.getReg();
	// process uses
	if (MO.isUse()) {
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
					//assert(dnode->isChild(unode) || MI->isPHI());
					return;
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
					if (MLI->getLoopFor(upbb) &&
						MLI->getLoopFor(upbb)->getLoopLatch() == upbb) {
						//no need to conside backedge for if-statements handling
						continue;
					}
					if (DT->dominates(dmbb, upbb))
					{ //including dmbb itself
						numIfParent++;
						if (numIfParent > 1) {
							assert(false && "TBD: support multiple if parents in CDG has not been implemented yet");
						}
						assert((MLI->getLoopFor(dmbb) == NULL ||
							MLI->getLoopFor(dmbb) != MLI->getLoopFor(upbb) ||
							MLI->getLoopFor(dmbb)->getLoopLatch() != dmbb) &&
							"latch node can't forward dominate nodes inside its own loop");

						MachineInstr *defSwitchInstr = getOrInsertSWITCHForReg(Reg, upbb);
						unsigned switchFalseReg = defSwitchInstr->getOperand(0).getReg();
						unsigned switchTrueReg = defSwitchInstr->getOperand(1).getReg();
						unsigned newVReg;
						if (upnode->isFalseChild(unode)) {
							//rename Reg to switchFalseReg
							newVReg = switchFalseReg;
						}
						else {
							//rename it to switchTrueReg
							newVReg = switchTrueReg;
						}
						if (phiIn) {
							MO.setReg(newVReg);
						}
						else {
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
	}
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
    // process each instruction in BB
    for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
      MachineInstr *MI = I;
			//loop header phi forward input is a kind of fork
			//if (MI->isPHI()) continue; //care about forks, not joints
			for (MIOperands MO(MI); MO.isValid(); ++MO) {
				insertSWITCHForOperand(*MO, mbb);
			}
    }//end of for MI
		for (MachineBasicBlock::succ_iterator isucc = mbb->succ_begin(); isucc != mbb->succ_end(); ++isucc) {
			MachineBasicBlock* succBB = *isucc;
			for (MachineBasicBlock::iterator iPhi = succBB->begin(); iPhi != succBB->end(); ++iPhi) {
				if (!iPhi->isPHI()) {
					break;
				}
				for (MIOperands MO(iPhi); MO.isValid(); ++MO) {
					if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
					unsigned Reg = MO->getReg();
					// process uses
					if (MO->isUse()) {
					  MachineOperand& mOpnd = *MO;
						++MO;
						if (MO->getMBB() == mbb) {
							if (mbb->succ_size() == 1 || 
								  mbb->succ_size() == 2 && MLI->getLoopFor(mbb) && MLI->getLoopFor(mbb)->getLoopLatch() == mbb) {
								insertSWITCHForOperand(mOpnd, mbb, iPhi);
							}	else {
								//mbb itself is a fork
								MachineInstr *defSwitchInstr = getOrInsertSWITCHForReg(Reg, mbb);
								unsigned switchFalseReg = defSwitchInstr->getOperand(0).getReg();
								unsigned switchTrueReg = defSwitchInstr->getOperand(1).getReg();
								unsigned newVReg;
								if (CDG->getEdgeType(mbb, succBB) == ControlDependenceNode::TRUE) {
									newVReg = switchTrueReg;
								}	else {
									newVReg = switchFalseReg;
								}
								mOpnd.setReg(newVReg);
							}
						}
					}
				}
			}
		}
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
                  assert(latchBB->succ_size() > 1);
                  if (mLatch->isFalseChild(CDG->getNode(mlphdr))) {
                    //rename Reg to switchTrueReg
                    newVReg = switchTrueReg;
                  } else {
                    //rename it to switchFalseReg
                    newVReg = switchFalseReg;
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
                  assert(TII.isSwitch(UseMI) || 
                         TII.isSwitch(MI) && unode->isParent(dnode) ||
                         //loop hdr Phi generated by SSAUpdater in handling repeat case
                         UseMI->isPHI());
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
      if (MI->isPHI()) continue; //Pick will take care of it when replacing Phi
      //To avoid infinitive recursive since the newly add SWITCH always use Reg
      if (TII.isSwitch(MI) && mbb == latchBB) {
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
          MachineInstr* dMI = MRI->getVRegDef(Reg);
          MachineBasicBlock* DefBB = dMI->getParent();
          bool defInsideLoop = false;
          if (MLI->getLoopFor(DefBB)) {
            if (MLI->getLoopFor(DefBB) == mloop) {
              defInsideLoop = true;
            } else {
              //switch dst is defined in an inner loop, but used in the current loop
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
        if (CDG->getEdgeType(bi->getParent(), latchBB) == ControlDependenceNode::FALSE) {
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
      pickInst->setFlag(MachineInstr::NonSequential);
      MI->removeFromParent();
    }
  }
}


void LPUCvtCFDFPass::assignLicForDF() {
  const TargetMachine &TM = thisMF->getTarget();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  LPUMachineFunctionInfo *LMFI = thisMF->getInfo<LPUMachineFunctionInfo>();
  std::deque<unsigned> renameQueue;
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator MI = BB->begin(), EI = BB->end(); MI != EI; ++MI) {
      if (TII.isPick(MI) || TII.isSwitch(MI)) {
        for (MIOperands MO(MI); MO.isValid(); ++MO) {
          if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
          unsigned Reg = MO->getReg();
          if (MO->isDef() && MRI->use_empty(Reg) && TII.isSwitch(MI)) {
            MI->substituteRegister(Reg, LPU::IGN, 0, TRI);
          } else {
            renameQueue.push_back(Reg);
          }
        }
      }
    }
  }
  while (!renameQueue.empty()) {
    unsigned dReg = renameQueue.front();
    renameQueue.pop_front();
    MachineInstr *DefMI = MRI->getVRegDef(dReg);
    if (!DefMI ) continue;
    //if (TII.isLoad(DefMI) || TII.isStore(DefMI)) continue;
    const TargetRegisterClass *TRC = MRI->getRegClass(dReg);
    const TargetRegisterClass* new_LIC_RC = LMFI->licRCFromGenRC(MRI->getRegClass(dReg));
    assert(new_LIC_RC && "unknown LPU register class");
    unsigned phyReg = LMFI->allocateLIC(new_LIC_RC);
    DefMI->substituteRegister(dReg, phyReg, 0, TRI);
    MachineRegisterInfo::use_iterator UI = MRI->use_begin(dReg);
    while (UI != MRI->use_end()) {
      MachineOperand &UseMO = *UI;
      ++UI;
      UseMO.setReg(phyReg);
    }
    for (MIOperands MO(DefMI); MO.isValid(); ++MO) {
      if (!MO->isReg() || !MO->isUse() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
      unsigned Reg = MO->getReg();
      renameQueue.push_back(Reg);
    }
  }

#if 1
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator MI = BB->begin(), EI = BB->end(); MI != EI; ++MI) {
			unsigned schedClass = MI->getDesc().getSchedClass();
			if (!schedClass) continue;

			bool allLics = true;
      for (MIOperands MO(MI); MO.isValid(); ++MO) {
        if (!MO->isReg()) {
          if (MO->isImm() || MO->isCImm() || MO->isFPImm()) {
            continue;
          } else {
            allLics = false;
            break;
          }
        } else {
          unsigned Reg = MO->getReg();
          if (Reg < LPU::CI0_0 || Reg > LPU::CI64_1023) {
            allLics = false;
            break;
          }
        }
      }
      if (allLics) {
        MI->setFlag(MachineInstr::NonSequential);
      }
    }
  }
#endif
}


void LPUCvtCFDFPass::handleAllConstantInputs() {
  const TargetMachine &TM = thisMF->getTarget();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  
  LPUMachineFunctionInfo *LMFI = thisMF->getInfo<LPUMachineFunctionInfo>();
  std::deque<unsigned> renameQueue;
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    MachineBasicBlock::iterator iterMI = BB->begin();
    while(iterMI != BB->end()) {
      MachineInstr* MI = iterMI;
      ++iterMI;      
      if (!TII.isMOV(MI)) continue;

      bool allConst = true;
      for (MIOperands MO(MI); MO.isValid(); ++MO) {
        if (MO->isReg() && MO->isDef()) continue;
        if (!MO->isImm() && !MO->isCImm() && !MO->isFPImm()) {
            allConst = false;
            break;
        }
      }
      if (allConst) {
        const TargetRegisterClass *TRC = MRI->getRegClass(MI->getOperand(0).getReg());
        ControlDependenceNode* mNode = CDG->getNode(BB);
        ControlDependenceNode* ctrlNode = getNonLatchParent(mNode, true);
        unsigned switchFalse = LPU::IGN, switchTrue = LPU::IGN;
        if (ctrlNode) {
          if (ctrlNode->isFalseChild(mNode)) {
            switchFalse = MI->getOperand(0).getReg();
          } else {
            switchTrue = MI->getOperand(0).getReg();
          }
          MachineInstr* bi = ctrlNode->getBlock()->getFirstTerminator();
          assert(bi->getOperand(0).isReg());
          unsigned predReg = bi->getOperand(0).getReg();
          const unsigned switchOpcode = TII.getPickSwitchOpcode(TRC, false /*not pick op*/);
          MachineInstr *switchInst = BuildMI(*BB, MI, DebugLoc(), TII.get(switchOpcode), switchFalse).addReg(switchTrue, RegState::Define).
                                                                                                        addReg(predReg).addOperand(MI->getOperand(1));
          MI->removeFromParent();
          switchInst->setFlag(MachineInstr::NonSequential);
        }
      }
    }
  }
}




void LPUCvtCFDFPass::removeBranch() {
	const TargetMachine &TM = thisMF->getTarget();
	const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
	const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
	MachineRegisterInfo *MRI = &thisMF->getRegInfo();
	LPUMachineFunctionInfo *LMFI = thisMF->getInfo<LPUMachineFunctionInfo>();
	std::deque<unsigned> renameQueue;
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    MachineBasicBlock::iterator iterMI = BB->begin();
    while (iterMI != BB->end()) {
      MachineInstr* MI = iterMI;
      ++iterMI;
      if (MI->isBranch()) {
        MI->removeFromParent();
      }
    }
  }
}



void LPUCvtCFDFPass::linearizeCFG() {
  typedef po_iterator<MachineBasicBlock *> po_mbb_iterator;
  MachineBasicBlock *root = thisMF->begin();
  std::stack<MachineBasicBlock *> mbbStack;
  for (po_mbb_iterator mbb = po_mbb_iterator::begin(root), END = po_mbb_iterator::end(root); mbb != END; ++mbb) {
    mbbStack.push(*mbb);
  }
  MachineBasicBlock *x = mbbStack.top();
  assert(x == root);
  MachineBasicBlock::succ_iterator SI = root->succ_begin();
  while (SI != root->succ_end()) {
	  SI = root->removeSuccessor(SI);
  }
  mbbStack.pop();
  while (!mbbStack.empty()) {
    MachineBasicBlock* mbb = mbbStack.top();
    mbbStack.pop();
	  root->splice(root->end(), mbb, mbb->begin(), mbb->end());
	  mbb->eraseFromParent();
  }
}



MachineInstr* LPUCvtCFDFPass::PatchOrInsertPickAtFork(
  MachineBasicBlock* ctrlBB, //fork
  unsigned dst,              //the SSA value
  unsigned Reg,              //input of phi
  MachineBasicBlock* inBB,   //incoming blk
  MachineInstr* phi,         //the multi-input phi
  unsigned pickReg)          //pick output
{
  const TargetMachine &TM = thisMF->getTarget();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  MachineInstr *pickInstr = nullptr;
  bool patched = false;
  DenseMap<unsigned, MachineInstr *>* reg2pick = nullptr;
  if (bb2pick.find(ctrlBB) == bb2pick.end()) {
    reg2pick = new DenseMap<unsigned, MachineInstr*>();
    bb2pick[ctrlBB] = reg2pick;
  } else {
    reg2pick = bb2pick[ctrlBB];
  }

  if (reg2pick->find(dst) == reg2pick->end()) {
    pickInstr = insertPICKForReg(ctrlBB, Reg, inBB, phi, pickReg);
    (*reg2pick)[dst] = pickInstr;
  } else {
    //find existing PICK, patch its %ign with Reg
    pickInstr = (*reg2pick)[dst];
    unsigned pickFalseReg = 0, pickTrueReg = 0;
    assignPICKSrcForReg(pickFalseReg, pickTrueReg, Reg, ctrlBB, inBB, phi);
    unsigned ignIndex = 0;
    if (pickFalseReg == LPU::IGN) {
      //reg assigned to pickTrue => make sure the original pick has %IGN for pickTrue;
      assert(pickTrueReg && pickTrueReg != LPU::IGN);
      assert(pickInstr->getOperand(3).getReg() == LPU::IGN);
      ignIndex = 3;
    } else {
      //reg assigned to pickFalse
      assert(pickTrueReg == LPU::IGN);
      assert(pickFalseReg && pickFalseReg != LPU::IGN);
      assert(pickInstr->getOperand(2).getReg() == LPU::IGN);
      ignIndex = 2;
    }
    MachineOperand &MO = pickInstr->getOperand(ignIndex);
    MO.substVirtReg(Reg, 0, TRI);
    MachineRegisterInfo *MRI = &thisMF->getRegInfo();
    MachineInstr *DefMI = MRI->getVRegDef(Reg);
    //if (TII.isPick(DefMI) && DefMI->getParent() == pickInstr->getParent()) {
    if (multiInputsPick.find(DefMI) != multiInputsPick.end()) {
      //make sure input src is before the pick
      assert(DefMI->getParent() == pickInstr->getParent());
      pickInstr->removeFromParent();
      DefMI->getParent()->insertAfter(DefMI, pickInstr); 
    }
    patched = true;
  }
  if (patched) {
    return NULL;
  } else {
    return pickInstr;
  }
}


MachineInstr* LPUCvtCFDFPass::insertPICKForReg(MachineBasicBlock* ctrlBB, unsigned Reg,
  MachineBasicBlock* inBB, MachineInstr* phi, unsigned pickReg) {
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
  MachineInstr* result = nullptr;
  MachineBasicBlock::iterator loc = ctrlBB->getFirstTerminator();
  MachineInstr* bi = loc;
  if (!pickReg) {
    pickReg = MRI->createVirtualRegister(TRC);
  }
  assert(bi->getOperand(0).isReg());
  unsigned predReg = bi->getOperand(0).getReg();
  unsigned pickFalseReg = 0, pickTrueReg = 0;
  assignPICKSrcForReg(pickFalseReg, pickTrueReg, Reg, ctrlBB, inBB, phi);
  const unsigned pickOpcode = TII.getPickSwitchOpcode(TRC, true /*pick op*/);
  MachineInstr *pickInst = BuildMI(*phi->getParent(), phi, DebugLoc(), TII.get(pickOpcode), pickReg).addReg(predReg).
    addReg(pickFalseReg).
    addReg(pickTrueReg);
  pickInst->setFlag(MachineInstr::NonSequential);
  multiInputsPick.insert(pickInst);
  return pickInst;
}

void LPUCvtCFDFPass::assignPICKSrcForReg(unsigned &pickFalseReg, unsigned &pickTrueReg, unsigned Reg, MachineBasicBlock* ctrlBB, MachineBasicBlock* inBB, MachineInstr* phi) {
  if (inBB) {
    ControlDependenceNode* inNode = CDG->getNode(inBB);
    ControlDependenceNode* ctrlNode = CDG->getNode(ctrlBB);
    if (ctrlNode->isFalseChild(inNode)) {
      pickFalseReg = Reg;
      pickTrueReg = LPU::IGN;
    }
    else {
      pickTrueReg = Reg;
      pickFalseReg = LPU::IGN;
    }
  }
  else {
    MachineBasicBlock* mbb = phi->getParent();
    //assert(DT->dominates(ctrlBB, mbb));
		if (CDG->getEdgeType(ctrlBB, mbb) == ControlDependenceNode::TRUE) {
      pickTrueReg = Reg;
      pickFalseReg = LPU::IGN;
    } else {
      pickFalseReg = Reg;
      pickTrueReg = LPU::IGN;
    }
  }
}

void LPUCvtCFDFPass::replaceIfFooterPhiSeq() {
  typedef po_iterator<MachineBasicBlock *> po_cfg_iterator;
  const TargetMachine &TM = thisMF->getTarget();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  MachineBasicBlock *root = thisMF->begin();
  for (po_cfg_iterator itermbb = po_cfg_iterator::begin(root), END = po_cfg_iterator::end(root); itermbb != END; ++itermbb) {
    MachineBasicBlock* mbb = *itermbb;
    MachineBasicBlock::iterator iterI = mbb->begin();
    while (iterI != mbb->end()) {
      MachineInstr *MI = iterI;
      ++iterI;
      if (!MI->isPHI()) continue;

      multiInputsPick.clear();
      unsigned dst = MI->getOperand(0).getReg();
      for (MIOperands MO(MI); MO.isValid(); ++MO) {
        if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
        if (MO->isUse()) {
          unsigned Reg = MO->getReg();
          //move to its incoming block operand
          ++MO;
          MachineBasicBlock* inBB = MO->getMBB();
          bool inBBFork = inBB->succ_size() > 1 && (!MLI->getLoopFor(inBB) || MLI->getLoopFor(inBB)->getLoopLatch() != inBB);
          if (!DT->dominates(inBB, mbb) && inBBFork) {
            MachineInstr* pickInstr = PatchOrInsertPickAtFork(inBB, dst, Reg, nullptr, MI, 0);
            if (!pickInstr) {
              //patched
              continue;
            } else {
              Reg = pickInstr->getOperand(0).getReg();
            }
          }

          MachineBasicBlock* ctrlBB = nullptr;
          bool patched = false;
          while (!DT->dominates(inBB, mbb)) {
            ControlDependenceNode* inNode = CDG->getNode(inBB);
            ControlDependenceNode* ctrlNode = nullptr;
            ctrlNode = getNonLatchParent(inNode);
            ctrlBB = ctrlNode->getBlock();
            unsigned pickReg = 0;
            if (DT->dominates(ctrlBB, mbb)) {
              pickReg = dst;
            }
            MachineInstr* pickInstr = PatchOrInsertPickAtFork(ctrlBB, dst, Reg, inBB, MI, pickReg);
            if (pickInstr == NULL) {
              patched = true;
              break;
            }
            inBB = ctrlBB;
            Reg = pickInstr->getOperand(0).getReg();
          }
          if (patched) continue;

          assert(DT->dominates(inBB, mbb));
          if (!ctrlBB) {
            //fall through
            MachineInstr* dMI = MRI->getVRegDef(Reg);
            MachineBasicBlock* DefBB = dMI->getParent();
            unsigned switchingDef = findSwitchingDstForReg(Reg, DefBB);
            if (switchingDef) {
              Reg = switchingDef;
            }
            PatchOrInsertPickAtFork(inBB, dst, Reg, nullptr, MI, dst);
          }
        }
      } //end of for MO
      MI->removeFromParent();
    }
  } //end of bb
}
 