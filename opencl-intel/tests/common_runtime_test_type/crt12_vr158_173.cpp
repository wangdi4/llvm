// WARRANTY DISCLAIMER// WARRANTY DISCLAIMER
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
#include "crt12_vr152_157.h"

#define CPU_DEVICE 0
#define GPU_DEVICE 1

class CRT12_VR158_173 : public CommonRuntime {};

//|  TEST: CRT12_VR158_173.CheckChangeImage
//|
//|  Purpose
//|  -------
//|  an auxiliary methood for vr162-vr173.
//|  Changes which are done by first device in a given image object are
// reflected in second device
//|
//|  Method
//|  ------
//|
//| 1.  Create context and such
//| 2.  create a an image object
//| 3.  run a kernel on first device that will change the image
//| 4.  run a kernel on second device that will verify that a change was made on
// the image | 5.  read image back to host | 6.  check result
//|
//|  Pass criteria
//|  -------------
//|
//|  image array should change according to both devices kernel.
//|

void CheckChangeImage(OpenCLDescriptor ocl_descriptor, cl_image_desc desc,
                      cl_int first_device_type, cl_int second_device_type,
                      const char *first_device_kernel,
                      const char *second_device_kernel) {

  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  DynamicArray<cl_int4> image_array(8);
  DynamicArray<cl_int4> output_array(4);
  cl_image_format image_format;
  size_t origin[] = {0, 0, 0};
  size_t region[] = {1, 1, 1};
  cl_mem image = nullptr;

  // initialize arrays
  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 4; i++) {
      output_array.dynamic_array[i].s[j] = 0;
      image_array.dynamic_array[i].s[j] = 0;
    }
  }

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

  // initialize environment
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "read_image_kernel.cl"));
  // create image
  // create buffer (if needed)
  if (0 == desc.mem_object) {
    ASSERT_NO_FATAL_FAILURE(
        createImage(&image, ocl_descriptor.context,
                    CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &image_format,
                    &desc, image_array.dynamic_array));

  } else {
    ASSERT_NO_FATAL_FAILURE(
        createBuffer(&ocl_descriptor.buffers[0], ocl_descriptor.context,
                     CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int4),
                     output_array.dynamic_array));
    desc.mem_object =
        ocl_descriptor
            .buffers[0]; // we need to initialize the buffer into the
                         // ocl_descriptor so that it will be released properly
    ASSERT_NO_FATAL_FAILURE(createImage(&image, ocl_descriptor.context,
                                        CL_MEM_READ_WRITE, &image_format, &desc,
                                        image_array.dynamic_array));
  }

  // create kernels
  ASSERT_NO_FATAL_FAILURE(createKernel(
      &ocl_descriptor.kernels[0], ocl_descriptor.program, first_device_kernel));
  ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[1],
                                       ocl_descriptor.program,
                                       second_device_kernel));

  // set kernel arg
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &image));
  //  ASSERT_NO_FATAL_FAILURE(setKernelArg(ocl_descriptor.kernels[0],1,sizeof(cl_mem),&image));

  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[1], 0, sizeof(cl_mem), &image));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[1], 1, sizeof(cl_mem), &image));

  // enqueue kernel on fisrt device
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[first_device_type], ocl_descriptor.kernels[0],
      work_dim, 0, &global_work_size, &local_work_size, 0, NULL,
      &ocl_descriptor.events[0]));

  // enqueue kernel on second device
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[second_device_type], ocl_descriptor.kernels[1],
      work_dim, 0, &global_work_size, &local_work_size, 1,
      &ocl_descriptor.events[0], NULL));

  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[first_device_type]));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[second_device_type]));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(
      enqueueReadImage(ocl_descriptor.queues[0], image, CL_TRUE, origin, region,
                       0, 0, output_array.dynamic_array, 0, NULL, NULL));

  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[1]));

  for (int j = 0; j < 4; j++) {
    ASSERT_EQ(2, output_array.dynamic_array[0].s[j])
        << "array was not changed as expected";
  }
}

//|  TEST: CRT12_VR158_173.ImageBufferVisibility1D_VR153
//|
//|  Purpose
//|  -------
//|
//|  Verify that A one-dimensional texture buffer object, in all supported
// formats, should be visible to all device types (CPU and GPU) in context.
//|
//|  Method
//|  ------
//|
//| 1.create context and such
//| 2. create a buffer memory object
//| 3. create a 1d image image buffer object from buffer
//| 4.copy the image to a buffer object on device
//| 5.run kernel
//| 6.copy the image back to host
//| 7.check result
//|
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
//|  image array should change according to kernel.
//|

