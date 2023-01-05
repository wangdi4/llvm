#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void multOCL(__global double *ptrAlpha, __global double *ptrData,
                      __global double *ptrLevel, __global double *ptrIndex,
                      __global double *ptrResult, uint fastStorageSize,
                      uint storageSize, uint offset) {
  int globalIdx = get_global_id(0);
  int localIdx = get_local_id(0);
  globalIdx = globalIdx + offset;
  __local double locLevel[320];
  __local double locIndex[320];
  __local double locAlpha[64];
  double eval, index_calc, abs, last, localSupport, curSupport;
  double myResult = 0.0;
  // Create registers for the data
  double data_0 = ptrData[(globalIdx * 5) + 0];
  double data_1 = ptrData[(globalIdx * 5) + 1];
  double data_2 = ptrData[(globalIdx * 5) + 2];
  double data_3 = ptrData[(globalIdx * 5) + 3];
  double data_4 = ptrData[(globalIdx * 5) + 4];
  // Iterate over all grid points (fast ones, with cache)
  for (int j = 0; j < fastStorageSize; j += 64) {
    locLevel[(localIdx * 5) + 0] = ptrLevel[((j + localIdx) * 5) + 0];
    locIndex[(localIdx * 5) + 0] = ptrIndex[((j + localIdx) * 5) + 0];
    locLevel[(localIdx * 5) + 1] = ptrLevel[((j + localIdx) * 5) + 1];
    locIndex[(localIdx * 5) + 1] = ptrIndex[((j + localIdx) * 5) + 1];
    locLevel[(localIdx * 5) + 2] = ptrLevel[((j + localIdx) * 5) + 2];
    locIndex[(localIdx * 5) + 2] = ptrIndex[((j + localIdx) * 5) + 2];
    locLevel[(localIdx * 5) + 3] = ptrLevel[((j + localIdx) * 5) + 3];
    locIndex[(localIdx * 5) + 3] = ptrIndex[((j + localIdx) * 5) + 3];
    locLevel[(localIdx * 5) + 4] = ptrLevel[((j + localIdx) * 5) + 4];
    locIndex[(localIdx * 5) + 4] = ptrIndex[((j + localIdx) * 5) + 4];
    locAlpha[localIdx] = ptrAlpha[j + localIdx];
    barrier(CLK_LOCAL_MEM_FENCE);
    for (int k = 0; k < 64; k++) {
      curSupport = locAlpha[k];
      eval = ((locLevel[(k * 5) + 0]) * (data_0));
      index_calc = eval - (locIndex[(k * 5) + 0]);
      abs = fabs(index_calc);
      last = 1.0 - abs;
      localSupport = fmax(last, 0.0);
      curSupport *= localSupport;
      eval = ((locLevel[(k * 5) + 1]) * (data_1));
      index_calc = eval - (locIndex[(k * 5) + 1]);
      abs = fabs(index_calc);
      last = 1.0 - abs;
      localSupport = fmax(last, 0.0);
      curSupport *= localSupport;
      eval = ((locLevel[(k * 5) + 2]) * (data_2));
      index_calc = eval - (locIndex[(k * 5) + 2]);
      abs = fabs(index_calc);
      last = 1.0 - abs;
      localSupport = fmax(last, 0.0);
      curSupport *= localSupport;
      eval = ((locLevel[(k * 5) + 3]) * (data_3));
      index_calc = eval - (locIndex[(k * 5) + 3]);
      abs = fabs(index_calc);
      last = 1.0 - abs;
      localSupport = fmax(last, 0.0);
      curSupport *= localSupport;
      eval = ((locLevel[(k * 5) + 4]) * (data_4));
      index_calc = eval - (locIndex[(k * 5) + 4]);
      abs = fabs(index_calc);
      last = 1.0 - abs;
      localSupport = fmax(last, 0.0);
      curSupport *= localSupport;
      myResult += curSupport;
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }
  // Iterate over all grid points (slow ones, without cache)
  for (int m = fastStorageSize; m < storageSize; m++) {
    curSupport = ptrAlpha[m];
    eval = ((ptrLevel[(m * 5) + 0]) * (data_0));
    index_calc = eval - (ptrIndex[(m * 5) + 0]);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    eval = ((ptrLevel[(m * 5) + 1]) * (data_1));
    index_calc = eval - (ptrIndex[(m * 5) + 1]);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    eval = ((ptrLevel[(m * 5) + 2]) * (data_2));
    index_calc = eval - (ptrIndex[(m * 5) + 2]);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    eval = ((ptrLevel[(m * 5) + 3]) * (data_3));
    index_calc = eval - (ptrIndex[(m * 5) + 3]);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    eval = ((ptrLevel[(m * 5) + 4]) * (data_4));
    index_calc = eval - (ptrIndex[(m * 5) + 4]);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    myResult += curSupport;
  }
  ptrResult[globalIdx] = myResult;
}
