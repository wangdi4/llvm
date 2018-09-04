//==--- CSASeqOpt.cpp - Sequence operator optimization --==//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
#include "CSASeqOpt.h"
#include "CSAInstrInfo.h"
#include "CSASequenceOpt.h"
#include "CSATargetMachine.h"
#include "MachineCDG.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
namespace {
  //128 is what simulator uses now.
  //32 is the experimental number
  static const unsigned  SimLicMaxDepth = 128;
  static const unsigned  DefaultSeqLicDepth = 32;
}

using namespace llvm;

static cl::opt<bool> DisableMultiSeq(
  "csa-disable-multiseq", cl::Hidden,
  cl::desc("CSA Specific: Disable multiple sequence conversion"));

static cl::opt<bool> EnableBuffer(
  "csa-enable-buffer", cl::Hidden, cl::init(false),
  cl::desc("CSA Specific: Add buffering to loop LICs"));

CSASeqOpt::CSASeqOpt(MachineFunction *F, MachineOptimizationRemarkEmitter &ORE,
                     const char *PassName) :
  ORE(ORE), PassName(PassName) {
  thisMF = F;
  TII    = static_cast<const CSAInstrInfo *>(
    thisMF->getSubtarget<CSASubtarget>().getInstrInfo());
  MRI  = &thisMF->getRegInfo();
  LMFI = thisMF->getInfo<CSAMachineFunctionInfo>();
  TRI  = thisMF->getSubtarget<CSASubtarget>().getRegisterInfo();
}

bool CSASeqOpt::isIntegerOpcode(unsigned opcode) {
  return TII->getOpcodeClass(opcode) == CSA::OpcodeClass::VARIANT_INT ||
         TII->getOpcodeClass(opcode) == CSA::OpcodeClass::VARIANT_SIGNED ||
         TII->getOpcodeClass(opcode) == CSA::OpcodeClass::VARIANT_UNSIGNED;
}

MachineInstr *CSASeqOpt::repeatOpndInSameLoop(MachineOperand &opnd,
                                              MachineInstr *lpCmp) {
  MachineInstr *rptInstr = MRI->getVRegDef(opnd.getReg());
  if (TII->getGenericOpcode(rptInstr->getOpcode()) == CSA::Generic::REPEATO) {
    unsigned rptCtrl = rptInstr->getOperand(1).getReg();
    while (MRI->getVRegDef(rptCtrl)->getOpcode() == CSA::MOV1)
      rptCtrl = MRI->getVRegDef(rptCtrl)->getOperand(1).getReg();
    if (MRI->getVRegDef(rptCtrl) == lpCmp) {
      return rptInstr;
    } else if (MRI->getVRegDef(rptInstr->getOperand(1).getReg())->getOpcode() ==
               CSA::NOT1) {
      MachineInstr *notInstr =
        MRI->getVRegDef(rptInstr->getOperand(1).getReg());
      unsigned notSrc = notInstr->getOperand(1).getReg();
      while (MRI->getVRegDef(notSrc)->getOpcode() == CSA::MOV1)
        notSrc = MRI->getVRegDef(notSrc)->getOperand(1).getReg();
      if (MRI->getVRegDef(notSrc) == lpCmp) {
        return rptInstr;
      }
    }
  }
  return nullptr;
}

void CSASeqOpt::FoldRptInit(MachineInstr *rptInstr) {
  assert(MRI->hasOneNonDBGUse(rptInstr->getOperand(0).getReg()));
  MachineRegisterInfo::use_iterator UI =
    MRI->use_begin(rptInstr->getOperand(0).getReg());
  MachineOperand &UseMO = *UI;
  MachineInstr *rptPick = UseMO.getParent();
  assert(TII->isPick(rptPick));
  unsigned pickDst = rptPick->getOperand(0).getReg();
  rptPick->removeFromParent();
  MRI->markUsesInDebugValueAsUndef(rptInstr->getOperand(0).getReg());
  rptInstr->getOperand(0).setReg(pickDst);
}

