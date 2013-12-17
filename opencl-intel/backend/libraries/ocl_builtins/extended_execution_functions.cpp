//Workaround for weired clang bug?
typedef int kernel_enqueue_flags_t;

////////// - enqueue_kernel
typedef void ExtendedExecutionContext;

extern int ocl20_enqueue_kernel_events(
    queue_t queue, kernel_enqueue_flags_t flags, const ndrange_t *ndrange,
    uint num_events_in_wait_list, const clk_event_t *in_wait_list,
    clk_event_t *event_ret, void *block, ExtendedExecutionContext *pEEC,
    void *RuntimeHandle);

int __attribute__((overloadable)) __attribute__((always_inline))
    enqueue_kernel(queue_t queue, kernel_enqueue_flags_t flags,
                   const ndrange_t ndrange, uint num_events_in_wait_list,
                   const clk_event_t *event_wait_list, clk_event_t *event_ret,
                   void (^block)(void)) {
  void *pEEC;
  void *RuntimeHandle;
  return ocl20_enqueue_kernel_events(queue, flags, &ndrange,
                                     num_events_in_wait_list, event_wait_list,
                                     event_ret, block, pEEC, RuntimeHandle);
}

extern int ocl20_enqueue_kernel_basic(queue_t queue,
                                      kernel_enqueue_flags_t flags,
                                      const ndrange_t *ndrange, void *block,
                                      ExtendedExecutionContext *pEEC,
                                      void *RuntimeHandle);
int __attribute__((overloadable)) __attribute__((always_inline))
    enqueue_kernel(queue_t queue, kernel_enqueue_flags_t flags,
                   const ndrange_t ndrange, void (^block)(void)) {
  void *pEEC;
  void *RuntimeHandle;
  return ocl20_enqueue_kernel_basic(queue, flags, &ndrange, block, pEEC,
                                    RuntimeHandle);
}

extern int ocl20_enqueue_kernel_localmem(
        queue_t queue,
        kernel_enqueue_flags_t flags,
        const ndrange_t* ndrange,
        void *block,
        unsigned *localbuf_size, unsigned localbuf_size_len,
        ExtendedExecutionContext * pEEC, void* RuntimeHandle);
int __attribute__((overloadable)) __attribute__((always_inline)) enqueue_kernel( queue_t queue, kernel_enqueue_flags_t flags, const ndrange_t ndrange, void (^block)(local void *, ...), uint size0,...) {
  unsigned *localbuf_size;
  unsigned localbuf_size_len;
  void *pEEC;
  void *RuntimeHandle;
  return ocl20_enqueue_kernel_localmem(queue, flags, &ndrange, block,
                                       localbuf_size, localbuf_size_len, pEEC,
                                       RuntimeHandle);
}

extern int ocl20_enqueue_kernel_events_localmem(
    const queue_t queue, kernel_enqueue_flags_t flags, const ndrange_t *ndrange,
    unsigned num_events_in_wait_list, const clk_event_t *in_wait_list,
    clk_event_t *event_ret, void *block, unsigned *localbuf_size,
    unsigned localbuf_size_len, ExtendedExecutionContext *pEEC,
    void *RuntimeHandle);
int __attribute__((overloadable)) __attribute__((always_inline))
    enqueue_kernel(queue_t queue, kernel_enqueue_flags_t flags,
                   const ndrange_t ndrange, uint num_events_in_wait_list,
                   const clk_event_t *event_wait_list, clk_event_t *event_ret,
                   void (^block)(local void *, ...), uint size0, ...) {
  unsigned *localbuf_size;
  unsigned localbuf_size_len;
  void *pEEC;
  void *RuntimeHandle;
  return ocl20_enqueue_kernel_events_localmem(
      queue, flags, &ndrange, num_events_in_wait_list, event_wait_list,
      event_ret, block, localbuf_size, localbuf_size_len, pEEC, RuntimeHandle);
}



////////// - enqueue_marker
extern int
ocl20_enqueue_marker(queue_t queue, uint num_events_in_wait_list,
                     const clk_event_t *event_wait_list, clk_event_t *event_ret,
                     ExtendedExecutionContext *pEEC);
