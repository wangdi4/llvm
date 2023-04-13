kernel void test(global TYPE *A, global TYPE *B, global TYPE *C,
                 global TYPE *dst) {
  size_t i = get_global_id(0);
  dst[i] = cos(A[i]) + exp(A[i]) + exp2(A[i]) + exp10(A[i]) + log(A[i]) +
           log2(A[i]) + pow(A[i], B[i]) + sin(B[i]) + tan(A[i]) + cos(C[i]) +
           sin(C[i]) // sincos
      ;

#ifdef MASKED
  // Add subgroup call in order to enable masked vectorized kernel.
  dst[i] += get_sub_group_size();
#endif
}
