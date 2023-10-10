#include "CL/cl.h"
#include "bi_tests.h"
#include "cl_types.h"
#include "test_utils.h"
#include <cstddef>

namespace {
const char *KERNEL_CODE_STR_GLOBAL_LOCAL =
    "__kernel void test_address_space_qualifier_func(__global int *src_a, "
    "__global int* src_b, __local int *local_src1, __global int *src_c, "
    "__global int *src_d, __local int *local_src2){"
    "int tid = get_global_id(0);"
    // infer global ptr, if can infer, src_a[tid] = -tid
    "__generic int* generic_ptr_a = src_a;"
    "if (to_local(generic_ptr_a) != NULL) {"
    "   __local int* local_ptr = to_local(generic_ptr_a);"
    "   local_ptr[tid] = -1;"
    "} else if(to_global(generic_ptr_a) != NULL) {"
    "   __global int* global_ptr = to_global(generic_ptr_a);"
    "   global_ptr[tid] = -tid;"
    "}"
    // infer local arg ptr, if can infer, src_b[tid] = tid
    "__generic int* generic_ptr_b = local_src1 + 4;"
    "if (to_local(generic_ptr_b) != NULL) {"
    "   src_b[tid] = tid;"
    "} else if(to_global(generic_ptr_b) != NULL) {"
    "   src_b[tid] = -1;"
    "}"
    "__generic int* generic_ptr_bb = local_src2;"
    "if (to_local(generic_ptr_bb) != NULL) {"
    "   src_b[tid] = src_b[tid] + tid;"
    "} else if(to_global(generic_ptr_bb) != NULL) {"
    "   src_b[tid] = -1;"
    "}"
    // infer local array ptr, if can infer, src_c[tid] = tid * 2
    "__local int array_ptr1[4];"
    "__local int array_ptr2[4];"
    "__generic int* generic_ptr_c = array_ptr1 + 2;"
    "if (to_local(generic_ptr_c) != NULL) {"
    "   src_c[tid] = tid;"
    "} else if(to_global(generic_ptr_c) != NULL) {"
    "   src_c[tid] = -1;"
    "}"
    "__generic int* generic_ptr_cc = array_ptr2 + 3;"
    "if (to_local(generic_ptr_cc) != NULL) {"
    "   src_c[tid] = src_c[tid] + tid;"
    "} else if(to_global(generic_ptr_cc) != NULL) {"
    "   src_c[tid] = -1;"
    "}"
    // infer private ptr, if can infer, src_c[tid] = -tid * 2
    "__generic int* generic_ptr_d = &tid;"
    "if (to_local(generic_ptr_d) != NULL) {"
    "   src_d[tid] = -1;"
    "} else if(to_global(generic_ptr_d) != NULL) {"
    "   src_d[tid] = -1;"
    "} else if(to_private(generic_ptr_d) != NULL){"
    "   src_d[tid] = - tid * 2;"
    "}"
    "}";
}

cl_device_type gCPUDeviceType = CL_DEVICE_TYPE_CPU;
int global_size = 32;
int local_size = 8;

