// Copyright (c) 2018 Intel Corporation
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

//
// Regression test to validate
//

#include "test_utils.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <time.h>

extern cl_device_type gDeviceType;

bool cl_CheckBuildNumber() {
  cl_int iRet = CL_SUCCESS;
  cl_platform_id platform = 0;
  bool bResult = true;
  cl_device_id device = NULL;

  try {
    iRet = clGetPlatformIDs(1, &platform, NULL);
    CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);
    iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
    CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

    std::string DriverVersion(120, '\0');
    iRet = clGetDeviceInfo(device, CL_DRIVER_VERSION, 120, &DriverVersion[0],
                           nullptr);
    CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

    std::cout << "CL_DRIVER_VERSION: " << DriverVersion << '\n';

    // The expected string in Buffer should be 'YYYY.L{1,}.MM.0',
    // where YYYY - current year, L{1,} - latest LLVM release version,
    // MM - current month (single digit until September) and 0 - internally
    // agreed digit.
    // We can't validate latest LLVM release version from here as well as
    // hardcoded '0', so let's validate build year YYYY and build month MM.
    // For Apr 2019 YYYY should be '2019' and MM '4'.

    std::vector<std::string> tokens = tokenize(DriverVersion, ".");

    std::string Year = tokens[0];
    // Check for 'YYYY' string:
    for (int i = 0; i < 4; ++i) {
      if (!isdigit(Year[i])) {
        CheckException("YYYY", true, false);
      }
    }

    // Year can't be less that the current one
    time_t Time = time(NULL);
    struct tm *CurrentTime = localtime(&Time);
    unsigned int CurrentYear = CurrentTime->tm_year + 1900;
    unsigned int Val = atoi(Year.c_str());
    CheckException("Year", Val >= CurrentYear, true);

    // LLVM version shouldn't be empty
    std::string LLVMVersion = tokens[1];
    if (!LLVMVersion.size()) {
      CheckException("LLVM Version shouldn't be empty", true, false);
    }

    // Check for 'MM' string:
    std::string Month = tokens[2];
    Val = atoi(Month.c_str());
    CheckException("Month", Val <= 12, true);

    if (strcmp(tokens[3].c_str(), "0")) {
      CheckException("Zero symbol was expected in the end "
                     "of driver version",
                     true, false);
    }
  } catch (const std::exception &) {
    bResult = false;
  }

  return bResult;
}
