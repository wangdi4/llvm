// Copyright 2021 Intel Corporation.
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

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "IBlockToKernelMapper.h"
#include "ICLDevBackendServiceFactory.h"
#include "IDeviceCommandManager.h"
#include "cl_types.h"

#define LLVM_BACKEND_NOINLINE_PRE
#include "opencl20_ext_execution.h"
#include "opencl_task_sequence.h"
#undef LLVM_BACKEND_NOINLINE_PRE

#define DEBUG_TYPE "opencl_task_sequence"

extern "C" LLVM_BACKEND_API size_t
__ocl_task_sequence_create(size_t ret_type_size) {
  // For each task_sequence object, other essential data is stored in
  // task_sequence_data along with it.
  task_sequence_data *data = new task_sequence_data;
  // No results of async task are returned at the beginning time.
  data->delivered = 0;
  // Save the size of every single result value. This is required because we
  // need this to calculate the address of each result of async task.
  data->result_size = ret_type_size;
  // Return the address of task_sequence_data as id of task_sequence object.
  return reinterpret_cast<size_t>(data);
}

extern "C" LLVM_BACKEND_API void
__ocl_task_sequence_async(task_sequence *obj, unsigned invocation_capacity,
                          void *block_invoke, void *block_literal,
                          IDeviceCommandManager *DCM, IBlockToKernelMapper *B2K,
                          void *RuntimeHandle) {
  task_sequence_data *data = reinterpret_cast<task_sequence_data *>(obj->id);
  assert(data && "data of task_sequence lossed!");
  // All tasks are enqueued into the same device queue. The sequence of async
  // tasks in the same task_sequence is guaranteed by setting the wait event for
  // each task when it is enqueued.
  queue_t queue = DCM->GetTaskSeqQueueForDevice();

  // Don't wait for the parent kernel to finish execution.
  kernel_enqueue_flags_t flags = CLK_ENQUEUE_FLAGS_NO_WAIT;

  // Construct a single work-item ndrange for each task.
  _ndrange_t ndrange;
  ndrange.workDimension = 1;
  ndrange.globalWorkOffset[0] = 0;
  ndrange.globalWorkSize[0] = 1;
  ndrange.localWorkSize[0] = 0;

  // If current task is not the first task of this task_sequence, it can't start
  // to execute until previous task in the same task_sequence finishes
  // execution.
  bool is_first_async = (obj->outstanding == 1);
  uint32_t num_events_in_wait_list = is_first_async ? 0 : 1;
  // Get the event of previous task.
  clk_event_t *event_wait_list =
      is_first_async ? nullptr : &data->events.back();

  if (is_first_async) {
    data->results.reserve(invocation_capacity);
    data->events.reserve(invocation_capacity);
  }

  // Fill the buffer address in which result is expected to be stored to block
  // literal. The layout of block_literal is:
  //
  //   struct block_literal {
  //     unsigned size;
  //     unsigned align;
  //     func_ptr *invoke;
  //     {
  //       block_arguments_and_captures...;
  //       void *ret;
  //     }
  //   }
  unsigned block_literal_size = *reinterpret_cast<unsigned *>(block_literal);
  char *ret = reinterpret_cast<char *>(block_literal) +
              (block_literal_size - sizeof(void *));
  char **addr = reinterpret_cast<char **>(ret);
  *addr = (char *)malloc(data->result_size);
  assert(*addr && "Out of memory!");

  clk_event_t event_ret;
  int err = __ocl20_enqueue_kernel_events(
      queue, flags, &ndrange, num_events_in_wait_list, event_wait_list,
      &event_ret, block_invoke, block_literal, DCM, B2K, RuntimeHandle);
  (void)err;
  LLVM_DEBUG(dbgs() << "__ocl_task_sequence_async. Return value " << err
                    << "\n");

  // Keep the handles of result and event.
  data->results.push_back(*addr);
  data->events.push_back(event_ret);
}

extern "C" LLVM_BACKEND_API void *
__ocl_task_sequence_get(task_sequence *obj, IDeviceCommandManager *DCM) {
  task_sequence_data *data = reinterpret_cast<task_sequence_data *>(obj->id);
  assert(data && "data of task_sequence lossed!");

  // Wait for task to finish execution.
  clk_event_t current_event = data->events[data->delivered];
  int err = DCM->WaitForEvents(1, &current_event);
  (void)err;
  LLVM_DEBUG(dbgs() << "__ocl_task_sequence_get. Return value " << err << "\n");

  __ocl20_release_event(current_event, DCM);
  data->delivered++;

  return reinterpret_cast<void *>(data->results[data->delivered - 1]);
}

extern "C" LLVM_BACKEND_API void
__ocl_task_sequence_release(task_sequence *obj) {
  task_sequence_data *data = reinterpret_cast<task_sequence_data *>(obj->id);
  assert(data && "data of task_sequence lossed!");
  for (auto *result : data->results)
    free(result);
  delete data;
}
