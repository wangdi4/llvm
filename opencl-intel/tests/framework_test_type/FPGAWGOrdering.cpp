#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include <vector>
#include "FrameworkTest.h"
#include <gtest/gtest.h>

#ifdef BUILD_FPGA_EMULATOR
extern cl_device_type gDeviceType;

TEST(FPGA, FPGAWorkItemsOrdering)
{
    cl_int         iRet     = CL_SUCCESS;
    cl_platform_id platform = nullptr;
    cl_device_id   device   = nullptr;

    //Get platform.
    iRet = clGetPlatformIDs(/*num_entries=*/1, &platform,
                            /*num_platforms=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetPlatformIDs failed.";

    //Get device.
    iRet = clGetDeviceIDs(platform, gDeviceType, /*num_entries=*/1,
                          &device, /*num_devices=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << " clGetDeviceIDs failed on trying to obtain "
        << gDeviceType << " device type.";

    //Create context.
    const cl_context_properties prop[5] = { CL_CONTEXT_PLATFORM,
                                            (cl_context_properties)platform,
                                            CL_CONTEXT_FPGA_EMULATOR_INTEL,
                                            CL_TRUE,
                                            0 };
    cl_context context = clCreateContext(prop, /*num_devices=*/1, &device,
                                         /*pfn_notify=*/nullptr,
                                         /*user_data=*/nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateContext failed.";

    const char* kernel = "\
        #pragma OPENCL EXTENSION cl_intel_channels : enable                  \n\
                                                                             \n\
        atomic_int counter = ATOMIC_VAR_INIT(0);                             \n\
        channel int ch;                                                      \n\
                                                                             \n\
        __kernel void channel_writer() {                                     \n\
            size_t gid = get_global_linear_id();                             \n\
            write_channel_intel(ch, gid);                                    \n\
        }                                                                    \n\
                                                                             \n\
        __kernel void channel_reader(__global int *out_buf) {                \n\
            int ind = atomic_fetch_add(&counter, 1);                         \n\
            out_buf[ind] = read_channel_intel(ch);                           \n\
        }                                                                    \n\
        ";
    cl_program program = clCreateProgramWithSource(context, /*count=*/1,
                                                   &kernel, /*length=*/nullptr,
                                                   &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed.";

    iRet = clBuildProgram(program, /*num_devices=*/0, /*device_list=*/nullptr,
                          /*options=*/"-cl-std=CL2.0", /*pfn_notify*/nullptr,
                          /*user_data=*/nullptr);
    EXPECT_EQ(CL_SUCCESS, iRet) << "clBuildProgram failed.";
    if(CL_SUCCESS != iRet)
    {
        size_t logSize = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              /*param_value_size=*/0, /*param_value=*/nullptr,
                              &logSize);
        std::string log("", logSize);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              log.size(), &log[0], /*value_size_ret=*/nullptr);
        FAIL() << log << "\n";
        return;
    }

    cl_kernel kernelWriter = clCreateKernel(program, "channel_writer", &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateKernel failed.";

    cl_kernel kernelReader = clCreateKernel(program, "channel_reader", &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateKernel failed.";

    const size_t numDims = 3;
    size_t globalSize[numDims] = { 32, 16, 8 };
    size_t localSize[numDims] = { 32, 16, 8 };
    size_t numElements =
        globalSize[0] * globalSize[1] * globalSize[2];
    size_t outBufferSize = sizeof(cl_int) * numElements;
    std::vector<cl_int> outBufferHost;
    outBufferHost.resize(numElements);
    cl_mem outBuffer = clCreateBuffer(
        context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
        /*size=*/outBufferSize, /*host_ptr=*/&outBufferHost[0], &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateBuffer failed.";

    cl_command_queue queueWriter =
        clCreateCommandQueueWithProperties(context, device,
                                           /*properties=*/nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateCommandQueueWithProperties failed.";

    cl_command_queue queueReader =
        clCreateCommandQueueWithProperties(context, device,
                                           /*properties=*/nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateCommandQueueWithProperties failed.";

    iRet = clSetKernelArg(kernelReader, 0, sizeof(outBuffer), &outBuffer);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clSetKernelArg 0 failed.";

    iRet = clEnqueueNDRangeKernel(queueWriter, kernelWriter, /*work_dim=*/numDims,
                                  /*global_work_offset=*/nullptr,
                                  globalSize, localSize,
                                  /*num_events_in_wait_list=*/0,
                                  /*event_wait_list=*/nullptr,
                                  /*event=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clEnqueueNDRangeKernel failed.";

    iRet = clEnqueueNDRangeKernel(queueReader, kernelReader, /*work_dim=*/numDims,
                                  /*global_work_offset=*/nullptr,
                                  globalSize, localSize,
                                  /*num_events_in_wait_list=*/0,
                                  /*event_wait_list=*/nullptr,
                                  /*event=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clEnqueueNDRangeKernel failed.";

    clFinish(queueWriter);
    clFinish(queueReader);

    for (size_t gid = 0; gid < numElements; ++gid)
    {
        ASSERT_EQ(gid, outBufferHost[gid])
            << "Incorrect result at index " << gid;
    }

    clReleaseCommandQueue(queueWriter);
    clReleaseCommandQueue(queueReader);
    clReleaseMemObject(outBuffer);
    clReleaseKernel(kernelWriter);
    clReleaseKernel(kernelWriter);
    clReleaseProgram(program);
    clReleaseContext(context);
}
#endif // BUILD_FPGA_EMULATOR
