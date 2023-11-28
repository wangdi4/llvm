#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include "common_utils.h"
#include "gtest_wrapper.h"
#include <fstream>
#include <stdio.h>

extern cl_device_type gDeviceType;

// The test is temporary disabled until
// https://git-amr-2.devtools.intel.com/gerrit/#/c/181490/ is merged.
class DISABLED_CheckSamplerTypedefInSPIRV : public ::testing::Test {
protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;

  void SetUp() override {
    SETENV("OCL_INTERMEDIATE", "SPIRV");
    m_platform = 0;
    cl_int rc = CL_SUCCESS;
    rc = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_EQ(CL_SUCCESS, rc) << "Unable to get platform";

    rc = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_EQ(CL_SUCCESS, rc) << "Unable to get device";

    cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                     (cl_context_properties)m_platform, 0};
    m_context = clCreateContext(prop, 1, &m_device, NULL, NULL, &rc);
    ASSERT_EQ(CL_SUCCESS, rc) << "Unable to create context";
  }

  void TearDown() override {
    cl_int rc = clReleaseContext(m_context);
    ASSERT_EQ(CL_SUCCESS, rc) << "Unable to release context";
    rc = clReleaseDevice(m_device);
    ASSERT_EQ(CL_SUCCESS, rc) << "Unable to release device";
    UNSETENV("OCL_INTERMEDIATE");
  }
};

// The test checks that OCL RT is able to recognize sampler typedef kernel
// argument and set it
TEST_F(DISABLED_CheckSamplerTypedefInSPIRV, Run) {
  cl_program m_program;
  cl_kernel m_kernel;
  cl_sampler m_sampler;
  cl_int rc = CL_SUCCESS;
  std::string program_sources = "                                          \n\
  typedef sampler_t SAMPLER;                                               \n\
  __kernel void smplr(SAMPLER sampler) {                                   \n\
    sampler_t s = sampler;                                                 \n\
  }                                                                        \n\
  ";
  const char *sources_str = program_sources.c_str();
  m_program = clCreateProgramWithSource(m_context, /*count=*/1, &(sources_str),
                                        /*length=*/nullptr, &rc);
  ASSERT_EQ(CL_SUCCESS, rc) << "Unable to create program";

  rc = clBuildProgram(m_program, 0, nullptr, nullptr, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, rc) << "Unable to build program";

  m_kernel = clCreateKernel(m_program, "smplr", &rc);
  ASSERT_EQ(CL_SUCCESS, rc) << "Unable to create kernel";

  cl_sampler_properties props[] = {CL_SAMPLER_NORMALIZED_COORDS,
                                   CL_FALSE,
                                   CL_SAMPLER_ADDRESSING_MODE,
                                   CL_ADDRESS_CLAMP,
                                   CL_SAMPLER_FILTER_MODE,
                                   CL_FILTER_NEAREST,
                                   0};
  m_sampler = clCreateSamplerWithProperties(m_context, props, &rc);
  ASSERT_EQ(CL_SUCCESS, rc) << "Unable to create kernel";

  rc = clSetKernelArg(m_kernel, 0, sizeof(cl_sampler), &m_sampler);
  ASSERT_EQ(CL_SUCCESS, rc) << "Unable to set kernel argument";
}
