#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/ADT/SCCIterator.h"
#include "CSASequenceOpt.h"
#include "CSAInstrInfo.h"
#include "CSATargetMachine.h"
#include "CSASeqOpt.h"
#include "MachineCDG.h"

using namespace llvm;

static cl::opt<bool> DisableMultiSeq("csa-disable-multiseq", cl::Hidden,
  cl::desc("CSA Specific: Disable multiple sequence conversion"));


CSASeqOpt::CSASeqOpt(MachineFunction *F) {
  thisMF = F;
  TII = static_cast<const CSAInstrInfo*>(thisMF->getSubtarget<CSASubtarget>().getInstrInfo());
  MRI = &thisMF->getRegInfo();
  LMFI = thisMF->getInfo<CSAMachineFunctionInfo>();
  TRI = thisMF->getSubtarget<CSASubtarget>().getRegisterInfo();
}

bool CSASeqOpt::isIntegerOpcode(unsigned opcode) {
  return TII->getOpcodeClass(opcode) == CSA::OpcodeClass::VARIANT_INT ||
    TII->getOpcodeClass(opcode) == CSA::OpcodeClass::VARIANT_SIGNED ||
    TII->getOpcodeClass(opcode) == CSA::OpcodeClass::VARIANT_UNSIGNED;
}

MachineInstr* CSASeqOpt::repeatOpndInSameLoop(MachineOperand& opnd, MachineInstr* lpCmp) {
  MachineInstr* pickInstr = MRI->getVRegDef(opnd.getReg());
  if (TII->isPick(pickInstr)) {
    MachineInstr* pick0 = MRI->getVRegDef(pickInstr->getOperand(2).getReg());
    MachineInstr* pick1 = MRI->getVRegDef(pickInstr->getOperand(3).getReg());
    if (TII->getGenericOpcode(pick0->getOpcode()) == CSA::Generic::REPEAT ||
        TII->getGenericOpcode(pick1->getOpcode()) == CSA::Generic::REPEAT) {
      MachineInstr* rptInstr = nullptr;
      MachineInstr* initInstr = nullptr;
      if (TII->getGenericOpcode(pick0->getOpcode()) == CSA::Generic::REPEAT) {
        rptInstr = pick0;
        initInstr = pick1;
      } else {
        rptInstr = pick1;
        initInstr = pick0;
      }
      if (MRI->getVRegDef(rptInstr->getOperand(2).getReg()) == initInstr) {
        if (MRI->getVRegDef(rptInstr->getOperand(1).getReg()) == lpCmp) {
          return rptInstr;
        } else if (MRI->getVRegDef(rptInstr->getOperand(1).getReg())->getOpcode() == CSA::NOT1) {
          MachineInstr*notInstr = MRI->getVRegDef(rptInstr->getOperand(1).getReg());
          if (MRI->getVRegDef(notInstr->getOperand(1).getReg()) == lpCmp) {
            return rptInstr;
          }
        }
      }
    }
  }
  return nullptr;
}


void CSASeqOpt::FoldRptInit(MachineInstr* rptInstr) {
  assert(MRI->hasOneUse(rptInstr->getOperand(0).getReg()));
  MachineRegisterInfo::use_iterator UI = MRI->use_begin(rptInstr->getOperand(0).getReg());
  MachineOperand &UseMO = *UI;
  MachineInstr* rptPick = UseMO.getParent();
  assert(TII->isPick(rptPick));
  unsigned pickDst = rptPick->getOperand(0).getReg();
  rptPick->removeFromParent();
  rptInstr->getOperand(0).setReg(pickDst);
}

