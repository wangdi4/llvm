#if !defined(__MIC__) && !defined(__MIC2__)

// This file contains implementation of all Extended Execution related built-in functions except for those that are var-args.
//The var-args built-ins that are not implemented here are:
//int __attribute__((overloadable))
//    enqueue_kernel(queue_t queue, kernel_enqueue_flags_t flags,
//                   const ndrange_t ndrange, void (^block)(local void *, ...),
//                   uint size0, ...);
//int __attribute__((overloadable))
//    enqueue_kernel(queue_t queue, kernel_enqueue_flags_t flags,
//                   const ndrange_t ndrange, uint num_events_in_wait_list,
//                   const clk_event_t *event_wait_list, clk_event_t *event_ret,
//                   void (^block)(local void *, ...), uint size0, ...);

// Upper bound on amount of var args that can be in a call
#define MAX_VAR_ARGS_COUNT (32)

////////// - externals used for accessing values from kernel's implicit args
extern void* __attribute__((const)) __get_device_command_manager(void);
extern void* __attribute__((const)) __get_block_to_kernel_mapper(void);
extern void* __attribute__((const)) __get_runtime_handle(void);

////////// - enqueue_kernel
extern int ocl20_enqueue_kernel_events(
    queue_t queue, kernel_enqueue_flags_t flags, const ndrange_t *ndrange,
    uint num_events_in_wait_list, const __global clk_event_t *in_wait_list,
    __global clk_event_t *event_ret, __private void *block, void* DCM,
    void* B2K, void *RuntimeHandle);
int __attribute__((overloadable)) __attribute__((always_inline))
    enqueue_kernel(queue_t queue, kernel_enqueue_flags_t flags,
                   const ndrange_t ndrange, uint num_events_in_wait_list,
                   const __global clk_event_t *event_wait_list, clk_event_t __global *event_ret,
                   void (^block)(void)) {
  void* DCM = __get_device_command_manager();
  void* B2K = __get_block_to_kernel_mapper();
  void *RuntimeHandle = __get_runtime_handle();
  return ocl20_enqueue_kernel_events(queue, flags, &ndrange,
                                     num_events_in_wait_list, event_wait_list,
                                     event_ret, block, DCM, B2K, RuntimeHandle);
}

extern int ocl20_enqueue_kernel_basic(queue_t queue,
                                      kernel_enqueue_flags_t flags,
                                      const ndrange_t *ndrange, private void *block,
                                      void* DCM,
                                      void* B2K,
                                      void *RuntimeHandle);
int __attribute__((overloadable)) __attribute__((always_inline))
    enqueue_kernel(queue_t queue, kernel_enqueue_flags_t flags,
                   const ndrange_t ndrange, void (^block)(void)) {
  void* DCM = __get_device_command_manager();
  void* B2K = __get_block_to_kernel_mapper();
  void *RuntimeHandle = __get_runtime_handle();
  return ocl20_enqueue_kernel_basic(queue, flags, &ndrange, block, DCM, B2K,
                                    RuntimeHandle);
}

////////// - enqueue_marker
extern int ocl20_enqueue_marker(queue_t queue, uint num_events_in_wait_list,
                                const __global clk_event_t *event_wait_list,
                                __global clk_event_t *event_ret,
                                void* DCM);
int __attribute__((always_inline)) __attribute__((overloadable))
    enqueue_marker(queue_t queue, uint num_events_in_wait_list,
                   const __global clk_event_t *event_wait_list, __global clk_event_t* event_ret) {
  void* DCM = __get_device_command_manager();
  return ocl20_enqueue_marker(queue, num_events_in_wait_list, event_wait_list,
                              event_ret, DCM);
}

////////// - get_default_queue
extern queue_t __attribute__((const)) ocl20_get_default_queue(void *DCM);
queue_t __attribute__((const)) __attribute__((always_inline))  __attribute__((overloadable))
    get_default_queue(void) {
  void* DCM = __get_device_command_manager();
  return ocl20_get_default_queue(DCM);
}

