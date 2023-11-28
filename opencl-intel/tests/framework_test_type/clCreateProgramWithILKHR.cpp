#include "CL.h"
#include "test_utils.h"
#include "FrameworkTest.h"
#include "common_utils.h"

extern cl_device_type gDeviceType;

TEST_F(CL, Test_CreateProgramWithILKHR_Negative) {
  cl_int iRet = CL_SUCCESS;
  cl_program program = nullptr;

  std::vector<char> spirv;
  ASSERT_NO_FATAL_FAILURE(GetSimpleSPIRV(spirv));

  program =
      clCreateProgramWithILKHR(nullptr, spirv.data(), spirv.size(), &iRet);
  ASSERT_EQ(CL_INVALID_CONTEXT, iRet)
      << " clCreateProgramWithILKHR with invalid context expected to fail. ";
  ASSERT_EQ(CL_INVALID_HANDLE, program);

  program = clCreateProgramWithILKHR(m_context, nullptr, spirv.size(), &iRet);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clCreateProgramWithILKHR with nullptr IL buffer expected to fail. ";
  ASSERT_EQ(CL_INVALID_HANDLE, program);

  program =
      clCreateProgramWithILKHR(m_context, spirv.data(), /*length*/ 0, &iRet);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clCreateProgramWithILKHR with 0 length expected to fail. ";
  ASSERT_EQ(CL_INVALID_HANDLE, program);

  std::string wrong_IL("trash trash trash");
  program =
      clCreateProgramWithILKHR(m_context, &wrong_IL[0], wrong_IL.size(), &iRet);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clCreateProgramWithILKHR with invalid IL expected to fail. ";
  ASSERT_EQ(CL_INVALID_HANDLE, program);
}

TEST_F(CL, Test_CreateProgramWithILKHR_FP) {
  cl_int iRet = CL_SUCCESS;
  cl_program program = nullptr;

  cl_program(CL_API_CALL * create_program_with_il)(cl_context, const void *,
                                                   size_t, cl_int *);
  create_program_with_il =
      (cl_program(CL_API_CALL *)(cl_context, const void *, size_t, cl_int *))
          clGetExtensionFunctionAddress("clCreateProgramWithILKHR");
  ASSERT_NE(nullptr, create_program_with_il)
      << " clCreateProgramWithILKHR(\"clCreateProgramWithILKHR\") "
         "failed";

  std::vector<char> spirv;
  ASSERT_NO_FATAL_FAILURE(GetSimpleSPIRV(spirv));

  program = create_program_with_il(nullptr, spirv.data(), spirv.size(), &iRet);
  ASSERT_EQ(CL_INVALID_CONTEXT, iRet)
      << " clCreateProgramWithILKHR with invalid context expected to fail. ";
  ASSERT_EQ(CL_INVALID_HANDLE, program);

  program = create_program_with_il(m_context, nullptr, spirv.size(), &iRet);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clCreateProgramWithILKHR with nullptr IL buffer expected to fail. ";
  ASSERT_EQ(CL_INVALID_HANDLE, program);

  program =
      create_program_with_il(m_context, spirv.data(), /*length*/ 0, &iRet);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clCreateProgramWithILKHR with 0 length expected to fail. ";
  ASSERT_EQ(CL_INVALID_HANDLE, program);

  std::string wrong_IL("trash trash trash");
  program =
      create_program_with_il(m_context, &wrong_IL[0], wrong_IL.size(), &iRet);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clCreateProgramWithILKHR with invalid IL expected to fail. ";
  ASSERT_EQ(CL_INVALID_HANDLE, program);
}

TEST_F(CL, Test_CreateProgramWithILKHR) {
  cl_int iRet = CL_SUCCESS;
  cl_program program = nullptr;

  std::vector<char> spirv;
  ASSERT_NO_FATAL_FAILURE(GetSimpleSPIRV(spirv));

  program =
      clCreateProgramWithILKHR(m_context, spirv.data(), spirv.size(), &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clCreateProgramTestCreateProgramWithILKHR failed. ";

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

TEST_F(CL, Test_CreateProgramWithILKHR_PROGRAM_IL) {
  cl_int iRet = CL_SUCCESS;
  cl_program program = nullptr;

  std::vector<char> spirv;
  ASSERT_NO_FATAL_FAILURE(GetSimpleSPIRV(spirv));

  program =
      clCreateProgramWithILKHR(m_context, spirv.data(), spirv.size(), &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithILKHR failed. ";

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

  ASSERT_EQ(spirv.size(), spirv_out.size());
  ASSERT_FALSE(memcmp(spirv.data(), spirv_out.data(), spirv.size()));
}