void CSASeqOpt::SequenceIndv(CSASSANode* cmpNode, CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPickNode) {
  //addNode has inputs phiNode, and an immediate input
  bool isIDVCycle = TII->isAdd(addNode->minstr) &&
                    isIntegerOpcode(addNode->minstr->getOpcode()) && 
                    addNode->children[0] == lhdrPickNode;
  //switchNode has inputs mov->cmpNode, addNode
  isIDVCycle = isIDVCycle && MRI->getVRegDef(switchNode->minstr->getOperand(3).getReg()) == addNode->minstr;
  unsigned backedgeReg = 0;
  MachineInstr* loopInit = lpInitForPickSwitchPair(lhdrPickNode->minstr, switchNode->minstr, backedgeReg, cmpNode->minstr);
  isIDVCycle = isIDVCycle && (loopInit != nullptr);
  unsigned idvIdx = 0;
  //cmpNode has inputs either pickNode or addNode
  if (cmpNode->children.size() == 1) {
    //cmp with immediate
    idvIdx = cmpNode->minstr->getOperand(1).isReg() ? 1 : 2;
  } else if (repeatOpndInSameLoop(cmpNode->minstr->getOperand(1), cmpNode->minstr)) {
    idvIdx = 2;
  } else if (repeatOpndInSameLoop(cmpNode->minstr->getOperand(2), cmpNode->minstr)) {
    idvIdx = 1;
  } else {
    return;
  }
  isIDVCycle = isIDVCycle &&
               (MRI->getVRegDef(cmpNode->minstr->getOperand(idvIdx).getReg()) == lhdrPickNode->minstr ||
                MRI->getVRegDef(cmpNode->minstr->getOperand(idvIdx).getReg()) == addNode->minstr);

  MachineOperand& bndOpnd = cmpNode->minstr->getOperand(3 - idvIdx);
  MachineInstr* rptBnd = nullptr;
  MachineInstr* rptStride = nullptr;
  //boundary and stride must be integer value or integer register defined outside the loop
  isIDVCycle = isIDVCycle && (bndOpnd.isImm() || (rptBnd = repeatOpndInSameLoop(bndOpnd, cmpNode->minstr)));
  //handle only |stride| == 1 for now
  unsigned strideIdx = addNode->minstr->getOperand(1).isReg() ? 2 : 1;
  MachineOperand& strideOpnd = addNode->minstr->getOperand(strideIdx);
  //isIDVCycle = isIDVCycle && (strideOpnd.isImm() && (strideOpnd.getImm() == 1 || strideOpnd.getImm() == -1));
  isIDVCycle = isIDVCycle && (strideOpnd.isImm() || (rptStride = repeatOpndInSameLoop(strideOpnd, cmpNode->minstr)));


#if 0
  //uses of phi can only be add or cmp, or pick of the nested loop
  //new seq instr uses phi's dst as its value channel -- no need to consider phi dst's usage
  unsigned phidst = lhdrPickNode->minstr->getOperand(0).getReg();
  MachineRegisterInfo::use_iterator UI = MRI->use_begin(phidst);
  while (UI != MRI->use_end()) {
    MachineOperand &UseMO = *UI;
    ++UI;
    MachineInstr *UseMI = UseMO.getParent();
    if (UseMI != addNode->minstr && UseMI != cmpNode->minstr) {
      isIDVCycle = false;
      break;
    }
  }
#endif 

  //new seq instr uses phi's dst as its value channel -- no need to consider phi dst's usage
  //uses of add can only be switch or cmp
  unsigned adddst = addNode->minstr->getOperand(0).getReg();
  MachineRegisterInfo::use_iterator UI = MRI->use_begin(adddst);
  while (UI != MRI->use_end()) {
    MachineOperand &UseMO = *UI;
    ++UI;
    MachineInstr *UseMI = UseMO.getParent();
    if (UseMI != switchNode->minstr && UseMI != cmpNode->minstr) {
      isIDVCycle = false;
      break;
    }
  }

  if (cmpNode->minstr->getOperand(idvIdx).getReg() != switchNode->minstr->getOperand(3).getReg()) {
    isIDVCycle = false;
  }

  unsigned pickInitIdx = 2 + loopInit->getOperand(1).getImm();
  MachineOperand* initOpnd = &lhdrPickNode->minstr->getOperand(pickInitIdx);
  unsigned switchOutIndex = switchNode->minstr->getOperand(0).getReg() == backedgeReg ? 1 : 0;
  isIDVCycle = isIDVCycle && 
               (switchNode->minstr->getOperand(1 - switchOutIndex).getReg() == backedgeReg) &&
               MRI->hasOneUse(backedgeReg);
  if (isIDVCycle) {
    unsigned compareSense = idvIdx - 1;
    //1 means: false control sig loop back, true control sig exit loop
    unsigned switchSense = loopInit->getOperand(1).getImm();
    unsigned switchOutIndex = switchNode->minstr->getOperand(0).getReg() == backedgeReg ? 1 : 0;
    assert(switchNode->minstr->getOperand(1 - switchOutIndex).getReg() == backedgeReg);
    //no use of switch outside the loop, only use is lhdrphi
      // Find a sequence opcode that matches our compare opcode.
    unsigned seqOp;
    if (!CSASeqLoopInfo::compute_matching_seq_opcode(cmpNode->minstr->getOpcode(),
      addNode->minstr->getOpcode(),
      compareSense,
      switchSense,
      *TII,
      &seqOp)) {
      assert(false && "can't find matching sequence opcode\n");
    }
#if 0
    //add is before cmp => need to adjust init value
    if (MRI->getVRegDef(cmpNode->minstr->getOperand(idvIdx).getReg()) == addNode->minstr) {
      if (initOpnd->isImm() && strideOpnd.isImm()) {
        int adjInit = addNode->minstr->getOperand(strideIdx).getImm() + initOpnd->getImm();
        initOpnd = &MachineOperand::CreateImm(adjInit);
      } else {
        const TargetRegisterClass *TRC = TII->lookupLICRegClass(addNode->minstr->getOperand(0).getReg());
        unsigned adjInitVReg = LMFI->allocateLIC(TRC);
        MachineInstr* adjInitInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
          lhdrPickNode->minstr, DebugLoc(),
          TII->get(addNode->minstr->getOpcode()),
          adjInitVReg).
          add(addNode->minstr->getOperand(strideIdx)).                      //stride
          add(*initOpnd);                                                      //init
        adjInitInstr->setFlag(MachineInstr::NonSequential);
        initOpnd = &adjInitInstr->getOperand(0);
      }
    }
#endif 
    MachineOperand cpyInit(*initOpnd);
    cpyInit.setIsDef(false);
    unsigned firstReg = LMFI->allocateLIC(&CSA::CI1RegClass);
    unsigned lastReg = LMFI->allocateLIC(&CSA::CI1RegClass);
    unsigned seqReg = lhdrPickNode->minstr->getOperand(0).getReg();
    MachineOperand& indvBnd = bndOpnd.isReg() && rptBnd ? rptBnd->getOperand(2) : bndOpnd;
    MachineOperand& indvStride = strideOpnd.isReg() && rptStride ? rptStride->getOperand(2) : strideOpnd;
    unsigned seqPred = cmpNode->minstr->getOperand(0).getReg();
    MachineInstr* seqInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
      lhdrPickNode->minstr, DebugLoc(),
      TII->get(seqOp),
      seqReg).
      addReg(seqPred, RegState::Define).  //pred
      addReg(firstReg, RegState::Define).
      addReg(lastReg, RegState::Define).
      add(cpyInit).                    //init
      add(indvBnd).                    //boundary
      add(indvStride);                 //stride
    seqInstr->setFlag(MachineInstr::NonSequential);

    MachineRegisterInfo::use_iterator UI = MRI->use_begin(seqPred);
    while (UI != MRI->use_end()) {
      MachineOperand &UseMO = *UI;
      ++UI;
      MachineInstr *UseMI = UseMO.getParent();
      if (TII->getGenericOpcode(UseMI->getOpcode()) == CSA::Generic::REPEAT && 
          UseMI->getOperand(1).getReg() == seqPred &&
          MRI->hasOneUse(UseMI->getOperand(0).getReg())) {
        MachineRegisterInfo::use_iterator UI = MRI->use_begin(UseMI->getOperand(0).getReg());
        MachineInstr* rptPick = UI->getParent();
        unsigned rptValue = UseMI->getOperand(2).getReg();
        if (TII->isPick(rptPick) && 
            rptPick->getOperand(1).getReg() == loopInit->getOperand(0).getReg() &&
           (rptPick->getOperand(2).getReg() == rptValue || rptPick->getOperand(3).getReg() == rptValue)) {
          FoldRptInit(UseMI);
        }
      } else if (UseMI->getOpcode() == CSA::NOT1 && loopInit->getOperand(1).getImm() == 1) {
        //1=>init/exit, 0=>loop back; repeat's ctrl is from a not1 instr; need to remove the not1 when replace cmpdst with seqindv's pred
        if (MRI->hasOneUse(UseMI->getOperand(0).getReg())) {
          MachineInstr* rptInstr = MRI->use_begin(UseMI->getOperand(0).getReg())->getParent();
          if (TII->getGenericOpcode(rptInstr->getOpcode()) == CSA::Generic::REPEAT &&
            rptInstr->getOperand(1).getReg() == UseMI->getOperand(0).getReg() &&
            MRI->hasOneUse(rptInstr->getOperand(0).getReg())) { //rpt only used by pick init
            MachineRegisterInfo::use_iterator UI = MRI->use_begin(rptInstr->getOperand(0).getReg());
            MachineInstr* rptPick = UI->getParent();
            unsigned rptValue = rptInstr->getOperand(2).getReg();
            if (TII->isPick(rptPick) &&
              (rptPick->getOperand(1).getReg() == loopInit->getOperand(0).getReg() && 
               rptPick->getOperand(3).getReg() == rptValue &&
               rptPick->getOperand(2).getReg() == rptInstr->getOperand(0).getReg())) {
              FoldRptInit(rptInstr);
              rptInstr->getOperand(1).setReg(seqPred);
              UseMI->removeFromParent();
            }
          }
        }
      }
    }

    if (loopInit->getOperand(1).getImm() == 1) {
      //1=>init/exit, 0=>loop back; need to flip switch dsts for backward value when replace cmpdst with seqindv's pred
      SequenceFlipSwitchDsts(cmpNode);
    }
    //remove the instructions in the IDV cycle.
    cmpNode->minstr->removeFromParent();
    switchNode->minstr->removeFromParent();
    addNode->minstr->removeFromBundle();
    lhdrPickNode->minstr->removeFromParent();
    //currently only the cmp instr can have usage outside the cycle
    cmpNode->minstr = seqInstr;
  }
}


