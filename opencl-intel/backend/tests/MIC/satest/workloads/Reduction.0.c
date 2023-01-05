#ifdef SINGLE_PRECISION
#define FPTYPE float
#elif K_DOUBLE_PRECISION
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define FPTYPE double
#elif AMD_DOUBLE_PRECISION
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#define FPTYPE double
#endif
#if defined(__INTEL_CPU__) || defined(__INTEL_MIC__)
#ifdef __INTEL_CPU__
#ifdef SINGLE_PRECISION
#define FPTYPE_NATIVE float8 // Assume AVX
#else
#define FPTYPE_NATIVE double4
#endif
#else
#ifdef SINGLE_PRECISION
#define FPTYPE_NATIVE float16
#else
#define FPTYPE_NATIVE double8
#endif
#endif
__kernel void reduce(__global const FPTYPE *g_idata, __global FPTYPE *g_odata,
                     __local FPTYPE *sdata, const unsigned int n) {
  const unsigned int tid = get_local_id(0);
  const unsigned int gid = get_group_id(0);
  const unsigned int nPerWG = n / get_num_groups(0);
  const unsigned int nPerWI =
      nPerWG / get_local_size(0); // * float_native_width;
  FPTYPE ldata = 0.;
  unsigned int i = nPerWG * gid + tid;
  for (unsigned int j = 0; j < nPerWI; ++j) {
    ldata += g_idata[i];
    i += get_local_size(0);
  }
  sdata[tid] = ldata;
  barrier(CLK_LOCAL_MEM_FENCE);
  // Do final reduction and write to global memory
  if (tid == 0) {
    // Left overs for the current WG
    // printf("WG leftovers from = %d\n", nPerWI*get_local_size(0));
    for (unsigned int j = nPerWI * get_local_size(0); j < nPerWG; ++j) {
      ldata += g_idata[j];
    }
    // Summarize for rest of WG's
    for (unsigned int j = 1; j < get_local_size(0); ++j) {
      ldata += sdata[j];
    }
    // left overs for whole NDRange
    if (gid == (get_num_groups(0) - 1)) {
      for (i = (gid + 1) * nPerWG; i < n; ++i) {
        ldata += g_idata[i];
      }
    }
    g_odata[gid] = ldata;
  }
}
__kernel void __attribute__((reqd_work_group_size(1, 1, 1)))
reduceNoLocal(__global FPTYPE *g_idata, __global FPTYPE *g_odata,
              unsigned int n) {
  const size_t nPerWG = n / get_num_groups(0);
  const size_t gid = get_group_id(0);
  size_t i = gid * nPerWG;
  size_t last_i = (gid + 1) * nPerWG;
  FPTYPE sum = 0.0f;
  for (; i < last_i; ++i) {
    sum += g_idata[i];
  }
  if ((gid + 1) == get_num_groups(0)) {
    while (i < n) {
      sum += g_idata[i];
      ++i;
    }
  }
  g_odata[gid] = sum;
}
#else
__kernel void reduce(__global const FPTYPE *g_idata, __global FPTYPE *g_odata,
                     __local FPTYPE *sdata, const unsigned int n) {
  const unsigned int tid = get_local_id(0);
  unsigned int i = (get_group_id(0) * (get_local_size(0) * 2)) + tid;
  const unsigned int gridSize = get_local_size(0) * 2 * get_num_groups(0);
  const unsigned int blockSize = get_local_size(0);
  sdata[tid] = 0;
  // Reduce multiple elements per thread, strided by grid size
  while (i < n) {
    sdata[tid] += g_idata[i] + g_idata[i + blockSize];
    i += gridSize;
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  // do reduction in shared mem
  for (unsigned int s = blockSize / 2; s > 0; s >>= 1) {
    if (tid < s) {
      sdata[tid] += sdata[tid + s];
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }
  // Write result back to global memory
  if (tid == 0) {
    g_odata[get_group_id(0)] = sdata[0];
  }
}
// Currently, CPUs on Snow Leopard only support a work group size of 1
// So, we have a separate version of the kernel which doesn't use
// local memory. This version is only used when the maximum
// supported local group size is 1.
__kernel void reduceNoLocal(__global FPTYPE *g_idata, __global FPTYPE *g_odata,
                            unsigned int n) {
  FPTYPE sum = 0.0f;
  for (int i = 0; i < n; i++) {
    sum += g_idata[i];
  }
  g_odata[0] = sum;
}
#endif
