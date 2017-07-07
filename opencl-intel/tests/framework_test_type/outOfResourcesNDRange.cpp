#include <stdio.h>

#include "CL/cl.h"
#include "cl_types.h"

#include <gtest/gtest.h>
#define PROVISIONAL_MALLOC_SIZE 100
#include "cl_provisional.h"
#include "cl_sys_defines.h"
#include "TestsHelpClasses.h"

#include "FrameworkTest.h"

extern cl_device_type gDeviceType;

const char *g_programSrc = 
    "__kernel void evenBytes(__global const uchar16 *inputBuffer,  __global uchar8 *outputBuffer,\
    const uint bufferWidth,\
    const uint inputBufferStride,\
    const uint outputBufferStride)\n\
    {\n\
        local uint local_dummy[SIZE_OF_LOCAL];\n\
        uint private_dummy[SIZE_OF_PRIVATE];\n\
        private_dummy[get_local_id(0)] = (uint)get_global_id(0);\n\
        local_dummy[get_local_id(0)] = (uint)get_global_id(0);\n\
        uint xCoordinate = get_global_id(0) % bufferWidth;\n\
        uint yCoordinate = get_global_id(0) / bufferWidth;\n\
        uchar16 data = inputBuffer[xCoordinate + yCoordinate * inputBufferStride];\n\
        uchar8 result = data.even;\n\
        outputBuffer[xCoordinate + yCoordinate * outputBufferStride] = result;\n\
        barrier(CLK_LOCAL_MEM_FENCE);\n\
        // Use private and local buffers, to prevent their prunnig out. Just make sure it never happens :)\n\
        if(get_local_id(0) == get_local_size(0) + 1){\n\
            outputBuffer[0] =  private_dummy[0];\n\
            outputBuffer[1] =  local_dummy[0];\n\
        }\n\
    }";


cl_program buildProgram(cl_context clContext, cl_device_id clDevice, const char *programSrc, cl_ulong localSize, cl_ulong privateSize, cl_int &buildStatus)
{
    // Create program with source
    size_t srcLen = strlen(programSrc);
    cl_program clProgram = clCreateProgramWithSource(clContext, 1, &programSrc, &srcLen, NULL);
    if (clProgram == NULL)
        return NULL;

    char buildOptions[1024];
    SPRINTF_S(buildOptions, 1024, "-cl-opt-disable -DSIZE_OF_LOCAL=%ld -DSIZE_OF_PRIVATE=%ld", localSize, privateSize);

    printf("Build options <%s>, source:%s\n", buildOptions, programSrc);
    // Build program executable from source. Prevent any optimizations (we want big memory etc.)
    buildStatus = clBuildProgram(clProgram, 1, &clDevice, buildOptions, NULL, NULL);
    if ( buildStatus != CL_SUCCESS )
    {
        char buildLog[1024];
        clGetProgramBuildInfo(clProgram, clDevice, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);
        printf("Build Failed, log:\n %s\n", buildLog);
    }

    // If error occured
    if (CL_SUCCESS != buildStatus && NULL != clProgram)
    {
        clReleaseProgram(clProgram);
        clProgram = NULL;
    }

    return clProgram;
}


cl_kernel buildAndSetKernelArgs(_PROVISONAL_MallocArray_t &PROV_ARRAY_NAME, cl_int &iRet,
    cl_program &clProgram, unsigned int inputWidth, unsigned int outputWidth, cl_mem &inputBuffer, cl_mem &outputBuffer)
{
    // Create kernel objects
    cl_kernel clKernel = clCreateKernel(clProgram, "evenBytes", &iRet);
    if (CL_SUCCESS != iRet) return NULL;
    //ASSERT_EQ(CL_SUCCESS, iRet) << "Kernel object creation error";

    // Set kernel args
    clSetKernelArg(clKernel, 0, sizeof(cl_mem), &inputBuffer);
    clSetKernelArg(clKernel, 1, sizeof(cl_mem), &outputBuffer);

    cl_uint bufferWidth;

    bufferWidth = inputWidth / 16;
    clSetKernelArg(clKernel, 2, sizeof(cl_uint), &bufferWidth);
    clSetKernelArg(clKernel, 3, sizeof(cl_uint), &bufferWidth);
    bufferWidth = outputWidth / 8;
    clSetKernelArg(clKernel, 4, sizeof(cl_uint), &bufferWidth);
    
    return clKernel;
}


