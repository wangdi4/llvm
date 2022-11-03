#if !defined(cl_intel_vec_len_hint)
#error cl_intel_vec_len_hint extension is not supported
#endif

#pragma OPENCL EXTENSION cl_intel_vec_len_hint : enable

#define KERNEL(length)                                                         \
  __attribute__((intel_vec_len_hint(length)))                                  \
  __kernel void kernel##length(__global int *in, __global int *out) {          \
    size_t gid = get_global_id(0);                                             \
    out[gid] = in[gid];                                                        \
  }

KERNEL(1)
KERNEL(4)
KERNEL(8)
KERNEL(16)

#define KERNEL_TYPE_HINT(length, type_hint)                                    \
  __attribute__((vec_type_hint(type_hint)))                                    \
  __attribute__((intel_vec_len_hint(length)))                                  \
  __kernel void kernel##length##_vec_hint_##type_hint(__global int *in,        \
                                                      __global int *out) {     \
    size_t gid = get_global_id(0);                                             \
    out[gid] = in[gid];                                                        \
  }

KERNEL_TYPE_HINT(8, float8)
KERNEL_TYPE_HINT(4, int)
KERNEL_TYPE_HINT(1, float)