TEST_F(CRT12_VR158_173, DISABLED_ImageBufferVisibility1D_VR153) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  DynamicArray<cl_int4> cpu_input_image_array(1);
  DynamicArray<cl_int> output_array(4);
  cl_image_format image_format;
  cl_image_desc desc;
  cl_mem image = nullptr;
  size_t origin[] = {0, 0, 0};
  size_t region[] = {0, 0, 0};
  // initialize arrays
  for (int i = 0; i < 4; i++) {
    output_array.dynamic_array[i] = 0;
  }
  for (int i = 0; i < 2; i++) {
    cpu_input_image_array.dynamic_array[0].s[i] = 1;
  }
  for (int i = 2; i < 4; i++) {
    cpu_input_image_array.dynamic_array[0].s[i] = 0;
  }

  // initialize image descriptor All unused values must be initialized to zeros!
  desc.image_type = CL_MEM_OBJECT_IMAGE1D_BUFFER;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 0;
  desc.image_depth = 0;
  desc.image_array_size = 0;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;

  // initialize environment
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "read_image_kernel.cl"));

  // create buffer
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.out_common_buffer, ocl_descriptor.context,
                   CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int4),
                   output_array.dynamic_array));

  // create image TODO what is the meanning of host pointer in here?
  desc.mem_object = ocl_descriptor.out_common_buffer;
  ASSERT_NO_FATAL_FAILURE(createImage(
      &image, ocl_descriptor.context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
      &image_format, &desc, cpu_input_image_array.dynamic_array));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels,
                                       ocl_descriptor.program,
                                       "read_write_image1D_int4"));

  // set kernel arg
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &image));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_mem), &image));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadImage(
      ocl_descriptor.queues[0], image, CL_TRUE, origin, region, 0, 0,
      cpu_input_image_array.dynamic_array, 0, NULL, NULL));

  for (int i = 0; i < 2; i++) {
    ASSERT_EQ(0, cpu_input_image_array.dynamic_array[0].s[i])
        << "image was not copyed properly";
  }
  for (int i = 2; i < 4; i++) {
    ASSERT_EQ(1, cpu_input_image_array.dynamic_array[0].s[i])
        << "image was not copyed properly";
  }
}

//|  TEST: CRT12_VR158_173.ImageArrayVisibility1D_VR154
//|
//|  Purpose
//|  -------
//|
//|  Verify that A one-dimensional array object, in all supported formats,
// should be visible to all device types (CPU and GPU) in context.
//|
//|  Method
//|  ------
//|
//| 1.create context and such
//| 2. create a 1d array of images object
//| 3.copy the array to a buffer object on device
//| 4.run kernel
//| 5.copy the array back to host
//| 6.check result
//| Kernel
//|  ------
//| transforming 1 to 0 and 0 to 1(reads and write from image)
//|
//|
//|
//|  Pass criteria
//|  -------------
//|
//|  image array should change according to kernel.
//|

TEST_F(CRT12_VR158_173, DISABLED_ImageArrayVisibility1D_VR154) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  DynamicArray<cl_int4> cpu_input_image_array(4);
  DynamicArray<cl_int> output_array(4);
  cl_image_format image_format;
  cl_image_desc desc;
  cl_mem image = nullptr;

  // initialize arrays
  for (int i = 0; i < 4; i++) {
    output_array.dynamic_array[i] = 0;
  }
  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 2; i++) {
      cpu_input_image_array.dynamic_array[j].s[i] = 1;
    }
    for (int i = 2; i < 4; i++) {
      cpu_input_image_array.dynamic_array[j].s[i] = 0;
    }
  }

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE1D_ARRAY;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 0;
  desc.image_depth = 0;
  desc.image_array_size = 4;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;
  size_t origin[] = {0, 0, 2};
  size_t region[] = {4, 1, 1};

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

  // initialize environment
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "read_image_kernel.cl"));

  // create image
  ASSERT_NO_FATAL_FAILURE(createImage(
      &image, ocl_descriptor.context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
      &image_format, &desc, cpu_input_image_array.dynamic_array));

  // create buffer
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.out_common_buffer, ocl_descriptor.context,
                   CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int4),
                   output_array.dynamic_array));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels,
                                       ocl_descriptor.program,
                                       "read_write_image_array_1D_int4"));

  // set kernel arg
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &image));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_mem), &image));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadImage(
      ocl_descriptor.queues[0], image, CL_TRUE, origin, region, 0, 0,
      cpu_input_image_array.dynamic_array, 0, NULL, NULL));

  for (int i = 0; i < 2; i++) {
    ASSERT_EQ(0, cpu_input_image_array.dynamic_array[0].s[i])
        << "image was not copyed properly";
  }
  for (int i = 2; i < 4; i++) {
    ASSERT_EQ(1, cpu_input_image_array.dynamic_array[0].s[i])
        << "image was not copyed properly";
  }
}

//|  TEST: CRT12_VR158_173.ImageVisibility2D_VR155
//|
//|  Purpose
//|  -------
//|
//|  Verify that A two-dimensional texture object, in all supported formats,
// should be visible to all device types (CPU and GPU) in context.
//|
//|  Method
//|  ------
//|
//| 1.create context and such
//| 2.create a 2d image object
//| 3.run kernel on device on device
//| 4.copy image back to host
//| 5.check result
//|
//| Kernel
//|  ------
//| transforming 1 to 0 and 0 to 1(reads and write from image)
//|
//|  Pass criteria
//|  -------------
//|
//|  image should change according to kernel.
//|

