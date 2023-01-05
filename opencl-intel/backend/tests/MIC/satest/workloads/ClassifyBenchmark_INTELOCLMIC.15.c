#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void multModOCL(__global double *ptrAlpha, __global double *ptrData,
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
      if ((locLevel[(k * 5) + 0]) == 2.0) {
        curSupport *= 1.0;
      } else if ((locIndex[(k * 5) + 0]) == 1.0) {
        curSupport *= max(2.0 - ((locLevel[(k * 5) + 0]) * (data_0)), 0.0);
      } else if ((locIndex[(k * 5) + 0]) == ((locLevel[(k * 5) + 0]) - 1.0)) {
        curSupport *= max(((locLevel[(k * 5) + 0]) * (data_0)) -
                              (locIndex[(k * 5) + 0]) + 1.0,
                          0.0);
      } else {
        curSupport *= max(1.0 - fabs(((locLevel[(k * 5) + 0]) * (data_0)) -
                                     (locIndex[(k * 5) + 0])),
                          0.0);
      }
      if ((locLevel[(k * 5) + 1]) == 2.0) {
        curSupport *= 1.0;
      } else if ((locIndex[(k * 5) + 1]) == 1.0) {
        curSupport *= max(2.0 - ((locLevel[(k * 5) + 1]) * (data_1)), 0.0);
      } else if ((locIndex[(k * 5) + 1]) == ((locLevel[(k * 5) + 1]) - 1.0)) {
        curSupport *= max(((locLevel[(k * 5) + 1]) * (data_1)) -
                              (locIndex[(k * 5) + 1]) + 1.0,
                          0.0);
      } else {
        curSupport *= max(1.0 - fabs(((locLevel[(k * 5) + 1]) * (data_1)) -
                                     (locIndex[(k * 5) + 1])),
                          0.0);
      }
      if ((locLevel[(k * 5) + 2]) == 2.0) {
        curSupport *= 1.0;
      } else if ((locIndex[(k * 5) + 2]) == 1.0) {
        curSupport *= max(2.0 - ((locLevel[(k * 5) + 2]) * (data_2)), 0.0);
      } else if ((locIndex[(k * 5) + 2]) == ((locLevel[(k * 5) + 2]) - 1.0)) {
        curSupport *= max(((locLevel[(k * 5) + 2]) * (data_2)) -
                              (locIndex[(k * 5) + 2]) + 1.0,
                          0.0);
      } else {
        curSupport *= max(1.0 - fabs(((locLevel[(k * 5) + 2]) * (data_2)) -
                                     (locIndex[(k * 5) + 2])),
                          0.0);
      }
      if ((locLevel[(k * 5) + 3]) == 2.0) {
        curSupport *= 1.0;
      } else if ((locIndex[(k * 5) + 3]) == 1.0) {
        curSupport *= max(2.0 - ((locLevel[(k * 5) + 3]) * (data_3)), 0.0);
      } else if ((locIndex[(k * 5) + 3]) == ((locLevel[(k * 5) + 3]) - 1.0)) {
        curSupport *= max(((locLevel[(k * 5) + 3]) * (data_3)) -
                              (locIndex[(k * 5) + 3]) + 1.0,
                          0.0);
      } else {
        curSupport *= max(1.0 - fabs(((locLevel[(k * 5) + 3]) * (data_3)) -
                                     (locIndex[(k * 5) + 3])),
                          0.0);
      }
      if ((locLevel[(k * 5) + 4]) == 2.0) {
        curSupport *= 1.0;
      } else if ((locIndex[(k * 5) + 4]) == 1.0) {
        curSupport *= max(2.0 - ((locLevel[(k * 5) + 4]) * (data_4)), 0.0);
      } else if ((locIndex[(k * 5) + 4]) == ((locLevel[(k * 5) + 4]) - 1.0)) {
        curSupport *= max(((locLevel[(k * 5) + 4]) * (data_4)) -
                              (locIndex[(k * 5) + 4]) + 1.0,
                          0.0);
      } else {
        curSupport *= max(1.0 - fabs(((locLevel[(k * 5) + 4]) * (data_4)) -
                                     (locIndex[(k * 5) + 4])),
                          0.0);
      }
      myResult += curSupport;
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }
  // Iterate over all grid points (slow ones, without cache)
  for (int m = fastStorageSize; m < storageSize; m++) {
    curSupport = ptrAlpha[m];
    if ((ptrLevel[(m * 5) + 0]) == 2.0) {
      curSupport *= 1.0;
    } else if ((ptrIndex[(m * 5) + 0]) == 1.0) {
      curSupport *= max(2.0 - ((ptrLevel[(m * 5) + 0]) * (data_0)), 0.0);
    } else if ((ptrIndex[(m * 5) + 0]) == ((ptrLevel[(m * 5) + 0]) - 1.0)) {
      curSupport *= max(((ptrLevel[(m * 5) + 0]) * (data_0)) -
                            (ptrIndex[(m * 5) + 0]) + 1.0,
                        0.0);
    } else {
      curSupport *= max(1.0 - fabs(((ptrLevel[(m * 5) + 0]) * (data_0)) -
                                   (ptrIndex[(m * 5) + 0])),
                        0.0);
    }
    if ((ptrLevel[(m * 5) + 1]) == 2.0) {
      curSupport *= 1.0;
    } else if ((ptrIndex[(m * 5) + 1]) == 1.0) {
      curSupport *= max(2.0 - ((ptrLevel[(m * 5) + 1]) * (data_1)), 0.0);
    } else if ((ptrIndex[(m * 5) + 1]) == ((ptrLevel[(m * 5) + 1]) - 1.0)) {
      curSupport *= max(((ptrLevel[(m * 5) + 1]) * (data_1)) -
                            (ptrIndex[(m * 5) + 1]) + 1.0,
                        0.0);
    } else {
      curSupport *= max(1.0 - fabs(((ptrLevel[(m * 5) + 1]) * (data_1)) -
                                   (ptrIndex[(m * 5) + 1])),
                        0.0);
    }
    if ((ptrLevel[(m * 5) + 2]) == 2.0) {
      curSupport *= 1.0;
    } else if ((ptrIndex[(m * 5) + 2]) == 1.0) {
      curSupport *= max(2.0 - ((ptrLevel[(m * 5) + 2]) * (data_2)), 0.0);
    } else if ((ptrIndex[(m * 5) + 2]) == ((ptrLevel[(m * 5) + 2]) - 1.0)) {
      curSupport *= max(((ptrLevel[(m * 5) + 2]) * (data_2)) -
                            (ptrIndex[(m * 5) + 2]) + 1.0,
                        0.0);
    } else {
      curSupport *= max(1.0 - fabs(((ptrLevel[(m * 5) + 2]) * (data_2)) -
                                   (ptrIndex[(m * 5) + 2])),
                        0.0);
    }
    if ((ptrLevel[(m * 5) + 3]) == 2.0) {
      curSupport *= 1.0;
    } else if ((ptrIndex[(m * 5) + 3]) == 1.0) {
      curSupport *= max(2.0 - ((ptrLevel[(m * 5) + 3]) * (data_3)), 0.0);
    } else if ((ptrIndex[(m * 5) + 3]) == ((ptrLevel[(m * 5) + 3]) - 1.0)) {
      curSupport *= max(((ptrLevel[(m * 5) + 3]) * (data_3)) -
                            (ptrIndex[(m * 5) + 3]) + 1.0,
                        0.0);
    } else {
      curSupport *= max(1.0 - fabs(((ptrLevel[(m * 5) + 3]) * (data_3)) -
                                   (ptrIndex[(m * 5) + 3])),
                        0.0);
    }
    if ((ptrLevel[(m * 5) + 4]) == 2.0) {
      curSupport *= 1.0;
    } else if ((ptrIndex[(m * 5) + 4]) == 1.0) {
      curSupport *= max(2.0 - ((ptrLevel[(m * 5) + 4]) * (data_4)), 0.0);
    } else if ((ptrIndex[(m * 5) + 4]) == ((ptrLevel[(m * 5) + 4]) - 1.0)) {
      curSupport *= max(((ptrLevel[(m * 5) + 4]) * (data_4)) -
                            (ptrIndex[(m * 5) + 4]) + 1.0,
                        0.0);
    } else {
      curSupport *= max(1.0 - fabs(((ptrLevel[(m * 5) + 4]) * (data_4)) -
                                   (ptrIndex[(m * 5) + 4])),
                        0.0);
    }
    myResult += curSupport;
  }
  ptrResult[globalIdx] = myResult;
}
