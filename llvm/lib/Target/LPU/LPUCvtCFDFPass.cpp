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
#include "llvm/CodeGen/SlotIndexes.h"
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


// Flag for controlling code that deals with memory ordering.
//
//   0: No extra code added at all for ordering.  Often incorrect.
//   1: Linear ordering of all memops.  Dumb but should be correct.
//   2: Wavefront.   Stores inside a basic block are totally ordered.
//                   Loads ordered between the stores, but
//                   unordered with respect to each other.
//                   No reordering across basic blocks.
//   3: Aggressive.  Reordering allowed based on other compiler info.
//
static cl::opt<int>
OrderMemops("lpu-order-memops", cl::Hidden,
            cl::desc("LPU Specific: Order memory operations"),
            cl::init(1));


// The register class we are going to use for all the memory-op
// dependencies.  Technically they could be I0, but I don't know how
// happy LLVM will be with that.
const TargetRegisterClass* MemopRC = &LPU::I1RegClass;


// Width of vectors we are using for memory op calculations.
// TBD(jsukha): As far as I know, this value only affects performance,
// not correctness?
#define MEMDEP_VEC_WIDTH 8

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
	ControlDependenceNode* getNonLatchParent(ControlDependenceNode* anode, bool &oneAndOnly);
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
			AU.addRequired<MachinePostDominatorTree>();
      AU.setPreservesAll();
      MachineFunctionPass::getAnalysisUsage(AU);
    }
		void insertSWITCHForOperand(MachineOperand& MO, MachineBasicBlock* mbb, MachineInstr* phiIn = nullptr);
    void insertSWITCHForIf();
		void renameAcrossLoopForRepeat(MachineLoop *);
    void insertSWITCHForRepeat();
    void insertSWITCHForLoopExit();
		void SwitchDefAcrossLatch(unsigned Reg, MachineBasicBlock* mbb, MachineLoop* mloop);
    void replacePhiWithPICK();
    void replaceLoopHdrPhi();
		void generateCompletePickTreeForPhi(MachineInstr *);
		void generateDynamicPickTreeForPhi(MachineInstr *);
		void generateDynamicPreds();
		unsigned getEdgePred(MachineBasicBlock* fromBB, ControlDependenceNode::EdgeType childType);
		void setEdgePred(MachineBasicBlock* fromBB, ControlDependenceNode::EdgeType childType, unsigned ch);
		unsigned getBBPred(MachineBasicBlock* inBB);
		void setBBPred(MachineBasicBlock* inBB, unsigned ch);
		MachineInstr* getOrInsertPredMerge(MachineBasicBlock* mbb, MachineInstr* loc, unsigned e1, unsigned e2);
		unsigned computeEdgePred(MachineBasicBlock* fromBB, MachineBasicBlock* toBB);
		unsigned computeEdgePred(MachineBasicBlock* fromBB, ControlDependenceNode::EdgeType childType, MachineBasicBlock* toBB);
		unsigned computeBBPred(MachineBasicBlock *inBB);
		void TraceCtrl(MachineBasicBlock* inBB, MachineBasicBlock* mbb, unsigned Reg, unsigned dst, MachineInstr* MI);
    void LowerXPhi(SmallVectorImpl<std::pair<unsigned, unsigned> *> &pred2values, MachineInstr *MI);
		bool CheckPhiInputBB(MachineBasicBlock* inBB, MachineBasicBlock* mbb);
    void replaceIfFooterPhiSeq();
  	void assignLicForDF();
  	void removeBranch();
    void linearizeCFG();
    unsigned findSwitchingDstForReg(unsigned Reg, MachineBasicBlock* mbb);
    void handleAllConstantInputs();
    void releaseMemory() override;


    // TBD(jsukha): Experimental code for ordering of memory ops.
    void addMemoryOrderingConstraints();

    // Helper methods:

    // Create a new OLD/OST instruction, to replace an existing LD /
    // ST instruction.
    //  issued_reg is the register to define as the extra output
    //  ready_reg is the register which is the extra input
    MachineInstr* convert_memop_ins(MachineInstr* memop,
                                    unsigned new_opcode,
                                    const LPUInstrInfo& TII,
                                    unsigned issued_reg,
                                    unsigned ready_reg);

    // Create a dependency chain in virtual registers through the
    // basic block BB.
    //
    //   mem_in_reg is the virtual register number being used as
    //   input, i.e., the "source" for all the memory ops in this
    //   block.
    //
    //   This function returns the virtual register that is the "sink"
    //   of all the memory operations in this block.  The returned
    //   register might be the same as the source "mem_in_reg" if
    //   there are no memory operations in this block.
    //
    // This method also converts the LD/ST instructions into OLD/OST
    // instructions, as they are encountered.
    //
    // linear version of this function links all memory operations in
    // the block together in a single chain.
    //
    unsigned convert_block_memops_linear(MachineFunction::iterator& BB,
                                         unsigned mem_in_reg);

    // Wavefront version.   Same conceptual functionality as linear version,
    // but more optimized.
    //
    // Only serializes stores in a block, but allows loads to occur in
    // parallel between stores.
    unsigned convert_block_memops_wavefront(MachineFunction::iterator& BB,
                                            unsigned mem_in_reg);

    // Merge all the .i1 registers stored in "current_wavefront" into
    // a single output register.
    // Returns the output register, or "input_mem_reg" if
    // current_wavefront is empty.
    //
    // Note that this method has several side-effects:
    //  (a) It inserts the merge instructions after
    //      instruction MI in BB, or before the last terminator in the
    //      block if MI == NULL, and
    //  (b) It clears current_wavefront.
    unsigned merge_dependency_signals(MachineFunction::iterator& BB,
                                      MachineInstr* MI,
                                      SmallVector<unsigned, MEMDEP_VEC_WIDTH>* current_wavefront,
                                      unsigned input_mem_reg);


    void createMemInRegisterDefs(DenseMap<MachineBasicBlock*, unsigned>& blockToMemIn,
                                 DenseMap<MachineBasicBlock*, unsigned>& blockToMemOut);



  private:
    MachineFunction *thisMF;
    MachineDominatorTree *DT;
		MachinePostDominatorTree *PDT;
    ControlDependenceGraph *CDG;
    MachineLoopInfo *MLI;
    DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *> bb2switch;  //switch for Reg added in bb
    DenseMap<MachineBasicBlock *, SmallVectorImpl<MachineInstr *>* > bb2predcpy;
    DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *> bb2pick;  //switch for Reg added in bb
		DenseMap<MachineBasicBlock*, SmallVectorImpl<unsigned>* > edgepreds;
		DenseMap<MachineBasicBlock *, unsigned> bbpreds;
		DenseMap<MachineBasicBlock*, MachineInstr*> bb2predmerge;
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
	bb2switch.clear();
	bb2pick.clear();
	bb2predcpy.clear();
	multiInputsPick.clear();
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
	bb2switch.clear();

  DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *> ::iterator itmp = bb2pick.begin();
  while (itmp != bb2pick.end()) {
    DenseMap<unsigned, MachineInstr *>* reg2pick = itmp->getSecond();
    ++itmp;
    delete reg2pick;
  }
	bb2pick.clear();

  DenseMap<MachineBasicBlock *, SmallVectorImpl<MachineInstr *> *> ::iterator itmv = bb2predcpy.begin();
  while (itmv != bb2predcpy.end()) {
    SmallVectorImpl<MachineInstr *>* instrv = itmv->getSecond();
    ++itmv;
    delete instrv;
  }
	bb2predcpy.clear();

	DenseMap<MachineBasicBlock *, SmallVectorImpl<unsigned> *> ::iterator itedge = edgepreds.begin();
	while (itedge != edgepreds.end()) {
		SmallVectorImpl<unsigned>* edges = itedge->getSecond();
		++itedge;
		delete edges;
	}
	edgepreds.clear();

}


void LPUCvtCFDFPass::replacePhiWithPICK() {
  replaceLoopHdrPhi();
  replaceIfFooterPhiSeq();
}

