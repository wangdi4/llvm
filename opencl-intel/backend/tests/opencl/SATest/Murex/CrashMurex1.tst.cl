

float ocl_Func1(float *xa, float *ya, float x, int n, float *c, float *d) {
  int i, m, ns = 0;
  float den, dif, dift, ho, hp, w;
  float y = 0.f;

  dif = 0.f;
  for (i = 0; i < n; i++) {
    if ((dift = fabs(x - xa[i])) < dif) {
      ns = i;
      dif = dift;
    }
  }
  for (m = 1; m < n; m++) {
    for (i = 0; i < n - m; i++) {
      ho = xa[i];
      hp = xa[i + m];
      w = c[i + 1] - d[i];
      if ((den = ho - hp) == 0.f)
        break;
      den = w / den;
      d[i] = hp;
      c[i] = ho;
    }
    y += (2 * ns < (n - m) ? c[ns] : d[(ns--) - 1]);
  }

  return y;
}

float ocl_Func2(int iIndex, int iNX, __global float *gpfX, __global float *gpfY,
                float fX) {
  float fValue = 0.;
  float c[4] = {0}, d[4] = {0};
  float afAbsc[4] = {0}, afOrd[4] = {0};

  if (iNX == 2 || iNX == 3) {
    if (iNX == 2)
      fValue = ocl_Func1(afAbsc, afOrd, fX, 2, c, d);
    else {
      fValue = ocl_Func1(afAbsc, afOrd, fX, 3, c, d);
    }
  }

  return fValue;
}

__kernel void ocl_Kernel1(int iEvalN, int iVarN, __global int *gpiReduceN,
                          __global float *gpfX, __global float *gpfV,
                          __global float *gpfResults) {
  int iEval = 0;
  int iGlobalSize = get_global_size(0);
  float fResult = 0.0f;

  for (iEval = get_global_id(0); iEval < iEvalN; iEval += iGlobalSize) {
    for (int iVar = 0; iVar < iVarN; ++iVar) {
      int iReduceN = gpiReduceN[iEval + iVar * iEvalN];
      fResult = ocl_Func2(0, iReduceN, gpfX, gpfV, 1.f);

      gpfResults[iVar + iEval * iVarN] = fResult;
    }
  }
}
