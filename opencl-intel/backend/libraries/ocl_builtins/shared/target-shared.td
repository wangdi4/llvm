// Copyright (C) 2014 Intel Corporation
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

include "GENERIC/opencl-builtin.td"
include "GENERIC/opencl-v1.2.td"
include "GENERIC/opencl-v2.0.td"
include "GENERIC/opencl-v2.0WG.td"
include "GENERIC/ll_intrinsics.td"
include "GENERIC/sycl-spirv.td"
include "shared/opencl-pipes.td"
include "GENERIC/code_helpers.td"
include "GENERIC/sub_group_builtin.td"

class Generic : TargetImpl {
// include Promote1_v3_to_v4, Promote2_v3_to_v4 codes
include "ssse3/code_promote.td"
include "GENERIC/math_shared.td"
include "GENERIC/masked_math.td"
include "GENERIC/conversion_shared.td"
include "GENERIC/conversion_sat_shared.td"
include "GENERIC/integer_shared.td"
include "GENERIC/common_shared.td"
include "GENERIC/geometric.td"
include "GENERIC/relational_shared.td"
}

defset list<OclBuiltinImpl> shared_impls = {
include "shared/common_impl.td"
include "shared/conversion_impl.td"
include "shared/integer_impl.td"
include "shared/math_impl.td"
include "shared/relational_impl.td"
include "shared/spirv_impl.td"
include "shared/work_group_impl.td"
} // defset shared_impls

class Shared : Generic {
let NativeTypes = [v16i8, v16u8, v8i16, v8u16, v4i32, v4u32, v2i64, v2u64, v4f32, v2f64];

code Prolog = [{
#include "svml_shared.h"
#define __OPENCL__
#include "opencl-intrin-wrapper.h"
#include "generic-builtin-defines.h"
#include "cpu-builtin-defines.h"
#include "pipes-internal.h"
}];

include "shared/math.td"
include "shared/conversion.td"
include "shared/conversion_sat.td"
include "shared/integers.td"
include "shared/common.td"
include "shared/geometric.td"
include "shared/relational.td"
include "shared/pipes.td"

let Impls = shared_impls;
}

def target : Shared {
  let Target = "shared";
}
