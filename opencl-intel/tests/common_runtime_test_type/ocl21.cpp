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
#include <iostream>
#include <vector>
#include <algorithm>

#include <cstdio>
#include <cstring>

#include "common_runtime_tests.h"
#include "ocl21.h"

class OCL21: public CommonRuntime{};

//| TEST: OCL21.clCreateProgramWithIL01
//|
//| Purpose
//| -------
//|
//| Verify the ability to create program with IL for shared context
//| Verify the ability to get IL from program
//|
//| Method
//| ------
//|
//| 1. Create a program with IL for shared context
//| 2. Build program
//| 3. Get IL from program
//|
//| Pass criteria
//| -------------
//|
//| Verify that valid non-zero program object is returned and build successfull
//| Verify size of returned IL and magic number in it
//|

TEST_F(OCL21, clCreateProgramWithIL01)
{
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromILSourceFile(ocl_descriptor, "simple_kernels.spv"));

    const char * kernelSource = nullptr;
    size_t length = 0;
    ASSERT_NO_FATAL_FAILURE(fileToBuffer(&kernelSource, "simple_kernels.spv", &length));

    void * il = nullptr;
    size_t ret = 0;

    getProgramInfo(ocl_descriptor.program, CL_PROGRAM_IL, sizeof(void *), &il, &ret);

    ASSERT_EQ(sizeof(void *), ret) << "param_value_size_ret assertion failed";
    ASSERT_EQ(((char *)il)[0], 0x07230203) << "Magic number assertion failed";

    if (kernelSource != nullptr)
    {
        delete[] kernelSource;
        kernelSource = nullptr;
    }
}

//| TEST: OCL21.clEnqeueuSVMMigrateMem01
//|
//| Purpose
//| -------
//|
//| Verify the ability to migrate SVM mem to the same device more than once
//|
//| Method
//| ------
//|
//| 1. Create a shared context
//| 2. Allocate SVM memory
//| 3. Migrate SVM memory to the same device more than once
//|
//| Pass criteria
//| -------------
//|
//| Verify that CL_SUCCESS return codes are returned
//|
TEST_F(OCL21, clEnqueueSVMMigrateMem01)
{
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

    const size_t nsizes[2] = { 1024, 1024 };
    const size_t bsizes[2] = { nsizes[0] * sizeof(int), nsizes[1] * sizeof(int) };
    void * svmp[2] = { nullptr, nullptr };
    std::vector<int> refp[2] = { std::vector<int>(nsizes[0]), std::vector<int>(nsizes[1]) };
    const size_t repeat = 5;

    svmp[0] = clSVMAlloc(ocl_descriptor.context, (cl_mem_flags)0, bsizes[0], 0);
    svmp[1] = clSVMAlloc(ocl_descriptor.context, (cl_mem_flags)0, bsizes[1], 0);

    ASSERT_FALSE(svmp[0] == nullptr) << "clSVMAlloc failed";
    ASSERT_FALSE(svmp[1] == nullptr) << "clSVMAlloc failed";

    std::generate(refp[0].begin(), refp[0].end(), IncrementingSequence<int>());
    std::generate(refp[1].begin(), refp[1].end(), IncrementingSequence<int>());

    std::generate((int *)svmp[0], (int *)svmp[0] + nsizes[0], IncrementingSequence<int>());
    std::generate((int *)svmp[1], (int *)svmp[1] + nsizes[1], IncrementingSequence<int>());

    for (size_t i = 0; i < repeat; ++i)
    {
        std::cout << "Starting migrate both pointers to CPU" << std::endl;
        enqueueSVMMigrateMem(ocl_descriptor.queues[0], 2,
            (const void **)svmp, (const size_t *)bsizes,
            (cl_mem_migration_flags)0, 0, nullptr, nullptr);

        ASSERT_TRUE(memcmp(&refp[0].front(), svmp[0], bsizes[0]) == 0) << "svmp[0] corrupted after " << i << "-th iteration";
        ASSERT_TRUE(memcmp(&refp[1].front(), svmp[1], bsizes[1]) == 0) << "svmp[1] corrupted after " << i << "-th iteration";
    }

    for (size_t i = 0; i < repeat; ++i)
    {
        std::cout << "Starting migrate both pointers to GPU" << std::endl;
        enqueueSVMMigrateMem(ocl_descriptor.queues[1], 2,
            (const void **)svmp, (const size_t *)bsizes,
            (cl_mem_migration_flags)0, 0, nullptr, nullptr);

        ASSERT_TRUE(memcmp(&refp[0].front(), svmp[0], bsizes[0]) == 0) << "svmp[0] corrupted after " << i << "-th iteration";
        ASSERT_TRUE(memcmp(&refp[1].front(), svmp[1], bsizes[1]) == 0) << "svmp[1] corrupted after " << i << "-th iteration";
    }

    clSVMFree(ocl_descriptor.context, svmp[0]);
    clSVMFree(ocl_descriptor.context, svmp[1]);
}

