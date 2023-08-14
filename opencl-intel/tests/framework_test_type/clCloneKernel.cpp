#include "CL/cl.h"
#include "FrameworkTest.h"
#include "gtest_wrapper.h"
#include "test_utils.h"
#include <iostream>
#include <string>
#include <vector>

extern cl_device_type gDeviceType;

const char *copy_arg[] = {
    "__kernel void copy_arg(uint IN, __global uint* OUT) { OUT[IN] = 1; }"};

void CloneKernel_RetErrors() {
  cl_int iRet = CL_SUCCESS;

  cl_kernel bad_source_kernel = (cl_kernel)(-1);
  clCloneKernel(bad_source_kernel, &iRet);
  CheckException("clCloneKernel with invalid kernel", CL_INVALID_KERNEL, iRet);

  clCloneKernel(NULL, &iRet);
  CheckException("clCloneKernel with invalid kernel", CL_INVALID_KERNEL, iRet);

  clCloneKernel(NULL, NULL);
}

void CloneKernel_SetArg(cl_context context, cl_kernel source_kernel,
                        cl_command_queue queue) {
  cl_int iRet = CL_SUCCESS;

  cl_kernel clone_kernel_1 = clCloneKernel(source_kernel, &iRet);
  CheckException("clCloneKernel", CL_SUCCESS, iRet);

  // Set kernel's args.
  cl_uint input_value_for_source = 0;
  cl_uint input_value_for_clone = 1;
  const size_t output_buffer_size = 2 * sizeof(cl_uint);
  cl_mem output_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                        output_buffer_size, NULL, &iRet);
  CheckException("clCreateBuffer", CL_SUCCESS, iRet);

  const cl_uint pattern = 0;
  iRet = clEnqueueFillBuffer(queue, output_buffer, &pattern, sizeof(pattern), 0,
                             output_buffer_size, 0, NULL, NULL);
  CheckException("clEnqueueFillBuffer", CL_SUCCESS, iRet);
  iRet = clFinish(queue);
  CheckException("clFinish", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(clone_kernel_1, 0, sizeof(input_value_for_source),
                        &input_value_for_source);
  CheckException("clSetKernelArg", CL_SUCCESS, iRet);

  iRet =
      clSetKernelArg(clone_kernel_1, 1, sizeof(output_buffer), &output_buffer);
  CheckException("clSetKernelArg", CL_SUCCESS, iRet);

  cl_kernel clone_kernel = clCloneKernel(clone_kernel_1, &iRet);
  CheckException("clCloneKernel", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(clone_kernel, 0, sizeof(input_value_for_clone),
                        &input_value_for_clone);
  CheckException("clSetKernelArg", CL_SUCCESS, iRet);

  const size_t global_size = 1;
  iRet = clEnqueueNDRangeKernel(queue, clone_kernel_1, 1, NULL, &global_size,
                                NULL, 0, NULL, NULL);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

  iRet = clFinish(queue);
  CheckException("clFinish", CL_SUCCESS, iRet);

  clReleaseKernel(clone_kernel_1);

  iRet = clEnqueueNDRangeKernel(queue, clone_kernel, 1, NULL, &global_size,
                                NULL, 0, NULL, NULL);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

  iRet = clFinish(queue);
  CheckException("clFinish", CL_SUCCESS, iRet);

  cl_uint *p_output_buffer = (cl_uint *)clEnqueueMapBuffer(
      queue, output_buffer, CL_TRUE, CL_MAP_READ, 0, output_buffer_size, 0,
      NULL, NULL, &iRet);
  CheckException("clEnqueueMapBuffer", CL_SUCCESS, iRet);

  // check that source kernel set 0 element and cloned kernel 1 element
  ASSERT_EQ((cl_uint)1, p_output_buffer[0])
      << "Expected 1 in p_output_buffer[0].";
  ASSERT_EQ((cl_uint)1, p_output_buffer[1])
      << "Expected 1 in p_output_buffer[1].";
}

void CloneKernel_RefCount(cl_context context, cl_kernel source_kernel,
                          cl_command_queue queue) {
  cl_int iRet = CL_SUCCESS;

  // Set kernel's args.
  const size_t output_buffer_size = 2 * sizeof(cl_uint);
  cl_mem output_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                        output_buffer_size, NULL, &iRet);
  CheckException("clCreateBuffer", CL_SUCCESS, iRet);

  iRet = clRetainKernel(source_kernel);
  CheckException("clRetainKernel", CL_SUCCESS, iRet);

  iRet =
      clSetKernelArg(source_kernel, 1, sizeof(output_buffer), &output_buffer);
  CheckException("clSetKernelArg", CL_SUCCESS, iRet);

  cl_kernel clone_kernel = clCloneKernel(source_kernel, &iRet);
  CheckException("clCloneKernel", CL_SUCCESS, iRet);

  // Check for ref count of source_kernel didn't change.
  cl_uint ref_count = 0;
  iRet = clGetKernelInfo(source_kernel, CL_KERNEL_REFERENCE_COUNT,
                         sizeof(ref_count), &ref_count, NULL);
  CheckException("clGetKernelInfo", CL_SUCCESS, iRet);
  ASSERT_EQ((cl_uint)2, ref_count)
      << "Ref count of original kernel has changed.";

  // Check for ref count of clone_kernel = 1.
  ref_count = 0;
  iRet = clGetKernelInfo(clone_kernel, CL_KERNEL_REFERENCE_COUNT,
                         sizeof(ref_count), &ref_count, NULL);
  CheckException("clGetKernelInfo", CL_SUCCESS, iRet);
  ASSERT_EQ((cl_uint)1, ref_count)
      << "Ref count of cloned kernel doesn't equal 1.";

  // Check for ref count of mem object passed to source_kernel didn't change.
  ref_count = 0;
  iRet = clGetMemObjectInfo(output_buffer, CL_MEM_REFERENCE_COUNT,
                            sizeof(ref_count), &ref_count, NULL);
  CheckException("clGetMemObjectInfo", CL_SUCCESS, iRet);
  ASSERT_EQ((cl_uint)1, ref_count)
      << "Ref count of buffer original and cloned kernel use doesn't equal 1.";
}

