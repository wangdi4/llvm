

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void sinCosBasic(__global float *outF, __global float *inF) {
  int index = get_global_id(0);
  float fo = inF[index];
  outF[index] = sin(fo) + cos(fo);
}
