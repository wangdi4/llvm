#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void multOCL(__global double *ptrAlpha, __global double *ptrData,
                      __global double *ptrLevel, __global double *ptrIndex,
                      __global double *ptrResult, int fastStorageSize,
                      int storageSize, int offset) {
  int globalIdx = get_global_id(0);
  globalIdx = globalIdx + offset;
  double eval, index_calc, abs, last, localSupport, curSupport;
  double myResult = 0.0;
  // Create registers for the data
  double data_0 = ptrData[(globalIdx * 5) + 0];
  double data_1 = ptrData[(globalIdx * 5) + 1];
  double data_2 = ptrData[(globalIdx * 5) + 2];
  double data_3 = ptrData[(globalIdx * 5) + 3];
  double data_4 = ptrData[(globalIdx * 5) + 4];
  // Iterate over all grid points (fast ones, with cache)
  for (int j = 0; j < storageSize; j++) {
    curSupport = ptrAlpha[j];
    eval = ((ptrLevel[(j * 5) + 0]) * (data_0));
    index_calc = eval - (ptrIndex[(j * 5) + 0]);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    eval = ((ptrLevel[(j * 5) + 1]) * (data_1));
    index_calc = eval - (ptrIndex[(j * 5) + 1]);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    eval = ((ptrLevel[(j * 5) + 2]) * (data_2));
    index_calc = eval - (ptrIndex[(j * 5) + 2]);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    eval = ((ptrLevel[(j * 5) + 3]) * (data_3));
    index_calc = eval - (ptrIndex[(j * 5) + 3]);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    eval = ((ptrLevel[(j * 5) + 4]) * (data_4));
    index_calc = eval - (ptrIndex[(j * 5) + 4]);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    myResult += curSupport;
  }
  ptrResult[globalIdx] = myResult;
}
