// Copyright (c) 2006-2013 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

/////////////////////////////////////////////////////////////
//  ExecutionTask.h
//  Implementation of the Class ExecutionTask
//  Class Object is responsible on execution of NDRange task
/////////////////////////////////////////////////////////////

#pragma once

#include "task_executor.h"
#include "base_command_list.h"

#include <cl_shared_ptr.h>
#include <tbb/tbb.h>

namespace Intel { namespace OpenCL { namespace TaskExecutor {

class TBB_ExecutionSchedulers 
{
public:
    static bool parallel_execute(
         base_command_list&                                cmdList,
         const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task );

private:

    // specific execution methods
    template <class BlockedRange, class TaskLoopBodySpecific>
    static void opencl_executor( 
        const size_t                                      dims[],
        size_t                                            grainsize,
        const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task,
        base_command_list&                                cmdList );

    // tiled, horizontal, vertical
    template <class BlockedRange, class TaskLoopBodySpecific>        
    static void auto_executor(
        const size_t                                      dims[],
        size_t                                            grainsize,
        const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task,
        base_command_list&                                cmdList );
    
    template <class BlockedRange, class TaskLoopBodySpecific>        
    static void affinity_executor( 
        const size_t                                      dims[],
        size_t                                            grainsize,
        const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task,
        base_command_list&                                cmdList );

    typedef void (*ExecutorFunc)( 
        const size_t                                      dims[],
        size_t                                            grainsize,
        const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task,
        base_command_list&                                cmdList );
    
    static ExecutorFunc auto_block_default[MAX_WORK_DIM];    // schedulers that use auto_partitioner with default blocked_range
    static ExecutorFunc affinity_block_default[MAX_WORK_DIM];// schedulers that use affinity_partitioner with default blocked_range
    static ExecutorFunc opencl_block_default[MAX_WORK_DIM];  // schedulers that use opencl_partitioner with default blocked_range

    static ExecutorFunc auto_block_row[MAX_WORK_DIM];        // schedulers that use auto_partitioner with blocked_range By Row
    static ExecutorFunc affinity_block_row[MAX_WORK_DIM];    // schedulers that use affinity_partitioner with blocked_range By Row
    static ExecutorFunc opencl_block_row[MAX_WORK_DIM];      // schedulers that use opencl_partitioner with blocked_range By Row

    static ExecutorFunc auto_block_column[MAX_WORK_DIM];     // schedulers that use auto_partitioner with blocked_range By Column
    static ExecutorFunc affinity_block_column[MAX_WORK_DIM]; // schedulers that use affinity_partitioner with blocked_range By Column
    static ExecutorFunc opencl_block_column[MAX_WORK_DIM];   // schedulers that use opencl_partitioner with blocked_range By Column

    static ExecutorFunc auto_block_tile[MAX_WORK_DIM];       // schedulers that use auto_partitioner with blocked_range By Tile
    static ExecutorFunc affinity_block_tile[MAX_WORK_DIM];   // schedulers that use affinity_partitioner with blocked_range By Tile
    static ExecutorFunc opencl_block_tile[MAX_WORK_DIM];     // schedulers that use opencl_partitioner with blocked_range By Tile
    
    static ExecutorFunc* g_executor[TE_CMD_LIST_PREFERRED_SCHEDULING_LAST][TASK_SET_OPTIMIZE_BY_LAST];
    
};


}}}
