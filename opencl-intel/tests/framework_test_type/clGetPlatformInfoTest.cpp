// Copyright (c) 2013 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_config.h"
#include "cl_types.h"
#include "string.h"
#include <stdio.h>

/*******************************************************************************
 * clGetPlatformInfoTest
 * -------------------
 * Get platform info (CL_PLATFORM_PROFILE)
 * Get platform info (CL_PLATFORM_VERSION)
 ******************************************************************************/
using namespace Intel::OpenCL::Utils;

extern cl_device_type gDeviceType;

bool clGetPlatformInfoTest() {
  printf("---------------------------------------\n");
  printf("clGetPlatformInfo\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  char platformInfoStr[256];
  cl_version platformVersion;
  size_t size_ret;

  cl_platform_id platform = 0;

  bool isFPGAEmulator = false;
  if (gDeviceType == CL_DEVICE_TYPE_ACCELERATOR) {
    isFPGAEmulator = true;
  }

  cl_int iRes = clGetPlatformIDs(1, &platform, NULL);
  bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRes);
  if (!bResult) {
    return bResult;
  }

  const char *expectedProfile =
      isFPGAEmulator ? "EMBEDDED_PROFILE" : "FULL_PROFILE";
  size_t expectedProfileSize = strlen(expectedProfile) + 1;

  // CL_PLATFORM_PROFILE
  // all NULL is allowed, albeit useless.
  iRes = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 0, NULL, NULL);
  bResult &= Check("CL_PLATFORM_PROFILE, all NU", CL_SUCCESS, iRes);

  // CL_PLATFORM_PROFILE
  // get size only
  iRes = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 0, NULL, &size_ret);
  bResult &= Check("CL_PLATFORM_PROFILE, get size only", CL_SUCCESS, iRes);
  if (CL_SUCCEEDED(iRes)) {
    bResult &= CheckSize("check value", expectedProfileSize, size_ret);
  }

  // CL_PLATFORM_PROFILE
  // size < actual size
  iRes = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 5, platformInfoStr,
                           &size_ret);
  bResult &=
      Check("CL_PLATFORM_PROFILE, size < actual size", CL_INVALID_VALUE, iRes);

  // CL_PLATFORM_PROFILE
  // size ok, no size return
  iRes = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 256, platformInfoStr,
                           NULL);
  bResult &=
      Check("CL_PLATFORM_PROFILE, size ok, no size return", CL_SUCCESS, iRes);
  if (CL_SUCCEEDED(iRes)) {
    bResult &= CheckStr("check value", expectedProfile, platformInfoStr);
  }

  // CL_PLATFORM_VERSION
  // all NULL is allowed, albeit useless.
  iRes = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 0, NULL, NULL);
  bResult &= Check("CL_PLATFORM_VERSION, all NU", CL_SUCCESS, iRes);

  // CL_PLATFORM_NUMERIC_VERSION
  iRes =
      clGetPlatformInfo(platform, CL_PLATFORM_NUMERIC_VERSION, 0, NULL, NULL);
  bResult &= Check("CL_PLATFORM_NUMERIC_VERSION, all NU", CL_SUCCESS, iRes);

  // CL_PLATFORM_VERSION
  // get size only
  iRes = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 0, NULL, &size_ret);
  bResult &= Check("CL_PLATFORM_VERSION, get size only", CL_SUCCESS, iRes);
  if (CL_SUCCEEDED(iRes)) {
    if (isFPGAEmulator) {
      bResult &= CheckSize("check value", 58, size_ret);
    } else {
#ifdef _WIN32
      bResult &= CheckSize("check value", 19, size_ret);
#else
      bResult &= CheckSize("check value", 17, size_ret);
#endif
    }
  }

  // CL_PLATFORM_NUMERIC_VERSION
  // get size only
  iRes = clGetPlatformInfo(platform, CL_PLATFORM_NUMERIC_VERSION, 0, NULL,
                           &size_ret);
  bResult &=
      Check("CL_PLATFORM_NUMERIC_VERSION, get size only", CL_SUCCESS, iRes);
  if (CL_SUCCEEDED(iRes))
    bResult &= CheckSize("check value", sizeof(cl_version), size_ret);

  // CL_PLATFORM_VERSION
  // size < actual size
  iRes = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 5, platformInfoStr,
                           &size_ret);
  bResult &=
      Check("CL_PLATFORM_VERSION, size < actual size", CL_INVALID_VALUE, iRes);

  // CL_PLATFORM_NUMERIC_VERSION
  // size < actual size
  iRes = clGetPlatformInfo(platform, CL_PLATFORM_NUMERIC_VERSION, 1,
                           &platformVersion, &size_ret);
  bResult &= Check("CL_PLATFORM_NUMERIC_VERSION, size < actual size",
                   CL_INVALID_VALUE, iRes);

  // CL_PLATFORM_VERSION
  // size ok, no size return
  iRes = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 256, platformInfoStr,
                           NULL);
  bResult &=
      Check("CL_PLATFORM_VERSION, size ok, no size return", CL_SUCCESS, iRes);
  if (CL_SUCCEEDED(iRes)) {
    std::string expectedString;
    expectedString += "OpenCL ";

    if (isFPGAEmulator) {
      expectedString += "1.0 ";
    } else {
#ifdef BUILD_OPENCL_21
      expectedString += "2.1 ";
#else
      switch (GetOpenclVerByCpuModel()) {
      case OPENCL_VERSION_3_0:
        expectedString += "3.0 ";
        break;

      case OPENCL_VERSION_2_2:
        expectedString += "2.2 ";
        break;

      case OPENCL_VERSION_2_1:
        expectedString += "2.1 ";
        break;

      case OPENCL_VERSION_2_0:
        expectedString += "2.0 ";
        break;

      case OPENCL_VERSION_1_2:
        expectedString += "1.2 ";
        break;
      case OPENCL_VERSION_1_0:
      default:
        expectedString += "1.0 ";
        break;
      }
    }
#endif // BUILD_EXPERIMENTAL_21
      if (isFPGAEmulator) {
        expectedString += "Intel(R) FPGA SDK for OpenCL(TM), Version 19.1";
      } else {
#ifdef _WIN32
        expectedString += "WINDOWS";
#elif defined(__linux__)
      expectedString += "LINUX";
#else
#error Unhandled platform!
#endif
      }

      bResult &= CheckStr("check value", &expectedString[0], platformInfoStr);
    }

    // CL_PLATFORM_NUMERIC_VERSION
    // size ok, no size return
    iRes = clGetPlatformInfo(platform, CL_PLATFORM_NUMERIC_VERSION,
                             sizeof(platformVersion), &platformVersion, NULL);
    bResult &= Check("CL_PLATFORM_NUMERIC_VERSION, size ok, no size return",
                     CL_SUCCESS, iRes);
    if (CL_SUCCEEDED(iRes)) {
      cl_version expectedVersion;

      if (isFPGAEmulator) {
        expectedVersion = CL_MAKE_VERSION(1, 2, 0);
      } else {
        switch (GetOpenclVerByCpuModel()) {
        case OPENCL_VERSION_3_0:
          expectedVersion = CL_MAKE_VERSION(3, 0, 0);
          break;
        case OPENCL_VERSION_2_2:
          expectedVersion = CL_MAKE_VERSION(2, 2, 0);
          break;
        case OPENCL_VERSION_2_1:
          expectedVersion = CL_MAKE_VERSION(2, 1, 0);
          break;
        case OPENCL_VERSION_2_0:
          expectedVersion = CL_MAKE_VERSION(2, 0, 0);
          break;
        case OPENCL_VERSION_1_2:
          expectedVersion = CL_MAKE_VERSION(1, 2, 0);
          break;
        case OPENCL_VERSION_1_0:
        default:
          expectedVersion = CL_MAKE_VERSION(1, 0, 0);
          break;
        }
      }

      bResult &= CheckInt("check value", expectedVersion, platformVersion);
    }

    return bResult;
  }
