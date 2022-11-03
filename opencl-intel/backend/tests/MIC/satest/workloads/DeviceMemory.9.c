__kernel void writeGlobalMemoryCoalesced(__global float *output, int size) {
  int gid = get_global_id(0), num_thr = get_global_size(0),
      grpid = get_group_id(0), j = 0;
  float sum = 0;
  int s = gid;
  for (j = 0; j < 1024; ++j) {
    output[(s + 0) & (size - 1)] = gid;
    output[(s + 614400) & (size - 1)] = gid;
    output[(s + 1228800) & (size - 1)] = gid;
    output[(s + 1843200) & (size - 1)] = gid;
    output[(s + 2457600) & (size - 1)] = gid;
    output[(s + 3072000) & (size - 1)] = gid;
    output[(s + 3686400) & (size - 1)] = gid;
    output[(s + 4300800) & (size - 1)] = gid;
    output[(s + 4915200) & (size - 1)] = gid;
    output[(s + 5529600) & (size - 1)] = gid;
    output[(s + 6144000) & (size - 1)] = gid;
    output[(s + 6758400) & (size - 1)] = gid;
    output[(s + 7372800) & (size - 1)] = gid;
    output[(s + 7987200) & (size - 1)] = gid;
    output[(s + 8601600) & (size - 1)] = gid;
    output[(s + 9216000) & (size - 1)] = gid;
    s = (s + 9830400) & (size - 1);
  }
}
