//===-- CSACvtCFDFPass.cpp - CSA convert control flow to data flow --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file "reexpresses" the code containing traditional control flow
// into a basically data flow representation suitable for the CSA.
//
//===----------------------------------------------------------------------===//
#include <stack>
#include "CSA.h"
#include "InstPrinter/CSAInstPrinter.h"
#include "CSATargetMachine.h"
#include "CSALicAllocation.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SparseSet.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
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
#include "CSAInstrInfo.h"

using namespace llvm;

static cl::opt<int>
CvtCFDFPass("csa-cvt-cf-df-pass", cl::Hidden,
               cl::desc("CSA Specific: Convert control flow to data flow pass"),
               cl::init(1));

static cl::opt<int>
RunSXU("csa-run-sxu", cl::Hidden,
  cl::desc("CSA Specific: run on sequential unit"),
  cl::init(0));

static cl::opt<int>
UseDynamicPred("csa-dynamic-pred", cl::Hidden,
  cl::desc("CSA Specific: run on sequential unit"),
  cl::init(0));


// Flag for controlling code that deals with memory ordering.
enum OrderMemopsMode {
  // No extra code added at all for ordering.  Often incorrect.  
  none = 0,

  // Linear ordering of all memops.  Dumb but should be correct.  
  linear = 1,
  
  //  Stores inside a basic block are totally ordered.
  //  Loads ordered between the stores, but
  //  unordered with respect to each other.
  //  No reordering across basic blocks.
  wavefront = 2,
};

static cl::opt<OrderMemopsMode>
OrderMemopsType("csa-order-memops-type",
                cl::Hidden,
                cl::desc("CSA Specific: Order memory operations"),
                cl::values(clEnumVal(none,
                                     "No memory ordering. Possibly incorrect"),
                           clEnumVal(linear,
                                     "Linear ordering. Dumb but correct"),
                           clEnumVal(wavefront,
                                     "Totally ordered stores, parallel loads between stores.")),
                cl::init(OrderMemopsMode::wavefront));

//  Boolean flag.  If it is set to 0, we force "none" for memory
//  ordering.  Otherwise, we just obey the OrderMemopsType variable.
static cl::opt<int>
OrderMemops("csa-order-memops",
            cl::Hidden,
            cl::desc("CSA Specific: Disable ordering of memory operations (by setting to 0)"),
            cl::init(1));

// The register class we are going to use for all the memory-op
// dependencies.  Technically they could be I0, but I don't know how
// happy LLVM will be with that.
const TargetRegisterClass* MemopRC = &CSA::I1RegClass;


// Width of vectors we are using for memory op calculations.
// TBD(jsukha): As far as I know, this value only affects performance,
// not correctness?
#define MEMDEP_VEC_WIDTH 8

#define DEBUG_TYPE "csa-cvt-cf-df-pass"

namespace llvm {
  class CSACvtCFDFPass : public MachineFunctionPass {
  public:
    struct CmpFcn {
      CmpFcn(const DenseMap<MachineBasicBlock*, unsigned>& m) : mbb2rpo(m) {};
      DenseMap<MachineBasicBlock*, unsigned> mbb2rpo;
      bool operator() (MachineBasicBlock* A, MachineBasicBlock* B) {
        return mbb2rpo[A] < mbb2rpo[B];
      }
    };
    static char ID;
    CSACvtCFDFPass();

    StringRef getPassName() const override {
      return "CSA Convert Control Flow to Data Flow";
    }

    bool runOnMachineFunction(MachineFunction &MF) override;
    ControlDependenceNode* getNonLatchParent(ControlDependenceNode* anode, bool &oneAndOnly);
    MachineInstr* insertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgpBB);
    MachineInstr* getOrInsertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgBB);
    MachineInstr* insertPICKForReg(MachineBasicBlock* ctrlBB, unsigned Reg, MachineBasicBlock* inBB, MachineInstr* phi, unsigned pickReg = 0);
    void assignPICKSrcForReg(unsigned &pickFalseReg, unsigned &pickTrueReg, unsigned Reg, MachineBasicBlock* ctrlBB, MachineBasicBlock* inBB, MachineInstr* phi);
    //generate a PICK for SSA value dst at fork of ctrlBB with source input Reg from inBB, and output in pickReg
    MachineInstr* PatchOrInsertPickAtFork(MachineBasicBlock* ctrlBB, unsigned dst, unsigned Reg, MachineBasicBlock* inBB, MachineInstr* phi, unsigned pickReg = 0);
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
    void renameOnLoopEntry();
    void renameAcrossLoopForRepeat(MachineLoop *);
    void insertSWITCHForRepeat();
    void insertSWITCHForRepeat(MachineLoop* mloop);
    unsigned repeatOperandInLoop(unsigned Reg, MachineLoop* mloop);
    MachineBasicBlock* getDominatingExitingBB(SmallVectorImpl<MachineBasicBlock*> &exitingBlks, MachineInstr* UseMI, unsigned Reg);
    void insertSWITCHForLoopExit();
    void insertSWITCHForLoopExit(MachineLoop* L, DenseMap<MachineBasicBlock *, std::set<unsigned> *> &LCSwitch);
    unsigned SwitchOutExitingBlk(MachineBasicBlock* exitingBlk, unsigned Reg, MachineLoop *mloop);
    void SwitchDefAcrossExits(unsigned Reg, MachineBasicBlock* mbb, MachineLoop* mloop, MachineOperand &UseMO);
    void SwitchDefAcrossLoops(unsigned Reg, MachineBasicBlock* mbb, MachineLoop* mloop);
    void replacePhiWithPICK();
    void replaceLoopHdrPhi();
    void replaceCanonicalLoopHdrPhi(MachineBasicBlock* lhdr);
    bool hasStraightExitings(MachineLoop* mloop);
    void replaceStraightExitingsLoopHdrPhi(MachineBasicBlock* mbb);
    void generateCompletePickTreeForPhi(MachineBasicBlock *);
    void generateDynamicPickTreeForFooter(MachineBasicBlock *);
    void generateDynamicPickTreeForHeader(MachineBasicBlock*);
    bool needDynamicPreds();
    bool needDynamicPreds(MachineLoop* L);
    void generateDynamicPreds();
    void generateDynamicPreds(MachineLoop* L);
    void replacePhiForUnstructed();
    void replacePhiForUnstructedLoop(MachineLoop* L);
    unsigned getEdgePred(MachineBasicBlock* fromBB, ControlDependenceNode::EdgeType childType);
    void setEdgePred(MachineBasicBlock* fromBB, ControlDependenceNode::EdgeType childType, unsigned ch);
    unsigned getBBPred(MachineBasicBlock* inBB);
    void setBBPred(MachineBasicBlock* inBB, unsigned ch);
    MachineInstr* getOrInsertPredMerge(MachineBasicBlock* mbb, MachineInstr* loc, unsigned e1, unsigned e2);
    MachineInstr* InsertPredProp(MachineBasicBlock* mbb, unsigned bbPred=0);
    unsigned computeEdgePred(MachineBasicBlock* fromBB, MachineBasicBlock* toBB, std::list<MachineBasicBlock*> &path);
    unsigned mergeIncomingEdgePreds(MachineBasicBlock* inBB, std::list<MachineBasicBlock*> &path);
    unsigned computeBBPred(MachineBasicBlock *inBB, std::list<MachineBasicBlock*> &path);
    void TraceCtrl(MachineBasicBlock* inBB, MachineBasicBlock* mbb, unsigned Reg, unsigned dst, MachineInstr* MI);
    void CombineDuplicatePhiInputs(SmallVectorImpl<std::pair<unsigned, unsigned> *> &pred2values, MachineInstr* iPhi);
    void LowerXPhi(SmallVectorImpl<std::pair<unsigned, unsigned> *> &pred2values, MachineInstr *MI);
    bool CheckPhiInputBB(MachineBasicBlock* inBB, MachineBasicBlock* mbb);
    void replaceIfFooterPhiSeq();
    void assignLicForDF();
    void createFIEntryDefs();
    void removeBranch();
    void linearizeCFG();
    unsigned findSwitchingDstForReg(unsigned Reg, MachineBasicBlock* mbb);
    void handleAllConstantInputs();
    void releaseMemory() override;
    bool replaceUndefWithIgn();
    bool isUnStructured(MachineBasicBlock* mbb);

    // TBD(jsukha): Experimental code for ordering of memory ops.
    void addMemoryOrderingConstraints();

    // Helper methods:

    // Create a new OLD/OST instruction, to replace an existing LD /
    // ST instruction.
    //  issued_reg is the register to define as the extra output
    //  ready_reg is the register which is the extra input
    MachineInstr* convert_memop_ins(MachineInstr* memop,
                                    unsigned new_opcode,
                                    const CSAInstrInfo& TII,
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
    const CSAInstrInfo* TII;
    MachineRegisterInfo* MRI;
    const TargetRegisterInfo* TRI;
    CSAMachineFunctionInfo *LMFI;
    MachineDominatorTree *DT;
    MachinePostDominatorTree *PDT;
    ControlDependenceGraph *CDG;
    MachineLoopInfo *MLI;
    DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *> bb2switch;  //switch for Reg added in bb
    DenseMap<MachineBasicBlock *, unsigned> bb2predcpy;
    DenseMap<MachineBasicBlock *, DenseMap<unsigned, MachineInstr *> *> bb2pick;  //switch for Reg added in bb
    DenseMap<MachineBasicBlock*, SmallVectorImpl<unsigned>* > edgepreds;
    DenseMap<MachineBasicBlock *, unsigned> bbpreds;
    DenseMap<MachineBasicBlock*, MachineInstr*> bb2predmerge;
    DenseMap<MachineBasicBlock*, unsigned> bb2rpo;
    std::set<MachineInstr *> multiInputsPick;
    std::set<MachineBasicBlock *> dcgBBs;
  };
}

//  Because of the namespace-related syntax limitations of gcc, we need
//  To hoist init out of namespace blocks.
char CSACvtCFDFPass::ID = 0;
//declare CSACvtCFDFPass Pass
INITIALIZE_PASS(CSACvtCFDFPass, "csa-cvt-cfdf", "CSA Convert Control Flow to Data Flow", true, true)

CSACvtCFDFPass::CSACvtCFDFPass() : MachineFunctionPass(ID) {
  initializeCSACvtCFDFPassPass(*PassRegistry::getPassRegistry());
}


MachineFunctionPass *llvm::createCSACvtCFDFPass() {
  return new CSACvtCFDFPass();
}

void CSACvtCFDFPass::releaseMemory() {
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

  DenseMap<MachineBasicBlock *, SmallVectorImpl<unsigned> *> ::iterator itedge = edgepreds.begin();
  while (itedge != edgepreds.end()) {
    SmallVectorImpl<unsigned>* edges = itedge->getSecond();
    ++itedge;
    delete edges;
  }
  edgepreds.clear();
  bb2predcpy.clear();
  bb2predmerge.clear();
  bbpreds.clear();
  bb2switch.clear();
  bb2pick.clear();
  bb2rpo.clear();
  dcgBBs.clear();
  multiInputsPick.clear();
}


void CSACvtCFDFPass::replacePhiWithPICK() {
#if 0
  replacePhiForUnstructed();
  {
    errs() << "after replace phi for unstructed" << ":\n";
    thisMF->print(errs(), getAnalysisIfAvailable<SlotIndexes>());
  }
#else
  replaceLoopHdrPhi();
  replaceIfFooterPhiSeq();
#endif
}

//return the first non latch parent found or NULL
ControlDependenceNode* CSACvtCFDFPass::getNonLatchParent(ControlDependenceNode* anode, bool &oneAndOnly ) {
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


bool CSACvtCFDFPass::runOnMachineFunction(MachineFunction &MF) {

  if (CvtCFDFPass == 0) return false;
  thisMF = &MF;

  TII = static_cast<const CSAInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  MRI = &thisMF->getRegInfo();
  TRI = thisMF->getSubtarget().getRegisterInfo();
  LMFI = thisMF->getInfo<CSAMachineFunctionInfo>();

  DT = &getAnalysis<MachineDominatorTree>();
  PDT = &getAnalysis<MachinePostDominatorTree>();
  if (PDT->getRootNode() == nullptr) return false;
  CDG = &getAnalysis<ControlDependenceGraph>();
  MLI = &getAnalysis<MachineLoopInfo>();

#if 1
  //exception handling code creates multiple exits from a function
  SmallVector<MachineBasicBlock*, 4> exitBlks;
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    if (BB->succ_empty()) exitBlks.push_back(&*BB);
  }
  if (exitBlks.size() > 1) return false;
#endif

  // Give up and run on SXU if there is dynamic stack activity we don't handle.
  if (thisMF->getFrameInfo().hasVarSizedObjects()) {
    errs() << "WARNING: dataflow conversion not attempting to handle dynamic stack allocation.\n";
    errs() << "Function \"" << thisMF->getName() << "\" will run on the SXU.\n";
    return false;
  }

  bool Modified = false;

#if 0
  // for now only well formed innermost loop regions are processed in this pass
  MachineLoopInfo *MLI = &getAnalysis<MachineLoopInfo>();
  if (!MLI) {
    DEBUG(errs() << "no loop info.\n");
    return false;
  }
#endif

  // Move reads of index references, which turn into invariant uses of
  // implicitly live-in registers, into the function entry. This is done so
  // that dataflow conversion recognizes these non-loop value defs as ones
  // which need to be flowed in.
  createFIEntryDefs();

  replaceUndefWithIgn();

  // TBD(jsukha): Experimental code to add dependencies for memory
  // operations.
  //
  // This step should run before the main dataflow conversion because
  // it introduces extra dependencies through virtual registers than
  // the dataflow conversion must also deal with.

  if (OrderMemops && (OrderMemopsType > OrderMemopsMode::none)) {
    addMemoryOrderingConstraints();
  }
#if 0
  {
    errs() << "CSACvtCFDFPass after memoryop order" << ":\n";
    MF.print(errs(), getAnalysisIfAvailable<SlotIndexes>());
  }
#endif
  typedef po_iterator<MachineBasicBlock *> po_cfg_iterator;
  MachineBasicBlock *root = &*thisMF->begin();
  std::stack<MachineBasicBlock*> postk;
  for (po_cfg_iterator itermbb = po_cfg_iterator::begin(root), END = po_cfg_iterator::end(root); itermbb != END; ++itermbb) {
    MachineBasicBlock* mbb = *itermbb;
#if 0
    //remove DBG_VALUE instrutions
    MachineBasicBlock::iterator iInstr = mbb->begin();
    while (iInstr != mbb->end()) {
      MachineInstr* mInstr = &*iInstr;
      iInstr++;
      if (mInstr->isDebugValue()) {
        mInstr->removeFromParent();
      }
    }
#endif 
    postk.push(mbb);
  }
  unsigned i = 0;
  while (!postk.empty()) {
    MachineBasicBlock *mbb = postk.top();
    postk.pop();
    bb2rpo[mbb] = i;
    i++;
  }

  //renaming using switch to seal all down rang of each definition within loop
  renameOnLoopEntry();
  insertSWITCHForLoopExit();
  //rename, adding lhdr phi to seal all up range of each defintions up till loop hdr
  insertSWITCHForRepeat();
  //if loop hdr is also an exiting blk, repeatOperand generated loop hdr phi need to go through SWITCHforIf process
  insertSWITCHForIf();


if (needDynamicPreds() || UseDynamicPred)
  generateDynamicPreds();
else
  replacePhiWithPICK();

  handleAllConstantInputs();
#if 0
  {
    errs() << "CSACvtCFDFPass before LIC allocation" << ":\n";
    MF.print(errs(), getAnalysisIfAvailable<SlotIndexes>());
  }
#endif
  assignLicForDF();
#if 0
  {
    errs() << "CSACvtCFDFPass after LIC allocation" << ":\n";
    MF.print(errs(), getAnalysisIfAvailable<SlotIndexes>());
  }
#endif
  if (!RunSXU) {
    removeBranch();
    linearizeCFG();
  }
  //destroy all the data structures for current function,
  //after branches are removed, BB pointer are no longer valid
  releaseMemory();

  return Modified;

}