MachineOperand CSASeqOpt::CalculateTripCnt(MachineOperand& initOpnd, MachineOperand& bndOpnd) {
  if (initOpnd.isImm() && bndOpnd.isImm()) {
    unsigned vtripcnt = bndOpnd.getImm() - initOpnd.getImm();
    return MachineOperand::CreateImm(vtripcnt);
  } else if (initOpnd.isImm() && initOpnd.getImm() == 0) {
    return bndOpnd;
  } else {
    //boundry - init
    MachineOperand& regOpnd = bndOpnd.isImm() ? initOpnd : bndOpnd;
    const TargetRegisterClass *TRC = TII->lookupLICRegClass(regOpnd.getReg());
    unsigned regTripcnt = LMFI->allocateLIC(TRC);
    MachineInstr* seqInstr = initOpnd.getParent();
    MachineInstr* tripcntInstr = BuildMI(*seqInstr->getParent(), initOpnd.getParent(), DebugLoc(),
      TII->get(TII->adjustOpcode(seqInstr->getOpcode(), CSA::Generic::SUB)),
      regTripcnt).
      add(bndOpnd).
      add(initOpnd);
    tripcntInstr->setFlag(MachineInstr::NonSequential);
    return tripcntInstr->getOperand(0);
  }
}


MachineOperand CSASeqOpt::tripCntForSeq(MachineInstr*seqIndv) {
  assert(TII->isSeqOT(seqIndv));
  MachineOperand& initIndv = seqIndv->getOperand(4);
  MachineOperand& bndIndv = seqIndv->getOperand(5);
  MachineOperand& stridIndv = seqIndv->getOperand(6);
  MachineOperand tripcntOpnd = MachineOperand::CreateImm(-2);
  if (stridIndv.isImm() && (stridIndv.getImm() == 1 || stridIndv.getImm() == -1)) {
    //TODO: only compute trip-counter when step is 1
    CSA::Generic indvGenOp = TII->getGenericOpcode(seqIndv->getOpcode());
    if (stridIndv.getImm() == -1 &&
      (indvGenOp == CSA::Generic::SEQOTGT || indvGenOp == CSA::Generic::SEQOTGE || indvGenOp == CSA::Generic::SEQOTNE)) {
      //trip counter = init - boundary
      tripcntOpnd = CalculateTripCnt(bndIndv, initIndv);
    } else if (stridIndv.getImm() == 1 &&
      (indvGenOp == CSA::Generic::SEQOTLT || indvGenOp == CSA::Generic::SEQOTLE || indvGenOp == CSA::Generic::SEQOTNE)) {
      //trip counter = boundary - init 
      tripcntOpnd = CalculateTripCnt(initIndv, bndIndv);
    }
    //adjust trip counter for equal comparison
    if (indvGenOp == CSA::Generic::SEQOTGE || indvGenOp == CSA::Generic::SEQOTLE) {
      const TargetRegisterClass *TRC = TII->lookupLICRegClass(stridIndv.getReg());
      unsigned regTripcnt = LMFI->allocateLIC(TRC);
      MachineInstr* tripcntInstr = BuildMI(*seqIndv->getParent(), seqIndv, DebugLoc(),
        TII->get(TII->adjustOpcode(seqIndv->getOpcode(), CSA::Generic::ADD)),
        regTripcnt).
        add(tripcntOpnd).addImm(1);
      tripcntInstr->setFlag(MachineInstr::NonSequential);
      tripcntOpnd = tripcntInstr->getOperand(0);
    } else {
      //no adjust needed for CSA::Generic::SEQOTGT || CSA::Generic::SEQOTLT || CSA::Generic::SEQOTNE
    }
  }
  return tripcntOpnd;
}


void CSASeqOpt::SequenceApp(CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPickNode) {
  unsigned phidst = lhdrPickNode->minstr->getOperand(0).getReg();
  unsigned adddst = addNode->minstr->getOperand(0).getReg();
  if (MRI->hasOneUse(phidst) && MRI->hasOneUse(adddst)) {
    SequenceReduction(switchNode, addNode, lhdrPickNode);
  } else {
    MultiSequence(switchNode, addNode, lhdrPickNode);
  }
}

