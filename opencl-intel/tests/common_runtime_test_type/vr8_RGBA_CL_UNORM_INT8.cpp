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

#include <limits.h>
#include "vr8_image.h"

//	FOR MORE INFORMATION ON THE FOLLOWING TESTS PLEASE REFER TO "vr8_image.h" file

//	VR8_RGBA_CL_UNORM_INT8 - tests for images with settings RGBA and CL_UNORM_INT8
template <typename T>
class VR8_RGBA_CL_UNORM_INT8 : public ImageTypedCommonRuntime<T>{
public:

	virtual void SetUp() 
	{
		this->image_format.image_channel_order = CL_RGBA;
		this->image_format.image_channel_data_type = CL_UNORM_INT8;
	}
};

// Do not add other types here
typedef ::testing::Types<cl_uchar4> VR8_RGBA_CL_UNORM_INT8Types;
TYPED_TEST_CASE(VR8_RGBA_CL_UNORM_INT8, VR8_RGBA_CL_UNORM_INT8Types);

static float divisor = UCHAR_MAX;
static const char* d2KernelName = "read_image2D_float4";
static const char* d3KernelName = "read_image3D_float4";

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//	TESTING VR8_RGBA_CL_UNORM_INT8 - for tests description see vc8_imageCL_FLOAT.cpp
////////////////////////////////////////////////////////////////////////////////////////////////////////
TYPED_TEST(VR8_RGBA_CL_UNORM_INT8, Image2DUseHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(test2DUseHostPtr<TypeParam>(this->ocl_descriptor, this->image_format, d2KernelName, divisor));
}

TYPED_TEST(VR8_RGBA_CL_UNORM_INT8, Image2AllocHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(test2DAllocHostPtr<TypeParam>(this->ocl_descriptor, this->image_format, d2KernelName, divisor));
}

TYPED_TEST(VR8_RGBA_CL_UNORM_INT8, Image2CopyHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(test2DCopyHostPtr<TypeParam>(this->ocl_descriptor, this->image_format, d2KernelName, divisor));
}

TYPED_TEST(VR8_RGBA_CL_UNORM_INT8, Image3DUseHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(test3DUseHostPtr<TypeParam>(this->ocl_descriptor, this->image_format, d3KernelName, divisor));
}

TYPED_TEST(VR8_RGBA_CL_UNORM_INT8, Image3DAllocHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(test3DAllocHostPtr<TypeParam>(this->ocl_descriptor, this->image_format, d3KernelName, divisor));
}

TYPED_TEST(VR8_RGBA_CL_UNORM_INT8, Image3DCopyHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(test3DCopyHostPtr<TypeParam>(this->ocl_descriptor, this->image_format, d3KernelName, divisor));
}
