#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_cpu_detect.h"
#include "cl_device_api.h"
#include "cl_env.h"
#include "cl_types.h"
#include "common_utils.h"
#include "gtest_wrapper.h"
#include <stdio.h>
#include <string>

// #define DEBUGGING_DEATH_TEST

#define CL_CONFIG_CPU_VECTORIZER_MODE "CL_CONFIG_CPU_VECTORIZER_MODE"

using namespace Intel::OpenCL;

extern cl_device_type gDeviceType;

static bool deathTestFailure() {
#ifndef DEBUGGING_DEATH_TEST
  exit(1);
#endif
  return false;
}

static bool deathTestSuccess() {
#ifndef DEBUGGING_DEATH_TEST
  exit(0);
#endif
  return true;
}

static bool setVectorizerMode(std::string const &mode) {
  bool bResult = true;
  if (mode.length() > 0)
    bResult = SETENV(CL_CONFIG_CPU_VECTORIZER_MODE, mode.c_str());
  else
    UNSETENV(CL_CONFIG_CPU_VECTORIZER_MODE);

  if (!bResult)
    printf("ERROR: Cannot set CL_CONFIG_CPU_VECTORIZER_MODE=%s!\n",
           mode.c_str());
  return bResult;
}

static bool vectorizerModeTest(std::string const &mode) {

  if (gDeviceType != CL_DEVICE_TYPE_CPU) {
    // At the moment only CPU device supports this
    return deathTestSuccess();
  }

  bool bResult = setVectorizerMode(mode);
  if (!bResult) {
    return deathTestFailure();
  }

  const std::string kernelName("\"test_kernel\"");
  const char *ocl_test_program =
      "__kernel void test_kernel(__global float* inout)\n"
      "{\n"
      "  size_t gid = get_global_id(0);\n"
      "  inout[gid] = inout[gid] + inout[gid];\n"
      "}\n";

  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;

  cl_context context;
  cl_program clProg;

  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return deathTestFailure();
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // get device(s)
  iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return deathTestFailure();
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];

  // initialize arrays pDevices = new cl_device_id[uiNumDevices];
  iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    delete[] pDevices;
    return deathTestFailure();
  }

  // check if target architecture supports width 8
  // 1. check if back-end specific env. variable (CL_CONFIG_CPU_ARCH) sets cpu
  // architecture that doesn't support vector size 8.
  std::string cpuArch;
  if (Intel::OpenCL::Utils::getEnvVar(cpuArch, "CL_CONFIG_CPU_ARCH") &&
      ("corei7" == cpuArch) && mode == "8")
    return deathTestSuccess();

  // 2. Let OpenCL run-time to determine supported vector size.
  cl_int nativeWidth;
  iRet = clGetDeviceInfo(pDevices[0], CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT,
                         sizeof(cl_int), &nativeWidth, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceInfo = %s\n", ClErrTxt(iRet));
    delete[] pDevices;
    return deathTestFailure();
  }

  // check if device supports this width.
  if (mode == "8" && nativeWidth < 8) {
    // TODO: add exceptions tests
    return deathTestSuccess();
  }

  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  if (CL_SUCCESS != iRet) {
    printf("clCreateContext = %s\n", ClErrTxt(iRet));
    delete[] pDevices;
    return deathTestFailure();
  }
  printf("context = %zd\n", (std::size_t)context);

  bResult = BuildProgramSynch(context, 1, (const char **)&ocl_test_program,
                              NULL, NULL, &clProg);
  if (!bResult) {
    delete[] pDevices;
    return deathTestFailure();
  }

  std::size_t szLogSize = 0;
  // get the build log
  iRet = clGetProgramBuildInfo(clProg, pDevices[0], CL_PROGRAM_BUILD_LOG, 0,
                               NULL, &szLogSize);
  bResult =
      Check("clGetProgramBuildInfo(CL_PROGRAM_BUILD_LOG)", CL_SUCCESS, iRet);
  if (bResult) {
    // check the logs
    std::string strLog;
    strLog.resize(szLogSize);
    iRet = clGetProgramBuildInfo(clProg, pDevices[0], CL_PROGRAM_BUILD_LOG,
                                 szLogSize, &*strLog.begin(), &szLogSize);
    bResult =
        Check("clGetProgramBuildInfo(CL_PROGRAM_BUILD_LOG)", CL_SUCCESS, iRet);

    std::size_t place = strLog.find(kernelName, 0);
    if (!bResult || place == std::string::npos) {
      printf("ERROR: Cannot get log about %s!\n", kernelName.c_str());
      return deathTestFailure();
    }

    if (mode == "1") {
      const std::string expectedMsg(" was not vectorized");
      const std::string actualMsg =
          strLog.substr(place + kernelName.length(), expectedMsg.length());
      if (actualMsg != expectedMsg) {
        printf("\nERROR: %s was not supposed to be vectorized - log indicates "
               "otherwise!\n",
               kernelName.c_str());
        printf("Log:\n%s\n\n", strLog.c_str());
        return deathTestFailure();
      }
    } else {
      std::string vecWidthMsg;
      if (mode.length() > 0 &&
          mode != "0") // do not check the width in the autonomous mode
        vecWidthMsg +=
            " (" + mode + ")"; // the vectorized width is surrounted by brackets

      const std::string expectedMsg =
          std::string(" was successfully vectorized") + vecWidthMsg;
      const std::string actualMsg =
          strLog.substr(place + kernelName.length(), expectedMsg.length());
      if (actualMsg != expectedMsg) {
        printf("\nERROR: %s was supposed to be vectorized%s\n",
               kernelName.c_str(), vecWidthMsg.c_str());
        printf("Log:\n%s\n\n", strLog.c_str());
        return deathTestFailure();
      }
    }
  }

  // Release objects
  clReleaseProgram(clProg);
  clReleaseContext(context);
  delete[] pDevices;

  return deathTestSuccess();
}