//return the first non latch parent found or NULL
ControlDependenceNode* LPUCvtCFDFPass::getNonLatchParent(ControlDependenceNode* anode, bool &oneAndOnly ) {
	ControlDependenceNode* pcdn = nullptr;
	if (anode->getNumParents() == 0) return pcdn;
	for (ControlDependenceNode::node_iterator pnode = anode->parent_begin(), pend = anode->parent_end(); pnode != pend; ++pnode) {
		MachineBasicBlock* pbb = (*pnode)->getBlock();
		if (!pbb) continue; //root of CDG is a fake node
		if (MLI->getLoopFor(pbb) == NULL ||
			MLI->getLoopFor(pbb)->getLoopLatch() != pbb) {
			if (oneAndOnly && pcdn) {
				DEBUG(errs() << "WARNING: CDG node has more than one if parents\n");
				//assert(false && "CDG node has more than one if parent");
				oneAndOnly = false;
				return nullptr;
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
	PDT = &getAnalysis<MachinePostDominatorTree>();
	if (PDT->getRootNode() == nullptr) return false;
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

  // TBD(jsukha): Experimental code to add dependencies for memory
  // operations.
  //
  // This step should run before the main dataflow conversion because
  // it introduces extra dependencies through virtual registers than
  // the dataflow conversion must also deal with.
  if (OrderMemops > 0) {
    addMemoryOrderingConstraints();
  }
#if 0
	{
		errs() << "LPUCvtCFDFPass after memoryop order" << ":\n";
		MF.print(errs(), getAnalysisIfAvailable<SlotIndexes>());
	}
#endif

	insertSWITCHForIf();
#if 0
  {
    errs() << "LPUCvtCFDFPass before xphi" << ":\n";
    MF.print(errs(), getAnalysisIfAvailable<SlotIndexes>());
  }
#endif
	generateDynamicPreds();

	insertSWITCHForRepeat();
  insertSWITCHForLoopExit();
  replacePhiWithPICK();
  handleAllConstantInputs();
#if 0
	{
	  errs() << "LPUCvtCFDFPass before LIC allocation" << ":\n";
		MF.print(errs(), getAnalysisIfAvailable<SlotIndexes>());
	}
#endif
  assignLicForDF();
#if 0
  {
    errs() << "LPUCvtCFDFPass after LIC allocation" << ":\n";
    MF.print(errs(), getAnalysisIfAvailable<SlotIndexes>());
  }
#endif
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
		MachineInstr *switchInst;
    switchInst = BuildMI(*cdgpBB, loc, DebugLoc(), TII.get(switchOpcode),
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
  SmallVectorImpl<MachineInstr *>* predcpyVec = nullptr;
  if (bb2predcpy.find(cdgpBB) == bb2predcpy.end()) {
    predcpyVec = insertPredCpy(cdgpBB);
    bb2predcpy[cdgpBB] = predcpyVec;
  } else {
    predcpyVec = bb2predcpy[cdgpBB];
  }
  return predcpyVec;
}

//TODO: rename for repeat
void LPUCvtCFDFPass::renameAcrossLoopForRepeat(MachineLoop* L) {
	const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
	MachineRegisterInfo *MRI = &thisMF->getRegInfo();
	for (MachineLoop::iterator LI = L->begin(), LE = L->end(); LI != LE; ++LI) {
		renameAcrossLoopForRepeat(*LI);
		MachineLoop *mloop = *LI;
		for (MachineLoop::block_iterator BI = mloop->block_begin(), BE = mloop->block_end(); BI != BE; ++BI) {
			MachineBasicBlock* mbb = *BI;
			//only conside blocks in the  urrent loop level, blocks in the nested level are done before.
			if (MLI->getLoopFor(mbb) != mloop) continue;
			for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
				MachineInstr *MI = I;
				for (MIOperands MO(MI); MO.isValid(); ++MO) {
					if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
					unsigned Reg = MO->getReg();
					if (MO->isUse()) {
						MachineInstr *DefMI = MRI->getVRegDef(Reg);
						MachineBasicBlock *dmbb = DefMI->getParent();
						MachineLoop* dmloop = MLI->getLoopFor(dmbb);

						//def is in immediate nesting level, this including def not in any loop at all
						if (mloop->getParentLoop() == dmloop || mloop == dmloop) continue;

						//def outside the loop of use, and not in the immediate nesting level
						if ((!dmloop || dmloop->contains(mloop)) && DT->properlyDominates(dmbb, mbb)) {
							MachineBasicBlock* landingPad = mloop->getLoopPreheader();
							//TODO:: create the landing pad if can't find one
							assert(landingPad && "can't find loop preheader as landing pad for renaming");
							const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
							const unsigned moveOpcode = TII.getMoveOpcode(TRC);
							unsigned cpyReg = MRI->createVirtualRegister(TRC);
							MachineInstr *cpyInst = BuildMI(*landingPad, landingPad->getFirstTerminator(), DebugLoc(), TII.get(moveOpcode), cpyReg).addReg(Reg);
							cpyInst->setFlag(MachineInstr::NonSequential);
							MachineRegisterInfo::use_iterator UI = MRI->use_begin(Reg);
							while (UI != MRI->use_end()) {
								MachineOperand &UseMO = *UI;
								MachineInstr *UseMI = UseMO.getParent();
								MachineBasicBlock* UseBB = UseMI->getParent();
								++UI;
								if (MLI->getLoopFor(UseBB) && MLI->getLoopFor(UseBB) == mloop) {
									UseMO.setReg(cpyReg);
								}
							}
						}
					}
				}
			}//end of for MI
		}
	}
}


void LPUCvtCFDFPass::insertSWITCHForOperand(MachineOperand& MO, MachineBasicBlock* mbb, MachineInstr* phiIn) {
	const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
	MachineRegisterInfo *MRI = &thisMF->getRegInfo();
	if (!MO.isReg() || !TargetRegisterInfo::isVirtualRegister(MO.getReg())) return;
	unsigned Reg = MO.getReg();
	// process uses
	if (MO.isUse()) {
		ControlDependenceNode *unode = CDG->getNode(mbb);
		CDGRegion *uregion = CDG->getRegion(unode);
		assert(uregion);
		MachineInstr *DefMI = MRI->getVRegDef(Reg);

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

				SmallVector<MachineInstr*, 8> NewPHIs;
				MachineSSAUpdater SSAUpdate(*thisMF, &NewPHIs);
				const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
				unsigned pickVReg = MRI->createVirtualRegister(TRC);
				SSAUpdate.Initialize(pickVReg);
				unsigned numIfParent = 0;
				unsigned newVReg;
				for (ControlDependenceNode::node_iterator uparent = unode->parent_begin(), uparent_end = unode->parent_end();
					uparent != uparent_end; ++uparent) {
					ControlDependenceNode *upnode = *uparent;
					MachineBasicBlock *upbb = upnode->getBlock();
					if (!upbb) {
						//this is typical define inside loop, used outside loop on the main execution path
						continue;
					}
					if (mbb == upbb) {
						//mbb is a loop latch node, use inside a loop will be take care of in HandleUseInLoop
						continue;
					}
					if (MLI->getLoopFor(upbb) &&
						MLI->getLoopFor(upbb)->getLoopLatch() == upbb) {
						//no need to conside backedge for if-statements handling
						continue;
					}
					if (DT->dominates(dmbb, upbb)) 	{ //including dmbb itself
						numIfParent++;

						assert((MLI->getLoopFor(dmbb) == NULL ||
							MLI->getLoopFor(dmbb) != MLI->getLoopFor(upbb) ||
							MLI->getLoopFor(dmbb)->getLoopLatch() != dmbb) &&
							"latch node can't forward dominate nodes inside its own loop");

						MachineInstr *defSwitchInstr = getOrInsertSWITCHForReg(Reg, upbb);
						unsigned switchFalseReg = defSwitchInstr->getOperand(0).getReg();
						unsigned switchTrueReg = defSwitchInstr->getOperand(1).getReg();
						if (upnode->isFalseChild(unode)) {
							//rename Reg to switchFalseReg
							newVReg = switchFalseReg;
						}	else {
							//rename it to switchTrueReg
							newVReg = switchTrueReg;
						}
						SSAUpdate.AddAvailableValue(upbb, newVReg);
					}
				} //end of for (parent

				if (phiIn) {
					if (numIfParent == 1) {
						MO.setReg(newVReg);
					}	else if (numIfParent > 1) {
						SSAUpdate.RewriteUse(MO);
					}
				}
				else {
					MachineRegisterInfo::use_iterator UI = MRI->use_begin(Reg);
					while (UI != MRI->use_end()) {
						MachineOperand &UseMO = *UI;
						MachineInstr *UseMI = UseMO.getParent();
						++UI;
						if (UseMI->getParent() == mbb) {
							if (numIfParent == 1) {
								UseMO.setReg(newVReg);
							}	else if (numIfParent > 1) {
								SSAUpdate.RewriteUse(UseMO);
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
  ControlDependenceNode *root = CDG->getRoot();
	for (po_cdg_iterator DTN = po_cdg_iterator::begin(root), END = po_cdg_iterator::end(root); DTN != END; ++DTN) {
		MachineBasicBlock *mbb = DTN->getBlock();
		if (!mbb) continue; //root node has no bb
		// process each instruction in BB
		for (MachineBasicBlock::succ_iterator isucc = mbb->succ_begin(); isucc != mbb->succ_end(); ++isucc) {
			MachineBasicBlock* succBB = *isucc;
			ControlDependenceNode* succNode = CDG->getNode(succBB);
			//phi in succNode has been processed or generated before
			if (!succNode->isParent(*DTN)) {
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
									(mbb->succ_size() == 2 && MLI->getLoopFor(mbb) && MLI->getLoopFor(mbb)->getLoopLatch() == mbb)) {
									insertSWITCHForOperand(mOpnd, mbb, iPhi);
								}
								else {
									//mbb itself is a fork
									MachineInstr *defSwitchInstr = getOrInsertSWITCHForReg(Reg, mbb);
									unsigned switchFalseReg = defSwitchInstr->getOperand(0).getReg();
									unsigned switchTrueReg = defSwitchInstr->getOperand(1).getReg();
									unsigned newVReg;
									if (CDG->getEdgeType(mbb, succBB, true) == ControlDependenceNode::TRUE) {
										newVReg = switchTrueReg;
									}
									else {
										assert(CDG->getEdgeType(mbb, succBB, true) == ControlDependenceNode::FALSE);
										newVReg = switchFalseReg;
									}
									mOpnd.setReg(newVReg);
								}
							}
						}
					}
				}
			}
			for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
				MachineInstr *MI = I;
				//phi block control depends on its input blocks, need to handle it here
				//if (MI->isPHI()) continue; //care about forks, not joints
				for (MIOperands MO(MI); MO.isValid(); ++MO) {
					insertSWITCHForOperand(*MO, mbb);
				}
			}//end of for MI
		}
	}//end of for DTN(mbb)
}



void LPUCvtCFDFPass::SwitchDefAcrossLatch(unsigned Reg, MachineBasicBlock* mbb, MachineLoop* mloop) {
	MachineRegisterInfo *MRI = &thisMF->getRegInfo();
	MachineBasicBlock *latchBB = mloop->getLoopLatch();
	ControlDependenceNode *mLatch = CDG->getNode(latchBB);
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
				//assert(UseMI->isPHI());
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
				}
				else {
					//rename it to switchTrueReg
					newVReg = switchTrueReg;
				}
				MachineBasicBlock* lphdr = mloop->getHeader();
				// Rewrite uses that outside of the original def's block, inside the loop
				if (MLI->getLoopFor(UseMI->getParent()) == mloop &&
					UseMI->getParent() == lphdr &&
					UseMI->isPHI()) {
					//rename loop header Phi
					UseMO.setReg(newVReg);
				}
			}
			else {   //mloop != defLoop
							 //two possibilites: a) def dom use;  b) def !dom use;
							 //two cases: each can only have one nesting level difference
							 // 1) def inside a loop, use outside the loop as LCSSA Phi with single input
							 // 2) def outside a loop, use inside the loop, not handled here
							 //use, def in different region cross latch
				bool isUseEnclosingDef =  MLI->getLoopFor(UseBB) == NULL ||
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
					}
					else {
						//rename it to switchFalseReg
						newVReg = switchFalseReg;
					}
					// Rewrite uses that outside of the original def's block, inside the loop
					//renameLCSSAPhi or other cross boundary uses
					UseMO.setReg(newVReg);
				} else {
					// use not enclosing def, def and use in different regions
					// assert(use have to be a switch from the repeat handling pass, or def is a switch from the if handling pass
					// or loop hdr Phi generated by SSAUpdater in handling repeat case)
				}
			}
		}
	}//end of while (use)
}

