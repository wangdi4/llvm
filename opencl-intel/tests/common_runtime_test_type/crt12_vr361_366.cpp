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

class CRT12_VR_361_366 : public CommonRuntime {};

//|  TEST: CRT12_VR_361_366.EnqueueMarkerWithWaitList_vr361
//|
//|  Purpose
//|  -------
//|  an auxilary
//|  Verify that the function clEnqueueMarkerWithWaitList() is abale to queue
// a marker command to the command-queues to to synchronize between the host and
// devices (CPU) .
//|
//|  Method
//|  ------
//|
//|  1.  Create context and such
//| 2.  Create a user event
//| 3.  Create a marker using the user event
//| 4.  Enqueue a simple kernel in device with the created user event
//| 5.  Run the kernel
//| 6.  Wait
//| 7.  Check thet the event changed to  CL_COMPLETE
//|
//| there are two options of device so well have 3 test cases
//|
//|  Pass criteria
//|  -------------
//|
//|  kernel should wait for the use event.
//|

void testEnqueueMarkerWithWaitList(OpenCLDescriptor ocl_descriptor,
                                   cl_int marker_device, cl_int kernel_device) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_int num = 0;
  cl_int event_status = 0;
  cl_int array[1];
  cl_event user_event = 0;
  cl_event marker_event = 0;
  cl_event kernel_event = 0;
  // set up shared context, program and queues with kernel1
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

  // create events
  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  // create marker
  ASSERT_NO_FATAL_FAILURE(enqueueMarkerWithWaitList(
      ocl_descriptor.queues[marker_device], 1, &user_event, &marker_event));

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

  // enqueue kernel on kernel_device with use event
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[kernel_device], ocl_descriptor.kernels[0], work_dim,
      0, &global_work_size, &local_work_size, 1, &marker_event, &kernel_event));

  // wait
  sleepMS(1000);

  // check that the kernel is not running
  ASSERT_NO_FATAL_FAILURE(getEventInfo(kernel_event,
                                       CL_EVENT_COMMAND_EXECUTION_STATUS,
                                       sizeof(cl_int), &event_status));
  if ((CL_QUEUED != event_status) &&
      (CL_SUBMITTED != event_status)) { // this was not made with ASSERT becouse
                                        // of some wierd problem with gtest
    FAIL() << "event status isnt set to CL_QUEUED or CL_SUBMITTED as expected";
  }

  // activate the user event
  ASSERT_NO_FATAL_FAILURE(setUserEventStatus(user_event, CL_COMPLETE));

  // wait
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[marker_device]));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[kernel_device]));

  // check that the kernel was excuted
  ASSERT_NO_FATAL_FAILURE(getEventInfo(kernel_event,
                                       CL_EVENT_COMMAND_EXECUTION_STATUS,
                                       sizeof(cl_int), &event_status));
  ASSERT_EQ(CL_COMPLETE, event_status)
      << "event status isnt set to CL_COMPLETE as expected";

  ASSERT_NO_FATAL_FAILURE(releaseEvent(user_event));
  ASSERT_NO_FATAL_FAILURE(releaseEvent(marker_event));
  ASSERT_NO_FATAL_FAILURE(releaseEvent(kernel_event));
}

//|  TEST: CRT12_VR_361_366.EnqueueMarkerWithWaitListCPU_vr361 (TC-1)
//|
//| marker = CPU
//| device = CPU
//|

TEST_F(CRT12_VR_361_366, EnqueueMarkerWithWaitListCPU) {
  ASSERT_NO_FATAL_FAILURE(
      testEnqueueMarkerWithWaitList(ocl_descriptor, CPU_DEVICE, CPU_DEVICE));
}
//|  TEST: CRT12_VR_361_366.EnqueueMarkerWithWaitListGPU_vr361 (TC-2)
//|
//| marker = GPU
//| device = GPU
//|

TEST_F(CRT12_VR_361_366, EnqueueMarkerWithWaitListGPU) {
  ASSERT_NO_FATAL_FAILURE(
      testEnqueueMarkerWithWaitList(ocl_descriptor, GPU_DEVICE, GPU_DEVICE));
}
//|  TEST: CRT12_VR_361_366.EnqueueMarkerWithWaitListCPU_vr361 (TC-3)
//|
//| marker = CPU
//| device = GPU
//|

TEST_F(CRT12_VR_361_366, EnqueueMarkerWithWaitListCPUGPU) {
  ASSERT_NO_FATAL_FAILURE(
      testEnqueueMarkerWithWaitList(ocl_descriptor, CPU_DEVICE, GPU_DEVICE));
}
//|  TEST: CRT12_VR_361_366.EnqueueMarkerWithWaitListGPUCPU_vr361 (TC-4)
//|
//| marker = GPU
//| device = CPU
//|

TEST_F(CRT12_VR_361_366, DISABLED_EnqueueMarkerWithWaitListGPUCPU) {
  ASSERT_NO_FATAL_FAILURE(
      testEnqueueMarkerWithWaitList(ocl_descriptor, GPU_DEVICE, CPU_DEVICE));
}

