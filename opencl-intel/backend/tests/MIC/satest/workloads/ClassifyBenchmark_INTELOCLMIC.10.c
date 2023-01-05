__kernel void
multTransModSPOCL(__global float *ptrSource, __global float *ptrData,
                  __global float *ptrLevel, __global float *ptrIndex,
                  __global float *ptrResult, uint sourceSize, uint offset) {
  int globalIdx = get_global_id(0);
  int localIdx = get_local_id(0);
  globalIdx = globalIdx + offset;
  float eval, index_calc, abs, last, localSupport, curSupport;
  float myResult = 0.0f;
  __local float locData[320];
  __local float locSource[64];
  float level_0 = ptrLevel[(globalIdx * 5) + 0];
  float index_0 = ptrIndex[(globalIdx * 5) + 0];
  float level_1 = ptrLevel[(globalIdx * 5) + 1];
  float index_1 = ptrIndex[(globalIdx * 5) + 1];
  float level_2 = ptrLevel[(globalIdx * 5) + 2];
  float index_2 = ptrIndex[(globalIdx * 5) + 2];
  float level_3 = ptrLevel[(globalIdx * 5) + 3];
  float index_3 = ptrIndex[(globalIdx * 5) + 3];
  float level_4 = ptrLevel[(globalIdx * 5) + 4];
  float index_4 = ptrIndex[(globalIdx * 5) + 4];
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
      if ((level_0) == 2.0f) {
        curSupport *= 1.0f;
      } else if ((index_0) == 1.0f) {
        curSupport *= max(2.0f - ((level_0) * (locData[(k * 5) + 0])), 0.0f);
      } else if ((index_0) == ((level_0)-1.0f)) {
        curSupport *=
            max(((level_0) * (locData[(k * 5) + 0])) - (index_0) + 1.0f, 0.0f);
      } else {
        curSupport *=
            max(1.0f - fabs(((level_0) * (locData[(k * 5) + 0])) - (index_0)),
                0.0f);
      }
      if ((level_1) == 2.0f) {
        curSupport *= 1.0f;
      } else if ((index_1) == 1.0f) {
        curSupport *= max(2.0f - ((level_1) * (locData[(k * 5) + 1])), 0.0f);
      } else if ((index_1) == ((level_1)-1.0f)) {
        curSupport *=
            max(((level_1) * (locData[(k * 5) + 1])) - (index_1) + 1.0f, 0.0f);
      } else {
        curSupport *=
            max(1.0f - fabs(((level_1) * (locData[(k * 5) + 1])) - (index_1)),
                0.0f);
      }
      if ((level_2) == 2.0f) {
        curSupport *= 1.0f;
      } else if ((index_2) == 1.0f) {
        curSupport *= max(2.0f - ((level_2) * (locData[(k * 5) + 2])), 0.0f);
      } else if ((index_2) == ((level_2)-1.0f)) {
        curSupport *=
            max(((level_2) * (locData[(k * 5) + 2])) - (index_2) + 1.0f, 0.0f);
      } else {
        curSupport *=
            max(1.0f - fabs(((level_2) * (locData[(k * 5) + 2])) - (index_2)),
                0.0f);
      }
      if ((level_3) == 2.0f) {
        curSupport *= 1.0f;
      } else if ((index_3) == 1.0f) {
        curSupport *= max(2.0f - ((level_3) * (locData[(k * 5) + 3])), 0.0f);
      } else if ((index_3) == ((level_3)-1.0f)) {
        curSupport *=
            max(((level_3) * (locData[(k * 5) + 3])) - (index_3) + 1.0f, 0.0f);
      } else {
        curSupport *=
            max(1.0f - fabs(((level_3) * (locData[(k * 5) + 3])) - (index_3)),
                0.0f);
      }
      if ((level_4) == 2.0f) {
        curSupport *= 1.0f;
      } else if ((index_4) == 1.0f) {
        curSupport *= max(2.0f - ((level_4) * (locData[(k * 5) + 4])), 0.0f);
      } else if ((index_4) == ((level_4)-1.0f)) {
        curSupport *=
            max(((level_4) * (locData[(k * 5) + 4])) - (index_4) + 1.0f, 0.0f);
      } else {
        curSupport *=
            max(1.0f - fabs(((level_4) * (locData[(k * 5) + 4])) - (index_4)),
                0.0f);
      }
      myResult += curSupport;
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }
  ptrResult[globalIdx] = myResult;
}
