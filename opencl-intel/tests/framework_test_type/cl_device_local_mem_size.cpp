#include <CL/cl.h>

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "FrameworkTest.h"
#include "common_utils.h"

#ifndef _WIN32
#include <sys/resource.h>
#endif

extern cl_device_type gDeviceType;

cl_ulong trySetLocalMemSize(cl_ulong size)
{
#ifdef _WIN32
    printf("NOTE:\nDue to some strange behaviour of env variables on Windows\n");
    printf("\tthis test works only if you specify CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE from shell\n");
    printf("\tIn CI system it is done by .pm runner (framework_test_type.pm)\n");
#endif
    std::string str = std::to_string(size) + "B";
    // set env variable to change the default value of local mem size
    if (!SETENV("CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE", str.c_str()))
    {
        return 0;
    }

    return size;
}

cl_platform_id platform = nullptr;
cl_device_id device = nullptr;
cl_context context = nullptr;
cl_command_queue queue = nullptr;
cl_kernel kernel = nullptr;
cl_mem buffer = nullptr;
cl_program program = nullptr;

bool cl_device_local_mem_size_test_body(cl_ulong, const std::string&);
void cleanup();

#define EXIT_IF_FAILED(expr)\
    if (!expr)\
    {\
        cleanup();\
        return false;\
    }

bool cl_device_local_mem_size_test()
{
    std::string programSources =
    "__kernel void test(__global int* o)\n"
    "{\n"
    "    const int size = (STACK_SIZE - 1024 * 1024) / sizeof(int);\n" // STACK_SIZE - 1 MB of local memory
    "    __local int buf[size];\n"
    "    int pwi = size / get_local_size(0);\n"
    "    int lid = get_local_id(0);\n"
    "    int gid = get_global_id(0);\n"
    "    for (int i = lid * pwi; i < lid * pwi + pwi; ++i)\n"
    "        buf[i] = gid;\n"
    "    o[gid] = buf[pwi * lid + 1] + 2;\n"
    "}";

    printf("cl_device_local_mem_size_test\n");

    cl_ulong stackSize = trySetStackSize(STACK_SIZE);
    EXIT_IF_FAILED(CheckCondition("trySetStackSize", stackSize != 0));

    cl_ulong expectedLocalMemSize = trySetLocalMemSize(STACK_SIZE);
    EXIT_IF_FAILED(CheckCondition("trySetLocalMemSize", expectedLocalMemSize != 0));

    return cl_device_local_mem_size_test_body(expectedLocalMemSize, programSources);
}
#ifndef _WIN32
bool cl_device_local_mem_size_unlimited_stack_test()
{
    std::string programSources =
    "__kernel void test(__global int* o)\n"
    "{\n"
    "    const int size = 7 * 1024 / sizeof(int);\n" // 7 KB of local memory
    "    __local int buf[size];\n"
    "    int pwi = size / get_local_size(0);\n"
    "    int lid = get_local_id(0);\n"
    "    int gid = get_global_id(0);\n"
    "    for (int i = lid * pwi; i < lid * pwi + pwi; ++i)\n"
    "        buf[i] = gid;\n"
    "    o[gid] = buf[pwi * lid + 1] + 2;\n"
    "}";

    printf("cl_device_local_mem_size_unlimited_stack_test\n");

    cl_ulong stackSize = trySetStackSize(RLIM_INFINITY);
    EXIT_IF_FAILED(CheckCondition("trySetStackSize", stackSize != 0));

    if (gDeviceType == CL_DEVICE_TYPE_ACCELERATOR)
    {
        return cl_device_local_mem_size_test_body(256 * 1024, programSources);
    }
    return cl_device_local_mem_size_test_body(32 * 1024, programSources);
}
#endif

void cleanup()
{
    if (buffer)
        clReleaseMemObject(buffer);
    if (kernel)
        clReleaseKernel(kernel);
    if (queue)
        clReleaseCommandQueue(queue);
    if (program)
        clReleaseProgram(program);
    if (context)
        clReleaseContext(context);
}

bool cl_device_local_mem_size_test_body(cl_ulong expectedLocalMemSize, const std::string &programSources)
{
    cl_int iRet = CL_SUCCESS;

    iRet = clGetPlatformIDs(1, &platform, nullptr);
    EXIT_IF_FAILED(Check("clGetPlatrormIDs", CL_SUCCESS, iRet));

    iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
    EXIT_IF_FAILED(Check("clGetDeviceIDs", CL_SUCCESS, iRet));

    cl_ulong localMemSize = 0;

    iRet = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &localMemSize, nullptr);
    EXIT_IF_FAILED(Check("clGetDeviceInfo", CL_SUCCESS, iRet));
    EXIT_IF_FAILED(CheckInt("CL_DEVICE_LOCAL_MEM_SIZE", expectedLocalMemSize, localMemSize));

    cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
    context = clCreateContext(prop, 1, &device, nullptr, nullptr, &iRet);
    EXIT_IF_FAILED(Check("clCreateContext", CL_SUCCESS, iRet));

    queue = clCreateCommandQueueWithProperties(context, device, nullptr, &iRet);
    EXIT_IF_FAILED(Check("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet));

    const char *ps = programSources.c_str();
    std::string options = "-DSTACK_SIZE=" + std::to_string(expectedLocalMemSize);
    EXIT_IF_FAILED(BuildProgramSynch(context, 1, (const char**)&ps, nullptr, options.c_str(), &program));

    const size_t global_work_size = 100;
    buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, global_work_size * sizeof(cl_int), nullptr, &iRet);
    EXIT_IF_FAILED(Check("clCreateBuffer", CL_SUCCESS, iRet));

    kernel = clCreateKernel(program, "test", &iRet);
    EXIT_IF_FAILED(Check("clCreateKernel", CL_SUCCESS, iRet));

    iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
    EXIT_IF_FAILED(Check("clSetKernelArg", CL_SUCCESS, iRet));

    const size_t local_work_size = 10;
    iRet = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_work_size, &local_work_size, 0, nullptr, nullptr);
    EXIT_IF_FAILED(Check("clEnqueueNDRangeKernel", CL_SUCCESS, iRet));

    iRet = clFinish(queue);
    EXIT_IF_FAILED(Check("clFinish", CL_SUCCESS, iRet));

    cl_int data[global_work_size] = { 0 };

    iRet = clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, global_work_size * sizeof(cl_int), data, 0, nullptr, nullptr);
    EXIT_IF_FAILED(Check("clEnqueueReadBuffer", CL_SUCCESS, iRet));

    bool bResult = true;
    for (size_t i = 0; i < global_work_size; ++i)
    {
        bResult &= SilentCheckInt("data[i]", (cl_int)(i + 2), data[i]);
    }

    bResult = Check("Kernel results verification", true, bResult);

    cleanup();

    return bResult;
}
