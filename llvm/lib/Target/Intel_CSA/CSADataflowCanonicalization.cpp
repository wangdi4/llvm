//===-- CSADataflowCanonicalization.cpp - Canonicalize dataflow operations ===//
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
// This file implements a series of tools to provide simplifications and a
// canonicalized form for dataflow instructions.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSAInstrInfo.h"
#include "CSAMachineFunctionInfo.h"
#include "CSAUtils.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "csa-dfcanon"
#define PASS_NAME "CSA: Dataflow simplification pass"

STATISTIC(NumSwitchesAdded, "Number of switches added due to inversion");
STATISTIC(NumFiltersToOncounts, "Number of filter0s converted to oncount0s");

static cl::opt<bool>
  DisableSwitchInversion("csa-disable-swi", cl::Hidden,
                         cl::desc("CSA Specific: Disable switch inversion"));

namespace llvm {
/// This pass is structured as a series of smaller, mini passes. Each of these
/// passes could effectively be thought of as a MachineInstructionPass, in that
/// they operate on a single MachineInstr and return the same changed boolean
/// that passes normally return.
class CSADataflowCanonicalizationPass : public MachineFunctionPass {
public:
  static char ID;
  CSADataflowCanonicalizationPass();

  StringRef getPassName() const override {
    return PASS_NAME;
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  MachineFunction *MF;
  MachineRegisterInfo *MRI;
  CSAMachineFunctionInfo *LMFI;
  const CSAInstrInfo *TII;
  /// A list of instructions to be deleted. This is accumulated during a run of
  /// a mini pass, and is deleted after the mini pass is finished.
  std::vector<MachineInstr *> to_delete;

  /// The following mini pass implements a peephole pass that removes NOT
  /// operands from the control lines of picks and switches.
  bool eliminateNotPicks(MachineInstr *MI);

  /// The following mini pass replaces side-effect-free operations entering a
  /// a SWITCH one of whose outputs is ignored with SWITCHes into those
  /// operations. This helps for memory ordering issues.
  bool invertIgnoredSwitches(MachineInstr *MI);

  /// The following mini pass replaces SWITCH operations where one output
  /// is ignored with FILTER operations.
  bool createFilterOps(MachineInstr *MI);

  // The following replaces filter0s driven by sequencers with oncount0s.
  bool createOncountFromFilter(MachineInstr *MI);

  /// The following mini pass replaces MOV operations.
  bool eliminateMovInsts(MachineInstr *MI);

  /// The following mini pass pushes literals as late as possible.
  bool stopPipingLiterals(MachineInstr *MI);

  MachineInstr *getDefinition(const MachineOperand &MO) const;
  void getUses(const MachineOperand &MO,
               SmallVectorImpl<MachineInstr *> &uses) const;
  MachineInstr *getSingleUse(const MachineOperand &MO) const;

  /// Get the number of a LIC that is the negation of MO, creating one if it is
  /// necessary.
  unsigned getNotReg(MachineInstr *MI, const MachineOperand &MO) const;
};

void initializeCSADataflowCanonicalizationPassPass(PassRegistry &);
} // namespace llvm

char CSADataflowCanonicalizationPass::ID = 0;

INITIALIZE_PASS(CSADataflowCanonicalizationPass, DEBUG_TYPE, PASS_NAME,
                false, false)

CSADataflowCanonicalizationPass::CSADataflowCanonicalizationPass()
    : MachineFunctionPass(ID) {
  initializeCSADataflowCanonicalizationPassPass(
      *PassRegistry::getPassRegistry());
}

MachineFunctionPass *llvm::createCSADataflowCanonicalizationPass() {
  return new CSADataflowCanonicalizationPass();
}

bool CSADataflowCanonicalizationPass::runOnMachineFunction(
  MachineFunction &MF) {
  if (!shouldRunDataflowPass(MF))
    return false;

  this->MF = &MF;
  MRI      = &MF.getRegInfo();
  LMFI     = MF.getInfo<CSAMachineFunctionInfo>();
  TII      = static_cast<const CSAInstrInfo *>(
    MF.getSubtarget<CSASubtarget>().getInstrInfo());

  // Run several functions one at a time on the entire graph. There is probably
  // a better way of implementing this sort of strategy (like how InstCombiner
  // does its logic), but until we have a need to go a fuller InstCombiner-like
  // route, this logic will do. Note that we can't delete instructions on the
  // fly due to how iteration works, but we do clean them up after every mini
  // pass.
  bool changed          = false;
  static auto functions = {
    &CSADataflowCanonicalizationPass::eliminateNotPicks,
    &CSADataflowCanonicalizationPass::eliminateMovInsts,
    &CSADataflowCanonicalizationPass::createFilterOps,
    &CSADataflowCanonicalizationPass::invertIgnoredSwitches,
    &CSADataflowCanonicalizationPass::stopPipingLiterals,
    &CSADataflowCanonicalizationPass::createOncountFromFilter
  };
  for (auto func : functions) {
    if (func == &CSADataflowCanonicalizationPass::invertIgnoredSwitches &&
        DisableSwitchInversion)
      continue;
    for (auto &MBB : MF) {
      for (auto &MI : MBB) {
        changed |= (this->*func)(&MI);
      }
      for (auto MI : to_delete)
        MI->eraseFromParent();
      to_delete.clear();
    }
  }

  this->MF = nullptr;
  return changed || true;
}

MachineInstr *
CSADataflowCanonicalizationPass::getDefinition(const MachineOperand &MO) const {
  assert(MO.isReg() && "LICs to search for can only be registers");
  return MRI->getUniqueVRegDef(MO.getReg());
}

void CSADataflowCanonicalizationPass::getUses(
  const MachineOperand &MO, SmallVectorImpl<MachineInstr *> &uses) const {
  assert(MO.isReg() && "LICs to search for can only be registers");
  for (auto &use : MRI->use_instructions(MO.getReg())) {
    uses.push_back(&use);
  }
}

MachineInstr *
CSADataflowCanonicalizationPass::getSingleUse(const MachineOperand &MO) const {
  SmallVector<MachineInstr *, 4> uses;
  getUses(MO, uses);
  return uses.size() == 1 ? uses[0] : nullptr;
}

bool CSADataflowCanonicalizationPass::eliminateNotPicks(MachineInstr *MI) {
  unsigned select_op, low_op, high_op;
  if (TII->isSwitch(MI)) {
    select_op = 2;
    low_op    = 0;
    high_op   = 1;
  } else if (TII->isPick(MI)) {
    select_op = 1;
    low_op    = 2;
    high_op   = 3;
  } else {
    return false;
  }

  if (MachineInstr *selector = getDefinition(MI->getOperand(select_op))) {
    if (selector->getOpcode() == CSA::NOT1 &&
        // TODO (vzakhari 9/21/2018): CMPLRLLVM-5595 - fix the code
        //       to support immediate operands.
        MI->getOperand(low_op).isReg() &&
        MI->getOperand(high_op).isReg() &&
        MI->getOperand(select_op).isReg()) {
      // This means the selector is a NOT. Swap the two definitions on the
      // output, and change the selector to be the NOT's inverse.
      int reg_tmp = MI->getOperand(low_op).getReg();
      MI->getOperand(low_op).setReg(MI->getOperand(high_op).getReg());
      MI->getOperand(high_op).setReg(reg_tmp);
      MI->getOperand(select_op).setReg(selector->getOperand(1).getReg());
      return true;
    }
  }
  return false;
}

unsigned CSADataflowCanonicalizationPass::getNotReg(MachineInstr *MI,
    const MachineOperand &MO) const {
  auto InputReg = MO.getReg();

  // Try to find an existing NOT we can reuse.
  for (auto &Use : MRI->use_instructions(InputReg)) {
    if (Use.getOpcode() == CSA::NOT1) {
      return Use.getOperand(0).getReg();
    }
  }

  // No prior NOT? Generate one instead.
  unsigned InvertedReg = LMFI->allocateLIC(&CSA::CI1RegClass);
  BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), TII->get(CSA::NOT1),
          InvertedReg)
    .addReg(InputReg)
    ->setFlag(MachineInstr::NonSequential);
  LMFI->setLICGroup(InvertedReg, LMFI->getLICGroup(InputReg));
  return InvertedReg;
}