MachineInstr* CSACvtCFDFPass::insertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgpBB) {
  // generate and insert SWITCH or copy
  const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
  MachineInstr* result = nullptr;
  if (cdgpBB->succ_size() > 1) {
    MachineBasicBlock::iterator loc = cdgpBB->getFirstTerminator();
    MachineInstr* bi = &*loc;
    unsigned switchFalseReg = MRI->createVirtualRegister(TRC);
    unsigned switchTrueReg = MRI->createVirtualRegister(TRC);
    assert(bi->getOperand(0).isReg());
    // generate switch op
    const unsigned switchOpcode = TII->getPickSwitchOpcode(TRC, false /*not pick op*/);
    MachineInstr *switchInst;
    switchInst = BuildMI(*cdgpBB, loc, DebugLoc(), TII->get(switchOpcode),
      switchFalseReg).
      addReg(switchTrueReg, RegState::Define).
      addReg(bi->getOperand(0).getReg()).
      addReg(Reg);
    
    switchInst->setFlag(MachineInstr::NonSequential);
    result = switchInst;
  } else {
    MachineBasicBlock::iterator loc = cdgpBB->getLastNonDebugInstr();
    assert(MLI->getLoopFor(cdgpBB)->getLoopLatch() == cdgpBB || MLI->getLoopFor(cdgpBB)->getLoopLatch() == nullptr);
    //LLVM 3.6 buggy latch with no exit edge
    //get a wierd latch with no exit edge from LLVM 3.6 buggy loop rotation
    const unsigned moveOpcode = TII->getMoveOpcode(TRC);
    unsigned cpyReg = MRI->createVirtualRegister(TRC);
    MachineInstr *cpyInst = BuildMI(*cdgpBB, loc, DebugLoc(), TII->get(moveOpcode), cpyReg).addReg(Reg);
    cpyInst->setFlag(MachineInstr::NonSequential);
    result = cpyInst;
  }
  return result;
}


unsigned CSACvtCFDFPass::findSwitchingDstForReg(unsigned Reg, MachineBasicBlock* mbb) {
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
  if (MRI->use_empty(switchFalseReg)) {
    return switchFalseReg;
  }
  else if (MRI->use_empty(switchTrueReg)) {
    return switchTrueReg;
  }
  return 0;
}



MachineInstr* CSACvtCFDFPass::getOrInsertSWITCHForReg(unsigned Reg, MachineBasicBlock *cdgpBB) {
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


//TODO: rename for repeat
void CSACvtCFDFPass::renameAcrossLoopForRepeat(MachineLoop* L) {
  for (MachineLoop::iterator LI = L->begin(), LE = L->end(); LI != LE; ++LI) {
    renameAcrossLoopForRepeat(*LI);
  }
  MachineLoop *mloop = L;
  for (MachineLoop::block_iterator BI = mloop->block_begin(), BE = mloop->block_end(); BI != BE; ++BI) {
    MachineBasicBlock* mbb = *BI;
    //only conside blocks in the  urrent loop level, blocks in the nested level are done before.
    if (MLI->getLoopFor(mbb) != mloop) continue;
    for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
      MachineInstr *MI = &*I;
      //if (MI->isPHI()) continue;
      for (MIOperands MO(*MI); MO.isValid(); ++MO) {
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
            const unsigned moveOpcode = TII->getMoveOpcode(TRC);
            unsigned cpyReg = MRI->createVirtualRegister(TRC);
            MachineInstr *cpyInst = BuildMI(*landingPad, landingPad->getFirstTerminator(), DebugLoc(), TII->get(moveOpcode), cpyReg).addReg(Reg);
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


void CSACvtCFDFPass::insertSWITCHForOperand(MachineOperand& MO, MachineBasicBlock* mbb, MachineInstr* phiIn) {
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
        if (TII->isSwitch(DefMI) && unode->isParent(dnode)) {
          //def already from a switch -- can only happen if use is an immediate child of def in CDG
          //assert(dnode->isChild(unode) || MI->isPHI());
          return;
        }

        SmallVector<MachineInstr*, 8> NewPHIs;
        MachineSSAUpdater SSAUpdate(*thisMF, &NewPHIs);
        const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
        unsigned pickVReg = MRI->createVirtualRegister(TRC);
        SSAUpdate.Initialize(pickVReg);
        SSAUpdate.AddAvailableValue(dmbb, Reg);
        unsigned newVReg;
        for (ControlDependenceNode::node_iterator uparent = unode->parent_begin(), uparent_end = unode->parent_end();
          uparent != uparent_end; ++uparent) {
          ControlDependenceNode *upnode = *uparent;
          MachineBasicBlock *upbb = upnode->getBlock();
          if (!upbb) {
            //this is typical define inside loop, used outside loop on the main execution path
            continue;
          }
          if (bb2rpo[upbb] >= bb2rpo[mbb]) {
            if (!phiIn || 
                !MLI->getLoopFor(phiIn->getParent()) ||
                MLI->getLoopFor(phiIn->getParent())->getHeader() != phiIn->getParent())
              //don't look back if not a loop hdr phi for mbb's loop
              continue;
          }
          if (DT->dominates(dmbb, upbb)) { //including dmbb itself
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
            } else {
              //rename it to switchTrueReg
              newVReg = switchTrueReg;
            }
            SSAUpdate.AddAvailableValue(upbb, newVReg);
          }
        } //end of for (parent

        if (phiIn) {
          SSAUpdate.RewriteUse(MO);
        } else {
          MachineRegisterInfo::use_iterator UI = MRI->use_begin(Reg);
          while (UI != MRI->use_end()) {
            MachineOperand &UseMO = *UI;
            MachineInstr *UseMI = UseMO.getParent();
            ++UI;
            if (UseMI->getParent() == mbb) {
              SSAUpdate.RewriteUse(UseMO);
            }
          }
        }
      }
    }
  }
}


//focus on uses
void CSACvtCFDFPass::insertSWITCHForIf() {
  typedef po_iterator<ControlDependenceNode *> po_cdg_iterator;
  ControlDependenceNode *root = CDG->getRoot();
  for (po_cdg_iterator DTN = po_cdg_iterator::begin(root), END = po_cdg_iterator::end(root); DTN != END; ++DTN) {
    MachineBasicBlock *mbb = DTN->getBlock();
    if (!mbb) continue; //root node has no bb
    // process each instruction in BB
    for (MachineBasicBlock::succ_iterator isucc = mbb->succ_begin(); isucc != mbb->succ_end(); ++isucc) {
      MachineBasicBlock* succBB = *isucc;
      //phi in succNode has been processed or generated before
      //if (!succNode->isParent(*DTN)), don't need this as long as we don't handle phi in its owning blks.
      //for loop hdr Phi, we still need to handle back to back instructions in same block:
      // %y = Phi(%x0, %x)
      // %x = ...
      for (MachineBasicBlock::iterator iPhi = succBB->begin(); iPhi != succBB->end(); ++iPhi) {
        if (!iPhi->isPHI()) {
          break;
        }
        for (MIOperands MO(*iPhi); MO.isValid(); ++MO) {
          if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
          unsigned Reg = MO->getReg();
          // process uses
          if (MO->isUse()) {
            MachineOperand& mOpnd = *MO;
            ++MO;
            if (MO->getMBB() == mbb) {
              //diamond if-branch input, closed loop latch input for loop hrd phi, or def across loop from outside loop
              //no switch at loop latch with exiting, which has been handled in loop exits processing
              if (mbb->succ_size() == 1 || 
                  (MLI->getLoopFor(mbb) && 
                   MLI->getLoopFor(mbb)->isLoopLatch(mbb) &&
                   MLI->getLoopFor(mbb)->getHeader() != succBB)) { //not loop hdr phi
                //possible multiple CDG parents
                insertSWITCHForOperand(mOpnd, mbb, &*iPhi);
              } else {
                //TODO: handle infinite loop
                //mbb itself is a fork, this includes non-latch exiting blk
                //1) triangle if's fall through branch
                //2) loop hdr phi
                MachineInstr *DefMI = MRI->getVRegDef(Reg);
#if 1
                if (TII->isSwitch(DefMI) && DefMI->getParent() == mbb) {
                  //alread switched reg from SwitchForRepeat 
                  continue;
                }
#endif
                MachineInstr *defSwitchInstr = getOrInsertSWITCHForReg(Reg, mbb);
                unsigned switchFalseReg = defSwitchInstr->getOperand(0).getReg();
                unsigned switchTrueReg = defSwitchInstr->getOperand(1).getReg();
                unsigned newVReg;
                if (CDG->getEdgeType(mbb, succBB, true) == ControlDependenceNode::TRUE) {
                  newVReg = switchTrueReg;
                } else {
                  assert(CDG->getEdgeType(mbb, succBB, true) == ControlDependenceNode::FALSE);
                  newVReg = switchFalseReg;
                }
                mOpnd.setReg(newVReg);
              }
            }
          }
        }
      }
      for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
        MachineInstr *MI = &*I;
        //to be consistent, never handle phi in its owning block, 
        //always rename it in its input predecessor block
        if (MI->isPHI()) 
          continue; 
        if (MI->getOpcode() == CSA::PREDPROP || MI->getOpcode() == CSA::PREDMERGE) 
          continue;
        for (MIOperands MO(*MI); MO.isValid(); ++MO) {
          insertSWITCHForOperand(*MO, mbb);
        }
      }//end of for MI
    }
  }//end of for DTN(mbb)
}



MachineBasicBlock* CSACvtCFDFPass::getDominatingExitingBB(SmallVectorImpl<MachineBasicBlock*> &exitingBlks, MachineInstr* UseMI, unsigned Reg) {
  MachineBasicBlock* anchorBB = nullptr;
  MachineBasicBlock* exitingBlk = nullptr;
  MachineBasicBlock* UseBB = UseMI->getParent();
  if (UseMI->isPHI()) {
    for (MIOperands MO(*UseMI); MO.isValid(); ++MO) {
      if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
      if (MO->isUse()) {
        unsigned MOReg = MO->getReg();
        //move to its incoming block operand
        ++MO;
        MachineBasicBlock* inBB = MO->getMBB();
        if (MOReg == Reg) {
          anchorBB = inBB;
          break;
        }
      }
    }
  } else {
    anchorBB = UseBB;
  }
  assert(anchorBB);
  std::sort(exitingBlks.begin(), exitingBlks.end(), CmpFcn(bb2rpo));
  for (int i = exitingBlks.size() - 1; i >= 0; i--) {
    if (DT->dominates(exitingBlks[i], anchorBB)) {
      exitingBlk = exitingBlks[i];
      break;
    }
  }
  //assert(exitingBlk && "can't find dominating exiting blk");
  return exitingBlk;
}


unsigned CSACvtCFDFPass::SwitchOutExitingBlk(MachineBasicBlock* exitingBlk, unsigned Reg, MachineLoop *mloop) {
  assert(exitingBlk->succ_size() == 2 && "exiting blok's # of successor not 2");
  MachineBasicBlock* succ1 = *exitingBlk->succ_begin();
  MachineBasicBlock* succ2 = *exitingBlk->succ_rbegin();
  MachineBasicBlock* exitBlk = mloop->contains(succ1) ? succ2 : succ1;
  assert(!mloop->contains(exitBlk));

  //this is case 1, can only have one level nesting difference
  MachineInstr *defSwitchInstr = getOrInsertSWITCHForReg(Reg, exitingBlk);
  unsigned switchFalseReg = defSwitchInstr->getOperand(0).getReg();
  unsigned switchTrueReg = defSwitchInstr->getOperand(1).getReg();
  unsigned newVReg;
  if (CDG->getEdgeType(exitingBlk, exitBlk, true) == ControlDependenceNode::FALSE) {
    //rename Reg to switchTrueReg
    newVReg = switchFalseReg;
  } else {
    //rename it to switchFalseReg
    assert(CDG->getEdgeType(exitingBlk, exitBlk, true) == ControlDependenceNode::TRUE);
    newVReg = switchTrueReg;
  }
  return newVReg;
}

