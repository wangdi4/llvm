__kernel void multTransSPOCL(__global float *ptrSource, __global float *ptrData,
                             __global float *ptrLevel, __global float *ptrIndex,
                             __global float *ptrResult, int sourceSize,
                             int offset) {
  int globalIdx = get_global_id(0);
  globalIdx = globalIdx + offset;
  float eval, index_calc, abs, last, localSupport, curSupport;
  float myResult = 0.0f;
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
  // Iterate over all instances
  for (int i = 0; i < sourceSize; i++) {
    curSupport = ptrSource[i];
    eval = ((level_0) * (ptrData[(i * 5) + 0]));
    index_calc = eval - (index_0);
    abs = fabs(index_calc);
    last = 1.0f - abs;
    localSupport = fmax(last, 0.0f);
    curSupport *= localSupport;
    eval = ((level_1) * (ptrData[(i * 5) + 1]));
    index_calc = eval - (index_1);
    abs = fabs(index_calc);
    last = 1.0f - abs;
    localSupport = fmax(last, 0.0f);
    curSupport *= localSupport;
    eval = ((level_2) * (ptrData[(i * 5) + 2]));
    index_calc = eval - (index_2);
    abs = fabs(index_calc);
    last = 1.0f - abs;
    localSupport = fmax(last, 0.0f);
    curSupport *= localSupport;
    eval = ((level_3) * (ptrData[(i * 5) + 3]));
    index_calc = eval - (index_3);
    abs = fabs(index_calc);
    last = 1.0f - abs;
    localSupport = fmax(last, 0.0f);
    curSupport *= localSupport;
    eval = ((level_4) * (ptrData[(i * 5) + 4]));
    index_calc = eval - (index_4);
    abs = fabs(index_calc);
    last = 1.0f - abs;
    localSupport = fmax(last, 0.0f);
    curSupport *= localSupport;
    myResult += curSupport;
  }
  ptrResult[globalIdx] = myResult;
}
