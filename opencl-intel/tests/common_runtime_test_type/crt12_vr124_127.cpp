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

#include "common_methods.h"
#include "common_runtime_tests.h"
#include "dynamic_array.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define CPU_DEVICE 0
#define GPU_DEVICE 1

class CRT12_VR_124_127 : public CommonRuntime {};

//|  TEST: CRT12_VR_124_127.FillBufferSynchronized_vr124
//|
//|  Purpose
//|  -------
//|
//|  Verify support for the use of events to synchronize between the host and
// devices when using the function clEnqueueFillBuffer() to queue a fill command
// to the command-queues.
//|
//|  Method
//|  ------
//|
//|  1. Create a shared context
//| 2. Create an out-of-order queue to the CPU device
//| 3. Create an in-order queue to the GPU device
//| 4. Create a user-event
//| 5. Create two buffers (CPU-Buffer and GPU-Buffer)
//| 6. Queue Fill-Buffer command for CPU-Buffer to the CPU commands-queue with
// the user-event in the event_wait_list and pointer to CPU-Event in event | 7.
// Queue Fill-Buffer command for GPU-Buffer to the GPU commands-queue with the
// user-event in the event_wait_list and pointer to GPU-Event in event | 8.
// Queue Read-Buffer command for CPU-Buffer to the CPU commands-queue with
// CPU-event and GPU-event in the  event_wait_list | 9. Check that none of the
// commands is executed, yet | 10. Set the user-event to CL_COMPLETE | 11. Check
// that all the commands were completed and the CPU-Buffer has the right pattern
//|
//|  Pass criteria
//|  -------------
//|
//|  The host should be able to receive events either from CPU device, GPU
// device, or both of them.
//|

TEST_F(CRT12_VR_124_127,
       DISABLED_FillBufferSynchronized_vr124) { // I think there's a dead lock
                                                // on line 111
  DynamicArray<cl_int> buffer1(2);
  DynamicArray<cl_int> buffer2(2);
  cl_int pattern[2] = {1, 2};

  memset(buffer1.dynamic_array, 0, sizeof(cl_int) * 8);
  memset(buffer2.dynamic_array, 0, sizeof(cl_int) * 8);
  cl_event_info event_status = 0;
  cl_event user_event = 0;
  cl_event read_event = 0;
  // those two elements will be used as list, so do not separate!
  cl_event event_list[2];

  // set up platform and CPU and GPU  devices
  //  CPU is at index 0, GPU is at index 1
  ASSERT_NO_FATAL_FAILURE(
      getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));

  // set up context
  cl_context_properties properties[] = {
      CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0],
      0};
  ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2,
                                        ocl_descriptor.devices, NULL, NULL));

  // create an out-of-order queue to the CPU device
  ASSERT_NO_FATAL_FAILURE(createCommandQueue(
      ocl_descriptor.queues, ocl_descriptor.context, ocl_descriptor.devices[0],
      CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE));

  // create an in-order queue to the GPU device
  ASSERT_NO_FATAL_FAILURE(createCommandQueue(&ocl_descriptor.queues[1],
                                             ocl_descriptor.context,
                                             ocl_descriptor.devices[1], 0));

  // create user event
  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  // create the buffer objects
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.buffers[0], ocl_descriptor.context,
                   CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int) * 8,
                   buffer1.dynamic_array));
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.buffers[1], ocl_descriptor.context,
                   CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int) * 8,
                   buffer2.dynamic_array));

  // enqueue buffers
  ASSERT_NO_FATAL_FAILURE(
      enqueueFillBuffer(ocl_descriptor.queues[0], ocl_descriptor.buffers[0],
                        pattern, sizeof(cl_int) * 2, 0, sizeof(cl_int) * 4, 1,
                        &user_event, &event_list[0]));
  ASSERT_NO_FATAL_FAILURE(
      enqueueFillBuffer(ocl_descriptor.queues[1], ocl_descriptor.buffers[1],
                        pattern, sizeof(cl_int) * 2, 0, sizeof(cl_int) * 4, 1,
                        &user_event, &event_list[1]));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[0], CL_FALSE, 0,
      sizeof(int) * 2, buffer1.dynamic_array, 2, event_list, NULL));
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[1], ocl_descriptor.buffers[1], CL_FALSE, 0,
      sizeof(int) * 2, buffer2.dynamic_array, 2, event_list, &read_event));

  sleepMS(500);

  // check that the kernel was not executed
  ASSERT_NO_FATAL_FAILURE(getEventInfo(read_event,
                                       CL_EVENT_COMMAND_EXECUTION_STATUS,
                                       sizeof(cl_int), &event_status));
  ASSERT_EQ(CL_QUEUED, event_status)
      << "device status does not set to CL_QUEUED";

  // activate the user event
  ASSERT_NO_FATAL_FAILURE(setUserEventStatus(user_event, CL_COMPLETE));

  // wait
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[0]));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[1]));

  // check that the kernel was executed
  ASSERT_NO_FATAL_FAILURE(getEventInfo(read_event,
                                       CL_EVENT_COMMAND_EXECUTION_STATUS,
                                       sizeof(cl_int), &event_status));
  ASSERT_EQ(CL_QUEUED, event_status)
      << "device status does not set to CL_COMPLETE";

  for (int i = 0; i < 2; i++) {
    printf("%d", buffer1.dynamic_array[i]);
    printf("%d", buffer2.dynamic_array[i]);
  }
  // check that output is as predicted
  ASSERT_TRUE(buffer1.dynamic_array[0] == 1)
      << "buffer was not filled properly";
  ASSERT_TRUE(buffer1.dynamic_array[1] == 2)
      << "buffer was not filled properly";
  ASSERT_TRUE(buffer2.dynamic_array[0] == 1)
      << "buffer was not filled properly";
  ASSERT_TRUE(buffer2.dynamic_array[1] == 2)
      << "buffer was not filled properly";

  ASSERT_NO_FATAL_FAILURE(releaseEvent(user_event));
  ASSERT_NO_FATAL_FAILURE(releaseEvent(read_event));
  for (int i = 0; i < 2; i++) {
    ASSERT_NO_FATAL_FAILURE(releaseEvent(event_list[i]));
  }
}

