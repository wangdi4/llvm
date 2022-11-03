
#include "cl_config.h"
#include "common_utils.h"
#include "gtest_wrapper.h"

TEST(FrameworkTestType, ConfigEnvSize) {
#ifndef _WIN32
  const char *devNull = "/dev/null";
#else
  const char *devNull = "nul";
#endif

  const char *env = nullptr;
  size_t expected = 0;

  if (sizeof(size_t) == 8) {
    env = "8GB";
    expected = 8LL * 1024 * 1024 * 1024;
  } else {
    env = "3GB";
    expected = 3LL * 1024 * 1024 * 1024;
  }

  SETENV("CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE", env);
  Intel::OpenCL::Utils::BasicCLConfigWrapper Config;
  Config.Initialize(devNull);
  ASSERT_EQ(expected, Config.GetForcedLocalMemSize());
}