void CSASeqOpt::SequenceIndv(CSASSANode *cmpNode, CSASSANode *switchNode,
                             CSASSANode *addNode, CSASSANode *lhdrPickNode) {
  // addNode has inputs phiNode, and an immediate input
  bool isIDVCycle = TII->isAdd(addNode->minstr) &&
                    isIntegerOpcode(addNode->minstr->getOpcode()) &&
                    addNode->children[0] == lhdrPickNode;
  // switchNode has inputs mov->cmpNode, addNode
  isIDVCycle =
    isIDVCycle && MRI->getVRegDef(switchNode->minstr->getOperand(3).getReg()) ==
                    addNode->minstr;
  unsigned backedgeReg   = 0;
  MachineInstr *loopInit = lpInitForPickSwitchPair(
    lhdrPickNode->minstr, switchNode->minstr, backedgeReg, cmpNode->minstr);
  isIDVCycle      = isIDVCycle && (loopInit != nullptr);
  unsigned idvIdx = 0;
  // cmpNode has inputs either pickNode or addNode
  if (cmpNode->children.size() == 1) {
    // cmp with immediate
    idvIdx = cmpNode->minstr->getOperand(1).isReg() ? 1 : 2;
  } else if (repeatOpndInSameLoop(cmpNode->minstr->getOperand(1),
                                  cmpNode->minstr)) {
    idvIdx = 2;
  } else if (repeatOpndInSameLoop(cmpNode->minstr->getOperand(2),
                                  cmpNode->minstr)) {
    idvIdx = 1;
  } else {
    return;
  }

  // CMPLRS-50091: we cannot mix differently sized values in one
  // sequence instruction.
  if (TII->getLicSize(cmpNode->minstr->getOpcode()) !=
      TII->getLicSize(addNode->minstr->getOpcode())) {
    MachineOptimizationRemarkMissed Remark(PassName, "CSASeqOptMissed: ",
                                           cmpNode->minstr->getDebugLoc(),
                                           cmpNode->minstr->getParent());
    ORE.emit(Remark << " bounded sequence cannot be generated; "
             "compare and increment have different size");
    return;
  }

  isIDVCycle = isIDVCycle &&
               (MRI->getVRegDef(cmpNode->minstr->getOperand(idvIdx).getReg()) ==
                  lhdrPickNode->minstr ||
                MRI->getVRegDef(cmpNode->minstr->getOperand(idvIdx).getReg()) ==
                  addNode->minstr);

  MachineOperand &bndOpnd = cmpNode->minstr->getOperand(3 - idvIdx);
  MachineInstr *rptBnd    = nullptr;
  MachineInstr *rptStride = nullptr;
  // boundary and stride must be integer value or integer register defined
  // outside the loop
  isIDVCycle =
    isIDVCycle && (bndOpnd.isImm() ||
                   (rptBnd = repeatOpndInSameLoop(bndOpnd, cmpNode->minstr)));
  // handle only |stride| == 1 for now
  unsigned strideIdx         = addNode->minstr->getOperand(1).isReg() ? 2 : 1;
  MachineOperand &strideOpnd = addNode->minstr->getOperand(strideIdx);
  // isIDVCycle = isIDVCycle && (strideOpnd.isImm() && (strideOpnd.getImm() == 1
  // || strideOpnd.getImm() == -1));
  isIDVCycle =
    isIDVCycle &&
    (strideOpnd.isImm() ||
     (rptStride = repeatOpndInSameLoop(strideOpnd, cmpNode->minstr)));

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

  // new seq instr uses phi's dst as its value channel -- no need to consider
  // phi dst's usage  uses of add can only be switch or cmp
  unsigned adddst = addNode->minstr->getOperand(0).getReg();
  MachineRegisterInfo::use_iterator UI = MRI->use_begin(adddst);
  bool hasPostIncUses = false;
  while (UI != MRI->use_end()) {
    MachineOperand &UseMO = *UI;
    ++UI;
    MachineInstr *UseMI = UseMO.getParent();
    if (UseMI != switchNode->minstr && UseMI != cmpNode->minstr) {
      hasPostIncUses = true;
      break;
    }
  }

  if (cmpNode->minstr->getOperand(idvIdx).getReg() !=
      switchNode->minstr->getOperand(3).getReg()) {
    isIDVCycle = false;
  }

  unsigned pickInitIdx     = 2 + loopInit->getOperand(1).getImm();
  MachineOperand *initOpnd = &lhdrPickNode->minstr->getOperand(pickInitIdx);
  unsigned switchOutIndex =
    switchNode->minstr->getOperand(0).getReg() == backedgeReg ? 1 : 0;
  isIDVCycle = isIDVCycle &&
               (switchNode->minstr->getOperand(1 - switchOutIndex).getReg() ==
                backedgeReg) &&
               MRI->hasOneNonDBGUse(backedgeReg);
  if (isIDVCycle) {
    unsigned compareSense = idvIdx - 1;
    // 1 means: false control sig loop back, true control sig exit loop
    unsigned switchSense = loopInit->getOperand(1).getImm();
    unsigned switchOutIndex =
      switchNode->minstr->getOperand(0).getReg() == backedgeReg ? 1 : 0;
    assert(switchNode->minstr->getOperand(1 - switchOutIndex).getReg() ==
           backedgeReg);
    (void) switchOutIndex;
    // no use of switch outside the loop, only use is lhdrphi
    // Find a sequence opcode that matches our compare opcode.
    unsigned seqOp;
    if (!CSASeqLoopInfo::compute_matching_seq_opcode(
          cmpNode->minstr->getOpcode(), addNode->minstr->getOpcode(),
          compareSense, switchSense, *TII, &seqOp)) {
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
    cpyInit.clearParent();
    cpyInit.setIsDef(false);
    unsigned firstReg = LMFI->allocateLIC(&CSA::CI1RegClass);
    unsigned lastReg  = LMFI->allocateLIC(&CSA::CI1RegClass);
    unsigned seqPred  = LMFI->allocateLIC(&CSA::CI1RegClass);
    unsigned seqReg   = lhdrPickNode->minstr->getOperand(0).getReg();
    MachineOperand &indvBnd =
      bndOpnd.isReg() && rptBnd ? rptBnd->getOperand(2) : bndOpnd;
    MachineOperand &indvStride =
      strideOpnd.isReg() && rptStride ? rptStride->getOperand(2) : strideOpnd;
    unsigned cmpReg = cmpNode->minstr->getOperand(0).getReg();
    MachineInstr *seqInstr =
      BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr,
              DebugLoc(), TII->get(seqOp), seqReg)
        .addReg(seqPred, RegState::Define)
        . // pred
      addReg(firstReg, RegState::Define)
        .addReg(lastReg, RegState::Define)
        .add(cpyInit)
        . // init
      add(indvBnd)
        .              // boundary
      add(indvStride); // stride
    seqInstr->setFlag(MachineInstr::NonSequential);
    unsigned licDepth = GetSeqIndvLicDepth(seqInstr);
    //set lic depth for sequence instrution, since it is the driver
    //instruction for all other instructions in the SCC
    SetSeqLicDepth(seqReg, licDepth);
    SetSeqLicDepth(seqPred, licDepth);
    SetSeqLicDepth(firstReg, licDepth);
    SetSeqLicDepth(lastReg, licDepth);

    // If the switcher is expecting 1, 1, 1, ... 1, 0, then
    // loop.header.switcherSense should be 0, and we want a NOT1 to
    // convert from "last" to the "switcher" channel.
    //
    // Otherwise, the switcher is expecting 0, 0, 0, ... 0, 1,
    // switcherSense is 1, and should just use a "MOV1" instead.
    unsigned adjCtrlOp =
      loopInit->getOperand(1).getImm() == 1 ? CSA::MOV1 : CSA::NOT1;
    MachineInstr *adjCmpInstr =
      BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr,
              DebugLoc(), TII->get(adjCtrlOp), cmpReg)
        .addReg(lastReg);
    adjCmpInstr->setFlag(MachineInstr::NonSequential);
    SequenceSwitchOut(switchNode, addNode, lhdrPickNode, seqInstr, seqReg,
                      backedgeReg);

    // If the add has uses, replace it with an add of the induction variable
    // from the switch.
    if (hasPostIncUses) {
      // if strideIdx == 2, set operand 1 else (it == 1) set operand 2.
      addNode->minstr->getOperand(3 - strideIdx).setReg(seqReg);
    } else {
      addNode->minstr->removeFromBundle();
    }

    MRI->markUsesInDebugValueAsUndef(backedgeReg);
    switchNode->minstr->removeFromParent();
    lhdrPickNode->minstr->removeFromParent();
    cmpNode->minstr->removeFromParent();
    // currently only the cmp instr can have usage outside the cycle
    cmpNode->minstr = seqInstr;

    // Assign LIC groups for the new registers. first and last execute as many
    // times as the value does, while pred executes a little more frequently.
    auto loopGroup = LMFI->getLICGroup(seqReg);
    LMFI->setLICGroup(firstReg, loopGroup);
    LMFI->setLICGroup(lastReg, loopGroup);
    LMFI->setLICGroup(seqPred, getLoopPredicate(lhdrPickNode));
  }
}

