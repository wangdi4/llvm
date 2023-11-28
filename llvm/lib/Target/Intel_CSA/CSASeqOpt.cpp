//===-- CSASeqOpt.cpp - Sequence operator optimization --------------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
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

static cl::opt<bool> DisableLCC(
  "csa-disable-loop-carry-collapse", cl::Hidden,
  cl::desc("CSA Specific: Disable loop carry collapse optimization"),
  cl::init(false));

CSASeqOpt::CSASeqOpt(MachineFunction *F, MachineOptimizationRemarkEmitter &ORE,
                     CSALoopInfoPass &LI, const char *PassName) :
  ORE(ORE), LI(LI), PassName(PassName) {
  thisMF = F;
  TII    = static_cast<const CSAInstrInfo *>(
    thisMF->getSubtarget<CSASubtarget>().getInstrInfo());
  MRI  = &thisMF->getRegInfo();
  LMFI = thisMF->getInfo<CSAMachineFunctionInfo>();
  TRI  = thisMF->getSubtarget<CSASubtarget>().getRegisterInfo();

  NextLoopId = LI.numLoops() + 1;
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

  doLoopCarryCollapse();

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

template <typename Callable>
static MachineInstr *getSrcIfMatches(MachineRegisterInfo *MRI,
    unsigned Reg, Callable Func) {
  MachineInstr *Src = MRI->getVRegDef(Reg);
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

void CSASeqOpt::doLoopCarryCollapse() {
  if (DisableLCC) return;

  // We use a temporary vector to record the new loops
  // created during LCC so as not to perturb the iterators on LI.
  std::vector<CSALoopInfo> NewLoops;
  auto Loop = LI.rbegin();
  while (Loop != LI.rend()) {
    LccCanvas Canvas(this, Loop, NewLoops);
    Canvas.startLCC();
    Loop++;
  }

  // Merge the new loops into LI to enable them to undergo
  // further optimizations based on LI.
  for (auto &Loop : NewLoops) {
    LI.addLoop(Loop);
  }
}

// The code that follows should probably go into its own file
// to keep the size of this file manageable.
void CSASeqOpt::LccCanvas::startLCC() {
  auto &Loop = OutermostLoop;

  if (!isLoopLccEligible(*Loop)) {
    return;
  }

  auto ExitSwitches = (*Loop).getExitSwitches(0);
  unsigned BackedgeIndex = (*Loop).getPickBackedgeIndex();

  for (auto &PickInst : (*Loop).getHeaderPicks()) {

    // The matching switch instruction of the pick.
    MachineInstr *SwitchInst = getSrcIfMatches(MRI,
      PickInst->getOperand(BackedgeIndex + 2),
      [this, ExitSwitches](MachineInstr *MI) {
        return TII->isSwitch(MI) && is_contained(ExitSwitches, MI);
      });

    if (!SwitchInst) continue;

    if (!isPairLccEligible(PickInst, SwitchInst, BackedgeIndex)) {
      continue;
    }

    auto CLoop = Loop;
    for (;;) {
      if (CurrentRegion.Sections.size() == 0) {
        // The first loop we consider. Check if the loop has an enclosing if.
        if (!addOutermostIfLoop(PickInst, SwitchInst, BackedgeIndex)) {
          if (!addOutermostLoop(PickInst, SwitchInst, BackedgeIndex)) {
            break;
          }
        }
        CLoop++;
        continue;
      } else if (!addInnerIf()) {
        if (CLoop != LI.rend() && addInnerLoop(*CLoop)) {
          CLoop++;
        } else {
          finalizeCurrentRegion();
          break;
        }
      }
    }
  }
  finalize();
}

bool CSASeqOpt::LccCanvas::isLoopLccEligible(const CSALoopInfo& Loop) {
  // We only consider loops with one exit and with aligned
  // backedge indexes.
  if (Loop.getNumExits() != 1) return false;
  if (Loop.getPickBackedgeIndex() != Loop.getSwitchBackedgeIndex(0))
    return false;

  return true;
}

bool CSASeqOpt::LccCanvas::isPairLccEligible(MachineInstr *PickInstr,
                                  MachineInstr *SwitchInstr,
                                  unsigned BackedgeIndex) {
  LLVM_DEBUG({
      dbgs() << "isPairLccEligible:\n";
      dbgs() << "  " << *PickInstr;
      dbgs() << "  " << *SwitchInstr;
    });

  // The backedge of the switch should have exactly one use.
  if (!MRI->hasOneNonDBGUse(SwitchInstr->getOperand(BackedgeIndex).getReg())) {
    LLVM_DEBUG(dbgs() << " false - Switch's backedge has multiple uses\n");
    return false;
  }

  unsigned PredReg = PickInstr->getOperand(1).getReg();

  if (MRI->hasOneDef(PredReg)) {
    LLVM_DEBUG(dbgs() << " false - PredReg doesn't have multiple defs\n");
    return false;
  }

  MachineRegisterInfo::def_instr_iterator DefI =
    MRI->def_instr_begin(PredReg);
  MachineRegisterInfo::def_instr_iterator DefInext = std::next(DefI);
  assert(std::next(DefInext) == MachineRegisterInfo::def_instr_end());

  if (!(TII->isMOV(&*DefI) && TII->isInit(&*DefInext)) &&
      !(TII->isInit(&*DefI) && TII->isMOV(&*DefInext))) {
    LLVM_DEBUG(dbgs() << " false - Defs are not by INIT and MOV\n");
    return false;
  }

  MachineInstr *InitInstr = TII->isInit(&*DefI) ? &*DefI : &*DefInext;

  // Check INIT's value is consistent with BackedgeIndex.
  MachineOperand &MO = InitInstr->getOperand(1);
  unsigned ExpectedInitImm = (BackedgeIndex == 1) ? 0 : 1;
  if (!MO.isImm() || MO.getImm() != ExpectedInitImm) {
    LLVM_DEBUG(dbgs() << " false - INIT is not " << ExpectedInitImm << "\n");
    return false;
  }

  // The MOV should have the same def as the pred to the switch.
  MachineInstr *MovInstr  = TII->isMOV(&*DefI) ? &*DefI : &*DefInext;
  if (SwitchInstr->getOperand(2).getReg()
      != MovInstr->getOperand(1).getReg()) {
    LLVM_DEBUG(dbgs() << " false - Pick and Switch don't share the same PredReg\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << " true\n");
  return true;
}

auto CSASeqOpt::LccCanvas::assignLicGroup(unsigned Reg, ScaledNumber<uint64_t> Frequency,
                               unsigned LoopId) {
  auto Res = std::make_shared<CSALicGroup>();
  Res->LoopId = LoopId;
  Res->executionFrequency = Frequency;
  LMFI->setLICGroup(Reg, Res);
  return Res;
}

bool CSASeqOpt::LccCanvas::addOutermostIfLoop(MachineInstr *LoopHead,
                                   MachineInstr *LoopTail,
                                   unsigned BackedgeIndex) {
  LLVM_DEBUG(dbgs() << "AddOutermostIfLoop\n");

  unsigned InitOpIdx = (BackedgeIndex == 0) ? 3 : 2;
  MachineInstr *IfHead = getSrcIfMatches(MRI,
    LoopHead->getOperand(InitOpIdx), [this](MachineInstr *MI) {
      return TII->isSwitch(MI);
    });
  if (!IfHead) {
    LLVM_DEBUG(dbgs() << " false - Outermost pick doesn't consume from a switch\n");
    return false;
  }

  // IfHead's outputs need to be one-use.
  if (!MRI->hasOneNonDBGUse(IfHead->getOperand(0).getReg()) ||
      !MRI->hasOneNonDBGUse(IfHead->getOperand(1).getReg())) {
    LLVM_DEBUG(dbgs() << " false - If switch's defs have multiple uses\n");
    return false;
  }

  // LoopTail's non-backedge output needs to be one-use.
  if (!MRI->hasOneNonDBGUse(LoopTail->getOperand(1 - BackedgeIndex).getReg())) {
    LLVM_DEBUG(dbgs() << " false - Loop switch's result has multiple uses\n");
    return false;
  }

  unsigned FallThroughIdx = (LoopHead->getOperand(InitOpIdx).getReg()
                             == IfHead->getOperand(1).getReg()) ? 0 : 1;
  unsigned SwitchToPickIdx = (FallThroughIdx == 0) ? 3 : 2;


  // Find a pick instruction that uses both LoopTail's
  // and ifHead's outputs.
  MachineInstr *IfTail = nullptr;
  // We have already checked a few lines up that there is exactly one nondbg use
  // for IfHead's outputs.
  MachineInstr *UI =
    &(*(MRI->use_instr_nodbg_begin(IfHead->getOperand(FallThroughIdx).getReg())));
  if (TII->isPick(UI) &&
      (UI->getOperand(SwitchToPickIdx).getReg()
       == LoopTail->getOperand(1 - BackedgeIndex).getReg())) {
    IfTail = UI;
  }

  if (!IfTail) {
    LLVM_DEBUG(dbgs() << " false - Can't find a pick the If switch feeds into\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "  Enclosing IfHead: " << *IfHead);
  LLVM_DEBUG(dbgs() << "  Enclosing IfTail: " << *IfTail);

  // The I/O LICs after the transformation.
  CurrentRegion.W = IfTail->getOperand(0).getReg();
  CurrentRegion.X = IfHead->getOperand(3).getReg();
  CurrentRegion.Y = LoopTail->getOperand(3).getReg();
  CurrentRegion.Z = LoopHead->getOperand(0).getReg();

  unsigned IfPredReg = IfTail->getOperand(1).getReg();

  CurrentRegion.Sections.emplace_back(
    If, FallThroughIdx == 0, IfHead, IfTail, IfPredReg);
  CurrentRegion.Sections.emplace_back(
    Loop, BackedgeIndex == 1, LoopHead, LoopTail,
    LoopTail->getOperand(2).getReg());
  CurrentRegion.Operations.emplace_back(TransformOutermostIfLoop);

  return true;
}

// Should this take a CSALoopInfo as an argument instead?
bool CSASeqOpt::LccCanvas::addOutermostLoop(MachineInstr *Head, MachineInstr *Tail,
                                 unsigned BackedgeIndex) {
  LLVM_DEBUG(dbgs() << "AddOutermostLoop\n");

  // Z needs to be one-use.
  if (!MRI->hasOneNonDBGUse(Head->getOperand(0).getReg())) {
    LLVM_DEBUG(dbgs() << " false - Loop pick's result has multiple uses\n");
    return false;
  }

  CurrentRegion.W = Tail->getOperand(1 - BackedgeIndex).getReg();
  CurrentRegion.X = Head->getOperand(BackedgeIndex ? 2 : 3).getReg();
  CurrentRegion.Y = Tail->getOperand(3).getReg();
  CurrentRegion.Z = Head->getOperand(0).getReg();

  CurrentRegion.Sections.emplace_back(Loop, BackedgeIndex == 1, Head, Tail,
                                      Tail->getOperand(2).getReg());
  CurrentRegion.Operations.emplace_back(TransformOutermostLoop);

  return true;
}

bool CSASeqOpt::LccCanvas::isOperationWithIdentity(MachineInstr *Operation) {
  unsigned Opcode = Operation->getOpcode();
  unsigned Class = TII->getOpcodeClass(Opcode);
  // SIMD ops are not supported at the moment.
  if (Class != CSA::OpcodeClass::VARIANT_INT &&
      Class != CSA::OpcodeClass::VARIANT_FLOAT &&
      Class != CSA::OpcodeClass::VARIANT_SIGNED &&
      Class != CSA::OpcodeClass::VARIANT_UNSIGNED) {
    return false;
  }

  switch (TII->getGenericOpcode(Opcode)) {
  case CSA::Generic::AND:
  case CSA::Generic::OR:
  case CSA::Generic::ADD:
  case CSA::Generic::MUL:
    return true;
  default:
    break;
  }

  return false;
}

bool CSASeqOpt::LccCanvas::isOperationStateless(MachineInstr *Operation) {
  switch (TII->getGenericOpcode(Operation->getOpcode())) {
  case CSA::Generic::AND:
  case CSA::Generic::OR:
  case CSA::Generic::XOR:
  case CSA::Generic::SLL:
  case CSA::Generic::SRL:
  case CSA::Generic::SRA:
  case CSA::Generic::ADD:
  case CSA::Generic::ADC:
  case CSA::Generic::SUB:
  case CSA::Generic::SBB:
  case CSA::Generic::MUL:
  case CSA::Generic::DIV:
  case CSA::Generic::POW:
  case CSA::Generic::ATAN2:
  case CSA::Generic::MIN:
  case CSA::Generic::MAX:
  case CSA::Generic::CMPLT:
  case CSA::Generic::CMPLE:
  case CSA::Generic::CMPGT:
  case CSA::Generic::CMPGE:
  case CSA::Generic::CMPEQ:
  case CSA::Generic::CMPNE:
  case CSA::Generic::SLADD:
  case CSA::Generic::FMA:
  case CSA::Generic::FMS:
  case CSA::Generic::FMRS:
    return true;
  default:
    break;
  }

  return false;
}

bool CSASeqOpt::LccCanvas::getIdIdx(MachineInstr *Merge,
                                    MachineInstr *Operation,
                                    unsigned& IdIdx) {
  // IdIdx is either 2 or 3.
  IdIdx = 2;

  const auto checkForId = [Merge, &IdIdx](int64_t Id) {

    const auto isNotId = [Merge, Id](unsigned IdIdx) {
      return !(Merge->getOperand(IdIdx).isImm() &&
               Merge->getOperand(IdIdx).getImm() == Id) &&
             !(Merge->getOperand(IdIdx).isFPImm() &&
               Merge->getOperand(IdIdx).getFPImm()->isExactlyValue(static_cast<double>(Id)));
    };

    if (isNotId(IdIdx)) {
      IdIdx++;
      if (isNotId(IdIdx)) {
        LLVM_DEBUG(dbgs() << "  Merge doesn't have identity as input.\n");
        return false;
      }
    }
    return true;
  };

  switch (TII->getGenericOpcode(Operation->getOpcode())) {
  case CSA::Generic::OR:
  case CSA::Generic::ADD:
    return checkForId(0);

  case CSA::Generic::MUL:
    return checkForId(1);

  case CSA::Generic::AND: {
    const TargetRegisterClass *TRC=
      TII->getRegisterClass(Operation->getOperand(0).getReg(), *MRI);
    if (TRC == &CSA::CI1RegClass) {
      return checkForId(0x1);
    } else if (TRC == &CSA::CI8RegClass) {
      return checkForId(0xFF);
    } else if (TRC == &CSA::CI16RegClass) {
      return checkForId(0xFFFF);
    } else if (TRC == &CSA::CI32RegClass) {
      return checkForId(0xFFFFFFFF);
    } else if (TRC == &CSA::CI64RegClass) {
      return checkForId(0xFFFFFFFFFFFFFFFF);
    } else {
      assert(!"The AND instruction is not in the right register class");
    }
    break;
  }
  default:
    break;
  }

  return false;
}

bool CSASeqOpt::LccCanvas::addInnerIf() {
  auto isMerge = [this](MachineInstr *MI) {
    return TII->getGenericOpcode(MI->getOpcode()) == CSA::Generic::MERGE;
  };
  const auto isLogical = [](const MachineInstr *MI) {
    switch (MI->getOpcode()) {
    case CSA::OR1:
    case CSA::AND1:
      return true;
    default:
      return false;
    }
  };

  LLVM_DEBUG(dbgs() << "addInnerIf:\n");

  if (CurrentRegion.Sections.back().Type == MergeOp ||
      CurrentRegion.Sections.back().Type == OpMerge ||
      CurrentRegion.Sections.back().Type == Merge ||
      CurrentRegion.Sections.back().Type == Logical) {
    LLVM_DEBUG(
      dbgs() << "  false- The innermost construct is a merge or logical.\n");
    return false;
  }

  MachineInstr *OuterHead = CurrentRegion.Sections.back().HeadInstr;
  MachineInstr *OuterTail = CurrentRegion.Sections.back().TailInstr;

  (void)OuterTail;

  LLVM_DEBUG(dbgs() << "  OuterHead: " << *OuterHead);
  LLVM_DEBUG(dbgs() << "  OuterTail: " << *OuterTail);

  unsigned OuterTailInput = CurrentRegion.Y;

  MachineInstr *IfTail = getSrcIfMatches(
    MRI, OuterTailInput, [this, isMerge, isLogical](MachineInstr *MI) {
      return TII->isPick(MI) || isMerge(MI) || isLogical(MI) ||
             isOperationWithIdentity(MI);
    });

  if (!IfTail) {
    LLVM_DEBUG(dbgs() << " false - Innermost tail doesn't consume from a pick or an instruction with identity\n");
    return false;
  }

  if (IfTail == OuterHead) {
    LLVM_DEBUG(dbgs() << " false - Innermost tail consumes directly from innermost pick\n");
    return false;
  }

  // IfTail's output needs to be one-use.
  if (!MRI->hasOneNonDBGUse(IfTail->getOperand(0).getReg())) {
    LLVM_DEBUG(dbgs() << " false - If pick's output has multiple uses\n");
    return false;
  }

  if (TII->isPick(IfTail)) {
    // We are dealing with an explicit 'if'.
    MachineInstr *IfHead = nullptr;
    unsigned FallThroughIdx = 0;

    MachineRegisterInfo::use_iterator UO = MRI->use_begin(CurrentRegion.Z);
    while (UO != MRI->use_end()) {
      MachineInstr *UI = (*UO).getParent();
      if (TII->isSwitch(UI)) {
        if (UI->getOperand(0).getReg() == IfTail->getOperand(2).getReg()) {
          FallThroughIdx = 0;
        } else if (UI->getOperand(1).getReg() == IfTail->getOperand(3).getReg()) {
          FallThroughIdx = 1;
        } else {
          UO++;
          continue;
        }

        IfHead = UI;
        break;
      }
      UO++;
    }

    if (!IfHead) {
      LLVM_DEBUG(dbgs() << " false - Innermost pick doesn't feed into a switch\n");
      return false;
    }

    // IfHead's fallthrough needs to be one-use.
    if (!MRI->hasOneNonDBGUse(IfHead->getOperand(FallThroughIdx).getReg())) {
      LLVM_DEBUG(dbgs() << " false - If switch's fallthrough has multiple uses\n");
      return false;
    }

    LLVM_DEBUG(dbgs() << "  IfHead: " << *IfHead);
    LLVM_DEBUG(dbgs() << "  IfTail: " << *IfTail);

    CurrentRegion.Y = IfTail->getOperand(FallThroughIdx == 0 ? 3 : 2).getReg();
    CurrentRegion.Z = IfHead->getOperand(FallThroughIdx == 0 ? 1 : 0).getReg();

    CurrentRegion.Sections.emplace_back(If, FallThroughIdx == 0, IfHead, IfTail,
                                        IfTail->getOperand(1).getReg());
    CurrentRegion.Operations.emplace_back(TransformInnerIf);

  } else if (isMerge(IfTail)) {
    // Ensure the merge has the OuterHead as a src.
    MachineInstr *Merge = IfTail;
    unsigned Idx = 2;

    MachineInstr *Instr = getSrcIfMatches(MRI,
      Merge->getOperand(Idx), [OuterHead](MachineInstr *MI) {
        return MI == OuterHead;
      });

    if (!Instr) {
      Idx++;
      Instr = getSrcIfMatches(MRI,
        Merge->getOperand(Idx), [OuterHead](MachineInstr *MI) {
          return MI == OuterHead;
        });
      if (!Instr) {
        LLVM_DEBUG(dbgs() << "  The merge doesn't consume from OuterHead.\n");
        return false;
      }
    }

    // If the merge is the only user of OuterHead, this is the InnerMerge case.
    if (MRI->hasOneNonDBGUse(Merge->getOperand(Idx).getReg())) {

      // TailInstr should point at the other operand, unless it's a literal.
      // Then it should point at the merge instead.
      const MachineOperand &OtherInput = Merge->getOperand(Idx == 2 ? 3 : 2);
      MachineInstr *const TailInstr =
        OtherInput.isReg() ? MRI->getUniqueVRegDef(OtherInput.getReg()) : Merge;

      // Set up the transform information. Y will be re-routed to a new filter
      // in fixupInnerMerge.
      CurrentRegion.Y = Merge->getOperand(0).getReg();
      CurrentRegion.Z = Merge->getOperand(Idx).getReg();
      CurrentRegion.Sections.emplace_back(SectionType::Merge, Idx == 2, Merge,
                                          TailInstr,
                                          Merge->getOperand(1).getReg());
      CurrentRegion.Operations.push_back(TransformInnerIf);

    } else {

      // Otherwise, ensure that the other input to the merge is a stateless op.
      MachineInstr *Op = getSrcIfMatches(
        MRI, Merge->getOperand(Idx == 2 ? 3 : 2),
        [this](MachineInstr *MI) { return isOperationStateless(MI); });

      if (!Op) {
        LLVM_DEBUG(
          dbgs() << "  The merge doesn't consume from a stateless op.\n");
        return false;
      }

      LLVM_DEBUG(dbgs() << "  See op: " << *Op);

      // Op's output needs to be one-use.
      if (!MRI->hasOneNonDBGUse(Op->getOperand(0).getReg())) {
        LLVM_DEBUG(dbgs() << "  Op's output has multiple uses\n");
        return false;
      }

      // Ensure the reg in 1. is also used by op.
      Register X = Merge->getOperand(Idx).getReg();
      int XIdx   = Op->findRegisterUseOperandIdx(X);
      if (XIdx == -1) {
        LLVM_DEBUG(dbgs() << "  The op doesn't consume from OuterHead\n");
        return false;
      }

      // Ensure that reg in 1. has exactly two uses.
      unsigned XUses =
        std::distance(MRI->use_nodbg_begin(X), MRI->use_nodbg_end());

      if (XUses != 2) {
        LLVM_DEBUG(dbgs() << "  OuterHead's output doesn't have 2 exact uses; "
                          << XUses << " found\n");
        return false;
      }

      // The LIC group for this is done in fixupInnerOpMerge.
      const TargetRegisterClass *TRC =
        TII->getRegisterClass(Op->getOperand(XIdx).getReg(), *MRI);
      unsigned SwitchToOp = LMFI->allocateLIC(TRC);

      // Y should be Op->getOperand(0).getReg(), but
      // usually it's Merge->getOperand(0).getReg() associated
      // with a variable in the source program. Therefore,
      // we use the latter, and adjust it at the codegen stage.
      CurrentRegion.Y = Merge->getOperand(0).getReg();
      CurrentRegion.Z = SwitchToOp;

      CurrentRegion.Sections.emplace_back(OpMerge, Idx == 2, Merge, Op,
                                          Merge->getOperand(1).getReg(),
                                          SwitchToOp, XIdx);
      CurrentRegion.Operations.emplace_back(TransformInnerIf);
    }
  } else if (isLogical(IfTail)) {

    // Ensure that the logical op is a direct user of OuterHead.
    unsigned HeadIdx;
    for (HeadIdx = 1; HeadIdx <= 2; ++HeadIdx) {
      const MachineInstr *const FoundHead = getSrcIfMatches(
        MRI, IfTail->getOperand(HeadIdx),
        [OuterHead](const MachineInstr *MI) { return MI == OuterHead; });
      if (FoundHead)
        break;
    }
    if (HeadIdx > 2) {
      LLVM_DEBUG(
        dbgs() << "  The logical op doesn't consume from OuterHead.\n");
      return false;
    }

    // And that it's the only user.
    if (!MRI->hasOneNonDBGUse(IfTail->getOperand(HeadIdx).getReg())) {
      LLVM_DEBUG(
        dbgs() << "  The logical op isn't the only user of OuterHead.\n");
      return false;
    }

    // And that the other input isn't a literal.
    const unsigned PredIdx = (HeadIdx == 1) ? 2 : 1;
    if (!IfTail->getOperand(PredIdx).isReg()) {
      LLVM_DEBUG(dbgs() << "  The logical op's pred input isn't a channel.\n");
      return false;
    }

    // Set up the transform information. Y will be re-routed to a mov of a
    // literal in fixupInnerLogical.
    CurrentRegion.Y = IfTail->getOperand(0).getReg();
    CurrentRegion.Z = IfTail->getOperand(HeadIdx).getReg();
    CurrentRegion.Sections.emplace_back(
      Logical, IfTail->getOpcode() == CSA::OR1, IfTail, IfTail,
      IfTail->getOperand(PredIdx).getReg());
    CurrentRegion.Operations.push_back(TransformInnerIf);

  } else {
    // We may be dealing with merge.
    MachineInstr *OpInstr = IfTail;
    LLVM_DEBUG(dbgs() << "  See op: " << *OpInstr);
    LLVM_DEBUG(dbgs() << "    Checking if-merge conversion can be done.\n");
    if ((OpInstr->getOperand(1).isReg() &&
         !MRI->hasOneNonDBGUse(OpInstr->getOperand(1).getReg())) ||
        (OpInstr->getOperand(2).isReg() &&
         !MRI->hasOneNonDBGUse(OpInstr->getOperand(2).getReg()))) {
      LLVM_DEBUG(dbgs() << "  false - one of the inputs to op is not one-use\n");
      return false;
    }

    unsigned OpIdx = 1;
    MachineInstr *Instr = getSrcIfMatches(MRI,
      OpInstr->getOperand(OpIdx), [OuterHead](MachineInstr *MI) {
        return MI == OuterHead;
      });

    if (!Instr) {
      OpIdx++;
      Instr = getSrcIfMatches(MRI,
        OpInstr->getOperand(OpIdx), [OuterHead](MachineInstr *MI) {
          return MI == OuterHead;
        });
      if (!Instr) {
        LLVM_DEBUG(dbgs() << "  The op doesn't consume from OuterHead.\n");
        return false;
      }
    }

    // Instr consumes from OuterHead. Now check if it also consumes from
    // a merge instruction.
    Instr = getSrcIfMatches(MRI,
                            OpInstr->getOperand(OpIdx == 1 ? 2 : 1), isMerge);

    if (!Instr) {
      LLVM_DEBUG(dbgs() << "  The op doesn't consume from MERGE.\n");
      return false;
    }

    LLVM_DEBUG(dbgs() << "  MERGE: " << *Instr);

    unsigned IdIdx;
    if (!getIdIdx(Instr, OpInstr, IdIdx)) return false;

    // The LIC group for this is done in fixupInnerMergeOp.
    const TargetRegisterClass *TRC =
      TII->getRegisterClass(OpInstr->getOperand(OpIdx).getReg(), *MRI);
    unsigned SwitchToOp  = LMFI->allocateLIC(TRC);

    CurrentRegion.Y = OpInstr->getOperand(0).getReg();
    CurrentRegion.Z = SwitchToOp;

    CurrentRegion.Sections.emplace_back(MergeOp, IdIdx == 2, Instr, OpInstr,
                                        Instr->getOperand(1).getReg(),
                                        SwitchToOp, OpIdx);
    CurrentRegion.Operations.emplace_back(TransformInnerIf);
  }

  return true;
}

bool CSASeqOpt::LccCanvas::addInnerLoop(const CSALoopInfo& Loop) {
  LLVM_DEBUG(dbgs() << "addInnerLoop:\n");

  auto &Section = CurrentRegion.Sections.back();
  if (Section.Type == MergeOp || Section.Type == OpMerge ||
      Section.Type == Merge || Section.Type == Logical) {
    LLVM_DEBUG(dbgs() << "  false- The innermost construct is a merge.\n");
    return false;
  }

  if (!isLoopLccEligible(Loop)) {
    LLVM_DEBUG(dbgs() << " false - Loop is not eligible\n");
    return false;
  }

  MachineInstr *HeadInstr = Section.HeadInstr;
  MachineInstr *TailInstr = Section.TailInstr;

  unsigned HeadDefOpIdx = (Section.Type == SectionType::Loop)
    ? 0
    : (Section.Canonical) ? 1 : 0;

  unsigned TailUseOpIdx = (Section.Type == SectionType::Loop)
    ? 3
    : (Section.Canonical) ? 3 : 2;

  MachineInstr *SwitchInstr =
    getSrcIfMatches(MRI, TailInstr->getOperand(TailUseOpIdx),
      [this](MachineInstr *MI) { return TII->isSwitch(MI); });

  if (!SwitchInstr) {
    LLVM_DEBUG(dbgs() << " false - The outer tail doesn't consume from a switch\n");
    return false;
  }

  bool SwitchIsFromLoop = false;
  for (auto &TailSwitch : Loop.getExitSwitches(0)) {
    if (TailSwitch == SwitchInstr) {
      SwitchIsFromLoop = true;
      break;
    }
 }

  if (!SwitchIsFromLoop) {
    LLVM_DEBUG(dbgs() << " SwitchInstr: " << *SwitchInstr);
    LLVM_DEBUG(dbgs() << " false - The switch is not from the loop\n");
    return false;
  }

  unsigned BackedgeIndex = Loop.getPickBackedgeIndex();
  MachineInstr *PickInstr = nullptr;
  MachineRegisterInfo::use_iterator UO =
    MRI->use_begin(SwitchInstr->getOperand(BackedgeIndex).getReg());
  while (UO != MRI->use_end()) {
    MachineInstr *UI = (*UO).getParent();
    if (TII->isPick(UI)) {
      bool PickIsFromLoop = false;
      for (auto &HeaderPick : Loop.getHeaderPicks()) {
        if (HeaderPick == UI) {
          PickIsFromLoop = true;
          break;
        }
      }

      if (PickIsFromLoop &&
          HeadInstr->getOperand(HeadDefOpIdx).getReg() ==
          UI->getOperand(BackedgeIndex ? 2 : 3).getReg()) {
        // The pick is from the loop and it takes the output
        // from the head of the innermost construct. We've
        // found the pick that may be eligible for LCC.
        PickInstr = UI;
        break;
      }
    }
    UO++;
  }

  if (!PickInstr) {
    LLVM_DEBUG(dbgs() << " false - No pick in the loop consumes from the head instr of the innermost construct\n");    
    return false;
  }

  if (!isPairLccEligible(PickInstr, SwitchInstr, BackedgeIndex)) {
    LLVM_DEBUG(dbgs() << " false - The pick switch pair is not eligible\n");
    return false;
  }

  // W needs to be one-use.
  if (!MRI->hasOneNonDBGUse(SwitchInstr->getOperand(1 - BackedgeIndex).getReg())) {
    LLVM_DEBUG(dbgs() << " false - Loop switch's result has multiple uses\n");
    return false;
  }

  CurrentRegion.Y = SwitchInstr->getOperand(3).getReg();
  CurrentRegion.Z = PickInstr->getOperand(0).getReg();

  LLVM_DEBUG(dbgs() << "  PickInstr: " << *PickInstr);
  LLVM_DEBUG(dbgs() << "  SwitchInstr: " << *SwitchInstr);

  CurrentRegion.Sections.emplace_back(SectionType::Loop, BackedgeIndex == 1,
                                      PickInstr, SwitchInstr,
                                      SwitchInstr->getOperand(2).getReg());
  CurrentRegion.Operations.emplace_back(TransformInnerLoop);

  return true;
}

void CSASeqOpt::LccCanvas::fixupInnerSections(
  CSASeqOpt::LccCanvas::Region &Region) {

  // Get the innermost section for this region.
  Section &InnermostSection = Region.Sections.back();

  // Call the appropriate fixupInner* method depending on its type.
  switch (InnermostSection.Type) {
  case MergeOp:
    return fixupInnerMergeOp(InnermostSection);
  case OpMerge:
    return fixupInnerOpMerge(InnermostSection);
  case Merge:
    return fixupInnerMerge(InnermostSection);
  case Logical:
    return fixupInnerLogical(InnermostSection);
  default:
    return; // No fixup needed for any other types of sections.
  }
}

MachineInstr* CSASeqOpt::LccCanvas::generatePredicate(CSASeqOpt::LccCanvas::Region &Region) {
  // This is a very simplistic way to generate the predicate stream.
  // An improvement would be to group the consecutive loops and ifs,
  // encoding them in lor and land, respectively.
  // Also, Region.Operations may be removed. We can get the same info
  // by looking at the section type and if the section is the first one.
  MachineInstr *LeadingInstr = nullptr;
  auto Section = Region.Sections.begin();
  for (auto Operation : Region.Operations) {
    switch (Operation) {
    case TransformOutermostIfLoop:
      LeadingInstr = transformOutermostIfLoop(*Section, *(Section + 1));
      Section++;
      break;
    case TransformOutermostLoop:
      LeadingInstr = transformOutermostLoop(*Section);
      break;
    case TransformInnerIf:
      LeadingInstr = transformInnerIf(LeadingInstr, *Section);
      break;
    case TransformInnerLoop:
      LeadingInstr = transformInnerLoop(LeadingInstr, *Section);
    }
    Section++;
  }

  return LeadingInstr;
}

void CSASeqOpt::LccCanvas::reExpandIfLoop(MachineInstr *Instr, CSASeqOpt::LccCanvas::Region &Region,
                               MachineInstr *&Pick, MachineInstr *&Switch) {
  // Use W because it's the output of the expanded if-loop construct.
  const TargetRegisterClass *TRC = TII->getRegisterClass(Region.W, *MRI);
  const unsigned SwitchOp = TII->makeOpcode(CSA::Generic::SWITCH, TRC);
  const unsigned PickOp   = TII->makeOpcode(CSA::Generic::PICK, TRC);

  auto LicGroupX = LMFI->getLICGroup(Region.X);
  auto LicGroupY = LMFI->getLICGroup(Region.Y);
  unsigned InnermostLoopBackedge = 0;
  unsigned InnermostLoopPredicate = 0;
  for (int i = Region.Sections.size() - 1; i >= 0; i--) {
    auto &Section = Region.Sections[i];
    if (Section.Type == Loop) {
      InnermostLoopBackedge = Section.TailInstr
        ->getOperand(Section.Canonical ? 1 : 0).getReg();
      InnermostLoopPredicate = Section.Predicate;
      break;
    }
  }
  auto LicGroupI = LMFI->getLICGroup(InnermostLoopBackedge);
  auto LicGroupP = LMFI->getLICGroup(InnermostLoopPredicate);

  unsigned NextLoopId = Parent->NextLoopId;

  unsigned FallThrough = LMFI->allocateLIC(TRC);
  if (LicGroupX && LicGroupY && LicGroupI) {
    assignLicGroup(FallThrough,
                   LicGroupX->executionFrequency - LicGroupY->executionFrequency
                   + LicGroupI->executionFrequency);
  }
  unsigned Backedge    = LMFI->allocateLIC(TRC);
  if (LicGroupI && LicGroupP && LicGroupY) {
    assignLicGroup(Backedge,
                   LicGroupI->executionFrequency - LicGroupP->executionFrequency
                   + LicGroupY->executionFrequency,
                   NextLoopId);
  }
  unsigned IfToLoop    = LMFI->allocateLIC(TRC);
  unsigned LoopToIf    = LMFI->allocateLIC(TRC);
  if (LicGroupY && LicGroupI) {
    auto LicGroup = assignLicGroup(IfToLoop,
      LicGroupY->executionFrequency- LicGroupI->executionFrequency, NextLoopId);
    LMFI->setLICGroup(LoopToIf, LicGroup);
  }

  unsigned SnullOut = LMFI->allocateLIC(&CSA::CI1RegClass);
  if (LicGroupX) {
    assignLicGroup(SnullOut, LicGroupX->executionFrequency);
  }
  unsigned LastOut  = LMFI->allocateLIC(&CSA::CI1RegClass);
  unsigned LoopPred = LMFI->allocateLIC(&CSA::CI1RegClass);
  if (LicGroupY) {
    auto LicGroup = assignLicGroup(LastOut,
                                   LicGroupY->executionFrequency, NextLoopId);
    LMFI->setLICGroup(LoopPred, LicGroup);
  }

  // If switch
  MachineInstr *NewInstr =
    BuildMI(*Instr->getParent(), Instr,
            DebugLoc(), TII->get(SwitchOp), IfToLoop)
    .addReg(FallThrough, RegState::Define)
    .addReg(SnullOut)
    .addReg(Region.X)
    .setMIFlag(MachineInstr::NonSequential);

  (void)NewInstr;

  LLVM_DEBUG({
      dbgs() << "reExpandIfLoop:\n";
      dbgs() << "  IfSwitch- " << *NewInstr;
    });

  // If pick
  NewInstr =
    BuildMI(*Instr->getParent(), Instr,
            DebugLoc(), TII->get(PickOp), Region.W)
    .addReg(SnullOut)
    .addReg(LoopToIf)
    .addReg(FallThrough)
    .setMIFlag(MachineInstr::NonSequential);

  LLVM_DEBUG(dbgs() << "  IfPick-" << *NewInstr);

  // Loop pick
  Pick = BuildMI(*Instr->getParent(), Instr,
          DebugLoc(), TII->get(PickOp), Region.Z)
    .addReg(LoopPred)
    .addReg(Backedge)
    .addReg(IfToLoop)
    .setMIFlag(MachineInstr::NonSequential);

  LLVM_DEBUG(dbgs() << "  LoopPick-" << *Pick);

  // Loop switch
  Switch =
    BuildMI(*Instr->getParent(), Instr,
          DebugLoc(), TII->get(SwitchOp), Backedge)
    .addReg(LoopToIf, RegState::Define)
    .addReg(LastOut)
    .addReg(Region.Y)
    .setMIFlag(MachineInstr::NonSequential);

  LLVM_DEBUG(dbgs() << "  LoopSwitch-" << *Switch);

  unsigned A = Instr->getOperand(0).getReg();

  // Snull
  NewInstr =
    BuildMI(*Instr->getParent(), Instr,
          DebugLoc(), TII->get(CSA::SNULL), SnullOut)
    .addReg(A)
    .setMIFlag(MachineInstr::NonSequential);

  LLVM_DEBUG(dbgs() << "  Snull-" << *NewInstr);

  // Last
  NewInstr =
    BuildMI(*Instr->getParent(), Instr,
          DebugLoc(), TII->get(CSA::LAST), LastOut)
    .addReg(A)
    .setMIFlag(MachineInstr::NonSequential);

  LLVM_DEBUG(dbgs() << "  Last-" << *NewInstr);

  // Mov
  NewInstr =
    BuildMI(*Instr->getParent(), Instr,
            DebugLoc(), TII->get(CSA::MOV1), LoopPred)
    .addReg(LastOut)
    .setMIFlag(MachineInstr::NonSequential);

  LLVM_DEBUG(dbgs() << "  Mov-" << *NewInstr);

  // Init
  NewInstr =
    BuildMI(*Instr->getParent(), Instr,
            DebugLoc(), TII->get(CSA::INIT1), LoopPred)
    .addImm(1)
    .setMIFlag(MachineInstr::NonSequential);

  LLVM_DEBUG(dbgs() << "  Init-" << *NewInstr);
}

MachineInstr *CSASeqOpt::LccCanvas::transformLoopPredicate(MachineInstr *LoopTail,
                                                bool Canonical) {
  LLVM_DEBUG(dbgs() << "transformLoopPredicate:\n");

  // Create the new predicate stream using the pred for the loop.
  unsigned LoopPredReg = LoopTail->getOperand(2).getReg();
  auto LicGroup = LMFI->getLICGroup(LoopPredReg);

  if (!Canonical) {
    LoopPredReg = Parent->negateRegister(LoopPredReg);
  }

  unsigned LccPredReg = LMFI->allocateLIC(&CSA::CI1RegClass, "lcc.loop.pred");
  if (LicGroup) {
    assignLicGroup(LccPredReg, LicGroup->executionFrequency);
  }

  MachineInstr *Result =
    BuildMI(*LoopTail->getParent(), LoopTail,
            LoopTail->getDebugLoc(),
            TII->get(CSA::REPLACE1), LccPredReg)
    .addReg(LoopPredReg)
    .addImm(1).addImm(0) // Replace {0} =>
    .addImm(2).addImm(2) // {0, 1}
    .setMIFlags(MachineInstr::NonSequential);

  LLVM_DEBUG({
      dbgs() << "  LoopTail: " << *LoopTail;
      dbgs() << "  result: " << *Result;
    });

  Result =
    BuildMI(*LoopTail->getParent(), LoopTail,
          LoopTail->getDebugLoc(),
          TII->get(CSA::INIT1), LccPredReg)
    .addImm(1)
    .setMIFlags(MachineInstr::NonSequential);

  LLVM_DEBUG(dbgs() << "  " << *Result);

  return Result;
}

MachineInstr *CSASeqOpt::LccCanvas::transformOutermostIfLoop(CSASeqOpt::LccCanvas::Section &IfSection,
                                                  CSASeqOpt::LccCanvas::Section &LoopSection) {
  // Create the new predicate stream for the loop.
  MachineInstr *Instr = transformLoopPredicate(LoopSection.TailInstr,
                                               LoopSection.Canonical);
  unsigned LoopPredReg = Instr->getOperand(0).getReg();
  auto LicGroup1 = LMFI->getLICGroup(LoopPredReg);

  // Create the new predicate stream using the pred for the combined if/loop.
  unsigned IfPredReg = IfSection.Predicate;

  if (!IfSection.Canonical) {
    IfPredReg = Parent->negateRegister(IfPredReg);
  }
  unsigned LccPredReg = LMFI->allocateLIC(&CSA::CI1RegClass, "lcc.if.loop.pred");

  // The fallthrough of the if.
  auto LicGroup2 = LMFI->getLICGroup(
    IfSection.HeadInstr->getOperand(IfSection.Canonical ? 0 : 1).getReg());

  if (LicGroup1 && LicGroup2) {
    assignLicGroup(LccPredReg,
                   LicGroup1->executionFrequency + LicGroup2->executionFrequency);
  }

  MachineInstr *Result =
    BuildMI(*Instr->getParent(), Instr, Instr->getDebugLoc(),
            TII->get(CSA::STPICK1), LccPredReg)
    .addReg(CSA::IGN, RegState::Define)
    .addReg(IfPredReg)
    .addImm(0).addImm(0)
    .addReg(LoopPredReg)
    .addImm(0)
    .setMIFlags(MachineInstr::NonSequential);

  LLVM_DEBUG({
      dbgs() << "transformOutermostIfLoop:\n";
      dbgs() << "  IfSection:";
      dbgs() << "    type: " << IfSection.Type << "\n";
      dbgs() << "    head: " << *IfSection.HeadInstr;
      dbgs() << "    tail: " << *IfSection.TailInstr;
      dbgs() << "  LoopSection:";
      dbgs() << "    type: " << LoopSection.Type << "\n";
      dbgs() << "    head: " << *LoopSection.HeadInstr;
      dbgs() << "    tail: " << *LoopSection.TailInstr;
      dbgs() << "  Result: " << *Result;
    });

  return Result;
}

MachineInstr *CSASeqOpt::LccCanvas::transformOutermostLoop(CSASeqOpt::LccCanvas::Section &Section) {
  return transformLoopPredicate(Section.TailInstr, Section.Canonical);
}

MachineInstr *CSASeqOpt::LccCanvas::transformInnerIf(MachineInstr *Instr,
                                          CSASeqOpt::LccCanvas::Section &Section) {
  // Create the new predicate stream using the pred for the if.
  unsigned IfPredReg = Section.Predicate;

  if (!Section.Canonical) {
    IfPredReg = Parent->negateRegister(IfPredReg);
  }
  unsigned APredReg = Instr->getOperand(0).getReg();
  auto LicGroupA = LMFI->getLICGroup(APredReg);

  unsigned FallThrough;
  switch (TII->getGenericOpcode(Section.HeadInstr->getOpcode())) {
  case CSA::Generic::MERGE:
  case CSA::Generic::OR:
  case CSA::Generic::AND:
    FallThrough = Section.HeadInstr->getOperand(1).getReg();
    break;
  case CSA::Generic::SWITCH:
    FallThrough =
      Section.HeadInstr->getOperand((Section.Canonical) ? 0 : 1).getReg();
    break;
  default:
    llvm_unreachable("Unexpected head instr opcode");
  }
  auto LicGroupFT = LMFI->getLICGroup(FallThrough);

  unsigned LccPredReg = LMFI->allocateLIC(&CSA::CI1RegClass, "lcc.if.pred");
  if (LicGroupA && LicGroupFT) {
    assignLicGroup(LccPredReg,
                   LicGroupA->executionFrequency -
                   LicGroupFT->executionFrequency);
  }

  MachineInstr *Result =
    BuildMI(*Instr->getParent(), Instr, Instr->getDebugLoc(),
            TII->get(CSA::PREDFILTER), LccPredReg)
      .addReg(APredReg)
      .addReg(IfPredReg)
      .setMIFlags(MachineInstr::NonSequential);

  LLVM_DEBUG({
      dbgs() << "transformInnerIf:\n";
      dbgs() << "  " << *Result;
    });

  return Result;
}

void CSASeqOpt::LccCanvas::fixupInnerMergeOp(
  CSASeqOpt::LccCanvas::Section &Section) {

  LLVM_DEBUG(dbgs() << "fixupInnerMergeOp:\n");
  MachineInstr *MergeInstr = Section.HeadInstr;
  MachineInstr *OpInstr = Section.TailInstr;
  auto LicGroup = LMFI->getLICGroup(MergeInstr->getOperand(0).getReg());

  bool Canonical = Section.Canonical;
  unsigned SwitchToOp = Section.SwitchToOp;

  const TargetRegisterClass *TRC =
    TII->getRegisterClass(MergeInstr->getOperand(0).getReg(), *MRI);
  const unsigned FilterOp = TII->makeOpcode(CSA::Generic::FILTER, TRC);

  unsigned FilterToOp  = LMFI->allocateLIC(TRC);
  // We add extra buffer here to account for the delay in 'last'
  // introduced in if-loop re-expansion in order to avoid deadlock.
  LMFI->setLICDepth(FilterToOp, 1);
  if (LicGroup) {
    auto Group = assignLicGroup(FilterToOp, LicGroup->executionFrequency /
                                ScaledNumber<uint64_t>(2, 0),
                                Parent->NextLoopId);
    LMFI->setLICGroup(SwitchToOp, Group);
  }

  unsigned MergePred = Section.Predicate;
  if (!Canonical) {
    MergePred = Parent->negateRegister(MergePred);
  }

  // New filter
  MachineInstr *Filter =
    BuildMI(*OpInstr->getParent(), OpInstr, DebugLoc(),
            TII->get(FilterOp), FilterToOp)
    .addReg(MergePred)
    .add(MergeInstr->getOperand((Canonical) ? 3 : 2))
    .setMIFlag(MachineInstr::NonSequential);

  (void)Filter;

  LLVM_DEBUG(dbgs() << "  New filter: " << *Filter);

  // Reroute Op's operands.
  unsigned OpIdx = Section.OpIdx;
  OpInstr->getOperand(OpIdx).setReg(SwitchToOp);
  OpInstr->getOperand(OpIdx == 1 ? 2 : 1).setReg(FilterToOp);

  LLVM_DEBUG(dbgs() << "  Re-routed op: " << *OpInstr);
}

void CSASeqOpt::LccCanvas::fixupInnerOpMerge(
  CSASeqOpt::LccCanvas::Section &Section) {

  LLVM_DEBUG(dbgs() << "fixupInnerOpMerge:\n");
  MachineInstr *MergeInstr = Section.HeadInstr;
  MachineInstr *OpInstr = Section.TailInstr;

  bool Canonical = Section.Canonical;

  unsigned MergePred = Section.Predicate;
  if (!Canonical) {
    MergePred = Parent->negateRegister(MergePred);
  }

  auto LicGroup = LMFI->getLICGroup(MergeInstr->getOperand(0).getReg());
  std::shared_ptr<CSALicGroup> NewLicGroup = nullptr;

  unsigned SwitchToOp = Section.SwitchToOp;
  unsigned OpIdx = Section.OpIdx;

  for (unsigned i = 0; i < OpInstr->getNumOperands(); i++) {
    MachineOperand &Operand = OpInstr->getOperand(i);

    if (!Operand.isReg())
      continue;

    if (!Operand.isUse())
      continue;

    if (i == OpIdx)
      continue;

    unsigned OpReg = Operand.getReg();

    const TargetRegisterClass *TRC =
      TII->getRegisterClass(OpReg, *MRI);
    const unsigned FilterOp = TII->makeOpcode(CSA::Generic::FILTER, TRC);

    unsigned FilterToOp  = LMFI->allocateLIC(TRC);
    // We add extra buffer here to account for the delay in 'last'
    // introduced in if-loop re-expansion in order to avoid deadlock.
    LMFI->setLICDepth(FilterToOp, 1);

    if (!NewLicGroup) {
      if (LicGroup) {
        NewLicGroup = assignLicGroup(FilterToOp, LicGroup->executionFrequency /
                                     ScaledNumber<uint64_t>(2, 0),
                                     Parent->NextLoopId);
        LMFI->setLICGroup(SwitchToOp, NewLicGroup);
      }
    } else {
      LMFI->setLICGroup(FilterToOp, NewLicGroup);
    }

    // New filter
    MachineInstr *Filter =
      BuildMI(*OpInstr->getParent(), OpInstr, DebugLoc(),
              TII->get(FilterOp), FilterToOp)
      .addReg(MergePred)
      .addReg(OpReg)
      .setMIFlag(MachineInstr::NonSequential);

    (void)Filter;

    LLVM_DEBUG(dbgs() << "  New filter: " << *Filter);

    // Route the new LIC to Op.
    Operand.setReg(FilterToOp);
  }

  // Reroute Op's other operands.
  OpInstr->getOperand(OpIdx).setReg(SwitchToOp);

  // Use MergeInstr's output LIC for OpInstr's output
  // because that LIC is most likely associated with a
  // variable in the source.
  OpInstr->getOperand(0).setReg(MergeInstr->getOperand(0).getReg());

  LLVM_DEBUG(dbgs() << "  Re-routed op: " << *OpInstr);
}

void CSASeqOpt::LccCanvas::fixupInnerMerge(
  CSASeqOpt::LccCanvas::Section &Section) {

  LLVM_DEBUG(dbgs() << "fixupInnerMerge:\n");

  // Replace the merge with a filter to get the correct value for Y.
  const unsigned FilterCtl = Section.Canonical
                               ? Section.Predicate
                               : Parent->negateRegister(Section.Predicate);
  const auto Filter =
    BuildMI(*Section.HeadInstr->getParent(), Section.HeadInstr,
            Section.HeadInstr->getDebugLoc(),
            TII->get(TII->adjustOpcode(Section.HeadInstr->getOpcode(),
                                       CSA::Generic::FILTER)),
            Section.HeadInstr->getOperand(0).getReg())
      .addUse(FilterCtl)
      .add(Section.HeadInstr->getOperand(Section.Canonical ? 3 : 2))
      .setMIFlag(MachineInstr::NonSequential);
  Section.HeadInstr->getOperand(0).setReg(CSA::IGN);

  (void)Filter;
  LLVM_DEBUG(dbgs() << "  New filter: " << *Filter);
}

void CSASeqOpt::LccCanvas::fixupInnerLogical(
  CSASeqOpt::LccCanvas::Section &Section) {

  LLVM_DEBUG(dbgs() << "fixupInnerLogical:\n");

  // Replace the logical op with a mov of the corresponding literal.
  const bool LiteralVal = (Section.HeadInstr->getOpcode() == CSA::OR1);
  const auto Mov =
    BuildMI(*Section.HeadInstr->getParent(), Section.HeadInstr,
            Section.HeadInstr->getDebugLoc(), TII->get(CSA::MOV1),
            Section.HeadInstr->getOperand(0).getReg())
      .addImm(LiteralVal)
      .setMIFlag(MachineInstr::NonSequential);
  Section.HeadInstr->getOperand(0).setReg(CSA::IGN);

  (void)Mov;
  LLVM_DEBUG(dbgs() << "  New mov: " << *Mov);
}

// Should this take a CSALoopInfo as an argument instead?
MachineInstr *CSASeqOpt::LccCanvas::transformInnerLoop(MachineInstr *Instr,
                                            CSASeqOpt::LccCanvas::Section &Section) {
  if (TII->isInit(Instr)) {
    unsigned InitOp0 = Instr->getOperand(0).getReg();
    MachineInstr *ReplaceInst = nullptr;
    MachineRegisterInfo::def_instr_iterator defI =
      MRI->def_instr_begin(InitOp0);
    if ((*defI).getOpcode() == CSA::REPLACE1) {
      ReplaceInst = &*defI;
    } else {
      ReplaceInst = &*(std::next(defI));
      assert(ReplaceInst->getOpcode() == CSA::REPLACE1);
    }

    LLVM_DEBUG({
        dbgs() << "transformInnerLoop:\n";
        dbgs() << "  Reuse REPLACE: " << *ReplaceInst;
      });

    unsigned APredReg = ReplaceInst->getOperand(1).getReg();
    unsigned B1PredReg = Section.Predicate;
    auto LicGroupB1 = LMFI->getLICGroup(B1PredReg);
    if (!Section.Canonical) {
      B1PredReg = Parent->negateRegister(B1PredReg);
    }
    unsigned LorPredReg = LMFI->allocateLIC(&CSA::CI1RegClass, "lcc.lor.pred");
    if (LicGroupB1) {
      assignLicGroup(LorPredReg, LicGroupB1->executionFrequency);
    }

    MachineInstr *LorInstr =
      BuildMI(*Instr->getParent(), Instr, Instr->getDebugLoc(),
              TII->get(CSA::LOR1), LorPredReg)
      .addReg(B1PredReg)
      .addReg(APredReg)
      .addImm(0)
      .addImm(0)
      .setMIFlags(MachineInstr::NonSequential);

    (void)LorInstr;

    LLVM_DEBUG(dbgs() << "  " << *LorInstr);

    ReplaceInst->getOperand(1).setReg(LorPredReg);

    // Update the frequency of the output of REPLACE1 to reflect
    // the rerouting.
    unsigned ReplaceOut = ReplaceInst->getOperand(0).getReg();
    auto LicGroupR = LMFI->getLICGroup(ReplaceOut);
    auto LicGroupA = LMFI->getLICGroup(APredReg);
    auto LicGroupB0 = LMFI->getLICGroup(
      Section.HeadInstr->getOperand(Section.Canonical ? 2 : 3).getReg());
    if (LicGroupB1 && LicGroupR && LicGroupA && LicGroupB0) {
      // This LIC has the same frequency as the output of nestrepeat1
      // below. See there for how it is derived.
      LicGroupR->executionFrequency = LicGroupA->executionFrequency -
        LicGroupB0->executionFrequency + LicGroupB1->executionFrequency;
    }

    return Instr;
    // LeadingPredStreamInstr doesn't need to be changed; it stills
    // points at INIT.
  } else {
    auto PredInst = transformLoopPredicate(Section.TailInstr, Section.Canonical);
    unsigned APredReg = Instr->getOperand(0).getReg();
    auto LicGroupA = LMFI->getLICGroup(APredReg);
    unsigned BPredReg = PredInst->getOperand(0).getReg();
    unsigned B1PredReg = Section.Predicate;
    auto LicGroupB1 = LMFI->getLICGroup(B1PredReg);
    auto LicGroupB0 = LMFI->getLICGroup(
      Section.HeadInstr->getOperand(Section.Canonical ? 2 : 3).getReg());
    unsigned LccPredReg = LMFI->allocateLIC(&CSA::CI1RegClass, "lcc.pred");
    if (LicGroupA && LicGroupB1 && LicGroupB0) {
      // This is collapsing outer A (== A1 + A0) and inner B (== B1 + B0).
      // The resulting stream is A0 + B1. Since the number of invocations
      // of the inner loop is the same as the number of the terminal tokens
      // of the inner loop, A1 == B0. Therefore,
      // A0 + B1 == (A - A1) + B1 == (A - B0) + B1
      assignLicGroup(LccPredReg,
                     LicGroupA->executionFrequency -
                     LicGroupB0->executionFrequency +
                     LicGroupB1->executionFrequency);
    }

    // Combine the two streams into one using nestrepeat.
    MachineInstr *Result =
      BuildMI(*PredInst->getParent(), PredInst,
              PredInst->getDebugLoc(),
              TII->get(CSA::NESTREPEAT1), LccPredReg)
        .addReg(APredReg)
        .addReg(BPredReg)
        .setMIFlags(MachineInstr::NonSequential);

    LLVM_DEBUG({
        dbgs() << "transformInnerLoop:\n";
        dbgs() << "  " << *Result;
      });

    return Result;
  }
}

bool CSASeqOpt::LccCanvas::sharingPredicate(Region &Region1, Region &Region2) {
  if (Region1.Operations.size() != Region2.Operations.size() ||
      Region1.Sections.size() != Region2.Sections.size()) {
    return false;
  }

  for (unsigned i = 0; i < Region1.Sections.size(); i++) {
    Section &Section1 = Region1.Sections[i];
    Section &Section2 = Region2.Sections[i];
    if (Section1.Predicate != Section2.Predicate) return false;
    if (Section1.Canonical != Section2.Canonical) return false;
  }

  return true;
}

void CSASeqOpt::LccCanvas::finalizeCurrentRegion() {
  auto &Sections = CurrentRegion.Sections;

  // We continue with the transformation (i.e. add a region
  // for code generation) only when there are more than
  // 2 constructs or when there are 2 and the first one
  // is a loop.
  if (Sections.size() > 2 || (Sections.size() == 2 && Sections[0].Type == Loop)) {
    LLVM_DEBUG(dbgs() << "Found a region!\n");
    Regions.emplace_back(CurrentRegion);

    LLVM_DEBUG(dbgs() << "finalizeCurrentRegion: \n");

    int i = 0;
    for (auto &Section : CurrentRegion.Sections) {
      (void)Section;
      LLVM_DEBUG({
          dbgs() << "  Section " << i << ": \n";
          dbgs() << "    type: " << Section.Type << "\n";
          dbgs() << "    head: " << *Section.HeadInstr;
          dbgs() << "    tail: " << *Section.TailInstr;
        });
      i++;
    }
  }

  // Refresh CurrentRegion in the Canvas.
  CurrentRegion.W = 0;
  CurrentRegion.X = 0;
  CurrentRegion.Y = 0;
  CurrentRegion.Z = 0;

  CurrentRegion.Sections.clear();
  CurrentRegion.Operations.clear();
}

void CSASeqOpt::LccCanvas::finalize() {
  LLVM_DEBUG({
    dbgs() << "finalize:\n";
    dbgs() << "  Regions.size() is " << Regions.size() << "\n";
  });

  if (Regions.size() == 0) return;

  // Divide the regions into distinct groups. Regions
  // in the same group share the same predicate stream.
  // We will generate the instructions/LICs for the
  // predicate stream of each group.
  unsigned MaxGroupIdx = 0;
  std::vector<std::vector<int>> Groups;
  unsigned RegionIdx = 0;
  Groups.push_back(std::vector<int>());
  Groups[MaxGroupIdx++].push_back(RegionIdx);
  for (RegionIdx = 1; RegionIdx < Regions.size(); RegionIdx++) {
    unsigned k = 0;
    for (; k < MaxGroupIdx; k++) {
      if (sharingPredicate(Regions[RegionIdx], Regions[Groups[k][0]])) {
        Groups[k].push_back(RegionIdx);
        break;
      }
    }
    if (k == MaxGroupIdx) {
      // A new group.
      Groups.push_back(std::vector<int>());
      Groups[MaxGroupIdx++].push_back(RegionIdx);
    }
  }

  LLVM_DEBUG(dbgs() << "  Groups.size is " << Groups.size() << "\n");

  for (auto Group : Groups) {
    CSALoopInfo LoopInfo;
    LoopInfo.setPickBackedgeIndex(0);
    LoopInfo.addExit(0);

    // Fixup any sections that need it.
    for (auto RegionIdx : Group)
      fixupInnerSections(Regions[RegionIdx]);

    // Use the first region in the group to derive the predicate stream.
    MachineInstr *PredInstr = generatePredicate(Regions[Group[0]]);

    // If-loop re-expansion for each region in the group.
    for (auto RegionIdx : Group) {
      Region &Region = Regions[RegionIdx];
      MachineInstr* PickInstr = nullptr;
      MachineInstr* SwitchInstr = nullptr;
      reExpandIfLoop(PredInstr, Region, PickInstr, SwitchInstr);
      Parent->NextLoopId++;
      LLVM_DEBUG({
        dbgs() << "Pick and switch in new loop:\n";
        dbgs() << "  " << *PickInstr;
        dbgs() << "  " << *SwitchInstr;
        });

      // Record the header pick and switch of the new loop.
      LoopInfo.addHeaderPick(PickInstr);
      LoopInfo.addExitSwitch(0, SwitchInstr);

      auto CLoop = OutermostLoop;
      for (auto &Section : Region.Sections) {
        if (Section.Type == Loop) {
          // Drop the const-ness of *CLoop.
          CSALoopInfo &MLoop = const_cast<CSALoopInfo&>(*CLoop);
          MLoop.removePickSwitch(Section.HeadInstr, Section.TailInstr);
          CLoop++;
        }
      }
    }

    // Record the new loop.
    NewLoops.push_back(LoopInfo);
  }

  // Emit the optimization report.
  for (auto Region : Regions) {
    auto ORE = Parent->ORE;
    MachineOptimizationRemark Remark(Parent->PassName, "LCC: ",
                                     DebugLoc(), Region.Sections[0].HeadInstr->getParent());
    ORE.emit(Remark << "Loop carry collapse has been applied to the following nested constructs:");
    for (auto Section : Region.Sections) {
      MachineOptimizationRemark SubRemark(Parent->PassName, "LCC: ",
                                          (Section.Type == Loop)
                                            ? Section.TailInstr->getDebugLoc()
                                            : Section.HeadInstr->getDebugLoc(),
                                          Section.HeadInstr->getParent());
      switch (Section.Type) {
      case Loop:
        ORE.emit(SubRemark << " Loop");
        break;
      case If:
        ORE.emit(SubRemark << " If");
        break;
      case MergeOp:
      case OpMerge:
      case Merge:
        ORE.emit(SubRemark << " Merge");
        break;
      case Logical:
        ORE.emit(SubRemark << " Logical");
        break;
      }
    }
  }

  // Erase the dead instructions.
  for (auto Region : Regions) {
    for (auto Section : Region.Sections) {
      Section.HeadInstr->eraseFromParentAndMarkDBGValuesForRemoval();
      // Do not erase the TailInstr for Merge/Logical-- It's an operator or has
      // already been erased.
      if (Section.Type != MergeOp && Section.Type != OpMerge &&
          Section.Type != Merge && Section.Type != Logical)
        Section.TailInstr->eraseFromParentAndMarkDBGValuesForRemoval();
    }
  }
}
