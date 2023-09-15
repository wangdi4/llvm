//===-- CSASeqotToSeqOptimization.cpp - Optimize SEQOT to SEQ ---*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// Optimization that transforms SEQOT to SEQ.
///
//===----------------------------------------------------------------------===//

#include "CSAMatcher.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define DEBUG_TYPE "csa-opt-seqot-to-seq"
#define PASS_NAME "CSA: SEQOT->SEQ optimization pass."

static cl::opt<bool> DisableSeqotToSeqOpt(
                         "csa-disable-opt-seqot-to-seq",
                         cl::Hidden,
                         cl::desc("CSA Specific: Disable optimization "
                                  "that transforms SEQOT to SEQ."));

namespace llvm {
class CSASeqotToSeqOptimizationImpl {
public:
  CSASeqotToSeqOptimizationImpl(MachineFunction &MF,
                                const CSAInstrInfo &TII,
                                const MachineRegisterInfo &MRI,
                                CSAMachineFunctionInfo &MFI)
    : MF(MF), TII(TII), MRI(MRI), MFI(MFI) {}

  /// \brief Look for specific SEQOT patterns and transform them
  /// into SEQ.
  ///
  /// The pass looks for the following patterns:
  ///
  /// Pattern #1: may be generated for loops with unknown tripcount.
  ///     %cmp:ci1 = CMPLTS<NT> %N:ci<WT>, C
  ///     %not:ci1 = NOT1 %cmp:ci1
  ///     %filter:ci<WT> = FILTER<WT> %not:ci1, %N:ci<WT>
  ///     %sext:ci<WT> = SEXT<WT> %filter:ci<WT>, sizeof(<NT>)
  ///     %add:ci<WT> = ADD<WT> %sext:ci<WT>, -C
  ///     (-, %pred:ci1, -, %last:ci1) =
  ///         SEQOTLTS<WT> -1, %add:ci<WT>, S
  ///     optional-1: %lnot:ci1 = NOT1 %last:ci1
  ///     optional-1: ... = REPEATO<AT1> %lnot:ci1, %add:ci<WT>
  ///     optional-2: %anyfilter1:ci<AT2> = FILTER<AT2> %not:ci1, %fval1:ci<AT2>
  ///     optional-2: ... = STRIDE<AT2> %pred:ci1,
  ///                                   %anyfilter1:ci<AT2>, <any constant>
  ////    optional-3: %anyfilter2:ci<AT3> = FILTER<AT3> %not:ci1, %fval2:ci<AT3>
  ///     optional-3: ... = REPEATO<AT3> %lnot:ci1, %anyfilter2:ci<AT3>
  ///
  /// Where:
  ///     * AT1, AT2, AT3 - any types
  ///     * WT - wide type
  ///     * NT - narrow type, such that sizeof(<NT>) < sizeof(<WT>)
  ///     * C is a constant greater than 0
  ///     * S is a constant greater than 0
  ///     * %filter:ci<WT> and %sext:ci<WT> have only one use each.
  ///     * add:ci<WT> may have multiple uses, but they all
  ///       have to be either SEQOTLTS instructions with operands
  ///       matching the operands of the pattern's SEQOTLTS
  ///       OR
  ///       REPEATO instructions that use the inverted (NOT1)
  ///       'last' signal from the pattern's SEQOTLTS.
  ///     * %pred:ci1 may have uses in the graph, but thet all
  ///       have to follow the optional-2 FILTER/STRIDE pattern.
  ///
  /// NOTE: the CMPLTS<NT> checks (N < C);  if it is true, then
  ///       the SEQOTLTS will not start and all the incoming values will
  ///       be filtered away;  if it is false, then (sext(N) - C)
  ///       is guaranteed to be not less than zero, so both SEQOTLTS
  ///       and SEQLTS with the same operands will run at least once.
  ///
  /// Optimized sequence:
  ///     %cmp:ci1 = CMPLTS<NT> %N:ci<WT>, C
  ///     %not:ci1 = NOT1 %cmp:ci1
  ///     %sext:ci<WT> = SEXT<WT> %N:ci<WT>, sizeof(<NT>)
  ///     %add:ci<WT> = ADD<WT> %sext:ci<WT>, -C
  ///     (-, %pred:ci1, -, %last:ci1) =
  ///         SEQLTS<WT> -1, %add:ci<WT>, S
  ///     optional-1: ... = REPEAT<AT1> %pred:ci1, %add:ci<WT>
  ///     optional-2: ... = STRIDE<AT2> %pred:ci1, %fval:ci<AT2>, <any constant>
  ///     optional-3: ... = REPEAT<AT3> %pred:ci1, %fval:ci<AT3>
  ///
  /// In some cases, the following sequence:
  ///     %cmp:ci1 = CMPLTS<NT> %N:ci<WT>, C
  ///     %not:ci1 = NOT1 %cmp:ci1
  /// may be represented as:
  ///     %not:ci1 = CMPGTS<NT> %N:ci<WT>, (C - 1)
  ///
  /// NOTE: all SEQOTLTS and REPEATO (optional-1) uses of %add:ci<WT>,
  ///       and all STRIDE (optional-2) uses of the %pred:ci1 must be
  ///       transformed at the same time, otherwise, the transformation
  ///       will not be legal.  At the same time, optional-3
  ///       instructions may be left untouched - this will not affect
  ///       correctness.
  ///
  /// The matching is done with the following steps:
  ///     1. Find the starting SEQOT instruction with constant 'base'
  ///        and 'stride' operands.
  ///     2. Pick the SEQOT's 'bound' operand and match the FILTER/SEXT/ADD
  ///        pattern.  The pattern matching guarantees that FILTER has only one
  ///        use, so that the transformation will make it dead.  The pattern
  ///        matching guarantees that SEXT has only one use, so that we know
  ///        the effect of making its operand unfiltered.
  ///     3. Pick the ADD from step #2 and collect all its uses.
  ///        Verify that the collected uses fit the pattern:
  ///            a. Each use is either SEQOT or REPEATO.
  ///            b. All SEQOTs must have exactly the same input operands.
  ///            c. All REPEATOs (if any) must use the NOT of one of the SEQOTs
  ///               'last' operand for the control operand (optional-1).
  ///        Go to step #1, if any of the uses does not fit the pattern.
  ///     4. Pick the FILTER from step #2 and check that the compare predicate
  ///        used as the FILTER's control operand matches the compare pattern:
  ///            a. It is either CMP/NOT or CMP as described in the pattern.
  ///            b. For all SEQOTs collected on step #3, the constants
  ///               used by SEQOTs and the compare instruction(s) match
  ///               the pattern description.  As long as all SEQOTs match,
  ///               we can use starting SEQOT for reference.
  ///        Go to step #1, if there is no match.
  ///     5. For each SEQOT collected on step #3, pick the 'pred' operand
  ///        and collect its uses.  If no uses, go to next step.
  ///        Verify that the collected uses fit the pattern (optional-2):
  ///            a. Each use is STRIDE with constant 'stride' operand.
  ///            b. The STRIDE control operand is 'pred'.
  ///            b. The base operand of the STRIDE is a FILTER controlled
  ///               by the compare predicate identified on step #4.
  ///        Go to step #1, if match failed.
  ///     6. (optional-3) For each SEQOT collected on step #3, find the NOT
  ///        of its 'last' operand.  For each found NOT collect its REPEATO
  ///        uses.  If the REPEATO's value operand is a FILTER with the control
  ///        operand equal to the compare predicate identified on step #4,
  ///        and the FILTER has only one use, then add this REPEATO
  ///        to optimization set.
  ///     7. Modify all collected SEQOT, STRIDE and REPEATO instructions.
  ///        Go to step #1.
  //
  // TODO (vzakhari 10/30/2018): at least for optional-2 and optional-3
  //       the pattern recognition has to be reworked.  We have to use
  //       the idea that any DF tree that starts with FILTERs controlled
  //       by %not:ci1 (these FILTERs are root nodes of the tree) and ends
  //       with an operation that consumes its operands, given '0' control
  //       (e.g. REPEATO/REPEAT, STRIDE, etc.), may be transformed
  //       by eliminating the FILTERs, using %pred:ci1 to control
  //       the leaf operations and sinking the FILTERs' value operands
  //       to their uses inside the tree.  This will help to handle stencil
  //       cases, where there is an intervening ADD between optional-2
  //       FILTER and STRIDE operations.  The obvious drawback of such
  //       a transformation is that the intervening operations may now
  //       run unconditionally, thus taking power and having other side-effects
  //       (like FP exceptions, though, we only work with integer operations
  //       now).
  bool run();

