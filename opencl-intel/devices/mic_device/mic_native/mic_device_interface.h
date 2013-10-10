
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

// Enum of directives
enum DIRECTIVE_ID
{
    KERNEL = 0,
    BUFFER,
    BARRIER,
    PRINTF
};

struct kernel_directive
{
    uint64_t kernelAddress;
};

struct buffer_directive
{
    unsigned int bufferIndex;
    uint64_t offset_in_blob;
    cl_mem_obj_descriptor mem_obj_desc;
};

struct barrier_directive
{
    COIEVENT barrier;
};

struct printf_directive
{
    unsigned int bufferIndex;
    uint64_t size;
};

struct directive_pack
{
    DIRECTIVE_ID id;
    union
    {
        kernel_directive kernelDirective;
        buffer_directive bufferDirective;
        barrier_directive barrierDirective;
        printf_directive printfDirective;
    };
};

struct cl_mic_work_description_type
{
    unsigned int workDimension;
    uint64_t globalWorkOffset[MAX_WORK_DIM];
    uint64_t globalWorkSize[MAX_WORK_DIM];
    uint64_t localWorkSize[MAX_WORK_DIM];

    cl_mic_work_description_type() {}

    cl_mic_work_description_type(const unsigned int workDim, const size_t* gWorkOffset, const size_t* gWorkSize, const size_t* lWorkSize)
    {
        setParams(workDim, gWorkOffset, gWorkSize, lWorkSize);
    }

    cl_mic_work_description_type& operator=(const cl_work_description_type& other)
    {
        setParams(other.workDimension, other.globalWorkOffset, other.globalWorkSize, other.localWorkSize);
        return *this;
    }

    // Copy the input data to this object data, CANNOT use memcpy because the change in type (size_t to uint64_t)
    void setParams(const unsigned int workDim, const size_t* gWorkOffset, const size_t* gWorkSize, const size_t* lWorkSize)
    {
        workDimension = workDim;
        uint64_t* groupedDlobalWork[3] = {globalWorkOffset, globalWorkSize, localWorkSize};
        const size_t* otherGroupedDlobalWork[3] = {gWorkOffset, gWorkSize, lWorkSize};
        for (unsigned int i = 0; i < 3; i++)
        {
            for (unsigned int j = 0; j < MAX_WORK_DIM; j++)
            {
                groupedDlobalWork[i][j] = otherGroupedDlobalWork[i][j];
            }
        }
    }

    // Copy this object data to cl_work_description_type object, CANNOT use memcpy because the change in type (uint64_t to size_t)
    void convertToClWorkDescriptionType(cl_work_description_type* outWorkDescType)
    {
        outWorkDescType->workDimension = workDimension;
        size_t* groupedGlobalWork[3] = {outWorkDescType->globalWorkOffset, outWorkDescType->globalWorkSize, outWorkDescType->localWorkSize};
        uint64_t* otherGroupedGlobalWork[3] = {globalWorkOffset, globalWorkSize, localWorkSize};
        for (unsigned int i = 0; i < 3; i++)
        {
            for (unsigned int j = 0; j < MAX_WORK_DIM; j++)
            {
                groupedGlobalWork[i][j] = otherGroupedGlobalWork[i][j];
            }
        }
    }

};

struct dispatcher_data
{
public:
    // Command identifier provided by the Framework (Unique for each NDRange execution)
    cl_dev_cmd_id commandIdentifier;
    // Flag that inform if it is inOrder execution
    bool isInOrderQueue;
    // Pre-execution directives count
    unsigned int preExeDirectivesCount;
    // Post-execution directives count
    unsigned int postExeDirectivesCount;
    // offset of pre execution directives array
    uint64_t preExeDirectivesArrOffset;
    // offset of post execution directives array
    uint64_t postExeDirectivesArrOffset;

    /* Claculate the offsets of 'preExeDirectivesArrOffset' / 'postExeDirectivesArrOffset' / other req. offsets
       Call it only after u set the parameters - 'preExeDirectivesCount' / 'postExeDirectivesCount' */
    virtual void calcAndSetOffsets() = 0;

    /* Return the size of the "header meta data" (this struct) plus the size of "preExeDirectivesArr" + "postExeDirectivesArr" + other specific struct size */
    virtual size_t getDispatcherDataSize() = 0;

    /* Copy the appropriate dispatcher data + the tail data which can include 'preExeDirectivesArr' / 'postExeDirectivesArr' + other specific struct data */
    virtual bool copyDispatcherDataWithTail(char* dst, size_t size, const directive_pack* preExeDirectivesTail, const directive_pack* postExeDirectivesTail, const char* lastTail) = 0;

protected:

    /* Calculate the offsets of 'preExeDirectivesArrOffset' / 'postExeDirectivesArrOffset'.
       Call it from child struct.
       dispatcherDataSize - the size of the child struct (which include my size) */
    void calcAndSetDirectivesArrOffsets(size_t dispatcherDataSize)
    {
        preExeDirectivesArrOffset = dispatcherDataSize;
        postExeDirectivesArrOffset = preExeDirectivesArrOffset + (preExeDirectivesCount * sizeof(directive_pack));
    }

