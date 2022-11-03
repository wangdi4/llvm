/// #define __DO_FLOAT__
#ifdef __DO_VECTOR__
#ifdef __DO_FLOAT__
#define vfloat float4
#define tfloat float
#else
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define vfloat double2
#define tfloat double
#endif
__kernel __attribute__((vec_type_hint(vfloat))) void
BlackScholesVector(const __global vfloat *S, const __global vfloat *K,
                   const __global vfloat *T, __global vfloat *vput,
                   __global vfloat *vcall, tfloat r_val, tfloat sig_val) {
  vfloat a, b, c, y, z, e, vc, d1, d2, w1, w2;
#ifdef __DO_FLOAT__
  const float4 inv_half = 0.5f, inv_one = 1.0f, inv_two = 2.0f;
  const float4 inv_sqrt2 = .7071067811865475727373109293694142252207f;
  const float4 r = r_val;
  const float4 sig = sig_val;
#else
  const double2 inv_half = 0.5, inv_one = 1.0, inv_two = 2.0;
  const double2 inv_sqrt2 = .7071067811865475727373109293694142252207;
  const double2 r = r_val;
  const double2 sig = sig_val;
#endif
  int i = get_global_id(0);
  a = log(S[i] / K[i]);
  b = T[i] * r;
  z = T[i] * sig * sig;
  c = inv_half * z;
  e = exp(-b);
  y = rsqrt(z);
  w1 = (a + b + c) * y;
  w2 = (a + b - c) * y;
  d1 = erf(inv_sqrt2 * w1);
  d2 = erf(inv_sqrt2 * w2);
  d1 = inv_half + inv_half * d1;
  d2 = inv_half + inv_half * d2;
  vc = S[i] * d1 - K[i] * e * d2;
  vcall[i] = vc;
  vput[i] = vc - S[i] + K[i] * e;
}
#else
// scalar version
#ifdef __DO_FLOAT__
#define vfloat float
#define tfloat float
#else
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define vfloat double
#define tfloat double
#endif
__kernel /*__attribute__((vec_type_hint(vfloat)))*/
    void
    BlackScholes(const __global vfloat *S, const __global vfloat *K,
                 const __global vfloat *T, __global vfloat *vput,
                 __global vfloat *vcall, tfloat r, tfloat sig) {
  vfloat a, b, c, y, z, e, vc, d1, d2, w1, w2;
#ifdef __DO_FLOAT__
  const float inv_half = 0.5f, inv_one = 1.0f, inv_two = 2.0f;
  const float inv_sqrt2 = .7071067811865475727373109293694142252207f;
#else
  const double inv_half = 0.5, inv_one = 1.0, inv_two = 2.0;
  const double inv_sqrt2 = .7071067811865475727373109293694142252207;
#endif
  int i = get_global_id(0);
  a = log(S[i] / K[i]);
  b = T[i] * r;
  z = T[i] * sig * sig;
  c = inv_half * z;
  e = exp(-b);
  y = rsqrt(z);
  w1 = (a + b + c) * y;
  w2 = (a + b - c) * y;
  d1 = erf(inv_sqrt2 * w1);
  d2 = erf(inv_sqrt2 * w2);
  d1 = inv_half + inv_half * d1;
  d2 = inv_half + inv_half * d2;
  vc = S[i] * d1 - K[i] * e * d2;
  vcall[i] = vc;
  vput[i] = vc - S[i] + K[i] * e;
}
#endif
