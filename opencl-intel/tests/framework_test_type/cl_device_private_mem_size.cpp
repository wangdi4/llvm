#include <CL/cl.h>

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "FrameworkTest.h"
#include "common_utils.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/resource.h>
#endif

extern cl_device_type gDeviceType;

cl_ulong trySetPrivateMemSize(cl_ulong size)
{
#ifdef _WIN32
    printf("NOTE:\nDue to some strange behaviour of env variables on Windows\n");
    printf("\tthis test works only if you specify "
            "CL_CONFIG_CPU_FORCE_PRIVATE_MEM_SIZE from shell\n");
    printf("\tIn CI system it is done by .pm runner (framework_test_type.pm)\n");
#endif
    std::string str = std::to_string(size) + "B";
    // set env variable to change the default value of private mem size
    if (!SETENV("CL_CONFIG_CPU_FORCE_PRIVATE_MEM_SIZE", str.c_str()))
    {
        return 0;
    }

    return size;
}

bool vectorizerMode(bool enabled)
{
    std::string mode = enabled ? "True" : "False";
    if (!SETENV("CL_CONFIG_USE_VECTORIZER", mode.c_str()))
    {
        return false;
    }
    return true;
}

cl_platform_id platform_private = nullptr;
cl_device_id device_private = nullptr;
cl_context context_private = nullptr;
cl_command_queue queue_private = nullptr;
cl_kernel kernel_private = nullptr;
cl_mem buffer_private = nullptr;
cl_program program_private = nullptr;

bool cl_device_private_mem_size_test_body(cl_ulong, const std::string&,
                                          bool out_of_recources = false);
void cleanup_private();

#define EXIT_IF_FAILED(expr)\
    if (!expr)\
    {\
        cleanup_private();\
        return false;\
    }

bool cl_device_private_mem_size_test()
{
    std::string programSources =
    "__kernel void test(__global int* o)\n"
    "{\n"
    "    const int size = (STACK_SIZE/16) / sizeof(int);\n" // (STACK_SIZE/SIMD_WIDTH)MB of private memory
    "    __private volatile int buf[size];\n"
    "    int gid = get_global_id(0);\n"
    "    for (int i = 0; i < size; ++i)\n"
    "        buf[i] = gid;\n"
    "    o[gid] = buf[gid + 1] + 2;\n"
    "}";

    printf("cl_device_private_mem_size_test\n");

    cl_ulong stackSize = trySetStackSize(STACK_SIZE);
    EXIT_IF_FAILED(CheckCondition("trySetStackSize", stackSize != 0));

    bool enabledVectorizer = vectorizerMode(true);
    EXIT_IF_FAILED(CheckCondition("vectorizerMode", enabledVectorizer == true));

    cl_ulong expectedPrivateMemSize = trySetPrivateMemSize(STACK_SIZE);
    EXIT_IF_FAILED(CheckCondition("trySetPrivateMemSize", expectedPrivateMemSize != 0));

    return cl_device_private_mem_size_test_body(expectedPrivateMemSize, programSources);
}

bool cl_device_private_mem_size_test_out_of_resources()
{
    std::string programSources =
    "__kernel void test(__global int* o)\n"
    "{\n"
    "    const int size = (STACK_SIZE/2) / sizeof(int);\n" // (STACK_SIZE/2)MB of private memory
    "    printf(\"SIZE: %d \\n\", size);\n"
    "    __private volatile int buf[size];\n"
    "    int gid = get_global_id(0);\n"
    "    for (int i = 0; i < size; ++i)\n"
    "        buf[i] = gid;\n"
    "    o[gid] = buf[gid + 1] + 2;\n"
    "}";

    printf("cl_device_private_mem_size_test_out_of_resources\n");

    cl_ulong stackSize = trySetStackSize(STACK_SIZE);
    EXIT_IF_FAILED(CheckCondition("trySetStackSize", stackSize != 0));

    bool enabledVectorizer = vectorizerMode(true);
    EXIT_IF_FAILED(CheckCondition("vectorizerMode", enabledVectorizer == true));

    cl_ulong expectedPrivateMemSize = trySetPrivateMemSize(STACK_SIZE);
    EXIT_IF_FAILED(CheckCondition("trySetPrivateMemSize", expectedPrivateMemSize != 0));

    return cl_device_private_mem_size_test_body(expectedPrivateMemSize,
                                                programSources, true);
}