MachineOperand CSASeqOpt::CalculateTripCnt(MachineOperand &initOpnd,
                                           MachineOperand &bndOpnd,
                                           MachineInstr *pos) {
  if (initOpnd.isImm() && bndOpnd.isImm()) {
    unsigned vtripcnt = bndOpnd.getImm() - initOpnd.getImm();
    return MachineOperand::CreateImm(vtripcnt);
  } else if (initOpnd.isImm() && initOpnd.getImm() == 0) {
    return bndOpnd;
  } else {
    // boundry - init
    MachineOperand &regOpnd = bndOpnd.isImm() ? initOpnd : bndOpnd;
    const TargetRegisterClass *TRC =
      TII->getRegisterClass(regOpnd.getReg(), *MRI);
    unsigned regTripcnt = LMFI->allocateLIC(TRC);
    unsigned subOp      = TII->makeOpcode(CSA::Generic::SUB, TRC);
    MachineInstr *tripcntInstr =
      BuildMI(*pos->getParent(), pos, DebugLoc(), TII->get(subOp), regTripcnt)
        .add(bndOpnd)
        .add(initOpnd);
    tripcntInstr->setFlag(MachineInstr::NonSequential);
    return tripcntInstr->getOperand(0);
  }
}
MachineOperand CSASeqOpt::getTripCntForSeq(MachineInstr *seqInstr,
                                           MachineInstr *pos) {
  if (seq2tripcnt.find(seqInstr) != seq2tripcnt.end())
    return *seq2tripcnt[seqInstr];
  else {
    MachineOperand tripcnt = tripCntForSeq(seqInstr, pos);
    tripcnt.clearParent();
    MachineOperand *cpyTripcnt = new MachineOperand(tripcnt);
    seq2tripcnt[seqInstr]      = cpyTripcnt;
    return tripcnt;
  }
}

MachineOperand CSASeqOpt::tripCntForSeq(MachineInstr *seqIndv,
                                        MachineInstr *pos) {
  //
  // TODO (vzakhari 3/7/2018): correctness of this transformation
  //       is questionable, because we cannot compute the tripcount
  //       by subtracting the init and bound values due to possible
  //       overflows.  Probably we can rely on loop canonicalization
  //       and assume that *good* loops come to us with 0 init value.
  //
  assert(TII->isSeqOT(seqIndv));
  MachineOperand &initIndv   = seqIndv->getOperand(4);
  MachineOperand &bndIndv    = seqIndv->getOperand(5);
  MachineOperand &stridIndv  = seqIndv->getOperand(6);
  MachineOperand tripcntOpnd = MachineOperand::CreateImm(-2);
  if (stridIndv.isImm() &&
      (stridIndv.getImm() == 1 || stridIndv.getImm() == -1)) {
    // TODO: only compute trip-counter when step is 1
    CSA::Generic indvGenOp = TII->getGenericOpcode(seqIndv->getOpcode());
    if (stridIndv.getImm() == -1 && (indvGenOp == CSA::Generic::SEQOTGT ||
                                     indvGenOp == CSA::Generic::SEQOTGE ||
                                     indvGenOp == CSA::Generic::SEQOTNE)) {
      // trip counter = init - boundary
      tripcntOpnd = CalculateTripCnt(bndIndv, initIndv, pos);
    } else if (stridIndv.getImm() == 1 &&
               (indvGenOp == CSA::Generic::SEQOTLT ||
                indvGenOp == CSA::Generic::SEQOTLE ||
                indvGenOp == CSA::Generic::SEQOTNE)) {
      // trip counter = boundary - init
      tripcntOpnd = CalculateTripCnt(initIndv, bndIndv, pos);
    }
    // adjust trip counter for equal comparison
    if (indvGenOp == CSA::Generic::SEQOTGE ||
        indvGenOp == CSA::Generic::SEQOTLE) {
      //
      // CMPLRS-49117: stridIndv is a literal constant, so we compute
      //               the register class for ADD operation using
      //               the VALUE result of the SEQ operation.
      //
      const auto &valueOperandReg = seqIndv->getOperand(0).getReg();
      const TargetRegisterClass *TRC =
        TII->getRegisterClass(valueOperandReg, *MRI);
      unsigned regTripcnt = LMFI->allocateLIC(TRC);
      MachineInstr *tripcntInstr =
        BuildMI(
          *seqIndv->getParent(), seqIndv, DebugLoc(),
          TII->get(TII->adjustOpcode(seqIndv->getOpcode(),
                                     CSA::Generic::ADD,
                                     // Use CSA::VARIANT_INT for the ADD
                                     // explicitly, because adjustOpcode will
                                     // fail to find unsigned variant of ADD
                                     // otherwise.
                                     CSA::VARIANT_INT)),
          regTripcnt)
          .add(tripcntOpnd)
          .addImm(1);
      tripcntInstr->setFlag(MachineInstr::NonSequential);
      tripcntOpnd = tripcntInstr->getOperand(0);
    } else {
      // no adjust needed for CSA::Generic::SEQOTGT || CSA::Generic::SEQOTLT ||
      // CSA::Generic::SEQOTNE
    }
  }
  return tripcntOpnd;
}

