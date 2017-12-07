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
bool CSASeqOpt::isIntegerReg(unsigned Reg) {
  const TargetRegisterClass *TRC = MRI->getRegClass(Reg);
  if (TRC->getID() == CSA::I0RegClassID ||
    TRC->getID() == CSA::I1RegClassID ||
    TRC->getID() == CSA::I8RegClassID ||
    TRC->getID() == CSA::I16RegClassID ||
    TRC->getID() == CSA::I32RegClassID ||
    TRC->getID() == CSA::I64RegClassID) {
    return true;
  }
  return false;
}


void CSASeqOpt::SequenceIndv(CSASSANode* cmpNode, CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPickNode) {
  //addNode has inputs phiNode, and an immediate input
  bool isIDVCycle = addNode->children.size() == 1 && addNode->children[0] == lhdrPickNode;
  
  //switchNode has inputs mov->cmpNode, addNode
  isIDVCycle = isIDVCycle && MRI->getVRegDef(switchNode->minstr->getOperand(3).getReg()) == addNode->minstr;
  unsigned backedgeReg = 0;
  MachineInstr* loopInit = lpInitForPickSwitchPair(lhdrPickNode->minstr, switchNode->minstr, backedgeReg, cmpNode->minstr);
  isIDVCycle = isIDVCycle && (loopInit != nullptr);
  unsigned idvIdx;
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

  assert(CSA::CMPNE16 < CSA::CMPNE32 && CSA::CMPNE32 < CSA::CMPNE64 && CSA::CMPNE64 < CSA::CMPNE8);
  isIDVCycle = isIDVCycle && cmpNode->minstr->getOpcode() >= CSA::CMPNE16 && cmpNode->minstr->getOpcode() <= CSA::CMPNE8;
  MachineOperand& bndOpnd = cmpNode->minstr->getOperand(3 - idvIdx);
  //boundary must be integer value or integer register defined outside the loop
  bool isBndRepeated = false;
  if (bndOpnd.isReg() && isIntegerReg(bndOpnd.getReg())) {
    MachineInstr* bndPick = MRI->getVRegDef(bndOpnd.getReg());
    if (TII->isPick(bndPick) &&
        MRI->hasOneUse(bndPick->getOperand(0).getReg())) {
      MachineInstr* bndSwitch = MRI->use_begin(bndPick->getOperand(0).getReg())->getParent();
      unsigned lpbkIdx = 1 - loopInit->getOperand(0).getImm();
      if (MRI->hasOneUse(bndSwitch->getOperand(lpbkIdx).getReg()) &&
          MRI->use_begin(bndSwitch->getOperand(lpbkIdx).getReg())->getParent() == bndPick && 
          MRI->use_empty(bndSwitch->getOperand(1-lpbkIdx).getReg())) {
        isBndRepeated = bndPick->getOperand(1).getReg() == loopInit->getOperand(0).getReg() &&
                        bndSwitch->getOperand(2).getReg() == loopInit->getOperand(0).getReg();
      }
    }
  }

  isIDVCycle = isIDVCycle && (isBndRepeated || bndOpnd.isImm());
  //handle only |stride| == 1 for now
  unsigned strideIdx = addNode->minstr->getOperand(1).isReg() ? 2 : 1;
  isIDVCycle = isIDVCycle && addNode->minstr->getOperand(strideIdx).isImm() &&
    (addNode->minstr->getOperand(strideIdx).getImm() == 1 ||
      addNode->minstr->getOperand(strideIdx).getImm() == -1);
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
  MachineOperand& bundOpnd = cmpNode->minstr->getOperand(3 - idvIdx);
  //init and boundary, one of them has to be 0.
  isIDVCycle = isIDVCycle && ((initOpnd.isImm() && initOpnd.getImm() == 0) || (bundOpnd.isImm() && bundOpnd.getImm() == 0));
  if (isIDVCycle) {
    unsigned compareSense = idvIdx - 1;
    //1 means: false control sig loop back, true control sig exit loop
    unsigned switchSense = loopInit->getOperand(1).getImm();
    unsigned switchOutIndex = switchNode->minstr->getOperand(0).getReg() == lhdrPickNode->minstr->getOperand(5-pickInitIdx).getReg() ? 1 : 0;

    //no use of switch outside the loop, only use is lhdrphi
    if ((switchNode->minstr->getOperand(switchOutIndex).getReg() == CSA::IGN || 
         MRI->use_empty(switchNode->minstr->getOperand(switchOutIndex).getReg())) &&
        MRI->hasOneUse(switchNode->minstr->getOperand(1 - switchOutIndex).getReg())) {

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
        if (initOpnd.isImm()) {
          int adjInit = addNode->minstr->getOperand(strideIdx).getImm() + initOpnd.getImm();
          initOpnd = MachineOperand::CreateImm(adjInit);
        } else {
          const TargetRegisterClass *TRC = TII->lookupLICRegClass(addNode->minstr->getOperand(0).getReg());
          unsigned adjInitVReg = LMFI->allocateLIC(TRC);
          MachineInstr* adjInitInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
            lhdrPickNode->minstr, DebugLoc(),
            TII->get(addNode->minstr->getOpcode()),
            adjInitVReg).
            addImm(addNode->minstr->getOperand(strideIdx).getImm()).                      //stride
            addReg(initOpnd.getReg());                                                      //init
          adjInitInstr->setFlag(MachineInstr::NonSequential);
          initOpnd = adjInitInstr->getOperand(0);
        }
      }
#endif 
      MachineOperand cpyInit(initOpnd);
      cpyInit.setIsDef(false);
      unsigned firstReg = LMFI->allocateLIC(&CSA:: CI1RegClass);
      unsigned lastReg = LMFI->allocateLIC(&CSA::CI1RegClass);
      MachineInstr* seqInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
        lhdrPickNode->minstr, DebugLoc(),
        TII->get(seqOp),
        lhdrPickNode->minstr->getOperand(0).getReg()).
        addReg(cmpNode->minstr->getOperand(0).getReg(), RegState::Define).  //pred
        addReg(firstReg, RegState::Define).
        addReg(lastReg, RegState::Define).
        add(cpyInit).                                                     //init
        add(cmpNode->minstr->getOperand(3 - idvIdx)).                       //boundary
        add(addNode->minstr->getOperand(strideIdx));                      //stride
      seqInstr->setFlag(MachineInstr::NonSequential);
      //remove the instructions in the IDV cycle.
      cmpNode->minstr->removeFromParent();
      switchNode->minstr->removeFromParent();
      addNode->minstr->removeFromBundle();
      lhdrPickNode->minstr->removeFromParent();
      //currently only the cmp instr can have usage outside the cycle
      cmpNode->minstr = seqInstr;
    }
  }
}


