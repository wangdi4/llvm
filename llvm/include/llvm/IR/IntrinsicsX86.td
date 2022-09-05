//===- IntrinsicsX86.td - Defines X86 intrinsics -----------*- tablegen -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines all of the X86-specific intrinsics.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Interrupt traps
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_int : Intrinsic<[], [llvm_i8_ty], [ImmArg<ArgIndex<0>>]>;
}

//===----------------------------------------------------------------------===//
// SEH intrinsics for Windows
let TargetPrefix = "x86" in {
  def int_x86_seh_lsda : Intrinsic<[llvm_ptr_ty], [llvm_ptr_ty], [IntrNoMem]>;

  // Marks the EH registration node created in LLVM IR prior to code generation.
  def int_x86_seh_ehregnode : Intrinsic<[], [llvm_ptr_ty], []>;

  // Marks the EH guard slot node created in LLVM IR prior to code generation.
  def int_x86_seh_ehguard : Intrinsic<[], [llvm_ptr_ty], []>;
}

//===----------------------------------------------------------------------===//
// FLAGS.
let TargetPrefix = "x86" in {
  def int_x86_flags_read_u32 : ClangBuiltin<"__builtin_ia32_readeflags_u32">,
        Intrinsic<[llvm_i32_ty], [], []>;
  def int_x86_flags_read_u64 : ClangBuiltin<"__builtin_ia32_readeflags_u64">,
        Intrinsic<[llvm_i64_ty], [], []>;
  def int_x86_flags_write_u32 : ClangBuiltin<"__builtin_ia32_writeeflags_u32">,
        Intrinsic<[], [llvm_i32_ty], []>;
  def int_x86_flags_write_u64 : ClangBuiltin<"__builtin_ia32_writeeflags_u64">,
        Intrinsic<[], [llvm_i64_ty], []>;
}

//===----------------------------------------------------------------------===//
// Read Time Stamp Counter.
let TargetPrefix = "x86" in {
  def int_x86_rdtsc : ClangBuiltin<"__builtin_ia32_rdtsc">,
              Intrinsic<[llvm_i64_ty], [], []>;
  def int_x86_rdtscp :
              Intrinsic<[llvm_i64_ty, llvm_i32_ty], [], []>;
}

// Read Performance-Monitoring Counter.
let TargetPrefix = "x86" in {
  def int_x86_rdpmc : ClangBuiltin<"__builtin_ia32_rdpmc">,
              Intrinsic<[llvm_i64_ty], [llvm_i32_ty], []>;
}

// Read processor ID.
let TargetPrefix = "x86" in {
  def int_x86_rdpid : ClangBuiltin<"__builtin_ia32_rdpid">,
              Intrinsic<[llvm_i32_ty], [], []>;
}

#if INTEL_CUSTOMIZATION
// Read CPUID.
let TargetPrefix = "x86" in {
  def int_x86_cpuid
      : Intrinsic<[llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty],
                  [llvm_i32_ty, llvm_i32_ty], []>;
}

// Read intel cpu core type, cpuid.1AH.eax[31:24]. This intrinsic may lazily
// use cpuid to detect cpu core type and cache it into a global array in libirc
// to minimize cost of frequently inquiring cpu core type. In such case, this
// intrinsic may return 0 (not likely) due to cpu migration. You should check
// returned core type if it must be non-zero on such targets. Read comments in
// Intel_X86PreISelIntrinsicLowering.cpp:lowerFastCpuIdCoreType to know which
// target it's capable to cache core type.
let TargetPrefix = "x86" in {
  def int_x86_intel_fast_cpuid_coretype : Intrinsic<[llvm_i8_ty], [], []>;
}
#endif // INTEL_CUSTOMIZATION

// Lock bit test.
let TargetPrefix = "x86" in {
  def int_x86_atomic_bts : Intrinsic<[llvm_anyint_ty], [llvm_ptr_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<1>>]>;
  def int_x86_atomic_btc : Intrinsic<[llvm_anyint_ty], [llvm_ptr_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<1>>]>;
  def int_x86_atomic_btr : Intrinsic<[llvm_anyint_ty], [llvm_ptr_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<1>>]>;
}

// Read Processor Register.
let TargetPrefix = "x86" in {
  def int_x86_rdpru : ClangBuiltin<"__builtin_ia32_rdpru">,
              Intrinsic<[llvm_i64_ty], [llvm_i32_ty], []>;
}

//===----------------------------------------------------------------------===//
// CET SS
let TargetPrefix = "x86" in {
  def int_x86_incsspd : ClangBuiltin<"__builtin_ia32_incsspd">,
              Intrinsic<[], [llvm_i32_ty], []>;
  def int_x86_incsspq : ClangBuiltin<"__builtin_ia32_incsspq">,
              Intrinsic<[], [llvm_i64_ty], []>;
  def int_x86_rdsspd : ClangBuiltin<"__builtin_ia32_rdsspd">,
              Intrinsic<[llvm_i32_ty], [llvm_i32_ty], []>;
  def int_x86_rdsspq : ClangBuiltin<"__builtin_ia32_rdsspq">,
              Intrinsic<[llvm_i64_ty], [llvm_i64_ty], []>;
  def int_x86_saveprevssp : ClangBuiltin<"__builtin_ia32_saveprevssp">,
              Intrinsic<[], [], []>;
  def int_x86_rstorssp : ClangBuiltin<"__builtin_ia32_rstorssp">,
              Intrinsic<[], [llvm_ptr_ty], []>;
  def int_x86_wrssd : ClangBuiltin<"__builtin_ia32_wrssd">,
              Intrinsic<[], [llvm_i32_ty, llvm_ptr_ty], []>;
  def int_x86_wrssq : ClangBuiltin<"__builtin_ia32_wrssq">,
              Intrinsic<[], [llvm_i64_ty, llvm_ptr_ty], []>;
  def int_x86_wrussd : ClangBuiltin<"__builtin_ia32_wrussd">,
              Intrinsic<[], [llvm_i32_ty, llvm_ptr_ty], []>;
  def int_x86_wrussq : ClangBuiltin<"__builtin_ia32_wrussq">,
              Intrinsic<[], [llvm_i64_ty, llvm_ptr_ty], []>;
  def int_x86_setssbsy : ClangBuiltin<"__builtin_ia32_setssbsy">,
              Intrinsic<[], [], []>;
  def int_x86_clrssbsy : ClangBuiltin<"__builtin_ia32_clrssbsy">,
              Intrinsic<[], [llvm_ptr_ty], []>;
}

//===----------------------------------------------------------------------===//
// 3DNow!

let TargetPrefix = "x86" in {
  def int_x86_3dnow_pavgusb : ClangBuiltin<"__builtin_ia32_pavgusb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pf2id : ClangBuiltin<"__builtin_ia32_pf2id">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_3dnow_pfacc : ClangBuiltin<"__builtin_ia32_pfacc">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pfadd : ClangBuiltin<"__builtin_ia32_pfadd">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pfcmpeq : ClangBuiltin<"__builtin_ia32_pfcmpeq">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pfcmpge : ClangBuiltin<"__builtin_ia32_pfcmpge">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pfcmpgt : ClangBuiltin<"__builtin_ia32_pfcmpgt">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pfmax : ClangBuiltin<"__builtin_ia32_pfmax">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pfmin : ClangBuiltin<"__builtin_ia32_pfmin">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pfmul : ClangBuiltin<"__builtin_ia32_pfmul">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pfrcp : ClangBuiltin<"__builtin_ia32_pfrcp">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_3dnow_pfrcpit1 : ClangBuiltin<"__builtin_ia32_pfrcpit1">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pfrcpit2 : ClangBuiltin<"__builtin_ia32_pfrcpit2">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pfrsqrt : ClangBuiltin<"__builtin_ia32_pfrsqrt">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_3dnow_pfrsqit1 : ClangBuiltin<"__builtin_ia32_pfrsqit1">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pfsub : ClangBuiltin<"__builtin_ia32_pfsub">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pfsubr : ClangBuiltin<"__builtin_ia32_pfsubr">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnow_pi2fd : ClangBuiltin<"__builtin_ia32_pi2fd">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_3dnow_pmulhrw : ClangBuiltin<"__builtin_ia32_pmulhrw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
}

//===----------------------------------------------------------------------===//
// 3DNow! extensions

let TargetPrefix = "x86" in {
  def int_x86_3dnowa_pf2iw : ClangBuiltin<"__builtin_ia32_pf2iw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_3dnowa_pfnacc : ClangBuiltin<"__builtin_ia32_pfnacc">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnowa_pfpnacc : ClangBuiltin<"__builtin_ia32_pfpnacc">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_3dnowa_pi2fw : ClangBuiltin<"__builtin_ia32_pi2fw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_3dnowa_pswapd :
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty], [IntrNoMem]>;
}

//===----------------------------------------------------------------------===//
// SSE1

// Arithmetic ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse_rcp_ss : ClangBuiltin<"__builtin_ia32_rcpss">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty],
                        [IntrNoMem]>;
  def int_x86_sse_rcp_ps : ClangBuiltin<"__builtin_ia32_rcpps">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty],
                        [IntrNoMem]>;
  def int_x86_sse_rsqrt_ss : ClangBuiltin<"__builtin_ia32_rsqrtss">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty],
                        [IntrNoMem]>;
  def int_x86_sse_rsqrt_ps : ClangBuiltin<"__builtin_ia32_rsqrtps">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty],
                        [IntrNoMem]>;
  def int_x86_sse_min_ss : ClangBuiltin<"__builtin_ia32_minss">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_min_ps : ClangBuiltin<"__builtin_ia32_minps">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_max_ss : ClangBuiltin<"__builtin_ia32_maxss">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_max_ps : ClangBuiltin<"__builtin_ia32_maxps">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
}

// Comparison ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse_cmp_ss : ClangBuiltin<"__builtin_ia32_cmpss">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty, llvm_i8_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  // NOTE: This comparison intrinsic is not used by clang as long as the
  //       distinction in signaling behaviour is not implemented.
  def int_x86_sse_cmp_ps :
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty, llvm_i8_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_sse_comieq_ss : ClangBuiltin<"__builtin_ia32_comieq">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_comilt_ss : ClangBuiltin<"__builtin_ia32_comilt">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_comile_ss : ClangBuiltin<"__builtin_ia32_comile">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_comigt_ss : ClangBuiltin<"__builtin_ia32_comigt">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_comige_ss : ClangBuiltin<"__builtin_ia32_comige">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_comineq_ss : ClangBuiltin<"__builtin_ia32_comineq">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_ucomieq_ss : ClangBuiltin<"__builtin_ia32_ucomieq">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_ucomilt_ss : ClangBuiltin<"__builtin_ia32_ucomilt">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_ucomile_ss : ClangBuiltin<"__builtin_ia32_ucomile">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_ucomigt_ss : ClangBuiltin<"__builtin_ia32_ucomigt">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_ucomige_ss : ClangBuiltin<"__builtin_ia32_ucomige">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_ucomineq_ss : ClangBuiltin<"__builtin_ia32_ucomineq">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
}


// Conversion ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse_cvtss2si : ClangBuiltin<"__builtin_ia32_cvtss2si">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_cvtss2si64 : ClangBuiltin<"__builtin_ia32_cvtss2si64">,
              Intrinsic<[llvm_i64_ty], [llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_cvttss2si : ClangBuiltin<"__builtin_ia32_cvttss2si">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_cvttss2si64 : ClangBuiltin<"__builtin_ia32_cvttss2si64">,
              Intrinsic<[llvm_i64_ty], [llvm_v4f32_ty], [IntrNoMem]>;

  def int_x86_sse_cvtps2pi : ClangBuiltin<"__builtin_ia32_cvtps2pi">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_cvttps2pi: ClangBuiltin<"__builtin_ia32_cvttps2pi">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse_cvtpi2ps : ClangBuiltin<"__builtin_ia32_cvtpi2ps">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
}

// Cacheability support ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse_sfence : ClangBuiltin<"__builtin_ia32_sfence">,
              Intrinsic<[], [], []>;
}

// Control register.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse_stmxcsr :
              Intrinsic<[], [llvm_ptr_ty],
                         [IntrWriteMem, IntrArgMemOnly,
                         // This prevents reordering with ldmxcsr
                         IntrHasSideEffects]>;
  def int_x86_sse_ldmxcsr :
              Intrinsic<[], [llvm_ptr_ty],
                         // FIXME: LDMXCSR does not actually write to memory,
                         // but intrinsic properties are generated incorrectly
                         // for IntrReadMem+IntrHasSideEffects.
                        [/*IntrReadMem, IntrArgMemOnly,*/ IntrHasSideEffects]>;
}

// Misc.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse_movmsk_ps : ClangBuiltin<"__builtin_ia32_movmskps">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty], [IntrNoMem]>;
}

//===----------------------------------------------------------------------===//
// SSE2

// FP arithmetic ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse2_min_sd : ClangBuiltin<"__builtin_ia32_minsd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_min_pd : ClangBuiltin<"__builtin_ia32_minpd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_max_sd : ClangBuiltin<"__builtin_ia32_maxsd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_max_pd : ClangBuiltin<"__builtin_ia32_maxpd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
}

// FP comparison ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse2_cmp_sd : ClangBuiltin<"__builtin_ia32_cmpsd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty, llvm_i8_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  // NOTE: This comparison intrinsic is not used by clang as long as the
  //       distinction in signaling behaviour is not implemented.
  def int_x86_sse2_cmp_pd :
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty, llvm_i8_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_sse2_comieq_sd : ClangBuiltin<"__builtin_ia32_comisdeq">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_comilt_sd : ClangBuiltin<"__builtin_ia32_comisdlt">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_comile_sd : ClangBuiltin<"__builtin_ia32_comisdle">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_comigt_sd : ClangBuiltin<"__builtin_ia32_comisdgt">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_comige_sd : ClangBuiltin<"__builtin_ia32_comisdge">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_comineq_sd : ClangBuiltin<"__builtin_ia32_comisdneq">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_ucomieq_sd : ClangBuiltin<"__builtin_ia32_ucomisdeq">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_ucomilt_sd : ClangBuiltin<"__builtin_ia32_ucomisdlt">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_ucomile_sd : ClangBuiltin<"__builtin_ia32_ucomisdle">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_ucomigt_sd : ClangBuiltin<"__builtin_ia32_ucomisdgt">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_ucomige_sd : ClangBuiltin<"__builtin_ia32_ucomisdge">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_ucomineq_sd : ClangBuiltin<"__builtin_ia32_ucomisdneq">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
}

// Integer arithmetic ops.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse2_pmulhu_w : ClangBuiltin<"__builtin_ia32_pmulhuw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem, Commutative]>;
  def int_x86_sse2_pmulh_w : ClangBuiltin<"__builtin_ia32_pmulhw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem, Commutative]>;
  def int_x86_sse2_pmadd_wd : ClangBuiltin<"__builtin_ia32_pmaddwd128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem, Commutative]>;
  def int_x86_sse2_pavg_b : ClangBuiltin<"__builtin_ia32_pavgb128">,
              Intrinsic<[llvm_v16i8_ty], [llvm_v16i8_ty,
                         llvm_v16i8_ty], [IntrNoMem, Commutative]>;
  def int_x86_sse2_pavg_w : ClangBuiltin<"__builtin_ia32_pavgw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem, Commutative]>;
  def int_x86_sse2_psad_bw : ClangBuiltin<"__builtin_ia32_psadbw128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v16i8_ty,
                         llvm_v16i8_ty], [IntrNoMem, Commutative]>;
}

// Integer shift ops.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse2_psll_w : ClangBuiltin<"__builtin_ia32_psllw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_sse2_psll_d : ClangBuiltin<"__builtin_ia32_pslld128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_sse2_psll_q : ClangBuiltin<"__builtin_ia32_psllq128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_sse2_psrl_w : ClangBuiltin<"__builtin_ia32_psrlw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_sse2_psrl_d : ClangBuiltin<"__builtin_ia32_psrld128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_sse2_psrl_q : ClangBuiltin<"__builtin_ia32_psrlq128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_sse2_psra_w : ClangBuiltin<"__builtin_ia32_psraw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_sse2_psra_d : ClangBuiltin<"__builtin_ia32_psrad128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;

  // Oddly these don't require an immediate due to a gcc compatibility issue.
  def int_x86_sse2_pslli_w : ClangBuiltin<"__builtin_ia32_psllwi128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_sse2_pslli_d : ClangBuiltin<"__builtin_ia32_pslldi128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_sse2_pslli_q : ClangBuiltin<"__builtin_ia32_psllqi128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_sse2_psrli_w : ClangBuiltin<"__builtin_ia32_psrlwi128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_sse2_psrli_d : ClangBuiltin<"__builtin_ia32_psrldi128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_sse2_psrli_q : ClangBuiltin<"__builtin_ia32_psrlqi128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_sse2_psrai_w : ClangBuiltin<"__builtin_ia32_psrawi128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_sse2_psrai_d : ClangBuiltin<"__builtin_ia32_psradi128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty,
                         llvm_i32_ty], [IntrNoMem]>;
}

// Conversion ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse2_cvtpd2dq : ClangBuiltin<"__builtin_ia32_cvtpd2dq">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_cvttpd2dq : ClangBuiltin<"__builtin_ia32_cvttpd2dq">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_cvtpd2ps : ClangBuiltin<"__builtin_ia32_cvtpd2ps">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_cvtps2dq : ClangBuiltin<"__builtin_ia32_cvtps2dq">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse2_cvttps2dq : ClangBuiltin<"__builtin_ia32_cvttps2dq">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse2_cvtsd2si : ClangBuiltin<"__builtin_ia32_cvtsd2si">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_cvtsd2si64 : ClangBuiltin<"__builtin_ia32_cvtsd2si64">,
              Intrinsic<[llvm_i64_ty], [llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_cvttsd2si : ClangBuiltin<"__builtin_ia32_cvttsd2si">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_cvttsd2si64 : ClangBuiltin<"__builtin_ia32_cvttsd2si64">,
              Intrinsic<[llvm_i64_ty], [llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_cvtsd2ss : ClangBuiltin<"__builtin_ia32_cvtsd2ss">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse_cvtpd2pi : ClangBuiltin<"__builtin_ia32_cvtpd2pi">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse_cvttpd2pi: ClangBuiltin<"__builtin_ia32_cvttpd2pi">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse_cvtpi2pd : ClangBuiltin<"__builtin_ia32_cvtpi2pd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_x86mmx_ty], [IntrNoMem]>;
}

// Misc.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse2_packsswb_128 : ClangBuiltin<"__builtin_ia32_packsswb128">,
              Intrinsic<[llvm_v16i8_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_sse2_packssdw_128 : ClangBuiltin<"__builtin_ia32_packssdw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v4i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_sse2_packuswb_128 : ClangBuiltin<"__builtin_ia32_packuswb128">,
              Intrinsic<[llvm_v16i8_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_sse2_movmsk_pd : ClangBuiltin<"__builtin_ia32_movmskpd">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse2_pmovmskb_128 : ClangBuiltin<"__builtin_ia32_pmovmskb128">,
              Intrinsic<[llvm_i32_ty], [llvm_v16i8_ty], [IntrNoMem]>;
  def int_x86_sse2_maskmov_dqu : ClangBuiltin<"__builtin_ia32_maskmovdqu">,
              Intrinsic<[], [llvm_v16i8_ty,
                         llvm_v16i8_ty, llvm_ptr_ty], []>;
  def int_x86_sse2_clflush : ClangBuiltin<"__builtin_ia32_clflush">,
              Intrinsic<[], [llvm_ptr_ty], []>;
  def int_x86_sse2_lfence : ClangBuiltin<"__builtin_ia32_lfence">,
              Intrinsic<[], [], []>;
  def int_x86_sse2_mfence : ClangBuiltin<"__builtin_ia32_mfence">,
              Intrinsic<[], [], []>;
  def int_x86_sse2_pause : ClangBuiltin<"__builtin_ia32_pause">,
              Intrinsic<[], [], []>;
}

//===----------------------------------------------------------------------===//
// SSE3

// Addition / subtraction ops.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse3_addsub_ps : ClangBuiltin<"__builtin_ia32_addsubps">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse3_addsub_pd : ClangBuiltin<"__builtin_ia32_addsubpd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
}

// Horizontal ops.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse3_hadd_ps : ClangBuiltin<"__builtin_ia32_haddps">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse3_hadd_pd : ClangBuiltin<"__builtin_ia32_haddpd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_sse3_hsub_ps : ClangBuiltin<"__builtin_ia32_hsubps">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_sse3_hsub_pd : ClangBuiltin<"__builtin_ia32_hsubpd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty], [IntrNoMem]>;
}

// Specialized unaligned load.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse3_ldu_dq : ClangBuiltin<"__builtin_ia32_lddqu">,
              Intrinsic<[llvm_v16i8_ty], [llvm_ptr_ty], [IntrReadMem]>;
}

// Thread synchronization ops.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse3_monitor : ClangBuiltin<"__builtin_ia32_monitor">,
              Intrinsic<[], [llvm_ptr_ty,
                         llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_sse3_mwait : ClangBuiltin<"__builtin_ia32_mwait">,
              Intrinsic<[], [llvm_i32_ty,
                         llvm_i32_ty], []>;
}

//===----------------------------------------------------------------------===//
// SSSE3

// Horizontal arithmetic ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_ssse3_phadd_w         : ClangBuiltin<"__builtin_ia32_phaddw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_ssse3_phadd_w_128     : ClangBuiltin<"__builtin_ia32_phaddw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;

  def int_x86_ssse3_phadd_d         : ClangBuiltin<"__builtin_ia32_phaddd">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_ssse3_phadd_d_128     : ClangBuiltin<"__builtin_ia32_phaddd128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;

  def int_x86_ssse3_phadd_sw        : ClangBuiltin<"__builtin_ia32_phaddsw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_ssse3_phadd_sw_128    : ClangBuiltin<"__builtin_ia32_phaddsw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;

  def int_x86_ssse3_phsub_w         : ClangBuiltin<"__builtin_ia32_phsubw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_ssse3_phsub_w_128     : ClangBuiltin<"__builtin_ia32_phsubw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;

  def int_x86_ssse3_phsub_d         : ClangBuiltin<"__builtin_ia32_phsubd">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_ssse3_phsub_d_128     : ClangBuiltin<"__builtin_ia32_phsubd128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;

  def int_x86_ssse3_phsub_sw        : ClangBuiltin<"__builtin_ia32_phsubsw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_ssse3_phsub_sw_128    : ClangBuiltin<"__builtin_ia32_phsubsw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;

  def int_x86_ssse3_pmadd_ub_sw     : ClangBuiltin<"__builtin_ia32_pmaddubsw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_ssse3_pmadd_ub_sw_128 : ClangBuiltin<"__builtin_ia32_pmaddubsw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v16i8_ty,
                         llvm_v16i8_ty], [IntrNoMem]>;
}

// Packed multiply high with round and scale
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_ssse3_pmul_hr_sw      : ClangBuiltin<"__builtin_ia32_pmulhrsw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;
  def int_x86_ssse3_pmul_hr_sw_128  : ClangBuiltin<"__builtin_ia32_pmulhrsw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem, Commutative]>;
}

// Shuffle ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_ssse3_pshuf_b         : ClangBuiltin<"__builtin_ia32_pshufb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_ssse3_pshuf_b_128     : ClangBuiltin<"__builtin_ia32_pshufb128">,
              Intrinsic<[llvm_v16i8_ty], [llvm_v16i8_ty,
                         llvm_v16i8_ty], [IntrNoMem]>;
  def int_x86_sse_pshuf_w           : ClangBuiltin<"__builtin_ia32_pshufw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_i8_ty],
                         [IntrNoMem, ImmArg<ArgIndex<1>>]>;
}

// Sign ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_ssse3_psign_b         : ClangBuiltin<"__builtin_ia32_psignb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_ssse3_psign_b_128     : ClangBuiltin<"__builtin_ia32_psignb128">,
              Intrinsic<[llvm_v16i8_ty], [llvm_v16i8_ty,
                         llvm_v16i8_ty], [IntrNoMem]>;

  def int_x86_ssse3_psign_w         : ClangBuiltin<"__builtin_ia32_psignw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_ssse3_psign_w_128     : ClangBuiltin<"__builtin_ia32_psignw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;

  def int_x86_ssse3_psign_d         : ClangBuiltin<"__builtin_ia32_psignd">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_ssse3_psign_d_128     : ClangBuiltin<"__builtin_ia32_psignd128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
}

// Absolute value ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_ssse3_pabs_b     : ClangBuiltin<"__builtin_ia32_pabsb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty], [IntrNoMem]>;

  def int_x86_ssse3_pabs_w     : ClangBuiltin<"__builtin_ia32_pabsw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty], [IntrNoMem]>;

  def int_x86_ssse3_pabs_d     : ClangBuiltin<"__builtin_ia32_pabsd">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty], [IntrNoMem]>;
}

//===----------------------------------------------------------------------===//
// SSE4.1

// FP rounding ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse41_round_ss        : ClangBuiltin<"__builtin_ia32_roundss">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                         llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_sse41_round_ps        : ClangBuiltin<"__builtin_ia32_roundps">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_sse41_round_sd        : ClangBuiltin<"__builtin_ia32_roundsd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                         llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_sse41_round_pd        : ClangBuiltin<"__builtin_ia32_roundpd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty,
                         llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<1>>]>;
}

// Vector min element
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse41_phminposuw     : ClangBuiltin<"__builtin_ia32_phminposuw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty],
                        [IntrNoMem]>;
}

// Advanced Encryption Standard (AES) Instructions
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_aesni_aesimc          : ClangBuiltin<"__builtin_ia32_aesimc128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty],
                        [IntrNoMem]>;

  def int_x86_aesni_aesenc          : ClangBuiltin<"__builtin_ia32_aesenc128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                        [IntrNoMem]>;
  def int_x86_aesni_aesenc_256      : ClangBuiltin<"__builtin_ia32_aesenc256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty],
                        [IntrNoMem]>;
  def int_x86_aesni_aesenc_512      : ClangBuiltin<"__builtin_ia32_aesenc512">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_v8i64_ty],
                        [IntrNoMem]>;

  def int_x86_aesni_aesenclast : ClangBuiltin<"__builtin_ia32_aesenclast128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                        [IntrNoMem]>;
  def int_x86_aesni_aesenclast_256 :
    ClangBuiltin<"__builtin_ia32_aesenclast256">,
    Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty],
              [IntrNoMem]>;
  def int_x86_aesni_aesenclast_512 :
    ClangBuiltin<"__builtin_ia32_aesenclast512">,
    Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_v8i64_ty],
              [IntrNoMem]>;

  def int_x86_aesni_aesdec          : ClangBuiltin<"__builtin_ia32_aesdec128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                        [IntrNoMem]>;
  def int_x86_aesni_aesdec_256      : ClangBuiltin<"__builtin_ia32_aesdec256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty],
                        [IntrNoMem]>;
  def int_x86_aesni_aesdec_512      : ClangBuiltin<"__builtin_ia32_aesdec512">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_v8i64_ty],
                        [IntrNoMem]>;

  def int_x86_aesni_aesdeclast : ClangBuiltin<"__builtin_ia32_aesdeclast128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                        [IntrNoMem]>;
  def int_x86_aesni_aesdeclast_256 :
    ClangBuiltin<"__builtin_ia32_aesdeclast256">,
    Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty],
              [IntrNoMem]>;
  def int_x86_aesni_aesdeclast_512 :
    ClangBuiltin<"__builtin_ia32_aesdeclast512">,
    Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_v8i64_ty],
              [IntrNoMem]>;

  def int_x86_aesni_aeskeygenassist :
              ClangBuiltin<"__builtin_ia32_aeskeygenassist128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_i8_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
}

// PCLMUL instructions
let TargetPrefix = "x86" in { // All intrinsics start with "llvm.x86.".
  def int_x86_pclmulqdq : ClangBuiltin<"__builtin_ia32_pclmulqdq128">,
          Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty, llvm_i8_ty],
                    [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_pclmulqdq_256 : ClangBuiltin<"__builtin_ia32_pclmulqdq256">,
          Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty, llvm_i8_ty],
                    [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_pclmulqdq_512 : ClangBuiltin<"__builtin_ia32_pclmulqdq512">,
          Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_v8i64_ty, llvm_i8_ty],
                    [IntrNoMem, ImmArg<ArgIndex<2>>]>;
}

// Vector pack
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse41_packusdw        : ClangBuiltin<"__builtin_ia32_packusdw128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                        [IntrNoMem]>;
}

// Vector insert
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse41_insertps       : ClangBuiltin<"__builtin_ia32_insertps128">,
          Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty],
                    [IntrNoMem, ImmArg<ArgIndex<2>>]>;
}

// Vector blend
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse41_pblendvb         : ClangBuiltin<"__builtin_ia32_pblendvb128">,
        Intrinsic<[llvm_v16i8_ty], [llvm_v16i8_ty, llvm_v16i8_ty,llvm_v16i8_ty],
                  [IntrNoMem]>;
  def int_x86_sse41_blendvpd         : ClangBuiltin<"__builtin_ia32_blendvpd">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,llvm_v2f64_ty],
                  [IntrNoMem]>;
  def int_x86_sse41_blendvps         : ClangBuiltin<"__builtin_ia32_blendvps">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,llvm_v4f32_ty],
                  [IntrNoMem]>;
}

// Vector dot product
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse41_dppd            : ClangBuiltin<"__builtin_ia32_dppd">,
          Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty, llvm_i8_ty],
                    [IntrNoMem, Commutative, ImmArg<ArgIndex<2>>]>;
  def int_x86_sse41_dpps            : ClangBuiltin<"__builtin_ia32_dpps">,
          Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty],
                    [IntrNoMem, Commutative, ImmArg<ArgIndex<2>>]>;
}

// Vector sum of absolute differences
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse41_mpsadbw         : ClangBuiltin<"__builtin_ia32_mpsadbw128">,
          Intrinsic<[llvm_v8i16_ty], [llvm_v16i8_ty, llvm_v16i8_ty,llvm_i8_ty],
                    [IntrNoMem, ImmArg<ArgIndex<2>>]>;
}

// Test instruction with bitwise comparison.
let TargetPrefix = "x86" in { // All intrinsics start with "llvm.x86.".
  def int_x86_sse41_ptestz          : ClangBuiltin<"__builtin_ia32_ptestz128">,
          Intrinsic<[llvm_i32_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                    [IntrNoMem]>;
  def int_x86_sse41_ptestc          : ClangBuiltin<"__builtin_ia32_ptestc128">,
          Intrinsic<[llvm_i32_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                    [IntrNoMem]>;
  def int_x86_sse41_ptestnzc        : ClangBuiltin<"__builtin_ia32_ptestnzc128">,
          Intrinsic<[llvm_i32_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                    [IntrNoMem]>;
}

//===----------------------------------------------------------------------===//
// SSE4.2

// Miscellaneous
// CRC Instruction
let TargetPrefix = "x86" in { // All intrinsics start with "llvm.x86.".
  def int_x86_sse42_crc32_32_8       : ClangBuiltin<"__builtin_ia32_crc32qi">,
          Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_sse42_crc32_32_16      : ClangBuiltin<"__builtin_ia32_crc32hi">,
          Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i16_ty],
                    [IntrNoMem]>;
  def int_x86_sse42_crc32_32_32      : ClangBuiltin<"__builtin_ia32_crc32si">,
          Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty],
                    [IntrNoMem]>;
  def int_x86_sse42_crc32_64_64      : ClangBuiltin<"__builtin_ia32_crc32di">,
          Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty],
                    [IntrNoMem]>;
}

// String/text processing ops.
let TargetPrefix = "x86" in { // All intrinsics start with "llvm.x86.".
  def int_x86_sse42_pcmpistrm128  : ClangBuiltin<"__builtin_ia32_pcmpistrm128">,
    Intrinsic<[llvm_v16i8_ty],
        [llvm_v16i8_ty, llvm_v16i8_ty, llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_sse42_pcmpistri128  : ClangBuiltin<"__builtin_ia32_pcmpistri128">,
    Intrinsic<[llvm_i32_ty],
        [llvm_v16i8_ty, llvm_v16i8_ty, llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_sse42_pcmpistria128 : ClangBuiltin<"__builtin_ia32_pcmpistria128">,
    Intrinsic<[llvm_i32_ty],
        [llvm_v16i8_ty, llvm_v16i8_ty, llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_sse42_pcmpistric128 : ClangBuiltin<"__builtin_ia32_pcmpistric128">,
    Intrinsic<[llvm_i32_ty],
        [llvm_v16i8_ty, llvm_v16i8_ty, llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_sse42_pcmpistrio128 : ClangBuiltin<"__builtin_ia32_pcmpistrio128">,
    Intrinsic<[llvm_i32_ty],
        [llvm_v16i8_ty, llvm_v16i8_ty, llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_sse42_pcmpistris128 : ClangBuiltin<"__builtin_ia32_pcmpistris128">,
    Intrinsic<[llvm_i32_ty],
        [llvm_v16i8_ty, llvm_v16i8_ty, llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_sse42_pcmpistriz128 : ClangBuiltin<"__builtin_ia32_pcmpistriz128">,
    Intrinsic<[llvm_i32_ty],
        [llvm_v16i8_ty, llvm_v16i8_ty, llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_sse42_pcmpestrm128  : ClangBuiltin<"__builtin_ia32_pcmpestrm128">,
    Intrinsic<[llvm_v16i8_ty],
        [llvm_v16i8_ty, llvm_i32_ty, llvm_v16i8_ty, llvm_i32_ty,
         llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_sse42_pcmpestri128  : ClangBuiltin<"__builtin_ia32_pcmpestri128">,
    Intrinsic<[llvm_i32_ty],
        [llvm_v16i8_ty, llvm_i32_ty, llvm_v16i8_ty, llvm_i32_ty,
         llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_sse42_pcmpestria128 : ClangBuiltin<"__builtin_ia32_pcmpestria128">,
    Intrinsic<[llvm_i32_ty],
        [llvm_v16i8_ty, llvm_i32_ty, llvm_v16i8_ty, llvm_i32_ty,
         llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_sse42_pcmpestric128 : ClangBuiltin<"__builtin_ia32_pcmpestric128">,
    Intrinsic<[llvm_i32_ty],
        [llvm_v16i8_ty, llvm_i32_ty, llvm_v16i8_ty, llvm_i32_ty,
         llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_sse42_pcmpestrio128 : ClangBuiltin<"__builtin_ia32_pcmpestrio128">,
    Intrinsic<[llvm_i32_ty],
        [llvm_v16i8_ty, llvm_i32_ty, llvm_v16i8_ty, llvm_i32_ty,
         llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_sse42_pcmpestris128 : ClangBuiltin<"__builtin_ia32_pcmpestris128">,
    Intrinsic<[llvm_i32_ty],
        [llvm_v16i8_ty, llvm_i32_ty, llvm_v16i8_ty, llvm_i32_ty,
         llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_sse42_pcmpestriz128 : ClangBuiltin<"__builtin_ia32_pcmpestriz128">,
    Intrinsic<[llvm_i32_ty],
        [llvm_v16i8_ty, llvm_i32_ty, llvm_v16i8_ty, llvm_i32_ty,
         llvm_i8_ty],
        [IntrNoMem, ImmArg<ArgIndex<4>>]>;
}

//===----------------------------------------------------------------------===//
// SSE4A

let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_sse4a_extrqi : ClangBuiltin<"__builtin_ia32_extrqi">,
    Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_i8_ty, llvm_i8_ty],
              [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_sse4a_extrq  : ClangBuiltin<"__builtin_ia32_extrq">,
    Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v16i8_ty], [IntrNoMem]>;

  def int_x86_sse4a_insertqi : ClangBuiltin<"__builtin_ia32_insertqi">,
    Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty,
                                llvm_i8_ty, llvm_i8_ty],
              [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
  def int_x86_sse4a_insertq  : ClangBuiltin<"__builtin_ia32_insertq">,
    Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty], [IntrNoMem]>;
}

//===----------------------------------------------------------------------===//
// AVX

// Arithmetic ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx_addsub_pd_256 : ClangBuiltin<"__builtin_ia32_addsubpd256">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty,
                  llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_avx_addsub_ps_256 : ClangBuiltin<"__builtin_ia32_addsubps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty,
                  llvm_v8f32_ty], [IntrNoMem]>;
  def int_x86_avx_max_pd_256 : ClangBuiltin<"__builtin_ia32_maxpd256">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty,
                  llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_avx_max_ps_256 : ClangBuiltin<"__builtin_ia32_maxps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty,
                  llvm_v8f32_ty], [IntrNoMem]>;
  def int_x86_avx_min_pd_256 : ClangBuiltin<"__builtin_ia32_minpd256">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty,
                  llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_avx_min_ps_256 : ClangBuiltin<"__builtin_ia32_minps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty,
                  llvm_v8f32_ty], [IntrNoMem]>;

  def int_x86_avx_rsqrt_ps_256 : ClangBuiltin<"__builtin_ia32_rsqrtps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty], [IntrNoMem]>;

  def int_x86_avx_rcp_ps_256 : ClangBuiltin<"__builtin_ia32_rcpps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty], [IntrNoMem]>;

  def int_x86_avx_round_pd_256 : ClangBuiltin<"__builtin_ia32_roundpd256">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty,
                  llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx_round_ps_256 : ClangBuiltin<"__builtin_ia32_roundps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty,
                  llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<1>>]>;
}

// Horizontal ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx_hadd_pd_256 : ClangBuiltin<"__builtin_ia32_haddpd256">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty,
                  llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_avx_hsub_ps_256 : ClangBuiltin<"__builtin_ia32_hsubps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty,
                  llvm_v8f32_ty], [IntrNoMem]>;
  def int_x86_avx_hsub_pd_256 : ClangBuiltin<"__builtin_ia32_hsubpd256">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty,
                  llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_avx_hadd_ps_256 : ClangBuiltin<"__builtin_ia32_haddps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty,
                  llvm_v8f32_ty], [IntrNoMem]>;
}

// Vector permutation
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx_vpermilvar_pd : ClangBuiltin<"__builtin_ia32_vpermilvarpd">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty,
                  llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_avx_vpermilvar_ps : ClangBuiltin<"__builtin_ia32_vpermilvarps">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                  llvm_v4i32_ty], [IntrNoMem]>;

  def int_x86_avx_vpermilvar_pd_256 :
        ClangBuiltin<"__builtin_ia32_vpermilvarpd256">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4i64_ty], [IntrNoMem]>;
  def int_x86_avx_vpermilvar_ps_256 :
        ClangBuiltin<"__builtin_ia32_vpermilvarps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_v8i32_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_d_128 :
       ClangBuiltin<"__builtin_ia32_vpermi2vard128">,
       Intrinsic<[llvm_v4i32_ty],
                 [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_d_256 :
        ClangBuiltin<"__builtin_ia32_vpermi2vard256">,
        Intrinsic<[llvm_v8i32_ty],
                  [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_d_512 :
        ClangBuiltin<"__builtin_ia32_vpermi2vard512">,
        Intrinsic<[llvm_v16i32_ty],
                  [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_hi_128 :
        ClangBuiltin<"__builtin_ia32_vpermi2varhi128">,
        Intrinsic<[llvm_v8i16_ty],
                  [llvm_v8i16_ty, llvm_v8i16_ty, llvm_v8i16_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_hi_256 :
        ClangBuiltin<"__builtin_ia32_vpermi2varhi256">,
        Intrinsic<[llvm_v16i16_ty],
                  [llvm_v16i16_ty, llvm_v16i16_ty, llvm_v16i16_ty],
                  [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_hi_512 :
        ClangBuiltin<"__builtin_ia32_vpermi2varhi512">,
        Intrinsic<[llvm_v32i16_ty],
                  [llvm_v32i16_ty, llvm_v32i16_ty, llvm_v32i16_ty],
                  [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_pd_128 :
        ClangBuiltin<"__builtin_ia32_vpermi2varpd128">,
        Intrinsic<[llvm_v2f64_ty],
                  [llvm_v2f64_ty, llvm_v2i64_ty, llvm_v2f64_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_pd_256 :
        ClangBuiltin<"__builtin_ia32_vpermi2varpd256">,
        Intrinsic<[llvm_v4f64_ty],
                  [llvm_v4f64_ty, llvm_v4i64_ty, llvm_v4f64_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_pd_512 :
        ClangBuiltin<"__builtin_ia32_vpermi2varpd512">,
        Intrinsic<[llvm_v8f64_ty],
                  [llvm_v8f64_ty, llvm_v8i64_ty, llvm_v8f64_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_ps_128 :
        ClangBuiltin<"__builtin_ia32_vpermi2varps128">,
        Intrinsic<[llvm_v4f32_ty],
                  [llvm_v4f32_ty, llvm_v4i32_ty, llvm_v4f32_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_ps_256 :
        ClangBuiltin<"__builtin_ia32_vpermi2varps256">,
        Intrinsic<[llvm_v8f32_ty],
                  [llvm_v8f32_ty, llvm_v8i32_ty, llvm_v8f32_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_ps_512 :
        ClangBuiltin<"__builtin_ia32_vpermi2varps512">,
        Intrinsic<[llvm_v16f32_ty],
                  [llvm_v16f32_ty, llvm_v16i32_ty, llvm_v16f32_ty],
                  [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_q_128 :
        ClangBuiltin<"__builtin_ia32_vpermi2varq128">,
        Intrinsic<[llvm_v2i64_ty],
                  [llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_q_256 :
        ClangBuiltin<"__builtin_ia32_vpermi2varq256">,
        Intrinsic<[llvm_v4i64_ty],
                  [llvm_v4i64_ty, llvm_v4i64_ty, llvm_v4i64_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_q_512 :
        ClangBuiltin<"__builtin_ia32_vpermi2varq512">,
        Intrinsic<[llvm_v8i64_ty],
                  [llvm_v8i64_ty, llvm_v8i64_ty, llvm_v8i64_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_qi_128 :
        ClangBuiltin<"__builtin_ia32_vpermi2varqi128">,
        Intrinsic<[llvm_v16i8_ty],
                  [llvm_v16i8_ty, llvm_v16i8_ty, llvm_v16i8_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_qi_256 :
        ClangBuiltin<"__builtin_ia32_vpermi2varqi256">,
        Intrinsic<[llvm_v32i8_ty],
                  [llvm_v32i8_ty, llvm_v32i8_ty, llvm_v32i8_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermi2var_qi_512 :
        ClangBuiltin<"__builtin_ia32_vpermi2varqi512">,
        Intrinsic<[llvm_v64i8_ty],
                  [llvm_v64i8_ty, llvm_v64i8_ty, llvm_v64i8_ty], [IntrNoMem]>;

  def int_x86_avx512_vpermilvar_pd_512 :
        ClangBuiltin<"__builtin_ia32_vpermilvarpd512">,
          Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8i64_ty],
          [IntrNoMem]>;

  def int_x86_avx512_vpermilvar_ps_512 :
        ClangBuiltin<"__builtin_ia32_vpermilvarps512">,
          Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16i32_ty],
          [IntrNoMem]>;

  def int_x86_avx512_pshuf_b_512 :
        ClangBuiltin<"__builtin_ia32_pshufb512">,
          Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
          [IntrNoMem]>;

}

// GFNI Instructions
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_vgf2p8affineinvqb_128 :
         ClangBuiltin<"__builtin_ia32_vgf2p8affineinvqb_v16qi">,
          Intrinsic<[llvm_v16i8_ty],
          [llvm_v16i8_ty, llvm_v16i8_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_vgf2p8affineinvqb_256 :
         ClangBuiltin<"__builtin_ia32_vgf2p8affineinvqb_v32qi">,
          Intrinsic<[llvm_v32i8_ty],
          [llvm_v32i8_ty, llvm_v32i8_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_vgf2p8affineinvqb_512 :
         ClangBuiltin<"__builtin_ia32_vgf2p8affineinvqb_v64qi">,
          Intrinsic<[llvm_v64i8_ty],
          [llvm_v64i8_ty, llvm_v64i8_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<2>>]>;

  def int_x86_vgf2p8affineqb_128 :
         ClangBuiltin<"__builtin_ia32_vgf2p8affineqb_v16qi">,
          Intrinsic<[llvm_v16i8_ty],
          [llvm_v16i8_ty, llvm_v16i8_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_vgf2p8affineqb_256 :
         ClangBuiltin<"__builtin_ia32_vgf2p8affineqb_v32qi">,
          Intrinsic<[llvm_v32i8_ty],
          [llvm_v32i8_ty, llvm_v32i8_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_vgf2p8affineqb_512 :
         ClangBuiltin<"__builtin_ia32_vgf2p8affineqb_v64qi">,
          Intrinsic<[llvm_v64i8_ty],
          [llvm_v64i8_ty, llvm_v64i8_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<2>>]>;

  def int_x86_vgf2p8mulb_128     :
         ClangBuiltin<"__builtin_ia32_vgf2p8mulb_v16qi">,
          Intrinsic<[llvm_v16i8_ty],
          [llvm_v16i8_ty, llvm_v16i8_ty],
          [IntrNoMem]>;
  def int_x86_vgf2p8mulb_256     :
         ClangBuiltin<"__builtin_ia32_vgf2p8mulb_v32qi">,
          Intrinsic<[llvm_v32i8_ty],
          [llvm_v32i8_ty, llvm_v32i8_ty],
          [IntrNoMem]>;
  def int_x86_vgf2p8mulb_512     :
         ClangBuiltin<"__builtin_ia32_vgf2p8mulb_v64qi">,
          Intrinsic<[llvm_v64i8_ty],
          [llvm_v64i8_ty, llvm_v64i8_ty],
          [IntrNoMem]>;
}

// Vector blend
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx_blendv_pd_256 : ClangBuiltin<"__builtin_ia32_blendvpd256">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty,
                  llvm_v4f64_ty, llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_avx_blendv_ps_256 : ClangBuiltin<"__builtin_ia32_blendvps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty,
                  llvm_v8f32_ty, llvm_v8f32_ty], [IntrNoMem]>;
}

// Vector dot product
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx_dp_ps_256 : ClangBuiltin<"__builtin_ia32_dpps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty,
                  llvm_v8f32_ty, llvm_i8_ty],
                  [IntrNoMem, Commutative, ImmArg<ArgIndex<2>>]>;
}

// Vector compare
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx_cmp_pd_256 :
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty,
                  llvm_v4f64_ty, llvm_i8_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx_cmp_ps_256 :
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty,
                  llvm_v8f32_ty, llvm_i8_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
}

// Vector convert
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx_cvt_pd2_ps_256 : ClangBuiltin<"__builtin_ia32_cvtpd2ps256">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_avx_cvt_ps2dq_256 : ClangBuiltin<"__builtin_ia32_cvtps2dq256">,
        Intrinsic<[llvm_v8i32_ty], [llvm_v8f32_ty], [IntrNoMem]>;
  def int_x86_avx_cvtt_pd2dq_256 : ClangBuiltin<"__builtin_ia32_cvttpd2dq256">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_avx_cvt_pd2dq_256 : ClangBuiltin<"__builtin_ia32_cvtpd2dq256">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_avx_cvtt_ps2dq_256 : ClangBuiltin<"__builtin_ia32_cvttps2dq256">,
        Intrinsic<[llvm_v8i32_ty], [llvm_v8f32_ty], [IntrNoMem]>;
}

// Vector bit test
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx_vtestz_pd : ClangBuiltin<"__builtin_ia32_vtestzpd">,
        Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                  llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_avx_vtestc_pd : ClangBuiltin<"__builtin_ia32_vtestcpd">,
        Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                  llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_avx_vtestnzc_pd : ClangBuiltin<"__builtin_ia32_vtestnzcpd">,
        Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                  llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_avx_vtestz_ps : ClangBuiltin<"__builtin_ia32_vtestzps">,
        Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                  llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_avx_vtestc_ps : ClangBuiltin<"__builtin_ia32_vtestcps">,
        Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                  llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_avx_vtestnzc_ps : ClangBuiltin<"__builtin_ia32_vtestnzcps">,
        Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                  llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_avx_vtestz_pd_256 : ClangBuiltin<"__builtin_ia32_vtestzpd256">,
        Intrinsic<[llvm_i32_ty], [llvm_v4f64_ty,
                  llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_avx_vtestc_pd_256 : ClangBuiltin<"__builtin_ia32_vtestcpd256">,
        Intrinsic<[llvm_i32_ty], [llvm_v4f64_ty,
                  llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_avx_vtestnzc_pd_256 : ClangBuiltin<"__builtin_ia32_vtestnzcpd256">,
        Intrinsic<[llvm_i32_ty], [llvm_v4f64_ty,
                  llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_avx_vtestz_ps_256 : ClangBuiltin<"__builtin_ia32_vtestzps256">,
        Intrinsic<[llvm_i32_ty], [llvm_v8f32_ty,
                  llvm_v8f32_ty], [IntrNoMem]>;
  def int_x86_avx_vtestc_ps_256 : ClangBuiltin<"__builtin_ia32_vtestcps256">,
        Intrinsic<[llvm_i32_ty], [llvm_v8f32_ty,
                  llvm_v8f32_ty], [IntrNoMem]>;
  def int_x86_avx_vtestnzc_ps_256 : ClangBuiltin<"__builtin_ia32_vtestnzcps256">,
        Intrinsic<[llvm_i32_ty], [llvm_v8f32_ty,
                  llvm_v8f32_ty], [IntrNoMem]>;
  def int_x86_avx_ptestz_256 : ClangBuiltin<"__builtin_ia32_ptestz256">,
        Intrinsic<[llvm_i32_ty], [llvm_v4i64_ty,
                  llvm_v4i64_ty], [IntrNoMem]>;
  def int_x86_avx_ptestc_256 : ClangBuiltin<"__builtin_ia32_ptestc256">,
        Intrinsic<[llvm_i32_ty], [llvm_v4i64_ty,
                  llvm_v4i64_ty], [IntrNoMem]>;
  def int_x86_avx_ptestnzc_256 : ClangBuiltin<"__builtin_ia32_ptestnzc256">,
        Intrinsic<[llvm_i32_ty], [llvm_v4i64_ty,
                  llvm_v4i64_ty], [IntrNoMem]>;

  def int_x86_avx512_fpclass_pd_128 :
          Intrinsic<[llvm_v2i1_ty], [llvm_v2f64_ty, llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_fpclass_pd_256 :
          Intrinsic<[llvm_v4i1_ty], [llvm_v4f64_ty, llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_fpclass_pd_512 :
          Intrinsic<[llvm_v8i1_ty], [llvm_v8f64_ty, llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_fpclass_ps_128 :
          Intrinsic<[llvm_v4i1_ty], [llvm_v4f32_ty, llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_fpclass_ps_256 :
          Intrinsic<[llvm_v8i1_ty], [llvm_v8f32_ty, llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_fpclass_ps_512 :
          Intrinsic<[llvm_v16i1_ty], [llvm_v16f32_ty, llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_fpclass_sd :
         ClangBuiltin<"__builtin_ia32_fpclasssd_mask">,
          Intrinsic<[llvm_i8_ty], [llvm_v2f64_ty, llvm_i32_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_fpclass_ss :
         ClangBuiltin<"__builtin_ia32_fpclassss_mask">,
          Intrinsic<[llvm_i8_ty], [llvm_v4f32_ty, llvm_i32_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<1>>]>;
}

// Vector extract sign mask
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx_movmsk_pd_256 : ClangBuiltin<"__builtin_ia32_movmskpd256">,
        Intrinsic<[llvm_i32_ty], [llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_avx_movmsk_ps_256 : ClangBuiltin<"__builtin_ia32_movmskps256">,
        Intrinsic<[llvm_i32_ty], [llvm_v8f32_ty], [IntrNoMem]>;
}

// Vector zero
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx_vzeroall : ClangBuiltin<"__builtin_ia32_vzeroall">,
        Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects]>;
  def int_x86_avx_vzeroupper : ClangBuiltin<"__builtin_ia32_vzeroupper">,
        Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects]>;
}

// SIMD load ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx_ldu_dq_256 : ClangBuiltin<"__builtin_ia32_lddqu256">,
        Intrinsic<[llvm_v32i8_ty], [llvm_ptr_ty], [IntrReadMem]>;
}

// Conditional load ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx_maskload_pd : ClangBuiltin<"__builtin_ia32_maskloadpd">,
        Intrinsic<[llvm_v2f64_ty], [llvm_ptr_ty, llvm_v2i64_ty],
                  [IntrReadMem, IntrArgMemOnly]>;
  def int_x86_avx_maskload_ps : ClangBuiltin<"__builtin_ia32_maskloadps">,
        Intrinsic<[llvm_v4f32_ty], [llvm_ptr_ty, llvm_v4i32_ty],
                  [IntrReadMem, IntrArgMemOnly]>;
  def int_x86_avx_maskload_pd_256 : ClangBuiltin<"__builtin_ia32_maskloadpd256">,
        Intrinsic<[llvm_v4f64_ty], [llvm_ptr_ty, llvm_v4i64_ty],
                  [IntrReadMem, IntrArgMemOnly]>;
  def int_x86_avx_maskload_ps_256 : ClangBuiltin<"__builtin_ia32_maskloadps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_ptr_ty, llvm_v8i32_ty],
                  [IntrReadMem, IntrArgMemOnly]>;
}

// Conditional store ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx_maskstore_pd : ClangBuiltin<"__builtin_ia32_maskstorepd">,
        Intrinsic<[], [llvm_ptr_ty,
                  llvm_v2i64_ty, llvm_v2f64_ty], [IntrArgMemOnly]>;
  def int_x86_avx_maskstore_ps : ClangBuiltin<"__builtin_ia32_maskstoreps">,
        Intrinsic<[], [llvm_ptr_ty,
                  llvm_v4i32_ty, llvm_v4f32_ty], [IntrArgMemOnly]>;
  def int_x86_avx_maskstore_pd_256 :
        ClangBuiltin<"__builtin_ia32_maskstorepd256">,
        Intrinsic<[], [llvm_ptr_ty,
                  llvm_v4i64_ty, llvm_v4f64_ty], [IntrArgMemOnly]>;
  def int_x86_avx_maskstore_ps_256 :
        ClangBuiltin<"__builtin_ia32_maskstoreps256">,
        Intrinsic<[], [llvm_ptr_ty,
                  llvm_v8i32_ty, llvm_v8f32_ty], [IntrArgMemOnly]>;
}

// BITALG bits shuffle
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx512_vpshufbitqmb_128 :
    Intrinsic<[llvm_v16i1_ty], [llvm_v16i8_ty, llvm_v16i8_ty], [IntrNoMem]>;
  def int_x86_avx512_vpshufbitqmb_256 :
    Intrinsic<[llvm_v32i1_ty], [llvm_v32i8_ty, llvm_v32i8_ty], [IntrNoMem]>;
  def int_x86_avx512_vpshufbitqmb_512 :
    Intrinsic<[llvm_v64i1_ty], [llvm_v64i8_ty, llvm_v64i8_ty], [IntrNoMem]>;
}

//===----------------------------------------------------------------------===//
// AVX2

// Integer arithmetic ops.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx2_pmulhu_w : ClangBuiltin<"__builtin_ia32_pmulhuw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrNoMem, Commutative]>;
  def int_x86_avx2_pmulh_w : ClangBuiltin<"__builtin_ia32_pmulhw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrNoMem, Commutative]>;
  def int_x86_avx2_pmadd_wd : ClangBuiltin<"__builtin_ia32_pmaddwd256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrNoMem, Commutative]>;
  def int_x86_avx2_pavg_b : ClangBuiltin<"__builtin_ia32_pavgb256">,
              Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty,
                         llvm_v32i8_ty], [IntrNoMem, Commutative]>;
  def int_x86_avx2_pavg_w : ClangBuiltin<"__builtin_ia32_pavgw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrNoMem, Commutative]>;
  def int_x86_avx2_psad_bw : ClangBuiltin<"__builtin_ia32_psadbw256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v32i8_ty,
                         llvm_v32i8_ty], [IntrNoMem, Commutative]>;
}

// Integer shift ops.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx2_psll_w : ClangBuiltin<"__builtin_ia32_psllw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_avx2_psll_d : ClangBuiltin<"__builtin_ia32_pslld256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_avx2_psll_q : ClangBuiltin<"__builtin_ia32_psllq256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_avx2_psrl_w : ClangBuiltin<"__builtin_ia32_psrlw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_avx2_psrl_d : ClangBuiltin<"__builtin_ia32_psrld256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_avx2_psrl_q : ClangBuiltin<"__builtin_ia32_psrlq256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_avx2_psra_w : ClangBuiltin<"__builtin_ia32_psraw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_avx2_psra_d : ClangBuiltin<"__builtin_ia32_psrad256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;

  // Oddly these don't require an immediate due to a gcc compatibility issue.
  def int_x86_avx2_pslli_w : ClangBuiltin<"__builtin_ia32_psllwi256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx2_pslli_d : ClangBuiltin<"__builtin_ia32_pslldi256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx2_pslli_q : ClangBuiltin<"__builtin_ia32_psllqi256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx2_psrli_w : ClangBuiltin<"__builtin_ia32_psrlwi256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx2_psrli_d : ClangBuiltin<"__builtin_ia32_psrldi256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx2_psrli_q : ClangBuiltin<"__builtin_ia32_psrlqi256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx2_psrai_w : ClangBuiltin<"__builtin_ia32_psrawi256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx2_psrai_d : ClangBuiltin<"__builtin_ia32_psradi256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty,
                         llvm_i32_ty], [IntrNoMem]>;

  def int_x86_avx512_psra_q_128 : ClangBuiltin<"__builtin_ia32_psraq128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_avx512_psra_q_256 : ClangBuiltin<"__builtin_ia32_psraq256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;

  // Oddly these don't require an immediate due to a gcc compatibility issue.
  def int_x86_avx512_psrai_q_128 : ClangBuiltin<"__builtin_ia32_psraqi128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx512_psrai_q_256 : ClangBuiltin<"__builtin_ia32_psraqi256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty,
                         llvm_i32_ty], [IntrNoMem]>;

  def int_x86_avx512_psll_w_512 : ClangBuiltin<"__builtin_ia32_psllw512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_avx512_psll_d_512 : ClangBuiltin<"__builtin_ia32_pslld512">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_avx512_psll_q_512 : ClangBuiltin<"__builtin_ia32_psllq512">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_avx512_psrl_w_512 : ClangBuiltin<"__builtin_ia32_psrlw512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_avx512_psrl_d_512 : ClangBuiltin<"__builtin_ia32_psrld512">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_avx512_psrl_q_512 : ClangBuiltin<"__builtin_ia32_psrlq512">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_avx512_psra_w_512 : ClangBuiltin<"__builtin_ia32_psraw512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_avx512_psra_d_512 : ClangBuiltin<"__builtin_ia32_psrad512">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_avx512_psra_q_512 : ClangBuiltin<"__builtin_ia32_psraq512">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;

  // Oddly these don't require an immediate due to a gcc compatibility issue.
  def int_x86_avx512_pslli_w_512 : ClangBuiltin<"__builtin_ia32_psllwi512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx512_pslli_d_512 : ClangBuiltin<"__builtin_ia32_pslldi512">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx512_pslli_q_512 : ClangBuiltin<"__builtin_ia32_psllqi512">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx512_psrli_w_512 : ClangBuiltin<"__builtin_ia32_psrlwi512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx512_psrli_d_512 : ClangBuiltin<"__builtin_ia32_psrldi512">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx512_psrli_q_512 : ClangBuiltin<"__builtin_ia32_psrlqi512">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx512_psrai_w_512 : ClangBuiltin<"__builtin_ia32_psrawi512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx512_psrai_d_512 : ClangBuiltin<"__builtin_ia32_psradi512">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_avx512_psrai_q_512 : ClangBuiltin<"__builtin_ia32_psraqi512">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty,
                         llvm_i32_ty], [IntrNoMem]>;

  def int_x86_avx512_pmultishift_qb_128:
        ClangBuiltin<"__builtin_ia32_vpmultishiftqb128">,
        Intrinsic<[llvm_v16i8_ty], [llvm_v16i8_ty, llvm_v16i8_ty], [IntrNoMem]>;
  def int_x86_avx512_pmultishift_qb_256:
        ClangBuiltin<"__builtin_ia32_vpmultishiftqb256">,
        Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty], [IntrNoMem]>;
  def int_x86_avx512_pmultishift_qb_512:
        ClangBuiltin<"__builtin_ia32_vpmultishiftqb512">,
        Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty], [IntrNoMem]>;
}

// Pack ops.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx2_packsswb : ClangBuiltin<"__builtin_ia32_packsswb256">,
              Intrinsic<[llvm_v32i8_ty], [llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrNoMem]>;
  def int_x86_avx2_packssdw : ClangBuiltin<"__builtin_ia32_packssdw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v8i32_ty,
                         llvm_v8i32_ty], [IntrNoMem]>;
  def int_x86_avx2_packuswb : ClangBuiltin<"__builtin_ia32_packuswb256">,
              Intrinsic<[llvm_v32i8_ty], [llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrNoMem]>;
  def int_x86_avx2_packusdw : ClangBuiltin<"__builtin_ia32_packusdw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v8i32_ty,
                         llvm_v8i32_ty], [IntrNoMem]>;
}

// Horizontal arithmetic ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx2_phadd_w : ClangBuiltin<"__builtin_ia32_phaddw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrNoMem]>;
  def int_x86_avx2_phadd_d : ClangBuiltin<"__builtin_ia32_phaddd256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty,
                         llvm_v8i32_ty], [IntrNoMem]>;
  def int_x86_avx2_phadd_sw : ClangBuiltin<"__builtin_ia32_phaddsw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrNoMem]>;
  def int_x86_avx2_phsub_w : ClangBuiltin<"__builtin_ia32_phsubw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrNoMem]>;
  def int_x86_avx2_phsub_d : ClangBuiltin<"__builtin_ia32_phsubd256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty,
                         llvm_v8i32_ty], [IntrNoMem]>;
  def int_x86_avx2_phsub_sw : ClangBuiltin<"__builtin_ia32_phsubsw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrNoMem]>;
  def int_x86_avx2_pmadd_ub_sw : ClangBuiltin<"__builtin_ia32_pmaddubsw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v32i8_ty,
                         llvm_v32i8_ty], [IntrNoMem]>;
}

// Sign ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx2_psign_b : ClangBuiltin<"__builtin_ia32_psignb256">,
              Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty,
                         llvm_v32i8_ty], [IntrNoMem]>;
  def int_x86_avx2_psign_w : ClangBuiltin<"__builtin_ia32_psignw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrNoMem]>;
  def int_x86_avx2_psign_d : ClangBuiltin<"__builtin_ia32_psignd256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty,
                         llvm_v8i32_ty], [IntrNoMem]>;
}

// Packed multiply high with round and scale
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx2_pmul_hr_sw : ClangBuiltin<"__builtin_ia32_pmulhrsw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrNoMem, Commutative]>;
  def int_x86_avx512_pmul_hr_sw_512 : ClangBuiltin<"__builtin_ia32_pmulhrsw512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty,
                         llvm_v32i16_ty], [IntrNoMem, Commutative]>;
}

// Vector blend
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx2_pblendvb : ClangBuiltin<"__builtin_ia32_pblendvb256">,
              Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty, llvm_v32i8_ty,
                         llvm_v32i8_ty], [IntrNoMem]>;
}


// Vector permutation
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx2_permd : ClangBuiltin<"__builtin_ia32_permvarsi256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
                        [IntrNoMem]>;
  def int_x86_avx2_permps : ClangBuiltin<"__builtin_ia32_permvarsf256">,
              Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_v8i32_ty],
                        [IntrNoMem]>;
}

// Conditional load ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx2_maskload_d : ClangBuiltin<"__builtin_ia32_maskloadd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_v4i32_ty],
                  [IntrReadMem, IntrArgMemOnly]>;
  def int_x86_avx2_maskload_q : ClangBuiltin<"__builtin_ia32_maskloadq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_ptr_ty, llvm_v2i64_ty],
                  [IntrReadMem, IntrArgMemOnly]>;
  def int_x86_avx2_maskload_d_256 : ClangBuiltin<"__builtin_ia32_maskloadd256">,
        Intrinsic<[llvm_v8i32_ty], [llvm_ptr_ty, llvm_v8i32_ty],
                  [IntrReadMem, IntrArgMemOnly]>;
  def int_x86_avx2_maskload_q_256 : ClangBuiltin<"__builtin_ia32_maskloadq256">,
        Intrinsic<[llvm_v4i64_ty], [llvm_ptr_ty, llvm_v4i64_ty],
                  [IntrReadMem, IntrArgMemOnly]>;
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX_COMPRESS
  def int_x86_avx2_maskload_b : ClangBuiltin<"__builtin_ia32_maskloadb">,
        Intrinsic<[llvm_v16i8_ty], [llvm_ptr_ty, llvm_v16i8_ty],
                  [IntrReadMem, IntrArgMemOnly]>;
  def int_x86_avx2_maskload_w : ClangBuiltin<"__builtin_ia32_maskloadw">,
        Intrinsic<[llvm_v8i16_ty], [llvm_ptr_ty, llvm_v8i16_ty],
                  [IntrReadMem, IntrArgMemOnly]>;
  def int_x86_avx2_maskload_b_256 : ClangBuiltin<"__builtin_ia32_maskloadb256">,
        Intrinsic<[llvm_v32i8_ty], [llvm_ptr_ty, llvm_v32i8_ty],
                  [IntrReadMem, IntrArgMemOnly]>;
  def int_x86_avx2_maskload_w_256 : ClangBuiltin<"__builtin_ia32_maskloadw256">,
        Intrinsic<[llvm_v16i16_ty], [llvm_ptr_ty, llvm_v16i16_ty],
                  [IntrReadMem, IntrArgMemOnly]>;
#endif // INTEL_FEATURE_ISA_AVX_COMPRESS
#if INTEL_FEATURE_ISA_AVX512_MEDIAX
  def int_x86_avx512_mpsadbw_512 :
         ClangBuiltin<"__builtin_ia32_mpsadbw512">,
          Intrinsic<[llvm_v32i16_ty],
                    [llvm_v64i8_ty, llvm_v64i8_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<2>>]>;
#endif // INTEL_FEATURE_ISA_AVX512_MEDIAX
#endif // INTEL_CUSTOMIZATION
}

// Conditional store ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx2_maskstore_d : ClangBuiltin<"__builtin_ia32_maskstored">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrArgMemOnly]>;
  def int_x86_avx2_maskstore_q : ClangBuiltin<"__builtin_ia32_maskstoreq">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v2i64_ty, llvm_v2i64_ty],
                  [IntrArgMemOnly]>;
  def int_x86_avx2_maskstore_d_256 :
        ClangBuiltin<"__builtin_ia32_maskstored256">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i32_ty, llvm_v8i32_ty],
                  [IntrArgMemOnly]>;
  def int_x86_avx2_maskstore_q_256 :
        ClangBuiltin<"__builtin_ia32_maskstoreq256">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4i64_ty, llvm_v4i64_ty],
                  [IntrArgMemOnly]>;
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX_COMPRESS
  def int_x86_avx2_maskstore_b : ClangBuiltin<"__builtin_ia32_maskstoreb">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrArgMemOnly]>;
  def int_x86_avx2_maskstore_w : ClangBuiltin<"__builtin_ia32_maskstorew">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrArgMemOnly]>;
  def int_x86_avx2_maskstore_b_256 :
        ClangBuiltin<"__builtin_ia32_maskstoreb256">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v32i8_ty, llvm_v32i8_ty],
                  [IntrArgMemOnly]>;
  def int_x86_avx2_maskstore_w_256 :
        ClangBuiltin<"__builtin_ia32_maskstorew256">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16i16_ty, llvm_v16i16_ty],
                  [IntrArgMemOnly]>;
#endif // INTEL_FEATURE_ISA_AVX_COMPRESS
#endif // INTEL_CUSTOMIZATION
}

// Variable bit shift ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx2_psllv_d : ClangBuiltin<"__builtin_ia32_psllv4si">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                        [IntrNoMem]>;
  def int_x86_avx2_psllv_d_256 : ClangBuiltin<"__builtin_ia32_psllv8si">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
                        [IntrNoMem]>;
  def int_x86_avx2_psllv_q : ClangBuiltin<"__builtin_ia32_psllv2di">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                        [IntrNoMem]>;
  def int_x86_avx2_psllv_q_256 : ClangBuiltin<"__builtin_ia32_psllv4di">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty],
                        [IntrNoMem]>;

  def int_x86_avx512_psllv_d_512 : ClangBuiltin<"__builtin_ia32_psllv16si">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                        [IntrNoMem]>;
  def int_x86_avx512_psllv_q_512 : ClangBuiltin<"__builtin_ia32_psllv8di">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_v8i64_ty],
                        [IntrNoMem]>;

  def int_x86_avx2_psrlv_d : ClangBuiltin<"__builtin_ia32_psrlv4si">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                        [IntrNoMem]>;
  def int_x86_avx2_psrlv_d_256 : ClangBuiltin<"__builtin_ia32_psrlv8si">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
                        [IntrNoMem]>;
  def int_x86_avx2_psrlv_q : ClangBuiltin<"__builtin_ia32_psrlv2di">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                        [IntrNoMem]>;
  def int_x86_avx2_psrlv_q_256 : ClangBuiltin<"__builtin_ia32_psrlv4di">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty],
                        [IntrNoMem]>;

  def int_x86_avx512_psrlv_d_512 : ClangBuiltin<"__builtin_ia32_psrlv16si">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                        [IntrNoMem]>;
  def int_x86_avx512_psrlv_q_512 : ClangBuiltin<"__builtin_ia32_psrlv8di">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_v8i64_ty],
                        [IntrNoMem]>;

  def int_x86_avx2_psrav_d : ClangBuiltin<"__builtin_ia32_psrav4si">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                        [IntrNoMem]>;
  def int_x86_avx2_psrav_d_256 : ClangBuiltin<"__builtin_ia32_psrav8si">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
                        [IntrNoMem]>;

  def int_x86_avx512_psrav_d_512 : ClangBuiltin<"__builtin_ia32_psrav16si">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                        [IntrNoMem]>;
  def int_x86_avx512_psrav_q_128 : ClangBuiltin<"__builtin_ia32_psravq128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                        [IntrNoMem]>;
  def int_x86_avx512_psrav_q_256 : ClangBuiltin<"__builtin_ia32_psravq256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty],
                        [IntrNoMem]>;
  def int_x86_avx512_psrav_q_512 : ClangBuiltin<"__builtin_ia32_psrav8di">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_v8i64_ty],
                        [IntrNoMem]>;

  def int_x86_avx512_psllv_w_128 : ClangBuiltin<"__builtin_ia32_psllv8hi">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty],
                        [IntrNoMem]>;
  def int_x86_avx512_psllv_w_256 : ClangBuiltin<"__builtin_ia32_psllv16hi">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty],
                        [IntrNoMem]>;
  def int_x86_avx512_psllv_w_512 : ClangBuiltin<"__builtin_ia32_psllv32hi">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                        [IntrNoMem]>;

  def int_x86_avx512_psrlv_w_128 : ClangBuiltin<"__builtin_ia32_psrlv8hi">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty],
                        [IntrNoMem]>;
  def int_x86_avx512_psrlv_w_256 : ClangBuiltin<"__builtin_ia32_psrlv16hi">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty],
                        [IntrNoMem]>;
  def int_x86_avx512_psrlv_w_512 : ClangBuiltin<"__builtin_ia32_psrlv32hi">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                        [IntrNoMem]>;

  def int_x86_avx512_psrav_w_128 : ClangBuiltin<"__builtin_ia32_psrav8hi">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty],
                        [IntrNoMem]>;
  def int_x86_avx512_psrav_w_256 : ClangBuiltin<"__builtin_ia32_psrav16hi">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty],
                        [IntrNoMem]>;
  def int_x86_avx512_psrav_w_512 : ClangBuiltin<"__builtin_ia32_psrav32hi">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                        [IntrNoMem]>;
}

// Gather ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  // NOTE: These can't be ArgMemOnly because you can put the address completely
  // in the index register.
  def int_x86_avx2_gather_d_pd : ClangBuiltin<"__builtin_ia32_gatherd_pd">,
      Intrinsic<[llvm_v2f64_ty],
        [llvm_v2f64_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_v2f64_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_d_pd_256 : ClangBuiltin<"__builtin_ia32_gatherd_pd256">,
      Intrinsic<[llvm_v4f64_ty],
        [llvm_v4f64_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_v4f64_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_q_pd : ClangBuiltin<"__builtin_ia32_gatherq_pd">,
      Intrinsic<[llvm_v2f64_ty],
        [llvm_v2f64_ty, llvm_ptr_ty, llvm_v2i64_ty, llvm_v2f64_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_q_pd_256 : ClangBuiltin<"__builtin_ia32_gatherq_pd256">,
      Intrinsic<[llvm_v4f64_ty],
        [llvm_v4f64_ty, llvm_ptr_ty, llvm_v4i64_ty, llvm_v4f64_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_d_ps : ClangBuiltin<"__builtin_ia32_gatherd_ps">,
      Intrinsic<[llvm_v4f32_ty],
        [llvm_v4f32_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_v4f32_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_d_ps_256 : ClangBuiltin<"__builtin_ia32_gatherd_ps256">,
      Intrinsic<[llvm_v8f32_ty],
        [llvm_v8f32_ty, llvm_ptr_ty, llvm_v8i32_ty, llvm_v8f32_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_q_ps : ClangBuiltin<"__builtin_ia32_gatherq_ps">,
      Intrinsic<[llvm_v4f32_ty],
        [llvm_v4f32_ty, llvm_ptr_ty, llvm_v2i64_ty, llvm_v4f32_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_q_ps_256 : ClangBuiltin<"__builtin_ia32_gatherq_ps256">,
      Intrinsic<[llvm_v4f32_ty],
        [llvm_v4f32_ty, llvm_ptr_ty, llvm_v4i64_ty, llvm_v4f32_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx2_gather_d_q : ClangBuiltin<"__builtin_ia32_gatherd_q">,
      Intrinsic<[llvm_v2i64_ty],
        [llvm_v2i64_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_v2i64_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_d_q_256 : ClangBuiltin<"__builtin_ia32_gatherd_q256">,
      Intrinsic<[llvm_v4i64_ty],
        [llvm_v4i64_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_v4i64_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_q_q : ClangBuiltin<"__builtin_ia32_gatherq_q">,
      Intrinsic<[llvm_v2i64_ty],
        [llvm_v2i64_ty, llvm_ptr_ty, llvm_v2i64_ty, llvm_v2i64_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_q_q_256 : ClangBuiltin<"__builtin_ia32_gatherq_q256">,
      Intrinsic<[llvm_v4i64_ty],
        [llvm_v4i64_ty, llvm_ptr_ty, llvm_v4i64_ty, llvm_v4i64_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_d_d : ClangBuiltin<"__builtin_ia32_gatherd_d">,
      Intrinsic<[llvm_v4i32_ty],
        [llvm_v4i32_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_v4i32_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_d_d_256 : ClangBuiltin<"__builtin_ia32_gatherd_d256">,
      Intrinsic<[llvm_v8i32_ty],
        [llvm_v8i32_ty, llvm_ptr_ty, llvm_v8i32_ty, llvm_v8i32_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_q_d : ClangBuiltin<"__builtin_ia32_gatherq_d">,
      Intrinsic<[llvm_v4i32_ty],
        [llvm_v4i32_ty, llvm_ptr_ty, llvm_v2i64_ty, llvm_v4i32_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx2_gather_q_d_256 : ClangBuiltin<"__builtin_ia32_gatherq_d256">,
      Intrinsic<[llvm_v4i32_ty],
        [llvm_v4i32_ty, llvm_ptr_ty, llvm_v4i64_ty, llvm_v4i32_ty, llvm_i8_ty],
        [IntrReadMem, ImmArg<ArgIndex<4>>]>;
}

// Misc.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx2_pmovmskb : ClangBuiltin<"__builtin_ia32_pmovmskb256">,
              Intrinsic<[llvm_i32_ty], [llvm_v32i8_ty], [IntrNoMem]>;
  def int_x86_avx2_pshuf_b : ClangBuiltin<"__builtin_ia32_pshufb256">,
              Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty,
                         llvm_v32i8_ty], [IntrNoMem]>;
  def int_x86_avx2_mpsadbw : ClangBuiltin<"__builtin_ia32_mpsadbw256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v32i8_ty, llvm_v32i8_ty,
                         llvm_i8_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
}

//===----------------------------------------------------------------------===//
// FMA3 and FMA4

let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_fma_vfmaddsub_ps : ClangBuiltin<"__builtin_ia32_vfmaddsubps">,
              Intrinsic<[llvm_v4f32_ty],
                        [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty],
                        [IntrNoMem]>;
  def int_x86_fma_vfmaddsub_pd : ClangBuiltin<"__builtin_ia32_vfmaddsubpd">,
              Intrinsic<[llvm_v2f64_ty],
                        [llvm_v2f64_ty, llvm_v2f64_ty, llvm_v2f64_ty],
                        [IntrNoMem]>;
  def int_x86_fma_vfmaddsub_ps_256 :
               ClangBuiltin<"__builtin_ia32_vfmaddsubps256">,
              Intrinsic<[llvm_v8f32_ty],
                        [llvm_v8f32_ty, llvm_v8f32_ty, llvm_v8f32_ty],
                        [IntrNoMem]>;
  def int_x86_fma_vfmaddsub_pd_256 :
              ClangBuiltin<"__builtin_ia32_vfmaddsubpd256">,
              Intrinsic<[llvm_v4f64_ty],
                        [llvm_v4f64_ty, llvm_v4f64_ty, llvm_v4f64_ty],
                        [IntrNoMem]>;

  def int_x86_avx512_vfmadd_pd_512 :
          Intrinsic<[llvm_v8f64_ty],
          [llvm_v8f64_ty, llvm_v8f64_ty, llvm_v8f64_ty, llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_vfmadd_ps_512 :
          Intrinsic<[llvm_v16f32_ty],
          [llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty, llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_vfmaddsub_pd_512 :
          Intrinsic<[llvm_v8f64_ty],
          [llvm_v8f64_ty, llvm_v8f64_ty, llvm_v8f64_ty, llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_vfmaddsub_ps_512 :
          Intrinsic<[llvm_v16f32_ty],
          [llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty, llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_vfmadd_f64 :
          Intrinsic<[llvm_double_ty],
                    [llvm_double_ty, llvm_double_ty, llvm_double_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_vfmadd_f32 :
          Intrinsic<[llvm_float_ty],
                    [llvm_float_ty, llvm_float_ty, llvm_float_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_vpmadd52h_uq_128 :
              ClangBuiltin<"__builtin_ia32_vpmadd52huq128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_avx512_vpmadd52l_uq_128 :
              ClangBuiltin<"__builtin_ia32_vpmadd52luq128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_avx512_vpmadd52h_uq_256 :
              ClangBuiltin<"__builtin_ia32_vpmadd52huq256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty,
                         llvm_v4i64_ty], [IntrNoMem]>;
  def int_x86_avx512_vpmadd52l_uq_256 :
              ClangBuiltin<"__builtin_ia32_vpmadd52luq256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty,
                         llvm_v4i64_ty], [IntrNoMem]>;
  def int_x86_avx512_vpmadd52h_uq_512 :
              ClangBuiltin<"__builtin_ia32_vpmadd52huq512">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_v8i64_ty,
                         llvm_v8i64_ty], [IntrNoMem]>;
  def int_x86_avx512_vpmadd52l_uq_512 :
              ClangBuiltin<"__builtin_ia32_vpmadd52luq512">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_v8i64_ty,
                         llvm_v8i64_ty], [IntrNoMem]>;
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX_IFMA
  def int_x86_avx_vpmadd52h_uq_128 :
              ClangBuiltin<"__builtin_ia32_vpmadd52huqvex128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_avx_vpmadd52l_uq_128 :
              ClangBuiltin<"__builtin_ia32_vpmadd52luqvex128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_avx_vpmadd52h_uq_256 :
              ClangBuiltin<"__builtin_ia32_vpmadd52huqvex256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty,
                         llvm_v4i64_ty], [IntrNoMem]>;
  def int_x86_avx_vpmadd52l_uq_256 :
              ClangBuiltin<"__builtin_ia32_vpmadd52luqvex256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty,
                         llvm_v4i64_ty], [IntrNoMem]>;
#endif // INTEL_FEATURE_ISA_AVX_IFMA
#endif // INTEL_CUSTOMIZATION
}

// VNNI
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx512_vpdpbusd_128 :
              ClangBuiltin<"__builtin_ia32_vpdpbusd128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_avx512_vpdpbusd_256 :
              ClangBuiltin<"__builtin_ia32_vpdpbusd256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty,
                         llvm_v8i32_ty], [IntrNoMem]>;
  def int_x86_avx512_vpdpbusd_512 :
              ClangBuiltin<"__builtin_ia32_vpdpbusd512">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty,
                         llvm_v16i32_ty], [IntrNoMem]>;

  def int_x86_avx512_vpdpbusds_128 :
              ClangBuiltin<"__builtin_ia32_vpdpbusds128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_avx512_vpdpbusds_256 :
              ClangBuiltin<"__builtin_ia32_vpdpbusds256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty,
                         llvm_v8i32_ty], [IntrNoMem]>;
  def int_x86_avx512_vpdpbusds_512 :
              ClangBuiltin<"__builtin_ia32_vpdpbusds512">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty,
                         llvm_v16i32_ty], [IntrNoMem]>;

  def int_x86_avx512_vpdpwssd_128 :
              ClangBuiltin<"__builtin_ia32_vpdpwssd128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_avx512_vpdpwssd_256 :
              ClangBuiltin<"__builtin_ia32_vpdpwssd256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty,
                         llvm_v8i32_ty], [IntrNoMem]>;
  def int_x86_avx512_vpdpwssd_512 :
              ClangBuiltin<"__builtin_ia32_vpdpwssd512">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty,
                         llvm_v16i32_ty], [IntrNoMem]>;

  def int_x86_avx512_vpdpwssds_128 :
              ClangBuiltin<"__builtin_ia32_vpdpwssds128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_avx512_vpdpwssds_256 :
              ClangBuiltin<"__builtin_ia32_vpdpwssds256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty,
                         llvm_v8i32_ty], [IntrNoMem]>;
  def int_x86_avx512_vpdpwssds_512 :
              ClangBuiltin<"__builtin_ia32_vpdpwssds512">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty,
                         llvm_v16i32_ty], [IntrNoMem]>;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX512_VNNI_FP16
  def int_x86_avx512_mask_vdpphps_128
      : ClangBuiltin<"__builtin_ia32_vdpphps128_mask">,
        Intrinsic<[llvm_v4f32_ty],
                  [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_maskz_vdpphps_128
      : ClangBuiltin<"__builtin_ia32_vdpphps128_maskz">,
        Intrinsic<[llvm_v4f32_ty],
                  [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_mask_vdpphps_256
      : ClangBuiltin<"__builtin_ia32_vdpphps256_mask">,
        Intrinsic<[llvm_v8f32_ty],
                  [llvm_v8f32_ty, llvm_v8f32_ty, llvm_v8f32_ty, llvm_i8_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_maskz_vdpphps_256
      : ClangBuiltin<"__builtin_ia32_vdpphps256_maskz">,
        Intrinsic<[llvm_v8f32_ty],
                  [llvm_v8f32_ty, llvm_v8f32_ty, llvm_v8f32_ty, llvm_i8_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_mask_vdpphps_512
      : ClangBuiltin<"__builtin_ia32_vdpphps512_mask">,
        Intrinsic<[llvm_v16f32_ty],
                  [llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty, llvm_i16_ty,
                  llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_maskz_vdpphps_512
      : ClangBuiltin<"__builtin_ia32_vdpphps512_maskz">,
        Intrinsic<[llvm_v16f32_ty],
                  [llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty, llvm_i16_ty,
                  llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<4>>]>;
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_FP16
#if INTEL_FEATURE_ISA_AVX512_VNNI_INT8
  def int_x86_avx512_vpdpbssd_512
      : ClangBuiltin<"__builtin_ia32_vpdpbssd512">,
        Intrinsic<[llvm_v16i32_ty],
                  [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_vpdpbssds_512
      : ClangBuiltin<"__builtin_ia32_vpdpbssds512">,
        Intrinsic<[llvm_v16i32_ty],
                  [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_vpdpbsud_512
      : ClangBuiltin<"__builtin_ia32_vpdpbsud512">,
        Intrinsic<[llvm_v16i32_ty],
                  [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_vpdpbsuds_512
      : ClangBuiltin<"__builtin_ia32_vpdpbsuds512">,
        Intrinsic<[llvm_v16i32_ty],
                  [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_vpdpbuud_512
      : ClangBuiltin<"__builtin_ia32_vpdpbuud512">,
        Intrinsic<[llvm_v16i32_ty],
                  [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_vpdpbuuds_512
      : ClangBuiltin<"__builtin_ia32_vpdpbuuds512">,
        Intrinsic<[llvm_v16i32_ty],
                  [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT8
#if INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
  def int_x86_avx2_vdpphps_128
      : ClangBuiltin<"__builtin_ia32_vdpphps128">,
        Intrinsic<[llvm_v4f32_ty],
                  [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vdpphps_256
      : ClangBuiltin<"__builtin_ia32_vdpphps256">,
        Intrinsic<[llvm_v8f32_ty],
                  [llvm_v8f32_ty, llvm_v8f32_ty, llvm_v8f32_ty],
                  [IntrNoMem]>;
#endif // INTEL_FEATURE_ISA_AVX_DOTPROD_PHPS
#if INTEL_FEATURE_ISA_AVX_VNNI_INT8
  def int_x86_avx2_vpdpbssd_128
      : ClangBuiltin<"__builtin_ia32_vpdpbssd128">,
        Intrinsic<[llvm_v4i32_ty],
                  [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vpdpbssd_256
      : ClangBuiltin<"__builtin_ia32_vpdpbssd256">,
        Intrinsic<[llvm_v8i32_ty],
                  [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vpdpbssds_128
      : ClangBuiltin<"__builtin_ia32_vpdpbssds128">,
        Intrinsic<[llvm_v4i32_ty],
                  [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vpdpbssds_256
      : ClangBuiltin<"__builtin_ia32_vpdpbssds256">,
        Intrinsic<[llvm_v8i32_ty],
                  [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vpdpbsud_128
      : ClangBuiltin<"__builtin_ia32_vpdpbsud128">,
        Intrinsic<[llvm_v4i32_ty],
                  [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vpdpbsud_256
      : ClangBuiltin<"__builtin_ia32_vpdpbsud256">,
        Intrinsic<[llvm_v8i32_ty],
                  [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vpdpbsuds_128
      : ClangBuiltin<"__builtin_ia32_vpdpbsuds128">,
        Intrinsic<[llvm_v4i32_ty],
                  [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vpdpbsuds_256
      : ClangBuiltin<"__builtin_ia32_vpdpbsuds256">,
        Intrinsic<[llvm_v8i32_ty],
                  [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vpdpbuud_128
      : ClangBuiltin<"__builtin_ia32_vpdpbuud128">,
        Intrinsic<[llvm_v4i32_ty],
                  [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vpdpbuud_256
      : ClangBuiltin<"__builtin_ia32_vpdpbuud256">,
        Intrinsic<[llvm_v8i32_ty],
                  [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vpdpbuuds_128
      : ClangBuiltin<"__builtin_ia32_vpdpbuuds128">,
        Intrinsic<[llvm_v4i32_ty],
                  [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vpdpbuuds_256
      : ClangBuiltin<"__builtin_ia32_vpdpbuuds256">,
        Intrinsic<[llvm_v8i32_ty],
                  [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
                  [IntrNoMem]>;
#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT8
#if INTEL_FEATURE_ISA_AVX512_CONVERT
  def int_x86_avx512_mask_vcvt2ps2ph_128
      : ClangBuiltin<"__builtin_ia32_vcvt2ps2ph128_mask">,
        Intrinsic<[llvm_v8f16_ty],
                  [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v8f16_ty, llvm_i8_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_mask_vcvt2ps2ph_256
      : ClangBuiltin<"__builtin_ia32_vcvt2ps2ph256_mask">,
        Intrinsic<[llvm_v16f16_ty],
                  [llvm_v8f32_ty, llvm_v8f32_ty, llvm_v16f16_ty, llvm_i16_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_mask_vcvt2ps2ph_512
      : ClangBuiltin<"__builtin_ia32_vcvt2ps2ph512_mask">,
        Intrinsic<[llvm_v32f16_ty],
                  [llvm_v16f32_ty, llvm_v16f32_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_vcvtbf162ph_128
      : ClangBuiltin<"__builtin_ia32_vcvtbf162ph128_mask">,
        Intrinsic<[llvm_v8f16_ty],
                  [llvm_v8i16_ty, llvm_v8f16_ty, llvm_i8_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_mask_vcvtbf162ph_256
      : ClangBuiltin<"__builtin_ia32_vcvtbf162ph256_mask">,
        Intrinsic<[llvm_v16f16_ty],
                  [llvm_v16i16_ty, llvm_v16f16_ty, llvm_i16_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_mask_vcvtbf162ph_512
      : ClangBuiltin<"__builtin_ia32_vcvtbf162ph512_mask">,
        Intrinsic<[llvm_v32f16_ty],
                  [llvm_v32i16_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_mask_vcvtneph2bf16_128
      : ClangBuiltin<"__builtin_ia32_vcvtneph2bf16_128_mask">,
        Intrinsic<[llvm_v8i16_ty],
                  [llvm_v8f16_ty, llvm_v8i16_ty, llvm_i8_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_mask_vcvtneph2bf16_256
      : ClangBuiltin<"__builtin_ia32_vcvtneph2bf16_256_mask">,
        Intrinsic<[llvm_v16i16_ty],
                  [llvm_v16f16_ty, llvm_v16i16_ty, llvm_i16_ty],
                  [IntrNoMem]>;
  def int_x86_avx512_mask_vcvtneph2bf16_512
      : ClangBuiltin<"__builtin_ia32_vcvtneph2bf16_512_mask">,
        Intrinsic<[llvm_v32i16_ty],
                  [llvm_v32f16_ty, llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem]>;
#endif // INTEL_FEATURE_ISA_AVX512_CONVERT
#if INTEL_FEATURE_ISA_AVX_CONVERT
  def int_x86_avx2_vcvt2ps2ph_128
      : ClangBuiltin<"__builtin_ia32_vcvt2ps2ph128">,
        Intrinsic<[llvm_v8f16_ty],
                  [llvm_v4f32_ty, llvm_v4f32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vcvt2ps2ph_256
      : ClangBuiltin<"__builtin_ia32_vcvt2ps2ph256">,
        Intrinsic<[llvm_v16f16_ty],
                  [llvm_v8f32_ty, llvm_v8f32_ty],
                  [IntrNoMem]>;
  def int_x86_avx2_vcvtbf162ph_128
      : ClangBuiltin<"__builtin_ia32_vcvtbf162ph128">,
        Intrinsic<[llvm_v8f16_ty],
                  [llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_avx2_vcvtbf162ph_256
      : ClangBuiltin<"__builtin_ia32_vcvtbf162ph256">,
        Intrinsic<[llvm_v16f16_ty],
                  [llvm_v16i16_ty], [IntrNoMem]>;
  def int_x86_avx2_vcvtneph2bf16_128
      : ClangBuiltin<"__builtin_ia32_vcvtneph2bf16_128">,
        Intrinsic<[llvm_v8i16_ty],
                  [llvm_v8f16_ty], [IntrNoMem]>;
  def int_x86_avx2_vcvtneph2bf16_256
      : ClangBuiltin<"__builtin_ia32_vcvtneph2bf16_256">,
        Intrinsic<[llvm_v16i16_ty],
                  [llvm_v16f16_ty], [IntrNoMem]>;
#endif // INTEL_FEATURE_ISA_AVX_CONVERT
#if INTEL_FEATURE_ISA_AVX_COMPRESS
  def int_x86_avx2_vpcompressb_store_128 :
               ClangBuiltin<"__builtin_ia32_vpcompressb_store_128">,
               Intrinsic<[], [llvm_ptr_ty, llvm_v16i8_ty,
                          llvm_v16i8_ty], [IntrArgMemOnly]>;
  def int_x86_avx2_vpcompressb_store_256 :
              ClangBuiltin<"__builtin_ia32_vpcompressb_store_256">,
              Intrinsic<[], [llvm_ptr_ty, llvm_v32i8_ty,
                         llvm_v32i8_ty], [IntrArgMemOnly]>;
  def int_x86_avx2_vpcompressw_store_128 :
              ClangBuiltin<"__builtin_ia32_vpcompressw_store_128">,
              Intrinsic<[], [llvm_ptr_ty, llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrArgMemOnly]>;
  def int_x86_avx2_vpcompressw_store_256 :
              ClangBuiltin<"__builtin_ia32_vpcompressw_store_256">,
              Intrinsic<[], [llvm_ptr_ty, llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrArgMemOnly]>;
  def int_x86_avx2_vpcompressd_store_128 :
              ClangBuiltin<"__builtin_ia32_vpcompressd_store_128">,
              Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty,
                         llvm_v4i32_ty], [IntrArgMemOnly]>;
  def int_x86_avx2_vpcompressd_store_256 :
              ClangBuiltin<"__builtin_ia32_vpcompressd_store_256">,
              Intrinsic<[], [llvm_ptr_ty, llvm_v8i32_ty,
                         llvm_v8i32_ty], [IntrArgMemOnly]>;
  def int_x86_avx2_vpcompressq_store_128 :
              ClangBuiltin<"__builtin_ia32_vpcompressq_store_128">,
              Intrinsic<[], [llvm_ptr_ty, llvm_v2i64_ty,
                         llvm_v2i64_ty], [IntrArgMemOnly]>;
  def int_x86_avx2_vpcompressq_store_256 :
              ClangBuiltin<"__builtin_ia32_vpcompressq_store_256">,
              Intrinsic<[], [llvm_ptr_ty, llvm_v4i64_ty,
                         llvm_v4i64_ty], [IntrArgMemOnly]>;
  def int_x86_avx2_vpcompressb_128 :
              ClangBuiltin<"__builtin_ia32_vpcompressb_128">,
              Intrinsic<[llvm_v16i8_ty], [llvm_v16i8_ty,
                         llvm_v16i8_ty], [IntrNoMem]>;
  def int_x86_avx2_vpcompressb_256 :
              ClangBuiltin<"__builtin_ia32_vpcompressb_256">,
              Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty,
                         llvm_v32i8_ty], [IntrNoMem]>;
  def int_x86_avx2_vpcompressw_128 :
              ClangBuiltin<"__builtin_ia32_vpcompressw_128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                         llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_avx2_vpcompressw_256 :
              ClangBuiltin<"__builtin_ia32_vpcompressw_256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                         llvm_v16i16_ty], [IntrNoMem]>;
  def int_x86_avx2_vpcompressd_128 :
              ClangBuiltin<"__builtin_ia32_vpcompressd_128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty,
                         llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_avx2_vpcompressd_256 :
              ClangBuiltin<"__builtin_ia32_vpcompressd_256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty,
                         llvm_v8i32_ty], [IntrNoMem]>;
  def int_x86_avx2_vpcompressq_128 :
              ClangBuiltin<"__builtin_ia32_vpcompressq_128">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty,
                         llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_avx2_vpcompressq_256 :
              ClangBuiltin<"__builtin_ia32_vpcompressq_256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty,
                         llvm_v4i64_ty], [IntrNoMem]>;
#endif // INTEL_FEATURE_ISA_AVX_COMPRESS
#endif // INTEL_CUSTOMIZATION
}

//===----------------------------------------------------------------------===//
// XOP

let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_xop_vpermil2pd : ClangBuiltin<"__builtin_ia32_vpermil2pd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                                          llvm_v2i64_ty, llvm_i8_ty],
                        [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_xop_vpermil2pd_256 :
              ClangBuiltin<"__builtin_ia32_vpermil2pd256">,
              Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty,
                                          llvm_v4i64_ty, llvm_i8_ty],
                        [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_xop_vpermil2ps : ClangBuiltin<"__builtin_ia32_vpermil2ps">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                                          llvm_v4i32_ty, llvm_i8_ty],
                        [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_xop_vpermil2ps_256 :
              ClangBuiltin<"__builtin_ia32_vpermil2ps256">,
              Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_v8f32_ty,
                                          llvm_v8i32_ty, llvm_i8_ty],
                        [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_xop_vfrcz_pd : ClangBuiltin<"__builtin_ia32_vfrczpd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_xop_vfrcz_ps : ClangBuiltin<"__builtin_ia32_vfrczps">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_xop_vfrcz_sd : ClangBuiltin<"__builtin_ia32_vfrczsd">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty], [IntrNoMem]>;
  def int_x86_xop_vfrcz_ss : ClangBuiltin<"__builtin_ia32_vfrczss">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty], [IntrNoMem]>;
  def int_x86_xop_vfrcz_pd_256 : ClangBuiltin<"__builtin_ia32_vfrczpd256">,
              Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty], [IntrNoMem]>;
  def int_x86_xop_vfrcz_ps_256 : ClangBuiltin<"__builtin_ia32_vfrczps256">,
              Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty], [IntrNoMem]>;

  def int_x86_xop_vphaddbd :
              ClangBuiltin<"__builtin_ia32_vphaddbd">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v16i8_ty], [IntrNoMem]>;
  def int_x86_xop_vphaddbq :
              ClangBuiltin<"__builtin_ia32_vphaddbq">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v16i8_ty], [IntrNoMem]>;
  def int_x86_xop_vphaddbw :
              ClangBuiltin<"__builtin_ia32_vphaddbw">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v16i8_ty], [IntrNoMem]>;
  def int_x86_xop_vphadddq :
              ClangBuiltin<"__builtin_ia32_vphadddq">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_xop_vphaddubd :
              ClangBuiltin<"__builtin_ia32_vphaddubd">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v16i8_ty], [IntrNoMem]>;
  def int_x86_xop_vphaddubq :
              ClangBuiltin<"__builtin_ia32_vphaddubq">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v16i8_ty], [IntrNoMem]>;
  def int_x86_xop_vphaddubw :
              ClangBuiltin<"__builtin_ia32_vphaddubw">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v16i8_ty], [IntrNoMem]>;
  def int_x86_xop_vphaddudq :
              ClangBuiltin<"__builtin_ia32_vphaddudq">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_xop_vphadduwd :
              ClangBuiltin<"__builtin_ia32_vphadduwd">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_xop_vphadduwq :
              ClangBuiltin<"__builtin_ia32_vphadduwq">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_xop_vphaddwd :
              ClangBuiltin<"__builtin_ia32_vphaddwd">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_xop_vphaddwq :
              ClangBuiltin<"__builtin_ia32_vphaddwq">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_xop_vphsubbw :
              ClangBuiltin<"__builtin_ia32_vphsubbw">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v16i8_ty], [IntrNoMem]>;
  def int_x86_xop_vphsubdq :
              ClangBuiltin<"__builtin_ia32_vphsubdq">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_xop_vphsubwd :
              ClangBuiltin<"__builtin_ia32_vphsubwd">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v8i16_ty], [IntrNoMem]>;
  def int_x86_xop_vpmacsdd :
              ClangBuiltin<"__builtin_ia32_vpmacsdd">,
              Intrinsic<[llvm_v4i32_ty],
                        [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_xop_vpmacsdqh :
              ClangBuiltin<"__builtin_ia32_vpmacsdqh">,
              Intrinsic<[llvm_v2i64_ty],
                        [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v2i64_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_xop_vpmacsdql :
              ClangBuiltin<"__builtin_ia32_vpmacsdql">,
              Intrinsic<[llvm_v2i64_ty],
                        [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v2i64_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_xop_vpmacssdd :
              ClangBuiltin<"__builtin_ia32_vpmacssdd">,
              Intrinsic<[llvm_v4i32_ty],
                        [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_xop_vpmacssdqh :
              ClangBuiltin<"__builtin_ia32_vpmacssdqh">,
              Intrinsic<[llvm_v2i64_ty],
                        [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v2i64_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_xop_vpmacssdql :
              ClangBuiltin<"__builtin_ia32_vpmacssdql">,
              Intrinsic<[llvm_v2i64_ty],
                        [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v2i64_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_xop_vpmacsswd :
              ClangBuiltin<"__builtin_ia32_vpmacsswd">,
              Intrinsic<[llvm_v4i32_ty],
                        [llvm_v8i16_ty, llvm_v8i16_ty, llvm_v4i32_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_xop_vpmacssww :
              ClangBuiltin<"__builtin_ia32_vpmacssww">,
              Intrinsic<[llvm_v8i16_ty],
                        [llvm_v8i16_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_xop_vpmacswd :
              ClangBuiltin<"__builtin_ia32_vpmacswd">,
              Intrinsic<[llvm_v4i32_ty],
                        [llvm_v8i16_ty, llvm_v8i16_ty, llvm_v4i32_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_xop_vpmacsww :
              ClangBuiltin<"__builtin_ia32_vpmacsww">,
              Intrinsic<[llvm_v8i16_ty],
                        [llvm_v8i16_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_xop_vpmadcsswd :
              ClangBuiltin<"__builtin_ia32_vpmadcsswd">,
              Intrinsic<[llvm_v4i32_ty],
                        [llvm_v8i16_ty, llvm_v8i16_ty, llvm_v4i32_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_xop_vpmadcswd :
              ClangBuiltin<"__builtin_ia32_vpmadcswd">,
              Intrinsic<[llvm_v4i32_ty],
                        [llvm_v8i16_ty, llvm_v8i16_ty, llvm_v4i32_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_xop_vpperm :
              ClangBuiltin<"__builtin_ia32_vpperm">,
              Intrinsic<[llvm_v16i8_ty],
                        [llvm_v16i8_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                        [IntrNoMem]>;
  def int_x86_xop_vpshab :
              ClangBuiltin<"__builtin_ia32_vpshab">,
              Intrinsic<[llvm_v16i8_ty], [llvm_v16i8_ty, llvm_v16i8_ty],
                        [IntrNoMem]>;
  def int_x86_xop_vpshad :
              ClangBuiltin<"__builtin_ia32_vpshad">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                        [IntrNoMem]>;
  def int_x86_xop_vpshaq :
              ClangBuiltin<"__builtin_ia32_vpshaq">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                        [IntrNoMem]>;
  def int_x86_xop_vpshaw :
              ClangBuiltin<"__builtin_ia32_vpshaw">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty],
                        [IntrNoMem]>;
  def int_x86_xop_vpshlb :
              ClangBuiltin<"__builtin_ia32_vpshlb">,
              Intrinsic<[llvm_v16i8_ty], [llvm_v16i8_ty, llvm_v16i8_ty],
                        [IntrNoMem]>;
  def int_x86_xop_vpshld :
              ClangBuiltin<"__builtin_ia32_vpshld">,
              Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                        [IntrNoMem]>;
  def int_x86_xop_vpshlq :
              ClangBuiltin<"__builtin_ia32_vpshlq">,
              Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                        [IntrNoMem]>;
  def int_x86_xop_vpshlw :
              ClangBuiltin<"__builtin_ia32_vpshlw">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty],
                        [IntrNoMem]>;
}

//===----------------------------------------------------------------------===//
// LWP
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_llwpcb :
              ClangBuiltin<"__builtin_ia32_llwpcb">,
              Intrinsic<[], [llvm_ptr_ty], []>;
  def int_x86_slwpcb :
              ClangBuiltin<"__builtin_ia32_slwpcb">,
              Intrinsic<[llvm_ptr_ty], [], []>;
  def int_x86_lwpins32 :
              ClangBuiltin<"__builtin_ia32_lwpins32">,
              Intrinsic<[llvm_i8_ty], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty],
                        [ImmArg<ArgIndex<2>>]>;
  def int_x86_lwpins64 :
              ClangBuiltin<"__builtin_ia32_lwpins64">,
              Intrinsic<[llvm_i8_ty], [llvm_i64_ty, llvm_i32_ty, llvm_i32_ty],
                        [ImmArg<ArgIndex<2>>]>;
  def int_x86_lwpval32 :
              ClangBuiltin<"__builtin_ia32_lwpval32">,
              Intrinsic<[], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty],
                        [ImmArg<ArgIndex<2>>]>;
  def int_x86_lwpval64 :
              ClangBuiltin<"__builtin_ia32_lwpval64">,
              Intrinsic<[], [llvm_i64_ty, llvm_i32_ty, llvm_i32_ty],
                        [ImmArg<ArgIndex<2>>]>;
}

//===----------------------------------------------------------------------===//
// MMX

// Empty MMX state op.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_mmx_emms  : ClangBuiltin<"__builtin_ia32_emms">,
              Intrinsic<[], [], []>;
  def int_x86_mmx_femms : ClangBuiltin<"__builtin_ia32_femms">,
              Intrinsic<[], [], []>;
}

// Integer arithmetic ops.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  // Addition
  def int_x86_mmx_padd_b : ClangBuiltin<"__builtin_ia32_paddb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_mmx_padd_w : ClangBuiltin<"__builtin_ia32_paddw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_mmx_padd_d : ClangBuiltin<"__builtin_ia32_paddd">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_mmx_padd_q : ClangBuiltin<"__builtin_ia32_paddq">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem, Commutative]>;

  def int_x86_mmx_padds_b : ClangBuiltin<"__builtin_ia32_paddsb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;
  def int_x86_mmx_padds_w : ClangBuiltin<"__builtin_ia32_paddsw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;

  def int_x86_mmx_paddus_b : ClangBuiltin<"__builtin_ia32_paddusb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;
  def int_x86_mmx_paddus_w : ClangBuiltin<"__builtin_ia32_paddusw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;

  // Subtraction
  def int_x86_mmx_psub_b : ClangBuiltin<"__builtin_ia32_psubb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_mmx_psub_w : ClangBuiltin<"__builtin_ia32_psubw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_mmx_psub_d : ClangBuiltin<"__builtin_ia32_psubd">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_mmx_psub_q : ClangBuiltin<"__builtin_ia32_psubq">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;

  def int_x86_mmx_psubs_b : ClangBuiltin<"__builtin_ia32_psubsb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_mmx_psubs_w : ClangBuiltin<"__builtin_ia32_psubsw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;

  def int_x86_mmx_psubus_b : ClangBuiltin<"__builtin_ia32_psubusb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_mmx_psubus_w : ClangBuiltin<"__builtin_ia32_psubusw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;

  // Multiplication
  def int_x86_mmx_pmulh_w : ClangBuiltin<"__builtin_ia32_pmulhw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;
  def int_x86_mmx_pmull_w : ClangBuiltin<"__builtin_ia32_pmullw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;
  def int_x86_mmx_pmulhu_w : ClangBuiltin<"__builtin_ia32_pmulhuw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;
  def int_x86_mmx_pmulu_dq : ClangBuiltin<"__builtin_ia32_pmuludq">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;
  def int_x86_mmx_pmadd_wd : ClangBuiltin<"__builtin_ia32_pmaddwd">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;

  // Bitwise operations
  def int_x86_mmx_pand : ClangBuiltin<"__builtin_ia32_pand">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_mmx_pandn : ClangBuiltin<"__builtin_ia32_pandn">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_mmx_por : ClangBuiltin<"__builtin_ia32_por">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem, Commutative]>;
  def int_x86_mmx_pxor : ClangBuiltin<"__builtin_ia32_pxor">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem, Commutative]>;

  // Averages
  def int_x86_mmx_pavg_b : ClangBuiltin<"__builtin_ia32_pavgb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;
  def int_x86_mmx_pavg_w : ClangBuiltin<"__builtin_ia32_pavgw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;

  // Maximum
  def int_x86_mmx_pmaxu_b : ClangBuiltin<"__builtin_ia32_pmaxub">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;
  def int_x86_mmx_pmaxs_w : ClangBuiltin<"__builtin_ia32_pmaxsw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;

  // Minimum
  def int_x86_mmx_pminu_b : ClangBuiltin<"__builtin_ia32_pminub">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;
  def int_x86_mmx_pmins_w : ClangBuiltin<"__builtin_ia32_pminsw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;

  // Packed sum of absolute differences
  def int_x86_mmx_psad_bw : ClangBuiltin<"__builtin_ia32_psadbw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;
}

// Integer shift ops.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  // Shift left logical
  def int_x86_mmx_psll_w : ClangBuiltin<"__builtin_ia32_psllw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_mmx_psll_d : ClangBuiltin<"__builtin_ia32_pslld">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_mmx_psll_q : ClangBuiltin<"__builtin_ia32_psllq">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;

  def int_x86_mmx_psrl_w : ClangBuiltin<"__builtin_ia32_psrlw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_mmx_psrl_d : ClangBuiltin<"__builtin_ia32_psrld">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_mmx_psrl_q : ClangBuiltin<"__builtin_ia32_psrlq">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;

  def int_x86_mmx_psra_w : ClangBuiltin<"__builtin_ia32_psraw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_mmx_psra_d : ClangBuiltin<"__builtin_ia32_psrad">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;

  // Oddly these don't require an immediate due to a gcc compatibility issue.
  def int_x86_mmx_pslli_w : ClangBuiltin<"__builtin_ia32_psllwi">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_mmx_pslli_d : ClangBuiltin<"__builtin_ia32_pslldi">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_mmx_pslli_q : ClangBuiltin<"__builtin_ia32_psllqi">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_i32_ty], [IntrNoMem]>;

  def int_x86_mmx_psrli_w : ClangBuiltin<"__builtin_ia32_psrlwi">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_mmx_psrli_d : ClangBuiltin<"__builtin_ia32_psrldi">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_mmx_psrli_q : ClangBuiltin<"__builtin_ia32_psrlqi">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_i32_ty], [IntrNoMem]>;

  def int_x86_mmx_psrai_w : ClangBuiltin<"__builtin_ia32_psrawi">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_i32_ty], [IntrNoMem]>;
  def int_x86_mmx_psrai_d : ClangBuiltin<"__builtin_ia32_psradi">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_i32_ty], [IntrNoMem]>;
}
// Permute
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx512_permvar_df_256 : ClangBuiltin<"__builtin_ia32_permvardf256">,
              Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty,
                        llvm_v4i64_ty],  [IntrNoMem]>;
  def int_x86_avx512_permvar_df_512 : ClangBuiltin<"__builtin_ia32_permvardf512">,
              Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty,
                        llvm_v8i64_ty],  [IntrNoMem]>;
  def int_x86_avx512_permvar_di_256 : ClangBuiltin<"__builtin_ia32_permvardi256">,
              Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty,
                        llvm_v4i64_ty],  [IntrNoMem]>;
  def int_x86_avx512_permvar_di_512 : ClangBuiltin<"__builtin_ia32_permvardi512">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty,
                        llvm_v8i64_ty],  [IntrNoMem]>;
  def int_x86_avx512_permvar_hi_128 : ClangBuiltin<"__builtin_ia32_permvarhi128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty,
                        llvm_v8i16_ty],  [IntrNoMem]>;
  def int_x86_avx512_permvar_hi_256 : ClangBuiltin<"__builtin_ia32_permvarhi256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty,
                        llvm_v16i16_ty],  [IntrNoMem]>;
  def int_x86_avx512_permvar_hi_512 : ClangBuiltin<"__builtin_ia32_permvarhi512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty,
                        llvm_v32i16_ty],  [IntrNoMem]>;
  def int_x86_avx512_permvar_qi_128 : ClangBuiltin<"__builtin_ia32_permvarqi128">,
              Intrinsic<[llvm_v16i8_ty], [llvm_v16i8_ty,
                        llvm_v16i8_ty],  [IntrNoMem]>;
  def int_x86_avx512_permvar_qi_256 : ClangBuiltin<"__builtin_ia32_permvarqi256">,
              Intrinsic<[llvm_v32i8_ty], [llvm_v32i8_ty,
                        llvm_v32i8_ty],  [IntrNoMem]>;
  def int_x86_avx512_permvar_qi_512 : ClangBuiltin<"__builtin_ia32_permvarqi512">,
              Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty,
                        llvm_v64i8_ty],  [IntrNoMem]>;
  def int_x86_avx512_permvar_sf_512 : ClangBuiltin<"__builtin_ia32_permvarsf512">,
              Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty,
                        llvm_v16i32_ty],  [IntrNoMem]>;
  def int_x86_avx512_permvar_si_512 : ClangBuiltin<"__builtin_ia32_permvarsi512">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty,
                        llvm_v16i32_ty],  [IntrNoMem]>;
}
// Pack ops.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_mmx_packsswb : ClangBuiltin<"__builtin_ia32_packsswb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_mmx_packssdw : ClangBuiltin<"__builtin_ia32_packssdw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_mmx_packuswb : ClangBuiltin<"__builtin_ia32_packuswb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
}

// Unpacking ops.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_mmx_punpckhbw : ClangBuiltin<"__builtin_ia32_punpckhbw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_mmx_punpckhwd : ClangBuiltin<"__builtin_ia32_punpckhwd">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_mmx_punpckhdq : ClangBuiltin<"__builtin_ia32_punpckhdq">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_mmx_punpcklbw : ClangBuiltin<"__builtin_ia32_punpcklbw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_mmx_punpcklwd : ClangBuiltin<"__builtin_ia32_punpcklwd">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
  def int_x86_mmx_punpckldq : ClangBuiltin<"__builtin_ia32_punpckldq">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty, llvm_x86mmx_ty],
                        [IntrNoMem]>;
}

// Integer comparison ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_mmx_pcmpeq_b : ClangBuiltin<"__builtin_ia32_pcmpeqb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;
  def int_x86_mmx_pcmpeq_w : ClangBuiltin<"__builtin_ia32_pcmpeqw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;
  def int_x86_mmx_pcmpeq_d : ClangBuiltin<"__builtin_ia32_pcmpeqd">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem, Commutative]>;

  def int_x86_mmx_pcmpgt_b : ClangBuiltin<"__builtin_ia32_pcmpgtb">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_mmx_pcmpgt_w : ClangBuiltin<"__builtin_ia32_pcmpgtw">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
  def int_x86_mmx_pcmpgt_d : ClangBuiltin<"__builtin_ia32_pcmpgtd">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                         llvm_x86mmx_ty], [IntrNoMem]>;
}

// Misc.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_mmx_maskmovq : ClangBuiltin<"__builtin_ia32_maskmovq">,
              Intrinsic<[], [llvm_x86mmx_ty, llvm_x86mmx_ty, llvm_ptr_ty], []>;

  def int_x86_mmx_pmovmskb : ClangBuiltin<"__builtin_ia32_pmovmskb">,
              Intrinsic<[llvm_i32_ty], [llvm_x86mmx_ty], [IntrNoMem]>;

  def int_x86_mmx_movnt_dq : ClangBuiltin<"__builtin_ia32_movntq">,
              Intrinsic<[], [llvm_ptrx86mmx_ty, llvm_x86mmx_ty], []>;

  def int_x86_mmx_palignr_b : ClangBuiltin<"__builtin_ia32_palignr">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                        llvm_x86mmx_ty, llvm_i8_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;

  def int_x86_mmx_pextr_w : ClangBuiltin<"__builtin_ia32_vec_ext_v4hi">,
              Intrinsic<[llvm_i32_ty], [llvm_x86mmx_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;

  def int_x86_mmx_pinsr_w : ClangBuiltin<"__builtin_ia32_vec_set_v4hi">,
              Intrinsic<[llvm_x86mmx_ty], [llvm_x86mmx_ty,
                        llvm_i32_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
}

//===----------------------------------------------------------------------===//
// BMI

let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_bmi_bextr_32 : ClangBuiltin<"__builtin_ia32_bextr_u32">,
              Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_x86_bmi_bextr_64 : ClangBuiltin<"__builtin_ia32_bextr_u64">,
              Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty], [IntrNoMem]>;
  def int_x86_bmi_bzhi_32 : ClangBuiltin<"__builtin_ia32_bzhi_si">,
              Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_x86_bmi_bzhi_64 : ClangBuiltin<"__builtin_ia32_bzhi_di">,
              Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty], [IntrNoMem]>;
  def int_x86_bmi_pdep_32 : ClangBuiltin<"__builtin_ia32_pdep_si">,
              Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_x86_bmi_pdep_64 : ClangBuiltin<"__builtin_ia32_pdep_di">,
              Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty], [IntrNoMem]>;
  def int_x86_bmi_pext_32 : ClangBuiltin<"__builtin_ia32_pext_si">,
              Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_x86_bmi_pext_64 : ClangBuiltin<"__builtin_ia32_pext_di">,
              Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty], [IntrNoMem]>;
}

//===----------------------------------------------------------------------===//
// FS/GS Base

let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_rdfsbase_32 : ClangBuiltin<"__builtin_ia32_rdfsbase32">,
              Intrinsic<[llvm_i32_ty], []>;
  def int_x86_rdgsbase_32 : ClangBuiltin<"__builtin_ia32_rdgsbase32">,
              Intrinsic<[llvm_i32_ty], []>;
  def int_x86_rdfsbase_64 : ClangBuiltin<"__builtin_ia32_rdfsbase64">,
              Intrinsic<[llvm_i64_ty], []>;
  def int_x86_rdgsbase_64 : ClangBuiltin<"__builtin_ia32_rdgsbase64">,
              Intrinsic<[llvm_i64_ty], []>;
  def int_x86_wrfsbase_32 : ClangBuiltin<"__builtin_ia32_wrfsbase32">,
              Intrinsic<[], [llvm_i32_ty]>;
  def int_x86_wrgsbase_32 : ClangBuiltin<"__builtin_ia32_wrgsbase32">,
              Intrinsic<[], [llvm_i32_ty]>;
  def int_x86_wrfsbase_64 : ClangBuiltin<"__builtin_ia32_wrfsbase64">,
              Intrinsic<[], [llvm_i64_ty]>;
  def int_x86_wrgsbase_64 : ClangBuiltin<"__builtin_ia32_wrgsbase64">,
              Intrinsic<[], [llvm_i64_ty]>;
}

//===----------------------------------------------------------------------===//
// FXSR
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_fxrstor : ClangBuiltin<"__builtin_ia32_fxrstor">,
              Intrinsic<[], [llvm_ptr_ty], []>;
  def int_x86_fxrstor64 : ClangBuiltin<"__builtin_ia32_fxrstor64">,
              Intrinsic<[], [llvm_ptr_ty], []>;
  def int_x86_fxsave : ClangBuiltin<"__builtin_ia32_fxsave">,
              Intrinsic<[], [llvm_ptr_ty], []>;
  def int_x86_fxsave64 : ClangBuiltin<"__builtin_ia32_fxsave64">,
              Intrinsic<[], [llvm_ptr_ty], []>;
}

//===----------------------------------------------------------------------===//
// XSAVE
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_xsave :
              Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_xsave64 :
              Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_xrstor :
              Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_xrstor64 :
              Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_xsaveopt :
              Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_xsaveopt64 :
              Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_xrstors :
              Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_xrstors64 :
              Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_xsavec :
              Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_xsavec64 :
              Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_xsaves :
              Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_xsaves64 :
              Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_xgetbv :
              Intrinsic<[llvm_i64_ty], [llvm_i32_ty], []>;
  def int_x86_xsetbv :
              Intrinsic<[], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], []>;
}

//===----------------------------------------------------------------------===//
// CLFLUSHOPT and CLWB
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_clflushopt : ClangBuiltin<"__builtin_ia32_clflushopt">,
              Intrinsic<[], [llvm_ptr_ty], []>;

  def int_x86_clwb : ClangBuiltin<"__builtin_ia32_clwb">,
              Intrinsic<[], [llvm_ptr_ty], []>;
}

//===----------------------------------------------------------------------===//
// Support protection key
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_rdpkru : ClangBuiltin <"__builtin_ia32_rdpkru">,
              Intrinsic<[llvm_i32_ty], [], []>;
  def int_x86_wrpkru : ClangBuiltin<"__builtin_ia32_wrpkru">,
              Intrinsic<[], [llvm_i32_ty], []>;
}
//===----------------------------------------------------------------------===//
// Half float conversion

let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_vcvtps2ph_128 : ClangBuiltin<"__builtin_ia32_vcvtps2ph">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v4f32_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_vcvtps2ph_256 : ClangBuiltin<"__builtin_ia32_vcvtps2ph256">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8f32_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_vcvtph2ps_512 :
              Intrinsic<[llvm_v16f32_ty], [llvm_v16i16_ty, llvm_v16f32_ty,
                                           llvm_i16_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_mask_vcvtps2ph_512 : ClangBuiltin<"__builtin_ia32_vcvtps2ph512_mask">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16f32_ty, llvm_i32_ty,
                                           llvm_v16i16_ty, llvm_i16_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_vcvtps2ph_256 : ClangBuiltin<"__builtin_ia32_vcvtps2ph256_mask">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8f32_ty, llvm_i32_ty,
                                           llvm_v8i16_ty, llvm_i8_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_vcvtps2ph_128 : ClangBuiltin<"__builtin_ia32_vcvtps2ph_mask">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v4f32_ty, llvm_i32_ty,
                                           llvm_v8i16_ty, llvm_i8_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
}

//===----------------------------------------------------------------------===//
// TBM

let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_tbm_bextri_u32 : ClangBuiltin<"__builtin_ia32_bextri_u32">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_tbm_bextri_u64 : ClangBuiltin<"__builtin_ia32_bextri_u64">,
        Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
}

//===----------------------------------------------------------------------===//
// RDRAND intrinsics - Return a random value and whether it is valid.
// RDSEED intrinsics - Return a NIST SP800-90B & C compliant random value and
// whether it is valid.

let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  // These are declared side-effecting so they don't get eliminated by CSE or
  // LICM.
  def int_x86_rdrand_16 : Intrinsic<[llvm_i16_ty, llvm_i32_ty], [], []>;
  def int_x86_rdrand_32 : Intrinsic<[llvm_i32_ty, llvm_i32_ty], [], []>;
  def int_x86_rdrand_64 : Intrinsic<[llvm_i64_ty, llvm_i32_ty], [], []>;
  def int_x86_rdseed_16 : Intrinsic<[llvm_i16_ty, llvm_i32_ty], [], []>;
  def int_x86_rdseed_32 : Intrinsic<[llvm_i32_ty, llvm_i32_ty], [], []>;
  def int_x86_rdseed_64 : Intrinsic<[llvm_i64_ty, llvm_i32_ty], [], []>;
}

//===----------------------------------------------------------------------===//
// ADX

let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_addcarry_32:
        Intrinsic<[llvm_i8_ty, llvm_i32_ty],
                  [llvm_i8_ty, llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_x86_addcarry_64:
        Intrinsic<[llvm_i8_ty, llvm_i64_ty],
                  [llvm_i8_ty, llvm_i64_ty, llvm_i64_ty], [IntrNoMem]>;
  def int_x86_subborrow_32:
        Intrinsic<[llvm_i8_ty, llvm_i32_ty],
                  [llvm_i8_ty, llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;
  def int_x86_subborrow_64:
        Intrinsic<[llvm_i8_ty, llvm_i64_ty],
                  [llvm_i8_ty, llvm_i64_ty, llvm_i64_ty], [IntrNoMem]>;
}

//===----------------------------------------------------------------------===//
// RTM intrinsics. Transactional Memory support.

let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_xbegin : ClangBuiltin<"__builtin_ia32_xbegin">,
              Intrinsic<[llvm_i32_ty], [], []>;
  def int_x86_xend : ClangBuiltin<"__builtin_ia32_xend">,
              Intrinsic<[], [], []>;
  def int_x86_xabort : ClangBuiltin<"__builtin_ia32_xabort">,
              Intrinsic<[], [llvm_i8_ty], [ImmArg<ArgIndex<0>>]>;
  def int_x86_xtest : ClangBuiltin<"__builtin_ia32_xtest">,
              Intrinsic<[llvm_i32_ty], [], []>;
}

//===----------------------------------------------------------------------===//
// AVX512

// Mask ops
let TargetPrefix = "x86" in {
  def int_x86_avx512_kadd_b :
        Intrinsic<[llvm_v8i1_ty], [llvm_v8i1_ty, llvm_v8i1_ty], [IntrNoMem]>;
  def int_x86_avx512_kadd_w :
        Intrinsic<[llvm_v16i1_ty], [llvm_v16i1_ty, llvm_v16i1_ty], [IntrNoMem]>;
  def int_x86_avx512_kadd_d :
        Intrinsic<[llvm_v32i1_ty], [llvm_v32i1_ty, llvm_v32i1_ty], [IntrNoMem]>;
  def int_x86_avx512_kadd_q :
        Intrinsic<[llvm_v64i1_ty], [llvm_v64i1_ty, llvm_v64i1_ty], [IntrNoMem]>;

  def int_x86_avx512_ktestc_b :
          Intrinsic<[llvm_i32_ty], [llvm_v8i1_ty, llvm_v8i1_ty], [IntrNoMem]>;
  def int_x86_avx512_ktestc_w :
          Intrinsic<[llvm_i32_ty], [llvm_v16i1_ty, llvm_v16i1_ty], [IntrNoMem]>;
  def int_x86_avx512_ktestc_d :
          Intrinsic<[llvm_i32_ty], [llvm_v32i1_ty, llvm_v32i1_ty], [IntrNoMem]>;
  def int_x86_avx512_ktestc_q :
          Intrinsic<[llvm_i32_ty], [llvm_v64i1_ty, llvm_v64i1_ty], [IntrNoMem]>;

  def int_x86_avx512_ktestz_b :
          Intrinsic<[llvm_i32_ty], [llvm_v8i1_ty, llvm_v8i1_ty], [IntrNoMem]>;
  def int_x86_avx512_ktestz_w :
          Intrinsic<[llvm_i32_ty], [llvm_v16i1_ty, llvm_v16i1_ty], [IntrNoMem]>;
  def int_x86_avx512_ktestz_d :
          Intrinsic<[llvm_i32_ty], [llvm_v32i1_ty, llvm_v32i1_ty], [IntrNoMem]>;
  def int_x86_avx512_ktestz_q :
          Intrinsic<[llvm_i32_ty], [llvm_v64i1_ty, llvm_v64i1_ty], [IntrNoMem]>;
}

// Conversion ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx512_cvttss2si : ClangBuiltin<"__builtin_ia32_vcvttss2si32">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_cvttss2si64 : ClangBuiltin<"__builtin_ia32_vcvttss2si64">,
              Intrinsic<[llvm_i64_ty], [llvm_v4f32_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_cvttss2usi : ClangBuiltin<"__builtin_ia32_vcvttss2usi32">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_cvttss2usi64 : ClangBuiltin<"__builtin_ia32_vcvttss2usi64">,
              Intrinsic<[llvm_i64_ty], [llvm_v4f32_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_cvtusi2ss : ClangBuiltin<"__builtin_ia32_cvtusi2ss32">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_i32_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_cvtusi642ss : ClangBuiltin<"__builtin_ia32_cvtusi2ss64">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_i64_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_cvttsd2si : ClangBuiltin<"__builtin_ia32_vcvttsd2si32">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_cvttsd2si64 : ClangBuiltin<"__builtin_ia32_vcvttsd2si64">,
              Intrinsic<[llvm_i64_ty], [llvm_v2f64_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_cvttsd2usi : ClangBuiltin<"__builtin_ia32_vcvttsd2usi32">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_cvttsd2usi64 : ClangBuiltin<"__builtin_ia32_vcvttsd2usi64">,
              Intrinsic<[llvm_i64_ty], [llvm_v2f64_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_cvtusi642sd : ClangBuiltin<"__builtin_ia32_cvtusi2sd64">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty,
                         llvm_i64_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_vcvtss2usi32 : ClangBuiltin<"__builtin_ia32_vcvtss2usi32">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_vcvtss2usi64 : ClangBuiltin<"__builtin_ia32_vcvtss2usi64">,
              Intrinsic<[llvm_i64_ty], [llvm_v4f32_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_vcvtss2si32 : ClangBuiltin<"__builtin_ia32_vcvtss2si32">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_vcvtss2si64 : ClangBuiltin<"__builtin_ia32_vcvtss2si64">,
              Intrinsic<[llvm_i64_ty], [llvm_v4f32_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_vcvtsd2usi32 : ClangBuiltin<"__builtin_ia32_vcvtsd2usi32">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_vcvtsd2usi64 : ClangBuiltin<"__builtin_ia32_vcvtsd2usi64">,
              Intrinsic<[llvm_i64_ty], [llvm_v2f64_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_vcvtsd2si32 : ClangBuiltin<"__builtin_ia32_vcvtsd2si32">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_vcvtsd2si64 : ClangBuiltin<"__builtin_ia32_vcvtsd2si64">,
              Intrinsic<[llvm_i64_ty], [llvm_v2f64_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_cvtsi2ss32 : ClangBuiltin<"__builtin_ia32_cvtsi2ss32">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_i32_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_cvtsi2ss64 : ClangBuiltin<"__builtin_ia32_cvtsi2ss64">,
              Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty,
                         llvm_i64_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_cvtsi2sd64 : ClangBuiltin<"__builtin_ia32_cvtsi2sd64">,
              Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty,
                         llvm_i64_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
}

// Pack ops.
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx512_packsswb_512 : ClangBuiltin<"__builtin_ia32_packsswb512">,
              Intrinsic<[llvm_v64i8_ty], [llvm_v32i16_ty,llvm_v32i16_ty],
                        [IntrNoMem]>;
  def int_x86_avx512_packssdw_512 : ClangBuiltin<"__builtin_ia32_packssdw512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                         [IntrNoMem]>;
  def int_x86_avx512_packuswb_512 : ClangBuiltin<"__builtin_ia32_packuswb512">,
              Intrinsic<[llvm_v64i8_ty], [llvm_v32i16_ty,llvm_v32i16_ty],
                         [IntrNoMem]>;
  def int_x86_avx512_packusdw_512 : ClangBuiltin<"__builtin_ia32_packusdw512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
                         [IntrNoMem]>;
}

// Vector convert
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
  def int_x86_avx512_sitofp_round :
          Intrinsic<[llvm_anyfloat_ty], [llvm_anyint_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<1>>]>;

  def int_x86_avx512_uitofp_round :
          Intrinsic<[llvm_anyfloat_ty], [llvm_anyint_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<1>>]>;

  def int_x86_avx512_mask_cvtpd2dq_128 :
        ClangBuiltin<"__builtin_ia32_cvtpd2dq128_mask">,
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v2f64_ty, llvm_v4i32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtpd2dq_512 :
        ClangBuiltin<"__builtin_ia32_cvtpd2dq512_mask">,
          Intrinsic<[llvm_v8i32_ty],
          [llvm_v8f64_ty, llvm_v8i32_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvtpd2ps_512 :
        ClangBuiltin<"__builtin_ia32_cvtpd2ps512_mask">,
          Intrinsic<[llvm_v8f32_ty],
          [llvm_v8f64_ty, llvm_v8f32_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvtsd2ss_round :
        ClangBuiltin<"__builtin_ia32_cvtsd2ss_round_mask">,
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v4f32_ty, llvm_v2f64_ty, llvm_v4f32_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_cvtss2sd_round :
        ClangBuiltin<"__builtin_ia32_cvtss2sd_round_mask">,
          Intrinsic<[llvm_v2f64_ty],
          [llvm_v2f64_ty, llvm_v4f32_ty, llvm_v2f64_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_cvtpd2ps :
        ClangBuiltin<"__builtin_ia32_cvtpd2ps_mask">,
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v2f64_ty, llvm_v4f32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtpd2qq_128 :
        ClangBuiltin<"__builtin_ia32_cvtpd2qq128_mask">,
          Intrinsic<[llvm_v2i64_ty],
          [llvm_v2f64_ty, llvm_v2i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtpd2qq_256 :
        ClangBuiltin<"__builtin_ia32_cvtpd2qq256_mask">,
          Intrinsic<[llvm_v4i64_ty],
          [llvm_v4f64_ty, llvm_v4i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtpd2qq_512 :
        ClangBuiltin<"__builtin_ia32_cvtpd2qq512_mask">,
          Intrinsic<[llvm_v8i64_ty],
          [llvm_v8f64_ty, llvm_v8i64_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvtpd2udq_128 :
        ClangBuiltin<"__builtin_ia32_cvtpd2udq128_mask">,
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v2f64_ty, llvm_v4i32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtpd2udq_256 :
        ClangBuiltin<"__builtin_ia32_cvtpd2udq256_mask">,
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v4f64_ty, llvm_v4i32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtpd2udq_512 :
        ClangBuiltin<"__builtin_ia32_cvtpd2udq512_mask">,
          Intrinsic<[llvm_v8i32_ty],
          [llvm_v8f64_ty, llvm_v8i32_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvtpd2uqq_128 :
        ClangBuiltin<"__builtin_ia32_cvtpd2uqq128_mask">,
          Intrinsic<[llvm_v2i64_ty],
          [llvm_v2f64_ty, llvm_v2i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtpd2uqq_256 :
        ClangBuiltin<"__builtin_ia32_cvtpd2uqq256_mask">,
          Intrinsic<[llvm_v4i64_ty],
          [llvm_v4f64_ty, llvm_v4i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtpd2uqq_512 :
        ClangBuiltin<"__builtin_ia32_cvtpd2uqq512_mask">,
          Intrinsic<[llvm_v8i64_ty],
          [llvm_v8f64_ty, llvm_v8i64_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvtps2dq_128 :
        ClangBuiltin<"__builtin_ia32_cvtps2dq128_mask">,
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v4f32_ty, llvm_v4i32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtps2dq_256 :
        ClangBuiltin<"__builtin_ia32_cvtps2dq256_mask">,
          Intrinsic<[llvm_v8i32_ty],
          [llvm_v8f32_ty, llvm_v8i32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtps2dq_512 :
        ClangBuiltin<"__builtin_ia32_cvtps2dq512_mask">,
          Intrinsic<[llvm_v16i32_ty],
          [llvm_v16f32_ty, llvm_v16i32_ty,  llvm_i16_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvtps2pd_512 :
        ClangBuiltin<"__builtin_ia32_cvtps2pd512_mask">,
          Intrinsic<[llvm_v8f64_ty],
          [llvm_v8f32_ty, llvm_v8f64_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvtps2qq_128 :
        ClangBuiltin<"__builtin_ia32_cvtps2qq128_mask">,
          Intrinsic<[llvm_v2i64_ty],
          [llvm_v4f32_ty, llvm_v2i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtps2qq_256 :
        ClangBuiltin<"__builtin_ia32_cvtps2qq256_mask">,
          Intrinsic<[llvm_v4i64_ty],
          [llvm_v4f32_ty, llvm_v4i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtps2qq_512 :
        ClangBuiltin<"__builtin_ia32_cvtps2qq512_mask">,
          Intrinsic<[llvm_v8i64_ty],
          [llvm_v8f32_ty, llvm_v8i64_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvtps2udq_128 :
        ClangBuiltin<"__builtin_ia32_cvtps2udq128_mask">,
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v4f32_ty, llvm_v4i32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtps2udq_256 :
        ClangBuiltin<"__builtin_ia32_cvtps2udq256_mask">,
          Intrinsic<[llvm_v8i32_ty],
          [llvm_v8f32_ty, llvm_v8i32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtps2udq_512 :
        ClangBuiltin<"__builtin_ia32_cvtps2udq512_mask">,
          Intrinsic<[llvm_v16i32_ty],
          [llvm_v16f32_ty, llvm_v16i32_ty,  llvm_i16_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvtps2uqq_128 :
        ClangBuiltin<"__builtin_ia32_cvtps2uqq128_mask">,
          Intrinsic<[llvm_v2i64_ty],
          [llvm_v4f32_ty, llvm_v2i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtps2uqq_256 :
        ClangBuiltin<"__builtin_ia32_cvtps2uqq256_mask">,
          Intrinsic<[llvm_v4i64_ty],
          [llvm_v4f32_ty, llvm_v4i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvtps2uqq_512 :
        ClangBuiltin<"__builtin_ia32_cvtps2uqq512_mask">,
          Intrinsic<[llvm_v8i64_ty],
          [llvm_v8f32_ty, llvm_v8i64_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvtqq2ps_128 :
        ClangBuiltin<"__builtin_ia32_cvtqq2ps128_mask">,
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v2i64_ty, llvm_v4f32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttpd2dq_128 :
        ClangBuiltin<"__builtin_ia32_cvttpd2dq128_mask">,
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v2f64_ty, llvm_v4i32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttpd2dq_512 :
        ClangBuiltin<"__builtin_ia32_cvttpd2dq512_mask">,
          Intrinsic<[llvm_v8i32_ty],
          [llvm_v8f64_ty, llvm_v8i32_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvttpd2qq_128 :
        ClangBuiltin<"__builtin_ia32_cvttpd2qq128_mask">,
          Intrinsic<[llvm_v2i64_ty],
          [llvm_v2f64_ty, llvm_v2i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttpd2qq_256 :
        ClangBuiltin<"__builtin_ia32_cvttpd2qq256_mask">,
          Intrinsic<[llvm_v4i64_ty],
          [llvm_v4f64_ty, llvm_v4i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttpd2qq_512 :
        ClangBuiltin<"__builtin_ia32_cvttpd2qq512_mask">,
          Intrinsic<[llvm_v8i64_ty],
          [llvm_v8f64_ty, llvm_v8i64_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvttpd2udq_128 :
        ClangBuiltin<"__builtin_ia32_cvttpd2udq128_mask">,
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v2f64_ty, llvm_v4i32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttpd2udq_256 :
        ClangBuiltin<"__builtin_ia32_cvttpd2udq256_mask">,
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v4f64_ty, llvm_v4i32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttpd2udq_512 :
        ClangBuiltin<"__builtin_ia32_cvttpd2udq512_mask">,
          Intrinsic<[llvm_v8i32_ty],
          [llvm_v8f64_ty, llvm_v8i32_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvttpd2uqq_128 :
        ClangBuiltin<"__builtin_ia32_cvttpd2uqq128_mask">,
          Intrinsic<[llvm_v2i64_ty],
          [llvm_v2f64_ty, llvm_v2i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttpd2uqq_256 :
        ClangBuiltin<"__builtin_ia32_cvttpd2uqq256_mask">,
          Intrinsic<[llvm_v4i64_ty],
          [llvm_v4f64_ty, llvm_v4i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttpd2uqq_512 :
        ClangBuiltin<"__builtin_ia32_cvttpd2uqq512_mask">,
          Intrinsic<[llvm_v8i64_ty],
          [llvm_v8f64_ty, llvm_v8i64_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvttps2dq_512 :
        ClangBuiltin<"__builtin_ia32_cvttps2dq512_mask">,
          Intrinsic<[llvm_v16i32_ty],
          [llvm_v16f32_ty, llvm_v16i32_ty,  llvm_i16_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvttps2qq_128 :
        ClangBuiltin<"__builtin_ia32_cvttps2qq128_mask">,
          Intrinsic<[llvm_v2i64_ty],
          [llvm_v4f32_ty, llvm_v2i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttps2qq_256 :
        ClangBuiltin<"__builtin_ia32_cvttps2qq256_mask">,
          Intrinsic<[llvm_v4i64_ty],
          [llvm_v4f32_ty, llvm_v4i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttps2qq_512 :
        ClangBuiltin<"__builtin_ia32_cvttps2qq512_mask">,
          Intrinsic<[llvm_v8i64_ty],
          [llvm_v8f32_ty, llvm_v8i64_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvttps2udq_128 :
        ClangBuiltin<"__builtin_ia32_cvttps2udq128_mask">,
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v4f32_ty, llvm_v4i32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttps2udq_256 :
        ClangBuiltin<"__builtin_ia32_cvttps2udq256_mask">,
          Intrinsic<[llvm_v8i32_ty],
          [llvm_v8f32_ty, llvm_v8i32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttps2udq_512 :
        ClangBuiltin<"__builtin_ia32_cvttps2udq512_mask">,
          Intrinsic<[llvm_v16i32_ty],
          [llvm_v16f32_ty, llvm_v16i32_ty,  llvm_i16_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvttps2uqq_128 :
        ClangBuiltin<"__builtin_ia32_cvttps2uqq128_mask">,
          Intrinsic<[llvm_v2i64_ty],
          [llvm_v4f32_ty, llvm_v2i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttps2uqq_256 :
        ClangBuiltin<"__builtin_ia32_cvttps2uqq256_mask">,
          Intrinsic<[llvm_v4i64_ty],
          [llvm_v4f32_ty, llvm_v4i64_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_cvttps2uqq_512 :
        ClangBuiltin<"__builtin_ia32_cvttps2uqq512_mask">,
          Intrinsic<[llvm_v8i64_ty],
          [llvm_v8f32_ty, llvm_v8i64_ty,  llvm_i8_ty,  llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_cvtuqq2ps_128 :
        ClangBuiltin<"__builtin_ia32_cvtuqq2ps128_mask">,
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v2i64_ty, llvm_v4f32_ty,  llvm_i8_ty],
          [IntrNoMem]>;

  def int_x86_avx512_mask_rndscale_pd_128 : ClangBuiltin<"__builtin_ia32_rndscalepd_128_mask">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_i32_ty,
                                     llvm_v2f64_ty,  llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_rndscale_pd_256 : ClangBuiltin<"__builtin_ia32_rndscalepd_256_mask">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_i32_ty,
                                     llvm_v4f64_ty,  llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_rndscale_pd_512 : ClangBuiltin<"__builtin_ia32_rndscalepd_mask">,
        Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_i32_ty, llvm_v8f64_ty,
                                     llvm_i8_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_rndscale_ps_128 : ClangBuiltin<"__builtin_ia32_rndscaleps_128_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_i32_ty,
                                     llvm_v4f32_ty,  llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_rndscale_ps_256 : ClangBuiltin<"__builtin_ia32_rndscaleps_256_mask">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_i32_ty,
                                     llvm_v8f32_ty,  llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_rndscale_ps_512 : ClangBuiltin<"__builtin_ia32_rndscaleps_mask">,
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_i32_ty, llvm_v16f32_ty,
                                     llvm_i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_reduce_pd_128 : ClangBuiltin<"__builtin_ia32_reducepd128_mask">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_i32_ty,
                                     llvm_v2f64_ty,  llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_reduce_pd_256 : ClangBuiltin<"__builtin_ia32_reducepd256_mask">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_i32_ty,
                                     llvm_v4f64_ty,  llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_reduce_pd_512 : ClangBuiltin<"__builtin_ia32_reducepd512_mask">,
        Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_i32_ty, llvm_v8f64_ty,
                                     llvm_i8_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_reduce_ps_128 : ClangBuiltin<"__builtin_ia32_reduceps128_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_i32_ty,
                                     llvm_v4f32_ty,  llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_reduce_ps_256 : ClangBuiltin<"__builtin_ia32_reduceps256_mask">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_i32_ty,
                                     llvm_v8f32_ty,  llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_reduce_ps_512 : ClangBuiltin<"__builtin_ia32_reduceps512_mask">,
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_i32_ty, llvm_v16f32_ty,
                                     llvm_i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<4>>]>;
def int_x86_avx512_mask_range_pd_128 : ClangBuiltin<"__builtin_ia32_rangepd128_mask">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty, llvm_i32_ty,
                                    llvm_v2f64_ty,  llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512_mask_range_pd_256 : ClangBuiltin<"__builtin_ia32_rangepd256_mask">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty, llvm_i32_ty,
                                    llvm_v4f64_ty,  llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512_mask_range_pd_512 : ClangBuiltin<"__builtin_ia32_rangepd512_mask">,
        Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty, llvm_i32_ty,
                                    llvm_v8f64_ty,  llvm_i8_ty,  llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<5>>]>;
def int_x86_avx512_mask_range_ps_128 : ClangBuiltin<"__builtin_ia32_rangeps128_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_i32_ty,
                                    llvm_v4f32_ty,  llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512_mask_range_ps_256 : ClangBuiltin<"__builtin_ia32_rangeps256_mask">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_v8f32_ty, llvm_i32_ty,
                                    llvm_v8f32_ty,  llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512_mask_range_ps_512 : ClangBuiltin<"__builtin_ia32_rangeps512_mask">,
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty, llvm_i32_ty,
                                     llvm_v16f32_ty,  llvm_i16_ty,  llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<5>>]>;
}

// Vector load with broadcast
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".
   def int_x86_avx512_broadcastmw_512 :
          ClangBuiltin<"__builtin_ia32_broadcastmw512">,
          Intrinsic<[llvm_v16i32_ty], [llvm_i16_ty], [IntrNoMem]>;
   def int_x86_avx512_broadcastmw_256 :
          ClangBuiltin<"__builtin_ia32_broadcastmw256">,
          Intrinsic<[llvm_v8i32_ty], [llvm_i16_ty], [IntrNoMem]>;
   def int_x86_avx512_broadcastmw_128 :
          ClangBuiltin<"__builtin_ia32_broadcastmw128">,
          Intrinsic<[llvm_v4i32_ty], [llvm_i16_ty], [IntrNoMem]>;
   def int_x86_avx512_broadcastmb_512 :
          ClangBuiltin<"__builtin_ia32_broadcastmb512">,
          Intrinsic<[llvm_v8i64_ty], [llvm_i8_ty], [IntrNoMem]>;
   def int_x86_avx512_broadcastmb_256 :
          ClangBuiltin<"__builtin_ia32_broadcastmb256">,
          Intrinsic<[llvm_v4i64_ty], [llvm_i8_ty], [IntrNoMem]>;
   def int_x86_avx512_broadcastmb_128 :
          ClangBuiltin<"__builtin_ia32_broadcastmb128">,
          Intrinsic<[llvm_v2i64_ty], [llvm_i8_ty], [IntrNoMem]>;
}

// Arithmetic ops
let TargetPrefix = "x86" in {  // All intrinsics start with "llvm.x86.".

  def int_x86_avx512_add_ps_512 : ClangBuiltin<"__builtin_ia32_addps512">,
          Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                     llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_add_pd_512 : ClangBuiltin<"__builtin_ia32_addpd512">,
          Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                     llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_sub_ps_512 : ClangBuiltin<"__builtin_ia32_subps512">,
          Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                     llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_sub_pd_512 : ClangBuiltin<"__builtin_ia32_subpd512">,
          Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                     llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_mul_ps_512 : ClangBuiltin<"__builtin_ia32_mulps512">,
          Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                     llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_mul_pd_512 : ClangBuiltin<"__builtin_ia32_mulpd512">,
          Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                     llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_div_ps_512 : ClangBuiltin<"__builtin_ia32_divps512">,
          Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                     llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_div_pd_512 : ClangBuiltin<"__builtin_ia32_divpd512">,
          Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                     llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;

  def int_x86_avx512_max_ps_512 : ClangBuiltin<"__builtin_ia32_maxps512">,
          Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                     llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_max_pd_512 : ClangBuiltin<"__builtin_ia32_maxpd512">,
          Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                     llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_min_ps_512 : ClangBuiltin<"__builtin_ia32_minps512">,
          Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                     llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_min_pd_512 : ClangBuiltin<"__builtin_ia32_minpd512">,
          Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                     llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;

  def int_x86_avx512_mask_add_ss_round : ClangBuiltin<"__builtin_ia32_addss_round_mask">,
          Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                     llvm_v4f32_ty, llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_div_ss_round : ClangBuiltin<"__builtin_ia32_divss_round_mask">,
          Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                     llvm_v4f32_ty, llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_mul_ss_round : ClangBuiltin<"__builtin_ia32_mulss_round_mask">,
          Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                     llvm_v4f32_ty, llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_sub_ss_round : ClangBuiltin<"__builtin_ia32_subss_round_mask">,
          Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                     llvm_v4f32_ty, llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_max_ss_round : ClangBuiltin<"__builtin_ia32_maxss_round_mask">,
          Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                     llvm_v4f32_ty, llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_min_ss_round : ClangBuiltin<"__builtin_ia32_minss_round_mask">,
          Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                     llvm_v4f32_ty, llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_add_sd_round : ClangBuiltin<"__builtin_ia32_addsd_round_mask">,
          Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                     llvm_v2f64_ty, llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_div_sd_round : ClangBuiltin<"__builtin_ia32_divsd_round_mask">,
          Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                     llvm_v2f64_ty, llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_mul_sd_round : ClangBuiltin<"__builtin_ia32_mulsd_round_mask">,
          Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                     llvm_v2f64_ty, llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_sub_sd_round : ClangBuiltin<"__builtin_ia32_subsd_round_mask">,
          Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                     llvm_v2f64_ty, llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_max_sd_round : ClangBuiltin<"__builtin_ia32_maxsd_round_mask">,
          Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                     llvm_v2f64_ty, llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_min_sd_round : ClangBuiltin<"__builtin_ia32_minsd_round_mask">,
          Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                     llvm_v2f64_ty, llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_rndscale_ss : ClangBuiltin<"__builtin_ia32_rndscaless_round_mask">,
          Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty,
                                     llvm_i8_ty, llvm_i32_ty, llvm_i32_ty],
                                     [IntrNoMem, ImmArg<ArgIndex<4>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_mask_rndscale_sd : ClangBuiltin<"__builtin_ia32_rndscalesd_round_mask">,
          Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty, llvm_v2f64_ty,
                                      llvm_i8_ty, llvm_i32_ty, llvm_i32_ty],
                                     [IntrNoMem, ImmArg<ArgIndex<4>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_mask_range_ss : ClangBuiltin<"__builtin_ia32_rangess128_round_mask">,
          Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty,
                                     llvm_i8_ty, llvm_i32_ty, llvm_i32_ty],
                                     [IntrNoMem, ImmArg<ArgIndex<4>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_mask_range_sd : ClangBuiltin<"__builtin_ia32_rangesd128_round_mask">,
          Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty, llvm_v2f64_ty,
                                      llvm_i8_ty, llvm_i32_ty, llvm_i32_ty],
                                     [IntrNoMem, ImmArg<ArgIndex<4>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_mask_reduce_ss : ClangBuiltin<"__builtin_ia32_reducess_mask">,
          Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty,
                                     llvm_i8_ty, llvm_i32_ty, llvm_i32_ty],
                                     [IntrNoMem, ImmArg<ArgIndex<4>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_mask_reduce_sd : ClangBuiltin<"__builtin_ia32_reducesd_mask">,
          Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty, llvm_v2f64_ty,
                                      llvm_i8_ty, llvm_i32_ty, llvm_i32_ty],
                                     [IntrNoMem, ImmArg<ArgIndex<4>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_mask_scalef_sd : ClangBuiltin<"__builtin_ia32_scalefsd_round_mask">,
          Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                                      llvm_v2f64_ty, llvm_i8_ty, llvm_i32_ty],
                                     [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_scalef_ss : ClangBuiltin<"__builtin_ia32_scalefss_round_mask">,
          Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                                      llvm_v4f32_ty, llvm_i8_ty, llvm_i32_ty],
                                     [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_scalef_pd_128 : ClangBuiltin<"__builtin_ia32_scalefpd128_mask">,
          Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                    llvm_v2f64_ty, llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_mask_scalef_pd_256 : ClangBuiltin<"__builtin_ia32_scalefpd256_mask">,
          Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty,
                    llvm_v4f64_ty, llvm_i8_ty],[IntrNoMem]>;
  def int_x86_avx512_mask_scalef_pd_512 : ClangBuiltin<"__builtin_ia32_scalefpd512_mask">,
          Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                                      llvm_v8f64_ty, llvm_i8_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_scalef_ps_128 : ClangBuiltin<"__builtin_ia32_scalefps128_mask">,
          Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                    llvm_v4f32_ty, llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_mask_scalef_ps_256 : ClangBuiltin<"__builtin_ia32_scalefps256_mask">,
          Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_v8f32_ty,
                    llvm_v8f32_ty, llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_mask_scalef_ps_512 : ClangBuiltin<"__builtin_ia32_scalefps512_mask">,
          Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                                       llvm_v16f32_ty, llvm_i16_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_sqrt_ss :
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty,
                                    llvm_i8_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_sqrt_sd :
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty, llvm_v2f64_ty,
                                    llvm_i8_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_sqrt_pd_512 :
        Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_sqrt_ps_512 :
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_mask_fixupimm_pd_128 :
         ClangBuiltin<"__builtin_ia32_fixupimmpd128_mask">,
          Intrinsic<[llvm_v2f64_ty],
          [llvm_v2f64_ty, llvm_v2f64_ty, llvm_v2i64_ty, llvm_i32_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_maskz_fixupimm_pd_128 :
         ClangBuiltin<"__builtin_ia32_fixupimmpd128_maskz">,
          Intrinsic<[llvm_v2f64_ty],
          [llvm_v2f64_ty, llvm_v2f64_ty, llvm_v2i64_ty, llvm_i32_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_mask_fixupimm_pd_256 :
         ClangBuiltin<"__builtin_ia32_fixupimmpd256_mask">,
          Intrinsic<[llvm_v4f64_ty],
          [llvm_v4f64_ty, llvm_v4f64_ty, llvm_v4i64_ty, llvm_i32_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_maskz_fixupimm_pd_256 :
         ClangBuiltin<"__builtin_ia32_fixupimmpd256_maskz">,
          Intrinsic<[llvm_v4f64_ty],
          [llvm_v4f64_ty, llvm_v4f64_ty, llvm_v4i64_ty, llvm_i32_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_mask_fixupimm_pd_512 :
         ClangBuiltin<"__builtin_ia32_fixupimmpd512_mask">,
          Intrinsic<[llvm_v8f64_ty],
          [llvm_v8f64_ty, llvm_v8f64_ty, llvm_v8i64_ty, llvm_i32_ty, llvm_i8_ty,
          llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_maskz_fixupimm_pd_512 :
         ClangBuiltin<"__builtin_ia32_fixupimmpd512_maskz">,
          Intrinsic<[llvm_v8f64_ty],
          [llvm_v8f64_ty, llvm_v8f64_ty, llvm_v8i64_ty, llvm_i32_ty, llvm_i8_ty,
          llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_mask_fixupimm_ps_128 :
         ClangBuiltin<"__builtin_ia32_fixupimmps128_mask">,
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4i32_ty, llvm_i32_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_maskz_fixupimm_ps_128 :
         ClangBuiltin<"__builtin_ia32_fixupimmps128_maskz">,
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4i32_ty, llvm_i32_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_mask_fixupimm_ps_256 :
         ClangBuiltin<"__builtin_ia32_fixupimmps256_mask">,
          Intrinsic<[llvm_v8f32_ty],
          [llvm_v8f32_ty, llvm_v8f32_ty, llvm_v8i32_ty, llvm_i32_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_maskz_fixupimm_ps_256 :
         ClangBuiltin<"__builtin_ia32_fixupimmps256_maskz">,
          Intrinsic<[llvm_v8f32_ty],
          [llvm_v8f32_ty, llvm_v8f32_ty, llvm_v8i32_ty, llvm_i32_ty, llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_mask_fixupimm_ps_512 :
         ClangBuiltin<"__builtin_ia32_fixupimmps512_mask">,
          Intrinsic<[llvm_v16f32_ty],
          [llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16i32_ty, llvm_i32_ty,
          llvm_i16_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_maskz_fixupimm_ps_512 :
         ClangBuiltin<"__builtin_ia32_fixupimmps512_maskz">,
          Intrinsic<[llvm_v16f32_ty],
          [llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16i32_ty, llvm_i32_ty,
          llvm_i16_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_mask_fixupimm_sd :
         ClangBuiltin<"__builtin_ia32_fixupimmsd_mask">,
          Intrinsic<[llvm_v2f64_ty],
          [llvm_v2f64_ty, llvm_v2f64_ty, llvm_v2i64_ty, llvm_i32_ty, llvm_i8_ty,
          llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_maskz_fixupimm_sd :
         ClangBuiltin<"__builtin_ia32_fixupimmsd_maskz">,
          Intrinsic<[llvm_v2f64_ty],
          [llvm_v2f64_ty, llvm_v2f64_ty, llvm_v2i64_ty, llvm_i32_ty, llvm_i8_ty,
          llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_mask_fixupimm_ss :
         ClangBuiltin<"__builtin_ia32_fixupimmss_mask">,
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4i32_ty, llvm_i32_ty, llvm_i8_ty,
          llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_maskz_fixupimm_ss :
         ClangBuiltin<"__builtin_ia32_fixupimmss_maskz">,
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4i32_ty, llvm_i32_ty, llvm_i8_ty,
          llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<5>>]>;
  def int_x86_avx512_mask_getexp_pd_128 : ClangBuiltin<"__builtin_ia32_getexppd128_mask">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                                    llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_mask_getexp_pd_256 : ClangBuiltin<"__builtin_ia32_getexppd256_mask">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty,
                                    llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_mask_getexp_pd_512 : ClangBuiltin<"__builtin_ia32_getexppd512_mask">,
        Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                                    llvm_i8_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_mask_getexp_ps_128 : ClangBuiltin<"__builtin_ia32_getexpps128_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                                     llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_mask_getexp_ps_256 : ClangBuiltin<"__builtin_ia32_getexpps256_mask">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_v8f32_ty,
                                     llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_mask_getexp_ps_512 : ClangBuiltin<"__builtin_ia32_getexpps512_mask">,
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                                     llvm_i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_mask_getexp_ss : ClangBuiltin<"__builtin_ia32_getexpss128_round_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty,
                                    llvm_i8_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_getexp_sd : ClangBuiltin<"__builtin_ia32_getexpsd128_round_mask">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty, llvm_v2f64_ty,
                                    llvm_i8_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_getmant_pd_128 :
         ClangBuiltin<"__builtin_ia32_getmantpd128_mask">,
          Intrinsic<[llvm_v2f64_ty],
          [llvm_v2f64_ty,llvm_i32_ty, llvm_v2f64_ty,  llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<1>>]>;

  def int_x86_avx512_mask_getmant_pd_256 :
         ClangBuiltin<"__builtin_ia32_getmantpd256_mask">,
          Intrinsic<[llvm_v4f64_ty],
          [llvm_v4f64_ty,llvm_i32_ty, llvm_v4f64_ty,  llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<1>>]>;

  def int_x86_avx512_mask_getmant_pd_512 :
         ClangBuiltin<"__builtin_ia32_getmantpd512_mask">,
          Intrinsic<[llvm_v8f64_ty],
          [llvm_v8f64_ty,llvm_i32_ty, llvm_v8f64_ty,  llvm_i8_ty,llvm_i32_ty ],
          [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_getmant_ps_128 :
         ClangBuiltin<"__builtin_ia32_getmantps128_mask">,
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v4f32_ty, llvm_i32_ty, llvm_v4f32_ty,  llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<1>>]>;

  def int_x86_avx512_mask_getmant_ps_256 :
         ClangBuiltin<"__builtin_ia32_getmantps256_mask">,
          Intrinsic<[llvm_v8f32_ty],
          [llvm_v8f32_ty, llvm_i32_ty, llvm_v8f32_ty,  llvm_i8_ty],
          [IntrNoMem, ImmArg<ArgIndex<1>>]>;

  def int_x86_avx512_mask_getmant_ps_512 :
         ClangBuiltin<"__builtin_ia32_getmantps512_mask">,
          Intrinsic<[llvm_v16f32_ty],
          [llvm_v16f32_ty,llvm_i32_ty, llvm_v16f32_ty,llvm_i16_ty,llvm_i32_ty],
          [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_getmant_ss :
         ClangBuiltin<"__builtin_ia32_getmantss_round_mask">,
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v4f32_ty, llvm_v4f32_ty, llvm_i32_ty, llvm_v4f32_ty,
           llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<5>>]>;

  def int_x86_avx512_mask_getmant_sd :
         ClangBuiltin<"__builtin_ia32_getmantsd_round_mask">,
          Intrinsic<[llvm_v2f64_ty],
          [llvm_v2f64_ty, llvm_v2f64_ty, llvm_i32_ty, llvm_v2f64_ty,
           llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<5>>]>;

  def int_x86_avx512_rsqrt14_ss : ClangBuiltin<"__builtin_ia32_rsqrt14ss_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty,
                                    llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_rsqrt14_sd : ClangBuiltin<"__builtin_ia32_rsqrt14sd_mask">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty, llvm_v2f64_ty,
                                    llvm_i8_ty], [IntrNoMem]>;

  def int_x86_avx512_rsqrt14_pd_128 : ClangBuiltin<"__builtin_ia32_rsqrt14pd128_mask">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                                    llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_rsqrt14_pd_256 : ClangBuiltin<"__builtin_ia32_rsqrt14pd256_mask">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty,
                                    llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_rsqrt14_pd_512 : ClangBuiltin<"__builtin_ia32_rsqrt14pd512_mask">,
        Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                                    llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_rsqrt14_ps_128 : ClangBuiltin<"__builtin_ia32_rsqrt14ps128_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                                     llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_rsqrt14_ps_256 : ClangBuiltin<"__builtin_ia32_rsqrt14ps256_mask">,
          Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_v8f32_ty,
                                     llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_rsqrt14_ps_512 : ClangBuiltin<"__builtin_ia32_rsqrt14ps512_mask">,
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                                     llvm_i16_ty], [IntrNoMem]>;
  def int_x86_avx512_rcp14_ss : ClangBuiltin<"__builtin_ia32_rcp14ss_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty,
                                    llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_rcp14_sd : ClangBuiltin<"__builtin_ia32_rcp14sd_mask">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty, llvm_v2f64_ty,
                                    llvm_i8_ty], [IntrNoMem]>;

  def int_x86_avx512_rcp14_pd_128 : ClangBuiltin<"__builtin_ia32_rcp14pd128_mask">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                                    llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_rcp14_pd_256 : ClangBuiltin<"__builtin_ia32_rcp14pd256_mask">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty,
                                    llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_rcp14_pd_512 : ClangBuiltin<"__builtin_ia32_rcp14pd512_mask">,
        Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                                    llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_rcp14_ps_128 : ClangBuiltin<"__builtin_ia32_rcp14ps128_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                                     llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_rcp14_ps_256 : ClangBuiltin<"__builtin_ia32_rcp14ps256_mask">,
          Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_v8f32_ty,
                                     llvm_i8_ty], [IntrNoMem]>;
  def int_x86_avx512_rcp14_ps_512 : ClangBuiltin<"__builtin_ia32_rcp14ps512_mask">,
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                                     llvm_i16_ty], [IntrNoMem]>;

  def int_x86_avx512_rcp28_ps : ClangBuiltin<"__builtin_ia32_rcp28ps_mask">,
            Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                                         llvm_i16_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_rcp28_pd : ClangBuiltin<"__builtin_ia32_rcp28pd_mask">,
            Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                                        llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_exp2_ps : ClangBuiltin<"__builtin_ia32_exp2ps_mask">,
            Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                                         llvm_i16_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_exp2_pd : ClangBuiltin<"__builtin_ia32_exp2pd_mask">,
            Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                                        llvm_i8_ty, llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_rcp28_ss : ClangBuiltin<"__builtin_ia32_rcp28ss_round_mask">,
            Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                                        llvm_v4f32_ty, llvm_i8_ty, llvm_i32_ty],
                      [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_rcp28_sd : ClangBuiltin<"__builtin_ia32_rcp28sd_round_mask">,
            Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                                        llvm_v2f64_ty, llvm_i8_ty, llvm_i32_ty],
                      [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_rsqrt28_ps : ClangBuiltin<"__builtin_ia32_rsqrt28ps_mask">,
            Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                                         llvm_i16_ty, llvm_i32_ty],
                      [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_rsqrt28_pd : ClangBuiltin<"__builtin_ia32_rsqrt28pd_mask">,
            Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                                        llvm_i8_ty, llvm_i32_ty],
                      [IntrNoMem, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_rsqrt28_ss : ClangBuiltin<"__builtin_ia32_rsqrt28ss_round_mask">,
            Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                                        llvm_v4f32_ty, llvm_i8_ty, llvm_i32_ty],
                      [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_rsqrt28_sd : ClangBuiltin<"__builtin_ia32_rsqrt28sd_round_mask">,
            Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                                        llvm_v2f64_ty, llvm_i8_ty, llvm_i32_ty],
                      [IntrNoMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_psad_bw_512 : ClangBuiltin<"__builtin_ia32_psadbw512">,
              Intrinsic<[llvm_v8i64_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                        [IntrNoMem, Commutative]>;
}
// Integer arithmetic ops
let TargetPrefix = "x86" in {
  def int_x86_avx512_pmulhu_w_512 : ClangBuiltin<"__builtin_ia32_pmulhuw512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty,
                         llvm_v32i16_ty], [IntrNoMem, Commutative]>;
  def int_x86_avx512_pmulh_w_512 : ClangBuiltin<"__builtin_ia32_pmulhw512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty,
                         llvm_v32i16_ty], [IntrNoMem, Commutative]>;
  def int_x86_avx512_pavg_b_512 : ClangBuiltin<"__builtin_ia32_pavgb512">,
          Intrinsic<[llvm_v64i8_ty], [llvm_v64i8_ty, llvm_v64i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_pavg_w_512 : ClangBuiltin<"__builtin_ia32_pavgw512">,
          Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_pmaddw_d_512 : ClangBuiltin<"__builtin_ia32_pmaddwd512">,
              Intrinsic<[llvm_v16i32_ty], [llvm_v32i16_ty,
                         llvm_v32i16_ty], [IntrNoMem, Commutative]>;
  def int_x86_avx512_pmaddubs_w_512 : ClangBuiltin<"__builtin_ia32_pmaddubsw512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v64i8_ty,
                         llvm_v64i8_ty], [IntrNoMem]>;

  def int_x86_avx512_dbpsadbw_128 :
         ClangBuiltin<"__builtin_ia32_dbpsadbw128">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v16i8_ty, llvm_v16i8_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<2>>]>;

  def int_x86_avx512_dbpsadbw_256 :
         ClangBuiltin<"__builtin_ia32_dbpsadbw256">,
          Intrinsic<[llvm_v16i16_ty],
                    [llvm_v32i8_ty, llvm_v32i8_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<2>>]>;

  def int_x86_avx512_dbpsadbw_512 :
         ClangBuiltin<"__builtin_ia32_dbpsadbw512">,
          Intrinsic<[llvm_v32i16_ty],
                    [llvm_v64i8_ty, llvm_v64i8_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<2>>]>;
}

// Gather and Scatter ops
let TargetPrefix = "x86" in {
  // NOTE: These are deprecated in favor of the versions that take a vXi1 mask.
  // NOTE: These can't be ArgMemOnly because you can put the address completely
  // in the index register.
  def int_x86_avx512_gather_dpd_512  :
          Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_ptr_ty,
                     llvm_v8i32_ty, llvm_i8_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_gather_dps_512  :
          Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_ptr_ty,
                     llvm_v16i32_ty, llvm_i16_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_gather_qpd_512  :
          Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_ptr_ty,
                     llvm_v8i64_ty, llvm_i8_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_gather_qps_512  :
          Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_ptr_ty,
                     llvm_v8i64_ty, llvm_i8_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;


  def int_x86_avx512_gather_dpq_512  :
          Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_ptr_ty,
                     llvm_v8i32_ty, llvm_i8_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_gather_dpi_512  :
          Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_ptr_ty,
                     llvm_v16i32_ty, llvm_i16_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_gather_qpq_512  :
          Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_ptr_ty,
                     llvm_v8i64_ty, llvm_i8_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_gather_qpi_512  :
          Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_ptr_ty,
                     llvm_v8i64_ty, llvm_i8_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3div2_df :
          Intrinsic<[llvm_v2f64_ty],
          [llvm_v2f64_ty, llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3div2_di :
          Intrinsic<[llvm_v2i64_ty],
          [llvm_v2i64_ty, llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3div4_df :
          Intrinsic<[llvm_v4f64_ty],
          [llvm_v4f64_ty, llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3div4_di :
          Intrinsic<[llvm_v4i64_ty],
          [llvm_v4i64_ty, llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3div4_sf :
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v4f32_ty, llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3div4_si :
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v4i32_ty, llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3div8_sf :
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v4f32_ty, llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3div8_si :
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v4i32_ty, llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3siv2_df :
          Intrinsic<[llvm_v2f64_ty],
          [llvm_v2f64_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3siv2_di :
          Intrinsic<[llvm_v2i64_ty],
          [llvm_v2i64_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3siv4_df :
          Intrinsic<[llvm_v4f64_ty],
          [llvm_v4f64_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3siv4_di :
          Intrinsic<[llvm_v4i64_ty],
          [llvm_v4i64_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3siv4_sf :
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v4f32_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3siv4_si :
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v4i32_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3siv8_sf :
          Intrinsic<[llvm_v8f32_ty],
          [llvm_v8f32_ty, llvm_ptr_ty, llvm_v8i32_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_gather3siv8_si :
          Intrinsic<[llvm_v8i32_ty],
          [llvm_v8i32_ty, llvm_ptr_ty, llvm_v8i32_ty, llvm_i8_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

// scatter
  // NOTE: These are deprecated in favor of the versions that take a vXi1 mask.
  // NOTE: These can't be ArgMemOnly because you can put the address completely
  // in the index register.
  def int_x86_avx512_scatter_dpd_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_i8_ty,
                        llvm_v8i32_ty, llvm_v8f64_ty, llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_scatter_dps_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_i16_ty,
                       llvm_v16i32_ty, llvm_v16f32_ty, llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_scatter_qpd_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_i8_ty,
                     llvm_v8i64_ty, llvm_v8f64_ty, llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_scatter_qps_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_i8_ty,
                     llvm_v8i64_ty, llvm_v8f32_ty, llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;


  def int_x86_avx512_scatter_dpq_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_i8_ty,
                         llvm_v8i32_ty, llvm_v8i64_ty, llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_scatter_dpi_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_i16_ty,
                     llvm_v16i32_ty, llvm_v16i32_ty, llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_scatter_qpq_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_i8_ty,llvm_v8i64_ty, llvm_v8i64_ty,
                         llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_scatter_qpi_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_i8_ty, llvm_v8i64_ty, llvm_v8i32_ty,
                         llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scatterdiv2_df :
        Intrinsic<[],
        [llvm_ptr_ty, llvm_i8_ty, llvm_v2i64_ty, llvm_v2f64_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scatterdiv2_di :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v2i64_ty, llvm_v2i64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scatterdiv4_df :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v4i64_ty, llvm_v4f64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scatterdiv4_di :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v4i64_ty, llvm_v4i64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scatterdiv4_sf :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v2i64_ty, llvm_v4f32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scatterdiv4_si :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v2i64_ty, llvm_v4i32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scatterdiv8_sf :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v4i64_ty, llvm_v4f32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scatterdiv8_si :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v4i64_ty, llvm_v4i32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scattersiv2_df :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v4i32_ty, llvm_v2f64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scattersiv2_di :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v4i32_ty, llvm_v2i64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scattersiv4_df :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v4i32_ty, llvm_v4f64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scattersiv4_di :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v4i32_ty, llvm_v4i64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scattersiv4_sf :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v4i32_ty, llvm_v4f32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scattersiv4_si :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v4i32_ty, llvm_v4i32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scattersiv8_sf :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v8i32_ty, llvm_v8f32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_scattersiv8_si :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_i8_ty, llvm_v8i32_ty, llvm_v8i32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  // gather prefetch
  // NOTE: These can't be ArgMemOnly because you can put the address completely
  // in the index register.
  def int_x86_avx512_gatherpf_dpd_512  : ClangBuiltin<"__builtin_ia32_gatherpfdpd">,
          Intrinsic<[], [llvm_i8_ty, llvm_v8i32_ty, llvm_ptr_ty,
                     llvm_i32_ty, llvm_i32_ty], [ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_gatherpf_dps_512  : ClangBuiltin<"__builtin_ia32_gatherpfdps">,
          Intrinsic<[], [llvm_i16_ty, llvm_v16i32_ty, llvm_ptr_ty,
                     llvm_i32_ty, llvm_i32_ty], [ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_gatherpf_qpd_512  : ClangBuiltin<"__builtin_ia32_gatherpfqpd">,
          Intrinsic<[], [llvm_i8_ty, llvm_v8i64_ty, llvm_ptr_ty,
                     llvm_i32_ty, llvm_i32_ty], [ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_gatherpf_qps_512  : ClangBuiltin<"__builtin_ia32_gatherpfqps">,
          Intrinsic<[], [llvm_i8_ty, llvm_v8i64_ty, llvm_ptr_ty,
                     llvm_i32_ty, llvm_i32_ty], [ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;

  // scatter prefetch
  // NOTE: These can't be ArgMemOnly because you can put the address completely
  // in the index register.
  def int_x86_avx512_scatterpf_dpd_512  : ClangBuiltin<"__builtin_ia32_scatterpfdpd">,
          Intrinsic<[], [llvm_i8_ty, llvm_v8i32_ty, llvm_ptr_ty,
                     llvm_i32_ty, llvm_i32_ty], [ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_scatterpf_dps_512  : ClangBuiltin<"__builtin_ia32_scatterpfdps">,
          Intrinsic<[], [llvm_i16_ty, llvm_v16i32_ty, llvm_ptr_ty,
                     llvm_i32_ty, llvm_i32_ty], [ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_scatterpf_qpd_512  : ClangBuiltin<"__builtin_ia32_scatterpfqpd">,
          Intrinsic<[], [llvm_i8_ty, llvm_v8i64_ty, llvm_ptr_ty,
                     llvm_i32_ty, llvm_i32_ty], [ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_scatterpf_qps_512  : ClangBuiltin<"__builtin_ia32_scatterpfqps">,
          Intrinsic<[], [llvm_i8_ty, llvm_v8i64_ty, llvm_ptr_ty,
                     llvm_i32_ty, llvm_i32_ty], [ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
}

// AVX512 gather/scatter intrinsics that use vXi1 masks.
let TargetPrefix = "x86" in {
  // NOTE: These can't be ArgMemOnly because you can put the address completely
  // in the index register.
  def int_x86_avx512_mask_gather_dpd_512  :
          Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_ptr_ty,
                     llvm_v8i32_ty, llvm_v8i1_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_gather_dps_512  :
          Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_ptr_ty,
                     llvm_v16i32_ty, llvm_v16i1_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_gather_qpd_512  :
          Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_ptr_ty,
                     llvm_v8i64_ty, llvm_v8i1_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_gather_qps_512  :
          Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_ptr_ty,
                     llvm_v8i64_ty, llvm_v8i1_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;


  def int_x86_avx512_mask_gather_dpq_512  :
          Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_ptr_ty,
                     llvm_v8i32_ty, llvm_v8i1_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_gather_dpi_512  :
          Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_ptr_ty,
                     llvm_v16i32_ty, llvm_v16i1_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_gather_qpq_512  :
          Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty, llvm_ptr_ty,
                     llvm_v8i64_ty, llvm_v8i1_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_gather_qpi_512  :
          Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_ptr_ty,
                     llvm_v8i64_ty, llvm_v8i1_ty, llvm_i32_ty],
                    [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3div2_df :
          Intrinsic<[llvm_v2f64_ty],
          [llvm_v2f64_ty, llvm_ptr_ty, llvm_v2i64_ty, llvm_v2i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3div2_di :
          Intrinsic<[llvm_v2i64_ty],
          [llvm_v2i64_ty, llvm_ptr_ty, llvm_v2i64_ty, llvm_v2i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3div4_df :
          Intrinsic<[llvm_v4f64_ty],
          [llvm_v4f64_ty, llvm_ptr_ty, llvm_v4i64_ty, llvm_v4i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3div4_di :
          Intrinsic<[llvm_v4i64_ty],
          [llvm_v4i64_ty, llvm_ptr_ty, llvm_v4i64_ty, llvm_v4i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3div4_sf :
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v4f32_ty, llvm_ptr_ty, llvm_v2i64_ty, llvm_v2i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3div4_si :
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v4i32_ty, llvm_ptr_ty, llvm_v2i64_ty, llvm_v2i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3div8_sf :
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v4f32_ty, llvm_ptr_ty, llvm_v4i64_ty, llvm_v4i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3div8_si :
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v4i32_ty, llvm_ptr_ty, llvm_v4i64_ty, llvm_v4i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3siv2_df :
          Intrinsic<[llvm_v2f64_ty],
          [llvm_v2f64_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_v2i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3siv2_di :
          Intrinsic<[llvm_v2i64_ty],
          [llvm_v2i64_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_v2i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3siv4_df :
          Intrinsic<[llvm_v4f64_ty],
          [llvm_v4f64_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_v4i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3siv4_di :
          Intrinsic<[llvm_v4i64_ty],
          [llvm_v4i64_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_v4i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3siv4_sf :
          Intrinsic<[llvm_v4f32_ty],
          [llvm_v4f32_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_v4i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3siv4_si :
          Intrinsic<[llvm_v4i32_ty],
          [llvm_v4i32_ty, llvm_ptr_ty, llvm_v4i32_ty, llvm_v4i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3siv8_sf :
          Intrinsic<[llvm_v8f32_ty],
          [llvm_v8f32_ty, llvm_ptr_ty, llvm_v8i32_ty, llvm_v8i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_gather3siv8_si :
          Intrinsic<[llvm_v8i32_ty],
          [llvm_v8i32_ty, llvm_ptr_ty, llvm_v8i32_ty, llvm_v8i1_ty, llvm_i32_ty],
          [IntrReadMem, ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scatter_dpd_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_v8i1_ty,
                        llvm_v8i32_ty, llvm_v8f64_ty, llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_scatter_dps_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_v16i1_ty,
                       llvm_v16i32_ty, llvm_v16f32_ty, llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_scatter_qpd_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_v8i1_ty,
                     llvm_v8i64_ty, llvm_v8f64_ty, llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_scatter_qps_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_v8i1_ty,
                     llvm_v8i64_ty, llvm_v8f32_ty, llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;


  // NOTE: These can't be ArgMemOnly because you can put the address completely
  // in the index register.
  def int_x86_avx512_mask_scatter_dpq_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_v8i1_ty,
                         llvm_v8i32_ty, llvm_v8i64_ty, llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_scatter_dpi_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_v16i1_ty,
                     llvm_v16i32_ty, llvm_v16i32_ty, llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_scatter_qpq_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_v8i1_ty,llvm_v8i64_ty, llvm_v8i64_ty,
                         llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_scatter_qpi_512  :
          Intrinsic<[], [llvm_ptr_ty, llvm_v8i1_ty, llvm_v8i64_ty, llvm_v8i32_ty,
                         llvm_i32_ty],
                    [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scatterdiv2_df :
        Intrinsic<[],
        [llvm_ptr_ty, llvm_v2i1_ty, llvm_v2i64_ty, llvm_v2f64_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scatterdiv2_di :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v2i1_ty, llvm_v2i64_ty, llvm_v2i64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scatterdiv4_df :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v4i1_ty, llvm_v4i64_ty, llvm_v4f64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scatterdiv4_di :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v4i1_ty, llvm_v4i64_ty, llvm_v4i64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scatterdiv4_sf :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v2i1_ty, llvm_v2i64_ty, llvm_v4f32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scatterdiv4_si :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v2i1_ty, llvm_v2i64_ty, llvm_v4i32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scatterdiv8_sf :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v4i1_ty, llvm_v4i64_ty, llvm_v4f32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scatterdiv8_si :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v4i1_ty, llvm_v4i64_ty, llvm_v4i32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scattersiv2_df :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v2i1_ty, llvm_v4i32_ty, llvm_v2f64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scattersiv2_di :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v2i1_ty, llvm_v4i32_ty, llvm_v2i64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scattersiv4_df :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v4i1_ty, llvm_v4i32_ty, llvm_v4f64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scattersiv4_di :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v4i1_ty, llvm_v4i32_ty, llvm_v4i64_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scattersiv4_sf :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v4i1_ty, llvm_v4i32_ty, llvm_v4f32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scattersiv4_si :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v4i1_ty, llvm_v4i32_ty, llvm_v4i32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scattersiv8_sf :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v8i1_ty, llvm_v8i32_ty, llvm_v8f32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;

  def int_x86_avx512_mask_scattersiv8_si :
          Intrinsic<[],
          [llvm_ptr_ty, llvm_v8i1_ty, llvm_v8i32_ty, llvm_v8i32_ty, llvm_i32_ty],
          [ImmArg<ArgIndex<4>>]>;
}

// AVX-512 conflict detection instruction
// Instructions that count the number of leading zero bits
let TargetPrefix = "x86" in {
  def int_x86_avx512_conflict_d_128 :
          ClangBuiltin<"__builtin_ia32_vpconflictsi_128">,
          Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_avx512_conflict_d_256 :
          ClangBuiltin<"__builtin_ia32_vpconflictsi_256">,
          Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty], [IntrNoMem]>;
  def int_x86_avx512_conflict_d_512 :
          ClangBuiltin<"__builtin_ia32_vpconflictsi_512">,
          Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty], [IntrNoMem]>;

  def int_x86_avx512_conflict_q_128 :
          ClangBuiltin<"__builtin_ia32_vpconflictdi_128">,
          Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty], [IntrNoMem]>;
  def int_x86_avx512_conflict_q_256 :
          ClangBuiltin<"__builtin_ia32_vpconflictdi_256">,
          Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty], [IntrNoMem]>;
  def int_x86_avx512_conflict_q_512 :
          ClangBuiltin<"__builtin_ia32_vpconflictdi_512">,
          Intrinsic<[llvm_v8i64_ty], [llvm_v8i64_ty], [IntrNoMem]>;
}

// Compares
let TargetPrefix = "x86" in {
  // 512-bit
  def int_x86_avx512_vcomi_sd : ClangBuiltin<"__builtin_ia32_vcomisd">,
              Intrinsic<[llvm_i32_ty], [llvm_v2f64_ty,
                         llvm_v2f64_ty, llvm_i32_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
  def int_x86_avx512_vcomi_ss : ClangBuiltin<"__builtin_ia32_vcomiss">,
              Intrinsic<[llvm_i32_ty], [llvm_v4f32_ty,
                         llvm_v4f32_ty, llvm_i32_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
}

// Compress, Expand
let TargetPrefix = "x86" in {
  def int_x86_avx512_mask_compress :
        Intrinsic<[llvm_anyvector_ty],
                  [LLVMMatchType<0>, LLVMMatchType<0>,
                   LLVMScalarOrSameVectorWidth<0, llvm_i1_ty>],
                  [IntrNoMem]>;
  def int_x86_avx512_mask_expand :
        Intrinsic<[llvm_anyvector_ty],
                  [LLVMMatchType<0>, LLVMMatchType<0>,
                   LLVMScalarOrSameVectorWidth<0, llvm_i1_ty>],
                  [IntrNoMem]>;
}

// truncate
let TargetPrefix = "x86" in {
  def int_x86_avx512_mask_pmov_qb_128 :
          ClangBuiltin<"__builtin_ia32_pmovqb128_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v2i64_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_qb_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovqb128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_qb_128 :
          ClangBuiltin<"__builtin_ia32_pmovsqb128_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v2i64_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_qb_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovsqb128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_qb_128 :
          ClangBuiltin<"__builtin_ia32_pmovusqb128_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v2i64_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_qb_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovusqb128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_qb_256 :
          ClangBuiltin<"__builtin_ia32_pmovqb256_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v4i64_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_qb_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovqb256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_qb_256 :
          ClangBuiltin<"__builtin_ia32_pmovsqb256_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v4i64_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_qb_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovsqb256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_qb_256 :
          ClangBuiltin<"__builtin_ia32_pmovusqb256_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v4i64_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_qb_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovusqb256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_qb_512 :
          ClangBuiltin<"__builtin_ia32_pmovqb512_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v8i64_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_qb_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovqb512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_qb_512 :
          ClangBuiltin<"__builtin_ia32_pmovsqb512_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v8i64_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_qb_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovsqb512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_qb_512 :
          ClangBuiltin<"__builtin_ia32_pmovusqb512_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v8i64_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_qb_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovusqb512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_qw_128 :
          ClangBuiltin<"__builtin_ia32_pmovqw128_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v2i64_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_qw_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovqw128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_qw_128 :
          ClangBuiltin<"__builtin_ia32_pmovsqw128_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v2i64_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_qw_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovsqw128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_qw_128 :
          ClangBuiltin<"__builtin_ia32_pmovusqw128_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v2i64_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_qw_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovusqw128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_qw_256 :
          ClangBuiltin<"__builtin_ia32_pmovqw256_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v4i64_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_qw_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovqw256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_qw_256 :
          ClangBuiltin<"__builtin_ia32_pmovsqw256_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v4i64_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_qw_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovsqw256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_qw_256 :
          ClangBuiltin<"__builtin_ia32_pmovusqw256_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v4i64_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_qw_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovusqw256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_qw_512 :
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v8i64_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_qw_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovqw512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_qw_512 :
          ClangBuiltin<"__builtin_ia32_pmovsqw512_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v8i64_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_qw_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovsqw512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_qw_512 :
          ClangBuiltin<"__builtin_ia32_pmovusqw512_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v8i64_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_qw_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovusqw512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_qd_128 :
          ClangBuiltin<"__builtin_ia32_pmovqd128_mask">,
          Intrinsic<[llvm_v4i32_ty],
                    [llvm_v2i64_ty, llvm_v4i32_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_qd_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovqd128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_qd_128 :
          ClangBuiltin<"__builtin_ia32_pmovsqd128_mask">,
          Intrinsic<[llvm_v4i32_ty],
                    [llvm_v2i64_ty, llvm_v4i32_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_qd_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovsqd128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_qd_128 :
          ClangBuiltin<"__builtin_ia32_pmovusqd128_mask">,
          Intrinsic<[llvm_v4i32_ty],
                    [llvm_v2i64_ty, llvm_v4i32_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_qd_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovusqd128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_qd_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovqd256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_qd_256 :
          ClangBuiltin<"__builtin_ia32_pmovsqd256_mask">,
          Intrinsic<[llvm_v4i32_ty],
                    [llvm_v4i64_ty, llvm_v4i32_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_qd_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovsqd256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_qd_256 :
          ClangBuiltin<"__builtin_ia32_pmovusqd256_mask">,
          Intrinsic<[llvm_v4i32_ty],
                    [llvm_v4i64_ty, llvm_v4i32_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_qd_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovusqd256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_qd_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovqd512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_qd_512 :
          ClangBuiltin<"__builtin_ia32_pmovsqd512_mask">,
          Intrinsic<[llvm_v8i32_ty],
                    [llvm_v8i64_ty, llvm_v8i32_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_qd_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovsqd512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_qd_512 :
          ClangBuiltin<"__builtin_ia32_pmovusqd512_mask">,
          Intrinsic<[llvm_v8i32_ty],
                    [llvm_v8i64_ty, llvm_v8i32_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_qd_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovusqd512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i64_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_db_128 :
          ClangBuiltin<"__builtin_ia32_pmovdb128_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v4i32_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_db_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovdb128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_db_128 :
          ClangBuiltin<"__builtin_ia32_pmovsdb128_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v4i32_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_db_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovsdb128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_db_128 :
          ClangBuiltin<"__builtin_ia32_pmovusdb128_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v4i32_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_db_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovusdb128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_db_256 :
          ClangBuiltin<"__builtin_ia32_pmovdb256_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v8i32_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_db_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovdb256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i32_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_db_256 :
          ClangBuiltin<"__builtin_ia32_pmovsdb256_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v8i32_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_db_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovsdb256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i32_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_db_256 :
          ClangBuiltin<"__builtin_ia32_pmovusdb256_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v8i32_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_db_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovusdb256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i32_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_db_512 :
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v16i32_ty, llvm_v16i8_ty, llvm_i16_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_db_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovdb512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v16i32_ty, llvm_i16_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_db_512 :
          ClangBuiltin<"__builtin_ia32_pmovsdb512_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v16i32_ty, llvm_v16i8_ty, llvm_i16_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_db_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovsdb512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v16i32_ty, llvm_i16_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_db_512 :
          ClangBuiltin<"__builtin_ia32_pmovusdb512_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v16i32_ty, llvm_v16i8_ty, llvm_i16_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_db_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovusdb512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v16i32_ty, llvm_i16_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_dw_128 :
          ClangBuiltin<"__builtin_ia32_pmovdw128_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v4i32_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_dw_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovdw128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_dw_128 :
          ClangBuiltin<"__builtin_ia32_pmovsdw128_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v4i32_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_dw_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovsdw128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_dw_128 :
          ClangBuiltin<"__builtin_ia32_pmovusdw128_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v4i32_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_dw_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovusdw128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_dw_256 :
          ClangBuiltin<"__builtin_ia32_pmovdw256_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v8i32_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_dw_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovdw256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i32_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_dw_256 :
          ClangBuiltin<"__builtin_ia32_pmovsdw256_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v8i32_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_dw_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovsdw256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i32_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_dw_256 :
          ClangBuiltin<"__builtin_ia32_pmovusdw256_mask">,
          Intrinsic<[llvm_v8i16_ty],
                    [llvm_v8i32_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_dw_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovusdw256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i32_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_dw_512 :
          Intrinsic<[llvm_v16i16_ty],
                    [llvm_v16i32_ty, llvm_v16i16_ty, llvm_i16_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_dw_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovdw512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v16i32_ty, llvm_i16_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_dw_512 :
          ClangBuiltin<"__builtin_ia32_pmovsdw512_mask">,
          Intrinsic<[llvm_v16i16_ty],
                    [llvm_v16i32_ty, llvm_v16i16_ty, llvm_i16_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_dw_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovsdw512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v16i32_ty, llvm_i16_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_dw_512 :
          ClangBuiltin<"__builtin_ia32_pmovusdw512_mask">,
          Intrinsic<[llvm_v16i16_ty],
                    [llvm_v16i32_ty, llvm_v16i16_ty, llvm_i16_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_dw_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovusdw512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v16i32_ty, llvm_i16_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_wb_128 :
          ClangBuiltin<"__builtin_ia32_pmovwb128_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v8i16_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmov_wb_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovwb128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_wb_128 :
          ClangBuiltin<"__builtin_ia32_pmovswb128_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v8i16_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_wb_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovswb128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_wb_128 :
          ClangBuiltin<"__builtin_ia32_pmovuswb128_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v8i16_ty, llvm_v16i8_ty, llvm_i8_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_wb_mem_128 :
          ClangBuiltin<"__builtin_ia32_pmovuswb128mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v8i16_ty, llvm_i8_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_wb_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovwb256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v16i16_ty, llvm_i16_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_wb_256 :
          ClangBuiltin<"__builtin_ia32_pmovswb256_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v16i16_ty, llvm_v16i8_ty, llvm_i16_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_wb_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovswb256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v16i16_ty, llvm_i16_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_wb_256 :
          ClangBuiltin<"__builtin_ia32_pmovuswb256_mask">,
          Intrinsic<[llvm_v16i8_ty],
                    [llvm_v16i16_ty, llvm_v16i8_ty, llvm_i16_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_wb_mem_256 :
          ClangBuiltin<"__builtin_ia32_pmovuswb256mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v16i16_ty, llvm_i16_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmov_wb_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovwb512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v32i16_ty, llvm_i32_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovs_wb_512 :
          ClangBuiltin<"__builtin_ia32_pmovswb512_mask">,
          Intrinsic<[llvm_v32i8_ty],
                    [llvm_v32i16_ty, llvm_v32i8_ty, llvm_i32_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovs_wb_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovswb512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v32i16_ty, llvm_i32_ty],
                    [IntrArgMemOnly]>;
  def int_x86_avx512_mask_pmovus_wb_512 :
          ClangBuiltin<"__builtin_ia32_pmovuswb512_mask">,
          Intrinsic<[llvm_v32i8_ty],
                    [llvm_v32i16_ty, llvm_v32i8_ty, llvm_i32_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_mask_pmovus_wb_mem_512 :
          ClangBuiltin<"__builtin_ia32_pmovuswb512mem_mask">,
          Intrinsic<[],
                    [llvm_ptr_ty, llvm_v32i16_ty, llvm_i32_ty],
                    [IntrArgMemOnly]>;
}

// Bitwise ternary logic
let TargetPrefix = "x86" in {
  def int_x86_avx512_pternlog_d_128 :
          ClangBuiltin<"__builtin_ia32_pternlogd128">,
          Intrinsic<[llvm_v4i32_ty],
                    [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_pternlog_d_256 :
          ClangBuiltin<"__builtin_ia32_pternlogd256">,
          Intrinsic<[llvm_v8i32_ty],
                    [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_pternlog_d_512 :
          ClangBuiltin<"__builtin_ia32_pternlogd512">,
          Intrinsic<[llvm_v16i32_ty],
                    [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty,
                     llvm_i32_ty], [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_pternlog_q_128 :
          ClangBuiltin<"__builtin_ia32_pternlogq128">,
          Intrinsic<[llvm_v2i64_ty],
                    [llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_pternlog_q_256 :
          ClangBuiltin<"__builtin_ia32_pternlogq256">,
          Intrinsic<[llvm_v4i64_ty],
                    [llvm_v4i64_ty, llvm_v4i64_ty, llvm_v4i64_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<3>>]>;

  def int_x86_avx512_pternlog_q_512 :
          ClangBuiltin<"__builtin_ia32_pternlogq512">,
          Intrinsic<[llvm_v8i64_ty],
                    [llvm_v8i64_ty, llvm_v8i64_ty, llvm_v8i64_ty, llvm_i32_ty],
                    [IntrNoMem, ImmArg<ArgIndex<3>>]>;
}

// vp2intersect
let TargetPrefix = "x86" in {
  def int_x86_avx512_vp2intersect_q_512 :
          Intrinsic<[llvm_v8i1_ty, llvm_v8i1_ty],
                    [llvm_v8i64_ty, llvm_v8i64_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_vp2intersect_q_256 :
          Intrinsic<[llvm_v4i1_ty, llvm_v4i1_ty],
                    [llvm_v4i64_ty, llvm_v4i64_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_vp2intersect_q_128 :
          Intrinsic<[llvm_v2i1_ty, llvm_v2i1_ty],
                    [llvm_v2i64_ty, llvm_v2i64_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_vp2intersect_d_512 :
          Intrinsic<[llvm_v16i1_ty, llvm_v16i1_ty],
                    [llvm_v16i32_ty, llvm_v16i32_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_vp2intersect_d_256 :
          Intrinsic<[llvm_v8i1_ty, llvm_v8i1_ty],
                    [llvm_v8i32_ty, llvm_v8i32_ty],
                    [IntrNoMem]>;
  def int_x86_avx512_vp2intersect_d_128 :
          Intrinsic<[llvm_v4i1_ty, llvm_v4i1_ty],
                    [llvm_v4i32_ty, llvm_v4i32_ty],
                    [IntrNoMem]>;
}

// Misc.
let TargetPrefix = "x86" in {
  // NOTE: These comparison intrinsics are not used by clang as long as the
  //       distinction in signaling behaviour is not implemented.
  def int_x86_avx512_mask_cmp_ps_512 :
              Intrinsic<[llvm_v16i1_ty], [llvm_v16f32_ty, llvm_v16f32_ty,
                         llvm_i32_ty, llvm_v16i1_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_cmp_pd_512 :
              Intrinsic<[llvm_v8i1_ty], [llvm_v8f64_ty, llvm_v8f64_ty,
                         llvm_i32_ty, llvm_v8i1_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_cmp_ps_256 :
              Intrinsic<[llvm_v8i1_ty], [llvm_v8f32_ty, llvm_v8f32_ty,
                         llvm_i32_ty, llvm_v8i1_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_mask_cmp_pd_256 :
              Intrinsic<[llvm_v4i1_ty], [llvm_v4f64_ty, llvm_v4f64_ty,
                         llvm_i32_ty, llvm_v4i1_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_mask_cmp_ps_128 :
            Intrinsic<[llvm_v4i1_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                       llvm_i32_ty, llvm_v4i1_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_mask_cmp_pd_128 :
            Intrinsic<[llvm_v2i1_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                       llvm_i32_ty, llvm_v2i1_ty], [IntrNoMem, ImmArg<ArgIndex<2>>]>;

  def int_x86_avx512_mask_cmp_ss :
        ClangBuiltin<"__builtin_ia32_cmpss_mask">,
              Intrinsic<[llvm_i8_ty], [llvm_v4f32_ty, llvm_v4f32_ty,
                         llvm_i32_ty, llvm_i8_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<4>>]>;
  def int_x86_avx512_mask_cmp_sd :
        ClangBuiltin<"__builtin_ia32_cmpsd_mask">,
              Intrinsic<[llvm_i8_ty], [llvm_v2f64_ty, llvm_v2f64_ty,
                         llvm_i32_ty, llvm_i8_ty, llvm_i32_ty],
                        [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<4>>]>;
}

//===----------------------------------------------------------------------===//
// SHA intrinsics
let TargetPrefix = "x86" in {
  def int_x86_sha1rnds4 : ClangBuiltin<"__builtin_ia32_sha1rnds4">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
  def int_x86_sha1nexte : ClangBuiltin<"__builtin_ia32_sha1nexte">,
      Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_sha1msg1 : ClangBuiltin<"__builtin_ia32_sha1msg1">,
      Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_sha1msg2 : ClangBuiltin<"__builtin_ia32_sha1msg2">,
      Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_sha256rnds2 : ClangBuiltin<"__builtin_ia32_sha256rnds2">,
      Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                [IntrNoMem]>;
  def int_x86_sha256msg1 : ClangBuiltin<"__builtin_ia32_sha256msg1">,
      Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_sha256msg2 : ClangBuiltin<"__builtin_ia32_sha256msg2">,
      Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty], [IntrNoMem]>;
}

//===----------------------------------------------------------------------===//
// Thread synchronization ops with timer.
let TargetPrefix = "x86" in {
  def int_x86_monitorx
      : ClangBuiltin<"__builtin_ia32_monitorx">,
        Intrinsic<[], [ llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty ], []>;
  def int_x86_mwaitx
      : ClangBuiltin<"__builtin_ia32_mwaitx">,
        Intrinsic<[], [ llvm_i32_ty, llvm_i32_ty, llvm_i32_ty ], []>;
}

//===----------------------------------------------------------------------===//
// Cache-line zero
let TargetPrefix = "x86" in {
  def int_x86_clzero : ClangBuiltin<"__builtin_ia32_clzero">,
      Intrinsic<[], [llvm_ptr_ty], []>;
}

//===----------------------------------------------------------------------===//
// Cache write back intrinsics

let TargetPrefix = "x86" in {
  // Write back and invalidate
  def int_x86_wbinvd : ClangBuiltin<"__builtin_ia32_wbinvd">,
      Intrinsic<[], [], []>;

  // Write back no-invalidate
  def int_x86_wbnoinvd : ClangBuiltin<"__builtin_ia32_wbnoinvd">,
      Intrinsic<[], [], []>;
}

//===----------------------------------------------------------------------===//
// Cache-line demote

let TargetPrefix = "x86" in {
  def int_x86_cldemote : ClangBuiltin<"__builtin_ia32_cldemote">,
      Intrinsic<[], [llvm_ptr_ty], []>;
}

//===----------------------------------------------------------------------===//
// Wait and pause enhancements
let TargetPrefix = "x86" in {
  def int_x86_umonitor : ClangBuiltin<"__builtin_ia32_umonitor">,
              Intrinsic<[], [llvm_ptr_ty], []>;
  def int_x86_umwait : ClangBuiltin<"__builtin_ia32_umwait">,
              Intrinsic<[llvm_i8_ty], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], []>;
  def int_x86_tpause : ClangBuiltin<"__builtin_ia32_tpause">,
              Intrinsic<[llvm_i8_ty], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], []>;
}

//===----------------------------------------------------------------------===//
// Direct Move Instructions

let TargetPrefix = "x86" in {
  def int_x86_directstore32 : ClangBuiltin<"__builtin_ia32_directstore_u32">,
      Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty], []>;
  def int_x86_directstore64 : ClangBuiltin<"__builtin_ia32_directstore_u64">,
      Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty], []>;
  def int_x86_movdir64b : ClangBuiltin<"__builtin_ia32_movdir64b">,
      Intrinsic<[], [llvm_ptr_ty, llvm_ptr_ty], []>;
}

//===----------------------------------------------------------------------===//
// PTWrite - Write data to processor trace pocket

let TargetPrefix = "x86" in {
  def int_x86_ptwrite32 : ClangBuiltin<"__builtin_ia32_ptwrite32">,
              Intrinsic<[], [llvm_i32_ty], []>;
  def int_x86_ptwrite64 : ClangBuiltin<"__builtin_ia32_ptwrite64">,
              Intrinsic<[], [llvm_i64_ty], []>;
}

//===----------------------------------------------------------------------===//
// INVPCID - Invalidate Process-Context Identifier

let TargetPrefix = "x86" in {
  def int_x86_invpcid : ClangBuiltin<"__builtin_ia32_invpcid">,
              Intrinsic<[], [llvm_i32_ty, llvm_ptr_ty], []>;
}

let TargetPrefix = "x86" in {
  def int_x86_avx512bf16_cvtne2ps2bf16_128:
              ClangBuiltin<"__builtin_ia32_cvtne2ps2bf16_128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v4f32_ty, llvm_v4f32_ty],
              [IntrNoMem]>;
  def int_x86_avx512bf16_cvtne2ps2bf16_256:
              ClangBuiltin<"__builtin_ia32_cvtne2ps2bf16_256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v8f32_ty, llvm_v8f32_ty],
              [IntrNoMem]>;
  def int_x86_avx512bf16_cvtne2ps2bf16_512:
              ClangBuiltin<"__builtin_ia32_cvtne2ps2bf16_512">,
              Intrinsic<[llvm_v32i16_ty], [llvm_v16f32_ty, llvm_v16f32_ty],
              [IntrNoMem]>;
  // Intrinsic must be masked due to it producing less than 128 bits of results.
  def int_x86_avx512bf16_mask_cvtneps2bf16_128:
              Intrinsic<[llvm_v8i16_ty],
                        [llvm_v4f32_ty, llvm_v8i16_ty, llvm_v4i1_ty],
                        [IntrNoMem]>;
  def int_x86_avx512bf16_cvtneps2bf16_256:
              ClangBuiltin<"__builtin_ia32_cvtneps2bf16_256">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v8f32_ty], [IntrNoMem]>;
  def int_x86_avx512bf16_cvtneps2bf16_512:
              ClangBuiltin<"__builtin_ia32_cvtneps2bf16_512">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v16f32_ty], [IntrNoMem]>;
  def int_x86_avx512bf16_dpbf16ps_128:
              ClangBuiltin<"__builtin_ia32_dpbf16ps_128">,
              Intrinsic<[llvm_v4f32_ty],
              [llvm_v4f32_ty, llvm_v4i32_ty, llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_avx512bf16_dpbf16ps_256:
              ClangBuiltin<"__builtin_ia32_dpbf16ps_256">,
              Intrinsic<[llvm_v8f32_ty],
              [llvm_v8f32_ty, llvm_v8i32_ty, llvm_v8i32_ty], [IntrNoMem]>;
  def int_x86_avx512bf16_dpbf16ps_512:
              ClangBuiltin<"__builtin_ia32_dpbf16ps_512">,
              Intrinsic<[llvm_v16f32_ty],
              [llvm_v16f32_ty, llvm_v16i32_ty, llvm_v16i32_ty], [IntrNoMem]>;
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX_BF16
  def int_x86_avx2bf16_cvtne2ps2bf16_128:
              ClangBuiltin<"__builtin_ia32_cvtne2ps2bf16vex128">,
              Intrinsic<[llvm_v8i16_ty], [llvm_v4f32_ty, llvm_v4f32_ty],
              [IntrNoMem]>;
  def int_x86_avx2bf16_cvtne2ps2bf16_256:
              ClangBuiltin<"__builtin_ia32_cvtne2ps2bf16vex256">,
              Intrinsic<[llvm_v16i16_ty], [llvm_v8f32_ty, llvm_v8f32_ty],
              [IntrNoMem]>;
  def int_x86_avx2bf16_dpbf16ps_128:
              ClangBuiltin<"__builtin_ia32_dpbf16psvex128">,
              Intrinsic<[llvm_v4f32_ty],
              [llvm_v4f32_ty, llvm_v4i32_ty, llvm_v4i32_ty], [IntrNoMem]>;
  def int_x86_avx2bf16_dpbf16ps_256:
              ClangBuiltin<"__builtin_ia32_dpbf16psvex256">,
              Intrinsic<[llvm_v8f32_ty],
              [llvm_v8f32_ty, llvm_v8i32_ty, llvm_v8i32_ty], [IntrNoMem]>;
#endif // INTEL_FEATURE_ISA_AVX_BF16
#endif // INTEL_CUSTOMIZATION
}

//===----------------------------------------------------------------------===//
// ENQCMD - Enqueue Stores Instructions

let TargetPrefix = "x86" in {
  def int_x86_enqcmd : ClangBuiltin<"__builtin_ia32_enqcmd">,
              Intrinsic<[llvm_i8_ty], [llvm_ptr_ty, llvm_ptr_ty], []>;
  def int_x86_enqcmds : ClangBuiltin<"__builtin_ia32_enqcmds">,
              Intrinsic<[llvm_i8_ty], [llvm_ptr_ty, llvm_ptr_ty], []>;
}

//===----------------------------------------------------------------------===//
// SERIALIZE - Serialize instruction fetch and execution

let TargetPrefix = "x86" in {
  def int_x86_serialize : ClangBuiltin<"__builtin_ia32_serialize">,
              Intrinsic<[], [], []>;
}

//===----------------------------------------------------------------------===//
// TSXLDTRK - TSX Suspend Load Address Tracking

let TargetPrefix = "x86" in {
  def int_x86_xsusldtrk : ClangBuiltin<"__builtin_ia32_xsusldtrk">,
              Intrinsic<[], [], []>;
  def int_x86_xresldtrk : ClangBuiltin<"__builtin_ia32_xresldtrk">,
              Intrinsic<[], [], []>;
}

//===----------------------------------------------------------------------===//
// Key Locker
let TargetPrefix = "x86" in {
  def int_x86_loadiwkey : ClangBuiltin<"__builtin_ia32_loadiwkey">,
      Intrinsic<[], [llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty, llvm_i32_ty],
                []>;
  def int_x86_encodekey128 :
      Intrinsic<[llvm_i32_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty],
                [llvm_i32_ty, llvm_v2i64_ty], []>;
  def int_x86_encodekey256 :
      Intrinsic<[llvm_i32_ty, llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty],
                [llvm_i32_ty, llvm_v2i64_ty, llvm_v2i64_ty], []>;
  def int_x86_aesenc128kl :
      Intrinsic<[llvm_i8_ty, llvm_v2i64_ty], [llvm_v2i64_ty, llvm_ptr_ty], []>;
  def int_x86_aesdec128kl :
      Intrinsic<[llvm_i8_ty, llvm_v2i64_ty], [llvm_v2i64_ty, llvm_ptr_ty], []>;
  def int_x86_aesenc256kl :
      Intrinsic<[llvm_i8_ty, llvm_v2i64_ty], [llvm_v2i64_ty, llvm_ptr_ty], []>;
  def int_x86_aesdec256kl :
      Intrinsic<[llvm_i8_ty, llvm_v2i64_ty], [llvm_v2i64_ty, llvm_ptr_ty], []>;
  def int_x86_aesencwide128kl :
      Intrinsic<[llvm_i8_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty],
                [llvm_ptr_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty], []>;
  def int_x86_aesdecwide128kl :
      Intrinsic<[llvm_i8_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty],
                [llvm_ptr_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty], []>;
  def int_x86_aesencwide256kl :
      Intrinsic<[llvm_i8_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty],
                [llvm_ptr_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty], []>;
  def int_x86_aesdecwide256kl :
      Intrinsic<[llvm_i8_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty],
                [llvm_ptr_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty,
                 llvm_v2i64_ty, llvm_v2i64_ty, llvm_v2i64_ty], []>;
}

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX_MEMADVISE
// FIXME: We assume that avxmemadvise is disclosed firstly. Renaming these
// if avx512memadvise is disclosed firstly.
let TargetPrefix = "x86" in {
  def int_x86_avx2_vmovadvisew_load_128 :
              ClangBuiltin<"__builtin_ia32_vmovadvisew_load_128">,
              Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty, llvm_i8_ty],
                        [IntrReadMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx2_vmovadvisew_store_128 :
              ClangBuiltin<"__builtin_ia32_vmovadvisew_store_128">,
              Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty],
                        [IntrArgMemOnly, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx2_vmovadvisew_load_256 :
              ClangBuiltin<"__builtin_ia32_vmovadvisew_load_256">,
              Intrinsic<[llvm_v8i32_ty], [llvm_ptr_ty, llvm_i8_ty],
                        [IntrReadMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx2_vmovadvisew_store_256 :
              ClangBuiltin<"__builtin_ia32_vmovadvisew_store_256">,
              Intrinsic<[], [llvm_ptr_ty, llvm_v8i32_ty, llvm_i8_ty],
                        [IntrArgMemOnly, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx2_vmemadvise_128 :
              ClangBuiltin<"__builtin_ia32_vmemadvise_128">,
              Intrinsic<[], [llvm_ptr_ty, llvm_i8_ty],
                        [IntrArgMemOnly, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx2_vmemadvise_256 :
              ClangBuiltin<"__builtin_ia32_vmemadvise_256">,
              Intrinsic<[], [llvm_ptr_ty, llvm_i8_ty],
                        [IntrArgMemOnly, ImmArg<ArgIndex<1>>]>;
}

let TargetPrefix = "x86" in {
  def int_x86_avx512_vmovadvisew_load_512 :
              ClangBuiltin<"__builtin_ia32_vmovadvisew_load_512">,
              Intrinsic<[llvm_v16i32_ty], [llvm_ptr_ty, llvm_i8_ty],
                        [IntrReadMem, ImmArg<ArgIndex<1>>]>;
  def int_x86_avx512_vmovadvisew_store_512 :
              ClangBuiltin<"__builtin_ia32_vmovadvisew_store_512">,
              Intrinsic<[], [llvm_ptr_ty, llvm_v16i32_ty, llvm_i8_ty],
                        [IntrArgMemOnly, ImmArg<ArgIndex<2>>]>;
  def int_x86_avx512_vmemadvise_512 :
              ClangBuiltin<"__builtin_ia32_vmemadvise_512">,
              Intrinsic<[], [llvm_ptr_ty, llvm_i8_ty],
                        [IntrArgMemOnly, ImmArg<ArgIndex<1>>]>;
}
#endif // INTEL_FEATURE_ISA_AVX_MEMADVISE
#if INTEL_FEATURE_ISA_AVX_MOVGET
let TargetPrefix = "x86" in {
  def int_x86_vmovget128 : ClangBuiltin<"__builtin_ia32_vmovget128">,
        Intrinsic<[llvm_v4i32_ty], [llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_vmovget256 : ClangBuiltin<"__builtin_ia32_vmovget256">,
        Intrinsic<[llvm_v8i32_ty], [llvm_ptr_ty],
        [IntrReadMem]>;
}
#endif // INTEL_FEATURE_ISA_AVX_MOVGET
#if INTEL_FEATURE_ISA_AVX512_MOVGET
let TargetPrefix = "x86" in {
  def int_x86_vmovget512 : ClangBuiltin<"__builtin_ia32_vmovget512">,
    Intrinsic<[llvm_v16i32_ty], [llvm_ptr_ty],
    [IntrReadMem]>;
}
#endif // INTEL_FEATURE_ISA_AVX512_MOVGET
#endif // INTEL_CUSTOMIZATION

//===----------------------------------------------------------------------===//
// AMX - Intel AMX extensions

let TargetPrefix = "x86" in {
  def int_x86_ldtilecfg : ClangBuiltin<"__builtin_ia32_tile_loadconfig">,
              Intrinsic<[], [llvm_ptr_ty], []>;
  def int_x86_sttilecfg : ClangBuiltin<"__builtin_ia32_tile_storeconfig">,
              Intrinsic<[], [llvm_ptr_ty], []>;
  def int_x86_tilerelease : ClangBuiltin<"__builtin_ia32_tilerelease">,
              Intrinsic<[], [], []>;
  def int_x86_tilezero : ClangBuiltin<"__builtin_ia32_tilezero">,
              Intrinsic<[], [llvm_i8_ty], [ImmArg<ArgIndex<0>>]>;
  def int_x86_tileloadd64 : ClangBuiltin<"__builtin_ia32_tileloadd64">,
              Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                        [ImmArg<ArgIndex<0>>]>;
  def int_x86_tileloaddt164 : ClangBuiltin<"__builtin_ia32_tileloaddt164">,
              Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                        [ImmArg<ArgIndex<0>>]>;
  def int_x86_tilestored64 : ClangBuiltin<"__builtin_ia32_tilestored64">,
              Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                        [ImmArg<ArgIndex<0>>]>;
  def int_x86_tdpbssd : ClangBuiltin<"__builtin_ia32_tdpbssd">,
              Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                         ImmArg<ArgIndex<2>>]>;
  def int_x86_tdpbsud : ClangBuiltin<"__builtin_ia32_tdpbsud">,
              Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                         ImmArg<ArgIndex<2>>]>;
  def int_x86_tdpbusd : ClangBuiltin<"__builtin_ia32_tdpbusd">,
              Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                         ImmArg<ArgIndex<2>>]>;
  def int_x86_tdpbuud : ClangBuiltin<"__builtin_ia32_tdpbuud">,
              Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                         ImmArg<ArgIndex<2>>]>;
  def int_x86_tdpbf16ps : ClangBuiltin<"__builtin_ia32_tdpbf16ps">,
              Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                         ImmArg<ArgIndex<2>>]>;
  // AMX - internal intrinsics
  def int_x86_ldtilecfg_internal :
              ClangBuiltin<"__builtin_ia32_tile_loadconfig_internal">,
              Intrinsic<[], [llvm_ptr_ty], []>;
  def int_x86_tileloadd64_internal :
              ClangBuiltin<"__builtin_ia32_tileloadd64_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_ptr_ty, llvm_i64_ty],
                        []>;
  def int_x86_tileloaddt164_internal :
              ClangBuiltin<"__builtin_ia32_tileloaddt164_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_ptr_ty, llvm_i64_ty],
                        []>;
  def int_x86_tdpbssd_internal :
              ClangBuiltin<"__builtin_ia32_tdpbssd_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
  def int_x86_tdpbsud_internal :
              ClangBuiltin<"__builtin_ia32_tdpbsud_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
  def int_x86_tdpbusd_internal :
              ClangBuiltin<"__builtin_ia32_tdpbusd_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
  def int_x86_tdpbuud_internal :
              ClangBuiltin<"__builtin_ia32_tdpbuud_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
  def int_x86_tilestored64_internal :
              ClangBuiltin<"__builtin_ia32_tilestored64_internal">,
              Intrinsic<[], [llvm_i16_ty, llvm_i16_ty, llvm_ptr_ty,
                             llvm_i64_ty, llvm_x86amx_ty], []>;
  def int_x86_tilezero_internal :
              ClangBuiltin<"__builtin_ia32_tilezero_internal">,
              Intrinsic<[llvm_x86amx_ty], [llvm_i16_ty, llvm_i16_ty],
                        []>;
  def int_x86_tdpbf16ps_internal :
              ClangBuiltin<"__builtin_ia32_tdpbf16ps_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
  def int_x86_cast_vector_to_tile:
              Intrinsic<[llvm_x86amx_ty], [llvm_anyvector_ty], [IntrNoMem]>;
  def int_x86_cast_tile_to_vector:
              Intrinsic<[llvm_anyvector_ty], [llvm_x86amx_ty], [IntrNoMem]>;
}

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AMX_BF8
let TargetPrefix = "x86" in {
  def int_x86_tdpbf8ps : ClangBuiltin<"__builtin_ia32_tdpbf8ps">,
              Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                        [ImmArg<ArgIndex<0>>,
                         ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
}
#endif // INTEL_FEATURE_ISA_AMX_BF8

#if INTEL_FEATURE_ISA_AMX_FUTURE
//===----------------------------------------------------------------------===//
// AMX_FUTURE - Intel AMX_FUTURE extensions
let TargetPrefix = "x86" in {
  // Reduce
  def int_x86_tcoladdbcastps
      : ClangBuiltin<"__builtin_ia32_tcoladdbcastps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tcoladdps : ClangBuiltin<"__builtin_ia32_tcoladdps">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i8_ty], [ImmArg<ArgIndex<1>>]>;
  // Format
  def int_x86_tblendvd : ClangBuiltin<"__builtin_ia32_tblendvd">,
                         Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                   [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                    ImmArg<ArgIndex<2>>]>;
  def int_x86_tinterleaveeb
      : ClangBuiltin<"__builtin_ia32_tinterleaveeb">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_tinterleaveew
      : ClangBuiltin<"__builtin_ia32_tinterleaveew">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_tinterleaveob
      : ClangBuiltin<"__builtin_ia32_tinterleaveob">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_tinterleaveow
      : ClangBuiltin<"__builtin_ia32_tinterleaveow">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_tnarrowb : ClangBuiltin<"__builtin_ia32_tnarrowb">,
                         Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                   [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                    ImmArg<ArgIndex<2>>]>;
  def int_x86_tnarroww : ClangBuiltin<"__builtin_ia32_tnarroww">,
                         Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                   [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                    ImmArg<ArgIndex<2>>]>;
  def int_x86_tpermb_reg : ClangBuiltin<"__builtin_ia32_tpermb_reg">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                      ImmArg<ArgIndex<2>>]>;
  def int_x86_tpermb_mem : ClangBuiltin<"__builtin_ia32_tpermb_mem">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tpermd_reg : ClangBuiltin<"__builtin_ia32_tpermd_reg">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                      ImmArg<ArgIndex<2>>]>;
  def int_x86_tpermd_mem : ClangBuiltin<"__builtin_ia32_tpermd_mem">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tpermw_reg : ClangBuiltin<"__builtin_ia32_tpermw_reg">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                      ImmArg<ArgIndex<2>>]>;
  def int_x86_tpermw_mem : ClangBuiltin<"__builtin_ia32_tpermw_mem">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_twidenb : ClangBuiltin<"__builtin_ia32_twidenb">,
                        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                   ImmArg<ArgIndex<2>>]>;
  def int_x86_twidenw : ClangBuiltin<"__builtin_ia32_twidenw">,
                        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                   ImmArg<ArgIndex<2>>]>;
  // Element
  def int_x86_taddps_reg : ClangBuiltin<"__builtin_ia32_taddps_reg">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                      ImmArg<ArgIndex<2>>]>;
  def int_x86_taddps_mem : ClangBuiltin<"__builtin_ia32_taddps_mem">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tandd_reg : ClangBuiltin<"__builtin_ia32_tandd_reg">,
                          Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                    [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                     ImmArg<ArgIndex<2>>]>;
  def int_x86_tandd_mem : ClangBuiltin<"__builtin_ia32_tandd_mem">,
                          Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                                    [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tandnd_reg : ClangBuiltin<"__builtin_ia32_tandnd_reg">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                      ImmArg<ArgIndex<2>>]>;
  def int_x86_tandnd_mem : ClangBuiltin<"__builtin_ia32_tandnd_mem">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tcmpps_reg
      : ClangBuiltin<"__builtin_ia32_tcmpps_reg">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>,
                   ImmArg<ArgIndex<3>>]>;
  def int_x86_tcmpps_mem
      : ClangBuiltin<"__builtin_ia32_tcmpps_mem">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<3>>]>;
  def int_x86_tcvtb2ps
      : ClangBuiltin<"__builtin_ia32_tcvtb2ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty], [ImmArg<ArgIndex<0>>,
                                                 ImmArg<ArgIndex<1>>]>;
  def int_x86_tcvtbf162ps
      : ClangBuiltin<"__builtin_ia32_tcvtbf162ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty], [ImmArg<ArgIndex<0>>,
                                                 ImmArg<ArgIndex<1>>]>;
  def int_x86_tcvtd2ps
      : ClangBuiltin<"__builtin_ia32_tcvtd2ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty], [ImmArg<ArgIndex<0>>,
                                                 ImmArg<ArgIndex<1>>]>;
  def int_x86_tcvtps2bf16
      : ClangBuiltin<"__builtin_ia32_tcvtps2bf16">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty], [ImmArg<ArgIndex<0>>,
                                                 ImmArg<ArgIndex<1>>]>;
  def int_x86_tcvtps2bs
      : ClangBuiltin<"__builtin_ia32_tcvtps2bs">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty], [ImmArg<ArgIndex<0>>,
                                                 ImmArg<ArgIndex<1>>]>;
  def int_x86_tcvtps2ubs
      : ClangBuiltin<"__builtin_ia32_tcvtps2ubs">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty], [ImmArg<ArgIndex<0>>,
                                                 ImmArg<ArgIndex<1>>]>;
  def int_x86_tcvtub2ps
      : ClangBuiltin<"__builtin_ia32_tcvtub2ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty], [ImmArg<ArgIndex<0>>,
                                                 ImmArg<ArgIndex<1>>]>;
  def int_x86_tfmaddps_reg : ClangBuiltin<"__builtin_ia32_tfmaddps_reg">,
                             Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                       [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                        ImmArg<ArgIndex<2>>]>;
  def int_x86_tfmaddps_mem
      : ClangBuiltin<"__builtin_ia32_tfmaddps_mem">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tfmsubps_reg : ClangBuiltin<"__builtin_ia32_tfmsubps_reg">,
                             Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                       [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                        ImmArg<ArgIndex<2>>]>;
  def int_x86_tfmsubps_mem
      : ClangBuiltin<"__builtin_ia32_tfmsubps_mem">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tfnmaddps_reg
      : ClangBuiltin<"__builtin_ia32_tfnmaddps_reg">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                   ImmArg<ArgIndex<2>>]>;
  def int_x86_tfnmaddps_mem
      : ClangBuiltin<"__builtin_ia32_tfnmaddps_mem">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tfnmsubps_reg
      : ClangBuiltin<"__builtin_ia32_tfnmsubps_reg">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_tfnmsubps_mem
      : ClangBuiltin<"__builtin_ia32_tfnmsubps_mem">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tmaxps_reg : ClangBuiltin<"__builtin_ia32_tmaxps_reg">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                      ImmArg<ArgIndex<2>>]>;
  def int_x86_tmaxps_mem : ClangBuiltin<"__builtin_ia32_tmaxps_mem">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tminps_reg : ClangBuiltin<"__builtin_ia32_tminps_reg">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                      ImmArg<ArgIndex<2>>]>;
  def int_x86_tminps_mem : ClangBuiltin<"__builtin_ia32_tminps_mem">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tmulps_reg : ClangBuiltin<"__builtin_ia32_tmulps_reg">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                      ImmArg<ArgIndex<2>>]>;
  def int_x86_tmulps_mem : ClangBuiltin<"__builtin_ia32_tmulps_mem">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tord_reg : ClangBuiltin<"__builtin_ia32_tord_reg">,
                         Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                   [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                    ImmArg<ArgIndex<2>>]>;
  def int_x86_tord_mem : ClangBuiltin<"__builtin_ia32_tord_mem">,
                         Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                                   [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_trcp14ps
      : ClangBuiltin<"__builtin_ia32_trcp14ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty], [ImmArg<ArgIndex<0>>,
                                                 ImmArg<ArgIndex<1>>]>;
  def int_x86_treduceps : ClangBuiltin<"__builtin_ia32_treduceps">,
                          Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                    [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                     ImmArg<ArgIndex<2>>]>;
  def int_x86_tscalefps_reg
      : ClangBuiltin<"__builtin_ia32_tscalefps_reg">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_tscalefps_mem
      : ClangBuiltin<"__builtin_ia32_tscalefps_mem">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tslld : ClangBuiltin<"__builtin_ia32_tslld">,
                      Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                 ImmArg<ArgIndex<2>>]>;
  def int_x86_tsrld : ClangBuiltin<"__builtin_ia32_tsrld">,
                      Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                 ImmArg<ArgIndex<2>>]>;
  def int_x86_tsrlvd_reg : ClangBuiltin<"__builtin_ia32_tsrlvd_reg">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                      ImmArg<ArgIndex<2>>]>;
  def int_x86_tsrlvd_mem : ClangBuiltin<"__builtin_ia32_tsrlvd_mem">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tsubps_reg : ClangBuiltin<"__builtin_ia32_tsubps_reg">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                      ImmArg<ArgIndex<2>>]>;
  def int_x86_tsubps_mem : ClangBuiltin<"__builtin_ia32_tsubps_mem">,
                           Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                                     [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_txord_reg : ClangBuiltin<"__builtin_ia32_txord_reg">,
                          Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                    [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                                     ImmArg<ArgIndex<2>>]>;
  def int_x86_txord_mem : ClangBuiltin<"__builtin_ia32_txord_mem">,
                          Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_ptr_ty],
                                    [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
}
#endif // INTEL_FEATURE_ISA_AMX_FUTURE

#if INTEL_FEATURE_ISA_AMX_LNC
//===----------------------------------------------------------------------===//
// AMX_LNC - Intel LNC AMX extensions
let TargetPrefix = "x86" in {
//AMX-AVX512
  def int_x86_tile16move : ClangBuiltin<"__builtin_ia32_tile16move">,
              Intrinsic<[], [llvm_i8_ty, llvm_v16f32_ty,
                             llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty,
                             llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty,
                             llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty,
                             llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty,
                             llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty],
                        [ImmArg<ArgIndex<0>>]>;
  def int_x86_tilemovei : ClangBuiltin<"__builtin_ia32_tilemovei">,
                          Intrinsic<[llvm_v16f32_ty], [llvm_i8_ty, llvm_i8_ty],
                                    [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tilemovee : ClangBuiltin<"__builtin_ia32_tilemovee">,
                          Intrinsic<[llvm_v16f32_ty], [llvm_i8_ty, llvm_i32_ty],
                                    [ImmArg<ArgIndex<0>>]>;
  def int_x86_tilemovex : ClangBuiltin<"__builtin_ia32_tilemovex">,
                          Intrinsic<[llvm_v16f32_ty], [llvm_i8_ty, llvm_v4f32_ty],
                                    [ImmArg<ArgIndex<0>>]>;
  def int_x86_tcvtrowps2bf16ei : ClangBuiltin<"__builtin_ia32_tcvtrowps2bf16ei">,
                                 Intrinsic<[llvm_v16i32_ty], [llvm_i8_ty, llvm_i32_ty],
                                 [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tcvtrowps2bf16ee : ClangBuiltin<"__builtin_ia32_tcvtrowps2bf16ee">,
                                 Intrinsic<[llvm_v16i32_ty], [llvm_i8_ty, llvm_i32_ty],
                                 [ImmArg<ArgIndex<0>>]>;
  def int_x86_tcvtrowps2phei : ClangBuiltin<"__builtin_ia32_tcvtrowps2phei">,
                                 Intrinsic<[llvm_v16f32_ty], [llvm_i8_ty, llvm_i32_ty],
                                 [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tcvtrowps2phee : ClangBuiltin<"__builtin_ia32_tcvtrowps2phee">,
                                 Intrinsic<[llvm_v16f32_ty], [llvm_i8_ty, llvm_i32_ty],
                                 [ImmArg<ArgIndex<0>>]>;
//AMX-MEMORY
  def int_x86_tgatherrowd
      : ClangBuiltin<"__builtin_ia32_tgatherrowd">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_ptr_ty], [ImmArg<ArgIndex<0>>]>;
  def int_x86_tgatherrowdt1
      : ClangBuiltin<"__builtin_ia32_tgatherrowdt1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_ptr_ty], [ImmArg<ArgIndex<0>>]>;
  def int_x86_tgatherrowq
      : ClangBuiltin<"__builtin_ia32_tgatherrowq">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_ptr_ty], [ImmArg<ArgIndex<0>>]>;
  def int_x86_tgatherrowqt1
      : ClangBuiltin<"__builtin_ia32_tgatherrowqt1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_ptr_ty], [ImmArg<ArgIndex<0>>]>;
  def int_x86_tscatterrowd
      : ClangBuiltin<"__builtin_ia32_tscatterrowd">,
        Intrinsic<[], [llvm_ptr_ty, llvm_ptr_ty, llvm_i8_ty], [ImmArg<ArgIndex<2>>]>;
  def int_x86_tscatterrowdt1
      : ClangBuiltin<"__builtin_ia32_tscatterrowdt1">,
        Intrinsic<[], [llvm_ptr_ty, llvm_ptr_ty, llvm_i8_ty], [ImmArg<ArgIndex<2>>]>;
  def int_x86_tscatterrowq
      : ClangBuiltin<"__builtin_ia32_tscatterrowq">,
        Intrinsic<[], [llvm_ptr_ty, llvm_ptr_ty, llvm_i8_ty], [ImmArg<ArgIndex<2>>]>;
  def int_x86_tscatterrowqt1
      : ClangBuiltin<"__builtin_ia32_tscatterrowqt1">,
        Intrinsic<[], [llvm_ptr_ty, llvm_ptr_ty, llvm_i8_ty], [ImmArg<ArgIndex<2>>]>;

// AMX-TRANSPOSE
  def int_x86_t2rpntlvw
      : ClangBuiltin<"__builtin_ia32_t2rpntlvw">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t2rpntlvwt1
      : ClangBuiltin<"__builtin_ia32_t2rpntlvwt1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t2rpntlvwz0
      : ClangBuiltin<"__builtin_ia32_t2rpntlvwz0">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t2rpntlvwz0t1
      : ClangBuiltin<"__builtin_ia32_t2rpntlvwz0t1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t2rpntlvwz1
      : ClangBuiltin<"__builtin_ia32_t2rpntlvwz1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t2rpntlvwz1t1
      : ClangBuiltin<"__builtin_ia32_t2rpntlvwz1t1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_ttdpbf16ps
      : ClangBuiltin<"__builtin_ia32_ttdpbf16ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_ttdpfp16ps
      : ClangBuiltin<"__builtin_ia32_ttdpfp16ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_ttransposed
      : ClangBuiltin<"__builtin_ia32_ttransposed">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;

  def int_x86_ttransposed_internal :
              ClangBuiltin<"__builtin_ia32_ttransposed_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty], []>;
  def int_x86_ttdpbf16ps_internal :
              ClangBuiltin<"__builtin_ia32_ttdpbf16ps_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
  def int_x86_ttdpfp16ps_internal :
              ClangBuiltin<"__builtin_ia32_ttdpfp16ps_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
  def int_x86_tilemovrowi_internal :
              ClangBuiltin<"__builtin_ia32_tilemovrowi_internal">,
              Intrinsic<[llvm_v16f32_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty, llvm_i32_ty],
                        [ImmArg<ArgIndex<3>>]>;
  def int_x86_tilemovrowe_internal :
              ClangBuiltin<"__builtin_ia32_tilemovrowe_internal">,
              Intrinsic<[llvm_v16f32_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty, llvm_i32_ty],
                        []>;
}
#endif // INTEL_FEATURE_ISA_AMX_LNC

#if INTEL_FEATURE_ISA_AMX_MEMORY2
//===----------------------------------------------------------------------===//
let TargetPrefix = "x86" in {
//AMX-MEMORY2-EVEX
  def int_x86_tbroadcastrowd
      : ClangBuiltin<"__builtin_ia32_tbroadcastrowd">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty], [ImmArg<ArgIndex<0>>]>;
  def int_x86_tstorehd
      : ClangBuiltin<"__builtin_ia32_tstorehd">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty], [ImmArg<ArgIndex<2>>]>;
  def int_x86_tstorehdt1
      : ClangBuiltin<"__builtin_ia32_tstorehdt1">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty], [ImmArg<ArgIndex<2>>]>;
  def int_x86_tstoreqd
      : ClangBuiltin<"__builtin_ia32_tstoreqd">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty], [ImmArg<ArgIndex<2>>]>;
  def int_x86_tstoreqdt1
      : ClangBuiltin<"__builtin_ia32_tstoreqdt1">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty], [ImmArg<ArgIndex<2>>]>;
  def int_x86_tstorerowd
      : ClangBuiltin<"__builtin_ia32_tstorerowd">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i8_ty], [ImmArg<ArgIndex<1>>]>;
  def int_x86_tstorentd
      : ClangBuiltin<"__builtin_ia32_tstorentd">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty], [ImmArg<ArgIndex<2>>]>;
}
#endif // INTEL_FEATURE_ISA_AMX_MEMORY2

#if INTEL_FEATURE_ISA_AMX_BF16_EVEX
//===----------------------------------------------------------------------===//
let TargetPrefix = "x86" in {
//AMX-BF16-EVEX
  def int_x86_tdpbf16pse : ClangBuiltin<"__builtin_ia32_tdpbf16pse">,
              Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>,
                         ImmArg<ArgIndex<2>>]>;
}
#endif // INTEL_FEATURE_ISA_AMX_BF16_EVEX

#if INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX
//===----------------------------------------------------------------------===//
let TargetPrefix = "x86" in {
//AMX-ELEMENT-EVEX
  def int_x86_tcvtd2pse
      : ClangBuiltin<"__builtin_ia32_tcvtd2pse">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty], [ImmArg<ArgIndex<2>>]>;

}
#endif // INTEL_FEATURE_ISA_AMX_ELEMENT_EVEX

#if INTEL_FEATURE_ISA_AMX_INT8_EVEX
//===----------------------------------------------------------------------===//
let TargetPrefix = "x86" in {
//AMX-INT8-EVEX
  def int_x86_tdpbssde : ClangBuiltin<"__builtin_ia32_tdpbssde">,
              Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_tdpbsude : ClangBuiltin<"__builtin_ia32_tdpbsude">,
              Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_tdpbusde : ClangBuiltin<"__builtin_ia32_tdpbusde">,
              Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_tdpbuude : ClangBuiltin<"__builtin_ia32_tdpbuude">,
              Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;

}
#endif // INTEL_FEATURE_ISA_AMX_INT8_EVEX

#if INTEL_FEATURE_ISA_AMX_TILE_EVEX
//===----------------------------------------------------------------------===//
let TargetPrefix = "x86" in {
//AMX-TILE-EVEX
  def int_x86_tileloadde64 : ClangBuiltin<"__builtin_ia32_tileloadde64">,
              Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                            [ImmArg<ArgIndex<0>>]>;
  def int_x86_tileloaddt1e64 : ClangBuiltin<"__builtin_ia32_tileloaddt1e64">,
              Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                            [ImmArg<ArgIndex<0>>]>;
  def int_x86_tilemove : ClangBuiltin<"__builtin_ia32_tilemove">,
                          Intrinsic<[], [llvm_i8_ty, llvm_i8_ty],
                                    [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_tilestorede64 : ClangBuiltin<"__builtin_ia32_tilestorede64">,
              Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                            [ImmArg<ArgIndex<0>>]>;
  def int_x86_tilezeroe : ClangBuiltin<"__builtin_ia32_tilezeroe">,
              Intrinsic<[], [llvm_i8_ty], [ImmArg<ArgIndex<0>>]>;


}
#endif // INTEL_FEATURE_ISA_AMX_TILE_EVEX

#if INTEL_FEATURE_ISA_AMX_TRANSPOSE2
//===----------------------------------------------------------------------===//
// AMX_TRANSPOSE2 - Intel TRANSPOSE2 AMX extensions
let TargetPrefix = "x86" in {
  def int_x86_t2transposew
      : ClangBuiltin<"__builtin_ia32_t2transposew">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t2transposewt1
      : ClangBuiltin<"__builtin_ia32_t2transposewt1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t4rqntlvbz0
      : ClangBuiltin<"__builtin_ia32_t4rqntlvbz0">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t4rqntlvbz0t1
      : ClangBuiltin<"__builtin_ia32_t4rqntlvbz0t1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t4rqntlvbz1
      : ClangBuiltin<"__builtin_ia32_t4rqntlvbz1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t4rqntlvbz1t1
      : ClangBuiltin<"__builtin_ia32_t4rqntlvbz1t1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t4rqntlvbz2
      : ClangBuiltin<"__builtin_ia32_t4rqntlvbz2">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t4rqntlvbz2t1
      : ClangBuiltin<"__builtin_ia32_t4rqntlvbz2t1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t4rqntlvbz3
      : ClangBuiltin<"__builtin_ia32_t4rqntlvbz3">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_t4rqntlvbz3t1
      : ClangBuiltin<"__builtin_ia32_t4rqntlvbz3t1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
  def int_x86_ttdpbssd
      : ClangBuiltin<"__builtin_ia32_ttdpbssd">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_ttdpbsud
      : ClangBuiltin<"__builtin_ia32_ttdpbsud">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_ttdpbusd
      : ClangBuiltin<"__builtin_ia32_ttdpbusd">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_ttdpbuud
      : ClangBuiltin<"__builtin_ia32_ttdpbuud">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
}
#endif // INTEL_FEATURE_ISA_AMX_TRANSPOSE2

#if INTEL_FEATURE_ISA_AMX_FP16
//===----------------------------------------------------------------------===//
let TargetPrefix = "x86" in {
// AMX_FP16 - Intel FP16 AMX extensions
//AMX-FP16
  def int_x86_tdpfp16ps : ClangBuiltin<"__builtin_ia32_tdpfp16ps">,
                          Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                                    [ImmArg<ArgIndex<0>>,
                                    ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
}
#endif // INTEL_FEATURE_ISA_AMX_FP16

#if INTEL_FEATURE_ISA_AMX_CONVERT
//===----------------------------------------------------------------------===//
let TargetPrefix = "x86" in {
// AMX-CONVERT - Intel CONVERT AMX extensions
  def int_x86_tcvt2ps2bf16 : ClangBuiltin<"__builtin_ia32_tcvt2ps2bf16">,
                            Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty, llvm_i8_ty],
                                      [ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
  def int_x86_tcvt2ps2ph : ClangBuiltin<"__builtin_ia32_tcvt2ps2ph">,
                           Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty, llvm_i8_ty],
                                     [ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
  def int_x86_amxconvert_tcvtd2ps : ClangBuiltin<"__builtin_ia32_amxconvert_tcvtd2ps">,
                                    Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty],
                                              [ImmArg<ArgIndex<2>>]>;
  def int_x86_amxconvert_tcvtps2bf16 : ClangBuiltin<"__builtin_ia32_amxconvert_tcvtps2bf16">,
                                                  Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty],
                                                            [ImmArg<ArgIndex<2>>]>;
  def int_x86_tcvtps2ph : ClangBuiltin<"__builtin_ia32_tcvtps2ph">,
                          Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty],
                                    [ImmArg<ArgIndex<2>>]>;
}
#endif // INTEL_FEATURE_ISA_AMX_CONVERT

#if INTEL_FEATURE_ISA_AMX_TILE2
//===----------------------------------------------------------------------===//
let TargetPrefix = "x86" in {
// AMX-TILE2 - Intel TILE2 AMX extensions
  def int_x86_tilemov
      : ClangBuiltin<"__builtin_ia32_tilemov">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
}
#endif // INTEL_FEATURE_ISA_AMX_TILE2

#if INTEL_FEATURE_ISA_AMX_MEMADVISE
//===----------------------------------------------------------------------===//
let TargetPrefix = "x86" in {
  def int_x86_tmovadvise_load
      : ClangBuiltin<"__builtin_ia32_tmovadvise_load">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<3>>]>;
  def int_x86_tmovadvise_store
      : ClangBuiltin<"__builtin_ia32_tmovadvise_store">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
  def int_x86_t2rpntlvwz0advise
      : ClangBuiltin<"__builtin_ia32_t2rpntlvwz0advise">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<3>>]>;
  def int_x86_t2rpntlvwz1advise
      : ClangBuiltin<"__builtin_ia32_t2rpntlvwz1advise">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<3>>]>;
}
#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE

#if INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX
let TargetPrefix = "x86" in {
  def int_x86_t2rpntlvwz0advisee
      : ClangBuiltin<"__builtin_ia32_t2rpntlvwz0advisee">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty, llvm_i32_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<3>>]>;
  def int_x86_t2rpntlvwz1advisee
      : ClangBuiltin<"__builtin_ia32_t2rpntlvwz1advisee">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty, llvm_i32_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<3>>]>;
}
#endif // INTEL_FEATURE_ISA_AMX_MEMADVISE_EVEX

#if INTEL_FEATURE_ISA_AMX_COMPLEX
let TargetPrefix = "x86" in {
  def int_x86_tcmmimfp16ps : ClangBuiltin<"__builtin_ia32_tcmmimfp16ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_tcmmrlfp16ps : ClangBuiltin<"__builtin_ia32_tcmmrlfp16ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_tconjtcmmimfp16ps : ClangBuiltin<"__builtin_ia32_tconjtcmmimfp16ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_tconjtfp16 : ClangBuiltin<"__builtin_ia32_tconjtfp16">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
  def int_x86_ttcmmimfp16ps : ClangBuiltin<"__builtin_ia32_ttcmmimfp16ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_ttcmmrlfp16ps : ClangBuiltin<"__builtin_ia32_ttcmmrlfp16ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;

  def int_x86_tcmmimfp16ps_internal :
              ClangBuiltin<"__builtin_ia32_tcmmimfp16ps_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
  def int_x86_tcmmrlfp16ps_internal :
              ClangBuiltin<"__builtin_ia32_tcmmrlfp16ps_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
  def int_x86_tconjtcmmimfp16ps_internal :
              ClangBuiltin<"__builtin_ia32_tconjtcmmimfp16ps_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
  def int_x86_tconjtfp16_internal :
              ClangBuiltin<"__builtin_ia32_tconjtfp16_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty], []>;
  def int_x86_ttcmmimfp16ps_internal :
              ClangBuiltin<"__builtin_ia32_ttcmmimfp16ps_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
  def int_x86_ttcmmrlfp16ps_internal :
              ClangBuiltin<"__builtin_ia32_ttcmmrlfp16ps_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
}
#endif // INTEL_FEATURE_ISA_AMX_COMPLEX

#if INTEL_FEATURE_ISA_AMX_TF32
let TargetPrefix = "x86" in {
  def int_x86_tmmultf32ps : ClangBuiltin<"__builtin_ia32_tmmultf32ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
  def int_x86_ttmmultf32ps : ClangBuiltin<"__builtin_ia32_ttmmultf32ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;

  def int_x86_tmmultf32ps_internal :
              ClangBuiltin<"__builtin_ia32_tmmultf32ps_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
  def int_x86_ttmmultf32ps_internal :
              ClangBuiltin<"__builtin_ia32_ttmmultf32ps_internal">,
              Intrinsic<[llvm_x86amx_ty],
                        [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty,
                         llvm_x86amx_ty, llvm_x86amx_ty,
                         llvm_x86amx_ty], []>;
}
#endif // INTEL_FEATURE_ISA_AMX_TF32

#if INTEL_FEATURE_ISA_GPR_MOVGET
let TargetPrefix = "x86" in {
  def int_x86_movget32 : ClangBuiltin<"__builtin_ia32_movget32">,
                Intrinsic<[llvm_i32_ty], [llvm_ptr_ty], [IntrReadMem]>;
  def int_x86_movget64 : ClangBuiltin<"__builtin_ia32_movget64">,
                Intrinsic<[llvm_i64_ty], [llvm_ptr_ty], [IntrReadMem]>;
}
#endif // INTEL_FEATURE_ISA_GPR_MOVGET
#if INTEL_FEATURE_ISA_MOVGET64B
let TargetPrefix = "x86" in {
  def int_x86_movget64b32 : ClangBuiltin<"__builtin_ia32_movget64b32">,
                Intrinsic<[llvm_i32_ty], [llvm_ptr_ty], [IntrReadMem]>;
  def int_x86_movget64b64 : ClangBuiltin<"__builtin_ia32_movget64b64">,
                Intrinsic<[llvm_i64_ty], [llvm_ptr_ty], [IntrReadMem]>;
}
#endif // INTEL_FEATURE_ISA_MOVGET64B

#if INTEL_FEATURE_ISA_RAO_INT
let TargetPrefix = "x86" in {
// AADD intrinsics
def int_x86_aadd32 : ClangBuiltin<"__builtin_ia32_aadd32">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty],[]>;
def int_x86_aadd64 : ClangBuiltin<"__builtin_ia32_aadd64">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty],[]>;
// AAND intrinsics
def int_x86_aand32 : ClangBuiltin<"__builtin_ia32_aand32">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty], []>;
def int_x86_aand64 : ClangBuiltin<"__builtin_ia32_aand64">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty], []>;
// AOR intrinsics
def int_x86_aor32 : ClangBuiltin<"__builtin_ia32_aor32">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty], []>;
def int_x86_aor64 : ClangBuiltin<"__builtin_ia32_aor64">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty], []>;
// AXOR intrinsics
def int_x86_axor32 : ClangBuiltin<"__builtin_ia32_axor32">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty], []>;
def int_x86_axor64 : ClangBuiltin<"__builtin_ia32_axor64">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty], []>;
}
#endif // INTEL_FEATURE_ISA_RAO_INT

#if INTEL_FEATURE_ISA_AVX_RAO_INT
let TargetPrefix = "x86" in {
def int_x86_vpaaddd128 : ClangBuiltin<"__builtin_ia32_vpaaddd128">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty], []>;
def int_x86_vpaaddd256 : ClangBuiltin<"__builtin_ia32_vpaaddd256">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v8i32_ty], []>;
def int_x86_vpaaddq128 : ClangBuiltin<"__builtin_ia32_vpaaddq128">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v2i64_ty], []>;
def int_x86_vpaaddq256 : ClangBuiltin<"__builtin_ia32_vpaaddq256">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v4i64_ty], []>;
def int_x86_vpaandd128 : ClangBuiltin<"__builtin_ia32_vpaandd128">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty], []>;
def int_x86_vpaandd256 : ClangBuiltin<"__builtin_ia32_vpaandd256">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v8i32_ty], []>;
def int_x86_vpaandq128 : ClangBuiltin<"__builtin_ia32_vpaandq128">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v2i64_ty], []>;
def int_x86_vpaandq256 : ClangBuiltin<"__builtin_ia32_vpaandq256">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v4i64_ty], []>;
def int_x86_vpaord128 : ClangBuiltin<"__builtin_ia32_vpaord128">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty], []>;
def int_x86_vpaord256 : ClangBuiltin<"__builtin_ia32_vpaord256">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v8i32_ty], []>;
def int_x86_vpaorq128 : ClangBuiltin<"__builtin_ia32_vpaorq128">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v2i64_ty], []>;
def int_x86_vpaorq256 : ClangBuiltin<"__builtin_ia32_vpaorq256">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v4i64_ty], []>;
def int_x86_vpaxord128 : ClangBuiltin<"__builtin_ia32_vpaxord128">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty], []>;
def int_x86_vpaxord256 : ClangBuiltin<"__builtin_ia32_vpaxord256">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v8i32_ty], []>;
def int_x86_vpaxorq128 : ClangBuiltin<"__builtin_ia32_vpaxorq128">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v2i64_ty], []>;
def int_x86_vpaxorq256 : ClangBuiltin<"__builtin_ia32_vpaxorq256">,
      Intrinsic<[], [llvm_ptr_ty, llvm_v4i64_ty], []>;
}
#endif // INTEL_FEATURE_ISA_AVX_RAO_INT

#if INTEL_FEATURE_ISA_AVX_RAO_FP
// AUTO GENERATED BY TOOL
let TargetPrefix = "x86" in {
def int_x86_vaaddpbf16128 : ClangBuiltin<"__builtin_ia32_vaaddpbf16128">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i16_ty], []>;
def int_x86_vaaddpbf16256 : ClangBuiltin<"__builtin_ia32_vaaddpbf16256">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16i16_ty], []>;
def int_x86_vaaddpd128 : ClangBuiltin<"__builtin_ia32_vaaddpd128">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v2f64_ty], []>;
def int_x86_vaaddpd256 : ClangBuiltin<"__builtin_ia32_vaaddpd256">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4f64_ty], []>;
def int_x86_vaaddph128 : ClangBuiltin<"__builtin_ia32_vaaddph128">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8f16_ty], []>;
def int_x86_vaaddph256 : ClangBuiltin<"__builtin_ia32_vaaddph256">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16f16_ty], []>;
def int_x86_vaaddps128 : ClangBuiltin<"__builtin_ia32_vaaddps128">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4f32_ty], []>;
def int_x86_vaaddps256 : ClangBuiltin<"__builtin_ia32_vaaddps256">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8f32_ty], []>;
def int_x86_vaaddsbf16128 : ClangBuiltin<"__builtin_ia32_vaaddsbf16">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i16_ty], []>;
def int_x86_vaaddsd128 : ClangBuiltin<"__builtin_ia32_vaaddsd">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v2f64_ty], []>;
def int_x86_vaaddsh128 : ClangBuiltin<"__builtin_ia32_vaaddsh">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8f16_ty], []>;
def int_x86_vaaddss128 : ClangBuiltin<"__builtin_ia32_vaaddss">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4f32_ty], []>;
}
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_RAO_FP

#if INTEL_FEATURE_ISA_AVX512_RAO_INT
// AUTO GENERATED BY TOOL
let TargetPrefix = "x86" in {
def int_x86_mask_vpaaddd128 : ClangBuiltin<"__builtin_ia32_vpaaddd128_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty], []>;
def int_x86_mask_vpaaddd256 : ClangBuiltin<"__builtin_ia32_vpaaddd256_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i32_ty, llvm_i8_ty], []>;
def int_x86_vpaaddd512 : ClangBuiltin<"__builtin_ia32_vpaaddd512">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16i32_ty], []>;
def int_x86_mask_vpaaddd512 : ClangBuiltin<"__builtin_ia32_vpaaddd512_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16i32_ty, llvm_i16_ty], []>;
def int_x86_mask_vpaaddq128 : ClangBuiltin<"__builtin_ia32_vpaaddq128_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty], []>;
def int_x86_mask_vpaaddq256 : ClangBuiltin<"__builtin_ia32_vpaaddq256_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty], []>;
def int_x86_vpaaddq512 : ClangBuiltin<"__builtin_ia32_vpaaddq512">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i64_ty], []>;
def int_x86_mask_vpaaddq512 : ClangBuiltin<"__builtin_ia32_vpaaddq512_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i64_ty, llvm_i8_ty], []>;
def int_x86_mask_vpaandd128 : ClangBuiltin<"__builtin_ia32_vpaandd128_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty], []>;
def int_x86_mask_vpaandd256 : ClangBuiltin<"__builtin_ia32_vpaandd256_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i32_ty, llvm_i8_ty], []>;
def int_x86_vpaandd512 : ClangBuiltin<"__builtin_ia32_vpaandd512">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16i32_ty], []>;
def int_x86_mask_vpaandd512 : ClangBuiltin<"__builtin_ia32_vpaandd512_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16i32_ty, llvm_i16_ty], []>;
def int_x86_mask_vpaandq128 : ClangBuiltin<"__builtin_ia32_vpaandq128_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty], []>;
def int_x86_mask_vpaandq256 : ClangBuiltin<"__builtin_ia32_vpaandq256_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty], []>;
def int_x86_vpaandq512 : ClangBuiltin<"__builtin_ia32_vpaandq512">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i64_ty], []>;
def int_x86_mask_vpaandq512 : ClangBuiltin<"__builtin_ia32_vpaandq512_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i64_ty, llvm_i8_ty], []>;
def int_x86_mask_vpaord128 : ClangBuiltin<"__builtin_ia32_vpaord128_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty], []>;
def int_x86_mask_vpaord256 : ClangBuiltin<"__builtin_ia32_vpaord256_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i32_ty, llvm_i8_ty], []>;
def int_x86_vpaord512 : ClangBuiltin<"__builtin_ia32_vpaord512">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16i32_ty], []>;
def int_x86_mask_vpaord512 : ClangBuiltin<"__builtin_ia32_vpaord512_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16i32_ty, llvm_i16_ty], []>;
def int_x86_mask_vpaorq128 : ClangBuiltin<"__builtin_ia32_vpaorq128_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty], []>;
def int_x86_mask_vpaorq256 : ClangBuiltin<"__builtin_ia32_vpaorq256_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty], []>;
def int_x86_vpaorq512 : ClangBuiltin<"__builtin_ia32_vpaorq512">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i64_ty], []>;
def int_x86_mask_vpaorq512 : ClangBuiltin<"__builtin_ia32_vpaorq512_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i64_ty, llvm_i8_ty], []>;
def int_x86_mask_vpaxord128 : ClangBuiltin<"__builtin_ia32_vpaxord128_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4i32_ty, llvm_i8_ty], []>;
def int_x86_mask_vpaxord256 : ClangBuiltin<"__builtin_ia32_vpaxord256_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i32_ty, llvm_i8_ty], []>;
def int_x86_vpaxord512 : ClangBuiltin<"__builtin_ia32_vpaxord512">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16i32_ty], []>;
def int_x86_mask_vpaxord512 : ClangBuiltin<"__builtin_ia32_vpaxord512_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16i32_ty, llvm_i16_ty], []>;
def int_x86_mask_vpaxorq128 : ClangBuiltin<"__builtin_ia32_vpaxorq128_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v2i64_ty, llvm_i8_ty], []>;
def int_x86_mask_vpaxorq256 : ClangBuiltin<"__builtin_ia32_vpaxorq256_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4i64_ty, llvm_i8_ty], []>;
def int_x86_vpaxorq512 : ClangBuiltin<"__builtin_ia32_vpaxorq512">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i64_ty], []>;
def int_x86_mask_vpaxorq512 : ClangBuiltin<"__builtin_ia32_vpaxorq512_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i64_ty, llvm_i8_ty], []>;
}
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_RAO_INT

#if INTEL_FEATURE_ISA_AVX512_RAO_FP
// AUTO GENERATED BY TOOL
let TargetPrefix = "x86" in {
def int_x86_mask_vaaddpbf16128 : ClangBuiltin<"__builtin_ia32_vaaddpbf16128_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i16_ty, llvm_i8_ty], []>;
def int_x86_mask_vaaddpbf16256 : ClangBuiltin<"__builtin_ia32_vaaddpbf16256_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16i16_ty, llvm_i16_ty], []>;
def int_x86_vaaddpbf16512 : ClangBuiltin<"__builtin_ia32_vaaddpbf16512">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v32i16_ty], []>;
def int_x86_mask_vaaddpbf16512 : ClangBuiltin<"__builtin_ia32_vaaddpbf16512_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v32i16_ty, llvm_i32_ty], []>;
def int_x86_mask_vaaddpd128 : ClangBuiltin<"__builtin_ia32_vaaddpd128_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v2f64_ty, llvm_i8_ty], []>;
def int_x86_mask_vaaddpd256 : ClangBuiltin<"__builtin_ia32_vaaddpd256_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4f64_ty, llvm_i8_ty], []>;
def int_x86_vaaddpd512 : ClangBuiltin<"__builtin_ia32_vaaddpd512">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8f64_ty], []>;
def int_x86_mask_vaaddpd512 : ClangBuiltin<"__builtin_ia32_vaaddpd512_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8f64_ty, llvm_i8_ty], []>;
def int_x86_mask_vaaddph128 : ClangBuiltin<"__builtin_ia32_vaaddph128_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8f16_ty, llvm_i8_ty], []>;
def int_x86_mask_vaaddph256 : ClangBuiltin<"__builtin_ia32_vaaddph256_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16f16_ty, llvm_i16_ty], []>;
def int_x86_vaaddph512 : ClangBuiltin<"__builtin_ia32_vaaddph512">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v32f16_ty], []>;
def int_x86_mask_vaaddph512 : ClangBuiltin<"__builtin_ia32_vaaddph512_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v32f16_ty, llvm_i32_ty], []>;
def int_x86_mask_vaaddps128 : ClangBuiltin<"__builtin_ia32_vaaddps128_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4f32_ty, llvm_i8_ty], []>;
def int_x86_mask_vaaddps256 : ClangBuiltin<"__builtin_ia32_vaaddps256_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8f32_ty, llvm_i8_ty], []>;
def int_x86_vaaddps512 : ClangBuiltin<"__builtin_ia32_vaaddps512">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16f32_ty], []>;
def int_x86_mask_vaaddps512 : ClangBuiltin<"__builtin_ia32_vaaddps512_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16f32_ty, llvm_i16_ty], []>;
def int_x86_mask_vaaddsbf16128 : ClangBuiltin<"__builtin_ia32_vaaddsbf16_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8i16_ty, llvm_i8_ty], []>;
def int_x86_mask_vaaddsd128 : ClangBuiltin<"__builtin_ia32_vaaddsd_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v2f64_ty, llvm_i8_ty], []>;
def int_x86_mask_vaaddsh128 : ClangBuiltin<"__builtin_ia32_vaaddsh_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v8f16_ty, llvm_i8_ty], []>;
def int_x86_mask_vaaddss128 : ClangBuiltin<"__builtin_ia32_vaaddss_mask">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v4f32_ty, llvm_i8_ty], []>;
}
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_RAO_FP
#if INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
// AUTO GENERATED BY TOOL
let TargetPrefix = "x86" in {
def int_x86_tcvtrowd2psi : ClangBuiltin<"__builtin_ia32_tcvtrowd2psi">,
        Intrinsic<[llvm_v16f32_ty], [llvm_i8_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
def int_x86_tcvtrowd2pse : ClangBuiltin<"__builtin_ia32_tcvtrowd2pse">,
        Intrinsic<[llvm_v16f32_ty], [llvm_i8_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<0>>]>;
def int_x86_tcvtrowps2pbf16he : ClangBuiltin<"__builtin_ia32_tcvtrowps2pbf16he">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i8_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<0>>]>;
def int_x86_tcvtrowps2pbf16hi : ClangBuiltin<"__builtin_ia32_tcvtrowps2pbf16hi">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i8_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
def int_x86_tcvtrowps2pbf16le : ClangBuiltin<"__builtin_ia32_tcvtrowps2pbf16le">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i8_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<0>>]>;
def int_x86_tcvtrowps2pbf16li : ClangBuiltin<"__builtin_ia32_tcvtrowps2pbf16li">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i8_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
def int_x86_tcvtrowps2phhe : ClangBuiltin<"__builtin_ia32_tcvtrowps2phhe">,
        Intrinsic<[llvm_v32f16_ty], [llvm_i8_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<0>>]>;
def int_x86_tcvtrowps2phhi : ClangBuiltin<"__builtin_ia32_tcvtrowps2phhi">,
        Intrinsic<[llvm_v32f16_ty], [llvm_i8_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;
def int_x86_tcvtrowps2phle : ClangBuiltin<"__builtin_ia32_tcvtrowps2phle">,
        Intrinsic<[llvm_v32f16_ty], [llvm_i8_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<0>>]>;
def int_x86_tcvtrowps2phli : ClangBuiltin<"__builtin_ia32_tcvtrowps2phli">,
        Intrinsic<[llvm_v32f16_ty], [llvm_i8_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>]>;

def int_x86_tcvtrowd2psi_internal :
            ClangBuiltin<"__builtin_ia32_tcvtrowd2psi_internal">,
            Intrinsic<[llvm_v16f32_ty],
                      [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty, llvm_i32_ty],
                      [ImmArg<ArgIndex<3>>]>;
def int_x86_tcvtrowd2pse_internal :
            ClangBuiltin<"__builtin_ia32_tcvtrowd2pse_internal">,
            Intrinsic<[llvm_v16f32_ty],
                      [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty, llvm_i32_ty],
                      []>;
def int_x86_tcvtrowps2pbf16hi_internal :
            ClangBuiltin<"__builtin_ia32_tcvtrowps2pbf16hi_internal">,
            Intrinsic<[llvm_v32i16_ty],
                      [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty, llvm_i32_ty],
                      [ImmArg<ArgIndex<3>>]>;
def int_x86_tcvtrowps2pbf16he_internal :
            ClangBuiltin<"__builtin_ia32_tcvtrowps2pbf16he_internal">,
            Intrinsic<[llvm_v32i16_ty],
                      [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty, llvm_i32_ty],
                      []>;
def int_x86_tcvtrowps2pbf16li_internal :
            ClangBuiltin<"__builtin_ia32_tcvtrowps2pbf16li_internal">,
            Intrinsic<[llvm_v32i16_ty],
                      [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty, llvm_i32_ty],
                      [ImmArg<ArgIndex<3>>]>;
def int_x86_tcvtrowps2pbf16le_internal :
            ClangBuiltin<"__builtin_ia32_tcvtrowps2pbf16le_internal">,
            Intrinsic<[llvm_v32i16_ty],
                      [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty, llvm_i32_ty],
                      []>;
def int_x86_tcvtrowps2phhi_internal :
            ClangBuiltin<"__builtin_ia32_tcvtrowps2phhi_internal">,
            Intrinsic<[llvm_v32f16_ty],
                      [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty, llvm_i32_ty],
                      [ImmArg<ArgIndex<3>>]>;
def int_x86_tcvtrowps2phhe_internal :
            ClangBuiltin<"__builtin_ia32_tcvtrowps2phhe_internal">,
            Intrinsic<[llvm_v32f16_ty],
                      [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty, llvm_i32_ty],
                      []>;
def int_x86_tcvtrowps2phli_internal :
            ClangBuiltin<"__builtin_ia32_tcvtrowps2phli_internal">,
            Intrinsic<[llvm_v32f16_ty],
                      [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty, llvm_i32_ty],
                      [ImmArg<ArgIndex<3>>]>;
def int_x86_tcvtrowps2phle_internal :
            ClangBuiltin<"__builtin_ia32_tcvtrowps2phle_internal">,
            Intrinsic<[llvm_v32f16_ty],
                      [llvm_i16_ty, llvm_i16_ty, llvm_x86amx_ty, llvm_i32_ty],
                      []>;
}
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_AVX512_CVTROW
#if INTEL_FEATURE_ISA_AVX_NE_CONVERT
// AUTO GENERATED BY TOOL
let TargetPrefix = "x86" in {
def int_x86_avx512_vbcstnebf162ps128 : ClangBuiltin<"__builtin_ia32_vbcstnebf162ps128">,
        Intrinsic<[llvm_v4f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vbcstnebf162ps256 : ClangBuiltin<"__builtin_ia32_vbcstnebf162ps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vbcstnesh2ps128 : ClangBuiltin<"__builtin_ia32_vbcstnesh2ps128">,
        Intrinsic<[llvm_v4f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vbcstnesh2ps256 : ClangBuiltin<"__builtin_ia32_vbcstnesh2ps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vcvtneebf162ps128 : ClangBuiltin<"__builtin_ia32_vcvtneebf162ps128">,
        Intrinsic<[llvm_v4f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vcvtneebf162ps256 : ClangBuiltin<"__builtin_ia32_vcvtneebf162ps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vcvtneeph2ps128 : ClangBuiltin<"__builtin_ia32_vcvtneeph2ps128">,
        Intrinsic<[llvm_v4f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vcvtneeph2ps256 : ClangBuiltin<"__builtin_ia32_vcvtneeph2ps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vcvtneobf162ps128 : ClangBuiltin<"__builtin_ia32_vcvtneobf162ps128">,
        Intrinsic<[llvm_v4f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vcvtneobf162ps256 : ClangBuiltin<"__builtin_ia32_vcvtneobf162ps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vcvtneoph2ps128 : ClangBuiltin<"__builtin_ia32_vcvtneoph2ps128">,
        Intrinsic<[llvm_v4f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vcvtneoph2ps256 : ClangBuiltin<"__builtin_ia32_vcvtneoph2ps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx2_vcvtneps2bf16128 : ClangBuiltin<"__builtin_ia32_vcvtneps2bf16128">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v4f32_ty], [IntrNoMem]>;
def int_x86_avx2_vcvtneps2bf16256 : ClangBuiltin<"__builtin_ia32_vcvtneps2bf16256">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8f32_ty], [IntrNoMem]>;
}
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_NE_CONVERT
#if INTEL_FEATURE_ISA_AVX512_NE_CONVERT
let TargetPrefix = "x86" in {
def int_x86_avx512_mask_vbcstnebf162ps128 :
        ClangBuiltin<"__builtin_ia32_vbcstnebf162ps128_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_i8_ty, llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_mask_vbcstnebf162ps256 :
        ClangBuiltin<"__builtin_ia32_vbcstnebf162ps256_mask">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_i8_ty, llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_maskz_vbcstnebf162ps128 :
        ClangBuiltin<"__builtin_ia32_vbcstnebf162ps128_maskz">,
        Intrinsic<[llvm_v4f32_ty], [llvm_i8_ty, llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_maskz_vbcstnebf162ps256 :
        ClangBuiltin<"__builtin_ia32_vbcstnebf162ps256_maskz">,
        Intrinsic<[llvm_v8f32_ty], [llvm_i8_ty, llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vbcstnebf162ps512 : ClangBuiltin<"__builtin_ia32_vbcstnebf162ps512">,
        Intrinsic<[llvm_v16f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_mask_vbcstnebf162ps512 :
        ClangBuiltin<"__builtin_ia32_vbcstnebf162ps512_mask">,
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_i16_ty, llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_maskz_vbcstnebf162ps512 :
        ClangBuiltin<"__builtin_ia32_vbcstnebf162ps512_maskz">,
        Intrinsic<[llvm_v16f32_ty], [llvm_i16_ty, llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_mask_vbcstnesh2ps128 :
        ClangBuiltin<"__builtin_ia32_vbcstnesh2ps128_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_i8_ty, llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_mask_vbcstnesh2ps256 :
        ClangBuiltin<"__builtin_ia32_vbcstnesh2ps256_mask">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_i8_ty, llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_maskz_vbcstnesh2ps128 :
        ClangBuiltin<"__builtin_ia32_vbcstnesh2ps128_maskz">,
        Intrinsic<[llvm_v4f32_ty], [llvm_i8_ty, llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_maskz_vbcstnesh2ps256 :
        ClangBuiltin<"__builtin_ia32_vbcstnesh2ps256_maskz">,
        Intrinsic<[llvm_v8f32_ty], [llvm_i8_ty, llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vbcstnesh2ps512 :
        ClangBuiltin<"__builtin_ia32_vbcstnesh2ps512">,
        Intrinsic<[llvm_v16f32_ty], [llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_mask_vbcstnesh2ps512 :
        ClangBuiltin<"__builtin_ia32_vbcstnesh2ps512_mask">,
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_i16_ty, llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_maskz_vbcstnesh2ps512 :
        ClangBuiltin<"__builtin_ia32_vbcstnesh2ps512_maskz">,
        Intrinsic<[llvm_v16f32_ty], [llvm_i16_ty, llvm_ptr_ty], [IntrReadMem]>;
def int_x86_avx512_vcvtne2ps2ph128 :
        ClangBuiltin<"__builtin_ia32_vcvtne2ps2ph128">,
        Intrinsic<[llvm_v8f16_ty], [llvm_v4f32_ty, llvm_v4f32_ty], [IntrNoMem]>;
def int_x86_avx512_vcvtne2ps2ph256 : ClangBuiltin<"__builtin_ia32_vcvtne2ps2ph256">,
        Intrinsic<[llvm_v16f16_ty], [llvm_v8f32_ty, llvm_v8f32_ty], [IntrNoMem]>;
def int_x86_avx512_vcvtne2ps2ph512 : ClangBuiltin<"__builtin_ia32_vcvtne2ps2ph512">,
        Intrinsic<[llvm_v32f16_ty], [llvm_v16f32_ty, llvm_v16f32_ty, llvm_i32_ty],
        [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512_mask_vcvtneebf162ps128 :
        ClangBuiltin<"__builtin_ia32_vcvtneebf162ps128_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_mask_vcvtneebf162ps256 :
        ClangBuiltin<"__builtin_ia32_vcvtneebf162ps256_mask">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_maskz_vcvtneebf162ps128 :
        ClangBuiltin<"__builtin_ia32_vcvtneebf162ps128_maskz">,
        Intrinsic<[llvm_v4f32_ty], [llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_maskz_vcvtneebf162ps256 :
        ClangBuiltin<"__builtin_ia32_vcvtneebf162ps256_maskz">,
        Intrinsic<[llvm_v8f32_ty], [llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_vcvtneebf162ps512 :
        ClangBuiltin<"__builtin_ia32_vcvtneebf162ps512">,
        Intrinsic<[llvm_v16f32_ty], [llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_mask_vcvtneebf162ps512 :
        ClangBuiltin<"__builtin_ia32_vcvtneebf162ps512_mask">,
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_i16_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_maskz_vcvtneebf162ps512 :
        ClangBuiltin<"__builtin_ia32_vcvtneebf162ps512_maskz">,
        Intrinsic<[llvm_v16f32_ty], [llvm_i16_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_mask_vcvtneeph2ps128 :
        ClangBuiltin<"__builtin_ia32_vcvtneeph2ps128_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_mask_vcvtneeph2ps256 :
        ClangBuiltin<"__builtin_ia32_vcvtneeph2ps256_mask">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_maskz_vcvtneeph2ps128 :
        ClangBuiltin<"__builtin_ia32_vcvtneeph2ps128_maskz">,
        Intrinsic<[llvm_v4f32_ty], [llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_maskz_vcvtneeph2ps256 :
        ClangBuiltin<"__builtin_ia32_vcvtneeph2ps256_maskz">,
        Intrinsic<[llvm_v8f32_ty], [llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_vcvtneeph2ps512 :
        ClangBuiltin<"__builtin_ia32_vcvtneeph2ps512">,
        Intrinsic<[llvm_v16f32_ty], [llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_mask_vcvtneeph2ps512 :
        ClangBuiltin<"__builtin_ia32_vcvtneeph2ps512_mask">,
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_i16_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_maskz_vcvtneeph2ps512 :
        ClangBuiltin<"__builtin_ia32_vcvtneeph2ps512_maskz">,
        Intrinsic<[llvm_v16f32_ty], [llvm_i16_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_mask_vcvtneobf162ps128 :
        ClangBuiltin<"__builtin_ia32_vcvtneobf162ps128_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_mask_vcvtneobf162ps256 :
        ClangBuiltin<"__builtin_ia32_vcvtneobf162ps256_mask">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_maskz_vcvtneobf162ps128 :
        ClangBuiltin<"__builtin_ia32_vcvtneobf162ps128_maskz">,
        Intrinsic<[llvm_v4f32_ty], [llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_maskz_vcvtneobf162ps256 :
        ClangBuiltin<"__builtin_ia32_vcvtneobf162ps256_maskz">,
        Intrinsic<[llvm_v8f32_ty], [llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_vcvtneobf162ps512 :
        ClangBuiltin<"__builtin_ia32_vcvtneobf162ps512">,
        Intrinsic<[llvm_v16f32_ty], [llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_mask_vcvtneobf162ps512 :
        ClangBuiltin<"__builtin_ia32_vcvtneobf162ps512_mask">,
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_i16_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_maskz_vcvtneobf162ps512 :
        ClangBuiltin<"__builtin_ia32_vcvtneobf162ps512_maskz">,
        Intrinsic<[llvm_v16f32_ty], [llvm_i16_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_mask_vcvtneoph2ps128 :
        ClangBuiltin<"__builtin_ia32_vcvtneoph2ps128_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_mask_vcvtneoph2ps256 :
        ClangBuiltin<"__builtin_ia32_vcvtneoph2ps256_mask">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_maskz_vcvtneoph2ps128 :
        ClangBuiltin<"__builtin_ia32_vcvtneoph2ps128_maskz">,
        Intrinsic<[llvm_v4f32_ty], [llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_maskz_vcvtneoph2ps256 :
        ClangBuiltin<"__builtin_ia32_vcvtneoph2ps256_maskz">,
        Intrinsic<[llvm_v8f32_ty], [llvm_i8_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_vcvtneoph2ps512 :
        ClangBuiltin<"__builtin_ia32_vcvtneoph2ps512">,
        Intrinsic<[llvm_v16f32_ty], [llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_mask_vcvtneoph2ps512 :
        ClangBuiltin<"__builtin_ia32_vcvtneoph2ps512_mask">,
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_i16_ty, llvm_ptr_ty],
        [IntrReadMem]>;
def int_x86_avx512_maskz_vcvtneoph2ps512 :
        ClangBuiltin<"__builtin_ia32_vcvtneoph2ps512_maskz">,
        Intrinsic<[llvm_v16f32_ty], [llvm_i16_ty, llvm_ptr_ty],
        [IntrReadMem]>;
}
#endif // INTEL_FEATURE_ISA_AVX512_NE_CONVERT

#if INTEL_FEATURE_ISA_SHA512
// AUTO GENERATED BY TOOL
let TargetPrefix = "x86" in {
def int_x86_vsha512msg1 : ClangBuiltin<"__builtin_ia32_vsha512msg1">,
        Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v2i64_ty], [IntrNoMem]>;
def int_x86_vsha512msg2 : ClangBuiltin<"__builtin_ia32_vsha512msg2">,
        Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty], [IntrNoMem]>;
def int_x86_vsha512rnds2 : ClangBuiltin<"__builtin_ia32_vsha512rnds2">,
        Intrinsic<[llvm_v4i64_ty], [llvm_v4i64_ty, llvm_v4i64_ty, llvm_v2i64_ty],
        [IntrNoMem]>;
}
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SHA512
#if INTEL_FEATURE_ISA_SM3
// AUTO GENERATED BY TOOL
let TargetPrefix = "x86" in {
def int_x86_vsm3msg1 : ClangBuiltin<"__builtin_ia32_vsm3msg1">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
        [IntrNoMem]>;
def int_x86_vsm3msg2 : ClangBuiltin<"__builtin_ia32_vsm3msg2">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
        [IntrNoMem]>;
def int_x86_vsm3rnds2 : ClangBuiltin<"__builtin_ia32_vsm3rnds2">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty, llvm_i32_ty],
        [ImmArg<ArgIndex<3>>, IntrNoMem]>;
}
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SM3

#if INTEL_FEATURE_ISA_SM4
// AUTO GENERATED BY TOOL
let TargetPrefix = "x86" in {
def int_x86_vsm4key4128 : ClangBuiltin<"__builtin_ia32_vsm4key4128">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
        [IntrNoMem]>;
def int_x86_vsm4key4256 : ClangBuiltin<"__builtin_ia32_vsm4key4256">,
        Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
        [IntrNoMem]>;
def int_x86_vsm4key4512 : ClangBuiltin<"__builtin_ia32_vsm4key4512">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
        [IntrNoMem]>;
def int_x86_vsm4rnds4128 : ClangBuiltin<"__builtin_ia32_vsm4rnds4128">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
        [IntrNoMem]>;
def int_x86_vsm4rnds4256 : ClangBuiltin<"__builtin_ia32_vsm4rnds4256">,
        Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty],
        [IntrNoMem]>;
def int_x86_vsm4rnds4512 : ClangBuiltin<"__builtin_ia32_vsm4rnds4512">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty],
        [IntrNoMem]>;
}
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_SM4

#if INTEL_FEATURE_ISA_AVX_VNNI_INT16
// AUTO GENERATED BY TOOL
let TargetPrefix = "x86" in {
def int_x86_vpdpwsud128 : ClangBuiltin<"__builtin_ia32_vpdpwsud128">,
      Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                [IntrNoMem]>;
def int_x86_vpdpwsud256 : ClangBuiltin<"__builtin_ia32_vpdpwsud256">,
      Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
                [IntrNoMem]>;
def int_x86_vpdpwsuds128 : ClangBuiltin<"__builtin_ia32_vpdpwsuds128">,
      Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                [IntrNoMem]>;
def int_x86_vpdpwsuds256 : ClangBuiltin<"__builtin_ia32_vpdpwsuds256">,
      Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
                [IntrNoMem]>;
def int_x86_vpdpwusd128 : ClangBuiltin<"__builtin_ia32_vpdpwusd128">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_vpdpwusd256 : ClangBuiltin<"__builtin_ia32_vpdpwusd256">,
      Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
                [IntrNoMem]>;
def int_x86_vpdpwusds128 : ClangBuiltin<"__builtin_ia32_vpdpwusds128">,
      Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                [IntrNoMem]>;
def int_x86_vpdpwusds256 : ClangBuiltin<"__builtin_ia32_vpdpwusds256">,
      Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
                [IntrNoMem]>;
def int_x86_vpdpwuud128 : ClangBuiltin<"__builtin_ia32_vpdpwuud128">,
      Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                [IntrNoMem]>;
def int_x86_vpdpwuud256 : ClangBuiltin<"__builtin_ia32_vpdpwuud256">,
      Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
                [IntrNoMem]>;
def int_x86_vpdpwuuds128 : ClangBuiltin<"__builtin_ia32_vpdpwuuds128">,
      Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                [IntrNoMem]>;
def int_x86_vpdpwuuds256 : ClangBuiltin<"__builtin_ia32_vpdpwuuds256">,
      Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_v8i32_ty, llvm_v8i32_ty],
                [IntrNoMem]>;
}
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX_VNNI_INT16

#if INTEL_FEATURE_ISA_AVX512_VNNI_INT16
let TargetPrefix = "x86" in {
def int_x86_vpdpwsud512 : ClangBuiltin<"__builtin_ia32_vpdpwsud512">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
def int_x86_vpdpwsuds512 : ClangBuiltin<"__builtin_ia32_vpdpwsuds512">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
def int_x86_vpdpwusd512 : ClangBuiltin<"__builtin_ia32_vpdpwusd512">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
def int_x86_vpdpwusds512 : ClangBuiltin<"__builtin_ia32_vpdpwusds512">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
def int_x86_vpdpwuud512 : ClangBuiltin<"__builtin_ia32_vpdpwuud512">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
def int_x86_vpdpwuuds512 : ClangBuiltin<"__builtin_ia32_vpdpwuuds512">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_v16i32_ty, llvm_v16i32_ty],
                  [IntrNoMem]>;
}
#endif // INTEL_FEATURE_ISA_AVX512_VNNI_INT16

#if INTEL_FEATURE_ISA_AMX_SPARSE
// AUTO GENERATED BY TOOL
let TargetPrefix = "x86" in {
def int_x86_tdpsbssd : ClangBuiltin<"__builtin_ia32_tdpsbssd">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_tdpsbsud : ClangBuiltin<"__builtin_ia32_tdpsbsud">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_tdpsbusd : ClangBuiltin<"__builtin_ia32_tdpsbusd">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_tdpsbuud : ClangBuiltin<"__builtin_ia32_tdpsbuud">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_tdpsbf16ps : ClangBuiltin<"__builtin_ia32_tdpsbf16ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_tdpsfp16ps : ClangBuiltin<"__builtin_ia32_tdpsfp16ps">,
        Intrinsic<[], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<0>>, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_tldexpandb : ClangBuiltin<"__builtin_ia32_tldexpandb">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty, llvm_i32_ty],
                  [ImmArg<ArgIndex<0>>]>;
def int_x86_tldexpandbt1 : ClangBuiltin<"__builtin_ia32_tldexpandbt1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty, llvm_i32_ty],
                  [ImmArg<ArgIndex<0>>]>;
def int_x86_tldexpandw : ClangBuiltin<"__builtin_ia32_tldexpandw">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty, llvm_i32_ty],
                  [ImmArg<ArgIndex<0>>]>;
def int_x86_tldexpandwt1 : ClangBuiltin<"__builtin_ia32_tldexpandwt1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty, llvm_i32_ty],
                  [ImmArg<ArgIndex<0>>]>;
}
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_SPARSE

#if INTEL_FEATURE_ISA_AMX_V3
// AUTO GENERATED BY TOOL
let TargetPrefix = "x86" in {
def int_x86_tloadtransposed : ClangBuiltin<"__builtin_ia32_tloadtransposed">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
def int_x86_tloadtransposedt1 : ClangBuiltin<"__builtin_ia32_tloadtransposedt1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
def int_x86_trpntlvwz0 : ClangBuiltin<"__builtin_ia32_trpntlvwz0">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
def int_x86_trpntlvwz0t1 : ClangBuiltin<"__builtin_ia32_trpntlvwz0t1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
def int_x86_trpntlvwz1 : ClangBuiltin<"__builtin_ia32_trpntlvwz1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
def int_x86_trpntlvwz1t1 : ClangBuiltin<"__builtin_ia32_trpntlvwz1t1">,
        Intrinsic<[], [llvm_i8_ty, llvm_ptr_ty, llvm_i64_ty],
                  [ImmArg<ArgIndex<0>>]>;
def int_x86_tstoretransposed : ClangBuiltin<"__builtin_ia32_tstoretransposed">,
        Intrinsic<[], [llvm_ptr_ty, llvm_i64_ty, llvm_i8_ty],
                  [ImmArg<ArgIndex<2>>]>;
}
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AMX_V3

#if INTEL_FEATURE_ISA_DSPV1
// AUTO GENERATED BY TOOL
let TargetPrefix = "x86" in {
def int_x86_dvpcr2bfrsw : ClangBuiltin<"__builtin_ia32_dvpcr2bfrsw">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty, llvm_v8i16_ty,
                  llvm_i32_ty], [ImmArg<ArgIndex<3>>, IntrNoMem]>;
def int_x86_dvplutsincosw : ClangBuiltin<"__builtin_ia32_dvplutsincosw">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_i32_ty],
                  [ImmArg<ArgIndex<1>>, IntrNoMem]>;
// Chapter 2
def int_x86_dvpmuluwr : ClangBuiltin<"__builtin_ia32_dvpmuluwr">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvpmulwr : ClangBuiltin<"__builtin_ia32_dvpmulwr">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvpmulws : ClangBuiltin<"__builtin_ia32_dvpmulws">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvpmulwfrs : ClangBuiltin<"__builtin_ia32_dvpmulwfrs">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
// end Chapter 2
// Chapter 3
def int_x86_dvpdpwuuq : ClangBuiltin<"__builtin_ia32_dvpdpwuuq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvpdpwssq : ClangBuiltin<"__builtin_ia32_dvpdpwssq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvpdpwsuq : ClangBuiltin<"__builtin_ia32_dvpdpwsuq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvpdpwusq : ClangBuiltin<"__builtin_ia32_dvpdpwusq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvpndpwssq : ClangBuiltin<"__builtin_ia32_dvpndpwssq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvpcmulwrs : ClangBuiltin<"__builtin_ia32_dvpcmulwrs">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_dvpccmulwrs : ClangBuiltin<"__builtin_ia32_dvpccmulwrs">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_dvpcdpwqre : ClangBuiltin<"__builtin_ia32_dvpcdpwqre">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvpcdpwqimm : ClangBuiltin<"__builtin_ia32_dvpcdpwqimm">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvpncdpwqre : ClangBuiltin<"__builtin_ia32_dvpncdpwqre">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvpncdpwqimm : ClangBuiltin<"__builtin_ia32_dvpncdpwqimm">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvpccdpwqimm : ClangBuiltin<"__builtin_ia32_dvpccdpwqimm">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvpnccdpwqimm : ClangBuiltin<"__builtin_ia32_dvpnccdpwqimm">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v8i16_ty, llvm_v8i16_ty],
                  [IntrNoMem]>;
// end Chapter 3
// Chapter 4
def int_x86_dvpdpbwuud : ClangBuiltin<"__builtin_ia32_dvpdpbwuud">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_dvpdpbwuuds : ClangBuiltin<"__builtin_ia32_dvpdpbwuuds">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_dvpdpbwssd : ClangBuiltin<"__builtin_ia32_dvpdpbwssd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_dvpdpbwssds : ClangBuiltin<"__builtin_ia32_dvpdpbwssds">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_dvpdpbwsud : ClangBuiltin<"__builtin_ia32_dvpdpbwsud">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_dvpdpbwsuds : ClangBuiltin<"__builtin_ia32_dvpdpbwsuds">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_dvpdpbwusd : ClangBuiltin<"__builtin_ia32_dvpdpbwusd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_dvpdpbwusds : ClangBuiltin<"__builtin_ia32_dvpdpbwusds">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
// end Chapter 4
// Chapter 5
def int_x86_dvpmuluwdq : ClangBuiltin<"__builtin_ia32_dvpmuluwdq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v8i16_ty, llvm_v4i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_dvpmulwdq : ClangBuiltin<"__builtin_ia32_dvpmulwdq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v8i16_ty, llvm_v4i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_dvpdpwduuq : ClangBuiltin<"__builtin_ia32_dvpdpwduuq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v8i16_ty, llvm_v4i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_dvpdpwdssq : ClangBuiltin<"__builtin_ia32_dvpdpwdssq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v8i16_ty, llvm_v4i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_dvpcr2bfrsdre : ClangBuiltin<"__builtin_ia32_dvpcr2bfrsdre">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_dvpcr2bfrsdimm : ClangBuiltin<"__builtin_ia32_dvpcr2bfrsdimm">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
// end Chapter 5
// Chapter 6
def int_x86_dvpmuludhhq : ClangBuiltin<"__builtin_ia32_dvpmuludhhq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmuldhhq : ClangBuiltin<"__builtin_ia32_dvpmuldhhq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmuldfrs : ClangBuiltin<"__builtin_ia32_dvpmuldfrs">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmulds : ClangBuiltin<"__builtin_ia32_dvpmulds">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmacudllsq : ClangBuiltin<"__builtin_ia32_dvpmacudllsq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmacudhhsq : ClangBuiltin<"__builtin_ia32_dvpmacudhhsq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmacudllq : ClangBuiltin<"__builtin_ia32_dvpmacudllq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmacudhhq : ClangBuiltin<"__builtin_ia32_dvpmacudhhq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmacdllsq : ClangBuiltin<"__builtin_ia32_dvpmacdllsq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmacdhhsq : ClangBuiltin<"__builtin_ia32_dvpmacdhhsq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmacdllq : ClangBuiltin<"__builtin_ia32_dvpmacdllq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmacdhhq : ClangBuiltin<"__builtin_ia32_dvpmacdhhq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpnmacdllsq : ClangBuiltin<"__builtin_ia32_dvpnmacdllsq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpnmacdhhsq : ClangBuiltin<"__builtin_ia32_dvpnmacdhhsq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpnmacdllq : ClangBuiltin<"__builtin_ia32_dvpnmacdllq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpnmacdhhq : ClangBuiltin<"__builtin_ia32_dvpnmacdhhq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmsubadddllq : ClangBuiltin<"__builtin_ia32_dvpmsubadddllq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v2i64_ty, llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmuludllq : ClangBuiltin<"__builtin_ia32_dvpmuludllq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpmuldllq : ClangBuiltin<"__builtin_ia32_dvpmuldllq">,
        Intrinsic<[llvm_v2i64_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
// end Chapter 6
// Chapter 7
def int_x86_dvpdpbuud : ClangBuiltin<"__builtin_ia32_dvpdpbuud">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
def int_x86_dvpdpbuuds : ClangBuiltin<"__builtin_ia32_dvpdpbuuds">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
def int_x86_dvpdpbssd : ClangBuiltin<"__builtin_ia32_dvpdpbssd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
def int_x86_dvpdpbssds : ClangBuiltin<"__builtin_ia32_dvpdpbssds">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
def int_x86_dvpdpbusd : ClangBuiltin<"__builtin_ia32_dvpdpbusd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
def int_x86_dvpdpbusds : ClangBuiltin<"__builtin_ia32_dvpdpbusds">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
def int_x86_dvpdpbsud : ClangBuiltin<"__builtin_ia32_dvpdpbsud">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
def int_x86_dvpdpbsuds : ClangBuiltin<"__builtin_ia32_dvpdpbsuds">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
def int_x86_dvpndpbssd : ClangBuiltin<"__builtin_ia32_dvpndpbssd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
// end Chapter 7
// Chapter 9
def int_x86_dvpsadaccubws : ClangBuiltin<"__builtin_ia32_dvpsadaccubws">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
// end Chapter 9
// Chapter 10
def int_x86_dvpsrrsuqw : ClangBuiltin<"__builtin_ia32_dvpsrrsuqw">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v2i64_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_dvpsrvrsuqw : ClangBuiltin<"__builtin_ia32_dvpsrvrsuqw">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                  [IntrNoMem]>;
def int_x86_dvpslrsuqw : ClangBuiltin<"__builtin_ia32_dvpslrsuqw">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v2i64_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_dvpslvrsuqw : ClangBuiltin<"__builtin_ia32_dvpslvrsuqw">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                  [IntrNoMem]>;
def int_x86_dvpsrarsqw : ClangBuiltin<"__builtin_ia32_dvpsrarsqw">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v2i64_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_dvpsravrsqw : ClangBuiltin<"__builtin_ia32_dvpsravrsqw">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                  [IntrNoMem]>;
def int_x86_dvpslrsqw : ClangBuiltin<"__builtin_ia32_dvpslrsqw">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v2i64_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_dvpslvrsqw : ClangBuiltin<"__builtin_ia32_dvpslvrsqw">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                  [IntrNoMem]>;
def int_x86_dvpsrrsuqd : ClangBuiltin<"__builtin_ia32_dvpsrrsuqd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v2i64_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_dvpsrvrsuqd : ClangBuiltin<"__builtin_ia32_dvpsrvrsuqd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                  [IntrNoMem]>;
def int_x86_dvpslrsuqd : ClangBuiltin<"__builtin_ia32_dvpslrsuqd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v2i64_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_dvpslvrsuqd : ClangBuiltin<"__builtin_ia32_dvpslvrsuqd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                  [IntrNoMem]>;
def int_x86_dvpsrarsqd : ClangBuiltin<"__builtin_ia32_dvpsrarsqd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v2i64_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_dvpsravrsqd : ClangBuiltin<"__builtin_ia32_dvpsravrsqd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                  [IntrNoMem]>;
def int_x86_dvpslrsqd : ClangBuiltin<"__builtin_ia32_dvpslrsqd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v2i64_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_dvpslvrsqd : ClangBuiltin<"__builtin_ia32_dvpslvrsqd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v2i64_ty, llvm_v2i64_ty],
                  [IntrNoMem]>;
// end Chapter 10
// Chapter 11
def int_x86_dvpsrard : ClangBuiltin<"__builtin_ia32_dvpsrard">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_dvpsravrd : ClangBuiltin<"__builtin_ia32_dvpsravrd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpslvsd : ClangBuiltin<"__builtin_ia32_dvpslvsd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpslsd : ClangBuiltin<"__builtin_ia32_dvpslsd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_dvpsrrud : ClangBuiltin<"__builtin_ia32_dvpsrrud">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_dvpsrvrud : ClangBuiltin<"__builtin_ia32_dvpsrvrud">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvpslsud : ClangBuiltin<"__builtin_ia32_dvpslsud">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_dvpslvsud : ClangBuiltin<"__builtin_ia32_dvpslvsud">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty],
                  [IntrNoMem]>;
// end Chapter 11
// Chapter 12
def int_x86_dvphaddlswuq : ClangBuiltin<"__builtin_ia32_dvphaddlswuq">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvphaddlswq : ClangBuiltin<"__builtin_ia32_dvphaddlswq">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty],
                  [IntrNoMem]>;
def int_x86_dvphaddlsduq : ClangBuiltin<"__builtin_ia32_dvphaddlsduq">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty],
                  [IntrNoMem]>;
def int_x86_dvphaddlsdq : ClangBuiltin<"__builtin_ia32_dvphaddlsdq">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty],
                  [IntrNoMem]>;
// end Chapter 12
// Chapter 13
def int_x86_dvpcaddrotsraw : ClangBuiltin<"__builtin_ia32_dvpcaddrotsraw">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_dvpcaddrotsrad : ClangBuiltin<"__builtin_ia32_dvpcaddrotsrad">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
// end Chapter 13
// Chapter 8
def int_x86_dvpdpint4uud : ClangBuiltin<"__builtin_ia32_dvpdpint4uud">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
def int_x86_dvpdpint4ssd : ClangBuiltin<"__builtin_ia32_dvpdpint4ssd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
def int_x86_dvpdpint4usd : ClangBuiltin<"__builtin_ia32_dvpdpint4usd">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
def int_x86_dvpdpint4sud : ClangBuiltin<"__builtin_ia32_dvpdpint4sud">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrNoMem]>;
// end Chapter 8
// Chapter 18
def int_x86_dvpunpckdq : ClangBuiltin<"__builtin_ia32_dvpunpckdq">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_v4i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
// end Chapter 18
// Chapter 19
def int_x86_dvpmasklddqu : ClangBuiltin<"__builtin_ia32_dvpmasklddqu">,
        Intrinsic<[llvm_v16i8_ty], [llvm_v16i8_ty, llvm_ptr_ty],
                  [IntrArgMemOnly]>;
def int_x86_dvpmaskstdqu : ClangBuiltin<"__builtin_ia32_dvpmaskstdqu">,
        Intrinsic<[], [llvm_ptr_ty, llvm_v16i8_ty, llvm_v16i8_ty],
                  [IntrArgMemOnly]>;
// end Chapter 19
}
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_DSPV1

#if INTEL_FEATURE_ISA_CMPCCXADD
let TargetPrefix = "x86" in {
def int_x86_cmpccxadd32
    : ClangBuiltin<"__builtin_ia32_cmpccxadd32">,
      Intrinsic<[llvm_i32_ty],
                [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty],
                [IntrArgMemOnly, ImmArg<ArgIndex<3>>]>;

def int_x86_cmpccxadd64
    : ClangBuiltin<"__builtin_ia32_cmpccxadd64">,
      Intrinsic<[llvm_i64_ty],
                [llvm_ptr_ty, llvm_i64_ty, llvm_i64_ty, llvm_i32_ty],
                [IntrArgMemOnly, ImmArg<ArgIndex<3>>]>;
}
#endif // INTEL_FEATURE_ISA_CMPCCXADD
#if INTEL_FEATURE_ISA_AVX512_SAT_CVT
let TargetPrefix = "x86" in {
def int_x86_vcvtnebf162ibs128 : ClangBuiltin<"__builtin_ia32_vcvtnebf162ibs128">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
 def int_x86_mask_vcvtnebf162ibs128 : ClangBuiltin<"__builtin_ia32_vcvtnebf162ibs128_mask">,
         Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<3>>]>;
 def int_x86_maskz_vcvtnebf162ibs128 : ClangBuiltin<"__builtin_ia32_vcvtnebf162ibs128_maskz">,
         Intrinsic<[llvm_v8i16_ty], [llvm_i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtnebf162ibs256 : ClangBuiltin<"__builtin_ia32_vcvtnebf162ibs256">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvtnebf162ibs256 : ClangBuiltin<"__builtin_ia32_vcvtnebf162ibs256_mask">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i16_ty, llvm_v16i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvtnebf162ibs256 : ClangBuiltin<"__builtin_ia32_vcvtnebf162ibs256_maskz">,
        Intrinsic<[llvm_v16i16_ty], [llvm_i16_ty, llvm_v16i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtnebf162ibs512 : ClangBuiltin<"__builtin_ia32_vcvtnebf162ibs512">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvtnebf162ibs512 : ClangBuiltin<"__builtin_ia32_vcvtnebf162ibs512_mask">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty, llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvtnebf162ibs512 : ClangBuiltin<"__builtin_ia32_vcvtnebf162ibs512_maskz">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtnebf162iubs128 : ClangBuiltin<"__builtin_ia32_vcvtnebf162iubs128">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
 def int_x86_mask_vcvtnebf162iubs128 : ClangBuiltin<"__builtin_ia32_vcvtnebf162iubs128_mask">,
         Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<3>>]>;
 def int_x86_maskz_vcvtnebf162iubs128 : ClangBuiltin<"__builtin_ia32_vcvtnebf162iubs128_maskz">,
         Intrinsic<[llvm_v8i16_ty], [llvm_i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtnebf162iubs256 : ClangBuiltin<"__builtin_ia32_vcvtnebf162iubs256">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvtnebf162iubs256 : ClangBuiltin<"__builtin_ia32_vcvtnebf162iubs256_mask">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i16_ty, llvm_v16i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvtnebf162iubs256 : ClangBuiltin<"__builtin_ia32_vcvtnebf162iubs256_maskz">,
        Intrinsic<[llvm_v16i16_ty], [llvm_i16_ty, llvm_v16i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtnebf162iubs512 : ClangBuiltin<"__builtin_ia32_vcvtnebf162iubs512">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvtnebf162iubs512 : ClangBuiltin<"__builtin_ia32_vcvtnebf162iubs512_mask">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty, llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvtnebf162iubs512 : ClangBuiltin<"__builtin_ia32_vcvtnebf162iubs512_maskz">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtph2ibs128 : ClangBuiltin<"__builtin_ia32_vcvtph2ibs128">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
 def int_x86_mask_vcvtph2ibs128 : ClangBuiltin<"__builtin_ia32_vcvtph2ibs128_mask">,
         Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_i8_ty, llvm_v8f16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<3>>]>;
 def int_x86_maskz_vcvtph2ibs128 : ClangBuiltin<"__builtin_ia32_vcvtph2ibs128_maskz">,
         Intrinsic<[llvm_v8i16_ty], [llvm_i8_ty, llvm_v8f16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtph2ibs256 : ClangBuiltin<"__builtin_ia32_vcvtph2ibs256">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvtph2ibs256 : ClangBuiltin<"__builtin_ia32_vcvtph2ibs256_mask">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i16_ty, llvm_v16f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvtph2ibs256 : ClangBuiltin<"__builtin_ia32_vcvtph2ibs256_maskz">,
        Intrinsic<[llvm_v16i16_ty], [llvm_i16_ty, llvm_v16f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtph2ibs512 : ClangBuiltin<"__builtin_ia32_vcvtph2ibs512">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvtph2ibs512 : ClangBuiltin<"__builtin_ia32_vcvtph2ibs512_mask">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvtph2ibs512 : ClangBuiltin<"__builtin_ia32_vcvtph2ibs512_maskz">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtph2ibs_round_512 : ClangBuiltin<"__builtin_ia32_vcvtph2ibs512_round">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_mask_vcvtph2ibs_round_512 : ClangBuiltin<"__builtin_ia32_vcvtph2ibs512_round_mask">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
def int_x86_maskz_vcvtph2ibs_round_512 : ClangBuiltin<"__builtin_ia32_vcvtph2ibs512_round_maskz">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
def int_x86_vcvtph2iubs128 : ClangBuiltin<"__builtin_ia32_vcvtph2iubs128">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
 def int_x86_mask_vcvtph2iubs128 : ClangBuiltin<"__builtin_ia32_vcvtph2iubs128_mask">,
         Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_i8_ty, llvm_v8f16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<3>>]>;
 def int_x86_maskz_vcvtph2iubs128 : ClangBuiltin<"__builtin_ia32_vcvtph2iubs128_maskz">,
         Intrinsic<[llvm_v8i16_ty], [llvm_i8_ty, llvm_v8f16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtph2iubs256 : ClangBuiltin<"__builtin_ia32_vcvtph2iubs256">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvtph2iubs256 : ClangBuiltin<"__builtin_ia32_vcvtph2iubs256_mask">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i16_ty, llvm_v16f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvtph2iubs256 : ClangBuiltin<"__builtin_ia32_vcvtph2iubs256_maskz">,
        Intrinsic<[llvm_v16i16_ty], [llvm_i16_ty, llvm_v16f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtph2iubs512 : ClangBuiltin<"__builtin_ia32_vcvtph2iubs512">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvtph2iubs512 : ClangBuiltin<"__builtin_ia32_vcvtph2iubs512_mask">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvtph2iubs512 : ClangBuiltin<"__builtin_ia32_vcvtph2iubs512_maskz">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtph2iubs_round_512 : ClangBuiltin<"__builtin_ia32_vcvtph2iubs512_round">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_mask_vcvtph2iubs_round_512 : ClangBuiltin<"__builtin_ia32_vcvtph2iubs512_round_mask">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
def int_x86_maskz_vcvtph2iubs_round_512 : ClangBuiltin<"__builtin_ia32_vcvtph2iubs512_round_maskz">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
def int_x86_vcvtps2ibs128 : ClangBuiltin<"__builtin_ia32_vcvtps2ibs128">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
 def int_x86_mask_vcvtps2ibs128 : ClangBuiltin<"__builtin_ia32_vcvtps2ibs128_mask">,
         Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_i8_ty, llvm_v4f32_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<3>>]>;
 def int_x86_maskz_vcvtps2ibs128 : ClangBuiltin<"__builtin_ia32_vcvtps2ibs128_maskz">,
         Intrinsic<[llvm_v4i32_ty], [llvm_i8_ty, llvm_v4f32_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtps2ibs256 : ClangBuiltin<"__builtin_ia32_vcvtps2ibs256">,
        Intrinsic<[llvm_v8i32_ty], [llvm_v8f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvtps2ibs256 : ClangBuiltin<"__builtin_ia32_vcvtps2ibs256_mask">,
        Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i8_ty, llvm_v8f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvtps2ibs256 : ClangBuiltin<"__builtin_ia32_vcvtps2ibs256_maskz">,
        Intrinsic<[llvm_v8i32_ty], [llvm_i8_ty, llvm_v8f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtps2ibs512 : ClangBuiltin<"__builtin_ia32_vcvtps2ibs512">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvtps2ibs512 : ClangBuiltin<"__builtin_ia32_vcvtps2ibs512_mask">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvtps2ibs512 : ClangBuiltin<"__builtin_ia32_vcvtps2ibs512_maskz">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtps2ibs_round_512 : ClangBuiltin<"__builtin_ia32_vcvtps2ibs512_round">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16f32_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_mask_vcvtps2ibs_round_512 : ClangBuiltin<"__builtin_ia32_vcvtps2ibs512_round_mask">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
def int_x86_maskz_vcvtps2ibs_round_512 : ClangBuiltin<"__builtin_ia32_vcvtps2ibs512_round_maskz">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
def int_x86_vcvtps2iubs128 : ClangBuiltin<"__builtin_ia32_vcvtps2iubs128">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
 def int_x86_mask_vcvtps2iubs128 : ClangBuiltin<"__builtin_ia32_vcvtps2iubs128_mask">,
         Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_i8_ty, llvm_v4f32_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<3>>]>;
 def int_x86_maskz_vcvtps2iubs128 : ClangBuiltin<"__builtin_ia32_vcvtps2iubs128_maskz">,
         Intrinsic<[llvm_v4i32_ty], [llvm_i8_ty, llvm_v4f32_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtps2iubs256 : ClangBuiltin<"__builtin_ia32_vcvtps2iubs256">,
        Intrinsic<[llvm_v8i32_ty], [llvm_v8f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvtps2iubs256 : ClangBuiltin<"__builtin_ia32_vcvtps2iubs256_mask">,
        Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i8_ty, llvm_v8f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvtps2iubs256 : ClangBuiltin<"__builtin_ia32_vcvtps2iubs256_maskz">,
        Intrinsic<[llvm_v8i32_ty], [llvm_i8_ty, llvm_v8f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtps2iubs512 : ClangBuiltin<"__builtin_ia32_vcvtps2iubs512">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvtps2iubs512 : ClangBuiltin<"__builtin_ia32_vcvtps2iubs512_mask">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvtps2iubs512 : ClangBuiltin<"__builtin_ia32_vcvtps2iubs512_maskz">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvtps2iubs_round_512 : ClangBuiltin<"__builtin_ia32_vcvtps2iubs512_round">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16f32_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_mask_vcvtps2iubs_round_512 : ClangBuiltin<"__builtin_ia32_vcvtps2iubs512_round_mask">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
def int_x86_maskz_vcvtps2iubs_round_512 : ClangBuiltin<"__builtin_ia32_vcvtps2iubs512_round_maskz">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
def int_x86_vcvttnebf162ibs128 : ClangBuiltin<"__builtin_ia32_vcvttnebf162ibs128">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
 def int_x86_mask_vcvttnebf162ibs128 : ClangBuiltin<"__builtin_ia32_vcvttnebf162ibs128_mask">,
         Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<3>>]>;
 def int_x86_maskz_vcvttnebf162ibs128 : ClangBuiltin<"__builtin_ia32_vcvttnebf162ibs128_maskz">,
         Intrinsic<[llvm_v8i16_ty], [llvm_i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttnebf162ibs256 : ClangBuiltin<"__builtin_ia32_vcvttnebf162ibs256">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvttnebf162ibs256 : ClangBuiltin<"__builtin_ia32_vcvttnebf162ibs256_mask">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i16_ty, llvm_v16i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvttnebf162ibs256 : ClangBuiltin<"__builtin_ia32_vcvttnebf162ibs256_maskz">,
        Intrinsic<[llvm_v16i16_ty], [llvm_i16_ty, llvm_v16i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttnebf162ibs512 : ClangBuiltin<"__builtin_ia32_vcvttnebf162ibs512">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvttnebf162ibs512 : ClangBuiltin<"__builtin_ia32_vcvttnebf162ibs512_mask">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty, llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvttnebf162ibs512 : ClangBuiltin<"__builtin_ia32_vcvttnebf162ibs512_maskz">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttnebf162iubs128 : ClangBuiltin<"__builtin_ia32_vcvttnebf162iubs128">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
 def int_x86_mask_vcvttnebf162iubs128 : ClangBuiltin<"__builtin_ia32_vcvttnebf162iubs128_mask">,
         Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<3>>]>;
 def int_x86_maskz_vcvttnebf162iubs128 : ClangBuiltin<"__builtin_ia32_vcvttnebf162iubs128_maskz">,
         Intrinsic<[llvm_v8i16_ty], [llvm_i8_ty, llvm_v8i16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttnebf162iubs256 : ClangBuiltin<"__builtin_ia32_vcvttnebf162iubs256">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvttnebf162iubs256 : ClangBuiltin<"__builtin_ia32_vcvttnebf162iubs256_mask">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i16_ty, llvm_v16i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvttnebf162iubs256 : ClangBuiltin<"__builtin_ia32_vcvttnebf162iubs256_maskz">,
        Intrinsic<[llvm_v16i16_ty], [llvm_i16_ty, llvm_v16i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttnebf162iubs512 : ClangBuiltin<"__builtin_ia32_vcvttnebf162iubs512">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvttnebf162iubs512 : ClangBuiltin<"__builtin_ia32_vcvttnebf162iubs512_mask">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty, llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvttnebf162iubs512 : ClangBuiltin<"__builtin_ia32_vcvttnebf162iubs512_maskz">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttph2ibs128 : ClangBuiltin<"__builtin_ia32_vcvttph2ibs128">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
 def int_x86_mask_vcvttph2ibs128 : ClangBuiltin<"__builtin_ia32_vcvttph2ibs128_mask">,
         Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_i8_ty, llvm_v8f16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<3>>]>;
 def int_x86_maskz_vcvttph2ibs128 : ClangBuiltin<"__builtin_ia32_vcvttph2ibs128_maskz">,
         Intrinsic<[llvm_v8i16_ty], [llvm_i8_ty, llvm_v8f16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttph2ibs256 : ClangBuiltin<"__builtin_ia32_vcvttph2ibs256">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvttph2ibs256 : ClangBuiltin<"__builtin_ia32_vcvttph2ibs256_mask">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i16_ty, llvm_v16f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvttph2ibs256 : ClangBuiltin<"__builtin_ia32_vcvttph2ibs256_maskz">,
        Intrinsic<[llvm_v16i16_ty], [llvm_i16_ty, llvm_v16f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttph2ibs512 : ClangBuiltin<"__builtin_ia32_vcvttph2ibs512">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvttph2ibs512 : ClangBuiltin<"__builtin_ia32_vcvttph2ibs512_mask">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvttph2ibs512 : ClangBuiltin<"__builtin_ia32_vcvttph2ibs512_maskz">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttph2ibs_round_512 : ClangBuiltin<"__builtin_ia32_vcvttph2ibs512_round">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_mask_vcvttph2ibs_round_512 : ClangBuiltin<"__builtin_ia32_vcvttph2ibs512_round_mask">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
def int_x86_maskz_vcvttph2ibs_round_512 : ClangBuiltin<"__builtin_ia32_vcvttph2ibs512_round_maskz">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
def int_x86_vcvttph2iubs128 : ClangBuiltin<"__builtin_ia32_vcvttph2iubs128">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
 def int_x86_mask_vcvttph2iubs128 : ClangBuiltin<"__builtin_ia32_vcvttph2iubs128_mask">,
         Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_i8_ty, llvm_v8f16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<3>>]>;
 def int_x86_maskz_vcvttph2iubs128 : ClangBuiltin<"__builtin_ia32_vcvttph2iubs128_maskz">,
         Intrinsic<[llvm_v8i16_ty], [llvm_i8_ty, llvm_v8f16_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttph2iubs256 : ClangBuiltin<"__builtin_ia32_vcvttph2iubs256">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvttph2iubs256 : ClangBuiltin<"__builtin_ia32_vcvttph2iubs256_mask">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_i16_ty, llvm_v16f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvttph2iubs256 : ClangBuiltin<"__builtin_ia32_vcvttph2iubs256_maskz">,
        Intrinsic<[llvm_v16i16_ty], [llvm_i16_ty, llvm_v16f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttph2iubs512 : ClangBuiltin<"__builtin_ia32_vcvttph2iubs512">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvttph2iubs512 : ClangBuiltin<"__builtin_ia32_vcvttph2iubs512_mask">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvttph2iubs512 : ClangBuiltin<"__builtin_ia32_vcvttph2iubs512_maskz">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttph2iubs_round_512 : ClangBuiltin<"__builtin_ia32_vcvttph2iubs512_round">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_mask_vcvttph2iubs_round_512 : ClangBuiltin<"__builtin_ia32_vcvttph2iubs512_round_mask">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
def int_x86_maskz_vcvttph2iubs_round_512 : ClangBuiltin<"__builtin_ia32_vcvttph2iubs512_round_maskz">,
        Intrinsic<[llvm_v32i16_ty], [llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
def int_x86_vcvttps2ibs128 : ClangBuiltin<"__builtin_ia32_vcvttps2ibs128">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
 def int_x86_mask_vcvttps2ibs128 : ClangBuiltin<"__builtin_ia32_vcvttps2ibs128_mask">,
         Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_i8_ty, llvm_v4f32_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<3>>]>;
 def int_x86_maskz_vcvttps2ibs128 : ClangBuiltin<"__builtin_ia32_vcvttps2ibs128_maskz">,
         Intrinsic<[llvm_v4i32_ty], [llvm_i8_ty, llvm_v4f32_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttps2ibs256 : ClangBuiltin<"__builtin_ia32_vcvttps2ibs256">,
        Intrinsic<[llvm_v8i32_ty], [llvm_v8f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvttps2ibs256 : ClangBuiltin<"__builtin_ia32_vcvttps2ibs256_mask">,
        Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i8_ty, llvm_v8f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvttps2ibs256 : ClangBuiltin<"__builtin_ia32_vcvttps2ibs256_maskz">,
        Intrinsic<[llvm_v8i32_ty], [llvm_i8_ty, llvm_v8f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttps2ibs512 : ClangBuiltin<"__builtin_ia32_vcvttps2ibs512">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvttps2ibs512 : ClangBuiltin<"__builtin_ia32_vcvttps2ibs512_mask">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvttps2ibs512 : ClangBuiltin<"__builtin_ia32_vcvttps2ibs512_maskz">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttps2ibs_round_512 : ClangBuiltin<"__builtin_ia32_vcvttps2ibs512_round">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16f32_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_mask_vcvttps2ibs_round_512 : ClangBuiltin<"__builtin_ia32_vcvttps2ibs512_round_mask">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
def int_x86_maskz_vcvttps2ibs_round_512 : ClangBuiltin<"__builtin_ia32_vcvttps2ibs512_round_maskz">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
def int_x86_vcvttps2iubs128 : ClangBuiltin<"__builtin_ia32_vcvttps2iubs128">,
        Intrinsic<[llvm_v4i32_ty], [llvm_v4f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
 def int_x86_mask_vcvttps2iubs128 : ClangBuiltin<"__builtin_ia32_vcvttps2iubs128_mask">,
         Intrinsic<[llvm_v4i32_ty], [llvm_v4i32_ty, llvm_i8_ty, llvm_v4f32_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<3>>]>;
 def int_x86_maskz_vcvttps2iubs128 : ClangBuiltin<"__builtin_ia32_vcvttps2iubs128_maskz">,
         Intrinsic<[llvm_v4i32_ty], [llvm_i8_ty, llvm_v4f32_ty, llvm_i32_ty],
                   [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttps2iubs256 : ClangBuiltin<"__builtin_ia32_vcvttps2iubs256">,
        Intrinsic<[llvm_v8i32_ty], [llvm_v8f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvttps2iubs256 : ClangBuiltin<"__builtin_ia32_vcvttps2iubs256_mask">,
        Intrinsic<[llvm_v8i32_ty], [llvm_v8i32_ty, llvm_i8_ty, llvm_v8f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvttps2iubs256 : ClangBuiltin<"__builtin_ia32_vcvttps2iubs256_maskz">,
        Intrinsic<[llvm_v8i32_ty], [llvm_i8_ty, llvm_v8f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttps2iubs512 : ClangBuiltin<"__builtin_ia32_vcvttps2iubs512">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>]>;
def int_x86_mask_vcvttps2iubs512 : ClangBuiltin<"__builtin_ia32_vcvttps2iubs512_mask">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>]>;
def int_x86_maskz_vcvttps2iubs512 : ClangBuiltin<"__builtin_ia32_vcvttps2iubs512_maskz">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_vcvttps2iubs_round_512 : ClangBuiltin<"__builtin_ia32_vcvttps2iubs512_round">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16f32_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<2>>]>;
def int_x86_mask_vcvttps2iubs_round_512 : ClangBuiltin<"__builtin_ia32_vcvttps2iubs512_round_mask">,
        Intrinsic<[llvm_v16i32_ty], [llvm_v16i32_ty, llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<3>>, ImmArg<ArgIndex<4>>]>;
def int_x86_maskz_vcvttps2iubs_round_512 : ClangBuiltin<"__builtin_ia32_vcvttps2iubs512_round_maskz">,
        Intrinsic<[llvm_v16i32_ty], [llvm_i16_ty, llvm_v16f32_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>>]>;
}
#endif // INTEL_FEATURE_ISA_AVX512_SAT_CVT
#if INTEL_FEATURE_ISA_AVX512_MINMAX
// AUTO GENERATED BY TOOL
let TargetPrefix = "x86" in {
def int_x86_avx512minmax_vminmaxnepbf16128 : ClangBuiltin<"__builtin_ia32_vminmaxnepbf16128">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_mask_vminmaxnepbf16128 : ClangBuiltin<"__builtin_ia32_vminmaxnepbf16128_mask">,
        Intrinsic<[llvm_v8i16_ty], [llvm_v8i16_ty, llvm_v8i16_ty, llvm_i32_ty, llvm_v8i16_ty, llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_vminmaxnepbf16256 : ClangBuiltin<"__builtin_ia32_vminmaxnepbf16256">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_mask_vminmaxnepbf16256 : ClangBuiltin<"__builtin_ia32_vminmaxnepbf16256_mask">,
        Intrinsic<[llvm_v16i16_ty], [llvm_v16i16_ty, llvm_v16i16_ty, llvm_i32_ty, llvm_v16i16_ty, llvm_i16_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_vminmaxnepbf16512 : ClangBuiltin<"__builtin_ia32_vminmaxnepbf16512">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_mask_vminmaxnepbf16512 : ClangBuiltin<"__builtin_ia32_vminmaxnepbf16512_mask">,
        Intrinsic<[llvm_v32i16_ty], [llvm_v32i16_ty, llvm_v32i16_ty, llvm_i32_ty, llvm_v32i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_vminmaxpd128 : ClangBuiltin<"__builtin_ia32_vminmaxpd128">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_mask_vminmaxpd128 : ClangBuiltin<"__builtin_ia32_vminmaxpd128_mask">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty, llvm_i32_ty, llvm_v2f64_ty, llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_vminmaxpd256 : ClangBuiltin<"__builtin_ia32_vminmaxpd256">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_mask_vminmaxpd256 : ClangBuiltin<"__builtin_ia32_vminmaxpd256_mask">,
        Intrinsic<[llvm_v4f64_ty], [llvm_v4f64_ty, llvm_v4f64_ty, llvm_i32_ty, llvm_v4f64_ty, llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_mask_vminmaxpd_round : ClangBuiltin<"__builtin_ia32_vminmaxpd_round_mask">,
        Intrinsic<[llvm_v8f64_ty], [llvm_v8f64_ty, llvm_v8f64_ty, llvm_i32_ty, llvm_v8f64_ty, llvm_i8_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<5>>]>;
def int_x86_avx512minmax_vminmaxph128 : ClangBuiltin<"__builtin_ia32_vminmaxph128">,
        Intrinsic<[llvm_v8f16_ty], [llvm_v8f16_ty, llvm_v8f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_mask_vminmaxph128 : ClangBuiltin<"__builtin_ia32_vminmaxph128_mask">,
        Intrinsic<[llvm_v8f16_ty], [llvm_v8f16_ty, llvm_v8f16_ty, llvm_i32_ty, llvm_v8f16_ty, llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_vminmaxph256 : ClangBuiltin<"__builtin_ia32_vminmaxph256">,
        Intrinsic<[llvm_v16f16_ty], [llvm_v16f16_ty, llvm_v16f16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_mask_vminmaxph256 : ClangBuiltin<"__builtin_ia32_vminmaxph256_mask">,
        Intrinsic<[llvm_v16f16_ty], [llvm_v16f16_ty, llvm_v16f16_ty, llvm_i32_ty, llvm_v16f16_ty, llvm_i16_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_mask_vminmaxph_round : ClangBuiltin<"__builtin_ia32_vminmaxph_round_mask">,
        Intrinsic<[llvm_v32f16_ty], [llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<5>>]>;
def int_x86_avx512minmax_vminmaxps128 : ClangBuiltin<"__builtin_ia32_vminmaxps128">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_mask_vminmaxps128 : ClangBuiltin<"__builtin_ia32_vminmaxps128_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_i32_ty, llvm_v4f32_ty, llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_vminmaxps256 : ClangBuiltin<"__builtin_ia32_vminmaxps256">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_v8f32_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_mask_vminmaxps256 : ClangBuiltin<"__builtin_ia32_vminmaxps256_mask">,
        Intrinsic<[llvm_v8f32_ty], [llvm_v8f32_ty, llvm_v8f32_ty, llvm_i32_ty, llvm_v8f32_ty, llvm_i8_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>]>;
def int_x86_avx512minmax_mask_vminmaxps_round : ClangBuiltin<"__builtin_ia32_vminmaxps_round_mask">,
        Intrinsic<[llvm_v16f32_ty], [llvm_v16f32_ty, llvm_v16f32_ty, llvm_i32_ty, llvm_v16f32_ty, llvm_i16_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<5>>]>;
def int_x86_avx512minmax_mask_vminmaxsd_round : ClangBuiltin<"__builtin_ia32_vminmaxsd_round_mask">,
        Intrinsic<[llvm_v2f64_ty], [llvm_v2f64_ty, llvm_v2f64_ty, llvm_i32_ty, llvm_v2f64_ty, llvm_i8_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<5>>]>;
def int_x86_avx512minmax_mask_vminmaxsh_round : ClangBuiltin<"__builtin_ia32_vminmaxsh_round_mask">,
        Intrinsic<[llvm_v8f16_ty], [llvm_v8f16_ty, llvm_v8f16_ty, llvm_i32_ty, llvm_v8f16_ty, llvm_i8_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<5>>]>;
def int_x86_avx512minmax_mask_vminmaxss_round : ClangBuiltin<"__builtin_ia32_vminmaxss_round_mask">,
        Intrinsic<[llvm_v4f32_ty], [llvm_v4f32_ty, llvm_v4f32_ty, llvm_i32_ty, llvm_v4f32_ty, llvm_i8_ty, llvm_i32_ty],
                  [IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<5>>]>;
}
// end AUTO GENERATED BY TOOL
#endif // INTEL_FEATURE_ISA_AVX512_MINMAX
#endif // INTEL_CUSTOMIZATION
//===----------------------------------------------------------------------===//
// UINTR - User Level Interrupt

let TargetPrefix = "x86" in {
  def int_x86_clui : ClangBuiltin<"__builtin_ia32_clui">,
              Intrinsic<[], [], []>;
  def int_x86_stui : ClangBuiltin<"__builtin_ia32_stui">,
              Intrinsic<[], [], []>;
  def int_x86_testui : ClangBuiltin<"__builtin_ia32_testui">,
              Intrinsic<[llvm_i8_ty], [], []>;
  def int_x86_senduipi : ClangBuiltin<"__builtin_ia32_senduipi">,
              Intrinsic<[], [llvm_i64_ty], []>;
}

//===----------------------------------------------------------------------===//
let TargetPrefix = "x86" in {
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX512_BF16_NE
  def int_x86_avx512bf16ne_vaddnepbf16512
      : ClangBuiltin<"__builtin_ia32_vaddnepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vaddnepbf16256
      : ClangBuiltin<"__builtin_ia32_vaddnepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vaddnepbf16128
      : ClangBuiltin<"__builtin_ia32_vaddnepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vsubnepbf16512
      : ClangBuiltin<"__builtin_ia32_vsubnepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vsubnepbf16256
      : ClangBuiltin<"__builtin_ia32_vsubnepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vsubnepbf16128
      : ClangBuiltin<"__builtin_ia32_vsubnepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vmulnepbf16512
      : ClangBuiltin<"__builtin_ia32_vmulnepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vmulnepbf16256
      : ClangBuiltin<"__builtin_ia32_vmulnepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vmulnepbf16128
      : ClangBuiltin<"__builtin_ia32_vmulnepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vdivnepbf16512
      : ClangBuiltin<"__builtin_ia32_vdivnepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vdivnepbf16256
      : ClangBuiltin<"__builtin_ia32_vdivnepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vdivnepbf16128
      : ClangBuiltin<"__builtin_ia32_vdivnepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vmaxnepbf16512
      : ClangBuiltin<"__builtin_ia32_vmaxnepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vmaxnepbf16256
      : ClangBuiltin<"__builtin_ia32_vmaxnepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vmaxnepbf16128
      : ClangBuiltin<"__builtin_ia32_vmaxnepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vminnepbf16512
      : ClangBuiltin<"__builtin_ia32_vminnepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vminnepbf16256
      : ClangBuiltin<"__builtin_ia32_vminnepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vminnepbf16128
      : ClangBuiltin<"__builtin_ia32_vminnepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vcomnesbf16eq
      : ClangBuiltin<"__builtin_ia32_vcomnesbf16eq">,
        Intrinsic<[llvm_i32_ty],
                  [llvm_v8bf16_ty, llvm_v8bf16_ty],
                  [IntrNoMem]>;
  def int_x86_avx512bf16ne_vcomnesbf16lt
      : ClangBuiltin<"__builtin_ia32_vcomnesbf16lt">,
        Intrinsic<[llvm_i32_ty],
                  [llvm_v8bf16_ty,llvm_v8bf16_ty],
                  [IntrNoMem]>;
  def int_x86_avx512bf16ne_vcomnesbf16le
      : ClangBuiltin<"__builtin_ia32_vcomnesbf16le">,
        Intrinsic<[llvm_i32_ty],
                  [llvm_v8bf16_ty, llvm_v8bf16_ty],
                  [IntrNoMem]>;
  def int_x86_avx512bf16ne_vcomnesbf16gt
      : ClangBuiltin<"__builtin_ia32_vcomnesbf16gt">,
        Intrinsic<[llvm_i32_ty],
                  [llvm_v8bf16_ty, llvm_v8bf16_ty],
                  [IntrNoMem]>;
  def int_x86_avx512bf16ne_vcomnesbf16ge
      : ClangBuiltin<"__builtin_ia32_vcomnesbf16ge">,
        Intrinsic<[llvm_i32_ty],
                  [llvm_v8bf16_ty, llvm_v8bf16_ty],
                  [IntrNoMem]>;
  def int_x86_avx512bf16ne_vcomnesbf16neq
      : ClangBuiltin<"__builtin_ia32_vcomnesbf16neq">,
        Intrinsic<[llvm_i32_ty],
                  [llvm_v8bf16_ty, llvm_v8bf16_ty],
                  [IntrNoMem]>;
  def int_x86_avx512bf16ne_vcmpnepbf16512_mask
      : ClangBuiltin<"__builtin_ia32_vcmpnepbf16512_mask">,
        Intrinsic<[ llvm_i32_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_i32_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;
  def int_x86_avx512bf16ne_vcmpnepbf16256_mask
      : ClangBuiltin<"__builtin_ia32_vcmpnepbf16256_mask">,
        Intrinsic<[ llvm_i16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_i32_ty, llvm_i16_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;
  def int_x86_avx512bf16ne_vcmpnepbf16128_mask
      : ClangBuiltin<"__builtin_ia32_vcmpnepbf16128_mask">,
        Intrinsic<[ llvm_i8_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_i32_ty, llvm_i8_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;
  def int_x86_avx512bf16ne_mask_sqrt_nepbf16_128
      : ClangBuiltin<"__builtin_ia32_vsqrtnepbf16128_mask">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_sqrt_nepbf16_256
      : ClangBuiltin<"__builtin_ia32_vsqrtnepbf16256_mask">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_i16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_sqrt_nepbf16_512
      : ClangBuiltin<"__builtin_ia32_vsqrtnepbf16512_mask">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_i32_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_rsqrt_nepbf16_128
      : ClangBuiltin<"__builtin_ia32_vrsqrtnepbf16128_mask">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_rsqrt_nepbf16_256
      : ClangBuiltin<"__builtin_ia32_vrsqrtnepbf16256_mask">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_i16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_rsqrt_nepbf16_512
      : ClangBuiltin<"__builtin_ia32_vrsqrtnepbf16512_mask">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_i32_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_rcp_nepbf16_128
      : ClangBuiltin<"__builtin_ia32_vrcpnepbf16128_mask">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_rcp_nepbf16_256
      : ClangBuiltin<"__builtin_ia32_vrcpnepbf16256_mask">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_i16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_rcp_nepbf16_512
      : ClangBuiltin<"__builtin_ia32_vrcpnepbf16512_mask">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_i32_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_reduce_nepbf16_128
      : ClangBuiltin<"__builtin_ia32_vreducenepbf16128_mask">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_i32_ty, llvm_v8bf16_ty, llvm_i8_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512bf16ne_mask_reduce_nepbf16_256
      : ClangBuiltin<"__builtin_ia32_vreducenepbf16256_mask">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_i32_ty, llvm_v16bf16_ty, llvm_i16_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512bf16ne_mask_reduce_nepbf16_512
      : ClangBuiltin<"__builtin_ia32_vreducenepbf16512_mask">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_i32_ty, llvm_v32bf16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512bf16ne_fpclass_nepbf16_128
      : Intrinsic<[ llvm_v8i1_ty ], [ llvm_v8bf16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512bf16ne_fpclass_nepbf16_256
      : Intrinsic<[ llvm_v16i1_ty ], [ llvm_v16bf16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512bf16ne_fpclass_nepbf16_512
      : Intrinsic<[ llvm_v32i1_ty ], [ llvm_v32bf16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512bf16ne_mask_getexp_nepbf16_128
      : ClangBuiltin<"__builtin_ia32_vgetexpnepbf16128_mask">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_getexp_nepbf16_256
      : ClangBuiltin<"__builtin_ia32_vgetexpnepbf16256_mask">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_i16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_getexp_nepbf16_512
      : ClangBuiltin<"__builtin_ia32_vgetexpnepbf16512_mask">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_i32_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_getmant_nepbf16_128
      : ClangBuiltin<"__builtin_ia32_vgetmantnepbf16128_mask">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_i32_ty, llvm_v8bf16_ty, llvm_i8_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512bf16ne_mask_getmant_nepbf16_256
      : ClangBuiltin<"__builtin_ia32_vgetmantnepbf16256_mask">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_i32_ty, llvm_v16bf16_ty, llvm_i16_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512bf16ne_mask_getmant_nepbf16_512
      : ClangBuiltin<"__builtin_ia32_vgetmantnepbf16512_mask">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_i32_ty, llvm_v32bf16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512bf16ne_mask_rndscale_nepbf16_128
      : ClangBuiltin<"__builtin_ia32_vrndscalenepbf16_128_mask">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_i32_ty, llvm_v8bf16_ty, llvm_i8_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512bf16ne_mask_rndscale_nepbf16_256
      : ClangBuiltin<"__builtin_ia32_vrndscalenepbf16_256_mask">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_i32_ty, llvm_v16bf16_ty, llvm_i16_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512bf16ne_mask_rndscale_nepbf16_512
      : ClangBuiltin<"__builtin_ia32_vrndscalenepbf16_mask">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_i32_ty, llvm_v32bf16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512bf16ne_mask_scalef_nepbf16_128
      : ClangBuiltin<"__builtin_ia32_vscalefnepbf16128_mask">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_scalef_nepbf16_256
      : ClangBuiltin<"__builtin_ia32_vscalefnepbf16256_mask">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_i16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_mask_scalef_nepbf16_512
      : ClangBuiltin<"__builtin_ia32_vscalefnepbf16512_mask">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_i32_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmadd213nepbf16512
      : ClangBuiltin<"__builtin_ia32_vfmadd213nepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmadd213nepbf16256
      : ClangBuiltin<"__builtin_ia32_vfmadd213nepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmadd213nepbf16128
      : ClangBuiltin<"__builtin_ia32_vfmadd213nepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_v8bf16_ty  ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmadd132nepbf16512
      : ClangBuiltin<"__builtin_ia32_vfmadd132nepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmadd132nepbf16256
      : ClangBuiltin<"__builtin_ia32_vfmadd132nepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmadd132nepbf16128
      : ClangBuiltin<"__builtin_ia32_vfmadd132nepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_v8bf16_ty  ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmadd231nepbf16512
      : ClangBuiltin<"__builtin_ia32_vfmadd231nepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmadd231nepbf16256
      : ClangBuiltin<"__builtin_ia32_vfmadd231nepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmadd231nepbf16128
      : ClangBuiltin<"__builtin_ia32_vfmadd231nepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_v8bf16_ty  ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmsub213nepbf16512
      : ClangBuiltin<"__builtin_ia32_vfmsub213nepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmsub213nepbf16256
      : ClangBuiltin<"__builtin_ia32_vfmsub213nepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmsub213nepbf16128
      : ClangBuiltin<"__builtin_ia32_vfmsub213nepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_v8bf16_ty  ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmsub132nepbf16512
      : ClangBuiltin<"__builtin_ia32_vfmsub132nepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmsub132nepbf16256
      : ClangBuiltin<"__builtin_ia32_vfmsub132nepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmsub132nepbf16128
      : ClangBuiltin<"__builtin_ia32_vfmsub132nepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_v8bf16_ty  ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmsub231nepbf16512
      : ClangBuiltin<"__builtin_ia32_vfmsub231nepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmsub231nepbf16256
      : ClangBuiltin<"__builtin_ia32_vfmsub231nepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfmsub231nepbf16128
      : ClangBuiltin<"__builtin_ia32_vfmsub231nepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_v8bf16_ty  ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmadd213nepbf16512
      : ClangBuiltin<"__builtin_ia32_vfnmadd213nepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmadd213nepbf16256
      : ClangBuiltin<"__builtin_ia32_vfnmadd213nepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmadd213nepbf16128
      : ClangBuiltin<"__builtin_ia32_vfnmadd213nepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_v8bf16_ty  ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmadd132nepbf16512
      : ClangBuiltin<"__builtin_ia32_vfnmadd132nepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmadd132nepbf16256
      : ClangBuiltin<"__builtin_ia32_vfnmadd132nepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmadd132nepbf16128
      : ClangBuiltin<"__builtin_ia32_vfnmadd132nepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_v8bf16_ty  ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmadd231nepbf16512
      : ClangBuiltin<"__builtin_ia32_vfnmadd231nepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmadd231nepbf16256
      : ClangBuiltin<"__builtin_ia32_vfnmadd231nepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmadd231nepbf16128
      : ClangBuiltin<"__builtin_ia32_vfnmadd231nepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_v8bf16_ty  ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmsub213nepbf16512
      : ClangBuiltin<"__builtin_ia32_vfnmsub213nepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmsub213nepbf16256
      : ClangBuiltin<"__builtin_ia32_vfnmsub213nepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmsub213nepbf16128
      : ClangBuiltin<"__builtin_ia32_vfnmsub213nepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_v8bf16_ty  ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmsub132nepbf16512
      : ClangBuiltin<"__builtin_ia32_vfnmsub132nepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmsub132nepbf16256
      : ClangBuiltin<"__builtin_ia32_vfnmsub132nepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmsub132nepbf16128
      : ClangBuiltin<"__builtin_ia32_vfnmsub132nepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_v8bf16_ty  ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmsub231nepbf16512
      : ClangBuiltin<"__builtin_ia32_vfnmsub231nepbf16512">,
        Intrinsic<[ llvm_v32bf16_ty ],
                  [ llvm_v32bf16_ty, llvm_v32bf16_ty, llvm_v32bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmsub231nepbf16256
      : ClangBuiltin<"__builtin_ia32_vfnmsub231nepbf16256">,
        Intrinsic<[ llvm_v16bf16_ty ],
                  [ llvm_v16bf16_ty, llvm_v16bf16_ty, llvm_v16bf16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512bf16ne_vfnmsub231nepbf16128
      : ClangBuiltin<"__builtin_ia32_vfnmsub231nepbf16128">,
        Intrinsic<[ llvm_v8bf16_ty ],
                  [ llvm_v8bf16_ty, llvm_v8bf16_ty, llvm_v8bf16_ty  ],
                  [ IntrNoMem ]>;
#endif // INTEL_FEATURE_ISA_AVX512_BF16_NE
#endif // INTEL_CUSTOMIZATION
  def int_x86_avx512fp16_add_ph_512
      : ClangBuiltin<"__builtin_ia32_addph512">,
        Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;
  def int_x86_avx512fp16_sub_ph_512
      : ClangBuiltin<"__builtin_ia32_subph512">,
        Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;
  def int_x86_avx512fp16_mul_ph_512
      : ClangBuiltin<"__builtin_ia32_mulph512">,
        Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;
  def int_x86_avx512fp16_div_ph_512
      : ClangBuiltin<"__builtin_ia32_divph512">,
        Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;
  def int_x86_avx512fp16_max_ph_128
      : ClangBuiltin<"__builtin_ia32_maxph128">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_max_ph_256
      : ClangBuiltin<"__builtin_ia32_maxph256">,
        Intrinsic<[ llvm_v16f16_ty ],
                  [ llvm_v16f16_ty, llvm_v16f16_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_max_ph_512
      : ClangBuiltin<"__builtin_ia32_maxph512">,
        Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;
  def int_x86_avx512fp16_min_ph_128
      : ClangBuiltin<"__builtin_ia32_minph128">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_min_ph_256
      : ClangBuiltin<"__builtin_ia32_minph256">,
        Intrinsic<[ llvm_v16f16_ty ],
                  [ llvm_v16f16_ty, llvm_v16f16_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_min_ph_512
      : ClangBuiltin<"__builtin_ia32_minph512">,
        Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;

  def int_x86_avx512fp16_mask_cmp_ph_512
      : Intrinsic<[ llvm_v32i1_ty ],
                  [ llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_v32i1_ty,
                  llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_cmp_ph_256
      : Intrinsic<[ llvm_v16i1_ty ],
                  [ llvm_v16f16_ty, llvm_v16f16_ty, llvm_i32_ty, llvm_v16i1_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;
  def int_x86_avx512fp16_mask_cmp_ph_128
      : Intrinsic<[ llvm_v8i1_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_i32_ty, llvm_v8i1_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;

  def int_x86_avx512fp16_mask_add_sh_round
      : ClangBuiltin<"__builtin_ia32_addsh_round_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_sub_sh_round
      : ClangBuiltin<"__builtin_ia32_subsh_round_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_mul_sh_round
      : ClangBuiltin<"__builtin_ia32_mulsh_round_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_div_sh_round
      : ClangBuiltin<"__builtin_ia32_divsh_round_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_min_sh_round
      : ClangBuiltin<"__builtin_ia32_minsh_round_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_max_sh_round
      : ClangBuiltin<"__builtin_ia32_maxsh_round_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_cmp_sh
      : ClangBuiltin<"__builtin_ia32_cmpsh_mask">,
        Intrinsic<[ llvm_i8_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_i32_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_vcomi_sh
      : ClangBuiltin<"__builtin_ia32_vcomish">,
        Intrinsic<[ llvm_i32_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_i32_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<3>> ]>;

  def int_x86_avx512fp16_mask_vcvtph2psx_128
      : ClangBuiltin<"__builtin_ia32_vcvtph2psx128_mask">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v8f16_ty, llvm_v4f32_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2psx_256
      : ClangBuiltin<"__builtin_ia32_vcvtph2psx256_mask">,
        Intrinsic<[ llvm_v8f32_ty ],
                  [ llvm_v8f16_ty, llvm_v8f32_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2psx_512
      : ClangBuiltin<"__builtin_ia32_vcvtph2psx512_mask">,
        Intrinsic<[ llvm_v16f32_ty ],
                  [ llvm_v16f16_ty, llvm_v16f32_ty, llvm_i16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_vcvtps2phx_128
      : ClangBuiltin<"__builtin_ia32_vcvtps2phx128_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v4f32_ty, llvm_v8f16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtps2phx_256
      : ClangBuiltin<"__builtin_ia32_vcvtps2phx256_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f32_ty, llvm_v8f16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtps2phx_512
      : ClangBuiltin<"__builtin_ia32_vcvtps2phx512_mask">,
        Intrinsic<[ llvm_v16f16_ty ],
                  [ llvm_v16f32_ty, llvm_v16f16_ty, llvm_i16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_vcvtpd2ph_128
      : ClangBuiltin<"__builtin_ia32_vcvtpd2ph128_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v2f64_ty, llvm_v8f16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtpd2ph_256
      : ClangBuiltin<"__builtin_ia32_vcvtpd2ph256_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v4f64_ty, llvm_v8f16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtpd2ph_512
      : ClangBuiltin<"__builtin_ia32_vcvtpd2ph512_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f64_ty, llvm_v8f16_ty, llvm_i8_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_vcvtph2pd_128
      : ClangBuiltin<"__builtin_ia32_vcvtph2pd128_mask">,
        Intrinsic<[ llvm_v2f64_ty ],
                  [ llvm_v8f16_ty, llvm_v2f64_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2pd_256
      : ClangBuiltin<"__builtin_ia32_vcvtph2pd256_mask">,
        Intrinsic<[ llvm_v4f64_ty ],
                  [ llvm_v8f16_ty, llvm_v4f64_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2pd_512
      : ClangBuiltin<"__builtin_ia32_vcvtph2pd512_mask">,
        Intrinsic<[ llvm_v8f64_ty ],
                  [ llvm_v8f16_ty, llvm_v8f64_ty, llvm_i8_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_vcvtsh2ss_round
      : ClangBuiltin<"__builtin_ia32_vcvtsh2ss_round_mask">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v4f32_ty, llvm_v8f16_ty, llvm_v4f32_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_vcvtss2sh_round
      : ClangBuiltin<"__builtin_ia32_vcvtss2sh_round_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v4f32_ty, llvm_v8f16_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_vcvtsd2sh_round
      : ClangBuiltin<"__builtin_ia32_vcvtsd2sh_round_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v2f64_ty, llvm_v8f16_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_vcvtsh2sd_round
      : ClangBuiltin<"__builtin_ia32_vcvtsh2sd_round_mask">,
        Intrinsic<[ llvm_v2f64_ty ],
                  [ llvm_v2f64_ty, llvm_v8f16_ty, llvm_v2f64_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;

  def int_x86_avx512fp16_mask_vcvtph2w_128
      : ClangBuiltin<"__builtin_ia32_vcvtph2w128_mask">,
        Intrinsic<[ llvm_v8i16_ty ],
                  [ llvm_v8f16_ty, llvm_v8i16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2w_256
      : ClangBuiltin<"__builtin_ia32_vcvtph2w256_mask">,
        Intrinsic<[ llvm_v16i16_ty ],
                  [ llvm_v16f16_ty, llvm_v16i16_ty, llvm_i16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2w_512
      : ClangBuiltin<"__builtin_ia32_vcvtph2w512_mask">,
        Intrinsic<[ llvm_v32i16_ty ],
                  [ llvm_v32f16_ty, llvm_v32i16_ty, llvm_i32_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_vcvttph2w_128
      : ClangBuiltin<"__builtin_ia32_vcvttph2w128_mask">,
        Intrinsic<[ llvm_v8i16_ty ],
                  [ llvm_v8f16_ty, llvm_v8i16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvttph2w_256
      : ClangBuiltin<"__builtin_ia32_vcvttph2w256_mask">,
        Intrinsic<[ llvm_v16i16_ty ],
                  [ llvm_v16f16_ty, llvm_v16i16_ty, llvm_i16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvttph2w_512
      : ClangBuiltin<"__builtin_ia32_vcvttph2w512_mask">,
        Intrinsic<[ llvm_v32i16_ty ],
                  [ llvm_v32f16_ty, llvm_v32i16_ty, llvm_i32_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_vcvtph2uw_128
      : ClangBuiltin<"__builtin_ia32_vcvtph2uw128_mask">,
        Intrinsic<[ llvm_v8i16_ty ],
                  [ llvm_v8f16_ty, llvm_v8i16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2uw_256
      : ClangBuiltin<"__builtin_ia32_vcvtph2uw256_mask">,
        Intrinsic<[ llvm_v16i16_ty ],
                  [ llvm_v16f16_ty, llvm_v16i16_ty, llvm_i16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2uw_512
      : ClangBuiltin<"__builtin_ia32_vcvtph2uw512_mask">,
        Intrinsic<[ llvm_v32i16_ty ],
                  [ llvm_v32f16_ty, llvm_v32i16_ty, llvm_i32_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_vcvttph2uw_128
      : ClangBuiltin<"__builtin_ia32_vcvttph2uw128_mask">,
        Intrinsic<[ llvm_v8i16_ty ],
                  [ llvm_v8f16_ty, llvm_v8i16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvttph2uw_256
      : ClangBuiltin<"__builtin_ia32_vcvttph2uw256_mask">,
        Intrinsic<[ llvm_v16i16_ty ],
                  [ llvm_v16f16_ty, llvm_v16i16_ty, llvm_i16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvttph2uw_512
      : ClangBuiltin<"__builtin_ia32_vcvttph2uw512_mask">,
        Intrinsic<[ llvm_v32i16_ty ],
                  [ llvm_v32f16_ty, llvm_v32i16_ty, llvm_i32_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;

  def int_x86_avx512fp16_mask_vcvtph2dq_128
      : ClangBuiltin<"__builtin_ia32_vcvtph2dq128_mask">,
        Intrinsic<[ llvm_v4i32_ty ],
                  [ llvm_v8f16_ty, llvm_v4i32_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2dq_256
      : ClangBuiltin<"__builtin_ia32_vcvtph2dq256_mask">,
        Intrinsic<[ llvm_v8i32_ty ],
                  [ llvm_v8f16_ty, llvm_v8i32_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2dq_512
      : ClangBuiltin<"__builtin_ia32_vcvtph2dq512_mask">,
        Intrinsic<[ llvm_v16i32_ty ],
                  [ llvm_v16f16_ty, llvm_v16i32_ty, llvm_i16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_vcvtph2udq_128
      : ClangBuiltin<"__builtin_ia32_vcvtph2udq128_mask">,
        Intrinsic<[ llvm_v4i32_ty ],
                  [ llvm_v8f16_ty, llvm_v4i32_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2udq_256
      : ClangBuiltin<"__builtin_ia32_vcvtph2udq256_mask">,
        Intrinsic<[ llvm_v8i32_ty ],
                  [ llvm_v8f16_ty, llvm_v8i32_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2udq_512
      : ClangBuiltin<"__builtin_ia32_vcvtph2udq512_mask">,
        Intrinsic<[ llvm_v16i32_ty ],
                  [ llvm_v16f16_ty, llvm_v16i32_ty, llvm_i16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_vcvtdq2ph_128
      : ClangBuiltin<"__builtin_ia32_vcvtdq2ph128_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v4i32_ty, llvm_v8f16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtudq2ph_128
      : ClangBuiltin<"__builtin_ia32_vcvtudq2ph128_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v4i32_ty, llvm_v8f16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvttph2dq_128
      : ClangBuiltin<"__builtin_ia32_vcvttph2dq128_mask">,
        Intrinsic<[ llvm_v4i32_ty ],
                  [ llvm_v8f16_ty, llvm_v4i32_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvttph2dq_256
      : ClangBuiltin<"__builtin_ia32_vcvttph2dq256_mask">,
        Intrinsic<[ llvm_v8i32_ty ],
                  [ llvm_v8f16_ty, llvm_v8i32_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvttph2dq_512
      : ClangBuiltin<"__builtin_ia32_vcvttph2dq512_mask">,
        Intrinsic<[ llvm_v16i32_ty ],
                  [ llvm_v16f16_ty, llvm_v16i32_ty, llvm_i16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_vcvttph2udq_128
      : ClangBuiltin<"__builtin_ia32_vcvttph2udq128_mask">,
        Intrinsic<[ llvm_v4i32_ty ],
                  [ llvm_v8f16_ty, llvm_v4i32_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvttph2udq_256
      : ClangBuiltin<"__builtin_ia32_vcvttph2udq256_mask">,
        Intrinsic<[ llvm_v8i32_ty ],
                  [ llvm_v8f16_ty, llvm_v8i32_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvttph2udq_512
      : ClangBuiltin<"__builtin_ia32_vcvttph2udq512_mask">,
        Intrinsic<[ llvm_v16i32_ty ],
                  [ llvm_v16f16_ty, llvm_v16i32_ty, llvm_i16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;

  def int_x86_avx512fp16_mask_vcvtqq2ph_128
      : ClangBuiltin<"__builtin_ia32_vcvtqq2ph128_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v2i64_ty, llvm_v8f16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtqq2ph_256
      : ClangBuiltin<"__builtin_ia32_vcvtqq2ph256_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v4i64_ty, llvm_v8f16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2qq_128
      : ClangBuiltin<"__builtin_ia32_vcvtph2qq128_mask">,
        Intrinsic<[ llvm_v2i64_ty ],
                  [ llvm_v8f16_ty, llvm_v2i64_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2qq_256
      : ClangBuiltin<"__builtin_ia32_vcvtph2qq256_mask">,
        Intrinsic<[ llvm_v4i64_ty ],
                  [ llvm_v8f16_ty, llvm_v4i64_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2qq_512
      : ClangBuiltin<"__builtin_ia32_vcvtph2qq512_mask">,
        Intrinsic<[ llvm_v8i64_ty ],
                  [ llvm_v8f16_ty, llvm_v8i64_ty, llvm_i8_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_vcvtuqq2ph_128
      : ClangBuiltin<"__builtin_ia32_vcvtuqq2ph128_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v2i64_ty, llvm_v8f16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtuqq2ph_256
      : ClangBuiltin<"__builtin_ia32_vcvtuqq2ph256_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v4i64_ty, llvm_v8f16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2uqq_128
      : ClangBuiltin<"__builtin_ia32_vcvtph2uqq128_mask">,
        Intrinsic<[ llvm_v2i64_ty ],
                  [ llvm_v8f16_ty, llvm_v2i64_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2uqq_256
      : ClangBuiltin<"__builtin_ia32_vcvtph2uqq256_mask">,
        Intrinsic<[ llvm_v4i64_ty ],
                  [ llvm_v8f16_ty, llvm_v4i64_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvtph2uqq_512
      : ClangBuiltin<"__builtin_ia32_vcvtph2uqq512_mask">,
        Intrinsic<[ llvm_v8i64_ty ],
                  [ llvm_v8f16_ty, llvm_v8i64_ty, llvm_i8_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_vcvttph2qq_128
      : ClangBuiltin<"__builtin_ia32_vcvttph2qq128_mask">,
        Intrinsic<[ llvm_v2i64_ty ],
                  [ llvm_v8f16_ty, llvm_v2i64_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvttph2qq_256
      : ClangBuiltin<"__builtin_ia32_vcvttph2qq256_mask">,
        Intrinsic<[ llvm_v4i64_ty ],
                  [ llvm_v8f16_ty, llvm_v4i64_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvttph2qq_512
      : ClangBuiltin<"__builtin_ia32_vcvttph2qq512_mask">,
        Intrinsic<[ llvm_v8i64_ty ],
                  [ llvm_v8f16_ty, llvm_v8i64_ty, llvm_i8_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_vcvttph2uqq_128
      : ClangBuiltin<"__builtin_ia32_vcvttph2uqq128_mask">,
        Intrinsic<[ llvm_v2i64_ty ],
                  [ llvm_v8f16_ty, llvm_v2i64_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvttph2uqq_256
      : ClangBuiltin<"__builtin_ia32_vcvttph2uqq256_mask">,
        Intrinsic<[ llvm_v4i64_ty ],
                  [ llvm_v8f16_ty, llvm_v4i64_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vcvttph2uqq_512
      : ClangBuiltin<"__builtin_ia32_vcvttph2uqq512_mask">,
        Intrinsic<[ llvm_v8i64_ty ],
                  [ llvm_v8f16_ty, llvm_v8i64_ty, llvm_i8_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;

  def int_x86_avx512fp16_vcvtsh2si32
      : ClangBuiltin<"__builtin_ia32_vcvtsh2si32">,
        Intrinsic<[ llvm_i32_ty ], [ llvm_v8f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_vcvtsh2usi32
      : ClangBuiltin<"__builtin_ia32_vcvtsh2usi32">,
        Intrinsic<[ llvm_i32_ty ], [ llvm_v8f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_vcvtsh2si64
      : ClangBuiltin<"__builtin_ia32_vcvtsh2si64">,
        Intrinsic<[ llvm_i64_ty ], [ llvm_v8f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_vcvtsh2usi64
      : ClangBuiltin<"__builtin_ia32_vcvtsh2usi64">,
        Intrinsic<[ llvm_i64_ty ], [ llvm_v8f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_vcvtusi2sh
      : ClangBuiltin<"__builtin_ia32_vcvtusi2sh">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_i32_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;
  def int_x86_avx512fp16_vcvtusi642sh
      : ClangBuiltin<"__builtin_ia32_vcvtusi642sh">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_i64_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;
  def int_x86_avx512fp16_vcvtsi2sh
      : ClangBuiltin<"__builtin_ia32_vcvtsi2sh">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_i32_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;
  def int_x86_avx512fp16_vcvtsi642sh
      : ClangBuiltin<"__builtin_ia32_vcvtsi642sh">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_i64_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>> ]>;
  def int_x86_avx512fp16_vcvttsh2si32
      : ClangBuiltin<"__builtin_ia32_vcvttsh2si32">,
        Intrinsic<[ llvm_i32_ty ], [ llvm_v8f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_vcvttsh2si64
      : ClangBuiltin<"__builtin_ia32_vcvttsh2si64">,
        Intrinsic<[ llvm_i64_ty ], [ llvm_v8f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_vcvttsh2usi32
      : ClangBuiltin<"__builtin_ia32_vcvttsh2usi32">,
        Intrinsic<[ llvm_i32_ty ], [ llvm_v8f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_vcvttsh2usi64
      : ClangBuiltin<"__builtin_ia32_vcvttsh2usi64">,
        Intrinsic<[ llvm_i64_ty ], [ llvm_v8f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;

  def int_x86_avx512fp16_sqrt_ph_512
      : Intrinsic<[ llvm_v32f16_ty ], [ llvm_v32f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_mask_sqrt_sh
      : Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_rsqrt_ph_128
      : ClangBuiltin<"__builtin_ia32_rsqrtph128_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_rsqrt_ph_256
      : ClangBuiltin<"__builtin_ia32_rsqrtph256_mask">,
        Intrinsic<[ llvm_v16f16_ty ],
                  [ llvm_v16f16_ty, llvm_v16f16_ty, llvm_i16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_rsqrt_ph_512
      : ClangBuiltin<"__builtin_ia32_rsqrtph512_mask">,
        Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_rsqrt_sh
      : ClangBuiltin<"__builtin_ia32_rsqrtsh_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_rcp_ph_128
      : ClangBuiltin<"__builtin_ia32_rcpph128_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_rcp_ph_256
      : ClangBuiltin<"__builtin_ia32_rcpph256_mask">,
        Intrinsic<[ llvm_v16f16_ty ],
                  [ llvm_v16f16_ty, llvm_v16f16_ty, llvm_i16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_rcp_ph_512
      : ClangBuiltin<"__builtin_ia32_rcpph512_mask">,
        Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_rcp_sh
      : ClangBuiltin<"__builtin_ia32_rcpsh_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_reduce_ph_128
      : ClangBuiltin<"__builtin_ia32_reduceph128_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_i32_ty, llvm_v8f16_ty, llvm_i8_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_mask_reduce_ph_256
      : ClangBuiltin<"__builtin_ia32_reduceph256_mask">,
        Intrinsic<[ llvm_v16f16_ty ],
                  [ llvm_v16f16_ty, llvm_i32_ty, llvm_v16f16_ty, llvm_i16_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_mask_reduce_ph_512
      : ClangBuiltin<"__builtin_ia32_reduceph512_mask">,
        Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_reduce_sh
      : ClangBuiltin<"__builtin_ia32_reducesh_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty,
                    llvm_i32_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>>, ImmArg<ArgIndex<5>> ]>;
  def int_x86_avx512fp16_fpclass_ph_128
      : Intrinsic<[ llvm_v8i1_ty ], [ llvm_v8f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_fpclass_ph_256
      : Intrinsic<[ llvm_v16i1_ty ], [ llvm_v16f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_fpclass_ph_512
      : Intrinsic<[ llvm_v32i1_ty ], [ llvm_v32f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_mask_fpclass_sh
      : ClangBuiltin<"__builtin_ia32_fpclasssh_mask">,
        Intrinsic<[ llvm_i8_ty ], [ llvm_v8f16_ty, llvm_i32_ty, llvm_i8_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_mask_getexp_ph_128
      : ClangBuiltin<"__builtin_ia32_getexpph128_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty ], [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_getexp_ph_256
      : ClangBuiltin<"__builtin_ia32_getexpph256_mask">,
        Intrinsic<[ llvm_v16f16_ty ],
                  [ llvm_v16f16_ty, llvm_v16f16_ty, llvm_i16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_getexp_ph_512
      : ClangBuiltin<"__builtin_ia32_getexpph512_mask">,
        Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_mask_getexp_sh
      : ClangBuiltin<"__builtin_ia32_getexpsh128_round_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_getmant_ph_128
      : ClangBuiltin<"__builtin_ia32_getmantph128_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_i32_ty, llvm_v8f16_ty, llvm_i8_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_mask_getmant_ph_256
      : ClangBuiltin<"__builtin_ia32_getmantph256_mask">,
        Intrinsic<[ llvm_v16f16_ty ],
                  [ llvm_v16f16_ty, llvm_i32_ty, llvm_v16f16_ty, llvm_i16_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_mask_getmant_ph_512
      : ClangBuiltin<"__builtin_ia32_getmantph512_mask">,
        Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_getmant_sh
      : ClangBuiltin<"__builtin_ia32_getmantsh_round_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_i32_ty, llvm_v8f16_ty,
                    llvm_i8_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<2>>, ImmArg<ArgIndex<5>> ]>;
  def int_x86_avx512fp16_mask_rndscale_ph_128
      : ClangBuiltin<"__builtin_ia32_rndscaleph_128_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_i32_ty, llvm_v8f16_ty, llvm_i8_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_mask_rndscale_ph_256
      : ClangBuiltin<"__builtin_ia32_rndscaleph_256_mask">,
        Intrinsic<[ llvm_v16f16_ty ],
                  [ llvm_v16f16_ty, llvm_i32_ty, llvm_v16f16_ty, llvm_i16_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>> ]>;
  def int_x86_avx512fp16_mask_rndscale_ph_512
      : ClangBuiltin<"__builtin_ia32_rndscaleph_mask">,
        Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_i32_ty, llvm_v32f16_ty, llvm_i32_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<1>>, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_rndscale_sh
      : ClangBuiltin<"__builtin_ia32_rndscalesh_round_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty,
                    llvm_i32_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>>, ImmArg<ArgIndex<5>> ]>;
  def int_x86_avx512fp16_mask_scalef_ph_128
      : ClangBuiltin<"__builtin_ia32_scalefph128_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_scalef_ph_256
      : ClangBuiltin<"__builtin_ia32_scalefph256_mask">,
        Intrinsic<[ llvm_v16f16_ty ],
                  [ llvm_v16f16_ty, llvm_v16f16_ty, llvm_v16f16_ty, llvm_i16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_scalef_ph_512
      : ClangBuiltin<"__builtin_ia32_scalefph512_mask">,
        Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_scalef_sh
      : ClangBuiltin<"__builtin_ia32_scalefsh_round_mask">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;

  def int_x86_avx512fp16_vfmadd_ph_512
      : Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_vfmaddsub_ph_128
      : ClangBuiltin<"__builtin_ia32_vfmaddsubph">,
        Intrinsic<[ llvm_v8f16_ty ],
                  [ llvm_v8f16_ty, llvm_v8f16_ty, llvm_v8f16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_vfmaddsub_ph_256
      : ClangBuiltin<"__builtin_ia32_vfmaddsubph256">,
        Intrinsic<[ llvm_v16f16_ty ],
                  [ llvm_v16f16_ty, llvm_v16f16_ty, llvm_v16f16_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_vfmaddsub_ph_512
      : Intrinsic<[ llvm_v32f16_ty ],
                  [ llvm_v32f16_ty, llvm_v32f16_ty, llvm_v32f16_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;
  def int_x86_avx512fp16_vfmadd_f16
      : Intrinsic<[ llvm_half_ty ],
                  [ llvm_half_ty, llvm_half_ty, llvm_half_ty, llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<3>> ]>;

  def int_x86_avx512fp16_mask_vfcmadd_cph_128
      : ClangBuiltin<"__builtin_ia32_vfcmaddcph128_mask">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_maskz_vfcmadd_cph_128
      : ClangBuiltin<"__builtin_ia32_vfcmaddcph128_maskz">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vfcmadd_cph_256
      : ClangBuiltin<"__builtin_ia32_vfcmaddcph256_mask">,
        Intrinsic<[ llvm_v8f32_ty ],
                  [ llvm_v8f32_ty, llvm_v8f32_ty, llvm_v8f32_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_maskz_vfcmadd_cph_256
      : ClangBuiltin<"__builtin_ia32_vfcmaddcph256_maskz">,
        Intrinsic<[ llvm_v8f32_ty ],
                  [ llvm_v8f32_ty, llvm_v8f32_ty, llvm_v8f32_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vfcmadd_cph_512
      : ClangBuiltin<"__builtin_ia32_vfcmaddcph512_mask3">,
        Intrinsic<[ llvm_v16f32_ty ],
                  [ llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty, llvm_i16_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_maskz_vfcmadd_cph_512
      : ClangBuiltin<"__builtin_ia32_vfcmaddcph512_maskz">,
        Intrinsic<[ llvm_v16f32_ty ],
                  [ llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty, llvm_i16_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_vfmadd_cph_128
      : ClangBuiltin<"__builtin_ia32_vfmaddcph128_mask">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_maskz_vfmadd_cph_128
      : ClangBuiltin<"__builtin_ia32_vfmaddcph128_maskz">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vfmadd_cph_256
      : ClangBuiltin<"__builtin_ia32_vfmaddcph256_mask">,
        Intrinsic<[ llvm_v8f32_ty ],
                  [ llvm_v8f32_ty, llvm_v8f32_ty, llvm_v8f32_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_maskz_vfmadd_cph_256
      : ClangBuiltin<"__builtin_ia32_vfmaddcph256_maskz">,
        Intrinsic<[ llvm_v8f32_ty ],
                  [ llvm_v8f32_ty, llvm_v8f32_ty, llvm_v8f32_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vfmadd_cph_512
      : ClangBuiltin<"__builtin_ia32_vfmaddcph512_mask3">,
        Intrinsic<[ llvm_v16f32_ty ],
                  [ llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty, llvm_i16_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_maskz_vfmadd_cph_512
      : ClangBuiltin<"__builtin_ia32_vfmaddcph512_maskz">,
        Intrinsic<[ llvm_v16f32_ty ],
                  [ llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty, llvm_i16_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_vfmadd_csh
      : ClangBuiltin<"__builtin_ia32_vfmaddcsh_mask">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_maskz_vfmadd_csh
      : ClangBuiltin<"__builtin_ia32_vfmaddcsh_maskz">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_vfcmadd_csh
      : ClangBuiltin<"__builtin_ia32_vfcmaddcsh_mask">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_maskz_vfcmadd_csh
      : ClangBuiltin<"__builtin_ia32_vfcmaddcsh_maskz">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_vfmul_cph_128
      : ClangBuiltin<"__builtin_ia32_vfmulcph128_mask">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vfcmul_cph_128
      : ClangBuiltin<"__builtin_ia32_vfcmulcph128_mask">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vfmul_cph_256
      : ClangBuiltin<"__builtin_ia32_vfmulcph256_mask">,
        Intrinsic<[ llvm_v8f32_ty ],
                  [ llvm_v8f32_ty, llvm_v8f32_ty, llvm_v8f32_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vfcmul_cph_256
      : ClangBuiltin<"__builtin_ia32_vfcmulcph256_mask">,
        Intrinsic<[ llvm_v8f32_ty ],
                  [ llvm_v8f32_ty, llvm_v8f32_ty, llvm_v8f32_ty, llvm_i8_ty ],
                  [ IntrNoMem ]>;
  def int_x86_avx512fp16_mask_vfmul_cph_512
      : ClangBuiltin<"__builtin_ia32_vfmulcph512_mask">,
        Intrinsic<[ llvm_v16f32_ty ],
                  [ llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty, llvm_i16_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_vfcmul_cph_512
      : ClangBuiltin<"__builtin_ia32_vfcmulcph512_mask">,
        Intrinsic<[ llvm_v16f32_ty ],
                  [ llvm_v16f32_ty, llvm_v16f32_ty, llvm_v16f32_ty, llvm_i16_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_vfmul_csh
      : ClangBuiltin<"__builtin_ia32_vfmulcsh_mask">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
  def int_x86_avx512fp16_mask_vfcmul_csh
      : ClangBuiltin<"__builtin_ia32_vfcmulcsh_mask">,
        Intrinsic<[ llvm_v4f32_ty ],
                  [ llvm_v4f32_ty, llvm_v4f32_ty, llvm_v4f32_ty, llvm_i8_ty,
                    llvm_i32_ty ],
                  [ IntrNoMem, ImmArg<ArgIndex<4>> ]>;
}