////////// - ndrange_1D, ndrange_2D, ndrange_3D
ndrange_t __attribute__((const)) __attribute__((overloadable))
    ndrange_1D(size_t global_work_offset, size_t global_work_size,
               size_t local_work_size) {
  ndrange_t T;
  T.workDimension = 1;
  T.globalWorkOffset[0] = global_work_offset;
  T.globalWorkSize[0] = global_work_size;
  T.localWorkSize[0] = local_work_size;
  return T;
}
ndrange_t __attribute__((const)) __attribute__((overloadable))
  ndrange_1D(size_t global_work_size) {
  size_t global_work_offset = 0;
  size_t local_work_size = 0;
  return ndrange_1D(global_work_offset, global_work_size, local_work_size);
}
ndrange_t __attribute__((const)) __attribute__((overloadable))
    __attribute__((always_inline))
    ndrange_1D(size_t global_work_size, size_t local_work_size) {
  size_t global_work_offset = 0;
  return ndrange_1D(global_work_offset, global_work_size, local_work_size);
}

ndrange_t __attribute__((const)) __attribute__((overloadable))
    __attribute__((always_inline)) ndrange_2D(const size_t global_work_size[2]) {
  size_t global_work_offset[2] = { 0, 0 };
  size_t local_work_size[2] = { 0, 0 };
  return ndrange_2D(global_work_offset, global_work_size, local_work_size);
}
ndrange_t __attribute__((const)) __attribute__((overloadable))
    __attribute__((always_inline))
    ndrange_2D(const size_t global_work_size[2], const size_t local_work_size[2]) {
  size_t global_work_offset[2]= { 0, 0 };
  return ndrange_2D(global_work_offset, global_work_size, local_work_size);
}
ndrange_t __attribute__((const))  __attribute__((overloadable))
    __attribute__((always_inline))
    ndrange_2D(const size_t global_work_offset[2], const size_t global_work_size[2],
               const size_t local_work_size[2]) {
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

ndrange_t __attribute__((const)) __attribute__((overloadable))
    __attribute__((always_inline)) ndrange_3D(const size_t global_work_size[3]) {
  size_t global_work_offset[3] = { 0, 0, 0 };
  size_t local_work_size[3] = { 0, 0, 0 };
  return ndrange_3D(global_work_offset, global_work_size, local_work_size);
}
ndrange_t __attribute__((const))  __attribute__((overloadable))
    __attribute__((always_inline))
    ndrange_3D(const size_t global_work_size[3], const size_t local_work_size[3]) {
  size_t global_work_offset[3] = { 0, 0, 0 };
  return ndrange_3D(global_work_offset, global_work_size, local_work_size);
}
ndrange_t __attribute__((const)) __attribute__((overloadable))
    __attribute__((always_inline))
    ndrange_3D(const size_t global_work_offset[3], const size_t global_work_size[3],
               const size_t local_work_size[3]) {
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

// This method computes the count of sub-groups accommodated
// by a given kernel for requested work-group size.
// For sub-groups emulation this query returns 'one' if a kernel can execute a requested work-group size and 'zero' otherwise.
uint __attribute__((overloadable)) __attribute__((always_inline))
get_kernel_sub_group_count_for_ndrange(const ndrange_t ndrange,
                                        void(^block)(void)) {
  uint maxWGSize = get_kernel_work_group_size(block);
  size_t prod = 1;
  for (unsigned int i = 0; i < ndrange.workDimension; ++i)
      prod *= ndrange.localWorkSize[i];
  if (prod > maxWGSize)
      return 0;
  else
      return 1;
}

// This method computes the count of sub-groups accommodated
// by a given kernel for requested work-group size.
// For sub-groups emulation this query returns 'one' if a kernel can execute a requested work-group size and 'zero' otherwise.
uint __attribute__((overloadable)) __attribute__((always_inline))
  get_kernel_sub_group_count_for_ndrange(const ndrange_t ndrange,
                                         void(^block)(local void *, ...)) {
  uint maxWGSize = get_kernel_work_group_size(block);
  size_t prod = 1;
  for (unsigned int i = 0; i < ndrange.workDimension; ++i)
      prod *= ndrange.localWorkSize[i];
  if (prod > maxWGSize)
      return 0;
  else
      return 1;
}

// This method returns product of local sizes
// (aka work-group size) in all dimensions specified by ndrange argument.
// If the work-group size is greater than maximum possible for a given kernel then result is zero.
uint __attribute__((overloadable)) __attribute__((always_inline))
  get_kernel_max_sub_group_size_for_ndrange(const ndrange_t ndrange,
                                            void(^block)(void)) {
  uint maxWGSize = get_kernel_work_group_size(block);
  size_t prod = 1;
  for (unsigned int i = 0; i < ndrange.workDimension; ++i)
      prod *= ndrange.localWorkSize[i];
  if (prod > maxWGSize)
      return 0;
  else
      return prod;
}

// This method returns product of local sizes
// (aka work-group size) in all dimensions specified by ndrange argument.
// If the work-group size is greater than maximum possible for a given kernel then result is zero.
uint __attribute__((overloadable)) __attribute__((always_inline))
  get_kernel_max_sub_group_size_for_ndrange(const ndrange_t ndrange,
                                            void(^block)(local void *, ...)) {
  uint maxWGSize = get_kernel_work_group_size(block);
  size_t prod = 1;
  for (unsigned int i = 0; i < ndrange.workDimension; ++i)
       prod *= ndrange.localWorkSize[i];
  if (prod > maxWGSize)
      return 0;
  else
      return prod;
}
////////// - retain_event, release_event, create_user_event, set_user_event_status, capture_event_profiling_info, is_valid_event
extern void ocl20_retain_event(clk_event_t event, void *DCM);
void __attribute__((always_inline)) __attribute__((overloadable))
    retain_event(clk_event_t event) {
  void *DCM = __get_device_command_manager();
  return ocl20_retain_event(event, DCM);
}

extern void ocl20_release_event(clk_event_t event, void *DCM);
void __attribute__((always_inline)) __attribute__((overloadable))
    release_event(clk_event_t event) {
  void* DCM = __get_device_command_manager();
  return ocl20_release_event(event, DCM);
}

extern clk_event_t ocl20_create_user_event(void* DCM);
clk_event_t __attribute__((always_inline)) __attribute((overloadable))
    create_user_event() {
  void* DCM = __get_device_command_manager();
  return ocl20_create_user_event(DCM);
}

extern void ocl20_set_user_event_status(clk_event_t event, uint status,
                                        void *DCM);
void __attribute__((always_inline)) __attribute__((overloadable))
    set_user_event_status(clk_event_t event, int status) {
  void *DCM = __get_device_command_manager();
  return ocl20_set_user_event_status(event, status, DCM);
}

extern void ocl20_capture_event_profiling_info(clk_event_t event,
                                               clk_profiling_info name,
                                               global ulong *value, void *DCM);
void __attribute__((always_inline)) __attribute__((overloadable))
    capture_event_profiling_info(clk_event_t event, clk_profiling_info name,
                                 __global ulong *value) {
  void *DCM = __get_device_command_manager();
  return ocl20_capture_event_profiling_info(event, name, value, DCM);
}
void __attribute__((always_inline)) __attribute__((overloadable))
    capture_event_profiling_info(clk_event_t event, clk_profiling_info name,
                                 __global void *value) {
  void *DCM = __get_device_command_manager();
  return ocl20_capture_event_profiling_info(event, name, (__global ulong*)value, DCM);
}

extern bool ocl20_is_valid_event(clk_event_t event, void* DCM);
bool __attribute__((overloadable)) __attribute__((always_inline)) is_valid_event(clk_event_t event) {
  void* DCM = __get_device_command_manager();
  return ocl20_is_valid_event(event, DCM);
}

////////// - get_kernel_work_group_size
extern uint __attribute__((const))
ocl20_get_kernel_wg_size(private void *block, void *DCM, void *B2K);
uint __attribute__((overloadable)) __attribute__((always_inline))
    __attribute__((const)) get_kernel_work_group_size(void (^block)(void)) {
  void* DCM = __get_device_command_manager();
  void* B2K = __get_block_to_kernel_mapper();
  return ocl20_get_kernel_wg_size(block, DCM, B2K);
}
uint __attribute__((overloadable)) __attribute__((always_inline))
    __attribute__((const)) get_kernel_work_group_size(void (^block)(local void *, ...)) {
  void* DCM = __get_device_command_manager();
  void* B2K = __get_block_to_kernel_mapper();
  return ocl20_get_kernel_wg_size(block, DCM, B2K);
}

////////// - get_kernel_preferred_work_group_size_multiple
extern uint ocl20_get_kernel_preferred_wg_size_multiple(private void *block, void *DCM,
                                                        void *B2K);
uint __attribute__((overloadable)) __attribute__((always_inline))
    get_kernel_preferred_work_group_size_multiple(void (^block)(void)) {
  void* DCM = __get_device_command_manager();
  void* B2K = __get_block_to_kernel_mapper();
  return ocl20_get_kernel_preferred_wg_size_multiple(block, DCM, B2K);
}
uint __attribute__((overloadable)) __attribute__((always_inline))
    get_kernel_preferred_work_group_size_multiple(void (^block)(local void *,
                                                                ...)) {
  void* DCM = __get_device_command_manager();
  void* B2K = __get_block_to_kernel_mapper();
  return ocl20_get_kernel_preferred_wg_size_multiple(block, DCM, B2K);
}

/// address space overloading for enqueue_kernel and enqueue_marker
const __global clk_event_t* __attribute__((always_inline)) __attribute__((overloadable))
    cast_to_global_const(const __generic clk_event_t* ptr) {
  return (const __global clk_event_t*)ptr;
}

__global clk_event_t* __attribute__((always_inline)) __attribute__((overloadable))
    cast_to_global(__generic clk_event_t* ptr) {
  return (__global clk_event_t*)ptr;
}

#define ADDR_SPACE_OVERLOADING(ADDR_SPACE_1ST, ADDR_SPACE_2ND)\
int __attribute__((always_inline)) __attribute__((overloadable))\
    enqueue_marker(queue_t queue, uint num_events_in_wait_list,\
                   const ADDR_SPACE_1ST clk_event_t *event_wait_list, ADDR_SPACE_2ND clk_event_t* event_ret) {\
  return enqueue_marker(queue, num_events_in_wait_list,\
                        cast_to_global_const(event_wait_list),\
                        cast_to_global(event_ret));\
}\
int __attribute__((overloadable)) __attribute__((always_inline))\
    enqueue_kernel(queue_t queue, kernel_enqueue_flags_t flags,\
                   const ndrange_t ndrange, uint num_events_in_wait_list,\
                   const ADDR_SPACE_1ST clk_event_t *event_wait_list, clk_event_t ADDR_SPACE_2ND *event_ret,\
                   void (^block)(void)) {\
  return enqueue_kernel(queue, flags, ndrange, num_events_in_wait_list,\
                        cast_to_global_const(event_wait_list),\
                        cast_to_global(event_ret), block);\
}

ADDR_SPACE_OVERLOADING(__global, __private)
ADDR_SPACE_OVERLOADING(__global, __local)
ADDR_SPACE_OVERLOADING(__private, __private)
ADDR_SPACE_OVERLOADING(__private, __local)
ADDR_SPACE_OVERLOADING(__private, __global)
ADDR_SPACE_OVERLOADING(__local, __private)
ADDR_SPACE_OVERLOADING(__local, __local)
ADDR_SPACE_OVERLOADING(__local, __global)
#undef ADDR_SPACE_OVERLOADING

#endif
