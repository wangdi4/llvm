// RUN: %clang_cc1 %s -cl-std=CL3.0 -finclude-default-header -verify -pedantic -fsyntax-only

// expected-no-diagnostics

kernel void addrspace_builtins(void) {
  int *generic_ptr;
  global int *global_ptr;
  local int *local_ptr;
  private int *private_ptr;

  global_ptr = to_global(generic_ptr);
  local_ptr = to_local(generic_ptr);
  private_ptr = to_private(generic_ptr);
}

void dse_builtins(void) {
  queue_t queue;
  ndrange_t ndrange;
  enqueue_kernel(queue, 0, ndrange, ^(void) {
    return;
  });
  unsigned size = get_kernel_work_group_size(^(void) {
    return;
  });
  size = get_kernel_preferred_work_group_size_multiple(^(void) {
    return;
  });
  size = get_kernel_sub_group_count_for_ndrange(ndrange, ^(void) {
    return;
  });
  size = get_kernel_max_sub_group_size_for_ndrange(ndrange, ^(void) {
    return;
  });
}

void pipe_builtins(read_only pipe int pipe_r, write_only pipe int pipe_w) {
  reserve_id_t reserve_id;
  uint num_packets;
  int *ptr;

  read_pipe(pipe_r, ptr);
  write_pipe(pipe_w, ptr);

  reserve_read_pipe(pipe_r, num_packets);
  reserve_write_pipe(pipe_w, num_packets);

  commit_read_pipe(pipe_r, reserve_id);
  commit_write_pipe(pipe_w, reserve_id);

  sub_group_reserve_read_pipe(pipe_r, num_packets);
  sub_group_reserve_write_pipe(pipe_w, num_packets);

  sub_group_commit_read_pipe(pipe_r, reserve_id);
  sub_group_commit_write_pipe(pipe_w, reserve_id);

  work_group_reserve_read_pipe(pipe_r, num_packets);
  work_group_reserve_write_pipe(pipe_w, num_packets);

  work_group_commit_read_pipe(pipe_r, reserve_id);
  work_group_commit_write_pipe(pipe_w, reserve_id);

  get_pipe_num_packets(pipe_r);
  get_pipe_max_packets(pipe_w);
}
