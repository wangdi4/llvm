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
#include <cstdio>
#include <cstring>
#include "common_runtime_tests.h"
#include "ocl21.h"

class OCL21: public CommonRuntime{};

//|	TEST: OCL21.clCreateProgramWithIL01
//|
//|	Purpose
//|	-------
//|
//| Verify the ability to create program with IL for shared context
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
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromILSource(ocl_descriptor, "subgroups.spv"));

    const char * kernelSource = nullptr;
	ASSERT_NO_FATAL_FAILURE(fileToBuffer(&kernelSource, "subgroups.spv"));

    void * il = nullptr;
    size_t ret = 0;

    getProgramInfo(ocl_descriptor.program, CL_PROGRAM_IL, sizeof(void *), &il, &ret);
    ASSERT_EQ(sizeof(void *), ret);

    if (kernelSource != nullptr)
    {
        delete[] kernelSource;
        kernelSource = nullptr;
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

    const size_t nsizes[2] = { 1024, 1024 };
    const size_t bsizes[2] = { nsizes[0] * sizeof(int), nsizes[1] * sizeof(int) };
    void * svmp[2] = { nullptr, nullptr };
    int * refp[2] = { nullptr, nullptr };

    refp[0] = new int[nsizes[0]];
    refp[1] = new int[nsizes[1]];

    ASSERT_FALSE(refp[0] == nullptr) << "new int[] failed";
    ASSERT_FALSE(refp[1] == nullptr) << "new int[] failed";

    svmp[0] = clSVMAlloc(ocl_descriptor.context, (cl_mem_flags)nullptr, bsizes[0], 0);
    svmp[1] = clSVMAlloc(ocl_descriptor.context, (cl_mem_flags)nullptr, bsizes[1], 0);

    ASSERT_FALSE(svmp[0] == nullptr) << "clSVMAlloc failed";
    ASSERT_FALSE(svmp[1] == nullptr) << "clSVMAlloc failed";

    fillMemory((int *)refp[0], nsizes[0], 0);
    fillMemory((int *)refp[1], nsizes[1], 0);

    fillMemory((int *)svmp[0], nsizes[0], 0);
    fillMemory((int *)svmp[1], nsizes[1], 0);

    enqueueSVMMigrateMem(ocl_descriptor.queues[1], 2,
        (const void **)svmp, (const size_t *)bsizes,
        (cl_mem_migration_flags)nullptr, 0, nullptr, nullptr);
    enqueueSVMMigrateMem(ocl_descriptor.queues[1], 2,
        (const void **)svmp, (const size_t *)bsizes,
        (cl_mem_migration_flags)nullptr, 0, nullptr, nullptr);

    ASSERT_TRUE(memcmp(refp[0], svmp[0], bsizes[0]) == 0) << "svmp[0] corrupted";
    ASSERT_TRUE(memcmp(refp[1], svmp[1], bsizes[1]) == 0) << "svmp[1] corrupted";

    delete[] refp[0];
    delete[] refp[1];

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

    const size_t nsizes[2] = { 1024, 1024 };
    const size_t bsizes[2] = { nsizes[0] * sizeof(int), nsizes[1] * sizeof(int) };
    const size_t hbsizes[2] = { bsizes[0] / 2, bsizes[1] / 2 };
    void * svmp[2] = { nullptr, nullptr };
    int * refp[2] = { nullptr, nullptr };

    refp[0] = new int[nsizes[0]];
    refp[1] = new int[nsizes[1]];

    ASSERT_FALSE(refp[0] == nullptr) << "new int[] failed";
    ASSERT_FALSE(refp[1] == nullptr) << "new int[] failed";

    svmp[0] = clSVMAlloc(ocl_descriptor.context, (cl_mem_flags)nullptr, bsizes[0], 0);
    svmp[1] = clSVMAlloc(ocl_descriptor.context, (cl_mem_flags)nullptr, bsizes[1], 0);

    ASSERT_FALSE(svmp[0] == nullptr) << "clSVMAlloc failed";
    ASSERT_FALSE(svmp[1] == nullptr) << "clSVMAlloc failed";

    fillMemory((int *)refp[0], nsizes[0], 0);
    fillMemory((int *)refp[1], nsizes[1], 0);

    fillMemory((int *)svmp[0], nsizes[0], 0);
    fillMemory((int *)svmp[1], nsizes[1], 0);

    printf("%p %u\n", svmp[0], hbsizes[0]);
    printf("%p %u\n", (char *)svmp[0] + hbsizes[0], hbsizes[0]);

    printf("%p %u\n", svmp[1], hbsizes[1]);
    printf("%p %u\n", (char *)svmp[1] + hbsizes[1], hbsizes[1]);

    enqueueSVMMigrateMem(ocl_descriptor.queues[0], 1,
        (const void **)svmp[0], (const size_t *)&hbsizes[0],
        (cl_mem_migration_flags)nullptr, 0, nullptr, nullptr); 
    enqueueSVMMigrateMem(ocl_descriptor.queues[1], 1,
        (const void **)((char *)svmp[0] + hbsizes[0]), (const size_t *)&hbsizes[0],
        (cl_mem_migration_flags)nullptr, 0, nullptr, nullptr);

    enqueueSVMMigrateMem(ocl_descriptor.queues[0], 1,
        (const void **)svmp[1], (const size_t *)&hbsizes[1],
        (cl_mem_migration_flags)nullptr, 0, nullptr, nullptr);
    enqueueSVMMigrateMem(ocl_descriptor.queues[1], 1,
        (const void **)((char *)svmp[1] + hbsizes[1]), (const size_t *)&hbsizes[1],
        (cl_mem_migration_flags)nullptr, 0, nullptr, nullptr);

    ASSERT_TRUE(memcmp(refp[0], svmp[0], bsizes[0]) == 0) << "svmp[0] corrupted";
    ASSERT_TRUE(memcmp(refp[1], svmp[1], bsizes[1]) == 0) << "svmp[1] corrupted";

    delete[] refp[0];
    delete[] refp[1];

    clSVMFree(ocl_descriptor.context, svmp[0]);
    clSVMFree(ocl_descriptor.context, svmp[1]);
}