void CSASeqOpt::SequenceReduction(CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPickNode) {
  //switchNode has inputs addNode, and switch's control is SeqOT
  MachineInstr* seqOT = MRI->getVRegDef(switchNode->minstr->getOperand(2).getReg());
  bool isIDVCycle = TII->isSeqOT(seqOT) && MRI->getVRegDef(switchNode->minstr->getOperand(3).getReg()) == addNode->minstr;
  unsigned backedgeReg = 0;
  MachineInstr* loopInit = lpInitForPickSwitchPair(lhdrPickNode->minstr, switchNode->minstr, backedgeReg, seqOT);
  unsigned idvIdx = MRI->getVRegDef(addNode->minstr->getOperand(1).getReg()) == lhdrPickNode->minstr ? 1 : 2;
  unsigned strideIdx = 3 - idvIdx;
  isIDVCycle = isIDVCycle && (loopInit != nullptr) && 
               MRI->getVRegDef(addNode->minstr->getOperand(idvIdx).getReg()) == lhdrPickNode->minstr;
  unsigned pickInitIdx = 2 + loopInit->getOperand(1).getImm();
  MachineOperand& initOpnd = lhdrPickNode->minstr->getOperand(pickInitIdx);
  unsigned switchOutIndex = switchNode->minstr->getOperand(0).getReg() == backedgeReg ? 1 : 0;
  unsigned switchOutReg = switchNode->minstr->getOperand(switchOutIndex).getReg();
  isIDVCycle = isIDVCycle &&
    (switchNode->minstr->getOperand(1 - switchOutIndex).getReg() == backedgeReg) &&
    MRI->hasOneUse(backedgeReg);

  if (isIDVCycle) {
    //build reduction seqeuence.
    unsigned redOp = TII->convertTransformToReductionOp(addNode->minstr->getOpcode());
    //64bit operations such as ADDF64 are not supported
    //if (redOp != CSA::INVALID_OPCODE) 
    assert(redOp != CSA::INVALID_OPCODE);
    MachineInstr* redInstr;
    if (TII->isFMA(addNode->minstr)) {
      //two input reduction besides init
      redInstr = BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr, DebugLoc(),
        TII->get(redOp),
        switchOutReg).                          // result
        addReg(backedgeReg, RegState::Define).  // each 
        add(initOpnd).                         // initial value
        add(addNode->minstr->getOperand(1)).   // input 1
        add(addNode->minstr->getOperand(2)).   // input 2
        addReg(seqOT->getOperand(1).getReg()); // control
    } else {
      //normal one input reduciton besides init
      redInstr = BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr, DebugLoc(),
        TII->get(redOp),
        switchOutReg).                          // result
        addReg(backedgeReg, RegState::Define).  // each 
        add(initOpnd).                         // initial value
        add(addNode->minstr->getOperand(strideIdx)).   // input 1
        addReg(seqOT->getOperand(1).getReg()); // control
    }
    redInstr->setFlag(MachineInstr::NonSequential);
    //remove the instructions in the IDV cycle.
    switchNode->minstr->removeFromParent();
    addNode->minstr->removeFromBundle();
    lhdrPickNode->minstr->removeFromParent();
  }
}



void CSASeqOpt::SequenceFlipSwitchDsts(CSASSANode* cmpNode) {
  unsigned cmpdst = cmpNode->minstr->getOperand(0).getReg();
  MachineRegisterInfo::use_iterator UI = MRI->use_begin(cmpdst);
  while (UI != MRI->use_end()) {
    MachineOperand &UseMO = *UI;
    ++UI;
    MachineInstr *UseMI = UseMO.getParent();
    if (TII->isSwitch(UseMI) && UseMI->getOperand(2).getReg() == cmpdst) {
      //flip pred for the backward value
      unsigned switchTrue = UseMI->getOperand(1).getReg();
      UseMI->getOperand(1).setReg(UseMI->getOperand(0).getReg());
      UseMI->getOperand(0).setReg(switchTrue);
    }
  }
}




#if 0
void CSASeqOpt::SequenceSwitchOutLast(CSASSANode* cmpNode, MachineInstr* initInstr, MachineInstr* seqIndv) {
  unsigned cmpdst = cmpNode->minstr->getOperand(0).getReg();
  MachineRegisterInfo::use_iterator UI = MRI->use_begin(cmpdst);
  while (UI != MRI->use_end()) {
    MachineOperand &UseMO = *UI;
    ++UI;
    MachineInstr *UseMI = UseMO.getParent();
    assert((TII->isSwitch(UseMI) && UseMI->getOperand(2).getReg() == cmpdst) || 
           (TII->isMOV(UseMI) && UseMI->getOperand(0).getReg() == initInstr->getOperand(0).getReg()));
    if (TII->isSwitch(UseMI) && UseMI->getOperand(2).getReg() == cmpdst) {
      if (initInstr->getOperand(1).getImm() == 1) {
        //1=>init/exit, 0=>loop back; need to flip switch dsts for backward value when replace cmpdst with seqindv's pred
        if (UseMI->getOperand(0).getReg() == CSA::IGN) {
          //using last_pred to switch out the last value
          UseMI->substituteRegister(cmpdst, seqIndv->getOperand(3).getReg(), 0, *TRI);
        } else if (UseMI->getOperand(1).getReg() == CSA::IGN) {
          //flip pred for the backward value
          UseMI->getOperand(1).setReg(UseMI->getOperand(0).getReg());
          UseMI->getOperand(0).setReg(CSA::IGN);
        } else {
          //split the switch into two switches: one backward only, one out only
          //first switch out
          MachineInstr* switchoutInstr = BuildMI(*UseMI->getParent(), UseMI, DebugLoc(),
            TII->get(UseMI->getOpcode()),
            CSA::IGN).
            addReg(UseMI->getOperand(1).getReg()).
            addReg(seqIndv->getOperand(3).getReg()). //last pred
            addReg(UseMI->getOperand(3).getReg());
          //then for switch backward flip pred for the backward value
          UseMI->getOperand(1).setReg(UseMI->getOperand(0).getReg());
          UseMI->getOperand(0).setReg(CSA::IGN);
          switchoutInstr->setFlag(MachineInstr::NonSequential);
        }
      } else {
        //0=>init/exit, 1=>loop back; need to flip switch dsts for last value when replace cmpdst with seqindv's pred
        if (UseMI->getOperand(1).getReg() == CSA::IGN) {
          UseMI->getOperand(1).setReg(UseMI->getOperand(0).getReg());
          UseMI->getOperand(0).setReg(CSA::IGN);
          //flip switch dst's for last value
          //using last_pred to switch out the last value
          UseMI->substituteRegister(cmpdst, seqIndv->getOperand(3).getReg(), 0, *TRI);
        } else if (UseMI->getOperand(0).getReg() == CSA::IGN) {
          //only backward value on pred true, do nothing
        } else {
          //split the switch into two switches: one backward only, one out only
          //first switch out
          MachineInstr* switchoutInstr = BuildMI(*UseMI->getParent(), UseMI, DebugLoc(),
            TII->get(UseMI->getOpcode()),
            CSA::IGN).
            addReg(UseMI->getOperand(0).getReg()).
            addReg(seqIndv->getOperand(3).getReg()). //last pred
            addReg(UseMI->getOperand(3).getReg());
          switchoutInstr->setFlag(MachineInstr::NonSequential);
          //then for switch backward, ignore last out 
          UseMI->getOperand(0).setReg(CSA::IGN);
        }
      }
    }
  }
}
#else
void CSASeqOpt::SequenceSwitchOutLast(MachineInstr* switchInstr, MachineInstr* seqIndv) {
  assert(TII->isSeqOT(MRI->getVRegDef(switchInstr->getOperand(2).getReg())));
  //can't use seq_pred in switch instr; since seq_pred ==~ not_last, need to replace seq_pred with last, and flip the switch's dsts.
  switchInstr->substituteRegister(switchInstr->getOperand(2).getReg(), seqIndv->getOperand(3).getReg(), 0, *TRI);
  unsigned switchFalse = switchInstr->getOperand(0).getReg();
  switchInstr->getOperand(0).setReg(switchInstr->getOperand(1).getReg());
  switchInstr->getOperand(1).setReg(switchFalse);

}
#endif

