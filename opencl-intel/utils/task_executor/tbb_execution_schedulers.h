// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#pragma once

#include "base_command_list.h"
#include "cl_shared_ptr.h"
#include "task_executor.h"

namespace Intel {
namespace OpenCL {
namespace TaskExecutor {

class TBB_ExecutionSchedulers {
public:
  static bool
  parallel_execute(base_command_list &cmdList,
                   const Intel::OpenCL::Utils::SharedPtr<ITaskSet> &task);

private:
  // tiled, horizontal, vertical
  template <class BlockedRange, class TaskLoopBodySpecific>
  static void
  auto_executor(const size_t dimsBegin[], const size_t dimsEnd[],
                size_t grainsize,
                const Intel::OpenCL::Utils::SharedPtr<ITaskSet> &task,
                base_command_list &cmdList);

  template <class BlockedRange, class TaskLoopBodySpecific>
  static void
  affinity_executor(const size_t dimsBegin[], const size_t dimsEnd[],
                    size_t grainsize,
                    const Intel::OpenCL::Utils::SharedPtr<ITaskSet> &task,
                    base_command_list &cmdList);

  template <class BlockedRange, class TaskLoopBodySpecific>
  static void
  static_executor(const size_t dimsBegin[], const size_t dimsEnd[],
                  size_t grainsize,
                  const Intel::OpenCL::Utils::SharedPtr<ITaskSet> &task,
                  base_command_list &cmdList);

  typedef void (*ExecutorFunc)(
      const size_t dimsBegin[], const size_t dimsEnd[], size_t grainsize,
      const Intel::OpenCL::Utils::SharedPtr<ITaskSet> &task,
      base_command_list &cmdList);

  static ExecutorFunc
      auto_block_default[MAX_WORK_DIM]; // schedulers that use auto_partitioner
                                        // with default blocked_range
  static ExecutorFunc
      affinity_block_default[MAX_WORK_DIM]; // schedulers that use
                                            // affinity_partitioner with default
                                            // blocked_range
  static ExecutorFunc
      static_block_default[MAX_WORK_DIM]; // schedulers that use
                                          // static_partitioner with default
                                          // blocked_range
  static ExecutorFunc
      opencl_block_default[MAX_WORK_DIM]; // schedulers that use
                                          // opencl_partitioner with default
                                          // blocked_range

  static ExecutorFunc
      auto_block_row[MAX_WORK_DIM]; // schedulers that use auto_partitioner with
                                    // blocked_range By Row
  static ExecutorFunc
      affinity_block_row[MAX_WORK_DIM]; // schedulers that use
                                        // affinity_partitioner with
                                        // blocked_range By Row
  static ExecutorFunc
      static_block_row[MAX_WORK_DIM]; // schedulers that use static_partitioner
                                      // with blocked_range By Row
  static ExecutorFunc
      opencl_block_row[MAX_WORK_DIM]; // schedulers that use opencl_partitioner
                                      // with blocked_range By Row

  static ExecutorFunc
      auto_block_column[MAX_WORK_DIM]; // schedulers that use auto_partitioner
                                       // with blocked_range By Column
  static ExecutorFunc
      affinity_block_column[MAX_WORK_DIM]; // schedulers that use
                                           // affinity_partitioner with
                                           // blocked_range By Column
  static ExecutorFunc
      static_block_column[MAX_WORK_DIM]; // schedulers that use
                                         // static_partitioner with
                                         // blocked_range By Column
  static ExecutorFunc
      opencl_block_column[MAX_WORK_DIM]; // schedulers that use
                                         // opencl_partitioner with
                                         // blocked_range By Column

  static ExecutorFunc
      auto_block_tile[MAX_WORK_DIM]; // schedulers that use auto_partitioner
                                     // with blocked_range By Tile
  static ExecutorFunc
      affinity_block_tile[MAX_WORK_DIM]; // schedulers that use
                                         // affinity_partitioner with
                                         // blocked_range By Tile
  static ExecutorFunc
      static_block_tile[MAX_WORK_DIM]; // schedulers that use static_partitioner
                                       // with blocked_range By Tile
  static ExecutorFunc
      opencl_block_tile[MAX_WORK_DIM]; // schedulers that use opencl_partitioner
                                       // with blocked_range By Tile

  static ExecutorFunc *g_executor[TE_CMD_LIST_PREFERRED_SCHEDULING_LAST]
                                 [TASK_SET_OPTIMIZE_BY_LAST];
};

} // namespace TaskExecutor
} // namespace OpenCL
} // namespace Intel
