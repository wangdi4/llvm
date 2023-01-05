__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  float fi = 8.32f;
  float fo = exp(fi);
  int i;
  float farr[4];
  for (i = 0; i < 4; ++i)
    farr[i] = fi + (float)i;

  float4 ff4;
  ff4.x = farr[0];
  ff4.y = farr[1];
  ff4.z = farr[2];
  ff4.w = farr[3];

  float4 expff4 = exp(ff4);

  buf_out[0] = 0;
}
