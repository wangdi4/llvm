#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void
multTransModOCL(__global double *ptrSource, __global double *ptrData,
                __global double *ptrLevel, __global double *ptrIndex,
                __global double *ptrResult, uint sourceSize, uint offset) {
  int globalIdx = get_global_id(0);
  int localIdx = get_local_id(0);
  globalIdx = globalIdx + offset;
  double eval, index_calc, abs, last, localSupport, curSupport;
  double myResult = 0.0f;
  __local double locData[320];
  __local double locSource[64];
  double level_0 = ptrLevel[(globalIdx * 5) + 0];
  double index_0 = ptrIndex[(globalIdx * 5) + 0];
  double level_1 = ptrLevel[(globalIdx * 5) + 1];
  double index_1 = ptrIndex[(globalIdx * 5) + 1];
  double level_2 = ptrLevel[(globalIdx * 5) + 2];
  double index_2 = ptrIndex[(globalIdx * 5) + 2];
  double level_3 = ptrLevel[(globalIdx * 5) + 3];
  double index_3 = ptrIndex[(globalIdx * 5) + 3];
  double level_4 = ptrLevel[(globalIdx * 5) + 4];
  double index_4 = ptrIndex[(globalIdx * 5) + 4];
  // Iterate over all grid points
  for (int i = 0; i < sourceSize; i += 64) {
    locData[(localIdx * 5) + 0] = ptrData[((i + localIdx) * 5) + 0];
    locData[(localIdx * 5) + 1] = ptrData[((i + localIdx) * 5) + 1];
    locData[(localIdx * 5) + 2] = ptrData[((i + localIdx) * 5) + 2];
    locData[(localIdx * 5) + 3] = ptrData[((i + localIdx) * 5) + 3];
    locData[(localIdx * 5) + 4] = ptrData[((i + localIdx) * 5) + 4];
    locSource[localIdx] = ptrSource[i + localIdx];
    barrier(CLK_LOCAL_MEM_FENCE);
    for (int k = 0; k < 64; k++) {
      curSupport = locSource[k];
      if ((level_0) == 2.0) {
        curSupport *= 1.0;
      } else if ((index_0) == 1.0) {
        curSupport *= max(2.0 - ((level_0) * (locData[(k * 5) + 0])), 0.0);
      } else if ((index_0) == ((level_0)-1.0)) {
        curSupport *=
            max(((level_0) * (locData[(k * 5) + 0])) - (index_0) + 1.0, 0.0);
      } else {
        curSupport *= max(
            1.0 - fabs(((level_0) * (locData[(k * 5) + 0])) - (index_0)), 0.0);
      }
      if ((level_1) == 2.0) {
        curSupport *= 1.0;
      } else if ((index_1) == 1.0) {
        curSupport *= max(2.0 - ((level_1) * (locData[(k * 5) + 1])), 0.0);
      } else if ((index_1) == ((level_1)-1.0)) {
        curSupport *=
            max(((level_1) * (locData[(k * 5) + 1])) - (index_1) + 1.0, 0.0);
      } else {
        curSupport *= max(
            1.0 - fabs(((level_1) * (locData[(k * 5) + 1])) - (index_1)), 0.0);
      }
      if ((level_2) == 2.0) {
        curSupport *= 1.0;
      } else if ((index_2) == 1.0) {
        curSupport *= max(2.0 - ((level_2) * (locData[(k * 5) + 2])), 0.0);
      } else if ((index_2) == ((level_2)-1.0)) {
        curSupport *=
            max(((level_2) * (locData[(k * 5) + 2])) - (index_2) + 1.0, 0.0);
      } else {
        curSupport *= max(
            1.0 - fabs(((level_2) * (locData[(k * 5) + 2])) - (index_2)), 0.0);
      }
      if ((level_3) == 2.0) {
        curSupport *= 1.0;
      } else if ((index_3) == 1.0) {
        curSupport *= max(2.0 - ((level_3) * (locData[(k * 5) + 3])), 0.0);
      } else if ((index_3) == ((level_3)-1.0)) {
        curSupport *=
            max(((level_3) * (locData[(k * 5) + 3])) - (index_3) + 1.0, 0.0);
      } else {
        curSupport *= max(
            1.0 - fabs(((level_3) * (locData[(k * 5) + 3])) - (index_3)), 0.0);
      }
      if ((level_4) == 2.0) {
        curSupport *= 1.0;
      } else if ((index_4) == 1.0) {
        curSupport *= max(2.0 - ((level_4) * (locData[(k * 5) + 4])), 0.0);
      } else if ((index_4) == ((level_4)-1.0)) {
        curSupport *=
            max(((level_4) * (locData[(k * 5) + 4])) - (index_4) + 1.0, 0.0);
      } else {
        curSupport *= max(
            1.0 - fabs(((level_4) * (locData[(k * 5) + 4])) - (index_4)), 0.0);
      }
      myResult += curSupport;
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }
  ptrResult[globalIdx] = myResult;
}