//| TEST: OCL21.clEnqeueuSVMMigrateMem02
//|
//| Purpose
//| -------
//|
//| Verify the ability to migrate part of SVM allocation
//|
//| Method
//| ------
//|
//| 1. Create a shared context
//| 2. Allocate SVM memory
//| 3. Migrate different parts of SVM allocations to different devices
//|
//| Pass criteria
//| -------------
//|
//| Verify that CL_SUCCESS return codes are returned
//|
TEST_F(OCL21, clEnqueueSVMMigrateMem02)
{
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

    const size_t nsizes[2] = { 1024, 1024 };
    const size_t bsizes[2] = { nsizes[0] * sizeof(int), nsizes[1] * sizeof(int) };
    const size_t hbsizes[2] = { bsizes[0] / 2, bsizes[1] / 2 };
    void * svmp[2] = { nullptr, nullptr };
    std::vector<int> refp[2] = { std::vector<int>(nsizes[0]), std::vector<int>(nsizes[1]) };
    void * second_half = nullptr;

    svmp[0] = clSVMAlloc(ocl_descriptor.context, (cl_mem_flags)0, bsizes[0], 0);
    svmp[1] = clSVMAlloc(ocl_descriptor.context, (cl_mem_flags)0, bsizes[1], 0);

    ASSERT_FALSE(svmp[0] == nullptr) << "clSVMAlloc failed";
    ASSERT_FALSE(svmp[1] == nullptr) << "clSVMAlloc failed";

    std::generate(refp[0].begin(), refp[0].end(), IncrementingSequence<int>());
    std::generate(refp[1].begin(), refp[1].end(), IncrementingSequence<int>());

    std::generate((int *)svmp[0], (int *)svmp[0] + nsizes[0], IncrementingSequence<int>());
    std::generate((int *)svmp[1], (int *)svmp[1] + nsizes[1], IncrementingSequence<int>());

    std::cout << "Starting migrate first half of svmp[0] to CPU..." << std::endl;
    enqueueSVMMigrateMem(ocl_descriptor.queues[0], 1,
        (const void **)&(svmp[0]), (const size_t *)&(hbsizes[0]),
        (cl_mem_migration_flags)0, 0, nullptr, nullptr);
    std::cout << "Starting migrate second half of svmp[0] to GPU..." << std::endl;
    second_half = ((char *)svmp[0]) + hbsizes[0];
    enqueueSVMMigrateMem(ocl_descriptor.queues[1], 1,
        (const void **)&second_half, (const size_t *)&(hbsizes[0]),
        (cl_mem_migration_flags)0, 0, nullptr, nullptr);

    std::cout << "Starting migrate first half of svmp[1] to GPU..." << std::endl;
    enqueueSVMMigrateMem(ocl_descriptor.queues[1], 1,
        (const void **)&(svmp[1]), (const size_t *)&(hbsizes[1]),
        (cl_mem_migration_flags)0, 0, nullptr, nullptr);
    std::cout << "Starting migrate second half of svmp[1] to CPU..." << std::endl;
    second_half = ((char *)svmp[1]) + hbsizes[1];
    enqueueSVMMigrateMem(ocl_descriptor.queues[0], 1,
        (const void **)&second_half, (const size_t *)&(hbsizes[1]),
        (cl_mem_migration_flags)0, 0, nullptr, nullptr);

    ASSERT_TRUE(memcmp(&refp[0].front(), svmp[0], bsizes[0]) == 0) << "svmp[0] corrupted";
    ASSERT_TRUE(memcmp(&refp[1].front(), svmp[1], bsizes[1]) == 0) << "svmp[1] corrupted";

    clSVMFree(ocl_descriptor.context, svmp[0]);
    clSVMFree(ocl_descriptor.context, svmp[1]);
}