bool CSADataflowCanonicalizationPass::createFilterOps(MachineInstr *MI) {
  if (!TII->isSwitch(MI))
    return false;

  if (MI->getOperand(0).isReg() && MI->getOperand(0).getReg() == CSA::IGN) {
    // The first value is ignored.
    MI->setDesc(
      TII->get(TII->adjustOpcode(MI->getOpcode(), CSA::Generic::FILTER)));
    MI->RemoveOperand(0);
    return true;
  } else if (MI->getOperand(1).isReg() &&
             MI->getOperand(1).getReg() == CSA::IGN) {
    // Second value is ignored. We need to generate a NOT -> FILTER
    MI->setDesc(
      TII->get(TII->adjustOpcode(MI->getOpcode(), CSA::Generic::FILTER)));
    MI->RemoveOperand(1);
    MI->getOperand(1).setReg(getNotReg(MI, MI->getOperand(1)));
    return true;
  }

  return false;
}

// This will transform a filter of the form:
//   -, -, -, %end = seqotneN 0, %N, 1
//   %out = filter0 %end, %in
// with an oncount based on the sequencer's range:
//   -, -, -, %end = seqotneN 0, %N, 1
//   %out = oncount0 %N, %in
bool CSADataflowCanonicalizationPass::createOncountFromFilter(MachineInstr *MI) {
  if (MI->getOpcode() != CSA::FILTER0)
    return false;

  if (!MI->getOperand(1).isReg())
    return false;

  unsigned endStream = MI->getOperand(1).getReg();
  MachineInstr *seqInst = MRI->getUniqueVRegDef(endStream);
  if (!seqInst)
    return false;

  // Look for a sequencer with a known trip count. There are certainly other
  // cases, but for now we look for a seqotne counting from 0 to N by 1.
  if (TII->getGenericOpcode(seqInst->getOpcode()) != CSA::Generic::SEQOTNE)
    return false;

  // The control stream indicating the final value must be driving our filter0.
  if (!seqInst->getOperand(3).isReg() || seqInst->getOperand(3).getReg() != endStream)
    return false;

  // Look for a start of literal 0...
  if (!seqInst->getOperand(4).isImm() || seqInst->getOperand(4).getImm() != 0)
    return false;

  // ...and an increment of literal 1.
  if (!seqInst->getOperand(6).isImm() || seqInst->getOperand(6).getImm() != 1)
    return false;

  // We've decided that we can make the transformation in this case. Bump the
  // statistic.
  NumFiltersToOncounts++;

  // Before making any transformation, note whether or not the filter is the
  // only user of the control stream.
  bool exclusiveStreamUser = MRI->hasOneNonDBGUser(endStream);

  // We then know that the filter0 is simply going to eat N-1 zeroes (and data
  // values) and then emit a 0-bit data value on the Nth one. This can be
  // replaced by an oncount0 which eats N values and then emits a 0-bit data
  // value. The oncount potentially uses an additional integer resource, but in
  // return we eliminate the need to send a control stream to do the eating.
  MachineOperand &count = seqInst->getOperand(5);
  BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), TII->get(CSA::ONCOUNT0),
      MI->getOperand(0).getReg()).add(count).add(MI->getOperand(2))
    .addReg(CSA::NA).addReg(CSA::NA).addReg(CSA::NA)
    ->setFlag(MachineInstr::NonSequential);

  // If the endStream had no other users, redirect the sequencer's output to
  // IGN. This could make the sequencer a dead op.
  if (exclusiveStreamUser)
    seqInst->getOperand(3).setReg(CSA::IGN);

  //Finally, disconnect and schedule the filter0 for deletion.
  for (MachineOperand &opnd : MI->operands())
    if (opnd.isReg())
      opnd.setReg(CSA::NA);

  to_delete.push_back(MI);
  return true;
}