TEST_F(CRT12_VR158_173, DISABLED_ImageVisibility2D_VR155) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  DynamicArray<cl_int4> cpu_input_image_array(1);
  DynamicArray<cl_int> output_array(4);
  cl_image_format image_format;
  cl_image_desc desc;
  cl_mem image = nullptr;

  // initialize arrays
  for (int i = 0; i < 4; i++) {
    output_array.dynamic_array[i] = 0;
  }
  for (int i = 0; i < 2; i++) {
    cpu_input_image_array.dynamic_array[0].s[i] = 1;
  }
  for (int i = 2; i < 4; i++) {
    cpu_input_image_array.dynamic_array[0].s[i] = 0;
  }

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 1;
  desc.image_depth = 0;
  desc.image_array_size = 0;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;
  size_t origin[] = {0, 0, 0};
  size_t region[] = {1, 1, 1};

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

  // initialize environment
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "read_image_kernel.cl"));

  // create image
  ASSERT_NO_FATAL_FAILURE(createImage(
      &image, ocl_descriptor.context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
      &image_format, &desc, cpu_input_image_array.dynamic_array));

  // create buffer
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.out_common_buffer, ocl_descriptor.context,
                   CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int4),
                   output_array.dynamic_array));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels,
                                       ocl_descriptor.program,
                                       "read_write_image2D_int4"));

  // set kernel arg
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &image));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_mem), &image));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadImage(
      ocl_descriptor.queues[0], image, CL_TRUE, origin, region, 0, 0,
      cpu_input_image_array.dynamic_array, 0, NULL, NULL));

  for (int i = 0; i < 2; i++) {
    ASSERT_EQ(0, cpu_input_image_array.dynamic_array[0].s[i])
        << "image was not copyed properly";
  }
  for (int i = 2; i < 4; i++) {
    ASSERT_EQ(1, cpu_input_image_array.dynamic_array[0].s[i])
        << "image was not copyed properly";
  }
}

//|  TEST: CRT12_VR158_173.ImageArrayVisibility2D_VR156
//|
//|  Purpose
//|  -------
//|
//|  Verify that A two-dimensional Array of texture object, in all supported
// formats, should be visible to all device types (CPU and GPU) in context.
//|
//|  Method
//|  ------
//|
//| 1.create context and such
//| 2.create a 2d array of images object
//| 3.run kernel on device on device
//| 4.copy image back to host
//| 5.check result
//|
//| Kernel
//|  ------
//| transforming 1 to 0 and 0 to 1(reads and write from image)
//|
//|  Pass criteria
//|  -------------
//|
//|  image array should change according to kernel.
//|

TEST_F(CRT12_VR158_173, DISABLED_ImageArrayVisibility2D_VR156) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  DynamicArray<cl_int4> cpu_input_image_array(1);
  DynamicArray<cl_int> output_array(4);
  cl_image_format image_format;
  cl_image_desc desc;
  cl_mem image = nullptr;

  // initialize arrays
  for (int i = 0; i < 4; i++) {
    output_array.dynamic_array[i] = 0;
  }
  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 2; i++) {
      cpu_input_image_array.dynamic_array[j].s[i] = 1;
    }
    for (int i = 2; i < 4; i++) {
      cpu_input_image_array.dynamic_array[j].s[i] = 0;
    }
  }

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE2D_ARRAY;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 1;
  desc.image_depth = 0;
  desc.image_array_size = 4;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;
  size_t origin[] = {0, 0, 3};
  size_t region[] = {1, 1, 1};

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

  // initialize environment
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "read_image_kernel.cl"));

  // create image
  ASSERT_NO_FATAL_FAILURE(createImage(
      &image, ocl_descriptor.context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
      &image_format, &desc, cpu_input_image_array.dynamic_array));

  // create buffer
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.out_common_buffer, ocl_descriptor.context,
                   CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int4),
                   output_array.dynamic_array));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels,
                                       ocl_descriptor.program,
                                       "read_write_image_array_2D_int4"));

  // set kernel arg
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &image));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_mem), &image));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadImage(
      ocl_descriptor.queues[0], image, CL_TRUE, origin, region, 0, 0,
      cpu_input_image_array.dynamic_array, 0, NULL, NULL));

  for (int i = 0; i < 2; i++) {
    ASSERT_EQ(0, cpu_input_image_array.dynamic_array[0].s[i])
        << "image was not copyed properly";
  }
  for (int i = 2; i < 4; i++) {
    ASSERT_EQ(1, cpu_input_image_array.dynamic_array[0].s[i])
        << "image was not copyed properly";
  }
}

//|  TEST: CRT12_VR158_173.ImageVisibility3D_VR157
//|
//|  Purpose
//|  -------
//|
//|  Verify that A three-dimensional Array of texture object, in all
// supported formats, should be visible to all device types (CPU and GPU) in
// context.
//|
//|  Method
//|  ------
//|
//| 1.create context and such
//| 2.create a 3d image object
//| 3.run kernel on device
//| 4.copy image back to host
//| 5.check result
//|
//| Kernel
//|  ------
//| transforming 1 to 0 and 0 to 1(reads and write from image)
//|
//|  Pass criteria
//|  -------------
//|
//|  image  should change according to kernel.
//|

