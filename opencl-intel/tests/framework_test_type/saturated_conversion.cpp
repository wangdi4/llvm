#include "CL/cl.h"
#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "cl_types.h"
#include <algorithm>
#include <limits>
#include <stdio.h>

/*******************************************************************************
 * saturated_conversion_NaN_test
 * According to OpenCL Spec, in saturated mode, values that are outside
 * the representable range shall clamp to the nearest representable value in the
 * destination format. (NaN should be converted to 0).
 *
 * So test checks if in saturated mode NaNs are properly handled
 ******************************************************************************/

extern cl_device_type gDeviceType;
const int num = 32;

cl_int check_scalar(std::vector<float> h_rhs, std::vector<char> h_res) {

  const char *ocl_test_program[] = {
      "__kernel void test_convert_char_sat_float(__global float *src, __global "
      "char *res)"
      "{"
      "    size_t i = get_global_id(0);"
      "    res[i] = convert_char_sat(src[i]);"
      "}"};

  cl_int iRet = 0;
  cl_device_id device = NULL;
  cl_context context;
  cl_platform_id platform = 0;

  iRet = clGetPlatformIDs(1, &platform, NULL);
  CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

  context = clCreateContext(prop, 1, &device, NULL, NULL, &iRet);
  CheckException("clCreateContext", CL_SUCCESS, iRet);

  cl_command_queue queue =
      clCreateCommandQueueWithProperties(context, device, NULL, &iRet);
  CheckException("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);

  cl_program prog = clCreateProgramWithSource(
      context, 1, (const char **)&ocl_test_program, NULL, &iRet);
  CheckException("clCreateProgramWithSource", CL_SUCCESS, iRet);

  iRet = clBuildProgram(prog, 1, &device, NULL, NULL, NULL);
  CheckException("clBuildProgram", CL_SUCCESS, iRet);

  cl_kernel kernel = clCreateKernel(prog, "test_convert_char_sat_float", &iRet);
  CheckException("clCreateKernel", CL_SUCCESS, iRet);

  cl_mem rhs = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * num,
                              NULL, &iRet);
  CheckException("clCreateBuffer", CL_SUCCESS, iRet);
  cl_mem res = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(char) * num,
                              NULL, &iRet);
  CheckException("clCreateBuffer", CL_SUCCESS, iRet);

  iRet = clEnqueueWriteBuffer(queue, rhs, CL_TRUE, 0, sizeof(float) * num,
                              &h_rhs[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &rhs);
  CheckException("clSetKernelArg", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), &res);
  CheckException("clSetKernelArg", CL_SUCCESS, iRet);

  size_t global = num;
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, NULL, 0, NULL,
                                NULL);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

  iRet = clEnqueueReadBuffer(queue, res, CL_TRUE, 0, sizeof(char) * num,
                             &h_res[0], 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer", CL_SUCCESS, iRet);

  clReleaseMemObject(rhs);
  clReleaseMemObject(res);
  clReleaseCommandQueue(queue);
  clReleaseKernel(kernel);
  clReleaseProgram(prog);
  clReleaseContext(context);
  return iRet;
}

