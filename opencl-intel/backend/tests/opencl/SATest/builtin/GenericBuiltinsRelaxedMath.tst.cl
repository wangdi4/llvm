__kernel void test_sincos(__global float *in, __global float *out,
                          __global float *glob, __local float *loc,
                          __global int *cond) {
  float *dest;

  if (*cond) {
    dest = glob;
  } else {
    dest = loc;
  }

  *out = sincos(*in, dest);
}
