__kernel void conversions_flt2char_sat_rte(__global uchar4 *x,
                                           __global float4 *y) {
  int tid = get_global_id(0);
  uchar4 res = convert_uchar4_sat_rte(y[tid]);
  x[tid] = res;
}

__kernel void conversions_flt2char_rte(__global uchar4 *x, __global float4 *y) {
  int tid = get_global_id(0);
  uchar4 res = convert_uchar4_rte(y[tid]);
  x[tid] = res;
}

__kernel void conversions_flt2char(__global uchar4 *x, __global float4 *y) {
  int tid = get_global_id(0);
  uchar4 res = convert_uchar4(y[tid]);
  x[tid] = res;
}

__kernel void conversions_flt2char_sat(__global uchar4 *x, __global float4 *y) {
  int tid = get_global_id(0);
  uchar4 res = convert_uchar4_sat(y[tid]);
  x[tid] = res;
}

__kernel void conversions_char2int_sat_rte(__global uchar4 *x,
                                           __global int4 *y) {
  int tid = get_global_id(0);
  uchar4 res = convert_uchar4_sat_rte(y[tid]);
  x[tid] = res;
}

__kernel void conversions_char2int_rte(__global uchar4 *x, __global int4 *y) {
  int tid = get_global_id(0);
  uchar4 res = convert_uchar4_rte(y[tid]);
  x[tid] = res;
}

__kernel void conversions_char2int(__global uchar4 *x, __global int4 *y) {
  int tid = get_global_id(0);
  uchar4 res = convert_uchar4(y[tid]);
  x[tid] = res;
}

__kernel void conversions_char2int_sat(__global uchar4 *x, __global int4 *y) {
  int tid = get_global_id(0);
  uchar4 res = convert_uchar4_sat(y[tid]);
  x[tid] = res;
}
