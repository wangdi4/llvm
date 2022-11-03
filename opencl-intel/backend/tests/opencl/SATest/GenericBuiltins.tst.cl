__kernel void test_fract(__global float *in, __global float *out,
                         __global float *glob, __local float *loc,
                         __global int *cond) {
  float *dest;

  if (*cond) {
    dest = glob;
  } else {
    dest = loc;
  }

  *out = fract(*in, dest);
}

__kernel void test_frexp(__global float *in, __global float *out,
                         __global int *glob, __local int *loc,
                         __global int *cond) {
  int *dest;

  if (*cond) {
    dest = glob;
  } else {
    dest = loc;
  }

  *out = frexp(*in, dest);
}

__kernel void test_lgamma_r(__global float *in, __global float *out,
                            __global int *glob, __local int *loc,
                            __global int *cond) {
  int *dest;

  if (*cond) {
    dest = glob;
  } else {
    dest = loc;
  }

  *out = lgamma_r(*in, dest);
}

__kernel void test_modf(__global float *in, __global float *out,
                        __global float *glob, __local float *loc,
                        __global int *cond) {
  float *dest;

  if (*cond) {
    dest = glob;
  } else {
    dest = loc;
  }

  *out = modf(*in, dest);
}

__kernel void test_remquo(__global float *in1, __global float *in2,
                          __global float *out, __global int *glob,
                          __local int *loc, __global int *cond) {
  int *dest;

  if (*cond) {
    dest = glob;
  } else {
    dest = loc;
  }

  *out = remquo(*in1, *in2, dest);
}

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

__kernel void test_vload(__global float2 *out, __global float *glob,
                         __local float *loc, __global int *cond) {
  float *src;

  if (*cond) {
    src = glob;
  } else {
    src = loc;
  }

  *out = vload2(0, src);
}

__kernel void test_vload_half(__global float *out, __global half *glob,
                              __local half *loc, __global int *cond) {
  half *src;

  if (*cond) {
    src = glob;
  } else {
    src = loc;
  }

  *out = vload_half(0, src);
}

__kernel void test_vstore(__global float2 *in, __global float *glob,
                          __local float *loc, __global int *cond) {
  float *dst;

  if (*cond) {
    dst = glob;
  } else {
    dst = loc;
  }

  vstore2(*in, 0, dst);
}

__kernel void test_store_half(__global float *in, __global half *glob,
                              __local half *loc, __global int *cond) {
  half *dst;

  if (*cond) {
    dst = glob;
  } else {
    dst = loc;
  }

  vstore_half(*in, 0, dst);
  vstore_half_rte(*in, 0, dst + 1);
  vstore_half_rtz(*in, 0, dst + 2);
  vstore_half_rtp(*in, 0, dst + 3);
  vstore_half_rtn(*in, 0, dst + 4);
}