//|  TEST: CRT12_VR_124_127.MemoryObjectVisibilityReadWrite_vr126 (TC-1)
//|
//|  Purpose
//|  -------
//|
//|  Verify that a buffer object created using the function
// clCreateBuffer()and the flag CL_MEM_READ_WRITE is visible to all devices in
// context
//|
//|  Method
//|  ------
//|
//|  1. Create a shared context queues etc
//| 2. Create two buffers (CPU-Buffer and GPU-Buffer)
//| 3. run a kernel that will change the memory object
//| 4. verify that the object changed.
//|
//|  Pass criteria
//|  -------------
//|
//|  the memory object is recognized in each of the devices.
//|

TEST_F(CRT12_VR_124_127, MemoryObjectVisibilityReadWrite_vr126) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_int num = 1;
  DynamicArray<cl_int> buffer1(1);
  DynamicArray<cl_int> buffer2(1);

  memset(buffer1.dynamic_array, 0, sizeof(cl_int));
  memset(buffer2.dynamic_array, 0, sizeof(cl_int));

  // set up shared context, program and queues with kernel1
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

  // create the buffer objects
  ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[0],
                                       ocl_descriptor.context,
                                       CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                                       sizeof(cl_int), buffer1.dynamic_array));
  ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[1],
                                       ocl_descriptor.context,
                                       CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                                       sizeof(cl_int), buffer2.dynamic_array));

  // create kernel and set arguments.
  ASSERT_NO_FATAL_FAILURE(
      createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_1"));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0,
                                       sizeof(cl_mem),
                                       (void *)&ocl_descriptor.buffers[0]));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[0], CL_TRUE, 0,
      sizeof(int) * 1, buffer1.dynamic_array, 0, NULL, NULL));

  // validate result
  ASSERT_EQ(num, buffer1.dynamic_array[0]) << "result is not 1 as expected";

  // set arguments for kernel
  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0,
                                       sizeof(cl_mem),
                                       (void *)&ocl_descriptor.buffers[1]));

  // enqueue and run kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[1]));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[1], ocl_descriptor.buffers[1], CL_TRUE, 0,
      sizeof(int) * 1, buffer2.dynamic_array, 0, NULL, NULL));
  ASSERT_EQ(num, buffer2.dynamic_array[0]) << "result is not 1 as expected";
}

//|  TEST: CRT12_VR_124_127.MemoryObjectVisibilityReadOnly_vr126 (TC-2)
//|
//|  Purpose
//|  -------
//|
//|  Verify that a buffer object created using the function
// clCreateBuffer()and the flag CL_MEM_READ_ONLY is visible to all devices in
// context
//|
//|  Method
//|  ------
//|
//|  1. Create a shared context queues etc
//| 2. Create three buffers (CPU-Buffer, GPU-Buffer, and a return result buffer)
//| 3. run a kernel that will change the result buffer according to the input
// buffer | 4. verify that the object changed.
//|
//|  Pass criteria
//|  -------------
//|
//|  the memory object is recognized in each of the devices.
//|

