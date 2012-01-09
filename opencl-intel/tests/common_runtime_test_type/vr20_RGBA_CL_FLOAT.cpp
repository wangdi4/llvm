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

#include "vr20_image.h"

//	FOR MORE INFORMATION ON THE FOLLOWING TESTS PLEASE REFER TO "vr20_image.h" file

//	VR20_RGBA_CL_FLOAT - tests for images with settings RGBA and CL_FLOAT
template <typename T>
class VR20_RGBA_CL_FLOAT : public ImageTypedCommonRuntime<T>{
public:

	virtual void SetUp() 
	{
		image_format.image_channel_order = CL_RGBA;
		image_format.image_channel_data_type = CL_FLOAT;
	}
};
// Do not add other types here
typedef ::testing::Types<cl_float4> VR20_RGBA_CL_FLOATTypes;
TYPED_TEST_CASE(VR20_RGBA_CL_FLOAT, VR20_RGBA_CL_FLOATTypes);

static const char* d2KernelName = "read_write_image2D_float4";
static const char* d3KernelName = "read_write_image3D_float4";

TYPED_TEST(VR20_RGBA_CL_FLOAT, Image2DReadWriteUseHostPtrCPUGPU)
{
	ASSERT_NO_FATAL_FAILURE(test2DReadWriteCommands<TypeParam>(ocl_descriptor, image_format, d2KernelName));
}

TYPED_TEST(VR20_RGBA_CL_FLOAT, Image3DReadWriteUseHostPtrCPUGPU)
{
	ASSERT_NO_FATAL_FAILURE(test2DReadWriteCommands<TypeParam>(ocl_descriptor, image_format, d3KernelName));
}

TYPED_TEST(VR20_RGBA_CL_FLOAT, Image2DReadWriteKernelCPUGPU)
{
	ASSERT_NO_FATAL_FAILURE(test2DReadWriteThroughKernel<TypeParam>(ocl_descriptor, image_format, d2KernelName));
}

TYPED_TEST(VR20_RGBA_CL_FLOAT, Image3DReadWriteKernelCPUGPU)
{
	ASSERT_NO_FATAL_FAILURE(test3DReadWriteThroughKernel<TypeParam>(ocl_descriptor, image_format, d3KernelName));
}