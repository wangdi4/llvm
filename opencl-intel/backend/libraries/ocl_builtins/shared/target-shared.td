include "GENERIC/opencl-builtin.td"
include "GENERIC/opencl-v1.2.td"
include "GENERIC/opencl-v2.0.td"
include "GENERIC/ll_intrinsics.td"

class Generic : TargetImpl {
include "GENERIC/code_expand_promote_common.td"
include "GENERIC/code_conversion_sat_common.td"
include "GENERIC/code_integer_common.td"
include "GENERIC/math_shared.td"
include "GENERIC/conversion_shared.td" 
include "GENERIC/conversion_sat_shared.td"
include "GENERIC/integer_shared.td"
include "GENERIC/common_shared.td"
include "GENERIC/geometric.td"
include "GENERIC/relational_shared.td"
}

class Shared : Generic {
let NativeTypes = [v16i8, v16u8, v8i16, v8u16, v4i32, v4u32, v2i64, v2u64, v4f32, v2f64];

code Prolog = [{
#include "svml_shared.h"
#define __OPENCL__
#include <intrin.h>
#include "generic-builtin-defines.h"
#include "cpu-builtin-defines.h"
}];

include "shared/math.td"
include "shared/conversion.td"
include "shared/conversion_sat.td"
include "shared/integers.td"
include "shared/common.td"
include "shared/geometric.td"
include "shared/relational.td"
}

def target : Shared {
  let Target = "shared";
}
