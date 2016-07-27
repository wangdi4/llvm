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

class OCL21: public CommonRuntime{};

//|	TEST: OCL21.clCreateProgramWithIL01
//|
//|	Purpose
//|	-------
//|	
//| Verify the ability to create program with il for shared context 
//| Verify the ability to get IL from program
//|	
//|	Method
//|	------
//|
//|	1. Create a program with IL for shared context
//|	2. Build program
//| 3. Get IL from program
//|
//|	Pass criteria
//|	-------------
//|
//|	Verify that valid non-zero program object are returned and build successfull
//|

TEST_F(OCL21, clCreateProgramWithIL01)
{
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromILSource(ocl_descriptor, "kernels/vector4_d.spir12_64"));

    const char * kernelSource = NULL;
	ASSERT_NO_FATAL_FAILURE(fileToBuffer(&kernelSource, "kernels/vector4_d.spir12_64"));

    void * il = NULL;
    size_t ret;

    getProgramInfo(ocl_descriptor.program, CL_PROGRAM_IL, sizeof(void *), &il, &ret);
    ASSERT_EQ(sizeof(void *), ret);
    ASSERT_EQ(sizeof(kernelSource), sizeof(il));

    if (kernelSource != NULL)
    {
        delete[] kernelSource;
        kernelSource = NULL;
    }
}

//|	TEST: OCL21.clEnqeueuSVMMigrateMem01
//|
//|	Purpose
//|	-------
//|	
//| Verify the ability to migrate SVM mem to the same device more than once
//|	
//|	Method
//|	------
//|
//|	1. Create a shared context
//|	2. Allocate SVM memory
//| 3. Migrate SVM memory to the same device more than once
//|
//|	Pass criteria
//|	-------------
//|
//|	Verify that CL_SUCCESS return codes are returned
//|
TEST_F(OCL21, clEnqueueSVMMigrateMem01)
{
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

    size_t size[2] = { 1024, 1024 };
    void * svmp[2];

    svmp[0] = clSVMAlloc(ocl_descriptor.context, NULL, size[0], 0);
    svmp[1] = clSVMAlloc(ocl_descriptor.context, NULL, size[1], 0);

    ASSERT_FALSE(svmp[0] == NULL) << "clSVMAlloc failed";
    ASSERT_FALSE(svmp[1] == NULL) << "clSVMAlloc failed";

    enqueueSVMMigrateMem(ocl_descriptor.queues[1], 2, (const void **)svmp, (const size_t *)size, NULL, 0, NULL, NULL);
    enqueueSVMMigrateMem(ocl_descriptor.queues[1], 2, (const void **)svmp, (const size_t *)size, NULL, 0, NULL, NULL);

    clSVMFree(ocl_descriptor.context, svmp[0]);
    clSVMFree(ocl_descriptor.context, svmp[1]);
}

//|	TEST: OCL21.clEnqeueuSVMMigrateMem02
//|
//|	Purpose
//|	-------
//|	
//| Verify the ability to migrate part of SVM allocation
//|	
//|	Method
//|	------
//|
//|	1. Create a shared context
//|	2. Allocate SVM memory
//| 3. Migrate different parts of SVM allocations to different devices
//|
//|	Pass criteria
//|	-------------
//|
//|	Verify that CL_SUCCESS return codes are returned
//|
TEST_F(OCL21, clEnqueueSVMMigrateMem02)
{
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

    const size_t size[2] = { 1024, 1024 };
    void * svmp[2];

    svmp[0] = clSVMAlloc(ocl_descriptor.context, NULL, size[0], 0);
    svmp[1] = clSVMAlloc(ocl_descriptor.context, NULL, size[1], 0);

    ASSERT_FALSE(svmp[0] == NULL) << "clSVMAlloc failed";
    ASSERT_FALSE(svmp[1] == NULL) << "clSVMAlloc failed";

    const size_t sizes[2] = { size[0] / 2, size[1] / 2 };
    const void ** ptrs[4] = {
        (const void **)svmp[0], (const void **)((size_t *)svmp[0] + sizes[0]),
        (const void **)svmp[1], (const void **)((size_t *)svmp[1] + sizes[1])
    };
    enqueueSVMMigrateMem(ocl_descriptor.queues[0], 1, ptrs[0], (const size_t *)&sizes[0], NULL, 0, NULL, NULL);
    enqueueSVMMigrateMem(ocl_descriptor.queues[1], 1, ptrs[1], (const size_t *)&sizes[0], NULL, 0, NULL, NULL);

    enqueueSVMMigrateMem(ocl_descriptor.queues[0], 1, ptrs[2], (const size_t *)&sizes[1], NULL, 0, NULL, NULL);
    enqueueSVMMigrateMem(ocl_descriptor.queues[1], 1, ptrs[3], (const size_t *)&sizes[1], NULL, 0, NULL, NULL);

    clSVMFree(ocl_descriptor.context, svmp[0]);
    clSVMFree(ocl_descriptor.context, svmp[1]);
}
