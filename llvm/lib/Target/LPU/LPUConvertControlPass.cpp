//===-- LPUConvertControlPass.cpp - LPU control flow conversion -----------===//
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
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;
namespace llvm { extern raw_ostream *CreateInfoOutputFile(); }

static cl::opt<int>
ConvertControlPass("lpu-cvt-ctrl-pass", cl::Hidden,
		   cl::desc("LPU Specific: Convert control flow pass"),
		   cl::init(100));

#define DEBUG_TYPE "lpu-convert-control"

namespace {
class LPUConvertControlPass : public MachineFunctionPass {
public:
  static char ID;
  LPUConvertControlPass() : MachineFunctionPass(ID) { thisMF = nullptr;}

  const char* getPassName() const override {
    return "LPU Convert Control Flow";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfo>();
    //AU.addRequired<LiveVariables>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  MachineFunction *thisMF;

  bool genDFInstructions(MachineInstr *MI, MachineBasicBlock *BB, 
                         MachineBasicBlock::iterator lastPhiInst, 
                         const MachineBasicBlock *backedgeBB);

  bool analyzePhiOperands(MachineInstr *MI, MachineBasicBlock *BB, 
                          unsigned int *srcReg1, unsigned int *predReg1,
                          unsigned int *predReg2, MachineBasicBlock **predBB1,
                          MachineBasicBlock **predBB2, 
                          const MachineBasicBlock *backedgeBB); 

};
}

MachineFunctionPass *llvm::createLPUConvertControlPass() {
  return new LPUConvertControlPass();
}

char LPUConvertControlPass::ID = 0;


bool 
LPUConvertControlPass::analyzePhiOperands(MachineInstr *MI,
                                          MachineBasicBlock *BB,
                                          unsigned int *srcReg1, 
                                          unsigned int *predReg1, 
                                          unsigned int *predReg2,
                                          MachineBasicBlock **predBB1,
                                          MachineBasicBlock **predBB2, 
                                          const MachineBasicBlock *backedgeBB) {

  const TargetInstrInfo &TII = *thisMF->getSubtarget().getInstrInfo();
  
  bool firstPred = true;
  for (unsigned i = 1, e = MI->getNumOperands(); i != e; i +=2 ) {
    // for each src operand get BB label & corresponding predicate reg
    MachineOperand &currOperand = MI->getOperand(i);
    if (currOperand.readsReg()) { // src operand
      MachineBasicBlock &predBB = *MI->getOperand(i+1).getMBB();
      SmallVector<MachineOperand, 4> predCond;
      MachineBasicBlock *predTBB = nullptr, *predFBB = nullptr;
      if (TII.AnalyzeBranch(predBB, predTBB, predFBB, predCond, true)) {
        DEBUG(errs() << "Phi predBB branch not analyzable \n");
        return false;
      }

      if (predCond.empty()) {
        DEBUG(errs() << "Phi predCond is empty \n");
        return false;	
      }
		
      // is the loop back branch predicate set to true?
      if (&predBB == backedgeBB) {
        if (predTBB != BB) {
          DEBUG(errs() << "Loop back branch predicate is set to false \n");
          return false;	
        }
      }

      // is the loop entry fall through predicate set to false?
      if (&predBB != backedgeBB) {
        if (predFBB && predFBB != BB) {
          DEBUG(errs() << "Loop entry fall through predicate is set to true \n");
          return false;	
        }
      }

      if (firstPred) {
        *predReg1 = predCond[1].getReg();
        *srcReg1 = currOperand.getReg();
        *predBB1 = &predBB;
        firstPred = false;
      }
      else {
        assert(*predReg1 != 0);
        if (*predReg2 != 0) {
          DEBUG(errs() << "Phi has more than 2 srcs \n");
	  return false;	
	}
	*predReg2 = predCond[1].getReg();
        *predBB2 = &predBB;
      } // second predicate
    } // src operand of Phi
  } // process each operand of Phi
  return true;
}

// generate PICKs or SWITCHes when possible
bool 
LPUConvertControlPass::genDFInstructions(MachineInstr *MI, 
                                         MachineBasicBlock *BB, 
                                         MachineBasicBlock::iterator 
                                           lastPhiInst, 
                                         const MachineBasicBlock *backedgeBB) {

  const TargetMachine &TM = thisMF->getTarget();
  const LPUInstrInfo &TII = *static_cast<const LPUInstrInfo*>
                            (thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *TM.getSubtargetImpl()->getRegisterInfo();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  bool genDFInst = false;

  if (MI->isPHI()) { // process Phi
    // process each live-in use also defined within the loop 
    unsigned predReg1 =0, predReg2 = 0, predReg3 = 0;
    unsigned srcReg1;
    MachineBasicBlock *predBB1;
    MachineBasicBlock *predBB2;


    if (!analyzePhiOperands(MI, BB, &srcReg1, &predReg1, &predReg2, &predBB1, 
                            &predBB2, backedgeBB)) {
      DEBUG(errs() << "Phi operands analysis returned false\n");
      return false;
    }

    // create a new Phi for predReg2 & predReg1
    predReg3 = MRI->createVirtualRegister(MRI->getRegClass(predReg2));

    // generate a Pick inst immediately after the Phi
    unsigned dst = MI->getOperand(0).getReg();
    const TargetRegisterClass *TRC = MRI->getRegClass(dst);
    unsigned newDst = MRI->createVirtualRegister(TRC);
    const unsigned pickOpcode = TII.getPickSwitchOpcode(TRC, true /*pick op*/);
    MachineInstr *pickInst = BuildMI(*BB, MI, MI->getDebugLoc(), 
                                     TII.get(pickOpcode), dst).addReg(predReg3).
                                     addReg(srcReg1).addReg(newDst);
	 
    //add a new Phi for the PICK inst src predicate
    BuildMI(*BB, MI, MI->getDebugLoc(), TII.get(TargetOpcode::PHI),predReg3).
            addReg(predReg1).addMBB(predBB1).addReg(predReg2).addMBB(predBB2);

    pickInst->removeFromParent();
    BB->insertAfter(lastPhiInst,pickInst);

    // replace all uses of dst with newDst
    MI->substituteRegister(dst, newDst, 0, TRI);

    genDFInst = true;
	 
  } // end process phi inst & PICK generation
  else { // generate SWITCHes for each live-out def used in loop
    for (const MachineOperand &currDef : MI->defs()) {

      if (!currDef.isReg()) continue;

      unsigned defReg = currDef.getReg();
      bool liveOut = false;
      MachineInstr *usePhi = nullptr;
      for (MachineInstr &useMI : MRI->use_instructions(defReg)) { 
        //for each use of this def
        // is there any use outside this BB?
        if (useMI.getParent() != BB) { 
          liveOut = true;
	  continue;
        }
        // is there any use in a Phi inside this BB?
        if (useMI.isPHI()) {
          usePhi = &useMI;
        } 
      } // process each use 

      if (liveOut && usePhi) { // generate a SWITCH
        const TargetRegisterClass *TRC = MRI->getRegClass(defReg);
        unsigned newLoopBackReg = MRI->createVirtualRegister(TRC);
        unsigned newLiveOutReg = MRI->createVirtualRegister(TRC);

        SmallVector<MachineOperand, 4> brCond;
        MachineBasicBlock *currTBB = nullptr, *currFBB = nullptr;
        if (TII.AnalyzeBranch(*BB, currTBB, currFBB, brCond, true)) {
          DEBUG(errs() << "BB branch not analyzable \n");
          return false;
        }

        if (brCond.empty()) {
          DEBUG(errs() << "brCond is empty \n");
          return false;	
        }

        MachineBasicBlock::iterator loopBranch = BB->getFirstTerminator(); 

        // generate switch op
        const unsigned switchOpcode = TII.getPickSwitchOpcode(TRC, false 
                                                              /*not pick op*/);
        MachineInstr *switchInst = BuildMI(*BB, BB->end(), 
                                           BB->end()->getDebugLoc(), 
                                           TII.get(switchOpcode), 
                                           newLiveOutReg).addReg(newLoopBackReg,
                                           RegState::Define).
                                           addReg(brCond[1].getReg()).
                                           addReg(defReg);

        switchInst->removeFromParent();
        BB->insert(loopBranch,switchInst);

        // update SSA
        usePhi->substituteRegister(defReg, newLoopBackReg, 0, TRI);
        for (MachineInstr &useMI : MRI->use_instructions(defReg)) { 
          //for each use of this def
	  // replace uses of defReg outside this BB with switch def
	  if (useMI.getParent() != BB) { 
	    useMI.substituteRegister(defReg, newLiveOutReg, 0, TRI);
   	  }
        } // process each use 

      genDFInst =  true;
      } // generate switch

    } // process each def in instruction          

  } // end process non-phi instructions & SWITCH generation
  return genDFInst;
}

bool LPUConvertControlPass::runOnMachineFunction(MachineFunction &MF) {

  if (ConvertControlPass == 0) return false;

  thisMF = &MF;
  const TargetMachine &TM = MF.getTarget();
  
  raw_ostream *OS = CreateInfoOutputFile();
  bool Modified = false;

#ifndef NDEBUG
  DEBUG(errs() << "\ngraph before ConvertControlPass\n");

  for (MachineFunction::iterator BB = MF.begin(), E = MF.end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      MachineInstr *MI = I;
      DEBUG(MI->print(*OS, &TM));  
    }
  }
#endif

  // for now only well formed innermost loops are processed in this pass
  MachineLoopInfo *MLI = &getAnalysis<MachineLoopInfo>();

  if (!MLI) {
    DEBUG(errs() << "no loop info.\n");
    return false;
  }

  // Walk through the CFG and generate dataflow ops like picks & switches

  // first collect the loops to be in cfg (top-down) order
  std::vector<MachineLoop *> cfgOrderLoops;
  for (MachineLoopInfo::iterator LI = MLI->begin(), LE = MLI->end(); 
       LI != LE; ++LI) { // for all top-level loops in MF
    
    MachineLoop *ML = *LI;
    // TODO: add nested loops
    cfgOrderLoops.push_back(ML);
  }

  // now walk the loops in cfg order
  int loopProcessedCnt = 0;
  for (std::vector<MachineLoop *>::reverse_iterator lpIter = 
       cfgOrderLoops.rbegin(), lpIterEnd = cfgOrderLoops.rend(); 
       lpIter != lpIterEnd; ++lpIter) { // for all loops in MF
    const MachineLoop *currLoop = *lpIter;

    
    DEBUG(errs() << "begin loop# - \n");

    // ignore outer loops
    const std::vector<MachineLoop*> &SubLoops = currLoop->getSubLoops();
    if (SubLoops.size() != 0) {
      DEBUG(errs() << "ignoring outer loop\n");
      continue;
    }
    // ignore loops with multiple blocks
    if (currLoop->getNumBlocks() > 1) {
      DEBUG(errs() << "ignoring loop with multiple blocks\n");
      continue;
    }
    // ignore loops with multiple backedges
    if (currLoop->getNumBackEdges() != 1) {
      DEBUG(errs() << "ignoring loop with multiple backedges\n");
      continue;
    }

    const MachineBasicBlock *backedgeBB = currLoop->getLoopLatch();
    assert(backedgeBB && "backedge BB is null!");

    // TODO: ignore loops with no live-in uses & no live-out defs - 
    // check this early & quickly
    bool loopModified = false;
    for (MachineLoop::block_iterator BI = currLoop->block_begin(), 
         E = currLoop->block_end(); BI != E; ++BI) { 
      // for each block in loop body - should be only 1 for now
      MachineBasicBlock *BB = *BI;

      DEBUG(errs() << "begin BB# - " << BB->getNumber() << "\n");

      // ignore empty blocks
      if (BB->empty()) {
        continue;
      }

      // TODO: ignore BB/pred/succ with un-analyzeable brs  - 
      // check this early & quickly
      MachineBasicBlock::iterator lastPhiInst =
        std::prev(BB->SkipPHIsAndLabels(BB->begin())); 

      // process each instruction in BB
      for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
        MachineInstr *MI = I;

	DEBUG(errs() << "processing inst: ");  
	DEBUG(MI->print(*OS, &TM));  

 	// TODO: do we need to process all insts? 
        // what about picks/switches/cmps/brs?

	if (genDFInstructions(MI, BB, lastPhiInst, backedgeBB)) {
           DEBUG(errs() << "modified graph - gen DF insts\n");
	   Modified = true;
	   loopModified = true;
	}

      } // for each instruction in BB
      DEBUG(errs() << "end BB# - " <<  BB->getNumber() << "\n");
    } // for each block in loop 
    DEBUG(errs() << "end loop - " << "\n");

#ifndef NDEBUG
    assert(ConvertControlPass > 0 && "ConvertControlPass has invalid value!");
    if (loopModified) loopProcessedCnt++;
    if (loopProcessedCnt == ConvertControlPass) return Modified;    
#endif
  } // end for all loops in MF 


#ifndef NDEBUG
  DEBUG(errs() << "\ngraph after ConvertControlPass\n");

  for (MachineFunction::iterator BB = MF.begin(), E = MF.end(); BB != E; ++BB) {
    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      MachineInstr *MI = I;
      DEBUG(MI->print(*OS, &TM));  
    }
  }
#endif

  return Modified;

}
