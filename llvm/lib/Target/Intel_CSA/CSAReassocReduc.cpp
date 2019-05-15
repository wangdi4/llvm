//===- CSAReassocReduc.cpp - CSA Reassociating Reductions -------*- C++ -*-===//
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
// This file implements a small pass to expand reduction instructions into
// fully-pipelined reassociating "software" versions.
//
//===----------------------------------------------------------------------===//

#include "CSAReassocReduc.h"
#include "CSA.h"
#include "CSAInstrInfo.h"
#include "CSAMachineFunctionInfo.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOptimizationRemarkEmitter.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

#include <iterator>

using namespace llvm;

static cl::opt<bool> EnableReassocReduc{
  "csa-enable-reassoc-reduc", cl::Hidden,
  cl::desc("CSA Specific: Explicitly enables expansion of reductions into "
           "fully pipelined \"software\" implementations."),
  cl::init(false)};

static cl::opt<bool> DisableReassocReduc{
  "csa-disable-reassoc-reduc", cl::Hidden,
  cl::desc("CSA Specific: Explicitly disables expansion of reductions into "
           "fully pipelined \"software\" implementations."),
  cl::init(false)};

static cl::opt<int> PartRedCount{
  "csa-reassoc-reduc-partred-count", cl::Hidden, cl::ZeroOrMore,
  cl::desc("CSA Specific: The number of partial reductions to use for "
           "-csa-reassoc-reduc. 0 (the default) requests a number of partial "
           "reductions corresponding to known loop latencies; setting this to "
           "higher numbers might be useful for runs without fusion enabled."),
  cl::init(0)};

enum MultiplexType { none, deterministic, nondeterministic };

static cl::opt<MultiplexType> Multiplex{
  "csa-reassoc-reduc-multiplex", cl::Hidden,
  cl::desc("CSA Specific: Reassociating reduction multiplexing strategy"),
  cl::values(
    clEnumVal(none, "no multiplexing: use two FMA units"),
    clEnumVal(deterministic, "multiplex one FMA unit deterministically"),
    clEnumVal(nondeterministic, "multiplex one FMA unit nondeterministically")),
  cl::init(deterministic)};

static cl::opt<bool> EmulateFountains{
    "csa-no-fountains", cl::Hidden,
    cl::desc("CSA Specific: Emulate fountain ops as a temporary workaround "
             "until real scratchpad support is restored."),
    cl::init(true)};

#define DEBUG_TYPE "csa-reassoc-reduc"
#define PASS_NAME "CSA: Pipelined Reassociating Reduction Expansion"

STATISTIC(ReducsExpanded, "Number of reduction instructions pipelined");

