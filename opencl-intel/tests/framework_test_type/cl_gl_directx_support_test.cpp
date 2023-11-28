//|
//|
//|
//| Purpose
//| -------
//|
//| Test that cl gl sharing and directX extensions are disabled if
// CL_CONFIG_GL_DIRECTX_INTEROP | environment variable is set to False, And
// enabled otherwise
//|
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include "CL/cl_ext.h"
#include "FrameworkTest.h"
#include "common_utils.h"
#include "gtest_wrapper.h"
#include <stdio.h>

#define CL_CONFIG_GL_DIRECTX_INTEROP "CL_CONFIG_GL_DIRECTX_INTEROP"

extern cl_device_type gDeviceType;

bool gl_directx_support_test(bool supported) {
  printf("---------------------------------------\n");
  printf("gl directx support test\nTest case: supported == ");
  printf(supported ? "true\n" : "false\n");
  printf("---------------------------------------\n");

  SETENV(CL_CONFIG_GL_DIRECTX_INTEROP, supported ? "True" : "False");

  bool bResult = true;
  cl_device_id device = NULL;
  cl_platform_id platform = NULL;
  cl_int err;

  // init platform
  err = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, err);

  // init Devices (only one CPU...)
  err = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, err);

  char *extStringDevice;
  size_t size = 0;
  err = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 0, NULL, &size);
  bResult &= SilentCheck("clGetDeviceInfo", CL_SUCCESS, err);

  extStringDevice = (char *)malloc(size);
  if (NULL == extStringDevice) {
    printf("Error: unable to allocate %zu byte buffer for extension string "
           "(err = %d)\n",
           size, err);
    return false;
  }
  err = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, size, extStringDevice,
                        NULL);
  bResult &= SilentCheck("clGetDeviceInfo", CL_SUCCESS, err);

  char *extStringPlatform;
  err = clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, 0, NULL, &size);
  bResult &= SilentCheck("clGetPlatformInfo", CL_SUCCESS, err);

  extStringPlatform = (char *)malloc(size);
  if (NULL == extStringPlatform) {
    printf("Error: unable to allocate %zu byte buffer for extension string "
           "(err = %d)\n",
           size, err);
    return false;
  }

  err = clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, size,
                          extStringPlatform, NULL);
  bResult &= SilentCheck("clGetPlatformInfo", CL_SUCCESS, err);

  if (supported) {
    if (!strstr(extStringDevice, "cl_khr_gl_sharing") ||
        !strstr(extStringDevice, "cl_intel_dx9_media_sharing") ||
        !strstr(extStringDevice, "cl_khr_dx9_media_sharing") ||
        !strstr(extStringDevice, "cl_khr_d3d11_sharing") ||
        !strstr(extStringPlatform, "cl_khr_gl_sharing") ||
        !strstr(extStringPlatform, "cl_intel_dx9_media_sharing") ||
        !strstr(extStringPlatform, "cl_khr_dx9_media_sharing") ||
        !strstr(extStringPlatform, "cl_khr_d3d11_sharing")) {
      printf("Error: extensions should be supported!\n");
      free(extStringPlatform);
      return false;
    }
  } else {
    if (strstr(extStringDevice, "cl_khr_gl_sharing") ||
        strstr(extStringDevice, "cl_intel_dx9_media_sharing") ||
        strstr(extStringDevice, "cl_khr_dx9_media_sharing") ||
        strstr(extStringDevice, "cl_khr_d3d11_sharing") ||
        strstr(extStringPlatform, "cl_khr_gl_sharing") ||
        strstr(extStringPlatform, "cl_intel_dx9_media_sharing") ||
        strstr(extStringPlatform, "cl_khr_dx9_media_sharing") ||
        strstr(extStringPlatform, "cl_khr_d3d11_sharing"))

    {
      printf("Error: extensions should not be supported!\n");
      free(extStringPlatform);
      return false;
    }
  }
  free(extStringPlatform);
  if (bResult) {
    printf("---------------------------------------\n");
    printf("gl directx support test passed\n");
    printf("---------------------------------------\n");
    exit(0);
  }

  return bResult;
}

#define death_test(t) death_test_imp(t)

void death_test_imp(bool supported) {
#ifdef DEBUGGING_DEATH_TEST
  EXPECT_TRUE(gl_directx_support_test(supported));
#else
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_EXIT(
      {
        gl_directx_support_test(supported);
        exit(1);
      },
      ::testing::ExitedWithCode(0), "");
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Do not remove DeathTest suffix from the test names - it is the Google Test
// requirement
//
////////////////////////////////////////////////////////////////////////////////////////////

#if (_WIN32)

TEST(FrameworkTestTypeDeathTest, Test_gl_directx_not_support) {
  death_test(false);
}

#endif // _WIN32
