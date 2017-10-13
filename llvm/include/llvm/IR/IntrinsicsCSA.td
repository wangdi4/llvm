//===- IntrinsicsCSA.td - Defines csa intrinsics --*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// INTEL PROPRIETARY
//   Do not distribute without consulting with the Intel Compiler Team
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

}



