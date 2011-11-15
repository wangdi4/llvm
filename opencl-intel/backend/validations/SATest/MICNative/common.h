/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  common.h

\*****************************************************************************/

#ifndef SATEST_NATIVE_COMMON_H
#define SATEST_NATIVE_COMMON_H

#include <stdint.h>
#include "cl_device_api.h"
#include "cl_types.h"

// Enum of directives
enum DIRECTIVE_ID
{
    KERNEL = 0,
    BUFFER,
    PRINTF
};

struct KernelDirective
{
    uint32_t kernelNameSize;
    uint64_t offset_in_blob;
};

struct BufferDirective
{
    uint32_t bufferIndex;
    uint64_t offset_in_blob;
    cl_mem_obj_descriptor mem_obj_desc;
};

struct PrintfDirective
{
    uint32_t bufferIndex;
    uint64_t size;
};

struct DirectivePack
{
    DIRECTIVE_ID id;
    union
    {
        KernelDirective kernelDirective;
        BufferDirective bufferDirective;
        PrintfDirective printfDirective;
    };
};

struct cl_mic_work_description_type
{
    uint32_t workDimension;
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

struct DispatcherData
{
    // Dispatcher function arguments
    KernelDirective kernelDirective;
    // The buffer index of misc data
    uint32_t miscDataBuffIndex;
    cl_mic_work_description_type workDesc;
    // Pre-execution directives count
    uint32_t preExeDirectivesCount;
    // Post-execution directives count
    uint32_t postExeDirectivesCount;
    // OpenCL kernel arguments size in bytes
    uint64_t kernelArgSize;
    // offset of pre execution directives array
    uint64_t preExeDirectivesArrOffset;
    // offset of post execution directives array
    uint64_t postExeDirectivesArrOffset;
    // offset of kernel arguments blob
    uint64_t kernelArgBlobOffset;

    /* Calculate the offsets of 'preExeDirectivesArrOffset' / 'postExeDirectivesArrOffset' / 'kernelArgBlobOffset'.
    Call it only after u set the parameters - 'preExeDirectivesCount' / 'postExeDirectivesCount' */
    void calcAndSetOffsets()
    {
        preExeDirectivesArrOffset = sizeof(DispatcherData);
        postExeDirectivesArrOffset = preExeDirectivesArrOffset + (preExeDirectivesCount * sizeof(DirectivePack));
        kernelArgBlobOffset = postExeDirectivesArrOffset + (postExeDirectivesCount * sizeof(DirectivePack));
    }

    /* Return the size of the "header meta data" (this struct) plus the size of "preExeDirectivesArr" + "postExeDirectivesArr" + kernelArgSize */
    size_t getDispatcherDataSize()
    {
        return kernelArgBlobOffset + kernelArgSize;
    }
};

struct ExecutionOptions
{
    bool measurePerformance;
    bool useTraceMarks;
    bool useVTune;
    bool runSingleWG;
    uint32_t defaultLocalWGSize;
    uint32_t executeIterationsCount;
};

struct MiscData
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

enum OPTIONAL_DISPATCH_BUFFERS
{
    DISPATCHER_DATA = 0,
    MISC_DATA,
    PRINTF_BUFFER,

    AMOUNT_OF_OPTIONAL_DISPATCH_BUFFERS
};

#endif // SATEST_NATIVE_COMMON_H