void CSASeqOpt::SequenceSwitchOut(CSASSANode* switchNode, 
                                  CSASSANode* addNode, 
                                  CSASSANode* lhdrPickNode, 
                                  MachineInstr* seqIndv, 
                                  unsigned seqReg,
                                  unsigned backedgeReg) {
  //adjust switch out value
  unsigned switchFalse = switchNode->minstr->getOperand(0).getReg();
  unsigned switchTrue = switchNode->minstr->getOperand(1).getReg();
  unsigned switchOut = switchFalse == backedgeReg ? switchTrue : switchFalse;
  unsigned strideIdx = addNode->minstr->getOperand(1).isReg() ? 2 : 1;
  if (switchOut != CSA::IGN) {
    //compute the outbouned value for switchout = last + stride
    const TargetRegisterClass *TRC = TII->lookupLICRegClass(addNode->minstr->getOperand(0).getReg());
    unsigned last = LMFI->allocateLIC(TRC);
    const unsigned switchOp = TII->makeOpcode(CSA::Generic::SWITCH, TRC);
    MachineInstr* switchLast = BuildMI(*lhdrPickNode->minstr->getParent(),
      lhdrPickNode->minstr,
      DebugLoc(),
      TII->get(switchOp),
      CSA::IGN).
      addReg(last, RegState::Define).
      addReg(seqIndv->getOperand(3).getReg()).   //last
      addReg(seqReg);
    switchLast->setFlag(MachineInstr::NonSequential);

    MachineInstr* outbndInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
      lhdrPickNode->minstr,
      DebugLoc(),
      TII->get(switchOp),
      switchOut).
      addReg(last).
      add(addNode->minstr->getOperand(strideIdx));
    outbndInstr->setFlag(MachineInstr::NonSequential);
  }
}