MachineInstr *CSASeqOpt::getSeqOTDef(MachineOperand &opnd) {
  if (!MRI->hasOneDef(opnd.getReg()))
    return nullptr;
  MachineInstr *defInstr = MRI->getVRegDef(opnd.getReg());
  if (TII->isSeqOT(defInstr))
    return defInstr;
  else if (defInstr->getOpcode() == CSA::NOT1 ||
           defInstr->getOpcode() == CSA::MOV1) {
    MachineInstr *srcDef = MRI->getVRegDef(defInstr->getOperand(1).getReg());
    if (TII->isSeqOT(srcDef)) {
      return srcDef;
    }
  }
  return nullptr;
}

void CSASeqOpt::SequenceApp(CSASSANode *switchNode, CSASSANode *addNode,
                            CSASSANode *lhdrPickNode) {
  unsigned phidst = lhdrPickNode->minstr->getOperand(0).getReg();
  unsigned adddst = addNode->minstr->getOperand(0).getReg();
  if (MRI->hasOneNonDBGUse(phidst) && MRI->hasOneNonDBGUse(adddst)) {
    SequenceReduction(switchNode, addNode, lhdrPickNode);
  } else {
    MultiSequence(switchNode, addNode, lhdrPickNode);
  }
}

void CSASeqOpt::SequenceReduction(CSASSANode *switchNode, CSASSANode *addNode,
                                  CSASSANode *lhdrPickNode) {
  // switchNode has inputs addNode, and switch's control is SeqOT
  MachineInstr *seqOT = getSeqOTDef(switchNode->minstr->getOperand(2));
  bool isIDVCycle =
    seqOT && MRI->getVRegDef(switchNode->minstr->getOperand(3).getReg()) ==
               addNode->minstr;
  unsigned backedgeReg = 0;
  unsigned idvIdx, strideIdx;
  MachineInstr *loopInit = lpInitForPickSwitchPair(
    lhdrPickNode->minstr, switchNode->minstr, backedgeReg, seqOT);
  if (TII->isFMA(addNode->minstr)) {
    assert(MRI->getVRegDef(addNode->minstr->getOperand(3).getReg()) ==
           lhdrPickNode->minstr);
    idvIdx    = 3;
    strideIdx = 100;
  } else {
    idvIdx = MRI->getVRegDef(addNode->minstr->getOperand(1).getReg()) ==
                 lhdrPickNode->minstr
               ? 1
               : 2;
    strideIdx = 3 - idvIdx;
  }
  isIDVCycle = isIDVCycle && (loopInit != nullptr) &&
               MRI->getVRegDef(addNode->minstr->getOperand(idvIdx).getReg()) ==
                 lhdrPickNode->minstr;
  if (!isIDVCycle)
    return;

  unsigned pickInitIdx     = 2 + loopInit->getOperand(1).getImm();
  MachineOperand &initOpnd = lhdrPickNode->minstr->getOperand(pickInitIdx);
  unsigned switchOutIndex =
    switchNode->minstr->getOperand(0).getReg() == backedgeReg ? 1 : 0;
  unsigned switchOutReg =
    switchNode->minstr->getOperand(switchOutIndex).getReg();
  isIDVCycle = isIDVCycle &&
               (switchNode->minstr->getOperand(1 - switchOutIndex).getReg() ==
                backedgeReg) &&
               MRI->hasOneNonDBGUse(backedgeReg);

  if (isIDVCycle) {
    // build reduction seqeuence.
    unsigned redOp =
      TII->convertTransformToReductionOp(addNode->minstr->getOpcode());
    // 64bit operations such as ADDF64 are not supported
    // if (redOp != CSA::INVALID_OPCODE)
    assert(redOp != CSA::INVALID_OPCODE);
    MachineInstr *redInstr;
    if (TII->isFMA(addNode->minstr)) {
      // two input reduction besides init
      redInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
                         lhdrPickNode->minstr, DebugLoc(), TII->get(redOp),
                         switchOutReg)
                   . // result
                 addReg(backedgeReg, RegState::Define)
                   . // each
                 add(initOpnd)
                   . // initial value
                 add(addNode->minstr->getOperand(1))
                   . // input 1
                 add(addNode->minstr->getOperand(2))
                   .                                    // input 2
                 addReg(seqOT->getOperand(1).getReg()); // control
    } else {
      // normal one input reduciton besides init
      redInstr = BuildMI(*lhdrPickNode->minstr->getParent(),
                         lhdrPickNode->minstr, DebugLoc(), TII->get(redOp),
                         switchOutReg)
                   . // result
                 addReg(backedgeReg, RegState::Define)
                   . // each
                 add(initOpnd)
                   . // initial value
                 add(addNode->minstr->getOperand(strideIdx))
                   .                                    // input 1
                 addReg(seqOT->getOperand(1).getReg()); // control
    }
    redInstr->setFlag(MachineInstr::NonSequential);
    // remove the instructions in the IDV cycle.
    switchNode->minstr->removeFromParent();
    MRI->markUsesInDebugValueAsUndef(addNode->minstr->getOperand(0).getReg());
    addNode->minstr->removeFromBundle();
    MRI->markUsesInDebugValueAsUndef(lhdrPickNode->minstr->getOperand(0).getReg());
    lhdrPickNode->minstr->removeFromParent();
  }
}

