// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "CL21.h"

const char *krnl_str = "__kernel void test_sum(\n"
                       "    __global const uint *a,\n"
                       "    __global const uint *b,\n"
                       "    __global uint *c, uint d) {\n"
                       "        int gid = get_global_id(0);\n"
                       "        c[gid] = a[gid] + b[gid] + d;\n"
                       "}\n";

TEST_F(CL21, GetKernelWorkGroupInfo_SizeRet) {
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

  /* Build program. */
  status = clBuildProgram(prg, 1, (const cl_device_id *)&dev, NULL, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, status) << "clBuildProgram failed";

  /* Create kernel. */
  krnl = clCreateKernel(prg, "test_sum", &status);
  ASSERT_EQ(CL_SUCCESS, status) << "clCreateKernel failed";

  /* Get size of work-group info: CL_KERNEL_WORK_GROUP_SIZE */
  status = clGetKernelWorkGroupInfo(krnl, dev, CL_KERNEL_WORK_GROUP_SIZE, 0,
                                    NULL, &param_value_size_ret);

  ASSERT_EQ(CL_SUCCESS, status)
      << "clGetKernelWorkGroupInfo(CL_KERNEL_WORK_GROUP_SIZE) failed";
  ASSERT_EQ(param_value_size_ret, sizeof(size_t))
      << "Wrong buffer data size returned";

  /* Get size of work-group info: CL_KERNEL_GLOBAL_WORK_SIZE */
#if 0 /// NOT IMPLEMENTED YET IN RUNTIME
  status = clGetKernelWorkGroupInfo(krnl, dev, CL_KERNEL_GLOBAL_WORK_SIZE, 0,
                                    NULL, &param_value_size_ret);

  ASSERT_EQ(CL_SUCCESS, status) << "clGetKernelWorkGroupInfo(CL_KERNEL_GLOBAL_WORK_SIZE) failed";
  ASSERT_EQ(param_value_size_ret, sizeof(size_t[3]))
      << "Wrong buffer data size returned";
#endif
  /* Get size of work-group info: CL_KERNEL_COMPILE_WORK_GROUP_SIZE */
  status =
      clGetKernelWorkGroupInfo(krnl, dev, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, 0,
                               NULL, &param_value_size_ret);

  ASSERT_EQ(CL_SUCCESS, status)
      << "clGetKernelWorkGroupInfo(CL_KERNEL_COMPILE_WORK_GROUP_SIZE) failed";
  ASSERT_EQ(param_value_size_ret, sizeof(size_t[3]))
      << "Wrong buffer data size returned";

  /* Get size of work-group info: CL_KERNEL_LOCAL_MEM_SIZE */
  status = clGetKernelWorkGroupInfo(krnl, dev, CL_KERNEL_LOCAL_MEM_SIZE, 0,
                                    NULL, &param_value_size_ret);

  ASSERT_EQ(CL_SUCCESS, status)
      << "clGetKernelWorkGroupInfo(CL_KERNEL_LOCAL_MEM_SIZE) failed";
  ASSERT_EQ(param_value_size_ret, sizeof(cl_ulong))
      << "Wrong buffer data size returned";

  /* Get size of work-group info: CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE
   */
  status = clGetKernelWorkGroupInfo(
      krnl, dev, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, 0, NULL,
      &param_value_size_ret);

  ASSERT_EQ(CL_SUCCESS, status) << "clGetKernelWorkGroupInfo(CL_KERNEL_"
                                   "PREFERRED_WORK_GROUP_SIZE_MULTIPLE) failed";
  ASSERT_EQ(param_value_size_ret, sizeof(size_t))
      << "Wrong buffer data size returned";

  /* Get size of work-group info: CL_KERNEL_PRIVATE_MEM_SIZE */
  status = clGetKernelWorkGroupInfo(krnl, dev, CL_KERNEL_PRIVATE_MEM_SIZE, 0,
                                    NULL, &param_value_size_ret);

  ASSERT_EQ(CL_SUCCESS, status)
      << "clGetKernelWorkGroupInfo(CL_KERNEL_PRIVATE_MEM_SIZE) failed";
  ASSERT_EQ(param_value_size_ret, sizeof(cl_ulong))
      << "Wrong buffer data size returned";

  /* Release stuff. */
  clReleaseKernel(krnl);
  clReleaseProgram(prg);
  clReleaseContext(ctx);
}
