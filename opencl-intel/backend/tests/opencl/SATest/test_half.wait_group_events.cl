__kernel void test(__global float *p, __global half *f) {
  __local ushort data[16 * 1024];
  size_t i = get_global_id(0);
  size_t lid = get_local_id(0);
  size_t lsize = get_local_size(0);
  size_t vecsize = vec_step(p[0]);
  event_t async_event;
  vstore_half_rtn(p[i], lid, (__local half *)(&data[0]));
  barrier(CLK_LOCAL_MEM_FENCE);
  async_event =
      async_work_group_copy((__global ushort *)f + vecsize * (i - lid),
                            (__local ushort *)(&data[0]), vecsize * lsize, 0);
  wait_group_events(1, &async_event);
}
