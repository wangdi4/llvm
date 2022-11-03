#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void
multTransModOCL(__global double *ptrSource, __global double *ptrData,
                __global double *ptrLevel, __global double *ptrIndex,
                __global double *ptrResult, int sourceSize, int offset,
                __global double *ptrAlpha1, __global double *ptrBeta,
                __global unsigned long *ptrGamma, __global double *ptrDelta,
                int isFirstTimeFlag) {
  int globalIdx = get_global_id(0);
  globalIdx = globalIdx + offset;
  double eval, index_calc, abs, last, localSupport, curSupport;
  double myResult = 0.0;
  double f_0 = 1.0;
  double f_1 = 1.0;
  double f_2 = 1.0;
  double f_3 = 1.0;
  double f_4 = 1.0;
  double Alpha[5], Beta[5], Delta[5];
  unsigned long Gamma[5];
  for (int i = 0; i < 5; ++i) {
    // Re-Store prameters prepared on HOST
    Alpha[i] = ptrAlpha1[(globalIdx * 5) + i];
    Beta[i] = ptrBeta[(globalIdx * 5) + i];
    Gamma[i] = ptrGamma[(globalIdx * 5) + i];
    Delta[i] = ptrDelta[(globalIdx * 5) + i];
    /// printf(" PARAMETERS RE-STORED \n");
  }
  // Iterate over all instances
  // for(int i = 0; i < sourceSize; i++)
  // int i = get_global_id(0);
  for (int i = 0; i < sourceSize; i++) {
    double x_0 = ptrData[(i * 5) + 0];
    f_0 =
        max(as_double(as_ulong(Alpha[0] * x_0 + Beta[0]) | Gamma[0]) + Delta[0],
            0.0);
    double x_1 = ptrData[(i * 5) + 1];
    f_1 =
        max(as_double(as_ulong(Alpha[1] * x_1 + Beta[1]) | Gamma[1]) + Delta[1],
            0.0);
    double x_2 = ptrData[(i * 5) + 2];
    f_2 =
        max(as_double(as_ulong(Alpha[2] * x_2 + Beta[2]) | Gamma[2]) + Delta[2],
            0.0);
    double x_3 = ptrData[(i * 5) + 3];
    f_3 =
        max(as_double(as_ulong(Alpha[3] * x_3 + Beta[3]) | Gamma[3]) + Delta[3],
            0.0);
    double x_4 = ptrData[(i * 5) + 4];
    f_4 =
        max(as_double(as_ulong(Alpha[4] * x_4 + Beta[4]) | Gamma[4]) + Delta[4],
            0.0);
    myResult += ptrSource[i] * (f_0) * (f_1) * (f_2) * (f_3) * (f_4);
  }
  ptrResult[globalIdx] = myResult;
}
