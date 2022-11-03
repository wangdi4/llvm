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

#ifndef VR19_BUFFER_GTEST_
#define VR19_BUFFER_GTEST_

//|  TEST: ReadBufferShared (TC-89)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability of different devices (CPU and GPU) to work in
// parallel on the same | shared memory using Read Buffer commands
//|
//|  Method
//|  ------
//|
//|  1. Initialize array of size arraySize in host with CPU pattern
//|  2. Initialize array of size arraySize in host with GPU pattern
//|  3. Initialize array of size 2*arraySize in host with pattern different
// from CPU and GPU patterns |  4. Create 2 OpenCL buffers with
// CL_MEM_ALLOC_HOST_PTR of size arraySize*sizeof(type of arrays in steps 1, 2
// and 3) |  5. Write array from step 1 to one of the buffers |  6. Write
// array from step 2 to the other buffer |  7. Create user event |  8.
// Enqueue command read from buffer in step 5 to lower half of array in step 3
// in
// queue of CPU device |    Command is dependant on status of event
// in step 7 |  9. Enqueue command read from buffer in step 6 to ther other half
// of array in step 3 in queue of GPU device |    Command is dependant on
// status of event in step 7 |  10. Wait |  11. Verify that commands in
// steps 8 and 9 are enqueued |  12. Set status of event in step 7 to
// CL_COMPLETE
//|  13. Wait for commands to finish
//|
//|  Pass criteria
//|  -------------
//|
//|  Verify that first half of array in step 3 is filled with CPU pattern,
// and the second with GPU pattern
//|
static void testVR19ReadBufferSharedBody(OpenCLDescriptor &ocl_descriptor) {
  // initial_pattern - initial array element content
  int initial_pattern = 1;
  // replacement[CPU pattern, GPU pattern]
  int replacement[] = {3, 4};

  // initialize array
  int arraySize = (size_t)128;
  DynamicArray<cl_int> cpu_input_array(arraySize, replacement[0]);
  DynamicArray<cl_int> gpu_input_array(arraySize, replacement[1]);
  DynamicArray<cl_int> output_array(arraySize * 2, -initial_pattern);

  // set up shared context, program and queues
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "shared_kernel.cl"));

  // create buffers
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(
        createBuffer(&ocl_descriptor.buffers[i], ocl_descriptor.context,
                     CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                     sizeof(int) * cpu_input_array.dynamic_array_size, NULL));
  }

  // write to input buffers device patterns
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(enqueueWriteBuffer(
        ocl_descriptor.queues[i], ocl_descriptor.buffers[i], CL_TRUE, 0,
        sizeof(int) * cpu_input_array.dynamic_array_size,
        (0 == i) ? (cpu_input_array.dynamic_array)
                 : (gpu_input_array.dynamic_array),
        0, NULL, NULL));
  }

  // create user event
  cl_event user_event = 0;
  cl_event device_done_event[] = {0, 0};
  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
        ocl_descriptor.queues[i], ocl_descriptor.buffers[i], CL_FALSE, 0,
        sizeof(int) * cpu_input_array.dynamic_array_size,
        &output_array.dynamic_array[i * cpu_input_array.dynamic_array_size], 1,
        &user_event, &device_done_event[i]));
  }

  // wait
  sleepMS(100);

  // check that both kernels are sill CL_QUEUED or CL_SUBMITTED
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(validateQueuedOrSubmitted(device_done_event[i]));
  }
  // set user event as CL_COMPLETE
  ASSERT_NO_FATAL_FAILURE(setUserEventStatus(user_event, CL_COMPLETE));

  // wait for completion of all read buffer executions
  ASSERT_NO_FATAL_FAILURE(waitForEvents(2, device_done_event));

  releaseEvent(user_event);
  for (int i = 0; i < 2; i++) {
    releaseEvent(device_done_event[i]);
  }

  // validate execution correctness
  // create and initialize host array for results comparison
  DynamicArray<int> comparison_array(arraySize * 2);
  for (int j = 0; j < 2; ++j) {
    for (int i = 0; i < arraySize; ++i) {
      comparison_array.dynamic_array[j * arraySize + i] = replacement[j];
    }
  }
  ASSERT_NO_FATAL_FAILURE(output_array.compareArray(comparison_array));
}

