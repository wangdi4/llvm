

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void sinCosTwice(__global float *outF, __global float *inF) {
  int index = get_global_id(0);
  float fo = inF[index];
  float fo1 = 2 * fo;
  float cos1Val = cos(fo1);
  float sinVal = sin(fo);
  float cosVal = cos(fo);
  outF[index] = cosVal + sinVal;
  float sin1Val = sin(fo1);
  outF[index] += cos1Val + sin1Val + sin(2 * fo1);
}
