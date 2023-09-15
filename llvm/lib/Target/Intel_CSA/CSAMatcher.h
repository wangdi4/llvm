//===- CSAMatcher.h - MIR pattern matcher ---------------------------------===//
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
// This file defines pattern-matching opcodes and special values for CSA for
// use with MIRMatcher.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDED_CSAMATCHER_DOT_H
#define INCLUDED_CSAMATCHER_DOT_H

#include "CSAInstrInfo.h"
#include "MCTargetDesc/CSAMCTargetDesc.h"
#include "llvm/CodeGen/Intel_MIRMatcher.h"

#include <cassert>

// Temporary macro to compare matcher against manual pattern match.
// Currently, pattern matching in the MIR back-end is done manually, but we
// want to transition to using CSAMatcher (this component), which is built on
// MIRMatcher.  During the transition, CSAMatcher code is deployed along-side
// manual pattern matching, with the actual results coming from the manual
// code. We verify that the two approaches give the same result by asserting
// conditions with `MATCH_ASSERT`.  If the results differ (causing assert
// failures in the field), disable `MATCH_ASSERT` in by not defining
// `USE_MATCH_ASSERT`.
#define USE_MATCH_ASSERT 1
#if USE_MATCH_ASSERT
#define MATCH_ASSERT(cond) assert(cond)
#else
#define MATCH_ASSERT(cond) ((void)(cond))
#endif

namespace llvm {
namespace CSAMatch {

template <unsigned FixedId> struct PhysicalReg {
  // Match a physical CSA register with a fixed ID.

  static constexpr mirmatch::RegisterSet<> registers{};

  constexpr PhysicalReg() = default;

  template <typename Op, typename Uses>
  constexpr mirmatch::InstructionMatcher<
    Op, mirmatch::OperandMatcherList<PhysicalReg>, Uses>
  operator=(mirmatch::InstructionMatcher<Op, mirmatch::OperandMatcherList<>,
                                         Uses>) const {
    return {};
  }

  static MachineInstr::const_mop_iterator
  matchOperand(mirmatch::MatchResult &rslt,
               MachineInstr::const_mop_iterator op_iter,
               MachineInstr::const_mop_iterator op_end) {
    return (op_iter->isReg() && op_iter->getReg() == FixedId) ? op_iter
                                                              : op_end;
  }
};

} // namespace CSAMatch

namespace mirmatch {

// Indicate that `RegisterMatcher` models the `OperandMatcher` concept.
template <unsigned FixedId>
struct IsOperandMatcher<CSAMatch::PhysicalReg<FixedId>> : std::true_type {};

} // namespace mirmatch

namespace CSAMatch {

constexpr PhysicalReg<CSA::IGN> ign{};
constexpr PhysicalReg<CSA::NA> na{};

constexpr mirmatch::LiteralMatcher<int, 0> litZero{};
constexpr mirmatch::LiteralMatcher<int, 1> litOne{};

} // namespace CSAMatch
} // namespace llvm

#define GET_MIRMATCHERS
#include "CSAGenCSAOpInfo.inc"

namespace llvm {
namespace CSAMatch {
constexpr mirmatch::OpcodeRangeMatcher<CSA::SEQOTGES16, CSA::SEQOTNE8> seqot{};
constexpr mirmatch::OpcodeRangeMatcher<CSA::SEQGES16, CSA::SEQOTNE8> seqozt{};
} // namespace CSAMatch
} // namespace llvm

#endif // INCLUDED_CSAMATCHER_DOT_H