//|  TEST: ReadBufferRectShared (TC-90)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability of different devices (CPU and GPU) to work in
// parallel on the same | shared memory using Read Buffer Rect commands
//|
//|  Method
//|  ------
//|
//|  1. Initialize array of size arraySize in host with CPU pattern
//|  2. Initialize array of size arraySize in host with GPU pattern
//|  3. Initialize array of size 2*arraySize in host with pattern different
// from CPU and GPU patterns |  4. Create 2 OpenCL buffers with
// CL_MEM_ALLOC_HOST_PTR of size arraySize*sizeof(type of arrays in steps 1, 2
// and 3) |  5. Write array from step 1 to one of the buffers with Read
// Buffer Rect |  6. Write array from step 2 to the other buffer with Read
// Buffer Rect |  7. Create user event |  8. Enqueue command read from
// buffer in step 5 to lower half of array in step 3 in queue of CPU device |
// Command is dependant on status of event in step 7 with Write Buffer Rect
//|  9. Enqueue command read from buffer in step 6 to ther other half of
// array in step 3 in queue of GPU device |    Command is dependant on
// status of event in step 7 with Write Buffer Rect |  10. Wait |  11.
// Verify that commands in steps 8 and 9 are enqueued |  12. Set status of event
// in step 7 to CL_COMPLETE |  13. Wait for commands to finish
//|
//|  Pass criteria
//|  -------------
//|
//|  Verify that first half of array in step 3 is filled with CPU pattern,
// and the second with GPU pattern
//|
static void testVR19ReadBufferRectSharedBody(OpenCLDescriptor &ocl_descriptor) {
  // initial_pattern - initial array element content
  int initial_pattern = 1;
  // replacement[CPU pattern, GPU pattern]
  int replacement[] = {3, 4};

  // create and initialize array
  size_t width = 16;
  size_t height = 16;
  size_t depth = 16;
  size_t row_pitch = 0;
  size_t slice_pitch = 0;
  int arraySize = (int)(width * height * depth);

  size_t origin[] = {0, 0, 0};
  size_t rect_region[] = {width * sizeof(cl_int), height, depth};
  DynamicArray<cl_int> cpu_input_array(arraySize, replacement[0]);
  DynamicArray<cl_int> gpu_input_array(arraySize, replacement[1]);
  DynamicArray<cl_int> output_array(arraySize * 2, -initial_pattern);

  // set up shared context, program and queues
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "shared_kernel.cl"));

  // create buffers
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(
        createBuffer(&ocl_descriptor.buffers[i], ocl_descriptor.context,
                     CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                     sizeof(int) * cpu_input_array.dynamic_array_size, NULL));
  }

  // write to input buffers device patterns
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(enqueueWriteBufferRect(
        ocl_descriptor.queues[i], ocl_descriptor.buffers[i], CL_TRUE, origin,
        origin, rect_region, row_pitch, slice_pitch, row_pitch, slice_pitch,
        (0 == i) ? (cpu_input_array.dynamic_array)
                 : (gpu_input_array.dynamic_array),
        0, NULL, NULL));
  }

  // create user event
  cl_event user_event = 0;
  cl_event device_done_event[] = {0, 0};
  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(enqueueReadBufferRect(
        ocl_descriptor.queues[i], ocl_descriptor.buffers[i], CL_FALSE, origin,
        origin, rect_region, row_pitch, slice_pitch, row_pitch, slice_pitch,
        &output_array.dynamic_array[i * cpu_input_array.dynamic_array_size], 1,
        &user_event, &device_done_event[i]));
  }

  // wait
  sleepMS(100);

  // check that both kernels are sill CL_QUEUED or CL_SUBMITTED
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(validateQueuedOrSubmitted(device_done_event[i]));
  }
  // set user event as CL_COMPLETE
  ASSERT_NO_FATAL_FAILURE(setUserEventStatus(user_event, CL_COMPLETE));

  // wait for completion of read buffer execution
  ASSERT_NO_FATAL_FAILURE(waitForEvents(2, device_done_event));

  // validate execution correctness
  // create and initialize host array for results comparison
  DynamicArray<int> comparison_array(arraySize * 2);
  for (int j = 0; j < 2; ++j) {
    for (int i = 0; i < arraySize; ++i) {
      comparison_array.dynamic_array[j * arraySize + i] = replacement[j];
    }
  }
  ASSERT_NO_FATAL_FAILURE(output_array.compareArray(comparison_array));

  releaseEvent(user_event);
  for (int i = 0; i < 2; i++) {
    releaseEvent(device_done_event[i]);
  }
}

