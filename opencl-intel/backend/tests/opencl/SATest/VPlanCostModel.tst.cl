#pragma OPENCL EXTENSION cl_intel_vec_len_hint: enable
#pragma OPENCL EXTENSION cl_intel_subgroups: enable

// The following kernel should have higher cost when vectorized, so that
// VectorKernelDiscard will discard it without any attributes or subgroups. If
// not, modify it to ensure the vectorized kernel is more costly.
#define KERNEL(NAME) \
kernel void NAME(global long *dst, global long *dst2, global long *src) { \
  size_t id = get_global_id(0); \
  const int c = 64; \
  for (int i = 0; i < c; i++) { \
    dst[id*c+i] = src[id*c+i]; \
    dst2[id*c+i] = src[id*c+i]; \
  } \
  EXTRA_CODE \
}

#define EXTRA_CODE

// The vectorized kernel should be discarded as it has higher cost
KERNEL(test_discard_vec_kernel)

// The vectorized kernel should be kept as vec_len_hint is specified
__attribute__((intel_vec_len_hint(4)))
KERNEL(test_vec_len_hint)

// The vectorized kernel should be kept as reqd_sub_group_size is specified
__attribute__((intel_reqd_sub_group_size(4)))
KERNEL(test_redq_sub_group_size)

#undef EXTRA_CODE

#define EXTRA_CODE volatile int i = get_sub_group_size();
// The vectorized kernel should be kept as it contains subgroups calls
KERNEL(test_subgroups)
#undef EXTRA_CODE

// The vectorized kernel should be kept as it has lower cost
kernel void test_keep_vec_kernel(global long *dst) {
  dst[get_global_id(0)] = 42;
}