// This will transform a switch of the form:
//   %val = [simple op] %arg1, %arg2
//   %out, %ign = SWITCH %ctl, %val
// into:
//   %swarg1, %ign = SWITCH %ctl, %arg1
//   %swarg2, %ign = SWITCH %ctl, %arg2
//   %val = [simple op] %swarg1, %swarg2
bool CSADataflowCanonicalizationPass::invertIgnoredSwitches(MachineInstr *MI) {
  // The value must be a switch, and one of its outputs must be ignored.
  if (!TII->isSwitch(MI))
    return false;
  if ((!MI->getOperand(0).isReg() || MI->getOperand(0).getReg() != CSA::IGN) &&
      (!MI->getOperand(1).isReg() || MI->getOperand(1).getReg() != CSA::IGN)) {
    return false;
  } else if (!MI->getOperand(3).isReg()) {
    // Switching a constant value... pass doesn't apply.
    return false;
  }

  // In order for the transform to be legal, we need:
  // 1. Not doing the switched operation before the switch must not be
  //    observable. Memory operations, SXU operations, and sequence operations
  //    all fail this test.
  // 2. There must only be a single output to be switched.
  // 3. The output must only reach the SWITCH.
  MachineInstr *switched = getDefinition(MI->getOperand(3));
  if (!switched || switched->mayLoadOrStore() ||
      switched->hasUnmodeledSideEffects() || TII->isMultiTriggered(switched) ||
      !switched->getFlag(MachineInstr::NonSequential))
    return false;
  if (switched->uses().begin() - switched->defs().begin() > 1)
    return false;
  if (getSingleUse(switched->getOperand(0)) != MI)
    return false;

  // For performance reasons, we need the SWITCH to be filtering out a large
  // fraction of its input values. In lieu of a performance analysis pass, we
  // use the first/last of the sequence of the operator being a high confidence
  // of being such an operation.
  auto switchCtl                   = MI->getOperand(2);
  const MachineInstr *switchCtlDef = getDefinition(switchCtl);
  if (!switchCtlDef || !TII->isSeqOT(switchCtlDef))
    return false;
  if (switchCtlDef->getOperand(2).getReg() == switchCtl.getReg() &&
      switchCtlDef->getOperand(3).getReg() == switchCtl.getReg())
    return false;

  // Generate new SWITCH's for each operand of the switched operation. The
  // operation itself is modified in-place, so we need to fix up all the LIC
  // operands of the operation.
  bool is0Dead = MI->getOperand(0).getReg() == CSA::IGN;
  for (MachineOperand &MO : switched->operands()) {
    if (!MO.isReg())
      continue;
    if (MO.isDef()) {
      // We asserted above that the switched operand has exactly one definition,
      // so we only need to replace this with the output of the switch.
      MO.setReg(MI->getOperand(is0Dead ? 1 : 0).getReg());
    } else if (MO.getReg() != CSA::IGN && MO.getReg() != CSA::NA) {
      // Non-ignored registers mean that we need to allocate a new SWITCH here.
      // Insert the new SWITCH before the operand instruction to try to keep the
      // operations in topological order.
      auto licClass = TII->getRegisterClass(MO.getReg(), *MRI);
      auto newParam = (decltype(CSA::IGN))LMFI->allocateLIC(licClass);
      auto newSwitch =
        BuildMI(*switched->getParent(), switched, MI->getDebugLoc(),
                TII->get(TII->makeOpcode(CSA::Generic::SWITCH, licClass)))
          .addReg(is0Dead ? CSA::IGN : newParam, RegState::Define)
          .addReg(is0Dead ? newParam : CSA::IGN, RegState::Define)
          .add(MI->getOperand(2))
          .add(MO);
      newSwitch->setFlag(MachineInstr::NonSequential);
      MO.setReg(newParam);

      // We added a new switch, which might be eligible for this optimization.
      invertIgnoredSwitches(newSwitch);
      NumSwitchesAdded++;
    }
  }
  NumSwitchesAdded--;

  // Delete the old switch.
  to_delete.push_back(MI);

  return true;
}

