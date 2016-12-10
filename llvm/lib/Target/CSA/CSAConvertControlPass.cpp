//===-- CSAConvertControlPass.cpp - CSA control flow conversion -----------===//
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

#include <map>
#include "CSA.h"
#include "InstPrinter/CSAInstPrinter.h"
#include "CSAInstrInfo.h"
#include "CSATargetMachine.h"
#include "CSAIfConversion.h"
#include "CSALicAllocation.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;

static cl::opt<int>
ConvertControlPass("csa-cvt-ctrl-pass", cl::Hidden,
		   cl::desc("CSA Specific: Convert control flow pass"),
		   cl::init(0));

static cl::opt<int>
IfConversionTokens("csa-if-conversion", cl::Hidden,
		   cl::desc("CSA Specific: if Conversion within a loop"),
		   cl::init(0));


// Flag to enable step that replaces registers with LICs.
static cl::opt<int>
LICAllocationStep("csa-lic-alloc", cl::Hidden,
                  cl::desc("CSA Specific: Enable LIC replacement of registers"),
                  cl::init(CSALicAllocation::LicAllocDisabled));


#define DEBUG_TYPE "csa-convert-control"

STATISTIC(NumSimple,       "Number of simple if-conversions performed");
STATISTIC(NumSimpleFalse,  "Number of simple (F) if-conversions performed");
STATISTIC(NumTriangle,     "Number of triangle if-conversions performed");
STATISTIC(NumTriangleRev,  "Number of triangle (R) if-conversions performed");
STATISTIC(NumTriangleFalse,"Number of triangle (F) if-conversions performed");
STATISTIC(NumTriangleFRev, "Number of triangle (F/R) if-conversions performed");
STATISTIC(NumDiamonds,     "Number of diamond if-conversions performed");
//STATISTIC(NumIfConvBBs,    "Number of if-converted blocks");
//STATISTIC(NumDupBBs,       "Number of duplicated blocks");
//STATISTIC(NumUnpred,       "Number of true blocks of diamonds unpredicated");

typedef CSAIfConversion::IfcvtToken IfcvtToken;
typedef CSAIfConversion::IfcvtKind IfcvtKind;
typedef CSAIfConversion::BBInfo BBInfo;
#define ICSimpleFalse CSAIfConversion::ICSimpleFalse
#define ICSimple CSAIfConversion::ICSimple
#define ICTriangleFRev CSAIfConversion::ICTriangleFRev
#define ICTriangleRev CSAIfConversion::ICTriangleRev
#define ICTriangleFalse CSAIfConversion::ICTriangleFalse
#define ICTriangle CSAIfConversion::ICTriangle
#define ICDiamond CSAIfConversion::ICDiamond

namespace {
class CSAConvertControlPass : public MachineFunctionPass {
public:
  static char ID;
  CSAConvertControlPass() : MachineFunctionPass(ID) { thisMF = nullptr;}

