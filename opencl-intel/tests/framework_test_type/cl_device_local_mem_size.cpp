#include <CL/cl.h>

#include <cstdio>
#include <cstdlib>
#include <climits>
#include <string>
#include "FrameworkTest.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/resource.h>
#endif

cl_ulong trySetLocalMemSize(cl_ulong size)
{
    if (size != 0)
    {
        std::string str = std::to_string(size) + "B";
        // set env variable to change the default value of local mem size
        if (!SETENV("CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE", str.c_str()))
        {
            return 0;
        }
    }

    return size;
}

cl_ulong trySetStackSize(cl_ulong size)
{
#ifdef _WIN32
    // on windows we cannot change stack size on runtime, so, just return
    // predefined value
    return STACK_SIZE;
#else
    // another way to set stack size on Linux is to use `ulimit -s stack_size`
    rlimit tLimitStruct;
    tLimitStruct.rlim_cur = size;
    tLimitStruct.rlim_max = ULLONG_MAX;
    if (setrlimit(RLIMIT_STACK, &tLimitStruct) != 0)
    {
        printf("Failed to set stack size. Error code: %d\n", errno);
        return 0;
    }
    else
    {
        return tLimitStruct.rlim_cur;
    }
#endif
}

bool cl_device_local_mem_size_test_body(cl_ulong, const std::string&);

bool cl_device_local_mem_size_test()
{
    std::string programSources =
    "__kernel void test(__global int* o)\n"
    "{\n"
    "    const int size = 7 * 1024 * 1024 / sizeof(int);\n" // 7 MB of local memory
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
    bool bResult = true;
    bResult &= CheckCondition(L"trySetStackSize", stackSize != 0);
    if (!bResult)
    {
        return bResult;
    }

    cl_ulong expectedLocalMemSize = trySetLocalMemSize(STACK_SIZE);
    bResult &= CheckCondition(L"trySetLocalMemSize", expectedLocalMemSize != 0);
    if (!bResult)
    {
        return bResult;
    }

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
    bool bResult = true;
    bResult &= CheckCondition(L"trySetStackSize", stackSize != 0);
    if (!bResult)
    {
        return bResult;
    }

    return cl_device_local_mem_size_test_body(32 * 1024, programSources);
}
#endif

bool cl_device_local_mem_size_test_body(cl_ulong expectedLocalMemSize, const std::string &programSources)
{
    cl_int iRet = CL_SUCCESS;

    cl_platform_id platform = nullptr;
    iRet = clGetPlatformIDs(1, &platform, nullptr);
    bool bResult = true;
    bResult &= Check(L"clGetPlatrormIDs", CL_SUCCESS, iRet);
    if (!bResult)
    {
        return bResult;
    }

    cl_device_id device = nullptr;
    iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, nullptr);
    bResult &= Check(L"clGetDeviceIDs", CL_SUCCESS, iRet);
    if (!bResult)
    {
        return bResult;
    }

    cl_ulong localMemSize = 0;

    iRet = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &localMemSize, nullptr);
    bResult &= Check(L"clGetDeviceInfo", CL_SUCCESS, iRet);
    bResult &= CheckInt(L"CL_DEVICE_LOCAL_MEM_SIZE", expectedLocalMemSize, localMemSize);
    if (!bResult)
    {
        return bResult;
    }

    cl_context context = nullptr;
    cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
    context = clCreateContext(prop, 1, &device, nullptr, nullptr, &iRet);
    bResult &= Check(L"clCreateContext", CL_SUCCESS, iRet);
    if (!bResult)
    {
        return bResult;
    }

    cl_command_queue queue = nullptr;
    queue = clCreateCommandQueueWithProperties(context, device, nullptr, &iRet);
    bResult &= Check(L"clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);
    if (!bResult)
    {
        clReleaseContext(context);
        return bResult;
    }

    cl_program program = nullptr;
    const char *ps = programSources.c_str();
    bResult &= BuildProgramSynch(context, 1, (const char**)&ps, nullptr, "", &program);
    if (!bResult)
    {
        clReleaseContext(context);
        return bResult;
    }

    const size_t global_work_size = 100;
    cl_mem buffer = nullptr;
    buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, global_work_size * sizeof(cl_int), nullptr, &iRet);
    bResult &= Check(L"clCreateBuffer", CL_SUCCESS, iRet);
    if (!bResult)
    {
        clReleaseProgram(program);
        clReleaseContext(context);
        return bResult;
    }

    cl_kernel kernel = nullptr;
    kernel = clCreateKernel(program, "test", &iRet);
    bResult &= Check(L"clCreateKernel", CL_SUCCESS, iRet);
    if (!bResult)
    {
        clReleaseMemObject(buffer);
        clReleaseProgram(program);
        clReleaseContext(context);
        return bResult;
    }

    iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
    bResult &= Check(L"clSetKernelArg", CL_SUCCESS, iRet);
    if (!bResult)
    {
        clReleaseMemObject(buffer);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseContext(context);
        return bResult;
    }

    const size_t local_work_size = 10;
    iRet = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_work_size, &local_work_size, 0, nullptr, nullptr);
    bResult &= Check(L"clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
    if (!bResult)
    {
        clReleaseMemObject(buffer);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseContext(context);
        return bResult;
    }

    iRet = clFinish(queue);
    bResult &= Check(L"clFinish", CL_SUCCESS, iRet);
    if (!bResult)
    {
        clReleaseMemObject(buffer);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseContext(context);
        return bResult;
    }

    cl_int data[global_work_size] = { 0 };

    iRet = clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, global_work_size * sizeof(cl_int), data, 0, nullptr, nullptr);
    bResult &= Check(L"clEnqueueReadBuffer", CL_SUCCESS, iRet);
    if (!bResult)
    {
        clReleaseMemObject(buffer);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseContext(context);
        return bResult;
    }

    for (size_t i = 0; i < global_work_size; ++i)
    {
        bResult &= SilentCheckInt(L"data[i]", (cl_int)(i + 2), data[i]);
    }

    clReleaseMemObject(buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);

    return bResult;
}