//focus on def
void LPUCvtCFDFPass::insertSWITCHForLoopExit() {
  typedef po_iterator<ControlDependenceNode *> po_cdg_iterator;
	DenseMap<MachineBasicBlock *, std::set<unsigned> *> LCSwitch;

  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
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
     //no SWITCH is needed if loop latch has no exit edge
      continue;
    }
    ControlDependenceNode *mLatch = CDG->getNode(latchBB);
    //inside a loop
    for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
      MachineInstr *MI = I;

      //avoid infinitive recursive
      if (TII.isSwitch(MI) && mbb == latchBB) {
				MachineBasicBlock* exitBB = mloop->getExitBlock();
				assert(exitBB && "multiple exit blocks from loop latch");

				unsigned switchOut = (mLatch->isFalseChild(CDG->getNode(exitBB))) ? 0 : 1;
				std::set<unsigned>* LCSwitchs;
				if (LCSwitch.find(exitBB) == LCSwitch.end()) {
					LCSwitchs = new std::set<unsigned>;
					LCSwitch[exitBB] = LCSwitchs;
				}	else {
					LCSwitchs = LCSwitch.find(exitBB)->getSecond();
				}
				LCSwitchs->insert(MI->getOperand(switchOut).getReg());
				continue;
      }
      for (MIOperands MO(MI); MO.isValid(); ++MO) {
        if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
        unsigned Reg = MO->getReg();
        // process defs
        if (MO->isDef()) {
					SwitchDefAcrossLatch(Reg, mbb, mloop);
        }
      }
    }//end of for MI

		if (LCSwitch.find(mbb) != LCSwitch.end()) {
			std::set<unsigned>* LCSwitchs = LCSwitch.find(mbb)->getSecond();
			for (std::set<unsigned>::iterator iReg = LCSwitchs->begin(); iReg != LCSwitchs->end(); ++iReg) {
				SwitchDefAcrossLatch(*iReg, mbb, mloop);
			}
		}
  }//end of for DTN(mbb)

	//release memory
	DenseMap<MachineBasicBlock *, std::set<unsigned> *> ::iterator itm = LCSwitch.begin();
	while (itm != LCSwitch.end()) {
		std::set<unsigned>* regs = itm->getSecond();
		++itm;
		delete regs;
	}
	LCSwitch.clear();
}


//focus on uses
void LPUCvtCFDFPass::insertSWITCHForRepeat() {
	for (MachineLoopInfo::iterator LI = MLI->begin(), LE = MLI->end(); LI != LE; ++LI) {
		renameAcrossLoopForRepeat(*LI);
	}
#if 0
	{
		errs() << "after rename for repeat" << ":\n";
		thisMF->print(errs(), getAnalysisIfAvailable<SlotIndexes>());
	}
#endif

  typedef po_iterator<ControlDependenceNode *> po_cdg_iterator;
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
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
              assert(TII.isMOV(defInstr));
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
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
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
      unsigned numUse = 0;
			MachineOperand* backEdgeInput = nullptr;
			MachineOperand* initInput = nullptr;
			unsigned numOpnd = 0;
			unsigned backEdgeIndex = 0;
			unsigned dst = MI->getOperand(0).getReg();

      for (MIOperands MO(MI); MO.isValid(); ++MO, ++numOpnd) {
        if (!MO->isReg()) continue;
        // process use at loop level
        if (MO->isUse()) {
					++numUse;
					MachineOperand& mOpnd = *MO;
					++MO;
					++numOpnd;
					MachineBasicBlock* inBB = MO->getMBB();
					if (inBB == latchBB) {
						backEdgeInput = &mOpnd;
						backEdgeIndex = numOpnd - 1;
					}	else {
						initInput = &mOpnd;
					}
        }
      } //end for MO
			if (numUse > 2) {
				//loop hdr phi has more than 2 inputs, 
				//remove backedge input reduce it to if-foot phi case to be handled by if-footer phi pass
				initInput = &MI->getOperand(0);
				const TargetRegisterClass *TRC = MRI->getRegClass(MI->getOperand(0).getReg());
				unsigned renameReg = MRI->createVirtualRegister(TRC);
				initInput->setReg(renameReg);
			}
			MachineOperand* pickFalse;
			MachineOperand* pickTrue;
      if (latchBB->succ_size() > 1) {
        assert(mLatch->isChild(*DTN));
        if (mLatch->isFalseChild(*DTN)) {
          pickFalse = backEdgeInput;
          pickTrue = initInput;
        } else {
          pickTrue = backEdgeInput;
          pickFalse = initInput;
        }
      } else {
        //LLVM 3.6 buggy latch, loop with > 1 exits
        //LLVM 3.6 buggy loop latch with no exit edge from latch, fixed in 3.9
        ControlDependenceNode* latchNode = CDG->getNode(latchBB);
        assert(latchNode->getNumParents() == 1);
        ControlDependenceNode* ctrlNode = *latchNode->parent_begin();
        MachineInstr* bi = ctrlNode->getBlock()->getFirstInstrTerminator();
        if (CDG->getEdgeType(bi->getParent(), latchBB, true) == ControlDependenceNode::FALSE) {
					pickFalse = backEdgeInput;
					pickTrue = initInput;
        } else {
					pickTrue = backEdgeInput;
          pickFalse = initInput;
        }
      }
      unsigned predReg = (*predCpy)[0]->getOperand(0).getReg();
      const TargetRegisterClass *TRC = MRI->getRegClass(dst);
      const unsigned pickOpcode = TII.getPickSwitchOpcode(TRC, true /*pick op*/);
      //generate PICK, and insert before MI
			MachineInstr *pickInst = nullptr;
			if (pickFalse->isReg() && pickTrue->isReg()) {
				pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII.get(pickOpcode), dst).addReg(predReg).
					                  addReg(pickFalse->getReg()).addReg(pickTrue->getReg());
			}	else if (pickFalse->isReg()) {
				pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII.get(pickOpcode), dst).addReg(predReg).
					                  addReg(pickFalse->getReg()).addOperand(*pickTrue);
			}	else if (pickTrue->isReg()) {
				pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII.get(pickOpcode), dst).addReg(predReg).
														addOperand(*pickFalse).addReg(pickTrue->getReg());
			}	else {
				pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII.get(pickOpcode), dst).addReg(predReg).
				                  	addOperand(*pickFalse).addOperand(*pickTrue);
			}

			pickInst->setFlag(MachineInstr::NonSequential);
			MI->removeFromParent();
			if (numUse > 2) {
				//move phi before the pick
				MachineBasicBlock::iterator tmpI = pickInst;
				mbb->insert(tmpI, MI);
				MI->RemoveOperand(backEdgeIndex);
				MI->RemoveOperand(backEdgeIndex);
			}
    }
  }
}