static void vectorizerModeDeathTest(const char *mode) {
#ifdef DEBUGGING_DEATH_TEST
  EXPECT_TRUE(vectorizerModeTest(mode));
#else
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_EXIT(
      {
        vectorizerModeTest(mode);
        exit(1);
      },
      ::testing::ExitedWithCode(0), "");
#endif
}

static void checkAndTestVectorizerMode(DeviceBackend::ETransposeSize TSize) {
  const auto *CpuId = Utils::CPUDetect::GetInstance();
  std::string TSizeStr = std::to_string(TSize);
  if (CpuId->isTransposeSizeSupported(TSize) == Utils::SUPPORTED)
    ASSERT_NO_FATAL_FAILURE(vectorizerModeDeathTest(TSizeStr.c_str()));
  else
    printf("Skip test since TRANSPOSE_SIZE_%s is not supported on %s\n",
           TSizeStr.c_str(), CpuId->GetCPUName(CpuId->GetCPU()));
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Do not remove DeathTest suffix from the test names - it is the Google Test
// requirement
//
////////////////////////////////////////////////////////////////////////////////////////////
TEST(FrameworkTestTypeDeathTest, Test_VectorizerMode_Default) {
  vectorizerModeDeathTest("");
}

TEST(FrameworkTestTypeDeathTest, Test_VectorizerMode_0) {
  vectorizerModeDeathTest("0");
}

TEST(FrameworkTestTypeDeathTest, Test_VectorizerMode_1) {
  vectorizerModeDeathTest("1");
}

TEST(FrameworkTestTypeDeathTest, Test_VectorizerMode_4) {
  vectorizerModeDeathTest("4");
}

TEST(FrameworkTestTypeDeathTest, Test_VectorizerMode_8) {
  checkAndTestVectorizerMode(DeviceBackend::TRANSPOSE_SIZE_8);
}

TEST(FrameworkTestTypeDeathTest, Test_VectorizerMode_16) {
  checkAndTestVectorizerMode(DeviceBackend::TRANSPOSE_SIZE_16);
}

TEST(FrameworkTestTypeDeathTest, Test_VectorizerMode_32) {
  checkAndTestVectorizerMode(DeviceBackend::TRANSPOSE_SIZE_32);
}

TEST(FrameworkTestTypeDeathTest, Test_VectorizerMode_64) {
  checkAndTestVectorizerMode(DeviceBackend::TRANSPOSE_SIZE_64);
}

TEST(FrameworkTestTypeDeathTest, Test_VectorizerMode_64_AVX2) {
  ASSERT_TRUE(SETENV("CL_CONFIG_CPU_TARGET_ARCH", "core-avx2"));
  checkAndTestVectorizerMode(DeviceBackend::TRANSPOSE_SIZE_64);
}

TEST(FrameworkTestTypeDeathTest, Test_VectorizerMode_64_AVX) {
  ASSERT_TRUE(SETENV("CL_CONFIG_CPU_TARGET_ARCH", "corei7-avx"));
  checkAndTestVectorizerMode(DeviceBackend::TRANSPOSE_SIZE_64);
}

TEST(FrameworkTestTypeDeathTest, Test_VectorizerMode_64_SSE) {
  ASSERT_TRUE(SETENV("CL_CONFIG_CPU_TARGET_ARCH", "corei7"));
  checkAndTestVectorizerMode(DeviceBackend::TRANSPOSE_SIZE_64);
}
// TODO: add negative tests
// void negativeVectorizerModeDeathTest( const char* mode )
//{
// #ifdef DEBUGGING_DEATH_TEST
//    EXPECT_FALSE( vectorizerModeTest( mode ) );
// #else
//    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
//    EXPECT_EXIT( { vectorizerModeTest( mode ); exit(0); },
//    ::testing::ExitedWithCode(1), "" );
// #endif
//}
// TEST(FrameworkTestTypeDeathTest, Test_VectorizerMode_16)
//{
//    negativeVectorizerModeDeathTest( "16" );
//}
