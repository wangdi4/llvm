__kernel void writeGlobalMemoryCoalesced(__global float *output, int size) {
  int gid = get_global_id(0), num_thr = get_global_size(0),
      grpid = get_group_id(0), j = 0;
  float sum = 0;
  int s = gid;
  for (j = 0; j < 1024; ++j) {
    output[(s + 0) & (size - 1)] = gid;
    output[(s + 32) & (size - 1)] = gid;
    output[(s + 64) & (size - 1)] = gid;
    output[(s + 96) & (size - 1)] = gid;
    output[(s + 128) & (size - 1)] = gid;
    output[(s + 160) & (size - 1)] = gid;
    output[(s + 192) & (size - 1)] = gid;
    output[(s + 224) & (size - 1)] = gid;
    output[(s + 256) & (size - 1)] = gid;
    output[(s + 288) & (size - 1)] = gid;
    output[(s + 320) & (size - 1)] = gid;
    output[(s + 352) & (size - 1)] = gid;
    output[(s + 384) & (size - 1)] = gid;
    output[(s + 416) & (size - 1)] = gid;
    output[(s + 448) & (size - 1)] = gid;
    output[(s + 480) & (size - 1)] = gid;
    s = (s + 512) & (size - 1);
  }
}