namespace {

class CSAReassocReduc : public MachineFunctionPass {
public:
  static char ID;
  CSAReassocReduc() : MachineFunctionPass(ID) {}
  StringRef getPassName() const override {
    return "CSA Pipelined Reassociating Reduction Expansion";
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineOptimizationRemarkEmitterPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  const CSAInstrInfo *TII;
  CSAMachineFunctionInfo *LMFI;
  MachineRegisterInfo *MRI;
  MachineConstantPool *MCP;
  MachineOptimizationRemarkEmitter *ORE;

  // Determines whether a MachineInstr is eligible for conversion. This is the
  // case if it is a floating-point reduction (integer reductions are already
  // fully pipelined) and its sequence reduction output is %ign (as it isn't
  // possible to produce those values when the reduction is reassociated)
  bool isEligibleFloatingReduction(const MachineInstr &) const;

  // Expands an eligible reduction into its pipelined "software" form. The
  // iterator to the instruction after the expanded reduction is returned.
  MachineBasicBlock::iterator expandReduction(MachineInstr &);

  // Reports known latencies around reductions loops by original reduction
  // opcode. Hardcoded here for now.
  int knownRedloopLatency(CSA::Generic) const;

  // Constant-propagates a floating point operand if it happens to be a constant
  // supplied through repeats/filters/movs. This is dangerous in general because
  // it might result in the operation running constantly if that value was the
  // last thing stopping it from executing immediately on the graph load, but
  // it's safe for reductions because that won't be the case.
  void constantPropagateFPOperand(MachineOperand &opnd);

  // Determines the use operand of a repeat/filter/mov. If the instruction is
  // not a repeat/filter/mov, returns nullptr.
  const MachineOperand *getPropagatableUse(const MachineInstr *) const;

  // Determines the opcode for the collapser op. This can also be used for the
  // one in the reduction loop unless the reduction is an FMA or SUB reduction.
  unsigned getCollapserOpcode(CSA::Generic, unsigned lic_size) const;
};

} // namespace

bool llvm::willRunCSAReassocReduc(const MachineFunction &MF) {

  // If either option was passed, honor that.
  if (DisableReassocReduc)
    return false;
  if (EnableReassocReduc)
    return true;

  // Otherwise, decide based on the unsafe-fp-math flag.
  return MF.getFunction().getFnAttribute("unsafe-fp-math").getValueAsString() ==
         "true";
}

char CSAReassocReduc::ID = 0;

INITIALIZE_PASS_BEGIN(CSAReassocReduc, DEBUG_TYPE, PASS_NAME, false, false)
INITIALIZE_PASS_DEPENDENCY(MachineOptimizationRemarkEmitterPass)
INITIALIZE_PASS_END(CSAReassocReduc, DEBUG_TYPE, PASS_NAME, false, false)

MachineFunctionPass *llvm::createCSAReassocReducPass() {
  return new CSAReassocReduc{};
}

bool CSAReassocReduc::runOnMachineFunction(MachineFunction &MF) {
  if (!shouldRunDataflowPass(MF))
    return false;

  TII  = static_cast<const CSAInstrInfo *>(MF.getSubtarget().getInstrInfo());
  LMFI = MF.getInfo<CSAMachineFunctionInfo>();
  MRI  = &MF.getRegInfo();
  MCP  = MF.getConstantPool();
  ORE  = &getAnalysis<MachineOptimizationRemarkEmitterPass>().getORE();

  // Skip if willRunCSAReassocReduc determines that the pass shouldn't be run.
  if (not willRunCSAReassocReduc(MF))
    return false;

  // Otherwise, expand any reductions that should be expanded.
  bool expanded_reduc = false;
  for (MachineBasicBlock &MBB : MF) {
    for (auto MI_it = std::begin(MBB); MI_it != std::end(MBB);) {
      if (isEligibleFloatingReduction(*MI_it)) {
        MI_it          = expandReduction(*MI_it);
        expanded_reduc = true;
      } else
        ++MI_it;
    }
  }

  return expanded_reduc;
}

bool CSAReassocReduc::isEligibleFloatingReduction(
  const MachineInstr &MI) const {
  return TII->isReduction(&MI) and
         TII->getOpcodeClass(MI.getOpcode()) ==
           CSA::OpcodeClass::VARIANT_FLOAT and
         MI.getOperand(1).isReg() and MI.getOperand(1).getReg() == CSA::IGN;
}

// Creates a value containing n ones in its lowest n bits.
static uint64_t n_ones(int n) { return (1ull << n) - 1; }

// Determines the number of intermediate sums/products in the collapser.
static int collapser_internal_count(int n) {

  // Each op application decreases the number of partial reductions by one. This
  // means that it takes n - 1 op applications to collapse the reduction, but
  // one of those is switched out so the number that need to be switched back is
  // n - 2.
  return n - 2;
}

MachineBasicBlock::iterator CSAReassocReduc::expandReduction(MachineInstr &MI) {

  LLVM_DEBUG(dbgs() << "Expanding reduction: " << MI);

  // NOTE: The following implements a generic version of this assembly
  // expansion for redaddf32 result, init, val, pred:
  /*
  # The parts lic holds the pipelined intermediate sums. These are initialized
  to # five zeros and the initial value for each reduction, and partial sums
  will be # sent through here when the reduction is running. .lic@6 .f32 parts
  .value 0; .avail 0; .value 0; .avail 0
  .value 0; .avail 0; .value 0; .avail 0
  .value 0; .avail 0

  # The reinitializer for parts; alternates between initial values and five
  zeros. .attrib csasim_ignore_on_exit .lic .i1  parts_init_pred .lic .f32
  parts_init .section .csa.sp.1 one0five1: .byte 0b111110 .text fountain1
  parts_init_pred, one0five1, 6 pick32 parts_init, parts_init_pred, init, 0

  # Control values going into and out of parts using pred. For each 1, send a
  # value from parts through the add and then back into parts. For each 0, dump
  # all six values from parts into the collapser and replace them with the
  values # coming out of the reinitializer. .lic .i1  parts_pred_ctl .lic .i1
  parts_pred_picker; .value 0; .avail 0 .lic .f32 parts_to_op .lic .f32
  parts_to_clpsr .lic .f32 op_to_parts replicate1 parts_pred_ctl, pred, 0, 6, 0
  switch32 parts_to_clpsr, parts_to_op, parts_pred_ctl, parts
  mov1 parts_pred_picker, parts_pred_ctl
  pick32 parts, parts_pred_picker, parts_init, op_to_parts

  # The adder used to add values to partial sums while the reduction is running.
  addf32 op_to_parts, val, parts_to_op

  # The logic for collapsing the partial sums when the reduction is done.
  .attrib csasim_ignore_on_exit
  .lic .i1 clpsr_alternate
  .attrib csasim_ignore_on_exit
  .lic .i1 clpsr_picker
  .attrib csasim_ignore_on_exit
  .lic .i1 clpsr_switcher
  .section .csa.sp.2
  alternate: .byte          0b10
  .section .csa.sp.3
  six0four1: .short 0b1111000000
  .section .csa.sp.4
  fourseq:   .byte       0b01111
  .text
  .lic .f32 clpsr_in
  .lic .f32 clpsr_left
  .lic .f32 clpsr_right
  .lic .f32 clpsr_out
  .lic .f32 clpsr_back
  fountain1 clpsr_picker, six0four1, 10
  pick32 clpsr_in, clpsr_picker, parts_to_clpsr, clpsr_back
  fountain1 clpsr_alternate, alternate, 2
  switch32 clpsr_left, clpsr_right, clpsr_alternate, clpsr_in
  addf32 clpsr_out, clpsr_left, clpsr_right
  fountain1 clpsr_switcher, fourseq, 5
  switch32 result, clpsr_back, clpsr_switcher, clpsr_out
  */

  // Extract information about the reduction. The two forms are:
  //  sredXfN redout, %ign, init, val, pred, rm
  //  fmsredafN redout, %ign, init, val1, val2, pred, rm
  const unsigned lic_size = TII->getLicSize(MI.getOpcode());
  const TargetRegisterClass *const lic_class =
    TII->getLicClassForSize(lic_size);
  const TargetRegisterClass *const i1_class = TII->getLicClassForSize(1);
  const CSA::Generic gen_opcode = TII->getGenericOpcode(MI.getOpcode());
  const bool is_fma             = gen_opcode == CSA::Generic::FMSREDA;
  MachineOperand &result        = MI.getOperand(0);
  MachineOperand &init          = MI.getOperand(2);
  MachineOperand &pred          = MI.getOperand(is_fma ? 5 : 4);

  // Determine the number of partial reductions needed. If the flag is set, use
  // that value. Otherwise, grab the value from the known ones.
  const int partred_count =
    PartRedCount ? PartRedCount : knownRedloopLatency(gen_opcode);

  // Try constant-propagating init. If there is an immediate for it, that can
  // be put in parts directly.
  constantPropagateFPOperand(init);
  const bool init_is_imm = init.isFPImm();

  // A convenient lambda for making new lics. This is mostly just a wrapper for
  // setting the name: we used to just precompute the prefix but it's difficult
  // to use Twines like that safely.
  assert(result.isReg());
  const StringRef base_name = LMFI->getLICName(result.getReg());
  const auto add_lic = [&](
    const TargetRegisterClass *RC, const Twine &name,
    bool ignore_on_exit
  ) {
    const unsigned lic = LMFI->allocateLIC(
      RC, base_name.empty() ? name : base_name + "." + name
    );
    if (ignore_on_exit) LMFI->addLICAttribute(lic, "csasim_ignore_on_exit");
    return lic;
  };

  // Another for laying down new instructions.
  const auto ins_pos   = std::next(MachineBasicBlock::iterator{MI});
  const auto add_instr = [&](unsigned opcode) {
    MachineInstrBuilder builder =
      BuildMI(*MI.getParent(), ins_pos, MI.getDebugLoc(), TII->get(opcode));
    builder->setFlag(MachineInstr::NonSequential);
    return builder;
  };

  // Another for propagating the rounding mode if present.
  const unsigned rm_opnd = is_fma ? 6 : 5;
  const auto add_rm      = [&](const MachineInstrBuilder &builder) {
    if (MI.getNumOperands() > rm_opnd)
      builder.add(MI.getOperand(rm_opnd));
  };

  // Another for creating small fountain1s producing <64-bit sequences.
  LLVMContext &ctx = MI.getParent()->getParent()->getFunction().getContext();
  const auto smallfountain = [&](uint64_t bits, uint64_t len,
                                 const Twine &name) {
    if (EmulateFountains) {
      // To keep the fountain-replacement loops saturated and avoid late tools
      // problems, repeat small sequences ceil(9/len) times so that there are at
      // least 9 values circulating around the loop.
      const unsigned reps = 8 / len + 1;
      const unsigned res = add_lic(i1_class, name, true);
      const unsigned lthack = add_lic(i1_class, name + ".lthack", true);
      LMFI->setLICDepth(lthack, reps * len);
      add_instr(CSA::MOV1).addDef(lthack).addUse(res);
      for (unsigned r = 0; r < reps; ++r) {
        for (uint64_t i = 0; i < len; ++i) {
          const uint64_t bit = bits >> i & 1;
          add_instr(CSA::INIT1).addDef(lthack).addImm(bit);
        }
      }
      add_instr(CSA::MOV1).addDef(res).addUse(lthack);
      return res;
    } else {
      const unsigned res = add_lic(i1_class, name, true);
      add_instr(CSA::FOUNTAIN1)
          .addDef(res)
          .addImm(bits)
          .addImm(len)
          .addImm(1);
      return res;
    }
  };

  // Look up common opcodes ahead of time.
  const unsigned opcode_pick = TII->makeOpcode(CSA::Generic::PICK, lic_size);
  const unsigned opcode_switch =
    TII->makeOpcode(CSA::Generic::SWITCH, lic_size);
  const unsigned opcode_clpsr = getCollapserOpcode(gen_opcode, lic_size);

  // Determine the identity element to use for this reduction.
  assert(lic_size == 32 or lic_size == 64);
  Type *const fp_type =
    (lic_size == 32) ? Type::getFloatTy(ctx) : Type::getDoubleTy(ctx);
  ConstantFP *const identity = static_cast<ConstantFP *>(ConstantFP::get(
    fp_type, (gen_opcode == CSA::Generic::SREDMUL) ? 1.0 : 0.0));
  const bool init_is_identity =
    init_is_imm and init.getFPImm()->isExactlyValue(identity->getValueAPF());

  // The partial reduction lic, with initial values.
  const unsigned parts = add_lic(lic_class, "parts", false);
  LMFI->setLICDepth(parts, partred_count);
  for (int i = 0; i < partred_count - 1; ++i) {
    add_instr(TII->getInitOpcode(lic_class)).addDef(parts).addFPImm(identity);
  }

  // If init is an immediate, add it here.
  if (init_is_imm) {
    add_instr(TII->getInitOpcode(lic_class)).addDef(parts).add(init);
  }

  // Add logic for moving values to and from parts.
  const unsigned parts_pred_ctl = add_lic(i1_class,  "parts_pred_ctl", false);
  const unsigned parts_to_op    = add_lic(lic_class, "parts_to_op", false);
  const unsigned parts_to_clpsr = add_lic(lic_class, "parts_to_clpsr", false);
  const unsigned op_to_parts    = add_lic(lic_class, "op_to_parts", false);
  add_instr(CSA::REPLICATE1)
    .addDef(parts_pred_ctl)
    .add(pred)
    .addImm(0)
    .addImm(partred_count)
    .addImm(0);
  add_instr(opcode_switch)
    .addDef(parts_to_clpsr)
    .addDef(parts_to_op)
    .addUse(parts_pred_ctl)
    .addUse(parts);

  // The backedge of the reducer loop should be parts because otherwise
  // auto-buffering fails to add necessary buffer on one leg of parts_pred_ctl.
  LMFI->addLICAttribute(parts, "csasim_backedge");

  // If init happens to be the identity element, parts can just be reinitialized
  // with identity elements.
  if (init_is_identity) {
    add_instr(opcode_pick)
      .addDef(parts)
      .addUse(parts_pred_ctl)
      .addFPImm(identity)
      .addUse(op_to_parts);
  }

  // If it's some other immediate, use a fountain to reinitialize parts.
  else if (init_is_imm) {
    if (EmulateFountains) {
      const unsigned parts_init = add_lic(lic_class, "parts_init", true);
      const unsigned lthack = add_lic(lic_class, "parts_init.lthack", true);
      LMFI->setLICDepth(lthack, partred_count);
      add_instr(TII->makeOpcode(CSA::Generic::MOV, lic_size))
          .addDef(lthack)
          .addUse(parts_init);
      const unsigned init_opcode =
          TII->makeOpcode(CSA::Generic::INIT, lic_size);
      for (int i = 0; i < partred_count - 1; ++i) {
        add_instr(init_opcode).addDef(lthack).addFPImm(identity);
      }
      add_instr(init_opcode).addDef(lthack).add(init);
      add_instr(TII->makeOpcode(CSA::Generic::MOV, lic_size))
          .addDef(parts_init)
          .addUse(lthack);
      add_instr(opcode_pick)
          .addDef(parts)
          .addUse(parts_pred_ctl)
          .addUse(parts_init)
          .addUse(op_to_parts);
    } else {
      SmallVector<Constant *, 10> consts(partred_count, identity);
      consts.back() = const_cast<ConstantFP *>(init.getFPImm());
      const unsigned scratch = MCP->getConstantPoolIndex(
          ConstantArray::get(ArrayType::get(fp_type, partred_count), consts),
          lic_size / 8);
      const unsigned parts_init = add_lic(lic_class, "parts_init", true);
      add_instr(TII->makeOpcode(CSA::Generic::FOUNTAIN, lic_size))
          .addDef(parts_init)
          .addConstantPoolIndex(scratch)
          .addImm(partred_count)
          .addImm(1);
      add_instr(opcode_pick)
          .addDef(parts)
          .addUse(parts_pred_ctl)
          .addUse(parts_init)
          .addUse(op_to_parts);
    }
  }

  // Otherwise, use a pick to pull in the value from init and inject a 0 to make
  // sure that it pulls in the init value for the first reduction.
  else {
    const unsigned parts_init_pred = smallfountain(
      n_ones(partred_count - 1) << 1, partred_count, "parts_init_pred");
    const unsigned parts_init        = add_lic(lic_class, "parts_init", false);
    const unsigned parts_pred_picker = add_lic(i1_class,  "parts_pred_picker", false);
    add_instr(opcode_pick)
      .addDef(parts_init)
      .addUse(parts_init_pred)
      .add(init)
      .addFPImm(identity);
    add_instr(TII->getInitOpcode(i1_class)).addDef(parts_pred_picker).addImm(0);
    add_instr(TII->getMoveOpcode(i1_class))
      .addDef(parts_pred_picker)
      .addUse(parts_pred_ctl);
    add_instr(opcode_pick)
      .addDef(parts)
      .addUse(parts_pred_picker)
      .addUse(parts_init)
      .addUse(op_to_parts);
  }

  // Put in the op for the reduction loop. This is the same as the collapser op
  // except in the case of FMREDAs where it is an FMA and REDSUBs where it is a
  // SUB. If multiplexing is enabled, this will be handled later.
  if (Multiplex == none) {
    if (is_fma) {
      add_rm(add_instr(
               TII->makeOpcode(CSA::Generic::FMA, lic_size, CSA::VARIANT_FLOAT))
               .addDef(op_to_parts)
               .add(MI.getOperand(3))
               .add(MI.getOperand(4))
               .addUse(parts_to_op));
    } else if (gen_opcode == CSA::Generic::SREDSUB) {
      add_rm(add_instr(
               TII->makeOpcode(CSA::Generic::SUB, lic_size, CSA::VARIANT_FLOAT))
               .addDef(op_to_parts)
               .addUse(parts_to_op)
               .add(MI.getOperand(3)));
    } else {

      // parts_to_op is the first operand here in order to make it more likely
      // to get fused.
      add_rm(add_instr(opcode_clpsr)
               .addDef(op_to_parts)
               .addUse(parts_to_op)
               .add(MI.getOperand(3)));
    }
  }

  // Put in the collapser.
  const int internal_count       = collapser_internal_count(partred_count);
  const unsigned clpsr_alternate = smallfountain(0x2, 2, "clpsr_alternate");
  const unsigned clpsr_picker =
    smallfountain(n_ones(internal_count) << partred_count,
                  internal_count + partred_count, "clpsr_picker");
  const unsigned clpsr_switcher =
    smallfountain(n_ones(internal_count), internal_count + 1, "clpsr_switcher");
  const unsigned clpsr_in    = add_lic(lic_class, "clpsr_in", false);
  const unsigned clpsr_left  = add_lic(lic_class, "clpsr_left", false);
  const unsigned clpsr_right = add_lic(lic_class, "clpsr_right", false);
  const unsigned clpsr_out   = add_lic(lic_class, "clpsr_out", false);
  const unsigned clpsr_back  = add_lic(lic_class, "clpsr_back", false);
  LMFI->addLICAttribute(clpsr_back, "csasim_backedge");
  add_instr(opcode_pick)
    .addDef(clpsr_in)
    .addUse(clpsr_picker)
    .addUse(parts_to_clpsr)
    .addUse(clpsr_back);
  add_instr(opcode_switch)
    .addDef(clpsr_left)
    .addDef(clpsr_right)
    .addUse(clpsr_alternate)
    .addUse(clpsr_in);
  if (Multiplex == none) {
    add_rm(add_instr(opcode_clpsr)
             .addDef(clpsr_out)
             .addUse(clpsr_left)
             .addUse(clpsr_right));
  }
  add_instr(opcode_switch)
    .add(result)
    .addDef(clpsr_back)
    .addUse(clpsr_switcher)
    .addUse(clpsr_out);

  // If multiplexing is enabled, insert the multiplexing code here.
  if (Multiplex != none) {
    const unsigned op_pred_ctl = add_lic(i1_class,  "op_pred_ctl", false);
    const unsigned op_left     = add_lic(lic_class, "op_left", false);
    const unsigned op_right    = add_lic(lic_class, "op_right", false);
    const unsigned op_out      = add_lic(lic_class, "op_out", false);

    // For deterministic multiplexing, drive both inputs off of a replicate1.
    // For nondeterministic multiplexing, drive both off of a pickany on the
    // _right_ input (the last to appear in the collapser and the one with the
    // partial sum in the reducer) to avoid deadlocks. The collapser has
    // precedence over the reducer to avoid starvation.
    if (Multiplex == deterministic) {
      add_instr(CSA::REPLICATE1)
        .addDef(op_pred_ctl)
        .add(pred)
        .addImm(0)
        .addImm(partred_count - 1)
        .addImm(0);
      add_instr(opcode_pick)
        .addDef(op_right)
        .addUse(op_pred_ctl)
        .addUse(clpsr_right)
        .addUse(parts_to_op);
    } else {
      add_instr(TII->makeOpcode(CSA::Generic::PICKANY, lic_size))
        .addDef(op_right)
        .addDef(op_pred_ctl)
        .addUse(clpsr_right)
        .addUse(parts_to_op);
    }

    // FMAs have a third argument (op_not_as_left) to operate on. In the
    // collapser the FMA needs to behave like an addf, so this is set to
    // 1.0 there.
    if (is_fma) {
      add_instr(opcode_pick)
        .addDef(op_left)
        .addUse(op_pred_ctl)
        .addUse(clpsr_left)
        .add(MI.getOperand(3));
      const unsigned op_not_as_left = add_lic(lic_class, "op_not_as_left", false);
      add_instr(opcode_pick)
        .addDef(op_not_as_left)
        .addUse(op_pred_ctl)
        .addFPImm(static_cast<ConstantFP *>(ConstantFP::get(fp_type, 1.0)))
        .add(MI.getOperand(4));

      // For the lowest loop latency, we need the last input to be fused here
      // rather than either of the first two. This disables fusion on the first
      // two inputs so that the last one can be fused instead. See LT2-92 for
      // more information.
      // However, the last input can't be fused anyway for the nondeterministic
      // case, so enable fusion on the other lics there to save space.
      if (Multiplex != nondeterministic) {
        LMFI->addLICAttribute(op_left, "csa_fuse_disable");
        LMFI->addLICAttribute(op_not_as_left, "csa_fuse_disable");
      }

      add_rm(add_instr(
               TII->makeOpcode(CSA::Generic::FMA, lic_size, CSA::VARIANT_FLOAT))
               .addDef(op_out)
               .addUse(op_left)
               .addUse(op_not_as_left)
               .addUse(op_right));
    }

    // For SUBs, the collapser needs to be equivalent to an addf so the "left"
    // input (which is really the right one for SUBs - the conventions here are
    // FMA-based) needs a negation there.
    else if (gen_opcode == CSA::Generic::SREDSUB) {
      const unsigned op_neg = add_lic(lic_class, "op_neg", false);
      add_instr(
        TII->makeOpcode(CSA::Generic::NEG, lic_size, CSA::VARIANT_FLOAT))
        .addDef(op_neg)
        .addUse(clpsr_left);
      add_instr(opcode_pick)
        .addDef(op_left)
        .addUse(op_pred_ctl)
        .addUse(op_neg)
        .add(MI.getOperand(3));
      add_rm(add_instr(
               TII->makeOpcode(CSA::Generic::SUB, lic_size, CSA::VARIANT_FLOAT))
               .addDef(op_out)
               .addUse(op_right)
               .addUse(op_left));
    }

    else {
      add_instr(opcode_pick)
        .addDef(op_left)
        .addUse(op_pred_ctl)
        .addUse(clpsr_left)
        .add(MI.getOperand(3));

      // Mapping tends to fuse the first input to binary ops, but we want the
      // "right" one fused because that is on the critical path. Therefore, the
      // operands are emitted in reverse order here.
      add_rm(add_instr(opcode_clpsr)
               .addDef(op_out)
               .addUse(op_right)
               .addUse(op_left));
    }
    add_instr(opcode_switch)
      .addDef(clpsr_out)
      .addDef(op_to_parts)
      .addUse(op_pred_ctl)
      .addUse(op_out);
  }

  // The expanded reduction is now complete. Delete the original reduction
  // instruction.
  MachineOptimizationRemark Remark{
    DEBUG_TYPE, "CSAReassocReduc: ", MI.getDebugLoc(), MI.getParent()};
  ORE->emit(Remark << " reduction optimized using pipelined expansion");
  MI.eraseFromParent();
  ++ReducsExpanded;
  return ins_pos;
}

int CSAReassocReduc::knownRedloopLatency(CSA::Generic generic) const {
  switch (Multiplex) {
  case none:
    return 9;
  case deterministic:
    return 9; // Multiplexing is "free" here because it is more fusion-friendly.
  case nondeterministic:
    return generic == CSA::Generic::FMSREDA
             ? 15  // Unfortunately fusion is not so helpful here, especially
             : 13; // for FMAs.
  default:
    llvm_unreachable("bad multiplexing mode");
  }
}

void CSAReassocReduc::constantPropagateFPOperand(MachineOperand &opnd) {

  // If the operand already is a floating point immediate, there isn't anything
  // to be done.
  if (opnd.isFPImm())
    return;

  // Walk backwards through repeats/filters/movs to see if there is a
  // constant to be propagated.
  const MachineOperand *cur_opnd = &opnd;
  while (not cur_opnd->isFPImm()) {
    if (not cur_opnd->isReg())
      return;
    const MachineInstr *const def = MRI->getUniqueVRegDef(cur_opnd->getReg());
    if (not def)
      return;
    cur_opnd = getPropagatableUse(def);
    if (not cur_opnd)
      return;
  }

  // If there is, propagate it and DCE anything along the path that doesn't have
  // other users.
  assert(opnd.isReg());
  MachineInstr *cur_def = MRI->getUniqueVRegDef(opnd.getReg());
  assert(cur_def);
  opnd.ChangeToFPImmediate(cur_opnd->getFPImm());
  while (cur_def and MRI->use_nodbg_empty(cur_def->getOperand(0).getReg())) {
    const MachineOperand *const prop_use = getPropagatableUse(cur_def);
    assert(prop_use);
    MachineInstr *next_def = nullptr;
    if (prop_use->isReg())
      next_def = MRI->getUniqueVRegDef(prop_use->getReg());

    cur_def->eraseFromParentAndMarkDBGValuesForRemoval();
    cur_def = next_def;
  }
}

const MachineOperand *
CSAReassocReduc::getPropagatableUse(const MachineInstr *MI) const {
  switch (TII->getGenericOpcode(MI->getOpcode())) {
  case CSA::Generic::REPEAT:
  case CSA::Generic::REPEATO:
  case CSA::Generic::FILTER:
    return &MI->getOperand(2);
  case CSA::Generic::MOV:
    return &MI->getOperand(1);
  default:
    return nullptr;
  }
}

unsigned CSAReassocReduc::getCollapserOpcode(CSA::Generic generic,
                                             unsigned lic_size) const {
  switch (generic) {
  case CSA::Generic::SREDADD:
  case CSA::Generic::FMSREDA:
  case CSA::Generic::SREDSUB:
    return TII->makeOpcode(CSA::Generic::ADD, lic_size, CSA::VARIANT_FLOAT);
  case CSA::Generic::SREDMUL:
    return TII->makeOpcode(CSA::Generic::MUL, lic_size, CSA::VARIANT_FLOAT);
  default:
    llvm_unreachable("unexpected generic reducer type");
  }
}
