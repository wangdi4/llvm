#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include "gtest_wrapper.h"
#include <float.h>
#include <stdio.h>

//
// Testing note: This test case creates a dedicated process for each test. In
// order to test it in Visual Studio define the following macro to forse running
// in the same process. GTEST will not work ok because of early exists but
// debugging is possible
// #define DEBUGGING_DEATH_TEST
// #define SilentCheck Check

#define BUFFERS_LENGTH 20000
extern cl_device_type gDeviceType;

/*******************************************************************************
 *
 *
 ******************************************************************************/
enum TEST_CASE {
  NORMAL_EXIT = 0,
  NO_BUFFER_RELEASES,
  NO_KERNEL_RELEASES,
  NO_PROGRAM_RELEASES,
  NO_QUEUE_RELEASES,
  NO_CONTEXT_RELEASES,
  NO_EVENTS_RELEASES
};

void CL_CALLBACK BufferDestruct(cl_mem memobj, void *user_data) {
// Disabled on Windows because it may crash.
// The problem is that we may call BufferDestruct callback after stdout and
// stderr handlers are destroyed. The handlers are destroyed before we get
// process terminating notification because the test is statically linked
// against windows runtime.
#ifndef _WIN32
  const char *buf_name = (const char *)user_data;
  fprintf(stdout, "%s buffer destructed\n", buf_name);
  fprintf(stderr, "%s buffer destructed\n",
          buf_name); // note - this output is checked by gtest
#endif
}