void CSASeqOpt::MultiSequence(CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPickNode) {
  //addNode has inputs phiNode
  bool isIDVCycle = TII->isAdd(addNode->minstr) &&
                    isIntegerOpcode(addNode->minstr->getOpcode()) && 
                    addNode->children[0] == lhdrPickNode;
  //switchNode has inputs addNode, and switch's control is SeqOT
  MachineInstr* seqIndv = MRI->getVRegDef(switchNode->minstr->getOperand(2).getReg());
  isIDVCycle = isIDVCycle &&
    TII->isSeqOT(seqIndv) &&
    MRI->getVRegDef(switchNode->minstr->getOperand(3).getReg()) == addNode->minstr;

  unsigned backedgeReg = 0;
  MachineInstr* loopInit = lpInitForPickSwitchPair(lhdrPickNode->minstr, switchNode->minstr, backedgeReg, seqIndv);
  isIDVCycle = isIDVCycle && (loopInit != nullptr);

  //handle only positive constant address stride for now
  unsigned strideIdx = addNode->minstr->getOperand(1).isReg() ? 2 : 1;
  isIDVCycle = isIDVCycle &&
               addNode->minstr->getOperand(strideIdx).isImm() &&
               addNode->minstr->getOperand(strideIdx).getImm() > 0;
  //uses of phi can only be add or address computing
  //bool phiuseOK = false;
  unsigned phidst = lhdrPickNode->minstr->getOperand(0).getReg();
  //bool adduseOK = false;
  unsigned adddst = addNode->minstr->getOperand(0).getReg();
  //otherwise reduced to reduction
  assert(!MRI->hasOneUse(phidst) || !MRI->hasOneUse(adddst));

#if 0
  //only consider init value from phi, not from add
  if (MRI->hasOneUse(phidst)) {
    phiuseOK = true;
    MachineRegisterInfo::use_instr_iterator I = MRI->use_instr_begin(adddst);
    MachineRegisterInfo::use_instr_iterator Inext = std::next(I);
    //add has exactly two uses
    if (std::next(Inext) == MachineRegisterInfo::use_instr_end()) {
      if ((TII->isLoad(&*I) || TII->isStore(&*I)) &&
        &*Inext == addNode->minstr) {
        adduseOK = true;
      } else if ((TII->isLoad(&*Inext) || TII->isStore(&*Inext)) &&
        &*I == addNode->minstr) {
        adduseOK = true;
      }
    }
  }
#endif 


#if 0
  //no multi-sequence for now
  if (MRI->hasOneUse(adddst)) {
    adduseOK = true;
    assert(!MRI->use_empty(phidst));
    MachineRegisterInfo::use_instr_iterator I = MRI->use_instr_begin(phidst);
    MachineRegisterInfo::use_instr_iterator Inext = std::next(I);
    //phi has exactly two uses
    if (std::next(Inext) == MachineRegisterInfo::use_instr_end()) {
      if ((TII->isLoad(&*I) || TII->isStore(&*I)) &&
        &*Inext == addNode->minstr) {
        phiuseOK = true;
      } else if ((TII->isLoad(&*Inext) || TII->isStore(&*Inext)) &&
        &*I == addNode->minstr) {
        phiuseOK = true;
      }
    }
  }
  isIDVCycle = isIDVCycle && phiuseOK && adduseOK;
#endif 
  if (isIDVCycle) {
    MachineOperand& initOpnd = lhdrPickNode->minstr->getOperand(2).getReg() == backedgeReg ?
      lhdrPickNode->minstr->getOperand(3) :
      lhdrPickNode->minstr->getOperand(2);
    //const TargetRegisterClass *addTRC = TII->lookupLICRegClass(addNode->minstr->getOperand(0).getReg());
    unsigned seqReg = phidst;
#if 0
    //adjust init value depend on whether load/store is before add
    if (!MRI->hasOneUse(phidst)) {
      assert(MRI->hasOneUse(adddst));
      seqReg = phidst;
    }  else {
      assert(!MRI->hasOneUse(adddst));
      seqReg = adddst;
      //adjust init
      if (initOpnd.isImm()) {
        assert(addNode->minstr->getOperand(strideIdx).isImm());
        int adjInit = initOpnd.getImm() + addNode->minstr->getOperand(strideIdx).getImm();
        initOpnd = MachineOperand::CreateImm(adjInit);
      } else {
        unsigned adjInitVReg = LMFI->allocateLIC(addTRC);
        MachineInstr* adjInitInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
          lhdrPickNode->minstr, DebugLoc(),
          TII->get(addNode->minstr->getOpcode()),
          adjInitVReg).
          add(addNode->minstr->getOperand(strideIdx)).                      //stride
          add(initOpnd);                                                      //init
        adjInitInstr->setFlag(MachineInstr::NonSequential);
        initOpnd = adjInitInstr->getOperand(0);
      }
    }
#endif

#if 1
    //no multiple sequence for now
    MachineOperand tripcnt = tripCntForSeq(seqIndv);
    if (tripcnt.isReg()) tripcnt.setIsDef(false);
    //got a valid trip counter, convert to squence; otherwise stride
    if (!DisableMultiSeq && (!tripcnt.isImm() || tripcnt.getImm() > 0))  { 
      const TargetRegisterClass *addTRC = TII->lookupLICRegClass(addNode->minstr->getOperand(0).getReg());
      //FMA only operates on register
      unsigned fmaReg = LMFI->allocateLIC(addTRC);
      if (addNode->minstr->getOperand(strideIdx).isImm()) {
        unsigned mulReg = LMFI->allocateLIC(addTRC);
        if (addNode->minstr->getOperand(strideIdx).getImm() != 1) {
          //avoid muptipy by 1
          const unsigned mulOp = TII->makeOpcode(CSA::Generic::MUL, addTRC);
          MachineInstr* mulInstr = BuildMI(*seqIndv->getParent(),
            lhdrPickNode->minstr, DebugLoc(),
            TII->get(mulOp),
            mulReg).
            add(tripcnt).
            add(addNode->minstr->getOperand(strideIdx));                                  //trip count from indv
          mulInstr->setFlag(MachineInstr::NonSequential);
        } else {
          mulReg = tripcnt.getReg();
        }
        const unsigned addOp = TII->makeOpcode(CSA::Generic::ADD, addTRC);
        MachineInstr* addInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
          lhdrPickNode->minstr, DebugLoc(),
          TII->get(addOp),
          fmaReg).
          addReg(mulReg).
          add(initOpnd);                                  //trip count from indv
        addInstr->setFlag(MachineInstr::NonSequential);
      } else {
        const unsigned FMAOp = TII->makeOpcode(CSA::Generic::FMA, addTRC);
        MachineInstr* fmaInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
          lhdrPickNode->minstr, DebugLoc(),
          TII->get(FMAOp),
          fmaReg).
          add(tripcnt).                                   //trip count from indv
          add(addNode->minstr->getOperand(strideIdx)).          //stride
          add(initOpnd);                                        //init
        fmaInstr->setFlag(MachineInstr::NonSequential);
      }
      unsigned firstReg = LMFI->allocateLIC(&CSA::CI1RegClass);
      unsigned lastReg = LMFI->allocateLIC(&CSA::CI1RegClass);
      unsigned predReg = LMFI->allocateLIC(&CSA::CI1RegClass);
      const unsigned seqOp = TII->makeOpcode(CSA::Generic::SEQOTLT, addTRC);
      MachineInstr* seqInstr = BuildMI(*seqIndv->getParent(),
        lhdrPickNode->minstr, DebugLoc(),
        //TII->get(seqIndv->getOpcode()),
        TII->get(seqOp),
        seqReg).
        addReg(predReg, RegState::Define).                    //pred
        addReg(firstReg, RegState::Define).
        addReg(lastReg, RegState::Define).
        add(initOpnd).                                        //init
        addReg(fmaReg).                                       //boundary
        add(addNode->minstr->getOperand(strideIdx));          //stride
      seqInstr->setFlag(MachineInstr::NonSequential);
    } 
    else
#endif
    {
      //can't figure out trip-counter; generate stride 
      const TargetRegisterClass *TRC = TII->lookupLICRegClass(addNode->minstr->getOperand(0).getReg());
      const unsigned strideOp = TII->makeOpcode(CSA::Generic::STRIDE, TRC);
      MachineInstr* strideInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
        lhdrPickNode->minstr,
        DebugLoc(),
        TII->get(strideOp),
        seqReg).
        addReg(seqIndv->getOperand(1).getReg()).
        add(initOpnd).
        add(addNode->minstr->getOperand(strideIdx));
      strideInstr->setFlag(MachineInstr::NonSequential);
    }
    //SequenceSwitchOut(switchNode, addNode, lhdrPickNode, seqIndv, seqReg, backedgeReg);
    //remove the instructions in the IDV cycle.
    switchNode->minstr->removeFromParent();
    addNode->minstr->removeFromBundle();
    lhdrPickNode->minstr->removeFromParent();
  }
}