TEST_F(CRT12_VR158_173, DISABLED_ImageVisibility3D_VR157) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  DynamicArray<cl_int4> cpu_input_image_array(1);
  DynamicArray<cl_int> output_array(4);
  cl_image_format image_format;
  cl_image_desc desc;
  cl_mem image = nullptr;

  // initialize arrays
  for (int i = 0; i < 4; i++) {
    output_array.dynamic_array[i] = 0;
  }
  for (int i = 0; i < 2; i++) {
    cpu_input_image_array.dynamic_array[0].s[i] = 1;
  }
  for (int i = 2; i < 4; i++) {
    cpu_input_image_array.dynamic_array[0].s[i] = 0;
  }

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE3D;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 1;
  desc.image_depth = 1;
  desc.image_array_size = 0;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;
  size_t origin[] = {0, 0, 0};
  size_t region[] = {1, 1, 1};

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

  // initialize environment
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "read_image_kernel.cl"));

  // create image
  ASSERT_NO_FATAL_FAILURE(createImage(
      &image, ocl_descriptor.context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
      &image_format, &desc, cpu_input_image_array.dynamic_array));

  // create buffer
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.out_common_buffer, ocl_descriptor.context,
                   CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int4),
                   output_array.dynamic_array));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(createKernel(ocl_descriptor.kernels,
                                       ocl_descriptor.program,
                                       "read_write_image3D_int4"));

  // set kernel arg
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &image));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 1, sizeof(cl_mem), &image));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 0, NULL, NULL));

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(enqueueReadImage(
      ocl_descriptor.queues[0], image, CL_TRUE, origin, region, 0, 0,
      cpu_input_image_array.dynamic_array, 0, NULL, NULL));

  for (int i = 0; i < 2; i++) {
    ASSERT_EQ(0, cpu_input_image_array.dynamic_array[0].s[i])
        << "image was not copyed properly";
  }
  for (int i = 2; i < 4; i++) {
    ASSERT_EQ(1, cpu_input_image_array.dynamic_array[0].s[i])
        << "image was not copyed properly";
  }
}

//|  TEST: CRT12_VR158_173.WriteImage1D_VR158
//|
//|  Purpose
//|  -------
//|
//|  Verify that CPU and GPU are able to simultaneously access different
// one-dimensional texture objects which are part of a single one-dimensional
// array of texture objects using the function ckEnqueueWriteImage().
//|
//|  Method
//|  ------
//|
//| 1.create context and such
//| 2.create a 1d array of images object of size 2
//| 3.write one image on CPU and one on GPU
//| 4.read the image back to buffer
//| 5.check result
//|
//|
//|
//|  Pass criteria
//|  -------------
//|
//|  image array should change from 0 to 1.
//|

TEST_F(CRT12_VR158_173, WriteImage1D_VR158) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  DynamicArray<cl_int4> image_array(4);
  DynamicArray<cl_int4> output_array(4);
  DynamicArray<cl_int4> input_array(4);
  cl_image_format image_format;
  cl_image_desc desc;
  cl_mem image = nullptr;
  cl_event user_event;
  cl_event CPU_event;
  cl_event GPU_event;
  // initialize arrays
  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 4; i++) {
      output_array.dynamic_array[i].s[j] = 0;
      input_array.dynamic_array[i].s[j] = 1;
      image_array.dynamic_array[i].s[j] = 0;
    }
  }

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE1D_ARRAY;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 0;
  desc.image_depth = 0;
  desc.image_array_size = 2;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;
  size_t origin[] = {0, 0, 0};
  size_t region[] = {1, 1, 1};

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

  // initialize environment
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "read_image_kernel.cl"));

  // create image
  ASSERT_NO_FATAL_FAILURE(createImage(
      &image, ocl_descriptor.context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
      &image_format, &desc, image_array.dynamic_array));

  // create event
  createUserEvent(&user_event, ocl_descriptor.context);
  // create buffer
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.out_common_buffer, ocl_descriptor.context,
                   CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int4),
                   output_array.dynamic_array));

  // write using CPU
  ASSERT_NO_FATAL_FAILURE(enqueueWriteImage(
      ocl_descriptor.queues[0], image, CL_FALSE, origin, region, 0, 0,
      input_array.dynamic_array, 1, &user_event, &CPU_event));
  origin[1] = 1;
  // write using GPU
  ASSERT_NO_FATAL_FAILURE(enqueueWriteImage(
      ocl_descriptor.queues[1], image, CL_FALSE, origin, region, 0, 0,
      input_array.dynamic_array, 1, &user_event, &GPU_event));
  ASSERT_NO_FATAL_FAILURE(setUserEventStatus(user_event, CL_COMPLETE));

  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[0]));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[1]));

  origin[1] = 0;
  region[1] = 2;

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(
      enqueueReadImage(ocl_descriptor.queues[0], image, CL_TRUE, origin, region,
                       0, 0, output_array.dynamic_array, 0, NULL, NULL));

  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[0]));
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      ASSERT_EQ(1, output_array.dynamic_array[i].s[j])
          << "image was not copyed properly";
    }
  }
}

//|  TEST: CRT12_VR158_173.WriteImage2D_VR159
//|
//|  Purpose
//|  -------
//|
//|  Verify that CPU and GPU are able to simultaneously access different
// one-dimensional texture objects which are part of a single one-dimensional
// array of texture objects using the function ckEnqueueWriteImage().
//|
//|  Method
//|  ------
//|
//| 1.create context and such
//| 2.create a 1d array of images object of size 2
//| 3.write one image on CPU and one on GPU
//| 4.read the image back to buffer
//| 5.check result
//|
//|
//|
//|  Pass criteria
//|  -------------
//|
//|  image array should change from 0 to 1.
//|

