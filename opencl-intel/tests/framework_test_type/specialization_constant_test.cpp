// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

// Original kernel:
// #pragma OPENCL EXTENSION cl_khr_fp16 : enable
// __kernel
// void foo(__global char* c,
//          __global short* s,
//          __global int* i,
//          __global long* l,
//          __global half* h,
//          __global float* f,
//          __global double* d) {
//   *c = 100;
//   *s = 1,
//   *i = 2,
//   *l = 3;
//   *h = 0.5h;
//   *f = 1.25f;
//   *d = 2.125;
// }
//
// Disassembled spec_const.spv
//; SPIR-V
//; Version: 1.1
//; Generator: Khronos LLVM/SPIR-V Translator; 14
//; Bound: 36
//; Schema: 0
//               OpCapability Addresses
//               OpCapability Kernel
//               OpCapability Float16Buffer
//               OpCapability Float64
//               OpCapability Int64
//               OpCapability Int16
//               OpCapability Int8
//          %1 = OpExtInstImport "OpenCL.std"
//               OpMemoryModel Physical32 OpenCL
//               OpEntryPoint Kernel %18 "foo"
//         %34 = OpString
//         "kernel_arg_type.foo.char*,short*,int*,long*,half*,float*,double*,"
//               OpSourceExtension "cl_khr_fp16"
//               OpSource OpenCL_C 200000
//               OpName %c "c"
//               OpName %s "s"
//               OpName %i "i"
//               OpName %l "l"
//               OpName %h "h"
//               OpName %f "f"
//               OpName %d "d"
//               OpName %entry "entry"
//               OpDecorate %uchar_100 SpecId 101
//               OpDecorate %ushort_1 SpecId 102
//               OpDecorate %uint_2 SpecId 103
//               OpDecorate %ulong_3 SpecId 104
//               OpDecorate %half_0x1pn1 SpecId 105
//               OpDecorate %float_1_25 SpecId 106
//               OpDecorate %double_2_125 SpecId 107
//               OpDecorate %35 FuncParamAttr NoCapture
//         %35 = OpDecorationGroup
//               OpGroupDecorate %35 %c %s %i %l %h %f %d
//      %uchar = OpTypeInt 8 0
//     %ushort = OpTypeInt 16 0
//       %uint = OpTypeInt 32 0
//      %ulong = OpTypeInt 64 0
//  %uchar_100 = OpSpecConstant %uchar 100
//   %ushort_1 = OpSpecConstant %ushort 1
//     %uint_2 = OpSpecConstant %uint 2
//    %ulong_3 = OpSpecConstant %ulong 3
//       %void = OpTypeVoid
//%_ptr_CrossWorkgroup_uchar = OpTypePointer CrossWorkgroup %uchar
//%_ptr_CrossWorkgroup_ushort = OpTypePointer CrossWorkgroup %ushort
//%_ptr_CrossWorkgroup_uint = OpTypePointer CrossWorkgroup %uint
//%_ptr_CrossWorkgroup_ulong = OpTypePointer CrossWorkgroup %ulong
//       %half = OpTypeFloat 16
//%_ptr_CrossWorkgroup_half = OpTypePointer CrossWorkgroup %half
//      %float = OpTypeFloat 32
//%_ptr_CrossWorkgroup_float = OpTypePointer CrossWorkgroup %float
//     %double = OpTypeFloat 64
//%_ptr_CrossWorkgroup_double = OpTypePointer CrossWorkgroup %double
//         %17 = OpTypeFunction %void %_ptr_CrossWorkgroup_uchar
//         %_ptr_CrossWorkgroup_ushort %_ptr_CrossWorkgroup_uint
//         %_ptr_CrossWorkgroup_ulong %_ptr_CrossWorkgroup_half
//         %_ptr_CrossWorkgroup_float %_ptr_CrossWorkgroup_double
//%half_0x1pn1 = OpSpecConstant %half 0x1p-1
// %float_1_25 = OpSpecConstant %float 1.25
//%double_2_125 = OpSpecConstant %double 2.125
//         %18 = OpFunction %void None %17
//          %c = OpFunctionParameter %_ptr_CrossWorkgroup_uchar
//          %s = OpFunctionParameter %_ptr_CrossWorkgroup_ushort
//          %i = OpFunctionParameter %_ptr_CrossWorkgroup_uint
//          %l = OpFunctionParameter %_ptr_CrossWorkgroup_ulong
//          %h = OpFunctionParameter %_ptr_CrossWorkgroup_half
//          %f = OpFunctionParameter %_ptr_CrossWorkgroup_float
//          %d = OpFunctionParameter %_ptr_CrossWorkgroup_double
//      %entry = OpLabel
//               OpStore %c %uchar_100 Aligned 1
//               OpStore %s %ushort_1 Aligned 2
//               OpStore %i %uint_2 Aligned 4
//               OpStore %l %ulong_3 Aligned 8
//               OpStore %h %half_0x1pn1 Aligned 2
//               OpStore %f %float_1_25 Aligned 4
//               OpStore %d %double_2_125 Aligned 8
//               OpReturn
//               OpFunctionEnd

