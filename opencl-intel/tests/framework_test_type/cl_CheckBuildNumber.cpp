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

#include <iostream>
#include <string.h>
#include "test_utils.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

bool cl_CheckBuildNumber() {
    cl_int iRet = CL_SUCCESS;
    cl_platform_id platform = 0;
    bool bResult = true;
    cl_device_id device = NULL;

    try
    {
        iRet = clGetPlatformIDs(1, &platform, NULL);
        CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);
        iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
        CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

        char Buffer[1024];
        iRet = clGetDeviceInfo(device, CL_DRIVER_VERSION, 1024, Buffer, nullptr);
        CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

        Buffer[1024 - 1] = 0;
        cout << "CL_DRIVER_VERSION: " << Buffer << '\n';

        // The expected string in Buffer should be 'YYYY.L.MM.0',
        // where YYYY - current year, L - latest LLVM release version,
        // MM - current month (single digit until September) and 0 - internally
        // agreed digit.
        // We can't validate latest LLVM release version from here as well as
        // hardcoded '0', so let's validate build year YYYY and build month MM.
        // For Apr 2019 YYYY should be '2019' and MM '4'.

        // Check for 'YYYY' string:
        for (int i = 0; i != 4; ++i) {
            if (!isdigit(Buffer[i])) {
                CheckException("YYYY", true, false);
            }
        }
        // Year can't be less that the current one (2019)
        char Year[4];
        Year[0] = Buffer[0];
        Year[1] = Buffer[1];
        Year[2] = Buffer[2];
        Year[3] = Buffer[3];
        int Val = atoi(Year);
        CheckException("Year", Val >= 2019, true);

        // Check for 'MM' string:
        char Month[2];
        int i = 0;
        while (isdigit(Buffer[7 + i])) {
          Month[0] = Buffer[7 + i];
          ++i;
        }
        Val = atoi(Month);
        CheckException("Month", Val <= 12, true);

        if (Buffer[11]) {
            CheckException("Zero symbol was expected here", true, false);
        }
    }
    catch (const std::exception&)
    {
        bResult = false;
    }

    return bResult;
}