/////////////////////////////fixed////////////////////////////
TEST_F(CRT12_VR_124_127, MemoryObjectVisibilityReadOnly_vr126) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_int num = 1;
  DynamicArray<cl_int> buffer1(1);
  DynamicArray<cl_int> buffer2(1);
  DynamicArray<cl_int> buffer3(1);

  memset(buffer1.dynamic_array, 1, sizeof(cl_int));
  memset(buffer2.dynamic_array, 0, sizeof(cl_int));
  memset(buffer3.dynamic_array, 0, sizeof(cl_int));

  buffer1.dynamic_array[0] = 1;
  buffer2.dynamic_array[0] = 2;

  // set up shared context, program and queues with kernel1
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "read_only_kernel.cl"));

  // create the buffer objects
  ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[0],
                                       ocl_descriptor.context,
                                       CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                       sizeof(cl_int), buffer1.dynamic_array));
  ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[1],
                                       ocl_descriptor.context,
                                       CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                       sizeof(cl_int), buffer2.dynamic_array));
  ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.out_common_buffer,
                                       ocl_descriptor.context,
                                       CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                                       sizeof(cl_int), buffer2.dynamic_array));

  // create kernel and set arguments.
  ASSERT_NO_FATAL_FAILURE(
      createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_0"));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0,
                                       sizeof(cl_mem),
                                       (void *)&ocl_descriptor.buffers[0]));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_mem),
                   (void *)&ocl_descriptor.out_common_buffer));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[0], ocl_descriptor.out_common_buffer, CL_TRUE, 0,
      sizeof(int) * 1, buffer3.dynamic_array, 0, NULL, NULL));

  // validate result
  ASSERT_EQ(num, buffer3.dynamic_array[0]) << "result is not 1 as expected";
  buffer2.dynamic_array[0] = 2;

  // set arguments for GPU
  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0,
                                       sizeof(cl_mem),
                                       (void *)&ocl_descriptor.buffers[1]));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));
  finish(ocl_descriptor.queues[1]);
  num = 2;
  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[1], ocl_descriptor.out_common_buffer, CL_TRUE, 0,
      sizeof(int) * 1, buffer3.dynamic_array, 0, NULL, NULL));
  finish(ocl_descriptor.queues[1]);
  ASSERT_EQ(num, buffer3.dynamic_array[0]) << "result is not 1 as expected";
}

//|  TEST: CRT12_VR_124_127.MemoryObjectVisibilityWriteOnly_vr126 (TC-3)
//|
//|  Purpose
//|  -------
//|
//|  Verify that a buffer object created using the function
// clCreateBuffer()and the flag CL_MEM_HOST_WRITE_ONLY is visible to all devices
// in context
//|
//|  Method
//|  ------
//|
//|  1. Create a shared context queues etc
//| 2. Create two buffers (CPU-Buffer and GPU-Buffer)
//| 3. run a kernel that will change the memory object
//| 4. verify that the object changed.
//|
//|  Pass criteria
//|  -------------
//|
//|  the memory object is recognized in each of the devices.
//|

TEST_F(CRT12_VR_124_127, MemoryObjectVisibilityWriteOnly_vr126) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_int num = 1;
  DynamicArray<cl_int> buffer1(1);
  DynamicArray<cl_int> buffer2(1);

  memset(buffer1.dynamic_array, 0, sizeof(cl_int));
  memset(buffer2.dynamic_array, 0, sizeof(cl_int));

  // set up shared context, program and queues with kernel1
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

  // create the buffer objects
  ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[0],
                                       ocl_descriptor.context,
                                       CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                       sizeof(cl_int), buffer1.dynamic_array));
  ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[1],
                                       ocl_descriptor.context,
                                       CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                       sizeof(cl_int), buffer2.dynamic_array));

  // create kernel and set arguments.
  ASSERT_NO_FATAL_FAILURE(
      createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_1"));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0,
                                       sizeof(cl_mem),
                                       (void *)&ocl_descriptor.buffers[0]));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[0], CL_TRUE, 0,
      sizeof(int) * 1, buffer1.dynamic_array, 0, NULL, NULL));

  // validate result
  ASSERT_EQ(num, buffer1.dynamic_array[0]) << "result is not 1 as expected";

  // set arguments for GPU
  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0,
                                       sizeof(cl_mem),
                                       (void *)&ocl_descriptor.buffers[1]));

  // enqueue and run kernel on GPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[1]));
  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[1], ocl_descriptor.buffers[1], CL_TRUE, 0,
      sizeof(int) * 1, buffer2.dynamic_array, 0, NULL, NULL));
  ASSERT_EQ(num, buffer2.dynamic_array[0]) << "result is not 1 as expected";
}

