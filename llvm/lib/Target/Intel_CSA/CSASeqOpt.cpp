//===-- CSASeqOpt.cpp - Sequence operator optimization --------------------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass adds sequence-style operators (such as sequence, repeat, or stride)
// to the dataflow loops where possible.
//
//===----------------------------------------------------------------------===//

#include "CSASeqOpt.h"
#include "CSAInstrInfo.h"
#include "CSAReassocReduc.h"
#include "CSATargetMachine.h"
#include "MachineCDG.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/Support/Debug.h"
namespace {
  //128 is what simulator uses now.
  //32 is the experimental number
  static const unsigned  SimLicMaxDepth = 128;
  static const unsigned  DefaultSeqLicDepth = 32;
}

#define DEBUG_TYPE "csa-seqopt"

using namespace llvm;

static cl::opt<bool> DisableMultiSeq(
  "csa-disable-multiseq", cl::Hidden,
  cl::desc("CSA Specific: Disable multiple sequence conversion"));

static cl::opt<bool> EnableBuffer(
  "csa-enable-buffer", cl::Hidden, cl::init(false),
  cl::desc("CSA Specific: Add buffering to loop LICs"));

CSASeqOpt::CSASeqOpt(MachineFunction *F, MachineOptimizationRemarkEmitter &ORE,
                     CSALoopInfoPass &LI, const char *PassName) :
  ORE(ORE), LI(LI), PassName(PassName) {
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

// Helper method: looks up the right opcode for the sequence for
// an induction variable, based on the opcode for the compare,
// transforming statement, and whether we need to invert the
// comparison.
static bool compute_matching_seq_opcode(unsigned ciOp, unsigned tOp,
                                        bool commute_compare_operands,
                                        bool negate_compare,
                                        const CSAInstrInfo &TII,
                                        unsigned *indvar_opcode) {

  *indvar_opcode = CSA::INVALID_OPCODE;

  // Transform the comparison opcode if needed.
  unsigned compareOp = ciOp;
  compareOp          = TII.commuteNegateCompareOpcode(
    compareOp, commute_compare_operands, negate_compare);
  if (compareOp == CSA::INVALID_OPCODE)
    return false;

  // Find a sequence opcode that matches our compare opcode.
  unsigned seqOp = TII.convertCompareOpToSeqOTOp(compareOp);

  // CMPLRS-50091: we cannot mix differently sized values in one
  // sequence instruction.
  if (TII.getLicSize(seqOp) != TII.getLicSize(tOp))
    return false;

  if (seqOp != CSA::INVALID_OPCODE &&
      TII.getGenericOpcode(tOp) == CSA::Generic::ADD) {
    // If we have a matching sequence op, then check that the
    // transforming op matches as well.
    *indvar_opcode = TII.promoteSeqOTOpBitwidth(seqOp, TII.getLicSize(tOp));
    return true;
  }
  return false;
}

void CSASeqOpt::SequenceIndv(MachineInstr *cmpInst, MachineInstr *switchInst,
                             MachineInstr *addInst, MachineInstr *lhdrPickInst) {
  // addInst has inputs phiInst, and an immediate input
  bool isIDVCycle = TII->isAdd(addInst) &&
                    isIntegerOpcode(addInst->getOpcode());
  // switchInst has inputs mov->cmpInst, addInst
  isIDVCycle =
    isIDVCycle && MRI->getVRegDef(switchInst->getOperand(3).getReg()) ==
                    addInst;
  unsigned backedgeReg = 0;
  // The index of the switch output that corresponds to the backedge.
  unsigned switchSense = 0;
  bool successful = analyzePickSwitchPair(
    lhdrPickInst, switchInst, backedgeReg, switchSense, cmpInst);
  if (!successful) {
    // can't generate in this case.
    return;
  }
  unsigned idvIdx = 0;
  // cmpInst has inputs either pickInst or addInst
  if (cmpInst->getOperand(1).isImm()) {
    idvIdx = 2;
  } else if (cmpInst->getOperand(2).isImm()) {
    idvIdx = 1;
  } else if (repeatOpndInSameLoop(cmpInst->getOperand(1),
                                  cmpInst)) {
    idvIdx = 2;
  } else if (repeatOpndInSameLoop(cmpInst->getOperand(2),
                                  cmpInst)) {
    idvIdx = 1;
  } else {
    return;
  }

  // CMPLRS-50091: we cannot mix differently sized values in one
  // sequence instruction.
  if (TII->getLicSize(cmpInst->getOpcode()) !=
      TII->getLicSize(addInst->getOpcode())) {
    MachineOptimizationRemarkMissed Remark(PassName, "CSASeqOptMissed: ",
                                           cmpInst->getDebugLoc(),
                                           cmpInst->getParent());
    ORE.emit(Remark << " bounded sequence cannot be generated; "
             "compare and increment have different size");
    return;
  }

  isIDVCycle = isIDVCycle &&
               (MRI->getVRegDef(cmpInst->getOperand(idvIdx).getReg()) ==
                  lhdrPickInst ||
                MRI->getVRegDef(cmpInst->getOperand(idvIdx).getReg()) ==
                  addInst);

  MachineOperand &bndOpnd = cmpInst->getOperand(3 - idvIdx);
  MachineInstr *rptBnd    = nullptr;
  MachineInstr *rptStride = nullptr;
  // boundary and stride must be integer value or integer register defined
  // outside the loop
  isIDVCycle =
    isIDVCycle && (bndOpnd.isImm() ||
                   (rptBnd = repeatOpndInSameLoop(bndOpnd, cmpInst)));
  // handle only |stride| == 1 for now
  unsigned strideIdx         = addInst->getOperand(1).isReg() ? 2 : 1;
  MachineOperand &strideOpnd = addInst->getOperand(strideIdx);
  // isIDVCycle = isIDVCycle && (strideOpnd.isImm() && (strideOpnd.getImm() == 1
  // || strideOpnd.getImm() == -1));
  isIDVCycle =
    isIDVCycle &&
    (strideOpnd.isImm() ||
     (rptStride = repeatOpndInSameLoop(strideOpnd, cmpInst)));

#if 0
  //uses of phi can only be add or cmp, or pick of the nested loop
  //new seq instr uses phi's dst as its value channel -- no need to consider phi dst's usage
  unsigned phidst = lhdrPickInst->getOperand(0).getReg();
  MachineRegisterInfo::use_iterator UI = MRI->use_begin(phidst);
  while (UI != MRI->use_end()) {
    MachineOperand &UseMO = *UI;
    ++UI;
    MachineInstr *UseMI = UseMO.getParent();
    if (UseMI != addInst && UseMI != cmpInst) {
      isIDVCycle = false;
      break;
    }
  }
#endif

  // new seq instr uses phi's dst as its value channel -- no need to consider
  // phi dst's usage  uses of add can only be switch or cmp
  unsigned adddst = addInst->getOperand(0).getReg();
  MachineRegisterInfo::use_iterator UI = MRI->use_begin(adddst);
  bool hasPostIncUses = false;
  while (UI != MRI->use_end()) {
    MachineOperand &UseMO = *UI;
    ++UI;
    MachineInstr *UseMI = UseMO.getParent();
    if (UseMI != switchInst && UseMI != cmpInst) {
      hasPostIncUses = true;
      break;
    }
  }

  if (cmpInst->getOperand(idvIdx).getReg() !=
      switchInst->getOperand(3).getReg()) {
    isIDVCycle = false;
  }

  unsigned pickInitIdx     = 2 + switchSense;
  MachineOperand *initOpnd = &lhdrPickInst->getOperand(pickInitIdx);
  unsigned switchOutIndex =
    switchInst->getOperand(0).getReg() == backedgeReg ? 1 : 0;
  isIDVCycle = isIDVCycle &&
               (switchInst->getOperand(1 - switchOutIndex).getReg() ==
                backedgeReg) &&
               MRI->hasOneNonDBGUse(backedgeReg);
  if (isIDVCycle) {
    unsigned compareSense = idvIdx - 1;
    unsigned switchOutIndex =
      switchInst->getOperand(0).getReg() == backedgeReg ? 1 : 0;
    assert(switchInst->getOperand(1 - switchOutIndex).getReg() ==
           backedgeReg);
    (void) switchOutIndex;
    // no use of switch outside the loop, only use is lhdrphi
    // Find a sequence opcode that matches our compare opcode.
    unsigned seqOp;
    if (!compute_matching_seq_opcode(
          cmpInst->getOpcode(), addInst->getOpcode(),
          compareSense, switchSense, *TII, &seqOp)) {
      assert(false && "can't find matching sequence opcode\n");
    }
#if 0
    //add is before cmp => need to adjust init value
    if (MRI->getVRegDef(cmpInst->getOperand(idvIdx).getReg()) == addInst) {
      if (initOpnd->isImm() && strideOpnd.isImm()) {
        int adjInit = addInst->getOperand(strideIdx).getImm() + initOpnd->getImm();
        initOpnd = &MachineOperand::CreateImm(adjInit);
      } else {
        const TargetRegisterClass *TRC = TII->lookupLICRegClass(addInst->getOperand(0).getReg());
        unsigned adjInitVReg = LMFI->allocateLIC(TRC);
        MachineInstr* adjInitInstr = BuildMI(*lhdrPickInst->getParent(),
          lhdrPickInst, DebugLoc(),
          TII->get(addInst->getOpcode()),
          adjInitVReg).
          add(addInst->getOperand(strideIdx)).                      //stride
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
    unsigned seqReg   = lhdrPickInst->getOperand(0).getReg();
    MachineOperand &indvBnd =
      bndOpnd.isReg() && rptBnd ? rptBnd->getOperand(2) : bndOpnd;
    MachineOperand &indvStride =
      strideOpnd.isReg() && rptStride ? rptStride->getOperand(2) : strideOpnd;
    unsigned cmpReg = cmpInst->getOperand(0).getReg();
    MachineInstr *seqInstr =
      BuildMI(*lhdrPickInst->getParent(), lhdrPickInst,
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
    unsigned adjCtrlOp = switchSense == 1 ? CSA::MOV1 : CSA::NOT1;
    MachineInstr *adjCmpInstr =
      BuildMI(*lhdrPickInst->getParent(), lhdrPickInst,
              DebugLoc(), TII->get(adjCtrlOp), cmpReg)
        .addReg(lastReg);
    adjCmpInstr->setFlag(MachineInstr::NonSequential);
    SequenceSwitchOut(switchInst, addInst, lhdrPickInst, seqInstr, seqReg,
                      backedgeReg);

    // If the add has uses, replace it with an add of the induction variable
    // from the switch.
    if (hasPostIncUses) {
      // if strideIdx == 2, set operand 1 else (it == 1) set operand 2.
      addInst->getOperand(3 - strideIdx).setReg(seqReg);
    } else {
      addInst->removeFromBundle();
    }

    MRI->markUsesInDebugValueAsUndef(backedgeReg);
    switchInst->eraseFromParent();
    lhdrPickInst->eraseFromParent();
    cmpInst->eraseFromParent();
    // currently only the cmp instr can have usage outside the cycle
    cmpInst = seqInstr;
    NewDrivenOps.push_back(seqInstr);
    LLVM_DEBUG(errs() << "  Replaced with " << *seqInstr);

    // Assign LIC groups for the new registers. first and last execute as many
    // times as the value does, while pred executes a little more frequently.
    auto loopGroup = LMFI->getLICGroup(seqReg);
    LMFI->setLICGroup(firstReg, loopGroup);
    LMFI->setLICGroup(lastReg, loopGroup);
    LMFI->setLICGroup(seqPred, getLoopPredicate(lhdrPickInst));
  }
}

MachineOperand *CSASeqOpt::getInvariantOperand(MachineOperand &Op) {
  if (Op.isImm())
    return &Op;

  if (!Op.isReg())
    return nullptr;

  MachineInstr *Src = MRI->getUniqueVRegDef(Op.getReg());
  if (!Src)
    return nullptr;

  auto Opcode = TII->getGenericOpcode(Src->getOpcode());
  if (Opcode != CSA::Generic::REPEAT && Opcode != CSA::Generic::REPEATO)
    return nullptr;

  return &Src->getOperand(2);
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

MachineInstr *CSASeqOpt::getSeqOTDef(MachineOperand &opnd, bool *isNegated) {
  if (!MRI->hasOneDef(opnd.getReg()))
    return nullptr;
  MachineInstr *defInstr = MRI->getVRegDef(opnd.getReg());
  if (isNegated)
    *isNegated = defInstr->getOpcode() == CSA::NOT1;
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

void CSASeqOpt::SequenceApp(MachineInstr *switchInst, MachineInstr *addInst,
                            MachineInstr *lhdrPickInst) {
  MultiSequence(switchInst, addInst, lhdrPickInst);
}

void CSASeqOpt::SequenceSwitchOut(MachineInstr *switchInst,
                                  MachineInstr *addInst,
                                  MachineInstr *lhdrPickInst,
                                  MachineInstr *seqIndv, unsigned seqReg,
                                  unsigned backedgeReg) {
  // adjust switch out value
  unsigned switchFalse = switchInst->getOperand(0).getReg();
  unsigned switchTrue  = switchInst->getOperand(1).getReg();
  unsigned switchOut   = switchFalse == backedgeReg ? switchTrue : switchFalse;
  unsigned strideIdx   = addInst->getOperand(1).isReg() ? 2 : 1;
  if (switchOut != CSA::IGN) {
    // compute the outbouned value for switchout = last + stride
    const TargetRegisterClass *TRC =
      TII->getRegisterClass(addInst->getOperand(0).getReg(), *MRI);
    unsigned last           = LMFI->allocateLIC(TRC);
    LMFI->setLICGroup(last, LMFI->getLICGroup(switchOut));
    const unsigned switchOp = TII->makeOpcode(CSA::Generic::SWITCH, TRC);
    MachineInstr *switchLast =
      BuildMI(*lhdrPickInst->getParent(), lhdrPickInst,
              DebugLoc(), TII->get(switchOp), CSA::IGN)
        .addReg(last, RegState::Define)
        .addReg(seqIndv->getOperand(3).getReg())
        . // last_pred
      addReg(seqReg);
    switchLast->setFlag(MachineInstr::NonSequential);

    const unsigned addOp =
      TII->makeOpcode(TII->getGenericOpcode(addInst->getOpcode()), TRC);
    MachineInstr *outbndInstr =
      BuildMI(*lhdrPickInst->getParent(), lhdrPickInst,
              DebugLoc(), TII->get(addOp), switchOut)
        .addReg(last)
        .add(addInst->getOperand(strideIdx));
    outbndInstr->setFlag(MachineInstr::NonSequential);
  }
}

void CSASeqOpt::MultiSequence(MachineInstr *switchInst, MachineInstr *addInst,
                              MachineInstr *lhdrPickInst) {
  // addInst has inputs phiInst
  bool isIDVCycle = TII->isAdd(addInst) &&
                    isIntegerOpcode(addInst->getOpcode());
  // switchInst has inputs addInst, and switch's control is SeqOT
  MachineInstr *seqIndv = getSeqOTDef(switchInst->getOperand(2));
  isIDVCycle            = isIDVCycle && seqIndv &&
               MRI->getVRegDef(switchInst->getOperand(3).getReg()) ==
                 addInst;

  unsigned backedgeReg   = 0;
  unsigned loopSense = 0;
  bool successful = analyzePickSwitchPair(
    lhdrPickInst, switchInst, backedgeReg, loopSense, seqIndv);
  if (!successful)
    return;

  unsigned PickBackedge =
    lhdrPickInst->getOperand(2).isReg() &&
        lhdrPickInst->getOperand(2).getReg() == backedgeReg
      ? 2
      : 3;

  // handle only positive constant address stride for now
  unsigned strideIdx = addInst->getOperand(1).isReg() ? 2 : 1;
  MachineInstr *rptStride = addInst->getOperand(strideIdx).isImm() || !seqIndv ?
                            nullptr :
                            repeatOpndInSameLoop(addInst->getOperand(strideIdx), seqIndv);
  MachineOperand &StrideDist = rptStride ? rptStride->getOperand(2) :
    addInst->getOperand(strideIdx);
  // Strides that aren't loop-invariant aren't supported.
  if (!rptStride && !StrideDist.isImm())
    return;

  // uses of phi can only be add or address computing
  unsigned phidst = lhdrPickInst->getOperand(0).getReg();

  if (isIDVCycle) {
    MachineOperand &initOpnd = lhdrPickInst->getOperand(PickBackedge == 2 ? 3 : 2);
    unsigned seqReg        = phidst;
    MachineOperand tripcnt = getTripCntForSeq(seqIndv, lhdrPickInst);
    tripcnt.clearParent();
    if (tripcnt.isReg())
      tripcnt.setIsDef(false);
    // got a valid trip counter, convert to squence; otherwise stride
    if (!DisableMultiSeq && (!tripcnt.isImm() || tripcnt.getImm() > 0)) {
      const TargetRegisterClass *addTRC =
        TII->getRegisterClass(addInst->getOperand(0).getReg(), *MRI);
      // FMA only operates on register
      unsigned fmaReg = LMFI->allocateLIC(addTRC);
      if (addInst->getOperand(strideIdx).isImm()) {
        unsigned mulReg = LMFI->allocateLIC(addTRC);
        if (addInst->getOperand(strideIdx).getImm() != 1) {
          // avoid muptipy by 1
          const unsigned mulOp = TII->makeOpcode(CSA::Generic::MUL, addTRC);
          MachineInstr *mulInstr =
            BuildMI(*seqIndv->getParent(), lhdrPickInst, DebugLoc(),
                    TII->get(mulOp),
                    mulReg)
              .add(tripcnt)
              . // trip count from indv
            add(addInst->getOperand(strideIdx));
          mulInstr->setFlag(MachineInstr::NonSequential);
        } else {
          mulReg = tripcnt.getReg();
        }
        const unsigned addOp = TII->makeOpcode(CSA::Generic::ADD, addTRC);
        MachineInstr *addInstr =
          BuildMI(*lhdrPickInst->getParent(), lhdrPickInst,
                  DebugLoc(), TII->get(addOp),
                  fmaReg)
            .addReg(mulReg)
            .add(initOpnd); // trip count from indv
        addInstr->setFlag(MachineInstr::NonSequential);
      } else {
        const unsigned FMAOp = TII->makeOpcode(CSA::Generic::FMA, addTRC);
        MachineInstr *fmaInstr =
          BuildMI(*lhdrPickInst->getParent(), lhdrPickInst,
                  DebugLoc(), TII->get(FMAOp),
                  fmaReg)
            .add(tripcnt)
            . // trip count from indv
          add(addInst->getOperand(strideIdx))
            .            // stride
          add(initOpnd); // init
        fmaInstr->setFlag(MachineInstr::NonSequential);
      }
      unsigned firstReg    = LMFI->allocateLIC(&CSA::CI1RegClass);
      unsigned lastReg     = LMFI->allocateLIC(&CSA::CI1RegClass);
      unsigned predReg     = LMFI->allocateLIC(&CSA::CI1RegClass);
      const unsigned seqOp = TII->makeOpcode(CSA::Generic::SEQOTLT, addTRC);
      MachineInstr *seqInstr =
        BuildMI(*seqIndv->getParent(), lhdrPickInst, DebugLoc(),
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
        add(addInst->getOperand(strideIdx)); // stride
      unsigned licDepth = GetSeqIndvLicDepth(seqIndv);
      //set lic depth for sequence instrution, since it is the driver
      //instruction for all other instructions in the SCC
      SetSeqLicDepth(seqReg, licDepth);
      SetSeqLicDepth(predReg, licDepth);
      SetSeqLicDepth(firstReg, licDepth);
      SetSeqLicDepth(lastReg, licDepth);
      seqInstr->setFlag(MachineInstr::NonSequential);
      NewDrivenOps.push_back(seqInstr);
      LLVM_DEBUG(errs() << "  Replaced with " << *seqInstr);
    } else {
      // can't figure out trip-counter; generate stride
      const TargetRegisterClass *TRC =
        TII->getRegisterClass(addInst->getOperand(0).getReg(), *MRI);
      const unsigned strideOp = TII->makeOpcode(CSA::Generic::STRIDE, TRC);
      MachineInstr *strideInstr =
        BuildMI(*lhdrPickInst->getParent(), lhdrPickInst,
                DebugLoc(), TII->get(strideOp), seqReg)
          .addReg(seqIndv->getOperand(1).getReg())
          .add(initOpnd)
          .add(StrideDist);
      unsigned licDepth = GetSeqIndvLicDepth(seqIndv);
      SetSeqLicDepth(seqReg, licDepth);
      strideInstr->setFlag(MachineInstr::NonSequential);
      NewDrivenOps.push_back(strideInstr);
      LLVM_DEBUG(errs() << "  Replaced with " << *strideInstr);
    }
    // Delete the pick, but not the add or the switch: if it's unused, deadcode
    // elimination will take care of it for us.
    lhdrPickInst->eraseFromParent();
  }
}

// pick/switch paire representing a repeat consturct,
// with pick dst leads to switch, and switch's dst defines pick's backedge
// cmp result is used to control switch; init/mov is used to control pick
bool CSASeqOpt::analyzePickSwitchPair(MachineInstr *pickInstr,
                                      MachineInstr *switchInstr,
                                      unsigned &backedgeReg,
                                      unsigned &loopSense,
                                      MachineInstr *lpcmpInstr) {
  unsigned channel = pickInstr->getOperand(1).getReg();
  assert(TII->isPick(pickInstr));
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
        movInstr->getOperand(1).getReg());
      // If we expect a specific sequence here, bail if the init isn't that.
      if (lpcmpInstr && !(lpcmpInstr == cmpInstr ||
            getSeqOTDef(cmpInstr->getOperand(1)) == lpcmpInstr)) {
        return false;
      }
      // If the switch doesn't use the same source as the compare, bail.
      if (MRI->getVRegDef(switchInstr->getOperand(2).getReg()) != cmpInstr) {
        return false;
      }
      loopSense = initInstr->getOperand(1).getImm();
      backedgeReg = loopSense == 0
                  ? pickInstr->getOperand(3).getReg()
                  : pickInstr->getOperand(2).getReg();
    }
  } else if (lpcmpInstr) {
    // Check for the same sequence driving the instruction.
    bool isPickNegated = false, isSwitchNegated = false;
    if (getSeqOTDef(pickInstr->getOperand(1), &isPickNegated) != lpcmpInstr ||
        getSeqOTDef(switchInstr->getOperand(2), &isSwitchNegated) != lpcmpInstr)
      return false;

    // Bail if the two negations don't match up.
    if (isPickNegated != isSwitchNegated)
      return false;

    backedgeReg = pickInstr->getOperand(isPickNegated ? 3 : 2).getReg();
    loopSense = isPickNegated ? 0 : 1;
    return true;
  }
  return true;
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

  LLVM_DEBUG(errs() << "Running sequence optimization " <<
      (runMultiSeq ? "with multiple sequences" : "with single sequences") <<
      "\n");

  for (auto &Loop : LI) {
    optimizeDFLoop(Loop);
  }

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
          if (getSeqOTDef(minstr->getOperand(1))) {
            lhdrPickNode = sccn;
          } else {
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

      LLVM_DEBUG({
        errs() << "Identified dataflow loop of " << SCCNodes.size()
          << " nodes:\n";
        for (auto Node : SCCNodes) {
          if (Node == lhdrPickNode)
            errs() << "    (pick) ";
          else if (Node == cmpNode)
            errs() << "     (cmp) ";
          else if (Node == addNode)
            errs() << "     (add) ";
          else if (Node == switchNode)
            errs() << "  (switch) ";
          else
            errs() << "           ";
          errs() << *Node->minstr;
        }
        if (!isIDVCandidate)
          errs() << "  Not an induction variable candidate.\n";
      });

      if (isIDVCandidate && cmpNode && switchNode && addNode && lhdrPickNode) {
        SequenceIndv(cmpNode->minstr, switchNode->minstr, addNode->minstr, lhdrPickNode->minstr);
      } else if (isIDVCandidate && switchNode && addNode && lhdrPickNode) {
        SequenceApp(switchNode->minstr, addNode->minstr, lhdrPickNode->minstr);
      }
    }
  }

  annotateBackedges();

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

template <typename Callable>
static MachineInstr *getSrcIfMatches(MachineRegisterInfo *MRI,
    MachineOperand &MO, Callable Func) {
  if (!MO.isReg())
    return nullptr;
  MachineInstr *Src = MRI->getVRegDef(MO.getReg());
  return Func(Src) ? Src : nullptr;
}

void CSASeqOpt::optimizeDFLoop(const CSALoopInfo &Loop) {
  if (Loop.getNumExits() != 1) {
    LLVM_DEBUG(dbgs() << "Skipping multi-exit loop.\n");
    return;
  }

  LLVM_DEBUG(Loop.print(dbgs()));

  auto ExitSwitches = Loop.getExitSwitches(0);
  if (ExitSwitches.begin() == ExitSwitches.end()) {
    LLVM_DEBUG(dbgs() << "Er, no switches?\n");
    return;
  }
  DebugLoc LoopLoc = (*std::begin(ExitSwitches))->getDebugLoc();

  // Get some useful loop details.
  unsigned PickOpIdx = Loop.getPickBackedgeIndex() + 2;
  unsigned SwitchOpIdx = Loop.getSwitchBackedgeIndex(0);
  // The register that controls the header pick. This is the (possibly negated)
  // %first output of the SEQ operator for the loop.
  unsigned LoopFirstReg = 0;

  // Generate a stream predicate for the loop. In a single-exit loop, the
  // predicate on the exit switch, or its negation, acts as the last operator of
  // the loop. To convert a not-last input to a predicate stream, we need to
  // replace the final 0 on the loop with a 0 followed by a 1, and then we need
  // to also arrange for an initial 1 on the stream.
  unsigned LoopNotLastReg = (*std::begin(ExitSwitches))->getOperand(2).getReg();
  if (SwitchOpIdx == 0) {
    LoopNotLastReg = negateRegister(LoopNotLastReg);
  }
  MachineInstr *IP = Loop.getHeaderPicks()[0];
  unsigned LoopPredReg = LMFI->allocateLIC(&CSA::CI1RegClass, "loop.pred");
  BuildMI(*IP->getParent(), IP, IP->getDebugLoc(), TII->get(CSA::REPLACE1),
      LoopPredReg)
    .addReg(LoopNotLastReg)
    .addImm(1).addImm(0) // Replace {0} =>
    .addImm(2).addImm(2) // {0, 1}
    .setMIFlags(MachineInstr::NonSequential);
  BuildMI(*IP->getParent(), IP, IP->getDebugLoc(), TII->get(CSA::INIT1),
      LoopPredReg)
    .addImm(1)
    .setMIFlags(MachineInstr::NonSequential);

  // Set frequency and grouping based on one of the header picks.
  LMFI->setLICGroup(LoopPredReg, getLoopPredicate(IP));

  // Collect pick switch pairs for the loop.
  std::vector<std::pair<MachineInstr *, MachineInstr *>> PickSwitchPairs;
  SmallVector<MachineInstr *, 8> NewInsts;
  SmallVector<MachineInstr *, 8> LoopCarryPicks;

  for (auto PickInst : Loop.getHeaderPicks()) {
    // Check if the value along the backedge comes from one of the exit
    // switches. This occuring allows us to see what the next iteration value
    // will be.
    MachineInstr *SwitchInst = getSrcIfMatches(MRI,
      PickInst->getOperand(PickOpIdx), [this, ExitSwitches](MachineInstr *MI) {
        return TII->isSwitch(MI) && is_contained(ExitSwitches, MI);
      });

    // If we don't have a matching switch, then we have to assume that the pick
    // is not loop-invariant.
    if (!SwitchInst) {
      LLVM_DEBUG(dbgs() << "Found loop-carried dependence: " << *PickInst);
      LoopCarryPicks.push_back(PickInst);
      continue;
    }

    // If this is a repeat instruction, deal with it immediately.
    if (SwitchInst->getOperand(3).isReg() &&
        SwitchInst->getOperand(3).getReg() == PickInst->getOperand(0).getReg()) {
      NewInsts.push_back(
        FormRepeat(PickInst, PickOpIdx, SwitchInst, LoopPredReg));
      continue;
    }

    // Keep track of the Pick/Switch pair for later.
    PickSwitchPairs.emplace_back(PickInst, SwitchInst);
  }

  // Now that we've collected all the repeats, try to form STRIDE and SEQOT
  // operations for the rest of the loop. Essentially, what we're doing is a
  // simple version of SCEV for machine IR, where we care only about the
  // innermost SCEVAddRecExpr.
  for (auto PickSwitchPair : PickSwitchPairs) {
    MachineInstr *PickInst = PickSwitchPair.first;
    MachineInstr *SwitchInst = PickSwitchPair.second;

    MachineInstr *AddInst = getSrcIfMatches(MRI, SwitchInst->getOperand(3),
        [this](MachineInstr *MI) { return
          (TII->isAdd(MI) || TII->isSub(MI)) &&
          isIntegerOpcode(MI->getOpcode());
        });
    MachineInstr *CmpInst = getSrcIfMatches(MRI, SwitchInst->getOperand(2),
        [this](MachineInstr *MI) { return TII->isCmp(MI); });
    MachineInstr *ReducModInst =
      getSrcIfMatches(MRI, SwitchInst->getOperand(3), [this](MachineInstr *MI) {
        return TII->convertTransformToReductionOp(MI->getOpcode()) !=
               CSA::INVALID_OPCODE;
      });

    // Filter out the add inst if it doesn't use the result of the pick.
    if (AddInst) {
      unsigned PickReg = PickInst->getOperand(0).getReg();
      bool UsesPick = false;
      for (auto &DefOp : AddInst->uses())
        if (DefOp.isReg() && DefOp.getReg() == PickReg)
          UsesPick = true;
      if (!UsesPick)
        AddInst = nullptr;
    }

    // Filter out the cmp if it doesn't use the result of the pick or add.
    if (CmpInst) {
      unsigned PickReg = PickInst->getOperand(0).getReg();
      unsigned AddReg = AddInst ? AddInst->getOperand(0).getReg() : Register();
      bool UsesPick = false;
      for (auto &DefOp : CmpInst->uses())
        if (DefOp.isReg() &&
            (DefOp.getReg() == PickReg || DefOp.getReg() == AddReg))
          UsesPick = true;
      if (!UsesPick)
        CmpInst = nullptr;
    }

    // Filter potential reductions based on the reduction generation settings.
    if (ReducModInst) {
      const CSA::Generic ReducModGeneric =
        TII->getGenericOpcode(ReducModInst->getOpcode());
      const bool IsAdd =
        ReducModGeneric == CSA::Generic::ADD ||
        ReducModGeneric == CSA::Generic::SUB ||
        (csa_reduc::fissionFMAs() && ReducModGeneric == CSA::Generic::FMA);
      csa_reduc::ReducLevel ReducLevel =
        IsAdd ? csa_reduc::REDUC_LEVEL_ADD : csa_reduc::REDUC_LEVEL_ALL;
      const bool HasReassocFlag =
        ReducModInst->getFlag(MachineInstr::FmReassoc);

      // Don't generate this reduction if it's not enabled.
      if (csa_reduc::reducsEnabled() < ReducLevel)
        ReducModInst = nullptr;

      // Also don't generate it if it isn't forced and doesn't have a flag
      // saying it can be reassociated.
      if (csa_reduc::reducsForced() < ReducLevel and not HasReassocFlag)
        ReducModInst = nullptr;
    }

    // Filter out the potential reduction if it doesn't use the result of the
    // pick or has exposed partial reduction uses.
    if (ReducModInst) {

      // Make sure that there is a supported use of the pick output.
      const unsigned PickReg     = PickInst->getOperand(0).getReg();
      const auto OpndIsUseOfPick = [&](unsigned OpIdx) {
        const MachineOperand &Op = ReducModInst->getOperand(OpIdx);
        return Op.isReg() && Op.getReg() == PickReg;
      };
      bool UsesPick      = false;
      const unsigned OpC = ReducModInst->getOpcode();
      switch (TII->getGenericOpcode(OpC)) {

      // SUB and FMA ops are non-commutative, so the pick needs to be on a
      // specific operand for those.
      case CSA::Generic::SUB:
        UsesPick = OpndIsUseOfPick(1);
        break;
      case CSA::Generic::FMA:
        UsesPick = OpndIsUseOfPick(3);
        break;

      // Other ones are commutative, so the pick can appear on either of the
      // input operands.
      default:
        UsesPick = OpndIsUseOfPick(1) || OpndIsUseOfPick(2);
        break;
      }

      // Also check that there are no external uses of the partial values.
      const unsigned ReducReg    = ReducModInst->getOperand(0).getReg();
      const unsigned BackReg     = PickInst->getOperand(PickOpIdx).getReg();
      const bool HasExternalUses = !MRI->hasOneNonDBGUse(PickReg) ||
                                   !MRI->hasOneNonDBGUse(ReducReg) ||
                                   !MRI->hasOneNonDBGUse(BackReg);
      bool IneligibleReduc = false;
      if (!UsesPick || HasExternalUses)
        IneligibleReduc = true;

      // SIMD ops must also have no swizzle/disable bits set.
      // TODO: Add support for disable bits with a code transform.
      if (ReducModInst && TII->getOpcodeClass(OpC) == CSA::VARIANT_SIMD) {
        if (TII->getGenericOpcode(OpC) == CSA::Generic::FMA) {
          if (ReducModInst->getOperand(4).getImm() != 0)
            IneligibleReduc = true;
          if (ReducModInst->getOperand(5).getImm() != 0)
            IneligibleReduc = true;
          if (ReducModInst->getOperand(6).getImm() != 0)
            IneligibleReduc = true;
        } else {
          if (ReducModInst->getOperand(3).getImm() != 0)
            IneligibleReduc = true;
          if (ReducModInst->getOperand(4).getImm() != 0)
            IneligibleReduc = true;
          if (ReducModInst->getOperand(5).getImm() != 0)
            IneligibleReduc = true;
        }
      }

      if (IneligibleReduc)
        ReducModInst = nullptr;
    }

    LLVM_DEBUG({
      dbgs() << "Header pick of loop has these dependencies:\n";
      dbgs() << "    (pick) " << *PickInst;
      if (AddInst)
        dbgs() << "     (add) " << *AddInst;
      if (CmpInst)
        dbgs() << "     (cmp) " << *CmpInst;
      if (ReducModInst)
        dbgs() << "   (reduc) " << *ReducModInst;
      dbgs() << "  (switch) " << *SwitchInst;
    });

    // Try to convert the pick/add/switch to a stride operator...
    MachineInstr *NewInst = nullptr;
    if (AddInst) {
      NewInst = CreateStride(PickInst, PickOpIdx, SwitchInst, LoopPredReg,
        AddInst);
      // ... and if it worked, try to add the compare as well into a sequence.
      if (NewInst && CmpInst) {
        MachineInstr *SeqInstr = StrideToSeq(CmpInst, SwitchInst, AddInst,
          NewInst, SwitchOpIdx, LoopPredReg);
        // (This won't always be successful, but the stride already was).
        if (SeqInstr) {
          NewInst = SeqInstr;
          // Note the new first register--we now have a sequence-driven loop.
          LoopFirstReg = SeqInstr->getOperand(2).getReg();
        }
      }
    }

    // Or, try to convert to a reduction operator.
    if (ReducModInst) {
      NewInst =
        CreateReduc(PickInst, PickOpIdx, SwitchInst, LoopPredReg, ReducModInst);
    }

    if (!NewInst) {
      LLVM_DEBUG(dbgs() << "  Loop-carried dependency.\n");
      LoopCarryPicks.push_back(PickInst);
    } else {
      LLVM_DEBUG(dbgs() << "  Replaced with " << *NewInst);
      NewInsts.push_back(NewInst);
    }
  }

  // If we produced a sequence operation in the loop, convert the operations to
  // use it.
  if (LoopFirstReg != 0) {
    // Make the pick registers use the first output of the sequence.
    if (Loop.getPickBackedgeIndex() == 1)
      LoopFirstReg = negateRegister(LoopFirstReg);

    for (auto PickInst : LoopCarryPicks) {
      PickInst->getOperand(1).setReg(LoopFirstReg);
    }
  } else {
    MachineOptimizationRemarkMissed Remark(
      PassName, "CSASeqOptMissed: ", LoopLoc, &thisMF->front());
    ORE.emit(Remark << " loop will not be driven by a sequence operator");
  }

  if (!LoopCarryPicks.empty()) {
    for (auto PickInst : LoopCarryPicks) {
      MachineOptimizationRemarkMissed Remark(PassName, "CSASeqOptMissed: ",
                                             PickInst->getDebugLoc(),
                                             PickInst->getParent());
      ORE.emit(Remark << " loop-carried dependence exists that could not be "
               "converted to a stride, repeat, or sequence");
    }
  }
}

MachineInstr *CSASeqOpt::FormRepeat(MachineInstr *PickInst,
                                    unsigned PickBackedge,
                                    MachineInstr *SwitchInst,
                                    unsigned LoopRepeat) {
  LLVM_DEBUG(dbgs() << "Found repeat:\n" << *PickInst << *SwitchInst);

  // We've already guaranteed that pick and switch are the header and exit of
  // the same loop. Get the parameters of the REPEAT operator.
  MachineOperand &valueRepeat = PickBackedge == 2
    ? PickInst->getOperand(3)
    : PickInst->getOperand(2);

  // Replace the PICK with the REPEAT.
  unsigned repeatOp =
    TII->adjustOpcode(PickInst->getOpcode(), CSA::Generic::REPEAT);
  assert(repeatOp != CSA::INVALID_OPCODE);
  unsigned rptOut = PickInst->getOperand(0).getReg();
  MachineInstr *repeatInstr = BuildMI(*PickInst->getParent(), PickInst,
      PickInst->getDebugLoc(), TII->get(repeatOp), rptOut)
    .addReg(LoopRepeat)
    .add(valueRepeat)
    .setMIFlag(MachineInstr::NonSequential);
  PickInst->eraseFromParent();

  NewDrivenOps.push_back(repeatInstr);
  LLVM_DEBUG(errs() << "  Replaced with " << *repeatInstr);
  return repeatInstr;
}

MachineInstr *CSASeqOpt::CreateStride(MachineInstr *PickInst,
                                      unsigned PickBackedgeIdx,
                                      MachineInstr *SwitchInst,
                                      unsigned Predicate,
                                      MachineInstr *AddInst) {
  // Check that we are using an add instruction.
  if (!TII->isAdd(AddInst) || !isIntegerOpcode(AddInst->getOpcode()))
    return nullptr;

  // Check that the switch uses the result of the add.
  unsigned AddResultReg = AddInst->getOperand(0).getReg();
  if (SwitchInst->getOperand(3).getReg() != AddResultReg)
    return nullptr;

  unsigned PickResultReg = PickInst->getOperand(0).getReg();

  // Add must depend on the pick in its first operand.
  if (AddInst->getOperand(1).isReg() &&
      AddInst->getOperand(1).getReg() == PickResultReg) {
    // Do nothing, this is what we expected.
  } else if (AddInst->getOperand(2).isReg() &&
             AddInst->getOperand(2).getReg() == PickResultReg) {
    // (but we can swap the operands if it's the second operand).
    TII->commuteInstruction(*AddInst, false, 1, 2);
  } else {
    // Nothing we can do here, add doesn't depend on the pick.
    return nullptr;
  }

  // And the other operand is loop-invariant.
  MachineOperand *StrideOperand = getInvariantOperand(AddInst->getOperand(2));
  if (!StrideOperand)
    return nullptr;

  MachineOperand &Initial = PickInst->getOperand(PickBackedgeIdx == 2 ? 3 : 2);
  unsigned StrideOp = TII->adjustOpcode(AddInst->getOpcode(),
    CSA::Generic::STRIDE);
  assert(StrideOp != CSA::INVALID_OPCODE && "Can't convert to stride");
  MachineInstr *StrideInst = BuildMI(*AddInst->getParent(), AddInst,
      AddInst->getDebugLoc(), TII->get(StrideOp), PickResultReg)
    .addReg(Predicate)
    .add(Initial)
    .add(*StrideOperand)
    .setMIFlags(MachineInstr::NonSequential | AddInst->getFlags());

  // Remove the pick. If the add or switch are unused, they will be DCE'd by
  // later passes anyways.
  PickInst->eraseFromParent();
  NewDrivenOps.push_back(StrideInst);
  return StrideInst;
}

MachineInstr *CSASeqOpt::StrideToSeq(MachineInstr *cmpInst,
                                     MachineInstr *switchInst,
                                     MachineInstr *addInst,
                                     MachineInstr *strideInst,
                                     unsigned SwitchBackedgeIdx,
                                     unsigned LoopPredicate) {
  LLVM_DEBUG(dbgs() << "Trying to convert to sequence...\n");
  // Check that the comparison is used for the switch instruction.
  unsigned CmpResultReg = cmpInst->getOperand(0).getReg();
  if (switchInst->getOperand(2).getReg() != CmpResultReg)
    return nullptr;

  // CMPLRS-50091: we cannot mix differently sized values in one
  // sequence instruction.
  if (TII->getLicSize(cmpInst->getOpcode()) !=
      TII->getLicSize(addInst->getOpcode())) {
    MachineOptimizationRemarkMissed Remark(PassName, "CSASeqOptMissed: ",
                                           cmpInst->getDebugLoc(),
                                           cmpInst->getParent());
    ORE.emit(Remark << " bounded sequence cannot be generated; "
             "compare and increment have different size");
    return nullptr;
  }

  // One of the operands of the compare is the stride or add...
  unsigned PickResultReg = strideInst->getOperand(0).getReg();
  unsigned AddResultReg = addInst->getOperand(0).getReg();
  unsigned CompareInvIndex;
  if (cmpInst->getOperand(1).isReg() &&
      (cmpInst->getOperand(1).getReg() == PickResultReg ||
       cmpInst->getOperand(1).getReg() == AddResultReg)) {
    CompareInvIndex = 2;
  } else if (cmpInst->getOperand(2).isReg() &&
      (cmpInst->getOperand(2).getReg() == PickResultReg ||
       cmpInst->getOperand(2).getReg() == AddResultReg)) {
    CompareInvIndex = 1;
  } else {
    return nullptr;
  }

  // ... and the other operand is loop-invariant.
  MachineOperand *LimitOperand =
    getInvariantOperand(cmpInst->getOperand(CompareInvIndex));
  if (!LimitOperand) {
    return nullptr;
  }

  bool postInc = cmpInst->getOperand(3 - CompareInvIndex).getReg() ==
    AddResultReg;
  if (!postInc) {
    // Try to make the compare post-increment.
    auto CmpOpcode = cmpInst->getOpcode();
    auto CmpKind = TII->getGenericOpcode(CmpOpcode);
    auto Variant = TII->getOpcodeClass(CmpOpcode);
    bool willNotWrap = (Variant == CSA::VARIANT_UNSIGNED &&
      strideInst->getFlag(MachineInstr::NoUWrap)) ||
      (Variant == CSA::VARIANT_SIGNED &&
       strideInst->getFlag(MachineInstr::NoSWrap));
    MachineOperand &StrideOp = strideInst->getOperand(3);
    if (CmpKind == CSA::Generic::CMPLT && willNotWrap && StrideOp.isImm() &&
        StrideOp.getImm() == 1) {
      cmpInst->setDesc(TII->get(TII->adjustOpcode(cmpInst->getOpcode(),
        CSA::Generic::CMPLE)));
    } else {
      LLVM_DEBUG(dbgs() << "  Pre-increment is too difficult to handle.\n");
      return nullptr;
    }
  }

  // Unhook the previous loop predicate generation.
  SmallVector<MachineInstr *, 2> PrevPredicates;
  for (auto &Inst : MRI->def_instructions(LoopPredicate))
    PrevPredicates.push_back(&Inst);
  assert(PrevPredicates.size() == 2 && "Unhooking a REPLACE/INIT combo only");
  for (auto Inst : PrevPredicates)
    Inst->eraseFromParent();

  // Find a sequence opcode that matches our compare opcode.
  unsigned seqOp;
  if (!compute_matching_seq_opcode(
        cmpInst->getOpcode(), addInst->getOpcode(),
        CompareInvIndex == 1, SwitchBackedgeIdx == 0, *TII, &seqOp)) {
    assert(false && "can't find matching sequence opcode\n");
  }
  unsigned firstReg = LMFI->allocateLIC(&CSA::CI1RegClass, "loop.first");
  unsigned lastReg  = LMFI->allocateLIC(&CSA::CI1RegClass, "loop.last");
  MachineInstr *seqInstr = BuildMI(*strideInst->getParent(), strideInst,
      strideInst->getDebugLoc(), TII->get(seqOp), PickResultReg)
    .addReg(LoopPredicate, RegState::Define)
    .addReg(firstReg, RegState::Define)
    .addReg(lastReg, RegState::Define)
    .add(strideInst->getOperand(2))
    .add(*LimitOperand)
    .add(strideInst->getOperand(3))
    .setMIFlags(MachineInstr::NonSequential);

  // If we switch on 1, 1, 1, ... 1, 0, then we want the switch to use
  // NOT on the last instead of the last directly.
  if (SwitchBackedgeIdx == 1) {
    BuildMI(*cmpInst->getParent(), cmpInst, cmpInst->getDebugLoc(),
        TII->get(CSA::NOT1), CmpResultReg)
      .addReg(lastReg)
      .setMIFlags(MachineInstr::NonSequential);
  } else {
    // Otherwise, use the result directly.
    MRI->replaceRegWith(lastReg, CmpResultReg);
  }

  // Remove the compare and stride.
  cmpInst->eraseFromParent();
  auto StrideInNew =
    std::find(NewDrivenOps.begin(), NewDrivenOps.end(), strideInst);
  if (StrideInNew != NewDrivenOps.end()) {
    NewDrivenOps.erase(StrideInNew);
  }
  strideInst->eraseFromParent();

  NewDrivenOps.push_back(seqInstr);

  // Assign LIC groups for the new registers. first and last execute as many
  // times as the value does, while pred executes a little more frequently.
  auto loopGroup = LMFI->getLICGroup(PickResultReg);
  LMFI->setLICGroup(firstReg, loopGroup);
  LMFI->setLICGroup(lastReg, loopGroup);

  auto predGroup = LMFI->getLICGroup(LoopPredicate);
  if (predGroup->LoopId == 0) {
    // Assign first/last's loop id to LoopPredicate's LIC group.
    predGroup->LoopId = loopGroup->LoopId;
  } else {
    assert(predGroup->LoopId == loopGroup->LoopId);
  }

  return seqInstr;
}

MachineInstr *CSASeqOpt::CreateReduc(MachineInstr *PickInst,
                                     unsigned PickBackedgeIdx,
                                     MachineInstr *SwitchInst,
                                     unsigned LoopPredicate,
                                     MachineInstr *ReducModInst) {

  // Get the opcode out of TII.
  const unsigned ReducOpC =
    TII->convertTransformToReductionOp(ReducModInst->getOpcode());
  const bool IsSIMD =
    (TII->getOpcodeClass(ReducModInst->getOpcode()) == CSA::VARIANT_SIMD);

  // Get the init value from the non-backedge input to the pick.
  const unsigned PickNonBackedgeIdx = (PickBackedgeIdx == 2) ? 3 : 2;
  const MachineOperand &Init        = PickInst->getOperand(PickNonBackedgeIdx);

  // The output of the new instruction will be the non-backedge switch output.
  const unsigned BackedgeReg = PickInst->getOperand(PickBackedgeIdx).getReg();
  const unsigned SwitchNonBackedgeIdx =
    (SwitchInst->getOperand(0).getReg() != BackedgeReg) ? 0 : 1;
  const unsigned SwitchNonBackedgeReg =
    SwitchInst->getOperand(SwitchNonBackedgeIdx).getReg();

  // FMA ops need to be handled specially because of the multiple inputs and
  // also possibly fission.
  MachineInstr *ReducInst = nullptr;
  if (TII->getGenericOpcode(ReducModInst->getOpcode()) == CSA::Generic::FMA) {
    const MachineOperand &Data0 = ReducModInst->getOperand(1);
    const MachineOperand &Data1 = ReducModInst->getOperand(2);
    const MachineOperand &RMode = ReducModInst->getOperand(IsSIMD ? 7 : 4);

    if (csa_reduc::fissionFMAs()) {
      const unsigned Mul =
        LMFI->allocateLIC(MRI->getRegClass(SwitchNonBackedgeReg), "redfiss");
      LMFI->setLICGroup(Mul, LMFI->getLICGroup(SwitchNonBackedgeReg));
      const unsigned MulOpC =
        TII->adjustOpcode(ReducModInst->getOpcode(), CSA::Generic::MUL);
      const unsigned RedAddOpC =
        TII->adjustOpcode(ReducOpC, CSA::Generic::REDADD);
      if (IsSIMD) {
        BuildMI(*ReducModInst->getParent(), ReducModInst,
                ReducModInst->getDebugLoc(), TII->get(MulOpC), Mul)
          .add(Data0)
          .add(Data1)
          .addImm(0)
          .addImm(0)
          .addImm(0)
          .add(RMode)
          .setMIFlags(MachineInstr::NonSequential | ReducModInst->getFlags());
      } else {
        BuildMI(*ReducModInst->getParent(), ReducModInst,
                ReducModInst->getDebugLoc(), TII->get(MulOpC), Mul)
          .add(Data0)
          .add(Data1)
          .add(RMode)
          .setMIFlags(MachineInstr::NonSequential | ReducModInst->getFlags());
      }
      ReducInst =
        BuildMI(*ReducModInst->getParent(), ReducModInst,
                ReducModInst->getDebugLoc(), TII->get(RedAddOpC),
                SwitchNonBackedgeReg)
          .add(Init)
          .addUse(Mul)
          .addUse(LoopPredicate)
          .add(RMode)
          .setMIFlags(MachineInstr::NonSequential | ReducModInst->getFlags());
    } else {
      ReducInst =
        BuildMI(*ReducModInst->getParent(), ReducModInst,
                ReducModInst->getDebugLoc(), TII->get(ReducOpC),
                SwitchNonBackedgeReg)
          .add(Init)
          .add(Data0)
          .add(Data1)
          .addUse(LoopPredicate)
          .add(RMode)
          .setMIFlags(MachineInstr::NonSequential | ReducModInst->getFlags());
    }
  } else {

    // For other ops, the data input should be the non-pick input.
    const unsigned PickResultReg = PickInst->getOperand(0).getReg();
    const unsigned NonPickIdx =
      (ReducModInst->getOperand(1).getReg() != PickResultReg) ? 1 : 2;
    const MachineOperand &Data  = ReducModInst->getOperand(NonPickIdx);
    const MachineOperand &RMode = ReducModInst->getOperand(IsSIMD ? 6 : 3);

    ReducInst =
      BuildMI(*ReducModInst->getParent(), ReducModInst,
              ReducModInst->getDebugLoc(), TII->get(ReducOpC),
              SwitchNonBackedgeReg)
        .add(Init)
        .add(Data)
        .addUse(LoopPredicate)
        .add(RMode)
        .setMIFlags(MachineInstr::NonSequential | ReducModInst->getFlags());
  }

  // Delete the pick, switch, and original op.
  PickInst->eraseFromParentAndMarkDBGValuesForRemoval();
  ReducModInst->eraseFromParentAndMarkDBGValuesForRemoval();
  SwitchInst->eraseFromParentAndMarkDBGValuesForRemoval();

  NewDrivenOps.push_back(ReducInst);
  return ReducInst;
}

unsigned CSASeqOpt::negateRegister(unsigned Reg) {
  if (reg2neg.find(Reg) != reg2neg.end()) {
    return reg2neg[Reg];
  } else {
    unsigned notReg = LMFI->allocateLIC(&CSA::CI1RegClass);
    LMFI->setLICGroup(notReg, LMFI->getLICGroup(Reg));
    LMFI->setLICName(notReg, LMFI->getLICName(Reg) + Twine(".not"));
    MachineInstr *Src = MRI->getUniqueVRegDef(Reg);
    assert(Src && "Can't negate a register with an INIT value");
    BuildMI(*Src->getParent(), Src, Src->getDebugLoc(), TII->get(CSA::NOT1),
        notReg)
      .addReg(Reg)
      .setMIFlags(MachineInstr::NonSequential);
    return reg2neg[Reg] = notReg;
  }
}

std::shared_ptr<CSALicGroup>
CSASeqOpt::getLoopPredicate(MachineInstr *lhdrPhi) {
  assert(TII->getGenericOpcode(lhdrPhi->getOpcode()) ==
      CSA::Generic::PICK && "loop predicate must be a PICK instruction");
  unsigned ctlreg = lhdrPhi->getOperand(1).getReg();
  if (loopPredicateGroups.find(ctlreg) == loopPredicateGroups.end()) {
    auto licGroup = std::make_shared<CSALicGroup>();

    // Assign frequency for the new group. Since we're based off of a PICK, it
    // is the case that the frequency of the output is the sum of its two input
    // parameters.
    auto opA = lhdrPhi->getOperand(2);
    auto opB = lhdrPhi->getOperand(3);
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

void CSASeqOpt::annotateBackedges() {
  DenseMap<unsigned, unsigned> AnnotatedControls;

  for (MachineInstr *MI : NewDrivenOps) {
    unsigned CtlInput;
    if (TII->isSeq(MI)) {
      continue;
    } else if (TII->isReduction(MI)) {
      // These have control inputs that could be included, but they are going to
      // be replaced in a later pass (in normal operation) anyways, so skip
      // them.
      continue;
    } else {
      CtlInput = 1;
    }

    unsigned CtlReg = MI->getOperand(CtlInput).getReg();
    if (AnnotatedControls.find(CtlReg) == AnnotatedControls.end()) {
      // Check to see who is driving the loop.
      MachineInstr *CtlSrc = MRI->getUniqueVRegDef(CtlReg);
      if (CtlSrc && TII->isSeq(CtlSrc)) {
        // If it's a sequence operation, there's no backedge possible.
        AnnotatedControls.insert(std::make_pair(CtlReg, CtlReg));
      } else if (!CtlSrc) {
        // CtlReg already has the INIT on itself directly. Just add the
        // annotation to this backedge.
        LMFI->addLICAttribute(CtlReg, "csasim_backedge");
        AnnotatedControls.insert(std::make_pair(CtlReg, CtlReg));
      } else {
        // Create a mov instruction to hang the annotation off of. Even if the
        // instruction here might not be directly implicated in a cycle, we
        // generally want to ignore this edge for viewing a graph as a DAG
        // anyways, since we expect the control edge to appear after the output
        // has been generated.
        unsigned MovReg = LMFI->allocateLIC(&CSA::CI1RegClass);
        AnnotatedControls.insert(std::make_pair(CtlReg, MovReg));
        BuildMI(*CtlSrc->getParent(), CtlSrc->getNextNode(),
            CtlSrc->getDebugLoc(), TII->get(CSA::MOV1), MovReg)
          .addReg(CtlReg)
          .setMIFlag(MachineInstr::NonSequential);
        LMFI->addLICAttribute(MovReg, "csasim_backedge");
        LMFI->setLICGroup(MovReg, LMFI->getLICGroup(CtlReg));
      }
    }

    // Replace the register with the new one, which may be the same one it was
    // originally.
    unsigned NewReg = AnnotatedControls[CtlReg];
    MI->getOperand(CtlInput).setReg(NewReg);
  }
}