cl_int check_assdress_space_qualifier(std::vector<int> &src_a,
                                      std::vector<int> &src_b,
                                      std::vector<int> &src_c,
                                      std::vector<int> &src_d,
                                      std::string &option) {
  cl_int iRet = 0;
  cl_device_id device = NULL;
  cl_context context;
  cl_platform_id platform = 0;
  const size_t num_a = src_a.size();
  const size_t num_b = src_b.size();
  const size_t num_c = src_c.size();
  const size_t num_d = src_d.size();

  iRet = clGetPlatformIDs(1, &platform, NULL);
  CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  iRet = clGetDeviceIDs(platform, gCPUDeviceType, 1, &device, NULL);
  CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

  context = clCreateContext(prop, 1, &device, NULL, NULL, &iRet);
  CheckException("clCreateContext", CL_SUCCESS, iRet);

  cl_command_queue queue =
      clCreateCommandQueueWithProperties(context, device, NULL, &iRet);
  CheckException("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);

  cl_program prog = clCreateProgramWithSource(
      context, 1, (const char **)&KERNEL_CODE_STR_GLOBAL_LOCAL, NULL, &iRet);
  CheckException("clCreateProgramWithSource", CL_SUCCESS, iRet);

  printf("Building program with options %s\n", option.c_str());
  iRet = clBuildProgram(prog, 1, &device, option.c_str(), NULL, NULL);
  if (iRet != CL_SUCCESS) {
    char buildLog[2048];
    clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog),
                          buildLog, NULL);
    printf("Build Failed, log:\n %s\n", buildLog);
  }
  CheckException("clBuildProgram", CL_SUCCESS, iRet);

  cl_kernel kernel =
      clCreateKernel(prog, "test_address_space_qualifier_func", &iRet);
  CheckException("clCreateKernel", CL_SUCCESS, iRet);

  cl_mem src_a_mem = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                    sizeof(int) * num_a, NULL, &iRet);
  CheckException("clCreateBuffer src_a_mem", CL_SUCCESS, iRet);
  cl_mem src_b_mem = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                    sizeof(int) * num_b, NULL, &iRet);
  CheckException("clCreateBuffer src_b_mem", CL_SUCCESS, iRet);
  cl_mem src_c_mem = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                    sizeof(int) * num_c, NULL, &iRet);
  CheckException("clCreateBuffer src_c_mem", CL_SUCCESS, iRet);
  cl_mem src_d_mem = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                    sizeof(int) * num_d, NULL, &iRet);
  CheckException("clCreateBuffer src_d_mem", CL_SUCCESS, iRet);

  iRet = clEnqueueWriteBuffer(queue, src_a_mem, CL_TRUE, 0, sizeof(int) * num_a,
                              &src_a[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer src a", CL_SUCCESS, iRet);
  iRet = clEnqueueWriteBuffer(queue, src_b_mem, CL_TRUE, 0, sizeof(int) * num_b,
                              &src_b[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer src b", CL_SUCCESS, iRet);
  iRet = clEnqueueWriteBuffer(queue, src_c_mem, CL_TRUE, 0, sizeof(int) * num_c,
                              &src_c[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer src c", CL_SUCCESS, iRet);
  iRet = clEnqueueWriteBuffer(queue, src_d_mem, CL_TRUE, 0, sizeof(int) * num_d,
                              &src_d[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer src c", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &src_a_mem);
  CheckException("clSetKernelArg src a", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), &src_b_mem);
  CheckException("clSetKernelArg src b", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 2, 256, nullptr);
  CheckException("clSetKernelArg local data", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 3, sizeof(cl_mem), &src_c_mem);
  CheckException("clSetKernelArg src c", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 4, sizeof(cl_mem), &src_d_mem);
  CheckException("clSetKernelArg src d", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 5, 256, nullptr);
  CheckException("clSetKernelArg local data", CL_SUCCESS, iRet);

  const size_t global = global_size;
  const size_t local = local_size;
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, &local, 0,
                                NULL, NULL);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

  iRet = clEnqueueReadBuffer(queue, src_a_mem, CL_TRUE, 0, sizeof(int) * num_a,
                             &src_a[0], 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer src_a_mem", CL_SUCCESS, iRet);
  iRet = clEnqueueReadBuffer(queue, src_b_mem, CL_TRUE, 0, sizeof(int) * num_b,
                             &src_b[0], 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer src_b_mem", CL_SUCCESS, iRet);
  iRet = clEnqueueReadBuffer(queue, src_c_mem, CL_TRUE, 0, sizeof(int) * num_c,
                             &src_c[0], 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer src_c_mem", CL_SUCCESS, iRet);
  iRet = clEnqueueReadBuffer(queue, src_d_mem, CL_TRUE, 0, sizeof(int) * num_d,
                             &src_d[0], 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer src_d_mem", CL_SUCCESS, iRet);

  clReleaseMemObject(src_a_mem);
  clReleaseMemObject(src_b_mem);
  clReleaseMemObject(src_c_mem);
  clReleaseCommandQueue(queue);
  clReleaseKernel(kernel);
  clReleaseProgram(prog);
  clReleaseContext(context);

  return iRet;
}

bool check_results(std::vector<int> &src_a, std::vector<int> &src_b,
                   std::vector<int> &src_c, std::vector<int> &src_d) {
  for (int i = 0; i < global_size; i++) {
    if (src_a[i] != -i)
      return false;
    if (src_b[i] != i * 2)
      return false;
    if (src_c[i] != i * 2)
      return false;
    if (src_d[i] != -i * 2)
      return false;
  }
  return true;
}

bool address_space_quailier_test() {
  printf("---------------------------------------\n");
  printf("address_space_quailier_test\n");
  printf("---------------------------------------\n");

  cl_int iRet = 0;

  std::vector<int> src_a(global_size, 0);
  std::vector<int> src_b(global_size, 0);
  std::vector<int> src_c(global_size, 0);
  std::vector<int> src_d(global_size, 0);

  std::vector<std::string> options = {"-cl-std=CL2.0", "-cl-std=CL2.0 -g",
                                      "-cl-std=CL2.0 -cl-opt-disable",
                                      "-cl-std=CL2.0 -cl-opt-disable -g"};
  for (auto &option : options) {
    iRet = check_assdress_space_qualifier(src_a, src_b, src_c, src_d, option);
    CheckException("check_assdress_space_qualifier", CL_SUCCESS, iRet);

    if (!check_results(src_a, src_b, src_c, src_d)) {
      printf(
          "Results differ from expected in check_assdress_space_qualifier\n");
      return false;
    }
  }

  return true;
}