//|	TEST: OCL21.clCloneKernel01
//|
//|	Purpose
//|	-------
//|
//|	Verify the ability to clone kernel objects for all kernel functions in a shared program
//|
//|	Method
//|	------
//|
//|	1. Build program with source of 10 kernels on both CPU and GPU
//|	2. Create 10 kernels for that program (will create for both CPU and GPU)
//| 3. Clone all kernels
//|
//|	Pass criteria
//|	-------------
//|
//|	Verify that valid non-zero kernel objects are returned
//|
TEST_F(OCL21, clCloneKernel01)
{
	// create OpenCL queues, program and context
	ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

	cl_kernel kernels[10];
    cl_kernel copied[10];
	for(int i=0; i<10; ++i)
	{
		std::stringstream ss;
		ss << "kernel_" << i;
		// create kernels
		kernels[i] = 0;
		ASSERT_NO_FATAL_FAILURE(createKernel(&kernels[i], ocl_descriptor.program, ss.str().c_str()));
	}

    for(int i=0; i<10; ++i)
    {
        ASSERT_NO_FATAL_FAILURE(cloneKernel(&copied[i], kernels[i]));
    }

	for(int i=0; i<10; ++i)
	{
		clReleaseKernel(kernels[i]);
        clReleaseKernel(copied[i]);
	}
}