void LPUCvtCFDFPass::assignLicForDF() {
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *thisMF->getSubtarget().getRegisterInfo();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  LPUMachineFunctionInfo *LMFI = thisMF->getInfo<LPUMachineFunctionInfo>();
  std::deque<unsigned> renameQueue;
	renameQueue.clear();
	std::set<unsigned> pinedVReg;
	for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
		for (MachineBasicBlock::iterator MI = BB->begin(), EI = BB->end(); MI != EI; ++MI) {
			if (MI->isPHI()) {
				for (MIOperands MO(MI); MO.isValid(); ++MO) {
					if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
					unsigned Reg = MO->getReg();
					pinedVReg.insert(Reg);
				}
			}
		}
	}

  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator MI = BB->begin(), EI = BB->end(); MI != EI; ++MI) {
      if (TII.isPick(MI) || TII.isSwitch(MI) || 
          MI->getOpcode() == LPU::PREDMERGE || 
          MI->getOpcode() == LPU::PREDPROP || 
          MI->getOpcode() == LPU::OR1) {
        for (MIOperands MO(MI); MO.isValid(); ++MO) {
          if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
          unsigned Reg = MO->getReg();
          renameQueue.push_back(Reg);
        }
      }
    }
  }

  while (!renameQueue.empty()) {
    unsigned dReg = renameQueue.front();
    renameQueue.pop_front();
    MachineInstr *DefMI = MRI->getVRegDef(dReg);
    if (!DefMI ) continue;
    MachineOperand *DefMO = DefMI->findRegisterDefOperand(dReg);
		if (DefMI->isPHI()) continue;

    // We've decided to convert this def to a LIC. If it was dead, we must send
    // it to the %ign LIC rather than allocating a new one.
    assert(DefMO->isDef() && "Trying to reason about uses of a non-def.");
    if (MRI->use_empty(dReg)) {
        DefMI->substituteRegister(dReg, LPU::IGN, 0, TRI);
        continue;
    }

		const TargetRegisterClass *TRC = MRI->getRegClass(dReg);
    const TargetRegisterClass* new_LIC_RC = LMFI->licRCFromGenRC(TRC);
    assert(new_LIC_RC && "unknown LPU register class");
    unsigned phyReg = LMFI->allocateLIC(new_LIC_RC);

		if (TII.isSwitch(DefMI)) {
			unsigned trueReg = DefMI->getOperand(1).getReg();
			unsigned falseReg = DefMI->getOperand(0).getReg();
			if (pinedVReg.find(trueReg) != pinedVReg.end() || pinedVReg.find(falseReg) != pinedVReg.end()) {
                DefMI->clearFlag(MachineInstr::NonSequential);
				continue;
			}
		}	else if (TII.isMOV(DefMI)) {
			unsigned dstReg = DefMI->getOperand(0).getReg();
			if (pinedVReg.find(dstReg) != pinedVReg.end()) {
				DefMI->clearFlag(MachineInstr::NonSequential);
				continue;
			}
		}

		DefMI->substituteRegister(dReg, phyReg, 0, TRI);

    MachineRegisterInfo::use_iterator UI = MRI->use_begin(dReg);
    while (UI != MRI->use_end()) {
      MachineOperand &UseMO = *UI;
      ++UI;
			UseMO.setReg(phyReg);
    }

    for (MIOperands MO(DefMI); MO.isValid(); ++MO) {
      if (!MO->isReg() || &*MO == DefMO || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
      unsigned Reg = MO->getReg();
      renameQueue.push_back(Reg);
    }
  }

  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator MI = BB->begin(), EI = BB->end(); MI != EI; ++MI) {
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

          // Note: this avoids magic constants, but requires that the LIC
          // virtual registers be defined at the end of the enum in
          // LPUGenRegisterInfo.inc.
          if ((Reg < LPU::CI0_0 || Reg >= LPU::NUM_TARGET_REGS) &&
					     Reg != LPU::IGN ) {
            allLics = false;
            break;
          }
        }
      }

      // Check for instructions where all the uses are constants.
      // These instructions shouldn't be moved on to dataflow units,
      // because they keep firing infinitely.
      bool allImmediateUses = true;
      for (MIOperands MO(MI); MO.isValid(); ++MO) {
        // Skip defs.
        if (MO->isReg() && MO->isDef())
          continue;
        if (!(MO->isImm() || MO->isCImm() || MO->isFPImm())) {
          allImmediateUses = false;
          break;
        }
      }

      //DEBUG(errs() << "Machine ins " << *MI << ": allLics = " << allLics << ", allImmediateUses = " << allImmediateUses << "\n");
      if (allLics && !allImmediateUses) {
        MI->setFlag(MachineInstr::NonSequential);
			}
			if (!allLics && TII.isSwitch(MI)) {
				MI->clearFlag(MachineInstr::NonSequential);
			}
    }
  }
}


void LPUCvtCFDFPass::handleAllConstantInputs() {
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();

  std::deque<unsigned> renameQueue;
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
		MachineBasicBlock* mbb = BB;
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
				MachineInstr *pickInst = nullptr;
				MachineInstr *switchInst = nullptr;
				const unsigned switchOpcode = TII.getPickSwitchOpcode(TRC, false);
				const unsigned pickOpcode = TII.getPickSwitchOpcode(TRC, true);
				unsigned pickFalseReg = LPU::IGN, pickTrueReg = LPU::IGN;
				unsigned switchFalse = LPU::IGN, switchTrue = LPU::IGN;
				int parentN = 0;
				for (ControlDependenceNode::node_iterator uparent = mNode->parent_begin(), uparent_end = mNode->parent_end();
					uparent != uparent_end; ++uparent) {
					ControlDependenceNode *upnode = *uparent;
					MachineBasicBlock *upbb = upnode->getBlock();
					if (!upbb) {
						//this is typical define inside loop, used outside loop on the main execution path
						continue;
					}
					if (mbb == upbb) {
						//mbb is a loop latch node, use inside a loop will be taken care of in HandleUseInLoop
						continue;
					}
					//TBD::can't skip loop latch upbb, llvm 3.6 put "mov 0.0000" inside a loop as manifested in
					// 022-regression/t006_HACCmk_v0_O2.s
#if 0
					if (MLI->getLoopFor(upbb) &&
						MLI->getLoopFor(upbb)->getLoopLatch() == upbb) {
						//no need to conside backedge for if-statements handling
						continue;
					}
#endif
					++parentN;
					MachineInstr* bi = upnode->getBlock()->getFirstTerminator();
					assert(bi->getOperand(0).isReg());
					unsigned predReg = bi->getOperand(0).getReg();
					unsigned pickReg = 0;
					if (parentN == 1) {
						if (upnode->isFalseChild(mNode)) {
							switchFalse = MI->getOperand(0).getReg();
						}	else {
							switchTrue = MI->getOperand(0).getReg();
						}
						switchInst = BuildMI(*BB, MI, DebugLoc(), TII.get(switchOpcode), switchFalse).addReg(switchTrue, RegState::Define).
							addReg(predReg).addOperand(MI->getOperand(1));
						switchInst->setFlag(MachineInstr::NonSequential);
					}	else {
						if (parentN == 2) {
							unsigned renameReg = MRI->createVirtualRegister(TRC);
							unsigned index = (switchFalse == LPU::IGN) ? 1 : 0;
							switchInst->getOperand(index).setReg(renameReg);
							pickTrueReg = renameReg;
							pickFalseReg = renameReg;
						}
						pickReg = MRI->createVirtualRegister(TRC);
						if (upnode->isFalseChild(mNode)) {
							pickInst = BuildMI(*BB, MI, DebugLoc(), TII.get(pickOpcode), pickReg).addReg(predReg).
								addOperand(MI->getOperand(1)).
								addReg(pickTrueReg);
						}	else {
							pickInst = BuildMI(*BB, MI, DebugLoc(), TII.get(pickOpcode), pickReg).addReg(predReg).
								addReg(pickFalseReg).
								addOperand(MI->getOperand(1));
						}
						pickInst->setFlag(MachineInstr::NonSequential);
						pickFalseReg = pickReg;
						pickTrueReg = pickReg;
					}
				}
				if (pickInst) {
					pickInst->getOperand(0).setReg(MI->getOperand(0).getReg());
				}
				if (switchInst) {
					MI->removeFromParent();
				}
      }
    }
  }
}




