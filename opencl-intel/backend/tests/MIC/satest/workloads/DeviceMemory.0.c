__kernel void readGlobalMemoryCoalesced(__global float *data,
                                        __global float *output, int size) {
  int gid = get_global_id(0), num_thr = get_global_size(0),
      grpid = get_group_id(0), j = 0;
  float sum = 0;
  int s = gid;
  for (j = 0; j < 1024; ++j) {
    float a0 = data[(s + 0) & (size - 1)];
    float a1 = data[(s + 32) & (size - 1)];
    float a2 = data[(s + 64) & (size - 1)];
    float a3 = data[(s + 96) & (size - 1)];
    float a4 = data[(s + 128) & (size - 1)];
    float a5 = data[(s + 160) & (size - 1)];
    float a6 = data[(s + 192) & (size - 1)];
    float a7 = data[(s + 224) & (size - 1)];
    float a8 = data[(s + 256) & (size - 1)];
    float a9 = data[(s + 288) & (size - 1)];
    float a10 = data[(s + 320) & (size - 1)];
    float a11 = data[(s + 352) & (size - 1)];
    float a12 = data[(s + 384) & (size - 1)];
    float a13 = data[(s + 416) & (size - 1)];
    float a14 = data[(s + 448) & (size - 1)];
    float a15 = data[(s + 480) & (size - 1)];
    sum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 +
           a13 + a14 + a15;
    s = (s + 512) & (size - 1);
  }
  output[gid] = sum;
}