//|	TEST: OCL21.clGetKernelSubGroupInfo01
//|
//|	Purpose
//|	-------
//|
//|	Verify the ability to get kernel subgroup info
//|
//|	Method
//|	------
//|
//|	1. Build program from IL
//|	2. Validate
//|
//|	Pass criteria
//|	-------------
//|
//|	Verify that valid non-zero kernel objects are returned
//|
TEST_F(OCL21, clGetKernelSubGroupInfo01)
{
    // create OpenCL queues, program and context
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromILSource(ocl_descriptor, "subgroups.spv"));

    cl_kernel kernel = 0;
    createKernel(&kernel, ocl_descriptor.program, "sub_groups_main");

    size_t cl_kernel_max_num_sub_groups = 0;
    size_t ret_size = 0;
    getKernelSubGroupInfo(kernel, ocl_descriptor.devices[0], CL_KERNEL_MAX_NUM_SUB_GROUPS,
        0, nullptr, sizeof(cl_kernel_max_num_sub_groups), &cl_kernel_max_num_sub_groups, &ret_size);
    ASSERT_EQ(sizeof(cl_kernel_max_num_sub_groups), ret_size);

    size_t local_size[3] = { 10, 10, 10 };
    ret_size = 0;
    getKernelSubGroupInfo(kernel, ocl_descriptor.devices[0], CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
        sizeof(cl_kernel_max_num_sub_groups), &cl_kernel_max_num_sub_groups,
        sizeof(local_size), local_size, &ret_size);
    ASSERT_GT(local_size[0], 0);
    ASSERT_EQ(local_size[1], 1);
    ASSERT_EQ(local_size[2], 1);

    local_size[0] = local_size[1] = local_size[2] = 10;
    ret_size = 0;
    size_t wrong_cl_kernel_max_num_sub_groups = cl_kernel_max_num_sub_groups + 10;
    getKernelSubGroupInfo(kernel, ocl_descriptor.devices[0], CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
        sizeof(wrong_cl_kernel_max_num_sub_groups), &wrong_cl_kernel_max_num_sub_groups,
        sizeof(local_size), local_size, &ret_size);
    ASSERT_EQ(local_size[0], 0);
    ASSERT_EQ(local_size[1], 0);
    ASSERT_EQ(local_size[2], 0);
}

//|	TEST: OCL21.clGetKernelSubGroupInfo02
//|
//|	Purpose
//|	-------
//|
//|	Verify the ability to get kernel subgroup info
//|
//|	Method
//|	------
//|
//|	1. Build program from IL
//|	2. Validate
//|
//|	Pass criteria
//|	-------------
//|
//|	Verify that valid non-zero kernel objects are returned
//|
TEST_F(OCL21, clGetKernelSubGroupInfo02)
{
    // create OpenCL queues, program and context
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromILSource(ocl_descriptor, "subgroups.spv"));

    cl_kernel kernel = 0;
   createKernel(&kernel, ocl_descriptor.program, "sub_groups_main");

    size_t cl_kernel_max_num_sub_groups = 0;
    size_t ret_size = 0;
    getKernelSubGroupInfo(kernel, ocl_descriptor.devices[0], CL_KERNEL_MAX_NUM_SUB_GROUPS,
        0, nullptr, sizeof(cl_kernel_max_num_sub_groups), &cl_kernel_max_num_sub_groups, &ret_size);
    ASSERT_EQ(sizeof(cl_kernel_max_num_sub_groups), ret_size);

    size_t local_size[3] = { 10, 10, 10 };
    ret_size = 0;
    getKernelSubGroupInfo(kernel, ocl_descriptor.devices[0], CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
        sizeof(cl_kernel_max_num_sub_groups), &cl_kernel_max_num_sub_groups,
        sizeof(local_size), local_size, &ret_size);
    ASSERT_GT(local_size[0], 0);
    ASSERT_EQ(local_size[1], 1);
    ASSERT_EQ(local_size[2], 1);

    local_size[0] = local_size[1] = local_size[2] = 10;
    ret_size = 0;
    size_t wrong_cl_kernel_max_num_sub_groups = cl_kernel_max_num_sub_groups + 10;
    getKernelSubGroupInfo(kernel, ocl_descriptor.devices[0], CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
        sizeof(wrong_cl_kernel_max_num_sub_groups), &wrong_cl_kernel_max_num_sub_groups,
        sizeof(local_size), local_size, &ret_size);
    ASSERT_EQ(local_size[0], 0);
    ASSERT_EQ(local_size[1], 0);
    ASSERT_EQ(local_size[2], 0);
}