//|  TEST: CRT12_VR_124_127.SubBufferObjectVisibilityReadWrite_vr127_VR127
//(TC-1)
//|
//|  Purpose
//|  -------
//|
//|  Verify that a buffer object created using the function
// clCreateBuffer()and the flag CL_MEM_HOST_WRITE_ONLY is visible to all devices
// in context
//|
//|  Method
//|  ------
//|
//|  1. Create a shared context queues etc
//| 2. Create two buffers (CPU-Buffer and GPU-Buffer)
//| 3. create a sub buffer for each buffer
//| 3. run a kernel that will change the sub buffer
//| 4. verify that the object changed.
//|
//|  Pass criteria
//|  -------------
//|
//|  the memory object is recognized in each of the devices.
//|

TEST_F(CRT12_VR_124_127, SubBufferVisibilityReadWrite_VR127) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_int num = 1;
  DynamicArray<cl_int> buffer1(2);
  DynamicArray<cl_int> buffer2(2);
  cl_buffer_region region;
  memset(buffer1.dynamic_array, 0, sizeof(cl_int));
  memset(buffer2.dynamic_array, 0, sizeof(cl_int));

  // initialize cl_buffer_region
  region.origin = 0;
  region.size = sizeof(cl_int);

  // set up shared context, program and queues with kernel1
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

  // create the buffer objects
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.buffers[0], ocl_descriptor.context,
                   CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int) * 2,
                   buffer1.dynamic_array));
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.buffers[1], ocl_descriptor.context,
                   CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int) * 2,
                   buffer2.dynamic_array));

  // create sub buffer object.
  ASSERT_NO_FATAL_FAILURE(createSubBuffer(
      &ocl_descriptor.in_common_sub_buffer, ocl_descriptor.buffers[0],
      CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &region));

  // create kernel and set arguments.
  ASSERT_NO_FATAL_FAILURE(
      createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_1"));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem),
                   (void *)&ocl_descriptor.in_common_sub_buffer));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[0], CL_TRUE, 0,
      sizeof(int) * 1, buffer1.dynamic_array, 0, NULL, NULL));

  // validate result
  ASSERT_EQ(num, buffer1.dynamic_array[0]) << "result is not 1 as expected";

  // initialize parameters for GPU
  ASSERT_NO_FATAL_FAILURE(createSubBuffer(
      &ocl_descriptor.in_common_sub_buffer, ocl_descriptor.buffers[1],
      CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &region));
  // ASSERT_NO_FATAL_FAILURE(createSubBuffer(&ocl_descriptor.in_common_sub_buffer,ocl_descriptor.buffers[1],CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,CL_BUFFER_CREATE_TYPE_REGION,&region));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem),
                   (void *)&ocl_descriptor.in_common_sub_buffer));

  // set arguments for GPU
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem),
                   (void *)&ocl_descriptor.in_common_sub_buffer));

  // enqueue kernel on GPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from GPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[1], ocl_descriptor.buffers[1], CL_TRUE, 0,
      sizeof(int) * 1, buffer2.dynamic_array, 0, NULL, NULL));
  ASSERT_EQ(num, buffer2.dynamic_array[0]) << "result is not 1 as expected";
}

//|  TEST: CRT12_VR_124_127.SubBufferVisibilityReadOnly_VR127 (TC-2)
//|
//|  Purpose
//|  -------
//|
//|  Verify that a buffer object created using the function
// clCreateBuffer()and the flag CL_MEM_HOST_WRITE_ONLY is visible to all devices
// in context
//|
//|  Method
//|  ------
//|
//|  1. Create a shared context queues etc
//| 2. Create three buffers (CPU-Buffer, GPU-Buffer, and a return result buffer)
//| 3. run a kernel that will change the result buffer according to the input
// buffer | 4. verify that the object changed.
//|
//|  Pass criteria
//|  -------------
//|
//|  the memory object is recognized in each of the devices.
//|

