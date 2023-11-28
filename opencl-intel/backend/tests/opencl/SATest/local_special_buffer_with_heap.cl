__kernel void test(__local int *data_l, __global int *data_g) {
  __local int temp_l[1024 * 32];
  __private int temp_p[1024 * 32];
  int lid = get_local_id(0);
  int lsize = get_local_size(0); // 32
  for (int i = 0; i < 1024 / lsize; i++) {
    temp_l[lid * 32 + i] = lid;
  }
  for (int i = 0; i < 1024; i++) {
    temp_p[i] = lid;
  }
  data_l[lid] = lid;
  barrier(CLK_GLOBAL_MEM_FENCE);
  data_g[lid] = temp_p[1023 - lid] + temp_l[1023 - lid] + data_l[lid];
}
