// Copyright (C) 2013 Intel Corporation
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

// NOTE
// Purporse of the file GENERIC_CPU.td is to use during CPU built ins transition to tblgen
// When transition is done it will be the same as GENERIC.td
// In this file Gradually we enable "include" lines for categories of BuiltIns including math, common, converions, etc
// Other parts of file should be the same as GENERIC.td. Otherwise we can get build error


include "GENERIC/opencl-builtin.td"
include "GENERIC/opencl-v1.2.td"
include "GENERIC/opencl-v2.0.td"
include "GENERIC/opencl-v2.0WG.td"
include "GENERIC/sub_group_builtin.td"
include "GENERIC/ll_intrinsics.td"
include "GENERIC/helper_function.td"
include "GENERIC/sycl-spirv.td"
include "GENERIC/code_helpers.td"
include "GENERIC/intel-matrix_layout_transform.td" // INTEL

class Generic : TargetImpl {
include "GENERIC/GENERIC_common.td"
include "GENERIC/math_cpu.td"
}
