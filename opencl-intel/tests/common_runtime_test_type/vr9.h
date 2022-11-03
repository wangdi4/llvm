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

#ifndef VR9_GTEST_
#define VR9_GTEST_

#include "WorkerThread.h"
#include "common_runtime_tests.h"

// slaveFunction - thread function (gcc compatible signature)
// Sets event's ststus to completed
static void *slaveFunction(void *data) {
  cl_event *user_event = (cl_event *)data;
  // set user event as CL_COMPLETE
  setUserEventStatus(*user_event, CL_COMPLETE);
  releaseEvent(*user_event);
  return NULL;
}

// winSlaveFunction - thread function (windows compatible signature)
// Sets event's ststus to completed
static void winSlaveFunction(void *data) { slaveFunction(data); }

//  setAndWaitForEvent - sets user_event's status in another thread to
// CL_COMPLETE and   waits until all events device_done_event[numEvents]
// complete
static void setAndWaitForEvent(cl_event *user_event,
                               cl_event *device_done_event, int numEvents) {
  //  slaveThread
  WorkerThread slaveThread;
  // start thread
#if defined(_WIN32)
  ASSERT_TRUE(slaveThread.start(winSlaveFunction, user_event))
      << "Could not start thread";
#else
  ASSERT_TRUE(slaveThread.start(slaveFunction, user_event))
      << "Could not start thread";
#endif

  //  wait until slave thread completes
  slaveThread.stop();

  // wait for completion of device_done_event
  ASSERT_NO_FATAL_FAILURE(waitForEvents(numEvents, device_done_event))
      << "waitForEvents failed";
}