TEST_F(CRT12_VR_124_127, DISABLED_SubBufferVisibilityReadOnly_VR127) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_int num = 1;
  DynamicArray<cl_int> buffer1(2);
  DynamicArray<cl_int> buffer2(2);
  cl_buffer_region region;
  memset(buffer1.dynamic_array, 0, sizeof(cl_int));
  memset(buffer2.dynamic_array, 0, sizeof(cl_int));

  // initialize cl_buffer_region
  region.origin = 0;
  region.size = sizeof(cl_int);

  // set up shared context, program and queues with kernel1
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

  // create the buffer objects
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.buffers[0], ocl_descriptor.context,
                   CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(cl_int) * 2,
                   buffer1.dynamic_array));
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.buffers[1], ocl_descriptor.context,
                   CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(cl_int) * 2,
                   buffer2.dynamic_array));

  // create sub buffer object.
  ASSERT_NO_FATAL_FAILURE(createSubBuffer(
      &ocl_descriptor.in_common_sub_buffer, ocl_descriptor.buffers[0],
      CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &region));

  // create kernel and set arguments.
  ASSERT_NO_FATAL_FAILURE(
      createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_1"));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem),
                   (void *)&ocl_descriptor.in_common_sub_buffer));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[0], CL_TRUE, 0,
      sizeof(int) * 1, buffer1.dynamic_array, 0, NULL, NULL));

  // validate result
  ASSERT_EQ(num, buffer1.dynamic_array[0]) << "result is not 1 as expected";

  // initialize parameters for GPU
  ASSERT_NO_FATAL_FAILURE(createSubBuffer(
      &ocl_descriptor.in_common_sub_buffer, ocl_descriptor.buffers[1],
      CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &region));
  // ASSERT_NO_FATAL_FAILURE(createSubBuffer(&ocl_descriptor.in_common_sub_buffer,ocl_descriptor.buffers[1],CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,CL_BUFFER_CREATE_TYPE_REGION,&region));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem),
                   (void *)&ocl_descriptor.in_common_sub_buffer));

  // set arguments for GPU
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem),
                   (void *)&ocl_descriptor.in_common_sub_buffer));

  // enqueue kernel on GPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from GPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[1], ocl_descriptor.buffers[1], CL_TRUE, 0,
      sizeof(int) * 1, buffer2.dynamic_array, 0, NULL, NULL));
  ASSERT_EQ(num, buffer2.dynamic_array[0]) << "result is not 1 as expected";
}

//|  TEST: CRT12_VR_124_127.SubBufferVisibilityWriteOnly (TC-3)
//|
//|  Purpose
//|  -------
//|
//|  Verify that a buffer object created using the function
// clCreateBuffer()and the flag CL_MEM_HOST_WRITE_ONLY is visible to all devices
// in context
//|
//|  Method
//|  ------
//|
//|  1. Create a shared context queues etc
//| 2. Create two buffers (CPU-Buffer and GPU-Buffer)
//| 3. run a kernel that will change the memory object
//| 4. verify that the object changed.
//|
//|  Pass criteria
//|  -------------
//|
//|  the memory object is recognized in each of the devices.
//|

TEST_F(CRT12_VR_124_127, SubBufferVisibilityWriteOnly_VR127) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_int num = 1;
  cl_buffer_region region;
  DynamicArray<cl_int> buffer1(1);
  DynamicArray<cl_int> buffer2(1);

  memset(buffer1.dynamic_array, 0, sizeof(cl_int));
  memset(buffer2.dynamic_array, 0, sizeof(cl_int));

  // initialize cl_buffer_region
  region.origin = 0;
  region.size = sizeof(cl_int);

  // set up shared context, program and queues with kernel1
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

  // create the buffer objects
  ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[0],
                                       ocl_descriptor.context,
                                       CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                       sizeof(cl_int), buffer1.dynamic_array));
  ASSERT_NO_FATAL_FAILURE(createBuffer(&ocl_descriptor.buffers[1],
                                       ocl_descriptor.context,
                                       CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                       sizeof(cl_int), buffer2.dynamic_array));

  // create kernel and set arguments.
  ASSERT_NO_FATAL_FAILURE(
      createKernel(ocl_descriptor.kernels, ocl_descriptor.program, "kernel_1"));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0,
                                       sizeof(cl_mem),
                                       (void *)&ocl_descriptor.buffers[0]));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_int), &num));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[0], CL_TRUE, 0,
      sizeof(int) * 1, buffer1.dynamic_array, 0, NULL, NULL));

  // validate result
  ASSERT_EQ(num, buffer1.dynamic_array[0]) << "result is not 1 as expected";

  // initialize parameters for GPU
  ASSERT_NO_FATAL_FAILURE(createSubBuffer(
      &ocl_descriptor.in_common_sub_buffer, ocl_descriptor.buffers[1],
      CL_MEM_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &region));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem),
                   (void *)&ocl_descriptor.in_common_sub_buffer));

  // set arguments for GPU
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem),
                   (void *)&ocl_descriptor.in_common_sub_buffer));

  // enqueue kernel on GPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[1], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from GPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[1], ocl_descriptor.buffers[1], CL_TRUE, 0,
      sizeof(int) * 1, buffer2.dynamic_array, 0, NULL, NULL));
  ASSERT_EQ(num, buffer2.dynamic_array[0]) << "result is not 1 as expected";
}
//|  TEST: CRT12_VR_124_127.subBufferSync
//|
//|  Purpose
//|  -------
//|
//|  Verify that OCL platform can support and use one sub buffer on two
// devices in the same time
//|
//|  Method
//|  ------
//|
//|  1. Create a shared context queues etc
//| 2. Create 2 buffers
//| 3. split the buffers into 4 sub buffers (2 for each device)
//| 4. copy sub buffer from the input buffer to result buffer
//|  5. validate result
//|
//|  Pass criteria
//|  -------------
//|
//|  the memory object is recognized in each of the devices.
//|

