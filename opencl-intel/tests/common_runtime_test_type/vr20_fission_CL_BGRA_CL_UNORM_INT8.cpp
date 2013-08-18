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

//	ImageBGRA_CL_UNORM_INT8 - tests for images with settings BGRA and CL_UNORM_INT8
template <typename T>
class Fission_VR20_BGRA_CL_UNORM_INT8 : public ImageTypedCommonRuntime<T>, public FissionWrapper{
public:

	virtual void SetUp() 
	{
		FissionWrapper::SetUp();
		this->image_format.image_channel_order = CL_BGRA;
		this->image_format.image_channel_data_type = CL_UNORM_INT8;
	}
    virtual void TearDown()
    {
        FissionWrapper::TearDown();
    }
};

// Do not add other types here
typedef ::testing::Types<cl_uchar4> Fission_VR20_BGRA_CL_UNORM_INT8Types;
TYPED_TEST_CASE(Fission_VR20_BGRA_CL_UNORM_INT8, Fission_VR20_BGRA_CL_UNORM_INT8Types);

static const char* d2KernelName = "read_write_image2D_float4";
static const char* d3KernelName = "read_write_image3D_float4";

TYPED_TEST(Fission_VR20_BGRA_CL_UNORM_INT8, DISABLED_Image2DReadWriteUseHostPtrCPUGPU_bug)
{
	ASSERT_NO_FATAL_FAILURE(test2DReadWriteCommands<TypeParam>(this->ocl_descriptor, this->image_format, d2KernelName));
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(this->ocl_descriptor));	
}

TYPED_TEST(Fission_VR20_BGRA_CL_UNORM_INT8, Image2DReadWriteUseHostPtrCPUGPU)
{

	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(this->ocl_descriptor));	
	ASSERT_NO_FATAL_FAILURE(test2DReadWriteCommands<TypeParam>(this->ocl_descriptor, this->image_format, d2KernelName));
}

TYPED_TEST(Fission_VR20_BGRA_CL_UNORM_INT8, Image3DReadWriteUseHostPtrCPUGPU)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(this->ocl_descriptor));	
	ASSERT_NO_FATAL_FAILURE(test3DReadWriteCommands<TypeParam>(this->ocl_descriptor, this->image_format, d3KernelName));
}

TYPED_TEST(Fission_VR20_BGRA_CL_UNORM_INT8, Image2DReadWriteKernelCPUGPU)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(this->ocl_descriptor));	
	ASSERT_NO_FATAL_FAILURE(test2DReadWriteThroughKernel<TypeParam>(this->ocl_descriptor, this->image_format, d2KernelName));
}

TYPED_TEST(Fission_VR20_BGRA_CL_UNORM_INT8, Image3DReadWriteKernelCPUGPU)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(this->ocl_descriptor));	
	ASSERT_NO_FATAL_FAILURE(test3DReadWriteThroughKernel<TypeParam>(this->ocl_descriptor, this->image_format, d3KernelName));
}