bool cl_device_private_mem_size_test_without_vectorizer()
{
    std::string programSources =
    "__kernel void test(__global int* o)\n"
    "{\n"
    "    const int size = (STACK_SIZE - 1024*1024) / sizeof(int);\n" // STACK_SIZE MB - 1MB of private memory
    "    printf(\"SIZE: %d \\n\", size);\n"
    "    __private volatile int buf[size];\n"
    "    int gid = get_global_id(0);\n"
    "    for (int i = 0; i < size; ++i)\n"
    "        buf[i] = gid;\n"
    "    o[gid] = buf[gid + 1] + 2;\n"
    "}";

    printf("cl_device_private_mem_size_test_without_vectorizer\n");

    cl_ulong stackSize = trySetStackSize(STACK_SIZE);
    EXIT_IF_FAILED(CheckCondition("trySetStackSize", stackSize != 0));

    bool disabledVectorizer = vectorizerMode(false);
    EXIT_IF_FAILED(CheckCondition("vectorizerMode", disabledVectorizer == true));

    cl_ulong expectedPrivateMemSize = trySetPrivateMemSize(STACK_SIZE);
    EXIT_IF_FAILED(CheckCondition("trySetPrivateMemSize", expectedPrivateMemSize != 0));

    bool res =  cl_device_private_mem_size_test_body(expectedPrivateMemSize, programSources);

    bool enableVectorizer = vectorizerMode(true);
    EXIT_IF_FAILED(CheckCondition("vectorizerMode", enableVectorizer == true));

    return res;
}

void cleanup_private()
{
    if (buffer_private)
        clReleaseMemObject(buffer_private);
    if (kernel_private)
        clReleaseKernel(kernel_private);
    if (queue_private)
        clReleaseCommandQueue(queue_private);
    if (program_private)
        clReleaseProgram(program_private);
    if (context_private)
        clReleaseContext(context_private);
}

bool cl_device_private_mem_size_test_body(cl_ulong expectedPrivateMemSize,
                                          const std::string &programSources,
                                          bool out_of_recources)
{
    cl_int iRet = CL_SUCCESS;

    iRet = clGetPlatformIDs(1, &platform_private, nullptr);
    EXIT_IF_FAILED(Check("clGetPlatrormIDs", CL_SUCCESS, iRet));

    iRet = clGetDeviceIDs(platform_private, gDeviceType, 1, &device_private, nullptr);
    EXIT_IF_FAILED(Check("clGetDeviceIDs", CL_SUCCESS, iRet));

    cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform_private, 0 };
    context_private = clCreateContext(prop, 1, &device_private, nullptr, nullptr, &iRet);
    EXIT_IF_FAILED(Check("clCreateContext", CL_SUCCESS, iRet));

    queue_private = clCreateCommandQueueWithProperties(context_private, device_private, nullptr, &iRet);
    EXIT_IF_FAILED(Check("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet));

    const char *ps = programSources.c_str();
    std::string options = "-DSTACK_SIZE=" + std::to_string(expectedPrivateMemSize);
    EXIT_IF_FAILED(BuildProgramSynch(context_private, 1, (const char**)&ps, nullptr, options.c_str(), &program_private));

    const size_t global_work_size = 1;
    buffer_private = clCreateBuffer(context_private, CL_MEM_READ_WRITE,
                                    global_work_size * sizeof(cl_int), nullptr, &iRet);
    EXIT_IF_FAILED(Check("clCreateBuffer", CL_SUCCESS, iRet));

    kernel_private = clCreateKernel(program_private, "test", &iRet);
    EXIT_IF_FAILED(Check("clCreateKernel", CL_SUCCESS, iRet));

    iRet = clSetKernelArg(kernel_private, 0, sizeof(cl_mem), &buffer_private);
    EXIT_IF_FAILED(Check("clSetKernelArg", CL_SUCCESS, iRet));

    const size_t local_work_size = 1;
    iRet = clEnqueueNDRangeKernel(queue_private, kernel_private, 1, nullptr, &global_work_size, &local_work_size, 0, nullptr, nullptr);
    if (out_of_recources)
    {
        EXIT_IF_FAILED(Check("clEnqueueNDRangeKernel", CL_OUT_OF_RESOURCES, iRet));
        return iRet;
    }
    EXIT_IF_FAILED(Check("clEnqueueNDRangeKernel", CL_SUCCESS, iRet));

    iRet = clFinish(queue_private);
    EXIT_IF_FAILED(Check("clFinish", CL_SUCCESS, iRet));

    cl_int data[global_work_size] = { 0 };

    iRet = clEnqueueReadBuffer(queue_private, buffer_private, CL_TRUE, 0,
                               global_work_size * sizeof(cl_int), data, 0, nullptr, nullptr);
    EXIT_IF_FAILED(Check("clEnqueueReadBuffer", CL_SUCCESS, iRet));

    bool bResult = true;
    for (size_t i = 0; i < global_work_size; ++i)
    {
        bResult &= SilentCheckInt("data[i]", (cl_int)(i + 2), data[i]);
    }

    bResult = Check("kernel_private results verification", true, bResult);

    cleanup_private();

    return bResult;
}