  /// The method transforms the recognized pattern, which was proven
  /// to be optimizable.
  ///
  /// \p SeqotsAndRepeatos - a set of SEQOT and (optional-1) REPEATOs.
  /// \p Opt2Instructions - a set of (optional-2) STRIDE instructions.
  /// \p Opt3Instructions - a set of (optional-3) REPEATO instructions.
  ///
  /// The transformation is done according to the description above.
  /// The method puts instructions that need to be removed into
  /// InstrsToDelete vector.
  void doTransformation(SmallPtrSet<MachineInstr *, 10> &SeqotsAndRepeatos,
                        ArrayRef<MachineInstr *> Opt2Instructions,
                        SmallPtrSet<MachineInstr *, 20> &Opt3Instructions);
private:
  MachineFunction &MF;
  const CSAInstrInfo &TII;
  const MachineRegisterInfo &MRI;
  CSAMachineFunctionInfo &MFI;

  // TODO (vzakhari 9/26/2018): there are multiple definitions
  //       of getSingleDef(), so we probably need to move it to
  //       something like CSAMachineRegisterInfo class.
  MachineInstr *getSingleDef(unsigned Reg,
                             const MachineRegisterInfo &MRI) const;

  bool areTypesComplying(MachineInstr *I1, MachineInstr *I2) const {
    return (TII.getLicSize(I1->getOpcode()) == TII.getLicSize(I2->getOpcode()));
  }

