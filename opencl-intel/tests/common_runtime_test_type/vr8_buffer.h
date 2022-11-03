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

#ifndef VR8_BUFFER_GTEST_
#define VR8_BUFFER_GTEST_

template <typename T> class BufferTests {
protected:
  std::string getKernelSource(std::string arrayTypeName) {
    std::stringstream ss;
    if (0 == arrayTypeName.compare("UserDefinedStructure")) {
      ss << "typedef struct{    \n";
      ss << "    int x;    \n";
      ss << "    float y;    \n";
      ss << "    char z;    \n";
      ss << "} UserDefinedStructure;    \n";
      ss << "__kernel void read_UserDefinedStructure(__global "
            "UserDefinedStructure* input,  __global UserDefinedStructure* "
            "output, int input_size)    \n";
      ss << "{    \n";
      ss << "    for(int i=0; i<input_size; ++i){    \n";
      ss << "      output[i].x=input[i].x;      \n";
      ss << "      output[i].y=input[i].y;      \n";
      ss << "      output[i].z=input[i].z;    \n";
      ss << "    }    \n";
      ss << "}    \n";

      return ss.str();
    }
    ss << "__kernel void read_" << arrayTypeName << "(__global "
       << arrayTypeName << "* input, __global " << arrayTypeName
       << "* output, int input_size)\n";
    ss << "{\n";
    ss << "    for(int i=0; i<input_size; ++i){\n";
    if (0 == arrayTypeName.compare("half")) {
      ss << "    float val = vloada_half(i, input);\n";
      ss << "     vstorea_half(val, i, output);\n";
    } else {
      ss << "    output[i]=input[i];\n";
    }
    ss << "    }\n";
    ss << "}\n";
    return ss.str();
  }
  TypeNameGetter<T> typeNameGetter;
  T value_;

  //|  TypedCommonRuntime
  //|
  //| Runs tests in TypedCommonRuntime for all OpenCL types defined in
  //|    typedef ::testing::Types<... TYPES DEFINED HERE .. >
  // bufferTypes; |    TYPED_TEST_CASE(TypedCommonRuntime,
  // bufferTypes); (defined in common_runtime_tests.h)

  //|  TEST: testSubBufferCopyHostPtr
  //|
  //|  Purpose
  //|  -------
  //|
  //|  Verify that the content of an OpenCL buffer is visible to all devices in
  // the context.

  //|
  //|  Method
  //|  ------
  //|
  //|  1. Create shared input cl_mem buffer on a shared context (shared for CPU
  // and GPU) with CL_MEM_USE_HOST_PTR |  2. For each device create
  // separate output buffer |  2. Create for each device a kernel which copies
  // all elements of input buffer into output buffer |  3. Run kernels on CPU
  // and GPU |  4. Validate that all output buffers contain elements of input
  // buffer
  //|
  //|  Pass criteria
  //|  -------------
  //|
  //|  Validate that each device is able to read all buffer elements
  //|

  void testBufferTypedUseHostPtr(OpenCLDescriptor &ocl_descriptor) {
    // create and initialize input array
    int arraySize = (size_t)4;
    DynamicArray<T> input_array(arraySize);
    // kernel source
    std::string kernelSource =
        getKernelSource(typeNameGetter.getType()).c_str();
    // create OpenCL queues, program and context
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromStringSource(
        ocl_descriptor, kernelSource.c_str()));

    // create shared input buffer
    ASSERT_NO_FATAL_FAILURE(
        createBuffer(&ocl_descriptor.in_common_buffer, ocl_descriptor.context,
                     CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                     sizeof(T) * arraySize, input_array.dynamic_array));

    // set up manual destruction
    ASSERT_NO_FATAL_FAILURE(
        setDeleteArrayOnCallback(ocl_descriptor.in_common_buffer, input_array));

    // setup and execute kernels, read and validate results
    ASSERT_NO_FATAL_FAILURE(executeSetAndExecuteKernelsCopy(
        typeNameGetter.getCombinedString("read_").c_str(), input_array,
        ocl_descriptor.in_common_buffer, ocl_descriptor.buffers,
        ocl_descriptor.kernels, ocl_descriptor.queues, ocl_descriptor.context,
        ocl_descriptor.program, arraySize));
  }

  //|  TEST: testSubBufferCopyHostPtr
  //|
  //|  Purpose
  //|  -------
  //|
  //|  Verify that the content of an OpenCL buffer is visible to all devices in
  // the context
  //|
  //|  Method
  //|  ------
  //|
  //|  1. Create shared input cl_mem buffer on a shared context (shared for CPU
  // and GPU) with CL_MEM_ALLOC_HOST_PTR |  2. For each device create
  // separate output buffer |  2. Create for each device a kernel which copies
  // all elements of input buffer into output buffer |  3. Run kernels on CPU
  // and GPU |  4. Validate that all output buffers contain elements of input
  // buffer
  //|
  //|  Pass criteria
  //|  -------------
  //|
  //|  Validate that each device is able to read all buffer elements
  //|

  void testBufferTypedAllocHostPtr(OpenCLDescriptor &ocl_descriptor) {
    // set work dimensions
    cl_uint work_dim = 1;
    size_t global_work_size = 1;

    // create and initialize array
    int arraySize = (size_t)4;
    DynamicArray<T> input_array(arraySize);
    // kernel source
    std::string kernelSource =
        getKernelSource(typeNameGetter.getType()).c_str();
    // create OpenCL queues, program and context
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromStringSource(
        ocl_descriptor, kernelSource.c_str()));

    // create shared buffer
    ASSERT_NO_FATAL_FAILURE(
        createBuffer(&ocl_descriptor.in_common_buffer, ocl_descriptor.context,
                     CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                     arraySize * sizeof(T), NULL));

    // write to input buffers
    for (int i = 0; i < 2; ++i) {
      ASSERT_NO_FATAL_FAILURE(enqueueWriteBuffer(
          ocl_descriptor.queues[i], ocl_descriptor.in_common_buffer, CL_TRUE, 0,
          sizeof(T) * arraySize, input_array.dynamic_array, 0, NULL, NULL));
    }

    // setup and execute kernels, read and validate results
    ASSERT_NO_FATAL_FAILURE(executeSetAndExecuteKernelsCopy(
        typeNameGetter.getCombinedString("read_").c_str(), input_array,
        ocl_descriptor.in_common_buffer, ocl_descriptor.buffers,
        ocl_descriptor.kernels, ocl_descriptor.queues, ocl_descriptor.context,
        ocl_descriptor.program, arraySize));
  }

  //|  TEST: testSubBufferCopyHostPtr
  //|
  //|  Purpose
  //|  -------
  //|
  //|  Verify that the content of an OpenCL buffer is visible to all devices in
  // the context
  //|
  //|  Method
  //|  ------
  //|
  //|  1. Create shared input cl_mem buffer on a shared context (shared for CPU
  // and GPU) with CL_MEM_COPY_HOST_PTR
  //|  2. For each device create separate output buffer
  //|  2. Create for each device a kernel which copies all elements of input
  // buffer into output buffer |  3. Run kernels on CPU and GPU |  4.
  // Validate that all output buffers contain elements of input buffer
  //|
  //|  Pass criteria
  //|  -------------
  //|
  //|  Validate that each device is able to read all buffer elements
  //|

  void testBufferTypedCopyHostPtr(OpenCLDescriptor &ocl_descriptor) {
    // set work dimensions
    cl_uint work_dim = 1;
    size_t global_work_size = 1;

    // create and initialize array
    int arraySize = (size_t)4;
    DynamicArray<T> input_array(arraySize);
    // kernel source
    std::string kernelSource =
        getKernelSource(typeNameGetter.getType()).c_str();
    // create OpenCL queues, program and context
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromStringSource(
        ocl_descriptor, kernelSource.c_str()));

    // create shared buffer
    ASSERT_NO_FATAL_FAILURE(
        createBuffer(&ocl_descriptor.in_common_buffer, ocl_descriptor.context,
                     CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                     arraySize * sizeof(T), input_array.dynamic_array));

    // setup and execute kernels, read and validate results
    ASSERT_NO_FATAL_FAILURE(executeSetAndExecuteKernelsCopy(
        typeNameGetter.getCombinedString("read_").c_str(), input_array,
        ocl_descriptor.in_common_buffer, ocl_descriptor.buffers,
        ocl_descriptor.kernels, ocl_descriptor.queues, ocl_descriptor.context,
        ocl_descriptor.program, arraySize));
  }

  //|  TEST: testSubBufferCopyHostPtr
  //|
  //|  Purpose
  //|  -------
  //|
  //|  Verify that the content of an OpenCL sub-buffer is visible to all
  // devices in the context
  //|
  //|  Method
  //|  ------
  //|
  //|  1. Create shared input cl_mem buffer on a shared context (shared for CPU
  // and GPU) with CL_MEM_USE_HOST_PTR |  2. Create shared sub-buffer of
  // buffer in step 1 with CL_MEM_USE_HOST_PTR |  2. For each device
  // create separate output buffer |  2. Create for each device a kernel which
  // copies all elements of input buffer into output buffer |  3. Run kernels
  // on CPU and GPU |  4. Validate that all output buffers contain elements of
  // input buffer
  //|
  //|  Pass criteria
  //|  -------------
  //|
  //|  Validate that each device is able to read all sub-buffer elements
  //|

  void testSubBufferUseHostPtr(OpenCLDescriptor &ocl_descriptor) {
    // set work dimensions
    cl_uint work_dim = 1;
    size_t global_work_size = 1;

    // create and initialize array
    int arraySize = (size_t)4;
    DynamicArray<T> input_array(arraySize);
    // kernel source
    std::string kernelSource =
        getKernelSource(typeNameGetter.getType()).c_str();
    // create OpenCL queues, program and context
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromStringSource(
        ocl_descriptor, kernelSource.c_str()));

    cl_mem in_common_buffer = 0;
    cl_mem in_common_sub_buffer = 0;

    // create shared buffer
    ASSERT_NO_FATAL_FAILURE(
        createBuffer(&in_common_buffer, ocl_descriptor.context,
                     CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                     sizeof(T) * arraySize, input_array.dynamic_array));

    // create sub-buffer
    cl_buffer_region input_buffer_region = {0, arraySize * sizeof(T)};

    ASSERT_NO_FATAL_FAILURE(createSubBuffer(
        &in_common_sub_buffer, in_common_buffer, CL_MEM_READ_WRITE,
        CL_BUFFER_CREATE_TYPE_REGION, &input_buffer_region));

    // release of in_common_buffer is dependant on release of
    // in_common_sub_buffer
    ASSERT_NO_FATAL_FAILURE(
        setReleaseMemObjOnCallback(in_common_sub_buffer, &in_common_buffer));

    // set up manual destruction of input_array
    ASSERT_NO_FATAL_FAILURE(
        setDeleteArrayOnCallback(in_common_buffer, input_array));

    // setup and execute kernels, read and validate results
    ASSERT_NO_FATAL_FAILURE(executeSetAndExecuteKernelsCopy(
        typeNameGetter.getCombinedString("read_").c_str(), input_array,
        in_common_sub_buffer, ocl_descriptor.buffers, ocl_descriptor.kernels,
        ocl_descriptor.queues, ocl_descriptor.context, ocl_descriptor.program,
        arraySize));

    if (0 != in_common_sub_buffer) {
      EXPECT_EQ(CL_SUCCESS, clReleaseMemObject(in_common_sub_buffer))
          << "clReleaseMemObject failed";
      in_common_sub_buffer = 0;
    }
  }

  //|  TEST: testSubBufferCopyHostPtr
  //|
  //|  Purpose
  //|  -------
  //|
  //|  Verify that the content of an OpenCL sub-buffer is visible to all
  // devices in the context
  //|
  //|  Method
  //|  ------
  //|
  //|  1. Create shared input cl_mem buffer on a shared context (shared for CPU
  // and GPU) with CL_MEM_ALLOC_HOST_PTR |  2. Create shared sub-buffer of
  // buffer in step 1 with CL_MEM_ALLOC_HOST_PTR |  2. For each device
  // create separate output buffer |  2. Create for each device a kernel which
  // copies all elements of input buffer into output buffer |  3. Run kernels
  // on CPU and GPU |  4. Validate that all output buffers contain elements of
  // input buffer
  //|
  //|  Pass criteria
  //|  -------------
  //|
  //|  Validate that each device is able to read all sub-buffer elements
  //|

  void testSubBufferAllocHostPtr(OpenCLDescriptor &ocl_descriptor) {
    // set work dimensions
    cl_uint work_dim = 1;
    size_t global_work_size = 1;

    // create and initialize array
    int arraySize = (size_t)4;
    DynamicArray<T> input_array(arraySize);
    // kernel source
    std::string kernelSource =
        getKernelSource(typeNameGetter.getType()).c_str();
    // create OpenCL queues, program and context
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromStringSource(
        ocl_descriptor, kernelSource.c_str()));

    // create shared buffer
    ASSERT_NO_FATAL_FAILURE(
        createBuffer(&ocl_descriptor.in_common_buffer, ocl_descriptor.context,
                     CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                     arraySize * sizeof(T), NULL));

    // create sub-buffer
    cl_buffer_region input_buffer_region = {0, arraySize * sizeof(T)};

    ASSERT_NO_FATAL_FAILURE(createSubBuffer(
        &ocl_descriptor.in_common_sub_buffer, ocl_descriptor.in_common_buffer,
        CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &input_buffer_region));

    // write to input buffer
    ASSERT_NO_FATAL_FAILURE(enqueueWriteBuffer(
        ocl_descriptor.queues[0], ocl_descriptor.in_common_buffer, CL_TRUE, 0,
        sizeof(T) * arraySize, input_array.dynamic_array, 0, NULL, NULL));

    // setup and execute kernels, read and validate results
    ASSERT_NO_FATAL_FAILURE(executeSetAndExecuteKernelsCopy(
        typeNameGetter.getCombinedString("read_").c_str(), input_array,
        ocl_descriptor.in_common_sub_buffer, ocl_descriptor.buffers,
        ocl_descriptor.kernels, ocl_descriptor.queues, ocl_descriptor.context,
        ocl_descriptor.program, arraySize));
  }

  //|  TEST: testSubBufferCopyHostPtr
  //|
  //|  Purpose
  //|  -------
  //|
  //|  Verify that the content of an OpenCL sub-buffer is visible to all
  // devices in the context
  //|
  //|  Method
  //|  ------
  //|
  //|  1. Create shared input cl_mem buffer on a shared context (shared for CPU
  // and GPU) with CL_MEM_COPY_HOST_PTR
  //|  2. Create shared sub-buffer of buffer in step 1 with
  // CL_MEM_COPY_HOST_PTR |  2. For each device create separate output buffer
  //|  2. Create for each device a kernel which copies all elements of input
  // buffer into output buffer |  3. Run kernels on CPU and GPU |  4.
  // Validate that all output buffers contain elements of input buffer
  //|
  //|  Pass criteria
  //|  -------------
  //|
  //|  Validate that each device is able to read all sub-buffer elements
  //|

  void testSubBufferCopyHostPtr(OpenCLDescriptor &ocl_descriptor) {
    // set work dimensions
    cl_uint work_dim = 1;
    size_t global_work_size = 1;

    // create and initialize array
    int arraySize = (size_t)4;
    DynamicArray<T> input_array(arraySize);
    // kernel source
    std::string kernelSource =
        getKernelSource(typeNameGetter.getType()).c_str();
    // create OpenCL queues, program and context
    ASSERT_NO_FATAL_FAILURE(setUpContextProgramQueuesFromStringSource(
        ocl_descriptor, kernelSource.c_str()));

    // create shared buffer
    ASSERT_NO_FATAL_FAILURE(
        createBuffer(&ocl_descriptor.in_common_buffer, ocl_descriptor.context,
                     CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                     arraySize * sizeof(T), input_array.dynamic_array));

    // create sub-buffer
    cl_buffer_region input_buffer_region = {0, arraySize * sizeof(T)};

    ASSERT_NO_FATAL_FAILURE(createSubBuffer(
        &ocl_descriptor.in_common_sub_buffer, ocl_descriptor.in_common_buffer,
        CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &input_buffer_region));

    // setup and execute kernels, read and validate results
    ASSERT_NO_FATAL_FAILURE(executeSetAndExecuteKernelsCopy(
        typeNameGetter.getCombinedString("read_").c_str(), input_array,
        ocl_descriptor.in_common_sub_buffer, ocl_descriptor.buffers,
        ocl_descriptor.kernels, ocl_descriptor.queues, ocl_descriptor.context,
        ocl_descriptor.program, arraySize));
  }
};