void CSACvtCFDFPass::SwitchDefAcrossExits(unsigned Reg, MachineBasicBlock* mbb, MachineLoop* mloop, MachineOperand &UseMO) {
  SmallVector<MachineBasicBlock*, 2> exitingBlks;
  mloop->getExitingBlocks(exitingBlks);
  MachineInstr* UseMI = UseMO.getParent();
  MachineBasicBlock* UseBB = UseMI->getParent();

  bool DefNotEncloseUse = MLI->getLoopFor(UseBB) == NULL || 
    (MLI->getLoopFor(UseBB) != MLI->getLoopFor(mbb) && 
      !MLI->getLoopFor(mbb)->contains(MLI->getLoopFor(UseBB)));
  //only need to handle use's loop immediately encloses def's loop, otherwise, reduced to case 2 which should already have been run
  if (DefNotEncloseUse) {
    MachineBasicBlock* exitingBlk = getDominatingExitingBB(exitingBlks, UseMI, Reg);
    if (exitingBlk) {
      unsigned outVReg = SwitchOutExitingBlk(exitingBlk, Reg, mloop);
      // Rewrite uses that outside of the original def's block, inside the loop
      //renameLCSSAPhi or other cross boundary uses
      UseMO.setReg(outVReg);
    } else {
      //no exiting blk dominates the useBB:
      // 1)defBB dominates all exiting blks, 
      // 2)UseBB is the enclosing loop's hdr
      SmallVector<MachineInstr*, 8> NewPHIs;
      MachineSSAUpdater SSAUpdate(*thisMF, &NewPHIs);
      const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
      unsigned pickVReg = MRI->createVirtualRegister(TRC);
      SSAUpdate.Initialize(pickVReg);
      SSAUpdate.AddAvailableValue(mbb, Reg);
      for (unsigned i = 0; i < exitingBlks.size(); i++) {
        MachineBasicBlock* exitingBlk = exitingBlks[i];
        unsigned outVReg = SwitchOutExitingBlk(exitingBlk, Reg, mloop);
        SSAUpdate.AddAvailableValue(exitingBlk, outVReg);
      }
      SSAUpdate.RewriteUse(UseMO);
    }
  }
  else {
    // use not enclosing def, def and use in different regions
    // assert(use have to be a switch from the repeat handling pass, or def is a switch from the if handling pass
    // or loop hdr Phi generated by SSAUpdater in handling repeat case)
  }
}





void CSACvtCFDFPass::SwitchDefAcrossLoops(unsigned Reg, MachineBasicBlock* mbb, MachineLoop* mloop) {
  MachineRegisterInfo::use_iterator UI = MRI->use_begin(Reg);
  while (UI != MRI->use_end()) {
    MachineOperand &UseMO = *UI;
    MachineInstr *UseMI = UseMO.getParent();
    ++UI;
    MachineBasicBlock *UseBB = UseMI->getParent();
    
    //for loop hdr Phi, we still need to handle back to back instructions in same block:
    // %y = Phi(%x0, %x)
    // %x = ...
    MachineLoop* useLoop = MLI->getLoopFor(UseBB);
    
    if (mloop != useLoop) {
        //mloop != defLoop
        //two possibilites: a) def dom use;  b) def !dom use;
        //two cases: each can only have one nesting level difference
        // 1) def inside a loop, use outside the loop as LCSSA Phi with single input
        // 2) def outside a loop, use inside the loop, not handled here
        //use, def in different region cross latch
        SwitchDefAcrossExits(Reg, mbb, mloop, UseMO);
    }
  }//end of while (use)
}




//focus on def
void CSACvtCFDFPass::insertSWITCHForLoopExit() {
  DenseMap<MachineBasicBlock *, std::set<unsigned> *> LCSwitch;
  for (MachineLoopInfo::iterator LI = MLI->begin(), LE = MLI->end(); LI != LE; ++LI) {
    insertSWITCHForLoopExit(*LI, LCSwitch);
  }
  //release memory
  DenseMap<MachineBasicBlock *, std::set<unsigned> *> ::iterator itm = LCSwitch.begin();
  while (itm != LCSwitch.end()) {
    std::set<unsigned>* regs = itm->getSecond();
    ++itm;
    delete regs;
  }
  LCSwitch.clear();
}

void CSACvtCFDFPass::insertSWITCHForLoopExit(MachineLoop* L, DenseMap<MachineBasicBlock *, std::set<unsigned> *> &LCSwitch) {
  for (MachineLoop::iterator LI = L->begin(), LE = L->end(); LI != LE; ++LI) {
    insertSWITCHForLoopExit(*LI, LCSwitch);
  }
  MachineLoop *mloop = L;
  for (MachineLoop::block_iterator BI = mloop->block_begin(), BE = mloop->block_end(); BI != BE; ++BI) {
    MachineBasicBlock* mbb = *BI;
    if (LCSwitch.find(mbb) != LCSwitch.end()) {
      //mbb is an exit blk, need to handle defs push in from exiting blk, those are defs of a switch instr
      std::set<unsigned>* LCSwitchs = LCSwitch.find(mbb)->getSecond();
      for (std::set<unsigned>::iterator iReg = LCSwitchs->begin(); iReg != LCSwitchs->end(); ++iReg) {
        SwitchDefAcrossLoops(*iReg, mbb, mloop);
      }
    }
    for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
      MachineInstr *MI = &*I;
      if (TII->isSwitch(MI))
        //encounter a switch just inserted in previous iter
        continue;
      for (MIOperands MO(*MI); MO.isValid(); ++MO) {
        if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
        unsigned Reg = MO->getReg();
        // process defs
        if (MO->isDef()) {
          SwitchDefAcrossLoops(Reg, mbb, mloop);
        }
      }
    }
  }//end of for mbb
  SmallVector<MachineBasicBlock*, 4> exitingBlks;
  mloop->getExitingBlocks(exitingBlks);
  for (unsigned i = 0; i < exitingBlks.size(); i++) {
    MachineBasicBlock* exitingBlk = exitingBlks[i];
    assert(exitingBlk);
    //close definitions live range in exiting blk
    for (MachineBasicBlock::iterator I = exitingBlk->begin(); I != exitingBlk->end(); ++I) {
      MachineInstr *MI = &*I;
      if (TII->isSwitch(MI)) {
        assert(exitingBlk->succ_size() == 2 && "loop exiting blk's # of successor not 2");
        MachineBasicBlock* succ1 = *exitingBlk->succ_begin();
        MachineBasicBlock* succ2 = *exitingBlk->succ_rbegin();
        MachineBasicBlock* exitBlk = mloop->contains(succ1) ? succ2 : succ1;
        unsigned switchOut = (CDG->getEdgeType(exitingBlk, exitBlk, true) == ControlDependenceNode::FALSE) ? 0 : 1;

        std::set<unsigned>* LCSwitchs;
        if (LCSwitch.find(exitBlk) == LCSwitch.end()) {
          LCSwitchs = new std::set<unsigned>;
          LCSwitch[exitBlk] = LCSwitchs;
        } else {
          LCSwitchs = LCSwitch.find(exitBlk)->getSecond();
        }
        LCSwitchs->insert(MI->getOperand(switchOut).getReg());
      }
    } //end of for MI
  }
}


void CSACvtCFDFPass::renameOnLoopEntry()
{
#if 0
  {
    errs() << "after rename for repeat" << ":\n";
    thisMF->print(errs(), getAnalysisIfAvailable<SlotIndexes>());
  }
#endif

  for (MachineLoopInfo::iterator LI = MLI->begin(), LE = MLI->end(); LI != LE; ++LI) {
    renameAcrossLoopForRepeat(*LI);
  }
}



unsigned CSACvtCFDFPass::repeatOperandInLoop(unsigned Reg, MachineLoop* mloop) {
  unsigned newVReg;
  MachineBasicBlock *latchBB = nullptr;
  MachineInstr* dMI = MRI->getVRegDef(Reg);
  MachineBasicBlock* DefBB = dMI->getParent();
  MachineBasicBlock *mlphdr = mloop->getHeader();

  SmallVector<MachineInstr*, 8> NewPHIs;
  MachineSSAUpdater SSAUpdate(*thisMF, &NewPHIs);
  const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
  unsigned hdrPhiVReg = MRI->createVirtualRegister(TRC);
  SSAUpdate.Initialize(hdrPhiVReg);
  SSAUpdate.AddAvailableValue(DefBB, Reg);
  for (MachineBasicBlock::pred_iterator hdrPred = mlphdr->pred_begin(); hdrPred != mlphdr->pred_end(); hdrPred++) {
    if (mloop->contains(*hdrPred)) {
      latchBB = *hdrPred;
    } else 
      continue;
    ControlDependenceNode *mLatch = CDG->getNode(latchBB);

    MachineInstr *defInstr = getOrInsertSWITCHForReg(Reg, latchBB);

    if (TII->isSwitch(defInstr)) {
      unsigned switchFalseReg = defInstr->getOperand(0).getReg();
      unsigned switchTrueReg = defInstr->getOperand(1).getReg();
      if (mLatch->isFalseChild(CDG->getNode(mlphdr))) {
        //rename Reg to switchFalseReg
        newVReg = switchFalseReg;
      } else {
        //rename it to switchTrueReg
        newVReg = switchTrueReg;
      }
    } else {
      //LLVM3.6 buggy latch
      assert(TII->isMOV(defInstr));
      newVReg = defInstr->getOperand(0).getReg();
    }
    SSAUpdate.AddAvailableValue(latchBB, newVReg);
  }
  // Rewrite uses that outside of the original def's block, inside the loop
  MachineRegisterInfo::use_iterator UI = MRI->use_begin(Reg);
  while (UI != MRI->use_end()) {
    MachineOperand &UseMO = *UI;
    MachineInstr *UseMI = UseMO.getParent();
    ++UI;
    if (MLI->getLoopFor(UseMI->getParent()) == mloop) {
      SSAUpdate.RewriteUse(UseMO);
    }
  }
  return hdrPhiVReg;
}





void CSACvtCFDFPass::insertSWITCHForRepeat(MachineLoop* L) {
  for (MachineLoop::iterator LI = L->begin(), LE = L->end(); LI != LE; ++LI) {
    insertSWITCHForRepeat(*LI);
  }
  MachineLoop *mloop = L;
  for (MachineLoop::block_iterator BI = mloop->block_begin(), BE = mloop->block_end(); BI != BE; ++BI) {
    MachineBasicBlock* mbb = *BI;
    //only conside blocks in the  urrent loop level, blocks in the nested level are done before.
    if (MLI->getLoopFor(mbb) != mloop) continue;
    for (MachineBasicBlock::iterator I = mbb->begin(); I != mbb->end(); ++I) {
      MachineInstr *MI = &*I;
      if (MI->isPHI() && mloop->getHeader() == mbb) {
        //loop hdr phi's init input is used only once, no need to repeat
        continue;
      }
      for (MIOperands MO(*MI); MO.isValid(); ++MO) {
        if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
        unsigned Reg = MO->getReg();
        if (MO->isUse()) {
          MachineInstr* dMI = MRI->getVRegDef(Reg);
          MachineBasicBlock* DefBB = dMI->getParent();
          if (DefBB == mbb) continue;
          //use, def in different region cross latch
          bool isDefOutsideLoop = MLI->getLoopFor(DefBB) == NULL ||
            MLI->getLoopFor(mbb) != MLI->getLoopFor(DefBB);

          if (isDefOutsideLoop&& DT->dominates(DefBB, mbb)) {
            repeatOperandInLoop(Reg, mloop);
          }
        }
      }
    }
  }
}

//focus on uses
void CSACvtCFDFPass::insertSWITCHForRepeat() {
  for (MachineLoopInfo::iterator LI = MLI->begin(), LE = MLI->end(); LI != LE; ++LI) {
    insertSWITCHForRepeat(*LI);
  }
}


