__kernel void multModSPOCL(__global float *ptrAlpha, __global float *ptrData,
                           __global float *ptrLevel, __global float *ptrIndex,
                           __global float *ptrResult, int fastStorageSize,
                           int storageSize, int offset,
                           __global float *ptrAlpha1, __global float *ptrBeta,
                           __global unsigned int *ptrGamma,
                           __global float *ptrDelta) {
  int globalIdx = get_global_id(0);
  globalIdx = globalIdx + offset;
  float eval, index_calc, abs, last, localSupport, curSupport;
  float myResult = 0.0f;
  // Create registers for the data
  float data_0 = ptrData[(globalIdx * 5) + 0];
  float f_0 = 1.0f;
  float data_1 = ptrData[(globalIdx * 5) + 1];
  float f_1 = 1.0f;
  float data_2 = ptrData[(globalIdx * 5) + 2];
  float f_2 = 1.0f;
  float data_3 = ptrData[(globalIdx * 5) + 3];
  float f_3 = 1.0f;
  float data_4 = ptrData[(globalIdx * 5) + 4];
  float f_4 = 1.0f;
  // Iterate over all grid points
  for (int j = 0; j < storageSize; j++) {
    f_0 = max(as_float(as_uint(ptrAlpha1[(j * 5) + 0] * data_0 +
                               ptrBeta[(j * 5) + 0]) |
                       ptrGamma[(j * 5) + 0]) +
                  ptrDelta[(j * 5) + 0],
              0.0f);
    f_1 = max(as_float(as_uint(ptrAlpha1[(j * 5) + 1] * data_1 +
                               ptrBeta[(j * 5) + 1]) |
                       ptrGamma[(j * 5) + 1]) +
                  ptrDelta[(j * 5) + 1],
              0.0f);
    f_2 = max(as_float(as_uint(ptrAlpha1[(j * 5) + 2] * data_2 +
                               ptrBeta[(j * 5) + 2]) |
                       ptrGamma[(j * 5) + 2]) +
                  ptrDelta[(j * 5) + 2],
              0.0f);
    f_3 = max(as_float(as_uint(ptrAlpha1[(j * 5) + 3] * data_3 +
                               ptrBeta[(j * 5) + 3]) |
                       ptrGamma[(j * 5) + 3]) +
                  ptrDelta[(j * 5) + 3],
              0.0f);
    f_4 = max(as_float(as_uint(ptrAlpha1[(j * 5) + 4] * data_4 +
                               ptrBeta[(j * 5) + 4]) |
                       ptrGamma[(j * 5) + 4]) +
                  ptrDelta[(j * 5) + 4],
              0.0f);
    myResult += ptrAlpha[j] * (f_0) * (f_1) * (f_2) * (f_3) * (f_4);
  }
  ptrResult[globalIdx] = myResult;
}
