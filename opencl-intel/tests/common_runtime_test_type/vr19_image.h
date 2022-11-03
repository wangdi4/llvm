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

#ifndef VR19_IMAGE_GTEST_
#define VR19_IMAGE_GTEST_

//|  TEST: ReadImage2DShared (TC-92)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability of different devices (CPU and GPU) to work in
// parallel on the same | shared memory using Read Image commands for 2D images
//|
//|  Method
//|  ------
//|
//|  1. Initialize array of size arraySize in host with CPU pattern
//|  2. Initialize array of size arraySize in host with GPU pattern
//|  3. Initialize array of size 2*arraySize in host with pattern different
// from CPU and GPU patterns |  4. Create 2 OpenCL 2D imaged with
// CL_MEM_ALLOC_HOST_PTR of size arraySize*sizeof(type of arrays in steps 1, 2
// and 3) |  5. Write array from step 1 to one of the images |  6. Write
// array from step 2 to the other image |  7. Create user event |  8.
// Enqueue command read from image in step 5 to lower half of array in step 3 in
// queue of CPU device |    Command is dependant on status of event
// in step 7 |  9. Enqueue command read from image in step 6 to ther other half
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
static void testReadImage2DSharedBody(OpenCLDescriptor &ocl_descriptor) {
  // initial_pattern - initial array element content
  cl_int4 initial_pattern = {1, 1, 1, 1};
  // replacement[CPU pattern, GPU pattern]
  cl_int4 replacement[] = {{3, 3, 3, 3}, {4, 4, 4, 4}};

  // create and initialize array
  size_t width = 4;
  size_t height = 4;
  size_t depth = 1;
  int arraySize = (int)(width * height * depth);

  size_t origin[] = {0, 0, 0};
  size_t region[] = {width, height, depth};
  size_t row_pitch = 0;
  size_t slice_pitch = 0;

  DynamicArray<cl_int4> cpu_input_array(arraySize, replacement[0]);
  DynamicArray<cl_int4> gpu_input_array(arraySize, replacement[1]);
  DynamicArray<cl_int4> output_array(arraySize * 2, initial_pattern);

  cl_image_format image_format;
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

  // check if image format is supported
  bool isSupported = false;
  ASSERT_NO_FATAL_FAILURE(isImageFormatSupportedByRequiredDevices(
      CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, CL_MEM_OBJECT_IMAGE2D,
      image_format, 2, &isSupported));

  if (false == isSupported) {
    // this image format is not supported for the required devices
    // return
    return;
  }

  // this image format is supported for the required devices
  // resume test

  // set up shared context, program and queues
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "shared_kernel.cl"));

  // create buffers
  for (int i = 0; i < 2; ++i) {
    // create 2d image
    ASSERT_NO_FATAL_FAILURE(
        createImage2D(&ocl_descriptor.buffers[i], ocl_descriptor.context,
                      CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, &image_format,
                      width, height, row_pitch, NULL));
  }

  // write to input buffers device patterns
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(
        enqueueWriteImage(ocl_descriptor.queues[i], ocl_descriptor.buffers[i],
                          CL_TRUE, origin, region, row_pitch, slice_pitch,
                          (0 == i) ? (cpu_input_array.dynamic_array)
                                   : (gpu_input_array.dynamic_array),
                          0, NULL, NULL));
  }

  // create user event
  cl_event user_event = 0;
  cl_event device_done_event[] = {0, 0};
  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(enqueueReadImage(
        ocl_descriptor.queues[i], ocl_descriptor.buffers[i], CL_FALSE, origin,
        region, row_pitch, slice_pitch,
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
  DynamicArray<cl_int4> comparison_array(arraySize * 2);
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

//|  TEST: ReadImage3DShared (TC-93)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability of different devices (CPU and GPU) to work in
// parallel on the same | shared memory using Read Image commands for 3D images
//|
//|  Method
//|  ------
//|
//|  1. Initialize array of size arraySize in host with CPU pattern
//|  2. Initialize array of size arraySize in host with GPU pattern
//|  3. Initialize array of size 2*arraySize in host with pattern different
// from CPU and GPU patterns |  4. Create 2 OpenCL 3D imaged with
// CL_MEM_ALLOC_HOST_PTR of size arraySize*sizeof(type of arrays in steps 1, 2
// and 3) |  5. Write array from step 1 to one of the images |  6. Write
// array from step 2 to the other image |  7. Create user event |  8.
// Enqueue command read from image in step 5 to lower half of array in step 3 in
// queue of CPU device |    Command is dependant on status of event
// in step 7 |  9. Enqueue command read from image in step 6 to ther other half
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
static void testReadImage3DSharedBody(OpenCLDescriptor &ocl_descriptor) {
  // initial_pattern - initial array element content
  cl_int4 initial_pattern = {1, 1, 1, 1};
  // replacement[CPU pattern, GPU pattern]
  cl_int4 replacement[] = {{3, 3, 3, 3}, {4, 4, 4, 4}};

  // create and initialize array
  size_t width = 4;
  size_t height = 4;
  size_t depth = 4;
  int arraySize = (int)(width * height * depth);

  size_t origin[] = {0, 0, 0};
  size_t region[] = {width, height, depth};
  size_t row_pitch = 0;
  size_t slice_pitch = 0;

  DynamicArray<cl_int4> cpu_input_array(arraySize, replacement[0]);
  DynamicArray<cl_int4> gpu_input_array(arraySize, replacement[1]);
  DynamicArray<cl_int4> output_array(arraySize * 2, initial_pattern);

  cl_image_format image_format;
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

  // check if image format is supported
  bool isSupported = false;
  ASSERT_NO_FATAL_FAILURE(isImageFormatSupportedByRequiredDevices(
      CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, CL_MEM_OBJECT_IMAGE3D,
      image_format, 2, &isSupported));

  if (false == isSupported) {
    // this image format is not supported for the required devices
    // return
    return;
  }

  // this image format is supported for the required devices
  // resume test

  // set up shared context, program and queues
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "shared_kernel.cl"));

  // create buffers
  for (int i = 0; i < 2; ++i) {
    // create 2d image
    ASSERT_NO_FATAL_FAILURE(
        createImage3D(&ocl_descriptor.buffers[i], ocl_descriptor.context,
                      CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, &image_format,
                      width, height, depth, row_pitch, slice_pitch, NULL));
  }

  // write to input buffers device patterns
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(
        enqueueWriteImage(ocl_descriptor.queues[i], ocl_descriptor.buffers[i],
                          CL_TRUE, origin, region, row_pitch, slice_pitch,
                          (0 == i) ? (cpu_input_array.dynamic_array)
                                   : (gpu_input_array.dynamic_array),
                          0, NULL, NULL));
  }

  // create user event
  cl_event user_event = 0;
  cl_event device_done_event[] = {0, 0};
  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(enqueueReadImage(
        ocl_descriptor.queues[i], ocl_descriptor.buffers[i], CL_FALSE, origin,
        region, row_pitch, slice_pitch,
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
  DynamicArray<cl_int4> comparison_array(arraySize * 2);
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

#endif /* VR19_IMAGE_GTEST_ */
