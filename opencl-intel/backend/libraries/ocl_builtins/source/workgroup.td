// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

// Finalize synchronization helper for uniform workgroup functions.
// A workgroup function is uniform if it returns same value for all work items.
// e.g. work_group_broadcast, work_group_reduce_*
//
// The finalize helper is inserted after barrier sync point by GroupBuiltin
// pass, to make sure the result is used after barrier synchronization.
//
// Prototypes
let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64,
                                       v1f16, v1f32, v1f64], [1, 4, 8, 16, 32, 64]>.Tout,
    Attrs = [OVERLOADABLE] in {
  // identity
  // This finalize helper is a special function, which returns its arguments as is, 
  // and is used for work_group_broadcast and all scalar work_group functions
  def work_group_identity_finalize_helper : OclBuiltin<"__finalize_work_group_identity", (outs gentype:$ret), (ins gentype:$accum)>;
}

// Implementations
def : OclBuiltinImpl<work_group_identity_finalize_helper, work_group_identity_finalize_helper.Types, /*isDeclOnly*/false, ret_arg0_code>, SHARED;

// VectInfos (not needed for finalize helpers)
