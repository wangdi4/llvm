// vim:ts=2:sw=2:et:

include "GENERIC_CPU.td"

class SSE42 : Generic {
// Native types for SSE42. Used in "Promote" calls
list<OclType> NativeTypes = [v16i8, v16u8, v8i16, v8u16, v4i32, v4u32, v2i64, v2u64, v4f32, v2f64];

code Prolog = [{
    #define ALIGN16 __attribute__((aligned(16)))
    #define ALIGN32 __attribute__((aligned(32)))

    #pragma OPENCL EXTENSION cl_khr_fp64 : enable
    #include <intrin.h>
    #include "generic-builtin-defines.h"
    #include "cpu-builtin-defines.h"
    #ifdef __APPLE__
    #define svmlcc intel_ocl_bicc
    #endif
    #ifdef __x86_64__
      #include "svml_ia32e.h"
    #else
      #include "svml_ia32.h"
    #endif
}];

include "sse42/math.td"
include "sse42/common.td"
include "sse42/geometric.td"
include "sse42/integers.td"
}