void CSASeqOpt::SequenceSwitchOut(CSASSANode *switchNode, CSASSANode *addNode,
                                  CSASSANode *lhdrPickNode,
                                  MachineInstr *seqIndv, unsigned seqReg,
                                  unsigned backedgeReg) {
  // adjust switch out value
  unsigned switchFalse = switchNode->minstr->getOperand(0).getReg();
  unsigned switchTrue  = switchNode->minstr->getOperand(1).getReg();
  unsigned switchOut   = switchFalse == backedgeReg ? switchTrue : switchFalse;
  unsigned strideIdx   = addNode->minstr->getOperand(1).isReg() ? 2 : 1;
  if (switchOut != CSA::IGN) {
    // compute the outbouned value for switchout = last + stride
    const TargetRegisterClass *TRC =
      TII->getRegisterClass(addNode->minstr->getOperand(0).getReg(), *MRI);
    unsigned last           = LMFI->allocateLIC(TRC);
    LMFI->setLICGroup(last, LMFI->getLICGroup(switchOut));
    const unsigned switchOp = TII->makeOpcode(CSA::Generic::SWITCH, TRC);
    MachineInstr *switchLast =
      BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr,
              DebugLoc(), TII->get(switchOp), CSA::IGN)
        .addReg(last, RegState::Define)
        .addReg(seqIndv->getOperand(3).getReg())
        . // last_pred
      addReg(seqReg);
    switchLast->setFlag(MachineInstr::NonSequential);

    const unsigned addOp =
      TII->makeOpcode(TII->getGenericOpcode(addNode->minstr->getOpcode()), TRC);
    MachineInstr *outbndInstr =
      BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr,
              DebugLoc(), TII->get(addOp), switchOut)
        .addReg(last)
        .add(addNode->minstr->getOperand(strideIdx));
    outbndInstr->setFlag(MachineInstr::NonSequential);
  }
}

void CSASeqOpt::MultiSequence(CSASSANode *switchNode, CSASSANode *addNode,
                              CSASSANode *lhdrPickNode) {
  // addNode has inputs phiNode
  bool isIDVCycle = TII->isAdd(addNode->minstr) &&
                    isIntegerOpcode(addNode->minstr->getOpcode()) &&
                    addNode->children[0] == lhdrPickNode;
  // switchNode has inputs addNode, and switch's control is SeqOT
  MachineInstr *seqIndv = getSeqOTDef(switchNode->minstr->getOperand(2));
  isIDVCycle            = isIDVCycle && seqIndv &&
               MRI->getVRegDef(switchNode->minstr->getOperand(3).getReg()) ==
                 addNode->minstr;

  unsigned backedgeReg   = 0;
  MachineInstr *loopInit = lpInitForPickSwitchPair(
    lhdrPickNode->minstr, switchNode->minstr, backedgeReg, seqIndv);
  isIDVCycle = isIDVCycle && (loopInit != nullptr);

  // handle only positive constant address stride for now
  unsigned strideIdx = addNode->minstr->getOperand(1).isReg() ? 2 : 1;
  MachineInstr *rptStride = addNode->minstr->getOperand(strideIdx).isImm() || !seqIndv ?
                            nullptr : 
                            repeatOpndInSameLoop(addNode->minstr->getOperand(strideIdx), seqIndv);
  isIDVCycle = isIDVCycle && (addNode->minstr->getOperand(strideIdx).isImm() || rptStride);
  // uses of phi can only be add or address computing
  // bool phiuseOK = false;
  unsigned phidst = lhdrPickNode->minstr->getOperand(0).getReg();
  // bool adduseOK = false;
  unsigned adddst = addNode->minstr->getOperand(0).getReg();
  // otherwise reduced to reduction
  assert(!MRI->hasOneNonDBGUse(phidst) || !MRI->hasOneNonDBGUse(adddst));

  isIDVCycle =
    isIDVCycle && MRI->hasOneNonDBGUse(adddst); // otherwise need to adjust init value
  if (isIDVCycle) {
    MachineOperand &initOpnd =
      lhdrPickNode->minstr->getOperand(2).getReg() == backedgeReg
        ? lhdrPickNode->minstr->getOperand(3)
        : lhdrPickNode->minstr->getOperand(2);
    unsigned seqReg        = phidst;
    MachineOperand tripcnt = getTripCntForSeq(seqIndv, lhdrPickNode->minstr);
    tripcnt.clearParent();
    if (tripcnt.isReg())
      tripcnt.setIsDef(false);
    // got a valid trip counter, convert to squence; otherwise stride
    if (!DisableMultiSeq && (!tripcnt.isImm() || tripcnt.getImm() > 0)) {
      const TargetRegisterClass *addTRC =
        TII->getRegisterClass(addNode->minstr->getOperand(0).getReg(), *MRI);
      // FMA only operates on register
      unsigned fmaReg = LMFI->allocateLIC(addTRC);
      if (addNode->minstr->getOperand(strideIdx).isImm()) {
        unsigned mulReg = LMFI->allocateLIC(addTRC);
        if (addNode->minstr->getOperand(strideIdx).getImm() != 1) {
          // avoid muptipy by 1
          const unsigned mulOp = TII->makeOpcode(CSA::Generic::MUL, addTRC);
          MachineInstr *mulInstr =
            BuildMI(*seqIndv->getParent(), lhdrPickNode->minstr, DebugLoc(),
                    TII->get(mulOp),
                    mulReg)
              .add(tripcnt)
              . // trip count from indv
            add(addNode->minstr->getOperand(strideIdx));
          mulInstr->setFlag(MachineInstr::NonSequential);
        } else {
          mulReg = tripcnt.getReg();
        }
        const unsigned addOp = TII->makeOpcode(CSA::Generic::ADD, addTRC);
        MachineInstr *addInstr =
          BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr,
                  DebugLoc(), TII->get(addOp),
                  fmaReg)
            .addReg(mulReg)
            .add(initOpnd); // trip count from indv
        addInstr->setFlag(MachineInstr::NonSequential);
      } else {
        const unsigned FMAOp = TII->makeOpcode(CSA::Generic::FMA, addTRC);
        MachineInstr *fmaInstr =
          BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr,
                  DebugLoc(), TII->get(FMAOp),
                  fmaReg)
            .add(tripcnt)
            . // trip count from indv
          add(addNode->minstr->getOperand(strideIdx))
            .            // stride
          add(initOpnd); // init
        fmaInstr->setFlag(MachineInstr::NonSequential);
      }
      unsigned firstReg    = LMFI->allocateLIC(&CSA::CI1RegClass);
      unsigned lastReg     = LMFI->allocateLIC(&CSA::CI1RegClass);
      unsigned predReg     = LMFI->allocateLIC(&CSA::CI1RegClass);
      const unsigned seqOp = TII->makeOpcode(CSA::Generic::SEQOTLT, addTRC);
      MachineInstr *seqInstr =
        BuildMI(*seqIndv->getParent(), lhdrPickNode->minstr, DebugLoc(),
                // TII->get(seqIndv->getOpcode()),
                TII->get(seqOp), seqReg)
          .addReg(predReg, RegState::Define)
          . // pred
        addReg(firstReg, RegState::Define)
          .addReg(lastReg, RegState::Define)
          .add(initOpnd)
          . // init
        addReg(fmaReg)
          .                                          // boundary
        add(addNode->minstr->getOperand(strideIdx)); // stride
      unsigned licDepth = GetSeqIndvLicDepth(seqIndv);
      //set lic depth for sequence instrution, since it is the driver
      //instruction for all other instructions in the SCC
      SetSeqLicDepth(seqReg, licDepth);
      SetSeqLicDepth(predReg, licDepth);
      SetSeqLicDepth(firstReg, licDepth);
      SetSeqLicDepth(lastReg, licDepth);
      seqInstr->setFlag(MachineInstr::NonSequential);
    } else {
      // can't figure out trip-counter; generate stride
      const TargetRegisterClass *TRC =
        TII->getRegisterClass(addNode->minstr->getOperand(0).getReg(), *MRI);
      const unsigned strideOp = TII->makeOpcode(CSA::Generic::STRIDE, TRC);
      MachineOperand &strideOpnd = addNode->minstr->getOperand(strideIdx).isReg() && rptStride ?
                                   rptStride->getOperand(2) :
                                   addNode->minstr->getOperand(strideIdx);
      MachineInstr *strideInstr =
        BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr,
                DebugLoc(), TII->get(strideOp), seqReg)
          .addReg(seqIndv->getOperand(1).getReg())
          .add(initOpnd)
          .add(strideOpnd);
      unsigned licDepth = GetSeqIndvLicDepth(seqIndv);
      SetSeqLicDepth(seqReg, licDepth);
      strideInstr->setFlag(MachineInstr::NonSequential);
    }
    SequenceSwitchOut(switchNode, addNode, lhdrPickNode, seqIndv, seqReg,
                      backedgeReg);
    // remove the instructions in the IDV cycle.
    switchNode->minstr->removeFromParent();
    MRI->markUsesInDebugValueAsUndef(adddst);
    addNode->minstr->removeFromBundle();
    lhdrPickNode->minstr->removeFromParent();
  }
}

