#include "CL/cl.h"
#include "FrameworkTest.h"
#include "gtest_wrapper.h"
#include <string>

extern cl_device_type gDeviceType;

void clSetKernelArgInvalidArgSizeTest() {
  const char *programSource = "\n\
__kernel void k1(int a, __global int *b, sampler_t c, __local int *d) \n\
{ }                              \n\
        ";

  cl_int iRet = CL_SUCCESS;
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_program program;
  cl_kernel kernel;

  iRet = clGetPlatformIDs(/*num_entries=*/1, &platform,
                          /*num_platforms=*/nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clGetPlatformIDs failed.";

  iRet = clGetDeviceIDs(platform, gDeviceType, /*num_entries=*/1, &device,
                        /*num_devices=*/nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clGetDeviceIDs failed on trying to obtain "
                              << gDeviceType << " device type.";

  const cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                         (cl_context_properties)platform, 0};

  context = clCreateContext(prop,
                            /*num_devices=*/1, &device,
                            /*pfn_notify=*/nullptr,
                            /*user_data=*/nullptr, &iRet);

  ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateContext failed.";

  program = clCreateProgramWithSource(context, /*count=*/1, &programSource,
                                      /*length=*/nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateProgramWithSource failed.";

  iRet = clBuildProgram(program, /*num_devices=*/0, /*device_list=*/nullptr,
                        /*options=*/"", /*pfn_notify*/ nullptr,
                        /*user_data=*/nullptr);
  if (CL_SUCCESS != iRet) {
    size_t logSize = 0;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                          /*param_value_size=*/0, /*param_value=*/nullptr,
                          &logSize);
    std::string log("", logSize);
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log.size(),
                          &log[0], /*value_size_ret=*/nullptr);

    FAIL() << "Build failed\n"
           << "Source:\n"
           << "---------\n"
           << programSource << "---------\n"
           << log;
  }

  kernel = clCreateKernel(program, "k1", &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateKernel failed.";

  /*if arg_size does not match the size of the data type for an argument
   *that is not a memory object */
  iRet = clSetKernelArg(kernel, 0, 1, nullptr);
  ASSERT_EQ(iRet, CL_INVALID_ARG_SIZE);

  /*if the argument is a memory object and arg_size != sizeof(cl_mem) */
  cl_mem my_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * 32,
                                    nullptr, nullptr);
  iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem) * 2, &my_buffer);
  ASSERT_EQ(iRet, CL_INVALID_ARG_SIZE);

  iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem) * 2, nullptr);
  ASSERT_EQ(iRet, CL_INVALID_ARG_SIZE);

  /*if arg_size is zero and the argument is declared with
   *the __local qualifier */
  iRet = clSetKernelArg(kernel, 3, 0, nullptr);
  ASSERT_EQ(iRet, CL_INVALID_ARG_SIZE);

  /*if the argument is a sampler and arg_size != sizeof(cl_sampler)*/
  cl_sampler_properties samplerProps[7] = {CL_SAMPLER_NORMALIZED_COORDS,
                                           CL_FALSE,
                                           CL_SAMPLER_ADDRESSING_MODE,
                                           CL_ADDRESS_MIRRORED_REPEAT,
                                           CL_SAMPLER_FILTER_MODE,
                                           CL_FILTER_LINEAR,
                                           0};
  cl_sampler my_sampler =
      clCreateSamplerWithProperties(context, samplerProps, &iRet);
  ASSERT_EQ(iRet, CL_SUCCESS);
  iRet = clSetKernelArg(kernel, 2, sizeof(cl_sampler) * 2, &my_sampler);
  ASSERT_EQ(iRet, CL_INVALID_ARG_SIZE);

  clReleaseMemObject(my_buffer);
  clReleaseSampler(my_sampler);

  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseContext(context);
}
