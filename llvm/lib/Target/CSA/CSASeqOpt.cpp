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


CSASeqOpt::CSASeqOpt(MachineFunction *F) {
  thisMF = F;
  TII = static_cast<const CSAInstrInfo*>(thisMF->getSubtarget<CSASubtarget>().getInstrInfo());
  MRI = &thisMF->getRegInfo();
  LMFI = thisMF->getInfo<CSAMachineFunctionInfo>();
}

bool CSASeqOpt::isIntegerOpcode(unsigned opcode) {
  return TII->getOpcodeClass(opcode) == CSA::OpcodeClass::VARIANT_INT ||
    TII->getOpcodeClass(opcode) == CSA::OpcodeClass::VARIANT_SIGNED ||
    TII->getOpcodeClass(opcode) == CSA::OpcodeClass::VARIANT_UNSIGNED;
}

bool CSASeqOpt::repeatOpndInSameLoop(MachineOperand& opnd, MachineInstr* lpCmp) {
  MachineInstr* bndDef = MRI->getVRegDef(opnd.getReg());
  bool result = false;
  if (TII->getGenericOpcode(bndDef->getOpcode()) == CSA::Generic::REPEAT ||
    TII->getGenericOpcode(bndDef->getOpcode()) == CSA::Generic::REPEATO) {
    result = MRI->getVRegDef(bndDef->getOperand(1).getReg()) == lpCmp;
  }
  return result;
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
  } else {
    idvIdx = MRI->getVRegDef(cmpNode->minstr->getOperand(1).getReg()) == lhdrPickNode->minstr ||
             MRI->getVRegDef(cmpNode->minstr->getOperand(1).getReg()) == addNode->minstr ? 1 : 2;
  }
  isIDVCycle = isIDVCycle &&
               (MRI->getVRegDef(cmpNode->minstr->getOperand(idvIdx).getReg()) == lhdrPickNode->minstr ||
                MRI->getVRegDef(cmpNode->minstr->getOperand(idvIdx).getReg()) == addNode->minstr);

  MachineOperand& bndOpnd = cmpNode->minstr->getOperand(3 - idvIdx);
  //boundary and stride must be integer value or integer register defined outside the loop
  isIDVCycle = isIDVCycle && (bndOpnd.isImm() || repeatOpndInSameLoop(bndOpnd, cmpNode->minstr));
  //handle only |stride| == 1 for now
  unsigned strideIdx = addNode->minstr->getOperand(1).isReg() ? 2 : 1;
  MachineOperand& strideOpnd = addNode->minstr->getOperand(strideIdx);
  //isIDVCycle = isIDVCycle && (strideOpnd.isImm() && (strideOpnd.getImm() == 1 || strideOpnd.getImm() == -1));
  isIDVCycle = isIDVCycle && (strideOpnd.isImm());

  //uses of phi can only be add or cmp
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
  //uses of add can only be switch or cmp
  unsigned adddst = addNode->minstr->getOperand(0).getReg();
  UI = MRI->use_begin(adddst);
  while (UI != MRI->use_end()) {
    MachineOperand &UseMO = *UI;
    ++UI;
    MachineInstr *UseMI = UseMO.getParent();
    if (UseMI != switchNode->minstr && UseMI != cmpNode->minstr) {
      isIDVCycle = false;
      break;
    }
  }
  unsigned pickInitIdx = 2 + loopInit->getOperand(1).getImm();
  MachineOperand& initOpnd = lhdrPickNode->minstr->getOperand(pickInitIdx);
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
#if 1 
    //add is before cmp => need to adjust init value
    if (MRI->getVRegDef(cmpNode->minstr->getOperand(idvIdx).getReg()) == addNode->minstr) {
      if (initOpnd.isImm() && strideOpnd.isImm()) {
        int adjInit = addNode->minstr->getOperand(strideIdx).getImm() + initOpnd.getImm();
        initOpnd = MachineOperand::CreateImm(adjInit);
      } else {
        const TargetRegisterClass *TRC = TII->lookupLICRegClass(addNode->minstr->getOperand(0).getReg());
        unsigned adjInitVReg = LMFI->allocateLIC(TRC);
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
    MachineOperand cpyInit(initOpnd);
    cpyInit.setIsDef(false);
    unsigned firstReg = LMFI->allocateLIC(&CSA::CI1RegClass);
    unsigned lastReg = LMFI->allocateLIC(&CSA::CI1RegClass);
    MachineInstr* seqInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
      lhdrPickNode->minstr, DebugLoc(),
      TII->get(seqOp),
      lhdrPickNode->minstr->getOperand(0).getReg()).
      addReg(cmpNode->minstr->getOperand(0).getReg(), RegState::Define).  //pred
      addReg(firstReg, RegState::Define).
      addReg(lastReg, RegState::Define).
      add(cpyInit).                                                     //init
      add(bndOpnd).                                                     //boundary
      add(addNode->minstr->getOperand(strideIdx));                      //stride
    seqInstr->setFlag(MachineInstr::NonSequential);
    //handle last value live out of loop
    if (switchNode->minstr->getOperand(switchOutIndex).getReg() != CSA::IGN) {
      const TargetRegisterClass *TRC = TII->lookupLICRegClass(addNode->minstr->getOperand(0).getReg());
      unsigned lastV = LMFI->allocateLIC(TRC);
      MachineInstr* lastValueSwitch = BuildMI(*lhdrPickNode->minstr->getParent(),
        lhdrPickNode->minstr, DebugLoc(), 
          TII->get(TII->adjustOpcode(addNode->minstr->getOpcode(), CSA::Generic::SWITCH)),
          CSA::IGN).
        addReg(lastV).
        addReg(lastReg).
        addReg(lhdrPickNode->minstr->getOperand(0).getReg());
      lastValueSwitch->setFlag(MachineInstr::NonSequential);

      const unsigned addOp = TII->makeOpcode(CSA::Generic::ADD, TRC);
      MachineInstr* outbndInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
        lhdrPickNode->minstr,
        DebugLoc(),
        TII->get(addOp),
        switchNode->minstr->getOperand(switchOutIndex).getReg()).
        addReg(lastV).
        add(addNode->minstr->getOperand(strideIdx));
      outbndInstr->setFlag(MachineInstr::NonSequential);
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
    assert(vtripcnt >= 0);
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
      indvGenOp == CSA::Generic::SEQOTLT || indvGenOp == CSA::Generic::SEQOTLE || indvGenOp == CSA::Generic::SEQOTNE) {
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
  isIDVCycle = isIDVCycle && (loopInit != nullptr);
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
    assert(redOp == CSA::INVALID_OPCODE);
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
    }  else {
      //normal one input reduciton besides init
      redInstr = BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr, DebugLoc(),
        TII->get(redOp),
        switchOutReg).                          // result
        addReg(backedgeReg, RegState::Define).  // each 
        add(initOpnd).                         // initial value
        add(addNode->minstr->getOperand(1)).   // input 1
        addReg(seqOT->getOperand(1).getReg()); // control
    }
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
  bool phiuseOK = false;
  unsigned phidst = lhdrPickNode->minstr->getOperand(0).getReg();
  bool adduseOK = false;
  unsigned adddst = addNode->minstr->getOperand(0).getReg();
  //otherwise reduced to reduction
  assert(!MRI->hasOneUse(phidst) || !MRI->hasOneUse(adddst));
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

  if (isIDVCycle) {
    MachineOperand& initOpnd = lhdrPickNode->minstr->getOperand(2).getReg() == backedgeReg ?
      lhdrPickNode->minstr->getOperand(3) :
      lhdrPickNode->minstr->getOperand(2);
    const TargetRegisterClass *addTRC = TII->lookupLICRegClass(addNode->minstr->getOperand(0).getReg());
    unsigned seqReg;
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
    MachineOperand tripcnt = tripCntForSeq(seqIndv);
    //got a valid trip counter, convert to squence; otherwise stride
    if (!(tripcnt.isImm() && tripcnt.getImm() < 0))  { 
      const TargetRegisterClass *TRC = TII->lookupLICRegClass(addNode->minstr->getOperand(0).getReg());
      const unsigned FMAOp = TII->makeOpcode(CSA::Generic::FMA, TRC);
      unsigned fmaReg = LMFI->allocateLIC(addTRC);
      MachineInstr* fmaInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
        lhdrPickNode->minstr, DebugLoc(),
        TII->get(FMAOp),
        fmaReg).
        add(tripcnt).                                   //trip count from indv
        add(addNode->minstr->getOperand(strideIdx)).          //stride
        add(initOpnd);                                        //init
      fmaInstr->setFlag(MachineInstr::NonSequential);

      unsigned firstReg = LMFI->allocateLIC(&CSA::CI1RegClass);
      unsigned lastReg = LMFI->allocateLIC(&CSA::CI1RegClass);
      unsigned predReg = LMFI->allocateLIC(&CSA::CI1RegClass);
      MachineInstr* seqInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
        lhdrPickNode->minstr, DebugLoc(),
        TII->get(seqIndv->getOpcode()),
        seqReg).
        addReg(predReg, RegState::Define).                    //pred
        addReg(firstReg, RegState::Define).
        addReg(lastReg, RegState::Define).
        add(initOpnd).                                        //init
        addReg(fmaReg).                                       //boundary
        add(addNode->minstr->getOperand(strideIdx));          //stride
      seqInstr->setFlag(MachineInstr::NonSequential);
    } else {
      //can't figure out trip-counter; generate stride 
      const TargetRegisterClass *TRC = TII->lookupLICRegClass(addNode->minstr->getOperand(0).getReg());
      const unsigned strideOp = TII->makeOpcode(CSA::Generic::STRIDEO, TRC);
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

    //handle switch out value
    unsigned switchFalse = switchNode->minstr->getOperand(0).getReg();
    unsigned switchTrue = switchNode->minstr->getOperand(1).getReg();
    unsigned switchOut = switchFalse == backedgeReg ? switchTrue : switchFalse;
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

      const unsigned addOp = TII->makeOpcode(CSA::Generic::ADD, TRC);
      MachineInstr* outbndInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
        lhdrPickNode->minstr,
        DebugLoc(),
        TII->get(switchOp),
        switchOut).
        addReg(last).
        add(addNode->minstr->getOperand(strideIdx));
      outbndInstr->setFlag(MachineInstr::NonSequential);
    }
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
    if (TII->isMOV(&*defI) && TII->isInit(&*defInext) || TII->isInit(&*defI) && TII->isMOV(&*defInext)) {
      MachineInstr* initInstr = TII->isInit(&*defI) ? &*defI : &*defInext;
      MachineInstr* movInstr = TII->isMOV(&*defI) ? &*defI : &*defInext;
      MachineInstr* cmpInstr = MRI->getVRegDef(movInstr->getOperand(1).getReg());
      if ((TII->isCmp(cmpInstr) || lpcmpInstr && cmpInstr == lpcmpInstr) && initInstr->getOperand(1).isImm() && 
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
  //uses of phi can only be add or address computing
  bool switchuseOK = false;
  unsigned lpbackReg;
  MachineInstr* lpInit = lpInitForPickSwitchPair(lhdrPickNode->minstr, switchNode->minstr, lpbackReg);
  unsigned switchFalse = switchNode->minstr->getOperand(0).getReg();
  unsigned switchTrue = switchNode->minstr->getOperand(1).getReg();
  if (switchFalse == CSA::IGN && MRI->hasOneUse(switchTrue) && (switchTrue == lpbackReg) ||
      switchTrue == CSA::IGN && MRI->hasOneUse(switchFalse) && (switchFalse == lpbackReg)) {
    switchuseOK = true;
  }
  
  isIDVCycle = isIDVCycle && switchuseOK && (lpInit != nullptr);

  if (isIDVCycle) {
    unsigned predRepeat = switchNode->minstr->getOperand(2).getReg();
    if (switchFalse == lpbackReg) {
      unsigned notReg = LMFI->allocateLIC(&CSA::CI1RegClass);
      MachineInstr* notInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
        lhdrPickNode->minstr, DebugLoc(), TII->get(CSA::NOT1),
        notReg).
        addReg(switchNode->minstr->getOperand(2).getReg());
      notInstr->setFlag(MachineInstr::NonSequential);
      predRepeat = notReg;
    }
    unsigned valueRepeat = lhdrPickNode->minstr->getOperand(2).getReg() == lpbackReg ?
      lhdrPickNode->minstr->getOperand(3).getReg() :
      lhdrPickNode->minstr->getOperand(2).getReg();
    //TODO: FIX THE BUG OF MISSING REPEATO in CSA::Generic
    unsigned repeatOp = TII->adjustOpcode(switchNode->minstr->getOpcode(), CSA::Generic::REPEATO);
    assert(repeatOp != CSA::INVALID_OPCODE);
    MachineInstr* repeatInstr = BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr, DebugLoc(), TII->get(repeatOp),
      lhdrPickNode->minstr->getOperand(0).getReg()).
      addReg(predRepeat).
      addReg(valueRepeat); 
    repeatInstr->setFlag(MachineInstr::NonSequential);

    //remove the instructions in the IDV cycle.
    switchNode->minstr->removeFromParent();
    lhdrPickNode->minstr->removeFromParent();

  }
}


void CSASeqOpt::SequenceOPT() {
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
        } else if (TII->isCmp(minstr) && !cmpNode && (nodeI + 1 != nodeIE)) { //cmp can't be the first or last
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
}