bool CSADataflowCanonicalizationPass::stopPipingLiterals(MachineInstr *MI) {
  if (!MI->getFlag(MachineInstr::NonSequential))
    return false;

  const MachineOperand *Literal = nullptr;
  unsigned RegCheck;
  switch (TII->getGenericOpcode(MI->getOpcode())) {
  case CSA::Generic::MOV:
    Literal = &MI->getOperand(1);
    RegCheck = MI->getOperand(0).getReg();
    break;
  case CSA::Generic::FILTER:
  case CSA::Generic::GATE:
  case CSA::Generic::REPEAT:
  case CSA::Generic::REPEATO:
    Literal = &MI->getOperand(2);
    RegCheck = MI->getOperand(0).getReg();
    break;
  case CSA::Generic::SWITCH: {
    // For switches, if the input is constant, break the switch into two
    // filter operations.
    if (MI->getOperand(3).isReg())
      return false;

    auto FilterL = BuildMI(*MI->getParent(), MI, MI->getDebugLoc(),
            TII->get(TII->adjustOpcode(MI->getOpcode(), CSA::Generic::FILTER)))
      .add(MI->getOperand(0))
      .addReg(getNotReg(MI, MI->getOperand(2)))
      .add(MI->getOperand(3))
      .setMIFlag(MachineInstr::NonSequential);
    auto FilterR = BuildMI(*MI->getParent(), MI, MI->getDebugLoc(),
            TII->get(TII->adjustOpcode(MI->getOpcode(), CSA::Generic::FILTER)))
      .add(MI->getOperand(1))
      .add(MI->getOperand(2))
      .add(MI->getOperand(3))
      .setMIFlag(MachineInstr::NonSequential);

    // Set the original outputs on the switch to ignore. This keeps them from
    // showing up as defined.
    MI->getOperand(0).setReg(CSA::IGN);
    MI->getOperand(1).setReg(CSA::IGN);
    to_delete.push_back(MI);

    // Process the generated instructions.
    stopPipingLiterals(FilterL);
    stopPipingLiterals(FilterR);
    return true;
  }
  default:
    return false;
  }

  // To be viable for optimization, we need the register to not have an initial
  // value (i.e., this must be the only definition), and we need the literal to
  // be a constant value.
  if (Literal->isReg() || !MRI->getUniqueVRegDef(RegCheck))
    return false;

  // Helper lambda to check if all of the input operands would be literal
  // registers if this value were replaced. This is the most common criterion
  // for whether or not it is safe to use the literal.
  auto wouldAllBeLiterals = [=](const MachineInstr &use) {
    for (auto &UseOp : use.uses()) {
      if (UseOp.isReg() &&
          UseOp.getReg() != RegCheck && UseOp.getReg() != CSA::IGN)
        return false;
    }
    return true;
  };

  auto safeToUseLiteral = [=](const MachineInstr &use) {
    // These pseudo-instructions do not accept literals for obvious reasons.
    if (use.getOpcode() == CSA::CSA_RETURN ||
        use.getOpcode() == CSA::CSA_CALL)
      return false;

    switch (TII->getGenericOpcode(use.getOpcode())) {
    case CSA::Generic::ANY:
    case CSA::Generic::PICKANY:
    case CSA::Generic::SWITCHANY:
      // These values depend on LIC availability for consistency. Replacing with
      // a literal changes semantic meaning.
      return false;
    case CSA::Generic::PICK:
    case CSA::Generic::SWITCH:
    case CSA::Generic::FILTER:
      // Making everything be literal here enables further optimization. Don't
      // check if everything becomes literal--we want that to happen.
      return true;
    default:
      // If making everything become literal would cause it to continuously
      // fire, don't do anything.
      return !wouldAllBeLiterals(use);
    }
  };

  SmallVector<MachineInstr *, 4> InstsToReplace;
  bool ReplacedAllUses = true;
  for (auto &Use : MRI->use_instructions(RegCheck)) {
    // Check to see if it's safe to replace all uses of this value.
    if (safeToUseLiteral(Use)) {
      InstsToReplace.push_back(&Use);
    } else {
      ReplacedAllUses = false;
    }
  }
  for (auto &Use : InstsToReplace) {
    // Replace the value with the constant.
    for (auto &Op : Use->operands()) {
      if (Op.isReg() && Op.getReg() == RegCheck) {
        // We need this to clear bits from MRI register def/use tracking, since
        // the copy constructor is just the default one.
        Op.ChangeToImmediate(0);
        Op = *Literal;
      }
    }
  }
  if (ReplacedAllUses)
    to_delete.push_back(MI);
  return !InstsToReplace.empty();
}