// waitOnSingleDevice - enqueues all commands from VR-9 to queue of device of
// type device_type, waits for their competion NOTE THE COMMENTED OUT CODE IN
// THIS METHOD - it did not work on AMD, although it should work
static void waitOnSingleDevice(OpenCLDescriptor &ocl_descriptor,
                               cl_device_type device_type) {
  // set up device, context, queues and program
  ASSERT_NO_FATAL_FAILURE(setUpSingleContextProgramQueue(
      "read_buffer.cl", &ocl_descriptor.context, &ocl_descriptor.program,
      ocl_descriptor.queues, device_type));

  // create and initialize array
  size_t width = 2;
  size_t height = 2;
  size_t depth = 2;
  int arraySize = (int)(width * height * depth);

  size_t origin[] = {0, 0, 0};
  size_t rect_region[] = {width * sizeof(cl_int4), height, depth};

  // create and initialize array
  size_t image_width = width;
  size_t image_height = height;
  size_t image_depth = depth;
  size_t region[] = {image_width, image_height, depth};
  size_t row_pitch = 0;
  size_t slice_pitch = 0;

  DynamicArray<cl_int4> input_array0(arraySize);
  DynamicArray<cl_int4> input_array1(arraySize);
  DynamicArray<cl_int4> input_array2(arraySize);
  DynamicArray<cl_int4> input_array3(arraySize);

  cl_image_format image_format;
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

  for (int i = 0; i < 2; ++i) {
    // create buffers
    ASSERT_NO_FATAL_FAILURE(createBuffer(
        &ocl_descriptor.buffers[i], ocl_descriptor.context,
        CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int4) * arraySize,
        (0 == i) ? (input_array0.dynamic_array)
                 : (input_array1.dynamic_array)));

    // set up manual destruction
    ASSERT_NO_FATAL_FAILURE(setDeleteArrayOnCallback(
        ocl_descriptor.buffers[i], (0 == i) ? (input_array0) : (input_array1)));
  }

  // create images
  cl_mem images[] = {0, 0};
  ASSERT_NO_FATAL_FAILURE(
      createImage3D(&images[0], ocl_descriptor.context,
                    CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, &image_format,
                    image_width, image_height, image_depth, row_pitch,
                    slice_pitch, input_array2.dynamic_array));

  // set up manual destruction
  ASSERT_NO_FATAL_FAILURE(setDeleteArrayOnCallback(images[0], input_array2));

  ASSERT_NO_FATAL_FAILURE(
      createImage3D(&images[1], ocl_descriptor.context,
                    CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, &image_format,
                    image_width, image_height, image_depth, row_pitch,
                    slice_pitch, input_array3.dynamic_array));

  // set up manual destruction
  ASSERT_NO_FATAL_FAILURE(setDeleteArrayOnCallback(images[1], input_array3));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(createKernel(
      &ocl_descriptor.kernels[0], ocl_descriptor.program, "read_int4event"));
  // set kernel arguments
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], i,
                                         sizeof(cl_mem),
                                         &ocl_descriptor.buffers[i]));
  }
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 2, sizeof(int), &arraySize));
  // enqueue kernel
  // set work dimensions
  cl_uint work_dim = 1;
  size_t global_work_size = 1;

  // create user event
  cl_event user_event = 0;
  int numEvents = 20;
  cl_event device_done_event[20];
  for (int i = 0; i < numEvents; ++i) {
    device_done_event[i] = 0;
  }

  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  int index = 0;

  // read from buffer 0 to array 0
  ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[0], CL_FALSE, 0,
      sizeof(cl_int4) * arraySize, input_array0.dynamic_array, 1, &user_event,
      &device_done_event[index++]));

  // write from array 1 from buffer 1
  ASSERT_NO_FATAL_FAILURE(enqueueWriteBuffer(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[1], CL_FALSE, 0,
      sizeof(cl_int4) * arraySize, input_array0.dynamic_array, 1, &user_event,
      &device_done_event[index++]));

  // read rect from buffer 0 to array 0
  ASSERT_NO_FATAL_FAILURE(enqueueReadBufferRect(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[0], CL_FALSE, origin,
      origin, rect_region, row_pitch, slice_pitch, row_pitch, slice_pitch,
      input_array0.dynamic_array, 1, &user_event, &device_done_event[index++]));

  // write from array 1 from buffer 1
  ASSERT_NO_FATAL_FAILURE(enqueueWriteBufferRect(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[1], CL_FALSE, origin,
      origin, rect_region, row_pitch, slice_pitch, row_pitch, slice_pitch,
      input_array0.dynamic_array, 1, &user_event, &device_done_event[index++]));

  // copy buffer 0 to buffer 1
  ASSERT_NO_FATAL_FAILURE(enqueueCopyBuffer(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[0],
      ocl_descriptor.buffers[1], 0, 0, sizeof(cl_int4) * arraySize, 1,
      &user_event, &device_done_event[index++]));

  // copy from 0 to 1
  ASSERT_NO_FATAL_FAILURE(enqueueCopyBufferRect(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[0],
      ocl_descriptor.buffers[1], origin, origin, rect_region, row_pitch,
      slice_pitch, row_pitch, slice_pitch, 1, &user_event,
      &device_done_event[index++]));

  // map buffer 0
  ASSERT_NO_FATAL_FAILURE(
      enqueueMapBuffer(&input_array0.dynamic_array, ocl_descriptor.queues[0],
                       ocl_descriptor.buffers[0], CL_FALSE, CL_MAP_READ, 0,
                       sizeof(cl_int4) * arraySize, 1, &user_event,
                       &device_done_event[index++]));

  ASSERT_NO_FATAL_FAILURE(enqueueUnmapMemObject(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[0],
      input_array0.dynamic_array, 1, &user_event, &device_done_event[index++]));

  // read image 0
  ASSERT_NO_FATAL_FAILURE(enqueueReadImage(
      ocl_descriptor.queues[0], images[0], CL_FALSE, origin, region, row_pitch,
      slice_pitch, input_array0.dynamic_array, 1, &user_event,
      &device_done_event[index++]));

  // write image 1
  ASSERT_NO_FATAL_FAILURE(enqueueWriteImage(
      ocl_descriptor.queues[0], images[1], CL_FALSE, origin, region, row_pitch,
      slice_pitch, input_array1.dynamic_array, 1, &user_event,
      &device_done_event[index++]));

  // copy from image 0 to image 1
  ASSERT_NO_FATAL_FAILURE(enqueueCopyImage(
      ocl_descriptor.queues[0], images[0], images[1], origin, origin, region, 1,
      &user_event, &device_done_event[index++]));

  // copy from image 0 to buffer 0
  ASSERT_NO_FATAL_FAILURE(enqueueCopyImageToBuffer(
      ocl_descriptor.queues[0], images[0], ocl_descriptor.buffers[0], origin,
      region, 0, 1, &user_event, &device_done_event[index++]));

  // copy from buffer 1 to image 1
  ASSERT_NO_FATAL_FAILURE(enqueueCopyBufferToImage(
      ocl_descriptor.queues[0], ocl_descriptor.buffers[1], images[1], 0, origin,
      region, 1, &user_event, &device_done_event[index++]));

  // map image 0 to array 3
  ASSERT_NO_FATAL_FAILURE(enqueueMapImage(
      &input_array2.dynamic_array, ocl_descriptor.queues[0], images[0],
      CL_FALSE, CL_MAP_READ, origin, region, &row_pitch, &slice_pitch, 1,
      &user_event, &device_done_event[index++]));

  // unmap image 0 from array 3
  ASSERT_NO_FATAL_FAILURE(enqueueUnmapMemObject(
      ocl_descriptor.queues[0], images[0], input_array2.dynamic_array, 1,
      &user_event, &device_done_event[index++]));

  // enqueue kernel
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], 1, NULL,
      &global_work_size, NULL, 1, &user_event, &device_done_event[index++]));

  // enqueue task
  ASSERT_NO_FATAL_FAILURE(enqueueTask(ocl_descriptor.queues[0],
                                      ocl_descriptor.kernels[0], 1, &user_event,
                                      &device_done_event[index++]));

  // enqueue marker
  ASSERT_NO_FATAL_FAILURE(
      enqueueMarker(ocl_descriptor.queues[0], &device_done_event[index++]));

  setAndWaitForEvent(&user_event, device_done_event, index);

  for (int i = 0; i < 2; ++i) {
    // release images[i]
    if (0 != images[i]) {
      EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(images[i]))
          << "clReleaseMemObject failed";
      images[i] = 0;
    }
  }

  // release user events
  //  releaseEvent(user_event);
  for (int i = 0; i <= index; i++) {
    if (device_done_event[i]) {
      releaseEvent(device_done_event[i]);
    }
  }
}