//sequence OPT is targeting at this transform
//single entry, single exiting, single latch, exiting blk post dominates loop hdr(always execute)
void CSACvtCFDFPass::replaceCanonicalLoopHdrPhi(MachineBasicBlock* mbb) {
  MachineLoop* mloop = MLI->getLoopFor(mbb);
  assert(mloop->getHeader() == mbb);
  if (mbb->getFirstNonPHI() == mbb->begin())
    return;

  assert(mloop->getExitingBlock() && "can't handle multi exiting blks in this funciton");
  MachineBasicBlock *latchBB = mloop->getLoopLatch();
  ControlDependenceNode *latchNode = CDG->getNode(latchBB);
  MachineBasicBlock *exitingBB = mloop->getExitingBlock();
  ControlDependenceNode *exitingNode = CDG->getNode(exitingBB);
  MachineBasicBlock* exitBB = mloop->getExitBlock();
  assert(exitBB);
  assert(latchBB && exitingBB);
  MachineInstr *bi = &*exitingBB->getFirstInstrTerminator();
  MachineBasicBlock::iterator loc = exitingBB->getFirstTerminator();
  unsigned predReg = bi->getOperand(0).getReg();

  const TargetRegisterClass *TRC = MRI->getRegClass(predReg);
  CSAMachineFunctionInfo *LMFI = thisMF->getInfo<CSAMachineFunctionInfo>();
  // Look up target register class corresponding to this register.
  const TargetRegisterClass* new_LIC_RC = LMFI->licRCFromGenRC(MRI->getRegClass(predReg));
  assert(new_LIC_RC && "Can't determine register class for register");
  unsigned cpyReg = LMFI->allocateLIC(new_LIC_RC);
  if (mloop->isLoopExiting(latchBB) || latchNode->isParent(exitingNode)) {
    const unsigned moveOpcode = TII->getMoveOpcode(TRC);
    MachineInstr *cpyInst = BuildMI(*exitingBB, loc, DebugLoc(), TII->get(moveOpcode), cpyReg).addReg(predReg);
    cpyInst->setFlag(MachineInstr::NonSequential);
  } else {
    //need filtering
    //can't using renaming due to maintaing the exiting condition
    ControlDependenceNode *filterNode = latchNode;
    unsigned filterOut = cpyReg; //cpyReg has to be the final output
    unsigned filterIn;
    MachineInstr *filterInst = nullptr;
    do {
      assert(filterNode->getNumParents() == 1 && "not implemented yet");
      ControlDependenceNode *filterParentNode = *filterNode->parent_begin();
      MachineBasicBlock *filterParentBB = filterParentNode->getBlock();
      MachineInstr *filterbi = &*filterParentBB->getFirstInstrTerminator();
      filterIn = MRI->createVirtualRegister(TRC);
      unsigned filterPred = filterbi->getOperand(0).getReg();
      if (filterParentNode->isFalseChild(filterNode)) {
        unsigned notReg = MRI->createVirtualRegister(&CSA::I1RegClass);
        BuildMI(*latchBB, latchBB->getFirstTerminator(), DebugLoc(), TII->get(CSA::NOT1), notReg).addReg(filterPred);
        filterPred = notReg;
      }
      filterInst = BuildMI(*latchBB, filterInst ? filterInst : latchBB->getFirstTerminator(), DebugLoc(), 
                           TII->get(CSA::PREDFILTER), filterOut).addReg(filterIn).addReg(filterPred);
      filterNode = filterParentNode;
      filterOut = filterIn;
    } while (!filterNode->isParent(exitingNode));

    if (CDG->getEdgeType(exitingBB, exitBB, true) == ControlDependenceNode::TRUE) {
      //filtering predReg's false value for inner loops
      unsigned notReg = MRI->createVirtualRegister(&CSA::I1RegClass);
      BuildMI(*latchBB, filterInst, DebugLoc(), TII->get(CSA::NOT1), notReg).addReg(predReg); //fliping the exiting condition
      filterInst->substituteRegister(filterIn, notReg, 0, *TRI);

      unsigned lastFilterReg = MRI->createVirtualRegister(&CSA::I1RegClass);
      MachineInstr *lastFilter = MRI->getVRegDef(cpyReg);
      lastFilter->substituteRegister(cpyReg, lastFilterReg, 0, *TRI);
      BuildMI(*latchBB, latchBB->getFirstTerminator(), DebugLoc(), TII->get(CSA::NOT1), cpyReg).addReg(lastFilterReg); //fliping back
    } else {
      filterInst->substituteRegister(filterIn, predReg, 0, *TRI);
    }
  }
  MachineBasicBlock *lphdr = mloop->getHeader();
  MachineBasicBlock::iterator hdrloc = lphdr->begin();
  const unsigned InitOpcode = TII->getInitOpcode(TRC);
  MachineInstr *initInst = nullptr;
  if (CDG->getEdgeType(exitingBB, exitBB, true) == ControlDependenceNode::FALSE) {
    initInst = BuildMI(*lphdr, hdrloc, DebugLoc(), TII->get(InitOpcode), cpyReg).addImm(0);
  } else {
    initInst = BuildMI(*lphdr, hdrloc, DebugLoc(), TII->get(InitOpcode), cpyReg).addImm(1);
  }
  initInst->setFlag(MachineInstr::NonSequential);

  MachineBasicBlock::iterator iterI = mbb->begin();
  while (iterI != mbb->end()) {
    MachineInstr *MI = &*iterI;
    ++iterI;
    if (!MI->isPHI()) continue;
   
    unsigned numUse = 0;
    MachineOperand* backEdgeInput = nullptr;
    MachineOperand* initInput = nullptr;
    unsigned numOpnd = 0;
    unsigned backEdgeIndex = 0;
    unsigned dst = MI->getOperand(0).getReg();

    for (MIOperands MO(*MI); MO.isValid(); ++MO, ++numOpnd) {
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
        } else {
          initInput = &mOpnd;
        }
      }
    } //end for MO
    if (numUse > 2) {
      //loop hdr phi has more than 2 init inputs, 
      //remove backedge input reduce it to if-foot phi case to be handled by if-footer phi pass
      initInput = &MI->getOperand(0);
      const TargetRegisterClass *TRC = MRI->getRegClass(MI->getOperand(0).getReg());
      unsigned renameReg = MRI->createVirtualRegister(TRC);
      initInput->setReg(renameReg);
    }

    MachineOperand* pickFalse;
    MachineOperand* pickTrue;
    MachineBasicBlock* exitBB = mloop->getExitBlock();
    if (CDG->getEdgeType(exitingBB, exitBB, true) == ControlDependenceNode::FALSE) {
      pickFalse = initInput;
      pickTrue = backEdgeInput;
    } else {
      pickFalse = backEdgeInput;
      pickTrue = initInput;
    }
    TRC = MRI->getRegClass(dst);
    const unsigned pickOpcode = TII->getPickSwitchOpcode(TRC, true /*pick op*/);
    //generate PICK, and insert before MI
    MachineInstr *pickInst = nullptr;
    predReg = cpyReg;
    if (pickFalse->isReg() && pickTrue->isReg()) {
      pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst).addReg(predReg).
        addReg(pickFalse->getReg()).addReg(pickTrue->getReg());
    } else if (pickFalse->isReg()) {
      pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst).addReg(predReg).
        addReg(pickFalse->getReg()).addOperand(*pickTrue);
    } else if (pickTrue->isReg()) {
      pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst).addReg(predReg).
        addOperand(*pickFalse).addReg(pickTrue->getReg());
    } else {
      pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst).addReg(predReg).
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


//single latch, straitht line exitings blks
void CSACvtCFDFPass::replaceStraightExitingsLoopHdrPhi(MachineBasicBlock* mbb) {
  MachineLoop* mloop = MLI->getLoopFor(mbb);
  assert(mloop->getHeader() == mbb);
  MachineBasicBlock *latchBB = mloop->getLoopLatch();
  assert(latchBB);

  SmallVector<MachineBasicBlock*, 4> exitingBlks;
  mloop->getExitingBlocks(exitingBlks);
  assert(exitingBlks.size() > 1);

  std::sort(exitingBlks.begin(), exitingBlks.end(), CmpFcn(bb2rpo));

  unsigned landResult = 0;
  unsigned landSrc = 0;
  MachineInstr* landInstr;
  unsigned i = 0;
  for (; i < exitingBlks.size(); i++) {
    MachineBasicBlock* exiting = exitingBlks[i];
    assert(exiting->succ_size() == 2);
    MachineBasicBlock* exit = mloop->contains(*exiting->succ_begin()) ?
                              *exiting->succ_rbegin() :
                              *exiting->succ_begin();
    MachineInstr* bi = &*exiting->getFirstInstrTerminator();
    unsigned exitReg = bi->getOperand(0).getReg();
    if (CDG->getEdgeType(exiting, exit, true) == ControlDependenceNode::TRUE) {
      unsigned notReg = MRI->createVirtualRegister(&CSA::I1RegClass);
      MachineInstr* notInstr = BuildMI(*latchBB, latchBB->getFirstTerminator(), DebugLoc(), TII->get(CSA::NOT1), 
                                       notReg).
                                       addReg(exitReg);
      notInstr->setFlag(MachineInstr::NonSequential);
      exitReg = notReg;
    }
    if (!landSrc) {
      landSrc = exitReg;
    } else if (!landResult) {
      landResult = MRI->createVirtualRegister(&CSA::I1RegClass);
      landInstr = BuildMI(*latchBB, latchBB->getFirstTerminator(), DebugLoc(), TII->get(CSA::LAND1), 
                          landResult).
                          addReg(landSrc).
                          addReg(exitReg);
      landInstr->setFlag(MachineInstr::NonSequential);
    } else {
      if (i % 4) {
        landInstr->addOperand(MachineOperand::CreateReg(exitReg, false));
      } else {
        unsigned newResult = MRI->createVirtualRegister(&CSA::I1RegClass);
        landInstr = BuildMI(*latchBB, latchBB->getFirstInstrTerminator(), DebugLoc(), TII->get(CSA::LAND1), 
                            newResult).
                            addReg(landResult).
                            addReg(exitReg);
        landInstr->setFlag(MachineInstr::NonSequential);
        landResult = newResult;
      }
    }
  }
  if (i % 4) {
    for (unsigned j = i % 4; j < 4; j++) {
      landInstr->addOperand(MachineOperand::CreateImm(1));
    }
  }


  CSAMachineFunctionInfo *LMFI = thisMF->getInfo<CSAMachineFunctionInfo>();
  // Look up target register class corresponding to this register.
  const TargetRegisterClass* new_LIC_RC = LMFI->licRCFromGenRC(&CSA::I1RegClass);
  assert(new_LIC_RC && "Can't determine register class for register");
  unsigned cpyReg = LMFI->allocateLIC(new_LIC_RC);
  const unsigned moveOpcode = TII->getMoveOpcode(&CSA::I1RegClass);
  MachineInstr *cpyInst = BuildMI(*latchBB, latchBB->getFirstInstrTerminator(), DebugLoc(), TII->get(moveOpcode), cpyReg).addReg(landResult);
  cpyInst->setFlag(MachineInstr::NonSequential);

  MachineBasicBlock *lphdr = mloop->getHeader();
  MachineBasicBlock::iterator hdrloc = lphdr->begin();
  const unsigned InitOpcode = TII->getInitOpcode(&CSA::I1RegClass);
  //land ==1 means take backedge
  MachineInstr *initInst = BuildMI(*lphdr, hdrloc, DebugLoc(), TII->get(InitOpcode), cpyReg).addImm(0);
  initInst->setFlag(MachineInstr::NonSequential);


  MachineBasicBlock::iterator iterI = mbb->begin();
  while (iterI != mbb->end()) {
    MachineInstr *MI = &*iterI;
    ++iterI;
    if (!MI->isPHI()) continue;

    MachineOperand* backEdgeInput = nullptr;
    MachineOperand* initInput = nullptr;
    unsigned numOpnd = 0;
    unsigned dst = MI->getOperand(0).getReg();

    for (MIOperands MO(*MI); MO.isValid(); ++MO, ++numOpnd) {
      if (!MO->isReg()) continue;
      // process use at loop level
      if (MO->isUse()) {
        MachineOperand& mOpnd = *MO;
        ++MO;
        ++numOpnd;
        MachineBasicBlock* inBB = MO->getMBB();
        if (inBB == latchBB) {
          backEdgeInput = &mOpnd;
        } else {
          initInput = &mOpnd;
        }
      }
    } //end for MO
    
    MachineOperand* pickFalse = initInput;
    MachineOperand* pickTrue = backEdgeInput;
    
    unsigned predReg = cpyReg;
    const TargetRegisterClass *TRC = MRI->getRegClass(dst);
    const unsigned pickOpcode = TII->getPickSwitchOpcode(TRC, true /*pick op*/);
    //generate PICK, and insert before MI
    MachineInstr *pickInst = nullptr;
    if (pickFalse->isReg() && pickTrue->isReg()) {
      pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst).addReg(predReg).
        addReg(pickFalse->getReg()).addReg(pickTrue->getReg());
    } else if (pickFalse->isReg()) {
      pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst).addReg(predReg).
        addReg(pickFalse->getReg()).addOperand(*pickTrue);
    } else if (pickTrue->isReg()) {
      pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst).addReg(predReg).
        addOperand(*pickFalse).addReg(pickTrue->getReg());
    } else {
      pickInst = BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst).addReg(predReg).
        addOperand(*pickFalse).addOperand(*pickTrue);
    }

    pickInst->setFlag(MachineInstr::NonSequential);
    MI->removeFromParent();
  }
}



bool CSACvtCFDFPass::hasStraightExitings(MachineLoop* mloop) {
  SmallVector<MachineBasicBlock*, 4> exitingBlks;
  mloop->getExitingBlocks(exitingBlks);
  //single backedge, single exiting
  bool straightlineExitings = mloop->getLoopLatch();
  for (unsigned i = 0; i < exitingBlks.size(); i++) {
    if (!straightlineExitings)
      break;
    MachineBasicBlock* exitingBlk = exitingBlks[i];
    ControlDependenceNode* exitingNd = CDG->getNode(exitingBlk);
    for (ControlDependenceNode::node_iterator uparent = exitingNd->parent_begin(), uparent_end = exitingNd->parent_end();
      uparent != uparent_end; ++uparent) {
      ControlDependenceNode *upnode = *uparent;
      MachineBasicBlock *upbb = upnode->getBlock();
      if (mloop->contains(upbb) && !mloop->isLoopExiting(upbb)) {
        straightlineExitings = false;
        break;
      }
    }
  }
  return straightlineExitings;
}


void CSACvtCFDFPass::replaceLoopHdrPhi() {
  typedef po_iterator<ControlDependenceNode *> po_cdg_iterator;
  ControlDependenceNode *root = CDG->getRoot();
  for (po_cdg_iterator DTN = po_cdg_iterator::begin(root), END = po_cdg_iterator::end(root); DTN != END; ++DTN) {
    MachineBasicBlock *mbb = DTN->getBlock();
    if (!mbb) continue; //root node has no bb
    MachineLoop* mloop = MLI->getLoopFor(mbb);
    //not inside a loop
    if (!mloop) continue;
    MachineBasicBlock* lhdr = mloop->getHeader();
    //only scan loop header
    if (mbb != lhdr) continue;

    SmallVector<MachineBasicBlock*, 4> exitingBlks;
    mloop->getExitingBlocks(exitingBlks);
    //single backedge, single exiting
    bool isCanonical = mloop->getLoopLatch() && mloop->getExitingBlock();
    if (!isCanonical) {
      //TODO: assert loop has only one entry, only canonical loop handing can
      //handle multiple entries by reducing it to if-foot
    }

    if (isCanonical) {
      //single exiting, single latch, with loop latch also the exiting blk
      replaceCanonicalLoopHdrPhi(mbb);
    } else if (hasStraightExitings(mloop)) {
      replaceStraightExitingsLoopHdrPhi(mbb);
    } else {
      assert(false && "not implemented yet");
    }
  }
}

/* Do a sweep over all instructions, looking for direct frame index uses. The
 * use will be replaced with a vreg defined in the entry of the function so
 * that they will be recognized as defs going into any dataflow. This is only
 * intended to handle fixed-size frame objects. The result should still be
 * suitable for SXU execution. */
void CSACvtCFDFPass::createFIEntryDefs() {
  // Build a set of FI users which should be moved to the function entry.
  std::set<MachineOperand*> toReplace;
  // The set of FIs which need vregs created for them in this function.
  std::set<int> entryFIs;
  // A map of FIs to their corresponding virtual registers in the function's
  // entry BB.
  std::map<int, unsigned> fiVRegMap;

  // Collect the FI users which need to be modified.
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    ControlDependenceNode* mNode = CDG->getNode(&*BB);
    bool hasCDGParent = (mNode->getNumParents() > 1 ||
        (mNode->getNumParents() == 1 && (*mNode->parent_begin())->getBlock()));

    // Only worry about uses which aren't already in control flow entry.
    if (!hasCDGParent)
      continue;

    for (MachineBasicBlock::iterator MI = BB->begin(), EI = BB->end(); MI != EI; ++MI) {
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
        TII->get(CSA::MOV64), vReg).addFrameIndex(index);

    fiVRegMap[index] = vReg;
  }

  // Try to eliminate the in-loop def, in case it's now just a redundant mov of
  // the entry def. If the use of the value wasn't as simple as a mov, then
  // just replace the operand.
  for (MachineOperand *mo : toReplace) {
    int index = mo->getIndex();
    unsigned vReg = fiVRegMap[index];
    MachineInstr *oldInst = mo->getParent();

    if (oldInst->getOpcode() == CSA::MOV64 && oldInst->getOperand(0).isReg() &&
        TargetRegisterInfo::isVirtualRegister(oldInst->getOperand(0).getReg())) {
      // If the value we're replacing is just being moved into another virtual
      // register, then replace the whole instruction.
      unsigned oldVReg = oldInst->getOperand(0).getReg();
      MRI->replaceRegWith(oldVReg, vReg);
      oldInst->eraseFromParent();
    } else {
      // This is an assert because I'm not sure that it ever happens.
      assert(false && "Found a non-MOV use of a frame index. This is not handled.");
      // If it does, replacing the use with the new vReg is the thing to do,
      // but doing this (below) alone doesn't seem to be sufficient to get the
      // values correctly passed through picks/switchs. Needs debugging.

      // Just replace the FI operand with our new vReg.
      mo->ChangeToRegister(vReg, false);
    }
  }
}

