#include "CL21.h"
#include "gtest_wrapper.h"

TEST_F(CL21, CreateProgramWithIL_IL_VERSION) {
  cl_int iRet = CL_SUCCESS;

  const size_t il_version_size = 14;
  std::string il_version(il_version_size, '\0');

  iRet = clGetDeviceInfo(m_device, CL_DEVICE_IL_VERSION, il_version.size(),
                         &il_version[0],
                         /*ret_size*/ nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetDeviceInfo with CL_DEVICE_IL_VERSION failed. ";
}

TEST_F(CL21, CreateProgramWithIL_Negative) {
  cl_int iRet = CL_SUCCESS;
  cl_program program = nullptr;

  std::vector<char> spirv;
  ASSERT_NO_FATAL_FAILURE(GetSimpleSPIRV(spirv));

  program = clCreateProgramWithIL(nullptr, spirv.data(), spirv.size(), &iRet);
  ASSERT_EQ(CL_INVALID_CONTEXT, iRet)
      << " clCreateProgramWithIL with invalid context failed. ";
  ASSERT_EQ(CL_INVALID_HANDLE, program);

  program = clCreateProgramWithIL(m_context, nullptr, spirv.size(), &iRet);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clCreateProgramWithIL with nullptr IL buffer failed. ";
  ASSERT_EQ(CL_INVALID_HANDLE, program);

  program = clCreateProgramWithIL(m_context, spirv.data(), /*length*/ 0, &iRet);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clCreateProgramWithIL with 0 length failed. ";
  ASSERT_EQ(CL_INVALID_HANDLE, program);

  std::string wrong_IL("trash trash trash");
  program =
      clCreateProgramWithIL(m_context, &wrong_IL[0], wrong_IL.size(), &iRet);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clCreateProgramWithIL with invalid IL failed. ";
  ASSERT_EQ(CL_INVALID_HANDLE, program);
}

TEST_F(CL21, CreateProgramWithIL) {
  cl_int iRet = CL_SUCCESS;
  cl_program program = nullptr;

  std::vector<char> spirv;
  ASSERT_NO_FATAL_FAILURE(GetSimpleSPIRV(spirv));

  program = clCreateProgramWithIL(m_context, spirv.data(), spirv.size(), &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithIL failed. ";

  iRet = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clBuildProgram failed. ";

  cl_kernel kern = clCreateKernel(program, "test_hostptr", &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateKernel failed. ";

  size_t arg_size = 4;
  cl_mem arg = clCreateBuffer(m_context, 0, arg_size, nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateBuffer failed. ";

  float pattern = 2;
  iRet = clEnqueueFillBuffer(m_queue, arg, &pattern, sizeof(pattern), 0,
                             arg_size, 0, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueFillBuffer failed. ";

  iRet = clFinish(m_queue);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clFinish failed. ";

  iRet = clSetKernelArg(kern, 0, sizeof(cl_mem), &arg);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clSetKernelArg failed. ";
  iRet = clSetKernelArg(kern, 1, sizeof(cl_mem), &arg);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clSetKernelArg failed. ";
  iRet = clSetKernelArg(kern, 2, sizeof(cl_mem), &arg);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clSetKernelArg failed. ";

  size_t gws = 1;
  iRet = clEnqueueNDRangeKernel(m_queue, kern, 1, nullptr, &gws, nullptr, 0,
                                nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueNDRangeKernel failed. ";

  iRet = clFinish(m_queue);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clFinish failed. ";

  float *out =
      (float *)clEnqueueMapBuffer(m_queue, arg, CL_TRUE, CL_MAP_READ, 0,
                                  arg_size, 0, nullptr, nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueMapBuffer failed. ";

  iRet = clFinish(m_queue);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clFinish failed. ";

  ASSERT_EQ(4, *out) << "Invalid kernel result.";
}

TEST_F(CL21, CreateProgramWithIL_PROGRAM_IL_Negative) {
  cl_int iRet = CL_SUCCESS;
  cl_program program = nullptr;

  const char *source[] = {"__kernel() {}"};
  program = clCreateProgramWithSource(m_context, 1, source, nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed. ";

  std::vector<char> spirv(1, 1);
  size_t ret_size = 1;

  iRet = clGetProgramInfo(program, CL_PROGRAM_IL, spirv.size(), &spirv[0],
                          &ret_size);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetProgramInfo with CL_PROGRAM_IL failed. ";
  ASSERT_EQ(size_t(0), ret_size)
      << " clGetProgramInfo with CL_PROGRAM_IL failed. Ret size not 0. ";
  ASSERT_EQ(char(1), spirv[0])
      << " clGetProgramInfo with CL_PROGRAM_IL failed. param_value changed. ";
}

TEST_F(CL21, CreateProgramWithIL_PROGRAM_IL) {
  cl_int iRet = CL_SUCCESS;
  cl_program program = nullptr;

  std::vector<char> spirv;
  ASSERT_NO_FATAL_FAILURE(GetSimpleSPIRV(spirv));

  program = clCreateProgramWithIL(m_context, spirv.data(), spirv.size(), &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithIL failed. ";

  size_t ret_size = 0;
  iRet = clGetProgramInfo(program, CL_PROGRAM_IL,
                          /*param_value_size*/ 0, nullptr, &ret_size);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetProgramInfo with CL_PROGRAM_IL failed. ";
  ASSERT_EQ(spirv.size(), ret_size)
      << " clGetProgramInfo with CL_PROGRAM_IL failed. ";

  std::vector<char> spirv_out(ret_size);

  iRet = clGetProgramInfo(program, CL_PROGRAM_IL, spirv_out.size(),
                          &spirv_out[0], nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetProgramInfo with CL_PROGRAM_IL failed. ";

  ASSERT_FALSE(memcmp(spirv.data(), spirv_out.data(), spirv.size()));
}