// pick/switch paire representing a repeat consturct,
// with pick dst leads to switch, and switch's dst defines pick's backedge
// cmp result is used to control switch; init/mov is used to control pick
MachineInstr *CSASeqOpt::lpInitForPickSwitchPair(MachineInstr *pickInstr,
                                                 MachineInstr *switchInstr,
                                                 unsigned &backedgeReg,
                                                 MachineInstr *lpcmpInstr) {
  unsigned channel = pickInstr->getOperand(1).getReg();
  assert(TII->isPick(pickInstr));
  MachineInstr *result = nullptr;
  if (!MRI->hasOneDef(channel)) {
    MachineRegisterInfo::def_instr_iterator defI =
      MRI->def_instr_begin(channel);
    MachineRegisterInfo::def_instr_iterator defInext = std::next(defI);
    assert(std::next(defInext) == MachineRegisterInfo::def_instr_end());
    if ((TII->isMOV(&*defI) && TII->isInit(&*defInext)) ||
        (TII->isInit(&*defI) && TII->isMOV(&*defInext))) {
      MachineInstr *initInstr = TII->isInit(&*defI) ? &*defI : &*defInext;
      MachineInstr *movInstr  = TII->isMOV(&*defI) ? &*defI : &*defInext;
      MachineInstr *cmpInstr  = MRI->getVRegDef(
        movInstr->getOperand(1).getReg()); // could be cmp or mov/not
      if ((TII->isCmp(cmpInstr) ||
           (lpcmpInstr &&
            getSeqOTDef(cmpInstr->getOperand(1)) == lpcmpInstr)) &&
          initInstr->getOperand(1).isImm() &&
          (initInstr->getOperand(1).getImm() == 1 ||
           initInstr->getOperand(1).getImm() == 0)) {
        backedgeReg = initInstr->getOperand(1).getImm() == 0
                        ? pickInstr->getOperand(3).getReg()
                        : pickInstr->getOperand(2).getReg();
        if (MRI->getVRegDef(switchInstr->getOperand(2).getReg()) == cmpInstr) {
          result = initInstr;
        }
      }
    }
  }
  return result;
}

