
typedef struct complex4_ {
  float4 r;
  float4 i;
} complex4;

complex4 expi4(float4 alpha) {
  complex4 res;

  res.r = cos(alpha);
  res.i = sin(alpha);

  return res;
}

__kernel void f(__global float4 *in, __global complex4 *out) {
  out[0] = expi4(in[0]);
}