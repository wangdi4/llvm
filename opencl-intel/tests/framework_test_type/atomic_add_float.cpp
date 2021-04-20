#include "CL/cl.h"
#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "cl_types.h"

#include <cmath>
#include <limits>

/*******************************************************************************
 * Test checks that atomic_add in kernel code sums float values correctly.
 *******************************************************************************/

extern cl_device_type gDeviceType;
const int num = 32;

namespace {
const char *OCL12_KERNEL_ARIFM_TEST_CODE_STR =
    "T __attribute__((overloadable)) "
    "atomic_add(volatile __global T *p, T val); "

    "__kernel void test_atomic_add_f_arifm(__global T *src_a, "
    "__global T *src_b, "
    "__global T *r_old, "
    "__global T *r_sum) { "
    "int tid = get_global_id(0); "
    "r_sum[tid] = src_a[tid]; "
    "r_old[tid] = atomic_add(&(r_sum[tid]), src_b[tid]); "
    "}";

const char *OCL12_KERNEL_ATOM_TEST_CODE_STR =
    "T __attribute__((overloadable)) "
    "atomic_add(volatile __global T *p, T val); "

    "__kernel void test_atomic_add_f_atomicity(__global T *addend, "
    "__global T *r_sum) { "
    "atomic_add(r_sum, *addend); "
    "atomic_add(r_sum, *addend); "
    "atomic_add(r_sum, *addend); "
    "atomic_add(r_sum, *addend); "
    "}";

const char *OCL20_KERNEL_ARIFM_TEST_CODE_STR =
    "T __attribute__((overloadable)) "
    "atomic_fetch_add_explicit(volatile __global AT *p, T val, "
    "memory_order order, memory_scope scope); "
    "void __attribute__((overloadable)) "
    "atomic_store_explicit(volatile __global AT *p, T val, memory_order order);"

    "__kernel void test_atomic_add_f_arifm(__global T *src_a, "
    "__global T *src_b, "
    "__global T *r_old, "
    "__global AT *r_sum) { "
    "int tid = get_global_id(0);"
    "atomic_store_explicit(&(r_sum[tid]), src_a[tid], memory_order_relaxed);"
    "r_old[tid] = atomic_fetch_add_explicit(&(r_sum[tid]), src_b[tid], "
    "memory_order_relaxed, "
    "memory_scope_device); "
    "}";

const char *OCL20_KERNEL_ATOM_TEST_CODE_STR =
    "T __attribute__((overloadable)) "
    "atomic_fetch_add_explicit(volatile AT *p, T val, "
    "memory_order order, memory_scope scope); "

    "__kernel void test_atomic_add_f_atomicity(__global T *addend, "
    "__global AT *r_sum) { "
    "atomic_fetch_add_explicit(r_sum, *addend, memory_order_relaxed, "
    "memory_scope_device); "
    "atomic_fetch_add_explicit(r_sum, *addend, memory_order_relaxed, "
    "memory_scope_device); "
    "atomic_fetch_add_explicit(r_sum, *addend, memory_order_relaxed, "
    "memory_scope_device); "
    "atomic_fetch_add_explicit(r_sum, *addend, memory_order_relaxed, "
    "memory_scope_device); "
    "}";
} // namespace

