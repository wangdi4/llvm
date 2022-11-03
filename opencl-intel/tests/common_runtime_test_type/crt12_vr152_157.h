// Copyright (c) 2006-2012 Intel Corporation
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
//
// vc8_image.h

#define CPU_DEVICE 0
#define GPU_DEVICE 1

//|  TEST: CRT12_VR152_173.ImageVisibility1D_VR152
//|
//|  Purpose
//|  -------
//|
//|  Verify that A one-dimensional texture object, in all supported formats,
// should be visible to all device types (CPU and GPU) in context.
//|
//|  Method
//|  ------
//|
//| 1.create context and such
//| 2. create two 1d image object
//| 3.copy the first image to second one on device (with kernel)
//| 4.copy the image back to host
//| 5.check result
//|
//| Kernel
//|  ------
//| transforming 1 to 0 and 0 to 1(reads and write from image)
//|
//|
//|
//|  Pass criteria
//|  -------------
//|
//|  second image should change according to first image.
//|

template <typename T>
void ImageVisibility(OpenCLDescriptor ocl_descriptor,
                     cl_image_format image_format, cl_image_desc desc,
                     const char *kernel_name, cl_int device_type,
                     cl_int chanel_order) {

  // dont cheack unsupporetd formats
  bool cpu_supported = false, gpu_supported;
  isImageFormatSupported(CL_MEM_READ_WRITE, desc.image_type, image_format,
                         &cpu_supported, &gpu_supported);
  if ((!cpu_supported && CPU_DEVICE == device_type) ||
      (!gpu_supported && GPU_DEVICE == device_type)) {
    return;
  }
  ocl_descriptor.freeAllResources();
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  DynamicArray<T> input_image_array(1);
  DynamicArray<T> output_array(1);
  cl_mem image = nullptr;
  size_t origin[] = {0, 0, 0};
  size_t region[] = {1, 1, 1};

  // initialize arrays
  for (int i = 0; i < chanel_order; i++) {
    input_image_array.dynamic_array[0].s[i] = 1;
  }
  output_array.multBy(0);
  // initialize environment
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "image_visibility_kernel.cl"));

  // create image
  if (CL_MEM_OBJECT_IMAGE1D_BUFFER != desc.image_type) {
    ASSERT_NO_FATAL_FAILURE(
        createImage(&image, ocl_descriptor.context,
                    CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &image_format,
                    &desc, input_image_array.dynamic_array));
  } else {

    ASSERT_NO_FATAL_FAILURE(
        createBuffer(&ocl_descriptor.buffers[0], ocl_descriptor.context,
                     CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(T),
                     input_image_array.dynamic_array));
    desc.mem_object = ocl_descriptor.buffers[0];
    ASSERT_NO_FATAL_FAILURE(createImage(&image, ocl_descriptor.context,
                                        CL_MEM_READ_WRITE, &image_format, &desc,
                                        input_image_array.dynamic_array));
    desc.image_type = CL_MEM_OBJECT_IMAGE1D;
    desc.mem_object = 0;
  }

  // create seconed image

  if (desc.image_depth != 0) { // in case of 3d images, will need to create 1d
                               // image that we can write to.
    desc.image_depth = 0;
    desc.image_height = 0;
    desc.image_type = CL_MEM_OBJECT_IMAGE1D;
  }
  ASSERT_NO_FATAL_FAILURE(
      createImage(&ocl_descriptor.out_common_buffer, ocl_descriptor.context,
                  CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &image_format,
                  &desc, output_array.dynamic_array));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels,
                                       ocl_descriptor.program, kernel_name));

  // set kernel arg
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &image));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1,
                                       sizeof(cl_mem),
                                       &ocl_descriptor.out_common_buffer));

  // enqueue kernel on device
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[device_type], ocl_descriptor.kernels[0], work_dim,
      0, &global_work_size, &local_work_size, 0, NULL, NULL));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[device_type]));
  // read from device
  ASSERT_NO_FATAL_FAILURE(enqueueReadImage(
      ocl_descriptor.queues[device_type], image, CL_TRUE, origin, region, 0, 0,
      output_array.dynamic_array, 0, NULL, NULL));
  // ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[device_type],ocl_descriptor.out_common_buffer,CL_TRUE,0,sizeof(T),output_array.dynamic_array,0,NULL,NULL));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[device_type]));
  for (int i = 0; i < chanel_order; i++) {
    ASSERT_EQ(input_image_array.dynamic_array[0].s[i],
              output_array.dynamic_array[0].s[i]);
  }
}

template <typename T>
void ImageVisibility1D_crush(OpenCLDescriptor ocl_descriptor,
                             cl_image_format image_format, cl_image_desc desc,
                             const char *kernel_name, cl_int device_type,
                             cl_int chanel_order) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  DynamicArray<T> input_image_array(1);
  DynamicArray<T> output_array(1);
  cl_mem image = nullptr;
  size_t origin[] = {0, 0, 0};
  size_t region[] = {1, 1, 1};

  // initialize arrays
  // TODO need to find a way to initialize this to one!!!!! maybe Ill just use
  // the first cell..?

  for (int i = 0; i < chanel_order; i++) {
    input_image_array.dynamic_array[0].s[i] = 1;
  }
  output_array.multBy(0);
  // initialize environment
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "image_visibility_kernel.cl"));

  // create image
  ASSERT_NO_FATAL_FAILURE(createImage(
      &image, ocl_descriptor.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
      &image_format, &desc, input_image_array.dynamic_array));

  // create buffer
  ASSERT_NO_FATAL_FAILURE(
      createImage(&ocl_descriptor.out_common_buffer, ocl_descriptor.context,
                  CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &image_format,
                  &desc, output_array.dynamic_array));

  // ASSERT_NO_FATAL_FAILURE(createBuffer(,ocl_descriptor.context,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,sizeof(T),output_array.dynamic_array));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels,
                                       ocl_descriptor.program, kernel_name));

  // set kernel arg
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &image));
  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0], 1,
                                       sizeof(cl_mem),
                                       &ocl_descriptor.out_common_buffer));

  // enqueue kernel on device
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[device_type], ocl_descriptor.kernels[0], work_dim,
      0, &global_work_size, &local_work_size, 0, NULL, NULL));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[device_type]));
  // read from device
  ASSERT_NO_FATAL_FAILURE(enqueueReadImage(
      ocl_descriptor.queues[device_type], image, CL_TRUE, origin, region, 0, 0,
      output_array.dynamic_array, 0, NULL, NULL));
  // ASSERT_NO_FATAL_FAILURE(enqueueReadBuffer(ocl_descriptor.queues[device_type],ocl_descriptor.out_common_buffer,CL_TRUE,0,sizeof(T),output_array.dynamic_array,0,NULL,NULL));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[device_type]));
  for (int i = 0; i < chanel_order; i++) {
    ASSERT_EQ(input_image_array.dynamic_array[0].s[i],
              output_array.dynamic_array[0].s[i]);
  }
}