//| TEST: OCL21.clCloneKernel01
//|
//| Purpose
//| -------
//|
//| Verify the ability to clone kernel objects for all kernel functions in a shared program
//|
//| Method
//| ------
//|
//| 1. Build program with source of 10 kernels on both CPU and GPU
//| 2. Create 10 kernels for that program (will create for both CPU and GPU)
//| 3. Clone all kernels
//|
//| Pass criteria
//| -------------
//|
//| Verify that valid non-zero kernel objects are returned
//|
TEST_F(OCL21, clCloneKernel01)
{
    // create OpenCL queues, program and context
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "simple_kernels.cl"));

    const size_t count = 10;

    cl_kernel kernels[count] = { (cl_kernel)nullptr };
    cl_kernel copied[count] = { (cl_kernel)nullptr };

    for (size_t i = 0; i < count; ++i)
    {
        std::stringstream ss;
        ss << "kernel_" << i;
        // create kernels
        ASSERT_NO_FATAL_FAILURE(createKernel(&kernels[i], ocl_descriptor.program, ss.str().c_str()));
    }

    for (size_t i = 0; i < count; ++i)
    {
        ASSERT_NO_FATAL_FAILURE(cloneKernel(&copied[i], kernels[i]));
    }

    for (size_t i = 0; i < count; ++i)
    {
        clReleaseKernel(kernels[i]);
        clReleaseKernel(copied[i]);
    }
}

//| TEST: OCL21.clCloneKernel02
//|
//| Purpose
//| -------
//|
//| Verify the ability to clone kernel object in a shared program
//|
//| Method
//| ------
//|
//| 1. Build program both CPU and GPU
//| 2. Create kernel for that program (will create for both CPU and GPU)
//| 3. Clone kernel
//|
//| Pass criteria
//| -------------
//|
//| Verify that valid non-zero kernel objects are returned
//| Verify that result of execution of cloned kernel is the same as result of
//| execution of original kernel
//| Verify that info about kernel successfully copied (num_args, context, ...)
//|
TEST_F(OCL21, clCloneKernel02)
{
    // create OpenCL queues, program and context
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueues(ocl_descriptor, "copy_kernels.cl"));

    // some staff for kernel execution
    const cl_int size = 1024;
    std::vector<cl_int> data(size);
    std::vector<cl_int> result(data);

    std::generate(data.begin(), data.end(), IncrementingSequence<cl_int>());

    size_t global_work_size[3] = { 1024, 1, 1 };
    size_t local_work_size[3] = { 32, 1, 1 };
    cl_int zero = 0;

    for (size_t index = 0; index < 2; ++index)
    {
        std::cout << "Testing " << index << "-th device..." << std::endl;
        cl_mem input_buffer = nullptr;
        cl_mem output_buffer = nullptr;

        createBuffer(&input_buffer, ocl_descriptor.context, CL_MEM_READ_ONLY, size * sizeof(cl_int), nullptr);
        createBuffer(&output_buffer, ocl_descriptor.context, CL_MEM_WRITE_ONLY, size * sizeof(cl_int), nullptr);

        // some staff to check copy of the kernel
        cl_uint original_reference_count = 0;
        cl_uint copied_reference_count = 0;

        cl_uint original_num_args = 0;
        cl_uint copied_num_args = 0;

        cl_context copied_context = 0;
        cl_program copied_program = 0;

        cl_kernel kernel = (cl_kernel)nullptr;
        cl_kernel copied = (cl_kernel)nullptr;

        // create original kernel
        ASSERT_NO_FATAL_FAILURE(createKernel(&kernel, ocl_descriptor.program, "copy_int"));

        setKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
        setKernelArg(kernel, 1, sizeof(cl_mem), &output_buffer);

        // save info about original kernel
        getKernelInfo(kernel, CL_KERNEL_REFERENCE_COUNT, sizeof(cl_uint),
            &original_reference_count, nullptr);
        getKernelInfo(kernel, CL_KERNEL_NUM_ARGS, sizeof(cl_uint),
            &original_num_args, nullptr);

        // execute original kernel and check results
        enqueueWriteBuffer(ocl_descriptor.queues[index], input_buffer, CL_TRUE, 0, size * sizeof(cl_int),
            &data.front(), 0, nullptr, nullptr);
        enqueueFillBuffer(ocl_descriptor.queues[index], output_buffer, &zero,
            sizeof(cl_int), 0, result.size() * sizeof(cl_int), 0, nullptr, nullptr);

        enqueueNDRangeKernel(ocl_descriptor.queues[index], kernel, 3, nullptr,
            global_work_size, local_work_size, 0, nullptr, nullptr);

        enqueueReadBuffer(ocl_descriptor.queues[index], output_buffer, CL_TRUE, 0, size * sizeof(cl_int),
            &result.front(), 0, nullptr, nullptr);

        for (size_t i = 0; i < size; ++i)
        {
            ASSERT_EQ(data[i], result[i]) << i << "-th element of result buffer is wrong";
        }

        result.assign(size, 0);

        // copy original kernel
        ASSERT_NO_FATAL_FAILURE(cloneKernel(&copied, kernel));
        std::cout << "Starting test cloned kernel" << std::endl;

        // check info about copied kernel
        getKernelInfo(copied, CL_KERNEL_REFERENCE_COUNT, sizeof(cl_uint),
            &copied_reference_count, nullptr);
        ASSERT_EQ(original_reference_count, copied_reference_count);
        getKernelInfo(copied, CL_KERNEL_NUM_ARGS, sizeof(cl_uint),
            &copied_num_args, nullptr);
        ASSERT_EQ(original_num_args, copied_num_args);

        enqueueFillBuffer(ocl_descriptor.queues[index], output_buffer, &zero,
            sizeof(cl_int), 0, result.size() * sizeof(cl_int), 0, nullptr, nullptr);

        // execute copied kernel and check results
        enqueueNDRangeKernel(ocl_descriptor.queues[index], copied, 3, nullptr,
            global_work_size, local_work_size, 0, nullptr, nullptr);

        enqueueReadBuffer(ocl_descriptor.queues[index], output_buffer, CL_TRUE, 0, size * sizeof(cl_int),
            &result.front(), 0, nullptr, nullptr);

        for (size_t i = 0; i < size; ++i)
        {
            ASSERT_EQ(data[i], result[i]) << i << "-th element of result buffer is wrong";
        }

        finish(ocl_descriptor.queues[index]);

        // testing done, release resources
        clReleaseMemObject(input_buffer);
        clReleaseMemObject(output_buffer);

        clReleaseKernel(kernel);
        clReleaseKernel(copied);
    }
}

