__kernel void async_copy_test(__global uchar *buf_in1, __global uchar *buf_out1,
                              __local uchar *local1, __local uchar *local2) {
  int checker = 0;
  event_t evs;
  if (get_local_id(0) == 0) {
    *local1 = 10;
    *local2 = 11;
  }

  evs =
      async_work_group_copy((__local uchar *)local1,
                            (__global uchar *)(buf_in1 + 2), sizeof(uchar), 0);
  wait_group_events(1, &evs);
  checker = *local1;
  evs = async_work_group_copy((__global uchar *)buf_out1,
                              (__local uchar *)local2, sizeof(uchar), 0);
  wait_group_events(1, &evs);
  checker = *buf_out1;
  if (get_global_id(0) == 0)
    if (checker != 11)
      // THIS LINE SHOULDN'T BE REACHED ALTHOUGH IT DOES
      printf("2 FAIL \n");
  checker++;
}