void CloneKernel_ExecInfo(cl_kernel source_kernel) {
  // I don't know way to check on CPU that svm buffers specified in
  // clSetKernelExecInfo works correctly with clCloneKernel. Just check that
  // call works for clone kernel.
  // TODO. Implement check if we start to support another devices.
  cl_int iRet = CL_SUCCESS;

  cl_kernel clone_kernel = clCloneKernel(source_kernel, &iRet);
  CheckException("clCloneKernel", CL_SUCCESS, iRet);

  cl_bool svm_fgs_support = CL_FALSE;
  iRet = clSetKernelExecInfo(clone_kernel,
                             CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM,
                             sizeof(svm_fgs_support), &svm_fgs_support);
  CheckException("clCloneKernel", CL_SUCCESS, iRet);
}

bool CloneKernel() {
  std::cout << "============================================================="
            << std::endl;
  std::cout << "CloneKernel" << std::endl;
  std::cout << "============================================================="
            << std::endl;

  cl_int iRet = CL_SUCCESS;

  // Get platform.
  cl_platform_id platform = 0;
  iRet = clGetPlatformIDs(1, &platform, NULL);
  CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);

  // Get device.
  cl_device_id device = NULL;
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

  // Create context.
  cl_context context = NULL;
  {
    const cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                           (cl_context_properties)platform, 0};
    context = clCreateContext(prop, 1, &device, NULL, NULL, &iRet);
    CheckException("clCreateContext(", CL_SUCCESS, iRet);
  }

  // Create and build program.
  cl_program program = 0;
  {
    size_t copy_arg_lengths[] = {strlen(copy_arg[0])};
    program = clCreateProgramWithSource(context, 1, copy_arg, copy_arg_lengths,
                                        &iRet);
    CheckException("clCreateProgramWithSource", CL_SUCCESS, iRet);

    iRet = clBuildProgram(program, 1, &device, "", NULL, NULL);
    if (CL_BUILD_PROGRAM_FAILURE == iRet) {
      std::string log(1000, ' ');
      iRet = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                   log.size(), &log[0], NULL);
      std::cout << log << std::endl;
    }
    CheckException("clBuildProgram", CL_SUCCESS, iRet);
  }

  // Create kernel.
  cl_kernel source_kernel = clCreateKernel(program, "copy_arg", &iRet);
  CheckException("clCreateKernel", CL_SUCCESS, iRet);

  // Create command queue.
  cl_command_queue queue =
      clCreateCommandQueueWithProperties(context, device, 0, &iRet);
  CheckException("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);

  CloneKernel_SetArg(context, source_kernel, queue);

  CloneKernel_RefCount(context, source_kernel, queue);

  CloneKernel_RetErrors();

  CloneKernel_ExecInfo(source_kernel);

  return true;
}