cl_int check_vector(std::vector<float> h_rhs, std::vector<char> h_res) {

  const char *ocl_test_program[] = {
      "__kernel void test_convert_char8_sat_float8(__global float8 *src, "
      "__global char8 *res)"
      "{"
      "    size_t i = get_global_id(0);"
      "    res[i] = convert_char8_sat(src[i]);"
      "}"};

  cl_int iRet = 0;
  cl_device_id device = NULL;
  cl_context context;
  cl_platform_id platform = 0;

  iRet = clGetPlatformIDs(1, &platform, NULL);
  CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

  context = clCreateContext(prop, 1, &device, NULL, NULL, &iRet);
  CheckException("clCreateContext", CL_SUCCESS, iRet);

  cl_command_queue queue =
      clCreateCommandQueueWithProperties(context, device, NULL, &iRet);
  CheckException("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);

  cl_program prog = clCreateProgramWithSource(
      context, 1, (const char **)&ocl_test_program, NULL, &iRet);
  CheckException("clCreateProgramWithSource", CL_SUCCESS, iRet);

  iRet = clBuildProgram(prog, 1, &device, NULL, NULL, NULL);
  CheckException("clBuildProgram", CL_SUCCESS, iRet);

  cl_kernel kernel =
      clCreateKernel(prog, "test_convert_char8_sat_float8", &iRet);
  CheckException("clCreateKernel", CL_SUCCESS, iRet);

  size_t global = num / 8;
  cl_mem rhs = clCreateBuffer(context, CL_MEM_READ_ONLY,
                              sizeof(cl_float8) * global, NULL, &iRet);
  CheckException("clCreateBuffer", CL_SUCCESS, iRet);
  cl_mem res = clCreateBuffer(context, CL_MEM_READ_WRITE,
                              sizeof(cl_char8) * global, NULL, &iRet);
  CheckException("clCreateBuffer", CL_SUCCESS, iRet);

  iRet =
      clEnqueueWriteBuffer(queue, rhs, CL_TRUE, 0, sizeof(cl_float8) * global,
                           &h_rhs[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &rhs);
  CheckException("clSetKernelArg", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), &res);
  CheckException("clSetKernelArg", CL_SUCCESS, iRet);

  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, NULL, 0, NULL,
                                NULL);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

  iRet = clEnqueueReadBuffer(queue, res, CL_TRUE, 0, sizeof(cl_char8) * global,
                             &h_res[0], 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer", CL_SUCCESS, iRet);

  clReleaseMemObject(rhs);
  clReleaseMemObject(res);
  clReleaseCommandQueue(queue);
  clReleaseKernel(kernel);
  clReleaseProgram(prog);
  clReleaseContext(context);
  return iRet;
}

bool check_results(char loVal, char hiVal, std::vector<float> &h_rhs,
                   std::vector<char> h_res) {
  for (size_t i = 0; i < h_rhs.size(); i++) {
    int ival = (int)h_rhs[i];
    char val = (char)std::min<float>(std::max<float>(ival, loVal), hiVal);
    // isnan(h_rhs[i])
    if (h_rhs[i] != h_rhs[i]) {
      val = 0;
    }
    if (val != h_res[i]) {
      printf("Value error at %zu\n", i);
      return false;
    }
  }
  return true;
}

bool saturated_conversion_NaN_test() {
  if (gDeviceType != CL_DEVICE_TYPE_CPU)
    return true;

  printf("---------------------------------------\n");
  printf("saturated_conversion_test\n");
  printf("---------------------------------------\n");

  cl_int iRet = 0;

  char loVal = std::numeric_limits<char>::min();
  char hiVal = std::numeric_limits<char>::max();

  std::vector<float> h_rhs(num);

  for (int i = 0; i < num; i++) {
    int val = i;
    // set some values on rhs to NaN
    if (val < hiVal && val > loVal) {
      h_rhs[i] = std::numeric_limits<float>::quiet_NaN();
    } else {
      h_rhs[i] = (float)val;
    }
  }

  std::vector<char> h_res(num);
  iRet = check_scalar(h_rhs, h_res);
  CheckException("check_scalar", CL_SUCCESS, iRet);

  if (!check_results(loVal, hiVal, h_rhs, h_res)) {
    printf("Results are different from expected in check_scalar\n");
    return false;
  }

  std::vector<char> h_res_vec(num);
  iRet = check_vector(h_rhs, h_res_vec);
  CheckException("check_vector", CL_SUCCESS, iRet);

  if (!check_results(loVal, hiVal, h_rhs, h_res_vec)) {
    printf("Results are different from expected in check_vector\n");
    return false;
  }

  return true;
}