void validateSubBuffer(OpenCLDescriptor ocl_descriptor,
                       cl_mem_flags input_flags, cl_mem_flags output_flags,
                       cl_int first_device, cl_int second_device) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_buffer_region region;
  cl_mem sub_buffers[4];
  cl_uint device_align_size = 0, temp_size = 0;

  // set up shared context, program and queues with kernel1
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "buffer_sync_kernel.cl"));

  // get the aligned buffer sizes and craete arrays
  for (int i = 0; i < 2; i++) {
    getDeviceInfo(ocl_descriptor.devices[0], CL_DEVICE_MEM_BASE_ADDR_ALIGN,
                  sizeof(cl_uint), &temp_size);
    if (temp_size > device_align_size) {
      device_align_size = temp_size;
    }
  }
  DynamicArray<cl_int> buffer1(device_align_size * 2);
  DynamicArray<cl_int> buffer2(device_align_size * 2);
  for (cl_uint i = 0; i < device_align_size; i++) {
    buffer1.dynamic_array[i] = 1;
    buffer1.dynamic_array[i + device_align_size] = 2;
  }
  memset(buffer2.dynamic_array, 0, sizeof(cl_int) * device_align_size * 2);

  // initialize cl_buffer_region
  region.origin = 0;
  region.size = sizeof(cl_int) * device_align_size;

  // create the buffer objects
  ASSERT_NO_FATAL_FAILURE(createBuffer(
      &ocl_descriptor.buffers[0], ocl_descriptor.context,
      input_flags | CL_MEM_USE_HOST_PTR, sizeof(cl_int) * 2 * device_align_size,
      buffer1.dynamic_array));
  ASSERT_NO_FATAL_FAILURE(createBuffer(
      &ocl_descriptor.buffers[1], ocl_descriptor.context,
      output_flags | CL_MEM_USE_HOST_PTR,
      sizeof(cl_int) * 2 * device_align_size, buffer2.dynamic_array));

  // split buffer into 4 sub buffers.
  // buffers in place 0 and 1 are input. 2 and 3 are output.
  ASSERT_NO_FATAL_FAILURE(
      createSubBuffer(&sub_buffers[0], ocl_descriptor.buffers[0], input_flags,
                      CL_BUFFER_CREATE_TYPE_REGION, &region));
  ASSERT_NO_FATAL_FAILURE(
      createSubBuffer(&sub_buffers[2], ocl_descriptor.buffers[1], output_flags,
                      CL_BUFFER_CREATE_TYPE_REGION, &region));
  // change region
  region.origin = sizeof(cl_int) * device_align_size;
  ASSERT_NO_FATAL_FAILURE(
      createSubBuffer(&sub_buffers[1], ocl_descriptor.buffers[0], input_flags,
                      CL_BUFFER_CREATE_TYPE_REGION, &region));
  ASSERT_NO_FATAL_FAILURE(
      createSubBuffer(&sub_buffers[3], ocl_descriptor.buffers[1], output_flags,
                      CL_BUFFER_CREATE_TYPE_REGION, &region));

  // create user event
  ASSERT_NO_FATAL_FAILURE(
      createUserEvent(&ocl_descriptor.events[0], ocl_descriptor.context));
  // create kernel and set arguments.
  ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels,
                                       ocl_descriptor.program, "copy_buffer"));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(
      ocl_descriptor.kernels[0], 0, sizeof(cl_mem), (void *)&sub_buffers[0]));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(
      ocl_descriptor.kernels[0], 1, sizeof(cl_mem), (void *)&sub_buffers[3]));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 2,
                                       sizeof(cl_uint),
                                       (void *)&device_align_size));
  // enqueue kernel on first device
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[first_device], ocl_descriptor.kernels[0], work_dim,
      0, &global_work_size, &local_work_size, 1, ocl_descriptor.events, NULL));

  // set new arguments
  ASSERT_NO_FATAL_FAILURE(setKernelArg(
      ocl_descriptor.kernels[0], 0, sizeof(cl_mem), (void *)&sub_buffers[1]));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(
      ocl_descriptor.kernels[0], 1, sizeof(cl_mem), (void *)&sub_buffers[2]));

  // enqueue kernel on second device
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[second_device], ocl_descriptor.kernels[0], work_dim,
      0, &global_work_size, &local_work_size, 1, ocl_descriptor.events, NULL));

  // run kernels
  ASSERT_NO_FATAL_FAILURE(
      setUserEventStatus(ocl_descriptor.events[0], CL_COMPLETE));
  finish(ocl_descriptor.queues[first_device]);
  finish(ocl_descriptor.queues[second_device]);

  // read the complete buffer with CPU device
  ASSERT_NO_FATAL_FAILURE(
      enqueueReadBuffer(ocl_descriptor.queues[0], ocl_descriptor.buffers[1],
                        CL_TRUE, 0, sizeof(int) * 2 * device_align_size,
                        buffer2.dynamic_array, 0, NULL, NULL));
  finish(ocl_descriptor.queues[0]);

  // validate result
  for (cl_uint i = 0; i < device_align_size; i++) {
    ASSERT_EQ(2, buffer2.dynamic_array[i]) << "result is not 2 as expected";
    ASSERT_EQ(1, buffer2.dynamic_array[device_align_size + i])
        << "result is not 1 as expected";
  }
}