//| TEST: OCL21.clGetKernelSubGroupInfo01
//|
//| Purpose
//| -------
//|
//| Verify that clGetKernelSubGroupInfo cannot generate local size for more
//| than CL_KERNEL_MAX_NUM_SUB_GROUPS subgroups
//|
//| Method
//| ------
//|
//| 1. Build program from IL
//| 2. Get count=CL_KERNEL_MAX_NUM_SUB_GROUPS using clGetKernelSubGroupInfo
//| 3. Generate local_size for `count` subgroups
//| 4. Validate that local_size[0] > 0, local_size[1] = 1 and local_size[2] = 1
//| 5. Generate local_size for `count` + 10 subgroups
//| 6. Validate that local_size[0] = 0, local_size[1] = 0 and local_size[2] = 0
//|
//| Pass criteria
//| -------------
//|
//| All assertions passed
//|
TEST_F(OCL21, DISABLED_clGetKernelSubGroupInfo01)
{
    // create OpenCL queues, program and context
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromILSourceFile(ocl_descriptor, "subgroups.spv"));

    cl_kernel kernel = 0;
    createKernel(&kernel, ocl_descriptor.program, "sub_groups_main");

    for (size_t index = 0; index < 2; ++index)
    {
        std::cout << "Testing " << index << "-th device..." << std::endl;
        size_t cl_kernel_max_num_sub_groups = 0;
        size_t ret_size = 0;
        getKernelSubGroupInfo(kernel, ocl_descriptor.devices[index], CL_KERNEL_MAX_NUM_SUB_GROUPS,
            0, nullptr, sizeof(cl_kernel_max_num_sub_groups), &cl_kernel_max_num_sub_groups, &ret_size);
        ASSERT_EQ(sizeof(cl_kernel_max_num_sub_groups), ret_size);

        size_t local_size[3] = { 10, 10, 10 };
        ret_size = 0;
        getKernelSubGroupInfo(kernel, ocl_descriptor.devices[index], CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
            sizeof(cl_kernel_max_num_sub_groups), &cl_kernel_max_num_sub_groups,
            sizeof(local_size), local_size, &ret_size);
        ASSERT_GT(local_size[0], 0);
        ASSERT_EQ(local_size[1], 1);
        ASSERT_EQ(local_size[2], 1);

        local_size[0] = local_size[1] = local_size[2] = 10;
        ret_size = 0;
        size_t wrong_cl_kernel_max_num_sub_groups = cl_kernel_max_num_sub_groups + 10;
        getKernelSubGroupInfo(kernel, ocl_descriptor.devices[index], CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
            sizeof(wrong_cl_kernel_max_num_sub_groups), &wrong_cl_kernel_max_num_sub_groups,
            sizeof(local_size), local_size, &ret_size);
        ASSERT_EQ(local_size[0], 0);
        ASSERT_EQ(local_size[1], 0);
        ASSERT_EQ(local_size[2], 0);
    }
}

