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

class CRT12_VR252 : public CommonRuntime {};

//|  TEST: CRT12_VR252.EnqueueMigrateMemObjects (TC-1)
//|
//|  Purpose
//|  -------
//|
//|  Verify that the function clEnqueueMigrateMemObjects() is able to queue
// a buffer command to the command-queues to to synchronize between the host and
// devices .
//|
//|  Method
//|  ------
//|
//| 1.  Create context and such
//| 2.  Create a user event
//| 3.  Create a memobject
//| 4.  Use clEnqueueMigrateMemObjects to allocate the mem object to the the
// device and using the user event | 5.  Enqueue a simple kernel in on device
// with the mem object event event | 6.  Wait
//| 7.  Check that the kernel was not executed.
//| 8.  Change the user event to CL_COMPLETE
//| 9.  Wait
//| 11.  Check that the kernel was executed
//|
//|
//|  Pass criteria
//|  -------------
//|
//|  kernel should wait for the use event.
//|

// maybe i should add another event before the buffer?

void checkMigrateObj(OpenCLDescriptor ocl_descriptor, cl_int device) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_int num = 0;
  cl_int event_status = 0;
  cl_int array[1];
  cl_event user_event = 0;
  cl_event migration_event = 0;
  cl_event kernel_event = 0;
  // set up shared context, program and queues with kernel1
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

  // create user event
  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(
      createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_0"));

  // initialize parameters for kernel.
  ASSERT_NO_FATAL_FAILURE(createBuffer(
      &ocl_descriptor.in_common_buffer, ocl_descriptor.context,
      CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int), array));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0,
                                       sizeof(cl_mem),
                                       &ocl_descriptor.in_common_buffer));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));

  // migrate mem object
  ASSERT_NO_FATAL_FAILURE(enqueueMigrateMemObjects(
      ocl_descriptor.queues[device], 1, &ocl_descriptor.in_common_buffer, 0, 1,
      &user_event, &migration_event));

  // enqueue kernel on CPU with use event

  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[device], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 1, &migration_event, &kernel_event));

  // wait
  sleepMS(100);

  // check that the kernel is not runing
  clGetEventInfo(kernel_event, CL_EVENT_COMMAND_EXECUTION_STATUS,
                 sizeof(cl_int), &event_status, NULL);
  ASSERT_EQ(CL_SUBMITTED, event_status) << "event status is not CL_SUBMITTED";

  // activate the user event
  setUserEventStatus(user_event, CL_COMPLETE);

  // wait
  sleepMS(1000);

  // check that the kernel was excuted
  ASSERT_NO_FATAL_FAILURE(getEventInfo(kernel_event,
                                       CL_EVENT_COMMAND_EXECUTION_STATUS,
                                       sizeof(cl_int), &event_status));
  ASSERT_EQ(CL_COMPLETE, event_status) << "event status is not CL_COMPLETE";
}

TEST_F(CRT12_VR252, EnqueueMigrateMemObjectsCPU) {
  ASSERT_NO_FATAL_FAILURE((ocl_descriptor, CPU_DEVICE));
}

TEST_F(CRT12_VR252, EnqueueMigrateMemObjectsGPU) {
  ASSERT_NO_FATAL_FAILURE((ocl_descriptor, GPU_DEVICE));
}
