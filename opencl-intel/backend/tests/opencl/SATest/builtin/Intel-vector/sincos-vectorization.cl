kernel void test(global float *A, global double *B, global float *dst,
                 global double *dst2) {
  size_t i = get_global_id(0);
  dst[i] = sin(A[i]) + cos(A[i]);
  dst2[i] = sin(B[i]) + cos(B[i]);
}
