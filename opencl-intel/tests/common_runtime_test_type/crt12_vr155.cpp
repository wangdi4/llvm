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
#include "crt12_vr152_157.h"

#define CPU_DEVICE 0
#define GPU_DEVICE 1

class CRT12_VR155 : public CommonRuntime {};

//|  TEST: CRT12_VR155.ImageVisibility1D_VR155
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
//| 2. create a 1d image object
//| 3.copy the image to a buffer object on device
//| 4.run kernel
//| 5.copy the image back to host
//| 6.check result
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

/*******************************RGBA*******************************/
TEST_F(CRT12_VR155, DISABLED_ImageVisibility1DCPU_CL_SNORM_INT8_crush_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SNORM_INT8;

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
  ImageVisibility1D_crush<cl_char4>(ocl_descriptor, image_format, desc,
                                    "copy_image2D", CPU_DEVICE, 4);
}
TEST_F(CRT12_VR155, ImageVisibility1D_CL_SNORM_INT8_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SNORM_INT16;

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
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            CPU_DEVICE, 4);
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            GPU_DEVICE, 4);
}
TEST_F(CRT12_VR155, ImageVisibility1D_CL_UNORM_INT8_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_UNORM_INT8;

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
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            CPU_DEVICE, 4);
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            GPU_DEVICE, 4);
}
TEST_F(CRT12_VR155, ImageVisibility1D_CL_SNORM_INT16_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SNORM_INT16;

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
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             CPU_DEVICE, 4);
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             GPU_DEVICE, 4);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_UNORM_INT16_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_UNORM_INT16;

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
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             CPU_DEVICE, 4);
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             GPU_DEVICE, 4);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_SIGNED_INT32_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_SIGNED_INT32;

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
  ImageVisibility<cl_int4>(ocl_descriptor, image_format, desc, "copy_image2D",
                           CPU_DEVICE, 4);
  ImageVisibility<cl_int4>(ocl_descriptor, image_format, desc, "copy_image2D",
                           GPU_DEVICE, 4);
}
TEST_F(CRT12_VR155, ImageVisibility1DCL_UNSIGNED_INT32_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_RGBA;
  image_format.image_channel_data_type = CL_UNSIGNED_INT32;

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
  ImageVisibility<cl_int4>(ocl_descriptor, image_format, desc, "copy_image2D",
                           CPU_DEVICE, 4);
  ImageVisibility<cl_int4>(ocl_descriptor, image_format, desc, "copy_image2D",
                           GPU_DEVICE, 4);
}

/*******************************INTENSITY*******************************/

TEST_F(CRT12_VR155, ImageVisibility1D_CL_INTENSITY_CL_UNORM_INT8_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_INTENSITY;
  image_format.image_channel_data_type = CL_UNORM_INT8;

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
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            CPU_DEVICE, 1);
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            GPU_DEVICE, 1);
}
TEST_F(CRT12_VR155, ImageVisibility1D_CL_INTENSITY_CL_SNORM_INT8_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_INTENSITY;
  image_format.image_channel_data_type = CL_SNORM_INT8;

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
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            CPU_DEVICE, 1);
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            GPU_DEVICE, 1);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_INTENSITY_CL_UNORM_INT16_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_INTENSITY;
  image_format.image_channel_data_type = CL_UNORM_INT16;

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
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             CPU_DEVICE, 1);
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             GPU_DEVICE, 1);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_INTENSITY_CL_SNORM_INT16_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_INTENSITY;
  image_format.image_channel_data_type = CL_SNORM_INT16;

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
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             CPU_DEVICE, 1);
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             GPU_DEVICE, 1);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_INTENSITY_CL_FLOAT_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_INTENSITY;
  image_format.image_channel_data_type = CL_FLOAT;

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
  ImageVisibility<cl_float4>(ocl_descriptor, image_format, desc,
                             "copy_image2D_float", CPU_DEVICE, 1);
  ImageVisibility<cl_float4>(ocl_descriptor, image_format, desc,
                             "copy_image2D_float", GPU_DEVICE, 1);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_INTENSITY_CL_HALF_FLOAT_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_INTENSITY;
  image_format.image_channel_data_type = CL_HALF_FLOAT;

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
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc,
                             "copy_image2D_float", CPU_DEVICE, 1);
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc,
                             "copy_image2D_float", GPU_DEVICE, 1);
}

/*******************************LUMINANCE*******************************/

