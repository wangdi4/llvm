#include "CL/cl.h"
#include "bi_tests.h"
#include "cl_types.h"
#include "test_utils.h"

#include <cmath>
#include <limits>

/*******************************************************************************
 * Test checks that atomic_min/atomic_max in kernel code compares float/double
 * values correctly.
 ******************************************************************************/

extern cl_device_type gDeviceType;

namespace {
const char *OCL12_KERNEL_MIN_TEST_CODE_STR =
    "__kernel void test_atomic_min_f(__global T *src_a, "
    "__global T *src_b, "
    "__global T *r_old, "
    "__global T *r_cmp) { "
    "int tid = get_global_id(0); "
    "r_cmp[tid] = src_a[tid]; "
    "r_old[tid] = atomic_min(&(r_cmp[tid]), src_b[tid]); "
    "}";

const char *OCL12_KERNEL_MAX_TEST_CODE_STR =
    "__kernel void test_atomic_max_f(__global T *src_a, "
    "__global T *src_b, "
    "__global T *r_old, "
    "__global T *r_cmp) { "
    "int tid = get_global_id(0); "
    "r_cmp[tid] = src_a[tid]; "
    "r_old[tid] = atomic_max(&(r_cmp[tid]), src_b[tid]); "
    "}";

const char *OCL20_KERNEL_MIN_TEST_CODE_STR =
    "__kernel void test_atomic_min_f(__global T *src_a, "
    "__global T *src_b, "
    "__global T *r_old, "
    "__global AT *r_cmp) { "
    "int tid = get_global_id(0); "
    "atomic_store_explicit(&(r_cmp[tid]), src_a[tid], memory_order_relaxed); "
    "r_old[tid] = atomic_fetch_min_explicit(&(r_cmp[tid]), src_b[tid], "
    "memory_order_relaxed, "
    "memory_scope_device); "
    "}";

const char *OCL20_KERNEL_MAX_TEST_CODE_STR =
    "__kernel void test_atomic_max_f(__global T *src_a, "
    "__global T *src_b, "
    "__global T *r_old, "
    "__global AT *r_cmp) { "
    "int tid = get_global_id(0); "
    "atomic_store_explicit(&(r_cmp[tid]), src_a[tid], memory_order_relaxed); "
    "r_old[tid] = atomic_fetch_max_explicit(&(r_cmp[tid]), src_b[tid], "
    "memory_order_relaxed, "
    "memory_scope_device); "
    "}";
} // namespace

