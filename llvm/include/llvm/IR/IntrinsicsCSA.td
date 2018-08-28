//===- IntrinsicsCSA.td - Defines csa intrinsics --*- tablegen -*-===//
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
    Intrinsic<[], [], [IntrArgMemOnly] /* , "llvm.csa.parallel_loop" */>;


def int_csa_parallel_region_entry : GCCBuiltin<"__builtin_csa_parallel_region_entry">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly] /* , "llvm.csa.parallel_region_entry" */>;

def int_csa_parallel_region_exit : GCCBuiltin<"__builtin_csa_parallel_region_exit">,
    Intrinsic<[], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly] /* , "llvm.csa.parallel_region_exit" */>;

def int_csa_parallel_section_entry : GCCBuiltin<"__builtin_csa_parallel_section_entry">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly] /* , "llvm.csa.parallel_section_entry" */>;

def int_csa_parallel_section_exit : GCCBuiltin<"__builtin_csa_parallel_section_exit">,
    Intrinsic<[], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly] /* , "llvm.csa.parallel_section_exit" */>;

def int_csa_spmdization : GCCBuiltin<"__builtin_csa_spmdization">,
    Intrinsic<[], [llvm_i32_ty, llvm_ptr_ty], [IntrInaccessibleMemOrArgMemOnly, IntrInaccessibleMemOrArgMemOnly] /* , "llvm.csa.spmdization" */>;

def int_csa_spmd : GCCBuiltin<"__builtin_csa_spmd">,
    Intrinsic<[], [llvm_i32_ty, llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly, IntrInaccessibleMemOrArgMemOnly] /* , "llvm.csa.spmd" */>;

def int_csa_spmdization_entry : GCCBuiltin<"__builtin_csa_spmdization_entry">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly] /* , "llvm.csa.spmdization_entry" */>;

def int_csa_spmdization_exit : GCCBuiltin<"__builtin_csa_spmdization_exit">,
    Intrinsic<[], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly] /* , "llvm.csa.spmdization_exit" */>;

def int_csa_pipeline_loop : GCCBuiltin<"__builtin_csa_pipeline_loop">,
    Intrinsic<[], [llvm_i32_ty], [IntrArgMemOnly] /* , "llvm.csa.pipeline_loop" */>;

// These are internal intrinsics not suitable for insertion by programmers.
def int_csa_pipeline_loop_entry :
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly] /* , "llvm.csa.pipeline_loop_entry" */>;

def int_csa_pipeline_loop_exit :
    Intrinsic<[], [llvm_i32_ty], [IntrInaccessibleMemOrArgMemOnly] /* , "llvm.csa.pipeline_loop_exit" */>;

def int_csa_pipelineable_loop_marker :
    Intrinsic<[], [llvm_i64_ty], [IntrInaccessibleMemOrArgMemOnly]
    /* , "llvm.csa.pipelineable_loop" */>;

}



