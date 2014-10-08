#include "CL/cl.h"
#include <stdio.h>
#include "FrameworkTest.h"

#define BUFFERS_LENGTH 32

extern cl_device_type gDeviceType;

#define INT_BUFFER_INPUT {92, 84, 69, 43, 82, 79, 52, 17, 70, 28, 34, 77, 72, 32, 94, 51, 78, 81, 76, 39, 32, 33, 88, 54, 91, 89, 87, 98, 52, 47, 78, 23}

/**************************************************************************************************
 * Test based on CSSD100019990:
 * Kernel with invalid code which LLVM correctly identifies (the uninitialized pointer
 * assignment is replaced with 'unreachable'). We expect the vectorizer to avoid this kernel
 * (otherwise the vectorizer crashes) and the entire compiler to succesfully compile it.
 * We run the kernel on safe input (where the invalid flow is never taken) and expect valid output.
 **************************************************************************************************/
static const char *ocl_test_program[] = {
"\
struct Foo\
{\
  int v;\
};\
\
__kernel void test(__global int* out, __global int* in)\
{\
  struct Foo* bar;\
  int gid = get_global_id(0);\
  if (in[gid] > 777) {\
    bar->v = 0;\
    bar++;\
  }\
  out[gid] = in[gid] + 3;\
}\
"
};


bool validate_result(int* result) {
  int input_int[BUFFERS_LENGTH] = INT_BUFFER_INPUT;
  for( int i = 0; i < BUFFERS_LENGTH; ++i ) {
    if( result[i] != input_int[i] + 3) {
      printf("Result did not validate:\n\tresult[%d] = %d\n\texpected[%d] = %d\n",
             i, result[i],
             i, input_int[i] + 3);

      return false;
    }
  }
  return true;
}

bool execute_kernel(const char* const kernel_name,
                    cl_context* context,
                    cl_program* program,
                    cl_command_queue* queue,
                    cl_mem* buffer_src) {
  bool bResult = true;
  cl_int iRet;

  int output[BUFFERS_LENGTH];

  // Create Packed Kernel
  cl_kernel kernel = clCreateKernel(*program, kernel_name, &iRet);
  bResult &= SilentCheck(L"clCreateKernel", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Create buffers
  cl_mem buffer_dst_int = clCreateBuffer(*context,
                                         CL_MEM_WRITE_ONLY,
                                         BUFFERS_LENGTH * sizeof(int),
                                         NULL,
                                         &iRet);
  bResult &= SilentCheck(L"clCreateBuffer - dst", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Set arguments - test
  iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_dst_int);
  bResult &= SilentCheck(L"clSetKernelArg - buffer_dst_int", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), buffer_src);
  bResult &= SilentCheck(L"clSetKernelArg - buffer_src", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Execute kernel - test
  size_t global_work_size[1] = { BUFFERS_LENGTH };
  size_t local_work_size[1] = { 16 };

  iRet = clEnqueueNDRangeKernel(*queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
  bResult &= SilentCheck(L"clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Read results. wait for completion - blocking!
  iRet = clEnqueueReadBuffer (*queue, buffer_dst_int, CL_TRUE,  0, BUFFERS_LENGTH * sizeof(int), output, 0, NULL, NULL);
  bResult &= SilentCheck(L"clEnqueueReadBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Release objects
  iRet = clReleaseMemObject(buffer_dst_int);
  bResult &= SilentCheck(L"clReleaseBuffer - buffer_dst_int", CL_SUCCESS, iRet);

  iRet = clReleaseKernel(kernel);
  bResult &= SilentCheck(L"clReleaseKernel - kernel", CL_SUCCESS, iRet);

  return validate_result(output);
}

bool clDoNotVectorizeUnreachable()
{
  bool bResult = true;

  cl_platform_id platform = 0;
  cl_device_id device;
  cl_context context;

  // get platform
  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck(L"clGetPlatformIDs", CL_SUCCESS, iRet);
  if (!bResult)
  {
    return bResult;
  }

  // get device
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  bResult &= SilentCheck(L"clGetDeviceIDs",CL_SUCCESS, iRet);
  if (!bResult)
  {
    return bResult;
  }

  // create context
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &iRet);
  bResult &= SilentCheck(L"clCreateContext",CL_SUCCESS, iRet);
  if (!bResult)
  {
    return bResult;
  }

  // create program with source
  cl_program program = clCreateProgramWithSource(context, 1, (const char**)&ocl_test_program, NULL, &iRet);
  bResult &= SilentCheck(L"clCreateProgramWithSource", CL_SUCCESS, iRet);
  if (!bResult)
  {
    return bResult;
  }

  // build program
  iRet = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  bResult &= SilentCheck(L"clBuildProgram", CL_SUCCESS, iRet);
  if (!bResult)
  {
    return bResult;
  }

  //
  // From here down it is the program execution implementation
  //
  int input_int[BUFFERS_LENGTH] = INT_BUFFER_INPUT;

  // Create queue
  cl_command_queue queue = clCreateCommandQueue (context, device, 0, &iRet);
  bResult &= SilentCheck(L"clCreateCommandQueue - queue", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Create buffers
  cl_mem buffer_src = clCreateBuffer(context,
                                     CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                     BUFFERS_LENGTH * sizeof(int),
                                     input_int,
                                     &iRet);
  bResult &= SilentCheck(L"clCreateBuffer - src", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // execute kernel
  bResult &= execute_kernel("test",
                            &context,
                            &program,
                            &queue,
                            &buffer_src);

  // Release objects

  iRet = clReleaseMemObject(buffer_src);
  bResult &= SilentCheck(L"clReleaseBuffer - buffer_src", CL_SUCCESS, iRet);

  iRet = clReleaseProgram(program);
  bResult &= SilentCheck(L"clReleaseProgram - program", CL_SUCCESS, iRet);

  iRet = clReleaseCommandQueue(queue);
  bResult &= SilentCheck(L"clReleaseCommandQueue - queue", CL_SUCCESS, iRet);

  iRet = clReleaseContext(context);
  bResult &= SilentCheck(L"clReleaseContext - context", CL_SUCCESS, iRet);

  return bResult;
}