    /* Copy the appropriate dispatcher data + the tail data which can include 'preExeDirectivesArr' / 'postExeDirectivesArr' + other specific struct data */
    void copyDispatcherDataDirectivesTail(char* dst, const directive_pack* preExeDirectivesTail, const directive_pack* postExeDirectivesTail, const char* lastTail, size_t lastTailSize)
    {
        char* tailPtr = dst + preExeDirectivesArrOffset;
        size_t coppiedSize = 0;
        size_t sizeToCopy = 0;
        // Copy the pre exe directives
        if (preExeDirectivesCount > 0)
        {
            assert(preExeDirectivesTail);
            sizeToCopy = sizeof(directive_pack) * preExeDirectivesCount;
            memcpy(tailPtr + coppiedSize, preExeDirectivesTail, sizeToCopy);
            coppiedSize += sizeToCopy;
        }
        // Copy the post exe directives
        if (postExeDirectivesCount > 0)
        {
            assert(postExeDirectivesTail);
            sizeToCopy = sizeof(directive_pack) * postExeDirectivesCount;
            memcpy(tailPtr + coppiedSize, postExeDirectivesTail, sizeToCopy);
            coppiedSize += sizeToCopy;
        }
        // Copy child specific tail data
        if (lastTailSize > 0)
        {
            assert(lastTail);
            memcpy(tailPtr + coppiedSize, lastTail, lastTailSize);
        }
    }
};

struct ndrange_dispatcher_data : public dispatcher_data
{
    // Dispatcher function arguments
    kernel_directive kernelDirective;

    cl_mic_work_description_type workDesc;

    // OpenCL kernel arguments size in bytes
    uint64_t kernelArgSize;

    // offset of kernel arguments blob
    uint64_t kernelArgBlobOffset;

    /* Calculate the offsets of 'preExeDirectivesArrOffset' / 'postExeDirectivesArrOffset' / 'kernelArgBlobOffset'.
       Call it only after u set the parameters - 'preExeDirectivesCount' / 'postExeDirectivesCount' */
    void calcAndSetOffsets()
    {
        calcAndSetDirectivesArrOffsets(sizeof(ndrange_dispatcher_data));
        kernelArgBlobOffset = postExeDirectivesArrOffset + (postExeDirectivesCount * sizeof(directive_pack));
    }

    /* Return the size of the "header meta data" (this struct) plus the size of "preExeDirectivesArr" + "postExeDirectivesArr" + kernelArgSize */
    size_t getDispatcherDataSize()
    {
        return kernelArgBlobOffset + kernelArgSize;
    }

    bool copyDispatcherDataWithTail(char* dst, size_t size, const directive_pack* preExeDirectivesTail, const directive_pack* postExeDirectivesTail, const char* lastTail)
    {
        assert(dst);
        assert(size == getDispatcherDataSize());
        if ((NULL == dst) || (size != getDispatcherDataSize())
            || ((preExeDirectivesCount > 0) && (NULL == preExeDirectivesTail))
            || ((postExeDirectivesCount > 0) && (NULL == postExeDirectivesTail))
            || ((kernelArgSize > 0) && (NULL == lastTail)))
        {
            return false;
        }
        // Copy dispatcher explicit data
        memcpy(dst, this, sizeof(ndrange_dispatcher_data));
        // Copy dispatcher implicit data
        copyDispatcherDataDirectivesTail(dst, preExeDirectivesTail,postExeDirectivesTail, lastTail, kernelArgSize);
        return true;
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
    // The pattern to copy
    char        pattern[MAX_PATTERN_SIZE];
    // The pattern size in bytes.
    uint64_t    pattern_size;

    /* Claculate the offsets of 'preExeDirectivesArrOffset' / 'postExeDirectivesArrOffset'.
       Call it only after u set the parameters - 'preExeDirectivesCount' / 'postExeDirectivesCount' */
    void calcAndSetOffsets()
    {
        calcAndSetDirectivesArrOffsets(sizeof(fill_mem_obj_dispatcher_data));
    }

    /* Return the size of the "header meta data" (this struct) plus the size of "preExeDirectivesArr" + "postExeDirectivesArr" */
    size_t getDispatcherDataSize()
    {
        return postExeDirectivesArrOffset + (postExeDirectivesCount * sizeof(directive_pack));
    }

    bool copyDispatcherDataWithTail(char* dst, size_t size, const directive_pack* preExeDirectivesTail, const directive_pack* postExeDirectivesTail, const char* lastTail)
    {
        assert(dst);
        assert(size == getDispatcherDataSize());
        if ((NULL == dst) || (size != getDispatcherDataSize())
            || ((preExeDirectivesCount > 0) && (NULL == preExeDirectivesTail))
            || ((postExeDirectivesCount > 0) && (NULL == postExeDirectivesTail)))
        {
            return false;
        }
        // Copy dispatcher explicit data
        memcpy(dst, this, sizeof(fill_mem_obj_dispatcher_data));
        // Copy dispatcher implicit data
        copyDispatcherDataDirectivesTail(dst, preExeDirectivesTail,postExeDirectivesTail, lastTail, 0);
        return true;
    }
};



struct misc_data
{
    void init()
    {
        invocationTime = 0;
        startRunningTime = 0;
        completionTime = 0;
        errCode = CL_DEV_SUCCESS;
    }
    cl_ulong invocationTime;
    cl_ulong startRunningTime;
    cl_ulong completionTime;
    cl_dev_err_code errCode;
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
    uint32_t            threads_per_core;
    uint32_t            num_of_cores;
    uint32_t            use_TBB_grain_size;
    uint32_t            min_work_groups_number; // recommended amount of workgroups per NDRange
    
    Intel::OpenCL::TaskExecutor::TE_CMD_LIST_PREFERRED_SCHEDULING   tbb_scheduler;
    Intel::OpenCL::TaskExecutor::TASK_SET_OPTIMIZATION              tbb_block_optimization;
    
    char mic_cpu_arch_str[MIC_CPU_ARCH_STR_SIZE];
};


enum OPTIONAL_DISPATCH_BUFFERS
{
    DISPATCHER_DATA = 0,
    MISC_DATA,
    PRINTF_BUFFER,

    AMOUNT_OF_OPTIONAL_DISPATCH_BUFFERS
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
    }                     options;
};

}}}