  // An array of instructions that has to be deleted at the end
  // of the optimization, because they were replaced by new ones.
  SmallVector<MachineInstr *, 10> InstrsToDelete;
};

class CSASeqotToSeqOptimization : public MachineFunctionPass {
public:
  static char ID;

  CSASeqotToSeqOptimization()
    : MachineFunctionPass(ID) {
    initializeCSASeqotToSeqOptimizationPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return PASS_NAME;
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    const auto &TII =
        *static_cast<const CSAInstrInfo *>(MF.getSubtarget().getInstrInfo());
    const auto &MRI = MF.getRegInfo();
    auto &MFI = *MF.getInfo<CSAMachineFunctionInfo>();
    return CSASeqotToSeqOptimizationImpl(MF, TII, MRI, MFI).run();
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
};
} // namespace llvm

char CSASeqotToSeqOptimization::ID = 0;

INITIALIZE_PASS(CSASeqotToSeqOptimization, DEBUG_TYPE, PASS_NAME, true, false)

MachineFunctionPass *llvm::createCSASeqotToSeqOptimizationPass() {
  return new CSASeqotToSeqOptimization();
}

namespace {
MIRMATCHER_REGS(MBound, MAddOp, MSextOp);
using namespace CSAMatch;
using mirmatch::AnyOperand;
using mirmatch::AnyLiteral;
constexpr auto Pattern1 = mirmatch::graph(
    MBound = add_N(MAddOp, AnyOperand),
    MAddOp = sext_N(MSextOp, AnyLiteral),
    MSextOp = filter_N(AnyOperand, AnyOperand)
);
} // end anonymous namespace

// Return the MachineInstr* if it is the single def of the Reg.
// This method is a simplfication of the method implemented in
// TwoAddressInstructionPass
MachineInstr *CSASeqotToSeqOptimizationImpl::getSingleDef(
    unsigned Reg, const MachineRegisterInfo &MRI) const {
  MachineInstr *Ret = nullptr;
  for (MachineInstr &DefMI : MRI.def_instructions(Reg)) {
    if (DefMI.isDebugValue())
      continue;
    if (!Ret)
      Ret = &DefMI;
    else if (Ret != &DefMI)
      return nullptr;
  }
  return Ret;
}

bool CSASeqotToSeqOptimizationImpl::run() {
  if (DisableSeqotToSeqOpt)
    return false;

  bool Modified = false;

  // A set of SEQOT instructions that we already processed,
  // and either transformed them or identified them as
  // non-transformable.
  SmallPtrSet<MachineInstr *, 10> VisitedOTInstructions;

  LLVM_DEBUG(dbgs() << "BEGIN: SEQOT->SEQ opt.\n");

  for (auto &MBB : MF)
    for (auto &StartingSeqot : MBB) {
      // Match pattern #1.

      // Step #1:

      // Match: (-, %pred:ci1, -, %last:ci1) =
      //            SEQOTLTS<WT> -1, %add<WT>:ci<WT>, S
      if (!TII.isSeqOT(&StartingSeqot) ||
          VisitedOTInstructions.count(&StartingSeqot) != 0)
        continue;

      VisitedOTInstructions.insert(&StartingSeqot);

      if (StartingSeqot.getOpcode() < CSA::SEQOTLTS16 ||
          StartingSeqot.getOpcode() > CSA::SEQOTLTS8 ||
          !StartingSeqot.getOperand(4).isImm() ||
          !StartingSeqot.getOperand(5).isReg() ||
          !StartingSeqot.getOperand(6).isImm())
        continue;

      LLVM_DEBUG(dbgs() << "--\nInitial candidate for SEQOT->SEQ opt: " <<
                 StartingSeqot);

      if (StartingSeqot.getOperand(6).getImm() <= 0) {
        LLVM_DEBUG(dbgs() << "Problem - non-positive stride.\n");
        continue;
      }

      auto BoundReg = StartingSeqot.getOperand(5).getReg();
      if (StartingSeqot.getOperand(4).getImm() != -1) {
        LLVM_DEBUG(dbgs() << "Problem - lower bound is not -1.\n");
        continue;
      }

      // Step #2:

      // Quickly check, if the ADD matches the pattern.
      // Match: %add:ci<WT> = ADD<WT> %sext<WT>:ci<WT>, -C
      auto *AddInst = getSingleDef(BoundReg, MRI);
      if (!AddInst || !TII.isAdd(AddInst) ||
          !AddInst->getOperand(1).isReg() ||
          !AddInst->getOperand(2).isImm() ||
          !areTypesComplying(&StartingSeqot, AddInst)) {
        LLVM_DEBUG(dbgs() << "Problem - unsupported definition "
                   "of 'bound' value.\n");
        continue;
      }

      auto AddImm = AddInst->getOperand(2).getImm();
      if (AddImm >= 0) {
        LLVM_DEBUG(dbgs() << "Problem - addend is non-negative.\n");
        continue;
      }

      // Match:
      //     %filter:ci<WT> = FILTER<WT> %not:ci1, %N:ci<WT>
      //     %sext:ci<WT> = SEXT<WT> %filter:ci<WT>, sizeof(<NT>)
      //     %add:ci<WT> = ADD<WT> %sext:ci<WT>, -C
      //
      // The matcher guarantees that %filter<WT>:ci<WT> and
      // %sext<WT>:ci<WT> have only one use each.
      auto MatchResult = mirmatch::match(Pattern1, AddInst);

      if (!MatchResult) {
        LLVM_DEBUG(dbgs() << "Problem - FILTER/SEXT/ADD pattern match "
                   "failed.\n");
        continue;
      }

      assert(MatchResult.instr(MBound = add_N(MAddOp, AnyOperand)) ==
             AddInst);
      auto *SextInst =
        MatchResult.instr(MAddOp = sext_N(MSextOp, AnyLiteral));
      auto *FilterInst =
          MatchResult.instr(MSextOp = filter_N(AnyOperand, AnyOperand));
      assert(SextInst && FilterInst && "Invalid matching.");

      // Check: %sext:ci<WT> = SEXT<WT> %filter:ci<WT>, sizeof(<NT>)
      if (!areTypesComplying(&StartingSeqot, SextInst)) {
        LLVM_DEBUG(dbgs() << "Problem - non-complying types "
                   "for SEQOT and SEXT.\n");
        continue;
      }

      auto NarrowTypeSize = SextInst->getOperand(2).getImm();
      assert(NarrowTypeSize < TII.getLicSize(AddInst->getOpcode()) &&
             "Inconsistent types in SEXT->ADD chain.");

      // Check: %filter:ci<WT> = FILTER<WT> %not:ci1, %N:ci<WT>
      if (!FilterInst->getOperand(1).isReg() ||
          !FilterInst->getOperand(2).isReg() ||
          !areTypesComplying(&StartingSeqot, FilterInst)) {
        LLVM_DEBUG(dbgs() << "Problem - FILTER does not match.\n");
        continue;
      }

      auto FilterControlReg = FilterInst->getOperand(1).getReg();
      auto *FilterControlDef = getSingleDef(FilterControlReg, MRI);
      auto FilterValReg = FilterInst->getOperand(2).getReg();
      auto *FilterValDef = getSingleDef(FilterValReg, MRI);

      if (!FilterControlDef || !FilterValDef) {
        LLVM_DEBUG(dbgs() << "Problem - FILTER has unknown operands.\n");
        continue;
      }

      // Step #3:

      // Check all the uses of AddInst.  They compose our working set
      // of SEQOT and REPEATO (optional-1) instructions.
      SmallPtrSet<MachineInstr *, 10> OTUses;
      bool HasUnknownUses = false;
      for (auto &Use : MRI.use_instructions(BoundReg)) {
        if (Use.isDebugValue())
          continue;

        if (TII.isRepeatO(&Use) ||
            Use.getOpcode() == StartingSeqot.getOpcode()) {
          OTUses.insert(&Use);

          // Mark all "forward" SEQOTs as visited.
          if (&Use != &StartingSeqot) {
            if (TII.isSeqOT(&Use)) {
              LLVM_DEBUG(dbgs() << "Adding SEQOT for optimization: " << Use);
              VisitedOTInstructions.insert(&Use);
            } else
              LLVM_DEBUG(dbgs() <<
                         "Adding optional-1 REPEATO for optimization: " << Use);
          }
        } else {
          HasUnknownUses = true;
          LLVM_DEBUG(dbgs() << "Problem - unknown use of ADD: " << Use);
        }
      }

      if (HasUnknownUses)
        continue;

      // For each REPEATO (optional-1) candidate
      // For each REPEATO candidate keep its control operand pointer
      // and a pointer to 'pred' operand of the corresponding SEQOT instruction.
      using RepeatoModT = std::pair<MachineOperand &, const MachineOperand &>;
      DenseMap<MachineInstr *, RepeatoModT> RepeatoMods;
      // For STRIDE (optional-2) instruction that uses 'pred' signal we keep
      // the corresponding FILTER instruction's control and value
      // operands.
      using StrideCandidateT =
        std::pair<MachineOperand &, const MachineOperand &>;
      DenseMap<MachineInstr *, StrideCandidateT> StrideCandidates;

      // Check if all SEQOT uses have the right operands.
      auto Step3SeqotIsNotOK = [&](MachineInstr *SeqotInst) {
        if (TII.isRepeatO(SeqotInst))
          return false;

        assert(TII.isSeqOT(SeqotInst) && "Expected SEQOT.");

        // Check that this SEQOT matches exactly with
        // the one that we started with (ignoring the result
        // registers).
        MachineInstr *BoundDef = nullptr;
        if (!SeqotInst->getOperand(4).isImm() ||
            SeqotInst->getOperand(4).getImm() != -1 ||
            !SeqotInst->getOperand(5).isReg() ||
            SeqotInst->getOperand(5).getReg() != BoundReg ||
            !(BoundDef = getSingleDef(BoundReg, MRI)) ||
            BoundDef != AddInst ||
            !SeqotInst->getOperand(6).isImm() ||
            SeqotInst->getOperand(6).getImm() <= 0) {
          LLVM_DEBUG(dbgs() << "Problem - non-matching SEQOT: " << SeqotInst);
          return true;
        }

        return false;
      };

      if (std::any_of(OTUses.begin(), OTUses.end(), Step3SeqotIsNotOK))
        continue;

      // Check if all REPEATO uses have the right operands.
      auto Step3RepeatoIsNotOK = [&](MachineInstr *RepeatoInst) {
        if (TII.isSeqOT(RepeatoInst))
          return false;

        assert(TII.isRepeatO(RepeatoInst) && "Expected REPEATO.");

        // Check: ... = REPEATO<AT1> %lnot:ci1, %add:ci<WT>
        if (!RepeatoInst->getOperand(2).isReg() ||
            RepeatoInst->getOperand(2).getReg() != BoundReg ||
            !RepeatoInst->getOperand(1).isReg()) {
          LLVM_DEBUG(dbgs() << "Problem - invalid REPEATO operands: " <<
                     RepeatoInst);
          return true;
        }

        // Check the control operand of the REPEATO:
        //     %lnot:ci1 = NOT1 %last:ci1
        auto RepeatoControlReg = RepeatoInst->getOperand(1).getReg();
        auto *NotInst = getSingleDef(RepeatoControlReg, MRI);
        if (!NotInst || !NotInst->getOperand(1).isReg()) {
          LLVM_DEBUG(dbgs() << "Problem - REPEATO control is not NOT1: " <<
                     RepeatoInst);
          return true;
        }

        auto NotOpReg = NotInst->getOperand(1).getReg();
        auto *SeqOTInst = getSingleDef(NotOpReg, MRI);
        if (!SeqOTInst ||
            // This has to be one of SEQOT instructions
            // from the OTUses set.
            OTUses.count(SeqOTInst) == 0 ||
            // Check that the NOT1 instruction uses the %last register
            // of the SEQOT instruction.
            !SeqOTInst->getOperand(3).isReg() ||
            // Is it possible that %last is %ign and the NOT1's
            // operand is also %ign?
            NotOpReg == CSA::IGN ||
            NotOpReg != SeqOTInst->getOperand(3).getReg()) {
          LLVM_DEBUG(dbgs() << "Problem - invalid REPEATO control operand: " <<
                     RepeatoInst);
          return true;
        }

        // We will want to use 'pred' operand of this SEQOT later,
        // so let's check if it is a register.
        // If 'pred' is %ign, then replace it with a new register
        // operand, so that we can use it later during REPEATO->REPEAT
        // rewrite.  This is a side-effect of the optimization even
        // if we decide not to transform the MIR later.
        if (!SeqOTInst->getOperand(1).isReg()) {
          // Should we actually assert here?
          LLVM_DEBUG(dbgs() << "'pred' operand is not a register: " <<
                     SeqOTInst);
          return true;
        }

        return false;
      };

      if (std::any_of(OTUses.begin(), OTUses.end(), Step3RepeatoIsNotOK))
        continue;

      // Step #4:

      // Check the compare predicate of:
      //     %filter:ci<WT> = FILTER<WT> %not:ci1, %N:ci<WT>
      int64_t CImm;

      if (TII.isCmp(FilterControlDef)) {
        // Match: %not:ci1 = CMPGTS<NT> %N:ci<WT>, (C - 1)
        auto CmpOpcode = FilterControlDef->getOpcode();
        if (TII.getLicSize(CmpOpcode) != NarrowTypeSize ||
            CmpOpcode < CSA::CMPGTS16 || CmpOpcode > CSA::CMPGTS8 ||
            !FilterControlDef->getOperand(1).isReg() ||
            FilterControlDef->getOperand(1).getReg() != FilterValReg ||
            !FilterControlDef->getOperand(2).isImm()) {
          LLVM_DEBUG(dbgs() << "Problem - unsupported FILTER predicate: " <<
                     FilterControlDef);
          continue;
        }

        // Check if we can add 1 to the CMPGTS's immediate without
        // overflowing int32.
        CImm = int64_t(FilterControlDef->getOperand(2).getImm()) + 1;
        if ((CImm & 0xFFFFFFFF) == 0) {
          LLVM_DEBUG(dbgs() << "Problem - cannot compute C: " << CImm);
          continue;
        }
      } else if (TII.isNot(FilterControlDef)) {
        // Match:
        //     %cmp:ci1 = CMPLTS<NT> %N:ci<WT>, C
        //     %not:ci1 = NOT1 %cmp:ci1
        if (!FilterControlDef->getOperand(1).isReg()) {
          LLVM_DEBUG(dbgs() << "Problem - invalid NOT operand: " <<
                     FilterControlDef);
          continue;
        }

        auto NotOpReg = FilterControlDef->getOperand(1).getReg();
        auto *CmpInst = getSingleDef(NotOpReg, MRI);

        if (!CmpInst || !TII.isCmp(CmpInst) ||
            TII.getLicSize(CmpInst->getOpcode()) != NarrowTypeSize ||
            CmpInst->getOpcode() < CSA::CMPLTS16 ||
            CmpInst->getOpcode() > CSA::CMPLTS8 ||
            !CmpInst->getOperand(1).isReg() ||
            CmpInst->getOperand(1).getReg() != FilterValReg ||
            !CmpInst->getOperand(2).isImm()) {
          LLVM_DEBUG(dbgs() << "Problem - unsupported CMP for NOT: " <<
                     FilterControlDef);
          continue;
        }

        CImm = int64_t(CmpInst->getOperand(2).getImm());
      } else {
        LLVM_DEBUG(dbgs() << "Problem - unsupported FILTER predicate: " <<
                   FilterControlDef);
        continue;
      }

      if (AddImm != -CImm) {
        LLVM_DEBUG(dbgs() << "Problem - bounds do not match: CMP bound(" <<
                   CImm << "), ADD immediate(" << AddImm << ")\n");
        continue;
      }

      auto Step4CmpPredicate = FilterControlDef->getOperand(0).getReg();

      // Step #5:

      // Check uses of the SEQOTs' 'pred' operands, and collect
      // STRIDE instructions following optional-2 pattern.
      SmallVector<MachineInstr *, 20> Opt2Instructions;
      auto PredUseIsNotOk = [&](MachineInstr *SeqotInst) {
        if (TII.isRepeatO(SeqotInst))
          return false;

        assert(TII.isSeqOT(SeqotInst) && "Expected SEQOT.");

        if (!SeqotInst->getOperand(1).isReg()) {
          // Should we actually assert here?
          LLVM_DEBUG(dbgs() << "Problem - SEQOT's 'pred' operand is not "
                     "a register.\n");
          return true;
        }

        auto PredReg = SeqotInst->getOperand(1).getReg();
        if (PredReg == CSA::IGN)
          return false;

        for (auto &StrideInst : MRI.use_instructions(PredReg)) {
          if (StrideInst.isDebugValue())
            continue;

          if (!TII.isStride(&StrideInst) ||
              !StrideInst.getOperand(1).isReg() ||
              StrideInst.getOperand(1).getReg() != PredReg ||
              !StrideInst.getOperand(2).isReg() ||
              !StrideInst.getOperand(3).isImm()) {
            LLVM_DEBUG(dbgs() << "Problem - unsupported use of 'pred': " <<
                       StrideInst);
            return true;
          }

          auto StrideBaseReg = StrideInst.getOperand(2).getReg();
          auto FilterInst = getSingleDef(StrideBaseReg, MRI);

          if (!FilterInst || !TII.isFilter(FilterInst) ||
              !areTypesComplying(&StrideInst, FilterInst) ||
              !FilterInst->getOperand(1).isReg() ||
              !FilterInst->getOperand(2).isReg()) {
            LLVM_DEBUG(dbgs() << "Problem - unsupported STRIDE base: " <<
                       (FilterInst ? *FilterInst : StrideInst));
            return true;
          }

          if (FilterInst->getOperand(1).getReg() != Step4CmpPredicate) {
            LLVM_DEBUG(dbgs() << "Problem - unsupported FILTER control: " <<
                       FilterInst);
            return true;
          }

          LLVM_DEBUG(dbgs() << "Adding optional-2 STRIDE for optimization: " <<
                     StrideInst);
          Opt2Instructions.push_back(&StrideInst);
        }

        return false;
      };

      if (std::any_of(OTUses.begin(), OTUses.end(), PredUseIsNotOk))
        continue;

      // Step #6:

      // Process use tree starting from SEQOTs' 'last' defs,
      // and collect REPEATO instructions following optional-3 pattern.
      // Optimizing such a REPEATO is only profitable, if the corresponding
      // FILTER operation may be removed.  This is true iff the FILTER's
      // result is only used by these REPEATOs (e.g. it may have multiple
      // uses, but all the uses are REPEATOs collected here).
      SmallPtrSet<MachineInstr *, 20> Opt3InstructionsCandidates;
      auto AddRepeato = [&](MachineInstr &RepeatoInst) {
        if (RepeatoInst.isDebugValue() || !TII.isRepeatO(&RepeatoInst) ||
            !RepeatoInst.getOperand(2).isReg())
          return;

        auto RepeatoValueReg = RepeatoInst.getOperand(2).getReg();
        if (RepeatoValueReg == CSA::IGN)
          return;

        auto *FilterInst = getSingleDef(RepeatoValueReg, MRI);

        if (!FilterInst || !TII.isFilter(FilterInst))
          return;

        if (!FilterInst->getOperand(1).isReg())
          return;

        auto FilterControlReg = FilterInst->getOperand(1).getReg();

        if (FilterControlReg != Step4CmpPredicate)
          return;

        LLVM_DEBUG(dbgs() << "Adding optional-3 REPEATO for optimization: " <<
                   RepeatoInst);
        Opt3InstructionsCandidates.insert(&RepeatoInst);
      };

      for (auto *SeqotInstr : OTUses) {
        if (TII.isRepeatO(SeqotInstr))
          continue;

        assert(TII.isSeqOT(SeqotInstr) && "Expected SEQOT.");

        if (!SeqotInstr->getOperand(3).isReg())
          continue;

        auto LastReg = SeqotInstr->getOperand(3).getReg();

        if (LastReg == CSA::IGN)
          continue;

        for (auto &NotInstr : MRI.use_instructions(LastReg)) {
          if (NotInstr.isDebugValue() || !TII.isNot(&NotInstr))
            continue;

          if (!NotInstr.getOperand(0).isReg())
            continue;

          auto NotReg = NotInstr.getOperand(0).getReg();
          if (NotReg == CSA::IGN)
            continue;

          std::for_each(MRI.use_instructions(NotReg).begin(),
                        MRI.use_instructions(NotReg).end(),
                        AddRepeato);
        }
      }

      // Remove a REPEATO from the optimization list, if any
      // of the corresponding FILTER's uses is not a REPEATO
      // currently in this list.
      // The intention here is to avoid introducing fanouts
      // for the FILTER's value operand.
      //
      // TODO (vzakhari 10/30/2018):  if the FILTER's value operand
      //       is a parameter, maybe it is safe to assume that the
      //       extra fanouts will not complicate the P&R?
      SmallPtrSet<MachineInstr *, 20> Opt3Instructions;
      for (auto *Repeato : Opt3InstructionsCandidates) {
        auto RepeatoValueReg = Repeato->getOperand(2).getReg();

        SmallVector<MachineInstr *, 20> FilterRepeatoUses;
        bool HasNonRepeatoUses = false;

        for (auto &Use : MRI.use_instructions(RepeatoValueReg)) {
          if (Opt3InstructionsCandidates.count(&Use) == 0) {
            HasNonRepeatoUses = true;
            continue;
          }

          // Keep a list of REPEATO instructions that use the FILTER's
          // result.
          FilterRepeatoUses.push_back(&Use);
        }

        if (!HasNonRepeatoUses) {
          std::for_each(FilterRepeatoUses.begin(),
                        FilterRepeatoUses.end(),
                        [&Opt3Instructions](MachineInstr *MI) {
                          Opt3Instructions.insert(MI);
                        });
        } else {
          // If we found a FILTER with uses outside of the set,
          // then all REPEATOs that use this FILTER should not have
          // been added to the optimization set.
          LLVM_DEBUG(
              std::for_each(FilterRepeatoUses.begin(),
                            FilterRepeatoUses.end(),
                            [&Opt3Instructions](MachineInstr *MI) {
                              assert(Opt3Instructions.count(MI) == 0 &&
                                     "REPEATO should have been dropped.");
                            });
          );
        }
      }

      LLVM_DEBUG(dbgs() << "SUCCESS: the pattern has been recognized.\n");

      // At this point we have the following instruction sets for optimization:
      //     1. OTUses - SEQOT and optional-1 REPEATO instructions that must be
      //        transformed into SEQ and REPEAT instructions.
      //     2. Opt2Instructions - STRIDE instructions that must be transformed
      //        in such a way that their 'base' operand produced by a FILTER
      //        instruction is replaced with the FILTER's 'value' operand.
      //     3. Opt3Instructions - REPEATO instructions that may be transformed
      //        in such a way that their 'value' operand produced by a FILTER
      //        instruction is replaced with the FILTER's 'value' operand,
      //        AND the REPEATO's 'predicate' operand is replaced with
      //        'pred' definition of the corresponding SEQ instruction.
      //
      // We have to transform REPEATOs described in (1) and (3), before
      // transforming SEQOTs desribed in (1): we are looking for a single
      // definition of the corresponding SEQOT's 'pred' operand, when we
      // transform the REPEATOs; if we create a SEQ for a SEQOT, then
      // there will be two definitions of the same register.

      Modified = true;
      doTransformation(OTUses, Opt2Instructions, Opt3Instructions);
    }

  for (auto *MI : InstrsToDelete)
    MI->eraseFromParent();

  LLVM_DEBUG(dbgs() << "--\nEND: SEQOT->SEQ opt.\n");

  return Modified;
}

void CSASeqotToSeqOptimizationImpl::doTransformation(
    SmallPtrSet<MachineInstr *, 10> &SeqotsAndRepeatos,
    ArrayRef<MachineInstr *> Opt2Instructions,
    SmallPtrSet<MachineInstr *, 20> &Opt3Instructions) {
  // Transform REPEATOs described in (1).
  for (auto *Repeato : SeqotsAndRepeatos) {
    if (TII.isRepeatO(Repeato)) {
      // Change REPEATO to REPEAT.
      // We have to use the SEQOT's 'pred' for the REPEAT control,
      // so let's find it.
      auto ControlReg = Repeato->getOperand(1).getReg();
      auto *NotInst = getSingleDef(ControlReg, MRI);
      assert(NotInst && TII.isNot(NotInst) && "Must be able to find NOT.");
      auto NotOpReg = NotInst->getOperand(1).getReg();
      auto *LastPredicateDef = getSingleDef(NotOpReg, MRI);
      assert(LastPredicateDef && TII.isSeqOT(LastPredicateDef) &&
             "Must be able to find SEQOT.");
      // If 'pred' is %ign, the define a new register for it.
      if (LastPredicateDef->getOperand(1).getReg() == CSA::IGN) {
        LastPredicateDef->getOperand(1).setReg(
                                            MFI.allocateLIC(&CSA::CI1RegClass));
        LLVM_DEBUG(dbgs() << "Defined 'pred' operand for SEQOT:\n" <<
                   LastPredicateDef);
      }

      LLVM_DEBUG(dbgs() << "Transforming REPEATO:\n" << *Repeato <<
                 "into REPEAT:\n");
      MachineInstr *NewInst =
        BuildMI(*Repeato->getParent(), Repeato, Repeato->getDebugLoc(),
                TII.get(TII.adjustOpcode(Repeato->getOpcode(),
                                         CSA::Generic::REPEAT)))
        .add(Repeato->getOperand(0))
        .addReg(LastPredicateDef->getOperand(1).getReg())
        .add(Repeato->getOperand(1))
        .setMIFlag(MachineInstr::NonSequential);

      (void)NewInst;
      LLVM_DEBUG(dbgs() << *NewInst);

      InstrsToDelete.push_back(Repeato);
    } else if (!TII.isSeqOT(Repeato))
      llvm_unreachable("Unexpected instruction in SeqotsAndRepeatos.");
  }

  // Transform REPEATOs described in (3).
  for (auto *Repeato : Opt3Instructions) {
    auto ControlReg = Repeato->getOperand(1).getReg();
    auto *NotInst = getSingleDef(ControlReg, MRI);
    assert(NotInst && TII.isNot(NotInst) && "Must be able find NOT.");
    auto NotOpReg = NotInst->getOperand(1).getReg();
    auto *LastPredicateDef = getSingleDef(NotOpReg, MRI);
    assert(LastPredicateDef && TII.isSeqOT(LastPredicateDef) &&
           "Must be able to find SEQOT.");
    // If 'pred' is %ign, the define a new register for it.
    if (LastPredicateDef->getOperand(1).getReg() == CSA::IGN) {
      LastPredicateDef->getOperand(1).setReg(
                                          MFI.allocateLIC(&CSA::CI1RegClass));
      LLVM_DEBUG(dbgs() << "Defined 'pred' operand for SEQOT:\n" <<
                 *LastPredicateDef);
    }

    auto PredReg = LastPredicateDef->getOperand(1).getReg();
    auto BaseReg = Repeato->getOperand(2).getReg();
    auto *FilterDef = getSingleDef(BaseReg, MRI);
    assert(FilterDef && TII.isFilter(FilterDef) &&
           "Must be able to find FILTER.");
    auto ValueReg = FilterDef->getOperand(2).getReg();
    LLVM_DEBUG(dbgs() << "Transforming REPEATO:\n" << *Repeato <<
               "into REPEAT:\n");
    MachineInstr *NewInst =
      BuildMI(*Repeato->getParent(), Repeato, Repeato->getDebugLoc(),
              TII.get(TII.adjustOpcode(Repeato->getOpcode(),
                                       CSA::Generic::REPEAT)))
      .add(Repeato->getOperand(0))
      .addReg(PredReg)
      .addReg(ValueReg)
      .setMIFlag(MachineInstr::NonSequential);

    (void)NewInst;
    LLVM_DEBUG(dbgs() << *NewInst);

    InstrsToDelete.push_back(Repeato);
  }

  // Transform SEQOTs described in (1).
  for (auto *Seqot : SeqotsAndRepeatos) {
    if (TII.isSeqOT(Seqot)) {
      // Change SEQOT to SEQ.
      LLVM_DEBUG(dbgs() << "Transforming SEQOT:\n" << *Seqot <<
                 "into SEQ:\n");
      // I guess setDesc() should work just fine here.
      MachineInstr *NewInst =
        BuildMI(*Seqot->getParent(), Seqot, Seqot->getDebugLoc(),
                TII.get(TII.convertSeqOTToSeqOp(Seqot->getOpcode())))
        .add(Seqot->getOperand(0))
        .add(Seqot->getOperand(1))
        .add(Seqot->getOperand(2))
        .add(Seqot->getOperand(3))
        .add(Seqot->getOperand(4))
        .add(Seqot->getOperand(5))
        .add(Seqot->getOperand(6))
        .setMIFlag(MachineInstr::NonSequential);

      (void)NewInst;
      LLVM_DEBUG(dbgs() << *NewInst);

      InstrsToDelete.push_back(Seqot);
    } else if (!TII.isRepeatO(Seqot))
      llvm_unreachable("Unexpected instruction in SeqotsAndRepeatos.");
  }

  // Transform STRIDEs described in (2).
  for (auto *Stride : Opt2Instructions) {
    auto BaseReg = Stride->getOperand(2).getReg();
    auto *FilterDef = getSingleDef(BaseReg, MRI);
    assert(FilterDef && TII.isFilter(FilterDef) &&
           "Must be able to find FILTER.");
    auto ValueReg = FilterDef->getOperand(2).getReg();
    LLVM_DEBUG(dbgs() << "Replacing 'base' operand of STRIDE:\n" <<
               *Stride << "with 'value' operand of FILTER:\n" <<
               *FilterDef);
    Stride->getOperand(2).setReg(ValueReg);
  }
}