bool cl_shutdown_test(TEST_CASE test_case, const char *name) {
  printf("---------------------------------------\n");
  printf("cl_shutdown_test: %s\n", name);
  printf("---------------------------------------\n");
  const char *ocl_test_program[] = {
      "__kernel void dot_product (__global const float4 *a, __global const "
      "float4 *b, __global float4 *c)"
      "{"
      "int tid = get_global_id(0);"
      "c[tid] = fma(a[tid], b[tid], c[tid]);"
      "}"};

  bool bResult = true;
  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  size_t *pBinarySizes;
  cl_int *pBinaryStatus;
  cl_context context;

  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // get device(s)
  iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];
  pBinarySizes = new size_t[uiNumDevices];
  pBinaryStatus = new cl_int[uiNumDevices];

  iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return bResult;
  }

  // create context
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  bResult &= SilentCheck("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return bResult;
  }
  printf("context = %p\n", (void *)context);

  //
  // Create queue
  //
  cl_command_queue queue1 = clCreateCommandQueueWithProperties(
      context, pDevices[0], NULL /*no properties*/, &iRet);
  bResult &= SilentCheck("clCreateCommandQueueWithProperties - queue1",
                         CL_SUCCESS, iRet);

  cl_context cntxInfo;
  iRet = clGetCommandQueueInfo(queue1, CL_QUEUE_CONTEXT, sizeof(cl_context),
                               &cntxInfo, NULL);
  bResult &= SilentCheck("clGetCommandQueueInfo", CL_SUCCESS, iRet);
  bResult &= CheckHandle("clGetCommandQueueInfo - context", context, cntxInfo);

  // create program with source
  cl_program program = clCreateProgramWithSource(
      context, 1, (const char **)&ocl_test_program, NULL, &iRet);
  bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, iRet);

  iRet = clBuildProgram(program, uiNumDevices, pDevices, "-cl-denorms-are-zero",
                        NULL, NULL);
  bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, iRet);

  //
  // Create Kernel
  //
  cl_kernel kernel1 = clCreateKernel(program, "dot_product", &iRet);
  bResult &= SilentCheck("clCreateKernel - dot_product", CL_SUCCESS, iRet);

  //
  // From here down it is the program execution implementation
  //
  cl_float srcA[BUFFERS_LENGTH];
  cl_float srcB[BUFFERS_LENGTH];
  cl_float dst[BUFFERS_LENGTH];

  for (int j = 0; j < BUFFERS_LENGTH; j++) {
    srcA[j] = FLT_MIN;
    srcB[j] = 0.1f;
    dst[j] = 0.0f;
  }

  //
  // Create buffers
  //
  size_t size = sizeof(cl_float);

  cl_mem buffer_srcA = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                      size * BUFFERS_LENGTH, NULL, &iRet);
  bResult &= SilentCheck("clCreateBuffer - srcA", CL_SUCCESS, iRet);
  iRet = clSetMemObjectDestructorCallback(buffer_srcA, BufferDestruct,
                                          (void *)"srcA");
  bResult &=
      SilentCheck("clSetMemObjectDestructorCallback - srcA", CL_SUCCESS, iRet);

  cl_mem buffer_srcB = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                      size * BUFFERS_LENGTH, NULL, &iRet);
  bResult &= SilentCheck("clCreateBuffer - srcB", CL_SUCCESS, iRet);
  iRet = clSetMemObjectDestructorCallback(buffer_srcB, BufferDestruct,
                                          (void *)"srcB");
  bResult &=
      SilentCheck("clSetMemObjectDestructorCallback - srcB", CL_SUCCESS, iRet);

  cl_mem buffer_dst = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                     size * BUFFERS_LENGTH, NULL, &iRet);
  bResult &= SilentCheck("clCreateBuffer - dst", CL_SUCCESS, iRet);
  iRet = clSetMemObjectDestructorCallback(buffer_dst, BufferDestruct,
                                          (void *)"dst");
  bResult &=
      SilentCheck("clSetMemObjectDestructorCallback - dst", CL_SUCCESS, iRet);

  //
  // Create Events
  //
  cl_event userEvent = clCreateUserEvent(context, &iRet);
  bResult &= SilentCheck("clCreateUserEvent", CL_SUCCESS, iRet);
  cl_event ndrEvent;

  //
  // Set arguments
  //
  iRet = clSetKernelArg(kernel1, 0, sizeof(cl_mem), &buffer_srcA);
  bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(kernel1, 1, sizeof(cl_mem), &buffer_srcB);
  bResult &= SilentCheck("clSetKernelArg - buffer_srcB", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(kernel1, 2, sizeof(cl_mem), &buffer_dst);
  bResult &= SilentCheck("clSetKernelArg - buffer_dst", CL_SUCCESS, iRet);

  //
  // Execute commands - Write buffers
  //
  iRet = clEnqueueWriteBuffer(queue1, buffer_srcA, false, 0,
                              size * BUFFERS_LENGTH, srcA, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueWriteBuffer - srcA", CL_SUCCESS, iRet);

  iRet = clEnqueueWriteBuffer(queue1, buffer_srcB, false, 0,
                              size * BUFFERS_LENGTH, srcB, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueWriteBuffer - srcB", CL_SUCCESS, iRet);

  iRet = clEnqueueWriteBuffer(queue1, buffer_dst, false, 0,
                              size * BUFFERS_LENGTH, dst, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueWriteBuffer - srcB", CL_SUCCESS, iRet);

  //
  // Execute kernel - dot_product
  //

  size_t global_work_size[1] = {BUFFERS_LENGTH / 4};
  size_t local_work_size[1] = {1};

  {
    for (int j = 0; j < BUFFERS_LENGTH; j++) {
      dst[j] = 0.0f;
    }
    iRet = clEnqueueWriteBuffer(queue1, buffer_dst, false, 0,
                                size * BUFFERS_LENGTH, dst, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueWriteBuffer - dst", CL_SUCCESS, iRet);

    //
    // Enqueue kernel that should never be executed as it depends on userEvetn
    // that is never going to be signaled
    //
    iRet = clEnqueueNDRangeKernel(queue1, kernel1, 1, NULL, global_work_size,
                                  local_work_size, 1, &userEvent, &ndrEvent);
    bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

    //
    // Map/Unmap results - non-blocking, but it will wait for kernel before
    //
    void *mapped = clEnqueueMapBuffer(
        queue1, buffer_dst, CL_FALSE, CL_MAP_READ | CL_MAP_WRITE, 0,
        size * BUFFERS_LENGTH, 0, NULL, NULL, &iRet);
    bResult &= SilentCheck("clEnqueueMapBuffer", CL_SUCCESS, iRet);
    iRet = clEnqueueUnmapMemObject(queue1, buffer_dst, mapped, 0, NULL, NULL);
  }

  //
  // Release objects
  //
  if (NO_BUFFER_RELEASES == test_case)
    exit(0);

  iRet = clReleaseMemObject(buffer_dst);
  bResult &= SilentCheck("clReleaseBuffer - buffer_dst", CL_SUCCESS, iRet);

  iRet = clReleaseMemObject(buffer_srcA);
  bResult &= SilentCheck("clReleaseBuffer - buffer_srcA", CL_SUCCESS, iRet);

  iRet = clReleaseMemObject(buffer_srcB);
  bResult &= SilentCheck("clReleaseBuffer - buffer_srcB", CL_SUCCESS, iRet);

  if (NO_KERNEL_RELEASES == test_case)
    exit(0);

  iRet = clReleaseKernel(kernel1);
  bResult &= SilentCheck("clReleaseKernel - kernel1", CL_SUCCESS, iRet);

  if (NO_PROGRAM_RELEASES == test_case)
    exit(0);

  iRet = clReleaseProgram(program);
  bResult &= SilentCheck("clReleaseProgram - program", CL_SUCCESS, iRet);

  if (NO_QUEUE_RELEASES == test_case)
    exit(0);

  iRet = clReleaseCommandQueue(queue1);
  bResult &= SilentCheck("clReleaseCommandQueue - queue1", CL_SUCCESS, iRet);

  if (NO_CONTEXT_RELEASES == test_case)
    exit(0);

  iRet = clReleaseContext(context);
  bResult &= SilentCheck("clReleaseContext - context", CL_SUCCESS, iRet);

  if (NO_EVENTS_RELEASES == test_case)
    exit(0);

  iRet = clReleaseEvent(ndrEvent);
  bResult &= SilentCheck("clReleaseEvent - ndrEvent", CL_SUCCESS, iRet);

  iRet = clReleaseEvent(userEvent);
  bResult &= SilentCheck("clReleaseEvent - userEvent", CL_SUCCESS, iRet);

  if (NORMAL_EXIT == test_case)
    exit(0);

  return bResult;
}

#define death_test(t) death_test_imp(t, #t)

void death_test_imp(TEST_CASE test_case, const char *name) {
#ifdef DEBUGGING_DEATH_TEST
  EXPECT_TRUE(cl_shutdown_test(test_case, name));
#else
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  // on Linux we DO clFinish for all queues during ShutDown - all buffers MUST
  // be destructed
  const char *expectedStdErrString = "dst buffer destructed";

#ifdef _WIN32
  // on Windows and FPGA emulator build we do not do clFinish for all queues
  // during ShutDown - not all buffers may be destructed
  // + TBB warning is printed last in Debug ------
  expectedStdErrString = "";
#endif
  if (gDeviceType == CL_DEVICE_TYPE_ACCELERATOR) {
    expectedStdErrString = "";
  }

  EXPECT_EXIT(
      {
        cl_shutdown_test(test_case, name);
        exit(1);
      },
      ::testing::ExitedWithCode(0), expectedStdErrString);
#endif
}

////////////////////////////////////////////////////////////////////////////////
//
// Do not remove DeathTest suffix from the test names - it is the Google Test
// requirement
//
////////////////////////////////////////////////////////////////////////////////

// The tests are disabled because the functionality they  test is disabled as
// well.

TEST(FrameworkTestTypeDeathTest, DISABLED_Test_NormalExit) {
  death_test(NORMAL_EXIT);
}

TEST(FrameworkTestTypeDeathTest, DISABLED_Test_NO_BUFFER_RELEASES) {
  death_test(NO_BUFFER_RELEASES);
}

TEST(FrameworkTestTypeDeathTest, DISABLED_Test_NO_KERNEL_RELEASES) {
  death_test(NO_KERNEL_RELEASES);
}

TEST(FrameworkTestTypeDeathTest, DISABLED_Test_NO_PROGRAM_RELEASES) {
  death_test(NO_PROGRAM_RELEASES);
}

TEST(FrameworkTestTypeDeathTest, DISABLED_Test_NO_QUEUE_RELEASES) {
  death_test(NO_QUEUE_RELEASES);
}

TEST(FrameworkTestTypeDeathTest, DISABLED_Test_NO_CONTEXT_RELEASES) {
  death_test(NO_CONTEXT_RELEASES);
}

TEST(FrameworkTestTypeDeathTest, DISABLED_Test_NO_EVENTS_RELEASES) {
  death_test(NO_EVENTS_RELEASES);
}