// like the preveuios methood but each device will run in his own time
void validateSubBufferOneByOne(OpenCLDescriptor ocl_descriptor,
                               cl_mem_flags input_flags,
                               cl_mem_flags output_flags, cl_int first_device,
                               cl_int second_device) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  cl_buffer_region region;
  DynamicArray<cl_int> buffer1(2);
  DynamicArray<cl_int> buffer2(2);
  cl_mem sub_buffers[4];
  buffer1.dynamic_array[0] = 1;
  buffer1.dynamic_array[1] = 2;
  memset(buffer2.dynamic_array, 0, sizeof(cl_int));

  // initialize cl_buffer_region
  region.origin = 0;
  region.size = sizeof(cl_int);

  // set up shared context, program and queues with kernel1
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "buffer_sync_kernel.cl"));

  // create the buffer objects
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.buffers[0], ocl_descriptor.context,
                   input_flags | CL_MEM_USE_HOST_PTR, sizeof(cl_int) * 2,
                   buffer1.dynamic_array));
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.buffers[1], ocl_descriptor.context,
                   output_flags | CL_MEM_USE_HOST_PTR, sizeof(cl_int) * 2,
                   buffer2.dynamic_array));

  // split buffer into 4 sub buffers.
  // buffers in place 0 and 1 are input. 2 and 3 are output.
  ASSERT_NO_FATAL_FAILURE(
      createSubBuffer(&sub_buffers[0], ocl_descriptor.buffers[0], input_flags,
                      CL_BUFFER_CREATE_TYPE_REGION, &region));
  ASSERT_NO_FATAL_FAILURE(
      createSubBuffer(&sub_buffers[2], ocl_descriptor.buffers[1], output_flags,
                      CL_BUFFER_CREATE_TYPE_REGION, &region));
  // change region
  region.origin = sizeof(cl_int);
  ASSERT_NO_FATAL_FAILURE(
      createSubBuffer(&sub_buffers[1], ocl_descriptor.buffers[0], input_flags,
                      CL_BUFFER_CREATE_TYPE_REGION, &region));
  ASSERT_NO_FATAL_FAILURE(
      createSubBuffer(&sub_buffers[3], ocl_descriptor.buffers[1], output_flags,
                      CL_BUFFER_CREATE_TYPE_REGION, &region));

  // create kernel and set arguments.
  /*
  ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels,
  ocl_descriptor.program, "kernel_1"));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 0,
  sizeof(cl_mem), (void*)&ocl_descriptor.buffers[0]));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1,
  sizeof(cl_int), &num));
  */
  ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels,
                                       ocl_descriptor.program, "copy_buffer"));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(
      ocl_descriptor.kernels[0], 0, sizeof(cl_mem), (void *)&sub_buffers[0]));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(
      ocl_descriptor.kernels[0], 1, sizeof(cl_mem), (void *)&sub_buffers[3]));

  // enqueue kernel on first device and run on first device
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[first_device], ocl_descriptor.kernels[0], work_dim,
      0, &global_work_size, &local_work_size, 0, NULL, NULL));
  finish(ocl_descriptor.queues[first_device]);

  // set new arguments
  ASSERT_NO_FATAL_FAILURE(setKernelArg(
      ocl_descriptor.kernels[0], 0, sizeof(cl_mem), (void *)&sub_buffers[1]));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(
      ocl_descriptor.kernels[0], 1, sizeof(cl_mem), (void *)&sub_buffers[2]));

  // enqueue kernel on second device and run on second device
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[second_device], ocl_descriptor.kernels[0], work_dim,
      0, &global_work_size, &local_work_size, 0, NULL, NULL));
  finish(ocl_descriptor.queues[second_device]);

  // read the complete buffer with CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[1], CL_TRUE, 0,
      sizeof(int) * 2, buffer2.dynamic_array, 0, NULL, NULL));
  finish(ocl_descriptor.queues[0]);

  // validate result
  ASSERT_EQ(2, buffer2.dynamic_array[0]) << "result is not 2 as expected";
  ASSERT_EQ(1, buffer2.dynamic_array[1]) << "result is not 1 as expected";
}

