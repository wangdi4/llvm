#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void multModOCL(__global double *ptrAlpha, __global double *ptrData,
                         __global double *ptrLevel, __global double *ptrIndex,
                         __global double *ptrResult, int fastStorageSize,
                         int storageSize, int offset,
                         __global double *ptrAlpha1, __global double *ptrBeta,
                         __global unsigned long *ptrGamma,
                         __global double *ptrDelta) {
  int globalIdx = get_global_id(0);
  globalIdx = globalIdx + offset;
  double eval, index_calc, abs, last, localSupport, curSupport;
  double myResult = 0.0;
  // Create registers for the data
  double data_0 = ptrData[(globalIdx * 5) + 0];
  double f_0 = 1.0;
  double data_1 = ptrData[(globalIdx * 5) + 1];
  double f_1 = 1.0;
  double data_2 = ptrData[(globalIdx * 5) + 2];
  double f_2 = 1.0;
  double data_3 = ptrData[(globalIdx * 5) + 3];
  double f_3 = 1.0;
  double data_4 = ptrData[(globalIdx * 5) + 4];
  double f_4 = 1.0;
  // Iterate over all grid points (fast ones, with cache)
  for (int j = 0; j < storageSize; j++) {
    f_0 = max(as_double(as_ulong(ptrAlpha1[(j * 5) + 0] * data_0 +
                                 ptrBeta[(j * 5) + 0]) |
                        ptrGamma[(j * 5) + 0]) +
                  ptrDelta[(j * 5) + 0],
              0.0);
    f_1 = max(as_double(as_ulong(ptrAlpha1[(j * 5) + 1] * data_1 +
                                 ptrBeta[(j * 5) + 1]) |
                        ptrGamma[(j * 5) + 1]) +
                  ptrDelta[(j * 5) + 1],
              0.0);
    f_2 = max(as_double(as_ulong(ptrAlpha1[(j * 5) + 2] * data_2 +
                                 ptrBeta[(j * 5) + 2]) |
                        ptrGamma[(j * 5) + 2]) +
                  ptrDelta[(j * 5) + 2],
              0.0);
    f_3 = max(as_double(as_ulong(ptrAlpha1[(j * 5) + 3] * data_3 +
                                 ptrBeta[(j * 5) + 3]) |
                        ptrGamma[(j * 5) + 3]) +
                  ptrDelta[(j * 5) + 3],
              0.0);
    f_4 = max(as_double(as_ulong(ptrAlpha1[(j * 5) + 4] * data_4 +
                                 ptrBeta[(j * 5) + 4]) |
                        ptrGamma[(j * 5) + 4]) +
                  ptrDelta[(j * 5) + 4],
              0.0);
    myResult += ptrAlpha[j] * (f_0) * (f_1) * (f_2) * (f_3) * (f_4);
  }
  ptrResult[globalIdx] = myResult;
}
