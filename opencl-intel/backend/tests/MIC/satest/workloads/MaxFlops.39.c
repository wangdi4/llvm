#pragma OPENCL EXTENSION cl_khr_fp64: enable
__kernel void MulMAdd8(__global double *data, int nIters) {
  int gid = get_global_id(0), globalSize = get_global_size(0);
  double s = data[gid];
  double8 s0 = s + (double8)(0,0.1,0.2,0.3,0.4,0.5,0.6,0.7);
  for (int j=0 ; j<nIters ; ++j) {
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
 s0=(3.75f-0.355f*s0)*s0;
  }
   data[gid] = s0.s0+s0.s1+s0.s2+s0.s3+s0.s4+s0.s5+s0.s6+s0.s7;
}