/*
 * VR8BufferTests - fixture for type dependant test cases, containins
 *typeNameGetter. typeNameGetter returns string representation of type name All
 *generated types should provide specialization for TypeNameGetter
 **/
template <typename T>
class VR8Buffer : public CommonRuntime, public BufferTests<T> {};

/*
 * VR8BufferTests - fixture for type dependant test cases, containins
 *typeNameGetter. typeNameGetter returns string representation of type name All
 *generated types should provide specialization for TypeNameGetter
 **/
template <typename T>
class VR8Buffer_Fission : public BufferTests<T>, public FissionWrapper {};

// define in bufferTypes the types this test case should be run on
// HalfWrapper - tests buffer of half. HalfWrapper is used in test
// cases as a wrapper to resolve colissions of half and ushort types
typedef ::testing::Types<
    cl_char, cl_uchar, cl_short, cl_ushort, cl_int, cl_uint, cl_long, cl_ulong,
    cl_float, cl_char2, cl_char4, cl_char8, cl_char16, cl_uchar2, cl_uchar4,
    cl_uchar8, cl_uchar16, cl_short2, cl_short4, cl_short8, cl_short16,
    cl_ushort2, cl_ushort4, cl_ushort8, cl_ushort16, cl_int2, cl_int4, cl_int8,
    cl_int16, cl_uint2, cl_uint4, cl_uint8, cl_uint16, cl_long2, cl_long4,
    cl_long8, cl_long16, cl_ulong2, cl_ulong4, cl_ulong8, cl_ulong16, cl_float2,
    cl_float4, cl_float8, cl_float16, UserDefinedStructure, HalfWrapper>
    bufferTypes;

TYPED_TEST_CASE(VR8Buffer, bufferTypes);

TYPED_TEST_CASE(VR8Buffer_Fission, bufferTypes);

#endif /* VR8_BUFFER_GTEST_ */
