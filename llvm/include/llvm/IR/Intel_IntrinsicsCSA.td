#if INTEL_FEATURE_CSA
//===- Intel_IntrinsicsCSA.td - Defines csa intrinsics -----*- tablegen -*-===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
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
def int_csa_directive : GCCBuiltin<"__builtin_csa_directive">,
    Intrinsic<[], [llvm_i32_ty], [IntrArgMemOnly] /* , "llvm.csa.directive" */>;

// "llvm.csa.parallel_loop" intrinsic.  A directive, probably resulting from
// an OMP pragma, that indicates that what follows is a parallel loop. Using
// IntrArgMemOnly forces sequencing wrt other operations (though this might
// not be quite right).
def int_csa_parallel_loop : GCCBuiltin<"__builtin_csa_parallel_loop">,
    Intrinsic<[], [], [IntrArgMemOnly]>;


// llvm.csa.parallel_region_entry
def int_csa_parallel_region_entry :
        GCCBuiltin<"__builtin_csa_parallel_region_entry">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty],
                  [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.parallel_region_exit
def int_csa_parallel_region_exit :
        GCCBuiltin<"__builtin_csa_parallel_region_exit">,
        Intrinsic<[], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.parallel_section_entry
def int_csa_parallel_section_entry :
        GCCBuiltin<"__builtin_csa_parallel_section_entry">,
        Intrinsic<[llvm_i32_ty], [llvm_i32_ty],
                  [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.parallel_section_exit
def int_csa_parallel_section_exit :
        GCCBuiltin<"__builtin_csa_parallel_section_exit">,
        Intrinsic<[], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.spmdization
def int_csa_spmdization : GCCBuiltin<"__builtin_csa_spmdization">,
    Intrinsic<[], [llvm_i32_ty, llvm_ptr_ty],
              [IntrInaccessibleMemOrArgMemOnly,
               IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.spmd
def int_csa_spmd : GCCBuiltin<"__builtin_csa_spmd">,
    Intrinsic<[], [llvm_i32_ty, llvm_i32_ty],
              [IntrInaccessibleMemOrArgMemOnly,
               IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.spmd.worker.num
def int_csa_spmd_worker_num : GCCBuiltin<"__builtin_csa_spmd_worker_num">,
    Intrinsic<[llvm_i32_ty], [],
              [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.spmdization_entry
def int_csa_spmdization_entry : GCCBuiltin<"__builtin_csa_spmdization_entry">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty],
              [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.spmdization_exit
def int_csa_spmdization_exit : GCCBuiltin<"__builtin_csa_spmdization_exit">,
    Intrinsic<[], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly]>;

// llvm.csa.pipeline_loop
def int_csa_pipeline_loop : GCCBuiltin<"__builtin_csa_pipeline_loop">,
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

// SIMD intrinsics (16x4).
def int_csa_addf16x4 : GCCBuiltin<"__builtin_csa_addf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_subf16x4 : GCCBuiltin<"__builtin_csa_subf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_addsubf16x4 : GCCBuiltin<"__builtin_csa_addsubf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_subaddf16x4 : GCCBuiltin<"__builtin_csa_subaddf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_mulf16x4 : GCCBuiltin<"__builtin_csa_mulf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmaf16x4 : GCCBuiltin<"__builtin_csa_fmaf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmsf16x4 : GCCBuiltin<"__builtin_csa_fmsf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmrsf16x4 : GCCBuiltin<"__builtin_csa_fmrsf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmasf16x4 : GCCBuiltin<"__builtin_csa_fmasf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmsaf16x4 : GCCBuiltin<"__builtin_csa_fmsaf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fnmsf16x4 : GCCBuiltin<"__builtin_csa_fnmsf16x4">,
    Intrinsic<[llvm_v4f16_ty], [llvm_v4f16_ty, llvm_v4f16_ty, llvm_v4f16_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

// SIMD intrinsics (32x2).
def int_csa_addf32x2 : GCCBuiltin<"__builtin_csa_addf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_subf32x2 : GCCBuiltin<"__builtin_csa_subf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_addsubf32x2 : GCCBuiltin<"__builtin_csa_addsubf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_subaddf32x2 : GCCBuiltin<"__builtin_csa_subaddf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_mulf32x2 : GCCBuiltin<"__builtin_csa_mulf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmaf32x2 : GCCBuiltin<"__builtin_csa_fmaf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmsf32x2 : GCCBuiltin<"__builtin_csa_fmsf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmrsf32x2 : GCCBuiltin<"__builtin_csa_fmrsf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmasf32x2 : GCCBuiltin<"__builtin_csa_fmasf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

def int_csa_fmsaf32x2 : GCCBuiltin<"__builtin_csa_fmsaf32x2">,
    Intrinsic<[llvm_v2f32_ty], [llvm_v2f32_ty, llvm_v2f32_ty, llvm_v2f32_ty,
      llvm_i8_ty, llvm_i8_ty, llvm_i8_ty], [IntrNoMem]>;

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
  GCCBuiltin<"__builtin_csa_pipeline_limited_entry">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty],
          [IntrInaccessibleMemOrArgMemOnly, IntrNoDuplicate]>;

// llvm.csa.pipeline_limited_exit
def int_csa_pipeline_limited_exit :
  GCCBuiltin<"__builtin_csa_pipeline_limited_exit">,
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