TEST_F(CRT12_VR158_173, WriteImage2D_VR159) {
  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  DynamicArray<cl_int4> image_array(4);
  DynamicArray<cl_int4> output_array(4);
  DynamicArray<cl_int4> input_array(4);
  cl_image_format image_format;
  cl_image_desc desc;
  cl_mem image = nullptr;
  cl_event user_event = nullptr;
  cl_event CPU_event;
  cl_event GPU_event;
  // initialize arrays
  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 4; i++) {
      output_array.dynamic_array[i].s[j] = 0;
      input_array.dynamic_array[i].s[j] = 1;
      image_array.dynamic_array[i].s[j] = 0;
    }
  }

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE2D_ARRAY;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 1;
  desc.image_depth = 0;
  desc.image_array_size = 2;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;
  size_t origin[] = {0, 0, 0};
  size_t region[] = {1, 1, 1};

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

  // initialize environment
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "read_image_kernel.cl"));

  // create image
  ASSERT_NO_FATAL_FAILURE(createImage(
      &image, ocl_descriptor.context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
      &image_format, &desc, image_array.dynamic_array));

  // create event
  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));
  // create buffer
  ASSERT_NO_FATAL_FAILURE(
      createBuffer(&ocl_descriptor.out_common_buffer, ocl_descriptor.context,
                   CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int4),
                   output_array.dynamic_array));

  // write using CPU
  ASSERT_NO_FATAL_FAILURE(enqueueWriteImage(
      ocl_descriptor.queues[0], image, CL_FALSE, origin, region, 0, 0,
      input_array.dynamic_array, 1, &user_event, &CPU_event));
  origin[2] = 1;
  // write using GPU
  ASSERT_NO_FATAL_FAILURE(enqueueWriteImage(
      ocl_descriptor.queues[1], image, CL_FALSE, origin, region, 0, 0,
      input_array.dynamic_array, 1, &user_event, &GPU_event));
  ASSERT_NO_FATAL_FAILURE(setUserEventStatus(user_event, CL_COMPLETE));

  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[0]));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[1]));

  origin[2] = 0;
  region[2] = 2;

  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(
      enqueueReadImage(ocl_descriptor.queues[0], image, CL_TRUE, origin, region,
                       0, 0, output_array.dynamic_array, 0, NULL, NULL));

  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[0]));
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      ASSERT_EQ(1, output_array.dynamic_array[i].s[j])
          << "image was not copyed properly";
    }
  }
}

//|  TEST: CRT12_VR158_173.BuildInFunction1D_VR160
//|
//|  Purpose
//|  -------
//|
//|  Verify that CPU and GPU on shared context can access different
// one-dimensional texture objects which are part of a single | one-dimensional
// array of texture objects using built-in functions.
//|
//|  Method
//|  ------
//|
//| 1.  Create context and such
//| 2.  create a 1d image (texture) object
//| 3.  Create a memobject
//| 4.  copy the image to a buffer object on device
//| 5.  check result
//|
//| Kernel
//|  ------
//| writes 1's into image
//|
//|
//|
//|  Pass criteria
//|  -------------
//|
//|  image array should change according to kernel.
//|

TEST_F(CRT12_VR158_173, BuildInFunction1D_VR160) {

  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  DynamicArray<cl_int4> image_array(8);
  DynamicArray<cl_int4> output_array(4);
  cl_image_format image_format;
  cl_image_desc desc;
  cl_mem image = nullptr;
  cl_event user_event;
  // initialize arrays
  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 4; i++) {
      output_array.dynamic_array[i].s[j] = 0;
      image_array.dynamic_array[i].s[j] = 0;
    }
  }

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE1D_ARRAY;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 0;
  desc.image_depth = 0;
  desc.image_array_size = 2;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;
  size_t origin[] = {0, 0, 0};
  size_t region[] = {1, 2, 1};
  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

  // initialize environment
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "read_image_kernel.cl"));

  // create user event
  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  // initialize user event
  createUserEvent(&user_event, ocl_descriptor.context);

  // create image
  ASSERT_NO_FATAL_FAILURE(createImage(
      &image, ocl_descriptor.context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
      &image_format, &desc, image_array.dynamic_array));

  // create kernels
  ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[0],
                                       ocl_descriptor.program,
                                       "write_image1D_int4_CPU"));
  ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[1],
                                       ocl_descriptor.program,
                                       "write_image1D_int4_GPU"));

  // set kernel arg
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &image));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[1], 0, sizeof(cl_mem), &image));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 1, &user_event, NULL));

  // enqueue kernel on GPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[1], ocl_descriptor.kernels[1], work_dim, 0,
      &global_work_size, &local_work_size, 1, &user_event, NULL));
  ASSERT_NO_FATAL_FAILURE(setUserEventStatus(user_event, CL_COMPLETE));

  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[0]));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[1]));
  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(
      enqueueReadImage(ocl_descriptor.queues[0], image, CL_TRUE, origin, region,
                       0, 0, output_array.dynamic_array, 0, NULL, NULL));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[0]));
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      ASSERT_EQ(1, output_array.dynamic_array[i].s[j])
          << "image was not copyed properly";
    }
  }
}

//|  TEST: CRT12_VR158_173.BuildInFunction2D_VR161
//|
//|  Purpose
//|  -------
//|
//|  Verify that CPU and GPU on shared context can access different
// two-dimensional texture objects which are part of a single | one-dimensional
// array of texture objects using built-in functions.
//|
//|  Method
//|  ------
//|
//| 1.  Create context and such
//| 2.  create a 2d image (texture) object
//| 3.  Create a memobject
//| 4.  copy the image to a buffer object on device
//| 5.  check result
//|
//| Kernel
//|  ------
//| writes 1's into image
//|
//|
//|
//|  Pass criteria
//|  -------------
//|
//|  image array should change according to kernel.
//|

