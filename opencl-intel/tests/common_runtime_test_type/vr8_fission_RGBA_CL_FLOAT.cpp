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

//	FOR MORE INFORMATION ON THE FOLLOWING TESTS PLEASE REFER TO "vr8_image.h" file

//	Fission_VR8_RGBA_CL_FLOAT - tests for images with settings RGBA and CL_FLOAT
template <typename T>
class Fission_VR8_RGBA_CL_FLOAT : public ImageTypedCommonRuntime<T>, public FissionWrapper{
public:

	virtual void SetUp() 
	{
		FissionWrapper::SetUp();
		image_format.image_channel_order = CL_RGBA;
		image_format.image_channel_data_type = CL_FLOAT;
	}
};
// Do not add other types here
typedef ::testing::Types<cl_float4> Fission_VR8_RGBA_CL_FLOATTypes;
TYPED_TEST_CASE(Fission_VR8_RGBA_CL_FLOAT, Fission_VR8_RGBA_CL_FLOATTypes);

static const char* d2KernelName = "read_image2D_float4";
static const char* d3KernelName = "read_image3D_float4";

//|	TEST: ImageRGBA_CL_FLOAT.Image2DUseHostPtr 
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL 2d image is visible to all devices in the context.
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem 2d image on a shared context (shared for CPU and GPU) with CL_MEM_USE_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which sums all elements of input buffer into a float
//|	3. Run kernels on CPU and GPU
//|	4. Validate that all sum of all elements was obtained
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all image elements
//|
TYPED_TEST(Fission_VR8_RGBA_CL_FLOAT, Image2DUseHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(test2DUseHostPtr<TypeParam>(ocl_descriptor, image_format, d2KernelName));
}

//|	TEST: ImageRGBA_CL_FLOAT.Image2AllocHostPtr
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL 2d image is visible to all devices in the context.
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem 2d image on a shared context (shared for CPU and GPU) with CL_MEM_ALLOC_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which sums all elements of input buffer into a float
//|	3. Run kernels on CPU and GPU
//|	4. Validate that all sum of all elements was obtained
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all image elements
//|
TYPED_TEST(Fission_VR8_RGBA_CL_FLOAT, Image2AllocHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(test2DAllocHostPtr<TypeParam>(ocl_descriptor, image_format, d2KernelName));
}

//|	TEST: ImageRGBA_CL_FLOAT.Image2CopyHostPtr
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL 2d image is visible to all devices in the context.
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem 2d image on a shared context (shared for CPU and GPU) with CL_MEM_COPY_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which sums all elements of input buffer into a float
//|	3. Run kernels on CPU and GPU
//|	4. Validate that all sum of all elements was obtained
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all image elements
//|
TYPED_TEST(Fission_VR8_RGBA_CL_FLOAT, Image2CopyHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(test2DCopyHostPtr<TypeParam>(ocl_descriptor, image_format, d2KernelName));
}

//|	TEST: ImageTypedCommonRuntime.Image3DUseHostPtr 
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL 3d image is visible to all devices in the context.
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem 3d image on a shared context (shared for CPU and GPU) with CL_MEM_USE_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which sums all elements of input buffer into a float
//|	3. Run kernels on CPU and GPU
//|	4. Validate that all sum of all elements was obtained
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all image elements
//|
TYPED_TEST(Fission_VR8_RGBA_CL_FLOAT, Image3DUseHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(test3DUseHostPtr<TypeParam>(ocl_descriptor, image_format, d3KernelName));
}

//|	TEST: ImageRGBA_CL_FLOAT.Image3AllocHostPtr
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL 3d image is visible to all devices in the context.
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem 3d image on a shared context (shared for CPU and GPU) with CL_MEM_ALLOC_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which sums all elements of input buffer into a float
//|	3. Run kernels on CPU and GPU
//|	4. Validate that all sum of all elements was obtained
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all image elements
//|
TYPED_TEST(Fission_VR8_RGBA_CL_FLOAT, Image3DAllocHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(test3DAllocHostPtr<TypeParam>(ocl_descriptor, image_format,  d3KernelName));
}

//|	TEST: ImageRGBA_CL_FLOAT.Image3CopyHostPtr
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL 3d image is visible to all devices in the context.
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem 3d image on a shared context (shared for CPU and GPU) with CL_MEM_COPY_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which sums all elements of input buffer into a float
//|	3. Run kernels on CPU and GPU
//|	4. Validate that all sum of all elements was obtained
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all image elements
//|
TYPED_TEST(Fission_VR8_RGBA_CL_FLOAT, Image3DCopyHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(test3DCopyHostPtr<TypeParam>(ocl_descriptor, image_format,  d3KernelName));
}