//pick/switch paire representing a repeat consturct, 
//with pick dst leads to switch, and switch's dst defines pick's backedge
//cmp result is used to control switch; init/mov is used to control pick
MachineInstr* CSASeqOpt::lpInitForPickSwitchPair(MachineInstr* pickInstr, MachineInstr* switchInstr, unsigned& backedgeReg, MachineInstr* lpcmpInstr) {
  unsigned channel = pickInstr->getOperand(1).getReg();
  assert(TII->isPick(pickInstr));
  MachineInstr* result = nullptr;
  if (!MRI->hasOneDef(channel)) {
    MachineRegisterInfo::def_instr_iterator defI = MRI->def_instr_begin(channel);
    MachineRegisterInfo::def_instr_iterator defInext = std::next(defI);
    assert(std::next(defInext) == MachineRegisterInfo::def_instr_end());
    if ((TII->isMOV(&*defI) && TII->isInit(&*defInext)) || (TII->isInit(&*defI) && TII->isMOV(&*defInext))) {
      MachineInstr* initInstr = TII->isInit(&*defI) ? &*defI : &*defInext;
      MachineInstr* movInstr = TII->isMOV(&*defI) ? &*defI : &*defInext;
      MachineInstr* cmpInstr = MRI->getVRegDef(movInstr->getOperand(1).getReg());
      if ((TII->isCmp(cmpInstr) || (lpcmpInstr && cmpInstr == lpcmpInstr)) && initInstr->getOperand(1).isImm() && 
          (initInstr->getOperand(1).getImm() == 1 || initInstr->getOperand(1).getImm() == 0)) {
        backedgeReg = initInstr->getOperand(1).getImm() == 0 ?
                      pickInstr->getOperand(3).getReg() :
                      pickInstr->getOperand(2).getReg();
        if (MRI->getVRegDef(backedgeReg) == switchInstr && MRI->getVRegDef(switchInstr->getOperand(2).getReg()) == cmpInstr) {
          result = initInstr;
        }
      }
    }
  }
  return result;
}



void CSASeqOpt::SequenceRepeat(CSASSANode* switchNode, CSASSANode* lhdrPickNode) {
  //switchNode has inputs lhdrPick
  bool isIDVCycle = MRI->getVRegDef(switchNode->minstr->getOperand(3).getReg()) == lhdrPickNode->minstr;
  bool switchuseOK = false;
  unsigned lpbackReg;
  MachineInstr* lpInit = nullptr;
  MachineInstr* switchCtrl = MRI->getVRegDef(switchNode->minstr->getOperand(2).getReg());
  if (TII->isSeqOT(switchCtrl)) {
    lpInit = lpInitForPickSwitchPair(lhdrPickNode->minstr, switchNode->minstr, lpbackReg, switchCtrl);
  } else {
    lpInit = lpInitForPickSwitchPair(lhdrPickNode->minstr, switchNode->minstr, lpbackReg);
  }
  unsigned switchFalse = switchNode->minstr->getOperand(0).getReg();
  unsigned switchTrue = switchNode->minstr->getOperand(1).getReg();
  if ((switchFalse == CSA::IGN && MRI->hasOneUse(switchTrue) && switchTrue == lpbackReg) ||
      (switchTrue == CSA::IGN && MRI->hasOneUse(switchFalse) && switchFalse == lpbackReg)) {
    switchuseOK = true;
  }
  
  isIDVCycle = isIDVCycle && switchuseOK && (lpInit != nullptr);
  if (isIDVCycle) {
    unsigned predRepeat = switchNode->minstr->getOperand(2).getReg();
#if 1
    if (switchFalse == lpbackReg) {
      unsigned notReg = LMFI->allocateLIC(&CSA::CI1RegClass);
      MachineInstr* notInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
        lhdrPickNode->minstr, DebugLoc(), TII->get(CSA::NOT1),
        notReg).
        addReg(predRepeat);
      notInstr->setFlag(MachineInstr::NonSequential);
      predRepeat = notReg;
    } 
#endif
    unsigned valueRepeat = lhdrPickNode->minstr->getOperand(2).getReg() == lpbackReg ?
      lhdrPickNode->minstr->getOperand(3).getReg() :
      lhdrPickNode->minstr->getOperand(2).getReg();
#if 0
    if (!TII->isSeqOT(switchCtrl)) {
      unsigned movOp = TII->adjustOpcode(switchNode->minstr->getOpcode(), CSA::Generic::MOV);
      MachineInstr* movInstr = BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr, DebugLoc(), TII->get(movOp),
        lhdrPickNode->minstr->getOperand(0).getReg()).
        addReg(valueRepeat);
      movInstr->setFlag(MachineInstr::NonSequential);
    }
#endif
    unsigned repeatOp = TII->adjustOpcode(switchNode->minstr->getOpcode(), CSA::Generic::REPEAT);
    assert(repeatOp != CSA::INVALID_OPCODE);
    MachineInstr* repeatInstr = BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr, DebugLoc(), TII->get(repeatOp),
      lpbackReg).
      addReg(predRepeat).
      addReg(valueRepeat); 
    repeatInstr->setFlag(MachineInstr::NonSequential);

    //remove the instructions in the IDV cycle.
    switchNode->minstr->removeFromParent();
    //lhdrPickNode->minstr->removeFromParent();

  }
}