TEST_F(BaseProvisionalTest, OutOfResourcesNDRange)
{
    printf("=============================================================\n");
    printf("clEnqeueNDRange exceeding local and private memory.\n");
    printf("=============================================================\n");
    cl_int iRet = 0;

    cl_platform_id platform = 0;
    cl_device_id clDefaultDeviceId;

    iRet = clGetPlatformIDs(1, &platform, NULL);
    ASSERT_EQ(CL_SUCCESS, iRet) << "Failed getting Platform IDs";

    cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

    cl_context context = PROV_OBJ( clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet) );
    ASSERT_EQ(CL_SUCCESS, iRet) << "Failed creating context";

    iRet = clGetDeviceIDs(platform, gDeviceType, 1, &clDefaultDeviceId, NULL);
    ASSERT_EQ(CL_SUCCESS, iRet) << "Failed getting device IDs";

    cl_command_queue queue = PROV_OBJ( clCreateCommandQueue (context, clDefaultDeviceId, 0 /*no properties*/, &iRet) );
    ASSERT_EQ(CL_SUCCESS, iRet) << "Failed creating command queue";

    // Create input and output buffers
    unsigned int inputWidth = (1280 + 512) * 2;
    unsigned int outputWidth = (1280 + 512);
    unsigned int height = 720;
    cl_mem inputBuffer = NULL;
    cl_mem outputBuffer = NULL;
    
    inputBuffer =  PROV_OBJ( clCreateBuffer(context, CL_MEM_READ_ONLY, inputWidth * height * sizeof(char), NULL, &iRet) );
    ASSERT_EQ(CL_SUCCESS, iRet) << "Failed creating input buffer";
    outputBuffer = PROV_OBJ( clCreateBuffer(context, CL_MEM_WRITE_ONLY, outputWidth * height * sizeof(char), NULL, &iRet) );
    ASSERT_EQ(CL_SUCCESS, iRet) << "Failed creating output buffer";

    cl_ulong sizeOfLocal = 256;
    cl_ulong sizeOfPrivate = 256;

    // Build CL Program
    cl_program clProgram = PROV_OBJ( buildProgram(context, clDefaultDeviceId, g_programSrc, sizeOfLocal, sizeOfPrivate, iRet) );
    ASSERT_EQ(CL_SUCCESS, iRet) << "Program build error";

    // Fill input buffer with test data
    char *bufferMap = (char*)clEnqueueMapBuffer(queue, inputBuffer, CL_TRUE, CL_MAP_WRITE, 0, inputWidth * height * sizeof(char), 0, NULL, NULL, NULL);
    for (unsigned int i = 0; i < inputWidth * height;)
    {
        bufferMap[i++] = (char)0x23;
        bufferMap[i++] = (char)0xAB;
    }

    cl_event bufferReadyEvent = NULL;
    clEnqueueUnmapMemObject(queue, inputBuffer, bufferMap, 0, NULL, &bufferReadyEvent);

    // Create kernel object, and set values
    cl_kernel clKernel = PROV_OBJ( buildAndSetKernelArgs(PROV_ARRAY_NAME, iRet, clProgram, inputWidth, outputWidth, inputBuffer, outputBuffer) );
    ASSERT_EQ(CL_SUCCESS, iRet) << "Kernel object creation error";

    // Get maximum number of WG items
    size_t maxNumOfWGItemsPerDvice = 0;
    clGetDeviceInfo(clDefaultDeviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &maxNumOfWGItemsPerDvice, NULL);

    //size_t kernelPrivateSize = 0;
    //clGetKernelWorkGroupInfo(clKernel, clDefaultDeviceId, CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(size_t), &kernelPrivateSize, NULL);

    size_t maxNumOfWGItems = 0;
    clGetKernelWorkGroupInfo(clKernel, clDefaultDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &maxNumOfWGItems, NULL);

    size_t globalSize[] = {inputWidth / 16 * height};
    size_t localSize[1];

    // NDRange local size should maintain 2 conditions:
    // 1- should be bounded by both local_dummy and private_dummy sizes (i.e. sizeOfLocal and sizeOfPrivate)
    localSize[0] = MIN(MIN((cl_ulong)maxNumOfWGItems, sizeOfLocal), sizeOfPrivate);
    // 2- should divide the global size
    while (globalSize[0] % localSize[0] != 0)
    {
        localSize[0]--;
    }

    cl_event kernelReadyEvent[1];

    // Execute kernel
    iRet = clEnqueueNDRangeKernel(queue, clKernel, 1, NULL, globalSize, localSize, 1, &bufferReadyEvent, kernelReadyEvent);
    ASSERT_EQ(CL_SUCCESS, iRet) << "Failed enqueueing NDRange for basic kernel.";

    // Check results
    bufferMap = (char*)clEnqueueMapBuffer(queue, outputBuffer, CL_TRUE, CL_MAP_READ, 0, outputWidth * height * sizeof(char), 1, kernelReadyEvent, NULL, NULL);
    for (unsigned int i = 0; i < outputWidth * height; i++)
    {
        ASSERT_EQ(0x23, bufferMap[i]) << "WRONG RESULTS!!! The basic functionality of the kernel is bad.";
    }
    clEnqueueUnmapMemObject(queue, outputBuffer, bufferMap, 0, NULL, NULL);


    /**
     ******************************************************************************
     * The next program uses HUGE private memory for WG items.
     */
    sizeOfLocal = 256;
    sizeOfPrivate = 8192;

    cl_program clProgramThatShouldFailPrivateMem = PROV_OBJ( buildProgram(context, clDefaultDeviceId, g_programSrc, sizeOfLocal, sizeOfPrivate, iRet) );
    ASSERT_EQ(CL_SUCCESS, iRet) << "wrongfully failed to build a program with huge private memory.";

    // Create kernel object, and set values
    cl_kernel clKernelShouldFailPrivateMem = PROV_OBJ( buildAndSetKernelArgs(PROV_ARRAY_NAME, iRet, clProgramThatShouldFailPrivateMem, inputWidth, outputWidth, inputBuffer, outputBuffer) );
    ASSERT_EQ(CL_SUCCESS, iRet) << "Kernel object, for huge private memory, creation error";

    //clGetKernelWorkGroupInfo(clKernelShouldFailPrivateMem, clDefaultDeviceId, CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(size_t), &kernelPrivateSize, NULL);

    // Get maximum number of WG items
    maxNumOfWGItems = 0;
    clGetKernelWorkGroupInfo(clKernelShouldFailPrivateMem, clDefaultDeviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &maxNumOfWGItems, NULL);

    size_t numOfWorkGroupItems = maxNumOfWGItems;
    while (0 != globalSize[0] % numOfWorkGroupItems)
    {
        numOfWorkGroupItems--;
    }

    ASSERT_NE(0, numOfWorkGroupItems) << "Test cannot run (fail) properly because there is no good number for WG items.";

    // In this case the test will simply not run and test the right error.
    // To fix this error you should adjust the size of the HUGE kernel private memory.
    // We assume 8MB (or less) assigned for WG memory.
    ASSERT_GT(maxNumOfWGItemsPerDvice/2, numOfWorkGroupItems) << 
        "Test cannot run (fail) properly because it will exceed device limits, and not kernel limits.";

    localSize[0] = numOfWorkGroupItems * 2;
    while (localSize[0] <= maxNumOfWGItems)
    {
        localSize[0] += numOfWorkGroupItems;
    }

    iRet = clEnqueueNDRangeKernel(queue, clKernelShouldFailPrivateMem, 1, NULL, globalSize, localSize, 1, &bufferReadyEvent, kernelReadyEvent);
    ASSERT_EQ(CL_OUT_OF_RESOURCES, iRet) << "Enqueueing NDRange for a bigger than allowed WG size (on HUGE private mem) erronously succeeded.";


    /**
     ******************************************************************************
     * The next program uses HUGE local memory.
     * TODO
     */

    cl_ulong maxLocalArea = 0;
    clGetDeviceInfo(clDefaultDeviceId, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &maxLocalArea, NULL);

    sizeOfLocal = maxLocalArea*2;
    sizeOfPrivate = 256;

    cl_program clProgramThatShouldFailLocalMem = PROV_OBJ( buildProgram(context, clDefaultDeviceId, g_programSrc, sizeOfLocal, sizeOfPrivate, iRet) );
    ASSERT_EQ(CL_SUCCESS, iRet) << "wrongfully failed to build a program with huge local memory.";

    // Create kernel object, and set values
    cl_kernel clKernelShouldFailLocalMem = PROV_OBJ( buildAndSetKernelArgs(PROV_ARRAY_NAME, iRet, clProgramThatShouldFailLocalMem, inputWidth, outputWidth, inputBuffer, outputBuffer) );
    ASSERT_EQ(CL_SUCCESS, iRet) << "Kernel object, for huge private memory, creation error";

    iRet = clEnqueueNDRangeKernel(queue, clKernelShouldFailLocalMem, 1, NULL, globalSize, NULL, 1, &bufferReadyEvent, kernelReadyEvent);
    ASSERT_EQ(CL_OUT_OF_RESOURCES, iRet) << "Enqueueing NDRange for a bigger than allowed (HUGE) local mem erronously succeeded.";

    //Release resources
    clReleaseEvent(bufferReadyEvent);
    clReleaseEvent(kernelReadyEvent[0]);
    if (inputBuffer)
    {
        clReleaseMemObject(inputBuffer);
    }
    if (outputBuffer)
    {
        clReleaseMemObject(outputBuffer);
    }
    if (clKernel)
    {
        clReleaseKernel(clKernel);
    }
    if (clKernelShouldFailPrivateMem)
    {
        clReleaseKernel(clKernelShouldFailPrivateMem);
    }
    if (clKernelShouldFailLocalMem)
    {
        clReleaseKernel(clKernelShouldFailLocalMem);
    }
    if (clProgram)
    {
        clReleaseProgram(clProgram);
    }
    if (clProgramThatShouldFailPrivateMem)
    {
        clReleaseProgram(clProgramThatShouldFailPrivateMem);
    }
    if (clProgramThatShouldFailLocalMem)
    {
        clReleaseProgram(clProgramThatShouldFailLocalMem);
    }
    if (queue)
    {
        clReleaseCommandQueue(queue);
    }
    if (context)
    {
        clReleaseContext(context);
    }
}
