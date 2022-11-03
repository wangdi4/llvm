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
#include <sstream>

#ifndef VR16_GTEST_
#define VR16_GTEST_

//|  TEST: SharedKernels (TC-83)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability to queue commands to execute a shared kernel on CPU
// device and a |  different shared kernel on GPU device.
//|
//|  Method
//|  ------
//|
//|  1. Create shared 2 kernels (shared on CPU and GPU devices)
//|  2. Enqueue one of the kernels on CPU and another on GPU queues, both
// kernels are waiting for user event |  3. Check that both kernels statuses are
// CL_QUEUED |  3. Set user to CL_COMPLETE |  4. Wait for kernels to complete
// their execution
//|
//|  Pass criteria
//|  -------------
//|
//|  Verify that both kernels were in CL_QUEUED status after NDRange and
// before their execution
//|

static void testSharedKernelsBody(OpenCLDescriptor &ocl_descriptor) {
  // set up OpenCL context, program and queues
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

  // create 2 shared kernels
  ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[0],
                                       ocl_descriptor.program, "kernel_0"));
  ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[1],
                                       ocl_descriptor.program, "kernel_1"));

  int arraySize = (size_t)128;
  DynamicArray<int> input_array(arraySize);
  // create shared buffer
  ASSERT_NO_FATAL_FAILURE(createBuffer(
      &ocl_descriptor.in_common_buffer, ocl_descriptor.context,
      CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
      sizeof(int) * input_array.dynamic_array_size, input_array.dynamic_array));

  // set up manual destruction of input_array
  ASSERT_NO_FATAL_FAILURE(
      setDeleteArrayOnCallback(ocl_descriptor.in_common_buffer, input_array));

  for (int i = 0; i < 2; ++i) {
    // set kernel arguments
    ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 0,
                                         sizeof(cl_mem),
                                         &ocl_descriptor.in_common_buffer));
    ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[i], 1,
                                         sizeof(int),
                                         &input_array.dynamic_array_size));
  }

  // create user event
  cl_event user_event = 0;
  cl_event device_done_event[] = {0, 0};
  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  // enqueue both kernels with required dependency (CPU then GPU)
  cl_uint work_dim = 1;
  size_t global_work_size = 1;

  // enqueue kernels
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
        ocl_descriptor.queues[i], ocl_descriptor.kernels[i], 1, NULL,
        &global_work_size, NULL, 1, &user_event, &device_done_event[i]));
  }

  // wait
  sleepMS(100);

  // check that both kernels are sill CL_QUEUED or CL_SUBMITTED
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(validateQueuedOrSubmitted(device_done_event[i]));
  }
  // set user event as CL_COMPLETE
  ASSERT_NO_FATAL_FAILURE(setUserEventStatus(user_event, CL_COMPLETE));

  // wait for completion of kernels execution
  ASSERT_NO_FATAL_FAILURE(waitForEvents(2, device_done_event));

  releaseEvent(user_event);
  for (int i = 0; i < 2; i++) {
    releaseEvent(device_done_event[i]);
  }
}

#endif /* VR16_GTEST_ */