void LPUCvtCFDFPass::removeBranch() {
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
  const TargetRegisterInfo &TRI = *thisMF->getSubtarget().getRegisterInfo();
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
    } else {
      pickTrueReg = Reg;
      pickFalseReg = LPU::IGN;
    }
  } else {
    MachineBasicBlock* mbb = phi->getParent();
    //assert(DT->dominates(ctrlBB, mbb));
		if (CDG->getEdgeType(ctrlBB, mbb, true) == ControlDependenceNode::TRUE) {
      pickTrueReg = Reg;
      pickFalseReg = LPU::IGN;
    } else {
      pickFalseReg = Reg;
      pickTrueReg = LPU::IGN;
    }
  }
}


void LPUCvtCFDFPass::generateCompletePickTreeForPhi(MachineInstr* MI) {
	MachineRegisterInfo *MRI = &thisMF->getRegInfo();
	multiInputsPick.clear();
	MachineBasicBlock* mbb = MI->getParent();
	unsigned dst = MI->getOperand(0).getReg();
	for (MIOperands MO(MI); MO.isValid(); ++MO) {
		if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
		if (MO->isUse()) {
			unsigned Reg = MO->getReg();
			//move to its incoming block operand
			++MO;
			MachineBasicBlock* inBB = MO->getMBB();
			if (DT->dominates(inBB, mbb)) {
				//fall through
				MachineInstr* dMI = MRI->getVRegDef(Reg);
				MachineBasicBlock* DefBB = dMI->getParent();
				unsigned switchingDef = findSwitchingDstForReg(Reg, DefBB);
				if (switchingDef) {
					Reg = switchingDef;
				}
				PatchOrInsertPickAtFork(inBB, dst, Reg, nullptr, MI, dst);
				continue;
			}	else {
				bool inBBFork = inBB->succ_size() > 1 && (!MLI->getLoopFor(inBB) || MLI->getLoopFor(inBB)->getLoopLatch() != inBB);
				if (inBBFork) {
					MachineInstr* pickInstr = PatchOrInsertPickAtFork(inBB, dst, Reg, nullptr, MI, 0);
					if (!pickInstr) {
						//patched
						continue;  //to next MO
					}	else {
						Reg = pickInstr->getOperand(0).getReg();
					}
				}
				TraceCtrl(inBB, mbb, Reg, dst, MI);
			}
		}
	} //end of for MO
	MI->removeFromParent();
}

unsigned LPUCvtCFDFPass::getEdgePred(MachineBasicBlock* mbb, ControlDependenceNode::EdgeType childType) {
	if (edgepreds.find(mbb) == edgepreds.end()) return 0;
	return (*edgepreds[mbb])[childType];
}

void LPUCvtCFDFPass::setEdgePred(MachineBasicBlock* mbb, ControlDependenceNode::EdgeType childType, unsigned ch) {
	if (edgepreds.find(mbb) == edgepreds.end()) {
		SmallVectorImpl<unsigned>* childVect = new SmallVector<unsigned, 2>;
		childVect->push_back(0);
		childVect->push_back(0);
		edgepreds[mbb] = childVect;
	}
	(*edgepreds[mbb])[childType] = ch;
}

unsigned LPUCvtCFDFPass::getBBPred(MachineBasicBlock* mbb) {
	if (bbpreds.find(mbb) == bbpreds.end()) return 0;
	return bbpreds[mbb];
}


void LPUCvtCFDFPass::setBBPred(MachineBasicBlock* mbb, unsigned ch) {
	//don't set it twice
	assert(bbpreds.find(mbb) == bbpreds.end() && "LPU: Try to set bb pred twice");
	bbpreds[mbb] = ch;
}

unsigned LPUCvtCFDFPass::computeEdgePred(MachineBasicBlock* fromBB, MachineBasicBlock* toBB) {
	ControlDependenceNode* fromNode = CDG->getNode(fromBB);
	ControlDependenceNode* toNode = CDG->getNode(toBB);
	if (fromBB->succ_size() == 1 || fromNode->isParent(fromNode) || fromNode->isChild(fromNode)) {
		return computeBBPred(fromBB);
	} else if (fromNode->isFalseChild(toNode)) {
		return computeEdgePred(fromBB, ControlDependenceNode::FALSE, toBB);
	}	else if (fromNode->isTrueChild(toNode)) {
		return computeEdgePred(fromBB, ControlDependenceNode::TRUE, toBB);
	} else {
		assert(toBB->isPredecessor(fromBB));
		ControlDependenceNode::EdgeType edgeType = CDG->getEdgeType(fromBB, toBB);
		return computeEdgePred(fromBB, edgeType, toBB);
	}
}

	
unsigned LPUCvtCFDFPass::computeEdgePred(MachineBasicBlock* fromBB, ControlDependenceNode::EdgeType childType, MachineBasicBlock* toBB) {
	const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
	MachineRegisterInfo* MRI = &thisMF->getRegInfo();

	assert(fromBB->succ_size() == 2 && "LPU bb has more than 2 successor");
	if (unsigned edgeReg = getEdgePred(fromBB, childType)) {
		return edgeReg;
	}
	unsigned bbPredReg = computeBBPred(fromBB);
	if (!toBB) {
		ControlDependenceNode* fromNode = CDG->getNode(fromBB);
		ControlDependenceNode* toNode;
		if (childType == ControlDependenceNode::FALSE) {
			//TODO:: assert only have one false child.
			toNode = *fromNode->false_begin();
		}	else {
			toNode = *fromNode->true_begin();
		}
		toBB = toNode->getBlock();
	}
	//using loop as the unit of the region
	//reaching the boundary, generate switch 
  if (MLI->getLoopFor(toBB) && MLI->getLoopFor(toBB)->getHeader() == toBB) {
		MachineInstr* bi = fromBB->getFirstTerminator();
		unsigned switchFalseReg = MRI->createVirtualRegister(&LPU::I1RegClass);
		unsigned switchTrueReg = MRI->createVirtualRegister(&LPU::I1RegClass);
		assert(bi->getOperand(0).isReg());
		// generate switch op
		const unsigned switchOpcode = TII.getPickSwitchOpcode(&LPU::I1RegClass, false /*not pick op*/);
		//special handling for predprop/premerge in loop to avoid cycle of dependence
		BuildMI(*fromBB, bi, DebugLoc(), TII.get(switchOpcode),
			switchFalseReg).
			addReg(switchTrueReg, RegState::Define).
			addReg(bi->getOperand(0).getReg()).
			addReg(bbPredReg);
		setEdgePred(fromBB, ControlDependenceNode::FALSE, switchFalseReg);
		setEdgePred(fromBB, ControlDependenceNode::TRUE, switchTrueReg);
		if (childType == 0) {
			return switchFalseReg;
		}	else {
			return switchTrueReg;
		}
	}	else {
		unsigned falseEdge = MRI->createVirtualRegister(&LPU::I1RegClass);
		unsigned trueEdge = MRI->createVirtualRegister(&LPU::I1RegClass);
		MachineBasicBlock::iterator loc = fromBB->getFirstTerminator();
		MachineInstr* bi = loc;
		BuildMI(*fromBB, loc, DebugLoc(), TII.get(LPU::PREDPROP),
			falseEdge).addReg(trueEdge, RegState::Define).addReg(bbPredReg).addReg(bi->getOperand(0).getReg());
		setEdgePred(fromBB, ControlDependenceNode::FALSE, falseEdge);
		setEdgePred(fromBB, ControlDependenceNode::TRUE, trueEdge);
		return getEdgePred(fromBB, childType);
	}
}

unsigned LPUCvtCFDFPass::computeBBPred(MachineBasicBlock* inBB) {
	if (unsigned c = getBBPred(inBB)) {
		return c;
	}
	const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
	MachineRegisterInfo* MRI = &thisMF->getRegInfo();
	MachineBasicBlock* ctrlBB = nullptr;
	unsigned ctrlEdge;
	unsigned predBB = 0;
	ControlDependenceNode* inNode = CDG->getNode(inBB);
	for (ControlDependenceNode::node_iterator pnode = inNode->parent_begin(), pend = inNode->parent_end(); pnode != pend; ++pnode) {
		ControlDependenceNode* ctrlNode = *pnode;
		ctrlBB = ctrlNode->getBlock();

		if (!ctrlBB) { //root node has no bb
			//mov 1
			// Look up target register class corresponding to this register.
			MachineBasicBlock* entryBB = thisMF->begin();
      unsigned cpyReg = MRI->createVirtualRegister(&LPU::I1RegClass);
			const unsigned moveOpcode = TII.getMoveOpcode(&LPU::I1RegClass);
			BuildMI(*entryBB, entryBB->getFirstTerminator(), DebugLoc(), TII.get(moveOpcode), cpyReg).addImm(1);
			ctrlEdge = cpyReg;
		}	else {
			//bypass loop latch node
			if (MLI->getLoopFor(ctrlBB) && MLI->getLoopFor(ctrlBB)->getLoopLatch() == ctrlBB)
				continue;
			assert(ctrlBB->succ_size() == 2 && "LPU: bb has more than 2 successor");
			computeBBPred(ctrlBB);
			unsigned falseEdgeReg = computeEdgePred(ctrlBB, ControlDependenceNode::FALSE, inBB);
			unsigned trueEdgeReg = computeEdgePred(ctrlBB, ControlDependenceNode::TRUE, inBB);
			if (ctrlNode->isFalseChild(inNode)) {
				ctrlEdge = falseEdgeReg;
			}	else {
				ctrlEdge = trueEdgeReg;
			}
		}
		//merge predecessor if needed
		if (!predBB) {
			predBB = ctrlEdge;
		}	else {
			unsigned mergeEdge = MRI->createVirtualRegister(&LPU::I1RegClass);
      MachineBasicBlock::iterator loc = inBB->getFirstTerminator();
			BuildMI(*inBB, loc, DebugLoc(), TII.get(LPU::OR1), mergeEdge).addReg(predBB).addReg(ctrlEdge);
			predBB = mergeEdge;
		}
	}
	//be prudent and only save when necessary
	if (inBB->pred_size() > 1 || inBB->succ_size() > 1) {
		setBBPred(inBB, predBB);
	}
	return predBB;
}