void CSACvtCFDFPass::assignLicForDF() {
  std::deque<unsigned> renameQueue;
  renameQueue.clear();
  std::set<unsigned> pinedVReg;
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    MachineBasicBlock* mbb = &*BB;
    for (MachineBasicBlock::iterator MI = BB->begin(), EI = BB->end(); MI != EI; ++MI) {
      if (MI->isPHI()) {
        for (MIOperands MO(*MI); MO.isValid(); ++MO) {
          if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
          unsigned Reg = MO->getReg();
          pinedVReg.insert(Reg);
        }
      } else if (MI->getOpcode() == CSA::JSR || MI->getOpcode() == CSA::JSRi) {
        //function call inside control region need to run on SXU
        ControlDependenceNode* mnode = CDG->getNode(mbb);
        if (mnode->getNumParents() > 1 || 
            (mnode->getNumParents() == 1 && (*mnode->parent_begin())->getBlock())) {
          RunSXU = true;
        }
      }
    }
  }

  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator MI = BB->begin(), EI = BB->end(); MI != EI; ++MI) {
      MachineInstr *mInst = &*MI;
      if (TII->isPick(mInst) || TII->isSwitch(mInst) ||  mInst->getOpcode() == CSA::MERGE64f ||
          TII->isFMA(mInst) || TII->isDiv(mInst) || TII->isMul(mInst) || TII->isMOV(mInst) ||
          TII->isAdd(mInst) || TII->isSub(mInst) || TII->isPickany(mInst) ||
          mInst->getOpcode() == CSA::PREDMERGE || 
          mInst->getOpcode() == CSA::PREDPROP || 
          mInst->getOpcode() == CSA::NOT1     ||
          mInst->getOpcode() == CSA::LAND1 ||
          mInst->getOpcode() == CSA::LOR1  || 
          mInst->getOpcode() == CSA::OR1) {
        for (MIOperands MO(*MI); MO.isValid(); ++MO) {
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
        DefMI->substituteRegister(dReg, CSA::IGN, 0, *TRI);
        continue;
    }

    const TargetRegisterClass *TRC = MRI->getRegClass(dReg);
    const TargetRegisterClass* new_LIC_RC = LMFI->licRCFromGenRC(TRC);
    assert(new_LIC_RC && "unknown CSA register class");
    unsigned phyReg = LMFI->allocateLIC(new_LIC_RC);

    if (TII->isSwitch(DefMI)) {
      unsigned trueReg = DefMI->getOperand(1).getReg();
      unsigned falseReg = DefMI->getOperand(0).getReg();
      if (pinedVReg.find(trueReg) != pinedVReg.end() || pinedVReg.find(falseReg) != pinedVReg.end()) {
                DefMI->clearFlag(MachineInstr::NonSequential);
        continue;
      }
    } else if (TII->isMOV(DefMI)) {
      unsigned dstReg = DefMI->getOperand(0).getReg();
      if (pinedVReg.find(dstReg) != pinedVReg.end()) {
        DefMI->clearFlag(MachineInstr::NonSequential);
        continue;
      }
    }

    DefMI->substituteRegister(dReg, phyReg, 0, *TRI);

    MachineRegisterInfo::use_iterator UI = MRI->use_begin(dReg);
    while (UI != MRI->use_end()) {
      MachineOperand &UseMO = *UI;
      ++UI;
      UseMO.setReg(phyReg);
    }

    for (MIOperands MO(*DefMI); MO.isValid(); ++MO) {
      if (!MO->isReg() || &*MO == DefMO || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
      unsigned Reg = MO->getReg();
      renameQueue.push_back(Reg);
    }
  }

  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator MI = BB->begin(), EI = BB->end(); MI != EI; ++MI) {
      bool allLics = true;
      for (MIOperands MO(*MI); MO.isValid(); ++MO) {
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
          // CSAGenRegisterInfo.inc.
          if ((Reg < CSA::CI0_0 || Reg >= CSA::NUM_TARGET_REGS) &&
               Reg != CSA::IGN ) {
            allLics = false;
            break;
          }
        }
      }

      // Check for instructions where all the uses are constants.
      // These instructions shouldn't be moved on to dataflow units,
      // because they keep firing infinitely.
      bool allImmediateUses = true;
      for (MIOperands MO(*MI); MO.isValid(); ++MO) {
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
      if (!allLics && TII->isSwitch(&*MI)) {
        MI->clearFlag(MachineInstr::NonSequential);
      }
    }
  }
}


