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

#include "vr8_image.h"

//  FOR MORE INFORMATION ON THE FOLLOWING TESTS PLEASE REFER TO
//"vr8_image.h" file   VR8_CL_A_CL_HALF_FLOAT - tests for images with settings
// CL_A and CL_HALF_FLOAT
template <typename T>
class VR8_CL_A_CL_HALF_FLOAT : public ImageTypedCommonRuntime<T> {
public:
  virtual void SetUp() {
    this->image_format.image_channel_order = CL_A;
    this->image_format.image_channel_data_type = CL_HALF_FLOAT;
  }
};
// Do not add other types here
typedef ::testing::Types<cl_half> VR8_CL_A_CL_HALF_FLOATTypes;
TYPED_TEST_CASE(VR8_CL_A_CL_HALF_FLOAT, VR8_CL_A_CL_HALF_FLOATTypes);

// succDevicesNum - device index for device which should succeed in final
// validation test if equal to 2 - should succeed on both devices if equal to 0
// - should succeed on CPU and fail on GPU if equal to 1 - should succeed on GPU
// and fail on CPU
static int succDevicesNum = 0;
static const char *d2KernelName = "read_image2D_float4_last";
static const char *d3KernelName = "read_image3D_float4_last";

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  TESTING VR8_CL_A_CL_HALF_FLOAT - for tests description see
// vc8_imageCL_FLOAT.cpp
////////////////////////////////////////////////////////////////////////////////////////////////////////
TYPED_TEST(VR8_CL_A_CL_HALF_FLOAT, Image2DUseHostPtr) {
  ASSERT_NO_FATAL_FAILURE(
      test2DUseHostPtr<TypeParam>(this->ocl_descriptor, this->image_format,
                                  d2KernelName, 1, succDevicesNum, true));
}

TYPED_TEST(VR8_CL_A_CL_HALF_FLOAT, Image2AllocHostPtr) {
  ASSERT_NO_FATAL_FAILURE(
      test2DAllocHostPtr<TypeParam>(this->ocl_descriptor, this->image_format,
                                    d2KernelName, 1, succDevicesNum, true));
}

TYPED_TEST(VR8_CL_A_CL_HALF_FLOAT, Image2CopyHostPtr) {
  ASSERT_NO_FATAL_FAILURE(
      test2DCopyHostPtr<TypeParam>(this->ocl_descriptor, this->image_format,
                                   d2KernelName, 1, succDevicesNum, true));
}

TYPED_TEST(VR8_CL_A_CL_HALF_FLOAT, Image3DUseHostPtr) {
  ASSERT_NO_FATAL_FAILURE(
      test3DUseHostPtr<TypeParam>(this->ocl_descriptor, this->image_format,
                                  d3KernelName, 1, succDevicesNum, true));
}

TYPED_TEST(VR8_CL_A_CL_HALF_FLOAT, Image3DAllocHostPtr) {
  ASSERT_NO_FATAL_FAILURE(
      test3DAllocHostPtr<TypeParam>(this->ocl_descriptor, this->image_format,
                                    d3KernelName, 1, succDevicesNum, true));
}

TYPED_TEST(VR8_CL_A_CL_HALF_FLOAT, Image3DCopyHostPtr) {
  ASSERT_NO_FATAL_FAILURE(
      test3DCopyHostPtr<TypeParam>(this->ocl_descriptor, this->image_format,
                                   d3KernelName, 1, succDevicesNum, true));
}