#include "CL_BASE.h"
#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "common_utils.h"
#include <CL/cl.h>

extern cl_device_type gDeviceType;

// The idea of the test is to run a kernel which writes some constants to its
// arguments. Using the clSetProgramSpecializationConstant API we can redefine
// the constants to be written.

class SpecializationConstant : public ::testing::Test {
protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
  cl_program m_program;
  cl_int iRet = CL_SUCCESS;

  // The test variables. During execution of the tests the constant values
  // are written to these variables, and after execution of the kernel
  // we can check what constants have been written to them.
  cl_char c;
  cl_short s;
  cl_int i;
  cl_long l;
  cl_half h;
  cl_float f;
  cl_double d;

  // Memory buffers to wrap the variables above in order to pass them as
  // kernel arguments.
  cl_mem mem_c;
  cl_mem mem_s;
  cl_mem mem_i;
  cl_mem mem_l;
  cl_mem mem_h;
  cl_mem mem_f;
  cl_mem mem_d;

  // Fixture for this test suite, i.e. this function is run before execution
  // of verey test in this suite. It implements the following
  // boilerplate code:
  //  - initialize OpenCL
  //  - read the SPIR-V file
  //  - create an OpenCL program from the SPIR-V file
  //  - reset the test variables
  //  - create memory buffers for the kernel arguments.
  virtual void SetUp() {
    ASSERT_OCL_SUCCESS(clGetPlatformIDs(1, &m_platform, NULL),
                       "clGetPlatformIDs");
    ASSERT_OCL_SUCCESS(
        clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL),
        "clGetDeviceIDs");

