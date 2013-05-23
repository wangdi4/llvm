__kernel void MAdd16(__global float *data, int nIters) {
  int gid = get_global_id(0), globalSize = get_global_size(0);
  float s = data[gid];
  float16 s0 = s + (float16)(0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1,1.1,1.2,1.3,1.4,1.5);
  for (int j=0 ; j<nIters ; ++j) {
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
 s0=10.0f-s0*0.9899f;
  }
   data[gid] = s0.s0+s0.s1+s0.s2+s0.s3+s0.s4+s0.s5+s0.s6+s0.s7+s0.s8+s0.s9+s0.sa+s0.sb+s0.sc+s0.sd+s0.se+s0.sf;
}