TEST_F(CRT12_VR_124_127, SubBufferSyncReadWriteCPU) {
  ASSERT_NO_FATAL_FAILURE(validateSubBuffer(ocl_descriptor, CL_MEM_READ_WRITE,
                                            CL_MEM_READ_WRITE, CPU_DEVICE,
                                            CPU_DEVICE));
}

TEST_F(CRT12_VR_124_127, SubBufferSyncReadOnlyWriteOnlyCPU) {
  ASSERT_NO_FATAL_FAILURE(validateSubBuffer(ocl_descriptor, CL_MEM_READ_ONLY,
                                            CL_MEM_WRITE_ONLY, CPU_DEVICE,
                                            CPU_DEVICE));
}

TEST_F(CRT12_VR_124_127, SubBufferSyncReadWriteGPU) {
  ASSERT_NO_FATAL_FAILURE(validateSubBuffer(ocl_descriptor, CL_MEM_READ_WRITE,
                                            CL_MEM_READ_WRITE, GPU_DEVICE,
                                            GPU_DEVICE));
}

TEST_F(CRT12_VR_124_127, SubBufferSyncReadOnlyWriteOnlyGPU) {
  ASSERT_NO_FATAL_FAILURE(validateSubBuffer(ocl_descriptor, CL_MEM_READ_ONLY,
                                            CL_MEM_WRITE_ONLY, GPU_DEVICE,
                                            GPU_DEVICE));
}

TEST_F(CRT12_VR_124_127, SubBufferSyncReadWriteGPUCPU) {
  ASSERT_NO_FATAL_FAILURE(validateSubBuffer(ocl_descriptor, CL_MEM_READ_WRITE,
                                            CL_MEM_READ_WRITE, GPU_DEVICE,
                                            CPU_DEVICE));
}

TEST_F(CRT12_VR_124_127, SubBufferSyncReadOnlyWriteOnlyGPUCPU) {
  ASSERT_NO_FATAL_FAILURE(validateSubBuffer(ocl_descriptor, CL_MEM_READ_ONLY,
                                            CL_MEM_WRITE_ONLY, GPU_DEVICE,
                                            CPU_DEVICE));
}

TEST_F(CRT12_VR_124_127, SubBufferSyncReadWriteCPUGPU) {
  ASSERT_NO_FATAL_FAILURE(validateSubBuffer(ocl_descriptor, CL_MEM_READ_WRITE,
                                            CL_MEM_READ_WRITE, CPU_DEVICE,
                                            GPU_DEVICE));
}

TEST_F(CRT12_VR_124_127, SubBufferOneByOneReadWriteCPUGPU) {
  ASSERT_NO_FATAL_FAILURE(validateSubBuffer(ocl_descriptor, CL_MEM_READ_WRITE,
                                            CL_MEM_READ_WRITE, CPU_DEVICE,
                                            CPU_DEVICE));
}

TEST_F(CRT12_VR_124_127, SubBufferOneByOneReadOnlyWriteOnlyCPUGPU) {
  ASSERT_NO_FATAL_FAILURE(validateSubBuffer(ocl_descriptor, CL_MEM_READ_ONLY,
                                            CL_MEM_WRITE_ONLY, GPU_DEVICE,
                                            CPU_DEVICE));
}

TEST_F(CRT12_VR_124_127, SubBufferSyncReadOnlyWriteOnlyCPUGPU) {
  ASSERT_NO_FATAL_FAILURE(validateSubBuffer(ocl_descriptor, CL_MEM_READ_ONLY,
                                            CL_MEM_WRITE_ONLY, CPU_DEVICE,
                                            GPU_DEVICE));
}
