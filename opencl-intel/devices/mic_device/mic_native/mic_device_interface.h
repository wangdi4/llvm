
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

///////////////////////////////////////////////////////////
//
// defines internal structures for params passing between MIC host and device
//
///////////////////////////////////////////////////////////
#pragma once

#include <stdint.h>
#include <cstring>
#include <assert.h>
#include <cl_device_api.h>
#include <cl_types.h>
#include <task_executor.h>

#include <common/COITypes_common.h>

// The maximum amount of worker threads.
#define MIC_NATIVE_MAX_CORES                64
#define MIC_NATIVE_MAX_THREADS_PER_CORE     4
#define MIC_NATIVE_MAX_WORKER_THREADS       (MIC_NATIVE_MAX_CORES * MIC_NATIVE_MAX_THREADS_PER_CORE)

namespace Intel { namespace OpenCL { namespace MICDeviceNative {
class QueueOnDevice;
}}}

namespace Intel { namespace OpenCL { namespace MICDevice {

//
// NOTE: please be careful for alignments!
//

//
// copy_program_to_device
//   Buffers:
//       buffer1 - normal buffer with serialized program [IN]
//       buffer2 - normal buffer with COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT [OUT]
//   MiscData
//       input - COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT
//       output - none
//
struct COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT
{
    uint64_t uid_program_on_device;
    uint64_t required_executable_size;
    uint64_t number_of_kernels;
};

struct COPY_PROGRAM_TO_DEVICE_KERNEL_INFO
{
    uint64_t    kernel_id;
    uint64_t    device_info_ptr;
};

struct COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT
{
    uint64_t    filled_kernels;
    // array of pointers to device kernel structs with size == number_of_kernels
    // in COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT
    COPY_PROGRAM_TO_DEVICE_KERNEL_INFO device_kernel_info_pts[1];
};

#define COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT_SIZE( number_of_kernels ) \
    ( sizeof(COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT) + sizeof(COPY_PROGRAM_TO_DEVICE_KERNEL_INFO)*((number_of_kernels) - 1))

struct INIT_QUEUE_ON_DEVICE_STRUCT
{
    bool is_in_order_queue;
};

struct INIT_QUEUE_ON_DEVICE_OUTPUT_STRUCT
{
    uint64_t        device_queue_address;
    cl_dev_err_code ret_code;
};

struct command_event_struct
{
  command_event_struct() : isRegistered(false) {};

  COIEVENT cmdEvent;
  bool isRegistered;
};

struct dispatcher_data
{
    uint64_t               deviceQueuePtr;       // Pointer to a queue on device
    uint64_t               commandIdentifier;    // Command identifier provided by the Framework (Unique for each NDRange execution)
    command_event_struct   startEvent;           // Event to be signaled when command starts
    command_event_struct   endEvent;             // Event to be signaled when command completes
};

// Defines a list of parameters required for NDRange kernel launch
struct ndrange_dispatcher_data : public dispatcher_data
{
    uint64_t                kernelAddress;      // Dispatcher function pointer
    cl_uniform_kernel_args  kernelArgs;

    // Assignment classes
    void AssignWorkData(const cl_dev_cmd_param_kernel* other)
    {
        kernelArgs.WorkDim = other->work_dim;
        // we can copy with memcpy if data type size matches
        if ( sizeof(size_t) == sizeof(uint64_t) )
        {
            // Copy data in a single call, hopefully compiler will optimize
            MEMCPY_S( &kernelArgs.GlobalOffset[0], sizeof(uint64_t)*MAX_WORK_DIM*3, &(other->glb_wrk_offs[0]), sizeof(uint64_t)*MAX_WORK_DIM*3);
        } else
        {
            size_t* groupedDlobalWork[3] = {kernelArgs.GlobalOffset, kernelArgs.GlobalSize, kernelArgs.LocalSize};
            const size_t* otherGroupedDlobalWork[3] = {other->glb_wrk_offs, other->glb_wrk_size, other->lcl_wrk_size};
            for (unsigned int i = 0; i < 3; i++)
            {
                for (unsigned int j = 0; j < MAX_WORK_DIM; j++)
                {
                    groupedDlobalWork[i][j] = otherGroupedDlobalWork[i][j];
                }
            }
        }
    }

    // !!!!!!!!!!
    // TODO: Remove after moving to new BE API
    // !!!!!!!!!!
    void convertToClWorkDescriptionType(cl_work_description_type& workDesc) const
    {
        workDesc.workDimension = kernelArgs.WorkDim;
        MEMCPY_S( &workDesc.globalWorkOffset[0], sizeof(size_t)*MAX_WORK_DIM*3, &kernelArgs.GlobalOffset[0], sizeof(size_t)*MAX_WORK_DIM*3);
    }
};

struct fill_mem_obj_dispatcher_data : public dispatcher_data
{
    // Num dimensions
    uint32_t    dim_count;
    // The initial offset
    uint64_t    from_offset;
    // Pitch vector of the memory object
    uint64_t    vFromPitch[MAX_WORK_DIM-1];
    // The region to copy in each dimension
    uint64_t    vRegion[MAX_WORK_DIM];
    // The pattern size in bytes.
    uint64_t    pattern_size;
    // The pattern to copy
    char        pattern[MAX_PATTERN_SIZE];
};

#define MIC_CPU_ARCH_STR_SIZE 64

struct mic_exec_env_options {
    bool                stop_at_load;
    bool                use_affinity;
    bool                ignore_core_0;
    bool                ignore_last_core;
    bool                kernel_safe_mode;
    bool                use_vtune;
    bool                enable_itt;
    bool                trap_workers;
    bool                logger_enable;
    uint32_t            threads_per_core;
    uint32_t            num_of_cores;
    uint32_t            use_TBB_grain_size;
    uint32_t            min_work_groups_number; // recommended amount of workgroups per NDRange
    
    Intel::OpenCL::TaskExecutor::TE_CMD_LIST_PREFERRED_SCHEDULING   tbb_scheduler;
    Intel::OpenCL::TaskExecutor::TASK_SET_OPTIMIZATION              tbb_block_optimization;
    
    char mic_cpu_arch_str[MIC_CPU_ARCH_STR_SIZE];
};

//
// Device utility function
//
enum UTILITY_FUNCTION_TYPE
{
    UTILITY_MEASURE_OVERHEAD = 0,
    QUEUE_CANCEL
};

struct utility_function_queue_cancel
{
    uint64_t            queue_address;
};

struct utility_function_options {
    UTILITY_FUNCTION_TYPE request;

    union 
    {
        utility_function_queue_cancel   queue_cancel;
    } options;
};

}}}

