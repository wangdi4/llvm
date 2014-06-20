include "GENERIC/opencl-builtin.td"
include "GENERIC/opencl-v1.2.td"

class Generic : TargetImpl {
include "GENERIC/code_expand_promote_common.td"
include "GENERIC/math_shared.td"
}

class Shared : Generic {
let NativeTypes = [v16i8, v16u8, v8i16, v8u16, v4i32, v4u32, v2i64, v2u64, v4f32, v2f64];

code Prolog = [{
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#include "svml_shared.h"
#define __OPENCL__
#ifdef __APPLE__
  #include <immintrin.h>
#else
  #include <intrin.h>
#endif
#include "generic-builtin-defines.h"
}];

include "shared/math.td"
}

def target : Shared {
  let Target = "shared";
}
