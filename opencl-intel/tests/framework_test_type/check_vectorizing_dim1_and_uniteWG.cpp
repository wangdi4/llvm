// Copyright (c) 2020 Intel Corporation
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
#include "TestsHelpClasses.h"
#include "test_utils.h"
#include <iostream>

#define WORK_SIZE_DIM 5
#define VEC_FACTOR 16

extern cl_device_type gDeviceType;

static const char *sProg = R"(
__kernel void test(__global int *dst, ulong size)
{
  if (get_global_id(0) < size)
    dst[get_global_id(0)] = 1;
  else
    dst[get_global_id(0)] = 64;
}
)";

bool clCheckVectorizingDim1AndUniteWG(bool hasNonUniformWG) {
  bool bResult = true;
  cl_int iRet = CL_SUCCESS;
  cl_platform_id platform = NULL;
  cl_device_id device = NULL;
  cl_context context = NULL;
  cl_command_queue queue = NULL;
  cl_program program = NULL;
  cl_kernel kernel = NULL;

  try {
    iRet = clGetPlatformIDs(1, &platform, NULL);
    CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);
    iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
    CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

    const cl_context_properties context_prop[3] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};
    context =
        clCreateContextFromType(context_prop, gDeviceType, NULL, NULL, &iRet);
    CheckException("clCreateContextFromType", CL_SUCCESS, iRet);

    queue = clCreateCommandQueueWithProperties(context, device, NULL, &iRet);
    CheckException("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);

    const size_t szLengths = {strlen(sProg)};
    program = clCreateProgramWithSource(context, 1, (const char **)&sProg,
                                        &szLengths, &iRet);

    CheckException("clCreateProgramWithSource", CL_SUCCESS, iRet);
    iRet = clBuildProgram(program, 1, &device, "-cl-std=CL2.0", NULL, NULL);
    CheckException("clBuildProgram", CL_SUCCESS, iRet);

    kernel = clCreateKernel(program, "test", &iRet);
    CheckException("clCreateKernel", CL_SUCCESS, iRet);

    cl_uint max_compute_units = 0;
    cl_ulong max_mem_alloc_size = 0;

    iRet = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS,
                           sizeof(max_compute_units), &max_compute_units, NULL);
    CheckException("clGetDeviceInfo", CL_SUCCESS, iRet);

    iRet =
        clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                        sizeof(max_mem_alloc_size), &max_mem_alloc_size, NULL);
    CheckException("clGetDeviceInfo", CL_SUCCESS, iRet);

    size_t local_work_size = WORK_SIZE_DIM;
    size_t global_work_size = local_work_size * max_compute_units * VEC_FACTOR;

    if (hasNonUniformWG) {
      global_work_size += 1;
    }

    cl_uint buffer_size =
        (global_work_size / local_work_size + hasNonUniformWG) *
        local_work_size * sizeof(int);
    if (buffer_size > max_mem_alloc_size) {
      buffer_size = max_mem_alloc_size;
      global_work_size = max_mem_alloc_size / sizeof(int);
    }

    // Create buffer.
    cl_mem buffer;
    buffer = clCreateBuffer(context, (cl_mem_flags)(CL_MEM_READ_WRITE),
                            buffer_size, NULL, &iRet);
    CheckException("clCreateBuffer", CL_SUCCESS, iRet);

    // Fill the buffer.
    const int fill_pattern = 0x0;
    iRet = clEnqueueFillBuffer(queue, buffer, (void *)&fill_pattern,
                               sizeof(fill_pattern), 0, buffer_size, 0, NULL,
                               NULL);
    CheckException("clEnqueueFillBuffer", CL_SUCCESS, iRet);

    // Set kernel arguments.
    iRet = clSetKernelArg(kernel, 0, sizeof(buffer), &buffer);
    CheckException("clSetKernelArg", CL_SUCCESS, iRet);
    cl_ulong gw_size_ulong = global_work_size;
    iRet =
        clSetKernelArg(kernel, 1, sizeof(gw_size_ulong), &gw_size_ulong);
    CheckException("clSetKernelArg", CL_SUCCESS, iRet);

    // Enqueue kernel.
    iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size,
                                  &local_work_size, 0, NULL, NULL);
    CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

    // Compare result.
    void *mapped = clEnqueueMapBuffer(queue, buffer, CL_TRUE, CL_MAP_READ, 0,
                                      buffer_size, 0, NULL, NULL, &iRet);
    CheckException("clEnqueueMapBuffer", CL_SUCCESS, iRet);

    size_t errors = 0;
    cl_uint *data = (cl_uint *)mapped;
    for (cl_uint i = 0; i < buffer_size / sizeof(int); ++i) {
      if ((i < global_work_size && data[i] != 1) ||
          (i >= global_work_size && data[i] != 0))
        errors++;
    }

    if (errors > 0) {
      bResult = false;
      std::cout << "errors: " << errors << std::endl;
    }

    iRet = clEnqueueUnmapMemObject(queue, buffer, mapped, 0, NULL, NULL);
    CheckException("clEnqueueUnmapMemObject", CL_SUCCESS, iRet);

    // Release buffer.
    if (buffer) {
      clReleaseMemObject(buffer);
    }
  } catch (const std::exception &exe) {
    cerr << exe.what() << endl;
    bResult = false;
  }
  if (kernel) {
    clReleaseKernel(kernel);
  }
  if (queue) {
    clReleaseCommandQueue(queue);
  }
  if (program) {
    clReleaseProgram(program);
  }
  if (context) {
    clReleaseContext(context);
  }
  return bResult;
}
