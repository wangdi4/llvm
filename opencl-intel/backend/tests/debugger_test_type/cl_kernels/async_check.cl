__kernel void main_kernel(__global uchar *buf_in1, __global uchar *buf_out1,
                          __global uchar *buf_in2, __global uchar *buf_out2,
                          __local uchar *local1, __local uchar *local2,
                          __local uchar *local3, __local uchar *local4) {
  int checker = 0;
  if (get_local_id(0) == 0) {
    *local1 = 10;
    *local2 = 11;
    *local3 = 12;
    *local4 = 13;
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  event_t evs =
      async_work_group_copy((__local uchar *)local1,
                            (__global uchar *)(buf_in1 + 2), sizeof(uchar), 0);
  wait_group_events(1, &evs);
  barrier(CLK_LOCAL_MEM_FENCE);
  barrier(CLK_GLOBAL_MEM_FENCE);
  checker = *local1;
  checker++;
  evs = async_work_group_copy((__global uchar *)buf_out1,
                              (__local uchar *)local2, sizeof(uchar), 0);
  wait_group_events(1, &evs);
  barrier(CLK_LOCAL_MEM_FENCE);
  barrier(CLK_GLOBAL_MEM_FENCE);
  checker = *buf_out1;
  checker++;
  evs = async_work_group_strided_copy((__local uchar *)local3,
                                      (__global uchar *)(buf_in2 + 4),
                                      sizeof(uchar), sizeof(uchar), 0);
  wait_group_events(1, &evs);
  barrier(CLK_LOCAL_MEM_FENCE);
  barrier(CLK_GLOBAL_MEM_FENCE);
  checker = *local3;
  checker++;
  evs = async_work_group_strided_copy(
      (__global uchar *)buf_out2, (__local uchar *)local4, sizeof(uchar), 1, 0);
  wait_group_events(1, &evs);
  barrier(CLK_LOCAL_MEM_FENCE);
  barrier(CLK_GLOBAL_MEM_FENCE);
  checker = *buf_out2;
  checker++;
}