template <typename T>
cl_int check_atomic_min(bool isOCL20, const std::vector<T> &src_a,
                        const std::vector<T> &src_b, std::vector<T> &r_old,
                        std::vector<T> &r_cmp) {
  cl_int iRet = 0;
  cl_device_id device = NULL;
  cl_context context;
  cl_platform_id platform = 0;
  const size_t num = src_a.size();

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

  const char *kernelString =
      isOCL20 ? OCL20_KERNEL_MIN_TEST_CODE_STR : OCL12_KERNEL_MIN_TEST_CODE_STR;
  cl_program prog = clCreateProgramWithSource(
      context, 1, (const char **)&kernelString, NULL, &iRet);
  CheckException("clCreateProgramWithSource", CL_SUCCESS, iRet);

  std::string options;
  if (isOCL20) {
    options = "-cl-std=CL2.0";
    options += (is_same<T, double>::value) ? " -D T=double -D AT=atomic_double"
                                           : " -D T=float -D AT=atomic_float";
  } else {
    options = "-cl-std=CL1.2";
    options += (is_same<T, double>::value) ? " -D T=double" : " -D T=float";
  }
  options += " -D float_atomics_enable";

  printf("Building program with options %s\n", options.c_str());
  iRet = clBuildProgram(prog, 1, &device, options.c_str(), NULL, NULL);
  if (iRet != CL_SUCCESS) {
    char buildLog[2048];
    clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog),
                          buildLog, NULL);
    printf("Build Failed, log:\n %s\n", buildLog);
  }
  CheckException("clBuildProgram", CL_SUCCESS, iRet);

  cl_kernel kernel = clCreateKernel(prog, "test_atomic_min_f", &iRet);
  CheckException("clCreateKernel", CL_SUCCESS, iRet);

  cl_mem k_a =
      clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(T) * num, NULL, &iRet);
  CheckException("clCreateBuffer src a", CL_SUCCESS, iRet);
  cl_mem k_b =
      clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(T) * num, NULL, &iRet);
  CheckException("clCreateBuffer src b", CL_SUCCESS, iRet);
  cl_mem k_o =
      clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(T) * num, NULL, &iRet);
  CheckException("clCreateBuffer res old", CL_SUCCESS, iRet);
  cl_mem k_s =
      clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(T) * num, NULL, &iRet);
  CheckException("clCreateBuffer res cmp", CL_SUCCESS, iRet);

  iRet = clEnqueueWriteBuffer(queue, k_a, CL_TRUE, 0, sizeof(T) * num,
                              &src_a[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer src a", CL_SUCCESS, iRet);
  iRet = clEnqueueWriteBuffer(queue, k_b, CL_TRUE, 0, sizeof(T) * num,
                              &src_b[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer src b", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &k_a);
  CheckException("clSetKernelArg src a", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), &k_b);
  CheckException("clSetKernelArg src b", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 2, sizeof(cl_mem), &k_o);
  CheckException("clSetKernelArg res old", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 3, sizeof(cl_mem), &k_s);
  CheckException("clSetKernelArg res cmp", CL_SUCCESS, iRet);

  size_t global = num;
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, NULL, 0, NULL,
                                NULL);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

  iRet = clEnqueueReadBuffer(queue, k_o, CL_TRUE, 0, sizeof(T) * num, &r_old[0],
                             0, NULL, NULL);
  CheckException("clEnqueueReadBuffer res old", CL_SUCCESS, iRet);
  iRet = clEnqueueReadBuffer(queue, k_s, CL_TRUE, 0, sizeof(T) * num, &r_cmp[0],
                             0, NULL, NULL);
  CheckException("clEnqueueReadBuffer res cmp", CL_SUCCESS, iRet);

  clReleaseMemObject(k_a);
  clReleaseMemObject(k_b);
  clReleaseMemObject(k_o);
  clReleaseMemObject(k_s);
  clReleaseCommandQueue(queue);
  clReleaseKernel(kernel);
  clReleaseProgram(prog);
  clReleaseContext(context);

  return iRet;
}

template <typename T>
bool check_min_results(const std::vector<T> &src_a, const std::vector<T> &src_b,
                       const std::vector<T> &r_old,
                       const std::vector<T> &r_cmp) {
  for (size_t i = 0; i < src_a.size(); i++) {
    if (src_a[i] != r_old[i] &&
        !(std::isnan(src_a[i]) && std::isnan(r_old[i]))) {
      printf("Old value error at %zu: got %f, expected %f\n", i, r_old[i],
             src_a[i]);
      return false;
    }

    T exp_cmp = src_a[i] < src_b[i] ? src_a[i] : src_b[i];
    if (exp_cmp != r_cmp[i] && !(std::isnan(exp_cmp) && std::isnan(r_cmp[i]))) {
      printf("Comparison error at %zu: got %f, expected %f\n", i, r_cmp[i],
             exp_cmp);
      return false;
    }
  }
  return true;
}

template <typename T> bool atomic_float_min_test(bool isOCL20) {
  printf("---------------------------------------\n");
  printf("atomic_float_min_test\n");
  printf("---------------------------------------\n");

  cl_int iRet = 0;

  const std::vector<T> src_a = {1.23f,
                                0.00023f,
                                213455444.3452f,
                                -23.12213f,
                                0.f,
                                -0.f,
                                std::numeric_limits<T>::max(),
                                std::numeric_limits<T>::min(),
                                std::numeric_limits<T>::infinity(),
                                std::numeric_limits<T>::quiet_NaN()};
  const std::vector<T> src_b = {
      56.23f, 0.00621f, 0.0000023f, 245.345f,
      10.f,   10.f,     1.f,        -std::numeric_limits<T>::min(),
      1.f,    1.f};

  std::vector<T> r_old(src_a.size());
  std::vector<T> r_cmp(src_a.size());

  iRet = check_atomic_min(isOCL20, src_a, src_b, r_old, r_cmp);
  CheckException("check_atomic_min", CL_SUCCESS, iRet);

  if (!check_min_results(src_a, src_b, r_old, r_cmp)) {
    printf("Results differ from expected in check_atomic_min\n");
    return false;
  }

  return true;
}

template <typename T>
cl_int check_atomic_max(bool isOCL20, const std::vector<T> &src_a,
                        const std::vector<T> &src_b, std::vector<T> &r_old,
                        std::vector<T> &r_cmp) {
  cl_int iRet = 0;
  cl_device_id device = NULL;
  cl_context context;
  cl_platform_id platform = 0;
  const size_t num = src_a.size();

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

  const char *kernelString =
      isOCL20 ? OCL20_KERNEL_MAX_TEST_CODE_STR : OCL12_KERNEL_MAX_TEST_CODE_STR;
  cl_program prog = clCreateProgramWithSource(
      context, 1, (const char **)&kernelString, NULL, &iRet);
  CheckException("clCreateProgramWithSource", CL_SUCCESS, iRet);

  std::string options;
  if (isOCL20) {
    options = "-cl-std=CL2.0";
    options += (is_same<T, double>::value) ? " -D T=double -D AT=atomic_double"
                                           : " -D T=float -D AT=atomic_float";
  } else {
    options = "-cl-std=CL1.2";
    options += (is_same<T, double>::value) ? " -D T=double" : " -D T=float";
  }
  options += " -D float_atomics_enable";

  printf("Building program with options %s\n", options.c_str());
  iRet = clBuildProgram(prog, 1, &device, options.c_str(), NULL, NULL);
  if (iRet != CL_SUCCESS) {
    char buildLog[2048];
    clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog),
                          buildLog, NULL);
    printf("Build Failed, log:\n %s\n", buildLog);
  }
  CheckException("clBuildProgram", CL_SUCCESS, iRet);

  cl_kernel kernel = clCreateKernel(prog, "test_atomic_max_f", &iRet);
  CheckException("clCreateKernel", CL_SUCCESS, iRet);

  cl_mem k_a =
      clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(T) * num, NULL, &iRet);
  CheckException("clCreateBuffer src a", CL_SUCCESS, iRet);
  cl_mem k_b =
      clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(T) * num, NULL, &iRet);
  CheckException("clCreateBuffer src b", CL_SUCCESS, iRet);
  cl_mem k_o =
      clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(T) * num, NULL, &iRet);
  CheckException("clCreateBuffer res old", CL_SUCCESS, iRet);
  cl_mem k_s =
      clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(T) * num, NULL, &iRet);
  CheckException("clCreateBuffer res cmp", CL_SUCCESS, iRet);

  iRet = clEnqueueWriteBuffer(queue, k_a, CL_TRUE, 0, sizeof(T) * num,
                              &src_a[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer src a", CL_SUCCESS, iRet);
  iRet = clEnqueueWriteBuffer(queue, k_b, CL_TRUE, 0, sizeof(T) * num,
                              &src_b[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer src b", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &k_a);
  CheckException("clSetKernelArg src a", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), &k_b);
  CheckException("clSetKernelArg src b", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 2, sizeof(cl_mem), &k_o);
  CheckException("clSetKernelArg res old", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 3, sizeof(cl_mem), &k_s);
  CheckException("clSetKernelArg res cmp", CL_SUCCESS, iRet);

  size_t global = num;
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, NULL, 0, NULL,
                                NULL);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

  iRet = clEnqueueReadBuffer(queue, k_o, CL_TRUE, 0, sizeof(T) * num, &r_old[0],
                             0, NULL, NULL);
  CheckException("clEnqueueReadBuffer res old", CL_SUCCESS, iRet);
  iRet = clEnqueueReadBuffer(queue, k_s, CL_TRUE, 0, sizeof(T) * num, &r_cmp[0],
                             0, NULL, NULL);
  CheckException("clEnqueueReadBuffer res cmp", CL_SUCCESS, iRet);

  clReleaseMemObject(k_a);
  clReleaseMemObject(k_b);
  clReleaseMemObject(k_o);
  clReleaseMemObject(k_s);
  clReleaseCommandQueue(queue);
  clReleaseKernel(kernel);
  clReleaseProgram(prog);
  clReleaseContext(context);

  return iRet;
}

template <typename T>
bool check_max_results(const std::vector<T> &src_a, const std::vector<T> &src_b,
                       const std::vector<T> &r_old,
                       const std::vector<T> &r_cmp) {
  for (size_t i = 0; i < src_a.size(); i++) {
    if (src_a[i] != r_old[i] &&
        !(std::isnan(src_a[i]) && std::isnan(r_old[i]))) {
      printf("Old value error at %zu: got %f, expected %f\n", i, r_old[i],
             src_a[i]);
      return false;
    }

    T exp_cmp = src_a[i] > src_b[i] ? src_a[i] : src_b[i];
    if (exp_cmp != r_cmp[i] && !(std::isnan(exp_cmp) && std::isnan(r_cmp[i]))) {
      printf("Comparison error at %zu: got %f, expected %f\n", i, r_cmp[i],
             exp_cmp);
      return false;
    }
  }
  return true;
}

template <typename T> bool atomic_float_max_test(bool isOCL20) {
  printf("---------------------------------------\n");
  printf("atomic_float_max_test\n");
  printf("---------------------------------------\n");

  cl_int iRet = 0;

  const std::vector<T> src_a = {1.23f,
                                0.00023f,
                                213455444.3452f,
                                -23.12213f,
                                0.f,
                                -0.f,
                                std::numeric_limits<T>::max(),
                                std::numeric_limits<T>::min(),
                                std::numeric_limits<T>::infinity(),
                                std::numeric_limits<T>::quiet_NaN()};
  const std::vector<T> src_b = {
      56.23f, 0.00621f, 0.0000023f, 245.345f,
      10.f,   10.f,     1.f,        -std::numeric_limits<T>::min(),
      1.f,    1.f};

  std::vector<T> r_old(src_a.size());
  std::vector<T> r_cmp(src_a.size());

  iRet = check_atomic_max(isOCL20, src_a, src_b, r_old, r_cmp);
  CheckException("check_atomic_max", CL_SUCCESS, iRet);

  if (!check_max_results(src_a, src_b, r_old, r_cmp)) {
    printf("Results differ from expected in check_atomic_max\n");
    return false;
  }

  return true;
}

bool atomic_min_max_float_test() {
  return (atomic_float_min_test<float>(false) &&
          atomic_float_min_test<double>(false) &&
          atomic_float_max_test<float>(false) &&
          atomic_float_max_test<double>(false) &&
          atomic_float_min_test<float>(true) &&
          atomic_float_min_test<double>(true) &&
          atomic_float_max_test<float>(true) &&
          atomic_float_max_test<double>(true));
}