    m_context = clCreateContext(NULL, 1, &m_device, NULL, NULL, &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateContext");

    m_queue =
        clCreateCommandQueueWithProperties(m_context, m_device, NULL, &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateCommandQueueWithProperties");

    std::ifstream spirv_file(get_exe_dir() + "spec_const.spv",
                             std::fstream::binary);
    std::vector<char> spirv(std::istreambuf_iterator<char>{spirv_file}, {});
    m_program =
        clCreateProgramWithIL(m_context, spirv.data(), spirv.size(), &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateProgramWithIL");

    c = 0;
    s = 0;
    i = 0;
    l = 0;
    h = 0;
    f = 0;
    d = 0;

    mem_c =
        clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, sizeof(c), &c, &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateBuffer");
    mem_s =
        clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, sizeof(s), &s, &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateBuffer");
    mem_i =
        clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, sizeof(i), &i, &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateBuffer");
    mem_l =
        clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, sizeof(l), &l, &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateBuffer");
    mem_h =
        clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, sizeof(h), &h, &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateBuffer");
    mem_f =
        clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, sizeof(f), &f, &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateBuffer");
    mem_d =
        clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, sizeof(d), &d, &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateBuffer");
  }

  virtual void TearDown() {
    EXPECT_OCL_SUCCESS(clReleaseMemObject(mem_c), "clReleaseMemObject");
    EXPECT_OCL_SUCCESS(clReleaseMemObject(mem_s), "clReleaseMemObject");
    EXPECT_OCL_SUCCESS(clReleaseMemObject(mem_i), "clReleaseMemObject");
    EXPECT_OCL_SUCCESS(clReleaseMemObject(mem_l), "clReleaseMemObject");
    EXPECT_OCL_SUCCESS(clReleaseMemObject(mem_h), "clReleaseMemObject");
    EXPECT_OCL_SUCCESS(clReleaseMemObject(mem_f), "clReleaseMemObject");
    EXPECT_OCL_SUCCESS(clReleaseMemObject(mem_d), "clReleaseMemObject");
    EXPECT_OCL_SUCCESS(clReleaseProgram(m_program), "clReleaseProgram");
    EXPECT_OCL_SUCCESS(clReleaseCommandQueue(m_queue), "clReleaseCommandQueue");
    EXPECT_OCL_SUCCESS(clReleaseContext(m_context), "clReleaseContext");
  }

  template <typename T> void setSpecConst(unsigned id, T *val) {
    iRet = clSetProgramSpecializationConstant(m_program, id, sizeof(T), val);
    ASSERT_OCL_SUCCESS(iRet, "clSetProgramSpecializationConstant");
  }

  // Helper function, which also contains boilerplate code. It does:
  // - build the program
  // - create a kernel
  // - set kernel arguments
  // - run the kernel
  // - wait until the kernel executoin complete
  void buildAndRun() {
    iRet = clBuildProgram(m_program, 0, nullptr, nullptr, nullptr, nullptr);
    if (iRet != CL_SUCCESS) {
      std::string log("", 1000);
      clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG,
                            log.size(), &log[0], nullptr);
      std::cout << log << std::endl;
    }
    ASSERT_OCL_SUCCESS(iRet, "clBuildProgram");

    cl_kernel kern = clCreateKernel(m_program, "foo", &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateKernel");

    ASSERT_OCL_SUCCESS(clSetKernelArg(kern, 0, sizeof(cl_mem), &mem_c),
                       "clSetKernelArg");
    ASSERT_OCL_SUCCESS(clSetKernelArg(kern, 1, sizeof(cl_mem), &mem_s),
                       "clSetKernelArg");
    ASSERT_OCL_SUCCESS(clSetKernelArg(kern, 2, sizeof(cl_mem), &mem_i),
                       "clSetKernelArg");
    ASSERT_OCL_SUCCESS(clSetKernelArg(kern, 3, sizeof(cl_mem), &mem_l),
                       "clSetKernelArg");
    ASSERT_OCL_SUCCESS(clSetKernelArg(kern, 4, sizeof(cl_mem), &mem_h),
                       "clSetKernelArg");
    ASSERT_OCL_SUCCESS(clSetKernelArg(kern, 5, sizeof(cl_mem), &mem_f),
                       "clSetKernelArg");
    ASSERT_OCL_SUCCESS(clSetKernelArg(kern, 6, sizeof(cl_mem), &mem_d),
                       "clSetKernelArg");

    size_t gws = 1;
    iRet = clEnqueueNDRangeKernel(m_queue, kern, 1, nullptr, &gws, nullptr, 0,
                                  nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueNDRangeKernel failed. ";

    EXPECT_OCL_SUCCESS(clFinish(m_queue), "clFinish");
    EXPECT_OCL_SUCCESS(clReleaseKernel(kern), "clReleaseKernel");
  }
};

// Make sure that if we do not specialize the constants, the default values
// are used.
TEST_F(SpecializationConstant, CheckDefaultValue) {
  buildAndRun();

  ASSERT_EQ(c, 100) << "Wrong result!";
  ASSERT_EQ(s, 1) << "Wrong result!";
  ASSERT_EQ(i, 2) << "Wrong result!";
  ASSERT_EQ(l, 3) << "Wrong result!";
  ASSERT_EQ(h, 0x3800)
      << "Wrong result!"; // 0x3800 is binary representaion of 0.5 for half
  ASSERT_EQ(f, 1.25f) << "Wrong result!";
  ASSERT_EQ(d, 2.125) << "Wrong result!";
}

// The typical usecase of constant specialization.
// In this test we specialize all spec. constants in the SPIR-V module and
// check that values provided via clSetProgramSpecializationConstant was
// used instead of default ones.
TEST_F(SpecializationConstant, CheckSpecialization) {
  cl_char spec_c = 'q';
  cl_short spec_s = -42;
  cl_int spec_i = -43;
  cl_long spec_l = -44;
  cl_half spec_h = 0xC480; // binary representation of -4.5 for half;
  cl_float spec_f = -46.75;
  cl_double spec_d = -47.875;

  setSpecConst(101, &spec_c);
  setSpecConst(102, &spec_s);
  setSpecConst(103, &spec_i);
  setSpecConst(104, &spec_l);
  setSpecConst(105, &spec_h);
  setSpecConst(106, &spec_f);
  setSpecConst(107, &spec_d);

  buildAndRun();

  ASSERT_EQ(c, spec_c) << "Wrong result!";
  ASSERT_EQ(s, spec_s) << "Wrong result!";
  ASSERT_EQ(i, spec_i) << "Wrong result!";
  ASSERT_EQ(l, spec_l) << "Wrong result!";
  ASSERT_EQ(h, spec_h) << "Wrong result!";
  ASSERT_EQ(f, spec_f) << "Wrong result!";
  ASSERT_EQ(d, spec_d) << "Wrong result!";
}

// According to the OpenCL spec:
// "Calling this[clSetProgramSpecializationConstant] function multiple times
// for the same specialization constant shall cause the last provided value to
// override any previously specified value. The values are used by a subsequent
// clBuildProgram call for the program."
// So we check that the latest values used for specialization was used by the
// kernel.
TEST_F(SpecializationConstant, CheckOverriding) {
  int spec_i = -43;
  float spec_f = -46.75f;
  setSpecConst(103, &spec_i);
  setSpecConst(106, &spec_f);

  int spec_i2 = 55;
  float spec_f2 = 3.14f;
  setSpecConst(103, &spec_i2);
  setSpecConst(106, &spec_f2);

  buildAndRun();

  ASSERT_EQ(i, spec_i2) << "Wrong result!";
  ASSERT_EQ(f, spec_f2) << "Wrong result!";
}

// Check that clSetProgramSpecializationConstant returns correct error codes
// per the spec.
TEST_F(SpecializationConstant, Negative) {
  // program is not a valid program object created from a module in an
  // intermediate format (e.g. SPIR-V).
  iRet = clSetProgramSpecializationConstant(nullptr, 103, sizeof(int), &i);
  ASSERT_EQ(CL_INVALID_PROGRAM, iRet)
      << " clSetProgramSpecializationConstant returned wrong value. ";

  // spec_id is not a valid specialization constant ID
  iRet = clSetProgramSpecializationConstant(m_program, 555, sizeof(int), &i);
  ASSERT_EQ(CL_INVALID_SPEC_ID, iRet)
      << " clSetProgramSpecializationConstant returned wrong value. ";

  // spec_size does not match the size of the specialization constant in the
  // module
  iRet = clSetProgramSpecializationConstant(m_program, 101, sizeof(int), &i);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clSetProgramSpecializationConstant returned wrong value. ";

  // 0 doesn't match the size of the int type
  iRet = clSetProgramSpecializationConstant(m_program, 103, 0, &i);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clSetProgramSpecializationConstant returned wrong value. ";

  // spec_value is NULL
  iRet =
      clSetProgramSpecializationConstant(m_program, 103, sizeof(int), nullptr);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clSetProgramSpecializationConstant returned wrong value. ";
}