void CSASeqOpt::SequenceRepeat(CSASSANode *switchNode,
                               CSASSANode *lhdrPickNode) {
  // switchNode has inputs lhdrPick
  bool isIDVCycle =
    MRI->getVRegDef(switchNode->minstr->getOperand(3).getReg()) ==
    lhdrPickNode->minstr;
  bool switchuseOK = false;
  unsigned lpbackReg;
  MachineInstr *lpInit = nullptr;
  if (MachineInstr *switchCtrl =
        getSeqOTDef(switchNode->minstr->getOperand(2))) {
    lpInit = lpInitForPickSwitchPair(lhdrPickNode->minstr, switchNode->minstr,
                                     lpbackReg, switchCtrl);
  } else {
    lpInit = lpInitForPickSwitchPair(lhdrPickNode->minstr, switchNode->minstr,
                                     lpbackReg);
  }
  unsigned switchFalse = switchNode->minstr->getOperand(0).getReg();
  unsigned switchTrue  = switchNode->minstr->getOperand(1).getReg();
  if ((switchFalse == CSA::IGN && MRI->hasOneNonDBGUse(switchTrue) &&
       switchTrue == lpbackReg) ||
      (switchTrue == CSA::IGN && MRI->hasOneNonDBGUse(switchFalse) &&
       switchFalse == lpbackReg)) {
    switchuseOK = true;
  }

  isIDVCycle = isIDVCycle && switchuseOK && (lpInit != nullptr);
  if (isIDVCycle) {
    unsigned predRepeat = switchNode->minstr->getOperand(2).getReg();
    if (switchFalse == lpbackReg) {
      if (reg2neg.find(predRepeat) != reg2neg.end()) {
        predRepeat = reg2neg[predRepeat];
      } else {
        unsigned notReg = LMFI->allocateLIC(&CSA::CI1RegClass);
        LMFI->setLICGroup(notReg, LMFI->getLICGroup(predRepeat));
        MachineInstr *notInstr =
          BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr,
                  DebugLoc(), TII->get(CSA::NOT1), notReg)
            .addReg(predRepeat);
        notInstr->setFlag(MachineInstr::NonSequential);
        reg2neg[predRepeat] = notReg;
        predRepeat          = notReg;
      }
    }
    unsigned valueRepeat =
      lhdrPickNode->minstr->getOperand(2).getReg() == lpbackReg
        ? lhdrPickNode->minstr->getOperand(3).getReg()
        : lhdrPickNode->minstr->getOperand(2).getReg();

    unsigned repeatOp =
      TII->adjustOpcode(switchNode->minstr->getOpcode(), CSA::Generic::REPEATO);
    assert(repeatOp != CSA::INVALID_OPCODE);
    unsigned rptOut = lhdrPickNode->minstr->getOperand(0).getReg();
    MachineInstr *repeatInstr =
      BuildMI(*lhdrPickNode->minstr->getParent(), lhdrPickNode->minstr,
              DebugLoc(), TII->get(repeatOp), rptOut)
        .addReg(predRepeat)
        .addReg(valueRepeat);
    repeatInstr->setFlag(MachineInstr::NonSequential);
    SetSeqLicDepth(rptOut, DefaultSeqLicDepth);
    // remove the instructions in the IDV cycle.
    switchNode->minstr->removeFromParent();
    lhdrPickNode->minstr->removeFromParent();
  }
}