//|  TEST: CopyBufferShared (TC-91)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability of different devices (CPU and GPU) to work in
// parallel on the same | shared memory using Copy Buffer commands
//|
//|  Method
//|  ------
//|
//|  1. Initialize array of size arraySize in host with CPU pattern
//|  2. Initialize array of size arraySize in host with GPU pattern
//|  3. Initialize array of size 2*arraySize in host with pattern different
// from CPU and GPU patterns |    and OpenCL buffer of this size with
// CL_MEM_USE_HOST_PTR |  4. Create 2 OpenCL buffers with
// CL_MEM_ALLOC_HOST_PTR of size arraySize*sizeof(type of arrays in steps 1, 2
// and 3) |  5. Write array from step 1 to one of the buffers with Read
// Buffer Rect |  6. Write array from step 2 to the other buffer with Read
// Buffer Rect |  7. Create user event |  8. Enqueue command copy from
// buffer in step 5 to lower half of buffer in step 3 in queue of CPU device |
// Command is dependant on status of event in step 7 with Write Buffer Rect
// |  9. Enqueue command copy from buffer in step 6 to ther other half of
// buffer in step 3 in queue of GPU device |    Command is dependant on
// status of event in step 7 with Write Buffer Rect |  10. enqueue
// enqueueMapBuffer dependant on events signaling completion of commands in
// steps 8 and 9 and wait |  11. Verify that commands in steps 8, 9 and 10
// are enqueued |  12. Set status of event in step 7 to CL_COMPLETE
// |  13. Wait for commands to finish
//|
//|  Pass criteria
//|  -------------
//|
//|  Verify that first half of array in step 3 is filled with CPU pattern,
// and the second with GPU pattern
//|
static void testVR19CopyBufferSharedBody(OpenCLDescriptor &ocl_descriptor) {
  // initial_pattern - initial array element content
  int initial_pattern = 1;
  // replacement[CPU pattern, GPU pattern]
  int replacement[] = {3, 4};

  // initialize array
  int arraySize = (size_t)128;
  DynamicArray<cl_int> cpu_input_array(arraySize, replacement[0]);
  DynamicArray<cl_int> gpu_input_array(arraySize, replacement[1]);
  DynamicArray<cl_int> output_array(arraySize * 2, -initial_pattern);

  // set up shared context, program and queues
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "shared_kernel.cl"));

  // create buffers
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(
        createBuffer(&ocl_descriptor.buffers[i], ocl_descriptor.context,
                     CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                     sizeof(int) * cpu_input_array.dynamic_array_size,
                     (0 == i) ? (cpu_input_array.dynamic_array)
                              : (gpu_input_array.dynamic_array)));

    // set up manual destruction
    ASSERT_NO_FATAL_FAILURE(setDeleteArrayOnCallback(
        ocl_descriptor.buffers[i],
        (0 == i) ? (cpu_input_array) : (gpu_input_array)));
  }
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.in_common_buffer, ocl_descriptor.context,
                   CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                   sizeof(int) * output_array.dynamic_array_size,
                   output_array.dynamic_array));

  // set up manual destruction
  ASSERT_NO_FATAL_FAILURE(
      setDeleteArrayOnCallback(ocl_descriptor.in_common_buffer, output_array));

  // create user event
  cl_event user_event = 0;
  cl_event device_done_event[] = {0, 0, 0, 0};
  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(
        enqueueCopyBuffer(ocl_descriptor.queues[i], ocl_descriptor.buffers[i],
                          ocl_descriptor.in_common_buffer, 0,
                          sizeof(int) * i * cpu_input_array.dynamic_array_size,
                          sizeof(int) * cpu_input_array.dynamic_array_size, 1,
                          &user_event, &device_done_event[i]));
  }
  // map output buffer
  ASSERT_NO_FATAL_FAILURE(
      enqueueMapBuffer(&output_array.dynamic_array, ocl_descriptor.queues[0],
                       ocl_descriptor.in_common_buffer, CL_FALSE, CL_MAP_READ,
                       0, sizeof(int) * output_array.dynamic_array_size, 2,
                       device_done_event, &device_done_event[2]));

  // unmap buffer
  ASSERT_NO_FATAL_FAILURE(enqueueUnmapMemObject(
      ocl_descriptor.queues[0], ocl_descriptor.in_common_buffer,
      output_array.dynamic_array, 1, &device_done_event[2],
      &device_done_event[3]));

  // wait
  sleepMS(100);

  // check that both kernels are sill CL_QUEUED or CL_SUBMITTED
  for (int i = 0; i < 3; ++i) {
    ASSERT_NO_FATAL_FAILURE(validateQueuedOrSubmitted(device_done_event[i]));
  }

  // set user event as CL_COMPLETE
  ASSERT_NO_FATAL_FAILURE(setUserEventStatus(user_event, CL_COMPLETE));

  // wait for completion of read buffer execution
  ASSERT_NO_FATAL_FAILURE(waitForEvents(4, device_done_event));

  // validate execution correctness
  // create and initialize host array for results comparison
  DynamicArray<int> comparison_array(arraySize * 2);
  for (int j = 0; j < 2; ++j) {
    for (int i = 0; i < arraySize; ++i) {
      comparison_array.dynamic_array[j * arraySize + i] = replacement[j];
    }
  }
  ASSERT_NO_FATAL_FAILURE(output_array.compareArray(comparison_array));

  releaseEvent(user_event);
  for (int i = 0; i < 4; i++) {
    releaseEvent(device_done_event[i]);
  }
}

#endif /* VR19_BUFFER_GTEST_ */