  StringRef getPassName() const override {
    return "CSA Convert Control Flow";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfo>();
    //AU.addRequired<LiveVariables>();
    AU.addRequired<MachineDominatorTree>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  MachineFunction *thisMF;
  MachineBasicBlock *loopBackedgeBB;
  MachineBasicBlock *loopPreheader;

  MachineDominatorTree *DT;

  CSAIfConversion *myIfConverter;
  CSALicAllocation *myLicAllocater;

  void addLoop(std::vector<MachineLoop *> &loopList, MachineLoop *ML);

  void prepareIfConverter() {
    myIfConverter = new CSAIfConversion();
    thisMF->RenumberBlocks();
    myIfConverter->BBAnalysis.resize(thisMF->getNumBlockIDs());
    myIfConverter->TII = thisMF->getSubtarget().getInstrInfo();
    const TargetSubtargetInfo &ST = thisMF->getSubtarget();
    myIfConverter->SchedModel.init(ST.getSchedModel(),&ST,myIfConverter->TII);
  }

  bool candidateForIfConversion(MachineLoop *loop, std::vector<IfcvtToken*> &Tokens);

  bool candidateLoopForDF(MachineLoop *currLoop);

  bool processLoopRegion(MachineLoop *currLoop);

  // generate picks/switches for loop CF
  bool genDFInstructions(MachineInstr *MI, MachineBasicBlock *BB,
                         MachineBasicBlock::iterator LastPhiInst);

  bool analyzePhiOperands(MachineInstr *MI, MachineBasicBlock *BB,
                          unsigned int *srcReg1, unsigned int *predReg1,
                          unsigned int *predReg2, MachineBasicBlock **predBB1,
                          MachineBasicBlock **predBB2);

  bool processIfConversionRegion(std::vector<IfcvtToken*> &Tokens);


  bool processIfConversionToken(IfcvtToken *Token);

  // generate picks/switches for straightline CF
  bool genDFInstructions(MachineInstr *MI, IfcvtToken *Token,
                         MachineBasicBlock::iterator LastPhiInst,
			 std::set<unsigned> UseRegsSet);


  // analyze each live range in loop and allocate LICs as appropriate
  bool processLiveRangesInLoop(MachineBasicBlock* BB);

};
}

MachineFunctionPass *llvm::createCSAConvertControlPass() {
  return new CSAConvertControlPass();
}

char CSAConvertControlPass::ID = 0;

bool
CSAConvertControlPass::
candidateForIfConversion(MachineLoop *loop, std::vector<IfcvtToken*> &Tokens) {

  // assumption is that this is a single entry loop

  assert(Tokens.empty() && "IfCvt tokens not empty!");

  // TODO: iterate until no change to handle nesting
  for (MachineLoop::block_iterator BI = loop->block_begin(),
       E = loop->block_end(); BI != E; ++BI) {
    // starting with each block in loop body analyze it
    MachineBasicBlock *BB = *BI;

    myIfConverter->AnalyzeBlock(BB, Tokens);
  }

  if (!Tokens.empty()) {
    // this sorting is inherited from the llvm ifConverter and not understood
    std::stable_sort(Tokens.begin(), Tokens.end(),
                     CSAIfConversion::IfcvtTokenCmp);
    return true;
  }

  return false;
}

bool CSAConvertControlPass::candidateLoopForDF(MachineLoop *currLoop) {

    // ignore outer loops
    const std::vector<MachineLoop*> &SubLoops = currLoop->getSubLoops();
    if (SubLoops.size() != 0) {
      DEBUG(errs() << "\nignoring outer loop\n");
      return false;
    }
    // ignore loops with multiple blocks
    if (currLoop->getNumBlocks() > 1) {
      std::vector<IfcvtToken*> Tokens;

      if (!candidateForIfConversion(currLoop,Tokens)) {
        DEBUG(errs() << "\nloop is not candidate for if-conversion\n");
        DEBUG(errs() << "\nignoring loop with multiple blocks\n");
        return false;
      }

      DEBUG(errs() << "\nloop is candidate for if-conversion\n");

      if (!processIfConversionRegion(Tokens)) {
        DEBUG(errs() << "ifcvt failed: ignoring loop with multiple blocks\n");
        return false;
      }

      if (currLoop->getNumBlocks() > 1) {
        DEBUG(errs() << "\nignoring loop with multiple blocks\n");
        return false;
      }
    }
    // ignore loops with multiple backedges
    if (currLoop->getNumBackEdges() != 1) {
      DEBUG(errs() << "\nignoring loop with multiple backedges\n");
      return false;
    }
    // TODO: ignore loops with no live-in uses & no live-out defs -
    // check this early & quickly

    return true;

}

bool
CSAConvertControlPass::analyzePhiOperands(MachineInstr *MI,
                                          MachineBasicBlock *BB,
                                          unsigned int *srcReg1,
                                          unsigned int *predReg1,
                                          unsigned int *predReg2,
                                          MachineBasicBlock **predBB1,
                                          MachineBasicBlock **predBB2) {

  const TargetInstrInfo &TII = *thisMF->getSubtarget().getInstrInfo();

  bool firstPred = true;
  for (unsigned i = 1, e = MI->getNumOperands(); i != e; i +=2 ) {
    // for each src operand get BB label & corresponding predicate reg
    MachineOperand &currOperand = MI->getOperand(i);
    if (currOperand.readsReg()) { // src operand
      MachineBasicBlock &predBB = *MI->getOperand(i+1).getMBB();

      MachineBasicBlock *branchBB = &predBB;

      if (&predBB == loopPreheader) { // preheader has no terminating branch
	if (predBB.pred_size() != 1) {
          DEBUG(errs() << "loopPreHeader doesn't have a single predecessor \n");
	  return false;
 	}
	MachineBasicBlock::pred_iterator PI = predBB.pred_begin();
        branchBB = *PI;
	if (!branchBB) {
          DEBUG(errs() << "loopPreHeader predecessor is null\n");
	  return false;
	}
      }

      SmallVector<MachineOperand, 4> predCond;
      MachineBasicBlock *predTBB = nullptr, *predFBB = nullptr;
      if (TII.analyzeBranch(*branchBB, predTBB, predFBB, predCond, true)) {
        DEBUG(errs() << "Phi predBB branch not analyzable \n");
        return false;
      }

      if (predCond.empty() || predCond.size() != 2) {
        DEBUG(errs() << "Phi predCond is empty \n");
        return false;
      }

      CSA::CondCode branchCC = (CSA::CondCode)predCond[0].getImm();

      // is the loop back branch predicate set to true?
      if (&predBB == loopBackedgeBB) {
        if (predTBB != BB) {
          DEBUG(errs() << "Loop back branch predicate is set to false \n");
          return false;
        }
        if (branchCC != CSA::COND_T) {
          DEBUG(errs() << "Loop back branch predicate is set to false \n");
          return false;
        }
      }

      // is the loop entry fall through predicate set to false?
      if (&predBB != loopBackedgeBB) {
        if (predFBB && (predFBB != BB || predFBB != loopPreheader)) {
          DEBUG(errs() << "Loop entry fallthrough predicate is set to true \n");
          return false;
        }
        if (!predFBB && (branchCC != CSA::COND_T)) {
          DEBUG(errs() << "Loop entry fallthrough predicate is set to true \n");
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
CSAConvertControlPass::genDFInstructions(MachineInstr *MI,
                                         MachineBasicBlock *BB,
                                         MachineBasicBlock::iterator
                                           LastPhiInst) {

  const CSAInstrInfo &TII = *static_cast<const CSAInstrInfo*>
                            (thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *thisMF->getSubtarget().getRegisterInfo();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  bool genDFInst = false;

  if (MI->isPHI()) { // process Phi
    // process each live-in use also defined within the loop
    unsigned predReg1 =0, predReg2 = 0, predReg3 = 0;
    unsigned srcReg1;
    MachineBasicBlock *predBB1;
    MachineBasicBlock *predBB2;


    if (!analyzePhiOperands(MI, BB, &srcReg1, &predReg1, &predReg2, &predBB1,
                            &predBB2)) {
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
    BB->insertAfter(LastPhiInst,pickInst);

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
        if (TII.analyzeBranch(*BB, currTBB, currFBB, brCond, true)) {
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
                                           DebugLoc(),
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

bool
CSAConvertControlPass::genDFInstructions(MachineInstr *MI, IfcvtToken *Token,
                                         MachineBasicBlock::iterator
						LastPhiInst,
					 std::set<unsigned> UseRegsSet) {

  IfcvtKind Kind = Token->Kind;

  assert(((Kind == ICTriangle) || (Kind == ICTriangleRev) ||
          (Kind == ICTriangleFalse) || (Kind == ICTriangleFRev)) &&
          "CSAIfConversion: genDFInstructions - unexpected Token Kind");

  const CSAInstrInfo &TII = *static_cast<const CSAInstrInfo*>
                            (thisMF->getSubtarget().getInstrInfo());
  const TargetRegisterInfo &TRI = *thisMF->getSubtarget().getRegisterInfo();
  MachineRegisterInfo *MRI = &thisMF->getRegInfo();
  bool genDFInst = false;

  BBInfo &BBI = Token->BBI;
  MachineBasicBlock *domBB = BBI.BB;
  BBInfo &TrueBBI = myIfConverter->BBAnalysis[BBI.TrueBB->getNumber()];
  BBInfo &FalseBBI = myIfConverter->BBAnalysis[BBI.FalseBB->getNumber()];
  MachineBasicBlock *currBB = TrueBBI.BB;
  MachineBasicBlock *postDomBB = FalseBBI.BB;


  if (Kind == ICTriangleFalse || Kind == ICTriangleFRev) {
    std::swap(currBB, postDomBB);
  }

  assert((currBB == MI->getParent()) && "CSAIfConversion: BB insanity");

  SmallVector<MachineOperand, 4> brCond;
  MachineBasicBlock *currTBB = nullptr, *currFBB = nullptr;
  if (TII.analyzeBranch(*domBB, currTBB, currFBB, brCond, true)) {
    DEBUG(errs() << "CSAIfConversion: domBB branch not analyzable \n");
    return false;
  }

  if (brCond.empty()) {
    DEBUG(errs() << "CSAIfConversion: brCond is empty \n");
    return false;
  }

  CSA::CondCode branchCC = (CSA::CondCode)brCond[0].getImm();
  bool swapPickSwitchRegs = false;
  if ((Kind == ICTriangle || Kind == ICTriangleRev)
       && (branchCC == CSA::COND_T)) {
    swapPickSwitchRegs = true;
  }

  if ((Kind == ICTriangleFalse || Kind == ICTriangleFRev)
       && (branchCC == CSA::COND_F)) {
    swapPickSwitchRegs = true;
  }

  for (MIOperands MO(*MI); MO.isValid(); ++MO) {
    if (!MO->isReg()) continue;
    unsigned Reg = MO->getReg();

    // process uses
    if (MO->isUse()) {
      MachineInstr *DefMI = MRI->getVRegDef(Reg);
      if (DefMI && (DefMI->getParent() != currBB)) { // live into MI BB

	if (UseRegsSet.count(Reg) != 0) { // skip uses already processed
          continue;
 	}
        else {
	  UseRegsSet.insert(Reg);
	}

        DEBUG(errs() << "CSAIfConversion: found live in Reg" <<
                     PrintReg(Reg) << "\n");

        // generate and insert SWITCH in dominating block
        const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
        unsigned predFalseReg = MRI->createVirtualRegister(TRC);
        unsigned predTrueReg = MRI->createVirtualRegister(TRC);

  	if (swapPickSwitchRegs) {
	  // swap the SWITCH dst regs since the branch condition is inverted
	  unsigned tmp = predFalseReg;
	  predFalseReg = predTrueReg;
	  predTrueReg = tmp;
	}

        MachineBasicBlock::iterator domBranch = domBB->getFirstTerminator();

        // generate switch op
        const unsigned switchOpcode = TII.getPickSwitchOpcode(TRC, false
                                                              /*not pick op*/);
        MachineInstr *switchInst = BuildMI(*domBB, domBB->end(),
                                           DebugLoc(),
                                           TII.get(switchOpcode),
                                           predFalseReg).addReg(predTrueReg,
                                           RegState::Define).
                                           addReg(brCond[1].getReg()).
                                           addReg(Reg);

        switchInst->removeFromParent();
        domBB->insert(domBranch,switchInst);

  	if (swapPickSwitchRegs) {
	  // swap the regs back for uses within currBB & postDomBB as
	  // the BBs were already swapped earlier
	  unsigned tmp = predFalseReg;
	  predFalseReg = predTrueReg;
	  predTrueReg = tmp;
	}

        // update SSA
        MI->substituteRegister(Reg, predFalseReg, 0, TRI);
        for (MachineInstr &useMI : MRI->use_instructions(Reg)) {

          if (useMI.getParent() == domBB) // ignore uses in domBB
	     continue;

          // for each use of this def
	  // replace uses of Reg in currBB with predFalseReg and
          // other BBs with predTrueReg
	  if (useMI.getParent() == currBB) {
	    useMI.substituteRegister(Reg, predFalseReg, 0, TRI);
          } else if (useMI.getParent() == postDomBB) {
	    useMI.substituteRegister(Reg, predTrueReg, 0, TRI);
   	  }

        } // process each use

        genDFInst =  true;

      }
    }

    // process defs
    if (MO->isDef()) {
      for (MachineInstr &useMI : MRI->use_instructions(Reg)) {
        //for each use of this def
        // is there any use outside this BB i.e. live out of conversion block?
        if (useMI.getParent() != currBB) {
	  assert(useMI.isPHI() &&
                 "use of live out conditional def. is not in a Phi");

          DEBUG(errs() << "CSAIfConversion: found live out Reg#  "
                << PrintReg(Reg) << "\n");

          // generate and insert PICK in post dominating block
          unsigned dst = useMI.getOperand(0).getReg(); // get dst reg of Phi

          const TargetRegisterClass *TRC = MRI->getRegClass(dst);
          unsigned newDst = MRI->createVirtualRegister(TRC);// new dst for Phi

          unsigned srcReg1 = useMI.getOperand(1).getReg();
	  if (useMI.getOperand(2).getMBB() == currBB) {
	    assert(useMI.getOperand(4).getMBB() == domBB &&
		   "genDFInstructions: cannot generate PICK for live out reg");
            srcReg1 = useMI.getOperand(3).getReg();
 	  }
	  else {
	    assert(useMI.getOperand(4).getMBB() == currBB &&
		   "genDFInstructions: cannot generate PICK for live out reg");
	    assert(useMI.getOperand(2).getMBB() == domBB &&
		   "genDFInstructions: cannot generate PICK for live out reg");
	  }

  	  if (swapPickSwitchRegs) {
	    // swap the PICK src regs since the branch condition is inverted
	    unsigned tmp = srcReg1;
	    srcReg1 = newDst;
	    newDst = tmp;
	  }

          const unsigned pickOpcode =
        	TII.getPickSwitchOpcode(TRC, true /*pick op*/);
          MachineInstr *pickInst = BuildMI(*postDomBB, &useMI,
                                           useMI.getDebugLoc(),
                                           TII.get(pickOpcode), dst).
                                           addReg(brCond[1].getReg()).
                                           addReg(newDst).
                                           addReg(srcReg1);

    	  pickInst->removeFromParent();
          postDomBB->insertAfter(LastPhiInst,pickInst);

  	  if (swapPickSwitchRegs) {
	    // swap the regs back for the SSA (Phi) update
	    unsigned tmp = srcReg1;
	    srcReg1 = newDst;
	    newDst = tmp;
	  }

          // update SSA
    	  // replace dst reg in Phi with newDst
    	  useMI.substituteRegister(dst, newDst, 0, TRI);

    	  genDFInst = true;

        } // end if live out use
      } // end for each use instruction of Reg
    } // end if this operand is a def

  }// end for each operand in this instruction

  return genDFInst;
}

bool CSAConvertControlPass::processIfConversionToken(IfcvtToken *Token) {

  bool genDFInst = false;

  IfcvtKind Kind = Token->Kind;
  switch(Kind) {
      case ICTriangle:
      case ICTriangleRev:
      case ICTriangleFalse:
      case ICTriangleFRev: break;
      default: return genDFInst;
  }

  BBInfo &BBI = Token->BBI;
  BBInfo &TrueBBI = myIfConverter->BBAnalysis[BBI.TrueBB->getNumber()];
  BBInfo &FalseBBI = myIfConverter->BBAnalysis[BBI.FalseBB->getNumber()];
  BBInfo *CvtBBI = &TrueBBI;
  BBInfo *NextBBI = &FalseBBI;

  if (Kind == ICTriangleFalse || Kind == ICTriangleFRev)
    std::swap(CvtBBI, NextBBI);

  MachineBasicBlock *BB = CvtBBI->BB;

  DEBUG(errs() << "\nbegin BB# - " << BB->getNumber() << "\n");

  // ignore empty blocks
  if (BB->empty()) {
    return genDFInst;
  }

  MachineBasicBlock::iterator LastPhiInst =
      std::prev(NextBBI->BB->SkipPHIsAndLabels(NextBBI->BB->begin()));


  std::set<unsigned> UseRegsSet;

  // process each instruction in BB
  for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
    MachineInstr *MI = &*I;

    //DEBUG(errs() << "processing inst: ");
    //DEBUG(errs() << *MI);

    // TODO: do we need to process all insts?
    // what about picks/switches/cmps/brs?

    if (genDFInstructions(MI, Token, LastPhiInst, UseRegsSet)) {
      DEBUG(errs() << "modified graph - gen DF insts\n");
      genDFInst = true;
    }

  } // for each instruction in BB
  DEBUG(errs() << "end BB# - " <<  BB->getNumber() << "\n");

  return genDFInst;
}

bool CSAConvertControlPass::
processIfConversionRegion(std::vector<IfcvtToken*> &Tokens) {

    bool Change = false;


    if (IfConversionTokens == 0) return false;

    int TokensProcessedCnt = 0;
    while (!Tokens.empty()) {
      IfcvtToken *Token = Tokens.back();
      Tokens.pop_back();
      BBInfo &BBI = Token->BBI;
      IfcvtKind Kind = Token->Kind;
      unsigned NumDups = Token->NumDups;
      unsigned NumDups2 = Token->NumDups2;


      // If the block has been evicted out of the queue or it has already been
      // marked dead (due to it being predicated), then skip it.
      if (BBI.IsDone)
        BBI.IsEnqueued = false;
      if (!BBI.IsEnqueued) {
        delete Token;
        continue;
      }

      BBI.IsEnqueued = false;

      Change |= processIfConversionToken(Token);
      delete Token;

      bool RetVal = false;
      switch (Kind) {
      default: llvm_unreachable("Unexpected!");
      case ICSimple:
      case ICSimpleFalse: {
        bool isFalse = Kind == ICSimpleFalse;
        //if ((isFalse && DisableSimpleF) || (!isFalse && DisableSimple)) break;
        DEBUG(dbgs() << "Ifcvt (Simple" << (Kind == ICSimpleFalse ?
                                            " false" : "")
                     << "): BB#" << BBI.BB->getNumber() << " ("
                     << ((Kind == ICSimpleFalse)
                         ? BBI.FalseBB->getNumber()
                         : BBI.TrueBB->getNumber()) << ") ");
        RetVal = myIfConverter->IfConvertSimple(BBI, Kind);
        DEBUG(dbgs() << (RetVal ? "succeeded!" : "failed!") << "\n");
        if (RetVal) {
          if (isFalse) ++NumSimpleFalse;
          else         ++NumSimple;
        }
       break;
      }
      case ICTriangle:
      case ICTriangleRev:
      case ICTriangleFalse:
      case ICTriangleFRev: {
        bool isFalse = Kind == ICTriangleFalse;
        bool isRev   = (Kind == ICTriangleRev || Kind == ICTriangleFRev);
        //if (DisableTriangle && !isFalse && !isRev) break;
        //if (DisableTriangleR && !isFalse && isRev) break;
        //if (DisableTriangleF && isFalse && !isRev) break;
        //if (DisableTriangleFR && isFalse && isRev) break;
        DEBUG(dbgs() << "Ifcvt (Triangle");
        if (isFalse)
          DEBUG(dbgs() << " false");
        if (isRev)
          DEBUG(dbgs() << " rev");
        DEBUG(dbgs() << "): BB#" << BBI.BB->getNumber() << " (T:"
                     << BBI.TrueBB->getNumber() << ",F:"
                     << BBI.FalseBB->getNumber() << ") ");
        RetVal = myIfConverter->IfConvertTriangle(BBI, Kind);
        DEBUG(dbgs() << (RetVal ? "succeeded!" : "failed!") << "\n");
        if (RetVal) {
          if (isFalse) {
            if (isRev) ++NumTriangleFRev;
            else       ++NumTriangleFalse;
          } else {
            if (isRev) ++NumTriangleRev;
            else       ++NumTriangle;
          }
        }
        break;
      }
      case ICDiamond: {
        //if (DisableDiamond) break;
        DEBUG(dbgs() << "Ifcvt (Diamond): BB#" << BBI.BB->getNumber() << " (T:"
                     << BBI.TrueBB->getNumber() << ",F:"
                     << BBI.FalseBB->getNumber() << ") ");
        RetVal = myIfConverter->IfConvertDiamond(BBI, Kind, NumDups, NumDups2);
        DEBUG(dbgs() << (RetVal ? "succeeded!" : "failed!") << "\n");
        if (RetVal) ++NumDiamonds;
        break;
      }
      }

      Change |= RetVal;

#ifndef NDEBUG
      assert(IfConversionTokens > 0 && "IfConversionTokens has invalid value!");
      TokensProcessedCnt++;
      if (TokensProcessedCnt == IfConversionTokens) break;
#endif

      //NumIfCvts = NumSimple + NumSimpleFalse + NumTriangle + NumTriangleRev +
      //  NumTriangleFalse + NumTriangleFRev + NumDiamonds;
     // if (IfCvtLimit != -1 && (int)NumIfCvts >= IfCvtLimit)
     //   break;
    }

    return Change;

}

// process live ranges within the loop.
// Does conversion of registers into LICs. 
bool CSAConvertControlPass::processLiveRangesInLoop(MachineBasicBlock *BB) {
  myLicAllocater = new CSALicAllocation(LICAllocationStep);
  bool loopModified = false;
  if (myLicAllocater->pass_enabled()) {
    loopModified = myLicAllocater->allocateLicsInBlock(BB);
  }
  delete myLicAllocater;
  return loopModified;
}

bool CSAConvertControlPass::processLoopRegion(MachineLoop *currLoop) {

  bool loopModified = false;

  loopBackedgeBB = currLoop->getLoopLatch();
  loopPreheader = currLoop->getLoopPreheader();
  assert(loopBackedgeBB && "backedge BB is null!");

  for (MachineLoop::block_iterator BI = currLoop->block_begin(),
       E = currLoop->block_end(); BI != E; ++BI) {
    // for each block in loop body - should be only 1 for now
    MachineBasicBlock *BB = *BI;

    DEBUG(errs() << "\nbegin BB# - " << BB->getNumber() << "\n");

    // ignore empty blocks
    if (BB->empty()) {
      continue;
    }

    // TODO: ignore BB/pred/succ with un-analyzeable brs  -
    // check this early & quickly
    MachineBasicBlock::iterator LastPhiInst =
      std::prev(BB->SkipPHIsAndLabels(BB->begin()));

    // generate pick/switch DF ops for live-in uses and live-out defs
    for (MachineBasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
      MachineInstr *MI = &*I;

      DEBUG(errs() << "gen pick/switch DF ops: processing inst: ");
      DEBUG(errs() << *MI);

      // TODO: do we need to process all insts?
      // what about picks/switches/cmps/brs?

      if (genDFInstructions(MI, BB, LastPhiInst)) {
        DEBUG(errs() << "modified graph - gen DF insts\n");
        loopModified = true;
      }

    } // for each instruction in BB


    if (processLiveRangesInLoop(BB)) {
      DEBUG(errs() << "modified graph - processLiveRangesInLoop\n");
      loopModified = true;
    }

    DEBUG(errs() << "end BB# - " <<  BB->getNumber() << "\n");
  } // for each block in loop
  DEBUG(errs() << "end loop - " << "\n");

  return loopModified;
}

void CSAConvertControlPass::addLoop(std::vector<MachineLoop *> &loopList, MachineLoop *loop) {

  loopList.push_back(loop);

  // add nested loops
  const std::vector<MachineLoop*> &SubLoops = loop->getSubLoops();
  if (SubLoops.size() == 0) return;

  for (size_t SLI = 0; SLI != SubLoops.size(); SLI++) {
      addLoop(loopList, SubLoops[SLI]);
  }
}

bool CSAConvertControlPass::runOnMachineFunction(MachineFunction &MF) {

  if (ConvertControlPass == 0) return false;

  thisMF = &MF;

  DT = &getAnalysis<MachineDominatorTree>();

  bool Modified = false;

  // for now only well formed innermost loop regions are processed in this pass
  MachineLoopInfo *MLI = &getAnalysis<MachineLoopInfo>();

  if (!MLI) {
    DEBUG(errs() << "no loop info.\n");
    return false;
  }


  // first collect the loops to be in cfg (top-down) order
  // is it necessary to walk the loops in cfg order?
  std::vector<MachineLoop *> cfgOrderLoops;
  for (MachineLoopInfo::iterator LI = MLI->begin(), LE = MLI->end();
       LI != LE; ++LI) { // for all top-level loops in MF

    MachineLoop *ML = *LI;
    addLoop(cfgOrderLoops, ML);
  }

  prepareIfConverter();

  // for each (loop) region in cfg order
  int loopProcessedCnt = 0;
  for (std::vector<MachineLoop *>::reverse_iterator lpIter =
       cfgOrderLoops.rbegin(), lpIterEnd = cfgOrderLoops.rend();
       lpIter != lpIterEnd; ++lpIter) { // for all loops in MF
    MachineLoop *currLoop = *lpIter;

    DEBUG(errs() << "\nbegin loop# - \n");

    //Is this a candidate loop for dataflow conversion?
    if (!candidateLoopForDF(currLoop)) continue;

    //Process this loop to generate dataflow ops
    bool loopModified = false;
    if (processLoopRegion(currLoop)) {
      loopModified = true;
      Modified = true;
    }

#ifndef NDEBUG
    assert(ConvertControlPass > 0 && "ConvertControlPass has invalid value!");
    if (loopModified) loopProcessedCnt++;
    if (loopProcessedCnt == ConvertControlPass) return Modified;
#endif
  } // end for all loops in MF


  return Modified;

}
