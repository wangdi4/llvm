#if INTEL_FEATURE_CSA
//===- Intel_IntrinsicsCSA.td - Defines csa intrinsics -----*- tablegen -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file defines all of the CSA-specific intrinsics.
///
//===----------------------------------------------------------------------===//

let TargetPrefix = "csa" in {  // All intrinsics start with "llvm.csa.".

// Placeholder "llvm.csa.directive" intrinsic, for experimentation.
def int_csa_directive : ClangBuiltin<"__builtin_csa_directive">,
    Intrinsic<[], [llvm_i32_ty], [IntrArgMemOnly] /* , "llvm.csa.directive" */>;

// "llvm.csa.parallel_loop" intrinsic.  A directive, probably resulting from
// an OMP pragma, that indicates that what follows is a parallel loop. Using
// IntrArgMemOnly forces sequencing wrt other operations (though this might
// not be quite right).
def int_csa_parallel_loop : ClangBuiltin<"__builtin_csa_parallel_loop">,
    Intrinsic<[], [], [IntrArgMemOnly]>;

// llvm.csa.parallel_region_entry
def int_csa_parallel_region_entry :
        ClangBuiltin<"__builtin_csa_parallel_region_entry">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty],
                  [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.parallel_region_exit
def int_csa_parallel_region_exit :
        ClangBuiltin<"__builtin_csa_parallel_region_exit">,
        Intrinsic<[], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.parallel_section_entry
def int_csa_parallel_section_entry :
        ClangBuiltin<"__builtin_csa_parallel_section_entry">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty],
                  [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.parallel_section_exit
def int_csa_parallel_section_exit :
        ClangBuiltin<"__builtin_csa_parallel_section_exit">,
        Intrinsic<[], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.spmdization
def int_csa_spmdization : ClangBuiltin<"__builtin_csa_spmdization">,
    Intrinsic<[], [llvm_i32_ty, llvm_ptr_ty],
              [IntrInaccessibleMemOrArgMemOnly,
               IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.spmd
def int_csa_spmd : ClangBuiltin<"__builtin_csa_spmd">,
    Intrinsic<[], [llvm_i32_ty, llvm_i32_ty],
              [IntrInaccessibleMemOrArgMemOnly,
               IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.spmd.worker.num
def int_csa_spmd_worker_num : ClangBuiltin<"__builtin_csa_spmd_worker_num">,
    Intrinsic<[llvm_i32_ty], [],
              [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.spmdization_entry
def int_csa_spmdization_entry : ClangBuiltin<"__builtin_csa_spmdization_entry">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty],
              [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.spmdization_exit
def int_csa_spmdization_exit : ClangBuiltin<"__builtin_csa_spmdization_exit">,
    Intrinsic<[], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.spmdization_local_cache
def int_csa_spmdization_local_cache : ClangBuiltin<"__builtin_csa_spmdization_local_cache">,
    Intrinsic<[], [llvm_i32_ty, llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.pipeline_loop
def int_csa_pipeline_loop : ClangBuiltin<"__builtin_csa_pipeline_loop">,
    Intrinsic<[], [llvm_i32_ty], [IntrArgMemOnly]>;

// These are internal intrinsics not suitable for insertion by programmers.
// llvm.csa.pipeline_loop_entry
def int_csa_pipeline_loop_entry :
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.pipeline_loop_exit
def int_csa_pipeline_loop_exit :
    Intrinsic<[], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.pipelineable_loop
def int_csa_pipelineable_loop_marker :
    Intrinsic<[], [llvm_i64_ty], [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.local_cache
def int_csa_local_cache : ClangBuiltin<"__builtin_csa_local_cache">,
    Intrinsic<[], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.local_cache_region_begin
def int_csa_local_cache_region_begin :
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.local_cache_region_end
def int_csa_local_cache_region_end :
    Intrinsic<[], [llvm_i32_ty, llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly]>;

// Scalar intrinsics
def int_csa_clxmul8 : ClangBuiltin<"__builtin_csa_clxmul8">,
    Intrinsic<[llvm_i16_ty], [llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_clxmul16 : ClangBuiltin<"__builtin_csa_clxmul16">,
    Intrinsic<[llvm_i32_ty], [llvm_i16_ty, llvm_i16_ty], [IntrNoMem]>;

def int_csa_clxmul32 : ClangBuiltin<"__builtin_csa_clxmul32">,
    Intrinsic<[llvm_i64_ty], [llvm_i32_ty, llvm_i32_ty], [IntrNoMem]>;

def int_csa_rcp14f32: ClangBuiltin<"__builtin_csa_rcp14f32">,
    Intrinsic<[llvm_float_ty], [llvm_float_ty], [IntrNoMem]>;

def int_csa_rcp14f64: ClangBuiltin<"__builtin_csa_rcp14f64">,
    Intrinsic<[llvm_double_ty], [llvm_double_ty], [IntrNoMem]>;

def int_csa_rsqrt14f32: ClangBuiltin<"__builtin_csa_rsqrt14f32">,
    Intrinsic<[llvm_float_ty], [llvm_float_ty], [IntrNoMem]>;

def int_csa_rsqrt14f64: ClangBuiltin<"__builtin_csa_rsqrt14f64">,
    Intrinsic<[llvm_double_ty], [llvm_double_ty], [IntrNoMem]>;

def int_csa_ternlog_u1 : ClangBuiltin<"__builtin_csa_ternlog_u1">,
    Intrinsic<[llvm_i1_ty], [llvm_i1_ty, llvm_i1_ty, llvm_i1_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_ternlog_u8 : ClangBuiltin<"__builtin_csa_ternlog_u8">,
    Intrinsic<[llvm_i8_ty], [llvm_i8_ty, llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_ternlog_u16 : ClangBuiltin<"__builtin_csa_ternlog_u16">,
    Intrinsic<[llvm_i16_ty], [llvm_i16_ty, llvm_i16_ty, llvm_i16_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_ternlog_u32 : ClangBuiltin<"__builtin_csa_ternlog_u32">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_ternlog_u64 : ClangBuiltin<"__builtin_csa_ternlog_u64">,
    Intrinsic<[llvm_i64_ty], [llvm_i64_ty, llvm_i64_ty, llvm_i64_ty, llvm_i8_ty], [IntrNoMem]>;

// SIMD intrinsics (16x4).
def int_csa_addf16x4 : ClangBuiltin<"__builtin_csa_addf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_subf16x4 : ClangBuiltin<"__builtin_csa_subf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_addsubf16x4 : ClangBuiltin<"__builtin_csa_addsubf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_subaddf16x4 : ClangBuiltin<"__builtin_csa_subaddf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_mulf16x4 : ClangBuiltin<"__builtin_csa_mulf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmaf16x4 : ClangBuiltin<"__builtin_csa_fmaf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmsf16x4 : ClangBuiltin<"__builtin_csa_fmsf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmrsf16x4 : ClangBuiltin<"__builtin_csa_fmrsf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmasf16x4 : ClangBuiltin<"__builtin_csa_fmasf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmsaf16x4 : ClangBuiltin<"__builtin_csa_fmsaf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fnmsf16x4 : ClangBuiltin<"__builtin_csa_fnmsf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

// SIMD intrinsics (32x2).
def int_csa_addf32x2 : ClangBuiltin<"__builtin_csa_addf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_subf32x2 : ClangBuiltin<"__builtin_csa_subf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_addsubf32x2 : ClangBuiltin<"__builtin_csa_addsubf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_subaddf32x2 : ClangBuiltin<"__builtin_csa_subaddf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_mulf32x2 : ClangBuiltin<"__builtin_csa_mulf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmaf32x2 : ClangBuiltin<"__builtin_csa_fmaf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmsf32x2 : ClangBuiltin<"__builtin_csa_fmsf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmrsf32x2 : ClangBuiltin<"__builtin_csa_fmrsf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmasf32x2 : ClangBuiltin<"__builtin_csa_fmasf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmsaf32x2 : ClangBuiltin<"__builtin_csa_fmsaf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fnmsf32x2 : ClangBuiltin<"__builtin_csa_fnmsf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

// Integer vector intrinsics //

// Compute the bitwise AND of corresponding 8-bit integers in a and b,and set the
// corresponding bit in the result if the result is non-zero.
// int __m64_test_epi8(__m64i a, __m64i b)
def int_csa_test8x8 : ClangBuiltin<"__builtin_csa_test8x8">,
    Intrinsic<[llvm_i8_ty], [llvm_v8i8_ty, llvm_v8i8_ty], [IntrNoMem]>;

// Compute the bitwise AND of corresponding 16-bit integers in a and b, and set the
// corresponding bit in the result if the result is non-zero.
// int __m64_test_epi16(__m64i a, __m64i b)
def int_csa_test16x4 : ClangBuiltin<"__builtin_csa_test16x4">,
    Intrinsic<[llvm_i8_ty], [llvm_v4i16_ty, llvm_v4i16_ty], [IntrNoMem]>;

// Compute the bitwise AND of corresponding 32-bit integers in a and b, and set the
// corresponding bit in the result if the result is non-zero.
// int __m64_test_epi32(__m64i a, __m64i b)
def int_csa_test32x2 : ClangBuiltin<"__builtin_csa_test32x2">,
    Intrinsic<[llvm_i8_ty], [llvm_v2f32_ty, llvm_v2f32_ty], [IntrNoMem]>;

// Compute the absolute value of packed 8-bit integers in a, and return the unsigned results.
// __m64i __m64_abs_epi8(__m64i a)
def int_csa_abs8x8 : ClangBuiltin<"__builtin_csa_abs8x8">,
    Intrinsic<[llvm_v8i8_ty], [llvm_v8i8_ty], [IntrNoMem]>;

// Compute the absolute value of packed 16-bit integers in a, and return the unsigned results.
// __m64i __m64_abs_epi16(__m64i a)
def int_csa_abs16x4 : ClangBuiltin<"__builtin_csa_abs16x4">,
    Intrinsic<[llvm_v4i16_ty], [llvm_v4i16_ty], [IntrNoMem]>;

// Compute the average of packed 8-bit integers in a, and return the results.
// __m64i __m64_avg_epi8(__m64i a, __m64i b)
def int_csa_avgs8x8 : ClangBuiltin<"__builtin_csa_avgs8x8">,
    Intrinsic<[llvm_v8i8_ty], [llvm_v8i8_ty, llvm_v8i8_ty], [IntrNoMem]>;

// Compute the average of packed unsigned 8-bit integers in a, and return the results.
// __m64i __m64_avg_epu8(__m64i a, __m64i b)
def int_csa_avgu8x8 : ClangBuiltin<"__builtin_csa_avgu8x8">,
    Intrinsic<[llvm_v8i8_ty], [llvm_v8i8_ty, llvm_v8i8_ty], [IntrNoMem]>;

// Compute the average of packed 16-bit integers in a, and return the results.
// __m64i __m64_avg_epi16(__m64i a, __m64i b)
def int_csa_avgs16x4 : ClangBuiltin<"__builtin_csa_avgs16x4">,
    Intrinsic<[llvm_v4i16_ty], [llvm_v4i16_ty, llvm_v4i16_ty], [IntrNoMem]>;

// Compute the average of packed unsigned 16-bit integers in a, and return the results.
// __m64i __m64_avg_epu16(__m64i a, __m64i b)
def int_csa_avgu16x4 : ClangBuiltin<"__builtin_csa_avgu16x4">,
    Intrinsic<[llvm_v4i16_ty], [llvm_v4i16_ty, llvm_v4i16_ty], [IntrNoMem]>;

// Create mask from the most significant bit of each 8-bit element in a, and return the result.
// int __m64_movemask_epi8(__m64i a)
def int_csa_movmsk8x8 : ClangBuiltin<"__builtin_csa_movmsk8x8">,
    Intrinsic<[llvm_i8_ty], [llvm_v8i8_ty], [IntrNoMem]>;

// Create mask from the most significant bit of each 16-bit element in a, and return the result.
// int __m64_movemask_epi16(__m64i a)
def int_csa_movmsk16x4 : ClangBuiltin<"__builtin_csa_movmsk16x4">,
    Intrinsic<[llvm_i8_ty], [llvm_v4i16_ty], [IntrNoMem]>;

// Blend packed 8-bit integers from a and b using the bits in mask. Each element is picked
// from a if its corresponding bit in the mask is 0, and from b if it is 1.
// __m64i _mm64_blend_epi8(unsigned char mask, __m64i a, __m64i b)
def int_csa_blend8x8 : ClangBuiltin<"__builtin_csa_blend8x8">,
    Intrinsic<[llvm_v8i8_ty], [llvm_i8_ty, llvm_v8i8_ty, llvm_v8i8_ty], [IntrNoMem]>;

// Blend packed 16-bit integers from a and b using the bits in mask. Each element is picked
// from a if its corresponding bit in the mask is 0, and from b if it is 1.
// __m64i _mm64_blend_epi16(unsigned char mask, __m64i a, __m64i b)
def int_csa_blend16x4 : ClangBuiltin<"__builtin_csa_blend16x4">,
    Intrinsic<[llvm_v4i16_ty], [llvm_i8_ty, llvm_v4i16_ty, llvm_v4i16_ty], [IntrNoMem]>;

// Blend packed 32-bit floating point numbers from a and b using the bits in mask. Each element
// is picked from a if its corresponding bit in the mask is 0, and from b if it is 1.
// __m64f _mm64_blend_ps(unsigned char mask, __m64f a, __m64f b)
def int_csa_blend32x2 : ClangBuiltin<"__builtin_csa_blend32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_i8_ty, llvm_v2f32_ty, llvm_v2f32_ty], [IntrNoMem]>;

// Comparison intrinsics for SIMD.
def int_csa_cmp8x8 : ClangBuiltin<"__builtin_csa_cmp8x8">,
    Intrinsic<[llvm_i8_ty], [llvm_v8i8_ty, llvm_v8i8_ty, llvm_i8_ty],
      [IntrNoMem, ImmArg<2>]>;

def int_csa_cmp16x4 : ClangBuiltin<"__builtin_csa_cmp16x4">,
    Intrinsic<[llvm_i8_ty], [llvm_v4i16_ty, llvm_v4i16_ty, llvm_i8_ty],
      [IntrNoMem, ImmArg<2>]>;

def int_csa_cmpf16x4 : ClangBuiltin<"__builtin_csa_cmpf16x4">,
    Intrinsic<[llvm_i8_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_i8_ty,
                    llvm_i8_ty, llvm_i8_ty, llvm_i8_ty, llvm_i1_ty, llvm_i1_ty],
      [IntrNoMem, ImmArg<2>, ImmArg<3>, ImmArg<4>, ImmArg<5>, ImmArg<6>, ImmArg<7>]>;

def int_csa_cmpf32x2 : ClangBuiltin<"__builtin_csa_cmpf32x2">,
    Intrinsic<[llvm_i8_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_i8_ty,
                    llvm_i8_ty, llvm_i8_ty, llvm_i8_ty, llvm_i1_ty, llvm_i1_ty],
      [IntrNoMem, ImmArg<2>, ImmArg<3>, ImmArg<4>, ImmArg<5>, ImmArg<6>, ImmArg<7>]>;

def int_csa_maxf16x4 : ClangBuiltin<"__builtin_csa_maxf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_i8_ty,
                                llvm_i8_ty, llvm_i8_ty, llvm_i1_ty, llvm_i1_ty],
      [IntrNoMem, ImmArg<2>, ImmArg<3>, ImmArg<4>, ImmArg<5>, ImmArg<6>]>;

def int_csa_minf16x4 : ClangBuiltin<"__builtin_csa_minf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_i8_ty,
                                llvm_i8_ty, llvm_i8_ty, llvm_i1_ty, llvm_i1_ty],
      [IntrNoMem, ImmArg<2>, ImmArg<3>, ImmArg<4>, ImmArg<5>, ImmArg<6>]>;

def int_csa_maxf32x2 : ClangBuiltin<"__builtin_csa_maxf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_i8_ty,
                                llvm_i8_ty, llvm_i8_ty, llvm_i1_ty, llvm_i1_ty],
      [IntrNoMem, ImmArg<2>, ImmArg<3>, ImmArg<4>, ImmArg<5>, ImmArg<6>]>;

def int_csa_minf32x2 : ClangBuiltin<"__builtin_csa_minf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_i8_ty,
                                llvm_i8_ty, llvm_i8_ty, llvm_i1_ty, llvm_i1_ty],
      [IntrNoMem, ImmArg<2>, ImmArg<3>, ImmArg<4>, ImmArg<5>, ImmArg<6>]>;

// Intrinsics for specifying ordering edges attached to memory references.
def int_csa_inord  : Intrinsic<[], [llvm_i1_ty], [] /* , "llvm.csa.inord"  */>;
def int_csa_outord : Intrinsic<[llvm_i1_ty], [], [] /* , "llvm.csa.outord" */>;

// An intrinsic for representing a function entry. Function exits are
// represented by inord intrinsics on return instructions.
def int_csa_mementry :
    Intrinsic<[llvm_i1_ty], [], [] /* , "llvm.csa.mementry" */>;

// An intrinsic for representing an n-ary all0 operation, used for ordering
// token merges in memory ordering.
def int_csa_all0 :
    Intrinsic<[llvm_i1_ty], [llvm_vararg_ty],
              [IntrNoMem] /* , "llvm.csa.all0" */>;

// Data-gated prefetches.
def int_csa_gated_prefetch :
    Intrinsic<[], [llvm_any_ty, llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty],
      [IntrInaccessibleMemOrArgMemOnly, ReadOnly<1>, NoCapture<1>]>;

// LIC Queues intrinsics
def int_csa_lic_init :
    Intrinsic<[llvm_i32_ty], [llvm_i8_ty, llvm_i64_ty, llvm_i64_ty],
   []>;

def int_csa_lic_preload :
    Intrinsic<[], [llvm_i32_ty, llvm_any_ty],
    [] /* , "llvm.csa.lic.init.val" */>;

def int_csa_lic_write :
    Intrinsic<[], [llvm_i32_ty, llvm_any_ty],
    [] /* , "llvm.csa.lic.write" */>;

def int_csa_lic_read :
    Intrinsic<[llvm_any_ty], [llvm_i32_ty],
    [] /* , "llvm.csa.lic.read" */>;

// LIC Queues Lowering intrinsics
def int_csa_lower_lic_init :
    Intrinsic<[], [llvm_i32_ty, llvm_i8_ty, llvm_i64_ty, llvm_i64_ty],
    [IntrConvergent, IntrNoDuplicate]>;

def int_csa_lower_lic_preload :
    Intrinsic<[], [llvm_i32_ty, llvm_i64_ty],
    [IntrConvergent, IntrNoDuplicate]>;

def int_csa_lower_lic_write :
    Intrinsic<[], [llvm_i32_ty, llvm_any_ty],
    [IntrConvergent, IntrNoDuplicate]>;

def int_csa_lower_lic_read :
    Intrinsic<[llvm_any_ty], [llvm_i32_ty],
    [IntrConvergent, IntrNoDuplicate]>;

def int_csa_stream_load : Intrinsic<[llvm_any_ty],
    [LLVMPointerType<LLVMMatchType<0>>, llvm_i64_ty, llvm_i64_ty],
    [IntrArgMemOnly, NoCapture<0>, ReadOnly<0>, IntrConvergent], "",
    [SDNPMemOperand]>;

def int_csa_stream_load_x2 : Intrinsic<[llvm_any_ty, LLVMMatchType<0>],
    [LLVMPointerType<LLVMMatchType<0>>, llvm_i64_ty, llvm_i64_ty],
    [IntrArgMemOnly, NoCapture<0>, ReadOnly<0>, IntrConvergent], "",
    [SDNPMemOperand]>;

def int_csa_stream_store : Intrinsic<[],
    [llvm_any_ty, LLVMPointerType<LLVMMatchType<0>>, llvm_i64_ty, llvm_i64_ty],
    [IntrArgMemOnly, NoCapture<1>, WriteOnly<1>, IntrConvergent], "",
    [SDNPMemOperand]>;

// The following intrinsics support limiting pipeline depth of loops.
// llvm.csa.pipeline_limited_entry and exit are generated by
// OpenMP processing of the dataflow(pipeline<depth>)) clause.
// llvm.csa.pipeline_limited_entry
def int_csa_pipeline_limited_entry :
  ClangBuiltin<"__builtin_csa_pipeline_limited_entry">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty],
          [IntrInaccessibleMemOrArgMemOnly, IntrNoDuplicate]>;

// llvm.csa.pipeline_limited_exit
def int_csa_pipeline_limited_exit :
  ClangBuiltin<"__builtin_csa_pipeline_limited_exit">,
    Intrinsic<[], [llvm_i32_ty],
          [IntrInaccessibleMemOrArgMemOnly, IntrNoDuplicate]>;

// These are internal intrinsics not suitable for insertion by programmers.
// llvm.csa.pipeline.depth.token.take
def int_csa_pipeline_depth_token_take :
    Intrinsic<[llvm_ptr_ty], [llvm_ptr_ty, llvm_i32_ty, llvm_i32_ty],
    [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.pipeline.depth.token.return
def int_csa_pipeline_depth_token_return :
    Intrinsic<[], [llvm_ptr_ty, llvm_ptr_ty],
    [IntrInaccessibleMemOrArgMemOnly]>;
}

#endif // INTEL_FEATURE_CSA
