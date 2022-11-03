#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void multTransOCL(__global double *ptrSource, __global double *ptrData,
                           __global double *ptrLevel, __global double *ptrIndex,
                           __global double *ptrResult, int sourceSize,
                           int offset) {
  int globalIdx = get_global_id(0);
  globalIdx = globalIdx + offset;
  double eval, index_calc, abs, last, localSupport, curSupport;
  double myResult = 0.0;
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
  // Iterate over all instances
  for (int i = 0; i < sourceSize; i++) {
    curSupport = ptrSource[i];
    eval = ((level_0) * (ptrData[(i * 5) + 0]));
    index_calc = eval - (index_0);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    eval = ((level_1) * (ptrData[(i * 5) + 1]));
    index_calc = eval - (index_1);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    eval = ((level_2) * (ptrData[(i * 5) + 2]));
    index_calc = eval - (index_2);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    eval = ((level_3) * (ptrData[(i * 5) + 3]));
    index_calc = eval - (index_3);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    eval = ((level_4) * (ptrData[(i * 5) + 4]));
    index_calc = eval - (index_4);
    abs = fabs(index_calc);
    last = 1.0 - abs;
    localSupport = fmax(last, 0.0);
    curSupport *= localSupport;
    myResult += curSupport;
  }
  ptrResult[globalIdx] = myResult;
}
