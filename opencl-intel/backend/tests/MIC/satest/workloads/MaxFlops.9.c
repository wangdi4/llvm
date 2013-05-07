
__kernel void Mul8(__global float *data, int nIters) {
  int gid = get_global_id(0), globalSize = get_global_size(0);
  float s = data[gid]-data[gid]+0.999f;
  float8 s0 = s + (float8)(0,0.1,0.2,0.3,0.4,0.5,0.6,0.7);
  for (int j=0 ; j<nIters ; ++j) {
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
 s0=s0*s0*1.01f;
  }
   data[gid] = s0.s0+s0.s1+s0.s2+s0.s3+s0.s4+s0.s5+s0.s6+s0.s7;
}