MachineInstr* LPUCvtCFDFPass::getOrInsertPredMerge(MachineBasicBlock* mbb, MachineInstr* loc, unsigned e1, unsigned e2) {
	const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
	MachineRegisterInfo *MRI = &thisMF->getRegInfo();
	MachineInstr* predMergeInstr = nullptr;
	if (bb2predmerge.find(mbb) == bb2predmerge.end()) {
		unsigned indexReg = MRI->createVirtualRegister(&LPU::I1RegClass);
		predMergeInstr = BuildMI(*mbb, loc, DebugLoc(), TII.get(LPU::PREDMERGE),
			LPU::IGN).    //in a two-way merge, it is %IGN to eat the BB's pred, they will be computed using "or" consistently
			addReg(indexReg, RegState::Define). 
			addReg(e1).   //last processed edge
			addReg(e2); //current edge
		bb2predmerge[mbb] = predMergeInstr;
	}	else {
		predMergeInstr = bb2predmerge[mbb];
	}
	return predMergeInstr;
}

void LPUCvtCFDFPass::generateDynamicPickTreeForPhi(MachineInstr* MI) {
	assert(MI->isPHI());
	const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
	MachineRegisterInfo *MRI = &thisMF->getRegInfo();
	SmallVector<std::pair<unsigned, unsigned> *, 4> pred2values;
	MachineBasicBlock* mbb = MI->getParent();
	unsigned predBB = 0;
	MachineInstr* predMergeInstr = nullptr;

	for (MIOperands MO(MI); MO.isValid(); ++MO) {
		if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
		if (MO->isUse()) {
			unsigned Reg = MO->getReg();
			//move to its incoming block operand
			++MO;
			MachineBasicBlock* inBB = MO->getMBB();
			unsigned edgePred = computeEdgePred(inBB, mbb);
			std::pair<unsigned, unsigned>* pred2value = new std::pair<unsigned, unsigned>;
			pred2value->first = edgePred;
			pred2value->second = Reg;
			pred2values.push_back(pred2value);
			//merge incoming edge pred to generate BB pred
			if (!predBB) {
				predBB = edgePred;
			}	else if (MI->getNumOperands() == 5) {
				//two input phi: use PREDMERGE to avoid further lowering.
				predMergeInstr = getOrInsertPredMerge(mbb, MI, predBB,      //last processed edge
					                                             edgePred);   //current edge
			}
		}
	} //end of for MO

  unsigned dst = MI->getOperand(0).getReg();
	//if we have two-way predMerge available, use predmerge/pick combination to generated pick directly
	if (predMergeInstr) {
		assert(MI->getNumOperands() == 5);
		unsigned reg1 = MI->getOperand(1).getReg();
		unsigned reg2 = MI->getOperand(3).getReg();
		const TargetRegisterClass *TRC = MRI->getRegClass(reg1);
		unsigned pickPred = predMergeInstr->getOperand(1).getReg();
		const unsigned pickOpcode = TII.getPickSwitchOpcode(TRC, true /*pick op*/);
		BuildMI(*mbb, MI, MI->getDebugLoc(), TII.get(pickOpcode), dst).addReg(pickPred).addReg(reg1).addReg(reg2);
	}	else {
#if 0
    MachineInstr* xphi = nullptr;
		//TODO::generated xphi sequence
    for (unsigned i = 0; i < pred2values.size(); i++) {
      std::pair<unsigned, unsigned>* pred2value = pred2values[i];
      if (i == 0) {
        xphi = BuildMI(*mbb, MI, MI->getDebugLoc(), TII.get(LPU::XPHI), dst).addReg(pred2value->first).addReg(pred2value->second);
      } else {
        MachineOperand edgeOp = MachineOperand::CreateReg(pred2value->first, true);
        MachineOperand valueOp = MachineOperand::CreateReg(pred2value->second, true);
        xphi->addOperand(edgeOp);
        xphi->addOperand(valueOp);
      }
    }
#else 
    LowerXPhi(pred2values, MI);
#endif 
	}
	//release memory
	for (unsigned i = 0; i < pred2values.size(); i++) {
		std::pair<unsigned, unsigned>* pred2value = pred2values[i];
		delete pred2value;
	}
	MI->removeFromParent();
}