template <typename T>
cl_int check_atomic_add_arifm(bool isOCL20, const std::vector<T> &src_a,
                              const std::vector<T> &src_b,
                              std::vector<T> &r_old, std::vector<T> &r_sum) {
  cl_int iRet = 0;
  cl_device_id device = NULL;
  cl_context context;
  cl_platform_id platform = 0;
  const size_t num = src_a.size();

  // TODO: to make the test robust, the assumption should be removed: device 1
  // is CPU device.
  iRet = clGetPlatformIDs(1, &platform, NULL);
  CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

  context = clCreateContext(prop, 1, &device, NULL, NULL, &iRet);
  CheckException("clCreateContext", CL_SUCCESS, iRet);

  cl_command_queue queue = clCreateCommandQueue(context, device, 0, &iRet);
  CheckException("clCreateCommandQueue", CL_SUCCESS, iRet);

  const char *kernelString = isOCL20 ? OCL20_KERNEL_ARIFM_TEST_CODE_STR
                                     : OCL12_KERNEL_ARIFM_TEST_CODE_STR;
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
  printf("Building program with options %s\n", options.c_str());
  iRet = clBuildProgram(prog, 1, &device, options.c_str(), NULL, NULL);
  if (iRet != CL_SUCCESS) {
    char buildLog[2048];
    clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog),
                          buildLog, NULL);
    printf("Build Failed, log:\n %s\n", buildLog);
  }
  CheckException("clBuildProgram", CL_SUCCESS, iRet);

  cl_kernel kernel = clCreateKernel(prog, "test_atomic_add_f_arifm", &iRet);
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
  CheckException("clCreateBuffer res sum", CL_SUCCESS, iRet);

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
  CheckException("clSetKernelArg res sum", CL_SUCCESS, iRet);

  size_t global = num;
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, NULL, 0, NULL,
                                NULL);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

  iRet = clEnqueueReadBuffer(queue, k_o, CL_TRUE, 0, sizeof(T) * num, &r_old[0],
                             0, NULL, NULL);
  CheckException("clEnqueueReadBuffer res old", CL_SUCCESS, iRet);
  iRet = clEnqueueReadBuffer(queue, k_s, CL_TRUE, 0, sizeof(T) * num, &r_sum[0],
                             0, NULL, NULL);
  CheckException("clEnqueueReadBuffer res sum", CL_SUCCESS, iRet);

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
bool check_arifm_results(const std::vector<T> &src_a,
                         const std::vector<T> &src_b,
                         const std::vector<T> &r_old,
                         const std::vector<T> &r_sum) {
  for (size_t i = 0; i < src_a.size(); i++) {
    if (src_a[i] != r_old[i] &&
        !(std::isnan(src_a[i]) && std::isnan(r_old[i]))) {
      printf("Old value error at %zu: got %f, expected %f\n", i, r_old[i],
             src_a[i]);
      return false;
    }

    T exp_sum = src_a[i] + src_b[i];
    if (exp_sum != r_sum[i] && !(std::isnan(exp_sum) && std::isnan(r_sum[i]))) {
      printf("Addition error at %zu: got %f, expected %f\n", i, r_sum[i],
             exp_sum);
      return false;
    }
  }
  return true;
}

template <typename T> bool atomic_add_float_arifm_test(bool isOCL20) {
  printf("---------------------------------------\n");
  printf("atomic_add_float_arifm_test (OpenCL %s)\n", isOCL20 ? "2.0" : "1.2");
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
  std::vector<T> r_sum(src_a.size());

  iRet = check_atomic_add_arifm(isOCL20, src_a, src_b, r_old, r_sum);
  CheckException("check_atomic_add_arifm", CL_SUCCESS, iRet);

  if (!check_arifm_results(src_a, src_b, r_old, r_sum)) {
    printf("Results differ from expected in check_atomic_add_arifm\n");
    return false;
  }

  return true;
}

