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

#include "common_runtime_tests.h"

#define CPU_DEVICE 0
#define GPU_DEVICE 1

class CRT12_VR268_270 : public CommonRuntime {};

void checkCompilationSuccess(OpenCLDescriptor ocl_descriptor, cl_int device1,
                             cl_int device2) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_int num = 0;
  cl_int array[1];
  cl_device_type device_type;

  if (device1 == 0) {
    device_type = CL_DEVICE_TYPE_CPU;
  } else {
    device_type = CL_DEVICE_TYPE_GPU; // this should work of mic as well..
  }
  // set up shared context, program and queues for device1
  ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesForSingelDevice(
      ocl_descriptor, "simple_kernels.cl", CL_DEVICE_TYPE_CPU));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(
      createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_0"));

  // initialize parameters for kernel.
  ASSERT_NO_FATAL_FAILURE(createBuffer(
      &ocl_descriptor.in_common_buffer, ocl_descriptor.context,
      CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int), array));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem),
                   (void *)&ocl_descriptor.in_common_buffer));

  // run kernel on device2, should succed
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[device2], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));
}

void checkCompilationFail(OpenCLDescriptor ocl_descriptor, cl_int device1,
                          cl_int device2) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_int num = 0;
  cl_int array[1];
  cl_device_type device_type;

  if (device1 == 0) {
    device_type = CL_DEVICE_TYPE_CPU;
  } else {
    device_type = CL_DEVICE_TYPE_GPU; // this should work of mic as well..
  }
  // set up shared context, program and queues for device1
  ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesForSingelDevice(
      ocl_descriptor, "simple_kernels.cl", CL_DEVICE_TYPE_CPU));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(
      createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_0"));

  // initialize parameters for kernel.
  ASSERT_NO_FATAL_FAILURE(createBuffer(
      &ocl_descriptor.in_common_buffer, ocl_descriptor.context,
      CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int), array));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem),
                   (void *)&ocl_descriptor.in_common_buffer));

  // run kernel on device2, should fail
  cl_uint ret = clEnqueueNDRangeKernel(
      ocl_descriptor.queues[1], ocl_descriptor.kernels[0], 1, NULL,
      &global_work_size, &local_work_size, 0, NULL, NULL);
  ASSERT_EQ(CL_INVALID_PROGRAM_EXECUTABLE, ret)
      << "return value was not CL_INVALID_PROGRAM_EXECUTABLE";
}
//|  TEST: CRT12_VR268_270.CompileForCPUOnCPU_vr268 (TC-1)
//|
//| Device1 = CPU
//|
//|  Device2 = CPU
//|
//|
//|

TEST_F(CRT12_VR268_270, CompileForCPUOnCPU_vr268) {
  checkCompilationSuccess(ocl_descriptor, CPU_DEVICE, CPU_DEVICE);
}

//|  TEST: CRT12_VR268_270.CompileForCPUOnGPU_vr268 (TC-2)
//|
//| Device1 = CPU
//|
//|  Device2 = GPU
//|
//|
//|

TEST_F(CRT12_VR268_270, CompileForCPUOnGPU_vr268) {
  checkCompilationFail(ocl_descriptor, CPU_DEVICE, GPU_DEVICE);
}

//|  TEST: CRT12_VR268_270.CompileForGPUOnGPU_vr269 (TC-1)
//|
//| Device1 = GPU
//|
//|  Device2 = GPU
//|
//|
//|

TEST_F(CRT12_VR268_270, CompileForGPUOnGPU_vr269) {
  checkCompilationSuccess(ocl_descriptor, CPU_DEVICE, CPU_DEVICE);
}

//|  TEST: CRT12_VR268_270.CompileForGPUOnCPU_vr269 (TC-2)
//|
//| Device1 = CPU
//|
//|  Device2 = GPU
//|
//|
//|

TEST_F(CRT12_VR268_270, CompileForGPUOnCPU_vr268) {
  checkCompilationFail(ocl_descriptor, GPU_DEVICE, CPU_DEVICE);
}

//|  TEST: CRT12_VR268_270.CompileForGPUOnGPU_vr270
//|
//|  Purpose
//|  -------
//|
//|  Verify that a program can be compiled for both CPU and GPU.
//|
//|  Method
//|  ------
//|
//|  1.  Create a program for GPU and CPU only
//| 2.  Run the program on a CPU and GPU device
//| 3.  Expect success
//|
//|  Pass criteria
//|  -------------
//|
//|  should succeed on CPU and GPU
//|

TEST_F(CRT12_VR268_270, CompileForGPUOnGPU_vr270) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_int num = 0;
  cl_int array[1];

  // set up shared context, program and queues for CPU
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(
      createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_0"));

  // initialize parameters for kernel.
  ASSERT_NO_FATAL_FAILURE(createBuffer(
      &ocl_descriptor.in_common_buffer, ocl_descriptor.context,
      CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int), array));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem),
                   (void *)&ocl_descriptor.in_common_buffer));

  // run kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // run kernel on GPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));
}