TEST_F(CRT12_VR158_173, BuildInFunction2D_VR161) {

  cl_uint work_dim = 1;
  size_t global_work_size = 1;
  size_t local_work_size = 1;
  DynamicArray<cl_int4> image_array(8);
  DynamicArray<cl_int4> output_array(4);
  cl_image_format image_format;
  cl_image_desc desc;
  cl_mem image = nullptr;
  cl_event user_event;
  // initialize arrays
  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 4; i++) {
      output_array.dynamic_array[i].s[j] = 0;
      image_array.dynamic_array[i].s[j] = 0;
    }
  }

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE2D_ARRAY;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 1;
  desc.image_depth = 0;
  desc.image_array_size = 2;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;
  size_t origin[] = {0, 0, 0};
  size_t region[] = {1, 1, 2};

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

  // initialize environment
  ASSERT_NO_FATAL_FAILURE(
      setUpContextProgramQueues(ocl_descriptor, "read_image_kernel.cl"));

  // create user event
  ASSERT_NO_FATAL_FAILURE(createUserEvent(&user_event, ocl_descriptor.context));

  // initialize user event
  createUserEvent(&user_event, ocl_descriptor.context);

  // create image
  ASSERT_NO_FATAL_FAILURE(createImage(
      &image, ocl_descriptor.context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
      &image_format, &desc, image_array.dynamic_array));

  // create kernels
  ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[0],
                                       ocl_descriptor.program,
                                       "write_image2D_int4_CPU"));
  ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[1],
                                       ocl_descriptor.program,
                                       "write_image2D_int4_GPU"));

  // set kernel arg
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[0], 0, sizeof(cl_mem), &image));
  ASSERT_NO_FATAL_FAILURE(
      setKernelArg(ocl_descriptor.kernels[1], 0, sizeof(cl_mem), &image));

  // enqueue kernel on CPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[0], ocl_descriptor.kernels[0], work_dim, 0,
      &global_work_size, &local_work_size, 1, &user_event, NULL));

  // enqueue kernel on GPU
  ASSERT_NO_FATAL_FAILURE(enqueueNDRangeKernel(
      ocl_descriptor.queues[1], ocl_descriptor.kernels[1], work_dim, 0,
      &global_work_size, &local_work_size, 1, &user_event, NULL));
  ASSERT_NO_FATAL_FAILURE(setUserEventStatus(user_event, CL_COMPLETE));

  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[0]));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[1]));
  // read from CPU device
  ASSERT_NO_FATAL_FAILURE(
      enqueueReadImage(ocl_descriptor.queues[0], image, CL_TRUE, origin, region,
                       0, 0, output_array.dynamic_array, 0, NULL, NULL));
  ASSERT_NO_FATAL_FAILURE(finish(ocl_descriptor.queues[0]));
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      ASSERT_EQ(1, output_array.dynamic_array[i].s[j])
          << "image was not copyed properly";
    }
  }
}

//|  TEST: CRT12_VR158_173.CheckChange1dCPUGPU_VR162
//|
//|  Purpose
//|  -------
//|
//|  Changes which are done by CPU device in a one-dimensional texture object
// are reflected in GPU
//|
//|  Method
//|  ------
//|
//| see CheckChangeImage
//|
//|  image type = 1D
//|
//|  first device = CPU
//|  first device kernel = write_image1D_int4
//|
//|  second device = GPU
//|  second device kernel = change_1_to_2_image1D_int4
//|
//|
TEST_F(CRT12_VR158_173, CheckChange1dCPUGPU_VR162) {
  cl_image_desc desc;

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE1D;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 0;
  desc.image_depth = 0;
  desc.image_array_size = 0;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;
  size_t origin[] = {0, 0, 0};
  ASSERT_NO_FATAL_FAILURE(CheckChangeImage(ocl_descriptor, desc, CPU_DEVICE,
                                           GPU_DEVICE, "write_image1D_int4",
                                           "change_1_to_2_image1D_int4"));
}

//|  TEST: CRT12_VR158_173.CheckChange1dBufferCPUGPU_VR163
//|
//|  Purpose
//|  -------
//|
//|  Changes which are done by CPU device in a one-dimensional texture buffer
// object are reflected in GPU
//|
//|  Method
//|  ------
//|
//| see CheckChangeImage
//|
//|  image type = 1D  buffer
//|
//|  first device = CPU
//|  first device kernel = write_image1D_int4
//|
//|  second device = GPU
//|  second device kernel = change_1_to_2_image1D_int4
//|
//|
TEST_F(CRT12_VR158_173, CheckChange1dBufferCPUGPU_VR163) {
  cl_image_desc desc;

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE1D_BUFFER;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 0;
  desc.image_depth = 0;
  desc.image_array_size = 0;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = (cl_mem)1; // this mean that a new buffer will be
                               // initialized

  ASSERT_NO_FATAL_FAILURE(CheckChangeImage(
      ocl_descriptor, desc, CPU_DEVICE, GPU_DEVICE, "write_image1D_buffer_int4",
      "change_1_to_2_image1D_buffer_int4"));
}