void CSACvtCFDFPass::handleAllConstantInputs() {
  std::deque<unsigned> renameQueue;
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    MachineBasicBlock* mbb = &*BB;
    MachineBasicBlock::iterator iterMI = BB->begin();
    while(iterMI != BB->end()) {
      MachineInstr* MI = &*iterMI;
      ++iterMI;
      if (!TII->isMOV(MI)) continue;

      bool allConst = true;
      for (MIOperands MO(*MI); MO.isValid(); ++MO) {
        if (MO->isReg() && MO->isDef()) continue;
        if (!MO->isImm() && !MO->isCImm() && !MO->isFPImm()) {
            allConst = false;
            break;
        }
      }
      if (allConst) {
        const TargetRegisterClass *TRC = MRI->getRegClass(MI->getOperand(0).getReg());
        ControlDependenceNode* mNode = CDG->getNode(mbb);
        MachineInstr *pickInst = nullptr;
        MachineInstr *switchInst = nullptr;
        const unsigned switchOpcode = TII->getPickSwitchOpcode(TRC, false);
        const unsigned pickOpcode = TII->getPickSwitchOpcode(TRC, true);
        unsigned pickFalseReg = CSA::IGN, pickTrueReg = CSA::IGN;
        unsigned switchFalse = CSA::IGN, switchTrue = CSA::IGN;
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
          MachineInstr* bi = &*(upnode->getBlock()->getFirstTerminator());
          assert(bi->getOperand(0).isReg());
          unsigned predReg = bi->getOperand(0).getReg();
          unsigned pickReg = 0;
          if (parentN == 1) {
            if (upnode->isFalseChild(mNode)) {
              switchFalse = MI->getOperand(0).getReg();
            } else {
              switchTrue = MI->getOperand(0).getReg();
            }
            switchInst = BuildMI(*BB, MI, DebugLoc(), TII->get(switchOpcode), switchFalse).addReg(switchTrue, RegState::Define).
              addReg(predReg).addOperand(MI->getOperand(1));
            switchInst->setFlag(MachineInstr::NonSequential);
          } else {
            if (parentN == 2) {
              unsigned renameReg = MRI->createVirtualRegister(TRC);
              unsigned index = (switchFalse == CSA::IGN) ? 1 : 0;
              switchInst->getOperand(index).setReg(renameReg);
              pickTrueReg = renameReg;
              pickFalseReg = renameReg;
            }
            pickReg = MRI->createVirtualRegister(TRC);
            if (upnode->isFalseChild(mNode)) {
              pickInst = BuildMI(*BB, MI, DebugLoc(), TII->get(pickOpcode), pickReg).addReg(predReg).
                addOperand(MI->getOperand(1)).
                addReg(pickTrueReg);
            } else {
              pickInst = BuildMI(*BB, MI, DebugLoc(), TII->get(pickOpcode), pickReg).addReg(predReg).
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




void CSACvtCFDFPass::removeBranch() {
  std::deque<unsigned> renameQueue;
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    MachineBasicBlock::iterator iterMI = BB->begin();
    while (iterMI != BB->end()) {
      MachineInstr* MI = &*iterMI;
      ++iterMI;
      if (MI->isBranch()) {
        MI->removeFromParent();
      }
    }
  }
}



void CSACvtCFDFPass::linearizeCFG() {
  typedef po_iterator<MachineBasicBlock *> po_mbb_iterator;
  MachineBasicBlock *root = &*thisMF->begin();
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



MachineInstr* CSACvtCFDFPass::PatchOrInsertPickAtFork(
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
    if (pickFalseReg == CSA::IGN) {
      //reg assigned to pickTrue => make sure the original pick has %IGN for pickTrue;
      assert(pickTrueReg && pickTrueReg != CSA::IGN);
      assert(pickInstr->getOperand(3).getReg() == CSA::IGN);
      ignIndex = 3;
    } else {
      //reg assigned to pickFalse
      assert(pickTrueReg == CSA::IGN);
      assert(pickFalseReg && pickFalseReg != CSA::IGN);
      assert(pickInstr->getOperand(2).getReg() == CSA::IGN);
      ignIndex = 2;
    }
    MachineOperand &MO = pickInstr->getOperand(ignIndex);
    MO.substVirtReg(Reg, 0, TRI);
    MachineInstr *DefMI = MRI->getVRegDef(Reg);
    //if (TII->isPick(DefMI) && DefMI->getParent() == pickInstr->getParent()) {
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


MachineInstr* CSACvtCFDFPass::insertPICKForReg(MachineBasicBlock* ctrlBB, unsigned Reg,
  MachineBasicBlock* inBB, MachineInstr* phi, unsigned pickReg) {
  const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
  MachineBasicBlock::iterator loc = ctrlBB->getFirstTerminator();
  MachineInstr* bi = &*loc;
  if (!pickReg) {
    pickReg = MRI->createVirtualRegister(TRC);
  }
  assert(bi->getOperand(0).isReg());
  unsigned predReg = bi->getOperand(0).getReg();
  unsigned pickFalseReg = 0, pickTrueReg = 0;
  assignPICKSrcForReg(pickFalseReg, pickTrueReg, Reg, ctrlBB, inBB, phi);
  const unsigned pickOpcode = TII->getPickSwitchOpcode(TRC, true /*pick op*/);
  MachineInstr *pickInst = BuildMI(*phi->getParent(), phi, DebugLoc(), TII->get(pickOpcode), pickReg).addReg(predReg).
    addReg(pickFalseReg).
    addReg(pickTrueReg);
  pickInst->setFlag(MachineInstr::NonSequential);
  multiInputsPick.insert(pickInst);
  return pickInst;
}

void CSACvtCFDFPass::assignPICKSrcForReg(unsigned &pickFalseReg, unsigned &pickTrueReg, unsigned Reg, MachineBasicBlock* ctrlBB, MachineBasicBlock* inBB, MachineInstr* phi) {
  if (inBB) {
    ControlDependenceNode* inNode = CDG->getNode(inBB);
    ControlDependenceNode* ctrlNode = CDG->getNode(ctrlBB);
    if (ctrlNode->isFalseChild(inNode)) {
      pickFalseReg = Reg;
      pickTrueReg = CSA::IGN;
    } else {
      pickTrueReg = Reg;
      pickFalseReg = CSA::IGN;
    }
  } else {
    MachineBasicBlock* mbb = phi->getParent();
    //assert(DT->dominates(ctrlBB, mbb));
    if (CDG->getEdgeType(ctrlBB, mbb, true) == ControlDependenceNode::TRUE) {
      pickTrueReg = Reg;
      pickFalseReg = CSA::IGN;
    } else {
      pickFalseReg = Reg;
      pickTrueReg = CSA::IGN;
    }
  }
}


void CSACvtCFDFPass::generateCompletePickTreeForPhi(MachineBasicBlock* mbb) {
  MachineBasicBlock::iterator iterI = mbb->begin();
  while (iterI != mbb->end()) {
    MachineInstr *MI = &*iterI;
    ++iterI;
    if (!MI->isPHI()) continue;
    multiInputsPick.clear();
    unsigned dst = MI->getOperand(0).getReg();
    for (MIOperands MO(*MI); MO.isValid(); ++MO) {
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
        } else {
          bool inBBFork = inBB->succ_size() > 1 && (!MLI->getLoopFor(inBB) || MLI->getLoopFor(inBB)->getLoopLatch() != inBB);          
		  if (inBBFork) {
            MachineInstr* pickInstr = PatchOrInsertPickAtFork(inBB, dst, Reg, nullptr, MI, 0);
            if (!pickInstr) {
              //patched
              continue;  //to next MO
            } else {
              Reg = pickInstr->getOperand(0).getReg();
            }
          }
          TraceCtrl(inBB, mbb, Reg, dst, MI);
        }
      }
    } //end of for MO
    MI->removeFromParent();
  }
}

unsigned CSACvtCFDFPass::getEdgePred(MachineBasicBlock* mbb, ControlDependenceNode::EdgeType childType) {
  if (edgepreds.find(mbb) == edgepreds.end()) return 0;
  return (*edgepreds[mbb])[childType];
}

void CSACvtCFDFPass::setEdgePred(MachineBasicBlock* mbb, ControlDependenceNode::EdgeType childType, unsigned ch) {
  assert(ch && "0 is not a valid vreg number");
  if (edgepreds.find(mbb) == edgepreds.end()) {
    SmallVectorImpl<unsigned>* childVect = new SmallVector<unsigned, 2>;
    childVect->push_back(0);
    childVect->push_back(0);
    edgepreds[mbb] = childVect;
  }
  (*edgepreds[mbb])[childType] = ch;
}

unsigned CSACvtCFDFPass::getBBPred(MachineBasicBlock* mbb) {
  if (bbpreds.find(mbb) == bbpreds.end()) return 0;
  return bbpreds[mbb];
}


void CSACvtCFDFPass::setBBPred(MachineBasicBlock* mbb, unsigned ch) {
  assert(ch && "0 is not a valid vreg number");
  //don't set it twice
  assert(bbpreds.find(mbb) == bbpreds.end() && "CSA: Try to set bb pred twice");
  bbpreds[mbb] = ch;
}

unsigned CSACvtCFDFPass::computeEdgePred(MachineBasicBlock* fromBB, 
                                         MachineBasicBlock* toBB,
                                         std::list<MachineBasicBlock*> &path) {
  assert(toBB);
  ControlDependenceNode::EdgeType childType = CDG->getEdgeType(fromBB, toBB);
  //make sure each edge is computed only once to avoid cycling on backedge
  if (unsigned edgeReg = getEdgePred(fromBB, childType)) {
    return edgeReg;
  }
  unsigned bbPredReg = 0;
  if (*CDG->getNode(fromBB)->parent_begin() == CDG->getRoot()) {
    //root level, out side any loop
    //mov 1
    MachineBasicBlock* entryBB = &*thisMF->begin();
    unsigned cpyReg = MRI->createVirtualRegister(&CSA::I1RegClass);
    const unsigned moveOpcode = TII->getMoveOpcode(&CSA::I1RegClass);
    BuildMI(*entryBB, entryBB->getFirstTerminator(), DebugLoc(), TII->get(moveOpcode), cpyReg).addImm(1);
    bbPredReg = cpyReg;
  } else if (MLI->getLoopFor(toBB) &&
    MLI->getLoopFor(toBB)->getHeader() == toBB &&
    !MLI->getLoopFor(toBB)->contains(fromBB)) {
    //fromBB outside current loop, using loop as the unit of the region,
    //reaching the boundary, use pre-assigned BB pred for init input of loop
    if (bb2predcpy.find(fromBB) == bb2predcpy.end()) {
      bbPredReg = MRI->createVirtualRegister(&CSA::I1RegClass);
      bb2predcpy[fromBB] = bbPredReg;
    } else {
      bbPredReg = bb2predcpy[fromBB];
    }
    if (!MLI->getLoopFor(fromBB)) {
      //post process after all loops are handled
      dcgBBs.insert(fromBB);
    }
  } else { 
    //non boundary -- need to compute
    bbPredReg = computeBBPred(fromBB, path);
  }
  unsigned result = 0;
  if (fromBB->succ_size() == 1) {
    result = bbPredReg;
  } else {
    //calling omputeBBPred => calling computeEdgePred 
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




unsigned CSACvtCFDFPass::mergeIncomingEdgePreds(MachineBasicBlock* inBB, std::list<MachineBasicBlock*> &path) {
  unsigned mergeOp, numInputs;
  if (MLI->getLoopFor(inBB) && MLI->getLoopFor(inBB)->getHeader() == inBB) {
    mergeOp = CSA::LOR1;
    numInputs = 4;
  } else {
    mergeOp = CSA::OR1;
    numInputs = 2;
  }

  unsigned predBB = 0; //lor Result
  MachineBasicBlock* ctrlBB = nullptr;
  unsigned ctrlEdge;
  unsigned lorSrc = 0;
  MachineInstr *lorInstr = nullptr;
  unsigned i = 0;
  unsigned backedgeCtrl = 0;
  for (MachineBasicBlock::pred_iterator ipred = inBB->pred_begin();
    ipred != inBB->pred_end(); ipred++, i++) {
    ctrlBB = *ipred;
    //no need to (bb2rpo[ctrlBB] < bb2rpo[inBB]), as long as each edge is computed once.
    //handle single edge as well as fork edge
    ctrlEdge = computeEdgePred(ctrlBB, inBB, path);
    if (MLI->getLoopFor(ctrlBB) && MLI->getLoopFor(ctrlBB)->isLoopLatch(ctrlBB)) {
      backedgeCtrl = ctrlEdge;
    }
    MachineBasicBlock::iterator loc = inBB->getFirstTerminator();
    //merge predecessor if needed
    if (!lorSrc) {
      lorSrc = ctrlEdge;
    } else if (!predBB) {
      predBB = MRI->createVirtualRegister(&CSA::I1RegClass);
      assert(TargetRegisterInfo::isVirtualRegister(ctrlEdge));
      assert(TargetRegisterInfo::isVirtualRegister(lorSrc));
      lorInstr = BuildMI(*inBB, loc, DebugLoc(), TII->get(mergeOp), predBB).addReg(lorSrc).addReg(ctrlEdge);
      lorInstr->setFlag(MachineInstr::NonSequential);
    } else {
      if ((i % numInputs) && (numInputs > 2)) {
        lorInstr->addOperand(MachineOperand::CreateReg(ctrlEdge, false));
      } else {
        unsigned newResult = MRI->createVirtualRegister(&CSA::I1RegClass);
        lorInstr = BuildMI(*inBB, loc, DebugLoc(), TII->get(mergeOp),
          newResult).
          addReg(predBB).
          addReg(ctrlEdge);
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

  //make sure back edge pred is the first operand of lor, otherwise exchang operands;
  //to avoid repeat operand
  MachineLoop* mloop = MLI->getLoopFor(inBB);
  if (mloop && mloop->getHeader() == inBB) {
    assert(inBB->pred_size() == 2);
    MachineInstr *lorInstr = MRI->getVRegDef(predBB);
    unsigned reg = lorInstr->getOperand(2).getReg();
    if (reg == backedgeCtrl) {
      //exchnage operands
      MachineOperand &MO2 = lorInstr->getOperand(2);
      MachineOperand &MO1 = lorInstr->getOperand(1);
      MO2.setReg(MO1.getReg());
      MO1.setReg(reg);
    }
  }
  return predBB;

}

unsigned CSACvtCFDFPass::computeBBPred(MachineBasicBlock* inBB, std::list<MachineBasicBlock*> &path) {
  if (unsigned c = getBBPred(inBB)) {
    return c;
  } else {
    if (std::find(path.begin(), path.end(), inBB) != path.end()) {
      //find a cycle 
      return bb2predcpy[inBB];
    }

    path.push_back(inBB);

    unsigned result;
    if (bb2predcpy.find(inBB) == bb2predcpy.end()) {
      result = MRI->createVirtualRegister(&CSA::I1RegClass);
      bb2predcpy[inBB] = result;
    } else {
      result = bb2predcpy[inBB];
    }
    
    unsigned predBB = mergeIncomingEdgePreds(inBB, path);
    
    setBBPred(inBB, predBB);

    path.pop_back();
    //copy predBB to result for cycle of depencence; dead code it if not used
    if (!MRI->use_empty(result)) {
      MachineInstr *movPred = BuildMI(*inBB, inBB->getFirstTerminator(), DebugLoc(), TII->get(CSA::MOV1), result).addReg(predBB);
      MachineRegisterInfo::use_iterator UI = MRI->use_begin(result);
      MachineInstr *usePred = UI->getParent();
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


MachineInstr* CSACvtCFDFPass::getOrInsertPredMerge(MachineBasicBlock* mbb, MachineInstr* loc, unsigned e1, unsigned e2) {
  MachineInstr* predMergeInstr = nullptr;
  if (bb2predmerge.find(mbb) == bb2predmerge.end()) {
    unsigned indexReg = MRI->createVirtualRegister(&CSA::I1RegClass);
    predMergeInstr = BuildMI(*mbb, loc, DebugLoc(), TII->get(CSA::PREDMERGE),
      CSA::IGN).    //in a two-way merge, it is %IGN to eat the BB's pred, they will be computed using "or" consistently
      addReg(indexReg, RegState::Define). 
      addReg(e1).   //last processed edge
      addReg(e2); //current edge
    bb2predmerge[mbb] = predMergeInstr;
  } else {
    predMergeInstr = bb2predmerge[mbb];
  }
  return predMergeInstr;
}

//also set up edge pred
MachineInstr* CSACvtCFDFPass::InsertPredProp(MachineBasicBlock* mbb, unsigned bbPred) {
  assert(mbb->succ_size() == 2);
  if (bbPred == 0) {
    bbPred = getBBPred(mbb);
  }
  unsigned falseEdge = MRI->createVirtualRegister(&CSA::I1RegClass);
  unsigned trueEdge = MRI->createVirtualRegister(&CSA::I1RegClass);
  MachineBasicBlock::iterator loc = mbb->getFirstTerminator();
  MachineInstr* bi = &*loc;
  MachineInstr* predPropInstr = BuildMI(*mbb, loc, DebugLoc(), TII->get(CSA::PREDPROP),
    falseEdge).
    addReg(trueEdge, RegState::Define).
    addReg(bbPred).
    addReg(bi->getOperand(0).getReg());
  setEdgePred(mbb, ControlDependenceNode::FALSE, falseEdge);
  setEdgePred(mbb, ControlDependenceNode::TRUE, trueEdge);
  return predPropInstr;
}


bool CSACvtCFDFPass::needDynamicPreds() {
  for (MachineLoopInfo::iterator LI = MLI->begin(), LE = MLI->end(); LI != LE; ++LI) {
    if (needDynamicPreds(*LI)) {
      return true;
    }
  }

  for (MachineFunction::iterator ibb = thisMF->begin(); ibb != thisMF->end(); ibb++) {
    MachineBasicBlock* mbb = &*ibb;
    if (isUnStructured(mbb)) return true;
  }
  return false;
}



bool CSACvtCFDFPass::needDynamicPreds(MachineLoop* L) {
  for (MachineLoop::iterator LI = L->begin(), LE = L->end(); LI != LE; ++LI) {
    if (needDynamicPreds(*LI)) return true;
  }
  MachineLoop *mloop = L;
  //multiple exiting blocks
  if (!mloop->getExitingBlock()) return true;
  MachineBasicBlock* lhdr = mloop->getHeader();

  if (lhdr->pred_size() > 2) return true;
  //multiple backedges
  MachineBasicBlock* latch = mloop->getLoopLatch();
  if (!latch) return true;
  //loop lattch is not an exiting point
  if (!mloop->isLoopExiting(mloop->getLoopLatch())) return true;

  for (MachineLoop::block_iterator BI = mloop->block_begin(), BE = mloop->block_end(); BI != BE; ++BI) {
    MachineBasicBlock* mbb = *BI;
    if (isUnStructured(mbb)) return true;
  }
  return false;
}




void CSACvtCFDFPass::generateDynamicPreds() {
  for (MachineLoopInfo::iterator LI = MLI->begin(), LE = MLI->end(); LI != LE; ++LI) {
    generateDynamicPreds(*LI);
  }

  for (std::set<MachineBasicBlock*>::iterator ibb = dcgBBs.begin(); ibb != dcgBBs.end(); ibb++) {
    MachineBasicBlock* mbb = *ibb;
    std::list<MachineBasicBlock*> path;
    computeBBPred(mbb, path);
  }
  for (MachineFunction::iterator ibb = thisMF->begin(); ibb != thisMF->end(); ibb++) {
    MachineBasicBlock* mbb = &*ibb;
    if (!MLI->getLoopFor(mbb) &&
        mbb->getFirstNonPHI() != mbb->begin()) {
      generateDynamicPickTreeForFooter(mbb);
    }
  }
}



void CSACvtCFDFPass::generateDynamicPreds(MachineLoop* L) {
  for (MachineLoop::iterator LI = L->begin(), LE = L->end(); LI != LE; ++LI) {
    generateDynamicPreds(*LI);
  }
  MachineLoop *mloop = L;
  MachineBasicBlock* lhdr = mloop->getHeader();
  
  generateDynamicPickTreeForHeader(lhdr);

  for (MachineLoop::block_iterator BI = mloop->block_begin(), BE = mloop->block_end(); BI != BE; ++BI) {
    MachineBasicBlock* mbb = *BI;
    //only conside blocks in the  urrent loop level, blocks in the nested level are done before.
    if (MLI->getLoopFor(mbb) != mloop) continue;
    for (MachineBasicBlock::succ_iterator psucc = mbb->succ_begin(); psucc != mbb->succ_end(); psucc++) {
      MachineBasicBlock* msucc = *psucc;
      //need to drive the compute for BB that is nested loop's preheader, in case the current loop missed it
      if (MLI->getLoopFor(msucc) &&
        MLI->getLoopFor(msucc) != mloop &&
        MLI->getLoopFor(msucc)->getHeader() == msucc) {
        std::list<MachineBasicBlock*> path;
        computeBBPred(mbb, path);
        break;
      }
    }

    if (mbb->getFirstNonPHI() == mbb->begin()) continue;
    generateDynamicPickTreeForFooter(mbb);
  }
}



void CSACvtCFDFPass::generateDynamicPickTreeForHeader(MachineBasicBlock* mbb) {
  SmallVector<std::pair<unsigned, unsigned> *, 4> pred2values;
  std::list<MachineBasicBlock*> path;
  MachineLoop* mloop = MLI->getLoopFor(mbb);
  //a loop header phi
  assert(mloop && mloop->getHeader() == mbb);
  MachineBasicBlock* latchBB = mloop->getLoopLatch();
  assert(latchBB && "TODO:: handle multiple backedges not implemented yet");

  unsigned backedgePred = 0;
  for (MachineBasicBlock::pred_iterator ipred = mbb->pred_begin(); ipred != mbb->pred_end(); ipred++) {
    MachineBasicBlock *inBB = *ipred;
    //this will cause lhdr BBPred get evaluated due to cycle
    unsigned edgePred = computeEdgePred(inBB, mbb, path);
    if (inBB == latchBB) {
      backedgePred = edgePred;
    }
  }
  assert(backedgePred);
  //incoming edge pred evaluation doesn't result in the bb pred get evaluated
  //due to nested loop
  if (!getBBPred(mbb)) {
    computeBBPred(mbb, path);
  }

  unsigned pb1 = MRI->createVirtualRegister(&CSA::I1RegClass);
  //filter 0 value backedge
  const unsigned switchOpcode = TII->getPickSwitchOpcode(&CSA::I1RegClass, false /*not pick op*/);
  MachineInstr *switchInst = BuildMI(*latchBB, latchBB->getFirstTerminator(), DebugLoc(), TII->get(switchOpcode),
    CSA::IGN).
    addReg(pb1, RegState::Define).
    addReg(backedgePred).
    addImm(1);
  switchInst->setFlag(MachineInstr::NonSequential);

  //combine exit condition's pred
  unsigned orSrc = 0;
  unsigned orResult = 0;
  MachineInstr* orInstr = nullptr;
  SmallVector<MachineBasicBlock*, 4> exitingBlks;
  mloop->getExitingBlocks(exitingBlks);
  for (unsigned i = 0; i < exitingBlks.size(); ++i) {
    MachineBasicBlock* exitingBlk = exitingBlks[i];
    assert(exitingBlk->succ_size() == 2);
    MachineBasicBlock* succ1 = *exitingBlk->succ_begin();
    MachineBasicBlock* succ2 = *exitingBlk->succ_rbegin();
    MachineBasicBlock* exitBlk = mloop->contains(succ1) ? succ2 : succ1;
    unsigned ec = computeEdgePred(exitingBlk, exitBlk, path);
    if (!orSrc) {
      orSrc = ec;
    } else if (!orResult) {
      orResult = MRI->createVirtualRegister(&CSA::I1RegClass);
      orInstr = BuildMI(*latchBB, latchBB->getFirstTerminator(), DebugLoc(), TII->get(CSA::OR1),
        orResult).
        addReg(orSrc).
        addReg(ec);
      orInstr->setFlag(MachineInstr::NonSequential);
    } else {
      unsigned newResult = MRI->createVirtualRegister(&CSA::I1RegClass);
      orInstr = BuildMI(*latchBB, latchBB->getFirstInstrTerminator(), DebugLoc(), TII->get(CSA::OR1),
        newResult).
        addReg(orResult).
        addReg(ec);
      orInstr->setFlag(MachineInstr::NonSequential);
      orResult = newResult;
    }
  }
  unsigned exitPred = orResult ? orResult : orSrc;

  //assign loop pred and its reset value
  const TargetRegisterClass* new_LIC_RC = LMFI->licRCFromGenRC(MRI->getRegClass(backedgePred));
  assert(new_LIC_RC && "Can't determine register class for register");
  unsigned loopPred = LMFI->allocateLIC(new_LIC_RC);

  MachineInstr *pickPB1 = BuildMI(*latchBB, latchBB->getFirstInstrTerminator(), DebugLoc(), TII->get(CSA::PICK1),
    loopPred).
    addReg(exitPred).
    addReg(pb1).
    addImm(0);
  pickPB1->setFlag(MachineInstr::NonSequential);
  const unsigned InitOpcode = TII->getInitOpcode(&CSA::I1RegClass);
  MachineBasicBlock::iterator hdrloc = mbb->begin();
  // init loopPred 0;
  BuildMI(*mbb, hdrloc, DebugLoc(), TII->get(InitOpcode), loopPred).addImm(0);

#if 1
  unsigned hdrPred = getBBPred(mbb);
  assert(hdrPred);
  MachineInstr *lorInstr = MRI->getVRegDef(hdrPred);
  assert(lorInstr->getOpcode() == CSA::LOR1);
  lorInstr->substituteRegister(lorInstr->getOperand(1).getReg(), loopPred, 0, *TRI);

  //filter 0 value init edge
  unsigned pi1 = MRI->createVirtualRegister(&CSA::I1RegClass);
  MachineInstr *switchInit = BuildMI(*lorInstr->getParent(), lorInstr, DebugLoc(), TII->get(switchOpcode),
    CSA::IGN).
    addReg(pi1, RegState::Define).
    addReg(lorInstr->getOperand(2).getReg()).
    addImm(1);
  lorInstr->substituteRegister(lorInstr->getOperand(2).getReg(), pi1, 0, *TRI);
#endif


  //fiiltering exit edge pred, only keep the value when exit condition is true
  for (unsigned i = 0; i < exitingBlks.size(); ++i) {
    MachineBasicBlock* exitingBlk = exitingBlks[i];
    assert(exitingBlk->succ_size() == 2);
    MachineBasicBlock* succ1 = *exitingBlk->succ_begin();
    MachineBasicBlock* succ2 = *exitingBlk->succ_rbegin();
    MachineBasicBlock* exitBlk = mloop->contains(succ1) ? succ2 : succ1;
    //switch off the exit edge value generated when exit condition is false;
    unsigned ec = getEdgePred(exitingBlk, CDG->getEdgeType(exitingBlk, exitBlk, true));
    unsigned ec1 = MRI->createVirtualRegister(&CSA::I1RegClass);
    MachineInstr *switchEC = BuildMI(*exitingBlk, exitingBlk->getFirstInstrTerminator(), DebugLoc(), TII->get(CSA::SWITCH1),
      CSA::IGN).
      addReg(ec1, RegState::Define).
      addReg(exitPred).
      addReg(ec);
    switchEC->setFlag(MachineInstr::NonSequential);

    //pick exit edge value from 0 when loop is not executed, and loop exiting value(ec1) when loop is executed using loop initial condition
    unsigned exitEdgePred = MRI->createVirtualRegister(&CSA::I1RegClass);
    MachineInstr *pickEC1 = BuildMI(*exitingBlk, exitingBlk->getFirstInstrTerminator(), DebugLoc(), TII->get(CSA::PICK1),
      exitEdgePred).
      addReg(switchInit->getOperand(2).getReg()).
      addImm(0).
      addReg(ec1);
    pickEC1->setFlag(MachineInstr::NonSequential);
    //reset the exit edge pred to the value after filtering
    setEdgePred(exitingBlk, CDG->getEdgeType(exitingBlk, exitBlk, true), exitEdgePred);
  }
  MachineBasicBlock::iterator iterI = mbb->begin();
  while (iterI != mbb->end()) {
    MachineInstr *MI = &*iterI;
    ++iterI;
    if (!MI->isPHI()) continue;
    unsigned initInput = 0;
    unsigned backedgeInput = 0;
    for (MIOperands MO(*MI); MO.isValid(); ++MO) {
      if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
      if (MO->isUse()) {
        unsigned Reg = MO->getReg();
        //move to its incoming block operand
        ++MO;
        MachineBasicBlock* inBB = MO->getMBB();
        if (inBB == latchBB) {
          backedgeInput = Reg;
        } else {
          initInput = Reg;
        }
      }
    } 
    unsigned dst = MI->getOperand(0).getReg();
    //if we have two-way predMerge available, use predmerge/pick combination to generated pick directly
    const TargetRegisterClass *TRC = MRI->getRegClass(dst);
    const unsigned pickOpcode = TII->getPickSwitchOpcode(TRC, true /*pick op*/);
    BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst).addReg(loopPred).
                                                                   addReg(initInput).
                                                                   addReg(backedgeInput);
    MI->removeFromParent();
  }
}

void CSACvtCFDFPass::generateDynamicPickTreeForFooter(MachineBasicBlock* mbb) {
  SmallVector<std::pair<unsigned, unsigned> *, 4> pred2values;
  unsigned predBB = 0;
  MachineInstr* predMergeInstr = nullptr;
  std::list<MachineBasicBlock*> path;
  MachineBasicBlock::iterator iterI = mbb->begin();
  while (iterI != mbb->end()) {
    MachineInstr *MI = &*iterI;
    ++iterI;
    if (!MI->isPHI()) continue;
    //not a loop header phi
    assert(!MLI->getLoopFor(mbb) || MLI->getLoopFor(mbb)->getHeader() != mbb);
    pred2values.clear();
    for (MIOperands MO(*MI); MO.isValid(); ++MO) {
      if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
      if (MO->isUse()) {
        unsigned Reg = MO->getReg();
        //move to its incoming block operand
        ++MO;
        MachineBasicBlock* inBB = MO->getMBB();
        unsigned edgePred = computeEdgePred(inBB, mbb, path);
        std::pair<unsigned, unsigned>* pred2value = new std::pair<unsigned, unsigned>;
        pred2value->first = edgePred;
        pred2value->second = Reg;
        pred2values.push_back(pred2value);
        //merge incoming edge pred to generate BB pred
        if (!predBB) {
          predBB = edgePred;
        } else if (MI->getNumOperands() == 5) {
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
      const unsigned pickOpcode = TII->getPickSwitchOpcode(TRC, true /*pick op*/);
      BuildMI(*mbb, MI, MI->getDebugLoc(), TII->get(pickOpcode), dst).addReg(pickPred).addReg(reg1).addReg(reg2);
    }
    else {
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
          xphi->addOperand(edgeOp);
          xphi->addOperand(valueOp);
        }
      }
#else 
      CombineDuplicatePhiInputs(pred2values, MI);
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
}

void CSACvtCFDFPass::CombineDuplicatePhiInputs(SmallVectorImpl<std::pair<unsigned, unsigned> *> &pred2values, MachineInstr* iPhi) {
  unsigned pairsLen = pred2values.size();
  for (unsigned i = 0; i < pairsLen; i++) {
    std::pair<unsigned, unsigned> *pair1 = pred2values[i];
    for (unsigned j = i+1; j < pairsLen;) {
      std::pair<unsigned, unsigned> *pair2 = pred2values[j];
      if (pair1->second == pair2->second) {
        unsigned orResult = MRI->createVirtualRegister(&CSA::I1RegClass);
        MachineInstr* orInstr = BuildMI(*iPhi->getParent(), iPhi, DebugLoc(), TII->get(CSA::OR1),
          orResult).
          addReg(pair1->first).
          addReg(pair2->first);
        orInstr->setFlag(MachineInstr::NonSequential);
        pair1->first = orResult;
        //remove pair2
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


void CSACvtCFDFPass::LowerXPhi(SmallVectorImpl<std::pair<unsigned, unsigned> *> &pred2values, MachineInstr* loc) {
  if (pred2values.empty() || pred2values.size() == 1) return;
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

      unsigned indexReg = MRI->createVirtualRegister(&CSA::I1RegClass);
      unsigned bbpredReg = MRI->createVirtualRegister(&CSA::I1RegClass);
      BuildMI(*loc->getParent(), loc, DebugLoc(), TII->get(CSA::PREDMERGE),
        bbpredReg).
        addReg(indexReg, RegState::Define).
        addReg(pair1->first).   //last processed edge
        addReg(pair2->first); //current edge

      const TargetRegisterClass *vTRC = MRI->getRegClass(pair1->second);
      const unsigned pickOpcode = TII->getPickSwitchOpcode(vTRC, true /*pick op*/);
      unsigned pickDst;
      if (pred2values.size() == 2) {
        pickDst = loc->getOperand(0).getReg();
      } else {
        pickDst = MRI->createVirtualRegister(vTRC);
      }
      BuildMI(*loc->getParent(), loc, loc->getDebugLoc(),
        TII->get(pickOpcode), pickDst).
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




bool CSACvtCFDFPass::isUnStructured(MachineBasicBlock* mbb) {
  MachineBasicBlock::iterator iterI = mbb->begin();
  while (iterI != mbb->end()) {
    MachineInstr *MI = &*iterI;
    ++iterI;
    if (!MI->isPHI()) continue;
    //check to see if needs PREDPROP/PREDMERGE
    //loop hdr phi with multiple back edges or loop with multiple exit blocks
    if (MLI->getLoopFor(mbb) && MLI->getLoopFor(mbb)->getHeader() == mbb) {
      MachineLoop* mloop = MLI->getLoopFor(mbb);
      if (mloop->getNumBackEdges() > 1) {
        return true;
      }
      if (mloop->getExitingBlock() == nullptr) {
        return !hasStraightExitings(mloop);
      }
#if 1 
      MachineBasicBlock* mlatch = mloop->getLoopLatch();
      assert(mlatch);
      ControlDependenceNode* nlatch = CDG->getNode(mlatch);
      bool oneAndOnly = true;
      getNonLatchParent(nlatch, oneAndOnly);
      if (!oneAndOnly) 
        return true;
#endif
    } else {
      for (MIOperands MO(*MI); MO.isValid(); ++MO) {
        if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
        if (MO->isUse()) {
          //move to its incoming block operand
          ++MO;
          MachineBasicBlock* inBB = MO->getMBB();
          if (!PDT->dominates(mbb, inBB) || !CheckPhiInputBB(inBB, mbb)) {
            return true;
          }
        }
      }
    }
  }
  return false;
}




void CSACvtCFDFPass::replacePhiForUnstructed() {
  for (MachineLoopInfo::iterator LI = MLI->begin(), LE = MLI->end(); LI != LE; ++LI) {
    replacePhiForUnstructedLoop(*LI);
  }

  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    MachineBasicBlock* mbb = &*BB;
    if (MLI->getLoopFor(mbb) == nullptr) {
      MachineBasicBlock::iterator iterI = mbb->begin();
      while (iterI != mbb->end()) {
        MachineInstr *iPhi = &*iterI;
        ++iterI;
        if (!iPhi->isPHI()) continue;
        unsigned dst = iPhi->getOperand(0).getReg();
        const TargetRegisterClass *TRC = MRI->getRegClass(dst);
        unsigned pickanyReg = 0;
        MachineInstr *pickanyInst = nullptr;
        for (MIOperands MO(*iPhi); MO.isValid(); ++MO) {
          if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
          unsigned Reg = MO->getReg();
          // process uses
          if (MO->isUse()) {
            ++MO;
            //pickany to replace phi 
            if (pickanyReg == 0) {
              pickanyReg = Reg;
            } else {
              unsigned valueReg = MRI->createVirtualRegister(TRC);
              const unsigned pickanyOpcode = TII->getPickanyOpcode(TRC);
              pickanyInst = BuildMI(*mbb, iPhi, DebugLoc(), TII->get(pickanyOpcode),
                valueReg).
                addReg(CSA::IGN, RegState::Define).
                addReg(pickanyReg).
                addReg(Reg);
              pickanyInst->setFlag(MachineInstr::NonSequential);
              pickanyReg = valueReg;
            }
          }
        }
        //update last pickany's register to phi's dest, and the index reg used in "ordered" switch
        pickanyInst->substituteRegister(pickanyReg, dst, 0, *TRI);
        iPhi->removeFromParent();
      }
    }
  }
}


void CSACvtCFDFPass::replacePhiForUnstructedLoop(MachineLoop* L) {
  for (MachineLoop::iterator LI = L->begin(), LE = L->end(); LI != LE; ++LI) {
    replacePhiForUnstructedLoop(*LI);
  }
  MachineLoop *mloop = L;
  unsigned predReg;
  MachineBasicBlock* lhdr = mloop->getHeader();
  //pickany for ordering based on exiting condition
  if (lhdr->getFirstNonPHI() != lhdr->begin()) {
    //hdr phi's init input controller
    // Look up target register class corresponding to this register.
    const TargetRegisterClass* new_LIC_RC = LMFI->licRCFromGenRC(&CSA::I1RegClass);
    assert(new_LIC_RC && "Can't determine register class for register");
    predReg = LMFI->allocateLIC(new_LIC_RC);
    const unsigned InitOpcode = TII->getInitOpcode(&CSA::I1RegClass);
    MachineInstr *initInst = BuildMI(*lhdr, lhdr->begin(), DebugLoc(), TII->get(InitOpcode), predReg).addImm(1);
    initInst->setFlag(MachineInstr::NonSequential);

    SmallVector<MachineBasicBlock*, 4> latchBBs;
    mloop->getLoopLatches(latchBBs);
    MachineBasicBlock *latchBB = latchBBs[0];

    SmallVector<MachineBasicBlock*, 4> exitingBlks;
    mloop->getExitingBlocks(exitingBlks);

    unsigned anyec = 0;
    MachineInstr* pickanyExits = nullptr;
    for (unsigned i = 0; i < exitingBlks.size(); ++i) {
      MachineBasicBlock* exitingBlk = exitingBlks[i];
      assert(exitingBlk->succ_size() == 2);
      MachineBasicBlock* succ1 = *exitingBlk->succ_begin();
      MachineBasicBlock* succ2 = *exitingBlk->succ_rbegin();
      MachineBasicBlock* exitBlk = mloop->contains(succ1) ? succ2 : succ1;
      MachineInstr* bi = &*exitingBlk->getFirstTerminator();
      unsigned ec = bi->getOperand(0).getReg();
      if (CDG->getEdgeType(exitingBlk, exitBlk, true) == ControlDependenceNode::FALSE) {
        unsigned notReg = MRI->createVirtualRegister(&CSA::I1RegClass);
        BuildMI(*exitingBlk, exitingBlk->getFirstTerminator(), DebugLoc(), TII->get(CSA::NOT1), notReg).addReg(ec);
        ec = notReg;
      }

      unsigned exitTrue = MRI->createVirtualRegister(&CSA::I1RegClass);
      //generate switch op to guard input to the loop
      const unsigned switchOpcode = TII->getPickSwitchOpcode(&CSA::I1RegClass, false /*not pick op*/);
      MachineInstr *switchInst = BuildMI(*exitingBlk, exitingBlk->getFirstTerminator(), DebugLoc(), TII->get(switchOpcode),
        CSA::IGN).
        addReg(exitTrue, RegState::Define).
        addReg(ec).
        addImm(1);
      switchInst->setFlag(MachineInstr::NonSequential);

      if (anyec == 0) {
        anyec = exitTrue;
      }
      else {
        unsigned valueReg = MRI->createVirtualRegister(&CSA::I1RegClass);
        const unsigned pickanyOpcode = TII->getPickanyOpcode(&CSA::I1RegClass);
        pickanyExits = BuildMI(*latchBB, latchBB->getFirstTerminator(), DebugLoc(), TII->get(pickanyOpcode),
          valueReg).
          addReg(CSA::IGN, RegState::Define).
          addReg(anyec).
          addReg(exitTrue);
        pickanyExits->setFlag(MachineInstr::NonSequential);
        anyec = valueReg;
      }
    }

    if (!pickanyExits) {
      const unsigned moveOpcode = TII->getMoveOpcode(&CSA::I1RegClass);
      MachineInstr *cpyInst = BuildMI(*latchBB, latchBB->getFirstTerminator(), DebugLoc(), TII->get(moveOpcode), 
                                                                                                      predReg).
                                                                                                      addReg(anyec);
      cpyInst->setFlag(MachineInstr::NonSequential);
    } else {
      pickanyExits->substituteRegister(anyec, predReg, 0, *TRI);
    }
  }

  //replace all phi's inside the loop with pickany
  for (MachineLoop::block_iterator BI = mloop->block_begin(), BE = mloop->block_end(); BI != BE; ++BI) {
    MachineBasicBlock* mbb = *BI;
    //only conside blocks in the current loop level, blocks in the nested level are done before.
    if (MLI->getLoopFor(mbb) != mloop) continue;
 
    MachineBasicBlock::iterator iterI = mbb->begin();
    while (iterI != mbb->end()) {
      MachineInstr *iPhi = &*iterI;
      ++iterI;
      if (!iPhi->isPHI()) continue;
      unsigned dst = iPhi->getOperand(0).getReg();
      const TargetRegisterClass *TRC = MRI->getRegClass(dst);
      unsigned pickanyReg = 0;
      MachineInstr *pickanyInst = nullptr;
      for (MIOperands MO(*iPhi); MO.isValid(); ++MO) {
        if (!MO->isReg() || !TargetRegisterInfo::isVirtualRegister(MO->getReg())) continue;
        unsigned Reg = MO->getReg();
        // process uses
        if (MO->isUse()) {
          ++MO;
          MachineBasicBlock *inbb = MO->getMBB();
          if (mloop->getHeader() == mbb && !mloop->contains(inbb)) {
            unsigned switchTrueReg = MRI->createVirtualRegister(TRC);
            //generate switch op to guard input to the loop
            const unsigned switchOpcode = TII->getPickSwitchOpcode(TRC, false /*not pick op*/);
            MachineInstr *switchInst = BuildMI(*mbb, iPhi, DebugLoc(), TII->get(switchOpcode),
              CSA::IGN).
              addReg(switchTrueReg, RegState::Define).
              addReg(predReg).
              addReg(Reg);
            switchInst->setFlag(MachineInstr::NonSequential);
            //replace the phi original operand with the switch guarded one
            Reg = switchTrueReg;
          }
          //pickany to replace phi 
          if (pickanyReg == 0) {
            pickanyReg = Reg;
          } else {
            unsigned valueReg = MRI->createVirtualRegister(TRC);
            const unsigned pickanyOpcode = TII->getPickanyOpcode(TRC);
            pickanyInst = BuildMI(*mbb, iPhi, DebugLoc(), TII->get(pickanyOpcode),
              valueReg).
              addReg(CSA::IGN, RegState::Define).
              addReg(pickanyReg).
              addReg(Reg);
            pickanyInst->setFlag(MachineInstr::NonSequential);
            pickanyReg = valueReg;
          }
        }
      }
      //update last pickany's register to phi's dest, and the index reg used in "ordered" switch
      pickanyInst->substituteRegister(pickanyReg, dst, 0, *TRI);
      iPhi->removeFromParent();
    }
  }
}


void CSACvtCFDFPass::replaceIfFooterPhiSeq() {
  typedef po_iterator<MachineBasicBlock *> po_cfg_iterator;
  MachineBasicBlock *root = &*thisMF->begin();
  for (po_cfg_iterator itermbb = po_cfg_iterator::begin(root), END = po_cfg_iterator::end(root); itermbb != END; ++itermbb) {
    MachineBasicBlock* mbb = *itermbb;
    if (isUnStructured(mbb)) {
      generateDynamicPickTreeForFooter(mbb);
      //TODO:: use repeat to iterate pred inside the loop
    }  else {
      generateCompletePickTreeForPhi(mbb);
    }
  } //end of bb
}



//make sure phi block post dominates all control points of all its inBBs
bool CSACvtCFDFPass::CheckPhiInputBB(MachineBasicBlock* inBB, MachineBasicBlock* mbb) {
  if (DT->dominates(inBB, mbb)) {
    return PDT->dominates(mbb, inBB);
  }
  ControlDependenceNode* inNode = CDG->getNode(inBB);
  unsigned numCtrl = 0;
  for (ControlDependenceNode::node_iterator pnode = inNode->parent_begin(), pend = inNode->parent_end(); pnode != pend; ++pnode) {
    ControlDependenceNode* ctrlNode = *pnode;
    MachineBasicBlock* ctrlBB = ctrlNode->getBlock();
    
    //ignore loop latch, keep looking beyond the loop
    if (MLI->getLoopFor(ctrlBB) && MLI->getLoopFor(ctrlBB)->getLoopLatch() == ctrlBB)
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


void CSACvtCFDFPass::TraceCtrl(MachineBasicBlock* inBB, MachineBasicBlock* mbb, unsigned Reg, unsigned dst, MachineInstr* MI) {
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

MachineInstr* CSACvtCFDFPass::convert_memop_ins(MachineInstr* MI,
                                                unsigned new_opcode,
                                                const CSAInstrInfo& TII,
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
void CSACvtCFDFPass::createMemInRegisterDefs(DenseMap<MachineBasicBlock*, unsigned>& blockToMemIn,
                                             DenseMap<MachineBasicBlock*, unsigned>& blockToMemOut) {
  const CSAInstrInfo &TII = *static_cast<const CSAInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  const unsigned MemTokenMOVOpcode = TII.getMemTokenMOVOpcode();
  
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
              TII.get(MemTokenMOVOpcode),
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
              TII.get(MemTokenMOVOpcode),
              mem_in_reg).addImm(1);
    }

    DEBUG(errs() << "After createMemInRegisterDefs: " << *BB << "\n");
  }
}



unsigned CSACvtCFDFPass::convert_block_memops_linear(MachineFunction::iterator& BB,
                                                     unsigned mem_in_reg)

{
  const CSAInstrInfo &TII = *static_cast<const CSAInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  MachineRegisterInfo *MRI= &thisMF->getRegInfo();

  unsigned current_mem_reg = mem_in_reg;

  MachineBasicBlock::iterator iterMI = BB->begin();
  while (iterMI != BB->end()) {
    MachineInstr* MI = &*iterMI;
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


unsigned CSACvtCFDFPass::merge_dependency_signals(MachineFunction::iterator& BB,
                                                  MachineInstr* MI,
                                                  SmallVector<unsigned, MEMDEP_VEC_WIDTH>* current_wavefront,
                                                  unsigned input_mem_reg) {

  if (current_wavefront->size() > 0) {
    const CSAInstrInfo &TII = *static_cast<const CSAInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
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
                               TII.get(CSA::MERGE1),
                               next_out_reg).addImm(0).addReg((*current_level)[i]).addReg((*current_level)[i+1]);
          }
          else {
            // Adding a merge at the end of the block.
            new_inst = BuildMI(*BB,
                               BB->getFirstTerminator(),
                               DebugLoc(),
                               TII.get(CSA::MERGE1),
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



unsigned CSACvtCFDFPass::convert_block_memops_wavefront(MachineFunction::iterator& BB,
                                                        unsigned mem_in_reg)
{
  const CSAInstrInfo &TII = *static_cast<const CSAInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();

  unsigned current_mem_reg = mem_in_reg;
  SmallVector<unsigned, MEMDEP_VEC_WIDTH> current_wavefront;
  current_wavefront.clear();
  DEBUG(errs() << "Wavefront memory ordering for block " << &*BB << "\n");

  MachineBasicBlock::iterator iterMI = BB->begin();
  while (iterMI != BB->end()) {
    MachineInstr* MI = &*iterMI;
    DEBUG(errs() << "Found instruction: " << *MI << "\n");

    unsigned current_opcode = MI->getOpcode();
    unsigned converted_opcode = TII.get_ordered_opcode_for_LDST(current_opcode);

    bool is_load = TII.isLoad(MI);

    if (current_opcode != converted_opcode) {
      // Create a register for the "issued" output of this memory
      // operation.
      unsigned next_out_reg = MRI->createVirtualRegister(MemopRC);

      if (is_load) {
        // Just a load. Build up the set of load outputs that we
        // depend on.
        current_wavefront.push_back(next_out_reg);
      }
      else {
        // This is a store or atomic instruction.
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

      convert_memop_ins(MI,
                        converted_opcode,
                        TII,
                        next_out_reg,
                        current_mem_reg);

      if (!is_load) {
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

/* Find all implicitly defined vregs. These are problematic with dataflow
 * conversion: LLVM will automatically expand them to registers (LICs, in our
 * case). While registers can be read without any value previously having been
 * written, LICs are different. We must replace the undef with a read from
 * %IGN, equivalent to reading 0. Note that we can do this even if we're not
 * sure that the instructions in question will be successfully converted to
 * data flow. Returns a boolean indicating modification.
 */
bool CSACvtCFDFPass::replaceUndefWithIgn() {
  bool modified = false;
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  const CSAInstrInfo &TII = *static_cast<const CSAInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  SmallPtrSet<MachineInstr*, 4> implicitDefs;
  DEBUG(errs() << "Finding implicit defs:\n");
  for (MachineFunction::iterator BB = thisMF->begin(); BB != thisMF->end(); ++BB) {
    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      MachineInstr *MI = &*I;
      // We're looking for instructions like '%vreg26<def> = IMPLICIT_DEF;'.
      if(MI->isImplicitDef()) {
        implicitDefs.insert(MI);
        DEBUG(errs() << "\tFound: " << *MI);
      }
    }
  }

  if(implicitDefs.empty()) {
    DEBUG(errs() << "(No implicit defs found.)\n");
  }

  for (SmallPtrSet<MachineInstr*, 4>::iterator I = implicitDefs.begin(), E = implicitDefs.end(); I != E; ++I) {
    MachineInstr *uMI = *I;
    MachineOperand uMO = uMI->getOperand(0);
    // Ensure we're dealing with a register definition.
    assert(uMO.isDef() && uMO.isReg());
    // Ensure SSA form and that we have right defining instruction.
    assert(MRI->getUniqueVRegDef(uMO.getReg()) &&
        MRI->getUniqueVRegDef(uMO.getReg()) == uMI);
    const TargetRegisterClass *TRC = MRI->getRegClass(uMI->getOperand(0).getReg());
    const unsigned moveOpcode = TII.getMoveOpcode(TRC);
    BuildMI(*uMI->getParent(), uMI, DebugLoc(),
      TII.get(moveOpcode),
      uMI->getOperand(0).getReg()).addImm(0);
    // Erase the implicit definition.
    uMI->removeFromParent();
    modified = true;
  }

  DEBUG(errs() << "Finished converting implicit defs to %IGN reads.\n\n");
  return modified;
}

void CSACvtCFDFPass::addMemoryOrderingConstraints() {

  const CSAInstrInfo &TII = *static_cast<const CSAInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
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
    switch (OrderMemopsType) {
    case OrderMemopsMode::wavefront:
      {
        last_mem_reg = convert_block_memops_wavefront(BB,
                                                      mem_in_reg);
      }
      break;
    case OrderMemopsMode::linear:
      {
        last_mem_reg = convert_block_memops_linear(BB,
                                                   mem_in_reg);
      }
      break;

      // We should never get here.
    case OrderMemopsMode::none:
    default:
      assert(0 && "Only linear and wavefront memory ordering implemented now.");

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
    const unsigned MemTokenMOVOpcode = TII.getMemTokenMOVOpcode();    
    MachineInstr* mem_out_def = BuildMI(*BB,
                                        BB->getFirstTerminator(),
                                        DebugLoc(),
                                        TII.get(MemTokenMOVOpcode),
                                        mem_out_reg).addReg(last_mem_reg);

    DEBUG(errs() << "Inserted mem_out_def instruction " << *mem_out_def << "\n");

    // Save mem_in_reg and mem_out_reg for each block into a DenseMap,
    // so that we can create a PHI instruction as an input to the
    // block.
    blockToMemIn[&*BB] = mem_in_reg;
    blockToMemOut[&*BB] = mem_out_reg;

    DEBUG(errs() << "After memop conversion of function: " << *BB << "\n");
  }

  // Another walk over basic blocks: add in definitions for mem_in
  // register for each block, based on predecessors.
  createMemInRegisterDefs(blockToMemIn, blockToMemOut);
}

