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
#include <iostream>
#include <sstream>
#include "vr8_buffer.h"

//|	VR8Buffer_Fission
//|
//| Runs tests in VR8Buffer_Fission for all OpenCL types defined in 
//|		typedef ::testing::Types<... TYPES DEFINED HERE .. > bufferTypes;
//|		TYPED_TEST_CASE(VR8Buffer_Fission, bufferTypes); (defined in common_runtime_tests.h)


//|	TEST: VR8Buffer_Fission.BufferTypedUseHostPtr (TC-115, TC-116, TC-117)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL buffer is visible to all devices in the context.

//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem buffer on a shared context (shared for CPU subdevice and GPU) with CL_MEM_USE_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which copies all elements of input buffer into output buffer
//|	3. Run kernels on CPU subdevice and GPU
//|	4. Validate that all output buffers contain elements of input buffer
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all buffer elements
//|
TYPED_TEST(VR8Buffer_Fission, BufferTypedUseHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(testBufferTypedUseHostPtr(ocl_descriptor));
}

//|	TEST: VR8Buffer_Fission.BufferTypedAllocHostPtr (TC-115, TC-116, TC-117)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL buffer is visible to all devices in the context
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem buffer on a shared context (shared for CPU subdevice and GPU) with CL_MEM_ALLOC_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which copies all elements of input buffer into output buffer
//|	3. Run kernels on CPU subdevice and GPU
//|	4. Validate that all output buffers contain elements of input buffer
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all buffer elements
//|
TYPED_TEST(VR8Buffer_Fission, BufferTypedAllocHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(testBufferTypedAllocHostPtr(ocl_descriptor));
}

//|	TEST: VR8Buffer_Fission.BufferTypedCopyHostPtr (TC-115, TC-116, TC-117)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL buffer is visible to all devices in the context
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem buffer on a shared context (shared for CPU subdevice and GPU) with CL_MEM_COPY_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which copies all elements of input buffer into output buffer
//|	3. Run kernels on CPU subdevice and GPU
//|	4. Validate that all output buffers contain elements of input buffer
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all buffer elements
//|
TYPED_TEST(VR8Buffer_Fission, BufferTypedCopyHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(testBufferTypedCopyHostPtr(ocl_descriptor));
}

//|	TEST: VR8Buffer_Fission.SubBufferUseHostPtr (TC-115, TC-116, TC-117)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL sub-buffer is visible to all devices in the context
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem buffer on a shared context (shared for CPU subdevice and GPU) with CL_MEM_USE_HOST_PTR
//|	2. Create shared sub-buffer of buffer in step 1 with CL_MEM_USE_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which copies all elements of input buffer into output buffer
//|	3. Run kernels on CPU subdevice and GPU
//|	4. Validate that all output buffers contain elements of input buffer
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all sub-buffer elements
//|
TYPED_TEST(VR8Buffer_Fission, SubBufferUseHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(testSubBufferUseHostPtr(ocl_descriptor));
}

//|	TEST: VR8Buffer_Fission.SubBufferAllocHostPtr (TC-115, TC-116, TC-117)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL sub-buffer is visible to all devices in the context
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem buffer on a shared context (shared for CPU subdevice and GPU) with CL_MEM_ALLOC_HOST_PTR
//|	2. Create shared sub-buffer of buffer in step 1 with CL_MEM_ALLOC_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which copies all elements of input buffer into output buffer
//|	3. Run kernels on CPU subdevice and GPU
//|	4. Validate that all output buffers contain elements of input buffer
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all sub-buffer elements
//|
TYPED_TEST(VR8Buffer_Fission, SubBufferAllocHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(testSubBufferAllocHostPtr(ocl_descriptor));
}

//|	TEST: VR8Buffer_Fission.SubBufferCopyHostPtr (TC-115, TC-116, TC-117)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that the content of an OpenCL sub-buffer is visible to all devices in the context
//|
//|	Method
//|	------
//|
//|	1. Create shared input cl_mem buffer on a shared context (shared for CPU subdevice and GPU) with CL_MEM_COPY_HOST_PTR
//|	2. Create shared sub-buffer of buffer in step 1 with CL_MEM_COPY_HOST_PTR
//|	2. For each device create separate output buffer
//|	2. Create for each device a kernel which copies all elements of input buffer into output buffer
//|	3. Run kernels on CPU subdevice and GPU
//|	4. Validate that all output buffers contain elements of input buffer
//|	
//|	Pass criteria
//|	-------------
//|
//|	Validate that each device is able to read all sub-buffer elements
//|
TYPED_TEST(VR8Buffer_Fission, SubBufferCopyHostPtr)
{
	ASSERT_NO_FATAL_FAILURE(createAndMergeWithGPU(ocl_descriptor));
	ASSERT_NO_FATAL_FAILURE(testSubBufferCopyHostPtr(ocl_descriptor));
}