template <typename T>
cl_int check_atomic_add_atomicity(bool isOCL20, size_t global_ws,
                                  size_t local_ws, T augend, T addend,
                                  T *r_sum) {
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

  cl_command_queue queue = clCreateCommandQueue(context, device, 0, &iRet);
  CheckException("clCreateCommandQueue", CL_SUCCESS, iRet);

  const char *kernelString = isOCL20 ? OCL20_KERNEL_ATOM_TEST_CODE_STR
                                     : OCL12_KERNEL_ATOM_TEST_CODE_STR;
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
  printf("Building program with options %s\n", options.c_str());
  iRet = clBuildProgram(prog, 1, &device, options.c_str(), NULL, NULL);
  if (iRet != CL_SUCCESS) {
    char buildLog[2048];
    clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog),
                          buildLog, NULL);
    printf("Build Failed, log:\n %s\n", buildLog);
  }
  CheckException("clBuildProgram", CL_SUCCESS, iRet);

  cl_kernel kernel = clCreateKernel(prog, "test_atomic_add_f_atomicity", &iRet);
  CheckException("clCreateKernel", CL_SUCCESS, iRet);

  cl_mem k_a =
      clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(T), NULL, &iRet);
  CheckException("clCreateBuffer addend", CL_SUCCESS, iRet);
  cl_mem k_s =
      clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(T), NULL, &iRet);
  CheckException("clCreateBuffer res", CL_SUCCESS, iRet);

  iRet = clEnqueueWriteBuffer(queue, k_a, CL_TRUE, 0, sizeof(T), &addend, 0,
                              NULL, NULL);
  CheckException("clEnqueueWriteBuffer addend", CL_SUCCESS, iRet);
  iRet = clEnqueueWriteBuffer(queue, k_s, CL_TRUE, 0, sizeof(T), &augend, 0,
                              NULL, NULL);
  CheckException("clEnqueueWriteBuffer augend", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &k_a);
  CheckException("clSetKernelArg addend", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), &k_s);
  CheckException("clSetKernelArg res sum", CL_SUCCESS, iRet);

  size_t global = global_ws;
  size_t local = local_ws;
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, &local, 0,
                                NULL, NULL);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

  iRet = clEnqueueReadBuffer(queue, k_s, CL_TRUE, 0, sizeof(T), r_sum, 0, NULL,
                             NULL);
  CheckException("clEnqueueReadBuffer res sum", CL_SUCCESS, iRet);

  clReleaseMemObject(k_a);
  clReleaseMemObject(k_s);
  clReleaseCommandQueue(queue);
  clReleaseKernel(kernel);
  clReleaseProgram(prog);
  clReleaseContext(context);

  return iRet;
}

template <typename T>
bool check_atomicity_results(size_t global_ws, T augend, T addend, T r_sum) {
  T exp_sum = augend;
  for (size_t i = 0; i < global_ws; i++) {
    for (size_t j = 0; j < 4; j++) {
      exp_sum += addend;
    }
  }

  if (exp_sum != r_sum && !(std::isnan(exp_sum) && std::isnan(r_sum))) {
    printf("Atomic addition error: got %f, expected %f\n", r_sum, exp_sum);
    return false;
  }

  return true;
}

template <typename T> bool atomic_add_float_atomicity_test(bool isOCL20) {
  printf("---------------------------------------\n");
  printf("atomic_add_float_atomicity_test (OpenCL %s)\n",
         isOCL20 ? "2.0" : "1.2");
  printf("---------------------------------------\n");

  cl_int iRet = 0;

  size_t global_ws = 64;
  size_t local_ws = 8;
  T augend = 12345.6789f;
  T addend = 0.98765f;
  T r_sum{};

  iRet = check_atomic_add_atomicity(isOCL20, global_ws, local_ws, augend,
                                    addend, &r_sum);
  CheckException("check_atomic_add_atomicity", CL_SUCCESS, iRet);

  if (!check_atomicity_results(global_ws, augend, addend, r_sum)) {
    printf("Results differ from expected in check_atomic_add_atomicity\n");
    return false;
  }

  return true;
}

bool atomic_add_float_test() {
  return (atomic_add_float_arifm_test<float>(false) &&
          atomic_add_float_arifm_test<double>(false) &&
          atomic_add_float_atomicity_test<float>(false) &&
          atomic_add_float_atomicity_test<double>(false) &&
          atomic_add_float_arifm_test<float>(true) &&
          atomic_add_float_arifm_test<double>(true) &&
          atomic_add_float_atomicity_test<float>(true) &&
          atomic_add_float_atomicity_test<double>(true));
}
