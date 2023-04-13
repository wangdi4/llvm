#pragma OPENCL EXTENSION cl_khr_fp16 : enable
kernel void test(global TYPE *A, global TYPE *B, global TYPE *C, ITYPE D,
                 UTYPE E, global ITYPE *dst) {
  size_t i = get_global_id(0);
  dst[i] = isfinite(A[i]) + isinf(A[i]) + isnan(A[i]) + isnormal(A[i]) +
           isordered(A[i], B[i]) + isunordered(A[i], B[i]) + signbit(B[i]) +
           bitselect(A[i], B[i], C[i]) + select(A[i], B[i], D) +
           select(A[i], B[i], E);
#ifdef MASKED
  // Add subgroup call in order to enable masked vectorized kernel.
  dst[i] += get_sub_group_size();
#endif
}