void CSASeqOpt::SequenceAddress(CSASSANode* switchNode, CSASSANode* addNode, CSASSANode* lhdrPickNode) {
  //addNode has inputs phiNode, and an immediate input
  bool isIDVCycle = addNode->children.size() == 1 && addNode->children[0] == lhdrPickNode;
  //switchNode has inputs addNode, and switch's control is SeqOT
  isIDVCycle = isIDVCycle &&
    TII->isSeqOT(MRI->getVRegDef(switchNode->minstr->getOperand(2).getReg())) &&
    MRI->getVRegDef(switchNode->minstr->getOperand(3).getReg()) == addNode->minstr;

  //handle only constant stride for now
  unsigned strideIdx = addNode->minstr->getOperand(1).isReg() ? 2 : 1;
  isIDVCycle = isIDVCycle &&
               addNode->minstr->getOperand(strideIdx).isImm() &&
               //TODO: handle decreemntal address computing
               addNode->minstr->getOperand(strideIdx).getImm() > 0;
  //uses of phi can only be add or address computing
  bool phiuseOK = false;
  unsigned phidst = lhdrPickNode->minstr->getOperand(0).getReg();
  if (MRI->hasOneUse(phidst)) {
    phiuseOK = true;
  } else {
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

  bool adduseOK = false;
  unsigned adddst = addNode->minstr->getOperand(0).getReg();
  if (MRI->hasOneUse(adddst)) {
    adduseOK = true;
  } else {
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
  isIDVCycle = isIDVCycle && phiuseOK && adduseOK;

  bool switchuseOK = false;
  unsigned switchBackReg;
  unsigned switchFalse = switchNode->minstr->getOperand(0).getReg();
  unsigned switchTrue = switchNode->minstr->getOperand(1).getReg();
  if (switchFalse == CSA::IGN && MRI->hasOneUse(switchTrue) &&
      &*MRI->use_instr_begin(switchTrue) == lhdrPickNode->minstr) {
    switchBackReg = switchFalse;
    switchuseOK = true;
  } else if (switchTrue == CSA::IGN && MRI->hasOneUse(switchFalse) &&
             &*MRI->use_instr_begin(switchFalse) == lhdrPickNode->minstr) {
    switchBackReg = switchTrue;
    switchuseOK = true;
  } 
  
  isIDVCycle = isIDVCycle && switchuseOK;

  if (isIDVCycle) {
    MachineInstr* seqIndv = MRI->getVRegDef(switchNode->minstr->getOperand(2).getReg());
    assert(TII->isSeqOT(seqIndv));
    MachineOperand& initIndv = seqIndv->getOperand(4);
    MachineOperand& bndIndv = seqIndv->getOperand(5);
    MachineOperand& stridIndv = seqIndv->getOperand(6);
    assert(stridIndv.isImm() && (stridIndv.getImm() == 1 || stridIndv.getImm() == -1));
    unsigned seqReg;
    MachineOperand& initOpnd = lhdrPickNode->minstr->getOperand(2).getReg() == switchBackReg ?
                       lhdrPickNode->minstr->getOperand(3) :
                       lhdrPickNode->minstr->getOperand(2);
    const TargetRegisterClass *addTRC = TII->lookupLICRegClass(addNode->minstr->getOperand(0).getReg());
#if 1
    if (!MRI->hasOneUse(phidst)) {
      assert(MRI->hasOneUse(adddst));
      seqReg = phidst;
    } else {
      assert(!MRI->hasOneUse(adddst));
      seqReg = adddst;
      //adjust init
      if (initOpnd.isImm()) {
        int adjInit = initOpnd.getImm() + addNode->minstr->getOperand(strideIdx).getImm();
        initOpnd = MachineOperand::CreateImm(adjInit);
      } else {
        unsigned adjInitVReg = LMFI->allocateLIC(addTRC);
        MachineInstr* adjInitInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
          lhdrPickNode->minstr, DebugLoc(),
          TII->get(addNode->minstr->getOpcode()),
          adjInitVReg).
          addImm(addNode->minstr->getOperand(strideIdx).getImm()).                      //stride
          add(initOpnd);                                                      //init
        adjInitInstr->setFlag(MachineInstr::NonSequential);
        initOpnd = adjInitInstr->getOperand(0);
      }
    }
#endif 
    //TODO: only compute trip-counter from NE comparison.
    assert(CSA::SEQOTNE16 < CSA::SEQOTNE32 && CSA::SEQOTNE32 < CSA::SEQOTNE64 && CSA::SEQOTNE64 < CSA::SEQOTNE8);
    assert(seqIndv->getOpcode() >= CSA::SEQOTNE16 && seqIndv->getOpcode() <= CSA::SEQOTNE8);
    const TargetRegisterClass *TRC = TII->lookupLICRegClass(addNode->minstr->getOperand(0).getReg());
    const unsigned FMAOp = TII->makeOpcode(CSA::Generic::FMA, TRC);
    MachineOperand& tripcountOpnd = initIndv.isImm() ? bndIndv : initIndv;
    unsigned fmaReg = LMFI->allocateLIC(addTRC);
    MachineInstr* fmaInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
      lhdrPickNode->minstr, DebugLoc(),
      TII->get(FMAOp),
      fmaReg).
      add(tripcountOpnd).                                   //trip count from indv
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
      if (TII->isCmp(cmpInstr) && initInstr->getOperand(1).isImm() && 
          (initInstr->getOperand(1).getImm() == 1 || initInstr->getOperand(1).getImm() == 0)) {
        backedgeReg = initInstr->getOperand(1).getImm() == 0 ?
                      pickInstr->getOperand(3).getReg() :
                      pickInstr->getOperand(2).getReg();
        if (MRI->getVRegDef(backedgeReg) == switchInstr && MRI->getVRegDef(switchInstr->getOperand(2).getReg()) == cmpInstr) {
          result = initInstr;
          if (lpcmpInstr && cmpInstr != lpcmpInstr) result = nullptr;
        }
      }
    }
  }
  return result;
}




#if 1
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
    //unsigned repeatOp = TII->adjustOpcode(switchNode->minstr->getOpcode(), CSA::Generic::REPEATO)
    unsigned repeatOp = TII->adjustOpcode(switchNode->minstr->getOpcode(), CSA::Generic::REPEAT);
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
#endif


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
        } else if (TII->isAdd(minstr) &&     //TODO: handle sub
                   TII->adjustOpcode(minstr->getOpcode(), CSA::Generic::STRIDE) != CSA::INVALID_OPCODE && 
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
        SequenceAddress(switchNode, addNode, lhdrPickNode);
      } else if (isIDVCandidate && switchNode && lhdrPickNode) {
        SequenceRepeat(switchNode, lhdrPickNode);
      }
    }
  }
}