int __attribute__((overloadable)) __attribute__((always_inline))
    enqueue_marker(queue_t queue, uint num_events_in_wait_list,
                   const clk_event_t *event_wait_list, clk_event_t *event_ret) {
  void *pEEC;
  return ocl20_enqueue_marker(queue, num_events_in_wait_list, event_wait_list,
                              event_ret, pEEC);
}

////////// - ndrange_1D, ndrange_2D, ndrange_3D
ndrange_t const_func __attribute__((overloadable))
    __attribute__((always_inline))
    ndrange_1D(size_t global_work_offset, size_t global_work_size,
               size_t local_work_size) {
  ndrange_t T;
  T.workDimension = 1;
  T.globalWorkOffset0[0] = global_work_offset;
  T.globalWorkSize[0] = global_work_size;
  T.localWorkSize[0] = local_work_size;
  return T;
}
ndrange_t const_func __attribute__((overloadable))
    __attribute__((always_inline)) ndrange_1D(size_t global_work_size) {
  size_t global_work_offset;
  size_t local_work_size;
  return ndrange_1D(global_work_offset, global_work_size, local_work_size);
}
ndrange_t const_func __attribute__((overloadable))
    __attribute__((always_inline))
    ndrange_1D(size_t global_work_size, size_t local_work_size) {
  size_t global_work_offset;
  return ndrange_1D(global_work_offset, global_work_size, local_work_size);
}

ndrange_t const_func __attribute__((overloadable))
    __attribute__((always_inline)) ndrange_2D(size_t global_work_size[2]) {
  size_t global_work_offset[2];
  size_t local_work_size[2];
  return ndrange_2D(global_work_offset, global_work_size, local_work_size);
}
ndrange_t const_func __attribute__((overloadable))
    __attribute__((always_inline))
    ndrange_2D(size_t global_work_size[2], size_t local_work_size[2]) {
  size_t global_work_offset[2];
  return ndrange_2D(global_work_offset, global_work_size, local_work_size);
}
ndrange_t const_func __attribute__((overloadable))
    __attribute__((always_inline))
    ndrange_2D(size_t global_work_offset[2], size_t global_work_size[2],
               size_t local_work_size[2]) {
  ndrange_t T;
  T.workDimension = 2;
  T.globalWorkOffset[0] = global_work_offset[0];
  T.globalWorkOffset[1] = global_work_offset[1];
  T.globalWorkSize[0] = global_work_size[0];
  T.globalWorkSize[1] = global_work_size[1];
  T.localWorkSize[0] = local_work_size[0];
  T.localWorkSize[1] = local_work_size[1];
  return T;
}

ndrange_t const_func __attribute__((overloadable)) __attribute__((always_inline)) ndrange_3D( size_t global_work_size[3]){
  size_t global_work_offset[3];
  size_t local_work_size[3];
  return ndrange_3D(global_work_offset, global_work_size, local_work_size);
}
ndrange_t const_func __attribute__((overloadable)) __attribute__((always_inline)) ndrange_3D( size_t global_work_size[3], size_t local_work_size[3]){
  size_t global_work_offset[3];
  return ndrange_3D(global_work_offset, global_work_size, local_work_size);
}
ndrange_t const_func __attribute__((overloadable)) __attribute__((always_inline)) ndrange_3D( size_t global_work_offset[3], size_t global_work_size[3], size_t local_work_size[3]){
  ndrange_t T;
  T.workDimension = 3;
  T.globalWorkOffset[0] = global_work_offset[0];
  T.globalWorkOffset[1] = global_work_offset[1];
  T.globalWorkOffset[2] = global_work_offset[2];
  T.globalWorkSize[0] = global_work_size[0];
  T.globalWorkSize[1] = global_work_size[1];
  T.globalWorkSize[2] = global_work_size[2];
  T.localWorkSize[0] = local_work_size[0];
  T.localWorkSize[1] = local_work_size[1];
  T.localWorkSize[2] = local_work_size[2];
}
