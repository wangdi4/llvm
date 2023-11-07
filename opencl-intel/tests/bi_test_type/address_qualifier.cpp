#include "CL/cl.h"
#include "bi_tests.h"
#include "cl_types.h"
#include "test_utils.h"
#include <cstddef>

namespace {
const char *KERNEL_CODE_STR_GLOBAL_LOCAL =
    // global variable
    "__global int gint = 1;\n"
    "__global int gint_array[4] = {1,2,3,4};\n"
    "__global int gint_arrayDim2[2][4] = {{1,2,3,4}, {1,2,3,4}};\n"
    "__global int4 gint_arrayDim2Int4[2][4] = {{{1,2,3,4}, {1,2,3,4}, "
    "{1,2,3,4}, {1,2,3,4}}, {{1,2,3,4}, {1,2,3,4}, {1,2,3,4}, {1,2,3,4}}};\n"
    // global struct variable
    "typedef struct {"
    "float x;"
    "int y;"
    "short z;"
    "} my_struct;\n"
    "__global const my_struct test1 = {1,2,3};\n"
    "__global my_struct gint_arrayStruct[2] = {test1, test1};\n"

    "__kernel void test_address_space_qualifier_func(__global int *data_a, "
    "__global int* data_b, __local int *local_data1, __global int *data_c, "
    "__global int *data_d, __local int *local_data2, __global int *data_gv){"
    "int tid = get_global_id(0);"
    "__local int array_ptr1[4];"
    "__local int array_ptr2[4];"
    // infer global ptr, if can infer, data_a[tid] = tid
    "__generic int* generic_ptr_a = data_a;"
    "if (to_local(generic_ptr_a) != NULL) {"
    "   __local int* local_ptr = to_local(generic_ptr_a);"
    "   local_ptr[tid] = -1;"
    "} else if(to_global(generic_ptr_a) != NULL) {"
    "   __global int* global_ptr = to_global(generic_ptr_a);"
    "   global_ptr[tid] = tid;"
    "}"
    // infer local argument ptr, if can infer, data_b[tid] = 2 * tid
    "__generic int* generic_ptr_b = local_data1 + 4;"
    "if (to_local(generic_ptr_b) != NULL) {"
    "   data_b[tid] = tid;"
    "} else if(to_global(generic_ptr_b) != NULL) {"
    "   data_b[tid] = -1;"
    "}"
    "__generic int* generic_ptr_bb = local_data2;"
    "if (to_local(generic_ptr_bb) != NULL) {"
    "   data_b[tid] = data_b[tid] + tid;"
    "} else if(to_global(generic_ptr_bb) != NULL) {"
    "   data_b[tid] = -1;"
    "}"
    // infer local array ptr, if can infer, data_c[tid] = tid * 2
    "__generic int* generic_ptr_c = array_ptr1 + 2;"
    "if (to_local(generic_ptr_c) != NULL) {"
    "   data_c[tid] = tid;"
    "} else if(to_global(generic_ptr_c) != NULL) {"
    "   data_c[tid] = -1;"
    "}"
    "__generic int* generic_ptr_cc = array_ptr2 + 3;"
    "if (to_local(generic_ptr_cc) != NULL) {"
    "   data_c[tid] = data_c[tid] + tid;"
    "} else if(to_global(generic_ptr_cc) != NULL) {"
    "   data_c[tid] = -1;"
    "}"
    // infer private ptr, if can infer, data_c[tid] = tid * 2
    "__generic int* generic_ptr_d = &tid;"
    "if (to_local(generic_ptr_d) != NULL) {"
    "   data_d[tid] = -1;"
    "} else if(to_global(generic_ptr_d) != NULL) {"
    "   data_d[tid] = -1;"
    "} else if(to_private(generic_ptr_d) != NULL){"
    "   data_d[tid] = tid * 2;"
    "}"
    // infer global variable, if can infer, data_gv = 1
    "__generic int* generic_ptr_gv1 = &gint;"
    "if (to_global(generic_ptr_gv1) != NULL)"
    "   data_gv[0] = 1;"
    "__generic int* generic_ptr_gv2 = gint_array;"
    "if (to_global(generic_ptr_gv2) != NULL)"
    "   data_gv[1] = 1;"
    "__generic void* generic_ptr_gv3 = gint_arrayDim2;"
    "if (to_global(generic_ptr_gv3) != NULL)"
    "   data_gv[2] = 1;"
    "__generic void* generic_ptr_gv4 = gint_arrayDim2Int4;"
    "if (to_global(generic_ptr_gv4) != NULL)"
    "   data_gv[3] = 1;"
    "__generic void* generic_ptr_gv5 = gint_arrayStruct;"
    "if (to_global(generic_ptr_gv5) != NULL)"
    "   data_gv[4] = 1;"
    "}";
}

cl_device_type gCPUDeviceType = CL_DEVICE_TYPE_CPU;
int global_size = 32;
int local_size = 16;

