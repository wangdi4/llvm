__kernel void test(__global int16 *in1, __global int16 *in2,
                   __global int16 *out) {
  int id = get_global_id(0);
  int16 tmp;
  tmp.s0 = in1[id].s1;
  tmp.s1 = in1[id].s2;
  tmp.s2 = in1[id].s3;
  tmp.s3 = in1[id].s4;
  tmp.s4 = in1[id].s5;
  tmp.s5 = in1[id].s6;
  tmp.s6 = in1[id].s7;
  tmp.s7 = in1[id].s8;
  tmp.s8 = in1[id].s9;
  tmp.s9 = in1[id].sa;
  tmp.sa = in1[id].sb;
  tmp.sb = in1[id].sc;
  tmp.sc = in1[id].sd;
  tmp.sd = in1[id].se;
  tmp.se = in1[id].sf;
  tmp.sf = in2[id].s0;
  out[id] = tmp;
}
