#if !defined(__MIC__) && !defined(__MIC2__)
typedef void ExtendedExecutionContext;

////////// - enqueue_kernel
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
    queue_t queue, kernel_enqueue_flags_t flags, const ndrange_t *ndrange,
    void *block, unsigned *localbuf_size, unsigned localbuf_size_len,
    ExtendedExecutionContext *pEEC, void *RuntimeHandle);
int __attribute__((overloadable)) __attribute__((always_inline))
    enqueue_kernel(queue_t queue, kernel_enqueue_flags_t flags,
                   const ndrange_t ndrange, void (^block)(local void *, ...),
                   uint size0, ...) {
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
extern int ocl20_enqueue_marker(queue_t queue, uint num_events_in_wait_list,
                                const clk_event_t *event_wait_list,
                                clk_event_t *event_ret,
                                ExtendedExecutionContext *pEEC);
int __attribute__((always_inline))
    enqueue_marker(queue_t queue, uint num_events_in_wait_list,
                   const clk_event_t *event_wait_list, clk_event_t *event_ret) {
  void *pEEC;
  return ocl20_enqueue_marker(queue, num_events_in_wait_list, event_wait_list,
                              event_ret, pEEC);
}

////////// - get_default_queue
extern queue_t ocl20_get_default_queue(ExtendedExecutionContext *pEEC);
queue_t __attribute__((always_inline)) get_default_queue(void) {
  ExtendedExecutionContext *pEEC;
  return ocl20_get_default_queue(pEEC);
}

////////// - ndrange_1D, ndrange_2D, ndrange_3D
ndrange_t const_func __attribute__((overloadable))
    __attribute__((always_inline))
    ndrange_1D(size_t global_work_offset, size_t global_work_size,
               size_t local_work_size) {
  ndrange_t T;
  T.workDimension = 1;
  T.globalWorkOffset[0] = global_work_offset;
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

ndrange_t const_func __attribute__((overloadable))
    __attribute__((always_inline)) ndrange_3D(size_t global_work_size[3]) {
  size_t global_work_offset[3];
  size_t local_work_size[3];
  return ndrange_3D(global_work_offset, global_work_size, local_work_size);
}
ndrange_t const_func __attribute__((overloadable))
    __attribute__((always_inline))
    ndrange_3D(size_t global_work_size[3], size_t local_work_size[3]) {
  size_t global_work_offset[3];
  return ndrange_3D(global_work_offset, global_work_size, local_work_size);
}
ndrange_t const_func __attribute__((overloadable))
    __attribute__((always_inline))
    ndrange_3D(size_t global_work_offset[3], size_t global_work_size[3],
               size_t local_work_size[3]) {
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
  return T;
}

////////// - retain_event, release_event, create_user_event, set_user_event_status, capture_event_profiling_info
extern void ocl20_retain_event(clk_event_t event,
                               ExtendedExecutionContext *pEEC);
void __attribute__((always_inline)) retain_event(clk_event_t event) {
  ExtendedExecutionContext *pEEC;
  return ocl20_retain_event(event, pEEC);
}

extern void ocl20_release_event(clk_event_t event,
                                ExtendedExecutionContext *pEEC);
void __attribute__((always_inline)) release_event(clk_event_t event) {
  ExtendedExecutionContext *pEEC;
  return ocl20_release_event(event, pEEC);
}

extern clk_event_t ocl20_create_user_event(ExtendedExecutionContext * pEEC);
clk_event_t __attribute__((always_inline)) create_user_event() {
  ExtendedExecutionContext *pEEC;
  return ocl20_create_user_event(pEEC);
}

extern void ocl20_set_user_event_status(clk_event_t event, uint status,
                                        ExtendedExecutionContext *pEEC);
void __attribute__((always_inline))
    set_user_event_status(clk_event_t event, int status) {
  ExtendedExecutionContext *pEEC;
  return ocl20_set_user_event_status(event, status, pEEC);
}

extern void ocl20_capture_event_profiling_info(clk_event_t event,
                                               clk_profiling_info name,
                                               global ulong *value,
                                               ExtendedExecutionContext *pEEC);
void __attribute__((always_inline))
    capture_event_profiling_info(clk_event_t event, clk_profiling_info name,
                                 global ulong *value) {
  ExtendedExecutionContext *pEEC;
  return ocl20_capture_event_profiling_info(event, name, value, pEEC);
}

////////// - get_kernel_work_group_size
extern uint ocl20_get_kernel_wg_size(void *block,
                                     ExtendedExecutionContext *pEEC);
uint __attribute__((overloadable)) __attribute__((always_inline))
    get_kernel_work_group_size(void (^block)(void)) {
  ExtendedExecutionContext *pEEC;
  return ocl20_get_kernel_wg_size(block, pEEC);
}

extern uint ocl20_get_kernel_wg_size_local(void *block,
                                               ExtendedExecutionContext *pEEC);
uint __attribute__((overloadable)) __attribute__((always_inline))
    get_kernel_work_group_size(void (^block)(local void *, ...)) {
  ExtendedExecutionContext *pEEC;
  return ocl20_get_kernel_wg_size_local(block, pEEC);
}

////////// - get_kernel_preferred_work_group_size_multiple
extern uint
ocl20_get_kernel_preferred_wg_size_multiple(void *block,
                                            ExtendedExecutionContext *pEEC);
uint __attribute__((overloadable)) __attribute__((always_inline))
    get_kernel_preferred_work_group_size_multiple(void (^block)(void)) {
  ExtendedExecutionContext *pEEC;
  return ocl20_get_kernel_preferred_wg_size_multiple(block, pEEC);
}

extern uint ocl20_get_kernel_preferred_wg_size_multiple_local(
    void *block, ExtendedExecutionContext *pEEC);
uint __attribute__((overloadable)) __attribute__((always_inline))
    get_kernel_preferred_work_group_size_multiple(void (^block)(local void *,
                                                                ...)) {
  ExtendedExecutionContext *pEEC;
  return ocl20_get_kernel_preferred_wg_size_multiple_local(block, pEEC);
}
#endif
