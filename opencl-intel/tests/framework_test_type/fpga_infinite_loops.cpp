#include "CL/cl.h"

#include "FrameworkTest.h"
#include "gtest/gtest.h"

extern cl_device_type gDeviceType;

void FPGAInfiniteLoopsBasic()
{
    cl_int error = CL_SUCCESS;
    cl_platform_id platform = nullptr;
    cl_device_id device = nullptr;

    error = clGetPlatformIDs(1, &platform, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clGetPlatformIDs failed";

    error = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clGetDeviceIDs failed";

    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr,
        &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateContext failed";

    const char* sources = "\
        global atomic_int counter = ATOMIC_VAR_INIT(0);\n\
        __kernel void infinite_loop_test() {\n\
            int val = atomic_load(&counter);\n\
            while (true) {\n\
                if (val < 999) {\n\
                    val = atomic_fetch_add(&counter, 1);\n\
                }\n\
            }\n\
        }\n\
        \n\
        __kernel void waiter(__global int* value) {\n\
            int val = atomic_load(&counter);\n\
            while (val != 1000) {\n\
                val = atomic_load(&counter);\n\
            }\n\
            *value = val;\n\
        }";

    cl_program program = clCreateProgramWithSource(context, 1, &sources,
        nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateProgramWithSource failed";

    error = clBuildProgram(program, 0, nullptr, "-cl-std=CL2.0", nullptr,
        nullptr);
    EXPECT_EQ(CL_SUCCESS, error);
    if (CL_SUCCESS != error)
    {
        size_t logSize = 0;
        error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
            0, nullptr, &logSize);
        ASSERT_EQ(CL_SUCCESS, error) << "clGetProgramBuildInfo failed";

        std::string log("", logSize);
        error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
            logSize, &log[0], nullptr);
        ASSERT_EQ(CL_SUCCESS, error) << "clGetProgramBuildInfo failed";
        std::cout << log << std::endl;
        return;
    }

    cl_kernel infLoop = clCreateKernel(program, "infinite_loop_test", &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateKernel failed";

    cl_kernel waiter = clCreateKernel(program, "waiter", &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateKernel failed";

    cl_mem resultBuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
        sizeof(cl_int), nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateBuffer failed";

    error = clSetKernelArg(waiter, 0, sizeof(cl_mem), &resultBuf);
    ASSERT_EQ(CL_SUCCESS, error) << "clSetKernelArg failed";

    cl_command_queue infLoopQueue = clCreateCommandQueueWithProperties(context,
        device, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateCommandQueueWithProperties failed";

    cl_command_queue waiterQueue = clCreateCommandQueueWithProperties(context,
        device, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateCommandQueueWithProperties failed";

    const size_t numDim = 1;
    const size_t globalSize[numDim] = { 1 };
    const size_t localSize[numDim] = { 1 };

    error = clEnqueueNDRangeKernel(infLoopQueue, infLoop, numDim, nullptr,
        globalSize, localSize, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueNDRangeKernel failed";

    error = clEnqueueNDRangeKernel(waiterQueue, waiter, numDim, nullptr,
        globalSize, localSize, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueNDRangeKernel failed";

    error = clFinish(waiterQueue);
    ASSERT_EQ(CL_SUCCESS, error) << "clFinish failed";

    cl_int resultValue = 0;

    error = clEnqueueReadBuffer(waiterQueue, resultBuf, CL_TRUE, 0,
        sizeof(cl_int), &resultValue, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clEnqueueReadBuffer failed";

    ASSERT_EQ(1000, resultValue) << "check of resultValue failed";

    return;
}