void LPUCvtCFDFPass::LowerXPhi(SmallVectorImpl<std::pair<unsigned, unsigned> *> &pred2values, MachineInstr* loc) {
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  if (pred2values.empty()) return;
  SmallVector<std::pair<unsigned, unsigned> *, 4> vpair;
  unsigned j = pred2values.size() - 1;
  unsigned i = 0;
  while (i <= j) {
    if (i == j) {
      //singular
      vpair.push_back(pred2values[i]);
    } else {
      std::pair<unsigned, unsigned> *pair1 = pred2values[i];
      std::pair<unsigned, unsigned> *pair2 = pred2values[j];
      //const TargetRegisterClass *pTRC = MRI->getRegClass(pair1->first);
      //MachineInstr* predMerge = getOrInsertPredMerge(loc->getParent(), loc, pair1->first, pair2->first);

      unsigned indexReg = MRI->createVirtualRegister(&LPU::I1RegClass);
      unsigned bbpredReg = MRI->createVirtualRegister(&LPU::I1RegClass);
      BuildMI(*loc->getParent(), loc, DebugLoc(), TII.get(LPU::PREDMERGE),
        bbpredReg).
        addReg(indexReg, RegState::Define).
        addReg(pair1->first).   //last processed edge
        addReg(pair2->first); //current edge

      const TargetRegisterClass *vTRC = MRI->getRegClass(pair1->second);
      const unsigned pickOpcode = TII.getPickSwitchOpcode(vTRC, true /*pick op*/);
      unsigned pickDst;
      if (pred2values.size() == 2) {
        pickDst = loc->getOperand(0).getReg();
      } else {
        pickDst = MRI->createVirtualRegister(vTRC);
      }
      BuildMI(*loc->getParent(), loc, loc->getDebugLoc(),
        TII.get(pickOpcode), pickDst).
        addReg(indexReg).
        addReg(pair1->second).
        addReg(pair2->second);
      pair1->first = bbpredReg;
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


void LPUCvtCFDFPass::generateDynamicPreds() {
	typedef po_iterator<MachineBasicBlock *> po_cfg_iterator;
	MachineBasicBlock *root = thisMF->begin();
	for (po_cfg_iterator itermbb = po_cfg_iterator::begin(root), END = po_cfg_iterator::end(root); itermbb != END; ++itermbb) {
		MachineBasicBlock* mbb = *itermbb;
		//skip loop hdr phi
		if (MLI->getLoopFor(mbb) && MLI->getLoopFor(mbb)->getHeader() == mbb) continue;
		MachineBasicBlock::iterator iterI = mbb->begin();
		bool needDynamicTree = false;
		bool checked = false;
		while (iterI != mbb->end()) {
			MachineInstr *MI = iterI;
			++iterI;
			if (!MI->isPHI()) continue;
			//check to see if needs PREDPROP/PREDMERGE
			if (!checked) {
				for (MIOperands MO(MI); MO.isValid(); ++MO) {
					if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
					if (MO->isUse()) {
						//move to its incoming block operand
						++MO;
						MachineBasicBlock* inBB = MO->getMBB();
						if (!PDT->dominates(mbb, inBB) || !CheckPhiInputBB(inBB, mbb)) {
							needDynamicTree = true;
							break;
						}
					}
				}
			}
			checked = true;
			if (needDynamicTree) {
				generateDynamicPickTreeForPhi(MI);
			}
		}
	} //end of bb
}


void LPUCvtCFDFPass::replaceIfFooterPhiSeq() {
  typedef po_iterator<MachineBasicBlock *> po_cfg_iterator;
  MachineBasicBlock *root = thisMF->begin();
  for (po_cfg_iterator itermbb = po_cfg_iterator::begin(root), END = po_cfg_iterator::end(root); itermbb != END; ++itermbb) {
    MachineBasicBlock* mbb = *itermbb;
    MachineBasicBlock::iterator iterI = mbb->begin();
    while (iterI != mbb->end()) {
      MachineInstr *MI = iterI;
      ++iterI;
      if (!MI->isPHI()) continue;
			generateCompletePickTreeForPhi(MI);
		}
  } //end of bb
}



//make sure phi block post dominates all control points of all its inBBs
bool LPUCvtCFDFPass::CheckPhiInputBB(MachineBasicBlock* inBB, MachineBasicBlock* mbb) {
	if (DT->dominates(inBB, mbb)) {
		return PDT->dominates(mbb, inBB);
	}
	ControlDependenceNode* inNode = CDG->getNode(inBB);
	unsigned numCtrl = 0;
	for (ControlDependenceNode::node_iterator pnode = inNode->parent_begin(), pend = inNode->parent_end(); pnode != pend; ++pnode) {
		ControlDependenceNode* ctrlNode = *pnode;
		MachineBasicBlock* ctrlBB = ctrlNode->getBlock();
		//ignore loop latch ???
		if (MLI->getLoopFor(ctrlBB) && MLI->getLoopFor(ctrlBB)->getLoopLatch() == ctrlBB)
			continue;
		
		++numCtrl;
		if (numCtrl > 1) return false;
		if (!PDT->dominates(mbb, ctrlBB)) {
			return false;
		}
		if (!CheckPhiInputBB(ctrlBB, mbb)) {
			return false;
		}
	}
	return true;
}


void LPUCvtCFDFPass::TraceCtrl(MachineBasicBlock* inBB, MachineBasicBlock* mbb, unsigned Reg, unsigned dst, MachineInstr* MI) {
	MachineBasicBlock* ctrlBB = nullptr;
	if (!DT->dominates(inBB, mbb)) {
		ControlDependenceNode* inNode = CDG->getNode(inBB);
		for (ControlDependenceNode::node_iterator pnode = inNode->parent_begin(), pend = inNode->parent_end(); pnode != pend; ++pnode) {
			ControlDependenceNode* ctrlNode = *pnode;
			ctrlBB = ctrlNode->getBlock();
			if (MLI->getLoopFor(ctrlBB) && MLI->getLoopFor(ctrlBB)->getLoopLatch() == ctrlBB)
				continue;
			unsigned pickReg = 0;
			if (DT->dominates(ctrlBB, mbb)) {
				pickReg = dst;
			}
			MachineInstr* pickInstr = PatchOrInsertPickAtFork(ctrlBB, dst, Reg, inBB, MI, pickReg);
			if (pickInstr) {
				//not patched, keep tracing
				TraceCtrl(ctrlBB, mbb, pickInstr->getOperand(0).getReg(), dst, MI);
			}
		}
	}
}

MachineInstr* LPUCvtCFDFPass::convert_memop_ins(MachineInstr* MI,
                                                unsigned new_opcode,
                                                const LPUInstrInfo& TII,
                                                unsigned issued_reg,
                                                unsigned ready_reg) {
  MachineInstr* new_inst = NULL;
  DEBUG(errs() << "We want convert this instruction.\n");
  for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
    MachineOperand& MO = MI->getOperand(i);
    DEBUG(errs() << "  Operand " << i << ": " << MO << "\n");
  }

  // Alternative implementation would be:
  //  1. Build an "copy" of the existing instruction,
  //  2. Remove the operands from the clonsed instruction.
  //  3. Add new ones, in the right order.
  //
  // This operation doesn't work, because the cloned instruction gets created
  // with too few operands.
  //
  // MachineInstr* new_inst = thisMF->CloneMachineInstr(MI);
  // BB->insert(iterMI, new_inst);
  // new_inst->setDesc(TII.get(new_opcode));
  // int k = MI->getNumOperands() - 1;
  // while (k >= 0) {
  //   new_inst->RemoveOperand(k);
  //   k--;
  // }
  new_inst = BuildMI(*MI->getParent(),
                     MI,
                     MI->getDebugLoc(),
                     TII.get(new_opcode));

  unsigned opidx = 0;
  // Create dummy operands for this instruction.
  MachineOperand issued_op = MachineOperand::CreateReg(issued_reg, true);
  MachineOperand ready_op = MachineOperand::CreateReg(ready_reg, false);


  // Figure out how many "def" operands we have in this instruction.
  // This code assumes that normal loads have exactly one definition,
  // and normal stores have no definitions.
  unsigned expected_def_operands = 0;
  if (TII.isLoad(MI)) {
    expected_def_operands = 1;
  } else if (TII.isStore(MI)) {
    expected_def_operands = 0;
  } else if (TII.isAtomic(MI)) {
    expected_def_operands = 1;
  }
  else {
    assert(false && "Converting unknown type of instruction to ordered memory op");
  }

  // We should have at least as many definitions as expected operands.
  assert(MI->getNumOperands() >= expected_def_operands);

  // 1. Add all the defs to the new instruction first.
  while(opidx < expected_def_operands) {
    MachineOperand& MO = MI->getOperand(opidx);
    // Sanity-check: if we have registers operands, then they had
    // better be definitions.
    if (MO.isReg()) {
      assert(MO.isDef());
    }
    new_inst->addOperand(MO);
    opidx++;
  }

  // 2. Add issued flag.
  new_inst->addOperand(issued_op);
  // Then add the remaining operands.
  while (opidx < MI->getNumOperands()) {
    MachineOperand& MO = MI->getOperand(opidx);
    // In the remaining operands, there should not be any register
    // definitions.
    if (MO.isReg()) {
      assert(!MO.isDef());
    }
    new_inst->addOperand(MO);
    opidx++;
  }
  // 3. Finally, add the ready flag.
  new_inst->addOperand(ready_op);

  // 4. Now copy over remaining state in MI:
  //      Flags
  //      MemRefs.
  //
  // Ideally, we'd be able to just call this function instead,
  // but with a different opcode that reserves more space for
  // operands.
  //   MachineInstr(MachineFunction &, const MachineInstr &);
  new_inst->setFlags(MI->getFlags());
  new_inst->setMemRefs(MI->memoperands_begin(),
                       MI->memoperands_end());

  DEBUG(errs() << "   Convert to ins: " << *new_inst << "\n");

  for (unsigned i = 0; i < new_inst->getNumOperands(); ++i) {
    MachineOperand& MO = new_inst->getOperand(i);
    DEBUG(errs() << "  Operand " << i << ": " << MO << "\n");
  }

  DEBUG(errs() << "   Original ins modified: " << *MI << "\n");

  return new_inst;
}


// Insert all the definitions of mem_in for each block,
// either as:
//   1. PHI from our predecessors, if multiple predecessors
//   2. Direct initialization, if 1 predecessor
//   3. mov of a constant, if 0 predecessors.
//
void LPUCvtCFDFPass::createMemInRegisterDefs(DenseMap<MachineBasicBlock*, unsigned>& blockToMemIn,
                                             DenseMap<MachineBasicBlock*, unsigned>& blockToMemOut) {
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());

  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {

    MachineBasicBlock* BBptr = &(*BB);

    assert(blockToMemIn.find(BBptr) != blockToMemIn.end());
    unsigned mem_in_reg = blockToMemIn[BBptr];

    if (BB->pred_size() > 1) {
      // Case 1: Insert a PHI of the mem_out registers from all the
      // predecessors.
      MachineInstrBuilder mbuilder = BuildMI(*BB,
                                             BB->getFirstNonPHI(),
                                             DebugLoc(),
                                             TII.get(TargetOpcode::PHI),
                                             mem_in_reg);

      // Scan the predecessors, and add the PHI value for each.
      for (MachineBasicBlock::pred_iterator PI = BB->pred_begin();
           PI != BB->pred_end();
           ++PI) {
        assert(blockToMemIn.find(*PI) != blockToMemIn.end());
        unsigned target_out_reg = blockToMemOut[*PI];
        mbuilder.addReg(target_out_reg);
        mbuilder.addMBB(*PI);
      }
    }
    else if (BB->pred_size() == 1) {
      // Case 2: Only one predecessor.  Just use the mem_out register
      // from the predecessor directly.
      MachineBasicBlock::pred_iterator PI = BB->pred_begin();
      MachineBasicBlock* PIptr = *PI;
      assert(blockToMemIn.find(PIptr) != blockToMemIn.end());
      unsigned target_out_reg = blockToMemOut[PIptr];

      // Add in the mov of the register from the previous block.
      BuildMI(*BB,
              BB->getFirstNonPHI(),
              DebugLoc(),
              TII.get(LPU::MOV1),
              mem_in_reg).addReg(target_out_reg);
    }
    else {
      assert(BB->pred_size() == 0);
      // Case 3: No predecessors.  Generate a simple mov of a
      // constant, to handle the initialization.

      // Add in the mov of the register from the previous block.
      BuildMI(*BB,
              BB->getFirstNonPHI(),
              DebugLoc(),
              TII.get(LPU::MOV1),
              mem_in_reg).addImm(1);
    }

    DEBUG(errs() << "After createMemInRegisterDefs: " << *BB << "\n");
  }
}



unsigned LPUCvtCFDFPass::convert_block_memops_linear(MachineFunction::iterator& BB,
                                                     unsigned mem_in_reg)

{
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();

  unsigned current_mem_reg = mem_in_reg;

  MachineBasicBlock::iterator iterMI = BB->begin();
  while (iterMI != BB->end()) {
    MachineInstr* MI = iterMI;
    DEBUG(errs() << "Found instruction: " << *MI << "\n");

    unsigned current_opcode = MI->getOpcode();
    unsigned converted_opcode = TII.get_ordered_opcode_for_LDST(current_opcode);

    if (current_opcode != converted_opcode) {
      // TBD(jsukha): For now, we are just going to create a linear
      // chain of dependencies for memory instructions within a
      // basic block.
      //
      // We will want to optimize this implementation further, but
      // this is the simple version for now.
      unsigned next_mem_reg = MRI->createVirtualRegister(MemopRC);

      convert_memop_ins(MI,
                        converted_opcode,
                        TII,
                        next_mem_reg,
                        current_mem_reg);

      // Erase the old instruction.
      iterMI = BB->erase(iterMI);

      // Advance the chain.
      current_mem_reg = next_mem_reg;
    }
    else {
      ++iterMI;
    }
  }

  return current_mem_reg;
}


unsigned LPUCvtCFDFPass::merge_dependency_signals(MachineFunction::iterator& BB,
                                                  MachineInstr* MI,
                                                  SmallVector<unsigned, MEMDEP_VEC_WIDTH>* current_wavefront,
                                                  unsigned input_mem_reg) {

  if (current_wavefront->size() > 0) {
    const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
    MachineRegisterInfo *MRI = &thisMF->getRegInfo();

    DEBUG(errs() << "Merging dependency signals from " << current_wavefront->size() << " register " << "\n");

    // BFS-like algorithm for merging the registers together.
    // Merge consecutive pairs of dependency signals together,
    // and push the output into "next_level".
    SmallVector<unsigned, MEMDEP_VEC_WIDTH> tmp_buffer;
    SmallVector<unsigned, MEMDEP_VEC_WIDTH>* current_level;
    SmallVector<unsigned, MEMDEP_VEC_WIDTH>* next_level;

    current_level = current_wavefront;
    next_level = &tmp_buffer;

    while (current_level->size() > 1) {
      assert(next_level->size() == 0);
      for (unsigned i = 0; i < current_level->size(); i+=2) {
        // Merge current_level[i] and current_level[i+1] into
        // next_level[i/2]
        if ((i+1) < current_level->size()) {

          // Even case: we have a pair to merge.  Create a virtual
          // register + instruction to do the merge.
          unsigned next_out_reg = MRI->createVirtualRegister(MemopRC);
          MachineInstr* new_inst;
          if (MI) {
            new_inst = BuildMI(*MI->getParent(),
                               MI,
                               MI->getDebugLoc(),
                               TII.get(LPU::MERGE1),
                               next_out_reg).addImm(0).addReg((*current_level)[i]).addReg((*current_level)[i+1]);
          }
          else {
            // Adding a merge at the end of the block.
            new_inst = BuildMI(*BB,
                               BB->getFirstTerminator(),
                               DebugLoc(),
                               TII.get(LPU::MERGE1),
                               next_out_reg).addImm(0).addReg((*current_level)[i]).addReg((*current_level)[i+1]);
          }
          DEBUG(errs() << "Inserted dependecy merge instruction " << *new_inst << "\n");
          next_level->push_back(next_out_reg);
        }
        else {
          // In an odd case, just pass register through to next level.
          next_level->push_back((*current_level)[i]);
        }
      }

      // Swap next and current.
      SmallVector<unsigned, MEMDEP_VEC_WIDTH>* tmp = current_level;
      current_level = next_level;
      next_level = tmp;
      next_level->clear();

      DEBUG(errs() << "Current level size is now " << current_level->size() << "\n");
      DEBUG(errs() << "Next level size is now " << next_level->size() << "\n");
    }

    assert(current_level->size() == 1);
    unsigned ans = (*current_level)[0];

    // Clear both vectors, just to be certain.
    current_level->clear();
    next_level->clear();

    return ans;
  }
  else {
    return input_mem_reg;
  }
}



unsigned LPUCvtCFDFPass::convert_block_memops_wavefront(MachineFunction::iterator& BB,
                                                        unsigned mem_in_reg)
{
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();

  unsigned current_mem_reg = mem_in_reg;
  SmallVector<unsigned, MEMDEP_VEC_WIDTH> current_wavefront;
  current_wavefront.clear();
  DEBUG(errs() << "Wavefront memory ordering for block " << BB << "\n");

  MachineBasicBlock::iterator iterMI = BB->begin();
  while (iterMI != BB->end()) {
    MachineInstr* MI = iterMI;
    DEBUG(errs() << "Found instruction: " << *MI << "\n");

    unsigned current_opcode = MI->getOpcode();
    unsigned converted_opcode = TII.get_ordered_opcode_for_LDST(current_opcode);

    bool is_store = TII.isStore(MI);

    if (current_opcode != converted_opcode) {
      // Create a register for the "issued" output of this memory
      // operation.
      unsigned next_out_reg = MRI->createVirtualRegister(MemopRC);

      if (is_store) {
        // If there were any loads in the last interval, merge all
        // their outputs into one output, and change the latest
        // source.
        if (current_wavefront.size() > 0) {
          current_mem_reg = merge_dependency_signals(BB,
                                                     MI,
                                                     &current_wavefront,
                                                     current_mem_reg);
          assert(current_wavefront.size() == 0);
        }
      }
      else {
        // Just a load. Build up the set of load outputs that we
        // depend on.
        assert(TII.isLoad(MI));
        current_wavefront.push_back(next_out_reg);
      }

      convert_memop_ins(MI,
                        converted_opcode,
                        TII,
                        next_out_reg,
                        current_mem_reg);

      if (is_store) {
        current_mem_reg = next_out_reg;
      }

      // Erase the old instruction.
      iterMI = BB->erase(iterMI);
    }
    else {
      ++iterMI;
    }
  }

  // Sink any loads at the end of the block to the end of the block.
  current_mem_reg = merge_dependency_signals(BB,
                                             NULL,
                                             &current_wavefront,
                                             current_mem_reg);

  return current_mem_reg;
}



void LPUCvtCFDFPass::addMemoryOrderingConstraints() {

  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();

  DenseMap<MachineBasicBlock*, unsigned> blockToMemIn;
  DenseMap<MachineBasicBlock*, unsigned> blockToMemOut;


  DEBUG(errs() << "Before addMemoryOrderingConstraints");
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {

    // Create a virtual register for the block input.
    unsigned mem_in_reg = MRI->createVirtualRegister(MemopRC);
    unsigned last_mem_reg;

    // Link all the memory ops in BB together.
    // Return the name of the last output register (which could be
    // mem_in_reg).

    if (OrderMemops == 2) {
      last_mem_reg = convert_block_memops_wavefront(BB,
                                                    mem_in_reg);
    }
    else if (OrderMemops == 1) {
      last_mem_reg = convert_block_memops_linear(BB,
                                                 mem_in_reg);

    }
    else {
      // ERROR: unsupported memory ordering.
      assert(((OrderMemops ==1) || (OrderMemops == 2))
             && "Only linear and wavefront memory ordering implemented now.");
    }

    // Create a last (virtual) register for the output of the block.
    unsigned mem_out_reg = MRI->createVirtualRegister(MemopRC);

    // This operation creates an instruction before the terminating
    // instruction in the block that moves the contents of the last
    // "issued" flag in the block into the mem_out register.
    //
    // TBD(jsukha): For now, I'm just going to do this operation
    // with a mov1.  I don't know if some other instruction will be
    // better.
    MachineInstr* mem_out_def = BuildMI(*BB,
                                        BB->getFirstTerminator(),
                                        DebugLoc(),
                                        TII.get(LPU::MOV0),
                                        mem_out_reg).addReg(last_mem_reg);

    DEBUG(errs() << "Inserted mem_out_def instruction " << *mem_out_def << "\n");

    // Save mem_in_reg and mem_out_reg for each block into a DenseMap,
    // so that we can create a PHI instruction as an input to the
    // block.
    blockToMemIn[BB] = mem_in_reg;
    blockToMemOut[BB] = mem_out_reg;

    DEBUG(errs() << "After memop conversion of function: " << *BB << "\n");
  }

  // Another walk over basic blocks: add in definitions for mem_in
  // register for each block, based on predecessors.
  createMemInRegisterDefs(blockToMemIn, blockToMemOut);
}

