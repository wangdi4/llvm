#define NSAMP (262144)
#define MAX(a, b) select(b, a, (a > b))
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#ifdef __DO_FLOAT__
#define HALF 0.5f
#define ZERO 0.0f
#define ONE 1.0f
#define TWO 2.0f
#define LOW 0.02425f
#define HIGH 0.97575f
#undef HUGE
#define HUGE 6.0f
#define PI 3.14159265358979f
#define DIVISOR 4294967296.0f
#define vfloat float4
#define tfloat float
#define vdouble double4
#define CHANNELS 4
__constant float a[] = {-3.969683028665376e+01f, 2.209460984245205e+02f,
                        -2.759285104469687e+02f, 1.383577518672690e+02f,
                        -3.066479806614716e+01f, 2.506628277459239e+00f};
__constant float b[] = {-5.447609879822406e+01f, 1.615858368580409e+02f,
                        -1.556989798598866e+02f, 6.680131188771972e+01f,
                        -1.328068155288572e+01f};
__constant float4 av[] = {
    (float4)(-3.969683028665376e+01f), (float4)(2.209460984245205e+02f),
    (float4)(-2.759285104469687e+02f), (float4)(1.383577518672690e+02f),
    (float4)(-3.066479806614716e+01f), (float4)(2.506628277459239e+00f)};
__constant float4 bv[] = {
    (float4)(-5.447609879822406e+01f), (float4)(1.615858368580409e+02f),
    (float4)(-1.556989798598866e+02f), (float4)(6.680131188771972e+01f),
    (float4)(-1.328068155288572e+01f)};
__constant float c[] = {-7.784894002430293e-03f, -3.223964580411365e-01f,
                        -2.400758277161838e+00f, -2.549732539343734e+00f,
                        4.374664141464968e+00f,  2.938163982698783e+00f};
__constant float d[] = {7.784695709041462e-03f, 3.224671290700398e-01f,
                        2.445134137142996e+00f, 3.754408661907416e+00f};
#else
#define HALF 0.5
#define ZERO 0.0
#define ONE 1.0
#define TWO 2.0
#define LOW 0.02425
#define HIGH 0.97575
#undef HUGE
#define HUGE 6.0
#define PI 3.14159265358979
#define DIVISOR 4294967296.0
#define vfloat double4
#define tfloat double
#define vdouble double4
#define CHANNELS 2
__constant double a[] = {-3.969683028665376e+01, 2.209460984245205e+02,
                         -2.759285104469687e+02, 1.383577518672690e+02,
                         -3.066479806614716e+01, 2.506628277459239e+00};
__constant double b[] = {-5.447609879822406e+01, 1.615858368580409e+02,
                         -1.556989798598866e+02, 6.680131188771972e+01,
                         -1.328068155288572e+01};
__constant double4 av[] = {
    (double4)(-3.969683028665376e+01), (double4)(2.209460984245205e+02),
    (double4)(-2.759285104469687e+02), (double4)(1.383577518672690e+02),
    (double4)(-3.066479806614716e+01), (double4)(2.506628277459239e+00)};
__constant double4 bv[] = {
    (double4)(-5.447609879822406e+01), (double4)(1.615858368580409e+02),
    (double4)(-1.556989798598866e+02), (double4)(6.680131188771972e+01),
    (double4)(-1.328068155288572e+01)};
__constant double c[] = {-7.784894002430293e-03, -3.223964580411365e-01,
                         -2.400758277161838e+00, -2.549732539343734e+00,
                         4.374664141464968e+00,  2.938163982698783e+00};
__constant double d[] = {7.784695709041462e-03, 3.224671290700398e-01,
                         2.445134137142996e+00, 3.754408661907416e+00};