//| TEST: OCL21.clGetKernelSubGroupInfo02
//|
//| Purpose
//| -------
//|
//| Verify the ability to get kernel subgroup info
//|
//| Method
//| ------
//|
//| 1. Build program from IL
//| 2. Get count=CL_KERNEL_MAX_NUM_SUB_GROUPS
//| 3. Get local_size for `count` subgroups
//| 4. Get sub_group_count for local_size
//| 5. Validate that sub_group_count = count
//|
//| Pass criteria
//| -------------
//|
//| Verify that valid non-zero kernel objects are returned and all assertions passed
//|
TEST_F(OCL21, DISABLED_clGetKernelSubGroupInfo02)
{
    // create OpenCL queues, program and context
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromILSourceFile(ocl_descriptor, "subgroups.spv"));

    cl_kernel kernel = 0;
    createKernel(&kernel, ocl_descriptor.program, "sub_groups_main");

    for (size_t index = 0; index < 2; ++index)
    {
        std::cout << "Testing " << index << "-th device..." << std::endl;
        size_t cl_kernel_max_num_sub_groups = 0;
        size_t ret_size = 0;
        getKernelSubGroupInfo(kernel, ocl_descriptor.devices[index], CL_KERNEL_MAX_NUM_SUB_GROUPS,
            0, nullptr, sizeof(cl_kernel_max_num_sub_groups), &cl_kernel_max_num_sub_groups, &ret_size);
        ASSERT_EQ(sizeof(cl_kernel_max_num_sub_groups), ret_size);

        size_t local_size[3] = { 10, 10, 10 };
        ret_size = 0;
        getKernelSubGroupInfo(kernel, ocl_descriptor.devices[index], CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
            sizeof(cl_kernel_max_num_sub_groups), &cl_kernel_max_num_sub_groups,
            sizeof(local_size), local_size, &ret_size);
        ASSERT_GT(local_size[0], 0);
        ASSERT_EQ(local_size[1], 1);
        ASSERT_EQ(local_size[2], 1);

        ret_size = 0;
        size_t sub_group_count = 0;
        getKernelSubGroupInfo(kernel, ocl_descriptor.devices[index], CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE,
            sizeof(local_size), local_size,
            sizeof(sub_group_count), &sub_group_count, &ret_size);
        ASSERT_EQ(sub_group_count, cl_kernel_max_num_sub_groups);
    }
}