bool CSADataflowCanonicalizationPass::eliminateMovInsts(MachineInstr *MI) {
  if (!MI->getFlag(MachineInstr::NonSequential))
    return false;

  if (TII->getGenericOpcode(MI->getOpcode()) != CSA::Generic::MOV &&
      MI->getOpcode() != CSA::COPY)
    return false;

  if (!MI->getOperand(0).isReg() || !MI->getOperand(1).isReg())
    return false;

  unsigned srcReg = MI->getOperand(1).getReg();
  unsigned destReg = MI->getOperand(0).getReg();

  // Moves involving physical registers should not be removed here
  if (!Register::isVirtualRegister(srcReg))  return false;
  if (!Register::isVirtualRegister(destReg)) return false;
  if (!MRI->getUniqueVRegDef(destReg))       return false;

  // If the opcode is a generic copy, replace it with a MOV instruction. This
  // prevents COPY->MOV conversion later in the pipeline from creating
  // sequential instructions.
  bool Changed = false;
  if (MI->getOpcode() == CSA::COPY) {
    MI->setDesc(TII->get(TII->makeOpcode(CSA::Generic::MOV,
      MRI->getRegClass(destReg))));
    Changed = true;
  }

  // We need to consider the interaction of MOV with mcast generation. Later
  // tools will collapse MOV operations and multiple uses of a single LIC into
  // mcast operations. Sometimes, we want to apply attributes to some, but not
  // all, outputs of the mcast. Retaining a MOV to a destination with different
  // attributes will cause this functionality to occur.
  auto &DestInfo = LMFI->getLICInfo(destReg);
  if (!MRI->hasOneNonDBGUse(srcReg)) {
    if (DestInfo.licDepth)
      return Changed;

    if (DestInfo.attribs.find("csasim_backedge") != DestInfo.attribs.end())
      return Changed;
  }

  // If there is lic group information on the destination but not the source,
  // propagate the group back to the source.
  auto &SrcInfo = LMFI->getLICInfo(srcReg);
  if (DestInfo.licGroup && !SrcInfo.licGroup)
    SrcInfo.licGroup = DestInfo.licGroup;

  MRI->replaceRegWith(destReg, srcReg);
  // Clear the definition of srcReg in this operation. If srcReg is itself the
  // target of a MOV operation, this will enable the above check for
  // getUniqueVRegDef to return true as it won't consider MI.
  MI->getOperand(0).setReg(0);
  to_delete.push_back(MI);
  return true;
}