void CSASeqOpt::PrepRepeat() {
  CSASSAGraph csaSSAGraph;
  csaSSAGraph.BuildCSASSAGraph(*thisMF, true); // no control flow dependence
                                               // edge
  for (scc_iterator<CSASSANode *> I  = scc_begin(csaSSAGraph.getRoot()),
                                  IE = scc_end(csaSSAGraph.getRoot());
       I != IE; ++I) {
    const std::vector<CSASSANode *> &SCCNodes = *I;
    if (SCCNodes.size() == 2) {
      CSASSANode *lhdrPickNode = nullptr;
      CSASSANode *switchNode   = nullptr;
      bool isIDVCandidate      = true;
      for (std::vector<CSASSANode *>::const_iterator nodeI  = SCCNodes.begin(),
                                                     nodeIE = SCCNodes.end();
           nodeI != nodeIE && isIDVCandidate; ++nodeI) {
        CSASSANode *sccn     = *nodeI;
        MachineInstr *minstr = sccn->minstr;
        // loop header phi
        if (TII->isPick(minstr) && !lhdrPickNode) {
          unsigned pickCtrl = minstr->getOperand(1).getReg();
          for (MachineInstr &ctrlDef : MRI->def_instructions(pickCtrl)) {
            if (ctrlDef.getOpcode() == CSA::MOV1)
              continue;
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
  reg2neg.clear();
}


void CSASeqOpt::SetSeqLicDepth(unsigned lic, unsigned newDepth) {
  if (!EnableBuffer)
    return;

  newDepth = newDepth > SimLicMaxDepth ? SimLicMaxDepth : newDepth;
  int depth = LMFI->getLICDepth(lic);
  if (depth >=0 && (unsigned int)(depth) < newDepth)
    LMFI->setLICDepth(lic, newDepth);
}

MachineOperand* CSASeqOpt::GetConstSrc(MachineOperand &opnd) {
  // Walk backwards through repeats/filters/movs to see if there is a
  // constant to be propagated.
  MachineOperand* cur_opnd = &opnd;
  while (!cur_opnd->isImm()) {
    if (!cur_opnd->isReg())
      return nullptr;
    MachineInstr *def = MRI->getUniqueVRegDef(cur_opnd->getReg());
    if (!def)
      return nullptr;
    switch (TII->getGenericOpcode(def->getOpcode())) {
        case CSA::Generic::REPEAT:
        case CSA::Generic::REPEATO:
        case CSA::Generic::FILTER:
          cur_opnd = &def->getOperand(2);
          break;
        case CSA::Generic::MOV:
          cur_opnd = &def->getOperand(1);
          break;
        default:
          return nullptr;
    }
  }
  return cur_opnd;
}


unsigned CSASeqOpt::GetSeqIndvLicDepth(MachineInstr *seqIndv) {
  unsigned tripCnt = DefaultSeqLicDepth;
  MachineOperand* initOpnd = GetConstSrc(seqIndv->getOperand(4));
  MachineOperand* bndOpnd = GetConstSrc(seqIndv->getOperand(5));
  MachineOperand* strideOpnd = GetConstSrc(seqIndv->getOperand(6));
  if (initOpnd && initOpnd->isImm() &&
      strideOpnd && strideOpnd->isImm() &&
      bndOpnd && bndOpnd->isImm()) {
    //sequence has one extra terminaion token
    tripCnt = (bndOpnd->getImm() - initOpnd->getImm()) / strideOpnd->getImm() + 1;
  }
  return tripCnt;
}


void CSASeqOpt::SequenceOPT(bool runMultiSeq) {
  if (!runMultiSeq)
    DisableMultiSeq = true;

  PrepRepeat();
  CSASSAGraph csaSSAGraph;
  csaSSAGraph.BuildCSASSAGraph(*thisMF);
  //for memory operation in a SCC, set its input lic depth to the length
  //of SCC assuming each operation in the SCC generate an output each cycle
  for (scc_iterator<CSASSANode *> I  = scc_begin(csaSSAGraph.getRoot()),
                                  IE = scc_end(csaSSAGraph.getRoot());
       I != IE; ++I) {
    const std::vector<CSASSANode *> &SCCNodes = *I;
    if (SCCNodes.size() > 1 && SCCNodes.size() < SimLicMaxDepth) {
      for (std::vector<CSASSANode *>::const_iterator nodeI  = SCCNodes.begin(),
             nodeIE = SCCNodes.end(); nodeI != nodeIE; ++nodeI) {
        CSASSANode *sccn     = *nodeI;
        MachineInstr *minstr = sccn->minstr;
        if (minstr->mayLoad() || minstr->mayStore()) {
          for (auto op = minstr->uses().begin(), e = minstr->uses().end() - 1;
               op != e; op++) {
            if (op->isReg()) {
              SetSeqLicDepth(op->getReg(), SCCNodes.size());
            }
          }
        }
      }
    }
  }
  
  for (scc_iterator<CSASSANode *> I  = scc_begin(csaSSAGraph.getRoot()),
                                  IE = scc_end(csaSSAGraph.getRoot());
       I != IE; ++I) {
    const std::vector<CSASSANode *> &SCCNodes = *I;
    if (SCCNodes.size() > 1 && SCCNodes.size() < 8) {
      CSASSANode *lhdrPickNode = nullptr;
      CSASSANode *cmpNode      = nullptr;
      CSASSANode *addNode      = nullptr;
      CSASSANode *switchNode   = nullptr;
      bool isIDVCandidate      = true;
      for (std::vector<CSASSANode *>::const_iterator nodeI  = SCCNodes.begin(),
                                                     nodeIE = SCCNodes.end();
           nodeI != nodeIE && isIDVCandidate; ++nodeI) {
        CSASSANode *sccn     = *nodeI;
        MachineInstr *minstr = sccn->minstr;
        // loop header phi
        if (TII->isPick(minstr) && !lhdrPickNode) {
          unsigned pickCtrl = minstr->getOperand(1).getReg();
          for (MachineInstr &ctrlDef : MRI->def_instructions(pickCtrl)) {
            if (ctrlDef.getOpcode() == CSA::MOV1)
              continue;
            else if (ctrlDef.getOpcode() == CSA::INIT1 &&
                     (ctrlDef.getOperand(1).getImm() == 0 ||
                      ctrlDef.getOperand(1).getImm() == 1)) {
              lhdrPickNode = sccn;
            } else
              isIDVCandidate = false;
          }
        } else if ((TII->isAdd(minstr) ||                         // induction
                    TII->isCommutingReductionTransform(minstr) || // reduction
                    TII->isFMA(minstr) ||                         // reduction
                    TII->isSub(minstr)) &&                        // reduction
                   !addNode) {
          addNode = sccn;
        } else if (TII->isCmp(minstr) && !cmpNode) {
          cmpNode = sccn;
        } else if (TII->isSwitch(minstr) && !switchNode) {
          switchNode = sccn;
        } else if (TII->isMOV(minstr)) {
          // skip mov
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
  DenseMap<MachineInstr *, MachineOperand *>::iterator itm =
    seq2tripcnt.begin();
  while (itm != seq2tripcnt.end()) {
    MachineOperand *tripCnt = itm->getSecond();
    ++itm;
    delete tripCnt;
  }
  seq2tripcnt.clear();
  reg2neg.clear();
}

std::shared_ptr<CSALicGroup>
CSASeqOpt::getLoopPredicate(CSASSANode *lhdrPhiNode) {
  assert(TII->getGenericOpcode(lhdrPhiNode->minstr->getOpcode()) ==
      CSA::Generic::PICK && "loop predicate must be a PICK instruction");
  unsigned ctlreg = lhdrPhiNode->minstr->getOperand(1).getReg();
  if (loopPredicateGroups.find(ctlreg) == loopPredicateGroups.end()) {
    auto licGroup = std::make_shared<CSALicGroup>();

    // Assign frequency for the new group. Since we're based off of a PICK, it
    // is the case that the frequency of the output is the sum of its two input
    // parameters.
    auto opA = lhdrPhiNode->minstr->getOperand(2);
    auto opB = lhdrPhiNode->minstr->getOperand(3);
    if (opA.isReg() && opB.isReg()) {
      auto groupA = LMFI->getLICGroup(opA.getReg());
      auto groupB = LMFI->getLICGroup(opB.getReg());
      if (groupA && groupB) {
        licGroup->executionFrequency = groupA->executionFrequency +
          groupB->executionFrequency;
      }
    }
    loopPredicateGroups[ctlreg] = licGroup;
  }
  return loopPredicateGroups[ctlreg];
}