#endif
typedef struct {
  unsigned int matrix_a;
  unsigned int mask_b;
  unsigned int mask_c;
  unsigned int seed;
} mt_struct_stripped;
typedef struct {
  uint4 matrix_a;
  uint4 mask_b;
  uint4 mask_c;
  uint4 seed;
} mt_struct_stripped_vec;
#define MT_RNG_COUNT 4
#define MT_MM 9
#define MT_NN 19
#define MT_WMASK ((uint4)0xFFFFFFFFU)
#define MT_UMASK ((uint4)0xFFFFFFFEU)
#define MT_LMASK ((uint4)0x1U)
#define MT_SHIFT0 ((uint4)12)
#define MT_SHIFTB ((uint4)7)
#define MT_SHIFTC ((uint4)15)
#define MT_SHIFT1 ((uint4)18)
#define UINT4_ONE ((uint4)1)
#define UINT4_ZERO ((uint4)0)
#define UINT4_THIRTY ((uint4)30)
#define UINT4_BIG_CONST ((uint4)1812433253U)
#define MT_WMASK_1 ((uint)0xFFFFFFFFU)
#define MT_UMASK_1 ((uint)0xFFFFFFFEU)
#define MT_LMASK_1 ((uint)0x1U)
#define MT_SHIFT0_1 ((uint)12)
#define MT_SHIFTB_1 ((uint)7)
#define MT_SHIFTC_1 ((uint)15)
#define MT_SHIFT1_1 ((uint)18)
#define UINT_ONE ((uint)1)
#define UINT_ZERO ((uint)0)
#define UINT_THIRTY ((uint)30)
#define UINT_BIG_CONST ((uint)1812433253U)
#if 0
//original
tfloat ltqnorm(tfloat p)
{
    tfloat q, r;
#if 0
    if (p < ZERO || p > ONE)
    {
        return ZERO;
    }
    else if (p == ZERO)
    {
        return -HUGE; // minus "infinity"
    }
    else if (p == ONE)
    {
        return HUGE ; // "infinity"
    }
    else if (p < LOW)
    {
        // Rational approximation for lower region
        q = sqrt(-TWO*log(p));
        return (((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
            ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+ONE);
    }
    else if (p > HIGH)
    {
        // Rational approximation for upper region
        q  = sqrt(-TWO*log(ONE-p));
        return -(((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
            ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+ONE);
    }
    else
#else
    {
        // Rational approximation for central region
            q = p - HALF;
            r = q*q;
        return (((((a[0]*r+a[1])*r+a[2])*r+a[3])*r+a[4])*r+a[5])*q /
            (((((b[0]*r+b[1])*r+b[2])*r+b[3])*r+b[4])*r+ONE);
    }
#endif
}
#else
#if 1
tfloat ltqnorm(tfloat p) {
  tfloat q, r;
  tfloat sighn;
  if (p > LOW && p < HIGH) {
    // Rational approximation for central region
    q = p - HALF;
    r = q * q;
    return (((((a[0] * r + a[1]) * r + a[2]) * r + a[3]) * r + a[4]) * r +
            a[5]) *
           q /
           (((((b[0] * r + b[1]) * r + b[2]) * r + b[3]) * r + b[4]) * r + ONE);
  } else {
    q = (p <= LOW) ? sqrt(-TWO * log(p)) : sqrt(-TWO * log(ONE - p));
    sighn = (p <= LOW) ? ONE : -ONE;
    return sighn *
           (((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q +
            c[5]) /
           ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + ONE);
  }
}
#else
tfloat ltqnorm(tfloat p) {
  tfloat q;
  q = sqrt(-TWO * log(p));
  return (((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
         ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + ONE);
}
#endif
#endif
#ifdef __DO_FLOAT__
vfloat ltqnorm_vec(vfloat p) {
  vfloat q, r, res;
  int i, f;
  tfloat p_loc;
  tfloat *p_ptr = (tfloat *)&p;
  tfloat *res_ptr = (tfloat *)&res;
  f = 0;
  for (i = 0; i < 4; i++) {
    p_loc = *(p_ptr + i);
    if ((p_loc < LOW) || (p_loc > HIGH))
      f = 1;
  }
  /// f = 1;
  if (f == 1) {
    for (i = 0; i < 4; i++) {
      p_loc = *(p_ptr + i);
      *(res_ptr + i) = ltqnorm(p_loc);
    }
  } else {
    q = p - (vfloat)(HALF);
    r = q * q;
    res = (((((av[0] * r + av[1]) * r + av[2]) * r + av[3]) * r + av[4]) * r +
           av[5]) *
          q /
          (((((bv[0] * r + bv[1]) * r + bv[2]) * r + bv[3]) * r + bv[4]) * r +
           (vfloat)ONE);
  }
  return res;
}
#else
vdouble ltqnorm_vec(vdouble p) {
  vdouble q, r, res;
  int i, f;
  tfloat p_loc;
  tfloat *p_ptr = (tfloat *)&p;
  tfloat *res_ptr = (tfloat *)&res;
  f = 0;
  for (i = 0; i < 4; i++) {
    p_loc = *(p_ptr + i);
    if ((p_loc < LOW) || (p_loc > HIGH))
      f = 1;
  }
  /// f = 1;
  if (f == 1) {
    for (i = 0; i < 4; i++) {
      p_loc = *(p_ptr + i);
      *(res_ptr + i) = ltqnorm(p_loc);
    }
  } else {
    q = p - (vfloat)(HALF);
    r = q * q;
    res = (((((av[0] * r + av[1]) * r + av[2]) * r + av[3]) * r + av[4]) * r +
           av[5]) *
          q /
          (((((bv[0] * r + bv[1]) * r + bv[2]) * r + bv[3]) * r + bv[4]) * r +
           (vdouble)ONE);
  }
  return res;
}
#endif
// temporary commented out - causes internal compiler error
#if 0
__kernel /*__attribute__((vec_type_hint(vfloat)))*/
void MonteCarloEuroOptCLKernel(
#ifdef __DO_FLOAT__
    __global vfloat *vcall,
    __global vfloat *vput,
    vfloat r,
    vfloat sig,
    __global vfloat *s0,
    __global vfloat *x1,
    __global vfloat *t,
#else
    __global vdouble *vcall,
    __global vdouble *vput,
    vdouble r,
    vdouble sig,
    __global vdouble *s0,
    __global vdouble *x1,
    __global vdouble *t,
#endif
    __global mt_struct_stripped_vec *d_MT)
{
#ifdef __DO_FLOAT__
    vfloat rn, st, rnd_num, a, nu, y;
    vfloat dif_call, dif_put;
    vfloat dif_float;
#else
    vdouble rn, st, rnd_num, a, nu, y;
    vdouble dif_call, dif_put;
    vdouble dif_float;
#endif
    int tid;
    int iState, iState1, iStateM, iOut, j, k;
    uint4 mti, mti1, mtiM, x;
    uint4 mt[MT_NN], matrix_a, mask_b, mask_c;
    tid = get_global_id(0);
#ifdef __DO_FLOAT__
    vcall[tid]   = (vfloat)ZERO;
    vput[tid]    = (vfloat)ZERO;
#else
    vcall[tid]   = (vdouble)ZERO;
    vput[tid]    = (vdouble)ZERO;
#endif
    //Load bit-vector Mersenne Twister parameters
    matrix_a = d_MT[tid].matrix_a;
    mask_b   = d_MT[tid].mask_b;
    mask_c   = d_MT[tid].mask_c;
    mt[0] = d_MT[tid].seed;
    for (iState = 1; iState < MT_NN; iState++)
        mt[iState] = (UINT4_BIG_CONST * (mt[iState - 1] ^ (mt[iState - 1] >> UINT4_THIRTY)) + ((uint4)iState)) & MT_WMASK;
    iState = 0;
    mti1 = mt[0];
    for (iOut = 0; iOut < NSAMP; iOut++) {
        iState1 = iState + 1;
        iStateM = iState + MT_MM;
        if(iState1 >= MT_NN) iState1 -= MT_NN;
        if(iStateM >= MT_NN) iStateM -= MT_NN;
        mti  = mti1;
        mti1 = mt[iState1];
        mtiM = mt[iStateM];
        // MT recurrence
        x = (mti & MT_UMASK) | (mti1 & MT_LMASK);
        x = mtiM ^ (x >> UINT4_ONE) ^ select(UINT4_ZERO,  matrix_a, (x&UINT4_ONE) << 63U );
        mt[iState] = x;
        iState = iState1;
        //Tempering transformation
        x ^= (x >> MT_SHIFT0);
        x ^= (x << MT_SHIFTB) & mask_b;
        x ^= (x << MT_SHIFTC) & mask_c;
        x ^= (x >> MT_SHIFT1);
#ifdef __DO_FLOAT__
        y = convert_float4(x);
#else
        *(((double*)&y)+0) = (double)(*(((unsigned int*)&x)+0));
        *(((double*)&y)+1) = (double)(*(((unsigned int*)&x)+1));
        *(((double*)&y)+2) = (double)(*(((unsigned int*)&x)+2));
        *(((double*)&y)+3) = (double)(*(((unsigned int*)&x)+3));
#endif
        rnd_num  = ltqnorm_vec( (y + (vfloat)(ONE)) / ((vfloat)(DIVISOR)) );
        a = (r - sig*sig*HALF)*t[tid];
        nu = sig*sqrt(t[tid]);
        rn = rnd_num*nu + a;
        st = s0[tid]*exp(rn);
#ifdef __DO_FLOAT__
        //dif_float = st-x1[tid];
        //*(((double*)&dif_call) + 0) = (double)(  *( ((float*)&dif_float) + 0 )  );
        //*(((double*)&dif_call) + 1) = (double)(  *( ((float*)&dif_float) + 1 )  );
        //*(((double*)&dif_call) + 2) = (double)(  *( ((float*)&dif_float) + 2 )  );
        //*(((double*)&dif_call) + 3) = (double)(  *( ((float*)&dif_float) + 3 )  );
        //
        //dif_float = x1[tid]-st;
        //*(((double*)&dif_put)  + 0) = (double)(  *( ((float*)&dif_float) + 0 )  );
        //*(((double*)&dif_put)  + 1) = (double)(  *( ((float*)&dif_float) + 1 )  );
        //*(((double*)&dif_put)  + 2) = (double)(  *( ((float*)&dif_float) + 2 )  );
        //*(((double*)&dif_put)  + 3) = (double)(  *( ((float*)&dif_float) + 3 )  );
    dif_call = st-x1[tid];
        dif_put  = x1[tid]-st;
#else
        dif_call = st-x1[tid];
        dif_put  = x1[tid]-st;
#endif
        vcall[tid] += max(dif_call, (tfloat)ZERO);
        vput[tid]  += max(dif_put,  (tfloat)ZERO);
    }
#ifdef __DO_FLOAT__
    //*(((double*)(&(vcall[tid])))+0) = (*(((double*)(&(vcall[tid])))+0)) / ((double)NSAMP) * exp(-(*(((float*)(&r))+0)) * (*(((float*)(&(t[tid])))+0)));
    //*(((double*)(&(vcall[tid])))+1) = (*(((double*)(&(vcall[tid])))+1)) / ((double)NSAMP) * exp(-(*(((float*)(&r))+1)) * (*(((float*)(&(t[tid])))+1)));
    //*(((double*)(&(vcall[tid])))+2) = (*(((double*)(&(vcall[tid])))+2)) / ((double)NSAMP) * exp(-(*(((float*)(&r))+2)) * (*(((float*)(&(t[tid])))+2)));
    //*(((double*)(&(vcall[tid])))+3) = (*(((double*)(&(vcall[tid])))+3)) / ((double)NSAMP) * exp(-(*(((float*)(&r))+3)) * (*(((float*)(&(t[tid])))+3)));
    //*(((double*)(&(vput[tid])))+0)  = (*(((double*)(&(vput[tid])))+0))  / ((double)NSAMP) * exp(-(*(((float*)(&r))+0)) * (*(((float*)(&(t[tid])))+0)));
    //*(((double*)(&(vput[tid])))+1)  = (*(((double*)(&(vput[tid])))+1))  / ((double)NSAMP) * exp(-(*(((float*)(&r))+1)) * (*(((float*)(&(t[tid])))+1)));
    //*(((double*)(&(vput[tid])))+2)  = (*(((double*)(&(vput[tid])))+2))  / ((double)NSAMP) * exp(-(*(((float*)(&r))+2)) * (*(((float*)(&(t[tid])))+2)));
    //*(((double*)(&(vput[tid])))+3)  = (*(((double*)(&(vput[tid])))+3))  / ((double)NSAMP) * exp(-(*(((float*)(&r))+3)) * (*(((float*)(&(t[tid])))+3)));
    vcall[tid] = vcall[tid] / ((vfloat)NSAMP) * exp(-r*t[tid]);
    vput[tid]  = vput[tid]  / ((vfloat)NSAMP) * exp(-r*t[tid]);
#else
    vcall[tid] = vcall[tid] / ((vfloat)NSAMP) * exp(-r*t[tid]);
    vput[tid]  = vput[tid]  / ((vfloat)NSAMP) * exp(-r*t[tid]);
#endif
}
#endif
// temporary commented out - causes internal compiler error
#if 0
__kernel /*__attribute__((vec_type_hint(tfloat)))*/
void MonteCarloEuroOptCLKernelScalar(
    __global tfloat *vcall,
    __global tfloat *vput,
    tfloat r,
    tfloat sig,
    __global tfloat *s0,
    __global tfloat *x1,
    __global tfloat *t,
    __global mt_struct_stripped *d_MT)
{
    tfloat rn, st, rnd_num, a, nu, y;
    tfloat dif_call, dif_put;
    tfloat dif_float;
    int tid;
    int iState, iState1, iStateM, iOut, j, k;
    uint mti, mti1, mtiM, x;
    uint mt[MT_NN], matrix_a, mask_b, mask_c;
    tid = get_global_id(0);
    vcall[tid]   = (tfloat)ZERO;
    vput[tid]    = (tfloat)ZERO;
    //Load bit-vector Mersenne Twister parameters
    matrix_a = d_MT[tid].matrix_a;
    mask_b   = d_MT[tid].mask_b;
    mask_c   = d_MT[tid].mask_c;
    mt[0] = d_MT[tid].seed;
    for (iState = 1; iState < MT_NN; iState++)
        mt[iState] = (UINT_BIG_CONST * (mt[iState - 1] ^ (mt[iState - 1] >> UINT_THIRTY)) + ((uint)iState)) & MT_WMASK_1;
    iState = 0;
    mti1 = mt[0];
    for (iOut = 0; iOut < NSAMP; iOut++) {
        iState1 = iState + 1;
        iStateM = iState + MT_MM;
        if(iState1 >= MT_NN) iState1 -= MT_NN;
        if(iStateM >= MT_NN) iStateM -= MT_NN;
        mti  = mti1;
        mti1 = mt[iState1];
        mtiM = mt[iStateM];
        // MT recurrence
        x = (mti & MT_UMASK_1) | (mti1 & MT_LMASK_1);
    x = mtiM ^ (x >> UINT_ONE) ^ ((x&UINT_ONE) ? matrix_a : UINT_ZERO); ///select(UINT_ZERO,  matrix_a, (x&UINT_ONE) << 63U );
        mt[iState] = x;
        iState = iState1;
        //Tempering transformation
        x ^= (x >> MT_SHIFT0_1);
        x ^= (x << MT_SHIFTB_1) & mask_b;
        x ^= (x << MT_SHIFTC_1) & mask_c;
        x ^= (x >> MT_SHIFT1_1);
    y = (tfloat)x;
        rnd_num  = ltqnorm( (y + (tfloat)(ONE)) / ((tfloat)(DIVISOR)) );
        a = (r - sig*sig*HALF)*t[tid];
        nu = sig*sqrt(t[tid]);
        rn = rnd_num*nu + a;
        st = s0[tid]*exp(rn);
    dif_call = st-x1[tid];
        dif_put  = x1[tid]-st;
        vcall[tid] += max(dif_call, (tfloat)ZERO);
        vput[tid]  += max(dif_put,  (tfloat)ZERO);
    }
    vcall[tid] = vcall[tid] / ((tfloat)NSAMP) * exp(-r*t[tid]);
    vput[tid]  = vput[tid]  / ((tfloat)NSAMP) * exp(-r*t[tid]);
}
#endif
__kernel /*__attribute__((vec_type_hint(tfloat)))*/
    void
    MonteCarloEuroOptCLKernelScalarBoxMuller(
        __global tfloat *vcall, __global tfloat *vput, tfloat r, tfloat sig,
        __global tfloat *s0, __global tfloat *x1, __global tfloat *t,
        __global mt_struct_stripped *d_MT) {
  tfloat rn, st, rnd_num, a, nu, y, rnd_num1, y1;
  tfloat dif_call, dif_put;
  tfloat dif_float;
  tfloat p, p1;
  tfloat t_sqrt;
  tfloat inv_divisor;
  int tid;
  int iState, iState1, iStateM, iOut, j, k;
  uint mti, mti1, mtiM, x;
  uint mt[MT_NN], matrix_a, mask_b, mask_c;
  tid = get_global_id(0);
  t_sqrt = sqrt(t[tid]);
  inv_divisor = ONE / ((tfloat)DIVISOR);
  a = (r - sig * sig * HALF) * t[tid];
  /// nu = sig*sqrt(t[tid]);
  nu = sig * t_sqrt;
  vcall[tid] = (tfloat)ZERO;
  vput[tid] = (tfloat)ZERO;
  // Load bit-vector Mersenne Twister parameters
  matrix_a = d_MT[tid].matrix_a;
  mask_b = d_MT[tid].mask_b;
  mask_c = d_MT[tid].mask_c;
  mt[0] = d_MT[tid].seed;
  for (iState = 1; iState < MT_NN; iState++)
    mt[iState] =
        (UINT_BIG_CONST * (mt[iState - 1] ^ (mt[iState - 1] >> UINT_THIRTY)) +
         ((uint)iState)) &
        MT_WMASK_1;
  iState = 0;
  mti1 = mt[0];
  for (iOut = 0; iOut < NSAMP; iOut = iOut + 2) {
    // first rnd number
    iState1 = iState + 1;
    iStateM = iState + MT_MM;
    if (iState1 >= MT_NN)
      iState1 -= MT_NN;
    if (iStateM >= MT_NN)
      iStateM -= MT_NN;
    mti = mti1;
    mti1 = mt[iState1];
    mtiM = mt[iStateM];
    // MT recurrence
    x = (mti & MT_UMASK_1) | (mti1 & MT_LMASK_1);
    x = mtiM ^ (x >> UINT_ONE) ^
        ((x & UINT_ONE) ? matrix_a : UINT_ZERO); /// select(UINT_ZERO, matrix_a,
                                                 /// (x&UINT_ONE) << 63U );
    mt[iState] = x;
    iState = iState1;
    // Tempering transformation
    x ^= (x >> MT_SHIFT0_1);
    x ^= (x << MT_SHIFTB_1) & mask_b;
    x ^= (x << MT_SHIFTC_1) & mask_c;
    x ^= (x >> MT_SHIFT1_1);
    y = (tfloat)x;
    // second rnd number
    iState1 = iState + 1;
    iStateM = iState + MT_MM;
    if (iState1 >= MT_NN)
      iState1 -= MT_NN;
    if (iStateM >= MT_NN)
      iStateM -= MT_NN;
    mti = mti1;
    mti1 = mt[iState1];
    mtiM = mt[iStateM];
    // MT recurrence
    x = (mti & MT_UMASK_1) | (mti1 & MT_LMASK_1);
    x = mtiM ^ (x >> UINT_ONE) ^
        ((x & UINT_ONE) ? matrix_a : UINT_ZERO); /// select(UINT_ZERO, matrix_a,
                                                 /// (x&UINT_ONE) << 63U );
    mt[iState] = x;
    iState = iState1;
    // Tempering transformation
    x ^= (x >> MT_SHIFT0_1);
    x ^= (x << MT_SHIFTB_1) & mask_b;
    x ^= (x << MT_SHIFTC_1) & mask_c;
    x ^= (x >> MT_SHIFT1_1);
    y1 = (tfloat)x;
    // Generate normal distribution
    /// p = (y + (tfloat)(ONE)) / ((tfloat)(DIVISOR));
    /// p1 = (y1 + (tfloat)(ONE)) / ((tfloat)(DIVISOR));
    p = (y + (tfloat)(ONE)) * inv_divisor;
    p1 = (y1 + (tfloat)(ONE)) * inv_divisor;
    // Polynoms
    /// rnd_num  = ltqnorm( p );
    /// rnd_num1  = ltqnorm( p1 );
    // BoxMuller
    tfloat tmp_p = sqrt(-TWO * log(p));
#if 1
    tfloat sin_val, cos_val;
    sin_val = sincos(TWO * PI * p1, &cos_val);
    rnd_num = tmp_p * cos_val;
    rnd_num1 = tmp_p * sin_val;
#else
    rnd_num = tmp_p * cos(TWO * PI * p1);
    rnd_num1 = tmp_p * sin(TWO * PI * p1);
#endif
    /// if(tid==0)
    ///{
    ///  printf("%f\n", rnd_num);
    ///  printf("%f\n", rnd_num1);
    /// }
    // add first sample from pair
    rn = rnd_num * nu + a;
    st = s0[tid] * exp(rn);
    dif_call = st - x1[tid];
    dif_put = x1[tid] - st;
    vcall[tid] += max(dif_call, (tfloat)ZERO);
    vput[tid] += max(dif_put, (tfloat)ZERO);
    // add second sample from pair
    rn = rnd_num1 * nu + a;
    st = s0[tid] * exp(rn);
    dif_call = st - x1[tid];
    dif_put = x1[tid] - st;
    vcall[tid] += max(dif_call, (tfloat)ZERO);
    vput[tid] += max(dif_put, (tfloat)ZERO);
  }
  vcall[tid] = vcall[tid] / ((tfloat)NSAMP) * exp(-r * t[tid]);
  vput[tid] = vput[tid] / ((tfloat)NSAMP) * exp(-r * t[tid]);
}
