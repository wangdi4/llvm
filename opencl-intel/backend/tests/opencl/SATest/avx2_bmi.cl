#define VECTOR_SIZE 32

#define SINGLE_PRECISION
#ifdef SINGLE_PRECISION
#define FPTYPE float
#elif K_DOUBLE_PRECISION
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define FPTYPE double
#elif AMD_DOUBLE_PRECISION
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#define FPTYPE double
#endif

__kernel void spmv_csr_scalar_kernel(__global const FPTYPE *restrict val,
                                     __global const FPTYPE *restrict vec,
                                     __global const int *restrict cols,
                                     __global const int *restrict rowDelimiters,
                                     const int dim,
                                     __global FPTYPE *restrict out) {
  int myRow = get_global_id(0);

  if (myRow < dim) {
    FPTYPE t = 0;
    int start = rowDelimiters[myRow];
    int end = rowDelimiters[myRow + 1];
    for (int j = start; j < end; j++) {
      int col = cols[j];
      t += val[j] * vec[col] + __builtin_elementwise_max(j - myRow, 0);
    }
    out[myRow] = t;
  }
}