// waitOnBothDevices - enqueues all commands from VR-9 to both devices' queues,
// waits for their competion NOTE THE COMMENTED OUT CODE IN THIS METHOD - it did
// not work on AMD, although it should work
static void waitOnBothDevices(OpenCLDescriptor &ocl_descriptor) {
  // create OpenCL queue, program and context
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "read_buffer.cl"));

  // create and initialize array
  size_t width = 2;
  size_t height = 2;
  size_t depth = 2;
  int arraySize = (int)(width * height * depth);

  size_t origin[] = {0, 0, 0};
  size_t rect_region[] = {width * sizeof(cl_int4), height, depth};

  // create and initialize array
  size_t image_width = width;
  size_t image_height = height;
  size_t image_depth = depth;
  size_t region[] = {image_width, image_height, depth};
  size_t row_pitch = 0;
  size_t slice_pitch = 0;

  DynamicArray<cl_int4> input_array0(arraySize);
  DynamicArray<cl_int4> input_array1(arraySize);
  DynamicArray<cl_int4> input_array2(arraySize);
  DynamicArray<cl_int4> input_array3(arraySize);

  cl_image_format image_format;
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

  for (int i = 0; i < 2; ++i) {
    // create buffers
    ASSERT_NO_FATAL_FAILURE(createBuffer(
        &ocl_descriptor.buffers[i], ocl_descriptor.context,
        CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int4) * arraySize,
        (0 == i) ? (input_array0.dynamic_array)
                 : (input_array1.dynamic_array)));

    // set up manual destruction
    ASSERT_NO_FATAL_FAILURE(setDeleteArrayOnCallback(
        ocl_descriptor.buffers[i], (0 == i) ? (input_array0) : (input_array1)));
  }

  // create images
  cl_mem images[] = {0, 0};
  ASSERT_NO_FATAL_FAILURE(
      createImage3D(&images[0], ocl_descriptor.context,
                    CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, &image_format,
                    image_width, image_height, image_depth, row_pitch,
                    slice_pitch, input_array2.dynamic_array));

  // set up manual destruction
  ASSERT_NO_FATAL_FAILURE(setDeleteArrayOnCallback(images[0], input_array2));

  ASSERT_NO_FATAL_FAILURE(
      createImage3D(&images[1], ocl_descriptor.context,
                    CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, &image_format,
                    image_width, image_height, image_depth, row_pitch,
                    slice_pitch, input_array3.dynamic_array));

  // set up manual destruction
  ASSERT_NO_FATAL_FAILURE(setDeleteArrayOnCallback(images[1], input_array3));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(createKernel(
      &ocl_descriptor.kernels[0], ocl_descriptor.program, "read_int4event"));
  // set kernel arguments
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], i,
                                         sizeof(cl_mem),
                                         &ocl_descriptor.buffers[i]));
  }
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 2, sizeof(int), &arraySize));
  // enqueue kernel
  // set work dimensions
  cl_uint work_dim = 1;
  size_t global_work_size = 1;

  // create user event
  cl_event user_event = 0;
  int numEvents = 50;
  cl_event device_done_event[50];
  for (int i = 0; i < numEvents; ++i) {
    device_done_event[i] = 0;
  }

  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  int index = 0;

  for (int d = 0; d < 2; ++d) {
    // read from buffer 0 to array 0
    ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(
        ocl_descriptor.queues[d], ocl_descriptor.buffers[d], CL_FALSE, 0,
        sizeof(cl_int4) * arraySize, input_array0.dynamic_array, 1, &user_event,
        &device_done_event[index++]));

    // write from array 1 from buffer 1
    ASSERT_NO_FATAL_FAILURE(enqueueWriteBuffer(
        ocl_descriptor.queues[d], ocl_descriptor.buffers[1], CL_FALSE, 0,
        sizeof(cl_int4) * arraySize, input_array0.dynamic_array, 1, &user_event,
        &device_done_event[index++]));

    // read rect from buffer 0 to array 0
    ASSERT_NO_FATAL_FAILURE(enqueueReadBufferRect(
        ocl_descriptor.queues[d], ocl_descriptor.buffers[d], CL_FALSE, origin,
        origin, rect_region, row_pitch, slice_pitch, row_pitch, slice_pitch,
        input_array0.dynamic_array, 1, &user_event,
        &device_done_event[index++]));

    // write from array 1 from buffer 1
    ASSERT_NO_FATAL_FAILURE(enqueueWriteBufferRect(
        ocl_descriptor.queues[d], ocl_descriptor.buffers[1], CL_FALSE, origin,
        origin, rect_region, row_pitch, slice_pitch, row_pitch, slice_pitch,
        input_array0.dynamic_array, 1, &user_event,
        &device_done_event[index++]));

    // copy buffer 0 to buffer 1
    ASSERT_NO_FATAL_FAILURE(enqueueCopyBuffer(
        ocl_descriptor.queues[d], ocl_descriptor.buffers[0],
        ocl_descriptor.buffers[1], 0, 0, sizeof(cl_int4) * arraySize, 1,
        &user_event, &device_done_event[index++]));

    // copy from 0 to 1
    ASSERT_NO_FATAL_FAILURE(enqueueCopyBufferRect(
        ocl_descriptor.queues[d], ocl_descriptor.buffers[0],
        ocl_descriptor.buffers[1], origin, origin, rect_region, row_pitch,
        slice_pitch, row_pitch, slice_pitch, 1, &user_event,
        &device_done_event[index++]));

    // map buffer 0
    ASSERT_NO_FATAL_FAILURE(
        enqueueMapBuffer(&input_array0.dynamic_array, ocl_descriptor.queues[d],
                         ocl_descriptor.buffers[d], CL_FALSE, CL_MAP_READ, 0,
                         sizeof(cl_int4) * arraySize, 1, &user_event,
                         &device_done_event[index++]));

    ASSERT_NO_FATAL_FAILURE(enqueueUnmapMemObject(
        ocl_descriptor.queues[d], ocl_descriptor.buffers[d],
        input_array0.dynamic_array, 1, &user_event,
        &device_done_event[index++]));

    // read image 0
    ASSERT_NO_FATAL_FAILURE(enqueueReadImage(
        ocl_descriptor.queues[d], images[d], CL_FALSE, origin, region,
        row_pitch, slice_pitch, input_array0.dynamic_array, 1, &user_event,
        &device_done_event[index++]));

    // write image 1
    ASSERT_NO_FATAL_FAILURE(enqueueWriteImage(
        ocl_descriptor.queues[d], images[1], CL_FALSE, origin, region,
        row_pitch, slice_pitch, input_array1.dynamic_array, 1, &user_event,
        &device_done_event[index++]));

    // copy from image 0 to image 1
    ASSERT_NO_FATAL_FAILURE(enqueueCopyImage(
        ocl_descriptor.queues[d], images[0], images[1], origin, origin, region,
        1, &user_event, &device_done_event[index++]));

    // copy from image 0 to buffer 0
    ASSERT_NO_FATAL_FAILURE(enqueueCopyImageToBuffer(
        ocl_descriptor.queues[d], images[0], ocl_descriptor.buffers[0], origin,
        region, 0, 1, &user_event, &device_done_event[index++]));

    // copy from buffer 1 to image 1
    ASSERT_NO_FATAL_FAILURE(enqueueCopyBufferToImage(
        ocl_descriptor.queues[d], ocl_descriptor.buffers[1], images[1], 0,
        origin, region, 1, &user_event, &device_done_event[index++]));

    // map image 0 to array 3
    ASSERT_NO_FATAL_FAILURE(enqueueMapImage(
        &input_array2.dynamic_array, ocl_descriptor.queues[d], images[d],
        CL_FALSE, CL_MAP_READ, origin, region, &row_pitch, &slice_pitch, 1,
        &user_event, &device_done_event[index++]));

    // unmap image 0 from array 3
    ASSERT_NO_FATAL_FAILURE(enqueueUnmapMemObject(
        ocl_descriptor.queues[d], images[d], input_array2.dynamic_array, 1,
        &user_event, &device_done_event[index++]));

    // enqueue kernel
    ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
        ocl_descriptor.queues[d], ocl_descriptor.kernels[0], 1, NULL,
        &global_work_size, NULL, 1, &user_event, &device_done_event[index++]));

    // enqueue task
    ASSERT_NO_FATAL_FAILURE(
        enqueueTask(ocl_descriptor.queues[d], ocl_descriptor.kernels[0], 1,
                    &user_event, &device_done_event[index++]));

    // enqueue marker
    ASSERT_NO_FATAL_FAILURE(
        enqueueMarker(ocl_descriptor.queues[d], &device_done_event[index++]));
  }

  ASSERT_NO_FATAL_FAILURE(
      setAndWaitForEvent(&user_event, device_done_event, index));

  for (int i = 0; i < 2; ++i) {
    // release images[i]
    if (0 != images[i]) {
      EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(images[i]))
          << "clReleaseMemObject failed";
      images[i] = 0;
    }
  }

  //  releaseEvent(user_event);
  for (int i = 0; i < 50; i++) {
    clReleaseEvent(device_done_event[i]);
  }
}

