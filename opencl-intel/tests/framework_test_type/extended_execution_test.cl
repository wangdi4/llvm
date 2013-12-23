#if 1
kernel void my_func_C(global int *a) {
  ndrange_t ndrange = ndrange_1D(4);
  void (^my_blk_A)(local void *, local void *) =
      ^(local void * lptr1, local void * lptr2) {
  };

  // calculate local memory size for lptr
  //  // argument in local address space for my_blk_A
  uint local_mem_size = 16;

  enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_WAIT_KERNEL, ndrange,
                 my_blk_A, local_mem_size, local_mem_size * 4);
}
#else
kernel void foo(global int *a, global int *b) {
  ndrange_t ndrange = ndrange_1D(4);
  enqueue_kernel(get_default_queue(), 1, ndrange,
                 ^(local int * p) {
                   size_t id = get_global_id(0);
                   //    a[id] = b[id]; // undefined behavior
                 },
                 16);
  enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_WAIT_KERNEL, ndrange,
                 ^(local int * p) {}, 16);
}
#endif