//|  TEST: CRT12_VR158_173.CheckChange1dArrayCPUGPU_VR164
//|
//|  Purpose
//|  -------
//|
//|  Changes which are done by CPU device in a one-dimensional array of
// texture object are reflected in GPU
//|
//|  Method
//|  ------
//|
//| see CheckChangeImage
//|
//|  image type = 1D  array
//|
//|  first device = CPU
//|  first device kernel = write_image1D_int4_CPU
//|
//|  second device = GPU
//|  second device kernel = change_1_to_2_image_array_1D_int4
//|
//|
TEST_F(CRT12_VR158_173, CheckChange1dArrayCPUGPU_VR164) {
  cl_image_desc desc;

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE1D_ARRAY;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 0;
  desc.image_depth = 0;
  desc.image_array_size = 1;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;

  ASSERT_NO_FATAL_FAILURE(CheckChangeImage(
      ocl_descriptor, desc, CPU_DEVICE, GPU_DEVICE, "write_image1D_int4_CPU",
      "change_1_to_2_image_array_1D_int4"));
}

//|  TEST: CRT12_VR158_173.CheckChange2dCPUGPU_VR165
//|
//|  Purpose
//|  -------
//|
//|  Changes which are done by CPU device in a two-dimensional texture object
// are reflected in GPU
//|
//|  Method
//|  ------
//|
//| see CheckChangeImage
//|
//|  image type = 2D
//|
//|  first device = CPU
//|  first device kernel = write_image2D_int4
//|
//|  second device = GPU
//|  second device kernel = change_1_to_2_image2D_int4
//|
//|
TEST_F(CRT12_VR158_173, CheckChange2dCPUGPU_VR165) {
  cl_image_desc desc;

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 1;
  desc.image_depth = 0;
  desc.image_array_size = 0;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;

  ASSERT_NO_FATAL_FAILURE(CheckChangeImage(ocl_descriptor, desc, CPU_DEVICE,
                                           GPU_DEVICE, "write_image2D_int4",
                                           "change_1_to_2_image2D_int4"));
}

//|  TEST: CRT12_VR158_173.CheckChange2darrayCPUGPU_VR166
//|
//|  Purpose
//|  -------
//|
//|  Changes which are done by CPU device in a two-dimensional array object
// are reflected in GPU
//|
//|  Method
//|  ------
//|
//| see CheckChangeImage
//|
//|  image type = 2D  array
//|
//|  first device = CPU
//|  first device kernel = write_image2D_int4_array
//|
//|  second device = GPU
//|  second device kernel = change_1_to_2_image_array_2D_int4
//|
//|

TEST_F(CRT12_VR158_173, CheckChange2darrayCPUGPU_VR166) {
  cl_image_desc desc;

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE2D_ARRAY;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 1;
  desc.image_depth = 0;
  desc.image_array_size = 1;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;

  ASSERT_NO_FATAL_FAILURE(CheckChangeImage(
      ocl_descriptor, desc, CPU_DEVICE, GPU_DEVICE, "write_image2D_int4_array",
      "change_1_to_2_image_array_2D_int4"));
}

/**********************************this test is disabled because there is no
 * support for 3d image writing**********************************/
//|  TEST: CRT12_VR158_173.CheckChange3dCPUGPU_VR167
//|
//|  Purpose
//|  -------
//|
//|  Changes which are done by CPU device in a three-dimensional texture
// object are reflected in GPU
//|
//|  Method
//|  ------
//|
//| see CheckChangeImage
//|
//|  image type = 3D
//|
//|  first device = CPU
//|  first device kernel = write_image3D_int4
//|
//|  second device = GPU
//|  second device kernel = change_1_to_2_image3D_int4
//|
//|
TEST_F(CRT12_VR158_173, DISABLED_CheckChange3dCPUGPU_VR167) {
  cl_image_desc desc;

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 1;
  desc.image_depth = 1;
  desc.image_array_size = 0;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;

  ASSERT_NO_FATAL_FAILURE(CheckChangeImage(ocl_descriptor, desc, CPU_DEVICE,
                                           GPU_DEVICE, "write_image3D_int4",
                                           "change_1_to_2_image3D_int4"));
}

//|  TEST: CRT12_VR158_173.CheckChange1dGPUCPU_VR168
//|
//|  Purpose
//|  -------
//|
//|  Changes which are done by GPU device in a one-dimensional texture object
// are reflected in CPU
//|
//|  Method
//|  ------
//|
//| see CheckChangeImage
//|
//|  image type = 1D
//|
//|  first device = GPU
//|  first device kernel = write_image1D_int4
//|
//|  second device = CPU
//|  second device kernel = change_1_to_2_image1D_int4
//|
//|
TEST_F(CRT12_VR158_173, CheckChange1dGPUCPU_VR168) {
  cl_image_desc desc;

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE1D;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 0;
  desc.image_depth = 0;
  desc.image_array_size = 0;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;
  size_t origin[] = {0, 0, 0};
  ASSERT_NO_FATAL_FAILURE(CheckChangeImage(ocl_descriptor, desc, GPU_DEVICE,
                                           CPU_DEVICE, "write_image1D_int4",
                                           "change_1_to_2_image1D_int4"));
}

