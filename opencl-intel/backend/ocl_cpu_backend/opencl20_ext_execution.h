// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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
///  int enqueue_kernel_basic (
///     queue_t queue,
///     kernel_enqueue_flags_t flags,
///     const cl_work_description_type ndrange,
///     void *block_invoke,
///     void *block_literal)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
ocl20_enqueue_kernel_basic(queue_t queue, kernel_enqueue_flags_t flags,
                           _ndrange_t *ndrange, void *block_invoke,
                           void *block_literal, IDeviceCommandManager *,
                           const IBlockToKernelMapper *, void *RuntimeHandle);

/// @brief callback for
///  int enqueue_kernel_basic_events (
///     queue_t queue,
///     kernel_enqueue_flags_t flags,
///     const ndrange_t ndrange,
///     uint num_events_in_wait_list,
///     const clk_event_t*event_wait_list,
///     clk_event_t*event_ret,
///     void *block_invoke,
///     void *block_literal)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
ocl20_enqueue_kernel_events(queue_t queue, kernel_enqueue_flags_t flags,
                            _ndrange_t *ndrange,
                            unsigned num_events_in_wait_list,
                            clk_event_t *in_wait_list, clk_event_t *event_ret,
                            void *block_invoke, void *block_literal,
                            IDeviceCommandManager *DCM,
                            const IBlockToKernelMapper *, void *RuntimeHandle);

/// @brief callback for
///  int enqueue_kernel_varargs (
///     queue_t queue,
///     kernel_enqueue_flags_t flags,
///     const ndrange_t ndrange,
///     void *block_invoke,
///     void *block_literal,
///     uint num, uint size0, ...)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
ocl20_enqueue_kernel_localmem(queue_t queue, kernel_enqueue_flags_t flags,
                              _ndrange_t *ndrange, void *block_invoke,
                              void *block_literal, unsigned localbuf_size_len,
                              size_t *localbuf_size,
                              IDeviceCommandManager *DCM,
                              const IBlockToKernelMapper *,
                              void *RuntimeHandle);

/// @brief callback for
///  int enqueue_kernel_events_varargs (
///     queue_t queue,
///     kernel_enqueue_flags_t flags,
///     const ndrange_t ndrange,
///     uint num_events_in_wait_list,
///     const clk_event_t*event_wait_list,
///     clk_event_t*event_ret,
///     void *block_invoke,
///     void *block_literal,
///     uint num, uint size0, ...)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE int
ocl20_enqueue_kernel_events_localmem(
    queue_t queue, kernel_enqueue_flags_t flags, _ndrange_t *ndrange,
    unsigned num_events_in_wait_list, clk_event_t *in_wait_list,
    clk_event_t *event_ret, void *block_invoke, void *block_literal,
    unsigned localbuf_size_len, size_t *localbuf_size,
    IDeviceCommandManager *DCM, const IBlockToKernelMapper *,
    void *RuntimeHandle);

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
///     void *block)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE uint32_t
ocl20_get_kernel_wg_size(void *block_invoke, void *block_literal,
                         IDeviceCommandManager *, const IBlockToKernelMapper *);

///@brief callback for
///  uint get_kernel_preferred_work_group_size_multiple (
///     void *block)
extern "C" LLVM_BACKEND_API LLVM_BACKEND_NOINLINE_PRE uint32_t
ocl20_get_kernel_preferred_wg_size_multiple(void *block_invoke,
                                            void *block_literal,
                                            IDeviceCommandManager *,
                                            const IBlockToKernelMapper *);
#endif