// executeKernelOnSingleDevice - enqueues a kernel to device's queue 100 times,
// waits for their competion
static void executeKernelOnSingleDevice(OpenCLDescriptor &ocl_descriptor,
                                        cl_device_type device_type) {
  // set up device, context, queues and program
  // create OpenCL queue, program and context
  ASSERT_NO_FATAL_FAILURE(setUpSingleContextProgramQueue(
      "read_buffer.cl", &ocl_descriptor.context, &ocl_descriptor.program,
      ocl_descriptor.queues, device_type));

  int arraySize = (size_t)4;
  DynamicArray<cl_int4> input_array0(arraySize);

  for (int i = 0; i < 2; ++i) {
    // create buffers
    ASSERT_NO_FATAL_FAILURE(
        createBuffer(&ocl_descriptor.buffers[i], ocl_descriptor.context,
                     CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                     sizeof(cl_int4) * arraySize, input_array0.dynamic_array));
  }

  // create kernel
  ASSERT_NO_FATAL_FAILURE(createKernel(
      &ocl_descriptor.kernels[0], ocl_descriptor.program, "read_int4event"));
  // set kernel arguments
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], i,
                                         sizeof(cl_mem),
                                         &ocl_descriptor.buffers[i]));
  }
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 2, sizeof(int), &arraySize));
  // enqueue kernel
  // set work dimensions
  cl_uint work_dim = 1;
  size_t global_work_size = 1;

  // create user event
  cl_event user_event = 0;
  int numEvents = 100;
  cl_event device_done_event[100];
  for (int i = 0; i < numEvents; ++i) {
    device_done_event[i] = 0;
  }

  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  for (int i = 0; i < numEvents; ++i) {
    // enqueue kernel
    ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
        ocl_descriptor.queues[0], ocl_descriptor.kernels[0], 1, NULL,
        &global_work_size, NULL, 1, &user_event, &device_done_event[i]));
  }
  setAndWaitForEvent(&user_event, device_done_event, numEvents);

  // releaseEvent(user_event);
  for (int i = 0; i < 100; i++) {
    releaseEvent(device_done_event[i]);
  }
}