void CSASeqOpt::PrepRepeat() {
  CSASSAGraph csaSSAGraph;
  csaSSAGraph.BuildCSASSAGraph(*thisMF, true); //no control flow dependence edge
  for (scc_iterator<CSASSANode*> I = scc_begin(csaSSAGraph.getRoot()), IE = scc_end(csaSSAGraph.getRoot()); I != IE; ++I) {
    const std::vector<CSASSANode *> &SCCNodes = *I;
    if (SCCNodes.size() == 2) {
      CSASSANode* lhdrPickNode = nullptr;
      CSASSANode* switchNode = nullptr;
      bool isIDVCandidate = true;
      for (std::vector<CSASSANode *>::const_iterator nodeI = SCCNodes.begin(), nodeIE = SCCNodes.end(); nodeI != nodeIE && isIDVCandidate; ++nodeI) {
        CSASSANode* sccn = *nodeI;
        MachineInstr* minstr = sccn->minstr;
        //loop header phi
        if (TII->isPick(minstr) && !lhdrPickNode) {
          unsigned pickCtrl = minstr->getOperand(1).getReg();
          for (MachineInstr &ctrlDef : MRI->def_instructions(pickCtrl)) {
            if (ctrlDef.getOpcode() == CSA::MOV1) continue;
            else if (ctrlDef.getOpcode() == CSA::INIT1 &&
              (ctrlDef.getOperand(1).getImm() == 0 ||
                ctrlDef.getOperand(1).getImm() == 1)) {
              lhdrPickNode = sccn;
            } else
              isIDVCandidate = false;
          }
        } else if (TII->isSwitch(minstr) && !switchNode) {
          switchNode = sccn;
        } else {
          isIDVCandidate = false;
        }
      }
      if (isIDVCandidate && switchNode && lhdrPickNode) {
        SequenceRepeat(switchNode, lhdrPickNode);
      }
    }
  }
}


void CSASeqOpt::SequenceOPT() {
  PrepRepeat();
  CSASSAGraph csaSSAGraph;
  csaSSAGraph.BuildCSASSAGraph(*thisMF);
  for (scc_iterator<CSASSANode*> I = scc_begin(csaSSAGraph.getRoot()), IE = scc_end(csaSSAGraph.getRoot()); I != IE; ++I) {
    const std::vector<CSASSANode *> &SCCNodes = *I;
    if (SCCNodes.size() > 1 && SCCNodes.size() < 8) {
      CSASSANode* lhdrPickNode = nullptr;
      CSASSANode* cmpNode = nullptr;
      CSASSANode* addNode = nullptr;
      CSASSANode* switchNode = nullptr;
      bool isIDVCandidate = true;
      for (std::vector<CSASSANode *>::const_iterator nodeI = SCCNodes.begin(), nodeIE = SCCNodes.end(); nodeI != nodeIE && isIDVCandidate; ++nodeI) {
        CSASSANode* sccn = *nodeI;
        MachineInstr* minstr = sccn->minstr;
        //loop header phi
        if (TII->isPick(minstr) && !lhdrPickNode) {
          unsigned pickCtrl = minstr->getOperand(1).getReg();
          for (MachineInstr &ctrlDef : MRI->def_instructions(pickCtrl)) {
            if (ctrlDef.getOpcode() == CSA::MOV1) continue;
            else if (ctrlDef.getOpcode() == CSA::INIT1 &&
              (ctrlDef.getOperand(1).getImm() == 0 ||
                ctrlDef.getOperand(1).getImm() == 1)) {
              if (SCCNodes.size() > 5) {

              }
              lhdrPickNode = sccn;
            } else
              isIDVCandidate = false;
          }
        } else if ((TII->isAdd(minstr) ||                          //induction
          TII->isCommutingReductionTransform(minstr) ||  //reduction
          TII->isFMA(minstr) ||                          //reduction
          TII->isSub(minstr)) &&                         //reduction
          !addNode) {
          addNode = sccn;
        } else if (TII->isCmp(minstr) && !cmpNode) {
          cmpNode = sccn;
        } else if (TII->isSwitch(minstr) && !switchNode) {
          switchNode = sccn;
        } else if (TII->isMOV(minstr)) {
          //skip mov
        } else {
          isIDVCandidate = false;
        }
      }

      if (isIDVCandidate && cmpNode && switchNode && addNode && lhdrPickNode) {
        SequenceIndv(cmpNode, switchNode, addNode, lhdrPickNode);
      } else if (isIDVCandidate && switchNode && addNode && lhdrPickNode) {
        SequenceApp(switchNode, addNode, lhdrPickNode);
      } else if (isIDVCandidate && switchNode && lhdrPickNode) {
        SequenceRepeat(switchNode, lhdrPickNode);
      }
    }
  }
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
    MachineBasicBlock::iterator MI = BB->begin();
    while (MI != BB->end()) {
      MachineInstr* minstr = &*MI;
      ++MI;
      if (TII->isMOV(minstr) && minstr->getOperand(1).isReg() && 
          MRI->hasOneDef(minstr->getOperand(1).getReg()) && 
          TII->isSeqOT(MRI->getVRegDef(minstr->getOperand(1).getReg()))) {
        MachineInstr* seqOT = MRI->getVRegDef(minstr->getOperand(1).getReg());
        if (seqOT->getOperand(1).getReg() == minstr->getOperand(1).getReg()) {
          assert(!MRI->hasOneDef(minstr->getOperand(0).getReg()));
          MachineRegisterInfo::def_instr_iterator defI = MRI->def_instr_begin(minstr->getOperand(0).getReg());
          MachineRegisterInfo::def_instr_iterator defInext = std::next(defI);
          assert(std::next(defInext) == MachineRegisterInfo::def_instr_end());
          assert((TII->isMOV(&*defI) && TII->isInit(&*defInext)) || (TII->isInit(&*defI) && TII->isMOV(&*defInext)));
          MachineInstr* initInstr = TII->isInit(&*defI) ? &*defI : &*defInext;
          unsigned adjCmpReg = seqOT->getOperand(3).getReg();
          if (initInstr->getOperand(1).getImm() == 0) {
            //can't use seq_pred in non-compound instr, need to replace it with not_last
            unsigned notReg = LMFI->allocateLIC(&CSA::CI1RegClass);
            MachineInstr* notInstr = BuildMI(*minstr->getParent(),
              minstr, DebugLoc(), TII->get(CSA::NOT1),
              notReg).
              addReg(seqOT->getOperand(3).getReg()); //last_reg
            notInstr->setFlag(MachineInstr::NonSequential);
            adjCmpReg = notReg;
          }
          minstr->getOperand(1).setReg(adjCmpReg);
        }
      } else if (TII->isSwitch(minstr) && TII->isSeqOT(MRI->getVRegDef(minstr->getOperand(2).getReg()))) {
        MachineInstr* seqOT = MRI->getVRegDef(minstr->getOperand(2).getReg());
        if (seqOT->getOperand(1).getReg() == minstr->getOperand(2).getReg()) {
          SequenceSwitchOutLast(minstr, seqOT);
        }
      }
    }
  }
}
