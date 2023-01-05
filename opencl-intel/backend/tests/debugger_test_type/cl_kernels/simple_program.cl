__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  float kf1 = buf_in[0];
  int i;
  for (i = 0; i < 16; ++i)
    kf1 += i;
  kf1 += sizeof(int);
  kf1 += sizeof(float);
  kf1 += sizeof(char);
  buf_out[0] = kf1;
}