//| TEST: OCL21.clGetKernelSubGroupInfo03
//|
//| Purpose
//| -------
//|
//| Verify the ability to get kernel subgroup info
//|
//| Method
//| ------
//|
//| 1. Build program from IL
//| 2. Get size = CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE
//| 3. Get count = CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE
//| 4. Execute kernel
//| 5. Validate data from kernel
//|
//| Pass criteria
//| -------------
//|
//| Verify that valid non-zero kernel objects are returned
//| Verify that data from kernel (from functions get_sub_group_size,
//| get_max_sub_group_size, get_num_sub_groups) is valid and conform with
//| data from API calls
//|
TEST_F(OCL21, DISABLED_clGetKernelSubGroupInfo03)
{
    // create OpenCL queues, program and context
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromILSourceFile(ocl_descriptor, "subgroups.spv"));

    cl_kernel kernel = 0;
    createKernel(&kernel, ocl_descriptor.program, "sub_groups_main");

    for (size_t index = 0; index < 2; ++index)
    {
        std::cout << "Testing " << index << "-th device..." << std::endl;
        cl_mem sub_group_size_buffer = nullptr;
        cl_mem max_sub_group_size_buffer = nullptr;
        cl_mem num_sub_groups_buffer = nullptr;

        size_t cl_kernel_max_num_sub_groups = 0;
        size_t ret_size = 0;
        getKernelSubGroupInfo(kernel, ocl_descriptor.devices[index], CL_KERNEL_MAX_NUM_SUB_GROUPS,
            0, nullptr, sizeof(cl_kernel_max_num_sub_groups), &cl_kernel_max_num_sub_groups, &ret_size);
        ASSERT_EQ(sizeof(cl_kernel_max_num_sub_groups), ret_size);

        size_t local_size[3] = { 1, 1, 1 };
        ret_size = 0;
        getKernelSubGroupInfo(kernel, ocl_descriptor.devices[index], CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
            sizeof(cl_kernel_max_num_sub_groups), &cl_kernel_max_num_sub_groups,
            sizeof(local_size), local_size, &ret_size);
        ASSERT_GT(local_size[0], 0);
        ASSERT_EQ(local_size[1], 1);
        ASSERT_EQ(local_size[2], 1);

        size_t cl_kernel_max_sub_group_size_for_ndrange = 0;
        ret_size = 0;
        getKernelSubGroupInfo(kernel, ocl_descriptor.devices[index], CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
            sizeof(local_size), local_size, sizeof(cl_kernel_max_sub_group_size_for_ndrange),
            &cl_kernel_max_sub_group_size_for_ndrange, &ret_size);

        size_t global_size[3] = { local_size[0], local_size[1], local_size[2] };
        size_t buffer_size = global_size[0] * sizeof(cl_uint);

        createBuffer(&sub_group_size_buffer, ocl_descriptor.context, CL_MEM_WRITE_ONLY, buffer_size, nullptr);
        createBuffer(&max_sub_group_size_buffer, ocl_descriptor.context, CL_MEM_WRITE_ONLY, buffer_size, nullptr);
        createBuffer(&num_sub_groups_buffer, ocl_descriptor.context, CL_MEM_WRITE_ONLY, buffer_size, nullptr);

        size_t pattern = 0;
        enqueueFillBuffer(ocl_descriptor.queues[index], sub_group_size_buffer, &pattern, sizeof(pattern), 0, buffer_size, 0, nullptr, nullptr);
        enqueueFillBuffer(ocl_descriptor.queues[index], max_sub_group_size_buffer, &pattern, sizeof(pattern), 0, buffer_size, 0, nullptr, nullptr);
        enqueueFillBuffer(ocl_descriptor.queues[index], num_sub_groups_buffer, &pattern, sizeof(pattern), 0, buffer_size, 0, nullptr, nullptr);

        setKernelArg(kernel, 0, buffer_size, sub_group_size_buffer);
        setKernelArg(kernel, 1, buffer_size, max_sub_group_size_buffer);
        setKernelArg(kernel, 2, buffer_size, num_sub_groups_buffer);

        enqueueNDRangeKernel(ocl_descriptor.queues[index], kernel, 3, nullptr, global_size, local_size, 0, nullptr, nullptr);

        cl_uint *ptr = nullptr;
        enqueueMapBuffer(&ptr, ocl_descriptor.queues[index], sub_group_size_buffer, CL_TRUE, CL_MAP_READ, 0, buffer_size, 0, nullptr, nullptr);
        cl_uint sub_group_size = ptr[0];
        for (size_t i = 0; i < global_size[0]; ++i) {
            ASSERT_GE(cl_kernel_max_sub_group_size_for_ndrange, ptr[i]);
            ASSERT_EQ(sub_group_size, ptr[i]);
        }
        enqueueUnmapMemObject(ocl_descriptor.queues[index], sub_group_size_buffer, ptr, 0, nullptr, nullptr);

        ptr = nullptr;
        enqueueMapBuffer(&ptr, ocl_descriptor.queues[index], max_sub_group_size_buffer, CL_TRUE, CL_MAP_READ, 0, buffer_size, 0, nullptr, nullptr);
        for (size_t i = 0; i < global_size[0]; ++i) {
            ASSERT_EQ(cl_kernel_max_sub_group_size_for_ndrange, ptr[i]);
        }
        enqueueUnmapMemObject(ocl_descriptor.queues[index], max_sub_group_size_buffer, ptr, 0, nullptr, nullptr);

        ptr = nullptr;
        enqueueMapBuffer(&ptr, ocl_descriptor.queues[index], num_sub_groups_buffer, CL_TRUE, CL_MAP_READ, 0, buffer_size, 0, nullptr, nullptr);
        for (size_t i = 0; i < global_size[0]; ++i) {
            ASSERT_EQ(ptr[i], cl_kernel_max_num_sub_groups);
        }
        enqueueUnmapMemObject(ocl_descriptor.queues[index], num_sub_groups_buffer, ptr, 0, nullptr, nullptr);

        finish(ocl_descriptor.queues[index]);

        clReleaseKernel(kernel);
        clReleaseMemObject(sub_group_size_buffer);
        clReleaseMemObject(max_sub_group_size_buffer);
        clReleaseMemObject(num_sub_groups_buffer);
    }
}
