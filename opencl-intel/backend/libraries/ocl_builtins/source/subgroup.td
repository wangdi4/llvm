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

include "source/core/targets.td"
include "GENERIC/opencl-builtin.td"

// Prototype
def get_sub_group_eq_mask : OclBuiltin<"get_sub_group_eq_mask", (outs v4u32:$ret), (ins), [v1i32]>;

// Implementation
def : OclBuiltinImpl<get_sub_group_eq_mask, get_sub_group_eq_mask.Types, /*isDeclOnly*/false, [{
  return 0;
}]>, SSE;

def : OclBuiltinImpl<get_sub_group_eq_mask, get_sub_group_eq_mask.Types, /*isDeclOnly*/false, [{
  return 1;
}]>, AVX512;