cl_int check_assdress_space_qualifier(std::vector<int> &data_a,
                                      std::vector<int> &data_b,
                                      std::vector<int> &data_c,
                                      std::vector<int> &data_d,
                                      std::vector<int> &data_gv,
                                      std::string &option) {
  cl_int iRet = 0;
  cl_device_id device = NULL;
  cl_context context;
  cl_platform_id platform = 0;
  const size_t num_a = data_a.size();
  const size_t num_b = data_b.size();
  const size_t num_c = data_c.size();
  const size_t num_d = data_d.size();
  const size_t num_gv = data_gv.size();

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

  cl_mem data_a_mem = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                     sizeof(int) * num_a, NULL, &iRet);
  CheckException("clCreateBuffer data_a_mem", CL_SUCCESS, iRet);
  cl_mem data_b_mem = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                     sizeof(int) * num_b, NULL, &iRet);
  CheckException("clCreateBuffer data_b_mem", CL_SUCCESS, iRet);
  cl_mem data_c_mem = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                     sizeof(int) * num_c, NULL, &iRet);
  CheckException("clCreateBuffer data_c_mem", CL_SUCCESS, iRet);
  cl_mem data_d_mem = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                     sizeof(int) * num_d, NULL, &iRet);
  CheckException("clCreateBuffer data_d_mem", CL_SUCCESS, iRet);
  cl_mem data_gv_mem = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                      sizeof(int) * num_gv, NULL, &iRet);
  CheckException("clCreateBuffer data_d_mem", CL_SUCCESS, iRet);

  iRet = clEnqueueWriteBuffer(queue, data_a_mem, CL_TRUE, 0,
                              sizeof(int) * num_a, &data_a[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer data a", CL_SUCCESS, iRet);
  iRet = clEnqueueWriteBuffer(queue, data_b_mem, CL_TRUE, 0,
                              sizeof(int) * num_b, &data_b[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer data b", CL_SUCCESS, iRet);
  iRet = clEnqueueWriteBuffer(queue, data_c_mem, CL_TRUE, 0,
                              sizeof(int) * num_c, &data_c[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer data c", CL_SUCCESS, iRet);
  iRet = clEnqueueWriteBuffer(queue, data_d_mem, CL_TRUE, 0,
                              sizeof(int) * num_d, &data_d[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer data c", CL_SUCCESS, iRet);
  iRet = clEnqueueWriteBuffer(queue, data_gv_mem, CL_TRUE, 0,
                              sizeof(int) * num_gv, &data_gv[0], 0, NULL, NULL);
  CheckException("clEnqueueWriteBuffer data c", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_a_mem);
  CheckException("clSetKernelArg data a", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), &data_b_mem);
  CheckException("clSetKernelArg data b", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 2, 256, nullptr);
  CheckException("clSetKernelArg local data", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 3, sizeof(cl_mem), &data_c_mem);
  CheckException("clSetKernelArg data c", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 4, sizeof(cl_mem), &data_d_mem);
  CheckException("clSetKernelArg data d", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 5, 256, nullptr);
  CheckException("clSetKernelArg local data", CL_SUCCESS, iRet);
  CheckException("clSetKernelArg data c", CL_SUCCESS, iRet);
  iRet = clSetKernelArg(kernel, 6, sizeof(cl_mem), &data_gv_mem);

  const size_t global = global_size;
  const size_t local = local_size;
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, &local, 0,
                                NULL, NULL);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

  iRet = clEnqueueReadBuffer(queue, data_a_mem, CL_TRUE, 0, sizeof(int) * num_a,
                             &data_a[0], 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer data_a_mem", CL_SUCCESS, iRet);
  iRet = clEnqueueReadBuffer(queue, data_b_mem, CL_TRUE, 0, sizeof(int) * num_b,
                             &data_b[0], 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer data_b_mem", CL_SUCCESS, iRet);
  iRet = clEnqueueReadBuffer(queue, data_c_mem, CL_TRUE, 0, sizeof(int) * num_c,
                             &data_c[0], 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer data_c_mem", CL_SUCCESS, iRet);
  iRet = clEnqueueReadBuffer(queue, data_d_mem, CL_TRUE, 0, sizeof(int) * num_d,
                             &data_d[0], 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer data_d_mem", CL_SUCCESS, iRet);
  iRet = clEnqueueReadBuffer(queue, data_gv_mem, CL_TRUE, 0,
                             sizeof(int) * num_gv, &data_gv[0], 0, NULL, NULL);
  CheckException("clEnqueueReadBuffer data_d_mem", CL_SUCCESS, iRet);

  clReleaseMemObject(data_a_mem);
  clReleaseMemObject(data_b_mem);
  clReleaseMemObject(data_c_mem);
  clReleaseCommandQueue(queue);
  clReleaseKernel(kernel);
  clReleaseProgram(prog);
  clReleaseContext(context);

  return iRet;
}

bool check_results(std::vector<int> &data_a, std::vector<int> &data_b,
                   std::vector<int> &data_c, std::vector<int> &data_d,
                   std::vector<int> &data_gv) {
  for (int i = 0; i < global_size; i++) {
    if (data_a[i] != i)
      return false;
    if (data_b[i] != i * 2)
      return false;
    if (data_c[i] != i * 2)
      return false;
    if (data_d[i] != i * 2)
      return false;
  }
  for (int i = 0; i < 5; i++)
    if (data_gv[i] != 1)
      return false;
  return true;
}

bool address_space_quailier_test() {
  printf("---------------------------------------\n");
  printf("address_space_quailier_test\n");
  printf("---------------------------------------\n");

  cl_int iRet = 0;

  std::vector<int> data_a(global_size, 0);
  std::vector<int> data_b(global_size, 0);
  std::vector<int> data_c(global_size, 0);
  std::vector<int> data_d(global_size, 0);
  std::vector<int> data_gv(5, 0);

  std::vector<std::string> options = {"-cl-std=CL2.0", "-cl-std=CL2.0 -g",
                                      "-cl-std=CL2.0 -cl-opt-disable",
                                      "-cl-std=CL2.0 -cl-opt-disable -g"};
  for (auto &option : options) {
    iRet = check_assdress_space_qualifier(data_a, data_b, data_c, data_d,
                                          data_gv, option);
    CheckException("check_assdress_space_qualifier", CL_SUCCESS, iRet);

    if (!check_results(data_a, data_b, data_c, data_d, data_gv)) {
      printf(
          "Results differ from expected in check_assdress_space_qualifier\n");
      return false;
    }
  }

  return true;
}