// executeKernelOnSingleDevice - enqueues a kernel to each device's queue 100
// times, waits for their competion
static void executeKernelOnBothDevices(OpenCLDescriptor &ocl_descriptor) {
  // create OpenCL queue, program and context
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "read_buffer.cl"));

  int arraySize = (size_t)4;
  DynamicArray<cl_int4> input_array0(arraySize);

  for (int i = 0; i < 2; ++i) {
    // create buffers
    ASSERT_NO_FATAL_FAILURE(
        createBuffer(&ocl_descriptor.buffers[i], ocl_descriptor.context,
                     CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                     sizeof(cl_int4) * arraySize, input_array0.dynamic_array));
  }

  // create kernel
  ASSERT_NO_FATAL_FAILURE(createKernel(
      &ocl_descriptor.kernels[0], ocl_descriptor.program, "read_int4event"));
  // set kernel arguments
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], i,
                                         sizeof(cl_mem),
                                         &ocl_descriptor.buffers[i]));
  }
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 2, sizeof(int), &arraySize));
  // enqueue kernel
  // set work dimensions
  cl_uint work_dim = 1;
  size_t global_work_size = 1;

  // create user event
  cl_event user_event = 0;
  int numEvents = 200;
  cl_event device_done_event[200];
  for (int i = 0; i < numEvents; ++i) {
    device_done_event[i] = 0;
  }

  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));
  int i = 0;
  for (int d = 0; d < 2; ++d) {
    for (; i < numEvents; ++i) {
      // enqueue kernel
      ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
          ocl_descriptor.queues[d], ocl_descriptor.kernels[0], 1, NULL,
          &global_work_size, NULL, 1, &user_event, &device_done_event[i]));
    }
  }
  setAndWaitForEvent(&user_event, device_done_event, numEvents);

  // releaseEvent(user_event);
  for (int i = 0; i < 200; i++) {
    releaseEvent(device_done_event[i]);
  }
}

#endif /* VR9_BUFFER_GTEST_ */
