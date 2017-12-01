//===- CSAMatcher.h - MIR pattern matcher ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
#include "llvm/CodeGen/MIRMatcher.h"

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
#  define MATCH_ASSERT(cond) assert(cond)
#else
#  define MATCH_ASSERT(cond) ((void) (cond))
#endif

namespace llvm {
namespace CSAMatch {

template <unsigned FixedId>
struct PhysicalReg {
  // Match a physical CSA register with a fixed ID.

  static constexpr
  mirmatch::RegisterSet<> registers{};

  constexpr PhysicalReg() = default;

  template <typename Op, typename Uses>
  constexpr mirmatch::InstructionMatcher<Op,
                             mirmatch::OperandMatcherList<PhysicalReg>, Uses>
  operator=(mirmatch::InstructionMatcher<Op, mirmatch::OperandMatcherList<>,
                                        Uses>) const
    { return {}; }

  static MachineInstr::const_mop_iterator
  matchOperand(mirmatch::MatchResult&            rslt,
               MachineInstr::const_mop_iterator op_iter,
               MachineInstr::const_mop_iterator op_end) {
    return (op_iter->isReg() && op_iter->getReg() == FixedId) ?
      op_iter : op_end;
  }
};

} // close namespace llvm::CSAMatch

namespace mirmatch {

// Indicate that `RegisterMatcher` models the `OperandMatcher` concept.
template <unsigned FixedId>
struct IsOperandMatcher<CSAMatch::PhysicalReg<FixedId>> : std::true_type { };

} // close namespace llvm::mirmatch

namespace CSAMatch {

constexpr PhysicalReg<CSA::IGN> ign{};
constexpr PhysicalReg<CSA::NA>  na{};

constexpr mirmatch::LiteralMatcher<int, 0> litZero{};
constexpr mirmatch::LiteralMatcher<int, 1> litOne{};

constexpr mirmatch::Opcode<CSA::ABSF16> absf16{};
constexpr mirmatch::Opcode<CSA::ABSF32> absf32{};
constexpr mirmatch::Opcode<CSA::ABSF64> absf64{};
constexpr mirmatch::Opcode<CSA::ADC16> adc16{};
constexpr mirmatch::Opcode<CSA::ADC32> adc32{};
constexpr mirmatch::Opcode<CSA::ADC64> adc64{};
constexpr mirmatch::Opcode<CSA::ADC8> adc8{};
constexpr mirmatch::Opcode<CSA::ADD16> add16{};
constexpr mirmatch::Opcode<CSA::ADD32> add32{};
constexpr mirmatch::Opcode<CSA::ADD64> add64{};
constexpr mirmatch::Opcode<CSA::ADD8> add8{};
constexpr mirmatch::Opcode<CSA::ADDF16> addf16{};
constexpr mirmatch::Opcode<CSA::ADDF32> addf32{};
constexpr mirmatch::Opcode<CSA::ADDF64> addf64{};
constexpr mirmatch::Opcode<CSA::ADJCALLSTACKDOWN> adjcallstackdown{};
constexpr mirmatch::Opcode<CSA::ADJCALLSTACKUP> adjcallstackup{};
constexpr mirmatch::Opcode<CSA::ALL0> all0{};
constexpr mirmatch::Opcode<CSA::AND1> and1{};
constexpr mirmatch::Opcode<CSA::AND16> and16{};
constexpr mirmatch::Opcode<CSA::AND32> and32{};
constexpr mirmatch::Opcode<CSA::AND64> and64{};
constexpr mirmatch::Opcode<CSA::AND8> and8{};
constexpr mirmatch::Opcode<CSA::ANY0> any0{};
constexpr mirmatch::Opcode<CSA::ATAN2F16> atan2f16{};
constexpr mirmatch::Opcode<CSA::ATAN2F32> atan2f32{};
constexpr mirmatch::Opcode<CSA::ATAN2F64> atan2f64{};
constexpr mirmatch::Opcode<CSA::ATANF16> atanf16{};
constexpr mirmatch::Opcode<CSA::ATANF32> atanf32{};
constexpr mirmatch::Opcode<CSA::ATANF64> atanf64{};
constexpr mirmatch::Opcode<CSA::ATMADD16> atmadd16{};
constexpr mirmatch::Opcode<CSA::ATMADD32> atmadd32{};
constexpr mirmatch::Opcode<CSA::ATMADD64> atmadd64{};
constexpr mirmatch::Opcode<CSA::ATMADD8> atmadd8{};
constexpr mirmatch::Opcode<CSA::ATMADDF32> atmaddf32{};
constexpr mirmatch::Opcode<CSA::ATMADDF64> atmaddf64{};
constexpr mirmatch::Opcode<CSA::ATMAND16> atmand16{};
constexpr mirmatch::Opcode<CSA::ATMAND32> atmand32{};
constexpr mirmatch::Opcode<CSA::ATMAND64> atmand64{};
constexpr mirmatch::Opcode<CSA::ATMAND8> atmand8{};
constexpr mirmatch::Opcode<CSA::ATMCMPXCHG16> atmcmpxchg16{};
constexpr mirmatch::Opcode<CSA::ATMCMPXCHG32> atmcmpxchg32{};
constexpr mirmatch::Opcode<CSA::ATMCMPXCHG64> atmcmpxchg64{};
constexpr mirmatch::Opcode<CSA::ATMCMPXCHG8> atmcmpxchg8{};
constexpr mirmatch::Opcode<CSA::ATMMAX16> atmmax16{};
constexpr mirmatch::Opcode<CSA::ATMMAX32> atmmax32{};
constexpr mirmatch::Opcode<CSA::ATMMAX64> atmmax64{};
constexpr mirmatch::Opcode<CSA::ATMMAX8> atmmax8{};
constexpr mirmatch::Opcode<CSA::ATMMAXF32> atmmaxf32{};
constexpr mirmatch::Opcode<CSA::ATMMAXF64> atmmaxf64{};
constexpr mirmatch::Opcode<CSA::ATMMIN16> atmmin16{};
constexpr mirmatch::Opcode<CSA::ATMMIN32> atmmin32{};
constexpr mirmatch::Opcode<CSA::ATMMIN64> atmmin64{};
constexpr mirmatch::Opcode<CSA::ATMMIN8> atmmin8{};
constexpr mirmatch::Opcode<CSA::ATMMINF32> atmminf32{};
constexpr mirmatch::Opcode<CSA::ATMMINF64> atmminf64{};
constexpr mirmatch::Opcode<CSA::ATMOR16> atmor16{};
constexpr mirmatch::Opcode<CSA::ATMOR32> atmor32{};
constexpr mirmatch::Opcode<CSA::ATMOR64> atmor64{};
constexpr mirmatch::Opcode<CSA::ATMOR8> atmor8{};
constexpr mirmatch::Opcode<CSA::ATMXCHG16> atmxchg16{};
constexpr mirmatch::Opcode<CSA::ATMXCHG32> atmxchg32{};
constexpr mirmatch::Opcode<CSA::ATMXCHG64> atmxchg64{};
constexpr mirmatch::Opcode<CSA::ATMXCHG8> atmxchg8{};
constexpr mirmatch::Opcode<CSA::ATMXOR16> atmxor16{};
constexpr mirmatch::Opcode<CSA::ATMXOR32> atmxor32{};
constexpr mirmatch::Opcode<CSA::ATMXOR64> atmxor64{};
constexpr mirmatch::Opcode<CSA::ATMXOR8> atmxor8{};
constexpr mirmatch::Opcode<CSA::BF> bf{};
constexpr mirmatch::Opcode<CSA::BR> br{};
constexpr mirmatch::Opcode<CSA::BT> bt{};
constexpr mirmatch::Opcode<CSA::CEILF32> ceilf32{};
constexpr mirmatch::Opcode<CSA::CEILF64> ceilf64{};
constexpr mirmatch::Opcode<CSA::CMPEQ16> cmpeq16{};
constexpr mirmatch::Opcode<CSA::CMPEQ32> cmpeq32{};
constexpr mirmatch::Opcode<CSA::CMPEQ64> cmpeq64{};
constexpr mirmatch::Opcode<CSA::CMPEQ8> cmpeq8{};
constexpr mirmatch::Opcode<CSA::CMPGES16> cmpges16{};
constexpr mirmatch::Opcode<CSA::CMPGES32> cmpges32{};
constexpr mirmatch::Opcode<CSA::CMPGES64> cmpges64{};
constexpr mirmatch::Opcode<CSA::CMPGES8> cmpges8{};
constexpr mirmatch::Opcode<CSA::CMPGEU16> cmpgeu16{};
constexpr mirmatch::Opcode<CSA::CMPGEU32> cmpgeu32{};
constexpr mirmatch::Opcode<CSA::CMPGEU64> cmpgeu64{};
constexpr mirmatch::Opcode<CSA::CMPGEU8> cmpgeu8{};
constexpr mirmatch::Opcode<CSA::CMPGTS16> cmpgts16{};
constexpr mirmatch::Opcode<CSA::CMPGTS32> cmpgts32{};
constexpr mirmatch::Opcode<CSA::CMPGTS64> cmpgts64{};
constexpr mirmatch::Opcode<CSA::CMPGTS8> cmpgts8{};
constexpr mirmatch::Opcode<CSA::CMPGTU16> cmpgtu16{};
constexpr mirmatch::Opcode<CSA::CMPGTU32> cmpgtu32{};
constexpr mirmatch::Opcode<CSA::CMPGTU64> cmpgtu64{};
constexpr mirmatch::Opcode<CSA::CMPGTU8> cmpgtu8{};
constexpr mirmatch::Opcode<CSA::CMPLES16> cmples16{};
constexpr mirmatch::Opcode<CSA::CMPLES32> cmples32{};
constexpr mirmatch::Opcode<CSA::CMPLES64> cmples64{};
constexpr mirmatch::Opcode<CSA::CMPLES8> cmples8{};
constexpr mirmatch::Opcode<CSA::CMPLEU16> cmpleu16{};
constexpr mirmatch::Opcode<CSA::CMPLEU32> cmpleu32{};
constexpr mirmatch::Opcode<CSA::CMPLEU64> cmpleu64{};
constexpr mirmatch::Opcode<CSA::CMPLEU8> cmpleu8{};
constexpr mirmatch::Opcode<CSA::CMPLTS16> cmplts16{};
constexpr mirmatch::Opcode<CSA::CMPLTS32> cmplts32{};
constexpr mirmatch::Opcode<CSA::CMPLTS64> cmplts64{};
constexpr mirmatch::Opcode<CSA::CMPLTS8> cmplts8{};
constexpr mirmatch::Opcode<CSA::CMPLTU16> cmpltu16{};
constexpr mirmatch::Opcode<CSA::CMPLTU32> cmpltu32{};
constexpr mirmatch::Opcode<CSA::CMPLTU64> cmpltu64{};
constexpr mirmatch::Opcode<CSA::CMPLTU8> cmpltu8{};
constexpr mirmatch::Opcode<CSA::CMPNE16> cmpne16{};
constexpr mirmatch::Opcode<CSA::CMPNE32> cmpne32{};
constexpr mirmatch::Opcode<CSA::CMPNE64> cmpne64{};
constexpr mirmatch::Opcode<CSA::CMPNE8> cmpne8{};
constexpr mirmatch::Opcode<CSA::CMPOEQF16> cmpoeqf16{};
constexpr mirmatch::Opcode<CSA::CMPOEQF32> cmpoeqf32{};
constexpr mirmatch::Opcode<CSA::CMPOEQF64> cmpoeqf64{};
constexpr mirmatch::Opcode<CSA::CMPOF16> cmpof16{};
constexpr mirmatch::Opcode<CSA::CMPOF32> cmpof32{};
constexpr mirmatch::Opcode<CSA::CMPOF64> cmpof64{};
constexpr mirmatch::Opcode<CSA::CMPOGEF16> cmpogef16{};
constexpr mirmatch::Opcode<CSA::CMPOGEF32> cmpogef32{};
constexpr mirmatch::Opcode<CSA::CMPOGEF64> cmpogef64{};
constexpr mirmatch::Opcode<CSA::CMPOGTF16> cmpogtf16{};
constexpr mirmatch::Opcode<CSA::CMPOGTF32> cmpogtf32{};
constexpr mirmatch::Opcode<CSA::CMPOGTF64> cmpogtf64{};
constexpr mirmatch::Opcode<CSA::CMPOLEF16> cmpolef16{};
constexpr mirmatch::Opcode<CSA::CMPOLEF32> cmpolef32{};
constexpr mirmatch::Opcode<CSA::CMPOLEF64> cmpolef64{};
constexpr mirmatch::Opcode<CSA::CMPOLTF16> cmpoltf16{};
constexpr mirmatch::Opcode<CSA::CMPOLTF32> cmpoltf32{};
constexpr mirmatch::Opcode<CSA::CMPOLTF64> cmpoltf64{};
constexpr mirmatch::Opcode<CSA::CMPONEF16> cmponef16{};
constexpr mirmatch::Opcode<CSA::CMPONEF32> cmponef32{};
constexpr mirmatch::Opcode<CSA::CMPONEF64> cmponef64{};
constexpr mirmatch::Opcode<CSA::CMPUEQF16> cmpueqf16{};
constexpr mirmatch::Opcode<CSA::CMPUEQF32> cmpueqf32{};
constexpr mirmatch::Opcode<CSA::CMPUEQF64> cmpueqf64{};
constexpr mirmatch::Opcode<CSA::CMPUGEF16> cmpugef16{};
constexpr mirmatch::Opcode<CSA::CMPUGEF32> cmpugef32{};
constexpr mirmatch::Opcode<CSA::CMPUGEF64> cmpugef64{};
constexpr mirmatch::Opcode<CSA::CMPUGTF16> cmpugtf16{};
constexpr mirmatch::Opcode<CSA::CMPUGTF32> cmpugtf32{};
constexpr mirmatch::Opcode<CSA::CMPUGTF64> cmpugtf64{};
constexpr mirmatch::Opcode<CSA::CMPULEF16> cmpulef16{};
constexpr mirmatch::Opcode<CSA::CMPULEF32> cmpulef32{};
constexpr mirmatch::Opcode<CSA::CMPULEF64> cmpulef64{};
constexpr mirmatch::Opcode<CSA::CMPULTF16> cmpultf16{};
constexpr mirmatch::Opcode<CSA::CMPULTF32> cmpultf32{};
constexpr mirmatch::Opcode<CSA::CMPULTF64> cmpultf64{};
constexpr mirmatch::Opcode<CSA::CMPUNEF16> cmpunef16{};
constexpr mirmatch::Opcode<CSA::CMPUNEF32> cmpunef32{};
constexpr mirmatch::Opcode<CSA::CMPUNEF64> cmpunef64{};
constexpr mirmatch::Opcode<CSA::CMPUOF16> cmpuof16{};
constexpr mirmatch::Opcode<CSA::CMPUOF32> cmpuof32{};
constexpr mirmatch::Opcode<CSA::CMPUOF64> cmpuof64{};
constexpr mirmatch::Opcode<CSA::COPY0> copy0{};
constexpr mirmatch::Opcode<CSA::COPY1> copy1{};
constexpr mirmatch::Opcode<CSA::COPY16> copy16{};
constexpr mirmatch::Opcode<CSA::COPY32> copy32{};
constexpr mirmatch::Opcode<CSA::COPY64> copy64{};
constexpr mirmatch::Opcode<CSA::COPY8> copy8{};
constexpr mirmatch::Opcode<CSA::COSF16> cosf16{};
constexpr mirmatch::Opcode<CSA::COSF32> cosf32{};
constexpr mirmatch::Opcode<CSA::COSF64> cosf64{};
constexpr mirmatch::Opcode<CSA::CSA_DIRECTIVE> csa_directive{};
constexpr mirmatch::Opcode<CSA::CSA_PARALLEL_LOOP> csa_parallel_loop{};
constexpr mirmatch::Opcode<CSA::CSA_PARALLEL_MEMDEP> csa_parallel_memdep{};
constexpr mirmatch::Opcode<CSA::CSA_PARALLEL_REGION_ENTRY> csa_parallel_region_entry{};
constexpr mirmatch::Opcode<CSA::CSA_PARALLEL_REGION_EXIT> csa_parallel_region_exit{};
constexpr mirmatch::Opcode<CSA::CSA_PARALLEL_SECTION_ENTRY> csa_parallel_section_entry{};
constexpr mirmatch::Opcode<CSA::CSA_PARALLEL_SECTION_EXIT> csa_parallel_section_exit{};
constexpr mirmatch::Opcode<CSA::CTLZ16> ctlz16{};
constexpr mirmatch::Opcode<CSA::CTLZ32> ctlz32{};
constexpr mirmatch::Opcode<CSA::CTLZ64> ctlz64{};
constexpr mirmatch::Opcode<CSA::CTLZ8> ctlz8{};
constexpr mirmatch::Opcode<CSA::CTPOP16> ctpop16{};
constexpr mirmatch::Opcode<CSA::CTPOP32> ctpop32{};
constexpr mirmatch::Opcode<CSA::CTPOP64> ctpop64{};
constexpr mirmatch::Opcode<CSA::CTPOP8> ctpop8{};
constexpr mirmatch::Opcode<CSA::CTTZ16> cttz16{};
constexpr mirmatch::Opcode<CSA::CTTZ32> cttz32{};
constexpr mirmatch::Opcode<CSA::CTTZ64> cttz64{};
constexpr mirmatch::Opcode<CSA::CTTZ8> cttz8{};
constexpr mirmatch::Opcode<CSA::CVTF32F64> cvtf32f64{};
constexpr mirmatch::Opcode<CSA::CVTF32S32> cvtf32s32{};
constexpr mirmatch::Opcode<CSA::CVTF32S64> cvtf32s64{};
constexpr mirmatch::Opcode<CSA::CVTF32U32> cvtf32u32{};
constexpr mirmatch::Opcode<CSA::CVTF32U64> cvtf32u64{};
constexpr mirmatch::Opcode<CSA::CVTF64F32> cvtf64f32{};
constexpr mirmatch::Opcode<CSA::CVTF64S32> cvtf64s32{};
constexpr mirmatch::Opcode<CSA::CVTF64S64> cvtf64s64{};
constexpr mirmatch::Opcode<CSA::CVTF64U32> cvtf64u32{};
constexpr mirmatch::Opcode<CSA::CVTF64U64> cvtf64u64{};
constexpr mirmatch::Opcode<CSA::CVTS32F32> cvts32f32{};
constexpr mirmatch::Opcode<CSA::CVTS32F64> cvts32f64{};
constexpr mirmatch::Opcode<CSA::CVTS64F32> cvts64f32{};
constexpr mirmatch::Opcode<CSA::CVTS64F64> cvts64f64{};
constexpr mirmatch::Opcode<CSA::CVTU32F32> cvtu32f32{};
constexpr mirmatch::Opcode<CSA::CVTU32F64> cvtu32f64{};
constexpr mirmatch::Opcode<CSA::CVTU64F32> cvtu64f32{};
constexpr mirmatch::Opcode<CSA::CVTU64F64> cvtu64f64{};
constexpr mirmatch::Opcode<CSA::DEBUGPRINT> debugprint{};
constexpr mirmatch::Opcode<CSA::DIVCHECKF32> divcheckf32{};
constexpr mirmatch::Opcode<CSA::DIVCHECKF64> divcheckf64{};
constexpr mirmatch::Opcode<CSA::DIVF16> divf16{};
constexpr mirmatch::Opcode<CSA::DIVF32> divf32{};
constexpr mirmatch::Opcode<CSA::DIVF64> divf64{};
constexpr mirmatch::Opcode<CSA::DIVS16> divs16{};
constexpr mirmatch::Opcode<CSA::DIVS32> divs32{};
constexpr mirmatch::Opcode<CSA::DIVS64> divs64{};
constexpr mirmatch::Opcode<CSA::DIVS8> divs8{};
constexpr mirmatch::Opcode<CSA::DIVU16> divu16{};
constexpr mirmatch::Opcode<CSA::DIVU32> divu32{};
constexpr mirmatch::Opcode<CSA::DIVU64> divu64{};
constexpr mirmatch::Opcode<CSA::DIVU8> divu8{};
constexpr mirmatch::Opcode<CSA::EXP2F16> exp2f16{};
constexpr mirmatch::Opcode<CSA::EXP2F32> exp2f32{};
constexpr mirmatch::Opcode<CSA::EXP2F64> exp2f64{};
constexpr mirmatch::Opcode<CSA::EXPF16> expf16{};
constexpr mirmatch::Opcode<CSA::EXPF32> expf32{};
constexpr mirmatch::Opcode<CSA::EXPF64> expf64{};
constexpr mirmatch::Opcode<CSA::FLOORF32> floorf32{};
constexpr mirmatch::Opcode<CSA::FLOORF64> floorf64{};
constexpr mirmatch::Opcode<CSA::FMAF16> fmaf16{};
constexpr mirmatch::Opcode<CSA::FMAF32> fmaf32{};
constexpr mirmatch::Opcode<CSA::FMAF64> fmaf64{};
constexpr mirmatch::Opcode<CSA::FMAORSF32> fmaorsf32{};
constexpr mirmatch::Opcode<CSA::FMAORSF64> fmaorsf64{};
constexpr mirmatch::Opcode<CSA::FMRSF16> fmrsf16{};
constexpr mirmatch::Opcode<CSA::FMRSF32> fmrsf32{};
constexpr mirmatch::Opcode<CSA::FMRSF64> fmrsf64{};
constexpr mirmatch::Opcode<CSA::FMRSORSF32> fmrsorsf32{};
constexpr mirmatch::Opcode<CSA::FMRSORSF64> fmrsorsf64{};
constexpr mirmatch::Opcode<CSA::FMSF16> fmsf16{};
constexpr mirmatch::Opcode<CSA::FMSF32> fmsf32{};
constexpr mirmatch::Opcode<CSA::FMSF64> fmsf64{};
constexpr mirmatch::Opcode<CSA::FMSORSF32> fmsorsf32{};
constexpr mirmatch::Opcode<CSA::FMSORSF64> fmsorsf64{};
constexpr mirmatch::Opcode<CSA::FMSREDAF32> fmsredaf32{};
constexpr mirmatch::Opcode<CSA::FMSREDAF64> fmsredaf64{};
constexpr mirmatch::Opcode<CSA::GETEXPF32> getexpf32{};
constexpr mirmatch::Opcode<CSA::GETEXPF64> getexpf64{};
constexpr mirmatch::Opcode<CSA::GETMANTF32> getmantf32{};
constexpr mirmatch::Opcode<CSA::GETMANTF64> getmantf64{};
constexpr mirmatch::Opcode<CSA::INIT0> init0{};
constexpr mirmatch::Opcode<CSA::INIT1> init1{};
constexpr mirmatch::Opcode<CSA::INIT16> init16{};
constexpr mirmatch::Opcode<CSA::INIT32> init32{};
constexpr mirmatch::Opcode<CSA::INIT64> init64{};
constexpr mirmatch::Opcode<CSA::INIT8> init8{};
constexpr mirmatch::Opcode<CSA::JMP> jmp{};
constexpr mirmatch::Opcode<CSA::JSR> jsr{};
constexpr mirmatch::Opcode<CSA::JSRi> jsri{};
constexpr mirmatch::Opcode<CSA::JTR> jtr{};
constexpr mirmatch::Opcode<CSA::JTRi> jtri{};
constexpr mirmatch::Opcode<CSA::LAND1> land1{};
constexpr mirmatch::Opcode<CSA::LD1> ld1{};
constexpr mirmatch::Opcode<CSA::LD16> ld16{};
constexpr mirmatch::Opcode<CSA::LD16D> ld16d{};
constexpr mirmatch::Opcode<CSA::LD16I> ld16i{};
constexpr mirmatch::Opcode<CSA::LD16R> ld16r{};
constexpr mirmatch::Opcode<CSA::LD16X> ld16x{};
constexpr mirmatch::Opcode<CSA::LD16f> ld16f{};
constexpr mirmatch::Opcode<CSA::LD16fD> ld16fd{};
constexpr mirmatch::Opcode<CSA::LD16fI> ld16fi{};
constexpr mirmatch::Opcode<CSA::LD16fR> ld16fr{};
constexpr mirmatch::Opcode<CSA::LD16fX> ld16fx{};
constexpr mirmatch::Opcode<CSA::LD1D> ld1d{};
constexpr mirmatch::Opcode<CSA::LD1I> ld1i{};
constexpr mirmatch::Opcode<CSA::LD1R> ld1r{};
constexpr mirmatch::Opcode<CSA::LD1X> ld1x{};
constexpr mirmatch::Opcode<CSA::LD32> ld32{};
constexpr mirmatch::Opcode<CSA::LD32D> ld32d{};
constexpr mirmatch::Opcode<CSA::LD32I> ld32i{};
constexpr mirmatch::Opcode<CSA::LD32R> ld32r{};
constexpr mirmatch::Opcode<CSA::LD32X> ld32x{};
constexpr mirmatch::Opcode<CSA::LD32f> ld32f{};
constexpr mirmatch::Opcode<CSA::LD32fD> ld32fd{};
constexpr mirmatch::Opcode<CSA::LD32fI> ld32fi{};
constexpr mirmatch::Opcode<CSA::LD32fR> ld32fr{};
constexpr mirmatch::Opcode<CSA::LD32fX> ld32fx{};
constexpr mirmatch::Opcode<CSA::LD64> ld64{};
constexpr mirmatch::Opcode<CSA::LD64D> ld64d{};
constexpr mirmatch::Opcode<CSA::LD64I> ld64i{};
constexpr mirmatch::Opcode<CSA::LD64R> ld64r{};
constexpr mirmatch::Opcode<CSA::LD64X> ld64x{};
constexpr mirmatch::Opcode<CSA::LD64f> ld64f{};
constexpr mirmatch::Opcode<CSA::LD64fD> ld64fd{};
constexpr mirmatch::Opcode<CSA::LD64fI> ld64fi{};
constexpr mirmatch::Opcode<CSA::LD64fR> ld64fr{};
constexpr mirmatch::Opcode<CSA::LD64fX> ld64fx{};
constexpr mirmatch::Opcode<CSA::LD8> ld8{};
constexpr mirmatch::Opcode<CSA::LD8D> ld8d{};
constexpr mirmatch::Opcode<CSA::LD8I> ld8i{};
constexpr mirmatch::Opcode<CSA::LD8R> ld8r{};
constexpr mirmatch::Opcode<CSA::LD8X> ld8x{};
constexpr mirmatch::Opcode<CSA::LOG2F16> log2f16{};
constexpr mirmatch::Opcode<CSA::LOG2F32> log2f32{};
constexpr mirmatch::Opcode<CSA::LOG2F64> log2f64{};
constexpr mirmatch::Opcode<CSA::LOGF16> logf16{};
constexpr mirmatch::Opcode<CSA::LOGF32> logf32{};
constexpr mirmatch::Opcode<CSA::LOGF64> logf64{};
constexpr mirmatch::Opcode<CSA::LOR1> lor1{};
constexpr mirmatch::Opcode<CSA::MERGE0> merge0{};
constexpr mirmatch::Opcode<CSA::MERGE1> merge1{};
constexpr mirmatch::Opcode<CSA::MERGE16> merge16{};
constexpr mirmatch::Opcode<CSA::MERGE16f> merge16f{};
constexpr mirmatch::Opcode<CSA::MERGE32> merge32{};
constexpr mirmatch::Opcode<CSA::MERGE32f> merge32f{};
constexpr mirmatch::Opcode<CSA::MERGE64> merge64{};
constexpr mirmatch::Opcode<CSA::MERGE64f> merge64f{};
constexpr mirmatch::Opcode<CSA::MERGE8> merge8{};
constexpr mirmatch::Opcode<CSA::MODF32> modf32{};
constexpr mirmatch::Opcode<CSA::MODF64> modf64{};
constexpr mirmatch::Opcode<CSA::MOV0> mov0{};
constexpr mirmatch::Opcode<CSA::MOV1> mov1{};
constexpr mirmatch::Opcode<CSA::MOV16> mov16{};
constexpr mirmatch::Opcode<CSA::MOV32> mov32{};
constexpr mirmatch::Opcode<CSA::MOV64> mov64{};
constexpr mirmatch::Opcode<CSA::MOV8> mov8{};
constexpr mirmatch::Opcode<CSA::MUL16> mul16{};
constexpr mirmatch::Opcode<CSA::MUL32> mul32{};
constexpr mirmatch::Opcode<CSA::MUL64> mul64{};
constexpr mirmatch::Opcode<CSA::MUL8> mul8{};
constexpr mirmatch::Opcode<CSA::MULF16> mulf16{};
constexpr mirmatch::Opcode<CSA::MULF32> mulf32{};
constexpr mirmatch::Opcode<CSA::MULF64> mulf64{};
constexpr mirmatch::Opcode<CSA::MULLOHIS16> mullohis16{};
constexpr mirmatch::Opcode<CSA::MULLOHIS32> mullohis32{};
constexpr mirmatch::Opcode<CSA::MULLOHIS64> mullohis64{};
constexpr mirmatch::Opcode<CSA::MULLOHIS8> mullohis8{};
constexpr mirmatch::Opcode<CSA::MULLOHIU16> mullohiu16{};
constexpr mirmatch::Opcode<CSA::MULLOHIU32> mullohiu32{};
constexpr mirmatch::Opcode<CSA::MULLOHIU64> mullohiu64{};
constexpr mirmatch::Opcode<CSA::MULLOHIU8> mullohiu8{};
constexpr mirmatch::Opcode<CSA::NEG16> neg16{};
constexpr mirmatch::Opcode<CSA::NEG32> neg32{};
constexpr mirmatch::Opcode<CSA::NEG64> neg64{};
constexpr mirmatch::Opcode<CSA::NEG8> neg8{};
constexpr mirmatch::Opcode<CSA::NEGF16> negf16{};
constexpr mirmatch::Opcode<CSA::NEGF32> negf32{};
constexpr mirmatch::Opcode<CSA::NEGF64> negf64{};
constexpr mirmatch::Opcode<CSA::NOT1> not1{};
constexpr mirmatch::Opcode<CSA::NOT16> not16{};
constexpr mirmatch::Opcode<CSA::NOT32> not32{};
constexpr mirmatch::Opcode<CSA::NOT64> not64{};
constexpr mirmatch::Opcode<CSA::NOT8> not8{};
constexpr mirmatch::Opcode<CSA::ONCOUNT0> oncount0{};
constexpr mirmatch::Opcode<CSA::ONEND> onend{};
constexpr mirmatch::Opcode<CSA::OR1> or1{};
constexpr mirmatch::Opcode<CSA::OR16> or16{};
constexpr mirmatch::Opcode<CSA::OR32> or32{};
constexpr mirmatch::Opcode<CSA::OR64> or64{};
constexpr mirmatch::Opcode<CSA::OR8> or8{};
constexpr mirmatch::Opcode<CSA::PARITY16> parity16{};
constexpr mirmatch::Opcode<CSA::PARITY32> parity32{};
constexpr mirmatch::Opcode<CSA::PARITY64> parity64{};
constexpr mirmatch::Opcode<CSA::PARITY8> parity8{};
constexpr mirmatch::Opcode<CSA::PICK0> pick0{};
constexpr mirmatch::Opcode<CSA::PICK1> pick1{};
constexpr mirmatch::Opcode<CSA::PICK16> pick16{};
constexpr mirmatch::Opcode<CSA::PICK32> pick32{};
constexpr mirmatch::Opcode<CSA::PICK64> pick64{};
constexpr mirmatch::Opcode<CSA::PICK8> pick8{};
constexpr mirmatch::Opcode<CSA::PICKANY0> pickany0{};
constexpr mirmatch::Opcode<CSA::PICKANY1> pickany1{};
constexpr mirmatch::Opcode<CSA::PICKANY16> pickany16{};
constexpr mirmatch::Opcode<CSA::PICKANY32> pickany32{};
constexpr mirmatch::Opcode<CSA::PICKANY64> pickany64{};
constexpr mirmatch::Opcode<CSA::PICKANY8> pickany8{};
constexpr mirmatch::Opcode<CSA::POWF32> powf32{};
constexpr mirmatch::Opcode<CSA::POWF64> powf64{};
constexpr mirmatch::Opcode<CSA::PREDFILTER> predfilter{};
constexpr mirmatch::Opcode<CSA::PREDMERGE> predmerge{};
constexpr mirmatch::Opcode<CSA::PREDPROP> predprop{};
constexpr mirmatch::Opcode<CSA::PREFETCH> prefetch{};
constexpr mirmatch::Opcode<CSA::PREFETCHI> prefetchi{};
constexpr mirmatch::Opcode<CSA::PREFETCHW> prefetchw{};
constexpr mirmatch::Opcode<CSA::PREFETCHWI> prefetchwi{};
constexpr mirmatch::Opcode<CSA::RCP14F32> rcp14f32{};
constexpr mirmatch::Opcode<CSA::RCP14F64> rcp14f64{};
constexpr mirmatch::Opcode<CSA::REPEAT0> repeat0{};
constexpr mirmatch::Opcode<CSA::REPEAT1> repeat1{};
constexpr mirmatch::Opcode<CSA::REPEAT16> repeat16{};
constexpr mirmatch::Opcode<CSA::REPEAT32> repeat32{};
constexpr mirmatch::Opcode<CSA::REPEAT64> repeat64{};
constexpr mirmatch::Opcode<CSA::REPEAT8> repeat8{};
constexpr mirmatch::Opcode<CSA::RET> ret{};
constexpr mirmatch::Opcode<CSA::RNDSCALEF32> rndscalef32{};
constexpr mirmatch::Opcode<CSA::RNDSCALEF64> rndscalef64{};
constexpr mirmatch::Opcode<CSA::RNDSCALESPEF32> rndscalespef32{};
constexpr mirmatch::Opcode<CSA::RNDSCALESPEF64> rndscalespef64{};
constexpr mirmatch::Opcode<CSA::ROUNDF32> roundf32{};
constexpr mirmatch::Opcode<CSA::ROUNDF64> roundf64{};
constexpr mirmatch::Opcode<CSA::RSQRT14F32> rsqrt14f32{};
constexpr mirmatch::Opcode<CSA::RSQRT14F64> rsqrt14f64{};
constexpr mirmatch::Opcode<CSA::SBB16> sbb16{};
constexpr mirmatch::Opcode<CSA::SBB32> sbb32{};
constexpr mirmatch::Opcode<CSA::SBB64> sbb64{};
constexpr mirmatch::Opcode<CSA::SBB8> sbb8{};
constexpr mirmatch::Opcode<CSA::SCALEF32> scalef32{};
constexpr mirmatch::Opcode<CSA::SCALEF64> scalef64{};
constexpr mirmatch::Opcode<CSA::SCALEIRSF32> scaleirsf32{};
constexpr mirmatch::Opcode<CSA::SCALEIRSF64> scaleirsf64{};
constexpr mirmatch::Opcode<CSA::SEQC16> seqc16{};
constexpr mirmatch::Opcode<CSA::SEQC32> seqc32{};
constexpr mirmatch::Opcode<CSA::SEQC64> seqc64{};
constexpr mirmatch::Opcode<CSA::SEQC8> seqc8{};
constexpr mirmatch::Opcode<CSA::SEQGES16> seqges16{};
constexpr mirmatch::Opcode<CSA::SEQGES32> seqges32{};
constexpr mirmatch::Opcode<CSA::SEQGES64> seqges64{};
constexpr mirmatch::Opcode<CSA::SEQGES8> seqges8{};
constexpr mirmatch::Opcode<CSA::SEQGEU16> seqgeu16{};
constexpr mirmatch::Opcode<CSA::SEQGEU32> seqgeu32{};
constexpr mirmatch::Opcode<CSA::SEQGEU64> seqgeu64{};
constexpr mirmatch::Opcode<CSA::SEQGEU8> seqgeu8{};
constexpr mirmatch::Opcode<CSA::SEQGTS16> seqgts16{};
constexpr mirmatch::Opcode<CSA::SEQGTS32> seqgts32{};
constexpr mirmatch::Opcode<CSA::SEQGTS64> seqgts64{};
constexpr mirmatch::Opcode<CSA::SEQGTS8> seqgts8{};
constexpr mirmatch::Opcode<CSA::SEQGTU16> seqgtu16{};
constexpr mirmatch::Opcode<CSA::SEQGTU32> seqgtu32{};
constexpr mirmatch::Opcode<CSA::SEQGTU64> seqgtu64{};
constexpr mirmatch::Opcode<CSA::SEQGTU8> seqgtu8{};
constexpr mirmatch::Opcode<CSA::SEQLES16> seqles16{};
constexpr mirmatch::Opcode<CSA::SEQLES32> seqles32{};
constexpr mirmatch::Opcode<CSA::SEQLES64> seqles64{};
constexpr mirmatch::Opcode<CSA::SEQLES8> seqles8{};
constexpr mirmatch::Opcode<CSA::SEQLEU16> seqleu16{};
constexpr mirmatch::Opcode<CSA::SEQLEU32> seqleu32{};
constexpr mirmatch::Opcode<CSA::SEQLEU64> seqleu64{};
constexpr mirmatch::Opcode<CSA::SEQLEU8> seqleu8{};
constexpr mirmatch::Opcode<CSA::SEQLTS16> seqlts16{};
constexpr mirmatch::Opcode<CSA::SEQLTS32> seqlts32{};
constexpr mirmatch::Opcode<CSA::SEQLTS64> seqlts64{};
constexpr mirmatch::Opcode<CSA::SEQLTS8> seqlts8{};
constexpr mirmatch::Opcode<CSA::SEQLTU16> seqltu16{};
constexpr mirmatch::Opcode<CSA::SEQLTU32> seqltu32{};
constexpr mirmatch::Opcode<CSA::SEQLTU64> seqltu64{};
constexpr mirmatch::Opcode<CSA::SEQLTU8> seqltu8{};
constexpr mirmatch::Opcode<CSA::SEQNE16> seqne16{};
constexpr mirmatch::Opcode<CSA::SEQNE32> seqne32{};
constexpr mirmatch::Opcode<CSA::SEQNE64> seqne64{};
constexpr mirmatch::Opcode<CSA::SEQNE8> seqne8{};
constexpr mirmatch::Opcode<CSA::SEQOTGES16> seqotges16{};
constexpr mirmatch::Opcode<CSA::SEQOTGES32> seqotges32{};
constexpr mirmatch::Opcode<CSA::SEQOTGES64> seqotges64{};
constexpr mirmatch::Opcode<CSA::SEQOTGES8> seqotges8{};
constexpr mirmatch::Opcode<CSA::SEQOTGEU16> seqotgeu16{};
constexpr mirmatch::Opcode<CSA::SEQOTGEU32> seqotgeu32{};
constexpr mirmatch::Opcode<CSA::SEQOTGEU64> seqotgeu64{};
constexpr mirmatch::Opcode<CSA::SEQOTGEU8> seqotgeu8{};
constexpr mirmatch::Opcode<CSA::SEQOTGTS16> seqotgts16{};
constexpr mirmatch::Opcode<CSA::SEQOTGTS32> seqotgts32{};
constexpr mirmatch::Opcode<CSA::SEQOTGTS64> seqotgts64{};
constexpr mirmatch::Opcode<CSA::SEQOTGTS8> seqotgts8{};
constexpr mirmatch::Opcode<CSA::SEQOTGTU16> seqotgtu16{};
constexpr mirmatch::Opcode<CSA::SEQOTGTU32> seqotgtu32{};
constexpr mirmatch::Opcode<CSA::SEQOTGTU64> seqotgtu64{};
constexpr mirmatch::Opcode<CSA::SEQOTGTU8> seqotgtu8{};
constexpr mirmatch::Opcode<CSA::SEQOTLES16> seqotles16{};
constexpr mirmatch::Opcode<CSA::SEQOTLES32> seqotles32{};
constexpr mirmatch::Opcode<CSA::SEQOTLES64> seqotles64{};
constexpr mirmatch::Opcode<CSA::SEQOTLES8> seqotles8{};
constexpr mirmatch::Opcode<CSA::SEQOTLEU16> seqotleu16{};
constexpr mirmatch::Opcode<CSA::SEQOTLEU32> seqotleu32{};
constexpr mirmatch::Opcode<CSA::SEQOTLEU64> seqotleu64{};
constexpr mirmatch::Opcode<CSA::SEQOTLEU8> seqotleu8{};
constexpr mirmatch::Opcode<CSA::SEQOTLTS16> seqotlts16{};
constexpr mirmatch::Opcode<CSA::SEQOTLTS32> seqotlts32{};
constexpr mirmatch::Opcode<CSA::SEQOTLTS64> seqotlts64{};
constexpr mirmatch::Opcode<CSA::SEQOTLTS8> seqotlts8{};
constexpr mirmatch::Opcode<CSA::SEQOTLTU16> seqotltu16{};
constexpr mirmatch::Opcode<CSA::SEQOTLTU32> seqotltu32{};
constexpr mirmatch::Opcode<CSA::SEQOTLTU64> seqotltu64{};
constexpr mirmatch::Opcode<CSA::SEQOTLTU8> seqotltu8{};
constexpr mirmatch::Opcode<CSA::SEQOTNE16> seqotne16{};
constexpr mirmatch::Opcode<CSA::SEQOTNE32> seqotne32{};
constexpr mirmatch::Opcode<CSA::SEQOTNE64> seqotne64{};
constexpr mirmatch::Opcode<CSA::SEQOTNE8> seqotne8{};
constexpr mirmatch::Opcode<CSA::SEXT16> sext16{};
constexpr mirmatch::Opcode<CSA::SEXT32> sext32{};
constexpr mirmatch::Opcode<CSA::SEXT64> sext64{};
constexpr mirmatch::Opcode<CSA::SEXT8> sext8{};
constexpr mirmatch::Opcode<CSA::SINF16> sinf16{};
constexpr mirmatch::Opcode<CSA::SINF32> sinf32{};
constexpr mirmatch::Opcode<CSA::SINF64> sinf64{};
constexpr mirmatch::Opcode<CSA::SLADD16> sladd16{};
constexpr mirmatch::Opcode<CSA::SLADD32> sladd32{};
constexpr mirmatch::Opcode<CSA::SLADD64> sladd64{};
constexpr mirmatch::Opcode<CSA::SLADD8> sladd8{};
constexpr mirmatch::Opcode<CSA::SLD1> sld1{};
constexpr mirmatch::Opcode<CSA::SLD16> sld16{};
constexpr mirmatch::Opcode<CSA::SLD16I> sld16i{};
constexpr mirmatch::Opcode<CSA::SLD16f> sld16f{};
constexpr mirmatch::Opcode<CSA::SLD16fI> sld16fi{};
constexpr mirmatch::Opcode<CSA::SLD1I> sld1i{};
constexpr mirmatch::Opcode<CSA::SLD32> sld32{};
constexpr mirmatch::Opcode<CSA::SLD32I> sld32i{};
constexpr mirmatch::Opcode<CSA::SLD32f> sld32f{};
constexpr mirmatch::Opcode<CSA::SLD32fI> sld32fi{};
constexpr mirmatch::Opcode<CSA::SLD64> sld64{};
constexpr mirmatch::Opcode<CSA::SLD64I> sld64i{};
constexpr mirmatch::Opcode<CSA::SLD64f> sld64f{};
constexpr mirmatch::Opcode<CSA::SLD64fI> sld64fi{};
constexpr mirmatch::Opcode<CSA::SLD8> sld8{};
constexpr mirmatch::Opcode<CSA::SLD8I> sld8i{};
constexpr mirmatch::Opcode<CSA::SLL16> sll16{};
constexpr mirmatch::Opcode<CSA::SLL32> sll32{};
constexpr mirmatch::Opcode<CSA::SLL64> sll64{};
constexpr mirmatch::Opcode<CSA::SLL8> sll8{};
constexpr mirmatch::Opcode<CSA::SQRTCHECKF32> sqrtcheckf32{};
constexpr mirmatch::Opcode<CSA::SQRTCHECKF64> sqrtcheckf64{};
constexpr mirmatch::Opcode<CSA::SQRTF16> sqrtf16{};
constexpr mirmatch::Opcode<CSA::SQRTF32> sqrtf32{};
constexpr mirmatch::Opcode<CSA::SQRTF64> sqrtf64{};
constexpr mirmatch::Opcode<CSA::SRA16> sra16{};
constexpr mirmatch::Opcode<CSA::SRA32> sra32{};
constexpr mirmatch::Opcode<CSA::SRA64> sra64{};
constexpr mirmatch::Opcode<CSA::SRA8> sra8{};
constexpr mirmatch::Opcode<CSA::SREDADD16> sredadd16{};
constexpr mirmatch::Opcode<CSA::SREDADD32> sredadd32{};
constexpr mirmatch::Opcode<CSA::SREDADD64> sredadd64{};
constexpr mirmatch::Opcode<CSA::SREDADD8> sredadd8{};
constexpr mirmatch::Opcode<CSA::SREDADDF32> sredaddf32{};
constexpr mirmatch::Opcode<CSA::SREDADDF64> sredaddf64{};
constexpr mirmatch::Opcode<CSA::SREDAND16> sredand16{};
constexpr mirmatch::Opcode<CSA::SREDAND32> sredand32{};
constexpr mirmatch::Opcode<CSA::SREDAND64> sredand64{};
constexpr mirmatch::Opcode<CSA::SREDAND8> sredand8{};
constexpr mirmatch::Opcode<CSA::SREDMUL16> sredmul16{};
constexpr mirmatch::Opcode<CSA::SREDMUL32> sredmul32{};
constexpr mirmatch::Opcode<CSA::SREDMUL64> sredmul64{};
constexpr mirmatch::Opcode<CSA::SREDMUL8> sredmul8{};
constexpr mirmatch::Opcode<CSA::SREDMULF32> sredmulf32{};
constexpr mirmatch::Opcode<CSA::SREDMULF64> sredmulf64{};
constexpr mirmatch::Opcode<CSA::SREDOR16> sredor16{};
constexpr mirmatch::Opcode<CSA::SREDOR32> sredor32{};
constexpr mirmatch::Opcode<CSA::SREDOR64> sredor64{};
constexpr mirmatch::Opcode<CSA::SREDOR8> sredor8{};
constexpr mirmatch::Opcode<CSA::SREDSUB16> sredsub16{};
constexpr mirmatch::Opcode<CSA::SREDSUB32> sredsub32{};
constexpr mirmatch::Opcode<CSA::SREDSUB64> sredsub64{};
constexpr mirmatch::Opcode<CSA::SREDSUB8> sredsub8{};
constexpr mirmatch::Opcode<CSA::SREDSUBF32> sredsubf32{};
constexpr mirmatch::Opcode<CSA::SREDSUBF64> sredsubf64{};
constexpr mirmatch::Opcode<CSA::SREDXOR16> sredxor16{};
constexpr mirmatch::Opcode<CSA::SREDXOR32> sredxor32{};
constexpr mirmatch::Opcode<CSA::SREDXOR64> sredxor64{};
constexpr mirmatch::Opcode<CSA::SREDXOR8> sredxor8{};
constexpr mirmatch::Opcode<CSA::SRL16> srl16{};
constexpr mirmatch::Opcode<CSA::SRL32> srl32{};
constexpr mirmatch::Opcode<CSA::SRL64> srl64{};
constexpr mirmatch::Opcode<CSA::SRL8> srl8{};
constexpr mirmatch::Opcode<CSA::SST1> sst1{};
constexpr mirmatch::Opcode<CSA::SST16> sst16{};
constexpr mirmatch::Opcode<CSA::SST16f> sst16f{};
constexpr mirmatch::Opcode<CSA::SST32> sst32{};
constexpr mirmatch::Opcode<CSA::SST32f> sst32f{};
constexpr mirmatch::Opcode<CSA::SST64> sst64{};
constexpr mirmatch::Opcode<CSA::SST64f> sst64f{};
constexpr mirmatch::Opcode<CSA::SST8> sst8{};
constexpr mirmatch::Opcode<CSA::ST1> st1{};
constexpr mirmatch::Opcode<CSA::ST16> st16{};
constexpr mirmatch::Opcode<CSA::ST16D> st16d{};
constexpr mirmatch::Opcode<CSA::ST16R> st16r{};
constexpr mirmatch::Opcode<CSA::ST16X> st16x{};
constexpr mirmatch::Opcode<CSA::ST16f> st16f{};
constexpr mirmatch::Opcode<CSA::ST16fD> st16fd{};
constexpr mirmatch::Opcode<CSA::ST16fR> st16fr{};
constexpr mirmatch::Opcode<CSA::ST16fX> st16fx{};
constexpr mirmatch::Opcode<CSA::ST1D> st1d{};
constexpr mirmatch::Opcode<CSA::ST1R> st1r{};
constexpr mirmatch::Opcode<CSA::ST1X> st1x{};
constexpr mirmatch::Opcode<CSA::ST32> st32{};
constexpr mirmatch::Opcode<CSA::ST32D> st32d{};
constexpr mirmatch::Opcode<CSA::ST32R> st32r{};
constexpr mirmatch::Opcode<CSA::ST32X> st32x{};
constexpr mirmatch::Opcode<CSA::ST32f> st32f{};
constexpr mirmatch::Opcode<CSA::ST32fD> st32fd{};
constexpr mirmatch::Opcode<CSA::ST32fR> st32fr{};
constexpr mirmatch::Opcode<CSA::ST32fX> st32fx{};
constexpr mirmatch::Opcode<CSA::ST64> st64{};
constexpr mirmatch::Opcode<CSA::ST64D> st64d{};
constexpr mirmatch::Opcode<CSA::ST64R> st64r{};
constexpr mirmatch::Opcode<CSA::ST64X> st64x{};
constexpr mirmatch::Opcode<CSA::ST64f> st64f{};
constexpr mirmatch::Opcode<CSA::ST64fD> st64fd{};
constexpr mirmatch::Opcode<CSA::ST64fR> st64fr{};
constexpr mirmatch::Opcode<CSA::ST64fX> st64fx{};
constexpr mirmatch::Opcode<CSA::ST8> st8{};
constexpr mirmatch::Opcode<CSA::ST8D> st8d{};
constexpr mirmatch::Opcode<CSA::ST8R> st8r{};
constexpr mirmatch::Opcode<CSA::ST8X> st8x{};
constexpr mirmatch::Opcode<CSA::STRIDE16> stride16{};
constexpr mirmatch::Opcode<CSA::STRIDE32> stride32{};
constexpr mirmatch::Opcode<CSA::STRIDE64> stride64{};
constexpr mirmatch::Opcode<CSA::STRIDE8> stride8{};
constexpr mirmatch::Opcode<CSA::SUB16> sub16{};
constexpr mirmatch::Opcode<CSA::SUB32> sub32{};
constexpr mirmatch::Opcode<CSA::SUB64> sub64{};
constexpr mirmatch::Opcode<CSA::SUB8> sub8{};
constexpr mirmatch::Opcode<CSA::SUBF16> subf16{};
constexpr mirmatch::Opcode<CSA::SUBF32> subf32{};
constexpr mirmatch::Opcode<CSA::SUBF64> subf64{};
constexpr mirmatch::Opcode<CSA::SWITCH0> switch0{};
constexpr mirmatch::Opcode<CSA::SWITCH1> switch1{};
constexpr mirmatch::Opcode<CSA::SWITCH16> switch16{};
constexpr mirmatch::Opcode<CSA::SWITCH32> switch32{};
constexpr mirmatch::Opcode<CSA::SWITCH64> switch64{};
constexpr mirmatch::Opcode<CSA::SWITCH8> switch8{};
constexpr mirmatch::Opcode<CSA::SWITCHANY0> switchany0{};
constexpr mirmatch::Opcode<CSA::SWITCHANY1> switchany1{};
constexpr mirmatch::Opcode<CSA::SWITCHANY16> switchany16{};
constexpr mirmatch::Opcode<CSA::SWITCHANY32> switchany32{};
constexpr mirmatch::Opcode<CSA::SWITCHANY64> switchany64{};
constexpr mirmatch::Opcode<CSA::SWITCHANY8> switchany8{};
constexpr mirmatch::Opcode<CSA::TANF16> tanf16{};
constexpr mirmatch::Opcode<CSA::TANF32> tanf32{};
constexpr mirmatch::Opcode<CSA::TANF64> tanf64{};
constexpr mirmatch::Opcode<CSA::TRUNCF32> truncf32{};
constexpr mirmatch::Opcode<CSA::TRUNCF64> truncf64{};
constexpr mirmatch::Opcode<CSA::UNIT> unit{};
constexpr mirmatch::Opcode<CSA::UNITA> unita{};
constexpr mirmatch::Opcode<CSA::UNITI> uniti{};
constexpr mirmatch::Opcode<CSA::XOR1> xor1{};
constexpr mirmatch::Opcode<CSA::XOR16> xor16{};
constexpr mirmatch::Opcode<CSA::XOR32> xor32{};
constexpr mirmatch::Opcode<CSA::XOR64> xor64{};
constexpr mirmatch::Opcode<CSA::XOR8> xor8{};
constexpr mirmatch::Opcode<CSA::XPHI> xphi{};

} // Close namespace CSAMatch
} // Close namespace llvm

#endif // INCLUDED_CSAMATCHER_DOT_H
