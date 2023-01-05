#include "CL/cl.h"
#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "cl_types.h"
#include "gtest/gtest.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

/*******************************************************************************
 * clIntegerExecuteTest
 * -------------------
 * Implement image access test
 ******************************************************************************/
void clLocalStructTest() {
  printf("=============================================================\n");
  printf("clILocalSTructAssingTest\n");
  printf("=============================================================\n");

  typedef struct _AABB {
    int Min[3];
    int Max[3];
  } AABB;

  const char *ocl_test_program[] = {
      "typedef struct _AABB {int Min[3];int Max[3];} AABB;\n"
      "kernel void Foo(global AABB* p, local AABB* lcl)\n"
      "{\n"
      "  int g=get_global_id(0);\n"
      "  __local AABB tileBB;\n"
      "  tileBB.Min[0] = -1.0; tileBB.Min[1] = -2.0; tileBB.Min[2] = -3.0;\n"
      "  tileBB.Max[0] = 1.0; tileBB.Max[1] = 2.0; tileBB.Max[2] = 3.0;\n"
      "  *lcl = tileBB;\n"
      "  event_t ev = async_work_group_copy((__global char*)&p[0], (__local "
      "char*)&tileBB, sizeof(AABB), 0);\n"
      "  wait_group_events(1, &ev);\n"
      "  p[1]=*lcl;\n"
      "}"};

  cl_platform_id platform = 0;

  cl_int err = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

  // define local variables
  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};
  cl_program program;
  cl_command_queue queue;
  cl_kernel kernel;

  size_t global_work_size[1] = {1};

  AABB pDstBuff[2];

  //
  // Initiate test infrastructure:
  // Create context, Queue
  //
  cl_context context =
      clCreateContextFromType(prop, gDeviceType, nullptr, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateContextFromType");

  cl_device_id clDefaultDeviceId;
  err = clGetDeviceIDs(platform, gDeviceType, 1, &clDefaultDeviceId, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

  queue = clCreateCommandQueueWithProperties(context, clDefaultDeviceId,
                                             nullptr /*no properties*/, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties - queue");

  ASSERT_TRUE(BuildProgramSynch(context, 1, (const char **)&ocl_test_program,
                                nullptr, nullptr, &program));

  kernel = clCreateKernel(program, "Foo", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel - Foo");

  srand(0);

  cl_mem clBuff = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(pDstBuff),
                                 nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  // Set Kernel Arguments
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuff);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg - clBuff");

  err = clSetKernelArg(kernel, 1, sizeof(AABB), (void *)nullptr);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg - local");

  // Execute kernel
  err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, global_work_size,
                               nullptr, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");
  //
  // Verification phase
  //
  err = clEnqueueReadBuffer(queue, clBuff, CL_TRUE, 0, sizeof(pDstBuff),
                            &pDstBuff, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer - Dst");

  ASSERT_TRUE(pDstBuff[0].Min[0] == -1);
  ASSERT_TRUE(pDstBuff[0].Min[1] == -2);
  ASSERT_TRUE(pDstBuff[0].Min[2] == -3);
  ASSERT_TRUE(pDstBuff[0].Max[0] == 1);
  ASSERT_TRUE(pDstBuff[0].Max[1] == 2);
  ASSERT_TRUE(pDstBuff[0].Max[2] == 3);
  ASSERT_TRUE(pDstBuff[1].Min[0] == -1);
  ASSERT_TRUE(pDstBuff[1].Min[1] == -2);
  ASSERT_TRUE(pDstBuff[1].Min[2] == -3);
  ASSERT_TRUE(pDstBuff[1].Max[0] == 1);
  ASSERT_TRUE(pDstBuff[1].Max[1] == 2);
  ASSERT_TRUE(pDstBuff[1].Max[2] == 3);

  err = clReleaseMemObject(clBuff);
  ASSERT_OCL_SUCCESS(err, "clReleaseMemObject");
  err = clReleaseKernel(kernel);
  ASSERT_OCL_SUCCESS(err, "clReleaseKernel");
  err = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
  err = clReleaseCommandQueue(queue);
  ASSERT_OCL_SUCCESS(err, "clReleaseCommandQueue");
  err = clReleaseContext(context);
  ASSERT_OCL_SUCCESS(err, "clReleaseContext");
}