//|  TEST: CRT12_VR158_173.CheckChange1dBufferGPUCPU_VR169
//|
//|  Purpose
//|  -------
//|
//|  Changes which are done by GPU device in a one-dimensional texture buffer
// object are reflected in CPU
//|
//|  Method
//|  ------
//|
//| see CheckChangeImage
//|
//|  image type = 1D  buffer
//|
//|  first device = GPU
//|  first device kernel = write_image1D_int4
//|
//|  second device = CPU
//|  second device kernel = change_1_to_2_image1D_int4
//|
//|
TEST_F(CRT12_VR158_173, CheckChange1dBufferGPUCPU_VR169) {
  cl_image_desc desc;

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE1D_BUFFER;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 0;
  desc.image_depth = 0;
  desc.image_array_size = 0;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = (cl_mem)1; // this mean that a new buffer will be
                               // initialized

  ASSERT_NO_FATAL_FAILURE(CheckChangeImage(
      ocl_descriptor, desc, GPU_DEVICE, CPU_DEVICE, "write_image1D_buffer_int4",
      "change_1_to_2_image1D_buffer_int4"));
}

//|  TEST: CRT12_VR158_173.CheckChange1dArrayGPUCPU_VR170
//|
//|  Purpose
//|  -------
//|
//|  Changes which are done by GPU device in a one-dimensional array of
// texture object are reflected in CPU
//|
//|  Method
//|  ------
//|
//| see CheckChangeImage
//|
//|  image type = 1D  array
//|
//|  first device = GPU
//|  first device kernel = write_image1D_int4_CPU
//|
//|  second device = CPU
//|  second device kernel = change_1_to_2_image_array_1D_int4
//|
//|
TEST_F(CRT12_VR158_173, CheckChange1dArrayGPUCPU_VR170) {
  cl_image_desc desc;

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE1D_ARRAY;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 0;
  desc.image_depth = 0;
  desc.image_array_size = 1;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;

  ASSERT_NO_FATAL_FAILURE(CheckChangeImage(
      ocl_descriptor, desc, GPU_DEVICE, CPU_DEVICE, "write_image1D_int4_CPU",
      "change_1_to_2_image_array_1D_int4"));
}

//|  TEST: CRT12_VR158_173.CheckChange2dGPUCPU_VR171
//|
//|  Purpose
//|  -------
//|
//|  Changes which are done by GPU device in a two-dimensional texture object
// are reflected in CPU
//|
//|  Method
//|  ------
//|
//| see CheckChangeImage
//|
//|  image type = 2D
//|
//|  first device = GPU
//|  first device kernel = write_image2D_int4
//|
//|  second device = CPU
//|  second device kernel = change_1_to_2_image2D_int4
//|
//|
TEST_F(CRT12_VR158_173, CheckChange2dGPUCPU_VR171) {
  cl_image_desc desc;

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 1;
  desc.image_depth = 0;
  desc.image_array_size = 0;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;

  ASSERT_NO_FATAL_FAILURE(CheckChangeImage(ocl_descriptor, desc, GPU_DEVICE,
                                           CPU_DEVICE, "write_image2D_int4",
                                           "change_1_to_2_image2D_int4"));
}

//|  TEST: CRT12_VR158_173.CheckChange2dArrayGPUCPU_VR172
//|
//|  Purpose
//|  -------
//|
//|  Changes which are done by CPU device in a two-dimensional array object
// are reflected in GPU
//|
//|  Method
//|  ------
//|
//| see CheckChangeImage
//|
//|  image type = 2D  array
//|
//|  first device = CPU
//|  first device kernel = write_image2D_int4_array
//|
//|  second device = GPU
//|  second device kernel = change_1_to_2_image_array_2D_int4
//|
//|
TEST_F(CRT12_VR158_173, CheckChange2dArrayGPUCPU_VR172) {
  cl_image_desc desc;

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE2D_ARRAY;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 1;
  desc.image_depth = 0;
  desc.image_array_size = 1;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;

  ASSERT_NO_FATAL_FAILURE(CheckChangeImage(
      ocl_descriptor, desc, GPU_DEVICE, CPU_DEVICE, "write_image2D_int4_array",
      "change_1_to_2_image_array_2D_int4"));
}

/**********************************this test is disabled because there is no
 * support for 3d image writing**********************************/
//|  TEST: CRT12_VR158_173.CheckChange3dGPUCPU_VR173
//|
//|  Purpose
//|  -------
//|
//|  Changes which are done by CPU device in a three-dimensional texture
// object are reflected in GPU
//|
//|  Method
//|  ------
//|
//| see CheckChangeImage
//|
//|  image type = 3D
//|
//|  first device = GPU
//|  first device kernel = write_image3D_int4
//|
//|  second device = CPU
//|  second device kernel = change_1_to_2_image3D_int4
//|
//|
TEST_F(CRT12_VR158_173, DISABLED_CheckChange3dGPUCPU_VR173) {
  cl_image_desc desc;

  // initialize image descriptor
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = 1;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.image_height = 1;
  desc.image_depth = 1;
  desc.image_array_size = 0;
  desc.image_row_pitch = 0;
  desc.image_slice_pitch = 0;
  desc.num_mip_levels = 0;
  desc.num_samples = 0;
  desc.mem_object = 0;

  ASSERT_NO_FATAL_FAILURE(CheckChangeImage(ocl_descriptor, desc, GPU_DEVICE,
                                           CPU_DEVICE, "write_image3D_int4",
                                           "change_1_to_2_image3D_int4"));
}
