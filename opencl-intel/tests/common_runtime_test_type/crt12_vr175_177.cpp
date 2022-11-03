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

class CRT12_VR175_177 : public CommonRuntime {};

static bool isFormatInList(cl_channel_order order, cl_channel_type data_type,
                           cl_uint list_size, cl_image_format *list) {
  for (cl_uint i = 0; i < list_size; i++) {
    if (list[i].image_channel_data_type == data_type &&
        list[i].image_channel_order == order) {
      return true;
    }
  }
  return false;
}

//|  checkSupportedFormats
//|  Purpose
//|  -------
//|
//|  verify that the function clGetSupportedImageFormats() returns all the
// suported image formats
//|
//|  Method
//|  ------
//|
//|  1. Create a shared context
//|  2. Create a memory object with the given flag
//| 3. get the list of supported format
//|  4. check the returned list for each of the returned values.
//|
//|  Pass criteria
//|  -------------
//|
//|  context should support all the listed image formats for all memory flags
//|

void checkSupportedFormats(cl_mem_object_type mem_type,
                           OpenCLDescriptor ocl_descriptor) {
  cl_image_format image_formats_array[100];
  cl_uint number_of_suported_formats;
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

  // query for supported formats
  ASSERT_NO_FATAL_FAILURE(getSupportedImageFormats(
      ocl_descriptor.context, mem_type | CL_MEM_USE_HOST_PTR,
      CL_MEM_OBJECT_IMAGE1D, 100, image_formats_array,
      &number_of_suported_formats));

  // check that all the formats are supported.
  ASSERT_TRUE(isFormatInList(CL_RGBA, CL_UNORM_INT8, number_of_suported_formats,
                             image_formats_array))
      << "format CL_RGBA, CL_UNORM_INT8 was not returned";
  ASSERT_TRUE(isFormatInList(CL_RGBA, CL_UNORM_INT16,
                             number_of_suported_formats, image_formats_array))
      << "format CL_RGBA, CL_UNORM_INT16 was not returned";
  ASSERT_TRUE(isFormatInList(CL_RGBA, CL_SIGNED_INT8,
                             number_of_suported_formats, image_formats_array))
      << "format CL_RGBA, CL_SIGNED_INT8 was not returned";
  ASSERT_TRUE(isFormatInList(CL_RGBA, CL_SIGNED_INT16,
                             number_of_suported_formats, image_formats_array))
      << "format CL_RGBA, CL_SIGNED_INT16 was not returned";
  ASSERT_TRUE(isFormatInList(CL_RGBA, CL_SIGNED_INT32,
                             number_of_suported_formats, image_formats_array))
      << "format CL_RGBA, CL_SIGNED_INT32 was not returned";
  ASSERT_TRUE(isFormatInList(CL_RGBA, CL_UNSIGNED_INT8,
                             number_of_suported_formats, image_formats_array))
      << "format CL_RGBA, CL_UNSIGNED_INT8 was not returned";
  ASSERT_TRUE(isFormatInList(CL_RGBA, CL_UNSIGNED_INT16,
                             number_of_suported_formats, image_formats_array))
      << "format CL_RGBA, CL_UNSIGNED_INT16 was not returned";
  ASSERT_TRUE(isFormatInList(CL_RGBA, CL_UNSIGNED_INT32,
                             number_of_suported_formats, image_formats_array))
      << "format CL_RGBA, CL_UNSIGNED_INT32 was not returned";
  ASSERT_TRUE(isFormatInList(CL_RGBA, CL_HALF_FLOAT, number_of_suported_formats,
                             image_formats_array))
      << "format CL_RGBA, CL_HALF_FLOAT was not returned";
  ASSERT_TRUE(isFormatInList(CL_RGBA, CL_FLOAT, number_of_suported_formats,
                             image_formats_array))
      << "format CL_RGBA, CL_FLOAT was not returned";
  ASSERT_TRUE(isFormatInList(CL_RGBA, CL_UNORM_INT8, number_of_suported_formats,
                             image_formats_array))
      << "format CL_RGBA, CL_UNORM_INT8 was not returned";
  ASSERT_TRUE(isFormatInList(CL_R, CL_SIGNED_INT32, number_of_suported_formats,
                             image_formats_array))
      << "format CL_R, CL_SIGNED_INT32 was not returned";
  ASSERT_TRUE(isFormatInList(CL_INTENSITY, CL_FLOAT, number_of_suported_formats,
                             image_formats_array))
      << "format CL_INTENSITY, CL_FLOAT was not returned";
  ASSERT_TRUE(isFormatInList(CL_LUMINANCE, CL_FLOAT, number_of_suported_formats,
                             image_formats_array))
      << "format CL_LUMINANCE, CL_FLOAT was not returned";
}

//|  TEST: CRT12_VR175_177.image_channel_order_READ_WRITE_vr175
//|  Purpose
//|  -------
//|
//|  verify that the function clGetSupportedImageFormats() returns all the
// suported image formats
//|
//|  Method
//|  ------
//|
//|  1. Create a shared context
//|  2. Create a memory object with the flag CL_MEM_READ_WRITE
//| 3. get the list of supported format
//|  4. check the returned list for each of the returned values.
//|
//|  Pass criteria
//|  -------------
//|
//|  context should support all the listed image formats for
// CL_MEM_READ_WRITE
//|

TEST_F(CRT12_VR175_177, image_channel_order_READ_WRITE_vr175) {

  ASSERT_NO_FATAL_FAILURE(
      checkSupportedFormats(CL_MEM_READ_WRITE, ocl_descriptor));
}

//|  TEST: CRT12_VR175_177.image_channel_order_WRITE_ONLY_vr176
//|  Purpose
//|  -------
//|
//|  verify that the function clGetSupportedImageFormats() returns all the
// supported image formats
//|
//|  Method
//|  ------
//|
//|  1. Create a shared context
//|  2. Create a memory object with the flag CL_MEM_READ_ONLY
//| 3. get the list of supported format
//|  4. check the returned list for each of the returned values.
//|
//|  Pass criteria
//|  -------------
//|
//|  context should support all the listed image formats for CL_MEM_READ_ONLY
//|

TEST_F(CRT12_VR175_177, image_channel_order_WRITE_ONLY_vr176) {

  ASSERT_NO_FATAL_FAILURE(
      checkSupportedFormats(CL_MEM_WRITE_ONLY, ocl_descriptor));
}

//|  TEST: CRT12_VR175_177.image_channel_order_READ_ONLY_vr177
//|  Purpose
//|  -------
//|
//|  vertify that the function clGetSupportedImageFormats() returns all the
// suported image formats
//|
//|  Method
//|  ------
//|
//|  1. Create a shared context
//|  2. Create a memory object with the flag CL_MEM_READ_ONLY
//| 3. get the list of supported format
//|  4. check the returned list for each of the returned values.
//|
//|  Pass criteria
//|  -------------
//|
//|  context should support all the listed image formats for CL_MEM_READ_ONLY
//|

TEST_F(CRT12_VR175_177, image_channel_order_READ_ONLY_vr177) {

  ASSERT_NO_FATAL_FAILURE(
      checkSupportedFormats(CL_MEM_READ_ONLY, ocl_descriptor));
}