//|  TEST: CRT12_VR_361_366.EnqueueBarrierWithWaitList_vr366
//|
//|  Purpose
//|  -------
//|
//|  Verify that the function clEnqueuebufferWithWaitList() is abale to queue
// a buffer command to the command-queues to to synchronize between the host and
// devices .
//|
//|  Method
//|  ------
//|
//|  1.  Create context and such
//|  2.  Create a user event
//|  3.  Enqueue a barrier with clEnqueueBarrierWithWaitList() on device
// with the created user event
//|  4.  Enqueue a simple kernel on device with the created user event
//|  5.  Run the kernel
//|  6.  Wait
//|  7.  Check that the buffer and the kernel ware not executed.
//|  8.  Change the use event to CL_COMPLETE
//|  9.  Wait
//|  10.  Check that the kernel was executed
//|
//|  Pass criteria
//|  -------------
//|
//|  kernel should wait for the use event.
//|
//| we have two device types so will have four test cases
//|

void enqueueBarrierWithWaitListCPU(OpenCLDescriptor ocl_descriptor,
                                   cl_int barrier_device,
                                   cl_int kernel_device) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_int num = 0;
  cl_int event_status = 0;
  cl_int array[1];
  cl_event user_event = 0;
  cl_event barrier_event = 0;
  cl_event kernel_event = 0;
  // set up shared context, program and queues with kernel1
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

  // create events
  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));
  // create barrier
  ASSERT_NO_FATAL_FAILURE(enqueueBarrierWithWaitList(
      ocl_descriptor.queues[barrier_device], 1, &user_event, &barrier_event));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(
      createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_0"));

  // initialize parameters for kernel.
  ASSERT_NO_FATAL_FAILURE(createBuffer(
      &ocl_descriptor.in_common_buffer, ocl_descriptor.context,
      CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int), array));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem),
                   (void *)&ocl_descriptor.in_common_buffer));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));

  // enqueue kernel on CPU with use event
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[kernel_device], ocl_descriptor.kernels[0], work_dim,
      0, &global_work_size, &local_work_size, 1, &barrier_event,
      &kernel_event));

  // wait
  sleepMS(1000);

  // check that the kernel is not running
  ASSERT_NO_FATAL_FAILURE(getEventInfo(kernel_event,
                                       CL_EVENT_COMMAND_EXECUTION_STATUS,
                                       sizeof(cl_int), &event_status));
  if ((CL_QUEUED != event_status) &&
      (CL_SUBMITTED != event_status)) { // this was not made with ASSERT becouse
                                        // of some wierd problem with gtest
    FAIL() << "event status isnt set to CL_QUEUED or CL_SUBMITTED as expected";
  }

  // activate the user event
  setUserEventStatus(user_event, CL_COMPLETE);

  // wait
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[barrier_device]));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[kernel_device]));

  // check that the kernel was executed
  ASSERT_NO_FATAL_FAILURE(getEventInfo(kernel_event,
                                       CL_EVENT_COMMAND_EXECUTION_STATUS,
                                       sizeof(cl_int), &event_status));
  ASSERT_EQ(CL_COMPLETE, event_status)
      << "event status isnt set to CL_COMPLETE as expected";
}
//|  TEST: CRT12_VR_361_366.EnqueueBarrierWithWaitListCPU_vr366 (TC-1)
//|
//| barrier = CPU
//| device = CPU
//|

TEST_F(CRT12_VR_361_366, EnqueueBarrierWithWaitListCPU) {
  ASSERT_NO_FATAL_FAILURE(
      enqueueBarrierWithWaitListCPU(ocl_descriptor, CPU_DEVICE, CPU_DEVICE));
}
//|  TEST: CRT12_VR_361_366.EnqueueBarrierWithWaitListGPU_vr366 (TC-2)
//|
//| barrier = GPU
//| device = GPU
//|

TEST_F(CRT12_VR_361_366, EnqueueBarrierWithWaitListGPU) {
  ASSERT_NO_FATAL_FAILURE(
      enqueueBarrierWithWaitListCPU(ocl_descriptor, GPU_DEVICE, GPU_DEVICE));
}

//|  TEST: CRT12_VR_361_366.EnqueueBarrierWithWaitListCPUGPU_vr366 (TC-3)
//|
//| barrier = CPU
//| device = GPU
//|

TEST_F(CRT12_VR_361_366, EnqueueBarrierWithWaitListCPUGPU) {
  ASSERT_NO_FATAL_FAILURE(
      enqueueBarrierWithWaitListCPU(ocl_descriptor, CPU_DEVICE, GPU_DEVICE));
}

//|  TEST: CRT12_VR_361_366.EnqueueBarrierWithWaitListGPUCPU_vr366 (TC-4)
//|
//| barrier = GPU
//| device = CPU
//|

TEST_F(CRT12_VR_361_366, EnqueueBarrierWithWaitListGPUCPU) {
  ASSERT_NO_FATAL_FAILURE(
      enqueueBarrierWithWaitListCPU(ocl_descriptor, GPU_DEVICE, CPU_DEVICE));
}