TEST_F(CRT12_VR155, ImageVisibility1D_CL_LUMINANCE_CL_UNORM_INT8_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_LUMINANCE;
  image_format.image_channel_data_type = CL_UNORM_INT8;

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
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            CPU_DEVICE, 1);
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            GPU_DEVICE, 1);
}
TEST_F(CRT12_VR155, ImageVisibility1D_CL_LUMINANCE_CL_SNORM_INT8_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_LUMINANCE;
  image_format.image_channel_data_type = CL_SNORM_INT8;

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
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            CPU_DEVICE, 1);
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            GPU_DEVICE, 1);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_LUMINANCE_CL_UNORM_INT16_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_LUMINANCE;
  image_format.image_channel_data_type = CL_UNORM_INT16;

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
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             CPU_DEVICE, 1);
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             GPU_DEVICE, 1);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_LUMINANCE_CL_SNORM_INT16_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_LUMINANCE;
  image_format.image_channel_data_type = CL_SNORM_INT16;

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
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             CPU_DEVICE, 1);
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             GPU_DEVICE, 1);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_LUMINANCE_CL_FLOAT_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_LUMINANCE;
  image_format.image_channel_data_type = CL_FLOAT;

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
  ImageVisibility<cl_float4>(ocl_descriptor, image_format, desc,
                             "copy_image2D_float", CPU_DEVICE, 1);
  ImageVisibility<cl_float4>(ocl_descriptor, image_format, desc,
                             "copy_image2D_float", GPU_DEVICE, 1);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_LUMINANCE_CL_HALF_FLOAT_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_LUMINANCE;
  image_format.image_channel_data_type = CL_HALF_FLOAT;

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
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc,
                             "copy_image2D_float", CPU_DEVICE, 1);
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc,
                             "copy_image2D_float", GPU_DEVICE, 1);
}

/*******************************ARGB*******************************/

TEST_F(CRT12_VR155, ImageVisibility1D_CL_ARGB_CL_UNORM_INT8_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_ARGB;
  image_format.image_channel_data_type = CL_UNORM_INT8;

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
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            CPU_DEVICE, 4);
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            GPU_DEVICE, 4);
}
TEST_F(CRT12_VR155, ImageVisibility1D_CL_ARGB_CL_SNORM_INT8_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_ARGB;
  image_format.image_channel_data_type = CL_SNORM_INT8;

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
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            CPU_DEVICE, 4);
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            GPU_DEVICE, 4);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_ARGB_CL_UNORM_INT16_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_ARGB;
  image_format.image_channel_data_type = CL_UNORM_INT16;

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
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             CPU_DEVICE, 4);
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             GPU_DEVICE, 4);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_ARGB_CL_SNORM_INT16_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_ARGB;
  image_format.image_channel_data_type = CL_SNORM_INT16;

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
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             CPU_DEVICE, 4);
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             GPU_DEVICE, 4);
}

/*******************************BGRA*******************************/

TEST_F(CRT12_VR155, ImageVisibility1D_CL_BGRA_CL_UNORM_INT8_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_BGRA;
  image_format.image_channel_data_type = CL_UNORM_INT8;

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
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            CPU_DEVICE, 4);
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            GPU_DEVICE, 4);
}
TEST_F(CRT12_VR155, ImageVisibility1D_CL_BGRA_CL_SNORM_INT8_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_BGRA;
  image_format.image_channel_data_type = CL_SNORM_INT8;

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
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            CPU_DEVICE, 4);
  ImageVisibility<cl_char4>(ocl_descriptor, image_format, desc, "copy_image2D",
                            GPU_DEVICE, 4);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_BGRA_CL_UNORM_INT16_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_BGRA;
  image_format.image_channel_data_type = CL_UNORM_INT16;

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
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             CPU_DEVICE, 4);
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             GPU_DEVICE, 4);
}

TEST_F(CRT12_VR155, ImageVisibility1D_CL_BGRA_CL_SNORM_INT16_VR155) {
  cl_image_format image_format;
  cl_image_desc desc;

  // initialize image format
  image_format.image_channel_order = CL_BGRA;
  image_format.image_channel_data_type = CL_SNORM_INT16;

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
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             CPU_DEVICE, 4);
  ImageVisibility<cl_short4>(ocl_descriptor, image_format, desc, "copy_image2D",
                             GPU_DEVICE, 4);
}

// need to add CL_RG, CL_RGx or CL_RA
// and finish float support
