// vim:ts=2:sw=2:et:

include "GENERIC/GENERIC_CPU.td"
//Memory Fence Functions
def mem_fence : OclBuiltin<"mem_fence", (outs), (ins cl_mem_fence_flags:$flags), [v1i64], []>;
def read_mem_fence : OclBuiltin<"read_mem_fence", (outs), (ins cl_mem_fence_flags:$flags), [v1i64], []>;
def write_mem_fence : OclBuiltin<"write_mem_fence", (outs), (ins cl_mem_fence_flags:$flags), [v1i64], []>;

class SSSE3 : Generic {
// Native types for SSSE3. Used in "Promote" calls
let NativeTypes = [v16i8, v16u8, v8i16, v8u16, v4i32, v4u32, v2i64, v2u64, v4f32, v2f64];

code Prolog = [{
    #define ALIGN16 __attribute__((aligned(16)))
    #define ALIGN32 __attribute__((aligned(32)))

    #pragma OPENCL EXTENSION cl_khr_fp64 : enable
    #pragma OPENCL EXTENSION cl_khr_fp16 : enable
    #ifdef __APPLE__
      #include <immintrin.h>
    #else
      #include <intrin.h>
      #define intel_ocl_bicc svmlcc
    #endif
    #include "generic-builtin-defines.h"
    #include "cpu-builtin-defines.h"
    #ifdef __x86_64__
      #include "svml_ia32e.h"
    #else
      #include "svml_ia32.h"
    #endif
}];

include "ssse3/common.td"
include "ssse3/math.td"
include "ssse3/integers.td"
include "ssse3/geometric.td"
include "ssse3/atomic.td"
include "ssse3/misc.td"
include "ssse3/conversion.td"
include "ssse3/conversion_sat.td"
include "ssse3/vloadstore.td"
include "ssse3/relational.td"
}
