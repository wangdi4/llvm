// Copyright (c) 2021 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "cl_types.h"
#include "common_utils.h"
#include <stdio.h>
#include <string>

extern cl_device_type gDeviceType;

void clKernelLocalMemSizeQueryTest() {
  const char *krnl_str = "__kernel void testKernelLocalMemSizeQuery(\n"
                         "    __global int *a,\n"
                         "    __local int *b,\n"
                         "    __local int *c) {\n"
                         "  __local int localVarTmp;\n"
                         "  localVarTmp = 10;\n"
                         "  for (int i = 0; i < 20; ++i) {\n"
                         "    a[i] = i;\n"
                         "    b[i] = i;\n"
                         "    c[i] = i;\n"
                         "    localVarTmp += i;\n"
                         "  }\n"
                         "}\n";
  cl_platform_id platf;
  cl_uint nplatf;
  cl_device_id dev = NULL;
  cl_context ctx = NULL;
  cl_program prg = NULL;
  cl_kernel krnl = NULL;
  size_t param_value_size_ret;
  cl_int status;

  /* Get platforms. */
  status = clGetPlatformIDs(1, &platf, &nplatf);
  ASSERT_EQ(CL_SUCCESS, status) << "clGetPlatformIDs failed";

  /* Get CPU device. */
  status = clGetDeviceIDs(platf, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
  ASSERT_EQ(CL_SUCCESS, status) << "clGetDeviceIDs failed";

  /* Create context. */
  ctx = clCreateContext(NULL, 1, &dev, NULL, NULL, &status);
  ASSERT_EQ(CL_SUCCESS, status) << "clGetCreateContext failed";

  /* Create program. */
  prg = clCreateProgramWithSource(ctx, 1, &krnl_str, NULL, &status);
  ASSERT_EQ(CL_SUCCESS, status) << "clCreateProgramWithSource failed";

  // Build program. Disable optimization so that localVarTmp won't be optimized
  // out.
  status = clBuildProgram(prg, 1, (const cl_device_id *)&dev, "-cl-opt-disable",
                          NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, status) << "clBuildProgram failed";

  /* Create kernel. */
  krnl = clCreateKernel(prg, "testKernelLocalMemSizeQuery", &status);
  ASSERT_EQ(CL_SUCCESS, status) << "clCreateKernel failed";

  /* Create buffer. */
  cl_mem buffer_a = clCreateBuffer(ctx, CL_MEM_READ_WRITE, 100, NULL, NULL);

  /* Set arguments. */
  status = clSetKernelArg(krnl, 0, sizeof(cl_mem), &buffer_a);
  ASSERT_OCL_SUCCESS(status, "clSetKernelArg");

  cl_ulong kernelLocalMemSize;

  /* Get size of work-group info: CL_KERNEL_LOCAL_MEM_SIZE. */
  status = clGetKernelWorkGroupInfo(krnl, dev, CL_KERNEL_LOCAL_MEM_SIZE,
                                    sizeof(cl_ulong), &kernelLocalMemSize,
                                    &param_value_size_ret);

  ASSERT_EQ(CL_SUCCESS, status)
      << "clGetKernelWorkGroupInfo(CL_KERNEL_LOCAL_MEM_SIZE) failed";
  ASSERT_EQ(param_value_size_ret, sizeof(cl_ulong))
      << "Wrong buffer data size returned";
  ASSERT_EQ(4, kernelLocalMemSize) << "Wrong kernelLocalMemSize returned";

  /* Set arguments. */
  status = clSetKernelArg(krnl, 1, 100, NULL);
  ASSERT_OCL_SUCCESS(status, "clSetKernelArg");

  status = clGetKernelWorkGroupInfo(krnl, dev, CL_KERNEL_LOCAL_MEM_SIZE,
                                    sizeof(cl_ulong), &kernelLocalMemSize,
                                    &param_value_size_ret);

  ASSERT_EQ(CL_SUCCESS, status)
      << "clGetKernelWorkGroupInfo(CL_KERNEL_LOCAL_MEM_SIZE) failed";
  ASSERT_EQ(param_value_size_ret, sizeof(cl_ulong))
      << "Wrong buffer data size returned";
  ASSERT_EQ(104, kernelLocalMemSize) << "Wrong kernelLocalMemSize returned";

  status = clSetKernelArg(krnl, 2, 200, NULL);
  ASSERT_OCL_SUCCESS(status, "clSetKernelArg");

  status = clGetKernelWorkGroupInfo(krnl, dev, CL_KERNEL_LOCAL_MEM_SIZE,
                                    sizeof(cl_ulong), &kernelLocalMemSize,
                                    &param_value_size_ret);

  ASSERT_EQ(CL_SUCCESS, status)
      << "clGetKernelWorkGroupInfo(CL_KERNEL_LOCAL_MEM_SIZE) failed";
  ASSERT_EQ(param_value_size_ret, sizeof(cl_ulong))
      << "Wrong buffer data size returned";
  ASSERT_EQ(304, kernelLocalMemSize) << "Wrong kernelLocalMemSize returned";

  /* Release stuff. */
  status = clReleaseMemObject(buffer_a);
  ASSERT_OCL_SUCCESS(status, "clReleaseMemObject");
  status = clReleaseKernel(krnl);
  ASSERT_OCL_SUCCESS(status, "clReleaseKernel");
  status = clReleaseProgram(prg);
  ASSERT_OCL_SUCCESS(status, "clReleaseProgram");
  status = clReleaseContext(ctx);
  ASSERT_OCL_SUCCESS(status, "clReleaseContext");
}
