#ifndef __OPENCL20_EXT_EXEC_H
#define __OPENCL20_EXT_EXEC_H

#ifndef LLVM_BACKEND_NOINLINE_PRE
#error define the LLVM_BACKEND_NOINLINE_PRE macro before #including this file
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// Declaration section
/// !!!NOTE: Callbacks should have the same argument order and same return type as original BI
/// If it's not, then implement passing of arguments to callback for specified function
/////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief call back for get default queue
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE queue_t
ocl20_get_default_queue(IDeviceCommandManager *);

/// @brief callback for
///  int enqueue_kernel (queue_t queue, kernel_enqueue_flags_t flags,
///                     const cl_work_description_type ndrange,void(^block)(void))
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
ocl20_enqueue_kernel_basic(queue_t queue, kernel_enqueue_flags_t flags,
                           _ndrange_t *ndrange, void *block,
                           IDeviceCommandManager *,
                           const IBlockToKernelMapper *, void *RuntimeHandle);

/// @brief callback for
///  int enqueue_kernel (
///     queue_t queue,
///     kernel_enqueue_flags_t flags,
///     const ndrange_t ndrange,
///     uint num_events_in_wait_list,
///     const clk_event_t*event_wait_list,
///     clk_event_t*event_ret,
///     void (^block)(void))
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
ocl20_enqueue_kernel_events(queue_t queue, kernel_enqueue_flags_t flags,
                            _ndrange_t *ndrange,
                            unsigned num_events_in_wait_list,
                            clk_event_t *in_wait_list, clk_event_t *event_ret,
                            void *block, IDeviceCommandManager *DCM,
                            const IBlockToKernelMapper *, void *RuntimeHandle);

/// @brief callback for
///  int enqueue_kernel (
///     queue_t queue,
///     kernel_enqueue_flags_t flags,
///     const ndrange_t ndrange,
///     void (^block)(local void *, ...),
///     uint size0, ...)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
ocl20_enqueue_kernel_localmem(queue_t queue, kernel_enqueue_flags_t flags,
                              _ndrange_t *ndrange, void *block,
                              unsigned *localbuf_size,
                              unsigned localbuf_size_len,
                              IDeviceCommandManager *DCM,
                              const IBlockToKernelMapper *,
                              void *RuntimeHandle);

/// @brief callback for
///  int enqueue_kernel (
///     queue_t queue,
///     kernel_enqueue_flags_t flags,
///     const ndrange_t ndrange,
///     uint num_events_in_wait_list,
///     const clk_event_t*event_wait_list,
///     clk_event_t*event_ret,
///     void (^block)(local void *, ...),
///     uint size0, ...)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
ocl20_enqueue_kernel_events_localmem(
    queue_t queue, kernel_enqueue_flags_t flags, _ndrange_t *ndrange,
    unsigned num_events_in_wait_list, clk_event_t *in_wait_list,
    clk_event_t *event_ret, void *block, unsigned *localbuf_size,
    unsigned localbuf_size_len, IDeviceCommandManager *DCM,
    const IBlockToKernelMapper *, void *RuntimeHandle);

/// @brief callback for
///  int enqueue_marker (
///     queue_t queue,
///     uint num_events_in_wait_list,
///     const clk_event_t*event_wait_list,
///     clk_event_t*event_ret)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
ocl20_enqueue_marker(queue_t queue, uint32_t num_events_in_wait_list,
                     const clk_event_t *event_wait_list, clk_event_t *event_ret,
                     IDeviceCommandManager *);

/// @brief callback for
///  int retain_event (
///     clk_event_tevent)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
ocl20_retain_event(clk_event_t event, IDeviceCommandManager *);

/// @brief callback for
///  int release_event (
///     clk_event_tevent)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
ocl20_release_event(clk_event_t event, IDeviceCommandManager *);

/// @brief callback for
///  int is_valid_event(
///     clk_event_tevent)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE bool
ocl20_is_valid_event(clk_event_t, IDeviceCommandManager *);

/// @brief callback for
///  clk_event_tcreate_user_event ()
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE clk_event_t
ocl20_create_user_event(IDeviceCommandManager *);

/// @brief callback for
///  void set_user_event_status (
///     clk_event_tevent,
///     int status)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
ocl20_set_user_event_status(clk_event_t event, uint32_t status,
                            IDeviceCommandManager *);

/// @brief callback for
///  void capture_event_profiling_info (
///     clk_event_tevent,
///     clk_profiling_info name,
///     global ulong *value)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
ocl20_capture_event_profiling_info(clk_event_t event, clk_profiling_info name,
                                   uint64_t *value, IDeviceCommandManager *);

/// @brief callback for
///  uint get_kernel_work_group_size (
///     void (^block)(void))
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE uint32_t
ocl20_get_kernel_wg_size(void *block, IDeviceCommandManager *,
                         const IBlockToKernelMapper *);

/// @brief callback for
///  uint get_kernel_work_group_size (
///     void (^block)(local void *, ...))
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE uint32_t
ocl20_get_kernel_wg_size_local(void *block, IDeviceCommandManager *,
                               const IBlockToKernelMapper *);

///@brief callback for
///  uint get_kernel_preferred_work_group_size_multiple (
///     void (^block)(void))
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE uint32_t
ocl20_get_kernel_preferred_wg_size_multiple(void *block,
                                            IDeviceCommandManager *,
                                            const IBlockToKernelMapper *);

///@brief callback for
///  uint get_kernel_preferred_work_group_size_multiple (
///     void (^block)(local void *, ...))
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE uint32_t
ocl20_get_kernel_preferred_wg_size_multiple_local(void *block,
                                                  IDeviceCommandManager *,
                                                  const IBlockToKernelMapper *);

///@brief externals to get access to ndrange opaque struct
/// setters
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
__set_work_dimension(_ndrange_t * T, size_t dimension);

extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
__set_global_work_offset(_ndrange_t * T, size_t index, size_t offset);

extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
__set_global_work_size(_ndrange_t * T, size_t index, size_t size);

extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE void
__set_local_work_size(_ndrange_t * T, size_t index, size_t size);

/// getters
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE size_t
__get_work_dimension(const _ndrange_t * T);

extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE size_t
__get_global_work_offset(const _ndrange_t * T, size_t index);

extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE size_t
__get_global_work_size(const _ndrange_t * T, size_t index);

extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE size_t
__get_local_work_size(const _ndrange_t * T, size_t index);

#endif
